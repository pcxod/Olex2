/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "fsext.h"
#include "ememstream.h"
#include "etime.h"
#include "egc.h"
#include "eutf8.h"

#ifndef _NO_PYTHON
#include "pyext.h"
#endif

#include "wxzipfs.h"

  const int16_t TFileHandlerManager::FVersion = 0x0001;
  const char TFileHandlerManager::FSignature[]="ODF_"; // olex2 data file?
  const int TFileHandlerManager_FSignatureLength = 4;

TFileHandlerManager *TFileHandlerManager::FHandler = NULL;
TStrList TFileHandlerManager::BaseDirs;

TMemoryBlock *TFileHandlerManager::GetMemoryBlock(const olxstr& FN)  {
  olxstr fileName = TEFile::UnixPath(FN);
  TMemoryBlock *mb = FMemoryBlocks[fileName];
  if( mb == NULL )  {
    if( !TEFile::Exists(fileName) )  return NULL;
    TEFile file(fileName, "rb");
    const uint32_t fl = OlxIStream::CheckSize<uint32_t>(file.GetSize());
    if( fl == 0 ) return NULL;
    mb = new TMemoryBlock;
    mb->Buffer = new char [fl + 1];
    mb->Length = fl;
    mb->DateTime = TEFile::FileAge(fileName);
    file.Read(mb->Buffer, mb->Length);
    FMemoryBlocks.Add(fileName, mb);
  }
  else  {
    if( mb->DateTime != 0 && TEFile::Exists(fileName) )  {
      if( (uint64_t)TEFile::FileAge(fileName) > mb->DateTime )  {
        size_t ind = FMemoryBlocks.IndexOf(fileName);
        FMemoryBlocks.Delete(ind);
        delete [] mb->Buffer;
        delete mb;
        return GetMemoryBlock(fileName);
      }
    }
  }
  return mb;
}
//..............................................................................
TFileHandlerManager::TFileHandlerManager()  {
  if (FHandler != NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  TEGC::AddP(FHandler = this);
#ifndef _NO_PYTHON
  PythonExt::GetInstance()->Register(&TFileHandlerManager::PyInit);
#endif
}
//..............................................................................
TFileHandlerManager::~TFileHandlerManager()  {
  _Clear();
  FHandler = NULL;
}
//..............................................................................
void TFileHandlerManager::_Clear()  {
  for( size_t i=0; i < FMemoryBlocks.Count(); i++ )  {
    delete [] FMemoryBlocks.GetObject(i)->Buffer;
    delete FMemoryBlocks.GetObject(i);
  }
  FMemoryBlocks.Clear();
#ifdef __WXWIDGETS__
  for( size_t i=0; i < FZipFiles.Count(); i++ )
    delete FZipFiles.GetObject(i);
  FZipFiles.Clear();
#endif
}
//..............................................................................
IDataInputStream *TFileHandlerManager::_GetInputStream(const olxstr &FN)  {
#ifdef __WXWIDGETS__
  if( TZipWrapper::IsZipFile(FN) )  {
    TZipEntry ze;
    TZipWrapper::SplitZipUrl(FN, ze);
    TZipWrapper *zw = FZipFiles[ze.ZipName];
    if( zw == NULL )  {
      zw = new TZipWrapper( ze.ZipName, true );
      FZipFiles.Add( ze.ZipName, zw );
    }
    return zw->OpenEntry(ze.EntryName);
  }
  else  {
#endif
    TMemoryBlock *mb = GetMemoryBlock(FN);
    if( mb == NULL )  return NULL;
    TEMemoryStream *ms = new TEMemoryStream;
    ms->Write(mb->Buffer, mb->Length);
    ms->SetPosition(0);
    return ms;
#ifdef __WXWIDGETS__
  }
#endif
}
//..............................................................................
#ifdef __WXWIDGETS__
wxFSFile *TFileHandlerManager::_GetFSFileHandler(const olxstr &FN)  {
  static wxString st(wxT("OCTET")), es;
  if( TZipWrapper::IsZipFile(FN) )  {
    TZipEntry ze;
    TZipWrapper::SplitZipUrl(FN, ze);
    TZipWrapper *zw = FZipFiles[ze.ZipName];
    if( zw == NULL )  {
      zw = new TZipWrapper( ze.ZipName, true );
      FZipFiles.Add( ze.ZipName, zw );
    }
    wxInputStream *wxIS = zw->OpenWxEntry( ze.EntryName );
    return wxIS == NULL ? NULL : new wxFSFile(
      wxIS, ze.EntryName.u_str(), st, es, wxDateTime((time_t)0));
  }
  else  {
    TMemoryBlock *mb = GetMemoryBlock( FN );
    return mb == NULL ? NULL : new wxFSFile(new wxMemoryInputStream(
      mb->Buffer, mb->Length), es, st, es, wxDateTime((time_t)0));
  }
}
#endif
//..............................................................................
void TFileHandlerManager::_SaveToStream(IDataOutputStream& os,
  short persistenceMask)
{
  os.Write(FSignature, TFileHandlerManager_FSignatureLength);
  os << FVersion;

  uint32_t ic = 0, strl;
  for( size_t i=0; i < FMemoryBlocks.Count(); i++ )  {
    if( (FMemoryBlocks.GetObject(i)->PersistenceId & persistenceMask) != 0  )
      ic++;
  }
  os << ic;
  olxcstr utfstr;
  for( size_t i=0; i < FMemoryBlocks.Count(); i++ )  {
    TMemoryBlock *mb = FMemoryBlocks.GetObject(i);
    if( (mb->PersistenceId & persistenceMask) != 0 )  {
      utfstr = TUtf8::Encode(FMemoryBlocks.GetString(i));
      strl = (uint32_t)utfstr.Length();
      os << strl;
      os.Write((void*)utfstr.raw_str(), strl);
      os << mb->Length;
      os << mb->DateTime;
      if( mb->Length != 0 )
        os.Write(mb->Buffer, mb->Length);
    }
  }
}
//..............................................................................
void TFileHandlerManager::_LoadFromStream(IDataInputStream& is,
  short persistenceId)
{
  // validation of the stream
  char fSignature[TFileHandlerManager_FSignatureLength+1];
  is.Read( fSignature, TFileHandlerManager_FSignatureLength );
  fSignature[TFileHandlerManager_FSignatureLength] = '\0';
  if( olxstr(fSignature) != FSignature )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");
  int16_t fVersion;
  is >> fVersion;
  if( fVersion > FVersion )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file version");
  size_t length = OlxIStream::CheckSize<size_t>(is.GetSize());
  uint32_t ic, strl;
  is >> ic;
  for(uint32_t i=0; i < ic; i++ )  {
    is >> strl;
    if( strl > (uint32_t)(length - is.GetPosition()) )  {
      _Clear();
      throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
    }
    olxcstr utfstr;
    olxstr in = TUtf8::Decode(utfstr.AppendFromStream(is, strl));
    TMemoryBlock* mb = FMemoryBlocks[in];
    if( mb == NULL )  {
      mb = new TMemoryBlock;
      mb->PersistenceId = persistenceId;
      FMemoryBlocks.Add(in, mb);
    }
    else
      delete [] mb->Buffer;
    is >> mb->Length;
    if( fVersion == 0x0001 )  is >> mb->DateTime;
    // validate ...
    if( mb->Length > (length - is.GetPosition()) )  {
      mb->Buffer = new char[4];  // recover the potential errors
      _Clear();
      throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
    }

    mb->Buffer = new char [mb->Length + 1];
    if( mb->Length != 0 )
      is.Read(mb->Buffer, mb->Length);
  }
}
//..............................................................................
const TMemoryBlock* TFileHandlerManager::FindMemoryBlock(const olxstr& bn) {
  if( bn.IsEmpty() )  return NULL;
  if( FHandler == NULL )  new TFileHandlerManager;
  return FHandler->FMemoryBlocks[TEFile::UnixPath(bn)];
}
//..............................................................................
IDataInputStream *TFileHandlerManager::GetInputStream(const olxstr &FN)  {
  if( FN.IsEmpty() )  return NULL;
  if( FHandler == NULL )  new TFileHandlerManager;
  return FHandler->_GetInputStream(LocateFile(FN));
}
//..............................................................................
#ifdef __WXWIDGETS__
wxFSFile *TFileHandlerManager::GetFSFileHandler(const olxstr &FN)  {
  if( FHandler == NULL )  new TFileHandlerManager;
  return FHandler->_GetFSFileHandler(LocateFile(FN));
}
#endif
//..............................................................................
void TFileHandlerManager::Clear(short persistenceMask)  {
  if( FHandler != NULL )  {
    if( persistenceMask == ~0 )  {
      delete FHandler;
    }
    else  {
      for( size_t i=0; i < FHandler->FMemoryBlocks.Count(); i++ )  {
        TMemoryBlock *mb = FHandler->FMemoryBlocks.GetObject(i);
        if( (mb->PersistenceId & persistenceMask) != 0 )  {
          delete [] mb->Buffer;
          delete mb;
          FHandler->FMemoryBlocks.Delete(i--);
        }
      }
    }
  }
}
//..............................................................................
olxstr TFileHandlerManager::LocateFile(const olxstr& fn)  {
  if( FHandler->IsMemoryBlock(fn) )  return fn;
  if( !TEFile::IsAbsolutePath(fn) )  {
    olxstr f = TEFile::AddPathDelimeter(TEFile::CurrentDir());
    if( TEFile::Exists(f + fn) )  return f + fn;
    for( size_t i=0; i < BaseDirs.Count(); i++ )  {
      olxstr ffn = BaseDirs[i] + fn;
      if( TEFile::Exists(ffn) )
        return ffn;
    }
  }
  return fn;
}
//..............................................................................
void TFileHandlerManager::AddBaseDir(const olxstr& bd)  {
  BaseDirs.Add(TEFile::AddPathDelimeter(bd));
}
//..............................................................................
void TFileHandlerManager::_AddMemoryBlock(const olxstr& name, const char *bf,
  size_t length, short persistenceId)
{
  olxstr fileName = TEFile::UnixPath(name);
  TMemoryBlock *mb = FMemoryBlocks[fileName];
  if( mb == NULL )  {
    mb = new TMemoryBlock;
    FMemoryBlocks.Add(fileName, mb);
  }
  else
    delete [] mb->Buffer;
  mb->Buffer = new char [length + 1];
  mb->Length = (uint32_t)length;
  mb->DateTime = TETime::Now();
  if( length != 0 )
    memcpy(mb->Buffer, bf, length);
  mb->PersistenceId = persistenceId;
}
//..............................................................................
void TFileHandlerManager::AddMemoryBlock(const olxstr& name, const char *bf,
  size_t length, short persistenceId)
{
  if( FHandler == NULL )  FHandler = new TFileHandlerManager;
  return FHandler->_AddMemoryBlock(name, bf, length, persistenceId);
}
//..............................................................................
size_t TFileHandlerManager::Count()  {
  if( FHandler == NULL )  FHandler = new TFileHandlerManager;
  return FHandler->FMemoryBlocks.Count();
}
//..............................................................................
const olxstr& TFileHandlerManager::GetBlockName(size_t i)  {
  if( FHandler == NULL )  FHandler = new TFileHandlerManager;
  return FHandler->FMemoryBlocks.GetString(i);
}
//..............................................................................
size_t TFileHandlerManager::GetBlockSize(size_t i)  {
  if( FHandler == NULL )  FHandler = new TFileHandlerManager;
  return FHandler->FMemoryBlocks.GetObject(i)->Length;
}
//..............................................................................
olxstr TFileHandlerManager::GetBlockDateTime(size_t i)  {
  return FHandler->FMemoryBlocks.GetObject(i)->DateTime;
}
//..............................................................................
short TFileHandlerManager::GetPersistenceId(size_t i)  {
  return FHandler->FMemoryBlocks.GetObject(i)->PersistenceId;
}
//..............................................................................
void TFileHandlerManager::SaveToStream(IDataOutputStream& os,
  short persistenceMask)
{
  if( FHandler == NULL )  new TFileHandlerManager;
  FHandler->_SaveToStream(os, persistenceMask);
}
//..............................................................................
void TFileHandlerManager::LoadFromStream(IDataInputStream& is,
  short persistenceId)
{
  if( FHandler == NULL )  new TFileHandlerManager;
  FHandler->_LoadFromStream(is, persistenceId);
}
//..............................................................................
bool TFileHandlerManager::Exists(const olxstr& fn)  {
  if( FHandler == NULL )  new TFileHandlerManager;
  return FHandler->IsMemoryBlock(fn);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TFileHandlerManager::LibExists(const TStrObjList& Params,
  TMacroError& E)
{
  E.SetRetVal<bool>(IsMemoryBlock(Params[0]));
}
//..............................................................................
void TFileHandlerManager::LibDump(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  const TMemoryBlock *mb = FindMemoryBlock(Cmds[0]);
  if (mb == NULL) {
    Error.ProcessingError(__OlxSrcInfo, "Could not locate: ").quote() <<
      Cmds[0];
    return;
  }
  TEFile o(Cmds[1], "w+b");
  o.Write(mb->Buffer, mb->Length);
}
//..............................................................................
void TFileHandlerManager::LibClear(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  int mask=-1;
  if (!Cmds.IsEmpty())
    mask = Cmds[0].ToInt();
  TFileHandlerManager::Clear(mask);
}
//..............................................................................
TLibrary* TFileHandlerManager::ExportLibrary(const olxstr& name)  {
  if( FHandler == NULL )  new TFileHandlerManager();
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("fs") : name);
  lib->RegisterFunction<TFileHandlerManager>(
    new TFunction<TFileHandlerManager>(FHandler,  
      &TFileHandlerManager::LibExists, "Exists", fpOne,
    "Returns true if the specified file exists on the virtual file system")
  );
  lib->RegisterMacro<TFileHandlerManager>(
    new TMacro<TFileHandlerManager>(FHandler,  
      &TFileHandlerManager::LibDump, "Dump", EmptyString(), fpTwo,
    "Saves a file in the VFS to the disk file")
  );
  lib->RegisterStaticMacro(
    new TStaticMacro(&TFileHandlerManager::LibClear,
    "Clear", EmptyString(), fpNone|fpOne,
    "Clear the content of the VFS. A mask [-1] can be used to remove only "
    "items with particular persistence mask")
  );
  return lib;
}

//..............................................................................
//..............................................................................
//..............................................................................
#ifndef _NO_PYTHON
PyObject* fsext_pyExists(PyObject* self, PyObject* args)  {
  olxstr fn;
  PythonExt::ParseTuple(args, "w", &fn);
  return Py_BuildValue("b", TFileHandlerManager::Exists(fn));
}
//..............................................................................
PyObject* fsext_pyTimestamp(PyObject* self, PyObject* args)  {
  olxstr fn;
  PythonExt::ParseTuple(args, "w", &fn);
  const TMemoryBlock* mb = TFileHandlerManager::FindMemoryBlock(fn);
  if( mb == NULL )
    return PythonExt::PyNone();
  return Py_BuildValue("l", mb->DateTime );
}
//..............................................................................
PyObject* fsext_pyNewFile(PyObject* self, PyObject* args)  {
  char *data = NULL;
  olxstr name;
  int persistenceId = 0;
  int length = 0;
  if( !PythonExt::ParseTuple(args, "ws#|i",
        &name, &data, &length, &persistenceId) )
  {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ws#|i");
  }
  if( data != NULL && !name.IsEmpty() && length > 0 )  {
    TFileHandlerManager::AddMemoryBlock(name, data, length, persistenceId);
    return Py_BuildValue("b", true);
  }
  return Py_BuildValue("b", false);
}
//..............................................................................
PyObject* fsext_pyReadFile(PyObject* self, PyObject* args)  {
  olxstr name;
  if( !PythonExt::ParseTuple(args, "w", &name) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  IInputStream* io = TFileHandlerManager::GetInputStream(name);
  if( !name.IsEmpty() && io != NULL )  {
    const size_t is = io->GetAvailableSizeT();
    char * bf = new char [is + 1];
    io->Read(bf, is);
    PyObject* po = Py_BuildValue("s#", bf, is);
    delete [] bf;
    delete io;
    return po;
  }
  if( io != NULL )  {
    delete io;
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      "Empty file name");
  }
  else
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
    "File does not exist");
}
//..............................................................................
PyObject* fsext_pyClear(PyObject* self, PyObject* args)  {
  int mask=-1;
  if( !PythonExt::ParseTuple(args, "|i", &mask) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "|i");
  TFileHandlerManager::Clear(mask);
  return Py_BuildValue("b", true);
}
//..............................................................................

static PyMethodDef OLEXFS_Methods[] = {
  {"Exists", fsext_pyExists, METH_VARARGS,
    "returns true if specified file exists"},
  {"Timestamp", fsext_pyTimestamp, METH_VARARGS,
    "returns timestamp (epoch time) of given file, if file does not exist, "
    "returns None"},
  {"NewFile", fsext_pyNewFile, METH_VARARGS,
    "creates a new file (file_name, data,[persistence]), returns true if "
    "operation succeeded"},
  {"ReadFile", fsext_pyReadFile, METH_VARARGS,
    "reads previously created file and reurns the content of the file or None, "
    "if error has occured"},
  {"Clear", fsext_pyClear, METH_VARARGS,
    "clears content of the VFS. Mask can be given to remove items with "
    "particular persistence level"},
  {NULL, NULL, 0, NULL}
   };

void TFileHandlerManager::PyInit()  {
  Py_InitModule( "olex_fs", OLEXFS_Methods );
}
#endif //_NO_PYTHON

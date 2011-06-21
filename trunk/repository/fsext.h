#ifndef __olx_fsext_H
#define __olx_fsext_H

#ifdef __WXWIDGETS__
  #include <wx/filesys.h>
#endif

#include "efile.h"
#include "library.h"

class TZipWrapper;
//class TOZPFS;
struct TMemoryBlock  {
  char * Buffer;
  uint32_t Length;
  uint64_t DateTime;
  short PersistenceId; // dynamic property - not saved to a file
  TMemoryBlock()  {
    Buffer = NULL;
    Length = 0;
    DateTime = 0;
    PersistenceId = 0;  // not persistent
  }
};
/*____________________________________________________________________________*/
class TFileHandlerManager : public IEObject  {
#ifdef __WXWIDGETS__
  TSStrPObjList<olxstr,TZipWrapper*, false> FZipFiles;
//  TSStrPObjList<olxstr,TOZPFS*, false> FOZPFiles;
#endif
  TSStrPObjList<olxstr,TMemoryBlock*, false> FMemoryBlocks;
  static const int16_t FVersion;
  static const char FSignature[];
protected:
  TMemoryBlock *GetMemoryBlock( const olxstr &FN );
public:
  TFileHandlerManager();
  ~TFileHandlerManager();
protected:
  static TFileHandlerManager *FHandler;
  static TStrList BaseDirs;
  void _Clear();
  IDataInputStream *_GetInputStream(const olxstr &FN);
#ifdef __WXWIDGETS__
  wxFSFile *_GetFSFileHandler( const olxstr &FN );
#endif
  void _AddMemoryBlock(const olxstr& name, const char *bf, size_t length, short persistenceId);
  static olxstr LocateFile(const olxstr& fn);
  void _SaveToStream(IDataOutputStream& os, short persistenceMask);
  void _LoadFromStream(IDataInputStream& is, short persistenceId);
  inline bool IsMemoryBlock(const olxstr &EM) const {  
    return FMemoryBlocks[TEFile::UnixPath(EM)] != NULL;  
  }
public:
  static IDataInputStream *GetInputStream(const olxstr &FN);
#ifdef __WXWIDGETS__
  static wxFSFile *GetFSFileHandler( const olxstr &FN );
#endif
  static void Clear(short persistenceMask = ~0);
  static void AddBaseDir(const olxstr& bd);
  static void AddMemoryBlock(const olxstr& name, const char *bf, size_t length, short persistenceId);

  static void SaveToStream(IDataOutputStream& os, short persistenceMask);
  static void LoadFromStream(IDataInputStream& is, short persistenceId);
  
  static const TMemoryBlock* FindMemoryBlock(const olxstr& bn);
  static size_t Count();
  static const olxstr& GetBlockName(size_t i);
  static size_t GetBlockSize(size_t i);
  static const olxstr& GetBlockDateTime(size_t i);
  static short GetPersistenceId(size_t i);
  static bool Exists(const olxstr& fn);

  void LibExists(const TStrObjList& Params, TMacroError& E);
  static TLibrary* ExportLibrary(const olxstr& name=EmptyString());
protected:
#ifndef _NO_PYTHON
  static void PyInit();
#endif

};
#endif

#ifdef __BORLANDC__
  #pragma hdrstop
  #include <windows.h>
  #include <winbase.h>
#endif

#ifdef __WXWIDGETS__

#include "wxzipfs.h"
#include "efile.h"
#include "bapp.h"
#include "ememstream.h"

olxstr TZipWrapper::ZipUrlSignature = "@zip:";
//---------------------------------------------------------------------------
//..............................................................................
TMemoryBlock* TZipWrapper::GetMemoryBlock(const olxstr &EM)  {
  olxstr entryName = TEFile::UnixPath(EM);
  TMemoryBlock * mb = FMemoryBlocks[entryName];
  if( mb == NULL )  {
    wxZipEntry *entry = FEntries[TEFile::UnixPath(entryName)];
    if( !entry )  return NULL;
    if( !FInputStream->OpenEntry(*entry) )  return NULL;
    if( FInputStream->GetLength() <=0 )  {  FInputStream->CloseEntry();  return NULL;  }

    mb = new TMemoryBlock;
    mb->Length = FInputStream->GetLength();
    mb->Buffer = new char [mb->Length + 1];
    mb->DateTime = entry->GetDateTime().GetTicks();
    FInputStream->Read( mb->Buffer, FInputStream->GetLength());
    FInputStream->CloseEntry();
    FMemoryBlocks.Add( entryName, mb );
  }
  return mb;
}
//..............................................................................
TZipWrapper::TZipWrapper(const olxstr &zipName)  {
  FInputStream = NULL;
  FFileInputStream = NULL;
  if( !TEFile::FileExists( zipName )  )  return;
  FFileInputStream = new wxFileInputStream( zipName.u_str() );
  FInputStream = new wxZipInputStream( *FFileInputStream );
  wxZipEntry *entry;
  while( (entry = FInputStream->GetNextEntry() ) != NULL )
    FEntries.Add( TEFile::UnixPath(entry->GetName().c_str()), entry );
}
//..............................................................................
TZipWrapper::~TZipWrapper()  {
  TMemoryBlock *mb;
  for( int i=0; i < FMemoryBlocks.Count(); i++ )  {
    mb = FMemoryBlocks.Object(i);
    delete [] mb->Buffer;
    delete mb;
  }
  if( FInputStream )  delete FInputStream;
  if( FFileInputStream )  delete FFileInputStream;
}
//..............................................................................
IDataInputStream* TZipWrapper::OpenEntry(const olxstr &EN)  {
  TMemoryBlock * mb = GetMemoryBlock(EN);
  if( mb == NULL )  return NULL;

  TEMemoryStream *ms = new TEMemoryStream();
  ms->Write( mb->Buffer, mb->Length );
  ms->SetPosition(0);
  return ms;
}
//..............................................................................
wxInputStream* TZipWrapper::OpenWxEntry(const olxstr &EN)  {
  TMemoryBlock * mb = GetMemoryBlock(EN);
  return mb == NULL ? NULL : new wxMemoryInputStream(mb->Buffer, mb->Length);
}
//..............................................................................
bool TZipWrapper::IsValidFileName(const olxstr &FN)  {
  int zi = FN.IndexOf(ZipUrlSignature);
  if( zi > 0 )  {
    if( TEFile::FileExists( ExtractZipName(FN) ) )  return true;
    return false;
  }
  return TEFile::FileExists(FN);
}
//..............................................................................
bool TZipWrapper::IsZipFile(const olxstr &FN)  {
  return FN.IndexOf(ZipUrlSignature) > 0;
}
//..............................................................................
olxstr TZipWrapper::ExtractZipName(const olxstr &FN)  {
  int zi = FN.IndexOf(ZipUrlSignature);
  if( zi > 0 )  return FN.SubStringTo(zi);
  return EmptyString;
}
//..............................................................................
olxstr TZipWrapper::ExtractZipEntryName(const olxstr &FN)  {
  int zi = FN.IndexOf(ZipUrlSignature);
  if( zi > 0 )  return FN.SubStringFrom(zi+ZipUrlSignature.Length());
  return EmptyString;
}
//..............................................................................
bool TZipWrapper::SplitZipUrl(const olxstr &fullName, TZipEntry &ZE)  {
  int zi = fullName.IndexOf(ZipUrlSignature);
  if( zi == -1 )  return false;
  ZE.ZipName = fullName.SubStringTo(zi);
  ZE.ZipName = TEFile::UnixPath( ZE.ZipName );
  ZE.EntryName = fullName.SubStringFrom( zi + ZipUrlSignature.Length() );
  ZE.EntryName = TEFile::UnixPath( ZE.EntryName );
  return true;
}
//..............................................................................
olxstr TZipWrapper::ComposeFileName(const olxstr &ZipFileNameA, const olxstr &FNA)  {
  int zi = ZipFileNameA.IndexOf(ZipUrlSignature);
  if( zi == -1 )  return FNA;
  olxstr ZipFileName = TEFile::WinPath(ZipFileNameA),
           FN = TEFile::WinPath(FNA);
  olxstr zipPart = ZipFileName.SubStringTo(zi+ZipUrlSignature.Length());
  olxstr zipPath = TEFile::ExtractFilePath(zipPart);
  TEFile::AddTrailingBackslashI(zipPath);
  if( FN.StartsFrom(zipPath) )
    return zipPart + TEFile::UnixPath( FN.SubStringFrom(zipPath.Length()) );
  return FNA;
}
/*____________________________________________________________________________*/


TwxZipFileSystem::TwxZipFileSystem(const olxstr& zip_name) : zip(zip_name) { }
//..............................................................................
IDataInputStream* TwxZipFileSystem::OpenFile(const olxstr& Source)  {
  TOnProgress Progress;
  olxstr fn( TEFile::UnixPath(Source) );
  Progress.SetAction( olxstr("Extracting ") << fn );
  Progress.SetPos(0);
  Progress.SetMax(1);
  TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
  
  IDataInputStream* rv = zip.OpenEntry(Source);
  if( rv == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate zip entry: ") << fn);
  
  Progress.SetAction("Done");
//  Progress.SetPos( TEFile::FileLength(TmpFN) );
  TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
  return rv;
}
//..............................................................................

#endif // __WXWIDGETS__

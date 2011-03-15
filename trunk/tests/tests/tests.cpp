#include <stdio.h>
#include <iostream>

#include "testsuit.h"
#include "xapp.h"
#include "outstream.h"
#include "edict.h"
#include "olxth.h"
#include "ellist.h"
#include "symmat.h"
#include "filetree.h"
#include "cif.h"
//..............................
#include "tests/sort_test.h"
#include "tests/encoding_test.h"
#include "tests/string_test.h"
#include "tests/file_test.h"
#include "tests/sys_test.h"
#include "tests/spline_test.h"
#include "tests/matrix_test.h"
#include "tests/mat_id_test.h"
#include "tests/symmparser_test.h"
#include "tests/vcov_test.h"
#include "tests/irange_test.h"
#include "tests/reflection_test.h"

class Listener : public AActionHandler  {
public:
  virtual bool Execute(const IEObject *Sender, const IEObject *Data) {  
    if( EsdlInstanceOf(*Data, TOnProgress) )  {
      TBasicApp::GetLog() << '\r' << ((TOnProgress*)Data)->GetAction() << "     "; 
      return true; 
    }
    return false;
  }
  virtual bool OnExit(const IEObject *Sender, const IEObject *Data)  {
    TBasicApp::NewLogEntry();
    return true;
  }
};

int main(int argc, char* argv[]) {
	TXApp xapp(argv[0]);
  xapp.XFile().RegisterFileFormat(new TIns, "ins");
  xapp.GetLog().AddStream(new TOutStream, true);
  xapp.GetLog().AddStream(TUtf8File::Create(xapp.GetBaseDir()+"log.out", false), true);
  OlxTests tests;
  tests.Add(&test::IsNumberTest);
  tests.Add(&test::ListTests<TArrayList<int> >).
    Add(&test::ListTests<TTypeList<int> >).
    Add(&test::DirectionalListTest);
  tests.Add(&test::MD5Test).
    Add(&test::SHA1Test).
    Add(&test::SHA2Test);
  tests.Add(new test::CriticalSectionTest(true), &test::CriticalSectionTest::DoTest).
    Add(new test::CriticalSectionTest(false), &test::CriticalSectionTest::DoTest);
  tests.Add(&test::RelativePathTest);
  tests.Add(new test::SortTest, &test::SortTest::DoTest);
  tests.Add(&test::TestSVD).
    Add(&test::TestInvert).
    Add(&test::TestLU).
    Add(&test::TestQR).
    Add(&test::TestCholesky);
  tests.Add(&test::spline_test);
  tests.Add(&full_smatd_id<>::Tests).Add(&test::rotation_id_test);
  tests.Add(&test::symm_parser_tests);
  tests.Add(&smatd::Tests);
  tests.Add(&test::vcov_test);
  tests.Add(&test::IndexRangeTest);
  tests.Add(&test::reflection_tests);
  tests.run();
  if( argc > 1 )  {
    olxstr data_dir = argv[1];
    if( !TEFile::Exists(data_dir) || !TEFile::IsDir(data_dir) )  {
      xapp.NewLogEntry() << "Skipping IO tests - no valid folder is given";
      return 0;
    }
    TFileTree ft(data_dir);
    Listener lsnr;
    ft.OnExpand->Add(&lsnr);
    ft.Expand();
    ft.OnExpand->Remove(&lsnr);
    TStrList files;
    ft.GetRoot().ListFiles(files, "*.cif");
    for( size_t i=0; i < files.Count(); i++ )  {
      try  {
        TCif cif;
        cif.LoadFromFile(files[i]);
      }
      catch(const TExceptionBase& e)  {
        TStrList out;
        e.GetException()->GetStackTrace(out);
        xapp.NewLogEntry().nl() << files[i] << ':';
        xapp.NewLogEntry() << out;
      }
    }
    files.Clear();
    ft.GetRoot().ListFiles(files, "*.ins;*.res");
    for( size_t i=0; i < files.Count(); i++ )  {
      try  {
        xapp.XFile().LoadFromFile(files[i]);
      }
      catch(const TExceptionBase& e)  {
        TStrList out;
        e.GetException()->GetStackTrace(out);
        xapp.NewLogEntry().nl() << files[i] << ':';
        xapp.NewLogEntry() << out;
      }
    }
  }
  xapp.NewLogEntry() << "Finished...";
  std::cin.get();
  return 0;
}

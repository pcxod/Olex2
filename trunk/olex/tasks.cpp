#include "tasks.h"
#include "xglapp.h"
#include "mainform.h"
#include "msgbox.h"
#include "ins.h"

void P4PTask::Run() {
  if (file_id.Contains("Rigaku CrystalClear")) {
    TMainForm *mf = TGlXApp::GetMainForm();
    olxstr cmd = "spy.xplain.exists()";
    mf->ProcessFunction(cmd, __OlxSrcInfo);
    if (cmd.IsBool() && cmd.ToBool()) {
      olxstr opt =
        TBasicApp::GetInstance().GetOptions().FindValue("run_rigaku_xplain");
      bool run_xplain=false;
      if (opt.IsEmpty()) {
        olxstr rv = TdlgMsgBox::Execute(mf, "Would you like to run "
          "Rigaku XPlain for the space group determination?",
          "Rigaku P4P file import",
          "Do this for all Rigaku P4P files",
          wxYES|wxNO|wxICON_QUESTION, true);
        run_xplain = rv.Contains('Y');
        if (rv.Containsi('R'))
          mf->UpdateUserOptions("run_rigaku_xplain", run_xplain);
      }
      else {
        run_xplain = opt.ToBool();
      }
      if (run_xplain) {
        olxstr loaded_fn = TXApp::GetInstance().XFile().GetFileName();
        olxstr exts[] = {"ins", "res", "cif"};
        bool cell_src_exists = false;
        for (size_t i=0; i < sizeof(exts)/sizeof(exts[0]); i++) {
          olxstr n = TEFile::ChangeFileExt(loaded_fn, exts[i]);
          if (TEFile::Exists(n)) {
            cell_src_exists = true;
            break;
          }
        }
        if (!cell_src_exists) {
          TIns ins;
          ins.Adopt(TXApp::GetInstance().XFile());
          ins.SaveForSolution(TEFile::ChangeFileExt(loaded_fn, "ins"), "TREF",
            "Imported by Olex2");
        }
        olxstr fn = "spy.xplain.run(false, true)",
          rv;
        mf->executeFunction(fn, rv);
        if (rv.IsBool() && rv.ToBool()) {
          mf->ProcessMacro("reap", __OlxSrcInfo);
        }
        else {
          TBasicApp::NewLogEntry() << "Failed to run XPlain";
        }
      }
    }
  }
}

void CellChangeTask::Run() {
  TMainForm *mf = TGlXApp::GetMainForm();
  if (hklsrc != TXApp::GetInstance().XFile().GetRM().GetHKLSource())
    return;
  TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
  olxstr o, n;
  for (size_t i=0; i < 3; i++) {
    o << ' ' << TEValueD(au.GetAxes()[i], au.GetAxisEsds()[i]).ToString() <<
      ' ' << TEValueD(au.GetAngles()[i], au.GetAngleEsds()[i]).ToString();
    n << ' ' << TEValueD(cell[i], esds[i]).ToString() <<
      ' ' << TEValueD(cell[3+i], esds[3+i]).ToString();
  }
  olxstr opt = TBasicApp::GetInstance().GetOptions().FindValue("use_hkl_cell");
  bool use = false;
  if (opt.IsEmpty() && o != n) {
    olxstr msg = "Cell parameters in your HKL file differ from currently "
      "used.\nCurrent:";
    msg << o << "\nNew:" << n << "\nWould you like to update them?";
    olxstr rv = TdlgMsgBox::Execute(mf, msg,
      "Different cells",
      "Remember my decision",
      wxYES|wxNO|wxICON_QUESTION, true);
    use = rv.Contains('Y');
    if (rv.Containsi('R'))
      mf->UpdateUserOptions("use_hkl_cell", use);
  }
  if (use) {
   TAsymmUnit &au1 =
     TXApp::GetInstance().XFile().LastLoader()->GetAsymmUnit();
    au1.GetAxes() = au.GetAxes() = vec3d(cell[0], cell[1], cell[2]);
    au1.GetAngles() = au.GetAngles() = vec3d(cell[3], cell[4], cell[5]);
    au1.GetAxisEsds() = au.GetAxisEsds() = vec3d(esds[0], esds[1], esds[2]);
    au1.GetAngleEsds() = au.GetAngleEsds() = vec3d(esds[3], esds[4], esds[5]);
    TBasicApp::NewLogEntry() <<
      "Updating cell parameters with value from HKL file";
  }
}

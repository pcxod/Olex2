//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>

#include "bapp.h"
#include <Dialogs.hpp>
#include "frame1.h"
//---------------------------------------------------------------------------
class TfMain : public TForm
{
__published:	// IDE-managed Components
  TPanel *Panel2;
  TImage *iSplash;
  TfrMain *frMain;
  void __fastcall iSplashMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
  void __fastcall iSplashMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall iSplashMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall bbBrowseClick(TObject *Sender);
  void __fastcall bbDoneClick(TObject *Sender);
  void __fastcall bbInstallClick(TObject *Sender);
  void __fastcall bbUninstallClick(TObject *Sender);
  void __fastcall cbProxyClick(TObject *Sender);
  void __fastcall FormPaint(TObject *Sender);
  void __fastcall sbPickZipClick(TObject *Sender);
private:	// User declarations
  bool Dragging, MouseDown;
  int MouseDownX, MouseDownY;
  bool Expanded;
  TBasicApp* Bapp;
  class TProgress* Progress;
  bool OlexInstalled;
  olxstr OlexInstalledPath;
protected:
  bool InitRegistry(const AnsiString& installPath);
  bool CleanRegistry();
  bool CleanInstallationFolder(class TFSItem& item);
  bool CheckOlexInstalled(olxstr& installPath);

  //the return value is the licence acceptance ...
  bool DoInstall(const olxstr& zipFile, const olxstr& installPath);
  static olxstr SettingsFile;

  bool LaunchFile( const olxstr& fileName, bool do_exit );
public:		// User declarations
  __fastcall TfMain(TComponent* Owner);
  __fastcall ~TfMain();
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif

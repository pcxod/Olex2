/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef _xl_mainformH
#define _xl_mainformH

#include "olex2app_imp.h"
#include "ctrls.h"
#include "eprocess.h"
#include "glconsole.h"
#include "datafile.h"
#include "gltextbox.h"
#include "ipimp.h"
#include "macroerror.h"
#include "library.h"
#include "eaccell.h"
#include "estlist.h"
#include "updateth.h"
#include "macrolib.h"
#include "exparse/exptree.h"
#include "nui/nui.h"
#include "tasks.h"
#include "olxstate.h"

#include "wx/wx.h"
#include "wx/dnd.h"
#include "wx/process.h"
#include "wx/thread.h"

enum  {
  ID_HtmlPanel=wxID_HIGHEST,  // view menu

  ID_StrGenerate,  // structure menu

  ID_MenuTang,  // menu item ids
  ID_MenuBang,
  ID_MenuGraphics,
  ID_MenuModel,
  ID_MenuView,
  ID_MenuFragment,
  ID_MenuDrawStyle,
  ID_MenuDrawQ,
  ID_MenuItemAtomInfo,
  ID_MenuItemBondInfo,
  ID_MenuAtomType,
  ID_MenuAtomOccu,
  ID_MenuAtomConn,
  ID_MenuAtomPoly,
  ID_MenuAtomPart,
  ID_MenuAtomUiso,

  ID_DSBS,  // drawing style, balls and sticks
  ID_DSES,  // ellipsoids and sticks
  ID_DSSP,  // sphere packing
  ID_DSWF,  // wireframe
  ID_DSST,   // sticks
  ID_SceneProps,

  ID_DQH,  // drawing quality
  ID_DQM,
  ID_DQL,

  ID_CellVisible,  // model menu
  ID_BasisVisible,
  ID_ShowAll,
  ID_ModelCenter,

  ID_AtomTypeChangeC,
  ID_AtomTypeChangeN,
  ID_AtomTypeChangeO,
  ID_AtomTypeChangeF,
  ID_AtomTypeChangeH,
  ID_AtomTypeChangeS,
  ID_AtomTypePTable,
  ID_AtomGrow,
  ID_AtomCenter,
  ID_AtomSelRings,
  
  ID_PlaneActivate,
  ID_BondViewAlong,

  ID_FragmentHide,  // fragment menu
  ID_FragmentShowOnly,
  ID_FragmentSelectAtoms,
  ID_FragmentSelectBonds,
  ID_FragmentSelectAll,
  ID_FileLoad,
  ID_FileClose,

  ID_View100,   // view menu
  ID_View010,
  ID_View001,
  ID_View110,
  ID_View101,
  ID_View011,
  ID_View111,

  ID_AtomOccu1,
  ID_AtomOccu34,
  ID_AtomOccu12,
  ID_AtomOccu13,
  ID_AtomOccu14,
  ID_AtomOccuFix,
  ID_AtomOccuFree,

  ID_AtomConn0,
  ID_AtomConn1,
  ID_AtomConn2,
  ID_AtomConn3,
  ID_AtomConn4,
  ID_AtomConn12,

  ID_AtomUisoCustom,
  ID_AtomUiso15,
  ID_AtomUiso12,
  ID_AtomUisoFree,
  ID_AtomUisoFix,

  ID_AtomPartCustom,
  ID_AtomPart_2,
  ID_AtomPart_1,
  ID_AtomPart0,
  ID_AtomPart1,
  ID_AtomPart2,

  ID_AtomPolyNone,
  ID_AtomPolyAuto,
  ID_AtomPolyRegular,
  ID_AtomPolyPyramid,
  ID_AtomPolyBipyramid,

  ID_Selection,
  ID_SelGroup,
  ID_SelUnGroup,
  ID_SelLabel,

  ID_GridMenu,
  ID_GridMenuCreateBlob,

  ID_GraphicsKill,
  ID_GraphicsHide,
  ID_GraphicsDS,
  ID_GraphicsP,
  ID_GraphicsEdit,
  ID_GraphicsSelect,

  ID_GStyleSave,
  ID_GStyleOpen,
  ID_FixLattice,
  ID_FreeLattice,
  ID_DELINS,
  ID_ADDINS,
  ID_VarChange,
  ID_BadReflectionSet,
  ID_CellChanged,

  ID_UpdateThreadTerminate,
  ID_UpdateThreadDownload,
  ID_UpdateThreadAction,

  ID_FILE0,

  ID_GLDRAW = ID_FILE0+100,
  ID_TIMER,
  ID_INFO,
  ID_WARNING,
  ID_ERROR,
  ID_EXCEPTION,
  ID_LOG,
  ID_ONLINK,
  ID_HTMLKEY,
  ID_COMMAND,
  ID_XOBJECTSDESTROY,
  ID_CMDLINECHAR,
  ID_CMDLINEKEYDOWN,
  ID_TEXTPOST,
  ID_UPDATE_GUI
};

//............................................................................//
const unsigned short
  mListen = 0x0001,    // modes
  mSilent = 0x0002,  // silent mode
  mPick   = 0x0020,  // pick mode, for a future use
  mFade   = 0x0080,  // structure fading ..
  mRota   = 0x0100,  // rotation
  mSolve  = 0x0200,  // structure solution
  mSGDet  = 0x0400;  // space group determination

// persistence level
const short
  plNone      = 0x0000,  // runtime data only - not saved at any moment
  plStructure = 0x0001,  // data saved/loaded when structure is un/loaded
  plGlobal    = 0x0002;  // data saved/loaded when olex is closed/executed

class TMainForm;
class TGlXApp;

//............................................................................//
class TMainForm: public TMainFrame, public AEventsDispatcher,
  public olex2::OlexProcessorImp
{
  //TFrameMaker FrameMaker;
protected:
  bool Destroying;
  TStack<AnAssociation2<wxCursor,wxString> > CursorStack;
  UpdateThread* _UpdateThread;
  TOnProgress* UpdateProgress, *ActionProgress;
  TEFile* ActiveLogFile;
  static void PyInit();
  TActionQList Action;
  TGlXApp* FParent;
  TArrayList< AnAssociation2<TDUnitCell*, TSpaceGroup*> > UserCells;
  TCSTypeList<olxstr, olxstr> StoredParams;

  TTypeList<TScheduledTask> Tasks;
  TPtrList<IOlxTask> RunWhenVisibleTasks;

  class TGlCanvas *FGlCanvas;
  Olex2App* FXApp;
  TDataFile FHelpFile, FMacroFile;
  TDataItem *FHelpItem;
  
  olxstr GradientPicture;
  TGlConsole *FGlConsole;
  TGlTextBox *FHelpWindow, *FInfoBox, *GlTooltip;
  // a list of commands called when a file is changed by another process
  TStrList FOnListenCmds;
  TMacroError MacroError;
  
  void PreviewHelp(const olxstr& Cmd);
  olxstr ExpandCommand(const olxstr &Cmd, bool inc_files);
  int MouseMoveTimeElapsed, MousePositionX, MousePositionY;

  TModeRegistry *Modes;
  size_t
    stateHtmlVisible,
    stateInfoWidnowVisible,
    stateHelpWindowVisible,
    stateCmdLineVisible,
    stateGlTooltips;
   // solution mode variables
  TIntList Solutions;
  int CurrentSolution;
  olxstr SolutionFolder;
  void ChangeSolution(int sol);

  // helper functions ...
  void CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS);
  void UpdateInfoBox();
  olx_nui::INUI *nui_interface;
  class THtmlManager &HtmlManager;
public:
  void OnMouseMove(int x, int y);
  void OnMouseWheel(int x, int y, double delta);
  bool OnMouseDown(int x, int y, short Flags, short Buttons);
  bool OnMouseUp(int x, int y, short Flags, short Buttons);
  bool OnMouseDblClick(int x, int y, short Flags, short Buttons);
  virtual bool Show(bool v);

  void SetUserCursor(const olxstr& param, const olxstr& mode);

  const TModeRegistry& GetModes() const {  return *Modes;  }

  bool CheckMode(size_t mode, const olxstr& modeData);
  bool CheckState(size_t state, const olxstr& stateData) const;
  bool PopupMenu(wxMenu* menu, const wxPoint& p=wxDefaultPosition);
  bool PopupMenu(wxMenu* menu, int x, int y)  {
    return PopupMenu(menu, wxPoint(x,y));
  }
protected:
  void PostCmdHelp(const olxstr &Cmd, bool Full=false);

  void OnSize(wxSizeEvent& event);
  void OnMove(wxMoveEvent& event);

  void OnQuit(wxCommandEvent& event);
  void OnFileOpen(wxCommandEvent& event);
  void OnGenerate(wxCommandEvent& event);
  void OnDrawStyleChange(wxCommandEvent& event);
  void OnDrawQChange(wxCommandEvent& event);
  void OnViewAlong(wxCommandEvent& event);
  void OnCloseWindow(wxCloseEvent &evt);
  void OnInternalIdle();

  friend class TObjectVisibilityChange;
  void BasisVChange();
  void CellVChange();
  void GridVChange();
  void FrameVChange();
  void OnBasisVisible(wxCommandEvent& event);
  void OnCellVisible(wxCommandEvent& event);

  AGDrawObject *FObjectUnderMouse;  //initialised when mouse clicked on an object on screen

  void OnGraphics(wxCommandEvent& event);
  void OnFragmentHide(wxCommandEvent& event);
  void OnShowAll(wxCommandEvent& event);
  void OnModelCenter(wxCommandEvent& event);
  void OnFragmentShowOnly(wxCommandEvent& event);
  void OnFragmentSelectAtoms(wxCommandEvent& event);
  void OnFragmentSelectBonds(wxCommandEvent& event);
  void OnFragmentSelectAll(wxCommandEvent& event);
  // helper function to get the list of fragments (if several selected)
  size_t GetFragmentList(TNetPList& res);

  void OnAtomTypeChange(wxCommandEvent& event);
  void OnAtomOccuChange(wxCommandEvent& event);
  void OnAtomConnChange(wxCommandEvent& event);
  void OnAtomPartChange(wxCommandEvent& event);
  void OnAtomUisoChange(wxCommandEvent& event);
  void OnAtomPolyChange(wxCommandEvent& event);
  void OnAtomTypePTable(wxCommandEvent& event);
  void OnAtom(wxCommandEvent& event); // general handler

  void OnBond(wxCommandEvent& event);
  void OnPlane(wxCommandEvent& event); // general handler

  void OnSelection(wxCommandEvent& event);
  void OnGraphicsStyle(wxCommandEvent& event);

  // view menu
  void OnHtmlPanel(wxCommandEvent& event);
  bool ImportFrag(const olxstr& line);
  static size_t DownloadFiles(const TStrList &files, const olxstr &dest);
  static bool DownloadFile(const olxstr &url, const olxstr &dest) {
    return DownloadFiles(TStrList() << url, dest) != 0;
  }
  // macro functions
private:

  DefMacro(Reap)
  DefMacro(Pict)
  DefMacro(Picta)
  DefMacro(PictPS)
  DefMacro(PictTEX)
  DefMacro(PictS)
  DefMacro(PictPR)
  DefMacro(Group)
  DefMacro(Clear)
  DefMacro(Rota)
  DefMacro(Listen)
  DefMacro(WindowCmd)
  DefMacro(ProcessCmd)
  DefMacro(Wait)
  DefMacro(SwapBg)
  DefMacro(Silent)
  DefMacro(Stop)
  DefMacro(Echo)
  DefMacro(Post)
  DefMacro(Exit)
  DefMacro(SetEnv)
  DefMacro(Help)
  DefMacro(Qual)
  DefMacro(AddLabel)
  DefMacro(Hide)
  DefMacro(Exec)
  DefMacro(Shell)
  DefMacro(Save)
  DefMacro(Load)
  DefMacro(Link)
  DefMacro(Style)
  DefMacro(Scene)

  DefMacro(Lines)

  DefMacro(Ceiling)
  DefMacro(Fade)
  DefMacro(WaitFor)

  DefMacro(HtmlPanelSwap)
  DefMacro(HtmlPanelWidth)
  DefMacro(HtmlPanelVisible)
  DefMacro(QPeakScale)
  DefMacro(QPeakSizeScale)

  DefMacro(Focus)
  DefMacro(Refresh)

  DefMacro(Mode)

  DefMacro(Text)
  DefMacro(ShowStr)

  DefMacro(Bind)

  DefMacro(Grad)

  DefMacro(EditAtom)
  DefMacro(EditIns)
  DefMacro(HklEdit)
  DefMacro(HklView)
  DefMacro(HklExtract)

  DefMacro(ViewGrid)

  DefMacro(Popup)

  DefMacro(Python)

  DefMacro(CreateMenu)
  DefMacro(DeleteMenu)
  DefMacro(EnableMenu)
  DefMacro(DisableMenu)
  DefMacro(CheckMenu)
  DefMacro(UncheckMenu)

  DefMacro(CreateShortcut)
  DefMacro(SetCmd)

  DefMacro(UpdateOptions)
  DefMacro(Update)
  DefMacro(Reload)
  DefMacro(StoreParam)
  DefMacro(SelBack)

  DefMacro(CreateBitmap)
  DefMacro(DeleteBitmap)
  DefMacro(Tref)
  DefMacro(Patt)

  DefMacro(InstallPlugin)
  DefMacro(SignPlugin)
  DefMacro(UninstallPlugin)

  DefMacro(UpdateFile)
  DefMacro(NextSolution)

  DefMacro(ShowWindow)

  DefMacro(OFileDel)
  DefMacro(OFileSwap)

  DefMacro(Schedule)
  DefMacro(Test)

  DefMacro(IT)
  DefMacro(StartLogging)
  DefMacro(ViewLattice)
  DefMacro(AddObject)
  DefMacro(DelObject)

  DefMacro(ImportFrag)
  DefMacro(ExportFrag)

  DefMacro(OnRefine)
  DefMacro(TestMT)
  DefMacro(SetFont)
  DefMacro(EditMaterial)
  DefMacro(SetMaterial)
  DefMacro(TestBinding)
  DefMacro(ShowSymm)
  DefMacro(Textm)
  DefMacro(TestStat)
  DefMacro(ExportFont)
  DefMacro(ImportFont)
  DefMacro(ProjSph)
  DefMacro(UpdateQPeakTable)
  DefMacro(Capitalise)
  DefMacro(FlushFS)
  DefMacro(Elevate)
////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FUNCTIONS/////////////////////////////////////////
  DefFunc(FileLast)
  DefFunc(FileSave)
  DefFunc(FileOpen)
  DefFunc(ChooseDir)

  DefFunc(Strcat)
  DefFunc(Strcmp)
  DefFunc(GetEnv)

  DefFunc(Sel)
  DefFunc(Atoms)
  DefFunc(FPS)

  DefFunc(Cursor)
  DefFunc(RGB)
  DefFunc(Color)

  DefFunc(HtmlPanelWidth)
  DefFunc(LoadDll)

  DefFunc(CmdList)
  DefFunc(Alert)

  DefFunc(ValidatePlugin)
  DefFunc(IsPluginInstalled)
  DefFunc(GetUserInput)
  DefFunc(TranslatePhrase)
  DefFunc(IsCurrentLanguage)
  DefFunc(CurrentLanguageEncoding)

  DefFunc(ChooseElement)
  DefFunc(StrDir)
  DefFunc(ChooseFont)
  DefFunc(GetFont)
  DefFunc(ChooseMaterial)
  DefFunc(GetMaterial)
  DefFunc(GetMouseX)
  DefFunc(GetMouseY)
  DefFunc(GetWindowSize)
  DefFunc(IsOS)
  DefFunc(HasGUI)
  DefFunc(CheckState)
  DefFunc(GlTooltip)
  DefFunc(CurrentLanguage)
  DefFunc(GetMAC)
  DefFunc(ThreadCount)
  DefFunc(FullScreen)
  DefFunc(Freeze)
//..............................................................................
public:
  bool IsControl(const olxstr& cname) const;
  //............................................................................

  void OnKeyUp(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnNavigation(wxNavigationKeyEvent& event);

  virtual bool ProcessEvent(wxEvent& evt);
  void OnResize();
  olxstr StylesDir, // styles folder
    ScenesDir, 
    DefStyle,         // default style file
    DefSceneP,        // default scene parameters file
    TutorialDir;
  TGlMaterial
    HelpFontColorCmd, HelpFontColorTxt,
    ExecFontColor, InfoFontColor,
    WarningFontColor, ErrorFontColor, ExceptionFontColor;
private:
  bool Dispatch( int MsgId, short MsgSubId, const IEObject *Sender,
    const IEObject *Data=NULL);
  olxstr FLastSettingsFile;

  class ProcessHandler : public ProcessManager::IProcessHandler  {
    TMainForm& parent;
    bool printed;
  public:
    ProcessHandler(TMainForm& _parent) : parent(_parent), printed(false)  {}
    virtual void BeforePrint();
    virtual void Print(const olxstr& line);
    virtual void AfterPrint();
    virtual void OnWait();
    virtual void OnTerminate(const AProcess& p);
  };
  ProcessHandler _ProcessHandler;
  ProcessManager* _ProcessManager;
  // class TIOExt* FIOExt;
  TTimer *FTimer;
  unsigned short FMode;
  uint64_t TimePerFrame,    // this is evaluated by FXApp->Draw()
           DrawSceneTimer;  // this is set for onTimer to check when the scene has to be drawn

  double FRotationIncrement, FRotationAngle;
  vec3d FRotationVector;
  
  vec3d FFadeVector; // stores: current position, end and increment
  
  olxstr FListenFile;

  TStrPObjList<olxstr,wxMenuItem*> FRecentFiles;
  TSStrStrList<olxstr,true> Bindings;
  uint16_t FRecentFilesToShow;
  void UpdateRecentFile(const olxstr& FN);
  TGlOption FBgColor;
  class TCmdLine* FCmdLine;
  olxstr FHtmlIndexFile;

  bool FHtmlMinimized, FHtmlOnLeft, FBitmapDraw, FHtmlWidthFixed, 
       RunOnceProcessed,
       StartupInitialised;
  bool InfoWindowVisible, HelpWindowVisible, CmdLineVisible, _UseGlTooltip;

  float FHtmlPanelWidth;

  bool UpdateRecentFilesTable(bool TableDef=true);
  void QPeakTable(bool TableDef=true, bool Create=true);
  void BadReflectionsTable(bool TableDef=true, bool Create=true);
  void RefineDataTable(bool TableDef=true, bool Create=true);

  TAccellList<olxstr> AccShortcuts;
  TAccellList<TMenuItem*> AccMenus;

  TSStrPObjList<olxstr,TMenu*, false> Menus;
  int32_t TranslateShortcut(const olxstr& sk);
  void SaveVFS(short persistenceId);
  void LoadVFS(short persistenceId);
  // this must be called at different times on GTK and windows
  void StartupInit();
  bool SkipSizing; // when size changed from the LoadSettings
  void DoUpdateFiles();
  // returns true if the thread is created
  bool CreateUpdateThread();
public:
  TMainForm(TGlXApp *Parent);
  virtual ~TMainForm();
  virtual bool Destroy();
  void LoadSettings(const olxstr &FN);
  void SaveSettings(const olxstr &FN);
  virtual const olxstr& GetScenesFolder() const {  return ScenesDir;  }
  virtual void SetScenesFolder(const olxstr &sf)  {  ScenesDir = sf;  }
  virtual void LoadScene(const TDataItem& Root, TGlLightModel &FLM);
  virtual void SaveScene(TDataItem& Root, const TGlLightModel &FLM) const;
  void UpdateUserOptions(const olxstr &option, const olxstr &value);

  // fires the state change as well
  void UseGlTooltip(bool v);

  const olxstr& GetStructureOlexFolder();
  float GetHtmlPanelWidth() const {  return FHtmlPanelWidth;  }
//..............................................................................
// properties
protected:
  wxToolBar   *ToolBar;
  wxStatusBar *StatusBar;
  wxMenuBar   *MenuBar;
  // file menu
  TMenu *MenuFile;
  // view menu
  wxMenuItem *miHtmlPanel;
  //popup menu
  TMenu      *pmMenu;
    TMenu      *pmDrawStyle,  // submenues
                *pmModel,
                *pmDrawQ;
  TMenu    *pmAtom;
    wxMenuItem *miAtomInfo;
    wxMenuItem *miAtomGrow;
    TMenu    *pmBang;  // bonds angles
    TMenu    *pmAtomType;
    TMenu    *pmAtomOccu, 
             *pmAtomConn,
             *pmAtomPoly,
             *pmAtomPart,
             *pmAtomUiso
             ;
  wxMenuItem *miAtomPartCustom,
    *miAtomUisoCustom,
    *miAtomUisoFree;
  TMenu    *pmBond;
    wxMenuItem *miBondInfo;
    TMenu    *pmTang;  // torsion angles
  TMenu    *pmFragment;
    wxMenuItem *miFragGrow;
  TMenu    *pmSelection;
  TMenu    *pmView;
  TMenu    *pmPlane;
  TMenu    *pmGraphics;  // generic menu for graphics
  TMenu    *pmGrid, *pmBlob;
  TMenu    *pmLabel;
  TMenu    *pmLattice;
  class TXGlLabel* LabelToEdit;
  wxMenu  *FCurrentPopup;

  class FileDropTarget : public wxFileDropTarget {
    TMainForm& parent;
  public:
    FileDropTarget(TMainForm& _parent) : parent(_parent)  {}
    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def)  {
      return wxFileDropTarget::OnDragOver(x,y,wxDragCopy);
    }
  };
public:
  wxMenu* CurrentPopupMenu()    {  return FCurrentPopup; }
  wxMenu* DefaultPopup()        {  return pmGraphics; }
  wxMenu* GeneralPopup()        {  return pmMenu; }
//..............................................................................
// TMainForm interface
  void GlCanvas(TGlCanvas *GC)  {  FGlCanvas = GC;  }
  TGlCanvas * GlCanvas()  {  return FGlCanvas;  }
  void XApp(Olex2App *XA);
  Olex2App *XApp()  {  return FXApp; }
  bool FindXAtoms(const TStrObjList &Cmds, TXAtomPList& xatoms, bool GetAll, bool unselect);
  ConstPtrList<TXAtom> FindXAtoms(const TStrObjList &Cmds,bool GetAll, bool unselect)  {
    TXAtomPList atoms;
    FindXAtoms(Cmds, atoms, GetAll, unselect);
    return atoms;
  }
//..............................................................................
// General interface
//..............................................................................
// actions
  void ObjectUnderMouse(AGDrawObject *G);
//..............................................................................
  DECLARE_CLASS(TMainForm)
  DECLARE_EVENT_TABLE()
};

#endif

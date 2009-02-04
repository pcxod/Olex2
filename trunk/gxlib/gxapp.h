//---------------------------------------------------------------------------//
// namespace TEXLib
// TGXApp - a wraper for basic crystallographic graphic application
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef _xl_gxappH
#define _xl_gxappH
#include "gxbase.h"

#include "xapp.h"
#include "etable.h"

#include "symmlib.h"
#include "undo.h"
#include "bitarray.h"
#include "glrender.h"
#include "xfader.h"
#include "glmouse.h"
#include "dframe.h"
#include "gltext.h"
#include "glfont.h"
#include "glscene.h"
#include "styles.h"

#include "satom.h"
#include "sbond.h"
#include "splane.h"
#include "glbitmap.h"
#include "typelist.h"
#include "hkl.h"
#include "fracmask.h"

#include "wx/zipstrm.h"

#if defined __APPLE__ && defined __MACH__
  #define OLX_RESOURCES_FOLDER "olex2.app/Contents/Resources/"
#else
  #define OLX_RESOURCES_FOLDER ""
#endif



BeginGxlNamespace()

const short simNone = 0x0000, // initialisation method
            simHDC  = 0x0001,
            simBMP  = 0x0002;

const short sdsNone   = 0, // drawing style
            sdsBS     = 1, // bals and sticks
            sdsSP     = 2, // space filling
            sdsES     = 3,  // ellipsoide and sticks
            sdsESR    = 4,  // ellipsoide and rims and sticks
            sdsWF     = 5,  // wire frame
            sdsSS     = 6,  // stippled cones + spheres
            sdsST     = 7;  // sticks

// constatns, identifying lists in saved groups
const short oglAtoms  = 0,
            oglBonds  = 1,
            oglPlanes = 2;
// grow mode
const short gmCovalent      = 0x0001,
            gmSInteractions = 0x0002,
            gmSameAtoms     = 0x0004;
//---------------------------------------------------------------------------
class TDUnitCell;
class TDBasis;
class TGraphicsObjects;
class TXGlLabels;
class TXAtom;
class TXBond;
class TXPlane;
class TXLine;
class TXGrowLine;
class TXGrowPoint;
class TXGlLabel;
class TXReflection;
class TXGrid;
class TXLattice;

  typedef TTypeListExt<TXAtom, IEObject> TXAtomList;
  typedef TTypeListExt<TXBond, IEObject> TXBondList;
  typedef TTypeListExt<TXPlane,IEObject> TXPlaneList;

typedef TPtrList<TXPlane> TXPlanePList;
typedef TPtrList<TXAtom> TXAtomPList;
typedef TPtrList<TXBond> TXBondPList;

class TGXApp : public TXApp, AEventsDispatcher, public ASelectionOwner  {
  TXAtomList XAtoms;
  TXBondList XBonds;
  TXPlaneList XPlanes;
  TTypeListExt<TXGrowPoint, IEObject> XGrowPoints;
  TTypeListExt<TXGrowLine, IEObject> XGrowLines;
  olxstr AtomsToGrow;
  smatd_list UsedTransforms;
  TTypeListExt<TXReflection, IEObject> XReflections;
  TPtrList<TGlBitmap> GlBitmaps;
  TTypeListExt<TXGlLabel, IEObject> XLabels;

  TXGlLabels *FLabels;

  // have to manage memory ourselves - base class is used
  TPtrList<AGDrawObject> LooseObjects;
  TPtrList<AGDrawObject> ObjectsToCreate;

  void ClearXObjects();

  void CreateXRefs();
  void CreateXGrowLines();
  void CreateXGrowPoints();

protected:
  TGlRender *FGlRender;
  TXFader *Fader;
  TTypeList<TXFile> OverlayedXFiles;

  THklFile *FHklFile;

  TGlMouse*   FGlMouse;
  TDUnitCell* FDUnitCell;
  TDBasis* FDBasis;
  TDFrame* FDFrame;

  TXGrid *FXGrid;

  void FragmentVisible( TNetwork *N, bool V);
  bool Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
  void SBonds2XBonds(TSBondPList& L, TXBondPList& Res);
  void SAtoms2XAtoms(TSAtomPList& L, TXAtomPList& Res);
  void SPlanes2XPlanes(TSPlanePList& L, TXPlanePList& Res);
  void GetGPCollections(TPtrList<AGDrawObject>& GDObjects, TPtrList<TGPCollection>& Result);
  // visibility is stored in a bitarray
  TEBitArray FVisibility;
  void RestoreVisibility();
  void StoreVisibility();

  TEList FOldGroups;
  void RestoreGroups();
  void StoreGroups();
  void ClearGroups();

  float FProbFactor;
  double ExtraZoom;  // the default is 1, Calculated Zoom is multiplid by this number
public:
  TGXApp(const olxstr & FileName);
  // FileNAme - argv[0]
  virtual ~TGXApp();
  void CreateObjects(bool SyncBonds, bool CenterModel=true);
  void UpdateBonds();
  void AddObjectToCreate(AGDrawObject* obj)  {  ObjectsToCreate.Add(obj);  }
  void Clear();
  void ClearXGrowPoints();

// drawing data and functions
private:
  double FPictureResolution;
  TStrList IndividualCollections;
public:
  void Quality(const short v);
  void Init();
//..............................................................................
  void ClearIndividualCollections() {  IndividualCollections.Clear();  }
//..............................................................................
// GlRender interface
  void ClearColor(int Color) {  FGlRender->LightModel.ClearColor() = Color; }
  inline int ClearColor()           {  return FGlRender->LightModel.ClearColor().GetRGB(); }
  inline TGlRender& GetRender()     {  return *FGlRender; }
  inline TXFader& GetFader()      {  return *Fader; }
  void InitFadeMode();

  // implementation of BasicApp function - renders the scene
  virtual void Update()  {  Draw();  }

  long Draw();
  void BeginDrawBitmap(double res);
  void FinishDrawBitmap();
  void Resize(int new_w, int new_h)            {  FGlRender->Resize(new_w, new_h); }
  AGDrawObject *SelectObject(int x, int y, int depth=0)  {
    return FGlRender->SelectObject(x, y, depth);
  }
  TGlPrimitive *SelectPrimitive(int x, int y)  {  return FGlRender->SelectPrimitive(x, y); }
  DefPropP(double, ExtraZoom)
//..............................................................................
// TXApp interface
  inline TDUnitCell& DUnitCell() {  return *FDUnitCell; }
  inline TDBasis& DBasis()       {  return *FDBasis; }
  inline THklFile& HklFile()     {  return *FHklFile; }
  inline TDFrame& DFrame()       {  return *FDFrame; }
  inline TXGrid& XGrid()         {  return *FXGrid;  }

  // this function to be used to get all networks, including th overlayed files
  int GetNetworks(TNetPList& nets);
  // overlayed files
  inline int OverlayedXFileCount()  const  {  return OverlayedXFiles.Count();  }
  TXFile& GetOverlayedXFile(int i)  {  return OverlayedXFiles[i];  }
  TXFile& NewOverlayedXFile();
  void DeleteOverlayedXFile(int index);

  void Select(const vec3d& From, const vec3d& To );
  void SelectAll(bool Select)  {
    if( !Select )  BackupSelection();
    GetRender().SelectAll(Select);
    Draw();
  }
  void InvertSelection()                    {  GetRender().InvertSelection(); Draw(); }
  inline TGlGroup* FindObjectGroup(AGDrawObject *G)   {  return GetRender().FindObjectGroup(G); }
  inline TGlGroup* FindGroup(const olxstr& colName) {  return GetRender().FindGroupByName(colName); }
  inline TGlGroup *Selection()              {  return GetRender().Selection(); }
  void GroupSelection(const olxstr& name) {  GetRender().GroupSelection(name);  Draw(); }
  void UnGroupSelection()                   {  GetRender().UnGroupSelection(); Draw(); }
  void UnGroup(TGlGroup *G)                 {  GetRender().UnGroup(G); Draw(); }
  olxstr GetSelectionInfo();
  // ASelection Owner interface
  virtual void ExpandSelection(TCAtomGroup& atoms);

  TGlBitmap* CreateGlBitmap(const olxstr& name,
    int left, int top, int width, int height, unsigned char* RGBa, unsigned int format);
    
  TGlBitmap* FindGlBitmap(const olxstr& name);
  void DeleteGlBitmap(const olxstr& name);
  inline int GlBitmapCount()  const {  return GlBitmaps.Count(); }
  inline TGlBitmap& GlBitmap(int i) {  return *GlBitmaps[i];  }

  bool ShowGrid(bool v, const olxstr& FN=EmptyString);
  bool GridVisible()  const;
  void SetGridDepth(const vec3d& crd);


protected:
  TPtrList<AGDrawObject> SelectionCopy;
  bool FHydrogensVisible,
       FHBondsVisible,
       FQPeaksVisible,
       FQPeakBondsVisible,
       FStructureVisible,
       FHklVisible,
       FXGrowLinesVisible,
       XGrowPointsVisible;
  short FGrowMode, PackMode;
public:
  bool LabelsVisible() const;
  void LabelsVisible(bool v);
  void LabelsMode(short lmode);
  short LabelsMode() const;
  void LabelsFont(short FontIndex);
  TGlMaterial & LabelsMarkMaterial();
  void MarkLabel(TXAtom *A, bool mark);
  void ClearLabelMarks();
  void InitLabels(TXAtomPList* Atoms=NULL);
  int GetNextAvailableLabel(const olxstr& AtomType);

  // moving atom from/to collection
  void Individualise(TXAtom* XA);
  void Collectivise(TXAtom* XA);
  // should not be used externaly
  void ClearLabels();

  void Link(TXAtom *A, TXAtom *B);
  void Free(TXAtom *A, TXAtom *B);
  //
//..............................................................................
// XFile interface
  void RegisterXFileFormat(TBasicCFile *Parser, const olxstr &ext)
  {  FXFile->RegisterFileFormat(Parser, ext); }
  void LoadXFile(const olxstr &fn);
  void SaveXFile(const olxstr &fn, bool Sort)  {  FXFile->SaveToFile(fn, Sort); }
  void Generate( const vec3d &From, const vec3d &To,
    TCAtomPList* Template, bool ClearPrevCont, bool IncludeQ)
  {    FXFile->GetLattice().Generate(From, To, Template, ClearPrevCont, IncludeQ);  }
  void Uniq(bool remEqs=false)  {    FXFile->GetLattice().Uniq(remEqs);  }
  void GrowFragments(bool Shell, TCAtomPList* Template=NULL)
  {    FXFile->GetLattice().GrowFragments(Shell, Template);  }
  void GrowAtoms(const olxstr &Atoms, bool Shell, TCAtomPList* Template=NULL);
  void GrowAtom(TXAtom *XA, bool Shell, TCAtomPList* Template=NULL);
  void Grow(const TXGrowLine& growLine);
  void Grow(const TXGrowPoint& growPoint);
  void ChangeAtomType( TXAtom *A, const olxstr &Element);
  bool AtomExpandable(TXAtom *XA);
  void GrowWhole(TCAtomPList* Template=NULL){  FXFile->GetLattice().GenerateWholeContent(Template); }
  void Grow(const TXAtomPList& atoms, const smatd_list& matrices);

  void MoveFragment(TXAtom* to, TXAtom* fragAtom, bool copy);
  void MoveFragment(const vec3d& to, TXAtom* fragAtom, bool copy);
  void MoveToCenter();
  void Compaq(bool AtomicLevel);
  void HydrogensVisible(bool v);
  bool HydrogensVisible()    {  return FHydrogensVisible;  };
  void HBondsVisible(bool v);
  bool HBondsVisible()       {  return FHBondsVisible;  };
  void QPeaksVisible(bool v);
  bool QPeaksVisible()       {  return FQPeaksVisible;  };
  void QPeakBondsVisible(bool v);
  bool QPeakBondsVisible()   {  return FQPeakBondsVisible;  };
  // hides all bonds for all hidden q-peaks
  void SyncQVisibility();
  void StructureVisible(bool v);
  void HklVisible(bool v);
  bool HklVisible()          {  return FHklVisible;  };
  bool StructureVisible()    {  return FStructureVisible;  };
  void ShowPart(const TIntList& parts, bool show);

  void SetXGrowLinesVisible(bool v);
  bool GetXGrowLinesVisible()   {  return FXGrowLinesVisible;  };
  inline short GetGrowMode()  const {  return FGrowMode;  }
  void SetGrowMode(short v, const olxstr& atoms);
  //
  void SetXGrowPointsVisible(bool v);
  bool GetXGrowPointsVisible()      {  return XGrowPointsVisible;  };
  inline short GetPackMode()  const {  return PackMode;  }
  void SetPackMode(short v, const olxstr& atoms);
  //
  void SwapExyz(TXAtom *XA, const olxstr& Elm);
  void AddExyz(TXAtom *XA, const olxstr& Elm);
protected:
  void XAtomsByMask(const olxstr& Name, int Mask, TXAtomPList& List);
  void CAtomsByMask(const olxstr& Name, int Mask, TCAtomPList& List);
  void XAtomsByType(const TBasicAtomInfo& AI, TXAtomPList& List, bool FindHidden=false);
  void CAtomsByType(const TBasicAtomInfo& AI, TCAtomPList& List);
  void GetSelectedXAtoms(TXAtomPList& List, bool Clear=true);
  void GetSelectedCAtoms(TCAtomPList& List, bool Clear=true);
public:
  TXAtom* GetXAtom(const olxstr& AtomName, bool Clear);
  void GetXAtoms(const olxstr& AtomName, TXAtomPList& res);
  // these two do a command line parsing "sel C1 $N C?? C4 to end"
  void FindCAtoms(const olxstr& Atoms, TCAtomPList& List, bool ClearSelection=true);
  void FindXAtoms(const olxstr& Atoms, TXAtomPList& List, bool ClearSelection=true, 
    bool FindHidden=false);

  TXAtom& GetAtom(int i) {  return XAtoms[i];  }
  const TXAtom& GetAtom(int i) const {  return XAtoms[i];  }
  inline int AtomCount() const {  return XAtoms.Count();  }

  TXBond& GetBond(int i) {  return XBonds[i];  }
  const TXBond& GetBond(int i) const {  return XBonds[i];  }
  inline int BondCount() const {  return XBonds.Count();  }

protected:
  /* the function simply checks if there are any invisible bonds connectd to the
   atom. Normally this happens when a Q-peak is renamed
  */
  void CheckQBonds(TXAtom& Atom);
public:
  void ClearSelectionCopy();
  void RestoreSelection();
protected:
  void BackupSelection();
  // helper functions
  void FillXAtomList( TXAtomPList& res, TXAtomPList* providedAtoms);
  void FillXBondList( TXBondPList& res, TXBondPList* providedBonds);
public:
  TUndoData* Name(const olxstr &From, const olxstr &To, bool CheckLabels, bool ClearSelection);
  TUndoData* Name(TXAtom& Atom, const olxstr &Name, bool CheckLabels);
  TUndoData* ChangeSuffix(const TXAtomPList& xatoms, const olxstr &To, bool CheckLabels);

  void InfoList(const olxstr &Atoms, TStrList &Info);

  void UpdateAtomPrimitives(int Mask, TXAtomPList* Atoms=NULL);
  void UpdateBondPrimitives(int Mask, TXBondPList* Bonds=NULL);

  void SetAtomDrawingStyle(short ADS, TXAtomPList* Atoms=NULL);

  void GetBonds(const olxstr &Bonds, TXBondPList& List);

  void AtomRad(const olxstr &Rad, TXAtomPList* Atoms=NULL); // pers, sfil
  void AtomZoom(float Zoom, TXAtomPList* Atoms=NULL);  // takes %

  void BondRad(float R, TXBondPList* Bonds=NULL);
protected:  float ProbFactor(float Prob);
public:     void CalcProbFactor(float Prob);

  TXPlane *AddPlane(TXAtomPList& Atoms, bool Rectangular, int weightExtent=0);
  TSPlane *TmpPlane(TXAtomPList* Atoms=NULL, int weightExtent=0); 
  void DeletePlane(TXPlane* plane);
  void ClearPlanes();
  TXPlane *XPlane(const olxstr &PlaneName);

  TXLine& AddLine(const olxstr& Name, const vec3d& base, const vec3d& edge);
  TXGlLabel *AddLabel(const olxstr& Name, const vec3d& center, const olxstr &T);
  AGDrawObject* FindLooseObject(const olxstr& Name);

  TXLattice& AddLattice(const olxstr& Name, const mat3d& basis);

  TXAtom *AddCentroid(TXAtomPList& Atoms);
  TXAtom* AddAtom(TXAtom* templ=NULL);
  // adopts atoms of the auinit and returns newly created atoms
  void AdoptAtoms(const TAsymmUnit& au, TXAtomPList& xatom);
  void SelectAtoms(const olxstr &Names, bool Invert=false);
  void SelectAtomsWhere(const olxstr &Where, bool Invert=false);
  void SelectBondsWhere(const olxstr &Where, bool Invert=false);
  /* allows selcting rings: Condition describes the rings to select:
    C5N - content and 1-4, substitutions..
    SelectRing( "C6 1-4") selects all 1,4 substituted benzene rings 
  */
  void SelectRings(const olxstr& Condition, bool Invert=false);
  void FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings );
  
  TXGlLabel* CreateLabel(TXAtom *A, int FontIndex);
  // recreated all labels (if any) in case if font size etc changed
  void UpdateLabels();

//..............................................................................
  void QPeakScale(float V);
  float QPeakScale();
//..............................................................................
// GlMouse interface
  bool MouseDown(int x, int y, short Shift, short Button)  {
    return FGlMouse->MouseDown(x, y, Shift, Button);
  }
  bool MouseUp(int x, int y, short Shift, short Button)  {
    return FGlMouse->MouseUp(x, y, Shift, Button);
  }
  bool MouseMove(int x, int y, short Shift)  {
    return FGlMouse->MouseMove(x, y, Shift);
  }
  bool DblClick()                {  return FGlMouse->DblClick();  }
  void ResetMouseState()         {  FGlMouse->ResetMouseState();  }
  void EnableSelection( bool v)  {  FGlMouse->SelectionEnabled = v;  }
//..............................................................................
// actions
  TActionQueue  *OnGraphicsVisible;
  TActionQueue  *OnFragmentVisible;
  TActionQueue  *OnAllVisible;
  TActionQueue  *OnObjectsDestroy;
  bool IsCellVisible()  const;
  void SetCellVisible( bool v);
  bool IsBasisVisible() const;
  void SetBasisVisible( bool v);
  inline bool IsGraphicsVisible( AGDrawObject *G ) const {  return G->Visible(); }
  TUndoData* SetGraphicsVisible( AGDrawObject *G, bool v );
  TUndoData* SetGraphicsVisible( TPtrList<AGDrawObject>& G, bool v );
  void InvertFragments(const TXAtomPList& NetworkAtoms);
  void MoveFragments(const TXAtomPList& NetworkAtoms, const vec3d& v);
  void TransformFragments(const TXAtomPList& NetworkAtoms, const smatd& m);

  void FragmentsVisible(const TNetPList& Networks, bool V);
  int InvertFragmentsList(const TNetPList& SelectedFragments, TNetPList& Result);

  TGlGroup& GroupFragments(const TNetPList& Fragments, const olxstr groupName);

  // inverts current list of TLattice using Selected Fragments, returns
  // the number of entries added to the result
  void AllVisible(bool V);
  void CenterModel();

  void CenterView(bool calcZoom = false);
  // creates a mask of visible scene
  void BuildSceneMask(FractMask& mask);
//..............................................................................
// X interface
  void BangList(TXAtom *A, TStrList &L);
  void BangTable(TXAtom *A, TTTable<TStrList>& Table);
  float Tang( TSBond *B1, TSBond *B2, TSBond *Middle, olxstr *Sequence=NULL);
  void TangList(TXBond *Middle, TStrList &L);

  TUndoData* DeleteXAtoms(TXAtomPList& L);
  TUndoData* DeleteXObjects(TPtrList<AGDrawObject>& L);
  /* function undoes deleted atoms bonds and planes */
  void undoDelete(TUndoData *data);
  /* function undoes renaming atoms */
  void undoName(TUndoData *data);

  void XAtomDS2XBondDS(const olxstr &Source);  // copies material properties from atoms
  void SynchroniseBonds( TXAtomPList& XAtoms );
  double CalcVolume(const TSStrPObjList<olxstr,double, true> *volumes, olxstr &report);

  void ToDataItem(TDataItem& item, wxOutputStream& zos) const;
  void FromDataItem(TDataItem& item, wxInputStream& zis);

  void SaveModel(const olxstr& file_name) const;
  void LoadModel(const olxstr& file_name);
//..............................................................................
  static TGXApp& GetInstance()  {
    TBasicApp *bai = TBasicApp::GetInstance();
    if( bai == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "unitialised application");
    TGXApp *gxai = dynamic_cast<TGXApp*>(bai);
    if( gxai == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "unsuitabe application instance");
    return *gxai;
  }
};
////////////////////////////////////////////////////////////////////////////////

EndGxlNamespace()
#endif

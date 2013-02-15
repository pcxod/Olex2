#ifndef __olx_xl_lattice_H
#define __olx_xl_lattice_H
#include "xbase.h"
#include "symmat.h"
#include "catom.h"
#include "satom.h"
#include "sbond.h"
#include "network.h"
#include "atomregistry.h"
#include "splane.h"

#include "macroerror.h"
#include "library.h"
#include "bitarray.h"

BeginXlibNamespace()

class TLattice: public IEObject  {
private:
  TNetwork* Network;  // for internal use only
private:
  /* generates matrices in volume {VFrom, VTo} and leaves only matrices, which
  transform the center of gravity of the asymmertic unit within {MFrom, MTo} volume
  usually VFrom = olx_round(MFrom), VTo = olx_round(VFrom)
  */
  size_t GenerateMatrices(const vec3d& VFrom, const vec3d& VTo,
        const vec3d& MFrom, const vec3d& MTo);
  // generates matrices from -4 to 4 which generate aunit within the rad sphere
  size_t GenerateMatrices(smatd_plist& Result, const vec3d& center, double rad);
  smatd_plist Matrices;    // list of all matrices
  TSAtomPList  Atoms;      // list of all atoms
  TSBondPList  Bonds;      // list of all bonds
  TNetPList    Fragments;
  TSPlanePList Planes;
  AtomRegistry atomRegistry;
  TTypeList<TSPlane::Def> PlaneDefs;
  void GenerateBondsAndFragments(TArrayList<vec3d> *crd);
protected:
  TActionQList Actions;
  bool Generated;
  void Generate(TCAtomPList* Template, bool ClearCont);  // generates atoms using current matrices list
  void GenerateAtoms( const TSAtomPList& atoms, TSAtomPList& result, const smatd_plist& matrices);
  void ClearFragments();
  void ClearAtoms();
  void ClearMatrices();
  void ClearBonds();
  void ClearPlanes();

  void AddSBond(TSBond* B);
  void AddSAtom(TSAtom* A);
  void AddSPlane(TSPlane* P);
  void BuildAtomRegistry();

  class TUnitCell*  UnitCell;
  class TAsymmUnit* AsymmUnit;
  double Delta, DeltaI;
  // if Template is specified, the CAtoms are taken from there instead of AsymmUnit
  void DoGrow(const TSAtomPList& Atoms, bool GrowShell, TCAtomPList* Template);
  // fills the list with atoms of asymmertic unit
  // the atoms have to be deleted with a call to delete
  void ListAsymmUnit(TSAtomPList& res, TCAtomPList* Template);
  // removes existing TSAtoms with TCAtom's masked
  void BuildPlanes();
  void InitBody();
  void Disassemble(bool create_planes=true);
  TSAtom& GenerateAtom(TCAtom& a, smatd& symop);
  static void _CreateFrags(TCAtom& start, TCAtomPList& dest);
public:
  TLattice();
  virtual ~TLattice();

  TActionQueue &OnStructureGrow, &OnStructureUniq, &OnDisassemble,
    &OnAtomsDeleted;

  // this is does not have any usefull data - just for functions call!!!
  inline TNetwork& GetNetwork() const {  return *Network; }

  void Clear(bool ClearUnitCell);
  void Uniq(bool removeSymmEquivalents = false);
  // used if atoms availibility etc has changed
  void UpdateConnectivity();
  // used if the connectivity info (CONN/BIND/FREE etc) is changed
  void UpdateConnectivityInfo();
  void UpdatePlaneDefinitions();
  void Init();
  /*adopts atoms, bonds and fragment from the given lattice */
  void AddLatticeContent(const TLattice& latt);
  // generates atoms inside the unit cell only
  void GenerateCell();
  // generates atoms inside the given box of dim[i].Length() size at position center
  void GenerateBox(const mat3d& norms, const vec3d& size, const vec3d& center, bool clear_content);
  // generates atoms within specified volume
  void Generate(const vec3d& MFrom, const vec3d& MTo, TCAtomPList* Template,
    bool ClearCont);
  // generates atoms within sphere volume at center
  void Generate(const vec3d& center, double rad, TCAtomPList* Template,
    bool ClearCont);
  // checks if the data alreade have been generated
  inline bool IsGenerated() const  {  return Generated && (Matrices.Count() > 1);  }

  // generates matrices so that the center of asymmetric unit is inisde the specified volume
  size_t GenerateMatrices(smatd_plist& Result, const vec3d& VFrom, const vec3d& VTo,
        const vec3d& MFrom, const vec3d& MTo);
  // finds matrices to be used for the next grow operation in GrowFragments
  void GetGrowMatrices(smatd_list& res) const;
  /* grows all atoms which can be grown, if GrowShells is true, only immediate environment
  of the atoms which can grow is generated */
  void GrowFragments(bool GrowShells, TCAtomPList* Template);
  /* grows a fragment using particular matrix */
  void GrowFragment(uint32_t FragId, const smatd& transform);
  void GrowAtom(TSAtom& A, bool GrowShells, TCAtomPList* Template);
  void GrowAtoms(const TSAtomPList& Atoms, bool GrowShells, TCAtomPList* Template);
  void GrowAtoms(const TCAtomPList& Atoms, const smatd_list& matrices);
  void GrowAtom(TCAtom& atom, const smatd& matrix);
  // adds new asymmetric unit transformed by the given symop 
  void Grow(const smatd& transform);
   /* generates content using current matrices, the current content stays */
  void GenerateWholeContent(TCAtomPList* Template);

  static int CompareFragmentsBySize(const TNetwork* N, const TNetwork* N1)  {
    return N1->NodeCount() < N->NodeCount() ? -1 : (N1->NodeCount() > N->NodeCount() ? -1 : 0);
  }
  inline size_t FragmentCount() const {  return Fragments.Count(); }
  inline TNetwork& GetFragment(size_t i) const {  return olx_is_valid_index(i) ? *Fragments[i] : *Network;  }
  const TNetPList& GetFragments() const {  return Fragments;  }

  inline size_t MatrixCount() const {  return Matrices.Count();  }
  const smatd& GetMatrix(size_t i) const {  return *Matrices[i];  }
  smatd& GetMatrix(size_t i)  {  return *Matrices[i];  }

  inline size_t AtomCount() const {  return Atoms.Count();  }
  inline TSAtom& GetAtom(size_t i) const {  return *Atoms[i];  }
  const TSAtomPList& GetAtoms() const {  return Atoms;  }
  TSAtom* FindSAtom(const olxstr &Label) const;
  TSAtom* FindSAtom(const TCAtom& ca) const;
  TSAtom* FindSAtom(const TSAtom::Ref& id) const {
    for( size_t i=0; i < Atoms.Count(); i++ )
      if( (*Atoms[i]) == id )
        return Atoms[i];
    return NULL;
  }
  //
  const AtomRegistry& GetAtomRegistry() const {  return atomRegistry;  }
  void RestoreAtom(const TSAtom::FullRef& id);

  inline size_t BondCount() const {  return Bonds.Count();  }
  inline TSBond& GetBond(size_t i) const {  return *Bonds[i];  }
  const TSBondPList& GetBonds() const {  return Bonds;  }

  inline size_t PlaneCount() const {  return Planes.Count(); }
  inline TSPlane& GetPlane(size_t i) const {  return *Planes[i];  }
  const TSPlanePList& GetPlanes() const {  return Planes;  }
  // for the grown structure might return more than one plane
  TSPlanePList NewPlane(const TSAtomPList& Atoms, double weightExtent=0, bool regular=false);
  void ClearPlaneDefinitions()  {  PlaneDefs.Clear();  }
  //the plane must be deleted by the caller !
  TSPlane* TmpPlane(const TSAtomPList& Atoms, double weightExtent=0);
  TSAtom* NewCentroid(const TSAtomPList& Atoms);
  TSAtom* NewAtom(const vec3d& center);

  void SetAnis(const TCAtomPList& atoms, bool anis);

  inline TUnitCell& GetUnitCell() const {  return *UnitCell; }
  inline TAsymmUnit& GetAsymmUnit() const {  return *AsymmUnit; }
  void UpdateAsymmUnit();
  // re-creats unit cell U's and reinitialises atom U's
  void RestoreADPs(bool restoreCoordinates=true);  
  // re-calculates the cartesian coordinates of atoms
  void RestoreCoordinates();
  void MoveFragment(const vec3d& to, TSAtom& fragAtom);
  void MoveFragment(TSAtom& to, TSAtom& fragAtom);
  void MoveToCenter();
  void MoveFragmentG(const vec3d& to, TSAtom& fragAtom);
  void MoveFragmentG(TSAtom& to, TSAtom& fragAtom);
  void MoveToCenterG();
  // assembles fragments around the largest one
  void Compaq();
  // assembles broken fragments
  void CompaqAll();
  // similar to Compaq, but considers atom to atom distances, not fragment centres
  void CompaqClosest();
  // moves atoms of particular type into the positions closest other atoms, does not affect the other atoms
  void CompaqType(short Z);
  // moves Q-peaks into the positions closest to real atoms, does not affect the other atoms
  void CompaqQ()  {  CompaqType(iQPeakZ);  }
  // as above, for H
  void CompaqH()  {  CompaqType(iHydrogenZ);  }
  // transforms fragments using a given smat
  void TransformFragments(const TSAtomPList& fragAtoms, const smatd& transform);

  // beware - only pointers are compared
  inline bool operator == (const TLattice& l) const {  return this == &l;  }
  inline bool operator == (const TLattice* l) const {  return this == l;  }
  inline bool operator != (const TLattice& l) const {  return this != &l;  }
  inline bool operator != (const TLattice* l) const {  return this != l;  }
  struct GrowInfo  {
    smatd_plist matrices;  // the list of all matrices
    TArrayList<TIndexList> info;  // TCAtomId -> matrix;
    size_t unc_matrix_count;
    ~GrowInfo()  {  matrices.DeleteItems();  }
  };
  // takes the ownership of the provided object
  void SetGrowInfo(GrowInfo* grow_info);
  // returns an object created with new
  GrowInfo* GetGrowInfo() const;
protected:
  GrowInfo* _GrowInfo;
  // returns true if the info is valid and applied
  bool ApplyGrowInfo();
  // removes H2O and R3N from the list of potential hydrogen bond forming atoms
  void RemoveNonHBonding(class TAtomEnvi& envi);
//  void AnalyseHBonding(class TAtomEnvi& Envi);
  bool _AnalyseAtomHAdd(class AConstraintGenerator& cg, TSAtom& atom, TSAtomPList& ProcessingAtoms, 
    int part = DefNoPart, TCAtomPList* generated = NULL);
  void _ProcessRingHAdd(AConstraintGenerator& cg, const ElementPList& rcont, const TSAtomPList& atoms);
public:
  // implements HADD command
  void AnalyseHAdd(class AConstraintGenerator& cg, const TSAtomPList& atoms);
  // returns a chemical moiety string for CIF
  olxstr CalcMoiety() const;
  double GetDelta() const {  return Delta;  }
  double GetDeltaI() const {  return DeltaI;  }
  void SetDelta(double v);
  void SetDeltaI(double v);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);

  void LibGetFragmentCount(const TStrObjList& Params, TMacroError& E);
  void LibGetFragmentAtoms(const TStrObjList& Params, TMacroError& E);
  void LibGetMoiety(const TStrObjList& Params, TMacroError& E);
  TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};

EndXlibNamespace()
#endif

#ifndef catomH
#define catomH

#include "xbase.h"
#include "atominfo.h"
#include "symmat.h"
#include "evpoint.h"
#include "typelist.h"
#include "tptrlist.h"

BeginXlibNamespace()

//struct Parameter  {
//  double Value;
//  bool Refinable;
//};

class TEllipsoid;
class TAfixGroup;

class TCAtom: public ACollectionItem  {
private:
  class TAsymmUnit *FParent;
  TBasicAtomInfo*  FAtomInfo;    // a pointer to TBasisAtomInfo object
  olxstr FLabel;    // atom's label
  int     Id, Tag;       // c_atoms id; this is also used to identify if TSAtoms are the same
  int     LoaderId; // id of the atom in the asymmertic unit of the loader
  int     ResiId, SameId, SharedSiteId, EllpId;   // residue and SADI ID
  double  Occp;     // occupancy and its variable
  double  Uiso, QPeak;    // isotropic thermal parameter; use it when Ellipsoid = NULL
  int     FragmentId;   // this is used in asymmetric unit sort and initialised in TLatice::InitBody()
  vec3d Center, Esd;  // fractional coordinates and esds
  evecd FEllpsE;   // errors for the values six values from INS file
  evecd FFixedValues;  // at least five values (x,y,z, Occ, Uiso), may be 10, (x,y,z, Occ, Uij)
  short Part, Degeneracy, 
        Hfix; // hfix is only an of the "interface" use; HFIX istructions are parsed
  bool Deleted, 
    Saved;  // is true the atoms already saved (to work aroung SAME, AFIX)
  bool CanBeGrown,
       HAttached;  // used for the hadd command
  TPtrList<TCAtom>* FAttachedAtoms, *FAttachedAtomsI;
  TAfixGroup* DependentAfixGroup, *ParentAfixGroup;
public:
  TCAtom(TAsymmUnit *Parent);
  virtual ~TCAtom();
  inline TAsymmUnit* GetParent()       const {  return FParent; }
  inline TBasicAtomInfo& GetAtomInfo() const {  return *FAtomInfo; }
  void AtomInfo(TBasicAtomInfo*A);

  inline olxstr& Label()                   {  return FLabel; }
  // function validates and changes the atom type, use the syntax above just to set the label
  bool SetLabel(const olxstr &L);

  inline const olxstr& GetLabel()  const   {  return FLabel;  }

  inline int AttachedAtomCount()       const {
    return FAttachedAtoms == NULL ? 0 : FAttachedAtoms->Count();
  }
  inline TCAtom& GetAttachedAtom(int i)       const {  return *FAttachedAtoms->Item(i);  }
  void AttachAtom(TCAtom *CA);
  inline bool IsAttachedTo(TCAtom& CA) const {
    return FAttachedAtoms == NULL ? false : FAttachedAtoms->IndexOf(&CA) != -1;
  }
  inline void ClearAttachedAtoms()  {
    if( FAttachedAtoms != NULL )  {
      delete FAttachedAtoms;
      FAttachedAtoms = NULL; 
    }
    if( FAttachedAtomsI != NULL )  {
      delete FAttachedAtomsI;
      FAttachedAtomsI = NULL; 
    }
  }

  inline int AttachedAtomICount()      const {
    return FAttachedAtomsI == NULL ? 0 : FAttachedAtomsI->Count();
  }
  inline TCAtom& GetAttachedAtomI(int i)      const {  return *FAttachedAtomsI->Item(i);  }
  void AttachAtomI(TCAtom *CA);
  inline bool IsAttachedToI(TCAtom& CA)const {
    return FAttachedAtomsI == NULL ? false : FAttachedAtomsI->IndexOf(&CA) != -1;
  }
  // beware - just the memory addresses compared!
  inline bool operator == (const TCAtom& ca)  const  {  return this == &ca;  }
  inline bool operator == (const TCAtom* ca)  const  {  return this == ca;  }
  inline bool operator != (const TCAtom& ca)  const  {  return this != &ca;  }
  inline bool operator != (const TCAtom* ca)  const  {  return this != ca;  }

  TAtomsInfo *AtomsInfo() const;
  void  Assign(const TCAtom& S);

  DefPropP(int, Id)
  DefPropP(int, Tag)
  DefPropP(int, LoaderId)
  DefPropP(int, ResiId)
  DefPropP(int, SameId)
  DefPropP(int, EllpId)
  DefPropP(int, FragmentId)
  DefPropP(int, SharedSiteId)
  inline bool IsSiteShared() const {  return SharedSiteId != -1;  }
//  short   Frag;    // fragment index
  DefPropP(short, Degeneracy)
  DefPropP(short, Part)
  int GetAfix() const;
  DefPropP(TAfixGroup*, ParentAfixGroup)
  DefPropP(TAfixGroup*, DependentAfixGroup)
  DefPropP(short, Hfix)
  DefPropP(double, Occp)
  DefPropP(double, Uiso)
  DefPropP(double, QPeak)
  DefPropB(Deleted)
  DefPropB(HAttached)
  DefPropB(Saved)
  // can be grown is set by UnitCell::Init
  DefPropP(bool, CanBeGrown)

  inline double GetUisoVar()           const {  return FFixedValues[4]; }
  inline void  SetUisoVar( double v)         {  FFixedValues[4] = v; };
  inline double GetOccpVar()           const {  return FFixedValues[3]; }
  inline void SetOccpVar( double v)          {  FFixedValues[3] = v; }

  inline evecd& FixedValues()                {  return FFixedValues;  }
  inline const evecd& GetFixedValues() const {  return FFixedValues;  }


  TEllipsoid* GetEllipsoid() const;
  void UpdateEllp( const evecd& Quad);
  void UpdateEllp( const TEllipsoid& NV);
  void AssignEllps(TEllipsoid *NV);

  inline evecd& EllpE()               {  return FEllpsE;  }
  inline vec3d& ccrd()                {  return Center;  }
  inline vec3d const& ccrd()    const {  return Center;  }
  inline vec3d& ccrdEsd()             {  return Esd;  }
  inline vec3d const& ccrdEsd() const {  return Esd;  }

  static int CompareAtomLabels(const olxstr& S, const olxstr& S1);
  static const short CrdFixedValuesOffset,
                     OccpFixedValuesOffset,
                     UisoFixedValuesOffset;
};
//..............................................................................
  typedef TPtrList<TCAtom> TCAtomPList;
  typedef TTypeList<TCAtom> TCAtomList;
//..............................................................................
class TCAtomPComparator  {
public:
  static int Compare(TCAtom* a1, TCAtom* a2)  {
    if( a1->GetFragmentId() != a2->GetFragmentId() )  return a1->GetFragmentId() - a2->GetFragmentId();
    if( a1->GetResiId() != a2->GetResiId() )          return a1->GetResiId() - a2->GetResiId();
    // asc sort by label
    if( a1->GetAtomInfo().GetIndex() == a2->GetAtomInfo().GetIndex() )
      return TCAtom::CompareAtomLabels( a1->GetLabel(), a2->GetLabel());
    // desc sort my mr
    if( (a1->GetAtomInfo().GetIndex() - a2->GetAtomInfo().GetIndex() ) < 0 )  return 1;
    return -1;
  }
};
//..............................................................................
class TCAtomPCenterComparator  {
public:
  static int Compare(TCAtom* a1, TCAtom* a2)  {
    double p = a1->ccrd().QLength() - a2->ccrd().QLength();
    if( p < 0 )  return -1;
    return (p > 0) ? 1 : 0;
  }
};
//..............................................................................
class TGroupCAtom  {
  TCAtom* Atom;
  const smatd* Matrix;
  olxstr Name;
public:
  TGroupCAtom() : Atom(NULL), Matrix(NULL)  {  }
  TGroupCAtom(TCAtom* a, const smatd* m=NULL) : Atom(a), Matrix(m)  { }
  TGroupCAtom(const olxstr& name, TCAtom* a, const smatd* m=NULL) : Name(name), Atom(a), Matrix(m)  { }
  DefPropP(TCAtom*, Atom)
  inline const smatd* GetMatrix() const {  return Matrix;  }
  DefPropC(olxstr, Name)
  olxstr GetFullLabel() const;
};
//....................................................................................
class TAfixGroup  {
  double D, Sof, U;
  int Afix, Id;
  TCAtom* Pivot;
  TCAtomPList Dependent;
public:
  TAfixGroup(int id, TCAtom* pivot, int afix, double d = 0, double sof = 0, double u = 0) :
      Id(id), Pivot(pivot), D(d), Afix(afix), Sof(sof), U(u)  {  
    if( pivot != NULL )  pivot->SetDependentAfixGroup(this);
  }
  TAfixGroup( TAsymmUnit& tau, const TAfixGroup& ag )  {  Assign(tau, ag);  }
  ~TAfixGroup()  {  Clear();  }
  DefPropP(double, D)
  DefPropP(double, Sof)
  DefPropP(double, U)
  DefPropP(int, Id)
  void Assign(TAsymmUnit& tau, const TAfixGroup& ag);
  TCAtom& GetPivot() {  return *Pivot;  }
  void SetPivot(TCAtom& ca)  {  
    Pivot = &ca;
    ca.SetDependentAfixGroup(this);
  }
  const TCAtom& GetPivot() const {  return *Pivot;  }
  int GetM() const {  return GetM(Afix);  }
  int GetN() const {  return GetN(Afix);  }
  bool IsFitted() const {  return IsFitted(Afix);  }
  bool DecAfixForDependent() const {  return DecAfixForDependent(Afix);  }
  static int GetM(int afix) {  return afix < 10 ? 0 : afix/10;  }
  static int GetN(int afix) {  return afix < 10 ? afix : afix%10;  }
  static bool IsFitted(int afix)  {  
    int m = GetM(afix);
    return (m == 5 || m == 6 || m == 7 || m == 10 || m == 11);
  }
  static bool DecAfixForDependent(int afix)  {
    int n = GetN(afix);
    return (n == 6 || n == 9);
  }
  TCAtom& AddDependent(TCAtom& a)  {
    Dependent.Add(&a);
    a.SetParentAfixGroup(this);
    return a;
  }
  int GetAfix() const {  return Afix;  }
  void SetAfix(int a)  {
    if( a == 0 )
      Clear();
  }
  void Clear()  {
    for( int i=0; i < Dependent.Count(); i++ )
      Dependent[i]->SetParentAfixGroup(NULL);
    Dependent.Clear();
    Pivot->SetDependentAfixGroup(NULL);
  }
  int Count() const {  return Dependent.Count();  }
  TCAtom& operator [] (int i) {  return *Dependent[i];  }
  const TCAtom& operator [] (int i) const {  return *Dependent[i];  }
};
//....................................................................................
class TAfixGroups {
  TTypeList<TAfixGroup> Groups;
public:
  TAfixGroup& New(TCAtom* pivot, int Afix, double d = 0, double sof = 0, double u = 0 )  {
    return Groups.Add( new TAfixGroup(Groups.Count(), pivot, Afix, d, sof, u) );
  }
  int Count() const {  return Groups.Count();  }
  TAfixGroup& operator [] (int i) {  return Groups[i];  }
  const TAfixGroup& operator [] (int i) const {  return Groups[i];  }
  void Clear() {  Groups.Clear();  }
  void Delete(int i)  {
    Groups.Delete(i);
    for( int j=i; j < Groups.Count(); j++ )
      Groups[j].SetId(j);
  }
  void Assign( TAsymmUnit& tau, const TAfixGroups& ags )  {
    Clear();
    for( int i=0; i < ags.Count(); i++ )
      Groups.Add( new TAfixGroup(tau, ags[i]) );
  }
};
typedef TTypeList<TGroupCAtom> TCAtomGroup;
//..............................................................................


EndXlibNamespace()
#endif


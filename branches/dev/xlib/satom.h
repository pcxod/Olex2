#ifndef satomH
#define satomH

#include "xbase.h"
#include "elist.h"
#include "atominfo.h"
#include "catom.h"
#include "symmat.h"
#include "typelist.h"
#include "tptrlist.h"
#include "sbond.h"

BeginXlibNamespace()

const short 
  satomDeleted    = 0x0001,
  satomGrown      = 0x0002;

class TSAtom : public TBasicNode<TNetwork, TSAtom, TSBond>  {
private:
  smatd_plist Matrices;
  // a list of pointers to matrices used for generation of atom
  TCAtom*  FCAtom;       // basic crystallographic information
//  int FTag; // override TCollectioItem and TGDrawObject tags
  const class TEllipsoid*  FEllipsoid;   // a pointer to TEllipse object
  vec3d  FCCenter;     // atom center in cell coordinates
  vec3d  FCenter;          // atom center in cartesian coordinates
protected:
  mutable short Flags;
  int _SortNodesByDistance(const TSAtom* a1, const TSAtom* a2)  {
    const double diff = FCenter.DistanceTo(a1->FCenter) - FCenter.DistanceTo(a2->FCenter);
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
public:
  TSAtom(TNetwork *N);
  virtual ~TSAtom();
  void Assign(const TSAtom& S);

  bool IsDeleted()     const {  return (Flags & satomDeleted) != 0;  }
  void SetDeleted(bool v)    {  SetBit(v, Flags, satomDeleted);  }

  bool IsGrown() const;
  inline void SetGrown(bool v)  {  SetBit(v, Flags, satomGrown);  }

  inline operator TCAtom* () const {  return FCAtom;  }

  inline TCAtom& CAtom()     const {  return *FCAtom; }
  void CAtom(TCAtom& CA);

  void AtomInfo(TBasicAtomInfo& AI);
  inline TBasicAtomInfo& GetAtomInfo()  const {  return FCAtom->GetAtomInfo(); }

  inline void SetLabel(const olxstr &L)       { FCAtom->SetLabel(L); }
  inline const olxstr& GetLabel() const       {  return FCAtom->GetLabel(); }
  // returns a label plus (if not identity) first matrix like label_resi.2_556
  olxstr GetGuiLabel() const;
  // returns a label plus (if not identity) first matrix like label_resi(-2/3+X,Y,2-Z)
  olxstr GetGuiLabelEx() const;

  inline int MatrixCount()             const {  return Matrices.Count();  }
  inline const smatd& GetMatrix(int i) const {  return *Matrices[i];  }
  // this also makes sure that the identity releated matrix is coming first in the list
  inline void AddMatrix(smatd* M)            {  
    if( !Matrices.IsEmpty() && M->GetTag() == 0 )
      Matrices.Insert(0, M);
    else
      Matrices.Add(M);  
  }
  inline void AddMatrices(TSAtom *A)         {  Matrices.AddList(A->Matrices); }
  inline void ClearMatrices()                {  Matrices.Clear();  }
  void ChangeType(const olxstr& Type);

  inline const TEllipsoid* GetEllipsoid() const {  return FEllipsoid;  }
  inline void SetEllipsoid(const TEllipsoid* v) {  FEllipsoid = v;  }
  inline vec3d&  ccrd()             {  return FCCenter;  }
  inline vec3d&  crd()              {  return FCenter;  }
  inline vec3d const&  ccrd() const {  return FCCenter;  }
  inline vec3d const&  crd()  const {  return FCenter;  }

  void SortNodesByDistance();
  // allows to trim the number of nodes
  void SetNodeCount(int cnt);
  // removes specified node from the list of nodes
  void RemoveNode(TSAtom& node);

  virtual void ToDataItem(TDataItem& item) const;
  virtual void FromDataItem(const TDataItem& item, class TLattice& parent);
};
  typedef TTypeList<TSAtom> TSAtomList;
  typedef TPtrList<TSAtom> TSAtomPList;

EndXlibNamespace()
#endif


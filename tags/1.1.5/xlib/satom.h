/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_satom_H
#define __olx_xl_satom_H
#include "xbase.h"
#include "catom.h"
#include "symmat.h"
#include "typelist.h"
#include "tptrlist.h"

BeginXlibNamespace()

const unsigned short 
  satom_Deleted    = 0x0001,
  satom_Grown      = 0x0002,
  satom_Standalone = 0x0004,
  satom_Masked     = 0x0008,
  satom_Processed  = 0x0010;  // generic bit

class TSAtom : public TBasicNode<class TNetwork, TSAtom, class TSBond>  {
private:
  smatd_plist Matrices;
  // a list of pointers to matrices used for generation of atom
  TCAtom* FCAtom;       // basic crystallographic information
//  int FTag; // override TCollectioItem and TGDrawObject tags
  class TEllipsoid* FEllipsoid;   // a pointer to TEllipsoid object
  vec3d  FCCenter;     // atom center in cell coordinates
  vec3d  FCenter;          // atom center in cartesian coordinates
protected:
  mutable short Flags;
  int _SortNodesByDistanceAsc(const TSAtom* a1, const TSAtom* a2) const {
    const double diff = FCenter.DistanceTo(a1->FCenter) - FCenter.DistanceTo(a2->FCenter);
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  int _SortNodesByDistanceDsc(const TSAtom* a1, const TSAtom* a2) const {
    const double diff = FCenter.DistanceTo(a2->FCenter) - FCenter.DistanceTo(a1->FCenter);
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  static int _SortBondsByLengthAsc(const TSBond* b1, const TSBond* b2);
  static int _SortBondsByLengthDsc(const TSBond* b1, const TSBond* b2);
public:
  TSAtom(TNetwork* N);
  virtual ~TSAtom()  {}
  void Assign(const TSAtom& S);
  // Is/Set
  virtual bool IsDeleted() const {  return  (Flags&satom_Deleted) != 0;  }
  virtual void SetDeleted(bool v)  {  olx_set_bit(v, Flags, satom_Deleted);  }
  DefPropBFIsSet(Standalone, Flags, satom_Standalone)
  DefPropBFIsSet(Masked, Flags, satom_Masked)
  DefPropBFIsSet(Processed, Flags, satom_Processed)

  bool IsAvailable() const {
    return !(IsDeleted() || IsMasked() || FCAtom->IsDetached());
  }
  bool IsGrown() const {  return NodeCount() == CAtom().AttachedSiteCount();  }
  template <class Accessor=DirectAccessor> struct CAtomAccessor  {
    template <class Item> static inline TCAtom& Access(Item& a)  {
      return Accessor::Access(a).CAtom();
    }
    template <class Item> static inline TCAtom& Access(Item* a)  {
      return Accessor::Access(*a).CAtom();
    }
    template <class Item> inline TCAtom& operator ()(Item& a) const {
      return Accessor::Access(a).CAtom();
    }
    template <class Item> inline TCAtom& operator ()(Item* a) const {
      return Accessor::Access(*a).CAtom();
    }
  };
  TCAtom& CAtom() const {  return *FCAtom;  }
  void CAtom(TCAtom& CA);

  const cm_Element& GetType() const {  return FCAtom->GetType(); }
  const olxstr& GetLabel() const {  return FCAtom->GetLabel(); }
  // returns a label plus (if not identity) first matrix like label_resi.2_556
  olxstr GetGuiLabel() const;
  // returns a label plus (if not identity) first matrix like label_resi(-2/3+X,Y,2-Z)
  olxstr GetGuiLabelEx() const;

  size_t MatrixCount() const {  return Matrices.Count();  }
  const smatd& GetMatrix(size_t i) const {  return *Matrices[i];  }
  /* this also makes sure that the identity matrix or matrix with the smallest ID
  is coming first in the list */
  void AddMatrix(smatd* M) {  
    Matrices.Add(M);  
    if( Matrices.Count() > 1 )  {
      if( M->IsFirst() )
        Matrices.Swap(0, Matrices.Count()-1);
      else if( M->GetId() < Matrices[0]->GetId() ) 
        Matrices.Swap(0, Matrices.Count()-1);
    }
  }
  void AddMatrices(const TSAtom& A)  {  
    const size_t cnt = Matrices.Count();
    Matrices.AddList(A.Matrices); 
    if( A.Matrices[0]->IsFirst() )
      Matrices.Swap(0, cnt);
    else if( A.Matrices[0]->GetId() < Matrices[0]->GetId() )
      Matrices.Swap(0, cnt);
  }
  void ClearMatrices()  {  Matrices.Clear();  }
  bool ContainsMatrix(uint32_t m_id) const {
    for( size_t i=0; i < Matrices.Count(); i++ )
      if( Matrices[i]->GetId() == m_id )
        return true;
    return false;
  }
  static double weight_unit(const TSAtom& a)  {  return 1.0;  }
  static double weight_occu(const TSAtom& a)  {  return a.CAtom().GetChemOccu();  }
  static double weight_z(const TSAtom& a)  {  return a.GetType().z;  }  static double weight_occu_z(const TSAtom& a)  {    return a.CAtom().GetChemOccu()*a.GetType().z;  }
  /* returns true if the atom os generated by the identity transformation
  i.e. - belong to the asymmetric unit */
  bool IsAUAtom() const {  return GetMatrix(0).IsFirst();  }
  // beware that underlying objkect might be shared by several atoms!
  TEllipsoid* GetEllipsoid() const {  return FEllipsoid;  }
  void SetEllipsoid(TEllipsoid* v) {  FEllipsoid = v;  }
  vec3d& ccrd()  {  return FCCenter;  }
  vec3d& crd()  {  return FCenter;  }
  vec3d const& ccrd() const {  return FCCenter;  }
  vec3d const& crd() const {  return FCenter;  }
  // pointers comparison!
  bool operator == (const TSAtom& a) const {  return this == &a;  }

  void SortNodesByDistanceAsc()  {
    Nodes.QuickSorter.SortMF<TSAtom>(Nodes, *this, &TSAtom::_SortNodesByDistanceAsc);
  }
  void SortNodesByDistanceDsc()  {
    Nodes.QuickSorter.SortMF<TSAtom>(Nodes, *this, &TSAtom::_SortNodesByDistanceDsc);
  }
  void SortBondsByLengthAsc()  {
    Bonds.QuickSorter.SortSF(Bonds, &TSAtom::_SortBondsByLengthAsc);
  }
  void SortBondsByLengthDsc()  {
    Bonds.QuickSorter.SortSF(Bonds, &TSAtom::_SortBondsByLengthDsc);
  }
  // allows to trim the number of nodes
  void SetNodeCount(size_t cnt);
  // removes specified node from the list of nodes
  void RemoveNode(TSAtom& node);

  struct Ref  {
    size_t catom_id;
    uint32_t matrix_id;
    Ref(size_t a_id, uint32_t m_id) : catom_id(a_id), matrix_id(m_id) { } 
    Ref(const Ref& r) : catom_id(r.catom_id), matrix_id(r.matrix_id)  { }
    Ref(const TDataItem& item)  {  FromDataItem(item);  }
    Ref& operator = (const Ref& r)  {
      catom_id = r.catom_id;
      matrix_id = r.matrix_id;
      return *this;
    }
    bool operator == (const Ref& r) const {
      return (catom_id == r.catom_id && matrix_id != r.matrix_id);
    }
    bool operator == (const TSAtom& a) const {  return a.operator == (*this);  }
    int Compare(const Ref& r) const {
      const int rv = olx_cmp(catom_id, r.catom_id);
      return rv ==0 ? olx_cmp(matrix_id, r.matrix_id) : rv;
    }
    void ToDataItem(TDataItem& item) const {
      item.AddField("a_id", catom_id).AddField("m_id", matrix_id);
    }
    void FromDataItem(const TDataItem& item)  {
      catom_id = item.GetRequiredField("a_id").ToSizeT();
      matrix_id = item.GetRequiredField("m_id").ToUInt();
    }
  };

  bool operator == (const Ref& id) const {
    return (FCAtom->GetId() == id.catom_id && Matrices[0]->GetId() == id.matrix_id);
  }
  Ref GetRef() const {  return Ref(FCAtom->GetId(), Matrices[0]->GetId());  }

  struct FullRef : public Ref {
    TArrayList<uint32_t>* matrices; 
    FullRef(size_t a_id, const smatd_plist& symm) : Ref(a_id, symm[0]->GetId()),
      matrices(NULL)
    {
      if( symm.Count() > 1 )  {
        matrices = new TArrayList<uint32_t>(symm.Count()-1);
        for( size_t i=1; i < symm.Count(); i++ )
          (*matrices)[i-1] = symm[i]->GetId();
      }
    } 
    FullRef(const FullRef& r) : Ref(r),
      matrices(r.matrices == NULL ? NULL : new TArrayList<uint32_t>(*r.matrices))  {}
    ~FullRef()  {
      if( matrices != NULL )
        delete matrices;
    }
    FullRef& operator = (const FullRef& r)  {
      Ref::operator = (r);
      matrices = (r.matrices == NULL ? NULL : new TArrayList<uint32_t>(*r.matrices));
      return *this;
    }
    bool operator == (const FullRef& r) const {
      if( catom_id == r.catom_id )  {
        if( matrix_id != r.matrix_id )  {
          if( matrices == NULL || r.matrices == NULL )
            return false;
          for( size_t i=0; i < r.matrices->Count(); i++ )
            if( matrix_id == (*r.matrices)[i] )
              return true;
        }
      }
      return false;
    }
  };

  FullRef GetFullRef() const {  return FullRef(FCAtom->GetId(), Matrices);  }

  virtual void ToDataItem(TDataItem& item) const;
  virtual void FromDataItem(const TDataItem& item, class TLattice& parent);
  
  template <class Accessor=DirectAccessor> struct FlagsAnalyser  {
    const short ref_flags;
    FlagsAnalyser(short _ref_flags) : ref_flags(_ref_flags)  {}
    template <class Item> inline bool OnItem(const Item& o) const {
      return (Accessor::Access(o).Flags&ref_flags) != 0;
    }
  };
  template <class Accessor=DirectAccessor> struct TypeAnalyser  {
    const short ref_type;
    TypeAnalyser(const cm_Element _ref_type) : ref_type(_ref_type.z)  {}
    TypeAnalyser(short _ref_type) : ref_type(_ref_type)  {}
    template <class Item> inline bool OnItem(const Item& o) const {
      return Accessor::Access(o).GetType() == ref_type;
    }
  };
};

typedef TTypeList<TSAtom> TSAtomList;
typedef TPtrList<TSAtom> TSAtomPList;
typedef TPtrList<const TSAtom> TSAtomCPList;

EndXlibNamespace()
#endif

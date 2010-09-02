//---------------------------------------------------------------------------
#ifndef srestraintH
#define srestraintH
#include "catom.h"
// covers DFIX, SADI, DANG
BeginXlibNamespace()
// I am really tired of this bshyt
#undef AddAtom
class TAsymmUnit;
class RefinementModel;
class TSRestraintList;
// restraint atom list types
const short rltNone   = 0, //default value for the constructor...
            rltAtoms  = 1, // set of independent atoms
            rltBonds  = 2, // set of "bonds" - atom pairs
            rltAngles = 3, // set of "angles" - atom triples
            rltGroup  = 4; // atoms represent a group

class TSimpleRestraint : public IEObject, public IXVarReferencer  {
  double Value, Esd, Esd1;
  XVarReference* VarRef;
  bool AllNonHAtoms;
  short ListType;
  TCAtomGroup InvolvedAtoms;
  bool AtomsEqual(TCAtom* a1, const smatd* m1, TCAtom* a2, const smatd* m2)  {
    if( a1 == a2 )  {
      if( (m1 == NULL && m2 == NULL) || ((m1 != NULL && m2 != NULL)  &&
        (*m1 == *m2) ) )  {
          return true;
      }
    }
    return false;
  }
  TSRestraintList& Parent;
  size_t Id;
protected:
  void SetId(size_t id)  {  Id = id;  }
public:
  TSimpleRestraint(TSRestraintList& parent, size_t id, const short listType);

  virtual ~TSimpleRestraint()  {  Clear();  }
  void AddAtoms(const TCAtomGroup& atoms);
  void AddAtom(TCAtom& aa, const smatd* ma);
  void AddAtomPair(TCAtom& aa, const smatd* ma,
                   TCAtom& ab, const smatd* mb);

  const TSRestraintList& GetParent() const {  return Parent;  }
  TSRestraintList& GetParent()  {  return Parent;  }

  void Delete();
  void Validate();
  void Clear();

  void OnCAtomCrdChange( TCAtom* ca, const smatd& matr );

  // removes dublicated information depending on the list type
  void Substruct(TSimpleRestraint& sr);

  // copies data from a restraon, but with atoms from the thisAU
  void Assign(const TSimpleRestraint&);
  //const TSimpleRestraint& operator = ( const TSimpleRestraint& );

  size_t AtomCount() const {  return InvolvedAtoms.Count();  }
  TGroupCAtom& GetAtom(size_t i) const {  return InvolvedAtoms[i];  }
  bool ContainsAtom(TCAtom& ca) const;

  short GetListType() const {  return ListType;  }

  size_t GetId() const {  return Id;  }
  DefPropP(double, Value)
  DefPropP(double, Esd)
  DefPropP(double, Esd1)
  DefPropBIsSet(AllNonHAtoms)

  // compares pointer addresses only!
  bool operator == (const TSimpleRestraint& sr) const {  return this == &sr;  }
// IXVarReferencer implementation
  virtual size_t VarCount() const {  return 1;  }
  virtual XVarReference* GetVarRef(size_t var_index) const {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return VarRef;  
  }

  virtual olxstr GetVarName(size_t var_index) const;
  
  virtual void SetVarRef(size_t var_index, XVarReference* var_ref) {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    VarRef = var_ref;  
  }

  virtual IXVarReferencerContainer& GetParentContainer() const;
  
  virtual double GetValue(size_t var_index) const {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return Value;  
  }
  virtual void SetValue(size_t var_index, const double& val) {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    Value = val;  
  }
  virtual bool IsValid() const {  return true;  }
  virtual olxstr GetIdName() const;
//

  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(TDataItem& item);
  friend class TSRestraintList;
};

class TSRestraintList : public IEObject, public IXVarReferencerContainer  {
  TTypeList<TSimpleRestraint> Restraints;
  short RestraintListType;
  RefinementModel& RefMod;
  olxstr IdName;
public:
  TSRestraintList(RefinementModel& rm, const short restraintListType, const olxstr& id_name) : 
      RefMod(rm), IdName(id_name) {
    RestraintListType = restraintListType;  
  }
  virtual ~TSRestraintList()  {}
  TSimpleRestraint& AddNew()  {  return Restraints.Add(new TSimpleRestraint(*this, Restraints.Count(), RestraintListType));  }
  // function checks uniquesness of the restraint data - previously defined values are removed
  void ValidateRestraint( TSimpleRestraint& sr);
  void ValidateAll()  {
    for( size_t i=0; i < Restraints.Count(); i++ )
      Restraints[i].Validate();
  }

  TSimpleRestraint& Release(size_t i);
  void Release(TSimpleRestraint& sr);
  void Restore(TSimpleRestraint& sr);

  const RefinementModel& GetRM() const {  return RefMod;  }
  RefinementModel& GetRM()             {  return RefMod;  }

  void OnCAtomCrdChange( TCAtom* ca, const smatd& matr );
  
  void Assign(const TSRestraintList& rl);
  void Clear();
  size_t Count() const {  return Restraints.Count();  }
  TSimpleRestraint& operator [] (size_t i) const {  return Restraints[i];  }
  short GetRestraintListType() const {  return RestraintListType;  }

// IXVarReferencerContainer implementation
  virtual olxstr GetIdName() const {  return IdName;  }
  virtual size_t GetIdOf(const IXVarReferencer& vr) const {
    if( !EsdlInstanceOf(vr, TSimpleRestraint) )
      throw TInvalidArgumentException(__OlxSourceInfo, "var referencer");
    return ((TSimpleRestraint&)vr).GetId();
  }
  virtual size_t GetPersistentIdOf(const IXVarReferencer& vr) const {
    return GetIdOf(vr);
  }
  virtual IXVarReferencer& GetReferencer(size_t id) const {  return Restraints[id];  }
  virtual size_t ReferencerCount() const {  return Restraints.Count();  }
//
  void ToDataItem(TDataItem& item) const;
  #ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(TDataItem& item);
};

typedef TTypeList<TSimpleRestraint> TSimpleRestraintList;
typedef TPtrList<TSimpleRestraint> TSimpleRestraintPList;

EndXlibNamespace()

#endif

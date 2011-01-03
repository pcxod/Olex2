#ifndef __olx_xlib_atom_registry_H
#define __olx_xlib_atom_registry_H
#include "sbond.h"
#include "arrays.h"

BeginXlibNamespace()

template <class, class> class ObjectCaster;
template <class obj_t> class TIObjectProvider  {
public:
  virtual ~TIObjectProvider()  {}
  virtual size_t Count() const = 0;
  virtual obj_t& New(TNetwork* n) = 0;
  virtual obj_t& Get(size_t i) const = 0;
  inline obj_t& operator [] (size_t i) const {  return Get(i);  }
  obj_t& GetLast() const {  return Get(Count()-1);  }
  virtual void Delete(size_t i) = 0;
  inline void DeleteLast()  {  Delete(Count()-1);  }
  virtual void Clear() = 0;
  virtual void Null(size_t i) = 0;
  virtual void Pack() = 0;
  virtual void IncCapacity(size_t v) = 0;
  inline bool IsEmpty() const {  return Count() == 0;  }
  template <class Functor> const TIObjectProvider& ForEach(const Functor& f) const {
    for( size_t i=0; i < Count(); i++ )
      f.OnItem(Get(i), i);
    return *this;
  }
  template <class act_t> ObjectCaster<obj_t, act_t> GetAccessor()  {
    return ObjectCaster<obj_t, act_t>(*this);
  }
};

template <class obj_t, class act_t> class ObjectCaster : public TIObjectProvider<act_t> {
  TIObjectProvider<obj_t>& list;
protected:
  // dummy function...
  virtual act_t& New(TNetwork* n) {  return *((act_t*)NULL);  }
public:
  ObjectCaster(TIObjectProvider<obj_t>& _list) : list(_list)  {}
  virtual size_t Count() const {  return list.Count();  }
  virtual act_t& Get(size_t i) const {  return (act_t&)list.Get(i);  }
  inline act_t& operator [] (size_t i) const {  return Get(i);  }
  virtual void Delete(size_t i)  {  list.Delete(i);  }
  virtual void Clear()  {  list.Clear();  }
  virtual void Null(size_t i)  {  list.Null(i);  }
  virtual void Pack()  {  list.Pack();  }
  virtual void IncCapacity(size_t v)  {  list.IncCapacity(v);  }
  template <class Functor> const ObjectCaster& ForEach(const Functor& f) const {
    for( size_t i=0; i < Count(); i++ )
      f.OnItem(Get(i), i);
    return *this;
  }
};


template <class obj_t> class TObjectProvider : public TIObjectProvider<obj_t> {
protected:
  TPtrList<obj_t> items;
public:
  virtual size_t Count() const {  return items.Count();  }
  virtual obj_t& New(TNetwork* n)  {  return *items.Add(new obj_t(n));  }
  virtual obj_t& Get(size_t i) const {  return *items[i];  }
  inline obj_t& operator [] (size_t i) const {  return Get(i);  }
  obj_t& GetLast() const {  return *items.GetLast();  }
  virtual void Delete(size_t i)  {
    delete items[i];
    items.Delete(i);
  }
  inline void DeleteLast()  {  Delete(Count()-1);  }
  virtual void Clear()   {  items.DeleteItems(false).Clear();  }
  virtual void Null(size_t i)  {
    delete items[i];
    items.Set(i, NULL);
  }
  virtual void Pack()  {  items.Pack();  }
  virtual void IncCapacity(size_t v)  {  items.SetCapacity(items.Count()+v);  }
  inline bool IsEmpty() const {  return items.IsEmpty();  }
  template <class Functor> const TObjectProvider& ForEach(const Functor& f) const {
    items.ForEach(f);
    return *this;
  }
};

class AtomRegistry  {
  struct DataStruct  {
    TArray3D<TArrayList<TSAtomPList*>*> registry;
    mutable int ref_cnt;
    DataStruct(const vec3i& mind, const vec3i& maxd) : registry(mind, maxd), ref_cnt(1) {} 
    ~DataStruct()  {
      for( size_t i=0; i < registry.Length1(); i++ )  {
        for( size_t j=0; j < registry.Length2(); j++ )  {
          for( size_t k=0; k < registry.Length3(); k++ )  {
            TArrayList<TSAtomPList*>* aum_slice = registry.Data[i][j][k];
            if( aum_slice == NULL )  continue;
            for( size_t l=0; l < aum_slice->Count(); l++ )
              if( (*aum_slice)[l] != NULL )
                delete (*aum_slice)[l];
            delete aum_slice;
          }
        }
      }
    }
  };
protected:
  DataStruct* data;
public:
  typedef TArray3D<TArrayList<TSAtomPList*>*> RegistryType;
  //..................................................................................................
  AtomRegistry() : data(NULL) {}
  //..................................................................................................
  AtomRegistry(const AtomRegistry& r) : data(r.data) {  data->ref_cnt++;  }
  //..................................................................................................
  RegistryType& Init(const vec3i& mind, const vec3i& maxd)  {
    if( data != NULL && --data->ref_cnt == 0 )
      delete data;
    data = new DataStruct(mind, maxd);
    return data->registry;
  }
  //..................................................................................................
  ~AtomRegistry()  {
    if( data != NULL && --data->ref_cnt  == 0 )
      delete data;
  }
  //..................................................................................................
  AtomRegistry& operator = (const AtomRegistry& ar)  {
    if( data != NULL && --data->ref_cnt == 0 )
      delete data;
    data = ar.data;
    return *this;
  }
  //..................................................................................................
  RegistryType& GetRegistry()  {  return data->registry;  }
  //..................................................................................................
  TSAtom* Find(const TSAtom::Ref& ref) const {
    if( data == NULL )  return NULL;
    const vec3i t = smatd::GetT(ref.matrix_id);
    if( !data->registry.IsInRange(t) ) return false;
    TArrayList<TSAtomPList*>* aum_slice = data->registry.Value(t);
    if( aum_slice == NULL )  return NULL;
    TSAtomPList* au_slice = (*aum_slice)[smatd::GetContainerId(ref.matrix_id)];
    if( au_slice == NULL ) return false;
    return (*au_slice)[ref.catom_id];
  }
  //..................................................................................................
  TSBond* Find(const TSBond::Ref& ref) const {
    TSAtom* a = Find(ref.a);
    if( a == NULL )  return NULL;
    for( size_t i=0; i < a->BondCount(); i++ )  {
      if( !a->Bond(i).IsDeleted() && a->Bond(i).Another(*a) == ref.b )
        return &a->Bond(i);
    }
    return NULL;
  }
};

struct ASObjectProvider : public IEObject {
  AtomRegistry atomRegistry;
  TIObjectProvider<TSAtom>& atoms;
  TIObjectProvider<TSBond>& bonds;
  TIObjectProvider<class TSPlane>& planes;
  ASObjectProvider(TIObjectProvider<TSAtom>& _as, TIObjectProvider<TSBond>& _bs, TIObjectProvider<TSPlane>& _ps) :
  atoms(_as), bonds(_bs), planes(_ps)  {}
  //TObjectProvider<TNetwork> fragments;
  virtual IEObject* Replicate() const = 0;

};

struct SObjectProvider : public ASObjectProvider {
  SObjectProvider() : ASObjectProvider(*(new TObjectProvider<TSAtom>),
    *(new TObjectProvider<TSBond>), *(new TObjectProvider<TSPlane>)) {}
  ~SObjectProvider()  {
    atoms.Clear();
    bonds.Clear();
    planes.Clear();
    delete &atoms;
    delete &bonds;
    delete &planes;
  }
  virtual IEObject* Replicate() const {  return new SObjectProvider;  }
};

EndXlibNamespace()
#endif

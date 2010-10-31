// (c) O. Dolomanov, 2004
#ifndef __olx_sdl_sortedTL_H
#define __olx_sdl_sortedTL_H
#include "tptrlist.h"
#ifdef GetObject
  #undef GetObject
#endif

BeginEsdlNamespace()

template <class ComparableClass, class ObjectClass, class Comparator>
struct TSortedListEntry {
  ComparableClass Comparable;
  mutable ObjectClass Object;
  TSortedListEntry(const ComparableClass& c, const ObjectClass& o ) :
  Comparable(c), 
    Object(o) {}
  TSortedListEntry(const TSortedListEntry& entry) :
  Comparable(entry.Comparable),
    Object(entry.Object)  {}
  virtual ~TSortedListEntry()  {}
  inline TSortedListEntry& operator = (const TSortedListEntry& entry)  {
    Comparable = entry.Comparable;
    Object = entry.Object;
    return *this;
  }
  inline int Compare(TSortedListEntry& entry) const {
    return Comparator::Compare(Comparable, entry.Comparable);  
  }
  template <class T> inline int Compare(const T& key ) const {
    return Comparator::Compare(Comparable, key);
  }
};
//---------------------------------------------------------------------------
template <class A, class B, class ComparatorType>
class TSTypeList : public IEObject  {
  // not an ArrayList - inserts are too 'heavy' 
  typedef TSortedListEntry<A,B,ComparatorType> EntryType;
  TPtrList<EntryType> Data;
protected:
  template <class T>
  size_t FindInsertIndex(const T& key, size_t from=InvalidIndex, size_t to=InvalidIndex)  {
    if( from == InvalidIndex ) from = 0;
    if( to == InvalidIndex )   to = Count()-1;
    if( to == from ) return to;
    if( (to-from) == 1 )  return from;
    int resfrom = Data[from]->Compare(key),
      resto   = Data[to]->Compare(key);
    if( resfrom == 0 )  return from;
    if( resto == 0 )    return to;
    if( resfrom < 0 && resto > 0 )  {
      size_t index = (to+from)/2;
      int res = Data[index]->Compare(key);
      if( res < 0 )  return FindInsertIndex(key, index, to);
      if( res > 0 )  return FindInsertIndex(key, from, index);
      if( res == 0 ) return index;
    }
    return InvalidIndex;
  }
  template <class T> size_t FindIndexOf(const T& key) const {
    if( Data.IsEmpty() )  return InvalidIndex;
    if( Data.Count() == 1 )  
      return (!Data[0]->Compare(key)) ? 0 : InvalidIndex;
    size_t from = 0, to = Count()-1;
    if( Data[from]->Compare(key) == 0 )  return from;
    if( Data[to]->Compare(key) == 0 )  return to;
    while( true ) {
      size_t index = (to+from)/2;
      int res = Data[index]->Compare(key);
      if( index == from || index == to)  
        return InvalidIndex;
      if( res < 0 )  from = index;
      else
        if( res > 0 )  to  = index;
        else
          if( res == 0 )  return index;
    }
    return InvalidIndex;
  }
//..............................................................................
public:
  TSTypeList()  {}
//..............................................................................
  /* copy constructor */
  TSTypeList(const TSTypeList& list)  {
    Data.SetCount(list.Count());
    for( size_t i=0; i < list.Data.Count(); i++ )
      Data[i] = new EntryType(*list.Data[i]);
  }
//..............................................................................
  virtual ~TSTypeList()  {  Clear();  }
//..............................................................................
  inline void Clear()  {  Data.DeleteItems().Clear();  }
//..............................................................................
  template <class T> size_t IndexOf(const T& key) const {  return FindIndexOf(key);  }
//..............................................................................
  size_t IndexOfObject(const B& v) const {
    for( size_t i=0; i < Data.Count(); i++ )
      if( Data[i]->Object == v )  
        return i;
    return InvalidIndex;
  }
//..............................................................................
  // retrieves indexes of all entries with same key and returns the number of added entries
  template <class T> size_t GetIndexes(const T& key, TSizeList& il)  {
    if( Data.IsEmpty() )  return 0;
    if( Data.Count() == 1 )  {
      if( Data[0]->Compare(key) != 0 )  return 0;
      il.Add(0);
      return 1;
    }
    const size_t index =  IndexOf(key);
    if( index == InvalidIndex )  return 0;
    il.Add(index);
    size_t i = index+1, addedc = 1;
    // go forward
    while( i < Data.Count() && (Data[i]->Compare(key) == 0) )  {
      il.Add(i);
      i++;
      addedc++;
    }
    // go backwards
    if( index == 0 )  return addedc;
    i = index-1;
    while( i > 0 && (Data[i]->Compare(key) == 0) )  {
      il.Add(i);
      addedc++;
      if( i == 0 )  break;
      i--;
    }
    return addedc;
  }
//..............................................................................
  inline bool IsNull(size_t index) const {  return Data[index] == NULL;  }
//..............................................................................
  inline void NullItem(size_t i)  {
    if( Data[i] != NULL )  {
      delete Data[i];
      Data[i] = NULL;
    }
  }
//..............................................................................
  inline TSTypeList& Pack()  {  Data.Pack();  return *this;  }
//..............................................................................
  template <class PackAnalyser> inline TSTypeList& Pack(const PackAnalyser& pa)  {
    Data.Pack(PackItemActor<PackAnalyser>(pa));
    return *this;
  }
//..............................................................................
  template <class Functor> inline TSTypeList& ForEach(const Functor& f) const {
    Data.ForEach(f);
    return *this;
  }
//..............................................................................
  inline void Delete(size_t i)  {  
    delete Data[i];  
    Data.Delete(i);  
  }
//..............................................................................
  inline size_t Count() const {  return Data.Count(); }
//..............................................................................
  inline bool IsEmpty() const {  return Data.IsEmpty();  }
//..............................................................................
  inline const A& GetKey(size_t index) const {  return Data[index]->Comparable;  }
//..............................................................................
  inline B& GetObject(size_t index) const {  return Data[index]->Object;  }
//..............................................................................
//..............................................................................
  inline const EntryType& GetLast() const {  return *Data.GetLast();  }
//..............................................................................
  inline const A& GetLastKey() const {  return Data.GetLast()->Comparable;  }
//..............................................................................
  inline const B& GetLastObject() const {  return Data.GetLast()->Object;  }
//..............................................................................
  inline TSTypeList& SetCapacity(size_t v)  {  Data.SetCapacity(v);  return *this;  }
//..............................................................................
  inline TSTypeList& SetIncrement(size_t v) {  Data.SetIncrement(v);  return *this;  }
//..............................................................................
  template <class T> inline B& operator [] (const T& key) const {
    const size_t ind = IndexOf(key);
    if( ind != InvalidIndex )
      return Data[ind]->Object;
    throw TFunctionFailedException(__OlxSourceInfo, "no object at specified location" );
  }
//..............................................................................
  const EntryType& Add(const A& key, const B& Object)  {
    EntryType *entry = new EntryType(key, Object);
    if( Data.IsEmpty() )
      Data.Add( entry);
    else if( Data.Count() == 1 )  {
      if(  Data[0]->Compare(*entry) < 0 )  
        Data.Add(entry);
      else                                 
        Data.Insert(0, entry);
    }
    else  {
      if( Data[0]->Compare(*entry) >= 0 ) // smaller than the first
        Data.Insert(0, entry);
      else if( Data.GetLast()->Compare(*entry) <= 0 ) // larger than the last 
        Data.Add(entry);  
      else if( Data.Count() == 2 ) // an easy case then with two items 
        Data.Insert(1, entry);
      else  {
        const size_t pos = FindInsertIndex(key);
        if( pos == InvalidIndex )  {
          delete entry;
          throw TIndexOutOfRangeException(__OlxSourceInfo, pos, 0, Count()-1);
        }
        Data.Insert(pos+1, entry);
      }
    }
    return *entry;
  }
};
//..............................................................................
//..............................................................................
//..............................................................................
  // to be used with objects, having Compare operator
  template <typename ComparableClass, typename ObjectClass>
    class TCSTypeList : public TSTypeList<ComparableClass, ObjectClass, TComparableComparator>  {};
//..............................................................................
//..............................................................................
  // to be used with objects, having >, < operators, or primitive types
  template <typename ComparableClass, typename ObjectClass>
    class TPSTypeList : public TSTypeList<ComparableClass, ObjectClass, TPrimitiveComparator>  {};
//..............................................................................
//..............................................................................
  // string specialisation ... special overriding for [] operator - returns NULL if no
  // specified comparable exist, beware it returns '0' for primitive types
  template <class SC, typename ObjectClass, bool caseinsensitive>
    class TSStrPObjList : public TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >  {
      typedef TSTypeList<SC,ObjectClass, olxstrComparator<caseinsensitive> > PList;
      typedef TSortedListEntry<SC,ObjectClass,olxstrComparator<caseinsensitive> > PListEntry;
    public:
      inline const olxstr& GetString(size_t i) const {  return PList::GetKey(i);  }
      inline const PListEntry& Add(const SC& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
        return PList::Add(s, v);
      }
      template <class T>
      inline ObjectClass operator [] (const T& key) const {
        const size_t ind = PList::IndexOf(key);
        return (ind != InvalidIndex)  ? PList::GetObject(ind) : NULL;
      }
    };
//..............................................................................
//..............................................................................
  // just a string to obj specialisation
  template <class SC, typename ObjectClass, bool caseinsensitive>
    class TSStrObjList : public TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >  {
      typedef TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> > PList;
      typedef TSortedListEntry<SC,ObjectClass,olxstrComparator<caseinsensitive> > PListEntry;
    public:
      inline const SC& GetString(size_t i) const {  return PList::GetKey(i);  }
      inline const PListEntry& Add(const SC& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
        return PList::Add(s, v);
      }
    };
//..............................................................................
//..............................................................................
  // string - string map specialisation ...
  template <class SC, bool caseinsensitive>
    class TSStrStrList : public TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >  {
      typedef TSTypeList<SC, SC, olxstrComparator<caseinsensitive> > PList;
      typedef TSortedListEntry<SC,SC,olxstrComparator<caseinsensitive> > PListEntry;
    public:
      inline const SC& GetString(size_t i) const {  return PList::GetKey(i);  }
      template <class T>
      inline const PListEntry& Add(const T& key, const SC& v = EmptyString )  {
        return PList::Add(key, v);
      }
    };

EndEsdlNamespace()
#endif

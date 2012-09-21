/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef olx_sdl_strlist_H
#define olx_sdl_strlist_H
#include "typelist.h"
#include "estrlist.h"
#include "talist.h"
#include "egc.h"
#include "tptrlist.h"
#include "datastream.h"
#undef GetObject

BeginEsdlNamespace()

template <typename> class ConstStrList;
template <typename> class ConstStrObjList;
class TEFile;

template <class SC> struct TSingleStringWrapper  {
  SC String;
  TSingleStringWrapper() {}
  template <class T>
  TSingleStringWrapper(const T& str) : String(str)  {}
  typedef SC string_type;
};

template <class T, bool CaseInsensetive> class TStringWrapperComparator  {
public:
  static int Compare(const T &A, const T &B)  {
   return (CaseInsensetive) ? A.String.Comparei(B.String)
     : A.String.Compare(B.String);
  }
};
// string class, string container class
template <class T> class TTStrList : public IEObject {
public:
  typedef typename T::string_type string_type;
protected:
  TPtrList<T> Strings;
  template <class StrClass>
  size_t FindIndexOf(const StrClass& Str, bool CI) const {
    if( CI )  {
      for( size_t i=0; i < Count(); i++ )
        if( Strings[i]->String.Equalsi(Str) )
          return i;
    }
    else  {
      for( size_t i=0; i < Count(); i++ )
        if( Strings[i]->String.Equals(Str) )
          return i;
    }
    return InvalidIndex;
  }
public:
  TTStrList()  {}

  template <class T1> TTStrList(const TTStrList<T1>& list)  {
    Strings.SetCapacity(list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i]);
  }
  template <class T1> TTStrList(const ConstStrObjList<T1>& list)  {
    Strings.SetCapacity(list.Count());
    for (size_t i=0; i < list.Count(); i++)
      Add(list[i]);
  }
  TTStrList(const TTStrList& list)  {
    Strings.SetCapacity(list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i]);
  }
  TTStrList(const ConstStrList<T> &list)  {
    TakeOver(list.Release(), true);
  }
  TTStrList(size_t count)  {
    Strings.SetCapacity( count );
    for( size_t i=0; i < count; i++ )
      Add(EmptyString());
  }
  // creates a list with strtok entries in it
  TTStrList(const string_type& string, const string_type& sep)  {
    Strtok(string, sep);
  }

  TTStrList(const string_type& string, olxch sep, bool skip_sequences=true)  {
    Strtok(string, sep, skip_sequences);
  }
  
  virtual ~TTStrList()  {  Clear();  }

  TTStrList &TakeOver(TTStrList &d, bool do_delete=false)  {
    Strings.TakeOver(d.Strings);
    if( do_delete )  delete &d;
    return *this;
  }

  TTStrList& SetCount(size_t nc)  {
    if( nc < Strings.Count() )  {
      for( size_t i=nc; i < Strings.Count(); i++ )
        delete Strings[i];
      Strings.SetCount(nc);
    }
    else if (nc > Strings.Count()) {
      Strings.SetCapacity(nc);
      while( Strings.Count() < nc )
        Add(EmptyString());
    }
    return *this;
  }

  void SetCapacity(size_t cap)   {  Strings.SetCapacity(cap);  }
  string_type& operator [] (size_t i) const {
    return Strings[i]->String;
  }
  string_type& GetString(size_t i) const {
    return Strings[i]->String;
  }
  T& GetLast() const {  return *Strings.GetLast();   }
  string_type& GetLastString() const {  return Strings.GetLast()->String;   }
  size_t Count() const {  return Strings.Count();  }
  bool IsEmpty() const {  return Strings.IsEmpty();  }
  string_type& Add()  {  return Strings.Add(new T)->String;  }
  string_type& Add(const string_type& str)  {
    return Strings.Add(new T(str))->String;
  }
  string_type& Add(const char* str)  {
    return Strings.Add(new T(str))->String;
  }
  string_type& Add(const wchar_t* str)  {
    return Strings.Add(new T(str))->String;
  }

  TTStrList& operator << (const string_type& str)  {
    Strings.Add(new T(str));
    return *this;
  }
  TTStrList& operator << (const char* str)  {
    Strings.Add(new T(str));
    return *this;
  }
  TTStrList& operator << (const wchar_t* str)  {
    Strings.Add(new T(str));
    return *this;
  }
  template <class T1> 
  TTStrList& operator << (const TTStrList<T1>& list)  {
    Strings.SetCapacity(Count() + list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i]);
    return *this;
  }
  TTStrList& operator << (const ConstStrList<T>& list)  {
    return *this << list.GetObject();
  }

  string_type& Insert(size_t i)  {  return Strings.Insert(i, new T)->String;  }
  string_type& Insert(size_t i, const string_type& S)  {
    return Strings.Insert(i, new T(S))->String;
  }
  string_type& Insert(size_t i, const char* S)  {
    return Strings.Insert(i, new T(S))->String;
  }
  string_type& Insert(size_t i, const wchar_t* S)  {
    return Strings.Insert(i, new T(S))->String;
  }
  template <class T1> 
  void Insert(size_t i, const TTStrList<T1>& list)  {
    if( list.IsEmpty() )  return;
    Strings.Insert(i, list.Count() );
    for( size_t j=0; j < list.Count(); j++ )
      Strings[i+j] = new T(list[j]);
  }
  void Swap(size_t i, size_t j)  {  Strings.Swap(i, j);  }
  void Move(size_t from, size_t to)  {  Strings.Move(from, to);  }
  void Rearrange(const TSizeList& indexes)  {  Strings.Rearrange(indexes);  }
  void Clear()  {
    for( size_t i=0; i < Strings.Count(); i++ )
      delete Strings[i];
    Strings.Clear();
  }
  void Delete(size_t i)  {
    delete Strings[i];
    Strings.Delete(i);
  }
  void DeleteRange(size_t from, size_t count)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from, 0, Strings.Count());
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from+count, 0, Strings.Count()+1);
#endif
    for( size_t i=0; i < count; i++ )
      delete Strings[from+i];
    Strings.DeleteRange(from, count);
  }

  TTStrList& SubList(size_t from, size_t count, TTStrList& SL) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from, 0, Strings.Count()+1);
    TIndexOutOfRangeException::ValidateRange(
      __POlxSourceInfo, from+count, 0, Strings.Count()+1);
#endif
    SL.Strings.SetCapacity(SL.Count()+count);
    for( size_t i=0; i < count; i++ )
      SL.Add(GetString(i+from));
    return SL;
  }

  ConstStrList<T> SubListFrom(size_t offset) const {
    TTStrList SL;
    return SubList(offset, Strings.Count()-offset, SL);
  }

  ConstStrList<T> SubListTo(size_t to) const {
    TTStrList SL;
    return SubList(0, to, SL);
  }

  template <class T1>
  TTStrList& Assign(const TTStrList<T1>& S)  {
      Clear();
      for( size_t i=0; i < S.Count(); i++ )
        Add(S.GetString(i));
      return *this;
    }

  TTStrList& AddList(const TTStrList& S)  {
    for( size_t i=0; i < S.Count(); i++ )
      Add(S.GetString(i));
    return *this;
  }

  TTStrList& TrimWhiteCharStrings(bool leading=true, bool trailing=true)  {
    if( IsEmpty() )  return *this;
    size_t start = 0, end = Count()-1;
    if( leading )  {
      while( start < end && GetString(start).IsWhiteCharString() ) start++;
      if( start >= Count() )  {  Clear();  return *this;  }
      for( size_t i=0; i < start; i++ )  {
        delete Strings[i];
        Strings[i] = NULL;
      }
    }
    if( trailing )  {
      while( end > start && GetString(end).IsWhiteCharString() )  end--;
      for( size_t i=end+1; i < Count(); i++ )  {
        delete Strings[i];
        Strings[i] = NULL;
      }
    }
    if( start == 0 && end == Count()-1 )  return *this;
    Strings.Pack();
    return *this;
  }

  TTStrList& CombineLines(const string_type& LineContinuationDel)  {
    for( size_t i=0; i < Count(); i++ )  {
      if( GetString(i).EndsWith(LineContinuationDel) )  {
        GetString(i).Delete(
          GetString(i).Length()-LineContinuationDel.Length(),
          LineContinuationDel.Length());
        if( (i+1) < Count() )  {
          GetString(i) << GetString(i+1);
          Delete(i+1);
          i--;
          continue;
        }
      }
    }
    return *this;
  }
  // this removes empty strings from the list
  TTStrList& Pack()  {
    for( size_t i=0; i < Count(); i++ )  {
      if( Strings[i]->String.IsEmpty() )  {
        delete Strings[i];
        Strings[i] = NULL;
      }
    }
    Strings.Pack();
    return *this;
  }

  template <class StrClass>
  size_t IndexOf(const StrClass& C) const {
    return FindIndexOf(C, false);
  }

  template <class StrClass>
  size_t IndexOfi(const StrClass& C) const {
    return FindIndexOf(C, true);
  }
  
  template <class StrClass>
  size_t FindIndexes(const StrClass& C, TSizeList& rv, bool CI) const {
    size_t cc = rv.Count();
    for( size_t i=0; i < Count(); i++ )
      if( Strings[i]->String.Compare(C, CI) == 0 )
        rv.Add(i);
    return rv.Count() - cc;
  }
  string_type Text(const string_type& Sep, size_t start=InvalidIndex,
    size_t end=InvalidIndex) const
  {
    if( start == InvalidIndex )  start = 0;
    if( end == InvalidIndex )    end = Strings.Count();
    size_t tc=1, slen = Sep.Length();
    for( size_t i=start; i < end; i++ )  {
      tc += Strings[i]->String.Length();
      tc += slen;
    }
    string_type E(EmptyString(), tc);
    for( size_t i=start; i < end; i++ )  {
      E << Strings[i]->String;
      if( i < end-1 ) // skip for the last line
        E << Sep;
    }
    return E;
  }
  // convenience methods
  TTStrList& LoadFromTextArray(char *bf, size_t bf_sz, bool take_ownership)  {
    Clear();
    const olxcstr str = take_ownership
      ? olxcstr(olxcstr::FromExternal(bf, bf_sz))
      : olxcstr((const char*)bf, bf_sz);
    // must preserve the new lines on Linux!!! 2008.08.17
    Strtok(str, '\n', false);
    for( size_t i=0; i < Count(); i++ )
      if( GetString(i).EndsWith('\r') )  
        GetString(i).SetLength(GetString(i).Length()-1);
    return *this;
  }
  TTStrList& LoadFromTextStream(IInputStream& io)  {
    Clear();
    size_t fl = io.GetAvailableSizeT();
    if( fl == 0 )  return *this;
    char *bf = olx_malloc<char>(fl+1);
    io.Read(bf, fl);
    return LoadFromTextArray(bf, fl, true);
  }
  TTStrList& LoadFromFile(const olxstr& fileName)  {
    TEFile file(fileName, "rb");
    return LoadFromTextStream(file);
  }
  void operator >> (IDataOutputStream &Stream) const {
    Stream << (uint32_t)Count();
    for( size_t i=0; i < Count(); i++ )
      Stream << Strings[i]->String;
  }
  TTStrList& operator << (IDataInputStream &Stream)  {
    Clear();
    uint32_t size;
    Stream >> size;
    Strings.SetCapacity(size);
    for( uint32_t i=0; i < size; i++ )
      Stream >> Add();
    return *this;
  }
  // convinience method
  const TTStrList& SaveToTextStream(IDataOutputStream& os) const {
    if( IsEmpty() )  return *this;
    for( size_t i=0; i < Count(); i++ )
      os.Writeln(Strings[i]->String);
    return *this;
  }
  const TTStrList& SaveToFile(const olxstr& fileName) const {
    TEFile file(fileName, "wb+");
    return SaveToTextStream(file);
  }

  virtual TIString ToString() const {
    return Text(NewLineSequence()).ToString();
  }

  TTStrList& operator = (const TTStrList& list)  {  return Assign(list);  }
  TTStrList &operator = (const ConstStrList<T> &list)  {
    return TakeOver(list.Release(), true);
  }

  void QSort(bool ci)  {
    if( ci )
      QuickSorter::Sort(Strings, TStringWrapperComparator<T,true>());
    else 
      QuickSorter::Sort(Strings, TStringWrapperComparator<T,false>());
  }

  size_t StrtokF(const string_type& Str, const TSizeList& indexes)  {
    if( indexes.IsEmpty() )  return 0;
    size_t fLength = 0, cnt = 0;
    for( size_t i=0; i < indexes.Count(); i++ )  {
      if( indexes[i] > (Str.Length()-fLength) )
        break;
      Add(Str.SubString(fLength, indexes[i]));
      cnt++;
      fLength += indexes[i];
      if( fLength >= Str.Length() )  return cnt;
    }
    if( fLength < Str.Length() )  {
      Add(Str.SubStringFrom(fLength));
      cnt++;
    }
    return cnt;
  }

  TTStrList& Strtok(const string_type& Str, olxch Sep,
    bool SkipSequences=true)
  {
    string_type Tmp(Str);
    size_t ind = Tmp.IndexOf(Sep);
    while( ind != InvalidIndex )  {
      if( ind != 0 )  // skip sequences of separators
        Add(Tmp.SubStringTo(ind));
      else if( !SkipSequences )
        Add();
      if( ind+1 >= Tmp.Length() )  {
        Tmp.SetLength(0);
        break;
      }
      while( ++ind < Tmp.Length() && Tmp.CharAt(ind) == Sep ) 
        if( !SkipSequences )
          Add(EmptyString());
      if( ind >= Tmp.Length() )  {
        Tmp.SetLength(0);
        break;
      }
      Tmp = Tmp.SubStringFrom(ind);
      ind = Tmp.IndexOf(Sep);
    }
    if( !Tmp.IsEmpty() ) // add last bit
      Add(Tmp);
    return *this;
  }
  // similar to previous implementation, takes string as a separator
  TTStrList& Strtok(const string_type& Str, const string_type& Sep)  {
    string_type Tmp(Str);
    const size_t sepl = Sep.Length();
    size_t ind = Tmp.IndexOf(Sep);
    while( ind != InvalidIndex )  {
      Add(Tmp.SubStringTo(ind));
      if( (ind+sepl) >= Tmp.Length() )  {
        Tmp.SetLength(0);
        break;
      }
      Tmp = Tmp.SubStringFrom(ind+Sep.Length());
      ind = Tmp.IndexOf(Sep);
    }
    if( !Tmp.IsEmpty() ) // add last bit
      Add(Tmp);
    return *this;
  }

  TTStrList& Hyphenate(const string_type& String, size_t Width,
    bool Force=true)
  {
    if( Width == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "width");
    string_type Str(String);
    while( Str.Length() > Width )  {
      size_t spi = Str.LastIndexOf(' ', Width);
      if( spi != InvalidIndex && spi > 0 )  {
        Add( Str.SubStringTo(spi) );
        Str.Delete(0, spi+1); // remove the space
      }
      else  {
        if( Force )  {
          Add( Str.SubStringTo(Width) );
          Str.Delete(0, Width); // remove the space
        }
      }
    }
    if( !Str.IsEmpty() )  
      Add(Str);
    return *this;
  }

  template <class Functor>
  const TTStrList& ForEach(const Functor& f) const {
    for( size_t i=0; i < Strings.Count(); i++ )
      f.OnItem(*Strings[i], i);
    return *this;
  }

  template <class Functor>
  const TTStrList& ForEachString(const Functor& f) const {
    for( size_t i=0; i < Strings.Count(); i++ )
      f.OnItem(Strings[i]->String, i);
    return *this;
  }
public:
  struct InternalAccessor : public TPtrList<T>::InternalAccessor {
    InternalAccessor(TTStrList &l)
      : TPtrList<T>::InternalAccessor(l.Strings)
    {}
  };
  typedef typename T::string_type list_item_type;
};


template <class SC, typename OC> 
struct TPrimitiveStrListData : public TSingleStringWrapper<SC>  {
  OC Object;
  TPrimitiveStrListData() : Object(0) {}
  template <class T>
  TPrimitiveStrListData(const T& str, const OC& obj = 0)
    : TSingleStringWrapper<SC>(str), Object(obj) {}
  typedef OC object_type;
};

template <class SC, typename OC> struct TObjectStrListData
  : public TSingleStringWrapper<SC>
{
  OC Object;
  TObjectStrListData()  {}
  template <class S>
  TObjectStrListData(const S& str) : TSingleStringWrapper<SC>(str)  {}
  template <class S>
  TObjectStrListData(const S& str, const OC& obj)
    : TSingleStringWrapper<SC>(str), Object(obj)  {}
  typedef OC object_type;
};

template <class GC> class TTOStringList
  : public TTStrList<GC>
{
  typedef TTStrList<GC> PList;
  typedef typename GC::string_type string_type;
  typedef typename GC::object_type object_type;
public:
  // creates empty list
  TTOStringList()  {}
  TTOStringList(size_t count) : TTStrList<GC>(count)  {}
  // copy constructor
  template <class T1>
  TTOStringList(const TTStrList<T1>& list)  {
    PList::Strings.SetCapacity(list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i]);
  }

  TTOStringList(const TTOStringList& list)  {
    PList::Strings.SetCapacity(list.Count());
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i], list.GetObject(i));
  }
  TTOStringList(const ConstStrObjList<GC> &list)  {
    PList::Strings.TakeOver(list.Relase(), true);
  }
  // creates a list with strtok entries in it
  TTOStringList(const string_type& string, const string_type& sep,
    TTypeList<object_type>* objects=NULL)
    : TTStrList<GC>(string, sep)
  {
    if( objects != NULL )  {
      for( size_t i=0; i < objects->Count(); i++ )  {
        if( (i+1) >= PList::Count() )  break;
        GetObject(i) = objects->GetItem(i);
      }
    }
  }
  // creates a list with strtok entries in it
  TTOStringList(const PList& strings, char sep, 
    TTypeList<object_type>* objects = NULL)
    : TTStrList<GC>(strings, sep)
  {
    if( objects != NULL )  {
      for( size_t i=0; i < objects->Count(); i++ )  {
        if( (i-1) > PList::Count() )  break;
        GetObject(i) = objects->GetItem(i);
      }
    }
  }
  virtual ~TTOStringList()  {}

  TTOStringList& SubList(size_t offset, size_t count, TTOStringList& SL) const
  {
    for( size_t i=offset; i < offset+count; i++ )
      SL.Add(PList::GetString(i), GetObject(i));
    return SL;
  }

  ConstStrObjList<GC> SubListFrom(size_t offset) const {
    TTOStringList SL;
    return SubList(offset, PList::Count()-offset, SL);
  }

  ConstStrObjList<GC> SubListTo(size_t to) const {
    TTOStringList SL;
    return SubList(0, to, SL);
  }

  template <class T1>
  TTOStringList& Assign(const TTStrList<T1>& S)  {
    PList::Clear();
    PList::SetCapacity(S.Count());
    for( size_t i=0; i < S.Count(); i++ )
      Add(S[i]);
    return *this;
  }

  TTOStringList& Assign(const TTOStringList& S)  {
    PList::Clear();
    PList::SetCapacity(S.Count());
    for( size_t i=0; i < S.Count(); i++ )
      Add(S[i], S.GetObject(i));
    return *this;
  }

  TTOStringList AddList(const TTOStringList& S)  {
    for( size_t i=0; i < S.Count(); i++ )
      Add(S[i], S.GetObject(i));
    return *this;
  }

  GC& Add()  {  return *PList::Strings.Add(new GC);  }
  GC& Add(const string_type& S)  {  return *PList::Strings.Add(new GC(S));  }
  GC& Add(const string_type& S, const object_type& Object)  {
    return *PList::Strings.Add(new GC(S,Object));
  }
  GC& Insert(size_t i, const string_type& S, const object_type& O)  {
    return *PList::Strings.Insert(i, new GC(S,O));
  }
  GC& Set(size_t i, const string_type& S, const object_type& O)  {  
    delete PList::Strings[i];
    return *(PList::Strings[i] = new GC(S,O));
  }

  object_type& GetObject(size_t i) const { return PList::Strings[i]->Object;  }

  TTOStringList& operator = (const TTOStringList& list)  {
    return Assign(list);
  }

  TTOStringList *operator = (const ConstStrObjList<GC> &list)  {
    PList::Strings.TakeOver(list.Relase(), true);
    return *this;
  }

  size_t IndexOfObject(const object_type& C) const {
    for( size_t i=0; i < PList::Count(); i++ )
      if( PList::Strings[i]->Object == C )
        return i;
    return InvalidIndex;
  }
  // the find function with this signature work only for objects;
  // for pointers it causes a lot of trouble
  template <class StrClass>
  const object_type& FindObject(const StrClass& Name) const {
    size_t in = PList::IndexOf(Name);
    return (in != InvalidIndex) ? PList::Strings[in]->Object
      : *(object_type*)NULL;
  }

  template <class StrClass>
  object_type* FindObjecti(const StrClass& Name) const {
    size_t in = PList::IndexOfi(Name);
    return (in != InvalidIndex) ? &PList::Strings[in]->Object
      : (object_type*)NULL;
  }
};

template <class SC, typename OC> class TStrPObjList:
   public TTOStringList<TPrimitiveStrListData<SC,OC> >
{
  typedef TPrimitiveStrListData<SC,OC> data_t;
  typedef TTOStringList<data_t> PList;
public:
  TStrPObjList()  {}
  TStrPObjList(size_t count) : PList(count)  {}
  TStrPObjList(const ConstStrObjList<data_t> &list) {
    PList::TakeOver(list.Release(), true);
  }

  template <class T1> TStrPObjList(const TTStrList<T1>& list)
    : PList(list) {}

  TStrPObjList(const TTOStringList<data_t>& list)
    : PList(list)  {}

  TStrPObjList(const SC& string, const SC& sep, TTypeList<OC>* objects = NULL)
    : PList(string, sep, objects)  {}

  TStrPObjList(const SC& string, char sep, TTypeList<OC>* objects = NULL)
    : PList(string, sep, objects)  {}

  template <typename StrClass> OC FindObject(const StrClass& Name) const {
    const size_t in = PList::IndexOf(Name);
    return (in != InvalidIndex) ? PList::Strings[in]->Object : NULL;
  }

  template <typename StrClass> OC FindObjecti(const StrClass& Name) const {
    const size_t in = PList::IndexOfi(Name);
    return (in != InvalidIndex) ? PList::Strings[in]->Object : NULL;
  }

  TStrPObjList &operator = (const ConstStrObjList<data_t> &list) {
    PList::Strings.TakeOver(list.Relase(), true);
    return *this;
  }

  TStrPObjList& SubList(size_t offset, size_t count, TStrPObjList& SL) const
  {
    for( size_t i=offset; i < offset+count; i++ )
      SL.Add(PList::GetString(i), PList::GetObject(i));
    return SL;
  }

  ConstStrObjList<data_t> SubListFrom(size_t offset) const {
    TStrPObjList SL;
    return SubList(offset, PList::Count()-offset, SL);
  }

  ConstStrObjList<data_t> SubListTo(size_t to) const {
    TStrPObjList SL;
    return SubList(0, to, SL);
  }

};

// const_strlist
template <typename item_t>
class ConstStrList : public const_list<TTStrList<item_t> >
{
  typedef TTStrList<item_t> list_t;
  typedef const_list<list_t> parent_t;
public:
  typedef typename list_t::list_item_type list_item_type;

  ConstStrList(const ConstStrList &d) : parent_t(d) {}
  ConstStrList(list_t &d) : parent_t(d) {}
  ConstStrList(list_t *d) : parent_t(d) {}
  ConstStrList &operator = (const ConstStrList &d) {
    parent_t::operator = (d);
    return *this;
  }
  list_item_type Text(const list_item_type& Sep,
    size_t start=InvalidIndex, size_t end=InvalidIndex) const
  {
    return parent_t::GetObject().Text(Sep, start, end);
  }
};

// const_strobjlist
template <typename item_t>
class ConstStrObjList : public const_list<TTOStringList<item_t> > {
  typedef TTOStringList<item_t> list_t;
  typedef const_list<list_t> parent_t;
public:
  typedef typename list_t::list_item_type list_item_type;

  ConstStrObjList(const ConstStrObjList &d) : parent_t(d) {}
  ConstStrObjList(list_t &d) : parent_t(d) {}
  ConstStrObjList(list_t *d) : parent_t(d) {}
  ConstStrObjList &operator = (const ConstStrObjList &d) {
    parent_t::operator = (d);
    return *this;
  }
  typename item_t::object_type& GetObject(size_t i) const {
    return parent_t::GetObject().GetObject(i);
  }
  const list_t & GetObject() const { return parent_t::GetObject(); }
  list_item_type Text(const list_item_type& Sep,
    size_t start=InvalidIndex, size_t end=InvalidIndex) const
  {
    return parent_t::GetObject().Text(Sep, start, end);
  }
};

typedef TStrPObjList<olxstr, IEObject*> TStrObjList;
typedef ConstStrObjList<TPrimitiveStrListData<olxstr,IEObject*> >
  const_strobjlist;
typedef TStrPObjList<olxcstr, IEObject*> TCStrObjList;
typedef ConstStrObjList<TPrimitiveStrListData<olxcstr,IEObject*> >
  const_cstrobjlist;
typedef TStrPObjList<olxwstr, IEObject*> TWStrObjList;
typedef ConstStrObjList<TPrimitiveStrListData<olxwstr,IEObject*> >
  const_wstrobjlist;

typedef TTOStringList<TObjectStrListData<olxstr,olxstr> >
  TStrStrList;
typedef ConstStrObjList<TObjectStrListData<olxstr,olxstr> >
  const_strstrlist;
typedef TTOStringList<TObjectStrListData<olxcstr,olxcstr> >
  TCStrCStrList;
typedef 
  ConstStrObjList<TObjectStrListData<olxcstr,olxcstr> >
  const_cstrcstrlist;
typedef TTOStringList<TObjectStrListData<olxwstr,olxwstr> >
  TWStrWStrList;
typedef
  ConstStrObjList<TObjectStrListData<olxwstr,olxwstr> >
  const_wstrwstrlist;

typedef TTStrList<TSingleStringWrapper<olxstr> > TStrList;
typedef ConstStrList<TSingleStringWrapper<olxstr> > const_strlist;
typedef TTStrList<TSingleStringWrapper<olxcstr> > TCStrList;
typedef ConstStrList<TSingleStringWrapper<olxstr> > const_cstrlist;
typedef TTStrList<TSingleStringWrapper<olxwstr> > TWStrList;
typedef ConstStrList< TSingleStringWrapper<olxstr> > const_wstrlist;


EndEsdlNamespace()
#endif
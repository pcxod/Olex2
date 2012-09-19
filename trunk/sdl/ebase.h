/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_base_H
#define __olx_sdl_base_H
#include <typeinfo>
#include <string.h>
#include <stdlib.h>

#define EsdlClassNameT(class)  typeid(class).name()
#define EsdlObjectNameT(object)  typeid(object).name()
#define EsdlClassName(class)  olxstr(typeid(class).name())
#define EsdlObjectName(object)  olxstr(typeid(object).name())

#define EsdlInstanceOf( class, className )  (typeid(class) == typeid(className))

// defines a primitive type property
#define DefPropP(Type, Name) \
public:\
  Type Get##Name() const {  return Name;  }\
  void Set##Name(Type MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBIsSet(Name) \
public:\
  bool Is##Name() const {  return Name;  }\
  void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBHasSet(Name) \
public:\
  bool Has##Name() const {  return Name;  }\
  void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as a bit mask as Is/Set
#define DefPropBFIsSet(Name, VarName, BitMask) \
public:\
  bool Is##Name() const {  return (VarName & BitMask) != 0;  }\
  void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a boolean type property as a bit mask as Has/Set
#define DefPropBFHasSet(Name, VarName, BitMask) \
public:\
  bool Has##Name() const {  return (VarName & BitMask) != 0;  }\
  void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a complex (class) type property
#define DefPropC(Type, Name) \
public:\
  const Type& Get##Name()    const {  return Name;  }\
  void Set##Name(const Type& MaCV) {  Name = MaCV;  }

#define BeginEsdlNamespace()  namespace esdl {
#define EndEsdlNamespace()  };\
  using namespace esdl;
#define UseEsdlNamespace()  using namespace esdl;
#define GlobalEsdlFunction( fun )     esdl::fun
#define EsdlObject( obj )     esdl::obj

#include "defs.h"

#ifdef __WIN32__
  #if !defined(_WIN32_WINNT) && _MSC_VER < 1500
    #define _WIN32_WINNT 0x400
  #endif
  #include <windows.h>
#endif
// there is a mistery how it manages to disapper!!!!
#ifdef __BORLANDC__
  #if !defined(__MT__)
    #define __MT__
  #endif
#endif

BeginEsdlNamespace()

static const size_t InvalidIndex = size_t(~0);
static const size_t InvalidSize = size_t(~0);
// validates if unsigned number is valid... since the move to size_t etc...
template <typename int_t> static bool olx_is_valid_index(const int_t& v)  {
  return v != int_t(~0);
}
template <typename int_t> static bool olx_is_valid_size(const int_t& v)  {
  return v != int_t(~0);
}
// wrap memory management functions
extern void *olx_malloc_(size_t sz);  // throws TOutOfMemoryException 
extern void *olx_realloc_(void * a, size_t sz);  // throws TOutOfMemoryException 
template <typename T> T *olx_malloc(size_t sz) {
  return (T*)olx_malloc_(sz*sizeof(T));
}
template <typename T> T *olx_realloc(T *a, size_t sz) {
  return (T*)olx_realloc_(a, sz*sizeof(T));
}
template <typename T> void olx_free(T *a) {  if( a != NULL )  free(a);  }
template <typename T> T *olx_memcpy(T *dest, const T *src, size_t sz) {
  return (T *)memcpy(dest, src, sz*sizeof(T));
}
template <typename T> T *olx_memmove(T *dest, const T *src, size_t sz) {
  return (T *)memmove(dest, src, sz*sizeof(T));
}
// string base
template <class T> class TTIString {
public:
  static const unsigned short CharSize;
  struct Buffer  {
    T *Data;
    unsigned int RefCnt;
    size_t Length;
    Buffer(size_t len, const T *data=NULL, size_t tocopy=0)  {
      Data = ((len != 0) ? olx_malloc<T>(len) : (T*)NULL);
      if( data != NULL )
        olx_memcpy(Data, data, tocopy);
      RefCnt = 1;
      Length = len;
    }
    // creates object from external array, must be created with malloc
    Buffer(T *data, size_t len)  {
      Data = data;
      RefCnt = 1;
      Length = len;
    }
    ~Buffer()  {  olx_free(Data);  }
    void SetCapacity(size_t newlen)  {
      if( newlen > Length )  {
        Data = olx_realloc(Data, newlen);
        Length = newlen;
      }
    }
  };
protected:
  mutable Buffer *SData;  // do not have much choice with c_str ...
  void IncLength(size_t len)  {  _Length += len;  }
  void DecLength(size_t len)  {  _Length -= len;  }
  size_t GetCapacity()  const {  return SData->Length;  }
  size_t _Increment, _Length;
  mutable size_t _Start;
  TTIString() {}
  void checkBufferForModification(size_t newSize) const {
    if( SData == NULL )
      SData = new Buffer(newSize + _Increment);
    else if( SData->RefCnt > 1 )  {
      SData->RefCnt--;
      Buffer *newData = new Buffer(newSize + _Increment, &SData->Data[_Start],
        olx_min(_Length, newSize));
      SData = newData;
      _Start = 0;
    }
    else if( SData->RefCnt == 1 && _Start != 0 )  {
      if( _Length != 0 )
        memmove(SData->Data, &SData->Data[_Start], _Length*CharSize);
      _Start = 0;
    }
    if( SData->Length < newSize )
      SData->SetCapacity((size_t)(newSize*1.5)+_Increment);
  }
public:
  TTIString(const TTIString& str) {
    SData = str.SData;
    if( SData != NULL )  SData->RefCnt++;
    _Length = str._Length;
    _Start = str._Start;
    _Increment = 8;
  }
  virtual ~TTIString() {
    if( SData != NULL && --SData->RefCnt == 0 )
      delete SData;
  }
  // might not have '\0' char, to be used with Length or RawLen (char count)
  T * raw_str() const { return ((SData == NULL) ? NULL : &SData->Data[_Start]);  }
  // length in bytes
  size_t RawLen() const { return _Length*CharSize;  }
  // length in items
  size_t Length() const { return _Length;  }
  // for internal: use with caution!
  inline Buffer *Data_() const {  return SData;  }
  // for internal stuff
  size_t Start_() const { return _Start;  }
  /* standard api requires terminating '\0'; the use of raw_str and Length() is
 preferable
 */
  const T * u_str() const {
    if( SData == NULL ) return NULL;
    if( (SData->Length == (_Start+_Length)) ||
        (SData->Data[_Start+_Length] != '\0') )
    {
      checkBufferForModification(_Length + 1);
      SData->Data[_Start+_Length] = '\0';
    }
    return &SData->Data[_Start];
  }
  bool IsEmpty() const { return _Length == 0;  }
  T CharAt(size_t i) const {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return SData->Data[_Start + i];
  }
  T operator[] (size_t i) const {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return SData->Data[_Start + i];
  }
  /* reads content of the string to external buffer, which must be able to
 accommodate string length + 1 for the end of string char
  */
  T *Read(T *v)  {
    if( SData == NULL ) return NULL;
    olx_memcpy(v, &SData->Data[_Start], _Length);
    v[_Length] = L'\0';
    return v;
  }
  // this does not help in GCC, 
  template <typename,typename> friend class TTSString;
};

template <typename T> const unsigned short TTIString<T>::CharSize = sizeof(T);
typedef TTIString<olxch> TIString;
typedef TTIString<char> TICString;
typedef TTIString<wchar_t> TIWString;

// implementation of basic object, providing usefull information about a class
class IEObject  {
  /* this function, if set, will be called from the destructor - useful for
 garbage collector...
 */
  struct a_destruction_handler  {
    a_destruction_handler *next;
    a_destruction_handler(a_destruction_handler* _prev) : next(NULL) {
      if( _prev != NULL )
        _prev->next = this;
    }
    virtual ~a_destruction_handler() {}
    virtual void call(IEObject* obj) const = 0;
    virtual const void* get_identifier() const = 0;
  };
  struct static_destruction_handler : public a_destruction_handler {
    void (*destruction_handler)(IEObject* obj);
    static_destruction_handler(
      a_destruction_handler* prev, 
      void (*_destruction_handler)(IEObject* obj)) :
        a_destruction_handler(prev),
        destruction_handler(_destruction_handler) {}
    virtual void call(IEObject* obj) const {  (*destruction_handler)(obj);  }
    virtual const void* get_identifier() const {
      return (const void*)destruction_handler;
    }
  };
  template <class base>
  struct member_destruction_handler : public a_destruction_handler {
    void (base::*destruction_handler)(IEObject* obj);
    base& instance;
    member_destruction_handler(
      a_destruction_handler* prev,
      base& base_instance,
      void (base::*_destruction_handler)(IEObject* obj)) :
        a_destruction_handler(prev),
        instance(base_instance),
        destruction_handler(_destruction_handler) {}
    virtual void call(IEObject* obj) const {
      (instance.*destruction_handler)(obj);
    }
    virtual const void* get_identifier() const {
      return (const void*)destruction_handler;
    }
  };
  
  a_destruction_handler *dsh_head, *dsh_tail;
  
  void _RemoveDestructionHandler(const void* identifier);
  bool _HasDestructionHandler(a_destruction_handler *dh) const;
public:
  IEObject() : dsh_head(NULL), dsh_tail(NULL) {}
  virtual ~IEObject();
  // throws an exception
  virtual TIString ToString() const;
  // throws an exception if not implemented
  virtual IEObject* Replicate() const;
  bool AddDestructionHandler(void (*func)(IEObject*));
  template <class base>
  bool AddDestructionHandler(base& instance, void (base::*func)(IEObject*))  {
    if( dsh_head == NULL ) {
      dsh_head = dsh_tail =
        new member_destruction_handler<base>(NULL, instance, func);
    }
    else {
      a_destruction_handler *e =
        new member_destruction_handler<base>(NULL, instance, func);
      if (_HasDestructionHandler(e)) {
        delete e;
        return false;
      }
      dsh_tail->next = e;
      dsh_tail = e;
    }
    return true;
  }
  template <class T> void RemoveDestructionHandler(const T& identifier)  {
    _RemoveDestructionHandler((const void*)identifier);
  }
};

extern const TIString& NewLineSequence();
extern const TICString& CNewLineSequence();
extern const TIWString& WNewLineSequence();

// an interface for a referencible object
class AReferencible : public IEObject  {
  short This_RefCount;
public:
  AReferencible()  {  This_RefCount = 0;  }
  virtual ~AReferencible();

  short GetRefCount() const {  return This_RefCount;  }
  short DecRef()  {  return --This_RefCount;  }
  short IncRef()  {  return ++This_RefCount;  }
};

// we need this class to throw exceptions from string with gcc ...
class TExceptionBase : public IEObject  {
protected:
  static bool AutoLog;
  /* to prevent creation this class directly. All instances must be of the
 TBasicExceptionClass defined in exception.h
 */
  virtual void CreationProtection() = 0;  
public:
  static void ThrowFunctionFailed(const char* file, const char* function,
    int line, const char* msg);
  static void ThrowIndexOutOfRange(const char* file, const char* function,
    int line, size_t index, size_t min_ind, size_t max_ind);
  static void ThrowInvalidUnsignedFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidUnsignedFormat(const char* file, const char* function,
    int line, const wchar_t* src, size_t src_len);
  static void ThrowInvalidIntegerFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidIntegerFormat(const char* file, const char* function,
    int line, const wchar_t* src, size_t src_len);
  static void ThrowInvalidFloatFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidFloatFormat(const char* file, const char* function,
  int line, const wchar_t* src, size_t src_len);
  static void ThrowInvalidBoolFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidBoolFormat(const char* file, const char* function,
    int line, const wchar_t* src, size_t src_len);
  static TIString FormatSrc(const char* file, const char* func, int line);
  static void SetAutoLogging(bool v)  {  AutoLog = v;  }
  static bool GetAutoLogging()  {  return AutoLog;  }
  // returns recasted this, or throws exception if dynamic_cast fails
  const class TBasicException* GetException() const; 
};

#include "olxptr.h"
#include "eaccessor.h"

struct olx_alg  {
protected:
// logical NOT operator for an analyser
  template <class Analyser> struct not_  {
    const Analyser& analyser;
    not_(const Analyser& _analyser) : analyser(_analyser)  {}
    template <class Item> bool OnItem(const Item& o) const {
      return !analyser.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return !analyser.OnItem(o, i);
    }
  };
  // logical AND operator for two analysers
  template <class AnalyserA, class AnalyserB> struct and_  {
    const AnalyserA& analyserA;
    const AnalyserB& analyserB;
    and_(const AnalyserA& _analyserA, const AnalyserB& _analyserB) :
    analyserA(_analyserA), analyserB(_analyserB)  {}
    template <class Item> bool OnItem(const Item& o) const {
      return analyserA.OnItem(o) && analyserB.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return analyserA.OnItem(o, i) && analyserB.OnItem(o, i);
    }
  };
  // logical OR operator for two analysers
  template <class AnalyserA, class AnalyserB> struct or_  {
    const AnalyserA& analyserA;
    const AnalyserB& analyserB;
    or_(const AnalyserA& _analyserA, const AnalyserB& _analyserB) :
    analyserA(_analyserA), analyserB(_analyserB)  {}
    template <class Item> bool OnItem(const Item& o) const {
      return analyserA.OnItem(o) || analyserB.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return analyserA.OnItem(o, i) || analyserB.OnItem(o, i);
    }
  };

  template <class Accessor> struct minus_ {
    const Accessor &accessor;
    minus_(const Accessor &accessor_) : accessor(accessor_) {}
    template <class Item>
    typename Accessor::return_type OnItem(const Item& o) const {
      return -accessor(o);
    }
    template <class Item>
    typename Accessor::return_type OnItem(const Item& o, size_t) const {
      return -accessor(o);
    }
    template <class Item>
    typename Accessor::return_type operator () (const Item& o) const {
      return -accessor(o);
    }
  };
public:
  /* creates a new not logical operator */
  template <class Analyser>
  static not_<Analyser> olx_not(const Analyser& a)  {
    return not_<Analyser>(a);
  }
  /* creates a new and logical operator */
  template <class AnalyserA, class AnalyserB>
  static and_<AnalyserA, AnalyserB> olx_and(
    const AnalyserA& a, const AnalyserB& b)
  {
    return and_<AnalyserA, AnalyserB>(a, b);
  }
  /* creates a new or logical operator */
  template <class AnalyserA, class AnalyserB>
  static or_<AnalyserA, AnalyserB> olx_or(
    const AnalyserA& a, const AnalyserB& b)
  {
    return or_<AnalyserA, AnalyserB>(a, b);
  }
  /* creates a new minus arithmetic functor/accessor */
  template <class Accessor>
  static minus_<Accessor> olx_minus(const Accessor& a)  {
    return minus_<Accessor>(a);
  }
  /* creates a new static boolean value */
  struct olx_bool  {
    bool value;
    olx_bool(bool _value) : value(_value)  {}
    template <typename Item> bool OnItem(const Item& o) const {
      return value;
    }
    template <typename Item> bool OnItem(const Item& o, size_t) const {
      return value;
    }
  };
};

/* swaps two objects using a temporary variable (copy constructor must be 
 available for complex types) */
template <typename obj> inline void olx_swap(obj& o1, obj& o2)  {
  obj tmp = o1;
  o1 = o2;
  o2 = tmp;
}
// returns 10^val, cannot put it to emath due to dependencies...
template <typename FT> FT olx_pow10(size_t val)  {
  if( val == 0 )  return 1;
  FT rv = 10;
  while( --val > 0 ) rv *=10;
  return rv;
}
/* comparison function (useful for the size_t on Win64, where size_t=uint64_t
 *  and int is int32_t) */
template <typename T1, typename T2> inline
int olx_cmp(T1 a, T2 b)  {  return a < b ? -1 : (a > b ? 1 : 0);  }

template <typename T, typename T1> bool olx_is(const T1 &v) {
  return typeid(T) == typeid(olx_ref::get(v));
};

#include "association.h"
#include "listalg.h"
#include "citem.h"

EndEsdlNamespace()

#include "istream.h"
#include "smart/ostring.h"
#endif

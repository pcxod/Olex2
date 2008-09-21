#ifndef olx_i_string
#define olx_i_string
#include <string.h>
#include <wctype.h>
#include <math.h>

#define CharSizeMask 0xE0000000
#define LengthMask   ~0xE0000000
#define CodeLength(CharSize, Length)   ((CharSize << 29) | (Length))
#define ExtractCharSize(Code)   ((Code) >> 29)
#define ExtractLength(Code)   (((Code) << 3) >> 3)

#ifdef _MSC_VER
  #include <memory.h>
  #define olx_strcmpn  strncmp
  #define olx_strcmpni _strnicmp
  #define olx_wcscmpn  wcsncmp
  #define olx_wcscmpni _wcsnicmp
#elif __BORLANDC__
  #define olx_strcmpn  strncmp
  #define olx_strcmpni strncmpi
  #define olx_wcscmpn  wcsncmp
  #define olx_wcscmpni _wcsnicmp
#else  // POSIX
  #include <strings.h>
  #include <wchar.h>
  #define olx_strcmpn  strncmp
  #define olx_strcmpni strncasecmp
  #define olx_wcscmpn  wcsncmp
  #if defined(__MAC__)
    static int olx_wcscmpni(const wchar_t* s1, const wchar_t* s2, size_t len)  {
	  for( size_t i=0; i < len; i++ )  {
	    int diff = towupper(s1[i]) - towupper(s2[i]); 
	    if( diff != 0 )  return diff; 
	  }
	  return 0;
	} 
  #else
    #define olx_wcscmpni wcsncasecmp
  #endif
#endif

#include "olx_wstring.h"
#include "olx_cstring.h"

BeginEsdlNamespace()
#ifndef __BORLANDC__
template <class,typename> class TTSString;

using esdl::TCString;
using esdl::TWString;

typedef TTSString<TCString, char> CString;
typedef TTSString<TWString, wchar_t> WString;

#ifdef _UNICODE
  typedef TTSString<TWString, wchar_t> olxstr;
#else
  typedef TTSString<TCString, char > olxstr;
#endif

extern const olxstr &EmptyString;
extern const olxstr &FalseString;
extern const olxstr &TrueString;
extern const olxstr &NullString;

extern const CString &CEmptyString;
extern const CString &CFalseString;
extern const CString &CTrueString;
extern const CString &CNullString;

extern const WString &WEmptyString;
extern const WString &WFalseString;
extern const WString &WTrueString;
extern const WString &WNullString;
#endif

template <class T, typename TC> class TTSString : public T  {
  void InitFromCharStr( const TC* str, size_t len)  {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = len;
    T::SData = new struct T::Buffer(T::_Length+T::_Increment, str, T::_Length);
  }
template <class SC>
  void InitFromString(const SC& str)  {
#ifdef __GNUC__
    T::SData = str.GetBuffer();
    T::_Length = str.Length();
    T::_Start = str.Start();
#else
    T::SData = str.SData;
    T::_Length = str._Length;
    T::_Start = str._Start;
#endif
    if( T::SData != NULL )  T::SData->RefCnt++;
    T::_Increment = 8;
  }
  TTSString& AssignCharStr(const TC *str, size_t len=~0)  {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = ((len == ~0) ? o_strlen(str) : len);
    if( T::SData != NULL )  {
      if( T::SData->RefCnt == 1 )  { // owed by this object
        T::SData->SetCapacity(T::_Length);
        memcpy(T::SData->Data, str, T::_Length * T::CharSize);
      }
      else  {
        T::SData->RefCnt--;
        T::SData = NULL;
      }
    }
    if( T::SData == NULL )  
      T::SData = new struct T::Buffer(T::_Length + T::_Increment, str, T::_Length);
    return *this;
  }

public:
  TTSString() : T()  {  }
  //............................................................................
  TTSString(const TTSString& str, size_t start, size_t length) {
    T::SData = str.SData;
    if( T::SData != NULL )  T::SData->RefCnt++;
    T::_Start = str._Start + start;
    T::_Length = length;
    T::_Increment = 8;
  }
  //............................................................................
  TTSString(const TTSString& str)  {  InitFromString(str);  }
  //............................................................................
  TTSString(const TTSString& str, size_t capacity)  {
    InitFromString(str);
    if( capacity != 0 )
      T::checkBufferForModification(T::_Length + capacity);
  }
  //............................................................................
  TTSString(const TC* str)  { InitFromCharStr(str, o_strlen(str));  }
  TTSString(TC* const &str) { InitFromCharStr(str, o_strlen(str));  }
  //............................................................................
  TTSString(const TC* str, size_t len)  { InitFromCharStr(str, len);  }
  TTSString(TC* const &str, size_t len) { InitFromCharStr(str, len);  }
  //............................................................................
  TTSString(const TC& v) {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = 1;
    T::SData = new struct T::Buffer(T::_Length+T::_Increment);
    T::SData->Data[0] = v;
  }
  //............................................................................
  TTSString(const TTIString<TC>& str)  { InitFromString(str);  }
  //............................................................................
  // creates a string from existing data
  static TTSString Adopt(TC* data, size_t length)  {
    TTSString<T,TC> rv;
    rv.SData->Data = data;
    rv.SData->RefCnt = 1;
    rv.SData->Length = length;
    rv._Length = length;
    return rv;
  }
//  TTSString(const char *str) : T(str) {  }

  template <class T1, typename TC1> TTSString(const TTSString<T1,TC1>& v) : T((const T1&)v)  {  }
  //............................................................................
  template <typename AC> TTSString(const AC& v) : T(v)  {  }
  //............................................................................
  inline TTSString& operator << (const TIString& v)  { T::Append(v.raw_str(), v.Length());  return *this;  }
  //............................................................................
  template <class AC> inline TTSString& operator << (const AC& v)  { T::operator << (v);  return *this;  }
  //............................................................................
  inline TTSString& operator << (const TC& v)  {
    T::checkBufferForModification(T::_Length + 1);
    T::SData->Data[T::_Length] = v;
    T::_Length++;
    return *this;
  }
  template <typename AC> inline TTSString& Append(const AC *data, size_t len)  {
    T::Append(data, len);
    return *this;
  }
  TTSString& operator << (TC * const &v)      { return Append(v, o_strlen(v));  }
  TTSString& operator << (const TC *v)        { return Append(v, o_strlen(v));  }
  TTSString& operator << (const TTSString &v) { return Append(v.Data(), v.Length());  }
  template <class T1, typename TC1>
  TTSString& operator << (const TTSString<T1,TC1> &v) { return Append(v.raw_str(), v.Length());  }
  TTSString& operator << (const bool &v)      { return operator << (v ? TrueString : FalseString);  }
  //............................................................................
  //............................................................................
  inline TTSString& operator = (const TTIString<TC>& v)   {
    if( T::SData != NULL && --T::SData->RefCnt == 0 )  delete T::SData;
#ifdef __GNUC__
    T::_Start = v.Start();
    T::_Length = v.Length();
    T::SData = v.GetBuffer();
#else
    T::_Start = v._Start;
    T::_Length = v._Length;
    T::SData = v.SData;
#endif
    if( T::SData != NULL )  T::SData->RefCnt++;
    return *this;
  }
  template <class AC> inline TTSString& operator = (const AC& v)   { T::operator =(v);  return *this; }
  //............................................................................
  //............................................................................
  inline TTSString& operator = (TC * const &str)  {  return AssignCharStr(str);  }
  inline TTSString& operator = (const TC * str)   {  return AssignCharStr(str);  }
  //............................................................................
  TTSString& operator = (const TC& ch)  {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = 1;
    if( T::SData != NULL )  {
      if( T::SData->RefCnt == 1 )  { // owed by this object
        T::SData->SetCapacity(T::_Length);
        T::SData->Data[0] = ch;
      }
      else  {
        T::SData->RefCnt--;
        T::SData = NULL;
      }
    }
    if( T::SData == NULL )  {
      T::SData = new struct T::Buffer(T::_Length + T::_Increment);
      T::SData->Data[0] = ch;
    }
    return *this;
  }
  //............................................................................
  template <class T1, typename TC1> inline TTSString& operator = (const TTSString<T1,TC1>& v)  {
    T::operator = ((const T1&)v);
    return *this;
  }
  TTSString& operator = (const TTSString& v)  {
    if( &v == this )  
      return *this;
    if( T::SData != NULL && --T::SData->RefCnt == 0 )  delete T::SData;
    T::_Start = v._Start;
    T::_Length = v._Length;
    T::SData = v.SData;
    if( T::SData != NULL )  T::SData->RefCnt++;
    return *this;
  }
  //............................................................................
  inline TTSString SubString(size_t from, size_t count) const {
    return TTSString<T,TC>(*this, from, count);
  }
  inline TTSString SubStringFrom(size_t from, size_t indexFromEnd=0) const {
    return TTSString<T,TC>(*this, from, T::_Length-from-indexFromEnd);
  }
  inline TTSString SubStringTo(size_t to, size_t indexFromStart=0) const {
    return TTSString<T,TC>(*this, indexFromStart, to-indexFromStart);
  }
  //............................................................................
  static inline char o_toupper(char ch)     {  return (ch >='a' && ch <= 'z') ? (ch + 'A'-'a') : ch;  }
  static inline char o_tolower(char ch)     {  return (ch >='A' && ch <= 'Z') ? (ch + 'a'-'A') : ch;  }
  static inline wchar_t o_toupper(wchar_t ch)  {  return towupper(ch);  }
  static inline wchar_t o_tolower(wchar_t ch)  {  return towlower(ch);  }
  static inline bool o_isdigit(char ch)     {  return (ch >= '0' && ch <= '9');  }
  static inline bool o_isdigit(wchar_t ch)  {  return (ch >= L'0' && ch <= L'9');  }
  static inline size_t o_strlen(const char *cstr)    {  return (cstr==NULL) ? 0 : strlen(cstr);  }
  static inline size_t o_strlen(const wchar_t *wstr) {  return (wstr==NULL) ? 0 : wcslen(wstr);  }
  static void o_strtup(TC *wht, size_t wht_len)  {
    for( size_t i=0; i < wht_len; i++ )  wht[i] = o_toupper(wht[i]);
  }
  static void o_strtlw(TC *wht, size_t wht_len)  {
    for( size_t i=0; i < wht_len; i++ )  wht[i] = o_tolower(wht[i]);
  }
  // returns length of common string
  template <typename OC, typename AC>
  static size_t o_cmnstr(OC *str1, size_t str1_len, AC *str2, size_t str2_len)  {
    size_t mlen = olx_min(str1_len, str2_len);
    for( size_t i=0; i < mlen; i++ )
      if( str1[i] != str2[i] )  return i;
    return mlen;
  }
  //............................................................................
  TTSString& UpperCase()  {
    T::checkBufferForModification(T::_Length);
    o_strtup( T::Data(), T::_Length );
    return *this;
  }
  //............................................................................
  TTSString& LowerCase()  {
    T::checkBufferForModification(T::_Length);
    o_strtlw( T::Data(), T::_Length );
    return *this;
  }
  //............................................................................
  TTSString ToUpperCase() const {
    TTSString<T, TC> rv(*this);
    return rv.UpperCase();
  }
  //............................................................................
  TTSString ToLowerCase() const {
    TTSString<T, TC> rv(*this);
    return rv.LowerCase();
  }
  //............................................................................
  static TTSString UpperCase(const TTSString& str)  {
    TTSString<T, TC> rv(str);
    return rv.UpperCase();
  }
  //............................................................................
  static TTSString LowerCase(const TTSString& str)  {
    TTSString<T, TC> rv(str);
    return rv.LowerCase();
  }
  //............................................................................
  TTSString CommonString(const TTSString& str)  const {
    return SubStringTo( o_cmnstr(T::Data(), T::_Length, str.Data(), str.Length()) );
  }
  //............................................................................
  static TTSString CommonString(const TTSString& str1, const TTSString& str2) {
    return str1.CommonString(str2);
  }
  //............................................................................
  template <typename OC, typename AC> static int o_memcmp(const OC* wht, const AC* with, size_t len) {
    for( size_t i=0; i < len; i++ )
      if( wht[i] != with[i])  return wht[i]-with[i];
    return 0;
  }
  static inline int o_memcmp(const char* wht, const char* with, size_t len) {
    return olx_strcmpn(wht, with, len);
  }
  static inline int o_memcmp(const wchar_t* wht, const wchar_t* with, size_t len) {
    return olx_wcscmpn(wht, with, len);
  }
  template <typename OC, typename AC> static int o_memcmpi(const OC* wht, const AC* with, size_t len) {
    for( size_t i=0; i < len; i++ )
      if( o_toupper(wht[i]) != o_toupper(with[i]))  return (o_toupper(wht[i])-o_toupper(with[i]));
    return 0;
  }
  static inline int o_memcmpi(const char* wht, const char* with, size_t len) {
    return olx_strcmpni(wht, with, len);

  }
  static inline int o_memcmpi(const wchar_t* wht, const wchar_t* with, size_t len) {
    return olx_wcscmpni(wht, with, len);
  }
  template <typename OC, typename AC>
  static int o_strcmp(const OC* wht, size_t len_a, const AC *with, size_t len_b, bool CI) {
    if( len_a == len_b )  {
      if( len_a == 0 )  return 0;
      return CI ? o_memcmpi(wht, with, len_a) : o_memcmp(wht, with, len_a);
    }
    if( len_a == 0 )  return -1;
    if( len_b == 0 )  return 1;
    int res = CI ? o_memcmpi(wht, with, olx_min(len_a, len_b)) :
                   o_memcmp(wht, with, olx_min(len_a, len_b));
    if( res != 0 )  return res;
    if( len_a < len_b )  return -1;
    return 1;
  }
  inline int Compare(const TTSString& v)       const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false );  }
  inline int Comparei(const TTSString& v)      const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, true );  }
  template <typename AC>
    inline int Compare(const AC *v)            const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false );  }
  template <typename AC>
    inline int Comparei(const AC *v)           const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), true );  }
  inline bool operator == (const TTSString& v) const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false ) == 0; }
  template <typename AC>
    inline bool operator == (const AC *v)      const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false ) == 0; }
  inline bool operator != (const TTSString& v) const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false ) != 0; }
  template <typename AC>
    inline bool operator != (const TC *v)      const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false ) != 0; }
  template <typename AC>
    inline bool operator >  (const AC *v)      const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false ) > 0; }
  template <typename AC>
    inline bool operator <  (const AC *v)      const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false ) < 0; }
  template <typename AC>
    inline bool operator >= (const AC *v)      const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false ) >= 0; }
  template <typename AC>
    inline bool operator <= (const AC *v)      const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false ) <= 0; }
  inline bool operator > (const TTSString &v)  const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false ) > 0; }
  inline bool operator < (const TTSString &v)  const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false ) < 0; }
  inline bool operator >= (const TTSString &v) const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false ) >= 0; }
  inline bool operator <= (const TTSString &v) const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false ) <= 0; }
  //............................................................................
  template <typename AC> inline bool StartsFrom(const AC *v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length ) ? false : (o_memcmp(T::Data(), v, len) == 0);
  }
  template <typename AC> inline bool StartsFromi(const AC *v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length ) ? false : (o_memcmpi(T::Data(), v, len) == 0);
  }
  inline bool StartsFrom(const TTSString &v) const {
    return (v._Length > T::_Length ) ? false : (o_memcmp(T::Data(), v.Data(), v._Length) == 0);
  }
  inline bool StartsFromi(const TTSString &v) const {
    return (v._Length > T::_Length ) ? false : (o_memcmpi(T::Data(), v.Data(), v._Length) == 0);
  }
  //............................................................................
  template <typename AC> inline size_t LeadingCharCount(AC v) const {
    size_t cc = 0;
    while( cc < T::_Length && T::CharAt(cc) == v )  cc++;
    return cc;
  }
  //............................................................................
  template <typename AC> inline bool StartFromi(const AC *v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length ) ? false : (o_memcmpi(T::Data(), v, len) == 0);
  }
  inline bool StartFromi(const TTSString &v) const {
    return (v._Length > T::_Length ) ? false : (o_memcmpi(T::Data(), v.Data(), v._Length) == 0);
  }
  //............................................................................
  template <typename AC> inline bool EndsWith(const AC *v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length ) ? false : (o_memcmp(&T::Data()[T::_Length-len], v, len) == 0);
  }
  inline bool EndsWith(const TTSString &v) const {
    return (v._Length > T::_Length ) ? false : (o_memcmp(&T::Data()[T::_Length-v._Length], v.Data(), v._Length) == 0);
  }
  //............................................................................
  template <typename AC> inline bool EndsWithi(const AC *v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length ) ? false : (o_memcmpi(&T::Data()[T::_Length-len], v, len) == 0);
  }
  inline bool EndsWithi(const TTSString &v) const {
    return (v._Length > T::_Length ) ? false : (o_memcmpi(&T::Data()[T::_Length-v._Length], v.Data(), v._Length) == 0);
  }
  //............................................................................
  TTSString operator + (const TC *v) const { return (TTSString<T,TC>&)TTSString<T,TC>(*this).Append(v, o_strlen(v)); }
  //............................................................................
  TTSString operator + (TC v) const { return (TTSString<T,TC>(*this) << v); }
  //............................................................................
  TTSString operator + (const TTSString& v) const { return (TTSString<T,TC>(*this) << v); }
  //............................................................................
  template <typename OC, typename AC>
  static int o_strpos(const OC *whr, size_t whr_len, const AC *wht, size_t wht_len) {
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return -1;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      if( found ) return i;
    }
    return -1;
  }
  //............................................................................
  template <typename OC, typename AC>
  static int o_chrpos(const OC *whr, size_t whr_len, AC wht) {
    for( size_t i=0; i < whr_len; i++ )
      if( whr[i] == wht )
        return i;
    return -1;
  }
  //............................................................................
  template <typename OC, typename AC>
  static int o_strposi(const OC *whr, size_t whr_len, const AC *wht, size_t wht_len) {
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return -1;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )
        if( o_toupper(whr[i+j]) != o_toupper(wht[j]) )  {
          found = false;
          break;
        }
      if( found ) return i;
    }
    return -1;
  }
  //............................................................................
  template <typename OC, typename AC>
  static int o_chrposi(const OC *whr, size_t whr_len, AC wht) {
    wht = o_toupper(wht);
    for( size_t i=0; i < whr_len; i++ )
      if( o_toupper(whr[i]) == wht )
        return i;
    return -1;
  }
  //............................................................................
  template <typename OC, typename AC>
  static int o_strposr(const OC *whr, size_t whr_len, const AC *wht, size_t wht_len) {
    if( wht_len > whr_len || whr_len == 0 || wht_len == 0)  return -1;
    for( size_t i=whr_len-1; i >= 0; i-- )  {
      if( i < wht_len )  return -1;
      bool found = true;
      for( size_t j=wht_len-1;  j >=0 ; j-- )
        if( whr[i-j] != wht[j] )  {
          found = false;
          break;
        }
      if( found ) return i;
    }
    return -1;
  }
  //............................................................................
  template <typename OC, typename AC>
  static int o_chrposr(const OC *whr, size_t whr_len, AC wht) {
    if( whr_len == 0 )  return -1;
    for( int i=whr_len-1; i >= 0; i-- )
      if( whr[i] == wht )
        return i;
    return -1;
  }
  //............................................................................
  template <typename OC, typename AC>
  static int o_strposri(const OC *whr, size_t whr_len, const AC *wht, size_t wht_len) {
    if( wht_len > whr_len || whr_len == 0 || wht_len == 0 )  return -1;
    for( size_t i=whr_len-1; i >= 0; i-- )  {
      if( i < wht_len )  return -1;
      bool found = true;
      for( size_t j=wht_len-1;  j >=0 ; j-- )
        if( o_toupper(whr[i-j]) != o_toupper(wht[j]) )  {
          found = false;
          break;
        }
      if( found ) return i;
    }
    return -1;
  }
  //............................................................................
  template <typename OC, typename AC>
  static int o_chrposri(const OC *whr, size_t whr_len, AC wht) {
    if( whr_len == 0 )  return -1;
    wht = o_toupper(wht);
    for( size_t i=whr_len-1; i >= 0; i-- )
      if( o_toupper(whr[i]) == wht )
        return i;
    return -1;
  }
  //............................................................................
  int IndexOf(const TTSString& wht)  const { return o_strpos( T::Data(), T::_Length, wht.Data(), wht._Length);  }
  int IndexOfi(const TTSString& wht) const { return o_strposi( T::Data(), T::_Length, wht.Data(), wht._Length);  }
  int IndexOf(const TC *wht)         const { return o_strpos( T::Data(), T::_Length, wht, o_strlen(wht));  }
  int IndexOfi(const TC *wht)        const { return o_strposi( T::Data(), T::_Length, wht, o_strlen(wht));  }
  int IndexOf(TC wht)                const { return o_chrpos( T::Data(), T::_Length, wht);  }
  int IndexOfi(TC wht)               const { return o_chrposi( T::Data(), T::_Length, wht);  }

  int FirstIndexOf(const TTSString& wht, size_t from = 0)  const {
    int i = o_strpos( &T::Data()[from], T::_Length-from, wht.Data(), wht._Length);
    return ((i==-1) ? -1 : i + from);
  }
  int FirstIndexOfi(const TTSString& wht, size_t from = 0) const {
    int i = o_strposi( &T::Data()[from], T::_Length-from, wht.Data(), wht._Length);
    return ((i==-1) ? -1 : i + from);
  }
  int FirstIndexOf(const TC *wht, size_t from = 0)         const {
    int i = o_strpos( &T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==-1) ? -1 : i + from);
  }
  int FirstIndexOfi(const TC *wht, size_t from = 0)        const {
    int i = o_strposi( &T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==-1) ? -1 : i + from);
  }
  int FirstIndexOf(TC wht, size_t from = 0)                const {
    int i = o_chrpos( &T::Data()[from], T::_Length-from, wht);
    return ((i==-1) ? -1 : i + from);
  }
  int FirstIndexOfi(TC wht, size_t from = 0)               const {
    int i = o_chrposi( &T::Data()[from], T::_Length-from, wht);
    return ((i==-1) ? -1 : i + from);
  }

  int LastIndexOf(const TTSString& wht, size_t from = ~0)  const {
    return o_strposr( T::Data(), olx_min(from, T::_Length), wht.Data(), wht._Length);
  }
  int LastIndexOfi(const TTSString& wht, size_t from = ~0) const {
    return o_strposri( T::Data(), olx_min(from, T::_Length), wht.Data(), wht._Length);
  }
  int LastIndexOf(const TC *wht, size_t from = ~0)         const {
    return o_strposr( T::Data(), olx_min(from, T::_Length), wht, o_strlen(wht));
  }
  int LastIndexOfi(const TC *wht, size_t from = ~0)        const {
    return o_strposri( T::Data(), olx_min(from, T::_Length), wht, o_strlen(wht));
  }
  int LastIndexOf(TC wht, size_t from = ~0)          const {
    return o_chrposr( T::Data(), olx_min(from, T::_Length), wht);
  }
  int LastIndexOfi(TC wht, size_t from = ~0)               const {
    return o_chrposri( T::Data(), olx_min(from, T::_Length), wht);
  }

  //............................................................................
  // function checks for preceding radix encoding
  template <typename IT> static IT o_atois(const TC *data, size_t len, unsigned short Rad=10) {
    if( len == 0 )    
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    size_t sts = 0; // string start, end
    while( data[sts] == ' ' && ++sts < len )
    if( sts >= len )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    // test for any particluar format specifier, here just '0x', for hexadecimal
    if( len > sts+1 && data[sts] == '0' && data[sts+1] == 'x' )  {
      Rad = 16;
      sts += 2;
    }
    return o_atoi<IT>(&data[sts], len-sts, Rad);
  }
  //............................................................................
  template <typename IT> inline IT SafeInt(unsigned short Rad=10) const  {
    return o_atois<IT>(T::Data(), T::_Length, Rad);
  }
  //............................................................................
  // no leading '\0', got to do it ourselves
  template <typename IT> static IT o_atoi(const TC *data, size_t len, unsigned short Rad=10) {
    if( len == 0 )    
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    size_t sts = 0; // string start, end
    while( data[sts] == ' ' && ++sts < len )
    if( sts >= len )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    IT val=0;
    bool Negative = false;
    if( data[sts] == '-' )  {  Negative = true;  sts++;  }
    else if( data[sts] == '+' )  sts++;
    if( sts == len )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    if( Rad > 10 )  {
      for( size_t i=sts; i < len; i++ )  {
        IT pv = -1;
        if( data[i] <= '9' && data[i] >= '0' )  // the order is important, chars are rearer
          pv = data[i] - '0';
        else if( data[i] <= 'Z' && data[i] >= 'A' )
          pv = data[i] - 'A' + 10;
        else if( data[i] <= 'z' && data[i] >= 'a' )
          pv = data[i] - 'a' + 10;
        else  // throw
          break;
        val = val*Rad + pv;
       }
     }
     else  {
       for( size_t i=sts; i < len; i++ )  {
         if( data[i] <= '9' && data[i] >= '0' )
           val = val*Rad + (data[i] - '0');
         else  // throw
           break;
       }
     }
     return Negative ? -val : val;
   }
  //............................................................................
  template <typename IT> inline IT RadInt(unsigned short Rad=10) const  {
     return o_atoi<IT>(T::Data(), T::_Length, Rad);
   }
  //............................................................................
  inline int ToInt() const  {  return o_atoi<int>( T::Data(), T::_Length, 10);  }
  //............................................................................
  inline bool ToBool() const  {  return (Comparei(TrueString) == 0);  }
  //............................................................................
  inline static double o_pow10(long val)  {
#ifdef __BORLANDC__
    return pow10(val);
#endif
    if( val == 0 )  return 1;
    double rv = 10;
    while( --val > 0 ) rv *=10;
    return rv;
  }
  // no leading '\0', got to do it ourselves
  static double o_atod(const TC *data, size_t len) {
    if( len == 0 )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    size_t sts = 0; // string start
    while( data[sts] == ' ' && ++sts < len )
    if( sts >= len )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    bool negative = false;
    if( data[sts] == '-' )  {  negative = true;  sts++;  }
    else if( data[sts] == '+' )  sts++;
    if( sts == len )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    double bp=0, ap=0, apexp=1;
    long exp = 0;
    bool fpfound = false, expfound = false, expneg = false;
    for( size_t i=sts; i < len; i++ )  {
      if( data[i] <= '9' && data[i] >= '0' )
        if( expfound )
          exp = exp*10 + (data[i] - '0');
        else if( fpfound  )  {
          ap = ap*10 + (data[i] - '0');
          apexp *= 10;
        }
        else
          bp = bp*10 + (data[i] - '0');
      else if( data[i] == '.' )
        fpfound = true;
      else if( data[i] == 'e' || data[i] == 'E' )  {
        expfound = true;
        if( ++i == len )  break;
        if( data[i] == '-' )
          expneg = true;
        else if( data[i] == '+' )
          ;
        else if( data[i] >= '0' && data[i] <= '9' )  // anonymous positiv exp
         i--;
        else  // invalid dddd.ddddE-/+/ddd format
          break;
      }
      else  // invalid char for a number
        break;
    }
    bp = (expneg) ? (bp + ap/apexp)/o_pow10(exp) : (bp + ap/apexp)*o_pow10(exp);
    return negative ? -bp : bp;
  }
  inline double ToDouble() const {  return o_atod(T::Data(), T::_Length);  }
  //............................................................................
  void SetLength(size_t newLen)  {
    T::checkBufferForModification(newLen);
    if( newLen < T::_Length )  DecLength( T::_Length - newLen );
  }
  //............................................................................
  void Delete(size_t from, size_t count)  {
    register size_t dv = from+count;
    if( dv > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid size to delete");
    else  {
      if( dv == T::_Length )  { 
        if( from != 0 ) {  T::_Length -= count;  return;  }  // substring to
        else            {  T::_Length = 0;      return;  }  // empty string ...
      }
    }
    // delete from start - just substring from
    if( from == 0 )  {  T::_Start += count;  T::_Length -= count;  return;  }
    
    T::checkBufferForModification(T::_Length);
    memmove( &T::Data()[from], &T::Data()[from+count], (T::_Length-from-count)*T::CharSize);
    T::DecLength(count);
  }
  //............................................................................
  //replaces sequences of chars with a single char
  template <typename AC> static size_t o_strdcs(TC *whr, size_t whr_len, AC wht)  {
    size_t ni = 0;
    for( size_t i=0; i < whr_len; i++, ni++ )  {
      if( whr[i] == wht && ((i+1) < whr_len && whr[i+1] == wht) )  {
        ni--;
        continue;
      }
      else if( ni != i )  whr[ni] = whr[i];
    }
    return whr_len - ni;
  }
  //............................................................................
  void DeleteSequencesOf(char wht, bool removeTrailing = true)  {
    T::checkBufferForModification(T::_Length);
    DecLength( o_strdcs(T::Data(), T::_Length, wht) );
    if( removeTrailing && T::_Length != 0 && TTIString<TC>::Data(T::_Length-1) == wht )
      T::DecLength(1);
  }
  //............................................................................
  void DeleteSequencesOf(wchar_t wht, bool removeTrailing = true)  {
    T::checkBufferForModification(T::_Length);
    DecLength( o_strdcs(T::Data(), T::_Length, wht) );
    if( removeTrailing && T::_Length != 0 && TTIString<TC>::Data(T::_Length-1) == wht )
      T::DecLength(1);
  }
  //............................................................................
  template <typename AC> static TTSString DeleteSequencesOf(const TTSString& str, AC wht, bool removeTrailing=true)  {
    TTSString<T,TC> rv(str);
    rv.DeleteSequencesOf(wht, removeTrailing);
    return rv;
  }
  //............................................................................
  //removes chars from string
  template <typename AC> static size_t o_strdch(TC *whr, size_t whr_len, AC wht)  {
    size_t ni = 0;
    for( size_t i=0; i < whr_len; i++, ni++ )  {
      if( whr[i] == wht )  {
        ni--;
        continue;
      }
      else if( ni != i )  whr[ni] = whr[i];
    }
    return (whr_len - ni);
  }
  //............................................................................
  template <typename AC> void DeleteChars(AC wht)  {
    T::checkBufferForModification(T::_Length);
    T::DecLength( o_strdch(T::Data(), T::_Length, wht) );
  }
  //............................................................................
  template <typename AC> static TTSString DeleteChars(const TTSString& str, AC wht)  {
    TTSString<T,TC> rv(str);
    rv.DeleteChars(wht);
    return rv;
  }
  //............................................................................
  static size_t o_strins(const TC *wht, size_t wht_len, TC *to, size_t to_len, size_t at, size_t amount=1)  {
    size_t toshift = wht_len*amount;
    if( at < to_len )
      memmove(&to[at+toshift], &to[at], (to_len - at)*T::CharSize);
    for( size_t i=0; i < amount; i++ )
      memcpy(&to[at+wht_len*i], wht, wht_len*T::CharSize);
    return toshift;
  }
  //............................................................................
  static size_t o_chrins(TC wht, TC *to, size_t to_len, size_t at, size_t amount=1)  {
    if( at < to_len )
      memmove(&to[at+amount], &to[at], (to_len - at)*T::CharSize);
    //memset(to, wht, CharSize*amount);
    for( size_t i=0; i < amount; i++ )
      to[at+i] = wht;
    return amount;
  }
  //............................................................................
  void Insert(const TTSString& wht, size_t whr, size_t amount=1)  {
    if( whr > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "index out of range");
    T::checkBufferForModification(T::_Length + wht._Length*amount);
    T::IncLength( o_strins(wht.Data(), wht._Length, T::Data(), T::_Length, whr, amount) );
  }
  //............................................................................
  void Insert(const TC *wht, size_t whr, size_t amount=1)  {
    if( whr > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "index out of range");
    size_t wht_len = o_strlen(wht);
    T::checkBufferForModification(T::_Length + wht_len*amount);
    T::IncLength( o_strins(wht, wht_len, T::Data(), T::_Length, whr, amount) );
  }
  //............................................................................
  void Insert(TC wht, size_t whr, size_t amount=1)  {
    if( whr > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "index out of range");
    T::checkBufferForModification(T::_Length + amount);
    T::IncLength( o_chrins(wht, T::Data(), T::_Length, whr, amount) );
  }
  //............................................................................
  // counts number of occurences of chars
  template <typename OC>
  static size_t o_chrcnt(OC wht, const TC *whr, size_t whr_len)  {
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( whr[i] == wht )
        cnt++;
    }
    return cnt;
  }
  //............................................................................
  template <typename AC> inline size_t CharCount(AC wht) const {  return o_chrcnt(wht, T::Data(), T::_Length);  }
  //............................................................................
  template <typename OC>
  static size_t o_strcnt(const OC *wht, size_t wht_len, const TC *whr, size_t whr_len)  {
    if( wht_len > whr_len )  return 0;
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return cnt;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      if( found ) cnt++;
    }
    return cnt;
  }
  // replaces a string with another, returns number of replacements
  template <typename OC, typename AC>
  static size_t o_strrplstr(const OC *wht, const size_t wht_len,
                         const AC *with, const size_t with_len,
                         TC *whr, size_t whr_len)  {
    if( wht_len > whr_len )  return 0;
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {                              
      if( i+wht_len > whr_len )  return cnt;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )  {
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      }
      if( found )  {
        if( with_len != wht_len )  {
          memmove( &whr[i+with_len], &whr[i+wht_len], (whr_len-i-wht_len)*T::CharSize );
          whr_len -= (wht_len-with_len);
        }
        for( size_t j=0;  j < with_len; j++ )
          whr[i+j] = with[j];
        cnt++;
        i += (with_len-1);
      }
    }
    return cnt;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_strrplch(const OC *wht, const size_t wht_len,
                         AC with,
                         TC *whr, size_t whr_len)  {
    if( wht_len > whr_len )  return 0;
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return cnt;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )  {
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      }
      if( found )  {
        if( wht_len != 1 )  {
          memmove( &whr[i+1], &whr[i+wht_len], (whr_len-i-wht_len)*T::CharSize );
          whr_len -= (wht_len-1);
        }
        whr[i] = with;
        cnt++;
      }
    }
    return cnt;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_chrplstr(OC wht,
                         const AC *with, const size_t with_len,
                         TC *whr, size_t whr_len)  {
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( whr[i] == wht )  {
        if( with_len != 1 )  {
          memmove( &whr[i+with_len], &whr[i+1], (whr_len-i-1)*T::CharSize );
          whr_len -= (1-with_len);
        }
        for( size_t j=0;  j < with_len; j++ )
          whr[i+j] = with[j];
        cnt++;
        i += (with_len-1);
      }
    }
    return cnt;
  }
  //............................................................................
  template <typename OC, typename AC> // replace chars
  static size_t o_chrplch(OC wht,  AC with,
                         TC *whr, size_t whr_len)  {
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( whr[i] == wht )  {
          whr[i] = with;
        cnt++;
      }
    }
    return cnt;
  }
  //............................................................................
  size_t Replace(const TTSString &wht, const TTSString &with)  {
    size_t extra_len = 0;
    if( wht._Length < with._Length )  {
      extra_len = (with._Length - wht._Length) * o_strcnt(wht.Data(), wht._Length, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht.Data(), wht._Length, with.raw_str(), with._Length, T::Data(), T::_Length);
    T::_Length -= (wht._Length - with._Length)*rv;
    return rv;
  }
  //............................................................................
  template <typename AC> size_t Replace(const TTSString &wht, const AC *with)  {
    size_t extra_len = 0, with_len = o_length(with);
    if( wht._Length < with_len )  {
      extra_len = (with_len - wht._Length) * o_strcnt(wht.Data(), wht._Length, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht.Data(), wht._Length, with, o_length(with), T::Data(), T::_Length);
    T::_Length -= (wht._Length - with_len)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(const TTSString &wht, wchar_t with)  {
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht.Data(), wht._Length, with, T::Data(), T::_Length);
    T::_Length -= (wht._Length - 1)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(const TTSString &wht, char with)  {
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht.Data(), wht._Length, with, T::Data(), T::_Length);
    T::_Length -= (wht._Length - 1)*rv;
    return rv;
  }
  //............................................................................
  template <typename AC> size_t Replace(const AC *wht, const TTSString &with)  {
    size_t extra_len = 0, wht_len = o_strlen(wht);
    if( wht_len < with._Length )  {
      extra_len = (with._Length - wht_len) * o_strcnt(wht, wht_len, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht, o_strlen(wht), with.Data(), with._Length, T::Data(), T::_Length);
    T::_Length -= (wht_len - with._Length)*rv;
    return rv;
  }
  //............................................................................
  template <typename OC, typename AC> size_t Replace(const OC *wht, const AC *with)  {
    size_t extra_len = 0, with_len = o_strlen(with), wht_len = o_strlen(wht);
    if( wht_len < with_len )  {
      extra_len = (with_len - wht_len) * o_strcnt(wht, wht_len, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht, o_strlen(wht), with, o_strlen(with), T::Data(), T::_Length);
    T::_Length -= (wht_len - with_len)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(const wchar_t *wht, wchar_t with)  {
    size_t wht_len = o_strlen(wht);
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht, wht_len, with, T::Data(), T::_Length);
    T::_Length -= (wht_len - 1)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(const char *wht, char with)  {
    size_t wht_len = o_strlen(wht);
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht, wht_len, with, T::Data(), T::_Length);
    T::_Length -= (wht_len - 1)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(char wht, const TTSString &with)  {
    size_t extra_len = 0;
    if( with._Length > 1 )  {
      extra_len = (with._Length - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with.Data(), with._Length, T::Data(), T::_Length);
    T::_Length -= (1 - with._Length)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(wchar_t wht, const TTSString &with)  {
    size_t extra_len = 0;
    if( with._Length > 1 )  {
      extra_len = (with._Length - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with.Data(), with._Length, T::Data(), T::_Length);
    T::_Length -= (1 - with._Length)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(char wht, const char *with)  {
    size_t extra_len = 0, with_len = o_strlen(with);
    if( with_len > 1 )  {
      extra_len = (with_len - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with, o_strlen(with), T::Data(), T::_Length);
    T::_Length -= (1 - with_len)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(wchar_t wht, const wchar_t *with)  {
    size_t extra_len = 0, with_len = o_strlen(with);
    if( with_len > 1 )  {
      extra_len = (with_len - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return 0;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with, o_strlen(with), T::Data(), T::_Length);
    T::_Length -= (1 - with_len)*rv;
    return rv;
  }
  //............................................................................
  size_t Replace(char wht, char with)  {
    T::checkBufferForModification(T::_Length);
    return o_chrplch(wht, with, T::Data(), T::_Length);
  }
  //............................................................................
  size_t Replace(wchar_t wht, wchar_t with)  {
    T::checkBufferForModification(T::_Length);
    return o_chrplch(wht, with, T::Data(), T::_Length);
  }
  //............................................................................
  template <typename AC> TTSString Trim(AC wht) const {
    if( T::_Length == 0 )  return EmptyString;
    size_t start = 0, end = T::_Length;
    while( TTIString<TC>::Data(start) == wht && ++start < end )  ;
    while( --end > start && TTIString<TC>::Data(end) == wht )  ;
    return SubString(start, end+1-start );
  }
  //............................................................................
  inline TTSString& TrimFloat() {
    T::TrimFloat();
    return *this;
  }
  //............................................................................
  TTSString& Format(size_t count, bool Right, const TTSString &sep)  {
    size_t extra = count-((T::_Length > count) ? count-1 : T::_Length)*sep._Length;
    T::checkBufferForModification(T::_Length + extra);
    if( T::_Length > count )  return (*this << sep);
    Insert(sep, (Right) ? T::_Length : 0, count-T::_Length);
    return *this;
  }
  //............................................................................
  TTSString& Format(size_t count, bool Right, char sep)  {
    int extra = count-((T::_Length > count) ? count-1 : T::_Length);
    checkBufferForModification(T::_Length + extra);
    if( T::_Length > count )  return (*this << sep);
    Insert(sep, (Right) ? T::_Length : 0, count - T::_Length);
    return *this;
  }
  //............................................................................
  static TTSString FormatFloat(int NumberOfDigits, double v, bool Exponent = false)  {
    TTSString<T,TC> fmt, rv;
    if( NumberOfDigits < 0 )  {
      NumberOfDigits = -NumberOfDigits;
      if( v >= 0 )  fmt = ' ';
    }
    fmt << "%." << NumberOfDigits << ( (Exponent) ? 'e' : 'f') << '\0';
    rv.setTypeValue(fmt.Data(), v);
    return rv;
  }
  //............................................................................
  bool IsNumber() const  {
    if( T::_Length == 0 )  return false;
    // hex notation
    if( T::_Length >= 3 && TTIString<TC>::Data(0) == '0' && TTIString<TC>::Data(1) == 'x' )  {
      for( size_t i=2; i < T::_Length; i++ )  {
        TC ch = o_toupper(TTIString<TC>::Data(i));
        if( !( (ch >= '0' && ch <= '9') ||
               (ch >= 'A' && ch <= 'F')
             )  )  return false;
      }
      return true;
    }
    size_t st = 0;
    bool expfound = false, fpfound = false;
    if( TTIString<TC>::Data(0) == '+' || TTIString<TC>::Data(0) == '-' )
      st++;
    for( size_t i = st; i < T::_Length; i++ )  {
      TC ch = TTIString<TC>::Data(i);
      if( ch <= '9' && ch >= '0' )
        continue;
      else if( ch == '.' )  {
        if( fpfound )  return false;
        fpfound = true;
      }
      else if( ch == 'e' || ch == 'E' )  {
        if( expfound )  return false;
        expfound = true;
        if( ++i == T::_Length )  return false;
        ch = TTIString<TC>::Data(i);
        if( ch == '-' )
          ;
        else if( ch == '+' )
          ;
        else if( ch >= '0' && ch <= '9' )  // anonymous positiv exp
         i--;
        else  // invalid dddd.ddddE-/+/ddd format
          return false;
      }
      else
        return false;
    }
    return true;
  }
  //............................................................................
  //TTIString<TC> toIstr() const {  return TTIString<TC>(Data());  }
  //............................................................................
  inline void SetIncrement(size_t ni)  {  T::_Increment = ni;  }
  void SetCapacity(size_t newc)  {
    if( T::SData == NULL )  T::SData = new struct T::Buffer(newc+T::_Increment);
    else                 
      T::checkBufferForModification(newc);
  }
  //............................................................................
  TTSString& AppendFromStream(IInputStream &ios, size_t len)  {
    T::checkBufferForModification(T::_Length + len+1);
    ios.Read((void*)&T::SData->Data[T::_Start+T::_Length], len*T::CharSize);
    T::_Length += len;
    return *this;
  }
  //............................................................................
  
  void FromBinaryStream(IInputStream &ios)  {
    uint32_t code, len, charsize;
    ios.Read(&code, sizeof(uint32_t));
    charsize = ExtractCharSize(code);
    if( T::CharSize != charsize )
      if( charsize != 0 || T::CharSize == 2 )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "incompatible Char size");
    len = ExtractLength(code);
    if( len > (uint32_t)(ios.GetSize() - ios.GetPosition()) )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid stream content");
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = len;
    if( T::SData != NULL )  {
      if( T::SData->RefCnt == 1 )  // owed by this object
        T::SData->SetCapacity(T::_Length);
      else  {
        T::SData->RefCnt--;
        T::SData = NULL;
      }
    }
    if( T::SData == NULL )  T::SData = new struct T::Buffer(T::_Length);
    ios.Read((void*)T::SData->Data, T::_Length*T::CharSize);
  }
  //............................................................................
  void ToBinaryStream(IOutputStream& os) const {
    uint32_t len = CodeLength(T::CharSize, T::_Length);
    os.Write(&len, sizeof(uint32_t));
    os.Write( (void*)T::Data(), T::_Length*T::CharSize );
  }
  //............................................................................
  static TTSString CharStr(TC ch, size_t count)  {
    TTSString<T,TC> rv(EmptyString, count);
    rv.Insert(ch, 0, count);
    return rv;
  }
  //............................................................................
  olxch Last() const {  
    if( T::_Length == 0 )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "empty string");
    return T::SData->Data[T::_Start+T::_Length-1];
  }
  //............................................................................
  virtual IEObject* Replicate() const {  return new TTSString<T,TC>(*this);  }
  //............................................................................
/*
  int Strtok(TArrayList< TTSString<T, TC> > &toks, const TTSString<T, TC> &sep) const {
    int cnt = toks.Count();
    if( sep.IsEmpty() )
      toks.Add( *this );
    else if( sep._Length == 1 )  {
      size_t start = 0;
      char ch = sep[0];
      for( size_t i=0; i < _Length; i++ )  {
        if( Data(i) == ch )  {
          toks.Add( TTSString<T,TC>(*this, start, i-start) );
          start = i+1;
        }
      }
      if( start < _Length )
        toks.Add( TTSString<T,TC>(*this, start, _Length-start) );
    }
    else  {
      size_t start = 0;
      for( size_t i=0; i < _Length; i++ )  {
        if( i+sep._Length >= _Length )  break;
        bool found = true;
        for( size_t j=0;  j < sep._Length; j++ )
        if( Data(i+j) != sep[j] )  {
          found = false;
          break;
        }
        if( found )  {
          toks.Add( TTSString<T,TC>(*this, start, i-start) );
          start = i+sep._Length;
        }
      }
      if( start < _Length )
        toks.Add( TTSString<T,TC>(*this, start, _Length-start) );
    }
    return toks.Count() - cnt;
  }
*/  
};

EndEsdlNamespace()

#endif

#ifndef __olx_sdl_bitarray_H
#define __olx_sdl_bitarray_H
#include "ebase.h"
#include "istream.h"

BeginEsdlNamespace()

class TEBitArray: public IEObject  {
  unsigned char *FData; 
  size_t FCount, FCharCount;
public:
  TEBitArray();
  TEBitArray(const TEBitArray& arr);
  TEBitArray(size_t size);
  // if own is true, data [created with new!] will be deleted automatically 
  TEBitArray(unsigned char* data, size_t size, bool own);
  virtual ~TEBitArray();
  void Clear();
  void SetSize(size_t newSize);
  inline size_t Count() const {  return FCount;  }
  inline bool IsEmpty() const {  return FCount == 0;  }
  inline bool operator [] (size_t index) const  {
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    return (FData[intIndex] & bitIndex) != 0;
  }
  bool Get(size_t index) const  {
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    return (FData[intIndex] & bitIndex) != 0;
  }
  inline void Set(size_t index, bool v)  {
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    if( !v )  FData[intIndex] &= ~bitIndex;
    else      FData[intIndex] |= bitIndex;
  }
  inline void SetTrue(size_t index)   {  
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] |= bitIndex;
  }
  inline void SetFalse(size_t index)  {  
    size_t intIndex = index/8;
    size_t bitIndex = 1 << index%8;
#ifdef _OLX_DEBUG
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, intIndex, 0, FIntCount);
#endif
    FData[intIndex] &= ~bitIndex;
  }
  void SetAll(bool v);
  inline const unsigned char* GetData() const {  return FData;  }
  inline size_t CharCount() const {  return FCharCount;  }

  void operator << (IInputStream& in);
  void operator >> (IOutputStream& out) const;
  
  TEBitArray& operator = (const TEBitArray& arr);
  bool operator == (const TEBitArray& arr)  const;
  int Compare(const TEBitArray& arr)  const;
  // base64 based string + one char {'0'+size%8}
  olxstr ToBase64String() const;
  void FromBase64String(const olxstr& str);

  virtual TIString ToString() const;
  olxstr FormatString(uint16_t bitsInSegment) const;
};

EndEsdlNamespace()
#endif

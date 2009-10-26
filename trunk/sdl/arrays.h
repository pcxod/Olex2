#ifndef arraysH
#define arraysH

#ifdef __BORLANDC__
  #include <mem.h>
#else
  #include <stdlib.h>
  #include <string.h>
#endif
#include "evector.h"

BeginEsdlNamespace()
/*
  These arrays are only suitable for primitive types, as memset zeroes the initialised arrays.
  These arrays are just for convinience, as the Data attribute is publically exposed for the 
  performance issues
*/
template <class AE> class TArray1D : public IEObject  {
  const size_t _Length;
  index_t MinIndex;
public:
// the indexes provided are inclusive
  TArray1D(index_t minIndex, index_t maxIndex) : MinIndex(minIndex), _Length(maxIndex - minIndex + 1)  {
    Data = NULL;
    if( minIndex >= maxIndex )  throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE[_Length];
    memset( Data, 0, _Length*sizeof(AE) );
  }

  virtual ~TArray1D()  {  if( Data != NULL )  delete [] Data;  }

  void InitWith( const AE& val)  {
    for( size_t i=0; i < _Length; i++ )
      Data[i] = val;
  }

  inline void FastInitWith(const int val)  {  memset( Data, val, _Length*sizeof(AE) );  }

  // direct access to private member
  AE* Data;

  inline size_t Length()  const  {  return _Length;  }
  inline bool IsInRange(index_t ind) const {  return ind >= MinIndex && ((ind-MinIndex) < _Length);  }
  inline AE& operator [] (index_t index)  const {  return Data[index-MinIndex];  }
  inline AE& Value(index_t index)  const {  return Data[index-MinIndex];  }
};

// we do not use TArray1D< TArray1D > for performance reasons...
template <class AE> class TArray2D : public IEObject {
  const size_t Width, Height;
  const index_t MinWidth, MinHeight;
public:
// the indexes provided are inclusive
  TArray2D(index_t minWidth, index_t maxWidth, index_t minHeight, index_t maxHeight) : 
      MinWidth(minWidth), Width(maxWidth - minWidth + 1), 
      MinHeight(minHeight), Height(maxHeight - minHeight + 1)  {
    Data = NULL;
    if( minWidth >= maxWidth || minHeight >= maxHeight )  
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE*[Width];
    for( size_t i=0; i < Width; i++ )  {
      Data[i] = new AE[Height];
      memset( Data[i], 0, Height*sizeof(AE) );
    }
  }

  virtual ~TArray2D()  {
    if( Data == NULL )  return;  // if exception is thrown
    for( size_t i=0; i < Width; i++ )
      delete [] Data[i];
    delete [] Data;
  }

  void InitWith(const AE& val)  {
    for( int i=0; i < Width; i++ ) 
      for( int j=0; j < Height; j++ )
        Data[i][j] = val;
  }

  void FastInitWith(const int val)  {
    for( int i=0; i < Width; i++ ) 
      memset( Data[i], val, Height*sizeof(AE) );
  }

  inline bool IsInRange(index_t x, index_t y) const {  
    return (x >= MinWidth && ((x-MinWidth) < Width)) &&
           (y >= MinHeight && ((y-MinHeight) < Height));  
  }
  inline size_t GetWidth()  const  {  return Width;  }
  inline size_t Length1()   const  {  return Width;  }
  inline size_t GetHeight() const  {  return Height;  }
  inline size_t Length2()   const  {  return Height;  }
  
  // direct access to private member
  AE** Data;

  inline AE& Value(index_t x, index_t y)  {  return Data[x-MinWidth][y-MinHeight];  }
  inline AE& operator () (index_t x, index_t y)  {  return Data[x-MinWidth][y-MinHeight];  }
  
  template <class VC>
    inline AE& Value(const TVector<VC>& ind)  {  return Data[(size_t)(ind[0]-MinWidth)][(size_t)(ind[1]-MinHeight)];  }
  template <class VC>
    inline AE& operator () (const TVector<VC>& ind)  {  return Data[(size_t)(ind[0]-MinWidth)][(size_t)(ind[1]-MinHeight)];  }
};

// we do not use TArray1D<TArray2D<AE>*>* Data for performance reasons
template <class AE> class TArray3D : public IEObject {
  const size_t Width, Height, Depth;
  const index_t MinWidth, MinHeight, MinDepth;
public:
  // the indexes provided are inclusive
  TArray3D(index_t minWidth, index_t maxWidth, index_t minHeight,
      index_t maxHeight, index_t minDepth, index_t maxDepth) : 
        MinWidth(minWidth), Width(maxWidth - minWidth + 1),
        MinHeight(minHeight), Height(maxHeight - minHeight + 1),
        MinDepth(minDepth), Depth(maxDepth - minDepth + 1)
  {
    Data = NULL;
    if( minWidth >= maxWidth || minHeight >= maxHeight || minDepth >= maxDepth )  
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    Data = new AE**[Width];
    for( size_t i=0; i < Width; i++ )  {
      Data[i] = new AE*[Height];
      for( size_t j=0; j < Height; j++ )  {
        Data[i][j] = new AE[Depth];
        memset( Data[i][j], 0, Depth*sizeof(AE) );
      }
    }
  }

  virtual ~TArray3D()  {
    for( size_t i=0; i < Width; i++ )  {
      for( size_t j=0; j < Height; j++ )
        delete [] Data[i][j];
      delete [] Data[i];
    }
    delete Data;
  }

  void InitWith(const AE& val)  {
    for( size_t i=0; i < Width; i++ )
      for( size_t j=0; j < Height; j++ )
        for( size_t k=0; k < Depth; k++ )
          Data[i][j][k] = val;
  }

  void FastInitWith(const int val)  {
    for( size_t i=0; i < Width; i++ )
      for( size_t j=0; j < Height; j++ )
        memset( Data[i][j], val, Depth*sizeof(AE) );
  }

  inline bool IsInRange(index_t x, index_t y, index_t z) const {  
    return (x >= MinWidth && ((x-MinWidth) < Width)) &&
           (y >= MinHeight && ((y-MinHeight) < Height)) &&
           (z >= MinDepth && ((z-MinDepth) < Depth));  
  }
  template <class vec> inline bool IsInRange(const vec& ind) const {  
    return (ind[0] >= MinWidth && ((ind[0]-MinWidth) < Width)) &&
           (ind[1] >= MinHeight && ((ind[1]-MinHeight) < Height)) &&
           (ind[2] >= MinDepth && ((ind[2]-MinDepth) < Depth));  
  }
  inline size_t GetWidth()  const  {  return Width;  }
  inline size_t Length1()   const  {  return Width;  }
  inline size_t GetHeight() const  {  return Height;  }
  inline size_t Length2()   const  {  return Height;  }
  inline size_t GetDepth()  const  {  return Depth;  }
  inline size_t Length3()   const  {  return Depth;  }

  // direct access to private member
  AE*** Data;

  inline AE& Value(index_t x, index_t y, index_t z)  {  return Data[x-MinWidth][y-MinHeight][z-MinDepth];  }
  inline const AE& Value(index_t x, index_t y, index_t z) const  {  
    return Data[x-MinWidth][y-MinHeight][z-MinDepth];  
  }
  inline AE& operator () (index_t x, index_t y, index_t z)  {  
    return Data[x-MinWidth][y-MinHeight][z-MinDepth];  
  }
  
  inline const AE& operator () (index_t x, index_t y, index_t z) const {  
    return Data[x-MinWidth][y-MinHeight][z-MinDepth];  
  }

  template <class VC> inline AE& Value(const VC& ind)  {  
    return Data[(size_t)(ind[0]-MinWidth)][(size_t)(ind[1]-MinHeight)][(size_t)(ind[2]-MinDepth)];  
  }
  template <class VC> inline const AE& Value(const VC& ind) const {  
    return Data[(size_t)(ind[0]-MinWidth)][(size_t)(ind[1]-MinHeight)][(size_t)(ind[2]-MinDepth)];  
  }
  template <class VC> inline AE& operator () (const VC& ind)  {  
    return Data[(size_t)(ind[0]-MinWidth)][(size_t)(ind[1]-MinHeight)][(size_t)(ind[2]-MinDepth)];  
  }
  template <class VC> inline const AE& operator () (const VC& ind) const {  
    return Data[(size_t)(ind[0]-MinWidth)][(size_t)(ind[1]-MinHeight)][(size_t)(ind[2]-MinDepth)];  
  }
};

class  TArraysTest  {
public:
  static void Test();
};

EndEsdlNamespace()
#endif

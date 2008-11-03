#ifndef __olx_3x3
#define __olx_3x3

#include "exception.h"
#include "evpoint.h"

BeginEsdlNamespace()

template <typename> class TVector33;
template <typename> class TMatrix33;

template <class T>  class TVector3 : public IEObject {
  T data[3];
public:
  TVector3()                  {  data[0] = data[1] = data[2] = 0;  }
  TVector3(T x, T y, T z)     { data[0] = x;  data[1] = y;  data[2] = z;  }

  TVector3(const TVector3<T>& v) {  data[0] = v[0];  data[1] = v[1];  data[2] = v[2];  }
  template <class AT> TVector3(const TVector3<AT>& v) {  data[0] = (T)v[0];  data[1] = (T)v[1];  data[2] = (T)v[2];  }

//  template <class AT> TVector3<T>(const TEVPoint<AT>& v)  {
//    data[0] = v[0].GetV();  data[1] = v[1].GetV();  data[2] = v[2].GetV();  
//  }

  inline T& operator [] (int i) {  return data[i];  }
  inline T const& operator [] (int i) const {  return data[i];  }
  inline T QLength()    const {  return (data[0]*data[0]+data[1]*data[1]+data[2]*data[2]);  }
  inline T Length()     const {  return sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]);  }
  
  template <class AT> inline T DistanceTo(const TVector3<AT>& v) const {  
    return sqrt( (data[0]-v[0])*(data[0]-v[0]) + (data[1]-v[1])*(data[1]-v[1]) + (data[2]-v[2])*(data[2]-v[2]) ); 
  }
  template <class AT> inline T QDistanceTo(const TVector3<AT>& v) const {  
    return ( (data[0]-v[0])*(data[0]-v[0]) + (data[1]-v[1])*(data[1]-v[1]) + (data[2]-v[2])*(data[2]-v[2]) ); 
  }
  template <class AT> inline T CAngle(const TVector3<AT>& v) const {
    T l = QLength()*v.QLength();
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    l = (T)((data[0]*v[0]+data[1]*v[1]+data[2]*v[2])/sqrt(l));
    // treat possible rounding errors
    if( l > 1 )  l = 1;
    if( l < -1 )  l = -1;
    return l;
  }
  template <class AT> inline T DotProd(const TVector3<AT>& v) const {
    return data[0]*v[0] + data[1]*v[1] + data[2]*v[2];
  }
  // ax(bxc) = b(a.c) - c(a.b)
  static inline TVector3<T> TripleProd(const TVector3<T>& a, const TVector3<T>& b, const TVector3<T>& c)  {
    double p1 = a.DotProd(c), p2 = a.DotProd(b);
    return TVector3<T>( b[0]*p1 - c[0]*p2, b[1]*p1 - c[1]*p2, b[2]*p1 - c[2]*p2);
  }
  // a*b*sin = a*b*(1-cos^2) = a*b - DotProd^2
  template <class AT> inline T XProdVal (const TVector3<AT>& v) const {
    T dp = DotProd(v); 
    return sqrt(QLength()*v.QLength()) - dp*dp;
  }
  template <class AT> inline TVector3<T> XProdVec(const TVector3<AT>& v) const  {
    return TVector3<T>( data[1]*v[2] - data[2]*v[1], 
                        data[2]*v[0] - data[0]*v[2], 
                        data[0]*v[1] - data[1]*v[0] );
  }
  /* returns a normal to vector through a point (vector with same origin as this vector)
  N = this*cos(this,point)*point.length/this.length = this*DotProd(this, point)/this.QLength
  */
  template <class AT> inline TVector3<T> Normal(const TVector3<AT>& point) const {
    T m = QLength();
    if( m == 0 )  throw TDivException(__OlxSourceInfo);
    m = DotProd(point)/m;
    return TVector3<T>(point[0]-data[0]*m, point[1]-data[1]*m, point[2]-data[2]*m);  
  }
  inline TVector3<T> operator -() const {
    return TVector3<T>( -data[0], -data[1], -data[2] );
  }
  // returns a reflection of this vector from a plane represented by normal
  template <class AT> inline TVector3<T> Reflect(const TVector3<AT>& normal) const {
    T m = DotProd(normal)*2;
    return TVector3<T>(data[0] - normal[0]*m, data[1] - normal[1]*m, data[2] - normal[2]*m);  
  }
  inline TVector3<T>& Normalise()  {
    T l = Length();
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    return (*this /= l);
  }
  inline TVector3<T>& NormaliseTo(T val)  {
    T l = Length();
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    return (*this *= (val/l));
  }
  inline TVector3<T>& Null()  {  data[0] = data[1] = data[2] = 0;  return *this;  }
  inline bool IsNull() const {  return (data[0] == 0 && data[1] == 0 && data[2] == 0) ? true: false;  }

  inline bool operator == (const TVector3<T>& v) const {
    return (data[0] == v[0] && data[1] == v[1] && data[2] == v[2]) ? true : false;  
  }
  inline TVector3<T>& operator = (const TVector3<T>& v)  {
    data[0] = v[0];  data[1] = v[1];  data[2] = v[2];  
    return *this;
  }
  // any vector
  template <class AT> inline TVector3<T>& operator = (const AT& v)  {
    data[0] = (T)v[0];  data[1] = (T)v[1];  data[2] = (T)v[2];  
    return *this;
  }
  template <class AT> inline TVector3<T>& operator += (const TVector3<AT>& v)  {
    data[0] += (T)v[0];  data[1] += (T)v[1];  data[2] += (T)v[2];  
    return *this;
  }
  template <class AT> inline TVector3<T>& operator -= (const TVector3<AT>& v)  {
    data[0] -= (T)v[0];  data[1] -= (T)v[1];  data[2] -= (T)v[2];  
    return *this;
  }
  template <class AT> inline TVector3<T>& operator *= (const TVector3<AT>& v)  {
    data[0] *= (T)v[0];  data[1] *= (T)v[1];  data[2] *= (T)v[2];  
    return *this;
  }
#ifndef __BORLANDC__ // stupid compiler
  template <class AT> inline TVector3<T>& operator += (AT v)  {
    data[0] += (T)v;  data[1] += (T)v;  data[2] += (T)v;
    return *this;
  }
  template <class AT> inline TVector3<T>& operator -= (AT v)  {
    data[0] -= (T)v;  data[1] -= (T)v;  data[2] -= (T)v;
    return *this;
  }
  template <class AT> inline TVector3<T>& operator *= (AT v)  {
    data[0] *= (T)v;  data[1] *= (T)v;  data[2] *= (T)v;
    return *this;
  }
#else
  inline TVector3<T>& operator *= (double v)  {
    data[0] *= v;  data[1] *= v;  data[2] *= v;
    return *this;
  }
#endif
  template <class AT> inline TVector3<T>& operator /= (AT v)  {
    data[0] /= v;  data[1] /= v;  data[2] /= v;
    return *this;
  }

  template <class AT> inline TVector3<T> operator + (const TVector3<AT>& v) const {
    return TVector3<T>(data[0]+v[0], data[1]+v[1], data[2]+v[2]);
  }
  template <class AT> inline TVector3<T> operator - (const TVector3<AT>& v) const {
    return TVector3<T>(data[0]-v[0], data[1]-v[1], data[2]-v[2]);
  }
  template <class AT> inline TVector3<T> operator * (const TVector3<AT>& v) const {
    return TVector3<T>(data[0]*v[0], data[1]*v[1], data[2]*v[2]);
  }

#ifndef __BORLANDC__ // stupid compiler
  template <class AT> inline TVector3<T> operator + (AT v) const {
    return TVector3<T>(data[0]+v, data[1]+v, data[2]+v);
  }
  template <class AT> inline TVector3<T> operator - (AT v) const {
    return TVector3<T>(data[0]-v, data[1]-v, data[2]-v);
  }
  template <class AT> inline TVector3<T> operator * (AT v) const {
    return TVector3<T>(data[0]*v, data[1]*v, data[2]*v);
  }
#else
  inline TVector3<T> operator * (double v) const {
    return TVector3<T>(data[0]*v, data[1]*v, data[2]*v);
  }
#endif
  template <class AT> inline TVector3<T> operator / (AT v) const {
    return TVector3<T>(data[0]/v, data[1]/v, data[2]/v);
  }

  /* beware - transposed form, use M.v for normal multiplication
    if matrix has more elements (in vectors) than given vector - only
    number of vector elements is used
  */
  template <class AT> TVector3<T>  operator * (const TMatrix33<AT>& a) const  {
    return TVector3<T>( data[0]*a[0][0] + data[1]*a[1][0] + data[2]*a[2][0],
                        data[0]*a[0][1] + data[1]*a[1][1] + data[2]*a[2][1],
                        data[0]*a[0][2] + data[1]*a[1][2] + data[2]*a[2][2]);
  }

  /* beware - transposed form, use M.v for normal multiplication
    if matrix has more elements (in vectors) than given vector - only
    number of vector elements is used
  */
  template <class AT> TVector3<T>& operator *=(const TMatrix33<AT>& a)  {
    T bf[] = {(T)(data[0]*a[0][0] + data[1]*a[1][0] + data[2]*a[2][0]),
              (T)(data[0]*a[0][1] + data[1]*a[1][1] + data[2]*a[2][1]),
              (T)(data[0]*a[0][2] + data[1]*a[1][2] + data[2]*a[2][2])};
    data[0] = bf[0];  data[1] = bf[1];  data[2] = bf[2];
    return *this;
  }
  template <class SC> SC StrRepr() const  {
    SC rv(data[0], 100);
    return rv << ", " << data[1] << ", " << data[2];
  }
  inline TIString ToString() const {  return StrRepr<olxstr>();  }
  inline CString  ToCStr()   const {  return StrRepr<CString>();  }
  inline WString  ToWStr()   const {  return StrRepr<WString>();  }
};

template <class T> class TMatrix33  {
protected:
  TVector3<T> data[3];
  TMatrix33(bool v)  { 
    if( v )  
      data[0][0] = data[0][1] = data[0][2] = data[1][0] = data[1][1] = data[1][2] = data[2][0] = data[2][1] = data[2][2] = 0; 
  }

public:
  TMatrix33()  {  
    data[0][0] = data[0][1] = data[0][2] = data[1][0] = data[1][1] = data[1][2] = data[2][0] = data[2][1] = data[2][2] = 0; 
  }
  TMatrix33(T xx, T xy, T xz, T yx, T yy, T yz, T zx, T zy, T zz)  {
    data[0][0] = xx;  data[0][1] = xy;  data[0][2] = xz;
    data[1][0] = yx;  data[1][1] = yy;  data[1][2] = yz;
    data[2][0] = zx;  data[2][1] = zy;  data[2][2] = zz;
  }
  TMatrix33(T xx, T xy, T xz, T yy, T yz, T zz)  {
    data[0][0] = xx;  data[0][1] = xy;  data[0][2] = xz;
    data[1][0] = xy;  data[1][1] = yy;  data[1][2] = yz;
    data[2][0] = xz;  data[2][1] = yz;  data[2][2] = zz;
  }
  template <class vt>
  TMatrix33(const TVector3<vt>& x, const TVector3<vt>& y, const TVector3<vt>& z)  {
    data[0] = x;  data[1] = y;  data[2] = z;
  }
  TMatrix33(const TMatrix33<T>& v)  {
    data[0][0] = v[0][0];  data[0][1] = v[0][1];  data[0][2] = v[0][2];
    data[1][0] = v[1][0];  data[1][1] = v[1][1];  data[1][2] = v[1][2];
    data[2][0] = v[2][0];  data[2][1] = v[2][1];  data[2][2] = v[2][2];
  }
  template <class AT> TMatrix33(const TMatrix33<AT>& v)  {
    data[0][0] = (T)v[0][0];  data[0][1] = (T)v[0][1];  data[0][2] = (T)v[0][2];
    data[1][0] = (T)v[1][0];  data[1][1] = (T)v[1][1];  data[1][2] = (T)v[1][2];
    data[2][0] = (T)v[2][0];  data[2][1] = (T)v[2][1];  data[2][2] = (T)v[2][2];
  }
  
  inline TVector3<T> const& operator [] (int i)  const {  return data[i];  } 
  inline TVector3<T>& operator [] (int i) {  return data[i];  } 
  
  template <class AT> TMatrix33<T> operator * (const TMatrix33<AT>& v) const {
    return TMatrix33<T>( data[0][0]*v[0][0] + data[0][1]*v[1][0] + data[0][2]*v[2][0],
                         data[0][0]*v[0][1] + data[0][1]*v[1][1] + data[0][2]*v[2][1],
                         data[0][0]*v[0][2] + data[0][1]*v[1][2] + data[0][2]*v[2][2],
                         data[1][0]*v[0][0] + data[1][1]*v[1][0] + data[1][2]*v[2][0],
                         data[1][0]*v[0][1] + data[1][1]*v[1][1] + data[1][2]*v[2][1],
                         data[1][0]*v[0][2] + data[1][1]*v[1][2] + data[1][2]*v[2][2],
                         data[2][0]*v[0][0] + data[2][1]*v[1][0] + data[2][2]*v[2][0],
                         data[2][0]*v[0][1] + data[2][1]*v[1][1] + data[2][2]*v[2][1],
                         data[2][0]*v[0][2] + data[2][1]*v[1][2] + data[2][2]*v[2][2]);
  }
  template <class AT> TMatrix33<T>& operator *= (const TMatrix33<AT>& v) {
    T bf[] = { data[0][0]*v[0][0] + data[0][1]*v[1][0] + data[0][2]*v[2][0],
                         data[0][0]*v[0][1] + data[0][1]*v[1][1] + data[0][2]*v[2][1],
                         data[0][0]*v[0][2] + data[0][1]*v[1][2] + data[0][2]*v[2][2],
                         data[1][0]*v[0][0] + data[1][1]*v[1][0] + data[1][2]*v[2][0],
                         data[1][0]*v[0][1] + data[1][1]*v[1][1] + data[1][2]*v[2][1],
                         data[1][0]*v[0][2] + data[1][1]*v[1][2] + data[1][2]*v[2][2],
                         data[2][0]*v[0][0] + data[2][1]*v[1][0] + data[2][2]*v[2][0],
                         data[2][0]*v[0][1] + data[2][1]*v[1][1] + data[2][2]*v[2][1],
                         data[2][0]*v[0][2] + data[2][1]*v[1][2] + data[2][2]*v[2][2]};
    data[0][0] = bf[0];  data[0][1] = bf[1];  data[0][2] = bf[2];
    data[1][0] = bf[3];  data[1][1] = bf[4];  data[1][2] = bf[5];
    data[2][0] = bf[6];  data[2][1] = bf[7];  data[2][2] = bf[8];
    return *this;
  }
  inline static TMatrix33 Transpose (const TMatrix33& v) {
    return TMatrix33<T>(v[0][0], v[1][0], v[2][0], 
                        v[0][1], v[1][1], v[2][1], 
                        v[0][2], v[1][2], v[2][2]);
  }
  inline TMatrix33 operator -() const {
    return TMatrix33<T>(-data[0][0], -data[0][1], -data[0][2], 
                        -data[1][0], -data[1][1], -data[1][2], 
                        -data[2][0], -data[2][1], -data[2][2]);
  }
  template <class AT> inline static TMatrix33<AT>& Transpose (const TMatrix33& src, TMatrix33<AT>& dest) {
    dest[0][0] = src[0][0];  dest[0][1] = src[1][0];  dest[0][2] = src[2][0];
    dest[1][0] = src[0][1];  dest[1][1] = src[1][1];  dest[1][2] = src[2][1];
    dest[2][0] = src[0][2];  dest[2][1] = src[1][2];  dest[2][2] = src[2][2];
    return dest;
  }
  inline TMatrix33<T>& Transpose() {
    T v = data[0][1];  data[0][1] = data[1][0];  data[1][0] = v; 
    v = data[0][2];    data[0][2] = data[2][0];  data[2][0] = v; 
    v = data[1][2];    data[1][2] = data[2][1];  data[2][1] = v; 
    return *this;
  }

  inline TMatrix33<T>& I()  {
    data[0][0] = 1;  data[0][1] = 0;  data[0][2] = 0;
    data[1][0] = 0;  data[1][1] = 1;  data[1][2] = 0;
    data[2][0] = 0;  data[2][1] = 0;  data[2][2] = 1;
    return *this;
  }
  inline bool IsI() const {
    return (data[0][0] == 1 && data[1][1] == 1 && data[2][2] == 1 && 
            data[0][1] == 0 && data[0][2] == 0 && data[1][0] == 0 &&
            data[1][2] == 0 && data[2][0] == 0 && data[2][1] == 0 ) ? true : false;
  }
  inline TMatrix33<T>& Null()  {
    data[0].Null();  data[1].Null();  data[2].Null();
    return *this;
  }

  inline bool operator == (const TMatrix33<T>& v) const {
    return (data[0][0] == v[0][0] && data[1][1] == v[1][1] && data[2][2] == v[2][2] && 
        data[0][1] == v[0][1] && data[0][2] == v[0][2] && data[1][0] == v[1][0] &&
        data[1][2] == v[1][2] && data[2][0] == v[2][0] && data[2][1] == v[2][1] ) ? true : false;
  }

  inline TMatrix33<T>& operator = (const TMatrix33<T>& v)  {
    data[0][0] = v[0][0];  data[0][1] = v[0][1];  data[0][2] = v[0][2];
    data[1][0] = v[1][0];  data[1][1] = v[1][1];  data[1][2] = v[1][2];
    data[2][0] = v[2][0];  data[2][1] = v[2][1];  data[2][2] = v[2][2];
    return *this;
  }

  template <class AT> inline TMatrix33<T>& operator = (const TMatrix33<AT>& v)  {
    data[0][0] = (T)v[0][0];  data[0][1] = (T)v[0][1];  data[0][2] = (T)v[0][2];
    data[1][0] = (T)v[1][0];  data[1][1] = (T)v[1][1];  data[1][2] = (T)v[1][2];
    data[2][0] = (T)v[2][0];  data[2][1] = (T)v[2][1];  data[2][2] = (T)v[2][2];
    return *this;
  }
#ifndef __BORLANDC__ // really annoying - would use same for the Matrix33!
  template <class AT> inline TMatrix33<T>& operator *= (AT v) {
    data[0][0] *= v;  data[0][1] *= v;  data[0][2] *= v;
    data[1][0] *= v;  data[1][1] *= v;  data[1][2] *= v;
    data[2][0] *= v;  data[2][1] *= v;  data[2][2] *= v;
    return *this;
  }
#else
  inline TMatrix33<T>& operator *= (double v) {
    data[0][0] *= v;  data[0][1] *= v;  data[0][2] *= v;
    data[1][0] *= v;  data[1][1] *= v;  data[1][2] *= v;
    data[2][0] *= v;  data[2][1] *= v;  data[2][2] *= v;
    return *this;
  }
#endif
  template <class AT> inline TMatrix33<T>& operator /= (AT v) {
    data[0][0] /= v;  data[0][1] /= v;  data[0][2] /= v;
    data[1][0] /= v;  data[1][1] /= v;  data[1][2] /= v;
    data[2][0] /= v;  data[2][1] /= v;  data[2][2] /= v;
	return *this;
  }

  inline T Determinant() const {
    return data[0][0]*(data[1][1]*data[2][2] - data[1][2]*data[2][1]) - 
           data[0][1]*(data[1][0]*data[2][2] - data[1][2]*data[2][0]) +
           data[0][2]*(data[1][0]*data[2][1] - data[1][1]*data[2][0]);
  }
  inline TMatrix33<T> Inverse()  const {
	return TMatrix33( data[2][2]*data[1][1] - data[2][1]*data[1][2],
			          data[2][1]*data[0][2] - data[2][2]*data[0][1],
			          data[1][2]*data[0][1] - data[1][1]*data[0][2],
			          data[2][0]*data[1][2] - data[2][2]*data[1][0],
					  data[2][2]*data[0][0] - data[2][0]*data[0][2],
					  data[1][0]*data[0][2] - data[1][2]*data[0][0],
					  data[2][1]*data[1][0] - data[2][0]*data[1][1],
					  data[2][0]*data[0][1] - data[2][1]*data[0][0],
					  data[1][1]*data[0][0] - data[1][0]*data[0][1])/=Determinant();
  }
  template <class AT> TVector3<AT>  operator * (const TVector3<AT>& a) const  {
    return TVector3<AT>( a[0]*data[0][0] + a[1]*data[0][1] + a[2]*data[0][2],
                         a[0]*data[1][0] + a[1]*data[1][1] + a[2]*data[1][2],
                         a[0]*data[2][0] + a[1]*data[2][1] + a[2]*data[2][2]);
  }
  static void  EigenValues(TMatrix33& A, TMatrix33& I)  {
    int i, j;
    double a = 2;
    while( fabs(a) > 1e-15 )  {
      MatMaxX( A, i, j );
      multMatrix( A, I, i, j );
      a = MatMaxX(A, i, j );
    }
  }

      // used in the Jacoby eigenvalues search procedure
protected: 
  static inline T MatMaxX(const TMatrix33& m, int &i, int &j )  {
    double c = fabs(m[0][1]);
    i = 0;  j = 1;
    if( fabs(m[0][2]) > c )  {
      j = 2;  
      return fabs(m[0][2]);
    }
    if( fabs(m[1][2]) > c )  {
       i = 1;  j = 2;
       return fabs(m[1][2]);
    }
    return c;
  }
  static inline void multMatrix(TMatrix33& D, TMatrix33& E, int i, int j)  {
    double cf, sf, cdf, sdf;
    static const double sqr2 = sqrt(2.0)/2;
    if( D[i][i] == D[j][j] )  {
      cdf = 0;
      cf  = sqr2;
      sf  = Sign(D[i][j])*sqr2;
      sdf = Sign(D[i][j]);
    }
    else  {
      double tdf = 2*D[i][j]/(D[j][j] - D[i][i]);
      double r = tdf*tdf;
      cdf = sqrt( 1.0/(1+r) );
      cf  = sqrt( (1+cdf)/2.0);
      sdf = (sqrt( r/(1+r) ) * Sign(tdf));
      sf  = (sqrt((1-cdf)/2.0)*Sign(tdf));
    }
    double ji, jj,ij,ii,ja,ia;
    ij = D[i][j];
    ii = D[i][i];
    jj = D[j][j];
    D[i][j] = D[j][i] = 0;
    D[i][i] = (ii*cf*cf + jj*sf*sf - ij*sdf);
    D[j][j] = (ii*sf*sf + jj*cf*cf + ij*sdf);
    
    ij = E[i][0];  ji = E[j][0];
    E[i][0] = ij*cf - ji*sf; //i
    E[j][0] = ij*sf + ji*cf;   //j

    ij = E[i][1];  ji = E[j][1];
    E[i][1] = ij*cf - ji*sf; //i
    E[j][1] = ij*sf + ji*cf;   //j

    ij = E[i][2];  ji = E[j][2];
    E[i][2] = ij*cf - ji*sf; //i
    E[j][2] = ij*sf + ji*cf;   //j

    if( i != 0 && j != 0 )  {
      ia = D[i][0];  ja = D[j][0];
      D[i][0] = D[0][i] = ia*cf - ja*sf;
      D[j][0] = D[0][j] = ia*sf + ja*cf;
    }
    if( i != 1 && j != 1 )  {
      ia = D[i][1];  ja = D[j][1];
      D[i][1] = D[1][i] = ia*cf - ja*sf;
      D[j][1] = D[1][j] = ia*sf + ja*cf;
    }
    if( i != 2 && j != 2 )  {
      ia = D[i][2];  ja = D[j][2];
      D[i][2] = D[2][i] = ia*cf - ja*sf;
      D[j][2] = D[2][j] = ia*sf + ja*cf;
    }
  }
public:
  // solves a set of equations by the Cramer rule {equation arr.c = b }, returns c
  static TVector3<T> CramerSolve(const TMatrix33<T>& arr, const TVector3<T>& b) {
    double det = arr.Determinant();
    if( det == 0 )  throw TDivException(__OlxSourceInfo);
    // det( {b[0], a12, a13}, {b[1], a22, a23}, {b[2], a32, a33} )/det
    return TVector3<T>( b[0]*(arr[1][1]*arr[2][2] - arr[1][2]*arr[2][1]) - 
                        arr[0][1]*(b[1]*arr[2][2] - arr[1][2]*b[2]) +
                        arr[0][2]*(b[1]*arr[2][1] - arr[1][1]*b[2]),
    // det( {a11, b[0], a13}, {a21, b[1], a23}, {a31, b[2], a33} )/det
                        arr[0][0]*(b[1]*arr[2][2] - arr[1][2]*b[2]) - 
                        b[0]*(arr[1][0]*arr[2][2] - arr[1][2]*arr[2][0]) +
                        arr[0][2]*(arr[1][0]*b[2] - b[1]*arr[2][0]),
    // det( {a11, a12, b[0]}, {a21, a22, b[1]}, {a31, a32, b[2]} )/det
                        arr[0][0]*(arr[1][1]*b[2] - b[1]*arr[2][1]) - 
                        arr[0][1]*(arr[1][0]*b[2] - b[1]*arr[2][0]) +
                        b[0]*(arr[1][0]*arr[2][1] - arr[1][1]*arr[2][0]))/det;
  }
  // solves a set of equations by the Gauss method {equation arr.c = b ?c }
  static void GaussSolve(TMatrix33<T>& arr, TVector3<T>& b, TVector3<T>& c) {
    MatrixElementsSort(arr, b );
    for ( int j = 1; j < 3; j++ )
      for( int i = j; i < 3; i++ )  {
        if( arr[i][j-1] ==0 )  continue;
        b[i]  *= -(arr[j-1][j-1]/arr[i][j-1]);
        arr[i] *= -(arr[j-1][j-1]/arr[i][j-1]);
        arr[i][0] += arr[j-1][0];
        arr[i][1] += arr[j-1][1];
        arr[i][2] += arr[j-1][2];
        b[i] += b[j-1];
      }
    if( arr[2][2]==0)
      throw TFunctionFailedException(__OlxSourceInfo, "dependent set of equations");

    c[2] = b[2]/arr[2][2];
    for(int j = 1; j >=0; j--)  {
      for(int k1=1; k1 < 4-j; k1++)  {
        if( k1 == (3-j) )
          for( int i=2; i > 3-k1; i-- )  
            b[j] -= arr[j][i]*c[i];
      }
      c[j]= b[j]/arr[j][j];
     }
   }

protected:  // used in GauseSolve to sort the matrix
  static void MatrixElementsSort(TMatrix33<T>& arr, TVector3<T>& b)  {
    T bf[3];
    for( int i = 0; i < 3; i++ )  {
      bf[0] = (arr[0][i] < 0) ? -arr[0][i] : arr[0][i];
      bf[1] = (arr[1][i] < 0) ? -arr[1][i] : arr[1][i];
      bf[2] = (arr[2][i] < 0) ? -arr[2][i] : arr[2][i];
      int n = 0;
      if( bf[1] > bf[n] )  n = 1;
      if( bf[2] > bf[n] )  n = 2;
      if( n != i )  {
        T c = arr[i][0];  arr[i][0] = arr[n][0];  arr[n][0] = c;
          c = arr[i][1];  arr[i][1] = arr[n][1];  arr[n][1] = c;
          c = arr[i][2];  arr[i][2] = arr[n][2];  arr[n][2] = c;
        // changing b[i] and b[n]
        c = b[i];     b[i] = b[n];     b[n] = c;
      }
    }
  }

};

  typedef TVector3<float>  vec3f;
  typedef TVector3<double> vec3d;
  typedef TVector3<int>    vec3i;

  typedef TTypeList<vec3i> vec3i_list;
  typedef TTypeList<vec3f> vec3f_list;
  typedef TTypeList<vec3d> vec3d_list;

  typedef TPtrList<vec3i> vec3i_plist;
  typedef TPtrList<vec3f> vec3f_plist;
  typedef TPtrList<vec3d> vec3d_plist;

  typedef TMatrix33<int>    mat3i;
  typedef TMatrix33<float>  mat3f;
  typedef TMatrix33<double> mat3d;

  typedef TTypeList<mat3i> mat3i_list;
  typedef TTypeList<mat3f> mat3f_list;
  typedef TTypeList<mat3d> mat3d_list;

  typedef TPtrList<mat3i> mat3i_plist;
  typedef TPtrList<mat3f> mat3f_plist;
  typedef TPtrList<mat3d> mat3d_plist;

EndEsdlNamespace()

#endif

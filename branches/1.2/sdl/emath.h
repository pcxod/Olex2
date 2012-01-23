/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_emath_H
#define __olx_emath_H
#include <math.h>
#include "exception.h"
// Linux stuff....
#undef QLength

BeginEsdlNamespace()

// returns corresponding character for sign
template <typename T> inline olxch olx_sign_char(const T& p)  {  return (p<0) ? olxT('-') : olxT('+');  }
// determines the sign of a number
template <typename T> inline T olx_sign(const T& a)  {  return (T)((a < 0 ) ? -1 : 1);  }
//copies sign from b to a: returns a with sign of b
template <class T1, typename T2> T2 olx_copy_sign(T1 a, T2 b)  {
  return (b < 0) ? (a < 0 ? a : -a) : (a < 0 ? -a : a);
}
/* solves an equation by the Newton method
 f - function, df - first derivative, point - starting point */
extern double olx_newton_solve( double (*f)(double), double (*df)(double), double point);
// calculates factorial of a number
template <typename arg_t> double olx_factorial(const arg_t& a)  {
  double b=1;
  for( arg_t i=2; i <= a; i++ )  b *= i;
  return b;
}
template <typename ret_t, typename arg_t> ret_t olx_factorial_t(const arg_t& a)  {
  arg_t b=1;
  for( arg_t i=2; i <= a; i++ )  b *= i;
  return static_cast<ret_t>(b);
}
// rounds a floating point number
template <typename float_t> inline long olx_round(const float_t a)  {
  long b = (long)a;  // |b| is always smaller than a
  return ((a < 0) ? (((b-a) >= .5) ? --b : b) : (((a-b) >= .5) ? ++b : b));
}
template <typename int_t, typename float_t> inline int_t olx_round_t(const float_t a)  {
  int_t b = (int_t)a;  // |b| is always smaller than a
  return ((a < 0) ? (((b-a) >= .5) ? --b : b) : (((a-b) >= .5) ? ++b : b));
}
// rounds a floating point: returns (float_t)(round(a*num))/num 
template <typename float_t> inline float_t olx_round(const float_t a, long num)  {
  return (float_t)olx_round(a*num)/num;
}
template <typename float_t> inline long olx_floor(const float_t a)  {
  long b = (long)a;
  return (a < 0) ? b-1 : b;
}
template <typename int_t, typename float_t> inline int_t olx_floor_t(const float_t a)  {
  int_t b = (int_t)a;
  return (a < 0) ? b-1 : b;
}
// returns true if the given value is within (-eps,eps) range
template <typename float_t> inline bool olx_is_zero(float_t v, float_t eps)  {
  return v < 0 ? (v > -eps) : (v < eps);
}
// compares float values within an epsilon range, suitable for stable sort in acsending order
template <typename float_t> inline int olx_cmp_float(float_t v1, float_t v2, float_t eps)  {
  const float_t diff = v1 -v2;
  return olx_is_zero(diff, eps) ? 0 : (diff > 0 ? 1 : -1);
}
// returns absolute value of a number
template <typename num> inline num olx_abs(num n)  {
  return n < 0 ? -n : n;
}
// return pow2
template <typename num> inline num olx_sqr(num n) {  return n*n;  } 

template <typename A, typename B>
  inline void olx_set_bit(const bool Set, A &V, const B Bit)  {
    if( Set )  V |= Bit;
    else       V &= ~Bit;
  }
template <typename A, typename B>
  inline void olx_set_true(A &V, const B Bit)  {
    V |= Bit;
  }
template <typename A, typename B>
  inline void olx_set_false(A &V, const B Bit)  {
    V &= ~Bit;
  }

// throws an exception
template <class VC>
  double olx_tetrahedron_volume(const VC& A, const VC& B, const VC& C,const VC& D)  {
    const VC a(A-B), b(C-B);
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    double caS = a.CAngle(b);
    if( olx_abs(caS) > (1.0-1e-15) )  
      return 0;
    const double sa = sqrt(1 - caS*caS);
    caS = a.Length() * b.Length() * sa / 2;
    VC n = a.XProdVec(b);
    double d = n[2]*A[2] + n[1]*A[1] + n[0]*A[0];
    d = n[2]*D[2] + n[1]*D[1] + n[0]*D[0] - d;
    d /= n.Length();
    return olx_abs(caS*d/3);
  }
  // dihedral angle in degrees [0..180], throws an exception
template <class VC>
  double olx_dihedral_angle(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    const VC a((v1-v2).XProdVec(v3-v2)), 
             b((v2-v3).XProdVec(v4-v3)); 
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    return acos(a.CAngle(b))*180/M_PI;
  }
  /* dihedral angle in degrees (-180..180] 
  http://en.wikipedia.org/wiki/Dihedral_angle
  */
template <class VC>
  double olx_dihedral_angle_signed(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    const VC b1(v2-v1), b2(v3-v2), b3(v4-v3),
             b2xb3(b2.XProdVec(b3));
    return atan2(b2.Length()*b1.DotProd(b2xb3), b1.XProdVec(b2).DotProd(b2xb3))*180/M_PI;
  }
  //angle in degrees for three coordinates A-B B-C angle
template <class VC>
  double olx_angle(const VC& v1, const VC& v2, const VC& v3)  {
    const VC a(v1-v2),
             b(v3-v2);
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    return acos(a.CAngle(b))*180.0/M_PI;
  }
  //angle in degrees for four coordinates A-B D-C angle
template <class VC>
  double olx_angle(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    const VC a(v1-v2),
             b(v4-v3);
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    return acos( a.CAngle(b) )*180.0/M_PI;
  }

// greatest common denominator
extern unsigned int olx_gcd(unsigned int u, unsigned int v);

//extern void olx_set_bit( const bool Set, short &V, const short Bit );
//returns volume of a sphere of radius r
inline double olx_sphere_volume(double r)  {  return 4.*M_PI/3.0*r*r*r;  }
//returns radius of a sphere of volume v
inline double olx_sphere_radius(double v)  {  return pow(v*3.0/(4.0*M_PI), 1./3.);  }
// creates a 3D rotation matrix aroung rv vector, provided cosine of the rotation angle

template <typename FloatType, typename MC, typename VC>
MC& olx_create_rotation_matrix_(MC& rm, const VC& rv, FloatType ca)  {
  const FloatType sa = (olx_abs(ca) > 1e-15) ? sqrt(1-ca*ca) : 1;
  const FloatType t = 1-ca;
  rm[0][0] = t*rv[0]*rv[0] + ca;
  rm[0][1] = t*rv[0]*rv[1] + sa*rv[2];
  rm[0][2] = t*rv[0]*rv[2] - sa*rv[1];

  rm[1][0] = t*rv[0]*rv[1] - sa*rv[2];
  rm[1][1] = t*rv[1]*rv[1] + ca;
  rm[1][2] = t*rv[1]*rv[2] + sa*rv[0];

  rm[2][0] = t*rv[0]*rv[2] + sa*rv[1];
  rm[2][1] = t*rv[1]*rv[2] - sa*rv[0];
  rm[2][2] = t*rv[2]*rv[2] + ca;
  return rm;
}
template <typename FloatType, typename MC, typename VC>
MC& olx_create_rotation_matrix_(MC& rm, const VC& rv, FloatType ca, FloatType sa)  {
  const FloatType t = 1-ca;
  rm[0][0] = t*rv[0]*rv[0] + ca;
  rm[0][1] = t*rv[0]*rv[1] + sa*rv[2];
  rm[0][2] = t*rv[0]*rv[2] - sa*rv[1];

  rm[1][0] = t*rv[0]*rv[1] - sa*rv[2];
  rm[1][1] = t*rv[1]*rv[1] + ca;
  rm[1][2] = t*rv[1]*rv[2] + sa*rv[0];

  rm[2][0] = t*rv[0]*rv[2] + sa*rv[1];
  rm[2][1] = t*rv[1]*rv[2] - sa*rv[0];
  rm[2][2] = t*rv[2]*rv[2] + ca;
  return rm;
}

template <typename MC, typename VC>
MC& olx_create_rotation_matrix(MC& rm, const VC& rv, double ca)  {
  return olx_create_rotation_matrix_<double, MC, VC>(rm, rv, ca);
}
template <typename MC, typename VC>
MC& olx_create_rotation_matrix(MC& rm, const VC& rv, double ca, double sa)  {
  return olx_create_rotation_matrix_<double, MC, VC>(rm, rv, ca, sa);
}
template <class MC, class VC> MC& QuaternionToMatrix(const VC& qt, MC& matr)  {
  matr[0][0] = qt[0]*qt[0] + qt[1]*qt[1] - qt[2]*qt[2] - qt[3]*qt[3];
  matr[0][1] = 2*(qt[1]*qt[2] - qt[0]*qt[3]);
  matr[0][2] = 2*(qt[1]*qt[3] + qt[0]*qt[2]);

  matr[1][0] = 2*(qt[1]*qt[2] + qt[0]*qt[3]);
  matr[1][1] = qt[0]*qt[0] + qt[2]*qt[2] - qt[1]*qt[1] - qt[3]*qt[3];
  matr[1][2] = 2*(qt[2]*qt[3] - qt[0]*qt[1]);

  matr[2][0] = 2*(qt[1]*qt[3] - qt[0]*qt[2]);
  matr[2][1] = 2*(qt[2]*qt[3] + qt[0]*qt[1]);
  matr[2][2] = qt[0]*qt[0] + qt[3]*qt[3] - qt[1]*qt[1] - qt[2]*qt[2];
  return matr;
}
/* 
generates a new permutation from the original list 
http://en.wikipedia.org/wiki/Permutation
*/
template <class List> void GeneratePermutation(List& out, size_t perm)  {
  const size_t cnt = out.Count();
  size_t fc = olx_factorial_t<size_t, size_t>(cnt-1);
  for( size_t i=0; i < cnt-1; i++ )  {
    size_t ti = (perm/fc) % (cnt - i);
    size_t tv = out[i+ti];
    for( size_t j = i+ti; j > i; j-- )
      out[j] = out[j-1];
    out[i] = tv;
    fc /= (cnt-i-1);
  }
}

namespace olx_vec  {
  template <class OutT, class VecT, class MatT>
  OutT& MulMatrix(const VecT& v, const MatT& m, OutT& o)  {
    if( v.Count() != m.RowCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    for( size_t i=0; i < m.ColCount(); i++ )  {
      for( size_t j = 0; j < v.Count(); j++ )
        o[i] += v[j]*m.Get(j, i);
    }
    return o;
  }
  template <class OutT, class VecT, class MatT>
  OutT& MulMatrixT(const VecT& v, const MatT& m, OutT& o)  {
    if( v.Count() != m.ColCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    for( size_t i=0; i < m.RowCount(); i++ )  {
      for( size_t j = 0; j < v.Count(); j++ )
        o[i] += v[j]*m.Get(i, j);
    }
    return o;
  }
  template <class OutT, class VecAT, class VecBT>
  OutT& MulVec(const VecAT& v1, const VecBT& v2, OutT& o)  {
    if( v1.Count() != v2.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    for( size_t i = 0; i < v1.Count(); i++ )
      o[i] = v1[i]*v2[i];
    return o;
  }
  template <class VecAT, class VecBT>
  VecAT& MulSelfVec(VecAT& v1, const VecBT& v2)  {
    if( v1.Count() != v2.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    for( size_t i = 0; i < v1.Count(); i++ )
      v1[i] *= v2[i];
    return v1;
  }
};  // end namespace olx_vec
namespace olx_mat  {
  template <class OutT, class MatAT, class MatBT>
  OutT& MulMatrix(const MatAT& m, const MatBT& n, OutT& o)  {
    if( m.ColCount() != m.RowCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    for( size_t i=0; i < m.RowCount(); i++ )  {
      for( size_t j = 0; j < n.ColCount(); j++ )
        for( size_t k = 0; k < m.ColCount(); k++ )
          o.Set(i, j, o.Get(i, j) + m.Get(i, k)*n.Get(k, j));
    }
    return o;
  }
  //http://en.wikipedia.org/wiki/Kronecker_product
  template <class InM, class OutM> OutM& KroneckerProduct(InM& a, InM& b, OutM& o)  {
    if( a.ColCount()*b.ColCount() > o.ColCount() || a.RowCount()*b.RowCount() > o.RowCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "output size");
    for( size_t ai=0; ai < a.RowCount(); ai++ )  {
      const size_t oi_off = ai*b.RowCount();
      for( size_t aj=0; aj < a.ColCount(); aj++ )  {
        const size_t oj_off = aj*b.ColCount();
        for( size_t bi=0; bi < b.RowCount(); bi++ )  {
          for( size_t bj=0; bj < b.ColCount(); bj++ )  {
            o.Set(oi_off+bi, oj_off+bj, a.Get(ai, aj)*b.Get(bi, bj));
          }
        }
      }
    }
    return o;
  }
  template <class InM, class OutM> OutM KroneckerProduct(InM& a, InM& b)  {
    OutM o(a.RowCount()*b.RowCount(), a.ColCount()*b.ColCount());
    return KroneckerProduct(a, b, o);
  }
};  //end of namespace olx_mat

template <typename float_type>
  static void olx_sincos(const float_type ang, float_type *sina, float_type *cosa)  {
#if defined(__WIN32__) && !defined(_WIN64) && !defined(__GNUC__) && !defined(__BORLANDC__)
  _asm  {
    FLD  ang
    FSINCOS
    MOV EAX, [cosa]
    FSTP  QWORD PTR [EAX]    // cosine
    MOV EAX, [sina]
    FSTP  QWORD PTR [EAX]    // sine
    FWAIT
  }
#else
  *sina = sin(ang);
  *cosa = cos(ang);
#endif
}

template <typename src_t, typename dest_t>
void olx_update_min_max(const src_t& src, dest_t& min_v, dest_t& max_v)  {
  if( src < min_v )  min_v = src;
  if( src > max_v )  max_v = src;
}


template <class list_t>
typename list_t::list_item_type olx_sum(const list_t &l,
  size_t from=0, size_t to=InvalidIndex)
{
  typename list_t::list_item_type rv(0);
  if (to == InvalidIndex) to = l.Count();
  for (size_t i=from; i < to; i++) rv += l[i];
  return rv;
}
template <class list_t, class accessor_t>
static typename accessor_t::return_type olx_sum(
  const list_t &l, const accessor_t &accessor,
  size_t from=0, size_t to=InvalidIndex)
{
  typename accessor_t::return_type rv(0);
  if (to == InvalidIndex) to = l.Count();
  for (size_t i=0; i < to; i++) rv += accessor(l[i]);
  return rv;
}

template <class list_t>
typename list_t::list_item_type olx_mean(const list_t &l,
  size_t from=0, size_t to=InvalidIndex)
{
  if (to == InvalidIndex) to = l.Count();
  typename list_t::list_item_type rv = olx_sum(l, from, to);
  size_t cnt = to-from;
  return cnt == 0 ? rv : rv/cnt;
}

template <class list_t, class accessor_t>
static typename accessor_t::return_type olx_mean(
  const list_t &l, const accessor_t &accessor,
  size_t from=0, size_t to=InvalidIndex)
{
  if (to == InvalidIndex) to = l.Count();
  typename accessor_t::return_type rv = olx_sum(l, accessor, from, to);
  size_t cnt = to-from;
  return cnt == 0 ? rv : rv/cnt;
}

EndEsdlNamespace()
#endif

//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef eSimpleLeastSquaresH
#define eSimpleLeastSquaresH
#include "ematrix.h"
#include "evector.h"
//#include "ematrix.h"

BeginEsdlNamespace()
class TLS {
public:
  typedef double (*OneArgFunc)(double);
  typedef double (*TwoArgFunc)(double, double);
  typedef double (*ThreeArgFunc)(double, double, double);
  typedef double (*VecArgFunc)(const TVectorD&);

  /*data is matrix [x y], where y = func(x), dervs is the lis of partial derivatives
    by all parameters; func is the function; The return value is R factor of the form:
       Sum( |Y-Ycalc| )/ SQRT( Sum( Y^2 ) ); Thefunction must use the solution to calculate
       the values
   */
  static double LS1D(const TMatrixD& data, TVectorD& solutions,
              const TTypeList<OneArgFunc>& dervs,
              OneArgFunc func)  {
    TMatrixD a( dervs.Count(), dervs.Count() );
    TVectorD r( dervs.Count() ), increments(dervs.Count());
    for(int j=0; j < dervs.Count(); j++ )  {
      for( int i=0; i< dervs.Count(); i++ )  {
        for( int k=0; k < data.Vectors();k++ )
          a[i][j] += dervs[i]( data[k][0] ) * dervs[j]( data[k][0] );
     }
     for(int k=0; k < data.Vectors(); k++ )
       r[j] += dervs[j](data[k][0]) * ( data[k][1]-func(data[k][0]) );
    }
    TMatrixD::GauseSolve(a, r, increments);
    for( int i=0; i < dervs.Count(); i++ )
      solutions += increments[i];

    double sdiff = 0, sval = 0;
    for(int i=0; i < data.Elements(); i++ ) {
      double v = fabs(data[i][1] - func( data[i][0] ) );  // |Y - Ycalc|, f depends on colutions
      sdiff += v*v;
      sval += (data[i][1] * data[i][1]);  // Y^2
    }
    return sdiff/sqrt(sval);
  }
  /*data is matrix [x y z], where z = func(x, y), dervs is the lis of partial derivatives
    by all parameters; func is the function; The return value is R factor of the form:
       Sum( |Z-Zcalc| )/ SQRT( Sum( Z^2 ) ); Thefunction must use the solution to calculate
       the values
   */
  static double LS2D(const TMatrixD& data, TVectorD& solutions,
              const TTypeList<TwoArgFunc>& dervs,
              TwoArgFunc func)  {
    TMatrixD a( dervs.Count(), dervs.Count() );
    TVectorD r( dervs.Count() ), increments(dervs.Count());
    for(int j=0; j < dervs.Count(); j++ )  {
      for( int i=0; i< dervs.Count(); i++ )  {
        for( int k=0; k< data.Vectors();k++ )
          a[i][j] += dervs[i]( data[k][0], data[k][1] ) * dervs[j]( data[k][0], data[k][1] );
     }
     for(int k=0; k < data.Vectors(); k++ )
       r[j] += dervs[j](data[k][0], data[k][1]) * ( data[k][2]-func(data[k][0], data[k][1]) );
    }
    TMatrixD::GauseSolve(a, r, increments);
    for( int i=0; i < dervs.Count(); i++ )
      solutions += increments[i];

    double sdiff = 0, sval = 0;
    for(int i=0; i < data.Vectors(); i++ )  {
      double v = fabs(data[i][2] - func( data[i][0], data[i][1] ) );  // |Y - Ycalc|, f depends on colutions
      sdiff = v*v;
      sval += (data[i][2] * data[i][2]);  // Y^2
    }
    return sdiff/sqrt(sval);
  }
  /*data is matrix [x y z v], where v = func(x, y, z), dervs is the list of partial derivatives
    by all parameters; func is the function; The return value is R factor of the form:
       Sum( |V-Vcalc| )/ SQRT( Sum( V^2 ) ); Thefunction must use the solution to calculate
       the values
   */
  static double LS3D(const TMatrixD& data, TVectorD& solutions,
              const TTypeList<ThreeArgFunc>& dervs,
              ThreeArgFunc func)  {
    TMatrixD a( dervs.Count(), dervs.Count() );
    TVectorD r( dervs.Count() ), increments(dervs.Count());
    for(int j=0; j < dervs.Count(); j++ )  {
      for( int i=0; i< dervs.Count(); i++ )  {
        for( int k=0; k< data.Vectors(); k++ )  {
          a[i][j] += dervs[i]( data[k][0], data[k][1], data[k][2] ) * dervs[j]( data[k][0], data[k][1], data[k][2] );
        }
     }
     for(int k=0; k < data.Vectors(); k++ )
       r[j] += dervs[j](data[k][0], data[k][1], data[k][2]) * ( data[k][3]-func(data[k][0], data[k][1], data[k][2]) );
    }
    TMatrixD::GauseSolve(a, r, increments);
    for( int i=0; i < dervs.Count(); i++ )
      solutions += increments[i];

    double sdiff = 0, sval = 0;
    for(int i=0; i < data.Vectors(); i++ )  {
      double v = fabs(data[i][3] - func( data[i][0], data[i][1], data[i][2] ) );  // |Y - Ycalc|, f depends on colutions
      sdiff += v*v;
      sval += (data[i][3] * data[i][3]);  // Y^2
    }
    return sdiff/sqrt(sval);
  }
  /*data is matrix [a1 a2 .. an v], where v = func(a1, a2, ..., an), dervs is the list of partial derivatives
    by all parameters; func is the function; The return value is R factor of the form:
       Sum( |V-Vcalc| )/ SQRT( Sum( V^2 ) ); Thefunction must use the solution to calculate
       the values
   */
  static double LSND(const TMatrixD& data, TVectorD& solutions,
              const TTypeList<VecArgFunc>& dervs,
              VecArgFunc func)  {
    TMatrixD a( dervs.Count(), dervs.Count() );
    TVectorD r( dervs.Count() ), increments(dervs.Count());
    for(int j=0; j < dervs.Count(); j++ )  {
      for( int i=0; i< dervs.Count(); i++ )  {
        for( int k=0; k< data.Vectors(); k++ )  {
          a[i][j] += dervs[i]( data[k] ) * dervs[j]( data[k] );
        }
     }
     for(int k=0; k < data.Vectors(); k++ )
       r[j] += dervs[j](data[k]) * ( data[k][data.Elements()-1]-func(data[k]) );
    }
    TMatrixD::GauseSolve(a, r, increments);
    for( int i=0; i < dervs.Count(); i++ )
      solutions[i] = increments[i];

    double sdiff = 0, sval = 0;
    for(int i=0; i < data.Vectors(); i++ )  {
      double v = fabs(data[i][data.Elements()-1] - func( data[i] ) );  // |Y - Ycalc|, f depends on colutions
      sdiff += v*v;
      sval += (data[i][data.Elements()-1] * data[i][data.Elements()-1]);  // Y^2
    }
    if( sval != 0 )
      return sdiff/sqrt(sval);
    return sdiff;
  }
};


EndEsdlNamespace()
#endif

#ifndef olx_xlib_symmparser_H
#define olx_xlib_symmparser_H
#include "xbase.h"
#include "symmat.h"
#include "emath.h"
#include "testsuit.h"

BeginXlibNamespace()

class TSymmParser  {
  // compares p with values in array axes. Used in SymmToMatrix function
  static short IsAxis(const olxstr& p) {
    if( p.IsEmpty() )  return -1;
    for( int i=0; i < 3; i++ )
      if( Axis[i] == p.CharAt(0) )  
        return i;
    return -1;
  }
  
  template <typename vc>  static vc& ExtractTranslation(const olxstr& str, vc& t)  {
    if( str.Length() == 3 )  {
      t[0] += (int)(str.CharAt(0)-'5');
      t[1] += (int)(str.CharAt(1)-'5');
      t[2] += (int)(str.CharAt(2)-'5');
    }
    else if( str.Length() == 6 ) {
      t[0] += (str.SubString(0, 2).ToInt()-50);
      t[1] += (str.SubString(2, 2).ToInt()-50);
      t[2] += (str.SubString(4, 2).ToInt()-50);
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong translation code: ") << str);
    return t;
  }
  static olxstr FormatFloatEx(double f)  {
    olxstr rv;
    if( f < 0 )  rv << '-';
    int v = olx_abs(olx_round(f*12)), base = 12;
    int denom = esdl::gcd(v, base);
    if( denom != 1 )  {
      v /= denom;
      base /= denom;
    }
    if( base == 1 )  return rv << v;
    else             return rv << v << '/' << base;
  }
  template <class SM> static olxstr _MatrixToSymm(const SM& M, bool fraction)  {
    olxstr T, T1;
    for( int j=0; j < 3; j ++ )  {
      if( j != 0 )
        T << ',';
      for( int i=0; i < 3; i ++ )  {
        if( i == j )  {
          if( M.r[j][i] != 0 )  {
            T1 << olx_sign_char(M.r[j][i]);
            T1 << Axis[j];
          }
          continue;
        }
        if( M.r[j][i] != 0 )  {
          T1.Insert(Axis[i], 0);
          T1.Insert(olx_sign_char(M.r[j][i]), 0);
        }
      }
      if( M.t[j] != 0 )  {
        if( !fraction )
          T1.Insert(olxstr::FormatFloat(3, M.t[j], false).TrimFloat(), 0);
        else
          T1.Insert(FormatFloatEx(M.t[j]), 0);
      }
      T << T1;
      T1 = EmptyString;
    }
    return T;
  }
  // can take both TAsymmUnit or TUnitCell
  template <class SymSpace>
  static smatd _SymmCodeToMatrix(const SymSpace& sp, const olxstr& Code, size_t* index=NULL)  {
    TStrList Toks(Code, '_');
    smatd mSymm;
    if( Toks.Count() == 1 )  {  // standard XP symm code like 3444
      if( Toks[0].Length() >= 4 )  {
        Toks.Add( Toks[0].SubStringFrom(Toks[0].Length()-3) );
        Toks[0].SetLength(Toks[0].Length()-3);
      }
      else  {
        size_t isymm = Toks[0].ToSizeT()-1;
        if( isymm >= sp.Count() )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
        return sp[isymm];
      }
    }
    if( Toks.Count() != 2 )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong code: ") << Code);
    const size_t isymm = Toks[0].ToSizeT()-1;
    if( isymm >= sp.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
    mSymm = sp[isymm];
    if( index != NULL )  *index = isymm;
    vec3i t;
    ExtractTranslation(Toks[1], t);
    mSymm.t += t;
    mSymm.SetRawId(smatd::GenerateId((uint8_t)isymm, t));
    return mSymm;
  }
  
  static const char Axis[];
public:
    // Transforms matrix to standard SYMM operation (INS, CIF files)
  static olxstr MatrixToSymm(const smatd& M)  {
    return _MatrixToSymm(M, false);
  }
    // Transforms matrix to standard SYMM operation (INS, CIF files), using fractions of 12 for translations
  static olxstr MatrixToSymmEx(const smatd& M)  {
    return _MatrixToSymm(M, true);
  }
    // Transforms matrix to standard SYMM operation (INS, CIF files), using fractions of 12 for translations
  static olxstr MatrixToSymmEx(const mat3i& M);
    // Transforms standard SYMM operation (INS, CIF files) to matrix
  static bool SymmToMatrix(const olxstr& symm, smatd& M);
  // return a matrix representation of 1_555 or 1_505050 code for the unit cell
  static smatd SymmCodeToMatrix(const smatd_list& ml, const olxstr& Code)  {
    size_t index = InvalidIndex;
    smatd rv = _SymmCodeToMatrix(ml, Code, &index);
    if( index != InvalidIndex )
      rv.SetId(smatd::GenerateId((uint8_t)index, rv, ml[index]));
    return rv;
  }
  template <class SymSpaceOwner>
  static smatd SymmCodeToMatrix(const SymSpaceOwner& u, const olxstr& Code)  {
    return _SymmCodeToMatrix(u.GetSymSpace(), Code);
  }
  /* return a string representation of a matrix like 1_555 or 1_505050 code in dependence on
  the length of translations; Matrix->ContainerId must be set to the corerct index in the container!!! */
  template <class SymSpace>
  static olxstr MatrixToSymmCode(const SymSpace& sp, const smatd& M)  {
    vec3i Trans(sp[M.GetContainerId()].t - M.t);
    int baseVal = 5;
    if( (olx_abs(Trans[0]) > 4) || (olx_abs(Trans[1]) > 4) || (olx_abs(Trans[1]) > 4) )
      baseVal = 50;
    static char bf[64];
#ifdef _MSC_VER
    sprintf_s(bf, 64, "%i_%i%i%i", M.GetContainerId()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#else
    sprintf(bf, "%i_%i%i%i", M.GetContainerId()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#endif
    return olxstr(bf);
  }
  /*checks if the given string represents a symmetry operation (1_554, 1554, 1505149, x,y,z) 
  works as a combination of the two following functions */
  static bool IsSymm(const olxstr& s);
  //checks if the given string represents a symmetry operation (1_554, 1554, 1505149)
  static bool IsRelSymm(const olxstr& s);
  //checks if the given string represents a symmetry operation (x,y,z)
  static bool IsAbsSymm(const olxstr& s);
  // runs various tests...
  static void Tests(OlxTests& t);
};


EndXlibNamespace()
#endif


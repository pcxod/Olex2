/* Structure factor utilities 
 (c) O Dolomanov, 2008
*/
#ifndef __structure_factor_h
#define __structure_factor_h

#include "xapp.h"
#include "emath.h"
#include "fastsymm.h"
#include "symmlib.h"
#include "chemdata.h"
#include "olxmps.h"

BeginXlibNamespace()

namespace SFUtil {

  static const short mapTypeDiff = 0,  // map type
    mapTypeObs  = 1,
    mapTypeCalc = 2,
    mapType2OmC = 3;
  static const short scaleSimple = 0,  // scale for difference map
    scaleRegression = 1;
  static const short sfOriginFcf = 0,  // structure factor origin
    sfOriginOlex2 = 1;
  static const double T_PI = M_PI*2;
  static const double MT_PI = -M_PI*2;
  const static double EQ_PI = 8*M_PI*M_PI;
  const static double TQ_PI = 2*M_PI*M_PI;
  
  static inline double GetReflectionF(const TReflection* r) {  return r->GetI() <= 0 ? 0 : sqrt(r->GetI());  }
  static inline double GetReflectionF(const TReflection& r) {  return r.GetI() <=0 ? 0 : sqrt(r.GetI());  }
  static inline double GetReflectionF(const double& r) {  return r;  }
  static inline double GetReflectionF2(const TReflection* r) {  return r->GetI();  }
  static inline double GetReflectionF2(const TReflection& r) {  return r.GetI();  }
  static inline double GetReflectionF2(const double& r) {  return r;  }
  static inline const vec3i& GetReflectionHkl(const TReflection* r) {  return r->GetHkl();  }
  static inline const vec3i& GetReflectionHkl(const TReflection& r) {  return r.GetHkl();  }
  static inline const vec3i& GetReflectionHkl(const vec3i& r) {  return r;  }

  struct StructureFactor  {
    vec3i hkl;  // hkl indexes
    double ps;  // phase shift
    compd val; // value
  };
  // interface description...
  class ISF_Util {
  public:
    virtual ~ISF_Util()  {}
    // expands indexes to P1
    virtual void Expand(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, TArrayList<StructureFactor>& out) const = 0;
    /* atoms[i]->Tag() must be index of the corresponding scatterer. U has 6 elements of Ucif or Uiso for each 
    atom  */
    virtual void Calculate(double eV, const TRefList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
      const ElementPList& scatterers, const TCAtomPList& atoms, const double* U, bool useFpFdp) const = 0;
    virtual void Calculate(double eV, const TRefPList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
      const ElementPList& scatterers, const TCAtomPList& atoms, const double* U, bool useFpFdp) const = 0;
    virtual size_t GetSGOrder() const = 0;
  };


  // for internal use
  void PrepareCalcSF(const TAsymmUnit& au, double* U, ElementPList& scatterers, TCAtomPList& alist); 
  /* calculates the scale sum(Fc)/sum(Fo) Fc = k*Fo. Can accept a list of doubles (Fo) */
  template <class RefList>
  static double CalcFScale(const TArrayList<compd>& F, const RefList& refs)  {
    double sF2o = 0, sF2c = 0;
    const size_t f_cnt = F.Count();
    for( size_t i=0; i < f_cnt; i++ )  {
      sF2o += GetReflectionF(refs[i]);
      sF2c += F[i].mod();
    }
    return sF2c/sF2o;
  }
  /* calculates the scale sum(Fc^2)/sum(Fo^2) Fc^2 = k*Fo^2. Can accept a list of doubles (Fo^2) */
  template <class RefList> double CalcF2Scale(const TArrayList<compd>& F, const RefList& refs)  {
    double sF2o = 0, sF2c = 0;
    const size_t f_cnt = F.Count();
    for( size_t i=0; i < f_cnt; i++ )  {
      sF2o += GetReflectionF2(refs[i]);
      sF2c += F[i].qmod();
    }
    return sF2c/sF2o;
  }
  /* calculates a best line scale : Fc = k*Fo + a. Can accept a list of doubles (Fo) */
  template <class RefList> void CalcFScale(const TArrayList<compd>& F, const RefList& refs, double& k, double& a)  {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const size_t f_cnt = F.Count();
    for( size_t i=0; i < f_cnt; i++ )  {
      const double I = GetReflectionF(refs[i]);
      const double qm = F[i].mod();
      sx += I;
      sy += qm;
      sxy += I*qm;
      sxs += I*I;
    }
    k = (sxy - sx*sy/f_cnt)/(sxs - sx*sx/f_cnt);
    a = (sy - k*sx)/f_cnt;
  }
  /* calculates a best line scale : Fc^2 = k*Fo^2 + a. Can accept a list of doubles (Fo^2) */
  template <class RefList> void CalcF2Scale(const TArrayList<compd>& F, const RefList& refs, double& k, double& a)  {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const size_t f_cnt = F.Count();
    for( size_t i=0; i < f_cnt; i++ )  {
      const double I = GetReflectionF2(refs[i]);
      const double qm = F[i].qmod();
      sx += I;
      sy += qm;
      sxy += I*qm;
      sxs += I*I;
    }
    k = (sxy - sx*sy/f_cnt)/(sxs - sx*sx/f_cnt);
    a = (sy - k*sx)/f_cnt;
  }
  // expands structure factors to P1 for given space group
  void ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, const TSpaceGroup& sg, TArrayList<StructureFactor>& out);
  template <class RefList, class MatList>
  void ExpandToP1(const RefList& hkl_list, const TArrayList<compd>& F, const MatList& ml,
    TArrayList<StructureFactor>& out)
  {
    const size_t ml_cnt = ml.Count();
    out.SetCount(ml_cnt* hkl_list.Count());
    for( size_t i=0; i < hkl_list.Count(); i++ )  {
      const size_t off = i*ml_cnt;
      for( size_t j=0; j < ml_cnt; j++ )  {
        const smatd& m = ml[j];
        const size_t ind = off+j;
        const vec3i& hkl = GetReflectionHkl(hkl_list[i]);
        out[ind].hkl = hkl*m.r;
        out[ind].ps = m.t.DotProd(hkl);
        if( out[ind].ps != 0 )  {
          double ca=1, sa=0;
          SinCos(-T_PI*out[ind].ps, &sa, &ca);
          out[ind].val = F[i]*compd(ca,sa);
        }
        else
          out[ind].val = F[i];
      }  
    }
  }
  // find minimum and maximum values of the miller indexes of the structure factor
  void FindMinMax(const TArrayList<StructureFactor>& F, vec3i& min, vec3i& max);
  // prepares the list of hkl and structure factors, return error message or empty string
  olxstr GetSF(TRefList& refs, TArrayList<compd>& F, 
    short mapType, short sfOrigin = sfOriginOlex2, short scaleType = scaleSimple);
  // calculates the structure factors for given reflections
  void CalcSF(const TXFile& xfile, const TRefList& refs, TArrayList<compd>& F, bool useFpFdp);
  // calculates the structure factors for given reflections
  void CalcSF(const TXFile& xfile, const TRefPList& refs, TArrayList<compd>& F, bool useFpFdp);
  // returns an instance according to __OLX_USE_FASTSYMM, must be deleted with delete
  ISF_Util* GetSF_Util_Instance(const TSpaceGroup& sg);

#ifdef __OLX_USE_FASTSYMM
  template <class sg> class SF_Util : public ISF_Util {
#else
  struct SG_Impl  {
    const size_t size;
    smatd_list matrices;
    SG_Impl(const smatd_list& ml) : size(ml.Count()), matrices(ml)  {}
    void GenHkl(const vec3i& hkl, TArrayList<vec3i>& out, TArrayList<double>& ps) const {
      for( size_t i=0; i < size; i++ )  {
        out[i] = hkl*matrices[i].r;
        ps[i] = matrices[i].t.DotProd(hkl);
      }
    }
  };
  template <class sg> class SF_Util : public ISF_Util, public sg {
#endif
  protected:
    // proxying functions
    inline size_t _getsize() const {  return sg::size;  }
    inline void _generate(const vec3i& hkl, TArrayList<vec3i>& out, TArrayList<double>& ps) const {
      sg::GenHkl(hkl, out, ps);
    }
    template <class RefList, bool use_fpfdp> struct SFCalculateTask  {
      const RefList& refs;
      const mat3d& hkl2c;
      TArrayList<compd>& F;
      TArrayList<compd> *fpfdp, fo;
      TArrayList<vec3i> rv;
      TArrayList<double> ps;
      const ElementPList& scatterers;
      const TCAtomPList& atoms;
      const double* U;
      const SF_Util& parent;
      const double eV;
      SFCalculateTask(const SF_Util& _parent, double _eV, const RefList& _refs, const mat3d& _hkl2c,
        TArrayList<compd>& _F, const ElementPList& _scatterers,
        const TCAtomPList& _atoms, const double* _U) :
        parent(_parent), eV(_eV), refs(_refs), hkl2c(_hkl2c), F(_F), scatterers(_scatterers),
        atoms(_atoms), U(_U), rv(_parent._getsize()), ps(_parent._getsize()), fo(_scatterers.Count()),
        fpfdp(NULL)
      {
        if( use_fpfdp )  {
          fpfdp = new TArrayList<compd>(scatterers.Count());
          for( size_t i=0; i < scatterers.Count(); i++ )  {
            (*fpfdp)[i] = scatterers[i]->CalcFpFdp(eV);
            (*fpfdp)[i] -= scatterers[i]->z;
          }
        }
      }
      virtual ~SFCalculateTask()  {
        if( use_fpfdp )
          delete fpfdp;
      }
      void Run(size_t i)  {
        const TReflection& ref = TReflection::GetRef(refs[i]);
        const double d_s2 = ref.ToCart(hkl2c).QLength()*0.25;
        parent._generate(ref.GetHkl(), rv, ps);
        for( size_t j=0; j < scatterers.Count(); j++)  {
          fo[j] = scatterers[j]->gaussians->calc_sq(d_s2);
          if( use_fpfdp )
            fo[j] += (*fpfdp)[j];
        }
        compd ir;
        for( size_t j=0; j < atoms.Count(); j++ )  {
          compd l;
          for( size_t k=0; k < parent._getsize(); k++ )  {
            double tv =  SFUtil::T_PI*(atoms[j]->ccrd().DotProd(rv[k])+ps[k]);  // scattering vector + phase shift
            double ca, sa;
            SinCos(tv, &sa, &ca);
            if( olx_is_valid_index(atoms[j]->GetEllpId()) )  {
              const double* Q = &U[j*6];  // pick up the correct ellipsoid
              const double B = exp(
                (Q[0]*rv[k][0]+Q[4]*rv[k][2]+Q[5]*rv[k][1])*rv[k][0] + 
                (Q[1]*rv[k][1]+Q[3]*rv[k][2])*rv[k][1] + 
                (Q[2]*rv[k][2])*rv[k][2] );
              l.Re() += ca*B;
              l.Im() += sa*B;
            }
            else  {
              l.Re() += ca;
              l.Im() += sa;
            }
          }
          compd scv = fo[atoms[j]->GetTag()];
          if( !olx_is_valid_index(atoms[j]->GetEllpId()) )
            scv *= exp(U[j*6]*d_s2);
          scv *= atoms[j]->GetOccu();
          scv *= l;
          ir += scv;
        }
        F[i] = ir;
      }
      SFCalculateTask* Replicate() const {
        return new SFCalculateTask(parent, eV, refs, hkl2c, F, scatterers, atoms, U);
      }
    };
  public:
#ifndef __OLX_USE_FASTSYMM
    SF_Util(const smatd_list& ml) : sg(ml)  {}
#endif
    virtual void Expand(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, TArrayList<SFUtil::StructureFactor>& out) const {
      TArrayList<vec3i> rv(sg::size);
      TArrayList<double> ps(sg::size);
      const size_t hkl_cnt = hkl.Count();
      for( size_t i=0; i < hkl_cnt; i++ )  {
        sg::GenHkl(hkl[i], rv, ps);
        const size_t off = i*sg::size;
        for( size_t j=0; j < sg::size; j++ )  {
          const size_t ind = j+off;
          out[ind].hkl = rv[j];
          out[ind].ps = ps[j];
          double ca = 1, sa = 0;
          if( ps[j] != 0 )  {
            SinCos(-SFUtil::T_PI*ps[j], &sa, &ca);
            out[ind].val = F[i]*compd(ca, sa);
          }
          else
            out[ind].val = F[i];
        }
      }
    }
    virtual void Calculate( double eV, const TRefList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
                            const ElementPList& scatterers, const TCAtomPList& atoms, 
                            const double* U, bool useFpFdp) const 
    {
      if( useFpFdp )  {
        SFCalculateTask<TRefList, true> task(*this, eV, refs, hkl2c, F, scatterers, atoms, U);
        TListIteratorManager<SFCalculateTask<TRefList, true> > tasks(task, refs.Count(), tLinearTask, 50);
      }
      else  {
        SFCalculateTask<TRefList, false> task(*this, eV, refs, hkl2c, F, scatterers, atoms, U);
        TListIteratorManager<SFCalculateTask<TRefList, false> > tasks(task, refs.Count(), tLinearTask, 50);
      }
    }
    virtual void Calculate( double eV, const TRefPList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
                            const ElementPList& scatterers, const TCAtomPList& atoms, 
                            const double* U, bool useFpFdp) const 
    {
      if( useFpFdp )  {
        SFCalculateTask<TRefPList, true> task(*this, eV, refs, hkl2c, F, scatterers, atoms, U);
        TListIteratorManager<SFCalculateTask<TRefPList, true> > tasks(task, refs.Count(), tLinearTask, 50);
      }
      else  {
        SFCalculateTask<TRefPList, false> task(*this, eV, refs, hkl2c, F, scatterers, atoms, U);
        TListIteratorManager<SFCalculateTask<TRefPList, false> > tasks(task, refs.Count(), tLinearTask, 50);
      }
    }
    virtual size_t GetSGOrder() const {  return sg::size;  }
  };

}; // SFUtil namespace

EndXlibNamespace()
#endif

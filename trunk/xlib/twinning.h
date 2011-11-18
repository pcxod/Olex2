/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_twinning_H
#define __olx_xlib_twinning_H
#include "reflection.h"
#include "refmodel.h"
#include "refutil.h"

BeginXlibNamespace()
namespace twinning  {

  struct twin_mate_calc {
    compd fc;
    double scale;
    twin_mate_calc(const compd& _fc, double _scale) : fc(_fc), scale(_scale)  {}
    twin_mate_calc() : scale(0)  {}
    double f_sq_calc() const {  return fc.qmod()*scale;  }
  };
  struct twin_mate_obs {
    double f_obs_sq, sig_obs, scale;
    twin_mate_obs(double _f_obs_sq, double _sig_obs, double _scale)
      : f_obs_sq(_f_obs_sq), sig_obs(_sig_obs), scale(_scale)  {}
    twin_mate_obs() : f_obs_sq(0), sig_obs(0), scale(0)  {}
  };
  struct detwin_result {
    double f_sq, sig;
    detwin_result() : f_sq(0), sig(0)  {} 
    detwin_result(double _f_sq, double _sig) : f_sq(_f_sq), sig(_sig) {}
  };
  struct twin_mate_full : public twin_mate_calc, public detwin_result {
    twin_mate_full(const compd& _fc, double _f_sq, double _sig, double _scale)
      : twin_mate_calc(_fc, _scale), detwin_result(_f_sq, _sig)  {}
    twin_mate_full() {}
  };
  // uses only Fc
  struct detwinner_shelx  {
    template <typename twin_generator_t>
    static detwin_result detwin(const twin_generator_t& itr)  {
      twin_mate_full pr = itr.NextFull();
      double sum_f_sq=0;
      while( itr.HasNext() )
        sum_f_sq += itr.NextCalc().f_sq_calc();
      double s = pr.fc.qmod();
      s = s/(s*pr.scale+sum_f_sq);
      return detwin_result(pr.f_sq*s, pr.sig*s);
    }
  };
  // uses both Fc and F_obs
  struct detwinner_mixed  {
    template <typename twin_generator_t>
    static detwin_result detwin(const twin_generator_t& itr)  {
      TTypeList<twin_mate_full> all;
      while( itr.HasNext() )
        all.AddCopy(itr.NextFull());
      double f_sq = 0, s_sq = 0;
      for( size_t i=0; i < all.Count(); i++ )  {
        double dn = 0;
        size_t s = i;
        for( size_t j=0; j < all.Count(); j++, s++ )  {
          size_t ind = (s >= all.Count() ? s - all.Count() : s);
          dn += all[j].fc.qmod()*all[ind].scale;
        }
        double coeff = all[i].scale*all[0].fc.qmod()/dn;
        f_sq += coeff*all[i].f_sq;
        s_sq += coeff*olx_sqr(all[i].sig);
      }
      return detwin_result(f_sq, sqrt(s_sq));
    }
  };
  // uses only scales and Fobs to deconvolute the intensities into components
  struct detwinner_algebraic  {
    ematd _m;
    template <typename list_t> detwinner_algebraic(const list_t& scales)
      : _m(scales.Count(), scales.Count())
    {
      for( size_t i=0; i < scales.Count(); i++ )  {
        size_t s = i;
        for( size_t j=0; j < scales.Count(); j++, s++ )
          _m[i][s >= scales.Count() ? s-scales.Count(): s] = scales[j];
      }
      if( !math::LU<double>::Invert(_m) )
        throw TFunctionFailedException(__OlxSourceInfo, "cannot invert the matrix");
    }
    template <typename twin_generator_t>
    void detwin(const twin_generator_t& itr, TTypeList<TReflection>& res) const {
      TTypeList<TReflection> all;
      evecd I(_m.ColCount()), S(_m.ColCount());
      while( itr.HasNext() )  {
        TReflection& r = all.AddCopy(itr.NextObs());
        const size_t si = olx_abs(r.GetBatch())-1;
        if( si >= _m.ColCount() )
          throw TInvalidArgumentException(__OlxSourceInfo, "batch number");
        I[si] = r.GetI();
        S[si] = olx_sqr(r.GetS());
      }
      I = _m*I;
      S = _m*S;
      for( size_t i=0; i < all.Count(); i++ )  {
        if( i > 0 && all[i].GetHkl() == all[0].GetHkl() )
          continue;
        TReflection& r = res.AddCopy(all[i]);
        r.SetI(I[i]);
        r.SetS(sqrt(S[i]));
      }
    }
  };

  template <typename twin_iterator> struct twin_mate_generator {
    const twin_iterator& itr;
    const TDoubleList& scales;
    const TArrayList<compd>& Fc;
    twin_mate_generator(const twin_iterator& _itr, const TDoubleList& _scales,
      const TArrayList<compd>& _Fc)
      : itr(_itr), scales(_scales), Fc(_Fc)  {}
    bool HasNext() const {  return itr.HasNext();  }
    twin_mate_full NextFull() const {
      TReflection r = itr.Next();
      if( r.GetTag() < 0 )
        return twin_mate_full();
      if( (size_t)r.GetTag() > Fc.Count() )
        throw TIndexOutOfRangeException(__OlxSourceInfo, r.GetTag(), 0, Fc.Count());
      const size_t bi = olx_abs(r.GetBatch())-1;
      return twin_mate_full(
        Fc[r.GetTag()], r.GetI(), r.GetS(), bi < scales.Count() ? scales[bi] : 0);
    }
    twin_mate_calc NextCalc() const {
      TReflection r = itr.Next();
      if( r.GetTag() < 0 )
        return twin_mate_calc();
      if( (size_t)r.GetTag() > Fc.Count() )
        throw TIndexOutOfRangeException(__OlxSourceInfo, r.GetTag(), 0, Fc.Count());
      const size_t bi = olx_abs(r.GetBatch())-1;
      return twin_mate_calc(Fc[r.GetTag()], bi < scales.Count() ? scales[bi] : 0);
    }
  };

  template <typename twin_iterator> struct obs_twin_mate_generator {
    const TRefList& refs;
    const twin_iterator& itr;
    obs_twin_mate_generator(const twin_iterator& _itr, const TRefList& _refs)
      : itr(_itr), refs(_refs)  {}
    bool HasNext() const {  return itr.HasNext();  }
    TReflection NextObs() const {
      TReflection r = itr.Next();
      const TReflection& _rv = refs[r.GetTag()];
      if( _rv.GetTag() < 0 )
        return TReflection(_rv.GetHkl(), 0, 0, r.GetBatch());
      return TReflection(_rv, r.GetBatch());
    }
  };
  // convinience method
  template <typename twin_calc_generator_t>
  double calc_f_sq(const twin_calc_generator_t& tw)  {
    double res = tw.NextCalc().f_sq_calc();
    while( tw.HasNext() )
      res += tw.NextCalc().f_sq_calc();
    return res;
  }
  
  struct merohedral  {
    struct iterator  {
      const merohedral& parent;
      const size_t src_index;
      mutable int current;
      mutable vec3i index;
      iterator(const merohedral& _parent, size_t _src_index)
        : parent(_parent),
          src_index(_src_index),
          index(parent.all_refs[src_index].GetHkl()),
          current(0) {}
      bool HasNext() const {  return current < olx_abs(parent.n);  }
      bool HasNextUniq() const {
        return current == 0 || 
          (current < olx_abs(parent.n) &&
           index != parent.all_refs[src_index].GetHkl());
      }
      TReflection Next() const {
        int i = current++;
        if( parent.n < 0 && i == olx_abs(parent.n)/2 )
          index = -index;
        TReflection rv = TReflection(parent.all_refs[src_index], index, (i+1)*(i==0 ? 1 : -1));
        rv.SetTag(parent.hkl_to_ref_map.IsInRange(index) ? parent.hkl_to_ref_map(index) : -1);
        index = TReflection::Standardise(parent.matrix*index, parent.sym_info);
        return rv;
      }
    };
    merohedral(const SymmSpace::InfoEx& _sym_info, const TRefList& _all_refs,
      const RefinementModel::HklStat& _ms, const TDoubleList& _scales, const mat3i& tm, int _n)
      : sym_info(_sym_info), all_refs(_all_refs), ms(_ms),
        scales(_scales),
        hkl_to_ref_map(_ms.MinIndexes, _ms.MaxIndexes),
        matrix(tm), n(_n)
    {
      hkl_to_ref_map.FastInitWith(-1);
      for( size_t i=0; i < all_refs.Count(); i++ )  {
        hkl_to_ref_map(all_refs[i].GetHkl()) = i;
        all_refs[i].SetTag(i);
      }
    }
    template <typename detwinner_t>
    void detwin(const detwinner_t& dt, TRefList& out, const TArrayList<compd>& Fc)  {
      out = all_refs;
      for( size_t i=0; i < out.Count(); i++ )  {
        TReflection& r = out[i];
        iterator itr(*this, i);
        detwin_result res = dt.detwin(twin_mate_generator<iterator>(itr, scales, Fc));
        r.SetI(res.f_sq);
        r.SetS(res.sig);
        r.SetBatch(TReflection::NoBatchSet);
      }

    }
    void calc_fsq(const TArrayList<compd>& Fc, evecd& Fsq)  {
      Fsq.Resize(all_refs.Count());
      for( size_t i=0; i < all_refs.Count(); i++ )  {
        Fsq[i] = calc_f_sq(
          twin_mate_generator<iterator>(iterator(*this, i), scales, Fc));
      }
    }
    const SymmSpace::InfoEx& sym_info;
    const TRefList& all_refs;
    RefinementModel::HklStat ms;
    TDoubleList scales;
    TArray3D<size_t> hkl_to_ref_map;
    mat3i matrix;
    int n;
  };
  /**/
  struct general  {
    struct iterator  {
      const general& parent;
      mutable size_t current;
      const size_t off;
      iterator(const general& _parent, size_t start) : parent(_parent), off(start), current(0)  {}
      bool HasNext() const {
        return ( current == 0 ||
          ((off-current) > 0 && parent.all_refs[off-current].GetBatch() < 0));
      }
      TReflection Next() const {
        return parent.all_refs[off-current++];
      }
    };
    general(const SymmSpace::InfoEx& _sym_info, const TRefList& _all_refs,
      const RefUtil::ResolutionAndSigmaFilter& filter, const TDoubleList& _scales)
      : sym_info(_sym_info), all_refs(_all_refs),
        scales(_scales),
        F_indices(NULL)
    {
      vec3i mi(100,100,100), mx = -mi;
      for( size_t i=0; i < all_refs.Count(); i++ )
        vec3i::UpdateMinMax(all_refs[i].GetHkl(), mi, mx);
      vec3i::UpdateMinMax(TReflection::Standardise(mi, sym_info), mi, mx);
      vec3i::UpdateMinMax(TReflection::Standardise(mx, sym_info), mi, mx);
      TArray3D<size_t>& hkl3d = *(F_indices = new TArray3D<size_t>(mi, mx));
      F_indices->FastInitWith(-1);
      reflections.Clear().SetCapacity(all_refs.Count());
      for( size_t i=all_refs.Count()-1; i != InvalidIndex; i-- )  {
        if( all_refs[i].IsOmitted() )  {
          ms.OmittedByUser++;
          continue;
        }
        if( filter.IsOutside(all_refs[i]) )  {
          all_refs[i].SetTag(-1);
          continue;
        }
        vec3i hkl = TReflection::Standardise(all_refs[i].GetHkl(), sym_info);
        if( TReflection::IsAbsent(hkl, sym_info) || filter.IsOmitted(hkl) )  {
          if( all_refs[i].GetBatch() > 0 )  {
            size_t j=i;
            bool all_absent = true;
            while( --j != InvalidIndex && all_refs[j].GetBatch() < 0 )  {
              if( !TReflection::IsAbsent(all_refs[j].GetHkl(), sym_info) &&
                !filter.IsOmitted(TReflection::Standardise(all_refs[j].GetHkl(), sym_info)))
              {
                all_absent = false;
                break;
              }
            }
            if( all_absent )  {
              all_refs[i].SetTag(-1);
              ms.SystematicAbsentcesRemoved++;
              i = j+1;
              continue;
            }
          }
        }
        if( hkl3d(hkl) == InvalidIndex )  {
          all_refs[i].SetTag(hkl3d(hkl) = unique_indices.Count());
          unique_indices.AddCopy(hkl);
        }
        else
          all_refs[i].SetTag(hkl3d(hkl));
        if( all_refs[i].GetBatch() >= 0 )
          reflections.AddCopy(all_refs[i]).SetTag(i);
      }
      reflections.ForEach(RefUtil::ResolutionAndSigmaFilter::IntensityModifier(filter));
    }
    ~general()  {
      if( F_indices != NULL )
        delete F_indices;
    }
    void calc_fsq(const TArrayList<compd>& Fc, evecd& Fsq)  {
      Fsq.Resize(reflections.Count());
      for( size_t i=0; i < reflections.Count(); i++ )  {
        iterator itr(*this, reflections[i].GetTag());
        Fsq[i] = calc_f_sq(twin_mate_generator<iterator>(itr, scales, Fc));
      }
    }

    template <typename detwinner_t>
    void detwin(const detwinner_t& dt, TRefList& out, const TArrayList<compd>& Fc)  {
      out = reflections;
      for( size_t i=0; i < out.Count(); i++ )  {
        TReflection& r = out[i];
        twinning::general::iterator itr(*this, r.GetTag());
        detwin_result res = dt.detwin(twin_mate_generator<iterator>(itr, scales, Fc));
        r.SetI(res.f_sq);
        r.SetS(res.sig);
        r.SetBatch(TReflection::NoBatchSet);
      }
    }
    template <typename detwinner_t, typename merger_t>
    void detwin_and_merge(const detwinner_t& dt, const merger_t& merger, TRefList& out,
      const TArrayList<compd>& Fc, TArrayList<compd>* pF)
    {
      detwin(dt, out, Fc);
      TRefPList to_merge(out, DirectAccessor());
      out.ReleaseAll();
      SymmSpace::InfoEx si = sym_info;
      si.centrosymmetric = true;
      RefMerger::Merge<merger_t>(sym_info, to_merge, out, vec3i_list());
      to_merge.DeleteItems(false);
      if( pF != NULL )  {
        TArrayList<compd>& F = *pF;
        TArray3D<size_t>& hkl3d = *F_indices;
        F.SetCount(out.Count());
        for( size_t i=0; i < out.Count(); i++ )  {
          size_t f_i;
          if( !hkl3d.IsInRange(out[i].GetHkl()) ||
            (f_i = hkl3d(out[i].GetHkl())) == InvalidIndex )
            throw TFunctionFailedException(__OlxSourceInfo, "merging does not match");
          F[i] = Fc[f_i];
        }
      }
    }
    const SymmSpace::InfoEx& sym_info;
    const TRefList& all_refs;
    TRefList reflections;
    vec3i_list unique_indices;
    RefinementModel::HklStat ms;
    TDoubleList scales;
    TArray3D<size_t>* F_indices;
  };
}; //end of the twinning namespace
EndXlibNamespace()
#endif

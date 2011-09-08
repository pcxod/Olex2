/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_align_H
#define __olx_sdl_align_H
#include "../threex3.h"
#include "../ematrix.h"
BeginEsdlNamespace()

namespace align  {

  struct out  {
    // a 4x4 matrix of the quaternions
    ematd quaternions;
    /* root mean square distances, note that if non-unit weights are used, these values
    will differe from the RMSD calculated directly:
    (sum(distance^2*w^2)/sum(w^2)^0.5 vs. (sum(distance^2)/n)^0.5
    */
    evecd rmsd;
    vec3d center_a, center_b;
    out() : quaternions(4,4), rmsd(4)  {}
  };
  struct point  {
    vec3d value;
    double weight;
    point() : weight(1.0)  {}
    point(const vec3d& p, double w=1.0) : value(p), weight(w)  {}
    point(const point& ap) : value(ap.value), weight(ap.weight)  {}
    point& operator = (const point& ap)  {
      value = ap.value;
      weight = ap.weight;
      return *this;
    }
  };
  struct pair  {
    point a, b;
    pair()  {}
    pair(const pair& p) : a(p.a), b(p.b)  {}
    pair(const point& _a, const point& _b) : a(_a), b(_b)  {}
    pair& operator = (const pair& p)  {
      a = p.a;
      b = p.b;
      return *this;
    }
    vec3d GetValueA() const {  return a.value;  }
    double GetWeightA() const {  return a.weight;  }
    vec3d GetValueB() const {  return b.value;  }
    double GetWeightB() const {  return b.weight;  }
  };
  /* finds allignment quaternions for given coordinates and their weights
  the rmsds (and the quaternions) are sorted ascending
  Acta A45 (1989), 208.
  The resulting matrices map {b} set to {a} */
  template <class List> out FindAlignmentQuaternions(const List& pairs)  {
    out ao;
    double swa = 0, swb = 0, sws = 0;
    for( size_t i=0; i < pairs.Count(); i++ )  {
      ao.center_a += pairs[i].GetValueA()*pairs[i].GetWeightA();
      ao.center_b += pairs[i].GetValueB()*pairs[i].GetWeightB();
      swa += pairs[i].GetWeightA();
      swb += pairs[i].GetWeightB();
      sws += pairs[i].GetWeightA()*pairs[i].GetWeightB();
    }
    ao.center_a /= swa;
    ao.center_b /= swb;
    ematd evm(4,4);
    for( size_t i=0; i < pairs.Count(); i++ )  {
      const vec3d v1 = (pairs[i].GetValueA() - ao.center_a)*pairs[i].GetWeightA();
      const vec3d v2 = (pairs[i].GetValueB() - ao.center_b)*pairs[i].GetWeightB();
      const vec3d p = v1+v2;
      const vec3d m = v1-v2;
      evm[0][0] += (m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);
      evm[0][1] += (p[1]*m[2] - m[1]*p[2]);
      evm[0][2] += (m[0]*p[2] - p[0]*m[2]);
      evm[0][3] += (p[0]*m[1] - m[0]*p[1]);
      evm[1][1] += (p[1]*p[1] + p[2]*p[2] + m[0]*m[0]);
      evm[1][2] += (m[0]*m[1] - p[0]*p[1]);
      evm[1][3] += (m[0]*m[2] - p[0]*p[2]);
      evm[2][2] += (p[0]*p[0] + p[2]*p[2] + m[1]*m[1]);
      evm[2][3] += (m[1]*m[2] - p[1]*p[2]);
      evm[3][3] += (p[0]*p[0] + p[1]*p[1] + m[2]*m[2]);
    }
    evm[1][0] = evm[0][1];
    evm[2][0] = evm[0][2];
    evm[2][1] = evm[1][2];
    evm[3][0] = evm[0][3];
    evm[3][1] = evm[1][3];
    evm[3][2] = evm[2][3];
    ematd::EigenValues(evm /= sws, ao.quaternions.I());
    for( int i=0; i < 4; i++ )
      ao.rmsd[i] = (evm[i][i] <= 0 ? 0 : sqrt(evm[i][i]));
    bool changes = true;
    while( changes )  {
      changes = false;
      for( int i=0; i < 3; i++ )  {
        if( ao.rmsd[i+1] < ao.rmsd[i] )  {
          ao.quaternions.SwapRows(i, i+1);
          olx_swap(ao.rmsd[i], ao.rmsd[i+1]);
          changes = true;
        }
      }
    }
    return ao;
  }
  // returns unweighted RMSD
  template <class List> double CalcRMSD(const List& pairs, const align::out& ao)  {
    double rmsd = 0;
    mat3d m;
    QuaternionToMatrix(ao.quaternions[0], m);
    for( size_t i=0; i < pairs.Count(); i++ )
      rmsd += (pairs[i].GetValueA() - ao.center_a).QDistanceTo((pairs[i].GetValueB() - ao.center_b)*m);
    return sqrt(rmsd/pairs.Count());
  }
  // two lists to 'pair' adaptor
  template <class ValueList, class WeightList> struct ListsToPairAdaptor  {
    const ValueList& vlist;
    const WeightList& wlist;
    const size_t count;
    struct pair  {
      const ListsToPairAdaptor& parent;
      size_t index;
      pair(const ListsToPairAdaptor& _parent, size_t i) : parent(_parent), index(i)  {}
      vec3d GetValueA() const {  return parent.vlist[index];  }
      double GetWeightA() const {  return parent.wlist[index];  }
      vec3d GetValueB() const {  return parent.vlist[parent.count+index];  }
      double GetWeightB() const {  return parent.wlist[parent.count+index];  }
    };
    ListsToPairAdaptor(const ValueList& _vlist, const WeightList& _wlist) :
      vlist(_vlist), wlist(_wlist), count(vlist.Count()/2)
    {
      if( wlist.Count() < vlist.Count() )
        throw TInvalidArgumentException(__OlxSourceInfo, "weights list");
      if( (vlist.Count()%2) != 0 )
        throw TInvalidArgumentException(__OlxSourceInfo, "list size");
    }
    pair operator [] (size_t i) const {
      return pair(*this, i);
    }
    size_t Count() const {  return count;  }
  };
};  // end namespace align

EndEsdlNamespace()
#endif

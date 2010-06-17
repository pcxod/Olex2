/* Map utilities 
 (c) O Dolomanov, 2008
*/
#ifndef __map_util_h
#define __map_util_h

#include "xbase.h"
#include "arrays.h"
#include "estack.h"

BeginXlibNamespace()

class MapUtil  {
public:
  struct peak  { 
    uint32_t count;
    vec3s center;
    bool process;
    double summ;
    peak() : process(true), summ(0), count(0)  {}
    peak(size_t _x, size_t _y, size_t _z) : process(true),  
      summ(0), count(0), center(_x, _y, _z) {}
  };
protected:
  template <typename MapT> 
  static void peak_search(MapT*** const data, const vec3s& dim,
    MapT pos_level, const TArray3D<bool>& Mask, TArrayList<peak>& maxima)  
  {
    TStack<vec3i> stack;
    bool*** const mask = Mask.Data;
    const MapT neg_level = -pos_level; 
    for( size_t mc=0; mc < maxima.Count(); mc++ )  {
      peak& peak = maxima[mc];
      if( mask[peak.center[0]][peak.center[1]][peak.center[2]] )  continue;
      stack.Push( peak.center );
      const MapT& ref_val = data[peak.center[0]][peak.center[1]][peak.center[2]];
      vec3d new_cent;
      while( !stack.IsEmpty() )  {
        vec3i cent = stack.Pop();
        vec3i norm_cent = cent;
        peak.count++;
        new_cent += cent;
        for( size_t i=0; i < 3; i++ )  {
          norm_cent[i] = norm_cent[i]%dim[i]; 
          if( norm_cent[i] < 0 )
            norm_cent[i] += (int)dim[i];
        }
        mask[norm_cent[0]][norm_cent[1]][norm_cent[2]] = true;
        peak.summ += data[norm_cent[0]][norm_cent[1]][norm_cent[2]];
        vec3i pt = norm_cent, _pt = cent;
        for( int di=0; di < 3; di++ )  {
          pt[di] = (norm_cent[di]+1)%dim[di];
          if( pt[di] < 0 )  pt[di] += (int)dim[di];
          if( mask[pt[0]][pt[1]][pt[2]] )  continue;
          if( ref_val < 0 )  {
            if( data[pt[0]][pt[1]][pt[2]] < neg_level )  {
              _pt[di] = cent[di]+1;
              stack.Push(_pt);
              mask[pt[0]][pt[1]][pt[2]] = true;
            }
          }
          else if( data[pt[0]][pt[1]][pt[2]] > pos_level )  {
            _pt[di] = cent[di]+1;
            stack.Push(_pt);
            mask[pt[0]][pt[1]][pt[2]] = true;
          }
          pt[di] = (norm_cent[di]-1)%dim[di];
          if( pt[di] < 0 )  pt[di] += (int)dim[di];
          if( mask[pt[0]][pt[1]][pt[2]] )  continue;
          if( ref_val < 0 )  {
            if( data[pt[0]][pt[1]][pt[2]] < neg_level )  {
              _pt[di] = cent[di]-1;
              stack.Push(_pt);
              mask[pt[0]][pt[1]][pt[2]] = true;
            }
          }
          else if( data[pt[0]][pt[1]][pt[2]] > pos_level )  {
            _pt[di] = cent[di]-1;
            stack.Push(_pt);
            mask[pt[0]][pt[1]][pt[2]] = true;
          }
          _pt[di] = cent[di];  // restore the original value
          pt[di] = norm_cent[di];
        }
      }
      new_cent /= peak.count;
      peak.center = new_cent.Round<size_t>();
      for( size_t i=0; i < 3; i++ )
        peak.center[i] = peak.center[i]%dim[i]; 
    }
  }

public:
  // a simple map integration, considering the peaks and holes as spheres
  template <typename MapT> static void Integrate(MapT*** const map, const vec3s& dim, 
    MapT pos_level, TArrayList<MapUtil::peak>& Peaks)  
  {
    TArray3D<bool> Mask(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
    const MapT neg_level = -pos_level; 
    for( size_t ix=0; ix < dim[0]; ix++ )  {
      for( size_t iy=0; iy < dim[1]; iy++ )  {
        for( size_t iz=0; iz < dim[2]; iz++ )  {
          const MapT& ref_val = map[ix][iy][iz];
          if( ref_val > pos_level || ref_val < neg_level )
            Peaks.Add(peak(ix, iy, iz));
        }
      }
    }
    peak_search<MapT>(map, dim, pos_level, Mask, Peaks);
  }
  /* a flood fill based algorithm to find undulating channels, which however come and exit at one of the 
  3 crystallographic directions */
  template <typename map_type> static vec3i AnalyseChannels1(map_type*** map, const vec3s& dim,
    map_type max_level)
  {
    map_type*** map_copy = ReplicateMap(map, dim);
    vec3i dim_ind(0, 1, 2);
    vec3i res;
    for( size_t dim_n=0; dim_n < 3; dim_n++ )  {
      res[dim_n] = max_level;
      if( dim_n == 1 )  // Y,Z,X
        dim_ind = vec3i(1, 2, 0);
      else if( dim_n == 2 )  // Z,X,Y
        dim_ind = vec3i(2, 0, 1);
      while( true )  {
        bool level_accessible = false;
        vec3s pt;
        TStack<vec3i> stack;
        while( stack.IsEmpty() )  { // seed
          for( size_t j=0; j < dim[dim_ind[1]]; j++ )  {
            for( size_t k=0; k < dim[dim_ind[2]]; k++ )  {
              pt[dim_ind[1]] = j;
              pt[dim_ind[2]] = k;
              if( map[pt[0]][pt[1]][pt[2]] >= res[dim_n] )  {  // find suitable start
                stack.Push(pt);
                map[pt[0]][pt[1]][pt[2]] = res[dim_n]-1;
              }
            }
          }
          if( stack.IsEmpty() && --res[dim_n] == 0 ) // would be odd
            break;
        }
        while( !stack.IsEmpty() )  {
          pt = stack.Pop();
          if( pt[dim_ind[0]] == dim[dim_ind[0]]-1 )  { // a weak condition, needs more work
            level_accessible = true;
            break;
          }
          vec3i p(pt);
          for( int ii=0; ii < 3; ii++ )  {
            p[dim_ind[ii]] = pt[dim_ind[ii]]-1;
            if( p[dim_ind[ii]] >= 0 && map[p[0]][p[1]][p[2]] == res[dim_n] )  {
              stack.Push(p);
              map[p[0]][p[1]][p[2]] = res[dim_n]-1;
            } 
            p[dim_ind[ii]] = pt[dim_ind[ii]]+1;
            if( p[dim_ind[ii]] < dim[dim_ind[ii]] && map[p[0]][p[1]][p[2]] >= res[dim_n] )  {
              stack.Push(p);
              map[p[0]][p[1]][p[2]] = res[dim_n]-1;
            } 
            p[dim_ind[ii]] = pt[dim_ind[ii]];
          }
        }
        if( !level_accessible )  {
          if( --res[dim_n] == 0 )
            break;
        }
        else
          break;
      }
      CopyMap(map, map_copy, dim);
    }
    DeleteMap(map_copy, dim);
    return res;
  }
  static int PeakSortByCount(const MapUtil::peak& a, const MapUtil::peak& b)  {
    return b.count - a.count;
  }
  static int PeakSortBySum(const MapUtil::peak& a, const MapUtil::peak& b)  {
    double diff = b.summ - a.summ;
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0); 
  }
  static int PeakSortByWeight(const MapUtil::peak& a, const MapUtil::peak& b)  {
    if( a.count == 0 )
      return b.count == 0 ? 0 : 1;
    else if( b.count == 0 )
      return a.count == 0 ? 0 : -1;
    const double diff = b.summ/b.count - a.summ/a.count;
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0); 
  }
protected:
  static int SortByDistance(const vec3d& a, const vec3d& b)  {
    const double d = a.QLength() - b.QLength();
    return d < 0 ? -1 : (d > 0 ? 1 : 0);
  }
public:
  static void StandardiseVec(vec3d& v, const smatd_list& ml)  {
    // make sure we start from the right point...
    for( size_t j=0; j < 3; j++ )  {
      while( v[j] < 0 )  v[j] += 1.0;
      while( v[j] >= 1.0 )  v[j] -= 1.0;
    }
    bool changes = true;
    while( changes )  {
      changes = false;
      for( size_t i=0; i < ml.Count(); i++ )  {
        vec3d tmp = ml[i]*v;
        for( size_t j=0; j < 3; j++ )  {
          while( tmp[j] < 0 )  tmp[j] += 1.0;
          while( tmp[j] >= 1.0 )  tmp[j] -= 1.0;
        }
        if( (tmp[0] < v[0] ) ||  // standardise then ...
            (olx_abs(tmp[0]-v[0]) < 1e-5 && (tmp[1] < v[1])) ||
            (olx_abs(tmp[0]-v[0]) < 1e-5 && olx_abs(tmp[1]-v[1]) < 1e-5 && (tmp[2] < v[2])) )    
        {
          v = tmp;
          changes = true;
        }
      }
    }
  }
  /*ml - all symmetry matrices, including identity matrix; cell2cart - symmetric matrix;
  norm - the reciprocal gridding (1/mapX, 1/mapY, 1/mapZ)*/
  template <class SymSpace>
  static void MergePeaks(const SymSpace& sp, const vec3d& norm, 
    TArrayList<MapUtil::peak>& Peaks, TTypeList<MapUtil::peak>& out)  
  {
    const size_t cnt = Peaks.Count();
    TTypeList<vec3d> crds;
    crds.SetCapacity(cnt);
    for( size_t i=0; i < cnt; i++ )  {
      crds.AddNew(Peaks[i].center) *= norm;
      Peaks[i].process = Peaks.Count() != 0;
    }
    Peaks.QuickSorter.SyncSortSF(Peaks, crds, PeakSortByCount);
    for( size_t i=0; i < cnt; i++ )  {
      if( !Peaks[i].process )  continue;
      TPtrList<MapUtil::peak> toMerge;
      toMerge.Add(Peaks[i]); 
      vec3d center(crds[i]), cmp_center(crds[i]);
      for( size_t j=i+1; j < cnt; j++ )  {
        if( !Peaks[j].process )  continue;
        for( size_t k=0; k < sp.Count(); k++ )  {
          const vec3d c = sp[k]*crds[j];
          vec3d tmp = c - cmp_center;
          const vec3i t = tmp.Round<int>();
          sp.CellToCartI(tmp -= t);
          if( tmp.QLength() < 0.5 )  {
            toMerge.Add(Peaks[j]);
            const vec3d mc = c-t;
            center += mc;
            cmp_center = (cmp_center*toMerge.Count()+mc*0.25)/((double)toMerge.Count()+0.25);
            Peaks[j].process = false;
            break;
          }
        }
      }
      MapUtil::peak& p = out.AddNew();
      for( size_t j=0; j < toMerge.Count(); j++ )  {
        p.count += toMerge[j]->count;
        p.summ += toMerge[j]->summ;
      }
      center /= toMerge.Count();
      center /= norm;
      p.center = center.Round<int16_t>();
    }
    for( size_t i=0; i < out.Count(); i++ )  {
      if( out[i].count == 0 )
        out.NullItem(i);
    }
    out.Pack();
  }
  template <typename map_type> static map_type*** ReplicateMap(map_type*** const map, const vec3s& dim)  {
    map_type*** map_copy = new map_type**[dim[0]];
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      map_copy[mi] = new map_type*[dim[1]];
      for( size_t mj=0; mj < dim[1]; mj++ )  {
        map_copy[mi][mj] = new map_type[dim[2]];
        memcpy(map_copy[mi][mj], map[mi][mj], dim[2]*sizeof(map_type));
      }
    }
    return map_copy;
  }
  template <typename map_type> static map_type*** CopyMap(map_type*** dest,
    map_type*** const src, const vec3s& dim)
  {
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      for( size_t mj=0; mj < dim[1]; mj++ )  {
        memcpy(dest[mi][mj], src[mi][mj], dim[2]*sizeof(map_type));
      }
    }
    return dest;
  }
  // a more generic case, cannot use memcpy
  template <typename dest_map_type, typename src_map_type>
  static dest_map_type*** CopyMap(dest_map_type*** dest, src_map_type*** const src, const vec3s& dim)  {
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      for( size_t mj=0; mj < dim[1]; mj++ )  {
        for( size_t mk=0; mk < dim[2]; mk++ )  {
          dest[mi][mj][mk] = src[mi][mj][mk];
        }
      }
    }
    return dest;
  }
  template <typename map_type> static void DeleteMap(map_type*** map, const vec3s& dim)  {
    for( size_t mi=0; mi < dim[0]; mi++ )  {
      for( size_t mj=0; mj < dim[1]; mj++ )
        delete [] map[mi][mj];
      delete [] map[mi];
    }
    delete [] map;
  }
};

EndXlibNamespace()
#endif

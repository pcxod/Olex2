#ifndef __olx_xl_splane_H
#define __olx_xl_splane_H
#include "xbase.h"
#include "typelist.h"
#include "satom.h"

BeginXlibNamespace()

const uint16_t
  plane_best = 1,
  plane_worst = 2;
const uint16_t
  plane_flag_deleted = 0x0001,
  plane_flag_regular = 0x0002;

class TSPlane : public TSObject<TNetwork>  {
private:
  TTypeList< AnAssociation2<TSAtom*, double> > Crds;
  vec3d FNormal, FCenter;
  double FDistance;
  uint16_t Flags;
public:
  TSPlane(TNetwork* Parent) : TSObject<TNetwork>(Parent), 
    FDistance(0), Flags(0)  {}
  virtual ~TSPlane()  {}

  DefPropBFIsSet(Deleted, Flags, plane_flag_deleted)
  // this is just a flag for the owner - is not used by the object itself
  DefPropBFIsSet(Regular, Flags, plane_flag_regular)

  inline size_t CrdCount() const {  return Crds.Count(); }
  // an association point, weight is provided
  void Init(const TTypeList<AnAssociation2<TSAtom*, double> >& Points);

  inline const vec3d& GetNormal() const {  return FNormal; }
  inline const vec3d& GetCenter() const {  return FCenter; }

  double DistanceTo(const vec3d& Crd) const {  return Crd.DotProd(FNormal) - FDistance;  }
  double DistanceTo(const TSAtom& a) const {  return DistanceTo(a.crd());  }
  double Angle(const vec3d& A, const vec3d& B) const {  return acos(FNormal.CAngle(B-A))*180/M_PI;  }
  double Angle(const vec3d& v) const {  return acos(FNormal.CAngle(v))*180/M_PI;  }
  double Angle(const class TSBond& B) const;
  double Angle(const TSPlane& P) const {  return Angle(P.GetNormal());  }
  double GetD() const {  return FDistance; }
  void SetD(double v) {  FDistance = v; }
  double GetZ(const double& X, const double& Y) const {
    return (FNormal[2] == 0) ? 0.0 : (FNormal[0]*X + FNormal[1]*Y + FDistance)/FNormal[2];
  }
  size_t Count() const {  return Crds.Count();  }
  const TSAtom& GetAtom(size_t i) const {  return *Crds[i].GetA();  }
  TSAtom& GetAtom(size_t i) {  return *Crds[i].A();  }
  double GetWeight(size_t i) const {  return Crds[i].GetB();  }

// static members
  /* calculates all three planes - best, worst and the complimentary, 
  the normals are sorted by rms ascending, so the best plane is at [0] and the worst - at [2]
  returns true if the function succeded (point cound > 2)
  */
  static bool CalcPlanes(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
    mat3d& params, vec3d& rms, vec3d& center);
  // a convinience function for non-weighted plane
  static bool CalcPlanes(const TSAtomPList& atoms, mat3d& params, vec3d& rms, vec3d& center);
  /* calculates the A,B and C for the best/worst plane Ax*By*Cz+D=0, D can be calculated as
   D = center.DotProd({A,B,C})
   for the point, weight association
   returns sqrt(smallest eigen value/point.Count())
  */
  static double CalcPlane(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  // a convinience function for non-weighted planes
  static double CalcPlane(const TSAtomPList& Points, 
    vec3d& Params, vec3d& center, const short plane_type = plane_best);
  // returns sqrt(smallest eigen value/point.Count())
  static double CalcRMS(const TSAtomPList& atoms);

  class Def  {
    struct DefData {
      TSAtom::Ref ref;
      double weight;
      DefData(const TSAtom::Ref& r, double w) : ref(r), weight(w)  {}
      DefData(const DefData& r) : ref(r.ref), weight(r.weight)  {}
      DefData& operator = (const DefData& r)  {  
        ref = r.ref;  
        weight = r.weight;
        return *this;
      }
      int Compare(const DefData& d) const {
        int diff = olx_cmp_size_t(ref.catom_id, d.ref.catom_id);
        if( diff == 0 )
          diff = olx_cmp_size_t(ref.matrix_id, d.ref.matrix_id);
        return diff;
      }
    };
    TTypeList<DefData> atoms;
  public:
    Def(const TSPlane& plane);
    Def(const Def& r) : atoms(r.atoms)  {}
    Def& operator = (const Def& r)  {
      atoms = r.atoms;
      return *this;
    }
    bool operator == (const Def& d)  const {
      if( atoms.Count() != d.atoms.Count() )  return false;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        if( atoms[i].ref.catom_id != d.atoms[i].ref.catom_id ||
          atoms[i].ref.matrix_id != d.atoms[i].ref.matrix_id )
          return false;
      }
      return true;
    }
    TSPlane* FromAtomRegistry(class AtomRegistry& ar, class TNetwork* parent, const smatd& matr) const;
  };

  Def GetDef() const { return Def(*this);  }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};

  typedef TTypeList<TSPlane> TSPlaneList;
  typedef TPtrList<TSPlane> TSPlanePList;
EndXlibNamespace()
#endif


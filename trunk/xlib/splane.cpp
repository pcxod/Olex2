//---------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TSPlane implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "splane.h"
#include "ematrix.h"
#include "satom.h"
#include "sbond.h"

//..............................................................................
TSPlane::TSPlane(TNetwork* Parent):TSObject(Parent)  {
  FDistance = 0;
  Deleted = false;
}
//..............................................................................
TSPlane::~TSPlane()  {  }
//..............................................................................
double TSPlane::CalcRMS(const TSAtomPList& atoms)  {
  if( atoms.Count() < 3 )  return 0;
  vec3d p, c;
  TTypeList< AnAssociation2<vec3d, double> > Points;
  Points.SetCapacity( atoms.Count() );
  for( int i=0; i < atoms.Count(); i++ )
    Points.AddNew( atoms[i]->crd(), 1);
  CalcPlane(Points, p, c);
  
  double d = c.DotProd(p)/p.Length(), rms=0;
  p.Normalise();
  for( int i=0; i < atoms.Count(); i++ )  {
    double tmp = p.DotProd(atoms[i]->crd())-d;
    rms += tmp*tmp;
  }
  return sqrt(rms/atoms.Count());
}
//..............................................................................
void TSPlane::Init(const TTypeList< AnAssociation2<vec3d, double> >& Points)  {
  CalcPlane(Points, FNormal, FCenter);
  FDistance = FNormal.DotProd(FCenter)/FNormal.Length();
  FNormal.Normalise();
  Crds.SetCapacity( Points.Count() );
  for( int i=0; i < Points.Count(); i++ )
    Crds.AddCCopy(Points[i].GetA());
}
//..............................................................................
void TSPlane::CalcPlane(const TTypeList< AnAssociation2<vec3d, double> >& Points, 
                        vec3d& Params, vec3d& center)  {
  if( Points.Count() < 3 )  return;
  mat3d m, vecs;
  vec3d p, t;
  double mass = 0;
  center.Null();
  for( int i=0; i < Points.Count(); i++ )  {
    center += Points[i].GetA()*Points[i].GetB();
    mass += Points[i].GetB();
  }
  center /= mass;

  for( int i=0; i < Points.Count(); i++ )  {
    t = Points[i].GetA() - center;
    double wght = Points[i].GetB()*Points[i].GetB();
    m[0][0] += (t[0]*t[0]*wght);
    m[0][1] += (t[0]*t[1]*wght);
    m[0][2] += (t[0]*t[2]*wght);

    m[1][1] += (t[1]*t[1]*wght);
    m[1][2] += (t[1]*t[2]*wght);

    m[2][2] += t[2]*t[2]*wght;
  } // equ: d = s[0]*x + s[1]*y + s[2]*z
  m[1][0] = m[0][1];
  m[2][0] = m[0][2];
  m[2][1] = m[1][2];
  mat3d::EigenValues(m, vecs.I());
  if( m[0][0] < m[1][1] )  {
    if( m[0][0] < m[2][2] )  
      Params = vecs[0];
    else                     
      Params = vecs[2];
  }
  else  {
    if( m[1][1] < m[2][2] )
      Params = vecs[1];
    else
      Params = vecs[2];
  }
}
//..............................................................................
double TSPlane::DistanceTo(const vec3d& Crd) const {
  return Crd.DotProd(FNormal) - FDistance;
}
//..............................................................................
double TSPlane::DistanceTo(const TSAtom& A) const  {
  return DistanceTo(A.crd());
}
//..............................................................................
double TSPlane::Angle( const vec3d &A,  const vec3d &B) const  {
  vec3d V;
  V = B-A;
  double ca = FNormal.CAngle(V), angle;
  angle = acos(ca)*180/M_PI;
  return angle;
}
//..............................................................................
double TSPlane::Angle(const TSBond& Bd) const  {
  return Angle(Bd.GetA().crd(), Bd.GetB().crd());
}
//..............................................................................
double TSPlane::Angle(const TSPlane& P) const  {
  vec3d A;
  return Angle(A, P.GetNormal());
}
//..............................................................................
double TSPlane::Z(double X, double Y) const  {
  if( !FNormal[2] )  return 0;
  return (FNormal[0]*X + FNormal[1]*Y + FDistance)/FNormal[2];
}
//..............................................................................
 

//----------------------------------------------------------------------------//
// TDBasis - a drawing object for basis
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "dbasis.h"
#include "gpcollection.h"
#include "glrender.h"
#include "styles.h"
#include "glmaterial.h"
#include "glprimitive.h"
#include "pers_util.h"

TDBasis::TDBasis(TGlRenderer& Render, const olxstr& collectionName) : 
AGlMouseHandlerImp(Render, collectionName),
Zoom(1.0)
{
  SetMove2D(true);
  SetMoveable(true);
  SetZoomable(true);
  SetSelectable(false);
  olxstr label_cn("db_label");
  for( int i=0; i < 3; i++ )  {
    (Labels[i] = new TXGlLabel(Parent, label_cn))->SetVisible(false);
    Labels[i]->SetTransformer(this);
  }
}
//..............................................................................
TDBasis::~TDBasis()  {
  for( int i=0; i < 3; i++ )
    delete Labels[i];
}
//..............................................................................
void TDBasis::SetAsymmUnit(TAsymmUnit& au)  {
  AU = &au;
  for( int i=0; i < 3; i++ )  {  
    //Labels[i]->SetCenter(FGlP->Vertices[i*2+1]);
    Labels[i]->SetLabel(olxstr((char)('a'+i)));
  }
}
//..............................................................................
void TDBasis::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;
  TGraphicsStyle& GS = GPC.GetStyle();

  mat3d m = AU->GetCellToCartesian();
  if( m[0].QLength() < 1.e-6 )
    m.I();
  TGlMaterial GlM, GlM1;
  GlM.SetFlags(0);   
  GlM.ShininessF = 128;      
  GlM.SpecularF = 0x03030303;  
  GlM.SpecularF = 0x03030303;
  
  GlM1.SetFlags(0);  
  GlM1.ShininessF = 128;     
  GlM1.SpecularF = 0x03030303;  
  GlM1.SpecularB = 0x03030303;

  const double ConeH = 0.8, ConeW = 0.2; // cylinder dimensions
  const int CQ = 5; // cylinder quality

  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);
  GlM1.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|  // non transluetn sphere
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);

  TGlPrimitive* GlP = &GPC.NewPrimitive("Sphere", sgloSphere);  // a sphere at the basis of the object {0,0,0}
  GlM.AmbientF = 0x800f0f0f;
  GlP->SetProperties(GlM1);
  GlP->Params[0] = ConeW/1.5;  GlP->Params[1] = 6;  GlP->Params[2] = 6;

  GlM.AmbientF = 0x800000ff;
  GlP = &GPC.NewPrimitive("ConeX", sgloCylinder);  // X cone
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  TEBasis* EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d,mat3d>(m[0]));  EB->SetCenter(m[0]/5);
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC.NewPrimitive("DiskX", sgloDisk);  // X cone bottom
  GlP->SetProperties( GS.GetMaterial(GlP->GetName(), GlM) );
  GlP->SetBasis(new TEBasis(*EB));
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC.NewPrimitive("CylinderX",sgloCylinder);  // X axis
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis(*EB);  GlP->SetBasis(EB);  EB->NullCenter();
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = m[0].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlM.AmbientF = 0x8000ff00;
  GlP = &GPC.NewPrimitive("ConeY", sgloCylinder);  // Y
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d,mat3d>(m[1]));  EB->SetCenter(m[1]/5);
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC.NewPrimitive("DiskY", sgloDisk);  // Y cone bottom
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  GlP->SetBasis(new TEBasis(*EB));
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC.NewPrimitive("CylinderY", sgloCylinder);  // y axis
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis(*EB);  GlP->SetBasis(EB);  EB->NullCenter();
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = m[1].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlM.AmbientF  = 0x80ff0000;
  GlP = &GPC.NewPrimitive("ConeZ", sgloCylinder);  //Z cone
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d,mat3d>(m[2]));  EB->SetCenter(m[2]/5);
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC.NewPrimitive("DiskZ", sgloDisk);  // Z cone bottom
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  GlP->SetBasis(new TEBasis(*EB));
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC.NewPrimitive("CylinderZ", sgloCylinder);  // Z axis
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis(*EB);  GlP->SetBasis(EB);  EB->NullCenter();
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = m[2].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;
  
  Compile();
  for( int i=0; i < 3; i++ )
    Labels[i]->Create();
}
//..............................................................................
vec3d TDBasis::ForRaster(const TXGlLabel& l) const {
  const double EZoom = Parent.GetExtraZoom()*Parent.GetViewZoom();
  const double scale = 1./Parent.GetScale();
  vec3d center = GetCenter();
  center[0] = center[0]*EZoom;
  center[1] = center[1]*EZoom;
  center = Parent.GetBasis().GetMatrix() * center;
  vec3d T;
  if( &l == Labels[0] )
    T = AU->GetCellToCartesian()[0];
  else if( &l == Labels[1] )
    T = AU->GetCellToCartesian()[1];
  else if( &l == Labels[2] )
    T = AU->GetCellToCartesian()[2];
  T /= 5;
  T += vec3d(T).NormaliseTo(0.8);
  T *= (GetZoom()*Parent.GetBasis().GetZoom()* scale);
  T += center;
  T *= Parent.GetBasis().GetMatrix();
  return T;
}
//..............................................................................
vec3d TDBasis::ForVector(const TXGlLabel& l) const {
  const double EZoom = Parent.GetExtraZoom()*Parent.GetViewZoom();
  vec3d center = GetCenter();
  center[0] = center[0]*EZoom;
  center[1] = center[1]*EZoom;
  center *= Parent.GetScale();
  center = Parent.GetBasis().GetMatrix() * center;
  vec3d T;
  if( &l == Labels[0] )
    T = AU->GetCellToCartesian()[0];
  else if( &l == Labels[1] )
    T = AU->GetCellToCartesian()[1];
  else if( &l == Labels[2] )
    T = AU->GetCellToCartesian()[2];
  T /= 5;
  T += vec3d(T).NormaliseTo(0.8);
  T *= (GetZoom()*Parent.GetBasis().GetZoom());
  T += center;
  T *= Parent.GetBasis().GetMatrix();
  return T;
}
//..............................................................................
vec3d& TDBasis::AdjustZ(vec3d& v) const {
  v[2] = Parent.GetMaxRasterZ();
  return v;
}
//..............................................................................
bool TDBasis::Orient(TGlPrimitive& P) {
  // extra zoom is very important for making pictures - it makes sure that the
  // object is translated to the right place!
  const double EZoom = Parent.GetExtraZoom()*Parent.GetViewZoom();
  //const double zoom = olx_sqr(Parent.GetBasis().GetZoom());
  vec3d T = GetCenter();
  T[0] = T[0]*EZoom;
  T[1] = T[1]*EZoom;
  T *= Parent.GetScale();
  T = Parent.GetBasis().GetMatrix() * T;
  T /= Parent.GetBasis().GetZoom();
  T -= Parent.GetBasis().GetCenter();
  olx_gl::translate(T);
  olx_gl::scale(GetZoom());
  return false;
}
//..............................................................................
void TDBasis::ToDataItem(TDataItem& di) const {
  di.AddField("center", PersUtil::VecToStr(GetCenter()));
  di.AddField("zoom", GetZoom());
}
//..............................................................................
void TDBasis::FromDataItem(const TDataItem& di)  {
  const olxstr& c = di.GetFieldValue("center");
  if( c.IsEmpty() )  {
    TEBasis b;
    b.FromDataItem(di.FindRequiredItem("basis"));
    _Center = b.GetCenter();
    Zoom = b.GetZoom();
  }
  else  {
    _Center = PersUtil::FloatVecFromStr(c);
    Zoom = di.GetRequiredField("zoom").ToDouble();
  }
}
//..............................................................................
void TDBasis::ListPrimitives(TStrList &List) const {}
//..............................................................................
void TDBasis::UpdatePrimitives(int32_t Mask, const ACreationParams* cpar) {}
//..............................................................................
void TDBasis::SetLabelsFont(uint16_t fnt_index)  {
  for( int i=0; i < 3; i++ )
    Labels[i]->SetFontIndex(fnt_index);
}
//..............................................................................
void TDBasis::UpdateLabels()  {
  for( int i=0; i < 3; i++ )
    Labels[i]->SetLabel(Labels[i]->GetLabel());
}
//..............................................................................
void TDBasis::SetVisible(bool v)  {
  AGDrawObject::SetVisible(v);
  for( int i=0; i < 3; i++ )
    Labels[i]->SetVisible(v);
}
//..............................................................................

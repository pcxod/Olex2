/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dsphere.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"
#include "glutil.h"
#include "esphere.h"

TDSphere::TDSphere(TGlRenderer& R, PointAnalyser &pa,
  const olxstr& collectionName)
  : AGlMouseHandlerImp(R, collectionName),
    analyser(pa),
    Generation(6)
{
  SetSelectable(false);
}
//...........................................................................
void TDSphere::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(false);
  TGlPrimitive& GlP = GPC->NewPrimitive("Sphere", sgloCommandList);
  TGlMaterial &m = GS.GetMaterial("Object",
      TGlMaterial("85;0;4286611584;4290822336;64"));
  m.SetColorMaterial(true);
  m.SetTransparent(true);
  GlP.SetProperties(m);
  TTypeList<TVector3<float> > vecs;
  TTypeList<IndexTriangle> triags;
  OlxSphere<float,OctahedronFP<vec3f> >::Generate(1.0, Generation, vecs,
    triags);
  const size_t tc = triags.Count();
  uint32_t last_cl = 0;
  TArrayList<uint32_t> colors(vecs.Count());
  for (size_t i=0; i < vecs.Count(); i++)
    colors[i] = analyser.Analyse(vecs[i]);
  GlP.StartList();
  olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  olx_gl::begin(GL_TRIANGLES);
  for( size_t i = 0; i < tc; i++ )  {
    const IndexTriangle& t = triags[i];
    for( size_t j=0; j < 3; j++ )  {
      uint32_t cl = colors[t.vertices[j]];
      if( cl != last_cl )  {
        olx_gl::color((float)OLX_GetRValue(cl)/255,
          (float)OLX_GetGValue(cl)/255,
          (float)OLX_GetBValue(cl)/255,
          (float)OLX_GetAValue(cl)/255);
        last_cl = cl;
      }
      olx_gl::normal(vecs[t.vertices[j]]);
      olx_gl::vertex(vecs[t.vertices[j]]);
    }
  }
  olx_gl::end();
  GlP.EndList();
}
//...........................................................................
bool TDSphere::Orient(TGlPrimitive& P)  {
  olx_gl::orient(Basis.GetMDataT());
  olx_gl::scale(Basis.GetZoom());
  return false;
}
//...........................................................................
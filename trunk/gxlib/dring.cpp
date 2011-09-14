/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dring.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"
#include "glutil.h"

TGlPrimitive* TDRing::torus = NULL;
bool TDRing::initialised = false;
//..............................................................................
TDRing::TContextClear::TContextClear(TGlRenderer& Render)  {
  Render.OnClear.Add(this);
}
//..............................................................................
bool TDRing::TContextClear::Enter(const IEObject *Sender, const IEObject *Data)
{
  TDRing::torus = NULL;
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
TDRing::TDRing(TGlRenderer& R, const olxstr& collectionName) :
  AGlMouseHandlerImp(R, collectionName),
  material("85;0;4286611584;4290822336;64")
{
  SetSelectable(false);
  if( !initialised )  {
    initialised = true;
    new TContextClear(R);
  }
}
//...........................................................................
double TDRing::GetRadius() const {
  return 1.0 + 0.075;
}
//...........................................................................
void TDRing::Create(const olxstr& cName)  {
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
  TGlPrimitive& GlP = GPC->NewPrimitive("Torus", sgloCommandList);
  GlP.SetProperties(GS.GetMaterial("Object", material));
  if( torus == NULL ) {
    torus = &Parent.NewPrimitive(sgloCommandList);
    torus->StartList();
    GlTorus::Render(0.075, 1, 8, 20);
    torus->EndList();
  }
  GlP.StartList();
  GlP.CallList(torus);
  GlP.EndList();
}
//...........................................................................
bool TDRing::Orient(TGlPrimitive& P)  {
  olx_gl::orient(Basis.GetMDataT());
  olx_gl::scale(Basis.GetZoom());
  return false;
}
//...........................................................................

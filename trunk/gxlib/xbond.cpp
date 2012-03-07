/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xbond.h"
#include "gpcollection.h"
#include "xatom.h"
#include "lattice.h"
#include "symmparser.h"
#include "unitcell.h"
#include "povdraw.h"

bool TXBond::TStylesClear::Enter(const IEObject *Sender, const IEObject *Data)  {
  TXBond::FBondParams = NULL;
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
bool TXBond::TStylesClear::Exit(const IEObject *Sender, const IEObject *Data)  {
  TXBond::ValidateBondParams();
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
//..............................................................................
TXBond::TContextClear::TContextClear(TGlRenderer& Render)  {
  Render.OnClear.Add(this);
}
//..............................................................................
bool TXBond::TContextClear::Enter(const IEObject *Sender, const IEObject *Data)  {
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
bool TXBond::TContextClear::Exit(const IEObject *Sender, const IEObject *Data)  {
  return true;
}
//..............................................................................
//----------------------------------------------------------------------------//
// TXBond function bodies
//----------------------------------------------------------------------------//
TStrPObjList<olxstr,TGlPrimitive*> TXBond::FStaticObjects;
TArrayList<TGlPrimitiveParams>  TXBond::FPrimitiveParams;
TGraphicsStyle* TXBond::FBondParams=NULL;
TXBond::TStylesClear *TXBond::OnStylesClear=NULL;
double TXBond::FDefR = -1;
int TXBond::FDefM = -1;
//..............................................................................
TXBond::TXBond(TNetwork* net, TGlRenderer& R, const olxstr& collectionName) :
  TSBond(net),
  AGDrawObject(R, collectionName),
  FDrawStyle(0x0001)
{
  SetGroupable(true);
  Params().Resize(5);
  Params()[4] = 0.8;
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName);
  Label->SetVisible(false);
  if( OnStylesClear == NULL )  {
    OnStylesClear = new TStylesClear(R);
    new TContextClear(R);
  }
}
//..............................................................................
TXBond::~TXBond()  {
  if( GetParentGroup() != NULL )  {
    GetParentGroup()->Remove(*this);
#ifdef _DEBUG
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
#endif
  }
  delete Label;
}
//..............................................................................
void TXBond::Update()  {
  if( !IsValid() )  return;
  vec3d C(B().crd() - A().crd());
  if( C.IsNull() )  
    Params().Null();
  else  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = acos(C[2])*180/M_PI;
    if( olx_abs(Params()[0]-180) < 1e-3 )  { // degenerate case with Pi rotation
      Params()[1] = 0;
      Params()[2] = 1;
    }
    else {
      Params()[1] = -C[1];
      Params()[2] = C[0];
    }
  }
}
//..............................................................................
void TXBond::Create(const olxstr& cName)  {
  SetCreated(true);
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  if( FStaticObjects.IsEmpty() )  
    CreateStaticObjects(Parent);
  if( IsValid() && Label->GetOffset().IsNull() )  // init label offset
    Label->SetOffset((A().crd()+B().crd())/2);
  Label->SetFontIndex(Parent.GetScene().FindFontIndexForType<TXBond>());
  Label->Create();
  // find collection
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  else if( GPC->PrimitiveCount() != 0 )  {
    GPC->AddObject(*this);
    Params()[4] = GPC->GetStyle().GetNumParam('R', DefR());
    return;
  }
  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(IsStyleSaveable());

  const int PrimitiveMask = GS.GetNumParam(GetPrimitiveMaskName(),
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable());

  GPC->AddObject(*this);
  if( PrimitiveMask == 0 )  
    return;  // nothing to create then...

  Params()[4]= GS.GetNumParam('R', DefR());
  const uint16_t legend_level = TXAtom::LegendLevel(GetPrimitives().GetName());
  for( size_t i=0; i < FStaticObjects.Count(); i++ )  {
    if( (PrimitiveMask & (1<<i)) != 0 )    {
      TGlPrimitive* SGlP = FStaticObjects.GetObject(i);
      TGlPrimitive& GlP = GPC->NewPrimitive(FStaticObjects[i], sgloCommandList);
      /* copy the default drawing style tag source */
      GlP.Params.Resize(GlP.Params.Count()+1);
      GlP.Params.GetLast() = SGlP->Params.GetLast();

      GlP.StartList();
      GlP.CallList(SGlP);
      GlP.EndList();
      TGlMaterial* style_mat = 
        legend_level == 3 ? GS.FindMaterial(FStaticObjects[i]) : NULL;
      if( IsValid() )  {
        if( style_mat != NULL )
          GlP.SetProperties(*style_mat);
        else  {
          TGlMaterial RGlM;
          if( SGlP->Params.GetLast() == ddsDefAtomA || SGlP->Params.GetLast() == ddsDef )  {
            if( !A().IsCreated() )
              A().Create();
            const size_t mi = A().Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = A().Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(A(), RGlM);
          }
          else if( SGlP->Params.GetLast() == ddsDefAtomB )  {
            if( !B().IsCreated() )
              B().Create();
            const size_t mi = B().Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = B().Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(B(), RGlM);
          }
          if( legend_level == 4 )
            GlP.SetProperties(GS.GetMaterial(FStaticObjects[i], RGlM));
          else // must be updated from atoms always
            GlP.SetProperties(RGlM);
        }
      }
      else  {  // no atoms
        GlP.SetProperties(GS.GetMaterial(FStaticObjects[i],
          TGlMaterial("85;2155839359;2155313015;1.000,1.000,1.000,0.502;36")));
      }
    }
  }
}
//..............................................................................
void TXBond::UpdateStyle()  {
  TGPCollection &gpc = GetPrimitives();
  const uint16_t legend_level = TXAtom::LegendLevel(gpc.GetName());
  if( legend_level == 3 )  // is user managed?
    return;
  TGraphicsStyle& GS = gpc.GetStyle();
  const int PrimitiveMask = GS.GetNumParam(GetPrimitiveMaskName(),
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable());
  for( size_t i=0; i < FStaticObjects.Count(); i++ )  {
    if( (PrimitiveMask & (1<<i)) != 0 )    {
      TGlPrimitive *SGlP = FStaticObjects.GetObject(i);
      TGlPrimitive *GlP = gpc.FindPrimitiveByName(FStaticObjects[i]);
      if( GlP == NULL )  // must not ever happen...
        continue;
      if( IsValid() )  {
        TGlMaterial RGlM;
        if( SGlP->Params.GetLast() == ddsDefAtomA || SGlP->Params.GetLast() == ddsDef )  {
          if( !A().IsCreated() )
            A().Create();
          const size_t mi = A().Style().IndexOfMaterial("Sphere");
          if( mi != InvalidIndex )
            RGlM = A().Style().GetPrimitiveStyle(mi).GetProperties();
          else
            TXAtom::GetDefSphereMaterial(A(), RGlM);
        }
        else if( SGlP->Params.GetLast() == ddsDefAtomB )  {
          if( !B().IsCreated() )
            B().Create();
          const size_t mi = B().Style().IndexOfMaterial("Sphere");
          if( mi != InvalidIndex )
            RGlM = B().Style().GetPrimitiveStyle(mi).GetProperties();
          else
            TXAtom::GetDefSphereMaterial(B(), RGlM);
        }
        GlP->SetProperties(RGlM);
      }
      else  {  // no atoms
        GlP->SetProperties(GS.GetMaterial(FStaticObjects[i],
          TGlMaterial("85;2155839359;2155313015;1.000,1.000,1.000,0.502;36")));
      }
    }
  }
}
//..............................................................................
bool TXBond::Orient(TGlPrimitive& GlP)  {
  olx_gl::translate(A().crd());
  olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
  olx_gl::scale(Params()[4], Params()[4], Params()[3]);
  return false;
}
//..............................................................................
void TXBond::ListParams(TStrList &List, TGlPrimitive *Primitive)  {
}
//..............................................................................
void TXBond::ListParams(TStrList &List)  {
}
//..............................................................................
void TXBond::UpdatePrimitiveParams(TGlPrimitive *Primitive)  {
}
//..............................................................................
void TXBond::ListPrimitives(TStrList &List) const {
  List.Assign(FStaticObjects);
}
//..............................................................................
void TXBond::Quality(const short Val)  {
  ValidateBondParams();
  olxstr& ConeQ = FBondParams->GetParam("ConeQ", "15", true);
  switch( Val )  {
    case qaPict:
    case qaHigh:   ConeQ = 30;  break;
    case qaMedium: ConeQ = 15;  break;
    case qaLow:    ConeQ = 5;  break;
  }
  return;
}
//..............................................................................
void TXBond::ListDrawingStyles(TStrList &L){  return; }
//..............................................................................
const vec3d &TXBond::GetBaseCrd() const {
  if( !IsValid() )
    throw TFunctionFailedException(__OlxSourceInfo, "atoms are not defined");
  return A().crd();
}
//..............................................................................
const_strlist TXBond::ToPov(olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials) const
{
  TStrList out;
  if( olx_abs(Params()[1]) + olx_abs(Params()[2]) < 1e-3 )
    return out;
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  for( size_t i=0; i < gpc.PrimitiveCount(); i++ )  {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    olxstr p_mat = pov::get_mat_name(glp.GetProperties(), materials, this);
    out.Add("  object {") << "bond_" << glp.GetName().ToLowerCase().Replace(' ', '_')
      << " texture {" << p_mat << "}}";
  }
  out.Add("  }");
  mat3d m;
  olx_create_rotation_matrix(m, vec3d(Params()[1], Params()[2], 0).Normalise(),
    cos(Params()[0]*M_PI/180));
  m[0] *= Params()[4];
  m[1] *= Params()[4];
  m[2] *= Params()[3];
  m *= Parent.GetBasis().GetMatrix();
  vec3d t = pov::CrdTransformer(Parent.GetBasis()).crd(GetBaseCrd());
  out.Add("  transform {");
  out.Add("   matrix") << pov::to_str(m, t);
  out.Add("   }");
  out.Add(" }");
  return out;
}
//..............................................................................
const_strlist TXBond::PovDeclare()  {
  TStrList out;
  out.Add("#declare bond_single_cone=object{ cylinder {<0,0,0>, <0,0,1>, 0.1} }");
  out.Add("#declare bond_top_disk=object{ disc {<0,0,1><0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_disk=object{ disc {<0,0,0><0,0,-1>, 0.1} }");
  out.Add("#declare bond_middle_disk=object{ disc {<0,0,0.5><0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_cone=object{ cylinder {<0,0,0>, <0,0,0.5>, 0.1} }");
  out.Add("#declare bond_top_cone=object{ cylinder {<0,0,0.5>, <0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_line=object{ cylinder {<0,0,0>, <0,0,0.5>, 0.01} }");
  out.Add("#declare bond_top_line=object{ cylinder {<0,0,0.5>, <0,0,1>, 0.01} }");

  double ConeStipples = FBondParams->GetNumParam("ConeStipples", 6.0, true);
  out.Add("#declare bond_stipple_cone=object { union {");
  for( double i=0; i < ConeStipples; i++ )  {
    out.Add(" disc {<0,0,") << i/ConeStipples << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << i/ConeStipples << ">, <0,0,"
      << (i+0.5)/ConeStipples << ">, 0.1}";
    out.Add(" disc {<0,0,") << (double)(i+0.5)/ConeStipples << "><0,0,1>, 0.1}";
  }
  out.Add("}}");
  
  out.Add("#declare bond_bottom_stipple_cone=object { union {");
  for( double i=0; i < ConeStipples/2; i++ )  {
    out.Add(" disc {<0,0,") << (i+0.5)/ConeStipples << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << (i+0.5)/ConeStipples << ">, <0,0,"
      << (i+1)/ConeStipples << ">, 0.1}";
    out.Add(" disc {<0,0,") << (i+1)/ConeStipples << "><0,0,1>, 0.1}";
  }
  out.Add("}}");

  out.Add("#declare bond_top_stipple_cone=object { union {");
  for( int i=0; i < ConeStipples/2; i++ )  {
    out.Add(" disc {<0,0,") << ((i+0.5)/ConeStipples+0.5) << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << ((i+0.5)/ConeStipples+0.5) << ">, <0,0,"
      << ((i+1)/ConeStipples+0.5) << ">, 0.1}";
    out.Add(" disc {<0,0,") << ((i+1)/ConeStipples+0.5) << "><0,0,1>, 0.1}";
  }
  out.Add("}}");

  out.Add("#declare bond_balls_bond=object { union {");
  for( int i=0; i < 12; i++ )
    out.Add(" sphere {<0,0,") << (double)i/12 << ">, 0.02}";
  out.Add("}}");

  out.Add("#declare bond_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  out.Add("#declare bond_stippled_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  
  return out;
}
//..............................................................................
void TXBond::CreateStaticObjects(TGlRenderer& Parent)  {
  ClearStaticObjects();
  TGlMaterial GlM;
  TGlPrimitive *GlP, *GlPRC1, *GlPRD1, *GlPRD2;
  ValidateBondParams();
  double ConeQ = FBondParams->GetNumParam("ConeQ", 15.0, true);
  double ConeStipples = FBondParams->GetNumParam("ConeStipples", 6.0, true);
//..............................
  // create single color cylinder
  GlP = &Parent.NewPrimitive(sgloCylinder);
  FStaticObjects.Add("Single cone", GlP);

  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 1;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create top disk
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Top disk", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloDisk); 
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0, 0, 1);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
//..............................
  // create bottom disk
  GlP = &Parent.NewPrimitive(sgloDisk);
  FStaticObjects.Add("Bottom disk", GlP);

  GlP->SetQuadricOrientation(GLU_INSIDE);
  GlP->Params[0] = 0;  GlP->Params[1] = 0.1;  GlP->Params[2] = ConeQ;
  GlP->Params[3] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create middle disk
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Middle disk", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloDisk);
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create bottom cylinder
  GlP = &Parent.NewPrimitive(sgloCylinder);
  FStaticObjects.Add("Bottom cone", GlP);

  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create top cylinder
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Top cone", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
//..............................
  // create bottom line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Bottom line", GlP);

  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0.0f, 0.0f, 0.5f);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create top line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Top line", GlP);

  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0.0f, 0.0f, 0.5f);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
//..............................
  // create stipple cone
  float CL = (float)(1.0/(2*ConeStipples));
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Stipple cone", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = CL;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlPRD1 = &Parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.1;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = &Parent.NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = 0;  GlPRD2->Params[1] = 0.1;  GlPRD2->Params[2] = ConeQ;
  GlPRD2->Params[3] = 1;
  GlPRD2->Compile();

  GlP->StartList();
  for( int i=0; i < ConeStipples; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDef;
  //..............................
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Bottom stipple cone", GlP);

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, CL/2);
  for( int i=0; i < ConeStipples/2; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Top stipple cone", GlP);

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, (float)(0.5 + CL/2));
  for( int i=0; i < ConeStipples/2; i++ )  {
    GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;

//..............................
  // create stipped ball bond
  CL = (float)(1.0/(12.0));
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Balls bond", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloSphere);
  GlPRC1->Params[0] = 0.02;    GlPRC1->Params[1] = 5;  GlPRC1->Params[2] = 5;
  GlPRC1->Compile();

  GlP->StartList();
  for( int i=0; i < 12; i++ )  {
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRC1);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDef;
//..............................
  // create line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Line", GlP);

  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create stippled line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Stippled line", GlP);

  GlP->StartList();
    olx_gl::enable(GL_LINE_STIPPLE);
    olx_gl::lineStipple(1, 0xf0f0);
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  olx_gl::disable(GL_LINE_STIPPLE);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
}
//..............................................................................
olxstr TXBond::GetLegend(const TSBond& Bnd, const short level)  {
  olxstr L(EmptyString(), 32);
  const TSAtom *A = &Bnd.A(),
               *B = &Bnd.B();
  if( A->GetType() != B->GetType() )  {
    if( A->GetType() < B->GetType() )
      olx_swap(A, B);
  }
  else  {
    if( A->GetLabel().Compare(B->GetLabel()) < 0 )
      olx_swap(A, B);
  }
  L << A->GetType().symbol << '-' << B->GetType().symbol;
  if( Bnd.GetType() == sotHBond )  
    L << "@H";
  if( level == 0 )  return L;
  L << '.' << A->GetLabel() << '-' << B->GetLabel();
  if( level == 1 )  return L;
  TUnitCell::SymmSpace sp =
    A->GetNetwork().GetLattice().GetUnitCell().GetSymmSpace();
  L << '.' << TSymmParser::MatrixToSymmCode(sp, A->GetMatrix()) <<
    '-' <<
    TSymmParser::MatrixToSymmCode(sp, B->GetMatrix());
  if( level == 2 )  return L;
  return L << ".u";
}
//..............................................................................
void TXBond::SetRadius(float V)  {
  Params()[4] = V;
  if( this->Primitives != NULL )  {
    GetPrimitives().GetStyle().SetParam("R", V, IsRadiusSaveable());
    // update radius for all members of the collection
    for( size_t i=0; i < GetPrimitives().ObjectCount(); i++ )
      GetPrimitives().GetObject(i).Params()[4] = V;
  }
}
//..............................................................................
uint32_t TXBond::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetNumParam(GetPrimitiveMaskName(),
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable());
}
//..............................................................................
void TXBond::OnPrimitivesCleared()  {
  if( !FStaticObjects.IsEmpty() )
    FStaticObjects.Clear();
}
//..............................................................................
void TXBond::ValidateBondParams()  {
  if( FBondParams == NULL )  {
    FBondParams = &TGlRenderer::_GetStyles().NewStyle("BondParams", true);
    FBondParams->SetPersistent(true);
  }
}
//..............................................................................
void TXBond::DefMask(int V)  {
  ValidateBondParams();
  FBondParams->SetParam("DefM", (FDefM=V), true);
}
//..............................................................................
int TXBond::DefMask()  {
  if (FDefM != -1) return FDefM;
  ValidateBondParams();
  return (FDefM = FBondParams->GetNumParam("DefM", 7, true));
}
//..............................................................................
void TXBond::DefR(double V)  {
  ValidateBondParams();
  FBondParams->SetParam("DefR", (FDefR=V), true);
}
//..............................................................................
double TXBond::DefR()  {
  if (FDefR > 0) return FDefR;
  ValidateBondParams();
  return (FDefR = FBondParams->GetNumParam("DefR", 1.0, true));
}
//..............................................................................
bool TXBond::OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseDown(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................

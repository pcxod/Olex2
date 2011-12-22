/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gllabel.h"
#include "gpcollection.h"
#include "styles.h"
#include "glrender.h"
#include "glprimitive.h"
#include "pers_util.h"

TXGlLabel::TXGlLabel(TGlRenderer& R, const olxstr& collectionName) :
  AGlMouseHandlerImp(R, collectionName), Transformer(NULL)
{
  SetMove2DZ(true);
  SetMoveable(true);
  SetZoomable(false);
  SetGroupable(true);
  FontIndex = ~0;
};
//..............................................................................
void TXGlLabel::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  text_rect = GetFont().GetTextRect(FLabel);
  if( GPC.PrimitiveCount() != 0 )  {
    TGlPrimitive* glpText = GPC.FindPrimitiveByName("Text");
    if( glpText != NULL )
      glpText->SetFont(&GetFont());
    return;
  }

  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetPersistent(true);
  TGlPrimitive& glpPlane = GPC.NewPrimitive("Plane", sgloQuads);
  glpPlane.SetProperties(GS.GetMaterial("Plane", TGlMaterial("3077;2131693327;427259767")));
  glpPlane.Vertices.SetCount(4);

  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties(GS.GetMaterial("Text", TGlMaterial("2049;0.000,0.000,0.000,1.000")));
  glpText.SetFont(&GetFont());
  glpText.Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
void TXGlLabel::SetLabel(const olxstr& L)  {
  FLabel = L;  
  text_rect = GetFont().GetTextRect(FLabel);
}
//..............................................................................
vec3d TXGlLabel::GetRasterPosition() const {
  const double ScaleR = Parent.GetExtraZoom()*Parent.GetViewZoom();
  vec3d off = (GetCenter()+vec3d(-text_rect.width/2,text_rect.height/2,0))*Parent.GetBasis().GetZoom();
  //vec3d off = (GetCenter()*Parent.GetBasis().GetZoom());
  if( Transformer != NULL )  {
    vec3d T = Transformer->ForRaster(*this);
    return Transformer->AdjustZ(T += off*ScaleR);
  }
  vec3d T(Parent.GetBasis().GetCenter()+GetOffset());
  T *= Parent.GetBasis().GetMatrix();
  T *= Parent.GetBasis().GetZoom();
  T /= Parent.GetScale();
  T += off*ScaleR;
  T[2] = Parent.GetMaxRasterZ() - 0.001;
  return T;
}
//..............................................................................
vec3d TXGlLabel::GetVectorPosition() const {
  vec3d off = Parent.GetBasis().GetMatrix()*(GetCenter()+vec3d(-text_rect.width/2,text_rect.height/2,0));
  //vec3d off = Parent.GetBasis().GetMatrix()*GetCenter();
  const double Scale = Parent.GetScale();
  const double ScaleR = Parent.GetExtraZoom()*Parent.GetViewZoom();
  if( Transformer != NULL )  {
    vec3d T = Transformer->ForVector(*this);
    return Transformer->AdjustZ(
      T += (off*Parent.GetBasis().GetMatrix())*(Scale*ScaleR*Parent.GetBasis().GetZoom()));
  }
  vec3d T(Parent.GetBasis().GetCenter()+GetOffset());
  T += off*(Scale*ScaleR);
  T *= Parent.GetBasis().GetMatrix();
  T *= Parent.GetBasis().GetZoom();
  T[2] = Parent.GetMaxRasterZ() - 0.001;
  return T;
}
//..............................................................................
bool TXGlLabel::Orient(TGlPrimitive& P)  {
  const double Scale = Parent.GetScale();
  TGlFont& glf = GetFont();
  if( P.GetType() == sgloText )  {
    if( !glf.IsVectorFont() )  {
      vec3d T = GetRasterPosition();
      Parent.DrawTextSafe(T, FLabel, glf);
    }
    else  {
      vec3d T = GetVectorPosition();
      //float glw;
      //glGetFloatv(GL_LINE_WIDTH, &glw);
      //glLineWidth((float)(1./Scale)/50);
      glf.DrawVectorText(T, FLabel, Parent.GetBasis().GetZoom()/Parent.CalcZoom());
      //glLineWidth(glw);
    }
    return true;
  }
  else  {
    vec3d T = GetVectorPosition();
    T[2] -= 0.0005;
    olx_gl::translate(T);
    if( !glf.IsVectorFont() )  {
      olx_gl::scale(Scale, Scale, 1.0);
    }
    else  {
      const double scale = Parent.GetBasis().GetZoom()/Parent.CalcZoom();
      olx_gl::scale(scale, scale, 1.0);
    }
    P.Vertices[0] = vec3d(text_rect.left, text_rect.top, 0);
    P.Vertices[1] = vec3d(text_rect.left+text_rect.width, text_rect.top, 0);
    P.Vertices[2] = vec3d(text_rect.left+text_rect.width, text_rect.top+text_rect.height, 0);
    P.Vertices[3] = vec3d(text_rect.left, text_rect.top+text_rect.height, 0);
  }
  return false;
}
//..............................................................................
TGlFont& TXGlLabel::GetFont() const {  return Parent.GetScene().GetFont(FontIndex, true);  }
//..............................................................................
void TXGlLabel::ToDataItem(TDataItem& item) const {
  item.AddField("text", FLabel);
  item.AddField("visible", IsVisible());
  item.AddField("font_id", FontIndex);
  item.AddField("offset", PersUtil::VecToStr(GetOffset()));
  item.AddField("center", PersUtil::VecToStr(GetCenter()));
}
//..............................................................................
void TXGlLabel::FromDataItem(const TDataItem& item) {
  SetVisible( item.GetRequiredField("visible").ToBool() );
  FontIndex = item.GetRequiredField("font_id").ToInt();
  SetLabel(item.GetRequiredField("text"));
  TDataItem* basis = item.FindItem("Basis");
  if( basis != NULL )  {
    Offset = PersUtil::FloatVecFromStr(item.GetRequiredField("center"));
    TEBasis b;
    b.FromDataItem(*basis);
    _Center = b.GetCenter();
  }
  else  {
    Offset = PersUtil::FloatVecFromStr(item.GetRequiredField("offset"));
    _Center = PersUtil::FloatVecFromStr(item.GetRequiredField("center"));
  }
}
//..............................................................................

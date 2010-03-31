//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "gllightmodel.h"
#include "dataitem.h"

UseGlNamespace();
//..............................................................................
//..............................................................................
TGlLightModel::TGlLightModel()  {
  AmbientColor = 0x7f7f7f;
  for( int i = 0; i < 8; i++ )
    Lights[i].SetIndex(GL_LIGHT0 + i);
  ClearColor = 0x7f7f7f7f;
  Lights[0].SetEnabled(true);
  Lights[0].SetSpotCutoff(180);
  Lights[0].SetAmbient(0);
  Lights[0].SetDiffuse(0x7f7f7f);
  Lights[0].SetSpecular(0x7f7f7f);
  Lights[0].SetPosition(TGlOption(0, 0, 100, 1));
  Lights[0].SetAttenuation(TGlOption(1,0,0,0));
  Flags = 0;
  SetSmoothShade(true);
  SetTwoSides(true);
}
//..............................................................................
TGlLightModel& TGlLightModel::operator = (TGlLightModel& M)  {
  for( int i=0; i < 8; i++ )
    Lights[i] = M.GetLight(i);
  ClearColor = M.ClearColor;
  AmbientColor = M.AmbientColor;
  Flags = M.Flags;
  return *this;
}
//..............................................................................
void TGlLightModel::Init()  {
  olx_gl::lightModel(GL_LIGHT_MODEL_AMBIENT, AmbientColor.Data());
  olx_gl::shadeModel(IsSmoothShade() ? GL_SMOOTH : GL_FLAT);
  olx_gl::lightModel(GL_LIGHT_MODEL_LOCAL_VIEWER, IsLocalViewer() ? 1 : 0);
  olx_gl::lightModel(GL_LIGHT_MODEL_TWO_SIDE, IsTwoSides() ? 1 : 0);
  olx_gl::clearColor(ClearColor.Data());
  for( int i=0; i<8; i++ )  {
    if( Lights[i].IsEnabled() )  {
      olx_gl::light(Lights[i].GetIndex(), GL_AMBIENT, Lights[i].GetAmbient().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_DIFFUSE, Lights[i].GetDiffuse().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_SPECULAR, Lights[i].GetSpecular().Data());

      olx_gl::light(Lights[i].GetIndex(), GL_POSITION, Lights[i].GetPosition().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_SPOT_CUTOFF, Lights[i].GetSpotCutoff());

      olx_gl::light(Lights[i].GetIndex(), GL_SPOT_DIRECTION, Lights[i].GetSpotDirection().Data());
      olx_gl::light(Lights[i].GetIndex(), GL_SPOT_EXPONENT, Lights[i].GetSpotExponent());

      olx_gl::light(Lights[i].GetIndex(), GL_CONSTANT_ATTENUATION, Lights[i].GetAttenuation()[0]);
      olx_gl::light(Lights[i].GetIndex(), GL_LINEAR_ATTENUATION, Lights[i].GetAttenuation()[1]);
      olx_gl::light(Lights[i].GetIndex(), GL_QUADRATIC_ATTENUATION, Lights[i].GetAttenuation()[2]);
      olx_gl::enable(Lights[i].GetIndex());
    }
    else  {
      olx_gl::disable(Lights[i].GetIndex());
    }
  }
}
//..............................................................................
bool TGlLightModel::FromDataItem(const TDataItem& Item)  {
  Flags = Item.GetFieldValue("Flags").ToInt();
  AmbientColor.FromString(Item.GetFieldValue("Ambient"));
  ClearColor.FromString(Item.GetFieldValue("ClearColor"));
  for( int i=0; i < 8; i++ )  {
    TDataItem &SI = Item.FindRequiredItem(olxstr("Light") << i);
    Lights[i].FromDataItem(SI);
  }
  return true;
}
//..............................................................................
void TGlLightModel::ToDataItem(TDataItem& Item) const {
  Item.AddField("Flags", (int)Flags);  // need the cast or string will consider it as a char...
  Item.AddField("Ambient", AmbientColor.ToString());
  Item.AddField("ClearColor", ClearColor.ToString());
  for( int i=0; i < 8; i++ )
    Lights[i].ToDataItem(Item.AddItem(olxstr("Light") << i ));
}
 

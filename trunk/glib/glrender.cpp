//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "glrender.h"
#include "glscene.h"
#include "glgroup.h"
#include "styles.h"
#include "glbackground.h"
#include "gltexture.h"
#include "library.h"

#include "bapp.h"

UseGlNamespace();
//..............................................................................
TGraphicsStyles* TGlRenderer::FStyles = NULL;
//..............................................................................
TGlRenderer::TGlRenderer(AGlScene *S, int width, int height) :
  StereoRightColor(1, 0, 0, 1), StereoLeftColor(0, 1, 1, 1)
{
  CompiledListId = -1;
  FScene = S;
  FZoom = 1;
  FViewZoom = 1;
  FScene->Parent(this);
  FWidth = width;
  FHeight = height;
  FLeft = FTop = 0;
  FPerspective = false;
  FPAngle = 1;
  StereoFlag = 0;
  StereoAngle = 3;
  FOWidth = -1;

  LookAt(0,0,1);

  Fog = false;
  FogType = GL_EXP;
  FogColor = 0x7f7f7f;
  FogDensity = 1;
  FogStart = 0;
  FogEnd = 10;
  SceneDepth = -1;

  FStyles = new TGraphicsStyles(*this);

  FSelection = new TGlGroup(*this, "Selection");
  FSelection->SetSelected(true);

  OnDraw = &TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_DRAW);
  BeforeDraw = &TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_BEFORE_DRAW);
  OnStylesClear  = &TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_CLEAR_STYLES);
  //GraphicsStyles = FStyles;
  FBackground = new TGlBackground(*this, "Background", false);
  FBackground->SetVisible(false);
  FCeiling = new TGlBackground(*this, "Ceiling", true);
  FCeiling->SetVisible(false);
  FGlImageChanged = true; // will cause its update
  FGlImage = NULL;
  TextureManager = new TTextureManager();
  FTranslucentObjects.SetIncrement(512);
  FCollections.SetIncrement(512);
  FGObjects.SetIncrement(512);

  FSelection->Create();
}
//..............................................................................
TGlRenderer::~TGlRenderer()  {
  Clear();
  //GraphicsStyles = NULL;
  delete FStyles;
  delete FBackground;
  delete FCeiling;
  delete FSelection;
  delete FScene;
  delete TextureManager;
}
//..............................................................................
void TGlRenderer::Initialise()  {
  InitLights();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    Primitives.GetObject(i).Compile();
  FBackground->Create();
  FCeiling->Create();
  olxcstr vendor( (const char*)glGetString(GL_VENDOR) );
  ATI = vendor.StartsFrom("ATI");
}
//..............................................................................
void TGlRenderer::InitLights()  {
  SetView(true, 1);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glClearDepth(1.0);
  glDepthFunc(GL_LEQUAL);
  LightModel.Init();
}
//..............................................................................
void TGlRenderer::ClearPrimitives()  {
  ClearGroups();
  FSelection->Clear();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    glDeleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for( size_t i=0; i < FCollections.Count(); i++ )
    delete FCollections.GetObject(i);
  FCollections.Clear();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    delete &Primitives.GetObject(i);
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )
    delete &Primitives.GetProperties(i);
  Primitives.Clear();
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  FGObjects.Clear();
  ClearMinMax();
  ReleaseGlImage();
}
//..............................................................................
void TGlRenderer::Clear()  {
  FSelection->SetSelected(false);
  FSelection->Clear();
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
  FListManager.ClearLists();
  if( CompiledListId != -1 )  {
    glDeleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for( size_t i=0; i < FCollections.Count(); i++ )
    delete FCollections.GetObject(i);
  FCollections.Clear();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    delete &Primitives.GetObject(i);
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )
    delete &Primitives.GetProperties(i);
  Primitives.Clear();
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  // the function automaticallt removes the objects and their properties
  FGObjects.Clear();
  ResetBasis();
  ClearMinMax();
  ReleaseGlImage();
}
//..............................................................................
void TGlRenderer::ReleaseGlImage()  {
  if( FGlImage != NULL )  {
    delete [] FGlImage;
    FGlImage = NULL;
  }
}
//..............................................................................
void TGlRenderer::UpdateGlImage()  {
  ReleaseGlImage();
  FGlImage = GetPixels();
  FGlImageHeight = FHeight;
  FGlImageWidth = FWidth;
  FGlImageChanged = false;
}
//..............................................................................
void TGlRenderer::ClearMinMax()  {
  FMaxV[0] = FMaxV[1] = FMaxV[2] = -100;
  FMinV[0] = FMinV[1] = FMinV[2] = +100;
  SceneDepth = -1;
}
//..............................................................................
void TGlRenderer::UpdateMinMax(const vec3d& Min, const vec3d& Max)  {
  if( Max[0] > FMaxV[0] )  FMaxV[0] = Max[0];
  if( Max[1] > FMaxV[1] )  FMaxV[1] = Max[1];
  if( Max[2] > FMaxV[2] )  FMaxV[2] = Max[2];

  if( Min[0] < FMinV[0] )  FMinV[0] = Min[0];
  if( Min[1] < FMinV[1] )  FMinV[1] = Min[1];
  if( Min[2] < FMinV[2] )  FMinV[2] = Min[2];
}
//..............................................................................
void TGlRenderer::operator = (const TGlRenderer &G)  { ; }
//..............................................................................
void TGlRenderer::_OnStylesClear()  {
  OnStylesClear->Enter(this);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->SetStyle(NULL);
}
//..............................................................................
void TGlRenderer::_OnStylesLoaded()  {
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->SetStyle( &FStyles->NewStyle(FCollections.GetObject(i)->GetName(), true) );
  TPtrList<AGDrawObject> GO( FGObjects );
  for( size_t i=0; i < GO.Count(); i++ )
    GO[i]->OnPrimitivesCleared();
  ClearPrimitives();
  for( size_t i=0; i < GO.Count(); i++ )
    GO[i]->Create();
  OnStylesClear->Exit(this);
}
//..............................................................................
TGPCollection& TGlRenderer::NewCollection(const olxstr &Name)  {
  TGPCollection *GPC = FCollections.Add(Name, new TGPCollection(*this, Name)).Object;
  GPC->SetStyle( &FStyles->NewStyle(Name, true) );
  return *GPC;
}
//..............................................................................
int TGlRenderer_CollectionComparator(const olxstr& c1, const olxstr& c2) {
  const size_t l = olx_min(c1.Length(), c2.Length());
  int dc = 0;
  size_t i=0;
  for( ; i < l; i++ )  {
    if( c1.CharAt(i) != c2.CharAt(i) )  break;
    if( c1.CharAt(i) == '.' )
      dc++;
  }
  if( i == l )  {
    if( c1.Length() == c2.Length() )
      dc++;
    else {
      if( l < c1.Length() && c1.CharAt(l) == '.' )
        dc++;
      else
        dc--;
    }
  }
  return dc;
}
TGPCollection *TGlRenderer::FindCollectionX(const olxstr& Name, olxstr& CollName)  {
  const size_t di = Name.FirstIndexOf('.');
  if( di != InvalidIndex )  {
    size_t ind = FCollections.IndexOfComparable(Name);
    if( ind != InvalidIndex )  
      return FCollections.GetObject(ind);

    TGPCollection *BestMatch=NULL;
    short maxMatchLevels = 0;
    for( size_t i=0; i < FCollections.Count(); i++ )  {
      int dc = TGlRenderer_CollectionComparator(Name, FCollections.GetComparable(i));
      if( dc == 0 || dc < maxMatchLevels )  continue;
      if( BestMatch != NULL && dc == maxMatchLevels )  {  // keep the one with shortes name
        if( BestMatch->GetName().Length() > FCollections.GetComparable(i).Length() )
          BestMatch = FCollections.GetObject(i);
      }
      else
        BestMatch = FCollections.GetObject(i);
      maxMatchLevels = dc;
    }
    if( BestMatch != NULL )  {
      if( Name.StartsFrom( BestMatch->GetName() ) )  
        return BestMatch;
      CollName = Name.SubStringTo(di);
      return FindCollection(CollName);
    }
    CollName = Name.SubStringTo(di);
    return FindCollection(CollName);
  }
  else  {
    CollName = Name;
    return FindCollection(Name);
  }
}
//..............................................................................
TGlPrimitive& TGlRenderer::NewPrimitive(short type)  {
  return Primitives.AddObject(new TGlPrimitive(Primitives, *this, type));
}
//..............................................................................
void TGlRenderer::EnableFog(bool Set)  {
  glFogi(GL_FOG_MODE, FogType);
  glFogf(GL_FOG_DENSITY, FogDensity);
  glFogfv(GL_FOG_COLOR, FogColor.Data());
  glFogf(GL_FOG_START, FogStart);
  glFogf(GL_FOG_END, FogEnd);
  if( Set )
    glEnable(GL_FOG);
  else
    glDisable(GL_FOG);
  Fog = Set;
}
//..............................................................................
void TGlRenderer::EnablePerspective(bool Set)  {
  if( Set == FPerspective )  return;
  FPerspective = Set;
}
//..............................................................................
void TGlRenderer::SetPerspectiveAngle(double angle)  {
  FPAngle = (float)tan(angle*M_PI/360);
}
//..............................................................................
void TGlRenderer::Resize(int w, int h)  {
  Resize(0, 0, w, h, 1);
}
//..............................................................................
void TGlRenderer::Resize(int l, int t, int w, int h, float Zoom)  {
  FLeft = l;
  FTop = t;
  if( StereoFlag == glStereoCross )  {
    FWidth = w/2;
    FOWidth = w;
  }
  else
    FWidth = w;
  FHeight = h;
  FZoom = Zoom;
  FGlImageChanged = true;
}
//..............................................................................
void TGlRenderer::SetView(bool i, short Res)  {  SetView(0, 0, i , false, Res);  }
//..............................................................................
void TGlRenderer::SetZoom(double V) {  
  //const double MaxZ = olx_max(FMaxV.DistanceTo(FMinV), 1);
  //double dv = V*MaxZ;
  if( V < 0.001 )  //  need to fix the zoom
    FBasis.SetZoom(0.001);
  else if( V > 100 )
    FBasis.SetZoom(100);
  else
    FBasis.SetZoom(V); 
}
//..............................................................................
void TGlRenderer::SetView(int x, int y, bool identity, bool Select, short Res)  {
  glViewport(FLeft*Res, FTop*Res, FWidth*Res, FHeight*Res);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if( Select )  {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    gluPickMatrix(x, FHeight-y, 2, 2, vp);
  }
  const double aspect = (double)FWidth/(double)FHeight;
  if( !identity )  {
    if( FPerspective )  {
      double right = FPAngle*aspect;
      glFrustum(right*FProjectionLeft, right*FProjectionRight,
        FPAngle*FProjectionTop, FPAngle*FProjectionBottom, 1, 10);
    }
    else  {
      glOrtho(aspect*FProjectionLeft, aspect*FProjectionRight,
        FProjectionTop, FProjectionBottom, 1, 10);
    }
    //glTranslated(0, 0, FMinV[2] > 0 ? -1 : FMinV[2]-FMaxV[2]);
    glTranslated(0, 0, -2);
  }
  else  {
    glOrtho(aspect*FProjectionLeft, aspect*FProjectionRight,
      FProjectionTop, FProjectionBottom, -1, 1);
  }
  glMatrixMode(GL_MODELVIEW);
  /* Mxv ->
    x = {(Bf[0][0]*x+Bf[0][1]*y+Bf[0][2]*z+Bf[0][3]*w)},
    y = {(Bf[1][0]*x+Bf[1][1]*y+Bf[1][2]*z+Bf[1][3]*w)},
    z = {(Bf[2][0]*x+Bf[2][1]*y+Bf[2][2]*z+Bf[2][3]*w)},
    w = {(Bf[3][0]*x+Bf[3][1]*y+Bf[3][2]*z+Bf[3][3]*w)}
  */
  if( !identity )  {
    static float Bf[4][4];
    memcpy(&Bf[0][0], GetBasis().GetMData(), 12*sizeof(float));
    Bf[3][0] = Bf[3][1] = 0;
    Bf[3][2] = -1;
    Bf[3][3] = 1;
    glLoadMatrixf(&Bf[0][0]);
    const double dv = GetBasis().GetZoom();
    glScaled(dv, dv, dv);
    glTranslated(GetBasis().GetCenter()[0], GetBasis().GetCenter()[1], GetBasis().GetCenter()[2]);
  }
  else  {
    LoadIdentity();
  }
  //glDepthRange(1, 0);
}
//..............................................................................
void TGlRenderer::Draw()  {
  if( FWidth < 50 || FHeight < 50 || !TBasicApp::GetInstance().IsMainFormVisible() )  return;
  glEnable(GL_NORMALIZE);
  BeforeDraw->Execute(this);
  //glLineWidth( (float)(0.07/GetScale()) );
  //glPointSize( (float)(0.07/GetScale()) );  
  GetScene().StartDraw();

  if( StereoFlag == glStereoColor )  {
    const double ry = GetBasis().GetRY(), ra = StereoAngle;
    GetBasis().RotateY(ry-ra);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glColorMask(
      StereoRightColor[0] != 0 ? GL_TRUE : GL_FALSE,
      StereoRightColor[1] != 0 ? GL_TRUE : GL_FALSE,
      StereoRightColor[2] != 0 ? GL_TRUE : GL_FALSE,
      StereoRightColor[3] != 0 ? GL_TRUE : GL_FALSE);
    glColor4fv(StereoRightColor.Data());
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry+ra);

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glColorMask(
      StereoLeftColor[0] != 0 ? GL_TRUE : GL_FALSE,
      StereoLeftColor[1] != 0 ? GL_TRUE : GL_FALSE,
      StereoLeftColor[2] != 0 ? GL_TRUE : GL_FALSE,
      StereoLeftColor[3] != 0 ? GL_TRUE : GL_FALSE);
    glColor4fv(StereoLeftColor.Data());
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  }
  // http://local.wasp.uwa.edu.au/~pbourke/texture_colour/anaglyph/
  else if( StereoFlag == glStereoAnaglyph )  {
    const double ry = GetBasis().GetRY();
    GetBasis().RotateY(ry-StereoAngle);
    glClearAccum(0.0,0.0,0.0,0.0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(
      StereoRightColor[0] != 0 ? GL_TRUE : GL_FALSE,
      StereoRightColor[1] != 0 ? GL_TRUE : GL_FALSE,
      StereoRightColor[2] != 0 ? GL_TRUE : GL_FALSE,
      StereoRightColor[3] != 0 ? GL_TRUE : GL_FALSE);
    DrawObjects(0, 0, false, false);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glAccum(GL_LOAD, 1);

    GetBasis().RotateY(ry+StereoAngle);
    if( !IsATI() )
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(
      StereoLeftColor[0] != 0 ? GL_TRUE : GL_FALSE,
      StereoLeftColor[1] != 0 ? GL_TRUE : GL_FALSE,
      StereoLeftColor[2] != 0 ? GL_TRUE : GL_FALSE,
      StereoLeftColor[3] != 0 ? GL_TRUE : GL_FALSE);
    DrawObjects(0, 0, false, false);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glAccum(GL_ACCUM, 1);
    glAccum(GL_RETURN, 1.0);
    GetBasis().RotateY(ry);
  }
  else if( StereoFlag == glStereoCross )  {
    const double ry = GetBasis().GetRY();
    int _l = FLeft;
    GetBasis().RotateY(ry-StereoAngle);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry+StereoAngle);
    FLeft = FWidth;
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    FLeft = _l;
  }
  else
    DrawObjects(0, 0, false, false);
  GetScene().EndDraw();
  FGlImageChanged = true;
  OnDraw->Execute(this);
}
//..............................................................................
void TGlRenderer::DrawObjects(int x, int y, bool SelectObjects, bool SelectPrimitives)  {
  const bool Select = SelectObjects || SelectPrimitives;
  const bool skip_mat = StereoFlag==glStereoColor;
  static const int DrawMask = sgdoVisible|sgdoSelected|sgdoDeleted|sgdoGrouped;
  if( !FIdentityObjects.IsEmpty() || FSelection->GetGlM().IsIdentityDraw() )  {
    SetView(x, y, true, Select, 1);
    const size_t id_obj_count = FIdentityObjects.Count();
    for( size_t i=0; i < id_obj_count; i++ )  {
      TGlMaterial* GlM = FIdentityObjects[i];
      if( !Select )  GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
          else if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
          glPopMatrix();
        }
      }
    }
    if( FSelection->GetGlM().IsIdentityDraw() )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      FSelection->Draw(SelectPrimitives, SelectObjects);
      glPopAttrib();
    }
    SetView(x, y, false, Select, 1);
  }
  else  {
    SetView(x, y, false, Select, 1);
  }

  if( !Select && IsCompiled() )  {
    glCallList(CompiledListId);
  }
  else  {
    const size_t prim_count = Primitives.PropertiesCount();
    for( size_t i=0; i < prim_count; i++ )  {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if( GlM.IsIdentityDraw() ) continue;  // already drawn
      if( GlM.IsTransparent() ) continue;  // will be drawn
      if( !Select )  GlM.Init(skip_mat);
      const size_t obj_count = GlM.ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if( GPC == NULL )  continue;
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
          else if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
          glPopMatrix();
        }
      }
    }
  }
  const size_t trans_obj_count = FTranslucentObjects.Count();
  for( size_t i=0; i < trans_obj_count; i++ )  {
    TGlMaterial* GlM = FTranslucentObjects[i];
    if( !Select )  GlM->Init(skip_mat);
    const size_t obj_count = GlM->ObjectCount();
    for( size_t j=0; j < obj_count; j++ )  {
      TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
      TGPCollection* GPC = GlP.GetParentCollection();
      const size_t c_obj_count = GPC->ObjectCount();
      for( size_t k=0; k < c_obj_count; k++ )  {
        AGDrawObject& GDO = GPC->GetObject(k);
        if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
        if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
        else if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
        glPushMatrix();
        if( GDO.Orient(GlP) )  // the object has drawn itself
        {  glPopMatrix(); continue; }
        GlP.Draw();
        glPopMatrix();
      }
    }
  }
  const size_t group_count = FGroups.Count();
  for( size_t i=0; i < group_count; i++ )
    FGroups[i]->Draw(SelectPrimitives, SelectObjects);

  if( !FSelection->GetGlM().IsIdentityDraw() )  {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    FSelection->Draw(SelectPrimitives, SelectObjects);
    glPopAttrib();
  }
  if( !FTranslucentIdentityObjects.IsEmpty() )  {
    SetView(x, y, true, Select, 1);
    const size_t trans_id_obj_count = FTranslucentIdentityObjects.Count();
    for( size_t i=0; i < trans_id_obj_count; i++ )  {
      TGlMaterial* GlM = FTranslucentIdentityObjects[i];
      if( !Select )  GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount(); 
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( GDO.MaskFlags(DrawMask) != sgdoVisible )  continue;
          if( SelectObjects )  glLoadName((GLuint)GDO.GetTag());
          else if( SelectPrimitives )  glLoadName((GLuint)GlP.GetTag());
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
          glPopMatrix();
        }
      }
    }
  }
}
//..............................................................................
AGDrawObject* TGlRenderer::SelectObject(int x, int y, int depth)  {
  if( FWidth*FHeight == 0 )  
    return NULL;

  AGDrawObject *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];

  for( size_t i=0; i < ObjectCount(); i++ )
    GetObject(i).SetTag((int)(i+1));
  GetScene().StartSelect(x, y, selectBuf);
  DrawObjects(x, y, true, false);
  int hits = GetScene().EndSelect();
  if (hits >= 1)  {
    if( hits == 1 )  {
      GLuint in = selectBuf[(hits-1)*4+3];
      if( in >=1 && in <= ObjectCount() )  
        Result = &GetObject(in-1);
    }
    else  {
      unsigned int maxz = ~0;
      GLuint in=0;
      for( int i=0; i < hits; i++ )  {
        if( selectBuf[i*4+1] < maxz )  {
          in = i;
          maxz = selectBuf[i*4+1];
        }
      }
      if( (int)(in-depth)*4+3 < 0 )  return NULL;
      in = selectBuf[(in-depth)*4+3] - 1;
      if( in < ObjectCount() )  
        Result = &GetObject(in);
    }
  }
  delete [] selectBuf;
  return Result;
}
//..............................................................................
TGlPrimitive* TGlRenderer::SelectPrimitive(int x, int y)  {
  if( (FWidth&FHeight) == 0 )  // test for 0
    return NULL;

  TGlPrimitive *Result = NULL;
  GLuint *selectBuf = new GLuint [MAXSELECT];
  const size_t prim_count = Primitives.ObjectCount();
  for( size_t i=0; i < prim_count; i++ )
    Primitives.GetObject(i).SetTag( (int)(i+1) );

  GetScene().StartSelect(x, y, selectBuf);
  DrawObjects(x, y, false, true);
  GetScene().EndSelect();

  int hits = glRenderMode(GL_RENDER);
  if( hits >= 1 )  {
    if( hits == 1 )  {
      GLuint in = selectBuf[(hits-1)*4+3];
      if( in >=1 && in <= (PrimitiveCount()+1) )
        Result = &GetPrimitive(in-1);
    }
    else  {
      unsigned int maxz = ~0;
      GLuint in=0;
      for( int i=0; i < hits; i++ )  {
        if( selectBuf[i*4+1] < maxz )  {
          in = i;
          maxz = selectBuf[i*4+1];
        }
      }
      in = selectBuf[in*4+3];
      if( in >=1 && in <= (PrimitiveCount()+1) )
        Result = &GetPrimitive(in-1);
    }
  }
  delete [] selectBuf;
  return Result;
}
//..............................................................................
TGlGroup* TGlRenderer::FindObjectGroup(AGDrawObject& G)  {
  // get the topmost group
  TGlGroup* G1 = G.GetParentGroup();
  if( G1 == NULL )  
    return NULL;
  while( G1->GetParentGroup() != NULL )  {
    if( G1->GetParentGroup() == FSelection )  break;
    G1 = G1->GetParentGroup(); 
  }
  return (G1 == FSelection) ? NULL : G1;
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G)  {
  if( !G.IsGroupable() )  return;
  if( G.GetPrimitives().PrimitiveCount() != 0 )  {
    if( FSelection->IsEmpty() )  {
      TGlMaterial glm = FSelection->GetGlM();
      if( glm.IsIdentityDraw() != G.GetPrimitives().GetPrimitive(0).GetProperties().IsIdentityDraw() )  {
        glm.SetIdentityDraw(G.GetPrimitives().GetPrimitive(0).GetProperties().IsIdentityDraw());
        FSelection->SetGlM(glm);
      }
    }
    else  {
      if( FSelection->GetGlM().IsIdentityDraw() != G.GetPrimitives().GetPrimitive(0).GetProperties().IsIdentityDraw() )
        return;
    }
  }
  G.SetSelected(FSelection->Add(G));
}
//..............................................................................
void TGlRenderer::DeSelect(AGDrawObject& G)  {
  FSelection->Remove(G);
}
//..............................................................................
void TGlRenderer::InvertSelection()  {
  TPtrList<AGDrawObject> Selected;
  const size_t oc = FGObjects.Count();
  for( size_t i=0; i < oc; i++ )  {
    AGDrawObject* GDO = FGObjects[i];
    if( !GDO->IsGrouped() && GDO->IsVisible() )  {
      if( GDO->GetPrimitives().PrimitiveCount() != 0 &&
        FSelection->GetGlM().IsIdentityDraw() != GDO->GetPrimitives().GetPrimitive(0).GetProperties().IsIdentityDraw())
          continue;
      if( !GDO->IsSelected() && GDO->IsGroupable() && GDO != FSelection )
        Selected.Add(GDO);
    }
  }
  FSelection->SetSelected(false);
  FSelection->Clear();
  for( size_t i=0; i < Selected.Count(); i++ )
    Selected[i]->SetSelected(FSelection->Add(*Selected[i]));
}
//..............................................................................
void TGlRenderer::SelectAll(bool Select)  {
  FSelection->SetSelected(false);
  FSelection->Clear();
  if( Select )  {
    for( size_t i=0; i < ObjectCount(); i++ )  {
      AGDrawObject& GDO = GetObject(i);
      if( !GDO.IsGrouped() && GDO.IsVisible() && GDO.IsGroupable() )  {
        if( GDO.GetPrimitives().PrimitiveCount() != 0 &&
          FSelection->GetGlM().IsIdentityDraw() != GDO.GetPrimitives().GetPrimitive(0).GetProperties().IsIdentityDraw())
          continue;
        if( EsdlInstanceOf(GDO, TGlGroup) )  {
          if( &GDO == FSelection )  continue;
          bool Add = false;
          for( size_t j=0; j < ((TGlGroup&)GDO).Count(); j++ )  {
            if( ((TGlGroup&)GDO).GetObject(j).IsVisible() )  {
              Add = true;
              break;
            }
          }
          if( Add )  
            FSelection->Add(GDO);
        }
        else
          FSelection->Add(GDO);
      }
    }
  }
  FSelection->SetSelected(true);
}
//..............................................................................
void TGlRenderer::ClearGroups()  {
  for( size_t i=0; i < FGroups.Count(); i++ )  {
    if( FGroups[i]->IsSelected() )  
      DeSelect(*FGroups[i]);
    FGroups[i]->Clear();
  }
  // just in case of groups in groups
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::FindGroupByName(const olxstr& colName)  {
  for( size_t i=0; i < FGroups.Count(); i++ )
    if( FGroups[i]->GetCollectionName() == colName )
      return FGroups[i];
  return NULL;
}
//..............................................................................
void TGlRenderer::ClearSelection()  {
  FSelection->Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::GroupSelection(const olxstr& groupName)  {
  if( FSelection->Count() > 1 )  {
    TGlGroup *OS = FSelection;
    FGroups.Add(FSelection);
    FSelection = new TGlGroup(*this, "Selection");
    FSelection->Create();
    FSelection->Add(*OS);
    // read style information for this particular group
    OS->GetPrimitives().RemoveObject(*OS);
    FGObjects.Remove(OS);  // avoid duplication in the list!
    OS->Create(groupName);
    return OS;
  }
  return NULL;
}
//..............................................................................
TGlGroup& TGlRenderer::NewGroup(const olxstr& collection_name) {  
  return *FGroups.Add(new TGlGroup(*this, collection_name));  
}
//..............................................................................
void TGlRenderer::UnGroup(TGlGroup& OS)  {
  if( !OS.IsGroup() )  return;
  FGroups.Remove(OS);
  if( FSelection->Contains(OS) )
    FSelection->Remove(OS);

  TPtrList<AGDrawObject> Objects(OS.Count());
  for( size_t i=0; i < OS.Count(); i++ )
    Objects[i] = &OS[i];
  OS.GetPrimitives().RemoveObject(OS); // 
  FGObjects.Remove(OS);
  delete &OS;  // it will reset Parent group to NULL in the objects
  for( size_t i=0; i < Objects.Count(); i++ )
    FSelection->Add( *Objects[i] );
  FSelection->SetSelected(true);
}
//..............................................................................
void TGlRenderer::UnGroupSelection()  {
  if( FSelection->Count() >= 1 )  {
    for( size_t i=0; i < FSelection->Count(); i++ )  {
      AGDrawObject& GDO = FSelection->GetObject(i);
      if( GDO.IsGroup() )
        UnGroup((TGlGroup&)GDO);
    }
  }
}
//..............................................................................
void TGlRenderer::EnableClipPlane(TGlClipPlane *P, bool v)  {
  if( v )  {
    double v[4];
    v[0] = P->Equation()[0];
    v[1] = P->Equation()[1];
    v[2] = P->Equation()[2];
    v[3] = P->Equation()[3];
    glClipPlane( P->Id(), &v[0]);
    glEnable(P->Id());
  }
  else
    glDisable(P->Id());
}
//..............................................................................
void TGlRenderer::SetProperties(TGlMaterial& P)  {  // tracks translucent and identity objects
  if( P.IsTransparent() && P.IsIdentityDraw() )  {
    FTranslucentIdentityObjects.AddUnique(&P);
    return;
  }
  if( P.IsTransparent() )  {
    FTranslucentObjects.AddUnique(&P);
    return;
  }
  if( P.IsIdentityDraw() )  {
    FIdentityObjects.AddUnique(&P);
    return;
  }
}
//..............................................................................
void TGlRenderer::OnSetProperties(const TGlMaterial& P)  {  // tracks translucent and identity objects
  //if( P == NULL )  return;
  if( P.ObjectCount() > 1 )  return; // the properties will not be removde
  if( P.IsTransparent() && P.IsIdentityDraw() )  {
    size_t index = FTranslucentIdentityObjects.IndexOf(&P);
    if( index != InvalidIndex )  
      FTranslucentIdentityObjects.Delete(index);
    return;
  }
  if( P.IsTransparent() )  {
    size_t index = FTranslucentObjects.IndexOf(&P);
    if( index != InvalidIndex )  
      FTranslucentObjects.Delete(index);
    return;
  }
  if( P.IsIdentityDraw() )  {
    size_t index = FIdentityObjects.IndexOf(&P);
    if( index != InvalidIndex ) 
      FIdentityObjects.Delete(index);
    return;
  }
}
//..............................................................................
void TGlRenderer::RemoveObjects(const TPtrList<AGDrawObject>& objects)  {
  for( size_t i=0; i < FGObjects.Count(); i++ )
    FGObjects[i]->SetTag(-1);
  for( size_t i=0; i < objects.Count(); i++ )
    objects[i]->SetTag(0);
  for( size_t i=0; i < FGObjects.Count(); i++ )
    if( FGObjects[i]->GetTag() == 0 )
      FGObjects[i] = NULL;
  FGObjects.Pack();
}
//..............................................................................
void TGlRenderer::AddObject(AGDrawObject& G)  {
  FGObjects.Add(G);
  if( FSceneComplete || !G.IsVisible() )  return;
  vec3d MaxV, MinV;
  if( G.GetDimensions(MaxV, MinV) )
    UpdateMinMax(MinV, MaxV);
}
//..............................................................................
/*
void TGlRenderer::ReplacePrimitives(TEList *CurObj, TEList *NewObj)
{
  if( CurObj->Count() != NewObj->Count() )
    BasicApp->Log->Exception("TGlRenderer:: lists count does not much!", true);
  Primitives.ReplaceObjects(CurObj, NewObj);
} */
//..............................................................................
void TGlRenderer::RemoveCollection(TGPCollection& GP)  {
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();

  for( size_t i=0; i < PrimitiveCount(); i++ )
    GetPrimitive(i).SetTag(-1);
  for( size_t i=0; i < GP.PrimitiveCount(); i++ )
    GP.GetPrimitive(i).SetTag(0);
  Primitives.RemoveObjectsByTag(0);
  FCollections.Delete( FCollections.IndexOfObject(&GP) );

  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw()  )
      FTranslucentIdentityObjects.Add(GlM);
    else if( GlM.IsTransparent() )
      FTranslucentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )
      FIdentityObjects.Add(GlM);
  }
  delete &GP;
}
//..............................................................................
void TGlRenderer::RemoveCollections(const TPtrList<TGPCollection>& Colls)  {
  if( Colls.Count() == 0 )  return;

  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();

  for( size_t i=0; i < PrimitiveCount(); i++ )
    GetPrimitive(i).SetTag(-1);
  for( size_t i=0; i < Colls.Count(); i++ )  {
    for( size_t j=0; j < Colls[i]->PrimitiveCount(); j++ )
      Colls[i]->GetPrimitive(j).SetTag(0);
    Primitives.RemoveObjectsByTag(0);
    size_t col_ind = FCollections.IndexOfObject(Colls[i]);
    FCollections.Remove( col_ind );
    delete Colls[i];
  }
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw() )
      FTranslucentIdentityObjects.Add(&GlM);
    else if( GlM.IsTransparent() )   
      FTranslucentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )  
      FIdentityObjects.Add(GlM);
  }
}
//..............................................................................
void TGlRenderer::LookAt(double x, double y, short res)  {
  FViewZoom = (float)(1.0/res);
  FProjectionLeft = (float)(x/(double)res - 0.5);
  FProjectionRight = (float)((x+1)/(double)res - 0.5);
  FProjectionTop = (float)(y/(double)res - 0.5);
  FProjectionBottom = (float)((y+1)/(double)res - 0.5);
}
//..............................................................................
char* TGlRenderer::GetPixels(bool useMalloc, short aligment)  {
  char *Bf;
  short extraBytes = (4-(FWidth*3)%4)%4;  //for bitmaps with 4 bytes aligment
  if( useMalloc )  {
    Bf = (char*)malloc((FWidth*3+extraBytes)*FHeight);
  }
  else  {
    Bf = new char[(FWidth*3+extraBytes)*FHeight];
  }
  if( Bf == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  glReadBuffer(GL_BACK);
  glPixelStorei(GL_PACK_ALIGNMENT, aligment);
  glReadPixels(0, 0, FWidth, FHeight, GL_RGB, GL_UNSIGNED_BYTE, Bf);
  return Bf;
}
//..............................................................................
void TGlRenderer::RemovePrimitiveByTag(int in)  {
  Primitives.RemoveObjectsByTag(in);
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if( GlM.IsTransparent() && GlM.IsIdentityDraw() )
      FTranslucentIdentityObjects.Add(GlM);
    else if( GlM.IsTransparent() )    
      FTranslucentObjects.Add(GlM);
    else if( GlM.IsIdentityDraw() )   
      FIdentityObjects.Add(GlM);
  }
}
//..............................................................................
void TGlRenderer::CleanUpStyles()  {// removes styles, which are not used by any collection
  OnStylesClear->Enter(this);
  GetStyles().SetStylesTag(0);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->GetStyle().SetTag(1);
  GetStyles().RemoveStylesByTag(0);
  OnStylesClear->Exit(this);
}
//...........TGLLISTMANAGER...................................................//
//............................................................................//
//............................................................................//
//............................................................................//
TGlListManager::TGlListManager()  {
  FInc = 10;
  FPos = 0;
}
//..............................................................................
TGlListManager::~TGlListManager()  {
  ClearLists();
}
//..............................................................................
unsigned int TGlListManager::NewList()  {
  if( FPos >= Lists.Count() )  {
    unsigned int s = glGenLists(10);
    if( s == GL_INVALID_VALUE || s == GL_INVALID_OPERATION )
      throw TFunctionFailedException(__OlxSourceInfo, "glGenLists");
    for( int i=0; i < 10; i++ )
      Lists.Add(s+i);
  }
  return Lists[FPos ++];
}
//..............................................................................
void TGlListManager::ClearLists()  {
  for( size_t i=0; i < Lists.Count(); i+= FInc )
    glDeleteLists(Lists[i], FInc);
  FPos = 0;
  Lists.Clear();
}
//..............................................................................
void TGlListManager::ReserveRange(unsigned int count)  {
  ClearLists();
  unsigned int s = glGenLists(count);
  if( s == GL_INVALID_VALUE || s == GL_INVALID_OPERATION )
    throw TFunctionFailedException(__OlxSourceInfo, "glGenLists");
  for( unsigned int i=0; i < count; i++ )
    Lists.Add( s+i );
  FPos = (unsigned int)Lists.Count();
}
//..............................................................................
TGraphicsStyles& TGlRenderer::_GetStyles()  {
  if( TGlRenderer::FStyles == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "Object is not initialised");
  return *TGlRenderer::FStyles;
}
//..............................................................................
void TGlRenderer::Compile(bool v)  {
  /* remark: it works, but gives no performance boost ... */
  //return;
  if( v )  {
    if( CompiledListId == -1 )  {
      CompiledListId = glGenLists(1);
    }
    glNewList(CompiledListId, GL_COMPILE);
    for( size_t i=0; i < Primitives.PropertiesCount(); i++ )  {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if( GlM.IsIdentityDraw() ) continue;  // already drawn
      if( GlM.IsTransparent() ) continue;  // will be drawn
      GlM.Init(false);
      for( size_t j=0; j < GlM.ObjectCount(); j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if( GPC == NULL )  continue;
        for( size_t k=0; k < GPC->ObjectCount(); k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if( !GDO.IsVisible() )  continue;
          if( GDO.IsSelected() ) continue;
          if( GDO.IsGrouped() ) continue;
          glPushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  glPopMatrix(); continue; }
          GlP.Draw();
          glPopMatrix();
        }
      }
    }
    glEndList();
  }
  else  {
    if( CompiledListId != -1 )  {
      glDeleteLists(CompiledListId, 1);
      CompiledListId = -1;
    }
  }
}
//..............................................................................
void TGlRenderer::DrawText(TGlPrimitive& p, double x, double y, double z)  {
  if( ATI )  {
    glRasterPos3d(0, 0, 0);
    glCallList(p.GetFont()->GetFontBase() + ' ');
  }
  glRasterPos3d(x, y, z);
  p.Draw();
}
//..............................................................................
void TGlRenderer::DrawTextSafe(const vec3d& pos, const olxstr& text, const TGlFont& fnt) {
  if( ATI )  {
    glRasterPos3d(0, 0, 0);
    glCallList(fnt.GetFontBase() + ' ');
  }
  // set a valid raster position
  glRasterPos3d(0, 0, pos[2]);
  glBitmap(0, 0, 0, 0, (float)(pos[0]/FViewZoom), (float)(pos[1]/FViewZoom), NULL);
  for( size_t i=0; i < text.Length(); i++ ) 
    glCallList(fnt.GetFontBase() + text.CharAt(i));
}
//..............................................................................
//..............................................................................
//..............................................................................

void TGlRenderer::LibCompile(const TStrObjList& Params, TMacroError& E)  {
  Compile( Params[0].ToBool() );
}
//..............................................................................
void TGlRenderer::LibPerspective(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {  EnablePerspective(false);  return;  }
  if( !Cmds[0].IsNumber() )  {
    E.ProcessingError(__OlxSrcInfo, "please specify a number in range [1-90]" );
    return;
  }
  float v = (float)Cmds[0].ToDouble();
  if( v < 0.5 )  v = 1;
  if( v > 180 )  v = 180;

  SetPerspectiveAngle(v);
  EnablePerspective(true);
}
//..............................................................................
void TGlRenderer::LibFog(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    SetFogType(GL_LINEAR);
    SetFogStart(0.0f);
    SetFogEnd(2.0f);
    SetFogColor(Cmds[0].SafeUInt<uint32_t>());
    EnableFog(true);
  }
  else
    EnableFog(false);
}
//..............................................................................
void TGlRenderer::LibZoom(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    SetZoom(CalcZoom());
  }
  else if( Cmds.Count() == 1 ) {
    double zoom = GetZoom() + Cmds[0].ToDouble();
    if( zoom < 0.001 )  zoom = 0.001;
    SetZoom(zoom);
  }
}
//..............................................................................
void TGlRenderer::LibCalcZoom(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(CalcZoom());
}
//..............................................................................
void TGlRenderer::LibStereo(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  {
    if( StereoFlag == glStereoColor )
      E.SetRetVal<olxstr>("color");
    else if( StereoFlag == glStereoCross )
      E.SetRetVal<olxstr>("cross");
    else if( StereoFlag == glStereoAnaglyph )
      E.SetRetVal<olxstr>("anaglyph");
    else
      E.SetRetVal<olxstr>("none");
  }
  else  {
    if( FOWidth > 0 )  {
      FWidth = FOWidth;
      FOWidth = -1;
    }
    if( Params[0].Equalsi("color") )
      StereoFlag = glStereoColor;
    else if( Params[0].Equalsi("anaglyph") )
      StereoFlag = glStereoAnaglyph;
    else if( Params[0].Equalsi("cross") )  {
      FOWidth = FWidth;
      FWidth /= 2;
      StereoFlag = glStereoCross;
    }
    else
      StereoFlag = 0;
    if( Params.Count() == 2 )
      StereoAngle = Params[1].ToDouble();
  }
}
//..............................................................................
void TGlRenderer::LibStereoColor(const TStrObjList& Params, TMacroError& E)  {
  TGlOption* glo = Params[0].Equalsi("left") ? &StereoLeftColor : 
    (Params[0].Equalsi("right") ? &StereoRightColor : NULL);
  if( glo == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined parameter, left/right is expected");
    return;
  }
  if( Params.Count() == 1 )  {
    E.SetRetVal(glo->ToString());
  }
  else if( Params.Count() == 2 )  {
    *glo = Params[1].SafeInt<uint32_t>();
    (*glo)[3] = 1;
  }
  else if( Params.Count() == 4 )  {
    (*glo)[0] = Params[1].ToFloat<float>();
    (*glo)[1] = Params[2].ToFloat<float>();
    (*glo)[2] = Params[3].ToFloat<float>();
    (*glo)[3] = 1;
  }
}
//..............................................................................
TLibrary*  TGlRenderer::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("gl") : name);
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibCompile, "Compile",
    fpOne, "Compiles or decompiles the model according to the boolean parameter") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibPerspective, "Perspective",
    EmptyString, fpNone|fpOne, "Un/Sets perspective view") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibFog, "Fog",
    EmptyString, fpNone|fpOne, "fog color - sets fog, fog without arguments removes fog") );
  lib->RegisterMacro<TGlRenderer>( new TMacro<TGlRenderer>(this,  &TGlRenderer::LibZoom, "Zoom",
    EmptyString, fpNone|fpOne, "If no arguments provided - resets zoom to fit to screen, otherwise increments/\
decrements current zoom by provided value") );
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibCalcZoom, "CalcZoom",
    fpNone, "Returns optimal zoom value") );
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibStereo, "Stereo",
    fpNone|fpOne|fpTwo, "Returns/sets color/cross stereo mode and optionally stereo angle [3]") );
  lib->RegisterFunction<TGlRenderer>( new TFunction<TGlRenderer>(this,  &TGlRenderer::LibStereoColor,
    "StereoColor",
    fpOne|fpTwo|fpFour, "Returns/sets colors for left/right color stereo mode glasses") );

  return lib;
}
//..............................................................................


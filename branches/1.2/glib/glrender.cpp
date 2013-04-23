/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glrender.h"
#include "glgroup.h"
#include "styles.h"
#include "glbackground.h"
#include "gltexture.h"
#include "library.h"
#include "bapp.h"
#include "log.h"
#include "estrbuffer.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLuint TGlRenderer::TGlListManager::NewList()  {
  if( Pos >= Lists.Count()*Inc )  {
    GLuint s = olx_gl::genLists(Inc);
    if( s == GL_INVALID_VALUE || s == GL_INVALID_OPERATION )
      throw TFunctionFailedException(__OlxSourceInfo, "glGenLists");
    Lists.Add(s);
  }
  const GLuint lp = Lists.GetLast() + Pos%Inc;
  Pos++;
  return lp;
}
//..............................................................................
void TGlRenderer::TGlListManager::Clear()  {
  for( size_t i=0; i < Lists.Count(); i+= Inc )
    olx_gl::deleteLists(Lists[i], Inc);
  Pos = 0;
  Lists.Clear();
}
//..............................................................................
//..............................................................................
//..............................................................................
TGraphicsStyles* TGlRenderer::FStyles = NULL;
//..............................................................................
TGlRenderer::TGlRenderer(AGlScene *S, size_t width, size_t height) :
  Top(0), Left(0), Width((int)width), Height((int)height), OWidth(0),
  StereoLeftColor(0, 1, 1, 1), StereoRightColor(1, 0, 0, 1),
  OnDraw(TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_DRAW)),
  OnStylesClear(TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_CLEAR_STYLES)),
  OnClear(TBasicApp::GetInstance().NewActionQueue("GL_CLEAR"))
{
  poly_stipple = NULL;
  AbsoluteTop = 0;
  CompiledListId = -1;
  FScene = S;
  FZoom = 1;
  FViewZoom = 1;
  FScene->Parent(this);
  Selecting = FPerspective = false;
  FPAngle = 1;
  StereoFlag = 0;
  StereoAngle = 3;
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
  FBackground = new TGlBackground(*this, "Background", false);
  FBackground->SetVisible(false);
  FCeiling = new TGlBackground(*this, "Ceiling", true);
  FCeiling->SetVisible(false);
  FGlImageChanged = true; // will cause its update
  FGlImage = NULL;
  TextureManager = new TTextureManager();
  FTranslucentObjects.SetIncrement(16);
  FCollections.SetIncrement(16);
  FGObjects.SetIncrement(16);
  MaxRasterZ = 1;
  GLUSelection = true;
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
  if (poly_stipple != NULL)
    delete [] poly_stipple;
}
//..............................................................................
void TGlRenderer::Initialise()  {
  GetScene().MakeCurrent();
  InitLights();
  for( size_t i=0; i < Primitives.ObjectCount(); i++ )
    Primitives.GetObject(i).Compile();
  FSelection->Create();
  FBackground->Create();
  FCeiling->Create();
  olxcstr vendor((const char*)olx_gl::getString(GL_VENDOR));
  ATI = vendor.StartsFrom("ATI");
  olx_gl::get(GL_LINE_WIDTH, &LineWidth);
  GLUSelection = TBasicApp::GetInstance().GetOptions().FindValue(
    "gl_selection", vendor.StartsFrom("Intel")).ToBool();
  if (TBasicApp::GetInstance().GetOptions().FindValue(
    "gl_multisample", FalseString()).ToBool())
  {
    olx_gl::enable(GL_MULTISAMPLE);
  }
}
//..............................................................................
void TGlRenderer::InitLights()  {
  SetView(true, 1);
  olx_gl::enable(GL_LIGHTING);
  olx_gl::enable(GL_DEPTH_TEST);
  olx_gl::hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  olx_gl::clearDepth(1.0f);
  olx_gl::depthFunc(GL_LEQUAL);
  LightModel.Init();
}
//..............................................................................
void TGlRenderer::Clear()  {
  GetScene().MakeCurrent();
  OnClear.Enter(this);
  FSelection->Clear();
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
  ListManager.Clear();
  if( CompiledListId != -1 )  {
    olx_gl::deleteLists(CompiledListId, 1);
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
  OnClear.Exit(this);
}
//..............................................................................
void TGlRenderer::ClearObjects()  {
  GetScene().MakeCurrent();
  FSelection->Clear();
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
  if( CompiledListId != -1 )  {
    olx_gl::deleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->ClearObjects();
  FGObjects.Clear();
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
  GlImageHeight = Height;
  GlImageWidth = Width;
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
  OnStylesClear.Enter(this);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->SetStyle(NULL);
}
//..............................................................................
void TGlRenderer::_OnStylesLoaded()  {
  for( size_t i=0; i < FCollections.Count(); i++ ) {
    FCollections.GetObject(i)->SetStyle(
      &FStyles->NewStyle(FCollections.GetObject(i)->GetName(), true));
  }
  // groups are deleted by Clear, so should be removed!
  for( size_t i=0; i < FGroups.Count(); i++ )
    FGObjects.Remove(FGroups[i]);
  AGDObjList GO = FGObjects.GetList();
  for( size_t i=0; i < GO.Count(); i++ )  {
    GO[i]->OnPrimitivesCleared();
    GO[i]->SetCreated(false);
  }
  Clear();
  for( size_t i=0; i < GO.Count(); i++ )  {
    // some loose objects as labels can be created twice otherwise
    if( !GO[i]->IsCreated() )   {
      GO[i]->Create();
      GO[i]->SetCreated(true);
    }
  }
  TGraphicsStyle* gs = FStyles->FindStyle("GL.Stereo");
  if( gs != NULL )  {
    StereoLeftColor = gs->GetParam("left", StereoLeftColor.ToString(), true);
    StereoRightColor = gs->GetParam("right", StereoRightColor.ToString(), true);
    StereoAngle = gs->GetParam("angle", StereoAngle, true).ToDouble();
  }
  OnStylesClear.Exit(this);
}
//..............................................................................
TGPCollection& TGlRenderer::NewCollection(const olxstr &Name)  {
  TGPCollection *GPC =
    FCollections.Add(Name, new TGPCollection(*this, Name)).Object;
  GPC->SetStyle(&FStyles->NewStyle(Name, true));
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
TGPCollection *TGlRenderer::FindCollectionX(const olxstr& Name,
  olxstr& CollName)
{
  const size_t di = Name.FirstIndexOf('.');
  if( di != InvalidIndex )  {
    const size_t ind = FCollections.IndexOf(Name);
    if( ind != InvalidIndex )  
      return FCollections.GetObject(ind);

    TGPCollection *BestMatch=NULL;
    short maxMatchLevels = 0;
    for( size_t i=0; i < FCollections.Count(); i++ )  {
      int dc = TGlRenderer_CollectionComparator(Name, FCollections.GetKey(i));
      if( dc == 0 || dc < maxMatchLevels )  continue;
      // keep the one with shortes name
      if( BestMatch != NULL && dc == maxMatchLevels )  {
        if( BestMatch->GetName().Length() > FCollections.GetKey(i).Length() )
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
  olx_gl::fog(GL_FOG_MODE, FogType);
  olx_gl::fog(GL_FOG_DENSITY, FogDensity);
  olx_gl::fog(GL_FOG_COLOR, FogColor.Data());
  olx_gl::fog(GL_FOG_START, FogStart);
  olx_gl::fog(GL_FOG_END, FogEnd);
  if( Set )
    olx_gl::enable(GL_FOG);
  else
    olx_gl::disable(GL_FOG);
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
void TGlRenderer::Resize(size_t w, size_t h)  {
  Resize(0, 0, w, h, 1);
}
//..............................................................................
void TGlRenderer::Resize(int l, int t, size_t w, size_t h, float Zoom)  {
  Left = l;
  Top = t;
  if( StereoFlag == glStereoCross )  {
    Width = (int)w/2;
    OWidth = (int)w;
  }
  else
    Width = (int)w;
  Height = (int)h;
  FZoom = Zoom;
  FGlImageChanged = true;
  SetView(false);
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
  olx_gl::viewport(Left*Res, Top*Res, Width*Res, Height*Res);
  olx_gl::matrixMode(GL_PROJECTION);
  olx_gl::loadIdentity();
  if (Select && GLUSelection) {
    GLint vp[4];
    olx_gl::get(GL_VIEWPORT, vp);
    gluPickMatrix(x, Height-y, 3, 3, vp);
  }
  const double aspect = (double)Width/(double)Height;
  if( !identity )  {
    if( FPerspective )  {
      double right = FPAngle*aspect;
      olx_gl::frustum(right*FProjectionLeft, right*FProjectionRight,
        FPAngle*FProjectionTop, FPAngle*FProjectionBottom, 1, 10);
    }
    else  {
      olx_gl::ortho(aspect*FProjectionLeft, aspect*FProjectionRight,
        FProjectionTop, FProjectionBottom, 1, 10);
    }
    //glTranslated(0, 0, FMinV[2] > 0 ? -1 : FMinV[2]-FMaxV[2]);
    olx_gl::translate(0, 0, -2);
  }
  else  {
    olx_gl::ortho(aspect*FProjectionLeft, aspect*FProjectionRight,
      FProjectionTop, FProjectionBottom, -1, 1);
  }
  olx_gl::matrixMode(GL_MODELVIEW);
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
    olx_gl::loadMatrix(&Bf[0][0]);
    olx_gl::scale(GetBasis().GetZoom());
    olx_gl::translate(GetBasis().GetCenter());
  }
  else  {
    olx_gl::loadIdentity();
  }
  //glDepthRange(1, 0);
}
//..............................................................................
void TGlRenderer::SetupStencilFoInterlacedDraw(bool even) {
  if (poly_stipple == NULL || (poly_even != even)) {
    if (poly_stipple == NULL)
      poly_stipple = new GLubyte [128];
    // horizontal interlacing
    for (size_t i=0; i < 128; i+=8) {
      *((uint32_t*)(&poly_stipple[i])) = even ? 0 : ~0;
      *((uint32_t*)(&poly_stipple[i+4])) = even ? ~0 : 0;
    }
    poly_even = even;
    // this is for the vertical interlacing
    //memset(poly_stipple, even ? 0x55 : 0xAA, 128);
  }
  SetView(true);
  olx_gl::disable(GL_LIGHTING);
  olx_gl::disable(GL_DEPTH_TEST);

  olx_gl::clear(GL_STENCIL_BUFFER_BIT);
  olx_gl::stencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
  olx_gl::stencilFunc(GL_ALWAYS, 1, ~0);
  olx_gl::enable(GL_STENCIL_TEST);

  olx_gl::polygonStipple(poly_stipple);
  olx_gl::enable(GL_POLYGON_STIPPLE);
  olx_gl::colorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  const double aspect = (double)Width/(double)Height;
  glRectd(-0.5*aspect, -0.5, 0.5*aspect, 0.5);
  
  olx_gl::colorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  olx_gl::enable(GL_DEPTH_TEST);
  olx_gl::disable(GL_POLYGON_STIPPLE);
  olx_gl::enable(GL_LIGHTING);
}
//..............................................................................
void TGlRenderer::Draw()  {
  if( Width < 50 || Height < 50 )  return;
  GetScene().MakeCurrent();
  olx_gl::enable(GL_NORMALIZE);
  OnDraw.Enter(this);
  //glLineWidth( (float)(0.07/GetScale()) );
  //glPointSize( (float)(0.07/GetScale()) );
  if( StereoFlag == glStereoColor )  {
    olx_gl::clearColor(0.0,0.0,0.0,0.0);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const double ry = GetBasis().GetRY();
    olx_gl::disable(GL_ALPHA_TEST);
    olx_gl::disable(GL_BLEND);
    olx_gl::disable(GL_CULL_FACE);
    olx_gl::enable(GL_COLOR_MATERIAL);
    olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    // right eye
    GetBasis().RotateY(ry+StereoAngle);
    olx_gl::colorMask(
      StereoRightColor[0] != 0,
      StereoRightColor[1] != 0,
      StereoRightColor[2] != 0,
      StereoRightColor[3] != 0);
    olx_gl::color(StereoRightColor.Data());
    DrawObjects(0, 0, false, false);
    //left eye
    GetBasis().RotateY(ry-StereoAngle);
    olx_gl::clear(GL_DEPTH_BUFFER_BIT);
    olx_gl::enable(GL_BLEND);
    olx_gl::blendFunc(GL_ONE, GL_ONE);
    olx_gl::colorMask(
      StereoLeftColor[0] != 0,
      StereoLeftColor[1] != 0,
      StereoLeftColor[2] != 0,
      StereoLeftColor[3] != 0);
    olx_gl::color(StereoLeftColor.Data());
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    olx_gl::colorMask(true, true, true, true);
  }
  // http://local.wasp.uwa.edu.au/~pbourke/texture_colour/anaglyph/
  else if( StereoFlag == glStereoAnaglyph )  {
    const double ry = GetBasis().GetRY();
    olx_gl::clearColor(0.0,0.0,0.0,0.0);
    olx_gl::clearAccum(0.0,0.0,0.0,0.0);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // right eye
    GetBasis().RotateY(ry+StereoAngle);
    olx_gl::colorMask(
      StereoRightColor[0] != 0,
      StereoRightColor[1] != 0,
      StereoRightColor[2] != 0,
      StereoRightColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_LOAD, 1);
    // left eye
    GetBasis().RotateY(ry-StereoAngle);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    olx_gl::colorMask(
      StereoLeftColor[0] != 0,
      StereoLeftColor[1] != 0,
      StereoLeftColor[2] != 0,
      StereoLeftColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_ACCUM, 1);
    olx_gl::accum(GL_RETURN, 1.0);
    GetBasis().RotateY(ry);
  }
  else if( StereoFlag == glStereoHardware )  {
    const double ry = GetBasis().GetRY();
    GetBasis().RotateY(ry+StereoAngle);
    olx_gl::drawBuffer(GL_BACK_LEFT);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry-StereoAngle);
    olx_gl::drawBuffer(GL_BACK_RIGHT);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawObjects(0, 0, false, false);
    olx_gl::drawBuffer(GL_BACK);
    GetBasis().RotateY(ry);
  }
  else if( StereoFlag == glStereoInterlace )  {
    olx_gl::drawBuffer(GL_BACK);
    SetupStencilFoInterlacedDraw((AbsoluteTop%2) != 0);
    olx_gl::enable(GL_STENCIL_TEST);
    olx_gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    const double ry = GetBasis().GetRY();
    GetBasis().RotateY(ry+StereoAngle);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    olx_gl::stencilFunc(GL_EQUAL, 0, ~0);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry-StereoAngle);
    olx_gl::stencilFunc(GL_NOTEQUAL, 0, ~0);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    olx_gl::disable(GL_STENCIL_TEST);
  }
  else if( StereoFlag == glStereoCross )  {
    const double ry = GetBasis().GetRY();
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const int _l = Left;
    GetBasis().RotateY(ry+StereoAngle);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry-StereoAngle);
    Left = Width;
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    Left = _l;
  }
  else  {
    GetScene().StartDraw();
    DrawObjects(0, 0, false, false);
  }
  GetScene().EndDraw();
  FGlImageChanged = true;
  OnDraw.Execute(this);
  OnDraw.Exit(this);
}
//..............................................................................
void TGlRenderer::HandleSelection(const AGDrawObject &o, const TGlPrimitive &p,
  bool SelectObjects, bool SelectPrimitives) const
{
  if (GLUSelection) {
    if (SelectObjects)
      olx_gl::loadName((GLuint)o.GetTag());
    else if (SelectPrimitives)
      olx_gl::loadName((GLuint)p.GetTag());
  }
  else {
    if (SelectObjects)
      olx_gl::color(o.GetTag());
    else if (SelectPrimitives)
      olx_gl::color(p.GetTag());
  }
}
//..............................................................................
void TGlRenderer::DrawObjects(int x, int y, bool SelectObjects,
  bool SelectPrimitives)
{
#if defined(_DEBUG) && 0
  for( size_t i=0; i < PrimitiveCount(); i++ )  {
    GetPrimitive(i).SetFont(NULL);
    GetPrimitive(i).SetString(NULL);
  }
#endif
  olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
  Selecting = (SelectObjects || SelectPrimitives);
  const bool skip_mat = (StereoFlag==glStereoColor || Selecting);
  if (Selecting) {
    olx_gl::enable(GL_COLOR_MATERIAL);
    olx_gl::disable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
  }
  static const int DrawMask = sgdoSelected|sgdoGrouped|sgdoHidden;
  if( !FIdentityObjects.IsEmpty() || FSelection->GetGlM().IsIdentityDraw() )  {
    SetView(x, y, true, Selecting, 1);
    const size_t id_obj_count = FIdentityObjects.Count();
    for( size_t i=0; i < id_obj_count; i++ )  {
      TGlMaterial* GlM = FIdentityObjects[i];
      GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if (GDO.MaskFlags(DrawMask) != 0) continue;
          HandleSelection(GDO, GlP, SelectObjects, SelectPrimitives);
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
    if( FSelection->GetGlM().IsIdentityDraw() )  {
      olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
      FSelection->Draw(SelectPrimitives, SelectObjects);
      olx_gl::popAttrib();
    }
    SetView(x, y, false, Selecting, 1);
  }
  else  {
    SetView(x, y, false, Selecting, 1);
  }

  if( !Selecting && IsCompiled() )  {
    olx_gl::callList(CompiledListId);
  }
  else  {
    const size_t prim_count = Primitives.PropertiesCount();
    for( size_t i=0; i < prim_count; i++ )  {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if( GlM.IsIdentityDraw() ) continue;  // already drawn
      if( GlM.IsTransparent() ) continue;  // will be drawn
      GlM.Init(skip_mat);
      const size_t obj_count = GlM.ObjectCount();
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if( GPC == NULL )  continue;
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if (GDO.MaskFlags(DrawMask) != 0) continue;
          HandleSelection(GDO, GlP, SelectObjects, SelectPrimitives);
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
  }
  const size_t trans_obj_count = FTranslucentObjects.Count();
  //olx_gl::disable(GL_DEPTH_TEST);
  /* disabling the depth test does help for a set of transparent objects but
  then it does not help if there are any solid objects on the way
  */
  for( size_t i=0; i < trans_obj_count; i++ )  {
    TGlMaterial* GlM = FTranslucentObjects[i];
    GlM->Init(skip_mat);
    const size_t obj_count = GlM->ObjectCount();
    for( size_t j=0; j < obj_count; j++ )  {
      TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
      TGPCollection* GPC = GlP.GetParentCollection();
      const size_t c_obj_count = GPC->ObjectCount();
      for( size_t k=0; k < c_obj_count; k++ )  {
        AGDrawObject& GDO = GPC->GetObject(k);
        if (GDO.MaskFlags(DrawMask) != 0) continue;
        HandleSelection(GDO, GlP, SelectObjects, SelectPrimitives);
        olx_gl::pushMatrix();
        if( GDO.Orient(GlP) )  // the object has drawn itself
        {  olx_gl::popMatrix(); continue; }
        GlP.Draw();
        olx_gl::popMatrix();
      }
    }
  }
  //olx_gl::enable(GL_DEPTH_TEST);
  const size_t group_count = FGroups.Count();
  for( size_t i=0; i < group_count; i++ )  {
    if( FGroups[i]->GetParentGroup() == NULL )
      FGroups[i]->Draw(SelectPrimitives, SelectObjects);
  }

  if( !FSelection->GetGlM().IsIdentityDraw() )  {
    olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
    FSelection->Draw(SelectPrimitives, SelectObjects);
    olx_gl::popAttrib();
  }
  if (!FTranslucentIdentityObjects.IsEmpty()) {
    SetView(x, y, true, Selecting, 1);
    const size_t trans_id_obj_count = FTranslucentIdentityObjects.Count();
    for (size_t i=0; i < trans_id_obj_count; i++) {
      TGlMaterial* GlM = FTranslucentIdentityObjects[i];
      GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount(); 
      for( size_t j=0; j < obj_count; j++ )  {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for( size_t k=0; k < c_obj_count; k++ )  {
          AGDrawObject& GDO = GPC->GetObject(k);
          if (GDO.MaskFlags(DrawMask) != 0) continue;
          HandleSelection(GDO, GlP, SelectObjects, SelectPrimitives);
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
  }
  olx_gl::popAttrib();
}
//..............................................................................
AGDrawObject* TGlRenderer::SelectObject(int x, int y) {
  if( (Width*Height) <= 100 )  return NULL;
  for (size_t i=0; i < ObjectCount(); i++)
    GetObject(i).SetTag((int)(i+1));
  if (GLUSelection) {
    AGDrawObject *Result=NULL;
    GLuint *selectBuf = new GLuint [MAXSELECT];
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
        if( (int)(in)*4+3 < 0 )  return NULL;
        in = selectBuf[(in)*4+3] - 1;
        if( in < ObjectCount() )  
          Result = &GetObject(in);
      }
    }
    delete [] selectBuf;
    return Result;
  }
  else {
    SetView(x, y, false, true, 1);
    GetScene().StartDraw();
    DrawObjects(x, y, true, false);
    GetScene().EndDraw();
    memset(&SelectionBuffer, 0, sizeof(SelectionBuffer));
    olx_gl::readPixels(x-1, Height-y-1, 3, 3, GL_RGB, GL_UNSIGNED_BYTE,
      &SelectionBuffer[0]);
    static const size_t indices [9][2] = {
      {1,3}, {1,0}, {1,6},
      {0,3}, {3,3},
      {0,0}, {0,6}, {3,0}, {3,6}
    };
    for (int i=0; i < 9; i++) {
      size_t idx = OLX_RGB(
        SelectionBuffer[indices[i][0]][indices[i][1]],
        SelectionBuffer[indices[i][0]][indices[i][1]+1],
        SelectionBuffer[indices[i][0]][indices[i][1]+2])-1;
      if (idx < FGObjects.Count() && FGObjects[idx]->IsVisible())
        return FGObjects[idx];
    }
    return NULL;
  }
}
//..............................................................................
TGlPrimitive* TGlRenderer::SelectPrimitive(int x, int y) {
  if( (Width*Height) <= 100 )  return NULL;
  for (size_t i=0; i < Primitives.ObjectCount(); i++)
    Primitives.GetObject(i).SetTag((index_t)(i+1));
  if (GLUSelection) {
    TGlPrimitive *Result=NULL;
    GLuint *selectBuf = new GLuint [MAXSELECT];
    GetScene().StartSelect(x, y, selectBuf);
    DrawObjects(x, y, false, true);
    GetScene().EndSelect();
    int hits = olx_gl::renderMode(GL_RENDER);
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
  else {
    SetView(x, y, false, true, 1);
    GetScene().StartDraw();
    DrawObjects(x, y, false, true);
    GetScene().EndDraw();
    memset(&SelectionBuffer, 0, sizeof(SelectionBuffer));
    olx_gl::readPixels(x-1, Height-y-1, 3, 3, GL_RGB, GL_UNSIGNED_BYTE,
      &SelectionBuffer[0]);
    size_t idx = OLX_RGB(
      SelectionBuffer[1][3],
      SelectionBuffer[1][4],
      SelectionBuffer[1][5])-1;
    return (idx < PrimitiveCount()) ? &GetPrimitive(idx) : NULL;
  }
}
//..............................................................................
TGlGroup* TGlRenderer::FindObjectGroup(const AGDrawObject& G) const {
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
  G.SetSelected(FSelection->Add(G));
}
//..............................................................................
void TGlRenderer::DeSelect(AGDrawObject& G)  {
  if( G.GetParentGroup() == FSelection )
    FSelection->Remove(G);
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G, bool v)  {
  if( v )  {
    if( !G.IsSelected() )
      Select(G);
  }
  else if( G.IsSelected() )
    DeSelect(G);
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G, glSelectionFlag v)  {
  if (v == glSelectionSelect) {
    if (!G.IsSelected())
      Select(G);
  }
  else if (v == glSelectionUnselect) {
    if (G.IsSelected())
      DeSelect(G);
  }
  else if (v == glSelectionInvert) {
    if (!G.IsSelected())
      Select(G);
    else
      DeSelect(G);
  }
}
//..............................................................................
void TGlRenderer::InvertSelection()  {
  AGDObjList Selected;
  const size_t oc = FGObjects.Count();
  for( size_t i=0; i < oc; i++ )  {
    AGDrawObject* GDO = FGObjects[i];
    if( !GDO->IsGrouped() && GDO->IsVisible() )
      Selected.Add(GDO);
  }
  FSelection->Clear();
  for( size_t i=0; i < Selected.Count(); i++ )
    Selected[i]->SetSelected(FSelection->Add(*Selected[i]));
}
//..............................................................................
void TGlRenderer::SelectAll(bool Select)  {
  if( Select )  {
    for( size_t i=0; i < ObjectCount(); i++ )  {
      AGDrawObject& GDO = GetObject(i);
      if( !GDO.IsGrouped() && GDO.IsVisible() && GDO.IsSelectable() )  // grouped covers selected
        FSelection->Add(GDO);
    }
    FSelection->SetSelected(true);
  }
  else
    FSelection->Clear();
}
//..............................................................................
void TGlRenderer::ClearGroups()  {
  if( FGroups.IsEmpty() )  return;
  for( size_t i=0; i < FGroups.Count(); i++ )  {
    if( FGroups[i]->IsSelected() )
      DeSelect(*FGroups[i]);
    FGroups[i]->Clear();
  }
  for( size_t i=0; i < FGroups.Count(); i++ )
    delete FGroups[i];
  FGroups.Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::FindGroupByName(const olxstr& colName) const {
  for( size_t i=0; i < FGroups.Count(); i++ )
    if( FGroups[i]->GetCollectionName() == colName )
      return FGroups[i];
  return NULL;
}
//..............................................................................
void TGlRenderer::ClearSelection()  {  FSelection->Clear();  }
//..............................................................................
TGlGroup* TGlRenderer::GroupSelection(const olxstr& groupName)  {
  if( FSelection->Count() > 0 )  {
    AGDObjList ungroupable;
    if( !FSelection->TryToGroup(ungroupable) )
      return NULL;
    TGlGroup *OS = FSelection;
    FGroups.Add(FSelection);
    OS->GetPrimitives().RemoveObject(*OS);
    FSelection = new TGlGroup(*this, "Selection");
    FSelection->Create();
    for( size_t i=0; i < ungroupable.Count(); i++ )
      FSelection->Add(*ungroupable[i]);
    // read style information for this particular group
    OS->SetSelected(false);
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
  FGroups.Remove(OS);
  if( FSelection->Contains(OS) )
    FSelection->Remove(OS);

  AGDObjList Objects(OS.Count());
  for( size_t i=0; i < OS.Count(); i++ )
    Objects[i] = &OS[i];
  OS.GetPrimitives().RemoveObject(OS); // 
  FGObjects.Remove(&OS);
  delete &OS;  // it will reset Parent group to NULL in the objects
  for( size_t i=0; i < Objects.Count(); i++ )
    FSelection->Add( *Objects[i] );
  FSelection->SetSelected(true);
}
//..............................................................................
void TGlRenderer::EnableClipPlane(TGlClipPlane *P, bool v)  {
  if( v )  {
    double v[4];
    v[0] = P->Equation()[0];
    v[1] = P->Equation()[1];
    v[2] = P->Equation()[2];
    v[3] = P->Equation()[3];
    olx_gl::clipPlane( P->Id(), &v[0]);
    olx_gl::enable(P->Id());
  }
  else
    olx_gl::disable(P->Id());
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
// tracks translucent and identity objects
void TGlRenderer::OnSetProperties(const TGlMaterial& P)  {
  //if( P == NULL )  return;
  if( P.ObjectCount() > 1 )  return; // the properties will not be removed
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
void TGlRenderer::RemoveObjects(const AGDObjList& objects)  {
  ACollectionItem::Exclude<>(FGObjects, objects);
}
//..............................................................................
void TGlRenderer::AddObject(AGDrawObject& G)  {
  FGObjects.AddUnique(&G);
  if( FSceneComplete || !G.IsVisible() )
    return;
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

  Primitives.GetObjects().ForEach(ACollectionItem::TagSetter(-1));
  GP.GetPrimitives().ForEach(ACollectionItem::TagSetter(0));
  Primitives.RemoveObjectsByTag(0);
  FCollections.Delete(FCollections.IndexOfObject(&GP));
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
  if( Colls.IsEmpty() )  return;
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();

  Primitives.GetObjects().ForEach(ACollectionItem::TagSetter(-1));
  for( size_t i=0; i < Colls.Count(); i++ )  {
    Colls[i]->GetPrimitives().ForEach(ACollectionItem::TagSetter(0));
    const size_t col_ind = FCollections.IndexOfObject(Colls[i]);
    FCollections.Delete(col_ind);
    delete Colls[i];
  }
  Primitives.RemoveObjectsByTag(0);
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
char* TGlRenderer::GetPixels(bool useMalloc, short aligment, GLuint format)  {
  GetScene().MakeCurrent();
  char *Bf;
  short extraBytes = aligment-(Width*3)%aligment;
  if( useMalloc )  {
    Bf = (char*)malloc((Width*3+extraBytes)*Height);
  }
  else  {
    Bf = new char[(Width*3+extraBytes)*Height];
  }
  if( Bf == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  olx_gl::readBuffer(GL_BACK);
  olx_gl::pixelStore(GL_PACK_ALIGNMENT, aligment);
  olx_gl::readPixels(0, 0, Width, Height, format, GL_UNSIGNED_BYTE, Bf);
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
  OnStylesClear.Enter(this);
  GetStyles().SetStylesTag(0);
  for( size_t i=0; i < FCollections.Count(); i++ )
    FCollections.GetObject(i)->GetStyle().SetTag(1);
  GetStyles().RemoveStylesByTag(0);
  OnStylesClear.Exit(this);
}
//............................................................................//
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
      CompiledListId = olx_gl::genLists(1);
    }
    olx_gl::newList(CompiledListId, GL_COMPILE);
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
          olx_gl::pushMatrix();
          if( GDO.Orient(GlP) )  // the object has drawn itself
          {  olx_gl::popMatrix(); continue; }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
    olx_gl::endList();
  }
  else  {
    if( CompiledListId != -1 )  {
      olx_gl::deleteLists(CompiledListId, 1);
      CompiledListId = -1;
    }
  }
}
//..............................................................................
void TGlRenderer::DrawText(TGlPrimitive& p, double x, double y, double z)  {
  p.GetFont()->Reset_ATI(ATI);
  olx_gl::rasterPos(x, y, z);
  p.Draw();
}
//..............................................................................
void TGlRenderer::DrawTextSafe(const vec3d& pos, const olxstr& text, const TGlFont& fnt) {
  fnt.Reset_ATI(ATI);
  // set a valid raster position
  olx_gl::rasterPos(0.0, 0.0, pos[2]);
  olx_gl::bitmap(0, 0, 0, 0, (float)(pos[0]/FViewZoom), (float)(pos[1]/FViewZoom), NULL);
  fnt.DrawRasterText(text);
}
//..............................................................................
void TGlRenderer::SetLineWidth(double lw) {
  LineWidth = lw;
  olx_gl::lineWidth(LineWidth);
}
//..............................................................................
void TGlRenderer::BeforeContextChange()  {
  //OnClear.Enter(this);
  Clear();
  //OnClear.SetEnabled(false);
  TextureManager->BeforeContextChange();
}
//..............................................................................
void TGlRenderer::AfterContextChange()  {
  TextureManager->AfterContextChange();
  //OnClear.SetEnabled(true);
  //OnClear.Exit(this);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TGlRenderer::LibCompile(const TStrObjList& Params, TMacroError& E)  {
  Compile(Params[0].ToBool());
}
//..............................................................................
void TGlRenderer::LibPerspective(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( Cmds.IsEmpty() )  {  EnablePerspective(false);  return;  }
  if( !Cmds[0].IsNumber() )  {
    E.ProcessingError(__OlxSrcInfo, "please specify a number in range [1-90]" );
    return;
  }
  double v = Cmds[0].ToDouble();
  if( v < 0.5f )  v = 1;
  if( v > 180 )  v = 180;

  SetPerspectiveAngle(v);
  EnablePerspective(true);
}
//..............................................................................
void TGlRenderer::LibFog(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if (Cmds.Count() == 1) {
    SetFogType(GL_LINEAR);
    SetFogStart(0.9f);
    SetFogEnd(1.4f);
    SetFogColor(Cmds[0].SafeUInt<uint32_t>());
    EnableFog(true);
  }
  else if (Cmds.Count() == 3) {
    SetFogType(GL_LINEAR);
    SetFogStart(Cmds[1].ToFloat());
    SetFogEnd(Cmds[2].ToFloat());
    SetFogColor(Cmds[0].SafeUInt<uint32_t>());
    EnableFog(true);
  }
  else
    EnableFog(false);
}
//..............................................................................
void TGlRenderer::LibZoom(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
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
void TGlRenderer::LibGetZoom(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(GetZoom());
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
    else if( StereoFlag == glStereoHardware )
      E.SetRetVal<olxstr>("hardware");
    else
      E.SetRetVal<olxstr>("none");
  }
  else  {
    if( OWidth != 0 )  {
      Width = OWidth;
      OWidth = 0;
    }
    if( Params[0].Equalsi("color") )
      StereoFlag = glStereoColor;
    else if( Params[0].Equalsi("anaglyph") )  {
      GLint bits = 0;
      olx_gl::get(GL_ACCUM_RED_BITS, &bits);
      if( bits == 0 ) {
        TBasicApp::NewLogEntry(logError) <<
          "Sorry accumulation buffer is not initialised/available";
      }
      else
        StereoFlag = glStereoAnaglyph;
    }
    else if( Params[0].Equalsi("interlace") )  {
      GLint bits = 0;
      olx_gl::get(GL_STENCIL_BITS, &bits);
      if( bits == 0 ) {
        TBasicApp::NewLogEntry(logError) <<
          "Sorry stencil buffer is not initialised/available";
      }
      else
        StereoFlag = glStereoInterlace;
    }
    else if( Params[0].Equalsi("cross") )  {
      olx_gl::clearColor(LightModel.GetClearColor().Data());
      OWidth = Width;
      Width /= 2;
      StereoFlag = glStereoCross;
    }
    else if( Params[0].Equalsi("hardware") )  {
      GLboolean stereo_supported = GL_FALSE;
      olx_gl::get(GL_STEREO, &stereo_supported);
      if( stereo_supported == GL_FALSE ) {
        TBasicApp::NewLogEntry(logError) <<
          "Sorry stereo buffers are not initialised/available";
      }
      else  {
        olx_gl::clearColor(LightModel.GetClearColor().Data());
        StereoFlag = glStereoHardware;
      }
    }
    else  {
      olx_gl::clearColor(LightModel.GetClearColor().Data());
      StereoFlag = 0;
    }
    if( Params.Count() == 2 )
      StereoAngle = Params[1].ToDouble();
  }
  TGraphicsStyle& gs = FStyles->NewStyle("GL.Stereo", true);
  gs.SetParam("angle", StereoAngle, true);
}
//..............................................................................
void TGlRenderer::LibStereoColor(const TStrObjList& Params, TMacroError& E)  {
  TGlOption* glo = Params[0].Equalsi("left") ? &StereoLeftColor : 
    (Params[0].Equalsi("right") ? &StereoRightColor : NULL);
  if( glo == NULL )  {
    E.ProcessingError(__OlxSrcInfo,
      "undefined parameter, left/right is expected");
    return;
  }
  if( Params.Count() == 1 )  {
    E.SetRetVal(glo->ToString());
  }
  if( Params.Count() == 2 )  {
    *glo = Params[1].SafeUInt<uint32_t>();
    (*glo)[3] = 1;
  }
  else if( Params.Count() == 4 )  {
    (*glo)[0] = Params[1].ToFloat();
    (*glo)[1] = Params[2].ToFloat();
    (*glo)[2] = Params[3].ToFloat();
    (*glo)[3] = 1;
  }
  TGraphicsStyle& gs = FStyles->NewStyle("GL.Stereo", true);
  gs.SetParam("left", StereoLeftColor.ToString(), true);
  gs.SetParam("right", StereoRightColor.ToString(), true);
}
//..............................................................................
void TGlRenderer::LibLineWidth(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(GetLineWidth());
  else
    SetLineWidth(Params[0].ToDouble());
}
//..............................................................................
void TGlRenderer::LibBasis(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  {
    TDataItem di(NULL, EmptyString());
    TEStrBuffer out;
    GetBasis().ToDataItem(di);
    di.SaveToStrBuffer(out);
    E.SetRetVal(out.ToString());
  }
  else  {
    TDataItem di(NULL, EmptyString());
    di.LoadFromString(0, Params[0], NULL);
    GetBasis().FromDataItem(di);
  }
}
//..............................................................................
void TGlRenderer::LibRasterZ(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(GetMaxRasterZ());
  else
    MaxRasterZ = Params[0].ToDouble();
}
//..............................................................................
TLibrary*  TGlRenderer::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("gl") : name);
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibCompile, "Compile",
      fpOne,
      "Compiles or decompiles the model according to the boolean parameter")
  );
  lib->Register(
    new TMacro<TGlRenderer>(this,  &TGlRenderer::LibPerspective, "Perspective",
      EmptyString(), fpNone|fpOne,
      "Un/Sets perspective view")
  );
  lib->Register(
    new TMacro<TGlRenderer>(this,  &TGlRenderer::LibFog, "Fog",
      EmptyString(), fpNone|fpOne|fpThree,
      "Sets fog color, fog without arguments removes fog")
  );
  lib->Register(
    new TMacro<TGlRenderer>(this,  &TGlRenderer::LibZoom, "Zoom",
      EmptyString(), fpNone|fpOne,
      "If no arguments provided - resets zoom to fit to screen, otherwise "
      "increments/decrements current zoom by provided value")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibCalcZoom, "CalcZoom",
      fpNone, "Returns optimal zoom value")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibGetZoom, "GetZoom",
      fpNone, "Returns current zoom value")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibStereo, "Stereo",
      fpNone|fpOne|fpTwo,
      "Returns/sets color/cross/anaglyph/hardware stereo mode and optionally "
      "stereo angle [3]")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibStereoColor,
      "StereoColor",
      fpOne|fpTwo|fpFour,
      "Returns/sets colors for left/right color stereo mode glasses")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibLineWidth,
      "LineWidth",
      fpNone|fpOne,
      "Returns/sets width of the raster OpenGl line")
      );
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibBasis,
      "Basis",
      fpNone|fpOne,
      "Returns/sets view basis")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this,  &TGlRenderer::LibRasterZ,
      "RasterZ",
      fpNone|fpOne,
      "Returns/sets maximum value of the raster Z 1 or -1 is typically "
      "expected")
  );
  lib->AttachLibrary(LightModel.ExportLibrary("lm"));
  lib->AttachLibrary(GetScene().ExportLibrary("scene"));
  return lib;
}
//..............................................................................

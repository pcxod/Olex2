/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xgrid_H
#define __olx_gxl_xgrid_H
#include "gxbase.h"
#include "arrays.h"
#include "gltextbox.h"
#include "macroerror.h"
#include "gltexture.h"
#include "glprimitive.h"
#ifdef __WXWIDGETS__
#include "wx/zipstrm.h"
#endif
#include "fracmask.h"
//#include "ps_writer.h"
#include "xblob.h"
#include "symmat.h"
#include "maputil.h"
#include "glbitmap.h"
#include "olxmps.h"
BeginGxlNamespace()

const short
  planeDrawModeOriginal = 1,
  planeDrawModeGradient = 2;
const short
  planeRenderModePlane   = 0x0001,
  planeRenderModeContour = 0x0002,
  planeRenderModePoint   = 0x0004,
  planeRenderModeFill    = 0x0008,
  planeRenderModeLine    = 0x0010;

//class TXGrid: public TGlMouseListener  {
class TXGrid: public AGDrawObject  {
  class TLegend : public AGlMouseHandlerImp  {
    int Width, Height;
    int Top, Left;
    GLuint TextureId;
    double Z;
    TGlMaterial GlM;
  protected:
    vec3d Center;
    virtual bool DoTranslate(const vec3d& t) {  Center += t;  return true;  }
    virtual bool DoRotate(const vec3d&, double) {  return false;  }
    virtual bool DoZoom(double zoom, bool inc)  { return false; }
    const vec3d& GetCenter() const {  return Center;  }
    void Init(unsigned char* RGB, GLenum format);
  public:
    TLegend(TGlRenderer& Render, const olxstr& collectionName);
    void SetData(unsigned char* RGB, GLsizei width, GLsizei height,
      GLenum format);
    void Create(const olxstr& cName=EmptyString());
    void SetMaterial(const TGlMaterial &m) { GlM = m; }
    void Fit();
    virtual bool Orient(TGlPrimitive& P);
    virtual bool GetDimensions(vec3d &, vec3d &) { return false; }
    bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
    void UpdateLabel() { Fit(); }
    TStrList text;
  };

  TArray3D<float>* ED;
  FractMask* Mask;
  // if mask is specified
  GLuint PListId, NListId;
  char *TextData, *LegendData;
  float **ContourData;
  float *ContourCrds[2], *ContourLevels;
  size_t ContourLevelCount;
  //TGlPrimitive *FPrimitive;
  class TGXApp * XApp;
  void DeleteObjects();
  GLuint TextIndex;
  static TXGrid* Instance;
  void RescaleSurface();
  TGlTextBox *Info;
  TLegend *Legend;
  short RenderMode;
  bool Extended, Boxed, Loading_;
  vec3f ExtMin, ExtMax;
  TGlPrimitive* glpP, *glpN, *glpC;
  // these will keep the masked objects
  TTypeList<vec3f> p_vertices, n_vertices;
  TTypeList<vec3f> p_normals, n_normals;
  TTypeList<IsoTriangle> p_triangles, n_triangles;
protected:
  float MaxVal, MinVal, Depth, Size, Scale;
  size_t MaxX, MaxY, MaxZ, MaxDim; 
  float MinHole, MaxHole;  // the values of scale to skip
  int LastMouseX, LastMouseY;
  bool MouseDown;
  void DoSmooth();
  void GlLine(float x1, float y1, float x2, float y2, float z);
  int GetPolygonMode() const {  return RenderMode == planeRenderModeFill ? GL_FILL :
    (RenderMode == planeRenderModeLine ? GL_LINE : 
    (RenderMode == planeRenderModePoint ? GL_POINT : -1));
  }
  bool Is3D() const {  return RenderMode == planeRenderModeFill ||
    RenderMode == planeRenderModeLine ||
    RenderMode == planeRenderModePoint;
  }
  // updates the text information regarding current map
  void UpdateInfo();
  class TContextClear: public AActionHandler  {
  public:
    TContextClear(TGlRenderer& Render);
    virtual ~TContextClear()  {}
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
    bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static void _ResetLists()  {
    if( Instance != NULL )  {
      Instance->PListId = Instance->NListId = ~0;
    }
  }
public:
  TXGrid(const olxstr& collectionName, TGXApp* xapp);
  virtual ~TXGrid();
  void Clear();
  inline TArray3D<float>* Data()  {  return ED;  }
  inline const TArray3D<float>* Data() const {  return ED;  }
  bool LoadFromFile(const olxstr& GridFile);

  void InitIso();
  void InitGrid(size_t maxX, size_t maxY, size_t MaxZ);
  void InitGrid(const vec3s& dim)  {  InitGrid(dim[0], dim[1], dim[2]);  }
  inline void SetValue(size_t i, size_t j, size_t k, float v) {
    ED->Data[i][j][k] = v;
  }
  inline double GetValue(int i, int j, int k) const {
    return ED->Data[i][j][k];
  }
  template <class T> void SetValue(const T& ind, float v)  {
    ED->Data[(int)ind[0]][(int)ind[1]][(int)ind[2]] = v;
  }
  template <class T> inline float GetValue(const T& v) const {
    return ED->Data[(int)v[0]][(int)v[1]][(int)v[2]];
  }
  
  // copies the 0yz x0z and xy0 layers to Maxyz xMaxyz and xyMaxZ
  void AdjustMap();
  virtual void Create(const olxstr& cName=EmptyString());

  virtual bool Orient(TGlPrimitive& P);
  virtual bool GetDimensions(vec3d& Max, vec3d& Min);

  void SetScale(float v);
  inline double GetScale() const {  return Scale;  }
  // this object will be deleted
  void SetMask(FractMask& fm) {  Mask = &fm;  }

  // extends the grid by +-1
  bool IsExtended() const {  return Extended;  }
  void SetExtended(bool v);

  void SetDepth(float v);
  void SetDepth(const vec3d& v);
  float GetDepth() const {  return Depth;  }
  vec3s GetDimVec() const {  return vec3s(MaxX, MaxY, MaxZ);  }
  size_t GetPlaneSize() const {  return MaxDim;  }
  /* v=2^n values are acepted only (64, 128, 256, etc to be compatible with textures) */
  void SetPlaneSize(size_t v);
  float GetSize() const {  return Size;  }

  const TTypeList<vec3f> &GetPVertices() const {  return p_vertices;  }
  const TTypeList<vec3f> &GetPNormals() const {  return p_normals;  }
  const TTypeList<IsoTriangle> &GetPTriangles() const {  return p_triangles;  }
  const TTypeList<vec3f> &GetNVertices() const {  return n_vertices;  }
  const TTypeList<vec3f> &GetNNormals() const {  return n_normals;  }
  const TTypeList<IsoTriangle> &GetNTriangles() const {  return n_triangles;  }
  
  DefPropP(float, MinHole)
  DefPropP(float, MaxHole)
  DefPropP(float, MinVal)
  DefPropP(float, MaxVal)

  inline bool IsEmpty() const {  return ED == NULL;  }
  short GetRenderMode() const {  return RenderMode;  }
  
  size_t GetContourLevelCount() const {  return ContourLevelCount;  }
  // sets new number of contours...
  void SetContourLevelCount(size_t v);

  inline virtual void SetVisible(bool On) {  
    AGDrawObject::SetVisible(On);  
    Info->SetVisible(On);
    Legend->SetVisible(On);
    if( !On )
      Clear();
  }

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data);

  inline static TXGrid* GetInstance()  {  return Instance;  }

  void LibScale(const TStrObjList& Params, TMacroError& E);
  void LibExtended(const TStrObjList& Params, TMacroError& E);
  void LibSize(const TStrObjList& Params, TMacroError& E);
  void LibDepth(const TStrObjList& Params, TMacroError& E);
  void LibMaxDepth(const TStrObjList& Params, TMacroError& E);
  void LibContours(const TStrObjList& Params, TMacroError& E);
  void LibPlaneSize(const TStrObjList& Params, TMacroError& E);
  void LibGetMin(const TStrObjList& Params, TMacroError& E);
  void LibGetMax(const TStrObjList& Params, TMacroError& E);
  void LibRenderMode(const TStrObjList& Params, TMacroError& E);
  void LibIsvalid(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString());
#ifdef _PYTHON
  static void PyInit();
#endif  
  void ToDataItem(TDataItem& item, IOutputStream& zos) const;
  void FromDataItem(const TDataItem& item, IInputStream& zis);
  // creates a blob at given screen coordinates (raster position)
  TXBlob* CreateBlob(int x, int y) const;
  const_strlist ToPov(olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials) const;
protected:
  struct TPlaneCalculationTask : public TaskBase {
    TXGrid& parent;
    float **data, ***src_data;
    char *text_data;
    const mat3f &proj_m, &c2c;
    const vec3f& center;
    const vec3s& dim;
    float minVal, maxVal, size, depth, hh;
    size_t max_dim;
    short mode;
    MapUtil::MapGetter<float,2> map_getter;
    void Run(size_t index);
    TPlaneCalculationTask(TXGrid& _parent, float*** _src_data, float** _data,
      char* _text_data, size_t _max_dim, float _size,
      float _depth, const mat3f& _proj_m, const mat3f& _c2c,
      const vec3f& _center, const vec3s& _dim, short _mode) :
        parent(_parent),
        data(_data), src_data(_src_data), text_data(_text_data),
        proj_m(_proj_m), c2c(_c2c), center(_center), dim(_dim),
        minVal(1000), maxVal(-1000),
        size(_size), depth(_depth), hh((float)_max_dim/2), max_dim(_max_dim),
        mode(_mode),
        map_getter(src_data, dim) {}
    TPlaneCalculationTask* Replicate() const {
      return new TPlaneCalculationTask(parent, src_data, data, text_data,
        max_dim, size, depth, proj_m, c2c, center, dim, mode);
    }
  };
};

EndGxlNamespace()
#endif
#ifndef __olx_ort_Draw_H
#define __olx_ort_Draw_H
#include "gxapp.h"
#include "ps_writer.h"

static const uint16_t
  ortep_color_lines = 0x0001,
  ortep_color_fill  = 0x0002,
  ortep_color_bond  = 0x0004,
  ortep_atom_rims   = 0x0008,
  ortep_atom_quads  = 0x0010;

class OrtDraw;

static const vec3f NullVec;

struct a_ort_object {
  const OrtDraw& parent;
public:
  a_ort_object(const OrtDraw& _parent) : parent(_parent) {} 
  virtual ~a_ort_object(){}
  virtual void render(PSWriter&) const = 0;
  virtual float get_z() const = 0;
};

struct ort_atom : public a_ort_object {
  const TXAtom& atom;
  vec3f crd;
  mat3f *p_elpm, *p_ielpm;
  double draw_rad;
  uint16_t draw_style;
  uint32_t sphere_color, rim_color, mask;
  ort_atom(const OrtDraw& parent, const TXAtom& a);
  ~ort_atom()  {
    if( p_elpm != NULL )  delete p_elpm;
    if( p_ielpm != NULL )  delete p_ielpm;
  }
  virtual void render(PSWriter&) const;
  virtual float get_z() const {  return crd[2];  }
  bool IsSpherical() const;
  bool IsSolid() const {  return mask != 0 && mask != 16;  }
protected:
  void render_rims(PSWriter&) const;
  void render_elp(PSWriter&) const;
  void render_sph(PSWriter&) const;
  void render_standalone(PSWriter&) const;
};

struct ort_bond : public a_ort_object  {
  const TXBond &bond;
  const ort_atom &atom_a, &atom_b;
  uint16_t draw_style;
  ort_bond(const OrtDraw& parent, const TXBond& _bond, const ort_atom& a1, const ort_atom& a2);
  virtual void render(PSWriter&) const;
  virtual float get_z() const {  return (atom_a.crd[2]+atom_b.crd[2])/2;  }
protected:
  void _render(PSWriter&, float scalex, uint32_t mask) const;
};

struct ort_poly : public a_ort_object  {
  vec3f_list points;
  bool fill;
  float line_width;
  uint32_t color;
  ort_poly(const OrtDraw& parent, bool _fill) :
    a_ort_object(parent),
    fill(_fill),
    line_width(1.0f),
    color(0x0) {  }
  virtual void render(PSWriter&) const;
  virtual float get_z() const {
    float z = 0;
    for( size_t i=0; i < points.Count(); i++ )
      z += points[i][2];
    return points.IsEmpty() ? 0 : z/points.Count();
  }
};

struct ort_circle : public a_ort_object  {
  bool fill;
  float line_width, r;
  uint32_t color;
  vec3f center;
  ort_circle(const OrtDraw& parent, const vec3f& _center, float _r, bool _fill) :
    a_ort_object(parent),
    center(_center),
    r(_r),
    fill(_fill),
    line_width(1.0f),
    color(0x0) {  }
  virtual void render(PSWriter&) const;
  virtual float get_z() const {  return center[2];  }
};

class OrtDraw  {
private:
  static int OrtObjectsZSort(const a_ort_object& a1, const a_ort_object& a2)  {
    const float diff = a1.get_z() - a2.get_z();
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  static int OrtObjectsPtrZSort(const a_ort_object* a1, const a_ort_object* a2)  {
    const float diff = a1->get_z() - a2->get_z();
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  float DrawScale, BondRad, LinearScale;
  mat3f ProjMatr, UnProjMatr;
  vec3f DrawOrigin, SceneOrigin;
  TGXApp& app;
  uint16_t ColorMode;
protected:
  float PieLineWidth, 
    ElpLineWidth, 
    QuadLineWidth,
    FontLineWidth,
    HBondScale;
  const TEBasis& basis;
  bool Perspective;
  uint16_t ElpDiv, PieDiv, BondDiv;
  mutable TArrayList<vec3f> ElpCrd, PieCrd, Arc, BondCrd, BondProjF, BondProjT, BondProjM;
  mutable TPtrList<const vec3f> FilteredArc;
  size_t PrepareArc(const TArrayList<vec3f>& in, TPtrList<const vec3f>& out, const vec3f& normal) const;
  float GetBondRad(const ort_bond& b, uint32_t mask) const;
  vec3f ProjectPoint(const vec3f& p) const {  return (p + SceneOrigin)*ProjMatr+DrawOrigin;  }
  void RenderRims(PSWriter& pw, const mat3f& pm, const vec3f& normal) const;
  void RenderQuads(PSWriter& pw, const mat3f& pm) const;
  void _process_points(TPtrList<vec3f>& points, ort_poly& otp)  {
    for( size_t i=0; i < otp.points.Count(); i++ )
      points.Add( otp.points[i] );
  }
public:
  OrtDraw() : app(TGXApp::GetInstance()), basis(app.GetRender().GetBasis()) {  
    ElpDiv = 36;
    BondDiv = 12;
    PieDiv = 4;
    QuadLineWidth = PieLineWidth = 0.5;
    ElpLineWidth = FontLineWidth = 1;
    BondRad = 1;
    ColorMode = 0;
    HBondScale = 0.5;
    Perspective = false;
  }
  // create ellipse and pie coordinates
  void Init(PSWriter& pw);

  void Render(const olxstr& fileName);

  DefPropP(uint16_t, ElpDiv)
  DefPropP(uint16_t, PieDiv) 
  DefPropP(uint16_t, BondDiv) 
  DefPropP(uint16_t, ColorMode)
  DefPropP(float, HBondScale)
  DefPropP(float, FontLineWidth)
  DefPropP(float, QuadLineWidth)
  DefPropP(float, PieLineWidth)
  DefPropP(float, ElpLineWidth)
  DefPropBIsSet(Perspective)

  friend struct ort_bond;
  friend struct ort_atom;
  friend struct ort_poly;
};


#endif

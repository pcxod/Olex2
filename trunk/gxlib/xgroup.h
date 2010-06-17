#ifndef __olx_glx_xgroup_H
#define __olx_glx_xgroup_H
#include "gxbase.h"
#include "glgroup.h"
#include "xatom.h"
#include "xbond.h"

BeginGxlNamespace()

class TXGroup : public TGlGroup, public AGlMouseHandler {
  vec3d RotationCenter;
protected:
  virtual void DoDraw(bool SelectPrimitives, bool SelectObjects) const {
    if( GetParentGroup() != NULL )  {  // is inside a group?
      TGlGroup::DoDraw(SelectPrimitives, SelectObjects);
      return;
    }
    olx_gl::translate(RotationCenter);  // + Basis.GetCenter() - in the next call to orient...
    olx_gl::orient(Basis);
    olx_gl::translate(-RotationCenter);
    for( size_t i=0; i < Count(); i++ )  {
      AGDrawObject& G = GetObject(i);
      if( !G.IsVisible() || G.IsDeleted() )  continue;
      if( G.IsGroup() )    {
        TGlGroup* group = dynamic_cast<TGlGroup*>(&G);
        if( group != NULL )  {
          group->Draw(SelectPrimitives, SelectObjects);
          continue;
        }
      }
      const size_t pc = G.GetPrimitives().PrimitiveCount();
      for( size_t j=0; j < pc; j++ )  {
        TGlPrimitive& GlP = G.GetPrimitives().GetPrimitive(j);
        TGlMaterial glm = GlP.GetProperties();
        glm.SetFlags(glm.GetFlags()|sglmColorMat|sglmShininessF|sglmSpecularF);
        glm.AmbientF *= 0.75;
        glm.AmbientF = glm.AmbientF.GetRGB() | 0x007070;
        glm.ShininessF = 32;
        glm.SpecularF = 0xff00;
        glm.Init(false);
        if( SelectObjects )     olx_gl::loadName((GLuint)G.GetTag());
        if( SelectPrimitives )  olx_gl::loadName((GLuint)GlP.GetTag());
        olx_gl::pushMatrix();
        if( G.Orient(GlP) )  {
          olx_gl::popMatrix();
          continue;
        }
        GlP.Draw();
        olx_gl::popMatrix();
      }
    }
  }
  TEBasis Basis;
  virtual bool DoTranslate(const vec3d& t) {  Basis.Translate(t);  return true;  }
  virtual bool DoRotate(const vec3d& vec, double angle) {  Basis.Rotate(vec, angle);  return true;  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  virtual const TGlRenderer& DoGetRenderer() const {  return GetParent();  }
  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnMouseDown(*this, Data);
  }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnMouseUp(*this, Data);
  }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnMouseMove(*this, Data);
  }
  bool OnDblClick(const IEObject *Sender, const TMouseData *Data)  {
    return GetHandler().OnDblClick(*this, Data);
  }
public:
  TXGroup(TGlRenderer& R, const olxstr& colName) : TGlGroup(R, colName)  {
    SetMoveable(true);
    SetRoteable(true);
  }
  void AddAtoms(const TPtrList<TXAtom>& atoms)  {
    for( size_t i=0; i < atoms.Count(); i++ )
      RotationCenter += atoms[i]->Atom().crd();
    RotationCenter /= atoms.Count();
    TGlGroup::AddObjects(atoms);
  }
  const vec3d& GetCenter() const {  return Basis.GetCenter();  }
  const mat3d& GetMatrix() const {  return Basis.GetMatrix();  }
  const vec3d& GetRotationCenter() const {  return RotationCenter;  }
  void ResetBasis() {  Basis.Reset();  }
};

EndGxlNamespace()
#endif

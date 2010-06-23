#ifndef __olx_glx_xline_H
#define __olx_glx_xline_H
#include "xbond.h"
#include "xatom.h"

BeginGxlNamespace()

class TXLine: public TXBond  {
  vec3d FBase, FEdge;
public:
  TXLine(TGlRenderer& Render, const olxstr& collectionName, const vec3d& base, const vec3d& edge);
  void Create(const olxstr& cName=EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TXLine();

  vec3d&  Base()  {  return FBase;  }
  vec3d&  Edge()  {  return FEdge;  }

  bool GetDimensions(vec3d &Max, vec3d &Min){  return false; };

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {  return true;  }
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {  return false;  }
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {  return false;  }

  bool Orient(TGlPrimitive& P);
  void Radius(float V);
  inline double Radius()  {  return Params()[4]; }
  void Length(float V);
  inline double Length()  {  return Params()[3]; }
};

EndGxlNamespace()
#endif
 

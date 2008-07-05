//---------------------------------------------------------------------------

#ifndef glclipplaneH
#define glclipplaneH

#include "glbase.h"
#include "evector.h"
#include "glrender.h"

BeginGlNamespace()

class TGlClipPlane  {
  TVectorD FEq;
  bool FEnabled;
  class TGlClipPlanes *FParent;
  int FId;  // GL_CLIPPLANE_i
public:
  TGlClipPlane( int Id, TGlClipPlanes *FParent, float A, float B, float C, float D );
  ~TGlClipPlane();
  inline bool Enabled() const     {  return FEnabled; }
  void Enabled(bool v);
  inline TVectorD& Equation()     {  return FEq; }
  inline TGlClipPlanes *Parent()  {  return FParent; }
  inline int Id() const           {  return FId; }
};

class TGlClipPlanes  {
  TPtrList<TGlClipPlane> FPlanes;
  class TGlRender *FParent;
public:
  TGlClipPlanes(TGlRender *R);
  ~TGlClipPlanes();
  inline TGlRender *Parent()         {  return FParent;  }
  inline TGlClipPlane *Plane(int i)  {  return FPlanes[i];  }
  inline int PlaneCount() const      {  return FPlanes.Count();  }
  void Enable(bool v);
};


EndGlNamespace()
#endif


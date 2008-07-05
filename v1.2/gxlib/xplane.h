#ifndef xplaneH
#define xplaneH
#include "gxbase.h"

#include "glrender.h"
#include "gdrawobject.h"

#include "splane.h"

BeginGxlNamespace()

class TXPlane: public AGDrawObject
{
private:
  TSPlane *FPlane;
  bool FRectangular;
public:
  TXPlane(const olxstr& collectionName, TSPlane *Plane, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TXPlane();

  inline operator TSPlane* () const {  return FPlane;  }
  inline TSPlane& Plane()     const {  return *FPlane; }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false; };

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; };
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; };
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; };

  inline bool Deleted()  const {  return AGDrawObject::Deleted(); }
  void Deleted(bool v){  AGDrawObject::Deleted(v);  FPlane->SetDeleted(v); }

  /*setting rectangulr after the plane was created does not make any change*/
  void Rectangular(bool v) {  FRectangular = v;  }
  bool Rectangular() {  return FRectangular; }
  // takes TPlaneSort as arguments
  static int PlaneSort(const void *I, const void *I1);
};

EndGxlNamespace()
#endif
 

#ifndef xgrowlineH
#define xgrowlinedH

#include "xbond.h"

#include "catom.h"
#include "satom.h"

BeginGxlNamespace()

class TXGrowLine : public TXBond  {
  TSAtom *FSAtom;
  TCAtom *FCAtom;
  smatd Transform;
  vec3d  FEdge, FBase;
public:
  TXGrowLine(const olxstr& collectionName, TSAtom *A,
               TCAtom* CA, const smatd& transform, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TXGrowLine();

  bool GetDimensions(vec3d &Max, vec3d &Min){  return false; };

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }

  bool Orient(TGlPrimitive *P);
  void Radius(float V);
  inline double Radius()     {  return Params()[4]; }
  void Length(float V);
  inline double Length()     {  return Params()[3]; }

  TSAtom *SAtom()      const {  return FSAtom;  }
  TCAtom *CAtom()      const {  return FCAtom;  }
  const smatd& GetTransform()  const {  return Transform;  }
};

EndGxlNamespace()
#endif
 

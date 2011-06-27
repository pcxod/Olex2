/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xgrowline_H
#define __olx_gxl_xgrowline_H
#include "xbond.h"
#include "catom.h"
#include "satom.h"
BeginGxlNamespace()

class TXGrowLine : public TXBond  {
  TXAtom& _XAtom;
  TCAtom& _CAtom;
  smatd Transform;
  vec3d FEdge, FBase;
protected:
  virtual bool IsMaskSaveable() const {  return true;  }
  virtual bool IsStyleSaveable() const {  return true;  }
  virtual bool IsRadiusSaveable() const {  return true;  }
public:
  TXGrowLine(TGlRenderer& Render, const olxstr& collectionName, TXAtom& A,
             TCAtom& CA, const smatd& transform);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TXGrowLine();

  bool GetDimensions(vec3d &Max, vec3d &Min){  return false; };

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {  return true;  }
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {  return false;  }
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {  return false;  }

  bool Orient(TGlPrimitive& P);
  void Radius(float V);
  inline double Radius()  {  return Params()[4]; }
  void Length(float V);
  inline double Length()  {  return Params()[3]; }

  TXAtom& XAtom() const {  return _XAtom;  }
  TCAtom& CAtom() const {  return _CAtom;  }
  const smatd& GetTransform() const {  return Transform;  }
};

EndGxlNamespace()
#endif

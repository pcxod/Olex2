#ifndef __olx_gl_cursor_H
#define __olx_gl_cursor_H
#include "glbase.h"
#include "gdrawobject.h"
#include "actions.h"
#include "macroerror.h"

BeginGlNamespace()

class TGlCursor: public AGDrawObject, AEventsDispatcher  {
  float X, Y;
  bool TextStyle;
  class TGlPrimitive *Primitive;
  olxch Symbol;
  size_t FontIndex;
protected:
  bool Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
public:
  TGlCursor(TGlRenderer& Render, const olxstr& collectionName, bool TextStyle = true);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TGlCursor() {}

  void SetPosition(float x, float y)  {  X = x;  Y = y;  }
  DefPropP(float, X)
  DefPropP(float, Y)
  DefPropP(size_t, FontIndex)
  class TGlFont& GetFont() const;
  
  bool IsText() const {  return TextStyle;  }
  inline olxch GetSymbol() const {  return Symbol;  }
  void SetSymbol(olxch v);

  virtual bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }

  void LibSymbol(const TStrObjList& Params, TMacroError& E);
  class TLibrary* ExportLibrary(const olxstr& name="cursor");
};

EndGlNamespace()
#endif

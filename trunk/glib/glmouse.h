//---------------------------------------------------------------------------

#ifndef glmouseH
#define glmouseH
#include "glbase.h"
#include "tptrlist.h"

BeginGlNamespace()

// mouse listner constants
const short  smbLeft   = 0x0001,
             smbMiddle = 0x0002,
             smbRight  = 0x0004,
             sssCtrl   = 0x0001,
             sssShift  = 0x0002,
             sssAlt    = 0x0004;
//  mouse move events
const short  smeMoveXY   = 1,
             smeMoveZ    = 2,
             smeRotateXY = 3,
             smeRotateZ  = 4,
             smeZoom     = 5,
             smeSelect   = 6;
// mouse actions             
const short  glmaTranslateXY  = 1,
             glmaRotateXY     = 2,
             glmaRotateZ      = 3,
             glmaTranslateZ   = 4,
             glmaZoom         = 5;

typedef void (*MMoveHandler)(class TGlMouse *, int dx, int dy);

class TMouseData: public IEObject  {
public:
  virtual ~TMouseData()  {  ;  }
  short Button, // mouse button
    Shift;      // shift state
  short DownX, DownY, // position of mouse when pressed down
        UpX, UpY,     // position of mous when released
        X, Y;         // current position
};

struct TGlMMoveEvent  {
  MMoveHandler Handler;
  short Button, Shift;
  bool ButtonDown;   // executed only if the mouse button is pressed
  TGlMMoveEvent()  {
    Button = Shift = 0;
    ButtonDown = true;
  }
};

class TGlMouse: public IEObject  {
  class TGlRender *FParent;
  class TDFrame *FDFrame;
protected:
  int FSX, FSY;
  bool FButtonDown, FDblClick;
  TPtrList<TGlMMoveEvent> Handlers;
  class AGDrawObject *Handler;
  class TMouseData *MData;
  short FAction;
public:
  TGlMouse(TGlRender *Parent, TDFrame *Frame);
  virtual ~TGlMouse();

  bool MouseUp(int x, int y, short Shift, short button);
  bool DblClick();
  void ResetMouseState();
  bool MouseDown(int x, int y, short Shift, short button);
  bool MouseMove(int x, int y, short Shift);
  inline TGlRender* Parent()  const {  return FParent;};
  inline int SX() const             {  return FSX; }
  inline int SY() const             {  return FSY; }
  void SetHandler( const short Button, const short Shift, MMoveHandler MH);
  // is set by handlers
  void Action( short A) {  FAction = A; }

  bool SelectionEnabled;
};

// default behaviour to mouse events
void meMoveXY(TGlMouse *, int dx, int dy);
void meMoveZ(TGlMouse *, int dx, int dy);
void meRotateXY(TGlMouse *, int dx, int dy);
void meRotateZ(TGlMouse *, int dx, int dy);
void meZoom(TGlMouse *, int dx, int dy);

EndGlNamespace()
#endif

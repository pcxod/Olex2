#ifndef __OLX_PACK_MODE_H
#define __OLX_PACK_MODE_H

class TPackMode : public AMode  {
public:
  TPackMode(size_t id) : AMode(id)  {}
  bool Initialise(TStrObjList &Cmds, const TParamList &Options) {
    olxstr AtomsToGrow( Cmds.Text(' ') );
    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    TGlXApp::GetGXApp()->SetPackMode( 0, AtomsToGrow );
    TGlXApp::GetGXApp()->SetXGrowPointsVisible(true);
    return true;
  }
  void Finalise() {  TGlXApp::GetGXApp()->SetXGrowPointsVisible(false);  }
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXGrowPoint) )  {
      TGlXApp::GetGXApp()->Grow((TXGrowPoint&)obj);
      return true;
    }
    return false;
  }
};

#endif

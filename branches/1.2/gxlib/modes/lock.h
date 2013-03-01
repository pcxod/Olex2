/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_LOCK_MODE_H
#define __OLX_LOCK_MODE_H

class TLockMode : public AModeWithLabels {
protected:
  class TLockModeUndo : public TUndoData {
    size_t index;
    bool v;
  public:
    TLockModeUndo(TXAtom& xa, bool v)
      : TUndoData(new TUndoActionImplMF<TLockModeUndo>(
          this, &TLockModeUndo::undo)),
        index(xa.GetOwnerId()),
        v(v)
    {}
    void undo(TUndoData* data) {
      TGXApp::GetInstance().MarkLabel(index, !v);
    }
  };
public:
  TLockMode(size_t id) : AModeWithLabels(id) {}
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    olex2.processMacro("cursor(hand)");
    olex2.processMacro("labels -l");
    TGXApp::BondIterator bi = gxapp.GetBonds();
    while (bi.HasNext())
      bi.Next().SetSelectable(false);
    TGXApp::AtomIterator ai = gxapp.GetAtoms();
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.CAtom().IsFixedType())
        gxapp.MarkLabel(a, true);
    }
    return true;
  }
  ~TLockMode() {}
  void Finalise()  {
    TGXApp::BondIterator bi = gxapp.GetBonds();
    while (bi.HasNext())
      bi.Next().SetSelectable(true);
    gxapp.XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject(AGDrawObject& obj) {
    if (EsdlInstanceOf(obj, TXAtom)) {
      TXAtom &XA = (TXAtom&)obj;
      TLockModeUndo* undo = new TLockModeUndo(XA, !XA.CAtom().IsFixedType());
      gxapp.GetUndo().Push(undo);
      XA.CAtom().SetFixedType(!XA.CAtom().IsFixedType());
      gxapp.MarkLabel(XA, XA.CAtom().IsFixedType());
      return true;
    }
    return false;
  }
};

#endif

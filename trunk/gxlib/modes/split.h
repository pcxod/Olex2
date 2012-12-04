/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_SPLIT_MODE_H
#define __OLX_SPLIT_MODE_H

enum {
  mode_split_Disassemble,
  mode_split_ObjectsCreate
};
class TSplitMode : public AEventsDispatcher, public AMode  {
  TTypeList<AnAssociation2<TXAtom*,TXAtom*> > SplitAtoms;
protected:
  void UpdateSelectionCrds() const {
    TGlGroup& sel = gxapp.GetSelection();
    if( sel.Count() > 1 )  {
      vec3d c, cr;
      TXAtomPList atoms;
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXAtom) )  {
          cr += ((TXAtom&)sel[i]).GetCenter();
          cr += ((TXAtom&)sel[i]).crd();
          atoms.Add((TXAtom&)sel[i]);
        }
      }
    }
  }
  void UpdateCrds() const {
    const TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
    UpdateSelectionCrds();
    TGXApp::AtomIterator ai = gxapp.GetAtoms();
    while( ai.HasNext() )  {
      TXAtom& xa = ai.Next();
      // summ the translations
      xa.crd() += xa.GetCenter();
      xa.NullCenter();
      vec3d c = xa.crd();
      au.CartesianToCell(c);
      xa.ccrd() = c;
      xa.CAtom().ccrd() = c;
    }
  }
  olxstr ReCon; // restraint or constraint to use for split atoms
public:
  TSplitMode(size_t id) : AMode(id)  {
    gxapp.OnObjectsCreate.Add(this, mode_split_ObjectsCreate, msiExit);
    gxapp.XFile().GetLattice().OnDisassemble.Add(
      this, mode_split_Disassemble, msiEnter);
  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    if( !gxapp.CheckFileType<TIns>() )  return false;
    ReCon = Options.FindValue("r", EmptyString()).ToLowerCase();
    olex2.processMacro("cursor(hand)");
    TGXApp::AtomIterator ai = gxapp.GetAtoms();
    while( ai.HasNext() )
      ai.Next().SetMoveable(true);
    return true;
  }
  ~TSplitMode()  {
    gxapp.OnObjectsCreate.Remove(this);
    gxapp.XFile().GetLattice().OnDisassemble.Remove(this);
  }
  void Finalise() {
    RefinementModel& rm = gxapp.XFile().GetRM();
    UpdateCrds();
    // if this is not done here it interferes and may cause a crash
    gxapp.XFile().GetLattice().OnDisassemble.Remove(this);
    gxapp.OnObjectsCreate.Remove(this);
    TGXApp::AtomIterator ai = gxapp.GetAtoms();
    while( ai.HasNext() )
      ai.Next().SetMoveable(false);
    if( SplitAtoms.IsEmpty() )  {
      gxapp.XFile().GetLattice().UpdateConnectivity();
      return;
    }
    TCAtomPList to_isot;
    XVar& xv = rm.Vars.NewVar(0.75);
    for( size_t i=0; i < SplitAtoms.Count(); i++ )  {
      TXAtom& a = *SplitAtoms[i].GetA();
      TXAtom& b = *SplitAtoms[i].GetB();
      to_isot.Add(a.CAtom());
      const double sp = 1./a.CAtom().GetDegeneracy();
      rm.Vars.AddVarRef(xv, a.CAtom(), catom_var_name_Sof, relation_AsVar, sp);
      rm.Vars.AddVarRef(xv, b.CAtom(), catom_var_name_Sof, relation_AsOneMinusVar, sp);
      int part = a.CAtom().GetPart();
      if( part == 0 )  part ++;
      a.CAtom().SetPart(part);
      a.CAtom().SetOccu(0.75*sp);
      b.CAtom().SetPart(part+1);
      b.CAtom().SetOccu(0.25*sp);
      TSimpleRestraint* sr = NULL;
      if( ReCon.IsEmpty() );
      else if( ReCon == "eadp" )
        sr = &rm.rEADP.AddNew();
      else if( ReCon == "isor" )
        sr = &rm.rISOR.AddNew();
      else if( ReCon == "simu" )
        sr = &rm.rSIMU.AddNew();
      if( sr != NULL )
        sr->AddAtomPair(a.CAtom(), NULL, b.CAtom(), NULL);
    }
    gxapp.XFile().GetLattice().SetAnis(to_isot, false);
    gxapp.XFile().GetLattice().Uniq();
  }
  virtual bool Dispatch(int msg, short id, const IEObject* Sender,
    const IEObject* Data=NULL)
  {
    if( msg == mode_split_ObjectsCreate )  {
      TGXApp::AtomIterator ai = gxapp.GetAtoms();
      while( ai.HasNext() )
        ai.Next().SetMoveable(true);
    }
    else if( msg == mode_split_Disassemble )  {
      UpdateCrds();
    }
    return true;
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      bool split = true;
      for( size_t i=0; i < SplitAtoms.Count(); i++ )
        if( SplitAtoms[i].A() == XA || SplitAtoms[i].B() == XA )  {
          split = false;
          break;
        }
      if( split )  {
        TXAtom &xa = gxapp.AddAtom(XA);
        const TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
        xa.SetMoveable(true);
        xa.SetRoteable(true);
        SplitAtoms.AddNew(XA, &xa);
        int part = XA->CAtom().GetPart();
        if( part == 0 )  part ++;
        XA->CAtom().SetPart(part);
        xa.CAtom().SetPart(part+1);
        xa.crd() += 0.5;
        vec3d c = au.Fractionalise(xa.crd());
        xa.CAtom().ccrd() = c;
        xa.ccrd() = c;
        olxstr new_l = XA->GetLabel();
        olxch lc = olxstr::o_tolower(new_l.GetLast());
        if( olxstr::o_isalpha(lc) && new_l.Length() > 1 )
          new_l[new_l.Length()-1] = ++lc;
        else
          new_l << 'a';
        xa.CAtom().SetLabel(au.CheckLabel(&xa.CAtom(), new_l), true);
        if( xa.GetType() == iQPeakZ )
          xa.CAtom().SetQPeak(1.0);
        gxapp.XFile().GetLattice().UpdateConnectivity();
        gxapp.UpdateDuplicateLabels();
      }
      else  {  // do selection then
        UpdateSelectionCrds();
        gxapp.GetRender().Select(*XA);
      }
      return true;
    }
    return false;
  }
};

#endif

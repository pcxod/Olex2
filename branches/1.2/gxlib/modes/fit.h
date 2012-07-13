/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_FIT_MODE_H
#define __OLX_FIT_MODE_H
#include "xgroup.h"
#include "match.h"

enum  {
  mode_fit_create,
  mode_fit_disassemble
};
class TFitMode : public AEventsDispatcher, public AMode  {
  TXGroup* group;
  TXAtomPList Atoms, AtomsToMatch;
  vec3d_alist original_crds;
  bool Initialised, DoSplit;
  int afix, part;
  size_t split_offset;
  class OnUniqHandler : public AActionHandler {
    TFitMode& fit_mode;
  public:
    OnUniqHandler(TFitMode& fm) : fit_mode(fm)  {}
    bool Enter(const IEObject* Sender, const IEObject* Data=NULL)  {
      fit_mode.Dispatch(mode_fit_disassemble, msiEnter, NULL, NULL);
      return true;
    }
  };
  class TFitModeUndo : public TUndoData {
    TArrayList<AnAssociation2<TCAtom*, vec3d> > data;
    typedef TUndoActionImplMF<TFitModeUndo> impl_t;
  public:
    TFitModeUndo() : TUndoData(new impl_t(this, &TFitModeUndo::undo)) {}
    TFitModeUndo(const TXAtomPList &atoms)
      : TUndoData(new impl_t(this, &TFitModeUndo::undo)),
        data(atoms.Count())
    {
      for (size_t i=0; i < atoms.Count(); i++) {
        data[i].A() = &atoms[i]->CAtom();
        data[i].B() = atoms[i]->ccrd();
      }
    }
    void undo(TUndoData *) {
      if (data.IsEmpty()) return;
      TAsymmUnit &au = *data[0].GetA()->GetParent();
      au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
      for (size_t i=0; i < data.Count(); i++) {
        data[i].A()->ccrd() = data[i].B();
        data[i].A()->SetTag(1);
      }
      TGXApp::AtomIterator ai = TGXApp::GetInstance().GetAtoms();
      while (ai.HasNext()) {
        TXAtom &xa = ai.Next();
        if (xa.CAtom().GetTag() != 1) continue;
        xa.ccrd() = xa.CAtom().ccrd();
        xa.crd() = au.Orthogonalise(xa.ccrd());
      }
      TGXApp::GetInstance().XFile().GetLattice().Init();
    }
  };
  double AngleInc;
  OnUniqHandler* uniq_handler;
  TFitModeUndo *undo;
public:
  TFitMode(size_t id)
    : AMode(id),
      Initialised(false),
      AngleInc(0),
      DoSplit(false),
      undo(NULL)
  {
    uniq_handler = new OnUniqHandler(*this);
    gxapp.OnObjectsCreate.Add(this, mode_fit_create, msiExit);
    gxapp.XFile().GetLattice().OnDisassemble.Add(this, mode_fit_disassemble,
      msiEnter);
    gxapp.XFile().GetLattice().OnStructureUniq.AddFirst(uniq_handler);
    gxapp.XFile().GetLattice().OnStructureGrow.AddFirst(uniq_handler);
    gxapp.EnableSelection(false);
  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    DoSplit = Options.Contains('s');
    if (DoSplit) {
      olxstr s = Options.FindValue('s');
      if (!s.IsEmpty()) split_offset = s.ToUInt();
      else
        split_offset = 0;
    }
    afix = Options.FindValue('a', "-1").ToInt();
    if (DoSplit && afix != -1) {
      TBasicApp::NewLogEntry(logError) <<
        "Split and Afix are not compatible, atoms will be only split";
    }
    AtomsToMatch.Clear();
    SetUserCursor('0', "<F>");
    TXAtomPList xatoms = gxapp.GetSelection().Extract<TXAtom>();
    undo = new TFitModeUndo(xatoms);
    original_crds.SetCount(xatoms.Count());
    for( size_t i=0; i < xatoms.Count(); i++ )
      original_crds[i] = xatoms[i]->crd();
    group = &gxapp.GetRender().ReplaceSelection<TXGroup>();
    AngleInc = Options.FindValue("r", "0").ToDouble();
    group->SetAngleInc(AngleInc*M_PI/180);
    AddAtoms(xatoms);
    group->SetOrgiginalCrds(original_crds);
    return (Initialised = true);
  }
  ~TFitMode()  {
    gxapp.OnObjectsCreate.Remove(this);
    gxapp.XFile().GetLattice().OnDisassemble.Remove(this);
    gxapp.XFile().GetLattice().OnStructureUniq.Remove(uniq_handler);
    gxapp.XFile().GetLattice().OnStructureGrow.Remove(uniq_handler);
    delete uniq_handler;
    if (undo != NULL) delete undo;
    gxapp.EnableSelection(true);
  }
  void Finalise() {
    gxapp.GetRender().ReplaceSelection<TGlGroup>();
    Initialised = false;
    RefinementModel& rm = gxapp.XFile().GetRM();
    TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
    XVar& xv = rm.Vars.NewVar(0.75);
    if( DoSplit )  {
      TCAtomPList to_iso;
      for( size_t i=split_offset; i < Atoms.Count(); i++ )  {
        if( Atoms[i]->crd().QDistanceTo(original_crds[i]) < 1e-3 )
          continue;
        TXAtom& nxa = gxapp.AddAtom(Atoms[i]);
        TCAtom& na = nxa.CAtom();
        // set parts
        int part = Atoms[i]->CAtom().GetPart();
        if( part == 0 )  part ++;
        Atoms[i]->CAtom().SetPart(part);
        na.SetPart(part+1);
        // link occupancies
        const double sp = 1./Atoms[i]->CAtom().GetDegeneracy();
        rm.Vars.AddVarRef(xv, Atoms[i]->CAtom(), catom_var_name_Sof, relation_AsVar, sp);
        rm.Vars.AddVarRef(xv, na, catom_var_name_Sof, relation_AsOneMinusVar, sp);
        Atoms[i]->CAtom().SetOccu(0.75*sp);
        na.SetOccu(0.25*sp);
        // set label
        olxstr new_l = Atoms[i]->GetLabel();
        olxch lc = olxstr::o_tolower(new_l.GetLast());
        if( olxstr::o_isalpha(lc) )
          new_l[new_l.Length()-1] = ++lc;
        else
          new_l << 'a';
        na.SetLabel(au.CheckLabel(&na, new_l), false);
        if( na.GetType() == iQPeakZ )
          na.SetQPeak(1.0);
        // set coordinates
        na.ccrd() = au.Fractionalise(Atoms[i]->crd());
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(original_crds[i]);
        to_iso.Add(Atoms[i]->CAtom());
      }
      gxapp.XFile().GetLattice().SetAnis(to_iso, false);
      gxapp.XFile().GetLattice().Uniq();
    }
    else  {
      TUnitCell& uc = gxapp.XFile().GetUnitCell();
      au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
      Atoms.ForEach(ACollectionItem::TagSetter(
        FunctionAccessor::MakeConst(&TSAtom::CAtom), 1));
      for (size_t i=0; i < Atoms.Count(); i++) {
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(Atoms[i]->crd());
        TTypeList<AnAssociation2<TCAtom*, vec3d> > res;
        uc.FindInRangeAC(Atoms[i]->CAtom().ccrd(), 0.5, res);
        for (size_t j=0; j < res.Count(); j++) {
          if (res[j].GetA()->GetTag() == 0 &&
              (res[j].GetA()->GetPart() == 0 ||
               res[j].GetA()->GetPart() == Atoms[i]->CAtom().GetPart()))
          {
            res[j].A()->SetDeleted(true);
          }
        }
      }
      if (afix != -1 && !Atoms.IsEmpty()) {
        bool has_pivot = TAfixGroup::HasExcplicitPivot(afix);
        TAfixGroup &ag = gxapp.XFile().GetRM().AfixGroups.New(
          has_pivot ? &Atoms[0]->CAtom() : NULL, afix);
        size_t start = has_pivot? 1 : 0;
        for( size_t i=start; i < Atoms.Count(); i++ )
          ag.AddDependent(Atoms[i]->CAtom());
      }
      gxapp.XFile().EndUpdate();
      
    }
    if (undo != NULL) {
      gxapp.GetUndo().Push(undo);
      undo = NULL;
    }
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      if( AtomsToMatch.IsEmpty() && Atoms.IndexOf((TXAtom&)obj) == InvalidIndex )
        return true;
      AtomsToMatch.Add((TXAtom&)obj);
      SetUserCursor(AtomsToMatch.Count(), "<F>");
      if( (AtomsToMatch.Count()%2) == 0 )  {
        TMatchMode::FitAtoms(AtomsToMatch, false);
        SetUserCursor(AtomsToMatch.Count(), "<F>");
        group->UpdateRotationCenter();
      }
    }
    return true;
  }
  virtual bool Dispatch(int msg, short id, const IEObject* Sender,
    const IEObject* Data=NULL)
  {
    if( !Initialised )  return false;
    TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
    if( msg == mode_fit_disassemble )  {
      if( !EsdlInstanceOf(gxapp.GetRender().GetSelection(), TXGroup) )
        return true;
      for( size_t i=0; i < Atoms.Count(); i++ )
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(Atoms[i]->crd());
      Atoms.Clear();
      AtomsToMatch.Clear();
      SetUserCursor('0', "<F>");
    }
    else if( msg == mode_fit_create )  {
      if( !EsdlInstanceOf(gxapp.GetRender().GetSelection(), TXGroup) )  {
        group = &gxapp.GetRender().ReplaceSelection<TXGroup>();
        group->SetAngleInc(AngleInc*M_PI/180);
      }
      Atoms = group->Extract<TXAtom>();
      group->Update();
      group->SetOrgiginalCrds(original_crds);
      group->SetSelected(true);
    }
    return true;
  }
  virtual bool OnKey(int keyId, short shiftState)  {
    if( shiftState == 0 && keyId == WXK_ESCAPE )  {
      if( AtomsToMatch.IsEmpty() )  return false;
      AtomsToMatch.Delete(AtomsToMatch.Count()-1);
      SetUserCursor(AtomsToMatch.Count(), "<F>");
      return true;
    }
    return false;
  }
  virtual bool AddAtoms(const TXAtomPList& atoms)  {
    Atoms.AddList(atoms);
    group->AddAtoms(atoms);
    group->SetSelected(true);
    return true;
  }
};

#endif

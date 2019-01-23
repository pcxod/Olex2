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
class TFitMode : public AEventsDispatcher, public AMode {
  TXGroup* group;
  TXAtomPList Atoms, AtomsToMatch;
  vec3d_alist original_crds;
  bool Initialised, DoSplit, Restrain, RestrainU;
  int afix, part;
  size_t split_offset;
  class OnUniqHandler : public AActionHandler {
    TFitMode& fit_mode;
  public:
    OnUniqHandler(TFitMode& fm) : fit_mode(fm) {}
    bool Enter(const IOlxObject* Sender, const IOlxObject* Data, TActionQueue *) {
      fit_mode.Dispatch(mode_fit_disassemble, msiEnter, NULL, NULL, NULL);
      return true;
    }
  };
  class TFitModeUndo : public TUndoData {
    TArrayList<olx_pair_t<TCAtom*, vec3d> > data;
    typedef TUndoActionImplMF<TFitModeUndo> impl_t;
  public:
    TFitModeUndo() : TUndoData(new impl_t(this, &TFitModeUndo::undo)) {}
    TFitModeUndo(const TXAtomPList &atoms)
      : TUndoData(new impl_t(this, &TFitModeUndo::undo)),
      data(atoms.Count())
    {
      for (size_t i = 0; i < atoms.Count(); i++) {
        data[i].a = &atoms[i]->CAtom();
        data[i].b = atoms[i]->ccrd();
      }
    }
    void undo(TUndoData *) {
      if (data.IsEmpty()) {
        return;
      }
      TAsymmUnit &au = *data[0].GetA()->GetParent();
      au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
      for (size_t i = 0; i < data.Count(); i++) {
        data[i].a->ccrd() = data[i].GetB();
        data[i].a->SetTag(1);
      }
      TGXApp::AtomIterator ai = TGXApp::GetInstance().GetAtoms();
      while (ai.HasNext()) {
        TXAtom &xa = ai.Next();
        if (xa.CAtom().GetTag() != 1) {
          continue;
        }
        xa.ccrd() = xa.CAtom().ccrd();
        xa.crd() = au.Orthogonalise(xa.ccrd());
      }
      TGXApp::GetInstance().XFile().GetLattice().Init();
    }
  };
  struct idx_pair_t : public olx_pair_t<size_t, size_t> {
    idx_pair_t(size_t a, size_t b)
      : olx_pair_t<size_t, size_t>(a, b)
    {}
    int Compare(const idx_pair_t &p) const {
      int d = olx_cmp(a, p.a);
      if (d == 0) {
        d = olx_cmp(b, p.b);
      }
      return d;
    }
  };

  double AngleInc;
  OnUniqHandler* uniq_handler;
  TFitModeUndo *undo;
public:
  TFitMode(size_t id)
    : AMode(id),
    Initialised(false),
    DoSplit(false),
    Restrain(false), RestrainU(false),
    AngleInc(0),
    undo(0)
  {
    uniq_handler = new OnUniqHandler(*this);
    gxapp.OnObjectsCreate.Add(this, mode_fit_create, msiExit);
    gxapp.XFile().GetLattice().OnDisassemble.Add(this, mode_fit_disassemble,
      msiEnter);
    gxapp.XFile().GetLattice().OnStructureUniq.InsertFirst(uniq_handler);
    gxapp.XFile().GetLattice().OnStructureGrow.InsertFirst(uniq_handler);
    gxapp.EnableSelection(false);
  }

  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    Restrain = Cmds.Containsi("same");
    RestrainU = Cmds.Containsi("rigu");
    DoSplit = Options.Contains('s');
    if (DoSplit) {
      split_offset = 0;
      olxstr s = Options.FindValue('s');
      if (!s.IsEmpty()) {
        if (s.IsBool()) {
          DoSplit = s.ToBool();
        }
        else {
          split_offset = s.ToUInt();
        }
      }
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
    for (size_t i = 0; i < xatoms.Count(); i++) {
      original_crds[i] = xatoms[i]->crd();
    }
    group = &gxapp.GetRenderer().ReplaceSelection<TXGroup>();
    AngleInc = Options.FindValue("r", "0").ToDouble();
    group->SetAngleInc(AngleInc*M_PI / 180);
    group->SetMirrororingEnabled(DoSplit);
    AddAtoms(xatoms);
    return (Initialised = true);
  }

  ~TFitMode() {
    gxapp.OnObjectsCreate.Remove(this);
    gxapp.XFile().GetLattice().OnDisassemble.Remove(this);
    gxapp.XFile().GetLattice().OnStructureUniq.Remove(uniq_handler);
    gxapp.XFile().GetLattice().OnStructureGrow.Remove(uniq_handler);
    delete uniq_handler;
    olx_del_obj(undo);
    gxapp.EnableSelection(true);
  }

  void Finalise_() {
    vec3d_alist crds = group->GetSrcCoordinates();
    gxapp.GetRenderer().ReplaceSelection<TGlGroup>();
    Initialised = false;
    RefinementModel& rm = gxapp.XFile().GetRM();
    TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
    TAsymmUnit::TLabelChecker lck(au);
    XVar& xv = rm.Vars.NewVar(0.75);
    if (DoSplit) {
      olxdict<size_t, size_t, TPrimitiveComparator> atom_map;
      atom_map.SetCapacity(Atoms.Count());
      olxset<size_t, TPrimitiveComparator> atom_set;
      atom_set.SetCapacity(Atoms.Count());
      for (size_t i = split_offset; i < Atoms.Count(); i++) {
        if (Atoms[i]->crd().QDistanceTo(original_crds[i]) < 0.01) {
          continue;
        }
        TXAtom& nxa = gxapp.AddAtom(Atoms[i]);
        TCAtom& na = nxa.CAtom();
        // set parts
        int part = Atoms[i]->CAtom().GetPart();
        if (part == 0) {
          part++;
        }
        Atoms[i]->CAtom().SetPart(part);
        // take care of negative parts too
        na.SetPart(olx_sign(part)*(olx_abs(part) + 1));
        na.SetUiso(Atoms[i]->CAtom().GetUiso());
        // link occupancies
        const double sp = 1. / Atoms[i]->CAtom().GetDegeneracy();
        rm.Vars.AddVarRef(xv, Atoms[i]->CAtom(), catom_var_name_Sof, relation_AsVar, sp);
        rm.Vars.AddVarRef(xv, na, catom_var_name_Sof, relation_AsOneMinusVar, sp);
        Atoms[i]->CAtom().SetOccu(0.75*sp);
        na.SetOccu(0.25*sp);
        // set label
        olxstr new_l = Atoms[i]->GetLabel();
        olxch lc = '1';
        if (new_l.Length() > Atoms[i]->GetType().symbol.Length()) {
          lc = olxstr::o_tolower(new_l.GetLast());
        }
        if (olxstr::o_isalpha(lc)) {
          new_l[new_l.Length() - 1] = ++lc;
        }
        else {
          new_l << 'a';
        }
        na.SetLabel(lck.CheckLabel(na, new_l, 0, true), false);
        if (na.GetType() == iQPeakZ) {
          na.SetQPeak(1.0);
        }
        // set coordinates
        na.ccrd() = au.Fractionalise(Atoms[i]->crd());
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(crds[i]);
        atom_map.Add(Atoms[i]->CAtom().GetId(), na.GetId());
        atom_set.Add(Atoms[i]->CAtom().GetId());
      }
      if (Restrain) {
        olxset<idx_pair_t, TComparableComparator> bonds12, bonds13;
        for (size_t i = 0; i < atom_set.Count(); i++) {
          TCAtom &a = au.GetAtom(atom_set[i]);
          for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
            TCAtom::Site &s1 = a.GetAttachedSite(j);
            if (!s1.matrix.IsFirst() || s1.atom->IsDeleted()) {
              continue;
            }
            size_t a_idx, b_idx;
            if (a.GetId() > s1.atom->GetId()) {
              a_idx = s1.atom->GetId();
              b_idx = a.GetId();
            }
            else {
              a_idx = a.GetId();
              b_idx = s1.atom->GetId();
            }
            bonds12.Add(idx_pair_t(a_idx, b_idx));
            bool set_atom = atom_set.Contains(s1.atom->GetId());
            for (size_t k = j+1; k < a.AttachedSiteCount(); k++) {
              TCAtom::Site &s2 = a.GetAttachedSite(k);
              if (!s2.matrix.IsFirst() || s2.atom->IsDeleted()) {
                continue;
              }
              // at least one of the atoms should be in the set
              if (!set_atom && !atom_set.Contains(s2.atom->GetId())) {
                continue;
              }
              if (s1.atom->GetId() > s2.atom->GetId()) {
                a_idx = s2.atom->GetId();
                b_idx = s1.atom->GetId();
              }
              else {
                a_idx = s1.atom->GetId();
                b_idx = s2.atom->GetId();
              }
              bonds13.Add(idx_pair_t(a_idx, b_idx));
            }
          }
        }
        for (size_t i = 0; i < bonds12.Count(); i++) {
          TSimpleRestraint &sr = rm.rSADI.AddNew();
          sr.AddAtomPair(au.GetAtom(bonds12[i].a), 0, au.GetAtom(bonds12[i].b), 0);
          sr.AddAtomPair(au.GetAtom(atom_map.Find(bonds12[i].a, bonds12[i].a)), 0,
            au.GetAtom(atom_map.Find(bonds12[i].b, bonds12[i].b)), 0);
        }
        for (size_t i = 0; i < bonds13.Count(); i++) {
          TSimpleRestraint &sr = rm.rSADI.AddNew();
          sr.SetEsd(sr.GetEsd() * 2);
          sr.AddAtomPair(au.GetAtom(bonds13[i].a), 0, au.GetAtom(bonds13[i].b), 0);
          sr.AddAtomPair(au.GetAtom(atom_map.Find(bonds13[i].a, bonds13[i].a)), 0,
            au.GetAtom(atom_map.Find(bonds13[i].b, bonds13[i].b)), 0);
        }
      }
      if (RestrainU) {
        TSimpleRestraint &r1 = rm.rRIGU.AddNew();
        TSimpleRestraint &r2 = rm.rRIGU.AddNew();
        for (size_t i = 0; i < atom_set.Count(); i++) {
          TCAtom &a = au.GetAtom(atom_set[i]);
          r1.AddAtom(a, 0);
          r2.AddAtom(au.GetAtom(atom_map[a.GetId()]), 0);
        }
      }
      gxapp.XFile().GetLattice().SetAnis(
          TCAtomPList(Atoms, FunctionAccessor::MakeConst(&TSAtom::CAtom)), false);
      gxapp.XFile().GetLattice().Uniq();
      gxapp.UpdateDuplicateLabels();
    }
    else {
      TUnitCell& uc = gxapp.XFile().GetUnitCell();
      au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
      Atoms.ForEach(ACollectionItem::TagSetter(
        FunctionAccessor::MakeConst(&TSAtom::CAtom), 1));
      for (size_t i = 0; i < Atoms.Count(); i++) {
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(Atoms[i]->crd());
        TTypeList<olx_pair_t<TCAtom*, vec3d> > res;
        uc.FindInRangeAC(Atoms[i]->CAtom().ccrd(), 0.5, res);
        for (size_t j = 0; j < res.Count(); j++) {
          if (res[j].GetA()->GetTag() == 0 &&
            (res[j].GetA()->GetPart() == 0 ||
              res[j].GetA()->GetPart() == Atoms[i]->CAtom().GetPart()))
          {
            res[j].a->SetDeleted(true);
          }
        }
      }
      if (afix != -1 && !Atoms.IsEmpty()) {
        bool has_pivot = TAfixGroup::HasExcplicitPivot(afix);
        TAfixGroup &ag = gxapp.XFile().GetRM().AfixGroups.New(
          has_pivot ? &Atoms[0]->CAtom() : NULL, afix);
        size_t start = has_pivot ? 1 : 0;
        for (size_t i = start; i < Atoms.Count(); i++) {
          ag.AddDependent(Atoms[i]->CAtom());
        }
      }
      gxapp.XFile().EndUpdate();
    }
    if (undo != 0) {
      gxapp.GetUndo().Push(undo);
      undo = 0;
    }
    if (TXApp::DoUseSafeAfix()) {
      gxapp.GetUndo().Push(
        gxapp.XFile().GetLattice().ValidateHGroups(true, true));
    }
  }

  virtual bool OnObject_(AGDrawObject &obj) {
    if (DoSplit) {
      return true;
    }
    if (obj.Is<TXAtom>()) {
      if (AtomsToMatch.IsEmpty() && Atoms.IndexOf((TXAtom&)obj) == InvalidIndex) {
        return true;
      }
      AtomsToMatch.Add((TXAtom&)obj);
      SetUserCursor(AtomsToMatch.Count(), "<F>");
      if ((AtomsToMatch.Count() % 2) == 0) {
        TMatchMode::FitAtoms(AtomsToMatch, false);
        SetUserCursor(AtomsToMatch.Count(), "<F>");
        group->UpdateRotationCenter();
      }
    }
    return true;
  }

  virtual bool Dispatch(int msg, short id, const IOlxObject* Sender,
    const IOlxObject* Data, TActionQueue *)
  {
    if (!Initialised) {
      return false;
    }
    TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
    if (msg == mode_fit_disassemble) {
      if (!gxapp.GetRenderer().GetSelection().Is<TXGroup>()) {
        return true;
      }
      for (size_t i = 0; i < Atoms.Count(); i++) {
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(Atoms[i]->crd());
      }
      Atoms.Clear();
      AtomsToMatch.Clear();
      SetUserCursor('0', "<F>");
    }
    else if (msg == mode_fit_create) {
      if (!gxapp.GetRenderer().GetSelection().Is<TXGroup>()) {
        group = &gxapp.GetRenderer().ReplaceSelection<TXGroup>();
        group->SetAngleInc(AngleInc*M_PI / 180);
      }
      Atoms = group->Extract<TXAtom>();
      group->Update();
      group->SetSelected(true);
    }
    return true;
  }

  virtual bool OnKey_(int keyId, short shiftState) {
    if (shiftState == 0 && keyId == OLX_KEY_ESCAPE) {
      if (AtomsToMatch.IsEmpty()) {
        return false;
      }
      AtomsToMatch.Delete(AtomsToMatch.Count() - 1);
      SetUserCursor(AtomsToMatch.Count(), "<F>");
      return true;
    }
    return false;
  }

  virtual bool AddAtoms(const TXAtomPList& atoms) {
    Atoms.AddAll(atoms);
    group->AddAtoms(atoms);
    group->SetSelected(true);
    return true;
  }
};

#endif

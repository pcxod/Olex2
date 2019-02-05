/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "conninfo.h"
#include "atomref.h"
#include "xapp.h"
#include "unitcell.h"

void ConnInfo::ProcessFree(const TStrList& ins)  {
  TAtomReference ar(ins.Text(' '));
  TCAtomGroup ag;
  size_t aag;
  try  {  ar.Expand(rm, ag, EmptyString(), aag);  }
  catch(const TExceptionBase&)  {
    TBasicApp::NewLogEntry(logError) << "Could not locate atoms for FREE " <<
      ar.GetExpression();
    return;
  }
  if( ag.Count() == 0 || (ag.Count()%2) != 0 )  {
    TBasicApp::NewLogEntry(logError) <<
      "Even number of atoms is expected for FREE " <<
      ar.GetExpression() << ' ' << ag.Count() << " is provided";
    return;
  }
  for( size_t i=0; i < ag.Count(); i += 2 )  {
    if( ag[i].GetMatrix() != NULL && ag[i+1].GetMatrix() != NULL )  {
      TBasicApp::NewLogEntry(logError) <<
        "Only one eqivalent is expected in FREE, skipping " <<
        ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm);
      continue;
    }
    // validate
    if( ag[i].GetAtom()->GetId() == ag[i+1].GetAtom()->GetId() )  {
      if( (ag[i].GetMatrix() != NULL && ag[i].GetMatrix()->r.IsI() &&
           ag[i].GetMatrix()->t.IsNull()) ||
        (ag[i+1].GetMatrix() != NULL && ag[i+1].GetMatrix()->r.IsI() &&
         ag[i+1].GetMatrix()->t.IsNull()) ||
        (ag[i].GetMatrix() == NULL && ag[i+1].GetMatrix() == NULL) )
      {
        TBasicApp::NewLogEntry(logError) << "Dummy  FREE, skipping " <<
          ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm);
        continue;
      }
    }
    RemBond(*ag[i].GetAtom(), *ag[i+1].GetAtom(), ag[i].GetMatrix(),
      ag[i+1].GetMatrix(), true);
  }
}
//........................................................................
void ConnInfo::ProcessBind(const TStrList& ins)  {
  if (olx_list_and(ins, &olxstr::IsNumber)) {
    SortedIntList &group = PartGroups.AddNew();
    for (size_t i=0; i < ins.Count(); i++) {
      group.AddUnique(ins[i].ToInt());
    }
    for (size_t i=0; i < group.Count(); i++) {
      SortedIntList &l = PartGroups_.Add(group[i]);
      for (size_t j=0; j < group.Count(); j++) {
        if (i == j) continue;
        l.AddUnique(group[j]);
      }
    }
  }
  else {
    TAtomReference ar(ins.Text(' '));
    TCAtomGroup ag;
    size_t aag;
    try  {  ar.Expand(rm, ag, EmptyString(), aag);  }
    catch(const TExceptionBase&)  {
      TBasicApp::NewLogEntry(logError) << "Could not locate atoms for BIND " <<
        ar.GetExpression();
      return;
    }
    if( ag.Count() == 0 || (ag.Count()%2) != 0 )  {
      TBasicApp::NewLogEntry(logError) <<
        "Even number of atoms is expected for FREE " <<
        ar.GetExpression() << ' ' << ag.Count() << " is provided";
      return;
    }
    for( size_t i=0; i < ag.Count(); i += 2 )  {
      if( ag[i].GetMatrix() != NULL && ag[i+1].GetMatrix() != NULL )  {
        TBasicApp::NewLogEntry(logError) <<
          "Only one eqivalent is expected in BIND, skipping " <<
          ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm);
        continue;
      }
      // validate
      if( ag[i].GetAtom()->GetId() == ag[i+1].GetAtom()->GetId() )  {
        if( (ag[i].GetMatrix() != NULL && ag[i].GetMatrix()->r.IsI() &&
             ag[i].GetMatrix()->t.IsNull()) ||
          (ag[i+1].GetMatrix() != NULL && ag[i+1].GetMatrix()->r.IsI() &&
           ag[i+1].GetMatrix()->t.IsNull()) ||
          (ag[i].GetMatrix() == NULL && ag[i+1].GetMatrix() == NULL) )
        {
          TBasicApp::NewLogEntry(logError) << "Dummy BIND, skipping " <<
            ag[i].GetFullLabel(rm) << ' ' << ag[i+1].GetFullLabel(rm);
          continue;
        }
      }
      AddBond(*ag[i].GetAtom(), *ag[i+1].GetAtom(), ag[i].GetMatrix(),
        ag[i+1].GetMatrix(), true);
    }
  }
}
//........................................................................
void ConnInfo::Disconnect(TCAtom& ca)  {
  const size_t i = AtomInfo.IndexOf(&ca);
  if (i == InvalidIndex) {
    AtomConnInfo& ai = AtomInfo.Add(&ca, AtomConnInfo(ca));
    ai.maxBonds = 0;
    ai.r = -1;
  }
  else {
    AtomInfo.GetValue(i).maxBonds = 0;
  }
  ca.SetConnInfo(GetConnInfo(ca));
}
//........................................................................
void ConnInfo::ProcessConn(TStrList& ins) {
  short maxB = def_max_bonds;
  double r = -1;
  TSizeList num_indexes;
  for (size_t i = 0; i < ins.Count(); i++) {
    if (ins[i].IsNumber()) {
      num_indexes.Add(i);
    }
  }
  if (num_indexes.Count() == 2) {
    maxB = ins[num_indexes[0]].ToInt();
    r = ins[num_indexes[1]].ToDouble();
  }
  else if (num_indexes.Count() == 1) {
    if (ins[num_indexes[0]].Contains('.')) {
      r = ins[num_indexes[0]].ToDouble();
    }
    else {
      maxB = ins[num_indexes[0]].ToInt();
    }
  }
  else { // invalid argument set - reset any existing conn info
    ;
  }
  // remove numbers to leave atom names/types only
  for (size_t i = num_indexes.Count(); i > 0; i--) {
    ins.Delete(num_indexes[i - 1]);
  }
  // extract and remove atom types
  for (size_t i = 0; i < ins.Count(); i++) {
    if (ins[i].CharAt(0) == '$') {
      ConstSortedElementPList elms = TAtomReference::DecodeTypes(
        ins[i].SubStringFrom(1), rm.aunit);
      for (size_t ei = 0; ei < elms.Count(); ei++) {
        TypeConnInfo& ci = TypeInfo.Add(elms[ei], TypeConnInfo(*elms[ei]));
        ci.maxBonds = maxB;
        ci.r = r;
        for (size_t ai = 0; ai < AtomInfo.Count(); ai++) {
          if (AtomInfo.GetKey(ai)->GetType() == *elms[ei]) {
            AtomInfo.GetValue(ai).maxBonds = maxB;
            AtomInfo.GetValue(ai).r = r;
          }
        }
      }
      ins.Delete(i--);
    }
  }
  if (!ins.IsEmpty()) {
    TAtomReference ar(ins.Text(' '));
    TCAtomGroup ag;
    size_t aag;
    try { ar.Expand(rm, ag, EmptyString(), aag); }
    catch (const TExceptionBase&) {}
    if (ag.IsEmpty()) {
      TBasicApp::NewLogEntry(logError) << "Undefined atom in CONN: " <<
        ins.Text(' ');
      return;
    }
    for (size_t i = 0; i < ag.Count(); i++) {
      bool add = true;
      if (maxB == def_max_bonds && r == -1) {  // reset to default?
        size_t ai = AtomInfo.IndexOf(ag[i].GetAtom());
        if (ai != InvalidIndex) {
          AtomInfo.Delete(ai);
          add = false;
        }
        // no specialisation, check if there is a type override
        else {
          size_t e_idx = TypeInfo.IndexOf(ag[i].GetAtom()->GetType());
          if (e_idx != InvalidIndex) {
            const TCAtomPList &atoms = ag[i].GetAtom()->GetParent()->GetAtoms();
            size_t ac = 0;
            for (size_t ai = 0; ai < atoms.Count(); ai++) {
              if (!atoms[ai]->IsDeleted() &&
                atoms[ai]->GetType() == ag[i].GetAtom()->GetType())
              {
                ac++;
              }
            }
            if (ac == 1) {
              TypeInfo.Delete(e_idx);
              add = false;
            }
          }
        }
      }
      if (add) {
        AtomConnInfo& ai = AtomInfo.Add(ag[i].GetAtom(),
          AtomConnInfo(*ag[i].GetAtom()));
        ai.maxBonds = maxB;
        ai.r = r;
        ai.temporary = false;
      }
    }
  }
  // update temporary connectivity objects
  for (size_t i = 0; i < AtomInfo.Count(); i++) {
    AtomConnInfo &ai = AtomInfo.GetValue(i);
    if (ai.temporary) {
      size_t ti = TypeInfo.IndexOf(&ai.atom->GetType());
      if (ti != InvalidIndex) {
        ai.maxBonds = TypeInfo.GetValue(ti).maxBonds;
        ai.r = TypeInfo.GetValue(ti).r;
      }
    }
  }
}
//........................................................................
void ConnInfo::ToInsList(TStrList& ins) const {
  // put the type specific info first
  olxset<const cm_Element *, TPointerComparator> element_types;
  for (size_t i = 0; i < TypeInfo.Count(); i++) {
    const TypeConnInfo& tci = TypeInfo.GetValue(i);
    if (tci.maxBonds == def_max_bonds && tci.r == -1) {
      continue;
    }
    element_types.Add(tci.atomType);
    olxstr& str = ins.Add("CONN ");
    if (tci.maxBonds != def_max_bonds || tci.r != -1) {
      str << tci.maxBonds << ' ';
    }
    if (tci.r != -1) {
      str << tci.r << ' ';
    }
    str << '$' << tci.atomType->symbol;
  }
  // specialisation for particular atoms to follow the generic type info
  for (size_t i = 0; i < AtomInfo.Count(); i++) {
    const AtomConnInfo& aci = AtomInfo.GetValue(i);
    if (aci.atom->IsDeleted()) {
      continue;
    }
    if (!aci.temporary) {
      bool has_elm = element_types.Contains(aci.atom->GetType());
      // check if defined by the element
      if (has_elm) {
        const TypeConnInfo& tci = TypeInfo[aci.atom->GetType()];
        if (aci.r == tci.r && aci.maxBonds == tci.maxBonds) {
          continue;
        }
      }
      if (has_elm || aci.r != -1 || aci.maxBonds != def_max_bonds) {
        olxstr &str = ins.Add("CONN");
        if (aci.maxBonds != def_max_bonds || aci.r != -1 || has_elm) {
          str << ' ' << aci.maxBonds;
        }
        if (aci.r != -1) {
          str << ' ' << aci.r;
        }
        str << ' ' << aci.atom->GetResiLabel();
      }
    }
    for (size_t j = 0; j < aci.BondsToCreate.Count(); j++) {
      const CXBondInfo& bi = aci.BondsToCreate[j];
      if (bi.to.IsDeleted()) {
        continue;
      }
      olxstr& str = ins.Add("BIND ");
      str << aci.atom->GetResiLabel() << ' ' << bi.to.GetResiLabel();
      if (bi.matr != 0) {
        size_t si = rm.UsedSymmIndex(*bi.matr);
        if (si == InvalidIndex) {
          throw TFunctionFailedException(__OlxSourceInfo,
            "Undefined EQIV in BIND");
        }
        str << "_$" << (si + 1);
      }
    }
    for (size_t j = 0; j < aci.BondsToRemove.Count(); j++) {
      const CXBondInfo& bi = aci.BondsToRemove[j];
      if (bi.to.IsDeleted()) {
        continue;
      }
      olxstr& str = ins.Add("FREE ");
      str << aci.atom->GetResiLabel() << ' ' << bi.to.GetResiLabel();
      if (bi.matr != 0) {
        size_t si = rm.UsedSymmIndex(*bi.matr);
        if (si == InvalidIndex) {
          throw TFunctionFailedException(__OlxSourceInfo,
            "Undefined EQIV in BIND");
        }
        str << "_$" << (si + 1);
      }
    }
  }
  for (size_t i = 0; i < PartGroups.Count(); i++) {
    if (PartGroups[i].Count() < 2) {
      continue;
    }
    ins.Add("BIND ") << olxstr(' ').Join(PartGroups[i]);
  }
}
//........................................................................
CXConnInfo& ConnInfo::GetConnInfo(const TCAtom& ca) const {
  CXConnInfo& ci = *(new CXConnInfo);
  size_t ai_ind, ti_ind;
  if( (ti_ind = TypeInfo.IndexOf(&ca.GetType())) != InvalidIndex )  {
    const TypeConnInfo& aci = TypeInfo.GetValue(ti_ind);
    ci.r = (aci.r < 0 ? ca.GetType().r_bonding : aci.r);
    ci.maxBonds = aci.maxBonds;
  }
  // specialise the connectivity info...
  if( (ai_ind = AtomInfo.IndexOf(&ca)) != InvalidIndex )  {
    const AtomConnInfo& aci = AtomInfo.GetValue(ai_ind);
    ci.r = aci.r < 0 ? ca.GetType().r_bonding : aci.r;
    ci.maxBonds = aci.maxBonds;
    ci.BondsToCreate.AddAll(aci.BondsToCreate);
    ci.BondsToRemove.AddAll(aci.BondsToRemove);
  }
  // use defaults then
  if( ai_ind == InvalidIndex && ti_ind == InvalidIndex )  {
    ci.r = ca.GetType().r_bonding;
    ci.maxBonds = def_max_bonds;
  }
  return ci;
}
//........................................................................
CXConnInfo& ConnInfo::GetConnInfo(const cm_Element& elm) const {
  CXConnInfo& ci = *(new CXConnInfo);
  size_t ti_ind = TypeInfo.IndexOf(&elm);
  if( ti_ind != InvalidIndex )  {
    const TypeConnInfo& aci = TypeInfo.GetValue(ti_ind);
    ci.r = (aci.r < 0 ? elm.r_bonding : aci.r);
    ci.maxBonds = aci.maxBonds;
  }
  else  {
    ci.r = elm.r_bonding;
    ci.maxBonds = def_max_bonds;
  }
  return ci;
}
//........................................................................
void ConnInfo::Assign(const ConnInfo& ci)  {
  Clear();
  for( size_t i=0; i < ci.AtomInfo.Count(); i++ )  {
    const AtomConnInfo& _aci = ci.AtomInfo.GetValue(i);
    if( _aci.atom->IsDeleted() )
      continue;
    TCAtom* ca = rm.aunit.FindCAtomById(_aci.atom->GetId());
    if( ca == NULL ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Asymmetric units mismatch");
    }
    AtomConnInfo& aci = AtomInfo.Add(ca);
    aci.atom = ca;
    (CXConnInfoBase&)aci = _aci;
    aci.temporary = _aci.temporary;
    for( size_t j=0; j < _aci.BondsToCreate.Count(); j++ )  {
      ca = rm.aunit.FindCAtomById(_aci.BondsToCreate[j].to.GetId());
      if( ca == NULL ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Asymmetric units mismatch");
      }
      if( ca->IsDeleted() )
        continue;
      const smatd* sm = _aci.BondsToCreate[j].matr == NULL ? NULL
        : &rm.AddUsedSymm(*_aci.BondsToCreate[j].matr);
      aci.BondsToCreate.Add(new CXBondInfo(*ca, sm));
    }
    for( size_t j=0; j < _aci.BondsToRemove.Count(); j++ )  {
      ca = rm.aunit.FindCAtomById(_aci.BondsToRemove[j].to.GetId());
      if( ca == NULL ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Asymmetric units mismatch");
      }
      if( ca->IsDeleted() )
        continue;
      const smatd* sm = _aci.BondsToRemove[j].matr == NULL ? NULL
        : &rm.AddUsedSymm(*_aci.BondsToRemove[j].matr);
      aci.BondsToRemove.Add(new CXBondInfo(*ca, sm));
    }
  }
  for( size_t i=0; i < ci.TypeInfo.Count(); i++ )
    TypeInfo.Add(ci.TypeInfo.GetKey(i), ci.TypeInfo.GetValue(i));
  PartGroups = ci.PartGroups;
  PartGroups_ = ci.PartGroups_;
}
//........................................................................
void ConnInfo::ToDataItem(TDataItem& item) const {
  TDataItem& ti_item = item.AddItem("TYPE");
  for( size_t i=0; i < TypeInfo.Count(); i++ ) {
    TypeInfo.GetValue(i).ToDataItem(
      ti_item.AddItem(TypeInfo.GetValue(i).atomType->symbol));
  }
  TDataItem& ai_item = item.AddItem("ATOM");
  for( size_t i=0; i < AtomInfo.Count(); i++ )  {
    if( AtomInfo.GetValue(i).atom->IsDeleted() )  continue;
    AtomInfo.GetValue(i).ToDataItem(
      ai_item.AddItem(AtomInfo.GetValue(i).atom->GetTag()));
  }
  if (!PartGroups.IsEmpty()) {
    TDataItem& groups = item.AddItem("PART");
    size_t cnt=0;
    for (size_t i=0; i < PartGroups.Count(); i++) {
      if (PartGroups[i].Count() < 2) continue;
      TDataItem& group = groups.AddItem(++cnt, olxstr(' ').Join(PartGroups[i]));
    }
  }
}
//........................................................................
void ConnInfo::FromDataItem(const TDataItem& item)  {
  Clear();
  TDataItem& ti_item = item.GetItemByName("TYPE");
  for( size_t i=0; i < ti_item.ItemCount(); i++ )  {
    cm_Element* elm = XElementLib::FindBySymbol(
      ti_item.GetItemByIndex(i).GetName());
    if( elm == NULL ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("Unknown symbol: ") << ti_item.GetItemByIndex(i).GetName());
    }
    TypeInfo.Add(elm).FromDataItem(ti_item.GetItemByIndex(i), elm);

  }
  TDataItem& ai_item = item.GetItemByName("ATOM");
  for( size_t i=0; i < ai_item.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom(
      ai_item.GetItemByIndex(i).GetName().ToSizeT());
    AtomInfo.Add(&ca).FromDataItem(ai_item.GetItemByIndex(i), rm, ca);
  }
  TDataItem* groups = item.FindItem("PART");
  if (groups != NULL) {
    for (size_t i=0; i < groups->ItemCount(); i++) {
      PartGroups.AddNew().FromList(
        TStrList(groups->GetItemByIndex(i).GetValue(), ' '),
        FunctionAccessor::MakeConst(&olxstr::ToInt));
    }
  }
}
#ifdef _PYTHON
PyObject* ConnInfo::PyExport()  {
  PyObject* main = PyDict_New(),
    *type = PyDict_New(),
    *atom = PyDict_New();
  SortedElementPList types;
  for (size_t i=0; i < rm.aunit.AtomCount(); i++) {
    if (!rm.aunit.GetAtom(i).IsDeleted() &&
      rm.aunit.GetAtom(i).GetType() != iQPeakZ)
    {
      types.AddUnique(&rm.aunit.GetAtom(i).GetType());
    }
  }
  for( size_t i=0; i < TypeInfo.Count(); i++ )  {
    PythonExt::SetDictItem(type, TypeInfo.GetValue(i).atomType->symbol,
      TypeInfo.GetValue(i).PyExport());
    types.Remove(TypeInfo.GetValue(i).atomType);
  }
  for( size_t i=0; i < types.Count(); i++ )  {
    PythonExt::SetDictItem(type, types[i]->symbol,
      Py_BuildValue("{s:f,s:i}", "radius", types[i]->r_bonding, "bonds", 12));
  }
  for( size_t i=0; i < AtomInfo.Count(); i++ )  {
    PythonExt::SetDictItem(atom, Py_BuildValue("i",
      AtomInfo.GetValue(i).atom->GetTag()),
      AtomInfo.GetValue(i).PyExport());
  }
  PythonExt::SetDictItem(main, "type", type);
  PythonExt::SetDictItem(main, "atom", atom);
  PythonExt::SetDictItem(main, "delta",
    Py_BuildValue("f", rm.aunit.GetLattice().GetDelta()));
  size_t gc=0;
  for (size_t i=0; i < PartGroups.Count(); i++) {
    if (PartGroups[i].Count() > 1)
      gc++;
  }
  PyObject *part = PyList_New(gc);
  for (size_t i=0; i < PartGroups.Count(); i++) {
    if (PartGroups[i].Count() < 2) continue;
    PyObject *p = PyList_New(PartGroups[i].Count());
    for (size_t j=0; j < PartGroups[i].Count(); j++) {
      PyList_SetItem(p, j, Py_BuildValue("i", PartGroups[i][j]));
    }
    PyList_SetItem(part, i, p);
  }
  PythonExt::SetDictItem(main, "part", part);
  return main;
}
#endif
//........................................................................
//........................................................................
//........................................................................
void ConnInfo::TypeConnInfo::ToDataItem(TDataItem& item) const {
  item.AddField("r", r);
  item.AddField("b", maxBonds);
}
void ConnInfo::TypeConnInfo::FromDataItem(const TDataItem& item,
  const cm_Element* elm)
{
  atomType = elm;
  r = item.GetFieldByName("r").ToDouble();
  maxBonds = item.GetFieldByName("b").ToInt();
}
//........................................................................
#ifdef _PYTHON
PyObject* ConnInfo::TypeConnInfo::PyExport()  {
  return Py_BuildValue("{s:f,s:i}", "radius",
    r < 0 ? atomType->r_bonding : r, "bonds", maxBonds);
}
#endif
//........................................................................
size_t ConnInfo::FindBondIndex(const BondInfoList& list, TCAtom* key,
  TCAtom& a1, TCAtom& a2, const smatd* eqiv)
{
  if( (key != &a1 && key != &a2) || list.IsEmpty() )
    return InvalidIndex;
  if( key == &a1 )  {
    for( size_t i=0; i < list.Count(); i++ )  {
      if( list[i].to == a2 )  {
        if( list[i].matr == NULL && eqiv == NULL )
          return i;
        if( list[i].matr != NULL && eqiv != NULL && *list[i].matr == *eqiv )
          return i;
      }
    }
  }
  else if( eqiv == NULL )  {
    for( size_t i=0; i < list.Count(); i++ )  {
      if( list[i].to == a1 )
        return i;
    }
  }
  return InvalidIndex;
}
//........................................................................
const smatd* ConnInfo::GetCorrectMatrix(const smatd* eqiv1, const smatd* eqiv2,
  bool release) const
{
  if( eqiv1 == NULL || (eqiv1->r.IsI() && eqiv1->t.IsNull()) )  {
    if( release && eqiv1 != NULL )
      rm.RemUsedSymm(*eqiv1);
    return (eqiv2 == NULL || (eqiv2->r.IsI() && eqiv2->t.IsNull()) ?
      NULL : eqiv2);
  }
  smatd mat;
  if (!rm.aunit.HasLattice()) {  // no lattice?
    if( eqiv2 == NULL || (eqiv2->r.IsI() && eqiv2->t.IsNull()) )  {
      mat = eqiv1->Inverse();
      if( release )  {
        rm.RemUsedSymm(*eqiv1);
        if( eqiv2 != NULL )
          rm.RemUsedSymm(*eqiv2);
      }
    }
    else  {
      mat = ((*eqiv2)*eqiv1->Inverse());
      if( release )  {
        rm.RemUsedSymm(*eqiv1);
        rm.RemUsedSymm(*eqiv2);
      }
    }
  }
  else {
    const TUnitCell& uc = rm.aunit.GetLattice().GetUnitCell();
    if( eqiv2 == NULL || (eqiv2->r.IsI() && eqiv2->t.IsNull()) )  {
      mat = uc.InvMatrix(*eqiv1);
      if( release )  {
        rm.RemUsedSymm(*eqiv1);
        if( eqiv2 != NULL )
          rm.RemUsedSymm(*eqiv2);
      }
    }
    else  {
      mat = uc.MulMatrix(*eqiv2, uc.InvMatrix(*eqiv1));
      if( release )  {
        rm.RemUsedSymm(*eqiv1);
        rm.RemUsedSymm(*eqiv2);
      }
    }
  }
  return ((mat.r.IsI() && mat.t.IsNull()) ? NULL : &rm.AddUsedSymm(mat));
}
//........................................................................
void ConnInfo::AddBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1,
  const smatd* eqiv2, bool release_eqiv)
{
  const smatd* eqiv = GetCorrectMatrix(eqiv1, eqiv2, release_eqiv);
  size_t ind = InvalidIndex;
  for (size_t i=0; i < AtomInfo.Count(); i++) {
    ind = FindBondIndex(AtomInfo.GetValue(i).BondsToCreate, AtomInfo.GetKey(i),
      a1, a2, eqiv);
    if (ind != InvalidIndex)
      break;
  }
  if (ind == InvalidIndex) {
    const size_t aii = AtomInfo.IndexOf(&a1);
    AtomConnInfo *ci = NULL;
    if (aii != InvalidIndex) {
      ci = &AtomInfo.GetValue(aii);
    }
    else {
      ci = &AtomInfo.Add(&a1, AtomConnInfo(a1));
      ci->temporary = true;
      size_t ti = TypeInfo.IndexOf(a1.GetType());
      if (ti != InvalidIndex) {
        ci->maxBonds = TypeInfo.GetValue(ti).maxBonds;
        ci->r = TypeInfo.GetValue(ti).r;
      }
    }
    ci->BondsToCreate.Add(new CXBondInfo(a2, eqiv));
  }
  // validate the bonds to delete
  for (size_t i=0; i < AtomInfo.Count(); i++) {
    ind = FindBondIndex(AtomInfo.GetValue(i).BondsToRemove, AtomInfo.GetKey(i),
      a1, a2, eqiv);
    if (ind != InvalidIndex) {
      AtomInfo.GetValue(i).BondsToRemove.Delete(ind);
      break;
    }
  }
}
//........................................................................
void ConnInfo::RemBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1,
  const smatd* eqiv2, bool release_eqiv)
{
  const smatd* eqiv = GetCorrectMatrix(eqiv1, eqiv2, release_eqiv);
  size_t ind = InvalidIndex;
  for (size_t i=0; i < AtomInfo.Count(); i++) {
    ind = FindBondIndex(AtomInfo.GetValue(i).BondsToRemove, AtomInfo.GetKey(i),
      a1, a2, eqiv);
    if (ind != InvalidIndex)
      break;
  }
  if (ind == InvalidIndex) {
    // validate the bonds to create
    for (size_t i=0; i < AtomInfo.Count(); i++) {
      ind = FindBondIndex(AtomInfo.GetValue(i).BondsToCreate,
        AtomInfo.GetKey(i), a1, a2, eqiv);
      if (ind != InvalidIndex) {
        AtomInfo.GetValue(i).BondsToCreate.Delete(ind);
        break;
      }
    }
    const size_t aii = AtomInfo.IndexOf(&a1);
    AtomConnInfo *ci = NULL;
    if (aii != InvalidIndex) {
      ci = &AtomInfo.GetValue(aii);
    }
    else {
      ci = &AtomInfo.Add(&a1, AtomConnInfo(a1));
      ci->temporary = true;
      size_t ti = TypeInfo.IndexOf(a1.GetType());
      if (ti != InvalidIndex) {
        ci->maxBonds = TypeInfo.GetValue(ti).maxBonds;
        ci->r = TypeInfo.GetValue(ti).r;
      }
    }
    ci->BondsToRemove.Add(new CXBondInfo(a2, eqiv));
  }
}
//........................................................................
void ConnInfo::Compile(const TCAtom& a, BondInfoList& toCreate,
  BondInfoList& toDelete, smatd_list& ml)
{
  const TAsymmUnit& au = *a.GetParent();
  const TUnitCell& uc = a.GetParent()->GetLattice().GetUnitCell();
  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom& ca = au.GetAtom(i);
    if (ca.IsDeleted())  continue;
    CXConnInfo& ci = ca.GetConnInfo();
    if (ca != a) {
      for (size_t j=0; j < ci.BondsToRemove.Count(); j++) {
        if (ci.BondsToRemove[j].to == a) {
          smatd matr = ci.BondsToRemove[j].matr == NULL ? uc.GetMatrix(0)
            : uc.InvMatrix(*ci.BondsToRemove[j].matr);
          bool uniq = true;
          for (size_t k=0; k < toDelete.Count(); k++) {
            if (toDelete[k].to == ca &&
                ((toDelete[k].matr != NULL &&
                  toDelete[k].matr->GetId() == matr.GetId()) ||
                (toDelete[k].matr == NULL && ci.BondsToRemove[j].matr == NULL)))
            {
              uniq = false;
              break;
            }
          }
          if (uniq)
            toDelete.Add(new CXBondInfo(ca, &ml.AddCopy(matr)));
        }
      }
      for (size_t j=0; j < ci.BondsToCreate.Count(); j++) {
        if (ci.BondsToCreate[j].to == a) {
          smatd matr = ci.BondsToCreate[j].matr == NULL ? uc.GetMatrix(0)
            : uc.InvMatrix(*ci.BondsToCreate[j].matr);
          bool uniq = true;
          for (size_t k=0; k < toCreate.Count(); k++) {
            if (toCreate[k].to == ca &&
                ((toCreate[k].matr != NULL &&
                  toCreate[k].matr->GetId() == matr.GetId()) ||
                (toCreate[k].matr == NULL && ci.BondsToCreate[j].matr == NULL)))
            {
              uniq = false;
              break;
            }
          }
          if (uniq)
            toCreate.Add(new CXBondInfo(ca, &ml.AddCopy(matr)));
        }
      }
    }
    else {  // own connectivity
      for (size_t i = 0; i < ci.BondsToCreate.Count(); i++) {
        toCreate.Add(new CXBondInfo(ci.BondsToCreate[i].to,
          ci.BondsToCreate[i].matr == NULL ? NULL
          : &ml.AddCopy(*ci.BondsToCreate[i].matr)));
      }
      for (size_t i = 0; i < ci.BondsToRemove.Count(); i++) {
        toDelete.Add(new CXBondInfo(ci.BondsToRemove[i].to,
          ci.BondsToRemove[i].matr == NULL ? NULL
          : &ml.AddCopy(*ci.BondsToRemove[i].matr)));
      }
    }
  }
  //for( size_t i=0; i < ml.Count(); i++ )
  //  uc.InitMatrixId(ml[i]);
}
//........................................................................
void ConnInfo::AtomConnInfo::ToDataItem(TDataItem& item) const {
  item.AddField('r', r)
    .AddField('b', maxBonds)
    .AddField('t', temporary);
  TDataItem& ab = item.AddItem("ADDBOND");
  for( size_t i=0; i < BondsToCreate.Count(); i++ )  {
    if( BondsToCreate[i].to.IsDeleted() )
      continue;
    TDataItem& bi = ab.AddItem("bi");
    bi.AddField("to", BondsToCreate[i].to.GetTag());
    if( BondsToCreate[i].matr != NULL )
      bi.AddField("eqiv", BondsToCreate[i].matr->GetId());
  }
  TDataItem& db = item.AddItem("DELBOND");
  for( size_t i=0; i < BondsToRemove.Count(); i++ )  {
    if( BondsToRemove[i].to.IsDeleted() )
      continue;
    TDataItem& bi = db.AddItem("bi");
    bi.AddField("to", BondsToRemove[i].to.GetTag());
    if( BondsToRemove[i].matr != NULL )
      bi.AddField("eqiv", BondsToRemove[i].matr->GetId());
  }
}
void ConnInfo::AtomConnInfo::FromDataItem(const TDataItem& item,
  RefinementModel& rm, TCAtom& a)
{
  atom = &a;
  r = item.GetFieldByName('r').ToDouble();
  maxBonds = item.GetFieldByName('b').ToInt();
  temporary = item.FindField('t', FalseString()).ToBool();
  TDataItem& ab = item.GetItemByName("ADDBOND");
  for( size_t i=0; i < ab.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom(
      ab.GetItemByIndex(i).GetFieldByName("to").ToInt());
    const olxstr& eq = ab.GetItemByIndex(i).FindField("eqiv");
    smatd const* eqiv = NULL;
    if( !eq.IsEmpty() )  {
      eqiv = &rm.GetUsedSymm(eq.ToInt());
      rm.AddUsedSymm(*eqiv);  // persist
    }
    BondsToCreate.Add(new CXBondInfo(ca, eqiv));
  }
  TDataItem& db = item.GetItemByName("DELBOND");
  for( size_t i=0; i < db.ItemCount(); i++ )  {
    TCAtom& ca = rm.aunit.GetAtom(
      db.GetItemByIndex(i).GetFieldByName("to").ToInt());
    const olxstr& eq = db.GetItemByIndex(i).FindField("eqiv");
    smatd const* eqiv = NULL;
    if( !eq.IsEmpty() )  {
      eqiv = &rm.GetUsedSymm(eq.ToInt());
      rm.AddUsedSymm(*eqiv);  // persist
    }
    BondsToRemove.Add(new CXBondInfo(ca, eqiv));
  }
}
//........................................................................
#ifdef _PYTHON
PyObject* ConnInfo::AtomConnInfo::PyExport()  {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "radius", Py_BuildValue("f", r));
  PythonExt::SetDictItem(main, "bonds", Py_BuildValue("i", maxBonds));
  size_t bc = 0;
  for( size_t i=0; i < BondsToCreate.Count(); i++ )  {
    if( BondsToCreate[i].to.IsDeleted() )  continue;
    bc++;
  }
  if( bc > 0 )  {
    PyObject* btc = PyTuple_New(bc);
    bc = 0;
    for( size_t i=0; i < BondsToCreate.Count(); i++ )  {
      if( BondsToCreate[i].to.IsDeleted() )  continue;
      PyTuple_SetItem(btc, bc++,
        Py_BuildValue("{s:i,s:i}", "to", BondsToCreate[i].to.GetTag(), "eqiv",
          BondsToCreate[i].matr == NULL ? -1 : BondsToCreate[i].matr->GetId()));
    }
    PythonExt::SetDictItem(main, "create", btc);
  }
  bc = 0;
  for( size_t i=0; i < BondsToRemove.Count(); i++ )  {
    if( BondsToRemove[i].to.IsDeleted() )  continue;
    bc++;
  }
  if( bc > 0 )  {
    PyObject* btd = PyTuple_New(bc);
    bc = 0;
    for( size_t i=0; i < BondsToRemove.Count(); i++ )  {
      if( BondsToRemove[i].to.IsDeleted() )  continue;
      PyTuple_SetItem(btd, bc++,
        Py_BuildValue("{s:i,s:i}", "to", BondsToRemove[i].to.GetTag(), "eqiv",
          BondsToRemove[i].matr == NULL ? -1 : BondsToRemove[i].matr->GetId()));
    }
    PythonExt::SetDictItem(main, "delete", btd);
  }
  return main;
}
#endif

/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "infotab.h"

bool InfoTab::operator == (const InfoTab &it) const {
  if ( Type != it.Type)  return false;
  // planes...
  if (Type == infotab_mpla) return false;
  if (atoms.GetResi() != it.atoms.GetResi()) return false;
  if (atoms.Count() != it.atoms.Count())  return false;
  TTypeList<ExplicitCAtomRef> ra = atoms.ExpandList(RM);
  TTypeList<ExplicitCAtomRef> rb = it.atoms.ExpandList(RM);
  if (ra.Count() != rb.Count()) return false;
  for (size_t i=0; i < ra.Count(); i++) {
    if (ra[i].GetAtom() != rb[i].GetAtom() ||
      ra[i].GetMatrix() != rb[i].GetMatrix())
    {
      return false;
    }
  }
  return true;
}

InfoTab& InfoTab::operator = (const InfoTab& it)  {
  ParamName = it.ParamName;
  Type = it.Type;
  atoms.Assign(it.atoms);
  return *this;
}

bool InfoTab::IsValid() const {
 TTypeList<ExplicitCAtomRef> a = atoms.ExpandList(RM);
 size_t ac = a.Count();
 if (ac == 0) return false;
  if ((Type == infotab_htab || Type == infotab_bond) && (ac%2) == 0)
    return true;
  if (Type == infotab_rtab && ac >= 1 && ac <= 4 && !ParamName.IsEmpty())
    return true;
  if (Type == infotab_mpla && ac >= 3) return true;
  return false;
}

olxstr InfoTab::InsStr() const {
  olxstr rv = GetName();
  if (!atoms.GetResi().IsEmpty())
    rv << '_' << atoms.GetResi();
  return (rv << ' ' << atoms.GetExpression());
}

void InfoTab::ToDataItem(TDataItem& di) const {
  di.SetValue(GetName());
  di.AddField("param", ParamName);
  atoms.ToDataItem(di.AddItem("AtomList"));
}

void InfoTab::FromDataItem(const TDataItem& di, RefinementModel& rm)  {
  if( di.GetValue() == "HTAB")
    Type = infotab_htab;
  else if( di.GetValue() == "RTAB")
    Type = infotab_rtab;
  else if( di.GetValue() == "MPLA")
    Type = infotab_mpla;
  else
    Type = infotab_bond;
  ParamName = di.GetFieldValue("param");
  TDataItem *ais = di.FindItem("atoms");
  if (ais != NULL) {
    //ResiName = di.GetFieldValue("resi");
    for( size_t i=0; i < ais->ItemCount(); i++ )  {
      const TDataItem& ai = ais->GetItem(i);
      size_t atom_id = ai.GetValue().ToSizeT();
      olxstr matr_id = ai.GetFieldValue("matrix");
      AddAtom(rm.aunit.GetAtom(atom_id),
        matr_id.IsEmpty() ? NULL : &rm.GetUsedSymm(matr_id.ToSizeT()));
    }
  }
  else {
    atoms.FromDataItem(*ais);
  }
}

#ifndef _NO_PYTHON
PyObject* InfoTab::PyExport() {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "type",
    PythonExt::BuildString(GetName()));
  PythonExt::SetDictItem(main, "param_name", PythonExt::BuildString(ParamName));
  TTypeList<ExplicitCAtomRef> a = atoms.ExpandList(RM);
  PyObject* pya = PyTuple_New(a.Count());
  for( size_t i=0; i < a.Count(); i++ )  {
    PyTuple_SetItem(pya, i,
      Py_BuildValue("(i,i)", a[i].GetAtom().GetTag(),
      a[i].GetMatrix() == NULL ? -1 : a[i].GetMatrix()->GetId()));
  }
  PythonExt::SetDictItem(main, "atoms", pya);
  return main;
}

TIString InfoTab::ToString() const {
  olxstr rv = InsStr();
  TTypeList<ExplicitCAtomRef> a = atoms.ExpandList(RM);
  if (Type == infotab_htab && a.Count() == 2) {
    vec3d p1 = a[0].GetAtom().ccrd();
    if (a[0].GetMatrix() != NULL)
      p1 = *a[0].GetMatrix() * p1;
    vec3d p2 = a[1].GetAtom().ccrd();
    if (a[1].GetMatrix() != NULL)
      p2 = *a[1].GetMatrix() * p2;
    rv << " d=" << olx_round(
      RM.aunit.Orthogonalise(p2-p1).Length(), 1000);
  }
  return rv;
}

void InfoTab::AddAtom(TCAtom& ca, const smatd* sm)  {
  atoms.AddExplicit(ca, sm);
}

#endif

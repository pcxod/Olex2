#include "afixgroup.h"
#include "refmodel.h"

void TAfixGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TAfixGroup::Assign(const TAfixGroup& ag)  {
  D = ag.D;
  Sof = ag.Sof;
  U = ag.U;
  Afix = ag.Afix;
  
  Pivot = Parent.RM.aunit.FindCAtomById(ag.Pivot->GetId());
  if( Pivot == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  SetPivot( *Pivot );
  for( size_t i=0; i < ag.Dependent.Count(); i++ )  {
    Dependent.Add(Parent.RM.aunit.FindCAtomById( ag.Dependent[i]->GetId()));
    if( Dependent.Last() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    Dependent.Last()->SetParentAfixGroup(this);
  }
}
//..............................................................................
void TAfixGroup::ToDataItem(TDataItem& item) const {
  item.AddField("afix", Afix);
  item.AddField("d", D);
  item.AddField("u", U);
  item.AddField("pivot_atom_id", Pivot->GetTag());
  TDataItem& dep = item.AddItem("dependent");
  int dep_id = 0;
  for( size_t i=0; i < Dependent.Count(); i++ )  {
    if( Dependent[i]->IsDeleted() )  continue;
    dep.AddField(olxstr("atom_id_") << dep_id++, Dependent[i]->GetTag());
  }
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TAfixGroup::PyExport(TPtrList<PyObject>& atoms)  {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "afix", Py_BuildValue("i", Afix));
  PythonExt::SetDictItem(main, "d", Py_BuildValue("d", U));
  PythonExt::SetDictItem(main, "u", Py_BuildValue("d", D));
  PythonExt::SetDictItem(main, "pivot", Py_BuildValue("i", Pivot->GetTag()));
  int dep_cnt = 0;
  for( size_t i=0; i < Dependent.Count(); i++ )  {
    if( Dependent[i]->IsDeleted() )  continue;
    dep_cnt++;
  }
  PyObject* dependent = PyTuple_New(dep_cnt);
  dep_cnt = 0;
  for( size_t i=0; i < Dependent.Count(); i++ )  {
    if( Dependent[i]->IsDeleted() )  continue;
    PyTuple_SetItem(dependent, dep_cnt++, Py_BuildValue("i", Dependent[i]->GetTag()));
  }
  PythonExt::SetDictItem(main, "dependent", dependent);
  return main;
}
#endif
//..............................................................................
void TAfixGroup::FromDataItem(TDataItem& item) {
  Afix = item.GetRequiredField("afix").ToInt();
  D = item.GetRequiredField("d").ToDouble();
  U = item.GetRequiredField("u").ToDouble();
  SetPivot(Parent.RM.aunit.GetAtom(item.GetRequiredField("pivot_atom_id").ToSizeT()));
  TDataItem& dep = item.FindRequiredItem("dependent");
  for( size_t i=0; i < dep.FieldCount(); i++ )
    Dependent.Add( &Parent.RM.aunit.GetAtom(dep.GetField(i).ToInt()) )->SetParentAfixGroup(this);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAfixGroups::ToDataItem(TDataItem& item) {
  int group_id = 0;
  for( size_t i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();
  item.AddField("n", Groups.Count());
  for( size_t i=0; i < Groups.Count(); i++ )
    Groups[i].ToDataItem(item.AddItem(i));
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TAfixGroups::PyExport(TPtrList<PyObject>& atoms)  {
  int group_id = 0;
  for( size_t i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();

  PyObject* main = PyTuple_New( Groups.Count() );
  for( size_t i=0; i < Groups.Count(); i++ )  {
    PyTuple_SetItem(main, i, Groups[i].PyExport(atoms));
  }
  return main;
}
#endif
//..............................................................................
void TAfixGroups::FromDataItem(TDataItem& item) {
  Clear();
  size_t n = item.GetRequiredField("n").ToSizeT();
  if( n != item.ItemCount() )
    throw TFunctionFailedException(__OlxSourceInfo, "number of items mismatch");
  for( size_t i=0; i < n; i++ )  {
    Groups.Add(new TAfixGroup(*this)).SetId(i);
    Groups.Last().FromDataItem(item.GetItem(i));
  }
}

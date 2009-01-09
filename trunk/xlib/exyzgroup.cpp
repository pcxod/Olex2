#include "exyzgroup.h"
#include "refmodel.h"

void TExyzGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TExyzGroup::Assign(const TExyzGroup& ag)  {
  for( int i=0; i < ag.Atoms.Count(); i++ )  {
    Atoms.Add( Parent.RM.aunit.FindCAtomByLoaderId( ag.Atoms[i]->GetLoaderId()) );
    if( Atoms.Last() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    Atoms.Last()->SetExyzGroup(this);
  }
}
//..............................................................................
void TExyzGroup::ToDataItem(TDataItem& item) const {
  int atom_id = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    item.AddField(olxstr("atom_id_") << atom_id++, Atoms[i]->GetTag());
  }
}
//..............................................................................
void TExyzGroup::FromDataItem(TDataItem& item) {
  Clear();
  for( int i=0; i < item.FieldCount(); i++ )
    Atoms.Add( &Parent.RM.aunit.GetAtom( item.GetField(i).ToInt()) );
}
//..............................................................................
//..............................................................................
//..............................................................................
void TExyzGroups::ToDataItem(TDataItem& item) {
  int group_id = 0;
  for( int i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();
  item.AddField("n", Groups.Count() );
  for( int i=0; i < Groups.Count(); i++ )  {
    Groups[i].ToDataItem( item.AddItem(group_id++) );
  }
}
//..............................................................................
void TExyzGroups::FromDataItem(TDataItem& item) {
  Clear();
  int n = item.GetRequiredField("n").ToInt();
  if( n != item.ItemCount() )
    throw TFunctionFailedException(__OlxSourceInfo, "number of items mismatch");
  for( int i=0; i < n; i++ )  {
    New().FromDataItem( item.GetItem(i) );
  }
}
//..............................................................................


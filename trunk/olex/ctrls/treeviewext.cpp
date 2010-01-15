#include "treeviewext.h"
#include "frameext.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TTreeView, wxGenericTreeCtrl)

enum  {
  ID_ExpandAll = 1000,
  ID_CollapseAll
};
BEGIN_EVENT_TABLE(TTreeView, wxGenericTreeCtrl)
  EVT_TREE_ITEM_ACTIVATED(-1, TTreeView::ItemActivateEvent)
  EVT_TREE_SEL_CHANGED(-1, TTreeView::SelectionEvent)
  EVT_TREE_END_LABEL_EDIT(-1, TTreeView::ItemEditEvent)
  EVT_LEFT_UP(TTreeView::OnMouseUp)
  EVT_RIGHT_UP(TTreeView::OnMouseUp)
  EVT_MENU(ID_ExpandAll, TTreeView::OnContextMenu)
  EVT_MENU(ID_CollapseAll, TTreeView::OnContextMenu)
END_EVENT_TABLE()

void TTreeView::ItemActivateEvent(wxTreeEvent& event)  {
  StartEvtProcessing()
    OnDblClick.Execute(this, &TEGC::New<olxstr>(GetOnItemActivateStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTreeView::SelectionEvent(wxTreeEvent& event) {
  StartEvtProcessing()
    OnSelect.Execute(this, &TEGC::New<olxstr>(GetOnSelectStr()));
  EndEvtProcessing()
}
//..............................................................................
void TTreeView::ItemEditEvent(wxTreeEvent& event) {
  StartEvtProcessing()
    OnSelect.Execute(this, &TEGC::New<olxstr>(GetOnEditStr()).Replace("~label~", event.GetLabel().c_str()));
  EndEvtProcessing()
}
//..............................................................................
size_t TTreeView::ReadStrings(size_t& index, const wxTreeItemId* thisCaller, const TStrList& strings)  {
  while( (index + 2) <= strings.Count() )  {
    size_t level = strings[index].LeadingCharCount( '\t' );
    index++;  // now index is on data string
    wxTreeItemId item;
    if( strings[index].Trim('\t').IsEmpty() )
      item = AppendItem(*thisCaller, olxstr(strings[index-1]).Trim('\t').u_str() );
    else
      item = AppendItem(*thisCaller, olxstr(strings[index-1]).Trim('\t').u_str(), -1, -1,
         new TTreeNodeData(new olxstr(strings[index])) );
    index++;  // and now on the next item
    if( index < strings.Count() )  {
      size_t nextlevel = strings[index].LeadingCharCount('\t');
      if( nextlevel > level )  {
        size_t slevel = ReadStrings(index, &item, strings);
        if( slevel != level )
          return slevel;
      }
      if( nextlevel < level )
        return  nextlevel;
    }
  }
  return 0;
}
//..............................................................................
void TTreeView::ClearData()  {
  return;
}
//..............................................................................
bool TTreeView::LoadFromStrings(const TStrList &strings)  {
  ClearData();
  DeleteAllItems();
  wxTreeItemId Root = AddRoot(wxT("Root"));
  size_t index = 0;
  ReadStrings(index, &Root, strings);
  return true;
}
//..............................................................................
void TTreeView::OnMouseUp(wxMouseEvent& me)  {
  me.Skip();
  if( Popup == NULL )  return;
  if( me.ButtonUp(wxMOUSE_BTN_RIGHT) )
    PopupMenu(Popup);
}
//..............................................................................
void TTreeView::OnContextMenu(wxCommandEvent& evt)  {
  if( evt.GetId() == ID_ExpandAll )  {
    ExpandAllChildren(GetSelection());
  }
  else if( evt.GetId() == ID_CollapseAll )  {
    CollapseAllChildren(GetSelection());
  }
}
//..............................................................................
size_t TTreeView::_SaveState(TEBitArray& res, const wxTreeItemId& item, size_t& counter) const {
  size_t selected = InvalidIndex;
  if( counter == 0 )  {
    if( (GetWindowStyle()&wxTR_HIDE_ROOT) == 0 )
      res.SetTrue(counter);
  }
  else if( IsExpanded(item) )
    res.SetTrue(counter);
  if( IsSelected(item) )  selected = counter;
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(item, cookie);
  while( ch_id.IsOk() )  {
    counter++;
    if( HasChildren(ch_id) )  {
      size_t sel = _SaveState(res, ch_id, counter);
      if( sel != InvalidIndex )
        selected = sel;
    }
    else  {
      res.SetFalse(counter);
      if( IsSelected(ch_id) )
        selected = counter;
    }
    ch_id = GetNextChild(item, cookie);
  }
  return selected;
}
//..............................................................................
olxstr TTreeView::SaveState() const {
  wxTreeItemId root = GetRootItem();
  TEBitArray res(GetChildrenCount(root, true)+1);
  size_t counter = 0;
  size_t selected = _SaveState(res, root, counter);
  olxstr rv = res.ToBase64String();
  return (rv << ';' << selected);
}
//..............................................................................
void TTreeView::_RestoreState(const TEBitArray& res, const wxTreeItemId& item, size_t& counter, size_t selected)  {
  if( res[counter] )  Expand(item);
  if( selected == counter )  SelectItem(item, true);
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(item, cookie);
  while( ch_id.IsOk() )  {
    counter++;
    if( HasChildren(ch_id) )
      _RestoreState(res, ch_id, counter, selected);
    else  {
      if( counter == selected )
        SelectItem(ch_id, true);
    }
    ch_id = GetNextChild(item, cookie);
  }
}
//..............................................................................
void TTreeView::RestoreState(const olxstr& state)  {
  size_t si = state.LastIndexOf(';');
  if( si == InvalidIndex )  return;
  wxTreeItemId root = GetRootItem();
  TEBitArray res;
  res.FromBase64String(state.SubStringTo(si));
  if( res.Count() != GetChildrenCount(root, true)+1 )
    return;
  size_t counter = 0;
  size_t selected = state.SubStringFrom(si+1).ToSizeT();
  OnSelect.SetEnabled(false);
  _RestoreState(res, root, counter, selected);
  OnSelect.SetEnabled(true);
}
//..............................................................................
wxTreeItemId TTreeView::_FindByLabel(const wxTreeItemId& root, const olxstr& label) const {
  if( label == GetItemText(root).c_str() )  return root;
  if( !HasChildren(root) )  return wxTreeItemId();
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(root, cookie);
  while( ch_id.IsOk() )  {
    wxTreeItemId id = _FindByLabel(ch_id, label);
    if( id.IsOk() )  return id;
    ch_id = GetNextChild(root, cookie);
  }
  return ch_id;
}
//..............................................................................
wxTreeItemId TTreeView::_FindByData(const wxTreeItemId& root, const olxstr& data) const {
  wxTreeItemData* _dt = GetItemData(root);
  if( _dt != NULL && EsdlInstanceOf(*_dt, TTreeNodeData) )  {
    TTreeNodeData* dt = (TTreeNodeData*)_dt;
    if( dt->GetData() != NULL && data == dt->GetData()->ToString() )
      return root;
  }
  if( !HasChildren(root) )  return wxTreeItemId();
  wxTreeItemIdValue cookie;
  wxTreeItemId ch_id = GetFirstChild(root, cookie);
  while( ch_id.IsOk() )  {
    wxTreeItemId id = _FindByData(ch_id, data);
    if( id.IsOk() )  return id;
    ch_id = GetNextChild(root, cookie);
  }
  return ch_id;
}
//..............................................................................
void TTreeView::SelectByLabel(const olxstr& label)  {
  wxTreeItemId item = _FindByLabel(GetRootItem(), label);
  if( item.IsOk() )  {
    OnSelect.SetEnabled(false);
    SelectItem(item);
    OnSelect.SetEnabled(true);
  }
}
//..............................................................................
void TTreeView::SelectByData(const olxstr& data)  {
  wxTreeItemId item = _FindByData(GetRootItem(), data);
  if( item.IsOk() )  {
    OnSelect.SetEnabled(false);
    SelectItem(item);
    OnSelect.SetEnabled(true);
  }
}
//..............................................................................

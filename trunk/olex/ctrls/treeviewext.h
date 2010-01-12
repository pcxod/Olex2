#ifndef __olx_ctrl_treeview_H
#define __olx_ctrl_treeview_H
#include "olxctrlbase.h"
#include "estrlist.h"
#include "wx/treectrl.h"
#include "wx/generic/treectlg.h"

namespace ctrl_ext  {

  class TTreeNodeData : public wxTreeItemData, public IEObject {
    IEObject* Data;
  public:
    TTreeNodeData(IEObject* obj) {  Data = obj;  }
    virtual ~TTreeNodeData()  { delete Data;  }
    inline IEObject* GetData() const {  return Data;  }
  };

  class TTreeView: public wxGenericTreeCtrl, public AOlxCtrl  {
    olxstr Data, OnItemActivateStr, OnSelectStr, OnEditStr;
  protected:
    void SelectionEvent(wxTreeEvent& event);
    void ItemActivateEvent(wxTreeEvent& event);
    void ItemEditEvent(wxTreeEvent& event);
    size_t ReadStrings(size_t& index, const wxTreeItemId* thisCaller, const TStrList& strings);
    void ClearData();
  public:
    TTreeView(wxWindow* Parent, long flags=(1|8)) :
      wxGenericTreeCtrl(Parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags), 
      AOlxCtrl(this),
      OnSelect(Actions.New(evt_on_select_id)),
      OnDblClick(Actions.New(evt_on_dbl_click_id)),
      OnEdit(Actions.New(evt_change_id)),
      Data(EmptyString),
      OnItemActivateStr(EmptyString),
      OnSelectStr(EmptyString)  {}
    virtual ~TTreeView()  {  ClearData();  }

    DefPropC(olxstr, Data)       // data associated with the object
    DefPropC(olxstr, OnItemActivateStr) // this is passed to the OnDoubleClick event
    DefPropC(olxstr, OnSelectStr) // this is passed to the OnSelect
    DefPropC(olxstr, OnEditStr) // this is passed to the OnEdit

    bool LoadFromStrings(const TStrList &strings);

    TActionQueue &OnDblClick, &OnSelect, &OnEdit;

    DECLARE_CLASS(TTreeView)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif

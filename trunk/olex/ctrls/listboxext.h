/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_listbox_H
#define __olx_ctrl_listbox_H
#include "olxctrlbase.h"
#include "estrlist.h"
#include "wx/listbox.h"
#include "wx/listctrl.h"

namespace ctrl_ext  {

  class TListBox: public wxListBox, public AOlxCtrl  {
    TActionQList *FActions;
    void ClickEvent(wxMouseEvent& event);
    void ItemSelectEvent(wxCommandEvent& event);
    olxstr Data, OnDblClickStr, OnSelectStr;
  public:
    TListBox(wxWindow *Parent) :
      wxListBox(Parent, -1), 
      AOlxCtrl(this),
      OnSelect(Actions.New(evt_on_select_id)),
      OnDblClick(Actions.New(evt_on_dbl_click_id)),
      Data(EmptyString()),
      OnDblClickStr(EmptyString()),
      OnSelectStr(EmptyString())  {}
    virtual ~TListBox() {}

    DefPropC(olxstr, Data)       // data associated with the object
    DefPropC(olxstr, OnDblClickStr) // this is passed to the OnDoubleClick event
    DefPropC(olxstr, OnSelectStr) // this is passed to the OnSelect

    void AddObject(const olxstr &Name, void *Data=NULL)  {
        wxListBox::Append(Name.u_str(), Data);
    }
    inline void Clear()  {  wxListBox::Clear();  }

    void AddItems(const TStrList &items);

    olxstr ItemsToString(const olxstr &separator);

    inline olxstr GetItem(int i)  {  return wxListBox::GetString(i);  }
    inline void *GetObject(int i)  {   return wxListBox::GetClientData(i);  }
    inline int Count() const {  return wxListBox::GetCount();  }

    olxstr GetValue() const  {
      int index = GetSelection();
      return (index == -1) ? EmptyString() : olxstr(wxListBox::GetString(index));
    }

    TActionQueue &OnDblClick, &OnSelect;

    DECLARE_CLASS(TListBox)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif

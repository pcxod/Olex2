/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_label_H
#define __olx_ctrl_label_H
#include "olxctrlbase.h"

namespace ctrl_ext  {
  class TLabel: public wxStaticText, public AOlxCtrl  {
    TActionQList *FActions;
    void ClickEvent(wxCommandEvent& event);
    olxstr Data, OnClickStr;
  public:
    TLabel(wxWindow *Parent, const olxstr &label) : 
      AOlxCtrl(this), 
      OnClick(Actions.New(evt_on_click_id)),
      wxStaticText(Parent, wxID_ANY, label.u_str()),
      Data(EmptyString()),
      OnClickStr(EmptyString())  {}
    virtual ~TLabel()  {}

    DefPropC(olxstr, Data)
    DefPropC(olxstr, OnClickStr)

    TActionQueue &OnClick;

    inline void SetCaption(const olxstr &T) {  SetLabel(T.u_str()); }
    inline olxstr GetCaption() const {  return GetLabel(); }

    DECLARE_CLASS(TLabel)
    DECLARE_EVENT_TABLE()
  };
}; // end namespace ctrl_ext
#endif

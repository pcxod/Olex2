/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ctrl_menu_H
#define __olx_ctrl_menu_H
#include "olxctrlbase.h"
#include "actions.h"
#include "wx/wx.h"

namespace ctrl_ext {

  const short mtSeparator = wxITEM_SEPARATOR,
    mtNormalItem = wxITEM_NORMAL,
    mtCheckItem = wxITEM_CHECK,
    mtRadioItem = wxITEM_RADIO;

  class TMenu: public wxMenu, public IEObject  {
  public:
    TMenu(const olxstr &Name=EmptyString()) : wxMenu(Name.u_str()) {}
    // the function creates a new clone of wxMenu
    TMenu *Clone() const {  return CopyMenu(*this); };
    // returns a new menu created with new
    static TMenu *CopyMenu(const wxMenu& menu);
    void Clear(); // empties the menu

    DECLARE_CLASS(TMenu)
  };

  class TMenuItem: public wxMenuItem,  public AActionHandler  {
    TActionQList Actions;
    TActionQueue *ActionQueue;
    olxstr DependMode, Command, OnModeChangeCmd;
    short DependentOn;
  public:
    TMenuItem(const short type, int id, TMenu* parent=NULL, const olxstr &Name=EmptyString()) :
      wxMenuItem(parent, id, Name.u_str(), wxString(), static_cast<wxItemKind>(type)),
      OnModeChange(Actions.New(evt_on_mode_change_id)),
      ActionQueue(NULL),
      DependentOn(0)  {  SetToDelete(false);  }
    virtual ~TMenuItem()  {}
    void SetActionQueue(TActionQueue& q, const olxstr& dependMode, short dependentOn);
    bool Execute(const IEObject *Sender, const IEObject *Data);
    // updates checked status
    void ValidateState(); 
    DefPropC(olxstr, Command)

    TActionQueue &OnModeChange;
    static const short ModeDependent  = 1,
    StateDependent = 2;
    DECLARE_CLASS(TMenuItem)
  };
};  // end namespace ctrl_ext
#endif

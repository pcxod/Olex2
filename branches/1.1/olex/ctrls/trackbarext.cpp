/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "trackbarext.h"
#include "frameext.h"
#include "olxvar.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TTrackBar, wxSlider)

BEGIN_EVENT_TABLE(TTrackBar, wxSlider)
  EVT_SCROLL(TTrackBar::ScrollEvent)
  EVT_LEFT_UP(TTrackBar::MouseUpEvent)
END_EVENT_TABLE()

void TTrackBar::ScrollEvent(wxScrollEvent& evt)  {
  if( this_Val == GetValue() )  return;
  this_Val = GetValue();
  StartEvtProcessing()
    OnChange.Execute((AOlxCtrl*)this, &GetOnChangeStr());
  EndEvtProcessing()
}
//..............................................................................
void TTrackBar::MouseUpEvent(wxMouseEvent& evt)  {
  evt.Skip();
  StartEvtProcessing()
    OnMouseUp.Execute((AOlxCtrl*)this, &GetOnMouseUpStr());
  EndEvtProcessing()
}

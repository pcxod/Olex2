/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "htmlext.h"
#include "imgcellext.h"
#include "widgetcellext.h"
#include "../mainform.h"
#include "../xglapp.h"
#include "wx/html/htmlcell.h"
#include "wx/html/m_templ.h"
#include "wxzipfs.h"

#ifdef _UNICODE
  #define _StrFormat_ wxT("%ls")
#else
  #define _StrFormat_ wxT("%s")
#endif

// helper tag
TAG_HANDLER_BEGIN(SWITCHINFOS, "SWITCHINFOS")
TAG_HANDLER_PROC(tag)  {
  THtml::SwitchSources.Push( THtml::SwitchSource );
  THtml::SwitchSource = tag.GetParam(wxT("SRC"));
  return true;
}
TAG_HANDLER_END(SWITCHINFOS)
// helper tag
TAG_HANDLER_BEGIN(SWITCHINFOE, "SWITCHINFOE")
TAG_HANDLER_PROC(tag)
{
  THtml::SwitchSource = THtml::SwitchSources.Pop();
  return true;
}
TAG_HANDLER_END(SWITCHINFOE)
// Z-ordered image map tag
TAG_HANDLER_BEGIN(RECT, "ZRECT")
TAG_HANDLER_PROC(tag)  {
  if( tag.HasParam(wxT("COORDS")) )  {
    if( m_WParser->GetContainer()->GetLastChild() != NULL && 
      EsdlInstanceOf(*m_WParser->GetContainer()->GetLastChild(), THtmlImageCell) )  
    {
      THtmlImageCell* ic = (THtmlImageCell*)m_WParser->GetContainer()->GetLastChild();
      TStrList toks(tag.GetParam(wxT("COORDS")), ',');
      if( toks.Count() == 4 )
        ic->AddRect(
          toks[0].ToInt(), 
          toks[1].ToInt(),
          toks[2].ToInt(),
          toks[3].ToInt(),
          tag.GetParam(wxT("HREF")),
          tag.GetParam(wxT("TARGET"))
        );
    }
  }
  return false;
}
TAG_HANDLER_END(RECT)
// Z-ordered image tag
TAG_HANDLER_BEGIN(CIRCLE, "ZCIRCLE")
TAG_HANDLER_PROC(tag)  {
  if( tag.HasParam(wxT("COORDS")) )  {
    if( m_WParser->GetContainer()->GetLastChild() != NULL && 
      EsdlInstanceOf(*m_WParser->GetContainer()->GetLastChild(), THtmlImageCell) )  
    {
      THtmlImageCell* ic = (THtmlImageCell*)m_WParser->GetContainer()->GetLastChild();
      TStrList toks(tag.GetParam(wxT("COORDS")), ',');
      if( toks.Count() == 3 )
        ic->AddCircle(
          toks[0].ToInt(), 
          toks[1].ToInt(),
          toks[2].ToFloat<float>(),
          tag.GetParam(wxT("HREF")),
          tag.GetParam(wxT("TARGET"))
        );
    }
  }
  return false;
}

TAG_HANDLER_END(CIRCLE)
// extended image tag
TAG_HANDLER_BEGIN(IMAGE, "ZIMG")
TAG_HANDLER_PROC(tag)  {
  int ax=-1, ay=-1;
  bool WidthInPercent = false, HeightInPercent = false;
  int fl = 0;
  wxString text = tag.GetParam(wxT("TEXT")),
           mapName = tag.GetParam(wxT("USEMAP"));
  olxstr ObjectName = tag.GetParam(wxT("NAME")),
    Tmp;
  try  {
    Tmp = tag.GetParam(wxT("WIDTH"));
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ax = (int)Tmp.SubStringTo(Tmp.Length()-1).ToDouble();
        WidthInPercent = true;
      }
      else
        ax = (int)Tmp.ToDouble();
    }
    Tmp = tag.GetParam(wxT("HEIGHT"));
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ay = (int)Tmp.SubStringTo(Tmp.Length()-1).ToDouble();
        HeightInPercent = true;
      }
      else
        ay = (int)Tmp.ToDouble();
    }
  }
  catch(const TExceptionBase& e)  {
    TBasicApp::NewLogEntry(logException) << e.GetException()->GetFullMessage();
    TBasicApp::NewLogEntry() << "While processing Width/Height for zimg::" << ObjectName;
    TBasicApp::NewLogEntry() << "Offending input: '" << Tmp << '\'';
  }

  if (tag.HasParam(wxT("FLOAT"))) fl = ax;

  if( !text.IsEmpty() )  {
    int textW = 0, textH = 0;
    m_WParser->GetDC()->GetTextExtent( text, &textW, &textH );
    if( textW > ax )  ax = textW;
    if( textH > ay )  ay = textH;
  }

  olxstr src = tag.GetParam(wxT("SRC"));

  TGlXApp::GetMainForm()->ProcessFunction(src, EmptyString(), true);

  if( TZipWrapper::IsZipFile(THtml::SwitchSource) && !TZipWrapper::IsZipFile(src) )
    src = TZipWrapper::ComposeFileName(THtml::SwitchSource, src);

  wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( src );
  if( fsFile == NULL )
    TBasicApp::NewLogEntry(logError) << "Could not locate image: " << src;

  if( (mapName.Length() > 0) && mapName.GetChar(0) == '#')
      mapName = mapName.Mid( 1 );

  THtmlImageCell *cell = new THtmlImageCell( m_WParser->GetWindowInterface()->GetHTMLWindow(),
                                           fsFile,
                                           ax, ay,
                                           m_WParser->GetPixelScale(),
                                           wxHTML_ALIGN_BOTTOM,
                                           mapName,
                                           WidthInPercent,
                                           HeightInPercent
                                           );

  cell->SetText( text );
  cell->SetSource( src );
  cell->SetLink(m_WParser->GetLink());
  cell->SetId(tag.GetParam(wxT("id"))); // may be empty
  m_WParser->GetContainer()->InsertCell(cell);
  if( !ObjectName.IsEmpty() )  {
    if( !TGlXApp::GetMainForm()->GetHtml()->AddObject(ObjectName, cell, NULL) )
      TBasicApp::NewLogEntry(logError) << "THTML: object already exist: " << ObjectName;
  }
  return false;
}
TAG_HANDLER_END(IMAGE)
// helpe function
olxstr ExpandMacroShortcuts(const olxstr &s,
  const olxdict<olxstr,olxstr, olxstrComparator<false> > &map)
{
  olxstr rv = s;
  for( size_t i=0; i < map.Count(); i++ )
    rv.Replace(map.GetKey(i), map.GetValue(i));
  return rv;
}
// input tag
TAG_HANDLER_BEGIN(INPUT, "INPUT")
TAG_HANDLER_PROC(tag)  {
  const olxstr TagName = tag.GetParam(wxT("TYPE"));
  olxstr ObjectName, Value, Data, Tmp, Label;
  ObjectName = tag.GetParam(wxT("NAME"));
  const olxstr SrcInfo = olxstr(__OlxSrcInfo) << " for input '" << TagName << '[' << ObjectName << "]'";
  int valign = -1, halign = -1, 
    fl=0,
    ax=100, ay=20;
  bool width_set = false, height_set = false;
  AOlxCtrl* CreatedObject = NULL;
  wxWindow* CreatedWindow = NULL;
  olxdict<olxstr,olxstr, olxstrComparator<false> > macro_map;
  // init the shortcuts
  {
    olxstr pn = TGlXApp::GetMainForm()->GetHtml()->GetPopupName();
    macro_map.Add("~name~", pn.IsEmpty() ? ObjectName : (pn << '.' << ObjectName));
  }
  try  {
    Tmp = tag.GetParam(wxT("WIDTH"));
    TGlXApp::GetMainForm()->ProcessFunction(Tmp, SrcInfo, false);
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )
        fl = Tmp.SubStringTo(Tmp.Length()-1).ToFloat<float>();
      else
        ax = (int)Tmp.ToDouble();
      width_set = true;
    }
    Tmp = tag.GetParam(wxT("HEIGHT"));
    TGlXApp::GetMainForm()->ProcessFunction(Tmp, SrcInfo, false);
    if( !Tmp.IsEmpty() )  {
      if( Tmp.EndsWith('%') )  {
        ay = 0;
        float _ay = Tmp.SubStringTo(Tmp.Length()-1).ToFloat<float>()/100;
        _ay *= m_WParser->GetWindowInterface()->GetHTMLWindow()->GetSize().GetHeight();
        ay = (int)_ay;
      }
      else
        ay = Tmp.ToDouble();
      height_set = true;
    }
  }
  catch(const TExceptionBase& e)  {
    TBasicApp::NewLogEntry(logException) << e.GetException()->GetFullMessage();
    TBasicApp::NewLogEntry() << "While processing Width/Height HTML tags for " <<
      TagName << "::" << ObjectName;
    TBasicApp::NewLogEntry() << "Offending input: '" << Tmp << '\'';
  }
  if( ax == 0 )  ax = 30;
  if( ay == 0 )  ay = 20;
 
  if( tag.HasParam(wxT("FLOAT")) )  
    fl = ax;

  {  // parse h alignment
    wxString ha;
    if( tag.HasParam(wxT("ALIGN")) )
      ha = tag.GetParam(wxT("ALIGN"));
    else if( tag.HasParam(wxT("HALIGN")) )
      ha = tag.GetParam(wxT("HALIGN"));
    if( !ha.IsEmpty() )  {
      if( ha.CmpNoCase(wxT("left")) == 0 )
        halign = wxHTML_ALIGN_LEFT;
      else if( ha.CmpNoCase(wxT("center")) == 0 || ha.CmpNoCase(wxT("middle")) == 0 )
        halign = wxHTML_ALIGN_CENTER;
      else if( ha.CmpNoCase(wxT("right")) == 0 )
        halign = wxHTML_ALIGN_RIGHT;
    }
  }
  if( tag.HasParam(wxT("VALIGN")) ){
    wxString va = tag.GetParam(wxT("VALIGN"));
    if( va.CmpNoCase(wxT("top")) == 0 )
      valign = wxHTML_ALIGN_TOP;
    else if( va.CmpNoCase(wxT("center")) == 0 || va.CmpNoCase(wxT("middle")) == 0 )
      valign = wxHTML_ALIGN_CENTER;
    else if( va.CmpNoCase(wxT("bottom")) == 0 )
      valign = wxHTML_ALIGN_BOTTOM;
  }
  Label = tag.GetParam(wxT("LABEL"));

  wxHtmlLinkInfo* LinkInfo = NULL;
  if( !Label.IsEmpty() )  {
    if( Label.StartsFromi("href=") )  {
      Label = Label.SubStringFrom(5);
      const size_t tagInd = Label.IndexOfi("&target=");
      olxstr tag(EmptyString());
      if( tagInd != InvalidIndex )  {
        tag = Label.SubStringFrom(tagInd+8);
        Label.SetLength(tagInd);
      }
      LinkInfo = new wxHtmlLinkInfo(Label.u_str(), tag.u_str() );
    }
  }
  if( !ObjectName.IsEmpty() )  {
    wxWindow* wnd = TGlXApp::GetMainForm()->GetHtml()->FindObjectWindow(ObjectName);
    if( wnd != NULL )  {
      if( !tag.HasParam(wxT("reuse")) )
        TBasicApp::NewLogEntry(logError) << "HTML: duplicated object \'" << ObjectName << '\'';
      else  {
        if( !Label.IsEmpty() )  {
          wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
          THtml::WordCell* wc = new THtml::WordCell( Label.u_str(), *m_WParser->GetDC());
          if( LinkInfo != NULL ) {  
            wc->SetLink(*LinkInfo);
            delete LinkInfo;
          }
          wc->SetDescent(0);
          contC->InsertCell( wc );
          contC->InsertCell(new wxHtmlWidgetCell(wnd, fl));
          if( valign != -1 )  contC->SetAlignVer(valign);
          if( halign != -1 )  contC->SetAlignHor(halign);
        }
        else
          m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(wnd, fl));
      }
      return false;
    }
  }

  Value = tag.GetParam(wxT("VALUE"));
  TGlXApp::GetMainForm()->ProcessFunction(Value, SrcInfo, true);
  Data = tag.GetParam(wxT("DATA"));
/******************* TEXT CONTROL *********************************************/
  if( TagName.Equalsi("text") )  {
    int flags = wxWANTS_CHARS;
    if( tag.HasParam( wxT("MULTILINE") ) )  flags |= wxTE_MULTILINE;
    if( tag.HasParam( wxT("PASSWORD") ) )   flags |= wxTE_PASSWORD;
    TTextEdit *Text = new TTextEdit(m_WParser->GetWindowInterface()->GetHTMLWindow(), flags);
    Text->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->SetSize(ax, ay);
    Text->SetData( Data );

    Text->SetText(Value);
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Text, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Text, fl));

    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Text->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      Text->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONLEAVE")) )  {
      Text->OnLeave.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONLEAVE")), macro_map);
      Text->OnLeave.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONENTER")) )  {
      Text->OnEnter.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONENTER")), macro_map);
      Text->OnEnter.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONRETURN")) )  {
      Text->OnReturn.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONRETURN")), macro_map);
      Text->OnReturn.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
  }
/******************* DATE CONTROL *****************************************/
  if( TagName.Equalsi("date") )  {
    int flags = wxDP_SPIN;
    if( tag.HasParam(wxT("dropdown")) )  {
      flags = wxDP_DROPDOWN;
    }
    TDateCtrl *DT = new TDateCtrl(m_WParser->GetWindowInterface()->GetHTMLWindow(), flags);
    DT->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = DT;
    CreatedWindow = DT;
    DT->SetSize(ax, ay);
    DT->SetData(Data);
    wxDateTime dt;
    dt.ParseDate(Value.u_str());
    if( dt.IsValid() )
      DT->SetValue(dt);
    else {
      TBasicApp::NewLogEntry(logError) << (
        olxstr("Invalid format for date and time: ").quote() << Value);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(DT, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(DT, fl));

    if( tag.HasParam(wxT("ONCHANGE")) )  {
      DT->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      DT->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
  }
/******************* COLOR CONTROL *******************************************/
  if( TagName.Equalsi("color") )  {
    TColorCtrl *CC = new TColorCtrl(m_WParser->GetWindowInterface()->GetHTMLWindow());
    CC->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = CC;
    CreatedWindow = CC;
    CC->SetSize(ax, ay);
    CC->SetData(Data);
    CC->SetValue(wxColor(Value.u_str()));
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(CC, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(CC, fl));

    if( tag.HasParam(wxT("ONCHANGE")) )  {
      CC->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      CC->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
  }
/******************* LABEL ***************************************************/
  else if( TagName.Equalsi("label") )  {
    TLabel *Text = new TLabel(m_WParser->GetWindowInterface()->GetHTMLWindow(), Value);
    Text->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->WI.SetWidth(ax);
    Text->WI.SetHeight(ay);
    Text->SetData(Data);
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Text, fl));
  }
/******************* BUTTON ***************************************************/
  else if( TagName.Equalsi("button") )  {
    AButtonBase *Btn;
    long flags = 0;
    if( tag.HasParam(wxT("FIT")) )  flags |= wxBU_EXACTFIT;
    if( tag.HasParam(wxT("FLAT")) )  flags |= wxNO_BORDER;
    olxstr buttonImage = tag.GetParam(wxT("IMAGE"));
    if( !buttonImage.IsEmpty() )  {
      if( buttonImage.IndexOf(',') != InvalidIndex )  {
        TImgButton* ibtn = new TImgButton(m_WParser->GetWindowInterface()->GetHTMLWindow());
        ibtn->SetImages(buttonImage, width_set ? ax : -1, height_set ? ay : -1);
        if( tag.HasParam(wxT("ENABLED")) )
          ibtn->SetEnabled(olxstr(tag.GetParam(wxT("ENABLED"))).ToBool());
        if( tag.HasParam(wxT("DOWN")) )
          ibtn->SetDown(olxstr(tag.GetParam(wxT("DOWN"))).ToBool());
        CreatedWindow = ibtn;
        Btn = ibtn;
      }
      else  {
        Btn = new TBmpButton(  m_WParser->GetWindowInterface()->GetHTMLWindow(), -1, wxNullBitmap, 
          wxDefaultPosition, wxDefaultSize, flags );
        ((TBmpButton*)Btn)->SetSource( buttonImage );
        wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( buttonImage );
        if( fsFile == NULL )
          TBasicApp::NewLogEntry(logError) << "THTML: could not locate image for button: " << ObjectName;
        else  {
          wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
          if ( !image.Ok() )
            TBasicApp::NewLogEntry(logError) << "THTML: could not load image for button: " << ObjectName;
          else  {
            if( (image.GetWidth() > ax || image.GetHeight() > ay) && tag.HasParam(wxT("STRETCH")) )
              image = image.Scale(ax, ay);
            else  {
              ax = image.GetWidth();
              ay = image.GetHeight();
            }
            ((TBmpButton*)Btn)->SetBitmapLabel( image );
          }
          delete fsFile;
        }
        Btn->WI.SetWidth(ax);
        Btn->WI.SetHeight(ay);
        ((TBmpButton*)Btn)->SetFont( m_WParser->GetDC()->GetFont() );
        CreatedWindow = (TBmpButton*)Btn;
      }
    }
    else  {
      Btn = new TButton(m_WParser->GetWindowInterface()->GetHTMLWindow(), -1, wxEmptyString, 
        wxDefaultPosition, wxDefaultSize, flags);
      ((TButton*)Btn)->SetCaption(Value);
      ((TButton*)Btn)->SetFont( m_WParser->GetDC()->GetFont() );
      if( (flags & wxBU_EXACTFIT) == 0 )  {
        Btn->WI.SetWidth(ax);
        Btn->WI.SetHeight(ay);
      }
#ifdef __WXGTK__  // got no idea what happens here, client size does not work?
      wxFont fnt(m_WParser->GetDC()->GetFont());
      fnt.SetPointSize( fnt.GetPointSize()-2);
      ((TButton*)Btn)->SetFont( fnt );
      wxCoord w=0, h=0, desc=0, exlead=0;
      wxString wxs(Value.u_str());
      m_WParser->GetDC()->GetTextExtent(wxs, &w, &h, &desc, &exlead, &fnt);
      int borderx = 12, bordery = 8;
      ((TButton*)Btn)->SetClientSize(w+borderx,h+desc+bordery);
#endif 
      CreatedWindow = (TButton*)Btn;
    }
    CreatedObject = Btn;
    Btn->SetData(Data);
    if( tag.HasParam(wxT("ONCLICK")) )  {
      Btn->OnClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCLICK")), macro_map);
      Btn->OnClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDOWN")) )  {
      Btn->OnUp.data = ExpandMacroShortcuts(tag.GetParam(wxT("ONUP")), macro_map);
      Btn->OnUp.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDOWN")) )  {
      Btn->OnDown.data = ExpandMacroShortcuts(tag.GetParam(wxT("ONDOWN")),macro_map);
      Btn->OnDown.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( TGlXApp::GetMainForm()->GetHtml()->GetShowTooltips() )  {
      Btn->SetHint(
        ExpandMacroShortcuts(tag.GetParam(wxT("HINT")),macro_map));
    }
    if( tag.HasParam(wxT("DOWN")) )
      Btn->SetDown(tag.GetParam(wxT("DOWN")).CmpNoCase(wxT("true")) == 0);

    olxstr modeDependent = tag.GetParam(wxT("MODEDEPENDENT"));
    if( !modeDependent.IsEmpty() )
      Btn->SetActionQueue(TGlXApp::GetMainForm()->OnModeChange, modeDependent);
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(CreatedWindow, fl));
  }
/******************* COMBOBOX *************************************************/
  else if( TagName.Equalsi("combo") )  {
    TComboBox *Box = new TComboBox(m_WParser->GetWindowInterface()->GetHTMLWindow(),
      tag.HasParam(wxT("READONLY")),
      wxSize(ax, ay));
    Box->SetFont(m_WParser->GetDC()->GetFont());

    CreatedObject = Box;
    CreatedWindow = Box;
    Box->WI.SetWidth(ax);
#ifdef __MAC__    
    Box->WI.SetHeight( olx_max(ay, Box->GetCharHeight()+10) );
#else
    Box->WI.SetHeight( ay );
#endif    
    if( tag.HasParam(wxT("ITEMS")) )  {
      olxstr Items = tag.GetParam(wxT("ITEMS"));
      TGlXApp::GetMainForm()->ProcessFunction(Items, SrcInfo, true);
      TStrList SL(Items, ';');
      if( SL.IsEmpty() )
        Box->AddObject(EmptyString());  // fix the bug in wxWidgets (if Up pressed, crass occurs)
      else
        Box->AddItems(SL);
    }
    else  {  // need to intialise the items - or wxWidgets will crash (pressing Up button)
      Box->AddObject(Value);
      Box->AddObject(EmptyString());  // fix the bug in wxWidgets (if Up pressed, crass occurs)
    }
    Box->SetText(Value);
    Box->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Box->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      Box->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONLEAVE")) )  {
      Box->OnLeave.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONLEAVE")), macro_map);
      Box->OnLeave.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONENTER")) )  {
      Box->OnEnter.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONENTER")), macro_map);
      Box->OnEnter.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONRETURN")) )  {
      Box->OnReturn.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONRETURN")), macro_map);
      Box->OnReturn.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell(wc);
      contC->InsertCell(new wxHtmlWidgetCell(Box, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Box, fl));
  }
/******************* SPIN CONTROL *********************************************/
  else if( TagName.Equalsi("spin") )  {
    TSpinCtrl *Spin = new TSpinCtrl(m_WParser->GetWindowInterface()->GetHTMLWindow());
    Spin->SetFont(m_WParser->GetDC()->GetFont());
    Spin->SetForegroundColour(m_WParser->GetDC()->GetTextForeground());
    int min=0, max = 100;
    if( tag.HasParam( wxT("MIN") ) )
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if( tag.HasParam( wxT("MAX") ) )
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    Spin->SetRange(min, max);
    try  {  Spin->SetValue((int)Value.ToDouble());  }
    catch(...)  {
      TBasicApp::NewLogEntry() << "Invalid value for spin control: \'" << Value << '\'';
    }
    CreatedObject = Spin;
    CreatedWindow = Spin;
    Spin->WI.SetHeight(ay);
    Spin->WI.SetWidth(ax);
    Spin->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Spin->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      Spin->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell(wc);
      contC->InsertCell(new wxHtmlWidgetCell(Spin, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Spin, fl));
  }
/******************* SLIDER ***************************************************/
  else  if( TagName.Equalsi("slider") )  {
    TTrackBar *Track = new TTrackBar(m_WParser->GetWindowInterface()->GetHTMLWindow());
    Track->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = Track;
    CreatedWindow = Track;
    int min=0, max = 100;
    if( tag.HasParam( wxT("MIN") ) )
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if( tag.HasParam( wxT("MAX") ) )
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    Track->WI.SetWidth(ax);
    Track->WI.SetHeight(ay);
    if( min < max )
      Track->SetRange(min, max);
    try  {  Track->SetValue((int)Value.ToDouble());  }
    catch(...)  {
      TBasicApp::NewLogEntry() << "Invalid value slider: \'" << Value << '\'';
    }
    Track->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Track->OnChange.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHANGE")), macro_map);
      Track->OnChange.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONMOUSEUP")) )  {
      Track->OnMouseUp.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONMOUSEUP")), macro_map);
      Track->OnMouseUp.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtml::WordCell* wc = new THtml::WordCell(Label.u_str(), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell(wc);
      contC->InsertCell(new wxHtmlWidgetCell(Track, fl));
      if( valign != -1 )  contC->SetAlignVer(valign);
      if( halign != -1 )  contC->SetAlignHor(halign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Track, fl));
  }
/******************* CHECKBOX *************************************************/
  else if( TagName.Equalsi("checkbox") )  {
    TCheckBox *Box = new TCheckBox( 
      m_WParser->GetWindowInterface()->GetHTMLWindow(), tag.HasParam(wxT("RIGHT")) ? wxALIGN_RIGHT : 0);
    Box->SetFont(m_WParser->GetDC()->GetFont());
    wxLayoutConstraints* wxa = new wxLayoutConstraints;
    wxa->centreX.Absolute(0);
    Box->SetConstraints(wxa);
    CreatedObject = Box;
    CreatedWindow = Box;
    Box->WI.SetWidth(ax);
    Box->WI.SetHeight(ay);
    Box->SetCaption(Value);
    if( tag.HasParam(wxT("CHECKED")) )  {
      Tmp = tag.GetParam(wxT("CHECKED"));
      if( Tmp.IsEmpty() )
        Box->SetChecked(true);
      else if( Tmp.IsBool() )
        Box->SetChecked(Tmp.ToBool());
      else  {
        TBasicApp::NewLogEntry(logError) << 
          (olxstr("Invalid value for boolean: ").quote() << Tmp);
      }
    }
    Box->SetData(Data);
    // binding events
    if( tag.HasParam(wxT("ONCLICK")) )  {
      Box->OnClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCLICK")), macro_map);
      Box->OnClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONCHECK")) )  {
      Box->OnCheck.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONCHECK")), macro_map);
      Box->OnCheck.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONUNCHECK")) )  {
      Box->OnUncheck.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONUNCHECK")), macro_map);
      Box->OnUncheck.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("MODEDEPENDENT")) )
      Box->SetActionQueue(TGlXApp::GetMainForm()->OnModeChange, tag.GetParam(wxT("MODEDEPENDENT")));
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Box, fl));
  }
/******************* TREE CONTROL *********************************************/
  else if( TagName.Equalsi("tree") )  {
    long flags = wxTR_HAS_BUTTONS;
    if( tag.HasParam(wxT("NOROOT")) )
      flags |= wxTR_HIDE_ROOT;
    if( tag.HasParam(wxT("EDITABLE")) )
      flags |= wxTR_EDIT_LABELS;

    TTreeView *Tree = new TTreeView(m_WParser->GetWindowInterface()->GetHTMLWindow(), flags);

    if( (flags&wxTR_HIDE_ROOT) == 0 && tag.HasParam(wxT("ROOTLABEL")) )
      Tree->SetItemText(Tree->GetRootItem(), tag.GetParam(wxT("ROOTLABEL")));
    olxstr src = tag.GetParam(wxT("SRC"));
    TGlXApp::GetMainForm()->ProcessFunction(src, SrcInfo, true);
    IInputStream* ios = TFileHandlerManager::GetInputStream(src);
    Tree->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = Tree;
    CreatedWindow = Tree;
    Tree->WI.SetWidth(ax);
    Tree->WI.SetHeight(ay);
    wxMenu* menu = new wxMenu;
    menu->Append(1000, wxT("Expand all"));
    menu->Append(1001, wxT("Collapse all"));
    Tree->SetPopup(menu);
    Tree->SetData( Data );
    if( tag.HasParam(wxT("ONSELECT")) )  {
      Tree->OnSelect.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONSELECT")), macro_map);
      Tree->OnSelect.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONITEM")) )  {
      Tree->OnDblClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONITEM")), macro_map);
      Tree->OnDblClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( (flags&wxTR_EDIT_LABELS) != 0 && tag.HasParam(wxT("ONEDIT")) )  {
      Tree->OnEdit.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONEDIT")), macro_map);
      Tree->OnEdit.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Tree, fl));
    if( ios == NULL )  {  // create test tree
      TBasicApp::NewLogEntry(logError) << "THTML: could not locate tree source: \'" << src <<  '\'';
      wxTreeItemId Root = Tree->AddRoot( wxT("Test data") );
      wxTreeItemId sc1 = Tree->AppendItem(Tree->AppendItem(Root, wxT("child")), wxT("subchild"));
         Tree->AppendItem(Tree->AppendItem(sc1, wxT("child1")), wxT("subchild1"));
      wxTreeItemId sc2 = Tree->AppendItem( Tree->AppendItem(Root, wxT("child1")), wxT("subchild1"));
        sc2 = Tree->AppendItem(Tree->AppendItem(sc2, wxT("child1")), wxT("subchild1"));
    }
    else  {
      TStrList list;
#ifdef _UNICODE
      TUtf8File::ReadLines(*ios, list, false);
#else
      list.LoadFromTextStream(*ios);
#endif
      Tree->LoadFromStrings(list);
      delete ios;
      wxString sel = tag.GetParam(wxT("SELECTED"));
      if( sel.IsEmpty() )  {
        sel = tag.GetParam(wxT("VALUE"));
        if( !sel.IsEmpty() )
          Tree->SelectByLabel(sel);
      }
      else
          Tree->SelectByData(sel);
    }
  }
/******************* LIST CONTROL *********************************************/
  else if( TagName.Equalsi("list") )  {
    bool srcTag   = tag.HasParam(wxT("SRC")),
         itemsTag = tag.HasParam(wxT("ITEMS"));
    TStrList itemsList;
    if( srcTag && itemsTag )
      TBasicApp::NewLogEntry(logError) << "THTML: list can have only src or items";
    else if( srcTag ) {
      olxstr src = tag.GetParam(wxT("SRC"));
      TGlXApp::GetMainForm()->ProcessFunction(src, SrcInfo, true);
      IInputStream* ios = TFileHandlerManager::GetInputStream(src);
      if( ios == NULL )
        TBasicApp::NewLogEntry(logError) << "THTML: could not locate list source: \'" << src <<  '\'';
      else  {
#ifdef _UNICODE
      TUtf8File::ReadLines(*ios, itemsList, false);
#else
        itemsList.LoadFromTextStream( *ios );
#endif
        delete ios;
      }
    }
    else if( itemsTag )  {
      olxstr items = tag.GetParam(wxT("ITEMS"));
      TGlXApp::GetMainForm()->ProcessFunction(items, SrcInfo, true);
      itemsList.Strtok(items, ';');
    }
    TListBox *List = new TListBox(m_WParser->GetWindowInterface()->GetHTMLWindow());
    List->SetFont(m_WParser->GetDC()->GetFont());
    CreatedObject = List;
    CreatedWindow = List;
    List->WI.SetWidth(ax);
    List->WI.SetHeight(ay);
    List->SetData(Data);
    List->AddItems(itemsList);
    // binding events
    if( tag.HasParam(wxT("ONSELECT")) )  {
      List->OnSelect.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONSELECT")), macro_map);
      List->OnSelect.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDBLCLICK")) )  {
      List->OnDblClick.data =
        ExpandMacroShortcuts(tag.GetParam(wxT("ONDBLCLICK")), macro_map);
      List->OnDblClick.Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    // creating cell
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(List, fl));
  }
/******************* END OF CONTROLS ******************************************/
  if( LinkInfo != NULL )  delete LinkInfo;
  if( ObjectName.IsEmpty() )  {  }  // create random name?
  if( CreatedWindow != NULL )  {  // set default colors
#ifdef __WIN32__
    if( EsdlInstanceOf(*CreatedWindow, TComboBox) )  {
      TComboBox* Box = (TComboBox*)CreatedWindow;
      if( Box->GetTextCtrl() != NULL )  {
        if( m_WParser->GetContainer()->GetBackgroundColour().IsOk() )
          Box->GetTextCtrl()->SetBackgroundColour( m_WParser->GetContainer()->GetBackgroundColour() );
        if( m_WParser->GetActualColor().IsOk() )
          Box->GetTextCtrl()->SetForegroundColour( m_WParser->GetActualColor() );
      }
      if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL )  {
        if( m_WParser->GetContainer()->GetBackgroundColour().IsOk() )
          Box->GetPopupControl()->GetControl()->SetBackgroundColour( 
            m_WParser->GetContainer()->GetBackgroundColour() );
        if( m_WParser->GetActualColor().IsOk() )  {
          Box->GetPopupControl()->GetControl()->SetForegroundColour( 
            m_WParser->GetActualColor() );
        }
      }
    }
#endif
    if( m_WParser->GetActualColor().IsOk() )
      CreatedWindow->SetForegroundColour(m_WParser->GetActualColor());
    if( m_WParser->GetContainer()->GetBackgroundColour().IsOk() )
      CreatedWindow->SetBackgroundColour(m_WParser->GetContainer()->GetBackgroundColour());
  }
  if( CreatedObject != NULL )  {
    if( !TGlXApp::GetMainForm()->GetHtml()->AddObject(
      ObjectName, CreatedObject, CreatedWindow, tag.HasParam(wxT("MANAGE")) ) )
    {
      TBasicApp::NewLogEntry(logError) << "HTML: duplicated object \'" << ObjectName << '\'';
    }
    if( CreatedWindow != NULL && !ObjectName.IsEmpty() )  {
      CreatedWindow->Hide();
      olxstr bgc, fgc;
      if( tag.HasParam(wxT("BGCOLOR")) )  {
        bgc = ExpandMacroShortcuts(tag.GetParam(wxT("BGCOLOR")), macro_map);
        TGlXApp::GetMainForm()->ProcessFunction(bgc, SrcInfo, false);
      }
      if( tag.HasParam(wxT("FGCOLOR")) )  {
        fgc = ExpandMacroShortcuts(tag.GetParam(wxT("FGCOLOR")), macro_map);
        TGlXApp::GetMainForm()->ProcessFunction(fgc, SrcInfo, false);
      }

      if( EsdlInstanceOf(*CreatedWindow, TComboBox) )  {
        TComboBox* Box = (TComboBox*)CreatedWindow;
        if( !bgc.IsEmpty() )  {
          wxColor bgCl = wxColor(bgc.u_str());
#ifdef __WIN32__          
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetBackgroundColour(bgCl);
          if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL )
            Box->GetPopupControl()->GetControl()->SetBackgroundColour(bgCl);
#endif
        }
        if( !fgc.IsEmpty() )  {
          wxColor fgCl = wxColor(bgc.u_str());
#ifdef __WIN32__          
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetForegroundColour( fgCl );
          if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL)
            Box->GetPopupControl()->GetControl()->SetForegroundColour(fgCl);
#endif
        }
      }
      if( !bgc.IsEmpty() )
        CreatedWindow->SetBackgroundColour(wxColor(bgc.u_str()));
      if( !fgc.IsEmpty() )
        CreatedWindow->SetForegroundColour(wxColor(fgc.u_str()));
    }
  }
  return false;
}

TAG_HANDLER_END(INPUT)

TAGS_MODULE_BEGIN(Input)
    TAGS_MODULE_ADD(INPUT)
    TAGS_MODULE_ADD(IMAGE)
    TAGS_MODULE_ADD(RECT)
    TAGS_MODULE_ADD(CIRCLE)
    TAGS_MODULE_ADD(SWITCHINFOS)
    TAGS_MODULE_ADD(SWITCHINFOE)
TAGS_MODULE_END(Input)

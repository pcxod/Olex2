//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "htmlext.h"
#include "wx/html/htmlcell.h"
#include "wx/html/m_templ.h"


#include "estack.h"
#include "fsext.h"
#include "ctrls.h"
#include "mainform.h"

#include "wx/tooltip.h"
#include "wx/artprov.h"

#include "xglapp.h"
#include "obase.h"
#include "utf8file.h"
#include "wxzipfs.h"

#define this_InitFunc(funcName, argc) \
  (*THtml::Library).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc));\
  (LC->GetLibrary()).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc))
#define this_InitFuncD(funcName, argc, desc) \
  (*THtml::Library).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc, desc));\
  (LC->GetLibrary()).RegisterFunction( new TFunction<THtml>(this, &THtml::fun##funcName, #funcName, argc, desc))
/*
#define this_InitMacro(macroName, validOptions, argc)\
  (*THtml::Library).RegisterMacro( new TMacro<THtml>(this, &THtml::mac##macroName, #macroName, #validOptions, argc));\
  (LC->GetLibrary()).RegisterMacro( new TMacro<THtml>(this, &THtml::mac##macroName, #(html##macroName), #validOptions, argc))
*/

TLibrary* THtml::Library = NULL;
/*____________________________________________________________________________*/
// unfortunately the global variables are required ....
    olxstr SwitchSource;
    TEStack<olxstr> SwitchSources;
/*____________________________________________________________________________*/

//..............................................................................
//..............................................................................
//----------------------------------------------------------------------------//
// implementation of the input tag
//----------------------------------------------------------------------------//

#ifdef _UNICODE
  #define _StrFormat_ wxT("%ls")
#else
  #define _StrFormat_ wxT("%s")
#endif


TAG_HANDLER_BEGIN(SWITCHINFOS, "SWITCHINFOS")
TAG_HANDLER_PROC(tag)  {
  SwitchSources.Push( SwitchSource );
  SwitchSource = tag.GetParam(wxT("SRC")).c_str();
  return true;
}
TAG_HANDLER_END(SWITCHINFOS)

TAG_HANDLER_BEGIN(SWITCHINFOE, "SWITCHINFOE")
TAG_HANDLER_PROC(tag)
{
  SwitchSource = SwitchSources.Pop();
  return true;
}
TAG_HANDLER_END(SWITCHINFOE)

TAG_HANDLER_BEGIN(IMAGE, "ZIMG")
TAG_HANDLER_PROC(tag)  {
  int ax=-1, ay=-1;
  olxch cBf[40];
  olxstr Tmp;
  int fl = 0;
  cBf[0] = '\0';
  wxString text = tag.GetParam(wxT("TEXT")),
           mapName = tag.GetParam(wxT("USEMAP"));

  olxstr ObjectName = tag.GetParam(wxT("NAME")).c_str();

  tag.ScanParam(wxT("WIDTH"), _StrFormat_, cBf);
  Tmp = cBf;
  if( !Tmp.IsEmpty() )  {
    ax = Tmp.ToInt();
    if( Tmp.EndsWith('%') )  {
      float w = (float)ax/100;
      w *= m_WParser->GetWindowInterface()->GetHTMLWindow()->GetSize().GetWidth();
      ax = (int)w;
    }
  }
  cBf[0] = '\0';
  tag.ScanParam(wxT("HEIGHT"), _StrFormat_, cBf);
  Tmp = cBf;
  if( !Tmp.IsEmpty() )  {
    ay = Tmp.ToInt();
    if( Tmp.EndsWith('%') )  {
      float h = (float)ay/100;
      h *= m_WParser->GetWindowInterface()->GetHTMLWindow()->GetSize().GetHeight();
      ay = (int)h;
    }
  }
  if (tag.HasParam(wxT("FLOAT"))) fl = ax;

  if( text.Len() != 0 )  {
    int textW = 0, textH = 0;
    m_WParser->GetDC()->GetTextExtent( text, &textW, &textH );
    if( textW > ax )  ax = textW;
    if( textH > ay )  ay = textH;
  }
  olxstr src = tag.GetParam(wxT("SRC")).c_str();

  TGlXApp::GetMainForm()->ProcessMacroFunc( src );

  if( TZipWrapper::IsZipFile(SwitchSource) && !TZipWrapper::IsZipFile(src) )
    src = TZipWrapper::ComposeFileName(SwitchSource, src);

  wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( src );
  if( fsFile == NULL )
    TBasicApp::GetLog().Error( olxstr("Could not locate image: ") << src );

  if( (mapName.Length() > 0) && mapName.GetChar(0) == '#')
      mapName = mapName.Mid( 1 );

  THtmlImageCell *cell = new THtmlImageCell( m_WParser->GetWindowInterface()->GetHTMLWindow(),
                                           fsFile,
                                           ax, ay,
                                           m_WParser->GetPixelScale(),
                                           wxHTML_ALIGN_BOTTOM,
                                           mapName
                                           );

  cell->SetText( text );
  cell->SetSource( src );
  cell->SetLink(m_WParser->GetLink());
  cell->SetId(tag.GetParam(wxT("id"))); // may be empty
  m_WParser->GetContainer()->InsertCell(cell);
  if( !ObjectName.IsEmpty() )  {
    if( !TGlXApp::GetMainForm()->GetHtml()->AddObject(ObjectName, cell, NULL) )
      TBasicApp::GetLog().Error(olxstr("THTML: object already exist: ") << ObjectName);
  }
  return false;
}
TAG_HANDLER_END(IMAGE)

TAG_HANDLER_BEGIN(INPUT, "INPUT")

TAG_HANDLER_PROC(tag)  {
  olxch Bf[80];
  tag.ScanParam(wxT("TYPE"), _StrFormat_, Bf);
  olxstr TagName(Bf), ObjectName, Value, Data, strValign, Tmp, Label;

  int ax=100, ay=20;
  int fl = 0,
      valign = wxHTML_ALIGN_CENTER;
  IEObject* CreatedObject = NULL;
  wxWindow* CreatedWindow = NULL;
  Bf[0] = '\0';
  tag.ScanParam(wxT("WIDTH"), _StrFormat_, Bf);
  Tmp = Bf;
  if( !Tmp.IsEmpty() )  {
    if( Tmp.EndsWith('%') )  {
      Tmp.SetLength(Tmp.Length()-1);
      ax = Tmp.ToInt();
      float w = (float)ax/100;
      w *= m_WParser->GetWindowInterface()->GetHTMLWindow()->GetSize().GetWidth();
      ax = (int)w;
    }
    else
      ax = Tmp.ToInt();
  }
  Bf[0] = '\0';
  tag.ScanParam(wxT("HEIGHT"), _StrFormat_, Bf);
  Tmp = Bf;
  if( !Tmp.IsEmpty() )  {
    if( Tmp.EndsWith('%') )  {
      Tmp.SetLength(Tmp.Length()-1);
      ay = Tmp.ToInt();
      float h = (float)ay/100;
      h *= m_WParser->GetWindowInterface()->GetHTMLWindow()->GetSize().GetHeight();
      ay = (int)h;
    }
    else
      ay = Tmp.ToInt();
  }

  if( ax == 0 )  ax = 30;
  if( ay == 0 )  ay = 20;
  if( tag.HasParam(wxT("FLOAT")) ) fl = ax;

  ObjectName = tag.GetParam(wxT("NAME")).c_str();
  Value = tag.GetParam(wxT("VALUE")).c_str();
  TGlXApp::GetMainForm()->ProcessMacroFunc( Value );
  Data = tag.GetParam(wxT("DATA")).c_str();
  Label = tag.GetParam(wxT("LABEL")).c_str();
  wxHtmlLinkInfo* LinkInfo = NULL;
  if( !Label.IsEmpty() )  {
    if( Label.StartsFromi("href=") )  {
      Label = Label.SubStringFrom(5);
      int tagInd = Label.IndexOfi("&target=");
      olxstr tag(EmptyString);
      if( tagInd != -1 )  {
        tag = Label.SubStringFrom(tagInd+8);
        Label.SetLength(tagInd);
      }
      LinkInfo = new wxHtmlLinkInfo(Label.u_str(), tag.u_str() );
    }
  }

  strValign = tag.GetParam(wxT("VALIGN")).c_str();
  if( strValign.Comparei("top") == 0 )
    valign = wxHTML_ALIGN_TOP;
  else if( strValign.Comparei("center") == 0 )
    valign = wxHTML_ALIGN_CENTER;
  else if( strValign.Comparei("bottom") == 0 )
    valign = wxHTML_ALIGN_BOTTOM;

/******************* TEXT CONTROL *********************************************/
  if( !TagName.Comparei("text") )  {
    TTextEdit *Text = new TTextEdit(m_WParser->GetWindowInterface()->GetHTMLWindow(),
      (tag.HasParam(wxT("MULTILINE")) ? wxTE_MULTILINE : 0));
    Text->SetFont( m_WParser->GetDC()->GetFont() );
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->WI.SetWidth( ax );
    Text->WI.SetHeight( ay );
    Text->SetData( Data );

    Text->SetText(Value);
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtmlWordCell* wc = new THtmlWordCell( uiStr(Label), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Text, fl));
      contC->SetAlignVer(valign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Text, fl));

    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Text->SetOnChangeStr( tag.GetParam(wxT("ONCHANGE")).c_str() );
      Text->OnChange->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONLEAVE")) )  {
      Text->SetOnLeaveStr( tag.GetParam(wxT("ONLEAVE")).c_str() );
      Text->OnLeave->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONENTER")) )  {
      Text->SetOnEnterStr( tag.GetParam(wxT("ONENTER")).c_str() );
      Text->OnEnter->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
  }
/******************* LABEL ***************************************************/
  else if( !TagName.Comparei("label") )  {
    TLabel *Text = new TLabel(m_WParser->GetWindowInterface()->GetHTMLWindow());
    Text->SetFont( m_WParser->GetDC()->GetFont() );
    CreatedObject = Text;
    CreatedWindow = Text;
    Text->WI.SetWidth( ax );
    Text->WI.SetHeight( ay );
    Text->SetData( Data );

    Text->SetCaption(Value);
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Text, fl));
  }
/******************* BUTTON ***************************************************/
  else if( !TagName.Comparei("button") )  {
    AButtonBase *Btn;
    long flags = 0;
    if( tag.HasParam(wxT("FIT")) )  flags |= wxBU_EXACTFIT;
    if( tag.HasParam(wxT("FLAT")) )  flags |= wxNO_BORDER;
    olxstr buttonImage = tag.GetParam(wxT("IMAGE")).c_str();
    if( !buttonImage.IsEmpty() )  {
      Btn = new TBmpButton( m_WParser->GetWindowInterface()->GetHTMLWindow(), flags );
      ((TBmpButton*)Btn)->SetSource( buttonImage );
      wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( buttonImage );
      if( fsFile == NULL )
        TBasicApp::GetLog().Error(olxstr("THTML: could not locate image for button: ") << ObjectName);
      else  {
        wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
        if ( !image.Ok() )
          TBasicApp::GetLog().Error(olxstr("THTML: could not load image for button: ") << ObjectName);
        else  {
          if( (image.GetWidth() > ax || image.GetHeight() > ay) && tag.HasParam(wxT("STRETCH")) )
            image = image.Scale(ax, ay);
          else  {
            ax = image.GetWidth();
            ay = image.GetHeight();
          }
          ((TBmpButton*)Btn)->SetBitmapLabel( image );
        }
      }
      Btn->GetWI().SetWidth(ax);
      Btn->GetWI().SetHeight(ay);
      ((TBmpButton*)Btn)->SetFont( m_WParser->GetDC()->GetFont() );

      CreatedWindow = (TBmpButton*)Btn;
    }
    else  {
      Btn = new TButton( m_WParser->GetWindowInterface()->GetHTMLWindow(), flags );
      ((TButton*)Btn)->SetCaption(Value);
      ((TButton*)Btn)->SetFont( m_WParser->GetDC()->GetFont() );
      if( (flags & wxBU_EXACTFIT) == 0 )  {
        Btn->GetWI().SetWidth(ax);
        Btn->GetWI().SetHeight(ay);
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
      Btn->SetOnClickStr( tag.GetParam(wxT("ONCLICK")).c_str() );
      Btn->OnClick->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDOWN")) )  {
      Btn->SetOnUpStr( tag.GetParam(wxT("ONUP")).c_str() );
      Btn->OnUp->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDOWN")) )  {
      Btn->SetOnDownStr( tag.GetParam(wxT("ONDOWN")).c_str() );
      Btn->OnDown->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }

    if( tag.HasParam(wxT("DOWN")) )
      Btn->SetDown( tag.GetParam(wxT("DOWN")).CmpNoCase(wxT("true")) == 0 );

    olxstr modeDependent = tag.GetParam(wxT("MODEDEPENDENT")).c_str();
    if( !modeDependent.IsEmpty() )  {
      Btn->ActionQueue( TGlXApp::GetMainForm()->OnModeChange, modeDependent );
    }

    if( EsdlInstanceOf(*Btn, TButton) )
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell((TButton*)Btn, fl));
    else  if( EsdlInstanceOf(*Btn, TBmpButton) )
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell((TBmpButton*)Btn, fl));
  }
/******************* COMBOBOX *************************************************/
  else if( !TagName.Comparei("combo") )  {
    TComboBox *Box = new TComboBox( m_WParser->GetWindowInterface()->GetHTMLWindow(),
                                    tag.HasParam(wxT("READONLY")),
                                    wxSize(ax, ay) );
    Box->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = Box;
    CreatedWindow = Box;
    Box->WI.SetWidth(ax);
#ifdef __MAC__    
    Box->WI.SetHeight( olx_max(ay, Box->GetCharHeight()+10) );
#else
    Box->WI.SetHeight( ay );
#endif    
    Box->SetText(Value);
    if( tag.HasParam(wxT("ITEMS")) )  {
      olxstr Items = tag.GetParam(wxT("ITEMS")).c_str();
      TGlXApp::GetMainForm()->ProcessMacroFunc( Items );
      TStrList SL(Items, ';');
      if( SL.IsEmpty() )
        Box->AddObject(EmptyString);  // fix the bug in wxWidgets (if Up pressed, crass occurs)
      else
        Box->AddItems(SL);
    }
    else  {  // need to intialise the items - or wxWidgets will crash (pressing Up button)
      Box->AddObject(Value);
      Box->AddObject(EmptyString);  // fix the bug in wxWidgets (if Up pressed, crass occurs)
    }
    Box->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Box->SetOnChangeStr( tag.GetParam(wxT("ONCHANGE")).c_str() );
      Box->OnChange->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONLEAVE")) )  {
      Box->SetOnLeaveStr( tag.GetParam(wxT("ONLEAVE")).c_str() );
      Box->OnLeave->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONENTER")) )  {
      Box->SetOnEnterStr( tag.GetParam(wxT("ONENTER")).c_str() );
      Box->OnEnter->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtmlWordCell* wc = new THtmlWordCell( uiStr(Label), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Box, fl));
      contC->SetAlignVer(valign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Box, fl));
  }
/******************* SPIN CONTROL *********************************************/
  else if( !TagName.Comparei("spin") )  {
    TSpinCtrl *Spin = new TSpinCtrl( m_WParser->GetWindowInterface()->GetHTMLWindow() );
    Spin->SetFont( m_WParser->GetDC()->GetFont() );
    Spin->SetForegroundColour( m_WParser->GetDC()->GetTextForeground() );

    int min=0, max = 100;
    if( tag.HasParam( wxT("MIN") ) )
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if( tag.HasParam( wxT("MAX") ) )
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    Spin->SetRange(min, max);
    Spin->SetValue( Value.ToInt() );
    CreatedObject = Spin;
    CreatedWindow = Spin;
    Spin->WI.SetHeight(ay);
    Spin->WI.SetWidth(ax);

    Spin->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Spin->SetOnChangeStr( tag.GetParam(wxT("ONCHANGE")).c_str() );
      Spin->OnChange->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtmlWordCell* wc = new THtmlWordCell( uiStr(Label), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Spin, fl));
      contC->SetAlignVer(valign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Spin, fl));
  }
/******************* SLIDER ***************************************************/
  else  if( !TagName.Comparei("slider") )  {
    TTrackBar *Track = new TTrackBar( m_WParser->GetWindowInterface()->GetHTMLWindow() );
    Track->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = Track;
    CreatedWindow = Track;
    int min=0, max = 100;
    if( tag.HasParam( wxT("MIN") ) )
      tag.ScanParam(wxT("MIN"), wxT("%i"), &min);
    if( tag.HasParam( wxT("MAX") ) )
      tag.ScanParam(wxT("MAX"), wxT("%i"), &max);
    Track->WI.SetWidth(ax);
    Track->WI.SetHeight(ay);

    Track->SetRange(min, max);
    Track->SetValue( Value.ToInt() );

    Track->SetData(Data);
    if( tag.HasParam(wxT("ONCHANGE")) )  {
      Track->SetOnChangeStr(tag.GetParam(wxT("ONCHANGE")).c_str());
      Track->OnChange->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( !Label.IsEmpty() )  {
      wxHtmlContainerCell* contC = new wxHtmlContainerCell(m_WParser->GetContainer());
      THtmlWordCell* wc = new THtmlWordCell( uiStr(Label), *m_WParser->GetDC());
      if( LinkInfo != NULL ) wc->SetLink(*LinkInfo);
      wc->SetDescent(0);
      contC->InsertCell( wc );
      contC->InsertCell(new wxHtmlWidgetCell(Track, fl));
      contC->SetAlignVer(valign);
    }
    else
      m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Track, fl));
  }
/******************* CHECKBOX *************************************************/
  else if( !TagName.Comparei("checkbox") )  {
    TCheckBox *Box = new TCheckBox( m_WParser->GetWindowInterface()->GetHTMLWindow() );
    Box->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = Box;
    CreatedWindow = Box;
    Box->WI.SetWidth(ax);
    Box->WI.SetHeight(ay);

    Box->SetCaption(Value);
    if( tag.HasParam(wxT("CHECKED")) )  {
      Tmp = tag.GetParam(wxT("CHECKED")).c_str();
      if( Tmp.IsEmpty() )
        Box->SetChecked( true );
      else
        Box->SetChecked( Tmp.ToBool() );
    }

    Box->SetData(Data);
    // binding events
    if( tag.HasParam(wxT("ONCLICK")) )  {
      Box->SetOnClickStr( tag.GetParam(wxT("ONCLICK")).c_str() );
      Box->OnClick->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONCHECK")) )  {
      Box->SetOnCheckStr( tag.GetParam(wxT("ONCHECK")).c_str() );
      Box->OnCheck->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONUNCHECK")) )  {
      Box->SetOnUncheckStr( tag.GetParam(wxT("ONUNCHECK")).c_str() );
      Box->OnUncheck->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("MODEDEPENDENT")) )  {
      Box->ActionQueue( TGlXApp::GetMainForm()->OnModeChange, tag.GetParam(wxT("MODEDEPENDENT")).c_str() );
    }
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Box, fl));
  }
/******************* TREE CONTROL *********************************************/
  else if( !TagName.Comparei("tree") )  {
    olxstr src = tag.GetParam(wxT("SRC")).c_str();
    TGlXApp::GetMainForm()->ProcessMacroFunc( src );
    IInputStream* ios = TFileHandlerManager::GetInputStream( src );
    TTreeView *Tree = new TTreeView(m_WParser->GetWindowInterface()->GetHTMLWindow());
    Tree->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = Tree;
    CreatedWindow = Tree;
    Tree->WI.SetWidth(ax);
    Tree->WI.SetHeight(ay);

    Tree->SetData( Data );
    if( tag.HasParam(wxT("ONSELECT")) )  {
      Tree->SetOnSelectStr( tag.GetParam(wxT("ONSELECT")).c_str() );
      Tree->OnSelect->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONITEM")) )  {
      Tree->SetOnItemActivateStr( tag.GetParam(wxT("ONITEM")).c_str() );
      Tree->OnDblClick->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(Tree, fl));
    if( ios == NULL )  {  // create test tree
      TBasicApp::GetLog().Error(olxstr("THTML: could not locate tree source: \'") << src <<  '\'');
      wxTreeItemId Root = Tree->AddRoot( wxT("Test data") );
      wxTreeItemId sc1 = Tree->AppendItem( Tree->AppendItem(Root, wxT("child")), wxT("subchild"));
         Tree->AppendItem( Tree->AppendItem(sc1, wxT("child1")), wxT("subchild1"));
      wxTreeItemId sc2 = Tree->AppendItem( Tree->AppendItem(Root, wxT("child1")), wxT("subchild1"));
        sc2 = Tree->AppendItem( Tree->AppendItem(sc2, wxT("child1")), wxT("subchild1"));
    }
    else  {
      TStrList list;
      list.LoadFromTextStream(*ios);
      Tree->LoadFromStrings(list);
      delete ios;
    }
  }
/******************* LIST CONTROL *********************************************/
  else if( !TagName.Comparei("list") )  {
    bool srcTag   = tag.HasParam(wxT("SRC")),
         itemsTag = tag.HasParam(wxT("ITEMS"));
    TStrList itemsList;
    if( srcTag && itemsTag )
      TBasicApp::GetLog().Error( "THTML: list can have only src or items");
    else if( srcTag ) {
      olxstr src = tag.GetParam(wxT("SRC")).c_str();
      TGlXApp::GetMainForm()->ProcessMacroFunc( src );
      IInputStream* ios = TFileHandlerManager::GetInputStream( src );
      if( ios == NULL )
        TBasicApp::GetLog().Error(olxstr("THTML: could not locate list source: \'") << src <<  '\'');
      else  {
        itemsList.LoadFromTextStream( *ios );
        delete ios;
      }
    }
    else if( itemsTag )  {
      olxstr items = tag.GetParam(wxT("ITEMS")).c_str();
      TGlXApp::GetMainForm()->ProcessMacroFunc( items );
      itemsList.Strtok( items, ';');
    }
    TListBox *List = new TListBox( m_WParser->GetWindowInterface()->GetHTMLWindow() );
    List->SetFont( m_WParser->GetDC()->GetFont() );

    CreatedObject = List;
    CreatedWindow = List;
    List->WI.SetWidth(ax);
    List->WI.SetHeight(ay);

    List->SetData( Data );
    List->AddItems( itemsList );
    // binding events
    if( tag.HasParam(wxT("ONSELECT")) )  {
      List->SetOnSelectStr( tag.GetParam(wxT("ONSELECT")).c_str() );
      List->OnSelect->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    if( tag.HasParam(wxT("ONDBLCLICK")) )  {
      List->SetOnDblClickStr( tag.GetParam(wxT("ONDBLCLICK")).c_str() );
      List->OnDblClick->Add((AEventsDispatcher*)(TGlXApp::GetMainForm()), ID_ONLINK);
    }
    // creating cell
    m_WParser->GetContainer()->InsertCell(new wxHtmlWidgetCell(List, fl));
  }
/******************* END OF CONTROLS ******************************************/
  if( LinkInfo != NULL )  delete LinkInfo;
  if( CreatedObject != NULL && !ObjectName.IsEmpty() )  {
    if( !TGlXApp::GetMainForm()->GetHtml()->AddObject(ObjectName, CreatedObject, CreatedWindow, tag.HasParam(wxT("MANAGE")) ) )
      TBasicApp::GetLog().Error(olxstr("HTML: duplicated object \'") << ObjectName << '\'');
    if( CreatedWindow != NULL )  {
      olxstr bgc, fgc;
      if( tag.HasParam(wxT("BGCOLOR")) )  {
        bgc = tag.GetParam( wxT("BGCOLOR") ).c_str();
        TGlXApp::GetMainForm()->ProcessMacroFunc(bgc);
      }
      if( tag.HasParam(wxT("FGCOLOR")) )  {
        fgc = tag.GetParam( wxT("FGCOLOR") ).c_str();
        TGlXApp::GetMainForm()->ProcessMacroFunc(fgc);
      }

      if( EsdlInstanceOf(*CreatedWindow, TComboBox) )  {
        TComboBox* Box = (TComboBox*)CreatedWindow;
        if( !bgc.IsEmpty() )  {
          wxColor bgCl = wxColor( uiStr(bgc) );
          Box->SetBackgroundColour( bgCl );
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetBackgroundColour( bgCl );
#ifndef __MAC__          
          if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL )
            Box->GetPopupControl()->GetControl()->SetBackgroundColour( bgCl );
#endif
        }
        if( !fgc.IsEmpty() )  {
          wxColor fgCl = wxColor( uiStr(bgc) );
          Box->SetForegroundColour( fgCl );
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetForegroundColour( fgCl );
#ifndef __MAC__          
          if( Box->GetPopupControl() != NULL && Box->GetPopupControl()->GetControl() != NULL)
            Box->GetPopupControl()->GetControl()->SetForegroundColour( fgCl );
#endif
        }
      }
      else  {
        if( !bgc.IsEmpty() )
          CreatedWindow->SetBackgroundColour( wxColor( uiStr(bgc) ) );
        if( !fgc.IsEmpty() )
          CreatedWindow->SetForegroundColour( wxColor( uiStr(fgc) ) );
      }
    }
  }
  return false;
}

TAG_HANDLER_END(INPUT)

TAGS_MODULE_BEGIN(Input)
    TAGS_MODULE_ADD(INPUT)
    TAGS_MODULE_ADD(IMAGE)
    TAGS_MODULE_ADD(SWITCHINFOS)
    TAGS_MODULE_ADD(SWITCHINFOE)
TAGS_MODULE_END(Input)

//----------------------------------------------------------------------------//
// THtmlObject implementation
//----------------------------------------------------------------------------//
AHtmlObject::~AHtmlObject(){  return; }
//----------------------------------------------------------------------------//
// THtmlSwitch implementation
//----------------------------------------------------------------------------//
THtmlSwitch::THtmlSwitch(THtml *ParentHtml, THtmlSwitch *ParentSwitch):AHtmlObject(ParentHtml)
{
  FParentHtml = ParentHtml;
  FParent = ParentSwitch;
  FUpdateSwitch = true;
  FFileIndex = 0;
}
//..............................................................................
THtmlSwitch::~THtmlSwitch()  {
  Clear();
}
//..............................................................................
void THtmlSwitch::Clear()  {
  FSwitches.Clear();
  FFuncs.Clear();
  FLinks.Clear();
  FStrings.Clear();
  //FParams.Clear();
}
//..............................................................................
void  THtmlSwitch::FileIndex(short ind)  {
  FFileIndex = ind;
  this->FParentHtml->SetSwitchState( *this, FFileIndex );
}
//..............................................................................
void THtmlSwitch::UpdateFileIndex()  {
  Clear();
  if( FFileIndex >= FileCount() || FFileIndex < 0 )  return;

  olxstr FN = FFiles.String(FFileIndex);
  IInputStream *is = TFileHandlerManager::GetInputStream(FN);
  if( is == NULL )  {
    TBasicApp::GetLog().Error( olxstr("THtmlSwitch::File does not exist: ") << FN );
    return;
  }
  FStrings.LoadFromTextStream(*is);
  delete is;
  FParentHtml->CheckForSwitches(*this, TZipWrapper::IsZipFile(FN) );
  for( int i=0; i < SwitchCount(); i++ )
    FSwitches[i].UpdateFileIndex();
}
//..............................................................................
bool THtmlSwitch::ToFile()  {
  if( FSwitches.IsEmpty() )  return true;
  if( FFileName.IsEmpty() )  return true;
  for( int i=0; i < FStrings.Count(); i++ )  {
    if( FStrings.Object(i) )  {
      AHtmlObject* HO = FStrings.Object(i);
      if( EsdlInstanceOf(*HO, THtmlSwitch) )  {
        FParentHtml->UpdateSwitchState(*(THtmlSwitch*)HO, FStrings.String(i));
        ((THtmlSwitch*)HO)->ToFile();
      }
    }
  }
  FStrings.SaveToFile(FFileName);
  return true;
}
//..............................................................................
THtmlFunc& THtmlSwitch::NewFunc()  {
  return FFuncs.AddNew(FParentHtml, this);
}
//..............................................................................
THtmlSwitch& THtmlSwitch::NewSwitch()  {
  return FSwitches.AddNew(FParentHtml, this);
}
//..............................................................................
THtmlLink& THtmlSwitch::NewLink()  {
  return FLinks.AddNew(FParentHtml, this);
}
//..............................................................................
int THtmlSwitch::FindSimilar(const olxstr& start, const olxstr& end, TPtrList<THtmlSwitch>& ret)  {
  int cnt = 0;
  for( int i=0; i < FSwitches.Count(); i++ )  {
    THtmlSwitch& I = FSwitches[i];
    if( end.IsEmpty() )  {
      if( I.Name().StartsFrom(start) )  {
        ret.Add( &I );
        cnt += (I.FindSimilar(start, end, ret) + 1);
      }
    }
    else {
      if( I.Name().StartsFrom(start) &&  I.Name().EndsWith(end) )  {
        ret.Add( &I );
        cnt += (I.FindSimilar(start, end, ret) + 1);
      }
    }
  }
  return cnt;
}
//..............................................................................
THtmlSwitch*  THtmlSwitch::FindSwitch(const olxstr &IName)  {
  for( int i=0; i < FSwitches.Count(); i++ )  {
    if( FSwitches[i].Name().Comparei(IName) == 0 )
      return &FSwitches[i];
    else  {
      THtmlSwitch* Res = FSwitches[i].FindSwitch(IName);
      if( Res != NULL ) return Res;
    }
  }
  return NULL;
}
//..............................................................................
void THtmlSwitch::ToStrings(TStrList &List)  {
  olxstr Tmp, Tmp1;
  AHtmlObject *HO;
  if( FFileIndex >= 0 && FFileIndex < FFiles.Count() )  {
    Tmp = "<SWITCHINFOS SRC=\"";
    Tmp << FFiles.String(FFileIndex) << "\">";
    List.Add(Tmp);
  }
  for( int i=0; i < FStrings.Count(); i++ )  {
    if( FStrings.Object(i) != NULL )  {
      HO = FStrings.Object(i);
      HO->ToStrings(List);
    }
    else  {
      // replace the parameters with their values
      if( FStrings.String(i).IndexOf('#') != -1 )  {
        Tmp = FStrings.String(i);
        // "key word parameter"
        Tmp.Replace( "#switch_name", FName );
        if( FParent != NULL )
          Tmp.Replace( "#parent_name", FParent->Name() );

        for( int j=0; j < FParams.Count(); j++ )  {
          Tmp1 = '#';  Tmp1 << FParams.GetName(j);
          Tmp.Replace( Tmp1, FParams.GetValue(j) );
        }
        List.Add( Tmp );

      } // end of parameter replcaement
      else
        List.Add( FStrings.String(i) );
    }
  }
  if( FFileIndex >= 0 && FFileIndex < FFiles.Count() )
    List.Add("<SWITCHINFOE>");
}
//----------------------------------------------------------------------------//
// THtmlLink implementation
//----------------------------------------------------------------------------//
THtmlLink::THtmlLink(THtml *ParentHtml, THtmlSwitch *ParentSwitch):AHtmlObject(ParentHtml)
{  FParent = ParentSwitch;  }
//..............................................................................
THtmlLink::~THtmlLink()
{  ;  }
//..............................................................................
void THtmlLink::ToStrings(TStrList &List) {
  if( FileName().IsEmpty() )  return;

  IInputStream *is = TFileHandlerManager::GetInputStream( FileName() );
  if( is == NULL )  {
    TBasicApp::GetLog().Error(olxstr("THtmlSwitch::File does not exist: ") << FileName());
    return;
  }
  TStrList SL;
  SL.LoadFromTextStream(*is);
  delete is;
  List.AddList(SL);
}
//----------------------------------------------------------------------------//
// THtmlFunc implementation
//----------------------------------------------------------------------------//
THtmlFunc::THtmlFunc(THtml *ParentHtml, THtmlSwitch *ParentSwitch):AHtmlObject(ParentHtml)
{  FParent = ParentSwitch;  }
//..............................................................................
THtmlFunc::~THtmlFunc()
{ ; }
//..............................................................................
olxstr THtmlFunc::Evaluate()  {
  olxstr F = Func();
  FParentHtml->OnCmd->Execute(this, &F);
  return F;
}
//..............................................................................
void THtmlFunc::ToStrings(TStrList &List){  List.Add(Evaluate()); };
//----------------------------------------------------------------------------//
// THtml implementation
//----------------------------------------------------------------------------//
BEGIN_EVENT_TABLE(THtml, wxHtmlWindow)
  EVT_LEFT_DCLICK(THtml::OnMouseDblClick)
  EVT_LEFT_DOWN(THtml::OnMouseDown)
  EVT_LEFT_UP(THtml::OnMouseUp)
  EVT_MOTION(THtml::OnMouseMotion)
  EVT_SCROLL(THtml::OnScroll)

  EVT_CHAR(THtml::OnChar)
END_EVENT_TABLE()
//..............................................................................
void THtml::OnLinkClicked(const wxHtmlLinkInfo& link)
{
  olxstr Href = link.GetHref().c_str();
  int val;
  int ind = Href.FirstIndexOf('%');
  while( ind >=0 && ((ind+2) < Href.Length()) )  {
    val = Href.SubString(ind+1, 2).RadInt<int>(16);
    Href.Delete(ind, 3);
    Href.Insert(val, ind);
    ind = Href.FirstIndexOf('%');
  }
  if( !OnLink->Execute(this, (IEObject*)&Href) )  {
    wxHtmlLinkInfo NewLink( uiStr(Href), link.GetTarget());
    wxHtmlWindow::OnLinkClicked(NewLink);
  }
}
//..............................................................................
wxHtmlOpeningStatus THtml::OnOpeningURL(wxHtmlURLType type, const wxString& url, wxString *redirect) const
{
  olxstr Url = url.c_str();
  if( !OnURL->Execute(this, &Url) )  return wxHTML_OPEN;
  return wxHTML_BLOCK;
}
//..............................................................................
THtml::THtml(wxWindow *Parent, ALibraryContainer* LC): 
     wxHtmlWindow(Parent), WI(this), ObjectsState(*this)  {
  FActions = new TActionQList;
  FRoot = new THtmlSwitch(this, NULL);
  OnLink = &FActions->NewQueue("ONLINK");
  OnURL = &FActions->NewQueue("ONURL");
  OnDblClick = &FActions->NewQueue("ONDBLCL");
  OnKey = &FActions->NewQueue("ONCHAR");
  OnCmd = &FActions->NewQueue("ONCMD");
  FMovable = false;
  FMouseDown = false;
  ShowTooltips = true;
  FLockPageLoad = 0;
  FPageLoadRequested = false;
  if( LC && ! THtml::Library )  {
    THtml::Library = LC->GetLibrary().AddLibrary("html");

    InitMacroA( *THtml::Library, THtml, ItemState, ItemState, , fpAny^(fpNone|fpOne) );
    InitMacro( LC->GetLibrary(), THtml, ItemState, , fpAny^(fpNone|fpOne) );

    InitMacroA( *THtml::Library, THtml, UpdateHtml, UpdateHtml, , fpNone|fpOne );
    InitMacro( LC->GetLibrary(), THtml, UpdateHtml, , fpNone|fpOne );

    InitMacroA( *THtml::Library, THtml, HtmlHome, Home, , fpNone|fpOne );
    InitMacro( LC->GetLibrary(), THtml, HtmlHome, , fpNone|fpOne );

    InitMacroA( *THtml::Library, THtml, HtmlReload, Reload, , fpNone|fpOne );
    InitMacro( LC->GetLibrary(), THtml, HtmlReload, , fpNone|fpOne );

    InitMacroA( *THtml::Library, THtml, HtmlLoad, Load, , fpOne|fpTwo );
    InitMacro( LC->GetLibrary(), THtml, HtmlLoad, , fpOne|fpTwo );

    InitMacroA( *THtml::Library, THtml, HtmlDump, Dump, , fpOne|fpTwo );
    InitMacro( LC->GetLibrary(), THtml, HtmlDump, , fpOne|fpTwo );

    InitMacroA( *THtml::Library, THtml, Tooltips, Tooltips, , fpNone|fpOne|fpTwo );
    InitMacroA( LC->GetLibrary(), THtml, Tooltips, Htmltt, , fpNone|fpOne|fpTwo );

    InitMacroD( *THtml::Library, THtml, SetFonts, EmptyString, fpTwo,
      "Sets normal and fixed fonts to display HTML content");

    InitMacroD( *THtml::Library, THtml, SetBorders, EmptyString, fpOne|fpTwo,
      "Sets borders between HTML content and window edges");
    InitMacroD( *THtml::Library, THtml, DefineControl, 
      "v-value&;i-tems&;c-checked/down&;bg-background color&;fg-foreground color;&;min-min value&;max-max value", 
      fpTwo, "Defines a managed control properties");
    InitMacroD( *THtml::Library, THtml, Hide, EmptyString, 
      fpOne, "Hides an Html popup window");

    this_InitFuncD(GetValue, fpOne, "Returns value of specified object");
    this_InitFuncD(GetData, fpOne, "Returns data associated with specified object");
    this_InitFuncD(GetState, fpOne, "Returns state of the checkbox");
    this_InitFuncD(GetLabel, fpOne,
"Returns labels of specified object. Applicable to labels, buttons and checkboxes");
    this_InitFuncD(GetImage, fpOne, "Returns image source for a button or zimg");
    this_InitFuncD(GetItems, fpOne, "Returns items of a combobox or list");
    this_InitFuncD(SetValue, fpTwo, "Sets value of specified object");
    this_InitFuncD(SetData, fpTwo, "Sets data for specified object");
    this_InitFuncD(SetState, fpTwo, "Sets state of a checkbox");
    this_InitFuncD(SetLabel, fpTwo, "Sets labels for a label, button or checkbox");
    this_InitFuncD(SetImage, fpTwo, "Sets image location for a button or a zimg");
    this_InitFuncD(SetItems, fpTwo, "Sets items for comboboxes and lists");
    this_InitFuncD(SaveData, fpOne,
"Saves state, data, label and value of all objects to a file");
    this_InitFuncD(LoadData, fpOne,
"Loads previously saved data of html objects form a file");
    this_InitFuncD(SetFG, fpTwo, "Sets foreground of specified object");
    this_InitFuncD(SetBG, fpTwo, "Sets background of specified object");
    this_InitFuncD(GetFontName, fpNone, "Returns current font name");
    this_InitFuncD(GetBorders, fpNone, "Returns borders width between HTML content and window boundaries");
    this_InitFuncD(SetFocus, fpOne,    "Sets input focus to the specified HTML control");
    this_InitFuncD(GetItemState, fpOne|fpTwo, "Returns item state of provided switch");
    this_InitFuncD(IsItem, fpOne, "Returns true if specified switch exists");
    this_InitFuncD(IsPopup, fpOne, "Returns true if specified popup window exists and visible");
  }
}
//..............................................................................
THtml::~THtml()  {
  TFileHandlerManager::Clear();
  delete FActions;
  delete FRoot;
  ClearSwitchStates();
}
//..............................................................................
void THtml::SetSwitchState(THtmlSwitch& sw, int state)  {
  int ind = FSwitchStates.IndexOf( sw.Name() );
  if( ind == -1 )
    FSwitchStates.Add(sw.Name(), state);
  else
    FSwitchStates.Object(ind) = state;
}
//..............................................................................
int THtml::GetSwitchState(const olxstr& switchName)  {
  int ind = FSwitchStates.IndexOf( switchName );
  return (ind == -1) ? UnknownSwitchState : FSwitchStates.GetObject(ind);
}
//..............................................................................
void THtml::ClearSwitchStates()  {
  FSwitchStates.Clear();
}
//..............................................................................
void THtml::OnMouseDown(wxMouseEvent& event)  {
  FMouseX = event.GetX();
  FMouseY = event.GetY();
  event.Skip();
  if( FMovable )  {
    SetCursor( wxCursor(wxCURSOR_SIZING) );
    FMouseDown = true;
  }
}
//..............................................................................
void THtml::OnMouseUp(wxMouseEvent& event)  {
  event.Skip();
  if( FMovable )  {
    FMouseDown = false;
    SetCursor( wxCursor(wxCURSOR_ARROW) );
  }
}
//..............................................................................
void THtml::OnMouseMotion(wxMouseEvent& event)  {
  event.Skip();
  if( !FMovable || !FMouseDown )  return;
  int dx = event.GetX() - FMouseX;
  int dy = event.GetY() - FMouseY;
  if( !dx && !dy )  return;
  wxWindow *parent = GetParent();
  if( !parent || !parent->GetParent() )  return;

  int x=0, y=0;
  parent->GetPosition(&x, &y);
  parent->SetSize(x+dx, y+dy, -1, -1, wxSIZE_USE_EXISTING);
}
//..............................................................................
void THtml::OnMouseDblClick(wxMouseEvent& event)  {
  event.Skip();
  OnDblClick->Execute(this, NULL);
}
//..............................................................................
void THtml::OnChar(wxKeyEvent& event)  {
  TKeyEvent KE(event);
  OnKey->Execute(this, &KE);
}
//..............................................................................
void THtml::UpdateSwitchState(THtmlSwitch &Switch, olxstr &String)  {
  if( !Switch.UpdateSwitch() )  return;
  olxstr Tmp = "<!-- #include ";
  Tmp << Switch.Name() << ' ';
  for( int i=0; i < Switch.FileCount(); i++ )
    Tmp << Switch.File(i) << ';';
  for( int i=0; i < Switch.Params().Count(); i++ )  {
    Tmp << Switch.Params().GetName(i) << '=';
    if( Switch.Params().GetValue(i).FirstIndexOf(' ') == -1 )
      Tmp << Switch.Params().GetValue(i);
    else
      Tmp << '\'' << Switch.Params().GetValue(i) << '\'';
    Tmp << ';';
  }

  Tmp << Switch.FileIndex()+1 << ';' << " -->";
  String = Tmp;
}
//..............................................................................
void THtml::CheckForSwitches(THtmlSwitch &Sender, bool izZip)  {
  TStrPObjList<olxstr,AHtmlObject*>& Lst = Sender.Strings();
  TStrList Toks;
  int ind;
  olxstr Tag  = "<!-- #include ",
           Tag1 = "<!-- #itemstate ",
           Tag2 = "<!-- #cmd ",
           Tag3 = "<!-- #link ",
           Tag4 = "<!-- #includeif ";
  olxstr Tmp;
  for( int i=0; i < Lst.Count(); i++ )  {
    // TRANSLATION START
    TGlXApp::GetMainForm()->TranslateString( Lst[i] );
    if( Lst[i].IndexOf("$") >= 0 )
      TGlXApp::GetMainForm()->ProcessMacroFunc( Lst[i] );
    // TRANSLATIOn END
    if( Lst[i].StartsFrom(Tag) || Lst[i].StartsFrom(Tag4) )  {
      if( Lst[i].StartsFrom(Tag4) )
        Tmp = Lst[i].SubStringFrom(Tag4.Length());
      else
        Tmp = Lst[i].SubStringFrom(Tag.Length());
      Toks.Clear();
      Toks.Strtok(Tmp, ' '); // extract item name
      if( (Lst[i].StartsFrom(Tag4) && Toks.Count() < 4) ||
          (Lst[i].StartsFrom(Tag) && Toks.Count() < 3) )  {
        TBasicApp::GetLog().Error(olxstr("Wrong #include[if] syntax: ") << Lst[i]);
        continue;
      }
      if( Lst[i].StartsFrom(Tag4) )  {
        Tmp = Toks[0];
        if( TGlXApp::GetMainForm()->ProcessMacroFunc( Tmp ) )  {
          if( !Tmp.ToBool() )  continue;
        }
        else  continue;
        Toks.Delete(0);
      }
      THtmlSwitch* Sw = &Sender.NewSwitch();
      Lst.Object(i) = Sw;
      Sw->Name(Toks[0]);
      Toks.Delete(0);
      Toks.Delete(Toks.Count()-1);
      Tmp = Toks.Text(' ');
      Toks.Clear();
      TParamList::StrtokParams(Tmp, ';', Toks); // extract arguments
      if( Toks.Count() < 2 )  { // must be at least 2 for filename and status
        TBasicApp::GetLog().Error( olxstr("Wrong defined field (not enough data)") << Sw->Name());
        continue;
      }

      for( int j=0; j < Toks.Count()-1; j++ )  {
        if( Toks.String(j).FirstIndexOf('=') == -1 )  {
          if( izZip && !TZipWrapper::IsZipFile(Toks.String(j)) )  {
            if( Toks.String(j).StartsFrom('\\') || Toks.String(j).StartsFrom('/') )
              Tmp = Toks.String(j).SubStringFrom(1);
            else
              Tmp = TZipWrapper::ComposeFileName(Sender.File(Sender.FileIndex()), Toks.String(j));
          }
          else
            Tmp = Toks.String(j);

          TGlXApp::GetMainForm()->ProcessMacroFunc( Tmp );
          Sw->AddFile(Tmp);
        }
        else  {
          // check for parameters
          if( Toks[j].IndexOf('#') != -1 )  {
            olxstr Tmp = Toks.String(j), Tmp1;
            for( int k=0; k < Sender.Params().Count(); k++ )  {
              Tmp1 = '#';  Tmp1 << Sender.Params().GetName(k);
              Tmp.Replace( Tmp1, Sender.Params().GetValue(k) );
            }
            Sw->AddParam(Tmp);
          }
          else
            Sw->AddParam(Toks.String(j));
        }
      }

      int switchState = GetSwitchState( Sw->Name() );
      if( switchState == UnknownSwitchState )  {
        ind = Toks.LastStr().ToInt();
        if( ind < 0 )  Sw->UpdateSwitch(false);
        ind = abs(ind)-1;
      }
      else  {
        ind = switchState;
      }
      Sw->FileIndex(ind);
    }
    if( Lst[i].StartsFrom(Tag1) )  {
      Toks.Clear();  
      Tmp = Lst[i].SubStringFrom(Tag1.Length());
      Toks.Strtok(Tmp, ' '); // extract item name
      if( Toks.Count() < 3 )  continue;
      THtmlSwitch* Sw = FRoot->FindSwitch(Toks[0]);
      if( Sw == NULL )  {
        TBasicApp::GetLog().Error(olxstr("THtml::CheckForSwitches: Unresolved: ") << Toks.String(0));
        continue;
      }
      Sw->FileIndex(Toks.String(1).ToInt()-1);
    }
    if( Lst[i].StartsFrom(Tag2) )  {
      Toks.Clear();  
      Tmp = Lst[i].SubStringFrom(Tag2.Length());
      Toks.Strtok(Tmp, ' '); // extract item name
      if( Toks.Count() < 2 )  continue;
      THtmlFunc* Fn = &Sender.NewFunc();
      Lst.Object(i) = Fn;
      Fn->Func(Toks.String(0));
//      if( OnCmd->Execute(this, &(Toks.String(0))) )
//      { Lst[i] = Toks.String(0); }
    }
    if( Lst[i].StartsFrom(Tag3) )   { // html link
      Toks.Clear();  
      Tmp = Lst[i].SubStringFrom(Tag3.Length());
      Toks.Strtok(Tmp, ' '); // extract file name
      if( Toks.Count() < 2 )  continue;
      Toks.Delete(Toks.Count()-1);  // delete --!>
      THtmlLink* Lk = &Sender.NewLink();
      Lst.Object(i) = Lk;
      Lk->FileName(Toks.Text(' '));
//      if( OnCmd->Execute(this, &(Toks.String(0))) )
//      { Lst[i] = Toks.String(0); }
    }
  }
}
//..............................................................................
bool THtml::ProcessPageLoadRequest()  {
  if( !FPageLoadRequested || IsPageLocked() )  return false;
  FPageLoadRequested = false;
  bool res = false;
  if( !FPageRequested.IsEmpty() )
    res = LoadPage( uiStr(FPageRequested) );
  else
    res = UpdatePage();
  FPageRequested  = EmptyString;
  return res;
}
//..............................................................................
bool THtml::ReloadPage()  {
  if( IsPageLocked() )  {
    FPageLoadRequested = true;
    FPageRequested = FFileName;
    return true;
  }
  FObjects.Clear();
  return  LoadPage( uiStr(FFileName) );
}
//..............................................................................
bool THtml::LoadPage(const wxString &file)  {
  if( file.length() == 0 )
    return false;
  
  if( IsPageLocked() )  {
    FPageLoadRequested = true;
    FPageRequested = file.c_str();
    return true;
  }

  olxstr File(file.c_str()), TestFile(file.c_str());
  olxstr Path = TEFile::ExtractFilePath(File);
  TestFile = TEFile::ExtractFileName(File);
  if( Path.Length() > 1 )  {
    Path = TEFile::OSPath(Path);
    if( Path.CharAt(1) == ':' )  FWebFolder = Path;
  }
  else
    Path = FWebFolder;
  if( Path == FWebFolder )  TestFile = FWebFolder + TestFile;
  else                      TestFile = FWebFolder + Path + TestFile;

  if( !TZipWrapper::IsValidFileName(TestFile) && !TFileHandlerManager::Exists(file.c_str()) )  {
    throw TFileDoesNotExistException(__OlxSourceInfo, file.c_str() );
  }
  FRoot->Clear();
  FRoot->ClearFiles();
  FRoot->AddFile(File);

  FRoot->FileIndex(0);
  FRoot->UpdateFileIndex();
  FFileName = File;
  return UpdatePage();
}
//..............................................................................
bool THtml::ItemState(const olxstr &ItemName, short State)  {
  THtmlSwitch * Sw = FRoot->FindSwitch(ItemName);
  if( Sw == NULL )  {
    TBasicApp::GetLog().Error( olxstr("THtml::ItemState: unresolved: ") << ItemName );
    return false;
  }
  Sw->FileIndex(State-1);
  return true;
}
//..............................................................................
bool THtml::UpdatePage()  {
  if( FLockPageLoad )  {     
    FPageLoadRequested = true;
    FPageRequested  = EmptyString;
    return true;
  }

  olxstr Path( TEFile::ExtractFilePath(FFileName) );
  if( TEFile::IsAbsolutePath(FFileName) )  {
    Path = TEFile::OSPath(Path);
    if( Path.CharAt(1) == ':' )  FWebFolder = Path;
  }
  else
    Path = FWebFolder;

  // locate currently focused control
  olxstr focusedControlName;
  wxWindow* focusedControl = FindFocus();
  if( focusedControl != NULL )  {
    for( int i=0; i < FObjects.Count(); i++ )  {
      if( FObjects.Object(i).GetB() == focusedControl )  {
        focusedControlName = FObjects.GetComparable(i);
        break;
      }
    }
  }

  olxstr oldPath( TEFile::CurrentDir() );

  TEFile::ChangeDir(FWebFolder);

  for( int i=0; i < FRoot->SwitchCount(); i++ )  // reload switches
    FRoot->Switch(i).UpdateFileIndex();

  TStrList Res;
//  Res.Add( "<meta http-equiv='Content-Type' content='text/html; charset=") <<
//    TGlXApp::GetMainForm()->GetCurrentLanguageEncodingStr() << "'>";
//  Res.Add( "<meta http-equiv='Content-Type' content='text/html; charset=UNICODE'>");
  FRoot->ToStrings(Res);
  ObjectsState.SaveState();
  FObjects.Clear();
//  TEFile::ChangeDir(FWebFolder);
  int xPos = -1, yPos = -1;
  wxHtmlWindow::GetViewStart(&xPos, &yPos);
  wxHtmlWindow::Freeze();
//  GetParser()->SetInputEncoding( wxFONTENCODING_UTF8 );
  SetPage( Res.Text(' ').u_str() );
  wxHtmlWindow::Scroll(xPos, yPos);
  wxHtmlWindow::Thaw();
  for( int i=0; i < FObjects.Count(); i++ )  {
    if( FObjects.GetObject(i).GetB() != NULL )  {
      // this i the only way to not show the bloody control at (0,0) on windows!
#ifndef __MAC__
      FObjects.Object(i).B()->Move(2000, 2000);
#endif      
      FObjects.Object(i).B()->Show(true);
    }
  }
  // reset global data
  //TFileHandlerManager::Clear();
  ObjectsState.RestoreState();
  SwitchSources.Clear();
  SwitchSource  = EmptyString;
  TEFile::ChangeDir(oldPath);

  if( GetParent() == (wxWindow*)TGlXApp::GetMainForm() && HasScrollbar(wxVERTICAL) )  {
    int w, h;
    GetClientSize(&w, &h);
    if( w != TGlXApp::GetMainForm()->GetHtmlPanelWidth() ) // scrollbar appeared?
      TGlXApp::GetMainForm()->OnResize();
  }
  if( FocusedControl.IsEmpty() )  
    FocusedControl = focusedControlName;
  if( !FocusedControl.IsEmpty() )  {
    int ind = FObjects.IndexOf( FocusedControl );
    FocusedControl = EmptyString;
    if( ind != -1 )  {
      wxWindow* wnd = FObjects.Object(ind).B();
      if( EsdlInstanceOf(*wnd, TTextEdit) )
        ((TTextEdit*)wnd)->SetSelection(-1,-1);
      else if( EsdlInstanceOf(*wnd, TComboBox) )  {
        TComboBox* cb = (TComboBox*)wnd;
        if( cb->GetTextCtrl() != NULL )  {
          cb->GetTextCtrl()->SetSelection(0, -1);
          wnd = cb->GetTextCtrl();
        }
      }
      else if( EsdlInstanceOf(*wnd, TSpinCtrl) )  {
        TSpinCtrl* sc = (TSpinCtrl*)wnd;
        olxstr sv(sc->GetValue());
        sc->SetSelection(sv.Length(),-1);
      }
      wnd->SetFocus();
    }
  }

  return true;
}
//..............................................................................
void THtml::OnScroll(wxScrollEvent& evt)  {  // this is vever called at least on GTK
  evt.Skip();
#ifdef __WXGTK__
  this->Update();
#endif
}
//..............................................................................
void THtml::ScrollWindow(int dx, int dy, const wxRect* rect)  {
#ifdef __WXGTK__
  if( dx == 0 && dy == 0 )  return;
  Freeze();
  wxWindow::ScrollWindow(dx,dy,rect);
  Thaw();
  this->Refresh();
  this->Update();
#else
  wxWindow::ScrollWindow(dx,dy,rect);
#endif
}
//..............................................................................
bool THtml::AddObject(const olxstr& Name, IEObject *Object, wxWindow* wxWin, bool Manage)  {
  if( Name.IsEmpty() )  return true;  // an anonymous object
  if( FObjects.IndexOf(Name) != -1 )  return false;
  FObjects.Add(Name, AnAssociation3<IEObject*, wxWindow*,bool>(Object, wxWin, Manage) );
  return true;
}
//..............................................................................
void THtml::OnCellMouseHover(wxHtmlCell *Cell, wxCoord x, wxCoord y)  {
  wxHtmlLinkInfo *Link = Cell->GetLink(x, y);
  if( Link != NULL )  {
    olxstr Href = Link->GetTarget().c_str();
    if( Href.IsEmpty() )
      Href = Link->GetHref().c_str();

    int val;
    int ind = Href.FirstIndexOf('%');
    while( ind >=0 && ((ind+2) < Href.Length()) )  {
      if( Href.CharAt(ind+1) == '%' )  {
        Href.Delete(ind, 1);
        ind = Href.FirstIndexOf('%', ind+1);
        continue;
      }
      val = Href.SubString(ind+1, 2).RadInt<int>(16);
      Href.Delete(ind, 3);
      Href.Insert((char)val, ind);
      ind = Href.FirstIndexOf('%', ind+1);
    }
#if defined(__WIN32__) || defined(__WXGTK__)
    if( ShowTooltips )  {
      wxToolTip *tt = GetToolTip();
      Href.Replace("#href", Link->GetHref().c_str() );
      wxString wxs( Href.u_str() );
      if( !tt || tt->GetTip() != wxs )  {
        SetToolTip( wxs );
      }
    }
#endif
  }
  else
    SetToolTip(NULL);
}
//..............................................................................
//..............................................................................
//..............................................................................
THtmlImageCell::THtmlImageCell(wxWindow *window, wxFSFile *input,
                                 int w, int h, double scale, int align,
                                 const wxString& mapname) : wxHtmlCell()
{
  m_window = window ? wxStaticCast(window, wxScrolledWindow) : NULL;
  m_scale = scale;
  m_showFrame = false;
  m_bitmap = NULL;
  m_bmpW = w;
  m_bmpH = h;
  m_mapName = mapname;
  SetCanLiveOnPagebreak(false);
#if wxUSE_GIF && wxUSE_TIMER
  m_gifDecoder = NULL;
  m_gifTimer = NULL;
  File = input;
  m_physX = m_physY = wxDefaultCoord;
#endif
  if( m_bmpW && m_bmpH )  {
    if( input != NULL )  {
      wxInputStream *s = input->GetStream();
      if( s != NULL )  {
        bool readImg = true;

#if wxUSE_GIF && wxUSE_TIMER && !wxCHECK_VERSION(2,8,0)
        if( (input->GetLocation().Matches(wxT("*.gif")) ||
             input->GetLocation().Matches(wxT("*.GIF"))) && m_window )
        {
          m_gifDecoder = new wxGIFDecoder(s, true);
          if( m_gifDecoder->ReadGIF() == wxGIF_OK )  {
            wxImage img;
            if( m_gifDecoder->ConvertToImage(&img) )
              SetImage(img);
            readImg = false;
            if( m_gifDecoder->IsAnimation() )  {
              m_gifTimer = new wxGIFTimer(this);
              m_gifTimer->Start(m_gifDecoder->GetDelay(), true);
            }
            else  {
              wxDELETE(m_gifDecoder);
            }
          }
          else  {
            wxDELETE(m_gifDecoder);
          }
        }
        if( readImg )
#endif // wxUSE_GIF && wxUSE_TIMER
        {
          wxImage image(*s, wxBITMAP_TYPE_ANY);
          if( image.Ok() )
            SetImage(image);
        }
      }
    }
  }
  else  {  // input==NULL, use "broken image" bitmap
    if( m_bmpW == wxDefaultCoord && m_bmpH == wxDefaultCoord )  {
      m_bmpW = 29;
      m_bmpH = 31;
    }
    else  {
      m_showFrame = true;
      if( m_bmpW == wxDefaultCoord ) m_bmpW = 31;
      if( m_bmpH == wxDefaultCoord ) m_bmpH = 33;
    }
    m_bitmap = new wxBitmap(wxArtProvider::GetBitmap(wxART_MISSING_IMAGE));
  }
    //else: ignore the 0-sized images used sometimes on the Web pages

  m_Width = (int)(scale * (double)m_bmpW);
  m_Height = (int)(scale * (double)m_bmpH);

  switch( align )  {
    case wxHTML_ALIGN_TOP :
      m_Descent = m_Height;
      break;
    case wxHTML_ALIGN_CENTER :
      m_Descent = m_Height / 2;
      break;
    case wxHTML_ALIGN_BOTTOM :
    default :
      m_Descent = 0;
      break;
  }
}
//..............................................................................
void THtmlImageCell::SetImage(const wxImage& img)  {
  if( img.Ok() )  {
    delete m_bitmap;
    int ww, hh;
    ww = img.GetWidth();
    hh = img.GetHeight();

     if( m_bmpW == wxDefaultCoord )  m_bmpW = ww;
     if( m_bmpH == wxDefaultCoord )  m_bmpH = hh;

     if ((m_bmpW != ww) || (m_bmpH != hh))  {
         wxImage img2 = img.Scale(m_bmpW, m_bmpH, wxIMAGE_QUALITY_HIGH);
         m_bitmap = new wxBitmap(img2);
     }
     else
         m_bitmap = new wxBitmap(img);
  }
}
//..............................................................................
#if wxUSE_GIF && wxUSE_TIMER && !wxCHECK_VERSION(2,8,0)
void THtmlImageCell::AdvanceAnimation(wxTimer *timer)  {
  wxImage img;
  m_gifDecoder->GoNextFrame(true);

  if( m_physX == wxDefaultCoord )  {
    m_physX = m_physY = 0;
    for(wxHtmlCell *cell = this; cell; cell = cell->GetParent() )  {
      m_physX += cell->GetPosX();
      m_physY += cell->GetPosY();
    }
  }

  int x, y;
  m_window->CalcScrolledPosition(m_physX, m_physY, &x, &y);
  wxRect rect(x, y, m_Width, m_Height);

  if( m_window->GetClientRect().Intersects(rect) &&
      m_gifDecoder->ConvertToImage(&img) )
  {
    if( (int)m_gifDecoder->GetWidth() != m_Width ||
         (int)m_gifDecoder->GetHeight() != m_Height ||
         m_gifDecoder->GetLeft() != 0 || m_gifDecoder->GetTop() != 0 )
      {
        wxBitmap bmp(img);
        wxMemoryDC dc;
        dc.SelectObject(*m_bitmap);
        dc.DrawBitmap(bmp, m_gifDecoder->GetLeft(), m_gifDecoder->GetTop(), true /* use mask */);
      }
      else
        SetImage(img);
      m_window->Refresh(img.HasMask(), &rect);
  }
  timer->Start(m_gifDecoder->GetDelay(), true);
}
#endif
//..............................................................................
void THtmlImageCell::Layout( int w )  {
  wxHtmlCell::Layout(w);
  m_physX = m_physY = wxDefaultCoord;
}
//..............................................................................

THtmlImageCell::~THtmlImageCell()  {
    if( File != NULL )  {
      delete File;
    }
    delete m_bitmap;
#if wxUSE_GIF && wxUSE_TIMER
    delete m_gifTimer;
    delete m_gifDecoder;
#endif
}


void THtmlImageCell::Draw(wxDC& dc, int x, int y,
                           int WXUNUSED(view_y1), int WXUNUSED(view_y2),
                           wxHtmlRenderingInfo& WXUNUSED(info))
{
    if ( m_showFrame )
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawRectangle(x + m_PosX, y + m_PosY, m_Width, m_Height);
        x++, y++;
    }
    if ( m_bitmap )
    {
        // We add in the scaling from the desired bitmap width
        // and height, so we only do the scaling once.
        double imageScaleX = 1.0;
        double imageScaleY = 1.0;
        if (m_bmpW != m_bitmap->GetWidth())
            imageScaleX = (double) m_bmpW / (double) m_bitmap->GetWidth();
        if (m_bmpH != m_bitmap->GetHeight())
            imageScaleY = (double) m_bmpH / (double) m_bitmap->GetHeight();

        double us_x, us_y;
        dc.GetUserScale(&us_x, &us_y);
        dc.SetUserScale(us_x * m_scale * imageScaleX, us_y * m_scale * imageScaleY);
        int cx = (int) ((double)(x + m_PosX) / (m_scale*imageScaleX)),
            cy = (int) ((double)(y + m_PosY) / (m_scale*imageScaleY));
//        dc.DrawBitmap(*m_bitmap, cx+1, cy, true);
        dc.DrawBitmap(*m_bitmap, cx, cy, true);
        dc.SetUserScale(us_x, us_y);
        if( Text.Len() )
        {
          dc.SetTextForeground( *wxBLACK );
//          wxFont fnt = dc.GetFont();
//          fnt.SetPointSize(25);
//          dc.SetFont( fnt );
          dc.DrawText(Text, x + m_PosX, y + m_PosY);
        }
    }
}

wxHtmlLinkInfo *THtmlImageCell::GetLink( int x, int y ) const {
  wxHtmlContainerCell *p, *op;
  op = p = GetParent();
  while( p != NULL )  {
    op = p;
    p = p->GetParent();
  }
  p = op;
  wxHtmlCell *cell = (wxHtmlCell*)p->Find(wxHTML_COND_ISIMAGEMAP,
                                              (const void*)(&m_mapName));
  return (cell == NULL) ? wxHtmlCell::GetLink( x, y ) : cell->GetLink( x, y );
}
//..............................................................................
/*
the format is following intemstate popup_name item_name statea stateb ... item_nameb ...
if there are more than 1 state for an item the function does the rotation if
one of the states correspond to current - the next one is selected
*/
void THtml::macItemState(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  THtml *html;
  if( Cmds.Count() > 2 && !Cmds[1].IsNumber() )  {
    html = TGlXApp::GetMainForm()->GetHtml( Cmds[0] );
    if( html == NULL )  {
      Error.ProcessingError(__OlxSrcInfo, "could not locate specified popup" );
      return;
    }
    Cmds.Delete(0);
  }
  else
    html = this;

  THtmlSwitch *rootSwitch = html->Root();
  TIntList states;
  TPtrList<THtmlSwitch> Switches;
  olxstr itemName( Cmds[0] );
  for( int i=1; i < Cmds.Count(); i++ )  {
    Switches.Clear();
    if( itemName.FirstIndexOf('*') == -1 )  {
      THtmlSwitch* sw = rootSwitch->FindSwitch(itemName);
      if( sw == NULL )  {
        Error.ProcessingError(__OlxSrcInfo, "could not locate specified switch: ") << itemName;
        return;
      }
      Switches.Add(sw);
    }
    else  {
      if( itemName == "*" )  {
        for( int j=0; j < rootSwitch->SwitchCount(); j++ )
          Switches.Add( &rootSwitch->Switch(j) );
      }
      else  {
        int sindex = itemName.FirstIndexOf('*');
        // *blabla* syntax
        if( sindex == 0 && itemName.Length() > 2 && itemName.CharAt(itemName.Length()-1) == '*')  {
          rootSwitch->FindSimilar( itemName.SubString(1, itemName.Length()-2), EmptyString, Switches );
        }
        else  {  // assuming bla*bla syntax
          olxstr from = itemName.SubStringTo(sindex), with;
          if( (sindex+1) < itemName.Length() )
            with = itemName.SubStringFrom(sindex+1);
          rootSwitch->FindSimilar( from, with, Switches );
        }
      }
    }
    states.Clear();
    for( int j=i; j < Cmds.Count();  j++,i++ )  {
      // is new switch encountered?
      if( !Cmds[j].IsNumber() )  {  itemName = Cmds[j];  break;  }
      states.Add(Cmds[j].ToInt());
    }
    if( states.Count() == 1 )  {  // simply change the state to the request
      for( int j=0; j < Switches.Count(); j++ )  {
        Switches[j]->FileIndex(states[0]-1);
      }
    }
    else  {
      for( int j=0; j < Switches.Count(); j++ )  {
        THtmlSwitch* sw = Switches[j];
        int currentState = sw->FileIndex();
        for( int k=0; k < states.Count(); k++ )  {
          if( states[k] == (currentState+1) )  {
            if( (k+1) < states.Count() )  {  sw->FileIndex( states[k+1] -1);  }
            else  {  sw->FileIndex( states[0]-1 );  }
          }
        }
      }
    }
  }
  html->UpdatePage();
  return;
}
//..............................................................................
void THtml::funGetItemState(const TStrObjList &Params, TMacroError &E)  {
  THtml *html = (Params.Count() == 2) ? TGlXApp::GetMainForm()->GetHtml(Params[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  olxstr itemName( (Params.Count() == 1) ? Params[0] : Params[1] );
  THtmlSwitch *rootSwitch = html->Root();
  THtmlSwitch* sw = rootSwitch->FindSwitch(itemName);
  if( sw == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate specified switch: ") << itemName;
    return;
  }
  E.SetRetVal( sw->FileIndex() );
}
//..............................................................................
void THtml::funIsPopup(const TStrObjList& Params, TMacroError &E)  {
  THtml *html = TGlXApp::GetMainForm()->GetHtml(Params[0]);
  E.SetRetVal( html != NULL && html->GetParent()->IsShown() ); 
}
//..............................................................................
void THtml::funIsItem(const TStrObjList &Params, TMacroError &E)  {
  THtml *html = (Params.Count() == 2) ? TGlXApp::GetMainForm()->GetHtml(Params[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  olxstr itemName( (Params.Count() == 1) ? Params[0] : Params[1] );
  THtmlSwitch *rootSwitch = html->Root();
  THtmlSwitch* sw = rootSwitch->FindSwitch(itemName);
  E.SetRetVal( sw == NULL ? false : true );
}
//..............................................................................
void THtml::SetShowTooltips(bool v)  {
  ShowTooltips = v;
  TStateChange sc(prsHtmlTTVis, v);
  TGlXApp::GetMainForm()->OnStateChange->Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void THtml::macTooltips(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    SetShowTooltips( !GetShowTooltips() );
    return;
  }
  if( Cmds.Count() == 1 )  {
    if( !Cmds.String(0).Comparei("true") || ! Cmds.String(0).Comparei("false") )  {
      this->SetShowTooltips( Cmds[0].ToBool() );
      return;
    }
    else  {
      THtml* html = TGlXApp::GetMainForm()->GetHtml( Cmds.String(0) );
      if( html == NULL )  return;
      html->SetShowTooltips( !html->GetShowTooltips() );
      return;
    }
  }

  THtml* html = TGlXApp::GetMainForm()->GetHtml( Cmds.String(0) );
  if( html != NULL )  {
    html->SetShowTooltips( Cmds[1].ToBool() );
  }
}
//..............................................................................
void THtml::macUpdateHtml(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 1) ? TGlXApp::GetMainForm()->GetHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  html->UpdatePage();
}
//..............................................................................
void THtml::macSetFonts(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  this->SetFonts(Cmds[0], Cmds[1]);
}
//..............................................................................
void THtml::macSetBorders(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 2) ? TGlXApp::GetMainForm()->GetHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  html->SetBorders( Cmds.Last().String().ToInt() );
}
//..............................................................................
void THtml::macHtmlHome(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 1) ? TGlXApp::GetMainForm()->GetHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  html->LoadPage( html->GetHomePage().u_str() );
}
//..............................................................................
void THtml::macHtmlReload(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 1) ? TGlXApp::GetMainForm()->GetHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  html->ReloadPage();
}
//..............................................................................
void THtml::macHtmlLoad(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 2) ? TGlXApp::GetMainForm()->GetHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  html->LoadPage( Cmds.Last().String().u_str() );
}
//..............................................................................
void THtml::macHide(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = TGlXApp::GetMainForm()->GetHtml(Cmds[0]);
  if( html == NULL )  {
    //E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  html->GetParent()->Show(false);
}
//..............................................................................
void THtml::macHtmlDump(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  THtml *html = (Cmds.Count() == 2) ? TGlXApp::GetMainForm()->GetHtml(Cmds[0]) : this;
  if( html == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined html window");
    return;
  }
  TStrList SL;
  html->Root()->ToStrings( SL );
  TUtf8File::WriteLines(Cmds.Last().String(), SL);
}
//..............................................................................
void THtml::macDefineControl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( ObjectsState.FindProperties(Cmds[0]) != NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("control ") << Cmds[0] << " already exists");
    return;
  }
  TSStrStrList<olxstr,false>* props = NULL;
  if( Cmds[1].Comparei("text") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TTextEdit) );
  }
  else if( Cmds[1].Comparei("label") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TLabel) );
  }
  else if( Cmds[1].Comparei("button") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TButton) );
    props->Item("down") = Options.FindValue("c", "false");
  }
  else if( Cmds[1].Comparei("combo") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TComboBox) );
    props->Item("items") = Options.FindValue("i");
  }
  else if( Cmds[1].Comparei("spin") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TSpinCtrl) );
    props->Item("min") = Options.FindValue("min", "0");
    props->Item("max") = Options.FindValue("max", "100");
  }
  else if( Cmds[1].Comparei("slider") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TTrackBar) );
    props->Item("min") = Options.FindValue("min", "0");
    props->Item("max") = Options.FindValue("max", "100");
  }
  else if( Cmds[1].Comparei("checkbox") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TCheckBox) );
    props->Item("checked") = Options.FindValue("c", "false");
  }
  else if( Cmds[1].Comparei("tree") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TTreeView) );
  }
  else if( Cmds[1].Comparei("list") == 0 )  {
    props = ObjectsState.DefineControl(Cmds[0], typeid(TListBox) );
    props->Item("items") = Options.FindValue("i");
  }
  if( props != NULL )
    props->Item("bg") = Options.FindValue("bg");
    props->Item("fg") = Options.FindValue("fg");
    if( props->IndexOfComparable("val") != -1 )
      props->Item("val") = Options.FindValue("v");
}
//..............................................................................
//..............................................................................
olxstr THtml::GetObjectValue(const IEObject *Obj)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )  {  return ((_xl_Controls::TTextEdit*)Obj)->GetText();  }
  if( EsdlInstanceOf(*Obj, TCheckBox) )  {  return ((_xl_Controls::TCheckBox*)Obj)->GetCaption();  }
  if( EsdlInstanceOf(*Obj, TTrackBar) )  {  return ((_xl_Controls::TTrackBar*)Obj)->GetValue(); }
  if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {  return ((_xl_Controls::TSpinCtrl*)Obj)->GetValue(); }
  if( EsdlInstanceOf(*Obj, TButton) )    {  return ((_xl_Controls::TButton*)Obj)->GetCaption();    }
  if( EsdlInstanceOf(*Obj, TComboBox) )  {  return ((_xl_Controls::TComboBox*)Obj)->GetText();  }
  if( EsdlInstanceOf(*Obj, TListBox) )   {  return ((_xl_Controls::TListBox*)Obj)->GetValue();  }
  if( EsdlInstanceOf(*Obj, TTreeView) )  {
    _xl_Controls::TTreeView* T = (_xl_Controls::TTreeView*)Obj;
    wxTreeItemId ni = T->GetSelection();
    //T->
    return T->GetItemText(ni).c_str();
  }
  return EmptyString;
}
void THtml::funGetValue(const TStrObjList &Params, TMacroError &E)  {
  const IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    TSStrStrList<olxstr,false>* props = ObjectsState.FindProperties(Params[0]);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ") << Params[0];
      return;
    }
    if( props->IndexOfComparable("val") == -1 )  {
      E.ProcessingError(__OlxSrcInfo,  "object definition does not have value for: ") << Params[0];
      return;
    }
    E.SetRetVal( props->Item("val") );
  }
  else
    E.SetRetVal( GetObjectValue(Obj) );
}
//..............................................................................
void THtml::SetObjectValue(IEObject *Obj, const olxstr& Value)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )       ((_xl_Controls::TTextEdit*)Obj)->SetText(Value);
  else if( EsdlInstanceOf(*Obj, TCheckBox) )  ((_xl_Controls::TCheckBox*)Obj)->SetCaption(Value);
  else if( EsdlInstanceOf(*Obj, TTrackBar) )  {
    int si = Value.IndexOf(',');
    if( si == -1 )
      ((_xl_Controls::TTrackBar*)Obj)->SetValue(Value.ToInt());
    else
      ((_xl_Controls::TTrackBar*)Obj)->SetRange( Value.SubStringTo(si).ToInt(), Value.SubStringFrom(si+1).ToInt() );
  }
  else if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {
    int si = Value.IndexOf(',');
    if( si == -1 )
      ((_xl_Controls::TSpinCtrl*)Obj)->SetValue(Value.ToInt());
    else
      ((_xl_Controls::TSpinCtrl*)Obj)->SetRange( Value.SubStringTo(si).ToInt(), Value.SubStringFrom(si+1).ToInt() );
  }
  else if( EsdlInstanceOf(*Obj, TButton) )    ((_xl_Controls::TButton*)Obj)->SetLabel(Value.u_str());
  else if( EsdlInstanceOf(*Obj, TComboBox) )  {
    ((_xl_Controls::TComboBox*)Obj)->SetText(Value);
    ((_xl_Controls::TComboBox*)Obj)->Update();
  }
  else if( EsdlInstanceOf(*Obj, TListBox) )  {
    _xl_Controls::TListBox *L = (_xl_Controls::TListBox*)Obj;
    int index = L->GetSelection();
    if( index >=0 && index < L->Count() )
      L->SetString(index, uiStr(Value));
  }
  else  return;
}
void THtml::funSetValue(const TStrObjList &Params, TMacroError &E)  {
  IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    TSStrStrList<olxstr,false>* props = ObjectsState.FindProperties(Params[0]);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ") << Params[0];
      return;
    }
    if( props->IndexOfComparable("val") == -1 )  {
      E.ProcessingError(__OlxSrcInfo,  "object definition does not accept value for: ") << Params[0];
      return;
    }
    if( props->Item("type") == EsdlClassName(TTrackBar) || props->Item("type") == EsdlClassName(TSpinCtrl) )  {
      int si = Params[1].IndexOf(',');
      if( si == -1 )
        props->Item("val") = Params[1];
      else  {
        props->Item("min") = Params[1].SubStringTo(si);
        props->Item("max") = Params[1].SubStringFrom(si+1);
      }
    }
    else
      props->Item("val") = Params[1];
  }
  else  {
    SetObjectValue(Obj, Params[1]);
    Refresh();
  }
}
//..............................................................................
const olxstr& THtml::GetObjectData(const IEObject *Obj)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )  {  return ((_xl_Controls::TTextEdit*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TCheckBox) )  {  return ((_xl_Controls::TCheckBox*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TTrackBar) )  {  return ((_xl_Controls::TTrackBar*)Obj)->GetData(); }
  if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {  return ((_xl_Controls::TSpinCtrl*)Obj)->GetData(); }
  if( EsdlInstanceOf(*Obj, TButton) )    {  return ((_xl_Controls::TButton*)Obj)->GetData();    }
  if( EsdlInstanceOf(*Obj, TComboBox) )  {  return ((_xl_Controls::TComboBox*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TListBox) )  {  return ((_xl_Controls::TListBox*)Obj)->GetData();  }
  if( EsdlInstanceOf(*Obj, TTreeView) )  {  return ((_xl_Controls::TTreeView*)Obj)->GetData();  }
  return EmptyString;
}
void THtml::funGetData(const TStrObjList &Params, TMacroError &E)  {
  const IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ") << Params[0];
    return;
  }
  else
    E.SetRetVal( GetObjectData(Obj) );
}
//..............................................................................
void THtml::funGetItems(const TStrObjList &Params, TMacroError &E)  {
  const IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ") << Params[0];
    return;
  }
  E.SetRetVal( GetObjectItems(Obj) );
}
//..............................................................................
olxstr THtml::GetObjectImage(const IEObject* Obj)  {
  if( EsdlInstanceOf(*Obj, TBmpButton) )
    return ((TBmpButton*)Obj)->GetSource();
  else if( EsdlInstanceOf(*Obj, THtmlImageCell) )
    return ((THtmlImageCell*)Obj)->GetSource();
  return EmptyString;
}
//..............................................................................
bool THtml::SetObjectImage(IEObject* Obj, const olxstr& src)  {
  if( src.IsEmpty() )  return false;

  wxFSFile *fsFile = TFileHandlerManager::GetFSFileHandler( src );
  if( fsFile == NULL )  {
    TBasicApp::GetLog().Error( olxstr("Setimage: could not locate specified file: ") << src );
    return false;
  }
  wxImage image(*(fsFile->GetStream()), wxBITMAP_TYPE_ANY);
  if ( !image.Ok() )  {
    TBasicApp::GetLog().Error( olxstr("Setimage: could not read specified file: ") << src );
    return false;
  }
  if( EsdlInstanceOf(*Obj, TBmpButton) )  {
    ((TBmpButton*)Obj)->SetBitmapLabel( image );
    ((TBmpButton*)Obj)->SetSource( src );
  }
  else if( EsdlInstanceOf(*Obj, THtmlImageCell) )  {
    ((THtmlImageCell*)Obj)->SetImage( image );
    ((THtmlImageCell*)Obj)->SetSource( src );
    ((THtmlImageCell*)Obj)->GetWindow()->Refresh(true);
  }
  else  {
    TBasicApp::GetLog().Error( "Setimage: unsupported object type" );
    return false;
  }
  return true;
}
//..............................................................................
olxstr THtml::GetObjectItems(const IEObject* Obj)  {
  if( EsdlInstanceOf(*Obj, TComboBox) )  {
    return ((TComboBox*)Obj)->ItemsToString(';');
  }
  else if( EsdlInstanceOf(*Obj, TListBox) )  {
    return ((TListBox*)Obj)->ItemsToString(';');
  }
  return EmptyString;
}
//..............................................................................
bool THtml::SetObjectItems(IEObject* Obj, const olxstr& src)  {
  if( src.IsEmpty() )  return false;

  if( EsdlInstanceOf(*Obj, TComboBox) )  {
    ((TComboBox*)Obj)->Clear();
    TStrList items(src, ';');
    ((TComboBox*)Obj)->AddItems( items );
  }
  else if( EsdlInstanceOf(*Obj, TListBox) )  {
    ((TListBox*)Obj)->Clear();
    TStrList items(src, ';');
    ((TListBox*)Obj)->AddItems( items )  ;
  }
  else  {
    TBasicApp::GetLog().Error( "SetItems: unsupported object type" );
    return false;
  }
  return true;
}
//..............................................................................
void THtml::SetObjectData(IEObject *Obj, const olxstr& Data)  {
  if( EsdlInstanceOf(*Obj, TTextEdit) )  {  ((_xl_Controls::TTextEdit*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TCheckBox) )  {  ((_xl_Controls::TCheckBox*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TTrackBar) )  {  ((_xl_Controls::TTrackBar*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TSpinCtrl) )  {  ((_xl_Controls::TSpinCtrl*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TButton) )    {  ((_xl_Controls::TButton*)Obj)->SetData(Data);    return;  }
  if( EsdlInstanceOf(*Obj, TComboBox) )  {  ((_xl_Controls::TComboBox*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TListBox) )  {  ((_xl_Controls::TListBox*)Obj)->SetData(Data);  return;  }
  if( EsdlInstanceOf(*Obj, TTreeView) )  {  ((_xl_Controls::TTreeView*)Obj)->SetData(Data);  return;  }
}
void THtml::funSetData(const TStrObjList &Params, TMacroError &E)  {
  IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  SetObjectData(Obj, Params[1]);
}
//..............................................................................
bool THtml::GetObjectState(const IEObject *Obj)  {
  return EsdlInstanceOf(*Obj, TCheckBox) ? ((_xl_Controls::TCheckBox*)Obj)->IsChecked() : false;
}
void THtml::funGetState(const TStrObjList &Params, TMacroError &E)  {
  const IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    TSStrStrList<olxstr,false>* props = ObjectsState.FindProperties(Params[0]);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ") << Params[0];
      return;
    }
    E.SetRetVal( props->Item("checked") );
  }
  else
    E.SetRetVal( GetObjectState(Obj) );
}
//..............................................................................
void THtml::funGetLabel(const TStrObjList &Params, TMacroError &E)  {
  const IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  olxstr rV;
  if( EsdlInstanceOf(*Obj, TButton) )  rV = ((_xl_Controls::TButton*)Obj)->GetCaption();
  else if( EsdlInstanceOf(*Obj, TLabel) )   rV = ((_xl_Controls::TLabel*)Obj)->GetCaption();
  else if( EsdlInstanceOf(*Obj, TCheckBox) )   rV = ((_xl_Controls::TCheckBox*)Obj)->GetCaption();
  else  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object type: ")  << EsdlObjectName(*Obj);
    return;
  }
  E.SetRetVal( rV );
}
//..............................................................................
void THtml::funSetLabel(const TStrObjList &Params, TMacroError &E)  {
  IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  if( EsdlInstanceOf(*Obj, TButton) )       ((_xl_Controls::TButton*)Obj)->SetCaption( Params[1] );
  else if( EsdlInstanceOf(*Obj, TLabel) )   ((_xl_Controls::TLabel*)Obj)->SetCaption( Params[1] );
  else if( EsdlInstanceOf(*Obj, TCheckBox) )   ((_xl_Controls::TCheckBox*)Obj)->SetCaption( Params[1] );
  else  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object type: ")  << EsdlObjectName(*Obj);
    return;
  }
}
//..............................................................................
void THtml::SetObjectState(IEObject *Obj, bool State)  {
  if( EsdlInstanceOf(*Obj, TCheckBox) )  {
    ((_xl_Controls::TCheckBox*)Obj)->SetChecked(State);
    return;
  }
}
//..............................................................................
void THtml::funSetImage(const TStrObjList &Params, TMacroError &E)  {
  IEObject *Obj = FindObject( Params[0] );
  if( Obj == NULL)  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  if( !SetObjectImage( Obj, Params[1] ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not set image for the object: ")  << Params[0];
  }
}
//..............................................................................
void THtml::funGetImage(const TStrObjList &Params, TMacroError &E)  {
  const IEObject *Obj = FindObject( Params[0] );
  if( Obj == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  E.SetRetVal( GetObjectImage( Obj ) );
}
//..............................................................................
void THtml::funSetFocus(const TStrObjList &Params, TMacroError &E)  {
  int ind = FObjects.IndexOf( Params[0] );
  FocusedControl = Params[0];
  if( ind == -1 )  {
    return;
  }
  wxWindow* wnd = FObjects.Object(ind).B();
  if( EsdlInstanceOf(*wnd, TTextEdit) )
    ((TTextEdit*)wnd)->SetSelection(-1,-1);
  else if( EsdlInstanceOf(*wnd, TComboBox) )  {
    TComboBox* cb = (TComboBox*)wnd;
    cb->GetTextCtrl()->SetSelection(-1, -1);
    wnd = cb->GetTextCtrl();
  }
  wnd->SetFocus();
}
//..............................................................................
void THtml::funSetState(const TStrObjList &Params, TMacroError &E)  {
  IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    TSStrStrList<olxstr,false>* props = ObjectsState.FindProperties(Params[0]);
    if( props == NULL )  {
      E.ProcessingError(__OlxSrcInfo,  "wrong html object name: ") << Params[0];
      return;
    }
    if( props->IndexOfComparable("checked") == -1 )  {
      E.ProcessingError(__OlxSrcInfo,  "object definition does have state for: ") << Params[0];
      return;
    }
    props->Item("checked") = Params[1];
  }
  else
    SetObjectState(Obj, Params[1].ToBool());
}
//..............................................................................
void THtml::funSetItems(const TStrObjList &Params, TMacroError &E)  {
  IEObject *Obj = FindObject(Params[0]);
  if( Obj == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  SetObjectItems(Obj, Params[1]);
}
//..............................................................................
void THtml::funSaveData(const TStrObjList &Params, TMacroError &E)  {
  ObjectsState.SaveToFile(Params[0]);
}
//..............................................................................
void THtml::funLoadData(const TStrObjList &Params, TMacroError &E)  {
  if( !TEFile::FileExists(Params[0]) )  {
    E.ProcessingError(__OlxSrcInfo, "file does not exist: ") << Params[0];
    return;
  }
  if( !ObjectsState.LoadFromFile(Params[0]) )  {
    E.ProcessingError(__OlxSrcInfo, "error occured while processing file" );
    return;
  }
}
//..............................................................................
void THtml::funSetFG(const TStrObjList &Params, TMacroError &E)  {
  int ind = FObjects.IndexOfComparable( Params[0] );
  if( ind == -1 )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  wxWindow* wxw = FObjects.Object(ind).B();
  if( wxw != NULL )  {
    if( EsdlInstanceOf(*wxw, TComboBox) )  {
      TComboBox* Box = (TComboBox*)wxw;
      wxColor fgCl = wxColor( uiStr(Params[1]) );
      Box->SetForegroundColour( fgCl );
      if( Box->GetPopupControl() != NULL )
        Box->GetPopupControl()->GetControl()->SetForegroundColour( fgCl );
      if( Box->GetTextCtrl() != NULL )
        Box->GetTextCtrl()->SetForegroundColour( fgCl );
    }
    else
      wxw->SetForegroundColour( wxColor( uiStr(Params[1]) ) );
    this->Refresh();
  }
}
//..............................................................................
void THtml::funSetBG(const TStrObjList &Params, TMacroError &E)  {
  int ind = FObjects.IndexOfComparable( Params[0] );
  if( ind == -1 )  {
    E.ProcessingError(__OlxSrcInfo, "wrong html object name: ")  << Params[0];
    return;
  }
  wxWindow* wxw = FObjects.Object(ind).B();
  if( wxw != NULL )  {
    if( EsdlInstanceOf(*wxw, TComboBox) )  {
      TComboBox* Box = (TComboBox*)wxw;
      wxColor fgCl = wxColor( uiStr(Params[1]) );
      Box->SetBackgroundColour( fgCl );
      if( Box->GetPopupControl() != NULL )
        Box->GetPopupControl()->GetControl()->SetBackgroundColour( fgCl );
      if( Box->GetTextCtrl() != NULL )
        Box->GetTextCtrl()->SetBackgroundColour( fgCl );
    }
    else
      wxw->SetBackgroundColour( wxColor( uiStr(Params[1]) ) );
    this->Refresh();
  }
}
//..............................................................................
void THtml::funGetFontName(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal<olxstr>( this->GetParser()->GetFontFace().c_str() );
}
//..............................................................................
void THtml::funGetBorders(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal( GetBorders() );
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
THtml::TObjectsState::~TObjectsState()  {
  for( int i=0; i < Objects.Count(); i++ )
    delete Objects.Object(i);
}
//..............................................................................
void THtml::TObjectsState::SaveState()  {
  for( int i=0; i < html.ObjectCount(); i++ )  {
    if( !html.IsObjectManageble(i) )  continue;
    int ind = Objects.IndexOf( html.GetObjectName(i) );
    IEObject* obj = html.GetObject(i);
    wxWindow* win = html.GetWindow(i);
    TSStrStrList<olxstr,false>* props;
    if( ind == -1 )  {
      props = new TSStrStrList<olxstr,false>;
      Objects.Add(html.GetObjectName(i), props);
    }
    else  {
      props = Objects.Object(ind);
      props->Clear();
    }
    props->Add("type", EsdlClassName(*obj));  // type
    if( EsdlInstanceOf(*obj, TTextEdit) )  {  
      _xl_Controls::TTextEdit* te = (_xl_Controls::TTextEdit*)obj;
      props->Add("val", te->GetText() );
    }
    else if( EsdlInstanceOf(*obj, TCheckBox) )  {  
      _xl_Controls::TCheckBox* cb = (_xl_Controls::TCheckBox*)obj;   
      props->Add("val", cb->GetCaption() );
      props->Add("checked", cb->IsChecked() );
    }
    else if( EsdlInstanceOf(*obj, TTrackBar) )  {  
      _xl_Controls::TTrackBar* tb = (_xl_Controls::TTrackBar*)obj;  
      props->Add("min", tb->GetMin() );
      props->Add("max", tb->GetMax() );
      props->Add("val", tb->GetValue() );
    }
    else if( EsdlInstanceOf(*obj, TSpinCtrl) )  {  
      _xl_Controls::TSpinCtrl* sc = (_xl_Controls::TSpinCtrl*)obj;  
      props->Add("min", sc->GetMin() );
      props->Add("max", sc->GetMax() );
      props->Add("val", sc->GetValue() );
    }
    else if( EsdlInstanceOf(*obj, TButton) )    {  
      _xl_Controls::TButton* bt = (_xl_Controls::TButton*)obj;
      props->Add("val", bt->GetCaption() );
      props->Add("down", bt->IsDown() );
    }
    else if( EsdlInstanceOf(*obj, TBmpButton) )    {  
      _xl_Controls::TBmpButton* bt = (_xl_Controls::TBmpButton*)obj;  
      props->Add("down", bt->IsDown() );
      props->Add("val", bt->GetSource() );
    }
    else if( EsdlInstanceOf(*obj, TComboBox) )  {  
      _xl_Controls::TComboBox* cb = (_xl_Controls::TComboBox*)obj;  
      props->Add("val", cb->GetValue().c_str() );
      props->Add("items", cb->ItemsToString(';') );
    }
    else if( EsdlInstanceOf(*obj, TListBox) )  {  
      _xl_Controls::TListBox* lb = (_xl_Controls::TListBox*)obj;  
      props->Add("val", lb->GetValue() );
      props->Add("items", lb->ItemsToString(';') );
    }
    else if( EsdlInstanceOf(*obj, TTreeView) )  {  
//      _xl_Controls::TTreeView* tv = (_xl_Controls::TTreeView*)obj;  
//      props->Add("val", tv->GetValue() );
//      props->Add("items", tv->GetItems() );
    }
    else if( EsdlInstanceOf(*obj, TLabel) )  {  
      _xl_Controls::TLabel* lb = (_xl_Controls::TLabel*)obj;  
      props->Add("val", lb->GetCaption() );
    }
    else //?
      ;
    // stroring the control colours, it is generic 
    if( win != NULL )  {
      props->Add("fg", win->GetForegroundColour().GetAsString(wxC2S_HTML_SYNTAX).c_str() );
      props->Add("bg", win->GetBackgroundColour().GetAsString(wxC2S_HTML_SYNTAX).c_str() );
    }
  }
}
//..............................................................................
void THtml::TObjectsState::RestoreState()  {
  for( int i=0; i < html.ObjectCount(); i++ )  {
    int ind = Objects.IndexOf( html.GetObjectName(i) );
    if( !html.IsObjectManageble(i) )  continue;
    IEObject* obj = html.GetObject(i);
    wxWindow* win = html.GetWindow(i);
    if( ind == -1 )  continue;
    TSStrStrList<olxstr,false>& props = *Objects.Object(ind);
    if( props["type"] != EsdlClassName(*obj) )  {
      TBasicApp::GetLog().Error(olxstr("Object type changed for: ") << Objects.GetString(ind) );
      continue;
    }
    if( EsdlInstanceOf(*obj, TTextEdit) )  {  
      _xl_Controls::TTextEdit* te = (_xl_Controls::TTextEdit*)obj;
      te->SetText( props["val"] );
    }
    else if( EsdlInstanceOf(*obj, TCheckBox) )  {  
      _xl_Controls::TCheckBox* cb = (_xl_Controls::TCheckBox*)obj;   
      cb->SetCaption( props["val"]);
      cb->SetChecked( props["checked"].ToBool() );
    }
    else if( EsdlInstanceOf(*obj, TTrackBar) )  {  
      _xl_Controls::TTrackBar* tb = (_xl_Controls::TTrackBar*)obj;  
      tb->SetRange( props["min"].ToInt(), props["max"].ToInt() );
      tb->SetValue( props["val"].ToInt() );
    }
    else if( EsdlInstanceOf(*obj, TSpinCtrl) )  {  
      _xl_Controls::TSpinCtrl* sc = (_xl_Controls::TSpinCtrl*)obj;  
      sc->SetRange( props["min"].ToInt(), props["max"].ToInt() );
      sc->SetValue( props["val"].ToInt() );
    }
    else if( EsdlInstanceOf(*obj, TButton) )    {  
      _xl_Controls::TButton* bt = (_xl_Controls::TButton*)obj;
      bt->SetCaption( props["val"] );
      bt->OnDown->SetEnabled(false);
      bt->OnUp->SetEnabled(false);
      bt->OnClick->SetEnabled(false);
      bt->SetDown( props["down"].ToBool() );
      bt->OnDown->SetEnabled(true);
      bt->OnUp->SetEnabled(true);
      bt->OnClick->SetEnabled(true);
    }
    else if( EsdlInstanceOf(*obj, TBmpButton) )    {  
      _xl_Controls::TBmpButton* bt = (_xl_Controls::TBmpButton*)obj;  
      bt->SetSource( props["val"] );
      bt->OnDown->SetEnabled(false);
      bt->OnUp->SetEnabled(false);
      bt->OnClick->SetEnabled(false);
      bt->SetDown( props["down"].ToBool() );
      bt->OnDown->SetEnabled(true);
      bt->OnUp->SetEnabled(true);
      bt->OnClick->SetEnabled(true);
    }
    else if( EsdlInstanceOf(*obj, TComboBox) )  {  
      _xl_Controls::TComboBox* cb = (_xl_Controls::TComboBox*)obj;  
      TStrList toks(props["items"], ';');
      cb->Clear();
      cb->AddItems(toks);
      cb->SetText( props["val"] );
    }
    else if( EsdlInstanceOf(*obj, TListBox) )  {  
      _xl_Controls::TListBox* lb = (_xl_Controls::TListBox*)obj;  
      TStrList toks(props["items"], ';');
      lb->Clear();
      lb->AddItems( toks );
    }
    else if( EsdlInstanceOf(*obj, TTreeView) )  {  
    }
    else if( EsdlInstanceOf(*obj, TLabel) )  {  
      _xl_Controls::TLabel* lb = (_xl_Controls::TLabel*)obj;  
      lb->SetCaption( props["val"] );
    }
    else //?
      ;
    // restoring the control colours, it is generic 
    if( win != NULL && false )  {
      olxstr bg(props["bg"]), fg(props["fg"]);
      TGlXApp::GetMainForm()->ProcessMacroFunc( bg );
      TGlXApp::GetMainForm()->ProcessMacroFunc( fg );
      if( EsdlInstanceOf(*win, TComboBox) )  {
        TComboBox* Box = (TComboBox*)win;
        if( !fg.IsEmpty() )  {
          wxColor fgCl = wxColor( fg.u_str() );
          Box->SetForegroundColour( fgCl );
          if( Box->GetPopupControl() != NULL )
            Box->GetPopupControl()->GetControl()->SetForegroundColour( fgCl );
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetForegroundColour( fgCl );
        }
        if( !bg.IsEmpty() )  {
          wxColor bgCl = wxColor( bg.u_str() );
          Box->SetBackgroundColour( bgCl );
          if( Box->GetPopupControl() != NULL )
            Box->GetPopupControl()->GetControl()->SetBackgroundColour( bgCl );
          if( Box->GetTextCtrl() != NULL )
            Box->GetTextCtrl()->SetBackgroundColour( bgCl );
        }
      }  
      else  {
        if( !fg.IsEmpty() )  win->SetForegroundColour( wxColor( fg.u_str() ) );
        if( !bg.IsEmpty() )  win->SetBackgroundColour( wxColor( bg.u_str() ) );
      }
    }
  }
}
//..............................................................................
void THtml::TObjectsState::SaveToFile(const olxstr& fn)  {
}
//..............................................................................
bool THtml::TObjectsState::LoadFromFile(const olxstr& fn)  {
  return true;
}
//..............................................................................
TSStrStrList<olxstr,false>* THtml::TObjectsState::DefineControl(const olxstr& name, const std::type_info& type) {
  int ind = Objects.IndexOf( name );
  if( ind != -1 )
    throw TFunctionFailedException(__OlxSourceInfo, "object already exists");
  TSStrStrList<olxstr,false>* props = new TSStrStrList<olxstr,false>;

  props->Add("type", type.name());  // type
  if( type == typeid(TTextEdit) )  {  
    props->Add("val");
  }
  else if( type == typeid(TCheckBox) )  {  
    props->Add("val");
    props->Add("checked");
  }
  else if( type == typeid(TTrackBar) || type == typeid(TSpinCtrl) )  {  
    props->Add("min");
    props->Add("max");
    props->Add("val");
  }
  else if( type == typeid(TButton) )    {  
    props->Add("down");
    props->Add("val");
  }
  else if( type == typeid(TBmpButton) )    {  
    props->Add("down");
    props->Add("val");
  }
  else if( type == typeid(TComboBox) )  {  
    props->Add("val");
    props->Add("items");
  }
  else if( type == typeid(TListBox) )  {  
    props->Add("val");
    props->Add("items");
  }
  else if( type == typeid(TTreeView) )  {  
  }
  else if( type == typeid(TLabel) )  {  
    props->Add("val");
  }
  else //?
    ;
  props->Add("fg");
  props->Add("bg");

  Objects.Add(name, props);

  return props;
}
//..............................................................................


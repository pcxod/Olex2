//----------------------------------------------------------------------------//
// material properties dialog
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "matprop.h"

#include "wx/colordlg.h"
#include "wx/fontdlg.h"

#include "gpcollection.h"
#include "glgroup.h"
#include "xatom.h"
//..............................................................................
enum  {
  ID_COPY = 1,
  ID_PASTE,
  ID_EDITFONT
};
//..............................................................................
BEGIN_EVENT_TABLE(TdlgMatProp, TDialog)
  EVT_BUTTON(wxID_OK, TdlgMatProp::OnOK)
  EVT_BUTTON(ID_COPY, TdlgMatProp::OnCopy)
  EVT_BUTTON(ID_PASTE, TdlgMatProp::OnPaste)
  EVT_BUTTON(ID_EDITFONT, TdlgMatProp::OnEditFont)
END_EVENT_TABLE()
//..............................................................................
TGlMaterial TdlgMatProp::MaterialCopy;
//..............................................................................
TdlgMatProp::TdlgMatProp(TMainFrame *ParentFrame, TGPCollection *GPC, TGXApp *XApp) :
  TDialog(ParentFrame, wxT("Material Parameters"), wxT("dlgMatProp"))
{
  AActionHandler::SetToDelete(false);
  bEditFont = NULL;
  if( GPC != NULL && GPC->ObjectCount() == 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "empty collection");
  short Border = 3, SpW = 40;
  GPCollection = GPC;
  FXApp = XApp;
  if( GPC != NULL )  {
    if( EsdlInstanceOf( GPC->GetObject(0), TXAtom) && FXApp->GetSelection().Count() > 1 )  {
      FAtom = &(TXAtom&)GPC->GetObject(0);
      cbApplyToGroup = new wxCheckBox(this, -1, wxT("Apply to All"), wxDefaultPosition, wxDefaultSize);
      cbApplyToGroup->SetValue(true);
    }
    else  {
      FAtom = NULL;
      cbApplyToGroup = NULL;
    }
  }
  else  {
    FAtom = NULL;
    cbApplyToGroup = NULL;
  }

  if( GPC != NULL && GPC->PrimitiveCount() )  {
    cbPrimitives = new TComboBox(this);
    FMaterials = new TGlMaterial[GPC->PrimitiveCount()+1];
    for( size_t i=0; i < GPC->PrimitiveCount(); i++ )  {
      TGlPrimitive& GlP = GPC->GetPrimitive(i);
      cbPrimitives->AddObject(GlP.GetName(), &GlP);
      FMaterials[i] = GlP.GetProperties();
    }
    cbPrimitives->SetValue(GPC->GetPrimitive(0).GetName().u_str());
    cbPrimitives->OnChange.Add(this);
    FCurrentMaterial = 0;
  }
  else  {
    cbPrimitives = NULL;
    FMaterials = new TGlMaterial[1];
    if( GPC != NULL && EsdlInstanceOf(GPC->GetObject(0), TGlGroup) )  {
      FMaterials[0] = ((TGlGroup&)GPC->GetObject(0)).GetGlM();
    }
    FCurrentMaterial = 0;
  }
  long flags = 0;wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER;
  cbAmbF = new wxCheckBox(this, -1, wxT("Ambient Front"), wxDefaultPosition, wxDefaultSize, flags);
  scAmbF = new TSpinCtrl(this);  scAmbF->WI.SetWidth(SpW);  scAmbF->SetRange(0, 100);
  cbAmbB = new wxCheckBox(this, -1, wxT("Ambient Back"), wxDefaultPosition, wxDefaultSize, flags);
  scAmbB = new TSpinCtrl(this);  scAmbB->WI.SetWidth(SpW);  scAmbB->SetRange(0, 100);
  cbDiffF = new wxCheckBox(this, -1, wxT("Diffuse Front"), wxDefaultPosition, wxDefaultSize, flags);
  scDiffF = new TSpinCtrl(this);  scDiffF->WI.SetWidth(SpW);  scDiffF->SetRange(0, 100);
  cbDiffB = new wxCheckBox(this, -1, wxT("Diffuse Back"), wxDefaultPosition, wxDefaultSize, flags);
  scDiffB = new TSpinCtrl(this);  scDiffB->WI.SetWidth(SpW);  scDiffB->SetRange(0, 100);
  cbEmmF = new wxCheckBox(this, -1, wxT("Emission Front"), wxDefaultPosition, wxDefaultSize, flags);
  scEmmF = new TSpinCtrl(this);  scEmmF->WI.SetWidth(SpW);  scEmmF->SetRange(0, 100);
  cbEmmB = new wxCheckBox(this, -1, wxT("Emission Back"), wxDefaultPosition, wxDefaultSize, flags);
  scEmmB = new TSpinCtrl(this);  scEmmB->WI.SetWidth(SpW);  scEmmB->SetRange(0, 100);
  cbSpecF = new wxCheckBox(this, -1, wxT("Specular Front"), wxDefaultPosition, wxDefaultSize, flags);
  scSpecF = new TSpinCtrl(this);  scSpecF->WI.SetWidth(SpW);  scSpecF->SetRange(0, 100);
  cbSpecB = new wxCheckBox(this, -1, wxT("Specular Back"), wxDefaultPosition, wxDefaultSize, flags);
  scSpecB = new TSpinCtrl(this);  scSpecB->WI.SetWidth(SpW);  scSpecB->SetRange(0, 100);
  cbShnF = new wxCheckBox(this, -1, wxT("Shininess Front"), wxDefaultPosition, wxDefaultSize, flags);
  cbShnB = new wxCheckBox(this, -1, wxT("Shininess Back"), wxDefaultPosition, wxDefaultSize, flags);

  SpinCtrls.Add(scAmbF);   SpinCtrls.Add(scAmbB);
  SpinCtrls.Add(scDiffF);  SpinCtrls.Add(scDiffB);
  SpinCtrls.Add(scEmmF);   SpinCtrls.Add(scEmmB);
  SpinCtrls.Add(scSpecF);  SpinCtrls.Add(scSpecB);

  cbTrans = new wxCheckBox(this, -1, wxT("Translucent"), wxDefaultPosition, wxDefaultSize);
  scTrans = new TSpinCtrl(this);  scTrans->SetRange(5, 95);  scTrans->OnChange.Add(this);
  cbIDraw = new wxCheckBox(this, -1, wxT("Identity Draw"), wxDefaultPosition, wxDefaultSize);

  tcAmbF = new TTextEdit(this);  tcAmbF->SetReadOnly(true);  tcAmbF->OnClick.Add(this);
  tcAmbB = new TTextEdit(this);  tcAmbB->SetReadOnly(true);  tcAmbB->OnClick.Add(this);
  tcDiffF = new TTextEdit(this); tcDiffF->SetReadOnly(true); tcDiffF->OnClick.Add(this);
  tcDiffB = new TTextEdit(this); tcDiffB->SetReadOnly(true); tcDiffB->OnClick.Add(this);
  tcEmmF = new TTextEdit(this);  tcEmmF->SetReadOnly(true);  tcEmmF->OnClick.Add(this);
  tcEmmB = new TTextEdit(this);  tcEmmB->SetReadOnly(true);  tcEmmB->OnClick.Add(this);
  tcSpecF = new TTextEdit(this); tcSpecF->SetReadOnly(true); tcSpecF->OnClick.Add(this);
  tcSpecB = new TTextEdit(this); tcSpecB->SetReadOnly(true); tcSpecB->OnClick.Add(this);
  tcShnF = new TTextEdit(this);  tcShnF->SetReadOnly(false); 
  tcShnB = new TTextEdit(this);  tcShnB->SetReadOnly(false); 

  wxBoxSizer *Sizer0 = NULL;
  if( cbPrimitives )  {
    Sizer0 = new wxBoxSizer(wxHORIZONTAL);
    Sizer0->Add(cbPrimitives, 0, wxEXPAND | wxALL, Border);
    if( cbApplyToGroup )
      Sizer0->Add(cbApplyToGroup, 0, wxEXPAND | wxALL, Border);
    Sizer0->Add(new wxButton(this, ID_COPY, wxT("Copy  Mat.")), 1, wxEXPAND | wxALL, Border);
    Sizer0->Add(new wxButton(this, ID_PASTE, wxT("Paste Mat.")), 1, wxEXPAND | wxALL, Border);
    bEditFont = new wxButton(this, ID_EDITFONT, wxT("Edit Font"));
    bEditFont->Enable(GPC->GetPrimitive(0).GetFont() != NULL);
    Sizer0->Add(bEditFont, 1, wxEXPAND | wxALL, Border);
  }

  wxFlexGridSizer *grid = new wxFlexGridSizer( 6, 8);
  grid->Add(-1,10);
  grid->Add(new wxStaticText(this, -1, wxT("Colour"), wxDefaultPosition), 0, wxALIGN_CENTRE | wxEXPAND | wxALL, Border);
  grid->Add(new wxStaticText(this, -1, wxT("Transparency"), wxDefaultPosition), 0, wxALIGN_CENTRE | wxEXPAND | wxALL, Border);
  grid->Add(-1,10);
  grid->Add(new wxStaticText(this, -1, wxT("Colour"), wxDefaultPosition), 0, wxALIGN_CENTRE | wxEXPAND | wxALL, Border);
  grid->Add(new wxStaticText(this, -1, wxT("Transparency"), wxDefaultPosition), 0, wxALIGN_CENTRE | wxEXPAND | wxALL, Border);

  grid->Add(cbAmbF, 0, wxALL, Border);
  grid->Add(tcAmbF, 0, wxEXPAND | wxALL, Border);
  grid->Add(scAmbF, 0, wxEXPAND | wxALL, Border);
  grid->Add(cbAmbB, 0, wxALL, Border);
  grid->Add(tcAmbB, 0, wxEXPAND | wxALL, Border);
  grid->Add(scAmbB, 0, wxEXPAND | wxALL, Border);

  grid->Add(cbDiffF, 0, wxALL, Border);
  grid->Add(tcDiffF, 0, wxEXPAND | wxALL, Border);
  grid->Add(scDiffF, 0, wxEXPAND | wxALL, Border);
  grid->Add(cbDiffB, 0, wxALL, Border);
  grid->Add(tcDiffB, 0, wxEXPAND | wxALL, Border);
  grid->Add(scDiffB, 0, wxEXPAND | wxALL, Border);

  grid->Add(cbEmmF, 0, wxALL, Border);
  grid->Add(tcEmmF, 0, wxEXPAND | wxALL, Border);
  grid->Add(scEmmF, 0, wxEXPAND | wxALL, Border);
  grid->Add(cbEmmB, 0, wxALL, Border);
  grid->Add(tcEmmB, 0, wxEXPAND | wxALL, Border);
  grid->Add(scEmmB, 0, wxEXPAND | wxALL, Border);

  grid->Add(cbSpecF, 0, wxALL, Border);
  grid->Add(tcSpecF, 0, wxEXPAND | wxALL, Border);
  grid->Add(scSpecF, 0, wxEXPAND | wxALL, Border);
  grid->Add(cbSpecB, 0, wxALL, Border);
  grid->Add(tcSpecB, 0, wxEXPAND | wxALL, Border);
  grid->Add(scSpecB, 0, wxEXPAND | wxALL, Border);

  grid->Add(-1,10);
  grid->Add(-1,10);
  grid->Add(-1,10);
  grid->Add(-1,10);
  grid->Add(-1,10);
  grid->Add(-1,10);

  grid->Add(cbShnF, 0, wxALL, Border);
  grid->Add(tcShnF, 0, wxEXPAND | wxALL, Border);
  grid->Add(-1,10);
  grid->Add(cbShnB, 0, wxALL, Border);
  grid->Add(tcShnB, 0, wxEXPAND | wxALL, Border);
  grid->Add(-1,10);
  
  grid->Add(cbTrans, 0, wxALL, Border);
  grid->Add(scTrans, 0, wxEXPAND | wxALL, Border);
  grid->Add(-1,10);
  grid->Add(cbIDraw, 0, wxALL, Border);
  grid->AddGrowableCol(1);
  grid->AddGrowableCol(2);
  grid->AddGrowableCol(4);
  grid->AddGrowableCol(5);
  
  wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL);
  ButtonsSizer->Add(new wxButton( this, wxID_OK, wxT("OK") ), 0, wxEXPAND | wxALL, Border);
  ButtonsSizer->Add(new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxEXPAND | wxALL, Border);
  ButtonsSizer->Add(new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxEXPAND | wxALL, Border);

  wxBoxSizer *TopSiser = new wxBoxSizer( wxVERTICAL);
  if( Sizer0 )  TopSiser->Add(Sizer0, 0, wxEXPAND | wxALL, 5);
  TopSiser->Add(grid, 0, wxEXPAND | wxALL, 5);
  TopSiser->Add(ButtonsSizer, 0, wxALL, 10);

  SetSizer(TopSiser);      // use the sizer for layout

  TopSiser->SetSizeHints(this);   // set size hints to honour minimum size

  Center();

  Init(FMaterials[0]);
}
//..............................................................................
TdlgMatProp::~TdlgMatProp()  {
  delete [] FMaterials;
  if( cbPrimitives != NULL )  
    cbPrimitives->OnChange.Clear();
  scTrans->OnChange.Clear();
  tcAmbF->OnClick.Clear();
  tcAmbB->OnClick.Clear();
  tcDiffF->OnClick.Clear();
  tcDiffB->OnClick.Clear();
  tcEmmF->OnClick.Clear();
  tcEmmB->OnClick.Clear();
  tcSpecF->OnClick.Clear();
  tcSpecB->OnClick.Clear();
}
//..............................................................................
bool TdlgMatProp::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( EsdlInstanceOf( *Sender, TTextEdit) )  {
    wxColourDialog *CD = new wxColourDialog(this);
    wxColor wc = ((TTextEdit*)Sender)->GetBackgroundColour();
    CD->GetColourData().SetColour(wc);
    if( CD->ShowModal() == wxID_OK )  {
      wc = CD->GetColourData().GetColour();
      ((TTextEdit*)Sender)->WI.SetColor(RGB(wc.Red(), wc.Green(), wc.Blue()));
    }
    delete CD;
  }
  if( (TComboBox*)Sender == cbPrimitives )  {
    Update(FMaterials[FCurrentMaterial]);
    int i = cbPrimitives->FindString(cbPrimitives->GetValue());
    if( i >= 0 )  {
      FCurrentMaterial = i;
      Init(FMaterials[FCurrentMaterial]);
      const TGlPrimitive* glp = (const TGlPrimitive*)cbPrimitives->GetObject(i);
      bEditFont->Enable( glp->GetFont() != NULL);
    }
  }
  if( (TSpinCtrl*)Sender == scTrans )  {
    for( size_t i=0; i < SpinCtrls.Count(); i++ ) 
      SpinCtrls[i]->SetValue(scTrans->GetValue());
  }
  return true;
}
//..............................................................................
void TdlgMatProp::Init( const TGlMaterial &Glm )  {
  cbAmbF->SetValue(Glm.HasAmbientF());
  cbAmbB->SetValue(Glm.HasAmbientB());
  scAmbF->SetValue(olx_round(Glm.AmbientF.Data()[3]*100));
  scAmbB->SetValue( (int)Glm.AmbientB.Data()[3]*100);

  cbDiffF->SetValue(Glm.HasDiffuseF());
  cbDiffB->SetValue(Glm.HasDiffuseB());
  scDiffF->SetValue(olx_round(Glm.DiffuseF.Data()[3]*100));
  scDiffB->SetValue(olx_round(Glm.DiffuseB.Data()[3]*100));

  cbEmmF->SetValue(Glm.HasEmissionF());
  cbEmmB->SetValue(Glm.HasEmissionB());
  scEmmF->SetValue(olx_round(Glm.EmissionF.Data()[3]*100));
  scEmmB->SetValue(olx_round(Glm.EmissionB.Data()[3]*100));

  cbSpecF->SetValue(Glm.HasSpecularF());
  cbSpecB->SetValue(Glm.HasSpecularB());
  scSpecF->SetValue(olx_round(Glm.SpecularF.Data()[3]*100));
  scSpecB->SetValue(olx_round(Glm.SpecularB.Data()[3]*100));

  cbShnF->SetValue(Glm.HasShininessF());
  cbShnB->SetValue(Glm.HasShininessB());

  cbTrans->SetValue(Glm.IsTransparent());
  cbIDraw->SetValue(Glm.IsIdentityDraw());

  tcAmbF->WI.SetColor(Glm.AmbientF.GetRGB());
  tcAmbB->WI.SetColor(Glm.AmbientB.GetRGB());

  tcDiffF->WI.SetColor(Glm.DiffuseF.GetRGB());
  tcDiffB->WI.SetColor(Glm.DiffuseB.GetRGB());

  tcEmmF->WI.SetColor(Glm.EmissionF.GetRGB());
  tcEmmB->WI.SetColor(Glm.EmissionB.GetRGB());

  tcSpecF->WI.SetColor(Glm.SpecularF.GetRGB());
  tcSpecB->WI.SetColor(Glm.SpecularB.GetRGB());

  tcShnF->SetValue(olxstr(Glm.ShininessF).u_str());
  tcShnB->SetValue(olxstr(Glm.ShininessB).u_str());
}
//..............................................................................
void TdlgMatProp::Update(TGlMaterial &Glm)  {
  Glm.SetAmbientF(cbAmbF->GetValue());
  Glm.SetAmbientB(cbAmbB->GetValue());

  Glm.SetDiffuseF(cbDiffF->GetValue());
  Glm.SetDiffuseB(cbDiffB->GetValue());

  Glm.SetEmissionF(cbEmmF->GetValue());
  Glm.SetEmissionB(cbEmmB->GetValue());

  Glm.SetSpecularF(cbSpecF->GetValue());
  Glm.SetSpecularB(cbSpecB->GetValue());

  Glm.SetShininessF(cbShnF->GetValue());
  Glm.SetShininessB(cbShnB->GetValue());

  Glm.SetTransparent(cbTrans->GetValue());
  Glm.SetIdentityDraw(cbIDraw->GetValue());

  Glm.AmbientF = tcAmbF->WI.GetColor();
  Glm.AmbientF[3] = (double)scAmbF->GetValue()/100;
  Glm.AmbientB = tcAmbB->WI.GetColor();
  Glm.AmbientB[3] = (double)scAmbB->GetValue()/100;

  Glm.DiffuseF = tcDiffF->WI.GetColor();
  Glm.DiffuseF[3] = (double)scDiffF->GetValue()/100;
  Glm.DiffuseB = tcDiffB->WI.GetColor();
  Glm.DiffuseB[3] = (double)scDiffB->GetValue()/100;

  Glm.EmissionF = tcEmmF->WI.GetColor();
  Glm.EmissionF[3] = (double)scEmmF->GetValue()/100;
  Glm.EmissionB = tcEmmB->WI.GetColor();
  Glm.EmissionB[3] = (double)scEmmB->GetValue()/100;

  Glm.SpecularF = tcSpecF->WI.GetColor();
  Glm.SpecularF[3] = (double)scSpecF->GetValue()/100;
  Glm.SpecularB = tcSpecB->WI.GetColor();
  Glm.SpecularB[3] = (double)scSpecB->GetValue()/100;

  Glm.ShininessF = olxstr(tcShnF->GetValue().c_str()).ToInt();
  Glm.ShininessB = olxstr(tcShnB->GetValue().c_str()).ToInt();
}
//..............................................................................
void TdlgMatProp::OnOK(wxCommandEvent& event)  {
  Update(FMaterials[FCurrentMaterial]);
  if( FAtom != NULL && cbApplyToGroup->GetValue() )  {
    TGlGroup& gl = FXApp->GetSelection();
    TGPCollection* ogpc = &FAtom->GetPrimitives();
    SortedPtrList<TGPCollection, TPointerPtrComparator> uniqCol;
    for( size_t i=0; i < gl.Count(); i++ )
      if( EsdlInstanceOf(gl.GetObject(i), TXAtom) )  {
        uniqCol.AddUnique(&gl.GetObject(i).GetPrimitives());
    }
    for( size_t i=0; i < uniqCol.Count(); i++ )  {
      if( uniqCol[i] != ogpc )  {
        for( size_t j=0; j < ogpc->PrimitiveCount(); j++ )  {
          TGlPrimitive* glp = uniqCol[i]->FindPrimitiveByName(ogpc->GetPrimitive(j).GetName());
          if( glp != NULL )  {
            glp->SetProperties(FMaterials[j]);
            uniqCol[i]->GetStyle().SetMaterial(glp->GetName(), FMaterials[j]);
          }
        }
      }
      else  {
        for( size_t j=0; j < ogpc->PrimitiveCount(); j++ )  {
          ogpc->GetPrimitive(j).SetProperties(FMaterials[j]);
          ogpc->GetStyle().SetMaterial(ogpc->GetPrimitive(j).GetName(), FMaterials[j]);
        }
      }
    }
  }
  else  {
    if( GPCollection != NULL && EsdlInstanceOf(GPCollection->GetObject(0), TGlGroup) )  {
      ((TGlGroup&)GPCollection->GetObject(0)).SetGlM(FMaterials[0]);
    }
    else  {
      if( GPCollection != NULL )  {
        //TGraphicsStyle* GS = FXApp->GetRender().Styles()->NewStyle( GPCollection->Name(), true);
        for( size_t i=0; i < GPCollection->PrimitiveCount(); i++ )  {
          GPCollection->GetPrimitive(i).SetProperties(FMaterials[i]);
          GPCollection->GetStyle().SetMaterial(GPCollection->GetPrimitive(i).GetName(), FMaterials[i]);
        }
      }
    }
  }
//  FDrawObject->UpdatePrimitives(24);
  EndModal(wxID_OK);
}
//..............................................................................
void TdlgMatProp::OnCopy(wxCommandEvent& event)  {
  Update(FMaterials[FCurrentMaterial]);
  MaterialCopy = FMaterials[FCurrentMaterial];
}
//..............................................................................
void TdlgMatProp::OnPaste(wxCommandEvent& event)  {
//  FMaterials[FCurrentMaterial] = MaterialCopy;
  Init(MaterialCopy);
}
//..............................................................................
void TdlgMatProp::OnEditFont(wxCommandEvent& event)  {
  FXApp->GetRender().GetScene().ShowFontDialog( GPCollection->GetPrimitive(FCurrentMaterial).GetFont());
}
//..............................................................................


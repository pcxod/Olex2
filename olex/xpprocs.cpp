/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "wx/protocol/http.h"

#include "mainform.h"
#include "xglcanv.h"
#include "xglapp.h"

#include "wx/clipbrd.h"
#include "wx/filesys.h"
#include "wx/cursor.h"
#include "wx/colordlg.h"
#include "wx/fontdlg.h"
#include "wx/progdlg.h"

#include "wx/image.h"
#include "wx/dcps.h"
#include "evaln.h"
#include "imagep.h"

#include "dgrad.h"
#include "edit.h"
#include "updateoptions.h"
#include "ptable.h"

#include "symmlib.h"
#include "bitarray.h"
#include "glbackground.h"
#include "glgroup.h"
#include "gpcollection.h"

#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "xline.h"
#include "xreflection.h"
#include "gllabel.h"
#include "gllabels.h"
#include "dunitcell.h"
#include "dbasis.h"
#include "xgroup.h"

#include "symmparser.h"

#include "efile.h"

#include "ins.h"
#include "cif.h"
#include "hkl.h"
#include "p4p.h"
#include "mol.h"
#include "crs.h"
#include "xyz.h"

#include "fsext.h"
#include "html/htmlmanager.h"

#include <iostream>

#include "pyext.h"

#include "ellipsoid.h"

#include "glbitmap.h"
#include "bitarray.h"
#include "arrays.h"
#include "sgtest.h"
#include "network.h"

#include "filesystem.h"
#include "cdsfs.h"
#include "wxzipfs.h"

#include "tls.h"

#include "cmdline.h"

#include "xlcongen.h"

#include "arrays.h"
#include "estrbuffer.h"
#include "unitcell.h"
#include "xgrid.h"
#include "symmtest.h"
#include "xlattice.h"

#include "matprop.h"
#include "utf8file.h"

#include "msgbox.h"

#include "shellutil.h"

#ifndef __BORLANDC__
  #include "ebtree.h"
#endif

#include "dusero.h"

#ifdef __GNUC__
  #ifdef Bool
    #undef Bool
  #endif
  #ifdef Success
    #undef Success
  #endif
#endif

#include "olxmps.h"
#include "maputil.h"

#include "olxvar.h"

#include "atomref.h"
#include "wxglscene.h"
#include "equeue.h"
#include "xmacro.h"
#include "vcov.h"

#include "sfutil.h"
#include "ortdraw.h"
#include "ortdrawtex.h"
#include "catomlist.h"
#include "updateapi.h"
#include "planesort.h"
// FOR DEBUG only
#include "egraph.h"
#include "olxth.h"
#include "md5.h"
#include "sha.h"
#include "exparse/expbuilder.h"
#include "encodings.h"
#include "cifdp.h"
#include "glutil.h"
#include "refutil.h"
#include "absorpc.h"
#include "file_filter.h"
#include "dsphere.h"
#include "patchapi.h"
#include "analysis.h"
#include "label_corrector.h"
//#include "gl2ps/gl2ps.c"

static const olxstr StartMatchCBName("startmatch");
static const olxstr OnMatchCBName("onmatch");
static const olxstr OnModeChangeCBName("modechange");
static const olxstr NoneString("none");

int CalcL( int v )  {
  int r = 0;
  while( (v/=2) > 2 )  r++;
  return r+2;
}

//olex::IBasicMacroProcessor *olex::OlexPort::MacroProcessor;

//..............................................................................
void TMainForm::funFileLast(const TStrObjList& Params, TMacroError &E)  {
  size_t index = 0;
  if( FXApp->XFile().HasLastLoader() )  index = 1;
  if( FRecentFiles.Count() <= index )  {
    E.ProcessingError(__OlxSrcInfo, "no last file");
    return;
  }
  if( !TEFile::Exists(FRecentFiles[index]) )  {
    E.ProcessingError(__OlxSrcInfo, "file does not exists anymore");
    return;
  }
  E.SetRetVal(FRecentFiles[index]);
}
//..............................................................................
void TMainForm::funCell(const TStrObjList& Params, TMacroError &E)  {
  if( Params[0].IsNumber() && false )  {  // mutliplies cartesian cell... tests...
    const double k = Params[0].ToDouble();
    TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
    au.GetAxes() *= k;
    au.InitMatrices();
    for( size_t i=0; i < au.AtomCount(); i++ )
      au.CartesianToCell(au.GetAtom(i).ccrd());
    Macros.ProcessMacro("fuse", E);
  }
  else  {
    if( Params[0].Equalsi('a') )
      E.SetRetVal(FXApp->XFile().GetAsymmUnit().GetAxes()[0]);
    else if( Params[0].Equalsi('b') )
      E.SetRetVal(FXApp->XFile().GetAsymmUnit().GetAxes()[1]);
    else if( Params[0].Equalsi('c') )
      E.SetRetVal(FXApp->XFile().GetAsymmUnit().GetAxes()[2]);
    else if( Params[0].Equalsi("alpha") )
      E.SetRetVal(FXApp->XFile().GetAsymmUnit().GetAngles()[0]);
    else if( Params[0].Equalsi("beta") )
      E.SetRetVal(FXApp->XFile().GetAsymmUnit().GetAngles()[1]);
    else if( Params[0].Equalsi("gamma") )
      E.SetRetVal(FXApp->XFile().GetAsymmUnit().GetAngles()[2]);
    else if( Params[0].Equalsi("volume") )
      E.SetRetVal(olxstr::FormatFloat(2, FXApp->XFile().GetUnitCell().CalcVolume()));
    else
      E.ProcessingError(__OlxSrcInfo, "invalid argument: ") << Params[0];
  }
}
//..............................................................................
void TMainForm::funCif(const TStrObjList& Params, TMacroError &E)  {
  TCif& cf = FXApp->XFile().GetLastLoader<TCif>();
  if( cf.ParamExists(Params[0]) )
    E.SetRetVal(cf.GetParamAsString(Params[0]));
  else
    E.SetRetVal(XLibMacros::NAString);
}
//..............................................................................
void TMainForm::funP4p(const TStrObjList& Params, TMacroError &E)  {
}
//..............................................................................
void TMainForm::funHasGUI(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( true );
}
//..............................................................................
void TMainForm::funExtraZoom(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( FXApp->GetExtraZoom() );
  else
    FXApp->SetExtraZoom( Params[0].ToDouble() );
}
//..............................................................................
void TMainForm::funIsOS(const TStrObjList& Params, TMacroError &E)  {
#if defined(__WXMSW__)
  E.SetRetVal( Params[0].Equalsi("win") );
#elif defined(__WXMAC__) || defined(__MAC__)
  E.SetRetVal( Params[0].Equalsi("mac") );
#elif defined(__WXGTK__)
  E.SetRetVal(Params[0].Equalsi("linux") );
#else
  E.SetRetVal(false);
#endif
}
//..............................................................................
void TMainForm::funCrs(const TStrObjList& Params, TMacroError &E)  {
  TCRSFile& cf = FXApp->XFile().GetLastLoader<TCRSFile>();
  if( Params[0].Equalsi("sg") )
    E.SetRetVal(cf.GetSG() != NULL ? cf.GetSG()->GetName() : EmptyString());
  else
    E.SetRetVal(XLibMacros::NAString);
}
//..............................................................................
void TMainForm::funStrcat(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(Params[0] + Params[1]);
}
//..............................................................................
void TMainForm::funStrcmp(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(Params[0] == Params[1]);
}
//..............................................................................
void TMainForm::funGetEnv(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  {
#if defined(__WIN32__) && defined(_UNICODE) && defined(_MSC_VER)
    if( _wenviron != NULL )  {
      for( size_t i=0; _wenviron[i] != NULL; i++ )
        TBasicApp::NewLogEntry() << _wenviron[i];
    }
#else
    extern char **environ;
    if( environ != NULL )  {
      for( size_t i=0; environ[i] != NULL; i++ )
        TBasicApp::NewLogEntry() << environ[i];
    }
#endif
  }
  else
    E.SetRetVal(olx_getenv(Params[0]));
}
//..............................................................................
void TMainForm::funFileSave(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(PickFile(Params[0], Params[1], Params[2], false));
}
//..............................................................................
void TMainForm::funFileOpen(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(PickFile(Params[0], Params[1], Params[2], true));
}
//..............................................................................
//..............................................................................
void TMainForm::funVVol(const TStrObjList& Params, TMacroError &E)  {
  ElementRadii radii;
  if( Params.Count() == 1 && TEFile::Exists(Params[0]) )
    radii = TXApp::ReadVdWRadii(Params[0]);
  TXApp::PrintVdWRadii(radii, FXApp->XFile().GetAsymmUnit().GetContentList());

  const TXApp::CalcVolumeInfo vi = FXApp->CalcVolume(&radii);
  olxstr report;
  E.SetRetVal(olxstr::FormatFloat(2, vi.total-vi.overlapping));
  TBasicApp::NewLogEntry(logWarning) << "Please note that this is a highly approximate procedure."
  " Volume of current fragment is calculated using a maximum two overlaping spheres," 
  " to calculate packing indexes, use calcvoid or MolInfo instead";
  
  TBasicApp::NewLogEntry() << "Molecular volume (A): " << olxstr::FormatFloat(2, vi.total-vi.overlapping);
  TBasicApp::NewLogEntry() << "Overlapping volume (A): " << olxstr::FormatFloat(2, vi.overlapping);
}
//..............................................................................
void TMainForm::funSel(const TStrObjList& Params, TMacroError &E)  {
  TSAtomPList atoms;
  TGlGroup& sel = FXApp->GetSelection();
  for( size_t i=0; i < sel.Count(); i++ )  {
    AGDrawObject& gdo = sel[i];
    if( EsdlInstanceOf(gdo, TXAtom) )
      atoms.Add(((TXAtom&)gdo));
    else if( EsdlInstanceOf(gdo, TXBond) )  {
      atoms.Add(((TXBond&)gdo).A());
      atoms.Add(((TXBond&)gdo).B());
    }
    else if( EsdlInstanceOf(gdo, TXPlane) )  {
      TSPlane& sp = ((TXPlane&)gdo);
      for( size_t j=0; j < sp.Count(); j++ )
        atoms.Add(sp.GetAtom(j));
    }
  }
  olxstr tmp;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    tmp << atoms[i]->GetLabel();
    if( atoms[i]->CAtom().GetResiId() != 0 )  {
      tmp << '_' <<
        atoms[i]->CAtom().GetParent()->GetResidue(atoms[i]->CAtom().GetResiId()).GetNumber();
    }
    if( (i+1) < atoms.Count() )
      tmp << ' ';
  }
  E.SetRetVal(tmp);
}
//..............................................................................
void TMainForm::funAtoms(const TStrObjList& Params, TMacroError &E)
{
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::funEnv(const TStrObjList& Params, TMacroError &E)  {
  TXAtom *XA = FXApp->GetXAtom(Params[0], false);
  if( XA == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom: ") << Params[0];
    return;
  }
  olxstr tmp;
  for( size_t i=0; i < XA->NodeCount(); i++ )
    tmp << XA->Node(i).GetLabel() << ' ';
  E.SetRetVal(tmp);
}
//..............................................................................
void TMainForm::funFPS(const TStrObjList& Params, TMacroError &E) {
  TimePerFrame = 0;
  for( int i=0; i < 10; i++ )
    TimePerFrame += FXApp->Draw();
  if( TimePerFrame != 0 )
    E.SetRetVal(10*(1000./TimePerFrame));
}
//..............................................................................
void TMainForm::funCursor(const TStrObjList& Params, TMacroError &E)  {
  if( CursorStack.Count() > 10 )  {
    CursorStack.Clear();
    TBasicApp::NewLogEntry(logError) << "Cursor stack size limit reached and cleared";
  }
  if( Params.IsEmpty() )  {
    if( !CursorStack.IsEmpty() )  {
      AnAssociation2<wxCursor,wxString> ci = CursorStack.Pop();
      SetCursor(ci.GetA());
      FGlCanvas->SetCursor(ci.GetA());
      SetStatusText(ci.GetB());
    }
    else  {
      wxCursor cr(wxCURSOR_ARROW);
      SetCursor(cr);
      FGlCanvas->SetCursor(cr);
      SetStatusText(wxT(""));
    }
  }
  else  {
    if (!Params[0].Equalsi("user")) {
      CursorStack.Push(AnAssociation2<wxCursor,wxString>(
        FGlCanvas->GetCursor(), GetStatusBar()->GetStatusText()));
    }
    if( Params[0].Equalsi("busy") )  {
      wxCursor cr(wxCURSOR_WAIT);
      SetCursor(cr);
      FGlCanvas->SetCursor(cr);
      if( Params.Count() == 2 )
        SetStatusText(Params[1].u_str());
    }
    else if( Params[0].Equalsi("brush") )  {
      wxCursor cr(wxCURSOR_PAINT_BRUSH);
      SetCursor(cr);
      FGlCanvas->SetCursor(cr);
    }
    else if( Params[0].Equalsi("hand") )  {
      wxCursor cr(wxCURSOR_HAND);
      SetCursor(cr);
      FGlCanvas->SetCursor(cr);
    }
    else if( Params[0].Equalsi("arrow") )  {
      wxCursor cr(wxCURSOR_ARROW);
      SetCursor(cr);
      FGlCanvas->SetCursor(cr);
    }
    else if (Params[0].Equalsi("user") && Params.Count() == 3)  {
      SetUserCursor(Params[1], Params[2]);
    }
    else  {
      if( TEFile::Exists(Params[0]) )  {
        wxImage img;
        img.LoadFile(Params[0].u_str());
        img.SetMaskColour(254, 254, 254);
        img.SetMask(true);
        wxCursor cr(img);
        SetCursor(cr);
        FGlCanvas->SetCursor(cr);
      }
    }
  }
}
//..............................................................................
void TMainForm::funRGB(const TStrObjList& Params, TMacroError &E)  {
  if( Params.Count() == 3 )  {
    E.SetRetVal((uint32_t)OLX_RGB(Params[0].ToInt(), Params[1].ToInt(),
      Params[2].ToInt()));
    return;
  }
  if( Params.Count() == 4 )  {
    E.SetRetVal((uint32_t)OLX_RGBA(Params[0].ToInt(), Params[1].ToInt(),
      Params[2].ToInt(), Params[3].ToInt()));
    return;
  }
}
//..............................................................................
void TMainForm::funHtmlPanelWidth(const TStrObjList &Cmds, TMacroError &E)  {
  if( HtmlManager.main == NULL || FHtmlMinimized )
    E.SetRetVal(olxstr("-1"));
  else
    E.SetRetVal(HtmlManager.main->WI.GetWidth());
}
//..............................................................................
void TMainForm::funColor(const TStrObjList& Params, TMacroError &E)  {
  wxColourDialog CD(this);
  wxColor wc;
  if( Params.Count() == 2 )  {
     wxColor wxdef(Params[1].u_str());
     wc = wxdef;
  }
  CD.GetColourData().SetColour(wc);
  if( CD.ShowModal() == wxID_OK )  {
    wc = CD.GetColourData().GetColour();
    if( Params.IsEmpty() )  {
      E.SetRetVal( (uint32_t)OLX_RGB(wc.Red(), wc.Green(), wc.Blue()) );
    }
    else if( Params[0].Equalsi("hex") )  {
      olx_array_ptr<char> p(olx_malloc<char>(8));
      sprintf(p(), "#%.2x%.2x%.2x", wc.Red(), wc.Green(), wc.Blue());
      E.SetRetVal<olxstr>(p());
    }
  }
  else
    E.ProcessingError(__OlxSrcInfo, EmptyString());
}
//..............................................................................
void TMainForm::funZoom(const TStrObjList &Cmds, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    E.SetRetVal(FXApp->GetRender().GetZoom());
  else  {
    double zoom = FXApp->GetRender().GetZoom() + Cmds[0].ToDouble();
    if( zoom < 0.001 )  zoom = 0.001;
    FXApp->GetRender().SetZoom(zoom);
  }
}
#ifdef __WIN32__
//..............................................................................
void TMainForm::funLoadDll(const TStrObjList &Cmds, TMacroError &E)  {
  if( !TEFile::Exists( Cmds[0] ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate specified file" );
    return;
  }
  HINSTANCE lib_inst = LoadLibrary(Cmds[0].u_str());
  if( !lib_inst )  {
    E.ProcessingError(__OlxSrcInfo, "could not load the library" );
    return;
  }
  FARPROC initPoint = GetProcAddress(lib_inst, "@olex@OlexPort@GetOlexRunnable$qv");
  if( !initPoint )
    initPoint = GetProcAddress(lib_inst, "?GetOlexRunnable@OlexPort@olex@@SGPAVIOlexRunnable@2@XZ");
  if( !initPoint )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate initialisation point" );
    FreeLibrary( lib_inst );
    return;
  }
  typedef olex::IOlexRunnable* (*GetOlexRunnable)();
  olex::IOlexRunnable* runnable = ((GetOlexRunnable)initPoint)();

  if( !runnable )  {
    E.ProcessingError(__OlxSrcInfo, "NULL runnable" );
    FreeLibrary( lib_inst );
    return;
  }
  runnable->Run( *this );
  //FreeLibrary( lib_inst );
}
#endif // __WIN32__
//..............................................................................
void TMainForm::macBasis(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
// the events are handled in void TMainForm::CellVChange()
  FXApp->SetBasisVisible(Cmds.IsEmpty() ? !FXApp->IsBasisVisible() : Cmds[0].ToBool());
}
//..............................................................................
void TMainForm::macLines(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FGlConsole->SetLinesToShow(Cmds[0].ToInt());
}
//..............................................................................
void TMainForm::macPict(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
#ifdef __WIN32__
  bool Emboss = Options.Contains("bw"), 
    EmbossColour = Options.Contains("c"), 
    PictureQuality = Options.Contains("pq");
  if( EmbossColour )  Emboss = true;
  int32_t previous_quality = -1;
  if( PictureQuality )  {
    previous_quality = FXApp->Quality(qaPict);
  }

  bool mask_bg = Options.Contains("nbg");
  uint32_t clear_color = FXApp->GetRender().LightModel.GetClearColor().GetRGB();
  unsigned char bg_r = OLX_GetRValue(clear_color),
    bg_g = OLX_GetGValue(clear_color),
    bg_b = OLX_GetBValue(clear_color);

  short bits = mask_bg ? 32 : 24,
    extraBytes;
  // keep old size values
  const int vpLeft = FXApp->GetRender().GetLeft(),
      vpTop = FXApp->GetRender().GetTop(),
      vpWidth = FXApp->GetRender().GetActualWidth(),
      vpHeight = FXApp->GetRender().GetHeight();

  double res = 2;
  if( Cmds.Count() == 2 && Cmds[1].IsNumber() )
    res = Cmds[1].ToDouble();
  if( res >= 100 )  // width provided
    res /= vpWidth;
  if( res > 10 )
    res = 10;
  if( res <= 0 )  
    res = 1;

  int BmpHeight = vpHeight*res, BmpWidth = vpWidth*res;

  extraBytes = (4-(BmpWidth*(bits/8))%4)%4;

  HDC hDC = wglGetCurrentDC();
  HGLRC glc = wglGetCurrentContext();
  HDC dDC = CreateCompatibleDC(NULL);

  BITMAPFILEHEADER BmpFHdr;
  // intialise bitmap header
  BITMAPINFO BmpInfo;
  BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  BmpInfo.bmiHeader.biWidth = BmpWidth;
  BmpInfo.bmiHeader.biHeight = BmpHeight;
  BmpInfo.bmiHeader.biPlanes = 1;
  BmpInfo.bmiHeader.biBitCount = (WORD) bits;
  BmpInfo.bmiHeader.biCompression = BI_RGB;
  //TODO: +1 check!!!
  BmpInfo.bmiHeader.biSizeImage = (BmpWidth*(bits/8)+extraBytes+1)*BmpHeight;
  BmpInfo.bmiHeader.biXPelsPerMeter = 0;
  BmpInfo.bmiHeader.biYPelsPerMeter = 0;
  BmpInfo.bmiHeader.biClrUsed = 0;
  BmpInfo.bmiHeader.biClrImportant = 0;

  BmpFHdr.bfType = 0x4d42;
  BmpFHdr.bfSize = sizeof(BmpFHdr)  + sizeof(BITMAPINFOHEADER) +
    (BmpWidth*(bits/8)+extraBytes)*BmpHeight;
  BmpFHdr.bfReserved1 = BmpFHdr.bfReserved2 = 0;
  BmpFHdr.bfOffBits = sizeof(BmpFHdr) + sizeof(BmpInfo.bmiHeader);

  unsigned char *DIBits = NULL;
  HBITMAP DIBmp = CreateDIBSection(dDC, &BmpInfo, DIB_RGB_COLORS,
    (void **)&DIBits, NULL, 0);

  SelectObject(dDC, DIBmp);

  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
    PFD_TYPE_RGBA,
    bits, //bits
    0,0,0,0,0,0,
    0,0,
    0,0,0,0,0,
    32,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0,0,
  };
  int PixelFormat = ChoosePixelFormat(dDC, &pfd);
  SetPixelFormat(dDC, PixelFormat, &pfd);
  HGLRC dglc = wglCreateContext(dDC);
  FXApp->GetRender().Resize(0, 0, BmpWidth, BmpHeight, res);
  FXApp->GetRender().BeforeContextChange();
  wglMakeCurrent(dDC, dglc);
  FXApp->GetRender().AfterContextChange();
  FBitmapDraw = true;
  FGlConsole->SetVisible(false);
  FXApp->BeginDrawBitmap(res);
  FXApp->GetRender().EnableFog(FXApp->GetRender().IsFogEnabled());

  FXApp->Draw();
  GdiFlush();
  FBitmapDraw = false;

  FXApp->GetRender().BeforeContextChange();
  wglDeleteContext(dglc);
  DeleteDC(dDC);
  wglMakeCurrent(hDC, glc);
  FXApp->GetRender().AfterContextChange();
  FXApp->GetRender().Resize(vpLeft, vpTop, vpWidth, vpHeight, 1);
  FGlConsole->SetVisible(true);
  FXApp->FinishDrawBitmap();
  if( PictureQuality )  FXApp->Quality(previous_quality);
  FXApp->GetRender().EnableFog(FXApp->GetRender().IsFogEnabled());

  if( Emboss )  {
    if( EmbossColour )  {
      TProcessImage::EmbossC(DIBits, BmpWidth, BmpHeight, 3,
                       FXApp->GetRender().LightModel.GetClearColor().GetRGB());
    }
    else  {
      TProcessImage::EmbossBW(DIBits, BmpWidth, BmpHeight, 3,
                       FXApp->GetRender().LightModel.GetClearColor().GetRGB());
    }
  }
  olxstr bmpFN, outFN;
  if( FXApp->XFile().HasLastLoader() && !TEFile::IsAbsolutePath(Cmds[0]) ) {
    outFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) <<
      TEFile::ExtractFileName( Cmds[0] );
  }
  else
    outFN = Cmds[0];
  // correct a common typo
  if( TEFile::ExtractFileExt(outFN).Equalsi("jpeg") )
    outFN = TEFile::ChangeFileExt(outFN, "jpg");

  if( TEFile::ExtractFileExt(outFN).Equalsi("bmp") )
    bmpFN = TEFile::ChangeFileExt(outFN, "bmp");
  else
    bmpFN = TEFile::ChangeFileExt(outFN, "bmp.tmp");
  TEFile* BmpFile = new TEFile(bmpFN, "w+b");
  BmpFile->Write(&(BmpFHdr), sizeof(BITMAPFILEHEADER));
  BmpFile->Write(&(BmpInfo), sizeof(BITMAPINFOHEADER));
  if (mask_bg) {
    const size_t inc = bits/8,
      ws = BmpWidth*inc;
    for (size_t i=0; i < BmpHeight; i++) {
      for (size_t j=0; j < BmpWidth; j++) {
        size_t off = (BmpWidth*inc+extraBytes)*i + j*inc;
        if (DIBits[off+2]   == bg_r &&
          DIBits[off+1] == bg_g &&
          DIBits[off] == bg_b)
        {
          DIBits[off] = DIBits[off+1] = DIBits[off+2] = ~0;
          DIBits[off+3] = 0;
        }
        else
          DIBits[off+3] = ~0;
      }
    }
  }
  BmpFile->Write(DIBits, (BmpWidth*(bits/8)+extraBytes)*BmpHeight);
  DeleteObject(DIBmp);
  delete BmpFile;
  //check if the image is bmp
  if( TEFile::ExtractFileExt(outFN).Equalsi("bmp") )
    return;
  wxImage image;
  image.LoadFile(bmpFN.u_str(), wxBITMAP_TYPE_BMP);
  if( !image.Ok() )  {
    Error.ProcessingError(__OlxSrcInfo, "could not process image conversion");
    return;
  }
  image.SetOption(wxT("quality"), 85);
  image.SaveFile(outFN.u_str() );
  image.Destroy();
  TEFile::DelFile(bmpFN);
#else
  macPicta(Cmds, Options, Error);
#endif // __WIN32__
}
//..............................................................................
void TMainForm::macPicta(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  int orgHeight = FXApp->GetRender().GetHeight(),
      orgWidth  = FXApp->GetRender().GetWidth();
  double res = 1;
  if( Cmds.Count() == 2 && Cmds[1].IsNumber() )
    res = Cmds[1].ToDouble();
  if( res >= 100 )  // width provided
    res /= orgWidth;
  if( res > 10 )
    res = 10;
  if( res <= 0 )  
    res = 1;
  if( res > 1 && res < 100 )
    res = olx_round(res);

  int SrcHeight = (int)(((double)orgHeight/(res*2)-1.0)*res*2),
      SrcWidth  = (int)(((double)orgWidth/(res*2)-1.0)*res*2);
  int BmpHeight = (int)(SrcHeight*res), BmpWidth = (int)(SrcWidth*res);
  if( BmpHeight < SrcHeight )
    SrcHeight = BmpHeight;
  if( BmpWidth < SrcWidth )
    SrcWidth = BmpWidth;
  FXApp->GetRender().Resize(0, 0, SrcWidth, SrcHeight, res); 
  bool mask_bg = Options.Contains("nbg");
  uint32_t clear_color = FXApp->GetRender().LightModel.GetClearColor().GetRGB();
  unsigned char bg_r = OLX_GetRValue(clear_color),
    bg_g = OLX_GetGValue(clear_color),
    bg_b = OLX_GetBValue(clear_color);
  unsigned char * alpha_bytes = NULL;
  const int alphaSize = BmpHeight*BmpWidth;
  if (mask_bg)
    alpha_bytes = olx_malloc<unsigned char>(alphaSize);
  const int bmpSize = BmpHeight*BmpWidth*3;
  unsigned char* bmpData = olx_malloc<unsigned char>(bmpSize);
  FGlConsole->SetVisible(false);
  FXApp->GetRender().OnDraw.SetEnabled(false);
  int32_t previous_quality = -1;
  if( res != 1 )  {
    FXApp->GetRender().GetScene().ScaleFonts(res);
    if( res >= 2 )
      previous_quality = FXApp->Quality(qaPict);
    FXApp->UpdateLabels();
  }
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, (int)(res < 1 ? 1 : res));
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      int mj = j*SrcWidth;
      int mi = i*SrcHeight;
      for( int k=0; k < SrcWidth; k++ )  {
        for( int l=0; l < SrcHeight; l++ )  {
          int indexA = (l*SrcWidth + k)*3;
          int indexB = bmpSize - (BmpWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
          if (mask_bg) {
            int index = alphaSize - (BmpWidth*(mi + l + 1) - mj - k);
            if (bmpData[indexB]   == bg_r &&
                bmpData[indexB+1] == bg_g &&
                bmpData[indexB+2] == bg_b)
            {
              bmpData[indexB] = bmpData[indexB+1] = bmpData[indexB+2] = ~0;
              alpha_bytes[index] = 0;
            }
            else
              alpha_bytes[index] = ~0;
          }
        }
      }
      delete [] PP;
    }
  }
  if( res != 1 ) {
    FXApp->GetRender().GetScene().RestoreFontScale();
    if( previous_quality != -1 )
      FXApp->Quality(previous_quality);
    FXApp->UpdateLabels();
  }

  FXApp->GetRender().OnDraw.SetEnabled(true);
  FGlConsole->SetVisible(true);
  // end drawing etc
  FXApp->GetRender().Resize(orgWidth, orgHeight); 
  FXApp->GetRender().LookAt(0,0,1);
  FXApp->GetRender().SetView(false, 1);
  FXApp->Draw();
  olxstr bmpFN;
  if( FXApp->XFile().HasLastLoader() && !TEFile::IsAbsolutePath(Cmds[0]) )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName(Cmds[0]);
  else
    bmpFN = Cmds[0];
  wxImage image;
  image.SetData(bmpData, BmpWidth, BmpHeight); 
  if (alpha_bytes != NULL)
    image.SetAlpha(alpha_bytes);
  // correct a common typo
  if( TEFile::ExtractFileExt(bmpFN).Equalsi("jpeg") )
    bmpFN = TEFile::ChangeFileExt(bmpFN, "jpg");
  image.SaveFile(bmpFN.u_str());
}
//..............................................................................
void TMainForm::macPictPS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  OrtDraw od;
  uint16_t color_mode = 0;
  if( Options.Contains("color_fill") )
    color_mode |= ortep_color_fill;
  if( Options.Contains("color_line") )
    color_mode |= ortep_color_lines;
  if( Options.Contains("color_bond") )
    color_mode |= ortep_color_bond;
  od.SetColorMode(color_mode);
  od.SetHBondScale(Options.FindValue("scale_hb", "0.5").ToDouble());
  od.SetPieDiv(Options.FindValue("div_pie", "4").ToInt());
  od.SetFontLineWidth(Options.FindValue("lw_font", "1").ToDouble());
  od.SetPieLineWidth(Options.FindValue("lw_pie", "0.5").ToDouble());
  od.SetElpLineWidth(Options.FindValue("lw_ellipse", "1").ToDouble());
  od.SetQuadLineWidth(Options.FindValue("lw_octant", "0.5").ToDouble());
  od.SetBondOutlineColor(Options.FindValue("bond_outline_color", "0xFFFFFF").SafeUInt<uint32_t>());
  od.SetBondOutlineSize(Options.FindValue("bond_outline_oversize", "10").ToFloat<float>()/100.0f);
  od.SetAtomOutlineColor(Options.FindValue("atom_outline_color", "0xFFFFFF").SafeUInt<uint32_t>());
  od.SetAtomOutlineSize(Options.FindValue("atom_outline_oversize", "5").ToFloat<float>()/100.0f);
  if( Options.Contains('p') )
    od.SetPerspective(true);
  olxstr octants = Options.FindValue("octants", "-$C");
  // store the atom draw styles
  TGXApp::AtomIterator ai = FXApp->GetAtoms();
  TIntList ds(ai.count);
  for( size_t i=0; ai.HasNext(); i++ )  {
    TXAtom& xa = ai.Next();
    ds[i] = xa.DrawStyle();
    if( ds[i] == adsEllipsoid )
      xa.DrawStyle(adsOrtep);
  }
  TStrList toks(octants, ',');
  for( size_t i=0; i < toks.Count(); i++ )  {
    if( toks[i].Length() < 2 || !(toks[i].CharAt(0) == '-' || toks[i].CharAt(0) == '+') )  continue;
    if( toks[i].CharAt(0) == '-' )  {  // exclude
      if( toks[i].Length() == 2 && toks[i].CharAt(1) == '*' )  { // special case...
        ai.Reset();
        while( ai.HasNext() )  {
          TXAtom& xa = ai.Next();
          if( xa.DrawStyle() == adsOrtep )
            xa.DrawStyle(adsEllipsoid);
        }
      }
      else if( toks[i].CharAt(1) == '$' )  {  // atom type
        cm_Element* elm = XElementLib::FindBySymbol(toks[i].SubStringFrom(2));
        if( elm == NULL )  continue;
        ai.Reset();
        while( ai.HasNext() )  {
          TXAtom& xa = ai.Next();
          if( xa.GetType() == *elm && xa.DrawStyle() == adsOrtep )
            xa.DrawStyle(adsEllipsoid);
        }
      }
      else  {  // atom name
        olxstr aname = toks[i].SubStringFrom(1);
        ai.Reset();
        while( ai.HasNext() )  {
          TXAtom& xa = ai.Next();
          if( xa.DrawStyle() == adsOrtep && xa.GetLabel().Equalsi(aname) )
            xa.DrawStyle(adsEllipsoid);
        }
      }
    }
    else  {  // include
      if( toks[i].Length() == 2 && toks[i].CharAt(1) == '*' )  { // special case...
        ai.Reset();
        while( ai.HasNext() )  {
          TXAtom& xa = ai.Next();
          if( xa.DrawStyle() == adsEllipsoid )
            xa.DrawStyle(adsOrtep);
        }
      }
      else if( toks[i].CharAt(1) == '$' )  {  // atom type
        cm_Element* elm = XElementLib::FindBySymbol(toks[i].SubStringFrom(2));
        if( elm == NULL )  continue;
        ai.Reset();
        while( ai.HasNext() )  {
          TXAtom& xa = ai.Next();
          if( xa.GetType() == *elm && xa.DrawStyle() == adsEllipsoid )
            xa.DrawStyle(adsOrtep);
        }
      }
      else  {  // atom name
        olxstr aname = toks[i].SubStringFrom(1);
        ai.Reset();
        while( ai.HasNext() )  {
          TXAtom& xa = ai.Next();
          if( xa.DrawStyle() == adsEllipsoid && xa.GetLabel().Equalsi(aname) )
            xa.DrawStyle(adsOrtep);
        }
      }
    }
  }

  od.Render(TEFile::ChangeFileExt(Cmds[0], "eps"));
  // restore atom draw styles
  ai.Reset();
  for( size_t i=0; ai.HasNext(); i++ )
    ai.Next().DrawStyle(ds[i]);
}
//..............................................................................
void TMainForm::macPictTEX(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  OrtDrawTex od;
  short color_mode = 0;
  if( Options.Contains("color_fill") )
    color_mode = ortep_color_fill;
  else if( Options.Contains("color_line") )
    color_mode = ortep_color_lines;
  od.SetColorMode(color_mode);
  od.Render(Cmds[0]);
}
//..............................................................................
void TMainForm::macPictPR(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr file_name = (Cmds[0].EndsWith(".pov") ? Cmds[0] : olxstr(Cmds[0]) << ".pov");
  TCStrList(TStrList(FXApp->ToPov())).SaveToFile(file_name);
}
//..............................................................................
void TMainForm::macBang(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TTTable<TStrList> Table;
  olxstr Tmp = Cmds.Text(' '), clipbrd;
  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, true, true) ) {
    Error.ProcessingError(__OlxSrcInfo, "could not find any atoms");
    return;
  }
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    FXApp->BangTable(*Atoms[i], Table);
    TStrList Output;
    Table.CreateTXTList(Output, EmptyString(), true, true, ' ');
    FGlConsole->PrintText(Output, NULL, false);
    clipbrd << Output.Text(NewLineSequence());
  }
  if (Options.Contains('c'))
    FXApp->ToClipboard(clipbrd);
}
//..............................................................................
void TMainForm::macUniq(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms = FindXAtoms(Cmds, false, true);
  if( Atoms.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  TNetPList L(Atoms, FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
  FXApp->FragmentsVisible(FXApp->InvertFragmentsList(
    ACollectionItem::Unique(L)), false);
  FXApp->CenterView(true);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::macGroup(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr name = Options.FindValue('n');
  if( FXApp->GetSelection().IsEmpty() )
    FXApp->SelectAll(true);
  if( name.IsEmpty() )  {
    name = "group";
    name << (FXApp->GetRender().GroupCount()+1);
  }
  FXApp->GroupSelection(name);
}
//..............................................................................
void TMainForm::macFmol(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->AllVisible(true);
  FXApp->CenterView();
  FXApp->GetRender().GetBasis().SetZoom(FXApp->GetRender().CalcZoom()*FXApp->GetExtraZoom());
}
//..............................................................................
void TMainForm::macClear(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  FGlConsole->ClearBuffer();  
}
//..............................................................................
void TMainForm::macCell(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
// the events are handled in void TMainForm::CellVChange()
  if( Cmds.IsEmpty() )
    FXApp->SetCellVisible(!FXApp->IsCellVisible());
  else
    FXApp->SetCellVisible(Cmds[0].ToBool());
  if (FXApp->DUnitCell().IsVisible()) {
    olxstr r = Options.FindValue('r', FalseString());
    if (r.IsEmpty()) r = FalseString();
    bool br = r.ToBool();
    FXApp->DUnitCell().SetReciprocal(br, br ? 100: 1);
  }
  FXApp->CenterView();
}
//..............................................................................
void TMainForm::macRota(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 2 )  {  // rota x 90 syntax
    double angle = Cmds[1].ToDouble();
    if( Cmds[0] == "1" || Cmds[0] == "x"  || Cmds[0] == "a" )
      FXApp->GetRender().GetBasis().RotateX(FXApp->GetRender().GetBasis().GetRX()+angle);
    else if( Cmds[0] == "2" || Cmds[0] == "y"  || Cmds[0] == "b" )
      FXApp->GetRender().GetBasis().RotateY(FXApp->GetRender().GetBasis().GetRY()+angle);
    else if( Cmds[0] == "3" || Cmds[0] == "z"  || Cmds[0] == "c" )
      FXApp->GetRender().GetBasis().RotateZ(FXApp->GetRender().GetBasis().GetRZ()+angle);
  }
  else if( Cmds.Count() == 5 )  {  // rota x y z 90 1 syntax - rotation around (x,y,z) 90 degrees with 1 degree inc
    FRotationVector[0] = Cmds[0].ToDouble();
    FRotationVector[1] = Cmds[1].ToDouble();
    FRotationVector[2] = Cmds[2].ToDouble();
    FRotationAngle = Cmds[3].ToDouble();
    FRotationIncrement = Cmds[4].ToDouble();
    FMode = FMode | mRota;
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "wrong parameters");
  }
}
//..............................................................................
void TMainForm::macListen(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() != 0 )  {
    FMode |= mListen;
    FListenFile = TEFile::OSPath(Cmds.Text(' '));
    TBasicApp::NewLogEntry() << "Listening for: '" << FListenFile << '\'';
  }
  else  {
    if( FMode & mListen )
      TBasicApp::NewLogEntry() << "Listening for: '" << FListenFile << '\'';
    else
      TBasicApp::NewLogEntry() << "Not in a listening mode";
  }
}
//..............................................................................
#ifdef __WIN32__
void TMainForm::macWindowCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  for( size_t i=1; i < Cmds.Count(); i++ )  {
    if( Cmds[i].Equalsi("nl") )    
      TWinWinCmd::SendWindowCmd(Cmds[0], '\r');
    else if( Cmds[i].Equalsi("sp") )
      TWinWinCmd::SendWindowCmd(Cmds[0], ' ');
    else
      TWinWinCmd::SendWindowCmd(Cmds[0], Cmds[i]+' ');
  }
}
#endif
//..............................................................................
void TMainForm::macProcessCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( _ProcessManager->GetRedirected() == NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "process does not exist" );
    return;
  }
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].Equalsi("nl") )
      _ProcessManager->GetRedirected()->Writenl();
    else if( Cmds[i].Equalsi("sp") )
      _ProcessManager->GetRedirected()->Write(' ');
    else
      _ProcessManager->GetRedirected()->Write(Cmds[i]+' ');
  }
}
//..............................................................................
void TMainForm::macWait(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  int val = Cmds[0].ToInt();
  if( Options.Contains('r') )  {
    int iv = 50;
    while( val > 0 )  {
      while( wxTheApp->Pending() )
        wxTheApp->Dispatch();
      olx_sleep(iv);
      val -= iv;
    }
  }
  else
    olx_sleep(val);
}
//..............................................................................
void TMainForm::macSwapBg(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->GetRender().Background()->SetVisible(false);  // hide the gradient background
  if( FXApp->GetRender().LightModel.GetClearColor().GetRGB() == 0xffffffff )
    FXApp->GetRender().LightModel.SetClearColor(FBgColor);
  else  {
    FBgColor = FXApp->GetRender().LightModel.GetClearColor();  // update if changed externally...
    FXApp->GetRender().LightModel.SetClearColor(0xffffffff);
  }
  FXApp->GetRender().InitLights();
}
//..............................................................................
void TMainForm::macSilent(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    if( FMode & mSilent )  TBasicApp::NewLogEntry() << "Silent mode is on";
    else                   TBasicApp::NewLogEntry() << "Silent mode is off";
  }
  else if( Cmds[0] == "on" )  {
    FMode |= mSilent;
    TBasicApp::NewLogEntry(logInfo) << "Silent mode is on";
  }
  else if( Cmds[0] == "off" )  {
    FMode &= ~mSilent;
    TBasicApp::NewLogEntry(logInfo) << "Silent mode is off";
  }
}
//..............................................................................
void TMainForm::macStop(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds[0] == "listen" )  {
    if( FMode & mListen )  {
      FMode ^= mListen;
      TBasicApp::NewLogEntry(logInfo) << "Listen mode is off";
    }
  }
  else if( Cmds[0] == "logging" )  {
    if( ActiveLogFile != NULL )  {
      delete ActiveLogFile;
      ActiveLogFile = NULL;
    }
  }
}
//..............................................................................
void TMainForm::macEcho(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  olxstr m = Options.FindValue('m');
  if( !m.IsEmpty() )  {
    if( m.Equalsi("info") )
      FGlConsole->SetPrintMaterial(&InfoFontColor);
    else if( m.Equalsi("warning") )
      FGlConsole->SetPrintMaterial(&WarningFontColor);
    else if( m.Equalsi("error") )
      FGlConsole->SetPrintMaterial(&ErrorFontColor);
    else if( m.Equalsi("exception") )
      FGlConsole->SetPrintMaterial(&ExceptionFontColor);
  }
  TBasicApp::NewLogEntry() << Cmds.Text(' ');
  if (Options.Contains('c'))
    FXApp->ToClipboard(Cmds.Text(' '));
}
//..............................................................................
void TMainForm::macPost(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FXApp == NULL || FGlConsole == NULL )  return;
  TBasicApp::NewLogEntry() << Cmds.Text(' ');
  FXApp->Draw();
  wxTheApp->Dispatch();
}
//..............................................................................
void TMainForm::macExit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  Close(true);
}
//..............................................................................
void TMainForm::macCapitalise(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TXAtomPList xatoms;
  const olxstr format = Cmds[0];
  Cmds.Delete(0);
  if( !FindXAtoms(Cmds, xatoms, true, true) )  return;
  for( size_t i=0; i < xatoms.Count(); i++ )
    xatoms[i]->CAtom().SetTag(0);
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    if( xatoms[i]->CAtom().GetTag() != 0 )  continue;
    const olxstr& label = xatoms[i]->CAtom().GetLabel();
    const size_t len = olx_min(Cmds[0].Length(), label.Length());
    olxstr new_label(label);
    for( size_t j=0; j < len; j++ )   {
      if( format[j] >= 'a' && format[j] <= 'z' )
        new_label[j] = olxstr::o_tolower(label.CharAt(j));
      else if( format[j] >= 'A' && format[j] <= 'Z' )
        new_label[j] = olxstr::o_toupper(label.CharAt(j));
    }
    xatoms[i]->CAtom().SetLabel(new_label, false);
    xatoms[i]->CAtom().SetTag(1);
  }
}
//..............................................................................
void TMainForm::macSetEnv(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if( !olx_setenv(Cmds[0], Cmds[1]) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not set the variable");
    return;
  }
}
//..............................................................................
void TMainForm::macSetView(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  const bool do_center = Options.Contains('c');
  bool process = false, has_center = true;
  vec3d center, normal;
  if( Cmds.IsEmpty() )  {
    TGlGroup& g = FXApp->GetSelection();
    if( g.Count() == 1 )  {
      if( EsdlInstanceOf(g[0], TXPlane) )  {
        TXPlane& xp = (TXPlane&)g[0];
        normal = xp.GetNormal();
        center = xp.GetCenter();
        process = true;
      }
      else if( EsdlInstanceOf(g[0], TXBond) )  {
        TXBond& xb = (TXBond&)g[0];
        normal = vec3d(xb.B().crd()-xb.A().crd()).Normalise();
        center = (xb.B().crd()+xb.A().crd())/2;
        process = true;
      }
    }
  }
  else  {
    if( Cmds.Count() == 3 && Cmds[0].IsNumber() && Cmds[1].IsNumber() && Cmds[2].IsNumber() )  {
      normal = vec3d(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble()).Normalise();
      process = true;
      has_center = false;
    }
    else if( Cmds.Count() == 6 &&
             Cmds[0].IsNumber() && Cmds[1].IsNumber() && Cmds[2].IsNumber() &&
             Cmds[3].IsNumber() && Cmds[4].IsNumber() && Cmds[5].IsNumber() )
    {
      normal = vec3d(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble()).Normalise();
      center = vec3d(Cmds[3].ToDouble(), Cmds[4].ToDouble(), Cmds[5].ToDouble());
      process = true;
    }
  }
  if( !process )  {
    TXAtomPList xatoms = FindXAtoms(Cmds, true, false);
    if( xatoms.Count() < 2 )  {
      Error.ProcessingError(__OlxSrcInfo, "At least two atoms are required");
      return;
    }
    process = true;
    if( xatoms.Count() == 2 )  {
      center = (xatoms[0]->crd()+xatoms[1]->crd())/2;
      normal = (xatoms[1]->crd()-xatoms[0]->crd()).Normalise();
    }
    else  {
      TSAtomPList satoms(xatoms, StaticCastAccessor<TSAtom>());
      mat3d params;
      vec3d rms;
      TSPlane::CalcPlanes(satoms, params, rms, center);
      normal = params[2];
    }
  }
  if( process )  {
    FXApp->GetRender().GetBasis().OrientNormal(normal);
    if( do_center && has_center )
      FXApp->GetRender().GetBasis().SetCenter(-center);
    if( FXApp->XGrid().IsVisible() && has_center )
      FXApp->SetGridDepth(center);
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macHelp(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FHelpItem == NULL )  {  // just print out built in functions if any
    if( Cmds.IsEmpty() )
      return;
    PostCmdHelp(Cmds[0], true);
    return;
  }
  if( Cmds.IsEmpty() )  {
    if( !Options.Count() )  {
      size_t period=6;
      olxstr Tmp;
      for( size_t i=0; i <= FHelpItem->ItemCount(); i+=period )  {
        Tmp.SetLength(0);
        for( size_t j=0; j < period; j++ )  {
          if( (i+j) >= FHelpItem->ItemCount() )
            break;
          Tmp << FHelpItem->GetItem(i+j).GetName();
          Tmp.RightPadding((j+1)*10, ' ', true);
        }
        FGlConsole->PrintText(Tmp);
      }
      return;
    }
    else  {
      if( Options.GetName(0)[0] ==  'c' )  {  // show categories
        TStrList Cats;
        TDataItem *Cat;
        for( size_t i=0; i < FHelpItem->ItemCount(); i++ )  {
          Cat = FHelpItem->GetItem(i).FindItemi("category");
          if( Cat == NULL )  continue;
          for( size_t j=0; j < Cat->ItemCount(); j++ )  {
            if( Cats.IndexOf(Cat->GetItem(j).GetName()) == InvalidIndex )
              Cats.Add(Cat->GetItem(j).GetName());
          }
        }
        if( Cats.Count() )
          FGlConsole->PrintText("Macro categories: ");
        else
          FGlConsole->PrintText("No macro categories was found...");
        Cats.QSort(true);
        for( size_t i=0; i < Cats.Count(); i++ )
          FGlConsole->PrintText(Cats[i]);
      }
    }
  }
  else  {
    if( !Options.Count() )
      PostCmdHelp(Cmds[0], true);
    else  {
      if( Options.GetName(0)[0] ==  'c' )  {  // show categories
        FGlConsole->PrintText(olxstr("Macroses for category: ") << Cmds[0]);
        TDataItem *Cat;
        for( size_t i=0; i < FHelpItem->ItemCount(); i++ )  {
          Cat = FHelpItem->GetItem(i).FindItemi("category");
          if( Cat == NULL )  continue;
          for( size_t j=0; j < Cat->ItemCount(); j++ )  {
            if( Cat->GetItem(j).GetName().Equalsi(Cmds[0]) )  {
              FGlConsole->PrintText(FHelpItem->GetItem(i).GetName());
              break;
            }
          }
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macMatr(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if( Cmds.IsEmpty() )  {
    const mat3d& Matr = FXApp->GetRender().GetBasis().GetMatrix();
    for( size_t i=0; i < 3; i++ )  {
      olxstr Tmp;
      Tmp << olxstr::FormatFloat(3, Matr[0][i]);
      Tmp.RightPadding(7, ' ', true);
      Tmp << olxstr::FormatFloat(3, Matr[1][i]);
      Tmp.RightPadding(14, ' ', true);
      Tmp << olxstr::FormatFloat(3, Matr[2][i]);
      Tmp.RightPadding(21, ' ', true);
      TBasicApp::NewLogEntry() << Tmp;
    }
  }
  else  {
    if( Cmds.Count() == 1 )  {
      const mat3d& M = FXApp->IsHklVisible() ?
        FXApp->XFile().GetAsymmUnit().GetHklToCartesian()
        : FXApp->XFile().GetAsymmUnit().GetCellToCartesian();
      olxstr arg;
      if( Cmds[0] == '1' || Cmds[0] == 'a' )
        arg = "100";
      else if( Cmds[0] == '2' || Cmds[0] == 'b' )
        arg = "010";
      else if( Cmds[0] == '3' || Cmds[0] == 'c' )
        arg = "001";
      else
        arg = Cmds[0];
      if( (arg.Length()%3) != 0 )  {
        Error.ProcessingError(__OlxSrcInfo,
          "invalid argument, an arguments like 010, 001000, +0-1+1 etc is"
          " expected");
        return;
      }
      vec3d n;
      const size_t s = arg.Length()/3;
      for( int i=0; i < 3; i++ )
        n += M[i]*arg.SubString(s*i, s).ToInt();
      if( n.QLength() < 1e-3 )  {
        Error.ProcessingError(__OlxSrcInfo, "non zero expression is expected");
        return;
      }
      FXApp->GetRender().GetBasis().OrientNormal(n);
    }
    else if( Cmds.Count() == 2 )  {  // from to view
      if( (Cmds[0].Length()%3) != 0 || (Cmds[1].Length()%3) != 0 )  {
        Error.ProcessingError(__OlxSrcInfo,
          "invalid arguments, a klm, two arguments like 010, 001000, +0-1+1 "
          "etc are expected");
        return;
      }
      const mat3d& M = FXApp->XFile().GetAsymmUnit().GetCellToCartesian();
      vec3d from, to;
      const size_t fs = Cmds[0].Length()/3, ts = Cmds[1].Length()/3;
      for( int i=0; i < 3; i++ )  {
        from += M[i]*Cmds[0].SubString(fs*i, fs).ToInt();
        to += M[i]*Cmds[1].SubString(ts*i, ts).ToInt();
      }
      vec3d n = from-to;
      if( n.QLength() < 1e-3 )  {
        Error.ProcessingError(__OlxSrcInfo,
          "from and to arguments must be different");
        return;
      }
      FXApp->GetRender().GetBasis().OrientNormal(n);
    }
    else if( Cmds.Count() == 3 )  {  // view along
      const mat3d& M = FXApp->XFile().GetAsymmUnit().GetCellToCartesian();
      vec3d n = M[0]*Cmds[0].ToDouble() +
        M[1]*Cmds[1].ToDouble() +
        M[2]*Cmds[2].ToDouble();
      try { n.Normalise(); }
      catch(const TExceptionBase &e) {
        Error.ProcessingError(__OlxSrcInfo, "non-singular direction is expected");
      }
      FXApp->GetRender().GetBasis().OrientNormal(n);
    }
    else if( Cmds.Count() == 9 )  {
      mat3d M(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble(),
      Cmds[3].ToDouble(), Cmds[4].ToDouble(), Cmds[5].ToDouble(),
      Cmds[6].ToDouble(), Cmds[7].ToDouble(), Cmds[8].ToDouble());
      M.Transpose();
      M[0].Normalise();
      M[1].Normalise();
      M[2].Normalise();
      FXApp->GetRender().GetBasis().SetMatrix(M);
    }
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macLine(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  bool process_atoms = true;
  vec3d from, to;
  if( Cmds.IsEmpty() || (Cmds.Count() == 1 && Cmds[0].Equalsi("sel")) )  {
    TGlGroup& sel = FXApp->GetSelection();
    if( sel.Count() == 2 )  {
      if( EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXPlane) )  {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if( EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXPlane) )  {
        from = ((TXAtom&)sel[0]).crd();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXAtom) )  {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXAtom&)sel[1]).crd();
        process_atoms = false;
      }
    }
  }
  TXAtomPList Atoms;
  if( process_atoms )
    FindXAtoms(Cmds, Atoms, true, true);
  if( Atoms.Count() > 2 )  {
    TSAtomPList satoms(Atoms, StaticCastAccessor<TSAtom>());
    mat3d params;
    vec3d rms, center;
    TSPlane::CalcPlanes(satoms, params, rms, center);
    double maxl = -1000, minl = 1000;
    for( size_t i=0; i < satoms.Count(); i++ )  {
      vec3d v = satoms[i]->crd() - center;
      if( v.QLength() < 0.0001 )
        continue;
      const double ca = params[2].CAngle(v);
      const double l = v.Length()*ca;
      if( l > maxl )
        maxl = l;
      if( l < minl )
        minl = l;
    }
    from = center+params[2]*minl;
    to = center+params[2]*maxl;
  }
  else if( Atoms.Count() == 2 )  {
    from = Atoms[0]->crd();
    to = Atoms[1]->crd();
  }

  else if( process_atoms ) {
    Error.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
    return;
  }
  olxstr name = Options.FindValue('n');
  if (Options.Contains('n') && name.IsEmpty())
    FXApp->GetRender().GetBasis().OrientNormal(to-from);
  else
    FXApp->AddLine(name, from, to);
  FXApp->Draw();
}
//..............................................................................
void TMainForm::macMpln(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSPlane* plane = NULL;
  bool orientOnly = Options.Contains('n'),
    rectangular = Options.Contains('r');
  olxstr name = Options.FindValue('n');
  const double weightExtent = olx_abs(Options.FindValue("we", "0").ToDouble());
  olxstr planeName;
  TXAtomPList Atoms = FindXAtoms(Cmds, true, true);
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    planeName << Atoms[i]->GetLabel();
    if( i+1 < Atoms.Count() )
      planeName << ' ';
  }

  if( Atoms.Count() < 3 )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong atom count");
    return;
  }
  if (orientOnly && name.IsEmpty()) {
    plane = FXApp->TmpPlane(&Atoms, weightExtent);
    if( plane != NULL )  {
      mat3d m = plane->GetBasis();
      vec3d d1 = (m[2]+m[1]).Normalise();
      vec3d d2 = m[0].XProdVec(d1).Normalise();
      FXApp->GetRender().GetBasis().Orient(d1, d2, m[0]);
      FXApp->SetGridDepth(plane->GetCenter());
      delete plane;
      plane = NULL;
    }
  }
  else  {
    TXPlane* xp = FXApp->AddPlane(name, Atoms, rectangular, weightExtent);
    if( xp != NULL )
      plane = xp;
  }
  if( plane != NULL )  {
    const TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
    size_t colCount = 3;
    TTTable<TStrList> tab(plane->Count()/colCount +
      (((plane->Count()%colCount)==0)?0:1), colCount*3);
    for( size_t i=0; i < colCount; i++ )  {
      tab.ColName(i*3) = "Label";
      tab.ColName(i*3+1) = "D/A";
    }
    double rmsd = 0;
    for( size_t i=0; i < plane->Count(); i+=colCount )  {
      for( size_t j=0; j < colCount; j++ )  {
        if( (i + j) >= Atoms.Count() )
          break;
        tab[i/colCount][j*3] = plane->GetAtom(i+j).GetLabel();
        const double v = plane->DistanceTo(plane->GetAtom(i+j).crd()); 
        rmsd += v*v;
        tab[i/colCount][j*3+1] = olxstr::FormatFloat(3, v);
      }
    }
    rmsd = sqrt(rmsd/Atoms.Count());
    TBasicApp::NewLogEntry() <<
      tab.CreateTXTList(olxstr("Atom-to-plane distances for ") << planeName,
      true, false, " | ");
    TBasicApp::NewLogEntry() << "Plane equation: " << plane->StrRepr();
    TBasicApp::NewLogEntry() << "HKL direction: " <<
      plane->GetCrystallographicDirection().ToString();
    if( weightExtent != 0 )  {
      TBasicApp::NewLogEntry() << "Weighted RMSD/A: " <<
        olxstr::FormatFloat(3, plane->GetWeightedRMSD());
      TBasicApp::NewLogEntry() << "RMSD/A: " <<
        olxstr::FormatFloat(3, plane->CalcRMSD());
    }
    else  {
      TBasicApp::NewLogEntry() << "RMSD/A: " <<
        olxstr::FormatFloat(3, plane->GetWeightedRMSD());
    }
  }
  else if( !orientOnly ) {
    TBasicApp::NewLogEntry() <<
      "The plane was not created because it is either not unique or valid";
  }
}
//..............................................................................
void TMainForm::macCent(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->AddCentroid(FindXAtoms(Cmds, true, true).GetObject());
}
//..............................................................................
void TMainForm::macKill(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Modes->GetCurrent() != NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "Kill inaccessible from within a mode");
    return;
  }
  if( Cmds.Count() == 1 && Cmds[0].Equalsi("sel") )  {
    AGDObjList Objects;
    TGlGroup& sel = FXApp->GetSelection();
    olxstr out;
    bool group_deletion = false;
    for( size_t i=0; i < sel.Count(); i++ )  {
      if( EsdlInstanceOf(sel[i], TXAtom) )
        out << ((TXAtom&)sel[i]).GetLabel();
      if( EsdlInstanceOf(sel[i], TGlGroup) )  {
        if( !group_deletion )  {
          group_deletion = true;
          TBasicApp::NewLogEntry() << "Please use 'ungroup' to delete groups";
        }
        continue;
      }
      else
        out << sel[i].GetPrimitives().GetName();
      out << ' ';
      Objects.Add(sel[i]);
    }
    if( !out.IsEmpty()  )  {
      FXApp->NewLogEntry() << "Deleting " << out;
      FXApp->GetUndo().Push(FXApp->DeleteXObjects(Objects));
      sel.Clear();
    }
  }
  else if( Cmds.Count() == 1 && Cmds[0].Equalsi("labels") )  {
    FXApp->NewLogEntry() << "Deleting labels";
    for( size_t i=0; i < FXApp->LabelCount(); i++ )
      FXApp->GetLabel(i).SetVisible(false);
    TGXApp::AtomIterator ai = FXApp->GetAtoms();
    while( ai.HasNext() )
      ai.Next().GetGlLabel().SetVisible(false);
    TGXApp::BondIterator bi = FXApp->GetBonds();
    while( bi.HasNext() )
      bi.Next().GetGlLabel().SetVisible(false);
  }
  else  {
    if( Options.Contains("au") )  {
      TCAtomPList atoms = FXApp->FindCAtoms(Cmds.Text(' '), false);
      for( size_t i=0; i < atoms.Count(); i++ )
        atoms[i]->SetDeleted(true);
    }
    else  {
      TXAtomPList Atoms = FXApp->FindXAtoms(Cmds.Text(' '), true, false),
        Selected;
      if( Atoms.IsEmpty() )  return;
      for( size_t i=0; i < Atoms.Count(); i++ )
        if( Atoms[i]->IsSelected() )
          Selected.Add(Atoms[i]);
      TXAtomPList& todel = Selected.IsEmpty() ? Atoms : Selected;
      if( todel.IsEmpty() )
        return;
      FXApp->GetLog() << "Deleting ";
      for( size_t i=0; i < todel.Count(); i++ )
        FXApp->GetLog() << todel[i]->GetLabel() << ' ';
      FXApp->NewLogEntry();
      FXApp->GetUndo().Push(FXApp->DeleteXAtoms(todel));
    }
  }
}
//..............................................................................
void TMainForm::macHide(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 0 || Cmds[0].Equalsi("sel") )  {
    AGDObjList Objects;
    TGlGroup& sel = FXApp->GetSelection();
    for( size_t i=0; i < sel.Count(); i++ )  
      Objects.Add( sel[i] );
    FXApp->GetUndo().Push( FXApp->SetGraphicsVisible( Objects, false ) );
    sel.Clear();
  }
  else  {
    TXAtomPList Atoms = FXApp->FindXAtoms(Cmds.Text(' '), true, Options.Contains('h'));
    if( Atoms.IsEmpty() )  return;
    AGDObjList go(Atoms, StaticCastAccessor<AGDrawObject>());
    FXApp->GetUndo().Push(FXApp->SetGraphicsVisible(go, false));
  }
}
//..............................................................................
void TMainForm::macExec(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool Asyn = !Options.Contains('s'), // synchronously
    Cout = !Options.Contains('o'),    // catch output
    quite = Options.Contains('q');

  olxstr dubFile(Options.FindValue('s',EmptyString()));

  olxstr Tmp;
  for( size_t i=0; i < Cmds.Count(); i++ )
    Tmp << AProcess::PrepareArg(Cmds[i]) << ' ';
  TBasicApp::NewLogEntry(logInfo) << "EXEC: " << Tmp;
  short flags = 0;
  if( (Cout && Asyn) || Asyn )  {  // the only combination
    if( !Cout )
      flags = quite ? spfQuiet : 0;
    else
      flags = quite ? spfRedirected|spfQuiet : spfRedirected;
  }
  else
    flags = spfSynchronised;

#ifdef __WIN32__
  TWinProcess* Process  = new TWinProcess(Tmp, flags);
#elif defined(__WXWIDGETS__)
  TWxProcess* Process = new TWxProcess(Tmp, flags);
#endif

  if( (Cout && Asyn) || Asyn )  {  // the only combination
    if( !Cout )  {
      _ProcessManager->OnCreate(*Process);
      if( !Process->Execute() )  {
        _ProcessManager->OnTerminate(*Process);
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
        return;
      }
    }
    else  {
      _ProcessManager->OnCreate(*Process);
      if( !dubFile.IsEmpty() )  {
        TEFile* df = new TEFile(dubFile, "wb+");
        Process->SetDubStream(df);
      }
      if( !Process->Execute() )  {
        _ProcessManager->OnTerminate(*Process);
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
        return;
      }
    }
  }
  else {
    if( !Process->Execute() )
      Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process");
    delete Process;
  }
}
//..............................................................................
void TMainForm::macShell(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )
    wxShell();
  else  {
    olxstr cmd = Cmds.Text(' ');
#ifdef __WIN32__
    ShellExecute((HWND)this->GetHWND(), wxT("open"), cmd.u_str(), NULL,
      TEFile::CurrentDir().u_str(), SW_SHOWNORMAL);
#else
    if( cmd.StartsFrom("http") || cmd.StartsFrom("https") || cmd.EndsWith(".htm") || 
        cmd.EndsWith(".html") || cmd.EndsWith(".php") || cmd.EndsWith(".asp") )
    {
      Macros.ProcessMacro(olxstr("exec -o getvar(defbrowser) '") << cmd << '\'', Error);
    }
# ifdef __linux__
    else if( cmd.EndsWith(".pdf") || cmd.EndsWith(".PDF") )  {
      wxString dskpAttr;
      wxGetEnv(wxT("DESKTOP_SESSION"), &dskpAttr);
      if (dskpAttr.Contains(wxT("gnome")))
        Macros.ProcessMacro(olxstr("exec -o xdg-open '") << cmd << '\'', Error);
      else if (dskpAttr.Contains(wxT("kde")))
        Macros.ProcessMacro(olxstr("exec -o konqueror '") << cmd << '\'', Error);
      else if (dskpAttr.Contains(wxT("xfce")))
        Macros.ProcessMacro(olxstr("exec -o thunar '") << cmd << '\'', Error);
      else
        Macros.ProcessMacro(olxstr("exec -o getvar(defbrowser) '") << cmd << '\'', Error);
    }
# endif
    else
      Macros.ProcessMacro(olxstr("exec -o getvar(defexplorer) '") << cmd << '\'', Error);
#endif
  }
}
//..............................................................................
void TMainForm::macSave(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr Tmp = Cmds[0];
  Cmds.Delete(0);
  olxstr FN = Cmds.Text(' ');
  if( Tmp == "style" )  {
    if( !FN.Length() )
    {  FN = PickFile("Save drawing style", "Drawing styles|*.glds", StylesDir, false);  }
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !(StylesDir.LowerCase() == Tmp.LowerCase()) )  {
        TBasicApp::NewLogEntry(logInfo) << "Styles folder is changed to: " << Tmp;
        StylesDir = Tmp;
      }
    }
    else  {
      if( !StylesDir.IsEmpty() )  Tmp = StylesDir;
      else                        Tmp = FXApp->GetBaseDir();
      Tmp << FN;  FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glds");
    TDataFile F;
    FXApp->GetRender().GetStyles().ToDataItem(F.Root().AddItem("style"));
    try  {  F.SaveToXLFile(FN); }
    catch( TExceptionBase& )  {
      Error.ProcessingError(__OlxSrcInfo, "failed to save file: " ) << FN;
    }
    return;
  }
  if( Tmp == "scene" )  {
    if( FN.IsEmpty() )
      FN = PickFile("Save scene parameters", "Scene parameters|*.glsp", ScenesDir, false);
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {

      if( !(ScenesDir.LowerCase() == Tmp.LowerCase()) )  {
        TBasicApp::NewLogEntry(logInfo) << "Scene parameters folder is changed to: " << Tmp;
        ScenesDir = Tmp;
      }
    }
    else  {
      Tmp = (ScenesDir.IsEmpty() ? FXApp->GetBaseDir() : ScenesDir);
      Tmp << FN;  FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glsp");
    TDataFile F;
    SaveScene(F.Root(), FXApp->GetRender().LightModel);
    try  {  F.SaveToXLFile(FN);  }
    catch( TExceptionBase& )  {
      Error.ProcessingError(__OlxSrcInfo, "failed to save file: ") << FN ;
    }
    return;
  }
  if( Tmp == "view" )  {
    if( FXApp->XFile().HasLastLoader() )  {
      Tmp = (Cmds.Count() == 1) ? TEFile::ChangeFileExt(Cmds[0], "xlv") :
                                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "xlv");
      TDataFile DF;
      FXApp->GetRender().GetBasis().ToDataItem(DF.Root().AddItem("basis"));
      DF.SaveToXLFile(Tmp);
    }
  }
  else if( Tmp == "model" )  {
    if( FXApp->XFile().HasLastLoader() )  {
      Tmp = (Cmds.Count() == 1) ? TEFile::ChangeFileExt(Cmds[0], "oxm") :
                                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "oxm");
      //TDataFile DF;
      //TDataItem& mi = DF.Root().AddItem("olex_model");
      FXApp->SaveModel(Tmp);
      //DF.SaveToXLFile(Tmp);
    }
  }
}
//..............................................................................
void TMainForm::macLoad(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  if( Cmds[0].Equalsi("style") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FN.IsEmpty() )
      FN = PickFile("Load drawing style", "Drawing styles|*.glds", StylesDir, true);
    if( FN.IsEmpty() )
      return;
    olxstr Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !StylesDir.Equalsi(Tmp) )  {
        TBasicApp::NewLogEntry(logInfo) << "Styles folder is changed to: " << Tmp;
        StylesDir = Tmp;
      }
    }
    else  {
      if( !StylesDir.IsEmpty() )
        Tmp = StylesDir;
      else
        Tmp = FXApp->GetBaseDir();
      FN = TEFile::AddPathDelimeterI(Tmp) << FN;
    }
    FN = TEFile::ChangeFileExt(FN, "glds");
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TDataFile F;
    F.LoadFromXLFile(FN, NULL);
    FXApp->GetRender().ClearSelection();
    // this forces the object creation, so if there is anything wrong...
    try  {  FXApp->GetRender().GetStyles().FromDataItem(*F.Root().FindItem("style"), false);  }
    catch(const TExceptionBase &e)  {
      FXApp->GetRender().GetStyles().Clear();
      TBasicApp::NewLogEntry(logError) << "Failed to load given style";
      TBasicApp::NewLogEntry(logExceptionTrace) << e;
    }
    FXApp->ClearIndividualCollections();
    FXApp->CreateObjects(true);
    FXApp->CenterView(true);
    FN = FXApp->GetRender().GetStyles().GetLinkFile();
    if( !FN.IsEmpty() )  {
      if( TEFile::Exists(FN) )  {
        F.LoadFromXLFile(FN, NULL);
        LoadScene(F.Root(), FXApp->GetRender().LightModel);
      }
      else
        TBasicApp::NewLogEntry(logError) << "Load::style: link file does not exist: " << FN;
    }
  }
  else if( Cmds[0].Equalsi("scene") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FN.IsEmpty() )
      FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", ScenesDir, true);
    if( FN.IsEmpty() )
      return;
    olxstr Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !ScenesDir.Equalsi(Tmp) )  {
        TBasicApp::NewLogEntry(logInfo) << "Scene parameters folder is changed to: " << Tmp;
        ScenesDir = Tmp;
      }
    }
    else  {
      if( !ScenesDir.IsEmpty() )
        Tmp = ScenesDir;
      else
        Tmp = FXApp->GetBaseDir();
      FN = TEFile::AddPathDelimeterI(Tmp) << FN;
    }
    FN = TEFile::ChangeFileExt(FN, "glsp");
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TDataFile DF;
    DF.LoadFromXLFile(FN, NULL);
    LoadScene(DF.Root(), FXApp->GetRender().LightModel);
    FXApp->UpdateLabels();
  }
  else if( Cmds[0].Equalsi("view") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FXApp->XFile().HasLastLoader() )  {
      if( !FN.IsEmpty() )
        FN = TEFile::ChangeFileExt(FN, "xlv");
      else
        FN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "xlv");
    }
    if( TEFile::Exists(FN) )  {
      TDataFile DF;
      DF.LoadFromXLFile(FN);
      FXApp->GetRender().GetBasis().FromDataItem(DF.Root().FindRequiredItem("basis"));
    }
  }
  else if( Cmds[0].Equalsi("model") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FXApp->XFile().HasLastLoader() )  {
      FN = (!FN.IsEmpty() ? TEFile::ChangeFileExt(FN, "oxm") :
                            TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "oxm") );
    }
    if( !FN.IsEmpty() && TEFile::Exists(FN) )  {
      if( !TEFile::IsAbsolutePath(FN) )
        FN = TEFile::AddPathDelimeter(TEFile::CurrentDir()) << FN;        
      FXApp->XFile().LoadFromFile(FN);
    }
  }
  else if ( Cmds[0].Equalsi("radii") )  {
    if( Cmds.Count() > 1  )  {
      olxstr fn = Cmds.Text(' ', 2);
      if( fn.IsEmpty() )
        fn = PickFile("Load atomic radii", "Text files|*.txt", EmptyString(), true);
      if( TEFile::Exists(fn) )  {
        olxdict<olxstr,double,olxstrComparator<false> > radii;
        TStrList sl, toks;
        sl.LoadFromFile(fn);
        // parse the file and fill the dictionary
        TBasicApp::NewLogEntry() << "Using user defined radii for:";
        for( size_t i=0; i < sl.Count(); i++ )  {
          toks.Clear();
          toks.Strtok(sl[i], ' ');
          if( toks.Count() == 2 )  {
            TBasicApp::NewLogEntry() << ' ' << toks[0] << '\t' << toks[1];
            radii(toks[0], toks[1].ToDouble());
          }
        }
        // end the file parsing
        if( Cmds[1].Equalsi("sfil") )  {
          for( size_t i=0; i < radii.Count(); i++ )  {
            cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
            if( cme != NULL )
              cme->r_sfil = radii.GetValue(i);
          }
        }
        else if( Cmds[1].Equalsi("pers") )  {
          for( size_t i=0; i < radii.Count(); i++ )  {
            cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
            if( cme != NULL )
              cme->r_pers = radii.GetValue(i);
          }
        }
        else if( Cmds[1].Equalsi("vdw") )  {
          for( size_t i=0; i < radii.Count(); i++ )  {
            cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
            if( cme != NULL )
              cme->r_vdw = radii.GetValue(i);
          }
        }
        else
          Error.ProcessingError(__OlxSrcInfo, "undefined radii name");
      }
    }
  }
  else
    Error.ProcessingError(__OlxSrcInfo, "undefined parameter");
}
//..............................................................................
void TMainForm::macLink(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr FN, Tmp;
  if( Cmds.IsEmpty() )
    FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", ScenesDir, false);
  else
    FN = Cmds.Text(' ');

  if( FN.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no file name is given");
    return;
  }
  if( FN == "none" )  {
    FXApp->GetRender().GetStyles().SetLinkFile(EmptyString());
    TBasicApp::NewLogEntry(logInfo) << "The link has been removed...";
    return;
  }
  Tmp = TEFile::ExtractFilePath(FN);
  if( Tmp.IsEmpty() )  {
    if( !ScenesDir.IsEmpty() )
      Tmp = ScenesDir;
    else
      Tmp = FXApp->GetBaseDir();
    FN = (Tmp << FN );
  }
  FN = TEFile::ChangeFileExt(FN, "glsp");
  if( TEFile::Exists(FN) )  
    FXApp->GetRender().GetStyles().SetLinkFile(FN);
  else  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exists : ") << FN;
    return;
  }
}
//..............................................................................
void TMainForm::macStyle(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() && Options.IsEmpty() )  {
    olxstr tmp = "Default style: ";
    if( !DefStyle.IsEmpty() )
      tmp << DefStyle;
    else
      tmp << "none";
    TBasicApp::NewLogEntry() << tmp;
    return;
  }
  else  {
    if( !Cmds.IsEmpty() )  {
      if( Cmds[0] == "none" )  {
        DefStyle.SetLength(0);
        TBasicApp::NewLogEntry() << "Default style is reset to none";
        return;
      }
      olxstr FN = Cmds.Text(' ');
      olxstr Tmp = TEFile::ExtractFilePath(FN);
      if( Tmp.IsEmpty() )  {
        if( !StylesDir.IsEmpty() )
          Tmp = StylesDir;
        else
          Tmp = FXApp->GetBaseDir();
        FN = (Tmp << FN);
      }
      if( TEFile::Exists(FN) )
        DefStyle = FN;
      else  {
        Error.ProcessingError(__OlxSrcInfo, "specified file does not exists" );
        return;
      }
      return;
    }
    else  {
      olxstr FN = PickFile("Load drawing style", "Drawing styles|*.glds", StylesDir, false);
      if( TEFile::Exists(FN) )
        DefStyle = FN;
    }
  }
}
//..............................................................................
void TMainForm::macScene(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty()  && Options.IsEmpty() )  {
    olxstr tmp = "Default scene: ";
    if( DefSceneP.IsEmpty() )
      tmp << "none";
    else
      tmp << DefSceneP;
    TBasicApp::NewLogEntry() << tmp;
    return;
  }
  if( !Cmds.IsEmpty() )  {
    if( Cmds[0] == "none" )  {
      TBasicApp::NewLogEntry() << "Default scene is reset to none";
      DefSceneP.SetLength(0);
      return;
    }
    olxstr FN = Cmds.Text(' ');
    olxstr Tmp = TEFile::ExtractFilePath(FN);
    if( Tmp.IsEmpty() )  {
      if( !ScenesDir.IsEmpty() )
        Tmp = ScenesDir;
      else
        Tmp = FXApp->GetBaseDir();
      FN = (Tmp << FN);
    }
    if( TEFile::Exists(FN) )
      DefSceneP = FN;
    else  {
      Error.ProcessingError(__OlxSrcInfo, "specified file does not exists");
      return;
    }
  }
  else  {
    olxstr FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", ScenesDir, false);
    if( TEFile::Exists(FN) )
      DefSceneP = FN;
  }
}
//..............................................................................
void TMainForm::macCeiling(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 1 )  {
    if( Cmds[0].Equalsi("on") )
      FXApp->GetRender().Ceiling()->SetVisible(true);
    else if( Cmds[0].Equalsi("off") )
      FXApp->GetRender().Ceiling()->SetVisible(false);
    else
      Error.ProcessingError(__OlxSrcInfo, "wrong arguments" );
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macFade(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FFadeVector[0] = Cmds[0].ToDouble();
  FFadeVector[1] = Cmds[1].ToDouble();
  FFadeVector[2] = Cmds[2].ToDouble();
  if( FFadeVector[1] < 0 || FFadeVector[1] > 1 )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong arguments");
    return;
  }
  TGlBackground *C = FXApp->GetRender().Ceiling();
  TGlBackground *G = FXApp->GetRender().Background();
  if( !C->IsVisible() )  {
    if( !G->IsVisible() )  {
      C->LT(FXApp->GetRender().LightModel.GetClearColor());
      C->RT(FXApp->GetRender().LightModel.GetClearColor());
      C->LB(FXApp->GetRender().LightModel.GetClearColor());
      C->RB(FXApp->GetRender().LightModel.GetClearColor());
    }
    else  {
      C->LT(G->LT());
      C->RT(G->RT());
      C->LB(G->LB());
      C->RB(G->RB());
    }

    C->SetVisible(true);
  }
  FMode = FMode | mFade;
  return;
}
//..............................................................................
void TMainForm::macWaitFor(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // we need to call the timer in case it is disabled ...
  if( Cmds[0].Equalsi("fade") )  {
    if( !IsVisible() )  return;
    while( FMode & mFade )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
      olx_sleep(50);
    }
  }
  if( Cmds[0].Equalsi("xfader") )  {
    if( !IsVisible() )  return;
    while( FXApp->GetFader().GetPosition() < 1 && FXApp->GetFader().IsVisible() )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
      olx_sleep(50);
    }
  }
  else if( Cmds[0].Equalsi("rota") )  {
    while( FMode & mRota )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
      olx_sleep(50);
    }
  }
  else if( Cmds[0].Equalsi("process") )  {
    _ProcessManager->WaitForLast();
  }
}
//..............................................................................
void TMainForm::macHtmlPanelSwap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    FHtmlOnLeft = !FHtmlOnLeft;
    OnResize();
  }
  else  {
    bool changed = false;
    if( Cmds[0].Equalsi("left") )  {
      if( !FHtmlOnLeft )  {
        changed = FHtmlOnLeft = true;
      }
    }
    else if( Cmds[0].Equalsi("right") )  {
      if( FHtmlOnLeft )  {
        changed = true;
        FHtmlOnLeft = false;
      }
    }
    else  {
      E.ProcessingError( __OlxSrcInfo, olxstr("unknown option '") << Cmds[0] << '\'');
      return;
    }
    if( changed )  OnResize();
  }
}
//..............................................................................
void TMainForm::macHtmlPanelVisible(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    FHtmlMinimized = !FHtmlMinimized;
    OnResize();
    miHtmlPanel->Check(!FHtmlMinimized);
  }
  else if( Cmds.Count() == 1 )  {
    FHtmlMinimized = !Cmds[0].ToBool();
    OnResize();
    miHtmlPanel->Check(!FHtmlMinimized);
  }
  else if( Cmds.Count() == 2 ) {
    bool show = Cmds[0].ToBool();
    THtmlManager::TPopupData *pd = HtmlManager.Popups.Find(Cmds[1], NULL);
    if( pd != NULL )  {
      if( show && !pd->Dialog->IsShown() )  pd->Dialog->Show();
      if( !show && pd->Dialog->IsShown() )  pd->Dialog->Hide();
    }
    else  {
      E.ProcessingError(__OlxSrcInfo, "undefined popup name");
    }
  }
}
//..............................................................................
void TMainForm::macHtmlPanelWidth(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::NewLogEntry(logInfo) << "Current HTML panel width: " << FHtmlPanelWidth;
    return;
  }
  if( Cmds.Count() == 1 )  {
    FHtmlWidthFixed = !Cmds[0].EndsWith('%');
    float width = (FHtmlWidthFixed ? Cmds[0].ToDouble() : Cmds[0].SubStringTo(Cmds[0].Length()-1).ToDouble());
    if( !FHtmlWidthFixed )  {
      width /= 100;
      if( width < 0.01 )
        FHtmlMinimized = true;
      else  {
        if( width > 0.75 )  width = 0.75;
        FHtmlMinimized = false;
        FHtmlPanelWidth = width;
      }
    }
    else  {
      if( width < 10 )
        FHtmlMinimized = true;
      else  {
        int w, h;
        GetClientSize(&w, &h);
        //if( width > w*0.75 )  width = w*0.75;
        FHtmlMinimized = false;
        FHtmlPanelWidth = width;
      }
    }
    OnResize();
    return;
  }
  E.ProcessingError(__OlxSrcInfo, "wrong number of arguments" );
  return;
}
//..............................................................................
void TMainForm::macQPeakScale(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    TBasicApp::NewLogEntry() << "Current Q-Peak scale: " << FXApp->GetQPeakScale();
  else  {
    float scale = Cmds[0].ToFloat<float>();
    FXApp->SetQPeakScale(scale);
    return;
  }
}
//..............................................................................
void TMainForm::macQPeakSizeScale(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    TBasicApp::NewLogEntry() << "Current Q-Peak size scale: " << FXApp->GetQPeakSizeScale();
  else  {
    float scale = Cmds[0].ToFloat<float>();
    FXApp->SetQPeakSizeScale(scale);
    return;
  }
}
//..............................................................................
void TMainForm::macFocus(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  this->Raise();
  FGlCanvas->SetFocus();
}
//..............................................................................
void TMainForm::macRefresh(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  while( wxTheApp->Pending() )
    wxTheApp->Dispatch();
}
//..............................................................................
//..............................................................................
//..............................................................................
void TMainForm::macMode(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  // this variable is set when the mode is changed from within this function
  static bool ChangingMode = false;
  if( ChangingMode )  return;
  olxstr name = Cmds[0], args;
  Cmds.Delete(0);
  args << Cmds.Text(' ');
  for (size_t i=0; i < Options.Count(); i++)
    args << " -" << Options.GetName(i) << '=' << Options.GetValue(i);
  AMode* md = Modes->SetMode(name, args);
  if( md != NULL )  {
    try  {
      if( !md->Initialise(Cmds, Options) )  {
        E.ProcessingError(__OlxSrcInfo, "Current mode is unavailable");
        Modes->ClearMode(false);
        return;
      }
    }
    catch(const TExceptionBase& e)  {
      Modes->ClearMode(false);
      throw TFunctionFailedException(__OlxSrcInfo, e);
    }
  }
  ChangingMode = false;
}
//..............................................................................
void TMainForm::macText(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr log="output.txt";
  if (!Cmds.IsEmpty()) log = Cmds[0];
  olxstr FN = FXApp->GetInstanceDir() + log;
  TUtf8File::WriteLines(FN, FGlConsole->Buffer());
  Macros.ProcessMacro(olxstr("exec getvar('defeditor') -o \"") << FN << '\"' , E);
}
//..............................................................................
void TMainForm::macShowStr(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {  //S+C -> C -> S -> S+C
    if( FXApp->IsStructureVisible() )  {
      if( FGlConsole->ShowBuffer() )
        FXApp->SetStructureVisible(false);
      else  {
        if( FGlConsole->GetLinesToShow() != InvalidSize )
          FGlConsole->SetLinesToShow(InvalidSize);
        FGlConsole->ShowBuffer(true);
      }
    }
    else {
      if( FGlConsole->ShowBuffer() )  {
        FXApp->SetStructureVisible(true);
        FGlConsole->ShowBuffer(false);
      }
      else
        FGlConsole->ShowBuffer(false);
    }
  }
  else
    FXApp->SetStructureVisible(Cmds[0].ToBool());
  FXApp->CenterView();
  TStateRegistry::GetInstance().SetState(FXApp->stateStructureVisible,
    FXApp->IsStructureVisible(), EmptyString(), true);
}
//..............................................................................
void TMainForm::macBind(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds[0].Equalsi("wheel") )  {
    size_t ind = Bindings.IndexOf("wheel");
    if( ind == InvalidIndex )
      Bindings.Add("wheel", Cmds[1]);
    else
      Bindings.GetObject(ind) = Cmds[1];
  }
  else
    E.ProcessingError(__OlxSrcInfo, olxstr("unknown binding parameter: ") << Cmds[0]);
}
//..............................................................................
void TMainForm::macGrad(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  bool invert = Options.Contains('i');
  if( invert )  {
    FXApp->GetRender().Background()->SetVisible(
      !FXApp->GetRender().Background()->IsVisible());
  }
  else if( Cmds.Count() == 1 )  {
    FXApp->GetRender().Background()->SetVisible(Cmds[0].ToBool());
  }
  else if( Cmds.IsEmpty() && !Options.Contains('p'))  {
    TdlgGradient *G = new TdlgGradient(this);
    G->ShowModal();
    G->Destroy();
  }
  else if( Cmds.Count() == 4 )  {
    TGlOption v;
    v = Cmds[0].ToInt();  FXApp->GetRender().Background()->RB(v);
    v = Cmds[1].ToInt();  FXApp->GetRender().Background()->LB(v);
    v = Cmds[2].ToInt();  FXApp->GetRender().Background()->RT(v);
    v = Cmds[3].ToInt();  FXApp->GetRender().Background()->LT(v);
  }
  if (Options.Contains('p')) {
    GradientPicture = Options.FindValue("p", GradientPicture);
    if( GradientPicture.IsEmpty() )  {
      TGlTexture* glt = FXApp->GetRender().Background()->GetTexture();
      if( glt != NULL  )
        glt->SetEnabled(false);
    }
    else if( TEFile::Exists(GradientPicture) )  {
      wxFSFile* inf = TFileHandlerManager::GetFSFileHandler(GradientPicture);
      if( inf == NULL )  {
        E.ProcessingError(__OlxSrcInfo, "Image file does not exist: ") << GradientPicture;
        return;
      }
      wxImage img(*inf->GetStream());
      delete inf;
      if( !img.Ok() )  {
        E.ProcessingError(__OlxSrcInfo, "Invalid image file: ") << GradientPicture;
        return;
      }
      int owidth = img.GetWidth(), oheight = img.GetHeight();
      int swidth = owidth, sheight = oheight;
      if (!Options.Contains('r')) {
        int l = CalcL(img.GetWidth());
        swidth = (int)pow((double)2, (double)l);
        l = CalcL(img.GetHeight());
        sheight = (int)pow((double)2, (double)l);
      }

      if( swidth != owidth || sheight != oheight )
        img.Rescale(swidth, sheight);

      int cl = 3, bmpType = GL_RGB;
      if( img.HasAlpha() )  {
        cl ++;
        bmpType = GL_RGBA;
      }
      unsigned char* RGBData = new unsigned char[ swidth * sheight * cl];
      for( int i=0; i < sheight; i++ )  {
        for( int j=0; j < swidth; j++ )  {
          int indexa = (i*swidth + (swidth-j-1)) * cl;
          RGBData[indexa] = img.GetRed(j, i);
          RGBData[indexa+1] = img.GetGreen(j, i);
          RGBData[indexa+2] = img.GetBlue(j, i);
          if( cl == 4 )
            RGBData[indexa+3] = img.GetAlpha(j, i);
        }
      }
      TGlTexture* glt = FXApp->GetRender().Background()->GetTexture();
      if( glt != NULL  ) {
        FXApp->GetRender().GetTextureManager().Replace2DTexture(
          *glt, 0, swidth, sheight, 0, bmpType, RGBData);
        glt->SetEnabled(true);
      }
      else  {
        int glti = FXApp->GetRender().GetTextureManager().Add2DTexture(
          "grad", 0, swidth, sheight, 0, bmpType, RGBData);
        FXApp->GetRender().Background()->SetTexture(
          FXApp->GetRender().GetTextureManager().FindTexture(glti));
        glt = FXApp->GetRender().Background()->GetTexture();
        glt->SetEnvMode(tpeDecal);
        glt->SetSCrdWrapping(tpCrdClamp);
        glt->SetTCrdWrapping(tpCrdClamp);

        glt->SetMagFilter(tpFilterNearest);
        glt->SetMinFilter(tpFilterLinear);
        glt->SetEnabled(true);
      }
      delete [] RGBData;
    }
  }
  TStateRegistry::GetInstance().SetState(FXApp->stateGradientOn,
    FXApp->GetRender().Background()->IsVisible(), EmptyString(), true);
}
//..............................................................................
void TMainForm::macSplit(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr cr(Options.FindValue("r", EmptyString()).ToLowerCase());
  TCAtomPList Atoms =
    FXApp->FindCAtoms(Cmds.IsEmpty() ? olxstr("sel") : Cmds.Text(' '), true);
  if( Atoms.IsEmpty() )  return;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  TLattice& latt = FXApp->XFile().GetLattice();
  RefinementModel& rm = FXApp->XFile().GetRM();
  TCAtomPList ProcessedAtoms;
  XVar& var = rm.Vars.NewVar();
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    TCAtom* CA = Atoms[i];
    if( CA->GetEllipsoid() == NULL )  continue;
    vec3d direction;
    double Length = 0;
    olxstr lbl = CA->GetLabel();
    if( CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSY() )  {
      if( CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSZ() )  {
        Length = CA->GetEllipsoid()->GetSX();
        direction = CA->GetEllipsoid()->GetMatrix()[0];
      }
      else  {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    else  {
      if( CA->GetEllipsoid()->GetSY() > CA->GetEllipsoid()->GetSZ() )  {
        Length = CA->GetEllipsoid()->GetSY();
        direction = CA->GetEllipsoid()->GetMatrix()[1];
      }
      else  {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    direction *= Length;
    direction /= 2;
    au.CartesianToCell(direction);
    const double sp = 1./CA->GetDegeneracy();
    TXAtom &XA = FXApp->AddAtom();
    TCAtom& CA1 = XA.CAtom();
    CA1.Assign(*CA);
    CA1.SetSameId(~0);
    CA1.SetPart(1);
    CA1.ccrd() += direction;
    XA.ccrd() = CA1.ccrd();
    CA1.SetLabel(au.CheckLabel(&CA1, lbl+'a'), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA1, catom_var_name_Sof, relation_AsVar, sp);
    CA1.SetOccu(0.5*sp);
    ProcessedAtoms.Add(CA1);
    TCAtom& CA2 = *CA;
    CA2.SetPart(2);
    CA2.ccrd() -= direction;
    CA2.SetLabel(au.CheckLabel(&CA2, lbl+'b'), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA2, catom_var_name_Sof, relation_AsOneMinusVar, sp);
    CA2.SetOccu(0.5*sp);
    ProcessedAtoms.Add(CA2);
    TSimpleRestraint* sr = NULL;
    if( cr.IsEmpty() );
    else if( cr == "eadp" )
      sr = &rm.rEADP.AddNew();
    else if( cr == "isor" )
      sr = &rm.rISOR.AddNew();
    else if( cr == "simu" )
      sr = &rm.rSIMU.AddNew();
    if( sr != NULL )
      sr->AddAtomPair(CA1, NULL, CA2, NULL);
  }
  latt.SetAnis(ProcessedAtoms, false);
  latt.Uniq();
}
//..............................................................................
void TMainForm::macShowP(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIntList parts;
  if( !Cmds.IsEmpty() )  {
    for( size_t i=0; i < Cmds.Count(); i++ )
      parts.Add(Cmds[i].ToInt());
  }
  FXApp->ShowPart(parts, true);
  if( !Options.Contains('m') )
    FXApp->CenterView();
}
//..............................................................................
void TMainForm::macEditAtom(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( Modes->GetCurrent() != NULL )  {
    E.ProcessingError(__OlxSourceInfo, "Please exit all modes before running this command");
    return;
  }
  TCAtomPList CAtoms;
  TXAtomPList Atoms;
  TIns* Ins = NULL;
  try {  Ins = &FXApp->XFile().GetLastLoader<TIns>();  }
  catch(...)  {}  // must be native then...
  
  if( !FindXAtoms(Cmds, Atoms, true, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom names");
    return;
  }
  // synchronise atom names etc
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  RefinementModel& rm = FXApp->XFile().GetRM();
  FXApp->XFile().UpdateAsymmUnit();
  if( Ins != NULL )
    Ins->UpdateParams();
  // get CAtoms and EXYZ equivalents
  SortedPtrList<TResidue, TPointerComparator> residues_to_release;
  au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    TCAtom &ca = Atoms[i]->CAtom();
    if (ca.GetTag() != 0) continue;
    CAtoms.Add(ca)->SetTag(1);
    TExyzGroup* eg = ca.GetExyzGroup();
    if( eg != NULL )  {
      for( size_t j=0; j < eg->Count(); j++ )
        if( !(*eg)[j].IsDeleted() )
          CAtoms.Add((*eg)[j]);
    }
    if (ca.GetResiId() != 0) {
      TResidue& resi = rm.aunit.GetResidue(ca.GetResiId());
      residues_to_release.AddUnique(&resi);
      for (size_t j=0; j < resi.Count(); j++) {
        if (resi[j].GetTag() != 0) continue;
        CAtoms.Add(resi[j])->SetTag(1);
      }
    }
  }
  TXApp::UnifyPAtomList(CAtoms);
  RefinementModel::ReleasedItems released;
  size_t ac = CAtoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    if( olx_is_valid_index(CAtoms[i]->GetSameId()) )  {
      TSameGroup& sg = FXApp->XFile().GetRM().rSAME[CAtoms[i]->GetSameId()];
      if( released.sameList.IndexOf(sg) != InvalidIndex )  continue;
      released.sameList.Add(sg);
      for( size_t j=0; j < sg.Count(); j++ )
        CAtoms.Add(sg[j]);
    }
  }
  // process dependent SAME's
  for( size_t i=0; i < released.sameList.Count(); i++ )  {
    for( size_t j=0; j < released.sameList[i]->DependentCount(); j++ )  {
      TSameGroup& sg = released.sameList[i]->GetDependent(j);
      if( released.sameList.IndexOf(sg) != InvalidIndex )  continue;
      released.sameList.Add( &sg );
      for( size_t k=0; k < sg.Count(); k++ )
        CAtoms.Add(sg[k]);
    }
  }

  for( size_t i=0; i < rm.AfixGroups.Count(); i++ )
    rm.AfixGroups[i].SetTag(0);
  for( size_t i=0; i < CAtoms.Count(); i++ )  {  // add afixed mates and afix parents
    TCAtom& ca = *CAtoms[i];
    for( size_t j=0; j < ca.DependentHfixGroupCount(); j++ )  {
      TAfixGroup& hg = ca.GetDependentHfixGroup(j);
      if( hg.GetTag() != 0 )  continue;
      for( size_t k=0; k < hg.Count(); k++ ) 
        CAtoms.Add(hg[k]);
      hg.SetTag(1);
    }
    if( ca.GetDependentAfixGroup() != NULL && ca.GetDependentAfixGroup()->GetTag() == 0)  {
      for( size_t j=0; j < ca.GetDependentAfixGroup()->Count(); j++ ) 
        CAtoms.Add((*ca.GetDependentAfixGroup())[j]);
      ca.GetDependentAfixGroup()->SetTag(1);
    }
    if( ca.GetParentAfixGroup() != NULL && ca.GetParentAfixGroup()->GetTag() == 0 )  {
      CAtoms.Add( &ca.GetParentAfixGroup()->GetPivot() );
      for( size_t j=0; j < ca.GetParentAfixGroup()->Count(); j++ ) 
        CAtoms.Add((*ca.GetParentAfixGroup())[j]);
      ca.GetParentAfixGroup()->SetTag(1);
    }
  }
  // make sure that the list is unique
  TXApp::UnifyPAtomList(CAtoms);
  FXApp->XFile().GetAsymmUnit().Sort(&CAtoms);
  TStrList SL;
  TStrPObjList<olxstr, TStrList* > RemovedIns;
  SL.Add("REM please do not modify atom names inside the instructions - they will be updated");
  SL.Add("REM by Olex2 automatically, though you can change any parameters");
  SL.Add("REM Also do not change the atoms order");
  // go through instructions
  if( Ins != NULL )  {
    for( size_t i=0; i < Ins->InsCount(); i++ )  {
      // do not process remarks
      if( !Ins->InsName(i).Equalsi("rem") )  {
        const TInsList* InsParams = &Ins->InsParams(i);
        bool found = false;
        for( size_t j=0; j < InsParams->Count(); j++ )  {
          TCAtom* CA1 = InsParams->GetObject(j);
          if( CA1 == NULL )  continue;
          for( size_t k=0; k < CAtoms.Count(); k++ )  {
            TCAtom* CA = CAtoms[k];
            if( CA->GetLabel().Equalsi(CA1->GetLabel()) )  {
              found = true;  break;
            }
          }
          if( found )  break;
        }
        if( found )  {
          SL.Add(Ins->InsName(i)) << ' ' << InsParams->Text(' ');
          TStrList* InsParamsCopy = new TStrList(*InsParams);
          RemovedIns.Add(Ins->InsName(i), InsParamsCopy);
          Ins->DelIns(i--);
        }
      }
    }
    SL.Add();
  }
  TIndexList atomIndex;
  TIns::SaveAtomsToStrings(FXApp->XFile().GetRM(), CAtoms, atomIndex, SL, &released);
  for( size_t i=0; i < released.restraints.Count(); i++ )
    released.restraints[i]->GetParent().Release(*released.restraints[i]);
  for( size_t i=0; i < released.sameList.Count(); i++ )
    released.sameList[i]->GetParent().Release(*released.sameList[i]);
  au.Release(TPtrList<TResidue>(residues_to_release));
  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText(SL.Text('\n'));
  bool undo=false;
  // prevent logging to interfere!
  FXApp->SetDisplayFrozen(true);
  if( dlg->ShowModal() == wxID_OK )  {
    SL.Clear();
    FXApp->XFile().GetRM().Vars.Clear();
    SL.Strtok(dlg->GetText(), '\n');
    TStrList NewIns;
    try {
      TIns::UpdateAtomsFromStrings(FXApp->XFile().GetRM(), atomIndex, SL, NewIns);
      if( Ins != NULL )  {
        for( size_t i=0; i < NewIns.Count(); i++ )  {
          NewIns[i] = NewIns[i].Trim(' ');
          if( NewIns[i].IsEmpty() )  continue;
          Ins->AddIns(NewIns[i], FXApp->XFile().GetRM());
        }
      }
      FXApp->XFile().EndUpdate();
    }
    catch (const TExceptionBase &e) {
      undo = true;
      TBasicApp::NewLogEntry(logExceptionTrace) << e;
    }
  }
  else  {
    undo = true;
  }
  if (undo) {
    au.Restore(TPtrList<TResidue>(residues_to_release));
    if( Ins != NULL )  {
      for( size_t i=0; i < RemovedIns.Count(); i++ ) {
        Ins->AddIns(RemovedIns[i], *RemovedIns.GetObject(i),
          FXApp->XFile().GetRM(), false);
      }
    }
    for( size_t i=0; i < released.restraints.Count(); i++ )
      released.restraints[i]->GetParent().Restore(*released.restraints[i]);
    for( size_t i=0; i < released.sameList.Count(); i++ )
      released.sameList[i]->GetParent().Restore(*released.sameList[i]);
  }
  else {
    for( size_t i=0; i < RemovedIns.Count(); i++ )
      delete (TStrList*)RemovedIns.GetObject(i);
    for( size_t i=0; i < released.restraints.Count(); i++ )  {
      if( released.restraints[i]->GetVarRef(0) != NULL )
        delete released.restraints[i]->GetVarRef(0);
      delete released.restraints[i];
    }
    for( size_t i=0; i < released.sameList.Count(); i++ )  {
      released.sameList[i]->ReleasedClear();
      delete released.sameList[i];
    }
    for (size_t i=0; i < residues_to_release.Count(); i++)
      delete residues_to_release[i];
  }
  dlg->Destroy();
  FXApp->SetDisplayFrozen(false);
}
//..............................................................................
void TMainForm::macEditIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIns& Ins = FXApp->XFile().GetLastLoader<TIns>();
  TStrList SL;
  FXApp->XFile().UpdateAsymmUnit();  // synchronise au's
  Ins.SaveHeader(SL, true, false);
  SL.Add("HKLF ") << Ins.GetRM().GetHKLFStr();

  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText(SL.Text('\n'));
  try  {
    if( dlg->ShowModal() == wxID_OK )  {
      // clear rems, as they are recreated
      for( size_t i=0; i < Ins.InsCount(); i++ )  {
        if( Ins.InsName(i).Equalsi("REM") )  {
          Ins.DelIns(i--);  continue;
        }
      }
      SL.Clear();
      SL.Strtok(dlg->GetText(), '\n');
      Ins.ParseHeader(SL);
      FXApp->XFile().LastLoaderChanged();
      BadReflectionsTable(false);
      UpdateInfoBox();
    }
    else  {
    }
  }
  catch(const TExceptionBase& e)  {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macHklEdit(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr HklFN( FXApp->LocateHklFile() );
  if( !TEFile::Exists(HklFN) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate the HKL file");
    return;
  }
  smatd_list matrices;
  FXApp->GetSymm(matrices);
  TSpaceGroup& sg = FXApp->XFile().GetLastLoaderSG();
  THklFile Hkl;
  Hkl.LoadFromFile(HklFN);
  TRefPList Hkls;
  TStrList SL;
  SL.Add("REM Please put \'-\' char in the front of reflections you wish to omit");
  SL.Add("REM and remove '-' char if you want the reflection to be used in the refinement");
  SL.Add();
  if( Cmds.Count() != 3 && FXApp->CheckFileType<TIns>() )  {
    const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
    if( Lst.IsLoaded() )  {
      for( size_t i=0; i < Lst.DRefCount(); i++ )  {
        olxstr Tmp = "REM    ";
        Tmp << Lst.DRef(i).H << ' ';
        Tmp << Lst.DRef(i).K << ' ';
        Tmp << Lst.DRef(i).L << ' ';
        Tmp << "Delta(F^2)/esd=" << Lst.DRef(i).DF;
        Tmp << " Resolution=" << Lst.DRef(i).Res;
        SL.Add(Tmp);
        Hkls.Clear();
        TReflection ref(Lst.DRef(i).H, Lst.DRef(i).K, Lst.DRef(i).L);
        Hkl.AllRefs(ref, matrices, Hkls);

        for( size_t j=0; j < Hkls.Count(); j++ )
          SL.Add(Hkls[j]->ToNString());
        SL.Add();
      }
    }
  }
  else  {
    TReflection Refl(Cmds[0].ToInt(), Cmds[1].ToInt(), Cmds[2].ToInt());
    Hkls.Clear();
    Hkl.AllRefs(Refl, matrices, Hkls);
    for( size_t i=0; i < Hkls.Count(); i++ )
      SL.Add( Hkls[i]->ToNString());
  }
  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText(SL.Text('\n'));
  if( dlg->ShowModal() == wxID_OK )  {
    olxstr Tmp = dlg->GetText();
    SL.Clear();
    SL.Strtok(Tmp, '\n');
    TReflection R(0, 0, 0);
    for( size_t i=0; i < SL.Count(); i++ )  {
      if( SL[i].ToUpperCase().StartsFrom("REM") )  continue;
      R.FromNString(SL[i]);
      Hkl.UpdateRef(R);
    }
    Hkl.SaveToFile(HklFN);
  }
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macHklView(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !Cmds.Count() )
    FXApp->SetHklVisible(!FXApp->IsHklVisible());
  else
    FXApp->SetHklVisible(Cmds[0].ToBool());
}
//..............................................................................
struct Main_3DIndex  {
  short x,y,z;
  Main_3DIndex(short a, short b, short c) : x(a), y(b), z(c)  {}
  Main_3DIndex()  { }
};
typedef TTypeList< Main_3DIndex > T3DIndexList;
bool InvestigateVoid(short x, short y, short z, TArray3D<short>& map, T3DIndexList& points)  {
const index_t mapX = map.Length1(),
              mapY = map.Length2(),
              mapZ = map.Length3();
  short*** D = map.Data;
  const short refVal = D[x][y][z]-1;
  // skip the surface points
  if( refVal < 0 )  return false;
  D[x][y][z] = -D[x][y][z];
  for( short ii = -1; ii <= 1; ii++)  {
    for( short jj = -1; jj <= 1; jj++)  {
      for( short kk = -1; kk <= 1; kk++)  {
        if( (ii|jj|kk) == 0 )  continue;
        int iind = x+ii,
            jind = y+jj,
            kind = z+kk;
        if( iind < 0 )  iind += mapX;
        if( jind < 0 )  jind += mapY;
        if( kind < 0 )  kind += mapZ;
        if( iind >= mapX )  iind -= mapX;
        if( jind >= mapY )  jind -= mapY;
        if( kind >= mapZ )  kind -= mapZ;
        if( D[iind][jind][kind] <= 0 )  {
          continue;
        }
        else if( D[iind][jind][kind] >= refVal )  {
          points.Add( new Main_3DIndex(iind, jind, kind) );
        }
      }
    }
  }
  return true;
}
//
void TMainForm::macCalcVoid(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  ElementRadii radii;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadVdWRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
  TCAtomPList catoms;
  // consider the selection if any
  TGlGroup& sel = FXApp->GetSelection();
  for( size_t i=0; i < sel.Count(); i++ )  {
    if( EsdlInstanceOf(sel[i], TXAtom) ) 
      catoms.Add(((TXAtom&)sel[i]).CAtom())->SetTag(catoms.Count());
  }
  catoms.Pack(ACollectionItem::IndexTagAnalyser());
  if( catoms.IsEmpty() ) {
    TBasicApp::NewLogEntry() <<
     "Calculating for all atoms of the asymmetric unit";
  }
  else  {
    TBasicApp::NewLogEntry() << "Calculating for " <<
      olx_analysis::alg::label(catoms);
  }
  olxstr_dict<olxstr> rv;
  double surfdis = rv('d', Options.FindValue('d', '0')).ToDouble();
  
  TBasicApp::NewLogEntry() << "Extra distance from the surface: " << surfdis;
  
  double resolution = Options.FindValue("r", "0.2").ToDouble();
  if( resolution < 0.01 )  
    resolution = 0.01;
  rv('r', resolution);
  resolution = 1./resolution;
  const vec3i dim(au.GetAxes()*resolution);
  const double mapVol = dim.Prod();
  const double vol = FXApp->XFile().GetLattice().GetUnitCell().CalcVolume();
  const int minLevel = olx_round(pow(6*mapVol*3/(4*M_PI*vol), 1./3));
  FXApp->XGrid().Clear();  // release the occupied memory
  TArray3D<short> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  map.FastInitWith(10000);
  if( Options.Contains('p') )
    FXApp->XFile().GetUnitCell().BuildDistanceMap_Direct(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  else  {
    FXApp->XFile().GetUnitCell().BuildDistanceMap_Masks(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  }
  size_t structureGridPoints = 0;
  TIntList levels;
  short*** amap = map.Data;
  short MaxLevel = 0;
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )  {
        if( amap[i][j][k] > MaxLevel )
          MaxLevel = amap[i][j][k];
        if( amap[i][j][k] < 0 )
          structureGridPoints++;
        else  {
          while( levels.Count() <= (size_t)amap[i][j][k] )
            levels.Add(0);
          levels[amap[i][j][k]]++;
        }
      }
    }
  }
  vec3d_list void_centers =
    MapUtil::FindLevelCenters<short>(amap, dim, MaxLevel, -MaxLevel);
  const vec3i MaxXCh = MapUtil::AnalyseChannels1(map.Data, dim, MaxLevel);
  for( int i=0; i < 3; i++ )  {
    if( MaxXCh[i] != 0 )
      TBasicApp::NewLogEntry() << (olxstr((olxch)('a'+i)) <<
        " direction can be penetrated by a sphere of " <<
        olxstr::FormatFloat(2, MaxXCh[i]/resolution) << "A radius");
  }
  TBasicApp::NewLogEntry() << "Cell volume (A^3) " << olxstr::FormatFloat(3, vol);
  TBasicApp::NewLogEntry() << "Radius [ volume ] of the largest spherical void is " <<
    olxstr::FormatFloat(2, (double)MaxLevel/resolution) <<
      " A [ " << olxstr::FormatFloat(2, olx_sphere_volume((double)MaxLevel/resolution))
      << " A^3 ]";
  TBasicApp::NewLogEntry() << "The void center(s) are at (fractional):";
  for (size_t i=0; i < void_centers.Count(); i++) {
    olxstr l1;
    for (int j=0; j < 3; j++)
      l1 << ' ' << olxstr::FormatFloat(-3, void_centers[i][j]);
    TBasicApp::NewLogEntry() << l1;
  }
  TBasicApp::NewLogEntry() << (catoms.IsEmpty() ? "Structure occupies" : "Selected atoms occupy")
    << " (A^3) " << olxstr::FormatFloat(2, structureGridPoints*vol/mapVol) 
    << " (" << olxstr::FormatFloat(2, structureGridPoints*100/mapVol) << "%)";

  double totalVol = 0;
  for( size_t i=levels.Count()-1; olx_is_valid_index(i); i-- )  {
    totalVol += levels[i];
    TBasicApp::NewLogEntry() << "Level " << i << " is " <<
      olxstr::FormatFloat(1, (double)i/resolution) << "A away from the surface " <<
      olxstr::FormatFloat(3, totalVol*vol/mapVol) << "(A^3)";
  }
  if( Options.Contains('i') )  {
    for( int i=0; i < dim[0]; i++ )  {
      for( int j=0; j < dim[1]; j++ )  {
        for( int k=0; k < dim[2]; k++ )
          amap[i][j][k] = MaxLevel-amap[i][j][k];
      }
    }
  }
  //// set map to view voids
  FXApp->XGrid().InitGrid(dim[0], dim[1], dim[2]);
  FXApp->XGrid().SetMinVal(0);
  FXApp->XGrid().SetMaxVal((float)MaxLevel/resolution);
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )
        FXApp->XGrid().SetValue(i, j, k, (float)map.Data[i][j][k]/resolution);
    }
  }
  FXApp->XGrid().AdjustMap();
  FXApp->XGrid().InitIso();
  FXApp->ShowGrid(true, EmptyString());
  TBasicApp::NewLogEntry();
  //E.SetRetVal(XLibMacros::NAString);
}
//..............................................................................
void TMainForm::macViewGrid(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr gname;
  if( FXApp->XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    gname = TEFile::ExtractFilePath(FXApp->XFile().GetFileName());
    TEFile::AddPathDelimeterI(gname) << "map.txt";
  }
  else if( !Cmds.IsEmpty() )
    gname = Cmds[0];
  FXApp->ShowGrid(true, gname);
}
//..............................................................................
void TMainForm::macHklExtract(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
  TGlGroup& sel = FXApp->GetSelection();
  if( sel.Count() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please select some reflections");
    return;
  }
  TRefPList Refs;
  for( size_t i=0; i < sel.Count(); i++ )  {
    AGDrawObject& obj = sel[i];
    if( EsdlInstanceOf(obj, TXReflection) )
      ;//Refs.Add( ((TXReflection*)obj)->Reflection() );
  }
  if( Refs.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "please select some reflections");
    return;
  }
  THklFile::SaveToFile(Cmds[0], Refs, true);
}
//..............................................................................
void TMainForm::macDirection(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  const mat3d Basis = FXApp->GetRender().GetBasis().GetMatrix();
  const vec3d Z(Basis[0][2], Basis[1][2], Basis[2][2]);
  if( FXApp->XFile().HasLastLoader() )  {
    TAsymmUnit &au = FXApp->XFile().GetAsymmUnit();
    mat3d m = au.GetCellToCartesian();
    vec3d fZ = au.Fractionalise(Z).Normalise();
    double min_v = 100;
    for (int i=0; i < 3; i++) {
      if (fZ[i] != 0 && olx_abs(fZ[i]) < min_v)
        min_v = olx_abs(fZ[i]);
    }
    if (min_v >= 1e-4)
      fZ /= min_v;
    olxstr Tmp =  "Direction: (";
    Tmp << olxstr::FormatFloat(3, fZ[0]) << "*A, " <<
      olxstr::FormatFloat(3, fZ[1]) << "*B, " <<
      olxstr::FormatFloat(3, fZ[2]) << "*C)";
    TBasicApp::NewLogEntry() << Tmp;
    const char *Dir[] = {"000", "100", "010", "001", "110", "101", "011", "111"};
    TTypeList<vec3d> Points;
    Points.AddNew();
    Points.AddCopy(m[0]);
    Points.AddCopy(m[1]);
    Points.AddCopy(m[2]);
    Points.AddCopy(m[0] + m[1]);
    Points.AddCopy(m[0] + m[2]);
    Points.AddCopy(m[1] + m[2]);
    Points.AddCopy(m[0] + m[1] + m[2]);
    for( size_t i=0; i < Points.Count(); i++ )  {
      for( size_t j=i+1; j < Points.Count(); j++ )  {
        double d = (Points[j]-Points[i]).Normalise().DistanceTo(Z)/2;
        if (d < 0.05) {
          Tmp = "View along ";
          Tmp << Dir[i] <<  '-' <<  Dir[j] << ' ' << '(' <<
            "normalised deviation: " <<  olxstr::FormatFloat(3, d) << "A)";
          TBasicApp::NewLogEntry() << Tmp;
        }
      }
    }
    if( !FXApp->XGrid().IsEmpty() && FXApp->XGrid().IsVisible() && 
      (FXApp->XGrid().GetRenderMode()&(planeRenderModeContour|planeRenderModePlane)) != 0 )
    {
      const vec3f center(FXApp->GetRender().GetBasis().GetCenter());
      vec3f p(0, 0, FXApp->XGrid().GetDepth());
      p = au.Fractionalise(Basis*p - center);
      olxstr Tmp =  "Grid center: (";
      Tmp << olxstr::FormatFloat(3, p[0]) << "*A, " <<
             olxstr::FormatFloat(3, p[1]) << "*B, " <<
             olxstr::FormatFloat(3, p[2]) << "*C)";
      TBasicApp::NewLogEntry() << Tmp;
    }
  }
  else  {
    olxstr Tmp =  "Normal: (";
    Tmp << olxstr::FormatFloat(3, Z[0]) << ", " <<
           olxstr::FormatFloat(3, Z[1]) << ", " <<
           olxstr::FormatFloat(3, Z[2]) << ')';
    TBasicApp::NewLogEntry() << Tmp;
  }
}
//..............................................................................
void TMainForm::macUndo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FXApp->GetUndo().isEmpty() )  {
    TUndoData* data = FXApp->GetUndo().Pop();
    data->Undo();
    delete data;
  }
}
//..............................................................................
void TMainForm::macIndividualise(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TXAtomPList atoms;
    TXBondPList bonds;
    TGlGroup& sel = FXApp->GetSelection();
    for( size_t i=0; i < sel.Count(); i++ )  {
      if( EsdlInstanceOf(sel[i], TXAtom) )
        atoms.Add((TXAtom&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXBond) )
        bonds.Add((TXBond&)sel[i]);
    }
    FXApp->Individualise(atoms);
    FXApp->Individualise(bonds);
  }
  else
    FXApp->Individualise(FindXAtoms(Cmds, false, false).GetObject());
}
//..............................................................................
void TMainForm::macCollectivise(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TGlGroup& glg = FXApp->GetSelection();
    TXAtomPList atoms;
    TXBondPList bonds;
    for( size_t i=0; i < glg.Count(); i++ )  {
      if( EsdlInstanceOf(glg[i], TXAtom) )
        atoms.Add((TXAtom&)glg[i]);
      else if( EsdlInstanceOf(glg[i], TXBond) )
        bonds.Add((TXBond&)glg[i]);
    }
    FXApp->Collectivise(atoms);
    FXApp->Collectivise(bonds);
  }
  else
    FXApp->Collectivise(FindXAtoms(Cmds, false, false).GetObject());
}
//..............................................................................
void TMainForm::macSel(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Modes->GetCurrent() != NULL )  {
    TBasicApp::NewLogEntry(logError) << "Unavailable in a mode";
    return;
  }
  if( Cmds.Count() >= 1 && Cmds[0].Equalsi("frag") )  {
    TNetPList nets(FindXAtoms(Cmds.SubListFrom(1), false, false),
      FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
    FXApp->SelectFragments(ACollectionItem::Unique(nets), !Options.Contains('u'));
  }
  else if( Cmds.Count() == 1 && Cmds[0].Equalsi("res") )  {
    //FXApp->GetRender().ClearSelection();
    //TStrList out;
    //TCAtomPList a_res;
    //TPtrList<TSimpleRestraint> b_res;
    //RefinementModel& rm = FXApp->XFile().GetRM();
    //rm.Describe(out, &a_res, &b_res);
    //FXApp->XFile().GetAsymmUnit().GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    //for( size_t i=0; i < a_res.Count(); i++ )
    //  a_res[i]->SetTag(1);
    //TGXApp::AtomIterator ai = FXApp->GetAtoms();
    //while( ai.HasNext() )  {
    //  TXAtom& xa = ai.Next();
    //  if( xa.CAtom().GetTag() != 1 )  continue;
    //  if( xa.IsSelected() )  continue;
    //  FXApp->GetRender().Select(xa);
    //}
    //for( size_t i=0; i < b_res.Count(); i++ )  {
    //  const TSimpleRestraint& res = *b_res[i];
    //  for( size_t j=0; j < res.AtomCount(); j+=2 )  {
    //    if( res.GetAtom(j).GetMatrix() != NULL || res.GetAtom(j+1).GetMatrix() != NULL )  continue;
    //    const size_t id1 = res.GetAtom(j).GetAtom()->GetId();
    //    const size_t id2 = res.GetAtom(j+1).GetAtom()->GetId();
    //    TGXApp::BondIterator bi = FXApp->GetBonds();
    //    while( bi.HasNext() )  {
    //      TXBond& xb = bi.Next();
    //      if( xb.IsSelected() )  continue;
    //      const TCAtom& ca1 = xb.A().CAtom();
    //      const TCAtom& ca2 = xb.B().CAtom();
    //      if( (ca1.GetId() == id1 && ca2.GetId() == id2) ||
    //          (ca1.GetId() == id2 && ca2.GetId() == id1) )  
    //      {
    //        FXApp->GetRender().Select(xb);
    //        break;
    //      }
    //    }
    //  }
    //}
    //FGlConsole->PrintText(out);
    //FXApp->GetLog() << NewLineSequence();
  }
  else if( Cmds.Count() > 1 && Cmds[0].Equalsi("resi") )  {
    TAsymmUnit &au = FXApp->XFile().GetAsymmUnit();
    au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    TSizeList nums;
    TStrList names;
    for (size_t i=1; i < Cmds.Count(); i++) {
      if (Cmds[i].IsInt())
        nums << Cmds[i].ToSizeT();
      else
        names << Cmds[i];
    }
    for (size_t i=0; i < au.ResidueCount(); i++) {
      TResidue &r = au.GetResidue(i);
      bool found = false;
      for (size_t j=0; j < nums.Count(); j++) {
        if (nums[j] == r.GetNumber() || nums[j] == r.GetAlias()) {
          found = true;
          break;
        }
      }
      if (!found) {
        for (size_t j=0; j < names.Count(); j++) {
          if (r.GetClassName().Equalsi(names[j])) {
            found = true;
            break;
          }
        }
      }
      if (!found) continue;
      for (size_t j=0; j < r.Count(); j++)
        r[j].SetTag(1);

    }
    TGXApp::AtomIterator ai = FXApp->GetAtoms();
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.IsVisible() && a.CAtom().GetTag() == 1)
        FXApp->GetRender().Select(a, true);
    }
  }
  else if( Cmds.Count() == 1 && TSymmParser::IsRelSymm(Cmds[0]) )  {
    bool invert = Options.Contains('i'), unselect=false;
    if( !invert )
      unselect = Options.Contains('u');
    const smatd matr = TSymmParser::SymmCodeToMatrix(FXApp->XFile().GetUnitCell(), Cmds[0]);
    TGXApp::AtomIterator ai = FXApp->GetAtoms();
    while( ai.HasNext() )  {
      TXAtom& a = ai.Next();
      if( a.IsDeleted() || !a.IsVisible() )  continue;
      if( a.IsGenerator(matr) )  {
        if( invert )
          FXApp->GetRender().Select(a, !a.IsSelected());
        else if( unselect )
          FXApp->GetRender().Select(a, false);
        else
          FXApp->GetRender().Select(a, true);
      }
    }
    TGXApp::BondIterator bi = FXApp->GetBonds();
    while( bi.HasNext() )  {
      TXBond& b = bi.Next();
      if( b.IsDeleted() || !b.IsVisible() )  continue;
      if( b.A().IsGenerator(matr) && b.B().IsGenerator(matr) )  {
        if( invert )
          FXApp->GetRender().Select(b, !b.IsSelected());
        else if( unselect )
          FXApp->GetRender().Select(b, false);
        else
          FXApp->GetRender().Select(b, true);
      }
    }
  }
  else if( Cmds.Count() > 1 && Cmds[0].Equalsi("part") )  {
    Cmds.Delete(0);
    TIntList parts;
    for( size_t i=0; Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        parts.Add(Cmds[i].ToInt());
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if( !parts.IsEmpty() )  {
      olxstr cond = "xatom.part==";
      cond << parts[0];
      for( size_t i=1; i < parts.Count(); i++ )
        cond << "||xatom.part==" << parts[i];
      FXApp->SelectAtomsWhere(cond);
    }
  }
  else if( Cmds.Count() > 1 && Cmds[0].Equalsi("fvar") )  {
    Cmds.Delete(0);
    TIntList fvars;
    for( size_t i=0; Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        fvars.Add(Cmds[i].ToInt()-1);
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if( !fvars.IsEmpty() )  {
      TGXApp::AtomIterator ai = FXApp->GetAtoms();
      while( ai.HasNext() )  {
        TXAtom &a = ai.Next();
        XVarReference *ref = a.CAtom().GetVarRef(catom_var_name_Sof);
        if( ref == NULL )  continue;
        int v = int(ref->Parent.GetId());
        if( ref->relation_type == relation_AsOneMinusVar )
          v *= -1;
        for( size_t i=0; i < fvars.Count(); i++ )  {
          if( fvars[i] == v )
            FXApp->GetRender().Select(a, true);
        }
      }
    }
  }
  else if( Cmds.Count() == 1 && Cmds[0].Equalsi("isot") )  {
    TGXApp::AtomIterator ai = FXApp->GetAtoms();
    while( ai.HasNext() )  {
      TXAtom& xa = ai.Next();
      if( xa.GetEllipsoid() == NULL )
        FXApp->GetRender().Select(xa, true);
    }
  }
  else if( Cmds.Count() == 1 && Cmds[0].Equalsi("wbox") )  {
    TSAtomPList atoms;
    if( FXApp->FindSAtoms(EmptyString(), atoms, true) )  {
      const WBoxInfo bs = TXApp::CalcWBox(atoms, NULL, TSAtom::weight_occu);
      T3DFrameCtrl& fr = FXApp->Get3DFrame();
      vec3d nx = bs.normals[0]*bs.s_from[0];
      vec3d px = bs.normals[0]*bs.s_to[0];
      vec3d ny = bs.normals[1]*bs.s_from[1];
      vec3d py = bs.normals[1]*bs.s_to[1];
      vec3d nz = bs.normals[2]*bs.s_from[2];
      vec3d pz = bs.normals[2]*bs.s_to[2];
      if( ny.XProdVec(nx).CAngle(nz) < 0 )  {
        olx_swap(nx, px);
        olx_swap(ny, py);
        olx_swap(nz, pz);
      }
      fr.SetEdge(0, nx+ny+nz);
      fr.SetEdge(1, nx+py+nz);
      fr.SetEdge(2, px+py+nz);
      fr.SetEdge(3, px+ny+nz);
      fr.SetEdge(4, nx+ny+pz);
      fr.SetEdge(5, nx+py+pz);
      fr.SetEdge(6, px+py+pz);
      fr.SetEdge(7, px+ny+pz);
      fr.UpdateEdges();
      fr.Translate(bs.center);
      FXApp->SetGraphicsVisible(&fr, true);
    }
  }
  else if( !(Options.Contains('a') ||
    Options.Contains('i') ||
    Options.Contains('u')))
  {
    size_t period=5;
    TXAtomPList Atoms = FXApp->FindXAtoms("sel", false, false);
    if( Atoms.IsEmpty() && Cmds.Count() == 1 )  {
      TGPCollection* gpc = FXApp->GetRender().FindCollection(Cmds[0]);
      if( gpc != NULL )  {
        for( size_t i=0; i < gpc->ObjectCount(); i++ )
          FXApp->GetRender().Select(gpc->GetObject(i));
        return;
      }
    }
    for( size_t i=0; i <= Atoms.Count(); i+=period )  {
      olxstr Tmp;
      for( size_t j=0; j < period; j++ )  {
        if( (j+i) >= Atoms.Count() )  break;
        Tmp << Atoms[i+j]->GetGuiLabel();
        Tmp.RightPadding((j+1)*14, ' ', true);
      }
      if( !Tmp.IsEmpty() )
        TBasicApp::NewLogEntry() << Tmp;
    }
    if( !Cmds.IsEmpty() )  {
      size_t whereIndex = Cmds.IndexOf("where");
      if( whereIndex >= 1 && whereIndex != InvalidIndex)  {
        olxstr Tmp = Cmds[whereIndex-1];
        while( olx_is_valid_index(whereIndex) )  {  Cmds.Delete(whereIndex--);  }
        if( Tmp.Equalsi("atoms") )
          FXApp->SelectAtomsWhere(Cmds.Text(' '));
        else if( Tmp.Equalsi("bonds") )
          FXApp->SelectBondsWhere(Cmds.Text(' '));
        else
          Error.ProcessingError(__OlxSrcInfo, "undefined keyword: " ) << Tmp;
        return;
      }
      else  {
        size_t ringsIndex = Cmds.IndexOf("rings");
        if( ringsIndex != InvalidIndex )  {
          Cmds.Delete( ringsIndex );
          FXApp->SelectRings(Cmds.Text(' '));
        }
        else
          FXApp->SelectAtoms(Cmds.Text(' '));
        return;
      }
    }
    olxstr seli = FXApp->GetSelectionInfo(Options.Contains('l'));
    if( !seli.IsEmpty() ) {
      FXApp->NewLogEntry() << seli;
      if (Options.Contains('c'))
        FXApp->ToClipboard(seli);
    }
  }
  else  {
    for( size_t i=0; i < Options.Count(); i++ )  {
      if( Options.GetName(i)[0] == 'a' )
        FXApp->SelectAll(true);
      else if( Options.GetName(i)[0] == 'u' )
        FXApp->SelectAll(false);
      else if( Options.GetName(i)[0] == 'i' )  {
        if( Cmds.Count() == 0 )
          FXApp->GetRender().InvertSelection();
        else
          FXApp->SelectAtoms(Cmds.Text(' '), true);
      }
      else
        Error.ProcessingError(__OlxSrcInfo, "wrong options");
    }
  }
}
//..............................................................................
void TMainForm::macReap(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // a Open dialog appearing breaks the wxWidgets sizing...
  if( !IsShown() && Cmds.IsEmpty() )  return;
  olxstr cmdl_fn = TOlxVars::FindValue("olx_reap_cmdl");
  if (!cmdl_fn.IsEmpty()) {
    TOlxVars::UnsetVar("olx_reap_cmdl");
    if (TEFile::Exists(cmdl_fn)) {
      Cmds.Clear();
      Cmds << cmdl_fn;
    }
  }
  SetSGList(EmptyString());
  TXFile::NameArg file_n;
  bool Blind = Options.Contains('b'); // a switch showing if the last file is remembered
  bool ReadStyle = !Options.Contains('r');
  bool OverlayXFile = Options.Contains('*');
  if( Cmds.Count() >= 1 && !Cmds[0].IsEmpty() )  {  // merge the file name if a long one...
    file_n = TXFile::ParseName(TEFile::ExpandRelativePath(Cmds.Text(' ')));
    if( TEFile::UnixPath(file_n.file_name).StartsFrom("http://") )  {
      TUrl url(TEFile::UnixPath(file_n.file_name));
      TStrList files;
      files << file_n.file_name;
      // loking for COD urls
      if (file_n.file_name.EndsWithi(".cif")) {
        files << olxstr(file_n.file_name).Replace("cif", "hkl");
      }
      olxstr dest_dir = getDataDir() + "web/";
      if (DownloadFiles(files, dest_dir) > 0) {
        olxstr dest_fn = dest_dir + TEFile::ExtractFileName(url.GetPath());
        if (TEFile::Exists(dest_fn)) {
          Macros.ProcessMacro(olxstr("@reap '") << dest_fn << '\'', Error);
        }
      }
      else
        Error.ProcessingError(__OlxSrcInfo, "Could not locate specified file");
      return;
    }
    if( !file_n.data_name.IsEmpty() && file_n.file_name.IsEmpty() )
      file_n.file_name = FXApp->XFile().GetFileName();
    if( TEFile::ExtractFileExt(file_n.file_name).IsEmpty() )  {
      olxstr res_fn = TEFile::ChangeFileExt(file_n.file_name, "res"),
             ins_fn = TEFile::ChangeFileExt(file_n.file_name, "ins");
      if( TEFile::Exists(res_fn) )  {
        if( TEFile::Exists(ins_fn) )
          file_n.file_name = TEFile::FileAge(ins_fn) < TEFile::FileAge(res_fn) ? res_fn : ins_fn;
        else
          file_n.file_name = res_fn;
      }
      else
        file_n.file_name = ins_fn;
    }
#ifdef __WIN32__ // tackle short path names problem
    WIN32_FIND_DATA wfd;
    ZeroMemory(&wfd, sizeof(wfd));
    HANDLE fsh = FindFirstFile(file_n.file_name.u_str(), &wfd);
    if( fsh != INVALID_HANDLE_VALUE )  {
      file_n.file_name = TEFile::ExtractFilePath(file_n.file_name);
      if( !file_n.file_name.IsEmpty() )
        TEFile::AddPathDelimeterI(file_n.file_name);
      file_n.file_name << wfd.cFileName;
      FindClose(fsh);
    }
#endif // win32
    if( TEFile::ExtractFilePath(file_n.file_name).IsEmpty() )
      file_n.file_name = XLibMacros::CurrentDir + file_n.file_name;
  }
  else  {
    if( !IsVisible() )  return;
    FileFilter ff;
    ff.AddAll("ins;cif;res;xyz;p4p;crs;pdb;fco;fcf;hkl");
    ff.Add("*.mol", "MDL MOL");
    ff.Add("*.mas", "XD master");
    ff.Add("*.mol2", "Tripos MOL2");
    if( !OverlayXFile )
      ff.Add("*.oxm", "Olex2 model");
    file_n.file_name = PickFile("Open File",
      ff.GetString(),
      XLibMacros::CurrentDir, true);
  }
  // the dialog has been successfully executed
  if( !file_n.file_name.IsEmpty() )  {
    /* FN might be a dir on windows when a file does not exist - the code above
    will get the folder name instead...
    */
    if( !TEFile::Exists(file_n.file_name) || TEFile::IsDir(file_n.file_name) )  {
      Error.ProcessingError(__OlxSrcInfo, "Could not locate specified file");
      return;
    }
    if( OverlayXFile )  {
      TXFile& xf = FXApp->NewOverlayedXFile();
      xf.LoadFromFile(TXFile::ComposeName(file_n));
      FXApp->CreateObjects(false);
      FXApp->AlignOverlayedXFiles();
      FXApp->CenterView(true);
      return;
    }
    if( Modes->GetCurrent() != NULL )
      Macros.ProcessMacro("mode off", Error);
    olxstr ds_fn = TEFile::ChangeFileExt(file_n.file_name, "xlds");
    if( TEFile::Exists(ds_fn) )  {
      Macros.ProcessMacro(olxstr("load view '") <<
        TEFile::ChangeFileExt(file_n.file_name, EmptyString()) << '\'', Error);
    }
    else  {
      if( TEFile::Exists(DefStyle) && ReadStyle )
        FXApp->GetRender().GetStyles().LoadFromFile(DefStyle, false);
    }
    // delete the Space groups information file
    if( !(TEFile::ChangeFileExt(file_n.file_name, EmptyString()) ==
      TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), EmptyString())) )
    {
      TEFile::DelFile(FXApp->GetInstanceDir()+"spacegroups.htm");
    }
    // special treatment of the kl files
    if( TEFile::ExtractFileExt(file_n.file_name).Equalsi("hkl") )  {
      if( !TEFile::Exists(TEFile::ChangeFileExt(file_n.file_name, "ins") ) )  {
        THklFile hkl;
        bool ins_initialised = false;
        TIns* ins = (TIns*)FXApp->XFile().FindFormat("ins");
        //ins->Clear();
        hkl.LoadFromFile(file_n.file_name, ins, &ins_initialised);
        if( !ins_initialised )  {
          olxstr src_fn = TEFile::ChangeFileExt(file_n.file_name, "p4p");
          if( !TEFile::Exists(src_fn) )
            src_fn = TEFile::ChangeFileExt(file_n.file_name, "crs");
          if( !TEFile::Exists(src_fn) )  {
            Error.ProcessingError(__OlxSrcInfo,
              "could not initialise CELL/SFAC from the hkl file");
            return;
          }
          else
            file_n.file_name = src_fn;
        }
        else  {
          FXApp->XFile().SetLastLoader(ins);
          FXApp->XFile().LastLoaderChanged();
          // make sure tha SGE finds the related HKL
          FXApp->XFile().GetRM().SetHKLSource(file_n.file_name);
          TMacroError er;
          Macros.ProcessMacro(olxstr("SGE '") <<
            TEFile::ChangeFileExt(file_n.file_name, "ins") << '\'', er);
          if( !er.HasRetVal() || !er.GetRetObj< TEPType<bool> >()->GetValue() )  {
            olxstr
              s_inp("getuserinput(1, \'Please, enter the spacegroup\', \'')"),
              s_sg(s_inp);
            TSpaceGroup* sg = NULL;
            while( sg == NULL )  {
              processFunction(s_sg);
              sg = TSymmLib::GetInstance().FindGroupByName(s_sg);
              if( sg != NULL ) break;
              s_sg = s_inp;
            }
            ins = (TIns*)FXApp->XFile().FindFormat("ins");
            ins->GetAsymmUnit().ChangeSpaceGroup(*sg);
            if( ins->GetRM().GetUserContent().IsEmpty() )  {
              s_inp = "getuserinput(1, \'Please, enter cell content\', \'C1')";
              processFunction(s_inp);
              ins->GetRM().SetUserFormula(s_inp);
            }
            else  {
              size_t sfac_count = ins->GetRM().GetUserContent().Count();
              TStrList unit;
              for( size_t i=0; i < sfac_count; i++ ) {
                unit.Add((sg->MatrixCount()+1)*
                  (sg->GetLattice().GetVectors().Count()+1));
              }
              ins->GetRM().SetUserContentSize(unit);
              ins->GetAsymmUnit().SetZ((sg->MatrixCount()+1)*
                (sg->GetLattice().GetVectors().Count()+1));
            }
            ins->SaveForSolution(
              TEFile::ChangeFileExt(file_n.file_name, "ins"),
              EmptyString(), EmptyString(), false);
            Macros.ProcessMacro( olxstr("reap '") <<
              TEFile::ChangeFileExt(file_n.file_name, "ins") << '\'', Error);
            Macros.ProcessMacro("solve", Error);
          }  // sge, if succeseded will run reap and solve
          return;
        }
      }
      else
        file_n.file_name = TEFile::ChangeFileExt(file_n.file_name, "ins");
    }
    try  {
      bool update_vfs =
        TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) !=
        TEFile::ExtractFilePath(file_n.file_name);
      if (update_vfs) {
        SaveVFS(plStructure); // save virtual fs file
        TFileHandlerManager::Clear(plStructure);
      }
      int64_t st = TETime::msNow();
      FXApp->LoadXFile(TXFile::ComposeName(file_n));
      st = TETime::msNow() - st;
      TBasicApp::NewLogEntry(logInfo) << "Structure loaded in " << st << " ms"
        << NewLineSequence();
      BadReflectionsTable(false, false);
      RefineDataTable(false, false);
      if (update_vfs)
        LoadVFS(plStructure);  // load virtual fs file
    }
    catch (const TExceptionBase& exc) { 
      // manual recovery of the situation...
      FXApp->ClearStructureRelated();
      FXApp->XFile().GetRM().Clear(rm_clear_ALL);
      FXApp->XFile().GetLattice().Clear(true);
      FXApp->XFile().SetLastLoader(NULL);
      FXApp->CreateObjects(true);
      // following shelx, try relaod the ins file?
      bool is_ok = false;
      if (TEFile::ExtractFileExt(file_n.file_name).Equalsi("res")) {
        file_n.file_name = TEFile::ChangeFileExt(file_n.file_name, "ins");
        if (TEFile::Exists(file_n.file_name)) {
          try {
            FXApp->LoadXFile(TXFile::ComposeName(file_n));
            TBasicApp::NewLogEntry(logError) << "Last operation was not"
              " successful, the INS file is reloaded, please resolve the "
              "conflicts and re-run the process again";
            is_ok = true;
          }
          catch (const TExceptionBase &e) {}
        }
      }
      if (!is_ok)
        throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    if( FXApp->XFile().HasLastLoader() )  {
      FInfoBox->Clear();
      if( FXApp->CheckFileType<TP4PFile>() ||
        FXApp->CheckFileType<TCRSFile>() )
      {
        if (TBasicApp::GetInstance().GetOptions().FindValue(
          "p4p_automate", FalseString()).ToBool())
        {
          TMacroError er;
          if( TEFile::Exists(TEFile::ChangeFileExt(file_n.file_name, "ins")))
            Macros.ProcessMacro("SG", er);
          else
            Macros.ProcessMacro("SGE", er);
        }
        else if (FXApp->CheckFileType<TP4PFile>()) {
          TP4PFile &p4p = FXApp->XFile().GetLastLoader<TP4PFile>();
          RunWhenVisibleTasks.Add(new P4PTask(p4p));
        }
      }
    // automatic export for kappa cif
      if( FXApp->CheckFileType<TCif>() )  {
        TCif& cif = FXApp->XFile().GetLastLoader<TCif>();
        if( cif.BlockCount() > 1 )  {
          FXApp->NewLogEntry() << "The following data blocks are available:";
          for( size_t i=0; i < cif.BlockCount(); i++ )
            FXApp->NewLogEntry() << '#' << i << ": " << cif.GetBlock(i).GetName();
        }
        if( !file_n.data_name.IsEmpty() )
          FXApp->NewLogEntry() << "Loading: " << file_n.data_name;
        FXApp->Draw();
        olxstr hklFileName = TEFile::ChangeFileExt(file_n.file_name, "hkl");
        olxstr insFileName = TEFile::ChangeFileExt(file_n.file_name, "ins");
        TMacroError er;
        if( !TEFile::Exists(hklFileName)  )  {
          size_t block_index = cif.GetBlockIndex();
          if( cif.FindLoopGlobal("_refln", true) != NULL )  {
            er.SetRetVal(&cif);
            Macros.ProcessMacro(olxstr("export ").quote() << hklFileName, er);
            if( !er.IsProcessingError() )  {
              if( !TEFile::Exists(insFileName) )  {
                TIns ins;
                ins.Adopt(FXApp->XFile());
                ins.GetRM().SetHKLSource(hklFileName);
                ins.SaveToFile( insFileName );
                Macros.ProcessMacro(olxstr("@reap \'") << insFileName << '\'', er);
                if( !er.IsProcessingError() )
                  Macros.ProcessMacro("reset", er);
                FXApp->Draw();
                return;
              }
            }
            else
              AnalyseError(er);
            cif.SetCurrentBlock(block_index);
          }
        }
      }
      UpdateInfoBox();
      // check if the associated HKL file has the same name and location
      olxstr 
        hkl_fn = TEFile::OSPath(FXApp->XFile().GetRM().GetHKLSource())
          .DeleteSequencesOf(TEFile::GetPathDelimeter()), 
        src_fn = TEFile::OSPath(FXApp->XFile().LastLoader()->GetFileName())
          .DeleteSequencesOf(TEFile::GetPathDelimeter()); 
#ifdef __WIN32__
      if( !TEFile::ChangeFileExt(hkl_fn, EmptyString()).Equalsi(
          TEFile::ChangeFileExt(src_fn, EmptyString())) )
      {
#else
      if( TEFile::ChangeFileExt(hkl_fn, EmptyString()) !=
          TEFile::ChangeFileExt(src_fn, EmptyString()) )
      {
#endif
        TBasicApp::NewLogEntry() << "Note that the associated HKL file differs"
          " from the loaded file name:";
        TBasicApp::NewLogEntry() << "SRC: " << src_fn;
        TBasicApp::NewLogEntry() << "HKL: " << hkl_fn;
      }
      // changes the loaded position of the box to left
      OnResize();

      olxstr Tmp = TEFile::ExtractFilePath(file_n.file_name);
      if( !Tmp.IsEmpty() && !(Tmp == XLibMacros::CurrentDir) )  {
        if( !TEFile::ChangeDir(Tmp) )  {
          TBasicApp::NewLogEntry() << "Cannot change current folder '" <<
            TEFile::CurrentDir() << "' to '" << Tmp;
        }
        else  {
          if( !Blind )  
            XLibMacros::CurrentDir = Tmp;
        }
      }
      if( !Blind )  UpdateRecentFile(file_n.file_name);
      //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      QPeakTable(false, true);
      UpdateRecentFilesTable(false);
      if( FXApp->CheckFileType<TIns>() )  {
        BadReflectionsTable(false, true);
        RefineDataTable(false, true);
        const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
        if( Lst.SplitAtomCount() )  {
          TBasicApp::NewLogEntry() << "The following atom(s) may be split:";
          for( size_t i=0; i < Lst.SplitAtomCount(); i++ )  {
            const TLstSplitAtom& SpA = Lst.SplitAtom(i);
            Tmp = SpA.AtomName;  Tmp.RightPadding(5, ' ', true);
            Tmp << olxstr::FormatFloat(3, SpA.PositionA[0]);
            Tmp.RightPadding(12, ' ', true);
            Tmp << olxstr::FormatFloat(3, SpA.PositionA[1]);
            Tmp.RightPadding(19, ' ', true);
            Tmp << olxstr::FormatFloat(3, SpA.PositionA[2]);
            Tmp.RightPadding(26, ' ', true);
            Tmp << "& ";
            Tmp << olxstr::FormatFloat(3, SpA.PositionB[0]);
            Tmp.RightPadding(35, ' ', true);
            Tmp << olxstr::FormatFloat(3, SpA.PositionB[1]);
            Tmp.RightPadding(42, ' ', true);
            Tmp << olxstr::FormatFloat(3, SpA.PositionB[2]);
            TBasicApp::NewLogEntry() << Tmp;
          }
        }
        if( Lst.TrefTryCount() )  {
          TBasicApp::NewLogEntry() << "TREF tries:";
          olxstr Tmp1;
          Tmp = "CFOM";  Tmp.RightPadding(6, ' ', true);
          Tmp1 = Tmp;
          Tmp = "NQual";  Tmp1 << Tmp.RightPadding(10, ' ', true);
          Tmp = "Try#";   Tmp1 << Tmp.RightPadding(10, ' ', true);
          Tmp1 << "Semivariants";
          TBasicApp::NewLogEntry() << Tmp1;
          int tcount = 0;
          for( size_t i=0; i < Lst.TrefTryCount(); i++ )  {
            if( i > 0 )  {
              if( Lst.TrefTry(i-1).CFOM == Lst.TrefTry(i).CFOM &&
                Lst.TrefTry(i-1).Semivariants == Lst.TrefTry(i).Semivariants &&
                Lst.TrefTry(i-1).NQual == Lst.TrefTry(i).NQual )
                continue;
            }
            Tmp = Lst.TrefTry(i).CFOM;  Tmp.RightPadding(6, ' ', true);
            Tmp1 = Tmp;
            Tmp = Lst.TrefTry(i).NQual;
            Tmp1 << Tmp.RightPadding(10, ' ', true);
            Tmp = Lst.TrefTry(i).Try;
            Tmp1 << Tmp.RightPadding(10, ' ', true);
            Tmp1 << Lst.TrefTry(i).Semivariants.FormatString(31);
            TBasicApp::NewLogEntry() << Tmp1;
            tcount ++;
            if( tcount > 5 && ( (i+1) < Lst.TrefTryCount()) )  {
              TBasicApp::NewLogEntry() << "There are " <<
                Lst.TrefTryCount() - i << " more tries";
              break;
            }
          }
        }
        if( Lst.PattSolutionCount() > 1 )  {
          TBasicApp::NewLogEntry() << "There are " << Lst.PattSolutionCount()
            << " possible patterson solutions in the listing file";
          TBasicApp::NewLogEntry() << "To browse possible solutions press "
            "Ctrl+Up and Ctrl+Down buttons. Press Enter to choose a particular"
            " solution";
          FMode |= mSolve;
          CurrentSolution = -1;
        }
      }
    }
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//      FXApp->Draw();  // to update the scene just in case...
    FGlConsole->SetCommand(FGlConsole->GetCommand());  // force the update
    return;
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, EmptyString());
    return;
  }
}
//..............................................................................
void TMainForm::macPopup(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int width = Options.FindValue("w", "100").ToInt(), 
    height = Options.FindValue("h", "200").ToInt(), 
    x = Options.FindValue("x", "0").ToInt(), 
    y = Options.FindValue("y", "0").ToInt();
  olxstr border = Options.FindValue("b"), 
    title = Options.FindValue("t"), 
    onDblClick = Options.FindValue("ondblclick"),
    onSize = Options.FindValue("onsize");
  int iBorder = 0;
  for( size_t i=0; i < border.Length(); i++ )  {
    if( border.CharAt(i) == 't' )  iBorder |= wxCAPTION;
    else if( border.CharAt(i) == 'r' )  iBorder |= wxRESIZE_BORDER;
    else if( border.CharAt(i) == 's' )  iBorder |= wxSYSTEM_MENU;
    else if( border.CharAt(i) == 'c' )  iBorder |= (wxCLOSE_BOX|wxSYSTEM_MENU);
    else if( border.CharAt(i) == 'a' )  iBorder |= (wxMAXIMIZE_BOX|wxSYSTEM_MENU);
    else if( border.CharAt(i) == 'i' )  iBorder |= (wxMINIMIZE_BOX|wxSYSTEM_MENU);
    else if( border.CharAt(i) == 'p' )  iBorder |= wxSTAY_ON_TOP;
  }
  if( iBorder == 0 )
    iBorder = wxNO_BORDER;
  // check if the popup already exists
  THtmlManager::TPopupData *pd = HtmlManager.Popups.Find(Cmds[0], NULL);
  if( pd != NULL )  {
    try  {  pd->Html->LoadPage(Cmds[1].u_str());  }
    catch( ... )  {}
    pd->Html->SetHomePage(TutorialDir + Cmds[1]);
    if( Options.Contains('w') && Options.Contains('h') )  {
#ifdef __WXGTK__  // any another way to force move ???
      pd->Dialog->SetSize(5000, 5000, 0, 0);
#endif
      pd->Dialog->SetSize(x, y, width, height);
      pd->Dialog->GetClientSize(&width, &height);
      pd->Html->SetSize(width, height);
    }
    if( !pd->Dialog->IsShown() && !Options.Contains('s'))
      pd->Dialog->Show(true);
    return;
  }
  TDialog *dlg = new TDialog(this, title.u_str(), wxT("htmlPopupWindow"), wxPoint(x,y),
    wxSize(width, height), iBorder);
  pd = &HtmlManager.NewPopup(dlg, Cmds[0]);
  dlg->OnResize.Add(pd->Html, html_parent_resize, msiExecute);
  pd->Html->SetWebFolder(TutorialDir);
  pd->Html->SetHomePage(TutorialDir + Cmds[1]);
  pd->Html->SetMovable(false);
  pd->Html->SetOnSizeData(onSize.Replace("\\(", '('));
  pd->Html->SetOnDblClickData(onDblClick.Replace("\\(", '('));
  dlg->GetClientSize(&width, &height);
  pd->Html->SetSize(width, height);
  try  {  pd->Html->LoadPage(Cmds[1].u_str());  }
  catch( ... )  {}
  
  pd->Html->OnKey.Add(this, ID_HTMLKEY);
  pd->Html->OnDblClick.Add(this, ID_ONLINK);
  pd->Html->OnSize.Add(this, ID_ONLINK);
  if( !Options.Contains('s') )
    dlg->Show();
}
//..............................................................................
void TMainForm::macDelta(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    double delta = Cmds[0].ToDouble();
    if( delta < 0.1 || delta > 0.9 )
      delta = 0.5;
    FXApp->XFile().GetLattice().SetDelta(delta);
  }
  else  {
    TBasicApp::NewLogEntry() << "Current delta (covalent bonds) is: " <<
      FXApp->XFile().GetLattice().GetDelta();
  }
}
//..............................................................................
void TMainForm::macDeltaI(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    double deltai = Cmds[0].ToDouble();
    if( deltai < 0.9 )
      deltai = 0.9;
    else if( deltai > 3 )  
      deltai = 3;
    FXApp->XFile().GetLattice().SetDeltaI(deltai);
  }
  else  {
    TBasicApp::NewLogEntry() << "Current delta (short interactions bonds) is: " <<
      FXApp->XFile().GetLattice().GetDeltaI();
  }
}
//..............................................................................
void TMainForm::macPython(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Options.Contains('i') || Options.Contains('l') )  {
    TdlgEdit *dlg = new TdlgEdit(this, true);
    dlg->SetTitle( wxT("Python script editor") );
    if( Options.Contains('l') )  {
      olxstr FN = PickFile("Open File",
        olxstr("Python scripts (*.py)|*.py")  <<
        "|Text files (*.txt)|*.txt"  <<
        "|All files (*.*)|*.*",
        TBasicApp::GetBaseDir(), true);
      if( !FN.IsEmpty() && TEFile::Exists(FN) )  {
        TStrList sl;
        sl.LoadFromFile(FN);
        dlg->SetText(sl.Text('\n'));
      }
    }
    else
      dlg->SetText(wxT("import olex\n"));
    if( dlg->ShowModal() == wxID_OK )
      PythonExt::GetInstance()->RunPython(dlg->GetText());
    dlg->Destroy();
  }
  olxstr tmp = Cmds.Text(' ');
  tmp.Replace("\\n", "\n");
  if( !tmp.EndsWith('\n') )  tmp << '\n';
  PythonExt::GetInstance()->RunPython(tmp);
}
//..............................................................................
void TMainForm::funEval(const TStrObjList& Params, TMacroError &E)  {
  TStrList Vars;
  TStrPObjList<olxstr,TSOperation*> Funcs;
  TSOperation S(NULL, &Funcs, &Vars, NULL);
  if( S.LoadFromExpression(Params[0]) == 0 )
    E.SetRetVal(S.Evaluate());
}
//..............................................................................
void TMainForm::macCreateMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  for( size_t i=0; i < Cmds.Count(); i++ )
    Cmds[i].Replace("\\t", '\t');
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  
    throw TInvalidArgumentException(__OlxSourceInfo, "menu name");
  olxstr menuName = Cmds[0].SubStringTo(ind);

  short itemType = mtNormalItem;
  if( Options.Contains("r") )  itemType = mtRadioItem;
  if( Options.Contains("c") )  itemType = mtCheckItem;
  if( Options.Contains("#") )  itemType = mtSeparator;
  olxstr stateDependent = Options.FindValue("s");
  olxstr modeDependent = Options.FindValue("m");
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  {
    TStrList toks;
    toks.Strtok( Cmds[0], ';');
    size_t mi=0;
    while( (ind = menuName.LastIndexOf(';')) != InvalidIndex && ! menu )  {
      menuName = menuName.SubStringTo(ind);
      menu = Menus[menuName];
      mi++;
      
      if( menu )  break;
    }
    mi = (menu == NULL ? 0 : toks.Count() - 1 - mi);
    for( size_t i=mi; i < toks.Count(); i++ )  {
      if( (i+1) == toks.Count() )  {
        int accell = AccMenus.GetLastId();
        if( accell == 0 )
          accell = 1000;
        else
          accell++;
        if( Cmds.Count() == 3 )  {
          size_t insindex = menu->FindItem( Cmds[2].u_str() );
          if( insindex == InvalidIndex )  insindex = 0;
          if( itemType == mtSeparator )
            menu->InsertSeparator(insindex);
          else {
            TMenuItem* item = new TMenuItem(itemType, accell, menu, toks[i]);
            if( !modeDependent.IsEmpty() ) {
              item->SetActionQueue(TModeRegistry::GetInstance().OnChange,
                modeDependent, TMenuItem::ModeDependent);
            }
            if( !stateDependent.IsEmpty() ) {
              item->SetActionQueue(TStateRegistry::GetInstance().OnChange,
                stateDependent, TMenuItem::StateDependent);
            }
            if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
            menu->Insert(insindex, item );
            AccMenus.AddAccell(accell, item );
          }
        }
        else  {
          if( itemType == mtSeparator )  menu->AppendSeparator();
          else {
            TMenuItem* item = new TMenuItem(itemType, accell, menu, toks[i]);
            if( !modeDependent.IsEmpty() ) {
              item->SetActionQueue(TModeRegistry::GetInstance().OnChange,
                modeDependent, TMenuItem::ModeDependent);
            }
            if( !stateDependent.IsEmpty() ) {
              item->SetActionQueue(TStateRegistry::GetInstance().OnChange,
                stateDependent, TMenuItem::StateDependent);
            }
            if( Cmds.Count() > 1 )  item->SetCommand(Cmds[1]);
            menu->Append( item );
            AccMenus.AddAccell(accell, item );
          }
        }
      }
      else  {
        TMenu* smenu = new TMenu();
        if( !menu )  {
          if( Cmds.Count() == 3 )  {
            size_t insindex = MenuBar->FindMenu(Cmds[2].u_str());
            if( insindex == InvalidIndex )  insindex = 0;
            MenuBar->Insert(insindex, smenu, toks[i].u_str());
          }
          else
            MenuBar->Append(smenu, toks[i].u_str());
        }
        else
          menu->Append(-1, toks[i].u_str(), (wxMenu*)smenu);
        olxstr addedMenuName;
        for( size_t j=0; j <= i; j++ )  {
          addedMenuName << toks[j];
          if( j < i )  addedMenuName << ';';
        }
        Menus.Add(addedMenuName, smenu);
        menu = smenu;
      }
    }
  }
  else  {
    int accell = AccMenus.GetLastId();
    if( accell == 0 )
      accell = 1000;
    else
      accell++;
    menuName = Cmds[0].SubStringFrom(ind+1);
    if( menuName == '#' )  menu->AppendSeparator();
    else  {
      size_t insindex;
      if( (insindex=menu->FindItem( menuName.u_str() )) != InvalidIndex )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("duplicated name: ") << menuName);
      if( Cmds.Count() == 3 )  {
        if( insindex == InvalidIndex )  insindex = 0;
        if( itemType == mtSeparator )  menu->InsertSeparator(insindex);
        else  {
          TMenuItem* item = new TMenuItem(itemType, accell, menu, menuName);
          if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
            if( !modeDependent.IsEmpty() ) {
              item->SetActionQueue(TModeRegistry::GetInstance().OnChange,
                modeDependent, TMenuItem::ModeDependent);
            }
            if( !stateDependent.IsEmpty() ) {
              item->SetActionQueue(TStateRegistry::GetInstance().OnChange,
                stateDependent, TMenuItem::StateDependent);
            }
          menu->Insert(insindex, item);
          AccMenus.AddAccell(accell, item);
        }
      }
      else  {
        if( itemType == mtSeparator )  menu->AppendSeparator();
        else  {
          TMenuItem* item = new TMenuItem(itemType, accell, menu, menuName);
            if( !modeDependent.IsEmpty() ) {
              item->SetActionQueue(TModeRegistry::GetInstance().OnChange,
                modeDependent, TMenuItem::ModeDependent);
            }
            if( !stateDependent.IsEmpty() ) {
              item->SetActionQueue(TStateRegistry::GetInstance().OnChange,
                stateDependent, TMenuItem::StateDependent);
            }
          if( Cmds.Count() > 1 )  item->SetCommand(Cmds[1]);
          menu->Append(item);
          AccMenus.AddAccell(accell, item);
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macDeleteMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TMenu* menu = Menus[Cmds[0]];
  if( menu == NULL )  {
    size_t ind = Cmds[0].LastIndexOf(';');
    if( ind == InvalidIndex )  return;
    olxstr menuName = Cmds[0].SubStringTo(ind);
    olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
    menu = Menus[menuName];
    if( menu == NULL )  return;
    ind = menu->FindItem(itemName.u_str());
    if( ind == InvalidIndex )  return;
    menu->Destroy((int)ind);
  }
  else
  {   /*
    size_t ind = Cmds[0].LastIndexOf(';');
    if( ind == InvalidIndex )  {
      ind = MenuBar->FindMenu( Cmds[0].c_str() );
      if( ind != InvalidIndex )  MenuBar
    }
    olxstr menuName = Cmds[0].SubStringTo( ind );
    olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
    menu = (TMenu*)Menus.ObjectByName( menuName );
    if( !menu )  return;
    ind = menu->FindItem( itemName.c_str() );
    if( ind == InvalidIndex )  return;
    menu->Destroy( ind );  */
  }
}
//..............................................................................
void TMainForm::macEnableMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Enable((int)ind, true);
}
//..............................................................................
void TMainForm::macDisableMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Enable((int)ind, false);
}
//..............................................................................
void TMainForm::macCheckMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Check((int)ind, true);
}
//..............................................................................
void TMainForm::macUncheckMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Check((int)ind, false);
}
//..............................................................................
void TMainForm::macCreateShortcut(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  AccShortcuts.AddAccell( TranslateShortcut( Cmds[0]), Cmds[1] );
}
//..............................................................................
void TMainForm::macSetCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  FGlConsole->SetCommand( Cmds.Text(' ') );
}
//..............................................................................
void TMainForm::funCmdList(const TStrObjList &Cmds, TMacroError &E) {
  if( FGlConsole->GetCommandCount() == 0 ) return;
  size_t cc = FGlConsole->GetCommandIndex() + Cmds[0].ToInt();
  if( FGlConsole->GetCommandCount() == 0 )  {
    E.SetRetVal(EmptyString());
    return;
  }
  if( cc >= FGlConsole->GetCommandCount() )
    cc = 0;
  E.SetRetVal(FGlConsole->GetCommandByIndex(cc));
}
//..............................................................................
void TMainForm::macUpdateOptions(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TdlgUpdateOptions* dlg = new TdlgUpdateOptions(this);
  dlg->ShowModal();
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macReload(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds[0].Equalsi("macro") )  {
    if( TEFile::Exists(FXApp->GetBaseDir() + "macro.xld") )  {
      TStrList SL;
      FMacroFile.LoadFromXLFile(FXApp->GetBaseDir() + "macro.xld", &SL);
      TDataItem* root = FMacroFile.Root().FindItem("xl_macro");
      FMacroFile.Include(&SL);
      TBasicApp::NewLogEntry() << SL;
      Macros.Load(*root);
    }
  }
  else if( Cmds[0].Equalsi("help") )  {
    if( TEFile::Exists(FXApp->GetBaseDir() + "help.xld") )  {
      TStrList SL;
      FHelpFile.LoadFromXLFile(FXApp->GetBaseDir() + "help.xld", &SL);
      FHelpItem = FHelpFile.Root().FindItem("xl_help");
      TBasicApp::NewLogEntry() << SL;
    }
  }
  else if( Cmds[0].Equalsi("dictionary") )  {
    if( TEFile::Exists( DictionaryFile ) )
      Dictionary.SetCurrentLanguage(DictionaryFile, Dictionary.GetCurrentLanguage() );
  }
}
//..............................................................................
void TMainForm::macSelBack(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FXApp->RestoreSelection();
}
//..............................................................................
void TMainForm::macStoreParam(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const size_t ind = StoredParams.IndexOf(Cmds[0]);
  if( ind == InvalidIndex )
    StoredParams.Add( Cmds[0], Cmds[1] );
  else
    StoredParams.GetObject(ind) = Cmds[1];
  if( Cmds.Count() == 3 && Cmds[2].ToBool() )
    SaveSettings(FXApp->GetInstanceDir() + FLastSettingsFile);
}
//..............................................................................
void TMainForm::macCreateBitmap(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  wxFSFile* inf = TFileHandlerManager::GetFSFileHandler(Cmds[1]);
  if( inf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Image file does not exist: ").quote() <<
      Cmds[1];
    return;
  }
  wxImage img(*inf->GetStream());
  delete inf;
  if( !img.Ok() )  {
    E.ProcessingError(__OlxSrcInfo, "Invalid image file: ") << Cmds[1];
    return;
  }
  bool resize = !Options.Contains('r');

  int owidth = img.GetWidth(), oheight = img.GetHeight();
  int l = CalcL(img.GetWidth());
  int swidth = (int)pow((double)2, (double)l);
  l = CalcL(img.GetHeight());
  int sheight = (int)pow((double)2, (double)l);

  if( swidth != owidth || sheight != oheight )
    img.Rescale(swidth, sheight);

  int cl = 3, bmpType = GL_RGB;
  if( img.HasAlpha() )  {
    cl ++;
    bmpType = GL_RGBA;
  }

  unsigned char* RGBData = new unsigned char[swidth * sheight * cl];
  for( int i=0; i < sheight; i++ )  {
    for( int j=0; j < swidth; j++ )  {
      int indexa = (i*swidth + (swidth-j-1)) * cl;
      RGBData[indexa] = img.GetRed(j, i);
      RGBData[indexa+1] = img.GetGreen(j, i);
      RGBData[indexa+2] = img.GetBlue(j, i);
      if( cl == 4 )
        RGBData[indexa+3] = img.GetAlpha(j, i);
    }
  }
  bool Created = (FXApp->FindGlBitmap(Cmds[0]) == NULL);

  TGlBitmap* glB = FXApp->CreateGlBitmap(Cmds[0], 0, 0, swidth, sheight, RGBData, bmpType);
  delete [] RGBData;

  int Top = FInfoBox->IsVisible() ? (FInfoBox->GetTop() + FInfoBox->GetHeight()) : 0;
  if( Created )  {
    for( size_t i=0; i < FXApp->GlBitmapCount(); i++ )  {
      TGlBitmap& b = FXApp->GlBitmap(i);
      if( &b == glB )  continue;
      Top += (b.GetHeight() + 2);
    }
  }

  if( Created ) {
    glB->SetWidth(owidth);
    glB->SetHeight(oheight);
  }
  glB->SetTop(Top);
  if( resize && Created ) {
    double r = ((double)FXApp->GetRender().GetWidth()/(double)owidth)/10.0;
    glB->SetZoom(r);
  }
  glB->SetLeft(FXApp->GetRender().GetWidth() - glB->GetWidth());
  FXApp->Draw();
}
//..............................................................................
void TMainForm::macDeleteBitmap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FXApp->DeleteGlBitmap(Cmds[0]);
}
//..............................................................................
void TMainForm::ChangeSolution(int sol)  {
  if( !FXApp->CheckFileType<TIns>() )  return;
  const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
  if( Lst.PattSolutionCount() == 0 )  {
    if( Solutions.IsEmpty() )  return;
    if( sol < 0 )  
      sol = Solutions.Count()-1;
    else if( (size_t)sol >= Solutions.Count() )  
      sol = 0;
    olxstr cf = olxstr(SolutionFolder) << Solutions[sol] << ".res";
    TEFile::Copy( cf, TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"), true);
    TEFile::Copy( TEFile::ChangeFileExt(cf, "lst"),
                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "lst"), true);
    FXApp->LoadXFile( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res") );
    CurrentSolution = sol;
    TBasicApp::NewLogEntry() << "Current solution with try #" << Solutions[sol];
    FXApp->Draw();
  }
  else  {
    if( sol < 0 )  
      sol = Lst.PattSolutionCount()-1;
    else if( (size_t)sol >= Lst.PattSolutionCount() )  
      sol = 0;

    TIns& Ins = FXApp->XFile().GetLastLoader<TIns>();
    Ins.SavePattSolution( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"),
      Lst.PattSolution(sol), olxstr("Solution #") << (sol+1) );

    FXApp->LoadXFile(TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"));
    CurrentSolution = sol;
    TBasicApp::NewLogEntry() << "Current patt solution #" << (sol+1);
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macTref(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
  if( !Lst.TrefTryCount() )  {
    E.ProcessingError(__OlxSrcInfo, "lst file does not contain tries");
    return;
  }
  Solutions.Clear();
  CurrentSolution = -1;
  FMode |= mSolve;
  SolutionFolder.SetLength(0);

  FXApp->XFile().UpdateAsymmUnit();  // update the last loader RM

  int reps = Cmds[0].ToInt();
  for( size_t i=0; i < Lst.TrefTryCount(); i++ )  {
    if( i > 0 )  {
      if( Lst.TrefTry(i-1).CFOM == Lst.TrefTry(i).CFOM &&
        Lst.TrefTry(i-1).Semivariants == Lst.TrefTry(i).Semivariants &&
        Lst.TrefTry(i-1).NQual == Lst.TrefTry(i).NQual )
      continue;
    }
    Solutions.AddCopy(Lst.TrefTry(i).Try);
    reps --;
    if( reps <=0 )  break;
  }
  SolutionFolder = TEFile::ExtractFilePath(FXApp->XFile().GetFileName() );
  TEFile::AddPathDelimeterI(SolutionFolder) << "olex_sol\\";
  if( !TEFile::Exists( SolutionFolder ) )
    TEFile::MakeDir( SolutionFolder );
  olxstr cinsFN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "ins");
  olxstr cresFN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res");
  olxstr clstFN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "lst");
  
  OlxStateVar _var(XLibMacros::VarName_InternalTref());
  try  {
    for( size_t i=0; i < Solutions.Count(); i++ )  {
      TIns& Ins = FXApp->XFile().GetLastLoader<TIns>();
      Ins.SaveForSolution(cinsFN, olxstr("TREF -") << Solutions[i], EmptyString());
      FXApp->LoadXFile(cinsFN);
      Macros.ProcessMacro("solve", E);
      Macros.ProcessMacro("waitfor process", E);
      TEFile::Copy(cresFN, olxstr(SolutionFolder) <<  Solutions[i] << ".res");
      TEFile::Copy(clstFN, olxstr(SolutionFolder) <<  Solutions[i] << ".lst");
    }
    ChangeSolution(0);
  }
  catch(...)  {  }
  TOlxVars::UnsetVar("internal_tref");
}
//..............................................................................
void TMainForm::macPatt(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
  if( Lst.PattSolutionCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "lst file does not contain Patterson solutions");
    return;
  }
  Solutions.Clear();
  CurrentSolution = -1;
  FMode |= mSolve;
  SolutionFolder.SetLength(0);

  FXApp->XFile().UpdateAsymmUnit();  // update the last loader RM
}
//..............................................................................
void TMainForm::macExport(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  olxstr exName;
  if( !Cmds.IsEmpty() )
    exName = Cmds[0];
  else
    exName = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "hkl");

  if( TEFile::Exists(exName) )  {
    E.ProcessingError(__OlxSrcInfo, "the hkl file already exists");
    return;
  }
  TCif* C = NULL;
  if( E.RetObj() != NULL && EsdlInstanceOf(*E.RetObj(), TCif) )
    C = E.GetRetObj<TCif>();
  else
    C = &FXApp->XFile().GetLastLoader<TCif>();
  cif_dp::cetTable* hklLoop = C->FindLoopGlobal("_refln", true);
  if( hklLoop == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  const size_t hInd = hklLoop->ColIndex("_refln_index_h");
  const size_t kInd = hklLoop->ColIndex("_refln_index_k");
  const size_t lInd = hklLoop->ColIndex("_refln_index_l");
  const size_t mInd = hklLoop->ColIndex("_refln_F_squared_meas");
  const size_t sInd = hklLoop->ColIndex("_refln_F_squared_sigma");

  if( (hInd|kInd|lInd|mInd|sInd) == InvalidIndex ) {
    E.ProcessingError(__OlxSrcInfo, "could not locate <h k l meas sigma> data");
    return;
  }

  THklFile file;
  for( size_t i=0; i < hklLoop->RowCount(); i++ )  {
    TReflection* r = new TReflection(
      hklLoop->Get(i, hInd).GetStringValue().ToInt(),
      hklLoop->Get(i, kInd).GetStringValue().ToInt(),
      hklLoop->Get(i, lInd).GetStringValue().ToInt(),
      hklLoop->Get(i, mInd).GetStringValue().ToDouble(),
      hklLoop->Get(i, sInd).GetStringValue().ToDouble());
    file.Append(*r);
  }
  file.SaveToFile(exName);
}
//..............................................................................
void TMainForm::funAlert(const TStrObjList& Params, TMacroError &E) {
  olxstr msg( Params[1] );
  msg.Replace("\\n", "\n");
  if( Params.Count() == 2 )  {
    E.SetRetVal( TdlgMsgBox::Execute(this, msg, Params[0]) );
  }
  else if( Params.Count() == 3 || Params.Count() == 4 )  {
    int flags = 0;
    bool showCheckBox=false;
    for( size_t i=0; i < Params[2].Length(); i++ )  {
      if( Params[2].CharAt(i) == 'Y' )  flags |= wxYES;
      else if( Params[2].CharAt(i) == 'N' )  flags |= wxNO;
      else if( Params[2].CharAt(i) == 'C' )  flags |= wxCANCEL;
      else if( Params[2].CharAt(i) == 'O' )  flags |= wxOK;
      else if( Params[2].CharAt(i) == 'X' )  flags |= wxICON_EXCLAMATION;
      else if( Params[2].CharAt(i) == 'H' )  flags |= wxICON_HAND;
      else if( Params[2].CharAt(i) == 'E' )  flags |= wxICON_ERROR;
      else if( Params[2].CharAt(i) == 'I' )  flags |= wxICON_INFORMATION;
      else if( Params[2].CharAt(i) == 'Q' )  flags |= wxICON_QUESTION;
      else if( Params[2].CharAt(i) == 'R' )  showCheckBox = true;
    }
    olxstr tickBoxMsg;
    if( Params.Count() == 4 )
      tickBoxMsg = Params[3];
    E.SetRetVal(TdlgMsgBox::Execute(this, msg, Params[0], tickBoxMsg, flags, showCheckBox));
  }
  FGlCanvas->SetFocus();
}
//..............................................................................
void TMainForm::macAddLabel(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  vec3d crd;
  olxstr name, label;
  if( Cmds.Count() == 3 )  {
    name = Cmds[0];
    label = Cmds[2];
    TStrList toks;
    toks.Strtok(Cmds[1], ' ');
    if( toks.Count() == 3 )  {
      crd[0] = toks[0].ToDouble();
      crd[1] = toks[1].ToDouble();
      crd[2] = toks[2].ToDouble();
    }
  }
  else  if( Cmds.Count() == 5 ) {
    name = Cmds[0];
    crd[0] = Cmds[1].ToDouble();
    crd[1] = Cmds[2].ToDouble();
    crd[2] = Cmds[3].ToDouble();
    label = Cmds[4];
  }
  FXApp->AddLabel(name, crd, label);
}
//..............................................................................
//
class TOnSync : public AActionHandler  {
  TGXApp& xa;
  olxstr BaseDir;
public:
  TOnSync(TGXApp& xapp, const olxstr baseDir) : xa(xapp)  {
    BaseDir = baseDir;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, olxstr) )  return false;
    olxstr cpath = olxstr::CommonString(BaseDir, *(const olxstr*)Data);
    TBasicApp::GetLog() << ( olxstr("\rInstalling /~/") << ((olxstr*)Data)->SubStringFrom(cpath.Length()) );
    xa.Draw();
    wxTheApp->Dispatch();
    return true;
  }
};
class TDownloadProgress: public AActionHandler  {
    TGXApp* xa;
public:
  TDownloadProgress(TGXApp& xapp) : xa(&xapp) {  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data != NULL && EsdlInstanceOf(*Data, TOnProgress) )
      TBasicApp::NewLogEntry() << ((TOnProgress*)Data)->GetAction();
    return true;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    TBasicApp::NewLogEntry() << NewLineSequence() << "Done";
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )
      return false;
    IEObject* p_d = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress*>(p_d);
    if( A->GetPos() <= 0 )  return false;
    if( A->GetMax() <= 0 )
      TBasicApp::GetLog() << (olxstr("\r") << A->GetPos()/1024 << "Kb");
    else
      TBasicApp::GetLog() << (olxstr("\r") << A->GetPos()*100/A->GetMax() << '%');
    xa->Draw();
    wxTheApp->Dispatch();
    return true;
  }
};
//
void TMainForm::macInstallPlugin(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( !FPluginItem->ItemExists(Cmds[0]) )  {
    olxstr local_file = Options['l'];
    if( !local_file.IsEmpty() )  {
      if( !TEFile::Exists(local_file) )  {
        E.ProcessingError(__OlxSrcInfo, "cannot find plugin archive");
        return;
      }
      TwxZipFileSystem zipFS(local_file, false);
      TOSFileSystem osFS(TBasicApp::GetBaseDir());
      TFSIndex fsIndex(zipFS);
      TStrList properties;
      properties.Add(Cmds[0]);
      TOnSync* progressListener = new TOnSync(*FXApp, TBasicApp::GetBaseDir());
      osFS.OnAdoptFile.Add(progressListener);

      IEObject* Cause = NULL;
      try  {  fsIndex.Synchronise(osFS, properties);  }
      catch( const TExceptionBase& exc )  {
        Cause = exc.Replicate();
      }
      osFS.OnAdoptFile.Remove(progressListener);
      delete progressListener;
      if (Cause != NULL)
        throw TFunctionFailedException(__OlxSourceInfo, Cause);

      FPluginItem->AddItem(Cmds[0]);
      TStateRegistry::GetInstance().SetState(statePluginInstalled, true,
        EmptyString(), true);

      FPluginFile.SaveToXLFile( PluginFile );
      TBasicApp::NewLogEntry() << "Installation complete";
      FXApp->Draw();
    }
    else  {
      olxstr SettingsFile(TBasicApp::GetBaseDir() + "usettings.dat");
      if( TEFile::Exists(SettingsFile) )  {
        updater::UpdateAPI api;
        short res = api.InstallPlugin(new TDownloadProgress(*FXApp), 
          new TOnSync(*FXApp, TBasicApp::GetBaseDir()),
          Cmds[0]);
        if( res == updater::uapi_OK )  {
          FPluginItem->AddItem(Cmds[0]);
          FPluginFile.SaveToXLFile(PluginFile);
          TBasicApp::NewLogEntry() << "\rInstallation complete";
        }
        else  {
          TBasicApp::NewLogEntry() <<
            "Plugin installation failed with error code: " << res;
          if( !api.GetLog().IsEmpty() )
            TBasicApp::NewLogEntry() << api.GetLog();
        }
        FXApp->Draw();
      }
      else  {
        TBasicApp::NewLogEntry() << "Could not locate usettings.dat file";
      }
    }
  }
  else  {
    TDataItem* di = FPluginItem->FindItem(Cmds[0]);
    if( di != NULL )  {
      Macros.ProcessMacro(olxstr("uninstallplugin ") << Cmds[0], E);
    }
    else  {
      TBasicApp::NewLogEntry() << "Specified plugin does not exist: " << Cmds[0];
    }
  }
}
//..............................................................................
void TMainForm::macSignPlugin(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TDataItem* di = FPluginItem->FindItem(Cmds[0]);
  if( di )  di->AddField("signature", Cmds[1]);
  FPluginFile.SaveToXLFile(PluginFile);
}
//..............................................................................
// local linkage (for classes within functions) is not supported by Borland, though MSVC is fine
  class TFSTraverser {
    olxstr BaseDir;
    SortedObjectList<olxstr, olxstrComparator<false> > props;
    TGXApp* xa;
    TPtrList<TFSItem> ToDelete;
    public:
      TFSTraverser(TGXApp& xapp, const olxstr& baseDir, const TStrList &props_)  {
        for (size_t i=0; i < props_.Count(); i++)
          props.AddUnique(props_[i]);
        BaseDir = baseDir;
        xa = &xapp;
      }
      ~TFSTraverser()  {
        TPtrList<TFSItem> FoldersToDelete;
        for( size_t i=0; i < ToDelete.Count(); i++ )
          if( !ToDelete[i]->IsFolder() && ToDelete[i]->GetParent() != NULL )  {
            if( FoldersToDelete.IndexOf(ToDelete[i]->GetParent()) == InvalidIndex )
              FoldersToDelete.Add(ToDelete[i]->GetParent());
            ToDelete[i]->GetParent()->Remove(*ToDelete[i]);
            ToDelete.Delete(i);
            i--;
          }
        while( true )  {
          bool deleted = false;
          for( size_t i=0; i < FoldersToDelete.Count(); i++ )  {
            if( FoldersToDelete[i]->IsEmpty() )  {
              olxstr path = FoldersToDelete[i]->GetIndexFS().GetBase() +
                FoldersToDelete[i]->GetFullName(),
              cpath = path.CommonString(path, BaseDir);
              TBasicApp::GetLog() << (olxstr("\rDeleting folder /~/") <<
                path.SubStringFrom(cpath.Length()));
              xa->Draw();
              wxTheApp->Dispatch();
              TEFile::RmDir( path );
              deleted = true;
              FoldersToDelete[i]->GetParent()->Remove(*FoldersToDelete[i]);
              FoldersToDelete.Delete(i);
              i--;
            }
          }
          if( !deleted )  break;
        }
      }
      bool OnItem(TFSItem& it) {
        bool hp = false;
        for (size_t i=0; i < it.PropertyCount(); i++) {
          if (props.IndexOf(it.GetProperty(i)) != InvalidIndex) {
            hp = true;
            break;
          }
        }
        if (hp) {
          olxstr path = it.GetIndexFS().GetBase() + it.GetFullName(),
                   cpath = path.CommonString(path, BaseDir);
          TBasicApp::GetLog() << (olxstr("\rDeleting /~/") <<
            path.SubStringFrom(cpath.Length()));
          xa->Draw();
          wxTheApp->Dispatch();
          TEFile::DelFile( path );
          ToDelete.Add( &it );
        }
        return true;
      }
  };
void TMainForm::macUninstallPlugin(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( Cmds[0].StartsFrom("olex") )  {
    E.ProcessingError(__OlxSrcInfo, "cannot uninstall core components");
    return;
  }
  TDataItem* di = FPluginItem->FindItem( Cmds[0] );
  if( di != NULL )  {
    FPluginItem->DeleteItem(di);
    TStateRegistry::GetInstance().SetState(statePluginInstalled, false,
      EmptyString(), true);
    olxstr indexFile = TBasicApp::GetBaseDir() + "index.ind";
    if( TEFile::Exists(indexFile) )  {
      TOSFileSystem osFS(TBasicApp::GetBaseDir());
      TFSIndex fsIndex(osFS);

      fsIndex.LoadIndex(indexFile);
      TFSTraverser* trav = new TFSTraverser(*FXApp, TBasicApp::GetBaseDir(),
        updater::UpdateAPI::GetPluginProperties(Cmds[0]));
      TFSItem::Traverser.Traverse<TFSTraverser>(fsIndex.GetRoot(), *trav);
      delete trav;
      fsIndex.SaveIndex(indexFile);
      TBasicApp::NewLogEntry() << "\rUninstallation complete";
      FXApp->Draw();
    }
  }
  FPluginFile.SaveToXLFile(PluginFile);
}
//..............................................................................
void TMainForm::funIsPluginInstalled(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( FPluginItem->ItemExists(Params[0]) );
}
//..............................................................................
void TMainForm::funValidatePlugin(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( true );
}
//..............................................................................
void TMainForm::macUpdateFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
#ifndef __WIN32__  // skip updating launch or other win files not on win
  olxstr ext(TEFile::ExtractFileExt(Cmds[0]));
  if( ext.Equalsi("exe") || ext.Equalsi("dll") || ext.Equalsi("pyd") )  {
    TBasicApp::NewLogEntry() << "Skipping windows files update...";
    return;
  }
#endif
  olxstr SettingsFile( TBasicApp::GetBaseDir() + "usettings.dat" );
  TEFile::CheckFileExists( __OlxSourceInfo, SettingsFile );
  const TSettingsFile settings(SettingsFile);
  olxstr Proxy, Repository;
  bool Force = Options.Contains('f');

  Proxy = settings["proxy"];
  Repository = settings["repository"];
  if( settings.GetParam("update").Equalsi("never") )  {
    TBasicApp::NewLogEntry() << "User settings prevented updating file: " << Cmds[0];
    return;
  }
  if( !Repository.IsEmpty() && !Repository.EndsWith('/') )  Repository << '/';

  TUrl url(Repository);
  if( !Proxy.IsEmpty() ) 
    url.SetProxy( Proxy );

  TSocketFS httpFS(url);
  TOSFileSystem osFS(TBasicApp::GetBaseDir());
  TFSIndex fsIndex(httpFS);

  IEObject* Cause = NULL;
  try  {
    if( fsIndex.UpdateFile(osFS, Cmds[0], Force) )
      TBasicApp::NewLogEntry() << "Updated '" << Cmds[0] << '\'';
    else
      TBasicApp::NewLogEntry() << "Up-to-date '" << Cmds[0] << '\'';
  }
  catch( const TExceptionBase& exc )  {  Cause = exc.Replicate();  }
  if( Cause != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
}
//..............................................................................
void TMainForm::macNextSolution(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( ((FMode&mSolve) == mSolve) )  {
    ChangeSolution( CurrentSolution - 1 );
    return;
  }
}
//..............................................................................
TNetwork::AlignInfo MatchAtomPairsQT(const TTypeList<AnAssociation2<TSAtom*,TSAtom*> >& atoms,
  bool TryInversion, double (*weight_calculator)(const TSAtom&),  bool print = true)
{
  TNetwork::AlignInfo rv = TNetwork::GetAlignmentRMSD(atoms, TryInversion, weight_calculator);
  if( print )  {
    const size_t cnt = olx_min(3, atoms.Count());
    olxstr f1 = '{', f2 = '{';
    for( size_t i=0; i < cnt; i++ )  {
      f1 << atoms[i].GetA()->GetLabel() << ',';
      f2 << atoms[i].GetB()->GetLabel() << ',';
    }
    TBasicApp::NewLogEntry() << "Alignment RMSD " << f2 << "...} to " << f1 << "...} " <<
      (TryInversion ? "with" : "without") << " inversion) is " <<
      olxstr::FormatFloat(3, rv.rmsd.GetV()) << " A";
  }
  return rv;
}
//..............................................................................
TNetwork::AlignInfo MatchAtomPairsQTEsd(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
  bool TryInversion, double (*weight_calculator)(const TSAtom&))
{
  TXApp& xapp = TXApp::GetInstance();
  VcoVContainer vcovc(xapp.XFile().GetAsymmUnit());
  xapp.NewLogEntry() << "Using " << xapp.InitVcoV(vcovc) << " matrix for the calculation";
  TSAtomPList atoms_out;
  vec3d_alist crds_out;
  TDoubleList wghts_out;
  TNetwork::PrepareESDCalc(atoms, TryInversion, atoms_out, crds_out ,wghts_out, weight_calculator);
  TEValue<double> rv = vcovc.CalcAlignmentRMSD(
    TSAtomCPList(atoms_out), crds_out, wghts_out);
  TBasicApp::NewLogEntry() << (olxstr("RMSD is ") << rv.ToString() << " A");
  return TNetwork::GetAlignmentRMSD(atoms, TryInversion, weight_calculator);
}
//..............................................................................
void TMainForm::CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS)  {
  olxstr arg;
  TStrList callBackArg;
  for( size_t i=0; i < netA.NodeCount(); i++ )  {
    arg << netA.Node(i).GetLabel();
    if( (i+1) < netA.NodeCount() )  arg << ',';
  }
  callBackArg.Add(RMS);
  callBackArg.Add(arg);
  arg.SetLength(0);
  for( size_t i=0; i < netB.NodeCount(); i++ )  {
    arg << netB.Node(i).GetLabel();
    if( (i+1) < netB.NodeCount() )  arg << ',';
  }
  callBackArg.Add(arg);
  callCallbackFunc(OnMatchCBName, callBackArg);
}
//..............................................................................
void TMainForm::macMatch(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TActionQueueLock __queuelock(FXApp->FindActionQueue(olxappevent_GL_DRAW));
  // restore if already applied
  TLattice& latt = FXApp->XFile().GetLattice();
  const TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  latt.RestoreADPs();
  if( FXApp->OverlayedXFileCount() != 0 )  {
    for( size_t i=0; i < FXApp->OverlayedXFileCount(); i++ )
      FXApp->GetOverlayedXFile(i).GetLattice().RestoreADPs();
    FXApp->AlignOverlayedXFiles();
    FXApp->CenterView(true);
  }
  else
    FXApp->CenterView();
  FXApp->UpdateBonds();
  if( Options.Contains("u") )  // do nothing...
    return;
  callCallbackFunc(StartMatchCBName, TStrList());
  const bool TryInvert = Options.Contains("i");
  double (*weight_calculator)(const TSAtom&) = &TSAtom::weight_occu;
  if( Options.Contains('w') )
    weight_calculator = &TSAtom::weight_occu_z;
  const bool subgraph = Options.Contains("s");
  olxstr suffix = Options.FindValue("n");
  const bool name = Options.Contains("n");
  const bool align = Options.Contains("a");
  int group_cnt = 2;
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    group_cnt = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  TXAtomPList atoms = FindXAtoms(Cmds, false, true);
  size_t match_cnt=0;
  if( !atoms.IsEmpty() )  {
    if( atoms.Count() == 2 &&
        (&atoms[0]->GetNetwork() != &atoms[1]->GetNetwork()) )
    {
      TTypeList<AnAssociation2<size_t, size_t> > res;
      TSizeList sk;
      TNetwork &netA = atoms[0]->GetNetwork(),
        &netB = atoms[1]->GetNetwork();
      bool match = subgraph ? netA.IsSubgraphOf(netB, res, sk) :
        netA.DoMatch(netB, res, TryInvert, weight_calculator);
      TBasicApp::NewLogEntry() << "Graphs match: " << match;
      if( match )  {
        match_cnt++;
        // restore the other unit cell, if any...
        if( &latt != &netA.GetLattice() || &latt != &netB.GetLattice() )  {
          TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice()
            : netA.GetLattice();
          latt1.RestoreADPs();
        }
        TTypeList< AnAssociation2<TSAtom*,TSAtom*> > satomp;
        TSAtomPList atomsToTransform;
        TBasicApp::NewLogEntry() << "Matching pairs:";
        olxstr tmp('=');
        for( size_t i=0; i < res.Count(); i++ )  {
          tmp << '{' << netA.Node( res[i].GetA()).GetLabel() <<
            ',' << netB.Node(res[i].GetB()).GetLabel() << '}';

          if( atomsToTransform.IndexOf(
                &netB.Node(res[i].GetB())) == InvalidIndex )
          {
            atomsToTransform.Add(netB.Node(res[i].GetB()));
            satomp.AddNew<TSAtom*,TSAtom*>(&netA.Node(res[i].GetA()),
              &netB.Node(res[i].GetB()));
          }
        }
        if( name )  {
          // change CXXX to CSuffix+whatever left of XXX
          if( suffix.Length() > 1 && suffix.CharAt(0) == '$' )  {
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for( size_t i=0; i < res.Count(); i++ )  {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if( l_d <= (index_t)l_val.Length() ) 
                new_l = elm.symbol + l_val;
              else if( l_d > (index_t)l_val.Length() ) {
                new_l = olxstr(elm.symbol) << l_val <<
                  old_l.SubStringFrom(elm.symbol.Length()+l_val.Length());
              }
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          // change the ending
          else if( suffix.Length() > 1 && suffix.CharAt(0) == '-' )  {
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for( size_t i=0; i < res.Count(); i++ )  {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if( l_d <= (index_t)l_val.Length() ) 
                new_l = elm.symbol + l_val;
              else if( l_d > (index_t)l_val.Length() )
                new_l = old_l.SubStringTo(old_l.Length()-l_val.Length()) << l_val;
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          else  {
            for( size_t i=0; i < res.Count(); i++ ) {
              netB.Node(res[i].GetB()).CAtom().SetLabel(
               netA.Node(res[i].GetA()).GetLabel() + suffix, false);
            }
          }
        }
        TNetwork::AlignInfo align_info;
        if( Options.Contains("esd") )
          align_info = MatchAtomPairsQTEsd(satomp, TryInvert, weight_calculator);
        else
          align_info = MatchAtomPairsQT(satomp, TryInvert, weight_calculator);
        // execute callback function
        CallMatchCallbacks(netA, netB, align_info.rmsd.GetV());
        // ends execute callback
        if( align )  {
          TNetwork::DoAlignAtoms(atomsToTransform, align_info);
          FXApp->UpdateBonds();
          FXApp->CenterView();
        }

        TBasicApp::NewLogEntry() << tmp;
        if( subgraph )  {
          sk.Add( res[0].GetB() );
          res.Clear();
          while( atoms[0]->GetNetwork().IsSubgraphOf(
                   atoms[1]->GetNetwork(), res, sk) )
          {
            sk.Add( res[0].GetB() );
            tmp = '=';
            for( size_t i=0; i < res.Count(); i++ )  {
              tmp << '{' <<
                atoms[0]->GetNetwork().Node(res[i].GetA()).GetLabel() << ',' <<
                atoms[1]->GetNetwork().Node(res[i].GetB()).GetLabel() << '}';
            }
            TBasicApp::NewLogEntry() << tmp;
            res.Clear();
          }
        }
      }
    }
    // a full basis provided
    else if (atoms.Count() >= 3*group_cnt && (atoms.Count()%group_cnt) == 0) {
      const size_t atom_a_group = atoms.Count()/group_cnt;
      TTypeList<AnAssociation2<TSAtom*,TSAtom*> > satomp(atom_a_group);
      // fill the reference group
      for (size_t i=0; i < atom_a_group; i++)
        satomp[i].SetA(atoms[i]);
      TNetwork &netA = atoms[0]->GetNetwork();
      for (int gi=1; gi < group_cnt; gi++) {
        for (size_t i=0; i < atom_a_group; i++)
          satomp[i].SetB(atoms[i+gi*atom_a_group]);
        TNetwork &netB = satomp[0].GetB()->GetNetwork();
        bool valid = true;
        for (size_t i=1; i < satomp.Count(); i++)  {
          if (satomp[i].GetA()->GetNetwork() != netA ||
              satomp[i].GetB()->GetNetwork() != netB)
          {
            valid = false;
            E.ProcessingError(__OlxSrcInfo,
              "Atoms should belong to two distinct fragments or the same"
              " fragment. Skipping group #") << gi;
            break;
          }
        }
        if (valid)  {
          match_cnt++;
          // restore the other unit cell, if any...
          if (&latt != &netA.GetLattice() || &latt != &netB.GetLattice())  {
            TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice()
              : netA.GetLattice();
            latt1.RestoreADPs();
          }
          TSAtomPList atomsToTransform;
          if (netA != netB) // collect all atoms
            atomsToTransform = netB.GetNodes();
          else
            atomsToTransform = atoms.SubList(gi*atom_a_group, atom_a_group);
          TNetwork::AlignInfo align_info =
            MatchAtomPairsQT(satomp, TryInvert, weight_calculator);
          TNetwork::DoAlignAtoms(atomsToTransform, align_info);
          CallMatchCallbacks(netA, netB, align_info.rmsd.GetV());
          //for (size_t ai=0; ai < netA.NodeCount(); ai++) {
          //  TSAtom &a = netA.Node(ai);
          //  TSAtom &b = netB.Node(ai);
          //  if (a.GetEllipsoid() == NULL || b.GetEllipsoid() == NULL) continue;
          //  mat3d m = a.GetEllipsoid()->GetMatrix();
          //  m[0] *= a.GetEllipsoid()->GetSX();
          //  m[1] *= a.GetEllipsoid()->GetSY();
          //  m[2] *= a.GetEllipsoid()->GetSZ();
          //  mat3d m1 = b.GetEllipsoid()->GetMatrix();
          //  m1[0] *= b.GetEllipsoid()->GetSX();
          //  m1[1] *= b.GetEllipsoid()->GetSY();
          //  m1[2] *= b.GetEllipsoid()->GetSZ();
          //  m -= m1;
          //  vec3d v = m[0].XProdVec(m[1]);
          //  if (m[2].CAngle(v) < 0)
          //    m[2] *= -1;
          //  a.GetEllipsoid()->SetMatrix(m);
          //  a.GetEllipsoid()->SetSX(1);
          //  a.GetEllipsoid()->SetSY(1);
          //  a.GetEllipsoid()->SetSZ(1);
          //  b.GetEllipsoid()->SetSX(0.01);
          //  b.GetEllipsoid()->SetSY(0.01);
          //  b.GetEllipsoid()->SetSZ(0.01);
          //}
        }
      }
    }
  }
  else  {
    TNetPList nets;
    FXApp->GetNetworks(nets);
    // restore the other unit cell, if any...
    for( size_t i=0; i < nets.Count(); i++ )  {
      if( &latt != &nets[i]->GetLattice() )
        nets[i]->GetLattice().RestoreADPs();
    }
    TEBitArray matched(nets.Count());
    for( size_t i=0; i < nets.Count(); i++ )  {
      if( !nets[i]->IsSuitableForMatching() || matched[i] )  continue;
      for( size_t j=i+1; j < nets.Count(); j++ )  {
        if( !nets[j]->IsSuitableForMatching() || matched[j] )  continue;
        TTypeList<AnAssociation2<size_t, size_t> > res;
        if( !nets[i]->DoMatch(*nets[j], res, false, weight_calculator) )
          continue;
        match_cnt++;
        matched.SetTrue(j);
        TTypeList<AnAssociation2<TSAtom*,TSAtom*> > ap;
        for( size_t k=0; k < res.Count(); k++ ) {
          ap.AddNew<TSAtom*,TSAtom*>(
            &nets[i]->Node(res[k].GetA()), &nets[j]->Node(res[k].GetB()));
        }
        TNetwork::AlignInfo a_i = MatchAtomPairsQT(ap, false, weight_calculator);
        // get info for the inverted fragment
        res.Clear();
        ap.Clear();
        nets[i]->DoMatch(*nets[j], res, true, weight_calculator);
        for( size_t k=0; k < res.Count(); k++ ) {
          ap.AddNew<TSAtom*,TSAtom*>(
            &nets[i]->Node(res[k].GetA()), &nets[j]->Node(res[k].GetB()));
        }
        TNetwork::AlignInfo ia_i =
          MatchAtomPairsQT(ap, true, weight_calculator);
        TNetwork::AlignInfo& ra_i =
          (a_i.rmsd.GetV() < ia_i.rmsd.GetV() ? a_i : ia_i);
        CallMatchCallbacks(*nets[i], *nets[j], ra_i.rmsd.GetV());
//
        //inertia<>::out io1 = inertia<>::calc(nets[i]->GetNodes(),
        //  FunctionAccessor::Make(&TSAtom::crd),
        //  FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
        //  io1.sort();
        //inertia<>::out io2;
        //if (ra_i.inverted) {
        //  io2 = inertia<>::calc(nets[j]->GetNodes(),
        //    olx_alg::olx_minus(FunctionAccessor::Make(&TSAtom::crd)),
        //    FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
        //    io2.center *= -1;
        //    io2.axis *= -1;
        //}
        //else {
        //  io2 = inertia<>::calc(nets[j]->GetNodes(),
        //    FunctionAccessor::Make(&TSAtom::crd),
        //    FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
        //}
        //io2.sort();
        //mat3d tm = mat3d::Transpose(io2.axis)*io1.axis;
        //TNetwork::DoAlignAtoms(nets[j]->GetNodes(), io2.center,
        //  tm,
        //  io1.center);
//
        TNetwork::DoAlignAtoms(nets[j]->GetNodes(), ra_i);
      }
    }
    // do match all possible fragments with similar number of atoms
  }
  if (match_cnt != 0) {
    FXApp->UpdateBonds();
    FXApp->CenterView();
  }
  else {
    TBasicApp::NewLogEntry() << "No matching fragments found";
  }
}
//..............................................................................
void TMainForm::macShowWindow(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 2 )  {
    if( Cmds[0].Equalsi("help") )  {
      HelpWindowVisible = Cmds[1].ToBool();
      FHelpWindow->SetVisible(HelpWindowVisible);
      FGlConsole->ShowBuffer(!HelpWindowVisible);  // sync states
      TStateRegistry::GetInstance().SetState(stateHelpWindowVisible,
        HelpWindowVisible, EmptyString(), true);
    } 
    else  if( Cmds[0].Equalsi("info") )  {
      InfoWindowVisible = Cmds[1].ToBool();
      FInfoBox->SetVisible(InfoWindowVisible);
      TStateRegistry::GetInstance().SetState(stateInfoWidnowVisible,
        InfoWindowVisible, EmptyString(), true);
      OnResize();
      FXApp->Draw();
    } 
    else if( Cmds[0].Equalsi("cmdline") )  {
      CmdLineVisible = Cmds[1].ToBool();
      FCmdLine->Show(CmdLineVisible);
      if( CmdLineVisible )  FCmdLine->SetFocus();
      FGlConsole->SetPromptVisible(!CmdLineVisible);
      TStateRegistry::GetInstance().SetState(stateCmdLineVisible,
        CmdLineVisible, EmptyString(), true);
      OnResize();
      FXApp->Draw();
    }
  }
  else  {
    if( Cmds[0].Equalsi("help") )  {
      HelpWindowVisible = !HelpWindowVisible;
      FHelpWindow->SetVisible(HelpWindowVisible);
      FGlConsole->ShowBuffer(!HelpWindowVisible);  // sync states
      TStateRegistry::GetInstance().SetState(stateHelpWindowVisible,
        HelpWindowVisible, EmptyString(), true);
    } 
    else if( Cmds[0].Equalsi("info") )  {
      InfoWindowVisible = !InfoWindowVisible;
      FInfoBox->SetVisible(InfoWindowVisible);
      TStateRegistry::GetInstance().SetState(stateInfoWidnowVisible,
        InfoWindowVisible, EmptyString(), true);
      OnResize();
      FXApp->Draw();
    } 
    else if( Cmds[0].Equalsi("cmdline") )  {
      CmdLineVisible = !CmdLineVisible;
      FCmdLine->Show( CmdLineVisible );
      if( CmdLineVisible )  FCmdLine->SetFocus();
      FGlConsole->SetPromptVisible(!CmdLineVisible);
      TStateRegistry::GetInstance().SetState(stateCmdLineVisible,
        CmdLineVisible, EmptyString(), true);
      OnResize();
      FXApp->Draw();
    }
  }
}
//..............................................................................
void TMainForm::funGetUserInput(const TStrObjList& Params, TMacroError &E) {
  bool MultiLine = Params[0].ToInt() != 1;
  TdlgEdit *dlg = new TdlgEdit(this, MultiLine);
  dlg->SetTitle(Params[1].u_str());
  dlg->SetText(Params[2]);
  if( dlg->ShowModal() == wxID_OK )
    E.SetRetVal(dlg->GetText());
  else
    E.SetRetVal(EmptyString());
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macOFileDel(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FXApp->OverlayedXFileCount() == 0 )  {
    Macros.ProcessMacro("fuse", Error);
    return;
  }
  int ind = Cmds[0].ToInt();
  if( ind <= -1 )  {
    if( FXApp->OverlayedXFileCount() > 0 )
      FXApp->DeleteOverlayedXFile(FXApp->OverlayedXFileCount()-1);
  }
  else  {
    if( (size_t)ind < FXApp->OverlayedXFileCount() )
      FXApp->DeleteOverlayedXFile(ind);
    else
      Error.ProcessingError(__OlxSrcInfo, "no overlayed files at given position");
  }
}
//..............................................................................
void TMainForm::macOFileSwap(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->SetActiveXFile(Cmds[0].ToSizeT());
}
//..............................................................................
class helper_Tetrahedron  {
  vec3d_list Points;
  olxstr Name;
  double Volume;
protected:
  double CalcVolume()  {
    return olx_tetrahedron_volume(Points[0], Points[1], Points[2], Points[3]);
  }
public:
  helper_Tetrahedron(const olxstr& name) : Name(name)  {
    Name = name;
    Volume = -1;
  }
  void AddPoint(const vec3d& p)  {
    Points.AddNew(p);
    if( Points.Count() == 4 )
      Volume = CalcVolume();
  }
  const olxstr& GetName() const {  return Name;  }
  const vec3d& operator [] (size_t i) const {  return Points[i];  }
  double GetVolume() const {  return Volume;  }
  int Compare(const helper_Tetrahedron& th) const {
    const double v = GetVolume() - th.GetVolume();
    return v < 0 ? -1 : (v > 0 ? 1 : 0);
  }
};


void TMainForm::macCalcVol(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms;
  if( !FindXAtoms(Cmds, xatoms, false, !Options.Contains("cs")) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atom(s) given");
    return;
  }
  bool normalise = Options.Contains('n');
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    TSAtomPList atoms;
    for( size_t j=0; j < xatoms[i]->NodeCount(); j++ ) {
      TSAtom& A = xatoms[i]->Node(j);
      if( A.IsDeleted() || (A.GetType() == iQPeakZ) )
        continue;
      atoms.Add(A);
    }
    if( atoms.Count() < 3 ) continue;
    if( normalise )
      TBasicApp::NewLogEntry() << (olxstr("Current atom: ") << xatoms[i]->GetLabel() << 
      " (volumes for normalised bonds)");
    else
      TBasicApp::NewLogEntry() << "Current atom: " << xatoms[i]->GetLabel();
    if( atoms.Count() == 3 )  {
      double sa = olx_angle(atoms[0]->crd(), xatoms[i]->crd(), atoms[1]->crd());
      sa += olx_angle(atoms[0]->crd(), xatoms[i]->crd(), atoms[2]->crd());
      sa += olx_angle(atoms[1]->crd(), xatoms[i]->crd(), atoms[2]->crd());
      TBasicApp::NewLogEntry() << "Sum of angles is " << olxstr::FormatFloat(3,sa);
      double v;
      if( normalise )  {
        v = olx_tetrahedron_volume(
        xatoms[i]->crd(), 
        xatoms[i]->crd() + (atoms[0]->crd()-xatoms[i]->crd()).Normalise(), 
        xatoms[i]->crd() + (atoms[1]->crd()-xatoms[i]->crd()).Normalise(), 
        xatoms[i]->crd() + (atoms[2]->crd()-xatoms[i]->crd()).Normalise());
      }
      else
        v = olx_tetrahedron_volume(xatoms[i]->crd(), atoms[0]->crd(), atoms[1]->crd(), atoms[2]->crd());
      TBasicApp::NewLogEntry() << "The tetrahedron volume is " << olxstr::FormatFloat(3,v);
    }
    else if( atoms.Count() == 4 )  {
      double v;
      if( normalise )  {
        v = olx_tetrahedron_volume(
        xatoms[i]->crd() + (atoms[0]->crd()-xatoms[i]->crd()).Normalise(), 
        xatoms[i]->crd() + (atoms[1]->crd()-xatoms[i]->crd()).Normalise(), 
        xatoms[i]->crd() + (atoms[2]->crd()-xatoms[i]->crd()).Normalise(), 
        xatoms[i]->crd() + (atoms[3]->crd()-xatoms[i]->crd()).Normalise());
      }
      else
        v = olx_tetrahedron_volume(atoms[0]->crd(), atoms[1]->crd(), atoms[2]->crd(), atoms[3]->crd());
      TBasicApp::NewLogEntry() << "The tetrahedron volume is " << olxstr::FormatFloat(3,v);
    }
    else  {
      TTypeList<helper_Tetrahedron> tetrahedra;
      for( size_t i1=0; i1 < atoms.Count(); i1++ ) {
        for( size_t i2=i1+1; i2 < atoms.Count(); i2++ ) {
          for( size_t i3=i2+1; i3 < atoms.Count(); i3++ ) {
            helper_Tetrahedron& th = tetrahedra.AddNew(olxstr(xatoms[i]->GetLabel()) << '-'
              << atoms[i1]->GetLabel() << '-' << atoms[i2]->GetLabel() << '-' << atoms[i3]->GetLabel());
            if( normalise )  {
              th.AddPoint(xatoms[i]->crd());
              th.AddPoint(xatoms[i]->crd() + (atoms[i1]->crd()-xatoms[i]->crd()).Normalise());
              th.AddPoint(xatoms[i]->crd() + (atoms[i2]->crd()-xatoms[i]->crd()).Normalise());
              th.AddPoint(xatoms[i]->crd() + (atoms[i3]->crd()-xatoms[i]->crd()).Normalise());
            }
            else  {
              th.AddPoint(xatoms[i]->crd());
              th.AddPoint(atoms[i1]->crd());
              th.AddPoint(atoms[i2]->crd());
              th.AddPoint(atoms[i3]->crd());
            }
          }
        }
      }
      const size_t thc = (atoms.Count()-2)*2;
      QuickSorter::Sort(tetrahedra);
      for( size_t j=0; j < tetrahedra.Count(); j++ )  {
        TBasicApp::NewLogEntry() << "Tetrahedron " << j+1 <<  ' ' << tetrahedra[j].GetName() 
          << " V = " << tetrahedra[j].GetVolume();
      }
      while(  tetrahedra.Count() > thc )
        tetrahedra.Delete(0);
      double v = 0;
      for( size_t j=0; j < tetrahedra.Count(); j++ )
        v += tetrahedra[j].GetVolume();
      TBasicApp::NewLogEntry() << "The sum of volumes of " << thc << " largest tetrahedra is " <<
        olxstr::FormatFloat(3, v);
    }
  }
}
//..............................................................................
void TMainForm::funTranslatePhrase(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal(TranslatePhrase(Params[0]));
}
//..............................................................................
void TMainForm::funCurrentLanguageEncoding(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( Dictionary.GetCurrentLanguageEncodingStr() );
}
//..............................................................................
void TMainForm::funIsCurrentLanguage(const TStrObjList& Params, TMacroError &E) {
  TStrList toks;
  toks.Strtok(Params[0], ';');
  for( size_t i=0; i < toks.Count(); i++ )  {
    if( toks[i] == Dictionary.GetCurrentLanguage() )  {
      E.SetRetVal( true );
      return;
    }
  }
  E.SetRetVal( false );
}
//..............................................................................
void TMainForm::macSchedule(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo, "invalid syntax: <interval 'task'> are expected ");
    return;
  }
  bool repeatable = false;
  for( size_t i=0; i < Options.Count(); i++ )
    if( Options.GetName(i)[0] == 'r' )  {  repeatable = true;  break;  }

  TScheduledTask& task = Tasks.AddNew();
  task.Repeatable= repeatable;
  task.Interval = Cmds[0].ToInt();
  task.Task = Cmds[1];
  task.LastCalled = TETime::Now();
}
//..............................................................................
void TMainForm::funSGList(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( GetSGList() );
}
//..............................................................................
void TMainForm::macTls(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms = FindXAtoms(Cmds, true, true);
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    if( xatoms[i]->GetEllipsoid() == NULL )
      xatoms[i] = NULL;
  }
  if( xatoms.Pack().Count() < 4 )  {
    Error.ProcessingError(__OlxSrcInfo, "at least 4 anisotropic atoms expected");
    return;
  }
  evecd Q(6);
  TAsymmUnit &au = FXApp->XFile().GetAsymmUnit();
  xlib::TLS tls(TSAtomPList(xatoms, StaticCastAccessor<TSAtom>()));
  olxstr ttitle = "TLS analysis for: ";
  tls.printTLS(ttitle << FXApp->Label(xatoms));
  tls.printFOM();
  tls.printDiff();
  if (Options.Contains('a') &&
      tls.GetElpList().Count() == xatoms.Count())
  {
    for (size_t i=0; i < xatoms.Count(); i++) {
      xatoms[i]->GetEllipsoid()->GetShelxQuad(Q);
      xatoms[i]->GetEllipsoid()->Initialise(tls.GetElpList()[i]);
      if (xatoms[i]->GetMatrix().IsFirst())
        *xatoms[i]->CAtom().GetEllipsoid() = tls.GetElpList()[i];
    }
  }
}
//..............................................................................
void TMainForm::funChooseElement(const TStrObjList& Params, TMacroError &E) {
  TPTableDlg *Dlg = new TPTableDlg(this);
  if( Dlg->ShowModal() == wxID_OK )
    E.SetRetVal(Dlg->GetSelected()->symbol);
  else
    E.SetRetVal(EmptyString());
  Dlg->Destroy();
}
//..............................................................................
void TMainForm::funChooseDir(const TStrObjList& Params, TMacroError &E) {
  olxstr title = "Choose directory",
           defPath = TEFile::CurrentDir();
  if( Params.Count() > 0 )  title = Params[0];
  if( Params.Count() > 1 )  defPath = Params[1];
  wxDirDialog dlg(this, title.u_str(), defPath.u_str());
  if( dlg.ShowModal() == wxID_OK )
    E.SetRetVal<olxstr>(dlg.GetPath());
  else
    E.ProcessingError(__OlxSrcInfo, EmptyString());
}
//..............................................................................
void TMainForm::funStrDir(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( GetStructureOlexFolder().SubStringFrom(0,1) );
}
//..............................................................................
void TMainForm::macTest(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList atoms = FindXAtoms(Cmds, true, true);

  return;
  TXApp& xapp = TXApp::GetInstance();
  //TRefList refs;// = xapp.XFile().GetRM().GetFriedelPairs();
  ////xapp.XFile().GetRM().FilterHkl(refs, ms);
  //TArrayList<compd> F;
  //TUnitCell::SymSpace sp = xapp.XFile().GetUnitCell().GetSymSpace();
  //RefinementModel::HklStat ms =
  //  xapp.XFile().GetRM().GetRefinementRefList<TUnitCell::SymSpace, RefMerger::ShelxMerger>(sp, refs);
  //F.SetCount(refs.Count());
  //SFUtil::CalcSF(xapp.XFile(), refs, F);
  //double scale_k =1./olx_sqr(xapp.XFile().GetRM().Vars.GetVar(0).GetValue());
  //double sums[5] = {0.0,0.0,0.0,0.0,0.0};
  //const vec3i min_i = ms.MinIndexes, max_i = ms.MaxIndexes;
  //TRefPList pos, neg;
  //RefUtil::GetBijovetPairs(refs, min_i, max_i, pos, neg, sp); 
  //TArray3D<TReflection*> hkl3d(min_i, max_i);
  //cm_Absorption_Coefficient_Reg ac;
  //ContentList cont = xapp.XFile().GetAsymmUnit().GetContentList();
  //double mass = 0, mu=0;
  //for( size_t i=0; i < cont.Count(); i++ )  {
  //  double v = ac.CalcMuenOverRhoForE(
  //    xapp.XFile().GetRM().expl.GetRadiationEnergy(), ac.locate(cont[i].element.symbol));
  //  mu += (cont[i].count*cont[i].element.GetMr())*v;
  //}
  //mu *= xapp.XFile().GetAsymmUnit().GetZ()/xapp.XFile().GetAsymmUnit().CalcCellVolume();
  //mu /= 6.022142;
  //xapp.NewLogEntry() << mu;  
  //for( size_t i=0; i < refs.Count(); i++ )  {
  //  hkl3d(refs[i].GetHkl()) = &refs[i];
  //  refs[i].SetTag(i);
  //}
  //for( size_t i=0; i < refs.Count(); i++ )  {
  //  if( refs[i].GetTag() < 0 )  continue;
  //  refs[i].SetTag(-1);
  //  if( refs[i].GetI()/refs[i].GetS() < 2 )  continue;
  //  for( size_t mi=0; mi < sp.Count(); mi++ )  {
  //    const vec3i& pi = refs[i].GetHkl();
  //    vec3i ni;
  //    refs[i].MulHkl(ni, sp[mi]);
  //    ni *= -1;
  //    if( hkl3d.IsInRange(ni) && hkl3d(ni) != NULL ) {
  //      TReflection& n = *hkl3d(ni);
  //      if( n.GetTag() < 0 )  continue;
  //      //const double y = (refs[i].GetI() - n.GetI())/(refs[i].GetI() + n.GetI());
  //      //const double x = (F[i].qmod() - F[n.GetTag()].qmod())/(F[i].qmod() + F[n.GetTag()].qmod());
  //      const double w = 1./olx_sqr(refs[i].GetS());
  //      const double y = (refs[i].GetI()*scale_k - F[i].qmod());
  //      const double x = (F[n.GetTag()].qmod() - F[i].qmod());
  //      sums[0] += w*x;
  //      sums[1] += w*y;
  //      sums[2] += w*x*y;
  //      sums[3] += w*x*x;
  //      sums[4] += w;
  //      n.SetTag(-1);
  //    }
  //  }
  //}
  //double k = (sums[2]-sums[0]*sums[1]/sums[4])/(sums[3]-sums[0]*sums[0]/sums[4]),
  //  //x = (k-1.0)/2,
  //  a = (sums[0] - k*sums[1])/sums[4];
  //xapp.NewLogEntry() << TEValueD(k, sqrt(sums[4]/(sums[3]-sums[0]*sums[0]/sums[4]))).ToString();

  //TSymmLib& sl = TSymmLib::GetInstance();
  //for( size_t i=0; i < sl.SGCount(); i++ )  {
  //  TSpaceGroup& sg = sl.GetGroup(i);
  //  smatd_list ml, ml1;
  //  for( size_t j=0; j < sg.MatrixCount(); j++ )
  //    ml.AddCopy(sg.GetMatrix(j));
  //  sg.GetMatrices(ml1, mattAll);
  //  const olxstr hse = HallSymbol::Evaluate(
  //    sg.GetLattice().GetLatt()*(sg.IsCentrosymmetric() ? 1 : -1), ml);
  //  const olxstr hs = olxstr(sg.GetHallSymbol()).TrimWhiteChars();
  //  if( hse != hs )
  //    TBasicApp::NewLogEntry() << hs << ": \t" << hse;
  //  SymSpace::Info si = SymSpace::GetInfo(ml1);
  //  if( si.latt != sg.GetLattice().GetLatt() || si.centrosymmetric != sg.IsCentrosymmetric() ||
  //    (sg.MatrixCount()+1) != si.matrices.Count() )
  //    TBasicApp::NewLogEntry() << sg.GetName();
  //  for( size_t j=0; j < ml.Count(); j++ )  {
  //    bool found = false;
  //    for( size_t k=0; k < si.matrices.Count(); k++ )  {
  //      if( *si.matrices[k] ==  ml[j])  {
  //        found = true;
  //        break;
  //      }
  //    }
  //    if( !found )
  //      break;
  //  }
  //}
  //return;
  //TStrList out;
  //vec3d_alist mult_vl(3), mult_vl_kr(3);
  //mat3d_alist mult_ml(9);
  //mat3d sym_mat;
  //sym_mat[0][1] = sym_mat[1][1] = -1;
  //sym_mat[1][0] = sym_mat[2][2] = 1;
  //CompositeVector<vec3d_alist, double> cv_(mult_vl);
  //CompositeVector<vec3d_alist, double> cv_kr(mult_vl_kr);
  //CompositeMatrix<mat3d_alist, double> cm_(mult_ml, 3, 3);
  //olx_mat::KroneckerProduct(sym_mat, sym_mat, cm_);
  //mat3d tr_mat(
  //  0.0255, 0.0134, -0.0072,
  //  0.0134, 0.0417, -0.007,
  //  -0.0072, -0.007, -0.0407);
  //mult_vl[0] = tr_mat[0];
  //mult_vl[1] = tr_mat[1];
  //mult_vl[2] = tr_mat[2];
  //ematd cm_out(9, 9);
  //evecd cv_out(9), kr_out(9);
  //olx_vec::MulMatrix(cv_, cm_, cv_out);
  //olx_mat::MulMatrix(cm_, cm_, cm_out);
  //olx_vec::MulMatrixT(cv_, cm_, kr_out);
  //olx_vec::MulMatrixT(cv_, cm_, cv_kr);
  //for( int i=0; i < 3; i++ )
  //  FXApp->GetLog() << mult_vl_kr[i].ToString() << '\n';
  //tr_mat = sym_mat*tr_mat*mat3d::Transpose(sym_mat);
  //for( int i=0; i < 3; i++ )
  //  FXApp->GetLog() << tr_mat[i].ToString() << '\n';
  //return;
  //TMatrix<int> kt_1(2,2), kt_2(2,2);
  //kt_1[0][0] = 1;  kt_1[0][1] = 2;
  //kt_1[1][0] = 3;  kt_1[1][1] = 4;
  //kt_2[0][0] = 0;  kt_2[0][1] = 5;
  //kt_2[1][0] = 6;  kt_2[1][1] = 7;
  //TMatrix<int> kr_o(4, 4);
  //KroneckerProduct(kt_1, kt_2, kr_o);
  //TETable kr_t(kr_o.RowCount(), kr_o.ColCount());
  //for( int i=0; i < kr_o.RowCount(); i++ )  {
  //  for( int j=0; j < kr_o.ColCount(); j++ )
  //    kr_t[i][j] = kr_o[i][j];
  //}
  //kr_t.CreateTXTList(out, "vcov", true, true, ' ');
  //TBasicApp::GetLog() << out << '\n';

  //TXAtomPList xatoms;
  //if( !FindXAtoms(Cmds, xatoms, false, true) )  {
  //  return;
  //}
  ////TXApp& xapp = TXApp::GetInstance();
  //VcoVContainer vcovc(xapp.XFile().GetAsymmUnit());
  //xapp.NewLogEntry() << "Using " << xapp.InitVcoV(vcovc) << " matrix for the calculation";
  //TSAtomPList atoms(xatoms, StaticCastAccessor<TSAtom>());
  //mat3d_list matrices;
  //vcovc.GetVcoV(atoms, matrices);
  ////vcovc.GetMatrix().FindVcoV(atoms, matrices);
  //const size_t m_dim = (size_t)sqrt((double)matrices.Count());
  //TETable tab(m_dim*3, m_dim*3);
  //for( size_t i=0; i < m_dim; i++ )  {
  //  for( size_t j=0; j < m_dim; j++ )  {
  //    for( int mi = 0; mi < 3; mi++ )  {
  //      for( int mj = 0; mj < 3; mj ++ )  {
  //        tab[i*3+mi][j*3+mj] = olxstr::FormatFloat(-3,matrices[i*m_dim+j][mi][mj], true);
  //      }
  //    }
  //  }
  //}
  //TBasicApp::NewLogEntry() << tab.CreateTXTList("vcov", true, true, ' ');

  //TSocketFS fs(TUrl("http://localhost:8082"));
  //if( fs.Exists("dist/cds.jar", true) )  {
  //  TEFile* ef = fs.OpenFileAsFile("dist/cds.jar");
  //  TEFile ef1("e:/1.tmp", "w+b");
  //  ef1 << *ef;
  //  delete ef;
  //}
  //TArray3D<double> src(0, 99, 0, 99, 0, 99), dest(0, 9, 0, 9, 0, 9);
  //const vec3s src_d = src.GetSize();
  //for( size_t i=0; i < src_d[0]; i++ )
  //  for( size_t j=0; j < src_d[1]; j++ )
  //    for( size_t k=0; k < src_d[2]; k++ )
  //      src.Data[i][j][k] = k;
  //smatdd g2c(mat3d(0, 1, 0, -1, 0, 0, 0, 0, 1), vec3d(0,0,0));  // xyz->
  //MapUtil::Cell2Cart(
  //  MapUtil::MapGetter<double, 1>(src.Data, src.GetSize()), dest.Data, dest.GetSize(),
  //  g2c, mat3d().I());
  //size_t v = src_d.Sum();

  //cif_dp::TCifDP cdp;
  //TStrList _sl;
  //_sl.LoadFromFile("e:/tmp/vdlee142.cif");
  //cdp.LoadFromStrings(_sl);
  //_sl.Clear();
  //cdp.SaveToStrings(_sl);
  //for( size_t i=0; i < cdp.Count(); i++ )  {
  //  cif_dp::CifBlock& cb = cdp[i];
  //  for( size_t j=0; j < cb.table_map.Count(); j++ )
  //    TBasicApp::GetLog() << cb.table_map.GetValue(j)->GetName() << '\n';
  //}
  //TCStrList(_sl).SaveToFile("e:/tmp/test_vdlee142.cif");
  //return;

  //uint64_t test_a = 1021;
  //uint64_t test_b = test_a%10, test_c = test_a/10;
  //uint64_t test_d = test_a/10, test_e = test_a-test_d*10;
  //FXApp->SetActiveXFile(0);
  //TAsymmUnit& _au = FXApp->XFile().GetAsymmUnit();
  //TUnitCell& uc = FXApp->XFile().GetUnitCell();
  //size_t ac = (size_t)olx_round((uc.CalcVolume()/18.6)/uc.MatrixCount());
  //TPSTypeList<double, TCAtom*> atoms;
  //for( size_t i=0; i < _au.AtomCount(); i++ )
  //  atoms.Add(_au.GetAtom(i).GetUiso(), &_au.GetAtom(i));
  //if( ac < _au.AtomCount() )  {
  //  size_t df = _au.AtomCount() - ac;
  //  for( size_t i=0; i < df; i++ )  {
  //    atoms.GetObject(atoms.Count()-i-1)->SetDeleted(true);
  //  }
  //  FXApp->XFile().EndUpdate();
  //}
  //return;
  
  //wxImage img;
  //img.LoadFile(wxT("c:/tmp/tex2d.jpg"));
  //int tex_id = FXApp->GetRender().GetTextureManager().Add2DTexture("shared_site", 1, img.GetWidth(), img.GetHeight(), 0, GL_RGB, img.GetData());
  //if( tex_id != -1 )  {
  //  TXAtomPList xatoms;
  //  FindXAtoms(Cmds, xatoms, true, true);
  //  for( size_t i=0; i < xatoms.Count(); i++ )  {
  //    xatoms[i]->Primitives()->Primitive(0)->Texture(tex_id); 
  //  }
  //}
  //return;
  //olxstr hklfn = FXApp->LocateHklFile();
  //if( TEFile::Exists(hklfn) )  {
  //  const TRefPList& fpp = FXApp->XFile().GetRM().GetFriedelPairs();
  //  if( fpp.IsEmpty() )  return;

  //  TRefList refs, nrefs, fp;
  //  fp.SetCapacity(fpp.Count());
  //  for( size_t i=0; i < fpp.Count(); i++ )
  //    fp.AddNew( *fpp[i] );
  //  const vec3i_list empty_omits;
  //  smatd_list ml;
  //  FXApp->XFile().GetLastLoaderSG().GetMatrices(ml, mattAll^mattIdentity);
  //  MergeStats stat = RefMerger::Merge<smatd_list,RefMerger::ShelxMerger>(ml, fp, refs, empty_omits, false);
  //  nrefs.SetCapacity(refs.Count());
  //  for( size_t i=0; i < refs.Count(); i++ ) 
  //    nrefs.AddNew(refs[i]).GetHkl() *= -1;
  //  TArrayList<compd> FP(refs.Count()), FN(refs.Count());
  //  SFUtil::CalcSF(FXApp->XFile(), refs, FP, true);
  //  SFUtil::CalcSF(FXApp->XFile(), nrefs, FN, true);
  //  double pscale = SFUtil::CalcFScale(FP, refs);
  //  double nscale = SFUtil::CalcFScale(FN, nrefs);
  //  double sx = 0, sy = 0, sxs = 0, sxy = 0;
  //  const size_t f_cnt = FP.Count();
  //  for( size_t i=0; i < f_cnt; i++ )  {
  //    const double I = pscale*pscale*refs[i].GetI();
  //    const double pqm = FP[i].qmod();
  //    const double nqm = FN[i].qmod();
  //    sx += (nqm - pqm);
  //    sy += (I - pqm);
  //    sxy += (nqm - pqm)*(I - pqm);
  //    sxs += (nqm - pqm)*(nqm - pqm);
  //  }
  //  double k = (sxy - sx*sy/f_cnt)/(sxs - sx*sx/f_cnt);
  //  double a = (sy - k*sx)/f_cnt;  
  //  double sdiff = 0;
  //  for( size_t i=0; i < f_cnt; i++ )  {
  //    const double I = pscale*pscale*refs[i].GetI();
  //    const double pqm = FP[i].qmod();
  //    const double nqm = FN[i].qmod();
  //    sdiff += olx_sqr((I - pqm) - k*(nqm - pqm) - a);
  //  }
  //  TBasicApp::GetLog() << "K: " << TEValue<double>(k, sqrt(sdiff/(f_cnt*(f_cnt-1)))).ToString() << '\n';
  //}
  //return;
//  TSymmLib& sl = TSymmLib::GetInstance();
//  smatd_list ml;
//  static const size_t dim = 29;
//  bool** cell[dim];
//  for( size_t i=0; i < dim; i++ )  {
//    cell[i] = new bool*[dim];
//    for( size_t j=0; j < dim; j++ )  {
//      cell[i][j] = new bool[dim]; 
//    }
//  }
//  vec3d p1;
//  vec3i ip;
//  for( size_t i=0; i < sl.SGCount(); i++ )  {
//    ml.Clear();
//    sl.GetGroup(i).GetMatrices(ml, mattAll);
//    for( size_t i1=0; i1 < dim; i1++ )
//      for( size_t i2=0; i2 < dim; i2++ )
//        for( size_t i3=0; i3 < dim; i3++ )
//          cell[i1][i2][i3] = false;
//    int sets = 0, mi1 = 0, mi2 = 0, mi3 = 0;
//    for( size_t i1=0; i1 < dim; i1++ )  {
//      const double d1 = (double)i1/dim;
//      for( size_t i2=0; i2 < dim; i2++ )  {
//        const double d2 = (double)i2/dim;
//        for( size_t i3=0; i3 < dim; i3++ )  {
//          const double d3 = (double)i3/dim;
//          if( cell[i1][i2][i3] )  continue;
//          int vc = 0;
//          vec3i minv;
//          for( size_t l=1; l < ml.Count(); l++)  {  // skip I
//            p1 = ml[l] * vec3d(d1,d2,d3);
//            p1 *= dim;
//            for( size_t k=0; k < 3; k++ )  {
//              ip[k] = olx_round(p1[k]);
//              while( ip[k] < 0  )   ip[k] += dim;
//              while( ip[k] >= dim ) ip[k] -= dim;
//            }
//            if( cell[ip[0]][ip[1]][ip[2]] )  continue;
//            if( vc++ == 0 )
//              minv = ip;
//            else if( ip.QLength() < minv.QLength() )
//              minv = ip;
//            cell[ip[0]][ip[1]][ip[2]] = true;
//            sets++;
//          }
//          if( minv[0] > mi1 )  mi1 = minv[0];
//          if( minv[1] > mi2 )  mi2 = minv[1];
//          if( minv[2] > mi3 )  mi3 = minv[2];
//        }
//      }
//    }
//    TBasicApp::GetLog() << sl.GetGroup(i).GetName() << "; mc = " << 
//      ml.Count() << " - " << (double)sets*100/(dim*dim*dim) << "% {" << mi1 << ',' << mi2 << ',' << mi3 << "}\n";
//  }
//  for( size_t i=0; i < dim; i++ )  {
//    for( size_t j=0; j < dim; j++ )
//      delete [] cell[i][j];
//    delete [] cell[i];
//  }
//    return;
//  TEBitArray ba;
//  olxstr rr = ba.FormatString(31);
//  if( FXApp->XFile().HasLastLoader() )  {
//    mat3d h2c = mat3d::Transpose(FXApp->XFile().GetAsymmUnit().GetHklToCartesian());
//    TBasicApp::GetLog() << 1./h2c[0][0] << ',' << 1./h2c[1][1] << ',' << 1./h2c[2][2] << '\n';
//    mat3d r, I;
//    r[0][0] = h2c[0].DotProd(h2c[0]);  r[0][1] = h2c[0].DotProd(h2c[1]);  r[0][2] = h2c[0].DotProd(h2c[2]);
//    r[0][1] = r[1][0];                 r[1][1] = h2c[1].DotProd(h2c[1]);  r[1][2] = h2c[1].DotProd(h2c[2]);
//    r[2][0] = r[0][2];                 r[2][1] = r[1][2];                 r[2][2] = h2c[2].DotProd(h2c[2]);
//    
//    mat3d::EigenValues(r, I.I() );
//    TBasicApp::GetLog() << r[0][0] << ',' << r[1][1] << ',' << r[2][2] << '\n';
//    TBasicApp::GetLog() << 1./r[0][0] << ',' << 1./r[1][1] << ',' << 1./r[2][2] << '\n';
//  }
//  
//  olxstr fn( FXApp->XFile().GetFileName() );
//  for( size_t i=0; i < 250; i++ )  {
//    Macros.ProcessMacro(olxstr("@reap '") << fn << '\'', Error);
//    Dispatch(ID_TIMER, msiEnter, (AEventsDispatcher*)this, NULL);
//    FHtml->Update();
//    FXApp->Draw();
//    wxTheApp->Dispatch();
//  }
//  return;
//  if( !Cmds.IsEmpty() )  {
//    TAtomReference ar(Cmds.Text(' '));
//    TCAtomGroup ag;
//    size_t atomAGroup;
//    olxstr unp(ar.Expand(FXApp->XFile().GetRM(), ag, EmptyString(), atomAGroup));
//    TBasicApp::GetLog() << "Expanding " << ar.GetExpression() << " to atoms \n";
//    for( size_t i=0; i < ag.Count(); i++ )
//      TBasicApp::GetLog() << ag[i].GetFullLabel(FXApp->XFile().GetRM()) << ' ';
//    TBasicApp::GetLog() << "\nUnprocessed instructions " << (unp.IsEmpty() ? olxstr("none") : unp) << '\n';
//    return;
//  }
//#ifndef __BORLANDC__
//  BTree<int, int> tree;
//  tree.Add(0,0);
//  tree.Add(-1,-10);
//  tree.Add(1,10);
//  tree.Add(-2,-20);
//  tree.Add(-2,-21);
//  tree.Add(-2,-22);
//  tree.Add(-2,-23);
//  tree.Add(2,20);
//  tree.Add(-3,-30);
//  tree.Add(3,30);
//  tree.Add(3,31);
//  tree.Add(3,32);
//  tree.Add(3,33);
//  tree.Add(4,40);
//  int* tv = NULL;
//  tv = tree.Find(0);
//  tv = tree.Find(1);
//  tv = tree.Find(2);
//  tv = tree.Find(3);
//  tv = tree.Find(4);
//  tv = tree.Find(-1);
//  tv = tree.Find(-2);
//  tv = tree.Find(-3);
//  Test_BTreeTraverser tt;
//  tree.Traverser.Traverse(tree, tt);
//
//  BTree2<int,int> tree2;
//  tree2.Add(0,0,0);
//  tree2.Add(0,1,1);
//  tv = tree2.Find(0,1);
//  tree2.Traverser.FullTraverse(tree2, tt);
//
//  BTree3<int, int> tree3;
//  tree3.Add(0, 0, 0, 0);
//  tree3.Add(0, 1, 0, 1);
//  tree3.Add(0, 0, 1, 2);
//  tree3.Add(1, 0, 1, 3);
//  tv = tree3.Find(0, 0, 1);
//  tv = tree3.Find(0, 0, 0);
//  tv = tree3.Find(0, 1, 0);
//  tree3.Traverser.FullTraverse(tree3, tt);
//#endif  
//  if( Cmds.IsEmpty() )  return;
//  const olxcstr atom_s("ATOM");
//  TCStrList lst, toks;
//  char bf[256];
//  char format[] = "%s";
//  TIntList AtomF;
//  AtomF.Add(6);  //"ATOM  "
//  AtomF.Add(5);  //serial number
//  AtomF.Add(5);  //name
//  AtomF.Add(4);  //residue name
//  AtomF.Add(2);  //chain ID      #21
//  AtomF.Add(4);  //  residue sequence number #26
//  AtomF.Add(12);  // x                        #38
//  AtomF.Add(8);  // y
//  AtomF.Add(8);  // z
//  AtomF.Add(6);  // occupancy
//  AtomF.Add(6);  // temperature factor  #66
//  AtomF.Add(12);  // element             #78
//  AtomF.Add(2);  // charge
//
//  lst.LoadFromFile(Cmds[0]);
//  for( size_t i=0; i < lst.Count(); i++ )  {
//    if( lst[i].StartsFrom(atom_s) )  {
//      toks.Clear();
//      toks.Strtok(lst[i], ' ');
//      memset(bf, 32, 255);
//      toks.Delete( toks.Count()-2 );
//      int offset = 0;
//      for( size_t j=0; j < olx_min(AtomF.Count(),toks.Count()); j++ )  {
//        if( j > 0 )  {
//          offset += AtomF[j];
//          memcpy(&bf[offset-toks[j].Length()], toks[j].c_str(), toks[j].Length() );
//        }
//        else if( j == 0 )  {
//          memcpy(&bf[offset], toks[j].c_str(), toks[j].Length() );
//          offset += AtomF[j];
//        }
//      }
//      bf[offset] = '\0';
//      lst[i] = bf;
//    }
//  }
//  lst.Pack();
//  lst.SaveToFile( Cmds[0] + ".out" );
//  return;
///*
//  TStrList lst, toks;
//  TEFile sgFile( FXApp->BaseDir() + "sg.txt", "rb" );
//  TDataFile df;
//  df.LoadFromXLFile( FXApp->BaseDir() + "symmlib.xld" );
//  lst.LoadFromStream( sgFile );
//  for( size_t i=2; i < lst.Count(); i++ )  {
//    toks.Clear();
//    olxstr::Strtok( olxstr::RemoveMultiSpaces( lst[i], true ), ' ', toks);
//    if( toks.Count() < 7 )  continue;
//    olxstr axis, num;
//    int si = toks[0].IndexOf(':');
//    if( si != -1 )  {
//      num = toks[0].SubStringTo(si);
//      axis = toks[0].SubStringFrom(si+1);
//    }
//    else
//      num = toks[0];
//
//    for( size_t j=0; j < df.Root().ItemCount(); j++ )  {
//      if( df.Root().Item(j).GetFieldValue( "#" ) == num &&
//          df.Root().Item(j).GetFieldValue("AXIS") == axis )  {
//        TBasicApp::GetLog() << ( olxstr("Found ") << df.Root().Item(j).GetName() );
//        olxstr tmp = toks[1];
//        tmp << ' ' << toks[2] << ' ' << toks[3] << ' ' << toks[4];
//        df.Root().Item(j).SetFieldValue( "FULL", tmp );
//        break;
//      }
//    }
//  }
//  // validating
//  for( size_t i=0; i < df.Root().ItemCount(); i++ )  {
//    olxstr tmp = df.Root().Item(i).GetFieldValue("FULL");
//    if( tmp.IsEmpty() )  {
//      if( df.Root().Item(i).GetName().Length() == 4 )  {
//        tmp << df.Root().Item(i).GetName()[0] << ' ' <<
//               df.Root().Item(i).GetName()[1] << ' ' <<
//               df.Root().Item(i).GetName()[2] << ' ' <<
//               df.Root().Item(i).GetName()[3];
//        TBasicApp::GetLog() << ( olxstr("Empty, but patched for ") << df.Root().Item(i).GetName() );
//      }
//      else
//        TBasicApp::GetLog() << ( olxstr("Empty val for ") << df.Root().Item(i).GetName() );
//      continue;
//    }
//    toks.Clear();
//    olxstr::Strtok( tmp, ' ', toks );
//    if( toks.Count() != 4 )  {
//      TBasicApp::GetLog() << ( olxstr("Wrong toks count for ") << df.Root().Item(i).GetName() );
//      continue;
//    }
//  }
//  // saving
//  df.SaveToXLFile( FXApp->BaseDir() + "sg.xld" );
//*/
///*
//  if( !FXApp->CheckFileType<TIns>() )  return;
//
//  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
//  olxstr HklFN = Ins->GetHKLSource();
//  if( !TEFile::Exists(HklFN) )  {
//    Error.ProcessingError(__OlxSrcInfo, "could not locate HKL file" );
//    return;
//  }
//  TScattererItLib scatlib;
//
//  THklFile Hkl;
//  Hkl.LoadFromFile(HklFN);
//  TVectorD vec(3);
//  TMatrixD M( Ins->GetAsymmUnit().GetHklToCartesian() );
//
//  double minV = 1000, maxV = 0;
//  TEFile out( TEFile::ChangeFileExt(Ins->GetFileName(), "out"), "wb+" );
//  olxstr tmp;
//  Hkl.AnalyseReflections(*TSymmLib::GetInstance().FindSG(Ins->GetAsymmUnit()));
////  Hkl.AnalyseReflections( *TSymmLib::GetInstance().FindGroup("P-1") );
//  for( size_t i=0; i < Hkl.RefCount(); i++ )  {
//    TReflection* ref = Hkl.Ref(i);
//    tmp = ref->ToString();
//    tmp << ' ' << ref->IsCentric() << ' ' << ref->GetDegeneracy();
//    out.Writenl(tmp);
//  }
//  double cm = 0, am = 0, integ = 0, meanFs = 0;
//  int cc = 0, ac = 0, arc = 0;
//  double cd = 0, ad = 0;
//
//  long minH = (long)Hkl.GetMinValues()[0];
//  long maxH = (long)Hkl.GetMaxValues()[0];
//  long minK = (long)Hkl.GetMinValues()[1];
//  long maxK = (long)Hkl.GetMaxValues()[1];
//  long minL = (long)Hkl.GetMinValues()[2];
//  long maxL = (long)Hkl.GetMaxValues()[2];
//
//
//  typedef AnAssociation4<double, double, int, double> refc;
//  TArray3D<refc*> Hkl3D(minH, maxH, minK, maxK, minL, maxL);
//
//  TTypeList<refc*> allRefs;
//
//
//  for( size_t i=0; i < Hkl.RefCount(); i++ )  {
//    TReflection* ref = Hkl.Ref(i);
//
//    if( olx_abs(ref->Data()[0])/ref->Data()[1] > 3 )
//      continue;
//
//    if( ref->Data()[0] < 0 )
//      continue;
//
//    refc* ref1 = Hkl3D[ref->H()][ref->K()][ref->L()];
//    if( ref1 != NULL )  {
//      ref1->A() += ref->Data()[0];
//      ref1->B() += QRT(ref->Data()[1]);
//      ref1->C() ++;
//      ref1->D() += QRT(ref->Data()[0]);
//    }
//    else
//      Hkl3D[ref->H()][ref->K()][ref->L()] =
//        new refc(ref->Data()[0], QRT(ref->Data()[1]), 1, QRT(ref->Data()[0]) );
//  }
//
//  allRefs.SetCapacity( Hkl.RefCount() );
//
//  for( int i=minH; i < maxH; i++ )  {
//    for( int j=minK; j < maxK; j++ )  {
//      for( int k=minL; k < maxL; k++ )  {
//        refc* ref1 = Hkl3D[i][j][k];
//        if( ref1 != NULL )  {
//          ref1->A() /= ref1->C();
//          ref1->B() /= sqrt(ref1->GetB());
//          ref1->D() /= ref1->C();
//          ref1->D() -= QRT( ref1->GetA() );
//          ref1->D() = sqrt( ref1->GetD() );
//          allRefs.AddACopy( ref1 );
//        }
//      }
//    }
//  }
//
//  integ = 0;
//  meanFs = 0;
//  for( size_t i=0; i < allRefs.Count(); i++ )  {
////    meanFs += allRefs[i]->GetA();
//    meanFs += allRefs[i]->GetA()/olx_max(allRefs[i]->GetB(), allRefs[i]->GetD());
//    //}
//    delete allRefs[i];
//  }
//  if( allRefs.Count() != 0 )
//    meanFs /= allRefs.Count();
//
//  for( size_t i=0; i < allRefs.Count(); i++ )
//    integ += olx_abs( (allRefs[i]->GetA()/olx_max(allRefs[i]->GetB(), allRefs[i]->GetD()))/meanFs -1 );
////    integ += olx_abs(allRefs[i]->GetA()/meanFs -1 );
//
//  if( allRefs.Count() != 0 )
//    integ /= allRefs.Count();
//  TBasicApp::GetLog() << (olxstr("Calculated value: ") << integ );
//*/
//  // qpeak anlysis
//  TPSTypeList<double, TCAtom*> SortedQPeaks;
//  TDoubleList vals;
//  int cnt = 0;
//  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
//  for( size_t i=0; i < au.AtomCount(); i++ )  {
//    if( au.GetAtom(i).GetType() == iQPeakZ )  {
//      SortedQPeaks.Add(au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
//    }
//  }
//  vals.Add(0);
//  for( size_t i=SortedQPeaks.Count()-1; i != InvalidIndex; i-- )  {
//    if( (SortedQPeaks.GetKey(i) - SortedQPeaks.GetKey(i-1))/SortedQPeaks.GetKey(i) > 0.1 )  {
//      TBasicApp::GetLog() << ( olxstr("Threshold here: ") << SortedQPeaks.GetObject(i)->GetLabel() << '\n' );
//      vals.GetLast() += SortedQPeaks.GetKey(i);
//      cnt++;
//      vals.GetLast() /= cnt;
//      cnt = 0;
//      vals.Add(0);
//      continue;
//    }
//    vals.GetLast() += SortedQPeaks.GetKey(i);
//    cnt ++;
//  }
//  if( cnt != 0 )
//    vals.GetLast() /= cnt;
//  for( size_t i=0; i < vals.Count(); i++ )
//    TBasicApp::GetLog() << vals[i] << '\n';
}
//..............................................................................
void TMainForm::macLstSymm(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TUnitCell& uc = FXApp->XFile().GetLattice().GetUnitCell();

  TTTable<TStrList> tab(uc.MatrixCount(), 2);
  tab.ColName(0) = "Code";
  tab.ColName(1) = "Symm";
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    tab[i][0] =
      TSymmParser::MatrixToSymmCode(uc.GetSymmSpace(), uc.GetMatrix(i));
    tab[i][1] = TSymmParser::MatrixToSymm(uc.GetMatrix(i));
  }
  TStrList output;
  tab.CreateTXTList(output, "Unit cell symmetry list", true, true, ' ');
  TBasicApp::NewLogEntry() << output;
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::macSgen(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TXAtomPList Atoms;
  smatd_list symm;
  smatd matr;
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    bool validSymm = false;
    if( TSymmParser::IsRelSymm(Cmds[i]) )  {
      try  {
        matr = TSymmParser::SymmCodeToMatrix(
          FXApp->XFile().GetLattice().GetUnitCell(), Cmds[i]);
        validSymm = true;
      }
      catch( TExceptionBase& )  {}
    }
    if( !validSymm )  {
      try  {
        TSymmParser::SymmToMatrix(Cmds[i], matr);
        validSymm = true;
      }
      catch( TExceptionBase& )  {}
    }
    if( validSymm )  {
      Cmds.Delete(i--);
      symm.AddCopy(FXApp->XFile().GetUnitCell().InitMatrixId(matr));
    }
  }
  if( symm.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no symm code(s) provided");
    return;
  }
  if( !FindXAtoms(Cmds, Atoms, true, true) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  FXApp->Grow(Atoms, symm);
}
//..............................................................................
void TMainForm::macIT(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms;
  if( !FindXAtoms(Cmds, xatoms, true, true) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  inertia<>::out io = inertia<>::calc(xatoms,
    FunctionAccessor::Make(&TSAtom::crd),
    FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
  io.sort();
  if (Options.Contains("o"))  {
    io.axis[2] = io.axis[0].XProdVec(io.axis[1]).Normalise();
    FXApp->GetRender().GetBasis().Orient(io.axis, false);
    FXApp->GetRender().GetBasis().SetCenter(
      FXApp->GetRender().GetBasis().GetCenter() - io.center);
  }
  else {
  TBasicApp::NewLogEntry() <<
    "Ixx =  " << olxstr::FormatFloat(3, io.moments[0]) <<
    "  Iyy = "  << olxstr::FormatFloat(3, io.moments[1]) <<
    "  Izz = "  << olxstr::FormatFloat(3, io.moments[2]);
  TBasicApp::NewLogEntry() << "Eigen vectors:";
  for( size_t i=0; i < 3; i++ )
    TBasicApp::NewLogEntry() << io.axis[i].ToString();
  }
}
//..............................................................................
void TMainForm::macStartLogging(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool clear = Options.Contains("c");
  if( clear )  {
    olxstr fn = ActiveLogFile->GetName();
    if( ActiveLogFile != NULL )  {
      ActiveLogFile->Delete();
      delete ActiveLogFile;
      ActiveLogFile = NULL;
    }
    else  {
      if( TEFile::Exists(Cmds[0]) )  {
        TEFile::DelFile( Cmds[0] );
      }
    }
  }
  if( ActiveLogFile != NULL )
    return;
  if( TEFile::Exists(Cmds[0]) )
    ActiveLogFile = new TEFile(Cmds[0], "a+b");
  else
    ActiveLogFile = new TEFile(Cmds[0], "w+b");
  ActiveLogFile->Writeln(EmptyString());
  ActiveLogFile->Writeln(olxstr("Olex2 log started on ") << TETime::FormatDateTime(TETime::Now()));
}
//..............................................................................
void TMainForm::macViewLattice(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if( !TEFile::Exists(Cmds[0])  )  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exist");
    return;
  }
  TBasicCFile* bcf = FXApp->XFile().FindFormat(TEFile::ExtractFileExt(Cmds[0]));
  if( bcf == NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "unknown file format");
    return;
  }
  bcf = (TBasicCFile*)bcf->Replicate();
  try  {  bcf->LoadFromFile(Cmds[0]);  }
  catch(const TExceptionBase& exc)  {
    delete bcf;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  TXLattice& xl = FXApp->AddLattice("xlatt", bcf->GetAsymmUnit().GetCellToCartesian());
  delete bcf;
  return;
}
//..............................................................................
void main_GenerateCrd(const vec3d_list& p, const smatd_list& sm, vec3d_list& res) {
  vec3d v, t;
  for( size_t i=0; i < sm.Count(); i++ )  {
    v = sm[i] * p[0];
    t.Null();
    for( size_t j=0; j < 3; j++ )  {
      while( v[j] > 1.01 )  {  v[j] -= 1;  t[j] -= 1;  }
      while( v[j] < -0.01 ) {  v[j] += 1;  t[j] += 1;  }
    }
    bool found = false;
    for( size_t j=0; j < res.Count(); j += p.Count() )  { 
      if( v.QDistanceTo( res[j] ) < 0.0001 )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      res.AddCopy(v);
      for( size_t j=1; j < p.Count(); j++ )  {
        v = sm[i] * p[j];
        v += t;
        res.AddCopy(v);
      }
    }
  }
  // expand translation
  size_t rc = res.Count();
  for( int x=-1; x <= 1; x++ )  {
    for( int y=-1; y <= 1; y++ )  {
      for( int z=-1; z <= 1; z++ )  {
        if( x == 0 && y == 0 && z == 0 )  continue;
        for( size_t i=0; i < rc; i+= p.Count() )  {
          v = res[i];
          v[0] += x;  v[1] += y;  v[2] += z;
          t.Null();
          for( size_t j=0; j < 3; j++ )  {
            if( v[j] > 1.01 )        {  v[j] -= 1;  t[j] -= 1;  }
            else if( v[j] < -0.01 )  {  v[j] += 1;  t[j] += 1;  }
          }
          bool found = false;
          for( size_t j=0; j < res.Count(); j+=p.Count() )  {  
            if( v.QDistanceTo( res[j] ) < 0.0001 )  {
              found = true;
              break;
            }
          }
          if( !found )  {
            res.AddCopy(v);
            for( size_t j=1; j < p.Count(); j++ )  {
              v = res[i+j];
              v[0] += x;  v[1] += y;  v[2] += z;
              v += t;
              res.AddCopy(v);
            }
          }
        }
      }
    }
  }
}

void TMainForm::macAddObject(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds[0].Equalsi("cell") && Cmds.Count() == 2 )  {
    if( !TEFile::Exists( Cmds[1] )  )  {
      Error.ProcessingError(__OlxSrcInfo, "file does not exist");
      return;
    }
    TBasicCFile* bcf = FXApp->XFile().FindFormat(TEFile::ExtractFileExt(Cmds[1]));
    if( bcf == NULL )  {
      Error.ProcessingError(__OlxSrcInfo, "unknown file format");
      return;
    }
    bcf = (TBasicCFile*)bcf->Replicate();
    try  {  bcf->LoadFromFile(Cmds[1]);  }
    catch(const TExceptionBase& exc)  {
      delete bcf;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    TDUnitCell* duc = new TDUnitCell(FXApp->GetRender(), olxstr("cell") << (UserCells.Count()+1) );
    const double cell[6] = {
      bcf->GetAsymmUnit().GetAxes()[0],
      bcf->GetAsymmUnit().GetAxes()[1],
      bcf->GetAsymmUnit().GetAxes()[2],
      bcf->GetAsymmUnit().GetAngles()[0],
      bcf->GetAsymmUnit().GetAngles()[1],
      bcf->GetAsymmUnit().GetAngles()[2]
    };
    duc->Init(cell);
    FXApp->AddObjectToCreate(duc);
    TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(bcf->GetAsymmUnit());
    UserCells.Add(AnAssociation2<TDUnitCell*, TSpaceGroup*>(duc, &sg));
    duc->Create();
    delete bcf;
  }
  else {
    TDUnitCell* uc = NULL;
    TSpaceGroup* sg = NULL;
    if( !Cmds[2].IsNumber() )  {
      Error.ProcessingError(__OlxSrcInfo, "invalid unit cell reference");
      return;
    }
    int cr = Cmds[2].ToInt()-1;
    if( cr == -1 )  {
      uc = &FXApp->DUnitCell();
      sg = &TSymmLib::GetInstance().FindSG(FXApp->XFile().GetAsymmUnit());
    }
    else if( cr >=0 && (size_t)cr < UserCells.Count() )  {
      uc = UserCells[cr].A();
      sg = UserCells[cr].B();
    }
    else {
      Error.ProcessingError(__OlxSrcInfo, "invalid unit cell reference");
      return;
    }
    smatd_list ml;
    sg->GetMatrices(ml, mattAll);
    vec3d_list p, allPoints;

    if( Cmds[0].Equalsi("sphere") )  {
      TDUserObj* uo = new TDUserObj(FXApp->GetRender(), sgloSphere, Cmds[1]);
      try  {
        if( Cmds.Count() == 4 )
          uo->Params().Resize(1)[0] = Cmds[3].ToDouble();
        else if( Cmds.Count() == 7 )  {
          uo->Params().Resize(1)[0] = Cmds[3].ToDouble();
          uo->Basis.Translate(
            vec3d(Cmds[4].ToDouble(), Cmds[5].ToDouble(), Cmds[6].ToDouble()));
        }
        else  {
          delete uo;
          uo = NULL;
        }
        if( uo != NULL )  {
          FXApp->AddObjectToCreate(uo);
          uo->SetZoomable(true);
          //uo->SetMove2D(true);
          uo->SetMoveable(true);
          uo->Create();
        }
      }
      catch(const TExceptionBase &e)  {
        delete uo;
        throw TFunctionFailedException(__OlxSourceInfo, e);
      }
      //for( size_t i=3; i < Cmds.Count(); i+=3 ) 
      //  p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
      //main_GenerateCrd(p, ml, allPoints);
      //TArrayList<vec3f>& data = *(new TArrayList<vec3f>(allPoints.Count()));
      //for( size_t i=0; i < allPoints.Count(); i++ )
      //  data[i] = allPoints[i] * uc->GetCellToCartesian();
      //TDUserObj* uo = new TDUserObj(FXApp->GetRender(), sgloSphere, Cmds[1]);
      //uo->SetVertices(&data);
      //FXApp->AddObjectToCreate(uo);
      //uo->Create();
    }
    else if( Cmds[0].Equalsi("line") )  {
      if( (Cmds.Count()-3)%6 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments" );
        return;
      }
      for( size_t i=3; i < Cmds.Count(); i+= 6 )  {
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
        p.AddNew(Cmds[i+3].ToDouble(), Cmds[i+4].ToDouble(), Cmds[i+5].ToDouble());
      }
      main_GenerateCrd(p, ml, allPoints);
      TArrayList<vec3f>& data = *(new TArrayList<vec3f>(allPoints.Count() ));
      for( size_t i=0; i < allPoints.Count(); i++ )
        data[i] = allPoints[i] * uc->GetCellToCartesian();
      TDUserObj* uo = new TDUserObj(FXApp->GetRender(), sgloLines, Cmds[1]);
      uo->SetVertices(&data);
      FXApp->AddObjectToCreate(uo);
      uo->Create();
    }
    else if( Cmds[0].Equalsi("plane") )  {
      if( (Cmds.Count()-3)%12 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments");
        return;
      }
      for( size_t i=3; i < Cmds.Count(); i+= 12 )  {
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
        p.AddNew(Cmds[i+3].ToDouble(), Cmds[i+4].ToDouble(), Cmds[i+5].ToDouble());
        p.AddNew(Cmds[i+6].ToDouble(), Cmds[i+7].ToDouble(), Cmds[i+8].ToDouble());
        p.AddNew(Cmds[i+9].ToDouble(), Cmds[i+10].ToDouble(), Cmds[i+11].ToDouble());
      }
      main_GenerateCrd(p, ml, allPoints);
      TArrayList<vec3f>& data = *(new TArrayList<vec3f>(allPoints.Count()));
      for( size_t i=0; i < allPoints.Count(); i++ )
        data[i] = allPoints[i] * uc->GetCellToCartesian();
      TDUserObj* uo = new TDUserObj(FXApp->GetRender(), sgloQuads, "user_plane");
      uo->SetVertices(&data);
      FXApp->AddObjectToCreate(uo);
      uo->Create();
    }
    else if( Cmds[0].Equalsi("poly") )  {
      ;
    }
    else
      Error.ProcessingError(__OlxSrcInfo, "unknown object type");
  }
}
//..............................................................................
void TMainForm::macDelObject(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macTextm(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !TEFile::Exists( Cmds[0] )  )  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exist");
    return;
  }
  TStrList sl;
  sl.LoadFromFile( Cmds[0] );
  for( size_t i=0; i < sl.Count(); i++ )  {
    Macros.ProcessMacro(sl[i], Error);
    if( !Error.IsSuccessful() )  break;
  }
}
//..............................................................................
void TMainForm::macOnRefine(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
class MTTestTh : public AOlxThread  {
  olxcstr msg;
  compd cd_res;
  compf cf_res;
public:
  MTTestTh() {  Detached = false;  }
  int Run()  {
#ifdef _DEBUG
    for( size_t i=0; i < 10000; i++ )  {
#else
    for( size_t i=0; i < 250000; i++ )  {
#endif
      msg = SHA256::Digest(MD5::Digest(msg));
      for( size_t j=0; j < 1000; j++ )  {
        cd_res = (cd_res + compd(1.2, 1.4))*compd(1.00000001, 0.9999999);
        cd_res /= compd(1.001, 0.999);
        cd_res -= 1;
        cf_res = (cf_res + compf(1.2, 1.4))*compf(1.00000001, 0.9999999);
        cf_res /= compf(1.001, 0.999);
        cf_res -= 1;
      }
    }
    return 0;
  }
  DefPropC(olxcstr, msg);
};

void TMainForm::macTestMT(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  uint64_t times[8], min_t;
  MTTestTh threads[8];
  size_t max_th = 1;
  memset(times, 0, sizeof(uint64_t)*8);
  TBasicApp::NewLogEntry() << "Testing multithreading compatibility...";
  for( size_t i=1; i <= 8; i++ )  {
    uint64_t st = TETime::msNow();
    for( size_t j=0; j < i; j++ )
      threads[j].Start();
   for( size_t j=0; j < i; j++ )
      threads[j].Join();
    times[i-1] = TETime::msNow() - st;
    if( i == 1 )
      min_t = times[0];
    else if( times[i-1] < min_t )
      min_t = times[i-1];
    TBasicApp::NewLogEntry() << i << " threads " << times[i-1] << " ms";
    TBasicApp::GetInstance().Update();
    if( i > 1 && ((double)times[i-1]/min_t) > 1.4 )  {
      max_th = i-1;
      break;
    }
  }
  TBasicApp::NewLogEntry() << "Maximum number of threads is set to " << max_th;
  FXApp->SetMaxThreadCount(max_th);
}
//..............................................................................
void TMainForm::macSetFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene& scene = dynamic_cast<TwxGlScene&>(FXApp->GetRender().GetScene());
  TGlFont* glf = scene.FindFont(Cmds[0]);
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Cmds[0]);
    return;
  }
  try  {
    TwxGlScene::MetaFont mf(Cmds[1]);
    olxstr ps(Options.FindValue("ps"));
    if( !ps.IsEmpty() )  {
      if( ps.CharAt(0) == '+' || ps.CharAt(0) == '-' )
        mf.SetSize(mf.GetSize() + ps.ToInt());
      else 
        mf.SetSize(ps.ToInt());
    }
    if( Options.Contains('i') )
      mf.SetItalic(true);
    if( Options.Contains('b') )
      mf.SetBold(true);
    scene.CreateFont(glf->GetName(), mf.GetIdString());
    FXApp->UpdateLabels();
  }
  catch(const TExceptionBase& e)  {
    E.ProcessingError(__OlxSrcInfo, e.GetException()->GetError());
  }
}

//..............................................................................
void TMainForm::funChooseFont(const TStrObjList &Params, TMacroError &E)  {
  olxstr fntId;
  if( !Params.IsEmpty() )  {
    if( Params[0].Equalsi("olex2") )  {
      if( Params.Count() == 2 && TwxGlScene::MetaFont::IsOlexFont(Params[1]) ) 
        fntId = Params[1];
      else
        fntId = TwxGlScene::MetaFont::BuildOlexFontId("olex2.fnt", 12, true, false, false);
    }
    else if( Params[0].Equalsi("system") )  {
      if( Params.Count() == 2 && !TwxGlScene::MetaFont::IsOlexFont(Params[1]) )
        fntId = Params[1];
      else  {
        wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);
        fntId = Font.GetNativeFontInfoDesc();
      }
    }
    else
      fntId = Params[0];
  }
  olxstr rv(FXApp->GetRender().GetScene().ShowFontDialog(NULL, fntId));
  E.SetRetVal(rv);
}
//..............................................................................
void TMainForm::funGetFont(const TStrObjList &Params, TMacroError &E)  {
  TGlFont* glf = FXApp->GetRender().GetScene().FindFont(Params[0]);
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Params[0]);
    return;
  }
  E.SetRetVal(glf->GetIdString());
}
//..............................................................................
void TMainForm::macEditMaterial(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGlMaterial* mat = NULL, *smat = NULL;
  TGPCollection* gpc = NULL;
  if( Cmds[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Cmds[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Cmds[0] == "execout" )
    mat = &ExecFontColor;
  else if( Cmds[0] == "error" )
    mat = &ErrorFontColor;
  else if( Cmds[0] == "exception" )
    mat = &ExceptionFontColor;
  else  {
    gpc = FXApp->GetRender().FindCollection(Cmds[0]);
    if( gpc == NULL )  {
      const size_t di = Cmds[0].IndexOf('.');
      if( di != InvalidIndex )  {
        TGPCollection* _gpc = FXApp->GetRender().FindCollection(Cmds[0].SubStringTo(di));
        if( _gpc != NULL )  {
          TGlPrimitive* glp = _gpc->FindPrimitiveByName(Cmds[0].SubStringFrom(di+1));
          if( glp != NULL )  {
            mat = &glp->GetProperties();
            smat = &_gpc->GetStyle().GetMaterial(Cmds[0].SubStringFrom(di+1), *mat);
          }
        }
        else  {  // modify the style if exists
          TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(Cmds[0].SubStringTo(di));
          if( gs != NULL )
            mat = gs->FindMaterial(Cmds[0].SubStringFrom(di+1));
        }
      }
    }
  }
  if( mat == NULL && (gpc == NULL || gpc->ObjectCount() == 0) )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined material/control ") << Cmds[0]);
    return;
  }
  TdlgMatProp* MatProp;
  if( gpc != NULL )
    MatProp = new TdlgMatProp(this, gpc->GetObject(0));
  else
    MatProp = new TdlgMatProp(this, *mat);

  if( MatProp->ShowModal() == wxID_OK )  {
    if( mat != NULL )
      *mat = MatProp->GetCurrent();
    if( smat != NULL )
      *smat = *mat;
  }
  MatProp->Destroy();
}
//..............................................................................
void TMainForm::macSetMaterial(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGlMaterial* mat = NULL;
  TGlMaterial glm(Cmds[1]);
  if( Cmds[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Cmds[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Cmds[0] == "execout" )
    mat = &ExecFontColor;
  else if( Cmds[0] == "error" )
    mat = &ErrorFontColor;
  else if( Cmds[0] == "exception" )
    mat = &ExceptionFontColor;
  else {
    size_t di = Cmds[0].IndexOf('.');
    bool found = false;
    olxstr col_name = di != InvalidIndex ? Cmds[0].SubStringTo(di) : Cmds[0];
    olxstr prm_name = di != InvalidIndex ? Cmds[0].SubStringFrom(di+1)
      : EmptyString();
    TGPCollection* gpc = FXApp->GetRender().FindCollection(col_name);
    if( gpc != NULL )  {
      if( !prm_name.IsEmpty() )  {
        TGlPrimitive* glp = gpc->FindPrimitiveByName(prm_name);
        if( glp != NULL )  {
          glp->SetProperties(glm);
          gpc->GetStyle().SetMaterial(prm_name, glm);
          found = true;
        }
      }
      else {
        for( size_t i=0; i < gpc->ObjectCount(); i++ )  {
          TGlGroup *glg = dynamic_cast<TGlGroup*>(&gpc->GetObject(i));
          if( glg != NULL ) {
            glg->SetGlM(glm);
            found = true;
          }
        }
      }
    }
    else  {  // try to modify the style then, if exists
      TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(col_name);
      if( gs != NULL )  {
        mat = gs->FindMaterial(prm_name);
        if( mat != NULL )  {
          *mat = glm;
          found = true;
        }
      }
    }
    if( !found )
      E.ProcessingError(__OlxSrcInfo, olxstr("Undefined object ") << Cmds[0]);
    return;
  }
  if( mat == NULL )
    E.ProcessingError(__OlxSrcInfo, olxstr("Undefined material ") << Cmds[0]);
  else
    *mat = glm;
}
//..............................................................................
void TMainForm::funChooseMaterial(const TStrObjList &Params, TMacroError &E)  {
  TGlMaterial glm;
  if( Params.Count() == 1 )
    glm.FromString(Params[0]);
  TdlgMatProp* MatProp = new TdlgMatProp(this, glm);
  if( MatProp->ShowModal() == wxID_OK )
    E.SetRetVal(MatProp->GetCurrent().ToString());
  else
    E.ProcessingError(__OlxSrcInfo, EmptyString());
  MatProp->Destroy();
}
//..............................................................................
void TMainForm::funGetMaterial(const TStrObjList &Params, TMacroError &E)  {
  const TGlMaterial* mat = NULL;
  if( Params[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Params[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Params[0] == "execout" )
    mat = &ExecFontColor;
  else if( Params[0] == "error" )
    mat = &ErrorFontColor;
  else if( Params[0] == "exception" )
    mat = &ExceptionFontColor;
  else  {
    size_t di = Params[0].IndexOf('.');
    if( di != InvalidIndex )  {
      TGPCollection* gpc = FXApp->GetRender().FindCollection(Params[0].SubStringTo(di));
      if( gpc != NULL )  {
        TGlPrimitive* glp = gpc->FindPrimitiveByName(Params[0].SubStringFrom(di+1));
        if( glp != NULL )
          mat = &glp->GetProperties();
      }
      else  {  // check if the style exists
        TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(Params[0].SubStringTo(di));
        if( gs != NULL )
          mat = gs->FindMaterial(Params[0].SubStringFrom(di+1));
      }
    }
    else  {
      TGPCollection* gpc = FXApp->GetRender().FindCollection(Params[0]);
      if( gpc != NULL && EsdlInstanceOf(*gpc, TGlGroup) )
        mat = &((TGlGroup*)gpc)->GetGlM();
      else  {  // check if the style exists
        TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(Params[0]);
        if( gs != NULL )
          mat = gs->FindMaterial("mat");
      }
    }
  }
  if( mat == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined material ") << Params[0]);
    return;
  }
  else  {
    if( Params.Count() == 2 )
      E.SetRetVal(mat->ToPOV());
    else
      E.SetRetVal(mat->ToString());
  }
}
//..............................................................................
void TMainForm::macLstGO(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TStrList output;
  output.SetCapacity( FXApp->GetRender().CollectionCount() );
  for( size_t i=0; i < FXApp->GetRender().CollectionCount(); i++ )  {
    TGPCollection& gpc = FXApp->GetRender().GetCollection(i);
    output.Add( gpc.GetName() ) << '[';
    for( size_t j=0; j < gpc.PrimitiveCount(); j++ )  {
      output.GetLastString() << gpc.GetPrimitive(j).GetName();
      if( (j+1) < gpc.PrimitiveCount() )
        output.GetLastString() << ';';
    }
    output.GetLastString() << "]->" << gpc.ObjectCount();
  }
  TBasicApp::NewLogEntry() << output;
}
//..............................................................................
//..............................................................................
void TMainForm::funGetMouseX(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal( ::wxGetMousePosition().x );
}
//..............................................................................
void TMainForm::funGetMouseY(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal( ::wxGetMousePosition().y );
}
//..............................................................................
void TMainForm::funGetWindowSize(const TStrObjList &Params, TMacroError &E)  {
  if( Params.IsEmpty() || Params[0].Equalsi("main") )  {
    wxRect sz = GetRect();
    E.SetRetVal( olxstr(sz.x) << ',' << sz.y << ',' << sz.width << ',' << sz.height);
  }
  else if( Params[0].Equalsi("gl") ) {
    wxRect sz = FGlCanvas->GetRect();
    E.SetRetVal( olxstr(sz.x) << ',' << sz.y << ',' << sz.width << ',' << sz.height);
  }
  else if( Params[0].Equalsi("html") ) {
    wxRect sz = HtmlManager.main->GetRect();
    E.SetRetVal( olxstr(sz.x) << ',' << sz.y << ',' << sz.width << ',' << sz.height);
  }
  else if( Params[0].Equalsi("main-cs") ) {
    if( Params.Count() == 1 )  {
      int w=0, h=0;
      GetClientSize(&w, &h);
      E.SetRetVal( olxstr('0') << ',' << '0' << ',' << w << ',' << h);
    }
    else if( Params.Count() == 3 )  {
      int w=Params[1].ToInt(), h=Params[2].ToInt();
      ClientToScreen(&w, &h);
      E.SetRetVal( olxstr(w) << ',' << h);
    }
  }
  else
    E.ProcessingError(__OlxSrcInfo, "undefined window");
}
//..............................................................................
void TMainForm::macShowSymm(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
struct PointAnalyser : public TDSphere::PointAnalyser {
  const TLattice &latt;
  TSAtom &center;
  TArrayList<uint32_t> colors;
  PointAnalyser(const TLattice &l, TXAtom &c)
    : latt(l), center(c)
  {
    size_t max_net_id = 0;
    for( size_t i=0; i < latt.GetObjects().atoms.Count(); i++ )  {
      TSAtom &a = latt.GetObjects().atoms[i];
      if( !a.IsAvailable() )  continue;
      if( a.CAtom().GetFragmentId() > max_net_id )
        max_net_id = a.CAtom().GetFragmentId();
    }
    colors.SetCount(max_net_id+1);
    if( max_net_id >= 1 )  {
      colors[0] = 0xff;
      colors[1] = 0xff00;
    }
    if( max_net_id >= 2 )  {
      colors[2] = 0xff0000;
    }
  }
  uint32_t Analyse(const vec3f &p)  {
    uint64_t cl = 0;
    size_t cnt = 0;
    for( size_t i=0; i < latt.GetObjects().atoms.Count(); i++ )  {
      TSAtom &a = latt.GetObjects().atoms[i];
      if( &a == &center || !a.IsAvailable() )
        continue;
      vec3f v = a.crd() - center.crd();
      float dp = p.DotProd(v);
      if( dp < 0 )
        continue;
      float d = (v-p*dp).Length();
      if( d < a.GetType().r_vdw )  {
        cl += colors[a.CAtom().GetFragmentId()];
        cnt++;
      }
    }
    if( cnt == 0 )
      cl = 0x00ffffff;
    else if( cnt > 1 )
      cl /= cnt;
    if( OLX_GetAValue(cl) != 0 )
      return cl;
    return cl|(127<<24);
  }
};
void TMainForm::macProjSph(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TArrayList<uint32_t> colors;
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      colors.Add(Cmds[i].SafeUInt<uint32_t>());
      Cmds.Delete(i--);
    }
  }
  TXAtomPList xatoms = FindXAtoms(Cmds, false, true);
  if( xatoms.Count() != 1 )  {
    E.ProcessingError(__OlxSourceInfo, "one atom is expected");
    return;
  }
  static size_t counter = 0;
  PointAnalyser &pa = *new PointAnalyser(FXApp->XFile().GetLattice(), *xatoms[0]);
  {
    if( pa.colors.Count() == 1 && !colors.IsEmpty() )
      pa.colors[0] = colors[0];
    else  {
      size_t cr = 0;
      for( size_t i=0; i < colors.Count(); i++ )  {
        if( xatoms[0]->CAtom().GetFragmentId() == i )
          cr = 1;
        if( i+cr >= pa.colors.Count() )
          break;
        pa.colors[i+cr] = colors[i];
      }
    }  
  }
  size_t g = Options.FindValue('g', 6).ToSizeT();
  if (g > 10)
    g = 10;
  TDSphere *sph = new TDSphere(FXApp->GetRender(), pa, olxstr("Sph_") << ++counter);
  sph->SetGeneration(g);
  sph->Create();
  sph->Basis.SetCenter(xatoms[0]->crd());
  sph->Basis.SetZoom(2);
  sph->SetMoveable(true);
  sph->SetZoomable(true);
  FXApp->AddObjectToCreate(sph);
}
//..............................................................................
  void TMainForm::macTestBinding(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr empty;
  AtomRefList arl(FXApp->XFile().GetRM(), Cmds.Text(' '), "suc");
  TTypeList<TAtomRefList> res;
  TResidue& main_resi = FXApp->XFile().GetAsymmUnit().GetResidue(0);
  arl.Expand(FXApp->XFile().GetRM(), res);
  for( size_t i=0; i < res.Count(); i++ )  {
    TBasicApp::GetLog() << NewLineSequence();
    for( size_t j=0; j < res[i].Count(); j++ )
      TBasicApp::GetLog() << res[i][j].GetExpression();
  }
  TBasicApp::NewLogEntry() << NewLineSequence() << arl.GetExpression();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )  {
    TEFile f(Cmds[0], "rb");
    uint64_t st = TETime::msNow();
    TBasicApp::NewLogEntry() << "MD5: " << MD5::Digest(f);
    TBasicApp::GetLog() <<
      olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s";
    f.SetPosition(0);
    st = TETime::msNow();
    TBasicApp::NewLogEntry() << "SHA1: " << SHA1::Digest(f);
    TBasicApp::GetLog() <<
      olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s";
    f.SetPosition(0);
    st = TETime::msNow();
    TBasicApp::NewLogEntry() << "SHA224: " << SHA224::Digest(f);
    TBasicApp::GetLog() <<
      olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s";
    f.SetPosition(0);
    st = TETime::msNow();
    TBasicApp::NewLogEntry() << "SHA256: " << SHA256::Digest(f);
    TBasicApp::GetLog() <<
      olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s";
  }
  using namespace esdl::exparse;
  EvaluableFactory evf;
  context cx;
  context::init_global(cx);
  evf.types.Add(&typeid(olxstr), new StringValue);
  evf.classes.Add(&typeid(olxstr), &StringValue::info);
  evf.types.Add(&typeid(ListValue::list_t), new ListValue);
  evf.classes.Add(&typeid(ListValue::list_t), &ListValue::info);
  //evf.classes.Add(&typeid(StringValue), &StringValue::info);
  StringValue::init_library();
  ListValue::init_library();

  exp_builder _exp(evf, cx);
  IEvaluable* iv = _exp.build("a = 'ab c, de\\';()'");
  iv = _exp.build("b = 'ab c'");
  //_exp.scope.add_var("a", new StringValue("abcdef"));
  iv = _exp.build("a.sub(0,4).sub(1,3).len()");
  if( !iv->is_final() )  {
    IEvaluable* iv1 = iv->_evaluate();
    delete iv1;
  }
  if( iv->ref_cnt() == 0 )  delete iv;
  //iv = _exp.build("x = a.sub (0,4).len() + b.len()");
  //iv = _exp.build("c = a.sub(0,3) == b.sub(0,3)", false);
  //iv = _exp.build("c = a.sub(0,3) != b.sub(0,3)");
  //iv = _exp.build("c = !(a.sub(0,3) == b.sub(0,3))");
  //iv = _exp.build("c = !(a.sub(0,4) == b.sub(0,3))");
  //iv = _exp.build("c = b.sub(0,3) + 'dfg'");
  //iv = _exp.build("c = c + 100");
  //iv = _exp.build("c = 1.2 + 1.1 - .05");
  //iv = _exp.build("a.len() + 1.2 + 1.1 - abs(-.05)*cos(PI/2)");
  iv = _exp.build("a='AaBc'.charAt(2)");
  iv = _exp.build("a='AaBc'[1].toUpper()");
  iv = _exp.build("a='100'.atoi()");
  //iv = _exp.build("a=['aBc',a,b, 1.2].add(4)");
  iv = _exp.build("a=['aBc',a,b, 1.2]");
  iv = _exp.build("a.add(['ab','ac'])");
  if (iv->ref_cnt() == 0)
    delete iv;
  iv = _exp.build("a=a[4][1][1].toUpper()");
  
  iv = _exp.build("if(a){ a = a.sub(0,3); }else{ a = a.sub(0,4); }");
  if( !iv->is_final() && false )  {
    IEvaluable* iv1 = iv->_evaluate();
    delete iv1;
    iv1 = _exp.build("a = 'cos(a)'");
    iv1 = iv->_evaluate();
    delete iv1;
    iv1 = _exp.build("a = cos(c)");
    iv1 = iv->_evaluate();
    delete iv1;
  }
  if( iv->ref_cnt() == 0 )  delete iv;
}
//..............................................................................
double Main_FindClosestDistance(const smatd_list& ml, vec3d& o_from, const TCAtom& a_to) {
  vec3d V1, V2, from(o_from), to(a_to.ccrd());
  V2 = from-to;
  a_to.GetParent()->CellToCartesian(V2);
  double minD = V2.QLength();
  for( size_t i=0; i < ml.Count(); i++ )  {
    V1 = ml[i] * from;
    V2 = V1;
    V2 -=to;
    int iv = olx_round(V2[0]);
    V2[0] -= iv;  V1[0] -= iv;  // find closest distance
    iv = olx_round(V2[1]);
    V2[1] -= iv;  V1[1] -= iv;
    iv = olx_round(V2[2]);
    V2[2] -= iv;  V1[2] -= iv;
    a_to.GetParent()->CellToCartesian(V2);
    double D = V2.QLength();
    if( D < minD )  {
      minD = D;
      o_from = V1;
    }
  }
  return sqrt(minD);
}
class TestDistanceAnalysisIteration {
  TCif cif;
  const TStrList& files;
  TAsymmUnit& au;
  smatd_list ml;
public:
  TPSTypeList<int, int> XYZ, XY, XZ, YZ, XX, YY, ZZ;  // length, occurence 

  TestDistanceAnalysisIteration( const TStrList& f_list) : 
      files(f_list), au(cif.GetAsymmUnit())  
  {  
    //TBasicApp::GetInstance().SetMaxThreadCount(1);  // reset for xfile
  }
  ~TestDistanceAnalysisIteration()  {   }
  int Run(long i)  {
    TBasicApp::NewLogEntry() << files[i];
    try { cif.LoadFromFile(files[i]);  }
    catch( ... )  {
      TBasicApp::NewLogEntry(logException) << "Failed on " << files[i];
      return 0;
    }
    TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(au);
    int cc = 0;
    ml.Clear();
    sg.GetMatrices(ml, mattAll);
    vec3d diff;
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& a1 = au.GetAtom(j);
      if( a1.GetTag() == -1 )  continue;
      if( a1.GetType() == iHydrogenZ )  continue;
      for( size_t k=j+1; k < au.AtomCount(); k++ )  {
        TCAtom& a2 = au.GetAtom(k);
        if( a2.GetTag() == -1 )  continue;
        cc ++;
        if( a2.GetType() == iHydrogenZ )  continue;
        vec3d from(a1.ccrd()), to(a2.ccrd());
        int d = olx_round(Main_FindClosestDistance(ml, from, a2)*100);
        if( d < 1 )  {  // symm eq
          a2.SetTag(-1);
          continue;
        }
        // XYZ
        size_t ind = XYZ.IndexOf(d);
        if( ind == InvalidIndex )  XYZ.Add(d, 1);
        else  XYZ.GetObject(ind)++;
        // XY
        diff[0] = from[0]-to[0];  diff[1] = from[1]-to[1]; diff[2] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = XY.IndexOf(d);
        if( ind == InvalidIndex )  XY.Add(d, 1);
        else  XY.GetObject(ind)++;
        // XZ
        diff[0] = from[0]-to[0];  diff[2] = from[2]-to[2]; diff[1] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = XZ.IndexOf(d);
        if( ind == InvalidIndex )  XZ.Add(d, 1);
        else  XZ.GetObject(ind)++;
        // YZ
        diff[1] = from[1]-to[1];  diff[2] = from[2]-to[2]; diff[0] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = YZ.IndexOf(d);
        if( ind == InvalidIndex )  YZ.Add(d, 1);
        else  YZ.GetObject(ind)++;
        // XX
        diff[0] = from[0]-to[0];  diff[1] = 0; diff[2] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = XX.IndexOf(d);
        if( ind == InvalidIndex )  XX.Add(d, 1);
        else  XX.GetObject(ind)++;
        // YY
        diff[1] = from[1]-to[1];  diff[0] = 0; diff[2] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = YY.IndexOf(d);
        if( ind == InvalidIndex )  YY.Add(d, 1);
        else  YY.GetObject(ind)++;
        // ZZ
        diff[2] = from[2]-to[2];  diff[0] = 0; diff[1] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = ZZ.IndexOf(d);
        if( ind == InvalidIndex )  ZZ.Add(d, 1);
        else  ZZ.GetObject(ind)++;
      }
    }
    return cc;
  }
  inline TestDistanceAnalysisIteration* Replicate() const {  return new TestDistanceAnalysisIteration(files);  }
};
void TMainForm::macTestStat(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TStrList files;
  TEFile::ListCurrentDir(files, "*.cif", sefFile);
  TCif cif;
  TAsymmUnit& au = cif.GetAsymmUnit();

  TPSTypeList<const cm_Element*, double*> atomTypes;
  vec3d v1;
  double tmp_data[601];
  smatd_list ml;
  for( size_t i=0; i < files.Count(); i++ )  {
    TBasicApp::NewLogEntry() << files[i];
    try { cif.LoadFromFile(files[i]);  }
    catch( ... )  {
      TBasicApp::NewLogEntry(logException) << "Failed on " << files[i];
      continue;
    }
    TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(au);
    ml.Clear();
    sg.GetMatrices(ml, mattAll);
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& a1 = au.GetAtom(j);
      if( a1.GetTag() == -1 )  continue;
      if( a1.GetType() == iHydrogenZ )  continue;
      const size_t ai = atomTypes.IndexOf(&a1.GetType());
      double* data = ((ai == InvalidIndex) ? new double[601] : atomTypes.GetObject(ai));
      if( ai == InvalidIndex )  {// new array, initialise
        memset(data, 0, sizeof(double)*600);
        atomTypes.Add(&a1.GetType(), data);
      }
      vec3d from(a1.ccrd());
      for( size_t k=j+1; k < au.AtomCount(); k++ )  {
        TCAtom& a2 = au.GetAtom(k);
        if( a2.GetTag() == -1 )  continue;
        if( a2.GetType() == iHydrogenZ )  continue;
        vec3d to(a2.ccrd());
        memset(tmp_data, 0, sizeof(double)*600);
        for( size_t l=0; l < ml.Count(); l++ )  {
          v1 = ml[l] * to - from;
          v1[0] -= olx_round(v1[0]);  v1[1] -= olx_round(v1[1]);  v1[2] -= olx_round(v1[2]);
          au.CellToCartesian(v1);
          double d = v1.Length();
          if( d <= 0.01 )  {
            a2.SetTag(-1);
            break;
          }
          if( d < 6 )
            tmp_data[olx_round(d*100)] ++;
        }
        if( a2.GetTag() != -1 )  {
          for( size_t l=0; l < 600; l++ )
            data[l] += tmp_data[l];
        }
      }
    }
    FXApp->Draw();
    wxTheApp->Dispatch();
  }
  memset(tmp_data, 0, sizeof(double)*600);
  TEFile out("c:\\tmp\\bin_r5.db", "w+b");
  for( size_t i=0; i < atomTypes.Count(); i++ )  {
    double* data = atomTypes.GetObject(i);
    TCStrList sl;
    sl.SetCapacity( 600 );
    double sq = 0;
    for( size_t j=0; j < 600; j++ )  {
      if( j < 598 )
        sq += ((data[j+1]+data[j])*0.01/2.0);
    }
    for( size_t j=0; j < 600; j++ )  {
      data[j] /= sq;
      tmp_data[j] += data[j]*100.0;
      sl.Add( (double)j/100 ) << '\t' << data[j]*1000.0;  // normalised by 1000 square
    }
    sl.SaveToFile( (Cmds[0]+'_') << atomTypes.GetKey(i)->symbol << ".xlt");
    out << (int16_t)atomTypes.GetKey(i)->index;
    out.Write(data, 600*sizeof(double));
    delete [] data;
  }
  TCStrList sl;
  sl.SetCapacity( 600 );
  for( size_t i=0; i < 600; i++ )
    sl.Add( (double)i/100 ) << '\t' << tmp_data[i];
  sl.SaveToFile( (Cmds[0]+"_all") << ".xlt");
  return;
// old procedure
  TestDistanceAnalysisIteration testdai(files);
//  FXApp->SetMaxThreadCount(4);
//  TListIteratorManager<TestDistanceAnalysisIteration>* Test = new TListIteratorManager<TestDistanceAnalysisIteration>(testdai, files.Count(), tLinearTask, 0);
//  delete Test;
  if( files.Count() == 1 && TEFile::Exists(Cmds[0]) )  {
    TStrList sl;
    sl.LoadFromFile( Cmds[0] );
    TPSTypeList<int, int> ref_data, &data = testdai.XYZ;
    for( size_t i=0; i < sl.Count(); i++ )  {
      const size_t ind = sl[i].IndexOf('\t');
      if( ind == InvalidIndex )  {
        TBasicApp::NewLogEntry() << "could not parse '" << sl[i] << "\'";
        continue;
      }
      ref_data.Add( olx_round(sl[i].SubStringTo(ind).ToDouble()*100), sl[i].SubStringFrom(ind+1).ToInt() );
    }
    double R = 0;
    for( size_t i=0; i < data.Count(); i++ )  {
      const size_t ind = ref_data.IndexOf(data.GetKey(i));
      if( ind == InvalidIndex )  {
        TBasicApp::NewLogEntry() << "undefined distance " << data.GetKey(i);
        continue;
      }
      R += sqrt( (double)ref_data.GetObject(ind)*data.GetObject(i) );
    }
//    if( cc != 0 )
//      TBasicApp::GetLog() << (olxstr("Rc=") << R/cc);
  }
  else  {  // just save the result
    TStrPObjList<olxstr, TPSTypeList<int, int>* > data;
    data.Add("xyz", &testdai.XYZ);
    data.Add("xx", &testdai.XX);
    data.Add("xy", &testdai.XY);
    data.Add("xz", &testdai.XZ);
    data.Add("yy", &testdai.YY);
    data.Add("yz", &testdai.YZ);
    data.Add("zz", &testdai.ZZ);
    for( size_t i=0; i < data.Count(); i++ )  {
      TCStrList sl;
      TPSTypeList<int, int>& d = *data.GetObject(i);
      sl.SetCapacity( d.Count() );
      for( size_t j=0; j < d.Count(); j++ )  {
        sl.Add( (double)d.GetKey(j)/100 ) << '\t' << d.GetObject(j);
      }
      sl.SaveToFile( (Cmds[0]+data[i]) << ".xlt");
    }
  }
}
//..............................................................................
void TMainForm::macExportFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene* wxs = dynamic_cast<TwxGlScene*>(&FXApp->GetRender().GetScene());
  if( wxs == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "invalid scene object");
    return;
  }
  wxs->ExportFont(Cmds[0], Cmds[1]);
}
//..............................................................................
void TMainForm::macImportFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene& wxs = dynamic_cast<TwxGlScene&>(FXApp->GetRender().GetScene());
  TGlFont* glf = wxs.FindFont(Cmds[0]);
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Cmds[0]);
    return;
  }
  glf->SetIdString(Cmds[1]);
  wxs.ImportFont(*glf);
}
//..............................................................................
class Esd_Tetrahedron  {
  TSAtomPList atoms;
  olxstr Name;
  TEValue<double> Volume;
  VcoVContainer& vcov;
protected:
  void CalcVolume()  {
    Volume =
      vcov.CalcTetrahedronVolume(*atoms[0], *atoms[1], *atoms[2], *atoms[3]);
  }
public:
  Esd_Tetrahedron(const olxstr& name, const VcoVContainer& _vcov)
    : Volume(-1,0), vcov(const_cast<VcoVContainer&>(_vcov))
  {
    Name = name;
  }
  void Add( TSAtom* a )  {
    atoms.Add( a );
    if( atoms.Count() == 4 )
      CalcVolume();
  }
  const olxstr& GetName() const  {  return Name;  }
  double GetVolume() const {  return Volume.GetV();  }
  double GetEsd() const {  return Volume.GetE();  }
};
int Esd_ThSort(const Esd_Tetrahedron &th1, const Esd_Tetrahedron &th2)  {
  return olx_cmp(th1.GetVolume(), th2.GetVolume());
}
void TMainForm::macEsd(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  VcoVContainer vcovc(FXApp->XFile().GetAsymmUnit());
  try  {
    olxstr src_mat = FXApp->InitVcoV(vcovc);
    FXApp->NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
  }
  catch(TExceptionBase& e)  {
    Error.ProcessingError(__OlxSrcInfo, e.GetException()->GetError());
    //throw TFunctionFailedException(__OlxSourceInfo, e);
    return;
  }
  TStrList values;
  TGlGroup& sel = FXApp->GetSelection();
  if (Options.Contains('l')) {
    for (size_t i=0; i < sel.Count(); i++) {
      if (EsdlInstanceOf(sel[i], TXBond)) {
        TXBond &xb = (TXBond&)sel[i];
        values.Add(xb.A().GetLabel()) << " to " << xb.B().GetLabel() <<
          " distance: " << vcovc.CalcDistance(xb.A(), xb.B()).ToString() <<
          " A";
      }
    }
  }
  else {
    if( sel.Count() == 1 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) )  {
        TSAtomPList atoms;
        TXAtom& xa = (TXAtom&)sel[0];
        for( size_t i=0; i < xa.NodeCount(); i++ ) {
          TSAtom& A = xa.Node(i);
          if( A.IsDeleted() || (A.GetType() == iQPeakZ) )
            continue;
          atoms.Add(A);
        }
        if( atoms.Count() == 3 )
          atoms.Add(xa);
        if( atoms.Count() < 4 )  {
          Error.ProcessingError(__OlxSrcInfo,
            "An atom with at least four bonds is expected");
          return;
        }
        TTypeList<Esd_Tetrahedron> tetrahedra;
        // special case for 4 nodes
        if( atoms.Count() == 4 )  {
          Esd_Tetrahedron& th = tetrahedra.AddNew( olxstr(atoms[0]->GetLabel() ) << '-'
            << atoms[1]->GetLabel() << '-'
            << atoms[2]->GetLabel() << '-'
            << atoms[3]->GetLabel(), vcovc);
          th.Add(atoms[0]);
          th.Add(atoms[1]);
          th.Add(atoms[2]);
          th.Add(atoms[3]);
        }
        else  {
          for( size_t i=0; i < atoms.Count(); i++ ) {
            for( size_t j=i+1; j < atoms.Count(); j++ ) {
              for( size_t k=j+1; k < atoms.Count(); k++ ) {
                Esd_Tetrahedron& th = tetrahedra.AddNew(olxstr(xa.GetLabel()) << '-'
                  << atoms[i]->GetLabel() << '-'
                  << atoms[j]->GetLabel() << '-'
                  << atoms[k]->GetLabel(), vcovc);
                th.Add(&xa);
                th.Add(atoms[i]);
                th.Add(atoms[j]);
                th.Add(atoms[k]);
              }
            }
          }
        }
        const size_t thc = (atoms.Count()-2)*2;
        QuickSorter::SortSF(tetrahedra, &Esd_ThSort);
        bool removed = false;
        while(  tetrahedra.Count() > thc )  {
          TBasicApp::NewLogEntry() << "Removing tetrahedron " <<  tetrahedra[0].GetName() <<
            " with volume " << tetrahedra[0].GetVolume();
          tetrahedra.Delete(0);
          removed = true;
        }
        double v = 0, esd = 0;
        for( size_t i=0; i < tetrahedra.Count(); i++ )  {
          v += tetrahedra[i].GetVolume();
          esd += tetrahedra[i].GetEsd()*tetrahedra[i].GetEsd();
        }
        TEValue<double> ev(v, sqrt(esd));
        if( removed )  {
          values.Add("The volume for remaining tetrahedra is ") <<
            ev.ToString() << " A^3";
        }
        else  {
          values.Add("The tetrahedra volume is ") << ev.ToString() << " A^3";
        }
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) )  {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[0];
        olxstr pld;
        for( size_t i=0; i < xp.Count(); i++ )  {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        values.Add("Plane ") << pld <<
          (values.Add("RMS: ") << vcovc.CalcPlane(atoms).ToString()) << " A";
        TEPoint3<double> c_cent(vcovc.CalcCentroid(atoms));
        values.Add("Plane ") << pld << "cartesian centroid : {" << c_cent[0].ToString() <<
          ", " << c_cent[1].ToString() << ", " << c_cent[2].ToString() << "}";
        TEPoint3<double> f_cent(vcovc.CalcCentroidF(atoms));
        values.Add("Plane ") << pld << "fractional centroid : {" << f_cent[0].ToString() <<
          ", " << f_cent[1].ToString() << ", " << f_cent[2].ToString() << "}";
      }
      else if( EsdlInstanceOf(sel[0], TXBond) )  {
        TXBond& xb = (TXBond&)sel[0];
        values.Add(xb.A().GetLabel()) << " to " << xb.B().GetLabel() <<
          " distance: " << vcovc.CalcDistance(xb.A(), xb.B()).ToString() <<
          " A";
      }
    }
    else if( sel.Count() == 2 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXAtom) )  {
        values.Add(((TXAtom&)sel[0]).GetLabel()) << " to " <<
          ((TXAtom&)sel[1]).GetLabel() << " distance: " <<
          vcovc.CalcDistance((TXAtom&)sel[0], (TXAtom&)sel[1]).ToString() << " A";
      }
      else if( EsdlInstanceOf(sel[0], TXBond) && EsdlInstanceOf(sel[1], TXBond) )  {
        TSBond& b1 = ((TXBond&)sel[0]);
        TSBond& b2 = ((TXBond&)sel[1]);
        TEValue<double> v(vcovc.CalcB2BAngle(b1.A(), b1.B(), b2.A(), b2.B())),
          v1(180-v.GetV(), v.GetE());
        values.Add(b1.A().GetLabel()) << '-' << b1.B().GetLabel() << " to " <<
          b2.A().GetLabel() << '-' << b2.B().GetLabel() << " angle: " <<
          v.ToString() << '(' << v1.ToString() << ')';
      }
      else if( (EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXPlane)) ||  
               (EsdlInstanceOf(sel[1], TXAtom) && EsdlInstanceOf(sel[0], TXPlane)))  {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[ EsdlInstanceOf(sel[0], TXPlane) ? 0 : 1];
        olxstr pld;
        for( size_t i=0; i < xp.Count(); i++ )  {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        TSAtom& sa = ((TXAtom&)sel[EsdlInstanceOf(sel[0],TXAtom) ? 0 : 1]);
        values.Add(sa.GetLabel()) << " to plane " << pld << "distance: " <<
          vcovc.CalcP2ADistance(atoms, sa).ToString() << " A";
        values.Add(sa.GetLabel()) << " to plane " << pld <<
          "centroid distance: " <<
          vcovc.CalcPC2ADistance(atoms, sa).ToString() << " A";
      }
      else if( (EsdlInstanceOf(sel[0], TXBond) && EsdlInstanceOf(sel[1], TXPlane)) ||  
               (EsdlInstanceOf(sel[1], TXBond) && EsdlInstanceOf(sel[0], TXPlane)))  {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[ EsdlInstanceOf(sel[0], TXPlane) ? 0 : 1];
        olxstr pld;
        for( size_t i=0; i < xp.Count(); i++ )  {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        TSBond& sb = ((TXBond&)sel[ EsdlInstanceOf(sel[0], TXBond) ? 0 : 1]);
        TEValue<double> v(vcovc.CalcP2VAngle(atoms, sb.A(), sb.B())),
          v1(180-v.GetV(), v.GetE());
        values.Add(sb.A().GetLabel()) << '-' << sb.B().GetLabel() << " to plane "
          << pld << "angle: " << v.ToString() << '(' << v1.ToString() << ')';
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXPlane) )  {
        TSAtomCPList p1, p2;
        TXPlane& xp1 = (TXPlane&)sel[0];
        TXPlane& xp2 = (TXPlane&)sel[1];
        olxstr pld1, pld2;
        for( size_t i=0; i < xp1.Count(); i++ )  {
          p1.Add(xp1.GetAtom(i));
          pld1 << p1.GetLast()->GetLabel() << ' ';
        }
        for( size_t i=0; i < xp2.Count(); i++ )  {
          p2.Add(xp2.GetAtom(i));
          pld2 << p2.GetLast()->GetLabel() << ' ';
        }
        const TEValue<double> angle = vcovc.CalcP2PAngle(p1, p2);
        values.Add("Plane ") << pld1 << "to plane angle: " << angle.ToString();
        values.Add("Plane centroid to plane centroid distance: ") <<
          vcovc.CalcPC2PCDistance(p1, p2).ToString() << " A";
        values.Add("Plane [") << pld1 << "] to plane centroid distance: " <<
          vcovc.CalcP2PCDistance(p1, p2).ToString() << " A";
        values.Add("Plane [") << pld1 << "] to plane shift: " <<
          vcovc.CalcP2PShiftDistance(p1, p2).ToString() << " A";
        if( olx_abs(angle.GetV()) > 1e-6 )  {
          values.Add("Plane [") << pld2 << "] to plane centroid distance: " <<
            vcovc.CalcP2PCDistance(p2, p1).ToString() << " A";
          values.Add("Plane [") << pld2 << "] to plane shift distance: " <<
            vcovc.CalcP2PShiftDistance(p2, p1).ToString() << " A";
        }
      }
    }
    else if( sel.Count() == 3 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) &&
          EsdlInstanceOf(sel[1], TXAtom) &&
          EsdlInstanceOf(sel[2], TXAtom) )
      {
        TSAtom& a1 = (TXAtom&)sel[0];
        TSAtom& a2 = (TXAtom&)sel[1];
        TSAtom& a3 = (TXAtom&)sel[2];
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel()
          << " angle (numerical): " << vcovc.CalcAngle(a1, a2, a3).ToString();
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel()
          << " angle (analytical): " << vcovc.CalcAngleA(a1, a2, a3).ToString();
      }
      else if(
        (EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXAtom) &&
         EsdlInstanceOf(sel[2], TXAtom)) ||
        (EsdlInstanceOf(sel[1], TXPlane) && EsdlInstanceOf(sel[0], TXAtom) &&
         EsdlInstanceOf(sel[2], TXAtom)) ||
        (EsdlInstanceOf(sel[2], TXPlane) && EsdlInstanceOf(sel[1], TXAtom) &&
         EsdlInstanceOf(sel[0], TXAtom)))
      {
        TSAtom* a1 = NULL, *a2 = NULL;
        TXPlane* xp = NULL;
        TSAtomCPList atoms;
        for( size_t  i=0; i < 3; i++ )  {
          if( EsdlInstanceOf(sel[i], TXPlane) )
            xp = &(TXPlane&)sel[i];
          else  {
            if( a1 == NULL )
              a1 = &((TXAtom&)sel[i]);
            else
              a2 = &((TXAtom&)sel[i]);
          }
        }
        olxstr pld;
        for( size_t i=0; i < xp->Count(); i++ )  {
          atoms.Add(xp->GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        values.Add(a1->GetLabel()) << '-' << a2->GetLabel() << " to plane " <<
          pld << "angle: " << vcovc.CalcP2VAngle(atoms, *a1, *a2).ToString();
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) &&
        EsdlInstanceOf(sel[1], TXPlane) &&
        EsdlInstanceOf(sel[2], TXPlane) )
      {
        TSPlane& p1 = (TXPlane&)sel[0];
        TSPlane& p2 = (TXPlane&)sel[1];
        TSPlane& p3 = (TXPlane&)sel[2];
        TSAtomCPList a1, a2, a3;
        for( size_t i=0; i < p1.Count(); i++ )  a1.Add(p1.GetAtom(i));
        for( size_t i=0; i < p2.Count(); i++ )  a2.Add(p2.GetAtom(i));
        for( size_t i=0; i < p3.Count(); i++ )  a3.Add(p3.GetAtom(i));
        values.Add("Angle between plane centroids: ") <<
          vcovc.Calc3PCAngle(a1, a2, a3).ToString();
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) &&
              EsdlInstanceOf(sel[1], TXAtom) &&
              EsdlInstanceOf(sel[2], TXPlane) )
      {
        TSPlane& p1 = (TXPlane&)sel[0];
        TSAtom& a = (TXAtom&)sel[1];
        TSPlane& p2 = (TXPlane&)sel[2];
        TSAtomCPList a1, a2;
        for( size_t i=0; i < p1.Count(); i++ )  a1.Add(p1.GetAtom(i));
        for( size_t i=0; i < p2.Count(); i++ )  a2.Add(p2.GetAtom(i));
        values.Add("Angle between plane centroid - atom - plane centroid: ") <<
          vcovc.CalcPCAPCAngle(a1, a, a2).ToString();
      }
    }
    else if( sel.Count() == 4 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXAtom) && 
          EsdlInstanceOf(sel[2], TXAtom) && EsdlInstanceOf(sel[3], TXAtom) )  {
        TSAtom& a1 = (TXAtom&)sel[0];
        TSAtom& a2 = (TXAtom&)sel[1];
        TSAtom& a3 = (TXAtom&)sel[2];
        TSAtom& a4 = (TXAtom&)sel[3];
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' <<
          a3.GetLabel() << '-' << a4.GetLabel() <<
          " torsion angle (numerical): " <<
          vcovc.CalcTAngle(a1, a2, a3, a4).ToString();
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' <<
          a3.GetLabel() << '-' << a4.GetLabel() <<
          " torsion angle (analytical): " <<
          vcovc.CalcTAngleA(a1, a2, a3, a4).ToString();
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' <<
          a3.GetLabel() << '-' << a4.GetLabel() << " tetrahedron volume: " <<
          vcovc.CalcTetrahedronVolume(a1, a2, a3, a4).ToString() << " A^3";
      }
    }
    else if( sel.Count() == 7 )  {
      TSAtomPList atoms, face_atoms, sorted_atoms;
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXAtom) )
          atoms.Add((TXAtom&)sel[i]);
      }
      if( atoms.Count() != 7 )
        return;
      values.Add("Octahedral distortion is (using best line, for the selection): ") << 
        vcovc.CalcOHDistortionBL(TSAtomCPList(atoms)).ToString();
      values.Add("Octahedral distortion is (using best plane, for the selection): ") << 
        vcovc.CalcOHDistortionBP(TSAtomCPList(atoms)).ToString();
      TSAtom* central_atom = atoms[0];
      atoms.Delete(0);
      olxdict<index_t, vec3d, TPrimitiveComparator> transforms;
      int face_cnt = 0;
      double total_val_bp = 0, total_esd_bp = 0;//, total_val_bl = 0, total_esd_bl=0;
      for( size_t i=0; i < 6; i++ )  {
        for( size_t j=i+1; j < 6; j++ )  {
          for( size_t k=j+1; k < 6; k++ )  {
            const double thv = olx_tetrahedron_volume( 
              central_atom->crd(),
              (atoms[i]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
              (atoms[j]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
              (atoms[k]->crd()-central_atom->crd()).Normalise() + central_atom->crd());
            if( thv < 0.1 )  continue;
            sorted_atoms.Clear();
            transforms.Clear();
            for( size_t l=0; l < atoms.Count(); l++ )
              atoms[l]->SetTag(0);
            atoms[i]->SetTag(1);  atoms[j]->SetTag(1);  atoms[k]->SetTag(1);
            const vec3d face_center = (atoms[i]->crd()+atoms[j]->crd()+atoms[k]->crd())/3;
            const vec3d normal = (face_center - central_atom->crd()).Normalise();
            transforms.Add(1, central_atom->crd() - face_center);
            vec3d face1_center;
            for( size_t l=0; l < atoms.Count(); l++ )
              if( atoms[l]->GetTag() == 0 )
                face1_center += atoms[l]->crd();
            face1_center /= 3;
            transforms.Add(0, central_atom->crd() - face1_center);
            PlaneSort::Sorter::DoSort(atoms, transforms, central_atom->crd(), normal, sorted_atoms);
            values.Add("Face ") << ++face_cnt << ": ";
            for( size_t l=0; l < sorted_atoms.Count(); l++ )
              values.GetLastString() << sorted_atoms[l]->GetLabel() << ' ';
            sorted_atoms.Insert(0, central_atom);
            TEValue<double> rv = vcovc.CalcOHDistortionBP(
              TSAtomCPList(sorted_atoms));
            total_val_bp += rv.GetV()*3;
            total_esd_bp += olx_sqr(rv.GetE()); 
            values.GetLastString() << rv.ToString();
            //TBasicApp::GetLog() << "BP: " << rv.ToString();
            //rv = vcovc.CalcOHDistortionBL(sorted_atoms);
            //total_val_bl += olx_abs(180.0 - rv.GetV() * 3);
            //total_esd_bl += olx_sqr(rv.GetE()); 
            //TBasicApp::NewLogEntry() << " BL: " << rv.ToString();
          }
        }
      }
      if( face_cnt == 8 )  {
        values.Add("Combined distortion (best plane): ") <<
          TEValue<double>(total_val_bp, 3*sqrt(total_esd_bp)).ToString();
        //TBasicApp::NewLogEntry() << "Combined distortion (best line): " << TEValue<double>(total_val_bl, 3*sqrt(total_esd_bl)).ToString()');
      }
      else  {
        TBasicApp::NewLogEntry() << "Could not locate required 8 octahedron faces";
      }
    }
  }
  TBasicApp::NewLogEntry() << values;
  if (Options.Contains('c'))
    FXApp->ToClipboard(values);
  if( Options.Contains("label") )  {
    vec3d cent;
    size_t cnt = 0;
    TGlGroup& gl = FXApp->GetSelection();
    for( size_t i=0; i < gl.Count(); i++ )  {
      if( EsdlInstanceOf(gl[i], TXAtom) )  {
        cent += ((TXAtom&)gl[i]).crd();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXBond) ) {
        cent += ((TXBond&)gl[i]).GetCenter();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXPlane) ) {
        cent += ((TXPlane&)gl[i]).GetCenter();
        cnt++;
      }
    }
    if( cnt != 0 )
      cent /= cnt;
    FXApp->CreateLabel(cent, values.Text('\n'), 4);
    TimePerFrame = FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macImportFrag(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TStrList content;
  if (Options.Contains('c')) {
    olxstr clipbrd_content;
    if( wxTheClipboard->Open() )  {
      if (wxTheClipboard->IsSupported(wxDF_TEXT) )  {
        wxTextDataObject data;
        wxTheClipboard->GetData(data);
        clipbrd_content = data.GetText();
      }
      wxTheClipboard->Close();
    }
    clipbrd_content.Trim(' ').Trim('\n').Trim('\r');
    if( !clipbrd_content.StartsFromi("FRAG") ||
        !clipbrd_content.EndsWithi("FEND") )
    {
      E.ProcessingError(__OlxSrcInfo, "Unrecognisable clipboard content");
      return;
    }
    content.Strtok(clipbrd_content, '\n');
    content.Delete(content.Count()-1);
    content.Delete(0);
    for( size_t i=0; i < content.Count(); i++ )  {
      TStrList toks(content[i].Trim('\r'), ' ');
      if( toks.Count() != 5 )  {
        content[i].SetLength(0);
        continue;
      }
      toks.Delete(1);
      content[i] = toks.Text(' ');
    }
  }
  else {
    olxstr FN = PickFile("Load Fragment", "XYZ files (*.xyz)|*.xyz",
      XLibMacros::CurrentDir, true);
    if (FN.IsEmpty()) {
      E.ProcessingError(__OlxSrcInfo, "A file is expected");
      return;
    }
    content.LoadFromFile(FN);
  }
  TXyz xyz;
  xyz.LoadFromStrings(content);
  TXAtomPList xatoms;
  TXBondPList xbonds;
  LabelCorrector lc(FXApp->XFile().GetAsymmUnit());
  FXApp->AdoptAtoms(xyz.GetAsymmUnit(), xatoms, xbonds);
  const int part = Options.FindValue("p", "-100").ToInt();
  for (size_t i=0; i < xatoms.Count(); i++) {
    FXApp->XFile().GetRM().Vars.FixParam(
      xatoms[i]->CAtom(), catom_var_name_Sof);
    lc.Correct(xatoms[i]->CAtom());
    if (part != -100)
      xatoms[i]->CAtom().SetPart(part);
  }
  if( xatoms.IsEmpty() )  return;
  Macros.ProcessMacro("mode fit", E);
  const int afix = Options.FindValue("a", "-100").ToInt();
  if( afix != -100 )  {
    TCAtom* pivot = TAfixGroup::HasExcplicitPivot(afix) ? &xatoms[0]->CAtom()
      : NULL;
    TAfixGroup& ag = FXApp->XFile().GetRM().AfixGroups.New(pivot, afix);
    const size_t start = pivot != NULL ? 1 : 0;
    for( size_t i=start; i < xatoms.Count(); i++ )
      ag.AddDependent(xatoms[i]->CAtom());
  }
  else if( Options.Contains('d') )  {
    RefinementModel& rm = FXApp->XFile().GetRM();
    olxdict<double, TSimpleRestraint*, TPrimitiveComparator> r12, r13;
    for( size_t i=0; i < xatoms.Count(); i++ )  {
      TXAtom& a = *xatoms[i];
      for( size_t j=0; j < a.BondCount(); j++ )  {
        TXAtom& b = a.Bond(j).Another(a);
        if( b.GetOwnerId() <= a.GetOwnerId() )  continue;
        const double d = (double)olx_round(a.Bond(j).Length()*1000)/1000;
        const size_t ri = r12.IndexOf(d);
        TSimpleRestraint& df = (ri == InvalidIndex) ? rm.rDFIX.AddNew()
          : *r12.GetValue(ri);
        df.AddAtomPair(a.CAtom(), NULL, b.CAtom(), NULL);
        if( ri == InvalidIndex )  {
          df.SetValue(d);
          df.SetEsd(0.02);
          r12.Add(d, &df);
        }
        for( size_t k=0; k < b.NodeCount(); k++ )  {
          TSAtom& b1 = b.Node(k);
          if( b1.GetOwnerId() <= a.GetOwnerId() )  continue;
          const double d1 = olx_round(a.crd().DistanceTo(b1.crd()), 1000);
          const size_t ri1 = r13.IndexOf(d1);
          TSimpleRestraint& df1 = (ri1 == InvalidIndex) ? rm.rDFIX.AddNew()
            : *r13.GetValue(ri1);
          df1.AddAtomPair(a.CAtom(), NULL, b1.CAtom(), NULL);
          if( ri1 == InvalidIndex )  {
            df1.SetValue(d1);
            df1.SetEsd(0.04);
            r13.Add(d1, &df1);
          }
        }
      }
    }
  }
  FXApp->CenterView(true);
  FXApp->SelectAll(false);
  AMode *md = Modes->GetCurrent();
  if( md != NULL )  {
    md->AddAtoms(xatoms);
    for( size_t i=0; i < xbonds.Count(); i++ )
      FXApp->GetRender().Select(*xbonds[i], true);
  }
}
//..............................................................................
void TMainForm::macExportFrag(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList xatoms;
  TGlGroup& glg = FXApp->GetSelection();
  for( size_t i=0; i < glg.Count(); i++ )  {
    if( EsdlInstanceOf(glg[i], TXAtom) )
      xatoms.Add((TXAtom&)glg[i]);
  }
  TNetPList nets;
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    TNetwork* net = &xatoms[i]->GetNetwork();
    if( nets.IndexOf(net) == InvalidIndex )
      nets.Add(net);
  }
  if( nets.Count() != 1 )  {
    E.ProcessingError(__OlxSrcInfo, "please select one fragment or one atom only");
    return;
  }
  olxstr FN = PickFile("Save Fragment as...", "XYZ files (*.xyz)|*.xyz", XLibMacros::CurrentDir, false);
  if( FN.IsEmpty() )  return;
  TXyz xyz;
  for( size_t i=0; i < nets[0]->NodeCount(); i++ )  {
    if( nets[0]->Node(i).IsDeleted() || nets[0]->Node(i).GetType() == iQPeakZ )
      continue;
    TCAtom& ca = xyz.GetAsymmUnit().NewAtom();
    ca.ccrd() = nets[0]->Node(i).crd();
    ca.SetType(nets[0]->Node(i).GetType());
  }
  xyz.SaveToFile(FN);
}
//..............................................................................
void TMainForm::macConn(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  try  {  
    TStrList lst(Cmds);
    if( (Cmds.Count() == 1 && Cmds[0].IsNumber()) || 
        (Cmds.Count() == 2 && Cmds[0].IsNumber() && Cmds[1].IsNumber()) )  
    {
      TCAtomPList atoms = FXApp->FindCAtoms("sel", false);
      if( atoms.IsEmpty() )  return;
      for( size_t i=0; i < atoms.Count(); i++ )
        lst.Add("#c") << atoms[i]->GetId();
    }
    FXApp->XFile().GetRM().Conn.ProcessConn(lst);
    FXApp->GetRender().SelectAll(false);
    FXApp->XFile().GetLattice().UpdateConnectivityInfo();
  }
  catch( const TExceptionBase& exc )  {
    E.ProcessingError(__OlxSrcInfo, exc.GetException()->GetError());
  }
}
//..............................................................................
void TMainForm::macAddBond(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList atoms;
  if( !FindXAtoms(Cmds, atoms, false, true) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  if( (atoms.Count() % 2) != 0 )  {
    E.ProcessingError(__OlxSrcInfo, "even number if atoms is expected");
    return;
  }
  for( size_t i=0; i < atoms.Count(); i += 2 )  {
    FXApp->XFile().GetRM().Conn.AddBond(
      atoms[i]->CAtom(), atoms[i+1]->CAtom(),
      &FXApp->XFile().GetRM().AddUsedSymm(atoms[i]->GetMatrix()),
      &FXApp->XFile().GetRM().AddUsedSymm(atoms[i+1]->GetMatrix()),
      true
    );
  }
  FXApp->XFile().GetLattice().UpdateConnectivityInfo();
}
//..............................................................................
void TMainForm::macDelBond(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TSAtomPList pairs;
  if( Cmds.IsEmpty() )  {
    TGlGroup& glg = FXApp->GetSelection();
    for( size_t i=0; i < glg.Count(); i++ )  {
      if( EsdlInstanceOf(glg[i], TXBond) )  {
        TXBond& sb = (TXBond&)glg[i];
        pairs.Add(sb.A());
        pairs.Add(sb.B());
      }
      else if( EsdlInstanceOf(glg[i], TXAtom) &&
        i+1 < glg.Count() && EsdlInstanceOf(glg[i+1], TXAtom) )
      {
        pairs.Add((TXAtom&)glg[i]);
        pairs.Add((TXAtom&)glg[++i]);
      }
    }
  }
  else  {
    TXAtomPList atoms = FindXAtoms(Cmds, false, false);
    if( (atoms.Count()%2) == 0 )
      pairs.AddList(atoms);
  }
  if( !pairs.IsEmpty()  )  {
    const TUnitCell &uc = FXApp->XFile().GetUnitCell();
    const smatd &fm = uc.GetMatrix(0);
    for( size_t i=0; i < pairs.Count(); i+=2 )  {
      TCAtom &a1 = pairs[i]->CAtom(),
        &a2 = pairs[i+1]->CAtom();
      for( size_t fasi=0; fasi <= a1.EquivCount(); fasi++ )  {
        const smatd m1 = uc.MulMatrix(pairs[i]->GetMatrix(),
          (fasi == a1.EquivCount() ? fm : a1.GetEquiv(fasi)));
        for( size_t sasi=0; sasi <= a2.EquivCount(); sasi++ )  {
          const smatd m2 = uc.MulMatrix(pairs[i+1]->GetMatrix(),
            (sasi == a2.EquivCount() ? fm : a2.GetEquiv(sasi)));
          FXApp->XFile().GetRM().Conn.RemBond(
            a1, a2,
            &FXApp->XFile().GetRM().AddUsedSymm(m1),
            &FXApp->XFile().GetRM().AddUsedSymm(m2),
            true);
        }
      }
    }
    FXApp->GetRender().SelectAll(false);
    FXApp->XFile().GetLattice().UpdateConnectivityInfo();
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "please select some bonds or provide atom pairs");
    return;
  }
}
//..............................................................................
void TMainForm::macUpdateQPeakTable(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  QPeakTable(false);
}
//..............................................................................
void TMainForm::funCheckState(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(TStateRegistry::GetInstance().CheckState(Params[0],
    Params.Count() == 2 ? Params[1] : EmptyString()));
}
//..............................................................................
void TMainForm::funGlTooltip(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( _UseGlTooltip );
  else
    UseGlTooltip( Params[0].ToBool() );
}
//..............................................................................
void TMainForm::funCurrentLanguage(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(Dictionary.GetCurrentLanguage());
  else
    Dictionary.SetCurrentLanguage(DictionaryFile, Params[0] );
}
//..............................................................................
void macSAME_expand(const TArrayList<TSAtomPList>& groups)  {
  throw TNotImplementedException(__OlxSourceInfo);
  //TXApp& xapp = TXApp::GetInstance();
  //xapp.XFile().GetAsymmUnit().GetAtoms().ForEach(ACollectionItem::TagSetter(-1));
  //for( size_t i=0; i < groups[0].Count(); i++ )  {
  //  for( size_t 
  //}
}
void TMainForm::macSAME(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const bool invert = Options.Contains("i"),
    expand = Options.Contains("e");
  size_t groups_count = InvalidSize;
  if( !Cmds.IsEmpty() && Cmds[0].IsNumber() )  {
    groups_count = Cmds[0].ToSizeT();
    Cmds.Delete(0);
  }
  TXAtomPList atoms = FindXAtoms(Cmds, false, true);
  if( atoms.Count() == 2 )  {
    TTypeList< AnAssociation2<size_t, size_t> > res;
    TNetwork &netA = atoms[0]->GetNetwork(),
      &netB = atoms[1]->GetNetwork();
    if( &netA == &netB )  {
      E.ProcessingError(__OlxSrcInfo, "Please select different fragments");
      return;
    }
    if( !netA.DoMatch(netB, res, invert, &TSAtom::weight_occu) )  {
      E.ProcessingError(__OlxSrcInfo, "Graphs do not match");
      return;
    }
    //got the pairs now...
    if( expand )  {
      TArrayList<TSAtomPList> groups(2);
      groups[0].SetCapacity(res.Count());
      groups[1].SetCapacity(res.Count());
      for( size_t i=0; i < res.Count(); i++ )  {
        if( netA.Node(res[i].GetA()).GetType().GetMr() < 3.5 )  continue;
        groups[0].Add(netA.Node(res[i].GetA()));
        groups[1].Add(netB.Node(res[i].GetB()));
      }
      macSAME_expand(groups);
    }
    else  {
      TSameGroup& sg = FXApp->XFile().GetRM().rSAME.New();
      for( size_t i=0; i < netA.NodeCount(); i++ )  {
        netA.Node(i).SetTag(-1);
        netB.Node(i).SetTag(-1);
      }
      for( size_t i=0; i < res.Count(); i++ )  {
        if( netA.Node(res[i].GetA()).GetType().GetMr() < 3.5 || netA.Node(res[i].GetA()).GetTag() == 0 )
          continue;
        sg.Add(netA.Node(res[i].GetA()).CAtom());
        netA.Node(res[i].GetA()).SetTag(0);
        TBasicApp::GetLog() << netA.Node(res[i].GetA()).GetLabel() << ' ';
      }
      TBasicApp::NewLogEntry();
      TSameGroup& d_sg = FXApp->XFile().GetRM().rSAME.NewDependent(sg);
      for( size_t i=0; i < res.Count(); i++ )  {
        if( netB.Node(res[i].GetB()).GetType().GetMr() < 3.5 || netB.Node(res[i].GetB()).GetTag() == 0 )
          continue;
        d_sg.Add( netB.Node(res[i].GetB()).CAtom() );
        netB.Node(res[i].GetB()).SetTag(0);
        TBasicApp::GetLog() << netB.Node(res[i].GetB()).GetLabel() << ' ';
      }
      TBasicApp::NewLogEntry();
    }
  }
  else if( groups_count != InvalidSize && (atoms.Count()%groups_count) == 0 )  {
    const size_t cnt = atoms.Count()/groups_count;
    if( expand )  {
      TArrayList<TSAtomPList> groups(groups_count);
      for( size_t i=0; i < cnt; i++ )  {
        for( size_t j=0; j < groups_count; j++ )
          groups[j].Add(atoms[cnt*j+i]);
      }
      macSAME_expand(groups);
    }
    else  {
      TPtrList<TSameGroup> deps;
      TSameGroup& sg = FXApp->XFile().GetRM().rSAME.New();
      for( size_t i=0; i < groups_count-1; i++ )
        deps.Add(FXApp->XFile().GetRM().rSAME.NewDependent(sg));
      for( size_t i=0; i < cnt; i++ )  {
        sg.Add(atoms[i]->CAtom());
        for( size_t j=1; j < groups_count; j++ )
          deps[j-1]->Add(atoms[cnt*j+i]->CAtom());
      }
      TBasicApp::NewLogEntry() << "SAME instruction is added";
    }
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "invalid input arguments");
  }
}
//..............................................................................
void TMainForm::macRESI(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr resi_class = Cmds[0];
  int resi_number = 0;
  if( resi_class.IsNumber() )  {
    resi_number = resi_class.ToInt();
    resi_class.SetLength(0);
  }
  Cmds.Delete(0);
  if( resi_number == 0 && (Cmds.Count() > 0  && Cmds[0].IsNumber()) )  {
    resi_number = olx_abs(Cmds[0].ToInt());
    Cmds.Delete(0);
  }
  TXAtomPList atoms = FindXAtoms(Cmds, false, true);
  if( atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  ACollectionItem::Unique(atoms);
  if( resi_class.Equalsi("none") )  {
    TResidue& main_resi = au.GetResidue(0);
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TCAtom& ca = atoms[i]->CAtom();
      if( olx_is_valid_index(ca.GetResiId()) )
        main_resi.Add(ca);
    }
  }
  else  {
    TResidue& resi = au.NewResidue(resi_class, resi_number,
      Options.FindValue('a', resi_number).ToInt());
    resi.SetCapacity(atoms.Count());
    for( size_t i=0; i < atoms.Count(); i++ )
      resi.Add(atoms[i]->CAtom());
  }
}
//..............................................................................
void TMainForm::funGetMAC(const TStrObjList& Params, TMacroError &E)  {
  bool full = (Params.Count() == 1 && Params[0].Equalsi("full") );
  olxstr rv(EmptyString(), 256);
  char bf[16];
  TShellUtil::MACInfo MACsInfo;
  TShellUtil::ListMACAddresses(MACsInfo);
  for( size_t i=0; i < MACsInfo.Count(); i++ )  {
    if( full )
      rv << MACsInfo[i] << '=';
    for( size_t j=0; j < MACsInfo.GetObject(i).Count(); j++ )  {
      sprintf(bf, "%02X", MACsInfo.GetObject(i)[j]);
      rv << bf;
      if( j < 5 )  rv << '-';
    }
    if( (i+1) < MACsInfo.Count() )  
      rv << ';';
  }
  E.SetRetVal(rv.IsEmpty() ? XLibMacros::NAString : rv);
}
//..............................................................................
void main_CreateWBox(TGXApp& app, const TSAtomPList& atoms, double (*weight_c)(const TSAtom&), 
  const TDoubleList& all_radii, bool print_info)  
{
  const WBoxInfo bs = TXApp::CalcWBox(atoms, &all_radii, weight_c);
  static int obj_cnt = 0;
  if( print_info )  {
    app.NewLogEntry() << "Wrapping box dimension: " << 
      olxstr::FormatFloat(3, bs.r_to[0]-bs.r_from[0]) << " x "  <<
      olxstr::FormatFloat(3, bs.r_to[1]-bs.r_from[1]) << " x "  <<
      olxstr::FormatFloat(3, bs.r_to[2]-bs.r_from[2]) << " A";
    app.NewLogEntry() << "Wrapping box volume: " << 
      olxstr::FormatFloat(3, (bs.r_to-bs.r_from).Prod()) << " A^3";
  }
  const vec3d nx = bs.normals[0]*bs.s_from[0];
  const vec3d px = bs.normals[0]*bs.s_to[0];
  const vec3d ny = bs.normals[1]*bs.s_from[1];
  const vec3d py = bs.normals[1]*bs.s_to[1];
  const vec3d nz = bs.normals[2]*bs.s_from[2];
  const vec3d pz = bs.normals[2]*bs.s_to[2];

  vec3d faces[] = {
    px + py + pz, px + ny + pz, px + ny + nz, px + py + nz, // normals[0]
    nx + py + pz, nx + py + nz, nx + ny + nz, nx + ny + pz, // -normals[0]
    px + py + pz, px + py + nz, nx + py + nz, nx + py + pz, // normals[1]
    px + ny + nz, px + ny + pz, nx + ny + pz, nx + ny + nz, // -normals[1]
    nx + py + pz, nx + ny + pz, px + ny + pz, px + py + pz, // normals[2]
    nx + py + nz, px + py + nz, px + ny + nz, nx + ny + nz // -normals[2]
  };

  TArrayList<vec3f>& poly_d = *(new TArrayList<vec3f>(24));
  TArrayList<vec3f>& poly_n = *(new TArrayList<vec3f>(6));
  for( int i=0; i < 6; i++ )  {
    const vec3d cnt((faces[i*4]+faces[i*4+1]+faces[i*4+2]+faces[i*4+3])/4);
    const vec3f norm = (faces[i*4+1]-faces[i*4]).XProdVec(faces[i*4+3]-faces[i*4]);
    if( norm.DotProd(cnt) < 0 )  {  // does normal look inside?
      poly_n[i] = -norm;
      olx_swap(faces[i*4+1], faces[i*4+3]);
    }
    else
      poly_n[i] = norm;
  }
  for( int i=0; i < 24; i++ )
    poly_d[i] = bs.center + faces[i];
  //TDUserObj* uo = new TDUserObj(app.GetRender(), sgloSphere, olxstr("wbox") << obj_cnt++);
  //TArrayList<vec3f>& sph_m = *(new TArrayList<vec3f>(3));
  //sph_m[0] = px;  sph_m[1] = py;  sph_m[2] = -pz;
  //uo->SetVertices(&sph_m);
  //uo->Basis.Translate(bs.center);
  TDUserObj* uo = new TDUserObj(app.GetRender(), sgloQuads, olxstr("wbox") << obj_cnt++);
  uo->SetVertices(&poly_d);
  uo->SetNormals(&poly_n);
  app.AddObjectToCreate(uo);
  uo->SetMaterial("1029;2566914048;2574743415");
  uo->Create();
  if( print_info )
    app.NewLogEntry() << "Please note that displayed and used atomic radii might be DIFFERENT";
}
void TMainForm::macWBox(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  ElementRadii radii;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadVdWRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
  const bool use_aw = Options.Contains('w');
  if( Options.Contains('s') )  {
    TLattice& latt = FXApp->XFile().GetLattice();
    for( size_t i=0; i < latt.FragmentCount(); i++ )  {
      TSAtomPList satoms;
      TNetwork& f = latt.GetFragment(i);
      for( size_t j=0; j < f.NodeCount(); j++ )  {
        if( f.Node(j).IsDeleted() )  continue;
        satoms.Add(f.Node(j));
      }
      if( satoms.Count() < 3 )  continue;
      TArrayList<double> all_radii(satoms.Count());
      for( size_t j=0; j < satoms.Count(); j++ )  {
        const size_t ri = radii.IndexOf(&satoms[j]->GetType());
        if( ri == InvalidIndex )
          all_radii[j] = satoms[j]->GetType().r_vdw;
        else
          all_radii[j] = radii.GetValue(ri);
      }
      main_CreateWBox(*FXApp, satoms,
        use_aw ? TSAtom::weight_occu_z : TSAtom::weight_occu, all_radii, false);
    }
  }
  else  {
    TXAtomPList xatoms;
    if( !FindXAtoms(Cmds, xatoms, true, true) || xatoms.Count() < 3 )  {
      E.ProcessingError(__OlxSrcInfo, "no enough atoms provided");
      return;
    }
    TArrayList<double> all_radii(xatoms.Count());
    for( size_t i=0; i < xatoms.Count(); i++ )  {
      const size_t ri = radii.IndexOf(&xatoms[i]->GetType());
      if( ri == InvalidIndex )
        all_radii[i] = xatoms[i]->GetType().r_vdw;
      else
        all_radii[i] = radii.GetValue(ri);
    }
    main_CreateWBox(*FXApp, TSAtomPList(xatoms, StaticCastAccessor<TSAtom>()),
      use_aw ? TSAtom::weight_occu_z : TSAtom::weight_occu, all_radii, true);
  }
}
//..............................................................................
void TMainForm::macCenter(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 3 && Cmds[0].IsNumber() && Cmds[1].IsNumber() && Cmds[2].IsNumber() )  {
    FXApp->GetRender().GetBasis().SetCenter(
     -vec3d(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble()));
  }
  else  {
    if( Options.Contains('z') )  {
      FXApp->GetRender().GetBasis().SetZoom(
        FXApp->GetRender().CalcZoom()*FXApp->GetExtraZoom());
    }
    else  {
      TXAtomPList atoms;
      if( !FindXAtoms(Cmds, atoms, true, true) )
        return;
      vec3d center;
      double sum = 0;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        center += atoms[i]->crd()*atoms[i]->CAtom().GetOccu();
        sum += atoms[i]->CAtom().GetOccu();;
      }
      if( sum != 0 )  {
        center /= sum;
        FXApp->GetRender().GetBasis().SetCenter(-center);
      }
    }
  }
}
//..............................................................................
void TMainForm::funThreadCount(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  E.SetRetVal(FXApp->GetMaxThreadCount());
  else  {
    int pthc = Params[0].ToInt();
    int rthc = wxThread::GetCPUCount();
    if( rthc != -1 && pthc > rthc )  {
      E.ProcessingError(__OlxSrcInfo, "Number of proposed threads is larger than number of physical ones");
      return;
    }
    if( pthc > 0 )
      FXApp->SetMaxThreadCount(pthc);
    else if( pthc == -1 )  {
      if( rthc == -1 )  {
        E.ProcessingError(__OlxSrcInfo, "Could not determine the number of CPU");
        return;
      }
      FXApp->SetMaxThreadCount(rthc);
    }
  }
}
//..............................................................................
void TMainForm::macPictS(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int orgHeight = FXApp->GetRender().GetHeight(),
      orgWidth  = FXApp->GetRender().GetWidth();
  double res = 1,
    half_ang = Options.FindValue('a', "6").ToDouble()/2,
    sep_width = Options.FindValue('s', "10").ToDouble();
  if( Cmds.Count() == 2 && Cmds[1].IsNumber() )  
    res = Cmds[1].ToDouble();
  if( res >= 100 )  // width provided
    res /= orgWidth;
  if( res > 10 )
    res = 10;
  if( res <= 0 )  
    res = 1;
  if( res > 1 && res < 100 )
    res = olx_round(res);

  int SrcHeight = (int)(((double)orgHeight/(res*2)-1.0)*res*2),
      SrcWidth  = (int)(((double)orgWidth/(res*2)-1.0)*res*2);
  SrcHeight = Options.FindValue('h', SrcHeight).ToInt();
  int BmpHeight = (int)(SrcHeight*res);
  if( (BmpHeight%2) != 0 )  BmpHeight++;
  int BmpWidth = (int)(SrcWidth*res),
      ImgWidth = olx_round((double)BmpWidth*(2 + sep_width/100));  // sep_width% in between pictures
  const int imgOff = ImgWidth - BmpWidth;
  if( BmpHeight < SrcHeight )
    SrcHeight = BmpHeight;
  if( BmpWidth < SrcWidth )
    SrcWidth = BmpWidth;
  FXApp->GetRender().Resize(0, 0, SrcWidth, SrcHeight, res); 

  const int bmpSize = BmpHeight*ImgWidth*3;
  char* bmpData = (char*)malloc(bmpSize);
  memset(bmpData, ~0, bmpSize);
  FGlConsole->SetVisible(false);
  FXApp->GetRender().OnDraw.SetEnabled(false);
  if( res != 1 )    {
    FXApp->GetRender().GetScene().ScaleFonts(res);
    if( res >= 3 )
      FXApp->Quality(qaPict);
  }
  const double RY = FXApp->GetRender().GetBasis().GetRY();
  FXApp->GetRender().GetBasis().RotateY(RY-half_ang);
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, (int)(res < 1 ? 1 : res));
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      const int mj = j*SrcWidth;
      const int mi = i*SrcHeight;
      for( int k=0; k < SrcWidth; k++ )  {
        for( int l=0; l < SrcHeight; l++ )  {
          const int indexA = (l*SrcWidth + k)*3;
          const int indexB = bmpSize - (ImgWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  // second half
  FXApp->GetRender().GetBasis().RotateY(RY+half_ang);
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, (int)(res < 1 ? 1 : res));
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      const int mj = j*SrcWidth;
      const int mi = i*SrcHeight;
      for( int k=0; k < SrcWidth; k++ )  {
        for( int l=0; l < SrcHeight; l++ )  {
          const int indexA = (l*SrcWidth + k)*3;
          const int indexB = bmpSize - (ImgWidth*(mi + l + 1) - mj - k - imgOff)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  FXApp->GetRender().GetBasis().RotateY(RY);
  if( res != 1 ) {
    FXApp->GetRender().GetScene().RestoreFontScale();
    if( res >= 3 ) 
      FXApp->Quality(qaMedium);
  }

  FXApp->GetRender().OnDraw.SetEnabled(true);
  FGlConsole->SetVisible(true);
  // end drawing etc
  FXApp->GetRender().Resize(orgWidth, orgHeight); 
  FXApp->GetRender().LookAt(0,0,1);
  FXApp->GetRender().SetView(false, 1);
  FXApp->Draw();
  olxstr bmpFN;
  if( FXApp->XFile().HasLastLoader() && !TEFile::IsAbsolutePath(Cmds[0]) )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName(Cmds[0]);
  else
    bmpFN = Cmds[0];
  wxImage image;
  image.SetData((unsigned char*)bmpData, ImgWidth, BmpHeight); 
  // correct a common typo
  if( TEFile::ExtractFileExt(bmpFN).Equalsi("jpeg") )
    bmpFN = TEFile::ChangeFileExt(bmpFN, "jpg");
  image.SaveFile(bmpFN.u_str());
}
//..............................................................................
void TMainForm::funFullScreen(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  E.SetRetVal(IsFullScreen());
  else  {
    if( Params[0].Equalsi("swap") )
      ShowFullScreen(!IsFullScreen());
    else
      ShowFullScreen(Params[0].ToBool());
  }
}
//..............................................................................
void TMainForm::funFreeze(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(FXApp->IsDisplayFrozen());
  else  {
    E.SetRetVal(FXApp->IsDisplayFrozen());
    FXApp->SetDisplayFrozen(Params[0].ToBool());
  }
}
//..............................................................................
olxstr TMainForm_funMatchNets(TNetwork& netA, TNetwork& netB, bool invert, bool verbose,
  double (*w_c)(const TSAtom&))
{
  TTypeList<AnAssociation2<size_t, size_t> > res;
  TSizeList sk;
  bool match = false;
  try  {  match = netA.DoMatch(netB, res, invert, w_c);  }
  catch(...)  {
    return "exception";
  }
  if( match )  {
    olxstr rv;
    TTypeList<AnAssociation2<TSAtom*,TSAtom*> > satomp;
    for( size_t i=0; i < res.Count(); i++ )
      satomp.AddNew<TSAtom*,TSAtom*>(&netA.Node(res[i].GetA()), &netB.Node(res[i].GetB()));
    TNetwork::AlignInfo align_info = MatchAtomPairsQT(satomp, invert, w_c, false);
    rv = olxstr::FormatFloat(3, align_info.rmsd.GetV());
    mat3d m;
    QuaternionToMatrix(align_info.align_out.quaternions[0], m);
    TPSTypeList<double, AnAssociation2<TSAtom*,TSAtom*>*> pairs;
    pairs.SetCapacity(satomp.Count());
    const TAsymmUnit& au2 = satomp[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
    for( size_t i=0; i < satomp.Count(); i++ )  {
      vec3d v = satomp[i].GetB()->ccrd();
      if( invert )  v *= -1;
      au2.CellToCartesian(v);
      const double d = (satomp[i].GetA()->crd()-align_info.align_out.center_a).DistanceTo(
        (v-align_info.align_out.center_b)*m);
      pairs.Add(d, &satomp[i]);
    }
    rv << ",{";
    if( verbose )  {
      for( size_t i=0; i < pairs.Count(); i++ )  {
        size_t index = pairs.Count() - i -1;
        rv << olxstr::FormatFloat(3, pairs.GetKey(index)) << "(";
        rv << pairs.GetObject(index)->GetA()->GetLabel() << ',' <<
          pairs.GetObject(index)->GetB()->GetLabel() << ')';
        if( i+1 < pairs.Count() )
          rv << ',';
      }
    }
    else  {
      rv << olxstr::FormatFloat(3, pairs.GetLastKey()) << "(";
      rv << pairs.GetLastObject()->GetA()->GetLabel() << ',' <<
        pairs.GetLastObject()->GetB()->GetLabel() << ')';
    }
    return rv << '}';
  }
  return XLibMacros::NAString;;
}
TStrList TMainForm_funMatchNets1(TNetwork& netA, TNetwork& netB, bool verbose)  {
  TStrList rv;
  if( !netA.IsSuitableForMatching() || !netB.IsSuitableForMatching() )  return rv;
  const olxstr r = TMainForm_funMatchNets(netA, netB, false, verbose, TSAtom::weight_unit);
  if( r == XLibMacros::NAString )  return rv;
  rv.Add("Fragment content: ") << netA.GetFormula();
  rv.Add("inverted:false,weight:unit=") << r; 
  rv.Add("inverted:false,weight:Z=") << TMainForm_funMatchNets(netA, netB, false, verbose, TSAtom::weight_z);
  rv.Add("inverted:true,weight:unit=") << TMainForm_funMatchNets(netA, netB, true, verbose, TSAtom::weight_unit); 
  rv.Add("inverted:true,weight:Z=") << TMainForm_funMatchNets(netA, netB, true, verbose, TSAtom::weight_z); 
  return rv;
}
TStrList TMainForm_funMatchLatts(TLattice& lattA, TLattice& lattB, const TStrObjList& Params)  {
  TStrList rv;
  const bool verbose = (Params.Count() == 3 && Params[2].Equalsi("verbose"));
  if( lattA.FragmentCount() > 1 )  {
    rv.Add("Comparing '") << TEFile::ExtractFileName(Params[0]) << "' to itself";
    for( size_t i=0; i < lattA.FragmentCount(); i++ )  {
      if( !lattA.GetFragment(i).IsSuitableForMatching() )  continue;
      for( size_t j=i+1; j < lattA.FragmentCount(); j++ )  {
        if( !lattA.GetFragment(j).IsSuitableForMatching() )  continue;
        rv.AddList(TMainForm_funMatchNets1(lattA.GetFragment(i), lattA.GetFragment(j), verbose));
      }
    }
  }
  if( lattB.FragmentCount() > 1 )  {
    rv.Add("Comparing '") << TEFile::ExtractFileName(Params[1]) << "' to itself";
    for( size_t i=0; i < lattB.FragmentCount(); i++ )  {
      if( !lattB.GetFragment(i).IsSuitableForMatching() )  continue;
      for( size_t j=i+1; j < lattB.FragmentCount(); j++ )  {
        if( !lattB.GetFragment(j).IsSuitableForMatching() )  continue;
        rv.AddList(TMainForm_funMatchNets1(lattB.GetFragment(i), lattB.GetFragment(j), verbose));
      }
    }
  }
  rv.Add("Comparing the two files");
  for( size_t i=0; i < lattA.FragmentCount(); i++ )  {
    if( !lattA.GetFragment(i).IsSuitableForMatching() )  continue;
    for( size_t j=0; j < lattB.FragmentCount(); j++ )  {
      if( !lattB.GetFragment(j).IsSuitableForMatching() )  continue;
      rv.AddList(TMainForm_funMatchNets1(lattA.GetFragment(i), lattB.GetFragment(j), verbose));
    }
  }
  return rv;
}
//..............................................................................
void TMainForm::funMatchFiles(const TStrObjList& Params, TMacroError &E)  {
  double (*weight_calculator)(const TSAtom&) = &TSAtom::weight_occu;
  try  {
    TXFile* f1 = (TXFile*)FXApp->XFile().Replicate();
    TXFile* f2 = (TXFile*)FXApp->XFile().Replicate();
    try  {
      f1->LoadFromFile(Params[0]);
      f2->LoadFromFile(Params[1]);
      TStrList rv;
      TLattice& lattA = f1->GetLattice();
      TLattice& lattB = f2->GetLattice();
      rv.Add("!H atoms included");
      rv.AddList(TMainForm_funMatchLatts(lattA, lattB, Params));
      rv.Add("!H atoms excluded");
      lattA.GetAsymmUnit().DetachAtomType(iHydrogenZ, true);
      lattA.UpdateConnectivity();
      lattB.GetAsymmUnit().DetachAtomType(iHydrogenZ, true);
      lattB.UpdateConnectivity();
      rv.AddList(TMainForm_funMatchLatts(lattA, lattB, Params));
      delete f1;
      delete f2;
      E.SetRetVal(rv.Text(NewLineSequence()));
      return;
    }
    catch(...)  {
      delete f1;
      delete f2;
    }
  }
  catch(...)  {}
  E.SetRetVal(XLibMacros::NAString);
}
//..............................................................................
void TMainForm::macPiM(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TTypeList<TSAtomPList> rings;
  TTypeList<TSAtomPList> ring_M;
  bool check_metal = true;
  ConstPtrList<TXAtom> atoms = FindXAtoms(Cmds, false, true);
  if( atoms.IsEmpty() )  {
    FXApp->FindRings("C4", rings);
    FXApp->FindRings("C5", rings);
    FXApp->FindRings("C6", rings);
    for( size_t i=0; i < rings.Count(); i++ )  {
      if( TSPlane::CalcRMSD(rings[i]) > 0.05 ||
          !TNetwork::IsRingRegular(rings[i]) )
      {
        rings.NullItem(i);
      }
    }
    rings.Pack();
  }
  else  {
    rings.Add(new TSAtomPList(atoms, StaticCastAccessor<TSAtom>()));
    TSAtomPList &metals = ring_M.AddNew();
    for( size_t i=0; i < rings[0].Count(); i++ )  {
      if( XElementLib::IsMetal(rings[0][i]->GetType()) )  {
        check_metal = false;
        metals.Add(rings[0][i]);
        rings[0][i] = NULL;
      }
    }
    rings[0].Pack();
    if( rings[0].IsEmpty() )  return;
    if( check_metal )
      ring_M.Delete(0);
  }
  if( check_metal )  {
    ring_M.SetCapacity(rings.Count());
    for( size_t i=0; i < rings.Count(); i++ )  {
      TSAtomPList &metals = ring_M.AddNew();
      for( size_t j=0; j < rings[i].Count(); j++ )  {
        for( size_t k=0; k < rings[i][j]->NodeCount(); k++ )  {
          if( XElementLib::IsMetal(rings[i][j]->Node(k).GetType()) )  {
            metals.Add(rings[i][j]->Node(k));
          }
        }
      }
      if( metals.IsEmpty() )  {
        rings.NullItem(i);
        ring_M.NullItem(ring_M.Count()-1);
      }
    }
    rings.Pack();
    ring_M.Pack();
  }
  // process rings...  
  if( rings.IsEmpty() )  return;
  const bool label = Options.Contains('l');
  TGXApp::AtomIterator ai = FXApp->GetAtoms();
  while( ai.HasNext() )
    ai.Next().SetTag(0);
  for( size_t i=0; i < rings.Count(); i++ )  {
    vec3d c;
    for( size_t j=0; j < rings[i].Count(); j++ )  {
      c += rings[i][j]->crd();
      rings[i][j]->SetTag(1);
    }
    c /= rings[i].Count();
    ACollectionItem::Unique(ring_M[i]);
    for( size_t j=0; j < ring_M[i].Count(); j++ )  {
      TXLine &l = FXApp->AddLine(ring_M[i][j]->GetLabel()+olxstr(i),
        ring_M[i][j]->crd(), c);
      TGraphicsStyle &ms = ((TXAtom*)ring_M[i][j])->GetPrimitives().GetStyle();
      TGlMaterial *sm = ms.FindMaterial("Sphere");
      if( sm != NULL )
        l.GetPrimitives().GetStyle().SetMaterial(TXBond::StaticPrimitives()[9], *sm);
      l.UpdatePrimitives((1<<9)|(1<<10));
      l.SetRadius(0.5);
      ring_M[i][j]->SetTag(2);
      if( !label )
        l.GetGlLabel().SetVisible(false);
    }
    // hide replaced bonds
  }
  TGXApp::BondIterator bi = FXApp->GetBonds();
  while( bi.HasNext() )  {
    TXBond &b = bi.Next();
    if( b.A().GetTag() == 2 && b.B().GetTag() == 1 )
      b.SetVisible(false);
  }
}
//..............................................................................
void TMainForm::macFlushFS(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( Cmds.IsEmpty() || Cmds[0].Equalsi("global") )  {
    SaveVFS(plGlobal);
  }
  else if( Cmds[0].Equalsi("structure") )  {
    if( !FXApp->XFile().HasLastLoader() )  {
      E.ProcessingError(__OlxSrcInfo, "a loaded file is required");
      return;
    }
    SaveVFS(plStructure);
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "unknown option: ").quote() << Cmds[0];
  }
}
//..............................................................................
void TMainForm::macChemDraw(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  FXApp->CreateRings(true, true);
}
//..............................................................................
void TMainForm::macRestrain(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  RefinementModel &rm = FXApp->XFile().GetRM();
  if( Cmds[0].Equalsi("ADP") && Cmds.Count() > 1 )  {
    olxstr target = Cmds[1];
    Cmds.DeleteRange(0, 2);
    double value = -1;
    if( Cmds.Count() > 0 && Cmds[0].IsNumber() )  {
      value = Cmds[0].ToDouble();
      Cmds.Delete(0);
    }
    TXAtomPList atoms = FindXAtoms(Cmds, false, true);
    TSimpleRestraint *r = NULL;
    if( target.Equalsi("Ueq") )  {
      if( atoms.Count() < 2 && value < 0 )  {
        E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
        return;
      }
      if( value > 0 )  {
        r = &rm.rFixedUeq.AddNew();
        r->SetValue(value);
      }
      else
        r = &rm.rSimilarUeq.AddNew();
    }
    else if( target.Equalsi("volume") )  {
      if( atoms.Count() < 2 )  {
        E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
        return;
      }
      r = &rm.rSimilarAdpVolume.AddNew();
    }
    if( r != NULL )  {
      for( size_t i=0; i < atoms.Count(); i++ )
        r->AddAtom(atoms[i]->CAtom(), NULL);
    }
  }
  else if( Cmds[0].Equalsi("bond") )  {
    Cmds.Delete(0);
    double val = -1, esd=0.02;
    TSimpleRestraint *r = NULL;
    size_t set_cnt = XLibMacros::Parse(Cmds, "dd", &val, &esd);
    TXAtomPList atoms = FindXAtoms(Cmds, false, false);
    if (atoms.IsEmpty()) {
      TXBondPList bonds = FXApp->GetSelection().Extract<TXBond>();
      FXApp->GetRender().SelectAll(false);
      for (size_t i=0; i < bonds.Count(); i++)
        atoms << bonds[i]->A() << bonds[i]->B();
    }
    if( (atoms.Count()%2) != 0 )  {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected");
      return;
    }
    if( (set_cnt == 1 && val >= 0.5) || set_cnt == 2)  {
      r = &rm.rDFIX.AddNew();
      r->SetValue(val);
      if (set_cnt == 2) r->SetEsd(esd);
    }
    else {
      r = &rm.rSADI.AddNew();
      if (set_cnt == 1) r->SetEsd(val);
    }
    for (size_t i=0; i < atoms.Count(); i++)
      r->AddAtom(atoms[i]->CAtom(), &atoms[i]->GetMatrix());
  }
  else if( Cmds[0].Equalsi("angle") && Cmds.Count() > 1 )  {
    double val = Cmds[1].ToDouble();
    Cmds.DeleteRange(0, 2);
    TXAtomPList atoms = FindXAtoms(Cmds, false, false);
    if (atoms.IsEmpty()) {
      TXBondPList bonds = FXApp->GetSelection().Extract<TXBond>();
      FXApp->GetRender().SelectAll(false);
      if ((bonds.Count()%2)!=0) {
        E.ProcessingError(__OlxSrcInfo, "even number of bonds is expected");
        return;
      }
      for (size_t i=0; i < bonds.Count(); i+=2) {
        TSAtom *s = bonds[i]->GetShared(*bonds[i+1]);
        if (s == NULL)  {
          TBasicApp::NewLogEntry(logError) << "No shared atom for: " <<
            bonds[i]->A().GetLabel() << '-' << bonds[i]->B().GetLabel() <<
            " and " <<
            bonds[i+1]->A().GetLabel() << '-' << bonds[i+1]->B().GetLabel() <<
            " skiping...";
        }
        else {
          atoms << bonds[i]->Another(*s) << (TXAtom*)s <<
            bonds[i+1]->Another(*s);
        }
      }
    }
    if ((atoms.Count()%3)!=0) {
      E.ProcessingError(__OlxSrcInfo, "triplets of atoms are expected");
      return;
    }
    if (!atoms.IsEmpty()) {
      TSimpleRestraint &sr = rm.rAngle.AddNew();
      sr.SetValue(val);
      for (size_t i=0; i < atoms.Count(); i++)
        sr.AddAtom(atoms[i]->CAtom(), &atoms[i]->GetMatrix());
    }
  }
  else if( Cmds[0].Equalsi("dihedral") && Cmds.Count() > 1 )  {
    double val = Cmds[1].ToDouble();
    Cmds.DeleteRange(0, 2);
    TXAtomPList atoms = FindXAtoms(Cmds, false, false);
    TTypeList<TSAtomPList> quadruplets;
    if (atoms.IsEmpty()) {
      TXBondPList bonds = FXApp->GetSelection().Extract<TXBond>();
      FXApp->GetRender().SelectAll(false);
      if ((bonds.Count()%2)!=0) {
        E.ProcessingError(__OlxSrcInfo, "even number of bonds is expected");
        return;
      }
      for (size_t i=0; i < bonds.Count(); i+=2) {
        ConstPtrList<TSAtom> dh = bonds[i]->GetDihedral(*bonds[i+1]);
        if (dh.IsEmpty())  {
          TBasicApp::NewLogEntry(logError) << "Do not form dihedral: " <<
            bonds[i]->A().GetLabel() << '-' << bonds[i]->B().GetLabel() <<
            " and " <<
            bonds[i+1]->A().GetLabel() << '-' << bonds[i+1]->B().GetLabel() <<
            " skiping...";
        }
        else
          quadruplets.AddNew(dh);
      }
    }
    else {
      if ((atoms.Count()%4)!=0) {
        E.ProcessingError(__OlxSrcInfo, "quadruplets of atoms are expected");
        return;
      }
      for (size_t i=0; i < atoms.Count(); i+=4) {
        TSAtomPList &l = quadruplets.AddNew(4);
        for( int j=0; j < 4; j++)
          l.Set(j, atoms[i+j]);
      }
    }
    if (!quadruplets.IsEmpty()) {
      TSimpleRestraint &sr = rm.rDihedralAngle.AddNew();
      sr.SetValue(val);
      for (size_t i=0; i < quadruplets.Count(); i++) {
        for (size_t j=0; j < quadruplets[i].Count(); j++) {
          sr.AddAtom(quadruplets[i][j]->CAtom(),
            &quadruplets[i][j]->GetMatrix());
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macConstrain(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  RefinementModel &rm = FXApp->XFile().GetRM();
  if (Cmds[0].Equalsi("U") || Cmds[0].Equalsi("ADP")) { // EADP
    //Cmds.Delete(0);
    //TXAtomPList atoms = FindXAtoms(Cmds, false, true);
    //if( atoms.Count() < 2 )  {
    //  E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
    //  return;
    //}
    //ACollectionItem::Unique(atoms,
    //  FunctionAccessor::MakeConst(&TXAtom::CAtom));
    //// search optimisation
    //SortedPtrList<TCAtom, TPointerComparator> s_catoms;
    //for( size_t i=0; i < atoms.Count(); i++ )
    //  s_catoms.AddUnique(&atoms[i]->CAtom());
    //TSimpleRestraint *e_sr = NULL;
    //for( size_t i=0; i < rm.rEADP.Count(); i++ )  {
    //  TSimpleRestraint &sr = rm.rEADP[i];
    //  for( size_t j=0; j < sr.AtomCount(); j++ )  {
    //    if( s_catoms.IndexOf(sr.GetAtom(j).GetAtom()) != InvalidIndex )  {
    //      e_sr = &sr;
    //      break;
    //    }
    //    if( e_sr != NULL )  break;
    //  }
    //  if( e_sr != NULL )  break;
    //}
    //if( e_sr == NULL )
    //  e_sr = &rm.rEADP.AddNew();
    //for( size_t i=0; i < atoms.Count(); i++ )
    //  e_sr->AddAtom(atoms[i]->CAtom(), NULL);
  }
  else if( Cmds[0].Equalsi("site") || Cmds[0].Equalsi("xyz"))  { // EXYZ
    Cmds.Delete(0);
    TXAtomPList atoms = FindXAtoms(Cmds, false, true);
    if( atoms.Count() < 2 )  {
      E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
      return;
    }
    ACollectionItem::Unique(atoms,
      FunctionAccessor::MakeConst(&TXAtom::CAtom));
    // search optimisation
    SortedPtrList<TCAtom, TPointerComparator> s_catoms;
    for( size_t i=0; i < atoms.Count(); i++ )
      s_catoms.AddUnique(&atoms[i]->CAtom());
    TExyzGroup *e_g = NULL;
    for( size_t i=0; i < rm.rEADP.Count(); i++ )  {
      TExyzGroup &sr = rm.ExyzGroups[i];
      for( size_t j=0; j < sr.Count(); j++ )  {
        if( s_catoms.IndexOf(&sr[j]) != InvalidIndex )  {
          e_g = &sr;
          break;
        }
        if( e_g != NULL )  break;
      }
      if( e_g != NULL )  break;
    }
    if( e_g == NULL )
      e_g = &rm.ExyzGroups.New();
    for( size_t i=0; i < atoms.Count(); i++ )
      e_g->Add(atoms[i]->CAtom());
  }
  else if( Cmds[0].Equalsi("same") && // same group constraint
          (Cmds.Count() > 1 && Cmds[1].Equalsi("group") ) )
  {
    size_t n=InvalidSize;
    if( Cmds.Count() > 2 && Cmds[2].IsNumber() )  {
      n = Cmds[2].ToSizeT();
      Cmds.DeleteRange(0, 3);
    }
    else
      Cmds.DeleteRange(0, 2);
    TXAtomPList atoms = FindXAtoms(Cmds, false, true);
    if( n != InvalidSize && atoms.Count() > n*3)  {
      if( (atoms.Count()%n) != 0 )  {
        E.ProcessingError(__OlxSrcInfo,
          "number of atoms do not match the number given of groups");
        return;
      }
      const size_t ag = atoms.Count()/n;
      TTypeList<TCAtomPList> groups(n);
      for( size_t i=0, ac=0; i < n; i++ )  {
        TCAtomPList &al = groups.Set(i, new TCAtomPList(ag));
        for( size_t j=0; j < ag; j++, ac++ )
          al[j] = &atoms[ac]->CAtom();
      }
      FXApp->XFile().GetRM().SameGroups.items.AddNew(
        ConstTypeList<TCAtomPList>(groups));
    }
    else if( atoms.Count() == 2 )  {
      TTypeList<AnAssociation2<size_t, size_t> > res;
      TNetwork &netA = atoms[0]->GetNetwork(),
        &netB = atoms[1]->GetNetwork();
      if( &netA == &netB )  {
        E.ProcessingError(__OlxSrcInfo,
          "please select atoms from different fragments");
        return;
      }
      if( !netA.DoMatch(netB, res, false, &TSAtom::weight_unit) )  {
        E.ProcessingError(__OlxSrcInfo, "fragments do not match");
        return;
      }
      TTypeList<AnAssociation2<TSAtom*,TSAtom*> > matoms(res.Count());
      for( size_t i=0; i < res.Count(); i++ )  {
        matoms.Set(i, new AnAssociation2<TSAtom*,TSAtom*>(
          &netA.Node(res[i].GetA()), &netB.Node(res[i].GetB())));
      }
      TNetwork::AlignInfo rv =
        TNetwork::GetAlignmentRMSD(matoms, false, &TSAtom::weight_unit);

      TTypeList<AnAssociation2<TSAtom*,TSAtom*> > imatoms(res.Count());
      res.Clear();
      netA.DoMatch(netB, res, true, &TSAtom::weight_unit);
      for( size_t i=0; i < res.Count(); i++ )  {
        imatoms.Set(i, new AnAssociation2<TSAtom*,TSAtom*>(
          &netA.Node(res[i].GetA()), &netB.Node(res[i].GetB())));
      }
      TNetwork::AlignInfo irv =
        TNetwork::GetAlignmentRMSD(imatoms, true, &TSAtom::weight_unit);

      TTypeList<AnAssociation2<TSAtom*,TSAtom*> > &ma =
        (rv.rmsd.GetV() < irv.rmsd.GetV() ? matoms : imatoms);
      
      TTypeList<TCAtomPList> groups;
      groups.AddNew(ma.Count());
      groups.AddNew(ma.Count());
      for( size_t i=0; i < ma.Count(); i++ )  {
        groups[0][i] = &ma[i].GetA()->CAtom();
        groups[1][i] = &ma[i].GetB()->CAtom();
      }
      FXApp->XFile().GetRM().SameGroups.items.AddNew(
        ConstTypeList<TCAtomPList>(groups));
    }
  }
  else if( Cmds[0].Equalsi("same") && // same group constraint
          (Cmds.Count() > 1 && Cmds[1].Equalsi("adp") ) )
  {
    Cmds.DeleteRange(0, 2);
    ABasicFunction *bf = GetLibrary().FindMacro("xf.rm.ShareADP");
    if (bf == NULL) {
      E.ProcessingError(__OlxSrcInfo, "could not locate required function");
      return;
    }
    else
      bf->Run(Cmds, Options, E);
  }
}
//..............................................................................
void TMainForm::macTolman(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TXAtomPList atoms = FindXAtoms(Cmds, false, true);
  if (atoms.Count() != 5) {
    E.ProcessingError(__OlxSrcInfo, "5 Atoms are expected - M P S1 S2 S3");
    return;
  }
  TBasicApp::NewLogEntry() << "Calculatinng Tolman cone angle for the "
    "selection: http://en.wikipedia.org/wiki/Tolman_cone_angle";
  double d = Options.FindValue("mpd", atoms[0]->crd().DistanceTo(
    atoms[1]->crd())).ToDouble();
  TBasicApp::NewLogEntry() << "Metal to P distance is: " <<
    olxstr::FormatFloat(3, d);
  vec3d v0 = (atoms[1]->crd() - atoms[0]->crd()).NormaliseTo(d);
  olxdict<const cm_Element*, double, TPointerComparator> radii;
  double sa = 0;
  for (size_t i=2; i < 5; i++) {
    vec3d v = atoms[i]->crd()-atoms[0]->crd();
    radii.Add(&atoms[i]->GetType()) = atoms[i]->GetType().r_vdw;
    double a = acos(v.CAngle(v0));
    double qvl = v.QLength();
    a += acos(sqrt((qvl-olx_sqr(atoms[i]->GetType().r_vdw))/qvl));
    sa += a;
  }
  TBasicApp::NewLogEntry() << "Used radii:";
  for (size_t i=0; i < radii.Count(); i++) {
    TBasicApp::NewLogEntry() << radii.GetKey(i)->symbol << '\t' <<
      radii.GetValue(i);
  }
  
  TBasicApp::NewLogEntry() << "Resulting angle: " << olxstr::FormatFloat(3, 180*(2*sa/3)/M_PI);
}
//..............................................................................
void TMainForm::macPoly(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short pt = 0;
  const olxstr &str_t = Cmds.GetLastString();
  if (str_t.Equalsi("none"))
    pt = 0;
  else if (str_t.Equalsi("auto"))
    pt = polyAuto;
  else if (str_t.Equalsi("regular"))
    pt = polyRegular;
  else if (str_t.Equalsi("pyramid"))
    pt = polyPyramid;
  else if (str_t.Equalsi("bipyramid"))
    pt = polyBipyramid;
  else {
    E.ProcessingError(__OlxSrcInfo, "Undefined poly type: ").quote() << str_t;
    return;
  }
  Cmds.Delete(Cmds.Count()-1);
  TXAtomPList atoms = FindXAtoms(Cmds, true, true);
  atoms.ForEach(ACollectionItem::IndexTagSetter(
    FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
  atoms.Pack(ACollectionItem::IndexTagAnalyser(
    FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
  for (size_t i=0; i < atoms.Count(); i++)
    atoms[i]->SetPolyhedronType(pt);
}
//..............................................................................
void TMainForm::macUpdate(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if (patcher::PatchAPI::HaveUpdates()) {
    TBasicApp::NewLogEntry() <<
      "Updates already available, please restart the program to apply";
    return;
  }
  CreateUpdateThread();
}
  //..............................................................................
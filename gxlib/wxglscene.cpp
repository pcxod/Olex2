//---------------------------------------------------------------------------

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "bapp.h"

// for InterlockedIncrement in
#if defined __WIN32__ && __BORLANDC__
  #include <windows.h>
  #include <winbase.h>
#endif

#include "wxglscene.h"
#include "exception.h"
#include "glfont.h"
#include "glrender.h"
#include "efile.h"
#include "bitarray.h"

#include "wx/zipstrm.h"
#include "wx/wfstream.h"

//#include "ft2build.h"  
//#include FT_FREETYPE_H
//#include FT_GLYPH_H
//---------------------------------------------------------------------------
// AGlScene
//---------------------------------------------------------------------------
#if defined(__WXWIDGETS__)

TwxGlScene::TwxGlScene()  { ; }
//..............................................................................
TwxGlScene::~TwxGlScene()  {
  Destroy();
}
//..............................................................................
//TGlFont* TwxGlScene::CreateFont(const olxstr& name, void *Data, TGlFont *ReplaceFnt, bool BmpF, bool FixedW)  {
//  TGlFont *Fnt;
//  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
//  
//  try  {  Font = *static_cast<wxFont*>(Data);  }
//  catch(...)  {
//    throw TInvalidArgumentException(__OlxSourceInfo, "invalid data type");
//  }
////////////////////////////////////////////////////////////////
//  FT_Library library;
//  FT_Face face;
//  FT_Error error = FT_Init_FreeType( &library );
//  if( error )
//    throw TFunctionFailedException(__OlxSourceInfo, "could not load the FreeType2 librray");
//  error = FT_New_Face(library, "C:/WINDOWS/Fonts/arial.ttf", 0, &face);
//  if( error )
//    throw TFunctionFailedException(__OlxSourceInfo, "could not load font face");
//  const int fs = 18;
//  error = FT_Set_Pixel_Sizes(face, 0, fs);
/////////////////////////////////////////////////////////////////
//  if( ReplaceFnt )  {
//    Fnt = ReplaceFnt;
//    Fnt->ClearData();
//  }
//  else
//    Fnt = new TGlFont(name);
//
//  const int maxHeight = face->size->metrics.height/64;
//  unsigned char* dt[256];
////  FT_BBox bbox;
//  Fnt->IdString(Font.GetNativeFontInfoDesc().c_str());
//  for( int i=0; i < 256; i++ )  {
//    FT_UInt glyph_index = FT_Get_Char_Index( face, i );
//    error = FT_Load_Glyph(face, glyph_index, 0);
//    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
//    FT_Bitmap& bmp = face->glyph->bitmap;
//    dt[i] = new unsigned char [maxHeight*maxHeight*3];
//    memset(dt[i], 0, maxHeight*maxHeight*3);
//    int dy = (maxHeight - face->glyph->bitmap_top-1);
//    for( int j=0; j < bmp.width; j++ )  {
//      for( int k=0; k < bmp.rows; k++ )  {
//        int ind = (k*bmp.pitch+j/8);
//        int bit = (1 << (7-j%8));
//        if( (bmp.buffer[ind]&bit) != 0 )  {
//          int ind1 = ((k+dy)*maxHeight + j+face->glyph->bitmap_left)*3;
//          if( ind1 >= maxHeight*maxHeight*3 )
//            continue;
//          dt[i][ind1] = 1;
//        }
//      }
//    }
//    Fnt->CharFromRGBArray(i, dt[i], maxHeight, maxHeight);
//  }
//  //Fnt->CreateTextures(ImageW, ImageW);
//  Fnt->CreateGlyphs(false, maxHeight, maxHeight);
//  for( int i=0; i < 256; i++ )
//    delete dt[i];
//  if( ReplaceFnt == NULL )
//    Fonts.Add(Fnt);
//  return Fnt;
//}
//..............................................................................
TGlFont* TwxGlScene::CreateFont(const olxstr& name, void *Data, TGlFont *ReplaceFnt, bool BmpF, bool FixedW)  {
  TGlFont *Fnt;
  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  
  try  {  Font = *static_cast<wxFont*>(Data);  }
  catch(...)  {
    throw TInvalidArgumentException(__OlxSourceInfo, "invalid data type");
  }
  if( ReplaceFnt )  {
    Fnt = ReplaceFnt;
    Fnt->ClearData();
  }
  else
    Fnt = new TGlFont(name);

  // LINUZ port - ... native font string is system dependent...
  if( Font.GetPointSize() <= 1 )
    Font.SetPointSize(6);
    
  Fnt->IdString(Font.GetNativeFontInfoDesc().c_str());

  TPtrList<wxImage> Images;
  wxImage *Image;
  int ImageW = Font.GetPointSize()*2;
  wxMemoryDC memDC;
  memDC.SetFont(Font);
#if defined(__WXGTK__)
  wxBitmap Bmp(ImageW, ImageW);
#elif defined(__MAC__)
  wxBitmap Bmp(ImageW, ImageW, 8);
//  Font.SetPointSize( Font.GetPointSize() );
#else
  wxBitmap Bmp(ImageW, ImageW, 1);
#endif
  memDC.SelectObject(Bmp);
  memDC.SetPen(*wxBLACK_PEN);
  memDC.SetBackground(*wxWHITE_BRUSH);
  memDC.SetBackgroundMode(wxSOLID);
  //memDC.SetTextBackground(*wxWHITE);
  memDC.SetTextForeground(*wxBLACK);
  for( int i=0; i < 256; i++ )  {
    memDC.SelectObject(Bmp);
    memDC.Clear();
    memDC.DrawText(wxString((olxch)i), 0, 0);
    memDC.SelectObject(wxNullBitmap);
    Image = new wxImage( Bmp.ConvertToImage() );
#ifdef __MAC__
    //Image->Rescale(ImageW, ImageW);
#endif    
    Fnt->CharFromRGBArray(i, Image->GetData(), ImageW, ImageW);
    Images.Add(Image);
  }
  //Fnt->CreateTextures(ImageW, ImageW);
  Fnt->CreateGlyphs(FixedW, ImageW, ImageW);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete Images[i];
  
  if( ReplaceFnt == NULL )
    Fonts.Add(Fnt);
  return Fnt;
}
//..............................................................................
void TwxGlScene::StartDraw()  {  AGlScene::StartDraw();  }
//..............................................................................
void TwxGlScene::EndDraw()  {  AGlScene::EndDraw();  }
//..............................................................................
void TwxGlScene::StartSelect(int x, int y, GLuint *Bf)  {  AGlScene::StartSelect(x, y, Bf);  }
//..............................................................................
void TwxGlScene::EndSelect()  {  AGlScene::EndSelect();  }
//..............................................................................
void TwxGlScene::Destroy()  {  AGlScene::Destroy();  }
//..............................................................................
void TwxGlScene_RipFont(wxFont& fnt, TGlFont& glf, TEBitArray& ba)  {
  TPtrList<wxImage> Images;
  glf.ClearData();
  int ImageW = fnt.GetPointSize()*2;
  wxMemoryDC memDC;
  memDC.SetFont(fnt);
  wxBitmap Bmp(ImageW, ImageW);
  memDC.SelectObject(Bmp);
  memDC.SetPen(*wxBLACK_PEN);
  memDC.SetBackground(*wxWHITE_BRUSH);
  memDC.SetBackgroundMode(wxSOLID);
  memDC.SetTextForeground(*wxBLACK);
  for( int i=0; i < 256; i++ )  {
    memDC.SelectObject(Bmp);
    memDC.Clear();
    memDC.DrawText(wxString((olxch)i), 0, 0);
    memDC.SelectObject(wxNullBitmap);
    wxImage* Image = new wxImage( Bmp.ConvertToImage() );
    glf.CharFromRGBArray(i, Image->GetData(), ImageW, ImageW);
    Images.Add(Image);
  }
  ba.Clear();
  ba.SetSize( 256*glf.MaxWidth()*glf.MaxHeight() );
  int bit_cnt=0;
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = glf.CharSize(i);
    if( cs->Data == NULL )  {
      bit_cnt += glf.MaxWidth()*glf.MaxHeight();
      continue;
    }
    for( int j=0; j < glf.MaxWidth(); j++ )  {
      for( int k=0; k < glf.MaxHeight(); k++ )  {
        int ind = (k*ImageW+j)*3;
        if( (cs->Data[ind] | cs->Data[ind+1] | cs->Data[ind+2]) != cs->Background ) // is black?
          ba.SetTrue(bit_cnt);
        bit_cnt++;
      }
    }
    delete Images[i];
  }
}
//..............................................................................
void TwxGlScene_RipFontA(wxFont& fnt, TGlFont& glf, wxZipOutputStream& zos)  {
  TEBitArray ba;
  olxstr prefix( fnt.IsFixedWidth() ? "f" : "n" );
  if( fnt.GetStyle() == wxFONTSTYLE_ITALIC )  {
    prefix << "i";
    if( fnt.GetWeight() == wxFONTWEIGHT_BOLD )
      prefix << "b";
  }
  else if( fnt.GetWeight() == wxFONTWEIGHT_BOLD )
    prefix << "rb";
  else
    prefix << "r";

  for( int i=10; i <= 36; i+=1 )  {
    fnt.SetPointSize(i);
    TwxGlScene_RipFont(fnt, glf, ba);
    zos.PutNextEntry((olxstr(prefix) << i).u_str());
    uint16_t s = glf.MaxWidth(); 
    zos.Write(&s, sizeof(uint16_t));
    s = glf.MaxHeight(); 
    zos.Write(&s, sizeof(uint16_t));
    zos.Write(ba.GetData(), ba.CharCount());
    zos.CloseEntry();
  }
  fnt.SetPointSize(400);
  TwxGlScene_RipFont(fnt, glf, ba);
  zos.PutNextEntry((olxstr(prefix) << "400").u_str());
  uint16_t s = glf.MaxWidth(); 
  zos.Write(&s, sizeof(uint16_t));
  s = glf.MaxHeight(); 
  zos.Write(&s, sizeof(uint16_t));
  zos.Write(ba.GetData(), ba.CharCount());
  zos.CloseEntry();
}
void TwxGlScene::ExportFont(const olxstr& name, const olxstr& fileName)  {
  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  TStrList toks(name, '&');
  TGlFont Fnt(name);
  wxFileOutputStream fos( fileName.u_str() );
  fos.Write("ofnt", 4);
  wxZipOutputStream zos(fos, 9);
  for( int i=0; i < toks.Count(); i++ )  {
    Font.SetNativeFontInfo( toks[i].u_str() );
    // regular
    TwxGlScene_RipFontA(Font, Fnt, zos);
    // bold
    Font.SetWeight(wxFONTWEIGHT_BOLD);
    TwxGlScene_RipFontA(Font, Fnt, zos);
    // bold italic
    Font.SetStyle(wxFONTSTYLE_ITALIC);
    TwxGlScene_RipFontA(Font, Fnt, zos);
    // italic
    Font.SetWeight(wxFONTWEIGHT_NORMAL);
    TwxGlScene_RipFontA(Font, Fnt, zos);
  }
  zos.Close();
  fos.Close();
}
//..............................................................................
TGlFont* TwxGlScene::ImportFont(const olxstr& Name, const olxstr& fileName, short Size, bool FixedWidth, TGlFont *ReplaceFnt)  {
  TGlFont* Fnt = NULL;
  wxFileInputStream fis( fileName.u_str() );
  char sig[4];
  fis.Read(sig, 4);
  if( olxstr::o_memcmp(sig, "ofnt", 4) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");
  olxstr entryName("fib16");
  wxZipInputStream zin(fis);
  uint16_t maxw, maxh;
  wxZipEntry* zen = NULL;
  while( (zen = zin.GetNextEntry()) != NULL )  {
    if( entryName == zen->GetName().c_str() )
      break;
  }
  if( zen == NULL || (entryName != zen->GetName().c_str()) )  {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid font description");
  }
  zin.Read(&maxw, sizeof(uint16_t));
  zin.Read(&maxh, sizeof(uint16_t));
  int contentLen = zin.GetLength() - 2*sizeof(uint16_t);
  char * bf1 = new char[ contentLen + 1];
  zin.Read(bf1, contentLen);
  TEBitArray ba(bf1, contentLen );
  delete [] bf1;
  if( ReplaceFnt != NULL )  {
    Fnt = ReplaceFnt;
    Fnt->ClearData();
  }
  else
    Fnt = new TGlFont(Name);
  Fnt->IdString( olxstr("#olx;") << fileName << ';' << Size);
  TPtrList<wxImage> Images;
  int bit_cnt = 0;
  for( int i=0; i < 256; i++ )  {
    wxImage* img = new wxImage(maxw, maxh);
    unsigned char* idata = img->GetData();
    memset(idata, 0, maxw*maxh);
    for( int j=0; j < maxw; j++ )  {
      for( int k=0; k < maxh; k++ )  {
        if( ba.Get(bit_cnt) )
          idata[(k*maxw+j)*3] = 255;
        bit_cnt++;
      }
    }
//    img->Rescale(imgW, imgH, wxIMAGE_QUALITY_HIGH);
    Fnt->CharFromRGBArray(i, img->GetData(), maxw, maxh);
    Images.Add(img);
  }
  Fnt->CreateGlyphs(FixedWidth, maxw, maxh);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete Images[i];
  if( ReplaceFnt == NULL )
    Fonts.Add(Fnt);
  return Fnt;
}
//..............................................................................
#endif // __WXWIDGETS__


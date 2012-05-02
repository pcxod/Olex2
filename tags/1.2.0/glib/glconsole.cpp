/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glconsole.h"
#include "glrender.h"
#include "actions.h"
#include "glcursor.h"
#include "gpcollection.h"
#include "glfont.h"
#include "glmouse.h"
#include "glprimitive.h"
#include "styles.h"
#include "integration.h"
#include "bapp.h"
// keyboard constanst, silly to include here, but...

// stolen from wxWidgets
enum {
    OLX_KEY_BACK    =    8,
    OLX_KEY_TAB     =    9,
    OLX_KEY_RETURN  =    13,
    OLX_KEY_ESCAPE  =    27,
    OLX_KEY_SPACE   =    32,
    OLX_KEY_DELETE  =    127,
    OLX_KEY_END = 312,
    OLX_KEY_HOME = 313,
    OLX_KEY_LEFT = 314,
    OLX_KEY_UP = 315,
    OLX_KEY_RIGHT = 316,
    OLX_KEY_DOWN = 317,
    OLX_KEY_PAGEUP = 366,
    OLX_KEY_PAGEDOWN = 367
};

/* There is a slight problem with cursor - depending on object properties it
might be drawn before the console, and then its position is validated after it
is drawn in previous position!  ...
*/

TGlConsole::TGlConsole(TGlRenderer& R, const olxstr& collectionName) :
  AGDrawObject(R, collectionName),
  LinesVisible(~0),
  FontIndex(~0),
  OnCommand(Actions.New("ONCOMMAND")),
  OnPost(Actions.New("ONPOST"))
{
  PrintMaterial = NULL;
  FLineSpacing = 0;
  Left = Top = 0;
  Width = Height = 100;
  PromptVisible = true;
  SkipPosting = false;
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FShowBuffer = true;
  SetSelectable(false);
  FTxtPos = ~0;
  FMaxLines = 1000;
  ScrollDirectionUp = true;
  FLinesToShow = ~0;
  FCmdPos = ~0;
  FCursor = new TGlCursor(R, "Cursor");
  SetToDelete(false);
  R.OnDraw.Add(this);
  // init size related valriables
  OnResize();
}
//..............................................................................
TGlConsole::~TGlConsole()  {
  Parent.OnDraw.Remove(this);
  ClearBuffer();
  delete FCursor;
}
//..............................................................................
void TGlConsole::Create(const olxstr& cName)  {
  FontIndex = Parent.GetScene().FindFontIndexForType<TGlConsole>(FontIndex);
  if( !cName.IsEmpty() )
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  {
    FCursor->Create();
    return;
  }
  TGraphicsStyle& GS = GPC.GetStyle();
  FLinesToShow = GS.GetParam("LinesToShow", FLinesToShow, true).ToInt();
  FLineSpacing = GS.GetParam("LineSpacing", "0", true).ToDouble();
  InviteStr = GS.GetParam("Prompt", ">>", true);
  TGlPrimitive& GlP = GPC.NewPrimitive("Text", sgloText);
  GlP.SetProperties(GS.GetMaterial("Text", GetFont().GetMaterial()));
  GlP.Params[0] = -1;  //bitmap; TTF by default
  FCursor->Create();
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FStringPos = FCommand.Length();
}
//..............................................................................
size_t TGlConsole::CalcScrollDown() const {
  TGlFont& Fnt = GetFont();
  const uint16_t th = Fnt.TextHeight(EmptyString());
  const double Scale = Parent.GetScale(),
               MaxY = ((double)Parent.GetHeight()/2-Top-th)*Scale,
               LineSpacer = (0.05+FLineSpacing)*th;
  const double empty_line_height = th*0.75*(1+FLineSpacing)*Scale;
  vec3d T(GlLeft*Scale, GlTop*Scale, 0);
  if( FTxtPos < FBuffer.Count() && FBuffer[FTxtPos].IsEmpty() )
    T[1] -= 0.5*th*Scale;
  TGlOption CC = Parent.LightModel.GetClearColor();
  size_t lines = 0;
  if( ShowBuffer() && FLinesToShow != 0 )  {
    for( size_t i=FTxtPos; i < FBuffer.Count(); i++ )  {
      if( FBuffer[i].IsEmpty() )  {
        T[1] += empty_line_height;
        lines++;
        continue;
      }
      if( olx_is_valid_size(FLinesToShow) && lines >= FLinesToShow )  break;
      if( T[1] > MaxY )  break;
      olxstr line = FBuffer[i].SubStringTo(Fnt.LengthForWidth(FBuffer[i],
        Parent.GetWidth()));
      // drawing spaces is not required ...
      size_t stlen = line.Length();
      while( line.CharAt(stlen-1) == ' ' && stlen > 1 ) stlen--;
      line = line.SubStringTo(stlen);
      if( stlen == 0 )  {
        T[1] += empty_line_height;
        lines++;
        continue;
      }
      const TTextRect tr = Fnt.GetTextRect(line);
      if( tr.top < 0 )
        T[1] += tr.top*Scale;
      if( i== 0 )  break;
      T[1] += (olx_max(tr.height, Fnt.GetMaxHeight())+LineSpacer)*Scale;
      lines++;
    }
  }
  return lines;
}
//..............................................................................
double TGlConsole::GetZ() const {
  return -Parent.CalcRasterZ(0.002);
}
//..............................................................................
bool TGlConsole::Orient(TGlPrimitive& P)  {
  TGlFont& Fnt = GetFont();
  //Fnt->DrawGlText( vec3d(0,0,0), "HELLW_O", true);
  P.SetFont(&Fnt);
  if( Parent.GetWidth() < 100 )  return true;
  const uint16_t th = Fnt.TextHeight(EmptyString());
  const double Scale = Parent.GetScale(),
               MaxY = ((double)Parent.GetHeight()/2-Top-th)*Scale,
               LineSpacer = (0.05+FLineSpacing)*th;
  const double Z = GetZ();
  const double empty_line_height = th*0.75*(1+FLineSpacing)*Scale;
  if( FTxtPos < FBuffer.Count() )  {
    vec3d T(GlLeft*Scale, GlTop*Scale, Z);
    if( FBuffer[FTxtPos].IsEmpty() )
      T[1] -= 0.5*th*Scale;
    LinesVisible = 0;
    const TGlMaterial& OGlM = P.GetProperties();
    TGlOption CC = Parent.LightModel.GetClearColor();
    if( ShowBuffer() && FLinesToShow != 0 )  {
      for( index_t i=FTxtPos; i >= 0; i-- )  {
        if( FBuffer[i].IsEmpty() )  {
          T[1] += empty_line_height;
          LinesVisible++;
          continue;
        }
        if( olx_is_valid_size(FLinesToShow) && LinesVisible >= FLinesToShow )  break;
        if( T[1] > MaxY )  break;
        olxstr line = FBuffer[i].SubStringTo(Fnt.LengthForWidth(FBuffer[i],
          Parent.GetWidth()));
        // drawing spaces is not required ...
        size_t stlen = line.Length();
        while( line.CharAt(stlen-1) == ' ' && stlen > 1 ) stlen--;
        line = line.SubStringTo(stlen);
        if( stlen == 0 )  {
          T[1] += empty_line_height;
          LinesVisible++;
          continue;
        }
        TGlMaterial* GlMP = FBuffer.GetObject(i);
        if( GlMP != NULL ) 
          GlMP->Init(Parent.IsColorStereo());
        else
          OGlM.Init(Parent.IsColorStereo());
        P.SetString(&line);
        const TTextRect tr = Fnt.GetTextRect(line);
        if( tr.top < 0 )
          T[1] -= tr.top*Scale;
        Parent.DrawText(P, T[0], T[1], Z);
        P.SetString(NULL);
        if( i == 0 )  break;
        T[1] += (olx_max(tr.height, Fnt.GetMaxHeight())+LineSpacer)*Scale;
        LinesVisible++;
      }
    }
    OGlM.Init(Parent.IsColorStereo()); // restore the material properties
  }
  if( PromptVisible && !FCommand.IsEmpty() )  {
    Fnt.Reset_ATI(Parent.IsATI());
    const double LineInc = (th*(1+FLineSpacing))*Parent.GetViewZoom();
    short cstate = 0, lstate = 0;
    double line_cnt = 1;
    olxstr tmp = FCommand;
    while( true )  {
      const size_t ml = Fnt.LengthForWidth(tmp, Parent.GetWidth(), lstate);
      olx_gl::rasterPos(GlLeft*Scale, (GlTop - line_cnt*LineInc)*Scale, Z);
      Fnt.DrawRasterText(tmp.SubStringTo(ml), cstate);
      if( tmp.Length() == ml )
        break;
      tmp = tmp.SubStringFrom(ml);
      line_cnt++;
    }
  }
  return true;
}
//..............................................................................
bool TGlConsole::WillProcessKey( int Key, short ShiftState )  {
  if( Key == OLX_KEY_DELETE )  {
    if( ShiftState == 0 && GetInsertPosition() < FCommand.Length() &&
         GetInsertPosition() >= PromptStr.Length() )  {
      return true;
    }
  }
  else if( Key == OLX_KEY_BACK && GetInsertPosition() != PromptStr.Length() )
    return true;
  return false;
}
//..............................................................................
bool TGlConsole::ProcessKey( int Key , short ShiftState)  {
  if( (Key == OLX_KEY_UP) && IsPromptVisible() && ShiftState == 0 )  {
    FCmdPos --;
    if( !olx_is_valid_size(FCmdPos) )  FCmdPos = FCommands.Count()-1;
    if( olx_is_valid_size(FCmdPos) && FCmdPos < FCommands.Count() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
      FCommand << FCommands[FCmdPos];
      SetInsertPosition(FCommand.Length());
    }
    return true;
  }
  if( (Key == OLX_KEY_DOWN) && IsPromptVisible() && ShiftState == 0 )  {
    FCmdPos ++;
    if( FCmdPos >= FCommands.Count() )  FCmdPos = 0;
    if( olx_is_valid_size(FCmdPos) && FCmdPos < FCommands.Count() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
      FCommand << FCommands[FCmdPos];
      SetInsertPosition(FCommand.Length());
    }
    return true;
  }
  if( (Key == OLX_KEY_LEFT) && IsPromptVisible() && (GetInsertPosition() > PromptStr.Length()) )  {
    if( ShiftState == 0 )  {
      SetInsertPosition(GetInsertPosition()-1);
      return true;
    }
    else if( ShiftState == sssCtrl  )  {
      size_t ind = FCommand.LastIndexOf(' ', GetInsertPosition()-1);
      if( ind != InvalidIndex )
        SetInsertPosition( ind );
      else
        SetInsertPosition(PromptStr.Length());
      return true;
    }
    return false;
  }
  if( (Key == OLX_KEY_RIGHT) && IsPromptVisible() && (GetInsertPosition() < FCommand.Length()) )  {
    if( ShiftState == 0 )  {
      SetInsertPosition(GetInsertPosition()+1);
      return true;
    }
    else if( ShiftState == sssCtrl )  {
      size_t ind = FCommand.FirstIndexOf(' ', GetInsertPosition()+1);
      if( ind != InvalidIndex )
        SetInsertPosition( ind );
      else
        SetInsertPosition(FCommand.Length());
      return true;
    }
    return false;
  }
  if( Key == OLX_KEY_DELETE )  {
    if( ShiftState == 0 && GetInsertPosition() < FCommand.Length() &&
         GetInsertPosition() >= PromptStr.Length() )  {
      FCommand.Delete(GetInsertPosition(), 1);
      UpdateCursorPosition(true);
      return true;
    }
    return false;
  }
  if( (Key == OLX_KEY_HOME) && !ShiftState  && IsPromptVisible() )  {
    SetInsertPosition(PromptStr.Length());
    return true;
  }
  if( (Key == OLX_KEY_END) && !ShiftState  && IsPromptVisible() )  {
    SetInsertPosition(FCommand.Length());
    return true;
  }
  if( (Key == OLX_KEY_PAGEUP || Key == OLX_KEY_PAGEDOWN) &&
      olx_is_valid_index(LinesVisible) )
  {
    if( Key == OLX_KEY_PAGEDOWN )  {
      const size_t lines = CalcScrollDown();
      FTxtPos += lines-1;
      if( FTxtPos >= FBuffer.Count() )
        FTxtPos = FBuffer.Count()-1;
    }
    else if( Key == OLX_KEY_PAGEUP )  {
      if( FTxtPos < LinesVisible-1 )
        FTxtPos = FBuffer.IsEmpty() ? ~0 : 0;
      else if( olx_is_valid_index(FTxtPos) )
        FTxtPos -= (LinesVisible-1);
    }
    SetInsertPosition(FCommand.Length());
    return true;
  }
  if( !Key || Key > 255 || (ShiftState & sssCtrl) || (ShiftState & sssAlt))  return false;
  if( !IsPromptVisible() )  return false;
  
  if( Key == OLX_KEY_ESCAPE )  {
    olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
    FCommand = PromptStr;
    SetInsertPosition(FCommand.Length());
    return true;
  }
  if( Key == OLX_KEY_RETURN )  {
    FCommand = GetCommand();
    if( FCommand.Length() )  {
      if( FCommands.Count() != 0 && FCommands[FCommands.Count()-1] == FCommand)  {
        ;
      }
      else  {
        FCommands.Add(FCommand);
      }
      FCmdPos = FCommands.Count();
    }
    OnCommand.Execute(dynamic_cast<IEObject*>((AActionHandler*)this) );
    if( FCommand.IsEmpty() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
    }
    SetInsertPosition(FCommand.Length());
    return true;
  }
  if( Key == OLX_KEY_BACK )  {
    if( FCommand.Length() > PromptStr.Length() )  {
      if( GetInsertPosition() == FCommand.Length() )  {
        FCommand.SetLength(FCommand.Length()-1);
        SetInsertPosition(FCommand.Length());
      }
      else  {  // works like delete
        if( GetInsertPosition() > PromptStr.Length() )  {
          FCommand.Delete(GetInsertPosition()-1, 1);
          SetInsertPosition(GetInsertPosition()-1);
        }
      }
    }
    return true;
  }
  if( GetInsertPosition() == FCommand.Length() )
    FCommand << (olxch)Key;
  else
    FCommand.Insert((olxch)Key, GetInsertPosition());
  SetInsertPosition(GetInsertPosition()+1);
  return true;
}
//..............................................................................
void TGlConsole::PrintText(const olxstr &S, TGlMaterial *M, bool Hyphenate)  {
  if( IsSkipPosting() )  {
    //SetSkipPosting(false);
    return;
  }
  bool SingleLine = false;
  if( Hyphenate || S.IndexOf('\n') != InvalidIndex )  {
    const size_t sz = GetFont().MaxTextLength(Parent.GetWidth());
    if( sz <= 0 )  return;
    TStrList Txt(S, '\n'), toks;
    for( size_t i=0; i < Txt.Count(); i++ )  {
      toks.Hyphenate(Txt[i], sz, true);
      if( toks.Count() > 1 )  {
        Txt[i] = toks[0];
        for( size_t j=1; j < toks.Count(); j++ )
          Txt.Insert(++i, toks[j]);
      }
      toks.Clear();
    }
    if( Txt.Count() > 1 )
      PrintText(Txt, M, false);
    else
      SingleLine = true;
  }
  if( !Hyphenate || SingleLine )  {
    TGlMaterial *GlM = NULL;
    if( M != NULL )  GlM = new TGlMaterial(*M);
    if( !FBuffer.IsEmpty() && FBuffer.GetLastString().IsEmpty() )  {
      FBuffer.GetLastString() = S;
      /* this line is added after memory leak analysis by Compuware DevPartner 8.2 trial */
      if( FBuffer.GetLast().Object != NULL )
        delete FBuffer.GetLast().Object;
      FBuffer.GetLast().Object = GlM;
    }
    else
      FBuffer.Add(S, GlM);
    OnPost.Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &S);
  }
  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  //FBuffer.Add(EmptyString());
}
//..............................................................................
void TGlConsole::PrintText(const TStrList &SL, TGlMaterial *M, bool Hyphenate)  {
  if( IsSkipPosting() )  {
    //SetSkipPosting(false);
    return;
  }
  const size_t sz = GetFont().MaxTextLength(Parent.GetWidth());
  if( sz <= 0 )  return;
  TStrList Txt;
  for( size_t i=0; i < SL.Count(); i++ )  {
    if( Hyphenate )  {
      TStrList Txt;
      Txt.Hyphenate(SL[i], sz, true);
      for( size_t j=0; j < Txt.Count(); j++ )  {
        TGlMaterial *GlM = NULL;
        if( M != NULL )  GlM = new TGlMaterial(*M);
        if( j == 0 && !FBuffer.IsEmpty() && FBuffer.GetLastString().IsEmpty() )  {
          FBuffer.GetLastString() = Txt[j];
          FBuffer.GetLast().Object = GlM;
        }
        else
          FBuffer.Add(Txt[j], GlM);
        OnPost.Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &Txt[j] );
      }
    }
    else  {
      TGlMaterial *GlM = NULL;
      if( M != NULL )  GlM = new TGlMaterial(*M);
      if( !FBuffer.IsEmpty() && FBuffer.GetLastString().IsEmpty() )  {
        FBuffer.GetLastString() = SL[i];
        FBuffer.GetLast().Object = GlM;
      }
      else
        FBuffer.Add(SL[i], GlM);
      OnPost.Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &SL[i]);
    }
  }
  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  FBuffer.Add(EmptyString());
}
//..............................................................................
olxstr TGlConsole::GetCommand() const  {
  return (FCommand.StartsFrom(PromptStr) ? FCommand.SubStringFrom(PromptStr.Length()) : FCommand);
}
//..............................................................................
void TGlConsole::SetCommand(const olxstr& NewCmd)  {
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FCommand << NewCmd;
  SetInsertPosition( FCommand.Length() );
}
//..............................................................................
void TGlConsole::ClearBuffer()  {
  size_t lc = FBuffer.Count();
  for( size_t i=0; i < lc; i++ )
    if( FBuffer.GetObject(i) != NULL )
      delete (TGlMaterial*)FBuffer.GetObject(i);
  FBuffer.Clear();
  //FCommand = FInviteString;
  FTxtPos = ~0;
}
//..............................................................................
void TGlConsole::KeepSize()  {
  const size_t lc = FBuffer.Count();
  if( lc > FMaxLines )  {
    for( size_t i = 0; i < lc-FMaxLines; i++ )
      if( FBuffer.GetObject(i) )
        delete FBuffer.GetObject(i);
    FBuffer.DeleteRange(0, lc-FMaxLines);
  }
}
//..............................................................................
void TGlConsole::UpdateCursorPosition(bool InitCmds)  {
  if( !IsPromptVisible() || Parent.GetWidth()*Parent.GetHeight() <= 50*50 )
    return;
  TGlFont& Fnt = GetFont();
  GlLeft = ((double)Left - (double)Parent.GetWidth()/2) + 0.1;
  GlTop = ((double)Parent.GetHeight()/2 - (Height+Top)) + 0.1;
  const double th = Fnt.TextHeight(EmptyString());
  const double LineInc = (th*(1+FLineSpacing))*Parent.GetViewZoom();
  const double Scale = Parent.GetScale();
  // update cursor position ...
  if( IsPromptVisible() )   {
    vec3d T;
    if( FCommand.IndexOf("\\-") != InvalidIndex )  // got subscript?
      GlTop += th/4;
    T[1] = GlTop;
    size_t printed_cnt = 0, line_cnt = 0, cursor_line=0;;
    short state = 0, cursor_state = 0;
    olxstr tmp = FCommand;
    bool init_x = true;
    while( true )  {
      short _state = state;
      const size_t ml = Fnt.LengthForWidth(tmp, Parent.GetWidth(), state);
      printed_cnt += ml;
      if( init_x && printed_cnt > GetInsertPosition() )  {
        T[0] = (double)Fnt.TextWidth(
          tmp.SubStringTo(ml-(printed_cnt-GetInsertPosition())), _state);
        cursor_state = _state;
        init_x = false;
        cursor_line = line_cnt;
      }
      if( tmp.Length() == ml )  {
        if( init_x )  {
          T[0] = (double)Fnt.TextWidth(tmp, _state);
          init_x = false;
          cursor_line = line_cnt;
          cursor_state = state;
        }
        break;
      }
      tmp = tmp.SubStringFrom(ml);
      line_cnt++;
    }
    T[0] += GlLeft;
    T[0] -= Fnt.GetCharHalfWidth(cursor_state);  // move the cursor half a char left
    T[1] += LineInc*(line_cnt-cursor_line);
    T *= Scale;
    FCursor->SetPosition(T[0], T[1], GetZ());
    GlTop += th*(line_cnt+1);
  }
}
//..............................................................................
void TGlConsole::SetInsertPosition(size_t v)  {
  FStringPos = v;
  UpdateCursorPosition(true);
}
//..............................................................................
TGlFont &TGlConsole::GetFont() const {
  return Parent.GetScene().GetFont(FontIndex, true);
}
//..............................................................................
bool TGlConsole::Enter(const IEObject *Sender, const IEObject *Data)  {
  if( IsVisible() )
    UpdateCursorPosition(false);
  return true;
}
//..............................................................................
void TGlConsole::SetVisible(bool v)  {
  AGDrawObject::SetVisible(v);
  if( !v )
    FCursor->SetVisible(false);
  else
    FCursor->SetVisible(PromptVisible);
}
//..............................................................................
void TGlConsole::SetPromptVisible(bool v)  {
  PromptVisible = v;
  FCursor->SetVisible(v);
}
//..............................................................................
void TGlConsole::SetInviteString(const olxstr &S)  {
  InviteStr = S;
  GetPrimitives().GetStyle().SetParam("Prompt", InviteStr.Replace("\\(", '('), true);
  olxstr cmd = GetCommand();
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FCommand << cmd;
  SetInsertPosition(FCommand.Length());
}
//..............................................................................
void TGlConsole::SetLinesToShow(size_t V)  {
  FLinesToShow = V;
  GetPrimitives().GetStyle().SetParam("LinesToShow", FLinesToShow, true);
}
//..............................................................................
void TGlConsole::SetLineSpacing(double v)  {
  FLineSpacing = olx_max(-0.9, v);
  GetPrimitives().GetStyle().SetParam("LineSpacing", FLineSpacing, true);
}
//..............................................................................
size_t TGlConsole::Write(const void *Data, size_t size)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
size_t TGlConsole::Write(const TTIString<olxch>& str)  {
  if( IsSkipPosting() )  {
    SetSkipPosting(false);
    return 1;
  }
  if( FBuffer.IsEmpty() )
    FBuffer.Add(EmptyString(), PrintMaterial == NULL ? NULL : new TGlMaterial(*PrintMaterial));
  else  {
    if( FBuffer.GetLast().Object == NULL )
      FBuffer.GetLast().Object = (PrintMaterial == NULL ? NULL : new TGlMaterial(*PrintMaterial));
    else if( FBuffer.GetLastString().IsEmpty() )  {  // reset for empty lines
      delete FBuffer.GetLast().Object;
      FBuffer.GetLast().Object = (PrintMaterial == NULL ? NULL : new TGlMaterial(*PrintMaterial));
    }
  }
  if( &str == &NewLineSequence() )  {
    if( !FBuffer.GetLastString().IsEmpty() )
      FBuffer.Add(EmptyString(), PrintMaterial == NULL ? NULL : new TGlMaterial(*PrintMaterial));
    return 1;  
  }
  FBuffer.GetLastString().SetCapacity(FBuffer.GetLastString().Length() + str.Length());
  for( size_t i=0; i < str.Length(); i++ )  {
    if( str.CharAt(i) == '\n' )
      FBuffer.Add(EmptyString(), PrintMaterial == NULL ? NULL : new TGlMaterial(*PrintMaterial));
    else if( str.CharAt(i) == '\r' )  {
      if( i+1 < str.Length() && str.CharAt(i+1) != '\n' &&!FBuffer.IsEmpty() )
        FBuffer.GetLastString()=EmptyString();
    }
    else
      FBuffer.GetLastString() << str.CharAt(i);
  }
  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  PrintMaterial = NULL;
  return 1;
}
//..............................................................................
IOutputStream& TGlConsole::operator << (IInputStream &is)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
bool TGlConsole::GetDimensions(vec3d &Max, vec3d &Min)  {
  Max = vec3d(0.5, 0.5, 0);
  Min = vec3d(-0.5, -0.5, 0);
  return true;
}
//..............................................................................
void TGlConsole::OnResize() {
  GlLeft = ((double)Left - (double)Parent.GetWidth()/2) + 0.1;
  GlTop = ((double)Parent.GetHeight()/2 - (Height+Top)) + 0.1;
}
//..............................................................................
void TGlConsole::Resize(int l, int t, int w, int h) {
  Left = (uint16_t)l;
  Top = (uint16_t)t;
  Width = (uint16_t)w;
  Height = (uint16_t)h;
  OnResize();
}
//..............................................................................
//..............................................................................
//..............................................................................
void TGlConsole::LibClear(const TStrObjList& Params, TMacroError& E)  {
  ClearBuffer();
}
//..............................................................................
void TGlConsole::LibLines(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    SetLinesToShow(Params[0].ToInt());
  else
    E.SetRetVal<olxstr>(FLinesToShow);
}
//..............................................................................
void TGlConsole::LibShowBuffer(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    ShowBuffer( Params[0].ToBool() );
  else
    E.SetRetVal<olxstr>(FShowBuffer);
}
//..............................................................................
void TGlConsole::LibPostText(const TStrObjList& Params, TMacroError& E)  {
  for( size_t i=0; i < Params.Count(); i++ )
    PrintText(Params[i]);
}
//..............................................................................
void TGlConsole::LibLineSpacing(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    SetLineSpacing(Params[0].ToDouble());
  else
    E.SetRetVal<olxstr>(FLineSpacing);
}
//..............................................................................
void TGlConsole::LibInviteString(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    SetInviteString(Params[0]);
  else
    E.SetRetVal(InviteStr);
}
//..............................................................................
void TGlConsole::LibCommand(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )  {
    olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
    FCommand = PromptStr;
    FCommand << Params[0];
    SetInsertPosition(FCommand.Length());
  }
  else
    E.SetRetVal(FCommand);
}
//..............................................................................
TLibrary* TGlConsole::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( (name.IsEmpty() ? olxstr("console") : name) );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibClear, "Clear",
    fpNone, "Clears the content of the output buffer") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibLines, "Lines",
    fpNone|fpOne, "Sets/returns the number of lines to display") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibShowBuffer, "ShowBuffer",
    fpNone|fpOne, "Shows/hides the output buffer or returns current status") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibPostText, "Post",
    fpAny^fpNone, "Adds provided text to the output buffer") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibLineSpacing, "LineSpacing",
    fpNone|fpOne, "Changes/returns current line spacing") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibInviteString, "PromptString",
    fpNone|fpOne, "Changes/returns current prompt string") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibCommand, "Command",
    fpNone|fpOne, "Changes/returns current command") );

  AGDrawObject::ExportLibrary(*lib);
  lib->AttachLibrary(FCursor->ExportLibrary());
  return lib;
}

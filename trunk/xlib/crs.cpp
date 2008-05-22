#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "crs.h"
#include "asymmunit.h"

#include "symmlib.h"

TCRSFile::TCRSFile():TBasicCFile(NULL)  {
  Radiation = 0.71;
  SGInitialised = false;
}
//..............................................................................
TCRSFile::~TCRSFile()  {  }
//..............................................................................
TSpaceGroup* TCRSFile::GetSG()  {
  return SGInitialised ? TSymmLib::GetInstance()->FindSG( GetAsymmUnit() ) : NULL;
}
//..............................................................................
void TCRSFile::SaveToStrings(TStrList& SL)  {
  olxstr Tmp;

  SL.Add(olxstr("TITLE   ") << GetTitle() );

  Tmp = "CELL ";
  Tmp << Radiation << ' ' <<
  GetAsymmUnit().Axes()[0].GetV() << ' ' <<
  GetAsymmUnit().Axes()[1].GetV()  << ' ' <<
  GetAsymmUnit().Axes()[2].GetV() << ' ' <<
  GetAsymmUnit().Angles()[0].GetV() << ' ' <<
  GetAsymmUnit().Angles()[1].GetV() << ' ' <<
  GetAsymmUnit().Angles()[2].GetV();
  SL.Add(Tmp);

  Tmp = "ZERR ";
  Tmp << GetAsymmUnit().GetZ() << ' ' <<
  GetAsymmUnit().Axes()[0].GetE() << ' ' <<
  GetAsymmUnit().Axes()[1].GetE() << ' ' <<
  GetAsymmUnit().Axes()[2].GetE() << ' ' <<
  GetAsymmUnit().Angles()[0].GetE() << ' ' <<
  GetAsymmUnit().Angles()[1].GetE() << ' ' <<
  GetAsymmUnit().Angles()[2].GetE();
  SL.Add(Tmp);

  TSpaceGroup* sg = GetSG();
  if( sg != NULL )  {
    Tmp = "LATT ";
    Tmp << sg->GetBravaisLattice().GetName() << ' ' << sg->GetLattice().GetSymbol();
    SL.Add(Tmp);

    Tmp = "SPGR ";
    Tmp << sg->GetName();
    SL.Add(Tmp);
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");

  SL.Add( olxstr("SFAC ") << Sfac );
  SL.Add( olxstr("UNIT ") << Unit );
}
//..............................................................................
void TCRSFile::LoadFromStrings(const TStrList& Strings)  {
  olxstr Tmp, TmpUC, Cell, Zerr, Sg, fcId("FACE");
  TStrList toks;
  TStrPObjList<olxstr, olxstr*> params;
  params.Add("TITL",  &FTitle);
  params.Add("CELL",   &Cell);
  params.Add("ZERR",  &Zerr);
  params.Add("SFAC",    &Sfac);
  params.Add("UNIT",    &Unit);
  params.Add("SPGR",  &Sg);
  for( int i=0; i < Strings.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>( Strings[i], ' ' );
    if( Tmp.IsEmpty() )  continue;
    TmpUC = Tmp.UpperCase();
    for( int j=0; j < params.Count(); j++ )  {
      if( TmpUC.StartsFrom( params.String(j) ) ) {
        *params.Object(j) = Tmp.SubStringFrom( params.String(j).Length() );
        params.Delete(j);
        break;
      }
    }
    // a crystal face
    if( TmpUC.StartsFrom( fcId ) )  {
      toks.Clear();
      toks.Strtok( Tmp.SubStringFrom( fcId.Length() ), ' ');
      if( toks.Count() == 4 )  {
        TVectorD& v = Faces.AddNew(4);
        v[0] = toks.String(0).ToDouble();
        v[1] = toks.String(1).ToDouble();
        v[2] = toks.String(2).ToDouble();
        v[3] = toks.String(3).ToDouble();
      }
    }
  }
  if( Cell.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate CELL");
  toks.Clear();
  toks.Strtok( Cell, ' ');
  if( toks.Count() >= 7 )  {
    Radiation = toks[0].ToDouble();
    GetAsymmUnit().Axes().Value(0).V() = toks[1].ToDouble();
    GetAsymmUnit().Axes().Value(1).V() = toks[2].ToDouble();
    GetAsymmUnit().Axes().Value(2).V() = toks[3].ToDouble();
    GetAsymmUnit().Angles().Value(0).V() = toks[4].ToDouble();
    GetAsymmUnit().Angles().Value(1).V() = toks[5].ToDouble();
    GetAsymmUnit().Angles().Value(2).V() = toks[6].ToDouble();
  }
  toks.Clear();
  toks.Strtok( Zerr, ' ');
  if( toks.Count() >= 7 )  {
    GetAsymmUnit().SetZ( toks[0].ToInt() );
    GetAsymmUnit().Axes().Value(0).E() = toks[1].ToDouble();
    GetAsymmUnit().Axes().Value(1).E() = toks[2].ToDouble();
    GetAsymmUnit().Axes().Value(2).E() = toks[3].ToDouble();
    GetAsymmUnit().Angles().Value(0).E() = toks[4].ToDouble();
    GetAsymmUnit().Angles().Value(1).E() = toks[5].ToDouble();
    GetAsymmUnit().Angles().Value(2).E() = toks[6].ToDouble();
  }

  Sg.DeleteChars(' ');
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup( Sg );
  if( sg != NULL )  {
    GetAsymmUnit().ChangeSpaceGroup( *sg );
    SGInitialised = true;
  }
}
//..............................................................................
bool TCRSFile::Adopt(TXFile* f)  {
  GetAsymmUnit().Angles() = f->GetAsymmUnit().Angles();
  GetAsymmUnit().Axes()   = f->GetAsymmUnit().Axes();

  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( f->GetAsymmUnit() );
  if( sg == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "undefined space group");
  GetAsymmUnit().ChangeSpaceGroup( *sg );
  SGInitialised = true;


  TStrPObjList<olxstr, TBasicAtomInfo*> BasicAtoms;
  Unit = EmptyString;
  Sfac = EmptyString;
  f->GetAsymmUnit().SummFormula(BasicAtoms, Sfac, Unit);

  if( f->GetLastLoader() )  {
    FTitle = f->GetLastLoader()->GetTitle();
    SetHKLSource( f->GetLastLoader()->GetHKLSource() );
  }
  else  {
    FHKLSource = EmptyString;
    FTitle = "?";
  }

  return true;
}
//..............................................................................


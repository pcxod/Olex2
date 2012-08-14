#include "p4p.h"
#include "asymmunit.h"
#include "symmlib.h"

TP4PFile::TP4PFile() {}

TP4PFile::~TP4PFile()  {}

void TP4PFile::SaveToStrings(TStrList& SL)  {
  olxstr Tmp;

  SL.Add("FILEID Created by OLEX2");
  SL.Add(olxstr("TITLE   ") << GetTitle() );
  SL.Add(olxstr("CHEM    ") << GetRM().GetUserContentStr() );

  Tmp = "CELL ";
              Tmp << GetAsymmUnit().Axes()[0].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Axes()[1].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Axes()[2].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[0].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[1].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[2].GetV();
  SL.Add(Tmp);

  Tmp = "CELLSD "; Tmp << GetAsymmUnit().GetZ();
  Tmp << ' ' << GetAsymmUnit().Axes()[0].GetE();
  Tmp << ' ' << GetAsymmUnit().Axes()[1].GetE();
  Tmp << ' ' << GetAsymmUnit().Axes()[2].GetE();
  Tmp << ' ' << GetAsymmUnit().Angles()[0].GetE();
  Tmp << ' ' << GetAsymmUnit().Angles()[1].GetE();
  Tmp << ' ' << GetAsymmUnit().Angles()[2].GetE();
  SL.Add(Tmp);

  SL.Add("MORPH   ") << GetMorph();
  SL.Add("CCOLOR  ") << GetColor();
  SL.Add("CSIZE  ") << GetRM().expl.GetCrystalSize()[0] 
    << ' ' << GetRM().expl.GetCrystalSize()[0]
    << ' ' << GetRM().expl.GetCrystalSize()[1]
    << ' ' << GetRM().expl.GetCrystalSize()[2]
    << ' ';
    if( GetRM().expl.IsTemperatureSet() ) 
      SL.Last().String << GetRM().expl.GetTempValue().ToString();
    else
      SL.Last().String << '0';
    SL.Add("SOURCE  ") << GetRM().expl.GetRadiation();
  // save only if preset
  if( !SGString.IsEmpty() )
    SL.Add("SG  ") << SGString;
}

void TP4PFile::LoadFromStrings(const TStrList& Strings)  {
  olxstr Cell, CellSd, Size, Source, chem;
  TStrPObjList<olxstr,olxstr*> params;
  params.Add("SITEID",  &SiteId);
  params.Add("MORPH",   &Morph);
  params.Add("CCOLOR",  &Color);
  params.Add("CSIZE",   &Size);
  params.Add("CHEM",    &chem);
  params.Add("MOSAIC",  &Mosaic);
  params.Add("SYMM",    &Symm);
  params.Add("BRAVAIS", &Bravais);
  params.Add("SOURCE",  &Source);
  params.Add("CELL",    &Cell);
  params.Add("CELLSD",  &CellSd);
  params.Add("TITLE",   &Title);
  params.Add("SG",      &SGString);
  for( size_t i=0; i < Strings.Count(); i++ )  {
    olxstr Tmp = olxstr::DeleteSequencesOf<char>(Strings[i], ' ');
    if( Tmp.IsEmpty() )  continue;
    olxstr TmpUC = Tmp.UpperCase();
    for( size_t j=0; j < params.Count(); j++ )  {
      if( TmpUC.StartsFrom( params[j] ) ) {
        *params.GetObject(j) = Tmp.SubStringFrom(params[j].Length()).Trim(' ');
        params.Delete(j);
        break;
      }
    }
    // do not read reflections and the background etc
    if( !params.Count() || TmpUC.StartsFrom("REF") )  break;
  }
  if( Cell.Length() == 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate CELL");
  params.Clear();
  params.Strtok( Cell, ' ');
  if( params.Count() >= 6 )  {
    GetAsymmUnit().Axes()[0].V() = params[0].ToDouble();
    GetAsymmUnit().Axes()[1].V() = params[1].ToDouble();
    GetAsymmUnit().Axes()[2].V() = params[2].ToDouble();
    GetAsymmUnit().Angles()[0].V() = params[3].ToDouble();
    GetAsymmUnit().Angles()[1].V() = params[4].ToDouble();
    GetAsymmUnit().Angles()[2].V() = params[5].ToDouble();
    GetAsymmUnit().InitMatrices();
  }
  params.Clear();
  params.Strtok( CellSd, ' ');
  if( params.Count() >= 6 )  {
    GetAsymmUnit().Axes()[0].E() = params[0].ToDouble();
    GetAsymmUnit().Axes()[1].E() = params[1].ToDouble();
    GetAsymmUnit().Axes()[2].E() = params[2].ToDouble();
    GetAsymmUnit().Angles()[0].E() = params[3].ToDouble();
    GetAsymmUnit().Angles()[1].E() = params[4].ToDouble();
    GetAsymmUnit().Angles()[2].E() = params[5].ToDouble();
  }
  params.Clear();
  params.Strtok( Size, ' ');
  if( params.Count() == 5 )  {
    if( !params[4].IsNumber() )  {
      size_t bi = params[4].IndexOf('(');
      if( bi != InvalidIndex && bi > 0 )
        params[4] = params[4].SubStringTo(bi);
    }
    if( params[4].IsNumber() )
      GetRM().expl.SetTemp(params[4]);
    params.Delete(4);
  }
  while( params.Count() > 3 )
    params.Delete(params.Count() - 1);

  if( params.Count() == 3 )  {
    if( !params[0].IsNumber() )  params[0] = '0';
    if( !params[1].IsNumber() )  params[1] = '0';
    if( !params[2].IsNumber() )  params[2] = '0';
    GetRM().expl.SetCrystalSize(params[0].ToDouble(), params[1].ToDouble(), params[2].ToDouble());
  }

  params.Clear();
  params.Strtok( Source, ' ');
  if( params.Count() > 2 )
    GetRM().expl.SetRadiation(params[1].ToDouble());
	try  {  GetRM().SetUserFormula(chem.DeleteChars('_'));  }
	catch(...)  {  }  // just skip...
}

bool TP4PFile::Adopt(TXFile& f)  {
  GetRM().Assign(f.GetRM(), false );
  GetAsymmUnit().Angles() = f.GetAsymmUnit().Angles();
  GetAsymmUnit().Axes() = f.GetAsymmUnit().Axes();
  Title = f.LastLoader()->GetTitle();

  SiteId  = "?";
  Morph   = SiteId;
  Color   = SiteId;
  Mosaic  = SiteId;
  Symm = SiteId;  
  SGString = SiteId;
  Bravais = SiteId;
  try  {
    TSpaceGroup& sg = f.GetLastLoaderSG();
    SGString = sg.GetName();
    Bravais = sg.GetBravaisLattice().GetName();
  }
  catch( ... )  {  }
  return true;
}

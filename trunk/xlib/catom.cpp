//---------------------------------------------------------------------------//
// namespace TXClasses: TCAtom - basic crystalographic atom
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "catom.h"
#include "ellipsoid.h"
#include "asymmunit.h"
#include "exception.h"
#include "estrlist.h"

const short TCAtom::CrdFixedValuesOffset  = 0;
const short TCAtom::OccpFixedValuesOffset = 3;
const short TCAtom::UisoFixedValuesOffset = 4;
//----------------------------------------------------------------------------//
// TCAtom function bodies
//----------------------------------------------------------------------------//
TCAtom::TCAtom(TAsymmUnit *Parent)  {
  Hfix = Afix   = 0;
  Part   = 0;
  Occp   = 1;
  QPeak  = -1;
  SetId(0);
  SetResiId(-1);  // default residue is unnamed one
  SetSameId(-1);
  FParent = Parent;
  FEllipsoid = NULL;
  Uiso = caDefIso;
  LoaderId = -1;
  SharedSiteId = AfixAtomId = FragmentId = -1;
  CanBeGrown = Deleted = false;
  FAttachedAtoms = NULL;
  FAttachedAtomsI = NULL;
  Degeneracy = 1;
  FFixedValues.Resize(10);
  HAttached = false;
  Sortable = true;
}
//..............................................................................
TCAtom::~TCAtom()  {
  if( FAttachedAtoms != NULL )  delete FAttachedAtoms;
  if( FAttachedAtomsI != NULL )  delete FAttachedAtomsI;
}
//..............................................................................
bool TCAtom::SetLabel(const olxstr &L)  {
  if( L.IsEmpty() )
    throw TInvalidArgumentException(__OlxSourceInfo, "empty label");

  olxstr Tmp;
  TAtomsInfo *AI = AtomsInfo();
  TBasicAtomInfo *BAI = NULL;
  if( L.Length() >= 2 )  {
    Tmp = L.SubString(0, 2);
    if( AI->IsElement(Tmp) )
      BAI = AI->FindAtomInfoBySymbol( Tmp );
    else  {
      Tmp = L.SubString(0, 1);
      if( AI->IsElement(Tmp) )
        BAI = AI->FindAtomInfoBySymbol( Tmp );
    }
  }
  else  {
    Tmp = L.SubString(0, 1);
    if( AI->IsElement(Tmp) )
      BAI = AI->FindAtomInfoBySymbol( Tmp );
  }
  if( BAI == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Unknown element: '") << L << '\'' );
  else  {
    FAtomInfo = BAI;
    FLabel = L;
    if( *BAI != iQPeakIndex )
      SetQPeak(0);
  }

  if( GetAtomInfo().GetSymbol().Length() == 2 )
      FLabel[1] = FLabel.o_tolower(FLabel[1]);
  return true;
}
//..............................................................................
void TCAtom::AtomInfo(TBasicAtomInfo* A)  {
  FAtomInfo = A;
  return;
  olxstr Tmp(A->GetSymbol());
  if( FLabel.Length() > Tmp.Length() )
    Tmp << FLabel.SubStringFrom(FAtomInfo->GetSymbol().Length());

  FAtomInfo = A;
  FLabel = FParent->CheckLabel(this, Tmp);
//  FLabel = Tmp;
}
//..............................................................................
void TCAtom::Assign(const TCAtom& S)  {
  SetAfix( S.GetAfix() );
  SetPart( S.GetPart() );
  SetOccp( S.GetOccp() );
  SetOccpVar( S.GetOccpVar() );
  SetQPeak( S.GetQPeak() );
  SetResiId( S.GetResiId() );
  SetSameId( S.GetSameId() );
  SetSharedSiteId( S.GetSharedSiteId() );
  FEllipsoid = S.GetEllipsoid();
  SetUiso( S.GetUiso() );
  SetUisoVar( S.GetUisoVar() );
  FLabel   = S.FLabel;
  FAtomInfo = &S.GetAtomInfo();
//  Frag    = S.Frag;
  LoaderId = S.GetLoaderId();
  Id = S.GetId();
  FragmentId = S.GetFragmentId();
  AfixAtomId = S.GetAfixAtomId();
  FEllpsE  = S.FEllpsE;
  FCCenter = S.GetCCenter();
  SetDeleted( S.IsDeleted() );
  SetSortable( S.IsSortable() );
  SetCanBeGrown( S.GetCanBeGrown() );
  Degeneracy = S.GetDegeneracy();

  /*
  if( FAttachedAtoms )  FAttachedAtomS.Clear();
  if( S.AttachedAtomCount() )
  {
    for(int i=0; i < S.AttachedAtomCount(); i++ )
    {
      AddAttachedAtom( FParent->Atom(S.AttachedAtom(i)->Label()));
    }
  }
  else
  {
    if( FAttachedAtoms )  {  delete FAttachedAtoms;  FAttachedAtoms = NULL;  }
  }
  */
  Hfix = S.GetHfix();
  FFixedValues = S.GetFixedValues();
}
//..............................................................................
TAtomsInfo *TCAtom::AtomsInfo() const {  return FParent->GetAtomsInfo(); }
//..............................................................................
//..............................................................................
void TCAtom::UpdateEllp( const TVectorD &Quad)  {
  if( FEllipsoid == NULL )
    FEllipsoid = &FParent->NewEllp(Quad);
  else
    FEllipsoid->Initialise(Quad);
}
//..............................................................................
void TCAtom::UpdateEllp(const TEllipsoid &NV ) {
  TVectorD Q;
  NV.GetQuad(Q);
  if( !FEllipsoid )
    FEllipsoid = &FParent->NewEllp(Q);
  else
    FEllipsoid->Initialise(Q);
}
//..............................................................................
void DigitStrtok(const olxstr &str, TStrPObjList<olxstr,bool> &chars)  {
  olxstr Dig, Char;
  for(int i=0; i < str.Length(); i++ )  {
    if( str[i] <= '9' && str[i] >= '0' )  {
      if( !Char.IsEmpty() )      {
        chars.Add(Char, true);
        Char = EmptyString;
      }
      Dig << str[i];
    }
    else  {
      if( !Dig.IsEmpty() )  {
        chars.Add(Dig, false);
        Dig = EmptyString;
      }
      Char << str[i];
    }
  }
  if( !Char.IsEmpty() )  chars.Add(Char, true);
  if( !Dig.IsEmpty() )   chars.Add(Dig, false);
}
int TCAtom::CompareAtomLabels(const olxstr& S, const olxstr& S1)  {
  TStrPObjList<olxstr, bool> Chars1, Chars2;

  DigitStrtok(S, Chars1);
  DigitStrtok(S1, Chars2);
  for( int i=0; i < olx_min(Chars1.Count(), Chars2.Count()); i++ )  {
    if( Chars1.Object(i) && Chars2.Object(i) )  {
      int res = Chars1.String(i).Comparei(Chars2.String(i));
      if( res != 0 )  return res;
    }
    if( !Chars1.Object(i) && !Chars2.Object(i) )  {
      int res = Chars1[i].ToInt() - Chars2[i].ToInt();
      //if( !res )  // to tackle 01 < 1
      //{  res = Chars1->String(i).Length() - Chars2->String(i).Length();  }
      //the following commented line allows sorting like C01 C02 and C01A then
      //but though it looks better, it is not correct, so using C01 C01A C02
      //if( res && (Chars1->Count() == Chars2->Count()))  return res;
      if( res != 0 )  return res;
    }

    if( !Chars1.Object(i) && Chars2.Object(i) )  return 1;
    if( Chars1.Object(i) && !Chars2.Object(i) )  return -1;
  }
  return Chars1.Count() - Chars2.Count();
}
//..............................................................................
void TCAtom::AttachAtom(TCAtom *CA)  {
  if( FAttachedAtoms == NULL )  FAttachedAtoms = new TCAtomPList;
  FAttachedAtoms->Add(CA);
}
//..............................................................................
void TCAtom::AttachAtomI(TCAtom *CA)  {
  if( !FAttachedAtomsI )  FAttachedAtomsI = new TCAtomPList;
  FAttachedAtomsI->Add(CA);
}
//..............................................................................
//..............................................................................
//..............................................................................
olxstr TGroupCAtom::GetFullLabel() const  {
  olxstr name(Atom->GetLabel());
  if( Atom->GetResiId() == -1 )  {
    if( Matrix != 0 )
      name << "_$" << (Atom->GetParent()->UsedSymmIndex(*Matrix) + 1);
  }
  else  {
    name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
    if( Matrix != 0 )
      name << '$' << (Atom->GetParent()->UsedSymmIndex(*Matrix) + 1);
  }
  return name;
}

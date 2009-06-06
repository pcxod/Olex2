//---------------------------------------------------------------------------//
// namespace TXFiles: TMol2 - basic procedures for loading Tripos MOL2 files
// (c) Oleg V. Dolomanov, 2009
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "mol2.h"

#include "catom.h"
#include "unitcell.h"
#include "estrlist.h"
#include "exception.h"

const olxstr TMol2::BondNames[] = {"1", "2", "3", "am", "ar", "du", "un", "nc"};
//----------------------------------------------------------------------------//
// TMol2 function bodies
//----------------------------------------------------------------------------//
void TMol2::Clear()  {
  GetAsymmUnit().Clear();
  Bonds.Clear();
}
//..............................................................................
olxstr TMol2::MOLAtom(TCAtom& A)  {
  olxstr rv(A.GetId(), 64);
  rv << '\t' << A.GetLabel() 
     << '\t' << A.ccrd()[0] 
     << '\t' << A.ccrd()[1] 
     << '\t' << A.ccrd()[2]
     << '\t' << A.GetAtomInfo().GetSymbol();
  return rv;
}
//..............................................................................
const olxstr& TMol2::EncodeBondType(short type) const  {
  if( type >= 8 || type < 0 )
    throw TInvalidArgumentException(__OlxSourceInfo, "bond type");
  return BondNames[type-1];
}
//..............................................................................
short TMol2::DecodeBondType(const olxstr& name) const  {
  for( int i=0; i < 8; i++ )
    if( BondNames[i].Equalsi(name) )
      return i+1;
  return mol2btUnknown;
}
//..............................................................................
olxstr TMol2::MOLBond(TMol2Bond& B)  {
  olxstr rv(B.GetId(), 32);
  rv << '\t' << B.a1->GetId() 
     << '\t' << B.a2->GetId() 
     << '\t' << EncodeBondType(B.BondType);
  return rv;
}
//..............................................................................
void TMol2::SaveToStrings(TStrList& Strings)  {
  Strings.Add("@<TRIPOS>MOLECULE");
  Strings.Add(GetTitle());
  Strings.Add( GetAsymmUnit().AtomCount() )  <<
    '\t' << Bonds.Count() << "\t1";
  Strings.Add("SMALL");
  Strings.Add("NO_CHARGES");
  Strings.Add(EmptyString);
  Strings.Add("@<TRIPOS>ATOM");
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )
    Strings.Add( MOLAtom(GetAsymmUnit().GetAtom(i) ) );
  if( !Bonds.IsEmpty() )  {
    Strings.Add(EmptyString);
    Strings.Add("@<TRIPOS>BOND");
    for( int i=0; i < Bonds.Count(); i++ )
      Strings.Add( MOLBond(Bonds[i]) );
  }
}
//..............................................................................
void TMol2::LoadFromStrings(const TStrList& Strings)  {
  Clear();

  olxstr Tmp1, Tmp, Msg;
  vec3d StrCenter;
  Title = "OLEX: imported from MDL MOL";
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  GetAsymmUnit().Axes()[0] = 1;
  GetAsymmUnit().Axes()[1] = 1;
  GetAsymmUnit().Axes()[2] = 1;
  GetAsymmUnit().Angles()[0] = 90;
  GetAsymmUnit().Angles()[1] = 90;
  GetAsymmUnit().Angles()[2] = 90;
  GetAsymmUnit().InitMatrices();
  bool AtomsCycle = false, BondsCycle = false;
  olxdict<int, TCAtom*, TPrimitiveComparator> atoms;
  for( int i=0; i < Strings.Count(); i++ )  {
    Tmp = Strings[i].UpperCase();
    Tmp.Replace('\t', ' ');
    Tmp = Tmp.Trim(' ');
    if( Tmp.IsEmpty() )  continue;
    if( AtomsCycle )  {
      if( Tmp.Compare("@<TRIPOS>BOND") == 0 )  {
        BondsCycle = true;
        AtomsCycle = false;
        continue;
      }
      TStrList toks(Tmp, ' ');
      if( toks.Count() < 6 )
        continue;
      double Ax = toks[2].ToDouble();
      double Ay = toks[3].ToDouble();
      double Az = toks[4].ToDouble();
      if( AtomsInfo.IsElement(toks[5]) )  {
        TCAtom& CA = GetAsymmUnit().NewAtom();
        CA.ccrd()[0] = Ax;
        CA.ccrd()[1] = Ay;
        CA.ccrd()[2] = Az;
        CA.SetLabel( (toks[5] << GetAsymmUnit().AtomCount()+1) );
        atoms(toks[0].ToInt(), &CA); 
      }
      continue;
    }
    if( BondsCycle )  {
      if( Tmp.CharAt(0) == '@' )  {
        BondsCycle = false;
        break;
      }
      TStrList toks( Tmp, ' ');
      if( toks.Count() < 4 )
        continue;
      TMol2Bond* MB = new TMol2Bond(Bonds.Count());
      MB->a1 = atoms[toks[1].ToInt()];
      MB->a2 = atoms[toks[2].ToInt()];
      MB->BondType = DecodeBondType(toks[3]);   // bond type
      Bonds.Add(*MB);
      continue;
    }
    if( Tmp.Compare("@<TRIPOS>ATOM") == 0 )  {
      AtomsCycle = true;
      continue;
    }
  }
}

//..............................................................................
bool TMol2::Adopt(TXFile *XF)  {
  Clear();
  GetAsymmUnit().Assign(XF->GetAsymmUnit());
  GetAsymmUnit().SetZ( (short)XF->GetLattice().GetUnitCell().MatrixCount() );
  return true;
}
//..............................................................................
void TMol2::DeleteAtom(TCAtom *CA)  {  return;  }
//..............................................................................

 

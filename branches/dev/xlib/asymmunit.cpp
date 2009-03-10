//----------------------------------------------------------------------------//
// namespace: crystallographic core
// TAsymmUnit: a collection of symmetry independent atoms
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "asymmunit.h"
#include "catom.h"
#include "ellipsoid.h"
#include "unitcell.h"
#include "estrlist.h"
#include "exception.h"
#include "symmlib.h"
#include "library.h"
#include "symmlib.h"
#include "estlist.h"
#include "lattice.h"
#include "symmparser.h"
#include "refmodel.h"

#undef GetObject

class TAU_SfacSorter  {
public:
  static int Compare(const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* s1, 
                    const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* s2)  {
    double diff = s1->GetObject()->GetMr() - s1->GetObject()->GetMr();
    if( diff < 0 )  return -1;
    if( diff > 0 )  return 1;
    return 0;
  }
};

const olxstr TAsymmUnit::IdName("catom");

//----------------------------------------------------------------------------//
// TAsymmetricUnit function bodies
//----------------------------------------------------------------------------//
TAsymmUnit::TAsymmUnit(TLattice *L) : MainResidue(*this, -1)  {
  AtomsInfo = &TAtomsInfo::GetInstance();
  Lattice   = L;
  Latt = -1;
  Z = 1;
  ContainsEquivalents = false;
  OnSGChange = &Actions.NewQueue("AU_SG_CHANGE");
  RefMod = NULL;
}
//..............................................................................
TAsymmUnit::~TAsymmUnit() {  Clear();  }
//..............................................................................
void  TAsymmUnit::Clear()  {
  Matrices.Clear();
  for( int i=0; i < CAtoms.Count(); i++ )
    delete CAtoms[i];
  CAtoms.Clear();
  for( int i=0; i < Centroids.Count(); i++ )
    delete Centroids[i];
  Centroids.Clear();
  for( int i=0; i < Ellipsoids.Count(); i++ )
    delete Ellipsoids[i];
  Ellipsoids.Clear();
  for( int i=0; i < Residues.Count(); i++ )
    delete Residues[i];
  Residues.Clear();
  MainResidue.Clear();
  Latt = -1;
  Z = 1;
  ContainsEquivalents = false;
}
//..............................................................................
void TAsymmUnit::Assign(const TAsymmUnit& C)  {
  Clear();
  FAxes   = C.FAxes;
  FAngles = C.FAngles;
  RAxes   = C.GetRAxes();
  RAngles = C.GetRAngles();
  Z = C.GetZ();
  Latt = C.GetLatt();

  for( int i = 0; i < C.MatrixCount(); i++ )
    Matrices.AddNew( C.GetMatrix(i) );
  
  for( int i = 0; i < C.EllpCount(); i++ )
    this->NewEllp() = C.GetEllp(i);

  for( int i=0; i < C.Residues.Count(); i++ )  {
    TResidue* resi = C.Residues[i];
    NewResidue( resi->GetClassName(), resi->GetNumber(), resi->GetAlias() ); 
  }
  for( int i = 0; i < C.AtomCount(); i++ )  {
    TCAtom& CA = this->NewAtom();
    CA.SetId(i);
    if( C.GetAtom(i).GetResiId() != -1 )  // main residue
      GetResidue(C.GetAtom(i).GetResiId()).AddAtom(&CA);
  }
  for( int i = 0; i < C.AtomCount(); i++ )  {
    GetAtom(i).Assign( C.GetAtom(i) );
    GetAtom(i).SetId(i);
  }
  // copy matrices
  Cartesian2Cell = C.GetCartesianToCell();
  Cell2Cartesian = C.GetCellToCartesian();
  Hkl2Cartesian =  C.GetHklToCartesian();
  UcifToUxyz     = C.UcifToUxyz;
  UxyzToUcif     = C.UxyzToUcif;
  UcifToUxyzT    = C.UcifToUxyzT;
  UxyzToUcifT    = C.UxyzToUcifT;

  SetContainsEquivalents( C.DoesContainEquivalents() );
  MaxQPeak = C.GetMaxQPeak();
  MinQPeak = C.GetMinQPeak();
}
//..............................................................................
void  TAsymmUnit::InitMatrices()  {
  if( !FAxes[0].GetV() || !FAxes[1].GetV() || !FAxes[2].GetV() )
    throw TFunctionFailedException(__OlxSourceInfo, "zero cell parameters");
  // just to check the validity of my deductions put this in seems to be the same ...
  double cG = cos(FAngles[2].GetV()/180*M_PI),
         cB = cos(FAngles[1].GetV()/180*M_PI),
         cA = cos(FAngles[0].GetV()/180*M_PI),
         sG = sin(FAngles[2].GetV()/180*M_PI),
         sB = sin(FAngles[1].GetV()/180*M_PI),
         sA = sin(FAngles[0].GetV()/180*M_PI);

  double V = FAxes[0].GetV() * FAxes[1].GetV() * FAxes[2].GetV()*sqrt( (1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));

  double cGs = (cA*cB-cG)/(sA*sB),
         cBs = (cA*cG-cB)/(sA*sG),
         cAs = (cB*cG-cA)/(sB*sG),
         as = FAxes[1].GetV()*FAxes[2].GetV()*sA/V,
         bs = FAxes[0].GetV()*FAxes[2].GetV()*sB/V,
         cs = FAxes[0].GetV()*FAxes[1].GetV()*sG/V
         ;
  // cartesian to cell transformation matrix
  Cartesian2Cell.Null();
  Cartesian2Cell[0][0] =  1./FAxes[0].GetV();
  Cartesian2Cell[1][0] = -cG/(sG*FAxes[0].GetV());
  Cartesian2Cell[2][0] = as*cBs;

  Cartesian2Cell[1][1] = 1./(sG*FAxes[1].GetV());
  Cartesian2Cell[2][1] = bs*cAs;

  Cartesian2Cell[2][2] = cs;

  // cell to cartesian transformation matrix
  Cell2Cartesian.Null();
  Cell2Cartesian[0][0] = FAxes[0].GetV();
  Cell2Cartesian[1][0] = FAxes[1].GetV()*cG;
  Cell2Cartesian[2][0] = FAxes[2].GetV()*cB;

  Cell2Cartesian[1][1] = FAxes[1].GetV()*sG;
  Cell2Cartesian[2][1] = -FAxes[2].GetV()*(cB*cG-cA)/sG;

  Cell2Cartesian[2][2] = 1./cs;

  // init hkl to cartesian transformation matrix
//  TMatrixD m( *Cartesian2Cell );
  mat3d m( Cell2Cartesian );
  vec3d v1(m[0]), v2(m[1]), v3(m[2]);

  Hkl2Cartesian[0] = v2.XProdVec(v3)/V;
  Hkl2Cartesian[1] = v3.XProdVec(v1)/V;
  Hkl2Cartesian[2] = v1.XProdVec(v2)/V;

// init Uaniso traformation matices
  m.Null();
  m[0][0] = Hkl2Cartesian[0].Length();
  m[1][1] = Hkl2Cartesian[1].Length();
  m[2][2] = Hkl2Cartesian[2].Length();

  UcifToUxyz = m * Cell2Cartesian;
  UcifToUxyzT = UcifToUxyz;
  UcifToUxyz.Transpose();

  m[0][0] = 1./Hkl2Cartesian[0].Length();
  m[1][1] = 1./Hkl2Cartesian[1].Length();
  m[2][2] = 1./Hkl2Cartesian[2].Length();

  UxyzToUcif = Cartesian2Cell * m;
  UxyzToUcifT = UxyzToUcif;
  UxyzToUcif.Transpose();
}
//..............................................................................
void TAsymmUnit::InitData()  {
  // init QPeak intensities
  MaxQPeak = -1000;
  MinQPeak = 1000;
  for( int i =0; i < AtomCount(); i++ )  {
    if( !CAtoms[i]->IsDeleted() && CAtoms[i]->GetAtomInfo() == iQPeakIndex )  {
      const double qpeak = CAtoms[i]->GetQPeak();
      if( qpeak < MinQPeak )  MinQPeak = qpeak;
      if( qpeak > MaxQPeak )  MaxQPeak = qpeak;
    }
  }
}
//..............................................................................
TAsymmUnit::TResidue& TAsymmUnit::NewResidue(const olxstr& RClass, int number, const olxstr& alias)  {
  for( int i=0; i < Residues.Count(); i++ )
    if( Residues[i]->GetNumber() == number )  {
      return *Residues[i];
      //throw TInvalidArgumentException(__OlxSourceInfo, "dublicated residue number");
    }
  TResidue* resi = new TResidue(*this, Residues.Count(), RClass, number, alias);
  Residues.Add( resi );
  return *resi;
}
//..............................................................................
void TAsymmUnit::FindResidues(const olxstr& resi, TPtrList<TAsymmUnit::TResidue>& list) {
  if( resi.IsEmpty() )  {
    list.Add(&MainResidue);
    return;
  }
  if( resi.IsNumber() )  {
    int number = resi.ToInt();
    for( int i=0; i < Residues.Count(); i++ )  {
      if( Residues[i]->GetNumber() == number )  {
        list.Add(Residues[i]);
        break;  // number must be unique
      }
    }
  }
  else  {
    if( resi.Length() == 1 && resi.CharAt(0) == '*' )  {  //special case
      list.Assign(Residues);
      list.Add( &MainResidue );
    }
    for( int i=0; i < Residues.Count(); i++ )
      if( Residues[i]->GetClassName().Comparei(resi) == 0 || Residues[i]->GetAlias().Comparei(resi) == 0 ) 
        list.Add(Residues[i]);
  }
}
//..............................................................................
void TAsymmUnit::ClearResidues(bool moveToMain)  {
  for( int i=0;  i < Residues.Count(); i++ )
    delete Residues[i];
  Residues.Clear();
  MainResidue.Clear();
  if( moveToMain)  {
    MainResidue.SetCapacity(CAtoms.Count());
    for( int i=0; i < CAtoms.Count(); i++ )  {
      CAtoms[i]->SetResiId(-1);
      MainResidue.AddAtom(CAtoms[i]);
    }
  }
}
//..............................................................................
void TAsymmUnit::AssignResidues(const TAsymmUnit& au)  {
  ClearResidues(false);
  MainResidue = au.MainResidue;
  for( int i=0; i < au.Residues.Count(); i++ )  {
    TResidue* resi = au.Residues[i];
    NewResidue( resi->GetClassName(), resi->GetNumber() ) = *resi; 
  }
}
//..............................................................................
TCAtom& TAsymmUnit::NewAtom(TResidue* resi)  {
  TCAtom *A = new TCAtom(this);
  A->SetId( CAtoms.Count() );
  CAtoms.Add(A);
  if( resi == NULL )  resi = &MainResidue;
  resi->AddAtom(A);
  return *A;
}
//..............................................................................
TCAtom& TAsymmUnit::NewCentroid(const vec3d& CCenter)  {
  TCAtom& A = NewAtom();
  A.ccrd() = CCenter;
  A.SetLabel( olxstr("Cnt") << CAtoms.Count() );
  return A;
}
//..............................................................................
TCAtom * TAsymmUnit::FindCAtom(const olxstr &Label, TResidue* resi)  const {
  const int defPart = 0x0faf;
  int part = defPart;
  olxstr lb(Label);
  int us_ind = Label.IndexOf('_');
  if( us_ind != -1 && ++us_ind < Label.Length() )  {
    part = olxstr::o_tolower(Label.CharAt(us_ind)) - 'a' + 1;
    lb = lb.SubStringTo(us_ind-1);
  }
  if( resi != NULL )  {
    for( int i=0; i < resi->Count(); i++ )
      if( !resi->GetAtom(i).IsDeleted() && resi->GetAtom(i).GetLabel().Comparei(lb) == 0 )
        if( part == -1 || resi->GetAtom(i).GetPart() == part )
        return &resi->GetAtom(i);
  }
  else  {  // global search
    for( int i=0; i < CAtoms.Count(); i++ )
      if( !CAtoms[i]->IsDeleted() && CAtoms[i]->GetLabel().Comparei(lb) == 0  )
        if( part == defPart || CAtoms[i]->GetPart() == part )
          return CAtoms[i];
  }
  return NULL;
}
//..............................................................................
void TAsymmUnit::InitAtomIds()  {  // initialises atom ids if any were added or removed
  for( int i=0; i < AtomCount(); i++ )    
    GetAtom(i).SetId(i);
  for( int i=0; i < EllpCount(); i++ )    
    GetEllp(i).SetId(i);
}
//..............................................................................
void TAsymmUnit::PackAtoms()  {
  for( int i=-1; i < Residues.Count(); i++ )  {
    TAsymmUnit::TResidue& resi = GetResidue(i);
    for( int j=0; j < resi.Count(); j++ )
      if( resi[j].IsDeleted() )
        resi.AtomList()[j] = NULL;
    resi.AtomList().Pack();
  }
  for( int i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->IsDeleted() )  {
      delete CAtoms[i];
      CAtoms[i] = NULL;
    }
  }
  CAtoms.Pack();
  for( int i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetId(i);
}
//..............................................................................
TEllipsoid& TAsymmUnit::NewEllp() {
  TEllipsoid *E = new TEllipsoid();
  Ellipsoids.Add(E);
  E->SetId( Ellipsoids.Count()-1 );
  return *E;
}
//..............................................................................
void TAsymmUnit::PackEllps() {
  int removed = 0;
  for( int i=0; i < Ellipsoids.Count(); i++ )  {
    if( Ellipsoids[i] == NULL )  {
      for( int j=0; j < CAtoms.Count(); j++ )  {
        if( CAtoms[j]->GetEllpId() > (i-removed) )
          CAtoms[j]->SetEllpId( CAtoms[j]->GetEllpId() - 1 );
      }
      removed++;
    }
    else
      Ellipsoids[i]->SetId(i-removed);
  }
  Ellipsoids.Pack();
}
//..............................................................................
void TAsymmUnit::NullEllp(size_t i)  {
  if( Ellipsoids[i] != NULL )  {
    delete Ellipsoids[i];
    Ellipsoids[i] = NULL;
  }
}
//..............................................................................
void TAsymmUnit::ClearEllps()  {
  for( int i=0; i < Ellipsoids.Count(); i++ )
    delete Ellipsoids[i];
  for( int i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->AssignEllp(NULL);
  Ellipsoids.Clear();
}
//..............................................................................
vec3d TAsymmUnit::GetOCenter(bool IncludeQ, bool IncludeH) const {
  vec3d P;
  double wght = 0;
  for( int i=0; i < AtomCount(); i++ )  {
    if( CAtoms[i]->IsDeleted() )  continue;
    if( !IncludeQ && CAtoms[i]->GetAtomInfo() == iQPeakIndex )  continue;
    if( !IncludeH && CAtoms[i]->GetAtomInfo() == iHydrogenIndex )  continue;
    P += CAtoms[i]->ccrd()*CAtoms[i]->GetOccu();
    wght += CAtoms[i]->GetOccu();
  }

  if( wght != 0 )
    P /= wght;
  return P;
}
//..............................................................................
void TAsymmUnit::SummFormula(TStrPObjList<olxstr,TBasicAtomInfo*>& BasicAtoms, olxstr &Elements,
                             olxstr &Numbers, bool MultiplyZ) const {
  BasicAtoms.Clear();
  TBasicAtomInfo *AI, *Carbon=NULL, *Hydrogen=NULL;

  for( int i=0; i < AtomCount(); i++ )  {
    if( CAtoms[i]->IsDeleted() )  continue;
    TCAtom& A = *CAtoms[i];
    bool Uniq = true;
    for( int j=0; j < BasicAtoms.Count(); j++)  {
      if( BasicAtoms.Object(j)->GetIndex() == A.GetAtomInfo().GetIndex() ) {  // already in the list ?
        A.GetAtomInfo().SetSumm( A.GetAtomInfo().GetSumm() + A.GetOccu() );       // update the quantity
        Uniq = false;
        break;
      }
    }
    if( Uniq )  {
      A.GetAtomInfo().SetSumm( A.GetOccu() );
      if( A.GetAtomInfo().GetIndex() == iCarbonIndex )   Carbon = &A.GetAtomInfo();
      if( A.GetAtomInfo().GetIndex() == iHydrogenIndex )  Hydrogen = &A.GetAtomInfo();
      BasicAtoms.Add(A.GetAtomInfo().GetSymbol(), &A.GetAtomInfo());
    }
  }
  BasicAtoms.QuickSort<TAU_SfacSorter>();
  if( Carbon != NULL )
    BasicAtoms.Swap(0, BasicAtoms.IndexOfObject(Carbon));
  if( Hydrogen != NULL && BasicAtoms.Count() > 1 )
    BasicAtoms.Swap(1, BasicAtoms.IndexOfObject(Hydrogen));
  for( int i=0; i < BasicAtoms.Count(); i++)  {
    AI = BasicAtoms.Object(i);
    Elements << AI->GetSymbol();
    if( MultiplyZ )
      Numbers << olxstr::FormatFloat(3, AI->GetSumm()*GetZ());
    else
      Numbers << olxstr::FormatFloat(3, AI->GetSumm());
    if( i < (BasicAtoms.Count()-1) )  {
      Elements << ' ';
      Numbers  << ' ';
    }
  }
}
//..............................................................................
olxstr TAsymmUnit::SummFormula(const olxstr &Sep, bool MultiplyZ) const  {
  TCAtomPList UniqAtoms;
  olxstr T;

  int matrixInc = 0;
  // searching for the identity matrix
  bool Uniq = true;
  for( int i=0; i < MatrixCount(); i++ )
    if( GetMatrix(i).r.IsI() )  {
      Uniq = false;  break;
    }
  if( Uniq )  matrixInc ++;

  for( int i=0; i < AtomCount(); i++ )  {
    TCAtom& A = *CAtoms[i];
    if( A.IsDeleted() )  continue;
    Uniq = true;
    for( int j=0; j < UniqAtoms.Count(); j++)  {
      if( UniqAtoms[j]->GetAtomInfo().GetIndex() == A.GetAtomInfo().GetIndex() )  { // already in the list ?
        A.GetAtomInfo().SetSumm( A.GetAtomInfo().GetSumm() + A.GetOccu() );       // update the quantity
        Uniq = false;
        break;
      }
    }
    if( Uniq )  {
      A.GetAtomInfo().SetSumm( A.GetOccu() );
      UniqAtoms.Add(&A);
    }
  }
  for( int i=0; i < UniqAtoms.Count(); i++)  {
    TCAtom& A = *UniqAtoms[i];
    if( A.GetAtomInfo().GetIndex() == iQPeakIndex )  continue;
    T << A.GetAtomInfo().GetSymbol();
    if( MultiplyZ )
      T << olxstr::FormatFloat(3, A.GetAtomInfo().GetSumm()*(MatrixCount()+matrixInc) );
    else
      T << olxstr::FormatFloat(3, A.GetAtomInfo().GetSumm());
    if( i < (UniqAtoms.Count()-1) )
      T << Sep;
  }
  return T;
}
//..............................................................................
double TAsymmUnit::MolWeight() const  {
  double Mw = 0;
  for( int i=0; i < AtomCount(); i++ )
    Mw += CAtoms[i]->GetAtomInfo().GetMr();
  return Mw;
}
//..............................................................................
void TAsymmUnit::AddMatrix(const smatd& a)  {
  if( a.r.IsI() )  Matrices.InsertCCopy(0, a);
  else             Matrices.AddCCopy(a);
}
//..............................................................................
olxstr TAsymmUnit::CheckLabel(const TCAtom* ca, const olxstr &Label, char a, char b, char c) const  {
  olxstr LB( (Label.Length() > 4) ? Label.SubStringTo(2) : Label );
  if( ca != NULL )  {
    const TResidue& resi = GetResidue(ca->GetResiId() );
    for( int i=0; i < resi.Count(); i++ )  {
      const TCAtom& atom = resi[i];
      if( atom.GetPart() != ca->GetPart() && (atom.GetPart()|ca->GetPart()) != 0 )  continue;
      if( !atom.IsDeleted() && (atom.GetLabel().Comparei(Label) == 0) && 
        (atom.GetId() != ca->GetId()) )  {
        LB = atom.GetAtomInfo().GetSymbol();
        if( LB.Length() == 2 )  LB[0] = LB.o_toupper(LB[0]);
        LB << a << b;
        if( LB.Length() < 4 )  LB << c;
        if( a < '9' )  return CheckLabel(ca, LB, (char)(a+1), b, c);
        if( b < 'z' )  return CheckLabel(ca, LB, '0', (char)(b+1), c);
        if( c < 'z' )  return CheckLabel(ca, LB, '0', 'a', (char)(c+1));
        throw TFunctionFailedException(__OlxSourceInfo, "cannot create label");
      }
    }
    return LB;
  }
  for( int i=0; i < AtomCount(); i++ )  {
    const TCAtom& CA = GetAtom(i);
    if( !CA.IsDeleted() && (CA.GetLabel().Comparei(Label) == 0) )  {
      LB = CA.GetAtomInfo().GetSymbol();
      if( LB.Length() == 2 )  LB[0] = LB.o_toupper(LB[0]);
      LB << a << b;
      if( LB.Length() < 4 )  LB << c;
      if( a < '9' )  return CheckLabel(ca, LB, (char)(a+1), b, c);
      if( b < 'z' )  return CheckLabel(ca, LB, '0', (char)(b+1), c);
      if( c < 'z' )  return CheckLabel(ca, LB, '0', 'a', (char)(c+1));
      throw TFunctionFailedException(__OlxSourceInfo, "cannot create label");
    }
  }
  return LB;
}
//..............................................................................
olxstr TAsymmUnit::ValidateLabel(const olxstr &Label) const  {
  olxstr LB( (Label.Length() > 4) ? Label.SubStringTo(4) : Label );
  int cnt=0;
  for( int i=0; i < AtomCount(); i++ )  {
    const TCAtom& CA = GetAtom(i);
    if( !CA.IsDeleted() && (CA.GetLabel().Comparei(Label) == 0) )
       cnt++;
    if( cnt > 1 )
      return CheckLabel(NULL, LB);
  }
  return LB;
}
//..............................................................................
size_t TAsymmUnit::CountElements(const olxstr &Symbol) const  {
  TBasicAtomInfo *BAI = GetAtomsInfo()->FindAtomInfoBySymbol(Symbol);
  if( BAI == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("unknown atom: '") << Symbol << '\'');
  int cnt = 0;
  for( int i=0; i < AtomCount(); i++ )
    if( &(GetAtom(i).GetAtomInfo()) == BAI )
      cnt++;
  return cnt;
}
//..............................................................................
void TAsymmUnit::Sort(TCAtomPList* list) {
 // sorting by four params
  if( list == NULL )  list = &MainResidue.AtomList();
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
 TCAtomPList::QuickSorter.Sort<TCAtomPComparator>(*list);
}
//..............................................................................
int TAsymmUnit::GetNextPart() const {
  int part = 0;
  for( int i=0; i < AtomCount(); i++ )
    if( GetAtom(i).GetPart() > part )
      part = GetAtom(i).GetPart();

  return part+1;
}
//..............................................................................
void TAsymmUnit::ChangeSpaceGroup(const TSpaceGroup& sg)  {
  OnSGChange->Execute(this, &sg);
  Latt = sg.GetLattice().GetLatt();
  if( !sg.IsCentrosymmetric() && Latt > 0 )  Latt = -Latt;

  Matrices.Clear();
  for( int i=0; i < sg.MatrixCount(); i++ )
    Matrices.AddCCopy( sg.GetMatrix(i) );
}
//..............................................................................
void TAsymmUnit::OnCAtomCrdChange( TCAtom* ca, const smatd& matr )  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
double TAsymmUnit::CalcCellVolume()  const  {
  double cosa = cos( FAngles[0].GetV()*M_PI/180 ),
         cosb = cos( FAngles[1].GetV()*M_PI/180 ),
         cosg = cos( FAngles[2].GetV()*M_PI/180 );
  return  FAxes[0].GetV()*
          FAxes[1].GetV()*
          FAxes[2].GetV()*sqrt( (1-cosa*cosa-cosb*cosb-cosg*cosg) + 2*(cosa*cosb*cosg));
}
double TAsymmUnit::EstimateZ(int atomCount) const  {
  double auv = CalcCellVolume()/(TUnitCell::GetMatrixMultiplier(GetLatt())*(MatrixCount()+1));
  int zp = Round(auv/(18.6*atomCount));
  return olx_max((TUnitCell::GetMatrixMultiplier(GetLatt())*(MatrixCount()+1) * zp), 1);
}
//..............................................................................
void TAsymmUnit::ToDataItem(TDataItem& item) const  {
  TDataItem& cell = item.AddItem("cell");
  cell.AddField("a", FAxes[0].ToString());
  cell.AddField("b", FAxes[1].ToString());
  cell.AddField("c", FAxes[2].ToString());
  cell.AddField("alpha", FAngles[0].ToString());
  cell.AddField("beta",  FAngles[1].ToString());
  cell.AddField("gamma", FAngles[2].ToString());
  cell.AddField("Z", Z);
  TDataItem& symm = item.AddItem("symm");
  symm.AddField("latt", Latt);
  for(int i=0; i < Matrices.Count(); i++ )  
    symm.AddItem(i, TSymmParser::MatrixToSymmEx(Matrices[i]) );
  TDataItem& resi = item.AddItem("residues");
  int atom_id = 0;
  for( int i=-1; i < Residues.Count(); i++ )  {
    TResidue& r = GetResidue(i);
    if( r.IsEmpty() )  continue;
    TDataItem* ri;
    if( i == -1 )
      ri = &resi.AddItem("default");
    else  {
      ri = &resi.AddItem( r.GetNumber() );
      ri->AddField("class_name", r.GetClassName());
      ri->AddField("alias", r.GetAlias());
    }
    for( int j=0; j < r.Count(); j++ )  {
      if( r[j].IsDeleted() )  continue;
      r[j].SetTag(atom_id);
      r[j].SetId(atom_id);
      r[j].ToDataItem(ri->AddItem(atom_id++));
    }
  }
  for( int i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetId(i);
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TAsymmUnit::PyExport(TPtrList<PyObject>& _atoms)  {
  for( int i=0; i < CAtoms.Count(); i++ )
    CAtoms[i]->SetId(i);
  PyObject* main = PyDict_New(), *cell = PyDict_New();
  PyDict_SetItemString(cell, "a", Py_BuildValue("(dd)", FAxes[0].GetV(), FAxes[0].GetE()));
  PyDict_SetItemString(cell, "b", Py_BuildValue("(dd)", FAxes[1].GetV(), FAxes[1].GetE()));
  PyDict_SetItemString(cell, "c", Py_BuildValue("(dd)", FAxes[2].GetV(), FAxes[2].GetE()));
  PyDict_SetItemString(cell, "alpha", Py_BuildValue("(dd)", FAngles[0].GetV(), FAngles[0].GetE()));
  PyDict_SetItemString(cell, "beta", Py_BuildValue("(dd)", FAngles[1].GetV(), FAngles[1].GetE()));
  PyDict_SetItemString(cell, "gamma", Py_BuildValue("(dd)", FAngles[2].GetV(), FAngles[2].GetE()));
  PyDict_SetItemString(cell, "z", Py_BuildValue("i", Z));
  PyDict_SetItemString(main, "cell", cell);
  int resi_cnt = 0;
  for( int i=-1; i < Residues.Count(); i++ )  {
    TResidue& r = GetResidue(i);
    if( r.IsEmpty() )  continue;
    resi_cnt++;
  }
  PyObject* residues = PyTuple_New(resi_cnt);
  resi_cnt = 0;

  int atom_id = 0;
  for( int i=-1; i < Residues.Count(); i++ )  {
    TResidue& r = GetResidue(i);
    if( r.IsEmpty() )  continue;
    int atom_cnt = 0;
    for( int j=0; j < r.Count(); j++ )  {
      if( r[j].IsDeleted() )  continue;
      r[j].SetTag(atom_id++);
      atom_cnt++;
    }
    PyObject* atoms = PyTuple_New(atom_cnt), 
      *ri = PyDict_New();

    if( i == -1 )
      PyDict_SetItemString(ri, "class", PythonExt::BuildString("default"));
    else  {
      PyDict_SetItemString(ri, "class", PythonExt::BuildString(r.GetClassName()));
      PyDict_SetItemString(ri, "alias", PythonExt::BuildString(r.GetAlias()));
      PyDict_SetItemString(ri, "number", Py_BuildValue("i", r.GetNumber()));
    }
    atom_cnt = 0;
    for( int j=0; j < r.Count(); j++ )  {
      if( r[j].IsDeleted() )  continue;
      PyObject* atom = _atoms.Add(r[j].PyExport());
      PyDict_SetItemString(atom, "aunit_id", Py_BuildValue("i", r[j].GetId()) );
      PyTuple_SetItem(atoms, atom_cnt++, atom );
    }
    PyDict_SetItemString(ri, "atoms", atoms);
    PyTuple_SetItem(residues, resi_cnt++, ri );
  }
  PyDict_SetItemString(main, "residues", residues);
  return main;
}
#endif
//..............................................................................
void TAsymmUnit::FromDataItem(TDataItem& item)  {
  Clear();
  TDataItem& cell = item.FindRequiredItem("cell");
  FAxes[0] = cell.GetRequiredField("a");
  FAxes[1] = cell.GetRequiredField("b");
  FAxes[2] = cell.GetRequiredField("c");
  FAngles[0] = cell.GetRequiredField("alpha");
  FAngles[1] = cell.GetRequiredField("beta");
  FAngles[2] = cell.GetRequiredField("gamma");
  Z = cell.GetRequiredField("Z").ToInt();
  TDataItem& symm = item.FindRequiredItem("symm");
  Latt = symm.GetRequiredField("latt").ToInt();
  for(int i=0; i < symm.ItemCount(); i++ )  
    TSymmParser::SymmToMatrix(symm.GetItem(i).GetValue(), Matrices.AddNew());
  TDataItem& resis = item.FindRequiredItem("residues");
  for( int i=0; i < resis.ItemCount(); i++ )  {
    TDataItem& resi = resis.GetItem(i);
    TResidue& r = (i==0 ? MainResidue : NewResidue(resi.GetRequiredField("class_name"),
      resi.GetValue().ToInt(), resi.GetRequiredField("alias")) );
    for( int j=0; j < resi.ItemCount(); j++ )  {
      NewAtom(&r).FromDataItem(resi.GetItem(j));
    }
  }
  InitMatrices();
  InitData();
}
//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................



void TAsymmUnit::LibGetAtomCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( AtomCount() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomCrd(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).ccrd().ToString() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomName(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).Label() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomType(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  E.SetRetVal( GetAtom(index).GetAtomInfo().GetSymbol() );
}
//..............................................................................
void TAsymmUnit::LibGetPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    int index = Params[0].ToInt();
    if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal( GetAtom(index).GetQPeak() );
  }
  else  {
    TCAtom* ca = FindCAtom( Params[0] );
    if( ca != NULL && ca->GetAtomInfo().GetIndex() == iQPeakIndex )
      E.SetRetVal( ca->GetQPeak() );
    else
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("unknown peak \'") << Params[0] << '\'');
  }
}
//..............................................................................
void TAsymmUnit::LibGetAtomU(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  evecd Q(1);
  if( GetAtom(index).GetEllipsoid() == NULL )  {
    // TODO: a special condition - the atom is isotropic, but a user wishes it to be
    // anisotropic - six values a, a, a, 0, 0, 0 have to be passed
    //if( GetAtom(index)->
    Q[0] = GetAtom(index).GetUiso();
  }
  else  {  // the function resises the vector automatically
    Q.Resize(6);
    GetAtom(index).GetEllipsoid()->GetQuad(Q);
  }

  E.SetRetVal( Q.ToString() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomUiso(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetUiso() );
}
//..............................................................................
void TAsymmUnit::LibGetCell(const TStrObjList& Params, TMacroError& E)  {
  evecd V(6);
  V[0] = FAxes[0].GetV();    V[1] = FAxes[1].GetV();    V[2] = FAxes[2].GetV();
  V[3] = FAngles[0].GetV();  V[4] = FAngles[1].GetV();  V[5] = FAngles[2].GetV();
  E.SetRetVal( V.ToString() );
}
//..............................................................................
void TAsymmUnit::LibGetVolume(const TStrObjList& Params, TMacroError& E)  {
  double v = CalcCellVolume()/Lattice->GetUnitCell().MatrixCount();
  E.SetRetVal( v );
}
//..............................................................................
void TAsymmUnit::LibGetCellVolume(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( CalcCellVolume() );
}
//..............................................................................
void TAsymmUnit::LibGetSymm(const TStrObjList& Params, TMacroError& E)  {
  if( TSymmLib::GetInstance() == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Symmetry librray is not initialised" );
    return;
  }
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( *this );
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Could not locate spacegroup" );
    return;
  }
  E.SetRetVal( sg->GetName() );
}
//..............................................................................
void TAsymmUnit::LibSetAtomCrd(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& ca = GetAtom(index);
  for( int i=0; i < 3; i++ )
    GetRefMod()->Vars.SetParam(ca, catom_var_name_X+i, Params[i+1].ToDouble());
  E.SetRetVal(true);
}
//..............................................................................
void TAsymmUnit::LibSetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  olxstr newLabel;
  if( Params.String(1).IsNumber() )  {
    int inc = Params[1].ToInt();
    int v = GetAtom(index).GetAtomInfo().GetIndex() + inc;
    if( v >= 0 && v <= iQPeakIndex )  {
      newLabel << GetAtomsInfo()->GetAtomInfo(v).GetSymbol()
               << GetAtom(index).Label().SubStringFrom(
                    GetAtom(index).GetAtomInfo().GetSymbol().Length() );
    }
  }
  else  {
    newLabel = Params[1];
  }
  newLabel = CheckLabel(&GetAtom(index), newLabel );
  if( !newLabel.Length() || !GetAtom(index).SetLabel(newLabel) )  {
    E.ProcessingError(__OlxSrcInfo, "incorrect label ") << Params.String(1);
    return;
  }
}
//..............................................................................
void TAsymmUnit::LibGetAtomLabel(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  olxstr newLabel;
  if( Params.String(1).IsNumber() )  {
    int inc = Params[1].ToInt();
    int v = GetAtom(index).GetAtomInfo().GetIndex() + inc;
    if( v >= 0 && v <= iQPeakIndex )  {
      E.SetRetVal( GetAtomsInfo()->GetAtomInfo(v).GetSymbol() );
      return;
    }
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "a number is expected" );
    E.SetRetVal( E.GetInfo() );
    return;
  }
}
//..............................................................................
void TAsymmUnit::LibIsAtomDeleted(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).IsDeleted() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetOccu() );
}
//..............................................................................
void TAsymmUnit::LibGetAtomAfix(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  E.SetRetVal( GetAtom(index).GetAfix() );
}
//..............................................................................
void TAsymmUnit::LibIsPeak(const TStrObjList& Params, TMacroError& E)  {
  if( Params[0].IsNumber() )  {
    int index = Params[0].ToInt();
    if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
    E.SetRetVal( GetAtom(index).GetAtomInfo().GetIndex() == iQPeakIndex );
  }
  else  {
    TCAtom* ca = FindCAtom( Params[0] );
    if( ca != NULL )
      E.SetRetVal( ca->GetAtomInfo().GetIndex() == iQPeakIndex );
    else
      E.SetRetVal( false );
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomU(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  TCAtom& ca = GetAtom(index);
  if( (GetAtom(index).GetEllipsoid() != NULL) && (Params.Count() == 7) )  {
    double V[6];
    for( int i=0; i < 6; i++ )
      V[i] = GetRefMod()->Vars.SetParam(ca, catom_var_name_U11+i, Params[i+1].ToDouble());
    ca.GetEllipsoid()->Initialise( V );
  }
  else if( (ca.GetEllipsoid() == NULL) && (Params.Count() == 2) ) {
    GetRefMod()->Vars.SetParam(ca, catom_var_name_Uiso, Params[1].ToDouble());
  }
  else {
    olxstr at = ca.GetEllipsoid() == NULL ? "isotropic" : "anisotropic";
    E.ProcessingError(__OlxSrcInfo, "invalid number of arguments: ") << Params.Count() << " for " <<
      at << " atom " << ca.Label();
  }
}
//..............................................................................
void TAsymmUnit::LibSetAtomOccu(const TStrObjList& Params, TMacroError& E)  {
  int index = Params[0].ToInt();
  if( index < 0 || index >= AtomCount() )  throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, AtomCount());
  GetRefMod()->Vars.SetParam(GetAtom(index), catom_var_name_Sof, Params[1].ToDouble());
}
//..............................................................................
void TAsymmUnit::LibNewAtom(const TStrObjList& Params, TMacroError& E)  {
  vec3d crd(Params[1].ToDouble(), Params[2].ToDouble(), Params[3].ToDouble());
  if( Lattice != NULL )  {
    vec3d test_pos(crd);
    if( Lattice->GetUnitCell().FindOverlappingAtom( test_pos, 0.3 ) != NULL)  {
      E.SetRetVal(-1);
      return;
    }
  }
  int QPeakIndex = -1;
  double qPeak = 0;
  olxstr qLabel("Q");
  if( Params[0].IsNumber() )  {
    TPSTypeList<double, TCAtom*> sortedPeaks;
    qPeak = Params[0].ToDouble();
    int ac = CAtoms.Count();
    for( int i=0; i < ac; i++ )  {
      if( CAtoms[i]->GetAtomInfo() != iQPeakIndex || CAtoms[i]->IsDeleted() )  continue;
      sortedPeaks.Add(CAtoms[i]->GetQPeak(), CAtoms[i] );
    }
    sortedPeaks.Add( qPeak, NULL);
    ac = sortedPeaks.Count();
    for( int i=0; i < ac; i++ )  {
      if( sortedPeaks.GetObject(i) != NULL )
        sortedPeaks.GetObject(i)->Label() = (qLabel + olxstr(ac-i));
    }
    QPeakIndex = ac - sortedPeaks.IndexOfComparable( qPeak );
    MinQPeak = sortedPeaks.GetComparable(0);
    MaxQPeak = sortedPeaks.Last().Comparable();
  }

  TCAtom& ca = this->NewAtom();
  if( QPeakIndex >= 0 )  {
    ca.Label() = qLabel << olxstr(QPeakIndex);
    ca.SetAtomInfo( &AtomsInfo->GetAtomInfo(iQPeakIndex) );
    ca.SetQPeak( qPeak );
    GetRefMod()->Vars.SetParam(ca, catom_var_name_Sof, 11.0);
    GetRefMod()->Vars.SetParam(ca, catom_var_name_Uiso, 0.5);
    for( int i=0; i < 3; i++ )
      GetRefMod()->Vars.SetParam(ca, catom_var_name_X+i, crd[i]);
  }
  else
    ca.SetLabel( Params[0] );
  E.SetRetVal( AtomCount() -1 );
  ca.AssignEllp( NULL );
}
//..............................................................................
void TAsymmUnit::LibGetZ(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( Z );
}
//..............................................................................
void TAsymmUnit::LibSetZ(const TStrObjList& Params, TMacroError& E)  {
  Z = Params[0].ToInt();
  if( Z <= 0 )  Z = 1;
}
//..............................................................................
void TAsymmUnit::LibGetZprime(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( 1 );
}
//..............................................................................
void TAsymmUnit::LibSetZprime(const TStrObjList& Params, TMacroError& E)  {
  double zp = Params[0].ToDouble();
  Z = (short)Round(TUnitCell::GetMatrixMultiplier(Latt)*MatrixCount()*zp);
  if( Z <= 0 ) Z = 1;
}
//..............................................................................

TLibrary* TAsymmUnit::ExportLibrary(const olxstr& name) {

  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("au") : name );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibNewAtom, "NewAtom", fpFour,
"Adds a new atom to the asymmetric unit and return its ID, by which it can be reffered.\
 The function takes the atom name and ccordinates, if -1 is returned, the atom is not created") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomCount, "GetAtomCount", fpNone,
"Returns the atom count in the asymmetric unit") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetSymm, "GetCellSymm", fpNone,
"Returns spacegroup of currently loaded file as name: 'C2', 'I41/amd', etc") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomCrd, "GetAtomCrd", fpOne,
"Returns a comma separated list of fractional coordinates for the specified atom") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomName, "GetAtomName", fpOne,
"Returns atom label") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomType, "GetAtomType", fpOne,
"Returns atom type (element)"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomOccu, "GetAtomOccu", fpOne,
"Returns atom occupancy"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomAfix, "GetAtomAfix", fpOne,
"Returns atom AFIX"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetPeak, "GetPeak", fpOne,
"Returns peak intensity"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomU, "GetAtomU", fpOne,
"Returns a single number or six, comma separated values") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomUiso, "GetAtomUiso", fpOne,
"Returns a single number Uiso or (U11+U22+U33)/3") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetCell, "GetCell", fpNone,
"Returns six comma separated values for a, b, c and alpha, beta, gamma") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetVolume, "GetVolume", fpNone,
"Returns volume of the unit cell divided by the number of symmetry elements") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetCellVolume, "GetCellVolume", fpNone,
"Returns volume of the unit cell") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomCrd, "SetAtomCrd", fpFour,
"Sets atom coordinates to specified values, first parameters is the atom ID") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomU, "SetAtomU", fpSeven | fpTwo,
"Sets atoms Uiso/anis first paramater is the atom ID followed by 1 or six parameters"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomOccu, "SetAtomOccu", fpTwo,
"Sets atom's occupancy; first parameter is the atom ID followed by occupancy"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetAtomLabel, "SetAtomlabel", fpTwo,
"Sets atom labels to provided value. The first parameter is the atom ID") );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetAtomLabel, "GetAtomlabel", fpTwo,
"The takes two arguments - the atom ID and increment. The increment is used to navigate through\
 the periodic table, so increment +1 will return next element and -1 the previous element in the\
 periodic table"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibIsAtomDeleted, "IsAtomDeleted", fpOne,
"Checks status of specified atom"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibIsPeak, "IsPeak", fpOne,
"Checks if specified atom is  peak"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetZ, "GetZ", fpNone,
"Returns current Z"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetZ, "SetZ", fpOne,
"Sets current Z. Does not update content or whatsoever"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibGetZprime, "GetZprime", fpNone,
"Returns current Z divided byt the number of matrices of current spacegroup"  ) );
  lib->RegisterFunction<TAsymmUnit>( new TFunction<TAsymmUnit>(this,  &TAsymmUnit::LibSetZprime, "SetZprime", fpOne,
"Sets Z' for the structure"  ) );
  return lib;
}
//..............................................................................
//..............................................................................


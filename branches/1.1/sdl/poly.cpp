//---------------------------------------------------------------------------//
// Binominal polynomial routins
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "poly.h"
#include "exception.h"
#include "emath.h"

UseEsdlNamespace()
//----------------------------------------------------------------------------//
int TSPoint::SPointsSortA(const TSPoint& I, const TSPoint& I1)  {
  if( I.Y < I1.Y )  return 1;
  if( I.Y > I1.Y )  return -1;
  return 0;
}
//..............................................................................
int TSPoint::SPointsSortB(const TSPoint& I, const TSPoint& I1)  {
  if( I.X < I1.X )  return 1;
  if( I.X > I1.X )  return -1;
  return 0;
}
//..............................................................................
int _PMembersSort(const TPMember& I, const TPMember& I1) {
  if( I.Id == I1.Id )
    return I.Extent - I1.Extent;
  else
    return olx_cmp(I.Id, I1.Id);
}
//----------------------------------------------------------------------------//
int _PolynomMembersSort(const TPolynomMember& I, const TPolynomMember& I1)  {
  if( I == I1 )
    return 0;
  else if( I.GetMult() < I1.GetMult() )
     return -1;
  else
    return 1;
}
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//..............................................................................
void TPolynomMember::Combine()  {
  const size_t count = FMembers.Count();
  for( size_t i = 0; i < count; i++ )  {
    if( FMembers.IsNull(i) )  continue;
    for( size_t j=i+1; j < count; j++ )  {
      if( FMembers.IsNull(j) )  continue;
      if( FMembers[j].Id == FMembers[i].Id )  {
        FMembers[i].Extent += FMembers[j].Extent;
        FMembers.NullItem(j);
      }
    }
  }
  FMembers.Pack();
  FMembers.QuickSorter.SortSF(FMembers, _PMembersSort);
}
//..............................................................................
void TPolynomMember::Mul(const TPolynomMember& P) {
  FMult = FMult*P.FMult;
  for( size_t i=0; i < P.FMembers.Count(); i++ )
    AddMember() = P.FMembers[i];
  Combine();
}
//----------------------------------------------------------------------------//
// TPolynom function bodies
//----------------------------------------------------------------------------//
void TPolynom::SetThreshold(double Threshold)  {
  if( FPolySort && FEvaluator && (FMembers.Count() > 0) )  {
    FMembers.QuickSorter.SortSF(FMembers, FPolySort);
    const double v = FEvaluator(FMembers[0]);
    if( v == 0 )  return;
    size_t S = FMembers.Count();
    for( size_t i=0; i < FMembers.Count(); i++ )  {
      if( FEvaluator(FMembers[i])/v < Threshold )  {
        S = i;
        break;
      }
    }
    FMembers.Shrink(S);
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "Cannot compact the polynom!");
}
//..............................................................................
void TPolynom::SetSize(size_t S)  {
  if( FMembers.Count() < S )  return;
  if( FPolySort != NULL )  {
    FMembers.QuickSorter.SortSF(FMembers, FPolySort);
    FMembers.Shrink(S);
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "The evaluation function is not defined");
}
//..............................................................................
TPolynom* TPolynom::PowX(size_t Members, size_t p) const {
  if( p == 1 )
    return &( *(new TPolynom(FAddEvaluator, FEvaluator, FPolySort)) = *this );
  if( p == 2 )
    return this->Qrt();

  TPolynom* R = PowX((short)(sqrt((double)(Members*2)+1)), (short)(p/2));
  if( p%2 )
    Members /= FMembers.Count();
  if( Members < 5 )  Members = 5;
  if( R->FMembers.Count() > Members )
    R->SetSize(Members);
  TPolynom* C = R->Qrt();
  delete R;
  if( (p%2) != 0 )  {
    R = C->Mul(*this);
    delete C;
    return R;
  }
  return C;
}
//..............................................................................
TPolynom* TPolynom::Pow(short p) const {
  if( p == 1 )
    return &( *(new TPolynom(FAddEvaluator, FEvaluator, FPolySort)) = *this );
  if( p == 2 )
    return this->Qrt();

  TPolynom* R = Pow((short)(p/2));
  TPolynom* C = R->Qrt();
  delete R;
  if( (p%2) != 0 )  {
    R = C->Mul(*this);
    delete C;
    return R;
  }
  return C;
}
//..............................................................................
void TPolynom::Combine()  {
  const size_t cnt = FMembers.Count();
  for( size_t i=0; i < cnt; i++ )  {
    if( FMembers.IsNull(i) )  continue;
    for( size_t j=i+1; j < cnt; j++ )  {
      if( FMembers.IsNull(j) )  continue;
      if( FMembers[i] == FMembers[j] )  {
        FMembers[i].IncMult(FMembers[j].GetMult());
        FMembers.NullItem(j);
      }
    }
  }
  FMembers.Pack();
}
//..............................................................................
olxstr TPolynom::Values()  {
  olxstr T;
  for( size_t i=0; i < FMembers.Count(); i++ )
    T << FMembers[i].Values() <<  "; ";
  return T;
}
//..............................................................................
TPolynom* TPolynom::Mul(const TPolynom& P) const {
  TPolynom *NP = new TPolynom(FAddEvaluator, FEvaluator, FPolySort);
  for( size_t i=0; i < FMembers.Count(); i++ )  {
    for( size_t j=0; j < P.FMembers.Count(); j++ )  {
      TPolynomMember* PM2 = new TPolynomMember;
      *PM2 = P.FMembers[j];
      PM2->Mul(FMembers[i]);
      if( FAddEvaluator != NULL )  {
        if( FAddEvaluator(*PM2) )
          NP->FMembers.Add(PM2);
        else
          delete PM2;
      }
      else
        NP->FMembers.Add(PM2);
    }
  }
  NP->Combine();
  return NP;
}
//..............................................................................
TPolynom* TPolynom::Qrt() const {
  TPolynom *NP = new TPolynom(FAddEvaluator, FEvaluator, FPolySort);
  NP->FMembers.SetCapacity(FMembers.Count() + (FMembers.Count()-1)*FMembers.Count()/2);
  for( size_t i=0; i < FMembers.Count(); i++ )  {
    TPolynomMember* PM1 = new TPolynomMember;
    *PM1 = FMembers[i];
    for( size_t j=0; j < PM1->Members().Count(); j++ )
      PM1->Members()[j].Extent *= 2;

    PM1->MulMult( PM1->GetMult() );
    if( FAddEvaluator != NULL )  {
      if( FAddEvaluator(*PM1) )
        NP->FMembers.Add(PM1);
      else
        delete PM1;
    }
    else
      NP->FMembers.Add(PM1);
  }
  for( size_t i=0; i < FMembers.Count(); i++ )  {
    for( size_t j=i+1; j < FMembers.Count(); j++ )  {
      TPolynomMember* PM1 = new TPolynomMember;
      *PM1 = FMembers[j];
      PM1->MulMult( FMembers[i].GetMult()*2 );
      for( size_t k=0; k < FMembers[i].Members().Count(); k++ )
        PM1->AddMember() = FMembers[i].Members()[k];

      PM1->Combine();
      if( FAddEvaluator != NULL )  {
        if( FAddEvaluator(*PM1) )
          NP->FMembers.Add(PM1);
        else
          delete PM1;
      }
      else
        NP->FMembers.Add(PM1);
    }
  }
  NP->Combine();
  return NP;
}
//..............................................................................


//---------------------------------------------------------------------------//
// Binominal polynomial routins
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef __olx_sdl_poly_H
#define __olx_sdl_poly_H
#include <math.h>
#include <stdlib.h>
#include "typelist.h"
//---------------------------------------------------------------------------
BeginEsdlNamespace()

struct TSPoint  {
  double X, Y;
  TSPoint(const double& a, const double& b) : X(a), Y(b) {  }
  static int SPointsSortA(const TSPoint& I, const TSPoint& I1);
  static int SPointsSortB(const TSPoint& I, const TSPoint& I1);
};

typedef TTypeList<TSPoint> TPolySerie;

struct TPMember  {
  size_t Id;
  int Extent;
  const void* Data;
  bool Used;
  TPMember() : Id(0), Extent(1), Data(NULL) {}
  bool operator == (const TPMember& P) const {
    if( P.Id != Id )          return false;
    if( P.Extent != Extent )  return false;
    if( P.Data != Data )      return false;
    return true;
  }
  TPMember& operator = (const TPMember& P)  {
    Id = P.Id;
    Extent = P.Extent;
    Data = P.Data;
    return *this;
  }
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class TPolynomMember  {
  TTypeList<TPMember> FMembers;
  double FMult;
protected:
  void SetUsed(bool Value)  {
    for( size_t i=0; i < FMembers.Count(); i++ )
      FMembers[i].Used = Value;
  }
public:
  TPolynomMember() : FMult(1) {}
  ~TPolynomMember() {}
  TPMember& AddMember()  {
    TPMember& P = FMembers.AddNew();
    P.Id = FMembers.Count()-1;
    return P;
  }
  
  bool operator == (const TPolynomMember& P) const  {
    if( FMembers.Count() != P.FMembers.Count() )
      return false;
    for( size_t i=0; i < FMembers.Count(); i++ )
      if( !(FMembers[i] == P.FMembers[i]) )
        return false;
    return true;
  }

  TPolynomMember& operator = (const TPolynomMember& P)  {
    FMembers.Clear();
    FMult = P.FMult;
    FMembers.AddListA(P.FMembers);
    return *this;
  }
  TTypeList<TPMember>& Members() {  return FMembers;  }
  const TTypeList<TPMember>& Members() const {  return FMembers;  }
  void Mul(const TPolynomMember& P);
  double GetMult() const {  return FMult;  }
  void IncMult(double c) { FMult +=c; }
  void MulMult(double c) { FMult *=c; }
  void Combine();

  olxstr Values() const {
    olxstr T;
    if( FMult != 1)  {
      T = (double)FMult;
      T << '*';
    }
    for( size_t i = 0; i < FMembers.Count(); i++ )  {
      T << (char)('a'+FMembers[i].Id);
      if( FMembers[i].Extent != 1 )
        T << '^' << FMembers[i].Extent;
    }
    return T;
  }
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef double (Evaluator)(const TPolynomMember& P);
typedef bool (AddEvaluator)(const TPolynomMember& P);
typedef int (PolySort)(const TPolynomMember& P, const TPolynomMember& P1);
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class TPolynom: public IEObject  {
  TTypeList<TPolynomMember> FMembers;
protected:
  void Clear()  {  FMembers.Clear();  }
  void Combine();
  AddEvaluator *FAddEvaluator;
  Evaluator *FEvaluator;
  PolySort *FPolySort;
  void SetSize(size_t s);
public:
  TPolynom(AddEvaluator* av, Evaluator* v, PolySort* v1) :
    FAddEvaluator(av),
    FEvaluator(v),
    FPolySort(v1)  
  {}

  ~TPolynom() {}
  TPolynomMember& AddMember()  {  return FMembers.AddNew();  }
  TTypeList<TPolynomMember>& Members()  {  return FMembers;  }
  const TTypeList<TPolynomMember>& Members() const {  return FMembers;  }
  TPolynom* Pow(short p) const;
  void SetThreshold(double Threshold);
  TPolynom* PowX(size_t MembersToLeave, size_t p) const;
  TPolynom* Mul(const TPolynom& P) const;
  
  void MulSelf(const TPolynom& P);
  
  TPolynom&  operator = (const TPolynom& P)  {
    FMembers.Clear();
    FMembers.AddListA(P.FMembers);
    return *this;
  }
  
  TPolynom* Qrt() const;
  void SetEvaluator(Evaluator *v){ FEvaluator = v; };
  olxstr Values();
};

EndEsdlNamespace()
#endif

#ifndef __olx__cl_idustribution_H
#define __olx__cl_idustribution_H
#include "chembase.h"
#include "chemdata.h"
#include "poly.h"

BeginChemNamespace()

struct TIsotopeData  {  // tree node
  TTypeList<TIsotopeData> Children;
  double M, W;
  TIsotopeData() : M(0), W(1) {}
  ~TIsotopeData() {}
  void Evail(TPolySerie& S, double &eM, double &eW);
};
//---------------------------------------------------------------------------
// calculates itsotopic distribution of a given formula; use TIPattern instead
//---------------------------------------------------------------------------
class TIDistribution {
  TTypeList<TPolynom> Polynomes;
  TIsotopeData Root;
  double Threshold;
  size_t MaxPoints;
protected:
  void Evail(const TPolynomMember& PM, double& M, double& W) const;
  TPolySerie* PolynomToSerie(const TPolynom& P);
public:
  TIDistribution();
  ~TIDistribution() {}
  void AddIsotope(const cm_Element& elm, size_t count);
  void Calc(TPolySerie& S);
  DefPropP(double, Threshold)
  void SetMaxPoints(size_t v)  {  MaxPoints = v;  }
  static void CombineSerie(TPolySerie& serie, double threshold);
};

EndChemNamespace()
#endif

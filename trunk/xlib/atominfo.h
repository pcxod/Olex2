#ifndef atominfoH
#define atominfoH
#include "chembase.h"
//#include "estring.h"
#include "elist.h"
#include "estrlist.h"

BeginChemNamespace()

const  short iHydrogenIndex   = 0,
             iBoronIndex   = 4,
             iCarbonIndex   = 5,
             iNitrogenIndex   = 6,
             iOxygenIndex     = 7,
             iFluorineIndex   = 8,
             iSiliconIndex    = 13,
             iPhosphorusIndex = 14,
             iSulphurIndex    = 15,
             iChlorineIndex   = 16,
             iQPeakIndex      = 104,
             iDeuteriumIndex      = 105;

class TIsotope: public ACollectionItem  {
  double Mr, W;
public:
  TIsotope()  {  Mr = W = 0;  }
  TIsotope(double mr, double w)  {  Mr = mr;  W = w;  }

  DefPropP(double, Mr)
  DefPropP(double, W)
};

typedef TTypeList<TIsotope> TIsotopeList;

class TBasicAtomInfo: public ACollectionItem  {
  TIsotopeList* Isotopes;  // list of isotopes
  olxstr Symbol,  // atom symbol, e.g. Mn
           Name;    // atom name e.g. carbon
  double Mr,    // molecular weight
         Rad,    // radius for bonds determination (WVan der Waals)
         Rad1,
         Rad2;
  int    DefColor;   // default colour
  short  Index;    // index of the record in the list; for internal use
  double Summ;
public:
  TBasicAtomInfo();
  virtual ~TBasicAtomInfo();

  inline int IsotopeCount()  const {  return (Isotopes==NULL) ? 0 : Isotopes->Count(); }
  inline TIsotope& GetIsotope(int index)  const  {  return Isotopes->Item(index);  }
  TIsotope& NewIsotope();

  inline bool operator == (const TBasicAtomInfo& bai )  const {  return Index == bai.Index;  }
  inline bool operator == (const short index)           const {  return Index == index;  }
  inline bool operator != (const TBasicAtomInfo& bai )  const {  return Index != bai.Index;  }
  inline bool operator != (const short index)           const {  return Index != index;  }
  inline bool operator < (const TBasicAtomInfo& bai )  const {  return Index < bai.Index;  }
  inline bool operator < (const short index)           const {  return Index < index;  }
  inline bool operator <= (const TBasicAtomInfo& bai )  const {  return Index <= bai.Index;  }
  inline bool operator <= (const short index)           const {  return Index <= index;  }
  inline bool operator > (const TBasicAtomInfo& bai )  const {  return Index > bai.Index;  }
  inline bool operator > (const short index)           const {  return Index > index;  }
  inline bool operator >= (const TBasicAtomInfo& bai )  const {  return Index >= bai.Index;  }
  inline bool operator >= (const short index)           const {  return Index >= index;  }

  DefPropC(olxstr, Symbol)
  DefPropC(olxstr, Name)

  DefPropP(double, Mr)
  DefPropP(double, Rad)
  DefPropP(double, Rad1)
  DefPropP(double, Rad2)

  DefPropP(int, Index)
  DefPropP(int, DefColor)

  DefPropP(double, Summ)

  olxstr StrRepr() const;
};
//---------------------------------------------------------------------------
class TAtomsInfo: public IEObject  {
private:
  TTypeList<TBasicAtomInfo> Data;
  // parses a string like BRhClO intp B, Rh, Cl and O; works for capitalised symbols only
  // and will not know how to treat BRH, but BRh
  void ParseSimpleElementStr(const olxstr& str, TStrList& toks) const;
public:
  TAtomsInfo(const olxstr& filename);
  virtual ~TAtomsInfo();
  inline int Count()  const  {  return Data.Count();  }
  void SaveToFile(const olxstr& fileName) const;
  // returns correpondinag value of the AtomsInfo list
  inline TBasicAtomInfo& GetAtomInfo(int index) const {  return Data[index];  }
  // returns correpondinag value of the AtomsInfo list
  TBasicAtomInfo* FindAtomInfoBySymbol(const olxstr& Symbol) const;
  // returns correpondinag value of the AtomsInfo list, Str can be "C10a" etc
  TBasicAtomInfo* FindAtomInfoEx(const olxstr& Str) const;
  // a simple check exact match is expected (case insesitive)
  inline bool IsElement(const olxstr& S) const  {  return (FindAtomInfoBySymbol(S) != NULL);  }
  // checks if p is an element symbol, will correctly distinguis "C " and "Cd"
  inline bool IsAtom(const olxstr &C)     const {  return (FindAtomInfoEx(C) != NULL);  }
  /* parses a string like C37H41P2BRhClO into a list of element names and theur
    count
  */
  void ParseElementString(const olxstr& su, TTypeList<AnAssociation2<olxstr, int> >& res) const;
};

EndChemNamespace()
#endif

#ifndef crsFileH
#define crsFileH

#include "xfiles.h"

// STOE "P4P" file
BeginXlibNamespace()

class TCRSFile : public TBasicCFile  {
  TVectorDList Faces;
  double Radiation;
  olxstr Sfac, Unit;
  bool SGInitialised;
public:
  TCRSFile();
  virtual ~TCRSFile();

  const double GetRadiation()  const  {  return Radiation;  }

  class TSpaceGroup* GetSG();

  inline const olxstr& GetSfac()  const  {  return Sfac;  }
  inline const olxstr& GetUnit()  const  {  return Unit;  }

  inline int FacesCount()  const  {  return Faces.Count();  }
  inline const TVectorD& GetFace(int i)  const  {  return Faces[i];  }

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *);
  virtual IEObject* Replicate() const {  return new TCRSFile();  }
  virtual void DeleteAtom(TCAtom *A)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
};

EndXlibNamespace()
#endif

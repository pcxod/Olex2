#ifndef oxmFileH
#define oxmFileH

#include "gxapp.h"

// Olex2 model file
BeginGxlNamespace()

class TOXMFile : public TBasicCFile  {
  TGXApp& gxapp;
public:
  TOXMFile(TGXApp& app) : gxapp(app) {  }
  virtual ~TOXMFile() {  }

  virtual void SaveToStrings(TStrList& Strings)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual void LoadFromStrings(const TStrList& Strings)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual bool IsNative() const {  return true;  }
  virtual bool Adopt(TXFile&)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual void LoadFromFile(const olxstr& fn)  {
    gxapp.LoadModel(fn);
    RefMod.Assign( gxapp.XFile().GetRM(), true);
    FileName = fn;
  }
  virtual void SaveToFile(const olxstr& fn)  {
    gxapp.SaveModel(fn);
  }
  virtual IEObject* Replicate() const {  return new TOXMFile(gxapp);  }
};

EndGxlNamespace()

#endif
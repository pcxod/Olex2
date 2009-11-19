// need this file to resolve includes and dependencies... not to be included first :)
#ifndef __olx_rm_base_H
#define __olx_rm_base_H
#include "xbase.h"
BeginXlibNamespace()

class IXVarReferencer;
class IXVarReferencerContainer;

class IXVarReferencer {
public:
  virtual size_t VarCount() const = 0;
  virtual struct XVarReference* GetVarRef(size_t i) const = 0;
  //virtual XVarReference* GetVarRef(size_t i) const = 0;
  virtual olxstr GetVarName(size_t i) const = 0;
  virtual void SetVarRef(size_t i, XVarReference* var_ref) = 0;
  virtual double GetValue(size_t var_index) const = 0;
  virtual void SetValue(size_t var_index, const double& val) = 0;
  virtual const IXVarReferencerContainer& GetParentContainer() const = 0;
  virtual bool IsValid() const = 0;
  virtual size_t GetReferencerId() const;
  virtual size_t GetPersistentId() const;
  virtual olxstr GetIdName() const = 0;
};

class IXVarReferencerContainer  {
public:
  // returns an object id and the name of id, like 'atom', 'dfix', 'basf', etc
  virtual olxstr GetIdName() const = 0;
  virtual size_t GetIdOf(const IXVarReferencer& vr) const = 0;
  virtual size_t GetPersistentIdOf(const IXVarReferencer& vr) const = 0;
  virtual size_t ReferencerCount() const = 0;
  virtual IXVarReferencer& GetReferencer(size_t id) const = 0;
};

EndXlibNamespace()
#endif

/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_constlist_H
#define __olx_sdl_constlist_H
#include "ebase.h"
#undef GetObject
BeginEsdlNamespace()

template <class obj_t>
class const_obj {
protected:
  mutable olx_ptr<obj_t> *p;
  static void throw_invalid(const char* file, const char* function, int line) {
    TExceptionBase::ThrowFunctionFailed(file, function, line, "uninitialised object");
  }
public:
  const_obj(const const_obj &l) : p(l.p == NULL ? NULL : l.p->inc_ref())  {}
  const_obj(obj_t *l) : p (new olx_ptr<obj_t>(l)) {}
  const_obj(obj_t &l) : p(new olx_ptr<obj_t>(new obj_t)) {
    p->p->TakeOver(l);
  }
  virtual ~const_obj()  {
    if( p != NULL && --p->ref_cnt == 0 ) {
      delete p->p;
      delete p;
    }
  }
  //operator const obj_t &() const {  return GetObject();  }
  const obj_t& GetObject() const {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    return *p->p;
  }
  const_obj &operator = (const const_obj &a) {
    if( p != NULL && --p->ref_cnt == 0 )  {
      delete p->p;
      delete p;
    }
    p = a.p->inc_ref();
    return *this;
  }
  // the caller must delete the container after this call
  obj_t &Release() const {
    if( p == NULL )
      throw_invalid(__POlxSourceInfo);
    obj_t *rv;
    if( --p->ref_cnt == 0 )  {
      rv = p->p;
      delete p;
    }
    else
      rv = new obj_t(*p->p);
    p = NULL;
    return *rv;
  }
  bool IsValid() const {  return p != NULL;  }
};

template <class cont_t, typename item_t>
class const_list : public const_obj<cont_t> {
  typedef const_obj<cont_t> _parent_t;
public:
  const_list(const const_list &l) : _parent_t(l) {}
  const_list(cont_t *l) : _parent_t(l) {}
  const_list(cont_t &l) : _parent_t(l) {}
  const item_t &operator [] (size_t i) const {  return (*_parent_t::p->p)[i];  }
  size_t Count() const {  return _parent_t::p == NULL ? 0 : _parent_t::p->p->Count();  }
  bool IsEmpty() const {  return Count() == 0;  }
  const_list& operator = (const const_list &a) {
    _parent_t:: operator = (a);
    return *this;
  }
};

template <class dict_t, typename key_t, typename val_t>
class const_dict : public const_obj<dict_t> {
  typedef const_obj<dict_t> _parent_t;
public:
  const_dict(const const_dict &l) : _parent_t(l) {}
  const_dict(dict_t *l) : _parent_t(l) {}
  const_dict(dict_t &l) : _parent_t(l) {}
  const key_t &GetKey(size_t i) const {  return _parent_t::p->p->GetKey(i);  }
  const val_t &GetValue(size_t i) const {  return _parent_t::p->p->GetValue(i);  }
  size_t Count() const {  return _parent_t::p == NULL ? 0 : _parent_t::p->p->Count();  }
  bool IsEmpty() const {  return Count() == 0;  }
  const_dict& operator = (const const_dict &a) {
    _parent_t:: operator = (a);
    return *this;
  }
};

EndEsdlNamespace()
#endif

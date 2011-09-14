/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* primitive directional lists, optimised for sequential access, the right cleanup class must be
chosen to avoid memory leaks! */
#ifndef __olx_sdl_linked_indexable_list_H
#define __olx_sdl_linked_indexable_list_H
#include "linked_list.h"
BeginEsdlNamespace()

template <typename T, class cleanupClass=DummyCleanup<T> >
class TUDTypeList : public TLinkedList<T, cleanupClass> {
  typedef TLinkedList<T, cleanupClass> parent_t;
  mutable size_t pos;
  mutable typename parent_t::Entry *cur;
public:
  TUDTypeList() : pos(InvalidIndex), cur(NULL) {}
  virtual ~TUDTypeList()  {  Clear();  }
  void Clear()  {
    parent_t::Clear();
    cur = NULL;
    pos = InvalidIndex;
  }
  TUDTypeList& Reset()  {
    cur = NULL;
    pos = InvalidIndex;
    return *this;
  }
  T& operator [] (size_t ind) const {
    if( ind >= parent_t::count )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid index");
    if( pos == (ind-1) )
      return Next();
    else if( ind == 0 )  {
      cur = parent_t::first;
      pos = 0;
      return cur->data;
    }
    pos = 0;
    cur = parent_t::first;
    while( ind-- != 1 )  {
      cur = cur->next;
      pos++;
    }
    return cur->data;
  }
  T& Add(T v)  {
    pos++;
    return parent_t::Add(v);
  }
protected:
  bool HasNext() const {
    return (cur == NULL ? parent_t::first : cur->next) != NULL;
  }
  T& Next() const {  
    if( cur != NULL )
      cur = cur->next;
    else
      cur = parent_t::first;
    if( cur == NULL )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "end of the list");
    pos++;
    return cur->data;
  }
};

EndEsdlNamespace()
#endif

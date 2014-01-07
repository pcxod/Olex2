/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_ptr_H
#define __olx_sdl_ptr_H

template <typename ptr> struct olx_ptr_  {
  ptr* p;
  int ref_cnt;
  olx_ptr_(ptr* _p) : p(_p), ref_cnt(1)  {}
  olx_ptr_* inc_ref()  {  ref_cnt++;  return this;  }
  template <bool is_array> int dec_ref() {
    int rc = --ref_cnt;
    if (rc <= 0) {
      if (p != NULL) {
        if (is_array)
          delete [] p;
        else
          delete p;
      }
      delete this;
    }
    return rc;
  }
};

template <typename ptr> struct olx_object_ptr  {
  olx_ptr_<ptr>* p;
  olx_object_ptr() { p = new olx_ptr_<ptr>(NULL); }
  olx_object_ptr(ptr* _p) { p = new olx_ptr_<ptr>(_p); }
  olx_object_ptr(const olx_object_ptr& _p) : p(_p.p->inc_ref())  {}
  ~olx_object_ptr()  { p->template dec_ref<false>(); }
  olx_object_ptr& operator = (const olx_object_ptr& _p)  {
    p->template dec_ref<false>();
    p = _p.p->inc_ref();
    return *this;
  }
  olx_object_ptr& operator = (ptr *p_)  {
    if (--p->ref_cnt <= 0 && p->p != NULL)
      delete p->p;
    p->p = p_;
    p->ref_cnt = 1;
    return *this;
  }
  ptr& operator ()() const {  return *p->p;  }
  bool is_valid() const { return p->p != NULL; }
  operator ptr& () const {  return *p->p;  }
  // releases the object from ALL references
  ptr &release() const {
    ptr *p_ = p->p;
    p->p = NULL;
    return *p_;
  }
};

template <typename ptr> struct olx_array_ptr  {
  olx_ptr_<ptr>* p;
  olx_array_ptr() {  p = new olx_ptr_<ptr>(NULL);  }
  olx_array_ptr(ptr* _p) {  p = new olx_ptr_<ptr>(_p);  }
  olx_array_ptr(const olx_array_ptr& _p) : p(_p.p->inc_ref())
  {}
  olx_array_ptr(size_t sz) : p(new olx_ptr_<ptr>(new ptr[sz]))
  {}

  ~olx_array_ptr() { p->template dec_ref<true>(); }
  olx_array_ptr& operator = (const olx_array_ptr& _p)  {
    p->template dec_ref<true>();
    p = _p.p->inc_ref();
    return *this;
  }
  olx_array_ptr& operator = (ptr *p_)  {
    if (--p->ref_cnt <= 0 && p->p != NULL)
      delete [] p->p;
    p->p = p_;
    p->ref_cnt = 1;
    return *this;
  }
  bool is_null() const { return p->p == NULL; }
  bool is_valid() const { return p->p != NULL; }
  ptr* operator ()() const {  return p->p;  }
  operator ptr* () const {  return p->p;  }
  // releases the array from ALL references
  ptr *release() const {
    ptr *p_ = p->p;
    p->p = NULL;
    return p_;
  }
  static olx_array_ptr copy(const ptr *p, size_t sz) {
    ptr *rv = new ptr[sz];
    memcpy(rv, p, sz*sizeof(ptr));
    return olx_array_ptr(rv);
  }
};

struct olx_ref {
  template <class P>
  static P& get(P* p)  {  return *p;  }
  static char *get(char *p)  {  return p;  }
  static wchar_t *get(wchar_t *p)  {  return p;  }
  template <class P>
  static const P& get(const P* p)  {  return *p;  }
  static const char *get(const char *p)  {  return p;  }
  static const wchar_t *get(const wchar_t *p)  {  return p;  }
  template <class P>
  static P& get(P& p)  {  return p;  }
  template <class P>
  static const P& get(const P& p)  {  return p;  }
};

struct olx_ptr {
  template <class P>
  static P* get(P* p)  {  return p;  }
  template <class P>
  static const P* get(const P* p)  {  return p;  }
  template <class P>
  static P* get(P& p)  {  return &p;  }
  template <class P>
  static const P* get(const P& p)  {  return &p;  }

  template <typename T>
  static olx_array_ptr<T> copy(const T *d, size_t sz) {
    return olx_array_ptr<T>::copy(d, sz);
  }
};

// this object could be safelly passed to/from dll function calls 
template <typename ptr> struct olx_dll_ptr {
protected:
  struct olx_dll_ptr_ {
    void *(*allocator_func)(size_t sz);
    void (*deallocator_func)(void *p);
    ptr* p;
    int ref_cnt;
  
    void init() {
      allocator_func = &olx_dll_ptr_::alloc;
      deallocator_func = &olx_dll_ptr_::dealloc;
    }
    olx_dll_ptr_()
      : p(NULL), ref_cnt(1)
    {
      init();
    }

    olx_dll_ptr_(size_t cnt) : p(alloc(cnt)), ref_cnt(1) {
      init();
    }

    olx_dll_ptr_(ptr *p_) : p(p_), ref_cnt(1) {
      init();
    }

    olx_dll_ptr_* inc_ref() { ref_cnt++;  return this; }
    int dec_ref() {
      int rc = --ref_cnt;
      if (rc <= 0) {
        if (p != NULL) {
          (*deallocator_func)(p);
        }
        delete this;
      }
      return rc;
    }
    static void *alloc(size_t sz) { return malloc(sz); }
    static void dealloc(void *p) { free(p); }
    static void *operator new(size_t n) { return alloc(n); }
    static void operator delete(void *p) {
      (*((olx_dll_ptr_*)p)->deallocator_func)(p);
    }
  };
public:
  olx_dll_ptr_* p;
  olx_dll_ptr() { p = new olx_dll_ptr_(); }
  olx_dll_ptr(size_t sz) { p = new olx_dll_ptr_(sz); }
  olx_dll_ptr(ptr *p_) { p = new olx_dll_ptr_(p_); }
  olx_dll_ptr(const olx_dll_ptr& _p)
    : p(_p.p->inc_ref())
  {}
  ~olx_dll_ptr()  { p->dec_ref(); }
  olx_dll_ptr& operator = (const olx_dll_ptr& p_)  {
    p->dec_ref();
    p = p_.p->inc_ref();
    return *this;
  }
  olx_dll_ptr& operator = (ptr *p_) {
    p->dec_ref();
    p = new olx_dll_ptr(p_);
    return *this;
  }
  ptr* operator ()() const { return p->p; }
  bool is_valid() const { return p->p != NULL;}
  operator ptr* () const { return p->p; }

  static olx_dll_ptr copy(const ptr *p, size_t sz) {
    size_t tsz = sz*sizeof(ptr);
    ptr *rv = (ptr *)olx_dll_ptr_::alloc(tsz);
    memcpy(rv, p, tsz);
    return olx_dll_ptr(rv);
  }
};


#endif

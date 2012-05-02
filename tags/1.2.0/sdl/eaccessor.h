/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_eaccessor_H
#define __olx_sdl_eaccessor_H

struct DummyAccessor  {
  template <typename item_t>
  const item_t& operator ()(const item_t& item) const { return item; }
  template <typename item_t>
  item_t& operator ()(item_t& item) const { return item; }
};

template <typename item_t>
struct TDirectAccessor  {
  typedef item_t return_type;

  const item_t& operator ()(const item_t& item) const { return item; }
  item_t& operator ()(item_t& item) const { return item; }
};

template <class list_t>
TDirectAccessor<typename list_t::list_item_type>
  ListAccessor(const list_t &t)
{
  return TDirectAccessor<typename list_t::list_item_type>();
}
struct DereferenceAccessor  {
  template <typename item_t>
  const item_t& operator ()(const item_t* item) const { return *item; }
  template <typename item_t>
  item_t& operator ()(item_t* item) const { return *item; }
};

template <typename item_t>
struct TDereferenceAccessor  {
  typedef item_t return_type;

  const item_t& operator ()(const item_t* item) const { return *item; }
  item_t& operator ()(item_t* item) const { return *item; }
};

template <typename CastType> struct CCastAccessor  {
  typedef CastType return_type;
  template <typename Item>
  static const CastType& Access(const Item& item)  {
    return (const CastType&)item;
  }
  template <typename Item>
  static CastType& Access(Item& item)  {
    return (CastType&)item;
  }
  template <typename Item>
  static const CastType* Access(const Item* item)  {
    return (const CastType*)item;
  }
  template <typename Item>
  static CastType* Access(Item* item)  {
    return (CastType*)item;
  }

  template <typename Item>
  const CastType& operator ()(const Item& item) const {
    return (const CastType&)item;
  }
  template <typename Item>
  CastType& operator ()(Item& item) const {
    return (CastType&)item;
  }
  template <typename Item>
  const CastType* operator ()(const Item* item) const {
    return (const CastType*)item;
  }
  template <typename Item>
  CastType* operator ()(Item* item) const {
    return (CastType*)item;
  }
};
template <typename CastType> struct StaticCastAccessor  {
  typedef CastType return_type;
  template <typename Item>
  static const CastType& Access(const Item& item)  {
    return static_cast<const CastType&>(item);
  }
  template <typename Item>
  static CastType& Access(Item& item)  {
    return static_cast<CastType&>(item);
  }
  template <typename Item>
  static const CastType* Access(const Item* item)  {
    return static_cast<const CastType*>(item);
  }
  template <typename Item>
  static CastType* Access(Item* item)  {
    return static_cast<CastType*>(item);
  }

  template <typename Item>
  const CastType& operator ()(const Item& item) const {
    return static_cast<const CastType&>(item);
  }
  template <typename Item>
  CastType& operator ()(Item& item) const {
    return static_cast<CastType&>(item);
  }
  template <typename Item>
  const CastType* operator ()(const Item* item) const {
    return static_cast<const CastType*>(item);
  }
  template <typename Item>
  CastType* operator ()(Item* item) const {
    return static_cast<CastType*>(item);
  }
};
template <typename CastType> struct DynamicCastAccessor  {
  typedef CastType return_type;
  template <typename Item>
  static const CastType& Access(const Item& item)  {
    return dynamic_cast<const CastType&>(item);
  }
  template <typename Item>
  static CastType& Access(Item& item)  {
    return dynamic_cast<CastType&>(item);
  }
  template <typename Item>
  static const CastType* Access(const Item* item)  {
    return dynamic_cast<const CastType*>(item);
  }
  template <typename Item>
  static CastType* Access(Item* item)  {
    return dynamic_cast<CastType*>(item);
  }

  template <typename Item>
  const CastType& operator ()(const Item& item) const {
    return dynamic_cast<const CastType&>(item);
  }
  template <typename Item>
  CastType& operator ()(Item& item) const {
    return dynamic_cast<CastType&>(item);
  }
  template <typename Item>
  const CastType* operator ()(const Item* item) const {
    return dynamic_cast<const CastType*>(item);
  }
  template <typename Item>
  CastType* operator ()(Item* item) const {
    return dynamic_cast<CastType*>(item);
  }
};

struct FunctionAccessor {
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (base_t::*func)() const;
    ConstFunctionAccessorT_(rv_t (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t operator ()(const item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(base_t::*func)() const;
    ConstFunctionAccessorR_(rv_t & (base_t::*_func)() const) : func(_func)  {}
    template <typename item_t> rv_t & operator ()(const item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct ConstFunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(base_t::*func)() const;
    ConstFunctionAccessorCR_(const rv_t & (base_t::*_func)() const)
      : func(_func)  {}
    template <typename item_t>
    const rv_t & operator ()(const item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };

  template <typename rv_t, typename base_t> struct FunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (base_t::*func)();
    FunctionAccessorT_(rv_t (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t operator()(item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(base_t::*func)();
    FunctionAccessorR_(rv_t & (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> rv_t & operator()(item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };
  template <typename rv_t, typename base_t> struct FunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(base_t::*func)();
    FunctionAccessorCR_(const rv_t & (base_t::*_func)()) : func(_func)  {}
    template <typename item_t> const rv_t & operator()(item_t& it) const {
      return (olx_ref::get(it).*func)();
    }
  };

  template <typename rv_t, typename item_t>
  struct StaticFunctionAccessorT_  {
    typedef rv_t return_type;
    rv_t (*func)(const item_t &);
    StaticFunctionAccessorT_(rv_t (*_func)(const item_t &))
      : func(_func)
    {}
    template <typename item_t_t>
    rv_t operator()(item_t_t& it) const {
      return (*func)(olx_ref::get(it));
    }
  };
  template <typename rv_t, typename item_t>
  struct StaticFunctionAccessorR_  {
    typedef rv_t return_type;
    rv_t &(*func)(const item_t &);
    StaticFunctionAccessorR_(rv_t & (*_func)(const item_t &))
      : func(_func)
    {}
    template <typename item_t_t>
    rv_t & operator()(item_t_t& it) const {
      return (*func)(olx_ref::get(it));
    }
  };
  template <typename rv_t, typename item_t>
  struct StaticFunctionAccessorCR_  {
    typedef rv_t return_type;
    const rv_t &(*func)(const item_t &);
    StaticFunctionAccessorCR_(const rv_t & (*_func)(const item_t &))
      : func(_func)
    {}
    template <typename item_t_t>
    const rv_t & operator()(item_t_t& it) const {
      return (*func)(olx_ref::get(it));
    }
  };

  template <typename rv_t, typename base_t> static
  ConstFunctionAccessorT_<rv_t,base_t> MakeConst(
    rv_t (base_t::*func)() const)
  {
    return ConstFunctionAccessorT_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  ConstFunctionAccessorR_<rv_t,base_t> MakeConst(
    rv_t & (base_t::*func)() const)
  {
    return ConstFunctionAccessorR_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  ConstFunctionAccessorCR_<rv_t,base_t> MakeConst(
    const rv_t &(base_t::*func)() const)
  {
    return ConstFunctionAccessorCR_<rv_t,base_t>(func);
  }

  template <typename rv_t, typename base_t> static
  FunctionAccessorT_<rv_t,base_t> Make(rv_t (base_t::*func)())  {
    return FunctionAccessorT_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  FunctionAccessorR_<rv_t,base_t> Make(rv_t & (base_t::*func)())  {
    return FunctionAccessorR_<rv_t,base_t>(func);
  }
  template <typename rv_t, typename base_t> static
  FunctionAccessorCR_<rv_t,base_t> Make(const rv_t & (base_t::*func)())  {
    return FunctionAccessorCR_<rv_t,base_t>(func);
  }

  template <typename rv_t, typename item_t> static
  StaticFunctionAccessorT_<rv_t,item_t>
  MakeStatic(rv_t (*func)(const item_t &))  {
    return StaticFunctionAccessorT_<rv_t,item_t>(func);
  }
  template <typename rv_t, typename item_t> static
  StaticFunctionAccessorR_<rv_t,item_t>
  MakeStatic(rv_t & (*func)(const item_t &))  {
    return StaticFunctionAccessorR_<rv_t,item_t>(func);
  }
  template <typename rv_t, typename item_t> static
  StaticFunctionAccessorCR_<rv_t,item_t>
  MakeStatic(const rv_t & (*func)(const item_t &))  {
    return StaticFunctionAccessorCR_<rv_t,item_t>(func);
  }
};

template <typename item_t, class data_list_t>
struct ConstIndexAccessor  {
  typedef item_t return_type;
  const data_list_t &data;
  ConstIndexAccessor(const data_list_t &data_) : data(data_) {}
  template <typename IndexT>
  const item_t& operator ()(const IndexT& idx) const {
    return data[idx];
  }
};

template <typename item_t, class data_list_t>
struct IndexAccessor  {
  typedef item_t return_type;
  data_list_t &data;
  IndexAccessor(data_list_t &data_) : data(data_) {}
  template <typename IndexT>
  item_t& operator ()(const IndexT& idx) const {
    return data[idx];
  }
};

#endif

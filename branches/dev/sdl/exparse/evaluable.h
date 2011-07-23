/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_evaluable_H
#define __olx_sdl_evaluable_H
#include "../ebase.h"
#include "../edict.h"
#include "expvalue.h"
BeginEsdlNamespace()

namespace exparse  {
  class TCastException : public TBasicException  {
    const std::type_info& type;
  public:
    TCastException(const olxstr& src, const std::type_info& ti) : type(ti),
      TBasicException(src, olxstr("Invalid cast to ") << ti.name()) {};
    TCastException(const TCastException& ce) : type(ce.type), TBasicException(ce) {}
    const std::type_info& GetTypeInfo() const {  return type;  }
    virtual IEObject* Replicate() const {  return new TCastException(*this);  }
  };

  struct IEvaluable {
    mutable int ref_cnt;
    IEvaluable() : ref_cnt(0) {}
    virtual ~IEvaluable() {
      if( ref_cnt != 0 )
        throw 1;
    }
    virtual IEvaluable* _evaluate() const = 0;
    typedef cast_result (*cast_operator)(const IEvaluable*);
    typedef olxdict<std::type_info const*, cast_operator, TPointerComparator> operator_dict;
    // self casting...
    static cast_result self_cast(const IEvaluable* i)  {  return cast_result(i, false);  }
    virtual cast_operator get_cast_operator(const std::type_info&) const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual IEvaluable* create_new(const void*) const {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual IEvaluable* find_property(const olxstr& name) {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual IEvaluable* find_method(const olxstr& name, const struct EvaluableFactory&, 
      const TPtrList<IEvaluable>& args, IEvaluable* proxy=NULL)
    {
      return NULL;
    }
    virtual bool is_final() const {  return false;  }
    inline int inc_ref() const {  return ++ref_cnt;  }
    inline int dec_ref() const {
      if( --ref_cnt < 0 )
        throw 1;
      return ref_cnt;
    }

    template <class T> struct caster  {
      cast_operator co;
      caster(cast_operator _co) : co(_co){}
      val_wrapper<T,IEvaluable> cast(const IEvaluable* i) const {  return val_wrapper<T,IEvaluable>((*co)(i));  }
    };
    template <class T> struct caster<const T&>  {
      cast_operator co;
      caster(cast_operator _co) : co(_co){}
      val_wrapper<const T&,IEvaluable> cast(const IEvaluable* i) const {  return val_wrapper<const T&,IEvaluable>((*co)(i));  }
    };

    virtual const std::type_info& get_type_info() const {  return typeid(*this);  }

    template <typename T> val_wrapper<T,IEvaluable> cast() const {
      if( !is_final() )  {
        IEvaluable* tmp = _evaluate();
        val_wrapper<T,IEvaluable> rv(tmp->cast<T>(), tmp);
        return rv;
      }
      const std::type_info& ti = typeid(T);
      // a simple case...
      if( ti == get_type_info() )
        return caster<T>(&IEvaluable::self_cast).cast(this);
      try  {  
        cast_operator co = get_cast_operator(ti);
        if( co != NULL )
          return caster<T>(co).cast(this);
      }
      catch(...)  {}
      throw TCastException(__OlxSourceInfo, ti);
    }
    template <class T> static const T* cast_helper(const IEvaluable* i)  {
      const T* ci = dynamic_cast<const T*>(i);
      if( ci == NULL )  
        throw TCastException(__OlxSourceInfo, typeid(i));
      return ci;
    }
  };

  struct ANumberEvaluator : public IEvaluable  {
  protected:
    static IEvaluable::operator_dict cast_operators;
    static const IEvaluable::operator_dict::Entry cast_operators_table[];
    static cast_result bool_cast(const IEvaluable* i)  {  
      return cast_result(new bool(IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate() != 0), true);  
    }
    static cast_result str_cast(const IEvaluable* i)  {  
      return cast_result(new olxstr(IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate()), true);
    }
    template<class T> static cast_result primitive_cast(const IEvaluable* i)  {  
      return cast_result(new T((T)IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate()), true);  
    }
    template<class T> static void register_cast()  {  cast_operators.Add(&typeid(T), &ANumberEvaluator::primitive_cast<T>);  }
    template<class T> static IEvaluable::operator_dict::Entry create_operator_entry()  {  
      return IEvaluable::operator_dict::Entry(&typeid(T), &ANumberEvaluator::primitive_cast<T>);  
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {  return cast_operators[&ti];  } 
    virtual double Evaluate() const {
      IEvaluable* ev = _evaluate();
      const double rv = ev->cast<double>();
      return rv;
    }
    bool final;  // does represent a number?
  public:
    ANumberEvaluator(bool _final=false) : final(_final)  {}
    inline bool is_final() const {  return final;  }
  };
  template <class BC, typename Type>
  struct TPrimitiveEvaluator : public ANumberEvaluator, public BC  {
    static cast_result str_cast(const IEvaluable* i)  {  
      return cast_result(new olxstr(IEvaluable::cast_helper<TPrimitiveEvaluator<BC,Type> >(i)->get_value()), true);
    }
    static cast_result val_cast(const IEvaluable* i)  {  
      return cast_result(&IEvaluable::cast_helper<TPrimitiveEvaluator<BC,Type> >(i)->get_value(), false);
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {  
      if( typeid(Type) == ti )
        return &val_cast;
      else if( typeid(olxstr) == ti )
        return &str_cast;
      return ANumberEvaluator::get_cast_operator(ti);  
    } 
    TPrimitiveEvaluator(const Type& val) : ANumberEvaluator(true), BC(val)  {}
    virtual double Evaluate() const {
      return (double)BC::val;
    }
    virtual IEvaluable* _evaluate() const {  
      throw 1;
    }
    virtual IEvaluable* create_new(const void* v) const {
      return new TPrimitiveEvaluator<BC,Type>( *static_cast<const Type*>(v) );
    }
  };

  template <class T> struct TPrimitiveInstance  {
    T val;
    TPrimitiveInstance(const T& _val) : val(_val)  {}
    const T& get_value() const {  return val;  }
  };

  struct VoidValue : public IEvaluable {
    virtual IEvaluable* _evaluate() const {
      throw TFunctionFailedException(__OlxSourceInfo, "cannot evaluate void type");
    }
    virtual cast_operator get_cast_operator(const std::type_info&) const {
      throw TFunctionFailedException(__OlxSourceInfo, "no casting is avilable for the void type");
    }
  };
  typedef TPrimitiveEvaluator<TPrimitiveInstance<bool>,bool> BoolValue;
  typedef TPrimitiveEvaluator<TPrimitiveInstance<int>,int> IntValue;
  typedef TPrimitiveEvaluator<TPrimitiveInstance<double>,double> DoubleValue;

  struct EvaluableFactory  {
    olxdict<std::type_info const*, IEvaluable*, TPointerComparator> types;
    olxdict<std::type_info const*, struct IClassInfo*, TPointerComparator> classes;
    template <class T> void add_ptype()  {
      types.Add(&typeid(T), new TPrimitiveEvaluator<TPrimitiveInstance<T>,T>(0));
    }
    EvaluableFactory()  {
      if( types.IsEmpty() )  {
        add_ptype<bool>();
        add_ptype<char>();
        add_ptype<unsigned char>();
        add_ptype<short int>();
        add_ptype<unsigned short int>();
        add_ptype<int>();
        add_ptype<unsigned int>();
        add_ptype<long int>();
        add_ptype<unsigned long int>();
        add_ptype<long long int>();
        add_ptype<unsigned long long int>();
        add_ptype<float>();
        add_ptype<double>();
        add_ptype<long double>();
      }
    }
    ~EvaluableFactory()  {
      for( size_t i=0; i < types.Count(); i++ )
        delete types.GetValue(i);
    }
    template <class T> IEvaluable* create(const T& val) const {
      const std::type_info& ti = typeid(T);
      size_t i = types.IndexOf(&ti);
      if( i == InvalidIndex )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate object factory for ") << ti.name());
      return types.GetValue(i)->create_new(&val);
    };
  };
};  // end namespace exparse

EndEsdlNamespace()
#endif

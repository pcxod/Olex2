/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xgl_xeval_H
#define __olx_xgl_xeval_H
#include "xbond.h"
#include "xatom.h"
#include "glgroup.h"
#include "seval.h"

// data provider interface
class ITXAtom_DataProvider: public ITSAtom_DataProvider {
public:
  virtual TXAtom *GetTXAtom() = 0;
  virtual void SetTXAtom(TXAtom *)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  TSAtom *GetTSAtom() { return GetTXAtom(); }
};
class TTXAtom_DataProvider : public ITXAtom_DataProvider {
  TXAtom *XAtom;
public:
  TTXAtom_DataProvider() : XAtom(NULL) {}
  TXAtom *GetTXAtom() {
    if (XAtom == NULL)
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    return XAtom;
  }
  void SetTXAtom(TXAtom *val) { XAtom = val; }
};

// data provider interface
class ITXBond_DataProvider: public IDataProvider  {
public:
  virtual TXBond *GetTXBond() = 0;
  virtual void SetTXBond(TXBond *) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual TSBond *GetTSBond() { return GetTXBond(); }
};
class TTXBond_DataProvider : public ITXBond_DataProvider {
  TXBond *XBond;
public:
  TXBond *GetTXBond() {
    if (XBond == NULL)
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    return XBond;
  }
  void SetTXBond(TXBond *val)  {  XBond = val;  }
};
// factory class implementation
class TTXBond_EvaluatorFactory : public IEvaluatorFactory {
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr &propertyName)  {
    return Evaluators[propertyName];
  }
  IEvaluator *Evaluator(size_t index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetKey(index);  }
  size_t EvaluatorCount()  {  return Evaluators.Count();  }
  // destructor
  ~TTXBond_EvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
    delete provider;
  }
  // constructor to create instaces of registered evaluators
  TTXBond_EvaluatorFactory(IEvaluatorFactory *parent,
    ITXBond_DataProvider *provider);
  ITXBond_DataProvider *provider;
};

// data provider interface
class ITGlGroupDataProvider
  : public ITXAtom_DataProvider, public ITXBond_DataProvider
{
public:
  virtual TGlGroup *GetTGlGroup() = 0;
  virtual void SetTGlGroup(TGlGroup *)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  TXAtom *GetTXAtom()  {
    TGlGroup *g = GetTGlGroup();
    if (g->Count() == 0 || !EsdlInstanceOf((*g)[0], TXAtom))
      throw TFunctionFailedException(__OlxSourceInfo, "invalid object type");
    return &(TXAtom&)g->GetObject(0);
  }
  TXBond *GetTXBond() {
    TGlGroup *g = GetTGlGroup();
    if (g->Count() == 0 || !EsdlInstanceOf((*g)[0], TXBond))
      throw TFunctionFailedException(__OlxSourceInfo, "invalid object type");
    return &(TXBond&)g->GetObject(0);
  }
};
class TTGlGroupDataProvider : public ITGlGroupDataProvider {
  TGlGroup *sel;
public:
  TGlGroup *GetTGlGroup() {
    if (sel == NULL)
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    return sel;
  }
  void SetTGlGroup(TGlGroup *val) { sel = val; }
};

// factory class implementation
class TTXAtom_EvaluatorFactory: public IEvaluatorFactory {
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(size_t index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetKey(index);  }
  size_t EvaluatorCount()  {  return Evaluators.Count();  }
  ~TTXAtom_EvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
    delete provider;
  }
  // constructor to create instaces of registered evaluators
  TTXAtom_EvaluatorFactory(IEvaluatorFactory *parent,
    ITXAtom_DataProvider *provider);
  ITXAtom_DataProvider *provider;
};
// evaluator implementation for bool selected
class TXAtom_SelectedEvaluator
  : public TEvaluator<ITXAtom_DataProvider,IBoolEvaluator,TXAtom_SelectedEvaluator>
{
public:
  // constructor
  TXAtom_SelectedEvaluator(ITXAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  bool EvaluateBool() const {  return Parent->GetTXAtom()->IsSelected();  }
};

// factory class implementation
class TTGlGroupEvaluatorFactory : public IEvaluatorFactory {
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IEvaluatorFactory*, true> Factories;
  TPtrList<ITGlGroupDataProvider> providers;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr &propertyName)  {
    return Evaluators[propertyName];
  }
  IEvaluator *Evaluator(size_t index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetKey(index);  }
  size_t EvaluatorCount()  {  return Evaluators.Count();  }
  ~TTGlGroupEvaluatorFactory()  {
    for( size_t i=0; i < Factories.Count(); i++ )
      delete Factories.GetObject(i);
  }
  void SetTGlGroup(TGlGroup *g) {
    for (size_t i=0; i < providers.Count(); i++)
      providers[i]->SetTGlGroup(g);
  }
  // constructor to create instaces of registered evaluators
  TTGlGroupEvaluatorFactory(IEvaluatorFactory *parent);
};
// evaluator implementation for complex A
template <int n>
class TXBond_AtomEvaluator : public ITSAtom_DataProvider {
  ITXBond_DataProvider *Parent;
public:
  TXBond_AtomEvaluator(ITXBond_DataProvider* parent) : Parent(parent) {}
  TSAtom* GetTSAtom()  {
    return &(n ==1 ? Parent->GetTXBond()->A() : Parent->GetTXBond()->B());
  }
};

// evaluator implementation for bool deleted
class TXBond_DeletedEvaluator
  : public TEvaluator<ITXBond_DataProvider,IBoolEvaluator,TXBond_DeletedEvaluator> {
public:
  // constructor
  TXBond_DeletedEvaluator(ITXBond_DataProvider* parent)
    : parent_t(parent)
  {}
  bool EvaluateBool() const {  return Parent->GetTXBond()->IsDeleted();  }
};
// evaluator implementation for scalar length
class TXBond_LengthEvaluator
  : public TEvaluator<ITXBond_DataProvider,IDoubleEvaluator,TXBond_LengthEvaluator>
{
public:
  TXBond_LengthEvaluator(ITXBond_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {  return Parent->GetTXBond()->Length();  }
};
// evaluator implementation for scalar type
class TXBond_TypeEvaluator
  : public TEvaluator<ITXBond_DataProvider,IDoubleEvaluator,TXBond_TypeEvaluator>
{
public:
  TXBond_TypeEvaluator(ITXBond_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {  return Parent->GetTXBond()->GetType();  }
};

class TXFactoryRegister : public TSFactoryRegister {
public:
  TXFactoryRegister();
};

#endif

/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_residue_H
#define __olx_residue_H
#include "asymmunit.h"

BeginXlibNamespace()

class TResidue : public IEObject  {
  olxstr ClassName, Alias;
  int Number;
  uint16_t Id;
  TCAtomPList Atoms;
  TAsymmUnit& Parent;
protected:
  // removes an atom and resets the ResiId, this is used internally from Add
  void Remove(TCAtom& ca)           {
    size_t i = Atoms.IndexOf(ca);
    if( i != InvalidIndex )  {
      ca.SetResiId(~0);
      Atoms.Delete(i);
    }
  }
  // adds an atom to this residue without trying to remove from the owning residue (if there is any!)
  void _Add(TCAtom& ca) {
    Atoms.Add(ca);
    ca.SetResiId(Id);
  }
public:
  TResidue(TAsymmUnit& parent, uint32_t id, const olxstr& cl=EmptyString(), int number = 0, const olxstr& alias=EmptyString()) : 
      Parent(parent), 
      Id(id), 
      ClassName(cl), 
      Number(number), 
      Alias(alias)  {  }
  //
  DefPropC(olxstr, ClassName)
  DefPropC(olxstr, Alias)
  DefPropP(int, Number)
  uint32_t GetId() const {  return Id;  }
  TCAtomPList& GetAtomList()  {  return Atoms;  }
  const TCAtomPList& GetAtomList() const {  return Atoms;  }
  TAsymmUnit& GetParent()  {  return Parent;  }
  virtual TIString ToString() const {
    if( Id == 0 )  return EmptyString();
    olxstr rv("RESI ");
    rv << ClassName;
    if( Number != 0 )  rv << ' ' << Number;
    return (rv << (Alias.IsEmpty() ? EmptyString() : (olxstr(' ') << Alias)));
  }
  size_t Count() const {  return Atoms.Count();  }  
  TCAtom& GetAtom(size_t i) const {  return *Atoms[i];  }
  TCAtom& operator [] (size_t i) const {  return *Atoms[i]; }
  void Clear()                      {  
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetResiId(~0);
    Atoms.Clear();  
  } 
  size_t IndexOf(const TCAtom& ca) const {  return Atoms.IndexOf(ca);  }
  // removes atom from previous residue and puts into current
  void Add(TCAtom& ca) {
    Parent.GetResidue(ca.GetResiId()).Remove(ca);
    Atoms.Add(ca);
    ca.SetResiId(Id);
  }
  void SetCapacity(size_t c)  {  Atoms.SetCapacity(c);  }
  bool IsEmpty() const {
    for( size_t i=0; i < Atoms.Count(); i++ )
      if( !Atoms[i]->IsDeleted() )  return false;
    return true;
  }
  TResidue* Next() const {  return Parent.NextResidue(*this);  }
  TResidue* Prev() const {  return Parent.PrevResidue(*this);  }
  friend class TAsymmUnit;
};

typedef TPtrList<TResidue> ResiPList;

EndXlibNamespace()

#endif


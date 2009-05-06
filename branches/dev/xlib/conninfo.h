#ifndef __olxconn_info_H
#define __olxconn_info_H

#include "edict.h"
#include "bapp.h"
#include "connext.h"

BeginXlibNamespace()

class RefinementModel;

class ConnInfo  {
public:
protected:
  struct AtomConnInfo : public CXConnInfoBase  {
    TCAtom* atom;
    BondInfoList BondsToCreate, BondsToRemove;
    AtomConnInfo() : atom(NULL) {}
    AtomConnInfo(TCAtom& ca) : atom(&ca) {}
    AtomConnInfo(const AtomConnInfo& ci) : CXConnInfoBase(ci), atom(ci.atom) {}
    AtomConnInfo& operator = (const AtomConnInfo& ci)  {
      CXConnInfoBase::operator = (ci);
      atom = ci.atom;
      BondsToCreate = ci.BondsToCreate;
      BondsToRemove = ci.BondsToRemove;
      return *this;
    }
  };
  struct TypeConnInfo : public CXConnInfoBase  {
    TBasicAtomInfo* atomInfo;
    TypeConnInfo() : atomInfo(NULL) {}
    TypeConnInfo(TBasicAtomInfo& bai) : atomInfo(&bai) {}
    TypeConnInfo(const TypeConnInfo& ci) : CXConnInfoBase(ci), atomInfo(ci.atomInfo) {}
    TypeConnInfo& operator = (const TypeConnInfo& ti)  {
      CXConnInfoBase::operator = (ti);
      atomInfo = ti.atomInfo;
      return *this;
    }
  };
  olxdict<TCAtom*, AtomConnInfo, TPointerPtrComparator> AtomInfo;
  olxdict<TBasicAtomInfo*, TypeConnInfo, TPointerPtrComparator> TypeInfo;
public:
  ConnInfo(RefinementModel& _rm) : rm(_rm) {}

  RefinementModel& rm;

  // prepares a list of extra connectivity info for each atom of the AUnit
  CXConnInfo& GetConnInfo(const TCAtom& ca) const;
  // an object created with new is returned always
  CXConnInfo& GetConnInfo(TBasicAtomInfo& bai) const;

  void ProcessConn(TStrList& ins);

  void AddBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv)  {
    AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
    bool found = false;
    for( int i=0; i < ai.BondsToCreate.Count(); i++ )  {
      if( ai.BondsToCreate[i].to == a2 )  {
        if( ai.BondsToCreate[i].matr == NULL && eqiv == NULL )  {
          found = true;
          break;
        }
        if( ai.BondsToCreate[i].matr != NULL && eqiv != NULL &&
            *ai.BondsToCreate[i].matr == *eqiv )  
        {
          found = true;
          break;
        }
      }
    }
    if( found )
      return;
    // need to check if the same bond is not in the BondsToRemove List
    ai.BondsToCreate.Add( new CXBondInfo(a2, eqiv) );
  }
  void RemBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv)  {
    AtomConnInfo& ai = AtomInfo.Add(&a1, AtomConnInfo(a1));
    bool found = false;
    for( int i=0; i < ai.BondsToRemove.Count(); i++ )  {
      if( ai.BondsToRemove[i].to == a2 )  {
        if( ai.BondsToRemove[i].matr == NULL && eqiv == NULL )  {
          found = true;
          break;
        }
        if( ai.BondsToRemove[i].matr != NULL && eqiv != NULL &&
            *ai.BondsToRemove[i].matr == *eqiv )  
        {
          found = true;
          break;
        }
      }
    }
    if( found )
      return;
    // need to check if the same bond is not in the BondsToCreate List
    ai.BondsToRemove.Add( new CXBondInfo(a2, eqiv) );
  }
  //.................................................................
  void ProcessFree(const TStrList& ins);
  void ProcessBind(const TStrList& ins);
  
  void Clear()  {
    AtomInfo.Clear();
    TypeInfo.Clear();
  }

  void ToInsList(TStrList& ins) const;

  void Assign(const ConnInfo& ci);

};

EndXlibNamespace()
#endif

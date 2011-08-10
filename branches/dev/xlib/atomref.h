/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_atomref_H
#define __olx_xl_atomref_H
#include "refmodel.h"
#include "residue.h"

BeginXlibNamespace()
/* Possible Shelx syntax
  C_*  - 'C' atom for all residues
  C_Res - 'C' of particular residue
  $C_Res - ? all carbons of particular residue
  $C_Res$Eqiv - all carbons of Res in Eqiv symmetry
  C_$1
  C_+, C_- - cannot be expanded, just references for previous and next residues
    when something is being calculated
  O21 > O25 -> [021..O25] 
  O25 < O21 -> [025..O21] 
*/
//TODO: add C? C?1 kind of expressions
class ASelectionOwner {
  bool DoClearSelection;
public:
  ASelectionOwner() : DoClearSelection(true) {}
  virtual void ExpandSelection(TCAtomGroup& catoms) = 0;
  virtual void ExpandSelectionEx(TPtrList<class TSAtom>& satoms) = 0;
  DefPropP(bool, DoClearSelection)
};
class TAtomReference : public IEObject  {
  olxstr Expression;
protected:
  inline bool IsValidAtom(TCAtom* ca)  {
    return !(ca->IsDeleted() || ca->GetType().GetMr() < 3);
  }
  ASelectionOwner* SelectionOwner;
public:
  TAtomReference(const olxstr& expression,
    ASelectionOwner* selectionOwner=NULL)
    : Expression(expression), SelectionOwner(selectionOwner)  {}

  const olxstr& GetExpression() const {  return Expression;  }

  size_t _Expand(RefinementModel& rm, TCAtomGroup& atoms,
    TResidue* CurrResi)
  {
    const size_t ac = atoms.Count();
    if( Expression.IsEmpty() )  {  // all atoms of au
      atoms.SetCapacity(atoms.Count() + rm.aunit.AtomCount());
      for( size_t i=0; i < rm.aunit.AtomCount(); i++ )  {
        TCAtom* ca = &rm.aunit.GetAtom(i);
        if( IsValidAtom(ca) )
          atoms.AddNew(ca);
      }
      return atoms.Count() - ac;
    }
    else if( Expression.Equalsi("sel") )  { 
      if( SelectionOwner == NULL )  {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "invalid selection owner");
      }
      const size_t ac = atoms.Count();
      SelectionOwner->ExpandSelection(atoms);
      return atoms.Count()-ac;
    }
    else if( Expression.Equalsi("first") )  { 
      if( rm.aunit.AtomCount() == 0 )  return 0;
      size_t i=0;
      TCAtom* ca = &rm.aunit.GetAtom(i);
      while( (i+1) < rm.aunit.AtomCount() && !IsValidAtom(ca) )  {
        i++;
        ca = &rm.aunit.GetAtom(i);
      }
      if( !IsValidAtom(ca) )  return 0;
      atoms.AddNew( ca );
      return 1;
    }
    else if( Expression.StartsFrom("#c") )  { 
      if( rm.aunit.AtomCount() == 0 )  return 0;
      size_t i= Expression.SubStringFrom(2).ToInt();
      if( i >= rm.aunit.AtomCount() )
        throw TInvalidArgumentException(__OlxSourceInfo, "catom id");
      TCAtom* ca = &rm.aunit.GetAtom(i);
      //skip validation here...
      //if( !IsValidAtom(ca) )  return 0;
      atoms.AddNew(ca);
      return 1;
    }
    else if( Expression.Equalsi("last") )  { 
      if( rm.aunit.AtomCount() == 0 )  return 0;
      size_t i = rm.aunit.AtomCount()-1;
      TCAtom* ca = &rm.aunit.GetAtom(i);
      while( i > 0 && !IsValidAtom(ca) )  {
        i--;
        ca = &rm.aunit.GetAtom(i);
      }
      if( !IsValidAtom(ca) )  return 0;
      atoms.AddNew(ca);
      return 1;
    }
    // validate complex expressions with >< chars
    size_t gs_ind = Expression.IndexOf('>'),
        ls_ind = Expression.IndexOf('<');
    if( gs_ind != InvalidIndex || ls_ind != InvalidIndex )  {
      TCAtomGroup from, to;
      if( gs_ind != InvalidIndex )  {  // it is inverted in shelx ...
        TAtomReference(Expression.SubStringTo(gs_ind).Trim(' '))._Expand(
          rm, from, CurrResi);
        TAtomReference(Expression.SubStringFrom(gs_ind+1).Trim(' '))._Expand(
          rm, to, CurrResi);
      }
      else  {
        TAtomReference(Expression.SubStringTo(ls_ind).Trim(' '))._Expand
          (rm, to, CurrResi);
        TAtomReference(Expression.SubStringFrom(ls_ind+1).Trim(' '))._Expand(
          rm, from, CurrResi);
      }
      if( to.Count() != 1 || from.Count() != 1 )  {
        throw TFunctionFailedException(__OlxSourceInfo,
          "failed to expand >/< expression");
      }
      if( from[0].GetAtom()->GetId() >= to[0].GetAtom()->GetId() )
        throw TFunctionFailedException(__OlxSourceInfo, "invalid direction");
      if( from[0].GetMatrix() != to[0].GetMatrix() )  {
        throw TFunctionFailedException(__OlxSourceInfo,
          "EQIV must be the same in >/< expresion");
      }
      if( gs_ind != InvalidIndex )  {
        for( size_t i=from[0].GetAtom()->GetId();
          i <= to[0].GetAtom()->GetId(); i++ )
        {
          TCAtom* ca = &rm.aunit.GetAtom(i);
          if( !IsValidAtom(ca) )  continue;
          atoms.AddNew(ca, from[0].GetMatrix());
        }
      }
      else  {
        for( size_t i=to[0].GetAtom()->GetId();
          i >= from[0].GetAtom()->GetId(); i-- )
        {
          TCAtom* ca = &rm.aunit.GetAtom(i);
          if( !IsValidAtom(ca) )  continue;
          atoms.AddNew(ca, from[0].GetMatrix());
        }
      }
      return atoms.Count() - ac;
    }
    //
    size_t resi_ind = Expression.IndexOf('_');
    olxstr resi_name = (resi_ind == InvalidIndex ? EmptyString()
      : Expression.SubStringFrom(resi_ind+1));
    // check if it is just an equivalent position
    const smatd* eqiv = NULL;
    size_t eqiv_ind = resi_name.IndexOf('$');
    if( eqiv_ind != InvalidIndex )  {  
      olxstr str_eqiv = resi_name.SubStringFrom(eqiv_ind);
      eqiv = rm.FindUsedSymm(str_eqiv);
      if( eqiv == NULL )  {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("Equivalent id: ") << str_eqiv);
      }
      resi_name = resi_name.SubStringTo(eqiv_ind);
    }
    // validate syntax
    TPtrList<TResidue> residues;
    if( !resi_name.IsEmpty() &&
      (resi_name.CharAt(0) == '+' || resi_name.CharAt(0) == '-') )
    {
      if( CurrResi == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "current residue");
      if( resi_name.CharAt(0) == '+' )
        residues.Add(rm.aunit.NextResidue(*CurrResi));
      else  
        residues.Add(rm.aunit.PrevResidue(*CurrResi));
    }
    else  {
      // empty resi name refers to all atom outside RESI
      if( !resi_name.IsEmpty() )
        rm.aunit.FindResidues(resi_name, residues);  
      else if( CurrResi != NULL )  
        residues.Add(CurrResi);
      if( residues.IsEmpty() )  {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("invalid residue class/number: ") << resi_name);
      }
    }
    if( Expression.CharAt(0) == '$' )  {  // sfac type
      olxstr sfac = ((resi_ind == InvalidIndex) ? Expression.SubStringFrom(1)
        : Expression.SubString(1, resi_ind-1));
      SortedElementPList elms = DecodeTypes(sfac, rm.aunit);
      //if( elms.IsEmpty() )  {
      //  throw TInvalidArgumentException(__OlxSourceInfo,
      //    olxstr("sfac=") << sfac);
      //}
      for( size_t i=0; i < residues.Count(); i++ )  {
        for( size_t j=0; j < residues[i]->Count(); j++ )  {
          TCAtom& ca = residues[i]->GetAtom(j);
          // cannot use IsValid here, $H will not work
          if( !ca.IsDeleted() && elms.IndexOf(&ca.GetType()) != InvalidIndex )
            atoms.AddNew(&ca, eqiv);
        }
      }
    }
    else  {  // just an atom
      olxstr aname = ((resi_ind == InvalidIndex) ? Expression
        : Expression.SubStringTo(resi_ind));
      for( size_t i=0; i < residues.Count(); i++ )  {
        if( residues[i] == NULL )  continue;
        for( size_t j=0; j < residues[i]->Count(); j++ )  {
          TCAtom* ca = &residues[i]->GetAtom(j);
          // must be unique!
          if( !ca->IsDeleted() && ca->GetLabel().Equalsi(aname) )  {
            atoms.AddNew(ca, eqiv);
            break;
          }
        }
      }
    }
    return atoms.Count() - ac;
  }
  // prcesses shelx expressions and returns unprocessed expressions (like sel)
  olxstr Expand(RefinementModel& rm, TCAtomGroup& atoms, const olxstr& DefResi,
    size_t& atomAGroup)
  {
    olxstr nexp, exp( olxstr::DeleteSequencesOf(Expression, ' ').Trim(' ') );
    nexp.SetCapacity( exp.Length() );
    // remove spaces from arounf >< chars for strtok
    for( size_t i=0; i < exp.Length(); i++ )  {
      if( (i+1) < exp.Length() && exp.CharAt(i) == ' ' &&
          (exp.CharAt(i+1) == '<' || exp.CharAt(i+1) == '>') )
      {
        continue;
      }
      if( (i > 0) && (exp.CharAt(i-1) == '<' || exp.CharAt(i-1) == '>') &&
        exp.CharAt(i) == ' ')
      {
        continue;
      }
      nexp << exp.CharAt(i);
    }
    atomAGroup = 0;
    TCAtomGroup tmp_atoms;
    TPtrList<TResidue> residues;
    // empty resi name refers to all atom outside RESI
    rm.aunit.FindResidues(DefResi, residues);
    TStrList toks(nexp, ' '), unprocessed;
    for( size_t i=0; i < residues.Count(); i++ )  {
      bool succeded = true;
      for( size_t j=0; j < toks.Count(); j++ )  {
        if( TAtomReference(toks[j], SelectionOwner)._Expand(
          rm, tmp_atoms, residues[i]) == 0 )
        {
          if( i == 0 )  unprocessed.Add( toks[j] );
          succeded = false;
          break;
        }
      }
      if( succeded )  {
        atoms.AddListC(tmp_atoms);
        if( atomAGroup == 0 )
          atomAGroup = tmp_atoms.Count();
      }
      tmp_atoms.Clear();
    }
    return unprocessed.IsEmpty() ? EmptyString() : unprocessed.Text(' ');
  }
  /*Returns a list of elements reffered like $C,O - all O and C or $*,C
  (alternatively: $*-C - all but C
  */
  static ConstSortedElementPList DecodeTypes(const olxstr &types,
    const TAsymmUnit & au)
  {
    SortedElementPList res;
    if( types.StartsFrom('*') )  {
      for( size_t i=0; i < au.AtomCount(); i++ )
        res.AddUnique(&au.GetAtom(i).GetType());
      if( types.Length() == 1 )  return res;
      TStrList exc(types.SubStringFrom(types.CharAt(1) == '-' ? 2 : 1), ',');
      for( size_t i=0; i < exc.Count(); i++ )  {
        const cm_Element *elm = XElementLib::FindBySymbol(exc[i]);
        if( elm == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom type=") << exc[i]);
        res.Remove(elm);
      }
    }
    else  {
      TStrList tps(types, ',');
      for( size_t i=0; i < tps.Count(); i++ )  {
        const cm_Element *elm = XElementLib::FindBySymbol(tps[i]);
        if( elm == NULL )  {
          throw TInvalidArgumentException(__OlxSourceInfo,
            olxstr("atom type=") << tps[i]);
        }
        res.AddUnique(elm);
      }
    }
    return res;
  }

};

EndXlibNamespace()
#endif

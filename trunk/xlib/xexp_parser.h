#ifndef _olx_xexp_parser
#define _olx_xexp_parser
#include "xmodel.h"
#include "estrlist.h"
#include "estlist.h"
BeginXlibNamespace()

class TShelxAtomListParser : public IEObject  {
  olxstr Expression;
protected:
  inline bool IsValidScatterer(XScatterer* xs)  {
    return !(*xs->Type == iHydrogenIndex ||
             *xs->Type == iDeuteriumIndex ||
             *xs->Type == iQPeakIndex );
  }
public:
  TShelxAtomListParser(const olxstr& expression) : Expression(expression)  {  }
  const olxstr& GetExpression() const {  return Expression;  }
  int _Expand( IRefinementModel& rm, 
               XScattererRefList& out,              // destination
               XResidue* resi,               // current residue, updated if different
               const TPtrList<XScatterer>& resi_cont)  {  // odered residue content
    int ac = out.Count();
    if( Expression.IsEmpty() )  {  // all atoms of the residue
      out.SetCapacity( out.Count() + resi_cont.Count() );
      for( int i=0; i < resi_cont.Count(); i++ )  {
        if( IsValidScatterer( resi_cont[i]) )
          out.AddNew( resi_cont[i], (smatd*)NULL );
      }
      return out.Count() - ac;
    }
    else if( Expression.Comparei("first") == 0 )  { 
      int i=0;
      XScatterer* xs = resi_cont[i];
      while( (i+1) < resi_cont.Count() && !IsValidScatterer(xs) )  {
        i++;
        xs = resi_cont[i];
      }
      if( !IsValidScatterer(xs) )  return 0;
      out.AddNew(xs, (smatd*)NULL);
      return 1;
    }
    else if( Expression.Comparei("last") == 0 )  { 
      int i=resi_cont.Count()-1;
      XScatterer* xs = resi_cont[i];
      while( (i-1) >= 0 && !IsValidScatterer(xs) )  {
        i--;
        xs = resi_cont[i];
      }
      if( !IsValidScatterer(xs) )  return 0;
      out.AddNew(xs, (smatd*)NULL);
      return 1;
    }
    // validate complex expressions with >< chars
    int gs_ind = Expression.IndexOf('>'),
        ls_ind = Expression.IndexOf('<');
    if( gs_ind != -1 || ls_ind != -1 )  {
      XScattererRefList from, to;
      if( gs_ind != -1 )  {  // it is inverted in shelx ...
        TShelxAtomListParser(Expression.SubStringTo(gs_ind).Trim(' '))._Expand(rm, from, resi, resi_cont);
        TShelxAtomListParser(Expression.SubStringFrom(gs_ind+1).Trim(' '))._Expand(rm, to, resi, resi_cont);
      }
      else  {
        TShelxAtomListParser(Expression.SubStringTo(ls_ind).Trim(' '))._Expand(rm, from, resi, resi_cont);
        TShelxAtomListParser(Expression.SubStringFrom(ls_ind+1).Trim(' '))._Expand(rm, to, resi, resi_cont);
      }
      if( to.Count() != 1 || from.Count() != 1 )
        throw TFunctionFailedException(__OlxSourceInfo, "failed to expand >/< expression");
      if( from[0].symm != to[0].symm )
        throw TFunctionFailedException(__OlxSourceInfo, "EQIV must be the same in >/< expresion");
      if( from[0].scatterer->Owner != to[0].scatterer->Owner )
        throw TFunctionFailedException(__OlxSourceInfo, "RESI must be the same in >/< expresion");

      TPtrList<XScatterer> const* resi_scat = &resi_cont;
      if( from[0].scatterer->Owner != resi )  {
        resi_scat = new TPtrList<XScatterer>;
        from[0].scatterer->Owner->GetScatterers(*const_cast<TPtrList<XScatterer>*>(resi_scat));
      }
      int from_ind = resi_scat->IndexOf(from[0].scatterer);
      int to_ind = resi_scat->IndexOf(to[0].scatterer);

      if( (from_ind >= to_ind && gs_ind != -1) || (from_ind <= to_ind && ls_ind != -1) )  {
        if( from[0].scatterer->Owner != resi )  
          delete resi_scat;
        throw TInvalidArgumentException(__OlxSourceInfo, "invalid direction in >/< expression");
      }

      if( gs_ind != -1 )  {
        for( int i=from_ind; i <= to_ind; i++ )  {
          if( !IsValidScatterer( (*resi_scat)[i] ) )  continue;
          out.AddNew( (*resi_scat)[i], from[0].symm );
        }
      }
      else  {
        for( int i=from_ind; i >= to_ind; i-- )  {
          if( !IsValidScatterer( (*resi_scat)[i] ) )  continue;
          out.AddNew( (*resi_scat)[i], from[0].symm );
        }
      }
      if( from[0].scatterer->Owner != resi )  
        delete resi_scat;
      return out.Count() - ac;
    }
    //
    int resi_ind = Expression.IndexOf('_');
    olxstr resi_name = (resi_ind == -1 ? EmptyString : Expression.SubStringFrom(resi_ind+1));
    // check if it is just an equivalent position
    const smatd* eqiv = NULL;
    int eqiv_ind = resi_name.IndexOf('$');
    if( eqiv_ind > 0 )  {  // 0 is for SFAC type
      olxstr str_eqiv( resi_name.SubStringFrom(eqiv_ind+1) );
      if( !str_eqiv.IsNumber() )  
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid equivalent number: ") << str_eqiv);
      int eqi = str_eqiv.ToInt()-1;
      if( eqi < 0 || eqi >= rm.UsedSymmCount() )  
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid equivalent index: ") << str_eqiv);
      eqiv = rm.GetUsedSymm(eqi);
      resi_name = resi_name.SubStringTo(eqiv_ind);
    }
    // validate syntax
    TPtrList<XResidue> residues;
    if( resi == NULL )  
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid residue for +/- referencing");
    if( !resi_name.IsEmpty() && (resi_name.CharAt(0) == '+' || resi_name.CharAt(0) == '-') )
      residues.Add( (resi_name.CharAt(0) == '+') ? rm.NextResidue(*resi) : rm.PrevResidue(*resi));
    else  {
      if( resi != NULL )  residues.Add(resi);
      if( !resi_name.IsEmpty() )  // empty resi name refers to all atom outside RESI
        rm.FindResidues(resi_name, residues);  
      if( residues.IsEmpty() )  
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid residue class/number: ") << resi_name);
    }
    if( Expression.CharAt(0) == '$' )  {  // sfac type
      olxstr sfac = ((resi_ind == -1) ? Expression.SubStringFrom(1) : Expression.SubString(1, resi_ind-1));
      TBasicAtomInfo* bai = TAtomsInfo::GetInstance()->FindAtomInfoBySymbol(sfac);
      if( bai == NULL )  
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("sfac=") << sfac);
      for( int i=0; i < residues.Count(); i++ )  {
        for( int j=0; j < residues[i]->Count(); j++ )  {
          if( *(*residues[i])[j].Type == *bai )  // cannot use IsValid here, $H will not work
            out.AddNew( &(*residues[i])[j], eqiv );
        }
      }
    }
    else  {  // just an atom
      olxstr aname = ( (resi_ind == -1) ? Expression : Expression.SubStringTo(resi_ind) );
      for( int i=0; i < residues.Count(); i++ )  {
        XScatterer* xs = residues[i]->FindScattererByName(aname);
        if( xs != NULL )  
          out.AddNew( xs, eqiv );
      }
    }
    return out.Count() - ac;
  }
  int Expand(IRefinementModel& rm, XScattererRefList& out, const olxstr& DefResi, int& atomAGroup)  {
    olxstr nexp, exp( olxstr::DeleteSequencesOf(Expression, ' ').Trim(' ') );
    nexp.SetCapacity( exp.Length() );
    // remove spaces from arounf >< chars for strtok
    for( int i=0; i < exp.Length(); i++ )  {
      if( (i+1) < exp.Length() && exp.CharAt(i) == ' ' && (exp.CharAt(i+1) == '<' || exp.CharAt(i+1) == '>') )
        continue;
      if( (i > 0) && (exp.CharAt(i-1) == '<' || exp.CharAt(i-1) == '>') && exp.CharAt(i) == ' ')
        continue;
      nexp << exp.CharAt(i);
    }
    atomAGroup = 0;
    TPtrList<XResidue> residues;
    rm.FindResidues(DefResi, residues);  // empty resi name refers to all atom outside RESI
    TStrList toks(nexp, ' ');
    TPtrList<XScatterer> resi_cont;
    int xsc = 0;
    for( int i=0; i < residues.Count(); i++ )  {
      resi_cont.Clear();
      residues[i]->GetScatterers(resi_cont);
      for( int j=0; j < toks.Count(); j++ )  {
        int fc = TShelxAtomListParser(toks[j])._Expand(rm, out, residues[i], resi_cont);
        if( atomAGroup == 0 )
          atomAGroup = fc;
        else if( atomAGroup != fc )
            TBasicApp::GetLog().Warning("uneven number of atoms in the groups");
        xsc += fc;
      }
    }
    return xsc;
  }
  int Expand(IRefinementModel& rm, XScattererRefList& out, XResidue* resi)  {
    olxstr nexp, exp( olxstr::DeleteSequencesOf(Expression, ' ').Trim(' ') );
    nexp.SetCapacity( exp.Length() );
    // remove spaces from arounf >< chars for strtok
    for( int i=0; i < exp.Length(); i++ )  {
      if( (i+1) < exp.Length() && exp.CharAt(i) == ' ' && (exp.CharAt(i+1) == '<' || exp.CharAt(i+1) == '>') )
        continue;
      if( (i > 0) && (exp.CharAt(i-1) == '<' || exp.CharAt(i-1) == '>') && exp.CharAt(i) == ' ')
        continue;
      nexp << exp.CharAt(i);
    }
    TStrList toks(nexp, ' ');
    TPtrList<XScatterer> resi_cont;
    resi->GetScatterers(resi_cont);
    int xsc = 0;
    for( int j=0; j < toks.Count(); j++ )
      xsc += TShelxAtomListParser(toks[j])._Expand(rm, out, resi, resi_cont);
    return xsc;
  }
};

EndXlibNamespace()
#endif

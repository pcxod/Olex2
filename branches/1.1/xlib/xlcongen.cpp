#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "xlcongen.h"
#include "unitcell.h"

//..............................................................................
bool TXlConGen::FixParam(const short paramMask, TStrList& res, const TCAtomPList& atoms, const TFixedValueList& values)  {
  throw TNotImplementedException( __OlxSourceInfo );
}
//..............................................................................
void TXlConGen::AnalyseMultipart(const TTypeList<TCAtomPList>& parts)  {
  size_t cnt = 0;
  for( size_t i=0; i < parts.Count(); i++ )  {
    if( !parts[i].IsEmpty() && parts[i][0]->GetParentAfixGroup() != NULL &&
      parts[i][0]->GetParentAfixGroup()->IsRefinable() )
    {
      cnt++;
    }
  }
  if( cnt > 1 )  {  // have to change the refineable groups to riding group...
    for( size_t i=0; i < parts.Count(); i++ )  {
      if( parts[i].IsEmpty() )  continue;
      if( parts[i][0]->GetParentAfixGroup() != NULL && parts[i][0]->GetParentAfixGroup()->IsRefinable() )  {
        const int m = parts[i][0]->GetParentAfixGroup()->GetM();
        if( m == 13 )
          parts[i][0]->GetParentAfixGroup()->SetAfix(33);
        else if( m == 14 )
          parts[i][0]->GetParentAfixGroup()->SetAfix(83);
        else
          parts[i][0]->GetParentAfixGroup()->SetAfix(m*10+3);
      }
    }
  }
}
//..............................................................................
bool TXlConGen::FixAtom(TAtomEnvi& envi, const short Group, const cm_Element& atomType, TAtomEnvi* pivoting, TCAtomPList* generated)  {
  try  {
    TSimpleRestraint* sr;
    TCAtomPList CreatedAtoms;
    TAtomEnvi NEnvi;
    GenerateAtom(CreatedAtoms, envi, Group, atomType, pivoting);
    if( CreatedAtoms.IsEmpty() )  // nothing inserted
      return false;
    short afix = 0;
    switch( Group )  {
      case fgNH3:
      case fgCH3:
        if( envi.Count() == 1 )  {
          //TSAtom *SA = envi.GetBase()->Network()->GetLattice()->FindSAtom( envi.GetCAtom(0) );
          //envi.GetBase()->Network()->GetLattice()->GetUnitCell()->GetAtomEnviList(*SA, NEnvi);
          if( NEnvi.Count() > 2 )  // TODO: makes sense?
            afix = 137;
          else
            afix = 137;
        }
        break;
      case fgCH2:
        if( envi.Count() == 2 )
          afix = 23;
        else if( envi.Count() == 1 )
          afix = 93;
        break;
      case fgCH1:
        if( envi.Count() == 3 )
          afix = 13;
        else if( envi.Count() == 2 )
          afix = 43;
        else if( envi.Count() == 1 )
          afix = 163;
        break;
      case fgSiH1:
        if( NEnvi.Count() == 3 )
          afix = 13;
        break;
      case fgOH3:
        break;
      case fgOH2:
        if( CreatedAtoms.Count() == 2 )  {
          sr = &RefMod.rDFIX.AddNew();
          sr->SetEsd(0.02);
          sr->SetValue(0.85);
          sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[0], NULL);
          sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[1], NULL);

          sr = &RefMod.rDANG.AddNew();
          sr->SetEsd(0.04);
          sr->SetValue(1.39);
          sr->AddAtomPair(*CreatedAtoms[1], NULL, *CreatedAtoms[0], NULL);

          if( envi.Count() == 1 )  {
            sr = &RefMod.rSADI.AddNew();
            sr->SetEsd(0.02);
            sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[0], NULL);
            sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[1], NULL);
          }
        }
        break;
      case fgSH1:
      case fgOH1:
        if( envi.Count() == 1 )
          afix = 147;
        break;
      case fgNH4:
        break;
      case fgNH2:
        if( envi.Count() == 1 )  {
          if( pivoting != NULL )
            afix = 93;
          else  {
            if( CreatedAtoms.Count() == 2 )  {
              sr = &RefMod.rDFIX.AddNew();
              sr->SetEsd(0.02);
              sr->SetValue(0.86);
              sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[0], NULL);
              sr->AddAtomPair(envi.GetBase().CAtom(), NULL, *CreatedAtoms[1], NULL);

              sr = &RefMod.rDANG.AddNew();
              sr->SetEsd(0.04);
              sr->SetValue(1.40);
              sr->AddAtomPair(*CreatedAtoms[1], NULL, *CreatedAtoms[0], NULL);

              if( envi.Count() == 1 )  {
                sr = &RefMod.rSADI.AddNew();
                sr->SetEsd(0.02);
                sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[0], NULL);
                sr->AddAtomPair(envi.GetCAtom(0), NULL, *CreatedAtoms[1], NULL);
              }
            }
          }
        }
        else if( envi.Count() == 2 )
          afix = 23;
        break;
      case fgNH1:
        if( envi.Count() == 3 )
          afix = 13;
        else if( envi.Count() == 2 )
          afix = 43;
        break;
      case fgBH1:
        if( envi.Count() == 3 )
          afix = 13;
        else if( envi.Count() == 4 ||  envi.Count() == 5 )
          afix = 153;
        break;
    }
    if( afix != 0 )  {
      TAfixGroup& ag = RefMod.AfixGroups.New(&envi.GetBase().CAtom(), afix);
      for( size_t i=0; i < CreatedAtoms.Count(); i++ )
        ag.AddDependent(*CreatedAtoms[i]);
    }
    for( size_t i=0; i < CreatedAtoms.Count(); i++ )  {
      CreatedAtoms[i]->SetPart(envi.GetBase().CAtom().GetPart());
      CreatedAtoms[i]->SetUisoOwner(&envi.GetBase().CAtom());
      if( envi.GetBase().GetType() == iOxygenZ || Group == fgCH3 )
        CreatedAtoms[i]->SetUisoScale(1.5);
      else
        CreatedAtoms[i]->SetUisoScale(1.2);
      CreatedAtoms[i]->SetUiso(4*caDefIso*caDefIso);
      RefMod.Vars.SetParam(*CreatedAtoms[i], catom_var_name_Sof, 
        RefMod.Vars.GetParam(envi.GetBase().CAtom(), catom_var_name_Sof));
      if( generated != NULL )
        generated->Add(CreatedAtoms[i]);
    }
  }
  catch( TExceptionBase& )  {}
  return false;
}
//..............................................................................


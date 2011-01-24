#include "egc.h"
#include "xmacro.h"

#include "hkl.h"
#include "sgtest.h"

#include "log.h"
#include "etable.h"

#include "integration.h"

using namespace olex;

//..............................................................................
void XLibMacros::macSG(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp &XApp = TXApp::GetInstance();
  TPtrList<TSpaceGroup>* rv = NULL;
  if( E.RetObj() != NULL )
    rv = E.GetRetObj< TPtrList<TSpaceGroup> >();

  IOlexProcessor* olx_inst = IOlexProcessor::GetInstance();

  if( olx_inst != NULL )
    olx_inst->setVar( IOlexProcessor::SGListVarName, EmptyString() );

  const TSymmLib& SymmLib = TSymmLib::GetInstance();
  TTypeList<TBravaisLatticeRef> BravaisLattices;
  if( XApp.XFile().HasLastLoader() )  {
    SymmLib.FindBravaisLattices(XApp.XFile().GetAsymmUnit(), BravaisLattices);
    size_t MatchCount = 0;
    if( BravaisLattices.Count() )  {
      XApp.NewLogEntry() << "Possible crystal systems from cell parameters";
      for( size_t i=0; i < BravaisLattices.Count(); i++ )  {
        if( BravaisLattices[i].GetB() == 0 )  {
          XApp.NewLogEntry() << BravaisLattices[i].GetA()->GetName();
          MatchCount++;
        }
      }
      if( MatchCount < BravaisLattices.Count() )  {
        XApp.NewLogEntry() << "Possible crystal systems of lower symmetry";
        for( size_t i=0; i < BravaisLattices.Count(); i++ )  {
          if( BravaisLattices[i].GetB() == -1 )
            XApp.NewLogEntry() << BravaisLattices[i].GetA()->GetName();
        }
      }
    }
  }
  TSGTest SGTest;
  XApp.NewLogEntry();
  XApp.NewLogEntry() << "Reflections count(unique in P1): " << SGTest.GetP1RefCount();
  XApp.NewLogEntry() << "Maximum/minimum intensity: " << SGTest.GetMaxI()
    << '('    << SGTest.GetMaxIS() << ')' << '/' << SGTest.GetMinI() << '(' << SGTest.GetMinIS() << ')';
  XApp.NewLogEntry() << "Average intensity/error: "
    << olxstr::FormatFloat(2, SGTest.GetAverageI()) << '/'
    << olxstr::FormatFloat(2, SGTest.GetAverageIS());
  TPtrList<TSpaceGroup> LaueClasses;
  TTypeList<TSGStats> LaueClassStats;
  for( size_t i=0; i < SymmLib.BravaisLatticeCount(); i++ )  {
    for( size_t j=0; j < SymmLib.GetBravaisLattice(i).SymmetryCount(); j++ )
      if( LaueClasses.IndexOf(SymmLib.GetBravaisLattice(i).GetSymmetry(j)) == InvalidIndex )
        LaueClasses.Add(SymmLib.GetBravaisLattice(i).GetSymmetry(j));
  }
//  for( size_t i=0; i < TSymmLib::GetInstance()->PointGroupCount(); i++ )  {
//    LaueClasses.AddACopy( &(TSymmLib::GetInstance()->GetPointGroup(i)) );
//  }

  //LaueClasses.Add( TSymmLib::GetInstance()->FindGroup("P112/m") );
  //LaueClasses.Add( TSymmLib::GetInstance()->FindGroup("P2/m11") );

  SGTest.MergeTest(LaueClasses, LaueClassStats);
  TPtrList<TSpaceGroup> CalculatedLaueClasses;
  // calculate average Sum(I-Ieq)/Count
  double averageLaueHit = 0;
  size_t laueHitCount = 0;
  for( size_t i=0; i < LaueClassStats.Count(); i++ )  {
    if( LaueClassStats[i].GetCount() != 0 )  {
      averageLaueHit += ( LaueClassStats[i].GetSummI()/LaueClassStats[i].GetCount() );
      laueHitCount ++;
    }
  }
  if( laueHitCount != 0 )
    averageLaueHit /= laueHitCount;
  // print the table of the results and evaluate possible Laue classes
  TTTable<TStrList> laueTab(LaueClassStats.Count(), 4);
  laueTab.ColName(0) = "Class";
  laueTab.ColName(1) = "(I-Ieq)/Count";
  laueTab.ColName(2) = "Count";
  laueTab.ColName(3) = "Flag";
  for( size_t i=0; i < LaueClassStats.Count(); i++ )  {
    laueTab[i][0] = LaueClassStats[i].GetSpaceGroup().GetBareName();
    if( LaueClassStats[i].GetCount() != 0 )  {
      double dv = LaueClassStats[i].GetSummI()/LaueClassStats[i].GetCount();
      laueTab[i][1] = olxstr::FormatFloat(2, dv );
      laueTab[i][1] << '(' << olxstr::FormatFloat(2, LaueClassStats[i].GetSummSI()/LaueClassStats[i].GetCount() ) << ')';
      if( dv < averageLaueHit/2 )  {
        laueTab[i][3] = '+';
        CalculatedLaueClasses.Add( &LaueClassStats[i].GetSpaceGroup() );
      }
      else
        laueTab[i][3] = '-';
    }
    else  {
      laueTab[i][1] = '-';
      CalculatedLaueClasses.Add(&LaueClassStats[i].GetSpaceGroup());
    }
    laueTab[i][2] = LaueClassStats[i].GetCount();
  }
  XApp.NewLogEntry().nl() << laueTab.CreateTXTList("1. Merge test", true, true, ' ');
  // analyse the crystal systems from the cell parameters and the diffraction matches
  // and give warnings
  for( size_t i=0; i < BravaisLattices.Count(); i++ )  {
    if( BravaisLattices[i].GetB() == 0 )  {  // exact match
      bool found = false;
      for( size_t j=0; j < CalculatedLaueClasses.Count(); j++ )  {
        if( BravaisLattices[i].GetA() == &CalculatedLaueClasses[j]->GetBravaisLattice() )  {
          found = true;
          break;
        }
      }
      if( !found )  {
        XApp.NewLogEntry(logWarning) << "Could not find match for " <<
          BravaisLattices[i].GetA()->GetName() << " crystal system";
      }
    }
  }
  // try if a higher symmetry is found and give a warnig in the case
  for( size_t i=0; i < CalculatedLaueClasses.Count(); i++ )  {
    bool found = false;
    for( size_t j=0; j < BravaisLattices.Count(); j++ )  {
      if( BravaisLattices[j].GetA() == &CalculatedLaueClasses[i]->GetBravaisLattice() )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      XApp.NewLogEntry() << "An alternative symmetry found: " <<
        CalculatedLaueClasses[i]->GetBravaisLattice().GetName();
    }
  }
  // evaluete and print systematic absences; also fill the Present elements list
  TPSTypeList<double, TCLattice*> SortedLatticeHits;
  TPtrList<TSpaceGroup>  SGToConsider;
  TPtrList<TSymmElement> PresentElements, AllElements, UniqueElements;

  TTypeList<TElementStats<TCLattice*> > LatticeHits;
  TTypeList<TSAStats> SAHits;
  SGTest.LatticeSATest( LatticeHits, SAHits );
  TTTable<TStrList> saStat(SAHits.Count(), 4);
  saStat.ColName(0) = "Symmetry element";
  saStat.ColName(1) = "Summ(I)/Count";
  saStat.ColName(2) = "Count";
  saStat.ColName(3) = "Flag";
  for( size_t i=0; i < SAHits.Count(); i++ )  {
    saStat[i][0] = SAHits[i].GetSymmElement().GetName();
    AllElements.Add( &SAHits[i].GetSymmElement() );
    if( SAHits[i].GetCount() != 0 )  {
      double v = SAHits[i].GetSummI()/SAHits[i].GetCount();
      saStat[i][1] << olxstr::FormatFloat(2, v) << '('
                               << olxstr::FormatFloat(2, SAHits[i].GetSummSI()/SAHits[i].GetCount()) << ')';
      if( SAHits[i].IsPresent() )  {
        saStat[i][3] = '+';
        if( SAHits[i].IsExcluded() )
          saStat[i][3] << '-';
        else 
          UniqueElements.Add(&SAHits[i].GetSymmElement());
        PresentElements.Add( &SAHits[i].GetSymmElement() );
      }
      else 
        saStat[i][3] = '-';
    }
    else  {
      saStat[i][3] << '-' << '+';
      PresentElements.Add( &SAHits[i].GetSymmElement() );
    }
    saStat[i][2] = SAHits[i].GetCount();
  }
  XApp.NewLogEntry().nl() << saStat.CreateTXTList("2. Systematic absences test", true, true, ' ');
  // print the cell centering statistics
  TTTable<TStrList> latTab(LatticeHits.Count(), 5);
  latTab.ColName(0) = "Centering";
  latTab.ColName(1) = "Strong I/Count";
  latTab.ColName(2) = "Count";
  latTab.ColName(3) = "Weak I/Count";
  latTab.ColName(4) = "Count";
  TPtrList<TCLattice>  ChosenLats;
  const double threshold = SGTest.GetAverageI()/20;
  for( size_t i=0; i < LatticeHits.Count(); i++ )  {
    if( LatticeHits[i].GetStrongCount() != 0 )  {
      if( LatticeHits[i].GetWeakCount() != 0 )
        SortedLatticeHits.Add(LatticeHits[i].GetSummWeakI()/LatticeHits[i].GetWeakCount(), LatticeHits[i].GetObject() );
      else
        SortedLatticeHits.Add(-1, LatticeHits[i].GetObject() );
    }
  }
  if( SortedLatticeHits.GetKey(0) > threshold || SortedLatticeHits.GetKey(0) == -1 )
    ChosenLats.Add(SymmLib.FindLattice("P"));
  for( size_t i=0; i < SortedLatticeHits.Count(); i++ )  {
    if( SortedLatticeHits.GetKey(i) == -1 || SortedLatticeHits.GetKey(i) < threshold )
      ChosenLats.Add( SortedLatticeHits.GetObject(i) );
  }
  if( ChosenLats.IsEmpty() )
    ChosenLats.Add( SortedLatticeHits.GetObject(0) );

  for( size_t i=0; i < LatticeHits.Count(); i++ )  {
    latTab[i][0] = LatticeHits[i].GetObject()->GetSymbol();
    if( LatticeHits[i].GetStrongCount() != 0 )
      latTab[i][1] << olxstr::FormatFloat(2, LatticeHits[i].GetSummStrongI()/LatticeHits[i].GetStrongCount() )
                               << '(' << olxstr::FormatFloat(2, LatticeHits[i].GetSummStrongSI()/LatticeHits[i].GetStrongCount() )  << ')';
    else
      latTab[i][1] = '-';
    latTab[i][2] = LatticeHits[i].GetStrongCount();
    if( LatticeHits[i].GetWeakCount() != 0 )
      latTab[i][3] << olxstr::FormatFloat(2, LatticeHits[i].GetSummWeakI()/LatticeHits[i].GetWeakCount() )
                               << '(' << olxstr::FormatFloat(2, LatticeHits[i].GetSummWeakSI()/LatticeHits[i].GetWeakCount() )  << ')';

    else
      latTab[i][3] = '-';
    latTab[i][4] = LatticeHits[i].GetWeakCount();
  }
  XApp.NewLogEntry().nl() << latTab.CreateTXTList("3. Cell centering test", true, true, ' ');

  olxstr Tmp;
  for( size_t i=0; i < ChosenLats.Count(); i++ )  {
    Tmp << ChosenLats[i]->GetSymbol();
    if( (i+1) < ChosenLats.Count() )  Tmp << ',';
  }
  XApp.NewLogEntry() << "Chosen lattice(s): " << Tmp;
  // print current spacegroup
  TSpaceGroup* sg = NULL;
  try  { sg = &XApp.XFile().GetLastLoaderSG();  }
  catch(...)  {}
  if( sg != NULL )  {
    olxstr Tmp("Current space group: ");
    Tmp << sg->GetName();
    if( sg->GetFullName().Length() )
      Tmp << " (" << sg->GetFullName() << ')';

    Tmp << " #" << sg->GetNumber();
    XApp.NewLogEntry();
    XApp.NewLogEntry() << Tmp;
  }
  // nor systematic absences in P-1!
  if( CalculatedLaueClasses.Count() == 1 &&
      CalculatedLaueClasses[0] == SymmLib.FindGroup("P-1") )  {
    PresentElements.Clear();
  }
  if( !PresentElements.IsEmpty() )  {
    for( size_t i=0; i < SymmLib.SGCount(); i++ )  {
      TSpaceGroup& sg = SymmLib.GetGroup(i);
      if( sg.GetNumber() > 230 )  continue;
      bool matchLaueClass = false;
      for( size_t j=0; j < CalculatedLaueClasses.Count(); j++ )  {
        if( &sg.GetLaueClass() == CalculatedLaueClasses[j] )  {
          matchLaueClass = true;
          break;
        }
      }
      if( !matchLaueClass )  continue;
      bool matchLattice = false;
      for( size_t j=0; j < ChosenLats.Count(); j++ ) {
        if( &sg.GetLattice() == ChosenLats[j] )  {
          matchLattice = true;
          break;
        }
      }
      if( !matchLattice )  continue;
      SGToConsider.Add( &sg);
    }
    if( !SGToConsider.IsEmpty() )  {
      XApp.NewLogEntry() << "Testing " << SGToConsider.Count() << " selected space groups";
    }
  }
  TPtrList<TSpaceGroup> FoundSpaceGroups;
  // merge test is the only way then...
  if( UniqueElements.IsEmpty() && !PresentElements.IsEmpty() )  {
    TTypeList<TSGStats> sgMergeStat;
    SGTest.MergeTest(SGToConsider, sgMergeStat);
    TPSTypeList<double, TSGStats*> sortedSGMergeResults;
    for( size_t i=0; i < SGToConsider.Count(); i++ )  {
      if( sgMergeStat[i].GetCount() == 0 )  continue;
      double dv = sgMergeStat[i].GetSummI()/sgMergeStat[i].GetCount();
      const TSpaceGroup& sg = sgMergeStat[i].GetSpaceGroup();
      double k = (double)((sg.MatrixCount()+1)*(sg.GetLattice().VectorCount()+1));
      if( sg.IsCentrosymmetric() ) // add just 1, not multiply by 2 - leads to distrortion
        k++;
      dv /= k;
      //if( dv < 0 )  dv = 0;
      //double mult = pow(10, olx_min(olx_abs(dv)/threshold, 100.0) );
      //while( dv < 1 )  dv ++;
      //dv *= mult;
      sortedSGMergeResults.Add( dv, &sgMergeStat[i] );
    }
    TTTable<TStrList> sgTab(sortedSGMergeResults.Count(), 3);
    sgTab.ColName(0) = "Class";
    sgTab.ColName(1) = "(I-Ieq)/Count";
    sgTab.ColName(2) = "Count";
    for( size_t i=sortedSGMergeResults.Count()-1; i != InvalidIndex; i-- )  {
      sgTab[i][0] = sortedSGMergeResults.GetObject(i)->GetSpaceGroup().GetName();
      sgTab[i][1] = olxstr::FormatFloat(2, 
        sortedSGMergeResults.GetObject(i)->GetSummI()/sortedSGMergeResults.GetObject(i)->GetCount() );
      sgTab[i][1] << '(' << olxstr::FormatFloat(2, 
        sortedSGMergeResults.GetObject(i)->GetSummSI()/sortedSGMergeResults.GetObject(i)->GetCount() ) << ')';
      sgTab[i][2] = sortedSGMergeResults.GetObject(i)->GetCount();
    }
    XApp.NewLogEntry().nl() << sgTab.CreateTXTList("4. Merge test (no unique systematic absences found)", true, true, ' ');
    size_t cs_cnt = 0, noncs_cnt = 0;
    for( size_t i=0; i < sortedSGMergeResults.Count(); i++ )  {
      if( cs_cnt < 3 && sortedSGMergeResults.GetObject(i)->GetSpaceGroup().IsCentrosymmetric() )  {
        FoundSpaceGroups.Add( &sortedSGMergeResults.GetObject(i)->GetSpaceGroup() );
        cs_cnt++;
      }
      if( noncs_cnt < 3 && !sortedSGMergeResults.GetObject(i)->GetSpaceGroup().IsCentrosymmetric() )  {
        FoundSpaceGroups.Add( &sortedSGMergeResults.GetObject(i)->GetSpaceGroup() );
        noncs_cnt++;
      }
    }
  }
  else  {
    TTypeList<TElementStats<TSpaceGroup*> > SATestResults;
    SGTest.WeakRefTest(SGToConsider, SATestResults);
    TPSTypeList<double, AnAssociation3<TElementStats<TSpaceGroup*>*, size_t, size_t>* > sortedSATestResults;

    for( size_t i=0; i < SATestResults.Count(); i++ )  {
      if( SATestResults[i].GetWeakCount() == 0  ) continue;
      double v = SATestResults[i].GetSummWeakI()/(SATestResults[i].GetWeakCount());
      if( v < 0 )  v = 0;
      double mult = pow(10, olx_min(olx_abs(v)/threshold, 100.0) );
      while( v < 1 )  v ++;
      v *= mult;
      sortedSATestResults.Add( SATestResults[i].GetWeakCount()/v, new AnAssociation3<TElementStats<TSpaceGroup*>*, size_t, size_t>(&SATestResults[i], 0, 0));
    }

    //TPtrList< AnAssociation3<TElementStats<TSpaceGroup*>*, size_t, size_t> >  
    TTTable<TStrList> sgTab(sortedSATestResults.Count(), 8);
    sgTab.ColName(0) = "SG";
    sgTab.ColName(1) = "Strong I/Count";
    sgTab.ColName(2) = "Count";
    sgTab.ColName(3) = "Weak I/Count";
    sgTab.ColName(4) = "Count";
    sgTab.ColName(5) = "Laue class";
    sgTab.ColName(6) = "SA match";
    sgTab.ColName(7) = "SA/SG SA";
    size_t maxElementFound = 0, maxUniqueElementFound = 0;
    bool FilterByElementCount = false;
    for( size_t i=0; i < sortedSATestResults.Count(); i++ )  {
      sgTab[i][0] = sortedSATestResults.GetObject(i)->GetA()->GetObject()->GetName();
      if( sortedSATestResults.GetObject(i)->GetA()->GetStrongCount() != 0 )  {
        sgTab[i][1] = olxstr::FormatFloat(2,
          sortedSATestResults.GetObject(i)->GetA()->GetSummStrongI()/sortedSATestResults.GetObject(i)->GetA()->GetStrongCount() );
      }
      else  {
        sgTab[i][1] = '-';
      }
      sgTab[i][2] = sortedSATestResults.GetObject(i)->GetA()->GetStrongCount();
      if( sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() != 0 )  {
        sgTab[i][3] = olxstr::FormatFloat(2,
          sortedSATestResults.GetObject(i)->GetA()->GetSummWeakI()/sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() );
      }
      else  {
        sgTab[i][3] = '-';
      }
      sgTab[i][4] = sortedSATestResults.GetObject(i)->GetA()->GetWeakCount();
      sgTab[i][5] = sortedSATestResults.GetObject(i)->GetA()->GetObject()->GetLaueClass().GetBareName();
      if( !PresentElements.IsEmpty() )  {
        smatd_list sgMl;
        sortedSATestResults.GetObject(i)->GetA()->GetObject()->GetMatrices( sgMl, mattAll);
        TPtrList<TSymmElement> sgElmAll, sgElmFound;
        TSpaceGroup::SplitIntoElements(sgMl, AllElements, sgElmAll);
        TSpaceGroup::SplitIntoElements(sgMl, PresentElements, sgElmFound);
        // validate all sg elements are in the list of present ones
        bool all_present = true;
        for( size_t j=0; j < sgElmAll.Count(); j++ )  {
          if( PresentElements.IndexOf(sgElmAll[j]) == InvalidIndex )  {
            all_present = false;
            break;
          }
        }
        sgTab[i][6] << olxstr::FormatFloat(0, (double)sgElmFound.Count()*100/PresentElements.Count()) << '%';
        if( all_present )  {
          size_t unique_elm = 0;
          for( size_t j=0; j < sgElmFound.Count(); j++ )
            if( UniqueElements.IndexOf(sgElmFound[j]) != InvalidIndex )
              unique_elm++;
          if( unique_elm > maxUniqueElementFound )
            maxUniqueElementFound = unique_elm;
          if( sgElmFound.Count() > maxElementFound )
            maxElementFound = sgElmFound.Count();
          if( sgElmFound.Count() <= maxElementFound )
            FilterByElementCount = true;
          sgTab[i][7] << '+';
          sortedSATestResults.GetObject(i)->B() = sgElmFound.Count();
          sortedSATestResults.GetObject(i)->C() = unique_elm;
        }
        else  {
          sgTab[i][7] << '-';
          sortedSATestResults.GetObject(i)->B() = 0;
          sortedSATestResults.GetObject(i)->C() = 0;
        }
      }
    }
    // print the result of analysis
    if( sortedSATestResults.Count() != 0 )
      XApp.NewLogEntry().nl() << sgTab.CreateTXTList("4. Space group test", true, true, ' ');
    if( !PresentElements.IsEmpty() )  {
      TPtrList<TSpaceGroup> ToAppend;  // alternative groups, but lower probability
      for( size_t i=sortedSATestResults.Count()-1; i != InvalidIndex; i-- )  {
        if( sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() != 0 )  {
          double v = sortedSATestResults.GetObject(i)->GetA()->GetSummWeakI()/sortedSATestResults.GetObject(i)->GetA()->GetWeakCount();
          if( v > SGTest.GetAverageI()/5 )
            break;
        }
        if( FilterByElementCount )  {
          if( maxElementFound > UniqueElements.Count() )  {
            if( sortedSATestResults.GetObject(i)->GetC() == maxUniqueElementFound )  {
              FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
            }  // this is still a good match!
            else if( sortedSATestResults.GetObject(i)->GetB() == maxElementFound )
              ToAppend.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
          }
          else  {
            if( sortedSATestResults.GetObject(i)->GetB() == maxElementFound )
              FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
          }
        }
        else
          FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
      }
      FoundSpaceGroups.AddList( ToAppend );
      // try to recover...
      if( FilterByElementCount && FoundSpaceGroups.IsEmpty() )  {
        for( size_t i=sortedSATestResults.Count()-1; i >= olx_max(0, sortedSATestResults.Count()-6) ; i-- )  {
          if( sortedSATestResults.GetObject(i)->GetB() == maxElementFound )
            FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
          if( i== 0 )  break;
        }
      }
      olxstr amb_sg;
      for( size_t i=0; i < SATestResults.Count(); i++ )  {
        if( SATestResults[i].GetWeakCount() == 0 && 
          (SATestResults[i].GetObject()->HasTranslations() || SATestResults[i].GetObject()->GetLattice().VectorCount() != 0) ) 
        {
          if( !amb_sg.IsEmpty() )
            amb_sg << ", ";
          amb_sg << SATestResults[i].GetObject()->GetName();
        }
      }
      if( !amb_sg.IsEmpty() )  {
        XApp.NewLogEntry() << "Ambiguous space groups (statistics incomplete to determine):";
        TStrList Output;
        Output.Hyphenate(amb_sg, 80);
        XApp.NewLogEntry() << Output;
      }
    }
    else  {
      // three hits from here
      for( size_t i=sortedSATestResults.Count()-1; i != InvalidIndex; i-- )  {
        if( sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() != 0 )  {
          double v = sortedSATestResults.GetObject(i)->GetA()->GetSummWeakI()/sortedSATestResults.GetObject(i)->GetA()->GetWeakCount();
          if( v > SGTest.GetAverageI()/5 )
            break;
        }
        FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
      }
      // check all spacegroups without translations as well
      TPtrList<TSpaceGroup> laueClassGroups, possibleGroups;
      for( size_t i=CalculatedLaueClasses.Count()-1; i != InvalidIndex ; i-- )  {
        laueClassGroups.Clear();
        SymmLib.FindLaueClassGroups(*CalculatedLaueClasses[i], laueClassGroups);
        for( size_t j=0; j < laueClassGroups.Count(); j++ )  {
          if( ChosenLats.IndexOf(laueClassGroups[j]->GetLattice()) == InvalidIndex )  continue;
          if( !laueClassGroups[j]->HasTranslations() )
            FoundSpaceGroups.Add(laueClassGroups[j]);
        }
      }
    }
    for( size_t i=0; i < sortedSATestResults.Count(); i++ )
      delete sortedSATestResults.GetObject(i);
  } // Unique elements present
  if( !FoundSpaceGroups.IsEmpty() )  {
    XApp.NewLogEntry().nl() << "Possible space groups:";
    olxstr tmp, sglist;
    TStrList Output, Output1;
    size_t cscount = 0, ncscount = 0;
    for( size_t i=0; i < FoundSpaceGroups.Count(); i++ )  {
      sglist << FoundSpaceGroups[i]->GetName() << ';';
      tmp = "  ";
      tmp << FoundSpaceGroups[i]->GetName();
      tmp.Format(10, true, ' ');
      tmp << "(#" << FoundSpaceGroups[i]->GetNumber() << ", Laue class " <<
      FoundSpaceGroups[i]->GetLaueClass().GetBareName() << ", Point group " <<
      FoundSpaceGroups[i]->GetPointGroup().GetBareName() << ')';

      if( FoundSpaceGroups[i]->IsCentrosymmetric() )  {
        Output.Add(tmp);  cscount++;
      }
      else  {
        Output1.Add(tmp);  ncscount++;
      }
    }
    if( olx_inst != NULL )
      olx_inst->setVar( IOlexProcessor::SGListVarName, sglist );

    XApp.NewLogEntry() << "Noncentrosymmetric:";
    if( !Output1.IsEmpty() )
      XApp.NewLogEntry() << Output1;
    else
      XApp.NewLogEntry() << "  None";
    XApp.NewLogEntry() << "Centrosymmetric:";
    if( !Output.IsEmpty() )
      XApp.NewLogEntry() << Output;
    else
      XApp.NewLogEntry() << "  None";

    if( rv != NULL )  {
      for( size_t i=0; i < FoundSpaceGroups.Count(); i++ )
        rv->Add(FoundSpaceGroups[i]);
    }
  }
  else  {
    XApp.NewLogEntry(logError) << "Could not find any suitable space group";
  }
}

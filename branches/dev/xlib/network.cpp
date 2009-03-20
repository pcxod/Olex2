//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "network.h"
#include "satom.h"
#include "sbond.h"

#include "actions.h"

#include "lattice.h"
#include "asymmunit.h"

#include "bapp.h"
#include "log.h"

#include "egraph.h"

#include "olxmps.h"
#include "estopwatch.h"
#include "edict.h"
#include "emath.h"

#undef GetObject

double TNetwork_FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms, 
                                    smatdd& res, const vec3d& centA, const vec3d centB, bool TryInversion)  {
  TTypeList< AnAssociation2<vec3d,vec3d> > crds;
  crds.SetCapacity( atoms.Count() );
  const TAsymmUnit& au = atoms[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  if( TryInversion )  {
    for( int i=0; i < atoms.Count(); i++ )  {
      vec3d v = atoms[i].GetB()->ccrd() * -1;
      crds.AddNew(atoms[i].GetA()->crd(), au.CellToCartesian(v) );
    }
  }
  else  {
    for( int i=0; i < atoms.Count(); i++ )  {
      vec3d v = atoms[i].GetB()->ccrd();
      crds.AddNew( atoms[i].GetA()->crd(), au.CellToCartesian(v) );
    }
  }
  res.t = centA;
  return TNetwork::FindAlignmentMatrix(crds, res.t, centB, res);
}
double TNetwork_FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms, smatdd& res, bool TryInversion)  {
  TTypeList< AnAssociation2<vec3d,vec3d> > crds;
  crds.SetCapacity( atoms.Count() );
  const TAsymmUnit& au = atoms[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  if( TryInversion )  {
    for( int i=0; i < atoms.Count(); i++ )  {
      vec3d v = atoms[i].GetB()->ccrd() * -1;
      crds.AddNew(atoms[i].GetA()->crd(), au.CellToCartesian(v) );
    }
  }
  else  {
    for( int i=0; i < atoms.Count(); i++ )  {
      vec3d v = atoms[i].GetB()->ccrd();
      crds.AddNew( atoms[i].GetA()->crd(), au.CellToCartesian(v) );
    }
  }
  double sumA = 0, sumB = 0;
  vec3d centA, centB;
  for( int i=0; i < atoms.Count(); i++ )  {
    centA += crds[i].GetA()*atoms[i].GetA()->CAtom().GetOccu()*atoms[i].GetA()->GetAtomInfo().GetMr();
    sumA += atoms[i].GetA()->CAtom().GetOccu()*atoms[i].GetA()->GetAtomInfo().GetMr();
    centB += crds[i].GetB()*atoms[i].GetB()->CAtom().GetOccu()*atoms[i].GetB()->GetAtomInfo().GetMr();
    sumB += atoms[i].GetB()->CAtom().GetOccu()*atoms[i].GetB()->GetAtomInfo().GetMr();
  }
  res.t = centA/sumA;
  return TNetwork::FindAlignmentMatrix(crds, res.t, centB/sumB, res);
}

//---------------------------------------------------------------------------
// TNetwork function bodies
//---------------------------------------------------------------------------
TNetwork::TNetwork(TLattice* P, TNetwork *N) : TBasicNode<TNetwork, TSAtom, TSBond>(N)  {
  Lattice = P;
}
//..............................................................................
TNetwork::~TNetwork()  {
  return;
}
//..............................................................................
// sorts atoms according to the distcance from {0,0,0}
int AtomsSortByDistance(const TSAtom* A, const TSAtom* A1)  {
  const double d = A->crd().QLength() - A1->crd().QLength();
  if( d < 0 )  return -1;
  if( d > 0 )  return 1;
  return 0;
}
//..............................................................................
void TNetwork::SortNodes()  {
  Nodes.QuickSorter.SortSF(Nodes, AtomsSortByDistance);
}
//..............................................................................
void TNetwork::TDisassembleTaskRemoveSymmEq::Run(long index)  {
  if( Atoms[index]->CAtom().GetExyzGroup() != NULL )  return;
  if( (Atoms[index]->GetTag() & 0x0002) != 0 )  return;
  const int ac = Atoms.Count();
  for( int i=index+1; i < ac; i++ )  {
    if( olx_abs(Distances[0][index] - Distances[0][i]) > 0.01 )  return;
    if( Atoms[index]->crd().QDistanceTo( Atoms[i]->crd() ) < 0.0001 )  {
      Atoms[index]->AddMatrices(Atoms[i]);
      Atoms[i]->SetTag(2);            // specify that the node has to be deleted
    }
  }
}
void TNetwork::TDisassembleTaskCheckConnectivity::Run(long index)  {
  const int this_p = Atoms[index]->CAtom().GetPart();
  const int ac = Atoms.Count();
  for( int i=index+1; i < ac; i++ )  {
    if( olx_abs(Distances[0][i] - Distances[0][index]) > dcMaxCBLength )  return;
    if( olx_abs(Distances[1][i] - Distances[1][index]) > dcMaxCBLength )  continue;
    if( olx_abs(Distances[2][i] - Distances[2][index]) > dcMaxCBLength )  continue;
    if( olx_abs(Distances[3][i] - Distances[3][index]) > dcMaxCBLength  )  continue;

    const double D = Atoms[index]->crd().QDistanceTo( Atoms[i]->crd());
    const double D1 = sqr(Atoms[index]->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta);
    if(  D < D1 )  {
      const int that_p = Atoms[i]->CAtom().GetPart();
      if( this_p == 0 || that_p == 0 )  {
        Atoms[index]->AddNode(*Atoms[i]);
        Atoms[i]->AddNode(*Atoms[index]);  // crosslinking
      }
      else if( (this_p == that_p) && (this_p > 0)  )  {
        Atoms[index]->AddNode(*Atoms[i]);
        Atoms[i]->AddNode(*Atoms[index]);  // crosslinking
      }
    }
  }
}
//..............................................................................
void TNetwork::Disassemble(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList* InterBonds)  {
  if( Atoms.Count() < 2 )  return;
  TStopWatch sw(__FUNC__);
  sw.start("Initialising");
  TNetwork *Net;
  //TSAtom *A;
  int ac;
  double** Distances = new double* [4];
  double  Delta = GetLattice().GetDelta();
  Atoms.QuickSorter.SortSF(Atoms, AtomsSortByDistance);
  Distances[0] = new double[ Atoms.Count() ];  // distsnaces from {0,0,0} to an atom
  ac = Atoms.Count();
  for( int i = 0; i < ac; i++ )  {
    Distances[0][i] = Atoms[i]->crd().Length();
    Atoms[i]->SetTag(1);
    Atoms[i]->SetNetId(-1);
  }
  // find & remove symmetrical equivalenrs from AllAtoms
  TDisassembleTaskRemoveSymmEq searchEqTask( Atoms, Distances);
  // profiling has shown it gives no benifit and makes the process slow
  //TListIteratorManager<TDisassembleTaskRemoveSymmEq> searchEq(searchEqTask, Atoms.Count(), tQuadraticTask, 100);
  ac = Atoms.Count();
  for( int i=0; i < ac; i++ )
    searchEqTask.Run(i);
  // removing symmetrical equivalents from the Atoms list (passes as param)
  //............................................
  for( int i = 0; i < ac; i++ )  {
    if( (Atoms[i]->GetTag() & 0x0002) != 0 )  {
      delete Atoms[i];
      Atoms[i] = NULL;
      continue;
    }
    Atoms[i]->Clear();
    // preallocate memory to improve mulithreading
    Atoms[i]->SetCapacity( Atoms[i]->NodeCount() + Atoms[i]->CAtom().AttachedAtomCount() );
  }
  Atoms.Pack();
  if( Atoms.IsEmpty() )  return;
  //............................................

  ac = Atoms.Count();
  Distances[1] = new double[ ac ];
  Distances[2] = new double[ ac ];
  Distances[3] = new double[ ac ];
  for( int i = 0; i < ac; i++ )  {  // recalculate distances and remove some function calls
    TSAtom* A = Atoms[i];
    Distances[0][i] = A->crd().Length();
    Distances[1][i] = A->crd()[0];
    Distances[2][i] = A->crd()[1];
    Distances[3][i] = A->crd()[2];
  }
  sw.start("Connectivity analysis");
  TDisassembleTaskCheckConnectivity searchConTask( Atoms, Distances, Delta);
  TListIteratorManager<TDisassembleTaskCheckConnectivity> searchCon(searchConTask, Atoms.Count(), tQuadraticTask, 100);
  sw.start("Creating bonds");
  CreateBondsAndFragments(Atoms, Frags);
  sw.start("Searching H-bonds");
  // preallocate 50 Hbonds per fragment
  if( InterBonds != NULL )  
    InterBonds->SetCapacity( InterBonds->Count() + Frags.Count()*50); 
  THBondSearchTask searchHBTask( Atoms, InterBonds, Distances, GetLattice().GetDeltaI());
  TListIteratorManager<THBondSearchTask> searchHB(searchHBTask, Atoms.Count(), tQuadraticTask, 100);
  sw.start("Finalising");
  delete [] Distances[0];
  delete [] Distances[1];
  delete [] Distances[2];
  delete [] Distances[3];
  delete [] Distances;
  sw.stop();
  if( TBasicApp::GetInstance()->IsProfiling() )
    sw.print( TBasicApp::GetLog(), &TLog::Info );
}
//..............................................................................
void TNetwork::CreateBondsAndFragments(TSAtomPList& Atoms, TNetPList& Frags)  {
  // creating bonds
  const int ac = Atoms.Count();
  for( int i=0; i < ac; i++ )  {
    TSAtom* A1 = Atoms[i];
    if( A1->GetTag() != 0 )  {
      TNetwork* Net = new TNetwork(&GetLattice(), this);
      Net->AddNode(*A1);
      Frags.Add( Net );
      A1->SetNetwork(*Net);
      A1->SetTag(0);
      for( int j=0; j < Net->NodeCount(); j++ )  {
        TSAtom& A2 = Net->Node(j);
        const int a2_cnt = A2.NodeCount(); 
        for( int k=0; k < a2_cnt; k++ )  {
          TSAtom& A3 = A2.Node(k);
          if( A3.GetTag() != 0 )  {
            Net->AddNode(A3);
            A3.SetNetwork(*Net);
            TSBond* B = new TSBond(Net);
            B->SetType(sotBond);
            B->SetA(A2); B->SetB(A3);
            A2.AddBond(*B);  A3.AddBond(*B);
            Net->AddBond(*B);
            A3.SetTag(0);
          }
          else if( A3.GetNetId() > j )  {  // the atom is in the list, but has not been processes
            TSBond* B = new TSBond(Net);                  // in this case we need to create a bond
            B->SetType(sotBond);
            B->SetA(A2); B->SetB(A3);
            A2.AddBond(*B);  A3.AddBond(*B);
            Net->AddBond(*B);
          }
        }
      }
    }
  }
}
//..............................................................................
void TNetwork::THBondSearchTask::Run(long ind)  {
  TSAtom *AA = NULL,
         *DA = NULL;
  TSAtom* A1 = Atoms[ind];
  int aiIndex = A1->GetAtomInfo().GetIndex();
  if( aiIndex == iHydrogenIndex )
    AA = A1;
  else if( aiIndex == iNitrogenIndex || aiIndex == iOxygenIndex || aiIndex == iFluorineIndex ||
      aiIndex == iChlorineIndex || aiIndex == iSulphurIndex )
    DA = A1;

  if( AA == NULL && DA == NULL )  return;

  const int this_p = A1->CAtom().GetPart();
  const int ac = Atoms.Count();
  for( int i=ind+1; i < ac; i++ )  {
    if( (Distances[0][ind] - Distances[0][i]) > dcMaxCBLength ||
        (Distances[0][ind] - Distances[0][i]) < -dcMaxCBLength )  return;
    if( (Distances[1][ind] - Distances[1][i]) > dcMaxCBLength ||
        (Distances[1][ind] - Distances[1][i]) < -dcMaxCBLength )  continue;
    if( (Distances[2][ind] - Distances[2][i]) > dcMaxCBLength ||
        (Distances[2][ind] - Distances[2][i]) < -dcMaxCBLength )  continue;
    if( (Distances[3][ind] - Distances[3][i]) > dcMaxCBLength ||
        (Distances[3][ind] - Distances[3][i]) < -dcMaxCBLength )  continue;

    int aiIndex1 = Atoms[i]->GetAtomInfo().GetIndex();
    if( !((AA != NULL && (aiIndex1 == iNitrogenIndex || aiIndex1 == iOxygenIndex||
                        aiIndex1 == iFluorineIndex || aiIndex1 == iChlorineIndex ||
                        aiIndex1 == iSulphurIndex))  ||
          (DA != NULL && aiIndex1 == iHydrogenIndex) ) )  continue;

    if( A1->GetNetwork() == Atoms[i]->GetNetwork() )  {
      if( A1->IsConnectedTo( *Atoms[i]) )  continue;
      // check for N-C-H (N-C) bonds are not leagal
      TSAtom *CSA = ((AA!=NULL)?AA:Atoms[i]),
             *NA  = ((DA!=NULL)?DA:Atoms[i]);
      bool connected = false;
      for( int j=0; j < CSA->NodeCount(); j++ )  {
        if( CSA->Node(j).GetAtomInfo() == iQPeakIndex )  // 17/05/2007 - skip the Q peaks!
          continue;
        if( CSA->Node(j).IsConnectedTo(*NA) )  {
          connected = true;
          break;
        }
      }
      if( connected )  continue;

      const int that_p = Atoms[i]->CAtom().GetPart();
      const double D = A1->crd().QDistanceTo( Atoms[i]->crd() );
      const double D1 = sqr(A1->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta);
      if(  D < D1 )  {
         if( (this_p == that_p && this_p >= 0) || this_p == 0 || that_p == 0 )  {
          TSBond* B = new TSBond(&A1->GetNetwork());
          B->SetType(sotHBond);
          B->SetA(*A1);
          B->SetB(*Atoms[i]);
          A1->AddBond(*B);
          Atoms[i]->AddBond(*B);
          A1->GetNetwork().AddBond(*B);
        }
      }
    }
    else  {
      const int that_p = Atoms[i]->CAtom().GetPart();
      const double D = A1->crd().QDistanceTo( Atoms[i]->crd() );
      const double D1 = sqr(A1->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta);
      if(  D < D1 )  {
        if( (this_p == that_p && this_p >= 0) || this_p == 0 || that_p == 0 )  {
          TSBond* B = new TSBond( &A1->GetNetwork() );
          B->SetType(sotHBond);
          B->SetA(*A1);
          B->SetB(*Atoms[i]);
          if( Bonds != NULL )
            Bonds->Add( B );
        }
      }
    }
  }
}
//..............................................................................
bool TNetwork::CBondExists(const TCAtom& CA1, const TCAtom& CA2, const double& D) const  {
  if(  D < (CA1.GetAtomInfo().GetRad1() + CA2.GetAtomInfo().GetRad1() + GetLattice().GetDelta() ) )  {
    if( (CA1.GetPart() == CA2.GetPart() && CA2.GetPart() >= 0 ) || 
         CA1.GetPart() == 0 ||CA2.GetPart() == 0 )
      return true;
  }
  return false;
}
//..............................................................................
bool TNetwork::HBondExists(const TCAtom& CA1, const TCAtom& CA2, const double& D) const  {
  if(  D < (CA1.GetAtomInfo().GetRad1() + CA2.GetAtomInfo().GetRad1() + GetLattice().GetDeltaI() ) )  {
    if( (CA1.GetPart() == CA2.GetPart() && CA2.GetPart() >=0 ) || 
      CA1.GetPart() == 0 || CA2.GetPart() == 0 )
      return true;
  }
  return false;
}
//..............................................................................
// HELPER function
class TNetTraverser  {
  olxstr Data;
public:
  TNetTraverser() {  Data.SetIncrement(1024);  }
  bool OnItem(const TEGraphNode<int, TSAtom*>& v) {
    olxstr tmp(v.GetData(), v.Count()*6);
    tmp << '{';
    for( int i=0; i < v.Count(); i++ )  {
      tmp << v.Item(i).GetData();
      if( (i+1) < v.Count() )  tmp << ',';
    }
    tmp << '}';
    Data << tmp;
    return true;
  }
  const olxstr& GetData()  const  {  return Data;  }
  void ClearData()  {  Data = EmptyString;  }
};

void ResultCollector( TEGraphNode<int,TSAtom*>& subRoot,
                        TEGraphNode<int,TSAtom*>& Root,
                        TTypeList< AnAssociation2<int, int> >& res )  {
  res.AddNew( subRoot.GetObject()->GetNetId(), Root.GetObject()->GetNetId() );
  for( int i=0; i < olx_min(subRoot.Count(),Root.Count()); i++ )
    ResultCollector( subRoot.Item(i), Root.Item(i), res );
}
void ResultCollector( TEGraphNode<int,TSAtom*>& subRoot,
                        TEGraphNode<int,TSAtom*>& Root,
                        TTypeList< AnAssociation2<TSAtom*, TSAtom*> >& res )  {
  res.AddNew( subRoot.GetObject(), Root.GetObject());
  for( int i=0; i < olx_min(subRoot.Count(),Root.Count()); i++ )
    ResultCollector( subRoot.Item(i), Root.Item(i), res );
}

/*
void ExpandGraphNode( TTypeList< TEGraphNode<int,TSAtom*>* >& allNodes, TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  if( node->GetTag() == 1 )  return;
  node->Tag( 1 );
  for( int i=0; i < node->NodeCount(); i++ )  {
    TSAtom* sa = (TSAtom*)node->Node(i);
    if( sa->GetTag() != 0 )  continue;
    allNodes.AddACopy( &graphNode.NewNode( sa->GetAtomInfo()->GetIndex(), sa ) );
  }
}
void BuildGraph( TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  TTypeList< TEGraphNode<int,TSAtom*>* > allNodes;

  ExpandGraphNode(allNodes, graphNode, node);

  for( int i=0; i < allNodes.Count(); i++ )
    ExpandGraphNode( allNodes, *allNodes[i], allNodes[i]->GetObject() );
}
*/

void BreadthFirstTag(TSAtomPList& all, TSAtom* node)  {
  for( int i=0; i < node->NodeCount(); i++ )  {
    TSAtom& sa = node->Node(i);
    if( sa.GetTag() != 0 )  continue;
    all.Add( &sa )->SetTag( node->GetTag() + 1);
  }
}
void BreadthFirstTags(TSAtom* sa)  {
  TSAtomPList all;
  all.SetCapacity( sa->GetNetwork().NodeCount() );
  all.Add( sa );
  sa->SetTag(1);
  BreadthFirstTag(all, sa);
  for( int i=0; i < all.Count(); i++ )  {
    BreadthFirstTag( all, all[i] );
    //all[i]->CAtom()->Label() = all[i]->GetTag();
  }
}

void ExpandGraphNode(TEGraphNode<int,TSAtom*>& graphNode)  {
  for( int i=0; i < graphNode.GetObject()->NodeCount(); i++ )  {
    TSAtom& sa = graphNode.GetObject()->Node(i);
    if( sa.GetTag() <= graphNode.GetObject()->GetTag() )  continue;
    ExpandGraphNode( graphNode.NewNode( sa.GetAtomInfo().GetIndex(), &sa ) );
  }
}

void BuildGraph( TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  BreadthFirstTags(node);
  ExpandGraphNode(graphNode);
}

struct GraphAnalyser  {
  TEGraphNode<int,TSAtom*> &RootA, &RootB; 
  int CallsCount;
  bool Invert;
  vec3d bCent, aCent;
  smatdd alignmentMatrix, bestMatrix;
  double minRms;
  GraphAnalyser(TEGraphNode<int,TSAtom*>& rootA, TEGraphNode<int,TSAtom*>& rootB, 
    const vec3d& acent, const vec3d& bcent) :
    RootA(rootA), RootB(rootB), CallsCount(0), Invert(false), aCent(acent), bCent(bcent) {
      minRms = -1;
    }

  double CalcRMS()  {
    TTypeList< AnAssociation2<TSAtom*,TSAtom*> > matchedAtoms;
    matchedAtoms.SetCapacity(1024);
    ResultCollector( RootA, RootB, matchedAtoms);
    CallsCount++;
    //const double rms = TNetwork_FindAlignmentMatrix(matchedAtoms, alignmentMatrix, aCent, bCent, Invert);
    const double rms = TNetwork_FindAlignmentMatrix(matchedAtoms, alignmentMatrix, Invert);
    if( minRms == -1 || rms < minRms )  {
      minRms = rms;
      bestMatrix = alignmentMatrix;
    }
    return rms;
    //return TNetwork::FindAlignmentMatrix(matchedAtoms, alignmentMatrix, Invert);
  }
  double CalcRMS(const TEGraphNode<int,TSAtom*>& src, const TEGraphNode<int,TSAtom*>& dest)  {
    if( src[0].Count() != 0  )
      return CalcRMS();
    if( bestMatrix.r.Trace() == 0 )
      CalcRMS();
    //alignmentMatrix.t = aCent;
    CallsCount++;
    const TAsymmUnit& au = *dest[0].GetObject()->CAtom().GetParent();
    double rsum = 0;
    if( !Invert )  {
      for( int i=0; i < src.Count(); i++ )  {
        vec3d v = bestMatrix*(dest[i].GetObject()->crd() - bCent);
        rsum += v.QDistanceTo( src[i].GetObject()->crd() );
      }
    }
    else  {
      for( int i=0; i < src.Count(); i++ )  {
        vec3d v = dest[i].GetObject()->ccrd() * -1;
        v = bestMatrix*(au.CellToCartesian(v) - bCent );
        rsum += v.QDistanceTo( src[i].GetObject()->crd() );
      }
    }
    return rsum;
  }
};

//..............................................................................
bool TNetwork::DoMatch( TNetwork& net, TTypeList< AnAssociation2<int, int> >& res, bool Invert)  {
  if( NodeCount() != net.NodeCount() )  return false;
  TSAtom* thisSa = NULL;
  int maxbc = 0;
  double maxMw = 0;
  vec3d centb, centa;
  double centa_wght = 0, centb_wght = 0;
  const TAsymmUnit& au = this->GetLattice().GetAsymmUnit();
  for( int i=0; i < net.NodeCount(); i++ )  {
    if( Invert )  {
      vec3d v = net.Node(i).ccrd() * -1;
      au.CellToCartesian(v);
      centb += v*net.Node(i).CAtom().GetOccu()*net.Node(i).GetAtomInfo().GetMr();
      centb_wght += net.Node(i).CAtom().GetOccu()*net.Node(i).GetAtomInfo().GetMr();
    }
    else  {
      centb += net.Node(i).crd()*net.Node(i).CAtom().GetOccu()*net.Node(i).GetAtomInfo().GetMr();
      centb_wght += net.Node(i).CAtom().GetOccu()*net.Node(i).GetAtomInfo().GetMr();
    }
  }
  centb /= centb_wght;

  for( int i=0; i < NodeCount(); i++ )  {
    Node(i).SetTag(0);
    if( Node(i).NodeCount() > maxbc )  {
      thisSa = &Node(i);
      maxbc = Node(i).NodeCount();
      maxMw = Node(i).GetAtomInfo().GetMr();
    }
    else if( Node(i).NodeCount() == maxbc )  {
      if( Node(i).GetAtomInfo().GetMr() > maxMw )  {
        thisSa = &Node(i);
        maxMw = thisSa->GetAtomInfo().GetMr();
      }
    }
    centa += Node(i).crd()*Node(i).CAtom().GetOccu()*Node(i).GetAtomInfo().GetMr();
    centa_wght += Node(i).CAtom().GetOccu()*Node(i).GetAtomInfo().GetMr();
  }
  centa /= centa_wght;

  if( thisSa == NULL )  return false;
  TEGraph<int, TSAtom*> thisGraph( thisSa->GetAtomInfo().GetIndex(), thisSa );
//  TEGraph<int, TSAtom*> thisGraph( 0, thisSa );
  BuildGraph( thisGraph.GetRoot(), thisSa );
  TNetTraverser trav;
  trav.OnItem( thisGraph.GetRoot() );
  thisGraph.GetRoot().Traverser.LevelTraverse(thisGraph.GetRoot(), trav);
  TBasicApp::GetLog().Info( trav.GetData() );
  for( int i=0; i < net.NodeCount(); i++ )  {
    TSAtom* thatSa = &net.Node(i);
    if( thisSa->NodeCount() != thatSa->NodeCount() )  continue;
    if( thisSa->GetAtomInfo().GetIndex() != thatSa->GetAtomInfo().GetIndex() )  continue;
    for( int j=0; j < net.NodeCount(); j++ )
      net.Node(j).SetTag(0);
    TEGraph<int, TSAtom*> thatGraph( thatSa->GetAtomInfo().GetIndex(), thatSa );
    BuildGraph(thatGraph.GetRoot(), thatSa);

    trav.ClearData();
    trav.OnItem( thatGraph.GetRoot() );
    thatGraph.GetRoot().Traverser.LevelTraverse(thatGraph.GetRoot(), trav);
    TBasicApp::GetLog().Info( trav.GetData() );
    if( thisGraph.GetRoot().DoMatch( thatGraph.GetRoot()) )  {  // match 
      GraphAnalyser ga(thisGraph.GetRoot(), thatGraph.GetRoot(), centa, centb);
      ga.Invert = Invert;
      thisGraph.GetRoot().FullMatchEx( thatGraph.GetRoot(), ga);
    
      trav.ClearData();
      trav.OnItem( thatGraph.GetRoot() );
      thatGraph.GetRoot().Traverser.LevelTraverse(thatGraph.GetRoot(), trav);
      TBasicApp::GetLog().Info( trav.GetData() );
      TBasicApp::GetLog().Info( olxstr("Number of permutations: ") << ga.CallsCount );
      ResultCollector( thisGraph.GetRoot(), thatGraph.GetRoot(), res);
      return true;
    }
  }
  return false;
}
//..............................................................................
bool TNetwork::IsSubgraphOf( TNetwork& net, TTypeList< AnAssociation2<int, int> >& res,
                             const TIntList& rootsToSkip )  {

  if( NodeCount() > net.NodeCount() )  return false;
  TSAtom* thisSa = NULL;
  int maxbc = 0;
  for( int i=0; i < NodeCount(); i++ )  {
    Node(i).SetTag(0);
    if( Node(i).NodeCount() > maxbc )  {
      thisSa = &Node(i);
      maxbc = Node(i).NodeCount();
    }
  }
  TEGraph<int, TSAtom*> thisGraph( thisSa->GetAtomInfo().GetIndex(), thisSa );
  BuildGraph( thisGraph.GetRoot(), thisSa );
  TIntList GraphId;
  for( int i=0; i < net.NodeCount(); i++ )  {
    TSAtom* thatSa = &net.Node(i);
    if( thisSa->NodeCount() > thatSa->NodeCount() )  continue;
    if( thisSa->GetAtomInfo().GetIndex() != thatSa->GetAtomInfo().GetIndex() )  continue;
    if( rootsToSkip.IndexOf(i) != -1 )
      continue;
    for( int j=0; j < net.NodeCount(); j++ )
      net.Node(j).SetTag(0);
    TEGraph<int, TSAtom*> thatGraph( thatSa->GetAtomInfo().GetIndex(), thatSa );
    BuildGraph(thatGraph.GetRoot(), thatSa);
    //continue;
    if( thisGraph.GetRoot().IsSubgraphOf( thatGraph.GetRoot() ) )  {
      ResultCollector( thisGraph.GetRoot(), thatGraph.GetRoot(), res);
      return true;
    }
  }
  return false;
}
//..............................................................................
void RS_BreadthFirstTag(TSAtomPList& all, TSAtom* node)  {
  for( int i=0; i < node->NodeCount(); i++ )  {
    TSAtom& sa = node->Node(i);
    if( sa.GetTag() != 0 )  continue;
    all.Add( &sa )->SetTag( node->GetTag() + 1);
  }
}
void RS_BreadthFirstTags(TSAtom* sa, int stopAfter)  {
  TSAtomPList all;
  all.SetCapacity( sa->GetNetwork().NodeCount() );
  all.Add( sa );
  sa->SetTag(1);
  BreadthFirstTag(all, sa);
  for( int i=0; i < all.Count(); i++ )  {
    BreadthFirstTag( all, all[i] );
    //all[i]->CAtom()->Label() = all[i]->GetTag();
  }
}

bool TNetwork_TryRing( TSAtom* sa, TSAtomPList& ring, const TPtrList<TBasicAtomInfo>& ringContent, int level=1 )  {
  if( ringContent[level-1] != NULL && (sa->GetAtomInfo().GetIndex() != ringContent[level-1]->GetIndex()) )
    return false;
  sa->SetTag(level);
  for( int i=0; i < sa->NodeCount(); i++ )  {
    TSAtom& a = sa->Node(i);
    if( a.IsDeleted() )  continue;
    if( level >= ringContent.Count() && a.GetTag() == 1 )
      return true;
    if( a.GetTag() != 0 && a.GetTag() < level )  continue;
    if( level < ringContent.Count() && TNetwork_TryRing(&a, ring, ringContent, level+1) ) {
      ring.Add( &a );
      return true;
    }
  }
  sa->SetTag(0); // unroll the tags
  return false;
}
// tries to find the ring in given direction
bool TNetwork_TryRing( TSAtom& sa, int node, TSAtomPList& ring, const TPtrList<TBasicAtomInfo>& ringContent )  {
  sa.SetTag(1);
  if( TNetwork_TryRing(&sa.Node(node), ring, ringContent, 2) ) {
    return true;
  }
  return false;
}

int TNetwork_SortRingAtoms( const TSAtom* a, const TSAtom* b )  {
  return a->GetTag() - b->GetTag();
}

void TNetwork_UnifyRings(TTypeList<TSAtomPList>& rings)  {
  for( int i=0; i < rings.Count(); i++ )
    rings[i].QuickSorter.SortSF( rings[i], TNetwork_SortRingAtoms);
  // leave unique rings only
  for( int i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )  continue;
    for( int j=i+1; j < rings.Count(); j++ )  {
      if( rings.IsNull(j) )  continue;
      bool found = true;
      for( int k=0; k < rings[i].Count(); k++ )  {
        TSAtom* a = rings[i][k],
          *b = rings[j][k];
        if( rings[i][k]->GetTag() != rings[j][k]->GetTag() )  {
          found = false;
          break;
        }
      }
      if( found )  rings.NullItem(j);
    }
  }
}


void TNetwork::FindRings( const TPtrList<TBasicAtomInfo>& ringContent,
                          TTypeList<TSAtomPList>& res)  {
  TSAtomPList all;
  all.SetCapacity( NodeCount() );
  for( int i=0; i < NodeCount(); i++ )  {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if( sa.IsDeleted() ) continue;
    if( sa.NodeCount() > 1 )  // a ring node must have at least two bonds!
      all.Add( &sa );
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  int resCount = res.Count();
  for( int i=0; i < all.Count(); i++ )  {
    if( all[i]->GetAtomInfo() != *ringContent[0] )  continue;
    ring.Clear();
    for( int j=0; j < NodeCount(); j++ )
      Node(j).SetTag(0);
    ring.Add( all[i] );
    if( TNetwork_TryRing( all[i], ring, ringContent) )  {
      res.AddCCopy( ring );
      rings.AddCCopy( ring );
    }
  }
  for( int i=0; i < NodeCount(); i++ )
    Node(i).SetTag(i);
  TNetwork_UnifyRings( rings );
  for( int i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )
      res.NullItem( resCount+i );
  }
  res.Pack();
}
//..............................................................................
void TNetwork::FindAtomRings(TSAtom& ringAtom, const TPtrList<TBasicAtomInfo>& ringContent,
                             TTypeList<TSAtomPList>& res)  {
  if( ringAtom.NodeCount() < 2 || &ringAtom.GetNetwork() != this )  return;
  if( ringContent[0] != NULL && ringAtom.GetAtomInfo() != *ringContent[0] )  return;
  TSAtomPList all;
  all.SetCapacity( NodeCount() );
  for( int i=0; i < NodeCount(); i++ )  {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if( sa.IsDeleted() ) continue;
    if( sa.NodeCount() > 1 )  // a ring node must have at least two bonds!
      all.Add( &sa );
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  int resCount = res.Count();
  for( int i=0; i < ringAtom.NodeCount(); i++ )  {
    TSAtom& a = ringAtom.Node(i);
    if( a.IsDeleted() || a.NodeCount() < 2 || 
       (ringContent[1] != NULL && a.GetAtomInfo() != *ringContent[1]) )  continue;
    ring.Clear();
    for( int j=0; j < NodeCount(); j++ )
      Node(j).SetTag(0);
    if( TNetwork_TryRing( ringAtom, i, ring, ringContent) )  {
      ring.Add( &a );  // the ring is in reverse order
      ring.Add(&ringAtom);
      res.AddCCopy( ring );
      rings.AddCCopy( ring );
    }
  }
  for( int i=0; i < NodeCount(); i++ )
    Node(i).SetTag(i);
  TNetwork_UnifyRings(rings);
  for( int i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )
      res.NullItem( resCount+i );
  }
  res.Pack();
}
//..............................................................................
TNetwork::RingInfo& TNetwork::AnalyseRing( const TSAtomPList& ring, TNetwork::RingInfo& ri )  {
  int pivot = -1, pivot_count = 0;
  double maxmw = 0;
  for( int i=0; i < ring.Count(); i++ )  {
    if( !ri.HasAfix && ring[i]->CAtom().GetAfix() != 0 )
      ri.HasAfix = true;
    TSAtomPList& al = ri.Substituents.AddNew();
   
    int nhc = 0, // not hydrogen atom count
      rnc = 0;   // of which belong to the ring
    double local_maxmw = 0;
    for( int j=0; j < ring[i]->NodeCount(); j++ )  {
      TSAtom& ra = ring[i]->Node(j);
      if( ra.IsDeleted() || ra.GetAtomInfo() == iQPeakIndex )  continue;
      double mw = ra.GetAtomInfo().GetMr();
      if( mw < 3 )  continue; // H, D
      if( ra.crd().DistanceTo( ring[i]->crd() ) > 2 )  continue;  // skip M-E bonds
      if( mw > local_maxmw )  local_maxmw = mw;
      if( ring.IndexOf(&ra) != -1 )
        rnc++;
      else
        al.Add(&ra);
      nhc++;
    }
    if( nhc != rnc )  {
      if( local_maxmw > maxmw )  {
        ri.HeaviestSubsIndex = i;
        ri.HeaviestSubsType = &ring[i]->GetAtomInfo();
        maxmw = local_maxmw;
      }
      ri.Substituted.Add(i);
      if( nhc-rnc > ri.MaxSubsANode )
        ri.MaxSubsANode = nhc-rnc;
    }
    else if( rnc > 2 ) 
      ri.Ternary.Add(i);
  }
  // analyse alpha atoms (substituted next to ternary atoms)
  for( int i=0; i < ri.Substituted.Count(); i++ )  {
    for( int j=0;  j < ri.Ternary.Count(); j++ )  {
      if( ring[ri.Substituted[i]]->IsConnectedTo( *ring[ri.Ternary[j]] ) )  {
        ri.Alpha.Add( ri.Substituted[i] );
        break;
      }
    }
  }
  return ri;
}
//..............................................................................
bool TNetwork::IsRingRegular(const TSAtomPList& ring)  {
  vec3d cent;
  for( int i=0; i < ring.Count(); i++ )
    cent += ring[i]->crd();
  cent /= ring.Count();
  double avAng = 2*M_PI/ring.Count(), avDis = 0;
  for( int i=0; i < ring.Count(); i++ )
    avDis += cent.DistanceTo( ring[i]->crd() );
  avDis /= ring.Count();
  for( int i=0; i < ring.Count(); i++ )  {
    double d = cent.DistanceTo( ring[i]->crd() );
    if( olx_abs(d-avDis) > 0.2 )  return false;
  }
  vec3d a, b;
  for( int i=0; i < ring.Count(); i++ )  {
    a = ring[i]->crd();
    if( (i+1) == ring.Count() )
      b = ring[0]->crd();
    else
      b = ring[i+1]->crd();
    a -= cent;
    b -= cent;
    double ca = a.CAngle(b);
    if( ca < -1 )  ca = -1;
    if( ca > 1 ) ca = 1;
    ca = acos(ca);
    if( olx_abs(ca-avAng) > 5./M_PI )  return false;
  }
  return true;
}
//..............................................................................
// finds quaternions to map B to A
void TNetwork::FindAlignmentQuaternions(const TTypeList< AnAssociation2<vec3d,vec3d> >& crds, 
  const vec3d& centA, const vec3d& centB, ematd& quaternions, evecd& rms)  
{
  ematd evm(4,4);
  for( int i=0; i < crds.Count(); i++ )  {
    vec3d v = crds[i].GetA() - centA;
    const double 
      xm = v[0] - (crds[i].GetB()[0]-centB[0]),
      xp = v[0] + (crds[i].GetB()[0]-centB[0]),
      yp = v[1] + (crds[i].GetB()[1]-centB[1]),
      ym = v[1] - (crds[i].GetB()[1]-centB[1]),
      zm = v[2] - (crds[i].GetB()[2]-centB[2]),
      zp = v[2] + (crds[i].GetB()[2]-centB[2]);
    evm[0][0] += (xm*xm + ym*ym + zm*zm);
      evm[0][1] += (yp*zm - ym*zp);
      evm[0][2] += (xm*zp - xp*zm);
      evm[0][3] += (xp*ym - xm*yp);
    evm[1][0] = evm[0][1];
      evm[1][1] += (yp*yp + zp*zp + xm*xm);
      evm[1][2] += (xm*ym - xp*yp);
      evm[1][3] += (xm*zm - xp*zp);
    evm[2][0] = evm[0][2];
      evm[2][1] = evm[1][2];
      evm[2][2] += (xp*xp + zp*zp + ym*ym);
      evm[2][3] += (ym*zm - yp*zp);
    evm[3][0] = evm[0][3];
      evm[3][1] = evm[1][3];
      evm[3][2] = evm[2][3];
      evm[3][3] += (xp*xp + yp*yp + zm*zm);
  }
  ematd::EigenValues(evm, quaternions.Resize(4,4).I());
  rms.Resize(4);
  for( int i=0; i < 4; i++ )  {
    if( evm[i][i] < 0 )
      rms[i] = 0;
    else
      rms[i] = sqrt(evm[i][i]/crds.Count());
  }
  bool changes = true;
  while( changes )  {
    changes = false;
    for( int i=0; i < 3; i++ )  {
      if( rms[i+1] < rms[i] )  {
        quaternions.SwapRows(i, i+1);
        olx_swap(rms[i], rms[i+1]);
        changes = true;
      }
    }
  }
}
//..............................................................................
double TNetwork::FindAlignmentMatrix(const TTypeList<AnAssociation2<vec3d,vec3d> >& crds, 
  const vec3d& centA, const vec3d& centB, smatdd& res)  
{
  ematd quaternions(4,4);
  evecd rms(4);
  FindAlignmentQuaternions(crds, centA, centB, quaternions, rms);
  QuaternionToMatrix(quaternions[0], res.r);
  return rms[0];
}
/* gradient descent shows that the procedure does converge and needs no refinement */
double TNetwork::FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatdd& res, bool TryInversion)  {
  return TNetwork_FindAlignmentMatrix(atoms, res, TryInversion);
}
//..............................................................................
void TNetwork::DoAlignAtoms(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& satomp,
                            const TSAtomPList& atomsToTransform, const smatdd& S, bool Inverted)  {
  vec3d mcent;
  const TAsymmUnit& au = satomp[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  double sum  = 0;
  if( Inverted )  {
    for( int i=0; i < atomsToTransform.Count(); i++ )
      au.CellToCartesian(atomsToTransform[i]->ccrd() * -1, atomsToTransform[i]->crd());
    for(int i=0; i < satomp.Count(); i++ )  {
      vec3d v = satomp[i].GetB()->ccrd() * -1;
      au.CellToCartesian(v);
      mcent += v*satomp[i].GetB()->CAtom().GetOccu()*satomp[i].GetB()->GetAtomInfo().GetMr();
      sum += satomp[i].GetB()->CAtom().GetOccu()*satomp[i].GetB()->GetAtomInfo().GetMr();
    }
  }
  else  {
    for( int i=0; i < atomsToTransform.Count(); i++ )
      au.CellToCartesian(atomsToTransform[i]->ccrd(), atomsToTransform[i]->crd());
    for(int i=0; i < satomp.Count(); i++ )  {
      mcent += satomp[i].GetB()->crd()*satomp[i].GetB()->CAtom().GetOccu()*satomp[i].GetB()->GetAtomInfo().GetMr();
      sum += satomp[i].GetB()->CAtom().GetOccu()*satomp[i].GetB()->GetAtomInfo().GetMr();
    }
  }
  mcent /= sum;

  for( int i=0; i < atomsToTransform.Count(); i++ )
    atomsToTransform[i]->crd() = S*(atomsToTransform[i]->crd() - mcent);
}
//..............................................................................
bool TNetwork::RingInfo::IsSingleCSubstituted() const  {
  for( int i=0; i < Substituents.Count(); i++ )  {
    if( Substituents[i].Count() != 1 )  return false;
    TSAtom& sa = *Substituents[i][0];
    if( sa.GetAtomInfo() != iCarbonIndex )  return false;
    int nhc = 0;
    for( int j=0; j < sa.NodeCount(); j++ )  {
      TSAtom& ra = sa.Node(j);
      if( ra.GetAtomInfo().GetMr() < 3 || ra.GetAtomInfo() == iQPeakIndex )  continue;
      nhc++;
    }
    if( nhc > 1 )  return false;  // only one to ring bond 
  }
  return true;
}
//..............................................................................
void TNetwork::ToDataItem(TDataItem& item) const {
  item.AddField("net_id", Network == NULL ? -1 : Network->GetTag());
  TDataItem& nodes = item.AddItem("Nodes");
  for( int i=0; i < Nodes.Count(); i++ )  {
    if( Nodes[i]->IsDeleted() )  continue;
    nodes.AddField("node_id", Nodes[i]->GetTag());
  }
  TDataItem& bonds = item.AddItem("Bonds");
  for( int i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    bonds.AddField("bond_id", Bonds[i]->GetTag());
  }
}
//..............................................................................
void TNetwork::FromDataItem(const TDataItem& item) {
  const int net_id = item.GetRequiredField("net_id").ToInt();
  Network = net_id == -1 ? NULL : &Lattice->GetFragment(net_id);
  const TDataItem& nodes = item.FindRequiredItem("Nodes");
  Nodes.SetCapacity( nodes.FieldCount() );
  for( int i=0; i < nodes.FieldCount(); i++ )
    Nodes.Add(&Lattice->GetAtom(nodes.GetField(i).ToInt()));
  const TDataItem& bonds = item.FindRequiredItem("Bonds");
  Bonds.SetCapacity( bonds.FieldCount() );
  for( int i=0; i < bonds.FieldCount(); i++ )
    Bonds.Add(&Lattice->GetBond(bonds.GetField(i).ToInt()));
}
//..............................................................................

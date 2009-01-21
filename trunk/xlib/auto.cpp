#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "auto.h"
#include "bapp.h"
#include "log.h"
#include "actions.h"
#include "symmtest.h"
#include "symmlib.h"
#include "cif.h"
#include "etime.h"

#include "olxmps.h"
#include "egc.h"
#include "eutf8.h"

#undef GetObject

const int MaxConnectivity = 12;

// file header: Signature,Version,Flags
const int16_t FileVersion = 0x0001;
// file flags
const int16_t ffZipped  = 0x0001;

const char FileSignature [] = "OADB";  // olex auto .. db
const long FileSignatureLength = 4;  // must not change ever!

// variations
const double LengthVar = 0.03,
             AngleVar  = 5.4;

//..............................................................................
void TAutoDBFolder::SaveToStream( IDataOutputStream& output ) const  {
  output << (long)Files.Count();
  for( int i=0; i < Files.Count(); i++ )
    output << TUtf8::Encode(Files.GetComparable(i));
}
//..............................................................................
void TAutoDBFolder::LoadFromStream( IDataInputStream& input )  {
  int32_t fc;
  input >> fc;
  Files.SetCapacity( fc);
  CString tmp;
  for( long i=0; i < fc; i++ )  {
    input >> tmp;
    Files.Add( TUtf8::Decode(tmp), new TAutoDBIdObject());
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
TAttachedNode::TAttachedNode( IDataInputStream& in )  {
  int32_t ind;
  in >> ind;
  BasicAtomInfo = &TAutoDB::GetAtomsInfo().GetAtomInfo(ind);
  float val;
  in >> val;  FCenter[0] = val;
  in >> val;  FCenter[1] = val;
  in >> val;  FCenter[2] = val;
}
//..............................................................................
void TAttachedNode::SaveToStream( IDataOutputStream& output ) const  {
  output << (int32_t)BasicAtomInfo->GetIndex();
  output << (float)FCenter[0];
  output << (float)FCenter[1];
  output << (float)FCenter[2];
}
//..............................................................................
//..............................................................................
//..............................................................................
vec3d TAutoDBNode::SortCenter;

static const vec3d ZAxis(0,0,1);

int TAutoDBNode::SortMetricsFunc(const TAttachedNode& a, const TAttachedNode& b )  {
  double diff = TAutoDBNode::SortCenter.DistanceTo(b.GetCenter()) -
                TAutoDBNode::SortCenter.DistanceTo(a.GetCenter());
/*  if( olx_abs(diff) < 0.001 )  {
    vec3d ap(a.crd()), bp(b.crd());
    ap -= TAutoDBNode::SortCenter;
    bp -= TAutoDBNode::SortCenter;
    double ca = ZAxis.CAngle( ap );
    if( ca < -1 )  ca = -1;
    if( ca > 1 )   ca = 1;
    ca = acos(ca)*180/M_PI;
    double cb = ZAxis.CAngle( bp );
    if( cb < -1 )  cb = -1;
    if( cb > 1 )   cb = 1;
    cb = acos(cb)*180/M_PI;
    diff = cb - ca;
  }
*/
  if( diff < 0 )  return -1;
  if( diff > 0 )  return 1;
  return 0;
}
int TAutoDBNode::SortCAtomsFunc(const AnAssociation2<TCAtom*, vec3d>& a,
                                const AnAssociation2<TCAtom*, vec3d>& b )  {
  double diff = TAutoDBNode::SortCenter.DistanceTo(b.GetB()) -
                TAutoDBNode::SortCenter.DistanceTo(a.GetB());
  if( diff < 0 )  return -1;
  if( diff > 0 )  return 1;
  return 0;
}

TAutoDBNode::TAutoDBNode(TSAtom& sa, TTypeList<AnAssociation2<TCAtom*, vec3d> >* atoms)  {
  //AppendedCount = 1;
  BasicAtomInfo = &sa.GetAtomInfo();
  Center = sa.crd();
  for( int i=0; i < sa.NodeCount(); i++ )  {
    if( sa.Node(i).IsDeleted() )  continue;
    if( sa.Node(i).GetAtomInfo() != iHydrogenIndex && sa.Node(i).GetAtomInfo() != iDeuteriumIndex )  {
      AttachedNodes.Add( *(new TAttachedNode(&sa.Node(i).GetAtomInfo(), sa.Node(i).crd())) );
      if( atoms != NULL )  {
        atoms->AddNew<TCAtom*, vec3d>(&sa.Node(i).CAtom(), sa.Node(i).crd());
      }
    }
  }
  vec3d a, b;
  vec3d_list TransformedCrds;
  TLattice& latt = sa.GetNetwork().GetLattice();
  for( int i=0; i < sa.CAtom().AttachedAtomCount(); i++ )  {
    if( sa.CAtom().GetAttachedAtom(i).GetAtomInfo() == iHydrogenIndex ||
        sa.CAtom().GetAttachedAtom(i).GetAtomInfo() == iDeuteriumIndex )  continue;
    a = sa.CAtom().GetAttachedAtom(i).ccrd();
    smatd_list* transforms = latt.GetUnitCell().GetInRange(sa.ccrd(), a,
                               sa.GetAtomInfo().GetRad1() +
                               sa.CAtom().GetAttachedAtom(i).GetAtomInfo().GetRad1() +
                               latt.GetDelta(),
                                false );
    if( transforms->IsEmpty() )  {
      delete transforms;
      continue;
    }
    TransformedCrds.Clear();
    for( int j=0; j < transforms->Count(); j++ )  {
      smatd& transform = transforms->Item(j);
      a = transform * a;
      latt.GetAsymmUnit().CellToCartesian(a);
      if( a.QDistanceTo( sa.crd() ) > 0.01 )  {
        bool found = false;
        for( int k=0; k < sa.NodeCount(); k++ )  {
          if( a.QDistanceTo( sa.Node(k).crd() ) < 0.01 )  {
            found = true;
            break;
          }
        }
        if( !found )
          TransformedCrds.AddCCopy( a );
      }
      a = sa.CAtom().GetAttachedAtom(i).ccrd();
    }
    for( int j=0; j < TransformedCrds.Count(); j++ )  {
      if( TransformedCrds.IsNull(j) )  continue;
      for( int k = j+1; k < TransformedCrds.Count(); k++ )  {
        if( TransformedCrds.IsNull(k) )  continue;
        if( TransformedCrds[j].QDistanceTo( TransformedCrds[k] ) < 0.01 )  {
          TransformedCrds.NullItem(k);
          break;
        }
      }
    }
    TransformedCrds.Pack();
    for( int j=0; j < TransformedCrds.Count(); j++ )  {
      AttachedNodes.Add( *(new TAttachedNode(&sa.CAtom().GetAttachedAtom(i).GetAtomInfo(), TransformedCrds[j])) );
      if( atoms != NULL )  {
        atoms->AddNew<TCAtom*, vec3d>(&sa.CAtom().GetAttachedAtom(i), TransformedCrds[j]);
      }
    }
    delete transforms;
  }
  TAutoDBNode::SortCenter = sa.crd();
  AttachedNodes.QuickSorter.SortSF(AttachedNodes, SortMetricsFunc);
  if( atoms != NULL )
    atoms->QuickSorter.SortSF(*atoms, SortCAtomsFunc);
  _PreCalc();
}
//..............................................................................
void TAutoDBNode::_PreCalc()  {
  Params.Resize( (AttachedNodes.Count()+1)*AttachedNodes.Count()/2);
  int index = AttachedNodes.Count();
  for( int i=0; i < AttachedNodes.Count(); i++ )  {
    Params[i] = CalcDistance(i);
    for( int j=i+1; j < AttachedNodes.Count(); j++ )  {
      Params[index] = CalcAngle(i,j);
      index ++;
    }
  }
}
//..............................................................................
double TAutoDBNode::CalcAngle(int i, int j)  const {
  vec3d a(AttachedNodes[i].GetCenter()),
           b(AttachedNodes[j].GetCenter());
  a -= Center;
  b -= Center;
  if( a.QLength()*b.QLength() == 0 )  {
    TBasicApp::GetLog().Error( olxstr("Overlapping atoms enountered") );
    return 0;
  }
  double ca = a.CAngle(b);
  if( ca < -1 )  ca = -1;
  if( ca > 1 )  ca = 1;
  return acos(ca)*180.0/M_PI;
}
//..............................................................................
void TAutoDBNode::SaveToStream( IDataOutputStream& output ) const  {
  output << (int32_t)BasicAtomInfo->GetIndex();
  output << (float)Center[0];
  output << (float)Center[1];
  output << (float)Center[2];
  output << (int)AttachedNodes.Count();
  for( int i=0; i < AttachedNodes.Count(); i++ )
    AttachedNodes[i].SaveToStream(output);
}
//..............................................................................
void TAutoDBNode::LoadFromStream( IDataInputStream& in )  {
  int32_t ind;
  in >> ind;
  BasicAtomInfo = &TAutoDB::GetAtomsInfo().GetAtomInfo(ind);
  float val;
  in >> val;  Center[0] = val;
  in >> val;  Center[1] = val;
  in >> val;  Center[2] = val;
  in >> ind;
  for( int i=0; i < ind; i++ )
    AttachedNodes.Add( *(new TAttachedNode(in)) );
  TAutoDBNode::SortCenter = Center;
  AttachedNodes.QuickSorter.SortSF(AttachedNodes, SortMetricsFunc);
  _PreCalc();
}
//..............................................................................
const olxstr& TAutoDBNode::ToString() const  {
  olxstr& tmp = TEGC::New<olxstr>(EmptyString, 100);
  tmp << BasicAtomInfo->GetSymbol() << '{';
  for( int i=0; i < AttachedNodes.Count(); i++ )  {
    tmp << AttachedNodes[i].GetAtomInfo().GetSymbol();
    if( (i+1) < AttachedNodes.Count() )
      tmp << ',';
  }
  tmp << '}'; /* << '[';
  for(int i=0; i < Params.Count(); i++ )  {
    tmp << olxstr::FormatFloat(3, GetDistance(i));
    if( (i+1) < Params.Count() )  tmp << ',';
  }
  tmp << ']';   */
  return tmp;
}
//..............................................................................
double TAutoDBNode::SearchCompare(const TAutoDBNode& dbn, double* fom) const  {
  double _fom = 0;
  int mc = (AttachedNodes.Count() > 4 ) ? AttachedNodes.Count() : Params.Count();
  for(int i=0; i < mc; i++ ) {
    double diff = Params[i] - dbn.Params[i];
    if( i < AttachedNodes.Count() )  {
      if( olx_abs(diff) > LengthVar )
        return diff;
    }
    else  {  // 5.7 degrees give about 0.1 deviation in distance for 1A bonds (b^2+a^2-2abcos)^1/2
      if( olx_abs(diff) > AngleVar )
        return diff/180;
      diff /= 180;
    }
    _fom += diff*diff;
  }
  if( fom != NULL )
    *fom += _fom/Params.Count();
  return 0;
/*
  for( int i=0; i < Params.Count(); i++ )  {
    double diff = Params[i] - dbn.Params[i];
    if( diff < 0 )  return -1;
    if( diff > 0 )  return 1;
  }
  return 0;
*/
}
//..............................................................................
int TAutoDBNode::UpdateCompare(const TAutoDBNode& dbn) const  {
  double diff = BasicAtomInfo->GetIndex() - dbn.BasicAtomInfo->GetIndex();
  if( diff == 0 )  {
    diff = AttachedNodes.Count() - dbn.AttachedNodes.Count();
    if( diff == 0 )  {
      for( int i=0; i < AttachedNodes.Count(); i++ )  {
        diff = AttachedNodes[i].GetAtomInfo().GetIndex() -
               dbn.AttachedNodes[i].GetAtomInfo().GetIndex();
        if( diff != 0 )  return (int)diff;
      }
      for( int i=0; i < Params.Count(); i++ )  {
        diff = Params[i] - dbn.Params[i];
        if( diff < 0 )  return -1;
        if( diff > 0 )  return 1;
      }
      return 0;
    }
    else
      return (int)diff;
  }
  else
    return (int)diff;
}
//..............................................................................
bool TAutoDBNode::IsSameType(const TAutoDBNode& dbn) const  {
  double diff = BasicAtomInfo->GetIndex() - dbn.BasicAtomInfo->GetIndex();
  if( diff != 0 )  return false;
  diff = AttachedNodes.Count() - dbn.AttachedNodes.Count();
  if( diff != 0 )  return false;
  for( int i=0; i < AttachedNodes.Count(); i++ )  {
    diff = AttachedNodes[i].GetAtomInfo().GetIndex() - dbn.AttachedNodes[i].GetAtomInfo().GetIndex();
    if( diff != 0 )  return false;
  }
  return true;
}
//..............................................................................
bool TAutoDBNode::IsSimilar(const TAutoDBNode& dbn) const  {
  double diff = BasicAtomInfo->GetIndex() - dbn.BasicAtomInfo->GetIndex();
  if( diff == 0 )  {
    diff = AttachedNodes.Count() - dbn.AttachedNodes.Count();
    if( diff == 0 )  {
      // check types
      for( int i=0; i < AttachedNodes.Count(); i++ )  {
        diff = AttachedNodes[i].GetAtomInfo().GetIndex() -
               dbn.AttachedNodes[i].GetAtomInfo().GetIndex();
        if( diff != 0 )  return false;
      }
      // check distance and angles
      for( int i=0; i < Params.Count(); i++ )  {
        diff = olx_abs(Params[i] - dbn.Params[i]);
        if( i < AttachedNodes.Count() )  {
          if( diff > 0.005) return false;
        }
        else
          if( diff > 4 ) return false;
      }
      return true;
    }
    else
      return false;
  }
  else
    return false;
}
//..............................................................................
bool TAutoDBNode::IsMetricSimilar(const TAutoDBNode& dbn, double& fom) const  {
  if( AttachedNodes.Count() != dbn.AttachedNodes.Count() )  return false;
  // check distance and angles
  double _fom = 0;
  int mc = (AttachedNodes.Count() > 4 ) ? AttachedNodes.Count() : Params.Count();
  for(int i=0; i < mc; i++ ) {
//  for(int i=0; i < Params.Count(); i++ ) {
    double diff = olx_abs(Params[i] - dbn.Params[i]);
    if( i < AttachedNodes.Count() )  {
      if( diff > LengthVar )  return false;
    }
    else  {  // 5.7 degrees give about 0.1 deviation in distance for 1A bonds (b^2+a^2-2abcos)^1/2
      if( diff > AngleVar ) return false;
      diff = diff/180.0;
    }
    _fom += diff*diff;
  }
  fom += _fom/Params.Count();
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TAutoDBNetNode::IsMetricSimilar(const TAutoDBNetNode& nd, double& cfom, int* cindexes, bool ExtraLevel)  const  {
  if( nd.AttachedNodes.Count() != AttachedNodes.Count() )
    return false;
  // have to do a full comparison, as the node order is unknown ...
  const int con = olx_min(MaxConnectivity, AttachedNodes.Count());
  TAutoDBNetNode* nodes[MaxConnectivity];
  for(int i=0; i < con; i++ )
    nodes[i] = AttachedNodes[i];
  int indexes[MaxConnectivity];
  for( int i=0; i < nd.AttachedNodes.Count(); i++ )  {
    for(int j=0; j < con; j++ )  {
      if( nodes[j] == NULL ) continue;
      if( nodes[j]->FCenter->IsMetricSimilar(*nd.AttachedNodes[i]->FCenter, cfom) )  {
        nodes[j] = NULL;
        indexes[i] = j;
        if( cindexes != NULL )  cindexes[i] = j;
        break;
      }
    }
  }
  int ndcnt = 0;
  for(int i=0; i < con; i++ )
    if( nodes[i] != NULL )  ndcnt++;

  if( !ExtraLevel )  {
    if( ndcnt != 0 )  return false;
  }
  else  {
    if( ndcnt != 0 )  return false;
    else  {
      for( int i=0; i < con; i++ )
        if( !nd.AttachedNodes[i]->IsMetricSimilar( *AttachedNodes[ indexes[i] ], cfom, NULL, false) )
          return false;
    }
  }
  return true;
}
//..............................................................................
bool TAutoDBNetNode::IsSameType(const TAutoDBNetNode& dbn, bool ExtraLevel) const  {
  if( !FCenter->IsSameType( *dbn.FCenter ) )  return false;
  TPtrList<TAutoDBNetNode> nodes;
  nodes.AddList(AttachedNodes);
  TIntList indexes;
  indexes.SetCapacity( nodes.Count() );
  for( int i=0; i < dbn.AttachedNodes.Count(); i++ )  {
    for(int j=0; j < nodes.Count(); j++ )  {
      if( nodes[j] == NULL ) continue;
      if( nodes[j]->FCenter->IsSameType(*dbn.AttachedNodes[i]->FCenter) )  {
        nodes[j] = NULL;
        indexes.Add(j);
        break;
      }
    }
  }
  nodes.Pack();
  if( !ExtraLevel )
    return (nodes.IsEmpty()) ? true : false;
  else  {
    if( nodes.Count() != 0 )  return false;
    for( int i=0; i < dbn.AttachedNodes.Count(); i++ )
      if( !dbn.AttachedNodes[i]->IsSameType( *AttachedNodes[ indexes[i] ], false) ) return false;
    return true;
  }
}
//..............................................................................
void TAutoDBNetNode::SaveToStream( IDataOutputStream& output ) const  {
  output << FCenter->GetId();
  output << (int8_t)AttachedNodes.Count();
  for( int i=0; i < AttachedNodes.Count(); i++ )
    output << AttachedNodes[i]->GetId();
}
void TAutoDBNetNode::LoadFromStream( IDataInputStream& input )  {
#ifdef __GNUC__  // dunno how it is implemented, lol, but need 8 bits after all
  char cnt; 
#else  
  int8_t cnt; 
#endif
  int32_t ind;
  input >> ind;
  FCenter = TAutoDB::GetInstance()->Node(ind);
  input >> cnt;
  for(int i=0; i < cnt; i++ )  {
    input >> ind;
    AttachedNodes.Add( &TAutoDBNet::GetCurrentlyLoading().Node(ind) );
  }
}
//..............................................................................
const olxstr& TAutoDBNetNode::ToString(int level) const  {
  olxstr& tmp = TEGC::New<olxstr>(EmptyString, 256);
  tmp << FCenter->ToString();
  if( level == 1 )  {
    tmp << '{';
    for( int i=0; i < AttachedNodes.Count(); i++ )  {
      tmp << AttachedNodes[i]->FCenter->ToString();
      if( (i+1) < AttachedNodes.Count() )
        tmp << ',';
    }
    tmp << '}';
  }
  else if( level == 2 )  {
    tmp << '[';
    for( int i=0; i < AttachedNodes.Count(); i++ )  {
      tmp << AttachedNodes[i]->ToString(1);
      if( (i+1) < AttachedNodes.Count() )
        tmp << ',';
    }
    tmp << ']';
  }
  return tmp;
}
//..............................................................................
//..............................................................................
//..............................................................................
TAutoDBNet* TAutoDBNet::CurrentlyLoading = NULL;
//..............................................................................
void TAutoDBNet::SaveToStream( IDataOutputStream& output ) const  {
  output << (int32_t)FReference->GetId();
  output << (int16_t)Nodes.Count();
  for( int i=0; i < Nodes.Count(); i++ )
    Nodes[i].SetId(i);
  for( int i=0; i < Nodes.Count(); i++ )
    Nodes[i].SaveToStream( output );
}
void TAutoDBNet::LoadFromStream( IDataInputStream& input )  {
  TAutoDBNet::CurrentlyLoading = this;
  int32_t ind;
  int16_t cnt;
  input >> ind;
  FReference = &TAutoDB::GetInstance()->Reference(ind);
  input >> cnt;
  for( int i=0; i < cnt; i++ )
    Nodes.Add( *( new TAutoDBNetNode(NULL)) );
  for( int i=0; i < cnt; i++ )
    Nodes[i].LoadFromStream(input);
  // build index
  for( int i=0; i < cnt; i++ )
    Nodes[i].Center()->AddParent(this,i);
  TAutoDBNet::CurrentlyLoading = NULL;
}
//..............................................................................
//..............................................................................
//..............................................................................
int UpdateNodeSortFunc( const TAutoDBNode* a, const TAutoDBNode* b )  {
  return a->UpdateCompare(*b);
}
//..............................................................................
int SearchCompareFunc( const TAutoDBNode* a, const TAutoDBNode* b )  {
  double v = a->SearchCompare(*b);
  if( v < 0 )  return -1;
  if( v > 0 )  return 1;
  return 0;
}
//..............................................................................
//..............................................................................
TAutoDB* TAutoDB::Instance = NULL;


TAutoDB::TAutoDB(TXFile& xfile, ALibraryContainer& lc) : XFile(xfile), AtomsInfo(TAtomsInfo::GetInstance())  {
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "dublicated object instance");
  Instance = this;
  for( int i=0; i < MaxConnectivity-1; i++ )
    Nodes.AddNew();
  BAIDelta = -1;
  URatio = 1.3;
  lc.GetLibrary().AttachLibrary( ExportLibrary() );
}
//..............................................................................
TAutoDB::~TAutoDB()  {
  for( int i=0; i < Nodes.Count(); i++ )
    for( int j=0; j < Nodes[i].Count(); j++ )
      delete Nodes[i][j];
  for( int i=0; i < Folders.Count(); i++ )
    delete Folders.Object(i);
  Instance = NULL;
  delete &XFile;
}
//..............................................................................
void TAutoDB::PrepareForSearch()  {
  for( int i=0; i < Nodes.Count(); i++ )
    Nodes[i].QuickSorter.SortSF(Nodes[i], SearchCompareFunc);
}
//..............................................................................
void TAutoDB::ProcessFolder(const olxstr& folder)  {
  if( !TEFile::FileExists(folder) )  return;
  olxstr currentDir = TEFile::CurrentDir();
  olxstr uf = TEFile::RemoveTrailingBackslash(folder);
  TEFile::ChangeDir( uf );
  TFileList files;
  TEFile::ListCurrentDirEx(files, "*.cif", sefFile|sefDir);
  if( files.IsEmpty() )  return;

  TAutoDBFolder* dbfolder = NULL;
  int folderIndex = Folders.IndexOfComparable(uf);
  if( folderIndex != -1 ) dbfolder = Folders.Object(folderIndex);
  if( dbfolder == NULL )  {
    dbfolder = new TAutoDBFolder;
    Folders.Add( uf, dbfolder );
  }
  TOnProgress progress;
  progress.SetMax( files.Count() );
  for( int i=0; i < Nodes.Count(); i++ )  {
    Nodes[i].SetCapacity( Nodes[i].Count() + files.Count()*100);
    Nodes[i].SetIncrement(64*1024);
  }
  for( int i=0; i < files.Count(); i++ )  {
    if( (files[i].GetAttributes() & sefFile) != 0 )  {
      progress.SetPos(i);
      if( dbfolder->Contains( files[i].GetName() ) )  continue;
      TAutoDBIdObject& adf = dbfolder->Add( files[i].GetName() );
      try  {
      TBasicApp::GetLog().Info( olxstr("Processing ") << files[i].GetName() << "..." );
        XFile.LoadFromFile( files[i].GetName() );
        TCif& cif = XFile.GetLastLoader<TCif>();
        olxstr r1 = cif.GetSParam("_refine_ls_R_factor_gt");
        if( r1.Length() && r1.ToDouble() > 5 )  {
          TBasicApp::GetLog().Info( olxstr("Skipped r1=") << r1 );
          continue;
        }
        olxstr shift = cif.GetSParam("_refine_ls_shift/su_max");
        if( shift.Length() && shift.ToDouble() > 0.05 )  {
          TBasicApp::GetLog().Info( olxstr("Skipped shift=") << shift );
          continue;
        }
        olxstr gof = cif.GetSParam("_refine_ls_goodness_of_fit_ref");
        if( gof.Length() && olx_abs(1-gof.ToDouble()) > 0.1 )  {
          TBasicApp::GetLog().Info( olxstr("Skipped GOF=") << gof );
          continue;
        }

        if( XFile.GetAsymmUnit().DoesContainEquivalents() )  {
          XFile.GetLattice().Uniq(true);
          XFile.GetLattice().CompaqAll();
          XFile.GetLattice().Compaq();
          XFile.SaveToFile(files[i].GetName(), false);
          XFile.LoadFromFile( files[i].GetName() );
        }
        XFile.GetLattice().Compaq();
        for( int j=0; j < XFile.GetLattice().FragmentCount(); j++ )
          ProcessNodes( &adf, XFile.GetLattice().GetFragment(j) );
      }
      catch( const TExceptionBase& exc )  {
        TBasicApp::GetLog().Error( olxstr("Failed to process: ") << exc.GetException()->GetError()  );
      }
    }
  }
  PrepareForSearch();
}
//..............................................................................
struct TTmpNetData  {
  TSAtom* Atom;
  TAutoDBNode* Node;
  TTypeList<AnAssociation2<TCAtom*, vec3d> >* neighbours;
};
void TAutoDB::ProcessNodes( TAutoDBIdObject* currentFile, TNetwork& net )  {
  if( net.NodeCount() == 0 )  return;
  TTypeList< TTmpNetData* > netMatch;
  TTmpNetData* netItem;
  for( int i=0; i < net.NodeCount(); i++ )
    net.Node(i).SetTag(1);
  for( int i=0; i < net.NodeCount(); i++ )  {
    if( net.Node(i).GetAtomInfo() != iQPeakIndex &&
       net.Node(i).GetAtomInfo() != iHydrogenIndex &&
       net.Node(i).GetAtomInfo() != iDeuteriumIndex ) {

      netItem = new TTmpNetData;
      netItem->neighbours = new TTypeList<AnAssociation2<TCAtom*, vec3d> >;
      netItem->Atom = &net.Node(i);
      TAutoDBNode* dbn = new TAutoDBNode( net.Node(i), netItem->neighbours );
      // instead of MaxConnectivity we use Nodes.Count() to comply with db format
      if( dbn->NodeCount() < 1 || dbn->NodeCount() > Nodes.Count() )  {
        delete dbn;
        delete netItem->neighbours;
        delete netItem;
      }
      else  {
        TPtrList<TAutoDBNode>& segment = Nodes[ dbn->NodeCount()-1 ];
        for( int j=0; j < segment.Count(); j++ )  {
          if( segment[j]->IsSimilar(*dbn) )  {
            netItem->Node = segment[j];
            delete dbn;
            dbn = NULL;
            break;
          }
        }
        if( dbn != NULL )  {
          netItem->Node = dbn;
          Nodes[dbn->NodeCount()-1].Add( dbn );
        }
        netMatch.AddACopy( netItem );
      }
    }
  }
  // construct the network
  if( netMatch.Count() > 1 )  {
    /*this gives a one-to-one match between CAtoms and net nodes with CAtom->GetId()
      However there is a problems since the fragments represent current content of
      the asymmetric unit and therefore some atoms might be attached to CAtoms of
      other fragments. We avoid this situation by considering only atoms of this
      fragment
    */
    for( int i=0; i < net.GetLattice().GetAsymmUnit().AtomCount(); i++ )
      net.GetLattice().GetAsymmUnit().GetAtom(i).SetId(-1);
    for( int i=0; i < netMatch.Count(); i++ )
      netMatch[i]->Atom->CAtom().SetId(i);
    TAutoDBNet& net = Nets.AddNew( currentFile );
    // precreate nodes
    for( int i=0; i < netMatch.Count(); i++ )  {
      net.NewNode(netMatch[i]->Node);
      netMatch[i]->Node->AddParent(&net, i); // build index
    }
    // build connectivity
    for( int i=0; i < netMatch.Count(); i++ )  {
      for( int j=0; j < netMatch[i]->neighbours->Count(); j++ )  {
        if( netMatch[i]->neighbours->Item(j).A()->GetId() < 0 )  continue;
        net.Node(i).AttachNode( &net.Node(netMatch[i]->neighbours->Item(j).A()->GetId()) );
      }
    }
    for( int i=0; i < netMatch.Count(); i++ ) {
      delete netMatch[i]->neighbours;
      delete netMatch[i];
    }
  }
//  for( int i=0; i < Nodes.Count(); i++ )
//    Nodes[i]->Average();

  return;
}
//..............................................................................
TAutoDBNet* TAutoDB::BuildSearchNet( TNetwork& net, TSAtomPList& cas )  {
  if( net.NodeCount() == 0 )  return NULL;
  TPtrList<TTmpNetData> netMatch;
  TTmpNetData* netItem;
  for( int i=0; i < net.NodeCount(); i++ )
    net.Node(i).SetTag(1);
  for( int i=0; i < net.NodeCount(); i++ )  {
//    if( net.Node(i).GetAtomInfo() != iQPeakIndex &&
    if( net.Node(i).GetAtomInfo() != iHydrogenIndex &&
       net.Node(i).GetAtomInfo() != iDeuteriumIndex ) {

      netItem = new TTmpNetData;
      netItem->neighbours = new TTypeList<AnAssociation2<TCAtom*, vec3d> >;
      netItem->Atom = &net.Node(i);
      TAutoDBNode* dbn = new TAutoDBNode( net.Node(i), netItem->neighbours );
      if( dbn->NodeCount() < 1 || dbn->NodeCount() > 12 )  {
        delete dbn;
        delete netItem->neighbours;
        delete netItem;
      }
      else  {
        netItem->Node = dbn;
        netMatch.Add( netItem );
      }
    }
  }
  // construct the network
  if( netMatch.Count() > 0 )  {
    for( int i=0; i < net.GetLattice().GetAsymmUnit().AtomCount(); i++ )
      net.GetLattice().GetAsymmUnit().GetAtom(i).SetId(-1);
    TAutoDBNet* dbnet = new TAutoDBNet(NULL);
    for( int i=0; i < netMatch.Count(); i++ )  {
      dbnet->NewNode(netMatch[i]->Node);
      cas.Add( netMatch[i]->Atom );
      netMatch[i]->Atom->CAtom().SetId(i);
    }
    for( int i=0; i < netMatch.Count(); i++ )  {
      for( int j=0; j < netMatch[i]->neighbours->Count(); j++ )  {
        if( netMatch[i]->neighbours->Item(j).A()->GetId() < 0 )  continue;
        dbnet->Node(i).AttachNode( &dbnet->Node(netMatch[i]->neighbours->Item(j).A()->GetId()) );
      }
    }
    for( int i=0; i < netMatch.Count(); i++ ) {
      delete netMatch[i]->neighbours;
      delete netMatch[i];
    }
    // must restore data!
    net.GetLattice().GetAsymmUnit().InitAtomIds();
    return dbnet;
  }
  return NULL;
}
//..............................................................................
void TAutoDB::SaveToStream( IDataOutputStream& output )  const {
  output.Write( FileSignature, FileSignatureLength );
  output << FileVersion;
  output << (int16_t)0;  // file flags - flat for now

  int32_t folderCount = 0;
  output << (int32_t)Folders.Count();
  for( int i=0; i < Folders.Count(); i++ )  {
    output << TUtf8::Encode(Folders.GetComparable(i));
    Folders.GetObject(i)->AssignIds(folderCount);
    Folders.GetObject(i)->SaveToStream( output);
    folderCount += Folders.GetObject(i)->Count();
  }

  int32_t nodeCount = 0;
  output << (int32_t)Nodes.Count();
  for( int i=0; i < Nodes.Count(); i++ )  {
    output << (int)Nodes[i].Count();
    for( int j=0; j < Nodes[i].Count(); j++ )  {
      Nodes[i][j]->SetId(nodeCount+j);
      Nodes[i][j]->SaveToStream( output );
    }
    nodeCount += Nodes[i].Count();
  }

  output << (int32_t)Nets.Count();
  for( int i=0; i < Nets.Count(); i++ )
    Nets[i].SaveToStream( output );
}
//..............................................................................
void TAutoDB::LoadFromStream( IDataInputStream& input )  {
  // validation of the file
  char fileSignature[FileSignatureLength+1];
  input.Read( fileSignature, FileSignatureLength );
  fileSignature[FileSignatureLength] = '\0';
  if( olxstr(fileSignature) != FileSignature )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");
  int16_t fileVersion;
  input >> fileVersion;
  if( fileVersion != FileVersion )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file version");
  // read file flags
  input >> fileVersion;
  // end of the file validation
  CString tmp;
  int32_t ind;
  int32_t fileCount = 0, nodeCount = 0;
  input >> ind;
  Folders.SetCapacity( ind );
  for( int i=0; i < ind; i++ )  {
    input >> tmp;
    Folders.Add( TUtf8::Decode(tmp),  new TAutoDBFolder(input) );
    Folders.GetObject(i)->AssignIds(fileCount);
    fileCount += Folders.GetObject(i)->Count();
  }

  int32_t listCount;
  input >> listCount;  // nt MaxConnectivity is overriden!
  Nodes.Clear();
  Nodes.SetCapacity( listCount );
  for( int i=0; i < listCount; i++ )  {
    Nodes.AddNew();
    input >> ind;
    Nodes[i].SetCapacity( ind );
    for( int j=0; j < ind; j++ )  {
      Nodes[i].Add( new TAutoDBNode(input) );
      Nodes[i][j]->SetId(nodeCount + j);
    }
    nodeCount += ind;
  }

  input >> ind;
  for( int i=0; i < ind; i++ )
    Nets.Add( *(new TAutoDBNet(input)) );

  PrepareForSearch();
}
//..............................................................................
void TAutoDB::AnalyseNode(TSAtom& sa, TStrList& report)  {
  TSAtomPList cas;
  TAutoDBNet* sn = BuildSearchNet( sa.GetNetwork(), cas );
  if( sn == NULL )  return;
  olxstr tmp;
  int index = -1;
  for( int i=0; i < cas.Count(); i++ )  {
    if( cas[i] == &sa )  {
      index = i;
      break;
    }
  }
  if( index == -1 )  return;
  TAutoDBNetNode& node = sn->Node(index);
  if( node.Count() < 1 || node.Count() > Nodes.Count() ) return;
  TPtrList< TAutoDBNode >& segment = Nodes[ node.Count() - 1];
  TTypeList< AnAssociation2<TAutoDBNode*, int> > S1Match;
  TTypeList< AnAssociation3<TAutoDBNetNode*, int, TAutoDBIdPList*> > S2Match, S3Match;
  for( int i=0; i < segment.Count(); i++ )  {
    double fom = 0;
    if( segment[i]->IsMetricSimilar( *node.Center(), fom ) )  {
      //
      bool found = false;
      for(int j=0; j < S1Match.Count(); j++ )  {
        if( S1Match[j].GetA()->IsSameType(*segment[i]) )  {
          S1Match[j].B() ++;
          found = true;
          break;
        }
      }
      if( !found )  S1Match.AddNew<TAutoDBNode*,int>(segment[i], 1);
      //
      for( int j=0; j < segment[i]->ParentCount(); j++ )  {
        double cfom = 0;
        TAutoDBNetNode& netnd = segment[i]->GetParent(j)->Node( segment[i]->GetParentIndex(j) );
        //TBasicApp::GetLog().Info( Nodes[i]->GetParent(j)->Reference()->GetName());
        if( netnd.IsMetricSimilar(node, cfom, NULL, false) )  {
          //
          found = false;
          for(int k=0; k < S2Match.Count(); k++ )  {
            if( S2Match[k].GetA()->IsSameType(netnd, false) )  {
              S2Match[k].B() ++;
              S2Match[k].C()->Add( segment[i]->GetParent(j)->Reference() );
              found = true;
              break;
            }
          }
          if( !found )  {
            S2Match.AddNew<TAutoDBNetNode*,int,TAutoDBIdPList*>(&netnd, 1, new TAutoDBIdPList);
            S2Match[S2Match.Count()-1].C()->Add( segment[i]->GetParent(j)->Reference() );
          }
          //
          if( netnd.IsMetricSimilar(node, cfom, NULL, true) )  {
            //
            found = false;
            for(int k=0; k < S3Match.Count(); k++ )  {
              if( S3Match[k].GetA()->IsSameType(netnd, false) )  {
                S3Match[k].B() ++;
                S3Match[k].C()->Add( segment[i]->GetParent(j)->Reference() );
                found = true;
                break;
              }
            }
            if( !found )  {
              S3Match.AddNew<TAutoDBNetNode*,int,TAutoDBIdPList*>(&netnd, 1, new TAutoDBIdPList);
              S3Match[S3Match.Count()-1].C()->Add( segment[i]->GetParent(j)->Reference() );
            }
            //
          }
        }
      }
    }
  }
  if( S1Match.Count() != 0 )  {
    report.Add( "S1 matches:" );
    for(int i=0; i < S1Match.Count(); i++ )  {
      report.Add( olxstr("   ") << S1Match[i].GetA()->ToString() <<
        ' ' << S1Match[i].GetB() << " hits" );
    }
    if( S2Match.Count() != 0 )  {
      report.Add( "S2 matches:" );
      for(int i=0; i < S2Match.Count(); i++ )  {
        report.Add( olxstr("   ") << S2Match[i].GetA()->ToString(1) <<
          ' ' << S2Match[i].GetB() << " hits" );
        olxstr tmp("Refs [");
        for( int j=0; j < S2Match[i].GetC()->Count(); j++ )  {
          tmp << LocateFileName(*S2Match[i].GetC()->Item(j)) << ';';
        }
        report.Add( tmp << ']' );
        delete S2Match[i].GetC();
      }
      if( S3Match.Count() != 0 )  {
        report.Add( "S3 matches:" );
        for(int i=0; i < S3Match.Count(); i++ )  {
          report.Add( olxstr("   ") << S3Match[i].GetA()->ToString(2) <<
            ' ' << S3Match[i].GetB() << " hits" );
          olxstr tmp("Refs [");
          for( int j=0; j < S3Match[i].GetC()->Count(); j++ )  {
            tmp << LocateFileName(*S3Match[i].GetC()->Item(j)) << ';';
          }
          report.Add( tmp << ']' );
          delete S3Match[i].GetC();
        }
      }
    }
  }

  for( int i=0; i < sn->Count(); i++ )
    delete sn->Node(i).Center();
  delete sn;
  return;
}
//..............................................................................
void TAutoDB::AnalyseStructure(const olxstr& lastFileName, TLattice& latt, 
                               TAtomTypePermutator* permutator, TAutoDB::AnalysisStat& stat,
                               TBAIPList* proposed_atoms)  {

  LastFileName = lastFileName;
  stat.Clear();
  stat.FormulaConstrained = (proposed_atoms != NULL);
  stat.AtomDeltaConstrained = (BAIDelta != -1);
  Uisos.Clear();
  for( int i=0; i < latt.FragmentCount(); i++ )  {
    Uisos.Add(0.0);
    AnalyseNet( latt.GetFragment(i), permutator, Uisos[Uisos.Count()-1], stat, proposed_atoms );
  }
  LastStat = stat;
}
//..............................................................................
int SortGuessListByCount(const AnAssociation3<double, TBasicAtomInfo*, int>& a,
                         const AnAssociation3<double, TBasicAtomInfo*, int>& b )  {
  return a.GetC() - b.GetC();
}
//..............................................................................
long TAutoDB::TAnalyseNetNodeTask::LocateDBNodeIndex(const TPtrList<TAutoDBNode>& segment,
  TAutoDBNode* nd, long from, long to)  {

  if( from == -1 ) from = 0;
  if( to == -1 )  to = segment.Count()-1;
  if( to == from )  {  return to;  }
  if( (to-from) == 1 )  return from;
  int resfrom = SearchCompareFunc(segment[from], nd),
      resto   = SearchCompareFunc(segment[to], nd);
  if( !resfrom )  return from;
  if( !resto )    return to;
  if( resfrom < 0 && resto > 0 )  {
    int index = (to+from)/2;
    int res = SearchCompareFunc(segment[index], nd);
    if( res < 0 )  {  return LocateDBNodeIndex(segment, nd, index, to);  }
    if( res > 0 )  {  return LocateDBNodeIndex(segment, nd, from, index);  }
    if( res == 0 )  {  return index;  }
  }
  return -1;
}
//..............................................................................
void TAutoDB::TAnalyseNetNodeTask::Run( long index )  {
  const TAutoDBNetNode& nd = Network.Node(index);
  if( nd.Count() < 1 || nd.Count() > Nodes.Count() ) return;
  const TPtrList< TAutoDBNode >& segment = Nodes[ nd.Count() - 1 ];
  //long ndind = LocateDBNodeIndex(segment, nd.Center());
  //if( ndind == -1 )  return;
  //long position = ndind, inc = 1;
  TGuessCount& gc = Guesses[index];
  bool found;
  double fom, cfom; //, var;
  for( int i=0; i < segment.Count(); i++ )  {
//    if( position >= segment.Count() )  {
//      inc = -1;
//      position = ndind - 1;
//    }
//    if( position < 0 )  return;
    fom = 0;
    TAutoDBNode& segnd = *segment[i];
//    TAutoDBNode& segnd = *segment[position];
//    var = nd.Center()->SearchCompare( segnd, &fom );
//    if( var == 0 )  {
    if( nd.Center()->IsMetricSimilar( segnd, fom ) )  {
      for( int j=0; j < segnd.ParentCount(); j++ )  {
        TAutoDBNetNode& netnd = segnd.GetParent(j)->Node( segnd.GetParentIndex(j) );
        cfom = 0;
        if( nd.IsMetricSimilar(netnd, cfom, NULL, false) )  {
          found = false;
          for( int k=0; k < gc.list2->Count(); k++ )  {
            if( gc.list2->Item(k).BAI == netnd.Center()->BAI() )  {
              gc.list2->Item(k).hits.AddNew( &netnd, cfom);
              found = true;
              break;
            }
          }
          if( !found )  {
            gc.list2->AddNew(netnd.Center()->BAI(), &netnd, cfom);
          }
          if( nd.IsMetricSimilar(netnd, cfom, NULL, true) )  {
            found = false;
            for( int k=0; k < gc.list3->Count(); k++ )  {
              if( gc.list3->Item(k).BAI == netnd.Center()->BAI() )  {
                gc.list3->Item(k).hits.AddNew(&netnd, cfom);
                found = true;
                break;
              }
            }
            if( !found )
              gc.list3->AddNew(netnd.Center()->BAI(), &netnd, cfom);
          }
        }
      }
      found = false;
      for( int j=0; j < gc.list1->Count(); j++ )  {
        if( gc.list1->Item(j).BAI == segnd.BAI() )  {
          gc.list1->Item(j).hits.AddNew( &segnd, fom);
          found = true;
          break;
        }
      }
      if( !found )
        gc.list1->AddNew(segnd.BAI(), &segnd, fom);
    }
//    if( var == 0 || olx_abs(var) <= LengthVar*4 )  {
//      position += inc;
//    }
//    else  {  // no match
//      if( inc == 1 )  {  // start in opposit direction
//        inc = -1;
//        position = ndind - 1;
//      }
//      else   // finsished the opposit direction too
//        return;
//    }
  }
}
//..............................................................................
void TAutoDB::AnalyseNet(TNetwork& net, TAtomTypePermutator* permutator, 
                         double& Uiso, TAutoDB::AnalysisStat& stat, TBAIPList* proposed_atoms)  {
  //TPSTypeList<double, TAutoDBNode*> hits;
  TSAtomPList cas;
  TAutoDBNet* sn = BuildSearchNet( net, cas );
  olxstr tmp;
  for(int i=0; i < net.NodeCount(); i++ )
    net.Node(i).SetTag(-1);
  if( sn == NULL )  return;
  const int sn_count = sn->Count();
  TTypeList< TGuessCount > guesses;
  guesses.SetCapacity( sn_count);
  for( int i=0; i < sn_count; i++ )  {
    sn->Node(i).SetTag(-1);
    sn->Node(i).SetId(0);
    TGuessCount& gc = guesses.AddNew();
    gc.list1 = new TTypeList< THitList<TAutoDBNode> >;
    gc.list2 = new TTypeList< THitList<TAutoDBNetNode> >;
    gc.list3 = new TTypeList< THitList<TAutoDBNetNode> >;
    gc.list1->SetCapacity(12);
    gc.list2->SetCapacity(6);
    gc.list3->SetCapacity(3);
    gc.atom = &cas[i]->CAtom();
  }
  TAnalyseNetNodeTask analyseNetNodeTask(Nodes, *sn, guesses);
  TListIteratorManager<TAnalyseNetNodeTask> nodesAnalysis(analyseNetNodeTask, sn_count, tLinearTask, 0);
  //TListIteratorManager<TAnalyseNetNodeTask> nodesAnalysis(analyseNetNodeTask, sn->Count(), tQuadraticTask);
  int cindexes[MaxConnectivity];
  int UisoCnt = 0;
  for(int i=0; i < sn_count; i++ )  {
    if( !guesses[i].list3->IsEmpty() )
      sn->Node(i).SetId(2);
    else if ( !guesses[i].list2->IsEmpty() )
      sn->Node(i).SetId(1);
    else
      sn->Node(i).SetId(0);
  }
  // analysis of "confident", L3 and L2 atom types and Uiso
  for(int i=0; i < sn_count; i++ )  {
    if( sn->Node(i).GetId() == 0 )  continue;
    tmp = EmptyString;
    TTypeList< THitList<TAutoDBNetNode> >* guessN =
      !guesses[i].list3->IsEmpty() ? guesses[i].list3 : guesses[i].list2;
    for( int j=0; j < guessN->Count(); j++ )
      guessN->Item(j).Sort();
    guessN->QuickSorter.SortSF(*guessN, THitList<TAutoDBNetNode>::SortByFOMFunc);
    tmp << guesses[i].atom->GetLabel() << ' ';
    double cfom = 0;
    sn->Node(i).IsMetricSimilar(*guessN->Item(0).hits[0].Node, cfom, cindexes, false);
    for( int j=0; j < sn->Node(i).Count(); j++ )  {
      if( sn->Node(i).Node(j)->GetTag() == -1 && sn->Node(i).Node(j)->GetId() == 0 )  {
        sn->Node(i).Node(j)->SetTag( 
          guessN->Item(0).hits[0].Node->Node( cindexes[j] )->Center()->BAI()->GetIndex() );
      }
      else  {
        int from = sn->Node(i).Node(j)->GetTag();
        int to = guessN->Item(0).hits[0].Node->Node( cindexes[j] )->Center()->BAI()->GetIndex();
        if( from != -1 && from != to )
          TBasicApp::GetLog().Info( "Oups ..." );
      }
    }
    if( sn->Node(i).Count() == 1 )  // normally wobly
      Uiso += guesses[i].atom->GetUiso()*3./4.;
    else
      Uiso += guesses[i].atom->GetUiso();
    UisoCnt ++;
    stat.ConfidentAtomTypes++;
  }
  if( UisoCnt != 0 )  Uiso /= UisoCnt;
  if( UisoCnt != 0 )
    TBasicApp::GetLog().Info( olxstr("Mean Uiso for confident atom types is ") << olxstr::FormatFloat(3,Uiso) );
  else
    TBasicApp::GetLog().Info("Could not locate confident atom types");
  // assigning atom types according to L3 and L2 and printing stats
  for(int i=0; i < sn_count; i++ )  {
    if( sn->Node(i).GetId() == 0 )  continue;
    tmp = EmptyString;
    TTypeList< THitList<TAutoDBNetNode> >* guessN =
      !guesses[i].list3->IsEmpty() ? guesses[i].list3 : guesses[i].list2;

    TBasicAtomInfo* type = guessN->Item(0).BAI;
    sn->Node(i).SetTag( type->GetIndex() );
    for( int j=0; j < guessN->Count(); j++ )  {
      tmp << guessN->Item(j).BAI->GetSymbol() << '(' << olxstr::FormatFloat(2,1.0/(guessN->Item(j).MeanFomN(1)+0.001)) << ")";
      if( (j+1) < guessN->Count() )  tmp << ',';
    }
    if( permutator == NULL || !permutator->IsActive() )  {
      bool searchHeavier = false, searchLighter = false; // have to do it here too!
      if( UisoCnt != 0 && Uiso != 0 )  {
        double uiso = sn->Node(i).Count() == 1 ? guesses[i].atom->GetUiso()*3./4. : guesses[i].atom->GetUiso();
        double scale = uiso / Uiso;
        if( scale > URatio )          searchLighter = true;
        else if( scale < 1./URatio )  searchHeavier = true;
      }
      if( searchLighter || searchHeavier )  {
        AnalyseUiso(*guesses[i].atom, *guessN, stat, searchHeavier, searchLighter, proposed_atoms);
        sn->Node(i).SetTag( guesses[i].atom->GetAtomInfo().GetIndex() ); // enforce atom type
      }
      else  {
        if( type != NULL && *type != guesses[i].atom->GetAtomInfo() )  {
          if( proposed_atoms != NULL )  {
            if( proposed_atoms->IndexOf( type ) != -1 )  {
              stat.AtomTypeChanges++;
              guesses[i].atom->Label() =  (olxstr( type->GetSymbol() ) << (i+1));
              guesses[i].atom->SetAtomInfo( type );
            }
          }
          else if( BAIDelta != -1 )  {
            if( abs(type->GetIndex() - guesses[i].atom->GetAtomInfo().GetIndex()) < BAIDelta )  {
              stat.AtomTypeChanges++;
              guesses[i].atom->Label() =  (olxstr( type->GetSymbol() ) << (i+1));
              guesses[i].atom->SetAtomInfo( type );
            }
          }
          else  {
            stat.AtomTypeChanges++;
            guesses[i].atom->Label() =  (olxstr( type->GetSymbol() ) << (i+1));
            guesses[i].atom->SetAtomInfo( type );
          }
        }
      }
    }
    TBasicApp::GetLog().Info( tmp );
  }
  for(int i=0; i < sn_count; i++ )  {
    if( sn->Node(i).GetTag() != -1 && guesses[i].atom->GetAtomInfo() != sn->Node(i).GetTag() )  {
      int change_evt = -1;
      TBasicAtomInfo* l_bai = &AtomsInfo.GetAtomInfo(sn->Node(i).GetTag());
      if( proposed_atoms != NULL )  { // change to only provided atoms if in the guess list
        if( proposed_atoms->IndexOf( l_bai ) != -1 )  {
          change_evt = 0;
          guesses[i].atom->Label() = (olxstr( l_bai->GetSymbol() ) << (i+1));
          guesses[i].atom->SetAtomInfo(l_bai);
        }
      }
      else  if( BAIDelta != -1 )  { // consider atom types within BAIDelta only
        if( abs(guesses[i].atom->GetAtomInfo().GetIndex() - sn->Node(i).GetTag()) < BAIDelta )  {
          change_evt = 1;
          guesses[i].atom->Label() = (olxstr( l_bai->GetSymbol() ) << (i+1));
          guesses[i].atom->SetAtomInfo(l_bai);
        }
      }
      else  {  // unrestrained assignment
        change_evt = 2;
        guesses[i].atom->Label() = (olxstr( l_bai->GetSymbol() ) << (i+1));
        guesses[i].atom->SetAtomInfo(l_bai);
      }
      if( change_evt != -1 )  {
        TBasicApp::GetLog().Info( olxstr("SN[") << change_evt << "] assignment " << guesses[i].atom->GetLabel() <<
              " to " << l_bai->GetSymbol() );
        stat.AtomTypeChanges++;
        stat.SNAtomTypeAssignments++;
      }
    }
  }
  for(int i=0; i < sn_count; i++ )  {
    if( sn->Node(i).GetTag() == -1 )  {
      bool searchHeavier = false, searchLighter = false;
      if( UisoCnt != 0 && Uiso != 0 )  {
        double scale = guesses[i].atom->GetUiso() / Uiso;
        if( scale > URatio )          searchLighter = true;
        else if( scale < 1./URatio )  searchHeavier = true;
      }
      //else if( Uiso == 0 )
      //  searchLighter = true;
      if( permutator != NULL && permutator->IsActive() )
        permutator->InitAtom( guesses[i] );
      tmp = EmptyString;
      for( int j=0; j < guesses[i].list1->Count(); j++ )
        guesses[i].list1->Item(j).Sort();
      guesses[i].list1->QuickSorter.SortSF(*guesses[i].list1, THitList<TAutoDBNode>::SortByFOMFunc);
      if( guesses[i].list1->Count() != 0 )  {
        tmp << guesses[i].atom->GetLabel() << ' ';
        TBasicAtomInfo* type = &guesses[i].atom->GetAtomInfo();
        if( searchHeavier )  {
          TBasicApp::GetLog().Info( olxstr("Searching element heavier for ") << guesses[i].atom->GetLabel() );
          for( int j=0; j < guesses[i].list1->Count(); j++ )  {
            if( guesses[i].list1->Item(j).BAI->GetIndex() > type->GetIndex() )  {
              if( proposed_atoms != NULL )  {
                if( proposed_atoms->IndexOf( guesses[i].list1->Item(j).BAI ) != -1 )  {
                  type = guesses[i].list1->Item(j).BAI;
                  break;
                }
              }
              else if( BAIDelta != -1 )  {
                if( (guesses[i].list1->Item(j).BAI->GetIndex() - guesses[i].atom->GetAtomInfo().GetIndex()) < BAIDelta )  {
                  type = guesses[i].list1->Item(j).BAI;
                  break;
                }
              }
//              else  {
//              }
            }
          }
        }
        else if( searchLighter )  {
          TBasicApp::GetLog().Info( olxstr("Searching element lighter for ") << guesses[i].atom->GetLabel() );
          for( int j=0; j < guesses[i].list1->Count(); j++ )  {
            if( guesses[i].list1->Item(j).BAI->GetIndex() < type->GetIndex() )  {
              if( proposed_atoms != NULL )  {
                if( proposed_atoms->IndexOf( guesses[i].list1->Item(j).BAI ) != -1 )  {
                  type = guesses[i].list1->Item(j).BAI;
                  break;
                }
              }
              else if( BAIDelta != -1 )  {
                if( (guesses[i].atom->GetAtomInfo().GetIndex() - guesses[i].list1->Item(j).BAI->GetIndex()) < BAIDelta  )  {
                  type = guesses[i].list1->Item(j).BAI;
                  break;
                }
              }
//              else  {
//              }
            }
          }
        }
        else  {
         type = NULL;  //guesses[i].list1->Item(0).BAI;
        }
        for( int j=0; j < guesses[i].list1->Count(); j++ )  {
          tmp << guesses[i].list1->Item(j).BAI->GetSymbol() << '(' <<
            olxstr::FormatFloat(3,1.0/(guesses[i].list1->Item(j).MeanFom()+0.001)) << ")" << guesses[i].list1->Item(j).hits[0].Fom;
          if( (j+1) < guesses[i].list1->Count() )  tmp << ',';
        }
        if( permutator == NULL || !permutator->IsActive() )  {
          if( type == NULL || *type == guesses[i].atom->GetAtomInfo() )  continue;
          if( proposed_atoms != NULL )  {
            if( proposed_atoms->IndexOf( type ) != -1 )  {
              stat.AtomTypeChanges++;
              guesses[i].atom->Label() =  (olxstr( type->GetSymbol() ) << (i+1));
              guesses[i].atom->SetAtomInfo( type );
            }
          }
          else if( BAIDelta != -1 )  {
            if( abs(type->GetIndex() - guesses[i].atom->GetAtomInfo().GetIndex()) < BAIDelta )  {
              stat.AtomTypeChanges++;
              guesses[i].atom->Label() =  (olxstr( type->GetSymbol() ) << (i+1));
              guesses[i].atom->SetAtomInfo( type );
            }
          }
          else  {
            stat.AtomTypeChanges++;
            guesses[i].atom->Label() =  (olxstr( type->GetSymbol() ) << (i+1));
            guesses[i].atom->SetAtomInfo( type );
          }
        }
        TBasicApp::GetLog().Info( tmp );
      }
    }
  }
  for( int i=0; i < sn_count; i++ )  {
    delete sn->Node(i).Center();
    delete guesses[i].list1;
    delete guesses[i].list2;
    delete guesses[i].list3;
  }
  delete sn;
}
//..............................................................................
void TAutoDB::ValidateResult(const olxstr& fileName, const TLattice& latt, TStrList& report)  {
  olxstr cifFN = TEFile::ChangeFileExt(fileName, "cif");
  report.Add( olxstr("Starting analysis of '") << cifFN << "' on " << TETime::FormatDateTime( TETime::Now() ) );
  if( !TEFile::FileExists(cifFN) )  {
    report.Add( olxstr("The cif file does not exist") );
    return;
  }
  try  {
    XFile.LoadFromFile( cifFN );
    if( XFile.GetAsymmUnit().DoesContainEquivalents() )  {
      XFile.GetLattice().Uniq(true);
      XFile.GetLattice().CompaqAll();
      XFile.GetLattice().Compaq();
    }
  }
  catch( const TExceptionBase& exc )  {
    report.Add( olxstr("Failed to load due to ") << exc.GetException()->GetError() );
    return;
  }
  TSpaceGroup* sga = TSymmLib::GetInstance()->FindSG( latt.GetAsymmUnit() );
  TSpaceGroup* sgb = TSymmLib::GetInstance()->FindSG( XFile.GetAsymmUnit() );
  if( sga == NULL || sgb == NULL )  {
    report.Add( olxstr("Could not evaluate space group") );
    return;
  }
  if( sga != sgb )  {
    report.Add( olxstr("Inconsistent space group. Changed from ") << sgb->GetName() << " to " << 
      sga->GetName() );
    return;
  }
  report.Add( olxstr("Current space group is ") << sga->GetName() );
  // have to locate possible translation using 'hard' method
  TTypeList< AnAssociation2<vec3d,TCAtom*> > alist, blist;
  TTypeList< TSymmTestData > vlist;
  latt.GetUnitCell().GenereteAtomCoordinates(alist, false);
  XFile.GetUnitCell().GenereteAtomCoordinates(blist, false);
  smatd mI;
  mI.r.I();
  TSymmTest::TestDependency(alist, blist, vlist, mI, 0.01);
  vec3d thisCenter, atomCenter;
  if( vlist.Count() != 0 )  {
    TBasicApp::GetLog().Info( vlist[vlist.Count()-1].Count() );
    if( vlist[vlist.Count()-1].Count() > (alist.Count()*0.75) )  {
      thisCenter = vlist[vlist.Count()-1].Center;
      if( !sga->IsCentrosymmetric() )
        thisCenter *= 2;
    }
  }

  int extraAtoms = 0, missingAtoms = 0;

  TAsymmUnit& au = latt.GetAsymmUnit();

  for( int i=0; i < XFile.GetAsymmUnit().AtomCount(); i++ )
    XFile.GetAsymmUnit().GetAtom(i).SetId(-1);

  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetAtomInfo() == iQPeakIndex || au.GetAtom(i).GetAtomInfo() == iHydrogenIndex )
      continue;
    atomCenter = au.GetAtom(i).ccrd();
    atomCenter -= thisCenter;
    TCAtom* ca = XFile.GetUnitCell().FindCAtom( atomCenter );
    if( ca == NULL )  {
      report.Add( olxstr("Extra atom '") << au.GetAtom(i).GetLabel() << '\'');
      extraAtoms++;
      continue;
    }
    ca->SetId(i);
    if( ca->GetAtomInfo() != au.GetAtom(i).GetAtomInfo() )  {
      report.Add( olxstr("Atom type changed from '") << ca->Label() <<
        "' to '" << au.GetAtom(i).GetLabel() << '\'' );
    }
  }
  for( int i=0; i < XFile.GetAsymmUnit().AtomCount(); i++ )  {
    if( XFile.GetAsymmUnit().GetAtom(i).GetId() == -1 &&
        XFile.GetAsymmUnit().GetAtom(i).GetAtomInfo() != iHydrogenIndex )  {
      report.Add( olxstr("Missing atom '") << XFile.GetAsymmUnit().GetAtom(i).GetLabel() << '\'');
      missingAtoms++;
    }
  }
  report.Add( olxstr("------Analysis complete with ") << extraAtoms << " extra atoms and " <<
    missingAtoms << " missing atoms-----" );
  report.Add(EmptyString);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAtomTypePermutator::Init(TPtrList<TBasicAtomInfo>* typeRestraints)  {
  Atoms.Clear();
  TypeRestraints.Clear();
  if( typeRestraints != NULL )
    TypeRestraints.AddList( *typeRestraints );
}
//..............................................................................
void TAtomTypePermutator::ReInit(const TAsymmUnit& au)  {
  // restore old atoms, remove deleted
  for( int i=0; i < Atoms.Count(); i++ )  {
    Atoms[i].Atom = au.GetLattice().GetUnitCell().FindCAtom( Atoms[i].AtomCenter );
    if( Atoms[i].Atom == NULL )
      Atoms.NullItem(i);
  }
  Atoms.Pack();
}
void TAtomTypePermutator::InitAtom(TAutoDB::TGuessCount& guess)  {
  TTypeList< TAutoDB::THitList<TAutoDBNetNode> >* list =
    (guess.list3->Count() != 0) ? guess.list3 : guess.list2;
  //if( list->IsEmpty() )  list = guess.list1;
  if( list->Count() > 0 || guess.list1->Count() > 0 )  {
    TPermutation* pm = NULL;
    for( int i=0; i < Atoms.Count(); i++ )  {
      if( Atoms[i].Atom == guess.atom )  {
        pm = &Atoms[i];
        break;
      }
    }
    if( list->Count() == 1 || (list->IsEmpty() && guess.list1->Count() == 1) )  {
      TBasicAtomInfo *bai = (list->Count() == 1) ? list->Item(0).BAI : guess.list1->Item(0).BAI;
      if( &guess.atom->GetAtomInfo() != bai )  {
        guess.atom->SetLabel( bai->GetSymbol() );
      }
      if( pm != NULL && pm->Tries.Count() )  {
        //Atoms.Delete(pmIndex);
        pm->Tries.Clear();
        TBasicApp::GetLog().Info( olxstr("Converged ") << guess.atom->GetLabel() );
      }
      return;
    }
    if( pm == NULL )  {
      pm = &Atoms.AddNew();
      pm->AtomCenter = guess.atom->ccrd();
      pm->Atom = guess.atom;
    }
    else  {  // check if converged
      if( pm->Tries.IsEmpty() )  return;
    }
    if( list->Count() > 1 )  {
      for( int i=0; i < list->Count(); i++ )  {
        bool found = false;
        for( int j=0; j < pm->Tries.Count(); j++ )  {
          if( pm->Tries[j].GetA() == list->Item(i).BAI )  {
            pm->Tries[j].C() = list->Item(i).MeanFom();
            if( list->Item(i).BAI == &guess.atom->GetAtomInfo() )
              pm->Tries[j].B() = guess.atom->GetUiso();
            found = true;
            break;
          }
        }
        if( !found )  {
          pm->Tries.AddNew<TBasicAtomInfo*,double,double>(list->Item(i).BAI, -1, list->Item(i).MeanFom());
          if( list->Item(i).BAI == &guess.atom->GetAtomInfo() )
            pm->Tries[pm->Tries.Count()-1].B() = guess.atom->GetUiso();
        }
      }
    }
    else if( guess.list1->Count() > 1 ) {
      for( int i=0; i < guess.list1->Count(); i++ )  {
        bool found = false;
        for( int j=0; j < pm->Tries.Count(); j++ )  {
          if( pm->Tries[j].GetA() == guess.list1->Item(i).BAI )  {
            pm->Tries[j].C() = guess.list1->Item(i).MeanFom();
            if( guess.list1->Item(i).BAI == &guess.atom->GetAtomInfo() )
              pm->Tries[j].B() = guess.atom->GetUiso();
            found = true;
            break;
          }
        }
        if( !found )  {
          pm->Tries.AddNew<TBasicAtomInfo*,double,double>(guess.list1->Item(i).BAI, -1, guess.list1->Item(i).MeanFom());
          if( guess.list1->Item(i).BAI == &guess.atom->GetAtomInfo() )
            pm->Tries[pm->Tries.Count()-1].B() = guess.atom->GetUiso();
        }
      }
    }
  }
}
//..............................................................................
void TAtomTypePermutator::Permutate()  {
  for( int i=0; i < Atoms.Count(); i++ )  {
    bool permuted = false;
    for( int j=0; j < Atoms[i].Tries.Count(); j++ )  {
      if( Atoms[i].Tries[j].GetB() == -1 )  {
        Atoms[i].Atom->SetLabel( olxstr( Atoms[i].Tries[j].GetA()->GetSymbol() ) );
        permuted = true;
        break;
      }
    }
    if( permuted )  {
      TBasicApp::GetLog().Info( olxstr(Atoms[i].Atom->GetLabel()) << " permutated") ;
    }
    else  {
      TBasicAtomInfo* type = NULL;
      double minDelta = 1;
      for( int j=0; j < Atoms[i].Tries.Count(); j++ )  {
        if( sqr(Atoms[i].Tries[j].GetB()-0.025) < minDelta )  {
          type = Atoms[i].Tries[j].GetA();
          minDelta = sqr(Atoms[i].Tries[j].GetB()-0.025);
        }
        TBasicApp::GetLog().Info( olxstr(Atoms[i].Atom->GetLabel()) << " permutation to " <<
          Atoms[i].Tries[j].GetA()->GetSymbol() << " leads to Uiso = " << Atoms[i].Tries[j].GetB() );
      }
      if( type != NULL )  {
        TBasicApp::GetLog().Info( olxstr("Most probable type is ") << type->GetSymbol() );
        if( &Atoms[i].Atom->GetAtomInfo() != type )  {
          Atoms[i].Atom->SetLabel( type->GetSymbol() );
        }
        Atoms[i].Tries.Clear();
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAutoDB::LibBAIDelta(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( BAIDelta );
  else
    BAIDelta = Params[0].ToInt();
}
//..............................................................................
void TAutoDB::LibURatio(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( URatio );
  else
    URatio = Params[0].ToDouble();
}
//..............................................................................
TLibrary*  TAutoDB::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("ata") : name );
  lib->RegisterFunction<TAutoDB>(
    new TFunction<TAutoDB>(this,  &TAutoDB::LibBAIDelta, "BAIDelta", fpNone|fpOne,
"Returns/sets maximum difference between element types to promote") );
  lib->RegisterFunction<TAutoDB>(
    new TFunction<TAutoDB>(this,  &TAutoDB::LibURatio, "URatio", fpNone|fpOne,
"Returns/sets a ration between atom U and mean U of the confident atoms to consider promotion") );
  return lib;
}
//..............................................................................



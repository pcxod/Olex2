#include "refmodel.h"
#include "lattice.h"
#include "symmparser.h"
#include "hkl.h"
#include "symmlib.h"

RefinementModel::RefinementModel(TAsymmUnit& au) : rDFIX(*this, rltBonds), rDANG(*this, rltBonds), 
  rSADI(*this, rltBonds), rCHIV(*this, rltAtoms), rFLAT(*this, rltGroup), rDELU(*this, rltAtoms), 
  rSIMU(*this, rltAtoms), rISOR(*this, rltAtoms), rEADP(*this, rltAtoms), 
  aunit(au), HklStatFileID(EmptyString, 0, 0), Vars(au)  {
  SetDefaults();
}
//....................................................................................................
void RefinementModel::SetDefaults() {
  HKLF = 4;
  HKLF_s = def_HKLF_s;
  HKLF_mat.I();
  HKLF_wt = def_HKLF_wt;
  HKLF_m = def_HKLF_m;
  MERG = def_MERG;
  OMIT_s = def_OMIT_s;
  OMIT_2t = def_OMIT_2t;
  MERG_set = OMIT_set = TWIN_set = false;
  TWIN_n = def_TWIN_n;
  TWIN_mat.I() *= -1;
}
//....................................................................................................
void RefinementModel::Clear() {
  for( int i=0; i < SfacData.Count(); i++ )
    delete SfacData.Object(i);
  SfacData.Clear();
  rDFIX.Clear();
  rDANG.Clear();
  rSADI.Clear();
  rCHIV.Clear();
  rFLAT.Clear();
  rSIMU.Clear();
  rDELU.Clear();
  rISOR.Clear();
  rEADP.Clear();
  rSAME.Clear();
  ExyzGroups.Clear();
  AfixGroups.Clear();
  UsedSymm.Clear();
  used_weight.Resize(0);
  proposed_weight.Resize(0);
  expl.Clear();
  RefinementMethod = "L.S.";
  SolutionMethod = EmptyString;
  HKLSource = EmptyString;
  Omits.Clear();
  BASF.Clear();
  SetDefaults();
  expl.Clear();
  Vars.Clear();
}
//....................................................................................................
const smatd& RefinementModel::AddUsedSymm(const smatd& matr) {
  int ind = UsedSymm.IndexOf(matr);
  smatd* rv = NULL;
  if( ind == -1 )  {
    rv = &UsedSymm.Add( *(new smatd(matr)) );
    rv->SetTag(1);
  }
  else  {
    UsedSymm[ind].IncTag();
    rv = &UsedSymm[ind];
  }
  return *rv;
}
//....................................................................................................
void RefinementModel::RemUsedSymm(const smatd& matr)  {
  int ind = UsedSymm.IndexOf(matr);
  if( ind == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
  UsedSymm[ind].DecTag();
  if( UsedSymm[ind].GetTag() == 0 )
    UsedSymm.Delete(ind);
}
//....................................................................................................
RefinementModel& RefinementModel::Assign(const RefinementModel& rm, bool AssignAUnit) {
  Clear();
  used_weight = rm.used_weight;
  proposed_weight = rm.proposed_weight;
  LS = rm.LS;
  PLAN = rm.PLAN;
  HKLF = rm.HKLF;
  HKLF_s = rm.HKLF_s;
  HKLF_mat = rm.HKLF_mat;
  HKLF_wt = rm.HKLF_wt;
  HKLF_m = rm.HKLF_m;
  MERG = rm.MERG;
  MERG_set = rm.MERG_set;
  OMIT_s = rm.OMIT_s;
  OMIT_2t = rm.OMIT_2t;
  OMIT_set = rm.OMIT_set;
  Omits = rm.Omits;
  TWIN_mat = rm.TWIN_mat;
  TWIN_n = rm.TWIN_n;
  TWIN_set = rm.TWIN_set;
  BASF = rm.BASF;
  HKLSource = rm.HKLSource;
  RefinementMethod = rm.RefinementMethod;
  SolutionMethod = rm.SolutionMethod;
  if( AssignAUnit )
    aunit.Assign(rm.aunit);
  Vars.Assign( rm.Vars );
  rDFIX.Assign(rm.rDFIX);
  rDANG.Assign(rm.rDANG);
  rSADI.Assign(rm.rSADI);
  rCHIV.Assign(rm.rCHIV);
  rFLAT.Assign(rm.rFLAT);
  rSIMU.Assign(rm.rSIMU);
  rDELU.Assign(rm.rDELU);
  rISOR.Assign(rm.rISOR);
  rEADP.Assign(rm.rEADP);
  rSAME.Assign(aunit, rm.rSAME);
  ExyzGroups.Assign(aunit, rm.ExyzGroups);
  AfixGroups.Assign(aunit, rm.AfixGroups);

  for( int i=0; i < rm.UsedSymm.Count(); i++ )
    UsedSymm.AddCCopy( rm.UsedSymm[i] );

  for( int i=0; i < rm.SfacData.Count(); i++ )
    SfacData.Add(rm.SfacData.GetComparable(i), new XScatterer( *rm.SfacData.GetObject(i)) );
  
  
  return *this;
}
//....................................................................................................
void RefinementModel::AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c, double mu, double r, double wt)  {
  olxstr lb(label.CharAt(0) == '$' ? label.SubStringFrom(1) : label);
  cm_Element* src = XElementLib::FindBySymbol(lb);
  XScatterer* sc;
  if( src != NULL )
    sc = new XScatterer(*src, expl.GetRadiationEnergy());
  else
    sc = new XScatterer;
  sc->SetLabel(lb);
  sc->SetGaussians(a1, a2, a3, a4, b1, b2, b3, b4, c);
  sc->SetAdsorptionCoefficient(mu);
  sc->SetBondingR(r);
  sc->SetWeight(wt);
  SfacData.Add(label, sc);
}
//....................................................................................................
double RefinementModel::FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2)  {
  for(int i=0; i < rDFIX.Count(); i++ )  {
    for( int j=0; j < rDFIX[i].AtomCount(); j+=2 )  {
      if( (rDFIX[i].GetAtom(j).GetAtom() == &a1 && rDFIX[i].GetAtom(j+1).GetAtom() == &a2) ||
          (rDFIX[i].GetAtom(j).GetAtom() == &a2 && rDFIX[i].GetAtom(j+1).GetAtom() == &a1) )  {
        return rDFIX[i].GetValue();
      }
    }
  }
  return -1;
}
//....................................................................................................
void RefinementModel::SetHKLSource(const olxstr& src) {
  if( HKLSource == src )  return;
  HKLSource = src;
}
//....................................................................................................
const RefinementModel::HklStat& RefinementModel::GetMergeStat() {
  // we need to take into the account MERG, HKLF and OMIT things here...
  try {
    TEFile::FileID hkl_src_id = TEFile::GetFileID(HKLSource);
    if( hkl_src_id == HklStatFileID )  {
      _HklStat.OmittedByUser = Omits.Count();  // this might change beyond the HKL file!
      return _HklStat;
    }
    else  {
      THklFile hf;
      hf.LoadFromFile(HKLSource);
      HklStatFileID = hkl_src_id;
      TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(aunit);
      if( sg == NULL )  // will not be seen outside though
        throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
      TRefPList refs;
      refs.SetCapacity( hf.RefCount() );
      const mat3d& hkl2c = aunit.GetHklToCartesian();
      const double h_o_s = 0.5*OMIT_s;
      double max_d = 2*sin(OMIT_2t*M_PI/360.0)/expl.GetRadiation();
      const double max_qd = QRT(max_d);  // maximum d squared
      const bool transform_hkl = !HKLF_mat.IsI();
      const int ref_cnt = hf.RefCount();
      _HklStat.MinD = 100;
      _HklStat.MaxD = -100;
      TRefPList Refs;
      Refs.SetCapacity(hf.RefCount());
      vec3d new_hkl;
      //apply OMIT transformation and filtering and calculate spacing limits
      for( int i=0; i < ref_cnt; i++ )  {
        TReflection& r = hf[i];
        if( r.GetI() < h_o_s*r.GetS()*r.GetI() )  {
          r.SetI( -h_o_s*r.GetI() );
          _HklStat.IntensityTransformed++;
        }
        if( transform_hkl )  {
          r.MulHkl(new_hkl, HKLF_mat);
          r.SetH( Round(new_hkl[0]) );
          r.SetK( Round(new_hkl[1]) );
          r.SetL( Round(new_hkl[2]) );
        }
        vec3d hkl(r.GetH()*hkl2c[0][0],
                  r.GetH()*hkl2c[0][1] + r.GetK()*hkl2c[1][1],
                  r.GetH()*hkl2c[0][2] + r.GetK()*hkl2c[1][2] + r.GetL()*hkl2c[2][2]);
        const double qd = hkl.QLength();
        if( qd < max_qd ) 
          Refs.Add(&r);
        else
          _HklStat.FilteredOff++;
        if( qd > _HklStat.MaxD ) 
          _HklStat.MaxD = qd;
        if( qd < _HklStat.MinD ) 
          _HklStat.MinD = qd;
      }
      _HklStat.LimD = sqrt(max_qd);
      _HklStat.MaxD = sqrt(_HklStat.MaxD);
      _HklStat.MinD = sqrt(_HklStat.MinD);
      _HklStat.OmittedByUser = Omits.Count();
      smatd_list ml;
      TRefList output;
      sg->GetMatrices(ml, mattAll^mattIdentity);
      if( MERG == 4 && !sg->IsCentrosymmetric() )  // merge all
        ml.AddNew().r.I() *= -1;
      _HklStat = RefMerger::Merge<TSimpleMerger>(ml, Refs, output);
    }
  }
  catch(TExceptionBase&)  {
    _HklStat.SetDefaults();
  }
  return _HklStat;
}
//....................................................................................................
void RefinementModel::ToDataItem(TDataItem& item) const {
  // save used equivalent positions
  TDataItem& eqiv = item.AddItem("eqiv");
  for( int i=0; i < UsedSymm.Count(); i++ )  
    eqiv.AddItem(i, TSymmParser::MatrixToSymmEx(UsedSymm[i]));
  
  Vars.ToDataItem(item.AddItem("vars"));
  
  AfixGroups.ToDataItem(item.AddItem("afix"));
  ExyzGroups.ToDataItem(item.AddItem("exyz"));
  rSAME.ToDataItem(item.AddItem("same"));
  rDFIX.ToDataItem(item.AddItem("dfix"));
  rDANG.ToDataItem(item.AddItem("dang"));
  rSADI.ToDataItem(item.AddItem("sadi"));
  rCHIV.ToDataItem(item.AddItem("chiv"));
  rFLAT.ToDataItem(item.AddItem("flat"));
  rDELU.ToDataItem(item.AddItem("delu"));
  rSIMU.ToDataItem(item.AddItem("simu"));
  rISOR.ToDataItem(item.AddItem("isor"));
  rEADP.ToDataItem(item.AddItem("eadp"));
  
  TDataItem& hklf = item.AddItem("HKLF", HKLF);
  hklf.AddField("s", HKLF_s);
  hklf.AddField("wt", HKLF_wt);
  hklf.AddField("m", HKLF_m);
  hklf.AddField("mat", TSymmParser::MatrixToSymmEx(HKLF_mat));

  TDataItem& omits = item.AddItem("OMIT", OMIT_set);
  omits.AddField("s", OMIT_s);
  omits.AddField("2theta", OMIT_2t);
  for( int i=0; i < Omits.Count(); i++ )  {
    TDataItem& omit = omit.AddItem(i);
    omit.AddField("h", Omits[i][0]);
    omit.AddField("k", Omits[i][1]);
    omit.AddField("l", Omits[i][2]);
  }
  if( TWIN_set )
    item.AddItem("TWIN", TWIN_n).AddField("mat", TSymmParser::MatrixToSymmEx(TWIN_mat));
  olxstr batch_s( BASF.IsEmpty() ? EmptyString : olxstr(BASF[0]));
  for( int i=1; i < BASF.Count(); i++ )
    batch_s << ' ' << BASF[i];
  if( !batch_s.IsEmpty() )
    item.AddField("BatchScales", batch_s);
  if( MERG_set )
    item.AddField("MERG", MERG);
  item.AddField("RefMeth", RefinementMethod);
  item.AddField("SolMeth", SolutionMethod);

  olxstr ref_in_arg( LS.Count() == 0 ? EmptyString : olxstr(LS[0]));
  for( int i=1; i < LS.Count(); i++ )
    ref_in_arg << ' ' << LS[i];
  item.AddField("RefInArg", ref_in_arg);

  olxstr ref_out_arg( PLAN.Count() == 0 ? EmptyString : olxstr(PLAN[0]));
  for( int i=1; i < PLAN.Count(); i++ )
    ref_out_arg << ' ' << PLAN[i];
  item.AddField("RefOutArg", ref_out_arg);
  item.AddCodedField("HklSrc", HKLSource);

}
//....................................................................................................
void RefinementModel::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//....................................................................................................

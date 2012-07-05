/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_refmodel_H
#define __olx_xl_refmodel_H
#include "asymmunit.h"
#include "srestraint.h"
#include "xscatterer.h"
#include "experimental.h"
#include "leq.h"
#include "refmerge.h"
#include "afixgroup.h"
#include "exyzgroup.h"
#include "reflection.h"
#include "fragment.h"
#include "symmlib.h"
#include "edict.h"
#include "constraints_ext.h"

BeginXlibNamespace()

static const double 
  def_HKLF_s  = 1,
  def_HKLF_wt = 1,
  def_HKLF_m  = 0,
  def_OMIT_s  = -2,
  def_OMIT_2t = 180.0,
  def_SHEL_hr = 0,
  def_SHEL_lr = 100;  // ['infinity' in A]
static const short 
  def_MERG   = 2,
  def_TWIN_n = 2;

// clear constants...
static uint32_t
  rm_clear_SAME = 0x00000001,
  rm_clear_AFIX = 0x00000002,
  rm_clear_VARS = 0x00000004,
  rm_clear_ALL  = 0xFFFFFFFF,
  rm_clear_DEF = (rm_clear_ALL^(rm_clear_SAME|rm_clear_AFIX|rm_clear_VARS));

class RefinementModel
  : public IXVarReferencerContainer, public IXVarReferencer, public IEObject
{
  // in INS file is EQUV command
  struct Equiv  {
    int ref_cnt;
    smatd symop;
    Equiv() : ref_cnt(0)  {}
    Equiv(const Equiv& e) : ref_cnt(e.ref_cnt), symop(e.symop)  {}
    Equiv(const smatd& so) : ref_cnt(0), symop(so)  {}
    Equiv& operator = (const Equiv& e) {
      symop = e.symop;
      ref_cnt = e.ref_cnt;
      return *this;
    }
  };
  olxdict<olxstr,Equiv,olxstrComparator<false> > UsedSymm;
  olxdict<olxstr,XScatterer*, olxstrComparator<false> > SfacData;
  olxdict<int, Fragment*, TPrimitiveComparator> Frags;
  ContentList UserContent;
protected:
  olxstr HKLSource;
  olxstr RefinementMethod,  // L.S. or CGLS
         SolutionMethod;
  int MERG, HKLF;
  mat3d HKLF_mat;
  double HKLF_s, HKLF_wt, HKLF_m;
  double OMIT_s, OMIT_2t;
  double SHEL_lr, SHEL_hr;
  double EXTI;
  mat3d TWIN_mat;
  int TWIN_n;
  bool TWIN_set, OMIT_set, MERG_set, HKLF_set, SHEL_set, OMITs_Modified,
    EXTI_set, DEFS_set;
  vec3i_list Omits;
  TDoubleList BASF, DEFS;
  TPtrList<XVarReference> BASF_Vars;
  olxstr VarRefrencerId;
  olxdict<olxstr, IXVarReferencerContainer*, olxstrComparator<false> >
    RefContainers;
  void SetDefaults();
  TTypeListExt<class InfoTab, IEObject> InfoTables;
  // adds a givin direction (if unique) and returns its name
  adirection *AddDirection(const TCAtomGroup &atoms, uint16_t type);
public:
  // needs to be extended for the us of the batch numbers...
  struct HklStat : public MergeStats {
    double MaxD, MinD, LimDmin, LimDmax, 
      OMIT_s, OMIT_2t, SHEL_lr, SHEL_hr, MaxI, MinI, HKLF_m, HKLF_s,
      Completeness;
    mat3d HKLF_mat;
    int MERG;
    //vec3i maxInd, minInd;
    size_t FilteredOff, // by LimD, OMIT_2t, SHEL_hr, SHEL_lr
      OmittedReflections, // refs after 0 0 0
      TotalReflections, // reflections read = OmittedRefs + TotalRefs
      IntensityTransformed;  // by OMIT_s
    vec3i FileMinInd, FileMaxInd;  // hkl range in the file (all before 0 0 0)
    HklStat()  {  SetDefaults();  }
    HklStat(const HklStat& hs) {  this->operator = (hs);  }
    HklStat& operator = (const HklStat& hs)  {
      MergeStats::operator = (hs);
      FileMinInd = hs.FileMinInd;
      FileMaxInd = hs.FileMaxInd;
      MaxD = hs.MaxD;         MinD = hs.MinD; 
      OMIT_s = hs.OMIT_s;     OMIT_2t = hs.OMIT_2t;
      SHEL_lr = hs.SHEL_lr;   SHEL_hr = hs.SHEL_hr;
      LimDmin = hs.LimDmin;   LimDmax = hs.LimDmax;
      MaxI = hs.MaxI;         MinI = hs.MinI;
      HKLF_m = hs.HKLF_m;     HKLF_s = hs.HKLF_s;
      HKLF_mat = hs.HKLF_mat;
      FilteredOff = hs.FilteredOff;
      IntensityTransformed = hs.IntensityTransformed;
      TotalReflections = hs.TotalReflections;
      OmittedReflections = hs.OmittedReflections;
      MERG = hs.MERG;
      Completeness = hs.Completeness;
      return *this;
    }
    HklStat& operator = (const MergeStats& ms)  {
      MergeStats::operator = (ms);
      return *this;
    }
    void SetDefaults()  {
      MergeStats::SetDefaults();
      MaxD = MinD = LimDmax = LimDmin = 0;
      MaxI = MinI = 0;
      HKLF_m = def_HKLF_m;
      HKLF_s = def_HKLF_s;
      HKLF_mat.I();
      FilteredOff = IntensityTransformed = OmittedByUser = 0;
      TotalReflections = OmittedReflections = 0;
      MERG = def_MERG;
      OMIT_s = def_OMIT_s;
      OMIT_2t = def_OMIT_2t;
      SHEL_lr = def_SHEL_lr;
      SHEL_hr = def_SHEL_hr;
      Completeness = 0;
    }
    size_t GetReadReflections() const {  return TotalReflections+OmittedReflections;  }
  };

  struct BadReflection {
    vec3i index;
    double Fo, Fc, esd;
    BadReflection(const vec3i &_index, double _Fo, double _Fc, double _esd)
    : index(_index), Fo(_Fo), Fc(_Fc), esd(_esd) {}
    BadReflection() : Fo(0), Fc(0), esd(0) {}
  };

protected:
  mutable HklStat _HklStat;
  mutable TRefList _Reflections;  // ALL un-merged reflections
  // if this is not the HKLSource, statistics is recalculated
  mutable TEFile::FileID HklStatFileID, HklFileID;
  mutable mat3d HklFileMat;
  mutable TIntList _Redundancy;
  mutable int _FriedelPairCount;  // the numbe of pairs only
  TTypeList<BadReflection> BadReflections;
  TActionQList Actions;
  olxstr AtomListToStr(const TTypeList<ExplicitCAtomRef> &al,
    size_t group_size, const olxstr &sep) const;
public:
  RefinementModel(TAsymmUnit& au);
  virtual ~RefinementModel() {  Clear(rm_clear_DEF);  }
  ExperimentalDetails expl;
  XVarManager Vars;
  TSRestraintList rDFIX,  // restrained distances (DFIX)
                  rDANG,  // restrained angles (DANG)
                  rSADI,  // similar distances (SADI)
                  rCHIV,  // restrained atomic volume (CHIV)
                  rFLAT,  // planar groups (FLAT)
                  rDELU,  // rigid bond restraints (DELU)
                  rSIMU,  // similar Uij (SIMU)
                  rISOR,  // Uij components approximate to isotropic behavior (ISOR)
                  rEADP,  // equivalent adp, constraint
                  rAngle,
                  rDihedralAngle,
                  rFixedUeq,
                  rSimilarUeq,
                  rSimilarAdpVolume;
  TExyzGroups ExyzGroups;
  TAfixGroups AfixGroups;
  TSameGroupList  rSAME;
  TActionQueue &OnSetBadReflections,
    &OnCellDifference;
  TAsymmUnit& aunit;
  ConstraintContainer<rotated_adp_constraint> SharedRotatedADPs;
  ConstraintContainer<same_group_constraint> SameGroups;
  ConstraintContainer<adirection> Directions;
  // restraints and constraints register
  olxdict<olxstr, IConstraintContainer*, olxstrComparator<false> > rcRegister;
  TPtrList<IConstraintContainer> rcList;  // when order matters
  TPtrList<TSRestraintList> rcList1;
  // removes references to all deleted atoms
  void Validate();
  // creates a human readable description of the refinement
  const_strlist Describe();
#ifndef _NO_PYTHON
  PyObject* PyExport(bool export_connectivity);
#endif

  TDoubleList used_weight, proposed_weight;
  TIntList LS;      // up to four params
  TDoubleList PLAN;  // up to three params

  ConnInfo Conn;          // extra connectivity information
  
  const olxstr& GetHKLSource() const {  return HKLSource;  }
  //TODO: handle the change
  void SetHKLSource(const olxstr& src);
  olxstr GetHKLFStr() const {  
    olxstr rv(HKLF, 80);
    if( HKLF_m == def_HKLF_m )  {
      if( HKLF_wt == def_HKLF_wt )  {
        if( HKLF_mat.IsI() )  {
          if( HKLF_s != def_HKLF_s )
            rv << ' ' << HKLF_s;
        }
        else {
          rv << ' ' << HKLF_s;
          for( int i=0; i < 9; i++ )
            rv << ' ' << HKLF_mat[i/3][i%3];
        }
      }
      else  {
        rv << ' ' << HKLF_s;
        for( int i=0; i < 9; i++ )
          rv << ' ' << HKLF_mat[i/3][i%3];
        rv << ' ' << HKLF_wt;
      }
    }
    else  {
      rv << ' ' << HKLF_s;
      for( int i=0; i < 9; i++ )
        rv << ' ' << HKLF_mat[i/3][i%3];
      rv << ' ' << HKLF_wt << ' ' << HKLF_m;
    }
    return rv;  
  }
  template <class list> void SetHKLF(const list& hklf) {
    if( hklf.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty HKLF");  
    HKLF = hklf[0].ToInt();
    if( HKLF > 4 )
      MERG = 0;
    if( hklf.Count() > 1 )
      HKLF_s = hklf[1].ToDouble();
    if( hklf.Count() > 10 )  {
      for( int i=0; i < 9; i++ )
        HKLF_mat[i/3][i%3] = hklf[2+i].ToDouble();
    }
    else if( hklf.Count() > 2 )  {
      TBasicApp::NewLogEntry(logError) <<
        (olxstr("Invalid HKLF matrix ignored: ").quote() << hklf.Text(' ', 2));
    }
    if( hklf.Count() > 11 )
      HKLF_wt = hklf[11].ToDouble();
    if( hklf.Count() > 12 )
      HKLF_wt = hklf[12].ToDouble();
    HKLF_set = true;
  }
  int GetHKLF() const {  return HKLF;  }
  void SetHKLF(int v)  {  HKLF = v;  HKLF_set = true;  }
  const mat3d& GetHKLF_mat() const {  return HKLF_mat;  }
  void SetHKLF_mat(const mat3d& v) {
    if( HKLF_mat != v ) // make sure it gets applied to the reflections
      _Reflections.Clear();
    HKLF_mat = v;
    HKLF_set = true;
  }
  double GetHKLF_s() const {  return HKLF_s;  }
  void SetHKLF_s(double v)  {  HKLF_s = v;  HKLF_set = true;  }
  double GetHKLF_wt() const {  return HKLF_wt;  }
  void SetHKLF_wt(double v)  {  HKLF_wt = v;  HKLF_set = true;  }
  double GetHKLF_m() const {  return HKLF_m;  }
  void SetHKLF_m(double v)  {  HKLF_m = v;  HKLF_set = true;  }
  bool IsHKLFSet() const {  return HKLF_set;  }

  int GetMERG() const {  return MERG;  }
  // MERG 4 specifies not to use f''
  bool UseFdp() const {  return MERG != 4;  }
  void SetMERG(int v)  {  MERG = v;  MERG_set = true;  }
  bool HasMERG() const {  return MERG_set;  }
  
  double GetEXTI() const {  return EXTI;  }
  void SetEXTI(double v)  {  EXTI = v;  EXTI_set = true;  }
  bool HasEXTI() const {  return EXTI_set;  }
  
  double GetOMIT_s() const {  return OMIT_s;  }
  void SetOMIT_s(double v)  {  OMIT_s = v;  OMIT_set = true;  }
  double GetOMIT_2t() const {  return OMIT_2t;  }
  void SetOMIT_2t(double v) {  OMIT_2t = v;  OMIT_set = true;}
  bool HasOMIT() const {  return OMIT_set;  }
  size_t OmittedCount() const {  return Omits.Count();  }
  const vec3i& GetOmitted(size_t i) const {  return Omits[i];  }
  void Omit(const vec3i& r)  {  Omits.AddCopy(r);  OMITs_Modified = true;  }
  void ClearOmits()  {  Omits.Clear();  OMITs_Modified = true;  }
  const vec3i_list& GetOmits() const {  return Omits;  }
  template <class list> void AddOMIT(const list& omit)  {
    if( omit.Count() == 3 )  {  // reflection omit
      Omits.AddNew(omit[0].ToInt(), omit[1].ToInt(), omit[2].ToInt());
      OMITs_Modified = true;
    }
    else  {  // reflection transformation/filtering
      if( omit.Count() > 0 )
        OMIT_s = omit[0].ToDouble();
      if( omit.Count() > 1 )
        OMIT_2t = omit[1].ToDouble();
      OMIT_set = true;
    }
  }
  olxstr GetOMITStr() const {
    return olxstr(OMIT_s) << ' ' << OMIT_2t;
  }
  // processed user omits (hkl) and returns the number of removed reflections
  size_t ProcessOmits(TRefList& refs);

  const TTypeList<BadReflection> &GetBadReflectionList() const {
    return BadReflections;
  }

  void SetBadReflectionList(const ConstTypeList<BadReflection> &bad_refs) {
    OnSetBadReflections.Enter(this);
    BadReflections = bad_refs;
    OnSetBadReflections.Exit(this);
  }

  // SHEL reflection resolution filter low/high
  double GetSHEL_lr() const {  return SHEL_lr;  }
  void SetSHEL_lr(double v)  {  SHEL_lr = v;  SHEL_set = true;  }
  double GetSHEL_hr() const {  return SHEL_hr;  }
  void SetSHEL_hr(double v)  {  SHEL_hr = v;  SHEL_set = true;  }
  bool HasSHEL() const {  return SHEL_set;  }
  template <class list> void SetSHEL(const list& shel)  {
    if( shel.Count() > 0 )  {
      SHEL_lr = shel[0].ToDouble();
      if( shel.Count() > 1 )
        SHEL_hr = shel[1].ToDouble();
      SHEL_set = true;
    }
  }
  olxstr GetSHELStr() const {  return olxstr(SHEL_lr) << ' ' << SHEL_hr;  }

  const TDoubleList& GetBASF() const {  return BASF;  }
  // returns a list of [1-sum(basf), basf[0], basf[1],...] - complete scales
  TDoubleList GetScales() const {
    if( !GetBASF().IsEmpty() )  {
      double pi = 0;  // 'prime' reflection fraction
      for( size_t bi=0; bi < GetBASF().Count(); bi++ )
        pi += GetBASF()[bi];
      return TDoubleList() << 1-pi << GetBASF();
    }
    else  {
      if( GetTWIN_n() != 0 )  {  // all the fractions are the same
        double f = 1./olx_abs(GetTWIN_n());
        TDoubleList rv(olx_abs(GetTWIN_n()));
        for( size_t i=0; i < rv.Count(); i++ )
          rv[i] = f;
        return rv;
      }
    }
    return TDoubleList();
  }
  olxstr GetBASFStr() const;
  
  template <class list> void SetTWIN(const list& twin) {
    if( twin.Count() > 8 )  {
      for( size_t i=0; i < 9; i++ )
        TWIN_mat[i/3][i%3] = twin[i].ToDouble();
    }
    if( twin.Count() > 9 )
      TWIN_n = twin[9].ToInt();
    TWIN_set = true;
  }
  olxstr GetTWINStr() const;
  const mat3d& GetTWIN_mat()  const {  return TWIN_mat;  }
  void SetTWIN_mat(const mat3d& m)  {  TWIN_mat = m;  TWIN_set = true;  }
  /* ShelXL manual:
If the racemic twinning is present at the same time as normal twinning, n
should be doubled (because there are twice as many components as before) and
given a negative sign (to indicate to the program that the inversion operator
is to be applied multiplicatively with the specified TWIN matrix). The number
of BASF parameters, if any, should be increased from m-1 to 2m-1 where m is the
original number of components (equal to the new |n| divided by 2). The TWIN
matrix is applied m-1 times to generate components 2 ... m from the prime
reflection (component 1); components m+1 ... 2m are then generated as the
Friedel opposites of components 1 ... m  
*/
  int GetTWIN_n() const {  return TWIN_n;  }
  void SetTWIN_n(int v)  {  TWIN_n = v;  TWIN_set = true;  }
  bool HasTWIN() const {  return TWIN_set;  }
  void RemoveTWIN()  {  TWIN_set = false;  }

  void AddBASF(double val)  {  
    BASF.Add(val);  
    BASF_Vars.Add(NULL);
  }
  template <class list> void SetBASF(const list& bs)  {
    BASF.SetCount(bs.Count());
    BASF_Vars.SetCount(bs.Count());
    for( uint16_t i=0; i < bs.Count(); i++ )  {
      BASF_Vars[i] = NULL;
      BASF[i] = Vars.SetParam(*this, i, bs[i].ToDouble());
    }
  }
  void ClearBASF()  {
    BASF.Clear();
    BASF_Vars.Clear();
  }
  // sets default esd values for restraints
  template <class list> void SetDEFS(const list& bs)  {
    size_t mc = olx_min(bs.Count(), DEFS.Count());
    for( size_t i=0; i < mc; i++ )
      DEFS[i] = bs[i].ToDouble();
    DEFS_set = true;
  }
  olxstr GetDEFSStr() const;
  bool IsDEFSSet() const {  return DEFS_set;  }

  DefPropC(olxstr, RefinementMethod)
  DefPropC(olxstr, SolutionMethod)
  
  void SetIterations(int v);
  void SetPlan(int v);

  /* clears restraints, SFAC and used symm but not AfixGroups, Exyzroups and
  Vars
  */
  void Clear(uint32_t clear_mask);
  // to be called by the Vars
  void ClearVarRefs();

  void AddInfoTab(const TStrList& l);
  size_t InfoTabCount() const {  return InfoTables.Count();  }
  const InfoTab& GetInfoTab(size_t i) const {  return InfoTables[i];  }
  InfoTab& GetInfoTab(size_t i)  {  return InfoTables[i];  }
  void DeleteInfoTab(size_t i)  {  InfoTables.Delete(i);  }
  InfoTab& AddHTAB();
  InfoTab& AddRTAB(const olxstr& codename, const olxstr& resi=EmptyString());
  bool ValidateInfoTab(const InfoTab& it);
  // adss new symmetry matrics, used in restraints/constraints 
  const smatd& AddUsedSymm(const smatd& matr, const olxstr& id=EmptyString());
  //removes the matrix or decriments the reference count
  void RemUsedSymm(const smatd& matr) const;
  // returns the number of the used symmetry matrices
  size_t UsedSymmCount() const {  return UsedSymm.Count();  }
  // returns used symmetry matric at specified index
  const smatd& GetUsedSymm(size_t ind) const {
    return UsedSymm.GetValue(ind).symop;
  }
  /* return index of given symmetry matrix in the list or -1, if it is not in
  the list
  */
  size_t UsedSymmIndex(const smatd& matr) const;
  // deletes all used symmetry matrices
  void ClearUsedSymm()  {  UsedSymm.Clear();  }
  const smatd* FindUsedSymm(const olxstr& name) const {
    const size_t i = UsedSymm.IndexOf(name);
    return (i == InvalidIndex ? NULL : &UsedSymm.GetValue(i).symop);
  }
  /* initialises ID's of the matrices to conform to the unit cell, this called
  by TLattice
  */
  void UpdateUsedSymm(const class TUnitCell& uc);
  // throws an exception if not found
  adirection& DirectionById(const olxstr &id) const;
  // adds new custom scatterer (created with new, will be deleted)
  void AddSfac(XScatterer& sc);
  // returns number of custom scatterers
  size_t SfacCount() const {  return SfacData.Count();  }
  // returns scatterer at specified index
  XScatterer& GetSfacData(size_t i) const {
    return *SfacData.GetValue(i);
  }
  // finds scatterer by label, returns NULL if nothing found
  XScatterer* FindSfacData(const olxstr& label) const {
    size_t ind = SfacData.IndexOf(label);
    return ind == InvalidIndex ? NULL : SfacData.GetValue(ind);
  }
  // user content management
  const ContentList& GetUserContent() const {  return UserContent;  }
  const olxstr GetUserContentStr() const {  
    olxstr rv;
    for( size_t i=0; i < UserContent.Count(); i++ )
      rv << UserContent[i].element.symbol << UserContent[i].count;
    return rv;
  }
  template <class StrLst> void SetUserContentType(const StrLst& sfac)  {
    UserContent.Clear();
    for( size_t i=0; i < sfac.Count(); i++ )
      AddUserContent(sfac[i], 0);
  }
  template <class StrLst> void SetUserContent(const StrLst& sfac,
    const StrLst& unit)
  {
    if( sfac.Count() != unit.Count() ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "UNIT/SFAC lists mismatch");
    }
    UserContent.Clear();
    for( size_t i=0; i < sfac.Count(); i++ )
      AddUserContent(sfac[i], unit[i].ToDouble());
  }
  void SetUserContent(const olxstr& sfac, const olxstr& unit)  {
    SetUserContent(TStrList(sfac, ' '), TStrList(unit, ' '));
  }
  void SetUserContent(const ContentList& cnt)  {
    UserContent = cnt;
  }
  template <class StrLst> void SetUserContentSize(const StrLst& unit)  {
    if( UserContent.Count() != unit.Count() ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "UNIT/SFAC lists mismatch");
    }
    for( size_t i=0; i < UserContent.Count(); i++ )
      UserContent[i].count = unit[i].ToDouble();
  }
  void AddUserContent(const olxstr& type, double amount=0)  {
    const cm_Element* elm = XElementLib::FindBySymbol(type);
    if( elm == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "element");
    UserContent.AddNew(*elm, amount);
  }
  void AddUserContent(const cm_Element& elm, double amount=0)  {
    UserContent.AddNew(elm, amount);
  }
  void SetUserFormula(const olxstr& frm, bool mult_z=true)  {
    UserContent.Clear();
    XElementLib::ParseElementString(frm, UserContent);
    for( size_t i=0; i < UserContent.Count(); i++ )
      UserContent[i].count *= (mult_z ? aunit.GetZ() : 1.0);
  }
  // returns the restrained distance or -1
  double FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2);

  template <class list> void AddEXYZ(const list& exyz) {
    if( exyz.Count() < 2 )
      throw TFunctionFailedException(__OlxSourceInfo, "incomplete EXYZ group");
    TExyzGroup& gr = ExyzGroups.New();
    for( size_t i=0; i < exyz.Count(); i++ )  {
      TCAtom* ca = aunit.FindCAtom(exyz[i]);
      if( ca == NULL )  {
        gr.Clear();
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("unknown atom: ") << exyz[i]);
      }
      gr.Add(*ca);
    }
  }

  RefinementModel& Assign(const RefinementModel& rm, bool AssignAUnit);
  /* updates refinable params - BASF, FVAR, WGHT, returns false if objects
  mismatch */
  bool Update(const RefinementModel& rm);

  size_t FragCount() const {  return Frags.Count();  }
  Fragment& GetFrag(size_t i)  {  return *Frags.GetValue(i);  }
  const Fragment& GetFrag(size_t i) const {  return *Frags.GetValue(i);  }
  Fragment* FindFragByCode(int code) {
    size_t ind = Frags.IndexOf(code);
    return ind == InvalidIndex ? NULL : Frags.GetValue(ind);
  }
  Fragment& AddFrag(int code, double a=1, double b=1, double c=1, double al=90,
    double be=90, double ga=90)
  {
    size_t ind = Frags.IndexOf(code);
    if( ind != InvalidIndex )  {
      throw TFunctionFailedException(__OlxSourceInfo,
        "duplicated FRAG instruction");
    }
    return *Frags.Add(code, new Fragment(code, a, b, c, al, be, ga));
  }
  // the function does the atom fitting and clears the fragments
  void ProcessFrags();

  const HklStat& GetMergeStat();
  // merged according to MERG
  template <class SymSpace, class Merger>
  HklStat GetRefinementRefList(const SymSpace& sp, TRefList& out)  {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    if( MERG != 0 && HKLF != 5 )  {
      bool mergeFP = (MERG == 4 || MERG == 3) && !sp.IsCentrosymmetric();
      stats = RefMerger::Merge<SymSpace,Merger>(sp, refs, out, 
        Omits, mergeFP);
      stats.MERG = MERG;
    }
    else
      stats = RefMerger::MergeInP1<Merger>(refs, out, Omits);
    return AdjustIntensity(out, stats);
  }
  // Friedel pairs always merged
  template <class Symm, class Merger>
  HklStat GetFourierRefList(const Symm& sp, TRefList& out) {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    stats = RefMerger::Merge<Symm,Merger>(sp, refs, out, Omits, true);
    return AdjustIntensity(out, stats);
  }
  // P-1 merged, filtered
  template <class Merger> HklStat GetWilsonRefList(TRefList& out) {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    smatd_list ml;
    ml.AddNew().I();
    stats = RefMerger::Merge<smatd_list,Merger>(ml, refs, out, Omits, true);
    return AdjustIntensity(out, stats);
  }
  // P1 merged, unfiltered
  template <class Merger> HklStat GetAllP1RefList(TRefList& out)  {
    HklStat stats;
    TRefList refs(GetReflections());
    vec3i_list empty_omits;
    stats = RefMerger::MergeInP1<Merger>(refs, out, empty_omits);
    return stats;
  }
  // P1 merged, filtered
  template <class Merger> HklStat GetFilteredP1RefList(TRefList& out)  {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    stats = RefMerger::MergeInP1<Merger>(refs, out, Omits);
    return AdjustIntensity(out, stats);
  }
  /* if none of the above functions help, try this ones
    return complete list of unmerged reflections (HKLF matrix, if any, is
    applied)
  */
  const TRefList& GetReflections() const;
  // this will be only valid if any list of the reflections was called
  const HklStat& GetReflectionStat() const {  return _HklStat;  } 
  // filters the reflections according to the parameters
  HklStat& FilterHkl(TRefList& out, HklStat& stats);
  // adjust intensity of reflections according to OMIT
  HklStat& AdjustIntensity(TRefList& out, HklStat& stats) const;
  /* returns redundancy information, like list[0] is the number of reflections
     collected once list[1] = number of reflections collected wtice etc
  */
  const TIntList& GetRedundancyInfo() const {
    GetReflections();
    return _Redundancy;
  }
  // uses algebraic relation
  void DetwinAlgebraic(TRefList& refs, const HklStat& st,
    const SymmSpace::InfoEx& info_ex) const;
  // convinience method for mrehedral::detwin<> with  detwin_mixed
  void DetwinMixed(TRefList& refs, const TArrayList<compd>& F,
    const HklStat& st, const SymmSpace::InfoEx& info_ex) const;
  // convinience method for mrehedral::detwin<> with  detwin_shelx
  void DetwinShelx(TRefList& refs, const TArrayList<compd>& F,
    const HklStat& st, const SymmSpace::InfoEx& info_ex) const;
  template <class RefList, class FList, class SymSpace>
  void CorrectExtiForF(const RefList& refs, FList& F, const SymSpace& sp) const
  {
    if( !EXTI_set || EXTI == 0 ) return;
    if( refs.Count() != F.Count() )
      throw TInvalidArgumentException(__OlxSrcInfo, "arrays size");
    const double l = expl.GetRadiation();
    for( size_t i=0; i < refs.Count(); i++ )  {
      const double x =
        sp.HklToCart(TReflection::GetHkl(refs[i])).QLength()*l*l/4;
      F[i] *=
        pow(1+0.0005*EXTI*F[i].qmod()*l*l*l/sqrt(olx_max(0,x*(1-x))), -0.25);
    }
  }
  template <class RefList, class FsqList, class SymSpace>
  void CorrectExtiForFsq(const RefList& refs, FsqList& Fsq,
    const SymSpace& sp) const
  {
    if( !EXTI_set || EXTI == 0 ) return;
    if( refs.Count() != Fsq.Count() )
      throw TInvalidArgumentException(__OlxSrcInfo, "arrays size");
    const double l = expl.GetRadiation();
    for( size_t i=0; i < refs.Count(); i++ )  {
      const double x
        = sp.HklToCart(TReflection::GetHkl(refs[i])).QLength()*l*l/4;
      Fsq[i] /= sqrt(1+0.0005*EXTI*Fsq[i]*l*l*l/sqrt(olx_max(0,x*(1-x))));
    }
  }
  // returns the number of pairs
  int GetFriedelPairCount() const {
    GetReflections();
    return _FriedelPairCount;
  }
  vec3i CalcMaxHklIndex(double two_theta=60) const;
  IXVarReferencerContainer& GetRefContainer(const olxstr& id_name)  {
    try {  return *RefContainers[id_name];  }
    catch(...)  {
      throw TInvalidArgumentException(__OlxSourceInfo, "container id");
    }
  }
// IXVarReferencer implementation
  virtual size_t VarCount() const {  return BASF.Count();  }
  virtual olxstr GetVarName(size_t i) const {  
    if( i >= BASF_Vars.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return olxstr("k") << (i+1);  
  }
  virtual XVarReference* GetVarRef(size_t i) const {  
    if( i >= BASF_Vars.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return BASF_Vars[i];  
  }
  virtual void SetVarRef(size_t i, XVarReference* var_ref)  {  
    if( i >= BASF_Vars.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    BASF_Vars[i] = var_ref;  
  }
  virtual const IXVarReferencerContainer& GetParentContainer() const {
    return *this;
  }
  virtual double GetValue(size_t var_index) const {  
    if( var_index >= BASF.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var_index");
    return BASF[var_index];  
  }
  virtual void SetValue(size_t var_index, const double& val) {  
    if( var_index >= BASF.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var_index");
    BASF[var_index] = val;  
  }
  virtual bool IsValid() const {  return true;  }
//
// IXVarReferencerContainer implementation
  virtual olxstr GetIdName() const {  return VarRefrencerId;  }
  virtual size_t GetIdOf(const IXVarReferencer &) const {  return 0;  }
  virtual size_t GetPersistentIdOf(const IXVarReferencer &) const {
    return 0;
  }
  virtual IXVarReferencer& GetReferencer(size_t) const {
    return const_cast<RefinementModel&>(*this);
  }
  virtual size_t ReferencerCount() const {  return 1;  }
//
  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);
  olxstr WriteInsExtras(const TCAtomPList* atoms,
    bool write_internals) const;
  void ReadInsExtras(const TStrList &items);
  // initialises default values for esd and if needs, value (SIMU)
  TSimpleRestraint &SetRestraintDefaults(TSimpleRestraint &restraint) const;
  // returns true if restraint parameters are default
  bool IsDefaultRestraint(const TSimpleRestraint &restraint) const;
  void LibOSF(const TStrObjList& Params, TMacroError& E);
  void LibFVar(const TStrObjList& Params, TMacroError& E);
  void LibEXTI(const TStrObjList& Params, TMacroError& E);
  void LibUpdateCRParams(const TStrObjList& Params, TMacroError& E);
  // restraints & constraints related functions
  void LibShareADP(TStrObjList &Cmds, const TParamList &Opts, TMacroError &E);

  TLibrary* ExportLibrary(const olxstr& name=EmptyString());

  struct ReleasedItems {
    TSimpleRestraintPList restraints;
    TPtrList<TSameGroup> sameList;
    //TPtrList<XLEQ> equations;
  };
};

EndXlibNamespace()

#endif
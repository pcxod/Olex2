/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "lattice.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "network.h"
#include "sbond.h"
#include "splane.h"
#include "ellipsoid.h"
#include "bapp.h"
#include "log.h"
#include "emath.h"
#include "congen.h"
#include "estlist.h"
#include "library.h"
#include "olxmps.h"
#include "estrbuffer.h"
#include "symmparser.h"
#include "equeue.h"
#include "analysis.h"
#include "estopwatch.h"

#undef GetObject

// sorts largest -> smallest
int TLattice_SortAtomsById(const TSAtom* a1, const TSAtom* a2)  {
  return olx_cmp(a1->CAtom().GetId(), a2->CAtom().GetId());
}
int TLattice_AtomsSortByDistance(const TSAtom* A1, const TSAtom* A2)  {
  const double d = A1->crd().QLength() - A2->crd().QLength();
  return (d < 0 ? -1 : ((d > 0 ) ? 1 : 0));
}

TLattice::TLattice(ASObjectProvider& ObjectProvider) :
  Objects(ObjectProvider),
  OnStructureGrow(Actions.New("STRGEN")),
  OnStructureUniq(Actions.New("STRUNIQ")),
  OnDisassemble(Actions.New("DISASSEBLE")),
  OnAtomsDeleted(Actions.New("ATOMSDELETE"))
{
  AsymmUnit = new TAsymmUnit(this);
  UnitCell = new TUnitCell(this);
  Network = new TNetwork(this, NULL);
  Delta = 0.5f;
  DeltaI = 1.2f;
  _GrowInfo = NULL;
}
//..............................................................................
TLattice::~TLattice()  {
  Clear(true);
  delete UnitCell;
  delete AsymmUnit;
  delete Network;
  if( _GrowInfo != NULL )
    delete _GrowInfo;
  delete &Objects;
}
//..............................................................................
void TLattice::ClearAtoms()  {
  if( !Objects.atoms.IsEmpty() )  {
    OnAtomsDeleted.Enter(this);
    Objects.atoms.Clear();
    OnAtomsDeleted.Exit(this);
  }
}
//..............................................................................
void TLattice::ClearFragments()  {
  for( size_t i=0; i < Fragments.Count(); i++ )
    delete Fragments[i];
  Fragments.Clear();
}
//..............................................................................
void TLattice::ClearMatrices()  {
  for( size_t i=0; i < Matrices.Count(); i++ )
    delete Matrices[i];
  Matrices.Clear();
}
//..............................................................................
void TLattice::Clear(bool ClearUnitCell)  {
  ClearAtoms();
  ClearBonds();
  ClearFragments();
  ClearMatrices();
  ClearPlanes();
  if( ClearUnitCell )  {
    GetUnitCell().Clear();
    GetAsymmUnit().Clear();
  }
}
//..............................................................................
ConstPtrList<smatd> TLattice::GenerateMatrices(
  const vec3d& VFrom, const vec3d& VTo)
{
  olxdict<uint32_t, smatd*, TPrimitiveComparator> matrices;
  const vec3i ts = (VFrom-vec3d(0)).Round<int>(),
    tt = (VTo-vec3d(0)).Round<int>();
  const TUnitCell &uc = GetUnitCell();
  TAsymmUnit &au = GetAsymmUnit();
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    const smatd& m = uc.GetMatrix(i);
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& ca = au.GetAtom(j);
      if( !ca.IsAvailable() )  continue;
      vec3d c = m*ca.ccrd();
      vec3i t = -c.Floor<int>();
      c += t;
      for (int tx=ts[0]; tx <= tt[1]; tx++) {
        for (int ty=ts[1]; ty <= tt[1]; ty++) {
          for (int tz=ts[2]; tz <= tt[2]; tz++) {
            vec3i t_(tx, ty, tz);
            if (!vec3d::IsInRangeInc(c+t_, VFrom, VTo)) continue;
            t_ += t;
            const uint32_t m_id = smatd::GenerateId((uint8_t)i, t_);
            if (!matrices.HasKey(m_id)) {
              smatd *m_ = new smatd(m);
              m_->SetRawId(m_id);
              matrices.Add(m_id, m_)->t += t_;
            }
          }
        }
      }
    }
  }
  TPtrList<smatd> result(matrices.Count());
  for (size_t i=0; i < matrices.Count(); i++)
    result[i] = matrices.GetValue(i);
  if( result.IsEmpty() )
    result.Add(new smatd)->I().SetId(0);
  return result;
}
//..............................................................................
ConstPtrList<smatd> TLattice::GenerateMatrices(
  const vec3d& center_, double rad)
{
  const TAsymmUnit& au = GetAsymmUnit();
  const TUnitCell &uc = GetUnitCell();
  const double qrad = rad*rad;
  olxdict<uint32_t, smatd*, TPrimitiveComparator> matrices;
  const vec3d center = au.Fractionalise(center_);
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    const smatd& m = uc.GetMatrix(i);
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& ca = au.GetAtom(j);
      if( !ca.IsAvailable() )  continue;
      vec3d c = m*ca.ccrd();
      vec3i t = -c.Floor<int>();
      c += t;
      c -= center;
      for (int tx=-4; tx <= 4; tx++) {
        for (int ty=-4; ty <= 4; ty++) {
          for (int tz=-4; tz <= 4; tz++) {
            vec3i t_(tx, ty, tz);
            if (au.Orthogonalise(c+t_).QLength() > qrad) continue;
            t_ += t;
            const uint32_t m_id = smatd::GenerateId((uint8_t)i, t_);
            if (!matrices.HasKey(m_id)) {
              smatd *m_ = new smatd(m);
              m_->SetRawId(m_id);
              matrices.Add(m_id, m_)->t += t_;
            }
          }
        }
      }
    }
  }
  TPtrList<smatd> result(matrices.Count());
  for (size_t i=0; i < matrices.Count(); i++)
    result[i] = matrices.GetValue(i);
  if( result.IsEmpty() )
    result.Add(new smatd)->I().SetId(0);
  return result;
}
//..............................................................................
void TLattice::GenerateBondsAndFragments(TArrayList<vec3d> *ocrd)  {
  volatile TStopWatch sw(__FUNC__);
  // treat detached and the rest of atoms separately
  size_t dac = 0;
  const size_t ac = Objects.atoms.Count();
  if( ocrd != NULL )  {
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& sa = Objects.atoms[i];
      (*ocrd)[i] = sa.crd();
      sa.crd() = GetAsymmUnit().Orthogonalise(sa.ccrd());
      if( !sa.CAtom().IsAvailable() )
        dac++;
    }
  }
  else  {
    for( size_t i=0; i < ac; i++ )  {
      if( !Objects.atoms[i].CAtom().IsAvailable() )
        dac++;
    }
  }
  TSAtomPList atoms(ac-dac);
  dac = 0;
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    sa.ClearNodes();
    sa.ClearBonds();
    if( !sa.CAtom().IsAvailable() )  {
      dac++;
      sa.SetNetwork(*Network);
    }
    else
      atoms[i-dac] = &sa;
  }
  BuildAtomRegistry();
  Network->Disassemble(Objects, Fragments);
  dac = 0;
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.IsDeleted() )
      dac++;
    else  {
      if( ocrd != NULL )
        sa.crd() = (*ocrd)[i];
    }
  }
  if( dac != 0 )  {
    OnAtomsDeleted.Enter(this);
    for( size_t i=0; i < ac; i++ )  {
      if( Objects.atoms[i].IsDeleted() )
        Objects.atoms.Null(i);
    }
    Objects.atoms.Pack();
    OnAtomsDeleted.Exit(this);
  }
  for (size_t i=0; i < GetAsymmUnit().AtomCount(); i++) {
    GetAsymmUnit().GetAtom(i).SetFragmentId(-1);
  }
  BubbleSorter::SortSF(Fragments, CompareFragmentsBySize);
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    for( size_t j=0; j < Fragments[i]->NodeCount(); j++ )
      Fragments[i]->Node(j).CAtom().SetFragmentId((uint32_t)i);
  }
}
//..............................................................................
void TLattice::BuildPlanes()  {
  ClearPlanes();
  for( size_t i=0; i < PlaneDefs.Count(); i++ )  {
    TSPlane::Def& pd = PlaneDefs[i];
    for( size_t j=0; j < Matrices.Count(); j++ )  {
      TSPlane* p = pd.FromAtomRegistry(Objects, i, Network, *Matrices[j]);
      if( p != NULL ) {
        bool uniq = true;
        for( size_t k=0; k < Objects.planes.Count()-1; k++ )  {
          if( Objects.planes[k].GetCenter().QDistanceTo(p->GetCenter()) < 1e-6 )  {
            uniq = false;
            break;
          }
        }
        if( !uniq )
          Objects.planes.DeleteLast();
      }
    }
  }
}
//..............................................................................
void TLattice::InitBody()  {
  volatile TStopWatch sw(__FUNC__);
  OnDisassemble.Enter(this);
  if( !ApplyGrowInfo() )  {
    // create identity matrix
    Matrices.Add(new smatd(GetUnitCell().GetMatrix(0)))->SetId(0);
    ClearPlanes();
    Objects.atoms.IncCapacity(GetAsymmUnit().AtomCount());
    for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
      TCAtom& CA = GetAsymmUnit().GetAtom(i);
      if( CA.IsDeleted() )  continue;
      GenerateAtom(CA, *Matrices[0]);
    }
  }
  GenerateBondsAndFragments(NULL);
  BuildPlanes();
  OnDisassemble.Exit(this);
}
//..............................................................................
void TLattice::Init()  {
  volatile TStopWatch sw(__FUNC__);
  Clear(false);
  GetUnitCell().ClearEllipsoids();
  GetUnitCell().InitMatrices();
  GetAsymmUnit().GetRefMod()->UpdateUsedSymm(GetUnitCell());
  try {
    GetUnitCell().FindSymmEq();
    InitBody();
  }
  catch (const TExceptionBase &e) {
    Clear(true);
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
}
//..............................................................................
void TLattice::Uniq()  {
  OnStructureUniq.Enter(this);
  Clear(false);
  ClearMatrices();
  GetUnitCell().UpdateEllipsoids();  // if new atoms are created...
  GetUnitCell().FindSymmEq();
  InitBody();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::GenerateWholeContent(TCAtomPList* Template)  {
  OnStructureGrow.Enter(this);
  Generate(Template, false);
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(TCAtomPList* Template, bool ClearCont)  {
  if( ClearCont && Template != NULL ) 
    ClearAtoms();
  else  {
    const size_t ac = Objects.atoms.Count();
    size_t da = 0;
    for( size_t i=0; i < ac; i++ )  {  // restore atom coordinates
      TSAtom& sa = Objects.atoms[i];
      if( sa.IsDeleted() )
        da++;
      else
        sa.crd() = GetAsymmUnit().Orthogonalise(sa.ccrd());
    }
    if( da != 0 )  {
      const size_t ac = Objects.atoms.Count();
      OnAtomsDeleted.Enter(this);
      for( size_t i=0; i < ac; i++ )  {  // restore atom coordinates
        if( Objects.atoms[i].IsDeleted() )
          Objects.atoms.Null(i);
      }
      Objects.atoms.Pack();
      OnAtomsDeleted.Exit(this);
    }
  }
  const TCAtomPList &al = (Template != NULL && !Template->IsEmpty())
    ? *Template : GetAsymmUnit().GetAtoms();
  Objects.atoms.IncCapacity(Matrices.Count()*al.Count());
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    for( size_t j=0; j < al.Count(); j++ )  {
      if( !al[j]->IsAvailable() )  continue;
      GenerateAtom(*al[j], *Matrices[i]);
    }
  }
  Disassemble();
}
//..............................................................................
void TLattice::GenerateCell()  {
  ClearAtoms();
  ClearMatrices();
  OnStructureGrow.Enter(this);
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  olxdict<uint32_t, smatd*, TPrimitiveComparator> matrices;
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    const smatd& m = uc.GetMatrix(i);
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& ca = au.GetAtom(j);
      if( !ca.IsAvailable() )  continue;
      TSAtom& sa = Objects.atoms.New(Network);
      sa.CAtom(ca);
      sa.ccrd() = m*ca.ccrd();
      const vec3i t = -sa.ccrd().Floor<int>();
      sa.ccrd() += t;
      const uint32_t m_id = smatd::GenerateId((uint8_t)i, t);
      smatd* lm = matrices.Find(m_id, NULL);
      if( lm == NULL )  {
        lm = matrices.Add(m_id, new smatd(m));
        lm->t += t;
        lm->SetRawId(m_id);
      }
      sa.crd() = au.Orthogonalise(sa.ccrd());
      sa.SetEllipsoid(&GetUnitCell().GetEllipsoid(m.GetContainerId(),
        ca.GetId()));
      sa._SetMatrix(lm);
    }
  }
  Matrices.SetCapacity(matrices.Count());
  for (size_t i=0; i < matrices.Count(); i++)
    Matrices.Add(matrices.GetValue(i));
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GenerateBox(const vec3d_alist& norms,
  const vec3d_alist& centres, bool clear_content)
{
  if (norms.Count() !=6 || norms.Count() != centres.Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "volume definition");
  }
  OnStructureGrow.Enter(this);
  if( clear_content )  {
    ClearAtoms();
    ClearMatrices();
  }
  const TUnitCell& uc = GetUnitCell();
  TAsymmUnit& au = GetAsymmUnit();
  olxdict<uint32_t, smatd*, TPrimitiveComparator> matrices;
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    const smatd& m = uc.GetMatrix(i);
    for( int di = -3; di <= 3; di++ )  {
      for( int dj = -3; dj <= 3; dj++ )  {
        for( int dk = -3; dk <= 3; dk++ )  {
          const vec3d t(di, dj, dk);
          const uint32_t m_id = smatd::GenerateId((uint8_t)i, t);
          smatd* lm = matrices.Find(m_id, NULL);
          bool matrix_created = false;
          if( lm == NULL )  {
            lm = new smatd(m);
            lm->t += t;
            lm->SetRawId(m_id);
            matrix_created = true;
          }
          for( size_t j=0; j < au.AtomCount(); j++ )  {
            TCAtom& ca = au.GetAtom(j);
            if( ca.IsDeleted() )  continue;
            vec3d p = m*ca.ccrd() + t;
            const vec3d c = au.CellToCartesian(p);
            bool inside = true;
            for (int fi=0; fi < 6; fi++) {
              if ((c-centres[fi]).DotProd(norms[fi]) > 0) {
                inside = false;
                break;
              }
            }
            if (!inside) continue;
            GenerateAtom(ca, *lm);
            if( matrix_created )  {
              matrices.Add(m_id, lm);
              matrix_created = false;
            }
          }
          if( matrix_created )
            delete lm;
        }
      }
    }
  }
  Matrices.SetCapacity(matrices.Count());
  for (size_t i=0; i < matrices.Count(); i++)
    Matrices.Add(matrices.GetValue(i));
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& MFrom, const vec3d& MTo,
  TCAtomPList* Template, bool ClearCont)
{
  OnStructureGrow.Enter(this);
  if( ClearCont )  {
    ClearAtoms();
    ClearMatrices();
  }
  Matrices.AddList(GenerateMatrices(MFrom, MTo));
  Generate(Template, ClearCont);
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::Generate(const vec3d& center, double rad, TCAtomPList* Template,
  bool ClearCont)
{
  OnStructureGrow.Enter(this);
  if( ClearCont )  {
    ClearAtoms();
    ClearMatrices();
  }
  Matrices.AddList(GenerateMatrices(center, rad));
  Generate(Template, ClearCont);
  OnStructureGrow.Exit(this);
}
//..............................................................................
SortedObjectList<smatd, smatd::ContainerIdComparator>
  TLattice::GetFragmentGrowMatrices(const TCAtomPList& l, bool use_q_peaks) const
{
  SortedObjectList<smatd, smatd::ContainerIdComparator> res;
  const TUnitCell& uc = GetUnitCell();
  res.Add(uc.GetMatrix(0));
  TQueue<const smatd*> q;
  q.Push(&res[0]);
  while (!q.IsEmpty()) {
    const smatd &ref_m = *q.Pop();
    for( size_t j=0; j < l.Count(); j++ )  {
      TCAtom& a = *l[j];
      for( size_t k=0; k < a.AttachedSiteCount(); k++ )  {
        TCAtom &aa = a.GetAttachedAtom(k);
        if (aa.IsDeleted() || (!use_q_peaks && aa.GetType() == iQPeakZ))
          continue;
        smatd m = uc.MulMatrix(a.GetAttachedSite(k).matrix, ref_m);
        size_t idx;
        if( res.AddUnique(m, &idx) )
          q.Push(&res[idx]);
      }
    }
  }
  return res;
}
//..............................................................................
void TLattice::GetGrowMatrices(smatd_list& res) const {
  const TUnitCell& uc = GetUnitCell();
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.IsGrown() || !sa.IsAvailable() || !sa.CAtom().IsAvailable() )  continue;
    const TCAtom& ca = sa.CAtom();
    for( size_t j=0; j < ca.AttachedSiteCount(); j++ )  {
      const TCAtom::Site& site = ca.GetAttachedSite(j);
      if( !site.atom->IsAvailable() )  continue;
      const smatd m = uc.MulMatrix(site.matrix, sa.GetMatrix());
      bool found = false;
      for( size_t l=0; l < MatrixCount(); l++ )  {
        if( Matrices[l]->GetId() == m.GetId() )  {
          found = true;  
          break;
        }
      }
      if( !found && res.IndexOf(m) == InvalidIndex )
        res.AddCopy(m);
    }
  }
}
//..............................................................................
void TLattice::DoGrow(const TSAtomPList& atoms, bool GrowShell, TCAtomPList* Template)  {
  RestoreCoordinates();
  const TUnitCell& uc = GetUnitCell();
  SortedPtrList<smatd, smatd::IdComparator> matrices;
  matrices.SetCapacity(Matrices.Count());
  for (size_t i=0; i < Matrices.Count(); i++)
    matrices.AddUnique(Matrices[i]);
  OnStructureGrow.Enter(this);
  if( GrowShell )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms[i];
      const TCAtom& CA = SA->CAtom();
      for( size_t j=0; j < CA.AttachedSiteCount(); j++ )  {
        const TCAtom::Site& site = CA.GetAttachedSite(j);
        if( !site.atom->IsAvailable() )  continue;
        const smatd m = uc.MulMatrix(site.matrix, atoms[i]->GetMatrix());
        if (Objects.atomRegistry.Find(
          TSAtom::Ref(site.atom->GetId(), m.GetId())) != NULL)
          continue;
        size_t mi = matrices.IndexOf(&m);
        smatd *mp;
        if (mi == InvalidIndex)
          matrices.Add(mp = Matrices.Add(new smatd(m)));
        else
          mp = matrices[mi];
        GenerateAtom(*site.atom, *mp);
      }
    }
  }
  else  {
  // the fragmens to grow by a particular matrix
    olxdict<smatd*, TIntList, TPointerComparator> Fragments2Grow;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms[i];
      const TCAtom& CA = SA->CAtom();
      for( size_t j=0; j < CA.AttachedSiteCount(); j++ )  {
        const TCAtom::Site& site = CA.GetAttachedSite(j);
        if( !site.atom->IsAvailable() )  continue;
        const smatd m = uc.MulMatrix(site.matrix, atoms[i]->GetMatrix());
        size_t mi = matrices.IndexOf(&m);
        smatd *mp;
        if (mi == InvalidIndex)
          matrices.Add(mp = Matrices.Add(new smatd(m)));
        else
          mp = matrices[mi];
        Fragments2Grow.Add(mp).Add(site.atom->GetFragmentId());
      }
    }
    for (size_t i=0; i < Fragments2Grow.Count(); i++) {
      TIntList& ToGrow = Fragments2Grow.GetValue(i);
      for( size_t j=0; j < GetAsymmUnit().AtomCount(); j++ )  {
        TCAtom& ca = GetAsymmUnit().GetAtom(j);
        if( ca.IsAvailable() && ToGrow.IndexOf(ca.GetFragmentId()) != InvalidIndex )
          GenerateAtom(ca, *const_cast<smatd *>(Fragments2Grow.GetKey(i)));
      }
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowFragments(bool GrowShells, TCAtomPList* Template)  {
  TSAtomPList TmpAtoms;
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& A = Objects.atoms[i];
    if( A.IsDeleted() || !A.CAtom().IsAvailable() )  
      continue;
    for( size_t j=0; j < A.NodeCount(); j++ )  {
      if( A.Node(j).IsDeleted() )
        A.NullNode(j);
    }
    A.PackNodes();
    if( !A.IsGrown() )
      TmpAtoms.Add(A);
  }
  if( !TmpAtoms.IsEmpty() )
    GrowAtoms(TmpAtoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtoms(const TSAtomPList& atoms, bool GrowShells,
  TCAtomPList* Template)
{
  if( atoms.IsEmpty() )  return;
  DoGrow(atoms, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowAtom(TSAtom& Atom, bool GrowShells, TCAtomPList* Template)  {
  if( Atom.IsGrown() )  return;
  DoGrow(TSAtomPList() << Atom, GrowShells, Template);
}
//..............................................................................
void TLattice::GrowFragment(uint32_t FragId, const smatd_list& transforms) {
  olxdict<uint32_t, smatd_list, TPrimitiveComparator> j;
  j.Add(FragId, transforms);
  GrowFragments(j);
}
//..............................................................................
void TLattice::GrowFragments(
  const olxdict<uint32_t, smatd_list, TPrimitiveComparator> &job)
{
  olxdict<uint32_t, smatd*, TPrimitiveComparator> matrix_map;
  // check if the matix is unique
  matrix_map.SetCapacity(Matrices.Count());
  for (size_t i=0; i < Matrices.Count(); i++)
    matrix_map.Add(Matrices[i]->GetId(), Matrices[i]);
  for (size_t i=0; i < job.Count(); i++) {
    const smatd_list &l = job.GetValue(i);
    for (size_t j=0; j < l.Count(); j++) {
      if (!matrix_map.HasKey(l[j].GetId()))
        matrix_map.Add(l[j].GetId(), Matrices.Add(new smatd(l[j])));
    }
  }
  OnStructureGrow.Enter(this);
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& ca = GetAsymmUnit().GetAtom(i);
    if (!ca.IsAvailable()) continue;
    size_t fi = job.IndexOf(ca.GetFragmentId());
    if (fi != InvalidIndex) {
      const smatd_list &l = job.GetValue(fi);
      for( size_t j=0; j < l.Count(); j++)
        GenerateAtom(ca, *matrix_map[l[j].GetId()]);
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::GrowAtoms(const TCAtomPList& atoms, const smatd_list& matrices)  {
  if( atoms.IsEmpty() )  return;
  smatd_plist addedMatrices;
  // check if the matices is unique
  for( size_t i=0; i < matrices.Count(); i++ )  {
    bool found = false;
    for( size_t j=0; j < Matrices.Count(); j++ )  {
      if( Matrices[j]->GetId() == matrices[i].GetId() )  {
        found = true;
        addedMatrices.Add(Matrices[j]);
        break;
      }
    }
    if( !found )
      addedMatrices.Add(Matrices.Add(new smatd(matrices[i])));
  }
  if( addedMatrices.IsEmpty() )  return;
  OnStructureGrow.Enter(this);
  Objects.atoms.IncCapacity(atoms.Count()*addedMatrices.Count());
  for( size_t i=0; i < addedMatrices.Count(); i++ )  {
    for( size_t j=0; j < atoms.Count(); j++ )  {
      if( atoms[j]->IsAvailable() )
        GenerateAtom(*atoms[j], *addedMatrices[i]);
    }
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
TSAtom *TLattice::GrowAtom(TCAtom& atom, const smatd& matrix)  {
  // check if unique
  TSAtom *a = GetAtomRegistry().Find(TSAtom::Ref(atom.GetId(), matrix.GetId()));
  if( a != NULL && !a->IsDeleted() )  {
    return a;
  }
  smatd* m = NULL;
  bool found = false;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i]->GetId() == matrix.GetId() )  {
      m = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )
    m = Matrices.Add(new smatd(matrix));
  OnStructureGrow.Enter(this);
  a = &GenerateAtom(atom, *m);
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
  return a;
}
//..............................................................................
TSAtom& TLattice::GenerateAtom(TCAtom& a, smatd& symop, TNetwork* net)  {
  TSAtom& SA = Objects.atoms.New(net == NULL ? Network : net);
  SA.CAtom(a);
  SA._SetMatrix(&symop);
  SA.crd() = GetAsymmUnit().Orthogonalise(SA.ccrd() = symop * SA.ccrd());
  SA.SetEllipsoid(&GetUnitCell().GetEllipsoid(
    symop.GetContainerId(), SA.CAtom().GetId()));
  return SA;
}
//..............................................................................
void TLattice::Grow(const smatd& transform)  {
  smatd *M = NULL;
  // check if the matix is unique
  bool found = false;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i]->GetId() == transform.GetId() )  {
      M = Matrices[i];
      found = true;
      break;
    }
  }
  if( !found )
    M = Matrices.Add(new smatd(transform));
  OnStructureGrow.Enter(this);
  TAsymmUnit& au = GetAsymmUnit();
  const size_t ac = au.AtomCount();
  Objects.atoms.IncCapacity(ac);
  for( size_t i=0; i < ac; i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsAvailable() )
      GenerateAtom(ca, *M);
  }
  RestoreCoordinates();
  Disassemble();
  OnStructureGrow.Exit(this);
}
//..............................................................................
void TLattice::RestoreAtom(const TSAtom::Ref& id)  {
  if( smatd::GetContainerId(id.matrix_id) >= GetUnitCell().MatrixCount() )
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix ID");
  if( id.catom_id >= GetAsymmUnit().AtomCount() )
    throw TInvalidArgumentException(__OlxSourceInfo, "catom ID");
  smatd* matr = NULL;
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i]->GetId() == id.matrix_id )  {
      matr = Matrices[i];
      break;
    }
  }
  if( matr == NULL )  {
    matr = Matrices.Add(new smatd(smatd::FromId(id.matrix_id,
      GetUnitCell().GetMatrix(smatd::GetContainerId(id.matrix_id)))));
  }
  TSAtom& sa = GenerateAtom(GetAsymmUnit().GetAtom(id.catom_id), *matr);
  sa.CAtom().SetDeleted(false);
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const olxstr& Label) const {
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )
    if( Label.Equalsi(Objects.atoms[i].GetLabel()) )  
      return &Objects.atoms[i];
  return NULL;
}
//..............................................................................
TSAtom* TLattice::FindSAtom(const TCAtom& ca) const {
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )
    if( ca.GetId() == Objects.atoms[i].CAtom().GetId() )  
      return &Objects.atoms[i];
  return NULL;
}
//..............................................................................
TSAtomPList TLattice::NewCentroid(const TSAtomPList& Atoms)  {
  TSAtomPList rv;
  if( Atoms.IsEmpty() )  return rv;
  vec3d cc, ce;
  const smatd itm = UnitCell->InvMatrix(Atoms[0]->GetMatrix());
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    cc += itm*Atoms[i]->ccrd();
    ce += vec3d::Qrt(Atoms[i]->CAtom().ccrdEsd());
  }
  ce.Sqrt();
  ce /= Atoms.Count();
  cc /= Atoms.Count();
  try  {
    TCAtom& CCent = AsymmUnit->NewCentroid(cc);
    GetUnitCell().AddEllipsoid();
    rv.SetCapacity(Matrices.Count());
    for( size_t i=0; i < Matrices.Count(); i++ )  {
      TSAtom& c = GenerateAtom(CCent, *Matrices[i]);
      CCent.ccrdEsd() = ce;
      GetUnitCell().AddEllipsoid();
      rv << c;
    }
    return rv;
  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//..............................................................................
TSAtom& TLattice::NewAtom(const vec3d& center)  {
  TCAtom& ca = AsymmUnit->NewAtom();
  ca.ccrd() = center;
  GetUnitCell().AddEllipsoid();
  return GenerateAtom(ca, *Matrices[0]);
}
//..............................................................................
TSPlanePList TLattice::NewPlane(const TSAtomPList& Atoms, double weightExtent, bool regular)  {
  TSPlane* Plane = TmpPlane(Atoms, weightExtent);
  TSPlanePList rv;
  if( Plane != NULL)  {
    Plane->SetRegular(regular);
    TSPlane::Def pd = Plane->GetDef();
    bool found = false;
    for( size_t i=0; i < PlaneDefs.Count(); i++ )  {
      if( PlaneDefs[i] == pd )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      PlaneDefs.AddCopy(pd);
      if( IsGenerated() )  {
        delete Plane;
        for( size_t i=0; i < Matrices.Count(); i++ )  {
          TSPlane* p = pd.FromAtomRegistry(Objects, PlaneDefs.Count()-1,
            Network, *Matrices[i]);
          if( p != NULL )  {
            bool uniq = true;
            for( size_t j=0; j < Objects.planes.Count()-1; j++ )  {
              if (!Objects.planes[j].IsDeleted() &&
                   Objects.planes[j].GetCenter().QDistanceTo(
                     p->GetCenter()) < 1e-6 &&
                     Objects.planes[j].GetNormal().IsParallel(p->GetNormal()))
              {
                rv << Objects.planes[j];
                uniq = false;
                break;
              }
            }
            if( !uniq )
              Objects.planes.DeleteLast();
            else
              rv.Add(p);
          }
        }
      }
      else  {
        Objects.planes.Attach(*Plane);
        rv.Add(Plane);
        Plane->_SetDefId(PlaneDefs.Count()-1);
      }
    }
    else {
      bool uniq=true;
      for (size_t i=0; i < Objects.planes.Count(); i++) {
        if (!Objects.planes[i].IsDeleted() &&
             Objects.planes[i].GetCenter().QDistanceTo(
               Plane->GetCenter()) < 1e-6 &&
               Objects.planes[i].GetNormal().IsParallel(Plane->GetNormal()))
        {
          rv << Objects.planes[i];
          uniq = false;
          break;
        }
      }
      if (!uniq)
        delete Plane;
      else
        rv << Plane;
    }
  }
  return rv;
}
//..............................................................................
TSPlane* TLattice::TmpPlane(const TSAtomPList& atoms, double weightExtent)  {
  if( atoms.Count() < 3 )  return NULL;
  //TODO: need to consider occupancy for disordered groups ...
  TTypeList<AnAssociation2<TSAtom*, double> > Points;
  Points.SetCapacity(atoms.Count());
  if( weightExtent != 0 )  {
    double swg = 0;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      const double wght = pow(atoms[i]->GetType().z, weightExtent);
      Points.AddNew(atoms[i], wght);
      swg += wght;
    }
    // normalise the sum of weights to atoms.Count()...
    const double m = atoms.Count()/swg;
    for( size_t i=0; i < Points.Count(); i++ )
      Points[i].B() *= m;
  }
  else  {
    for( size_t i=0; i < atoms.Count(); i++ )
      Points.AddNew(atoms[i], 1);
  }

  Objects.planes.New(Network).Init(Points);
  return &Objects.planes.Detach(Objects.planes.Count()-1);
}
//..............................................................................
void TLattice::UpdatePlaneDefinitions()  {
  PlaneDefs.ForEach(ACollectionItem::TagSetter(0));
  for( size_t i=0; i < Objects.planes.Count(); i++ )  {
    TSPlane& sp = Objects.planes[i];
    if( sp.IsDeleted() || sp.GetDefId() >= PlaneDefs.Count() )  // would be odd
      continue;
    PlaneDefs[sp.GetDefId()].IncTag();
  }
  TSizeList ids(PlaneDefs.Count());
  size_t id=0;
  for( size_t i=0; i < PlaneDefs.Count(); i++ )  {
    if( PlaneDefs[i].GetTag() != 0 )
      ids[i] = id++;
    else
      PlaneDefs.NullItem(i);
  }
  for( size_t i=0; i < Objects.planes.Count(); i++ )  {
    TSPlane& sp = Objects.planes[i];
    if( sp.IsDeleted() || sp.GetDefId() >= PlaneDefs.Count() )  // would be odd
      continue;
    sp._SetDefId(ids[sp.GetDefId()]);
  }
  PlaneDefs.Pack();
}
//..............................................................................
void TLattice::UpdateAsymmUnit()  {
  if( Objects.atoms.IsEmpty() )  return;
  const size_t ac = GetAsymmUnit().AtomCount();
  TArrayList<TSAtomPList> AUAtoms(ac);
  TSizeList del_cnt(ac);
  for( size_t i=0; i < ac; i++ )
    del_cnt[i] = 0;
  const size_t lat_ac = Objects.atoms.Count();
  for( size_t i=0; i < lat_ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.IsDeleted() )  {
      del_cnt[sa.CAtom().GetId()]++;
      continue;
    }
    AUAtoms[sa.CAtom().GetId()].Add(sa);
  }
  for( size_t i=0; i < ac; i++ )  {  // create lists to store atom groups
    TSAtomPList& l = AUAtoms[i];
    if( del_cnt[i] == 0 && (l.Count() > 1) )  continue;  // nothing to do
    TCAtom& ca = AsymmUnit->GetAtom(i);
    if( l.IsEmpty() )  {  // all atoms are deleted or none generated
      if( !ca.IsDeleted() && ca.IsAvailable() )
        ca.SetDeleted(del_cnt[i] != 0);
      continue;
    }
    else if( l.Count() == 1 )  {  // special case...
      if( l[0]->IsAUAtom() )  continue;
      if( l[0]->GetEllipsoid() )
        ca.UpdateEllp(*l[0]->GetEllipsoid());
      ca.ccrd() = l[0]->ccrd();
      continue;
    }
    // find the original atom, or symmetry equivalent if removed
    // !2011.07.01
    // this bit bites for grown structures - the basis must be changed for all
    // atoms and symetry operators
    //TSAtom* OA = NULL;
    //const size_t lst_c = l.Count();
    //for( size_t j=0; j < lst_c; j++ )  {
    //  TSAtom* A = l[j];
    //  const size_t am_c = A->MatrixCount();
    //  for( size_t k=0; k < am_c; k++ )  {
    //    const smatd& m = A->GetMatrix(k);
    //    if( m.IsFirst() )  {  // the original atom
    //      OA = A;  
    //      break; 
    //    }
    //  }
    //  if( OA != NULL )  break;
    //}
    //if( OA == NULL )
    //  OA = l[0];
    //ca.SetDeleted(false);
    //if( OA->GetEllipsoid() )
    //  ca.UpdateEllp(*OA->GetEllipsoid());
    //ca.ccrd() = OA->ccrd();
  }
}
//..............................................................................
void TLattice::MoveFragment(const vec3d& to, TSAtom& fragAtom)  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  smatd* m = GetUnitCell().GetClosest(to, fragAtom.ccrd(), true);
  if( m != NULL )  {
    for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
      if( SA.CAtom().GetEllipsoid() != NULL ) {
        *SA.CAtom().GetEllipsoid() =
          GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
      }
    }
    delete m;
    GetUnitCell().UpdateEllipsoids();
    Uniq();
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
}
//..............................................................................
void TLattice::MoveFragment(TSAtom& to, TSAtom& fragAtom)  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  smatd* m = GetUnitCell().GetClosest(to.CAtom(), fragAtom.CAtom(), true);
  if( m != NULL )  {
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )  {
      fragAtom.CAtom().ccrd() = *m * fragAtom.CAtom().ccrd();
      if( fragAtom.CAtom().GetEllipsoid() != NULL ) {
        *fragAtom.CAtom().GetEllipsoid() =
          GetUnitCell().GetEllipsoid(m->GetContainerId(), fragAtom.CAtom().GetId());
      }
    }
    else  {  // move whole fragment then
      uint32_t fragId = fragAtom.CAtom().GetFragmentId();
      for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
        TSAtom& sa = Objects.atoms[i];
        if( sa.CAtom().GetFragmentId() == fragId )  {
          sa.CAtom().ccrd() = *m * sa.CAtom().ccrd();
          if( sa.CAtom().GetEllipsoid() != NULL ) {
            *sa.CAtom().GetEllipsoid() =
              GetUnitCell().GetEllipsoid(m->GetContainerId(), sa.CAtom().GetId());
          }
        }
      }
    }
    delete m;
    GetUnitCell().UpdateEllipsoids();
    Uniq();
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
}
//..............................................................................
void TLattice::MoveFragmentG(const vec3d& to, TSAtom& fragAtom)  {
  vec3d from;
  from = fragAtom.ccrd();
  smatd* m = GetUnitCell().GetClosest(to, from, true);
  vec3d offset;
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )  {
      TSAtom& SA = fragAtom.GetNetwork().Node(i);
      if( SA.IsDeleted() )  continue;
      GenerateAtom(SA.CAtom(), *m, &SA.GetNetwork());
    }
    Disassemble();
    OnStructureGrow.Exit(this);
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
}
//..............................................................................
void TLattice::MoveFragmentG(TSAtom& to, TSAtom& fragAtom)  {
  smatd* m = GetUnitCell().GetClosest(to.ccrd(), fragAtom.ccrd(), true);
  if( m != NULL )  {
/* restore atom centres if were changed by some other procedure */
    RestoreCoordinates();
    OnStructureGrow.Enter(this);
    Matrices.Add(m);
    TSAtomPList atoms;
    if( to.CAtom().GetFragmentId() == fragAtom.CAtom().GetFragmentId() )
      atoms.Add(&fragAtom);
    else  // copy whole fragment then
      for( size_t i=0; i < fragAtom.GetNetwork().NodeCount(); i++ )
        atoms.Add(&fragAtom.GetNetwork().Node(i));

    for( size_t i=0; i < atoms.Count(); i++ )  {
      TSAtom* SA = atoms.GetItem(i);
      if( SA->IsDeleted() )  continue;
      GenerateAtom(SA->CAtom(), *m);
    }
    Disassemble();
    OnStructureGrow.Exit(this);
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not find closest matrix";
}
//..............................................................................
void TLattice::MoveToCenter()  {
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  vec3d cnt(0.5),
    ocnt = GetAsymmUnit().Orthogonalise(cnt);
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    TNetwork* frag = Fragments[i];
    vec3d molCenter;
    size_t ac = 0;
    for( size_t j=0; j < frag->NodeCount(); j++ )  {
      if( frag->Node(j).IsDeleted() )  continue;
      molCenter += frag->Node(j).ccrd();
      ac++;
    }
    if( ac == 0 )  continue;
    molCenter /= ac;
    smatd* m = GetUnitCell().GetClosest(cnt, molCenter, true);
    if( m == NULL )  continue;
    double d1 = GetAsymmUnit().Orthogonalise(molCenter).DistanceTo(ocnt);
    double d2 = GetAsymmUnit().Orthogonalise(*m*molCenter).DistanceTo(ocnt);
    if (olx_abs(d1-d2) < 1e-6) {
      delete m;
      continue;
    }
    for( size_t j=0; j < frag->NodeCount(); j++ )  {
      TSAtom& SA = frag->Node(j);
      SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
      if( SA.CAtom().GetEllipsoid() != NULL )  {
        *SA.CAtom().GetEllipsoid() =
          GetUnitCell().GetEllipsoid(m->GetContainerId(), SA.CAtom().GetId());
      }
    }
    delete m;
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::Compaq()  {
  if( IsGenerated() || Fragments.Count() < 2 )  return;
  TNetwork* frag = Fragments[0];
  vec3d acenter;
  size_t ac = 0;
  for( size_t i=0; i < frag->NodeCount(); i++ )  {
    if( frag->Node(i).IsDeleted() )  continue;
    acenter += frag->Node(i).ccrd();
    ac++;
  }
  if( ac == 0 )  return;
  acenter /= ac;
  for( size_t i=1; i < Fragments.Count(); i++ )  {
    frag = Fragments[i];
    smatd* m = NULL;

    for( size_t j=0; j < Fragments[0]->NodeCount(); j++ )  {
      TSAtom& fa = Fragments[0]->Node(j);
      for( size_t k=0; k < frag->NodeCount(); k++ )  {
        if( frag->Node(k).CAtom().IsAttachedTo(fa.CAtom()) )  {
          m = GetUnitCell().GetClosest(fa.ccrd(), frag->Node(k).ccrd(), true);
          if( m != NULL )  break;
        }
      }
      if( m != NULL )  break;
    }
    if( m == NULL )  {
      vec3d molCenter;
      ac = 0;
      for( size_t j=0; j < frag->NodeCount(); j++ )  {
        if( frag->Node(j).IsDeleted() )  continue;
        molCenter += frag->Node(j).ccrd();
        ac++;
      }
      if( ac == 0 )  continue;
      molCenter /= ac;
      m = GetUnitCell().GetClosest(acenter, molCenter, true);
    }
    if( m != NULL )  {
      for( size_t j=0; j < frag->NodeCount(); j++ )  {
        TSAtom& SA = frag->Node(j);
        if( SA.IsDeleted() )  continue;
        SA.CAtom().ccrd() = *m * SA.CAtom().ccrd();
        if( SA.CAtom().GetEllipsoid() != NULL ) {
          *SA.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(
            m->GetContainerId(), SA.CAtom().GetId());
        }
      }
      delete m;
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
int TLattice_CompaqAll_SiteCmp(const TCAtom::Site &s1,
  const TCAtom::Site &s2)
{
  return olx_cmp(s2.atom->GetQPeak(), s1.atom->GetQPeak());
}
size_t TLattice_CompaqAll_Process(TUnitCell& uc, TCAtom& ca,
  const smatd& matr, bool q_peaks)
{
  if (!q_peaks && ca.GetType() == iQPeakZ) return 0;
  size_t cnt = 0;
  ca.SetTag(1);
  TPtrList<TCAtom::Site> sites;
  for( size_t i=0; i < ca.AttachedSiteCount(); i++ )  {
    TCAtom::Site& site = ca.GetAttachedSite(i);
    if( site.atom->GetTag() != 0 )  continue;
    if( !matr.IsFirst() )  {
      cnt++;
      site.matrix = uc.MulMatrix(site.matrix, matr);
    }
    else if( site.atom->GetFragmentId() == ca.GetFragmentId() &&
      !site.matrix.IsFirst() && ca.GetType() != iQPeakZ )
    {
      continue;
    }
    site.atom->SetTag(1);
    site.atom->SetFragmentId(ca.GetFragmentId());
    site.atom->ccrd() = site.matrix*site.atom->ccrd();
    if( site.atom->GetEllipsoid() != NULL )  {
      *site.atom->GetEllipsoid() = uc.GetEllipsoid(
        site.matrix.GetContainerId(), site.atom->GetId());
    }
    if( site.atom->IsAvailable() )
      sites << site;
  }
  QuickSorter::SortSF(sites, &TLattice_CompaqAll_SiteCmp);
  for (size_t i=0; i < sites.Count(); i++) {
    cnt += TLattice_CompaqAll_Process(uc,
      *sites[i]->atom, sites[i]->matrix, q_peaks);
  }
  return cnt;
}
void TLattice_CompaqAll_ProcessRest(TUnitCell& uc, TCAtom& ca)
{
  TPtrList<TCAtom::Site> sites;
  for( size_t i=0; i < ca.AttachedSiteCount(); i++ )  {
    TCAtom::Site& site = ca.GetAttachedSite(i);
    if( site.atom->GetTag() != 1 )  continue;
    double d=0;
    smatd *m = uc.GetClosest(*site.atom, ca, true, &d);
    if (m == NULL) return;
    ca.ccrd() = *m*ca.ccrd();
    if( ca.GetEllipsoid() != NULL )  {
      *ca.GetEllipsoid() = uc.GetEllipsoid(m->GetContainerId(), ca.GetId());
    }
    delete m;
    return;
  }
}
void TLattice::CompaqAll()  {
  if (IsGenerated())  return;
  TUnitCell& uc = GetUnitCell();
  using namespace olx_analysis;
  TCAtomPList sqp;
  sqp.SetCapacity(Objects.atoms.Count());
  size_t ac=0, cnt=0;
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    TCAtom& ca = Objects.atoms[i].CAtom();
    if(ca.GetType() == iQPeakZ)
      sqp.Add(ca);
    else
      ac++;
    ca.SetTag(0);
  }
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.CAtom().GetTag() == 0 || sa.CAtom().IsAvailable() )
      cnt += TLattice_CompaqAll_Process(uc, sa.CAtom(), uc.GetMatrix(0), false);
  }
  if (!sqp.IsEmpty()) {
    TTypeList<peaks::range> peak_ranges = peaks::analyse(sqp);
    for (size_t i=peak_ranges.Count()-1; i != 0; i--)
      ac += peak_ranges[i].peaks.Count();
    bool z_processed=true;
    if (uc.CalcVolume()/(ac*uc.MatrixCount()) > 14) {
      for (size_t i=0; i < peak_ranges[0].peaks.Count(); i++) {
        if (peak_ranges[0].peaks[i]->GetTag() == 0)
          peak_ranges[0].peaks[i]->SetTag(2);
      }
      z_processed = false;
    }
    for (size_t i=0; i < sqp.Count(); i++) {
      if (sqp[i]->GetTag() == 0 && sqp[i]->IsAvailable())
        TLattice_CompaqAll_Process(uc, *sqp[i], uc.GetMatrix(0), true);
    }
    if (!z_processed) {
      for (size_t i=peak_ranges[0].peaks.Count()-1; i !=InvalidIndex; i--) {
        TCAtom &p = *peak_ranges[0].peaks[i];
        if (p.IsAvailable() && p.GetTag() == 2)
          TLattice_CompaqAll_ProcessRest(uc, p);
      }
    }
  }
  OnStructureUniq.Enter(this);
  TActionQueueLock __queuelock(&OnStructureUniq);
  Init();
  if( cnt != 0 )
    MoveToCenter();
  __queuelock.Unlock();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::CompaqClosest()  {
  if( IsGenerated() || Fragments.Count() < 2 )  return;
  const size_t fr_cnt = Fragments.Count();
  TDoubleList vminQD(fr_cnt);
  for( size_t i = 0; i < fr_cnt; i++ )
    vminQD[i] = 100000;
  for( size_t fi = 0; fi < fr_cnt; fi++ )  {
    TNetwork* neta = Fragments[fi];
    const size_t neta_cnt = neta->NodeCount();
    for( size_t i=fi+1; i < fr_cnt; i++ )  {
      //for( size_t j=0; j < fr_cnt; j++ )
      //  TBasicApp::GetLog() << olxstr::FormatFloat(3, vminQD[j]) << ' ';
      //TBasicApp::GetLog() << '\n';
      TNetwork* netb = Fragments[i];
      const size_t netb_cnt = netb->NodeCount();
      double minQD = 100000;
      smatd* transform = NULL;
      for( size_t j=0; j < neta_cnt; j++ )  {
        if( !neta->Node(j).CAtom().IsAvailable() )
          continue;
        const vec3d& crda = neta->Node(j).CAtom().ccrd();
        for( size_t k=0; k < netb_cnt; k++ )  {
          if( !netb->Node(k).CAtom().IsAvailable() )
            continue;
          const vec3d& crdb = netb->Node(k).CAtom().ccrd();
          double qd = 0;
          smatd* m = GetUnitCell().GetClosest(crda, crdb, true, &qd);
          if( m == NULL )  {
            if( qd < minQD )  { // still remember the minimal distance
              minQD = qd;
              if( transform != NULL )  {  // reset the transform as well
                delete transform;
                transform = NULL;
              }
            }
            continue;
          }
          if( qd < minQD )  {
            minQD = qd;
            if( transform != NULL )
              delete transform;
            transform = m;
          }
          else
            delete m;
        }
      }
      if( vminQD[i] <= minQD )  {
        if( transform )
          delete transform;
        continue;
      }
      vminQD[i] = minQD;
      if( transform == NULL )  continue;
      for( size_t k=0; k < netb_cnt; k++ )  {
        TSAtom& fb = netb->Node(k);
        if( fb.IsDeleted() )  continue;
        fb.CAtom().ccrd() = *transform * fb.CAtom().ccrd();
        if( fb.CAtom().GetEllipsoid() != NULL ) {
          *fb.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(
          transform->GetContainerId(), fb.CAtom().GetId());
        }
      }
      // this needs to be done if any one fragment is transformed multiple times...
      GetUnitCell().UpdateEllipsoids();
      delete transform;
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::CompaqType(short type)  {
  if( IsGenerated() )  return;
  const size_t ac = Objects.atoms.Count();
  const TAsymmUnit& au = GetAsymmUnit();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( sa.GetType() != type )  continue;
    smatd* transform = NULL;
    double minQD = 1000;
    for( size_t j=0; j < ac; j++ )  {
      TSAtom& sb = Objects.atoms[j];
      if( sb.GetType() == type || sb.GetType() == iQPeakZ )  continue;
      double qd = 0;
      smatd* m = GetUnitCell().GetClosest(sb.ccrd(), sa.ccrd(), true, &qd);
      if( qd < minQD )  {
        if( transform != NULL )
          delete transform;
        transform = m;
        minQD = qd;
      }
      else if( m != NULL )
        delete m;
    }
    if( transform == NULL )  continue;
    sa.ccrd() = sa.CAtom().ccrd() = (*transform * sa.ccrd());
    sa.crd() = au.Orthogonalise(sa.CAtom().ccrd());
    if( sa.CAtom().GetEllipsoid() != NULL )  {
      *sa.CAtom().GetEllipsoid() = GetUnitCell().GetEllipsoid(
        transform->GetContainerId(), sa.CAtom().GetId());
    }
    delete transform;
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::TransformFragments(const TSAtomPList& fragAtoms,
  const smatd& transform)
{
  if( IsGenerated() )  {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  /* transform may come with random Tag, so need to process ADP's manually -
  cannot pick from UC
  */
  const mat3d abc2xyz(mat3d::Transpose(GetAsymmUnit().GetCellToCartesian())),
              xyz2abc(mat3d::Transpose(GetAsymmUnit().GetCartesianToCell()));
  const mat3d etm = abc2xyz*transform.r*xyz2abc;
  ematd J = TEllipsoid::GetTransformationJ(etm),
    Jt = ematd::Transpose(J);
  for( size_t i=0; i < fragAtoms.Count(); i++ )
    fragAtoms[i]->GetNetwork().SetTag(i);

  for( size_t i=0; i < fragAtoms.Count(); i++ )  {
    if( (size_t)fragAtoms[i]->GetNetwork().GetTag() == i )  {
      for( size_t j=0; j < fragAtoms[i]->GetNetwork().NodeCount(); j++ )  {
        TSAtom& SA = fragAtoms[i]->GetNetwork().Node(j);
        SA.CAtom().ccrd() = transform * SA.CAtom().ccrd();
        if( SA.CAtom().GetEllipsoid() != NULL )
          SA.CAtom().GetEllipsoid()->Mult(etm, J, Jt);
      }
    }
  }
  OnStructureUniq.Enter(this);
  Init();
  OnStructureUniq.Exit(this);
}
//..............................................................................
void TLattice::UpdateConnectivity()  {
  UpdateAsymmUnit();
  Disassemble(false);
}
//..............................................................................
void TLattice::UpdateConnectivityInfo()  {
  GetAsymmUnit()._UpdateConnInfo();
  GetUnitCell().FindSymmEq();
  UpdateAsymmUnit();
  Disassemble(false);
}
//..............................................................................
void TLattice::Disassemble(bool create_planes)  {
  if( Objects.atoms.IsEmpty() )  return;
  OnDisassemble.Enter(this);
  // clear bonds & fragments
  ClearBonds();
  ClearFragments();
  {
    smatd *I = 0;
    for (size_t i = 0; i < Matrices.Count(); i++) {
      if (Matrices[i]->IsFirst()) {
        I = Matrices[i];
        break;
      }
    }
    if (I == 0) {
      I = Matrices.Add(new smatd());
      I->I().SetId(0);
    }
    for (size_t i = 0; i < Objects.atoms.Count(); i++) {
      TSAtom &a = Objects.atoms[i];
      if (a.CAtom().EquivCount() == 0) continue;
      if (!a.GetMatrix().IsFirst()) {
        for (size_t j = 0; j < a.CAtom().EquivCount(); j++) {
          uint32_t id = GetUnitCell().MulMatrixId(a.CAtom().GetEquiv(j), a.GetMatrix());
          if (smatd::IsFirst(id)) {
            a._SetMatrix(I);
            break;
          }
        }
      }
    }
  }
  TArrayList<vec3d> ocrd(Objects.atoms.Count());
  GenerateBondsAndFragments(&ocrd);
  if (create_planes)
    BuildPlanes();
  OnDisassemble.Exit(this);
}
//..............................................................................
void TLattice::RestoreCoordinates()  {
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    sa.crd() = GetAsymmUnit().Orthogonalise(sa.ccrd());
  }
}
//..............................................................................
bool TLattice::_AnalyseAtomHAdd(AConstraintGenerator& cg, TSAtom& atom,
  TSAtomPList& ProcessingAtoms, int part, TCAtomPList* generated)
{
  if( ProcessingAtoms.IndexOf(atom) != InvalidIndex ||
      (atom.CAtom().IsHAttached() && part == DefNoPart) )
    return false;
  ProcessingAtoms.Add(atom);

  cm_Element& h_elm = XElementLib::GetByIndex(iHydrogenIndex);
  TAtomEnvi AE;
  UnitCell->GetAtomEnviList(atom, AE, false, part);
  if( part == DefNoPart )  {  // check for disorder
    TIntList parts;
    TDoubleList occu;
    RefinementModel* rm = GetAsymmUnit().GetRefMod();
    for( size_t i=0; i < AE.Count(); i++ )  {
      if( AE.GetCAtom(i).GetPart() != 0 && AE.GetCAtom(i).GetPart() !=
          AE.GetBase().CAtom().GetPart() )
      {
        if( parts.IndexOf(AE.GetCAtom(i).GetPart()) == InvalidIndex )  {
          parts.Add(AE.GetCAtom(i).GetPart());
          occu.Add(rm->Vars.GetParam(AE.GetCAtom(i), catom_var_name_Sof));
        }
      }
    }
    if (!parts.IsEmpty()) {  // here we go..
      TTypeList<TCAtomPList> gen_atoms;
      ProcessingAtoms.Remove(atom);
      if (parts.Count() > 1) {
        for (size_t i=0; i < parts.Count(); i++) {
          _AnalyseAtomHAdd(cg, atom, ProcessingAtoms, parts[i],
            &gen_atoms.AddNew());
          TCAtomPList& gen = gen_atoms.GetLast();
          for (size_t j=0; j < gen.Count(); j++) {
            gen[j]->SetPart(parts[i]);
            rm->Vars.SetParam(*gen[j], catom_var_name_Sof, occu[i]);
          }
        }
      }
      else if (parts[0] > 0) { // special case with just a single part
        _AnalyseAtomHAdd(cg, atom, ProcessingAtoms, 0,
          &gen_atoms.AddNew());
        TCAtomPList& gen = gen_atoms.GetLast();
        /* if occu is fixed, it is > 5, then we 'invert' the variable like
        21 -> -21, otherwise, just set a value of 1-occu
        */
        double soccu = (olx_abs(occu[0]) > 5 ? -occu[0] : 1-occu[0]);
        int spart = (parts[0] == 2 ? 1 : olx_abs(parts[0])+1);
        for (size_t j=0; j < gen.Count(); j++) {
          gen[j]->SetPart(spart);
          rm->Vars.SetParam(*gen[j], catom_var_name_Sof, soccu);
        }
      }
      cg.AnalyseMultipart(AE, gen_atoms);
      return false;
    }
  }
  if( atom.GetType() == iCarbonZ )  {
    if( AE.Count() == 1 )  {
      // check acetilene
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      TSAtom* A = FindSAtom(AE.GetCAtom(0));
      if( A == 0 ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Could not locate atom ").quote() << AE.GetLabel(0));
      }

      TAtomEnvi NAE;
      UnitCell->GetAtomEnviList(*A, NAE, false, part);
      if( A->GetType() == iCarbonZ && NAE.Count() == 2 && d < 1.2)  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XCH";
        cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
      }
      else  {
        if( d > 1.35 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XCH3";
          cg.FixAtom(AE, fgCH3, h_elm, NULL, generated);
        }
        else  {
          if( d < 1.25 )  {
            if( NAE.Count() > 1 ) {
              bool done = false;
              if (NAE.Count() == 2) { // check acetilene again
                vec3d v = (NAE.GetCAtom(0) == atom.CAtom() ? NAE.GetCrd(1)
                  : NAE.GetCrd(0));
                double ang = (v-NAE.GetBase().crd())
                  .CAngle(atom.crd()-NAE.GetBase().crd());
                if ((ang+1) < 0.03) {
                  TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
                    ": XCH";
                  cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
                  done = true;
                }
              }
              if (!done) {
                TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
                  ": X=CH2";
                cg.FixAtom(AE, fgCH2, h_elm, NULL, generated);
              }
            }
            else {
              TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
                ": possibly X=CH2";
            }
          }
        }
      }
    }
    else  if( AE.Count() == 2 )  {
      const double db = 1.41;
      vec3d a = AE.GetCrd(0);
        a -= atom.crd();
      vec3d b = AE.GetCrd(1);
        b -= atom.crd();
      double v = a.CAngle(b);
      v = acos(v)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo( atom.crd() );
      double d2 = AE.GetCrd(1).DistanceTo( atom.crd() );
      if(  d1 > db && d2 > db && v < 125 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYCH2";
        cg.FixAtom(AE, fgCH2, h_elm, NULL, generated);
      }
      else  {
        if( (d1 < db || d2 < db) && v < 160 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X(Y=C)H";
          cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 3 )  {
      double v = olx_tetrahedron_volume(
        atom.crd(), AE.GetCrd(0), AE.GetCrd(1), AE.GetCrd(2));
      if( v > 0.3 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZCH";
        cg.FixAtom(AE, fgCH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 5 )  {  // carboranes ...
      //check
      if (TSPlane::CalcRMSD(AE) < 0.1) {
        bool proceed = false;
        for( size_t j=0; j < AE.Count(); j++ ) {
          if( AE.GetType(j) == iBoronZ )  {
            proceed = true;
            break;
          }
        }
        if( proceed )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R5CH";
          cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
        }
      }
    }
  }
  else if( atom.GetType() == iNitrogenZ )  {  // nitrogen
    if( AE.Count() == 1 )  {
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if( d > 1.35 )  {
        if( d > 1.44 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XNH3";
          cg.FixAtom(AE, fgNH3, h_elm, NULL, generated);
        }
        else  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XNH2";
          cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
        }
      }
      else  if( d > 1.2 )  {  //else nitrile
        // have to check if double bond
        TSAtom* A = FindSAtom(AE.GetCAtom(0));
        if( A == 0 ) {
          throw TFunctionFailedException(__OlxSourceInfo,
            olxstr("Could not locate atom ").quote() << AE.GetLabel(0));
        }
        TAtomEnvi NAE;
        UnitCell->GetAtomEnviList(*A, NAE, false, part);
        NAE.Exclude(atom.CAtom());

        if( A->GetType() == iCarbonZ && NAE.Count() > 1 )  {
          vec3d a = NAE.GetCrd(0);
          a -= NAE.GetBase().crd();
          vec3d b = AE.GetBase().crd();
          b -= NAE.GetBase().crd();

          d = a.CAngle(b);
          d = acos(d)*180/M_PI;
          if( d > 115 && d < 130 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X=NH2";
            cg.FixAtom(AE, fgNH2, h_elm, &NAE, generated);
          }
        }
        else  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X=NH";
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
    }
    else  if( AE.Count() == 2 )  {
      vec3d a = AE.GetCrd(0);
        a -= atom.crd();
      vec3d b = AE.GetCrd(1);
        b -= atom.crd();
      double v = a.CAngle(b);
      v = acos(v)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      if( d1 > 1.72 || d2 > 1.72 )  {  // coordination?
        if( v > 165 )  // skip ..
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": RN->M";
        else if( (d1 < 1.5 && d1 > 1.35) || (d2 < 1.5 && d2 > 1.35) )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": RNH(2)M";
          cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
        }
        else if( d1 > 1.72 && d2 > 1.72 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XX'NH";
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
      else if( v < 121 && d1 > 1.45 && d2 > 1.45 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R2NH2+";
        cg.FixAtom(AE, fgNH2, h_elm, NULL, generated);
      }
      else if( v < 121 && (d1 < 1.3 || d2 < 1.3) )
        ;
      else  {
        if( (d1+d2) > 2.70 && v < 140 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYNH";
          if (d1 > 1.4 && d2 > 1.4)
            cg.FixAtom(AE, fgNH1t, h_elm, NULL, generated);
          else
            cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
      }
    }
    else if( AE.Count() == 3 )  {
    // remove coordination bond ...
      vec3d a = AE.GetCrd(0);
        a -= atom.crd();
      vec3d b = AE.GetCrd(1);
        b -= atom.crd();
      vec3d c = AE.GetCrd(2);
        c -= atom.crd();
      double v1 = a.CAngle(b);  v1 = acos(v1)*180/M_PI;
      double v2 = a.CAngle(c);  v2 = acos(v2)*180/M_PI;
      double v3 = b.CAngle(c);  v3 = acos(v3)*180/M_PI;
      double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      double d3 = AE.GetCrd(2).DistanceTo(atom.crd());
      if( (v1+v2+v3) < 350 && d1 > 1.45 && d2 > 1.45 && d3 > 1.45 )  {
        if( d1 > 1.75 || d2 > 1.75 || d3 > 1.75 )  {
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R2HN->M";
          cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
        }
        else  {
          // this excludes P-N bonds, http://www.olex2.org/olex2-bugs/359
          if( d1 < 1.65 && d2 < 1.65 && d3 < 1.65 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": R3NH+";
            cg.FixAtom(AE, fgNH1, h_elm, NULL, generated);
          }
        }
      }
    }
  }
  if( atom.GetType() == iOxygenZ )  {  // oxygen
    if( AE.IsEmpty() )  {
      TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": OH2";
      TAtomEnvi pivoting;
      UnitCell->GetAtomPossibleHBonds(AE, pivoting);
      UnitCell->FilterHBonds(AE, pivoting, true);
      RemoveNonHBonding(pivoting);
      cg.FixAtom(AE, fgOH2, h_elm, &pivoting, generated);
    }
    else if( AE.Count() == 1 )  {
      const double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if( d > 1.3 )   {  // otherwise a doubl bond
        TAtomEnvi pivoting;
        UnitCell->GetAtomPossibleHBonds(AE, pivoting);
        // d < 1.8 - move bonds only if not coordination
        UnitCell->FilterHBonds(AE, pivoting, d < 1.8);
        RemoveNonHBonding(pivoting);
        if( AE.GetType(0) == iChlorineZ )
          ;
        else  if( AE.GetType(0) == iCarbonZ )  {  // carbon
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": COH";
          cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
        }
        else  if( AE.GetType(0) == iSulphurZ )  {
          if( d > 1.48 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": SOH";
            cg.FixAtom(AE, fgOH1, h_elm, NULL, generated);
          }
        }
        else  if( AE.GetType(0) == iPhosphorusZ )  {
          if( d > 1.54 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": POH";
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iSiliconZ )  {
          if( d > 1.6 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": SiOH";
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iBoronZ )  {
          if( d < 1.38 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": B(III)OH";
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else  if( AE.GetType(0) == iNitrogenZ )  {
          if( d > 1.37 )  {
            TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": NOH";
            cg.FixAtom(AE, fgOH1, h_elm, &pivoting, generated);
          }
        }
        else if( d > 1.8 )  {  // coordination bond?
          TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
            ": possibly M-OH2";
          cg.FixAtom(AE, fgOH2, h_elm, &pivoting, generated);
        }
      }
    }
    else if( AE.Count() == 2 )  {
      const double d1 = AE.GetCrd(0).DistanceTo(atom.crd());
      const double d2 = AE.GetCrd(1).DistanceTo(atom.crd());
      if( (d1 > 1.8 && d2 < 1.8 && d2 > 1.38) ||
          (d2 > 1.8 && d1 < 1.8 && d1 > 1.38) )
      {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() <<
          ": possibly M-O(H)R";
        cg.FixAtom(AE, fgOH1, h_elm, NULL, generated);
      }
    }
  }
  else if( atom.GetType() == iBoronZ )  {  // boron
    if( AE.Count() == 3 )  {
      const vec3d cnt = AE.GetBase().crd();
      const double v = olx_tetrahedron_volume( 
        cnt, 
        (AE.GetCrd(0)-cnt).Normalise() + cnt, 
        (AE.GetCrd(1)-cnt).Normalise() + cnt, 
        (AE.GetCrd(2)-cnt).Normalise() + cnt);
      if( v > 0.1 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZBH";
        cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 4 )  {
      vec3d a, b;
      double sumAng = 0;
      for( size_t i=0; i < AE.Count(); i++ )  {
        a = AE.GetCrd(i);
        a -= atom.crd();
        for( size_t j=i+1; j < AE.Count(); j++ )  {
          b = AE.GetCrd(j);
          b -= atom.crd();
          double ca = b.CAngle(a);
          if( ca < -1 )  ca = -1;
          if( ca > 1 )   ca = 1;
          sumAng += acos(ca);
        }
      }
      if( sumAng*180/M_PI > 700 )  {   //!! not sure it works, lol
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X4BH";
        cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 5 )  {
      TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": X5BH";
      cg.FixAtom(AE, fgBH1, h_elm, NULL, generated);
    }
  }
  else if( atom.GetType() == iSiliconZ )  {
    if( AE.Count() == 3 )  {
      const vec3d cnt = AE.GetBase().crd();
      const double v = olx_tetrahedron_volume( 
        cnt, 
        (AE.GetCrd(0)-cnt).Normalise() + cnt, 
        (AE.GetCrd(1)-cnt).Normalise() + cnt, 
        (AE.GetCrd(2)-cnt).Normalise() + cnt);
      if( v > 0.1 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYZSiH";
        cg.FixAtom(AE, fgSiH1, h_elm, NULL, generated);
      }
    }
    else if( AE.Count() == 2 )  {  // no validation yet...
      TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": XYSiH2";
      cg.FixAtom(AE, fgSiH2, h_elm, NULL, generated);
    }
  }
  else if( atom.GetType() == iSulphurZ )  {
    if( AE.Count() == 1 && AE.GetType(0) == iCarbonZ )  {
      double d = AE.GetCrd(0).DistanceTo(atom.crd());
      if( d > 1.72 )  {
        TBasicApp::NewLogEntry(logInfo) << atom.GetLabel() << ": CSH";
        cg.FixAtom(AE, fgSH1, h_elm, NULL, generated);
      }
    }
  }
  ProcessingAtoms.Delete(ProcessingAtoms.IndexOf(&atom));
  return true;
}
//..............................................................................
void TLattice::_ProcessRingHAdd(AConstraintGenerator& cg,
  const ElementPList& rcont, const TSAtomPList& atoms)
{
  TTypeList<TSAtomPList> rings;
  cm_Element& h_elm = XElementLib::GetByIndex(iHydrogenIndex);
  for( size_t i=0; i < FragmentCount(); i++ )
    GetFragment(i).FindRings(rcont, rings);
  TAtomEnvi AE;
  for( size_t i=0; i < rings.Count(); i++ )  {
    double rms = TSPlane::CalcRMSD(rings[i]);
    if( rms < 0.05 && TNetwork::IsRingRegular( rings[i]) )  {
      for( size_t j=0; j < rings[i].Count(); j++ )  {
        AE.Clear();
        UnitCell->GetAtomEnviList(*rings[i][j], AE);
        if( AE.Count() == 3 )  {
          const vec3d cnt = AE.GetBase().crd();
          try  {
            const double v = olx_tetrahedron_volume( 
              cnt, 
              (AE.GetCrd(0)-cnt).Normalise() + cnt, 
              (AE.GetCrd(1)-cnt).Normalise() + cnt, 
              (AE.GetCrd(2)-cnt).Normalise() + cnt);
            if( v < 0.1 )  continue;  // coordination or substituted
          }
          catch(...)  {  continue;  }
        }
        for( size_t k=0; k < AE.Count(); k++ )  {
          if( (AE.GetCrd(k) - rings[i][j]->crd()).QLength() > 4.0 )
            AE.Delete(k--);
        }
        if( AE.Count() == 2 && rings[i][j]->GetType() == iCarbonZ &&
            atoms.IndexOf(AE.GetBase()) != InvalidIndex )
        {
          TBasicApp::NewLogEntry(logInfo) << rings[i][j]->GetLabel() <<
            ": X(Y=C)H (ring)";
          cg.FixAtom(AE, fgCH1, h_elm);
          rings[i][j]->CAtom().SetHAttached(true);
        }
      }
    }
  }
}
//..............................................................................
void TLattice::AnalyseHAdd(AConstraintGenerator& cg, const TSAtomPList& atoms)  {
  ElementPList CTypes;
  CTypes.Add(XElementLib::FindBySymbol("C"));
  CTypes.Add(XElementLib::FindBySymbol("N"));
  CTypes.Add(XElementLib::FindBySymbol("O"));
  CTypes.Add(XElementLib::FindBySymbol("B"));
  CTypes.Add(XElementLib::FindBySymbol("Si"));
  CTypes.Add(XElementLib::FindBySymbol("S"));
  TSAtomPList ProcessingAtoms;

  for( size_t i=0; i < atoms.Count(); i++ )
    atoms[i]->CAtom().SetHAttached(false);

  // treat rings
  ElementPList rcont;
  rcont.Add(CTypes[0]);
  for( size_t i=0; i < 4; i++ )  
    rcont.Add(rcont[0]);
  _ProcessRingHAdd(cg, rcont, atoms); // Cp
  rcont.Add(rcont[0]);
  _ProcessRingHAdd(cg, rcont, atoms); // Ph
  rcont.GetLast() = CTypes[1];
  _ProcessRingHAdd(cg, rcont, atoms); // Py

  TAsymmUnit &au = GetAsymmUnit();
  au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
  TSAtomPList waters;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i]->IsDeleted() || !atoms[i]->CAtom().IsAvailable() ||
      atoms[i]->CAtom().GetTag() != 0 )
      continue;
    // mark the atoms processed
    atoms[i]->CAtom().SetTag(1);
    bool consider = false;
    for( size_t j=0; j < CTypes.Count(); j++ )  {
      if( atoms[i]->GetType() == *CTypes[j] )  {
        consider = true;
        break;
      }
    }
    if( !consider )  continue;
    for( size_t j=0; j < atoms[i]->NodeCount(); j++ )  {
      TSAtom& A = atoms[i]->Node(j);
      if( A.IsDeleted() )  continue;
      if( A.GetType() == iHydrogenZ ) {
        consider = false;
        break;
      }
    }
    for( size_t j=0; j < atoms[i]->CAtom().AttachedSiteCount(); j++ )  {
      if( atoms[i]->CAtom().GetAttachedAtom(j).GetType() == iHydrogenZ &&
        !atoms[i]->CAtom().GetAttachedAtom(j).IsDeleted() )
      {
        consider = false;
        break;
      }
    }
    if( !consider )  continue;
    if (atoms[i]->GetType() == iOxygenZ) {
      size_t nhc=0;
      for (size_t si=0; si < atoms[i]->CAtom().AttachedSiteCount(); si++) {
        TCAtom::Site &s = atoms[i]->CAtom().GetAttachedSite(si);
        if (s.atom->IsDeleted() || s.atom->GetType().z < 2)
          continue;
        nhc++;
      }
      if (nhc == 0) {
        waters << atoms[i];
        atoms[i]->CAtom().SetTag(1);
        continue;
      }
    }
    _AnalyseAtomHAdd(cg, *atoms[i], ProcessingAtoms);
  }
  for (size_t i=0; i < waters.Count(); i++) {
    _AnalyseAtomHAdd(cg, *waters[i], ProcessingAtoms);
  }
  /* // this might be useful for hadd on grown structures
  GetUnitCell().AddEllipsoid(au.AtomCount()-au_cnt);
  GetUnitCell().FindSymmEq();
  au.GetAtoms().ForEach(ACollectionItem::TagSetter(-1));
  for( size_t i=au_cnt; i < au.AtomCount(); i++ )
    GenerateAtom(au.GetAtom(i), *Matrices[0]);
    */
}
//..............................................................................
void TLattice::RemoveNonHBonding(TAtomEnvi& Envi)  {
  TAtomEnvi AE;
  for( size_t i=0; i < Envi.Count(); i++ )  {
    TSAtom* SA = FindSAtom(Envi.GetCAtom(i));
    AE.Clear();
    UnitCell->GetAtomEnviList(*SA, AE);
    if( SA->GetType() == iOxygenZ )  {
      if( AE.Count() == 1 )  {
        const double d = AE.GetCrd(0).DistanceTo(SA->crd());
        if( d > 1.8 && XElementLib::IsMetal(SA->GetType()))  // coordination bond?
          Envi.Exclude(SA->CAtom());
      }
      else if( AE.Count() == 2 )  {  // not much can be done here ... needs thinking
        //Envi.Exclude( SA->CAtom() );
        // commented 17.03.08, just trying to what the shortest distance will give
      }
      //else if( AE.Count() == 3 )  {  // coordinated water molecule
      //  Envi.Exclude(SA->CAtom());
      //}
    }
    else if( SA->GetType() == iNitrogenZ )  {
      if( AE.Count() > 3 )
          Envi.Exclude(SA->CAtom());
    }
  }
  // choose the shortest bond ...
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;
    for( size_t i=0; i < Envi.Count(); i++ )  {
      double d = Envi.GetBase().crd().DistanceTo(Envi.GetCrd(i));
      if( Envi.GetMatrix(i).IsFirst() && // prioritise sligtly longer intramolecular bonds
        Envi.GetBase().CAtom().GetFragmentId() == Envi.GetCAtom(i).GetFragmentId() )
      {
        d -= 0.15;
      }
      hits.Add(d, &Envi.GetCAtom(i));
    }

    while( hits.Count() > 1 &&
      ((hits.GetLastKey() - hits.GetKey(0)) > 0.15) )  {
      Envi.Exclude(*hits.GetObject(hits.Count()-1));
      hits.Delete(hits.Count()-1);
    }
  }
  // all similar length  .... Q peaks might help :)
  if( Envi.Count() > 1 )  {
    TPSTypeList<double, TCAtom*> hits;
    AE.Clear();
    UnitCell->GetAtomQEnviList(Envi.GetBase(), AE);
    for( size_t i=0; i < AE.Count(); i++ )  {
//      v1 = AE.GetCrd(i);
//      v1 -= Envi.GetBase()->crd();
      const double d = Envi.GetBase().crd().DistanceTo(AE.GetCrd(i));
      if( d < 0.7 || d > 1.3 )
        AE.Exclude(AE.GetCAtom(i--));
    }
    if( AE.IsEmpty() || AE.Count() > 1 )  return;
    vec3d vec1 = AE.GetCrd(0) - Envi.GetBase().crd();
    for( size_t i=0; i < Envi.Count(); i++ )  {
      vec3d vec2 = Envi.GetCrd(i) - Envi.GetBase().crd();
      hits.Add(olx_abs(-1 + vec2.CAngle(vec1)), &Envi.GetCAtom(i));
    }
    while( hits.Count() > 1 )  {
      Envi.Exclude(*hits.GetObject( hits.Count() - 1 ) );
      hits.Delete(hits.Count() - 1);
    }
  }

}
//..............................................................................
void TLattice::SetAnis(const TCAtomPList& atoms, bool anis)  {
  if (atoms.IsEmpty()) return; 
  if( !anis )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( olx_is_valid_index(atoms[i]->GetEllpId()) )  {
         GetAsymmUnit().NullEllp(atoms[i]->GetEllpId());
         atoms[i]->AssignEllp(NULL);
      }
    }
    GetAsymmUnit().PackEllps();
  }
  else  {
    evecd ee(6);
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL)  {
        ee[0] = ee[1] = ee[2] = atoms[i]->GetUiso();
        atoms[i]->UpdateEllp(ee);
      }
    }
  }
  GetUnitCell().UpdateEllipsoids();
  RestoreADPs(false);
}
//..............................................................................
void TLattice::ToDataItem(TDataItem& item) const  {
  item.AddField("delta", Delta);
  item.AddField("deltai", DeltaI);
  GetAsymmUnit().ToDataItem(item.AddItem("AUnit"));
  TDataItem& mat = item.AddItem("Matrices");
  const size_t mat_c = Matrices.Count();
  /* save matrices, change matrix tags to the position in the list and remember
    old tags
  */
  TArrayList<uint32_t> m_tags(mat_c);
  for( size_t i=0; i < mat_c; i++ )  {
    mat.AddItem("symop", TSymmParser::MatrixToSymmEx(*Matrices[i]))
      .AddField("id", Matrices[i]->GetId());
    m_tags[i] = Matrices[i]->GetId();
    Matrices[i]->SetRawId((uint32_t)i);
  }
  // initialise bond tags
  size_t sbond_tag = 0;
  for( size_t i=0; i < Objects.bonds.Count(); i++ )  {
    if( Objects.bonds[i].IsDeleted() )  continue;
    Objects.bonds[i].SetTag(sbond_tag++);
  }
  // initialise atom tags
  size_t satom_tag = 0;
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    if( Objects.atoms[i].IsDeleted() )  continue;
    Objects.atoms[i].SetTag(satom_tag++);
  }
  // initialise fragment tags
  size_t frag_tag = 0;
  Network->SetTag(-1);
  for( size_t i=0; i < Fragments.Count(); i++ )
    Fragments[i]->SetTag(frag_tag++);
  // save satoms - only the original CAtom Tag and the generating matrix tag
  TDataItem& atoms = item.AddItem("Atoms");
  for( size_t i=0; i < Objects.atoms.Count(); i++ )  {
    if( !Objects.atoms[i].IsDeleted() )
      Objects.atoms[i].ToDataItem(atoms.AddItem("Atom"));
  }
  // save bonds
  TDataItem& bonds = item.AddItem("Bonds");
  for( size_t i=0; i < Objects.bonds.Count(); i++ )  {
    if( !Objects.bonds[i].IsDeleted() )
      Objects.bonds[i].ToDataItem(bonds.AddItem("Bond"));
  }
  // save fragments
  TDataItem& frags = item.AddItem("Fragments");
  for( size_t i=0; i < Fragments.Count(); i++ )
    Fragments[i]->ToDataItem(frags.AddItem("Fragment"));
  // restore original matrix tags 
  for( size_t i=0; i < mat_c; i++ )
    Matrices[i]->SetRawId(m_tags[i]);
  // save planes
  TSPlanePList valid_planes;
  for( size_t i=0; i < Objects.planes.Count(); i++ )  {
    if( Objects.planes[i].IsDeleted() ) continue;
    size_t p_ac = 0;  
    for( size_t j=0; j < Objects.planes[i].Count(); j++ ) 
      if( Objects.planes[i].GetAtom(j).IsAvailable() )
        p_ac++;
    if( p_ac >= 3 ) // a plane must contain at least three atoms
      valid_planes.Add(Objects.planes[i]);
    else
      Objects.planes[i].SetDeleted(true);
  }
  TDataItem& planes = item.AddItem("Planes");
  for( size_t i=0; i < valid_planes.Count(); i++ )
    valid_planes[i]->ToDataItem(planes.AddItem("Plane"));
}
//..............................................................................
void TLattice::FromDataItem(TDataItem& item)  {
  TActionQueueLock ql(&OnAtomsDeleted);
  Clear(true);
  Delta = item.GetFieldByName("delta").ToDouble();
  DeltaI = item.GetFieldByName("deltai").ToDouble();
  GetAsymmUnit().FromDataItem(item.GetItemByName("AUnit"));
  GetUnitCell().InitMatrices();
  const TDataItem& mat = item.GetItemByName("Matrices");
  Matrices.SetCapacity(mat.ItemCount());
  for( size_t i=0; i < mat.ItemCount(); i++ )  {
    smatd* m = new smatd(
      TSymmParser::SymmToMatrix(mat.GetItemByIndex(i).GetValue()));
    GetUnitCell().InitMatrixId(*Matrices.Add(m));
    m->SetRawId(mat.GetItemByIndex(i).GetFieldByName("id").ToUInt());
  }
  // precreate fragments
  const TDataItem& frags = item.GetItemByName("Fragments");
  Fragments.SetCapacity(frags.ItemCount());
  for( size_t i=0; i < frags.ItemCount(); i++ )
    Fragments.Add(new TNetwork(this, NULL));
  // precreate bonds
  const TDataItem& bonds = item.GetItemByName("Bonds");
  Objects.bonds.IncCapacity(bonds.ItemCount());
  for( size_t i=0; i < bonds.ItemCount(); i++ )
    Objects.bonds.New(NULL);
  // precreate and load atoms
  const TDataItem& atoms = item.GetItemByName("Atoms");
  Objects.atoms.IncCapacity(atoms.ItemCount());
  for( size_t i=0; i < atoms.ItemCount(); i++ )
    Objects.atoms.New(NULL);
  for( size_t i=0; i < atoms.ItemCount(); i++ )
    Objects.atoms[i].FromDataItem(atoms.GetItemByIndex(i), *this);
  // load bonds
  for( size_t i=0; i < bonds.ItemCount(); i++ )
    Objects.bonds[i].FromDataItem(bonds.GetItemByIndex(i), *this);
  // load fragments
  for( size_t i=0; i < frags.ItemCount(); i++ )
    Fragments[i]->FromDataItem(frags.GetItemByIndex(i));
  TDataItem& planes = item.GetItemByName("Planes");
  for( size_t i=0; i < planes.ItemCount(); i++ )  {
    TSPlane& p = Objects.planes.New(Network);
    p.FromDataItem(planes.GetItemByIndex(i));
    TSPlane::Def def = p.GetDef();
    size_t di = InvalidIndex;
    for( size_t j=0; j < PlaneDefs.Count(); j++ )  {
      if( PlaneDefs[j] == def )  {
        di = j;
        break;
      }
    }
    if( di == InvalidIndex )  {
      p._SetDefId(PlaneDefs.Count());
      PlaneDefs.AddNew(def);
    }
    else
      p._SetDefId(di);
  }
  //FinaliseLoading();
}
//..............................................................................
void TLattice::FinaliseLoading() {
  GetAsymmUnit()._UpdateConnInfo();
  GetUnitCell().FindSymmEq();
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )
    GetAsymmUnit().GetAtom(i).SetDeleted(false);
  BuildAtomRegistry();
}
//..............................................................................
void TLattice::SetGrowInfo(GrowInfo* grow_info)  {
  if( _GrowInfo != NULL )
    delete _GrowInfo;
  _GrowInfo = grow_info;
}
//..............................................................................
TLattice::GrowInfo* TLattice::GetGrowInfo() const {
  if( !IsGenerated() )  return NULL;
  const TAsymmUnit& au = GetAsymmUnit();
  GrowInfo& gi = *(new GrowInfo);
  gi.matrices.SetCount( Matrices.Count() );
  gi.unc_matrix_count = GetUnitCell().MatrixCount();
  // save matrix tags and init gi.matrices
  TArrayList<uint32_t> mtags(Matrices.Count());
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    mtags[i] = Matrices[i]->GetId();
    (gi.matrices[i] = new smatd(*Matrices[i]))->SetRawId(mtags[i]);
    Matrices[i]->SetRawId((uint32_t)i);
  }

  gi.info.SetCount(au.AtomCount());
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    gi.info[sa.CAtom().GetId()] << sa.GetMatrix().GetId();
  }
  // restore matrix tags
  for( size_t i=0; i < mtags.Count(); i++ )
    Matrices[i]->SetRawId(mtags[i]);
  return &gi;
}
//..............................................................................
bool TLattice::ApplyGrowInfo()  {
  TAsymmUnit& au = GetAsymmUnit();
  if( _GrowInfo == NULL || !Objects.atoms.IsEmpty() || !Matrices.IsEmpty() || 
    GetUnitCell().MatrixCount() != _GrowInfo->unc_matrix_count )
  {
    if( _GrowInfo != NULL )  {
      delete _GrowInfo;
      _GrowInfo = NULL;
    }
    return false;
  }
  Matrices.Assign(_GrowInfo->matrices);
  _GrowInfo->matrices.Clear();
  Objects.atoms.IncCapacity(au.AtomCount()*Matrices.Count());
  for( size_t i=0; i < au.AtomCount(); i++ )    {
    TCAtom& ca = GetAsymmUnit().GetAtom(i);
    // we still need masked and detached atoms here
    if( ca.IsDeleted() )  continue;
    if( i >= _GrowInfo->info.Count() )  {  // create just with I matrix
      GenerateAtom(ca, *Matrices[0]);
      continue;
    }
    const TIndexList& mi = _GrowInfo->info[i];
    for( size_t j=0; j < mi.Count(); j++ )
      GenerateAtom(ca, *Matrices[mi[j]]);
  }
  delete _GrowInfo;
  _GrowInfo = NULL;
  return true;
}
//..............................................................................
void TLattice::_CreateFrags(TCAtom& start, TCAtomPList& dest)  {
  start.SetTag(1);
  dest.Add(start);
  for( size_t i=0; i < start.AttachedSiteCount(); i++ )  {
    const TCAtom::Site& site = start.GetAttachedSite(i);
    if( site.atom->GetTag() != 0 )  continue;
    _CreateFrags(*site.atom, dest);
  }
}
//..............................................................................
olxstr TLattice::CalcMoiety() const {
  const TAsymmUnit& au = GetAsymmUnit();
  TTypeList<TCAtomPList> cfrags;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() || au.GetAtom(i).GetType() == iQPeakZ )
      au.GetAtom(i).SetTag(1);  // ignore
    else
      au.GetAtom(i).SetTag(0); // unprocessed
  }
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetTag() == 0 )
      _CreateFrags(au.GetAtom(i), cfrags.AddNew());
  }
  // multiplicity,content, reference fragment index
  TTypeList<AnAssociation3<double,ContentList, size_t> > frags;
  for( size_t i=0; i < cfrags.Count(); i++ )  {
    ElementDict _cld;
    for( size_t j=0; j < cfrags[i].Count(); j++ )
      _cld.Add(&cfrags[i][j]->GetType(), 0) += cfrags[i][j]->GetOccu();
    ContentList cl(_cld.Count(), false);
    for( size_t j=0; j < _cld.Count(); j++ )
      cl.Set(j, new ElementCount(*_cld.GetKey(j), _cld.GetValue(j)));
    XElementLib::SortContentList(cl);
    bool uniq = true;
    double overall_occu = 0;
    for( size_t j=0; j < cfrags[i].Count(); j++ )  {
      const double occu = cfrags[i][j]->GetOccu();
      if( overall_occu == 0 )
        overall_occu = occu;
      else if( overall_occu != -1 && olx_abs(overall_occu-occu) > 0.01 )  {
        overall_occu = -1;
        break;
      }
    }
    for( size_t j=0; j < frags.Count(); j++ )  {
      if( frags[j].GetB().Count() != cl.Count() )  continue;
      bool equals = true;
      if( frags[j].GetB()[0].element != cl[0].element )
        equals = false;
      else  {
        for( size_t k=1; k < cl.Count(); k++ )  {
          if( frags[j].GetB()[k].element != cl[k].element || 
            olx_abs((frags[j].GetB()[k].count/frags[j].GetB()[0].count)-(cl[k].count/cl[0].count)) > 0.01 )
          {
            equals = false;
            break;
          }
        }
      }
      if( equals )  {
        frags[j].A() += cl[0].count/frags[j].GetB()[0].count;
        uniq = false;
        break;
      }
    }
    if( uniq )  {
      if( olx_abs(overall_occu) == 1 )
        frags.AddNew(1, cl, i);
      else  {  // apply overal atom occupancy
        for( size_t j=0; j < cl.Count(); j++ )
          cl[j].count /= overall_occu;
        frags.AddNew(overall_occu, cl, i);
      }
    }
  }
  // apply Z multiplier...
  const double zp_mult = (double)GetUnitCell().MatrixCount()/olx_max(au.GetZ(), 1);
  if( zp_mult != 1 )  {
    for( size_t i=0; i < frags.Count(); i++ )  {
      const TCAtomPList& l = cfrags[frags[i].GetC()];
      const size_t generators = GetFragmentGrowMatrices(l, false).Count();
      const int gd = int(generators == 0 ? 1 : generators);
      frags[i].A() *= zp_mult/gd;
      for( size_t j=0; j < frags[i].GetB().Count(); j++ )
        frags[i].B()[j].count *= gd;
    }
  }
  olxstr rv;
  for( size_t i=0; i < frags.Count(); i++ )  {
    if( !rv.IsEmpty() )  rv << ", ";
    if( frags[i].GetA() != 1 )
      rv << olx_round(frags[i].GetA(), 100) << '(';
    for( size_t j=0; j < frags[i].GetB().Count(); j++ )  {
      rv << frags[i].GetB()[j].element.symbol;
      if( frags[i].GetB()[j].count != 1 )
        rv << olx_round(frags[i].GetB()[j].count, 100);
      if( (j+1) < frags[i].GetB().Count() )
        rv << ' ';
    }
    if( frags[i].GetA() != 1 )
      rv << ')';
  }
  return rv;
}
//..............................................................................
void TLattice::RestoreADPs(bool restoreCoordinates)  {
  TUnitCell& uc = GetUnitCell();
  const TAsymmUnit& au = GetAsymmUnit();
  uc.UpdateEllipsoids();
  const size_t ac = Objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = Objects.atoms[i];
    if( restoreCoordinates )
      sa.crd() = au.Orthogonalise(sa.ccrd());
    if( sa.CAtom().GetEllipsoid() != NULL ) {
      sa.SetEllipsoid(
        &uc.GetEllipsoid(sa.GetMatrix().GetContainerId(), sa.CAtom().GetId()));
    }
    else
      sa.SetEllipsoid(NULL);
  }
  for( size_t i=0; i < uc.EllpCount(); i++ )  {
    TEllipsoid* elp = uc.GetEllp(i);
    if( elp != NULL )  
      elp->SetTag(0);
  }
}
//..............................................................................
void TLattice::BuildAtomRegistry()  {
  if( Matrices.IsEmpty() )  return;
  vec3i mind(100,100,100), maxd(-100,-100,-100);
  const size_t ac = Objects.atoms.Count();
  TTypeList<TSAtom::Ref> refs(ac);
  for( size_t i=0; i < ac; i++ )  {
    TSAtom &sa = Objects.atoms[i];
    if( !sa.IsAvailable() )  continue;
    refs[i] = sa.GetRef();
    vec3i::UpdateMinMax(smatd::GetT(refs[i].matrix_id), mind, maxd);
  }
  if (ac == 0) {
    maxd = mind = vec3i(0);
  }
  if (mind[0] == 100) return;
  maxd[0] += 1;  maxd[1] += 1;  maxd[2] += 1;
  AtomRegistry::RegistryType& registry = Objects.atomRegistry.Init(mind, maxd);
  for( size_t i=0; i < ac; i++ )  {
    TSAtom* sa = &Objects.atoms[i];
    if( !sa->IsAvailable() || sa->CAtom().IsMasked() )  continue;
    const vec3i t = smatd::GetT(refs[i].matrix_id);
    TArrayList<TSAtomPList*>* aum_slice = registry.Value(t);
    if( aum_slice == NULL )  {
      const size_t matr_cnt = GetUnitCell().MatrixCount();
      aum_slice = (registry.Value(t) =
        new TArrayList<TSAtomPList*>(matr_cnt, olx_list_init::zero()));
    }
    uint8_t  c_id = smatd::GetContainerId(refs[i].matrix_id);
    TSAtomPList* au_slice = (*aum_slice)[c_id];
    if( au_slice == NULL )  {
      const size_t atom_cnt = GetAsymmUnit().AtomCount();
      au_slice = ((*aum_slice)[c_id] = new TSAtomPList(atom_cnt));
    }
    else if( (*au_slice)[refs[i].catom_id] != NULL &&
        (*au_slice)[refs[i].catom_id] != sa )
    {
      (*au_slice)[refs[i].catom_id]->SetDeleted(true);
    }
    (*au_slice)[refs[i].catom_id] = sa;
  }
}
//..............................................................................
void TLattice::AddLatticeContent(const TLattice& latt)  {
  if( latt.IsGenerated() )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot adopt grown structure");
  TSAtomPList new_atoms;
  TSBondPList new_bonds;
  for( size_t i=0; i < latt.Objects.atoms.Count(); i++ )  {
    const TSAtom& src_a = latt.Objects.atoms[i];
    TCAtom& ca = GetAsymmUnit().NewAtom();
    GetAsymmUnit().CartesianToCell(ca.ccrd() = src_a.crd());
    ca.SetType(src_a.GetType());
    ca.SetLabel(src_a.GetLabel(), false);
    TSAtom* sa = new_atoms.Add(Objects.atoms.New(Network));
    sa->CAtom(ca);
    sa->crd() = GetAsymmUnit().Orthogonalise(sa->ccrd());
    sa->_SetMatrix(Matrices[0]);
  }
  for( size_t i=0; i < latt.Objects.bonds.Count(); i++ )  {
    const TSBond& src_b = latt.Objects.bonds[i];
    TSBond* sb = new_bonds.Add(Objects.bonds.New(Network));
    sb->SetA(*new_atoms[src_b.A().GetOwnerId()]);
    sb->SetB(*new_atoms[src_b.B().GetOwnerId()]);
  }
  for( size_t i=0; i < latt.Objects.atoms.Count(); i++ )  {
    const TSAtom& src_a = latt.Objects.atoms[i];
    TSAtom& sa = *new_atoms[i];
    for( size_t j=0; j < src_a.NodeCount(); j++ )
      sa.AddNode(*new_atoms[src_a.Node(j).GetOwnerId()]);
    for( size_t j=0; j < src_a.BondCount(); j++ )
      sa.AddBond(*new_bonds[src_a.Bond(j).GetOwnerId()]);
  }
  for( size_t i=0; i < latt.FragmentCount(); i++ )  {
    const TNetwork& src_n = latt.GetFragment(i);
    TNetwork& net = *Fragments.Add(new TNetwork(this, Network));
    net.SetOwnerId(Fragments.Count()-1);
    for( size_t j=0; j < src_n.NodeCount(); j++ )  {
      TSAtom& a = *new_atoms[src_n.Node(j).GetOwnerId()];
      net.AddNode(a);
      a.SetNetwork(net);
    }
    for( size_t j=0; j < src_n.BondCount(); j++ )  {
      TSBond& b = *new_bonds[src_n.Bond(j).GetOwnerId()];
      net.AddBond(b);
      b.SetNetwork(net);
    }
  }
  GetUnitCell().UpdateEllipsoids();
  RestoreADPs(false);
}
//..............................................................................
void TLattice::SetDelta(double v)  {
  if( Delta != v )  {
    Delta = v;
    UpdateConnectivity();
  }
}
//..............................................................................
void TLattice::SetDeltaI(double v)  {
  if( DeltaI != v )  {
    DeltaI = v;
    GetUnitCell().FindSymmEq();
    UpdateConnectivity();
  }
}
//..............................................................................
void TLattice::undoDelete(TUndoData *data)  {
  TDeleteUndo *undo = dynamic_cast<TDeleteUndo*>(data);
  for (size_t i=0; i < undo->SAtomIds.Count(); i++)
    RestoreAtom(undo->SAtomIds[i]);
  UpdateConnectivity();
}
//..............................................................................
TUndoData *TLattice::ValidateHGroups(bool reinit, bool report) {
  TCAtomPList deleted;
  TAsymmUnit &au = GetAsymmUnit();
  RefinementModel &rm = *au.GetRefMod();
  while (true) {
    size_t deleted_cnt = deleted.Count();
    for (size_t i=0; i < rm.AfixGroups.Count(); i++) {
      TAfixGroup &ag = rm.AfixGroups[i];
      if (!ag.HasImplicitPivot() || ag.IsEmpty()) continue;
      int part=0;
      for (size_t j=0; j < ag.Count(); j++) {
        if (ag[j].IsDeleted()) continue;
        part = ag[j].GetPart();
        break;
      }
      size_t attached_cnt=0, metal_cnt=0;
      for (size_t j=0; j < ag.GetPivot().AttachedSiteCount(); j++) {
        TCAtom &a = ag.GetPivot().GetAttachedAtom(j);
        if (a.IsDeleted() || a.GetType().z < 2)
          continue;
        if (TAfixGroup::HasImplicitPivot(a.GetAfix())) {
          continue;
        }
        if (part == 0 || a.GetPart() == 0 || a.GetPart() == part) {
          if (XElementLib::IsMetal(a.GetType()))
            metal_cnt++;
          attached_cnt++;
        }
      }
      bool valid = true;
      int m = ag.GetM();
      switch (m) {
      case 1: // XYZC-H
        valid = (attached_cnt == 3) || (attached_cnt-metal_cnt == 3);
        break;
      case 2: // XYC-H2
      case 4: // XYC-H
        valid = (attached_cnt == 2) || (attached_cnt-metal_cnt == 2);
        break;
      case 3: // X-H3
      case 8: // O-H
      case 9: // X=C-H2
      case 12: // X-H3 - disordered
      case 13: // X-H3
      case 14: // O-H
      case 16: // CC-H
        valid = (attached_cnt == 1) || (attached_cnt-metal_cnt == 1);
        break;
      case 15: // {R4/5}B-H
        valid = (attached_cnt == 4 || attached_cnt == 5) || (
          (attached_cnt-metal_cnt == 4) || (attached_cnt-metal_cnt == 5));
        break;
      }
      if (!valid) {
        for (size_t gi=0; gi < ag.Count(); gi++) {
          deleted.Add(ag[gi])->SetDeleted(true);
        }
        if (report) {
          TBasicApp::NewLogEntry(logError) << "Pivot atom " <<
            ag.GetPivot().GetLabel() << " has wrong connectivity for the given "
            "AFIX group and the group was removed. Please revise your model.";
          TBasicApp::NewLogEntry() << ag.GetPivot().GetLabel() << ", AFIX " <<
            ag.GetAfix() << ", attached atoms " << attached_cnt <<
            ", attached metals " << metal_cnt;
        }
        ag.Clear();
      }
    }
    if (deleted.IsEmpty()) return NULL;
    if (deleted.Count() == deleted_cnt)
      break;
  }
  TDeleteUndo *du = new TDeleteUndo(
    UndoAction::New(this, &TLattice::undoDelete));
  for (size_t i=0; i < deleted.Count(); i++) {
    TSAtomPList dl = Objects.atomRegistry.FindAll(*deleted[i]);
    du->SAtomIds.SetCapacity(du->SAtomIds.Count()+dl.Count());
    for (size_t j=0; j < dl.Count(); j++)
      du->AddSAtom(*dl[j]);
  }
  if (reinit) {
    Init();
  }
  return du;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TLattice::LibGetFragmentCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(olxstr(FragmentCount()));
}
//..............................................................................
void TLattice::LibGetMoiety(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(CalcMoiety());
}
//..............................................................................
void TLattice::LibGetFragmentAtoms(const TStrObjList& Params, TMacroError& E)  {
  size_t index = Params[0].ToSizeT();
  if( index >= FragmentCount() )
    throw TIndexOutOfRangeException(__OlxSourceInfo, index, 0, FragmentCount());
  olxstr rv;
  for( size_t i=0; i < Fragments[index]->NodeCount(); i++ )  {
    rv << Fragments[index]->Node(i).GetLabel();
    if( (i+1) < Fragments[index]->NodeCount() )
      rv << ',';
  }
  E.SetRetVal( rv );
}
//..............................................................................
void TLattice::LibIsGrown(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(IsGenerated());
}
//..............................................................................
TLibrary*  TLattice::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("latt") : name);
  lib->Register(
    new TFunction<TLattice>(this,  &TLattice::LibGetFragmentCount,
    "GetFragmentCount", fpNone,
    "Returns number of fragments in the lattice")
  );
  lib->Register(
    new TFunction<TLattice>(this,  &TLattice::LibGetFragmentAtoms,
    "GetFragmentAtoms", fpOne,
    "Returns a comma separated list of atoms in specified fragment")
  );
  lib->Register(
    new TFunction<TLattice>(this,  &TLattice::LibGetMoiety,
    "GetMoiety", fpNone,
    "Returns molecular moiety")
  );
  lib->Register(
    new TFunction<TLattice>(this,  &TLattice::LibIsGrown,
    "IsGrown", fpNone,
    "Returns true if the structure is grow")
  );
  return lib;
}

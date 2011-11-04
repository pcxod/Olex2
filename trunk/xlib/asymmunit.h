/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_asymmunit_H
#define __olx_xl_asymmunit_H
#include "xbase.h"
#include "ematrix.h"
#include "threex3.h"
#include "macroerror.h"
#include "srestraint.h"
#include "ellipsoid.h"
#include "samegroup.h"
#include "dataitem.h"
#include "conninfo.h"

#undef GetObject

BeginXlibNamespace()

class TResidue;

class TAsymmUnit: public IXVarReferencerContainer, public IEObject  {
  TCAtomPList CAtoms;
  // list of unique SG matrices (no centering or inversion)
  smatd_list  Matrices;
  TEllpPList Ellipsoids;
  mat3d
    // transformation from cell crd to cartesian
    Cell2Cartesian, Cell2CartesianT,
    // transformation from cartesian crd to cell
    Cartesian2Cell, Cartesian2CellT;
  // ADP transformation matrices
  mat3d UcifToUxyz,  UxyzToUcif,
        UcifToUxyzT, UxyzToUcifT;
  // transformation from Miller index to cartesian
  mat3d Hkl2Cartesian;
  TCAtomPList Centroids;
  double MaxQPeak,
         MinQPeak;
  unsigned short Z;
  short Latt;
  /* this flag specifies that _OnAtomTypeChange will do nothing, however
  whatever called Assign must call _UpdateConnInfo
  */
  bool Assigning;
  class TLattice* Lattice;  // parent lattice
  vec3d Axes, AxisEsds;     // axes and esds
  vec3d Angles, AngleEsds;  // angles and esds
  vec3d RAxes;              // reciprical axes
  vec3d RAngles;            // reciprical angles
protected:
  TActionQList Actions;
  TTypeListExt<TResidue, IEObject> Residues;
  TResidue& MainResidue;
  class RefinementModel* RefMod;
  static const olxstr IdName;
  void _UpdateQPeaks();
public:

  TAsymmUnit(TLattice* L);
  virtual ~TAsymmUnit();

  inline TLattice& GetLattice() const {  return *Lattice;  }
  inline vec3d& GetAxes()  {  return Axes;  }
  inline vec3d& GetAxisEsds()  {  return AxisEsds;  }
  inline vec3d& GetAngles()  {  return Angles;  }
  inline vec3d& GetAngleEsds()  {  return AngleEsds;  }
  inline const vec3d& GetAxes() const {  return Axes;  }
  inline const vec3d& GetAxisEsds() const {  return AxisEsds;  }
  inline const vec3d& GetAngles() const {  return Angles;  }
  inline const vec3d& GetAngleEsds() const {  return AngleEsds;  }
  inline const vec3d& GetRAxes() const {  return RAxes;  }
  inline const vec3d& GetRAngles() const {  return RAngles;  }
  double CalcCellVolume() const;
  /* estimates Z=Z'*sg.multiplicity according to 18.6A rule, partial occupancy
  implied double...
  */
  double EstimateZ(double atomCount) const;
  double GetZPrime() const;
  DefPropP(short, Z)
  DefPropP(short, Latt)

  const mat3d& GetCellToCartesian() const {  return Cell2Cartesian; }
  const mat3d& GetCartesianToCell() const {  return Cartesian2Cell; }
  const mat3d& GetHklToCartesian() const {  return Hkl2Cartesian; }
  template <typename VC, typename VC1>
  inline VC& CellToCartesian(const VC& cell, VC1& crt) const {
    crt[0] = cell[0]*Cell2Cartesian[0][0] + cell[1]*Cell2Cartesian[1][0] +
      cell[2]*Cell2Cartesian[2][0];
    crt[1] = cell[1]*Cell2Cartesian[1][1] + cell[2]*Cell2Cartesian[2][1];
    crt[2] = cell[2]*Cell2Cartesian[2][2];
    return crt;
    //Cartesian = Cell * Cell2Cartesian;
  }
  template <class VC> inline VC& CellToCartesian(VC& crt) const {
    crt[0] = crt[0]*Cell2Cartesian[0][0] + crt[1]*Cell2Cartesian[1][0] +
      crt[2]*Cell2Cartesian[2][0];
    crt[1] = crt[1]*Cell2Cartesian[1][1] + crt[2]*Cell2Cartesian[2][1];
    crt[2] = crt[2]*Cell2Cartesian[2][2];
    return crt;
    //crt *= Cell2Cartesian;
  }
  template <class VC> inline VC Orthogonalise(const VC& crt) const {
    return VC(
      crt[0]*Cell2Cartesian[0][0] + crt[1]*Cell2Cartesian[1][0] +
        crt[2]*Cell2Cartesian[2][0],
      crt[1]*Cell2Cartesian[1][1] + crt[2]*Cell2Cartesian[2][1],
      crt[2]*Cell2Cartesian[2][2]);
  }
  template <class VC> inline VC& CartesianToCell(VC& cll) const {
    cll[0] = cll[0]*Cartesian2Cell[0][0] + cll[1]*Cartesian2Cell[1][0] +
      cll[2]*Cartesian2Cell[2][0];
    cll[1] = cll[1]*Cartesian2Cell[1][1] + cll[2]*Cartesian2Cell[2][1];
    cll[2] = cll[2]*Cartesian2Cell[2][2];
    return cll;
  }
  template <class VC> inline VC Fractionalise(const VC& cll) const {
    return VC(
      cll[0]*Cartesian2Cell[0][0] + cll[1]*Cartesian2Cell[1][0] +
        cll[2]*Cartesian2Cell[2][0],
      cll[1]*Cartesian2Cell[1][1] + cll[2]*Cartesian2Cell[2][1],
      cll[2]*Cartesian2Cell[2][2]);
  }

  // J App Cryst 2002, 35, 477-480
  template <class T> T& UcifToUcart(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = UcifToUxyz*M*UcifToUxyzT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  template <class T> T& UcartToUcif(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = UxyzToUcif*M*UxyzToUcifT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  template <class T> T& UstarToUcart(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = Cell2Cartesian*M*Cell2CartesianT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  template <class T> T& UcartToUstar(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = Cartesian2Cell*M*Cartesian2CellT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  /* copies the atoms from another AU, _UpdateConnInfo must be called after
  this
  */
  void Assign(const TAsymmUnit& C);
  void ChangeSpaceGroup(const class TSpaceGroup& sg);
  // executed from the above function, Data is the new space group
  TActionQueue& OnSGChange;
  /* initialises transofrmation matrices, called after axis and angles
  initialised
  */
  void InitMatrices();
  /* initialises data such as Q-peak heights, called after all atoms
  initialised
  */
  void InitData();
  
  void Clear();
  //creates a new residue
  TResidue& NewResidue(const olxstr& RClass, int number,
    const olxstr& alias=EmptyString());
  inline size_t ResidueCount() const {  return Residues.Count()+1;  }
  inline TResidue& GetResidue(size_t i) const {
    return (i==0) ? const_cast<TAsymmUnit*>(this)->MainResidue : Residues[i-1];
  }
  TResidue* NextResidue(const TResidue& r) const;
  TResidue* PrevResidue(const TResidue& r) const;
  void AssignResidues(const TAsymmUnit& au);
  // changes the atom order as in residues
  void ComplyToResidues();
  // if a number is provided, seraches by Number otherwise - by ClassName
  void FindResidues(const olxstr& resi, TPtrList<TResidue>& list);
  // this is called internally by the TCAtom, to sync connectivity info
  void _OnAtomTypeChanged(TCAtom& caller);
  // called by the ref model
  void _UpdateConnInfo();
  //creates a new atom and puts it into the list
  TCAtom& NewAtom(TResidue* resi = NULL);
  //creates a new atom and puts it into the list
  TCAtom& NewCentroid(const vec3d &CCenter);
  /* returns an atom by label; if the label is not unique, returns the first
  found
  */
  TCAtom* FindCAtom(const olxstr &Label, TResidue* resi = NULL) const;
  //returns an atom by LoaderId
  TCAtom* FindCAtomById(size_t id) const  {
    return (id >= CAtoms.Count()) ? NULL : CAtoms[id];
  }
  // makes specified type detached or attached
  void DetachAtomType(short type, bool detach);
  /* removes all atoms marked as deleted */
  void PackAtoms();
  inline TCAtom& GetAtom(size_t i) const {  return *CAtoms[i];  }
  const TCAtomPList& GetAtoms() const {  return CAtoms;  }
  TCAtomPList& GetAtoms() {  return CAtoms;  }
  inline size_t AtomCount() const { return CAtoms.Count();  }

  inline size_t MatrixCount() const {  return Matrices.Count();  }
  inline const smatd& GetMatrix(size_t i) const {  return Matrices[i];  }
  void ClearMatrices()  {  Matrices.Clear();  }
  void AddMatrix(const smatd& a);

  inline size_t EllpCount() const {  return Ellipsoids.Count();  }
  inline TEllipsoid& GetEllp(size_t i) const {  return *Ellipsoids[i];  }
  void NullEllp(size_t i);
  void ClearEllps();
  void PackEllps();
  // Q - six values representing quadratic form of a thermal ellipsoid
  TEllipsoid& NewEllp();  // initialisation performed manually !

  vec3d GetOCenter(bool IncludeQ, bool IncludeH) const;
  // returns properly sorted content list
  ContentList GetContentList(double mult=1.0) const;
  /* returns summarised formula of the asymmetric unit, use MutiplyZ to
  multiply the content by Z
  */
  olxstr SummFormula(const olxstr& sep, bool MultiplyZ=true) const;
  olxstr _SummFormula(const olxstr& sep, double mult) const;
  /* returns molecular weight of the asymmetric unit */
  double MolWeight() const;
  size_t CountElements(const olxstr& Symbol) const;

  // sorts the content of the asymmetric unit or the list if provided
  void Sort(TCAtomPList* list = NULL);

  olxstr CheckLabel(const TCAtom* ca, const olxstr &Label,
    char a='0', char b='a', char c='a') const;

  bool IsQPeakMinMaxInitialised() const {  return MaxQPeak != -1000;  }
  inline double GetMaxQPeak() const {  return MaxQPeak;  }
  inline double GetMinQPeak() const {  return MinQPeak;  }
  /* atoms should have at least three atoms for fitting. If the atoms.atom is
  NULL, atoms.element must be provided, if atoms.bool is false, the atom is not
  used in the fitting. The missing atoms will be initialised on successful
  completion of the procedure. try_invert - try the fitting of the inverted set
  of coordinates
  */
  void FitAtoms(
    TTypeList<AnAssociation3<TCAtom*, const cm_Element*, bool> >& atoms,
    const vec3d_list& crds, bool try_invert);
  // returns next available positive/negative part
  int GetNextPart(bool negative=false) const;

  DefPropP(RefinementModel*, RefMod)
// IXVarReferencerContainer implementation
  static const olxstr& _GetIdName() {  return IdName;  }
  
  virtual olxstr GetIdName() const {  return IdName;  }
  // note - possibly unsafe, type is not checked
  virtual size_t GetIdOf(const IXVarReferencer& vr) const {  
    if( !EsdlInstanceOf(vr, TCAtom) )
      throw TInvalidArgumentException(__OlxSourceInfo, "referencer");
    return ((TCAtom&)vr).GetId();
  }
  virtual size_t GetPersistentIdOf(const IXVarReferencer& vr) const {  
    if( !EsdlInstanceOf(vr, TCAtom) )
      throw TInvalidArgumentException(__OlxSourceInfo, "referencer");
    return ((TCAtom&)vr).GetTag();
  }
  // note - possibly unsafe, range is unchecked
  virtual IXVarReferencer& GetReferencer(size_t id) const {
    if( id >= CAtoms.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "id");
    return *CAtoms[id];
  }
  virtual size_t ReferencerCount() const {  return CAtoms.Count();  }
//
  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, bool export_conn);
#endif
  void FromDataItem(TDataItem& item);

  void LibNewAtom(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomCount(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomCrd(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomU(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomUiso(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomOccu(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomName(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomType(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomAfix(const TStrObjList& Params, TMacroError& E);
  void LibGetPeak(const TStrObjList& Params, TMacroError& E);
  void LibGetCell(const TStrObjList& Params, TMacroError& E);
  void LibGetVolume(const TStrObjList& Params, TMacroError& E);
  void LibGetCellVolume(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomCrd(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomLabel(const TStrObjList& Params, TMacroError& E);
  // retruns label if +-num is used
  void LibIsAtomDeleted(const TStrObjList& Params, TMacroError& E);
  void LibIsPeak(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomLabel(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomU(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomOccu(const TStrObjList& Params, TMacroError& E);
  void LibGetSymm(const TStrObjList& Params, TMacroError& E);
  void LibGetZ(const TStrObjList& Params, TMacroError& E);
  void LibSetZ(const TStrObjList& Params, TMacroError& E);
  void LibGetZprime(const TStrObjList& Params, TMacroError& E);
  void LibSetZprime(const TStrObjList& Params, TMacroError& E);
  void LibFormula(const TStrObjList& Params, TMacroError& E);
  void LibWeight(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString());
};

EndXlibNamespace()
#endif


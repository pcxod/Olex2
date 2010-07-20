//---------------------------------------------------------------------------//
// namespace TXClasses: TCAtom - basic crystalographic atom
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "catom.h"
#include "ellipsoid.h"
#include "exception.h"
#include "estrlist.h"
#include "refmodel.h"
#include "pers_util.h"
#include "residue.h"

olxstr TCAtom::VarNames[] = {"Scale", "X", "Y", "Z", "Sof", "Uiso", "U11", "U22", "U33", "U23", "U13", "U12"};
//----------------------------------------------------------------------------//
// TCAtom function bodies
//----------------------------------------------------------------------------//
TCAtom::TCAtom(TAsymmUnit *Parent)  {
  Part   = 0;
  Occu   = 1;
  QPeak  = 0;
  SetId(0);
  SetResiId(0);  // default residue is unnamed one
  SetSameId(~0);
  FParent = Parent;
  EllpId = ~0;
  Uiso = caDefIso;
  OccuEsd = UisoEsd = 0;
  UisoScale = 0;
  UisoOwner = NULL;
  FragmentId = ~0;
  FAttachedAtoms = NULL;
  FAttachedAtomsI = NULL;
  Equivs = NULL;
  Type = NULL;
  SetTag(-1);
  DependentAfixGroup = ParentAfixGroup = NULL;
  DependentHfixGroups = NULL;
  ExyzGroup = NULL;
  Flags = 0;
  ConnInfo = NULL;
  memset(Vars, 0, sizeof(Vars));
}
//..............................................................................
TCAtom::~TCAtom()  {
  if( FAttachedAtoms != NULL )       delete FAttachedAtoms;
  if( FAttachedAtomsI != NULL )      delete FAttachedAtomsI;
  if( DependentHfixGroups != NULL )  delete DependentHfixGroups; 
  if( ConnInfo != NULL )             delete ConnInfo;
  if( Equivs != NULL )               delete Equivs;
}
//..............................................................................
void TCAtom::SetConnInfo(CXConnInfo& ci) {
  if( ConnInfo != NULL )
    delete ConnInfo;
  ConnInfo = &ci;
}
//..............................................................................
void TCAtom::SetLabel(const olxstr& L, bool validate)  {
  if( validate )  {
    if( L.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty label");
    cm_Element *atype = XElementLib::FindBySymbolEx(L);
    if( atype == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Unknown element: '") << L << '\'' );
    if( Type != atype )  {
      if( Type != NULL && *Type == iQPeakZ )
        SetQPeak(0);
      Type = atype;
      FParent->_OnAtomTypeChanged(*this);
    }
    Label = L;
    if( Type->symbol.Length() == 2 )
      Label[1] = Label.o_tolower(Label.CharAt(1));
  }
  else
    Label = L;
}
//..............................................................................
void TCAtom::SetType(const cm_Element& t)  {
  if( Type != &t )  {
    if( Type != NULL && *Type == iQPeakZ )
      SetQPeak(0);
    Type = &t;
    FParent->_OnAtomTypeChanged(*this);
  }
}
//..............................................................................
void TCAtom::AssignEquivs(const TCAtom& S)  {
  if( S.Equivs != NULL )  {
    if( Equivs == NULL )
      Equivs = new smatd_list(*S.Equivs);
    else
      *Equivs = *S.Equivs;
  }
  else if( Equivs != NULL )
    delete Equivs;
}
//..............................................................................
void TCAtom::ClearEquivs()  {  
  if(Equivs != NULL )  {
    delete Equivs;
    Equivs = NULL;
  }
}
//..............................................................................
void TCAtom::Assign(const TCAtom& S)  {
  DependentAfixGroup = ParentAfixGroup = NULL;  // managed by the group
  if( DependentHfixGroups != NULL )  {
    delete DependentHfixGroups;
    DependentHfixGroups = NULL;
  }
  ExyzGroup = NULL;  // also managed by the group
  SetPart(S.GetPart());
  SetOccu(S.GetOccu());
  SetOccuEsd(S.GetOccuEsd());
  SetQPeak(S.GetQPeak());
  SetResiId(S.GetResiId());
  SetSameId(S.GetSameId());
  EllpId = S.EllpId;
  SetUiso(S.GetUiso());
  SetUisoEsd(S.GetUisoEsd());
  SetUisoScale( S.GetUisoScale() );
  if( S.UisoOwner != NULL )  {
    UisoOwner = FParent->FindCAtomById(S.UisoOwner->GetId());
    if( UisoOwner == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  }
  else
    UisoOwner = NULL;
  Label   = S.Label;
  if( Type != &S.GetType() )  {
    Type = &S.GetType();
    FParent->_OnAtomTypeChanged(*this);
  }
//  Frag    = S.Frag;
  //Id = S.GetId();
  FragmentId = S.GetFragmentId();
  Center = S.Center;
  Esd = S.Esd;
  AssignEquivs(S);
  Flags = S.Flags;
}
//..............................................................................
int TCAtom::GetAfix() const {
  if( ParentAfixGroup == NULL )  {
    if( DependentAfixGroup != NULL && (DependentAfixGroup->IsFitted() || DependentAfixGroup->GetM() == 0) )
      return DependentAfixGroup->GetAfix();
    //if( DependentHfixGroup != NULL && !DependentHfixGroup->IsRiding() )
    //  return DependentHfixGroup->GetAfix();
    return 0;
  }
  if( ParentAfixGroup->HasExcplicitPivot() )
    return (ParentAfixGroup->GetAfix()/10)*10 + 5;
  else
    return ParentAfixGroup->GetAfix();
}
//..............................................................................
TEllipsoid* TCAtom::GetEllipsoid() const {  return EllpId == InvalidIndex ? NULL : &FParent->GetEllp(EllpId);  }
//..............................................................................
void TCAtom::AssignEllp(TEllipsoid* NV) {  NV == NULL ? EllpId = InvalidIndex : EllpId = NV->GetId();  }
//..............................................................................
void TCAtom::UpdateEllp(const TEllipsoid &NV ) {
  double Q[6], E[6];
  NV.GetQuad(Q, E);
  if( EllpId == InvalidIndex )  {
    TEllipsoid& elp = FParent->NewEllp();
    elp.Initialise(Q, E);
    EllpId = elp.GetId();
  }
  else
    FParent->GetEllp(EllpId).Initialise(Q);
}
//..............................................................................
void TCAtom::ToDataItem(TDataItem& item) const  {
  item.AddField("label", Label);
  item.AddField("type", Type->symbol);
  item.AddField("part", (int)Part);
  item.AddField("sof", TEValue<double>(Occu, OccuEsd).ToString());
  item.AddField("flags", Flags);
  item.AddField("x", TEValue<double>(Center[0], Esd[0]).ToString());
  item.AddField("y", TEValue<double>(Center[1], Esd[1]).ToString());
  item.AddField("z", TEValue<double>(Center[2], Esd[2]).ToString());
  if( !olx_is_valid_index(EllpId) )  {
    item.AddField("Uiso", TEValue<double>(Uiso, UisoEsd).ToString());
    if( UisoOwner != NULL && !UisoOwner->IsDeleted() )  {
      TDataItem& uo = item.AddItem("Uowner");
      uo.AddField("id", UisoOwner->GetTag());
      uo.AddField("k", UisoScale);
    }
  }
  else {
    double Q[6], E[6];
    GetEllipsoid()->GetQuad(Q, E);
    TDataItem& elp = item.AddItem("adp");
    elp.AddField("xx", TEValue<double>(Q[0], E[0]).ToString());
    elp.AddField("yy", TEValue<double>(Q[1], E[1]).ToString());
    elp.AddField("zz", TEValue<double>(Q[2], E[2]).ToString());
    elp.AddField("yz", TEValue<double>(Q[3], E[3]).ToString());
    elp.AddField("xz", TEValue<double>(Q[4], E[4]).ToString());
    elp.AddField("xy", TEValue<double>(Q[5], E[5]).ToString());
  }
  if( *Type == iQPeakZ )
    item.AddField("peak", QPeak);
}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TCAtom::PyExport()  {
  PyObject* main = PyDict_New();
  PythonExt::SetDictItem(main, "label", PythonExt::BuildString(Label));
  PythonExt::SetDictItem(main, "type", PythonExt::BuildString(Type->symbol));
  PythonExt::SetDictItem(main, "part", Py_BuildValue("i", Part));
  PythonExt::SetDictItem(main, "occu", Py_BuildValue("(dd)", Occu, OccuEsd));
  PythonExt::SetDictItem(main, "tag", Py_BuildValue("i", GetTag()));
  PythonExt::SetDictItem(main, "crd", 
    Py_BuildValue("(ddd)(ddd)", Center[0], Center[1], Center[2], Esd[0], Esd[1], Esd[2]));
  if( !olx_is_valid_index(EllpId) )  {
    PythonExt::SetDictItem(main, "uiso", Py_BuildValue("(dd)", Uiso, UisoEsd));
    if( UisoOwner != NULL && !UisoOwner->IsDeleted() )  {
      PyObject* uo = PyDict_New();
      PythonExt::SetDictItem(uo, "id", Py_BuildValue("i", UisoOwner->GetTag())) ;
      PythonExt::SetDictItem(uo, "k", Py_BuildValue("d", UisoScale)) ;
    }
  }
  else  {
    double Q[6], E[6];
    GetEllipsoid()->GetQuad(Q, E);
    PythonExt::SetDictItem(main, "adp", 
      Py_BuildValue("(dddddd)(dddddd)", Q[0], Q[1], Q[2], Q[3], Q[4], Q[5], 
       E[0], E[1], E[2], E[3], E[4], E[5]
      ) );
  }
  if( *Type == iQPeakZ )
    PythonExt::SetDictItem(main, "peak", Py_BuildValue("d", QPeak));
  return main;
}
#endif
//..............................................................................
void TCAtom::FromDataItem(TDataItem& item)  {
  Type = XElementLib::FindBySymbol(item.GetRequiredField("type"));
  if( Type == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid atom type");
  TEValue<double> ev;
  Label = item.GetRequiredField("label");
  Part = item.GetRequiredField("part").ToInt();
  ev = item.GetRequiredField("sof");
  Occu = ev.GetV();  OccuEsd = ev.GetE();
  Flags = item.GetRequiredField("flags").ToInt();
  ev = item.GetRequiredField("x");
  Center[0] = ev.GetV();  Esd[0] = ev.GetE();
  ev = item.GetRequiredField("y");
  Center[1] = ev.GetV();  Esd[1] = ev.GetE();
  ev = item.GetRequiredField("z");
  Center[2] = ev.GetV();  Esd[2] = ev.GetE();

  TDataItem* adp = item.FindItem("adp");
  if( adp != NULL )  {
    double Q[6], E[6];
    if( adp->FieldCount() != 6 )
      throw TInvalidArgumentException(__OlxSourceInfo, "6 parameters expected for the ADP");
    for( int i=0; i < 6; i++ )  {
      ev = adp->GetField(i);
      E[i] = ev.GetE();  Q[i] = ev.GetV();
    }
    EllpId = FParent->NewEllp().Initialise(Q,E).GetId();
    Uiso = GetEllipsoid()->GetUiso();
  }
  else  {
    EllpId = InvalidIndex;
    ev = item.GetRequiredField("Uiso");
    Uiso = ev.GetV();  UisoEsd = ev.GetE();
    TDataItem* uo = item.FindItem("Uowner");
    if( uo != NULL )  {
      UisoOwner = &GetParent()->GetAtom(uo->GetRequiredField("id").ToSizeT());
      UisoScale = uo->GetRequiredField("k").ToDouble();
    }
  }
  if( *Type == iQPeakZ )
    QPeak = item.GetRequiredField("peak").ToDouble();
}
//..............................................................................
void DigitStrtok(const olxstr &str, TStrPObjList<olxstr,bool>& chars)  {
  olxstr Dig, Char;
  for( size_t i=0; i < str.Length(); i++ )  {
    if( str[i] <= '9' && str[i] >= '0' )  {
      if( !Char.IsEmpty() )      {
        chars.Add(Char, true);
        Char = EmptyString;
      }
      Dig << str[i];
    }
    else  {
      if( !Dig.IsEmpty() )  {
        chars.Add(Dig, false);
        Dig = EmptyString;
      }
      Char << str[i];
    }
  }
  if( !Char.IsEmpty() )  chars.Add(Char, true);
  if( !Dig.IsEmpty() )   chars.Add(Dig, false);
}
//..............................................................................
int TCAtom::CompareAtomLabels(const olxstr& S, const olxstr& S1)  {
  TStrPObjList<olxstr, bool> Chars1, Chars2;

  DigitStrtok(S, Chars1);
  DigitStrtok(S1, Chars2);
  for( size_t i=0; i < olx_min(Chars1.Count(), Chars2.Count()); i++ )  {
    if( Chars1.GetObject(i) && Chars2.GetObject(i) )  {
      int res = Chars1[i].Comparei(Chars2[i]);
      if( res != 0 )  return res;
    }
    if( !Chars1.GetObject(i) && !Chars2.GetObject(i) )  {
      int res = Chars1[i].ToInt() - Chars2[i].ToInt();
      //if( !res )  // to tackle 01 < 1
      //{  res = Chars1->String(i).Length() - Chars2->String(i).Length();  }
      //the following commented line allows sorting like C01 C02 and C01A then
      //but though it looks better, it is not correct, so using C01 C01A C02
      //if( res && (Chars1->Count() == Chars2->Count()))  return res;
      if( res != 0 )  return res;
    }

    if( !Chars1.GetObject(i) && Chars2.GetObject(i) )  return 1;
    if( Chars1.GetObject(i) && !Chars2.GetObject(i) )  return -1;
  }
  return olx_cmp_size_t(Chars1.Count(), Chars2.Count());
}
//..............................................................................
void TCAtom::AttachAtom(TCAtom *CA)  {
  if( FAttachedAtoms == NULL )  FAttachedAtoms = new TCAtomPList;
  FAttachedAtoms->Add(CA);
}
//..............................................................................
void TCAtom::AttachAtomI(TCAtom *CA)  {
  if( !FAttachedAtomsI )  FAttachedAtomsI = new TCAtomPList;
  FAttachedAtomsI->Add(CA);
}
//..............................................................................
olxstr TCAtom::GetResiLabel() const {
  if( GetResiId() == 0 )
    return GetLabel();
  return (olxstr(GetLabel()) << '_' << GetParent()->GetResidue(GetResiId()).GetNumber());
}
//..............................................................................
//..............................................................................
//..............................................................................
olxstr TGroupCAtom::GetFullLabel(RefinementModel& rm) const  {
  olxstr name(Atom->GetLabel());
  if( Atom->GetResiId() == 0 )  {
    if( Matrix != NULL )
      name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
  }
  else  {  // it is however shown that shelx just IGNORES $EQIV in this notation...
    name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
    if( Matrix != NULL )
      name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
  }
  return name;
}
//..............................................................................
olxstr TGroupCAtom::GetFullLabel(RefinementModel& rm, const int resiId) const  {
  olxstr name(Atom->GetLabel());
  if( Atom->GetResiId() == 0 || 
    Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber() == resiId )  
  {
    if( Matrix != NULL )
      name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
  }
  else  {  // it is however shown that shelx just IGNORES $EQIV in this notation...
    name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
    if( Matrix != NULL )
      name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
  }
  return name;
}
//..............................................................................
olxstr TGroupCAtom::GetFullLabel(RefinementModel& rm, const olxstr& resiName) const  {
  if( resiName.IsEmpty() )
    return GetFullLabel(rm);

  olxstr name(Atom->GetLabel());
  if( resiName.IsNumber() )  {
    if( Atom->GetResiId() == 0 || 
      (Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber() == resiName.ToInt()) )  
    {
      if( Matrix != NULL )
        name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
    }
    else  {  // it is however shown that shelx just IGNORES $EQIV in this notation...
      name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
      if( Matrix != NULL )
        name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
    }
  }
  else  {
    if( !olx_is_valid_index(Atom->GetResiId()) || 
      (!resiName.IsEmpty() && Atom->GetParent()->GetResidue(Atom->GetResiId()).GetClassName().Equalsi(resiName)) )  
    {
      if( Matrix != NULL )
        name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
    }
    else  {  // it is however shown that shelx just IGNORES $EQIV in this notation...
      name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
      if( Matrix != NULL )
        name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
    }
  }
  return name;
}
//..............................................................................
IXVarReferencerContainer& TCAtom::GetParentContainer() const {  return *FParent;  }
//..............................................................................
double TCAtom::GetValue(size_t var_index) const {
  switch( var_index)  {
    case catom_var_name_X:     return Center[0];
    case catom_var_name_Y:     return Center[1];
    case catom_var_name_Z:     return Center[2];
    case catom_var_name_Sof:   return Occu;
    case catom_var_name_Uiso:  return Uiso;  
    case catom_var_name_U11:
    case catom_var_name_U22:
    case catom_var_name_U33:
    case catom_var_name_U23:
    case catom_var_name_U13:
    case catom_var_name_U12:
      if( !olx_is_valid_index(EllpId) )
        throw TInvalidArgumentException(__OlxSourceInfo, "Uanis is not defined");
      return FParent->GetEllp(EllpId).GetQuadVal(var_index-catom_var_name_U11);
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
}
//..............................................................................
void TCAtom::SetValue(size_t var_index, const double& val) {
  switch( var_index)  {
    case catom_var_name_X:     Center[0] = val;  break;
    case catom_var_name_Y:     Center[1] = val;  break;
    case catom_var_name_Z:     Center[2] = val;  break;
    case catom_var_name_Sof:   Occu = val;  break;
    case catom_var_name_Uiso:  Uiso = val;  break;  
    case catom_var_name_U11:
    case catom_var_name_U22:
    case catom_var_name_U33:
    case catom_var_name_U23:
    case catom_var_name_U13:
    case catom_var_name_U12:
      break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
}

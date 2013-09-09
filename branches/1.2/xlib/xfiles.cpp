/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xfiles.h"
#include "efile.h"
#include "xapp.h"
#include "unitcell.h"
#include "catom.h"
#include "library.h"
#include "ins.h"
#include "crs.h"
#include "cif.h"
#include "hkl.h"
#include "utf8file.h"
#include "atomsort.h"
#include "infotab.h"
#include "absorpc.h"
#include "analysis.h"
#include "estopwatch.h"

enum {
  XFILE_SG_Change,
  XFILE_UNIQ
};

TBasicCFile::TBasicCFile()
  : RefMod(AsymmUnit), AsymmUnit(NULL)
{
  AsymmUnit.SetRefMod(&RefMod);
}
//..............................................................................
TBasicCFile::~TBasicCFile()  {
  /* this must be called, as the AU might get destroyed beforehand and then
  AfixGroups cause crash
  */
  RefMod.Clear(rm_clear_ALL);
}
//..............................................................................
void TBasicCFile::SaveToFile(const olxstr& fn)  {
  TStrList L;
  SaveToStrings(L);
  TUtf8File::WriteLines(fn, L, false);
  FileName = fn;
};
//..............................................................................
void TBasicCFile::PostLoad() {
  /* fix labels for not native formats, will not help for FE1A, because it
  could come from Fe1A or from Fe1a ...
  */
  if (!IsNative()) {
    for (size_t i=0; i < AsymmUnit.AtomCount(); i++) {
      TCAtom& a = AsymmUnit.GetAtom(i);
      if (a.GetType().symbol.Length() == 2 &&
          a.GetLabel().StartsFromi(a.GetType().symbol))
      {
        a.SetLabel(a.GetType().symbol +
          a.GetLabel().SubStringFrom(a.GetType().symbol.Length()), false);
      }
    }
  }
}
//..............................................................................
void TBasicCFile::LoadFromStream(IInputStream &is, const olxstr& nameToken) {
  TStrList lines;
  lines.LoadFromTextStream(is);
  LoadStrings(lines, nameToken);
}
//..............................................................................
void TBasicCFile::LoadStrings(const TStrList &lines, const olxstr &nameToken) {
  FileName.SetLength(0);
  Title.SetLength(0);
  TXFile::NameArg file_n(nameToken);
  if (lines.IsEmpty())
    throw TInvalidArgumentException(__OlxSourceInfo, "empty content");
  try  {
    LoadFromStrings(lines);
    if (EsdlInstanceOf(*this, TCif)) {
      if (!file_n.data_name.IsEmpty()) {
        if (file_n.is_index)
          ((TCif*)this)->SetCurrentBlock(file_n.data_name.ToSizeT());
        else
          ((TCif*)this)->SetCurrentBlock(file_n.data_name);
      }
      else {  // set first then
        ((TCif*)this)->SetCurrentBlock(InvalidIndex);
      }
    }
  }
  catch (const TExceptionBase& exc) {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FileName = nameToken;
  PostLoad();
}
//..............................................................................
void TBasicCFile::LoadFromFile(const olxstr& _fn)  {
  TStopWatch(__FUNC__);
  TXFile::NameArg file_n(_fn);
  TEFile::CheckFileExists(__OlxSourceInfo, file_n.file_name);
  TStrList L;
  L.LoadFromFile(file_n.file_name);
  if (L.IsEmpty())
    throw TEmptyFileException(__OlxSourceInfo, _fn);
  try {
    LoadStrings(L, _fn);
  }
  catch (const TExceptionBase& exc) {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FileName = file_n.file_name;
}
//----------------------------------------------------------------------------//
// TXFile function bodies
//----------------------------------------------------------------------------//
TXFile::TXFile(ASObjectProvider& Objects) :
  Lattice(Objects),
  RefMod(Lattice.GetAsymmUnit()),
  OnFileLoad(Actions.New("XFILELOAD")),
  OnFileSave(Actions.New("XFILESAVE")),
  OnFileClose(Actions.New("XFILECLOSE"))
{
  Lattice.GetAsymmUnit().SetRefMod(&RefMod);
  Lattice.GetAsymmUnit().OnSGChange.Add(this, XFILE_SG_Change); 
  Lattice.OnStructureUniq.Add(this, XFILE_UNIQ);
  FLastLoader = NULL;
  FSG = NULL;
}
//..............................................................................
TXFile::~TXFile()  {
// finding uniq objects and deleting them
  for( size_t i=0; i < FileFormats.Count(); i++ )
    FileFormats.GetObject(i)->SetTag(i);
  for( size_t i=0; i < FileFormats.Count(); i++ )
    if( (size_t)FileFormats.GetObject(i)->GetTag() == i )
      delete FileFormats.GetObject(i);
}
//..............................................................................
void TXFile::RegisterFileFormat(TBasicCFile *F, const olxstr &Ext)  {
  if( FileFormats.IndexOf(Ext) != InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "Ext");
  FileFormats.Add(Ext.ToLowerCase(), F);
}
//..............................................................................
TBasicCFile *TXFile::FindFormat(const olxstr &Ext)  {
  const size_t i = FileFormats.IndexOf(Ext.ToLowerCase());
  if( i == InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "unknown file format");
  return FileFormats.GetObject(i);
}
//..............................................................................
void TXFile::LastLoaderChanged() {
  if( FLastLoader == NULL )  {
    GetRM().Clear(rm_clear_ALL);
    GetLattice().Clear(true);
    return;
  }
  FSG = &TSymmLib::GetInstance().FindSG(FLastLoader->GetAsymmUnit());
  OnFileLoad.Enter(this, &FLastLoader->GetFileName());
  GetRM().Clear(rm_clear_ALL);
  GetLattice().Clear(true);
  GetRM().Assign(FLastLoader->GetRM(), true);
  OnFileLoad.Execute(this);
  GetLattice().Init();
  OnFileLoad.Exit(this, &FLastLoader->GetFileName());
}
//..............................................................................
bool TXFile::Dispatch(int MsgId, short MsgSubId, const IEObject* Sender,
  const IEObject* Data, TActionQueue *)
{
  if( MsgId == XFILE_SG_Change )  {
    if( Data == NULL || !EsdlInstanceOf(*Data, TSpaceGroup) )
      throw TInvalidArgumentException(__OlxSourceInfo, "space group");
    FSG = const_cast<TSpaceGroup*>( dynamic_cast<const TSpaceGroup*>(Data) );
  }
  else if( MsgId == XFILE_UNIQ && MsgSubId == msiEnter )  {
    //RefMod.Validate();
    //UpdateAsymmUnit();
    //GetAsymmUnit().PackAtoms();
    //if( !FLastLoader->IsNative() )  {
    //  FLastLoader->GetRM().Validate();
    //  FLastLoader->GetAsymmUnit().PackAtoms();
    //}
  }
  else
    return false;
  return true;
}
//..............................................................................
void TXFile::PostLoad(const olxstr &fn, TBasicCFile *Loader, bool replicated) {
  for (size_t i=0; i < Loader->GetAsymmUnit().AtomCount(); i++) {
    TCAtom &a = Loader->GetAsymmUnit().GetAtom(i);
    if (olx_abs(a.ccrd()[0]) > 127 ||
      olx_abs(a.ccrd()[1]) > 127 ||
      olx_abs(a.ccrd()[2]) > 127)
    {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("atom coordinates for ").quote() << a.GetLabel());
    }
  }
  if( !Loader->IsNative() )  {
    OnFileLoad.Enter(this, &fn);
    try  {
      GetRM().Clear(rm_clear_ALL);
      GetLattice().Clear(true);
      GetRM().Assign(Loader->GetRM(), true);
      OnFileLoad.Execute(this);
      GetLattice().Init();
    }
    catch(const TExceptionBase& exc)  {
      OnFileLoad.Exit(this);
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    OnFileLoad.Exit(this);
  }
  FSG = &TSymmLib::GetInstance().FindSG(Loader->GetAsymmUnit());
  if( replicated )  {
    for( size_t i=0; i < FileFormats.Count(); i++ )
      if( FileFormats.GetObject(i) == FLastLoader )
        FileFormats.GetObject(i) = Loader;
    delete FLastLoader;
  }
  FLastLoader = Loader;
  if( GetRM().GetHKLSource().IsEmpty() ||
     !TEFile::Exists(GetRM().GetHKLSource()) )
  {
    olxstr src = LocateHklFile();
    if( !src.IsEmpty() && !TEFile::Existsi(olxstr(src), src) )
      src.SetLength(0);
    GetRM().SetHKLSource(src);
  }
  TXApp::GetInstance().SetLastSGResult_(EmptyString());
}
//..............................................................................
void TXFile::LoadFromStrings(const TStrList& lines, const olxstr &nameToken) {
  TStopWatch(__FUNC__);
  // this thows an exception if the file format loader does not exist
  const NameArg file_n(nameToken);
  const olxstr ext(TEFile::ExtractFileExt(file_n.file_name));
  TBasicCFile* Loader = FindFormat(ext);
  bool replicated = false;
  if( FLastLoader == Loader )  {
    Loader = (TBasicCFile*)Loader->Replicate();
    replicated = true;
  }
  try  {
    Loader->LoadStrings(lines, nameToken);
  }
  catch( const TExceptionBase& exc )  {
    if( replicated )
      delete Loader;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  PostLoad(EmptyString(), Loader, replicated);
}
//..............................................................................
void TXFile::LoadFromStream(IInputStream& in, const olxstr &nameToken) {
  TStopWatch(__FUNC__);
  // this thows an exception if the file format loader does not exist
  const NameArg file_n(nameToken);
  const olxstr ext(TEFile::ExtractFileExt(file_n.file_name));
  TBasicCFile* Loader = FindFormat(ext);
  bool replicated = false;
  if( FLastLoader == Loader )  {
    Loader = (TBasicCFile*)Loader->Replicate();
    replicated = true;
  }
  try  {
    Loader->LoadFromStream(in, nameToken);
  }
  catch( const TExceptionBase& exc )  {
    if( replicated )
      delete Loader;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  PostLoad(EmptyString(), Loader, replicated);
}
//..............................................................................
void TXFile::LoadFromFile(const olxstr & _fn) {
  TStopWatch(__FUNC__);
  const NameArg file_n(_fn);
  const olxstr ext(TEFile::ExtractFileExt(file_n.file_name));
  // this thows an exception if the file format loader does not exist
  TBasicCFile* Loader = FindFormat(ext);
  bool replicated = false;
  if( FLastLoader == Loader )  {
    Loader = (TBasicCFile*)Loader->Replicate();
    replicated = true;
  }
  try  {
    Loader->LoadFromFile(_fn);
  }
  catch( const TExceptionBase& exc )  {
    if( replicated )
      delete Loader;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  PostLoad(_fn, Loader, replicated);
}
//..............................................................................
void TXFile::UpdateAsymmUnit()  {
  TBasicCFile* LL = FLastLoader;
  if( LL->IsNative() )
    return;
  GetLattice().UpdateAsymmUnit();
  LL->GetAsymmUnit().ClearEllps();
  for( size_t i=0; i < GetAsymmUnit().EllpCount(); i++ )
    LL->GetAsymmUnit().NewEllp() = GetAsymmUnit().GetEllp(i);
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& CA = GetAsymmUnit().GetAtom(i);
    TCAtom& CA1 = LL->GetAsymmUnit().AtomCount() <= i ? 
      LL->GetAsymmUnit().NewAtom() : LL->GetAsymmUnit().GetAtom(i);
    CA1.Assign(CA);
  }
  LL->GetAsymmUnit().AssignResidues(GetAsymmUnit());
  RefMod.Validate();
  ValidateTabs();
  LL->GetRM().Assign(RefMod, false);
  LL->GetAsymmUnit().SetZ(GetAsymmUnit().GetZ());
  LL->GetAsymmUnit().GetAxes() = GetAsymmUnit().GetAxes();
  LL->GetAsymmUnit().GetAxisEsds() = GetAsymmUnit().GetAxisEsds();
  LL->GetAsymmUnit().GetAngles() = GetAsymmUnit().GetAngles();
  LL->GetAsymmUnit().GetAngleEsds() = GetAsymmUnit().GetAngleEsds();
}
//..............................................................................
void TXFile::Sort(const TStrList& ins) {
  if (FLastLoader == NULL) return;
  if (!FLastLoader->IsNative())
    UpdateAsymmUnit();
  TStrList labels;
  TCAtomPList &list = GetAsymmUnit().GetResidue(0).GetAtomList();
  size_t moiety_index = InvalidIndex, h_cnt=0, del_h_cnt = 0, free_h_cnt = 0;
  bool keeph = true;
  for (size_t i=0; i < list.Count(); i++) {
    if (list[i]->GetType() == iHydrogenZ) {
      if (!list[i]->IsDeleted()) {
        h_cnt++;
        if (list[i]->GetParentAfixGroup() == NULL)
          free_h_cnt++;
      }
      else
        del_h_cnt++;
    }
  }
  if (h_cnt == 0 || del_h_cnt != 0) {
    keeph = false;
    if (del_h_cnt != 0 && free_h_cnt != 0) {
      TBasicApp::NewLogEntry(logError) << "Hydrogen atoms, which are not "
        "attached using AFIX will not be kept with pivot atom until the file "
        "is reloaded";
    }
  }
  try {
    AtomSorter::CombiSort cs;
    olxstr sort;
    for (size_t i=0; i < ins.Count(); i++) {
      if (ins[i].CharAt(0) == '+')
        sort << ins[i].SubStringFrom(1);
      else if (ins[i].Equalsi("moiety")) {
        moiety_index = i;
        break;
      }
      else
        labels.Add(ins[i]);
    }
    for (size_t i=0; i < sort.Length(); i++) {
      if (sort.CharAt(i) == 'm')
        cs.sequence.AddNew(&AtomSorter::atom_cmp_Mw);
      else if (sort.CharAt(i) == 'z')
        cs.sequence.AddNew(&AtomSorter::atom_cmp_Z);
      else if (sort.CharAt(i) == 'l')
        cs.sequence.AddNew(&AtomSorter::atom_cmp_Label);
      else if (sort.CharAt(i) == 'p')
        cs.sequence.AddNew(&AtomSorter::atom_cmp_Part);
      else if (sort.CharAt(i) == 'h')
        keeph = false;
      else if (sort.CharAt(i) == 's')
        cs.sequence.AddNew(&AtomSorter::atom_cmp_Suffix);
      else if (sort.CharAt(i) == 'n')
        cs.sequence.AddNew(&AtomSorter::atom_cmp_Number);
    }
    if (!cs.sequence.IsEmpty()) {
      if (!labels.IsEmpty()) {
        for (size_t i=0; i < cs.sequence.Count(); i++)
          cs.sequence[i].AddExceptions(labels);
      }
      AtomSorter::Sort(list, cs);
    }
    labels.Clear();
    if( moiety_index != InvalidIndex )  {
      sort.SetLength(0);
      if( moiety_index+1 < ins.Count() )  {
        for( size_t i=moiety_index+1; i < ins.Count(); i++ )  {
          if( ins[i].CharAt(0) == '+' )
            sort << ins[i].SubStringFrom(1);
          else
            labels.Add(ins[i]);
        }
        for( size_t i=0; i < sort.Length(); i++ )  {
          if( sort.CharAt(i) == 's' )
            MoietySorter::SortBySize(list);
          else if( sort.CharAt(i) == 'h' )
            MoietySorter::SortByHeaviestElement(list);
          else if( sort.CharAt(i) == 'm' )
            MoietySorter::SortByWeight(list);
        }
        if( !labels.IsEmpty() )
          MoietySorter::SortByMoietyAtom(list, labels);
      }
      else
        MoietySorter::CreateMoieties(list);
    }
    if( keeph )
      AtomSorter::KeepH(list,GetLattice(), AtomSorter::atom_cmp_Label);
  }
  catch(const TExceptionBase& exc)  {
    TBasicApp::NewLogEntry(logError) << exc.GetException()->GetError();
  }
  if( !FLastLoader->IsNative() )  {
    AtomSorter::SyncLists(list,
      FLastLoader->GetAsymmUnit().GetResidue(0).GetAtomList());
    FLastLoader->GetAsymmUnit().ComplyToResidues();
  }
  // this changes Id's !!! so must be called after the SyncLists
  GetAsymmUnit().ComplyToResidues();
  // 2010.11.29, ASB bug fix for ADPS on H...
  GetUnitCell().UpdateEllipsoids();
  GetLattice().RestoreADPs(false);
}
//..............................................................................
void TXFile::ValidateTabs()  {
  for( size_t i=0; i < RefMod.InfoTabCount(); i++ )  {
    if( RefMod.GetInfoTab(i).GetType() != infotab_htab )
      continue;
    if (!RefMod.GetInfoTab(i).GetAtoms().IsExplicit()) continue;
    TTypeList<ExplicitCAtomRef> ta =
      RefMod.GetInfoTab(i).GetAtoms().ExpandList(GetRM(), 2);
    if (ta.IsEmpty()) continue;
    TSAtom* sa = NULL;
    InfoTab& it = RefMod.GetInfoTab(i);
    ASObjectProvider& objects = Lattice.GetObjects();
    const size_t ac = objects.atoms.Count();
    for( size_t j=0; j < ac; j++ )  {
      TSAtom& sa1 = objects.atoms[j];
      if( sa1.CAtom().GetId() == ta[0].GetAtom().GetId() )  {
        sa = &sa1;
        break;
      }
    }
    if( sa == NULL )  {
      RefMod.DeleteInfoTab(i--);
      continue;
    }
    bool hasH = false;
    for( size_t j=0; j < sa->NodeCount(); j++ )  {
      if( !sa->Node(j).IsDeleted() && sa->Node(j).GetType() == iHydrogenZ )  {
        hasH = true;
        break;
      }
    }
    if( !hasH )  {  
      TBasicApp::NewLogEntry() << "Removing HTAB (donor has no H atoms): "
        << it.InsStr();
      RefMod.DeleteInfoTab(i--);
      continue;  
    }
    // validate the distance makes sense
    const TAsymmUnit& au = *ta[0].GetAtom().GetParent();
    vec3d v1 = ta[0].GetAtom().ccrd();
    if( ta[0].GetMatrix() != NULL )
      v1  = *ta[0].GetMatrix()*v1;
    vec3d v2 = ta[1].GetAtom().ccrd();
    if( ta[1].GetMatrix() != NULL )
      v2  = *ta[1].GetMatrix()*v2;
    const double dis = au.CellToCartesian(v1).DistanceTo(au.CellToCartesian(v2));
    if( dis > 5 )  {
      TBasicApp::NewLogEntry() << "Removing HTAB (d > 5A): " << it.InsStr();
      RefMod.DeleteInfoTab(i--);  
      continue;
    }
  }
}
//..............................................................................
void TXFile::SaveToFile(const olxstr& FN, bool Sort)  {
  olxstr Ext = TEFile::ExtractFileExt(FN);
  TBasicCFile *Loader = FindFormat(Ext);
  TBasicCFile *LL = FLastLoader;
  if( !Loader->IsNative() )  {
    if( LL != Loader ) {
      if( !Loader->Adopt(*this) ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "could not adopt specified file format");
      }
    }
    else
      UpdateAsymmUnit();
    if( Sort )  
      Loader->GetAsymmUnit().Sort();
  }
  OnFileSave.Enter(this);
  IEObject* Cause = NULL;
  try  {  Loader->SaveToFile(FN);  }
  catch(const TExceptionBase& exc)  {
    Cause = exc.Replicate();
  }
  OnFileSave.Exit(this);
  if( Cause != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
}
//..............................................................................
void TXFile::Close()  {
  OnFileClose.Enter(this, FLastLoader);
  FLastLoader = NULL;
  RefMod.Clear(rm_clear_ALL);
  Lattice.Clear(true);
  OnFileClose.Exit(this, NULL);
}
//..............................................................................
IEObject* TXFile::Replicate() const {
  TXFile* xf = new TXFile(*(SObjectProvider*)Lattice.GetObjects().Replicate());
  for( size_t i=0; i < FileFormats.Count(); i++ )  {
    xf->RegisterFileFormat((TBasicCFile*)FileFormats.GetObject(i)->Replicate(),
                              FileFormats[i]);
  }
  return xf;
}
//..............................................................................
void TXFile::EndUpdate()  {
  OnFileLoad.Enter(this, &GetFileName());
  OnFileLoad.Execute(this);
  // we keep the asymmunit but clear the unitcell
  try {
    GetLattice().Init();
    OnFileLoad.Exit(this);
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
    Close();
  }
}
//..............................................................................
void TXFile::ToDataItem(TDataItem& item) {
  GetLattice().ToDataItem(item.AddItem("Lattice"));
  GetRM().ToDataItem(item.AddItem("RefModel"));
}
//..............................................................................
void TXFile::FromDataItem(TDataItem& item) {
  GetRM().Clear(rm_clear_ALL);
  GetLattice().FromDataItem(item.FindRequiredItem("Lattice"));
  GetRM().FromDataItem(item.FindRequiredItem("RefModel"));
  GetLattice().FinaliseLoading();
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXFile::LibGetFormula(const TStrObjList& Params, TMacroError& E)  {
  bool list = false, html = false, split = false;
  int digits = -1;
  if( Params.Count() > 0 )  {
    if( Params[0].Equalsi("list") )
      list = true;
    else if( Params[0].Equalsi("html") )
      html = true;
    else if( Params[0].Equalsi("split") )
      split = true;
  }
  if( Params.Count() == 2 )
    digits = Params[1].ToInt();

  const ContentList& content = GetRM().GetUserContent();
  olxstr rv;
  for( size_t i=0; i < content.Count(); i++) {
    rv << content[i].element.symbol;
    if( list )  rv << ':';
    else if (split) rv << ' ';
    bool subAdded = false;
    const double dv = content[i].count/GetAsymmUnit().GetZ();
    olxstr tmp = (digits > 0) ? olxstr::FormatFloat(digits, dv) : olxstr(dv);
    if( tmp.IndexOf('.') != InvalidIndex )
      tmp.TrimFloat();
    if( html )  {
      if( olx_abs(dv-1) > 0.01 && olx_abs(dv) > 0.01 )  {
        rv << "<sub>" << tmp;
        subAdded = true;
      }
    }
    else
      rv << tmp;

    if( (i+1) <  content.Count() )  {
      if( list )
        rv << ',';
      else
        if( html )  {
          if( subAdded )
            rv << "</sub>";
        }
        else
          rv << ' ';
    }
    else  // have to close the html tag
      if( html && subAdded )
        rv << "</sub>";

  }
  E.SetRetVal(rv);
}
//..............................................................................
void TXFile::LibSetFormula(const TStrObjList& Params, TMacroError& E) {
  if( Params[0].IndexOf(':') == InvalidIndex )
    GetRM().SetUserFormula(Params[0]);
  else  {
    ContentList content;
    TStrList toks(Params[0], ',');
    for( size_t i=0; i < toks.Count(); i++ )  {
      size_t ind = toks[i].FirstIndexOf(':');
      if( ind == InvalidIndex )  {
        E.ProcessingError(__OlxSrcInfo, "invalid formula syntax" );
        return;
      }
      const cm_Element* elm =
        XElementLib::FindBySymbol(toks[i].SubStringTo(ind));
      if( elm == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "element");
      content.AddNew(*elm,
        toks[i].SubStringFrom(ind+1).ToDouble()*GetAsymmUnit().GetZ());
    }
    if( content.IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "empty SFAC - check formula syntax");
      return;
    }
    GetRM().SetUserContent(content);
  }
}
//..............................................................................
void TXFile::LibEndUpdate(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  EndUpdate();
}
//..............................................................................
void TXFile::LibSaveSolution(const TStrObjList& Params, TMacroError& E)  {
  TIns* oins = (TIns*)FLastLoader;
  TIns ins;
  // needs to be called to assign the loaderIds for new atoms
  UpdateAsymmUnit();
  ins.GetRM().Assign( GetRM(), true );
  ins.AddIns("FMAP 2", ins.GetRM());
  ins.GetRM().SetRefinementMethod("L.S.");
  ins.GetRM().SetIterations(4);
  ins.GetRM().SetPlan(20);
  ins.GetRM().SetUserContent(oins->GetRM().GetUserContent());
  ins.SaveToFile(Params[0]);
}
void TXFile::LibDataCount(const TStrObjList& Params, TMacroError& E)  {
  if( EsdlInstanceOf(*FLastLoader, TCif) )
    E.SetRetVal(((TCif*)FLastLoader)->BlockCount());
  else
    E.SetRetVal(1);
}
//..............................................................................
void TXFile::LibCurrentData(const TStrObjList& Params, TMacroError& E)  {
  TCif &cif = *(TCif*)FLastLoader;
  if( Params.IsEmpty() )
    E.SetRetVal(cif.GetBlockIndex());
  else
    cif.SetCurrentBlock(Params[0].ToInt());
}
//..............................................................................
void TXFile::LibDataName(const TStrObjList& Params, TMacroError& E)  {
  int i = Params[0].ToInt();
  TCif &cif = *(TCif*)FLastLoader;
  if( i < 0 )
    E.SetRetVal(cif.GetDataName());
  else  {
    if( (size_t)i >= cif.BlockCount() )
      throw TIndexOutOfRangeException(__OlxSourceInfo, i, 0, cif.BlockCount());
    E.SetRetVal(cif.GetBlock(i).GetName());
  }
}
//..............................................................................
void TXFile::LibGetMu(const TStrObjList& Params, TMacroError& E)  {
  cm_Absorption_Coefficient_Reg ac;
  ContentList cont = GetAsymmUnit().GetContentList();
  double mu=0;
  for( size_t i=0; i < cont.Count(); i++ )  {
    XScatterer *xs = GetRM().FindSfacData(cont[i].element.symbol);
    if (xs != NULL && xs->IsSet(XScatterer::setMu)) {
      mu += cont[i].count*xs->GetMu()/10;
    }
    else {
      double v = ac.CalcMuOverRhoForE(
        GetRM().expl.GetRadiationEnergy(), *ac.locate(cont[i].element.symbol));
      mu += (cont[i].count*cont[i].element.GetMr())*v/6.022142;
    }
  }
  mu *= GetAsymmUnit().GetZ()/GetAsymmUnit().CalcCellVolume()/
    GetAsymmUnit().GetZPrime();
  E.SetRetVal(olxstr::FormatFloat(3,mu));
}
//..............................................................................
TLibrary* TXFile::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xf") : name);

  lib->Register(
    new TFunction<TXFile>(this, &TXFile::LibGetFormula, "GetFormula",
      fpNone|fpOne|fpTwo|psFileLoaded,
      "Returns a string for content of the asymmetric unit. Takes single or "
      "none parameters. If parameter equals 'html' and html formatted string is"
      " returned, for 'list' parameter a string like 'C:26,N:45' is returned. "
      "If no parameter is specified, just formula is returned")
   );

  lib->Register(
    new TFunction<TXFile>(this,  &TXFile::LibSetFormula, "SetFormula",
      fpOne|psCheckFileTypeIns|psCheckFileTypeP4P,
      "Sets formula for current file, takes a string of the following form "
      "'C:25,N:4'")
  );

  lib->Register(
    new TMacro<TXFile>(this,  &TXFile::LibEndUpdate, "EndUpdate",
      EmptyString(),
      fpNone|psCheckFileTypeIns,
      "Must be called after the content of the asymmetric unit has changed - "
      "this function will update the program state")
  );

  lib->Register(
    new TFunction<TXFile>(this,  &TXFile::LibSaveSolution, "SaveSolution",
      fpOne|psCheckFileTypeIns,
      "Saves current Q-peak model to provided file (res-file)")
  );

  lib->Register(
    new TFunction<TXFile>(this,  &TXFile::LibDataCount, "DataCount",
      fpNone|psFileLoaded,
      "Returns number of available data sets")
  );

  lib->Register(
    new TFunction<TXFile>(this,  &TXFile::LibDataName, "DataName",
      fpOne|psCheckFileTypeCif,
      "Returns data name for given CIF block")
  );
  
  lib->Register(
    new TFunction<TXFile>(this,  &TXFile::LibCurrentData, "CurrentData",
      fpNone|fpOne|psCheckFileTypeCif,
      "Returns current data index or changes current data block within the CIF")
  );
  
  lib->Register(
    new TFunction<TXFile>(this,  &TXFile::LibGetMu, "GetMu",
      fpNone|psFileLoaded,
      "Changes current data block within the CIF")
  );
  
  lib->AttachLibrary(Lattice.GetAsymmUnit().ExportLibrary());
  lib->AttachLibrary(Lattice.GetUnitCell().ExportLibrary());
  lib->AttachLibrary(Lattice.ExportLibrary());
  lib->AttachLibrary(RefMod.expl.ExportLibrary());
  lib->AttachLibrary(RefMod.ExportLibrary());
  lib->AttachLibrary(olx_analysis::Analysis::ExportLibrary());
  return lib;
}
//..............................................................................
void TXFile::NameArg::Parse(const olxstr& fn)  {
  this->file_name = fn;
  this->data_name.SetLength(0);
  this->is_index = false;
  const size_t hi = fn.LastIndexOf('#');
  const size_t ui = fn.LastIndexOf('$');
  if (hi == InvalidIndex && ui == InvalidIndex)
    return;
  const size_t di = fn.LastIndexOf('.');
  if (di != InvalidIndex) {
    if (hi != InvalidIndex && ui != InvalidIndex && di < hi && di < ui) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "only one data ID is allowed");
    }
    if (hi != InvalidIndex && di < hi) {
      this->data_name = fn.SubStringFrom(hi+1);
      this->file_name = fn.SubStringTo(hi);
      this->is_index = true;
    }
    else if (ui != InvalidIndex && di < ui) {
      this->data_name = fn.SubStringFrom(ui+1);
      this->file_name = fn.SubStringTo(ui);
      this->is_index = false;
    }
  }
  else {
    if (hi != InvalidIndex && ui != InvalidIndex) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "only one data ID is allowed");
    }
    if (hi != InvalidIndex) {
      this->data_name = fn.SubStringFrom(hi+1);
      this->file_name = fn.SubStringTo(hi);
      this->is_index = true;
    }
    else if (ui != InvalidIndex) {
      this->data_name = fn.SubStringFrom(ui+1);
      this->file_name = fn.SubStringTo(ui);
      this->is_index = false;
    }
  }
}
//..............................................................................
olxstr TXFile::NameArg::ToString() const {
  if (data_name.IsEmpty())
    return file_name;
  return olxstr(file_name) << (is_index ? '#' : '$') << data_name;
}
//..............................................................................
olxstr TXFile::LocateHklFile()  {
  olxstr HklFN = GetRM().GetHKLSource();
  if (TEFile::Existsi(olxstr(HklFN), HklFN))
    return HklFN;
  const olxstr fn = GetFileName();
  HklFN = TEFile::ChangeFileExt(fn, "hkl");
  if (TEFile::Existsi(olxstr(HklFN), HklFN))
    return HklFN;
  HklFN = TEFile::ChangeFileExt(fn, "raw");
  if (TEFile::Existsi(olxstr(HklFN), HklFN)) {
    THklFile Hkl;
    Hkl.LoadFromFile(HklFN);
    HklFN = TEFile::ChangeFileExt(fn, "hkl");
    for (size_t i=0; i < Hkl.RefCount(); i++) {
      Hkl[i].SetI((double)olx_round(Hkl[i].GetI())/100.0);
      Hkl[i].SetS((double)olx_round(Hkl[i].GetS())/100.0);
    }
    Hkl.SaveToFile(HklFN);
    TBasicApp::NewLogEntry() << "The scaled hkl file is prepared";
    return HklFN;
  }
  else {  // check for stoe format
    HklFN = TEFile::ChangeFileExt(fn, "hkl");
    olxstr HkcFN = TEFile::ChangeFileExt(fn, "hkc");
    if (TEFile::Existsi(olxstr(HkcFN), HkcFN)) {
      TEFile::Copy(HkcFN, HklFN);
      return HklFN;
    }
  }
  // last chance - get any hkl in the same folder (only if one!)
  TStrList hkl_files;
  olxstr dir = TEFile::ExtractFilePath(fn);
  TEFile::ListDir(dir, hkl_files, "*.hkl", sefFile);
  if (hkl_files.Count() == 1)
    return TEFile::AddPathDelimeterI(dir) << hkl_files[0];
  return EmptyString();
}
//..............................................................................


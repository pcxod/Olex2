//---------------------------------------------------------------------------//
// namespace TXFiles
// CIF and related data management procedures
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "cif.h"
#include "dataitem.h"

#include "catom.h"
#include "satom.h"
#include "symmparser.h"

#include "unitcell.h"
#include "ellipsoid.h"

#include "bapp.h"
#include "log.h"

#include "symmlib.h"
#include "etime.h"

using namespace exparse::parser_util;
using namespace cif_dp;

//----------------------------------------------------------------------------//
// TCif function bodies
//----------------------------------------------------------------------------//
TCif::TCif() : FDataNameUpperCase(true), block_index(InvalidIndex)  {  }
//..............................................................................
TCif::~TCif()  {  Clear();  }
//..............................................................................
void TCif::Clear()  {
  FWeightA.SetLength(0);
  FWeightB.SetLength(0);
  for( size_t i=0; i < Loops.Count(); i++ )
    delete Loops.GetObject(i);
  Loops.Clear();
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
  DataManager.Clear();
  Matrices.Clear();
  MatrixMap.Clear();
}
//..............................................................................
void TCif::LoadFromStrings(const TStrList& Strings)  {
  block_index = InvalidIndex;
  data_provider.LoadFromStrings(Strings);
  for( size_t i=0; i < data_provider.Count(); i++ )  {
    CifBlock& cb = data_provider[i];
    if( cb.param_map.IndexOf("_cell_length_a") == InvalidIndex )
      continue;
    bool valid = false;
    for( size_t j=0; j < cb.table_map.Count(); i++ )  {
      if( cb.table_map.GetKey(j).StartsFrom("_atom_site") )  {
        valid = true;
        break;
      }
    }
    if( valid )  {  
      block_index = i;
      break;
    }
  }
  if( block_index == InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate required data");
  CifBlock& cif_data = data_provider[block_index];
  Clear();
  for( size_t i=0; i < cif_data.table_map.Count(); i++ )  {
    Loops.Add(cif_data.table_map.GetValue(i)->GetName(), new TCifLoop(*cif_data.table_map.GetValue(i)));
  }
  /*search for the weigting sceme*************************************/
  const size_t ws_i = cif_data.param_map.IndexOf("_refine_ls_weighting_details");
  if( ws_i != InvalidIndex )  {
    IStringCifEntry* ci = dynamic_cast<IStringCifEntry*>(cif_data.param_map.GetValue(ws_i));
    if( ci != NULL && ci->Count() == 1 )  {
      const olxstr& tmp = (*ci)[0];
      for( size_t k=0; k < tmp.Length(); k++ )  {
        if( tmp[k] == '+' )  {
          if( FWeightA.IsEmpty() )  {
            while( tmp[k] != ')' )  {
              k++;
              if( k >= tmp.Length() )  break;
              FWeightA << tmp[k];
            }
            k--;
            continue;
          }
          if( FWeightB.IsEmpty() )  {
            while( tmp[k] != ']' )  {
              k++;
              if( k >= tmp.Length() )  break;
              FWeightB << tmp[k];
            }
            FWeightB.Delete(FWeightB.Length()-1, 1); // remove the [ bracket
          }
        }
      }
    }
  }
  Initialize();
}
//..............................................................................
//void GroupSection(TStrPObjList<olxstr,TCif::CifData*>& lines, size_t index,
//       const olxstr& sectionName, AnAssociation2<size_t,size_t>& indexes)  {
//  olxstr tmp;
//  for( size_t i=index; i < lines.Count(); i++ )  {
//    tmp = lines[i].Trim(' ');
//    if( tmp.IsEmpty() || tmp.StartsFromi("loop_") )  continue;
//    size_t ind = tmp.FirstIndexOf('_', 1);
//    if( ind == InvalidIndex || ind == 0 ) // a _loop ?
//      continue;
//    tmp = tmp.SubStringTo(ind);
//    if( tmp == sectionName )  {
//      if( indexes.GetB() != (i+1) )
//        lines.Move(i, indexes.GetB()+1);
//      indexes.B() ++;
//    }
//  }
//}
void TCif::Group()  {
  //TCSTypeList<olxstr, AnAssociation2<size_t,size_t> > sections;
  //olxstr tmp;
  //for( size_t i=0; i < Lines.Count(); i++ )  {
  //  tmp = Lines[i].Trim(' ');
  //  if( tmp.IsEmpty() || tmp.StartsFrom("loop_") )  continue;
  //  size_t ind = tmp.FirstIndexOf('_', 1);
  //  if( ind == InvalidIndex || ind == 0 ) // a _loop ?
  //    continue;
  //  tmp = tmp.SubStringTo(ind);
  //  ind = sections.IndexOfComparable(tmp);
  //  if( ind == InvalidIndex )  {
  //    sections.Add( tmp, AnAssociation2<size_t,size_t>(i,i) );
  //    AnAssociation2<size_t,size_t>& indexes = sections[tmp];
  //    GroupSection(Lines, i+1, tmp, indexes);
  //  }
  //}
  //// sorting the groups internally ...
  //for( size_t i=0; i < sections.Count(); i++ )  {
  //  size_t ss = sections.GetObject(i).GetA(),
  //      se = sections.GetObject(i).GetB();
  //  bool changes = true;
  //  while( changes )  {
  //    changes = false;
  //    for( size_t j=ss; j < se; j++ )  {
  //      if( Lines[j].Compare(Lines[j+1]) > 0 )  {
  //        Lines.Swap(j, j+1);
  //        changes = true;
  //      }
  //    }
  //  }
  //}
}
//..............................................................................
void TCif::SaveToStrings(TStrList& Strings)  {
  if( block_index == InvalidIndex )  return;
  CifBlock& cb = data_provider[block_index];
  for( size_t i=0; i < Loops.Count(); i++ )  {
    TCifLoop* lp = Loops.GetObject(i);

  }
//  cb.
}
//..............................................................................
bool TCif::ParamExists(const olxstr& Param) const {
  return (block_index == InvalidIndex) ? false :
    data_provider[block_index].param_map.HasKey(Param);
}
//..............................................................................
olxstr TCif::GetParamAsString(const olxstr &Param) const {
  if( block_index == InvalidIndex )  return EmptyString;
  IStringCifEntry* ce = dynamic_cast<IStringCifEntry*>(
    data_provider[block_index].param_map.Find(Param, NULL));
  if( ce == NULL || ce->Count() == 0 )
    return EmptyString;
  olxstr rv = (*ce)[0];
  for( size_t i = 1; i < ce->Count(); i++ )
    rv << '\n' << (*ce)[i];
  return rv;
}
//..............................................................................
void TCif::SetParam(const olxstr& name, const ICifEntry& value)  {
  if( block_index == InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
  data_provider[block_index].Add(name, value.Replicate());
}
//..............................................................................
void TCif::ReplaceParam(const olxstr& old_name, const olxstr& new_name, const ICifEntry& value)  {
  if( block_index == InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
  data_provider[block_index].Remove(old_name);
  data_provider[block_index].Add(new_name, value.Replicate());
}
//..............................................................................
void TCif::Rename(const olxstr& old_name, const olxstr& new_name)  {
  data_provider[block_index].Rename(old_name, new_name);
}
//..............................................................................
void TCif::Initialize()  {
  olxstr Param;
  TCifLoop *ALoop, *Loop;
  double Q[6], E[6]; // quadratic form of ellipsoid
  TEValueD EValue;
  try  {
    GetAsymmUnit().Axes()[0] = GetParamAsString("_cell_length_a");
    GetAsymmUnit().Axes()[1] = GetParamAsString("_cell_length_b");
    GetAsymmUnit().Axes()[2] = GetParamAsString("_cell_length_c");

    GetAsymmUnit().Angles()[0] = GetParamAsString("_cell_angle_alpha");
    GetAsymmUnit().Angles()[1] = GetParamAsString("_cell_angle_beta");
    GetAsymmUnit().Angles()[2] = GetParamAsString("_cell_angle_gamma");
  }
  catch(...) {  return;  }
  // check if the cif file contains valid parameters
  if( GetAsymmUnit().CalcCellVolume() == 0 )
    return;

  GetAsymmUnit().InitMatrices();

  Loop = FindLoop("_space_group_symop");
  if( Loop == NULL )
    Loop = FindLoop("_space_group_symop_operation_xyz");
  if( Loop != NULL  )  {
    size_t sindex = Loop->GetTable().ColIndex("_space_group_symop_operation_xyz");
    size_t iindex = Loop->GetTable().ColIndex("_space_group_symop_id");
    if( sindex != InvalidIndex )  {
      for( size_t i=0; i < Loop->GetTable().RowCount(); i++ )  {
        if( !TSymmParser::SymmToMatrix(Loop->GetTable()[i][sindex]->GetStringValue(), Matrices.AddNew()) )
          throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
        if( iindex == InvalidIndex )
          MatrixMap.Add(i+1, i);
        else
          MatrixMap.Add(Loop->GetTable()[i][iindex]->GetStringValue(), i);
      }
    }
  }
  else  {
    Loop = FindLoop("_symmetry_equiv_pos");
    if( Loop == NULL )
      Loop = FindLoop("_symmetry_equiv_pos_as_xyz");
    if( Loop != NULL  )  {
      TCifLoop& symop_loop = *(new TCifLoop(
        (cetTable&)data_provider[block_index].Add("_space_group_symop", new cetTable)));
      symop_loop.GetTable().AddCol("_space_group_symop_id");
      symop_loop.GetTable().AddCol("_space_group_symop_operation_xyz");

      size_t sindex = Loop->GetTable().ColIndex("_symmetry_equiv_pos_as_xyz");
      size_t iindex = Loop->GetTable().ColIndex("_symmetry_equiv_pos_site_id");
      if( sindex != InvalidIndex )  {
        for( size_t i=0; i < Loop->GetTable().RowCount(); i++ )  {
          if( !TSymmParser::SymmToMatrix(Loop->GetTable()[i][sindex]->GetStringValue(), Matrices.AddNew()) )
            throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
          CifRow& row = symop_loop.GetTable().AddRow();
          if( iindex == InvalidIndex )  {
            MatrixMap.Add(i+1, i);
            row[0] = new cetString(i+1);
          }
          else  {
            MatrixMap.Add(Loop->GetTable()[i][iindex]->GetStringValue(), i);
            row[0] = Loop->GetTable()[i][iindex]->Replicate();
          }
          row[1] = Loop->GetTable()[i][sindex]->Replicate();
        }
      }
      // replace obsolete loop
      const size_t li = Loops.IndexOfObject(Loop);
      Loops.Delete(li);
      delete Loop;
      Loops.Insert(li, symop_loop.GetLoopName(), &symop_loop);
    }
  }
  TSpaceGroup* sg = TSymmLib::GetInstance().FindSymSpace(Matrices);
  if( sg != NULL )
    GetAsymmUnit().ChangeSpaceGroup(*sg);
  else   {
    GetAsymmUnit().ChangeSpaceGroup(*TSymmLib::GetInstance().FindGroup("P1"));
    //throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  }
  
  try  {
    GetRM().SetUserFormula(olxstr::DeleteChars(GetParamAsString("_chemical_formula_sum"), ' '));
  }
  catch(...)  {  }
  
  this->Title = GetDataName().ToUpperCase();
  this->Title << " OLEX2: imported from CIF";

  ALoop = FindLoop("_atom_site");
  if( ALoop == NULL )  return;

  size_t ALabel =  ALoop->GetTable().ColIndex("_atom_site_label");
  size_t ACi[] = {
    ALoop->GetTable().ColIndex("_atom_site_fract_x"),
    ALoop->GetTable().ColIndex("_atom_site_fract_y"),
    ALoop->GetTable().ColIndex("_atom_site_fract_z")
  };
  size_t ACUiso =  ALoop->GetTable().ColIndex("_atom_site_U_iso_or_equiv");
  size_t ASymbol = ALoop->GetTable().ColIndex("_atom_site_type_symbol");
  size_t APart   = ALoop->GetTable().ColIndex("_atom_site_disorder_group");
  size_t SiteOccu = ALoop->GetTable().ColIndex("_atom_site_occupancy");
  size_t Degen = ALoop->GetTable().ColIndex("_atom_site_symmetry_multiplicity");
  if( (ALabel|ACi[0]|ACi[1]|ACi[2]|ASymbol) == InvalidIndex )  {
    TBasicApp::GetLog().Error("Failed to locate required fields in atoms loop");
    return;
  }
  for( size_t i=0; i < ALoop->GetTable().RowCount(); i++ )  {
    TCAtom& A = GetAsymmUnit().NewAtom();
    A.SetLabel(ALoop->GetTable()[i][ALabel]->GetStringValue(), false);
    cm_Element* type = XElementLib::FindBySymbol(ALoop->GetTable()[i][ASymbol]->GetStringValue());
    if( type == NULL )  {
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Undefined element: ") <<
        ALoop->GetTable()[i][ASymbol]->GetStringValue());
    }
    A.SetType(*type);
    for( int j=0; j < 3; j++ )  {
      EValue = ALoop->GetTable()[i][ACi[j]]->GetStringValue();
      A.ccrd()[j] = EValue.GetV();  A.ccrdEsd()[j] = EValue.GetE();
      if( EValue.GetE() == 0 )
        GetRM().Vars.FixParam(A, catom_var_name_X+j);
    }
    if( ACUiso != InvalidIndex )    {
      EValue = ALoop->GetTable()[i][ACUiso]->GetStringValue();
      A.SetUisoEsd(EValue.GetE());
      A.SetUiso(EValue.GetV());
      if( EValue.GetE() == 0 )  GetRM().Vars.FixParam(A, catom_var_name_Uiso);
    }
    if( APart != InvalidIndex && ALoop->GetTable()[i][APart]->GetStringValue().IsNumber() )
      A.SetPart(ALoop->GetTable()[i][APart]->GetStringValue().ToInt());
    if( SiteOccu != InvalidIndex )  {
      EValue = ALoop->GetTable()[i][SiteOccu]->GetStringValue();
      A.SetOccu(EValue.GetV());
      A.SetOccuEsd(EValue.GetE());
      if( EValue.GetE() == 0 )  GetRM().Vars.FixParam(A, catom_var_name_Sof);
    }
    if( Degen != InvalidIndex )
      A.SetOccu(A.GetOccu()/ALoop->GetTable()[i][Degen]->GetStringValue().ToDouble());
    ALoop->SetData(i, ALabel, new AtomCifEntry(&A));
  }
  for( size_t i=0; i < Loops.Count(); i++ )  {
    if( Loops.GetObject(i) == ALoop )  continue;
    CifTable& tab = Loops.GetObject(i)->GetTable();
    for( size_t j=0; j < tab.ColCount(); j++ )  {
      if(  tab.ColName(j).IndexOf("atom_site") != InvalidIndex &&
        tab.ColName(j).IndexOf("label") != InvalidIndex )
      {
        for( size_t k=0; k < tab.RowCount(); k++ )  {
          TCAtom* ca = GetAsymmUnit().FindCAtom(tab[k][j]->GetStringValue());
          if( ca != NULL )
            Loops.GetObject(i)->SetData(k, j, new AtomCifEntry(ca));
        }
      }
    }
  }

  ALoop = FindLoop("_atom_site_aniso");
  if( ALoop == NULL )  return;
  ALabel =  ALoop->GetTable().ColIndex("_atom_site_aniso_label");
  size_t Ui[] = {
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_11"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_22"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_33"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_23"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_13"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_12")
  };
  if( (ALabel|Ui[0]|Ui[1]|Ui[2]|Ui[3]|Ui[4]|Ui[5]) != InvalidIndex )  {
    for( size_t i=0; i < ALoop->GetTable().RowCount(); i++ )  {
      TCAtom* A = GetAsymmUnit().FindCAtom(ALoop->GetTable()[i][ALabel]->GetStringValue());
      if( A == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("wrong atom in the aniso loop ") << ALabel);
      for( int j=0; j < 6; j++ )  {
        EValue = ALoop->GetTable()[i][Ui[j]]->GetStringValue();
        Q[j] = EValue.GetV();  E[j] = EValue.GetE();
        if( EValue.GetE() == 0 )
          GetRM().Vars.FixParam(*A, catom_var_name_U11+j);
      }
      GetAsymmUnit().UcifToUcart(Q);
      A->AssignEllp(&GetAsymmUnit().NewEllp().Initialise(Q, E));
    }
  }
  // geometric parameters
  ALoop = FindLoop("_geom_bond");
  if( ALoop != NULL )  {
    CifTable& tab = ALoop->GetTable();
    size_t ALabel =  tab.ColIndex("_geom_bond_atom_site_label_1");
    size_t ALabel1 = tab.ColIndex("_geom_bond_atom_site_label_2");
    size_t BD =  tab.ColIndex("_geom_bond_distance");
    size_t SymmA = tab.ColIndex("_geom_bond_site_symmetry_2");
    if( (ALabel|ALabel1|BD|SymmA) != InvalidIndex )  {
      TEValueD ev;
      for( size_t i=0; i < tab.RowCount(); i++ )  {
        CifRow& Row = tab[i];
        ACifValue* cv = NULL;
        ev = Row[BD]->GetStringValue();
        if( Row[SymmA]->GetStringValue() == '.' )  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            ev);
        }
        else  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            SymmCodeToMatrix(Row[SymmA]->GetStringValue()),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
  ALoop = FindLoop("_geom_hbond");
  if( ALoop != NULL )  {
    CifTable& tab = ALoop->GetTable();
    size_t ALabel =  tab.ColIndex("_geom_hbond_atom_site_label_D");
    size_t ALabel1 = tab.ColIndex("_geom_hbond_atom_site_label_A");
    size_t BD =  tab.ColIndex("_geom_hbond_distance_DA");
    size_t SymmA = tab.ColIndex("_geom_hbond_site_symmetry_A");
    if( (ALabel|ALabel1|BD|SymmA) != InvalidIndex )  {
      TEValueD ev;
      for( size_t i=0; i < tab.RowCount(); i++ )  {
        CifRow& Row = tab[i];
        ACifValue* cv = NULL;
        ev = Row[BD]->GetStringValue();
        if( Row[SymmA]->GetStringValue() == '.' )  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            ev);
        }
        else  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            SymmCodeToMatrix(Row[SymmA]->GetStringValue()),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
  ALoop = FindLoop("_geom_angle");
  if( ALoop != NULL )  {
    CifTable& tab = ALoop->GetTable();
    const size_t ind_l =  tab.ColIndex("_geom_angle_atom_site_label_1");
    const size_t ind_m =  tab.ColIndex("_geom_angle_atom_site_label_2");
    const size_t ind_r =  tab.ColIndex("_geom_angle_atom_site_label_3");
    const size_t ind_a =  tab.ColIndex("_geom_angle");
    const size_t ind_sl = tab.ColIndex("_geom_angle_site_symmetry_1");
    const size_t ind_sr = tab.ColIndex("_geom_angle_site_symmetry_3");
    if( (ind_l|ind_m|ind_r|ind_a|ind_sl|ind_sr) != InvalidIndex )  {
      TEValueD ev;
      smatd im;
      im.I();
      for( size_t i=0; i < tab.RowCount(); i++ )  {
        CifRow& Row = tab[i];
        ACifValue* cv = NULL;
        ev = Row[ind_a]->GetStringValue();
        if( Row[ind_sl]->GetStringValue() == '.' && Row[ind_sr]->GetStringValue() == '.' )  {
          cv = new CifAngle(
            *GetAsymmUnit().FindCAtom(Row[ind_l]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_m]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_r]->GetStringValue()),
            ev);
        }
        else  {
          cv = new CifAngle(
            *GetAsymmUnit().FindCAtom(Row[ind_l]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_m]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_r]->GetStringValue()),
            Row[ind_sl]->GetStringValue() == '.' ? im : SymmCodeToMatrix(Row[ind_sl]->GetStringValue()),
            Row[ind_sr]->GetStringValue() == '.' ? im : SymmCodeToMatrix(Row[ind_sr]->GetStringValue()),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
}
//..............................................................................
TCifLoop& TCif::AddLoop(const olxstr& name)  {
  TCifLoop *CF = FindLoop(name);
  if( CF != NULL )  return *CF;
  return *Loops.Add(name,
    new TCifLoop((cetTable&)data_provider[block_index].Add(name, new cetTable))).Object;
}
//..............................................................................
TCifLoop& TCif::GetPublicationInfoLoop()  {
  const static olxstr publ_ln( "_publ_author" ), publ_jn("_publ_requested_journal");
  TCifLoop *CF = FindLoop(publ_ln);
  if( CF != NULL )  return *CF;
  // to make the automatic grouping to work ...
  if( !ParamExists(publ_jn) )
    data_provider[block_index].Add(publ_jn, new cetNamedString(publ_jn, "?"));
  CF = Loops.Insert(0, publ_ln,
    new TCifLoop((cetTable&)data_provider[block_index].Add(publ_ln, new cetTable))).Object;
  CF->GetTable().AddCol("_publ_author_name");
  CF->GetTable().AddCol("_publ_author_email");
  CF->GetTable().AddCol("_publ_author_address");
  return *CF;
}
//..............................................................................
bool TCif::Adopt(TXFile& XF)  {
  Clear();
  double Q[6], E[6];  // quadratic form of s thermal ellipsoid
  GetRM().Assign(XF.GetRM(), true);
  GetAsymmUnit().SetZ((short)XF.GetLattice().GetUnitCell().MatrixCount());
  Title = TEFile::ChangeFileExt(TEFile::ExtractFileName(XF.GetFileName()), EmptyString);

  block_index = 0;
  data_provider.Add(Title);
  SetParam("_audit_creation_method", "OLEX2", true);
  SetParam("_chemical_name_systematic", "?", true);
  SetParam("_chemical_name_common", "?", true);
  SetParam("_chemical_melting_point", "?", false);
  SetParam("_chemical_formula_moiety", XF.GetLattice().CalcMoiety(), true);
  SetParam("_chemical_formula_sum", GetAsymmUnit().SummFormula(' ', false), true);
  SetParam("_chemical_formula_weight", olxstr::FormatFloat(2, GetAsymmUnit().MolWeight()), false);

  SetParam("_cell_length_a", GetAsymmUnit().Axes()[0].ToString(), false);
  SetParam("_cell_length_b", GetAsymmUnit().Axes()[1].ToString(), false);
  SetParam("_cell_length_c", GetAsymmUnit().Axes()[2].ToString(), false);

  SetParam("_cell_angle_alpha", GetAsymmUnit().Angles()[0].ToString(), false);
  SetParam("_cell_angle_beta",  GetAsymmUnit().Angles()[1].ToString(), false);
  SetParam("_cell_angle_gamma", GetAsymmUnit().Angles()[2].ToString(), false);
  SetParam("_cell_volume", XF.GetUnitCell().CalcVolumeEx().ToString(), false);
  SetParam("_cell_formula_units_Z", XF.GetAsymmUnit().GetZ(), false);

  SetParam("_diffrn_ambient_temperature",
    XF.GetRM().expl.IsTemperatureSet() ? XF.GetRM().expl.GetTemperature() : olxstr('?'), false);
  SetParam("_diffrn_radiation_wavelength", XF.GetRM().expl.GetRadiation(), false);

  if( XF.GetAsymmUnit().IsQPeakMinMaxInitialised() )
    SetParam("_refine_diff_density_max", XF.GetAsymmUnit().GetMaxQPeak(), false);
  TSpaceGroup& sg = XF.GetLastLoaderSG();
  SetParam("_space_group_crystal_system", sg.GetBravaisLattice().GetName().ToLowerCase(), true);
  SetParam("_space_group_name_H-M_alt", sg.GetFullName(), true);
  SetParam("_space_group_name_Hall", sg.GetHallSymbol(), true);
  SetParam("_space_group_IT_number", sg.GetNumber(), false);
  {
    TCifLoop& Loop = AddLoop("_space_group_symop");
    CifTable& Table = Loop.GetTable();
    Table.AddCol("_space_group_symop_id");
    Table.AddCol("_space_group_symop_operation_xyz");
    sg.GetMatrices(Matrices, mattAll);
    for( size_t i=0; i < Matrices.Count(); i++ )  {
      CifRow& row = Table.AddRow();
      row[0] = new cetString(i+1);
      row[1] = new cetString(TSymmParser::MatrixToSymm(Matrices[i]));
    }
  }

  SetParam("_computing_structure_solution", "?", true);
  SetParam("_computing_molecular_graphics", "?", true);
  SetParam("_computing_publication_material", "?", true);

  SetParam("_atom_sites_solution_primary", "?", false);
  CifTable& atom_table = AddLoop("_atom_site").GetTable();
  atom_table.AddCol("_atom_site_label");
  atom_table.AddCol("_atom_site_type_symbol");
  atom_table.AddCol("_atom_site_fract_x");
  atom_table.AddCol("_atom_site_fract_y");
  atom_table.AddCol("_atom_site_fract_z");
  atom_table.AddCol("_atom_site_U_iso_or_equiv");
  atom_table.AddCol("_atom_site_adp_type");
  atom_table.AddCol("_atom_site_occupancy");
  atom_table.AddCol("_atom_site_refinement_flags_posn");
  atom_table.AddCol("_atom_site_symmetry_multiplicity");
  atom_table.AddCol("_atom_site_disorder_group");

  CifTable& u_table = AddLoop("_atom_site_aniso").GetTable();
  u_table.AddCol("_atom_site_aniso_label");
  u_table.AddCol("_atom_site_aniso_U_11");
  u_table.AddCol("_atom_site_aniso_U_22");
  u_table.AddCol("_atom_site_aniso_U_33");
  u_table.AddCol("_atom_site_aniso_U_23");
  u_table.AddCol("_atom_site_aniso_U_13");
  u_table.AddCol("_atom_site_aniso_U_12");

  for( size_t i = 0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& A = GetAsymmUnit().GetAtom(i);
    CifRow& Row = atom_table.AddRow();
    Row[0] = new cetString(A.GetLabel());
    Row[1] = new cetString(A.GetType().symbol);
    for( int j=0; j < 3; j++ )
      Row.Set(j+2, new cetString(TEValueD(A.ccrd()[j], A.ccrdEsd()[j]).ToString()));
    Row.Set(5, new cetString(TEValueD(A.GetUiso(), A.GetUisoEsd()).ToString()));
    Row.Set(6, new cetString(A.GetEllipsoid() == NULL ? "Uiso" : "Uani"));
    Row.Set(7, new cetString(TEValueD(A.GetOccu()*A.GetDegeneracy(), A.GetOccuEsd()).ToString()));
    if( A.GetParentAfixGroup() != NULL && A.GetParentAfixGroup()->IsRiding() )
      Row.Set(8, new cetString("R"));
    else
      Row.Set(8, new cetString('.'));
    Row.Set(9, new cetString(A.GetDegeneracy()));
    // process part as well
    if( A.GetPart() != 0 )
      Row[10] = new cetString((int)A.GetPart());
    else
      Row[10] = new cetString('.');
    if( A.GetEllipsoid() != NULL )  {
      A.GetEllipsoid()->GetQuad(Q, E);
      GetAsymmUnit().UcartToUcif(Q);
      CifRow& Row1 = u_table.AddRow(EmptyString);
      Row1[0] = new AtomCifEntry(&A);
      for( int j=0; j < 6; j++ )  {
        Row1.Set(j+1, new cetString(TEValueD(Q[j], E[j]).ToString()));
      }
    }
  }
  return true;
}
//..............................................................................
smatd TCif::SymmCodeToMatrix(const olxstr &Code) const {
  size_t ui = Code.LastIndexOf('_');
  if( ui == InvalidIndex )
    return GetMatrixById(Code);
  smatd mSymm = GetMatrixById(Code.SubStringTo(ui));
  olxstr str_t = Code.SubStringFrom(ui+1);
  if( str_t.Length() != 3 )
    return mSymm;
  mSymm.t[0] += (int)(str_t.CharAt(0)-'5');
  mSymm.t[1] += (int)(str_t.CharAt(1)-'5');
  mSymm.t[2] += (int)(str_t.CharAt(2)-'5');
  return mSymm;
}
//..............................................................................
bool TCif::ResolveParamsFromDictionary(TStrList &Dic, olxstr &String,
 olxch Quote,
 olxstr (*ResolveExternal)(const olxstr& valueName),
 bool DoubleTheta) const
{
  size_t start, end;
  for( size_t i=0; i < String.Length(); i++ )  {
    if( String.CharAt(i) == Quote )  {
      if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
        String.Delete(i, 1);
        continue;
      }
      if( i > 0 && String.CharAt(i-1) == '\\' )  // escaped?
        continue;
      olxstr Val;
      if( (i+1) < String.Length() &&
          (String.CharAt(i+1) == '$' || String.CharAt(i+1) == '_' ||
          olxstr::o_isdigit(String.CharAt(i+1))) )
      {
        start = i;
        while( ++i < String.Length() )  {
          if( String.CharAt(i) == Quote )  {
            if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
              String.Delete(i, 1);
              Val << Quote;
              continue;
            }
            else if( String.CharAt(i-1) == '\\' ) // escaped?
              ;
            else  {
              end = i;  
              break;
            }
          }
          Val << String.CharAt(i);
        }
      }
      if( !Val.IsEmpty() )  {
        if( !Val.IsNumber() )  {
          if( Val.CharAt(0) == '$' )  {
            if( ResolveExternal != NULL )  {
              String.Delete(start, end-start+1);
              Val.Replace("\\%", '%');
              ResolveParamsFromDictionary(Dic, Val, Quote, ResolveExternal);
              olxstr Tmp = ResolveExternal(Val);
              ResolveParamsFromDictionary(Dic, Tmp, Quote, ResolveExternal);
              String.Insert(Tmp, start);
              i = start + Tmp.Length() - 1;
            }
          }
          else if( Val.CharAt(0) == '_' )  {
            IStringCifEntry* Params = FindParam<IStringCifEntry>(Val);
            olxstr Tmp = 'N';
            if( Params != NULL && Params->Count() != 0 )  
              Tmp = (*Params)[0];
            String.Delete(start, end-start+1);
            String.Insert(Tmp, start);
            i = start + Tmp.Length() - 1;
          }
          else
            TBasicApp::GetLog() << olxstr("A number or function starting from '$' or '_' is expected");
          continue;
        }
        size_t index = Val.ToSizeT();
        // Not much use if not for personal use :D
        /*
        if( index >= 73 )  {  //direct insert
          // 73 - crystals handling
          // 74 - other programs
          // 75 - solved and refined by...
          // 76 - collected for
          // 77 - anisotropic atoms
          // ....
          if( (index > Dic.Count()) || (index <= 0) )
            TBasicApp::GetLog()->Info( olxstr("Wrong parameter index ") << index);
          else  {
            String.Delete(start, end-start+1);
            String.Insert(Dic.String(index-1), start);
            i = start + Dic.String(index-1).Length() - 1;
          }
          continue;
        }
        */
        if( (index > Dic.Count()) || (index <= 0) )
          TBasicApp::GetLog().Error(olxstr("Wrong parameter index ") << index);
        else  {  // resolve indexes
          String.Delete(start, end-start+1);
          olxstr SVal = Dic[index-1];
          olxstr value;
          if( !SVal.IsEmpty() )  {
            if( SVal.Equalsi("date") )
              value = TETime::FormatDateTime(TETime::Now());
            else if( SVal.Equalsi("sg_number") )  {
              TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(GetAsymmUnit());
              if( sg != NULL )
                value = sg->GetNumber();
              else
                value = "unknown";
            }
            else if( SVal.Equalsi("data_name") )
              value = GetDataName();
            else if( SVal.Equalsi("weighta") )
              value = GetWeightA();
            else if( SVal.Equalsi("weightb") )
              value = GetWeightB();
            else {
              IStringCifEntry* Params = FindParam<IStringCifEntry>(SVal);
              if( Params == NULL )  {
                TBasicApp::GetLog().Info(olxstr("The parameter \'") << SVal << "' is not found");
                value = 'N';
              }
              else if( Params->Count() == 0 )  {
                TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  value = "none";
              }
              else if( Params->Count() == 1 )  {
                if( (*Params)[0].IsEmpty() )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  value = "none";
                }
                else if( (*Params)[0].CharAt(0) == '?' )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not defined");
                  value = '?';
                }
                else  {
                  if( (index == 13 || index == 14 || index == 30) && DoubleTheta )
                    value = (*Params)[0].ToDouble()*2;
                  else
                    value = (*Params)[0];
                }
              }
              else  {
                value = (*Params)[0];
                for( size_t sti=1; sti < Params->Count(); sti++ )  {
                  value << ' ' << (*Params)[sti];
                }
              }
            }
            String.Insert(value, start);
            i = start + value.Length() - 1;
          }
        }
      }
    }
  }
  return true;
}
//..............................................................................
void TCif::MultValue(olxstr &Val, const olxstr &N)  {
  Val = (TEValue<double>(Val) *= N.ToDouble()).ToString();
}
//..............................................................................
bool TCif::CreateTable(TDataItem *TD, TTTable<TStrList> &Table, smatd_list& SymmList) const {
  int RowDeleted=0, ColDeleted=0;
  SymmList.Clear();
  CifTable* LT = NULL;
  for( size_t i=0; i < Loops.Count(); i++ )  {
    TCifLoop* Loop = Loops.GetObject(i);
    LT = &Loop->GetTable();
    if( LT->ColCount() < TD->ItemCount() )  continue;
    size_t defcnt = 0;
    for( size_t j=0; j < LT->ColCount(); j++ )  {
      if( TD->FindItemi(LT->ColName(j)) != NULL )
        defcnt ++;
    }
    if( defcnt == TD->ItemCount() )  break;
    else  LT = NULL;
  }
  if( LT == NULL )  {
    TBasicApp::GetLog().Info(olxstr("Could not find loop for table definition: ") << TD->GetName());
    return false;
  }
  Table.Resize(LT->RowCount(), LT->ColCount());
  for( size_t i =0; i < Table.ColCount(); i++ )  {
    Table.ColName(i) = LT->ColName(i);
    for( size_t j=0; j < Table.RowCount(); j++ )
      Table[i][j] = (*LT)[i][j]->GetStringValue();
  }
  // process rows
  for( size_t i=0; i < LT->RowCount(); i++ )  {
    bool AddRow = true;
    for( size_t j=0; j < LT->ColCount(); j++ )  {
      TDataItem *DI = TD->FindItemi(LT->ColName(j));
      if( LT->ColName(j).StartsFrom("_geom_") && 
        LT->ColName(j).IndexOf("site_symmetry") != InvalidIndex)
      {
        if( (*LT)[i][j]->GetStringValue() != '.' )  {  // 1_555
          olxstr tmp = LT->ColName(j).SubStringFrom(LT->ColName(j).LastIndexOf('_')+1);
          //if( !tmp.IsNumber() ) continue;
          olxstr Tmp = "label_";
          Tmp << tmp;
          smatd SymmMatr = SymmCodeToMatrix((*LT)[i][j]->GetStringValue());
          size_t matIndex = SymmList.IndexOf(SymmMatr);
          if( matIndex == InvalidIndex )  {
            SymmList.AddCCopy(SymmMatr);
            matIndex = SymmList.Count()-1;
          }
          for( size_t k=0; k < Table.ColCount(); k++ )  {
            if( Table.ColName(k).EndsWith(Tmp) )  {
              Table[i-RowDeleted][k] << "<sup>" << (matIndex+1) << "</sup>";
              break;
            }
          }
        }
      }
      if( DI == NULL )  continue;
      olxstr Val = (*LT)[i][j]->GetStringValue();
      olxstr Tmp = DI->GetFieldValue("mustequal", EmptyString);
      TStrList Toks(Tmp, ';');
      if( !Tmp.IsEmpty() && (Toks.IndexOfi(Val) == InvalidIndex) ) // equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("atypeequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        ICifEntry* CD = (*LT)[i][j];
        if( CD != NULL && EsdlInstanceOf(*CD, AtomCifEntry) )
          if( !((AtomCifEntry*)CD)->data->GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("atypenotequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        ICifEntry* CD = (*LT)[i][j];
        if( CD != NULL && EsdlInstanceOf(*CD, AtomCifEntry) )
          if( ((AtomCifEntry*)CD)->data->GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("mustnotequal", EmptyString);
      Toks.Clear();
      Toks.Strtok(Tmp, ';');
      if( !Tmp.IsEmpty() && (Toks.IndexOfi(Val) != InvalidIndex) ) // not equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("multiplier", EmptyString);
      if( !Tmp.IsEmpty() )  {  // Multiply
        Val = Table[i-RowDeleted][j];
        MultValue(Val, Tmp);
        Table[i-RowDeleted][j] = Val;
      }
    }
    if( !AddRow )  {
      Table.DelRow(i-RowDeleted);
      RowDeleted++;
    }
  }
  // process columns
  for( size_t i=0; i < LT->ColCount(); i++ )  {
    TDataItem *DI = TD->FindItemi(LT->ColName(i));
    if( DI != NULL )  {
      Table.ColName(i-ColDeleted) = DI->GetFieldValueCI("caption");
      if( !DI->GetFieldValueCI("visible", FalseString).ToBool() )  {
        Table.DelCol(i-ColDeleted);
        ColDeleted++;
      }
    }
    else  {
      Table.DelCol(i-ColDeleted);
      ColDeleted++;
    }
  }
  return true;
}
//..............................................................................



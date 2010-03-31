#include "cifloop.h"
#include "cif.h"

//..............................................................................
void  TCifLoop::Clear()  {
  for( size_t i=0; i < FTable.RowCount(); i++ )  {
    for( size_t j=0; j < FTable.ColCount(); j++ )
      delete FTable[i].GetObject(j);
  }
  FTable.Clear();
  FComments = EmptyString;
}
//..............................................................................
void TCifLoop::DeleteAtom(TCAtom *A)  {
  for( size_t i=0; i < FTable.RowCount(); i++ )  {
    for( size_t j=0; j < FTable.ColCount(); j++ )  {
      ICifCell* CD = FTable[i].GetObject(j);
      if( CD->GetAtomRef() == A )  {
        for( size_t k=0; k < FTable.ColCount(); k++ )
          delete FTable[i].GetObject(k);
        FTable.DelRow(i--);
        break;
      }
    }
  }
}
//..............................................................................
void TCifLoop::Format(TStrList& Data)  {
  if( FTable.ColCount() == 0 )  return;
  FComments = EmptyString;
  const size_t ColCount = FTable.ColCount();
  for( size_t i=0; i < Data.Count(); i++ )  {
    if( Data[i].IsEmpty() )  continue;
    if( Data[i].CharAt(0) == '#' )  {
      FComments << Data[i] << '\n';
      Data[i] = EmptyString;
    }
  }
  TCifRow Params;
  TStrList toks;
  for( size_t i=0; i < Data.Count(); i++ )  {
    if( Data[i].IsEmpty() )  continue;
    if( Data[i].StartsFrom(';') )  {
      toks.Clear();
      if( Data[i].Length() > 1 )
        toks.Add(Data[i].SubStringFrom(1));
      while( ++i < Data.Count() && !Data[i].StartsFrom(';') )
        toks.Add(Data[i]);
      toks.TrimWhiteCharStrings();
      Params.Add(toks.Text("\\n"), new StringListCifCell);
      continue;
    }
    toks.Clear();
    TCif::CIFToks(Data[i], toks);
    for( size_t j=0; j < toks.Count(); j++ )  {
      if( toks[j].StartsFrom('\'') || toks[j].StartsFrom('"') )
        Params.Add(toks[j].SubStringFrom(1,1).TrimWhiteChars(), new StringCifCell(true));
      else
        Params.Add(toks[j], new StringCifCell(false));
    }
  }
  if( (Params.Count() % ColCount) != 0 )  {
#ifdef _DEBUG
    for( size_t j=0; j < ColCount; j++ )
      TBasicApp::GetLog() << FTable.ColName(j) << ' ';
    TBasicApp::GetLog() << '\n';
    for( size_t i=0; i < Params.Count(); i+= ColCount)  {
      olxstr line;
      for( size_t j=0; j < ColCount; j++ )  {
        if( i+j >= Params.Count() )
          break;
        line << Params[i+j] << ' ';
      }
      TBasicApp::GetLog() << line << '\n';
    }
#endif
    for( size_t i=0; i < Params.Count(); i++ )  // clean up the memory
      if( Params.GetObject(i) != NULL )
        delete Params.GetObject(i);
    throw TFunctionFailedException(__OlxSourceInfo, 
      olxstr("Wrong number of parameters in '") << GetLoopName() << "' loop");
  }
  const size_t RowCount = Params.Count()/ColCount;
  FTable.SetRowCount(RowCount);
  for( size_t i=0; i < RowCount; i++ )  {
    for( size_t j=0; j < ColCount; j++ )
      FTable[i].Set(j, Params[i*ColCount+j], Params.GetObject(i*ColCount+j));
  }
}
//..............................................................................
void TCifLoop::SaveToStrings(TStrList& Strings) const {
  for( size_t i=0; i < FTable.ColCount(); i++ )  // loop header
    Strings.Add("  ") << FTable.ColName(i);
  for( size_t i=0; i < FTable.RowCount(); i++ ) {  // loop content
    Strings.Add(EmptyString);
    for( size_t j=0; j < FTable.ColCount(); j++ )  {
      ICifCell* CLD = FTable[i].GetObject(j);
      if( CLD->GetAtomRef() != NULL )  {
        if( CLD->GetAtomRef()->IsDeleted() )  {  // skip deleted atom
          break;
        }
      }
      olxstr str = FTable[i][j];
      if( CLD->IsBlock() )  {
        if( str.EndsWith("\\n") )
          str.SetLength(str.Length()-2);
        Strings.Add(';');
        Strings.AddList(TStrList(str, "\\n"));
        Strings.Add(';');
        Strings.Add(EmptyString);
      }
      else if( CLD->IsQuoted() )  {
        if( Strings.Last().String.Length() + str.Length() < 77 )
          Strings.Last().String << " '" << str << '\'';
        else
          Strings.Add(" '") << str << '\'';
      }
      else  {
        if( Strings.Last().String.Length() + str.Length() < 80 )
          Strings.Last().String << ' ' << str;
        else
          Strings.Add(' ') << str;
      }
    }
  }
  if( !FComments.IsEmpty() )
    Strings.Add(FComments);
}
//..............................................................................
olxstr TCifLoop::GetLoopName() const {
  if( FTable.ColCount() == 0 )  return EmptyString;
  if( FTable.ColCount() == 1 )  return FTable.ColName(0);
  
  olxstr C = olxstr::CommonString(FTable.ColName(0), FTable.ColName(1));
  for( size_t i=2; i < FTable.ColCount(); i++ )
    C = olxstr::CommonString(FTable.ColName(i), C);
  if( C.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "Mismatching loop columns");
  if( C.Last() == '_' )
    C.SetLength(C.Length()-1);
  return C;
}
//..............................................................................
olxstr TCifLoop::ShortColumnName(size_t in) const {
  return FTable.ColName(in).SubStringFrom(GetLoopName().Length());
}
//..............................................................................
int TCifLoop::CifLoopSorter::Compare(const TCifLoopTable::TableSort& r1,
                                     const TCifLoopTable::TableSort& r2)
{
  uint64_t id1 = 0, id2 = 0;
  size_t ac = 0;
  for( size_t i=r1.data.Count(); i > 0; i-- )  {
    bool atom = false;
    if( r1.data.GetObject(i-1)->GetAtomRef() != NULL )  {
      id1 |= ((uint64_t)(r1.data.GetObject(i-1)->GetAtomRef()->GetId()) << ac*16);
      atom = true;
    }
    if( r2.data.GetObject(i-1)->GetAtomRef() != NULL )  {
      id2 |= ((uint64_t)(r2.data.GetObject(i-1)->GetAtomRef()->GetId()) << ac*16);
      atom = true;
    }
    if( atom )  ac++;
  }
  return (id1 < id2 ? -1 : (id1 > id2 ? 1 : 0)); 
}
//..............................................................................
void TCifLoop::UpdateTable(const TCif& parent)  {
  if( FTable.RowCount() == 0 )  return;
  bool update = false;
  for( size_t i=0; i < FTable.ColCount(); i++ )  {
    if( FTable[0].GetObject(i)->GetAtomRef() != NULL )  {
      update = true;
      break;
    }
  }
  if( !update  )  return;
  for( size_t i=0; i < FTable.RowCount(); i++ )  {
    for( size_t j=0; j < FTable.ColCount(); j++ )  {
      if( FTable[i].GetObject(j)->GetAtomRef() != NULL )
        FTable[i][j] = FTable[i].GetObject(j)->GetAtomRef()->GetLabel();
    }
  }
  FTable.SortRows<CifLoopSorter>();
  size_t pi = FTable.ColIndex("_atom_site_disorder_group");
  if( pi != InvalidIndex )  {
    for( size_t i=0; i < FTable.RowCount(); i++ )
      if( FTable[i].GetObject(0)->GetAtomRef() != NULL &&
        FTable[i].GetObject(0)->GetAtomRef()->GetPart() != 0 )
      {
        FTable[i][pi] = (int)FTable[i].GetObject(0)->GetAtomRef()->GetPart();
      }
  }
}

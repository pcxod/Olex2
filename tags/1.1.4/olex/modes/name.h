#ifndef __OLX_NAME_MODE_H
#define __OLX_NAME_MODE_H

class TNameMode : public AModeWithLabels  {
  size_t Index;
  olxstr Prefix, Suffix, Symbol;
  TUndoData* FirstUndo;
  bool AutoComplete;
protected:
  static TNameMode* Instance;
  class TNameModeUndo : public TUndoData {
    TSizeList LabelIndeces;
  public:
    TNameModeUndo() : TUndoData(new TUndoActionImplMF<TNameModeUndo>(this, &TNameModeUndo::undo))  {  }
    TNameModeUndo(TXAtom& xa) : TUndoData(new TUndoActionImplMF<TNameModeUndo>(this, &TNameModeUndo::undo))  {  
      AddAtom(xa);
    }
    void AddAtom(TXAtom& xa)  {  LabelIndeces.Add(xa.GetXAppId());  }
    void undo(TUndoData* data)  {
      if( TNameMode::Instance != NULL )  {
        for( size_t i=0; i < LabelIndeces.Count(); i++ )  {
          TGlXApp::GetGXApp()->MarkLabel(LabelIndeces[i], false);
          TNameMode::Instance->Index--;
        }
        TNameMode::Instance->SetCursor();
      }
    }
  };
#ifdef __BORLANDC__
  friend class TNameModeUndo;
#endif
  void SetCursor()  {
    olxstr Labl( Symbol.IsEmpty() ? olxstr('$') : Symbol );
    TGlXApp::GetMainForm()->SetUserCursor(Labl << Prefix << Index << Suffix, "name");
  }
  void Autocomplete(TXAtom& xa, TNameModeUndo* undo)  {
    TGXApp& app = *TGlXApp::GetGXApp();
    for( size_t i=0; i < app.AtomCount(); i++ )
      app.GetAtom(i).Atom().SetTag(i);
    TXAtomPList outgoing;
    const TSAtom& sa = xa.Atom();
    for( size_t i=0; i < sa.NodeCount(); i++ )  {
      const TSAtom& nd = sa.Node(i);
      if( nd.IsDeleted() || nd.GetType() < 3.5 ) // H,D,Q
        continue;
      if( app.IsLabelMarked(app.GetAtom(nd.GetTag())) )
        continue;
      // 2009.07.17 --
      if( xa.Atom().GetType() != nd.GetType() )
        continue;
      // 2009.08.03 --
      if( xa.Atom().CAtom().GetPart() != nd.CAtom().GetPart() )
        continue;
      outgoing.Add( app.GetAtom(nd.GetTag()) );
    }
    if( outgoing.Count() == 1 )  {
      olxstr Labl (Symbol.IsEmpty() ? outgoing[0]->Atom().GetType().symbol : Symbol);
      Labl << Prefix <<  Index << Suffix;
      undo->AddAction(TGlXApp::GetGXApp()->Name(*outgoing[0], Labl, false));
      undo->AddAtom( *outgoing[0] );
      TGlXApp::GetGXApp()->MarkLabel(*outgoing[0], true);
      Index++;
      SetCursor();
      Autocomplete( *outgoing[0], undo );
    }
  }
public:
  TNameMode(size_t id) : AModeWithLabels(id)  {  Instance = this;  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    Index = Cmds.IsEmpty() ? 1 : Cmds[0].ToInt();
    Prefix = Options.FindValue('p');
    Suffix = Options.FindValue('s');
    Symbol = Options.FindValue('t');  // type
    AutoComplete = Options.Contains('a');
    bool typeSet = false;
    // validate if type is correct
    if( !Symbol.IsEmpty() && !XElementLib::IsElement(Symbol) )
      throw TInvalidArgumentException(__OlxSourceInfo, "element type");
    if( Cmds.IsEmpty() && !Symbol.IsEmpty() )
      Index = TGlXApp::GetGXApp()->GetNextAvailableLabel(Symbol);

    SetCursor();
    olxstr labels("labels -l");
    if( Symbol.Equalsi('H') || Symbol.Equalsi('D') )
      labels << " -h";
    TGlXApp::GetMainForm()->executeMacro(labels);
    TGXApp& app = *TGlXApp::GetGXApp();
    for( size_t i=0; i < app.BondCount(); i++ )
      app.GetBond(i).SetSelectable(false);
    return true;
  }
  ~TNameMode() {  Instance = NULL;  }
  void Finalise()  {
    TGXApp& app = *TGlXApp::GetGXApp();
    for( size_t i=0; i < app.BondCount(); i++ )
      app.GetBond(i).SetSelectable(true);
    app.XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom &XA = (TXAtom&)obj;
      olxstr Labl(Symbol.IsEmpty() ? XA.Atom().GetType().symbol : Symbol);
      Labl << Prefix <<  Index << Suffix;
      TNameModeUndo* undo = new TNameModeUndo(XA);
      undo->AddAction(TGlXApp::GetGXApp()->Name(XA, Labl, false));
      TGlXApp::GetMainForm()->GetUndoStack()->Push(undo);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      Index++;
      SetCursor();
      if( AutoComplete )
        Autocomplete(XA, undo);
      return true;
    }
    return false;
  }
};

#endif
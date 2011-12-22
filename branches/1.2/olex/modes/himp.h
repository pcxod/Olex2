/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_HIMP_MODE_H
#define __OLX_HIMP_MODE_H

class THimpMode : public AMode  {
  double BondLength;
protected:
public:
  THimpMode(size_t id) : AMode(id)  {}
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    BondLength = Cmds.IsEmpty() ? 0 : Cmds[0].ToDouble();
    if( BondLength <= 0.5 )  {
      TBasicApp::NewLogEntry(logError) << "suspicious bond length";
      return false;
    }
    TGlXApp::GetMainForm()->SetUserCursor("<->", olxstr(BondLength));
    return true;
  }
  void Finalise()  {
    TXApp::GetInstance().XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      if( XA.GetType() == iHydrogenZ )  {
        TSAtom* aa = NULL;
        for( size_t i=0; i < XA.NodeCount(); i++ )  {
          if( XA.Node(i).GetType().z > 2 )  {
            if( aa == NULL )  aa = &XA.Node(i);
            else  {  // bad connectivity
              aa = NULL;
              break;
            }
          }
        }
        if( aa != NULL )  {
          XA.crd() = aa->crd() + (XA.crd()-aa->crd()).NormaliseTo(BondLength);
          XA.ccrd() = TGlXApp::GetGXApp()->XFile().GetAsymmUnit()
            .Fractionalise(XA.crd());
          XA.CAtom().ccrd() = XA.ccrd();
          TGlXApp::GetGXApp()->MarkLabel(XA, true);
        }
      }
      else {
        for( size_t i=0; i < XA.NodeCount(); i++ )  {
          if (XA.Node(i).GetType().z != iHydrogenZ) continue;
          XA.Node(i).crd() = XA.crd() + (XA.Node(i).crd()-XA.crd())
            .NormaliseTo(BondLength);
          XA.Node(i).ccrd() = TGlXApp::GetGXApp()->XFile().GetAsymmUnit()
            .Fractionalise(XA.Node(i).crd());
          XA.Node(i).CAtom().ccrd() = XA.Node(i).ccrd();
          TGlXApp::GetGXApp()->MarkLabel(XA.Node(i), true);
        }
      }
      return true;
    }
    return false;
  }
};

#endif

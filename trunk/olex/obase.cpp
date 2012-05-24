/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "obase.h"

#include "modes/name.h"
#include "modes/match.h"

#include "ins.h"
#include "modes/split.h"
#include "modes/himp.h"
#include "modes/part.h"
#include "modes/hfix.h"
#include "modes/occu.h"
#include "modes/fixu.h"
#include "modes/fixc.h"

#include "modes/grow.h"
#include "xgrowpoint.h"
#include "modes/pack.h"
#include "modes/move.h"
#include "modes/fit.h"

bool TPartMode::HasInstance = false;
bool TOccuMode::HasInstance = false;
bool TFixUMode::HasInstance = false;
bool TFixCMode::HasInstance = false;
TNameMode* TNameMode::Instance = NULL;

AMode::AMode(size_t id)
  : Id(id), gxapp(TGXApp::GetInstance()),
    olex2(*olex::IOlexProcessor::GetInstance()),
    ObjectPicker(*this)
{
  TModeChange mc(Id, true);
  TModeRegistry::GetInstance().OnChange.Execute(NULL, &mc);
  gxapp.GetMouseHandler().SetInMode(true);
  gxapp.GetMouseHandler().OnObject.Add(&ObjectPicker);
}
//..............................................................................
bool AMode::ObjectPicker_::Execute(
  const IEObject *sender, const IEObject *data)
{
  const AGDrawObject *o = dynamic_cast<const AGDrawObject *>(data);
  if (o != NULL) {
    mode.OnObject(*const_cast<AGDrawObject *>(o));
    return true;
  }
  return false;
}
//..............................................................................
AMode::~AMode() {
  gxapp.GetMouseHandler().OnObject.Remove(&ObjectPicker);
  gxapp.GetMouseHandler().SetInMode(false);
  TModeChange mc(Id, false);
  TModeRegistry::GetInstance().OnChange.Execute(NULL, &mc);
  //reset the screen cursor
  olex2.processMacro("cursor()");
  gxapp.ClearLabelMarks();  // hide atom marks if any
}
//..............................................................................
//..............................................................................
//..............................................................................
AModeWithLabels::AModeWithLabels(size_t id) : AMode(id)  {
  LabelsVisible = gxapp.AreLabelsVisible();
  LabelsMode = gxapp.GetLabelsMode();
}
//..............................................................................
AModeWithLabels::~AModeWithLabels()  {
  gxapp.SetLabelsVisible(LabelsVisible);
  gxapp.SetLabelsMode(LabelsMode);
}
//..............................................................................
//..............................................................................
//..............................................................................
olxstr TModeRegistry::ModeChangeCB = "modechange";
TModeRegistry* TModeRegistry::Instance = NULL;

TModeRegistry::TModeRegistry()
  : OnChange(Actions.New("OnChange"))
{
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance = this;
  Modes.Add("name",   new TModeFactory<TNameMode>(Modes.Count()+1));
  Modes.Add("match",  new TModeFactory<TMatchMode>(Modes.Count()+1));
  Modes.Add("split",  new TModeFactory<TSplitMode>(Modes.Count()+1));
  Modes.Add("himp",   new TModeFactory<THimpMode>(Modes.Count()+1));
  Modes.Add("hfix",   new TModeFactory<THfixMode>(Modes.Count()+1));
  Modes.Add("part",   new TModeFactory<TPartMode>(Modes.Count()+1));
  Modes.Add("occu",   new TModeFactory<TOccuMode>(Modes.Count()+1));
  Modes.Add("fixu",   new TModeFactory<TFixUMode>(Modes.Count()+1));
  Modes.Add("fixxyz", new TModeFactory<TFixCMode>(Modes.Count()+1));
  Modes.Add("grow",   new TModeFactory<TGrowMode>(Modes.Count()+1));
  Modes.Add("pack",   new TModeFactory<TPackMode>(Modes.Count()+1));
  Modes.Add("move",   new TModeFactory<TMoveMode>(Modes.Count()+1));
  Modes.Add("fit",    new TModeFactory<TFitMode>(Modes.Count()+1));
  CurrentMode = NULL;
}
//..............................................................................
AMode* TModeRegistry::SetMode(const olxstr& name, const olxstr &args)  {
  AModeFactory* mf = Modes.Find(name, NULL);
  if( CurrentMode != NULL )  {
    CurrentMode->Finalise();
    delete CurrentMode;
  }
  CurrentMode = NULL;  // mf->New Calls other functions, validating current mode...
  CurrentMode = (mf == NULL) ? NULL : mf->New();
  if (CurrentMode != NULL || name.Equalsi("off")) {
    olex::IOlexProcessor *op = olex::IOlexProcessor::GetInstance();
    olxstr tmp = name;
    if (!args.IsEmpty()) tmp << ' ' << args;
    op->callCallbackFunc(ModeChangeCB, TStrList() << tmp);
  }
  return CurrentMode;
}
//..............................................................................
void TModeRegistry::ClearMode(bool finalise)  {
  if( CurrentMode == NULL )  return;
  if( finalise )  CurrentMode->Finalise();
  delete CurrentMode;
  CurrentMode = NULL;
}
//..............................................................................
TModeRegistry::~TModeRegistry()  {
  for( size_t i=0; i < Modes.Count(); i++ )
    delete Modes.GetValue(i);
  if( CurrentMode != NULL )
    delete CurrentMode;
  Instance = NULL;
}
//..............................................................................
size_t TModeRegistry::DecodeMode(const olxstr& mode)  {
  return GetInstance().Modes.IndexOf(mode) + 1;  // -1 +1 = 0 = mmNone
}
//..............................................................................
TModeRegistry &TModeRegistry::GetInstance() {
  if (Instance == NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised instance");
  return *Instance;
}
//..............................................................................
bool TModeRegistry::CheckMode(size_t mode) {
  TModeRegistry &inst = GetInstance();
  return inst.GetCurrent() == NULL ? false
    : inst.GetCurrent()->GetId() == mode;
}
//..............................................................................
bool TModeRegistry::CheckMode(const olxstr& mode) {
  return CheckMode(DecodeMode(mode));
}
//..............................................................................
//..............................................................................
//..............................................................................
TStateRegistry *TStateRegistry::Instance = NULL;
olxstr TStateRegistry::StateChangeCB = "statechange";
//..............................................................................
TStateRegistry::TStateRegistry()
 : OnChange(Actions.New("state_change"))
{
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance = this;
}
//..............................................................................
void TStateRegistry::SetState(size_t id, bool status, const olxstr &data,
  bool internal_call)
{
  TStateChange sc(id, status, data);
  OnChange.Execute(NULL, &sc);
  olex::IOlexProcessor *op = olex::IOlexProcessor::GetInstance();
  TStrList args;
  args.Add(slots[id]->name);
  args.Add(status);
  if (!data.IsEmpty())
    args.Add(data);
  op->callCallbackFunc(StateChangeCB, args);
  if (internal_call) return;
  slots[id]->Set(status, data);
}
//..............................................................................
void TStateRegistry::TMacroSetter::operator ()(bool v, const olxstr &data) {
  olxstr c = cmd;
  c << ' ' << v;
  if (!data.IsEmpty()) c << ' ' << data;
  olex::IOlexProcessor::GetInstance()->processMacro(c, __OlxSrcInfo);
}
//..............................................................................
TStateRegistry &TStateRegistry::GetInstance() {
  if (Instance == NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised instance");
  return *Instance;
}

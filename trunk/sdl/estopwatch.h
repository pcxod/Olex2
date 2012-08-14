/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_stop_watch_h
#define __olx_stop_watch_h
#include "etime.h"
#include "bapp.h"
#include "etable.h"
BeginEsdlNamespace()

class TStopWatchManager {
public:
  class Record {
  protected:
    TTypeList<Record> nodes;
    time_t creation_time;
    TIntList sequence;
  public:
    Record *parent;
    olxstr FunctionName;
    time_t termination_time;
    TTypeList<AnAssociation3<uint64_t,olxstr, uint64_t> > steps;
    Record(Record *parent, const olxstr &functionName)
      : parent(parent), FunctionName(functionName)
    {
      creation_time = TETime::msNow();
      termination_time = time_t(~0);
    }
    const TTypeList<Record> & GetNodes() const { return nodes; };
    Record &New(const olxstr &functionName) {
      sequence.Add(-int(nodes.Count()+1));
      return nodes.Add(new Record(this, functionName));
    }
    void start(const olxstr& name)  {
      if (!steps.IsEmpty() && steps.GetLast().GetC() == 0)
        steps.GetLast().SetC(TETime::msNow());
      steps.AddNew(TETime::msNow(), name, 0).B();
      sequence.Add(int(steps.Count()));
    }
    void stop()  {
      steps.GetLast().C() = TETime::msNow();
    }
    const_strlist prepareList(size_t level);
  };
protected:
  static Record *current;
  static void print();
public:
  static Record &Push(const olxstr &functionName);
  static void Pop();
};

class TStopWatch  {
  TStopWatchManager::Record &record;
public:
  TStopWatch(const olxstr& functionName)
    : record(TStopWatchManager::Push(functionName))
  {}
  ~TStopWatch() {
    record.termination_time = TETime::msNow();
    TStopWatchManager::Pop();
  }
  void start(const olxstr& name)  { record.start(name); }
  void stop() { record.stop(); }
};

EndEsdlNamespace()
#endif
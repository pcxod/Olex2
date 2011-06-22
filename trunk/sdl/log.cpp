/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "log.h"
#include "etime.h"

TLog::TLog() : 
  OnInfo(Actions.New("ONINF")),
  OnWarning(Actions.New("ONWRN")),
  OnError(Actions.New("ONERR")),
  OnException(Actions.New("ONEXC"))  {}
//..............................................................................
TLog::~TLog()  {
  for( size_t i=0; i < Streams.Count(); i++ )
    if( Streams[i].GetB() )
      delete Streams[i].GetA();
}
//..............................................................................
size_t TLog::Write(const void *Data, size_t size)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
//..............................................................................
TLog::LogEntry::LogEntry(TLog& _parent, int _evt, bool annotate) : parent(_parent), evt(_evt)  {
  if( annotate )
    buffer << "New log entry at: " << TETime::FormatDateTime(TETime::Now()) << NewLineSequence();
}
//..............................................................................
TLog::LogEntry::~LogEntry()  {
  buffer << NewLineSequence();
  olxstr res = buffer;
  if( evt == logDefault )  {
    parent << res;
    //parent.Flush();
  }
  else  {
    TActionQueue* ac = NULL;
    switch( evt )  {
    case logInfo: 
      ac = &parent.OnInfo;
      break;
    case logWarning: 
      ac = &parent.OnWarning;
      break;
    case logError: 
      ac = &parent.OnError;
      break;
    case logException: 
      ac = &parent.OnException;
      break;
    default: 
      ac = &parent.OnInfo;
      break;
    }
    if( !ac->Enter(&parent, &res) )  {
      parent << res;
      //parent.Flush();
    }
    ac->Exit(&parent);
  }
}
//..............................................................................

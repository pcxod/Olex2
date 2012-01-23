/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_bapp_H
#define __olx_bapp_H
#include "paramlist.h"
#include "os_util.h"
#include "library.h"
#include "log.h"
BeginEsdlNamespace()

// app event registry, these might not be implemented
static olxstr 
  olxappevent_GL_DRAW("GLDRAW"),
  olxappevent_GL_CLEAR_STYLES("GLDSCLEAR"),
  olxappevent_UPDATE_GUI("UPDATE_GUI");

class TBasicApp: public IEObject  {
  olxstr BaseDir, InstanceDir, SharedDir, ExeName;
protected:
  class TActionQList Actions;
  static TBasicApp* Instance;
  class TLog* Log;
  short MaxThreadCount;
  bool MainFormVisible, Profiling, BaseDirWriteable;
  static olx_critical_section app_cs;
  void ReadOptions(const olxstr &fn);
public:
  TParamList Options;
  TStrList Arguments;
  // the file name of the application with full path
  TBasicApp(const olxstr& AppName);
  virtual ~TBasicApp();

  /* instance dir dependents on the location of the executable and to be used
  to store instance specific data - updates etc.  If unset, the value is equal
  to the BaseDir
  */
  static const olxstr& GetSharedDir();
  /* will create the folder if does not exist, if fails - throws
  TFunctionfailedException. Shared dir is the same for all insatnces of
  Olex2 disregarding the location
  */
  static const olxstr& SetSharedDir(const olxstr& cd);
  static bool HasSharedDir() {  return !GetInstance().SharedDir.IsEmpty();  }
  const olxstr &_GetInstanceDir() const {
    return InstanceDir.IsEmpty() ? GetBaseDir() : InstanceDir;
  }
  /* sets instance dir - folder dependent on the exe location
  */
  void SetInstanceDir(const olxstr &d);
  static TLog& GetLog()  {  return *GetInstance().Log;  }
  static TLog::LogEntry NewLogEntry(int evt_type = logDefault,
    bool annotate=false)
  {
    return GetInstance().Log->NewEntry(evt_type, annotate);
  }
  /* if var_name is not NULL, tries to get its value and combine with file name
  of path if either are empty - the curent folder is used with exename
  'unknown.exe'
  */
  static olxstr GuessBaseDir(const olxstr& path,
    const olxstr& var_name=EmptyString());
  static const olxstr& GetBaseDir()  {  return GetInstance().BaseDir;  }
  /* instance dir dependents on the location of the executable and to be used
  to store insatnce specific data - updates etc.  If unset, the value is equal
  to the BaseDir
  */
  static const olxstr& GetInstanceDir()  {
    return GetInstance()._GetInstanceDir();
  }
  // this resets the ExeName to the file name of the bd
  static const olxstr& SetBaseDir(const olxstr& bd);
  static bool IsBaseDirWriteable() {  return GetInstance().BaseDirWriteable;  }
  // valid only if correct string is passed to the constructor
  static const olxstr& GetExeName()  {  return GetInstance().ExeName;  }
  static TBasicApp& GetInstance() {  
    if( Instance == NULL ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Uninitialised application layer...");
    }
    return *Instance;  
  }

  static bool HasInstance()  {  return Instance != NULL;  }

  TActionQueue& NewActionQueue(const olxstr& Name);
  TActionQueue* FindActionQueue(const olxstr& Name)  {  
    try  {  return Actions.Find(Name);   }
    catch(...)  {  return NULL;  }
  }

  const TActionQList& GetActions() const {  return Actions; }

  static bool IsProfiling()  {  return GetInstance().Profiling;  }
  static void SetProfiling(bool v)  {  GetInstance().Profiling = v;  }

  // implementation might consider drawing scene, update GUI etc..
  virtual void Update()  {}
  static size_t GetArgCount()  {  return GetInstance().Arguments.Count();  }
  static const olxstr& GetArg(size_t i)  {  return GetInstance().Arguments[i];  }
  // returns WIN32, WIN64, LINUX32, LINUX64, MAC etc
  static olxstr GetPlatformString();
  static bool Is64BitCompilation();
  TLibrary* ExportLibrary(const olxstr& lib_name="app");

  // application layer critical section
  inline static void EnterCriticalSection()  {  app_cs.enter();  }
  inline static void LeaveCriticalSection()  {  app_cs.leave();  }
  inline static olx_critical_section& GetCriticalSection() {  return app_cs;  }
  DefPropP(short, MaxThreadCount)
  TActionQueue& OnProgress;
  TActionQueue& OnTimer;
  TActionQueue& OnIdle;
};

EndEsdlNamespace()
#endif

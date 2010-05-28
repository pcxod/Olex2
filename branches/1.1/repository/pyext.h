#ifndef __olx_python_ext_H
#define __olx_python_ext_H
#include "macrolib.h"
#include "etime.h"
#include "etbuffer.h"

#if defined __APPLE__ && defined __MACH__
  #include "Python/python.h"
#else
  #ifdef _DEBUG
    #undef _DEBUG
    #include "Python.h"
    #define _DEBUG
  #else
    #include "Python.h"
  #endif
#endif
//---------------------------------------------------------------------------
using namespace olex;
typedef void (*pyRegFunc)();

class PythonExt  {
  static PythonExt* Instance;
  IOlexProcessor* OlexProcessor;
  TLibrary *Library, *BindLibrary;
  TTypeList<pyRegFunc> ToRegister;
//..............................................................................
  struct ProfileInfo  {
    int CallCount;
    uint64_t TotalTimeMs;
    ProfileInfo()  {  Reset();  }
    void Reset()   {  CallCount = 0;  TotalTimeMs = 0;  }
  };
//..............................................................................
public:
  class BasicWrapper : public IEObject  {
    PythonExt::ProfileInfo *PI;
    uint64_t StartTime;
    int Recursion;  // specifies that the function is called recursively
  protected:
    PyObject *PyFunction;
    inline bool IsProfilingEnabled()  const  {  return PI != NULL;  }
    inline void OnCallEnter()  {
      if( PI == NULL )  return;
      PI->CallCount++;
      if( ++Recursion > 1 )  return;
      StartTime = TETime::msNow();
     }
    inline void OnCallLeave()  {
      if( PI == NULL )  return;
      if( --Recursion > 0 )  return;
      uint64_t now = TETime::msNow();
      PI->TotalTimeMs += ((StartTime < now) ? (now - StartTime) : 0);
      Recursion = 0;
    }
    //..........................................................................
    void EnableProfiling(bool enable) {
      if( enable )  {
        if( PI == NULL )  PI = new PythonExt::ProfileInfo();
        else  PI->Reset();
      }
      else  {
        delete PI;
        PI = NULL;
      }
    }
    //..........................................................................
  public:
    BasicWrapper(bool enableProfiling)  {
      PI = NULL;
      StartTime = 0;
      Recursion = 0;
      EnableProfiling(enableProfiling);
    }
    //..........................................................................
    virtual ~BasicWrapper()  {  if( PI != NULL )  delete PI;  }
    inline PyObject* GetFunction() const {  return PyFunction;  }
    inline PythonExt::ProfileInfo *ProfileInfo() {  return PI;  }
  };
//..............................................................................
  TPtrList<BasicWrapper> ToDelete;
//..............................................................................
  void ClearToDelete()  {
    for( size_t i=0; i < ToDelete.Count(); i++ )
      delete ToDelete[i];
  }
//..............................................................................
  void macReset(TStrObjList& Cmds, const TParamList &Options, TMacroError& E);
  void macRun(TStrObjList& Cmds, const TParamList &Options, TMacroError& E);
  void funLogLevel(const TStrObjList& Params, TMacroError& E);
  PythonExt(IOlexProcessor* olexProcessor);
  uint16_t LogLevel;
public:
  ~PythonExt();
  // must be called only once
  static PythonExt& Init(IOlexProcessor* olexProcessor)  { return *(new PythonExt(olexProcessor));  }
  static void Finilise()  {
    if( Instance != NULL )  {
      delete Instance;
      Instance = NULL;
    }
  }
  int RunPython(const olxstr& script);
  DefPropP(uint16_t, LogLevel)
  template <class T>
    inline T * AddToDelete(T* td)  {  return (T*)ToDelete.Add(td);  }

  void Register( pyRegFunc regFunc )  {
    ToRegister.AddACopy(regFunc);
    if( Py_IsInitialized() )  {
      (*regFunc)();
    }
  }
  TLibrary* ExportLibrary(const olxstr& name=EmptyString);
//  static inline TLibrary* GetExportedLibrary()  {  return Library;  }
  inline TLibrary* GetBindLibrary()  {  return BindLibrary;  }
  IOlexProcessor* GetOlexProcessor()  {  return OlexProcessor;  }
  void CheckInitialised();

  static inline PythonExt* GetInstance()  {
    if( Instance == NULL )  throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised object");
    return Instance;
  }
  PyObject* GetProfileInfo();
// building string
  inline static PyObject* BuildString(const olxstr& str)  {
#ifdef _UNICODE
    return PyUnicode_FromWideChar(str.raw_str(), str.Length());
#else
    return Py_BuildValue("s#", str.raw_str(), str.Length());
#endif
  }
  inline static PyObject* BuildCString(const olxcstr& str)  {
    return Py_BuildValue("s#", str.raw_str(), str.Length());
  }
  inline static PyObject* BuildWString(const olxwstr& str) {
    return PyUnicode_FromWideChar(str.raw_str(), str.Length());
  }
// parsing string 
  inline static olxstr ParseStr(PyObject *pobj) {
    char*  crv;
    olxstr rv;
    if( pobj->ob_type == &PyString_Type && PyArg_Parse(pobj, "s", &crv) )  {
      rv = crv;
    }
    else if( pobj->ob_type == &PyUnicode_Type )  {
      size_t sz =  PyUnicode_GetSize(pobj);
      TTBuffer<wchar_t> wc_bf(sz+1);      
      sz = PyUnicode_AsWideChar((PyUnicodeObject*)pobj, wc_bf.Data(), sz);
      if( sz > 0 )
        rv.Append( wc_bf.Data(), sz);
    }
    else
      rv = PyObject_REPR(pobj);
    return rv;
  }
  // to tackle the difference with list and tuple... it steals the reference now
  static void SetDictItem(PyObject* dict, const char* field_name, PyObject* val)  {
    PyDict_SetItemString(dict, field_name, val);
    Py_DECREF(val);
  }
  static PyObject* SetErrorMsg(PyObject* err_type, const olxcstr& location, const olxstr& msg)  {
    PyObject* val = BuildString(olxstr(location) << ": " << msg);
    PyErr_SetObject(err_type, val);
    Py_DECREF(val);
    return PyNone();
  }
  static PyObject* SetErrorMsg(PyObject* err_type, const olxcstr& location, const char* msg)  {
    PyObject* val = Py_BuildValue("s: s", location.c_str(), msg);
    PyErr_SetObject(err_type, val);
    Py_DECREF(val);
    return PyNone();
  }
  static PyObject* InvalidArgumentException(const olxcstr& location, const char* msg)  {
    PyObject* val = Py_BuildValue("s: ss", location.c_str(), "invalid argument format for ", msg);
    PyErr_SetObject(PyExc_TypeError, val);
    Py_DECREF(val);
    return PyNone();
  }
  static PyObject* PyNone()  {  Py_INCREF(Py_None);  return Py_None;  }
  static PyObject* PyTrue()  {  Py_INCREF(Py_True);  return Py_True;  }
  static PyObject* PyFalse()  {  Py_INCREF(Py_False);  return Py_False;  }
  /* tuple parsing to process unicode and string in the same way...
    s# - char*, len
    s - char*
    w - olxstr
    i - int
    f - float
    O - PyObject
    | - optional params after column
  */
  static bool ParseTuple(PyObject* tuple, const char* format, ...);
};


#endif

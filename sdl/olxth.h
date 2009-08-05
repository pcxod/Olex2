#ifndef __olx_thread_H
#define __olx_thread_H

// sort out the gheader files
#ifdef __WIN32__
  #include <windows.h>
#undef Yield
#else
  #include <pthread.h>
  #include <errno.h>
#endif
#include "exception.h"
#include "bapp.h"
#include "actions.h"
#include "egc.h"

BeginEsdlNamespace()
// converts function (taking no arguents) to a thread - ready function
template <class T> struct ThreadFunctionConverter  {
#ifdef __WIN32__
  static unsigned long _stdcall Func(void* data) {
#else
  static void* Func(void* data) {
#endif
    ((T)(data))();
    return 0;
  }
};

class AOlxThread : public IEObject {
protected:
  int volatile RetVal;
  bool volatile Terminate, Detached, Running;
  //..................................................................................................
#ifdef __WIN32__
  struct HandleRemover  : public IEObject  {
    HANDLE handle;
    HandleRemover(HANDLE _handle) : handle(_handle) {}
    ~HandleRemover()  {
      if( handle != NULL )
        CloseHandle(handle);
    }
  };
  //..................................................................................................
  HANDLE Handle;
  //..................................................................................................
  static unsigned long _stdcall _Run(void* _instance) {
#else
  pthread_t Handle;
  static void* _Run(void* _instance) {
#endif
    ((AOlxThread*)_instance)->Running = true;
    ((AOlxThread*)_instance)->RetVal = ((AOlxThread*)_instance)->Run();
    // running prevents the object deletion...
    ((AOlxThread*)_instance)->Running = false;
    if( ((AOlxThread*)_instance)->Detached )
      delete (AOlxThread*)_instance;
#ifdef __WIN32__
    ExitThread(0);
#else
    pthread_exit(NULL);
#endif
    return 0;
  }
protected:  // do not allow to create externally
  //..................................................................................................
  TActionQList Actions;
  //..................................................................................................
  AOlxThread() : 
    Detached(true), 
    Terminate(false),
    Running(false),
    Handle(0), 
    RetVal(0) 
  {  
    OnTerminate = &Actions.NewQueue("ON_TERMINATE");  
  }
  //..................................................................................................
  /* thread can do some extras here, as it will be called from SendTerminate 
  before the Terimate flag is set */
  virtual void OnSendTerminate() {}
public:
  //..................................................................................................
  virtual ~AOlxThread()  {
    OnTerminate->Execute(this, NULL);
    if( Running )  {  // prevent deleting
      Detached = false;
      Terminate = true;
    }
    while( Running )
      TBasicApp::Sleep(50);
#ifdef __WIN32__  // costed me may restarts...
    if( Handle != NULL )
      CloseHandle(Handle);
#else
#endif
  }
  //..................................................................................................
  TActionQueue* OnTerminate;
  //..................................................................................................
  /* It is crutial to check if the terminate flag is set. In that case the function should
  return a value, or a deadlock situation may arise. */
  virtual int Run() = 0;
  //..................................................................................................
  bool Start() {
#ifdef __WIN32__
    unsigned long thread_id;
    Handle = CreateThread(NULL, 0, _Run, this, 0, &thread_id);
    if( Handle == NULL )  
      return false;
#else  
    return (pthread_create(&Handle, NULL, _Run, this) == 0);
#endif
    return true;
  }
  //..................................................................................................
  /* returns true if successful, the process calling Join is responsible for the
  memory deallocation... */
  bool Join(bool send_terminate = false)  {
    if( Handle == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "the tread must be started at first");
    Detached = false;
    if( send_terminate && !Terminate )
      SendTerminate();
#ifdef __WIN32__
    unsigned long ec = STILL_ACTIVE, rv;
    while( ec == STILL_ACTIVE && (rv=GetExitCodeThread(Handle, &ec)) != 0 )  {
      if( SwitchToThread() == 0 )
        TBasicApp::Sleep(50);
    }
    return rv != 0;
#else  
    if( pthread_join(Handle, NULL) != 0 )
      return false;
#endif
    return true;
  }
  //..................................................................................................
  // this only has effect if the main procedure of the thread checks for this flag...
  void SendTerminate()  {  
    OnSendTerminate(); 
    Terminate = true;  
  }
  //..................................................................................................
  /* executes a simplest function thread, the function should not take any arguments
  the return value will be ignored. To be used for global detached threads such timers etc.
  Simplest example:
    void TestTh()  {  your code here...  }
    AOlxThread::RunThread(&TestTh);
  */
  template <class T> static bool RunThread(T f)  {
#ifdef __WIN32__
    unsigned long thread_id;
    HANDLE h = CreateThread(NULL, 0, &ThreadFunctionConverter<T>::Func, f, 0, &thread_id);
    if( h == NULL )  
      return false;
    TEGC::AddP( new HandleRemover(h) );  // make sure it gets deleted at the end!
#else  
    pthread_t thread_id;
    return (pthread_create(&thread_id, NULL, &ThreadFunctionConverter<T>::Func, (void*)(f)) == 0);
#endif
    return true;
  }
  //..................................................................................................
  static void Yield()  {
#ifdef __WIN32__
    SwitchToThread();
#else
    sched_yield();
#endif
  }
  //..................................................................................................
  static unsigned long GetCurrentThreadId()  {
#ifdef __WIN32__
    return ::GetCurrentThreadId();
#else
    return (unsigned long)pthread_self();
#endif
  }
};
// http://en.wikipedia.org/wiki/Critical_section
struct olx_critical_section  {
#ifdef __WIN32__
  CRITICAL_SECTION cs;
  olx_critical_section()  {
    InitializeCriticalSection(&cs);  
  }
  ~olx_critical_section()  {
    DeleteCriticalSection(&cs);
  }
  bool tryEnter()  {  return TryEnterCriticalSection(&cs) != 0;  }
  void enter() {  EnterCriticalSection(&cs);  }
  void leave() {  LeaveCriticalSection(&cs);  }
#else
  pthread_mutex_t cs;
  olx_critical_section() {
    pthread_mutex_init(&cs, NULL);
  }

  ~olx_critical_section()  {
    pthread_mutex_destroy(&cs);
  }
  bool tryEnter()  {  return pthread_mutex_trylock(&cs) != EBUSY;  }
  void enter() {  pthread_mutex_lock(&cs);  }
  void leave() {  pthread_mutex_unlock(&cs);  }
#endif
};
// 'scope critical section'
struct olx_scope_cs  {
protected:
  olx_critical_section cs;
public:
  olx_scope_cs()  {  cs.enter();  }
	~olx_scope_cs() {  cs.leave();  }
};

EndEsdlNamespace()
#endif

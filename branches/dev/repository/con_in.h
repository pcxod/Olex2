#ifndef __olx_console_interface_H
#define __olx_console_interface_H
#ifdef __WIN32__ 
#include <windows.h>
const int 
  fgcReset  = 0,
  fgcRed   = FOREGROUND_RED,
  fgcGreen = FOREGROUND_GREEN,
  fgcBlue  = FOREGROUND_BLUE,
  fgcIntensity = FOREGROUND_INTENSITY;
const int
  bgcRed   = BACKGROUND_RED,
  bgcGreen = BACKGROUND_GREEN,
  bgcBlue  = BACKGROUND_BLUE,
  bgcIntensity = BACKGROUND_INTENSITY;
#else
const int
  fgcReset = 0,
  fgcRed   = 31,
  fgcGreen = 32,
  fgcBlue  = 34,
  fgcIntensity = 1;
const int
  bgcRed   = 41,
  bgcGreen = 42,
  bgcBlue  = 44,
  bgcIntensity = 1;
#endif

#include "estack.h"
/*
class IConsoleInterface  {
public:
  virtual ~IConsoleInterface() {  }
  virtual void SetTextBackground(const int, bool intensity=false) = 0;
  virtual void SetTextForeground(const int, bool intensity=false) = 0;
};
*/
#ifdef __WIN32__
class ConcoleInterface  { // : public IConsoleInterface {
  HANDLE conin, conout;
  TStack<CONSOLE_SCREEN_BUFFER_INFO> TextAttrib;
public:
  ConcoleInterface()  {
    conin = CreateFile(olx_T("CONIN$"), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
    conout  = CreateFile(olx_T("CONOUT$"), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CONSOLE_SCREEN_BUFFER_INFO pi;
    GetConsoleScreenBufferInfo(conout, &pi); 
    TextAttrib.Push(pi);
  }
  ~ConcoleInterface()  {
    CloseHandle(conin);
    CloseHandle(conout);
  }
  void Push()  {
    CONSOLE_SCREEN_BUFFER_INFO pi;
    GetConsoleScreenBufferInfo(conout, &pi); 
    TextAttrib.Push(pi);
  }
  void Pop()  {
    SetConsoleTextAttribute(conout, TextAttrib.Pop().wAttributes); 
  }
  void SetTextBackground(const int cl, bool intensity=false)  {
    if( intensity ) 
      SetConsoleTextAttribute(conout, cl|FOREGROUND_INTENSITY);
    else
      SetConsoleTextAttribute(conout, cl|FOREGROUND_INTENSITY);
  }
  void SetTextForeground(const int cl, bool intensity=false)  {
    SetTextBackground(cl, intensity);
  }
};
#else
class ConcoleInterface  { // : public IConsoleInterface {
public:
  ConcoleInterface()  {
  
  }
  ~ConcoleInterface()  {
  }
  void Push()  {
  }
  void Pop()  {  // just reset the values to default
    cout << "\033[0";
  }
  void SetTextBackground(const int cl, bool intensity=false)  {
    cout << "\033[" << cl;
    if( intensity ) 
      cout << ";1";
    cout << "m";
  }
  void SetTextForeground(const int cl, bool intensity=false)  {
    SetTextBackground(cl, intensity);
  }
};
#endif  // __WIN32__

#endif
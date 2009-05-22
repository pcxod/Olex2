#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "ipbase.h"
#ifndef __GNUC__
class TestClass {
public:
  void vf(void)  {  return;  }
  void vf1(int a)  {  return;  }
  void vf2(int a, int b)  {  return;  }
  void vf3(int a, int b, int c)  {  return;  }
  void vf4(int a, int b, int c, int d)  {  return;  }

  bool f()  {  return true;  }
  bool f1(int a)  {  return true;  }
  bool f2(int a, int b)  {  return true;  }
  bool f3(int a, int b, int c)  {  return true;  }
  bool f4(int a, int b, int c, int d)  {  return true;  }
};
  void vf()  {  return;  }
  void vf1(int a)  {  return;  }
  void vf2(int a, int b)  {  return;  }
  void vf3(int a, int b, int c)  {  return;  }
  void vf4(int a, int b, int c, int d)  {  return;  }

  bool f()  {  return true;  }
  bool f1(int a)  {  return true;  }
  bool f2(int a, int b)  {  return true;  }
  bool f3(int a, int b, int c)  {  return true;  }
  bool f4(int a, int b, int c, int d)  {  return true;  }

void TFuncRegistry::CompileTest()  {
  TFuncRegistry fr;
  TestClass tc;

  fr.Reg("vf", &vf);
  fr.Reg("vf1", &vf1);
  fr.Reg("vf2", &vf2);
  fr.Reg("vf3", &vf3);
  fr.Reg("vf4", &vf4);

  fr.Reg("f", &f);
  fr.Reg("f1", &f1);
  fr.Reg("f2", &f2);
  fr.Reg("f3", &f3);
  fr.Reg("f4", &f4);

  //fr.Reg<TestClass>("vf", &tc, &TestClass::vf);
  fr.Reg<TestClass,int>("vf1", &tc, &TestClass::vf1);
  fr.Reg<TestClass,int,int>("vf2", &tc, &TestClass::vf2);
  fr.Reg<TestClass,int,int,int>("vf3", &tc, &TestClass::vf3);
  fr.Reg<TestClass,int,int,int,int>("vf4", &tc, &TestClass::vf4);

  fr.CallFunction<void>("f");
  fr.CallFunction<void, int>("f", 1);
  fr.CallFunction<void, int, int>("f", 1, 1);
  fr.CallFunction<void, int, int, int>("f", 1, 1, 1);
  fr.CallFunction<void, int, int, int, int>("f", 1, 1, 1, 1);

  bool v;
  v = fr.CallFunction<bool>("f");
  v = fr.CallFunction<bool, int>("f", 1);
  v = fr.CallFunction<bool, int, int>("f", 1, 1);
  v = fr.CallFunction<bool, int, int, int>("f", 1, 1, 1);
  v = fr.CallFunction<bool, int, int, int, int>("f", 1, 1, 1, 1);
}
#endif

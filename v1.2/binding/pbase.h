#ifndef pbaseH
#define pbaseH

#include "estlist.h"
#include "talist.h"
#include "estrlist.h"
#include "tptrlist.h"
#include "bapp.h"
#include "log.h"
#undef GetObject

static const TEString& BNullStr = *(TEString*)NULL;
// function modifiers
const short bfmInline   = 0x0001,
            bfmStatic   = 0x0002,
            bfmVirtual  = 0x0004;
// type modifiers
const short btmConst          = 0x0001, // const int
            btmConst1         = 0x0002, // int const*
            btmReference      = 0x0004, // int&
            btmPointer        = 0x0008, // int*
            btmArray          = 0x0010; // int[]

// forward references
class BFunction;
class BObjects;


class BTemplate : public AReferencible {
  TStrPObjList<TEString*> Args;
public:
  BTemplate()  { ; }
  BTemplate(const BTemplate& tmpl)  {
    for( int i=0; i < tmpl.Args.Count(); i++ )  {
      if( tmpl.Args.Object(i) != NULL )
        Args.Add( tmpl.Args[i], new TEString(tmpl.Args.Object(i)) );
      else
        Args.Add( tmpl.Args[i], NULL );
    }
  }
  ~BTemplate()  {
    for( int i=0; i < Args.Count(); i++ )
      if( Args.Object(i) != NULL )
        delete Args.Object(i);
  }
  void AddArg(const TEString& name, const TEString& value=BNullStr)  {
    if( &value == NULL )
      Args.Add(name, NULL);
    else
      Args.Add(name, new TEString(value) );
  }
  inline int ArgCount() const {  return Args.Count(); }
  inline const TEString& GetArgName(int i)  const {  return Args[i];  }
  inline TEString* GetArgValue(int i)  const {  return Args.Object(i);  }

  TEString ToCString()  const  {
    TEString rv("template <");
    for( int i=0; i < Args.Count(); i++ )  {
      rv << Args[i];
      if( Args.Object(i) != NULL )
        rv << '=' << *Args.Object(i);
      if( (i+1) < Args.Count() )
        rv << ',';
    }
    return rv << '>';
  }
};

class BType : public AReferencible {
protected:
  TEString Type;
  short Modifiers;
  BType()  {  Modifiers = 0;  }
public:
  BType(const TEString& type, short modifiers=0) : Type(type)  {
    Modifiers = modifiers;
  }
  BType(const BType& type, short modifiers=0 ) : Type(type.Type) {
    Modifiers = modifiers;
  }
  virtual ~BType()  {  }

  inline const TEString& GetType()  const  {  return Type;  }
  inline void SetType(const TEString& t)   {  Type = t;  }
  inline const BType& operator = (const BType& t)  {
    Type = t.Type;
    return t;
  }
  inline bool operator == (const BType& t)  {  return Type.Compare( t.Type ) == 0;  }
  inline bool operator != (const BType& t)  {  return Type.Compare( t.Type ) != 0;  }
  inline int Compare(const BType& t)        {  return Type.Compare( t.Type );  }
  inline bool IsConst()  const  {  return (Modifiers&btmConst)!=0; }
  inline bool IsConst1() const  {  return (Modifiers&btmConst1)!=0; }
  inline bool IsReference() const  {  return (Modifiers&btmReference)!=0; }
  inline bool IsPointer() const  {  return (Modifiers&btmPointer)!=0; }
  inline bool IsArray() const  {  return (Modifiers&btmArray)!=0; }
  // mod might refers to Type<mod>
  virtual TEString ToCString(const TEString& mod, int arraySize=-1) const {
    TEString rv;
    if( (Modifiers&btmConst) != 0 )
      rv << "const " << Type;
    else
      rv << Type;
    if( !mod.IsEmpty() )
      rv << '<' << mod << '>';
    if( (Modifiers&btmPointer) != 0 )    rv << " *";
    if( (Modifiers&btmConst1) != 0 )
      rv << " const";
    if( (Modifiers&btmReference) != 0 )  rv << " &";
    if( (Modifiers&btmArray) != 0 )  {
      if( arraySize != -1 )
        rv << " [" << arraySize << ']';
      else
        rv << " []";
    }
    return rv;
  }
};

class BObject : public BType {
  TSStrPObjList<BFunction*, true> Functions;
  BTemplate* Template;
public:
  BObject(const TEString& type, BTemplate* templ = NULL, short modifiers=0) : BType(type, modifiers)  {
    Template = templ;
    if( Template != NULL )  Template->IncRef();
  }
  BObject(const BObject& obj) : BType(*this)  {
    Template = obj.Template;
    if( Template != NULL )  Template->IncRef();
  }
  virtual ~BObject();
  // an instance created with new must be provided
  void AddFunction(BFunction* func);
  inline bool IsTemplate()  const {  return Template != NULL;  }
  inline int FunctionCount()  const {  return Functions.Count();  }
  inline const BFunction& Function(int i) const {  return *Functions.GetObject(i);  }

  virtual TEString ToCString(const TEString& mod, int arraySize=-1) const {
    return BType::ToCString(mod, arraySize);
  }
  void WriteDefinition(TStrList& out) const;
};

class BTemplateSpecialisation {
  BType* Specialisation;
  BObject* Object;
public:
  BTemplateSpecialisation(BObject* object, BType* specialisation)  {
    Object = object;
    Specialisation = specialisation;
    Object->IncRef();
    Specialisation->IncRef();
  }
  virtual ~BTemplateSpecialisation()  {
    if( Object->DecRef() == 0 )  delete Object;
    if( Specialisation->DecRef() == 0 ) delete Specialisation;
  }
  virtual TEString ToCString(int arraySize=-1) const {
    return Object->ToCString(Specialisation->GetType(), arraySize);
  }
};

class BArg {
  TEString *DefVal, Name;
  BObject* Object;
public:
  BArg(BObject* object, const TEString& name, const TEString& defval = BNullStr) {
    DefVal = (&defval != NULL) ? new TEString(defval) : NULL;
    Name = name;
    Object = object;
    Object->IncRef();
  }
  virtual ~BArg()  {
    if( DefVal != NULL )  delete DefVal;
    if( Object->DecRef() == 0 )  delete Object;
  }
  inline bool HasDefVal() const  {  return DefVal != NULL;  }
  inline const TEString& GetDefVal() const  {  return *DefVal;  }
  inline const TEString& GetName()   const  {  return Name;  }

  TEString ToCString(int arraySize = -1)  const {
    TEString rv( Object->ToCString(EmptyString, arraySize) );
    rv << ' ' << Name;
    if( DefVal != NULL )
      rv << '=' << *DefVal;
    return rv;
  }
};

class BFunction : public AReferencible {
  TPtrList< BArg > Args;
  BObject* RetVal;
  TEString Name;
  short Modifiers;
  BTemplate* Template;
public:
  BFunction(BObject* retVal, const TEString& name, BTemplate* templ = NULL, short modifiers=0);
  virtual ~BFunction()  {
    if( Template != NULL && Template->DecRef() == 0 )
      delete Template;
    if( RetVal->DecRef() == 0 )
      delete RetVal;
    for(int i=0; i < Args.Count(); i++ )
      delete Args[i];
  }
  inline const TEString& GetName() const {  return Name;  }
  inline int ArgCount() const {  return Args.Count();  }
  inline const BObject& GetRetVal() const  {  return *RetVal; }
  inline void AddArg(BArg* arg)  {  Args.Add(arg); }

  TEString ToCHString() const  {
    TEString rv( (Template != NULL) ? Template->ToCString() : EmptyString );
    if( (Modifiers&bfmVirtual) != 0 )
      rv << " virtual";
    if( (Modifiers&bfmInline) != 0 )
      rv << " inline";
    if( (Modifiers&bfmStatic) != 0 )
      rv << " static";
    rv << RetVal->ToCString(EmptyString);
    rv << ' ' << Name << '(';
    for( int i=0; i < Args.Count(); i++ )  {
      rv << Args[i]->ToCString();
      if( (i+1) < Args.Count() )
        rv << ',';
    }
    rv << ')' << ';';
    return rv;
  }

  TEString Declare() const  {
    TEString rv( (Template != NULL) ? Template->ToCString() : EmptyString );
    if( (Modifiers&bfmVirtual) != 0 )
      rv << " virtual";
    if( (Modifiers&bfmInline) != 0 )
      rv << " inline";
    if( (Modifiers&bfmStatic) != 0 )
      rv << " static";
    rv << RetVal->ToCString(EmptyString);
    rv << ' ' << Name << '(';
    for( int i=0; i < Args.Count(); i++ )  {
      rv << Args[i]->ToCString();
      if( (i+1) < Args.Count() )
        rv << ',';
    }
    rv << ')' << ' ' << '{';
    return rv;
  }
  TEString CallStr() const  {
    TEString rv(Name);
    rv << '(';
    for( int i=0; i < Args.Count(); i++ )  {
      rv << Args[i]->GetName();
      if( (i+1) < Args.Count() )
        rv << ',';
    }
    rv << ')';
    return rv;
  }
};


class BObjects  {
  BObject* VoidType;
  TSStrPObjList<BObject*, true> Objects;  // all objects
  TSStrPObjList<BFunction*, true> Functions;  // all global functions
  BTemplate* ParseTypeid(const char* ti, TEString& type, short& modifiers)  {
    int stlen = strlen(ti);
    TEString tmpl;
    type.SetCapacity( stlen);
    for( int i=0; i < stlen; i++ )  {
      if( ti[i] == '<' )  {
        i++;
        while( i < stlen && ti[i] != '>')  {
          tmpl += ti[i];
          i++;
        }
        //TBasicApp::GetLog()->CriticalInfo(tmpl);
        i++;
        continue;
      }
      type += ti[i];
    }
    return NULL;
  }
  // typedefs
public:
  BObjects();
  ~BObjects();
  inline int ObjectCount() const {  return Objects.Count();  }
  inline BObject& Object(int i)  { return *Objects.Object(i);  }
  BObject* FindObject(const TEString& name)  {
    int ind = Objects.IndexOfComparable(name);
    return (ind==-1) ? NULL : Objects.Object(ind);
  }
  inline void AddObject(BObject* obj)  {
    Objects.Add( obj->GetType(), obj );
  }
  template <class T>
    BObject* FindObject()  {
      TEString type;
      short mods = 0;
      BTemplate* templ = ParseTypeid( typeid(T).name(), type, mods );
      return FindObject(type);
    }
  template <class T>
    BObject* NewObject()  {
      TEString type;
      short mods = 0;
      BTemplate* templ = ParseTypeid( typeid(T).name(), type, mods );
      BObject* obj = new BObject(type, templ, mods);
      AddObject( obj ) ;
      return obj;
    }

  inline int FunctionCount() const {  return Functions.Count();  }
  inline BFunction& Function(int i)  { return *Functions.Object(i);  }
  inline void AddFunction(BFunction* func)  {
    Functions.Add( func->GetName(), func );
    func->IncRef();
  }
  void PyBind(BObject* obj, TStrList& out);
};

#endif

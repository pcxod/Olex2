/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_base_H
#define __olx_base_H
#include "estlist.h"
#include "integration.h"
#include "gxapp.h"
#undef Status

class TStateChange: public IEObject {
  const size_t State;
  const bool Status;
  const olxstr Data;
public:
  TStateChange(size_t state, bool status, const olxstr& data=EmptyString())
  : State(state), Status(status), Data(data)
  {}
  bool GetStatus() const {  return Status;  }
  size_t GetState() const {  return State;  }
  const olxstr& GetData() const {  return Data;  }
};

class TStateRegistry {
public:
  struct IGetter {
    virtual ~IGetter() {}
    virtual bool operator ()(const olxstr &) const = 0;
  };

  template <class base_t>
  struct TMemberFunctionGetter_0 : public IGetter {
    const base_t &instance;
    bool (base_t::*getter)() const;
    TMemberFunctionGetter_0(const base_t &instance,
      bool (base_t::*getter)() const)
      : instance(instance), getter(getter)
    {}
    virtual bool operator ()(const olxstr &data) const {
      if (!data.IsEmpty())
        throw TInvalidArgumentException(__OlxSourceInfo, "data");
      return (instance.*getter)();
    }
  };
  template <class base_t>
  struct TMemberFunctionGetter_1 : public IGetter {
    const base_t &instance;
    bool (base_t::*getter)(const olxstr &) const;
    TMemberFunctionGetter_1(const base_t &instance,
      bool (base_t::*getter)(const olxstr &) const)
      : instance(instance), getter(getter)
    {}
    virtual bool operator ()(const olxstr &data) const {
      return (instance.*getter)(data);
    }
  };
  template <class base_t>
  struct TMemberFunctionGetter_2 : public IGetter {
    const base_t &instance;
    const size_t state_id;
    bool (base_t::*getter)(size_t id, const olxstr &) const;
    TMemberFunctionGetter_2(const base_t &instance, size_t state_id,
      bool (base_t::*getter)(size_t, const olxstr &) const)
      : instance(instance), state_id(state_id), getter(getter)
    {}
    virtual bool operator ()(const olxstr &data) const {
      return (instance.*getter)(state_id, data);
    }
  };

  struct ISetter {
    virtual ~ISetter() {}
    virtual void operator ()(bool, const olxstr &) = 0;
  };
  template <class base_t>
  struct TMemberFunctionSetter_1 : public ISetter {
    base_t &instance;
    void (base_t::*setter)(bool v);
    TMemberFunctionSetter_1(base_t &instance,
      void (base_t::*setter)(bool v))
      : instance(instance), setter(setter)
    {}
    virtual void operator ()(bool v, const olxstr &data) {
      if (!data.IsEmpty())
        throw TInvalidArgumentException(__OlxSourceInfo, "data");
      (instance.*setter)(v);
    }
  };
  template <class base_t>
  struct TMemberFunctionSetter_2 : public ISetter {
    base_t &instance;
    void (base_t::*setter)(bool v, const olxstr &);
    TMemberFunctionSetter_2(base_t &instance,
      void (base_t::*setter)(bool v, const olxstr &))
      : instance(instance), setter(setter)
    {}
    virtual void operator ()(bool v, const olxstr &data) {
      (instance.*setter)(v, data);
    }
  };
  struct TMacroSetter : public ISetter {
    olxstr cmd;
    TMacroSetter(const olxstr &cmd) : cmd(cmd)
    {}
    virtual void operator ()(bool v, const olxstr &data);
  };
  struct TNoneSetter : public ISetter {
    TNoneSetter() {}
    virtual void operator ()(bool v, const olxstr &data) {
      throw TNotImplementedException(__OlxSourceInfo);
    }
  };
  struct Slot {
    IGetter &getter;
    ISetter &setter;
    olxstr name;
    Slot(IGetter *getter, ISetter *setter)
      : getter(*getter), setter(*setter)
    {}
    ~Slot() {
      delete &getter;
      delete &setter;
    }
    virtual bool Get(const olxstr &data) const { return getter(data); }
    virtual void Set(bool v, const olxstr & data) { setter(v, data); }
  };
private:
  TPtrList<Slot> slots;
  olxstr_dict<size_t, true> slots_d;
  TActionQList Actions;
  static olxstr StateChangeCB;
  static TStateRegistry *Instance;
public:
  TStateRegistry();
  ~TStateRegistry() {
    slots.DeleteItems(false);
  }
  size_t Register(const olxstr &str_repr, Slot *slot) {
    slots.Add(slot)->name = str_repr;
    return slots_d.Add(str_repr, slots.Count()-1);
  }
  template <class base_t> static IGetter *NewGetter(const base_t &inst,
    bool (base_t::*getter)() const)
  {
    return new TMemberFunctionGetter_0<base_t>(inst, getter);
  }
  template <class base_t> static IGetter *NewGetter(const base_t &inst,
    bool (base_t::*getter)(const olxstr &) const)
  {
    return new TMemberFunctionGetter_1<base_t>(inst, getter);
  }
  // special shortcut - the ID as assumed to be generated next step
  template <class base_t> IGetter *NewGetter(const base_t &inst,
    bool (base_t::*getter)(size_t, const olxstr &) const) const
  {
    return new TMemberFunctionGetter_2<base_t>(inst, slots.Count(), getter);
  }
  template <class base_t> static IGetter *NewGetter(const base_t &inst,
    size_t state_id, bool (base_t::*getter)(size_t, const olxstr &) const)
  {
    return new TMemberFunctionGetter_2<base_t>(inst, state_id, getter);
  }

  template <class base_t> static ISetter *NewSetter(base_t &inst,
    void (base_t::*setter)(bool v, const olxstr &))
  {
    return new TMemberFunctionSetter_2<base_t>(inst, setter);
  }
  template <class base_t> static ISetter *NewSetter(base_t &inst,
    void (base_t::*setter)(bool v))
  {
    return new TMemberFunctionSetter_1<base_t>(inst, setter);
  }

  bool CheckState(size_t id, const olxstr &data=EmptyString()) {
    return slots[id]->Get(data);
  }
  bool CheckState(const olxstr &str_id, const olxstr &data=EmptyString()) {
    return slots[slots_d[str_id]]->Get(data);
  }
  void SetState(size_t id, bool status, const olxstr &data=EmptyString(),
    bool internal_call=false);

  TActionQueue &OnChange;
  static TStateRegistry &GetInstance();
};

class AMode : public IEObject {
protected:
  size_t Id;
  TGXApp &gxapp;
  olex::IOlexProcessor &olex2;
  void SetUserCursor(const olxstr &val, const olxstr &name) const{
    olxstr v = val;
    v.Replace('$', "\\$");
    olex2.processMacro(olxstr("cursor(user,") << v << ',' << name << ')');
  }
public:
  AMode(size_t id);
  virtual ~AMode();
  // mode initialisation
  virtual bool Initialise(TStrObjList &Cmds, const TParamList &Options) = 0;
  virtual void Finalise() = 0;
  // an action to be exected then any particular object is selected/clicked
  virtual bool OnObject(AGDrawObject &obj) = 0;
  /* if the mode holds any reference to graphical objects - these should be
  cleared
  */
  virtual void OnGraphicsDestroy()  {}
  //if the mode processes the key - true should be returned to skip the event
  virtual bool OnKey(int keyId, short shiftState)  {  return false;  }
  // if the function supported - returns true
  virtual bool AddAtoms(const TPtrList<TXAtom>& atoms) {  return false;  }
  inline size_t GetId() const {  return Id;  }
};
//..............................................................................
class AModeWithLabels : public AMode {
  short LabelsMode;
  bool LabelsVisible;
public:
  AModeWithLabels(size_t id);
  ~AModeWithLabels();
};
//..............................................................................

class AModeFactory  {
protected:
  size_t modeId;
public:
  AModeFactory(size_t id) : modeId(id)  {}
  virtual ~AModeFactory()  {}
  virtual AMode* New() = 0;
};

template <class ModeClass> class TModeFactory : public AModeFactory {
public:
  TModeFactory(size_t id) : AModeFactory(id) {}
  virtual AMode* New()  {  return new ModeClass(modeId);  }
};

class TModeRegistry  {
  olxstr_dict<AModeFactory*, true> Modes;
  AMode *CurrentMode;
  TActionQList Actions;
  static olxstr ModeChangeCB;
  static TModeRegistry *Instance;
public:
  TModeRegistry();
  ~TModeRegistry();
  // NULL is returned of no mode found
  AMode* SetMode(const olxstr& name, const olxstr &args);
  void ClearMode(bool finalise);
  static size_t DecodeMode(const olxstr& mode);
  AMode* GetCurrent() const {  return CurrentMode;  }
  TActionQueue &OnChange;

  static TModeRegistry &GetInstance();
  static bool CheckMode(size_t mode);
  static bool CheckMode(const olxstr &mode);
};

class TModeChange: public IEObject  {
  bool FStatus;
  size_t Mode;
public:
  TModeChange(size_t mode, bool status) : FStatus(status), Mode(mode) {}
  ~TModeChange()  {}
  bool GetStatus() const {  return FStatus;  }
};
//..............................................................................
#endif

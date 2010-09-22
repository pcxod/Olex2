#ifndef __olx_x_scatterer_H
#define __olx_x_scatterer_H
#include "chemdata.h"

BeginXlibNamespace()
/* scatterer wrapping to allow user defined values. */
class XScatterer {
  static const short
    setGaussian   = 0x0001,
    setDispersion = 0x0002,
    setMu         = 0x0004,
    setR          = 0x0008,
    setWt         = 0x0010,
    setAll = setGaussian|setDispersion|setMu|setR|setWt;
  cm_Gaussians gaussians;
  double mu, wt, r;
  compd fpfdp;
  const cm_Element* source;
  olxstr Label;
  short set_items;
public:
  // creates a dummy scatterer
  XScatterer(const olxstr& lbl) : Label(lbl), mu(0), r(0), wt(0), source(NULL), set_items(0)  {}
  // creates scatterer from the library
  XScatterer(const cm_Element& src, double energy) : mu(0), set_items()  {  
    SetSource(src, energy);
  }
  // searches for the scatterer in the library and initialises data. If scatterer no found, throws exception
  XScatterer(const olxstr& lbl, double energy) : mu(0), set_items(0)  {  
    cm_Element* src = XElementLib::FindBySymbol(lbl);
    if( src == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "unknown scatterer symbol");
    SetSource(*src, energy);
  }
  // copy constructor
  XScatterer(const XScatterer& sc) : 
    gaussians(sc.gaussians), 
    mu(sc.mu),
    wt(sc.wt),
    r(sc.r),
    Label(sc.Label),
    source(sc.source),
    fpfdp(sc.fpfdp),
    set_items(sc.set_items)  {}
  // initialises data from the provided library element
  void SetSource(const cm_Element& src, double energy)  {
    if( src.gaussians == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "given scatterer is only partially initialised");
    gaussians = *src.gaussians;
    Label = src.symbol;
    wt = src.GetMr();
    r = src.r_bonding;
    fpfdp = src.CalcFpFdp(energy) - src.z;
    source = &src;
    set_items = 0;
  }
  // updates a scetterer info
  void Merge(const XScatterer& sc)  {
    if( (sc.set_items & setGaussian) )
      SetGaussians(sc.gaussians);
    if( (sc.set_items & setDispersion) )
      SetFpFdp(sc.fpfdp);
    if( (sc.set_items & setMu) )
      SetMu(sc.mu);
    if( (sc.set_items & setR) )
      SetR(sc.r);
    if( (sc.set_items & setWt) )
      SetWeight(sc.wt);
  }
  XScatterer& operator = (const XScatterer& sc)  {
    gaussians = sc.gaussians;
    fpfdp = sc.fpfdp;
    mu = sc.mu;
    r = sc.r;
    wt = sc.wt;
    set_items = sc.set_items;
    return *this;
  }
  // sets custom fp and fdp
  void SetFpFdp(const compd& v) {
    if( v != fpfdp )  {
      fpfdp = v;
      set_items |= setDispersion;
    }
  }
  const compd& GetFpFdp() const {  return fpfdp;  }
  // sets custom bonding radius
  void SetR(double v)  {
    if( r != v )  {
      r = v;
      set_items |= setR;
    }
  }
  double GetR() const {  return r;  }
  // sets custom molecular weight
  void SetWeight(double v)  {
    if( v != wt )  {
      wt = v;
      set_items |= setWt;
    }
  }
  double GetWeight() const {  return wt;  }
  // sets custom adsorption coefficient
  void SetMu(double v)  {
    if( v != mu )  {
      mu = v;
      set_items |= setMu;
    }
  }
  double GetMu() const {  return mu;  }
  // sets custom gaussians
  void SetGaussians(const cm_Gaussians& g)  {
    if( gaussians != g )  {
      gaussians = g;
      set_items |= setGaussian;
    }
  }
  DefPropC(olxstr, Label)
  // return an INS file string representation
  olxstr ToInsString() const {
    if( set_items == setDispersion || set_items == (setDispersion|setMu) )  {
      olxstr rv("DISP ", 80);
      rv << Label << ' ' << fpfdp.GetRe() << ' ' << fpfdp.GetIm();
      if( (set_items & setMu) != 0 )
        rv << ' ' << mu;
      return rv;
    }
    else if( set_items == setAll ) {
      olxstr rv("SFAC ", 100);
      rv << Label << ' ' << gaussians.a1 << ' ' << gaussians.a2 << ' ' << gaussians.a3 << ' ' <<
        gaussians.a4 << ' ' << -gaussians.b1 << ' ' << -gaussians.b2 << ' ' << -gaussians.b3 << ' ' <<
        -gaussians.b4 << ' ' << gaussians.c <<  ' ' << fpfdp.GetRe() << ' ' << fpfdp.GetIm() << ' ' <<
        mu << ' ' << r << ' ' << wt;
      return rv;
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "failed to produce INS string");
  }
  inline double calc_sq(double sqv) const {
    return gaussians.calc_sq(sqv);
  }
  inline compd calc_sq_anomalous(double sqv) const {
    return compd(gaussians.calc_sq(sqv) + fpfdp.GetRe(), fpfdp.GetIm());
  }
  bool IsSFAC() const {  return (set_items == setAll);  }
  bool IsDISP() const {  return (set_items == setDispersion || set_items == (setDispersion|setMu));  }
  void ToDataItem(TDataItem& _di) const {
    TDataItem& di = _di.AddItem(Label, set_items);
    olxstr data;
    if( (set_items & setGaussian) != 0 )  {
      data << gaussians.a1 << ' ' << gaussians.a2 << ' ' << gaussians.a3 << ' ' << gaussians.a4 << ' '
           << gaussians.b1 << ' ' << gaussians.b2 << ' ' << gaussians.b3 << ' ' << gaussians.b4 << ' '
           << gaussians.c;
    }
    if( (set_items & setDispersion) != 0 )  {
      if( !data.IsEmpty() )  data << ' ';
      data << fpfdp.GetRe() << ' ' << fpfdp.GetIm();
    }
    if( (set_items & setMu) != 0 )  {
      if( !data.IsEmpty() )  data << ' ';
      data << mu;
    }
    if( (set_items & setR) != 0 )  {
      if( !data.IsEmpty() )  data << ' ';
      data << r;
    }
    if( (set_items & setWt) != 0 )  {
      if( !data.IsEmpty() )  data << ' ';
      data << wt;
    }
    di.AddField("data", data);
  }
  void FromDataItem(const TDataItem& di)  {
    Label = di.GetName();
    set_items = di.GetValue().ToInt();
    const TStrList toks(di.GetRequiredField("data"), ' ');
    size_t ind  =0;
    if( (set_items & setGaussian) != 0 )  {
      gaussians.a1 = toks[ind++].ToDouble();  gaussians.a2 = toks[ind++].ToDouble();
      gaussians.a4 = toks[ind++].ToDouble();  gaussians.a4 = toks[ind++].ToDouble();
      gaussians.b1 = toks[ind++].ToDouble();  gaussians.b2 = toks[ind++].ToDouble();
      gaussians.b3 = toks[ind++].ToDouble();  gaussians.b4 = toks[ind++].ToDouble();
      gaussians.c = toks[ind++].ToDouble();
    }
    if( (set_items & setDispersion) != 0 )  {
      fpfdp.Re() = toks[ind++].ToDouble();
      fpfdp.Im() = toks[ind++].ToDouble();
    }
    if( (set_items & setMu) != 0 )
      mu = toks[ind++].ToDouble();
    if( (set_items & setR) != 0 )
      r = toks[ind++].ToDouble();
    if( (set_items & setWt) != 0 )
      wt = toks[ind++].ToDouble();
    source = XElementLib::FindBySymbol(Label);
  }
#ifndef _NO_PYTHON
  PyObject* PyExport()  {
    PyObject* main = PyDict_New();
    PythonExt::SetDictItem(main, "gaussian",
      Py_BuildValue("(dddd)(dddd)d", gaussians.a1, gaussians.a2, gaussians.a3, gaussians.a4,
      gaussians.b1, gaussians.b2, gaussians.b3, gaussians.b4, gaussians.c));
    PythonExt::SetDictItem(main, "fpfdp", Py_BuildValue("(dd)", fpfdp.GetRe(), fpfdp.GetIm()));
    PythonExt::SetDictItem(main, "mu", Py_BuildValue("d", mu));
    PythonExt::SetDictItem(main, "r", Py_BuildValue("d", r));
    PythonExt::SetDictItem(main, "wt", Py_BuildValue("d", wt));
    return main;
  }
#endif
};


EndXlibNamespace()

#endif

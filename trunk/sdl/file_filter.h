#ifndef __olx_sdl_file_filter_H
#define __olx_sdl_file_filter_H
#include "estlist.h"

class FileFilter {
  TCSTypeList<olxstr, olxstr> extensions;
public:
  FileFilter() {}
  void Add(const olxstr &ext, const olxstr &desc=EmptyString())  {
    extensions.Add(ext, desc);
  }
  // use ';' separated list, like "res;ins"
  void AddAll(const olxstr &exts)  {
    TStrList toks(exts, ';');
    for( size_t i=0; i < toks.Count(); i++ )
      Add(olxstr("*.") << toks[i]);
  }
  static olxstr ProcessExt(const olxstr &ext)  {
#if defined(__WIN32__) || defined(__MAC__)
    return ext;
#else
    const size_t di = ext.LastIndexOf('.');
    if( di == InvalidIndex )  return ext;
    olxstr rv(EmptyString(), ext.Length()*2+3);
    rv << ext.SubStringTo(di+1);
    for( size_t k=di+1; k < ext.Length(); k++ )
      rv << '[' << olxstr::o_tolower(ext.CharAt(k)) << olxstr::o_toupper(ext.CharAt(k)) << ']';
    return rv;
#endif
  }
  olxstr GetString() const {
    olxstr_buf out;
    olxstr esep = ';', isep = '|';
    out << "All supported files|";
    for( size_t i=0; i < extensions.Count(); i++ )  {
      out << ProcessExt(extensions.GetKey(i));
      if( i+1 < extensions.Count() )
        out << esep;
    }
    for( size_t i=0; i < extensions.Count(); i++ )  {
      out << isep;
      if( extensions.GetObject(i).IsEmpty() )  {
        if( extensions.GetKey(i).StartsFrom("*.") )
          out << extensions.GetKey(i).SubStringFrom(2).ToUpperCase();
      }
      else
        out << extensions.GetObject(i);
      out << " files (" << extensions.GetKey(i) << ')' << isep <<
        ProcessExt(extensions.GetKey(i));
    }
    return out;
  }
};
#endif

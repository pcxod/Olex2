#ifndef __OLX__UTF8_FILE__
#define __OLX__UTF8_FILE__
#include "efile.h"
#include "eutf8.h"
#include "estrlist.h"

class TUtf8File : public TEFile  {
protected:
  TUtf8File(const olxstr& fn, const olxstr& attr, bool CheckHeader=true) : TEFile(fn, attr)  {
    if( Length() >= 3 )  {
      uint32_t header = 0;
      TEFile::Read(&header, 3);
      if( header != TUtf8::FileSignature )  {
        if( CheckHeader )
          throw TFunctionFailedException(__OlxSourceInfo, "invalid UTF8 file signature");
        else
          TEFile::SetPosition(0);
      }
    }
  }
  virtual inline size_t WritelnFor(const TIWString& str)  {  return Write(CNewLineSequence);  }
  virtual inline size_t WritelnFor(const wchar_t* Data)  {  return Write(CNewLineSequence);  }
  virtual inline size_t WritelnFor(const TICString& str)  {  return Write(CNewLineSequence);  }
  virtual inline size_t WritelnFor(const char* Data)  {  return Write(CNewLineSequence);  }
public:
  // pointer must be deleted
  static TUtf8File* Create(const olxstr& name, bool write_header=true)  {  
    TUtf8File* file = new TUtf8File(name, "wb+");  
    if( write_header )
      ((TEFile*)file)->Write(&TUtf8::FileSignature, 3);
    return file;
  }
  
  // pointer must be deleted, creates/opens for appending
  static TUtf8File* Open(const olxstr& name, bool CheckHeader)  {
    TUtf8File* file = new TUtf8File(name, "ab+", CheckHeader);  
    return file;
  }
  // creates a file and writes data to it, closes it
  static void Create(const olxstr& name, const TIString& data, bool write_header=true)  {  
    TUtf8File file(name, "wb+");  
    if( write_header )
      ((TEFile&)file).Write(&TUtf8::FileSignature, 3);
    file.Write(data);
  }

  static bool IsUtf8File(const olxstr& fn)  {
    try  {
      TEFile file(fn, "rb");
      if( file.Length() < 3 )  return false;
      uint32_t header;
      file.Read(&header, 3);
      return (header == TUtf8::FileSignature);
    }
    catch(...)  {
      return false;
    }
  }

  virtual inline size_t Write(const olxwstr &S)  {  return TEFile::Write(TUtf8::Encode(S));  }
  virtual inline size_t Write(const TIWString& S)  {  return TEFile::Write(TUtf8::Encode(S));  }
  virtual inline size_t Write(const wchar_t* bf)  {  return TEFile::Write(TUtf8::Encode(bf));  }
  virtual inline size_t Write(const wchar_t* bf, size_t size)  {  return TEFile::Write(TUtf8::Encode(bf, size));  }
  virtual inline size_t Write(const void* bf, size_t size)  {  return TEFile::Write(bf, size);  }

  template <class T>
  static void ReadLines(IInputStream& io, TTStrList<olxwstr,T>& list, bool CheckHeader=true)  {
    if( io.GetSize() >= 3 )  {
      uint32_t header = 0;
      io.Read(&header, 3);
      if( header != TUtf8::FileSignature )  {
        if( CheckHeader )
          throw TFunctionFailedException(__OlxSourceInfo, "invalid UTF8 stream");
        else
          io.SetPosition(0);
      }
    }
    const size_t fl = io.GetAvailableSizeT();
    olx_array_ptr<char> bf(new char [fl+1]);
    io.Read(bf, fl);
    list.Clear();
    list.Strtok(TUtf8::Decode(bf, fl), '\n', false);
    for( size_t i=0; i < list.Count(); i++ )
      if( list[i].EndsWith('\r') )  
        list[i].SetLength(list[i].Length()-1);
  }
  // returns one long string
  static olxwstr ReadAsString(IInputStream& io, bool CheckHeader=true)  {
    if( io.GetSize() >= 3 )  {
      uint32_t header = 0;
      io.Read(&header, 3);
      if( header != TUtf8::FileSignature )  {
        if( CheckHeader )
          throw TFunctionFailedException(__OlxSourceInfo, "invalid UTF8 stream");
        else
          io.SetPosition(0);
      }
    }
    const size_t fl = io.GetAvailableSizeT();
    olx_array_ptr<char> bf(new char [fl+1]);
    io.Read(bf, fl);
    olxwstr rv = TUtf8::Decode(bf, fl);
    return rv;
  }
  template <class T>
  static void ReadLines(const olxstr& fn, TTStrList<olxwstr,T>& list, bool CheckHeader=true)  {
    TUtf8File file(fn, "rb", CheckHeader);
    const size_t fl = file.GetAvailableSizeT();
    olx_array_ptr<char> bf(new char [fl+1]);
    file.Read(bf, fl);
    list.Clear();
    list.Strtok(TUtf8::Decode(bf, fl), '\n', false);
    for( size_t i=0; i < list.Count(); i++ )
      if( list[i].EndsWith('\r') )  
        list[i].SetLength(list[i].Length()-1);
  }
  // returns one long string
  static olxwstr ReadAsString(const olxstr& fn, bool CheckHeader=true)  {
    TUtf8File file(fn, "rb", CheckHeader);
    const size_t fl = file.GetAvailableSizeT();
    olx_array_ptr<char> bf(new char [fl+1]);
    file.Read(bf, fl);
    olxwstr rv = TUtf8::Decode(bf, fl);
    return rv;
  }
  template <class T>
  static void WriteLines(const olxstr& fn, const TTStrList<olxwstr,T>& list, bool WriteHeader=true)  {
    TUtf8File file(fn, "wb+");
    if( WriteHeader )
      ((TEFile&)file).Write(&TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      file.Writeln(list[i]);
  }
  template <class T>
  static void WriteLines(IDataOutputStream& out, const TTStrList<olxwstr,T>& list, bool WriteHeader=true)  {
    if( WriteHeader )
      out.Write(&TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      out.Writeln(list[i]);
  }
  template <class T>
  static void WriteLines(IDataOutputStream& out, const TTStrList<olxcstr,T>& list, bool WriteHeader=false)  {
    if( WriteHeader )
      out.Write(&TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      out.Writeln(list[i]);
  }
  template <class T>
  static void WriteLines(const olxstr& fn, const TTStrList<olxcstr,T>& list, bool WriteHeader=false)  {
    TUtf8File file(fn, "wb+");
    if( WriteHeader )
      ((TEFile&)file).Write(&TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      file.Writeln(list[i]);
  }

};

#endif
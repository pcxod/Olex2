/* original code by Marius Bancila at
http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451/
*/

#ifndef __OLX__UTF8__
#define __OLX__UTF8__

#include "ebase.h"
#include "edlist.h"

#define  UTF8_MASKBITS                0x3F
#define  UTF8_MASKBYTE                0x80
#define  UTF8_MASK2BYTES              0xC0
#define  UTF8_MASK3BYTES              0xE0
#define  UTF8_MASK4BYTES              0xF0
#define  UTF8_MASK5BYTES              0xF8
#define  UTF8_MASK6BYTES              0xFC

BeginEsdlNamespace()

class TUtf8  {
  olxcstr (*EncodeFunc)(const void* arr, size_t len);
  olxwstr (*DecodeFunc)(const char* arr, size_t len);
  static TUtf8 Instance;
protected:
  TUtf8()  {
    short sz = sizeof(wchar_t);
    if(  sz == 2 )  {
      EncodeFunc = &Encode2;
      DecodeFunc = &Decode2;
    }
    else if( sz == 4 )  {
      EncodeFunc = &Encode4;
      DecodeFunc = &Decode4;
    }
  }
public:
  static inline olxcstr Encode(const olxcstr& str)  {  return str;  }
  static inline olxcstr Encode(const olxwstr& str)  {
    return (*Instance.EncodeFunc)(str.raw_str(), str.Length());  
  }
  static inline olxcstr Encode(const TTIString<wchar_t>& str)  {
    return (*Instance.EncodeFunc)(str.raw_str(), str.Length());
  }
  static inline olxcstr Encode(const wchar_t* wstr)  {
    return (*Instance.EncodeFunc)(wstr, olxstr::o_strlen(wstr));
  }
  static inline olxcstr Encode(const wchar_t* wstr, size_t len)  {
    return (*Instance.EncodeFunc)(wstr, len);
  }
  
  static inline olxwstr Decode(const olxwstr& str)  {  return str;  }
  static inline olxwstr Decode(const olxcstr& str)  {
    return (*Instance.DecodeFunc)(str.raw_str(), str.Length());
  }
  static inline olxwstr Decode(const TTIString<char>& str)  {
    return (*Instance.DecodeFunc)(str.raw_str(), str.Length());
  }
  static inline olxwstr Decode(const char* str)  {
    return (*Instance.DecodeFunc)(str, olxstr::o_strlen(str));
  }
  static inline olxwstr Decode(const char* str, size_t len) {
    return (*Instance.DecodeFunc)(str, len);
  }

  const static uint32_t FileSignature;

protected:  // functions below are unsafe to use if wchar_t size is unknown!!
  static olxcstr Encode2(const void* vinput, size_t len)  {
    const uint16_t* input = (const uint16_t*)vinput;
    TDirectionalList<char> bf(len);
    for( size_t i=0; i < len; i++ )  {
      if( input[i] < 0x80 )  {  // 0xxxxxxx
        bf.Write((uint8_t)input[i]);
      }
      else if( input[i] < 0x800 )  {  // 110xxxxx 10xxxxxx
        bf.Write((uint8_t)(UTF8_MASK2BYTES | input[i] >> 6));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] & UTF8_MASKBITS));
      }
      else { // if( input[i] < 0x10000 )  {  // 1110xxxx 10xxxxxx 10xxxxxx. always true
        bf.Write((uint8_t)(UTF8_MASK3BYTES | input[i] >> 12));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 6 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] & UTF8_MASKBITS));
      }
    }
    olxcstr str(CEmptyString, bf.GetLength() );
    bf.ToString(str);
    return str;
  }
  static olxwstr Decode2(const char* input, size_t len)  {
    TDirectionalList<wchar_t> bf(len);
    for( size_t i=0; i < len; )  {
      uint16_t ch;
      if((input[i] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES)  {  // 1110xxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x0F) << 12) | (
          (input[i+1] & UTF8_MASKBITS) << 6)
          | (input[i+2] & UTF8_MASKBITS);
        i += 3;
      }
      else if( (input[i] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES )  {  // 110xxxxx 10xxxxxx
        ch = ((input[i] & 0x1F) << 6) | (input[i+1] & UTF8_MASKBITS);
        i += 2;
      }
      else  {  // if( input[i] < UTF8_MASKBYTE )  {  // 0xxxxxxx, always true
        ch = input[i];
        i += 1;
      }
      bf.Write(ch);
    }
    olxwstr str(WEmptyString, bf.GetLength() );
    bf.ToString(str);
    return str;
  }

  static olxcstr Encode4( const void* vinput, size_t len )  {
    const uint32_t* input = (const uint32_t*)vinput;
    TDirectionalList<char> bf(len);
    for( size_t i=0; i < len; i++ )  {
      if( input[i] < 0x80 )  {  // 0xxxxxxx
        bf.Write((uint8_t)input[i]);
      }
      else if( input[i] < 0x800 )  {  // 110xxxxx 10xxxxxx
        bf.Write((uint8_t)(UTF8_MASK2BYTES | input[i] >> 6));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] & UTF8_MASKBITS));
      }
      else if( input[i] < 0x10000 )  {  // 1110xxxx 10xxxxxx 10xxxxxx
        bf.Write((uint8_t)(UTF8_MASK3BYTES | input[i] >> 12));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 6 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] & UTF8_MASKBITS));
      }
      else if( input[i] < 0x200000 )  {  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        bf.Write((uint8_t)(UTF8_MASK4BYTES | input[i] >> 18));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 12 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 6 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] & UTF8_MASKBITS));
      }
      else if( input[i] < 0x4000000 )  {  // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        bf.Write((uint8_t)(UTF8_MASK5BYTES | input[i] >> 24));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 18 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 12 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 6 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] & UTF8_MASKBITS));
      }
      else if( input[i] < 0x8000000 )  {  // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        bf.Write((uint8_t)(UTF8_MASK6BYTES | input[i] >> 30));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 18 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 12 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] >> 6 & UTF8_MASKBITS));
        bf.Write((uint8_t)(UTF8_MASKBYTE | input[i] & UTF8_MASKBITS));
      }
    }
    olxcstr str(CEmptyString, bf.GetLength() );
    bf.ToString(str);
    return str;
  }

  static olxwstr Decode4(const char* input, size_t len)  {
    TDirectionalList<wchar_t> bf(len);
    for( size_t i=0; i < len; )  {
      uint32_t ch;
      if( (input[i] & UTF8_MASK6BYTES) == UTF8_MASK6BYTES )  {  // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x01) << 30) | ((input[i+1] & UTF8_MASKBITS) << 24)
          | ((input[i+2] & UTF8_MASKBITS) << 18) | ((input[i+3]
        & UTF8_MASKBITS) << 12)
          | ((input[i+4] & UTF8_MASKBITS) << 6) | (input[i+5] & UTF8_MASKBITS);
        i += 6;
      }
      else if( (input[i] & UTF8_MASK5BYTES) == UTF8_MASK5BYTES )  {  // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x03) << 24) | ((input[i+1]
        & UTF8_MASKBITS) << 18)
          | ((input[i+2] & UTF8_MASKBITS) << 12) | ((input[i+3]
        & UTF8_MASKBITS) << 6)
          | (input[i+4] & UTF8_MASKBITS);
        i += 5;
      }
      else if( (input[i] & UTF8_MASK4BYTES) == UTF8_MASK4BYTES )  {  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x07) << 18) | ((input[i+1]
        & UTF8_MASKBITS) << 12)
          | ((input[i+2] & UTF8_MASKBITS) << 6) | (input[i+3] & UTF8_MASKBITS);
        i += 4;
      }
      else if( (input[i] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES )  {  // 1110xxxx 10xxxxxx 10xxxxxx
        ch = ((input[i] & 0x0F) << 12) | ((input[i+1] & UTF8_MASKBITS) << 6)
          | (input[i+2] & UTF8_MASKBITS);
        i += 3;
      }
      else if( (input[i] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES )  {  // 110xxxxx 10xxxxxx
        ch = ((input[i] & 0x1F) << 6) | (input[i+1] & UTF8_MASKBITS);
        i += 2;
      }
      else  {  // if( input[i] < UTF8_MASKBYTE )  {  // 0xxxxxxx, always true
        ch = input[i];
        i += 1;
      }
      bf.Write(ch);
    }
    olxwstr str(WEmptyString, bf.GetLength() );
    bf.ToString(str);
    return str;
}
};
EndEsdlNamespace()

#endif


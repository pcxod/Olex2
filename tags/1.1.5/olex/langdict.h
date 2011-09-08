/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_langdict_H
#define __olx_langdict_H
#include "estlist.h"
#include "estrlist.h"
#include "wx/wx.h"

class TLangDict  {
  TSStrPObjList<olxstr,olxstr*, true> Records;
  olxstr CurrentLanguage, CurrentLanguageEncodingStr;
  int CurrentLanguageIndex;
protected:
  void Clear();
public:
  TLangDict();
  virtual ~TLangDict();

  const olxstr& Translate( const olxstr& Phrase) const;
  const olxstr& GetCurrentLanguageEncodingStr() const {  return CurrentLanguageEncodingStr;  }
  void SetCurrentLanguage(const olxstr& fileName, const olxstr& lang);
  const olxstr& GetCurrentLanguage()  const {  return CurrentLanguage;  }

};

#endif

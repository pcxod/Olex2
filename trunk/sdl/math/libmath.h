/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_libmath_H
#define __olx_sdl_libmath_H
#include "../library.h"

struct LibMath {
  static void Eval(const TStrObjList& Params, TMacroError& E);
  static void Abs(const TStrObjList& Params, TMacroError& E) {
    E.SetRetVal(olx_abs(Params[0].ToDouble()));
  }
  static TLibrary *ExportLibrary(const olxstr &name=EmptyString());
};

#endif

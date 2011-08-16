/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gdrawobject.h"
#include "glprimitive.h"
#include "gpcollection.h"
#include "styles.h"
UseGlNamespace()

AGDrawObject::AGDrawObject(TGlRenderer& parent, const olxstr& collectionName) :
  Parent(parent)
{
  Primitives = NULL;
  ParentGroup = NULL;
  Flags = 0;
  SetVisible(true);
  SetGroupable(false);
  SetSelectable(true);
  CollectionName = collectionName.IsEmpty() ? EsdlObjectName(*this) : collectionName;
}
//..............................................................................
void AGDrawObject::Compile()  {
  for( size_t i=0; i < Primitives->PrimitiveCount(); i++ )
    Primitives->GetPrimitive(i).Compile();
}
//..............................................................................
void AGDrawObject::UpdatePrimitives(int32_t Mask)  {
  olxstr& mstr = GetPrimitives().GetStyle().GetParam(GetPrimitiveMaskName(), "0");
  if( mstr.ToInt() == Mask )  return;
  mstr = Mask;
  GetPrimitives().ClearPrimitives();
  GetPrimitives().RemoveObject(*this);
  Create();
}
//..............................................................................
//..............................................................................
//..............................................................................
void AGDrawObject::LibVisible(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() ) 
    SetVisible(Params[0].ToBool());
  else
    E.SetRetVal<olxstr>(IsVisible());
}
//..............................................................................
void AGDrawObject::LibIsGrouped(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>(IsGrouped());
}
//..............................................................................
void AGDrawObject::LibIsSelected(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>(IsSelected());
}
//..............................................................................
void AGDrawObject::LibGetName(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<olxstr>(CollectionName);
}
//..............................................................................
void AGDrawObject::ExportLibrary(TLibrary& lib)  {
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibVisible,
  "Visible", fpNone|fpOne, "Changes/returns object visibility") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibIsGrouped,
  "IsGrouped", fpNone, "Returns true if the object is in a group") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibIsSelected,
  "IsSelected", fpNone, "Returns true if the object is selected") );
  lib.RegisterFunction<AGDrawObject>( new TFunction<AGDrawObject>(this,  &AGDrawObject::LibGetName,
  "GetName", fpNone, "Returns object collection name") );
}
//..............................................................................

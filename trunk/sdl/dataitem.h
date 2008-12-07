//---------------------------------------------------------------------------

#ifndef dataitemH
#define dataitemH
//---------------------------------------------------------------------------
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "estrlist.h"
#include "exception.h"

BeginEsdlNamespace()

class TEStrBuffer;

class TDataItem: public AReferencible  {
  TStrPObjList<olxstr,TDataItem*> Items;
  TStrStrList  Fields;
  olxstr Name, Value;
  olxstr ExtractBlockName(olxstr &Str);

  olxstr CodeString(const olxstr& Str)  const;
  olxstr DecodeString(const olxstr& Str) const;

  TDataItem* Parent;
  int Level, Index;
  void* Data;
protected:
  TDataItem& Root();
  olxstr GetFullName();
  TDataItem *DotItem(const olxstr& DotName, TStrList* Log);
  olxstr *DotField(const olxstr& DotName, olxstr &RefFieldName);
  TDataItem& AddItem(TDataItem& Item);
  olxstr* FieldPtr(const olxstr &Name);
  inline void SetParent(TDataItem* p)  {  Parent = p;  }
  TEStrBuffer& writeFullName(TEStrBuffer& bf);
public:
  TDataItem(TDataItem *Parent, const olxstr& Name, const olxstr& value=EmptyString);
  virtual ~TDataItem();
  void Clear();
  void Sort();  // sorts fields and items - improve the access by name performance
  void ResolveFields(TStrList* Log); // resolves referenced fields
  int LoadFromString( int start, olxstr &Data, TStrList* Log);
  void SaveToString(olxstr &Data);
  void SaveToStrBuffer(TEStrBuffer &Data);

  TDataItem& AddItem(const olxstr& Name, const olxstr& value=EmptyString);
  void AddContent(TDataItem& DI);
  // implementation of the include instruction object.item
  TDataItem& AddItem(const olxstr &Name, TDataItem *Reference);
  void DeleteItem(TDataItem *Item);

  TDataItem* GetAnyItem(const olxstr& Name) const;
  TDataItem* GetAnyItemCI(const olxstr& Name) const;
  // returns an item by name using recursive search within subitems as well
  // as in the current item
  TDataItem* FindItemCI(const olxstr& Name) const {  return Items.FindObjectCI(Name);  }
  TDataItem* FindItem(const olxstr& Name)   const {  return Items.FindObject(Name);  }
  TDataItem& FindRequiredItem(const olxstr& Name)   const {  
    int i = Items.IndexOf(Name);
    if( i == -1 )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Required item does not exist: ") << Name);
    return *Items.Object(i);  
  }

  TDataItem& GetItem(int index)                {  return *Items.Object(index); }
  const TDataItem& GetItem(int index)    const {  return *Items.Object(index); }
  void FindSimilarItems(const olxstr& StartsFrom, TPtrList<TDataItem>& List);
  inline int ItemCount() const                  {  return Items.Count(); }
  bool ItemExists(const olxstr &Name);
  int IndexOf(TDataItem *I) const               {  return Items.IndexOfObject(I); };

  TDataItem& AddField(const olxstr& Name, const olxstr& Data);
  TDataItem& AddCodedField(const olxstr& Name, const olxstr& Data);
  inline int FieldCount() const                 {  return Fields.Count(); }

  int FieldIndex(const olxstr& Name)    const {  return Fields.IndexOf(Name);  }
  int FieldIndexCI(const olxstr& Name)  const {  return Fields.CIIndexOf(Name);  }

  olxstr Field(int i)                   const {  return DecodeString(Fields.Object(i)); }
  // the filed will not be decoded
  const olxstr& RawField(int i)         const {  return Fields.Object(i); }
  const olxstr& FieldName(int i) const        {  return Fields.String(i); }
  // if field does not exist, a new one added
  void SetFieldValue(const olxstr& fieldName, const olxstr& newValue);
  void DeleteField(int index);
  void DeleteField(const olxstr& Name);

  const olxstr& GetFieldValue( const olxstr& Name, const olxstr& Default=EmptyString ) const {
    int i = Fields.IndexOf(Name);
    return (i==-1) ? Default : Fields.Object(i);
  }
  const olxstr& GetFieldValueCI( const olxstr& Name, const olxstr& Default=EmptyString ) const {
    int i = Fields.CIIndexOf(Name);
    return (i==-1) ? Default : Fields.Object(i);
  }
  const olxstr& GetRequiredField( const olxstr& Name) const  {
    int i = Fields.IndexOf(Name);
    if( i == -1 )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Required attribute is missing: ") << Name);
    return Fields.Object(i);
  }

  bool FieldExists(const olxstr &Name);

  TDataItem* GetParent() const           {  return Parent; }
  inline int GetLevel() const            {  return Level; }
  inline int GetIndex() const            {  return Index; }
  DefPropC(olxstr, Name)
  olxstr GetValue()                const {  return DecodeString(Value); }
  void SetValue(const olxstr &V)         {  Value = CodeString(V); }
  // for use with whatsoever, initialised twith NULL
  DefPropP(void*, Data)

  void Validate(TStrList& Log);

  class TNonexistingDataItemException: public TBasicException  {
  public:
    TNonexistingDataItemException(const olxstr& location, const olxstr &Msg):
      TBasicException(location, Msg)  {  ;  }
    virtual IEObject* Replicate()  const  {  return new TNonexistingDataItemException(*this);  }
  };
};

EndEsdlNamespace()
#endif




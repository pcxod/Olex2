// (c) O Dolomanov, 2004-2010
#ifndef __olx_sdl_collectionItem_H
#define __olx_sdl_collectionItem_H
// implements data for collection item
class ACollectionItem : public IEObject  {
  index_t CollectionItemTag;
public:
  ACollectionItem()  {  CollectionItemTag = -1;  }
  virtual ~ACollectionItem()  {  ;  }
  index_t GetTag() const {  return CollectionItemTag;  }
  index_t GetTag(const ACollectionItem& i) const {  i.GetTag();  }
  index_t GetTag(const ACollectionItem* i) const {  i->GetTag();  }
  void SetTag(index_t v) { CollectionItemTag = v;  }
  index_t IncTag()  {  return ++CollectionItemTag;  }
  index_t DecTag()  {  return --CollectionItemTag;  }
  // for extended functionality of containers
  template <class Accessor=DirectAccessor> struct TagAnalyser  {
    const index_t ref_tag;
    TagAnalyser(index_t _ref_tag) : ref_tag(_ref_tag)  {}
    template <class Item> inline bool OnItem(const Item& o, size_t) const {
      return Accessor::Access(o).GetTag() == ref_tag;
    }
  };
  template <class Accessor=DirectAccessor> struct IndexTagAnalyser  {
    template <class Item> static inline bool OnItem(const Item& o, size_t i)  {
      return Accessor::Access(o).GetTag() != i;
    }
  };
  template <class Accessor=DirectAccessor> struct TagSetter  {
    const index_t ref_tag;
    TagSetter(index_t _ref_tag) : ref_tag(_ref_tag)  {}
    template <class Item> inline void OnItem(Item& o, size_t) const {
      Accessor::Access(o).SetTag(ref_tag);
    }
  };
  template <class Accessor=DirectAccessor> struct IndexTagSetter  {
    template <class Item> static inline void OnItem(Item& o, size_t i)  {
      Accessor::Access(o).SetTag(i);
    }
  };
  template <class Accessor=DirectAccessor> struct TagAccessor  {
    template <class Item> static inline index_t Access(const Item& o)  {
      return GetTag(Accessor::Access(o));
    }
  };

  // common algorithms
  // creates a list of unique items
  template <class Accessor=DirectAccessor>
  struct Unique  {
    template <class List> Unique(List& list)  {  Do(list);  }
    template <class List> static List& Do(List& list)  {
      list.ForEach(IndexTagSetter<Accessor>());
      list.Pack(IndexTagAnalyser<Accessor>());
      return list;
    }
  };
  // exludes a set of items from a list of items
  template <class AccessorA=DirectAccessor, class AccessorB=DirectAccessor>
  struct Exclude  {
    template <class ListA, class ListB> Exclude(ListA& from, const ListB& set)  {
      Do(from, set);
    }
    template <class ListA, class ListB> static
    ListA& Do(ListA& from, const ListB& set)
    {
      from.ForEach(TagSetter<AccessorA>(0));
      set.ForEach(TagSetter<AccessorB>(1));
      from.Pack(TagAnalyser<AccessorA>(1));
      return from;
    }
  };
};

#endif

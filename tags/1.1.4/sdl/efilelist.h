#ifndef efilelistH
#define efilelistH
#include "ebase.h"
#include "typelist.h"
#include "etime.h"

BeginEsdlNamespace()
  class TFileListItem  {
  protected:
    unsigned short Attributes;
    uint64_t CreationTime, ModificationTime, LastAccessTime;
    uint64_t Size;
    olxstr Name;
  public:
    TFileListItem()  { 
      Size = 0;  
      CreationTime = ModificationTime = LastAccessTime = 0;   
      Attributes = 0;
    }
    TFileListItem( const TFileListItem& item)  {
      Name = item.Name;
      Size = item.Size;
      Attributes = item.Attributes;
      CreationTime = item.CreationTime;
      ModificationTime = item.ModificationTime;
      LastAccessTime = item.LastAccessTime;
    }
    DefPropP(uint64_t, CreationTime)
    DefPropP(uint64_t, ModificationTime)
    DefPropP(uint64_t, LastAccessTime)
    DefPropP(uint64_t, Size)
    DefPropP(unsigned short, Attributes)
    DefPropC(olxstr, Name)

    template <short field> class TFileListItemSorter  {
      static olxstr ExtractFileExt(const olxstr& fn)  {
        size_t ind = fn.LastIndexOf('.');
        return (ind != InvalidIndex) ?  fn.SubStringFrom(ind+1) : EmptyString;
      }
    public:
      static int Compare( const TFileListItem* i1, const TFileListItem* i2 )  {
        if( field == 0 )  return i1->GetName().Compare(i2->GetName());
        if( field == 1 )  return ExtractFileExt(i1->GetName()).Compare(ExtractFileExt(i2->GetName()));
        if( field == 2 )  return TPrimitiveComparator::Compare<uint64_t>(i1->GetCreationTime(), i2->GetCreationTime());
        if( field == 3 )  return TPrimitiveComparator::Compare<uint64_t>(i1->GetModificationTime(), i2->GetModificationTime());
        if( field == 4 )  return TPrimitiveComparator::Compare<uint64_t>(i1->GetLastAccessTime(), i2->GetLastAccessTime());
        if( field == 5 )  return TPrimitiveComparator::Compare<uint64_t>(i1->GetSize(), i2->GetSize());
        return 0;
      }
    };

    static void SortListByName(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort(list, TFileListItemSorter<0>());
    }
    static void SortListByExt(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort(list, TFileListItemSorter<1>());
    }
    static void SortListByAge(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort(list, TFileListItemSorter<2>());
    }
    static void SortListByModificationTime(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort(list, TFileListItemSorter<3>());
    }
    static void SortListByLastAccessTime(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort(list, TFileListItemSorter<4>());
    }
    static void SortListBySize(TTypeList<TFileListItem>& list)  {
      TTypeList<TFileListItem>::QuickSorter.Sort(list, TFileListItemSorter<5>());
    }
  };

  typedef TTypeList<TFileListItem> TFileList;

EndEsdlNamespace()

#endif

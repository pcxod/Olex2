/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_atom_sort_H
#define __olx_atom_sort_H
#include "lattice.h"
#include "edict.h"

const short 
  atom_sort_None      = 0x0000,
  atom_sort_Mw        = 0x0001,
  atom_sort_Label     = 0x0002,  // clever sort C1a1  
  atom_sort_Label1    = 0x0004, // string comparison sort
  atom_sort_MoietySize = 0x0008,  // fgrament size
  atom_sort_KeepH     = 0x0010,  // keeps H atoms after the pivoting one
  moiety_sort_Create   = 0x0020,
  moiety_sort_Size     = 0x0040,
  moiety_sort_Weight   = 0x0080,
  moiety_sort_Heaviest = 0x0100;  // sorts moieties by heaviest element of the moiety

class AtomSorter {
public:
  struct Sorter {
    int (*func)(const TCAtom &, const TCAtom &);
    olxdict<olxstr, int, olxstrComparator<true> > label_exc;
    olxdict<const cm_Element *, int, TPointerComparator> type_exc;

    Sorter(int (*f)(const TCAtom &, const TCAtom &)) : func(f) {}
    void AddExceptions(const TStrList &e) {
      for (size_t i=0; i < e.Count(); i++) {
        if (e[i].StartsFrom('$')) {
          cm_Element *elm = XElementLib::FindBySymbol(e[i].SubStringFrom(1));
          if (elm == NULL) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              olxstr("element ").quote() << e[i].SubStringFrom(1));
          }
          type_exc.Add(elm, type_exc.Count());
        }
        else {
          label_exc.Add(e[i], label_exc.Count());
        }
      }
    }
    int sort(const TCAtom &a, const TCAtom &b) const {
      if (label_exc.HasKey(a.GetLabel()) && label_exc.HasKey(b.GetLabel())) {
        return olx_cmp(label_exc[a.GetLabel()], label_exc[b.GetLabel()]);
      }
      else if (type_exc.HasKey(&a.GetType()) && type_exc.HasKey(&b.GetType())) {
        return olx_cmp(type_exc[&a.GetType()], type_exc[&b.GetType()]);
      }
      return (*func)(a, b);
    }
  };

  static int atom_cmp_Part(const TCAtom &a1, const TCAtom &a2) {
    if( a2.GetPart() < 0 && a1.GetPart() >= 0 )
      return -1;
    if( a2.GetPart() >= 0 && a1.GetPart() < 0 )
      return 1;
    return a1.GetPart() - a2.GetPart();  // smallest goes first
  }
  static int atom_cmp_Z(const TCAtom &a1, const TCAtom &a2) {
    return olx_cmp(a2.GetType().z, a1.GetType().z);
  }
  static int atom_cmp_Mw(const TCAtom &a1, const TCAtom &a2) {
    return olx_cmp(a2.GetType().GetMr(), a1.GetType().GetMr());
  }
  static int atom_cmp_Label(const TCAtom &a1, const TCAtom &a2)  {
    return TCAtom::CompareAtomLabels(a1.GetLabel(), a2.GetLabel());
  }
  static int atom_cmp_Suffix(const TCAtom &a1, const TCAtom &a2)  {
    olxstr sa, sb;
    size_t i=a1.GetType().symbol.Length();
    while (++i < a1.GetLabel().Length() && olxstr::o_isdigit(a1.GetLabel()[i]))
      ;
    if (i < a1.GetLabel().Length())
      sa = a1.GetLabel().SubStringFrom(i);
    i = a2.GetType().symbol.Length();
    while (++i < a2.GetLabel().Length() && olxstr::o_isdigit(a2.GetLabel()[i]))
      ;
    if (i < a2.GetLabel().Length())
      sb = a2.GetLabel().SubStringFrom(i);
    return olxstrComparator<false>::Compare(sa, sb);
  }
  static int atom_cmp_Number(const TCAtom &a1, const TCAtom &a2)  {
    olxstr sa, sb;
    for( size_t i=a1.GetType().symbol.Length(); i < a1.GetLabel().Length(); i++ )  {
      if( olxstr::o_isdigit(a1.GetLabel().CharAt(i)) )
        sa << a1.GetLabel().CharAt(i);
      else
        break;
    }
    for( size_t i=a2.GetType().symbol.Length(); i < a2.GetLabel().Length(); i++ )  {
      if( olxstr::o_isdigit(a2.GetLabel().CharAt(i)) )
        sb << a2.GetLabel().CharAt(i);
      else
        break;
    }
    if( sa.IsEmpty() )
      return (sb.IsEmpty() ? 0 : -1);
    return (sb.IsEmpty() ? 1 : olx_cmp(sa.ToInt(), sb.ToInt()));
  }
  static int atom_cmp_Id(const TCAtom &a1, const TCAtom &a2)  {
    return olx_cmp(a1.GetId(), a2.GetId());
  }
  static int atom_cmp_Label1(const TCAtom &a1, const TCAtom &a2) {
    return a1.GetLabel().Comparei(a2.GetLabel());
  }
  static int atom_cmp_MoietySize(const TCAtom &a1, const TCAtom &a2) {
    return a1.GetFragmentId() - a2.GetFragmentId();
  }

  struct CombiSort {
    TTypeList<Sorter> sequence;
    int atom_cmp(const TCAtom &a1, const TCAtom &a2) const {
      for( size_t i=0; i < sequence.Count(); i++ )  {
        int res = sequence[i].sort(a1, a2);
        if (res != 0) return res;
      }
      return 0;
    }
  };

  static void KeepH(TCAtomPList& l1, const TLattice& latt,
    int (*sort_func)(const TCAtom &, const TCAtom &))
  {
    typedef AnAssociation2<TCAtom*,TCAtomPList> tree_node;
    TTypeList<tree_node> atom_tree;
    for( size_t i=0; i < latt.GetAsymmUnit().AtomCount(); i++ ) // tag all atoms with -1
      latt.GetAsymmUnit().GetAtom(i).SetTag(-1);
    for( size_t i=0; i < l1.Count(); i++ )  // tag atoms in the list with 0
      l1[i]->SetTag(0);
    size_t atom_count = 0;
    for( size_t i=0; i < l1.Count(); i++ )  {
      if( l1[i]->GetType() == iHydrogenZ )
        continue;
      TSAtom* sa = NULL;
      const ASObjectProvider& objects = latt.GetObjects();
      for( size_t j=0; j < objects.atoms.Count(); j++ )  {
        TSAtom& sa1 = objects.atoms[j];
        if( sa1.CAtom().GetId() == l1[i]->GetId() )  {
          sa = &sa1;
          break;
        }
      }
      TCAtomPList& ca_list = atom_tree.Add( new tree_node(l1[i])).B();
      if( l1[i]->GetType() == iQPeakZ || l1[i]->IsDeleted() )  {
        atom_count++;
        continue;
      }
      if( sa == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "aunit and lattice mismatch");
      for( size_t j=0; j < sa->NodeCount(); j++ )  {
        // check if the atom in the list
        if( sa->Node(j).CAtom().GetTag() == 0 && (sa->Node(j).GetType() == iHydrogenZ) )
          ca_list.Add( &sa->Node(j).CAtom() )->SetTag(1);
      }
      if( ca_list.Count() > 1 && sort_func != NULL )
        QuickSorter::SortSF(ca_list, sort_func);
      atom_count += (ca_list.Count() + 1);
    }
    if( atom_count != l1.Count() ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "atom list does not match the lattice, could not keep H atoms next to "
        "pivot atom");
    }
    atom_count = 0;
    for( size_t i=0; i < atom_tree.Count(); i++ )  {
      l1[atom_count++] = atom_tree[i].A();
      for( size_t j=0; j < atom_tree[i].GetB().Count(); j++ )
        l1[atom_count++] = atom_tree[i].GetB()[j];
    }
  }
  static void SyncLists(const TCAtomPList& ref, TCAtomPList& list)  {
    if( ref.Count() != list.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "lists mismatch");
    TCAtomPList list_copy(list);
    for( size_t i=0; i < ref.Count(); i++ )  {
      bool found = false;
      for( size_t j=0; j < ref.Count(); j++ )  {
        if( ref[i]->GetId() == list_copy[j]->GetId() )  {
          list[i] = list_copy[j];
          found = true;
          break;
        }
      }
      if( !found )
        throw TInvalidArgumentException(__OlxSourceInfo, "lists content mismatch");
    }
  }
  static void SortByName(TCAtomPList& list, const TStrList& atom_names)  {
    if( atom_names.Count() < 2 )
      return;
    size_t fi = InvalidIndex;
    TCAtomPList sorted;
    for( size_t i=0; i < list.Count(); i++ )  {
      if( list[i]->GetLabel().Equalsi(atom_names[0]) )  {
        fi = i;
        sorted.Add(list[i]);
        list[i] = NULL;
        break;
      }
    }
    if( fi == InvalidIndex )
      throw TInvalidArgumentException(__OlxSourceInfo, "atom names");
    for( size_t i=1; i < atom_names.Count(); i++ )  {
      for( size_t j=0; j < list.Count(); j++ )  {
        if( list[j] == NULL )  continue;
        if( list[j]->GetLabel().Equalsi(atom_names[i]) )  {
          sorted.Add(list[j]);
          list[j] = NULL;
          break;
        }
      }
    }
    size_t ins_pos = 0;
    for( size_t i=0; i < fi; i++ )  {
      if( list[i] != NULL  )  
        ins_pos++;
    }
    list.Pack();
    list.Insert(ins_pos, sorted);
  }
  static void Sort(TCAtomPList& list,
    int (*sort_func)(const TCAtom&, const TCAtom&))
  {
    QuickSorter::SortSF(list, sort_func);
  }
  static void Sort(TCAtomPList& list, AtomSorter::CombiSort& cs)  {
    QuickSorter::SortMF(list, cs, &AtomSorter::CombiSort::atom_cmp);
  }
};

class MoietySorter {
  static int moiety_cmp_Mr(const AnAssociation3<size_t,double,TCAtomPList> &m1,
    const AnAssociation3<size_t,double,TCAtomPList> &m2)
  {
    return olx_cmp(m2.GetB(), m1.GetB());
  }
  static int moiety_cmp_size(const AnAssociation2<size_t,TCAtomPList> &m1,
    const AnAssociation2<size_t,TCAtomPList> &m2)
  {
    return olx_cmp(m2.GetB().Count(), m1.GetB().Count());
  }
public:
  static void SortByMoietyAtom(TCAtomPList& list, const TStrList& atom_names)  {
    if( atom_names.Count() < 2 )
      return;
    typedef AnAssociation2<TCAtom*, TCAtomPList> moiety;
    TTypeList<moiety> moieties;
    for( size_t i=0; i < list.Count(); i++ )
      list[i]->SetTag(-1);
    for( size_t i=0; i < atom_names.Count(); i++ )  {
      for( size_t j=0; j < list.Count(); j++ )  {
        if( list[j]->GetLabel().Equalsi(atom_names[i]) )  {
          if( list[j]->GetTag() == -1 )  {  // avoid duplicated moieties
            list[j]->SetTag(moieties.Count());
            moieties.Add(new moiety(list[j]));
          }
        }
      }
    }
    if( moieties.IsEmpty() )
      return;
    for( size_t i=0; i < moieties.Count(); i++ )  {
      const size_t moiety_id = moieties[i].GetA()->GetFragmentId();
      for( size_t j=0; j < list.Count(); j++ )  {
        if( moiety_id == list[j]->GetFragmentId() && list[j]->GetTag() == -1 )  {
          list[j]->SetTag(i);
          moieties[i].B().Add(list[j]);
        }
      }
    }
    for( size_t i=0; i < list.Count(); i++ )  {
      if( list[i]->GetTag() != -1 )
        list[i] = NULL;
    }
    list.Pack();
    for( size_t i=0; i < moieties.Count(); i++ )  {
      moiety& mt = moieties[i];
      list.Add( mt.A() );
      for( size_t j=0; j < mt.GetB().Count(); j++ )
        list.Add( mt.GetB()[j] );
    }
  }
  // does not do the sorting - just organises atoms into moieties
  static void CreateMoieties(TCAtomPList& list)  {
    // dictionary will not do here - the moiety order will be destroyed...
    typedef AnAssociation2<size_t, TCAtomPList> moiety;
    TTypeList<moiety> moieties;
    for( size_t i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( size_t j=0; j < moieties.Count(); j++ )  {
        if( moieties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moieties[j].B();
          break;
        }
      }
      if( ca_list == NULL )
        ca_list = &moieties.Add( new moiety(list[i]->GetFragmentId()) ).B();
      ca_list->Add(list[i]);
    }
    if( moieties.Count() < 2 )  return;
    int atom_cnt = 0;
    for( size_t i=0; i < moieties.Count(); i++ )  {
      for( size_t j=0; j < moieties[i].GetB().Count(); j++ )
        list[atom_cnt++] = moieties[i].GetB()[j];
    }
  }
  static void SortByHeaviestElement(TCAtomPList& list)  {
    typedef AnAssociation3<size_t, double, TCAtomPList> moiety;
    TTypeList<moiety> moieties;
    for( size_t i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( size_t j=0; j < moieties.Count(); j++ )  {
        if( moieties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moieties[j].C();
          if( list[i]->GetType().GetMr() > moieties[j].GetB() )
            moieties[j].B() = list[i]->GetType().GetMr();
          break;
        }
      }
      if( ca_list == NULL ) {
        ca_list = &moieties.Add(
          new moiety(list[i]->GetFragmentId(), list[i]->GetType().GetMr())).C();
      }
      ca_list->Add(list[i]);
    }
    if( moieties.Count() < 2 )  return;
    QuickSorter::SortSF(moieties, moiety_cmp_Mr);
    int atom_cnt = 0;
    for( size_t i=0; i < moieties.Count(); i++ )  {
      for( size_t j=0; j < moieties[i].GetC().Count(); j++ )
        list[atom_cnt++] = moieties[i].GetC()[j];
    }
  }
  static void SortByWeight(TCAtomPList& list)  {
    typedef AnAssociation3<size_t, double, TCAtomPList> moiety;
    TTypeList<moiety> moieties;
    for( size_t i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( size_t j=0; j < moieties.Count(); j++ )  {
        if( moieties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moieties[j].C();
          moieties[j].B() += list[i]->GetType().GetMr();
          break;
        }
      }
      if( ca_list == NULL ) {
        ca_list = &moieties.Add(
          new moiety(list[i]->GetFragmentId(), list[i]->GetType().GetMr())).C();
      }
      ca_list->Add(list[i]);
    }
    if( moieties.Count() < 2 )  return;
    QuickSorter::SortSF(moieties, moiety_cmp_Mr);
    int atom_cnt = 0;
    for( size_t i=0; i < moieties.Count(); i++ )  {
      for( size_t j=0; j < moieties[i].GetC().Count(); j++ )
        list[atom_cnt++] = moieties[i].GetC()[j];
    }
  }
  static void SortBySize(TCAtomPList& list)  {
    typedef AnAssociation2<size_t, TCAtomPList> moiety;
    TTypeList<moiety> moieties;
    for( size_t i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( size_t j=0; j < moieties.Count(); j++ )  {
        if( moieties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moieties[j].B();
          break;
        }
      }
      if( ca_list == NULL )
        ca_list = &moieties.Add(new moiety(list[i]->GetFragmentId()) ).B();
      ca_list->Add(list[i]);
    }
    if( moieties.Count() < 2 )  return;
    QuickSorter::SortSF(moieties, moiety_cmp_size);
    int atom_cnt = 0;
    for( size_t i=0; i < moieties.Count(); i++ )  {
      for( size_t j=0; j < moieties[i].GetB().Count(); j++ )
        list[atom_cnt++] = moieties[i].GetB()[j];
    }
  }
};



#endif


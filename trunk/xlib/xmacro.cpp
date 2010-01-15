#ifdef __BORLANC__
  #pragma hdrstop
#endif

#include "xmacro.h"
#include "xapp.h"

#include "p4p.h"
#include "mol.h"
#include "crs.h"
#include "ins.h"
#include "cif.h"
#include "hkl.h"

#include "unitcell.h"
#include "symmtest.h"
#include "integration.h"
#include "utf8file.h"
#include "datafile.h"
#include "dataitem.h"
#include "fsext.h"
#include "ecast.h"
#include "xlcongen.h"
#include "bitarray.h"
#include "olxvar.h"
#include "strmask.h"
#include "sgset.h"
#include "sfutil.h"
#include "infotab.h"
#include "idistribution.h"
#include "ipattern.h"
#include "chnexp.h"
#include "maputil.h"
#include "vcov.h"

#define xlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro( new TStaticMacro(&XLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))
#define xlib_InitFunc(funcName, argc, desc) \
  lib.RegisterStaticFunction( new TStaticFunction(&XLibMacros::fun##funcName, #funcName, argc, desc))

const olxstr XLibMacros::NoneString("none");
const olxstr XLibMacros::NAString("n/a");
olxstr XLibMacros::CurrentDir;
TActionQList XLibMacros::Actions;
TActionQueue& XLibMacros::OnDelIns = XLibMacros::Actions.New("OnDelIns");
TActionQueue& XLibMacros::OnAddIns = XLibMacros::Actions.New("OnAddIns");

void XLibMacros::Export(TLibrary& lib)  {
  xlib_InitMacro(Run, EmptyString, fpAny^fpNone, "Runs provided macros (combined by '>>')");
  xlib_InitMacro(HklStat, "l-list the reflections&;m-merge reflection in current space group", fpAny|psFileLoaded, 
    "If no arguments provided, prints the statistics on the reflections as well as the ones used in the refinement.\
 If an expressions (condition) is given in the following form: x[ahbkcl], meaning that x=ah+bk+cl;\
 the subsequent expressions are combined using logical 'and' operator. For instance 0[2l] expression means: to find all\
 reflections where 2l = 0. The function operates on all P1 merged reflections after\
 filtering by SHEL and OMIT, -m option merges the reflections in current space group");
  xlib_InitMacro(BrushHkl, "f-consider Friedel law", fpAny, "for high redundancy\
 data sets, removes equivalents with high sigma");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(SG, "a", fpNone|fpOne, "suggest space group");
  xlib_InitMacro(SGE, EmptyString, fpNone|fpOne|psFileLoaded, "Extended spacegroup determination. Internal use" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(GraphSR, "b-number of bins", fpNone|fpOne|psFileLoaded,
"Prints a scale vs resolution graph for current file (fcf file must exist in current folder)");
  xlib_InitMacro(GraphPD, "r-resolution in degrees [0.5]&;fcf-take structure factors from the FCF file, otherwise calculate from current model\
&;s-use simple scale when calculating structure factors from the mode, otherwise regression scaling will be used", fpNone|psFileLoaded,
"Prints a intensity vs. 2 theta graph");
  xlib_InitMacro(Wilson, "b-number of bins&;p-uses linear bins for picture, otherwise uses spherical bins", 
    fpNone|fpOne|psFileLoaded, "Prints Wilson plot data");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(TestSymm, "e-tolerance limit", fpNone|psFileLoaded, "Tests current \
  structure for missing symmetry");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(VATA, EmptyString, fpAny|psFileLoaded,
"Compares current model with the cif file and write the report to provided file (appending)" );
  xlib_InitMacro(Clean, "npd-does not change atom types of NPD atoms\
&;f-does not run 'fuse' after the completion\
&;aq-disables analysis of the Q-peaks based on thresholds\
&;at-disables lonely atom types assignment to O and Cl", fpNone,
"Tidies up current model" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AtomInfo, EmptyString, fpAny|psFileLoaded,
"Searches information for given atoms in the database" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Compaq, "a-assembles broken fragments&;c-similar as with no options, but considers atom-to-atom distances\
&;q-moves Q-peaks to the atoms, atoms are not affected", 
    fpNone|psFileLoaded,
"Moves all atoms or fragments of the asymmetric unit as close to each other as possible. If no options provided, all fragments\
 are assembled around the largest one." );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Envi, "q-adds Q-peaks to the list&;h-adds hydrogen atoms to the list&;cs-leaves selection unchanged",
    fpNone|fpOne|fpTwo,
"This macro prints environment of any particular atom. Default search radius is 2.7A."  );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AddSE, EmptyString, (fpAny^fpNone)|psFileLoaded,
"Tries to add a new symmetry element to current space group to form a new one. [-1] is for center of symmetry" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Fuse, "f-removes symmetrical equivalents", fpNone|fpOne|psFileLoaded,
"Re-initialises the connectivity list. If a number is provided, atoms of the same type connected by bonds shorter\
 than the provided number are merged into one atom with center at the centroid formed by all removed atoms" );
  xlib_InitMacro(Flush, EmptyString, fpNone|fpOne, "Flushes log streams" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(EXYZ, "eadp-sets the equivalent anisotropic parameter constraints for the shared sites\
&;lo-links occupancies of all elements sharing the site", (fpAny^fpNone)|psCheckFileTypeIns,
"Adds a new element to the give site. Takes the site as selected atom and element types\
 as any subsequent argument" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(EADP, EmptyString, fpAny|psCheckFileTypeIns,
"Forces EADP/Uiso of provided atoms to be constrained the same" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Cif2Doc, "n-output file name", fpNone|fpOne|psFileLoaded, "converts cif to a document" );
  xlib_InitMacro(Cif2Tab, "n-output file name", fpAny|psFileLoaded, "creates a table from a cif" );
  xlib_InitMacro(CifMerge, EmptyString, (fpAny^fpNone)|psFileLoaded,
  "Merges loaded or provided as first argument cif with other cif(s)" );
  xlib_InitMacro(CifExtract, EmptyString, fpTwo|psFileLoaded, "extract a list of items from one cif to another" );
  xlib_InitMacro(CifCreate, EmptyString, fpNone|psFileLoaded, "Creates cif from current file, variance-covariance matrix should be available" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(VoidE, EmptyString, fpNone|psFileLoaded, "calculates number of electrons in the voids area" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(ChangeSG, EmptyString, fpOne|fpFour|psFileLoaded, "[shift] SG Changes space group of current structure" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Htab, "t-adds extra elements (comma separated -t=Br,I) to the donor list. Defaults are [N,O,F,Cl,S]", fpNone|fpOne|fpTwo|psCheckFileTypeIns, 
    "Adds HTAB instructions to the ins file, maximum bond length [2.9] and minimal angle [150] might be provided" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(HAdd, EmptyString, fpAny|psCheckFileTypeIns, "Adds hydrogen atoms to all or provided atoms, however\
 the ring atoms are treated separately and added all the time" );
  xlib_InitMacro(HImp, EmptyString, (fpAny^fpNone)|psFileLoaded, "Increases, decreases length of H-bonds.\
 Arguments: value [H atoms]. Value might be +/- to specify to increase/decrease current value" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(FixUnit, EmptyString, fpNone|fpOne|psFileLoaded, "Sets SFAc and UNIT to current content of the asymmetric unit.\
 Takes Z', with default value of 1.");
  xlib_InitMacro(GenDisp, EmptyString, fpNone|fpOne|psFileLoaded, "Generates anisotropic dispertion parameters for current radiation wavelength");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AddIns,EmptyString, (fpAny^fpNone)|psCheckFileTypeIns, "Adds an instruction to the INS file" );
  xlib_InitMacro(DelIns, EmptyString, fpOne|psCheckFileTypeIns, "A number or the name (will remove all accurances) can be provided" );
  xlib_InitMacro(LstIns, EmptyString, fpNone|psCheckFileTypeIns, "Lists all instructions of currently loaded Ins file" );
  xlib_InitMacro(FixHL, EmptyString, fpNone|psFileLoaded, "Fixes hydrogen atom labels" );
  xlib_InitMacro(Fix, EmptyString, (fpAny^fpNone)|psCheckFileTypeIns, "Fixes specified parameters of atoms: XYZ, Uiso, Occu" );
  xlib_InitMacro(Free, EmptyString, (fpAny^fpNone)|psCheckFileTypeIns, "Frees specified parameters of atoms: XYZ, Uiso, Occu" );
  xlib_InitMacro(Isot,EmptyString , fpAny|psFileLoaded,
"makes provided atoms isotropic, if no arguments provided, current selection or all atoms become isotropic");
  xlib_InitMacro(Anis,"h-adds hydrogen atoms" , (fpAny) | psFileLoaded, 
"makes provided atoms anisotropic if no arguments provided current selection or all atoms are considered" );
xlib_InitMacro(File, "s-sort the main residue of the asymmetric unit", fpNone|fpOne|psFileLoaded, 
    "Saves current model to a file. By default an ins file is saved and loaded" );
  xlib_InitMacro(LS, EmptyString, fpOne|fpTwo|psCheckFileTypeIns, "Sets refinement method and/or the number of iterations.");
  xlib_InitMacro(Plan, EmptyString, fpOne|psCheckFileTypeIns, "Sets the number of Fourier peaks to be found from the difference map");
  xlib_InitMacro(UpdateWght, EmptyString, fpAny|psCheckFileTypeIns, "Copies proposed weight to current");
  xlib_InitMacro(User, EmptyString, fpNone|fpOne, "Changes current folder");
  xlib_InitMacro(Dir, EmptyString, fpNone|fpOne, "Lists current folder. A file name mask may be provided");
  xlib_InitMacro(LstVar, EmptyString, fpAny, "Lists all defined variables. Accepts * based masks" );
  xlib_InitMacro(LstMac, "h-Shows help", fpAny, "Lists all defined macros. Accepts * based masks" );
  xlib_InitMacro(LstFun, "h-Shows help", fpAny, "Lists all defined functions. Accepts * based masks" );
  xlib_InitMacro(LstFS, EmptyString, fpAny, "Prints out detailed content of virtual file system. Accepts * based masks");
  xlib_InitMacro(SGS, EmptyString, fpOne|fpTwo|psFileLoaded, "Changes current space group settings using provided cell setting (if applicable) and axis");
  xlib_InitMacro(ASR, EmptyString, fpNone^psFileLoaded, "Absolute structure refinement: adds TWIN and BASF to current model in the case of non-centrosymmetric structure");
  xlib_InitMacro(Describe, EmptyString, fpNone^psFileLoaded, "Describes current refinement in a human readable form");
  xlib_InitMacro(Sort, EmptyString, fpAny^psFileLoaded, "Sorts atoms of the default residue. Atom sort arguments:\
 m - atomic weight; l - label, considering numbers; p - part, 0 is first followed by all positive parts in ascending order and then negative ones;\
 h - to treat hydrogen atoms independent of the pivot atom.\
 Moiety sort arguments: s - size, h - by heaviest atom, m - molecular weight. Usage: sort [+atom_sort_type] or [Atoms] [moiety [+moety sort type] [moiety atoms]].\
 If just 'moiety' is provided - the atoms will be split into the moieties without sorting.\
 Example: sort +ml F2 F1 moiety +s - will sort atoms by atomic weight and label, put F1 after F2 and form moieties sorted by size.\
 Note that when sorting atoms, any subsequent sort type operates inside the groups created by the preceeding sort types.");
  xlib_InitMacro(SGInfo, "c-include lattice centering matrices&;i-include inversion generated matrices if any", fpNone|fpOne, 
    "Prints space group information.");
  xlib_InitMacro(SAInfo, EmptyString, fpAny, "Finds and prints space groups which include any of the provided systematic absences in the form 'b~~', '~b~' or '~~b'");
  xlib_InitMacro(Inv, "f-force inversion for non-centrosymmetric space groups", fpAny|psFileLoaded, "Inverts whole structure or provided fragments of the structure");
  xlib_InitMacro(Push, EmptyString, (fpAny^(fpNone|fpOne|fpTwo))|psFileLoaded, "Shifts the sctructure (or provided fragments) by the provided translation");
  xlib_InitMacro(Transform, EmptyString, fpAny|psFileLoaded, "Transforms the structure or provided fragments according to the given matrix\
 (a11, a12, a13, a21, a22, a23, a31, a32, a33, t1, t2, t3)");
  xlib_InitMacro(Standardise, EmptyString, fpNone|fpOne|psFileLoaded, "Standardises atom coordinates (similar to HKL standardisation procedure). If '0' is provided as\
 argument, the asymmetric unit content is arranged as close to (0,0,0), while being inside the unit cell as possible");
  xlib_InitMacro(FitCHN, EmptyString, (fpAny^(fpNone|fpOne)),
    "Fits CHN analysis for given formula and observed data given a lits of possible solvents. A mixture of up to 3 solvents only considered,\
 however any number of observed elements can be provided.\
 Example: FitCHN C12H22O11 C:40.1 H:6 N:0 H2O CCl3H" );
  xlib_InitMacro(CalcCHN, EmptyString, fpNone|fpOne, "Calculates CHN composition of current structure or for provided formula" );
  xlib_InitMacro(CalcMass, EmptyString, fpNone|fpOne, "Calculates Mass spectrum of current structure or for provided formula" );
  xlib_InitMacro(Omit, EmptyString, fpOne|fpTwo|fpThree|psCheckFileTypeIns, 
    "removes any particular reflection from the refinement list. If a single number is provided,\
 all reflections with delta(F^2)/esd greater than given number are omitted");
  xlib_InitMacro(Reset, "s-space group&;c-content&;f-alternative file name&;rem-exclude remarks", 
    fpAny|psFileLoaded, "Resets current structure for the solution with ShelX");
  xlib_InitMacro(Degen, "cs-clear selection", fpAny|psFileLoaded, "Prints how many symmetry operators put given atom to the same site");
  xlib_InitMacro(Close, EmptyString, fpNone|psFileLoaded, "Closes currently loaded file");
//_________________________________________________________________________________________________________________________
//_________________________________________________________________________________________________________________________

  xlib_InitFunc(FileName, fpNone|fpOne,
"If no arguments provided, returns file name of currently loaded file, for one\
 argument returns extracted file name");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(FileExt, fpNone|fpOne, "Returns file extension. If no arguments provided - of currently loaded file");
  xlib_InitFunc(FilePath, fpNone|fpOne, "Returns file path. If no arguments provided - of currently loaded file");
  xlib_InitFunc(FileFull, fpNone, "Returns full path of currently loaded file");
  xlib_InitFunc(FileDrive, fpNone|fpOne, "Returns file drive. If no arguments provided - of currently loaded file");
  xlib_InitFunc(Title, fpNone|fpOne, "If the file is loaded, returns it's title else if a parameter passed, it is returned");
  xlib_InitFunc(IsFileLoaded, fpNone, "Returns true/false");
  xlib_InitFunc(IsFileType, fpOne, "Checks type of currently loaded file [ins,res,ires,cif,mol,xyz]");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(BaseDir, fpNone|fpOne, "Returns the startup folder");
  xlib_InitFunc(HKLSrc, fpNone|fpOne|psFileLoaded, "Returns/sets hkl source for currently loaded file");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(LSM, fpNone|psCheckFileTypeIns, "Return current refinement method, L.S. or CGLS currently.");
  xlib_InitFunc(SSM, fpNone|fpOne, "Return current structure solution method, TREF or PATT currently. If current method is unknown\
 and an argument is provided, that argument is returned");
  xlib_InitFunc(Ins, fpOne|psCheckFileTypeIns, "Returns instruction value (all data after the instruction). In case the instruction\
 does not exist it return 'n/a' string");
  xlib_InitFunc(SG, fpNone|fpOne, "Returns space group of currently loaded file.\
 Also takes a string template, where %# is replaced with SG number, %n - short name,\
 %N - full name, %h - html representation of the short name, %H - same as %h for full name,\
 %s - syngony, %HS -Hall symbol" );
  xlib_InitFunc(SGS, fpNone|fpOne|psFileLoaded, "Returns current space settings" );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(ATA, fpAny|psFileLoaded, "Test current structure against database.\
  (Atom Type Assignment). Returns true if any atom type changed" );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(FATA, fpAny|psFileLoaded, "Calculates the diff Fourier map and integrates it to find artifacts around atoms.\
  (Fourier Atom Type Analysis). Returns true if any atom type changed" );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(VSS, fpOne|psFileLoaded, "Validate Structure or Solution.\
  Takes a boolean value. If value is true, the number of tested atoms is limited\
 by the 18A rule. Returns proportion of know atom types to the all atoms number." );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(RemoveSE, fpOne|psFileLoaded, "Returns a new space group name without provided element");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(Run, fpOne, "Same as the macro, executes provided commands (separated by >>) returns true if succeded");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(Lst, fpOne|psCheckFileTypeIns, "returns a value from the Lst file");
}
//..............................................................................
void XLibMacros::macTransform(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  smatd tm;
  if( !Parse<TStrObjList>(Cmds, "mivd", true,&tm.r, &tm.t) )  {
    Error.ProcessingError(__OlxSrcInfo, "invalid transformation matrix" );
    return;
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms;
  xapp.FindSAtoms(Cmds.Text(' '), atoms, true);
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
}
//..............................................................................
void XLibMacros::macPush(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  vec3d pnt;
  if( !Parse<TStrObjList>(Cmds, "vd", true, &pnt) )  {
    Error.ProcessingError(__OlxSrcInfo, "invalid translation" );
    return;
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms;
  xapp.FindSAtoms(Cmds.Text(' '), atoms, true);
  smatd tm;
  tm.I();
  tm.t = pnt;
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
}
//..............................................................................
void XLibMacros::macInv(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool Force = Options.Contains("f");  // forces inversion for sg without center of inversion
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  if( xapp.CheckFileType<TIns>() || xapp.CheckFileType<TCif>() )  {
    try  { sg = &xapp.XFile().GetLastLoaderSG();  }
    catch(...)  {
      Error.ProcessingError(__OlxSrcInfo, "unknown file space group" );
      return;
    }
    if( !sg->IsCentrosymmetric() &&  !Force )  {
      Error.ProcessingError(__OlxSrcInfo, "non-centrosymmetric space group, use -f to force" );
      return;
    }
  }
  TSAtomPList atoms;
  xapp.FindSAtoms(Cmds.Text(' '), atoms, true);
  smatd tm;
  tm.I() *= -1;
  tm.t = sg->GetInversionCenter();
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
}
//..............................................................................
void XLibMacros::macSAInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const TSymmLib& sl = TSymmLib::GetInstance();
  TLog& log = TBasicApp::GetLog();
  TPtrList<TSymmElement> ref, sg_elm;
  if( Cmds.IsEmpty() )  {
    TPtrList<TSpaceGroup> hits, bl_hits;
    for( size_t i=0; i < sl.SymmElementCount(); i++ )
      ref.Add( & sl.GetSymmElement(i) );
    for( size_t i=0; i < sl.SGCount(); i++ )  {
      sl.GetGroup(i).SplitIntoElements(ref, sg_elm);
      if( sg_elm.IsEmpty() )
        hits.Add( &sl.GetGroup(i) );
      else
        sg_elm.Clear();
    }
    TStrList output;
    for( size_t i=0; i < sl.BravaisLatticeCount(); i++ )  {
      for( size_t j=0; j < hits.Count(); j++ )  {
        if( &hits[j]->GetBravaisLattice() == &sl.GetBravaisLattice(i) )
          bl_hits.Add(hits[j]);
      }
      if( bl_hits.IsEmpty() )  continue;
      TTTable<TStrList> tab( bl_hits.Count()/6+((bl_hits.Count()%6) != 0 ? 1 : 0), 6 );
      for( size_t j=0; j < bl_hits.Count(); j++ )
        tab[j/6][j%6] = bl_hits[j]->GetName();
      tab.CreateTXTList(output, sl.GetBravaisLattice(i).GetName(), false, false, "  ");
      log << output << '\n';
      output.Clear();
      bl_hits.Clear();
    }
  }
  else  {
    TPSTypeList<size_t, TSpaceGroup*> hits, bl_hits;
    for( size_t i=0; i < Cmds.Count(); i++ )  {
      TSymmElement* se = sl.FindSymmElement( olxstr(Cmds[i]).Replace('~', '-') );
      if( se == NULL )  {
        E.ProcessingError(__OlxSrcInfo, olxstr("Unknown symmetry element: ") << Cmds[i]);
        return;
      }
      ref.Add(se);
    }
    for( size_t i=0; i < sl.SGCount(); i++ )  {
      sl.GetGroup(i).SplitIntoElements(ref, sg_elm);
      if( !sg_elm.IsEmpty() )  {
        hits.Add( sg_elm.Count(), &sl.GetGroup(i) );
        sg_elm.Clear();
      }
    }
    TStrList output;
    for( size_t i=0; i < sl.BravaisLatticeCount(); i++ )  {
      for( size_t j=0; j < hits.Count(); j++ )  {
        if( &hits.GetObject(j)->GetBravaisLattice() == &sl.GetBravaisLattice(i) )
          bl_hits.Add(hits.GetComparable(j), hits.GetObject(j));
      }
      if( bl_hits.IsEmpty() )  continue;
      TTTable<TStrList> tab( bl_hits.Count()/5+((bl_hits.Count()%5) != 0 ? 1 : 0), 5);
      olxstr tmp;
      for( size_t j=0; j < bl_hits.Count(); j++ )  {
        tmp = bl_hits.GetObject(j)->GetName();
        tmp.Format(10, true, ' ');
        tab[j/5][j%5] << tmp << ' ' << bl_hits.GetComparable(j) << '/' << ref.Count();
      }
      tab.CreateTXTList(output, sl.GetBravaisLattice(i).GetName(), false, false, "  ");
      log << output  << '\n';
      output.Clear();
      bl_hits.Clear();
    }
    log << "Exact matche(s)\n"; 
    TPtrList<TSymmElement> all_elm;
    for( size_t i=0; i < sl.SymmElementCount(); i++ )
      all_elm.Add( & sl.GetSymmElement(i) );
    olxstr exact_match;
    for( size_t i=0; i < hits.Count(); i++ )  {
      if( hits.GetComparable(i) == ref.Count() )  {
        if( !exact_match.IsEmpty() )
          exact_match << ", ";
        hits.GetObject(i)->SplitIntoElements(all_elm, sg_elm);
        bool exact = true;
        for( size_t j=0; j < sg_elm.Count(); j++ )  {
          if( ref.IndexOf(sg_elm[j]) == InvalidIndex )  {
            exact = false;
            break;
          }
        }
        if( exact )  exact_match << '[';
        exact_match << hits.GetObject(i)->GetName();
        if( exact )  exact_match << ']';
        sg_elm.Clear();
      }
    }
    output.Hyphenate(exact_match, 80);
    log << (output << '\n');
    log << "Space groups inclosed in [] have exact match to the provided elements\n";
  }
}
//..............................................................................
void XLibMacros::macSGInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TPtrList<TSpaceGroup> sgList;
    TSymmLib& symlib = TSymmLib::GetInstance();
    for( size_t i=0; i < symlib.BravaisLatticeCount(); i++ )  {
      TBravaisLattice& bl = symlib.GetBravaisLattice(i);
      bl.FindSpaceGroups( sgList );
      TBasicApp::GetLog() << (olxstr("------------------- ") << bl.GetName() << " --- "  << sgList.Count() << '\n' );
      olxstr tmp, tmp1;
      TPSTypeList<int, TSpaceGroup*> SortedSG;
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add( sgList[j]->GetNumber(), sgList[j] );
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetComparable(j) << ')';
        tmp <<tmp1.Format(15, true, ' ');
        tmp1 = EmptyString;
        if( tmp.Length() > 60 )  {
          TBasicApp::GetLog() << ( tmp << '\n' );
          tmp = EmptyString;
        }
      }
      TBasicApp::GetLog() << ( tmp << '\n' );
      sgList.Clear();
    }
    return;
  }
  bool Identity = Options.Contains("i"), 
       Centering = Options.Contains("c");
  TSpaceGroup* sg = TSymmLib::GetInstance().FindGroup(Cmds[0]);
  bool LaueClassPG = false;
  if( sg == NULL )  {
    sg = TSymmLib::GetInstance().FindGroup(olxstr("P") << Cmds[0]);
    if( !sg )  {
      E.ProcessingError(__OlxSrcInfo, "Could not find specified space group/Laue class/Point group: ") << Cmds[0];
      return;
    }
    LaueClassPG = true;
  }
  if( LaueClassPG )  {
    TPtrList<TSpaceGroup> sgList;
    TPSTypeList<int, TSpaceGroup*> SortedSG;
    if( &sg->GetLaueClass() == sg )  {
      TBasicApp::GetLog() << ( olxstr("Space groups of the Laue class ") << sg->GetBareName() << '\n');
      TSymmLib::GetInstance().FindLaueClassGroups( *sg, sgList);
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add(sgList[j]->GetNumber(), sgList[j]);
      olxstr tmp, tmp1;
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetComparable(j) << ')';
        tmp << tmp1.Format(15, true, ' ');
        tmp1 = EmptyString;
        if( tmp.Length() > 60 )  {
          TBasicApp::GetLog() << ( tmp << '\n' );
          tmp = EmptyString;
        }
      }
      TBasicApp::GetLog() << ( tmp << '\n' );
    }
    if( &sg->GetPointGroup() == sg )  {
      sgList.Clear();
      SortedSG.Clear();
      olxstr tmp, tmp1;
      TBasicApp::GetLog() << ( olxstr("Space groups of the point group ") << sg->GetBareName() << '\n');
      TSymmLib::GetInstance().FindPointGroupGroups(*sg, sgList);
      TPSTypeList<int, TSpaceGroup*> SortedSG;
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add( sgList[j]->GetNumber(), sgList[j] );
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetComparable(j) << ')';
        tmp << tmp1.Format(15, true, ' ');
        tmp1 = EmptyString;
        if( tmp.Length() > 60 )  {
          TBasicApp::GetLog() << ( tmp << '\n' );
          tmp = EmptyString;
        }
      }
      TBasicApp::GetLog() << ( tmp << '\n' );
    }
    return;
  }
  TPtrList<TSpaceGroup> AllGroups;
  smatd_list SGMatrices;

  TBasicApp::GetLog() << (sg->IsCentrosymmetric() ? "Centrosymmetric" : "Non centrosymmetric") << '\n';
  TBasicApp::GetLog() << (olxstr("Hall symbol: ") << sg->GetHallSymbol() << '\n');

  TSymmLib::GetInstance().GetGroupByNumber( sg->GetNumber(), AllGroups );
  if( AllGroups.Count() > 1 )  {
    TBasicApp::GetLog() << ("Alternative settings:\n");
    olxstr tmp;
    for( size_t i=0; i < AllGroups.Count(); i++ )  {
      if( AllGroups[i] == sg )  continue;
      tmp << AllGroups[i]->GetName() << '(' << AllGroups[i]->GetFullName() <<  ") ";
    }
    TBasicApp::GetLog() << (tmp << '\n');
  }
  TBasicApp::GetLog() << ( olxstr("Space group number: ") << sg->GetNumber() << '\n');
  TBasicApp::GetLog() << ( olxstr("Crystal system: ") << sg->GetBravaisLattice().GetName() << '\n');
  TBasicApp::GetLog() << ( olxstr("Laue class: ") << sg->GetLaueClass().GetBareName() << '\n');
  TBasicApp::GetLog() << ( olxstr("Point group: ") << sg->GetPointGroup().GetBareName() << '\n');
  short Flags = mattAll;
  if( !Centering )  Flags ^= (mattCentering|mattTranslation);
  if( !Identity )  Flags ^= mattIdentity;
  sg->GetMatrices( SGMatrices, Flags );

  TTTable<TStrList> tab(SGMatrices.Count(), 2);

  for( size_t i=0; i < SGMatrices.Count(); i++ )
    tab[i][0] = TSymmParser::MatrixToSymmEx(SGMatrices[i]);

  TStrList Output;
  tab.CreateTXTList(Output, "Symmetry operators", true, true, ' ');
  if( !sg->GetInversionCenter().IsNull() )  {
    const vec3d& ic = sg->GetInversionCenter();
    TBasicApp::GetLog() << "Inversion center position: " << olxstr::FormatFloat(3, ic[0])
      << ", " << olxstr::FormatFloat(3, ic[1]) << ", " << olxstr::FormatFloat(3, ic[2]) << '\n';
  }
  // possible systematic absences
  Output.Add("Elements causing systematic absences: ");
  TPtrList<TSymmElement> ref, sg_elm;
  for( size_t i=0; i < TSymmLib::GetInstance().SymmElementCount(); i++ )
    ref.Add(TSymmLib::GetInstance().GetSymmElement(i));
  sg->SplitIntoElements(ref, sg_elm);
  if( sg_elm.IsEmpty() )
    Output.Last().String << "none";
  else  {
    for( size_t i=0; i < sg_elm.Count(); i++ )
      Output.Last().String << sg_elm[i]->GetName() << ' ';
  }
  Output.Add(EmptyString);
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void XLibMacros::macSort(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp::GetInstance().XFile().Sort(TStrList(Cmds));
}
//..............................................................................
void XLibMacros::macRun(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "this function requires Olex2 processor implementation");
  TStrList allCmds(Cmds.Text(' '), ">>");
  for( size_t i=0; i < allCmds.Count(); i++ )  {
    op->executeMacroEx(allCmds[i], Error);
    if( !Error.IsSuccessful() )  {
      if( (i+1) < allCmds.Count() )
        op->print("Not all macros in the provided list were executed", olex::mtError);
      break;
    }
  }
}
//..............................................................................
void XLibMacros::macHklStat(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr hklSrc = xapp.LocateHklFile();
  if( !TEFile::Exists( hklSrc ) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  if( Cmds.IsEmpty() )  {
    RefinementModel::HklStat hs = xapp.XFile().GetRM().GetMergeStat();
    TTTable<TStrList> tab(21, 2);
    tab[0][0] << "Total reflections (after filtering)";   tab[0][1] << hs.TotalReflections;
    tab[1][0] << "Unique reflections";            tab[1][1] << hs.UniqueReflections;
    tab[2][0] << "Centric reflections";           tab[2][1] << hs.CentricReflections;
    tab[3][0] << "Friedel pairs merged";          tab[3][1] << hs.FriedelOppositesMerged;
    tab[4][0] << "Inconsistent equivalents";     tab[4][1] << hs.InconsistentEquivalents;
    tab[5][0] << "Systematic absences removed";   tab[5][1] << hs.SystematicAbsentcesRemoved;
    tab[6][0] << "Min d";                         tab[6][1] << olxstr::FormatFloat(3, hs.MinD);
    tab[7][0] << "Max d";                         tab[7][1] << olxstr::FormatFloat(3, hs.MaxD);
    tab[8][0] << "Limiting d min (SHEL)";         tab[8][1] << olxstr::FormatFloat(3, hs.LimDmin);
    tab[9][0] << "Limiting d max (SHEL/OMIT_2t)";    tab[9][1] << hs.LimDmax;
    tab[10][0] << "Filtered off reflections (SHEL/OMIT_s/OMIT_2t)";  tab[10][1] << hs.FilteredOff;
    tab[11][0] << "Reflections omitted by user (OMIT_hkl)";   tab[11][1] << hs.OmittedByUser;
    tab[12][0] << "Reflections skipped (after 0 0 0)";        tab[12][1] << hs.OmittedReflections;
    tab[13][0] << "Intensity transformed for (OMIT_s)";       tab[13][1] << hs.IntensityTransformed << " reflections";
    tab[14][0] << "Rint";                         tab[14][1] << olxstr::FormatFloat(3, hs.Rint);
    tab[15][0] << "Rsigma";                       tab[15][1] << olxstr::FormatFloat(3, hs.Rsigma);
    tab[16][0] << "Mean I/sig";                   tab[16][1] << olxstr::FormatFloat(3, hs.MeanIOverSigma);
    tab[17][0] << "HKL range";                    
    tab[17][1] << "h=[" << hs.MinIndexes[0] << ',' << hs.MaxIndexes[0] << "] "
               << "k=[" << hs.MinIndexes[1] << ',' << hs.MaxIndexes[1] << "] "
               << "l=[" << hs.MinIndexes[2] << ',' << hs.MaxIndexes[2] << "] ";
    tab[18][0] << "Maximum redundancy (+symm eqivs)";    tab[18][1] << hs.ReflectionAPotMax;
    tab[19][0] << "Average redundancy (+symm eqivs)";    tab[19][1] << olxstr::FormatFloat(2, (double)hs.TotalReflections/hs.UniqueReflections);

    TStrList Output;
    tab.CreateTXTList(Output, olxstr("Refinement reflection statistsics"), true, false, "  ");
    xapp.GetLog() << Output << '\n';
    const TIntList& redInfo = xapp.XFile().GetRM().GetRedundancyInfo();
    int red_cnt = 0;
    for( size_t i=0; i < redInfo.Count(); i++ )
      if( redInfo[i] != 0 )
        red_cnt++;
    tab.Resize( red_cnt, 2 );
    tab.ColName(0) = "Times measured";
    tab.ColName(1) = "Count";
    red_cnt = 0;
    for( size_t i=0; i < redInfo.Count(); i++ )  {
      if( redInfo[i] == 0 )  continue;
      tab[red_cnt][0] = i+1;
      tab[red_cnt++][1] = redInfo[i];
    }
    Output.Clear();
    tab.CreateTXTList(Output, olxstr("All reflection statistics"), true, false, "  ");
    xapp.GetLog() << Output << '\n';
    //const vec3i_list empty_omits;
    //MergeStats fr_ms = RefMerger::DryMergeInP1<RefMerger::UnitMerger>(xapp.XFile().GetRM().GetFriedelPairs(), empty_omits);
    xapp.GetLog() << "Friedel pairs measured: " << xapp.XFile().GetRM().GetFriedelPairCount() << '\n';
    return;
  }
  bool list = Options.Contains("l"), 
       merge = Options.Contains("m");
  TRefList Refs;
  if( merge )
    xapp.XFile().GetRM().GetRefinementRefList<RefMerger::StandardMerger>(xapp.XFile().GetLastLoaderSG(), Refs);
  else
    xapp.XFile().GetRM().GetFilteredP1RefList<RefMerger::StandardMerger>(Refs);
  evecd_list con;

  for( size_t i=0; i < Cmds.Count(); i++ )  {
    size_t obi = Cmds[i].FirstIndexOf('[');
    if( obi == InvalidIndex || !Cmds[i].EndsWith(']') )  {
      Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cmds[i];
      return;
    }
    con.AddNew(4);
    con[i][3] = Cmds[i].SubStringTo(obi).ToInt();
    olxstr tmp = Cmds[i].SubString(obi+1, Cmds[i].Length() - obi - 2);
    int hkli=-1;
    for( size_t j=tmp.Length()-1; j != InvalidIndex; j-- ) {
      if( tmp.CharAt(j) == 'l' )  hkli = 2;
      else if( tmp.CharAt(j) == 'k' )  hkli = 1;
      else if( tmp.CharAt(j) == 'h' )  hkli = 0;
      if( hkli == -1 )  {
        Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cmds[i];
        return;
      }
      j--;
      olxstr strV;
      while( j != InvalidIndex && !(tmp[j] >= 'a' && tmp[j] <= 'z' ) )  {
        strV.Insert( (olxch)tmp[j], 0 );
        j--;
      }
      if( !strV.IsEmpty() && !(strV == "+") && !(strV == "-") )
        con[i][hkli] = strV.ToDouble();
      else  {
        if( !strV.IsEmpty() && strV == "-" )
          con[i][hkli] = -1.0;
        else
          con[i][hkli] = 1.0;
      }
      if( con[i][hkli] == 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "illegal value: ") << Cmds[i];
        return;
      }
      j++;
    }
  }
  double SI = 0, SE = 0;
  size_t count = 0;
  for( size_t i=0; i < Refs.Count(); i++ )  {
    bool fulfilled = true;
    const TReflection& ref = Refs[i];
    for( size_t j=0; j < Cmds.Count(); j ++ )  {
      int v = olx_round(ref.GetH()*con[j][0] +
                    ref.GetK()*con[j][1] +
                    ref.GetL()*con[j][2] );
      if( con[j][3] == 0 )  {
        if( v != 0 ) {
          fulfilled = false;
          break;
        }
      }
      else if( con[j][3] < 0 )  {
        if( (v%(int)con[j][3]) == 0 )  {
          fulfilled = false;
          break;
        }
      }
      else if( con[j][3] > 0 )  {
        if( (v%(int)con[j][3]) != 0 )  {
          fulfilled = false;
          break;
        }
      }
    }
    if( !fulfilled )  continue;
    count ++;
    SI += ref.GetI();
    SE += olx_sqr(ref.GetS());
    if( list )  {
      TBasicApp::GetLog() << ref.ToString()<< '\n';
    }
  }
  if( count == 0 )  {
    TBasicApp::GetLog() << ("Could not find any reflections fulfilling given condition\n");
    return;
  }
  SI /= count;
  SE = sqrt(SE/count);

  xapp.GetLog() << ( olxstr("Found " ) << count << " reflections fulfilling given condition\n");
  xapp.GetLog() << ( olxstr("I(s) is ") << olxstr::FormatFloat(3, SI) << '(' << olxstr::FormatFloat(3, SE) << ")\n" );

}
//..............................................................................
void XLibMacros::macHtab(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( TXApp::GetInstance().XFile().GetLattice().IsGenerated() )  {
    E.ProcessingError(__OlxSrcInfo, "operation is not applicable to the grown structure");
    return;
  }
  double max_d = 2.9, min_ang = 150.0;
  size_t cnt = XLibMacros::ParseNumbers<double,TStrObjList>(Cmds, 2, &max_d, &min_ang);
  if( cnt == 1 )  {
    if( max_d > 100 )  {
      min_ang = max_d;
      max_d = 2.9;
    }
    else if( max_d > 5 )
      max_d = 2.9;
  }
  TIntList bais;
  bais.Add(iNitrogenIndex);
  bais.Add(iOxygenIndex);
  bais.Add(iFluorineIndex);
  bais.Add(iChlorineIndex);
  bais.Add(iSulphurIndex);
  TBasicApp::GetLog() << "Processing HTAB with max D-A distance " << max_d << " and minimum angle " << min_ang << '\n';
  min_ang = cos(min_ang*M_PI/180.0);
  if( Options.Contains('t') )  {
    TStrList elm(Options.FindValue('t'), ',');
    for( size_t i=0; i < elm.Count(); i++ )  {
      cm_Element* e = XElementLib::FindBySymbol(elm[i]);
      if( e == NULL )
        TBasicApp::GetLog() << (olxstr("Unknown element type: ") << elm[i] << '\n');
      else if( bais.IndexOf(e->index) == InvalidIndex )
        bais.Add(e->index);
    }
  }
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  TUnitCell& uc = TXApp::GetInstance().XFile().GetUnitCell();
  TLattice& lat = TXApp::GetInstance().XFile().GetLattice();
  TIns& ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  TArrayList< AnAssociation2<TCAtom const*, smatd> > all;
  size_t h_indexes[4];
  for( size_t i=0; i < lat.AtomCount(); i++ )  {
    TSAtom& sa = lat.GetAtom(i);
    const cm_Element& elm = sa.GetType();
    if( elm.GetMr() < 3.5 )  // H,D,Q
      continue;
    size_t hc = 0;
    for( size_t j=0; j < sa.NodeCount(); j++ )  {
      const cm_Element& elm1 = sa.Node(j).GetType();
      if( elm1 == iHydrogenZ )  {
        h_indexes[hc] = j;
        hc++;
        if( hc >= 4 )
          break;
      }
    }
    if( hc == 0 || hc >= 4 )  continue;
    all.Clear();
    uc.FindInRangeAM(sa.ccrd(), max_d+elm.r_bonding-0.6, all);
    for( size_t j=0; j < all.Count(); j++ )  {
      const TCAtom& ca = *all[j].GetA();
      const cm_Element& elm1 = ca.GetType();
      if(  bais.IndexOf(elm1.z) == InvalidIndex )  continue;
      vec3d cvec(all[j].GetB()*ca.ccrd()),
            bond(cvec - sa.ccrd());
      const double d = au.CellToCartesian(bond).Length();
      if( d < (elm.r_bonding + elm1.r_bonding + 0.4) ) // coval bond
        continue;  
      // analyse angles
      for( size_t k=0; k < hc; k++ )  {
        vec3d base = sa.Node(h_indexes[k]).ccrd();
        vec3d v1 = sa.ccrd() - base;
        vec3d v2 = cvec - base;
        au.CellToCartesian(v1);
        au.CellToCartesian(v2);
        const double c_a = v1.CAngle(v2);
        if( c_a < min_ang )  {  // > 150 degrees
          if( sa.GetType() == iCarbonZ )  {
            InfoTab& it_d = rm.AddRTAB(sa.GetType().symbol + ca.GetType().symbol);
            it_d.AddAtom( &sa.CAtom(), NULL );
            const smatd* mt = (!(all[j].GetB().t.IsNull() && all[j].GetB().r.IsI()) ? &all[j].GetB() : NULL);
            it_d.AddAtom(const_cast<TCAtom*>(&ca), mt);
            if( rm.ValidateInfoTab(it_d) )
              TBasicApp::GetLog() << it_d.InsStr() << " d=" << olxstr::FormatFloat(3, d) << '\n';

            InfoTab& it_a = rm.AddRTAB(sa.GetType().symbol + ca.GetType().symbol);
            it_a.AddAtom( &sa.CAtom(), NULL );
            it_a.AddAtom( &sa.Node(h_indexes[k]).CAtom(), NULL );
            it_a.AddAtom(const_cast<TCAtom*>(&ca), mt);
            if( rm.ValidateInfoTab(it_a) )
              TBasicApp::GetLog() << it_a.InsStr() << " a=" << olxstr::FormatFloat(3, acos(c_a)*180.0/M_PI) << '\n';
          }
          else  {
            InfoTab& it = rm.AddHTAB();
            it.AddAtom(&sa.CAtom(), NULL);
            const smatd* mt = (!(all[j].GetB().t.IsNull() && all[j].GetB().r.IsI()) ? &all[j].GetB() : NULL);
            it.AddAtom(const_cast<TCAtom*>(&ca), mt);
            if( rm.ValidateInfoTab(it) )
              TBasicApp::GetLog() << it.InsStr() << " d=" << olxstr::FormatFloat(3, d) << '\n';
          }
        }
      }
    }
  }
}
//..............................................................................
void XLibMacros::macHAdd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp &XApp = TXApp::GetInstance();
  if( XApp.XFile().GetLattice().IsGenerated() )  {
    Error.ProcessingError(__OlxSrcInfo, "command is not applicable to grown structure");
    return;
  }
  int Hfix = 0;
  if( !Cmds.IsEmpty() && Cmds[0].IsNumber() )  {
    Hfix = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  TAsymmUnit &au = XApp.XFile().GetAsymmUnit();
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom &ca = au.GetAtom(i);
    if( ca.GetType() == iHydrogenZ )
      ca.SetDetached(false);
  }
  TActionQueue* q_draw = XApp.ActionQueue(olxappevent_GL_DRAW);
  if( q_draw != NULL )  q_draw->SetEnabled(false);
  XApp.XFile().GetLattice().UpdateConnectivity();
  try  {
    TSAtomPList satoms;
    XApp.FindSAtoms( Cmds.Text(' '), satoms, true );
    TXlConGen xlConGen( XApp.XFile().GetRM() );
    if( Hfix == 0 ) 
      XApp.XFile().GetLattice().AnalyseHAdd(xlConGen, satoms);
    else  {
      RefinementModel& rm = XApp.XFile().GetRM();
      for( size_t aitr=0; aitr < satoms.Count(); aitr++ )  {
        TIntList parts;
        TDoubleList occu;
        TAtomEnvi AE;
        XApp.XFile().GetUnitCell().GetAtomEnviList(*satoms[aitr], AE);
        for( size_t i=0; i < AE.Count(); i++ )  {
          if( AE.GetCAtom(i).GetPart() != 0 && AE.GetCAtom(i).GetPart() != AE.GetBase().CAtom().GetPart() ) 
            if( parts.IndexOf(AE.GetCAtom(i).GetPart()) == InvalidIndex )  {
              parts.Add( AE.GetCAtom(i).GetPart() );
              occu.Add( rm.Vars.GetParam(AE.GetCAtom(i), catom_var_name_Sof) );
            }
        }
        if( parts.Count() < 2 )  {
          int afix = TXlConGen::ShelxToOlex(Hfix, AE);
          if( afix != -1 )  {
            TCAtomPList generated;
            xlConGen.FixAtom(AE, afix, XElementLib::GetByIndex(iHydrogenIndex), NULL, &generated);
            if( !generated.IsEmpty() && generated[0]->GetParentAfixGroup() != NULL ) // hack to get desired Hfix...
              generated[0]->GetParentAfixGroup()->SetAfix(Hfix);
          }
          else  {
            XApp.GetLog() << (olxstr("Failed to translate HFIX code for ") << satoms[aitr]->GetLabel() << 
              " with " << AE.Count() << " bonds\n");
          }
        }
        else  {
          TCAtomPList generated;
          XApp.GetLog() << (olxstr("Processing ") << parts.Count() << " parts\n");
          for( size_t i=0; i < parts.Count(); i++ )  {
            AE.Clear();
            XApp.XFile().GetUnitCell().GetAtomEnviList(*satoms[aitr], AE, false, parts[i]);
            //consider special case where the atom is bound to itself but very long bond > 1.6 A
            smatd* eqiv = NULL;
            for( size_t j=0; j < AE.Count(); j++ )  {
              if( &AE.GetCAtom(j) == &AE.GetBase().CAtom() )  {
                const double d = AE.GetCrd(j).DistanceTo(AE.GetBase().crd() );
                if( d > 1.6 )  {
                  eqiv = new smatd(AE.GetMatrix(j));
                  AE.Delete(j);
                  break;
                }
              }
            }
            if( eqiv != NULL )  {
              TIns& ins = XApp.XFile().GetLastLoader<TIns>();
              const smatd& e = rm.AddUsedSymm(*eqiv);
              rm.Conn.RemBond(satoms[aitr]->CAtom(), satoms[aitr]->CAtom(), NULL, &e, true);
              XApp.GetLog() << (olxstr("The atom" ) << satoms[aitr]->GetLabel() << 
                " is connected to itself through symmetry, removing the symmetry generated bond\n");
              delete eqiv;
            }
            //
            int afix = TXlConGen::ShelxToOlex(Hfix, AE);
            if( afix != -1 )  {
              xlConGen.FixAtom(AE, afix, XElementLib::GetByIndex(iHydrogenIndex), NULL, &generated);
              for( size_t j=0; j < generated.Count(); j++ )  {
                generated[j]->SetPart( parts[i] );
                rm.Vars.SetParam(*generated[j], catom_var_name_Sof, occu[i]);
              }
              if( !generated.IsEmpty() && generated[0]->GetParentAfixGroup() != NULL )
                generated[0]->GetParentAfixGroup()->SetAfix(Hfix); // a hack again
              generated.Clear();
            }
            else  {
              XApp.GetLog() << (olxstr("Failed to translate HFIX code for ") << satoms[aitr]->GetLabel() << 
                " with " << AE.Count() << " bonds\n");
            }
          }
        }
      }
    }
  }
  catch(const TExceptionBase& e)  {
    Error.ProcessingError(__OlxSrcInfo, e.GetException()->GetError());
  }
  if( q_draw != NULL )  q_draw->SetEnabled(true);
  XApp.XFile().GetLattice().Init();
  delete XApp.FixHL();
}
//..............................................................................
void XLibMacros::macHImp(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& XApp = TXApp::GetInstance();
  if( XApp.XFile().GetLattice().IsGenerated() )  {
    Error.ProcessingError(__OlxSrcInfo, "The procedure is not applicable for the grown structure");
    return;
  }
  bool increase = false, 
    decrease = false;
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo, "first arument should be a number or +/- number");
    return;
  }
  double val = Cmds[0].ToDouble();
  if( Cmds[0].CharAt(0) == '+' )
    increase = true;
  else if( Cmds[0].CharAt(0) == '-' )
    decrease = true;
  Cmds.Delete(0);

  TSAtomPList satoms;
  XApp.FindSAtoms( Cmds.Text(' '), satoms, true );
  const double delta = XApp.XFile().GetLattice().GetDelta();
  for( size_t i=0; i < satoms.Count(); i++ )  {
    if( satoms[i]->GetType() != iHydrogenZ )
      continue;
    TSAtom& h = *satoms[i], *attached = NULL;
    size_t ac = 0;
    for( size_t j=0; j < h.NodeCount(); j++ )  {
      TSAtom& n = h.Node(j);
      if( !(n.IsDeleted() || n.GetType() == iQPeakZ) )  {
        ac++;
        attached = &n;
      }
    }
    if( ac > 1 || ac == 0 )  {
      XApp.GetLog() << "Skipping " << h.GetLabel() << '\n';
      continue;
    }
    vec3d v( h.crd() - attached->crd() );
    if( increase || decrease )
      v.NormaliseTo( v.Length() + val );
    else
      v.NormaliseTo(val);
    v += attached->crd();
    double qd1 = v.QDistanceTo(attached->crd());
    double qd2 =  attached->GetType().r_bonding + h.GetType().r_bonding + delta;
    qd2 *= qd2;
    if( qd1 >= qd2-0.01 )  {
      XApp.GetLog() << "Skipping " << h.GetLabel() << '\n';
      continue;
    }
    h.crd() = v;
    XApp.XFile().GetAsymmUnit().CartesianToCell(v);
    h.CAtom().ccrd() = v;
    h.ccrd() = v;
  }
  XApp.XFile().GetLattice().UpdateConnectivity();
  //XApp.XFile().EndUpdate();
}
//..............................................................................
void XLibMacros::macAnis(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  TCAtomPList catoms;
  TListCaster::POP(atoms, catoms);
  bool useH = Options.Contains("h");
  for( size_t i=0; i < catoms.Count(); i++ )
    if( !useH && catoms[i]->GetType() == iHydrogenZ )
      catoms[i] = NULL;
  catoms.Pack();
  TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, true);
}
//..............................................................................
void XLibMacros::macIsot(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  TCAtomPList catoms;
  TListCaster::POP(atoms, catoms);
  TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, false);
}
//..............................................................................
void XLibMacros::macFix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars( Cmds[0] );
  Cmds.Delete(0);
  double var_val = 0;
  if( !Cmds.IsEmpty() && Cmds[0].IsNumber() )  {
    var_val = Cmds[0].ToDouble();
    Cmds.Delete(0);
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms;
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms, true, true) )  
    return;

  if( vars.Equalsi( "XYZ" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      for( short j=0; j < 3; j++ )
        xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_X+j);
    }
  }
  else if( vars.Equalsi( "UISO" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL )  // isotropic atom
        xapp.SetAtomUiso(*atoms[i], var_val);
      else  {
        for( short j=0; j < 6; j++ )
          xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_U11+j);
      }
    }
  }
  else if( vars.Equalsi( "OCCU" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_Sof);
      if( var_val == 0 )  {
        if( atoms[i]->CAtom().GetPart() == 0 )  // else leave as it is
          atoms[i]->CAtom().SetOccu( 1./atoms[i]->CAtom().GetDegeneracy() );
      }
      else
        atoms[i]->CAtom().SetOccu( var_val );
    }
  }
}
//..............................................................................
void XLibMacros::macFree(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars = Cmds[0];
  Cmds.Delete(0);
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms, true, true) )  return;
  if( vars.Equalsi( "XYZ" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      for( short j=0; j < 3; j++ )
        xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_X+j);
    }
  }
  else if( vars.Equalsi( "UISO" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->CAtom().GetEllipsoid() == NULL )  {  // isotropic atom
        xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_Uiso);
      }
      else  {
        for( short j=0; j < 6; j++ )
          xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_U11+j);
      }
      atoms[i]->CAtom().SetUisoOwner(NULL);
    }
  }
  else if( vars.Equalsi( "OCCU" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ ) 
      xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_Sof);
  }
}
//..............................................................................
void XLibMacros::macFixHL(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp & xapp = TXApp::GetInstance();
  // do not print a warning...
  if( xapp.XFile().GetLattice().IsGenerated() )  return;
  TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
  TEBitArray detached((uint32_t)au.AtomCount());
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom &ca = au.GetAtom(i);
    detached.Set(i, ca.IsDetached());
    if( ca.GetType() == iQPeakZ )
      ca.SetDetached(true);
    else if( ca.GetType().GetMr() < 3.5 )
      ca.SetDetached(false);
  }
  TActionQueue* q_draw = xapp.ActionQueue(olxappevent_GL_DRAW);
  if( q_draw != NULL )  q_draw->SetEnabled(false);
  xapp.XFile().GetLattice().UpdateConnectivity();
  delete TXApp::GetInstance().FixHL();
  for( size_t i=0; i < au.AtomCount(); i++ )
    au.GetAtom(i).SetDetached(detached[i]);
  xapp.XFile().GetLattice().UpdateConnectivity();
  if( q_draw != NULL )  q_draw->SetEnabled(true);
}
//..............................................................................
// http://www.minsocam.org/ammin/AM78/AM78_1104.pdf
int macGraphPD_Sort(const AnAssociation2<double,double>& a1, const AnAssociation2<double,double>& a2) {
  const double df = a1.GetA() - a2.GetA();
  if( df < 0 )  return -1;
  if( df > 0 )  return 1;
  return 0;
}
void XLibMacros::macGraphPD(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TRefList refs;
  double res = Options.FindValue("r", "0.5").ToDouble();
  TArrayList<compd > F;
  olxstr err( SFUtil::GetSF(refs, F, SFUtil::mapTypeObs, 
    Options.Contains("fcf") ? SFUtil::sfOriginFcf : SFUtil::sfOriginOlex2, 
    (Options.FindValue("s", "r").ToLowerCase().CharAt(0) == 'r') ? SFUtil::scaleRegression : SFUtil::scaleSimple) );
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TEFile out( TEFile::ExtractFilePath(xapp.XFile().GetFileName()) << "olx_pd_calc.csv", "w+b");
  const mat3d& hkl2c = xapp.XFile().GetAsymmUnit().GetHklToCartesian();
  const double d_2_sin = xapp.XFile().GetRM().expl.GetRadiation()/2.0;
  TTypeList< AnAssociation2<double,double> > gd;
  gd.SetCapacity( refs.Count() );
  double max_2t = 0, min_2t=180;
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    vec3d hkl = ref.ToCart(hkl2c);
    const double theta_2 = 360*asin(d_2_sin*hkl.Length())/M_PI;
    gd.AddNew( theta_2, ref.GetI()*ref.GetMultiplicity());
  }
  gd.QuickSorter.SortSF(gd, macGraphPD_Sort);
  min_2t = gd[0].GetA();
  max_2t = gd.Last().GetA();
  const double sig_0 = 1./80. + (max_2t-min_2t)/800.0;
  const size_t ref_cnt = refs.Count();
  for( double s = min_2t; s <= max_2t; s += res )  {
    double y = 0.0001;  
    for( size_t i=0; i < ref_cnt; i++ )  { 
      const double sig = sig_0*(1.0+gd[i].GetA()/140.0);
      const double qsig = sig*sig;
      y += gd[i].GetB()*exp(-(s-gd[i].GetA())*(s-gd[i].GetA())/(2*qsig))/sig;
    }
    out.Writenl( olxcstr(s, 40) << ',' << y);
  }

}
//..............................................................................
void XLibMacros::macFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& XApp = TXApp::GetInstance();
  olxstr Tmp;
  if( Cmds.IsEmpty() )  {  // res -> Ins rotation if ins file
    if( XApp.CheckFileType<TIns>() )  {
      Tmp = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "ins");
    }
    else
      Tmp = XApp.XFile().GetFileName();
  }
  else
    Tmp = Cmds[0];

  bool Sort = Options.Contains('s');

  if( TEFile::ExtractFilePath(Tmp).IsEmpty() )
    Tmp = TEFile::AddPathDelimeter(CurrentDir) + Tmp;
  TEBitArray removedSAtoms, removedCAtoms;
  if( TEFile::ExtractFileExt(Tmp).Equalsi("ins"))  {  // kill Q peak in the ins file
    TLattice& latt = XApp.XFile().GetLattice();
    removedSAtoms.SetSize(latt.AtomCount());
    for( size_t i=0; i < latt.AtomCount(); i++ )  {
      TSAtom& sa = latt.GetAtom(i);
      if( sa.GetType() == iQPeakZ && !sa.IsDeleted() )  {
        sa.SetDeleted(true);
        removedSAtoms.SetTrue(i);
      }
    }
    TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
    removedCAtoms.SetSize(au.AtomCount());
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      TCAtom& ca = au.GetAtom(i);
      if( ca.GetType() == iQPeakZ && !ca.IsDeleted() )  {
        ca.SetDeleted(true);
        removedCAtoms.SetTrue(i);
      }
    }
  }
  
  XApp.XFile().SaveToFile(Tmp, Sort);
  if( XApp.XFile().HasLastLoader() )  {
    olxstr fd = TEFile::ExtractFilePath(Tmp);
    if( !fd.IsEmpty() && !fd.Equalsi(CurrentDir) )  {
      if( !TEFile::ChangeDir(fd) )
        TBasicApp::GetLog().Error("Cannot change current folder...");
      else
        CurrentDir = fd;
    }
  }
  else  if( !Sort )  {
    Sort = true;  // forse reading the file
  }
  if( !removedSAtoms.IsEmpty() )  {  // need to restore, a bit of mess here...
    TLattice& latt = XApp.XFile().GetLattice();
    for( size_t i=0; i < latt.AtomCount(); i++ )  {
      if( removedSAtoms.Get(i) )
        latt.GetAtom(i).SetDeleted(false);
    }
    TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      if( removedCAtoms[i] )
          au.GetAtom(i).SetDeleted(false);
    }
  }
  if( Sort )  {
    olex::IOlexProcessor* op = olex::IOlexProcessor::GetInstance();
      if( op != NULL )
        op->executeMacro(olxstr("reap \'") << Tmp << '\'');
  }
}
//..............................................................................
void XLibMacros::macFuse(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds.Count() == 1 && Cmds[0].IsNumber() )  {
    const double th = Cmds[0].ToDouble();
    TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
    for( size_t i=0; i < latt.AtomCount(); i++ )  {
      TSAtom& sa = latt.GetAtom(i);
      if( sa.IsDeleted() )  continue;
      if( sa.BondCount() == 0 )  continue;
      sa.SortBondsByLengthAsc();
      vec3d cnt(sa.crd());
      size_t ac = 1;
      for( size_t j=0; j < sa.BondCount(); j++ )  {
        if( sa.Bond(j).Length() < th )  {
          TSAtom& asa = sa.Bond(j).Another(sa);
          if( asa.GetType() != sa.GetType() )
            continue;
          ac++;
          cnt += asa.crd();
          asa.CAtom().SetDeleted(true);
          asa.SetDeleted(true);
        }    
        else
          break;
      }
      if( ac > 1 )  {
        cnt /= ac;
        sa.CAtom().ccrd() = latt.GetAsymmUnit().CartesianToCell(cnt);
      }
    }
    TXApp::GetInstance().XFile().GetLattice().Uniq(true);
  }
  else
    TXApp::GetInstance().XFile().GetLattice().Uniq( Options.Contains("f") );
}
//..............................................................................
void XLibMacros::macLstIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  bool remarks = Options.Contains("r");
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  TBasicApp::GetLog() << ("List of current instructions:\n");
  olxstr Tmp;
  for( size_t i=0; i < Ins.InsCount(); i++ )  {
    if( !remarks && Ins.InsName(i).Equalsi("REM") )  continue;
    Tmp = i;  Tmp.Format(3, true, ' ');
    Tmp << Ins.InsName(i) << ' ' << Ins.InsParams(i).Text(' ');
    TBasicApp::GetLog() << (Tmp << '\n');
  }
}
//..............................................................................
void XLibMacros::macAddIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // if instruction is parsed, it goes to current model, otherwise i stays in the ins file
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if( !Ins.AddIns(TStrList(Cmds), TXApp::GetInstance().XFile().GetRM()) )  {
    Error.ProcessingError(__OlxSrcInfo, olxstr("could not add instruction: ") << Cmds.Text(' ') );
    return;
  }
}
//..............................................................................
void XLibMacros::macDelIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  bool isOmit = false;
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if( Cmds[0].IsNumber() )  {
    int insIndex = Cmds[0].ToInt();
    Ins.DelIns(insIndex);
  }
  else  {
    if( Cmds[0].Equalsi("OMIT") )
      TXApp::GetInstance().XFile().GetRM().ClearOmits();
    else  {
      for( size_t i=0; i < Ins.InsCount(); i++ )  {
        if( Ins.InsName(i).Equalsi(Cmds[0]) )  {
          Ins.DelIns(i);  i--;  continue;
        }
      }
    }
  }
  OnDelIns.Exit(NULL, &Cmds[0]);
}
//..............................................................................
void XLibMacros::macLS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  int ls = -1;
  XLibMacros::ParseNumbers<int,TStrObjList>(Cmds, 1, &ls);
  if( ls != -1 )  
    TXApp::GetInstance().XFile().GetRM().SetIterations( (int)ls);
  if( !Cmds.IsEmpty() )
    TXApp::GetInstance().XFile().GetRM().SetRefinementMethod( Cmds[0] );
}
//..............................................................................
void XLibMacros::macUpdateWght(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  if( rm.proposed_weight.Count() == 0 )  return;
  if( Cmds.IsEmpty() )  
    rm.used_weight = rm.proposed_weight;
  else  {
    rm.used_weight.SetCount(Cmds.Count());
    for( size_t i=0; i < Cmds.Count(); i++ )  
      rm.used_weight[i] = Cmds[i].ToDouble();
  }
}
//..............................................................................
void XLibMacros::macUser(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::GetLog() << TEFile::CurrentDir() << '\n';
  }
  else if( !TEFile::ChangeDir(Cmds[0]) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not change current folder" );
  }
  else
    CurrentDir = Cmds[0]; 
}
//..............................................................................
void XLibMacros::macDir(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr Filter( Cmds.IsEmpty() ? olxstr("*.*")  : Cmds[0] );
  TStrList Output;
  TFileList fl;
  TEFile::ListCurrentDirEx(fl, Filter, sefFile|sefDir);
  TTTable<TStrList> tab(fl.Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Last Modified";
  tab.ColName(3) = "Attributes";

  TFileListItem::SortListByName(fl);

  for( size_t i=0; i < fl.Count(); i++ )  {
    tab[i][0] = fl[i].GetName();
    if( (fl[i].GetAttributes() & sefDir) != 0 )
      tab[i][1] = "Folder";
    else
      tab[i][1] = fl[i].GetSize();
    tab[i][2] = TETime::FormatDateTime("yyyy.MM.dd hh:mm:ss", fl[i].GetModificationTime());
    if( (fl[i].GetAttributes() & sefReadOnly) != 0 )
      tab[i][3] << 'r';
    if( (fl[i].GetAttributes() & sefWriteOnly) != 0 )
      tab[i][3] << 'w';
    if( (fl[i].GetAttributes() & sefHidden) != 0 )
      tab[i][3] << 'h';
    if( (fl[i].GetAttributes() & sefSystem) != 0 )
      tab[i][3] << 's';
    if( (fl[i].GetAttributes() & sefExecute) != 0 )
      tab[i][3] << 'e';
  }

  tab.CreateTXTList(Output, "Directory list", true, true, ' ');
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void XLibMacros::macLstMac(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  TBasicFunctionPList macros;
  TXApp::GetInstance().GetLibrary().ListAllMacros( macros );
  for( size_t i=0; i < macros.Count(); i++ )  {
    ABasicFunction* func = macros[i];
    bool add = false;
    olxstr fn = func->GetQualifiedName();
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(fn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    TBasicApp::GetLog() << (fn << " - " << func->GetSignature() << '\n');
  }
}
//..............................................................................
void XLibMacros::macLstFun(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  TBasicFunctionPList functions;
  TXApp::GetInstance().GetLibrary().ListAllFunctions( functions );
  for( size_t i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    bool add = false;
    olxstr fn = func->GetQualifiedName();
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(fn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    TBasicApp::GetLog() << (fn << " - " << func->GetSignature() << '\n');
  }
}
//..............................................................................
void XLibMacros::ChangeCell(const mat3d& tm, const TSpaceGroup& new_sg)  {
  TXApp& xapp = TXApp::GetInstance();
  TBasicApp::GetLog() << (olxstr("Cell choice trasformation matrix: \n"));
  TBasicApp::GetLog() << tm[0].ToString() << '\n';
  TBasicApp::GetLog() << tm[1].ToString() << '\n';
  TBasicApp::GetLog() << tm[2].ToString() << '\n';
  TBasicApp::GetLog() << ((olxstr("New space group: ") << new_sg.GetName() << '\n'));
  const mat3d tm_t( mat3d::Transpose(tm) );
  xapp.XFile().UpdateAsymmUnit();
  TAsymmUnit& au = xapp.XFile().LastLoader()->GetAsymmUnit();
  const mat3d i_tm( tm.Inverse() );
  mat3d f2c( mat3d::Transpose(xapp.XFile().GetAsymmUnit().GetCellToCartesian())*tm );
  mat3d ax_err;
  ax_err[0] = vec3d(olx_sqr(au.Axes()[0].GetE()), olx_sqr(au.Axes()[1].GetE()), olx_sqr(au.Axes()[2].GetE()));
  ax_err[1] = ax_err[0];  ax_err[2] = ax_err[0];
  mat3d an_err;
  an_err[0] = vec3d(olx_sqr(au.Angles()[0].GetE()), olx_sqr(au.Angles()[1].GetE()), olx_sqr(au.Angles()[2].GetE()));
  an_err[1] = an_err[0];  an_err[2] = an_err[0];
  // prepare positive matrix for error estimation
  mat3d tm_p(tm);
  for( size_t i=0; i < 3; i++ )
    for( size_t j=0; j < 3; j++ )
      if( tm_p[i][j] < 0 ) 
        tm_p[i][j] = - tm_p[i][j];
  ax_err *= tm_p;
  an_err *= tm_p;
  f2c.Transpose();
  au.Axes()[0].V() = f2c[0].Length();  au.Axes()[0].E() = sqrt(ax_err[0][0]);
  au.Axes()[1].V() = f2c[1].Length();  au.Axes()[1].E() = sqrt(ax_err[1][1]);
  au.Axes()[2].V() = f2c[2].Length();  au.Axes()[2].E() = sqrt(ax_err[2][2]);
  au.Angles()[0].V() = acos(f2c[1].CAngle(f2c[2]))*180.0/M_PI;  au.Angles()[0].E() = sqrt(an_err[0][0]);
  au.Angles()[1].V() = acos(f2c[0].CAngle(f2c[2]))*180.0/M_PI;  au.Angles()[1].E() = sqrt(an_err[1][1]);
  au.Angles()[2].V() = acos(f2c[0].CAngle(f2c[1]))*180.0/M_PI;  au.Angles()[2].E() = sqrt(an_err[2][2]);
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    ca.ccrd() = i_tm * ca.ccrd();
    if( ca.GetEllipsoid() != NULL )  { // reset to usio
      au.NullEllp( ca.GetEllipsoid()->GetId() );
      ca.AssignEllp(NULL);
    }
  }
  au.PackEllps();
  TBasicApp::GetLog() << (olxstr("New cell: ") << au.Axes()[0].ToString() << 
    ' ' << au.Axes()[1].ToString() << 
    ' ' << au.Axes()[2].ToString() <<
    ' '  << au.Angles()[0].ToString() << 
    ' '  << au.Angles()[1].ToString() << 
    ' '  << au.Angles()[2].ToString() << '\n'
    );
  TBasicApp::GetLog().Error("Cell esd's are estimated!");
  olxstr hkl_fn( xapp.LocateHklFile() );
  if( !hkl_fn.IsEmpty() )  {
    THklFile hklf;
    hklf.LoadFromFile(hkl_fn);
    for( size_t i=0; i < hklf.RefCount(); i++ )  {
      vec3d hkl(hklf[i].GetH(), hklf[i].GetK(), hklf[i].GetL());
      hkl = tm_t * hkl;
      hklf[i].SetH(olx_round(hkl[0]));
      hklf[i].SetK(olx_round(hkl[1]));
      hklf[i].SetL(olx_round(hkl[2]));
    }
    olxstr new_hkl_fn( TEFile::ExtractFilePath(hkl_fn) );
    TEFile::AddPathDelimeterI(new_hkl_fn) << "test.hkl";
    hklf.SaveToFile( new_hkl_fn );
    xapp.XFile().GetRM().SetHKLSource(new_hkl_fn);
  }
  au.ChangeSpaceGroup(new_sg);
  au.InitMatrices();
  xapp.XFile().LastLoaderChanged();
  // keep the settings, as the hkl file has changed, so if user exists... inconsistency
  xapp.XFile().SaveToFile( xapp.XFile().GetFileName(), false );
}
//..............................................................................
TSpaceGroup* XLibMacros_macSGS_FindSG(TPtrList<TSpaceGroup>& sgs, const olxstr& axis)  {
  for( size_t i=0; i < sgs.Count(); i++ )
    if( sgs[i]->GetAxis().Compare(axis) == 0 )
      return sgs[i];
  return NULL;
}
olxstr XLibMacros_macSGS_SgInfo(const olxstr& caxis)  {
  if( caxis.IsEmpty() )
    return "standard";
  else  {
    if( caxis.Length() == 3 && caxis.CharAt(0) == '-' )    // -axis + cell choice
      return olxstr("axis: -") << caxis.CharAt(1) << ", cell choice " << caxis.CharAt(2);
    else if( caxis.Length() == 2 )    // axis + cell choice
      return olxstr("axis: ") << caxis.CharAt(0) << ", cell choice " << caxis.CharAt(1);
    else  
      return olxstr("axis: ") << caxis;
  }
}
void XLibMacros::macSGS(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = Cmds.Count() == 1 ? xapp.XFile().GetLastLoaderSG() : *TSymmLib::GetInstance().FindGroup(Cmds[1]);
  SGSettings sg_set(sg);
  olxstr axis = sg_set.axisInfo.GetAxis();
  TBasicApp::GetLog() << (olxstr("Current setting: ") << XLibMacros_macSGS_SgInfo(axis) << '\n');
  if( axis.IsEmpty() )  {
    TBasicApp::GetLog() << "Nothing to do\n";
    return;
  }
  const TSymmLib& sl = TSymmLib::GetInstance();
  TPtrList<TSpaceGroup> sgs;
  sl.GetGroupByNumber(sg.GetNumber(), sgs);
  for( size_t i=0; i < sgs.Count(); i++ )  {
    if( &sg != sgs[i] )
      TBasicApp::GetLog() << (olxstr("Possible: ") << XLibMacros_macSGS_SgInfo(sgs[i]->GetAxis()) << '\n');
  }
  AxisInfo n_ai(sg, Cmds[0]);
  if( sg_set.axisInfo.HasMonoclinicAxis() && !n_ai.HasMonoclinicAxis() )
    n_ai.ChangeMonoclinicAxis(sg_set.axisInfo.GetMonoclinicAxis());
  if( sg_set.axisInfo.HasCellChoice() && !n_ai.HasCellChoice() )
    n_ai.ChangeCellChoice(sg_set.axisInfo.GetCellChoice());
  if( sg_set.axisInfo.GetAxis() == n_ai.GetAxis() )  {
    TBasicApp::GetLog() << "Nothing to change\n";
    return;
  }
  mat3d tm;
  if( sg_set.GetTrasformation(n_ai, tm) )  {
    TSpaceGroup* new_sg = XLibMacros_macSGS_FindSG( sgs, n_ai.GetAxis() );
    if( new_sg == NULL && n_ai.GetAxis() == "abc" )
      new_sg = XLibMacros_macSGS_FindSG( sgs, EmptyString );
    if( new_sg == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "Could not locate space group for given settings");
      return;
    }
    ChangeCell(tm, *new_sg);
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "could not find appropriate transformation");
  }
}
//..............................................................................
void XLibMacros::macLstVar(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( TOlxVars::VarCount() == 0 )  return;
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  TTTable<TStrList> tab(TOlxVars::VarCount(), 3);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Value";
#ifndef _NO_PYTHON_
  tab.ColName(2) = "RefCnt";
#endif
  size_t rowsCount = 0;
  for( size_t i=0; i < TOlxVars::VarCount(); i++ )  {
    bool add = false;
    const olxstr& vn = TOlxVars::GetVarName(i);
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(vn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    tab[rowsCount][0] = vn;
    tab[rowsCount][1] = TOlxVars::GetVarStr(i);
#ifndef _NO_PYTHON
    if( TOlxVars::GetVarWrapper(i) != NULL )
      tab[rowsCount][2] = TOlxVars::GetVarWrapper(i)->ob_refcnt;
    else
      tab[rowsCount][2] = NAString;
#endif
    rowsCount++;
  }
  tab.SetRowCount(rowsCount);
  TStrList Output;
  tab.CreateTXTList(Output, "Variables list", true, true, ' ');
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void XLibMacros::macLstFS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  double tc = 0;
  TTTable<TStrList> tab(TFileHandlerManager::Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Timestamp";
  tab.ColName(3) = "Persistent";
  size_t rowsAdded = 0;
  for( size_t i=0; i < TFileHandlerManager::Count(); i++ )  {
    bool add = false;
    const olxstr& bn = TFileHandlerManager::GetBlockName(i);
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(bn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    tab[rowsAdded][0] = bn;
    tab[rowsAdded][1] = TFileHandlerManager::GetBlockSize(i);
    tab[rowsAdded][2] = TFileHandlerManager::GetBlockDateTime(i);
    tab[rowsAdded][3] = TFileHandlerManager::GetPersistenceId(i);
    tc += TFileHandlerManager::GetBlockSize(i);
    rowsAdded++;
  }
  tc /= (1024*1024);
  tab.SetRowCount(rowsAdded);
  TStrList Output;
  tab.CreateTXTList(Output, olxstr("Virtual FS content"), true, false, "|");
  TBasicApp::GetLog() << Output;
  TBasicApp::GetLog() << (olxstr("Content size is ") << olxstr::FormatFloat(3, tc)  << "Mb\n");
}
//..............................................................................
void XLibMacros::macPlan(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  int plan = Cmds[0].ToInt();
  if( plan == -1 )  return; // leave like it is
  TXApp::GetInstance().XFile().GetRM().SetPlan(plan);
}
//..............................................................................
void XLibMacros::macFixUnit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  double Zp = Cmds.IsEmpty() ? 1 : Cmds[0].ToDouble();
  if( Zp <= 0 )  Zp = 1;
  TXApp::GetInstance().XFile().UpdateAsymmUnit();
  ElementPList content;
  TDoubleList counts;
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  double nhc = 0;
  const cm_Element *cBai = NULL, *hBai = NULL;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    const cm_Element& elm = ca.GetType();
    if( ca.IsDeleted() || elm == iQPeakZ )  continue;
    if( elm.GetMr() > 3.5 )
      nhc += ca.GetOccu();
    size_t ind = content.IndexOf(elm);
    if( ind == InvalidIndex )  {
      content.Add(elm);
      counts.Add(ca.GetOccu());
      if( cBai == NULL && elm == iCarbonZ )    cBai = &elm;
      if( hBai == NULL && elm == iHydrogenZ )  hBai = &elm;
    }
    else
      counts[ind] += ca.GetOccu();
  }
  int Z_est = olx_round(au.EstimateZ(nhc));
  int Z = olx_max(olx_round(Z_est*Zp), 1);
  au.SetZ(Z);
  TBasicApp::GetLog() << (olxstr("for Z'=") << olxstr::FormatFloat(2, Zp).TrimFloat() <<
    " and " << nhc << " non hydrogen atoms Z is estimated to be " << Z << '\n');
  olxstr sfac, unit, n_c;
  content.QuickSorter.SyncSort<ElementPSymbolSorter>(content, counts);
  if( cBai != NULL && content.Count() > 1 )  {
    size_t ind = content.IndexOf(cBai);
    content.Move(ind, 0);
    counts.Move(ind, 0);
  }
  if( hBai != NULL && content.Count() > 2 )  {
    size_t ind = content.IndexOf(hBai);
    content.Move(ind, 1);
    counts.Move(ind, 1);
  }
  ContentList new_content;
  for( size_t i=0; i < content.Count(); i++ )  {
    new_content.AddNew(content[i]->symbol, counts[i]*Z_est);
    n_c << content[i]->symbol << olxstr::FormatFloat(3,counts[i]/Zp).TrimFloat();
    if( (i+1) < content.Count() )
      n_c << ' ';
  }
  TBasicApp::GetLog() << "New content is: " << n_c << '\n';
  TXApp::GetInstance().XFile().GetRM().SetUserContent(new_content);
}
//..............................................................................
void XLibMacros::macGenDisp(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  const ContentList& content = rm.GetUserContent();
  const double en = rm.expl.GetRadiationEnergy();
  for( size_t i=0; i < content.Count(); i++ )  {
    XDispersion* xd = rm.FindDispData(content[i].GetA());
    cm_Element* ce = XElementLib::FindBySymbol(content[i].GetA());
    if( ce == NULL )  continue;
    if( xd == NULL )  {
      compd fpfdp = ce->CalcFpFdp(en) - ce->z;
      rm.AddDisp(content[i].GetA(), fpfdp.GetRe(), fpfdp.GetIm());
    }
    else
      ;//xd->fpfdp =  ce->CalcFpFdp(en);
  }
}
//..............................................................................
void XLibMacros::macEXYZ(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  xapp.FindSAtoms(EmptyString, atoms, false, true);
  if( atoms.Count() != 1 )  {
    E.ProcessingError(__OlxSrcInfo, "please select one atom exactly" );
    return;
  }
  bool set_eadp = Options.Contains("eadp");
  bool link_occu = Options.Contains("lo");
  TExyzGroup* eg;
  ElementPList elms;
  if( atoms[0]->CAtom().GetExyzGroup() != NULL && !atoms[0]->CAtom().GetExyzGroup()->IsEmpty() ) {
    eg = atoms[0]->CAtom().GetExyzGroup();
    elms.Add(atoms[0]->GetType());
  }
  else  {
    eg = &xapp.XFile().GetRM().ExyzGroups.New();
    eg->Add(atoms[0]->CAtom());
  }
  RefinementModel& rm = xapp.XFile().GetRM();
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    cm_Element* elm = XElementLib::FindBySymbol(Cmds[i]);
    if( elm == NULL )  {
      xapp.GetLog().Error(olxstr("Unknown element: ") << Cmds[i]);
      continue;
    }
    if( elms.IndexOf(elm) == InvalidIndex )  {
      TCAtom& ca = xapp.XFile().GetAsymmUnit().NewAtom();
      ca.ccrd() = atoms[0]->CAtom().ccrd();
      ca.SetLabel(elm->symbol + atoms[0]->GetLabel().SubStringFrom( 
        atoms[0]->GetType().symbol.Length()), false);
      ca.SetType(*elm);
      ca.SetDegeneracy(atoms[0]->CAtom().GetDegeneracy());
      rm.Vars.FixParam(ca, catom_var_name_Sof);
      eg->Add(ca);
      ca.SetUiso(atoms[0]->CAtom().GetUiso());
    }
  }
  if( (set_eadp || link_occu) && eg->Count() > 1 )  {
    TSimpleRestraint* sr = set_eadp ? &rm.rEADP.AddNew() : NULL;
    XLEQ* leq = NULL;
    if( link_occu )  {
      if( eg->Count() == 2 )  {
        XVar& vr = rm.Vars.NewVar();
        rm.Vars.AddVarRef(vr, (*eg)[0], catom_var_name_Sof, relation_AsVar, 1.0); 
        rm.Vars.AddVarRef(vr, (*eg)[1], catom_var_name_Sof, relation_AsOneMinusVar, 1.0); 
      }
      else
        leq = &rm.Vars.NewEquation();
    }
    for( size_t i=0; i < eg->Count(); i++ )  {
      if( (*eg)[i].IsDeleted() )  continue;
      if( leq != NULL )  {
        XVar& vr = rm.Vars.NewVar( 1./eg->Count() );
        rm.Vars.AddVarRef(vr, (*eg)[i], catom_var_name_Sof, relation_AsVar, 1.0); 
        leq->AddMember(vr);
      }
      if( sr != NULL )
        sr->AddAtom((*eg)[i], NULL);
    }
  }
  // force the split atom to become isotropic
  TCAtomPList processed;
  processed.Add( &atoms[0]->CAtom() );
  xapp.XFile().GetLattice().SetAnis(processed, false);
}
//..............................................................................
void XLibMacros::macEADP(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  xapp.FindSAtoms(Cmds.Text(' '), atoms, false, true);
  if( atoms.Count() < 2 )  {
    E.ProcessingError(__OlxSrcInfo, "not enough atoms provided" );
    return;
  }
  // validate that atoms of the same type
  bool allIso = (atoms[0]->GetEllipsoid() == NULL);
  for( size_t i=1; i < atoms.Count(); i++ )  {
    if( (atoms[i]->GetEllipsoid() == NULL) != allIso )  {
      E.ProcessingError(__OlxSrcInfo, "mixed atoms types (aniso and iso)" );
      return;
    }
  }
  TSimpleRestraint& sr = xapp.XFile().GetRM().rEADP.AddNew();
  for( size_t i=0; i < atoms.Count(); i++ )
    sr.AddAtom(atoms[i]->CAtom(), NULL);
  xapp.XFile().GetRM().rEADP.ValidateRestraint(sr);
}
//..............................................................................
void XLibMacros::macAddSE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify current space group");
    return;
  }
  if( sg->IsCentrosymmetric() && Cmds.Count() == 1 )  {
    E.ProcessingError(__OlxSrcInfo, "Centrosymmetric space group");
    return;
  }
  if( Cmds.Count() == 1 )  {
    smatd_list ml;
    sg->GetMatrices(ml, mattAll);
    ml.SetCapacity( ml.Count()*2 );
    const size_t mc = ml.Count();
    for( size_t i=0; i < mc; i++ )  {
      ml.AddCCopy(ml[i]);
      ml[i+mc] *= -1;
    }
    for( size_t i=0; i < TSymmLib::GetInstance().SGCount(); i++ )  {
      if( TSymmLib::GetInstance().GetGroup(i) == ml )  {
        xapp.GetLog() << "found " << TSymmLib::GetInstance().GetGroup(i).GetName() << '\n';
        sg = &TSymmLib::GetInstance().GetGroup(i);
      }
    }
  }
  else if( Cmds.Count() == 2 )  {
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString, atoms);
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      for( size_t j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
    latt.Init();
    latt.Compaq();
    latt.CompaqAll();
    return;
  }
  TSymmTest st(uc);
  smatd m;
  m.r.I();
  double tol = 0.1;
  st.TestMatrix(m, tol);
  if( !st.GetResults().IsEmpty() )  {
    size_t ind = st.GetResults().Count()-1;
    double match = (double)(st.GetResults()[ind].Count()*200/st.AtomCount());
    while( match > 125 && tol > 1e-4 )  {
      tol /= 4;
      st.TestMatrix(m, tol);
      ind = st.GetResults().Count()-1;
      match = st.GetResults().IsEmpty() ? 0.0 :st.GetResults()[ind].Count()*200/st.AtomCount();
      continue;
    }
    if( st.GetResults().IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "ooops...");
      return;
    }
    TEStrBuffer out;
    vec3d trans( st.GetResults()[ind].Center );
    //TVectorD trans = st.GetGravityCenter();
    trans /= 2;
    trans *= -1;
    m.t = trans;
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString, atoms);
    xapp.XFile().GetLattice().TransformFragments(atoms, m);
    au.ChangeSpaceGroup(*sg);
    xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
    latt.Init();
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      for( size_t j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
//    latt.OnStructureGrow->SetEnabled(false);
//    latt.OnStructureUniq->SetEnabled(false);
    latt.Init();
    latt.Compaq();
//    latt.OnStructureGrow->SetEnabled(true);
//    latt.OnStructureUniq->SetEnabled(true);
    latt.CompaqAll();
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "could not find interatomic relations");
  }
}
//..............................................................................
void XLibMacros::macCompaq(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Options.Contains('a') )  
    TXApp::GetInstance().XFile().GetLattice().CompaqAll();
  else if( Options.Contains('c') )  
    TXApp::GetInstance().XFile().GetLattice().CompaqClosest();
  else if( Options.Contains('q') )  
    TXApp::GetInstance().XFile().GetLattice().CompaqQ();
  else
    TXApp::GetInstance().XFile().GetLattice().Compaq();
}
//..............................................................................
void XLibMacros::macEnvi(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  double r = 2.7;
  ParseNumbers<double,TStrObjList>(Cmds, 1, &r);
  if( r < 1 || r > 10 )  {
    E.ProcessingError(__OlxSrcInfo, "radius must be within [1;10] range" );
    return;
  }
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms, false, false) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  TStrList output;
  ElementPList Exceptions;
  Exceptions.Add(XElementLib::GetByIndex(iQPeakIndex));
  Exceptions.Add(XElementLib::GetByIndex(iHydrogenIndex));
  if( Options.Contains('q') )
    Exceptions.Remove(XElementLib::GetByIndex(iQPeakIndex));
  if( Options.Contains('h') )
    Exceptions.Remove(XElementLib::GetByIndex(iHydrogenIndex));

  TSAtom& SA = *atoms[0];
  TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  vec3d V;
  smatd_list* L;
  TArrayList< AnAssociation3<TCAtom*, vec3d, smatd> > rowData;
  TCAtomPList allAtoms;

  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    bool skip = false;
    for( size_t j=0; j < Exceptions.Count(); j++ )  {
      if( au.GetAtom(i).GetType() == *Exceptions[j] )
      {  skip = true;  break;  }
    }
    if( !skip )  allAtoms.Add( &au.GetAtom(i) );
  }
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    if( SA.CAtom().GetId() == allAtoms[i]->GetId() )
      L = latt.GetUnitCell().GetInRange(SA.ccrd(), allAtoms[i]->ccrd(), r, false);
    else
      L = latt.GetUnitCell().GetInRange(SA.ccrd(), allAtoms[i]->ccrd(), r, true);
    if( !L->IsEmpty() )  {
      for( size_t j=0; j < L->Count(); j++ )  {
        const smatd& m = L->Item(j);
        V = m * allAtoms[i]->ccrd() - SA.ccrd();
        au.CellToCartesian(V);
        if( V.Length() == 0 )  // symmetrical equivalent?
          continue;
        rowData.Add( AnAssociation3<TCAtom*, vec3d, smatd>(allAtoms[i], V, m) );
      }
    }
    delete L;
  }
  TTTable<TStrList> table(rowData.Count(), rowData.Count()+2); // +SYM + LEN
  table.ColName(0) = SA.GetLabel();
  table.ColName(1) = "SYMM";
  rowData.BubleSorter.Sort<XLibMacros::TEnviComparator>(rowData);
  for( size_t i=0; i < rowData.Count(); i++ )  {
    const AnAssociation3<TCAtom*, vec3d, smatd>& rd = rowData[i];
    table.RowName(i) = rd.GetA()->GetLabel();
    table.ColName(i+2) = table.RowName(i);
    if( rd.GetC().r.IsI() && rd.GetC().t.IsNull() )
     table[i][1] = 'I';  // identity
    else
      table[i][1] = TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell(), rd.GetC() );
    table[i][0] = olxstr::FormatFloat(2, rd.GetB().Length());
    for( size_t j=0; j < rowData.Count(); j++ )  {
      if( i == j )  { table[i][j+2] = '-'; continue; }
      if( i < j )   { table[i][j+2] = '-'; continue; }
      const AnAssociation3<TCAtom*, vec3d, smatd>& rd1 = rowData[j];
      if( rd.GetB().Length() != 0 && rd1.GetB().Length() != 0 )  {
        double angle = rd.GetB().CAngle(rd1.GetB());
        angle = acos(angle)*180/M_PI;
        table[i][j+2] = olxstr::FormatFloat(1, angle);
      }
      else
        table[i][j+2] = '-';
    }
  }
  table.CreateTXTList(output, EmptyString, true, true, ' ');
  TBasicApp::GetLog() << output << '\n';
  //TBasicApp::GetLog() << ("----\n");
  //if( !latt.IsGenerated() )
  //  TBasicApp::GetLog() << ("Use move \"atom_to atom_to_move\" command to move the atom/fragment\n");
  //else
  //  TBasicApp::GetLog() << ("Use move \"atom_to atom_to_move -c\" command to move the atom/fragment\n");
}
//..............................................................................
void XLibMacros::funRemoveSE(const TStrObjList &Params, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not identify current space group");
    return;
  }
  if( Params[0] == "-1" )  {
    if( !sg->IsCentrosymmetric() )  {
      E.SetRetVal( sg->GetName() );
      return;
    }
    smatd_list ml;
    sg->GetMatrices(ml, mattAll^mattInversion);
    TPSTypeList<double, TSpaceGroup*> sglist;
    for( size_t i=0; i < TSymmLib::GetInstance().SGCount(); i++ )  {
      double st=0;
      if( TSymmLib::GetInstance().GetGroup(i).Compare(ml, st) )
        sglist.Add(st, &TSymmLib::GetInstance().GetGroup(i) );
    }
    E.SetRetVal( sglist.IsEmpty() ? sg->GetName() : sglist.GetObject(0)->GetName() );
  }
}
//..............................................................................
void XLibMacros::funFileName(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFileName(Params[0]);
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFileName( TXApp::GetInstance().XFile().GetFileName() );
    else
      Tmp = NoneString;
  }
  E.SetRetVal(TEFile::ChangeFileExt(Tmp, EmptyString));
}
//..............................................................................
void XLibMacros::funFileExt(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal( TEFile::ExtractFileExt(Params[0]) );
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      E.SetRetVal( TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()) );
    else
      E.SetRetVal(NoneString);
  }
}
//..............................................................................
void XLibMacros::funFilePath(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFilePath( Params[0] );
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFilePath( TXApp::GetInstance().XFile().GetFileName() );
    else
      Tmp = NoneString;
  }
  // see notes in funBaseDir
  TEFile::TrimPathDelimeterI(Tmp);
  E.SetRetVal(Tmp);
}
//..............................................................................
void XLibMacros::funFileDrive(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal( TEFile::ExtractFileDrive(Params[0]) );
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      E.SetRetVal( TEFile::ExtractFileDrive(TXApp::GetInstance().XFile().GetFileName()) );
    else
      E.SetRetVal(NoneString);
  }
}
//..............................................................................
void XLibMacros::funFileFull(const TStrObjList &Params, TMacroError &E)  {
  if( TXApp::GetInstance().XFile().HasLastLoader() )
    E.SetRetVal( TXApp::GetInstance().XFile().GetFileName() );
  else
    E.SetRetVal(NoneString);
}
//..............................................................................
void XLibMacros::funIsFileLoaded(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( TXApp::GetInstance().XFile().HasLastLoader() );
}
//..............................................................................
void XLibMacros::funTitle(const TStrObjList& Params, TMacroError &E)  {
  if( !TXApp::GetInstance().XFile().HasLastLoader() )  {
    if( Params.IsEmpty() )
      E.SetRetVal(olxstr("File is not loaded"));
    else
      E.SetRetVal(Params[0]);
  }
  else
    E.SetRetVal( TXApp::GetInstance().XFile().LastLoader()->GetTitle() );
}
//..............................................................................
void XLibMacros::funIsFileType(const TStrObjList& Params, TMacroError &E) {
  if( Params[0].Equalsi("ins") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Equalsi("ins") );
  }
  else if( Params[0].Equalsi("res") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Equalsi("res")  );
  }
  else if( Params[0].Equalsi("ires") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() );
  }
  else if( Params[0].Equalsi("cif") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TCif>() );
  }
  else if( Params[0].Equalsi("p4p") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TP4PFile>() );
  }
  else if( Params[0].Equalsi("mol") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TMol>() );
  }
  else if( Params[0].Equalsi("xyz") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TMol>() );
  }
  else if( Params[0].Equalsi("crs") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TCRSFile>() );
  }
  else
    E.SetRetVal( false );
}
//..............................................................................
void XLibMacros::funBaseDir(const TStrObjList& Params, TMacroError &E)  {
  olxstr tmp( TBasicApp::GetBaseDir() );
  // remove the trailing backslash, as it causes a lot of problems with
  // passing parameters to other programs:
  // windows parser assumes that \" is " and does wrong parsing...
  if( !tmp.IsEmpty() )  tmp.SetLength( tmp.Length()-1 );
  E.SetRetVal( tmp );
}
//..............................................................................
void XLibMacros::funLSM(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( TXApp::GetInstance().XFile().GetRM().GetRefinementMethod() );
}
//..............................................................................
void XLibMacros::funRun(const TStrObjList& Params, TMacroError &E) {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "this function requires Olex2 processor implementation");
  TStrList allCmds(Params.Text(' '), ">>");
  for( size_t i=0; i < allCmds.Count(); i++ )  {
    op->executeMacroEx(allCmds[i], E);
    if( !E.IsSuccessful() )  {
      if( (i+1) < allCmds.Count() )
        op->print("Not all macros in the provided list were executed", olex::mtError);
      break;
    }
  }
  //E.SetRetVal(E.IsSuccessful());
  // we do not care about result, but nothing should be printed on the html...
  E.SetRetVal(EmptyString);
}
//..............................................................................
void XLibMacros::funIns(const TStrObjList& Params, TMacroError &E)  {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  olxstr tmp;
  if( Params[0].Equalsi("weight") || Params[0].Equalsi("wght") )  {
    for( size_t j=0; j < rm.used_weight.Count(); j++ )  {
      tmp << rm.used_weight[j];
      if( (j+1) < rm.used_weight.Count() )  tmp << ' ';
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString : tmp);
  }
  else if( Params[0].Equalsi("weight1") )  {
    for( size_t j=0; j < rm.proposed_weight.Count(); j++ )  {
      tmp << rm.proposed_weight[j];
      if( (j+1) < rm.proposed_weight.Count() )  tmp << ' ';
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString : tmp);
  }
  else if( Params[0].Equalsi("L.S.") || Params[0].Equalsi("CGLS")  )  {
    for( size_t i=0; i < rm.LS.Count(); i++ )  {
      tmp << rm.LS[i];
      if( (i+1) < rm.LS.Count() )  tmp << ' ';
    }
    E.SetRetVal(rm.LS.Count() == 0 ? NAString : tmp);
  }
  else if( Params[0].Equalsi("ls") )
    E.SetRetVal(rm.LS.Count() == 0 ? NAString : olxstr(rm.LS[0]));
  else if( Params[0].Equalsi("plan") )  {
    for( size_t i=0; i < rm.PLAN.Count(); i++ )  {
      tmp << ((i < 1) ? olx_round(rm.PLAN[i]) : rm.PLAN[i]);
      if( (i+1) < rm.PLAN.Count() )  tmp << ' ';
    }
    E.SetRetVal(rm.PLAN.Count() == 0 ? NAString : tmp);
  }
  else if( Params[0].Equalsi("qnum") )
    E.SetRetVal(rm.PLAN.Count() == 0 ? NAString : olxstr(rm.PLAN[0]));
  else  {
    TIns& I = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
    if( Params[0].Equalsi("R1") )
      E.SetRetVal(I.GetR1() < 0 ? NAString : olxstr(I.GetR1()));
    if( !I.InsExists(Params[0]) )  {
      E.SetRetVal(NAString);
      return;
    }
    //  xapp.XFile().UpdateAsymmUnit();
    //  I->UpdateParams();

    TInsList* insv = I.FindIns(Params[0]);
    if( insv != 0 )
      E.SetRetVal(insv->Text(' '));
    else
      E.SetRetVal(EmptyString);
  }
}
//..............................................................................
void XLibMacros::funSSM(const TStrObjList& Params, TMacroError &E) {
  RefinementModel& rm  = TXApp::GetInstance().XFile().GetRM();
  if( rm.GetSolutionMethod().IsEmpty() && Params.Count() == 1 )
    E.SetRetVal( Params[0] );
  else
    E.SetRetVal(rm.GetSolutionMethod());
}
//..............................................................................
bool XLibMacros_funSGNameIsNextSub(const olxstr& name, size_t i)  {
  if( (i+1) < name.Length() )  {
    if( olxstr::o_isdigit(name[i])  &&  olxstr::o_isdigit(name[i+1]) )  {
      if( name[i] != '1' && name[i] > name[i+1] )
        return true;
    }
  }
  return false;
}
olxstr XLibMacros_funSGNameToHtml(const olxstr& name)  {
  olxstr res;
  res.SetCapacity( name.Length() + 20 );
  for( size_t i=0; i < name.Length(); i++ )  {
    if( XLibMacros_funSGNameIsNextSub(name, i+1) )  {
      res << name[i];
      continue;
    }
    if( XLibMacros_funSGNameIsNextSub(name, i) )  {
      res << name[i] << "<sub>" << name[i+1] << "</sub>";
      i++;
      continue;
    }
    res << name[i];
  }
  return res;
}
olxstr XLibMacros_funSGNameToHtmlX(const olxstr& name)  {
  TStrList toks(name, ' ');
  olxstr res;
  for( size_t i=0; i < toks.Count(); i++ )  {
    if( toks[i].Length() == 2 )
      res << toks[i].CharAt(0) << "<sub>" << toks[i].CharAt(1) << "</sub>";
    else
      res << toks[i];
  }
  return res;
}
void XLibMacros::funSG(const TStrObjList &Cmds, TMacroError &E)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &TXApp::GetInstance().XFile().GetLastLoaderSG();  }
  catch(...)  {}
  if( sg != NULL )  {
    olxstr Tmp;
    if( Cmds.IsEmpty() )  {
      Tmp = sg->GetName();
      if( !sg->GetFullName().IsEmpty() )  {
        Tmp << " (" << sg->GetFullName() << ')';
      }
      Tmp << " #" << sg->GetNumber();
    }
    else  {
      Tmp = Cmds[0];
      Tmp.Replace("%#", olxstr(sg->GetNumber()) ).\
        Replace("%n", sg->GetName()).\
        Replace("%N", sg->GetFullName()).\
        Replace("%HS", sg->GetHallSymbol()).\
        Replace("%s", sg->GetBravaisLattice().GetName());
      if( Tmp.IndexOf("%H") != InvalidIndex )
        Tmp.Replace("%H", XLibMacros_funSGNameToHtmlX(sg->GetFullName()));
      if( Tmp.IndexOf("%h") != InvalidIndex )
        Tmp.Replace("%h", XLibMacros_funSGNameToHtml(sg->GetName()));
    }
    E.SetRetVal( Tmp );
  }
  else  {
    E.SetRetVal( NAString );
//    E.ProcessingError(__OlxSrcInfo, "could not find space group for the file" );
    return;
  }
}
//..............................................................................
void XLibMacros::funSGS(const TStrObjList &Cmds, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  const olxstr& axis =  sg.GetAxis();
  if( axis.IsEmpty() )
    E.SetRetVal<olxstr>( "standard" );
  else  {
    if( axis.Length() == 2 )  {  // axis + cell choice
      E.SetRetVal(olxstr(axis.CharAt(0)) << ": cell choice " << axis.CharAt(1));
    }
    else  {
      E.SetRetVal(olxstr("axis: ") << axis);
    }
  }
}
//..............................................................................
void XLibMacros::funHKLSrc(const TStrObjList& Params, TMacroError &E)  {
  if( Params.Count() == 1 )
    TXApp::GetInstance().XFile().GetRM().SetHKLSource( Params[0] );
  else
    E.SetRetVal( TXApp::GetInstance().XFile().GetRM().GetHKLSource() );
}
//..............................................................................
void XLibMacros::macCif2Doc(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifDictionaryFile( xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    TStrList Output;
    olxstr CDir = TEFile::CurrentDir();
    TEFile::ChangeDir( xapp.GetCifTemplatesDir() );
    TEFile::ListCurrentDir(Output, "*.rtf;*html;*.htm", sefFile);
    TEFile::ChangeDir(CDir);
    xapp.GetLog() << "Templates found: \n";
    xapp.GetLog() << Output << '\n';
    return;
  }

  olxstr TN = Cmds[0];
  if( !TEFile::Exists(TN) )
    TN = xapp.GetCifTemplatesDir() + TN;
  if( !TEFile::Exists(TN) )  {
    Error.ProcessingError(__OlxSrcInfo, "template for CIF does not exist: ") << Cmds[0];
    return;
  }
  // resolvind the index file
  if( !TEFile::Exists(CifDictionaryFile) )  {
    Error.ProcessingError(__OlxSrcInfo, "CIF dictionary does not exist" );
    return;
  }

  TCif *Cif, Cif1;
  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists( cifFN ) ) 
      Cif1.LoadFromFile( cifFN );
    else  {
      Error.ProcessingError(__OlxSrcInfo, "existing cif is expected");
      return;
    }
    Cif = &Cif1;
  }

  TStrList SL, Dic;
  olxstr RF( Options.FindValue("n", EmptyString) );
  if( RF.IsEmpty() )  {
    RF = TEFile::ChangeFileExt(Cif->GetFileName(), EmptyString);
    RF << "_doc";
  }
  RF = TEFile::ChangeFileExt(RF, TEFile::ExtractFileExt(TN));

  SL.LoadFromFile( TN );
  Dic.LoadFromFile( CifDictionaryFile );
  for( size_t i=0; i < SL.Count(); i++ )
    Cif->ResolveParamsFromDictionary(Dic, SL[i], '%', &XLibMacros::CifResolve);
  TUtf8File::WriteLines( RF, SL, false );
  TBasicApp::GetLog().Info(olxstr("Document name: ") << RF);
}
//..............................................................................
void XLibMacros::macCif2Tab(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifTablesFile( xapp.GetCifTemplatesDir() + "tables.xlt");
  olxstr CifDictionaryFile( xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    if( !TEFile::Exists(CifTablesFile) )  {
      Error.ProcessingError(__OlxSrcInfo, "tables definition file is not found" );
      return;
    }

    TDataFile DF;
    TStrList SL;
    TDataItem *Root;
    olxstr Tmp;
    DF.LoadFromXLFile(CifTablesFile, &SL);

    Root = DF.Root().FindItemi("Cif_Tables");
    if( Root != NULL )  {
      xapp.GetLog().Info("Found table definitions:");
      for( size_t i=0; i < Root->ItemCount(); i++ )  {
        Tmp = "Table "; 
        Tmp << Root->GetItem(i).GetName()  << "(" << " #" << (int)i+1 <<  "): caption <---";
        xapp.GetLog().Info(Tmp);
        xapp.GetLog().Info(Root->GetItem(i).GetFieldValueCI("caption"));
        xapp.GetLog().Info("--->");
      }
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo, "tables definition node is not found" );
      return;
    }
    return;
  }
  TCif *Cif, Cif1;

  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists( cifFN ) )  {
      Cif1.LoadFromFile( cifFN );
    }
    else
        throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }

  TStrList SL, SL1, Dic, 
    CLA, // cell attributes
    THA;  // header (th) attributes
  TDataFile DF;
  TDataItem *TD, *Root;
  TTTable<TStrList> DT;
  DF.LoadFromXLFile(CifTablesFile, NULL);
  Dic.LoadFromFile( CifDictionaryFile );

  olxstr RF( Options.FindValue("n", EmptyString) ), 
         Tmp;
  if( RF.IsEmpty() )  {
    RF = TEFile::ChangeFileExt(Cif->GetFileName(), EmptyString);
    RF << "_tables";
  }
  RF = TEFile::ChangeFileExt(RF, "html");

  Root = DF.Root().FindItemi("Cif_Tables");
  smatd_list SymmList;
  size_t tab_count = 1;
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    TD = NULL;
    if( Cmds[i].IsNumber() )  {
      size_t index = Cmds[i].ToSizeT();
      if( index < Root->ItemCount() )
        TD = &Root->GetItem(index);
    }
    if( TD == NULL  )
      TD = Root->FindItem(Cmds[i]);
    if( TD == NULL )  {
      xapp.GetLog().Warning( olxstr("Could not find table definition: ") << Cmds[i] );
      continue;
    }
    if( TD->GetName().Equalsi("footer") || TD->GetName().Equalsi("header") )  {
      olxstr fn = TD->GetFieldValue("source");
      if( fn.IndexOf("$") != InvalidIndex )
        ProcessExternalFunction(fn);
      if( !TEFile::IsAbsolutePath(fn) )
        fn = xapp.GetCifTemplatesDir() + fn;
      SL1.LoadFromFile( fn );
      for( size_t j=0; j < SL1.Count(); j++ )  {
        Cif->ResolveParamsFromDictionary(Dic, SL1[j], '%', &XLibMacros::CifResolve);
        SL.Add( SL1[j] );
      }
      continue;
    }
    if( Cif->CreateTable(TD, DT, SymmList) )  {
      Tmp = "Table "; Tmp << ++tab_count << ' ' << TD->GetFieldValueCI("caption");
      Tmp.Replace("%DATA_NAME%", Cif->GetDataName());
      if( Tmp.IndexOf("$") != InvalidIndex )
        ProcessExternalFunction( Tmp );
      // attributes of the row names ...
      CLA.Clear();
      THA.Clear();
      CLA.Add( TD->GetFieldValue("tha", EmptyString) );
      THA.Add( TD->GetFieldValue("tha", EmptyString) );
      for( size_t j=0; j < TD->ItemCount(); j++ )  {
        CLA.Add( TD->GetItem(j).GetFieldValue("cola", EmptyString) );
        THA.Add( TD->GetItem(j).GetFieldValue("tha", EmptyString) );
      }

      olxstr footer;
      for( size_t i=0; i < SymmList.Count(); i++ )  {
        footer << "<sup>" << (i+1) << "</sup>" <<
           TSymmParser::MatrixToSymmEx(SymmList[i]);
        if( (i+1) < SymmList.Count() )
          footer << "; ";
      }


      DT.CreateHTMLList(SL, Tmp,
                      footer,
                      true, false,
                      TD->GetFieldValue("tita", EmptyString),  // title paragraph attributes
                      TD->GetFieldValue("foota", EmptyString),  // footer paragraph attributes
                      TD->GetFieldValue("taba", EmptyString),  //const olxstr& tabAttr,
                      TD->GetFieldValue("rowa", EmptyString),  //const olxstr& rowAttr,
                      THA, // header attributes
                      CLA, // cell attributes,
                      true,
                      TD->GetFieldValue("coln", "1").ToInt()
                      ); //bool Format) const  {

      //DT.CreateHTMLList(SL, Tmp, true, false, true);
    }
  }
  TUtf8File::WriteLines(RF, SL, false);
  TBasicApp::GetLog().Info(olxstr("Tables file: ") << RF);
}
//..............................................................................
olxstr XLibMacros::CifResolve(const olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return func;
  olxstr rv;
  if( op->executeFunction(func, rv) )
    return rv;
  return func;
}
//..............................................................................
bool XLibMacros::ProcessExternalFunction(olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return false;
  olxstr rv;
  if( op->executeFunction(func, rv) )  {
    func = rv;
    return true;
  }
  return false;
}
//..............................................................................
void XLibMacros::MergePublTableData(TCifLoopTable& to, TCifLoopTable& from)  {
  if( from.RowCount() == 0 )  return;
  static const olxstr authorNameCN("_publ_author_name");
  // create a list of unique colums, and prepeare them for indexed access
  TSStrPObjList<olxstr, AnAssociation2<size_t,size_t>, false> uniqCols;
  for( size_t i=0; i < from.ColCount(); i++ )  {
    if( uniqCols.IndexOfComparable(from.ColName(i)) == InvalidIndex )  {
      uniqCols.Add(from.ColName(i), AnAssociation2<size_t,size_t>(i, InvalidIndex) );
    }
  }
  for( size_t i=0; i < to.ColCount(); i++ )  {
    size_t ind = uniqCols.IndexOfComparable( to.ColName(i) );
    if( ind == InvalidIndex )
      uniqCols.Add( to.ColName(i), AnAssociation2<size_t,size_t>(InvalidIndex, i) );
    else
      uniqCols.GetObject(ind).B() = i;
  }
  // add new columns, if any
  for( size_t i=0; i < uniqCols.Count(); i++ ) {
    if( uniqCols.GetObject(i).GetB() == InvalidIndex )  {
      to.AddCol( uniqCols.GetComparable(i) );
      uniqCols.GetObject(i).B() = to.ColCount() - 1;
    }
  }
  /* by this point the uniqCols contains all the column names and the association
  holds corresponding column indexes in from and to tables */
  // the actual merge, by author name
  size_t authNCI = uniqCols.IndexOfComparable( authorNameCN );
  if( authNCI == InvalidIndex )  return;  // cannot do much, can we?
  AnAssociation2<size_t,size_t> authCA(uniqCols.GetObject(authNCI));
  if( authCA.GetA() == InvalidIndex )  return;  // no author?, bad ...
  for( size_t i=0; i < from.RowCount(); i++ )  {
    size_t ri = InvalidIndex;
    for( size_t j=0; j < to.RowCount(); j++ )  {
      if( to[j][ authCA.GetB() ].Equalsi(from[i][authCA.GetA()]) )  {
        ri = j;
        break;
      }
    }
    if( ri == InvalidIndex )  {  // add a new row
      to.AddRow( EmptyString );
      ri = to.RowCount()-1;
    }
    for( size_t j=0; j < uniqCols.Count(); j++ )  {
      AnAssociation2<size_t,size_t>& as = uniqCols.GetObject(j);
      if( as.GetA() == InvalidIndex )  continue;
      to[ri][as.GetB()] = from[i][ as.GetA() ];
    }
  }
  // null the objects - they must not be here anyway ..
  for( size_t i=0; i < to.RowCount(); i++ )  {
    for( size_t j=0; j < to.ColCount(); j++ )  {
      if( to[i].GetObject(j) == NULL )
        to[i].GetObject(j) = new TCifLoopData(true);
      to[i].GetObject(j)->String = true;
    }
  }
}
//..............................................................................
void XLibMacros::macCifMerge(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TCif *Cif, Cif1, Cif2;

  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists( cifFN ) )  {
      Cif2.LoadFromFile( cifFN );
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif2;
  }

  TCifLoop& publ_info = Cif->GetPublicationInfoLoop();

  for( size_t i=0; i < Cmds.Count(); i++ )  {
    try {
      IInputStream *is = TFileHandlerManager::GetInputStream(Cmds[i]);
      if( is == NULL )  {
        TBasicApp::GetLog().Error(olxstr("Could not find file: ") << Cmds[i]);
        continue;
      }
      TStrList sl;
      sl.LoadFromTextStream(*is);
      delete is;
      Cif1.LoadFromStrings(sl);
    }
    catch( ... )  {    }  // most like the cif does not have cell, so pass it
    TCifLoop& pil = Cif1.GetPublicationInfoLoop();
    for( size_t j=0; j < Cif1.ParamCount(); j++ )  {
      if( !Cif->ParamExists(Cif1.Param(j)) )
        Cif->AddParam(Cif1.Param(j), Cif1.ParamValue(j));
      else
        Cif->SetParam(Cif1.Param(j), Cif1.ParamValue(j));
    }
    // update publication info loop
    MergePublTableData(publ_info.GetTable(), pil.GetTable());
  }
  // generate moiety string if does not exist
  if( !Cif->ParamExists("_chemical_formula_moiety") )
    Cif->AddParam("_chemical_formula_moiety", xapp.XFile().GetLattice().CalcMoiety(), true);
  TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(Cif->GetAsymmUnit());
  if( sg != NULL )  {
    if( !Cif->ParamExists("_symmetry_cell_setting") )
      Cif->AddParam("_symmetry_cell_setting", sg->GetBravaisLattice().GetName(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_cell_setting");
      if( cd->Data->IsEmpty() )
        cd->Data->Add(sg->GetBravaisLattice().GetName());
      else
        cd->Data->GetString(0) = sg->GetBravaisLattice().GetName();
      cd->String = true;
    }
    if( !Cif->ParamExists("_symmetry_space_group_name_H-M") )
      Cif->AddParam("_symmetry_space_group_name_H-M", sg->GetFullName(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_space_group_name_H-M");
      if( cd->Data->IsEmpty() )
        cd->Data->Add(sg->GetFullName());
      else
        cd->Data->GetString(0) = sg->GetFullName();
      cd->String = true;
    }
    if( !Cif->ParamExists("_symmetry_space_group_name_Hall") )
      Cif->AddParam("_symmetry_space_group_name_Hall", sg->GetHallSymbol(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_space_group_name_Hall");
      if( cd->Data->IsEmpty() )
        cd->Data->Add(sg->GetHallSymbol());
      else
        cd->Data->GetString(0) = sg->GetHallSymbol();
      cd->String = true;
    }
  }
  else
    TBasicApp::GetLog().Error("Could not locate space group ...");
  Cif->Group();
  Cif->SaveToFile( Cif->GetFileName() );
}
//..............................................................................
void XLibMacros::macCifExtract(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr Dictionary = Cmds[0];
  if( !TEFile::Exists(Dictionary) )  {  // check if the dictionary exists
    Dictionary = xapp.GetCifTemplatesDir();  Dictionary << Cmds[0];
    if( !TEFile::Exists(Dictionary) )  {
      Error.ProcessingError(__OlxSrcInfo, "dictionary file does not exists" );
      return;
    }
  }
  TCif In,  Out, *Cif, Cif1;
  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists( cifFN ) )  {
      Cif1.LoadFromFile( cifFN );
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }
  // dictionary does not have cell etc - so it should fail to initialise
  try  {  In.LoadFromFile(Dictionary);  }
  catch( TExceptionBase& )  {}

  for( size_t i=0; i < In.ParamCount(); i++ )  {
    TCifData* CifData = Cif->FindParam(In.Param(i));
    if( CifData != NULL )
      Out.AddParam(In.Param(i), CifData);
  }
  try  {  Out.SaveToFile(Cmds[1]);  }
  catch( TExceptionBase& )  {
    Error.ProcessingError(__OlxSrcInfo, "could not save file: ") << Cmds[1];
    return;
  }
}
//..............................................................................
void XLibMacros::macCifCreate(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  VcoVContainer vcovc;
  vcovc.ReadShelxMat(TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "mat"), xapp.XFile().GetAsymmUnit());
  TAsymmUnit& _au = xapp.XFile().GetAsymmUnit();
  for( size_t i=0; i < _au.AtomCount(); i++ )  {
    TCAtom& a = _au.GetAtom(i);
    if( a.GetEllipsoid() != NULL )  {
      TEllipsoid& elp = *a.GetEllipsoid();
      a.SetUiso(elp.GetUiso());
      double esd = 0;
      for( int j=0; j < 3; j++ )
        esd += olx_sqr(elp.GetEsd(j));
      a.SetUisoEsd(sqrt(esd)/4.);
    }
    else if( a.GetType() == iHydrogenZ )  {
      long val = olx_round(a.GetUiso()*1000);
      a.SetUiso((double)val/1000);
    }
  }
  TCif cif;
  cif.Adopt(xapp.XFile());
  TAsymmUnit& au = cif.GetAsymmUnit();
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetType() == iQPeakZ )
      au.GetAtom(i).SetDeleted(true);
  }
  TLattice& latt = xapp.XFile().GetLattice();
  TCifLoop& bonds = cif.AddLoop("_geom_bond");
  bonds.GetTable().AddCol("_geom_bond_atom_site_label_1");
  bonds.GetTable().AddCol("_geom_bond_atom_site_label_2");
  bonds.GetTable().AddCol("_geom_bond_distance");
  bonds.GetTable().AddCol("_geom_bond_site_symmetry_2");
  bonds.GetTable().AddCol("_geom_bond_publ_flag");
  for( size_t i=0; i < latt.BondCount(); i++ )  {
    TSBond& b = latt.GetBond(i);
    if( b.A().GetType().GetMr() < 3 || b.A().IsDeleted() )  {
      b.SetTag(0);
      continue;
    }
    if( b.B().GetType().GetMr() < 3 || b.B().IsDeleted() )  {
      b.SetTag(0);
      continue;
    }
    b.SetTag(-1);
  }
  for( size_t i=0; i < latt.AtomCount(); i++ )  {
    TSAtom& a = latt.GetAtom(i);
    if( a.GetType().GetMr()  < 3 || a.IsDeleted() || !a.GetMatrix(0).IsFirst() )  continue;
    for( size_t j=0; j < a.BondCount(); j++ )  {
      TSBond& b = a.Bond(j);
      if( b.GetTag() == 0 )  continue;
      b.SetTag(0);
      TCifRow& row = bonds.GetTable().AddRow(EmptyString);
      row[0] = b.A().GetLabel();  row.GetObject(0) = new TCifLoopData(&b.A().CAtom());
      row[1] = b.B().GetLabel();  row.GetObject(1) = new TCifLoopData(&b.B().CAtom());
      row[2] = vcovc.CalcDistance(b.A(), b.B()).ToString();
      if( !b.B().GetMatrix(0).IsFirst() )
        row[3] = TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell(), b.B().GetMatrix(0));
      else
        row[3] = '.';
      row[4] = '?';
      for( int k=2; k < 5; k++ )
        row.GetObject(k) = new TCifLoopData;
    }
  }
  TCifLoop& angles = cif.AddLoop("_geom_angle");
  angles.GetTable().AddCol("_geom_angle_atom_site_label_1");
  angles.GetTable().AddCol("_geom_angle_atom_site_label_2");
  angles.GetTable().AddCol("_geom_angle_atom_site_label_3");
  angles.GetTable().AddCol("_geom_angle");
  angles.GetTable().AddCol("_geom_angle_site_symmetry_1");
  angles.GetTable().AddCol("_geom_angle_site_symmetry_3");
  angles.GetTable().AddCol("_geom_angle_publ_flag");
  for( size_t i=0; i < latt.AtomCount(); i++ )  {
    TSAtom& a = latt.GetAtom(i);
    if( a.GetType().GetMr()  < 3 || a.IsDeleted() || !a.GetMatrix(0).IsFirst() )  continue;
    for( size_t j=0; j < a.NodeCount(); j++ )  {
      TSAtom& b = a.Node(j);
      if( b.IsDeleted() || b.GetType().GetMr() < 3 )
        continue;
      for( size_t k=j+1; k < a.NodeCount(); k++ )  {
        TSAtom& c = a.Node(k);      
        if( c.IsDeleted() || c.GetType().GetMr() < 3 )
          continue;
        TCifRow& row = angles.GetTable().AddRow(EmptyString);
        row[0] = b.GetLabel();  row.GetObject(0) = new TCifLoopData(&b.CAtom());
        row[1] = a.GetLabel();  row.GetObject(1) = new TCifLoopData(&a.CAtom());
        row[2] = c.GetLabel();  row.GetObject(2) = new TCifLoopData(&c.CAtom());
        row[3] = vcovc.CalcAngle(b, a, c).ToString();
        if( !b.GetMatrix(0).IsFirst() )
          row[4] = TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell(), b.GetMatrix(0));
        else
          row[4] = '.';
        if( !c.GetMatrix(0).IsFirst() )
          row[5] = TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell(), c.GetMatrix(0));
        else
          row[5] = '.';
        row[6] = '?';
        for( int l=3; l < 7; l++ )
          row.GetObject(l) = new TCifLoopData;
      }
    }
  }
  RefinementModel& rm = xapp.XFile().GetRM();
  if( rm.InfoTabCount() != 0 )  {
    TCifLoop& hbonds = cif.AddLoop("_geom_hbond");
    hbonds.GetTable().AddCol("_geom_hbond_atom_site_label_D");
    hbonds.GetTable().AddCol("_geom_hbond_atom_site_label_H");
    hbonds.GetTable().AddCol("_geom_hbond_atom_site_label_A");
    hbonds.GetTable().AddCol("_geom_hbond_distance_DH");
    hbonds.GetTable().AddCol("_geom_hbond_distance_HA");
    hbonds.GetTable().AddCol("_geom_hbond_distance_DA");
    hbonds.GetTable().AddCol("_geom_hbond_angle_DHA");
    hbonds.GetTable().AddCol("_geom_hbond_site_symmetry_A");
    smatd I;
    I.I().SetId(0);
    TAtomEnvi envi;
    for( size_t i=0; i < rm.InfoTabCount(); i++ )  {
      InfoTab& it = rm.GetInfoTab(i);
      if( it.GetType() != infotab_htab || !it.IsValid() )  continue;
      TGroupCAtom *d = NULL, *a = NULL;
      for( size_t j=0; j < it.Count(); j++ )  {
        if( it.GetAtom(j).GetAtom()->IsDeleted() )  continue;
        if( d == NULL )
          d = &it.GetAtom(j);
        else  {
          a = &it.GetAtom(j);
          break;
        }
      }
      TSAtom* dsa = xapp.XFile().GetLattice().FindSAtom(*d->GetAtom());
      if( dsa == NULL )  continue;  //eh?
      envi.Clear();
      xapp.XFile().GetUnitCell().GetAtomEnviList(*dsa, envi);
      for( size_t j=0; j < envi.Count(); j++ )  {
        if( envi.GetType(j).GetMr() != iHydrogenZ)  continue;
        TCifRow& row = hbonds.GetTable().AddRow(EmptyString);
        row[0] = d->GetAtom()->GetLabel();  row.GetObject(0) = new TCifLoopData(d->GetAtom());
        row[1] = envi.GetCAtom(j).GetLabel();  row.GetObject(1) = new TCifLoopData(&envi.GetCAtom(j));
        row[2] = a->GetAtom()->GetLabel();  row.GetObject(2) = new TCifLoopData(a->GetAtom());
        TSAtom da(NULL), aa(NULL);
        da.CAtom(*d->GetAtom());
        da.AddMatrix(&I);
        au.CellToCartesian(da.ccrd(), da.crd());
        aa.CAtom(*a->GetAtom());
        smatd am;
        if( a->GetMatrix() == 0 )  {
          am.I();
          am.SetId(0);
        }
        else  {
          am = *a->GetMatrix();
          xapp.XFile().GetUnitCell().InitMatrixId(am);
        }
        aa.AddMatrix(&am);
        aa.ccrd() = am*aa.ccrd();
        au.CellToCartesian(aa.ccrd(), aa.crd());
        row[3] = olxstr::FormatFloat(2, envi.GetCrd(j).DistanceTo(da.crd()));
        row[4] = olxstr::FormatFloat(2, envi.GetCrd(j).DistanceTo(aa.crd()));
        row[5] = vcovc.CalcDistance(da, aa).ToString();
        row[6] = olxstr::FormatFloat(1, Angle(da.crd(), envi.GetCrd(j), aa.crd()));
        if( a->GetMatrix() == NULL )
          row[7] = '.';
        else
          row[7] = TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell(), aa.GetMatrix(0));
        for( int l=3; l < 8; l++ )
          row.GetObject(l) = new TCifLoopData;
      }
    }
  }
  cif.SaveToFile(TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif"));
}
//..............................................................................
struct XLibMacros_StrF  {
  int h, k, l;
  double ps;
  TEComplex<double> v;
};
void XLibMacros::macVoidE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& XApp = TXApp::GetInstance();
  double F000 = 0;
  double factor = 2;
  TRefList refs;
  TArrayList<TEComplex<double> > F;
  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
  const TUnitCell& uc = XApp.XFile().GetUnitCell();
  // space group matrix list
  TSpaceGroup* sg = NULL;
  try  { sg = &XApp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not locate space group");
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll^mattInversion);
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetType() == iQPeakZ )  
      continue;
    F000 += ca.GetType().z*uc.MatrixCount()*ca.GetOccu();
  }
  olxstr fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fcf");
  if( !TEFile::Exists(fcffn) )  {
    fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fco");
    if( !TEFile::Exists(fcffn) )  {
      E.ProcessingError(__OlxSrcInfo, "please load fcf file or make sure the one exists in current folder");
      return;
    }
  }
  TCif cif;
  cif.LoadFromFile( fcffn );
//  F000 = cif.GetSParam("_exptl_crystal_F_000").ToDouble();
  TCifLoop* hklLoop = cif.FindLoop("_refln");
  if( hklLoop == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  size_t hInd = hklLoop->GetTable().ColIndex("_refln_index_h");
  size_t kInd = hklLoop->GetTable().ColIndex("_refln_index_k");
  size_t lInd = hklLoop->GetTable().ColIndex("_refln_index_l");
  // list 3, F
  size_t mfInd = hklLoop->GetTable().ColIndex("_refln_F_meas");
  size_t sfInd = hklLoop->GetTable().ColIndex("_refln_F_sigma");
  size_t aInd = hklLoop->GetTable().ColIndex("_refln_A_calc");
  size_t bInd = hklLoop->GetTable().ColIndex("_refln_B_calc");

  if( (hInd|kInd|lInd|mfInd|sfInd|aInd|bInd) == InvalidIndex ) {
      E.ProcessingError(__OlxSrcInfo, "list 3 fcf file is expected");
      return;
  }
  refs.SetCapacity( hklLoop->GetTable().RowCount() );
  F.SetCount( hklLoop->GetTable().RowCount() );
  for( size_t i=0; i < hklLoop->GetTable().RowCount(); i++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->GetTable()[i];
    TReflection& ref = refs.AddNew(row[hInd].ToInt(), row[kInd].ToInt(), 
      row[lInd].ToInt(), row[mfInd].ToDouble(), row[sfInd].ToDouble());
    if( ref.GetH() < 0 )
      factor = 4;
//    const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
//    F[i] = TEComplex<double>::polar(ref.GetI(), rv.arg());
//    F[i].A() = row[aInd].ToDouble();
//    F[i].B() = row[bInd].ToDouble();
      const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
      double dI = (ref.GetI() - rv.mod());
      F[i] = TEComplex<double>::polar(dI, rv.arg());
  }
  olxstr hklFileName = XApp.LocateHklFile();
  if( !TEFile::Exists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  double vol = XApp.XFile().GetLattice().GetUnitCell().CalcVolume();
  int minH = 100,  minK = 100,  minL = 100;
  int maxH = -100, maxK = -100, maxL = -100;

  vec3d hkl;
  TArrayList<XLibMacros_StrF> AllF(refs.Count()*ml.Count());
  int index = 0;
  double f000 = 0;
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    for( size_t j=0; j < ml.Count(); j++, index++ )  {
      ref.MulHkl(hkl, ml[j]);
      if( hkl[0] < minH )  minH = (int)hkl[0];
      if( hkl[1] < minK )  minK = (int)hkl[1];
      if( hkl[2] < minL )  minL = (int)hkl[2];
      if( hkl[0] > maxH )  maxH = (int)hkl[0];
      if( hkl[1] > maxK )  maxK = (int)hkl[1];
      if( hkl[2] > maxL )  maxL = (int)hkl[2];
      AllF[index].h = (int)hkl[0];
      AllF[index].k = (int)hkl[1];
      AllF[index].l = (int)hkl[2];
      AllF[index].ps = hkl[0]*ml[j].t[0] + hkl[1]*ml[j].t[1] + hkl[2]*ml[j].t[2];
      AllF[index].v = F[i];
      AllF[index].v *= TEComplex<double>::polar(1, 2*M_PI*AllF[index].ps);
    }
  }
// init map, 0.1A for now
  const int mapX = (int)au.Axes()[0].GetV()*3,
			mapY = (int)au.Axes()[1].GetV()*3,
			mapZ = (int)au.Axes()[2].GetV()*3;
  double mapVol = mapX*mapY*mapZ;
  TArray3D<double> fMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
//////////////////////////////////////////////////////////////////////////////////////////
  TEComplex<double> ** S, *T;
  int kLen = maxK-minK+1, hLen = maxH-minH+1, lLen = maxL-minL+1;
  S = new TEComplex<double>*[kLen];
  for( int i=0; i < kLen; i++ )
    S[i] = new TEComplex<double>[lLen];
  T = new TEComplex<double>[lLen];
  const double T_PI = 2*M_PI;
// precalculations
  int minInd = olx_min(minH, minK);
  if( minL < minInd )  minInd = minL;
  int maxInd = olx_max(maxH, maxK);
  if( maxL > maxInd )  maxInd = maxL;
  int iLen = maxInd - minInd + 1;
  int mapMax = olx_max(mapX, mapY);
  if( mapZ > mapMax )  mapMax = mapZ;
  TEComplex<double>** sin_cosX = new TEComplex<double>*[mapX],
                      **sin_cosY, **sin_cosZ;
  for( int i=0; i < mapX; i++ )  {
    sin_cosX[i] = new TEComplex<double>[iLen];
    for( int j=minInd; j <= maxInd; j++ )  {
      double rv = (double)(i*j)/mapX, ca, sa;
      rv *= T_PI;
      SinCos(-rv, &sa, &ca);
      sin_cosX[i][j-minInd].SetRe(ca);
      sin_cosX[i][j-minInd].SetIm(sa);
    }
  }
  if( mapX == mapY )  {
    sin_cosY = sin_cosX;
  }
  else  {
    sin_cosY = new TEComplex<double>*[mapY];
    for( int i=0; i < mapY; i++ )  {
      sin_cosY[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapY, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosY[i][j-minInd].SetRe(ca);
        sin_cosY[i][j-minInd].SetIm(sa);
      }
    }
  }
  if( mapX == mapZ )  {
    sin_cosZ = sin_cosX;
  }
  else if( mapY == mapZ )  {
    sin_cosZ = sin_cosY;
  }
  else  {
    sin_cosZ = new TEComplex<double>*[mapZ];
    for( int i=0; i < mapZ; i++ )  {
      sin_cosZ[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapZ, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosZ[i][j-minInd].SetRe(ca);
        sin_cosZ[i][j-minInd].SetIm(sa);
      }
    }
  }
  TEComplex<double> R;
  double maxMapV = -1000, minMapV = 1000;
  for( int ix=0; ix < mapX; ix++ )  {
    for( size_t i=0; i < AllF.Count(); i++ )  {
      const XLibMacros_StrF& sf = AllF[i];
      S[sf.k-minK][sf.l-minL] += sf.v*sin_cosX[ix][sf.h-minInd];
    }
    for( int iy=0; iy < mapY; iy++ )  {
      for( int i=minK; i <= maxK; i++ )  {
        for( int j=minL; j <= maxL; j++ )  {
          T[j-minL] += S[i-minK][j-minL]*sin_cosY[iy][i-minInd];
        }
      }
      for( int iz=0; iz < mapZ; iz++ )  {
        R.Null();
        for( int i=minL; i <= maxL; i++ )  {
          R += T[i-minL]*sin_cosZ[iz][i-minInd];
        }
        double val = factor*R.Re()/vol;
        if( val > maxMapV )  maxMapV = val;
        if( val < minMapV )  minMapV = val;
        fMap.Data[ix][iy][iz] = val;
      }
      for( int i=0; i < lLen; i++ )  
        T[i].Null();
    }
    for( int i=0; i < kLen; i++ )  
      for( int j=0; j < lLen; j++ )  
        S[i][j].Null();
  }
  TBasicApp::GetLog() << (olxstr("Map max val ") << olxstr::FormatFloat(3, maxMapV) << " min val " << olxstr::FormatFloat(3, minMapV) << '\n');
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // calculate the map
  double surfdis = Options.FindValue("d", "1.0").ToDouble();
  size_t structurePoints = 0;
  vec3d voidCenter;
  TArray3D<short> maskMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
  short MaxLevel = XApp.CalcVoid(maskMap, surfdis, -101, &structurePoints, voidCenter, NULL);
  XApp.GetLog() << ( olxstr("Cell volume (A^3) ") << olxstr::FormatFloat(3, vol) << '\n');
  XApp.GetLog() << ( olxstr("Max level reached ") << MaxLevel << '\n');
  XApp.GetLog() << ( olxstr("Largest spherical void is (A^3) ") << olxstr::FormatFloat(3, MaxLevel*MaxLevel*MaxLevel*4*M_PI/(3*mapVol)*vol) << '\n');
  XApp.GetLog() << ( olxstr("Structure occupies (A^3) ") << olxstr::FormatFloat(3, structurePoints*vol/mapVol) << '\n');
  int minLevel = olx_round( pow( 6*mapVol*3/(4*M_PI*vol), 1./3) );
  XApp.GetLog() << ( olxstr("6A^3 level is ") << minLevel << '\n');
  // calculate new structure factors
  double Re = 0, Te=0, F0 = 0;
  int RePointCount = 0, TePointCount = 0;
//  for( int i=0; i < refs.Count(); i++ )  {
//    TReflection& ref = refs[i];
//    double A = 0, B = 0;
    for( int ix=0; ix < mapX; ix++ )  {
      for( int iy=0; iy < mapY; iy++ )  {
        for( int iz=0; iz < mapZ; iz++ )  {
          if( maskMap.Data[ix][iy][iz] <= 0  )  {
//            double tv =  (double)ref.GetH()*ix/mapX;  
//            tv += (double)ref.GetK()*iy/mapY;  
//            tv += (double)ref.GetL()*iz/mapZ;
//            tv *= T_PI;
//            double ca, sa;
//            SinCos(tv, &sa, &ca);
//            A += fMap.Data[ix][iy][iz]*ca;
//            B += fMap.Data[ix][iy][iz]*sa;
//            if( i == 0 )  {
              Te += fMap.Data[ix][iy][iz];
              TePointCount++;
//            }
          }
          else   {
//            if( i == 0 )  {
              Re += fMap.Data[ix][iy][iz];
              RePointCount++;
//            }
          }
//          if( i == 0 )  {
            F0 += fMap.Data[ix][iy][iz];
//          }
        }
      }
    }
//    ref.SetI( sqrt(A*A+B*B)/100 );
//  }
//  TCStrList sl;
//  for( int i=0;  i < refs.Count(); i++ )
//    sl.Add( refs[i].ToString() );
//  sl.SaveToFile( "test.hkl" );
  XApp.GetLog() << "Voids         " << Re*vol/(mapVol) << "e-\n";
//  XApp.GetLog() << "F000 calc     " << Te*vol/(mapVol) << "e-\n";
  XApp.GetLog() << "F000 (formula)" << F000 << "e-\n";
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  for( int i=0; i < kLen; i++ )
    delete [] S[i];
  delete [] S;
  delete [] T;
  if( sin_cosY == sin_cosX )  sin_cosY = NULL;
  if( sin_cosZ == sin_cosX || sin_cosZ == sin_cosY )  sin_cosZ = NULL;
  for( int i=0; i < mapX; i++ )
    delete [] sin_cosX[i];
  delete [] sin_cosX;
  if( sin_cosY != NULL )  {
    for( int i=0; i < mapY; i++ )
      delete [] sin_cosY[i];
    delete [] sin_cosY;
  }
  if( sin_cosZ != NULL )  {
    for( int i=0; i < mapZ; i++ )
      delete [] sin_cosZ[i];
    delete [] sin_cosZ;
  }
}

//..............................................................................
void XLibMacros::macChangeSG(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup& from_sg = xapp.XFile().GetLastLoaderSG();
  TSpaceGroup* sg = TSymmLib::GetInstance().FindGroup(Cmds.Last().String);
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify given space group");
    return;
  }
  // change centering?
  if( from_sg.GetName().SubStringFrom(1) == sg->GetName().SubStringFrom(1) )  {
    olxch from = from_sg.GetLattice().GetSymbol()[0],
          to = sg->GetLattice().GetSymbol()[0];
    mat3d tm;
    tm.I();
    if( from == 'I' )  {
      if( to == 'P' )
        tm = mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);
    }
    else if( from == 'P' )  {
      if( to == 'I' )
        tm = mat3d(0, 1, 1, 0, 1, 0);
      else if( to == 'C' )  {
        tm = mat3d(0, 1, 1, 0, 1, 0);  // P->I
        tm *= mat3d(-1, 0, 1, 0, 1, 0, -1, 0, 0);  // I->C, uniq axis b
      }
      else if( to == 'F' )
        tm = mat3d(-1, 1, 1, -1, 1, 1);
    }
    else if( from == 'C' )  {
      if( to == 'P' )  {
        tm = mat3d(0, 0, -1, 0, 1, 0, 1, 0, -1);  // C->I, uniq axis b
        tm *= mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);  // I->P 
      }
    }
    else if( from == 'F')  {
      if( to == 'P' )
        tm = mat3d(0, 0.5, 0.5, 0, 0.5, 0);
    }
    if( !tm.IsI() )  {
      TBasicApp::GetLog() << "EXPERIMENTAL: transformations considering b unique\n";
      ChangeCell(tm, *sg);
    }
    else  {
      TBasicApp::GetLog() << "The transformation is not supported\n";
    }
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll);
  TTypeList<AnAssociation3<vec3d,TCAtom*, int> > list;
  uc.GenereteAtomCoordinates(list, true);
  if( Cmds.Count() == 4 )  {
    vec3d trans(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble());
    for( size_t i=0; i < list.Count(); i++ )  {
      list[i].A() += trans;
      list[i].SetC(1);
    }
  }
  else   {
    for( size_t i=0; i < list.Count(); i++ )  { 
      list[i].SetC(1);
    }
  }
  vec3d v;
  for( size_t i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    for( size_t j=i+1; j < list.Count(); j++ )  {
      if( list[j].GetC() == 0 )  continue;
      for( size_t k=1; k < ml.Count(); k++ )  {
        v = ml[k] * list[i].GetA();
        v -= list[j].GetA();
        v[0] -= olx_round(v[0]);  v[1] -= olx_round(v[1]);  v[2] -= olx_round(v[2]);
        au.CellToCartesian(v);
        if( v.QLength() < 0.01 )  {
          list[i].C() ++;
          list[j].SetC(0);
        }
      }
    }
  }
  for( size_t i=0; i < au.AtomCount(); i++ )
    au.GetAtom(i).SetTag(0);
  TCAtomPList newAtoms;
  for( size_t i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    TCAtom* ca;
    if( list[i].GetB()->GetTag() > 0 )  {
      ca = &au.NewAtom();
      ca->Assign(*list[i].GetB());
    }
    else  {
      ca = list[i].GetB();
      ca->SetTag(ca->GetTag() + 1);
    }
    ca->ccrd() = list[i].GetA();
    ca->AssignEllp(NULL);
  }
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetTag() == 0 )
      au.GetAtom(i).SetDeleted(true);
  }
  au.ChangeSpaceGroup(*sg);
  xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
  latt.Init();
  latt.CompaqAll();
}
//..............................................................................
void XLibMacros::macFlush(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TBasicApp::GetLog().Flush();
}
//..............................................................................
void XLibMacros::macSGE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  using namespace olex;
  TXApp& xapp = TXApp::GetInstance();
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "this function requires Olex2 processor implementation");
  TSpaceGroup* sg = NULL;
  if( EsdlInstanceOf(*xapp.XFile().LastLoader(), TCRSFile) && 
    ((TCRSFile*)xapp.XFile().LastLoader())->HasSG() )  
  {
    sg = &xapp.XFile().GetLastLoaderSG();
    TBasicApp::GetLog() << "Choosing CRS file space group: " << sg->GetName() << '\n';
  }
  else  {
    TPtrList<TSpaceGroup> sgs;
    bool cntro = false;
    E.SetRetVal(&sgs);
    op->executeMacroEx("SG", E);
    E.SetRetVal<bool>(false);
    if( sgs.Count() == 0 )  {
      TBasicApp::GetLog().Error( "Could not find any suitable space group. Terminating ... " );
      return;
    }
    else if( sgs.Count() == 1 )  {
      sg = sgs[0];
      TBasicApp::GetLog() << "Univocal space group choice: " << sg->GetName() << '\n';
    }
    else  {
      E.Reset();
      op->executeMacroEx("Wilson", E);
      bool centro = E.GetRetVal().ToBool();
      TBasicApp::GetLog() << "Searching for centrosymmetric group: " << centro << '\n';
      for( size_t i=0; i < sgs.Count(); i++ )  {
        if( centro )  {
          if( sgs[i]->IsCentrosymmetric() )  {
            sg = sgs[i];
            break;
          }
        }
        else  {
          if( !sgs[i]->IsCentrosymmetric() )  {
            sg = sgs[i];
            break;
          }
        }
      }
      if( sg == NULL )  {  // no match to centre of symmetry found
        sg = sgs[0];
        TBasicApp::GetLog() << "Could not match, choosing: " << sg->GetName() << '\n';
      }
      else  {
        TBasicApp::GetLog() << "Chosen: " << sg->GetName() << '\n';
      }
    }
  }
  olxstr fn( Cmds.IsEmpty() ? TEFile::ChangeFileExt(TXApp::GetInstance().XFile().GetFileName(), "ins") : Cmds[0] );
  op->executeMacroEx(olxstr("reset -s=") << sg->GetName() << " -f='" << fn << '\'', E);
  if( E.IsSuccessful() )  {
    op->executeMacroEx(olxstr("reap '") << fn << '\'', E);
    if( E.IsSuccessful() )  { 
      op->executeMacroEx(olxstr("solve"), E);
      // this will reset zoom!
      op->executeMacroEx(olxstr("fuse"), E);
    }
    E.SetRetVal<bool>(E.IsSuccessful());
  }
}
//..............................................................................
void XLibMacros::macASR(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  if( sg.IsCentrosymmetric() )  {
    E.ProcessingError(__OlxSrcInfo, "not applicable to centrosymmetric space groups");
    return;
  }
  if( xapp.XFile().GetRM().GetHKLF() == 5 || xapp.XFile().GetRM().GetHKLF() == 6 )  {
    E.ProcessingError(__OlxSrcInfo, "not applicable to HKLF 5/6 data format");
    return;
  }
  if( xapp.XFile().GetRM().GetBASF().IsEmpty() )  {
    xapp.XFile().GetRM().AddBASF(0.2);
    xapp.GetLog() << "BASF 0.2 is added\n";
  }
  if( !xapp.XFile().GetRM().HasTWIN() )  {
    xapp.XFile().GetRM().SetTWIN_n(2);
    xapp.GetLog() << "TWIN set to 2 components\n";
  }
  if( xapp.XFile().GetRM().HasMERG() && xapp.XFile().GetRM().GetMERG() == 4 )
    xapp.GetLog() << "Please note, that currently Friedel pairs are merged\n";
  xapp.GetLog() << "Done\n";
}
//..............................................................................
void XLibMacros::macDescribe(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TStrList lst, out;
  xapp.XFile().GetRM().Describe(lst);
  for( size_t i=0; i < lst.Count(); i++ )
    out.Hyphenate(lst[i], 80, true);
  xapp.GetLog() << out << '\n'; 
}
//..............................................................................
void XLibMacros::macCalcCHN(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "Nor file is loaded neither formula is provided" );
    return;
  }
  TCHNExp chn;
  double C=0, H=0, N=0, Mr=0;
  if( Cmds.Count() == 1 )  {
    chn.LoadFromExpression(Cmds[0]);
    chn.CHN(C, H, N, Mr);
    TBasicApp::GetLog() << (olxstr("Molecular weight: ") << Mr << '\n');
    olxstr Msg("C: ");
    Msg << olxstr::FormatFloat(3, C*100./Mr) <<
      " H: " << olxstr::FormatFloat(3, H*100./Mr) <<
      " N: " << olxstr::FormatFloat(3, N*100./Mr);
    TBasicApp::GetLog() << (Msg << '\n' << '\n');
    TBasicApp::GetLog() << (olxstr("Full composition:\n") << chn.Composition() << '\n');
    return;
  }
  chn.LoadFromExpression(xapp.XFile().GetAsymmUnit().SummFormula(EmptyString));
  chn.CHN(C, H, N, Mr);
  TBasicApp::GetLog() << (olxstr("Molecular weight: ") << Mr << '\n');
  olxstr Msg("C: ");
  Msg << olxstr::FormatFloat(3, C*100./Mr) << 
    " H: " << olxstr::FormatFloat(3, H*100./Mr) <<
    " N: " << olxstr::FormatFloat(3, N*100./Mr);
  TBasicApp::GetLog() << (Msg << '\n' << '\n');
  TBasicApp::GetLog() << (olxstr("Full composition:\n") << chn.Composition() << '\n');
}
//..............................................................................
void XLibMacros::macCalcMass(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "Nor file is loaded neither formula is provided" );
    return;
  }
  TIPattern ip;
  if( Cmds.Count() == 1 )  {
    olxstr err;
    if( !ip.Calc(Cmds[0], err, true, 0.5) )  {
      Error.ProcessingError(__OlxSrcInfo, "could not parse the given expression: ") << err;
      return;
    }
  }
  else  {
    olxstr err;
    if( !ip.Calc(xapp.XFile().GetAsymmUnit().SummFormula(EmptyString), err, true, 0.5) )  {
      Error.ProcessingError(__OlxSrcInfo, "could not parse the given expression: ") << err;
      return;
    }
  }
  for( size_t i=0; i < ip.PointCount(); i++ )  {
    const TSPoint& point = ip.Point(i);
    if( point.Y < 0.001 )  break;
    olxstr Msg = point.X;
    Msg.Format(11, true, ' ');
    Msg << ": " << point.Y;
    xapp.GetLog() << (Msg << '\n');
  }
  TBasicApp::GetLog() << ("    -- NOTE THAT NATURAL DISTRIBUTION OF ISOTOPES IS ASSUMED --    \n");
  TBasicApp::GetLog() << ("******* ******* SPECTRUM ******* ********\n");
  ip.SortDataByMolWeight();
  for( size_t i=0; i < ip.PointCount(); i++ )  {
    const TSPoint& point = ip.Point(i);
    if( point.Y < 1 )  continue;
    olxstr Msg = point.X;
    Msg.Format(11, true, ' ');
    Msg << "|";
    long yVal = olx_round(point.Y/2);
    for( long j=0; j < yVal; j++ )
      Msg << '-';
    xapp.GetLog() << (Msg << '\n');
  }
}
//..............................................................................
evecd XLibMacros_fit_chn_calc(const ematd& m, const evecd& p, size_t cnt)  {
  ematd _m(m.Vectors(), cnt), _mt(cnt, m.Vectors());
  evecd _v(m.Vectors()), res(cnt);
  for( size_t i=0; i < m.Vectors(); i++ )  {
    for( size_t j=0; j < cnt; j++ )  {
      _m[i][j] = m[i][j];
      _mt[j][i] = m[i][j];
    }
    _v[i] = p[i];
  }
  ematd nm = _mt*_m;
  evecd nv = _mt*_v;
  if( cnt == 1 )  {
    if( nm[0][0] == 0 )
      res[0] = -1;
    else
      res[0] = nv[0]/nm[0][0];
  }
  else  {  
    try  {  ematd::GauseSolve(nm, nv, res);  }
    catch(...)  {
      for( size_t i=0; i < res.Count(); i++ )
        res[i] = -1.0;
    }
  }
  return res;
}
struct XLibMacros_ChnFitData  {
  double dev;
  olxstr formula;
  int Compare(const XLibMacros_ChnFitData& v) const {
    const double df = dev - v.dev;
    return df < 0 ? -1 : (df > 0 ? 1 : 0);
  }
};
void XLibMacros_fit_chn_process(TTypeList<XLibMacros_ChnFitData>& list, const ematd& chn,
                                const evecd& p,
                                const olxstr names[4],
                                const olxdict<short, double, TPrimitiveComparator>& obs,
                                size_t cnt)
{
  for( size_t i=0; i < cnt; i++ )  {
    if( p[i] < 0 || fabs(p[i]) < 0.05 || p[i] > 5 )  return;
  }
  double mw = chn[0][obs.Count()];
  evecd e(obs.Count());
  for( size_t i=0; i < obs.Count(); i++ )
    e[i] = chn[0][i];
  olxstr name = names[0];
  for( size_t i=0; i < cnt; i++ )  {
    mw += p[i]*chn[i+1][obs.Count()];
    for( size_t j=0; j < obs.Count(); j++ )
      e[j] += p[i]*chn[i+1][j];
    name << " (" << names[i+1] << ')';
    name << olxstr::FormatFloat(2, p[i]);
  }
  XLibMacros_ChnFitData& cfd = list.AddNew();
  cfd.formula = name;
  double dev = 0;
  for( size_t i=0; i < obs.Count(); i++ )  {
    dev += olx_sqr(obs.GetValue(i)-e[i]/mw);
  }
  cfd.dev = sqrt(dev);
}
void XLibMacros::macFitCHN(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TCHNExp chne;
  chne.LoadFromExpression(Cmds[0]);
  TStrList solvents;
  olxdict<short, double, TPrimitiveComparator> obs, calc;
  for( size_t i=1; i < Cmds.Count(); i++ )  {
    size_t si = Cmds[i].IndexOf(':');
    if( si == InvalidIndex )
      solvents.Add(Cmds[i]);
    else  {
      cm_Element* elm = XElementLib::FindBySymbol(Cmds[i].SubStringTo(si));
      if( elm == NULL )  {
        Error.ProcessingError(__OlxSrcInfo, olxstr("Invalid element: ") << Cmds[i]);
        return;
      }
      obs(elm->index, Cmds[i].SubStringFrom(si+1).ToDouble()/100);
      calc(elm->index, 0);
    }
  }
  if( solvents.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "a space separated list of solvents is expected");
    return;
  }
  TTypeList<XLibMacros_ChnFitData> list;
  const double Mw = chne.CHN(calc);
  olxstr names[4] = {chne.SummFormula(EmptyString), EmptyString, EmptyString, EmptyString};
  ematd m(obs.Count(), 3), chn(4, obs.Count()+1);
  evecd p(obs.Count());
  olxstr fit_info_from = "Fitting ", fit_info_to;
  for( size_t i=0; i < obs.Count(); i++ )  {
    p[i] = calc.GetValue(i) - obs.GetValue(i)*Mw;
    chn[0][i] = calc.GetValue(i);
    fit_info_from << XElementLib::GetByIndex(calc.GetKey(i)).symbol << ':' << olxstr::FormatFloat(2, calc.GetValue(i)*100/Mw);
    fit_info_to << XElementLib::GetByIndex(obs.GetKey(i)).symbol << ':' << olxstr::FormatFloat(2, obs.GetValue(i)*100);
    if( (i+1) < calc.Count() )  {
       fit_info_to << ' ';
       fit_info_from << ' ';
    }
  }
  TBasicApp::GetLog() << (fit_info_from << " to " << fit_info_to << '\n');
  chn[0][obs.Count()] = Mw;
  for( size_t i=0; i < solvents.Count(); i++ )  {
    chne.LoadFromExpression(solvents[i]);
    const double mw = chne.CHN(calc);
    names[1] = chne.SummFormula(EmptyString);
    for( size_t i1=0; i1 < obs.Count(); i1++ )  {
      m[i1][0] = obs.GetValue(i1)*mw - calc.GetValue(i1);
      chn[1][i1] = calc.GetValue(i1);
    }
    chn[1][obs.Count()] = mw;
    evecd res = XLibMacros_fit_chn_calc(m, p, 1);
    XLibMacros_fit_chn_process(list, chn, res, names, obs, 1);
    for( size_t j = i+1; j < solvents.Count(); j++ )  {
      chne.LoadFromExpression(solvents[j]);
      const double mw1 = chne.CHN(calc);
      names[2] = chne.SummFormula(EmptyString);
      for( size_t i1=0; i1 < obs.Count(); i1++ )  {
        m[i1][1] = obs.GetValue(i1)*mw1 - calc.GetValue(i1);
        chn[2][i1] = calc.GetValue(i1);
      }
      chn[2][obs.Count()] = mw1;
      evecd res = XLibMacros_fit_chn_calc(m, p, 2);
      XLibMacros_fit_chn_process(list, chn, res, names, obs, 2);
      for( size_t k=j+1; k < solvents.Count(); k++ )  {
        chne.LoadFromExpression(solvents[k]);
        const double mw2 = chne.CHN(calc);
        names[3] = chne.SummFormula(EmptyString);
        for( size_t i1=0; i1 < obs.Count(); i1++ )  {
          m[i1][2] = obs.GetValue(i1)*mw2 - calc.GetValue(i1);
          chn[3][i1] = calc.GetValue(i1);
        }
        chn[3][obs.Count()] = mw2;
        evecd res = XLibMacros_fit_chn_calc(m, p, 3);
        XLibMacros_fit_chn_process(list, chn, res, names, obs, 3);
      }
    }
  }
  if( list.IsEmpty() )
    TBasicApp::GetLog() << "Could not fit provided data\n";
  else  {
    list.QuickSorter.Sort<TComparableComparator>(list);
    TETable tab(list.Count(), 3);
    tab.ColName(0) = "Formula";
    tab.ColName(1) = "CHN";
    tab.ColName(2) = "Deviation";
    for( size_t i=0; i < list.Count(); i++ )  {
      tab[i][0] = list[i].formula;
      chne.LoadFromExpression(list[i].formula);
      const double M = chne.CHN(calc);
      for( size_t j=0; j < calc.Count(); j++ )  {
        tab[i][1] << XElementLib::GetByIndex(calc.GetKey(j)).symbol << ':' << olxstr::FormatFloat(2, calc.GetValue(j)*100/M);
        if( (j+1) < calc.Count() )
          tab[i][1] << ' ';
      }
      tab[i][2] = olxstr::FormatFloat(2, list[i].dev*100);
    }
    TStrList sl;
    tab.CreateTXTList(sl, "Summary", true, false, ' ');
    TBasicApp::GetLog() << sl << '\n';
  }
}
//..............................................................................
void XLibMacros::macStandardise(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  smatd_list sm;
  sg.GetMatrices(sm, mattAll^mattIdentity);
  if( Cmds.IsEmpty() )  {
    for( size_t i=0; i < au.AtomCount(); i++ )
      MapUtil::StandardiseVec(au.GetAtom(i).ccrd(), sm);
  }
  else  {
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      TCAtom& ca = au.GetAtom(i);
      vec3d &v = ca.ccrd(), cart;
      for( int j=0; j < 3; j++ )  {
        while( v[j] < 0 )  v[j] += 1.0;
        while( v[j] >= 1.0 )  v[j] -= 1.0;
      }
      double d = au.CellToCartesian(v, cart).QLength();
      for( size_t j=0; j < sm.Count(); j++ )  {
        vec3d tmp = sm[j]*v;
        for( int k=0; k < 3; k++ )  {
          while( tmp[k] < 0 )  tmp[k] += 1.0;
          while( tmp[k] >= 1.0 )  tmp[k] -= 1.0;
        }
        const double _d = au.CellToCartesian(tmp, cart).QLength();
        if( _d < d )  {
          d = _d;
          v = tmp;
        }
      }
      //MapUtil::StandardiseVec(ca.ccrd(), sm);
    }
  }
  xapp.XFile().GetLattice().Init();
  xapp.XFile().GetLattice().Uniq();
}
//..............................................................................
void XLibMacros::macOmit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  static olxstr sig("OMIT");
  const TIns& ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  const TLst& Lst = ins.GetLst();
  if( !Lst.IsLoaded() )  {  return;  }
  if( Cmds.Count() == 1 )  {
    if( Cmds[0].IsNumber() )  {
      const double th = Cmds[0].ToDouble();
      for( size_t i=0; i < Lst.DRefCount(); i++ )  {
        const TLstRef& r = Lst.DRef(i);
        if( !r.Deleted && r.DF >= th )
          rm.Omit(vec3i(r.H, r.K, r.L));
      }
    }
  }
  else if( Cmds.Count() == 2 )  {
    rm.SetOMIT_s(Cmds[0].ToDouble());
    rm.SetOMIT_2t(Cmds[1].ToDouble());
  }
  else 
    rm.AddOMIT(Cmds);
  OnAddIns.Exit(NULL, &sig);
}
//..............................................................................
void XLibMacros::funLst(const TStrObjList &Cmds, TMacroError &E)  {
  const TIns& ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  const TLst& Lst = ins.GetLst();
  if( !Lst.IsLoaded() )
    E.SetRetVal(NAString);
  else if( Cmds[0].Equalsi("rint") )
    E.SetRetVal(Lst.Rint());
  else if( Cmds[0].Equalsi("rsig") )
    E.SetRetVal(Lst.Rsigma());
  else if( Cmds[0].Equalsi("r1") )
    E.SetRetVal(Lst.R1());
  else if( Cmds[0].Equalsi("r1a") )
    E.SetRetVal(Lst.R1a());
  else if( Cmds[0].Equalsi("wr2") )
    E.SetRetVal(Lst.wR2());
  else if( Cmds[0].Equalsi("s") )
    E.SetRetVal(Lst.S());
  else if( Cmds[0].Equalsi("rs") )
    E.SetRetVal(Lst.RS());
  else if( Cmds[0].Equalsi("params") )
    E.SetRetVal(Lst.Params());
  else if( Cmds[0].Equalsi("rtotal") )
    E.SetRetVal(Lst.TotalRefs());
  else if( Cmds[0].Equalsi("runiq") )
    E.SetRetVal(Lst.UniqRefs());
  else if( Cmds[0].Equalsi("r4sig") )
    E.SetRetVal(Lst.Refs4sig());
  else if( Cmds[0].Equalsi("peak") )
    E.SetRetVal(Lst.Peak());
  else if( Cmds[0].Equalsi("hole") )
    E.SetRetVal(Lst.Hole());
  else if( Cmds[0].Equalsi("flack") )  {
    if( Lst.HasFlack() )
      E.SetRetVal(Lst.Flack().ToString());
    else
      E.SetRetVal(NAString);
  }
  else
    E.SetRetVal(NAString);
}
//..............................................................................
void XLibMacros::macReset(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TXApp& xapp = TXApp::GetInstance();
  if( !(xapp.CheckFileType<TIns>() ||
        xapp.CheckFileType<TP4PFile>() ||
        xapp.CheckFileType<TCRSFile>()  )  )  return;
  if( TOlxVars::IsVar("internal_tref") )
    return;
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  olxstr newSg(Options.FindValue('s')), 
         content( olxstr::DeleteChars(Options.FindValue('c'), ' ')),
         fileName(Options.FindValue('f') );
  xapp.XFile().UpdateAsymmUnit();
  TIns *Ins = (TIns*)xapp.XFile().FindFormat("ins");
  if( xapp.CheckFileType<TP4PFile>() )  {
    if( !newSg.Length() )  {
      E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch" );
      return;
    }
    Ins->Adopt(xapp.XFile());
  }
  else if( xapp.CheckFileType<TCRSFile>() )  {
    TSpaceGroup* sg = xapp.XFile().GetLastLoader<TCRSFile>().GetSG();
    if( newSg.IsEmpty() )  {
      if( sg == NULL )  {
        E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch" );
        return;
      }
      else  {
        TBasicApp::GetLog() << ( olxstr("The CRS file format space group is: ") << sg->GetName() << '\n');
      }
    }
    Ins->Adopt(xapp.XFile());
  }
  if( !content.IsEmpty() )
    Ins->GetRM().SetUserFormula(content);
  if( Ins->GetRM().GetUserContent().IsEmpty() )  {
    if( op != NULL )  {
      content = "getuserinput(1, \'Please, enter structure composition\', \'C1\')";
      op->executeFunction(content, content);
      Ins->GetRM().SetUserFormula(content);
      if( Ins->GetRM().GetUserContent().IsEmpty() )  {
        E.ProcessingError(__OlxSrcInfo, "empty SFAC instruction, please use -c=Content to specify" );
        return;
      }
    }
  }

  if( !newSg.IsEmpty() )  {
    TSpaceGroup* sg = TSymmLib::GetInstance().FindGroup(newSg);
    if( !sg )  {
      E.ProcessingError(__OlxSrcInfo, "could not find space group: ") << newSg;
      return;
    }
    Ins->GetAsymmUnit().ChangeSpaceGroup(*sg);
    newSg = EmptyString;
    newSg <<  " reset to " << sg->GetName() << " #" << sg->GetNumber();
    olxstr titl( TEFile::ChangeFileExt(TEFile::ExtractFileName(xapp.XFile().GetFileName()), EmptyString) );
    Ins->SetTitle( titl << " in " << sg->GetName() << " #" << sg->GetNumber());
  }
  if( fileName.IsEmpty() )
    fileName = xapp.XFile().GetFileName();
  olxstr FN(TEFile::ChangeFileExt(fileName, "ins"));
  olxstr lstFN(TEFile::ChangeFileExt(fileName, "lst"));

  Ins->SaveForSolution(FN, Cmds.Text(' '), newSg, !Options.Contains("rem"));
  if( TEFile::Exists(lstFN) )  {
    olxstr lstTmpFN(lstFN);
    lstTmpFN << ".tmp";
    TEFile::Rename(lstFN, lstTmpFN);
  }
  if( op != NULL )  {
    op->executeMacroEx(olxstr("@reap \'") << FN << '\'', E);
    if( E.IsSuccessful() )
      op->executeMacro("htmlreload");
  }
}
//..............................................................................
void XLibMacros::macDegen(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TSAtomPList atoms;
  TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true, !Options.Contains("cs"));
  for( size_t i=0; i < atoms.Count(); i++ ) 
    atoms[i]->CAtom().SetTag(i);
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i]->CAtom().GetTag() != i || atoms[i]->CAtom().GetDegeneracy() == 1)  continue;
    olxstr str(atoms[i]->CAtom().GetLabel());
    TBasicApp::GetLog() << (str.Format(6, true, ' ') <<  atoms[i]->CAtom().GetDegeneracy() << '\n');
  }
}
//..............................................................................
void XLibMacros::macClose(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp::GetInstance().XFile().Close();
}
//..............................................................................

#include "gxmacro.h"
#include "sfutil.h"
#include "maputil.h"
#include "beevers-lipson.h"
#include "unitcell.h"
#include "symmparser.h"
#include "vcov.h"
#include "dunitcell.h"
#include "dusero.h"
#include "xgrid.h"
#include "gllabels.h"
#include "xline.h"
#include "olxstate.h"
#include "estopwatch.h"
#include "analysis.h"
#include "planesort.h"
#include "xmacro.h"
#include "cif.h"
#ifdef __WXWIDGETS__
#include "wx/wx.h"
#endif

#define gxlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.Register(\
    new TMacro<GXLibMacros>(this, &GXLibMacros::mac##macroName, #macroName,\
      (validOptions), argc, desc))
#define gxlib_InitMacroA(macroName, amacroName, validOptions, argc, desc)\
  lib.Register(\
    new TMacro<GXLibMacros>(this, &GXLibMacros::mac##macroName, #amacroName,\
      (validOptions), argc, desc))
#define gxlib_InitFunc(funcName, argc, desc) \
  lib.Register(\
    new TFunction<GXLibMacros>(this, &GXLibMacros::fun##funcName, #funcName, argc,\
      desc))

//.............................................................................
void GXLibMacros::Export(TLibrary& lib) {
  gxlib_InitMacro(Grow,
    "s-grow shells vs fragments&;"
    "w-grows the rest of the structure, using already applied generators&;"
    "t-grows only provided atoms/atom types&;"
    "b-grows all visible grow bonds (when in a grow mode)",
    fpAny|psFileLoaded,
    "Grows whole structure or provided atoms only");

  gxlib_InitMacro(Pack,
    "c-specifies if current lattice content should not be deleted",
    fpAny|psFileLoaded,
    "Packs structure within default or given volume(6 or 2 values for "
    "parallelepiped "
    "or 1 for sphere). If atom names/types are provided it only packs the "
    "provided atoms.");

  gxlib_InitMacro(Name,
    "c-enables checking labels for duplications&;"
    "s-simply changes suffix of provided atoms to the provided one (or none)&;"
    "cs-leaves current selection unchanged",
    fpOne|fpTwo,
    "Names atoms. If the 'sel' keyword is used and a number is provided as "
    "second argument the numbering will happen in the order the atoms were "
    "selected (make sure -c option is added)");

  gxlib_InitMacro(CalcPatt, EmptyString(), fpNone|psFileLoaded,
    "Calculates Patterson map");
  gxlib_InitMacro(CalcFourier,
    "fcf-reads structure factors from a fcf file&;"
    "diff-calculates difference map&;"
    "tomc-calculates 2Fo-Fc map&;"
    "obs-calculates observed map&;"
    "calc-calculates calculated map&;"
    "scale-scale to use for difference maps, currently available simple(s) "
    "sum(Fo^2)/sum(Fc^2)) and regression(r)&;"
    "r-resolution in Angstrems&;"
    "i-integrates the map&;"
    "m-mask the structure", fpNone|psFileLoaded,
  "Calculates fourier map");

  gxlib_InitMacro(Qual,
    "h-High&;"
    "m-Medium&;"
    "l-Low", fpNone,
    "Sets drawings quality");

  gxlib_InitMacro(Mask, EmptyString(), fpAny^fpNone, 
    "Sets primitives for atoms or bonds according to provided mask. Accepts "
    "atoms, bonds, hbonds or a name (like from LstGO).\n"
    "Example: 'mask hbonds 2048' - this resets hydrogen bond style to "
    "default");

  gxlib_InitMacro(ARad, EmptyString(), fpAny^fpNone, 
    "Changes how the atoms are drawn [sfil - sphere packing, pers - static "
    "radii, isot - radii proportional to Ueq, isoth - as isot, but applied to "
    "H atoms as well]");
  gxlib_InitMacro(ADS, EmptyString(), fpAny^(fpNone),
    "Changes atom draw style [sph,elp,std]");
  gxlib_InitMacro(AZoom, EmptyString(), fpAny^fpNone,
    "Modifies given atoms [all] radius. The first argument is the new radius "
    "in %");
  gxlib_InitMacro(BRad, "a-specified value is absolute, in A", fpAny^fpNone,
    "Multiplies provided [all] bonds default radius by given number. The"
    " default radius for covalent bonds is 0.1A and for H-bonds is 0.02A. To "
    "set radius for H-bonds use:"
    "\n\tbrad R hbonds"
    "\nAny particula bond type can also be specified like:\n\tbrad 0.5 C-H"
    "\nNote that the heavier atom type is always first"
    );
  gxlib_InitMacro(TelpV, EmptyString(), fpOne,
    "Calculates ADP scale for given thermal probability (in %)");
  gxlib_InitMacro(Labels,
    "p-part&;"
    "l-label&;"
    "v-variables&;"
    "o-occupancy&;"
    "co-chemical occupancy&;"
    "a-afix&;"
    "h-show hydrogen atom labels&;"
    "f-fixed parameters&;"
    "u-Uiso&;"
    "r-Uiso multiplier for riding atoms&;"
    "ao-actual occupancy (as in the ins file)&;"
    "qi-Q peak intensity&;"
    "i-display labels for identity atoms only&;"
    "f-applies given format to the labels like -f=AaBB Aaaa or -f=Aabb etc",
    fpNone,
    "Inverts visibility of atom labels on/off. Look at the options");
  gxlib_InitMacro(Label,
    "type-type of labels to make - subscript, brackers, default&;"
    "symm-symmetry dependent tag type {[$], #, full}&;"
    "cif-creates labels for CIF data a combination of {b,a,t,h}",
    fpAny,
    "Creates moveable labels for provided atoms/bonds/angles (selection)");
  gxlib_InitMacro(ShowH,
    EmptyString(),
    fpNone|fpTwo|psFileLoaded,
    "Changes the H-atom and H-bonds visibility");
  gxlib_InitMacro(Info,
    "s-sorts the atom list&;p-coordinate precision [3]&;f-print fractional "
    "coordinates vs Cartesian [true]&;"
    "c-copy the printed information ito the clipboard",
    fpAny,
    "Prints out information for gived [all] atoms");
  gxlib_InitMacro(ShowQ,
    "wheel-number of peaks to hide (if negative) or to show ",
    fpNone|fpOne|fpTwo|psFileLoaded,
    "Traverses the three states - peaks and peak bonds are visible, only peaks"
    " visible, no peaks or peak bonds. One numeric argument is taken to "
    "increment/decrement the numegbr of visibe peaks. Two aruments are taken "
    "to control the visibility of atoms or bonds, like in: 'showq a true' or "
    "'showq b false'");
  gxlib_InitMacro(Load,
    EmptyString(),
    fpAny^(fpNone),
    "Arguments - textures");
  gxlib_InitMacro(SetView,
    "c-center",
    fpAny,
    "Sets view normal to the normal of the selected plane, to a bond or mean "
    "line");
  gxlib_InitMacro(Matr,
    "r-used reciprocal cell instead",
    fpNone|fpOne|fpTwo|fpThree|fpNine,
    "Displays or sets current orientation matrix. For single argument, 1,2,3 "
    "001, 111, etc values are acceptable, two values taken are of the klm "
    "form, which specify a view from k1*a+l1*b+m1*c to k2*a+l2*b+m2*c, three "
    "values pecify the view normal and nine values provide a full matrix");
  gxlib_InitMacro(Line,
    "n-just sets current view normal to the line without creating the object&;"
    "f-consider input in fractional coordinates vs Cartesian",
    fpAny,
    "Creates a line or best line for provided atoms");
  gxlib_InitMacro(Mpln,
    "n-just orient, do not create plane&;"
    "r-create regular plane&;"
    "we-use weights proportional to the (atomic mass)^we", 
    fpAny,
    "Sets current view along the normal of the best plane");
  gxlib_InitMacro(Cent,
    EmptyString(),
    fpAny,
    "Creates a centroid for given/selected/all atoms");
  gxlib_InitMacro(Cell, "r-shows reciprocal cell",
    fpNone|fpOne|psFileLoaded,
    "If no arguments provided inverts visibility of unit cell, otherwise sets"
    " it to the boolean value of the parameter");
  gxlib_InitMacro(Basis, EmptyString(), fpNone|fpOne,
    "Shows/hides the orientation basis");
  gxlib_InitMacro(Group,
    "n-a custom name can be provided",
    fpNone|fpOne|psFileLoaded,
  "Groups current visible objects or selection");
  gxlib_InitMacro(Fmol, EmptyString(), fpNone|psFileLoaded,
    "Shows all fragments (as opposite to uniq)");
  gxlib_InitMacro(Sel,
    "a-select all&;"
    "u-unselect all&;"
    "l-consider the list of bonds as independent&;"
    "c-copies printed values to the clipboard&;"
    "i-invert selection&;",
    fpAny,
    "If no arguments provided, prints current selection. This includes "
    "distances, angles and torsion angles and other geometrical parameters. "
    "Selects atoms fulfilling provided conditions, like"
    "\nsel $type - selects any particular atom type; type can be one of the "
    "following shortcuts - * - for all atoms, M - for metals, X - for halogens"
    "\nsel $*,type - selects all but type atoms"
    "\n\n"
    "An extended syntax include keyword 'where' and 'rings' which "
    "allow selecting atoms and bonds according to their properties, like type "
    "and length or rings of particular connectivity like C6 or NC5. If the "
    "'where' keyword is used, logical operators, like and (&&), and or (||) "
    "can be used to refine the selection. For example:"
    "\nsel atoms where xatom.bai.z > 2 - to select all atoms heavier after H"
    "\nsel bonds where xbond.length > 2 - to select all bonds longer than 2 A"
    "\nsel bonds where xbond.b.bai.z == 1 - to select all bonds where the "
    "lightest atoms is H"
    );
  gxlib_InitMacro(Uniq, EmptyString(), fpAny | psFileLoaded,
    "Shows only fragments specified by atom name(s) or selection");
  gxlib_InitMacro(PiM,
    "l-display labels for the created lines",
    fpAny|psFileLoaded,
    "Creates an illustration of a pi-system to metal bonds");
  gxlib_InitMacro(ShowP, "m-do not modify the display view", fpAny,
    "Shows specified or all parts of the structure");
  gxlib_InitMacro(Undo, EmptyString(), fpNone,
    "Reverts some of the previous operations");
  gxlib_InitMacro(Esd,
    "label-creates a graphics label&;"
    "l-consider the list of bonds as independent&;"
    "c-copies printed values to the clipboard",
    fpAny|psFileLoaded,
    "This procedure calculates possible parameters for the selection and "
    "evaluates their esd using the variance-covariance matrix coming from the "
    "ShelXL refinement with negative 'MORE' like 'MORE -1' option or from the "
    "olex2.refine");
  gxlib_InitMacro(CalcVoid,
    "d-distance from Van der Waals surface [0]&;r-resolution[0.2]&;"
    "p-precise calculation&;"
    "i-invert the map for rendering",
    fpNone|fpOne|psFileLoaded,
      "Calculates solvent accessible void and packing parameters; optionally "
      "accepts a file with space separated values of Atom Type and radius, an "
      "entry a line");
  gxlib_InitMacro(LstGO,
    EmptyString(),
    fpNone,
    "List current graphical objects");
  gxlib_InitMacro(WBox,
    "w-use atomic mass instead of unit weights for atoms&;"
    "s-create separate boxes for fragments", 
    (fpAny)|psFileLoaded,
    "Calculates wrapping box around provided box using the set of best, "
    "intermidiate and worst planes");
  gxlib_InitMacro(Center,
    "z-also recalculates the scene zoom", 
    (fpAny)|psFileLoaded,
    "Sets the centre of rotation to given point");
  gxlib_InitMacro(ChemDraw,
    EmptyString(),
    fpAny|psFileLoaded,
    "Currently only creates aromatic rings for Ph, Py and Cp rings");
  gxlib_InitMacro(Direction,
    EmptyString(),
    fpNone,
    "Prints current orientation of the model in factional coordinates");
  gxlib_InitMacro(Individualise,
    EmptyString(),
    fpAny,
    "Moves provided atoms to individual collections, so that the atom "
    "properties, such as draw style and appearance can be changed separately "
    "of the group. The first call to this macro creates a group unique to the"
    " asymmetric unit, the second call makes the atom unique to the lattice");
  gxlib_InitMacro(Collectivise,
    EmptyString(),
    fpAny,
    "Does the opposite to the Individialise. If provided atoms are unique to "
    "the lattice a call to this function makes them uniq to the asymmetric "
    "unit, the following call makes the uniq to the element type");
  gxlib_InitMacro(Poly,
    EmptyString(),
    (fpAny^fpNone)|psFileLoaded,
    "Sets polyhedra type for all/selected/given atoms. Last argument specifies"
    " the type - none, auto, regular");
  gxlib_InitMacro(Kill,
    "au-kill atoms in the asymmetric unit (disregarding visibility/"
    "availability). This option is intended for the internal use - the model is"
    " not rebuilt after its execution",
    fpAny,
    "Deletes provided/selected atoms, bonds and other objects. 'kill labels' "
    "hides all atom and bond labels");
  gxlib_InitMacro(Match,
    "s-subgraph match&;"
    "w-use Z as weights&;"
    "n-naming. If the value a symbol [or set of] this is appended to the "
    "label, '$xx' replaces the symbols after the atom type symbol with xx, "
    "leaving the ending, '-xx' - changes the ending of the label with xx&;"
    "a-align&;"
    "i-try inversion&;"
    "u-unmatch&;"
    "esd-calculate esd (works for pairs only)",
    fpNone|fpOne|fpTwo,
    "Fragment matching, alignment and label transfer routine");
  gxlib_InitFunc(ExtraZoom, fpNone|fpOne,
    "Sets/reads current extra zoom (default zoom correction)");
  gxlib_InitFunc(MatchFiles, fpTwo|fpThree,
    "Matches given files");
}
//.............................................................................
void GXLibMacros::macGrow(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if (Options.Contains('b')) {  // grow XGrowBonds only
    app.GrowBonds();
    return;
  }
  bool GrowShells = Options.Contains('s'),
       GrowContent = Options.Contains('w');
  TCAtomPList TemplAtoms;
  if( Options.Contains('t') )
    TemplAtoms = app.FindCAtoms(olxstr(Options['t']).Replace(',', ' '));
  if( Cmds.IsEmpty() )  {  // grow fragments
    if( GrowContent ) 
      app.GrowWhole(TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    else  {
      TXAtomPList atoms;
      app.GrowFragments(GrowShells, TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
      if( !GrowShells )  {
        const TLattice& latt = app.XFile().GetLattice();
        smatd_list gm;
        /* check if next grow will not introduce simple translations */
        bool grow_next = true;
        while( grow_next )  {
          gm.Clear();
          latt.GetGrowMatrices(gm);
          if( gm.IsEmpty() )  break;
          for( size_t i=0; i < latt.MatrixCount(); i++ )  {
            for( size_t j=0; j < gm.Count(); j++ )  {
              if( latt.GetMatrix(i).r == gm[j].r )  {
                const vec3d df = latt.GetMatrix(i).t - gm[j].t;
                if( (df-df.Round<int>()).QLength() < 1e-6 )  {
                  grow_next = false;
                  break;
                }
              }
            }
            if( !grow_next )  break;
          }
          if( grow_next ) {
            app.GrowFragments(GrowShells,
              TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
          }
        }
      }
    }
  }
  else  {  // grow atoms
    if( GrowContent )
      app.GrowWhole(TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    else {
      app.GrowAtoms(Cmds.Text(' '), GrowShells,
        TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    }

  }
}
//.............................................................................
void GXLibMacros::macPack(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  const bool ClearCont = !Options.Contains("c");
  const bool cell = (Cmds.Count() > 0 && Cmds[0].Equalsi("cell"));
  const bool wbox = (Cmds.Count() > 0 && Cmds[0].Equalsi("wbox"));
  if( cell || wbox )
    Cmds.Delete(0);
  const uint64_t st = TETime::msNow();
  if( cell )
    app.XFile().GetLattice().GenerateCell();
  else if( wbox && app.Get3DFrame().IsVisible() )  {
    vec3d_alist norms(6), centres(6);
    for (int i=0; i < 6; i++) {
      norms[i] = app.Get3DFrame().Faces[i].GetN();
      centres[i] = app.Get3DFrame().Faces[i].GetCenter();
    }
    app.XFile().GetLattice().GenerateBox(norms, centres, ClearCont);
  }
  else  {
    vec3d From(-0.5), To(1.5);
    size_t number_count = 0;
    for( size_t i=0; i < Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        if( !(number_count%2) )
          From[number_count/2] = Cmds[i].ToDouble();
        else
          To[number_count/2]= Cmds[i].ToDouble();
        number_count++;
        Cmds.Delete(i--);
      }
    }

    if( number_count != 0 && !(number_count == 6 || number_count == 1 ||
      number_count == 2) )
    {
      Error.ProcessingError(__OlxSrcInfo, "please provide 6, 2 or 1 number");
      return;
    }

    TCAtomPList TemplAtoms;
    if( !Cmds.IsEmpty() )
      TemplAtoms = app.FindCAtoms(Cmds.Text(' '));

    if( number_count == 6 || number_count == 0 || number_count == 2 )  {
      if( number_count == 2 )  {
        From[1] = From[2] = From[0];
        To[1] = To[2] = To[0];
      }
      app.Generate(From, To, TemplAtoms.IsEmpty() ? NULL
        : &TemplAtoms, ClearCont);
    }
    else  {
      TXAtomPList xatoms = app.FindXAtoms(Cmds, true, true);
      vec3d cent;
      double wght = 0;
      for( size_t i=0; i < xatoms.Count(); i++ )  {
        cent += xatoms[i]->crd()*xatoms[i]->CAtom().GetChemOccu();
        wght += xatoms[i]->CAtom().GetChemOccu();
      }
      if( wght != 0 )
        cent /= wght;
      app.Generate(cent, From[0], TemplAtoms.IsEmpty() ? NULL
        : &TemplAtoms, ClearCont);
    }
  }
  if (TBasicApp::GetInstance().IsProfiling()) {
    TBasicApp::NewLogEntry(logInfo) <<
      app.XFile().GetLattice().GetObjects().atoms.Count() <<
      " atoms and " <<
      app.XFile().GetLattice().GetObjects().bonds.Count() <<
      " bonds generated in " <<
      app.XFile().GetLattice().FragmentCount() << " fragments ("
      << (TETime::msNow()-st) << "ms)";
  }
  // optimise drawing ...
  //app.GetRender().Compile(true);
}
//.............................................................................
void GXLibMacros::macName(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  bool checkLabels = Options.Contains("c");
  bool changeSuffix = Options.Contains("s");
  if( changeSuffix )  {
    TXAtomPList xatoms = app.FindXAtoms(Cmds, true, !Options.Contains("cs"));
    if (!xatoms.IsEmpty()) {
      app.GetUndo().Push(app.ChangeSuffix(
        xatoms, Options.FindValue("s"), checkLabels));
    }
  }
  else  {
    bool processed = false;
    if( Cmds.Count() == 1 )  { // bug #49
      const size_t spi = Cmds[0].IndexOf(' ');
      if( spi != InvalidIndex )  {
        app.GetUndo().Push(
          app.Name(Cmds[0].SubStringTo(spi),
          Cmds[0].SubStringFrom(spi+1), checkLabels,
          !Options.Contains("cs"))
        );
      }
      else {
        app.GetUndo().Push(
          app.Name("sel", Cmds[0], checkLabels, !Options.Contains("cs")));
      }
      processed = true;
    }
    else if( Cmds.Count() == 2 )  {
      app.GetUndo().Push(app.Name(
        Cmds[0], Cmds[1], checkLabels, !Options.Contains("cs")));
      processed = true;
    }
    if( !processed )  {
      Error.ProcessingError(__OlxSrcInfo,
        olxstr("invalid syntax: ") << Cmds.Text(' '));
    }
  }
}
//.............................................................................
void GXLibMacros::macQual(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if( Options.IsEmpty() )
    Error.ProcessingError(__OlxSrcInfo, "wrong number of arguments");
  else  {
    int32_t v;
     if( Options.GetName(0)[0] == 'h' ) v = qaHigh;
     else if( Options.GetName(0)[0] == 'm' ) v = qaMedium;
     else if( Options.GetName(0)[0] == 'l' ) v = qaLow;
     else {
       Error.ProcessingError(__OlxSrcInfo, "wrong argument");
       return;
     }
    app.Quality(v);
  }
}
//.............................................................................
void GXLibMacros::macCalcFourier(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TStopWatch st(__FUNC__);
  double resolution = Options.FindValue("r", "0.25").ToDouble(),
    maskInc = 1.0;
  if( resolution < 0.1 )  resolution = 0.1;
  resolution = 1./resolution;
  short mapType = SFUtil::mapTypeCalc;
  if( Options.Contains("tomc") )
    mapType = SFUtil::mapType2OmC;
  else if( Options.Contains("obs") )
    mapType = SFUtil::mapTypeObs;
  else if( Options.Contains("diff") )
    mapType = SFUtil::mapTypeDiff;
  olxstr strMaskInc = Options.FindValue("m");
  if( !strMaskInc.IsEmpty() )
    maskInc = strMaskInc.ToDouble();
  TRefList refs;
  TArrayList<compd> F;
  st.start("Obtaining structure factors");
  olxstr err = SFUtil::GetSF(refs, F, mapType,
    Options.Contains("fcf") ? SFUtil::sfOriginFcf : SFUtil::sfOriginOlex2,
    (Options.FindValue("scale", "r").ToLowerCase().CharAt(0) == 'r') ?
      SFUtil::scaleRegression : SFUtil::scaleSimple);
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  TUnitCell& uc = app.XFile().GetUnitCell();
  TArrayList<SFUtil::StructureFactor> P1SF;
  st.start("Expanding SF to P1");
  SFUtil::ExpandToP1(refs, F, uc.GetMatrixList(), P1SF);
  const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
  BVFourier::MapInfo mi;
// init map
  const vec3i dim(au.GetAxes()*resolution);
  TArray3D<float> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  st.start("Calcuating ED map");
  mi = BVFourier::CalcEDM(P1SF, map.Data, dim, vol);
///////////////////////////////////////////////////////////////////////////////
  st.start("Map operations");
  app.XGrid().InitGrid(dim);

  app.XGrid().SetMaxHole(mi.sigma*1.4);
  app.XGrid().SetMinHole(-mi.sigma*1.4);
  app.XGrid().SetScale(-mi.sigma*6);
  //app.XGrid().SetScale( -(mi.maxVal - mi.minVal)/2.5 );
  app.XGrid().SetMinVal(mi.minVal);
  app.XGrid().SetMaxVal(mi.maxVal);
  // copy map
  MapUtil::CopyMap(app.XGrid().Data()->Data, map.Data, dim);
  app.XGrid().AdjustMap();

  TBasicApp::NewLogEntry() <<
    "Map max val " << olxstr::FormatFloat(3, mi.maxVal) <<
    " min val " << olxstr::FormatFloat(3, mi.minVal) << NewLineSequence() <<
    "Map sigma " << olxstr::FormatFloat(3, mi.sigma);
  // map integration
  if( Options.Contains('i') )  {
    TArrayList<MapUtil::peak> Peaks;
    TTypeList<MapUtil::peak> MergedPeaks;
    vec3d norm(1./dim[0], 1./dim[1], 1./dim[2]);
    MapUtil::Integrate<float>(map.Data, dim, mi.sigma*6, Peaks);
    MapUtil::MergePeaks(uc.GetSymmSpace(), norm, Peaks, MergedPeaks);
    QuickSorter::SortSF(MergedPeaks, MapUtil::PeakSortBySum);
    for( size_t i=0; i < MergedPeaks.Count(); i++ )  {
      const MapUtil::peak& peak = MergedPeaks[i];
      if( peak.count == 0 )  continue;
      vec3d cnt((double)peak.center[0]/dim[0], (double)peak.center[1]/dim[1],
        (double)peak.center[2]/dim[2]); 
      const double ed = (double)((long)((peak.summ*1000)/peak.count))/1000;
      TCAtom& ca = au.NewAtom();
      ca.SetLabel(olxstr("Q") << olxstr((100+i)));
      ca.ccrd() = cnt;
      ca.SetQPeak(ed);
    }
    au.InitData();
    TActionQueue* q_draw = app.FindActionQueue(olxappevent_GL_DRAW);
    bool q_draw_changed = false;
    if( q_draw != NULL )  {
      q_draw->SetEnabled(false);
      q_draw_changed = true;
    }
    app.XFile().GetLattice().Init();
    olex::IOlexProcessor::GetInstance()->processMacro("compaq -q");
    if( q_draw != NULL && q_draw_changed )
      q_draw->SetEnabled(true);
  }  // integration
  if( Options.Contains("m") )  {
    FractMask* fm = new FractMask;
    app.BuildSceneMask(*fm, maskInc);
    app.XGrid().SetMask(*fm);
  }
  app.XGrid().InitIso();
  //TStateChange sc(prsGridVis, true);
  app.ShowGrid(true, EmptyString());
  //OnStateChange.Execute((AEventsDispatcher*)this, &sc);
}
//.............................................................................
void GXLibMacros::macCalcPatt(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  // space group matrix list
  TUnitCell::SymmSpace sp = app.XFile().GetUnitCell().GetSymmSpace();
  olxstr hklFileName = app.LocateHklFile();
  if( !TEFile::Exists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  TRefList refs;
  app.XFile().GetRM().GetFourierRefList<
      TUnitCell::SymmSpace,RefMerger::StandardMerger>(sp, refs);
  const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
  TArrayList<SFUtil::StructureFactor> P1SF(refs.Count()*sp.Count());
  size_t index = 0;
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    double sI = sqrt(refs[i].GetI() < 0 ? 0 : refs[i].GetI());
    for( size_t j=0; j < sp.Count(); j++, index++ )  {
      P1SF[index].hkl = ref * sp[j];
      P1SF[index].ps = sp[j].t.DotProd(ref.GetHkl());
      P1SF[index].val = sI;
      P1SF[index].val *= compd::polar(1, 2*M_PI*P1SF[index].ps);
    }
  }
  const double resolution = 5;
  const vec3i dim(au.GetAxes()*resolution);
  app.XGrid().InitGrid(dim);
  BVFourier::MapInfo mi = BVFourier::CalcPatt(
    P1SF, app.XGrid().Data()->Data, dim, vol);
  app.XGrid().AdjustMap();
  app.XGrid().SetMinVal(mi.minVal);
  app.XGrid().SetMaxVal(mi.maxVal);
  app.XGrid().SetMaxHole( mi.sigma*1.4);
  app.XGrid().SetMinHole(-mi.sigma*1.4);
  app.XGrid().SetScale( -(mi.maxVal - mi.minVal)/2.5);
  app.XGrid().InitIso();
  app.ShowGrid(true, EmptyString());
}
//.............................................................................
void GXLibMacros::macMask(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if (Cmds[0].Equalsi("atoms") && Cmds.Count() > 1) {
    int Mask = Cmds[1].ToInt();
    short AtomsStart=2;
    TXAtomPList Atoms =
      app.FindXAtoms(TStrObjList(Cmds.SubListFrom(AtomsStart)), false, false);
    app.UpdateAtomPrimitives(Mask, Atoms.IsEmpty() ? NULL : &Atoms);
  }
  else if( (Cmds[0].Equalsi("bonds") || Cmds[0].Equalsi("hbonds")) &&
           Cmds.Count() > 1 )
  {
    int Mask = Cmds[1].ToInt();
    TXBondPList Bonds = app.GetBonds(TStrList(Cmds.SubListFrom(2)), false);
    app.UpdateBondPrimitives(Mask, 
      (Bonds.IsEmpty() && app.GetSelection().Count() == 0) ? NULL : &Bonds,
      Cmds[0].Equalsi("hbonds"));
  }
  else  {
    int Mask = Cmds.GetLastString().ToInt();
    Cmds.Delete(Cmds.Count() - 1);
    TGPCollection *GPC = app.GetRender().FindCollection(Cmds.Text(' '));
    if( GPC != NULL )  {
      if( GPC->ObjectCount() != 0 )
        GPC->GetObject(0).UpdatePrimitives(Mask);
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo,"Undefined graphics:").quote() <<
        Cmds.Text(' ');
      return;
    }
  }
}
//.............................................................................
void GXLibMacros::macARad(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  olxstr arad(Cmds[0]);
  Cmds.Delete(0);
  TXAtomPList xatoms = app.FindXAtoms(Cmds, false, false);
  app.AtomRad(arad, xatoms.IsEmpty() ? NULL : &xatoms);
}
//.............................................................................
void GXLibMacros::macADS(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  short ads = -1;
  if( Cmds[0].Equalsi("elp") )
    ads = adsEllipsoid;
  else if( Cmds[0].Equalsi("sph") )
    ads = adsSphere;
  else if( Cmds[0].Equalsi("ort") )
    ads = adsOrtep;
  else if( Cmds[0].Equalsi("std") )
    ads = adsStandalone;
  if( ads == -1 )  {
    Error.ProcessingError(__OlxSrcInfo,
      "unknown atom type (elp/sph/ort/std) supported only");
    return;
  }
  Cmds.Delete(0);
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, false);
  app.SetAtomDrawingStyle(ads, Atoms.IsEmpty() ? NULL : &Atoms);
}
//.............................................................................
void GXLibMacros::macAZoom(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "a number is expected as first argument");
    return;
  }
  double zoom = Cmds[0].ToDouble();
  Cmds.Delete(0);
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, false);
  app.AtomZoom(zoom, Atoms.IsEmpty() ? NULL : &Atoms);
}
//.............................................................................
void GXLibMacros::macBRad(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  double r = Cmds[0].ToDouble();
  Cmds.Delete(0);
  TXBondPList bonds;
  bool absolute = Options.GetBoolOption('a');
  if( Cmds.Count() == 1 && Cmds[0].Equalsi("hbonds") )  {
    if (absolute) r /= 0.02;
    TGXApp::BondIterator bi = app.GetBonds();
    while( bi.HasNext() )  {
      TXBond& xb = bi.Next();
      if( xb.GetType() == sotHBond )
        bonds.Add(xb);
    }
    app.BondRad(r, &bonds);
  }
  else  {
    if (absolute) r /= 0.1;
    bonds = app.GetBonds(Cmds, true);
    if( bonds.IsEmpty() && Cmds.IsEmpty() )  {  // get all non-H
      TGXApp::BondIterator bi = app.GetBonds();
      while( bi.HasNext() )  {
        TXBond& xb = bi.Next();
        if( xb.GetType() != sotHBond )
          bonds.Add(xb);
      }
      TXBond::DefR(r);
    }
    app.BondRad(r, &bonds);
  }
}
//.............................................................................
void GXLibMacros::macTelpV(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  app.CalcProbFactor(Cmds[0].ToDouble());
}
//.............................................................................
void GXLibMacros::macInfo(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TStrList Output;
  app.InfoList(Cmds.IsEmpty() ? EmptyString() : Cmds.Text(' '),
    Output, Options.Contains("p"),
    Options.FindValue('p', "-3").ToInt(),
    Options.GetBoolOption('f')
  );
  TBasicApp::NewLogEntry() << Output;
  if (Options.Contains('c'))
    app.ToClipboard(Output);
}
//.............................................................................
void GXLibMacros::macLabels(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  short lmode = 0;
  if (Options.Contains('p'))   lmode |= lmPart;
  if (Options.Contains('l'))   lmode |= lmLabels;
  if (Options.Contains('v'))   lmode |= lmOVar;
  if (Options.Contains('o'))   lmode |= lmOccp;
  if (Options.Contains("ao"))  lmode |= lmAOcc;
  if (Options.Contains('u'))   lmode |= lmUiso;
  if (Options.Contains('r'))   lmode |= lmUisR;
  if (Options.Contains('a'))   lmode |= lmAfix;
  if (Options.Contains('h'))   lmode |= lmHydr;
  if (Options.Contains('f'))   lmode |= lmFixed;
  if (Options.Contains("qi"))  lmode |= lmQPeakI;
  if (Options.Contains('i'))   lmode |= lmIdentity;
  if (Options.Contains("co"))  lmode |= lmCOccu;
  if( lmode == 0 )  {
    lmode |= lmLabels;
    lmode |= lmQPeak;
    app.SetLabelsMode(lmode);
    if (Cmds.Count() == 1 && Cmds[0].IsBool())
      app.SetLabelsVisible(Cmds[0].ToBool());
    else
      app.SetLabelsVisible(!app.AreLabelsVisible());
  }
  else  {
    app.SetLabelsMode(lmode |= lmQPeak);
    app.SetLabelsVisible(true);
  }
  TStateRegistry::GetInstance().SetState(app.stateLabelsVisible,
    app.AreLabelsVisible(), EmptyString(), true);
}
//.............................................................................
void GXLibMacros::macLabel(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
  {
  TXAtomPList atoms;
  TXBondPList bonds;
  TPtrList<TXLine> lines;
  if( Cmds.IsEmpty() )  {
    TGlGroup& sel = app.GetSelection();
    for( size_t i=0; i < sel.Count(); i++)  {
      if( EsdlInstanceOf(sel[i], TXAtom) )
        atoms.Add((TXAtom&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXBond) )
        bonds.Add((TXBond&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXLine) )
        lines.Add((TXLine&)sel[i]);
    }
    if( atoms.IsEmpty() && bonds.IsEmpty() && lines.IsEmpty() )  {
      TGXApp::AtomIterator ai = app.GetAtoms();
      atoms.SetCapacity(ai.count);
      while( ai.HasNext() )  {
        TXAtom& xa = ai.Next();
        if( xa.IsVisible() )
          atoms.Add(xa);
      }
      TGXApp::BondIterator bi = app.GetBonds();
      bonds.SetCapacity(bi.count);
      while( bi.HasNext() )  {
        TXBond& xb = bi.Next();
        if( xb.IsVisible() )
          bonds.Add(xb);
      }
    }
  }
  else
    atoms = app.FindXAtoms(Cmds, true, false);
  short lt = 0, symm_tag = 0;
  const olxstr str_lt = Options.FindValue("type");
  olxstr str_symm_tag = Options.FindValue("symm");
  // enforce the default
  if( str_lt.Equalsi("brackets") )
    lt = 1;
  else if( str_lt.Equalsi("subscript") )
    lt = 2;
  // have to kill labels in this case, for consistency of _$ or ^#
  if( str_symm_tag =='$' || str_symm_tag == '#' )  {
    for( size_t i=0; i < app.LabelCount(); i++ )
      app.GetLabel(i).SetVisible(false);
    symm_tag = (str_symm_tag =='$' ? 1 : 2);
  }
  else if( str_symm_tag.Equals("full") )
    symm_tag = 3;
  TTypeList<uint32_t> equivs;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    TXGlLabel& gxl = atoms[i]->GetGlLabel();
    olxstr lb;
    if( lt != 0 &&
        atoms[i]->GetLabel().Length() > atoms[i]->GetType().symbol.Length() )
    {
      olxstr bcc = atoms[i]->GetLabel().SubStringFrom(
        atoms[i]->GetType().symbol.Length());
      lb = atoms[i]->GetType().symbol;
      if( lt == 1 )
        lb << '(' << bcc << ')';
      else if( lt == 2 )
        lb << "\\-" << bcc;
    }
    else
      lb = atoms[i]->GetLabel();
    if( !atoms[i]->IsAUAtom() )  {
      if( symm_tag == 1 || symm_tag == 2 )  {
        size_t pos = equivs.IndexOf(atoms[i]->GetMatrix().GetId());
        if( pos == InvalidIndex )  {
          equivs.AddCopy(atoms[i]->GetMatrix().GetId());
          pos = equivs.Count()-1;
        }
        if( symm_tag == 1 )
          lb << "_$" << (pos+1);
        else
          lb << "\\+" << (pos+1);
      }
      else if( symm_tag == 3 )
        lb << ' ' << TSymmParser::MatrixToSymmEx(atoms[i]->GetMatrix());
    }
    gxl.SetOffset(atoms[i]->crd());
    gxl.SetLabel(lb);
    gxl.SetVisible(true);
  }
  TPtrList<TXGlLabel> labels;
  if( !bonds.IsEmpty() )  {
    VcoVContainer vcovc(app.XFile().GetAsymmUnit());
    bool have_vcov = false;
    try  {
      olxstr src_mat = app.InitVcoV(vcovc);
      app.NewLogEntry() << "Using " << src_mat <<
        " matrix for the calculation";
      have_vcov = true;
    }
    catch(const TExceptionBase&)  {}
    for( size_t i=0; i < bonds.Count(); i++ )  {
      TXGlLabel& l = bonds[i]->GetGlLabel();
      l.SetOffset(bonds[i]->GetCenter());
      if( have_vcov ) {
        l.SetLabel(vcovc.CalcDistance(bonds[i]->A(),
          bonds[i]->B()).ToString());
      }
      else
        l.SetLabel(olxstr::FormatFloat(3, bonds[i]->Length()));
      labels.Add(l)->SetVisible(true);
      l.TranslateBasis(-l.GetCenter());
    }
  }
  for( size_t i=0; i < lines.Count(); i++ )
    lines[i]->GetGlLabel().SetVisible(true);

  for( size_t i=0; i < equivs.Count(); i++ )  {
    smatd m = app.XFile().GetUnitCell().GetMatrix(
      smatd::GetContainerId(equivs[i]));
    m.t += smatd::GetT(equivs[i]);
    olxstr line("$");
    line << (i+1);
    line.RightPadding(4, ' ', true) << TSymmParser::MatrixToSymmEx(m);
    if( i != 0 && (i%3) == 0 )
      TBasicApp::GetLog() << NewLineSequence();
    TBasicApp::GetLog() << line.RightPadding(26, ' ');
  }
  TBasicApp::GetLog() << NewLineSequence();

  const olxstr _cif = Options.FindValue("cif");
  if( !_cif.IsEmpty() )  {
    if( app.CheckFileType<TCif>() )  {
      const TCifDataManager& cifdn =
        app.XFile().GetLastLoader<TCif>().GetDataManager();
      const TGlGroup& sel = app.GetSelection();
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXBond) )  {
          TXBond& xb = (TXBond&)sel[i];
          ACifValue* v = cifdn.Match(xb.A(), xb.B());
          if( v == NULL )  continue;
          TXGlLabel& l = xb.GetGlLabel();
          l.SetOffset(xb.GetCenter());
          l.SetLabel(v->GetValue().ToString());
          labels.Add(l);
        }
      }
    }
    else  {
      const TGlGroup& sel = app.GetSelection();
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXBond) )  {
          TXBond& xb = (TXBond&)sel[i];
          TXGlLabel& l = xb.GetGlLabel();
          l.SetOffset(xb.GetCenter());
          l.SetLabel(olxstr::FormatFloat(2, xb.Length()));
          labels.Add(l);
        }
      }
    }
  }
  for( size_t i=0; i < labels.Count(); i++ )  {
    TXGlLabel& l = *labels[i];
    vec3d off(-l.GetRect().width/2, -l.GetRect().height/2, 0);
    const double scale1 =
      l.GetFont().IsVectorFont() ? 1.0/app.GetRender().GetScale() : 1.0;
    const double scale = scale1/app.GetRender().GetBasis().GetZoom();
    l.TranslateBasis(off*scale);
    l.SetVisible(true);
  }
  app.SelectAll(false);
}
//.............................................................................
void GXLibMacros::macShowH(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TEBasis basis = app.GetRender().GetBasis();
  if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == 'a' )  {
      if( v && !app.AreHydrogensVisible() )  {
        app.SetHydrogensVisible(true);
      }
      else if( !v && app.AreHydrogensVisible() )  {
        app.SetHydrogensVisible(false);
      }
    }
    else if( Cmds[0] == 'b' )  {
      if( v && !app.AreHBondsVisible() )  {
        app.SetHBondsVisible(true);
      }
      else if( !v && app.AreHBondsVisible() )  {
        app.SetHBondsVisible(false);
      }
    }
  }
  else  {
    if( app.AreHydrogensVisible() && !app.AreHBondsVisible() )  {
      app.SetHBondsVisible(true);
    }
    else if( app.AreHydrogensVisible() && app.AreHBondsVisible() )  {
      app.SetHBondsVisible(false);
      app.SetHydrogensVisible(false);
    }
    else if( !app.AreHydrogensVisible() && !app.AreHBondsVisible() )  {
      app.SetHydrogensVisible(true);
    }
  }
  TStateRegistry::GetInstance().SetState(app.stateHydrogensVisible,
    app.AreHydrogensVisible(), EmptyString(), true);
  TStateRegistry::GetInstance().SetState(app.stateHydrogenBondsVisible,
    app.AreHBondsVisible(), EmptyString(), true);
  app.GetRender().SetBasis(basis);
}
//.............................................................................
int GXLibMacros::QPeakSortA(const TCAtom &a, const TCAtom &b)  {
  double v = a.GetQPeak() - b.GetQPeak();
  if (v == 0 && a.GetLabel().Length() > 1 && b.GetLabel().Length() > 1) {
    if (a.GetLabel().SubStringFrom(1).IsNumber() &&
        b.GetLabel().SubStringFrom(1).IsNumber())
    {
      v = b.GetLabel().SubStringFrom(1).ToInt() -
        a.GetLabel().SubStringFrom(1).ToInt();
    }
  }
  return v < 0 ? 1 : (v > 0 ? -1 : 0);
}
void GXLibMacros::macShowQ(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  double wheel = Options.FindValue("wheel", '0').ToDouble();
  TEBasis basis = app.GetRender().GetBasis();
  if( wheel != 0 )  {
    //if( !app.QPeaksVisible() )  return;
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for( size_t i=0; i < au.AtomCount(); i++ )
      if( !au.GetAtom(i).IsDeleted() && au.GetAtom(i).GetType() == iQPeakZ )
        qpeaks.Add(au.GetAtom(i));
    QuickSorter::SortSF(qpeaks, GXLibMacros::QPeakSortA);
    index_t d_cnt = 0;
    for( size_t i=0; i < qpeaks.Count(); i++ )
      if( !qpeaks[i]->IsDetached() )
        d_cnt++;
    if( d_cnt == 0 && wheel < 0 )  return;
    if( d_cnt == (index_t)qpeaks.Count() && wheel > 0 )  return;
    d_cnt += (index_t)(wheel);
    if( d_cnt < 0 )  d_cnt = 0;
    if( d_cnt > (index_t)qpeaks.Count() )
      d_cnt = qpeaks.Count();
    for( size_t i=0; i < qpeaks.Count(); i++ )
      qpeaks[i]->SetDetached(i >= (size_t)d_cnt);
    //app.XFile().GetLattice().UpdateConnectivityInfo();
    app.UpdateConnectivity();
    app.Draw();
  }
  else if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == "a" )  {
      if( v && !app.AreQPeaksVisible() )  {
        app.SetQPeaksVisible(true);
      }
      else if( !v && app.AreQPeaksVisible() )  {
        app.SetQPeaksVisible(false);
      }
    }
    else if( Cmds[0] == "b" )  {
      if( v && !app.AreQPeakBondsVisible() )  {
        app.SetQPeakBondsVisible(true);
      }
      else if( !v && app.AreQPeakBondsVisible() )  {
        app.SetQPeakBondsVisible(false);
      }
    }
  }
  else if( Cmds.Count() == 1 && Cmds[0].IsNumber() )  {
    index_t num = Cmds[0].ToInt();
    const bool negative = num < 0;
    if( num < 0 )  num = olx_abs(num);
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for( size_t i=0; i < au.AtomCount(); i++ )
      if( au.GetAtom(i).GetType() == iQPeakZ )
        qpeaks.Add(au.GetAtom(i));
    QuickSorter::SortSF(qpeaks,
      negative ? GXLibMacros::QPeakSortD : GXLibMacros::QPeakSortA);
    num = olx_min(qpeaks.Count()*num/100, qpeaks.Count());
    for( size_t i=0; i < qpeaks.Count(); i++ )  
      qpeaks[i]->SetDetached( i >= (size_t)num );
    app.GetSelection().Clear();
    app.UpdateConnectivity();
    app.Draw();
  }
  else  {
    if( (!app.AreQPeaksVisible() && !app.AreQPeakBondsVisible()) )  {
      app.SetQPeaksVisible(true);
    }
    else if( app.AreQPeaksVisible() && !app.AreQPeakBondsVisible())  {
      app.SetQPeakBondsVisible(true);
    }
    else if( app.AreQPeaksVisible() && app.AreQPeakBondsVisible() )  {
      app.SetQPeaksVisible(false);
      app.SetQPeakBondsVisible(false);
    }
  }
  TStateRegistry::GetInstance().SetState(app.stateQPeaksVisible,
    app.AreQPeaksVisible(), EmptyString(), true);
  TStateRegistry::GetInstance().SetState(app.stateQPeakBondsVisible,
    app.AreQPeakBondsVisible(), EmptyString(), true);
  app.GetRender().SetBasis(basis);
}
//.............................................................................
void GXLibMacros::macLoad(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if (Cmds.IsEmpty() || !Cmds[0].Equalsi("textures")) {
    Error.SetUnhandled(true);
    return;
  }
  Cmds.Delete(0);
  if (Cmds.IsEmpty())
    app.ClearTextures(~0);
  else
    app.LoadTextures(Cmds.Text(' '));
}
//.............................................................................
void GXLibMacros::macMatr(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if (Cmds.IsEmpty()) {
    TBasicApp::NewLogEntry() << "Current orientation matrix:";
    const mat3d& Matr = app.GetRender().GetBasis().GetMatrix();
    for (size_t i=0; i < 3; i++) {
      olxstr Tmp;
      for (size_t j=0; j < 3; j++) {
        Tmp << olxstr::FormatFloat(4, Matr[j][i]);
        Tmp.RightPadding(7*(j+1), ' ', true);
      }
      TBasicApp::NewLogEntry() << Tmp;
    }
  }
  else {
    const mat3d& M = Options.Contains('r') ?
      app.XFile().GetAsymmUnit().GetHklToCartesian()
      : app.XFile().GetAsymmUnit().GetCellToCartesian();
    if (Cmds.Count() == 1) {
      olxstr arg;
      if (Cmds[0] == '1' || Cmds[0] == 'a')
        arg = "100";
      else if (Cmds[0] == '2' || Cmds[0] == 'b')
        arg = "010";
      else if (Cmds[0] == '3' || Cmds[0] == 'c')
        arg = "001";
      else
        arg = Cmds[0];
      if ((arg.Length()%3) != 0 ) {
        Error.ProcessingError(__OlxSrcInfo,
          "invalid argument, an arguments like 010, 001000, +0-1+1 etc is"
          " expected");
        return;
      }
      vec3d n;
      const size_t s = arg.Length()/3;
      for (int i=0; i < 3; i++)
        n += M[i]*arg.SubString(s*i, s).ToInt();
      if (n.QLength() < 1e-3) {
        Error.ProcessingError(__OlxSrcInfo,
          "non zero expression is expected");
        return;
      }
      app.GetRender().GetBasis().OrientNormal(n);
    }
    else if (Cmds.Count() == 2) {  // from to view
      if ((Cmds[0].Length()%3) != 0 || (Cmds[1].Length()%3) != 0) {
        Error.ProcessingError(__OlxSrcInfo,
          "invalid arguments, a klm, two arguments like 010, 001000, +0-1+1 "
          "etc are expected");
        return;
      }
      vec3d from, to;
      const size_t fs = Cmds[0].Length()/3, ts = Cmds[1].Length()/3;
      for (int i=0; i < 3; i++) {
        from += M[i]*Cmds[0].SubString(fs*i, fs).ToInt();
        to += M[i]*Cmds[1].SubString(ts*i, ts).ToInt();
      }
      vec3d n = from-to;
      if (n.QLength() < 1e-3 ) {
        Error.ProcessingError(__OlxSrcInfo,
          "from and to arguments must be different");
        return;
      }
      app.GetRender().GetBasis().OrientNormal(n);
    }
    else if (Cmds.Count() == 3) {  // view along
      vec3d n = M[0]*Cmds[0].ToDouble() +
        M[1]*Cmds[1].ToDouble() +
        M[2]*Cmds[2].ToDouble();
      if (n.IsNull(1e-6)) {
        Error.ProcessingError(__OlxSrcInfo,
          "a non-singular direction is expected");
        return;
      }
      app.GetRender().GetBasis().OrientNormal(n);
    }
    else if (Cmds.Count() == 9) {
      mat3d M(
        Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble(),
        Cmds[3].ToDouble(), Cmds[4].ToDouble(), Cmds[5].ToDouble(),
        Cmds[6].ToDouble(), Cmds[7].ToDouble(), Cmds[8].ToDouble());
      M.Transpose();
      app.GetRender().GetBasis().SetMatrix(M.Normalise());
    }
    app.Draw();
  }
}
//.............................................................................
void GXLibMacros::macSetView(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  const bool do_center = Options.Contains('c');
  bool process = false, has_center = true;
  vec3d center, normal;
  if (Cmds.IsEmpty()) {
    TGlGroup& g = app.GetSelection();
    if (g.Count() == 1) {
      if (EsdlInstanceOf(g[0], TXPlane)) {
        TXPlane& xp = (TXPlane&)g[0];
        normal = xp.GetNormal();
        center = xp.GetCenter();
        process = true;
      }
      else if (EsdlInstanceOf(g[0], TXBond)) {
        TXBond& xb = (TXBond&)g[0];
        normal = vec3d(xb.B().crd()-xb.A().crd()).Normalise();
        center = (xb.B().crd()+xb.A().crd())/2;
        process = true;
      }
    }
  }
  else  {
    if (Cmds.Count() == 3) {
      for (int i=0; i < 3; i++)
        normal[i] = Cmds[i].ToDouble();
      normal.Normalise();
      process = true;
      has_center = false;
    }
    else if (Cmds.Count() == 6) {
      for (int i=0; i < 3; i++) {
        normal[i] = Cmds[i].ToDouble();
        center[i] = Cmds[3+i].ToDouble();
      }
      normal.Normalise();
      process = true;
    }
  }
  if (!process) {
    TXAtomPList xatoms = app.FindXAtoms(Cmds, true, false);
    if( xatoms.Count() < 2 )  {
      Error.ProcessingError(__OlxSrcInfo, "At least two atoms are required");
      return;
    }
    process = true;
    if (xatoms.Count() == 2) {
      center = (xatoms[0]->crd()+xatoms[1]->crd())/2;
      normal = (xatoms[1]->crd()-xatoms[0]->crd()).Normalise();
    }
    else {
      TSAtomPList satoms(xatoms, StaticCastAccessor<TSAtom>());
      mat3d params;
      vec3d rms;
      TSPlane::CalcPlanes(satoms, params, rms, center);
      normal = params[2];
    }
  }
  if (process) {
    app.GetRender().GetBasis().OrientNormal(normal);
    if (do_center && has_center)
      app.GetRender().GetBasis().SetCenter(-center);
    if (app.XGrid().IsVisible() && has_center)
      app.SetGridDepth(center);
    app.Draw();
  }
}
//.............................................................................
void GXLibMacros::macLine(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  bool process_atoms = true;
  vec3d from, to;
  if (Cmds.IsEmpty() || (Cmds.Count() == 1 && Cmds[0].Equalsi("sel"))) {
    TGlGroup& sel = app.GetSelection();
    if( sel.Count() == 2 )  {
      if (EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXPlane)) {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if (EsdlInstanceOf(sel[0], TXAtom) &&
        EsdlInstanceOf(sel[1], TXPlane))
      {
        from = ((TXAtom&)sel[0]).crd();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if (EsdlInstanceOf(sel[0], TXPlane) &&
        EsdlInstanceOf(sel[1], TXAtom))
      {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXAtom&)sel[1]).crd();
        process_atoms = false;
      }
    }
  }
  if (Cmds.Count() == 6 && olx_list_and(Cmds, &olxstr::IsNumber)) {
    TDoubleList a = TDoubleList::FromList(Cmds,
      FunctionAccessor::MakeConst(&olxstr::ToDouble));
    from = vec3d(a[0], a[1], a[2]);
    to = vec3d(a[3], a[4], a[5]);
    if (Options.GetBoolOption('f')) {
      from = app.XFile().GetAsymmUnit().Orthogonalise(from);
      to = app.XFile().GetAsymmUnit().Orthogonalise(to);
    }
    process_atoms = false;
  }
  TXAtomPList Atoms;
  if (process_atoms)
    Atoms = app.FindXAtoms(Cmds, true, true);
  if (Atoms.Count() > 2) {
    TSAtomPList satoms(Atoms, StaticCastAccessor<TSAtom>());
    mat3d params;
    vec3d rms, center;
    TSPlane::CalcPlanes(satoms, params, rms, center);
    double maxl = -1000, minl = 1000;
    for( size_t i=0; i < satoms.Count(); i++ )  {
      vec3d v = satoms[i]->crd() - center;
      if (v.QLength() < 0.0001) continue;
      const double ca = params[2].CAngle(v);
      const double l = v.Length()*ca;
      if (l > maxl) maxl = l;
      if (l < minl) minl = l;
    }
    from = center+params[2]*minl;
    to = center+params[2]*maxl;
  }
  else if (Atoms.Count() == 2) {
    from = Atoms[0]->crd();
    to = Atoms[1]->crd();
  }
  else if (process_atoms) {
    Error.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
    return;
  }
  olxstr name = Options.FindValue('n');
  if (Options.Contains('n') && name.IsEmpty())
    app.GetRender().GetBasis().OrientNormal(to-from);
  else
    app.AddLine(name, from, to);
  app.Draw();
}
//.............................................................................
void GXLibMacros::macMpln(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TSPlane* plane = NULL;
  bool orientOnly = Options.Contains('n'),
    rectangular = Options.Contains('r');
  olxstr name = Options.FindValue('n');
  const double weightExtent = olx_abs(Options.FindValue("we", "0").ToDouble());
  olxstr planeName;
  TXAtomPList Atoms = app.FindXAtoms(Cmds, true, true);
  for (size_t i=0; i < Atoms.Count(); i++ ) {
    planeName << Atoms[i]->GetLabel();
    if( i+1 < Atoms.Count() )
      planeName << ' ';
  }

  if (Atoms.Count() < 3) {
    Error.ProcessingError(__OlxSrcInfo, "at least 3 atoms are expected");
    return;
  }
  if (orientOnly && name.IsEmpty()) {
    plane = app.TmpPlane(&Atoms, weightExtent);
    if( plane != NULL )  {
      mat3d m = plane->GetBasis();
      vec3d d1 = (m[2]+m[1]).Normalise();
      vec3d d2 = m[0].XProdVec(d1).Normalise();
      app.GetRender().GetBasis().Orient(d1, d2, m[0]);
      app.SetGridDepth(plane->GetCenter());
      delete plane;
      plane = NULL;
    }
  }
  else  {
    TXPlane* xp = app.AddPlane(name, Atoms, rectangular, weightExtent);
    if (xp != NULL)
      plane = xp;
  }
  if (plane != NULL) {
    const TAsymmUnit& au = app.XFile().GetAsymmUnit();
    size_t colCount = 3;
    TTTable<TStrList> tab(plane->Count()/colCount +
      (((plane->Count()%colCount)==0)?0:1), colCount*3);
    for (size_t i=0; i < colCount; i++) {
      tab.ColName(i*3) = "Label";
      tab.ColName(i*3+1) = "D/A";
    }
    double rmsd = 0;
    for (size_t i=0; i < plane->Count(); i+=colCount) {
      for (size_t j=0; j < colCount; j++) {
        if ((i + j) >= Atoms.Count())
          break;
        tab[i/colCount][j*3] = plane->GetAtom(i+j).GetLabel();
        const double v = plane->DistanceTo(plane->GetAtom(i+j).crd()); 
        rmsd += v*v;
        tab[i/colCount][j*3+1] = olxstr::FormatFloat(3, v);
      }
    }
    rmsd = sqrt(rmsd/Atoms.Count());
    TBasicApp::NewLogEntry() <<
      tab.CreateTXTList(olxstr("Atom-to-plane distances for ") << planeName,
      true, false, " | ");
    TBasicApp::NewLogEntry() << "Plane equation: " << plane->StrRepr();
    TBasicApp::NewLogEntry() << "HKL direction: " <<
      plane->GetCrystallographicDirection().ToString();
    if (weightExtent != 0) {
      TBasicApp::NewLogEntry() << "Weighted RMSD/A: " <<
        olxstr::FormatFloat(3, plane->GetWeightedRMSD());
      TBasicApp::NewLogEntry() << "RMSD/A: " <<
        olxstr::FormatFloat(3, plane->CalcRMSD());
    }
    else {
      TBasicApp::NewLogEntry() << "RMSD/A: " <<
        olxstr::FormatFloat(3, plane->GetWeightedRMSD());
    }
  }
  else if (!orientOnly) {
    TBasicApp::NewLogEntry() <<
      "The plane was not created because it is either not unique or valid";
  }
}
//.............................................................................
void GXLibMacros::macCent(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  app.AddCentroid(app.FindXAtoms(Cmds, true, true).GetObject());
}
//.............................................................................
void GXLibMacros::macUniq(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, true);
  if( Atoms.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  TNetPList L(Atoms, FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
  app.FragmentsVisible(app.InvertFragmentsList(
    ACollectionItem::Unique(L)), false);
  app.CenterView(true);
  app.Draw();
}
//.............................................................................
void GXLibMacros::macGroup(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  olxstr name = Options.FindValue('n');
  if (app.GetSelection().IsEmpty())
    app.SelectAll(true);
  if (name.IsEmpty())  {
    name = "group";
    name << (app.GetRender().GroupCount()+1);
  }
  app.GroupSelection(name);
}
//.............................................................................
void GXLibMacros::macFmol(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  app.AllVisible(true);
  app.CenterView();
  app.GetRender().GetBasis().SetZoom(
    app.GetRender().CalcZoom()*app.GetExtraZoom());
}
//.............................................................................
void GXLibMacros::macCell(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  app.SetCellVisible(Cmds.IsEmpty() ? !app.IsCellVisible() : Cmds[0].ToBool());
  if (app.DUnitCell().IsVisible()) {
    bool r = Options.GetBoolOption('r');
    app.DUnitCell().SetReciprocal(r, r ? 100: 1);
  }
  app.CenterView();
}
//.............................................................................
void GXLibMacros::macSel(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if (TModeRegistry::GetInstance().GetCurrent() != NULL) {
    TBasicApp::NewLogEntry(logError) << "Unavailable in a mode";
    return;
  }
  if (Cmds.Count() >= 1 && Cmds[0].Equalsi("frag")) {
    TNetPList nets(
      app.FindXAtoms(TStrObjList(Cmds.SubListFrom(1)), false, false),
      FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
    app.SelectFragments(ACollectionItem::Unique(nets), !Options.Contains('u'));
  }
  else if( Cmds.Count() == 1 && Cmds[0].Equalsi("res") )  {
    //app.GetRender().ClearSelection();
    //TStrList out;
    //TCAtomPList a_res;
    //TPtrList<TSimpleRestraint> b_res;
    //RefinementModel& rm = app.XFile().GetRM();
    //rm.Describe(out, &a_res, &b_res);
    //app.XFile().GetAsymmUnit().GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    //for( size_t i=0; i < a_res.Count(); i++ )
    //  a_res[i]->SetTag(1);
    //TGXApp::AtomIterator ai = app.GetAtoms();
    //while( ai.HasNext() )  {
    //  TXAtom& xa = ai.Next();
    //  if( xa.CAtom().GetTag() != 1 )  continue;
    //  if( xa.IsSelected() )  continue;
    //  app.GetRender().Select(xa);
    //}
    //for( size_t i=0; i < b_res.Count(); i++ )  {
    //  const TSimpleRestraint& res = *b_res[i];
    //  for( size_t j=0; j < res.AtomCount(); j+=2 )  {
    //    if( res.GetAtom(j).GetMatrix() != NULL || res.GetAtom(j+1).GetMatrix() != NULL )  continue;
    //    const size_t id1 = res.GetAtom(j).GetAtom()->GetId();
    //    const size_t id2 = res.GetAtom(j+1).GetAtom()->GetId();
    //    TGXApp::BondIterator bi = app.GetBonds();
    //    while( bi.HasNext() )  {
    //      TXBond& xb = bi.Next();
    //      if( xb.IsSelected() )  continue;
    //      const TCAtom& ca1 = xb.A().CAtom();
    //      const TCAtom& ca2 = xb.B().CAtom();
    //      if( (ca1.GetId() == id1 && ca2.GetId() == id2) ||
    //          (ca1.GetId() == id2 && ca2.GetId() == id1) )  
    //      {
    //        app.GetRender().Select(xb);
    //        break;
    //      }
    //    }
    //  }
    //}
    //FGlConsole->PrintText(out);
    //app.GetLog() << NewLineSequence();
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("resi")) {
    TAsymmUnit &au = app.XFile().GetAsymmUnit();
    au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    TSizeList nums;
    TStrList names;
    for (size_t i=1; i < Cmds.Count(); i++) {
      if (Cmds[i].IsInt())
        nums << Cmds[i].ToSizeT();
      else
        names << Cmds[i];
    }
    for (size_t i=0; i < au.ResidueCount(); i++) {
      TResidue &r = au.GetResidue(i);
      bool found = false;
      for (size_t j=0; j < nums.Count(); j++) {
        if (nums[j] == r.GetNumber() || nums[j] == r.GetAlias()) {
          found = true;
          break;
        }
      }
      if (!found) {
        for (size_t j=0; j < names.Count(); j++) {
          if (r.GetClassName().Equalsi(names[j])) {
            found = true;
            break;
          }
        }
      }
      if (!found) continue;
      for (size_t j=0; j < r.Count(); j++)
        r[j].SetTag(1);

    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.IsVisible() && a.CAtom().GetTag() == 1)
        app.GetRender().Select(a, true);
    }
  }
  else if (Cmds.Count() == 1 && TSymmParser::IsRelSymm(Cmds[0])) {
    bool invert = Options.Contains('i'), unselect=false;
    if( !invert )
      unselect = Options.Contains('u');
    const smatd matr = TSymmParser::SymmCodeToMatrix(
      app.XFile().GetUnitCell(), Cmds[0]);
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom& a = ai.Next();
      if (a.IsDeleted() || !a.IsVisible() )  continue;
      if (a.IsGenerator(matr) )  {
        if (invert)
          app.GetRender().Select(a, !a.IsSelected());
        else if (unselect)
          app.GetRender().Select(a, false);
        else
          app.GetRender().Select(a, true);
      }
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      TXBond& b = bi.Next();
      if (b.IsDeleted() || !b.IsVisible())  continue;
      if (b.A().IsGenerator(matr) && b.B().IsGenerator(matr)) {
        if (invert)
          app.GetRender().Select(b, !b.IsSelected());
        else if (unselect)
          app.GetRender().Select(b, false);
        else
          app.GetRender().Select(b, true);
      }
    }
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("part")) {
    Cmds.Delete(0);
    TIntList parts;
    for (size_t i=0; Cmds.Count(); i++) {
      if (Cmds[i].IsNumber()) {
        parts.Add(Cmds[i].ToInt());
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if (!parts.IsEmpty()) {
      olxstr cond = "xatom.part==";
      cond << parts[0];
      for (size_t i=1; i < parts.Count(); i++)
        cond << "||xatom.part==" << parts[i];
      app.SelectAtomsWhere(cond);
    }
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("fvar")) {
    Cmds.Delete(0);
    TIntList fvars;
    for (size_t i=0; Cmds.Count(); i++) {
      if (Cmds[i].IsNumber()) {
        int &v = fvars.Add(Cmds[i].ToInt());
        v = olx_sign(v)*(olx_abs(v)-1);
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if (!fvars.IsEmpty()) {
      TGXApp::AtomIterator ai = app.GetAtoms();
      while (ai.HasNext()) {
        TXAtom &a = ai.Next();
        XVarReference *ref = a.CAtom().GetVarRef(catom_var_name_Sof);
        if (ref == NULL) continue;
        int v = int(ref->Parent.GetId());
        if (ref->relation_type == relation_AsOneMinusVar)
          v *= -1;
        for (size_t i=0; i < fvars.Count(); i++) {
          if (fvars[i] == v)
            app.GetRender().Select(a, true);
        }
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("isot")) {
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext())  {
      TXAtom& xa = ai.Next();
      if (xa.GetEllipsoid() == NULL)
        app.GetRender().Select(xa, true);
    }
  }
  else if (Cmds.Count() >= 1 && Cmds[0].Equalsi("wbox")) {
    if (Cmds.Count() >= 2 && Cmds[1].Equalsi("cell")) {
      const mat3d m = app.XFile().GetAsymmUnit().GetCellToCartesian();
      T3DFrameCtrl& fr = app.Get3DFrame();
      fr.SetEdge(0, vec3d());
      fr.SetEdge(1, m[1]);
      fr.SetEdge(2, m[0] + m[1]);
      fr.SetEdge(3, m[0]);
      fr.SetEdge(4, m[2]);
      fr.SetEdge(5, m[1] + m[2]);
      fr.SetEdge(6, m[0] + m[1] + m[2]);
      fr.SetEdge(7, m[0] + m[2]);
      fr.UpdateEdges();
      app.SetGraphicsVisible(&fr, true);
    }
    else {
      TSAtomPList atoms;
      if (app.FindSAtoms(EmptyString(), atoms, true)) {
        const WBoxInfo bs = TXApp::CalcWBox(atoms, NULL, TSAtom::weight_occu);
        T3DFrameCtrl& fr = app.Get3DFrame();
        vec3d nx = bs.normals[0]*bs.s_from[0];
        vec3d px = bs.normals[0]*bs.s_to[0];
        vec3d ny = bs.normals[1]*bs.s_from[1];
        vec3d py = bs.normals[1]*bs.s_to[1];
        vec3d nz = bs.normals[2]*bs.s_from[2];
        vec3d pz = bs.normals[2]*bs.s_to[2];
        if (ny.XProdVec(nx).DotProd(nz) < 0) {
          olx_swap(nx, px);
          olx_swap(ny, py);
          olx_swap(nz, pz);
        }
        fr.SetEdge(0, nx+ny+nz);
        fr.SetEdge(1, nx+py+nz);
        fr.SetEdge(2, px+py+nz);
        fr.SetEdge(3, px+ny+nz);
        fr.SetEdge(4, nx+ny+pz);
        fr.SetEdge(5, nx+py+pz);
        fr.SetEdge(6, px+py+pz);
        fr.SetEdge(7, px+ny+pz);
        fr.UpdateEdges();
        fr.Translate(bs.center);
        app.SetGraphicsVisible(&fr, true);
      }
    }
  }
  else if (!(Options.Contains('a') ||
    Options.Contains('i') ||
    Options.Contains('u')))
  {
    size_t period=5;
    TXAtomPList Atoms = app.FindXAtoms("sel", false, false);
    if (Atoms.IsEmpty() && Cmds.Count() == 1) {
      TGPCollection* gpc = app.GetRender().FindCollection(Cmds[0]);
      if (gpc != NULL) {
        for (size_t i=0; i < gpc->ObjectCount(); i++)
          app.GetRender().Select(gpc->GetObject(i));
        return;
      }
    }
    for (size_t i=0; i <= Atoms.Count(); i+=period) {
      olxstr Tmp;
      for( size_t j=0; j < period; j++ )  {
        if( (j+i) >= Atoms.Count() )  break;
        Tmp << Atoms[i+j]->GetGuiLabel();
        Tmp.RightPadding((j+1)*14, ' ', true);
      }
      if (!Tmp.IsEmpty())
        TBasicApp::NewLogEntry() << Tmp;
    }
    if (!Cmds.IsEmpty()) {
      size_t whereIndex = Cmds.IndexOf("where");
      if (whereIndex >= 1 && whereIndex != InvalidIndex) {
        olxstr Tmp = Cmds[whereIndex-1];
        while (olx_is_valid_index(whereIndex)) { Cmds.Delete(whereIndex--); }
        if (Tmp.Equalsi("atoms"))
          app.SelectAtomsWhere(Cmds.Text(' '));
        else if (Tmp.Equalsi("bonds"))
          app.SelectBondsWhere(Cmds.Text(' '));
        else
          Error.ProcessingError(__OlxSrcInfo, "undefined keyword: " ) << Tmp;
        return;
      }
      else {
        size_t ringsIndex = Cmds.IndexOf("rings");
        if (ringsIndex != InvalidIndex) {
          Cmds.Delete( ringsIndex );
          app.SelectRings(Cmds.Text(' '));
        }
        else
          app.SelectAtoms(Cmds.Text(' '));
        return;
      }
    }
    olxstr seli = app.GetSelectionInfo(Options.Contains('l'));
    if (!seli.IsEmpty()) {
      app.NewLogEntry() << seli;
      if (Options.Contains('c'))
        app.ToClipboard(seli);
    }
  }
  else {
    for (size_t i=0; i < Options.Count(); i++) {
      if (Options.GetName(i)[0] == 'a')
        app.SelectAll(true);
      else if (Options.GetName(i)[0] == 'u')
        app.SelectAll(false);
      else if( Options.GetName(i)[0] == 'i' )  {
        if (Cmds.Count() == 0)
          app.GetRender().InvertSelection();
        else
          app.SelectAtoms(Cmds.Text(' '), true);
      }
      else
        Error.ProcessingError(__OlxSrcInfo, "wrong options");
    }
  }
}
//.............................................................................
void GXLibMacros::macUndo(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if (!app.GetUndo().isEmpty()) {
    TUndoData* data = app.GetUndo().Pop();
    data->Undo();
    delete data;
  }
}
//.............................................................................
void GXLibMacros::macBasis(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  app.SetBasisVisible(
    Cmds.IsEmpty() ? !app.IsBasisVisible() : Cmds[0].ToBool());
}
//.............................................................................
void GXLibMacros::macPiM(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TTypeList<TSAtomPList> rings;
  TTypeList<TSAtomPList> ring_M;
  bool check_metal = true;
  ConstPtrList<TXAtom> atoms = app.FindXAtoms(Cmds, false, true);
  if( atoms.IsEmpty() )  {
    app.FindRings("C4", rings);
    app.FindRings("C5", rings);
    app.FindRings("C6", rings);
    for( size_t i=0; i < rings.Count(); i++ )  {
      if( TSPlane::CalcRMSD(rings[i]) > 0.05 ||
          !TNetwork::IsRingRegular(rings[i]) )
      {
        rings.NullItem(i);
      }
    }
    rings.Pack();
  }
  else  {
    rings.Add(new TSAtomPList(atoms, StaticCastAccessor<TSAtom>()));
    TSAtomPList &metals = ring_M.AddNew();
    for( size_t i=0; i < rings[0].Count(); i++ )  {
      if( XElementLib::IsMetal(rings[0][i]->GetType()) )  {
        check_metal = false;
        metals.Add(rings[0][i]);
        rings[0][i] = NULL;
      }
    }
    rings[0].Pack();
    if( rings[0].IsEmpty() )  return;
    if( check_metal )
      ring_M.Delete(0);
  }
  if( check_metal )  {
    ring_M.SetCapacity(rings.Count());
    for( size_t i=0; i < rings.Count(); i++ )  {
      TSAtomPList &metals = ring_M.AddNew();
      for( size_t j=0; j < rings[i].Count(); j++ )  {
        for( size_t k=0; k < rings[i][j]->NodeCount(); k++ )  {
          if( XElementLib::IsMetal(rings[i][j]->Node(k).GetType()) )  {
            metals.Add(rings[i][j]->Node(k));
          }
        }
      }
      if( metals.IsEmpty() )  {
        rings.NullItem(i);
        ring_M.NullItem(ring_M.Count()-1);
      }
    }
    rings.Pack();
    ring_M.Pack();
  }
  // process rings...
  if( rings.IsEmpty() )  return;
  const bool label = Options.Contains('l');
  TGXApp::AtomIterator ai = app.GetAtoms();
  while( ai.HasNext() )
    ai.Next().SetTag(0);
  for( size_t i=0; i < rings.Count(); i++ )  {
    vec3d c;
    for( size_t j=0; j < rings[i].Count(); j++ )  {
      c += rings[i][j]->crd();
      rings[i][j]->SetTag(1);
    }
    c /= rings[i].Count();
    ACollectionItem::Unique(ring_M[i]);
    for( size_t j=0; j < ring_M[i].Count(); j++ )  {
      TXLine &l = app.AddLine(ring_M[i][j]->GetLabel()+olxstr(i),
        ring_M[i][j]->crd(), c);
      TGraphicsStyle &ms = ((TXAtom*)ring_M[i][j])->GetPrimitives().GetStyle();
      TGlMaterial *sm = ms.FindMaterial("Sphere");
      if (sm != NULL) {
        l.GetPrimitives().GetStyle().SetMaterial(
          TXBond::StaticPrimitives()[9], *sm);
      }
      l.UpdatePrimitives((1<<9)|(1<<10));
      l.SetRadius(0.5);
      ring_M[i][j]->SetTag(2);
      if( !label )
        l.GetGlLabel().SetVisible(false);
    }
    // hide replaced bonds
  }
  TGXApp::BondIterator bi = app.GetBonds();
  while( bi.HasNext() )  {
    TXBond &b = bi.Next();
    if( b.A().GetTag() == 2 && b.B().GetTag() == 1 )
      b.SetVisible(false);
  }
}
//.............................................................................
void GXLibMacros::macShowP(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TIntList parts;
  if (!Cmds.IsEmpty())  {
    for (size_t i=0; i < Cmds.Count(); i++)
      parts.Add(Cmds[i].ToInt());
  }
  app.ShowPart(parts, true);
  if (!Options.GetBoolOption('m'))
    app.CenterView();
}
//.............................................................................
void main_CreateWBox(TGXApp& app, const TSAtomPList& atoms,
  double (*weight_c)(const TSAtom&),
  const TDoubleList& all_radii, bool print_info)
{
  const WBoxInfo bs = TXApp::CalcWBox(atoms, &all_radii, weight_c);
  static int obj_cnt = 0;
  if( print_info )  {
    app.NewLogEntry() << "Wrapping box dimension: " << 
      olxstr::FormatFloat(3, bs.r_to[0]-bs.r_from[0]) << " x "  <<
      olxstr::FormatFloat(3, bs.r_to[1]-bs.r_from[1]) << " x "  <<
      olxstr::FormatFloat(3, bs.r_to[2]-bs.r_from[2]) << " A";
    app.NewLogEntry() << "Wrapping box volume: " << 
      olxstr::FormatFloat(3, (bs.r_to-bs.r_from).Prod()) << " A^3";
  }
  const vec3d nx = bs.normals[0]*bs.s_from[0];
  const vec3d px = bs.normals[0]*bs.s_to[0];
  const vec3d ny = bs.normals[1]*bs.s_from[1];
  const vec3d py = bs.normals[1]*bs.s_to[1];
  const vec3d nz = bs.normals[2]*bs.s_from[2];
  const vec3d pz = bs.normals[2]*bs.s_to[2];

  vec3d faces[] = {
    px + py + pz, px + ny + pz, px + ny + nz, px + py + nz, // normals[0]
    nx + py + pz, nx + py + nz, nx + ny + nz, nx + ny + pz, // -normals[0]
    px + py + pz, px + py + nz, nx + py + nz, nx + py + pz, // normals[1]
    px + ny + nz, px + ny + pz, nx + ny + pz, nx + ny + nz, // -normals[1]
    nx + py + pz, nx + ny + pz, px + ny + pz, px + py + pz, // normals[2]
    nx + py + nz, px + py + nz, px + ny + nz, nx + ny + nz // -normals[2]
  };

  TArrayList<vec3f>& poly_d = *(new TArrayList<vec3f>(24));
  TArrayList<vec3f>& poly_n = *(new TArrayList<vec3f>(6));
  for( int i=0; i < 6; i++ )  {
    const vec3d cnt((faces[i*4]+faces[i*4+1]+faces[i*4+2]+faces[i*4+3])/4);
    const vec3f norm = (faces[i*4+1]-faces[i*4]).XProdVec(
      faces[i*4+3]-faces[i*4]);
    if( norm.DotProd(cnt) < 0 )  {  // does normal look inside?
      poly_n[i] = -norm;
      olx_swap(faces[i*4+1], faces[i*4+3]);
    }
    else
      poly_n[i] = norm;
  }
  for( int i=0; i < 24; i++ )
    poly_d[i] = bs.center + faces[i];
  TDUserObj* uo = new TDUserObj(app.GetRender(), sgloQuads,
    olxstr("wbox") << obj_cnt++);
  uo->SetVertices(&poly_d);
  uo->SetNormals(&poly_n);
  app.AddObjectToCreate(uo);
  uo->SetMaterial("1029;2566914048;2574743415");
  uo->Create();
  if (print_info) {
    app.NewLogEntry() << "Please note that displayed and used atomic radii "
      "might be DIFFERENT";
  }
}
void GXLibMacros::macWBox(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  ElementRadii radii;
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadVdWRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
  const bool use_aw = Options.Contains('w');
  if (Options.GetBoolOption('s')) {
    TLattice& latt = app.XFile().GetLattice();
    for (size_t i=0; i < latt.FragmentCount(); i++) {
      TSAtomPList satoms;
      TNetwork& f = latt.GetFragment(i);
      for (size_t j=0; j < f.NodeCount(); j++) {
        if (f.Node(j).IsDeleted()) continue;
        satoms.Add(f.Node(j));
      }
      if (satoms.Count() < 3) continue;
      TArrayList<double> all_radii(satoms.Count());
      for (size_t j=0; j < satoms.Count(); j++) {
        const size_t ri = radii.IndexOf(&satoms[j]->GetType());
        if (ri == InvalidIndex)
          all_radii[j] = satoms[j]->GetType().r_vdw;
        else
          all_radii[j] = radii.GetValue(ri);
      }
      main_CreateWBox(app, satoms, use_aw ?
        TSAtom::weight_occu_z : TSAtom::weight_occu, all_radii, false);
    }
  }
  else {
    TXAtomPList xatoms = app.FindXAtoms(Cmds, true, true);
    if (xatoms.Count() < 3) {
      E.ProcessingError(__OlxSrcInfo, "no enough atoms provided");
      return;
    }
    TArrayList<double> all_radii(xatoms.Count());
    for (size_t i=0; i < xatoms.Count(); i++) {
      const size_t ri = radii.IndexOf(&xatoms[i]->GetType());
      if (ri == InvalidIndex)
        all_radii[i] = xatoms[i]->GetType().r_vdw;
      else
        all_radii[i] = radii.GetValue(ri);
    }
    main_CreateWBox(app, TSAtomPList(xatoms, StaticCastAccessor<TSAtom>()),
      use_aw ? TSAtom::weight_occu_z : TSAtom::weight_occu, all_radii, true);
  }
}
//..............................................................................
void GXLibMacros::macCenter(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if (Cmds.Count() == 3 && olx_list_and(Cmds, &olxstr::IsNumber)) {
    app.GetRender().GetBasis().SetCenter(
     -vec3d(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble()));
  }
  else  {
    if (Options.GetBoolOption('z')) {
      app.GetRender().GetBasis().SetZoom(
        app.GetRender().CalcZoom()*app.GetExtraZoom());
    }
    else  {
      TXAtomPList atoms = app.FindXAtoms(Cmds, true, true);
      vec3d center;
      double sum = 0;
      for (size_t i=0; i < atoms.Count(); i++) {
        center += atoms[i]->crd()*atoms[i]->CAtom().GetOccu();
        sum += atoms[i]->CAtom().GetOccu();;
      }
      if (sum != 0) {
        center /= sum;
        app.GetRender().GetBasis().SetCenter(-center);
      }
    }
  }
}
//.............................................................................
void GXLibMacros::macCalcVoid(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  ElementRadii radii;
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadVdWRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
  TCAtomPList catoms;
  // consider the selection if any
  TGlGroup& sel = app.GetSelection();
  for( size_t i=0; i < sel.Count(); i++ )  {
    if( EsdlInstanceOf(sel[i], TXAtom) ) 
      catoms.Add(((TXAtom&)sel[i]).CAtom())->SetTag(catoms.Count());
  }
  catoms.Pack(ACollectionItem::IndexTagAnalyser());
  if( catoms.IsEmpty() ) {
    TBasicApp::NewLogEntry() <<
     "Calculating for all atoms of the asymmetric unit";
  }
  else  {
    TBasicApp::NewLogEntry() << "Calculating for " <<
      olx_analysis::alg::label(catoms);
  }
  olxstr_dict<olxstr> rv;
  double surfdis = rv('d', Options.FindValue('d', '0')).ToDouble();
  
  TBasicApp::NewLogEntry() << "Extra distance from the surface: " << surfdis;
  
  double resolution = Options.FindValue("r", "0.2").ToDouble();
  if( resolution < 0.01 )  
    resolution = 0.01;
  rv('r', resolution);
  resolution = 1./resolution;
  const vec3i dim(au.GetAxes()*resolution);
  const double mapVol = dim.Prod();
  const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
  const int minLevel = olx_round(pow(6*mapVol*3/(4*M_PI*vol), 1./3));
  app.XGrid().Clear();  // release the occupied memory
  TArray3D<short> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  map.FastInitWith(10000);
  if( Options.Contains('p') )
    app.XFile().GetUnitCell().BuildDistanceMap_Direct(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  else  {
    app.XFile().GetUnitCell().BuildDistanceMap_Masks(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  }
  size_t structureGridPoints = 0;
  TIntList levels;
  short*** amap = map.Data;
  short MaxLevel = 0;
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )  {
        if( amap[i][j][k] > MaxLevel )
          MaxLevel = amap[i][j][k];
        if( amap[i][j][k] < 0 )
          structureGridPoints++;
        else  {
          while( levels.Count() <= (size_t)amap[i][j][k] )
            levels.Add(0);
          levels[amap[i][j][k]]++;
        }
      }
    }
  }
  vec3d_list void_centers =
    MapUtil::FindLevelCenters<short>(amap, dim, MaxLevel, -MaxLevel);
  const vec3i MaxXCh = MapUtil::AnalyseChannels1(map.Data, dim, MaxLevel);
  for( int i=0; i < 3; i++ )  {
    if( MaxXCh[i] != 0 )
      TBasicApp::NewLogEntry() << (olxstr((olxch)('a'+i)) <<
        " direction can be penetrated by a sphere of " <<
        olxstr::FormatFloat(2, MaxXCh[i]/resolution) << "A radius");
  }
  TBasicApp::NewLogEntry() <<
    "Cell volume (A^3) " << olxstr::FormatFloat(3, vol);
  TBasicApp::NewLogEntry() <<
    "Radius [ volume ] of the largest spherical void is " <<
    olxstr::FormatFloat(2, (double)MaxLevel/resolution) <<
    " A [ " << olxstr::FormatFloat(2,
      olx_sphere_volume((double)MaxLevel/resolution)) << " A^3 ]";
  TBasicApp::NewLogEntry() << "The void center(s) are at (fractional):";
  for (size_t i=0; i < void_centers.Count(); i++) {
    olxstr l1;
    for (int j=0; j < 3; j++)
      l1 << ' ' << olxstr::FormatFloat(-3, void_centers[i][j]);
    TBasicApp::NewLogEntry() << l1;
  }
  TBasicApp::NewLogEntry() << (catoms.IsEmpty() ? "Structure occupies"
    : "Selected atoms occupy")
    << " (A^3) " << olxstr::FormatFloat(2, structureGridPoints*vol/mapVol) 
    << " (" << olxstr::FormatFloat(2, structureGridPoints*100/mapVol) << "%)";

  double totalVol = 0;
  for( size_t i=levels.Count()-1; olx_is_valid_index(i); i-- )  {
    totalVol += levels[i];
    TBasicApp::NewLogEntry() << "Level " << i << " is " <<
      olxstr::FormatFloat(1, (double)i/resolution) << "A away from the surface "
      << olxstr::FormatFloat(3, totalVol*vol/mapVol) << "(A^3)";
  }
  if( Options.Contains('i') )  {
    for( int i=0; i < dim[0]; i++ )  {
      for( int j=0; j < dim[1]; j++ )  {
        for( int k=0; k < dim[2]; k++ )
          amap[i][j][k] = MaxLevel-amap[i][j][k];
      }
    }
  }
  //// set map to view voids
  app.XGrid().InitGrid(dim[0], dim[1], dim[2]);
  app.XGrid().SetMinVal(0);
  app.XGrid().SetMaxVal((float)MaxLevel/resolution);
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )
        app.XGrid().SetValue(i, j, k, (float)map.Data[i][j][k]/resolution);
    }
  }
  app.XGrid().AdjustMap();
  app.XGrid().InitIso();
  app.ShowGrid(true, EmptyString());
  TBasicApp::NewLogEntry();
  //E.SetRetVal(XLibMacros::NAString());
}
//.............................................................................
void GXLibMacros::macDirection(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  const mat3d Basis = app.GetRender().GetBasis().GetMatrix();
  const vec3d Z(Basis[0][2], Basis[1][2], Basis[2][2]);
  if( app.XFile().HasLastLoader() )  {
    TAsymmUnit &au = app.XFile().GetAsymmUnit();
    mat3d m = au.GetCellToCartesian();
    vec3d fZ = au.Fractionalise(Z).Normalise();
    double min_v = 100;
    for (int i=0; i < 3; i++) {
      if (fZ[i] != 0 && olx_abs(fZ[i]) < min_v)
        min_v = olx_abs(fZ[i]);
    }
    if (min_v >= 1e-4)
      fZ /= min_v;
    olxstr Tmp =  "Direction: (";
    Tmp << olxstr::FormatFloat(3, fZ[0]) << "*A, " <<
      olxstr::FormatFloat(3, fZ[1]) << "*B, " <<
      olxstr::FormatFloat(3, fZ[2]) << "*C)";
    TBasicApp::NewLogEntry() << Tmp;
    const char *Dir[] = {"000", "100", "010", "001", "110", "101", "011", "111"};
    TTypeList<vec3d> Points;
    Points.AddNew();
    Points.AddCopy(m[0]);
    Points.AddCopy(m[1]);
    Points.AddCopy(m[2]);
    Points.AddCopy(m[0] + m[1]);
    Points.AddCopy(m[0] + m[2]);
    Points.AddCopy(m[1] + m[2]);
    Points.AddCopy(m[0] + m[1] + m[2]);
    for( size_t i=0; i < Points.Count(); i++ )  {
      for( size_t j=i+1; j < Points.Count(); j++ )  {
        double d = (Points[j]-Points[i]).Normalise().DistanceTo(Z)/2;
        if (d < 0.05) {
          Tmp = "View along ";
          Tmp << Dir[i] <<  '-' <<  Dir[j] << ' ' << '(' <<
            "normalised deviation: " <<  olxstr::FormatFloat(3, d) << "A)";
          TBasicApp::NewLogEntry() << Tmp;
        }
      }
    }
    if( !app.XGrid().IsEmpty() && app.XGrid().IsVisible() && 
      (app.XGrid().GetRenderMode()&(planeRenderModeContour|planeRenderModePlane)) != 0 )
    {
      const vec3f center(app.GetRender().GetBasis().GetCenter());
      vec3f p(0, 0, app.XGrid().GetDepth());
      p = au.Fractionalise(Basis*p - center);
      olxstr Tmp =  "Grid center: (";
      Tmp << olxstr::FormatFloat(3, p[0]) << "*A, " <<
             olxstr::FormatFloat(3, p[1]) << "*B, " <<
             olxstr::FormatFloat(3, p[2]) << "*C)";
      TBasicApp::NewLogEntry() << Tmp;
    }
  }
  else  {
    olxstr Tmp =  "Normal: (";
    Tmp << olxstr::FormatFloat(3, Z[0]) << ", " <<
           olxstr::FormatFloat(3, Z[1]) << ", " <<
           olxstr::FormatFloat(3, Z[2]) << ')';
    TBasicApp::NewLogEntry() << Tmp;
  }
}
//.............................................................................
void GXLibMacros::macIndividualise(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if (Cmds.IsEmpty()) {
    TXAtomPList atoms;
    TXBondPList bonds;
    TGlGroup& sel = app.GetSelection();
    for (size_t i=0; i < sel.Count(); i++) {
      if (EsdlInstanceOf(sel[i], TXAtom))
        atoms.Add((TXAtom&)sel[i]);
      else if (EsdlInstanceOf(sel[i], TXBond))
        bonds.Add((TXBond&)sel[i]);
    }
    app.Individualise(atoms);
    app.Individualise(bonds);
  }
  else
    app.Individualise(app.FindXAtoms(Cmds, false, false).GetObject());
}
//..............................................................................
void GXLibMacros::macCollectivise(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( Cmds.IsEmpty() )  {
    TGlGroup& glg = app.GetSelection();
    TXAtomPList atoms;
    TXBondPList bonds;
    for( size_t i=0; i < glg.Count(); i++ )  {
      if( EsdlInstanceOf(glg[i], TXAtom) )
        atoms.Add((TXAtom&)glg[i]);
      else if( EsdlInstanceOf(glg[i], TXBond) )
        bonds.Add((TXBond&)glg[i]);
    }
    app.Collectivise(atoms);
    app.Collectivise(bonds);
  }
  else
    app.Collectivise(app.FindXAtoms(Cmds, false, false).GetObject());
}
//.............................................................................
void GXLibMacros::macLstGO(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TStrList output;
  output.SetCapacity( app.GetRender().CollectionCount() );
  for( size_t i=0; i < app.GetRender().CollectionCount(); i++ )  {
    TGPCollection& gpc = app.GetRender().GetCollection(i);
    output.Add( gpc.GetName() ) << '[';
    for( size_t j=0; j < gpc.PrimitiveCount(); j++ )  {
      output.GetLastString() << gpc.GetPrimitive(j).GetName();
      if( (j+1) < gpc.PrimitiveCount() )
        output.GetLastString() << ';';
    }
    output.GetLastString() << "]->" << gpc.ObjectCount();
  }
  TBasicApp::NewLogEntry() << output;
}
//.............................................................................
class Esd_Tetrahedron  {
  TSAtomPList atoms;
  olxstr Name;
  TEValue<double> Volume;
  VcoVContainer& vcov;
protected:
  void CalcVolume()  {
    Volume =
      vcov.CalcTetrahedronVolume(*atoms[0], *atoms[1], *atoms[2], *atoms[3]);
  }
public:
  Esd_Tetrahedron(const olxstr& name, const VcoVContainer& _vcov)
    : Volume(-1,0), vcov(const_cast<VcoVContainer&>(_vcov))
  {
    Name = name;
  }
  void Add( TSAtom* a )  {
    atoms.Add( a );
    if( atoms.Count() == 4 )
      CalcVolume();
  }
  const olxstr& GetName() const  {  return Name;  }
  double GetVolume() const {  return Volume.GetV();  }
  double GetEsd() const {  return Volume.GetE();  }
};
int Esd_ThSort(const Esd_Tetrahedron &th1, const Esd_Tetrahedron &th2)  {
  return olx_cmp(th1.GetVolume(), th2.GetVolume());
}
void GXLibMacros::macEsd(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  VcoVContainer vcovc(app.XFile().GetAsymmUnit());
  try  {
    olxstr src_mat = app.InitVcoV(vcovc);
    app.NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
  }
  catch(TExceptionBase& e)  {
    Error.ProcessingError(__OlxSrcInfo, e.GetException()->GetError());
    return;
  }
  TStrList values;
  TGlGroup& sel = app.GetSelection();
  if (Options.Contains('l')) {
    for (size_t i=0; i < sel.Count(); i++) {
      if (EsdlInstanceOf(sel[i], TXBond)) {
        TXBond &xb = (TXBond&)sel[i];
        values.Add(xb.A().GetLabel()) << " to " << xb.B().GetLabel() <<
          " distance: " << vcovc.CalcDistance(xb.A(), xb.B()).ToString() <<
          " A";
      }
    }
  }
  else {
    if( sel.Count() == 1 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) )  {
        TSAtomPList atoms;
        TXAtom& xa = (TXAtom&)sel[0];
        for( size_t i=0; i < xa.NodeCount(); i++ ) {
          TSAtom& A = xa.Node(i);
          if( A.IsDeleted() || (A.GetType() == iQPeakZ) )
            continue;
          atoms.Add(A);
        }
        if( atoms.Count() == 3 )
          atoms.Add(xa);
        if( atoms.Count() < 4 )  {
          Error.ProcessingError(__OlxSrcInfo,
            "An atom with at least four bonds is expected");
          return;
        }
        TTypeList<Esd_Tetrahedron> tetrahedra;
        // special case for 4 nodes
        if( atoms.Count() == 4 )  {
          Esd_Tetrahedron& th = tetrahedra.AddNew(
            olxstr(atoms[0]->GetLabel()) << '-'
            << atoms[1]->GetLabel() << '-'
            << atoms[2]->GetLabel() << '-'
            << atoms[3]->GetLabel(), vcovc);
          th.Add(atoms[0]);
          th.Add(atoms[1]);
          th.Add(atoms[2]);
          th.Add(atoms[3]);
        }
        else  {
          for( size_t i=0; i < atoms.Count(); i++ ) {
            for( size_t j=i+1; j < atoms.Count(); j++ ) {
              for( size_t k=j+1; k < atoms.Count(); k++ ) {
                Esd_Tetrahedron& th = tetrahedra.AddNew(
                  olxstr(xa.GetLabel()) << '-'
                  << atoms[i]->GetLabel() << '-'
                  << atoms[j]->GetLabel() << '-'
                  << atoms[k]->GetLabel(), vcovc);
                th.Add(&xa);
                th.Add(atoms[i]);
                th.Add(atoms[j]);
                th.Add(atoms[k]);
              }
            }
          }
        }
        const size_t thc = (atoms.Count()-2)*2;
        QuickSorter::SortSF(tetrahedra, &Esd_ThSort);
        bool removed = false;
        while(  tetrahedra.Count() > thc )  {
          TBasicApp::NewLogEntry() << "Removing tetrahedron " <<
            tetrahedra[0].GetName() << " with volume " <<
            tetrahedra[0].GetVolume();
          tetrahedra.Delete(0);
          removed = true;
        }
        double v = 0, esd = 0;
        for( size_t i=0; i < tetrahedra.Count(); i++ )  {
          v += tetrahedra[i].GetVolume();
          esd += tetrahedra[i].GetEsd()*tetrahedra[i].GetEsd();
        }
        TEValue<double> ev(v, sqrt(esd));
        if( removed )  {
          values.Add("The volume for remaining tetrahedra is ") <<
            ev.ToString() << " A^3";
        }
        else  {
          values.Add("The tetrahedra volume is ") << ev.ToString() << " A^3";
        }
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) )  {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[0];
        olxstr pld;
        for( size_t i=0; i < xp.Count(); i++ )  {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        values.Add("Plane ") << pld <<
          (values.Add("RMS: ") << vcovc.CalcPlane(atoms).ToString()) << " A";
        TEPoint3<double> c_cent(vcovc.CalcCentroid(atoms));
        values.Add("Plane ") << pld << "cartesian centroid : {" <<
          c_cent[0].ToString() << ", " << c_cent[1].ToString() << ", " <<
          c_cent[2].ToString() << "}";
        TEPoint3<double> f_cent(vcovc.CalcCentroidF(atoms));
        values.Add("Plane ") << pld << "fractional centroid : {" <<
          f_cent[0].ToString() << ", " << f_cent[1].ToString() << ", " <<
          f_cent[2].ToString() << "}";
      }
      else if( EsdlInstanceOf(sel[0], TXBond) )  {
        TXBond& xb = (TXBond&)sel[0];
        values.Add(xb.A().GetLabel()) << " to " << xb.B().GetLabel() <<
          " distance: " << vcovc.CalcDistance(xb.A(), xb.B()).ToString() <<
          " A";
      }
    }
    else if( sel.Count() == 2 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXAtom) )  {
        values.Add(((TXAtom&)sel[0]).GetLabel()) << " to " <<
          ((TXAtom&)sel[1]).GetLabel() << " distance: " <<
          vcovc.CalcDistance((TXAtom&)sel[0], (TXAtom&)sel[1]).ToString() <<
          " A";
      }
      else if( EsdlInstanceOf(sel[0], TXBond) &&
        EsdlInstanceOf(sel[1], TXBond) )
      {
        TSBond& b1 = ((TXBond&)sel[0]);
        TSBond& b2 = ((TXBond&)sel[1]);
        TEValue<double> v(vcovc.CalcB2BAngle(b1.A(), b1.B(), b2.A(), b2.B())),
          v1(180-v.GetV(), v.GetE());
        values.Add(b1.A().GetLabel()) << '-' << b1.B().GetLabel() << " to " <<
          b2.A().GetLabel() << '-' << b2.B().GetLabel() << " angle: " <<
          v.ToString() << '(' << v1.ToString() << ')';
      }
      else if(
        (EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXPlane)) ||
        (EsdlInstanceOf(sel[1], TXAtom) && EsdlInstanceOf(sel[0], TXPlane)))
      {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[ EsdlInstanceOf(sel[0], TXPlane) ? 0 : 1];
        olxstr pld;
        for( size_t i=0; i < xp.Count(); i++ )  {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        TSAtom& sa = ((TXAtom&)sel[EsdlInstanceOf(sel[0],TXAtom) ? 0 : 1]);
        values.Add(sa.GetLabel()) << " to plane " << pld << "distance: " <<
          vcovc.CalcP2ADistance(atoms, sa).ToString() << " A";
        values.Add(sa.GetLabel()) << " to plane " << pld <<
          "centroid distance: " <<
          vcovc.CalcPC2ADistance(atoms, sa).ToString() << " A";
      }
      else if(
        (EsdlInstanceOf(sel[0], TXBond) && EsdlInstanceOf(sel[1], TXPlane)) ||
        (EsdlInstanceOf(sel[1], TXBond) && EsdlInstanceOf(sel[0], TXPlane)))
      {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[ EsdlInstanceOf(sel[0], TXPlane) ? 0 : 1];
        olxstr pld;
        for( size_t i=0; i < xp.Count(); i++ )  {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        TSBond& sb = ((TXBond&)sel[ EsdlInstanceOf(sel[0], TXBond) ? 0 : 1]);
        TEValue<double> v(vcovc.CalcP2VAngle(atoms, sb.A(), sb.B())),
          v1(180-v.GetV(), v.GetE());
        values.Add(sb.A().GetLabel()) << '-' << sb.B().GetLabel() << " to plane "
          << pld << "angle: " << v.ToString() << '(' << v1.ToString() << ')';
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) &&
        EsdlInstanceOf(sel[1], TXPlane) )
      {
        TSAtomCPList p1, p2;
        TXPlane& xp1 = (TXPlane&)sel[0];
        TXPlane& xp2 = (TXPlane&)sel[1];
        olxstr pld1, pld2;
        for( size_t i=0; i < xp1.Count(); i++ )  {
          p1.Add(xp1.GetAtom(i));
          pld1 << p1.GetLast()->GetLabel() << ' ';
        }
        for( size_t i=0; i < xp2.Count(); i++ )  {
          p2.Add(xp2.GetAtom(i));
          pld2 << p2.GetLast()->GetLabel() << ' ';
        }
        const TEValue<double> angle = vcovc.CalcP2PAngle(p1, p2);
        values.Add("Plane ") << pld1 << "to plane angle: " << angle.ToString();
        values.Add("Plane centroid to plane centroid distance: ") <<
          vcovc.CalcPC2PCDistance(p1, p2).ToString() << " A";
        values.Add("Plane [") << pld1 << "] to plane centroid distance: " <<
          vcovc.CalcP2PCDistance(p1, p2).ToString() << " A";
        values.Add("Plane [") << pld1 << "] to plane shift: " <<
          vcovc.CalcP2PShiftDistance(p1, p2).ToString() << " A";
        if( olx_abs(angle.GetV()) > 1e-6 )  {
          values.Add("Plane [") << pld2 << "] to plane centroid distance: " <<
            vcovc.CalcP2PCDistance(p2, p1).ToString() << " A";
          values.Add("Plane [") << pld2 << "] to plane shift distance: " <<
            vcovc.CalcP2PShiftDistance(p2, p1).ToString() << " A";
        }
      }
    }
    else if( sel.Count() == 3 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) &&
          EsdlInstanceOf(sel[1], TXAtom) &&
          EsdlInstanceOf(sel[2], TXAtom) )
      {
        TSAtom& a1 = (TXAtom&)sel[0];
        TSAtom& a2 = (TXAtom&)sel[1];
        TSAtom& a3 = (TXAtom&)sel[2];
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel()
          << " angle (numerical): " << vcovc.CalcAngle(a1, a2, a3).ToString();
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel()
          << " angle (analytical): " << vcovc.CalcAngleA(a1, a2, a3).ToString();
      }
      else if(
        (EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXAtom) &&
         EsdlInstanceOf(sel[2], TXAtom)) ||
        (EsdlInstanceOf(sel[1], TXPlane) && EsdlInstanceOf(sel[0], TXAtom) &&
         EsdlInstanceOf(sel[2], TXAtom)) ||
        (EsdlInstanceOf(sel[2], TXPlane) && EsdlInstanceOf(sel[1], TXAtom) &&
         EsdlInstanceOf(sel[0], TXAtom)))
      {
        TSAtom* a1 = NULL, *a2 = NULL;
        TXPlane* xp = NULL;
        TSAtomCPList atoms;
        for( size_t  i=0; i < 3; i++ )  {
          if( EsdlInstanceOf(sel[i], TXPlane) )
            xp = &(TXPlane&)sel[i];
          else  {
            if( a1 == NULL )
              a1 = &((TXAtom&)sel[i]);
            else
              a2 = &((TXAtom&)sel[i]);
          }
        }
        olxstr pld;
        for( size_t i=0; i < xp->Count(); i++ )  {
          atoms.Add(xp->GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        values.Add(a1->GetLabel()) << '-' << a2->GetLabel() << " to plane " <<
          pld << "angle: " << vcovc.CalcP2VAngle(atoms, *a1, *a2).ToString();
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) &&
        EsdlInstanceOf(sel[1], TXPlane) &&
        EsdlInstanceOf(sel[2], TXPlane) )
      {
        TSPlane& p1 = (TXPlane&)sel[0];
        TSPlane& p2 = (TXPlane&)sel[1];
        TSPlane& p3 = (TXPlane&)sel[2];
        TSAtomCPList a1, a2, a3;
        for( size_t i=0; i < p1.Count(); i++ )  a1.Add(p1.GetAtom(i));
        for( size_t i=0; i < p2.Count(); i++ )  a2.Add(p2.GetAtom(i));
        for( size_t i=0; i < p3.Count(); i++ )  a3.Add(p3.GetAtom(i));
        values.Add("Angle between plane centroids: ") <<
          vcovc.Calc3PCAngle(a1, a2, a3).ToString();
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) &&
              EsdlInstanceOf(sel[1], TXAtom) &&
              EsdlInstanceOf(sel[2], TXPlane) )
      {
        TSPlane& p1 = (TXPlane&)sel[0];
        TSAtom& a = (TXAtom&)sel[1];
        TSPlane& p2 = (TXPlane&)sel[2];
        TSAtomCPList a1, a2;
        for( size_t i=0; i < p1.Count(); i++ )  a1.Add(p1.GetAtom(i));
        for( size_t i=0; i < p2.Count(); i++ )  a2.Add(p2.GetAtom(i));
        values.Add("Angle between plane centroid - atom - plane centroid: ") <<
          vcovc.CalcPCAPCAngle(a1, a, a2).ToString();
      }
    }
    else if( sel.Count() == 4 &&
      olx_list_and_st(sel, &olx_is<TXAtom, TGlGroup::list_item_type>))
    {
      TSAtomPList a(sel, DynamicCastAccessor<TSAtom>());
      olxstr lbl = olx_analysis::alg::label(a, true, '-');
      values.Add(lbl) << " torsion angle (numerical): " <<
        vcovc.CalcTAngle(*a[0], *a[1], *a[2], *a[3]).ToString();
      values.Add(lbl) << " torsion angle (analytical): " <<
        vcovc.CalcTAngleA(*a[0], *a[1], *a[2], *a[3]).ToString();
      values.Add(lbl) << " tetrahedron volume: " <<
        vcovc.CalcTetrahedronVolume(*a[0], *a[1], *a[2], *a[3]).ToString() << " A^3";
    }
    else if( sel.Count() == 7 &&
      olx_list_and_st(sel, &olx_is<TXAtom, TGlGroup::list_item_type>))
    {
      TSAtomPList atoms(sel, DynamicCastAccessor<TSAtom>()),
        face_atoms, sorted_atoms;
      values.Add("Octahedral distortion is (for the selection): ")
        << vcovc.CalcOHDistortion(TSAtomCPList(atoms)).ToString();
      TSAtom* central_atom = atoms[0];
      atoms.Delete(0);
      olxdict<index_t, vec3d, TPrimitiveComparator> transforms;
      int face_cnt = 0;
      double total_val=0, total_esd=0, total_val_bp=0, total_esd_bp=0;
      for( size_t i=0; i < 6; i++ )  {
        for( size_t j=i+1; j < 6; j++ )  {
          for( size_t k=j+1; k < 6; k++ )  {
            const double thv = olx_tetrahedron_volume_n(central_atom->crd(),
              atoms[i]->crd(), atoms[j]->crd(), atoms[k]->crd());
            if (thv < 0.1) continue;
            sorted_atoms.Clear();
            transforms.Clear();
            atoms.ForEach(ACollectionItem::TagSetter(0));
            atoms[i]->SetTag(1);
            atoms[j]->SetTag(1);
            atoms[k]->SetTag(1);
            const vec3d face_center =
              (atoms[i]->crd()+atoms[j]->crd()+atoms[k]->crd())/3;
            const vec3d normal = vec3d::Normal(
              atoms[i]->crd(), atoms[j]->crd(), atoms[k]->crd());
            transforms.Add(1, central_atom->crd() - face_center);
            vec3d face1_center;
            for( size_t l=0; l < atoms.Count(); l++ )
              if( atoms[l]->GetTag() == 0 )
                face1_center += atoms[l]->crd();
            face1_center /= 3;
            transforms.Add(0, central_atom->crd() - face1_center);
            PlaneSort::Sorter::DoSort(atoms, transforms, central_atom->crd(),
              normal, sorted_atoms);
            if (sorted_atoms[0]->GetTag() != 1)
              sorted_atoms.ShiftR(1);
            values.Add("Face ") << ++face_cnt << ": " <<
              olx_analysis::alg::label(sorted_atoms, true, ' ') << ' ';
            sorted_atoms.Insert(0, central_atom);
            TEValue<double> rv = vcovc.CalcOHDistortionBP(
              TSAtomCPList(sorted_atoms));
            total_val_bp += rv.GetV()*3;
            total_esd_bp += olx_sqr(rv.GetE()); 
            rv = vcovc.CalcOHDistortion(
              TSAtomCPList(sorted_atoms));
            total_val += rv.GetV()*3;
            total_esd += olx_sqr(rv.GetE()); 
            values.GetLastString() << rv.ToString();
          }
        }
      }
      if( face_cnt == 8 )  {
        if (olx_abs(total_val-total_val_bp) < 1e-3) {
          values.Add("Combined distortion: ") <<
            TEValue<double>(total_val, 3*sqrt(total_esd)).ToString() <<
            ", mean: " <<
            TEValue<double>(total_val/24, 3*sqrt(total_esd)/24).ToString();
        }
        else {
          values.Add("Combined distortion (cross-projections): ") <<
            TEValue<double>(total_val, 3*sqrt(total_esd)).ToString() <<
            ", mean: " <<
            TEValue<double>(total_val/24, 3*sqrt(total_esd)/24).ToString();
          values.Add("Combined distortion (best plane): ") <<
            TEValue<double>(total_val_bp, 3*sqrt(total_esd_bp)).ToString() <<
            ", mean: " <<
            TEValue<double>(total_val_bp/24, 3*sqrt(total_esd_bp)/24).ToString()
            << " degrees";
        }
      }
      else  {
        TBasicApp::NewLogEntry() << "Could not locate required 8 octahedron faces";
      }
    }
  }
  TBasicApp::NewLogEntry() << values;
  if (Options.Contains('c'))
    app.ToClipboard(values);
  if( Options.Contains("label") )  {
    vec3d cent;
    size_t cnt = 0;
    TGlGroup& gl = app.GetSelection();
    for( size_t i=0; i < gl.Count(); i++ )  {
      if( EsdlInstanceOf(gl[i], TXAtom) )  {
        cent += ((TXAtom&)gl[i]).crd();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXBond) ) {
        cent += ((TXBond&)gl[i]).GetCenter();
        cnt++;
      }
      else if( EsdlInstanceOf(gl[i], TXPlane) ) {
        cent += ((TXPlane&)gl[i]).GetCenter();
        cnt++;
      }
    }
    if( cnt != 0 )
      cent /= cnt;
    app.CreateLabel(cent, values.Text('\n'), 4);
  }
}
//.............................................................................
void GXLibMacros::macChemDraw(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  app.CreateRings(true, true);
}
//.............................................................................
void GXLibMacros::macPoly(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  short pt = 0;
  const olxstr &str_t = Cmds.GetLastString();
  if (str_t.Equalsi("none"))
    pt = 0;
  else if (str_t.Equalsi("auto"))
    pt = polyAuto;
  else if (str_t.Equalsi("regular"))
    pt = polyRegular;
  else if (str_t.Equalsi("pyramid"))
    pt = polyPyramid;
  else if (str_t.Equalsi("bipyramid"))
    pt = polyBipyramid;
  else {
    E.ProcessingError(__OlxSrcInfo, "Undefined poly type: ").quote() << str_t;
    return;
  }
  Cmds.Delete(Cmds.Count()-1);
  TXAtomPList atoms = app.FindXAtoms(Cmds, true, true);
  atoms.ForEach(ACollectionItem::IndexTagSetter(
    FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
  atoms.Pack(ACollectionItem::IndexTagAnalyser(
    FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
  for (size_t i=0; i < atoms.Count(); i++)
    atoms[i]->SetPolyhedronType(pt);
}
//.............................................................................
void GXLibMacros::funExtraZoom(const TStrObjList& Params, TMacroError &E) {
  if (Params.IsEmpty())
    E.SetRetVal(app.GetExtraZoom());
  else
    app.SetExtraZoom(Params[0].ToDouble());
}
//.............................................................................
void GXLibMacros::macKill(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if (TModeRegistry::GetInstance().GetCurrent() != NULL)  {
    Error.ProcessingError(__OlxSrcInfo, "Kill inaccessible from within a mode");
    return;
  }
  if( Cmds.Count() == 1 && Cmds[0].Equalsi("sel") )  {
    AGDObjList Objects;
    TGlGroup& sel = app.GetSelection();
    olxstr out;
    bool group_deletion = false;
    for( size_t i=0; i < sel.Count(); i++ )  {
      if( EsdlInstanceOf(sel[i], TXAtom) )
        out << ((TXAtom&)sel[i]).GetLabel();
      if( EsdlInstanceOf(sel[i], TGlGroup) )  {
        if( !group_deletion )  {
          group_deletion = true;
          TBasicApp::NewLogEntry() << "Please use 'ungroup' to delete groups";
        }
        continue;
      }
      else
        out << sel[i].GetPrimitives().GetName();
      out << ' ';
      Objects.Add(sel[i]);
    }
    if( !out.IsEmpty()  )  {
      TBasicApp::NewLogEntry() << "Deleting " << out;
      app.GetUndo().Push(app.DeleteXObjects(Objects));
      sel.Clear();
    }
  }
  else if( Cmds.Count() == 1 && Cmds[0].Equalsi("labels") )  {
    TBasicApp::NewLogEntry() << "Deleting labels";
    for( size_t i=0; i < app.LabelCount(); i++ )
      app.GetLabel(i).SetVisible(false);
    TGXApp::AtomIterator ai = app.GetAtoms();
    while( ai.HasNext() )
      ai.Next().GetGlLabel().SetVisible(false);
    TGXApp::BondIterator bi = app.GetBonds();
    while( bi.HasNext() )
      bi.Next().GetGlLabel().SetVisible(false);
  }
  else  {
    if( Options.Contains("au") )  {
      TCAtomPList atoms = app.FindCAtoms(Cmds.Text(' '), false);
      for( size_t i=0; i < atoms.Count(); i++ )
        atoms[i]->SetDeleted(true);
    }
    else  {
      TXAtomPList Atoms = app.FindXAtoms(Cmds.Text(' '), true, false),
        Selected;
      if( Atoms.IsEmpty() )  return;
      for( size_t i=0; i < Atoms.Count(); i++ )
        if( Atoms[i]->IsSelected() )
          Selected.Add(Atoms[i]);
      TXAtomPList& todel = Selected.IsEmpty() ? Atoms : Selected;
      if( todel.IsEmpty() )
        return;
      app.GetLog() << "Deleting ";
      for( size_t i=0; i < todel.Count(); i++ )
        app.GetLog() << todel[i]->GetLabel() << ' ';
      app.NewLogEntry();
      app.GetUndo().Push(app.DeleteXAtoms(todel));
    }
  }
}
//.............................................................................
TNetwork::AlignInfo GXLibMacros_MatchAtomPairsQT(
  const TTypeList<AnAssociation2<TSAtom*,TSAtom*> >& atoms,
  bool TryInversion, double (*weight_calculator)(const TSAtom&),
  bool print = true)
{
  TNetwork::AlignInfo rv = TNetwork::GetAlignmentRMSD(atoms, TryInversion,
    weight_calculator);
  if (print) {
    const size_t cnt = olx_min(3, atoms.Count());
    olxstr f1 = '{', f2 = '{';
    for (size_t i=0; i < cnt; i++) {
      f1 << atoms[i].GetA()->GetLabel() << ',';
      f2 << atoms[i].GetB()->GetLabel() << ',';
    }
    TBasicApp::NewLogEntry() << "Alignment RMSD " << f2 << "...} to " << f1 <<
      "...} " << (TryInversion ? "with" : "without") << " inversion) is " <<
      olxstr::FormatFloat(3, rv.rmsd.GetV()) << " A";
  }
  return rv;
}
//..............................................................................
TNetwork::AlignInfo GXLibMacros_MatchAtomPairsQTEsd(
  const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
  bool TryInversion, double (*weight_calculator)(const TSAtom&))
{
  TXApp& xapp = TXApp::GetInstance();
  VcoVContainer vcovc(xapp.XFile().GetAsymmUnit());
  xapp.NewLogEntry() << "Using " << xapp.InitVcoV(vcovc) <<
    " matrix for the calculation";
  TSAtomPList atoms_out;
  vec3d_alist crds_out;
  TDoubleList wghts_out;
  TNetwork::PrepareESDCalc(atoms, TryInversion, atoms_out, crds_out, wghts_out,
    weight_calculator);
  TEValue<double> rv = vcovc.CalcAlignmentRMSD(
    TSAtomCPList(atoms_out), crds_out, wghts_out);
  TBasicApp::NewLogEntry() << (olxstr("RMSD is ") << rv.ToString() << " A");
  return TNetwork::GetAlignmentRMSD(atoms, TryInversion, weight_calculator);
}
//..............................................................................
void GXLibMacros_CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS)  {
  static const olxstr OnMatchCBName("onmatch");
  olxstr arg;
  TStrList callBackArg;
  for( size_t i=0; i < netA.NodeCount(); i++ )  {
    arg << netA.Node(i).GetLabel();
    if( (i+1) < netA.NodeCount() )  arg << ',';
  }
  callBackArg.Add(RMS);
  callBackArg.Add(arg);
  arg.SetLength(0);
  for( size_t i=0; i < netB.NodeCount(); i++ )  {
    arg << netB.Node(i).GetLabel();
    if( (i+1) < netB.NodeCount() )  arg << ',';
  }
  callBackArg.Add(arg);
  olex::IOlexProcessor::GetInstance()->callCallbackFunc(
    OnMatchCBName, callBackArg);
}
//..............................................................................
void GXLibMacros::macMatch(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  static const olxstr StartMatchCBName("startmatch");
  TActionQueueLock __queuelock(app.FindActionQueue(olxappevent_GL_DRAW));
  // restore if already applied
  TLattice& latt = app.XFile().GetLattice();
  const TAsymmUnit& au = app.XFile().GetAsymmUnit();
  latt.RestoreADPs();
  if( app.OverlayedXFileCount() != 0 )  {
    for( size_t i=0; i < app.OverlayedXFileCount(); i++ )
      app.GetOverlayedXFile(i).GetLattice().RestoreADPs();
    app.AlignOverlayedXFiles();
    app.CenterView(true);
  }
  else
    app.CenterView();
  app.UpdateBonds();
  if( Options.Contains("u") )  // do nothing...
    return;
  olex::IOlexProcessor::GetInstance()->callCallbackFunc(
    StartMatchCBName, TStrList() << EmptyString());
  const bool TryInvert = Options.Contains("i");
  double (*weight_calculator)(const TSAtom&) = &TSAtom::weight_occu;
  if( Options.Contains('w') )
    weight_calculator = &TSAtom::weight_occu_z;
  const bool subgraph = Options.Contains("s");
  olxstr suffix = Options.FindValue("n");
  const bool name = Options.Contains("n");
  const bool align = Options.Contains("a");
  int group_cnt = 2;
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    group_cnt = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  TXAtomPList atoms = app.FindXAtoms(Cmds, false, true);
  size_t match_cnt=0;
  if( !atoms.IsEmpty() )  {
    if( atoms.Count() == 2 &&
        (&atoms[0]->GetNetwork() != &atoms[1]->GetNetwork()) )
    {
      TTypeList<AnAssociation2<size_t, size_t> > res;
      TSizeList sk;
      TNetwork &netA = atoms[0]->GetNetwork(),
        &netB = atoms[1]->GetNetwork();
      bool match = subgraph ? netA.IsSubgraphOf(netB, res, sk) :
        netA.DoMatch(netB, res, TryInvert, weight_calculator);
      TBasicApp::NewLogEntry() << "Graphs match: " << match;
      if( match )  {
        match_cnt++;
        // restore the other unit cell, if any...
        if( &latt != &netA.GetLattice() || &latt != &netB.GetLattice() )  {
          TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice()
            : netA.GetLattice();
          latt1.RestoreADPs();
        }
        TTypeList< AnAssociation2<TSAtom*,TSAtom*> > satomp;
        TSAtomPList atomsToTransform;
        TBasicApp::NewLogEntry() << "Matching pairs:";
        olxstr tmp('=');
        for( size_t i=0; i < res.Count(); i++ )  {
          tmp << '{' << netA.Node( res[i].GetA()).GetLabel() <<
            ',' << netB.Node(res[i].GetB()).GetLabel() << '}';

          if( atomsToTransform.IndexOf(
                &netB.Node(res[i].GetB())) == InvalidIndex )
          {
            atomsToTransform.Add(netB.Node(res[i].GetB()));
            satomp.AddNew<TSAtom*,TSAtom*>(&netA.Node(res[i].GetA()),
              &netB.Node(res[i].GetB()));
          }
        }
        if( name )  {
          // change CXXX to CSuffix+whatever left of XXX
          if( suffix.Length() > 1 && suffix.CharAt(0) == '$' )  {
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for( size_t i=0; i < res.Count(); i++ )  {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if( l_d <= (index_t)l_val.Length() ) 
                new_l = elm.symbol + l_val;
              else if( l_d > (index_t)l_val.Length() ) {
                new_l = olxstr(elm.symbol) << l_val <<
                  old_l.SubStringFrom(elm.symbol.Length()+l_val.Length());
              }
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          // change the ending
          else if( suffix.Length() > 1 && suffix.CharAt(0) == '-' )  {
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for( size_t i=0; i < res.Count(); i++ )  {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if( l_d <= (index_t)l_val.Length() ) 
                new_l = elm.symbol + l_val;
              else if( l_d > (index_t)l_val.Length() )
                new_l = old_l.SubStringTo(old_l.Length()-l_val.Length()) << l_val;
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          else  {
            for( size_t i=0; i < res.Count(); i++ ) {
              netB.Node(res[i].GetB()).CAtom().SetLabel(
               netA.Node(res[i].GetA()).GetLabel() + suffix, false);
            }
          }
        }
        TNetwork::AlignInfo align_info;
        if (Options.GetBoolOption("esd")) {
          align_info = GXLibMacros_MatchAtomPairsQTEsd(
            satomp, TryInvert, weight_calculator);
        }
        else {
          align_info = GXLibMacros_MatchAtomPairsQT(
            satomp, TryInvert, weight_calculator);
        }
        // execute callback function
        GXLibMacros_CallMatchCallbacks(netA, netB, align_info.rmsd.GetV());
        // ends execute callback
        if( align )  {
          TNetwork::DoAlignAtoms(atomsToTransform, align_info);
          app.UpdateBonds();
          app.CenterView();
        }

        TBasicApp::NewLogEntry() << tmp;
        if( subgraph )  {
          sk.Add( res[0].GetB() );
          res.Clear();
          while( atoms[0]->GetNetwork().IsSubgraphOf(
                   atoms[1]->GetNetwork(), res, sk) )
          {
            sk.Add( res[0].GetB() );
            tmp = '=';
            for( size_t i=0; i < res.Count(); i++ )  {
              tmp << '{' <<
                atoms[0]->GetNetwork().Node(res[i].GetA()).GetLabel() << ',' <<
                atoms[1]->GetNetwork().Node(res[i].GetB()).GetLabel() << '}';
            }
            TBasicApp::NewLogEntry() << tmp;
            res.Clear();
          }
        }
      }
    }
    // a full basis provided
    else if (atoms.Count() >= 3*group_cnt && (atoms.Count()%group_cnt) == 0) {
      const size_t atom_a_group = atoms.Count()/group_cnt;
      TTypeList<AnAssociation2<TSAtom*,TSAtom*> > satomp(atom_a_group);
      // fill the reference group
      for (size_t i=0; i < atom_a_group; i++)
        satomp[i].SetA(atoms[i]);
      TNetwork &netA = atoms[0]->GetNetwork();
      for (int gi=1; gi < group_cnt; gi++) {
        for (size_t i=0; i < atom_a_group; i++)
          satomp[i].SetB(atoms[i+gi*atom_a_group]);
        TNetwork &netB = satomp[0].GetB()->GetNetwork();
        bool valid = true;
        for (size_t i=1; i < satomp.Count(); i++)  {
          if (satomp[i].GetA()->GetNetwork() != netA ||
              satomp[i].GetB()->GetNetwork() != netB)
          {
            valid = false;
            E.ProcessingError(__OlxSrcInfo,
              "Atoms should belong to two distinct fragments or the same"
              " fragment. Skipping group #") << gi;
            break;
          }
        }
        if (valid)  {
          match_cnt++;
          // restore the other unit cell, if any...
          if (&latt != &netA.GetLattice() || &latt != &netB.GetLattice())  {
            TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice()
              : netA.GetLattice();
            latt1.RestoreADPs();
          }
          TSAtomPList atomsToTransform;
          if (netA != netB) // collect all atoms
            atomsToTransform = netB.GetNodes();
          else
            atomsToTransform = atoms.SubList(gi*atom_a_group, atom_a_group);
          TNetwork::AlignInfo align_info =
            GXLibMacros_MatchAtomPairsQT(satomp, TryInvert, weight_calculator);
          TNetwork::DoAlignAtoms(atomsToTransform, align_info);
          GXLibMacros_CallMatchCallbacks(netA, netB, align_info.rmsd.GetV());
          //for (size_t ai=0; ai < netA.NodeCount(); ai++) {
          //  TSAtom &a = netA.Node(ai);
          //  TSAtom &b = netB.Node(ai);
          //  if (a.GetEllipsoid() == NULL || b.GetEllipsoid() == NULL) continue;
          //  mat3d m = a.GetEllipsoid()->GetMatrix();
          //  m[0] *= a.GetEllipsoid()->GetSX();
          //  m[1] *= a.GetEllipsoid()->GetSY();
          //  m[2] *= a.GetEllipsoid()->GetSZ();
          //  mat3d m1 = b.GetEllipsoid()->GetMatrix();
          //  m1[0] *= b.GetEllipsoid()->GetSX();
          //  m1[1] *= b.GetEllipsoid()->GetSY();
          //  m1[2] *= b.GetEllipsoid()->GetSZ();
          //  m -= m1;
          //  vec3d v = m[0].XProdVec(m[1]);
          //  if (m[2].CAngle(v) < 0)
          //    m[2] *= -1;
          //  a.GetEllipsoid()->SetMatrix(m);
          //  a.GetEllipsoid()->SetSX(1);
          //  a.GetEllipsoid()->SetSY(1);
          //  a.GetEllipsoid()->SetSZ(1);
          //  b.GetEllipsoid()->SetSX(0.01);
          //  b.GetEllipsoid()->SetSY(0.01);
          //  b.GetEllipsoid()->SetSZ(0.01);
          //}
        }
      }
    }
  }
  else  {
    TNetPList nets;
    app.GetNetworks(nets);
    // restore the other unit cell, if any...
    for( size_t i=0; i < nets.Count(); i++ )  {
      if( &latt != &nets[i]->GetLattice() )
        nets[i]->GetLattice().RestoreADPs();
    }
    TEBitArray matched(nets.Count());
    for( size_t i=0; i < nets.Count(); i++ )  {
      if( !nets[i]->IsSuitableForMatching() || matched[i] )  continue;
      for( size_t j=i+1; j < nets.Count(); j++ )  {
        if( !nets[j]->IsSuitableForMatching() || matched[j] )  continue;
        TTypeList<AnAssociation2<size_t, size_t> > res;
        if( !nets[i]->DoMatch(*nets[j], res, false, weight_calculator) )
          continue;
        match_cnt++;
        matched.SetTrue(j);
        TTypeList<AnAssociation2<TSAtom*,TSAtom*> > ap;
        for( size_t k=0; k < res.Count(); k++ ) {
          ap.AddNew<TSAtom*,TSAtom*>(
            &nets[i]->Node(res[k].GetA()), &nets[j]->Node(res[k].GetB()));
        }
        TNetwork::AlignInfo a_i =
          GXLibMacros_MatchAtomPairsQT(ap, false, weight_calculator);
        // get info for the inverted fragment
        res.Clear();
        ap.Clear();
        nets[i]->DoMatch(*nets[j], res, true, weight_calculator);
        for( size_t k=0; k < res.Count(); k++ ) {
          ap.AddNew<TSAtom*,TSAtom*>(
            &nets[i]->Node(res[k].GetA()), &nets[j]->Node(res[k].GetB()));
        }
        TNetwork::AlignInfo ia_i =
          GXLibMacros_MatchAtomPairsQT(ap, true, weight_calculator);
        TNetwork::AlignInfo& ra_i =
          (a_i.rmsd.GetV() < ia_i.rmsd.GetV() ? a_i : ia_i);
        GXLibMacros_CallMatchCallbacks(*nets[i], *nets[j], ra_i.rmsd.GetV());
//
        //inertia<>::out io1 = inertia<>::calc(nets[i]->GetNodes(),
        //  FunctionAccessor::Make(&TSAtom::crd),
        //  FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
        //  io1.sort();
        //inertia<>::out io2;
        //if (ra_i.inverted) {
        //  io2 = inertia<>::calc(nets[j]->GetNodes(),
        //    olx_alg::olx_minus(FunctionAccessor::Make(&TSAtom::crd)),
        //    FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
        //    io2.center *= -1;
        //    io2.axis *= -1;
        //}
        //else {
        //  io2 = inertia<>::calc(nets[j]->GetNodes(),
        //    FunctionAccessor::Make(&TSAtom::crd),
        //    FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
        //}
        //io2.sort();
        //mat3d tm = mat3d::Transpose(io2.axis)*io1.axis;
        //TNetwork::DoAlignAtoms(nets[j]->GetNodes(), io2.center,
        //  tm,
        //  io1.center);
//
        TNetwork::DoAlignAtoms(nets[j]->GetNodes(), ra_i);
      }
    }
    // do match all possible fragments with similar number of atoms
  }
  if (match_cnt != 0) {
    app.UpdateBonds();
    app.CenterView();
  }
  else {
    TBasicApp::NewLogEntry() << "No matching fragments found";
  }
}
//..............................................................................
olxstr GXLibMacros_funMatchNets(TNetwork& netA, TNetwork& netB, bool invert,
  bool verbose,
  double (*w_c)(const TSAtom&))
{
  TTypeList<AnAssociation2<size_t, size_t> > res;
  TSizeList sk;
  bool match = false;
  try  {  match = netA.DoMatch(netB, res, invert, w_c);  }
  catch(...)  {
    return "exception";
  }
  if( match )  {
    olxstr rv;
    TTypeList<AnAssociation2<TSAtom*,TSAtom*> > satomp;
    for( size_t i=0; i < res.Count(); i++ ) {
      satomp.AddNew<TSAtom*,TSAtom*>(
        &netA.Node(res[i].GetA()), &netB.Node(res[i].GetB()));
    }
    TNetwork::AlignInfo align_info =
      GXLibMacros_MatchAtomPairsQT(satomp, invert, w_c, false);
    rv = olxstr::FormatFloat(3, align_info.rmsd.GetV());
    mat3d m;
    QuaternionToMatrix(align_info.align_out.quaternions[0], m);
    TPSTypeList<double, AnAssociation2<TSAtom*,TSAtom*>*> pairs;
    pairs.SetCapacity(satomp.Count());
    const TAsymmUnit& au2 =
      satomp[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
    for( size_t i=0; i < satomp.Count(); i++ )  {
      vec3d v = satomp[i].GetB()->ccrd();
      if( invert )  v *= -1;
      au2.CellToCartesian(v);
      const double d = (satomp[i].GetA()->crd()-align_info.align_out.center_a)
        .DistanceTo((v-align_info.align_out.center_b)*m);
      pairs.Add(d, &satomp[i]);
    }
    rv << ",{";
    if( verbose )  {
      for( size_t i=0; i < pairs.Count(); i++ )  {
        size_t index = pairs.Count() - i -1;
        rv << olxstr::FormatFloat(3, pairs.GetKey(index)) << "(";
        rv << pairs.GetObject(index)->GetA()->GetLabel() << ',' <<
          pairs.GetObject(index)->GetB()->GetLabel() << ')';
        if( i+1 < pairs.Count() )
          rv << ',';
      }
    }
    else  {
      rv << olxstr::FormatFloat(3, pairs.GetLastKey()) << "(";
      rv << pairs.GetLastObject()->GetA()->GetLabel() << ',' <<
        pairs.GetLastObject()->GetB()->GetLabel() << ')';
    }
    return rv << '}';
  }
  return XLibMacros::NAString();
}
TStrList TMainForm_funMatchNets1(TNetwork& netA, TNetwork& netB, bool verbose)
{
  TStrList rv;
  if( !netA.IsSuitableForMatching() || !netB.IsSuitableForMatching() )
    return rv;
  const olxstr r = GXLibMacros_funMatchNets(netA, netB, false, verbose,
    TSAtom::weight_unit);
  if( r == XLibMacros::NAString() )  return rv;
  rv.Add("Fragment content: ") << netA.GetFormula();
  rv.Add("inverted:false,weight:unit=") << r; 
  rv.Add("inverted:false,weight:Z=") <<
    GXLibMacros_funMatchNets(netA, netB, false, verbose, TSAtom::weight_z);
  rv.Add("inverted:true,weight:unit=") <<
    GXLibMacros_funMatchNets(netA, netB, true, verbose, TSAtom::weight_unit); 
  rv.Add("inverted:true,weight:Z=") <<
    GXLibMacros_funMatchNets(netA, netB, true, verbose, TSAtom::weight_z); 
  return rv;
}
TStrList GXLibMacros_funMatchLatts(TLattice& lattA, TLattice& lattB,
  const TStrObjList& Params)
{
  TStrList rv;
  const bool verbose = (Params.Count() == 3 && Params[2].Equalsi("verbose"));
  if( lattA.FragmentCount() > 1 )  {
    rv.Add("Comparing '") << TEFile::ExtractFileName(Params[0]) <<
      "' to itself";
    for( size_t i=0; i < lattA.FragmentCount(); i++ )  {
      if( !lattA.GetFragment(i).IsSuitableForMatching() )  continue;
      for( size_t j=i+1; j < lattA.FragmentCount(); j++ )  {
        if( !lattA.GetFragment(j).IsSuitableForMatching() )  continue;
        rv.AddList(
          TMainForm_funMatchNets1(
            lattA.GetFragment(i), lattA.GetFragment(j), verbose));
      }
    }
  }
  if( lattB.FragmentCount() > 1 )  {
    rv.Add("Comparing '") << TEFile::ExtractFileName(Params[1]) <<
      "' to itself";
    for( size_t i=0; i < lattB.FragmentCount(); i++ )  {
      if( !lattB.GetFragment(i).IsSuitableForMatching() )  continue;
      for( size_t j=i+1; j < lattB.FragmentCount(); j++ )  {
        if( !lattB.GetFragment(j).IsSuitableForMatching() )  continue;
        rv.AddList(
          TMainForm_funMatchNets1(lattB.GetFragment(i),
            lattB.GetFragment(j), verbose));
      }
    }
  }
  rv.Add("Comparing the two files");
  for( size_t i=0; i < lattA.FragmentCount(); i++ )  {
    if( !lattA.GetFragment(i).IsSuitableForMatching() )  continue;
    for( size_t j=0; j < lattB.FragmentCount(); j++ )  {
      if( !lattB.GetFragment(j).IsSuitableForMatching() )  continue;
      rv.AddList(
        TMainForm_funMatchNets1(
          lattA.GetFragment(i), lattB.GetFragment(j), verbose));
    }
  }
  return rv;
}
//..............................................................................
void GXLibMacros::funMatchFiles(const TStrObjList& Params, TMacroError &E)  {
  double (*weight_calculator)(const TSAtom&) = &TSAtom::weight_occu;
  try  {
    TXFile* f1 = (TXFile*)app.XFile().Replicate();
    TXFile* f2 = (TXFile*)app.XFile().Replicate();
    try  {
      f1->LoadFromFile(Params[0]);
      f2->LoadFromFile(Params[1]);
      TStrList rv;
      TLattice& lattA = f1->GetLattice();
      TLattice& lattB = f2->GetLattice();
      rv.Add("!H atoms included");
      rv.AddList(GXLibMacros_funMatchLatts(lattA, lattB, Params));
      rv.Add("!H atoms excluded");
      lattA.GetAsymmUnit().DetachAtomType(iHydrogenZ, true);
      lattA.UpdateConnectivity();
      lattB.GetAsymmUnit().DetachAtomType(iHydrogenZ, true);
      lattB.UpdateConnectivity();
      rv.AddList(GXLibMacros_funMatchLatts(lattA, lattB, Params));
      delete f1;
      delete f2;
      E.SetRetVal(rv.Text(NewLineSequence()));
      return;
    }
    catch(...)  {
      delete f1;
      delete f2;
    }
  }
  catch(...)  {}
  E.SetRetVal(XLibMacros::NAString());
}
//..............................................................................


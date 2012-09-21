/* A simple postscript file interface 
(c) Oleg Dolomanov, 2009
*/
#ifndef _olx_tex_writerH
#define _olx_tex_writerH
#include "efile.h"
#include "glbase.h"
#include "threex3.h"

class TEXWriter  {
  TEFile out;
  char bf[80];
  uint32_t CurrentColor;
  float CurrentLineWidth;
public:
  typedef void (TEXWriter::*RenderFunc)();
  TEXWriter(const olxstr& fileName) {
    CurrentColor = 0;
    CurrentLineWidth = 1;
    out.Open(fileName, "w+b");
      
    out.Writeln("\\documentclass[a4paper,10pt]{article}");
    out.Writeln("\\usepackage{color}");
    out.Writeln("\\usepackage{graphicx}");
    out.Writeln("\\usepackage{tikz}");
    out.Writeln("\\usetikzlibrary{trees}");
    out.Writeln("\\usetikzlibrary{shapes}");
    out.Writeln("\\usetikzlibrary{patterns}");
    out.Writeln("\\usetikzlibrary{calc,through,backgrounds}");
    out.Writeln("\\usetikzlibrary{arrows,decorations.pathmorphing,backgrounds,positioning,fit}");
    out.Writeln("\\usepackage[english]{babel}");
    //out.Writeln("\\begin{document}");
    out.Writeln("");
    //white border width around elements
    out.Writeln("\\newcommand{\\whitespace}{1pt}");
    out.Writeln("");   
  }
  //..........................................................................
  void Writenl(const char* bf)  {
    out.Writeln(bf);
  }  
};

#endif
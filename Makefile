# $HeadURL$
# $LastChangedDate$
# $LastChangedRevision$
# $LastChangedBy$
# $Id$
# #############################################################################
# Makefile for compiling, linking and install olex2
# This currently doesn't check the ld paths
#
###############################################################################
# MACROS - some of these can and should be set using configure when this evolves#
# VERSION = 1
# PATCHLEVEL = 0
# SUBLEVEL = 1
# EXTRAVERSION =u
# NAME = Olex v1.1u - super banana monkey
#
PROXY = 
OWNER = $(shell USER)
GROUP = $(shell GROUP)
#######################################
# Files and path settings
USER_SETTINGS = usettings.dat
START_FILE = startup
CWD :=  $(shell pwd)
SRC_DIR = $(CWD)/
OBJ_DIR = $(CWD)/obj/
EXE_DIR = $(CWD)/bin/
OLEX_INS := $(HOME)/olex
OLEX_BIN := $(HOME)/bin
#######################################
# Compiling
CC = gcc
CFLAGS = `wx-config --cxxflags --unicode --toolkit=gtk2` `python-config --includes` -I$(SRC_DIR)sdl -I$(SRC_DIR)xlib -I$(SRC_DIR)glib -I$(SRC_DIR)gxlib -I$(SRC_DIR)repository -I$(SRC_DIR)olex -I$(SRC_DIR)alglib -S -D__WXWIDGETS__ -D_UNICODE -fexceptions -O3 -combine
###############################################################################

# All will compile and link all of olex
all: obj unirun olex link

# obj will create the obj directory and compile the objects
obj: $@
	@echo "Building object libraries, this can take a while"
	@mkdir $(OBJ_DIR);
	@cd $(OBJ_DIR); $(CC) $(SRC_DIR)alglib/*.cpp  $(SRC_DIR)sdl/*.cpp  $(SRC_DIR)sdl/smart/*.cpp  $(SRC_DIR)xlib/*.cpp  $(SRC_DIR)xlib/macro/*.cpp  $(SRC_DIR)glib/*.cpp  $(SRC_DIR)gxlib/*.cpp  $(SRC_DIR)repository/filesystem.cpp  $(SRC_DIR)repository/shellutil.cpp  $(SRC_DIR)repository/httpex.cpp  $(SRC_DIR)repository/url.cpp  $(SRC_DIR)repository/wxhttpfs.cpp  $(SRC_DIR)repository/wxzipfs.cpp  $(SRC_DIR)repository/fsext.cpp  $(SRC_DIR)repository/pyext.cpp  $(SRC_DIR)repository/integration.cpp   $(SRC_DIR)repository/IsoSurface.cpp $(SRC_DIR)repository/eprocess.cpp $(SRC_DIR)repository/olxvar.cpp \
	$(CFLAGS)

# unirun will create the obj/unirun directory and compile the source
unirun : $(OBJ_DIR)$@
	@echo "Making unirun this is relatively quick"
	@mkdir $(OBJ_DIR)unirun;
	@cd $(OBJ_DIR)unirun/;	$(CC) $(SRC_DIR)unirun/*.cpp  $(CFLAGS)

# olex will create the obj/olex directory and compile the source
olex : $(OBJ_DIR)$@
	@echo "Making olex this can take a while"
	@mkdir $(OBJ_DIR)olex;
	@cd $(OBJ_DIR)olex/; $(CC) $(SRC_DIR)olex/*.cpp $(CFLAGS)
# There now appears to be no files in the olex/macro directory?
#	@cd $(OBJ_DIR)olex/; $(CC) $(SRC_DIR)olex/*.cpp $(SRC_DIR)olex/macro/*.cpp $(CFLAGS)

# link will link the *.s objects created and build the binaries in the bin directory
link : $(OBJ_DIR)unirun$@ $(OBJ_DIR)olex$@
	@echo "Linking unirun and olex"
	@mkdir $(EXE_DIR); $(CC) $(OBJ_DIR)unirun/*.s $(OBJ_DIR)*.s -o $(EXE_DIR)unirun `wx-config --libs gl,core,html,net,aui --unicode --toolkit=gtk2` `python-config --libs --ldflags` -L. -fexceptions -g -rdynamic -O3
	@$(CC) $(OBJ_DIR)*.s $(OBJ_DIR)olex/*.s -o $(EXE_DIR)olex2 `wx-config --libs gl,core,html,net,aui --unicode --toolkit=gtk2` `python-config --libs --ldflags` -L. -fexceptions -g -rdynamic -O3

# install will allow a user with root/sudo permission to install a central copy of olex2
install-root:
	@echo "You must be root to install"

# This is my sandbox for testing variables
test:
	@echo "Testing"
	@echo $(HOME)

# install installs into the users home directory ~/olex this should be
# altered to install into the path provided by configure or the user at the top
# of this file
#
install: 
	@echo "Installing to local directory: " $(HOME) 
	@mkdir $(HOME)/olex;
	@cp -r $(EXE_DIR)* $(HOME)/olex/;
	@cp $(SRC_DIR)scripts/usettings.dat $(HOME)/olex/;
	@cp $(SRC_DIR)scripts/startup $(HOME)/olex/;
# experimental put startup script into bin so olex2 can be called from anywhere
	@cp $(SRC_DIR)scripts/startup $(HOME)/bin/olex2;
	@cp $(SRC_DIR)scripts/olex2.xpm $(HOME)/olex/;
	@cp $(SRC_DIR)scripts/olex2.desktop $(HOME)/Desktop/;
	@sed -i 's/PROXY/$(PROXY)/' $(HOME)/olex/usettings.dat;
	@sed -i 's|OLEX2BINPATH|$(OLEX_BIN)/olex2|g' $(HOME)/Desktop/olex2.desktop;
	@sed -i 's|OLEX2PATH|$(OLEX_INS)|g' $(HOME)/Desktop/olex2.desktop;	
# use sed to alter startup path for different install dir
	@chmod +x $(HOME)/olex/startup $(HOME)/bin/olex2;

# Update
# This function just updates the binaries of an existing olex2 install.
.PHONY : update
update: 
	@echo "Updating local directory: " $(HOME)
	@cp -r $(EXE_DIR)* $(HOME)/olex/;

# clean - remove build and binary
.PHONY : clean
clean_bin:
	@if test -d $(EXE_DIR); then cd $(EXE_DIR); if test -f olex2; then rm olex2; fi; if test -f unirun; then rm unirun; fi; fi;
	@cd $(SRC_DIR); if test -d $(EXE_DIR); then rmdir $(EXE_DIR); fi;
clean_olex:
	@if test -d $(OBJ_DIR)olex; then cd $(OBJ_DIR)olex; if ls *.s >/dev/null; then rm *.s; fi; fi;
	@if test -d $(OBJ_DIR); then cd $(OBJ_DIR); if test -d olex; then rmdir olex; fi; fi;
clean_unirun:
	@if test -d $(OBJ_DIR)unirun; then cd $(OBJ_DIR)unirun; if ls *.s >/dev/null; then rm *.s; fi; fi;
	@if test -d $(OBJ_DIR); then cd $(OBJ_DIR); if test -d unirun; then rmdir unirun; fi; fi;
clean_obj:
	@if test -d $(OBJ_DIR); then cd $(OBJ_DIR); if ls *.s >/dev/null; then rm *.s; fi; fi;
	@cd $(SRC_DIR); if test -d $(OBJ_DIR); then rmdir $(OBJ_DIR); fi;

clean:	clean_bin clean_olex clean_unirun clean_obj

.PHONY : help
help:
	@echo  'Cleaning targets:'
	@echo  '  clean           - Remove generated files but keep the installed'
	@echo  '                    binary and ~/olex directory intact'
	@echo  '  clean_bin       - Remove binaries only'
	@echo  ' '
	@echo  'Other generic targets:'
	@echo  'R  all             - Build all targets marked with [*]'
	@echo  '*  obj             - Build obj files'
	@echo  '*  olex            - Build olex specific files'
	@echo  '*  unirun          - Build unirun specific files'
	@echo  '*  link            - Links unirun and olex2 creating binaries'
	@echo  '   install-root    - Install to /usr/local/ **REQUIRES ROOT**'
	@echo  '   update          - Update the binaries of an existing install only'
	@echo  'R  install         - Install all to local folder'
	@echo  ' '
	@echo  'Execute "make all" to build all marked with a * '
	@echo	'Recommmended make targets all labelled R'
	@echo	'Execute "make install" to install to $(OLEX_INS)'
#	@echo  'For further info see the ./README file'


# DONE!

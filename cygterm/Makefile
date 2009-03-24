# Makefile for CygTerm

BINDIR = $(HOME)/bin

CC = gcc
CFLAGS = -O2 -fno-exceptions
#CFLAGS = -g -fno-exceptions
LIBS = -mwindows

EXE = cygterm.exe
SRC = $(EXE:.exe=.cc)
CFG = $(EXE:.exe=.cfg)
RES = $(EXE:.exe=.res)
ICO = $(EXE:.exe=.ico)
RC  = $(EXE:.exe=.rc)

$(EXE) : $(SRC) $(RES)
	$(CC) $(CFLAGS) -o $(EXE) $(SRC) $(RES) $(LIBS)
	strip $(EXE)

$(RC):
	echo 'icon ICON $(ICO)' > $(RC)

$(RES):	$(ICO) $(RC)
	windres -O coff -o $(RES) $(RC)

clean :
	rm -f $(EXE) $(RC) $(RES)

install : $(EXE)
	@ install -v $(EXE) $(BINDIR)/$(EXE)
	@ if [ ! -f $(BINDIR)/$(CFG) ]; then \
	    install -v $(CFG) $(BINDIR)/$(CFG) \
	; fi

uninstall :
	rm -f $(BINDIR)/$(EXE)
	rm -f $(BINDIR)/$(CFG)


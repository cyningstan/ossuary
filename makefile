#   ----------------------------------------------------------------
#   Ossuary
#   A DOS port of the ZX Spectrum rogue-like.
#   Main game source file.
# 
#   Copyright (C) Damian Gareth Walker 2020.
#   Created: 22-Jun-2020.
#

# Directories
SRCDIR = src
INCDIR = inc
DOCDIR = doc
PICDIR = pic
FONDIR = fon
OBJDIR = obj
BINDIR = bin
LIBCGA = cgalib
TGTDIR = ossuary

# Tool commands and their options
CC = wcc
LD = wcl
LIB = wlib
COPTS = -q -0 -W4 -ml -I=$(INCDIR)
LOPTS = -q

# Host specific build tools
!ifdef __LINUX__
CP = cp
MAKEGFX = dosbox -c "mount y $(%cwd)" &
	-c "y:" &
	-c "$(BINDIR)\\makegfx $[@ $@" &
	-c "exit"
RM = rm
!else
CP = copy
MAKEGFX = $(BINDIR)\\makegfx $[@ $@
RM = del
!endif

GAME : $(TGTDIR)/ossuary.exe &
	$(TGTDIR)/OSSUARY.GFX &
	$(TGTDIR)/ossuary.txt

# Game executables
$(TGTDIR)/ossuary.exe : $(OBJDIR)/ossuary.obj &
	$(OBJDIR)/display.obj &
	$(OBJDIR)/game.obj &
	$(OBJDIR)/scores.obj &
	$(OBJDIR)/errors.obj &
	$(LIBCGA)/cga-ml.lib
	$(LD) $(LOPTS) -fe=$@ $<
$(BINDIR)/makegfx.exe : $(OBJDIR)/makegfx.obj $(LIBCGA)/cga-ml.lib
	$(LD) $(LOPTS) -fe=$@ $<

# Game data files
$(TGTDIR)/OSSUARY.GFX : $(PICDIR)/ossuary.pic $(BINDIR)/makegfx.exe
	$(MAKEGFX)
$(TGTDIR)/ossuary.txt : $(DOCDIR)/ossuary.txt
	$(CP) $< $@

# Object files (Ossuary)
$(OBJDIR)/ossuary.obj : $(SRCDIR)/ossuary.c
	$(CC) $(COPTS) -fo=$@ $<
$(OBJDIR)/game.obj : $(SRCDIR)/game.c
	$(CC) $(COPTS) -fo=$@ $<
$(OBJDIR)/display.obj : $(SRCDIR)/display.c
	$(CC) $(COPTS) -fo=$@ $<
$(OBJDIR)/scores.obj : $(SRCDIR)/scores.c
	$(CC) $(COPTS) -fo=$@ $<
$(OBJDIR)/errors.obj : $(SRCDIR)/errors.c
	$(CC) $(COPTS) -fo=$@ $<
$(OBJDIR)/makegfx.obj : $(SRCDIR)/makegfx.c
	$(CC) $(COPTS) -fo=$@ $<

# CGALIB Wide Font Variant
$(LIBCGA)/cga-ml.lib : &
	$(OBJDIR)/screen.obj &
	$(OBJDIR)/bitmap.obj &
	$(OBJDIR)/font.obj
	$(LIB) $(LIBOPTS) $@ &
		+-$(OBJDIR)/screen.obj &
		+-$(OBJDIR)/bitmap.obj &
		+-$(OBJDIR)/font.obj
$(OBJDIR)/screen.obj : $(SRCDIR)/screen.c
	$(CC) $(COPTS) -fo=$@ $<
$(OBJDIR)/bitmap.obj : $(SRCDIR)/bitmap.c
	$(CC) $(COPTS) -fo=$@ $<
$(OBJDIR)/font.obj : $(SRCDIR)/font.c
	$(CC) $(COPTS) -fo=$@ $<

# Clean the repo
CLEAN :
	$(RM) $(TGTDIR)/ossuary.exe
	$(RM) $(TGTDIR)/OSSUARY.GFX
	$(RM) $(TGTDIR)/ossuary.txt
	$(RM) $(LIBCGA)/cga-ml.lib
	$(RM) $(BINDIR)/makegfx.exe

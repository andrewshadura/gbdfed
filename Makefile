#
# Copyright 2004 Computing Research Labs, New Mexico State University
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
# OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#
# $Id: Makefile,v 1.13 2004/02/17 15:24:06 mleisher Exp $
#
CC = gcc
CFLAGS = -g -Wall

OTHER = README CHANGES COPYRIGHTS Makefile Imakefile.lesstif Project.tmpl \
        xmbdfedrc xmbdfed.man

HDRS = FGrid.h FGridP.h GEShared.h GEdit.h GEditP.h GEditTB.h GEditTBP.h \
       GTest.h GTestP.h ProgBar.h ProgBarP.h bdf.h bdfP.h bitmaps.h \
       htext.h xmbdfed.h

SRCS = FGrid.c GEdit.c GEditTB.c GTest.c ProgBar.c bdf.c bdfcons.c bdffnt.c \
       bdfgname.c bdfgrab.c bdfgrid.c bdfotf.c bdfpkgf.c bdfpsf.c color.c \
       comment.c finfo.c fntin.c glyphed.c grab.c help.c ops.c otfin.c prog.c \
       props.c setup.c test.c xmbdfed.c

OBJS = FGrid.o GEdit.o GEditTB.o GTest.o ProgBar.o bdf.o bdfcons.o bdffnt.o \
       bdfgname.o bdfgrab.o bdfgrid.o bdfotf.o bdfpkgf.o bdfpsf.o color.o \
       comment.o finfo.o fntin.o glyphed.o grab.o help.o ops.o otfin.o prog.o \
       props.o setup.o test.o xmbdfed.o $(HBF_OBJS)

#
# Uncomment these if you have the FreeType library and want to use it to
# import OpenType fonts.
#
#FTYPE_INCS = -I/usr/local/include
#FTYPE_LIBS = -L/usr/local/lib -lfreetype
#FTYPE_DEFS = -DHAVE_FREETYPE

#
# Uncomment these if you have the hbf.h and hbf.c files in the current
# directory and the patch has been applied.
#
#HBF_HDRS = hbf.h
#HBF_OBJS = hbf.o
#HBF_DEFS = -DHAVE_HBF

#
# Specify the various defines needed for HBF fonts.  If you do not have GNU
# zip/unzip, then don't worry, you just can not open HBF fonts that refer to
# files with a ".gz" extension.  If you don't care about HBF fonts, just
# comment this line out.
#
HBFDEFS = -Dunix -DIN_MEMORY -DGUNZIP_CMD="\"/usr/local/bin/gunzip -c\""

#
# Set the defines used for all files except the HBF support.
#
DEFS = -DHAVE_XLIB $(FTYPE_DEFS) $(HBF_DEFS)

#
# Uncomment these for SunOS.
#
#INCS = -I/usr/local/X11/include -I/usr/local/Motif-2.0/include $(FTYPE_INCS)
#LIBS = -L/usr/local/Motif-2.0/lib -lXm \
#       -L/usr/local/X11/lib -lXpm -lXmu -lXt -lXext -lX11 -lSM -lICE \
#       $(FTYPE_LIBS)

#
# Uncomment these for Solaris.
#
INCS = -I/usr/openwin/include -I/usr/dt/include $(FTYPE_INCS)
LIBS = -R/usr/openwin/lib -R/usr/dt/lib -L/usr/dt/lib -lXm \
       -L/usr/openwin/lib -lXmu -lXt -lXext -lX11 $(FTYPE_LIBS)

#
# Uncomment these for Linux.
#
#INCS = -I/usr/X11/include $(FTYPE_INCS)
#LIBS = -L/usr/X11/lib -lXm -lXpm -lXmu -lXt -lXext -lX11 -lSM -lICE $(FTYPE_LIBS)

#
# Uncomment these for HPUX.
#
# MIT Athena include files are in /usr/local/include/X11 and subdirectories.
# They *do not* come with HP-UX 10.20 or earlier; you can get them from e.g.
# ftp://hpux.csc.liv.ac.uk/hpux/X11/Core/Xaw-5.00/
#
#CC = cc
#CFLAGS = -Aa -Wp,-H,32384 -D_HPUX_SOURCE
#INCS = -I/usr/include/X11R5 -I/usr/local/x11r6/include \
#       -I/usr/include/Motif1.2 $(FTYPE_INCS)
#LIBS = -L/usr/lib/Motif1.2 -lXm -L/usr/lib/X11R5 -L/usr/local/x11r6/lib \
#       -lXmu -lXt -lX11 $(FTYPE_LIBS)

#
# Another HP/UX 10.20 setup for X11R6.
#
#CC = cc
#CFLAGS = -Aa -Wp,-H,32384 -D_HPUX_SOURCE
#INCS = -I/usr/include/X11R6 -I/usr/contrib/X11R6/include \
#       -I/usr/include/Motif1.2 $(FTYPE_INCS)
#LIBS = -L/usr/lib/Motif1.2 -lXm -L/usr/lib/X11R6 -L/usr/contrib/X11R6/lib \
#       -lXmu -lXt -lX11 $(FTYPE_LIBS)

#
# Uncomment these for SCO.
# 
#INCS = $(FTYPE_INCS)
#LIBS = -lXm -lXmu -lXt -lXext -lX11 -lsocket $(FTYPE_LIBS)

#
# Uncomment these for IRIX 6.5.2.
#
#INCS = -I/usr/include/Xm -I/usr/include/X11 $(FTYPE_INCS)
#LIBS = -L/usr/Motif-1.2/lib32 -lXm \
#       -L/usr/lib/X11 -lXmu -lXt -lXext -lX11 -lSM -lICE $(FTYPE_LIBS)

all: xmbdfed

xmbdfed: $(OBJS)
	$(PURIFY) $(CC) $(STATIC) $(CFLAGS) -o xmbdfed $(OBJS) $(LIBS)

hbf.o: hbf.c
	$(CC) $(CFLAGS) $(DEFS) $(HBFDEFS) $(INCS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

tar:
	gtar zcf xmbdfed.tar.gz $(OTHER) $(HDRS) $(SRCS)

clean:
	/bin/rm -f *.o *BAK *CKP *~

realclean: clean
	/bin/rm -f xmbdfed

#
# Local dependencies.
#
bdfcons.o: bdfcons.c bdfP.h bdf.h 
bdf.o: bdf.c bdfP.h bdf.h $(HBF_HDRS)
bdffnt.o: bdffnt.c bdfP.h bdf.h 
bdfgname.o: bdfgname.c bdfP.h bdf.h 
bdfgrab.o: bdfgrab.c bdfP.h bdf.h 
bdfgrid.o: bdfgrid.c bdfP.h bdf.h 
bdfotf.o: bdfotf.c bdfP.h bdf.h 
bdfpkgf.o: bdfpkgf.c bdfP.h bdf.h 
bdfpsf.o: bdfpsf.c bdfP.h bdf.h 
color.o: color.c bdfP.h bdf.h xmbdfed.h
comment.o: comment.c FGrid.h bdfP.h bdf.h xmbdfed.h
FGrid.o: FGrid.c FGridP.h FGrid.h bdfP.h bdf.h 
finfo.o: finfo.c FGrid.h bdfP.h bdf.h xmbdfed.h
fntin.o: fntin.c FGrid.h bdfP.h bdf.h GTest.h xmbdfed.h
GEdit.o: GEdit.c GEditP.h GEdit.h bdfP.h bdf.h GEShared.h 
GEditTB.o: GEditTB.c GEditTBP.h GEditTB.h bdfP.h bdf.h GEShared.h bitmaps.h
glyphed.o: glyphed.c FGrid.h bdfP.h bdf.h GEdit.h GEShared.h GEditTB.h \
           xmbdfed.h
grab.o: grab.c FGrid.h bdfP.h bdf.h xmbdfed.h
GTest.o: GTest.c GTestP.h GTest.h bdfP.h bdf.h 
$(HBF_OBJS): $(HBF_SRCS) $(HBF_HDRS)
help.o: help.c bdf.h xmbdfed.h htext.h
ops.o: ops.c FGrid.h bdfP.h bdf.h GTest.h xmbdfed.h
otfin.o: otfin.c FGrid.h bdfP.h bdf.h GTest.h xmbdfed.h
ProgBar.o: ProgBar.c ProgBarP.h ProgBar.h
prog.o: prog.c ProgBar.h bdf.h xmbdfed.h
props.o: props.c FGrid.h bdfP.h bdf.h xmbdfed.h
setup.o: setup.c FGrid.h bdfP.h bdf.h xmbdfed.h
test.o: test.c FGrid.h bdfP.h bdf.h xmbdfed.h GTest.h
xmbdfed.o: xmbdfed.c FGrid.h bdfP.h bdf.h GTest.h xmbdfed.h

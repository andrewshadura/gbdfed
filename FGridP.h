/*
 * Copyright 2001 Computing Research Labs, New Mexico State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _h_XmuttFontGridP
#define _h_XmuttFontGridP

/*
 * $Id: FGridP.h,v 1.12 2001/09/19 21:00:41 mleisher Exp $
 */

#include <stdio.h>
#include <Xm/XmP.h>
#if (XmVERSION == 1) && (XmREVISION >= 2)
#include <Xm/PrimitiveP.h>
#elif (XmVERSION == 2)
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif
#include "FGrid.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))
#endif

#ifndef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))
#endif

/*
 * The Font Grid clipboard for handling selections.
 */
#define XmuttFONTGRID_CLIPBOARD(dsp) \
    XInternAtom(dsp, "FONTGRID_CLIPBOARD", False)

/*
 * The only (currently) clipboard type available.
 */
#define XmuttFONTGRID_GLYPHLIST(dsp) \
        XInternAtom(dsp, "FONTGRID_GLYPHLIST", False)

#define FGRID_MAX_COLS     16
#define FGRID_MAX_ROWS     16
#define FGRID_DEFAULT_COLS 16
#define FGRID_DEFAULT_ROWS 8

#define FGRID_MODIFIED 1
#define FGRID_SELECTED 2

#define IsSelected(code, map) (map[(code) >> 5] & (1 << ((code) & 31)))
#define Select(code, map) (map[(code) >> 5] |= (1 << ((code) & 31)))
#define Unselect(code, map) (map[(code) >> 5] &= ~(1 << ((code) & 31)))

typedef struct {
    long minpage;
    long maxpage;
    long npage;
    long ppage;

    long pageno;
    long bcode;
    long sel_start;
    long sel_end;

    unsigned long selmap[2048];
} _XmuttFGridPageInfo;

/*
 * Class and instance structures.
 */
typedef struct _XmuttFGridClassPart {
    int ref_cnt;
    XtPointer extension;
} XmuttFGridClassPart;

typedef struct _XmuttFGridClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmuttFGridClassPart fgrid_class;
} XmuttFGridClassRec;

typedef struct _XmuttFGridPart {
    /*
     * Public members.
     */
    int base;
    bdf_font_t *font;
    XtCallbackList activate;
    XtCallbackList page;
    XtCallbackList code_page;
    XtCallbackList sel_start;
    XtCallbackList sel_extend;
    XtCallbackList sel_end;
    XtCallbackList modified;
    Dimension cell_rows;
    Dimension cell_cols;
    Dimension mwidth;
    Dimension mheight;

    Boolean noblanks;
    Boolean unencoded;
    Boolean overwrite_mode;

    long initial_glyph;

    /*
     * The horizontal and vertical resolutions, determined dynamically
     * if not supplied by user.
     */
    long hres;
    long vres;

    /*
     * The default point size.
     */
    long ptsize;

    /*
     * The font spacing.
     */
    long spacing;

    /*
     * The multi-click time.
     */
    int mclick;

    /*
     * The display orientation.
     */
    unsigned char orientation;

    /*
     * Always adjust the rows and columns to powers of 2.
     */
    Boolean power2;

    /*
     * Bits per pixel.
     */
    int bpp;

    /*
     * The pixels with the colors.
     */
    unsigned long *colors;

    /*******************************************************************
     *
     * Private members.
     *
     *******************************************************************/

    XFontStruct *idfnt;
    GC gc;
    GC invgc;
    XPoint *gpoints;
    unsigned long gpoints_size;
    unsigned long gpoints_used;

    Dimension label_height;
    Dimension cell_width;
    Dimension cell_height;
    Position xoff;
    Position yoff;

    int count;
    long pagesize;

    _XmuttFGridPageInfo npage;
    _XmuttFGridPageInfo upage;

    Boolean owns_clipboard;
    Boolean hasfocus;
    Boolean resizing;
    Boolean clearwin;
    Boolean client_page;
    Boolean no_sel_callback;
    Boolean from_keyboard;

    int mclick_time;
    Time last_click;

    long pxsize;
    long defhres;
    long defvres;

    bdf_glyphlist_t clipboard;
} XmuttFGridPart;

typedef struct _XmuttFGridRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmuttFGridPart fgrid;
} XmuttFGridRec;

#ifdef __cplusplus
}
#endif

#endif /* _h_XmuttFontGridP */

/*
 * Copyright 2004 Computing Research Labs, New Mexico State University
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
#ifndef _h_XmuttGEditP
#define _h_XmuttGEditP

/*
 * $Id: GEditP.h,v 1.7 2004/01/29 17:15:35 mleisher Exp $
 */

#include <stdio.h>
#include <Xm/XmP.h>
#if (XmVERSION == 1) && (XmREVISION >= 2)
#include <Xm/PrimitiveP.h>
#elif (XmVERSION == 2)
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif
#include <X11/Xmu/Atoms.h>
#include "GEdit.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))
#endif

#ifndef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))
#endif

#ifndef MYABS
#define MYABS(xx) ((xx) < 0 ? -(xx) : (xx))
#endif

/*
 * The Glyph Edit clipboard for handling selections.
 */
#define XmuttGLYPHEDIT_CLIPBOARD(dsp) \
    XInternAtom(dsp, "GLYPHEDIT_CLIPBOARD", False)

/*
 * Types that the Glyph Edit clipboard knows about:
 *
 *   GLYPHEDIT_BITMAP   - A bitmap with width and height.
 *   GLYPHEDIT_GLYPH    - The glyph in binary form.
 *   GLYPHEDIT_BDF_CHAR - The glyph in BDF syntax.
 */
#define XmuttGLYPHEDIT_BITMAP(dsp) \
    XInternAtom(dsp, "GLYPHEDIT_BITMAP", False)
#define XmuttGLYPHEDIT_GLYPH(dsp) \
    XInternAtom(dsp, "GLYPHEDIT_GLYPH", False)
#define XmuttGLYPHEDIT_BDF_CHAR(dsp) \
    XInternAtom(dsp, "GLYPHEDIT_BDF_CHAR", False)

/*
 * Class and instance structures.
 */
typedef struct _XmuttGlyphEditClassPart {
    Cursor cursor;
    int refcnt;
    XtPointer extension;
} XmuttGlyphEditClassPart;

typedef struct _XmuttGlyphEditClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmuttGlyphEditClassPart gedit_class;
} XmuttGlyphEditClassRec;

typedef struct {
    /*
     * Public members.
     */
    bdf_glyph_grid_t *grid;

    Pixel bbxpix;
    Pixel selpix;
    Pixel cursorpix;

    int default_pixel_size;

    XtCallbackList change;
    XtCallbackList move;
    XtCallbackList opswitch;
    XtCallbackList modified;

    Boolean show_x_height;
    Boolean show_cap_height;

    unsigned long *colors;

    /*
     * Private members.
     */
    GC gridgc;
    GC bbxgc;
    GC pixgc;
    GC cleargc;
    GC selgc;

    /*
     * Current color index value.
     */
    int cidx;

    /*
     * Last color set.
     */
    int lcolor;

    /*
     * Size of one side of the square pixel.
     */
    int pixel_size;

    /*
     * The last X and Y position of the pointer.
     */
    Position last_x;
    Position last_y;

    /*
     * The two points that defines the selection rectangle.
     */
    XPoint sel_start;
    XPoint sel_end;

    /*
     * Current operation (draw, select, move, copy).
     */
    int op;

    /*
     * Pending copy or move operation.
     */
    int pending_op;

    /*
     * Boolean indicating whether the widget owns the GLYPHEDIT_CLIPBOARD.
     */
    Boolean owns_clipboard;

    /*
     * Boolean indicating whether the widget has the focus or not.
     */
    Boolean hasfocus;
} XmuttGlyphEditPart;

typedef struct _XmuttGlyphEditRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmuttGlyphEditPart gedit;
} XmuttGlyphEditRec;

#ifdef __cplusplus
}
#endif

#endif /* _h_XmuttGEditP */

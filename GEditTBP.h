/*
 * Copyright 2000 Computing Research Labs, New Mexico State University
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
#ifndef _h_XmuttGEditToolboxP
#define _h_XmuttGEditToolboxP

/*
 * $Id: GEditTBP.h,v 1.5 2000/03/16 20:08:49 mleisher Exp $
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
#include "GEditTB.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A simple structure representing the buttons on the toolbox.
 */
typedef struct {
    char *help;
    int x;
    int y;
    Pixmap pixmap;
    Region region;
    int other_toggles[2];
    Boolean set;
    Boolean toggle;
} _XmuttGEditTBoxButton;

typedef struct {
    int x;
    int y;
    Region region;
} _XmuttGEditTBoxColor;

/*
 * Class and instance structures.
 */
typedef struct _XmuttGlyphEditTBoxClassPart {
    Pixmap draw;
    Pixmap move;
    Pixmap copy;
    Pixmap fliph;
    Pixmap flipv;
    Pixmap shear;
    Pixmap rleft;
    Pixmap rright;
    Pixmap rotate;
    Pixmap uleft;
    Pixmap up;
    Pixmap uright;
    Pixmap left;
    Pixmap right;
    Pixmap dleft;
    Pixmap down;
    Pixmap dright;
    int refcnt;
    XtPointer extension;
} XmuttGlyphEditTBoxClassPart;

typedef struct _XmuttGlyphEditTBoxClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmuttGlyphEditTBoxClassPart gedittb_class;
} XmuttGlyphEditTBoxClassRec;

typedef struct {
    /*
     * Public members.
     */
    XtCallbackList help;
    XtCallbackList activate;

    bdf_bitmap_t *gimage;

    unsigned long *colors;

    /*
     * Private members.
     */
    GC gc;
    GC fillgc;
    GC armgc;
    GC armfillgc;
    GC tsgc;
    GC bsgc;

    int cidx;

    XtIntervalId timer;
    int timer_button;
    int timer_count;

    Pixel arm_color;

    Region image_region;

    int pressed;
    int last;

    XPoint *points;
    unsigned long points_size;
    unsigned long points_used;

    /*
     * Width and height of the color spots.
     */
    int cbw;
    int cbh;

    _XmuttGEditTBoxColor cols[16];
    _XmuttGEditTBoxButton buttons[18];

    Boolean hasfocus;
} XmuttGlyphEditTBoxPart;

typedef struct _XmuttGlyphEditTBoxRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmuttGlyphEditTBoxPart gedittb;
} XmuttGlyphEditTBoxRec;

#ifdef __cplusplus
}
#endif

#endif /* _h_XmuttGEditToolboxP */

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
#ifndef _h_ProgBarP
#define _h_ProgBarP

/*
 * $Id: ProgBarP.h,v 1.5 2000/03/16 20:08:50 mleisher Exp $
 */

#include <stdio.h>
#include <Xm/XmP.h>
#if (XmVERSION == 1) && (XmREVISION >= 2)
#include <Xm/PrimitiveP.h>
#elif (XmVERSION == 2)
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif
#include "ProgBar.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))
#endif

#ifndef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))
#endif

typedef struct {
    XtPointer extension;
} XmuttProgBarClassPart;

typedef struct _XmuttProgBarClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmuttProgBarClassPart progress_class;
} XmuttProgBarClassRec;

typedef struct {
    /*
     * Public members.
     */
    unsigned long minval;
    unsigned long maxval;
    unsigned long currval;

    /*
     * Update the status bar every certain percentage.
     */
    unsigned long percent;

    /*
     * The message and number fonts.
     */
    XmFontList font;

    /*
     * The label string.
     */
    XmString label;

    /*
     * Margins.
     */
    Dimension mwidth;
    Dimension mheight;

    /*
     * Update direction.
     */
    int dir;

    Boolean number_only;

    /*
     * Private members.
     */
    GC labelgc;
    GC numgc;
    GC bargc;

    /*
     * Strings for labeling beginning and end of progress bar.
     */
    Dimension percheight;
    Dimension label_height;
    Dimension label_width;

    /*
     * The bounds of the rectangle holding the bar.
     */
    XRectangle bar;

    Dimension lastwidth;

    unsigned long lastpercent;

    Boolean hasfocus;
} XmuttProgBarPart;

typedef struct _XmuttProgBarRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmuttProgBarPart progress;
} XmuttProgBarRec;

#ifdef __cplusplus
}
#endif

#endif /* _h_ProgBarP */

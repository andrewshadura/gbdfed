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
#ifndef _h_XmuttGTestP
#define _h_XmuttGTestP

#include <stdio.h>
#include <Xm/XmP.h>
#if (XmVERSION == 1) && (XmREVISION >= 2)
#include <Xm/PrimitiveP.h>
#elif (XmVERSION == 2)
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif
#include "GTest.h"

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
 * Class and instance structures.
 */
typedef struct _XmuttGlyphTestClassPart {
    XtPointer extension;
} XmuttGlyphTestClassPart;

typedef struct _XmuttGlyphTestClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmuttGlyphTestClassPart gtest_class;
} XmuttGlyphTestClassRec;

typedef struct {
    bdf_font_t *font;
    bdf_glyph_t *glyph;
} _XmuttGlyphTestGlyph;

typedef struct {
    _XmuttGlyphTestGlyph *glyphs;
    unsigned long glyphs_used;
    unsigned long glyphs_size;
    XPoint cpoint;
    Dimension width;
    Dimension height;
    bdf_bbx_t bbx;
} _XmuttGlyphTestLine;

typedef struct {
    /*
     * Public members.
     */
    int dir;

    XtCallbackList add;
    XtCallbackList update;

    unsigned long *colors;

    Boolean show_baseline;

    /*
     * Private members.
     */
    GC glyphgc;

    _XmuttGlyphTestLine line;

    XPoint *image;
    unsigned long image_used;
    unsigned long image_size;

    Boolean unencoded;

    /*
     * Boolean indicating whether the widget has the focus or not.
     */
    Boolean hasfocus;
} XmuttGlyphTestPart;

typedef struct _XmuttGlyphTestRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmuttGlyphTestPart gtest;
} XmuttGlyphTestRec;

#ifdef __cplusplus
}
#endif

#endif /* _h_XmuttGTestP */

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
#ifndef _h_XmuttGlyphTest
#define _h_XmuttGlyphTest

/*
 * $Id: GTest.h,v 1.4 2000/03/16 20:08:49 mleisher Exp $
 */
#include <Xm/Xm.h>
#include "bdfP.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A macro for prototypes.
 */
#undef __
#ifndef _NO_PROTO
#define __(x) x
#else
#define __(x) ()
#endif

/*
 * Widget class and widget declarations.
 */
externalref WidgetClass xmuttGlyphTestWidgetClass;

typedef struct _XmuttGlyphTestClassRec *XmuttGlyphTestWidgetClass;
typedef struct _XmuttGlyphTestRec      *XmuttGlyphTestWidget;

#ifndef XmuttIsGlyphTest
#define XmuttIsGlyphTest(w) XtIsSubclass(w, xmuttGlyphTestWidgetClass)
#endif /* XmuttIsGlyphTest */

/**************************************************************************
 *
 * Resources.
 *
 **************************************************************************/

#define XmuttGlyphTestLeftToRight 0
#define XmuttGlyphTestRightToLeft 1

#define XmNdirection "direction"
#define XmCDirection "Direction"

#define XmNshowBaseline "showBaseline"
#define XmCShowBaseline "ShowBaseline"

#define XmNaddGlyphCallback "addGlyphCallback"
#define XmCAddGlyphCallback "AddGlyphCallback"

#define XmNcolorList "colorList"
#define XmCColorList "ColorList"

extern void XmuttGlyphTestAddGlyph __((Widget w, bdf_font_t *font,
                                       bdf_glyph_t *glyph));

extern void XmuttGlyphTestErase __((Widget w));

extern void XmuttGlyphTestRedisplay __((Widget w));

extern void XmuttGlyphTestRemoveFont __((Widget w, bdf_font_t *font));

extern void XmuttGlyphTestResize __((Widget w));

#undef __

#ifdef __cplusplus
}
#endif

#endif /* _h_XmuttGlyphTest */

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
#ifndef _h_XmuttGEditToolbox
#define _h_XmuttGEditToolbox

/*
 * $Id: GEditTB.h,v 1.4 2000/03/16 20:08:48 mleisher Exp $
 */

#include <Xm/Xm.h>
#include "bdfP.h"
#include "GEShared.h"

/*
 * A macro for prototypes.
 */
#undef __
#ifndef _NO_PROTO
#define __(x) x
#else
#define __(x) ()
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Widget class and widget declarations.
 */
externalref WidgetClass xmuttGlyphEditToolboxWidgetClass;

typedef struct _XmuttGlyphEditTBoxClassRec *XmuttGlyphEditToolboxWidgetClass;
typedef struct _XmuttGlyphEditTBoxRec      *XmuttGlyphEditToolboxWidget;

#ifndef XmuttIsGlyphEditToolbox
#define XmuttIsGlyphEditToolbox(w) \
XtIsSubclass(w, xmuttGlyphEditToolboxWidgetClass)
#endif /* XmuttIsGlyphEditToolbox */

#define XmNglyphImage "glyphImage"
#define XmCGlyphImage "GlyphImage"

#define XmNcontextHelpCallback "contextHelpCallback"
#define XmCContextHelpCallback "ContextHelpCallback"

#define XmNcolorList "colorList"
#define XmCColorList "ColorList"

/*
 * Values that can appear in the `reason' field of the callback structure.
 */
#define XmuttGLYPHEDIT_CONTEXT_HELP -1
#define XmuttGLYPHEDIT_OPERATION    -2
#define XmuttGLYPHEDIT_ACTION       -3
#define XmuttGLYPHEDIT_COLOR        -4

/*
 * Callback structure used for XmNcontextHelpCallback and XmNactivateCallback.
 * The `op_action' field is filled with one of the values from the
 * "geshared.h" file.
 */
typedef struct {
    int reason;
    XEvent *event;
    char *help;
    int op_action;
    int color;
} XmuttGlyphEditToolboxCallbackStruct;

extern void XmuttGlyphEditToolboxSetToggle __((Widget w, int toggle,
                                               Boolean activate));

extern int XmuttGlyphEditToolboxQueryToggle __((Widget w));

extern void XmuttGlyphEditToolboxSetColor __((Widget w, int color));

#ifdef __cplusplus
}
#endif

#undef __

#endif /* _h_XmuttGEditToolbox */

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
#ifndef _h_XmuttGEdit
#define _h_XmuttGEdit

/*
 * $Id: GEdit.h,v 1.6 2000/03/16 20:08:48 mleisher Exp $
 */

#include <Xm/Xm.h>
#include "bdfP.h"
#include "GEShared.h"

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
externalref WidgetClass xmuttGlyphEditWidgetClass;

typedef struct _XmuttGlyphEditClassRec *XmuttGlyphEditWidgetClass;
typedef struct _XmuttGlyphEditRec      *XmuttGlyphEditWidget;

#ifndef XmuttIsGlyphEdit
#define XmuttIsGlyphEdit(w) XtIsSubclass(w, xmuttGlyphEditWidgetClass)
#endif /* XmuttIsGlyphEdit */

/**************************************************************************
 *
 * Resources.
 *
 **************************************************************************/

#define XmNglyphGrid "glyphGrid"
#define XmCGlyphGrid "GlyphGrid"

#define XmNcursorColor "cursorColor"
#define XmCCursorColor "CursorColor"

#define XmNbaselineColor "baselineColor"
#define XmCBaselineColor "BaselineColor"

#define XmNselectionColor "selectionColor"
#define XmCSelectionColor "SelectionColor"

#define XmNimageChangeCallback "imageChangeCallback"
#define XmCImageChangeCallback "ImageChangeCallback"

#define XmNpointerMovedCallback "pointerMovedCallback"
#define XmCPointerMovedCallback "PointerMovedCallback"

#define XmNoperationSwitchCallback "operationSwitchCallback"
#define XmCOperationSwitchCallback "OperationSwitchCallback"

#define XmNglyphModifiedCallback "glyphModifiedCallback"
#define XmCGlyphModifiedCallback "GlyphModifiedCallback"

#define XmNdefaultPixelSize "defaultPixelSize"
#define XmCDefaultPixelSize "DefaultPixelSize"

#define XmNshowXHeight "showXHeight"
#define XmCShowXHeight "ShowXHeight"

#define XmNshowCapHeight "showCapHeight"
#define XmCShowCapHeight "ShowCapHeight"

#define XmNcolorList "colorList"
#define XmCColorList "ColorList"

/**************************************************************************
 *
 * Callback macros and structures.
 *
 **************************************************************************/

/*
 * Callback reasons.
 */
#define XmuttGLYPHEDIT_IMAGE_CHANGE     -1
#define XmuttGLYPHEDIT_POINTER_MOVED    -2
#define XmuttGLYPHEDIT_OPERATION_CHANGE -3
#define XmuttGLYPHEDIT_GLYPH_MODIFIED   -4
#define XmuttGLYPHEDIT_COLOR_CHANGE     -5

/*
 * Callback structures.
 */
typedef struct {
    int reason;
    XEvent *event;
    bdf_bitmap_t *image;
    int operation;
    bdf_metrics_t *metrics;
    Position x;
    Position y;
    int color;
} XmuttGlyphEditUpdateCallbackStruct;

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

/*
 * Macros used with the XmuttGlyphEditFlip() function.
 */
#define XmuttGLYPHEDIT_HORIZONTAL 1
#define XmuttGLYPHEDIT_VERTICAL   2

/*
 * Get the encoding of the glyph.
 */
extern long XmuttGlyphEditEncoding __((Widget w));

/*
 * Get the current set of metrics from the glyph editor.
 */
extern void XmuttGlyphEditMetrics __((Widget w, bdf_metrics_t *metrics,
                                      Boolean font));

/*
 * Changes the device width, width, and height values from the metrics
 * supplied.
 */
extern void XmuttGlyphEditSetMetrics __((Widget w, bdf_metrics_t *metrics));

/*
 * Adjust the font spacing and the mono width if necessary.
 */
extern void XmuttGlyphEditSetSpacing __((Widget w, unsigned short spacing,
                                         unsigned short monowidth));

/*
 * Check to see if the glyph was modified.
 */
extern Boolean XmuttGlyphEditModified __((Widget w));

/*
 * Change the flag that indicates whether the glyph has been modified or not.
 */
extern void XmuttGlyphEditSetModified __((Widget w, Boolean modified));

/*
 * Check to see if the glyph editor has a selection or not.
 */
extern Boolean XmuttGlyphEditHasSelection __((Widget w));

/*
 * Check to see if the glyph edit clipboard is empty or not.
 */
extern Boolean XmuttGlyphEditClipboardEmpty __((Widget w));

/*
 * Retrieve the image from the grid.
 */
extern void XmuttGlyphEditImage __((Widget w, bdf_bitmap_t *image));

/*
 * Retrieve the grid.
 */
extern bdf_glyph_grid_t *XmuttGlyphEditGrid __((Widget w));

/*
 * Retrieve the glyph from the grid.
 */
extern bdf_glyph_t *XmuttGlyphEditGlyph __((Widget w, Boolean *unencoded));

/*
 * Crop the glyph bitmap to get rid of empty rows and columns around the
 * glyph.
 */
extern void XmuttGlyphEditCrop __((Widget w));

/*
 * Shift the bitmap horizontally, vertically or a combination of both.
 */
extern void XmuttGlyphEditShift __((Widget w, short xcount, short ycount));

/*
 * Rotate the bitmap clockwise (positive count) or counter-clockwise
 * (negative count).
 */
extern void XmuttGlyphEditRotate __((Widget w, short degrees));

/*
 * Shear the bitmap clockwise (positive count) or counter-clockwise
 * (negative count).  Limited to the range of [-20,20] degrees.
 */
extern void XmuttGlyphEditShear __((Widget w, short degrees));

/*
 * Make the glyph bold.
 */
extern void XmuttGlyphEditEmbolden __((Widget w));

/*
 * Flip the bitmap horizontally or vertically.
 */
extern void XmuttGlyphEditFlip __((Widget w, short direction));

/*
 * Change to the draw, select, move, or copy operation.
 */
extern void XmuttGlyphEditChangeOperation __((Widget w, int op));

/*
 * Change the current color.
 */
extern void XmuttGlyphEditChangeColor __((Widget w, int color));

/*
 * Insert a bitmap from some outside source.
 */
extern void XmuttGlyphEditInsertBitmap __((Widget w, bdf_bitmap_t *bitmap));

/*
 * Functions explicitly for importing and exporting XBM bitmaps.
 */
extern int XmuttGlyphEditImportXBM __((Widget w, String filename));
extern int XmuttGlyphEditExportXBM __((Widget w, String filename));

extern void XmuttGlyphEditCopySelection __((Widget w));
extern void XmuttGlyphEditCutSelection __((Widget w));
extern void XmuttGlyphEditPasteSelection __((Widget w));

extern void XmuttGlyphEditSelectAll __((Widget w));

#undef __

#ifdef __cplusplus
}
#endif

#endif /* _h_XmuttGEdit */

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
#ifndef _h_XmuttFontGrid
#define _h_XmuttFontGrid

/*
 * $Id: FGrid.h,v 1.9 2000/03/16 20:08:47 mleisher Exp $
 */

/*
 * This is the BDF Font Grid widget.
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

#ifndef XmNbdfFont
#define XmNbdfFont "bdfFont"
#define XmCBDFFont "BDFFont"
#endif

#define XmNcodeBase "codeBase"
#define XmCCodeBase "CodeBase"

#define XmNpointSize "pointSize"
#define XmCPointSize "PointSize"

#define XmNhorizontalResolution "horizontalResolution"
#define XmCHorizontalResolution "HorizontalResolution"

#define XmNverticalResolution "verticalResolution"
#define XmCVerticalResolution "VerticalResolution"

#define XmNfontSpacing "fontSpacing"
#define XmCFontSpacing "FontSpacing"

#define XmNcellRows "cellRows"
#define XmCCellRows "CellRows"

#define XmNcellColumns "cellColumns"
#define XmCCellColumns "CellColumns"

#define XmNskipBlankPages "skipBlankPages"
#define XmCSkipBlankPages "SkipBlankPages"

#define XmNdisplayUnencoded "displayUnencoded"
#define XmCDisplayUnencoded "DisplayUnencoded"

#define XmNpageCallback "pageCallback"
#define XmCPageCallback "PageCallback"

#define XmNcodePageCallback "codePageCallback"
#define XmCCodePageCallback "CodePageCallback"

#define XmNstartSelectionCallback "startSelection"
#define XmCStartSelectionCallback "StartSelection"

#define XmNextendSelectionCallback "extendSelection"
#define XmCExtendSelectionCallback "ExtendSelection"

#define XmNendSelectionCallback "endSelection"
#define XmCEndSelectionCallback "EndSelection"

#define XmNmodifiedCallback "fontModified"
#define XmCModifiedCallback "FontModified"

#define XmNmultiClickTime "multiClickTime"
#define XmCMultiClickTime "MultiClickTime"

#define XmNgridOverwriteMode "gridOverwriteMode"
#define XmCGridOverwriteMode "GridOverwriteMode"

#define XmNpowersOfTwo "powersOfTwo"
#define XmCPowersOfTwo "PowersOfTwo"

#define XmNbitsPerPixel "bitsPerPixel"
#define XmCBitsPerPixel "CitsPerPixel"

#define XmNcolorList "colorList"
#define XmCColorList "ColorList"

#define XmNinitialGlyph "initialGlyph"
#define XmCInitialGlyph "InitialGlyph"

externalref WidgetClass xmuttFontGridWidgetClass;

typedef struct _XmuttFGridClassRec *XmuttFontGridWidgetClass;
typedef struct _XmuttFGridRec      *XmuttFontGridWidget;

#ifndef XmuttIsFontGrid
#define XmuttIsFontGrid(w) XtIsSubclass(w, xmuttFontGridWidgetClass)
#endif /* XmuttIsFontGrid */

typedef struct {
    long previous_page;
    long current_page;
    long next_page;
    Boolean unencoded;
    long num_unencoded;
    long num_encoded;
} XmuttFontGridPageInfoStruct;

#define XmuttFG_SELECTION_INFO   -1
#define XmuttFG_ACTIVATE          0
#define XmuttFG_START_SELECTION   1
#define XmuttFG_EXTEND_SELECTION  2
#define XmuttFG_END_SELECTION     3
#define XmuttFG_DELETE            4
#define XmuttFG_INSERT            5
#define XmuttFG_REPLACE           6
#define XmuttFG_NAME_CHANGE       7
#define XmuttFG_RESIZE            8
#define XmuttFG_MODIFIED          9
#define XmuttFG_GLYPH_NAME_CHANGE 10

typedef struct {
    int reason;
    long start;
    long end;
    int base;
    bdf_glyph_t *glyphs;
    unsigned long num_glyphs;
    Boolean unencoded;
} XmuttFontGridSelectionStruct;

typedef struct {
    int reason;
    long start;
    long end;
    char *font_name;
    Boolean unencoded;
} XmuttFontGridModifiedCallbackStruct;

typedef struct {
    char *name;
    char *comments;
    char *messages;
    long bits_per_pixel;
    long default_char;
    long point_size;
    unsigned short monowidth;
    unsigned short spacing;
    long font_ascent;
    long font_descent;
    long resolution_x;
    long resolution_y;
    bdf_bbx_t bbx;
} XmuttFontGridFontInfoStruct;

/*
 * Paste type operations for XmuttFontGridPasteSelection().
 */
#define XmuttFG_NORMAL_PASTE  0
#define XmuttFG_INSERT_PASTE  1
#define XmuttFG_MERGE_PASTE   2
#define XmuttFG_OVERLAY_PASTE 2

extern void XmuttFontGridUpdateMetrics __((Widget w, bdf_metrics_t *metrics));

extern void XmuttFontGridUpdateFont __((Widget w, bdf_glyph_t *glyph,
                                        Boolean unencoded));

extern void XmuttFontGridPageInfo
    __((Widget w, XmuttFontGridPageInfoStruct *pageinfo));

extern void XmuttFontGridSelectionInfo
    __((Widget w, XmuttFontGridSelectionStruct *selinfo));

extern void XmuttFontGridViewUnencoded __((Widget w, Boolean unencoded));
extern int XmuttFontGridCodeBase __((Widget w));
extern void XmuttFontGridChangeCodeBase __((Widget w, int base));
extern void XmuttFontGridFirstPage __((Widget w));
extern void XmuttFontGridGotoPage __((Widget w, long pageno));
extern void XmuttFontGridLastPage __((Widget w));
extern void XmuttFontGridNextPage __((Widget w));
extern void XmuttFontGridPreviousPage __((Widget w));
extern void XmuttFontGridGotoCodePage __((Widget w, long code));

extern Boolean XmuttFontGridModified __((Widget w));
extern void XmuttFontGridSetModified __((Widget w, Boolean modified));
extern void XmuttFontGridCallModifiedCallback __((Widget w));

extern Boolean XmuttFontGridHasXLFDName __((Widget w));
extern void XmuttFontGridMakeXLFDName __((Widget w));
extern void XmuttFontGridUpdateNameFromProperties __((Widget w));
extern void XmuttFontGridUpdatePropertiesFromName __((Widget w));
extern void XmuttFontGridUpdateAverageWidth __((Widget w));
extern void XmuttFontGridSetFontName __((Widget w, char *name));

/*
 * Font info functions.
 */
extern bdf_font_t *XmuttFontGridFont __((Widget w));

extern char *XmuttFontGridFontName __((Widget w));
extern char *XmuttFontGridFontComments __((Widget w));
extern char *XmuttFontGridFontMessages __((Widget w));
extern int XmuttFontGridFontSpacing __((Widget w));
extern unsigned short XmuttFontGridFontDeviceWidth __((Widget w));
extern void XmuttFontGridFontResolution __((Widget w, long *resolution_x,
                                            long *resolution_y));

extern void XmuttFontGridGetFontInfo __((Widget w,
                                         XmuttFontGridFontInfoStruct *info));

extern void XmuttFontGridSetFontInfo __((Widget w,
                                         XmuttFontGridFontInfoStruct *info));

extern void XmuttFontGridUpdateComments __((Widget w, char *comments,
                                            unsigned long comments_len));

extern void XmuttFontGridTranslateGlyphs __((Widget w,
                                             Position dx,
                                             Position dy,
                                             bdf_callback_t callback,
                                             void *data,
                                             Boolean all_glyphs));

extern void XmuttFontGridRotateGlyphs __((Widget w,
                                          short degrees,
                                          bdf_callback_t callback,
                                          void *data,
                                          Boolean all_glyphs));

extern void XmuttFontGridShearGlyphs __((Widget w,
                                         short degrees,
                                         bdf_callback_t callback,
                                         void *data,
                                         Boolean all_glyphs));

extern void XmuttFontGridEmboldenGlyphs __((Widget w,
                                            bdf_callback_t callback,
                                            void *data,
                                            Boolean all_glyphs));

extern Boolean XmuttFontGridHasSelection __((Widget w));
extern Boolean XmuttFontGridClipboardEmpty __((Widget w));

extern void XmuttFontGridCopySelection __((Widget w));
extern void XmuttFontGridCutSelection __((Widget w));
extern void XmuttFontGridPasteSelection __((Widget w, int type));

#undef __

#ifdef __cplusplus
}
#endif

#endif /* _h_XmuttFontGrid */

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
#ifndef lint
#ifdef __GNUC__
static char rcsid[] __attribute__ ((unused)) = "$Id: GEdit.c,v 1.13 2001/09/19 21:00:41 mleisher Exp $";
#else
static char rcsid[] = "$Id: GEdit.c,v 1.13 2001/09/19 21:00:41 mleisher Exp $";
#endif
#endif

#include <stdlib.h>
#include "GEditP.h"
#include <X11/Xmu/Drawing.h>

static unsigned char bitswap[] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0,
    0x30, 0xb0, 0x70, 0xf0, 0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8, 0x04, 0x84, 0x44, 0xc4,
    0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc,
    0x3c, 0xbc, 0x7c, 0xfc, 0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 0x0a, 0x8a, 0x4a, 0xca,
    0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6,
    0x36, 0xb6, 0x76, 0xf6, 0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 0x01, 0x81, 0x41, 0xc1,
    0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9,
    0x39, 0xb9, 0x79, 0xf9, 0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 0x0d, 0x8d, 0x4d, 0xcd,
    0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3,
    0x33, 0xb3, 0x73, 0xf3, 0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 0x07, 0x87, 0x47, 0xc7,
    0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf,
    0x3f, 0xbf, 0x7f, 0xff,
};

#define cursor_width 13
#define cursor_height 13
static unsigned char cursor_bits[] = {
   0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00,
   0x1f, 0x1f, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00,
   0x40, 0x00};

/**************************************************************************
 *
 * Utility functions.
 *
 **************************************************************************/

static void
#ifndef _NO_PROTO
_XmuttGlyphEditCheckClass(Widget w)
#else
_XmuttGlyphEditCheckClass(w)
Widget w;
#endif
{
    if (XtIsSubclass(w, xmuttGlyphEditWidgetClass) == False) {
        fprintf(stderr,
                "XmuttGlyphEdit: %s not an XmuttGlyphEditWidget.\n",
                XtName(w));
        exit(1);
    }
}

static Dimension
#ifndef _NO_PROTO
_XmuttGlyphEditPMargins(XmuttGlyphEditWidget gw)
#else
_XmuttGlyphEditPMargins(gw)
XmuttGlyphEditWidget gw;
#endif
{
    return (gw->primitive.shadow_thickness << 1) +
        (gw->primitive.highlight_thickness << 1);
}

/*
 * Determine the default cursor color.
 */
static void
#ifndef _NO_PROTO
_XmuttGlyphEditDefaultColor(Widget w, int offset, XrmValue *val)
#else
_XmuttGlyphEditDefaultColor(w, offset, val)
Widget w;
int offset;
XrmValue *val;
#endif
{
    XmuttGlyphEditWidget gw;

    gw = (XmuttGlyphEditWidget) w;

    val->size = sizeof(Pixel);
    val->addr = (XPointer) &gw->primitive.foreground;
}

/*
 * Change window highlighting when focus is gained or lost.
 */
static void
#ifndef _NO_PROTO
_XmuttGlyphEditHighlightWindow(Widget w)
#else
_XmuttGlyphEditHighlightWindow(w)
Widget w;
#endif
{
    int i;
    XmuttGlyphEditWidget gw;
    XRectangle hi[4];

    gw = (XmuttGlyphEditWidget) w;

    if (!XtIsRealized(w) || gw->primitive.highlight_thickness == 0)
      return;

    /*
     * Top of window.
     */
    hi[0].x = hi[0].y = 0;
    hi[0].width = gw->core.width;
    hi[0].height = gw->primitive.highlight_thickness;

    /*
     * Right side.
     */
    hi[1].x = gw->core.width - gw->primitive.highlight_thickness;
    hi[1].y = hi[0].y + gw->primitive.highlight_thickness;
    hi[1].width = gw->primitive.highlight_thickness;
    hi[1].height = gw->core.height - (gw->primitive.highlight_thickness << 1);

    /*
     * Bottom of window.
     */
    hi[2].x = hi[0].x;
    hi[2].y = gw->core.height - gw->primitive.highlight_thickness;
    hi[2].width = hi[0].width;
    hi[2].height = hi[0].height;

    /*
     * Left side.
     */
    hi[3].x = hi[0].x;
    hi[3].y = hi[1].y;
    hi[3].width = hi[1].width;
    hi[3].height = hi[1].height;

    if (gw->gedit.hasfocus == True)
      XFillRectangles(XtDisplay(w), XtWindow(w), gw->primitive.highlight_GC,
                      hi, 4);
    else {
        for (i = 0; i < 4; i++)
          XClearArea(XtDisplay(w), XtWindow(w), hi[i].x, hi[i].y,
                     hi[i].width, hi[i].height, False);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditShadowWindow(Widget w)
#else
_XmuttGlyphEditShadowWindow(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    int i;
    XPoint br[3], tl[3];

    gw = (XmuttGlyphEditWidget) w;

    if (!XtIsRealized(w) || gw->primitive.shadow_thickness == 0)
      return;

    /*
     * Lower left corner outside line.
     */
    tl[0].x = gw->primitive.highlight_thickness;
    tl[0].y = gw->core.height - gw->primitive.highlight_thickness - 1;

    /*
     * Upper left corner outside line.
     */
    tl[1].x = tl[0].x;
    tl[1].y = gw->primitive.highlight_thickness;

    /*
     * Upper right corner outside line.
     */
    tl[2].x = gw->core.width - gw->primitive.highlight_thickness - 1;
    tl[2].y = tl[1].y;

    /*
     * Upper right corner outside line.
     */
    br[0].x = tl[2].x;
    br[0].y = tl[2].y + 1;

    /*
     * Lower right corner outside line.
     */
    br[1].x = br[0].x;
    br[1].y = gw->core.height - gw->primitive.highlight_thickness - 1;

    /*
     * Lower left corner outside line.
     */
    br[2].x = gw->primitive.highlight_thickness + 1;
    br[2].y = br[1].y;

    XDrawLines(XtDisplay(w), XtWindow(w), gw->primitive.top_shadow_GC,
               tl, 3, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), gw->primitive.bottom_shadow_GC,
               br, 3, CoordModeOrigin);

    /*
     * Draw the remaining shadows successively inward.
     */
    for (i = 1; i < gw->primitive.shadow_thickness; i++) {
        tl[0].x++;
        tl[0].y--;
        tl[1].x++;
        tl[1].y++;
        tl[2].x--;
        tl[2].y++;

        br[0].x--;
        br[0].y++;
        br[1].x--;
        br[1].y--;
        br[2].x++;
        br[2].y--;
        XDrawLines(XtDisplay(w), XtWindow(w), gw->primitive.top_shadow_GC,
                   tl, 3, CoordModeOrigin);
        XDrawLines(XtDisplay(w), XtWindow(w), gw->primitive.bottom_shadow_GC,
                   br, 3, CoordModeOrigin);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditMakeGCs(Widget w, Boolean force)
#else
_XmuttGlyphEditMakeGCs(w, force)
Widget w;
Boolean force;
#endif
{
    XmuttGlyphEditWidget gw;
    XtGCMask gcm;
    XGCValues gcv;

    gw = (XmuttGlyphEditWidget) w;

    gcm = GCForeground|GCBackground|GCFunction;

    if (gw->gedit.gridgc == 0 || force == True) {
        if (gw->gedit.gridgc != 0)
          XFreeGC(XtDisplay(w), gw->gedit.gridgc);
        gcv.foreground = gw->primitive.foreground;
        gcv.background = gw->core.background_pixel;
        gcv.function = GXcopy;
        gcv.dashes = 1;
        gcv.line_style = LineOnOffDash;
        gw->gedit.gridgc = XCreateGC(XtDisplay(w), XtWindow(w),
                                     gcm | GCDashList | GCLineStyle, &gcv);
    }

    if (gw->gedit.bbxgc == 0 || force == True) {
        if (gw->gedit.bbxgc != 0)
          XFreeGC(XtDisplay(w), gw->gedit.bbxgc);
        gcm = GCForeground|GCFunction;
        gcv.foreground = gw->gedit.bbxpix;
        gcv.function = GXcopy;
        gw->gedit.bbxgc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }

    if (gw->gedit.pixgc == 0 || force == True) {
        if (gw->gedit.pixgc != 0)
          XFreeGC(XtDisplay(w), gw->gedit.pixgc);
        gcm = GCForeground|GCBackground|GCFunction;
        gcv.foreground = gw->primitive.foreground;
        gcv.background = gw->core.background_pixel;
        gcv.function = GXcopy;
        gw->gedit.pixgc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }

    if (gw->gedit.cleargc == 0 || force == True) {
        if (gw->gedit.cleargc != 0)
          XFreeGC(XtDisplay(w), gw->gedit.cleargc);
        gcm = GCForeground|GCFunction;
        gcv.foreground = gw->core.background_pixel;
        gcv.function = GXcopy;
        gw->gedit.cleargc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }

    if (gw->gedit.selgc == 0 || force == True) {
        if (gw->gedit.selgc != 0)
          XFreeGC(XtDisplay(w), gw->gedit.selgc);
        gcm = GCForeground|GCBackground|GCFunction;
        gcv.foreground = gw->gedit.selpix;
        gcv.background = gw->core.background_pixel;
        if (gw->gedit.selpix != gw->primitive.foreground)
          gcv.function = GXcopy;
        else {
            gcv.foreground ^= gcv.background;
            gcv.function = GXxor;
        }
        gw->gedit.selgc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditPreferredGeometry(XmuttGlyphEditWidget gw,
                                 Dimension *width, Dimension *height)
#else
_XmuttGlyphEditPreferredGeometry(gw, width, height)
XmuttGlyphEditWidget gw;
Dimension *width, *height;
#endif
{
    Dimension margin, dht;
    bdf_glyph_grid_t *grid;

    dht = DisplayHeight(XtDisplay((Widget) gw),
                        XScreenNumberOfScreen(XtScreen((Widget) gw)));

    grid = gw->gedit.grid;
    margin = _XmuttGlyphEditPMargins(gw) + 4;

    /*
     * Recalculate a nice pixel size based on the size of the display.
     */
    gw->gedit.pixel_size = gw->gedit.default_pixel_size;

    *height = margin + ((gw->gedit.pixel_size + 4) * grid->grid_height);
    while (gw->gedit.pixel_size >= 2) {
        *height = margin + ((gw->gedit.pixel_size + 4) * grid->grid_height);
        if (*height < (dht >> 1))
          break;
        gw->gedit.pixel_size--;
    }
    if (gw->gedit.pixel_size < 2)
      gw->gedit.pixel_size = 2;

    *width = margin + ((gw->gedit.pixel_size + 4) * grid->grid_width);
}

#define GEDIT_SET    1
#define GEDIT_INVERT 2
#define GEDIT_CLEAR  3

static void
#ifndef _NO_PROTO
_XmuttGlyphEditDrawPixel(XmuttGlyphEditWidget gw, int x, int y, Boolean select)
#else
_XmuttGlyphEditDrawPixel(gw, x, y, select)
XmuttGlyphEditWidget gw;
int x, y;
Boolean select;
#endif
{
    Widget w;
    int bpr, set;
    short dx, dy, di, si;
    Dimension adj;
    bdf_glyph_grid_t *grid;
    unsigned char *masks;
    XRectangle pix;

    w = (Widget) gw;

    if (!XtIsRealized(w))
      return;

    grid = gw->gedit.grid;

    di = 0;
    masks = 0;
    switch (grid->bpp) {
      case 1: masks = onebpp; di = 7; break;
      case 2: masks = twobpp; di = 3; break;
      case 4: masks = fourbpp; di = 1; break;
    }

    adj = (_XmuttGlyphEditPMargins(gw) >> 1) + 2;
    bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;

    pix.width = pix.height = gw->gedit.pixel_size + 1;
    pix.x = adj + ((gw->gedit.pixel_size + 4) * x) + 2;
    pix.y = adj + ((gw->gedit.pixel_size + 4) * y) + 2;

    if (select == True && grid->sel.width != 0) {
        bpr = ((grid->sel.width * grid->bpp) + 7) >> 3;
        dy = y - grid->sel.y;
        dx = (x - grid->sel.x) * grid->bpp;
        si = (dx & 7) / grid->bpp;
        set = grid->sel.bitmap[(dy * bpr) + (dx >> 3)] & masks[si];
        if (di > si)
          set >>= (di - si) * grid->bpp;
    } else {
        dy = y;
        dx = x * grid->bpp;
        si = (dx & 7) / grid->bpp;
        set = grid->bitmap[(dy * bpr) + (dx >> 3)] & masks[si];
        if (di > si)
          set >>= (di - si) * grid->bpp;
    }

    if (set) {
        if (grid->bpp > 1) {
            if (grid->bpp == 2)
              XSetForeground(XtDisplay(w), gw->gedit.pixgc,
                             gw->gedit.colors[set]);
            else
              XSetForeground(XtDisplay(w), gw->gedit.pixgc,
                             gw->gedit.colors[set + 4]);
        }
        XFillRectangle(XtDisplay(w), XtWindow(w), gw->gedit.pixgc,
                       pix.x, pix.y, pix.width, pix.height);
    } else
      XClearArea(XtDisplay(w), XtWindow(w), pix.x, pix.y,
                 pix.width, pix.height, False);

    if (select == True)
      XFillRectangle(XtDisplay(w), XtWindow(w), gw->gedit.selgc,
                     pix.x + 1, pix.y + 1, pix.width - 2, pix.height - 2);
}

/*
 * Determine if the point is in the old selection.
 */
static Boolean
#ifndef _NO_PROTO
_XmuttGlyphEditInSelection(XmuttGlyphEditWidget gw, Position x, Position y)
#else
_XmuttGlyphEditInSelection(gw, x, y)
XmuttGlyphEditWidget gw;
Position x, y;
#endif
{
    return (((gw->gedit.sel_start.y <= y && y <= gw->gedit.sel_end.y) ||
             (gw->gedit.sel_end.y <= y && y <= gw->gedit.sel_start.y)) &&
            ((gw->gedit.sel_start.x <= x && x <= gw->gedit.sel_end.x) ||
             (gw->gedit.sel_end.x <= x && x <= gw->gedit.sel_start.x)))
        ? True : False;
}

/*
 * Check to see if a point is in the intersection of the old and new
 * selections.
 */
static Boolean
#ifndef _NO_PROTO
_XmuttGlyphEditInIntersection(XmuttGlyphEditWidget gw, Position ix,
                              Position iy, Position x, Position y)
#else
_XmuttGlyphEditInIntersection(gw, ix, iy, x, y)
XmuttGlyphEditWidget gw;
Position ix, iy, x, y;
#endif
{
    return (((gw->gedit.sel_start.y <= y && y <= iy) ||
             (iy <= y && y <= gw->gedit.sel_start.y)) &&
            ((gw->gedit.sel_start.x <= x && x <= ix) ||
             (ix <= x && x <= gw->gedit.sel_start.x))) ? True : False;
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditUpdateSelection(XmuttGlyphEditWidget gw, Position x, Position y,
                               Boolean set)
#else
_XmuttGlyphEditUpdateSelection(gw, x, y, set)
XmuttGlyphEditWidget gw;
Position x, y;
Boolean set;
#endif
{
    Position ht, wd;
    bdf_glyph_grid_t *grid;

    grid = gw->gedit.grid;
    for (ht = 0; ht < grid->grid_height; ht++) {
        for (wd = 0; wd < grid->grid_width; wd++) {
            if (_XmuttGlyphEditInIntersection(gw, x, y, wd, ht) == False &&
                _XmuttGlyphEditInSelection(gw, wd, ht) == True)
              /*
               * Clear or set the pixel.
               */
              _XmuttGlyphEditDrawPixel(gw, wd, ht, set);
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditDrawGlyph(Widget w)
#else
_XmuttGlyphEditDrawGlyph(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    short x, y;
    bdf_glyph_grid_t *grid;
    Boolean sel;

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    for (y = 0; y < grid->grid_height; y++) {
        for (x = 0; x < grid->grid_width; x++) {
            sel = (bdf_in_selection(grid, x, y, 0)) ? True : False;
            _XmuttGlyphEditDrawPixel(gw, x, y, sel);
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditDrawFontBBX(Widget w, Boolean clear)
#else
_XmuttGlyphEditDrawFontBBX(w, clear)
Widget w;
Boolean clear;
#endif
{
    XmuttGlyphEditWidget gw;
    GC gc;
    Dimension xoff, yoff, fxoff, fyoff, psize;
    bdf_glyph_grid_t *grid;
    XRectangle frame;
    
    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    gc = (clear == False) ? gw->gedit.bbxgc : gw->gedit.cleargc;

    /*
     * Draw the font bounding box frame.
     */
    frame.x = frame.y = (_XmuttGlyphEditPMargins(gw) >> 1) + 2;

    psize = gw->gedit.pixel_size + 4;

    if (grid->font_bbx.x_offset < 0)
      fxoff = psize * (grid->base_x + grid->font_bbx.x_offset);
    else
      fxoff = psize * grid->base_x;

    fyoff = psize * (grid->base_y - grid->font_bbx.ascent);

    frame.width = psize * grid->font_bbx.width;
    frame.height = psize * (grid->font_bbx.ascent + grid->font_bbx.descent);

    if (clear == False)
      XDrawRectangle(XtDisplay(w), XtWindow(w), gc, frame.x + fxoff,
                     frame.y + fyoff, frame.width, frame.height);
    else
      XDrawRectangle(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                     frame.x + fxoff, frame.y + fyoff,
                     frame.width, frame.height);

    /*
     * Draw vertical baseline.
     */
    xoff = (gw->gedit.pixel_size + 4) * grid->base_x;
    yoff = (gw->gedit.pixel_size + 4) * grid->base_y;

    if (clear == False)
      XDrawLine(XtDisplay(w), XtWindow(w), gc,
                frame.x + xoff, frame.y + fyoff,
                frame.x + xoff, frame.y + fyoff + frame.height);
    else
      XDrawLine(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                frame.x + xoff, frame.y + fyoff,
                frame.x + xoff, frame.y + fyoff + frame.height);

    /*
     * Draw horizontal baseline.
     */
    if (clear == False)
      XDrawLine(XtDisplay(w), XtWindow(w), gc,
                frame.x + fxoff, frame.y + yoff,
                frame.x + fxoff + frame.width, frame.y + yoff);
    else
      XDrawLine(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                frame.x + fxoff, frame.y + yoff,
                frame.x + fxoff + frame.width, frame.y + yoff);

    /*
     * Draw the CAP_HEIGHT if indicated and exists.
     */
    if (gw->gedit.show_cap_height == True && grid->cap_height != 0) {
        yoff = (gw->gedit.pixel_size + 4) * (grid->base_y - grid->cap_height);
        if (clear == False)
          XDrawLine(XtDisplay(w), XtWindow(w), gc,
                    frame.x + fxoff, frame.y + yoff,
                    frame.x + fxoff + frame.width, frame.y + yoff);
        else
          XDrawLine(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                    frame.x + fxoff, frame.y + yoff,
                    frame.x + fxoff + frame.width, frame.y + yoff);
    }

    /*
     * Draw the X_HEIGHT if indicated and exists.
     */
    if (gw->gedit.show_x_height == True && grid->x_height != 0) {
        yoff = (gw->gedit.pixel_size + 4) * (grid->base_y - grid->x_height);
        if (clear == False)
          XDrawLine(XtDisplay(w), XtWindow(w), gc,
                    frame.x + fxoff, frame.y + yoff,
                    frame.x + fxoff + frame.width, frame.y + yoff);
        else
          XDrawLine(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                    frame.x + fxoff, frame.y + yoff,
                    frame.x + fxoff + frame.width, frame.y + yoff);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditDrawGrid(Widget w)
#else
_XmuttGlyphEditDrawGrid(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    Position x, y;
    short limit, unit, wd, ht;
    XRectangle frame;

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    wd = grid->grid_width;
    ht = grid->grid_height;

    frame.x = frame.y = (_XmuttGlyphEditPMargins(gw) >> 1) + 2;
    frame.width = (gw->gedit.pixel_size + 4) * wd;
    frame.height = (gw->gedit.pixel_size + 4) * ht;

    XDrawRectangle(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                   frame.x, frame.y, frame.width, frame.height);

    /*
     * Draw the vertical grid lines.
     */
    limit = frame.x + frame.width;
    unit = gw->gedit.pixel_size + 4;
    for (x = frame.x + unit, y = frame.y; x < limit; x += unit)
      XDrawLine(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                x, y, x, y + frame.height);

    /*
     * Draw the horizontal grid lines.
     */
    limit = frame.y + frame.height;
    for (x = frame.x, y = frame.y + unit; y < limit; y += unit)
      XDrawLine(XtDisplay(w), XtWindow(w), gw->gedit.gridgc,
                x, y, x + frame.width, y);

    /*
     * Draw the font bounding box.
     */
    _XmuttGlyphEditDrawFontBBX(w, False);
    _XmuttGlyphEditDrawGlyph(w);
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditXYToCoords(XmuttGlyphEditWidget gw, XEvent *event,
                          Position *x, Position *y)
#else
_XmuttGlyphEditXYToCoords(gw, event, x, y)
XmuttGlyphEditWidget gw;
XEvent *event;
Position *x, *y;
#endif
{
    Dimension adj;
    int ex, ey;

    ex = ey = 0;
    switch (event->type) {
      case KeyPress:
      case KeyRelease:
        ex = event->xkey.x;
        ey = event->xkey.y;
        break;
      case ButtonPress:
      case ButtonRelease:
        ex = event->xbutton.x;
        ey = event->xbutton.y;
        break;
      case MotionNotify:
        ex = event->xmotion.x;
        ey = event->xmotion.y;
        break;
    }

    adj = (_XmuttGlyphEditPMargins(gw) >> 1) + 2;

    if (ex < adj ||
        ex > adj + ((gw->gedit.pixel_size + 4) * gw->gedit.grid->grid_width))
      *x = -1;
    else
      *x = (ex - adj) / (gw->gedit.pixel_size + 4);

    if (ey < adj ||
        ey > adj + ((gw->gedit.pixel_size + 4) * gw->gedit.grid->grid_height))
      *y = -1;
    else
      *y = (ey - adj) / (gw->gedit.pixel_size + 4);
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditSetCursor(Widget w)
#else
_XmuttGlyphEditSetCursor(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    XmuttGlyphEditWidgetClass gwc;
    Screen *screen;
    Pixmap bm;
    XColor color;

    gw = (XmuttGlyphEditWidget) w;
    gwc = (XmuttGlyphEditWidgetClass) XtClass(w);

    if (gwc->gedit_class.refcnt == 1) {
        color.pixel = gw->gedit.cursorpix;
        screen = XtScreen(w);
        XQueryColor(XtDisplay(w), XDefaultColormapOfScreen(screen), &color);
        bm = XCreateBitmapFromData(XtDisplay(w), XtWindow(w), cursor_bits,
                                   cursor_width, cursor_height);
        gwc->gedit_class.cursor =
            XCreatePixmapCursor(XtDisplay(w), bm, bm, &color, &color,
                                (cursor_width >> 1), (cursor_height >> 1));
        XFreePixmap(XtDisplay(w), bm);
    }
    XDefineCursor(XtDisplay(w), XtWindow(w), gwc->gedit_class.cursor);
}

/*
 * Encode just the selection bitmap.
 */
static unsigned char *
#ifndef _NO_PROTO
_XmuttGlyphEditEncodeSelection(XmuttGlyphEditWidget gw, unsigned long *bytes)
#else
_XmuttGlyphEditEncodeSelection(gw, bytes)
XmuttGlyphEditWidget gw;
unsigned long *bytes;
#endif
{
    unsigned long bcount, size;
    unsigned short wd, ht;
    unsigned char *bmap, *bp;
    bdf_glyph_grid_t *grid;

    grid = gw->gedit.grid;

    *bytes = 0;
    if (!bdf_has_selection(grid, 0, 0, (short *) &wd, (short *) &ht))
      return 0;

    /*
     * Get the bytes used for the selection.
     */
    size = bcount = grid->sel.bytes >> 1;
    size += sizeof(unsigned short) * 3;
    bp = bmap = (unsigned char *) XtMalloc(size);

    /*
     * Encode the width and height in Most Significant Byte order assuming
     * the width and height types are 16-bit values.
     */
    if (!bdf_little_endian()) {
        *bp++ = (grid->bpp >> 8) & 0xff;
        *bp++ = grid->bpp & 0xff;
        *bp++ = (wd >> 8) & 0xff;
        *bp++ = wd & 0xff;
        *bp++ = (ht >> 8) & 0xff;
        *bp++ = ht & 0xff;
    } else {
        *bp++ = grid->bpp & 0xff;
        *bp++ = (grid->bpp >> 8) & 0xff;
        *bp++ = wd & 0xff;
        *bp++ = (wd >> 8) & 0xff;
        *bp++ = ht & 0xff;
        *bp++ = (ht >> 8) & 0xff;
    }

    (void) memcpy((char *) bp, (char *) grid->sel.bitmap, bcount);

    *bytes = size;
    return bmap;
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditOwnClipboard(Widget w)
#else
_XmuttGlyphEditOwnClipboard(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    Display *d;
    Window win;
    XEvent ev;

    gw = (XmuttGlyphEditWidget) w;

    if (!XtIsRealized(w) || gw->gedit.owns_clipboard == True)
      return;

    d = XtDisplay(w);

    /*
     * Get the current owner of the clipboard.
     */
    win = XGetSelectionOwner(d, XmuttGLYPHEDIT_CLIPBOARD(d));

    XSetSelectionOwner(d, XmuttGLYPHEDIT_CLIPBOARD(d),
                       XtWindow(w), XtLastTimestampProcessed(d));

    if (XGetSelectionOwner(d, XmuttGLYPHEDIT_CLIPBOARD(d)) == XtWindow(w))
      gw->gedit.owns_clipboard = True;
    else
      gw->gedit.owns_clipboard = False;

    /*
     * Finally, because the Intrinsics don't seem to be doing it, we have to
     * send a SelectionClear notice to the previous owner of the clipboard so
     * it knows it lost it.
     */
    if (win != None) {
        ev.xselectionclear.type = SelectionClear;
        ev.xselectionclear.serial = LastKnownRequestProcessed(d);
        ev.xselectionclear.send_event = True;
        ev.xselectionclear.display = d;
        ev.xselectionclear.window = win;
        ev.xselectionclear.selection = XmuttGLYPHEDIT_CLIPBOARD(d);
        ev.xselectionclear.time = XtLastTimestampProcessed(d);
        XSendEvent(d, win, False, 0, &ev);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphEditAttemptResize(XmuttGlyphEditWidget gw, Dimension width,
                             Dimension height)
#else
_XmuttGlyphEditAttemptResize(gw, width, height)
XmuttGlyphEditWidget gw;
Dimension width, height;
#endif
{
    Dimension rwidth, rheight, owidth, oheight;
    XtGeometryResult res;

    owidth = gw->core.width;
    oheight = gw->core.height;

    res = XtMakeResizeRequest((Widget) gw, width, height, &rwidth, &rheight);

    if (res == XtGeometryAlmost) {
        res = XtMakeResizeRequest((Widget) gw, rwidth, rheight,
                                  &rwidth, &rheight);
        if (rwidth == owidth && rheight == oheight)
          res = XtGeometryNo;
    } else if (gw->core.width != width && gw->core.height != height)
      res = XtGeometryNo;

    if (res == XtGeometryYes)
      (*gw->core.widget_class->core_class.resize)((Widget) gw);
}

/**************************************************************************
 *
 * Class methods.
 *
 **************************************************************************/

static void
#ifndef _NO_PROTO
ClassInitialize(void)
#else
ClassInitialize()
#endif
{
}

static void
#ifndef _NO_PROTO
ClassPartInitialize (WidgetClass wc)
#else
ClassPartInitialize (wc)
WidgetClass wc;
#endif
{
}

static void
#ifndef _NO_PROTO
Initialize(Widget req, Widget newone, ArgList args, Cardinal *num_args)
#else
Initialize(req, newone, args, num_args)
Widget req, newone;
ArgList args;
Cardinal *num_args;
#endif
{
    XmuttGlyphEditWidget gw;
    XmuttGlyphEditWidgetClass gwc;
    Dimension wd, ht;

    gw = (XmuttGlyphEditWidget) newone;

    /*
     * Increment the reference count for the class.
     */
    gwc = (XmuttGlyphEditWidgetClass) XtClass(newone);
    gwc->gedit_class.refcnt++;

    /*
     * Initialize the GC's.
     */
    gw->gedit.gridgc = gw->gedit.bbxgc =
        gw->gedit.pixgc = gw->gedit.cleargc = gw->gedit.selgc = 0;

    /*
     * Set the number of pixels used for the enlarged pixels.
     */
    gw->gedit.pixel_size = 10;

    /*
     * Initialize the editor operation and the pending operation.
     */
    gw->gedit.op = XmuttGLYPHEDIT_DRAW;
    gw->gedit.pending_op = -1;

    /*
     * Initialize the booleans.
     */
    gw->gedit.owns_clipboard = False;
    gw->gedit.hasfocus = False;

    /*
     * Initialize the last x and y position of the pointer to the baseline
     * so the reported coordinates will be Cartesian.
     */
    gw->gedit.last_x = gw->gedit.grid->base_x;
    gw->gedit.last_y = gw->gedit.grid->base_y;

    /*
     * Initialize the selection coordinates.
     */
    (void) memset((char *) &gw->gedit.sel_start, 0, sizeof(XPoint));
    (void) memset((char *) &gw->gedit.sel_end, 0, sizeof(XPoint));

    /*
     * Initialize the current color index.
     */
    if (gw->gedit.grid) {
        if ((gw->gedit.cidx = (1 << gw->gedit.grid->bpp) - 1) == 0)
          gw->gedit.cidx = 1;
    } else
      gw->gedit.cidx = -1;

    /*
     * Initialize the last color to something different than the current color
     * index so the first color will be set in the GC when the pixels are
     * drawn.
     */
    gw->gedit.lcolor = -2;

    /*
     * Set the initial width and height.
     */
    _XmuttGlyphEditPreferredGeometry(gw, &wd, &ht);

    gw->core.width = wd;
    gw->core.height = ht;
}

static void
#ifndef _NO_PROTO
Resize(Widget w)
#else
Resize(w)
Widget w;
#endif
{
}

static void
#ifndef _NO_PROTO
Realize(Widget w, Mask *valueMask, XSetWindowAttributes *attributes)
#else
Realize(w, valueMask, attributes)
Widget w;
Mask *valueMask;
XSetWindowAttributes *attributes;
#endif
{
    XtCreateWindow(w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
                   *valueMask, attributes);

    _XmuttGlyphEditSetCursor(w);
    _XmuttGlyphEditMakeGCs(w, False);
}

static void
#ifndef _NO_PROTO
Destroy(Widget w)
#else
Destroy(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    XmuttGlyphEditWidgetClass gwc;

    gw = (XmuttGlyphEditWidget) w;
    gwc = (XmuttGlyphEditWidgetClass) XtClass(w);

    /*
     * Free up the cursor if this is the last instance of the widget
     * being destroyed.
     */
    if (gwc->gedit_class.refcnt > 0) {
        gwc->gedit_class.refcnt--;
        if (gwc->gedit_class.refcnt == 0) {
            XFreeCursor(XtDisplay(w), gwc->gedit_class.cursor);
            gwc->gedit_class.cursor = 0;
        }
    }

    /*
     * Free up the bounding box and glyph info.
     */
    if (gw->gedit.grid != 0) {
        bdf_free_glyph_grid(gw->gedit.grid);
        gw->gedit.grid = 0;
    }

    /*
     * Free up the GC's.
     */
    if (gw->gedit.gridgc != 0)
      XFreeGC(XtDisplay(w), gw->gedit.gridgc);
    if (gw->gedit.bbxgc != 0)
      XFreeGC(XtDisplay(w), gw->gedit.bbxgc);
    if (gw->gedit.pixgc != 0)
      XFreeGC(XtDisplay(w), gw->gedit.pixgc);
    if (gw->gedit.cleargc != 0)
      XFreeGC(XtDisplay(w), gw->gedit.cleargc);
    if (gw->gedit.selgc != 0)
      XFreeGC(XtDisplay(w), gw->gedit.selgc);

    gw->gedit.gridgc = gw->gedit.bbxgc =
        gw->gedit.pixgc = gw->gedit.cleargc = gw->gedit.selgc = 0;
}

static void
#ifndef _NO_PROTO
Redisplay(Widget w, XEvent *event, Region region)
#else
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
#endif
{
    _XmuttGlyphEditHighlightWindow(w);
    _XmuttGlyphEditShadowWindow(w);
    _XmuttGlyphEditDrawGrid(w);
}

static Boolean
#ifndef _NO_PROTO
SetValues(Widget curr, Widget req, Widget newone,
          ArgList args, Cardinal *num_args)
#else
SetValues(curr, req, newone, args, num_args)
Widget curr, req, newone;
ArgList args;
Cardinal *num_args;
#endif
{
    XmuttGlyphEditWidget cgw, ngw;
    Dimension width, height;
    Boolean redo;

    redo = False;

    cgw = (XmuttGlyphEditWidget) curr;
    ngw = (XmuttGlyphEditWidget) newone;

    if (ngw->gedit.grid != cgw->gedit.grid) {
        redo = True;

        /*
         * If the widget is in Move or Copy mode, change it back
         * to Select mode.
         */
        if (ngw->gedit.op == XmuttGLYPHEDIT_MOVE ||
            ngw->gedit.op == XmuttGLYPHEDIT_COPY) {
            ngw->gedit.pending_op = ngw->gedit.op;
            ngw->gedit.op = _XmuttGLYPHEDIT_SELECT;
        }

        _XmuttGlyphEditPreferredGeometry(ngw, &width, &height);
        if (width != cgw->core.width || height != cgw->core.height) {
            ngw->core.width = width;
            ngw->core.height = height;
        }
        bdf_free_glyph_grid(cgw->gedit.grid);

        /*
         * Adjust the current color index value if necessary.
         */
        if (ngw->gedit.cidx == -1) {
            if ((ngw->gedit.cidx = (1 << ngw->gedit.grid->bpp) - 1) == 0)
              ngw->gedit.cidx = 1;
        } else if (ngw->gedit.grid->bpp == 1)
          ngw->gedit.cidx = 1;
        else if (ngw->gedit.cidx >= (1 << ngw->gedit.grid->bpp))
          ngw->gedit.cidx = (1 << ngw->gedit.grid->bpp) - 1;

        /*
         * Set the last color to something different than the current color so
         * the GC will be changed.
         */
        ngw->gedit.lcolor = -2;
    }

    if (ngw->gedit.default_pixel_size != cgw->gedit.default_pixel_size) {
        _XmuttGlyphEditPreferredGeometry(ngw, &width, &height);
        if (width != cgw->core.width || height != cgw->core.height) {
            ngw->core.width = width;
            ngw->core.height = height;
        }
        redo = True;
    }

    if (ngw->gedit.show_x_height != cgw->gedit.show_x_height ||
        ngw->gedit.show_cap_height != cgw->gedit.show_cap_height)
      redo = True;

    return redo;
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

long
#ifndef _NO_PROTO
XmuttGlyphEditEncoding(Widget w)
#else
XmuttGlyphEditEncoding(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;

    return (gw->gedit.grid->unencoded) ? -1 : gw->gedit.grid->encoding;
}

void
#ifndef _NO_PROTO
XmuttGlyphEditMetrics(Widget w, bdf_metrics_t *metrics, Boolean font)
#else
XmuttGlyphEditMetrics(w, metrics, font)
Widget w;
bdf_metrics_t *metrics;
Boolean font;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    if (metrics == 0)
      return;

    gw = (XmuttGlyphEditWidget) w;

    metrics->font_spacing = gw->gedit.grid->spacing;
    metrics->dwidth = gw->gedit.grid->dwidth;

    if (font == True) {
        metrics->width = gw->gedit.grid->font_bbx.width;
        metrics->height = gw->gedit.grid->font_bbx.height;
        metrics->x_offset = gw->gedit.grid->font_bbx.x_offset;
        metrics->y_offset = gw->gedit.grid->font_bbx.y_offset;
        metrics->ascent = gw->gedit.grid->font_bbx.ascent;
        metrics->descent = gw->gedit.grid->font_bbx.descent;
    } else {
        metrics->width = gw->gedit.grid->glyph_bbx.width;
        metrics->height = gw->gedit.grid->glyph_bbx.height;
        metrics->x_offset = gw->gedit.grid->glyph_bbx.x_offset;
        metrics->y_offset = gw->gedit.grid->glyph_bbx.y_offset;
        metrics->ascent = gw->gedit.grid->glyph_bbx.ascent;
        metrics->descent = gw->gedit.grid->glyph_bbx.descent;
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditSetMetrics(Widget w, bdf_metrics_t *metrics)
#else
XmuttGlyphEditSetMetrics(w, metrics)
Widget w;
bdf_metrics_t *metrics;
#endif
{
    XmuttGlyphEditWidget gw;
    Dimension wd, ht;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    if (metrics == 0)
      return;

    gw = (XmuttGlyphEditWidget) w;

    if (bdf_grid_resize(gw->gedit.grid, metrics)) {
        /*
         * Attempt a resize if the metrics caused the grid to get larger.
         * This should also cause everything to be redrawn.
         */
        _XmuttGlyphEditPreferredGeometry(gw, &wd, &ht);
        _XmuttGlyphEditAttemptResize(gw, wd, ht);

        /*
         * Alert the client the glyph was modified.
         */
        metrics->font_spacing = gw->gedit.grid->spacing;
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    } else if (XtIsRealized(w))
      /*
       * Just force everything to be redrawn.
       */
      XClearArea(XtDisplay(w), XtWindow(w), 0, 0, 0, 0, True);
}

void
#ifndef _NO_PROTO
XmuttGlyphEditSetSpacing(Widget w, unsigned short spacing,
                         unsigned short monowidth)
#else
XmuttGlyphEditSetSpacing(w, spacing, monowidth)
Widget w;
unsigned short spacing, monowidth;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_metrics_t metrics;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;

    gw->gedit.grid->spacing = spacing;
    if (spacing != BDF_PROPORTIONAL) {
        XmuttGlyphEditMetrics(w, &metrics, True);
        metrics.dwidth = metrics.width = monowidth;
        XmuttGlyphEditSetMetrics(w, &metrics);
    }
}

Boolean
#ifndef _NO_PROTO
XmuttGlyphEditModified(Widget w)
#else
XmuttGlyphEditModified(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    return (gw->gedit.grid->modified) ? True : False;
}

void
#ifndef _NO_PROTO
XmuttGlyphEditSetModified(Widget w, Boolean modified)
#else
XmuttGlyphEditSetModified(w, modified)
Widget w;
Boolean modified;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    gw->gedit.grid->modified = (modified == True) ? 1 : 0;
}

Boolean
#ifndef _NO_PROTO
XmuttGlyphEditHasSelection(Widget w)
#else
XmuttGlyphEditHasSelection(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;

    return bdf_has_selection(gw->gedit.grid, 0, 0, 0, 0) ? True : False;
}

Boolean
#ifndef _NO_PROTO
XmuttGlyphEditClipboardEmpty(Widget w)
#else
XmuttGlyphEditClipboardEmpty(w)
Widget w;
#endif
{
    Display *d;
    Window owner;
    Atom atype;
    int afmt;
    unsigned long nitems, nbytes;
    unsigned char *data;
    Boolean empty;

    _XmuttGlyphEditCheckClass(w);

    empty = True;

    d = XtDisplay(w);
    if ((owner = XGetSelectionOwner(d, XmuttGLYPHEDIT_CLIPBOARD(d))) == None)
      return empty;

    if (XGetWindowProperty(d, owner, XmuttGLYPHEDIT_CLIPBOARD(d), 0, 10240,
                           False, XmuttGLYPHEDIT_BITMAP(d), &atype, &afmt,
                           &nitems, &nbytes, &data) == Success) {
        if (nitems > 0) {
            empty = False;
            free((char *) data);
        }
    }

    return empty;
}

void
#ifndef _NO_PROTO
XmuttGlyphEditImage(Widget w, bdf_bitmap_t *image)
#else
XmuttGlyphEditImage(w, image)
Widget w;
bdf_bitmap_t *image;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;

    _XmuttGlyphEditCheckClass(w);

    if (image == 0)
      return;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    bdf_grid_image(gw->gedit.grid, image);
}

bdf_glyph_grid_t *
#ifndef _NO_PROTO
XmuttGlyphEditGrid(Widget w)
#else
XmuttGlyphEditGrid(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    return gw->gedit.grid;
}

bdf_glyph_t *
#ifndef _NO_PROTO
XmuttGlyphEditGlyph(Widget w, Boolean *unencoded)
#else
XmuttGlyphEditGlyph(w, unencoded)
Widget w;
Boolean *unencoded;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    *unencoded = (gw->gedit.grid->unencoded == 0) ? False : True;
    return bdf_grid_glyph(gw->gedit.grid);
}

void
#ifndef _NO_PROTO
XmuttGlyphEditCrop(Widget w)
#else
XmuttGlyphEditCrop(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (bdf_grid_crop(grid, 1)) {
        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditShift(Widget w, short xcount, short ycount)
#else
XmuttGlyphEditShift(w, xcount, ycount)
Widget w;
short xcount, ycount;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (bdf_grid_shift(grid, xcount, ycount)) {
        _XmuttGlyphEditDrawGlyph(w);
        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = 0;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditRotate(Widget w, short degrees)
#else
XmuttGlyphEditRotate(w, degrees)
Widget w;
short degrees;
#endif
{
    int resize;
    Dimension wd, ht;
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (bdf_grid_rotate(grid, degrees, &resize)) {
        if (resize) {
            _XmuttGlyphEditPreferredGeometry(gw, &wd, &ht);
            _XmuttGlyphEditAttemptResize(gw, wd, ht);
        } else if (XtIsRealized(w))
          /*
           * Just force everything to be redrawn.
           */
          XClearArea(XtDisplay(w), XtWindow(w), 0, 0, 0, 0, True);

        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = 0;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditShear(Widget w, short degrees)
#else
XmuttGlyphEditShear(w, degrees)
Widget w;
short degrees;
#endif
{
    int resize;
    Dimension wd, ht;
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (bdf_grid_shear(grid, degrees, &resize)) {
        if (resize) {
            _XmuttGlyphEditPreferredGeometry(gw, &wd, &ht);
            _XmuttGlyphEditAttemptResize(gw, wd, ht);
        } else if (XtIsRealized(w))
          /*
           * Just force everything to be redrawn.
           */
          XClearArea(XtDisplay(w), XtWindow(w), 0, 0, 0, 0, True);

        _XmuttGlyphEditDrawGlyph(w);
        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = 0;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditEmbolden(Widget w)
#else
XmuttGlyphEditEmbolden(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (bdf_grid_embolden(grid)) {
        XClearArea(XtDisplay(w), XtWindow(w), 0, 0, 0, 0, True);

        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = 0;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditFlip(Widget w, short direction)
#else
XmuttGlyphEditFlip(w, direction)
Widget w;
short direction;
#endif
{
    int flipped;
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    flipped = (direction == XmuttGLYPHEDIT_HORIZONTAL) ?
        bdf_grid_flip(grid, -1) : bdf_grid_flip(grid, 1);

    if (flipped) {
        _XmuttGlyphEditDrawGlyph(w);
        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = 0;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditChangeOperation(Widget w, int op)
#else
XmuttGlyphEditChangeOperation(w, op)
Widget w;
int op;
#endif
{
    XmuttGlyphEditWidget gw;
    Boolean call_modify;
    short sx, sy, x, y, wd, ht;
    bdf_glyph_grid_t *grid;
    bdf_metrics_t metrics;
    bdf_bitmap_t image;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    /*
     * If the operation specified is not in the correct range, just return.
     */
    if (op < XmuttGLYPHEDIT_DRAW || op > XmuttGLYPHEDIT_COPY)
      return;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;
    call_modify = True;

    /*
     * Special handling is needed for move and copy operations.  If a
     * selection does not exist yet, then make the move/copy a pending
     * operation and set the operation to select.  Once the selection is made,
     * the operation will be changed to the pending move/copy operation.  If a
     * selection exists, then set the move/copy operation and detach/attach
     * the selection accordingly.
     */
    if (bdf_has_selection(grid, &x, &y, &wd, &ht)) {
        if (op == XmuttGLYPHEDIT_MOVE)
          bdf_detach_selection(grid);
        else if (op == XmuttGLYPHEDIT_COPY)
          bdf_attach_selection(grid);
        else {
            if (op == XmuttGLYPHEDIT_DRAW) {
                /*
                 * Attach the selected part of the bitmap.
                 */
                bdf_attach_selection(grid);

                /*
                 * Erase the selected rectangle.
                 */
                for (sy = y; sy < y + ht; sy++) {
                    for (sx = x; sx < x + wd; sx++)
                      _XmuttGlyphEditDrawPixel(gw, sx, sy, False);
                }
                bdf_lose_selection(grid);

                /*
                 * Update the glyph image.
                 */
                bdf_grid_image(grid, &image);
                cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
                cb.event = 0;
                cb.image = &image;
                XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
                if (image.bytes > 0)
                  free((char *) image.bitmap);
            } else
              call_modify = False;
            gw->gedit.op = op;
        }
        gw->gedit.pending_op = -1;

        /*
         * If indicated, call the modified callback to indicate that
         * the metrics changed.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    } else {
        if (op == XmuttGLYPHEDIT_MOVE || op == XmuttGLYPHEDIT_COPY) {
            gw->gedit.op = _XmuttGLYPHEDIT_SELECT;
            gw->gedit.pending_op = op;
        } else {
            gw->gedit.op = op;
            gw->gedit.pending_op = -1;
        }
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditChangeColor(Widget w, int color)
#else
XmuttGlyphEditChangeColor(w, color)
Widget w;
int color;
#endif
{
    XmuttGlyphEditWidget gw;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    gw->gedit.cidx = color;
}

void
#ifndef _NO_PROTO
XmuttGlyphEditInsertBitmap(Widget w, bdf_bitmap_t *bitmap)
#else
XmuttGlyphEditInsertBitmap(w, bitmap)
Widget w;
bdf_bitmap_t *bitmap;
#endif
{
    XmuttGlyphEditWidget gw;
    Display *d;
    Window win;
    short sx, sy, x, y, wd, ht;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    if (!XtIsRealized(w))
      return;

    d = XtDisplay(w);

    gw = (XmuttGlyphEditWidget) w;

    if ((win = XGetSelectionOwner(d, XmuttGLYPHEDIT_CLIPBOARD(d))) == None) {
        _XmuttGlyphEditOwnClipboard(w);
        win = XtWindow(w);
    } else if (win != XtWindow(w))
      /*
       * Attempt to own the clipboard if this widget does not own it.
       */
      _XmuttGlyphEditOwnClipboard(w);

    if (bdf_has_selection(gw->gedit.grid, &x, &y, &wd, &ht)) {
        /*
         * This widget already has a selection, so release it.
         */
        if (gw->gedit.op != _XmuttGLYPHEDIT_SELECT)
          bdf_attach_selection(gw->gedit.grid);

        for (sy = y; sy < y + ht; sy++) {
            for (sx = x; sx < x + wd; sx++)
              _XmuttGlyphEditDrawPixel(gw, sx, sy, False);
        }
        bdf_lose_selection(gw->gedit.grid);
    }

    bitmap->x = gw->gedit.last_x;
    bitmap->y = gw->gedit.last_y;

    /*
     * If the bitmap being inserted is larger than the current grid, then
     * resize the grid before doing anything else.
     */
    XmuttGlyphEditMetrics(w, &metrics, True);
    if (bitmap->width > metrics.width || bitmap->height > metrics.height) {
        /*
         * Adjust the insert position on the X axis if necessary.
         */
        if (bitmap->width > metrics.width)
          bitmap->x = gw->gedit.grid->base_x +
              gw->gedit.grid->font_bbx.x_offset;
        /*
         * Adjust the insert position on the Y axis and the ascent if
         * necessary.
         */
        if (bitmap->height > metrics.height) {
            bitmap->y = 0;
            metrics.ascent = bitmap->height - gw->gedit.grid->font_bbx.descent;
        }
        metrics.width = bitmap->width;
        metrics.height = bitmap->height;
        XmuttGlyphEditSetMetrics(w, &metrics);
    }

    /*
     * Set the selection in the grid.
     */
    bdf_add_selection(gw->gedit.grid, bitmap);

    /*
     * Now update the grid.
     */
    if (bdf_has_selection(gw->gedit.grid, &x, &y, &wd, &ht)) {
        for (sy = y; sy < y + ht; sy++) {
            for (sx = x; sx < x + wd; sx++)
              _XmuttGlyphEditDrawPixel(gw, sx, sy, True);
        }
    }

    /*
     * Set up and call the image update.
     */
    bdf_grid_image(gw->gedit.grid, &image);
    cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
    cb.event = 0;
    cb.image = &image;
    XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
    if (image.bytes > 0)
      free((char *) image.bitmap);

    /*
     * Alert the client that the widget is changing to the MOVE
     * operation.
     */
    cb.reason = XmuttGLYPHEDIT_OPERATION_CHANGE;
    cb.event = 0;
    cb.operation = XmuttGLYPHEDIT_MOVE;
    XtCallCallbackList(w, gw->gedit.opswitch, (XtPointer) &cb);

    /*
     * Make sure the widget goes into MOVE mode at this point.
     * This allows the user to position what was pasted without
     * destroying the glyph bitmap that was already there.
     */
    if (gw->gedit.op != XmuttGLYPHEDIT_MOVE) {
        gw->gedit.op = XmuttGLYPHEDIT_MOVE;
        gw->gedit.pending_op = -1;
    }

    /*
     * Alert the client the glyph was modified.
     */
    XmuttGlyphEditMetrics(w, &metrics, False);
    cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
    cb.metrics = &metrics;
    XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);

    /*
     * Last, recopy the selection to the clipboard because changing owners
     * causes the data to be lost.
     */
    XmuttGlyphEditCopySelection(w);
}

int
#ifndef _NO_PROTO
XmuttGlyphEditImportXBM(Widget w, String filename)
#else
XmuttGlyphEditImportXBM(w, filename)
Widget w;
String filename;
#endif
{
    int res, bpr, byte;
    unsigned int wd, ht, xh, yh;
    bdf_bitmap_t bmap;

    _XmuttGlyphEditCheckClass(w);

    res = BitmapSuccess;
    if (filename == 0 || *filename == 0)
      return res;

    if ((res = XmuReadBitmapDataFromFile(filename, &wd, &ht, &bmap.bitmap,
                                         &xh, &yh)) != BitmapSuccess)
      return res;

    bmap.bpp = 1;
    bmap.width = (unsigned short) wd;
    bmap.height = (unsigned short) ht;
    bpr = (bmap.width + 7) >> 3;
    bmap.bytes = bpr * bmap.height;

    /*
     * Swap the bits around so the bitmap will show up correctly.
     */
    for (yh = 0; yh < bmap.height; yh++) {
        for (xh = 0; xh < bpr; xh++) {
            byte = bmap.bitmap[(yh * bpr) + xh] & 0xff;
            bmap.bitmap[(yh * bpr) + xh] = bitswap[byte];
        }
    }

    if (bmap.bytes > 0) {
        XmuttGlyphEditInsertBitmap(w, &bmap);
        XtFree((char *) bmap.bitmap);
    }

    return res;
}

int
#ifndef _NO_PROTO
XmuttGlyphEditExportXBM(Widget w, String filename)
#else
XmuttGlyphEditExportXBM(w, filename)
Widget w;
String filename;
#endif
{
    XmuttGlyphEditWidget gw;
    FILE *out;
    bdf_glyph_grid_t *grid;
    int cnt, x, y, sx, sy, bpr, i, byte;
    unsigned char *masks;
    char *np, *ep, name[256];

    _XmuttGlyphEditCheckClass(w);

    if (filename == 0 || *filename == 0)
      return BitmapSuccess;

    /*
     * Attempt to open the output file.
     */
    if ((out = fopen(filename, "w")) == 0)
      return BitmapOpenFailed;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    switch (grid->bpp) {
      case 1: masks = onebpp; break;
      case 2: masks = twobpp; break;
      case 4: masks = fourbpp; break;
    }

    /*
     * Determine the name to use for the bitmap fields based on the filename.
     */
    if ((np = strrchr(filename, '/')) != 0)
      np++;
    else
      np = filename;
    if ((ep = strrchr(np, '.')) == 0)
      ep = np + strlen(np);

    /*
     * If the name begins with a digit, simply prefix it with an 'x' so it can
     * be used with C code.
     */
    if (*np >= '0' && *np <= '9') {
        name[0] = 'x';
        (void) memcpy(&name[1], np, ep - np);
        name[(ep - np) + 1] = 0;
    } else {
        (void) memcpy(name, np, ep - np);
        name[(ep - np)] = 0;
    }

    /*
     * Emit the width and height of the bitmap.
     */
    fprintf(out, "#define %s_width %hd\n", name, grid->glyph_bbx.width);
    fprintf(out, "#define %s_height %hd\n", name, grid->glyph_bbx.height);

    /*
     * Now dump the bitmap.
     */
    sy = grid->glyph_y;
    bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
    fprintf(out, "static char %s_bits[] = {", name);
    for (cnt = y = 0; y < grid->glyph_bbx.height; y++, sy++) {
        for (sx = grid->glyph_x * grid->bpp, x = 0;
             x < grid->glyph_bbx.width; ) {
            /*
             * Accumulate bits until there are 8.
             */
            for (byte = i = 0; x < grid->glyph_bbx.width && i < 8;
                 i++, x++, sx += grid->bpp) {
                if (grid->bitmap[(sy * bpr) + (sx >> 3)] &
                    masks[(sx & 7) / grid->bpp])
                  byte |= 1 << i;
            }
            /*
             * Move to a new line or emit a space.
             */
            if (cnt % 12 == 0)
              fprintf(out, "\n    ");
            else
              putc(' ', out);
            fprintf(out, "0x%02x,", byte & 0xff);
            cnt++;
        }
    }
    fprintf(out, "\n};\n");
    fclose(out);

    return BitmapSuccess;
}

void
#ifndef _NO_PROTO
XmuttGlyphEditCopySelection(Widget w)
#else
XmuttGlyphEditCopySelection(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    Display *d;
    unsigned char *sel;
    unsigned long bytes;

    _XmuttGlyphEditCheckClass(w);

    /*
     * If the widget is not realized, there is no point to this operation.
     */
    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditWidget) w;
    d = XtDisplay(w);

    /*
     * If the widget has no selection, then this routine will return 0.
     */
    if ((sel = _XmuttGlyphEditEncodeSelection(gw, &bytes)) == 0)
      return;

    /*
     * Go ahead and actually write the data to the clipboard and then free the
     * buffer.
     */
    XChangeProperty(d, XtWindow(w), XmuttGLYPHEDIT_CLIPBOARD(d),
                    XmuttGLYPHEDIT_BITMAP(d), 8, PropModeReplace, sel, bytes);

    XtFree((char *) sel);
}

void
#ifndef _NO_PROTO
XmuttGlyphEditCutSelection(Widget w)
#else
XmuttGlyphEditCutSelection(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    Display *d;
    unsigned char *sel;
    unsigned long bytes;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    /*
     * If the widget is not realized, there is no point to this operation.
     */
    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditWidget) w;
    d = XtDisplay(w);

    /*
     * If the widget has no selection, then this routine will return 0.
     */
    if ((sel = _XmuttGlyphEditEncodeSelection(gw, &bytes)) == 0)
      return;

    /*
     * Go ahead and actually write the data to the clipboard and then free the
     * buffer.
     */
    XChangeProperty(d, XtWindow(w), XmuttGLYPHEDIT_CLIPBOARD(d),
                    XmuttGLYPHEDIT_BITMAP(d), 8, PropModeReplace, sel, bytes);

    XtFree((char *) sel);

    /*
     * Now actually delete the selection and update the glyph.
     */
    bdf_delete_selection(gw->gedit.grid);
    bdf_lose_selection(gw->gedit.grid);
    if (gw->gedit.op != XmuttGLYPHEDIT_DRAW) {
        gw->gedit.pending_op = gw->gedit.op;
        gw->gedit.op = _XmuttGLYPHEDIT_SELECT;
    }
    _XmuttGlyphEditDrawGlyph(w);

    bdf_grid_image(gw->gedit.grid, &image);
    cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
    cb.event = 0;
    cb.image = &image;
    XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
    if (image.bytes > 0)
      free((char *) image.bitmap);

    /*
     * Alert the client the glyph was modified.
     */
    XmuttGlyphEditMetrics(w, &metrics, False);
    cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
    cb.metrics = &metrics;
    XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttGlyphEditPasteSelection(Widget w)
#else
XmuttGlyphEditPasteSelection(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    Display *d;
    Window win;
    Atom atype;
    int afmt;
    unsigned long nitems, nbytes;
    unsigned char *data, *bp;
    short sx, sy, x, y, wd, ht;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    if (!XtIsRealized(w))
      return;

    d = XtDisplay(w);

    gw = (XmuttGlyphEditWidget) w;

    if ((win = XGetSelectionOwner(d, XmuttGLYPHEDIT_CLIPBOARD(d))) == None) {
        _XmuttGlyphEditOwnClipboard(w);
        win = XtWindow(w);
    }

    nitems = nbytes = 0;
    XGetWindowProperty(d, win, XmuttGLYPHEDIT_CLIPBOARD(d), 0, 10240,
                       False, XmuttGLYPHEDIT_BITMAP(d), &atype, &afmt,
                       &nitems, &nbytes, &data);

    /*
     * Attempt to own the clipboard after getting the value if this widget
     * does not own it.
     */
    if (win != XtWindow(w))
      _XmuttGlyphEditOwnClipboard(w);

    if (nitems > 0) {
        /*
         * Got a bitmap.
         */

        if (bdf_has_selection(gw->gedit.grid, &x, &y, &wd, &ht)) {
            /*
             * This widget already has a selection, so release it.
             */
            if (gw->gedit.op != _XmuttGLYPHEDIT_SELECT)
              bdf_attach_selection(gw->gedit.grid);

            for (sy = y; sy < y + ht; sy++) {
                for (sx = x; sx < x + wd; sx++)
                  _XmuttGlyphEditDrawPixel(gw, sx, sy, False);
            }
            bdf_lose_selection(gw->gedit.grid);
        }

        bp = data;

        if (!bdf_little_endian()) {
            image.bpp = (*bp++ << 8) & 0xff00;
            image.bpp |= *bp++;
            image.width = (*bp++ << 8) & 0xff00;
            image.width |= *bp++;
            image.height = (*bp++ << 8) & 0xff00;
            image.height |= *bp++;
        } else {
            image.bpp = *bp++ & 0xff;
            image.bpp |= (*bp++ << 8) & 0xff00;
            image.width = *bp++ & 0xff;
            image.width |= (*bp++ << 8) & 0xff00;
            image.height = *bp++ & 0xff;
            image.height |= (*bp++ << 8) & 0xff00;
        }

        image.bytes = (((image.width * image.bpp) + 7) >> 3) * image.height;
        image.bitmap = bp;

        image.x = gw->gedit.last_x;
        image.y = gw->gedit.last_y;

        /*
         * If the bitmap being pasted is larger than the current grid, then
         * resize the grid before doing anything else.
         */
        XmuttGlyphEditMetrics(w, &metrics, True);
        if (image.width > metrics.width || image.height > metrics.height) {
            /*
             * Adjust the insert position on the X axis if necessary.
             */
            if (image.width > metrics.width)
              image.x = gw->gedit.grid->base_x +
                  gw->gedit.grid->font_bbx.x_offset;
            /*
             * Adjust the insert position on the Y axis and the ascent if
             * necessary.
             */
            if (image.height > metrics.height) {
                image.y = 0;
                metrics.ascent = image.height -
                    gw->gedit.grid->font_bbx.descent;
            }
            metrics.width = image.width;
            metrics.height = image.height;
            XmuttGlyphEditSetMetrics(w, &metrics);
        }

        /*
         * Set the selection in the grid.
         */
        bdf_add_selection(gw->gedit.grid, &image);

        /*
         * Now update the grid.
         */
        if (bdf_has_selection(gw->gedit.grid, &x, &y, &wd, &ht)) {
            for (sy = y; sy < y + ht; sy++) {
                for (sx = x; sx < x + wd; sx++)
                  _XmuttGlyphEditDrawPixel(gw, sx, sy, True);
            }
        }

        /*
         * Set up and call the image update.
         */
        bdf_grid_image(gw->gedit.grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = 0;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Free up the original value passed.
         */
        XFree((char *) data);

        /*
         * Alert the client that the widget is changing to the MOVE
         * operation.
         */
        cb.reason = XmuttGLYPHEDIT_OPERATION_CHANGE;
        cb.event = 0;
        cb.operation = XmuttGLYPHEDIT_MOVE;
        XtCallCallbackList(w, gw->gedit.opswitch, (XtPointer) &cb);

        /*
         * Make sure the widget goes into MOVE mode at this point.
         * This allows the user to position what was pasted without
         * destroying the glyph bitmap that was already there.
         */
        if (gw->gedit.op != XmuttGLYPHEDIT_MOVE) {
            gw->gedit.op = XmuttGLYPHEDIT_MOVE;
            gw->gedit.pending_op = -1;
        }

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);

        /*
         * Last, recopy the selection to the clipboard because changing owners
         * causes the data to be lost.
         */
        XmuttGlyphEditCopySelection(w);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphEditSelectAll(Widget w)
#else
XmuttGlyphEditSelectAll(w)
Widget w;
#endif
{
    XmuttGlyphEditWidget gw;
    short tx, ty, sx, sy, wd, ht;
    bdf_glyph_grid_t *grid;
    XmuttGlyphEditUpdateCallbackStruct cb;

    _XmuttGlyphEditCheckClass(w);

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    /*
     * If a selection already exists, clear it.
     */
    if (bdf_has_selection(grid, &sx, &sy, &wd, &ht)) {
        if (gw->gedit.op != _XmuttGLYPHEDIT_SELECT)
          bdf_attach_selection(grid);

        for (ty = sy; ty < sy + ht; ty++) {
            for (tx = sx; tx < sx + wd; tx++)
              _XmuttGlyphEditDrawPixel(gw, tx, ty, False);
        }
        bdf_lose_selection(grid);
    }

    wd = grid->glyph_bbx.width;
    ht = grid->glyph_bbx.height;

    sx = gw->gedit.sel_start.x = grid->glyph_x;
    sy = gw->gedit.sel_start.y = grid->glyph_y;
    gw->gedit.sel_end.x = grid->glyph_x + wd;
    gw->gedit.sel_end.y = grid->glyph_y + ht;

    /*
     * Gain control of the GLYPHEDIT_CLIPBOARD atom.
     */
    _XmuttGlyphEditOwnClipboard(w);

    bdf_set_selection(grid, sx, sy, wd, ht);
    bdf_detach_selection(grid);

    for (ty = sy; ty < sy + ht; ty++) {
        for (tx = sx; tx < sx + wd; tx++)
          _XmuttGlyphEditDrawPixel(gw, tx, ty, True);
    }

    /*
     * Alert the client that the widget is changing to the MOVE
     * operation.
     */
    cb.reason = XmuttGLYPHEDIT_OPERATION_CHANGE;
    cb.event = 0;
    cb.operation = XmuttGLYPHEDIT_MOVE;
    XtCallCallbackList(w, gw->gedit.opswitch, (XtPointer) &cb);

    /*
     * Make sure the widget goes into MOVE mode at this point.
     * This allows the user to position what was pasted without
     * destroying the glyph bitmap that was already there.
     */
    if (gw->gedit.op != XmuttGLYPHEDIT_MOVE) {
        gw->gedit.op = XmuttGLYPHEDIT_MOVE;
        gw->gedit.pending_op = -1;
    }
}

/**************************************************************************
 *
 * Action functions.
 *
 **************************************************************************/

static void
#ifndef _NO_PROTO
GainFocus(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
GainFocus(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;

    /*
     * There is only one way to really gain the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    gw = (XmuttGlyphEditWidget) w;
    gw->gedit.hasfocus = True;

    _XmuttGlyphEditHighlightWindow(w);
}

static void
#ifndef _NO_PROTO
LoseFocus(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
LoseFocus(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;

    /*
     * There is only one way to really lose the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    gw = (XmuttGlyphEditWidget) w;
    gw->gedit.hasfocus = False;

    _XmuttGlyphEditHighlightWindow(w);
}

static void
#ifndef _NO_PROTO
StartAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
StartAction(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    int changed;
    XmuttGlyphEditWidget gw;
    Position x, y;
    short tx, ty, sx, sy, wd, ht;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    _XmuttGlyphEditXYToCoords(gw, event, &x, &y);

    /*
     * Return if the mouse is off the edge of the grid.
     */
    if (x < 0 || y < 0)
      return;

    /*
     * Force a traversal to this widget.
     */
    (void) XmProcessTraversal(w, XmTRAVERSE_CURRENT);

    changed = 0;
    if (gw->gedit.op == XmuttGLYPHEDIT_DRAW) {
        /*
         * Drawing.
         */
        if (event->type == ButtonPress) {
            switch (event->xbutton.button) {
              case Button1:
                if ((changed = bdf_grid_set_pixel(grid, x, y, gw->gedit.cidx)))
                  _XmuttGlyphEditDrawPixel(gw, x, y, False);
                break;
              case Button2:
                if ((changed = bdf_grid_invert_pixel(grid, x, y,
                                                     gw->gedit.cidx)))
                  _XmuttGlyphEditDrawPixel(gw, x, y, False);
                break;
              case Button3:
                if ((changed = bdf_grid_clear_pixel(grid, x, y)))
                  _XmuttGlyphEditDrawPixel(gw, x, y, False);
                break;
            }
        } else if (event->type == KeyPress) {
            /*
             * Current assume that all key presses are the same as inverting
             * a pixel.
             */
            if ((changed = bdf_grid_invert_pixel(grid, x, y, gw->gedit.cidx)))
              _XmuttGlyphEditDrawPixel(gw, x, y, False);
        }

        /*
         * If one of the pixels changed, then call the callback.
         */
        if (changed) {
            bdf_grid_image(grid, &image);
            cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
            cb.event = event;
            cb.image = &image;
            XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
            if (image.bytes > 0)
              free((char *) image.bitmap);

            /*
             * Alert the client the glyph was modified.
             */
            XmuttGlyphEditMetrics(w, &metrics, False);
            cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
            cb.metrics = &metrics;
            XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
        }
    } else if (gw->gedit.op == _XmuttGLYPHEDIT_SELECT) {
        /*
         * If a selection already exists, clear it.
         */
        if (bdf_has_selection(grid, &sx, &sy, &wd, &ht)) {
            if (gw->gedit.pending_op != -1)
              bdf_attach_selection(grid);

            for (ty = sy; ty < sy + ht; ty++) {
                for (tx = sx; tx < sx + wd; tx++)
                  _XmuttGlyphEditDrawPixel(gw, tx, ty, False);
            }
            bdf_lose_selection(grid);
        }

        /*
         * Select the pixel at the point and initialize the selection
         * rectangle.
         */
        _XmuttGlyphEditDrawPixel(gw, x, y, True);

        gw->gedit.sel_start.x = gw->gedit.sel_end.x = x;
        gw->gedit.sel_start.y = gw->gedit.sel_end.y = y;
    } else {
        /*
         * Check to see if this is Button3 and a selection exists.  If so,
         * then copy the selection to the clipboard and return.
         */
        if (event->xbutton.button == Button3 &&
            bdf_has_selection(grid, &sx, &sy, &wd, &ht)) {
            XmuttGlyphEditCopySelection(w);
            gw->gedit.last_x = x;
            gw->gedit.last_y = y;
            return;
        }

        /*
         * The operation is one of move or copy.  If the button is clicked
         * outside the selection, remove the selection and start over.
         */
        if (bdf_has_selection(grid, &sx, &sy, &wd, &ht) &&
            !bdf_in_selection(grid, x, y, 0)) {

            if (gw->gedit.op != _XmuttGLYPHEDIT_SELECT)
              bdf_attach_selection(grid);

            for (ty = sy; ty < sy + ht; ty++) {
                for (tx = sx; tx < sx + wd; tx++)
                  _XmuttGlyphEditDrawPixel(gw, tx, ty, False);
            }
            bdf_lose_selection(grid);

            gw->gedit.pending_op = gw->gedit.op;
            gw->gedit.op = _XmuttGLYPHEDIT_SELECT;

            /*
             * Select the pixel at the point and initialize the selection
             * rectangle.
             */
            _XmuttGlyphEditDrawPixel(gw, x, y, True);

            gw->gedit.sel_start.x = gw->gedit.sel_end.x = x;
            gw->gedit.sel_start.y = gw->gedit.sel_end.y = y;
        }
    }

    /*
     * Set the last coordinate to the point just handled.
     */
    gw->gedit.last_x = x;
    gw->gedit.last_y = y;
}

#define ANYBUTTON (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)

static void
#ifndef _NO_PROTO
ExtendAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ExtendAction(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    int changed;
    XmuttGlyphEditWidget gw;
    Position x, y, ix, iy;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    _XmuttGlyphEditXYToCoords(gw, event, &x, &y);

    /*
     * Return if the mouse is off the edges of the grid or the mouse is still
     * on the same point as the last one.
     */
    if (x < 0 || y < 0 ||
        (x == gw->gedit.last_x && y == gw->gedit.last_y))
      return;

    /*
     * Report the coordinates before doing anything else.
     */
    cb.reason = XmuttGLYPHEDIT_POINTER_MOVED;
    cb.event = event;
    cb.x = x - grid->base_x;
    cb.y = -(y - grid->base_y) - 1;
    cb.color = bdf_grid_color_at(grid, x, y);
    XtCallCallbackList(w, gw->gedit.move, (XtPointer) &cb);

    ix = gw->gedit.last_x;
    iy = gw->gedit.last_y;

    /*
     * Set the current coordinates as the last known.
     */
    gw->gedit.last_x = x;
    gw->gedit.last_y = y;

    /*
     * If the event is a simple motion event, then return at this point.
     */
    if (gw->gedit.hasfocus == False ||
        (event->type == MotionNotify && !(event->xmotion.state & ANYBUTTON)))
      return;

    changed = 0;
    if (gw->gedit.op == XmuttGLYPHEDIT_DRAW) {
        /*
         * Drawing.
         */
        if (event->xmotion.state & Button1Mask) {
            if ((changed = bdf_grid_set_pixel(grid, x, y, gw->gedit.cidx)))
              _XmuttGlyphEditDrawPixel(gw, x, y, False);
        } else if (event->xmotion.state & Button2Mask) {
            if ((changed = bdf_grid_invert_pixel(grid, x, y, gw->gedit.cidx)))
              _XmuttGlyphEditDrawPixel(gw, x, y, False);
        } else if (event->xmotion.state & Button3Mask) {
            if ((changed = bdf_grid_clear_pixel(grid, x, y)))
              _XmuttGlyphEditDrawPixel(gw, x, y, False);
        }

        /*
         * If one of the pixels changed, then call the callback.
         */
        if (changed) {
            bdf_grid_image(grid, &image);
            cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
            cb.event = event;
            cb.image = &image;
            XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
            if (image.bytes > 0)
              free((char *) image.bitmap);

            /*
             * Alert the client the glyph was modified.
             */
            XmuttGlyphEditMetrics(w, &metrics, False);
            cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
            cb.metrics = &metrics;
            XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
        }
    } else if (gw->gedit.op == _XmuttGLYPHEDIT_SELECT) {
        /*
         * Determine the other point on the intersection rectangle.
         */
        ix = gw->gedit.sel_start.x;
        iy = gw->gedit.sel_start.y;

        if (x > ix)
          ix = MIN(gw->gedit.sel_end.x, x);
        else if (x < ix)
          ix = MAX(gw->gedit.sel_end.x, x);

        if (y > iy)
          iy = MIN(gw->gedit.sel_end.y, y);
        else if (y < iy)
          iy = MAX(gw->gedit.sel_end.y, y);

        /*
         * Clear the pixels outside the intersection of the old selection
         * rectangle and the new selection rectangle.
         */
        _XmuttGlyphEditUpdateSelection(gw, ix, iy, False);

        /*
         * Set the new endpoint of the selection rectangle.
         */
        gw->gedit.sel_end.x = x;
        gw->gedit.sel_end.y = y;

        /*
         * Set all pixels outside the intersection of the old selection
         * rectangle and the new selection rectangle, but inside the new
         * selection rectangle.
         */
        _XmuttGlyphEditUpdateSelection(gw, ix, iy, True);
    } else {
        /*
         * A move or copy is in progress.
         */
        if (bdf_has_selection(grid, 0, 0, 0, 0) &&
            bdf_grid_shift(grid, x - ix, y - iy)) {
            _XmuttGlyphEditDrawGlyph(w);
            bdf_grid_image(grid, &image);
            cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
            cb.event = event;
            cb.image = &image;
            XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
            if (image.bytes > 0)
              free((char *) image.bitmap);

            /*
             * Alert the client the glyph was modified.
             */
            XmuttGlyphEditMetrics(w, &metrics, False);
            cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
            cb.metrics = &metrics;
            XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
        }
    }
}

static void
#ifndef _NO_PROTO
EndAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
EndAction(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;
    short sx, sy, ex, ey;
    bdf_glyph_grid_t *grid;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (gw->gedit.hasfocus == False)
      return;

    sx = MIN(gw->gedit.sel_start.x, gw->gedit.sel_end.x);
    ex = MAX(gw->gedit.sel_start.x, gw->gedit.sel_end.x);
    sy = MIN(gw->gedit.sel_start.y, gw->gedit.sel_end.y);
    ey = MAX(gw->gedit.sel_start.y, gw->gedit.sel_end.y);

    if (gw->gedit.op == _XmuttGLYPHEDIT_SELECT) {
        if (sx == ex && sy == ey)
          _XmuttGlyphEditDrawPixel(gw, gw->gedit.sel_start.x,
                                   gw->gedit.sel_start.y, False);
        else {
            /*
             * Gain control of the GLYPHEDIT_CLIPBOARD atom.
             */
            _XmuttGlyphEditOwnClipboard(w);

            bdf_set_selection(grid, sx, sy, (ex - sx) + 1, (ey - sy) + 1);

            /*
             * Switch to a move/copy operations if necessary.
             */
            if (gw->gedit.pending_op != -1) {
                gw->gedit.op = gw->gedit.pending_op;
                gw->gedit.pending_op = -1;
                /*
                 * If the pending operation is a move, then make sure the
                 * selection is detached.
                 */
                if (gw->gedit.op == XmuttGLYPHEDIT_MOVE)
                  bdf_detach_selection(grid);
            }
        }
    }
}

static void
#ifndef _NO_PROTO
HandleClipboard(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
HandleClipboard(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;
    short sx, sy, x, y, wd, ht;

    gw = (XmuttGlyphEditWidget) w;

    if (*num_params == 0)
      return;

    if (params[0][0] == 'l') {
        /*
         * Lose control of the clipboard.
         */

        /*
         * If this function is called for some invalid reason, just return if
         * this widget does not own the clipboard atom.
         */
        if (gw->gedit.owns_clipboard == False)
          return;

        /*
         * If the widget owns the clipboard atom, then make sure the selection
         * is removed and the glyph redrawn.
         */
        if (bdf_has_selection(gw->gedit.grid, &x, &y, &wd, &ht)) {
            if (gw->gedit.op != _XmuttGLYPHEDIT_SELECT)
              bdf_attach_selection(gw->gedit.grid);

            for (sy = y; sy < y + ht; sy++) {
                for (sx = x; sx < x + wd; sx++)
                  _XmuttGlyphEditDrawPixel(gw, sx, sy, False);
            }
            bdf_lose_selection(gw->gedit.grid);
            if (gw->gedit.op != XmuttGLYPHEDIT_DRAW) {
                gw->gedit.pending_op = gw->gedit.op;
                gw->gedit.op = _XmuttGLYPHEDIT_SELECT;
            }
        }

        /*
         * Indicate the clipboard has been lost.
         */
        gw->gedit.owns_clipboard = False;
        return;
    }

    /*
     * The remaining operations can only be done if the widget has the focus.
     */
    if (gw->gedit.hasfocus == False)
      return;

    if (params[0][0] == 'p')
      XmuttGlyphEditPasteSelection(w);
    else if (params[0][0] == 'c') {
        if (params[0][1] == 'o')
          XmuttGlyphEditCopySelection(w);
        else
          XmuttGlyphEditCutSelection(w);
    }
}

static void
#ifndef _NO_PROTO
ColorChange(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ColorChange(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    int n;
    XmuttGlyphEditWidget gw;
    XmuttGlyphEditUpdateCallbackStruct cb;

    gw = (XmuttGlyphEditWidget) w;

    if (gw->gedit.grid->bpp == 1)
      return;

    n = (1 << gw->gedit.grid->bpp) - 1;

    switch (params[0][0]) {
      case 'd': gw->gedit.cidx = (gw->gedit.cidx + 1) & n; break;
      case 'l':
        if (gw->gedit.cidx == 0)
          gw->gedit.cidx = n;
        else
          gw->gedit.cidx--;
        break;
    }
    cb.reason = XmuttGLYPHEDIT_COLOR_CHANGE;
    cb.event = event;
    cb.color = gw->gedit.cidx;
    XtCallCallbackList(w, gw->gedit.opswitch, (XtPointer) &cb);
}

static void
#ifndef _NO_PROTO
ShiftGrid(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ShiftGrid(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    int shifted;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (gw->gedit.hasfocus == False)
      return;

    shifted = 0;
    switch (params[0][0]) {
      case 'r': shifted = bdf_grid_shift(grid, 1, 0); break;
      case 'l': shifted = bdf_grid_shift(grid, -1, 0); break;
      case 'd': shifted = bdf_grid_shift(grid, 0, 1); break;
      case 'u': shifted = bdf_grid_shift(grid, 0, -1); break;
    }
    if (shifted) {
        _XmuttGlyphEditDrawGlyph(w);
        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = event;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics; 
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

static void
#ifndef _NO_PROTO
RotateGrid(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
RotateGrid(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    int rotated, resize;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (gw->gedit.hasfocus == False)
      return;

    rotated = 0;
    switch (params[0][0]) {
      case '+': rotated = bdf_grid_rotate(grid, 1, &resize); break;
      case '-': rotated = bdf_grid_rotate(grid, -1, &resize); break;
    }
    if (rotated) {
        _XmuttGlyphEditDrawGlyph(w);
        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = event;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

static void
#ifndef _NO_PROTO
FlipGrid(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
FlipGrid(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;
    bdf_glyph_grid_t *grid;
    int flipped;
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    XmuttGlyphEditUpdateCallbackStruct cb;

    gw = (XmuttGlyphEditWidget) w;
    grid = gw->gedit.grid;

    if (gw->gedit.hasfocus == False)
      return;

    flipped = 0;
    switch (params[0][0]) {
      case 'h': flipped = bdf_grid_flip(grid, -1); break;
      case 'v': flipped = bdf_grid_flip(grid, 1); break;
    }
    if (flipped) {
        _XmuttGlyphEditDrawGlyph(w);
        bdf_grid_image(grid, &image);
        cb.reason = XmuttGLYPHEDIT_IMAGE_CHANGE;
        cb.event = event;
        cb.image = &image;
        XtCallCallbackList(w, gw->gedit.change, (XtPointer) &cb);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Alert the client the glyph was modified.
         */
        XmuttGlyphEditMetrics(w, &metrics, False);
        cb.reason = XmuttGLYPHEDIT_GLYPH_MODIFIED;
        cb.metrics = &metrics;
        XtCallCallbackList(w, gw->gedit.modified, (XtPointer) &cb);
    }
}

static void
#ifndef _NO_PROTO
Traverse(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
Traverse(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditWidget gw;

    gw = (XmuttGlyphEditWidget) w;

    if (gw->gedit.hasfocus == False)
      return;

    switch (params[0][0]) {
      case 'n':
        (void) XmProcessTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
        break;
      case 'p':
        (void) XmProcessTraversal(w, XmTRAVERSE_PREV_TAB_GROUP);
        break;
      case 'r':
        (void) XmProcessTraversal(w, XmTRAVERSE_RIGHT);
        break;
      case 'l':
        (void) XmProcessTraversal(w, XmTRAVERSE_LEFT);
        break;
      case 'u':
        (void) XmProcessTraversal(w, XmTRAVERSE_UP);
        break;
      case 'd':
        (void) XmProcessTraversal(w, XmTRAVERSE_DOWN);
        break;
    }
}

#if defined(XlibSpecificationRelease) && (XlibSpecificationRelease == 6)

static char default_trans[] = "\
<SelClr>: handle-clipboard(lose)\n\
<FocusIn>: gain-focus()\n\
<FocusOut>: lose-focus()\n\
Shift Ctrl ~Meta ~Alt<Key>Return: start-action()\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Left: shift(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Up: shift(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Down: shift(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Right: shift(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Left: shift(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Up: shift(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Down: shift(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfRight: shift(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfLeft: shift(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfUp: shift(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfDown: shift(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>9: rotate(-)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>0: rotate(+)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>-: flip(horizontal)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>=: flip(vertical)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>comma: color-change(lighter)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>period: color-change(darker)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>z: color-change(lighter)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>x: color-change(darker)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>SunCopy: handle-clipboard(copy)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>SunCut: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>SunPaste: handle-clipboard(paste)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfCopy: handle-clipboard(copy)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfCut: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfPaste: handle-clipboard(paste)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfBackSpace: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfDelete: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>BackSpace: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Delete: handle-clipboard(cut)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(prev)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(next)\n\
~Shift ~Ctrl ~Meta ~Alt<BtnDown>: start-action()\n\
~Shift ~Ctrl ~Meta ~Alt<Motion>: extend-action()\n\
~Shift ~Ctrl ~Meta ~Alt<BtnUp>: end-action()\n\
Shift ~Ctrl ~Meta ~Alt<Btn2Down>: handle-clipboard(paste)\n\
";

#else /* R5 or lower */

static char default_trans[] = "\
<SelClr>: handle-clipboard(lose)\n\
<FocusIn>: gain-focus()\n\
<FocusOut>: lose-focus()\n\
Shift Ctrl ~Meta ~Alt<Key>Return: start-action()\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Right: shift(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Left: shift(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Up: shift(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Down: shift(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfRight: shift(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfLeft: shift(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfUp: shift(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfDown: shift(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>9: rotate(-)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>0: rotate(+)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>-: flip(horizontal)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>=: flip(vertical)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>comma: color-change(lighter)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>period: color-change(darker)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>z: color-change(lighter)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>x: color-change(darker)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>SunCopy: handle-clipboard(copy)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>SunCut: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>SunPaste: handle-clipboard(paste)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfCopy: handle-clipboard(copy)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfCut: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfPaste: handle-clipboard(paste)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfBackSpace: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfDelete: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>BackSpace: handle-clipboard(cut)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Delete: handle-clipboard(cut)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(prev)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(next)\n\
~Shift ~Ctrl ~Meta ~Alt<BtnDown>: start-action()\n\
~Shift ~Ctrl ~Meta ~Alt<Motion>: extend-action()\n\
~Shift ~Ctrl ~Meta ~Alt<BtnUp>: end-action()\n\
Shift ~Ctrl ~Meta ~Alt<Btn2Down>: handle-clipboard(paste)\n\
";

#endif /* R5 or lower */

static XtActionsRec actions[] = {
    {"traverse", Traverse},
    {"gain-focus", GainFocus},
    {"lose-focus", LoseFocus},
    {"shift", ShiftGrid},
    {"rotate", RotateGrid},
    {"flip", FlipGrid},
    {"start-action", StartAction},
    {"extend-action", ExtendAction},
    {"end-action", EndAction},
    {"handle-clipboard", HandleClipboard},
    {"color-change", ColorChange},
};

static XtResource resources[] = {
    {
        XmNtraversalOn,
        XmCTraversalOn,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmPrimitiveWidget, primitive.traversal_on),
        XmRImmediate,
        (XtPointer) True,
    },
    {
        XmNnavigationType,
        XmCNavigationType,
        XmRNavigationType,
        sizeof(unsigned char),
        XtOffset(XmPrimitiveWidget, primitive.navigation_type),
        XmRImmediate,
        (XtPointer) XmTAB_GROUP,
    },
    {
        XmNglyphGrid,
        XmCGlyphGrid,
        XmRPointer,
        sizeof(bdf_glyph_grid_t *),
        XtOffset(XmuttGlyphEditWidget, gedit.grid),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNbaselineColor,
        XmCBaselineColor,
        XmRPixel,
        sizeof(Pixel),
        XtOffset(XmuttGlyphEditWidget, gedit.bbxpix),
        XmRString,
        (XtPointer) "red",
    },
    {
        XmNselectionColor,
        XmCSelectionColor,
        XmRPixel,
        sizeof(Pixel),
        XtOffset(XmuttGlyphEditWidget, gedit.selpix),
        XmRCallProc,
        (XtPointer) _XmuttGlyphEditDefaultColor,
    },
    {
        XmNcursorColor,
        XmCCursorColor,
        XmRPixel,
        sizeof(Pixel),
        XtOffset(XmuttGlyphEditWidget, gedit.cursorpix),
        XmRCallProc,
        (XtPointer) _XmuttGlyphEditDefaultColor,
    },
    {
        XmNimageChangeCallback,
        XmCImageChangeCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttGlyphEditWidget, gedit.change),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNpointerMovedCallback,
        XmCPointerMovedCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttGlyphEditWidget, gedit.move),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNoperationSwitchCallback,
        XmCOperationSwitchCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttGlyphEditWidget, gedit.opswitch),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNglyphModifiedCallback,
        XmCGlyphModifiedCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttGlyphEditWidget, gedit.modified),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNdefaultPixelSize,
        XmCDefaultPixelSize,
        XmRInt,
        sizeof(int),
        XtOffset(XmuttGlyphEditWidget, gedit.default_pixel_size),
        XmRImmediate,
        (XtPointer) 10,
    },
    {
        XmNshowXHeight,
        XmCShowXHeight,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttGlyphEditWidget, gedit.show_x_height),
        XmRImmediate,
        (XtPointer) False,
    },
    {
        XmNshowCapHeight,
        XmCShowCapHeight,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttGlyphEditWidget, gedit.show_cap_height),
        XmRImmediate,
        (XtPointer) False,
    },
    {
        XmNcolorList,
        XmCColorList,
        XmRPointer,
        sizeof(unsigned long *),
        XtOffset(XmuttGlyphEditWidget, gedit.colors),
        XmRImmediate,
        (XtPointer) 0,
    },
};

externaldef(xmuttglypheditclassrec)
XmuttGlyphEditClassRec xmuttGlyphEditClassRec = {
    {
	(WidgetClass) &xmPrimitiveClassRec,	/* superclass		*/
	"XmuttGlyphEdit",			/* class_name		*/
	sizeof(XmuttGlyphEditRec),		/* widget_size		*/
        ClassInitialize,			/* class_initialize	*/
	ClassPartInitialize,			/* chained class init	*/
	FALSE,					/* class_inited		*/
	Initialize,				/* initialize		*/
        NULL,					/* initialize hook	*/
	Realize,				/* realize		*/
	actions,				/* actions		*/
	XtNumber(actions),			/* num_actions		*/
	resources,				/* resources		*/
	XtNumber(resources),			/* num_resources	*/
	NULLQUARK,				/* xrm_class		*/
	TRUE,					/* compress_motion	*/
	XtExposeCompressMaximal,		/* compress_exposure	*/
        TRUE,					/* compress enter/exit	*/
	FALSE,					/* visible_interest	*/
	Destroy,				/* destroy		*/
	Resize,					/* resize		*/
	Redisplay,				/* expose		*/
	(XtSetValuesFunc) SetValues,		/* set_values		*/
        NULL,					/* set values hook	*/
        XtInheritSetValuesAlmost,		/* set values almost	*/
        NULL,					/* get values hook	*/
	NULL,					/* accept_focus		*/
        XtVersion,				/* version		*/
        NULL,					/* callback offsetlst	*/
        default_trans,				/* default trans	*/
	XtInheritQueryGeometry,			/* query geo proc	*/
	XtInheritDisplayAccelerator,		/* display accelerator	*/
        NULL,					/* extension record reserved*/
    },

    {						/* XmPrimitive		*/
        (XtWidgetProc) _XtInherit,		/* border_highlight	*/
        (XtWidgetProc) _XtInherit,		/* border_unhighlight	*/
        NULL,					/* translations		*/
        NULL,					/* arm_and_activate	*/
        NULL,					/* syn resources	*/
        0,					/* num syn_resources	*/
        0,					/* extension		*/
    },

    {						/* XmuttGlyphEdit	*/
        0,					/* cursor		*/
        0,					/* refcnt		*/
        0,					/* Extension record pointer.*/
    }
};

externaldef(xmuttglypheditwidgetclass)
WidgetClass xmuttGlyphEditWidgetClass = (WidgetClass) &xmuttGlyphEditClassRec;

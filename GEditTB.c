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
static char rcsid[] __attribute__ ((unused)) = "$Id: GEditTB.c,v 1.6 2001/09/19 21:00:41 mleisher Exp $";
#else
static char rcsid[] = "$Id: GEditTB.c,v 1.6 2001/09/19 21:00:41 mleisher Exp $";
#endif
#endif

#include <stdlib.h>
#include "GEditTBP.h"
#include "bitmaps.h"

/*
 * Sizes for the button shapes.
 */
#define _GETB_TOGGLE_SIZE 36
#define _GETB_BUTTON_SIZE 33

/**************************************************************************
 *
 * Button indexes.
 *
 **************************************************************************/

#define _GETB_DRAW_TOGGLE     0
#define _GETB_MOVE_TOGGLE     1
#define _GETB_COPY_TOGGLE     2
#define _GETB_FLIPH_BUTTON    3
#define _GETB_FLIPV_BUTTON    4
#define _GETB_SHEAR_BUTTON    5
#define _GETB_RLEFT_BUTTON    6
#define _GETB_RRIGHT_BUTTON   7
#define _GETB_ROTATE_BUTTON   8
#define _GETB_ULEFT_BUTTON    9
#define _GETB_UP_BUTTON       10
#define _GETB_URIGHT_BUTTON   11
#define _GETB_LEFT_BUTTON     12
#define _GETB_RIGHT_BUTTON    13
#define _GETB_DLEFT_BUTTON    14
#define _GETB_DOWN_BUTTON     15
#define _GETB_DRIGHT_BUTTON   16
#define _GETB_GLYPH_IMAGE     17

/**************************************************************************
 *
 * Callback values for each button.
 *
 **************************************************************************/

static int cback_values[] = {
    XmuttGLYPHEDIT_DRAW,
    XmuttGLYPHEDIT_MOVE,
    XmuttGLYPHEDIT_COPY,
    XmuttGLYPHEDIT_FLIP_HORIZ,
    XmuttGLYPHEDIT_FLIP_VERT,
    XmuttGLYPHEDIT_SHEAR,
    XmuttGLYPHEDIT_ROT_LEFT,
    XmuttGLYPHEDIT_ROT_RIGHT,
    XmuttGLYPHEDIT_ROTATE,
    XmuttGLYPHEDIT_SHIFT_ULEFT,
    XmuttGLYPHEDIT_SHIFT_UP,
    XmuttGLYPHEDIT_SHIFT_URIGHT,
    XmuttGLYPHEDIT_SHIFT_LEFT,
    XmuttGLYPHEDIT_SHIFT_RIGHT,
    XmuttGLYPHEDIT_SHIFT_DLEFT,
    XmuttGLYPHEDIT_SHIFT_DOWN,
    XmuttGLYPHEDIT_SHIFT_DRIGHT,
};

/**************************************************************************
 *
 * Help strings for context sensitive help.
 *
 **************************************************************************/

static String help_strings[18] = {
    "Draw",
    "Move",
    "Copy",
    "Flip Horizontally",
    "Flip Vertically",
    "Shear ±45°",
    "Rotate -90°",
    "Rotate +90°",
    "Rotate ±359°",
    "Shift Up+Left",
    "Shift Up",
    "Shift Up+Right",
    "Shift Left",
    "Shift Right",
    "Shift Down+Left",
    "Shift Down",
    "Shift Down+Right",
    "Glyph Image",
};

/**************************************************************************
 *
 * Utility functions.
 *
 **************************************************************************/

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxCheckClass(Widget w)
#else
_XmuttGEditTBoxCheckClass(w)
Widget w;
#endif
{
    if (XtIsSubclass(w, xmuttGlyphEditToolboxWidgetClass) == False) {
        fprintf(stderr,
                "XmuttGlyphEditToolbox: %s not an XmuttGlyphEditToolboxWidget.\n",
                XtName(w));
        exit(1);
    }
}

static Dimension
#ifndef _NO_PROTO
_XmuttGEditTBoxPMargins(XmuttGlyphEditToolboxWidget gw)
#else
_XmuttGEditTBoxPMargins(gw)
XmuttGlyphEditToolboxWidget gw;
#endif
{
    return (gw->primitive.shadow_thickness << 2) +
        (gw->primitive.highlight_thickness << 1);
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxPreferredGeometry(XmuttGlyphEditToolboxWidget gw,
                                 Dimension *width, Dimension *height)
#else
_XmuttGEditTBoxPreferredGeometry(gw, width, height)
XmuttGlyphEditToolboxWidget gw;
Dimension *width, *height;
#endif
{
    Dimension margin;
    bdf_bitmap_t *im;

    margin = _XmuttGEditTBoxPMargins(gw) + 4;

    *width = 50 + margin + (3 * (_GETB_TOGGLE_SIZE + 4)) + 4;
    *height = margin + (_GETB_TOGGLE_SIZE + 6) +
        ((5 * _GETB_BUTTON_SIZE) + 8);

    /*
     * If a glyph image was provided, adjust the height.  The width is highly
     * unlikely to be greater than the width of the toolbox buttons.
     */
    if ((im = gw->gedittb.gimage) != 0)
      *height += im->height + 6;
}

/*
 * Change window highlighting when focus is gained or lost.
 */
static void
#ifndef _NO_PROTO
_XmuttGEditTBoxHighlightWindow(Widget w)
#else
_XmuttGEditTBoxHighlightWindow(w)
Widget w;
#endif
{
    int i;
    XmuttGlyphEditToolboxWidget gw;
    XRectangle hi[4];

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditToolboxWidget) w;

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

    if (gw->gedittb.hasfocus == True)
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
_XmuttGEditTBoxShadowWindow(Widget w)
#else
_XmuttGEditTBoxShadowWindow(w)
Widget w;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    int i;
    XPoint br[3], tl[3];

    gw = (XmuttGlyphEditToolboxWidget) w;

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
_XmuttGEditTBoxMakeBitmaps(Widget w)
#else
_XmuttGEditTBoxMakeBitmaps(w)
Widget w;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    XmuttGlyphEditToolboxWidgetClass gwc;
    _XmuttGEditTBoxButton *bp;

    gw = (XmuttGlyphEditToolboxWidget) w;
    gwc = (XmuttGlyphEditToolboxWidgetClass) XtClass(w);

    /*
     * Only create the bitmaps if this is the first instance of this
     * kind of widget.
     */
    if (gwc->gedittb_class.refcnt != 1)
      return;

    bp = gw->gedittb.buttons;

    gwc->gedittb_class.draw = bp[_GETB_DRAW_TOGGLE].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), draw_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.move = bp[_GETB_MOVE_TOGGLE].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), move_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.copy = bp[_GETB_COPY_TOGGLE].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), copy_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.fliph = bp[_GETB_FLIPH_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), fliph_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.flipv = bp[_GETB_FLIPV_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), flipv_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.shear = bp[_GETB_SHEAR_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), shear_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.rleft = bp[_GETB_RLEFT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), rleft_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.rright = bp[_GETB_RRIGHT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), rright_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.rotate = bp[_GETB_ROTATE_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), rotate_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.uleft = bp[_GETB_ULEFT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), uleft_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.up = bp[_GETB_UP_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), up_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.uright = bp[_GETB_URIGHT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), uright_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.left = bp[_GETB_LEFT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), left_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.right = bp[_GETB_RIGHT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), right_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.dleft = bp[_GETB_DLEFT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), dleft_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.down = bp[_GETB_DOWN_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), down_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
    gwc->gedittb_class.dright = bp[_GETB_DRIGHT_BUTTON].pixmap =
        XCreateBitmapFromData(XtDisplay(w), XtWindow(w), dright_bits,
                              _GETB_WIDTH, _GETB_HEIGHT);
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxAssignBitmaps(Widget w)
#else
_XmuttGEditTBoxAssignBitmaps(w)
Widget w;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    XmuttGlyphEditToolboxWidgetClass gwc;
    _XmuttGEditTBoxButton *bp;

    gw = (XmuttGlyphEditToolboxWidget) w;
    gwc = (XmuttGlyphEditToolboxWidgetClass) XtClass(w);

    bp = gw->gedittb.buttons;

    /*
     * Assign the pixmaps.
     */
    bp[_GETB_DRAW_TOGGLE].pixmap = gwc->gedittb_class.draw;
    bp[_GETB_MOVE_TOGGLE].pixmap = gwc->gedittb_class.move;
    bp[_GETB_COPY_TOGGLE].pixmap = gwc->gedittb_class.copy;
    bp[_GETB_FLIPH_BUTTON].pixmap = gwc->gedittb_class.fliph;
    bp[_GETB_FLIPV_BUTTON].pixmap = gwc->gedittb_class.flipv;
    bp[_GETB_SHEAR_BUTTON].pixmap = gwc->gedittb_class.shear;
    bp[_GETB_RLEFT_BUTTON].pixmap = gwc->gedittb_class.rleft;
    bp[_GETB_RRIGHT_BUTTON].pixmap = gwc->gedittb_class.rright;
    bp[_GETB_ROTATE_BUTTON].pixmap = gwc->gedittb_class.rotate;
    bp[_GETB_ULEFT_BUTTON].pixmap = gwc->gedittb_class.uleft;
    bp[_GETB_UP_BUTTON].pixmap = gwc->gedittb_class.up;
    bp[_GETB_URIGHT_BUTTON].pixmap = gwc->gedittb_class.uright;
    bp[_GETB_LEFT_BUTTON].pixmap = gwc->gedittb_class.left;
    bp[_GETB_RIGHT_BUTTON].pixmap = gwc->gedittb_class.right;
    bp[_GETB_DLEFT_BUTTON].pixmap = gwc->gedittb_class.dleft;
    bp[_GETB_DOWN_BUTTON].pixmap = gwc->gedittb_class.down;
    bp[_GETB_DRIGHT_BUTTON].pixmap = gwc->gedittb_class.dright;
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxMakeGCs(Widget w, Boolean force)
#else
_XmuttGEditTBoxMakeGCs(w, force)
Widget w;
Boolean force;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    XtGCMask gcm;
    XGCValues gcv;

    gw = (XmuttGlyphEditToolboxWidget) w;

    gcm = GCForeground|GCBackground|GCFunction;

    if (gw->gedittb.gc == 0 || force == True) {
        if (gw->gedittb.gc != 0)
          XFreeGC(XtDisplay(w), gw->gedittb.gc);
        gcv.foreground = gw->primitive.foreground;
        gcv.background = gw->core.background_pixel;
        gcv.function = GXcopy;
        gw->gedittb.gc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }

    if (gw->gedittb.fillgc == 0 || force == True) {
        if (gw->gedittb.fillgc != 0)
          XFreeGC(XtDisplay(w), gw->gedittb.fillgc);
        gcv.foreground = gw->core.background_pixel;
        gcv.background = gw->primitive.foreground;
        gcv.function = GXcopy;
        gw->gedittb.fillgc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }

    if (gw->gedittb.armgc == 0 || force == True) {
        if (gw->gedittb.armgc != 0)
          XFreeGC(XtDisplay(w), gw->gedittb.armgc);
        gcv.foreground = gw->primitive.foreground;
        gcv.background = gw->gedittb.arm_color;
        gcv.function = GXcopy;
        gw->gedittb.armgc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }

    if (gw->gedittb.armfillgc == 0 || force == True) {
        if (gw->gedittb.armfillgc != 0)
          XFreeGC(XtDisplay(w), gw->gedittb.armfillgc);
        gcv.foreground = gw->gedittb.arm_color;
        gcv.background = gw->primitive.foreground;
        gcv.function = GXcopy;
        gw->gedittb.armfillgc =
            XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }

    if (gw->gedittb.tsgc == 0 || force == True) {
        if (gw->gedittb.tsgc != 0)
          XFreeGC(XtDisplay(w), gw->gedittb.tsgc);
        gw->gedittb.tsgc =
            XCreateGC(XtDisplay(w), XtWindow(w), 0, &gcv);
        XCopyGC(XtDisplay(w), gw->primitive.top_shadow_GC, gcm,
                gw->gedittb.tsgc);
    }
    if (gw->gedittb.bsgc == 0 || force == True) {
        if (gw->gedittb.bsgc != 0)
          XFreeGC(XtDisplay(w), gw->gedittb.bsgc);
        gw->gedittb.bsgc =
            XCreateGC(XtDisplay(w), XtWindow(w), 0, &gcv);
        XCopyGC(XtDisplay(w), gw->primitive.bottom_shadow_GC, gcm,
                gw->gedittb.bsgc);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxDrawColors(Widget w)
#else
_XmuttGEditTBoxDrawColors(w)
Widget w;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    int i, s, e;
    _XmuttGEditTBoxColor *cp;
    XRectangle cr;

    gw = (XmuttGlyphEditToolboxWidget) w;

    if (gw->gedittb.colors == 0 || gw->gedittb.gimage->bpp == 1)
      return;

    if ((gw->gedittb.gimage->bpp == 2 && gw->gedittb.colors[0] == ~0) ||
        (gw->gedittb.gimage->bpp == 4 && gw->gedittb.colors[4] == ~0))
      return;

    if (gw->gedittb.gimage->bpp == 2) {
        s = 0;
        e = 4;
    } else {
        s = 4;
        e = 20;
    }

    cp = gw->gedittb.cols;

    cr.width = gw->gedittb.cbw;
    cr.height = gw->gedittb.cbh;

    for (i = s; i < e; i++, cp++) {
        cr.x = cp->x;
        cr.y = cp->y;
        /*
         * If this is the current color index, put a box around it.
         */
        if (i - s == gw->gedittb.cidx)
          XDrawRectangle(XtDisplay(w), XtWindow(w), gw->gedittb.gc,
                         cr.x - 1, cr.y - 1, cr.width + 3, cr.height + 3);
        XSetForeground(XtDisplay(w), gw->gedittb.fillgc,
                       gw->gedittb.colors[i]);
        XFillRectangle(XtDisplay(w), XtWindow(w), gw->gedittb.fillgc,
                       cr.x + 1, cr.y + 1, cr.width, cr.height);
    }
    XSetForeground(XtDisplay(w), gw->gedittb.fillgc,
                   gw->core.background_pixel);
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxSelectColor(Widget w, int which)
#else
_XmuttGEditTBoxSelectColor(w, which)
Widget w;
int which;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    XRectangle rect;

    gw = (XmuttGlyphEditToolboxWidget) w;

    /*
     * Locate the current color and clear the rectangle.
     */
    rect.x = gw->gedittb.cols[gw->gedittb.cidx].x - 1;
    rect.y = gw->gedittb.cols[gw->gedittb.cidx].y - 1;
    rect.width = gw->gedittb.cbw + 3;
    rect.height = gw->gedittb.cbh + 3;
    if (XtIsRealized(w)) {
        XSetForeground(XtDisplay(w), gw->gedittb.gc,
                       gw->core.background_pixel);
        XDrawRectangle(XtDisplay(w), XtWindow(w), gw->gedittb.gc,
                       rect.x, rect.y, rect.width, rect.height);
    }

    /*
     * Locate the new color and draw the rectangle.
     */
    gw->gedittb.cidx = which;
    rect.y = gw->gedittb.cols[gw->gedittb.cidx].y - 1;
    if (XtIsRealized(w)) {
        XSetForeground(XtDisplay(w), gw->gedittb.gc, gw->primitive.foreground);
        XDrawRectangle(XtDisplay(w), XtWindow(w), gw->gedittb.gc,
                       rect.x, rect.y, rect.width, rect.height);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxGetPixels(XmuttGlyphEditToolboxWidget gw, bdf_bitmap_t *im,
                       int color)
#else
_XmuttGEditTBoxGetPixels(gw, im, color)
XmuttGlyphEditToolboxWidget gw;
bdf_bitmap_t *im;
int color;
#endif
{
    int byte;
    unsigned short x, y, bpr, si, di, nx;
    unsigned char *masks;

    gw->gedittb.points_used = 0;

    di = 0;
    masks = 0;
    switch (im->bpp) {
      case 1: masks = onebpp; di = 7; break;
      case 2: masks = twobpp; di = 3; break;
      case 4: masks = fourbpp; di = 1; break;
    }

    bpr = ((im->width * im->bpp) + 7) >> 3;
    for (y = 0; y < im->height; y++) {
        for (nx = x = 0; x < im->width; x++, nx += im->bpp) {
            si = (nx & 7) / im->bpp;

            byte = im->bitmap[(y * bpr) + (nx >> 3)] & masks[si];
            if (di > si)
              byte >>= (di - si) * im->bpp;
            if (byte == color) {
                if (gw->gedittb.points_used == gw->gedittb.points_size) {
                    if (gw->gedittb.points_size == 0)
                      gw->gedittb.points =
                          (XPoint *) XtMalloc(sizeof(XPoint) * 64);
                    else
                      gw->gedittb.points = (XPoint *)
                          XtRealloc((char *) gw->gedittb.points,
                                    sizeof(XPoint) *
                                    (gw->gedittb.points_size + 64));;
                    gw->gedittb.points_size += 64;
                }
                gw->gedittb.points[gw->gedittb.points_used].x = x + im->x + 2;
                gw->gedittb.points[gw->gedittb.points_used].y = y + im->y + 2;
                gw->gedittb.points_used++;
            }
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxDrawImage(Widget w)
#else
_XmuttGEditTBoxDrawImage(w)
Widget w;
#endif
{
    int color;
    short si, ei;
    XmuttGlyphEditToolboxWidget gw;
    bdf_bitmap_t *im;

    gw = (XmuttGlyphEditToolboxWidget) w;

    if (!XtIsRealized(w) || (im = gw->gedittb.gimage) == 0)
      return;

    si = ei = 0;
    switch (im->bpp) {
      case 1: si = 0; ei = 2; break;
      case 2: si = 0; ei = 4; break;
      case 4: si = 4; ei = 20; break;
    }

    /*
     * Draw a rectangle around the image area.
     */
    XDrawRectangle(XtDisplay(w), XtWindow(w), gw->gedittb.gc, im->x, im->y,
                   im->width + 3, im->height + 3);

    /*
     * Go through and set all of the pixels with the correct colors
     * for the image.
     */
    for (color = si; color < ei; color++) {
        _XmuttGEditTBoxGetPixels(gw, im, color);
        if (gw->gedittb.points_used > 0) {
            if (im->bpp == 1) {
                if (color)
                  XSetForeground(XtDisplay(w), gw->gedittb.gc,
                                 gw->primitive.foreground);
                else
                  XSetForeground(XtDisplay(w), gw->gedittb.gc,
                                 gw->core.background_pixel);
            } else
              XSetForeground(XtDisplay(w), gw->gedittb.gc,
                             gw->gedittb.colors[color - si]);
            XDrawPoints(XtDisplay(w), XtWindow(w), gw->gedittb.gc,
                        gw->gedittb.points, gw->gedittb.points_used,
                        CoordModeOrigin);
        }
    }
    XSetForeground(XtDisplay(w), gw->gedittb.gc,
                   gw->primitive.foreground);

}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxDrawToggle(Widget w, int which)
#else
_XmuttGEditTBoxDrawToggle(w, which)
Widget w;
int which;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    GC tgc, bgc, fgc, dgc;
    int i;
    _XmuttGEditTBoxButton *bp;
    XPoint toggle[6];

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditToolboxWidget) w;
    bp = gw->gedittb.buttons;

    /*
     * Choose the appropriate GCs.
     */
    if (bp[which].set == True) {
        tgc = gw->gedittb.bsgc;
        bgc = gw->gedittb.tsgc;
        fgc = gw->gedittb.armfillgc;
        dgc = gw->gedittb.armgc;
    } else {
        tgc = gw->gedittb.tsgc;
        bgc = gw->gedittb.bsgc;
        fgc = gw->gedittb.fillgc;
        dgc = gw->gedittb.gc;
    }

    /*
     * Set up the polygon for clearing the background first.
     */
    toggle[0].x = bp[which].x;
    toggle[0].y = bp[which].y;

    toggle[1].x = toggle[0].x - ((_GETB_TOGGLE_SIZE >> 1) + 2);
    toggle[1].y = toggle[0].y + ((_GETB_TOGGLE_SIZE >> 1) + 2);

    toggle[2].x = toggle[0].x - 1;
    toggle[2].y = toggle[1].y + (_GETB_TOGGLE_SIZE >> 1) + 1;

    toggle[3].x = toggle[0].x + (_GETB_TOGGLE_SIZE >> 1) + 2;
    toggle[3].y = toggle[1].y;

    toggle[4].x = toggle[0].x;
    toggle[4].y = toggle[0].y;

    /*
     * Clear the foreground.
     */
    XFillPolygon(XtDisplay(w), XtWindow(w), fgc, toggle, 5, Convex,
                 CoordModeOrigin);

    /*
     * Reset the extra points for drawing the polygon shadow.
     */
    toggle[3].x = toggle[0].x + 1;
    toggle[3].y = toggle[0].y + 1;

    toggle[4].x = toggle[0].x + (_GETB_TOGGLE_SIZE >> 1) + 2;
    toggle[4].y = toggle[1].y;

    toggle[5].x = toggle[2].x + 1;
    toggle[5].y = toggle[2].y + 1;

    XDrawLines(XtDisplay(w), XtWindow(w), tgc, toggle, 3, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), bgc, &toggle[3], 3, CoordModeOrigin);

    for (i = 1; i < 2; i++) {
        toggle[0].y++;
        toggle[1].x++;
        toggle[2].y--;
        toggle[3].y++;
        toggle[4].x--;
        toggle[5].y--;
        XDrawLines(XtDisplay(w), XtWindow(w), tgc, toggle, 3, CoordModeOrigin);
        XDrawLines(XtDisplay(w), XtWindow(w), bgc, &toggle[3], 3,
                   CoordModeOrigin);
    }

    /*
     * Copy the bitmap onto the polygon.
     */
    XCopyPlane(XtDisplay(w), bp[which].pixmap, XtWindow(w), dgc,
               0, 0, _GETB_WIDTH, _GETB_HEIGHT,
               bp[which].x - 7, bp[which].y + 12, 1);

}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxDrawButton(Widget w, int which)
#else
_XmuttGEditTBoxDrawButton(w, which)
Widget w;
int which;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    GC tgc, bgc, fgc, dgc;
    int i;
    _XmuttGEditTBoxButton *bp;
    XRectangle clear;
    XPoint br[3], tl[3];

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphEditToolboxWidget) w;
    bp = gw->gedittb.buttons;

    clear.x = bp[which].x + 2;
    clear.y = bp[which].y + 2;
    clear.width = clear.height = _GETB_BUTTON_SIZE - 2;

    /*
     * Set up the appropriate GC's.
     */
    if (bp[which].set == True) {
        tgc = gw->gedittb.bsgc;
        bgc = gw->gedittb.tsgc;
        fgc = gw->gedittb.armfillgc;
        dgc = gw->gedittb.armgc;
    } else {
        tgc = gw->gedittb.tsgc;
        bgc = gw->gedittb.bsgc;
        fgc = gw->gedittb.fillgc;
        dgc = gw->gedittb.gc;
    }

    XFillRectangle(XtDisplay(w), XtWindow(w), fgc,
                   clear.x, clear.y, clear.width, clear.height);
    XCopyPlane(XtDisplay(w), bp[which].pixmap, XtWindow(w), dgc,
               0, 0, _GETB_WIDTH, _GETB_HEIGHT,
               bp[which].x + ((_GETB_BUTTON_SIZE >> 1) - (_GETB_WIDTH >> 1)),
               bp[which].y + ((_GETB_BUTTON_SIZE >> 1) - (_GETB_HEIGHT >> 1)),
               1);

    /*
     * Lower left corner outside line.
     */
    tl[0].x = bp[which].x;
    tl[0].y = bp[which].y + (_GETB_BUTTON_SIZE - 1);

    /*
     * Upper left corner outside line.
     */
    tl[1].x = tl[0].x;
    tl[1].y = bp[which].y;

    /*
     * Upper right corner outside line.
     */
    tl[2].x = tl[0].x + _GETB_BUTTON_SIZE;
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
    br[1].y = tl[0].y;

    /*
     * Lower left corner outside line.
     */
    br[2].x = tl[0].x + 1;
    br[2].y = br[1].y;

    XDrawLines(XtDisplay(w), XtWindow(w), tgc, tl, 3, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), bgc, br, 3, CoordModeOrigin);

    /*
     * Draw the remaining shadows successively inward.
     */
    for (i = 1; i < 2; i++) {
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
        XDrawLines(XtDisplay(w), XtWindow(w), tgc, tl, 3, CoordModeOrigin);
        XDrawLines(XtDisplay(w), XtWindow(w), bgc, br, 3, CoordModeOrigin);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxPosition(XmuttGlyphEditToolboxWidget gw)
#else
_XmuttGEditTBoxPosition(gw)
XmuttGlyphEditToolboxWidget gw;
#endif
{
    XmuttGlyphEditToolboxWidgetClass gwc;
    int i, j, x, y, dx, dy;
    Dimension cp;
    _XmuttGEditTBoxButton *bp;
    _XmuttGEditTBoxColor *cl;
    bdf_bitmap_t *im;
    XRectangle button;
    XPoint toggle[5];

    gwc = (XmuttGlyphEditToolboxWidgetClass) XtClass((Widget) gw);

    /*
     * Set up the initial polygon for creating toggle regions.
     */
    toggle[0].x = ((_GETB_TOGGLE_SIZE >> 1) + 2);
    toggle[0].y = 0;

    toggle[1].x = toggle[0].x - ((_GETB_TOGGLE_SIZE >> 1) + 2);
    toggle[1].y = toggle[0].y + ((_GETB_TOGGLE_SIZE >> 1) + 2);

    toggle[2].x = toggle[0].x - 1;
    toggle[2].y = toggle[1].y + (_GETB_TOGGLE_SIZE >> 1) + 1;

    toggle[3].x = toggle[0].x + (_GETB_TOGGLE_SIZE >> 1) + 2;
    toggle[3].y = toggle[1].y;

    toggle[4].x = toggle[0].x;
    toggle[4].y = toggle[0].y;

    bp = gw->gedittb.buttons;

    y = (_XmuttGEditTBoxPMargins(gw) >> 1) + 2;

    cp = x = gw->core.width >> 1;

    /*
     * Position the top-left coordinates of the glyph image if it exists.
     */
    if ((im = gw->gedittb.gimage) != 0) {
        dx = im->x;
        dy = im->y;
        im->x = cp - ((im->width + 1) >> 1);
        im->y = y;

        if (gw->gedittb.image_region == 0) {
            gw->gedittb.image_region = XCreateRegion();
            button.x = im->x;
            button.y = im->y;
            button.width = im->width + 4;
            button.height = im->height + 4;
            XUnionRectWithRegion(&button, gw->gedittb.image_region,
                                 gw->gedittb.image_region);
        } else {
            dx = im->x - dx;
            dy = im->y - dy;
            XOffsetRegion(gw->gedittb.image_region, dx, dy);
        }

        y += im->height + 6;
    }

    bp[_GETB_DRAW_TOGGLE].x = cp - (_GETB_TOGGLE_SIZE + 6);
    bp[_GETB_DRAW_TOGGLE].y = y;
    dx = bp[_GETB_DRAW_TOGGLE].x - toggle[0].x;
    dy = y;

    /*
     * Position the toggle polygon at the beginning.
     */
    for (i = 0; i < 5; i++) {
        toggle[i].x += dx;
        toggle[i].y += dy;
    }

    for (i = _GETB_DRAW_TOGGLE; i <= _GETB_COPY_TOGGLE; i++) {
        if (bp[i].region != 0)
          XDestroyRegion(bp[i].region);
        bp[i].region = XPolygonRegion(toggle, 5, WindingRule);

        if (i > _GETB_DRAW_TOGGLE) {
            bp[i].x = bp[i - 1].x + (_GETB_TOGGLE_SIZE + 4) + 2;
            bp[i].y = y;
        }
        for (j = 0; j < 5; j++)
          toggle[j].x += _GETB_TOGGLE_SIZE + 6;
    }
    y += _GETB_TOGGLE_SIZE + 7;

    /*
     * Set up the initial rectangle for creating button regions.
     */
    button.x = button.y = 0;
    button.width = button.height = _GETB_BUTTON_SIZE;

    /*
     * Reset the starting X coordinate for layout of the three-button rows.
     */
    x = cp - ((_GETB_BUTTON_SIZE >> 1) + _GETB_BUTTON_SIZE + 2);

    for (i = _GETB_FLIPH_BUTTON; i <= _GETB_DRIGHT_BUTTON; i++) {
        if (bp[i].region == 0) {
            bp[i].region = XCreateRegion();
            XUnionRectWithRegion(&button, bp[i].region, bp[i].region);
        }
        switch (i) {
          case _GETB_FLIPH_BUTTON:
          case _GETB_FLIPV_BUTTON:
          case _GETB_RLEFT_BUTTON:
          case _GETB_RRIGHT_BUTTON:
          case _GETB_ULEFT_BUTTON:
          case _GETB_UP_BUTTON:
          case _GETB_LEFT_BUTTON:
          case _GETB_DLEFT_BUTTON:
          case _GETB_DOWN_BUTTON:
            dx = bp[i].x;
            dy = bp[i].y;
            bp[i].x = x;
            bp[i].y = y;
            dx = bp[i].x - dx;
            dy = bp[i].y - dy;
            XOffsetRegion(bp[i].region, dx, dy);
            x += _GETB_BUTTON_SIZE + 2;
            break;
          case _GETB_RIGHT_BUTTON:
            /*
             * Adjust the X offset by the width of one
             * button and drop through.
             */
            x += _GETB_BUTTON_SIZE + 2;
          case _GETB_SHEAR_BUTTON:
          case _GETB_ROTATE_BUTTON:
          case _GETB_URIGHT_BUTTON:
          case _GETB_DRIGHT_BUTTON:
            dx = bp[i].x;
            dy = bp[i].y;
            bp[i].x = x;
            bp[i].y = y;
            dx = bp[i].x - dx;
            dy = bp[i].y - dy;
            XOffsetRegion(bp[i].region, dx, dy);
            /*
             * Reset the X coordinate and move to the next row.
             */
            x = cp - ((_GETB_BUTTON_SIZE >> 1) + _GETB_BUTTON_SIZE + 2);
            y += _GETB_BUTTON_SIZE + 2;
            break;
        }
    }

    /*
     * Set up the color regions starting with the two
     */
    cl = gw->gedittb.cols;
    button.width = gw->gedittb.cbw;
    button.height = gw->gedittb.cbh;
    for (i = 0; i < 16; i++) {
        if (!i) {
            button.x = cl[i].x = (_XmuttGEditTBoxPMargins(gw) >> 1) + 2;
            button.y = cl[i].y = bp[_GETB_DRAW_TOGGLE].y + 2;
        } else {
            cl[i].x = button.x;
            button.y = cl[i].y = cl[i - 1].y + gw->gedittb.cbh + 3;
        }

        if (cl[i].region == 0) {
            cl[i].region = XCreateRegion();
            XUnionRectWithRegion(&button, cl[i].region, cl[i].region);
        } else {
            dy = cl[i].y;
            cl[i].y = button.y;
            if ((dy = cl[i].y - dy) != 0)
              XOffsetRegion(cl[i].region, cl[i].x, dy);
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttGEditTBoxSetClip(Widget w)
#else
_XmuttGEditTBoxSetClip(w)
Widget w;
#endif
{
    XmuttGlyphEditToolboxWidget gw = (XmuttGlyphEditToolboxWidget) w;
    XRectangle clip;

    clip.x = clip.y = (_XmuttGEditTBoxPMargins(gw) >> 1);
    clip.width = gw->core.width - (clip.x << 1);
    clip.height = gw->core.height - (clip.y << 1);

    XSetClipRectangles(XtDisplay(w), gw->gedittb.gc, 0, 0, &clip, 1, YSorted);
    XSetClipRectangles(XtDisplay(w), gw->gedittb.fillgc,
                       0, 0, &clip, 1, YSorted);
    XSetClipRectangles(XtDisplay(w), gw->gedittb.armgc,
                       0, 0, &clip, 1, YSorted);
    XSetClipRectangles(XtDisplay(w), gw->gedittb.armfillgc,
                       0, 0, &clip, 1, YSorted);
    XSetClipRectangles(XtDisplay(w), gw->gedittb.tsgc,
                       0, 0, &clip, 1, YSorted);
    XSetClipRectangles(XtDisplay(w), gw->gedittb.bsgc,
                       0, 0, &clip, 1, YSorted);
}

static int
#ifndef _NO_PROTO
_XmuttGEditTBoxXYToButton(XmuttGlyphEditToolboxWidget gw, int x, int y)
#else
_XmuttGEditTBoxXYToButton(gw, x, y)
XmuttGlyphEditToolboxWidget gw;
int x, y;
#endif
{
    int i, which, top;
    _XmuttGEditTBoxButton *bp;
    _XmuttGEditTBoxColor *cp;
    
    bp = gw->gedittb.buttons;

    if (gw->gedittb.image_region != 0 &&
        XPointInRegion(gw->gedittb.image_region, x, y))
      return _GETB_GLYPH_IMAGE;

    for (i = _GETB_DRAW_TOGGLE, which = -1;
         which == -1 && i <= _GETB_DRIGHT_BUTTON; i++) {
        if (XPointInRegion(bp[i].region, x, y) == True)
          which = i;
    }

    /*
     * Check to see if the point is on one of the colors.
     */
    top = 0;
    if (gw->gedittb.gimage && gw->gedittb.gimage->bpp > 1)
      top = 1 << gw->gedittb.gimage->bpp;
    cp = gw->gedittb.cols;
    for (i = 0; which == -1 && i < top; i++) {
        if (XPointInRegion(cp[i].region, x, y) == True)
          which = i + _GETB_GLYPH_IMAGE + 1;
    }

    return which;
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
    XmuttGlyphEditToolboxWidget gw;
    XmuttGlyphEditToolboxWidgetClass gwc;
    Screen *screen;
    Dimension wd, ht;
    Pixel unused;
    int i;
    _XmuttGEditTBoxButton *bp;
    bdf_bitmap_t *gimage;

    gw = (XmuttGlyphEditToolboxWidget) newone;
    gwc = (XmuttGlyphEditToolboxWidgetClass) XtClass(newone);

    gwc->gedittb_class.refcnt++;

    /*
     * Set the color spot size.
     */
    gw->gedittb.cbw = 20;
    gw->gedittb.cbh = 10;

    /*
     * Get the color used to arm the buttons.
     */
    screen = XtScreen(newone);
    XmGetColors(screen, XDefaultColormapOfScreen(screen),
                gw->core.background_pixel, &unused, &unused, &unused,
                &gw->gedittb.arm_color);

    /*
     * Initialize the current color index.
     */
    if (gw->gedittb.gimage) {
        if ((gw->gedittb.cidx = (1 << gw->gedittb.gimage->bpp) - 1) == 0)
          gw->gedittb.cidx = -1;
    } else
      gw->gedittb.cidx = -1;

    /*
     * Initialize the points list.
     */
    gw->gedittb.points_size = gw->gedittb.points_used = 0;

    /*
     * Intialize the timer information.
     */
    gw->gedittb.timer = 0;
    gw->gedittb.timer_button = gw->gedittb.timer_count = 0;

    /*
     * Initialize the booleans.
     */
    gw->gedittb.hasfocus = False;

    /*
     * Initialize the GC's.
     */
    gw->gedittb.gc = gw->gedittb.fillgc =
        gw->gedittb.armgc = gw->gedittb.armfillgc =
        gw->gedittb.tsgc = gw->gedittb.bsgc = 0;

    /*
     * Initialize the fields that track the buttons pressed and moved over.
     */
    gw->gedittb.pressed = gw->gedittb.last = -1;

    /*
     * Initialize the button structures.
     */
    for (bp = gw->gedittb.buttons, i = _GETB_DRAW_TOGGLE;
         i <= _GETB_DRIGHT_BUTTON; i++, bp++) {
        /*
         * Assign the context sensitive help string.
         */
        bp->help = help_strings[i];

        /*
         * Initialize the region.
         */
        bp->region = 0;

        /*
         * Set the initial coordinates to 0, 0.
         */
        bp->x = bp->y = 0;

        /*
         * The Draw button will always be initialized as being set the first
         * time.
         */
        bp->set = (i == _GETB_DRAW_TOGGLE) ? True : False;

        switch (i) {
          case _GETB_DRAW_TOGGLE:
            bp->toggle = True;
            bp->other_toggles[0] = _GETB_MOVE_TOGGLE;
            bp->other_toggles[1] = _GETB_COPY_TOGGLE;
            break;
          case _GETB_MOVE_TOGGLE:
            bp->toggle = True;
            bp->other_toggles[0] = _GETB_DRAW_TOGGLE;
            bp->other_toggles[1] = _GETB_COPY_TOGGLE;
            break;
          case _GETB_COPY_TOGGLE:
            bp->toggle = True;
            bp->other_toggles[0] = _GETB_DRAW_TOGGLE;
            bp->other_toggles[1] = _GETB_MOVE_TOGGLE;
            break;
          default:
            bp->toggle = False;
            break;
        }
    }

    /*
     * Add the glyph image help message.
     */
    bp->help = help_strings[_GETB_GLYPH_IMAGE];

    /*
     * Initialize the color structures.
     */
    for (i = 0; i < 16; i++) {
        gw->gedittb.cols[i].x = gw->gedittb.cols[i].y = 0;
        gw->gedittb.cols[i].region = 0;
    }

    /*
     * Initialize the glyph image region.
     */
    gw->gedittb.image_region = 0;
    if (gw->gedittb.gimage != 0) {
        /*
         * Copy the image.
         */
        gimage = (bdf_bitmap_t *) XtMalloc(sizeof(bdf_bitmap_t));
        gimage->bpp = gw->gedittb.gimage->bpp;
        gimage->x = gimage->y = 0;
        gimage->width = gw->gedittb.gimage->width;
        gimage->height = gw->gedittb.gimage->height;
        gimage->bytes = gw->gedittb.gimage->bytes;
        gimage->bitmap = (unsigned char *) XtMalloc(gimage->bytes);
        (void) memcpy((char *) gimage->bitmap, gw->gedittb.gimage->bitmap,
                      gimage->bytes);

        gw->gedittb.gimage = gimage;
    }

    /*
     * Get the initial geometry.
     */
    _XmuttGEditTBoxPreferredGeometry(gw, &wd, &ht);
    gw->core.width = wd;
    gw->core.height = ht;

    /*
     * Position the buttons.
     */
    _XmuttGEditTBoxPosition(gw);
}

static void
#ifndef _NO_PROTO
Resize(Widget w)
#else
Resize(w)
Widget w;
#endif
{
    if (XtIsRealized(w))
      _XmuttGEditTBoxSetClip(w);

    _XmuttGEditTBoxPosition((XmuttGlyphEditToolboxWidget) w);
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

    _XmuttGEditTBoxMakeBitmaps(w);
    _XmuttGEditTBoxAssignBitmaps(w);
    _XmuttGEditTBoxMakeGCs(w, False);
    _XmuttGEditTBoxSetClip(w);
}

static void
#ifndef _NO_PROTO
Destroy(Widget w)
#else
Destroy(w)
Widget w;
#endif
{
    int i;
    XmuttGlyphEditToolboxWidget gw;
    XmuttGlyphEditToolboxWidgetClass gwc;
    _XmuttGEditTBoxButton *bp;

    gw = (XmuttGlyphEditToolboxWidget) w;
    gwc = (XmuttGlyphEditToolboxWidgetClass) XtClass(w);

    /*
     * Destroy the GCs.
     */
    if (gw->gedittb.gc != 0)
      XFreeGC(XtDisplay(w), gw->gedittb.gc);
    if (gw->gedittb.fillgc != 0)
      XFreeGC(XtDisplay(w), gw->gedittb.fillgc);
    if (gw->gedittb.armgc != 0)
      XFreeGC(XtDisplay(w), gw->gedittb.armgc);
    if (gw->gedittb.armfillgc != 0)
      XFreeGC(XtDisplay(w), gw->gedittb.armfillgc);
    if (gw->gedittb.tsgc != 0)
      XFreeGC(XtDisplay(w), gw->gedittb.tsgc);
    if (gw->gedittb.bsgc != 0)
      XFreeGC(XtDisplay(w), gw->gedittb.bsgc);

    gw->gedittb.gc = gw->gedittb.fillgc =
        gw->gedittb.armgc = gw->gedittb.armfillgc =
        gw->gedittb.tsgc = gw->gedittb.bsgc = 0;

    /*
     * Destroy the regions.
     */
    if (gw->gedittb.image_region != 0)
      XDestroyRegion(gw->gedittb.image_region);
    gw->gedittb.image_region = 0;

    bp = gw->gedittb.buttons;
    for (i = _GETB_DRAW_TOGGLE; i <= _GETB_DRIGHT_BUTTON; i++) {
        if (bp[i].region != 0)
          XDestroyRegion(bp[i].region);
        bp[i].region = 0;
    }

    /*
     * Destroy the glyph image if it exists.
     */
    if (gw->gedittb.gimage != 0) {
        if (gw->gedittb.gimage->bytes > 0)
          XtFree((char *) gw->gedittb.gimage->bitmap);
        XtFree((char *) gw->gedittb.gimage);
    }
    gw->gedittb.gimage = 0;

    /*
     * Free the points if any have been allocated.
     */
    if (gw->gedittb.points_size > 0)
      XtFree((char *) gw->gedittb.points);
    gw->gedittb.points_used = gw->gedittb.points_size = 0;

    if (gwc->gedittb_class.refcnt > 0) {
        /*
         * Decrement the class reference count.
         */
        gwc->gedittb_class.refcnt--;

        /*
         * Destroy the pixmaps if this is the last instance of this kind of
         * widget.
         */
        if (gwc->gedittb_class.refcnt == 0) {
            if (gwc->gedittb_class.draw != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.draw);
            if (gwc->gedittb_class.move != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.move);
            if (gwc->gedittb_class.copy != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.copy);
            if (gwc->gedittb_class.fliph != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.fliph);
            if (gwc->gedittb_class.flipv != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.flipv);
            if (gwc->gedittb_class.shear != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.shear);
            if (gwc->gedittb_class.rleft != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.rleft);
            if (gwc->gedittb_class.rright != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.rright);
            if (gwc->gedittb_class.rotate != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.rotate);
            if (gwc->gedittb_class.uleft != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.uleft);
            if (gwc->gedittb_class.up != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.up);
            if (gwc->gedittb_class.uright != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.uright);
            if (gwc->gedittb_class.left != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.left);
            if (gwc->gedittb_class.right != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.right);
            if (gwc->gedittb_class.dleft != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.dleft);
            if (gwc->gedittb_class.down != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.down);
            if (gwc->gedittb_class.dright != 0)
              XFreePixmap(XtDisplay(w), gwc->gedittb_class.dright);
        }
    }
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
    int i;
    XmuttGlyphEditToolboxWidget gw;
    _XmuttGEditTBoxButton *bp;

    _XmuttGEditTBoxHighlightWindow(w);
    _XmuttGEditTBoxShadowWindow(w);

    gw = (XmuttGlyphEditToolboxWidget) w;
    bp = gw->gedittb.buttons;

    /*
     * Draw the glyph image if it exists.
     */
    _XmuttGEditTBoxDrawImage(w);

    for (i = _GETB_DRAW_TOGGLE; i <= _GETB_DRIGHT_BUTTON; i++) {
        if (bp[i].toggle == False)
          _XmuttGEditTBoxDrawButton(w, i);
        else
          _XmuttGEditTBoxDrawToggle(w, i);
    }

    _XmuttGEditTBoxDrawColors(w);
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
    XmuttGlyphEditToolboxWidget cgw, ngw;
    Screen *screen;
    Pixel unused;
    Dimension width, height;
    short ox, oy, owidth, oheight;
    bdf_bitmap_t *gimage;
    Boolean redo, repos;

    cgw = (XmuttGlyphEditToolboxWidget) curr;
    ngw = (XmuttGlyphEditToolboxWidget) newone;

    redo = repos = False;

    /*
     * Check for changes in the width and height.
     */
    if (ngw->core.width != cgw->core.width ||
        ngw->core.height != cgw->core.height)
        redo = repos = True;

    /*
     * Check if the GC's need to be changed.
     */
    if (ngw->primitive.foreground != cgw->primitive.foreground ||
        ngw->core.background_pixel != cgw->core.background_pixel) {
        /*
         * Get the new arm color.
         */
        screen = XtScreen(newone);
        XmGetColors(screen, XDefaultColormapOfScreen(screen),
                    ngw->core.background_pixel, &unused, &unused, &unused,
                    &ngw->gedittb.arm_color);
        _XmuttGEditTBoxMakeGCs(newone, True);
        redo = True;
    }

    /*
     * Check to see if the glyph image changed.
     */
    if (ngw->gedittb.gimage != cgw->gedittb.gimage) {
        repos = False;

        /*
         * The glyph image has changed.  Delete the old one, free the image
         * pixmap (if it has been created), and copy the new glyph image.
         */
        ox = oy = owidth = oheight = 0;
        if (cgw->gedittb.gimage != 0) {
            /*
             * Save the original width and height to determine if a reposition
             * needs to be done.
             */
            ox = cgw->gedittb.gimage->x;
            oy = cgw->gedittb.gimage->y;
            owidth = cgw->gedittb.gimage->width;
            oheight = cgw->gedittb.gimage->height;

            if (cgw->gedittb.gimage->bytes > 0)
              XtFree((char *) cgw->gedittb.gimage->bitmap);
            XtFree((char *) cgw->gedittb.gimage);
        }
        cgw->gedittb.gimage = 0;

        if (ngw->gedittb.gimage != 0) {
            /*
             * Copy the image.
             */
            gimage = (bdf_bitmap_t *) XtMalloc(sizeof(bdf_bitmap_t));
            gimage->bpp = ngw->gedittb.gimage->bpp;
            gimage->width = ngw->gedittb.gimage->width;
            gimage->height = ngw->gedittb.gimage->height;
            gimage->bytes = ngw->gedittb.gimage->bytes;
            gimage->bitmap = (unsigned char *) XtMalloc(gimage->bytes);
            (void) memcpy((char *) gimage->bitmap, ngw->gedittb.gimage->bitmap,
                          gimage->bytes);

            ngw->gedittb.gimage = gimage;

            /*
             * Set the color index if necessary.
             */
            if (ngw->gedittb.cidx == -1) {
                if ((ngw->gedittb.cidx = (1 << gimage->bpp) - 1) == 0)
                  ngw->gedittb.cidx = 1;
            } else if (gimage->bpp == 1)
              ngw->gedittb.cidx = 1;
            else if (ngw->gedittb.cidx >= (1 << gimage->bpp))
              ngw->gedittb.cidx = (1 << gimage->bpp) - 1;

            if (gimage->width != owidth || gimage->height != oheight) {
                /*
                 * Since a size change is taking place, figure out
                 * the new geometry.
                 */
                _XmuttGEditTBoxPreferredGeometry(ngw, &width, &height);
                ngw->core.width = width;
                ngw->core.height = height;

                /*
                 * Destroy the image region if a reposition is going to occur.
                 */
                if (ngw->gedittb.image_region != 0) {
                    XDestroyRegion(ngw->gedittb.image_region);
                    ngw->gedittb.image_region = 0;
                }
                repos = redo = True;
            } else {
                /*
                 * Restore the original X and Y coordinates of the
                 * image because it didn't change.
                 */
                gimage->x = ox;
                gimage->y = oy;
            }
        } else if (owidth || oheight)
          repos = True;

        /*
         * If the widget will not be redrawn and the new glyph image didn't
         * change size, then simply redraw the glyph image.
         */
        if (redo == False && repos == False)
          _XmuttGEditTBoxDrawImage(newone);
    }

    /*
     * Reposition the glyph image and buttons if necessary.
     */
    if (repos == True)
      _XmuttGEditTBoxPosition(ngw);

    return redo;
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

void
#ifndef _NO_PROTO
XmuttGlyphEditToolboxSetToggle(Widget w, int toggle, Boolean activate)
#else
XmuttGlyphEditToolboxSetToggle(w, toggle, activate)
Widget w;
int toggle;
Boolean activate;
#endif
{
    int i, o;
    XmuttGlyphEditToolboxWidget gw;
    _XmuttGEditTBoxButton *bp;
    XmuttGlyphEditToolboxCallbackStruct cb;

    _XmuttGEditTBoxCheckClass(w);

    if (toggle < XmuttGLYPHEDIT_DRAW || toggle > XmuttGLYPHEDIT_COPY)
      return;

    /*
     * Decrement the toggle index to get the usable value.
     */
    toggle--;

    gw = (XmuttGlyphEditToolboxWidget) w;

    bp = gw->gedittb.buttons;

    if (bp[toggle].set == False) {
        bp[toggle].set = True;
        if (XtIsRealized(w)) {
            /*
             * Turn off the other toggles.
             */
            for (i = 0; i < 2; i++) {
                o = bp[toggle].other_toggles[i];
                if (bp[o].set == True) {
                    bp[o].set = False;
                    _XmuttGEditTBoxDrawToggle(w, o);
                }
            }
            /*
             * Turn on the one that was set.
             */
            _XmuttGEditTBoxDrawToggle(w, toggle);
        }
    }

    /*
     * Set up and call the callback if requested.
     */
    if (activate == True) {
        cb.reason = XmuttGLYPHEDIT_OPERATION;
        cb.event = 0;
        cb.op_action = cback_values[toggle];
        XtCallCallbackList(w, gw->gedittb.activate, (XtPointer) &cb);
    }
}

int
#ifndef _NO_PROTO
XmuttGlyphEditToolboxQueryToggle(Widget w)
#else
XmuttGlyphEditToolboxQueryToggle(w)
Widget w;
#endif
{
    int i, which;
    XmuttGlyphEditToolboxWidget gw;
    _XmuttGEditTBoxButton *bp;

    _XmuttGEditTBoxCheckClass(w);

    gw = (XmuttGlyphEditToolboxWidget) w;

    bp = gw->gedittb.buttons;

    for (which = -1, i = _GETB_DRAW_TOGGLE;
         which == -1 && i <= _GETB_COPY_TOGGLE; i++) {
        if (bp[i].set == True)
          which = i;
    }
    return (which >= 0) ? cback_values[which] : which;
}

void
#ifndef _NO_PROTO
XmuttGlyphEditToolboxSetColor(Widget w, int color)
#else
XmuttGlyphEditToolboxSetColor(w, color)
Widget w;
int color;
#endif
{
    _XmuttGEditTBoxCheckClass(w);

    _XmuttGEditTBoxSelectColor(w, color);
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
    XmuttGlyphEditToolboxWidget gw;

    /*
     * There is only one way to really gain the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    gw = (XmuttGlyphEditToolboxWidget) w;
    gw->gedittb.hasfocus = True;

    _XmuttGEditTBoxHighlightWindow(w);
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
    XmuttGlyphEditToolboxWidget gw;

    /*
     * There is only one way to really lose the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    gw = (XmuttGlyphEditToolboxWidget) w;
    gw->gedittb.hasfocus = False;

    _XmuttGEditTBoxHighlightWindow(w);
}

static void
#ifndef _NO_PROTO
_XmuttGEditActivateCallback(XtPointer client_data, XtIntervalId *id)
#else
_XmuttGEditActivateCallback(client_data, id)
XtPointer client_data;
XtIntervalId *id;
#endif
{
    Widget w;
    XmuttGlyphEditToolboxWidget gw;
    XmuttGlyphEditToolboxCallbackStruct cb;

    w = (Widget) client_data;
    gw = (XmuttGlyphEditToolboxWidget) w;

    /*
     * Call the activate callback for the button in question.
     */
    cb.reason = XmuttGLYPHEDIT_OPERATION;
    cb.event = 0;
    cb.op_action = cback_values[gw->gedittb.timer_button];
    XtCallCallbackList(w, gw->gedittb.activate, (XtPointer) &cb);

    /*
     * Increment the number of times the timer caused the callback to be
     * called.
     */
    gw->gedittb.timer_count++;

    /*
     * Add the timer again, because the toolkit removed it this call.
     */
    gw->gedittb.timer = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
                                        100, _XmuttGEditActivateCallback,
                                        (XtPointer) w);
}

static void
#ifndef _NO_PROTO
ArmButton(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ArmButton(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttGlyphEditToolboxWidget gw;
    _XmuttGEditTBoxButton *bp;
    int i, b, o;
    XmuttGlyphEditToolboxCallbackStruct cb;

    gw = (XmuttGlyphEditToolboxWidget) w;
    bp = gw->gedittb.buttons;

    b = _XmuttGEditTBoxXYToButton(gw, event->xbutton.x, event->xbutton.y);

    if (b < 0 || b == _GETB_GLYPH_IMAGE)
      return;

    /*
     * Handle the case of a color being selected first.
     */
    if (b > _GETB_GLYPH_IMAGE) {
        _XmuttGEditTBoxSelectColor(w, b - (_GETB_GLYPH_IMAGE + 1));
        cb.reason = XmuttGLYPHEDIT_COLOR;
        cb.event = event;
        cb.color = b - (_GETB_GLYPH_IMAGE + 1);
        XtCallCallbackList(w, gw->gedittb.activate, (XtPointer) &cb);
        return;
    }

    if (bp[b].toggle == True && bp[b].set == False) {
        for (i = 0; i < 2; i++) {
            o = bp[b].other_toggles[i];
            if (bp[o].set == True) {
                bp[o].set = False;
                _XmuttGEditTBoxDrawToggle(w, o);
            }
        }
        /*
         * Activate the toggle.
         */
        cb.reason = XmuttGLYPHEDIT_OPERATION;
        cb.event = event;
        cb.op_action = cback_values[b];
        XtCallCallbackList(w, gw->gedittb.activate, (XtPointer) &cb);
    }
    bp[b].set = True;
    if (bp[b].toggle == False) {
        /*
         * Draw the button and start the timer for continual pressing of
         * the button.
         */
        _XmuttGEditTBoxDrawButton(w, b);

        /*
         * Only add the timer to the shift buttons.
         */
        if (b >= _GETB_ULEFT_BUTTON && b <= _GETB_DRIGHT_BUTTON) {
            gw->gedittb.timer_count = 0;
            gw->gedittb.timer_button = b;
            gw->gedittb.timer =
                XtAppAddTimeOut(XtWidgetToApplicationContext(w),
                                200, _XmuttGEditActivateCallback,
                                (XtPointer) w);
        }
    } else
      _XmuttGEditTBoxDrawToggle(w, b);

    gw->gedittb.pressed = b;
}

static void
#ifndef _NO_PROTO
DisarmButton(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
DisarmButton(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    int b, which;
    Boolean call_callback;
    _XmuttGEditTBoxButton *bp;
    XmuttGlyphEditToolboxWidget gw;
    XmuttGlyphEditToolboxCallbackStruct cb;

    gw = (XmuttGlyphEditToolboxWidget) w;

    /*
     * Set the flag that tells this routine whether it should call the
     * callback or if the callback was handled by the timer.
     */
    call_callback = (gw->gedittb.timer_count == 0) ? True : False;

    /*
     * If a timer exists, remove it.  Only call the activate callback
     * if the timer was on and was actually called.
     */
    if (gw->gedittb.timer != 0) {
        XtRemoveTimeOut(gw->gedittb.timer);
        gw->gedittb.timer = 0;
        gw->gedittb.timer_button = gw->gedittb.timer_count = 0;
    }

    bp = gw->gedittb.buttons;

    b = _XmuttGEditTBoxXYToButton(gw, event->xbutton.x, event->xbutton.y);

    if (b < 0 || b > _GETB_DRIGHT_BUTTON)
      return;

    which = (b != gw->gedittb.pressed) ? gw->gedittb.pressed : b;
    if (bp[which].toggle == False) {
        bp[which].set = False;
        if (which == b && call_callback == True) {
            cb.reason = XmuttGLYPHEDIT_ACTION;
            cb.event = event;
            cb.op_action = cback_values[b];
            XtCallCallbackList(w, gw->gedittb.activate, (XtPointer) &cb);
        }
        _XmuttGEditTBoxDrawButton(w, which);
    }
    gw->gedittb.pressed = -1;
}

static void
#ifndef _NO_PROTO
ContextHelp(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ContextHelp(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    int b;
    XmuttGlyphEditToolboxWidget gw;
    _XmuttGEditTBoxButton *bp;
    char cbuf[16];
    XmuttGlyphEditToolboxCallbackStruct cb;

    gw = (XmuttGlyphEditToolboxWidget) w;
    bp = gw->gedittb.buttons;

    b = _XmuttGEditTBoxXYToButton(gw, event->xbutton.x, event->xbutton.y);

    if (b < 0) {
        if (gw->gedittb.last != b) {
            /*
             * Cause the last message to be erased.
             */
            cb.reason = XmuttGLYPHEDIT_CONTEXT_HELP;
            cb.event = event;
            cb.help = " ";
            XtCallCallbackList(w, gw->gedittb.help, (XtPointer) &cb);
        }
        gw->gedittb.last = b;
        return;
    }

    if (b != gw->gedittb.last) {
        cb.reason = XmuttGLYPHEDIT_CONTEXT_HELP;
        cb.event = event;
        if (b > _GETB_GLYPH_IMAGE) {
            sprintf(cbuf, "Color %d", b - _GETB_GLYPH_IMAGE);
            cb.help = cbuf;
        } else
          cb.help = bp[b].help;
        XtCallCallbackList(w, gw->gedittb.help, (XtPointer) &cb);
    }
    gw->gedittb.last = b;
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

static char default_trans[] = "\
<FocusIn>: gain-focus()\n\
<FocusOut>: lose-focus()\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(next)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(prev)\n\
<Btn1Down>: arm-button()\n\
<Btn1Up>: disarm-button()\n\
<Motion>: context-help()\n\
";

static XtActionsRec actions[] = {
    {"traverse", Traverse},
    {"gain-focus", GainFocus},
    {"lose-focus", LoseFocus},
    {"arm-button", ArmButton},
    {"disarm-button", DisarmButton},
    {"context-help", ContextHelp},
};

static XtResource resources[] = {
    {
        XmNtraversalOn,
        XmCTraversalOn,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmPrimitiveWidget, primitive.traversal_on),
        XmRImmediate,
        (XtPointer) False,
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
        XmNcontextHelpCallback,
        XmCContextHelpCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttGlyphEditToolboxWidget, gedittb.help),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNactivateCallback,
        XmCCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttGlyphEditToolboxWidget, gedittb.activate),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNglyphImage,
        XmCGlyphImage,
        XmRPointer,
        sizeof(bdf_bitmap_t *),
        XtOffset(XmuttGlyphEditToolboxWidget, gedittb.gimage),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNcolorList,
        XmCColorList,
        XmRPointer,
        sizeof(unsigned long *),
        XtOffset(XmuttGlyphEditToolboxWidget, gedittb.colors),
        XmRImmediate,
        (XtPointer) 0,
    },
};

externaldef(xmuttglyphedittboxclassrec)
XmuttGlyphEditTBoxClassRec xmuttGlyphEditTBoxClassRec = {
    {
	(WidgetClass) &xmPrimitiveClassRec,	/* superclass		*/
	"XmuttGlyphEditToolbox",		/* class_name		*/
	sizeof(XmuttGlyphEditTBoxRec),		/* widget_size		*/
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

    {						/* XmuttGlyphEditToolbox*/
        0,					/* Draw pixmap.		*/
        0,					/* Move pixmap.		*/
        0,					/* Copy pixmap.		*/
        0,					/* Flip horiz pixmap.	*/
        0,					/* Flip vert pixmap.	*/
        0,					/* Shear pixmap.	*/
        0,					/* Rotate right pixmap.	*/
        0,					/* Rotate left pixmap.	*/
        0,					/* Rotate pixmap.	*/
        0,					/* Up-left pixmap.	*/
        0,					/* Up pixmap.		*/
        0,					/* Up-right pixmap.	*/
        0,					/* Left pixmap.		*/
        0,					/* Right pixmap.	*/
        0,					/* Down-left pixmap.	*/
        0,					/* Down pixmap.		*/
        0,					/* Down-right pixmap.	*/
        0,					/* Class reference count*/
        0,					/* Extension record pointer.*/
    }
};

externaldef(xmuttglyphedittoolboxwidgetclass)
WidgetClass xmuttGlyphEditToolboxWidgetClass =
(WidgetClass) &xmuttGlyphEditTBoxClassRec;

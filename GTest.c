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
#ifndef lint
#ifdef __GNUC__
static char rcsid[] __attribute__ ((unused)) = "$Id: GTest.c,v 1.7 2000/03/16 20:08:49 mleisher Exp $";
#else
static char rcsid[] = "$Id: GTest.c,v 1.7 2000/03/16 20:08:49 mleisher Exp $";
#endif
#endif

#include <stdlib.h>
#include "GTestP.h"

static void
#ifndef _NO_PROTO
_XmuttGlyphTestAttemptResize(XmuttGlyphTestWidget gw, Dimension width,
                             Dimension height)
#else
_XmuttGlyphTestAttemptResize(gw, width, height)
XmuttGlyphTestWidget gw;
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

static void
#ifndef _NO_PROTO
_XmuttGlyphTestCheckClass(Widget w)
#else
_XmuttGlyphTestCheckClass(w)
Widget w;
#endif
{
    if (XtIsSubclass(w, xmuttGlyphTestWidgetClass) == False) {
        fprintf(stderr,
                "XmuttGlyphTest: %s not an XmuttGlyphTestWidget.\n",
                XtName(w));
        exit(1);
    }
}

static Dimension
#ifndef _NO_PROTO
_XmuttGlyphTestPMargins(XmuttGlyphTestWidget gw)
#else
_XmuttGlyphTestPMargins(gw)
XmuttGlyphTestWidget gw;
#endif
{
    return (gw->primitive.shadow_thickness << 1) +
        (gw->primitive.highlight_thickness << 1);
}

/*
 * Change window highlighting when focus is gained or lost.
 */
static void
#ifndef _NO_PROTO
_XmuttGlyphTestHighlightWindow(Widget w)
#else
_XmuttGlyphTestHighlightWindow(w)
Widget w;
#endif
{
    int i;
    XmuttGlyphTestWidget gw;
    XRectangle hi[4];

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphTestWidget) w;

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

    if (gw->gtest.hasfocus == True)
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
_XmuttGlyphTestShadowWindow(Widget w)
#else
_XmuttGlyphTestShadowWindow(w)
Widget w;
#endif
{
    XmuttGlyphTestWidget gw;
    int i;
    XPoint br[3], tl[3];

    gw = (XmuttGlyphTestWidget) w;

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

static Boolean
#ifndef _NO_PROTO
_XmuttGlyphTestSetLineSize(XmuttGlyphTestWidget gw)
#else
_XmuttGlyphTestSetLineSize(gw)
XmuttGlyphTestWidget gw;
#endif
{
    Boolean changed;
    unsigned long i;
    Dimension wd, wwidth;
    _XmuttGlyphTestLine *lp;
    bdf_bbx_t bbx;

    changed = False;
    lp = &gw->gtest.line;

    (void) memset((char *) &bbx, 0, sizeof(bdf_bbx_t));

    wwidth = gw->core.width - (_XmuttGlyphTestPMargins(gw) + 4);

    for (i = 0, wd = 0; i < lp->glyphs_used; i++) {
        bbx.ascent = MAX(bbx.ascent, lp->glyphs[i].font->bbx.ascent);
        bbx.descent = MAX(bbx.descent, lp->glyphs[i].font->bbx.descent);
        bbx.width = MAX(bbx.width, lp->glyphs[i].font->bbx.width);
        if (lp->glyphs[i].glyph->dwidth !=
            lp->glyphs[i].glyph->bbx.width + lp->glyphs[i].glyph->bbx.x_offset)
          wd += lp->glyphs[i].glyph->dwidth;
        else
          wd += lp->glyphs[i].glyph->bbx.x_offset +
            lp->glyphs[i].glyph->bbx.width;
    }

    if (lp->glyphs_used == 0) {
        /*
         * If no glyphs are present, then set the overall bounding box
         * to some simple default.
         */
        bbx.ascent = 12;
        bbx.descent = 3;
        bbx.width = 10;
        wd = bbx.width << 3;
    }

    /*
     * If the actual line width changed, set the indicator.
     */
    if (wd != lp->width) {
        lp->width = wd;

        /*
         * If the line width overflows the window width, set the changed flag.
         */
        if (wd > wwidth)
          changed = True;
    }

    /*
     * If the new bounding box is not the same as the current line bounding
     * box, then make the new one the current line bounding box.
     */
    if (bbx.ascent != lp->bbx.ascent || bbx.descent != lp->bbx.descent ||
        bbx.width != lp->bbx.width) {
        (void) memcpy((char *) &lp->bbx, (char *) &bbx, sizeof(bdf_bbx_t));
        changed = True;
    }

    /*
     * Now set the line size.
     */
    lp->height = lp->bbx.ascent + lp->bbx.descent;
    lp->cpoint.y = (_XmuttGlyphTestPMargins(gw) >> 1) + 2 + lp->bbx.ascent;

    return changed;
}

static void
#ifndef _NO_PROTO
_XmuttGlyphTestPreferredGeometry(XmuttGlyphTestWidget gw, Dimension *width,
                                 Dimension *height)
#else
_XmuttGlyphTestPreferredGeometry(gw, width, height)
XmuttGlyphTestWidget gw;
Dimension *width, *height;
#endif
{
    Dimension wd, ht;

    wd = _XmuttGlyphTestPMargins(gw) + 4 + gw->gtest.line.width;
    ht = _XmuttGlyphTestPMargins(gw) + 4 + gw->gtest.line.height;
    wd = MAX(gw->core.width, wd);
    ht = MAX(gw->core.height, ht);
    *width = wd;
    *height = ht;
}

static void
#ifndef _NO_PROTO
_XmuttGlyphTestMakeGCs(Widget w, Boolean force)
#else
_XmuttGlyphTestMakeGCs(w, force)
Widget w;
Boolean force;
#endif
{
    XmuttGlyphTestWidget gw;
    XtGCMask gcm;
    XGCValues gcv;

    gw = (XmuttGlyphTestWidget) w;

    gcm = GCForeground|GCBackground|GCFunction;

    if (gw->gtest.glyphgc == 0 || force == True) {
        if (gw->gtest.glyphgc != 0)
          XFreeGC(XtDisplay(w), gw->gtest.glyphgc);
        gcv.function = GXcopy;
        gcv.foreground = gw->primitive.foreground;
        gcv.background = gw->core.background_pixel;
        gw->gtest.glyphgc = XCreateGC(XtDisplay(w), XtWindow(w), gcm, &gcv);
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphTestGetPixels(XmuttGlyphTestWidget gw, bdf_glyph_t *glyph,
                         bdf_font_t *font, Position x, Position y, int color)
#else
_XmuttGlyphTestGetPixels(gw, glyph, font, x, y, color)
XmuttGlyphTestWidget gw;
bdf_glyph_t *glyph;
bdf_font_t *font;
Position x, y;
int color;
#endif
{
    int byte;
    unsigned short i, j, bpr, si, di, nx;
    unsigned char *masks;

    gw->gtest.image_used = 0;

    switch (font->bpp) {
      case 1: masks = onebpp; di = 7; break;
      case 2: masks = twobpp; di = 3; break;
      case 4: masks = fourbpp; di = 1; break;
    }

    bpr = ((glyph->bbx.width * font->bpp) + 7) >> 3;
    for (i = 0; i < glyph->bbx.height; i++) {
        for (nx = j = 0; j < glyph->bbx.width; j++, nx += font->bpp) {
            si = (nx & 7) / font->bpp;

            byte = glyph->bitmap[(i * bpr) + (nx >> 3)] & masks[si];
            if (di > si)
              byte >>= (di - si) * font->bpp;
            if (byte == color) {
                if (gw->gtest.image_used == gw->gtest.image_size) {
                    if (gw->gtest.image_size == 0)
                      gw->gtest.image =
                          (XPoint *) XtMalloc(sizeof(XPoint) * 64);
                    else
                      gw->gtest.image = (XPoint *)
                          XtRealloc((char *) gw->gtest.image,
                                    sizeof(XPoint) *
                                    (gw->gtest.image_size + 64));;
                    gw->gtest.image_size += 64;
                }
                gw->gtest.image[gw->gtest.image_used].x =
                    x + glyph->bbx.x_offset + j;
                gw->gtest.image[gw->gtest.image_used].y =
                    (y - glyph->bbx.ascent) + i;
                gw->gtest.image_used++;
            }
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphTestDrawGlyph(Widget w, bdf_glyph_t *glyph, bdf_font_t *font)
#else
_XmuttGlyphTestDrawGlyph(w, glyph, font)
Widget w;
bdf_glyph_t *glyph;
bdf_font_t *font;
#endif
{
    int i, s, e;
    Position rx, ry;
    XmuttGlyphTestWidget gw;

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphTestWidget) w;

    switch (font->bpp) {
      case 1: s = 0; e = 2; break;
      case 2: s = 0; e = 4; break;
      case 4: s = 4; e = 20; break;
    }

    ry = gw->gtest.line.cpoint.y;
    rx = gw->gtest.line.cpoint.x;
    if (gw->gtest.dir != XmuttGlyphTestLeftToRight)
      rx -= glyph->bbx.width;

    for (i = s; i < e; i++) {
        if (i == 0)
          /*
           * Avoid drawing the pixels with the background color.
           */
          continue;

        _XmuttGlyphTestGetPixels(gw, glyph, font, rx, ry, i);
        if (gw->gtest.image_used > 0) {
            if (font->bpp > 1) {
                if (i - s)
                  XSetForeground(XtDisplay(w), gw->gtest.glyphgc,
                                 gw->gtest.colors[i - s]);
                else
                  /*
                   * Make all 0 colors the same as the background.
                   */
                  XSetForeground(XtDisplay(w), gw->gtest.glyphgc,
                                 gw->core.background_pixel);
            }
            XDrawPoints(XtDisplay(w), XtWindow(w), gw->gtest.glyphgc,
                        gw->gtest.image, gw->gtest.image_used,
                        CoordModeOrigin);
        }
    }
    if (font->bpp > 1)
      XSetForeground(XtDisplay(w), gw->gtest.glyphgc,
                     gw->primitive.foreground);
}

static void
#ifndef _NO_PROTO
_XmuttGlyphTestRedrawGlyphs(Widget w)
#else
_XmuttGlyphTestRedrawGlyphs(w)
Widget w;
#endif
{
    XmuttGlyphTestWidget gw;
    unsigned long i;
    _XmuttGlyphTestLine *lp;
    _XmuttGlyphTestGlyph *gp;

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphTestWidget) w;

    lp = &gw->gtest.line;

    lp->width = 0;
    if (gw->gtest.dir == XmuttGlyphTestLeftToRight)
      lp->cpoint.x = (_XmuttGlyphTestPMargins(gw) >> 1) + 2;
    else
      lp->cpoint.x = gw->core.width - ((_XmuttGlyphTestPMargins(gw) >> 1) + 2);

    for (i = 0, gp = lp->glyphs; i < lp->glyphs_used; i++, gp++) {

        /*
         * Handle the special cases of the first glyph in case the normal
         * drawing position is going to put part of the glyph off the edge of
         * the window.
         */
        if (gw->gtest.dir == XmuttGlyphTestLeftToRight) {
            if (i == 0 && gp->glyph->bbx.x_offset < 0)
              lp->cpoint.x += -gp->glyph->bbx.x_offset;
        } else {
            if (i == 0 && gp->glyph->bbx.x_offset > 0 &&
                gp->glyph->bbx.x_offset > gp->glyph->bbx.width)
              lp->cpoint.x -= gp->glyph->bbx.width - gp->glyph->bbx.x_offset;
        }
        _XmuttGlyphTestDrawGlyph(w, gp->glyph, gp->font);

        if (gw->gtest.dir == XmuttGlyphTestLeftToRight) {
            if (gp->glyph->dwidth !=
                gp->glyph->bbx.width + gp->glyph->bbx.x_offset)
              lp->cpoint.x += gp->glyph->dwidth;
            else
              lp->cpoint.x += gp->glyph->bbx.width + gp->glyph->bbx.x_offset;
        } else {
            if (gp->glyph->dwidth !=
                gp->glyph->bbx.width + gp->glyph->bbx.x_offset)
              lp->cpoint.x -= gp->glyph->dwidth;
            else
              lp->cpoint.x -= gp->glyph->bbx.width - gp->glyph->bbx.x_offset;
        }
        if (gp->glyph->dwidth !=
            gp->glyph->bbx.width + gp->glyph->bbx.x_offset)
          lp->width += gp->glyph->dwidth;
        else
          lp->width += gp->glyph->bbx.width + gp->glyph->bbx.x_offset;
    }
}

static void
#ifndef _NO_PROTO
_XmuttGlyphTestRedraw(Widget w)
#else
_XmuttGlyphTestRedraw(w)
Widget w;
#endif
{
    XmuttGlyphTestWidget gw;
    XPoint s, e;
    XRectangle clear;

    if (!XtIsRealized(w))
      return;

    gw = (XmuttGlyphTestWidget) w;

    /*
     * Erase the window.
     */
    clear.x = clear.y = (_XmuttGlyphTestPMargins(gw) >> 1);
    clear.width = gw->core.width - (clear.x << 1);
    clear.height = gw->core.height - (clear.y << 1);
    XClearArea(XtDisplay(w), XtWindow(w), clear.x, clear.y,
               clear.width, clear.height, False);

    /*
     * Redraw the glyphs.
     */
    _XmuttGlyphTestRedrawGlyphs(w);

    /*
     * Draw the baseline if indicated.
     */
    if (gw->gtest.show_baseline == True) {
        s.x = (_XmuttGlyphTestPMargins(gw) >> 1) + 2;
        e.x = gw->core.width - s.x;
        s.y = e.y = gw->gtest.line.cpoint.y;

        XDrawLine(XtDisplay(w), XtWindow(w), gw->gtest.glyphgc, s.x, s.y,
                  e.x, e.y);
    }
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
    XmuttGlyphTestWidget gw;
    Dimension wd, ht;

    gw = (XmuttGlyphTestWidget) newone;

    /*
     * Initialize the GCs.
     */
    gw->gtest.glyphgc = 0;

    /*
     * Initialize the points list for the glyph images.
     */
    gw->gtest.image_size = gw->gtest.image_used = 0;

    /*
     * Initialize the line structures.
     */
    (void) memset((char *) &gw->gtest.line, 0, sizeof(_XmuttGlyphTestLine));

    gw->gtest.show_baseline = True;
    gw->gtest.hasfocus = False;

    /*
     * Set the initial geometry.
     */
    (void) _XmuttGlyphTestSetLineSize(gw);
    _XmuttGlyphTestPreferredGeometry(gw, &wd, &ht);
    gw->core.width = wd;
    gw->core.height = ht;

    /*
     * Initialize the starting point for glyph drawing.
     */
    if (gw->gtest.dir == XmuttGlyphTestLeftToRight)
      gw->gtest.line.cpoint.x = (_XmuttGlyphTestPMargins(gw) >> 1) + 2;
    else
      gw->gtest.line.cpoint.x = gw->core.width -
          ((_XmuttGlyphTestPMargins(gw) >> 1) + 2);

    gw->gtest.line.cpoint.y = (_XmuttGlyphTestPMargins(gw) >> 1) + 2 +
        gw->gtest.line.bbx.ascent;
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

    _XmuttGlyphTestMakeGCs(w, False);
}

static void
#ifndef _NO_PROTO
Destroy(Widget w)
#else
Destroy(w)
Widget w;
#endif
{
    XmuttGlyphTestWidget gw;

    gw = (XmuttGlyphTestWidget) w;

    if (gw->gtest.image_size > 0)
      XtFree((char *) gw->gtest.image);
    gw->gtest.image_size = gw->gtest.image_used = 0;

    if (gw->gtest.glyphgc != 0)
      XFreeGC(XtDisplay(w), gw->gtest.glyphgc);
    gw->gtest.glyphgc = 0;

    if (gw->gtest.line.glyphs_size > 0)
      XtFree((char *) gw->gtest.line.glyphs);
    gw->gtest.line.glyphs_size = gw->gtest.line.glyphs_used = 0;
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
    _XmuttGlyphTestHighlightWindow(w);
    _XmuttGlyphTestShadowWindow(w);
    _XmuttGlyphTestRedraw(w);
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
    XmuttGlyphTestWidget ogw, ngw;
    Boolean redo;

    ogw = (XmuttGlyphTestWidget) curr;
    ngw = (XmuttGlyphTestWidget) newone;
    redo = False;

    if (ngw->gtest.show_baseline != ogw->gtest.show_baseline ||
        ngw->gtest.dir != ogw->gtest.dir)
      redo = True;

    return redo;
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

void
#ifndef _NO_PROTO
XmuttGlyphTestAddGlyph(Widget w, bdf_font_t *font,
                       bdf_glyph_t *glyph)
#else
XmuttGlyphTestAddGlyph(w, font, glyph)
Widget w;
bdf_font_t *font;
bdf_glyph_t *glyph;
#endif
{
    XmuttGlyphTestWidget gw;
    _XmuttGlyphTestGlyph *gp;
    _XmuttGlyphTestLine *lp;
    Dimension wd, ht;

    _XmuttGlyphTestCheckClass(w);

    if (glyph == 0 || font == 0)
      return;

    gw = (XmuttGlyphTestWidget) w;

    lp = &gw->gtest.line;

    if (lp->glyphs_used == lp->glyphs_size) {
        if (lp->glyphs_size == 0)
          lp->glyphs = (_XmuttGlyphTestGlyph *)
              XtMalloc(sizeof(_XmuttGlyphTestGlyph) << 3);
        else
          lp->glyphs = (_XmuttGlyphTestGlyph *)
              XtRealloc((char *) lp->glyphs,
                        sizeof(_XmuttGlyphTestGlyph) * (lp->glyphs_size + 8));
        lp->glyphs_size += 8;
    }

    /*
     * Add the glyph to the line.
     */
    gp = lp->glyphs + lp->glyphs_used++;
    gp->font = font;
    gp->glyph = glyph;

    if (_XmuttGlyphTestSetLineSize(gw) == True) {
        _XmuttGlyphTestPreferredGeometry(gw, &wd, &ht);
        if (wd > gw->core.width || ht > gw->core.height)
          /*
           * The resize will force a redraw to occur.
           */
          _XmuttGlyphTestAttemptResize(gw, wd, ht);
        else
          /*
           * Force the widget to redraw completely.
           */
          _XmuttGlyphTestRedraw(w);
    } else {
        /*
         * Just draw the glyph.
         */

        /*
         * If the first glyph would be drawn off the edge of the window, make
         * sure the initial position is adjusted to display the first glyph at
         * the edge.
         */
        if (gw->gtest.dir == XmuttGlyphTestLeftToRight) {
            if (gw->gtest.line.glyphs_used == 1 && glyph->bbx.x_offset < 0)
              lp->cpoint.x += -glyph->bbx.x_offset;
        } else {
            if (gw->gtest.line.glyphs_used == 1 && glyph->bbx.x_offset > 0 &&
                glyph->bbx.x_offset > glyph->bbx.width)
              lp->cpoint.x -= glyph->bbx.width - glyph->bbx.x_offset;
        }

        _XmuttGlyphTestDrawGlyph(w, glyph, font);

        if (gw->gtest.dir == XmuttGlyphTestLeftToRight) {
            if (glyph->dwidth !=
                glyph->bbx.width + glyph->bbx.x_offset)
              lp->cpoint.x += glyph->dwidth;
            else
              lp->cpoint.x += glyph->bbx.width + glyph->bbx.x_offset;
        } else {
            if (glyph->dwidth !=
                glyph->bbx.width + glyph->bbx.x_offset)
              lp->cpoint.x -= glyph->dwidth;
            else
              lp->cpoint.x -= glyph->bbx.width - glyph->bbx.x_offset;
        }
        if (glyph->dwidth !=
            glyph->bbx.width + glyph->bbx.x_offset)
          lp->width += glyph->dwidth;
        else
          lp->width += glyph->bbx.width + glyph->bbx.x_offset;
    }

    /*
     * Last, call the add callback.
     */
    XtCallCallbackList(w, gw->gtest.add, 0);
}

void
#ifndef _NO_PROTO
XmuttGlyphTestErase(Widget w)
#else
XmuttGlyphTestErase(w)
Widget w;
#endif
{
    XmuttGlyphTestWidget gw;

    _XmuttGlyphTestCheckClass(w);

    gw = (XmuttGlyphTestWidget) w;

    gw->gtest.line.glyphs_used = 0;

    _XmuttGlyphTestRedraw(w);
}

void
#ifndef _NO_PROTO
XmuttGlyphTestRedisplay(Widget w)
#else
XmuttGlyphTestRedisplay(w)
Widget w;
#endif
{
    _XmuttGlyphTestCheckClass(w);

    _XmuttGlyphTestRedraw(w);
}

void
#ifndef _NO_PROTO
XmuttGlyphTestRemoveFont(Widget w, bdf_font_t *font)
#else
XmuttGlyphTestRemoveFont(w, font)
Widget w;
bdf_font_t *font;
#endif
{
    unsigned long i, j, used, redo;
    Dimension wd, ht;
    XmuttGlyphTestWidget gw;
    _XmuttGlyphTestLine *lp;

    _XmuttGlyphTestCheckClass(w);

    gw = (XmuttGlyphTestWidget) w;

    lp = &gw->gtest.line;
    for (i = j = redo = 0, used = lp->glyphs_used; i < used; i++) {
        if (lp->glyphs[i].font != font) {
            lp->glyphs[j].font = lp->glyphs[i].font;
            lp->glyphs[j].glyph = lp->glyphs[i].glyph;
            j++;
        }
    }
    redo = (lp->glyphs_used != j);
    lp->glyphs_used = j;

    if (redo) {
        if (_XmuttGlyphTestSetLineSize(gw) == True) {
            _XmuttGlyphTestPreferredGeometry(gw, &wd, &ht);
            if (wd > gw->core.width || ht > gw->core.height)
              _XmuttGlyphTestAttemptResize(gw, wd, ht);
            else
              _XmuttGlyphTestRedraw(w);
        } else
          _XmuttGlyphTestRedraw(w);
    }
}

void
#ifndef _NO_PROTO
XmuttGlyphTestResize(Widget w)
#else
XmuttGlyphTestResize(w)
Widget w;
#endif
{
    XmuttGlyphTestWidget gw;
    Dimension wd, ht;

    _XmuttGlyphTestCheckClass(w);

    gw = (XmuttGlyphTestWidget) w;

    if (_XmuttGlyphTestSetLineSize(gw) == True) {
        _XmuttGlyphTestPreferredGeometry(gw, &wd, &ht);
        if (wd > gw->core.width || ht > gw->core.height)
          _XmuttGlyphTestAttemptResize(gw, wd, ht);
        else
          _XmuttGlyphTestRedraw(w);
    } else
      _XmuttGlyphTestRedraw(w);
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
    XmuttGlyphTestWidget gw;

    /*
     * There is only one way to really gain the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    gw = (XmuttGlyphTestWidget) w;
    gw->gtest.hasfocus = True;

    _XmuttGlyphTestHighlightWindow(w);
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
    XmuttGlyphTestWidget gw;

    /*
     * There is only one way to really lose the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    gw = (XmuttGlyphTestWidget) w;
    gw->gtest.hasfocus = False;

    _XmuttGlyphTestHighlightWindow(w);
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
    XmuttGlyphTestWidget gw;

    gw = (XmuttGlyphTestWidget) w;

    if (gw->gtest.hasfocus == False)
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
<FocusIn>: gain-focus()\n\
<FocusOut>: lose-focus()\n\
~Shift Ctrl ~Meta ~Alt<Key>Right: traverse(right)\n\
~Shift Ctrl ~Meta ~Alt<Key>Left: traverse(left)\n\
~Shift Ctrl ~Meta ~Alt<Key>Up: traverse(up)\n\
~Shift Ctrl ~Meta ~Alt<Key>Down: traverse(down)\n\
~Shift Ctrl ~Meta ~Alt<Key>KP_Right: traverse(right)\n\
~Shift Ctrl ~Meta ~Alt<Key>KP_Left: traverse(left)\n\
~Shift Ctrl ~Meta ~Alt<Key>KP_Up: traverse(up)\n\
~Shift Ctrl ~Meta ~Alt<Key>KP_Down: traverse(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(prev)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(next)\n\
";

#else /* R5 or lower */

static char default_trans[] = "\
<FocusIn>: gain-focus()\n\
<FocusOut>: lose-focus()\n\
~Shift Ctrl ~Meta ~Alt<Key>Right: traverse(right)\n\
~Shift Ctrl ~Meta ~Alt<Key>Left: traverse(left)\n\
~Shift Ctrl ~Meta ~Alt<Key>Up: traverse(up)\n\
~Shift Ctrl ~Meta ~Alt<Key>Down: traverse(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(prev)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(next)\n\
";

#endif /* R5 or lower */

static XtActionsRec actions[] = {
    {"traverse", Traverse},
    {"gain-focus", GainFocus},
    {"lose-focus", LoseFocus},
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
        XmNdirection,
        XmCDirection,
        XmRInt,
        sizeof(int),
        XtOffset(XmuttGlyphTestWidget, gtest.dir),
        XmRImmediate,
        (XtPointer) XmuttGlyphTestLeftToRight,
    },
    {
        XmNshowBaseline,
        XmCShowBaseline,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttGlyphTestWidget, gtest.show_baseline),
        XmRImmediate,
        (XtPointer) True,
    },
    {
        XmNaddGlyphCallback,
        XmCAddGlyphCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttGlyphTestWidget, gtest.add),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNcolorList,
        XmCColorList,
        XmRPointer,
        sizeof(unsigned long *),
        XtOffset(XmuttGlyphTestWidget, gtest.colors),
        XmRImmediate,
        (XtPointer) 0,
    },
};

externaldef(xmuttglyphtestclassrec)
XmuttGlyphTestClassRec xmuttGlyphTestClassRec = {
    {
	(WidgetClass) &xmPrimitiveClassRec,	/* superclass		*/
	"XmuttGlyphTest",			/* class_name		*/
	sizeof(XmuttGlyphTestRec),		/* widget_size		*/
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

    {						/* XmuttGlyphTest	*/
        0,					/* Extension record pointer.*/
    }
};

externaldef(xmuttglyphtestwidgetclass)
WidgetClass xmuttGlyphTestWidgetClass = (WidgetClass) &xmuttGlyphTestClassRec;

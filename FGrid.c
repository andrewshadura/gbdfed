/*
 * Copyright 2004 Computing Research Labs, New Mexico State University
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
static char rcsid[] __attribute__ ((unused)) = "$Id: FGrid.c,v 1.28 2004/02/08 23:58:59 mleisher Exp $";
#else
static char rcsid[] = "$Id: FGrid.c,v 1.28 2004/02/08 23:58:59 mleisher Exp $";
#endif
#endif

#include <stdlib.h>
#include "FGridP.h"

static bdf_glyph_t empty_glyph;

#define GETSHORT(s) ((s[0] << 8) | s[1])
#define GETLONG(s) ((s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3])

static void
#ifndef _NO_PROTO
_XmuttFGridAttemptResize(XmuttFontGridWidget fw, Dimension width,
                         Dimension height)
#else
_XmuttFGridAttemptResize(fw, width, height)
XmuttFontGridWidget fw;
Dimension width, height;
#endif
{
    Dimension rwidth, rheight, owidth, oheight;
    XtGeometryResult res;

    owidth = fw->core.width;
    oheight = fw->core.height;

    res = XtMakeResizeRequest((Widget) fw, width, height, &rwidth, &rheight);

    if (res == XtGeometryAlmost) {
        res = XtMakeResizeRequest((Widget) fw, rwidth, rheight,
                                  &rwidth, &rheight);
        if (rwidth == owidth && rheight == oheight)
          res = XtGeometryNo;
    } else if (fw->core.width != width && fw->core.height != height)
      res = XtGeometryNo;

    if (res == XtGeometryYes)
      (*fw->core.widget_class->core_class.resize)((Widget) fw);
}

static void
#ifndef _NO_PROTO
_XmuttFGridCheckClass(Widget w)
#else
_XmuttFGridCheckClass(w)
Widget w;
#endif
{
    if (XtIsSubclass(w, xmuttFontGridWidgetClass) == False) {
        fprintf(stderr, "XmuttFontGrid: %s not an XmuttFontGridWidget.\n",
                XtName(w));
        exit(1);
    }
}

static Dimension
#ifndef _NO_PROTO
_XmuttFGridPMargins(XmuttFontGridWidget fw)
#else
_XmuttFGridPMargins(fw)
XmuttFontGridWidget fw;
#endif
{
    return (fw->primitive.shadow_thickness << 2) +
        (fw->primitive.highlight_thickness << 1);
}

static Dimension
#ifndef _NO_PROTO
_XmuttFGridHMargins(XmuttFontGridWidget fw)
#else
_XmuttFGridHMargins(fw)
XmuttFontGridWidget fw;
#endif
{
    return _XmuttFGridPMargins(fw) + (fw->fgrid.mwidth << 2);
}

static Dimension
#ifndef _NO_PROTO
_XmuttFGridVMargins(XmuttFontGridWidget fw)
#else
_XmuttFGridVMargins(fw)
XmuttFontGridWidget fw;
#endif
{
    return _XmuttFGridPMargins(fw) + (fw->fgrid.mheight << 2);
}

static Dimension
#ifndef _NO_PROTO
_XmuttFGridLeftX(XmuttFontGridWidget fw)
#else
_XmuttFGridLeftX(fw)
XmuttFontGridWidget fw;
#endif
{
    return (_XmuttFGridPMargins(fw) >> 1) + fw->fgrid.mwidth;
}

static Dimension
#ifndef _NO_PROTO
_XmuttFGridTopY(XmuttFontGridWidget fw)
#else
_XmuttFGridTopY(fw)
XmuttFontGridWidget fw;
#endif
{
    return (_XmuttFGridPMargins(fw) >> 1) + fw->fgrid.mheight;
}

/*
 * Change window highlighting when focus is gained or lost.
 */
static void
#ifndef _NO_PROTO
_XmuttFGridHighlightWindow(Widget w)
#else
_XmuttFGridHighlightWindow(w)
Widget w;
#endif
{
    int i;
    XmuttFontGridWidget fw;
    XRectangle hi[4];

    fw = (XmuttFontGridWidget) w;

    if (!XtIsRealized(w) || fw->primitive.highlight_thickness == 0)
      return;

    /*
     * Top of window.
     */
    hi[0].x = hi[0].y = 0;
    hi[0].width = fw->core.width;
    hi[0].height = fw->primitive.highlight_thickness;

    /*
     * Right side.
     */
    hi[1].x = fw->core.width - fw->primitive.highlight_thickness;
    hi[1].y = hi[0].y + fw->primitive.highlight_thickness;
    hi[1].width = fw->primitive.highlight_thickness;
    hi[1].height = fw->core.height - (fw->primitive.highlight_thickness << 1);

    /*
     * Bottom of window.
     */
    hi[2].x = hi[0].x;
    hi[2].y = fw->core.height - fw->primitive.highlight_thickness;
    hi[2].width = hi[0].width;
    hi[2].height = hi[0].height;

    /*
     * Left side.
     */
    hi[3].x = hi[0].x;
    hi[3].y = hi[1].y;
    hi[3].width = hi[1].width;
    hi[3].height = hi[1].height;

    if (fw->fgrid.hasfocus == True)
      XFillRectangles(XtDisplay(w), XtWindow(w), fw->primitive.highlight_GC,
                      hi, 4);
    else {
        for (i = 0; i < 4; i++)
          XClearArea(XtDisplay(w), XtWindow(w), hi[i].x, hi[i].y,
                     hi[i].width, hi[i].height, False);
    }
}

static void
#ifndef _NO_PROTO
_XmuttFGridShadowWindow(XmuttFontGridWidget fw)
#else
_XmuttFGridShadowWindow(fw)
XmuttFontGridWidget fw;
#endif
{
    Widget w;
    int i;
    XPoint br[3], tl[3];

    w = (Widget) fw;

    if (!XtIsRealized(w) || fw->primitive.shadow_thickness == 0)
      return;

    /*
     * Lower left corner outside line.
     */
    tl[0].x = fw->primitive.highlight_thickness;
    tl[0].y = fw->core.height - fw->primitive.highlight_thickness - 1;

    /*
     * Upper left corner outside line.
     */
    tl[1].x = tl[0].x;
    tl[1].y = fw->primitive.highlight_thickness;

    /*
     * Upper right corner outside line.
     */
    tl[2].x = fw->core.width - fw->primitive.highlight_thickness - 1;
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
    br[1].y = fw->core.height - fw->primitive.highlight_thickness - 1;

    /*
     * Lower left corner outside line.
     */
    br[2].x = fw->primitive.highlight_thickness + 1;
    br[2].y = br[1].y;

    XDrawLines(XtDisplay(w), XtWindow(w), fw->primitive.top_shadow_GC,
               tl, 3, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), fw->primitive.bottom_shadow_GC,
               br, 3, CoordModeOrigin);

    /*
     * Draw the remaining shadows successively inward.
     */
    for (i = 1; i < fw->primitive.shadow_thickness; i++) {
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
        XDrawLines(XtDisplay(w), XtWindow(w), fw->primitive.top_shadow_GC,
                   tl, 3, CoordModeOrigin);
        XDrawLines(XtDisplay(w), XtWindow(w), fw->primitive.bottom_shadow_GC,
                   br, 3, CoordModeOrigin);
    }
}

/*
 * Encode the selected glyphs in a form that can be put on the clipboard.
 */
static unsigned char *
#ifndef _NO_PROTO
_XmuttFGridEncodeSelection(XmuttFontGridWidget fw, unsigned long *bytes)
#else
_XmuttFGridEncodeSelection(fw, bytes)
XmuttFontGridWidget fw;
unsigned long *bytes;
#endif
{
    unsigned long i, bcount, nlen;
    unsigned char *sel, *sp;
    unsigned short a;
    bdf_glyph_t *gp;
    bdf_glyphlist_t *gl;
    _XmuttFGridPageInfo *pi;

    *bytes = 0;
    gl = &fw->fgrid.clipboard;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    if (pi->sel_start == -1)
      return 0;

    if (fw->fgrid.unencoded == False)
      bdf_copy_glyphs(fw->fgrid.font, pi->sel_start, pi->sel_end, gl, 0);
    else
      bdf_copy_glyphs(fw->fgrid.font, pi->sel_start, pi->sel_end, gl, 1);

    /*
     * Calculate the number of bytes that will be needed for everything except
     * the name strings and the bitmap data.
     */
    bcount = (sizeof(unsigned long) << 1) + (6 * sizeof(unsigned short)) +
        (((6 * sizeof(unsigned short)) + sizeof(unsigned long)) *
         gl->glyphs_used);

    /*
     * Figure out how much extra will be needed for the names, bitmaps, and
     * PSF Unicode mappings.
     */
    for (i = 0, gp = gl->glyphs; i < gl->glyphs_used; i++, gp++) {
        nlen = (gp->name) ? (unsigned long) (strlen(gp->name) + 1) : 0;
        /*
         * The extra 2 bytes is for encoding the number of bytes used for the
         * Unicode mappings, even if it is 0.  This could be a problem later
         * if a set of mappings legitimately exceeds 2^16 in length.
         */
        bcount += nlen + gp->bytes + 2 + gp->unicode.map_used;
    }

    /*
     * Allocate all the storage needed.
     */
    sel = sp = (unsigned char *) XtMalloc(bcount);

    /*
     * Set the returned byte count.
     */
    *bytes = bcount;

    /*
     * Encode the 20-byte header.
     */
    a = (unsigned short) gl->bpp;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    nlen = (unsigned long) gl->start;
    *sp++ = (nlen >> 24) & 0xff;
    *sp++ = (nlen >> 16) & 0xff;
    *sp++ = (nlen >> 8) & 0xff;
    *sp++ = nlen & 0xff;

    nlen = (unsigned long) gl->end;
    *sp++ = (nlen >> 24) & 0xff;
    *sp++ = (nlen >> 16) & 0xff;
    *sp++ = (nlen >> 8) & 0xff;
    *sp++ = nlen & 0xff;

    a = (unsigned short) gl->glyphs_used;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (unsigned short) gl->bbx.width;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (unsigned short) gl->bbx.x_offset;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (unsigned short) gl->bbx.ascent;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (unsigned short) gl->bbx.descent;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    /*
     * Go through each glyph entry and encode the data.
     */
    for (i = 0, gp = gl->glyphs; i < gl->glyphs_used; i++, gp++) {
        /*
         * Encode the glyph encoding.
         */
        nlen = (unsigned long) gp->encoding;
        *sp++ = (nlen >> 24) & 0xff;
        *sp++ = (nlen >> 16) & 0xff;
        *sp++ = (nlen >> 8) & 0xff;
        *sp++ = nlen & 0xff;

        /*
         * Encode the glyph device width.
         */
        a = (unsigned short) gp->dwidth;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        /*
         * Encode the glyph name length.
         */
        nlen = 0;
        if (gp->name)
          nlen = (unsigned short) (strlen(gp->name) + 1);
        a = (unsigned short) nlen;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        /*
         * Encode the four bounding box values needed.
         */
        a = (unsigned short) gp->bbx.width;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        a = (unsigned short) gp->bbx.x_offset;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        a = (unsigned short) gp->bbx.ascent;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        a = (unsigned short) gp->bbx.descent;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        /*
         * Encode the name if it exists.
         */
        if (nlen > 0) {
            (void) memcpy((char *) sp, gp->name, nlen);
            sp += nlen;
        }

        /*
         * Encode the bitmap.
         */
        if (gp->bytes > 0) {
            (void) memcpy((char *) sp, (char *) gp->bitmap, gp->bytes);
            sp += gp->bytes;
        }

        /*
         * Encode the PSF Unicode mappings.  Even if there aren't any
         */
        *sp++ = (gp->unicode.map_used >> 8) & 0xff;
        *sp++ = gp->unicode.map_used & 0xff;
        if (gp->unicode.map_used > 0) {
            (void) memcpy((char *) sp, (char *) gp->unicode.map,
                          sizeof(unsigned char) * gp->unicode.map_used);
            sp += gp->unicode.map_used;
        }
    }

    /*
     * Return the selection encoded as a byte stream.
     */
    return sel;
}

/*
 * Convert an encoded selection (probably from the clipboard) into the
 * internal glyph list clipboard.
 */
static void
#ifndef _NO_PROTO
_XmuttFGridDecodeSelection(XmuttFontGridWidget fw, unsigned char *sel)
#else
_XmuttFGridDecodeSelection(fw, sel)
XmuttFontGridWidget fw;
unsigned char *sel;
#endif
{
    unsigned long i, range, nlen;
    bdf_glyph_t *gp;
    bdf_glyphlist_t *gl;

    if (sel == 0)
      return;

    gl = &fw->fgrid.clipboard;

    /*
     * Clear out the bitmaps and names from the existing glyphs.
     */
    for (gp = gl->glyphs, i = 0; i < gl->glyphs_size; i++, gp++) {
        if (gp->name != 0)
          free(gp->name);
        if (gp->bytes > 0)
          free((char *) gp->bitmap);
        if (gp->unicode.map_size > 0)
          free((char *) gp->unicode.map);
    }

    /*
     * Extract the glyph list bits per pixel.
     */
    gl->bpp = GETSHORT(sel);
    sel += 2;

    /*
     * Extract the glyph list starting and ending encodings.
     */
    gl->start = (long) GETLONG(sel);
    sel += 4;

    gl->end = (long) GETLONG(sel);
    sel += 4;

    /*
     * Extract the number of encoded glyphs.
     */
    range = (unsigned long) GETSHORT(sel);
    sel += 2;

    /*
     * Resize the internal glyph list clipboard if necessary.
     */
    if (range > gl->glyphs_size) {
        if (gl->glyphs_size == 0)
          gl->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * range);
        else
          gl->glyphs = (bdf_glyph_t *) realloc((char *) gl->glyphs,
                                               sizeof(bdf_glyph_t) * range);
        gl->glyphs_size = range;
    }

    /*
     * Initialize the glyph list.
     */
    (void) memset((char *) &gl->bbx, 0, sizeof(bdf_bbx_t));
    (void) memset((char *) gl->glyphs, 0,
                  sizeof(bdf_glyph_t) * gl->glyphs_size);

    gl->glyphs_used = range;

    /*
     * Decode the overall metrics of the glyph list.
     */
    gl->bbx.width = GETSHORT(sel);
    sel += 2;
    gl->bbx.x_offset = GETSHORT(sel);
    sel += 2;
    gl->bbx.ascent = GETSHORT(sel);
    sel += 2;
    gl->bbx.descent = GETSHORT(sel);
    sel += 2;
    gl->bbx.height = gl->bbx.ascent + gl->bbx.descent;
    gl->bbx.y_offset = -gl->bbx.descent;

    /*
     * Decode the glyphs.
     */
    for (i = 0, gp = gl->glyphs; i < range; i++, gp++) {
        /*
         * Get the glyph encoding.
         */
        gp->encoding = (long) GETLONG(sel);
        sel += 4;

        /*
         * Get the device width.
         */
        gp->dwidth = GETSHORT(sel);
        sel += 2;

        /*
         * Get the name length.
         */
        nlen = GETSHORT(sel);
        sel += 2;

        /*
         * Get the bounding box.
         */
        gp->bbx.width = GETSHORT(sel);
        sel += 2;
        gp->bbx.x_offset = GETSHORT(sel);
        sel += 2;
        gp->bbx.ascent = GETSHORT(sel);
        sel += 2;
        gp->bbx.descent = GETSHORT(sel);
        sel += 2;
        gp->bbx.height = gp->bbx.ascent + gp->bbx.descent;
        gp->bbx.y_offset = -gp->bbx.descent;

        /*
         * Get the name.
         */
        if (nlen > 0) {
            gp->name = (char *) malloc(nlen);
            (void) memcpy(gp->name, (char *) sel, nlen);
            sel += nlen;
        }

        /*
         * Get the bitmap.
         */

        gp->bytes = ((gp->bbx.width + 7) >> 3) * gp->bbx.height;
        if (gp->bytes > 0) {
            gp->bitmap = (unsigned char *) malloc(gp->bytes);
            (void) memcpy((char *) gp->bitmap, (char *) sel, gp->bytes);
            sel += gp->bytes;
        }

        /*
         * Get the Unicode mappings.
         */
        gp->unicode.map_used = GETSHORT(sel);
        sel += 2;
        if (gp->unicode.map_used > 0) {
            gp->unicode.map_size = ((gp->unicode.map_used >> 2) + 
                                    ((gp->unicode.map_used & 3) ? 1 : 0)) << 2;
            gp->unicode.map = (unsigned char *) malloc(gp->unicode.map_size);
            (void) memcpy((char *) gp->unicode.map, (char *) sel,
                          gp->unicode.map_used);
            sel += gp->unicode.map_used;
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttFGridOwnClipboard(Widget w)
#else
_XmuttFGridOwnClipboard(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    Display *d;
    Window win;
    XEvent ev;

    fw = (XmuttFontGridWidget) w;

    if (!XtIsRealized(w) || fw->fgrid.owns_clipboard == True)
      return;

    d = XtDisplay(w);

    /*
     * Get the current owner of the clipboard.
     */
    win = XGetSelectionOwner(d, XmuttFONTGRID_CLIPBOARD(d));

    XSetSelectionOwner(d, XmuttFONTGRID_CLIPBOARD(d),
                       XtWindow(w), XtLastTimestampProcessed(d));

    if (XGetSelectionOwner(d, XmuttFONTGRID_CLIPBOARD(d)) == XtWindow(w))
      fw->fgrid.owns_clipboard = True;
    else
      fw->fgrid.owns_clipboard = False;

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
        ev.xselectionclear.selection = XmuttFONTGRID_CLIPBOARD(d);
        ev.xselectionclear.time = XtLastTimestampProcessed(d);
        XSendEvent(d, win, False, 0, &ev);
    }
}

static void
#ifndef _NO_PROTO
_XmuttFGridSetCellGeometry(XmuttFontGridWidget fw)
#else
_XmuttFGridSetCellGeometry(fw)
XmuttFontGridWidget fw;
#endif
{
    XFontStruct *idfnt;
    bdf_font_t *font;
    Dimension lw, lh;

    idfnt = fw->fgrid.idfnt;
    font = fw->fgrid.font;

    /*
     * Determine minimum cell width based on the longest label and the label
     * font.
     */
    lw = XTextWidth(idfnt, "177777", 6) + 6;
    fw->fgrid.label_height = lh = idfnt->max_bounds.ascent + 6;

    /*
     * Make sure the label height is even to force the labels to appear
     * vertically centered.
     */
    fw->fgrid.label_height = lh = lh + (lh & 1);

    /*
     * Determine the size of one font grid cell.
     */
    fw->fgrid.cell_width = font->bbx.width + 6;
    fw->fgrid.cell_height = font->bbx.height + 6;
    fw->fgrid.cell_width = MAX(fw->fgrid.cell_width, lw);
    fw->fgrid.cell_height = MAX(fw->fgrid.cell_height, lh);

    /*
     * Adjust the cell height.
     */
    fw->fgrid.cell_height += lh - 1;
}

static void
#ifndef _NO_PROTO
_XmuttFGridSetRowsCols(XmuttFontGridWidget fw, Boolean core)
#else
_XmuttFGridSetRowsCols(fw, core)
XmuttFontGridWidget fw;
Boolean core;
#endif
{
    int i;
    Display *d;
    Dimension dw, dh, wd, ht;

    /*
     * Set the threshold to 7/8 of the display width and height.
     */
    d = XtDisplay((Widget) fw);
    dw = (DisplayWidth(d, DefaultScreen(d)) * 7) >> 3;
    dh = (DisplayHeight(d, DefaultScreen(d)) * 7) >> 3;

    if (core == False) {
        /*
         * Adjust the rows and columns based on the preferred geometry.
         */
        wd = (fw->fgrid.cell_width * fw->fgrid.cell_cols) +
            _XmuttFGridHMargins(fw);
        ht = (fw->fgrid.cell_height * fw->fgrid.cell_rows) +
            _XmuttFGridVMargins(fw);

        if (wd > dw) {
            if ((fw->fgrid.cell_cols =
                 (dw - _XmuttFGridHMargins(fw)) / fw->fgrid.cell_width) == 0)
              fw->fgrid.cell_cols = 1;
        }

        if (fw->fgrid.power2 == True) {
            /*
             * Make sure the columns are a power of 2.
             */
            for (i = 15; i >= 0; i--) {
                if (fw->fgrid.cell_cols & (1 << i)) {
                    fw->fgrid.cell_cols = 1 << i;
                    break;
                }
            }
        }

        if (ht > dh) {
            if ((fw->fgrid.cell_rows =
                 (dh - _XmuttFGridVMargins(fw)) / fw->fgrid.cell_height) == 0)
              fw->fgrid.cell_rows = 1;
        }

        if (fw->fgrid.power2 == True) {
            /*
             * Make sure the rows are a power of 2.
             */
            for (i = 15; i >= 0; i--) {
                if (fw->fgrid.cell_rows & (1 << i)) {
                    fw->fgrid.cell_rows = 1 << i;
                    break;
                }
            }
        }
    } else {
        /*
         * Adjust the rows and columns to fit the core geometry.
         */
        if ((fw->fgrid.cell_cols = (fw->core.width - _XmuttFGridHMargins(fw)) /
             fw->fgrid.cell_width) == 0)
          fw->fgrid.cell_cols = 1;

        if (fw->fgrid.power2 == True) {
            /*
             * Make sure the columns are a power of 2.
             */
            for (i = 15; i >= 0; i--) {
                if (fw->fgrid.cell_cols & (1 << i)) {
                    fw->fgrid.cell_cols = 1 << i;
                    break;
                }
            }
        }

        if ((fw->fgrid.cell_rows =
             (fw->core.height - _XmuttFGridVMargins(fw)) /
             fw->fgrid.cell_height) == 0)
          fw->fgrid.cell_rows = 1;

        if (fw->fgrid.power2 == True) {
            /*
             * Make sure the rows are a power of 2.
             */
            for (i = 15; i >= 0; i--) {
                if (fw->fgrid.cell_rows & (1 << i)) {
                    fw->fgrid.cell_rows = 1 << i;
                    break;
                }
            }
        }
    }

    /*
     * Set the new pagesize based on the adjusted rows and columns.
     */
    fw->fgrid.pagesize = fw->fgrid.cell_rows * fw->fgrid.cell_cols;
}

static void
#ifndef _NO_PROTO
_XmuttFGridPreferredGeometry(XmuttFontGridWidget fw, Dimension *width,
                             Dimension *height)
#else
_XmuttFGridPreferredGeometry(fw, width, height)
XmuttFontGridWidget fw;
Dimension *width, *height;
#endif
{
    /*
     * Determine the desired width and height based on the cell size.
     */
    *width = (fw->fgrid.cell_width * fw->fgrid.cell_cols) +
        _XmuttFGridHMargins(fw);
    *height = (fw->fgrid.cell_height * fw->fgrid.cell_rows) +
        _XmuttFGridVMargins(fw);
}

static bdf_glyph_t *
#ifndef _NO_PROTO
_XmuttFGridCodeToGlyph(XmuttFontGridWidget fw, long code)
#else
_XmuttFGridCodeToGlyph(fw, code)
XmuttFontGridWidget fw;
long code;
#endif
{
    long l, r, m, nc;
    bdf_glyph_t *gl;
    bdf_font_t *font;

    font = fw->fgrid.font;

    if (code < 0 || font == 0)
      return 0;

    if ((fw->fgrid.unencoded == True && font->unencoded_used == 0) ||
        font->glyphs_used == 0)
      return 0;

    if (fw->fgrid.unencoded == True) {
        gl = font->unencoded;
        nc = font->unencoded_used;
    } else {
        gl = font->glyphs;
        nc = font->glyphs_used;
    }
    for (l = 0, r = nc - 1; l <= r; ) {
        m = (l + r) >> 1;
        if (gl[m].encoding < code)
          l = m + 1;
        else if (gl[m].encoding > code)
          r = m - 1;
        else
          return &gl[m];
    }
    return 0;
}

static void
#ifndef _NO_PROTO
_XmuttFGMakeGCs(Widget w, Boolean force)
#else
_XmuttFGMakeGCs(w, force)
Widget w;
Boolean force;
#endif
{
    XmuttFontGridWidget fw;
    XtGCMask gcvm;
    XGCValues gcv;

    fw = (XmuttFontGridWidget) w;

    gcvm = GCForeground|GCBackground|GCFunction|GCFont;
    gcv.font = fw->fgrid.idfnt->fid;

    if (fw->fgrid.gc == 0 || force == True) {
        if (fw->fgrid.gc != 0)
          XFreeGC(XtDisplay(w), fw->fgrid.gc);

        gcv.foreground = fw->primitive.foreground;
        gcv.background = fw->core.background_pixel;
        gcv.function = GXcopy;

        fw->fgrid.gc = XCreateGC(XtDisplay(w), XtWindow(w), gcvm, &gcv);
    }

    if (fw->fgrid.invgc == 0 || force == True) {
        if (fw->fgrid.invgc != 0)
          XFreeGC(XtDisplay(w), fw->fgrid.invgc);

        gcv.background = fw->primitive.foreground;
        gcv.foreground = fw->core.background_pixel;
        gcv.function = GXcopy;

        fw->fgrid.invgc = XCreateGC(XtDisplay(w), XtWindow(w), gcvm, &gcv);
    }
}

static bdf_glyph_t *
#ifndef _NO_PROTO
_XmuttFontGridInitialGlyph(XmuttFontGridWidget fw, long code)
#else
_XmuttFontGridInitialGlyph(fw, code)
XmuttFontGridWidget fw;
long code;
#endif
{
    long l, r, m, nc;
    bdf_glyph_t *gl;
    bdf_font_t *font;

    font = fw->fgrid.font;

    if (code < 0 || font == 0)
      return 0;

    if ((fw->fgrid.unencoded == True && font->unencoded_used == 0) ||
        font->glyphs_used == 0)
      return 0;

    if (fw->fgrid.unencoded == True) {
        gl = font->unencoded;
        nc = font->unencoded_used;
    } else {
        gl = font->glyphs;
        nc = font->glyphs_used;
    }
    for (l = m = 0, r = nc - 1; l < r; ) {
        m = (l + r) >> 1;
        if (gl[m].encoding < code)
          l = m + 1;
        else if (gl[m].encoding > code)
          r = m - 1;
        else
          break;
    }

    /*
     * Go back until we hit the beginning of the glyphs or until
     * we find the glyph with a code less than the specified code.
     */
    while (m > 0 && gl[m].encoding > code)
      m--;

    /*
     * Look forward if necessary.
     */
    while (m < nc && gl[m].encoding < code)
      m++;

    return (m < nc) ? &gl[m] : 0;
}

/*
 * Generate a list of points that can be drawn for selected glyphs and
 * glyphs that have one bit per pixel.
 */
static void
#ifndef _NO_PROTO
_XmuttFGridGlyphPointList(XmuttFontGridWidget fw, int x, int y,
                          int rx, int by, bdf_glyph_t *glyph)
#else
_XmuttFGridGlyphPointList(fw, x, y, rx, by, glyph)
XmuttFontGridWidget fw;
int x, y, rx, by;
bdf_glyph_t *glyph;
#endif
{
    int i, j, bpr, col;
    unsigned char *bmap, *masks;

    masks = 0;

    switch (fw->fgrid.bpp) {
      case 1: masks = onebpp; break;
      case 2: masks = twobpp; break;
      case 4: masks = fourbpp; break;
    }

    /*
     * Don't forget the x_offset when positioning the glyph in the Fgrid.
     */
    x += glyph->bbx.x_offset;

    fw->fgrid.gpoints_used = 0;
    bmap = glyph->bitmap;
    bpr = ((glyph->bbx.width * fw->fgrid.bpp) + 7) >> 3;

    for (i = 0; i + y - glyph->bbx.ascent < by && i < glyph->bbx.height; i++) {
        for (col = j = 0; j + x < rx && j < glyph->bbx.width;
             j++, col += fw->fgrid.bpp) {
            if (bmap[(i * bpr) + (col >> 3)] &
                masks[(col & 7) / fw->fgrid.bpp]) {
                if (fw->fgrid.gpoints_used == fw->fgrid.gpoints_size) {
                    if (fw->fgrid.gpoints_size == 0)
                      fw->fgrid.gpoints =
                          (XPoint *) XtMalloc(sizeof(XPoint) * 128);
                    else
                      fw->fgrid.gpoints =
                          (XPoint *) XtRealloc((char *) fw->fgrid.gpoints,
                                               sizeof(XPoint) *
                                               (fw->fgrid.gpoints_size + 128));
                    fw->fgrid.gpoints_size += 128;
                }

                fw->fgrid.gpoints[fw->fgrid.gpoints_used].x = j + x;
                fw->fgrid.gpoints[fw->fgrid.gpoints_used].y =
                    i + y - glyph->bbx.ascent;
                fw->fgrid.gpoints_used++;
            }
        }
    }
}

/*
 * Generate a list of points that can be drawn for glyphs with 2 or 4 bits per
 * pixel.
 */
static void
#ifndef _NO_PROTO
_XmuttFGridGlyphPointListN(XmuttFontGridWidget fw, int x, int y,
                           int rx, int by, int cidx, bdf_glyph_t *glyph)
#else
_XmuttFGridGlyphPointListN(fw, x, y, rx, by, cidx, glyph)
XmuttFontGridWidget fw;
int x, y, rx, by, cidx;
bdf_glyph_t *glyph;
#endif
{
    int i, j, bpr, col, byte, di, si;
    unsigned char *bmap, *masks;

    di = 0;
    masks = 0;
    
    switch (fw->fgrid.bpp) {
      case 1: masks = onebpp; di = 7; break;
      case 2: masks = twobpp; di = 3; break;
      case 4: masks = fourbpp; di = 1; break;
    }

    fw->fgrid.gpoints_used = 0;
    bmap = glyph->bitmap;
    bpr = ((glyph->bbx.width * fw->fgrid.bpp) + 7) >> 3;

    for (i = 0; i + y - glyph->bbx.ascent < by && i < glyph->bbx.height; i++) {
        for (col = j = 0; j + x < rx && j < glyph->bbx.width;
             j++, col += fw->fgrid.bpp) {
            si = (col & 7) / fw->fgrid.bpp;
            byte = bmap[(i * bpr) + (col >> 3)] & masks[si];
            if (byte) {
                /*
                 * Check to see if the byte matches the color index being
                 * collected.
                 */
                if (di > si)
                  byte >>= (di - si) * fw->fgrid.bpp;

                if (byte == cidx) {
                    if (fw->fgrid.gpoints_used == fw->fgrid.gpoints_size) {
                        if (fw->fgrid.gpoints_size == 0)
                          fw->fgrid.gpoints =
                              (XPoint *) XtMalloc(sizeof(XPoint) * 64);
                        else
                          fw->fgrid.gpoints =
                              (XPoint *) XtRealloc((char *) fw->fgrid.gpoints,
                                               sizeof(XPoint) *
                                               (fw->fgrid.gpoints_size + 64));
                        fw->fgrid.gpoints_size += 64;
                    }

                    fw->fgrid.gpoints[fw->fgrid.gpoints_used].x = j + x;
                    fw->fgrid.gpoints[fw->fgrid.gpoints_used].y =
                        i + y - glyph->bbx.ascent;
                    fw->fgrid.gpoints_used++;
                }
            }
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttFGridDrawCells(XmuttFontGridWidget fw, long start, long end,
                     Boolean labels, Boolean glyphs)
#else
_XmuttFGridDrawCells(fw, start, end, labels, glyphs)
XmuttFontGridWidget fw;
long start, end;
Boolean labels, glyphs;
#endif
{
    Widget w;
    unsigned short n, si, ei, changed, li;
    long i, ng;
    int llen, dir, as, ds, lx, ly;
    bdf_glyph_t *glyph, *glyph_end;
    bdf_font_t *font;
    _XmuttFGridPageInfo *pi;
    Position x, y, rx, by;
    Dimension r, c;
    Boolean mod;
    XRectangle rect;
    XCharStruct ov;
    char nbuf[10];

    w = (Widget) fw;

    if (!XtIsRealized(w) || (labels == False && glyphs == False))
      return;

    font = fw->fgrid.font;

    /*
     * Get the active page info.
     */
    glyph = 0;
    ng = 0;
    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        if (font) {
            glyph = font->glyphs;
            ng = font->glyphs_used;
        }
    } else {
        /*
         * If we are viewing the unencoded, make all labels a -1.
         */
        sprintf(nbuf, "-1");

        pi = &fw->fgrid.upage;
        if (font) {
            glyph = font->unencoded;
            ng = font->unencoded_used;
        }
    }

    /*
     * Determine the initial code to work from.
     */
    n = pi->bcode;

    /*
     * Skip to the specified starting glyph.
     */
    glyph_end = glyph + ng;
    if ((glyph = _XmuttFontGridInitialGlyph(fw, start)) == 0)
      ng = 0;

    for (i = start; i <= end; i++) {
        /*
         * Only draw those cells that are on the current page.
         */
        if (i < pi->bcode || i >= pi->bcode + fw->fgrid.pagesize)
          continue;

        if (fw->fgrid.orientation == XmHORIZONTAL) {
            r = (i - n) / fw->fgrid.cell_cols;
            c = (i - n) % fw->fgrid.cell_cols;
        } else {
            c = (i - n) / fw->fgrid.cell_rows;
            r = (i - n) % fw->fgrid.cell_rows;
        }
        x = _XmuttFGridLeftX(fw) + (c * fw->fgrid.cell_width) + fw->fgrid.xoff;
        y = _XmuttFGridTopY(fw) + (r * fw->fgrid.cell_height) + fw->fgrid.yoff;

        if (labels == True) {
            /*
             * Format the label code.
             */
            if (fw->fgrid.unencoded == False) {
                switch (fw->fgrid.base) {
                  case 8: sprintf(nbuf, "%lo", i); break;
                  case 10: sprintf(nbuf, "%ld", i); break;
                  case 16: sprintf(nbuf, "%lX", i); break;
                }
            }

            rect.x = x + 1;
            rect.y = y + 1;
            rect.width = fw->fgrid.cell_width - 2;
            rect.height = fw->fgrid.label_height - 2;
            XDrawRectangle(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                           rect.x, rect.y, rect.width, rect.height);

            llen = strlen(nbuf);
            XTextExtents(fw->fgrid.idfnt, nbuf, llen, &dir, &as, &ds, &ov);
            lx = x + ((fw->fgrid.cell_width >> 1) - (ov.width >> 1));
            ly = y + ((fw->fgrid.label_height >> 1) -
                      ((ov.ascent + ov.descent) >> 1)) + ov.ascent;

            if (i <= 0xffff) {
                if (fw->fgrid.unencoded == False)
                  mod = bdf_glyph_modified(font, i, 0) ? True : False;
                else
                  mod = bdf_glyph_modified(font, i, 1) ? True : False;
            } else
              mod = False;

            XClearArea(XtDisplay(w), XtWindow(w), rect.x + 1, rect.y + 1,
                       rect.width - 1, rect.height - 1, False);

            /*
             * Draw the label appropriately.
             */
            if (fw->fgrid.unencoded == False && mod == True) {
                XFillRectangle(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                               rect.x + 2, rect.y + 2,
                               rect.width - 3, rect.height - 3);
                XDrawString(XtDisplay(w), XtWindow(w), fw->fgrid.invgc,
                            lx, ly, nbuf, llen);
            } else {
                if (_XmuttFGridCodeToGlyph(fw, i))
                  XDrawRectangle(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                                 rect.x + 1, rect.y + 1,
                                 rect.width - 2, rect.height - 2);

                XDrawString(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                            lx, ly, nbuf, llen);
            }
        }

        if (glyphs == True) {
            rect.x = x + 1;
            rect.y = y + fw->fgrid.label_height + 1;
            rect.width = fw->fgrid.cell_width - 2;
            rect.height = (fw->fgrid.cell_height - fw->fgrid.label_height) - 2;

            if (i <= 0xffff &&
                ng > 0 && glyph < glyph_end && glyph->encoding == i) {
                /*
                 * Draw the glyph.
                 */

                /*
                 * Set the upper left position before generating the list of
                 * glyph points.
                 */
                rx = x + fw->fgrid.cell_width - 2;
                by = y + fw->fgrid.cell_height - 2;

                /*
                 * Adjust upper left position before generating the list of
                 * glyph points.
                 */
                x += (fw->fgrid.cell_width >> 1) -
                    ((font->bbx.width + font->bbx.x_offset) >> 1) + 1;
                y += fw->fgrid.label_height + font->bbx.ascent + 3;

                if (IsSelected(glyph->encoding, pi->selmap)) {
                    XFillRectangle(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                                   rect.x + 1, rect.y + 1,
                                   rect.width - 1, rect.height - 1);
                    if (glyph->bytes > 0) {
                        _XmuttFGridGlyphPointList(fw, x, y, rx, by, glyph);
                        XDrawPoints(XtDisplay(w), XtWindow(w), fw->fgrid.invgc,
                                    fw->fgrid.gpoints, fw->fgrid.gpoints_used,
                                    CoordModeOrigin);
                    }
                } else {
                    XClearArea(XtDisplay(w), XtWindow(w), rect.x, rect.y,
                               rect.width, rect.height, False);

                    if (glyph->bytes > 0) {
                        if (fw->fgrid.bpp == 1) {
                            _XmuttFGridGlyphPointList(fw, x, y, rx, by, glyph);
                            XDrawPoints(XtDisplay(w), XtWindow(w),
                                        fw->fgrid.gc,
                                        fw->fgrid.gpoints,
                                        fw->fgrid.gpoints_used,
                                        CoordModeOrigin);
                        } else {
                            if (fw->fgrid.bpp == 2) {
                                si = 0;
                                ei = 4;
                            } else {
                                si = 4;
                                ei = 20;
                            }
                            for (changed = 0, li = si; li < ei; li++) {
                                _XmuttFGridGlyphPointListN(fw, x, y, rx, by,
                                                           li - si, glyph);
                                if (fw->fgrid.gpoints_used > 0) {
                                    changed = 1;
                                    XSetForeground(XtDisplay(w),
                                                   fw->fgrid.gc,
                                                   fw->fgrid.colors[li]);
                                    XDrawPoints(XtDisplay(w), XtWindow(w),
                                                fw->fgrid.gc,
                                                fw->fgrid.gpoints,
                                                fw->fgrid.gpoints_used,
                                                CoordModeOrigin);
                                }
                            }

                            /*
                             * Restore the original foreground if it was
                             * changed to draw a glyph.
                             */
                            if (changed)
                              XSetForeground(XtDisplay(w), fw->fgrid.gc,
                                             fw->primitive.foreground);
                        }
                    }
                }

                glyph++;
            } else {
                /*
                 * Clear the empty cell.
                 */
                if (i <= 0xffff && IsSelected(i, pi->selmap))
                  XFillRectangle(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                                 rect.x + 1, rect.y + 1,
                                 rect.width - 1, rect.height - 1);
                else {
                    XClearArea(XtDisplay(w), XtWindow(w), rect.x, rect.y,
                               rect.width, rect.height, False);
                    if (i > 0xffff) {
                        /*
                         * Draw an X through the cell.
                         */
                        XDrawLine(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                                  rect.x , rect.y,
                                  rect.x + rect.width, rect.y + rect.height);
                        XDrawLine(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                                  rect.x + rect.width, rect.y,
                                  rect.x, rect.y + rect.height);
                    }
                }
            }
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttFGridRedrawGrid(XmuttFontGridWidget fw)
#else
_XmuttFGridRedrawGrid(fw)
XmuttFontGridWidget fw;
#endif
{
    Widget w;
    long start, end;
    int i, x, y;
    Dimension pmargin, lh, wd, ht;
    XRectangle rect;

    w = (Widget) fw;

    if (!XtIsRealized(w))
      return;

    pmargin = _XmuttFGridPMargins(fw) >> 1;
    rect.width = fw->fgrid.cell_width * fw->fgrid.cell_cols;
    rect.height = fw->fgrid.cell_height * fw->fgrid.cell_rows;

    /*
     * Recalculate the X and Y offsets needed to center the grid.
     */
    wd = fw->core.width - _XmuttFGridPMargins(fw);
    ht = fw->core.height - _XmuttFGridPMargins(fw);

    fw->fgrid.xoff = (wd >> 1) - (rect.width >> 1);
    fw->fgrid.yoff = (ht >> 1) - (rect.height >> 1);

    rect.x = fw->fgrid.xoff + pmargin + 2;
    rect.y = fw->fgrid.yoff + pmargin + 2;

    lh = fw->fgrid.label_height;

    x = rect.x;
    y = rect.y;
    for (i = 0; i < fw->fgrid.cell_rows; i++) {
        XDrawLine(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                  x, y, x + rect.width, y);
        XDrawLine(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                  x, y + lh, x + rect.width, y + lh);
        y += fw->fgrid.cell_height;
    }

    /*
     * Draw the bottom line.
     */
    XDrawLine(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
              x, y, x + rect.width, y);

    x = rect.x;
    y = rect.y;
    for (i = 0; i < fw->fgrid.cell_cols + 1; i++) {
        XDrawLine(XtDisplay(w), XtWindow(w), fw->fgrid.gc,
                  x, y, x, y + rect.height);
        x += fw->fgrid.cell_width;
    }

    start = (fw->fgrid.unencoded == False) ?
        fw->fgrid.npage.bcode : fw->fgrid.upage.bcode;
    end = start + (fw->fgrid.pagesize - 1);
    _XmuttFGridDrawCells(fw, start, end, True, True);
}

static void
#ifndef _NO_PROTO
_XmuttFGridSelectRange(XmuttFontGridWidget fw, long start, long end)
#else
_XmuttFGridSelectRange(fw, start, end)
XmuttFontGridWidget fw;
long start, end;
#endif
{
    long i, tmp;
    _XmuttFGridPageInfo *pi;

    if (start > end) {
        tmp = start;
        start = end;
        end = tmp;
    }

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    for (i = start; i <= end; i++)
      Select(i, pi->selmap);

    /*
     * Adjust the start and end values to the current page to determine which
     * cells need to be redrawn.
     */
    tmp = pi->bcode + (fw->fgrid.pagesize - 1);
    if (start >= tmp || end < pi->bcode)
      return;

    if (start < pi->bcode)
      start = pi->bcode;
    if (end > tmp)
      end = tmp;
    _XmuttFGridDrawCells(fw, start, end, False, True);
}

static void
#ifndef _NO_PROTO
_XmuttFGridDeselectRange(XmuttFontGridWidget fw, long start, long end)
#else
_XmuttFGridDeselectRange(fw, start, end)
XmuttFontGridWidget fw;
long start, end;
#endif
{
    long i, tmp;
    _XmuttFGridPageInfo *pi;

    if (start > end) {
        tmp = start;
        start = end;
        end = tmp;
    }

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    for (i = start; i <= end; i++) {
        if (IsSelected(i, pi->selmap)) {
            Unselect(i, pi->selmap);
            if (i >= pi->bcode && i <= pi->bcode + (fw->fgrid.pagesize - 1))
              _XmuttFGridDrawCells(fw, i, i, False, True);
        }
    }
}

static void
#ifndef _NO_PROTO
_XmuttFGridDeselectAll(XmuttFontGridWidget fw)
#else
_XmuttFGridDeselectAll(fw)
XmuttFontGridWidget fw;
#endif
{
    _XmuttFGridPageInfo *pi, *opi;

    if (fw->fgrid.unencoded == True) {
        pi = &fw->fgrid.upage;
        opi = &fw->fgrid.npage;
    } else {
        pi = &fw->fgrid.npage;
        opi = &fw->fgrid.upage;
    }

    if (pi->sel_start != -1 || pi->sel_end != -1)
      _XmuttFGridDeselectRange(fw, pi->bcode,
                               pi->bcode + fw->fgrid.pagesize - 1);
    else if (opi->sel_start != -1 || opi->sel_end != -1)
      _XmuttFGridDeselectRange(fw, opi->bcode,
                               opi->bcode + fw->fgrid.pagesize - 1);

    /*
     * Now clear the selected bitmaps.
     */
    (void) memset((char *) pi->selmap, 0, sizeof(unsigned long) * 2048);
    (void) memset((char *) opi->selmap, 0, sizeof(unsigned long) * 2048);

    /*
     * Reset the selection start and end points.
     */
    pi->sel_start = pi->sel_end = opi->sel_start = opi->sel_end = -1;
}

static void
#ifndef _NO_PROTO
_XmuttFGridDefaultHResolution(Widget w, int offset, XrmValue *val)
#else
_XmuttFGridDefaultHResolution(w, offset, val)
Widget w;
int offset;
XrmValue *val;
#endif
{
    Display *d;
    XmuttFontGridWidget fw;

    d = XtDisplay(w);
    fw = (XmuttFontGridWidget) w;

    /*
     * Determine the default horizontal resolution of this display.
     */
    fw->fgrid.defhres =
        (long) (((((double) DisplayWidth(d, DefaultScreen(d))) * 25.4) /
                 ((double) DisplayWidthMM(d, DefaultScreen(d)))) + 0.5);

    val->size = sizeof(long);
    val->addr = (XPointer) &fw->fgrid.defhres;
}

static void
#ifndef _NO_PROTO
_XmuttFGridDefaultVResolution(Widget w, int offset, XrmValue *val)
#else
_XmuttFGridDefaultVResolution(w, offset, val)
Widget w;
int offset;
XrmValue *val;
#endif
{
    Display *d;
    XmuttFontGridWidget fw;

    d = XtDisplay(w);
    fw = (XmuttFontGridWidget) w;

    /*
     * Determine the default vertical resolution of this display.
     */
    fw->fgrid.defvres =
        (long) (((((double) DisplayHeight(d, DefaultScreen(d))) * 25.4) /
                 ((double) DisplayHeightMM(d, DefaultScreen(d)))) + 0.5);

    val->size = sizeof(long);
    val->addr = (XPointer) &fw->fgrid.defvres;
}

/*
 * Routine to do a binary search for the pages preceding and following
 * the specified page.
 */
static void
#ifndef _NO_PROTO
_XmuttFontGridNeighborPages(XmuttFontGridWidget fw, long page, long *prev,
                            long *next)
#else
_XmuttFontGridNeighborPages(fw, page, prev, next)
XmuttFontGridWidget fw;
long page, *prev, *next;
#endif
{
    long bcode, nglyphs, l, r, m;
    bdf_glyph_t *glyphs;
    bdf_font_t *font;
    _XmuttFGridPageInfo *pip;

    font = fw->fgrid.font;

    pip = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : &fw->fgrid.upage;

    /*
     * Always make sure navigation works for all pages in empty fonts.
     */
    if (fw->fgrid.noblanks == False ||
        (fw->fgrid.unencoded == False && fw->fgrid.font->glyphs_used == 0)) {
        *prev = page - 1;
        *next = (page < pip->maxpage) ? page + 1 : -1;
        return;
    }

    /*
     * Determine the base code of the specified page.
     */
    bcode = page * fw->fgrid.pagesize;

    if (fw->fgrid.unencoded == False) {
        glyphs = font->glyphs;
        nglyphs = font->glyphs_used;
    } else {
        glyphs = font->unencoded;
        nglyphs = font->unencoded_used;
    }

    /*
     * Do a binary search to find the the preceding page number.
     */
    for (l = m = 0, r = nglyphs - 1; l < r; ) {
        m = (l + r) >> 1;
        if (glyphs[m].encoding < bcode)
          l = m + 1;
        else if (glyphs[m].encoding > bcode)
          r = m - 1;
        else {
            /*
             * Exact match.
             */
            l = r = m - 1;
            break;
        }
    }

    /*
     * In case the search ends on a code in the specified page.
     */
    while (r >= 0 && glyphs[r].encoding >= bcode)
      r--;

    /*
     * Set the previous page code.
     */
    *prev = (r >= 0) ? glyphs[r].encoding / fw->fgrid.pagesize : -1;

    /*
     * Determine the following page code.
     */
    if (r < 0)
      r = 0;
    while (r < nglyphs && glyphs[r].encoding < bcode + fw->fgrid.pagesize)
      r++;

    *next = (r < nglyphs) ? glyphs[r].encoding / fw->fgrid.pagesize : -1;
}

/*************************************************************************
 *
 * Start class methods.
 *
 *************************************************************************/

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
    XmuttFontGridWidget fw = (XmuttFontGridWidget) newone;
    long i, boundary;
    Dimension wd, ht;
    String appname, appclass;
    bdf_font_t *font;
    bdf_glyph_t *glyphs;
    _XmuttFGridPageInfo *pi;

    /*
     * Load the font used for printing the code of the character.
     */
    if ((fw->fgrid.idfnt = XLoadQueryFont(XtDisplay(newone), "6x10")) == 0) {
        fprintf(stderr, "XmuttFontGrid: can't load the \"6x10\" font.\n");
        exit(1);
    }

    fw->fgrid.gc = 0;
    fw->fgrid.invgc = 0;
    fw->fgrid.gpoints = 0;
    fw->fgrid.gpoints_size = 0;
    fw->fgrid.gpoints_used = 0;

    font = fw->fgrid.font;

    if (font == 0) {
        /*
         * Create a new font.
         */
        fw->fgrid.font = font =
            bdf_new_font(0, fw->fgrid.ptsize, fw->fgrid.hres,
                         fw->fgrid.vres, fw->fgrid.spacing, fw->fgrid.bpp);

        /*
         * Add an XLFD name to it.
         */
        XtGetApplicationNameAndClass(XtDisplay((Widget) fw),
                                     &appname, &appclass);
        font->name = bdf_make_xlfd_name(font, appname, "Unknown");
    }

    /*
     * Set the bits-per-pixel from the font.
     */
    fw->fgrid.bpp = font->bpp;

    /*
     * Make sure the number of rows and columns are within reasonable
     * initial bounds.
     */
    fw->fgrid.cell_cols = MIN(fw->fgrid.cell_cols, FGRID_MAX_COLS);
    fw->fgrid.cell_rows = MIN(fw->fgrid.cell_rows, FGRID_MAX_ROWS);

    /*
     * Swap the rows and columns if they are the defaults and the orientation
     * is vertical.
     */
    if (fw->fgrid.orientation == XmVERTICAL &&
        fw->fgrid.cell_cols == FGRID_DEFAULT_COLS &&
        fw->fgrid.cell_rows == FGRID_DEFAULT_ROWS) {
        fw->fgrid.cell_cols = FGRID_DEFAULT_ROWS;
        fw->fgrid.cell_rows = FGRID_DEFAULT_COLS;
    }

    /*
     * If the rows or columns are 0, set them to the default values.
     */
    if (fw->fgrid.cell_cols == 0)
      fw->fgrid.cell_cols =
          (fw->fgrid.orientation == XmHORIZONTAL) ?
          FGRID_DEFAULT_COLS : FGRID_DEFAULT_ROWS;
    if (fw->fgrid.cell_rows == 0)
      fw->fgrid.cell_rows =
          (fw->fgrid.orientation == XmHORIZONTAL) ?
          FGRID_DEFAULT_ROWS : FGRID_DEFAULT_COLS;

    /*
     * Determine the geometry of one grid cell.
     */
    _XmuttFGridSetCellGeometry(fw);

    /*
     * Determine if the rows and columns need to be adjusted if the geometry
     * exceeds the maximum threshold dimensions.
     */
    _XmuttFGridSetRowsCols(fw, False);

    /*
     * Set the count which specifies the number of times a cursor motion is
     * done.
     */
    fw->fgrid.count = 0;

    /*
     * Get the initial geometry.
     */
    _XmuttFGridPreferredGeometry(fw, &wd, &ht);
    fw->core.width = wd;
    fw->core.height = ht;

    /*
     * Initialize the page info for encoded and unencoded.
     */
    pi = &fw->fgrid.upage;
    pi->minpage = 0;
    pi->maxpage = 0xffff / fw->fgrid.pagesize;
    pi->npage = pi->ppage = -1;
    pi->pageno = pi->bcode = 0;
    pi->sel_start = pi->sel_end = -1;
    (void) memset((char *) pi->selmap, 0, sizeof(unsigned long) * 2048);
    (void) memcpy((char *) &fw->fgrid.npage, (char *) pi,
                  sizeof(_XmuttFGridPageInfo));

    /*
     * Determine unencoded page info.
     */
    if (font->unencoded_used > 0) {
        pi = &fw->fgrid.upage;
        if (fw->fgrid.noblanks == True) {
            glyphs = font->unencoded;

            /*
             * Determine the minimum page based on the encoding
             * of the first glyph and set the current page.
             */
            pi->pageno = pi->minpage =
                glyphs->encoding / fw->fgrid.pagesize;
            /*
             * Determine the maximum page based on the encoding of the
             * last glyph.
             */
            glyphs = font->unencoded + (font->unencoded_used - 1);
            pi->maxpage = glyphs->encoding / fw->fgrid.pagesize;

            /*
             * Set the base code for the current page.
             */
            pi->bcode = pi->pageno * fw->fgrid.pagesize;

            /*
             * Determine the previous and next pages.
             */
            pi->ppage = -1;

            /*
             * The lower boundary for the next page.
             */
            boundary = pi->bcode + fw->fgrid.pagesize;
            for (i = 0, glyphs = font->unencoded;
                 i < font->unencoded_used && glyphs->encoding < boundary;
                 i++, glyphs++) ;

            if (i == font->unencoded_used)
              pi->npage = -1;
            else
              pi->npage = glyphs->encoding / fw->fgrid.pagesize;
        } else {
            /*
             * Set the min and max pages to all.
             */
            pi->pageno = pi->minpage = 0;
            pi->maxpage = 0xffff / fw->fgrid.pagesize;

            /*
             * Set the previous and next pages.
             */
            pi->ppage = -1;
            pi->npage = pi->pageno + 1;
        }
    }

    if (fw->fgrid.initial_glyph == -1)
      fw->fgrid.initial_glyph = (font->glyphs_used > 0) ?
          font->glyphs->encoding : 0;

    pi = &fw->fgrid.npage;

    /*
     * Set the current page number from the initial_glyph.
     */
    pi->pageno = fw->fgrid.initial_glyph / fw->fgrid.pagesize;
    pi->bcode = pi->pageno * fw->fgrid.pagesize;

    /*
     * Make sure the initial glyph is selected.
     */
    pi->sel_start = pi->sel_end = fw->fgrid.initial_glyph;
    Select(fw->fgrid.initial_glyph, pi->selmap);

    _XmuttFontGridNeighborPages(fw, pi->pageno, &pi->ppage, &pi->npage);

    /*
     * Set the page info for the encoded font glyphs.
     */
    if (font->glyphs_used > 0) {
        /*
         * Set the minimum and maximum pages.
         */
        if (fw->fgrid.noblanks == True) {
            /*
             * Determine the minimum page based on the encoding
             * of the first glyph and set the current page.
             */
            pi->minpage = font->glyphs->encoding / fw->fgrid.pagesize;

            /*
             * Determine the maximum page based on the encoding of the
             * last glyph.
             */
            glyphs = font->glyphs + (font->glyphs_used - 1);
            pi->maxpage = glyphs->encoding / fw->fgrid.pagesize;
        } else {
            /*
             * Set the min and max pages to all.
             */
            pi->minpage = 0;
            pi->maxpage = 0xffff / fw->fgrid.pagesize;
        }
    }

    fw->fgrid.owns_clipboard = False;
    fw->fgrid.unencoded = False;
    fw->fgrid.hasfocus = False;
    fw->fgrid.resizing = False;
    fw->fgrid.clearwin = False;
    fw->fgrid.client_page = True;
    fw->fgrid.no_sel_callback = False;
    fw->fgrid.from_keyboard = False;

    fw->fgrid.mclick_time = 400;
    fw->fgrid.last_click = 0;

    /*
     * Make sure the base for the character code is in limits.
     */
    if (fw->fgrid.base != 8 && fw->fgrid.base != 10 && fw->fgrid.base != 16)
      fw->fgrid.base = 16;

    /*
     * Initialize the character clipboard.
     */
    (void) memset((char *) &fw->fgrid.clipboard, 0, sizeof(bdf_glyphlist_t));
}

static void
#ifndef _NO_PROTO
Resize(Widget w)
#else
Resize(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    long pageno;
    bdf_glyph_t *glyph;
    bdf_font_t *font;
    _XmuttFGridPageInfo *pi;

    /*
     * Adjust the cell rows and columns to the new core size.
     */
    fw = (XmuttFontGridWidget) w;
    font = fw->fgrid.font;
    fw->fgrid.resizing = True;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;

    _XmuttFGridSetRowsCols((XmuttFontGridWidget) w, True);

    /*
     * Determine which page we want to make sure the base code of the
     * page before resize is always displayed.
     */
    pageno = pi->bcode / fw->fgrid.pagesize;

    /*
     * Adjust the page info to reflect the modified number of rows and columns.
     */
    if (font->unencoded_used > 0) {
        pi = &fw->fgrid.upage;
        if (fw->fgrid.noblanks == True) {
            glyph = font->unencoded;
            pi->minpage = glyph->encoding / fw->fgrid.pagesize;
            glyph = font->unencoded + (font->unencoded_used - 1);
            pi->maxpage = glyph->encoding / fw->fgrid.pagesize;
        } else {
            pi->minpage = 0;
            pi->maxpage = 0xffff / fw->fgrid.pagesize;
        }
    }
    if (font->glyphs_used > 0) {
        pi = &fw->fgrid.npage;
        if (fw->fgrid.noblanks == True) {
            glyph = font->glyphs;
            pi->minpage = glyph->encoding / fw->fgrid.pagesize;
            glyph = font->glyphs + (font->glyphs_used - 1);
            pi->maxpage = glyph->encoding / fw->fgrid.pagesize;
        } else {
            pi->minpage = 0;
            pi->maxpage = 0xffff / fw->fgrid.pagesize;
        }
    }

    XmuttFontGridGotoPage(w, pageno);

    /*
     * Always do a redraw here because Motif does not in some cases.
     */
    if (XtIsRealized(w)) {
        XClearWindow(XtDisplay(w), XtWindow(w));
        _XmuttFGridHighlightWindow(w);
        _XmuttFGridShadowWindow(fw);
        _XmuttFGridRedrawGrid(fw);
    }

    /*fw->fgrid.clearwin = True;*/

    fw->fgrid.resizing = False;
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
    /*
     * Force NorthWest bit gravity and SaveUnder's so redrawing looks a lot
     * smoother.
     */
    *valueMask |= CWBitGravity | CWSaveUnder;
    attributes->bit_gravity = NorthWestGravity;
    attributes->save_under = True;

    XtCreateWindow(w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
                   *valueMask, attributes);

    _XmuttFGMakeGCs(w, False);
}

static void
#ifndef _NO_PROTO
Destroy(Widget w)
#else
Destroy(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    unsigned long i;
    bdf_glyph_t *gp;
    bdf_glyphlist_t *gl;

    fw = (XmuttFontGridWidget) w;

    /*
     * Free up the internal glyph list clipboard if necessary.
     */
    gl = &fw->fgrid.clipboard;
    for (i = 0, gp = gl->glyphs; i < gl->glyphs_used; i++, gp++) {
        if (gp->name != 0)
          free(gp->name);
        if (gp->bytes > 0)
          free((char *) gp->bitmap);
    }
    if (gl->glyphs_size > 0)
      free((char *) gl->glyphs);
    gl->glyphs_size = gl->glyphs_used = 0;

    if (fw->fgrid.gpoints_size > 0)
      XtFree((char *) fw->fgrid.gpoints);
    fw->fgrid.gpoints_size = fw->fgrid.gpoints_used = 0;

    if (fw->fgrid.font != 0)
      bdf_free_font(fw->fgrid.font);
    fw->fgrid.font = 0;

    if (fw->fgrid.idfnt != 0)
      XFreeFont(XtDisplay(w), fw->fgrid.idfnt);
    fw->fgrid.idfnt = 0;

    if (fw->fgrid.gc != 0)
      XFreeGC(XtDisplay(w), fw->fgrid.gc);
    if (fw->fgrid.invgc != 0)
      XFreeGC(XtDisplay(w), fw->fgrid.invgc);
    fw->fgrid.gc = fw->fgrid.invgc = 0;
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
    XmuttFontGridWidget fw;

    fw = (XmuttFontGridWidget) w;

    if (fw->fgrid.clearwin == True) {
        /*
         * When a resize occurs as a result of the font bounding box changing
         * size, the window needs to be cleared because of the NorthWest
         * gravity.
         */
        fw->fgrid.clearwin = False;
        if (XtIsRealized(w))
          XClearWindow(XtDisplay(w), XtWindow(w));
    }

    _XmuttFGridHighlightWindow(w);
    _XmuttFGridShadowWindow(fw);
    _XmuttFGridRedrawGrid(fw);
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
    XmuttFontGridWidget cfw, nfw;
    long pageno;
    Dimension wd, ht;
    bdf_font_t *font;
    bdf_glyph_t *glyphs;
    _XmuttFGridPageInfo *pi;
    Boolean redraw;

    cfw = (XmuttFontGridWidget) curr;
    nfw = (XmuttFontGridWidget) newone;

    font = 0;

    redraw = False;

    if (nfw->fgrid.font != cfw->fgrid.font ||
        nfw->fgrid.orientation != cfw->fgrid.orientation) {
        redraw = True;
        if (nfw->fgrid.font != cfw->fgrid.font) {
            bdf_free_font(cfw->fgrid.font);

            if ((font = nfw->fgrid.font) == 0)
              nfw->fgrid.font = font =
                  bdf_new_font(0, nfw->fgrid.ptsize, nfw->fgrid.hres,
                               nfw->fgrid.vres, nfw->fgrid.spacing,
                               nfw->fgrid.bpp);
            else if (font->glyphs_used == 0 && font->unencoded_used == 0) {
                /*
                 * Set the default metrics in case the font loaded successfully
                 * but has no glyphs for some reason.
                 */
                bdf_set_default_metrics(font);
                bdf_update_name_from_properties(font);
            }

            /*
             * Set the bits-per-pixel from the font.
             */
            nfw->fgrid.bpp = nfw->fgrid.font->bpp;

            /*
             * Deal with the initial glyph code and make sure it
             * fits in the bounds of the new font.
             */
            glyphs = (font->glyphs_used > 0) ? font->glyphs : 0;

            nfw->fgrid.initial_glyph = (glyphs) ? glyphs->encoding : 0;
        }

        /*
         * If the orientation changed, swap the rows and columns before
         * recalculating anything.
         */
        if (nfw->fgrid.orientation != cfw->fgrid.orientation) {
            wd = nfw->fgrid.cell_cols;
            nfw->fgrid.cell_cols = nfw->fgrid.cell_rows;
            nfw->fgrid.cell_rows = wd;
        }

        /*
         * Set the cell geometry for the new font.
         */
        _XmuttFGridSetCellGeometry(nfw);

        /*
         * Determine if the rows and columns need to be adjusted if the
         * geometry exceeds the maximum threshold dimensions.
         */
        _XmuttFGridSetRowsCols(nfw, False);

        /*
         * Get the new geometry.
         */
        _XmuttFGridPreferredGeometry(nfw, &wd, &ht);
        nfw->core.width = wd;
        nfw->core.height = ht;

        if (nfw->fgrid.font != cfw->fgrid.font) {
            /*
             * Initialize the page info for encoded and unencoded.
             */
            pi = &nfw->fgrid.upage;
            pi->minpage = 0;
            pi->maxpage = 0xffff / nfw->fgrid.pagesize;
            pi->npage = pi->ppage = -1;
            pi->pageno = pi->bcode = 0;
            pi->sel_start = pi->sel_end = -1;
            (void) memset((char *) pi->selmap, 0,
                          sizeof(unsigned long) * 2048);
            (void) memcpy((char *) &nfw->fgrid.npage, (char *) pi,
                          sizeof(_XmuttFGridPageInfo));

            /*
             * Determine unencoded page info.
             */
            if (font->unencoded_used > 0) {
                pi = &nfw->fgrid.upage;
                if (nfw->fgrid.noblanks == True) {
                    glyphs = font->unencoded;

                    /*
                     * Determine the minimum page based on the encoding
                     * of the first glyph and set the current page.
                     */
                    pi->pageno = pi->minpage =
                        glyphs->encoding / nfw->fgrid.pagesize;

                    /*
                     * Determine the maximum page based on the encoding of the
                     * last glyph.
                     */
                    glyphs = font->unencoded + (font->unencoded_used - 1);
                    pi->maxpage = glyphs->encoding / nfw->fgrid.pagesize;

                    /*
                     * Set the base code for the current page.
                     */
                    pi->bcode = pi->pageno * nfw->fgrid.pagesize;
                } else {
                    /*
                     * Set the min and max pages to all.
                     */
                    pi->pageno = pi->minpage = 0;
                    pi->maxpage = 0xffff / nfw->fgrid.pagesize;
                }
                _XmuttFontGridNeighborPages(nfw, pi->pageno, &pi->ppage,
                                            &pi->npage);
            }

            /*
             * Set the page info for the encoded font glyphs.
             */
            pi = &nfw->fgrid.npage;
            pi->pageno = nfw->fgrid.initial_glyph / nfw->fgrid.pagesize;
            pi->bcode = pi->pageno * nfw->fgrid.pagesize;

            _XmuttFontGridNeighborPages(nfw, pi->pageno,
                                        &pi->ppage, &pi->npage);

            if (font->glyphs_used > 0) {
                glyphs = font->glyphs;
                pi->minpage = glyphs->encoding / nfw->fgrid.pagesize;
                glyphs += font->glyphs_used - 1;
                pi->maxpage = glyphs->encoding / nfw->fgrid.pagesize;
            } else {
                pi->minpage = 0;
                pi->maxpage = 0xffff / nfw->fgrid.pagesize;
            }

            /*
             * Set the selection to the initial glyph.
             */
            pi->sel_start = pi->sel_end = nfw->fgrid.initial_glyph;
            Select(pi->sel_start, pi->selmap);
        }
    } else if (nfw->core.width != cfw->core.width ||
               nfw->core.height != cfw->core.height) {
        redraw = True;

        font = nfw->fgrid.font;

        _XmuttFGridSetRowsCols(nfw, True);

        pi = (nfw->fgrid.unencoded == False) ? &nfw->fgrid.npage :
            &nfw->fgrid.upage;

        /*
         * Determine which page we want to make sure the base code of the
         * page before resize is always displayed.
         */
        pageno = pi->bcode / nfw->fgrid.pagesize;

        /*
         * Adjust the page info to reflect the modified number of rows and
         * columns.
         */
        if (font->unencoded_used > 0) {
            pi = &nfw->fgrid.upage;
            if (nfw->fgrid.noblanks == True) {
                glyphs = font->unencoded;
                pi->minpage = glyphs->encoding / nfw->fgrid.pagesize;
                glyphs = font->unencoded + (font->unencoded_used - 1);
                pi->maxpage = glyphs->encoding / nfw->fgrid.pagesize;
            } else {
                pi->minpage = 0;
                pi->maxpage = 0xffff / nfw->fgrid.pagesize;
            }
        }
        if (font->glyphs_used > 0) {
            pi = &nfw->fgrid.npage;
            if (nfw->fgrid.noblanks == True) {
                glyphs = font->glyphs;
                pi->minpage = glyphs->encoding / nfw->fgrid.pagesize;
                glyphs = font->glyphs + (font->glyphs_used - 1);
                pi->maxpage = glyphs->encoding / nfw->fgrid.pagesize;
            } else {
                pi->minpage = 0;
                pi->maxpage = 0xffff / nfw->fgrid.pagesize;
            }
        }

        XmuttFontGridGotoPage(newone, pageno);
    }

    /*
     * Handle the change of blank page skipping.
     */
    if (nfw->fgrid.noblanks != cfw->fgrid.noblanks) {
        pi = (nfw->fgrid.unencoded == False) ?
            &nfw->fgrid.npage : &nfw->fgrid.upage;

        if (nfw->fgrid.noblanks == True)
          /*
           * Skipping blank pages, so determine the next non-blank page.
           */
          _XmuttFontGridNeighborPages(nfw, pi->pageno, &pi->ppage, &pi->npage);
        else {
            /*
             * Not skipping blank pages, so determine the next page.
             */
            pi->ppage = pi->pageno - 1;
            pi->npage = (pi->pageno < pi->maxpage) ? pi->pageno + 1 : -1;
        }
    }

    if (nfw->fgrid.power2 != cfw->fgrid.power2) {
        _XmuttFGridSetRowsCols(nfw, True);
        redraw = True;
    }

    /*
     * Make sure the base for the character code is in limits.
     */
    if (nfw->fgrid.base != 8 && nfw->fgrid.base != 10 &&
        nfw->fgrid.base != 16)
      nfw->fgrid.base = 16;

    return redraw;
}

/*************************************************************************
 *
 * Start API functions.
 *
 *************************************************************************/

void
#ifndef _NO_PROTO
XmuttFontGridUpdateMetrics(Widget w, bdf_metrics_t *metrics)
#else
XmuttFontGridUpdateMetrics(w, metrics)
Widget w;
bdf_metrics_t *metrics;
#endif
{
    XmuttFontGridWidget fw;
    Dimension width, height;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    /*
     * Check to see if the metrics changes caused the bounding box to be
     * resized.
     */
    if (bdf_set_font_bbx(fw->fgrid.font, metrics)) {
        /*
         * Recalculate the cell geometry.
         */
        _XmuttFGridSetCellGeometry(fw);

        /*
         * Adjust the rows and columns if necessary.
         */
        _XmuttFGridSetRowsCols(fw, False);

        /*
         * Determine the new width and height.
         */
        _XmuttFGridPreferredGeometry(fw, &width, &height);

        if (width != fw->core.width || height != fw->core.height) {
            /*
             * Resize if the pasted glyphs caused a change in the font
             * bounding box.  Force the window to be cleared when the
             * redisplay is done to account for NorthWest gravity.
             */
            fw->fgrid.clearwin = True;
            _XmuttFGridAttemptResize(fw, width, height);
        }

        /*
         * Set up and call the modified callback.
         */
        cb.reason = XmuttFG_RESIZE;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridUpdateFont(Widget w, bdf_glyph_t *glyph, Boolean unencoded)
#else
XmuttFontGridUpdateFont(w, glyph, unencoded)
Widget w;
bdf_glyph_t *glyph;
Boolean unencoded;
#endif
{
    XmuttFontGridWidget fw;
    int unenc;
    Dimension width, height;
    _XmuttFGridPageInfo *pi;
    bdf_font_t *font;
    bdf_glyph_t *gp;
    bdf_glyphlist_t gl;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    unenc = (unencoded == False) ? 0 : 1;

    font = fw->fgrid.font;

    gl.bpp = font->bpp;
    gl.start = gl.end = glyph->encoding;
    gl.glyphs = glyph;
    gl.glyphs_used = 1;
    (void) memcpy((char *) &gl.bbx, (char *) &glyph->bbx, sizeof(bdf_bbx_t));

    /*
     * The font bounding box changed.
     */
    if (bdf_replace_glyphs(font, glyph->encoding, &gl, unenc)) {
        /*
         * Recalculate the cell geometry.
         */
        _XmuttFGridSetCellGeometry(fw);

        /*
         * Adjust the rows and columns if necessary.
         */
        _XmuttFGridSetRowsCols(fw, False);

        /*
         * Determine the new width and height.
         */
        _XmuttFGridPreferredGeometry(fw, &width, &height);

        if (width != fw->core.width || height != fw->core.height)
          /*
           * Resize if the pasted glyphs caused a change in the font
           * bounding box.
           */
          _XmuttFGridAttemptResize(fw, width, height);
        else
          /*
           * Otherwise, simply redraw the cells that are affected.
           */
          _XmuttFGridDrawCells(fw, glyph->encoding, glyph->encoding,
                               True, True);
    } else
      /*
       * Simply redraw the cells affected.
       */
      _XmuttFGridDrawCells(fw, glyph->encoding, glyph->encoding, True, True);

    /*
     * Update the number of pages used.
     */
    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    if (unenc) {
        if (fw->fgrid.noblanks == True) {
            if (font->unencoded_used == 0)
              pi->maxpage = 0;
            else {
                gp = font->unencoded + (font->unencoded_used - 1);
                pi->maxpage = gp->encoding / fw->fgrid.pagesize;
            }
        }
    } else {
        if (fw->fgrid.noblanks == True) {
            if (font->glyphs_used == 0)
              pi->maxpage = 0;
            else {
                gp = font->glyphs + (font->glyphs_used - 1);
                pi->maxpage = gp->encoding / fw->fgrid.pagesize;
            }
        }
    }

    /*
     * Set up and call the modified callback.
     */
    cb.reason = XmuttFG_REPLACE;
    cb.start = cb.end = glyph->encoding;
    cb.unencoded = fw->fgrid.unencoded;
    XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttFontGridUpdatePSFMappings(Widget w, long encoding,
                               bdf_psf_unimap_t *mappings)
#else
XmuttFontGridUpdatePSFMappings(w, encoding, mappings)
Widget w;
long encoding;
bdf_psf_unimap_t *mappings;
#endif
{
    XmuttFontGridWidget fw;
    XmuttFontGridModifiedCallbackStruct cb;
    int unenc;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    unenc = (fw->fgrid.unencoded == True) ? 1 : 0;

    if (bdf_replace_mappings(fw->fgrid.font, encoding, mappings, unenc)) {
        /*
         * Set up and call the modified callback.
         */
        cb.reason = XmuttFG_REPLACE_MAPPINGS;
        cb.start = cb.end = encoding;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

int
#ifndef _NO_PROTO
XmuttFontGridCodeBase(Widget w)
#else
XmuttFontGridCodeBase(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;
    return fw->fgrid.base;
}

void
#ifndef _NO_PROTO
XmuttFontGridChangeCodeBase(Widget w, int base)
#else
XmuttFontGridChangeCodeBase(w, base)
Widget w;
int base;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    if (base == fw->fgrid.base)
      return;

    if (base != 8 && base != 10 && base != 16)
      return;

    fw->fgrid.base = base;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    _XmuttFGridDrawCells(fw, pi->bcode, pi->bcode + (fw->fgrid.pagesize - 1),
                         True, False);
}

void
#ifndef _NO_PROTO
XmuttFontGridPageInfo(Widget w, XmuttFontGridPageInfoStruct *pageinfo)
#else
XmuttFontGridPageInfo(w, pageinfo)
Widget w;
XmuttFontGridPageInfoStruct *pageinfo;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    pi = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : &fw->fgrid.upage;

    pageinfo->previous_page = pi->ppage;
    pageinfo->current_page = pi->pageno;
    pageinfo->next_page = pi->npage;
    pageinfo->unencoded = fw->fgrid.unencoded;
    pageinfo->num_unencoded = (fw->fgrid.font != 0) ?
        fw->fgrid.font->unencoded_used : 0;
    pageinfo->num_encoded = (fw->fgrid.font != 0) ?
        fw->fgrid.font->glyphs_used : 0;
}

void
#ifndef _NO_PROTO
XmuttFontGridSelectionInfo(Widget w, XmuttFontGridSelectionStruct *selinfo)
#else
XmuttFontGridSelectionInfo(w, selinfo)
Widget w;
XmuttFontGridSelectionStruct *selinfo;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    long i, ng;
    bdf_font_t *font;
    bdf_glyph_t *glyphs;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    font = fw->fgrid.font;
    ng = 0;
    glyphs = 0;
    selinfo->glyphs = 0;
    selinfo->num_glyphs = 0;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        if (font) {
            glyphs = font->glyphs;
            ng = font->glyphs_used;
        }
    } else {
        pi = &fw->fgrid.upage;
        if (font) {
            glyphs = font->unencoded;
            ng = font->unencoded_used;
        }
    }

    /*
     * Position the glyphs if there are any.
     */
    for (i = 0; i < ng && glyphs->encoding < pi->sel_start; i++, glyphs++) ;
    selinfo->glyphs = glyphs;

    for (; i < ng && glyphs->encoding <= pi->sel_end; i++, glyphs++) ;
    selinfo->num_glyphs = glyphs - selinfo->glyphs;

    selinfo->reason = XmuttFG_SELECTION_INFO;
    selinfo->start = pi->sel_start;
    selinfo->end = pi->sel_end;
    selinfo->base = fw->fgrid.base;
    selinfo->unencoded = fw->fgrid.unencoded;
}

void
#ifndef _NO_PROTO
XmuttFontGridGotoCodePage(Widget w, long code)
#else
XmuttFontGridGotoCodePage(w, code)
Widget w;
long code;
#endif
{
    XmuttFontGridWidget fw;
    long pageno;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridSelectionStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    /*
     * Make sure the code is within reasonable bounds.
     */
    if (code < 0)
      code = 0;
    else if (code > 0xffff)
      code = 0xffff;

    /*
     * If this function is being called by an action proc and not from the
     * application, call the codePageCallback resource.
     */
    if (fw->fgrid.client_page == False)
      XtCallCallbackList(w, fw->fgrid.code_page, (XtPointer) code);

    pageno = code / fw->fgrid.pagesize;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;

    if (pageno != pi->pageno) {
        /*
         * Tell the GotoPage routine to ignore the start selection callback
         * because it is handled here.
         */
        fw->fgrid.no_sel_callback = True;
        XmuttFontGridGotoPage(w, pageno);
    }

    _XmuttFGridDeselectAll(fw);
    Select(code, pi->selmap);
    pi->sel_start = pi->sel_end = code;
    _XmuttFGridDrawCells(fw, code, code, False, True);

    if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
        empty_glyph.encoding = code;
        cb.glyphs = &empty_glyph;
    }
    cb.num_glyphs = 1;
    cb.reason = XmuttFG_START_SELECTION;
    cb.start = pi->sel_start;
    cb.end = pi->sel_end;
    cb.base = fw->fgrid.base;
    cb.unencoded = fw->fgrid.unencoded;
    XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttFontGridGotoPage(Widget w, long pageno)
#else
XmuttFontGridGotoPage(w, pageno)
Widget w;
long pageno;
#endif
{
    unsigned long mpage;
    long code;
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pip;
    XmuttFontGridPageInfoStruct pi;
    XmuttFontGridSelectionStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    if (fw->fgrid.font == 0)
      return;

    pip = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : &fw->fgrid.upage;

    mpage = 0xffff / fw->fgrid.pagesize;

    if (pageno < 0)
      pageno = 0;
    if (pageno > mpage)
      pageno = mpage;

    code = pip->sel_start - pip->bcode;

    pip->pageno = pageno;
    pip->bcode = pageno * fw->fgrid.pagesize;

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Move the cursor.
         */
        if (code >= 0) {
            _XmuttFGridDeselectAll(fw);
            code += pip->bcode;
            pip->sel_start = pip->sel_end = code;
            Select(code, pip->selmap);
        }
    }

    _XmuttFontGridNeighborPages(fw, pip->pageno, &pip->ppage, &pip->npage);

    if (fw->fgrid.resizing == False)
      /*
       * Redraw the page.
       */
      _XmuttFGridDrawCells(fw, pip->bcode,
			   pip->bcode + (fw->fgrid.pagesize - 1), True, True);

    /*
     * Get the new page info and tell the application about it.
     */
    XmuttFontGridPageInfo(w, &pi);
    XtCallCallbackList(w, fw->fgrid.page, (XtPointer) &pi);

    if (fw->fgrid.no_sel_callback == False &&
        fw->fgrid.from_keyboard == True) {
        /*
         * Get the new selection info and tell the application about it.
         */
        if (code >= 0) {
            if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
                empty_glyph.encoding = code;
                cb.glyphs = &empty_glyph;
            }
            cb.num_glyphs = 1;
            cb.reason = XmuttFG_START_SELECTION;
            cb.start = pip->sel_start;
            cb.end = pip->sel_end;
            cb.base = fw->fgrid.base;
            cb.unencoded = fw->fgrid.unencoded;
            XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
        }
    }
    
    fw->fgrid.no_sel_callback = False;
}

void
#ifndef _NO_PROTO
XmuttFontGridFirstPage(Widget w)
#else
XmuttFontGridFirstPage(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pip;
    long code;
    XmuttFontGridPageInfoStruct pi;
    XmuttFontGridSelectionStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    pip = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : &fw->fgrid.upage;

    if (pip->pageno == pip->minpage)
      return;

    code = pip->sel_start - pip->bcode;

    pip->pageno = pip->minpage;
    pip->bcode = pip->pageno * fw->fgrid.pagesize;

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Move the cursor.
         */
        _XmuttFGridDeselectAll(fw);
        code += pip->bcode;
        pip->sel_start = pip->sel_end = code;
        Select(code, pip->selmap);
    }

    /*
     * Determine the new neighbor pages.
     */
    _XmuttFontGridNeighborPages(fw, pip->pageno, &pip->ppage, &pip->npage);

    /*
     * Redraw the page.
     */
    _XmuttFGridDrawCells(fw, pip->bcode, pip->bcode + (fw->fgrid.pagesize - 1),
                         True, True);

    /*
     * Get the new page info and tell the application about it.
     */
    XmuttFontGridPageInfo(w, &pi);
    XtCallCallbackList(w, fw->fgrid.page, (XtPointer) &pi);

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Get the new selection info and tell the application about it.
         */
        if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
            empty_glyph.encoding = code;
            cb.glyphs = &empty_glyph;
        }
        cb.num_glyphs = 1;
        cb.reason = XmuttFG_START_SELECTION;
        cb.start = pip->sel_start;
        cb.end = pip->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridLastPage(Widget w)
#else
XmuttFontGridLastPage(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pip;
    long code;
    XmuttFontGridPageInfoStruct pi;
    XmuttFontGridSelectionStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    pip = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : &fw->fgrid.upage;

    if (pip->pageno == pip->maxpage)
      return;

    code = pip->sel_start - pip->bcode;

    pip->pageno = pip->maxpage;
    pip->bcode = pip->pageno * fw->fgrid.pagesize;

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Move the cursor.
         */
        _XmuttFGridDeselectAll(fw);
        code += pip->bcode;
        pip->sel_start = pip->sel_end = code;
        Select(code, pip->selmap);
    }

    /*
     * Determine the new neighbor pages.
     */
    _XmuttFontGridNeighborPages(fw, pip->pageno, &pip->ppage, &pip->npage);

    /*
     * Redraw the page.
     */
    _XmuttFGridDrawCells(fw, pip->bcode, pip->bcode + (fw->fgrid.pagesize - 1),
                         True, True);

    /*
     * Get the new page info and tell the application about it.
     */
    XmuttFontGridPageInfo(w, &pi);
    XtCallCallbackList(w, fw->fgrid.page, (XtPointer) &pi);

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Get the new selection info and tell the application about it.
         */
        if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
            empty_glyph.encoding = code;
            cb.glyphs = &empty_glyph;
        }
        cb.num_glyphs = 1;
        cb.reason = XmuttFG_START_SELECTION;
        cb.start = pip->sel_start;
        cb.end = pip->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridNextPage(Widget w)
#else
XmuttFontGridNextPage(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pip;
    bdf_font_t *font;
    long code;
    XmuttFontGridPageInfoStruct pi;
    XmuttFontGridSelectionStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    font = fw->fgrid.font;
    pip = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : &fw->fgrid.upage;

    if (pip->pageno == pip->maxpage)
      return;

    code = pip->sel_start - pip->bcode;

    pip->pageno = pip->npage;
    pip->bcode = pip->pageno * fw->fgrid.pagesize;

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Move the cursor.
         */
        _XmuttFGridDeselectAll(fw);
        code += pip->bcode;
        pip->sel_start = pip->sel_end = code;
        Select(code, pip->selmap);
    }

    _XmuttFontGridNeighborPages(fw, pip->pageno, &pip->ppage, &pip->npage);

    /*
     * Redraw the page.
     */
    _XmuttFGridDrawCells(fw, pip->bcode, pip->bcode + (fw->fgrid.pagesize - 1),
                         True, True);

    /*
     * Get the new page info and tell the application about it.
     */
    XmuttFontGridPageInfo(w, &pi);
    XtCallCallbackList(w, fw->fgrid.page, (XtPointer) &pi);

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Get the new selection info and tell the application about it.
         */
        if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
            empty_glyph.encoding = code;
            cb.glyphs = &empty_glyph;
        }
        cb.num_glyphs = 1;
        cb.reason = XmuttFG_START_SELECTION;
        cb.start = pip->sel_start;
        cb.end = pip->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridPreviousPage(Widget w)
#else
XmuttFontGridPreviousPage(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pip;
    bdf_font_t *font;
    long code;
    XmuttFontGridPageInfoStruct pi;
    XmuttFontGridSelectionStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    font = fw->fgrid.font;
    pip = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : &fw->fgrid.upage;


    if (pip->pageno == pip->minpage)
      return;

    code = pip->sel_start - pip->bcode;

    pip->pageno = pip->ppage;
    pip->bcode = pip->pageno * fw->fgrid.pagesize;

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Move the cursor.
         */
        _XmuttFGridDeselectAll(fw);
        code += pip->bcode;
        pip->sel_start = pip->sel_end = code;
        Select(code, pip->selmap);
    }

    _XmuttFontGridNeighborPages(fw, pip->pageno, &pip->ppage, &pip->npage);

    /*
     * Redraw the page.
     */
    _XmuttFGridDrawCells(fw, pip->bcode, pip->bcode + (fw->fgrid.pagesize - 1),
                         True, True);

    /*
     * Get the new page info and tell the application about it.
     */
    XmuttFontGridPageInfo(w, &pi);
    XtCallCallbackList(w, fw->fgrid.page, (XtPointer) &pi);

    if (fw->fgrid.from_keyboard == True) {
        /*
         * Get the new selection info and tell the application about it.
         */
        if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
            empty_glyph.encoding = code;
            cb.glyphs = &empty_glyph;
        }
        cb.num_glyphs = 1;
        cb.reason = XmuttFG_START_SELECTION;
        cb.start = pip->sel_start;
        cb.end = pip->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridViewUnencoded(Widget w, Boolean unencoded)
#else
XmuttFontGridViewUnencoded(w, unencoded)
Widget w;
Boolean unencoded;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pip;
    XmuttFontGridPageInfoStruct pi;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    if (fw->fgrid.unencoded == unencoded)
      return;

    fw->fgrid.unencoded = unencoded;

    pip = (unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;

    /*
     * Redraw the page.
     */
    _XmuttFGridDrawCells(fw, pip->bcode,
                         pip->bcode + (fw->fgrid.pagesize - 1),
                         True, True);

    /*
     * Get the new page info and tell the application about it.
     */
    XmuttFontGridPageInfo(w, &pi);
    XtCallCallbackList(w, fw->fgrid.page, (XtPointer) &pi);
}

Boolean
#ifndef _NO_PROTO
XmuttFontGridModified(Widget w)
#else
XmuttFontGridModified(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    return (fw->fgrid.font && fw->fgrid.font->modified) ? True : False;
}

void
#ifndef _NO_PROTO
XmuttFontGridSetModified(Widget w, Boolean modified)
#else
XmuttFontGridSetModified(w, modified)
Widget w;
Boolean modified;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    Boolean old;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    old = (fw->fgrid.font && fw->fgrid.font->modified) ? True : False;

    if (old == modified)
      return;

    if (modified == True)
      bdf_set_modified(fw->fgrid.font, 1);
    else
      bdf_set_modified(fw->fgrid.font, 0);

    if (old == True) {
        /*
         * If the old value was true, then mark everything as being unmodified
         * and redraw.
         */
        pi = (fw->fgrid.unencoded == False) ?
            &fw->fgrid.npage : &fw->fgrid.upage;
        _XmuttFGridDrawCells(fw, pi->bcode,
                             pi->bcode + (fw->fgrid.pagesize - 1),
                             True, False);
    }

    /*
     * Alert the client that the font has been modified.
     */
    cb.reason = XmuttFG_MODIFIED;
    XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttFontGridCallModifiedCallback(Widget w)
#else
XmuttFontGridCallModifiedCallback(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    /*
     * Alert the client that the font has been modified.
     */
    cb.reason = XmuttFG_MODIFIED;
    XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttFontGridSetFontName(Widget w, char *name)
#else
XmuttFontGridSetFontName(w, name)
Widget w;
char *name;
#endif
{
    XmuttFontGridWidget fw;
    int len;
    String appname, appclass;
    bdf_font_t *font;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    font = fw->fgrid.font;
    if (font->name != 0)
      free(font->name);
    if (name == 0 || *name == 0) {
        XtGetApplicationNameAndClass(XtDisplay((Widget) fw),
                                     &appname, &appclass);
        font->name = bdf_make_xlfd_name(font, appname, "Unknown");
    } else {
        len = strlen(name) + 1;
        font->name = (char *) malloc(len);
        (void) memcpy(font->name, name, len);
    }

    /*
     * Indicate that the font has changed.
     */
    bdf_set_modified(font, 1);

    /*
     * Call the modified callback to indicate that the font has been modified.
     */
    cb.reason = XmuttFG_NAME_CHANGE;
    cb.font_name = font->name;
    XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
}

Boolean
#ifndef _NO_PROTO
XmuttFontGridHasXLFDName(Widget w)
#else
XmuttFontGridHasXLFDName(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    return bdf_has_xlfd_name(fw->fgrid.font) ? True : False;
}

void
#ifndef _NO_PROTO
XmuttFontGridMakeXLFDName(Widget w)
#else
XmuttFontGridMakeXLFDName(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    char *name;
    String appname, appclass;
    bdf_font_t *font;
    bdf_property_t prop;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    font = fw->fgrid.font;

    if (bdf_has_xlfd_name(font) == 0) {
        /*
         * Add the original font name as a font property.
         */
        prop.name = "_ORIGINAL_FONT_NAME";
        prop.format = BDF_ATOM;
        prop.value.atom = font->name;
        bdf_add_font_property(font, &prop);

        /*
         * Create an XLFD name from the font data and change it.
         */
        XtGetApplicationNameAndClass(XtDisplay((Widget) fw),
                                     &appname, &appclass);
        name = bdf_make_xlfd_name(font, appname, "Unknown");
        XmuttFontGridSetFontName(w, name);
        free(name);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridUpdateNameFromProperties(Widget w)
#else
XmuttFontGridUpdateNameFromProperties(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;
    bdf_update_name_from_properties(fw->fgrid.font);

    /*
     * Call the modified callback to indicate that the font has been modified.
     */
    cb.reason = XmuttFG_NAME_CHANGE;
    cb.font_name = fw->fgrid.font->name;
    XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttFontGridUpdatePropertiesFromName(Widget w)
#else
XmuttFontGridUpdatePropertiesFromName(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    bdf_update_properties_from_name(fw->fgrid.font);

    /*
     * Call the modified callback to indicate that the font has been modified.
     */
    cb.reason = XmuttFG_MODIFIED;
    XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttFontGridUpdateAverageWidth(Widget w)
#else
XmuttFontGridUpdateAverageWidth(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    if (bdf_update_average_width(fw->fgrid.font)) {
        /*
         * Call the modified callback to indicate that the font has been
         * modified.
         */
        cb.reason = XmuttFG_NAME_CHANGE;
        cb.font_name = fw->fgrid.font->name;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

char *
#ifndef _NO_PROTO
XmuttFontGridFontName(Widget w)
#else
XmuttFontGridFontName(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    return fw->fgrid.font->name;
}

bdf_font_t *
#ifndef _NO_PROTO
XmuttFontGridFont(Widget w)
#else
XmuttFontGridFont(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;
    return fw->fgrid.font;
}

char *
#ifndef _NO_PROTO
XmuttFontGridFontComments(Widget w)
#else
XmuttFontGridFontComments(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    return fw->fgrid.font->comments;
}

void
#ifndef _NO_PROTO
XmuttFontGridUpdateComments(Widget w, char *comments,
                            unsigned long comments_len)
#else
XmuttFontGridUpdateComments(w, comments, comments_len)
Widget w;
char *comments;
unsigned long comments_len;
#endif
{
    XmuttFontGridWidget fw;
    bdf_font_t *f;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    f = fw->fgrid.font;
    if (bdf_replace_comments(f, comments, comments_len)) {
        cb.reason = XmuttFG_MODIFIED;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

char *
#ifndef _NO_PROTO
XmuttFontGridFontMessages(Widget w)
#else
XmuttFontGridFontMessages(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    return fw->fgrid.font->acmsgs;
}

int
#ifndef _NO_PROTO
XmuttFontGridFontSpacing(Widget w)
#else
XmuttFontGridFontSpacing(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    return fw->fgrid.font->spacing;
}

void
#ifndef _NO_PROTO
XmuttFontGridGetFontInfo(Widget w, XmuttFontGridFontInfoStruct *info)
#else
XmuttFontGridGetFontInfo(w, info)
Widget w;
XmuttFontGridFontInfoStruct *info;
#endif
{
    XmuttFontGridWidget fw;
    bdf_font_t *f;

    _XmuttFGridCheckClass(w);

    if (info == 0)
      return;

    fw = (XmuttFontGridWidget) w;
    f = fw->fgrid.font;

    info->name = f->name;
    info->comments = f->comments;
    info->messages = f->acmsgs;
    info->default_char = f->default_glyph;
    info->point_size = f->point_size;
    info->monowidth = f->monowidth;
    info->spacing = (unsigned short) f->spacing;
    info->font_ascent = f->font_ascent;
    info->font_descent = f->font_descent;
    info->resolution_x = f->resolution_x;
    info->resolution_y = f->resolution_y;
    info->bits_per_pixel = f->bpp;
    (void) memcpy((char *) &info->bbx, (char *) &f->bbx, sizeof(bdf_bbx_t));
}

void
#ifndef _NO_PROTO
XmuttFontGridSetFontInfo(Widget w, XmuttFontGridFontInfoStruct *info)
#else
XmuttFontGridSetFontInfo(w, info)
Widget w;
XmuttFontGridFontInfoStruct *info;
#endif
{
    XmuttFontGridWidget fw;
    int mod;
    bdf_font_t *f;
    bdf_property_t prop;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    if (info == 0)
      return;

    fw = (XmuttFontGridWidget) w;
    f = fw->fgrid.font;

    /*
     * Do some special stuff with the modified field so we know whether to
     * call the modified callback or not.
     */
    mod = f->modified;
    f->modified = 0;

    cb.reason = XmuttFG_MODIFIED;

    /*
     * The only fields not handled by this function yet are the name,
     * comments, acmsgs, and bbx fields.
     */

    /*
     * Handle the default character field.  If it happens to be -1, then
     * delete the font property.  Otherwise add it.
     */
    if (info->default_char < 0)
      bdf_delete_font_property(f, "DEFAULT_CHAR");
    else {
        prop.name = "DEFAULT_CHAR";
        prop.format = BDF_CARDINAL;
        prop.value.card32 = info->default_char;
        bdf_add_font_property(f, &prop);
    }

    prop.name = "POINT_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = info->point_size;
    bdf_add_font_property(f, &prop);

    prop.name = "FONT_ASCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = info->font_ascent;
    bdf_add_font_property(f, &prop);

    prop.name = "FONT_DESCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = info->font_descent;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_X";
    prop.format = BDF_CARDINAL;
    prop.value.int32 = info->resolution_x;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_Y";
    prop.format = BDF_CARDINAL;
    prop.value.int32 = info->resolution_y;
    bdf_add_font_property(f, &prop);

    prop.name = "SPACING";
    prop.format = BDF_ATOM;
    prop.value.atom = 0;
    switch (info->spacing) {
      case BDF_PROPORTIONAL: prop.value.atom = "P"; break;
      case BDF_MONOWIDTH: prop.value.atom = "M"; break;
      case BDF_CHARCELL: prop.value.atom = "C"; break;
    }
    if (prop.value.atom != 0)
      bdf_add_font_property(f, &prop);

    /*
     * If the font was modified, and has an XLFD name, make sure the XLFD name
     * gets updated from the properties and the appropriate callback is
     * called.
     */
    if (f->modified && bdf_has_xlfd_name(f))
      XmuttFontGridUpdateNameFromProperties(w);

    /*
     * Now determine if the monowidth field will have a resize affect on
     * things.
     */
    if (f->spacing != BDF_PROPORTIONAL) {
        if (f->monowidth == 0) {
            /*
             * Handle the special case of a proportional font being changed to
             * some other spacing.
             */
            f->monowidth = f->bbx.width;
            f->modified = 1;
        }
        if (info->monowidth != f->monowidth) {
            cb.reason = XmuttFG_RESIZE;
            f->monowidth = f->bbx.width = info->monowidth;
            f->modified = 1;
        }
    }

    /*
     * Set up the callback to indicate something has changed if necessaary.
     */
    if (f->modified)
      XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    f->modified |= mod;
}

void
#ifndef _NO_PROTO
XmuttFontGridFontResolution(Widget w, long *resolution_x, long *resolution_y)
#else
XmuttFontGridFontResolution(w, resolution_x, resolution_y)
Widget w;
long *resolution_x, *resolution_y;
#endif
{
    XmuttFontGridWidget fw;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    *resolution_x = fw->fgrid.font->resolution_x;
    *resolution_y = fw->fgrid.font->resolution_y;
}

void
#ifndef _NO_PROTO
XmuttFontGridSetFontDeviceWidth(Widget w, unsigned short dwidth)
#else
XmuttFontGridSetFontDeviceWidth(w, dwidth)
Widget w;
unsigned short dwidth;
#endif
{
    XmuttFontGridWidget fw;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    if (dwidth != fw->fgrid.font->monowidth) {
        fw->fgrid.font->modified = 1;
        fw->fgrid.font->monowidth =
            fw->fgrid.font->bbx.width = dwidth;

        cb.reason = XmuttFG_RESIZE;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

/*
 * Glyph transformation functions: translate, rotate, shear, embolden.
 */
void
#ifndef _NO_PROTO
XmuttFontGridTranslateGlyphs(Widget w, Position dx, Position dy,
                             bdf_callback_t callback, void *data,
                             Boolean all_glyphs)
#else
XmuttFontGridTranslateGlyphs(w, dx, dy, callback, data, all_glyphs)
Widget w;
Position dx, dy;
bdf_callback_t callback;
void *data;
Boolean all_glyphs;
#endif
{
    int unenc;
    long start, end;
    Dimension width, height;
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    unenc = (fw->fgrid.unencoded == True) ? 1 : 0;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    if (all_glyphs == True) {
        start = pi->minpage * fw->fgrid.pagesize;
        end = (pi->maxpage * fw->fgrid.pagesize) + fw->fgrid.pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    /*
     * If there is no selection, just return.
     */
    if (start < 0 || end < 0)
      return;

    if (bdf_translate_glyphs(fw->fgrid.font, dx, dy, start, end, callback,
                             data, unenc)) {
        /*
         * Recalculate the cell geometry.
         */
        _XmuttFGridSetCellGeometry(fw);

        /*
         * Adjust the rows and columns if necessary.
         */
        _XmuttFGridSetRowsCols(fw, False);

        /*
         * Determine the new width and height.
         */
        _XmuttFGridPreferredGeometry(fw, &width, &height);

        if (width != fw->core.width || height != fw->core.height) {
            /*
             * Resize if the pasted glyphs caused a change in the font
             * bounding box.
             */
            fw->fgrid.clearwin = True;
            _XmuttFGridAttemptResize(fw, width, height);
        } else
          /*
           * Otherwise, simply redraw the cells that are affected.
           */
          _XmuttFGridDrawCells(fw, start, end, True, True);

        /*
         * Alert the client that the font has been modified.
         */
        cb.reason = XmuttFG_MODIFIED;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridRotateGlyphs(Widget w, short degrees, bdf_callback_t callback,
                          void *data, Boolean all_glyphs)
#else
XmuttFontGridRotateGlyphs(w, degrees, callback, data, all_glyphs)
Widget w;
short degrees;
bdf_callback_t callback;
void *data;
Boolean all_glyphs;
#endif
{
    int unenc;
    long start, end;
    Dimension width, height;
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    unenc = (fw->fgrid.unencoded == True) ? 1 : 0;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    if (all_glyphs == True) {
        start = pi->minpage * fw->fgrid.pagesize;
        end = (pi->maxpage * fw->fgrid.pagesize) + fw->fgrid.pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    /*
     * If there is no selection, just return.
     */
    if (start < 0 || end < 0)
      return;

    if (bdf_rotate_glyphs(fw->fgrid.font, degrees, start, end, callback,
                          data, unenc)) {
        /*
         * Recalculate the cell geometry.
         */
        _XmuttFGridSetCellGeometry(fw);

        /*
         * Adjust the rows and columns if necessary.
         */
        _XmuttFGridSetRowsCols(fw, False);

        /*
         * Determine the new width and height.
         */
        _XmuttFGridPreferredGeometry(fw, &width, &height);

        if (width != fw->core.width || height != fw->core.height) {
            /*
             * Resize if the pasted glyphs caused a change in the font
             * bounding box.
             */
            fw->fgrid.clearwin = True;
            _XmuttFGridAttemptResize(fw, width, height);
        } else
          /*
           * Otherwise, simply redraw the cells that are affected.
           */
          _XmuttFGridDrawCells(fw, start, end, True, True);

        /*
         * Alert the client that the font has been modified.
         */
        cb.reason = XmuttFG_MODIFIED;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridShearGlyphs(Widget w, short degrees, bdf_callback_t callback,
                          void *data, Boolean all_glyphs)
#else
XmuttFontGridShearGlyphs(w, degrees, callback, data, all_glyphs)
Widget w;
short degrees;
bdf_callback_t callback;
void *data;
Boolean all_glyphs;
#endif
{
    int unenc;
    long start, end;
    Dimension width, height;
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    unenc = (fw->fgrid.unencoded == True) ? 1 : 0;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    if (all_glyphs == True) {
        start = pi->minpage * fw->fgrid.pagesize;
        end = (pi->maxpage * fw->fgrid.pagesize) + fw->fgrid.pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    /*
     * If there is no selection, just return.
     */
    if (start < 0 || end < 0)
      return;

    if (bdf_shear_glyphs(fw->fgrid.font, degrees, start, end, callback,
                         data, unenc)) {
        /*
         * Recalculate the cell geometry.
         */
        _XmuttFGridSetCellGeometry(fw);

        /*
         * Adjust the rows and columns if necessary.
         */
        _XmuttFGridSetRowsCols(fw, False);

        /*
         * Determine the new width and height.
         */
        _XmuttFGridPreferredGeometry(fw, &width, &height);

        if (width != fw->core.width || height != fw->core.height) {
            /*
             * Resize if the pasted glyphs caused a change in the font
             * bounding box.
             */
            fw->fgrid.clearwin = True;
            _XmuttFGridAttemptResize(fw, width, height);
        } else
          /*
           * Otherwise, simply redraw the cells that are affected.
           */
          _XmuttFGridDrawCells(fw, start, end, True, True);

        /*
         * Alert the client that the font has been modified.
         */
        cb.reason = XmuttFG_MODIFIED;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

void
#ifndef _NO_PROTO
XmuttFontGridEmboldenGlyphs(Widget w, bdf_callback_t callback, void *data,
                            Boolean all_glyphs)
#else
XmuttFontGridEmboldenGlyphs(w, callback, data, all_glyphs)
Widget w;
bdf_callback_t callback;
void *data;
Boolean all_glyphs;
#endif
{
    int unenc, resize;
    long start, end;
    Dimension width, height;
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;

    unenc = (fw->fgrid.unencoded == True) ? 1 : 0;

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;
    if (all_glyphs == True) {
        start = pi->minpage * fw->fgrid.pagesize;
        end = (pi->maxpage * fw->fgrid.pagesize) + fw->fgrid.pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    /*
     * If there is no selection, just return.
     */
    if (start < 0 || end < 0)
      return;

    resize = 0;
    if (bdf_embolden_glyphs(fw->fgrid.font, start, end, callback, data,
                            unenc, &resize)) {
        if (resize) {
            /*
             * Recalculate the cell geometry.
             */
            _XmuttFGridSetCellGeometry(fw);

            /*
             * Adjust the rows and columns if necessary.
             */
            _XmuttFGridSetRowsCols(fw, False);

            /*
             * Determine the new width and height.
             */
            _XmuttFGridPreferredGeometry(fw, &width, &height);

            if (width != fw->core.width || height != fw->core.height) {
                /*
                 * Resize if the emboldened glyphs caused a change in the font
                 * bounding box.
                 */
                fw->fgrid.clearwin = True;
                _XmuttFGridAttemptResize(fw, width, height);
            } else
              /*
               * Otherwise, simply redraw the cells that are affected.
               */
              _XmuttFGridDrawCells(fw, start, end, True, True);
        } else
          _XmuttFGridDrawCells(fw, start, end, True, True);

        /*
         * Alert the client that the font has been modified.
         */
        cb.reason = XmuttFG_MODIFIED;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
    }
}

Boolean
#ifndef _NO_PROTO
XmuttFontGridHasSelection(Widget w)
#else
XmuttFontGridHasSelection(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;

    _XmuttFGridCheckClass(w);

    fw = (XmuttFontGridWidget) w;
    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;

    return (pi->sel_start == -1) ? False : True;
}

Boolean
#ifndef _NO_PROTO
XmuttFontGridClipboardEmpty(Widget w)
#else
XmuttFontGridClipboardEmpty(w)
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

    _XmuttFGridCheckClass(w);

    empty = True;

    d = XtDisplay(w);
    if ((owner = XGetSelectionOwner(d, XmuttFONTGRID_CLIPBOARD(d))) == None)
      return empty;

    if (XGetWindowProperty(d, owner, XmuttFONTGRID_CLIPBOARD(d), 0, 102400,
                           False, XmuttFONTGRID_GLYPHLIST(d), &atype, &afmt,
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
XmuttFontGridCopySelection(Widget w)
#else
XmuttFontGridCopySelection(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    Display *d;
    unsigned char *sel;
    unsigned long bytes;

    _XmuttFGridCheckClass(w);

    /*
     * If the widget is not realized, there is no point to this operation.
     */
    if (!XtIsRealized(w))
      return;

    fw = (XmuttFontGridWidget) w;
    d = XtDisplay(w);

    /*
     * If the widget has no selection, then this routine will return 0.
     */
    if ((sel = _XmuttFGridEncodeSelection(fw, &bytes)) == 0)
      return;

    /*
     * If only a single cell is selected and this widget doesn't own the
     * clipboard, then attempt to gain control of the clipboard.
     */
    if (fw->fgrid.owns_clipboard == False)
      _XmuttFGridOwnClipboard(w);

    /*
     * Go ahead and actually write the data to the clipboard and then free the
     * buffer.
     */
    XChangeProperty(d, XtWindow(w), XmuttFONTGRID_CLIPBOARD(d),
                    XmuttFONTGRID_GLYPHLIST(d), 8, PropModeReplace,
                    sel, bytes);
    XtFree((char *) sel);
}

void
#ifndef _NO_PROTO
XmuttFontGridCutSelection(Widget w)
#else
XmuttFontGridCutSelection(w)
Widget w;
#endif
{
    XmuttFontGridWidget fw;
    Display *d;
    unsigned char *sel;
    unsigned long bytes;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridModifiedCallbackStruct cb;

    _XmuttFGridCheckClass(w);

    /*
     * If the widget is not realized, there is no point to this operation.
     */
    if (!XtIsRealized(w))
      return;

    fw = (XmuttFontGridWidget) w;
    d = XtDisplay(w);

    /*
     * If the widget has no selection, then this routine will return 0.
     */
    if ((sel = _XmuttFGridEncodeSelection(fw, &bytes)) == 0)
      return;

    /*
     * If only a single cell is selected and this widget doesn't own the
     * clipboard, then attempt to gain control of the clipboard.
     */
    if (fw->fgrid.owns_clipboard == False)
      _XmuttFGridOwnClipboard(w);

    /*
     * Go ahead and actually write the data to the clipboard and then free the
     * buffer.
     */
    XChangeProperty(d, XtWindow(w), XmuttFONTGRID_CLIPBOARD(d),
                    XmuttFONTGRID_GLYPHLIST(d), 8, PropModeReplace,
                    sel, bytes);
    XtFree((char *) sel);

    pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage : &fw->fgrid.upage;

    /*
     * Now delete the glyphs in the specified range.
     */
    if (fw->fgrid.unencoded == False)
      bdf_delete_glyphs(fw->fgrid.font, pi->sel_start, pi->sel_end, 0);
    else
      bdf_delete_glyphs(fw->fgrid.font, pi->sel_start, pi->sel_end, 1);

    /*
     * Set up for the callback.
     */
    cb.reason = XmuttFG_DELETE;
    cb.start = pi->sel_start;
    cb.end = pi->sel_end;
    cb.unencoded = fw->fgrid.unencoded;

    /*
     * Update the labels and clear the selection.
     */
    _XmuttFGridDeselectAll(fw);

    /*
     * Reselect the start of the selection and set it.
     */
    pi->sel_start = pi->sel_end = cb.start;
    Select(pi->sel_start, pi->selmap);

    _XmuttFGridDrawCells(fw, pi->sel_start, pi->sel_end, True, True);

    /*
     * Call the modified callback.
     */
    XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);
}

void
#ifndef _NO_PROTO
XmuttFontGridPasteSelection(Widget w, int type)
#else
XmuttFontGridPasteSelection(w, type)
Widget w;
int type;
#endif
{
    XmuttFontGridWidget fw;
    Dimension width, height;
    Display *d;
    Window win;
    Atom atype;
    int afmt, unenc, doresize;
    unsigned long nitems, nbytes, ng;
    unsigned char *data;
    bdf_font_t *font;
    bdf_glyph_t *gp;
    bdf_glyphlist_t *gl;
    _XmuttFGridPageInfo *pi;
    bdf_glyphlist_t overflow;
    XmuttFontGridModifiedCallbackStruct cb;
    XmuttFontGridSelectionStruct cbs;

    _XmuttFGridCheckClass(w);

    /*
     * If the widget is not realized, there is no point to this operation.
     */
    if (!XtIsRealized(w))
      return;

    fw = (XmuttFontGridWidget) w;
    d = XtDisplay(w);
    
    if ((win = XGetSelectionOwner(d, XmuttFONTGRID_CLIPBOARD(d))) == None) {
        _XmuttFGridOwnClipboard(w);
        /*
         * A return is in order here because the clipboard can't have any data
         * if there was no owner previously.
         */
        return;
    }

    doresize = 0;
    unenc = (fw->fgrid.unencoded == False) ? 0 : 1;

    /*
     * If there is no single cell selected as a paste position, then
     * simply ring the bell and return.
     */
    pi = (unenc == 0) ? &fw->fgrid.npage : &fw->fgrid.upage;
    if (unenc == 0 && (pi->sel_start == -1 || pi->sel_start != pi->sel_end)) {
        XBell(XtDisplay(w), 0);
        return;
    }

    nitems = nbytes = 0;
    XGetWindowProperty(d, win, XmuttFONTGRID_CLIPBOARD(d), 0, 10240,
                       False, XmuttFONTGRID_GLYPHLIST(d), &atype, &afmt,
                       &nitems, &nbytes, &data);

    /*
     * Attempt to own the clipboard after getting the value if this widget
     * does not own it.
     */
    if (win != XtWindow(w))
      _XmuttFGridOwnClipboard(w);

    if (nitems > 0) {
        font = fw->fgrid.font;
        gl = &fw->fgrid.clipboard;

        /*
         * Convert the encoded selection into a glyph list in the internal
         * glyph list clipboard.
         */
        _XmuttFGridDecodeSelection(fw, data);

        /*
         * If the paste is occuring in the unencoded section, make sure the
         * paste is appended as opposed to being inserted.  Also turn off
         * the selected cell before doing the paste.
         */
        if (unenc == 1) {
            _XmuttFGridDeselectAll(fw);
            pi->sel_start = font->unencoded_used;
            gl->start = 0;
            gl->end = gl->glyphs_used - 1;
        }

        /*
         * Set the end point of the selection.
         */
        pi->sel_end = pi->sel_start + (gl->end - gl->start);

        /*
         * First, check to see if pasting the glyphs will exceed the maximum
         * encoding value of 0xffff.  If some of them do, then transfer the
         * extra glyphs to the unencoded area before doing anything else.
         * This means that a new glyph list needs to be constructed to do the
         * insert into the unencoded area.
         */
        if (unenc == 0 && pi->sel_end > 0xffff) {
            /*
             * Determine if any of the glyphs would actually get encoded after
             * 0xffff or if those are all empty glyphs.
             */
            for (ng = 0, gp = gl->glyphs; ng < gl->glyphs_used; ng++, gp++) {
                if (pi->sel_start + (gp->encoding - gl->start) > 0xffff)
                  /*
                   * The glyph list does contain glyphs that will overflow.
                   */
                  break;
            }

            if (ng < gl->glyphs_used) {
                /*
                 * Construct a new glyph list containing only the glyphs that
                 * overflow the 0xffff boundary.  There is no need to
                 * recalculate the bounding box for the new glyph list.  Any
                 * resize will be handled correctly anyway.
                 */
                (void) memcpy((char *) &overflow.bbx, (char *) &gl->bbx,
                              sizeof(bdf_bbx_t));
                overflow.bpp = font->bpp;
                overflow.glyphs_used = gl->glyphs_used - ng;
                overflow.glyphs = gp;
                overflow.start = 0;
                overflow.end = overflow.glyphs_used - 1;

                /*
                 * Add the glyphs to the unencoded area.
                 */
                doresize = bdf_replace_glyphs(font, font->unencoded_used,
                                              &overflow, 1);
            }

            /*
             * Adjust the glyph list and selection to fit within the 0xffff
             * limit before pasting the glyphs into the font.
             */
            gl->glyphs_used = ng;
            gl->end -= pi->sel_end - 0xffff;
            pi->sel_end = 0xffff;
        }

        /*
         * If the grid is in insert mode, then determine if moving glyphs
         * forward from the insert location would cause an overflow.
         */
        if (unenc == 0 && (fw->fgrid.overwrite_mode == False ||
                           type == XmuttFG_INSERT_PASTE)) {
            doresize += bdf_insert_glyphs(font, pi->sel_start, gl);
            /*
             * Force a page recalculation to be done so the application can
             * update if needed.
             */
            XmuttFontGridGotoPage(w, fw->fgrid.npage.pageno);
        } else if (type == XmuttFG_MERGE_PASTE)
          doresize += bdf_merge_glyphs(font, pi->sel_start, gl, unenc);
        else
          doresize += bdf_replace_glyphs(font, pi->sel_start, gl, unenc);

        /*
         * If the incoming glyphs changed the font bounding box, then
         * determine the new geometry and attempt a resize.
         */
        if (doresize) {
            /*
             * Recalculate the cell geometry.
             */
            _XmuttFGridSetCellGeometry(fw);

            /*
             * Adjust the rows and columns if necessary.
             */
            _XmuttFGridSetRowsCols(fw, False);

            /*
             * Determine the new width and height.
             */
            _XmuttFGridPreferredGeometry(fw, &width, &height);

            if (width != fw->core.width || height != fw->core.height)
              /*
               * Resize if the pasted glyphs caused a change in the
               * font bounding box.
               */
              _XmuttFGridAttemptResize(fw, width, height);
            else
              /*
               * Otherwise, simply redraw the cells that are affected.
               */
              _XmuttFGridDrawCells(fw, pi->sel_start, pi->sel_end, True, True);
        } else {
            _XmuttFGridRedrawGrid(fw);

            /*
             * The font bounding box didn't change, so simply reselect
             * the range just replaced.
             */
            _XmuttFGridSelectRange(fw, pi->sel_start, pi->sel_end);
        }

        /*
         * Update the number of pages used.
         */
        pi = (fw->fgrid.unencoded == False) ? &fw->fgrid.npage :
            &fw->fgrid.upage;
        if (unenc) {
            if (fw->fgrid.noblanks == True) {
                if (font->unencoded_used == 0)
                  pi->maxpage = 0;
                else {
                    gp = font->unencoded + (font->unencoded_used - 1);
                    pi->maxpage = gp->encoding / fw->fgrid.pagesize;
                }
            }
        } else {
            if (fw->fgrid.noblanks == True) {
                if (font->glyphs_used == 0)
                  pi->maxpage = 0;
                else {
                    gp = font->glyphs + (font->glyphs_used - 1);
                    pi->maxpage = gp->encoding / fw->fgrid.pagesize;
                }
            }
        }

        /*
         * Set up and call the modified callback.
         */
        cb.reason = XmuttFG_REPLACE;
        cb.start = pi->sel_start;
        cb.end = pi->sel_end;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.modified, (XtPointer) &cb);

        /*
         * Set up and call the glyph update routine so the calling application
         * can use the new glyph name.
         */
        cbs.reason = XmuttFG_START_SELECTION;
        cbs.glyphs = _XmuttFGridCodeToGlyph(fw, pi->sel_start);
        cbs.num_glyphs = 1;
        cbs.start = cbs.end = pi->sel_start;
        cbs.base = fw->fgrid.base;
        cbs.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cbs);

        /*
         * And last, since the change of the selection owner caused the
         * clipboard to lose its data, add the data to it again so
         * it can be pasted in some other font editor.
         */
        XChangeProperty(d, XtWindow(w), XmuttFONTGRID_CLIPBOARD(d),
                        XmuttFONTGRID_GLYPHLIST(d), 8, PropModeReplace,
                        data, nitems);

        XFree((char *) data);
    }
}

/*************************************************************************
 *
 * Start action procedures.
 *
 *************************************************************************/

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
    XmuttFontGridWidget fw;

    /*
     * There is only one way to really gain the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    fw = (XmuttFontGridWidget) w;
    fw->fgrid.hasfocus = True;

    _XmuttFGridHighlightWindow(w);
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
    XmuttFontGridWidget fw;

    /*
     * There is only one way to really lose the focus and that is
     * from a send_event.
     */
    if (!event->xfocus.send_event)
      return;

    fw = (XmuttFontGridWidget) w;
    fw->fgrid.hasfocus = False;

    _XmuttFGridHighlightWindow(w);
}

static void
#ifndef _NO_PROTO
StartSelection(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
StartSelection(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    int x, y, row, col;
    Time mclick;
    long code;
    bdf_glyph_t *glyph;
    _XmuttFGridPageInfo *pi, *opi;
    XmuttFontGridSelectionStruct cb;

    fw = (XmuttFontGridWidget) w;

    /*
     * Force a traversal if the widget doesn't have the focus on a button
     * press.
     */
    if (fw->fgrid.hasfocus == False)
      (void) XmProcessTraversal(w, XmTRAVERSE_CURRENT);

    /*
     * Figure out which cell has been selected.
     */
    x = event->xbutton.x;
    y = event->xbutton.y;

    /*
     * Simply return if the mouse is outside the bounds of the actual grid
     * drawing area.
     */
    col = _XmuttFGridLeftX(fw) + fw->fgrid.xoff +
        (fw->fgrid.cell_width * fw->fgrid.cell_cols);
    row = _XmuttFGridTopY(fw) + fw->fgrid.yoff +
        (fw->fgrid.cell_height * fw->fgrid.cell_rows);
    if (x < fw->fgrid.xoff || x >=  col || y < fw->fgrid.yoff || y >= row)
      return;

    row = (y - fw->fgrid.yoff - _XmuttFGridTopY(fw)) / fw->fgrid.cell_height;
    col = (x - fw->fgrid.xoff - _XmuttFGridLeftX(fw)) / fw->fgrid.cell_width;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        opi = &fw->fgrid.upage;
    } else {
        pi = &fw->fgrid.upage;
        opi = &fw->fgrid.npage;
    }

    if (fw->fgrid.orientation == XmHORIZONTAL)
      code = pi->bcode + (row * fw->fgrid.cell_cols) + col;
    else
      code = pi->bcode + (col * fw->fgrid.cell_rows) + row;

    /*
     * If the code is beyond 0xFFFF, simply ring the bell and return.
     */
    if (code > 0xffff) {
        XBell(XtDisplay(w), 0);
        return;
    }

    glyph = _XmuttFGridCodeToGlyph(fw, code);

    /*
     * If the code happens to be exactly the same as the start
     * and end of the selection range, then skip around the
     * highlighting part.
     */
    if (code != pi->sel_start || code != pi->sel_end) {
        /*
         * If there was something selected in either the normal pages or the
         * unencoded pages, deselect it.
         */
        if (pi->sel_start != -1 || pi->sel_end != -1 ||
            opi->sel_start != -1 || opi->sel_end != -1)
          _XmuttFGridDeselectAll(fw);

        Select(code, pi->selmap);

        if (glyph == 0) {
            empty_glyph.encoding = code;
            glyph = &empty_glyph;
        }

        _XmuttFGridDrawCells(fw, glyph->encoding, glyph->encoding,
                             False, True);

        pi->sel_start = pi->sel_end = code;

        /*
         * Set the last click time to 0 to avoid situations where the second
         * click on a different cell will cause the select callback to be
         * invoked.
         */
        fw->fgrid.last_click = 0;
    }

    /*
     * Set the glyph pointer for the callback.
     */
    cb.glyphs = glyph;
    cb.num_glyphs = 1;

    mclick = (Time) MAX(fw->fgrid.mclick, XtGetMultiClickTime(XtDisplay(w)));
    if (event->xbutton.time - fw->fgrid.last_click < mclick) {
        cb.reason = XmuttFG_ACTIVATE;
        cb.start = pi->sel_start;
        cb.end = pi->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.activate, (XtPointer) &cb);
    } else {
        /*
         * Only call the callback if the selection starts in a new
         * location.
         */
        cb.reason = XmuttFG_START_SELECTION;
        cb.start = pi->sel_start;
        cb.end = pi->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
    }

    fw->fgrid.last_click = event->xbutton.time;
}

static void
#ifndef _NO_PROTO
ExtendSelection(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ExtendSelection(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    Boolean call_extend;
    long code;
    int x, y, row, col;
    bdf_glyph_t *glyph;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridSelectionStruct cb;

    fw = (XmuttFontGridWidget) w;

    /*
     * Figure out which cell has been selected.
     */
    x = event->xbutton.x;
    y = event->xbutton.y;

    /*
     * Simply return if the mouse is outside the bounds of the actual grid
     * drawing area.
     */
    col = _XmuttFGridLeftX(fw) + fw->fgrid.xoff +
        (fw->fgrid.cell_width * fw->fgrid.cell_cols);
    row = _XmuttFGridTopY(fw) + fw->fgrid.yoff +
        (fw->fgrid.cell_height * fw->fgrid.cell_rows);
    if (x < fw->fgrid.xoff || x >=  col || y < fw->fgrid.yoff || y >= row)
      return;

    row = (y - fw->fgrid.yoff - _XmuttFGridTopY(fw)) / fw->fgrid.cell_height;
    col = (x - fw->fgrid.xoff - _XmuttFGridLeftX(fw)) / fw->fgrid.cell_width;

    pi = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : & fw->fgrid.upage;

    if (fw->fgrid.orientation == XmHORIZONTAL)
      code = pi->bcode + (row * fw->fgrid.cell_cols) + col;
    else
      code = pi->bcode + (col * fw->fgrid.cell_rows) + row;

    call_extend = False;
    if (code > pi->sel_end) {
        call_extend = True;
        if (code <= pi->sel_start)
          _XmuttFGridDeselectRange(fw, pi->sel_end, code - 1);
        else {
            if (pi->sel_end < pi->sel_start) {
                _XmuttFGridDeselectRange(fw, pi->sel_end, pi->sel_start - 1);
                _XmuttFGridSelectRange(fw, pi->sel_start + 1, code);
            } else
              _XmuttFGridSelectRange(fw, pi->sel_end, code);
        }
    } else if (code < pi->sel_end) {
        call_extend = True;
        if (code < pi->sel_start) {
            if (pi->sel_end > pi->sel_start) {
                _XmuttFGridDeselectRange(fw, pi->sel_start + 1, pi->sel_end);
                _XmuttFGridSelectRange(fw, code, pi->sel_start);
            } else
              _XmuttFGridSelectRange(fw, code, pi->sel_end);
        } else
          _XmuttFGridDeselectRange(fw, code + 1, pi->sel_end);
    }
    pi->sel_end = code;

    if (call_extend == True) {
        if (pi->sel_start == pi->sel_end) {
            if ((glyph = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
                empty_glyph.encoding = code;
                glyph = &empty_glyph;
            }
            cb.glyphs = glyph;
            cb.num_glyphs = 1;
        }
        cb.reason = XmuttFG_EXTEND_SELECTION;
        cb.start = pi->sel_start;
        cb.end = pi->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_extend, (XtPointer) &cb);
    }
}

static void
#ifndef _NO_PROTO
EndSelection(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
EndSelection(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    int wd, ht;
    long i, ng;
    bdf_font_t *font;
    bdf_glyph_t *glyphs;
    XmuttFontGridSelectionStruct cb;

    fw = (XmuttFontGridWidget) w;

    glyphs = 0;

    /*
     * If the event occurs somewhere off the edge of the actual grid area,
     * simply return.
     */
    wd = _XmuttFGridLeftX(fw) + fw->fgrid.xoff +
        (fw->fgrid.cell_width * fw->fgrid.cell_cols);
    ht = _XmuttFGridTopY(fw) + fw->fgrid.yoff +
        (fw->fgrid.cell_height * fw->fgrid.cell_rows);
    if (event->xbutton.x < fw->fgrid.xoff || event->xbutton.x >= wd ||
        event->xbutton.y < fw->fgrid.yoff || event->xbutton.y >= ht)
      return;

    font = fw->fgrid.font;
    ng = 0;
    cb.glyphs = 0;
    cb.num_glyphs = 0;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        if (font) {
            glyphs = font->glyphs;
            ng = font->glyphs_used;
        }
    } else {
        pi = &fw->fgrid.upage;
        if (font) {
            glyphs = font->unencoded;
            ng = font->unencoded_used;
        }
    }

    /*
     * Position the glyphs if there are any.
     */
    for (i = 0; i < ng && glyphs->encoding < pi->sel_start; i++, glyphs++) ;
    cb.glyphs = glyphs;

    for (; i < ng && glyphs->encoding <= pi->sel_end; i++, glyphs++) ;
    cb.num_glyphs = glyphs - cb.glyphs;

    /*
     * If the selection contains more than one glyph, then attempt to
     * gain control of the clipboard.
     */
    if (pi->sel_end - pi->sel_start != 0)
      _XmuttFGridOwnClipboard(w);

    cb.reason = XmuttFG_END_SELECTION;
    cb.start = pi->sel_start;
    cb.end = pi->sel_end;
    cb.base = fw->fgrid.base;
    cb.unencoded = fw->fgrid.unencoded;
    XtCallCallbackList(w, fw->fgrid.sel_end, (XtPointer) &cb);
}

static void
#ifndef _NO_PROTO
AdjustSelection(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
AdjustSelection(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    Boolean call_extend;
    long code;
    int x, y, row, col;
    bdf_glyph_t *glyph;
    _XmuttFGridPageInfo *pi;
    XmuttFontGridSelectionStruct cb;

    fw = (XmuttFontGridWidget) w;

    pi = (fw->fgrid.unencoded == False) ?
        &fw->fgrid.npage : & fw->fgrid.upage;

    /*
     * If there is no selection yet, call the StartSelection routine.
     */
    if (pi->sel_start == pi->sel_end) {
        StartSelection(w, event, params, num_params);
        return;
    }

    /*
     * Figure out which cell has been selected.
     */
    x = event->xbutton.x;
    y = event->xbutton.y;
    row = (y - fw->fgrid.yoff - _XmuttFGridTopY(fw)) / fw->fgrid.cell_height;
    col = (x - fw->fgrid.xoff - _XmuttFGridLeftX(fw)) / fw->fgrid.cell_width;

    if (fw->fgrid.orientation == XmHORIZONTAL)
      code = pi->bcode + (row * fw->fgrid.cell_cols) + col;
    else
      code = pi->bcode + (col * fw->fgrid.cell_rows) + row;

    /*
     * Determine what kind of redrawing needs to be done.
     */
    call_extend = False;
    if (code > pi->sel_end) {
        call_extend = True;
        if (code <= pi->sel_start)
          _XmuttFGridDeselectRange(fw, pi->sel_end, code - 1);
        else {
            if (pi->sel_end > pi->sel_start)
              _XmuttFGridSelectRange(fw, pi->sel_end, code);
            else {
                _XmuttFGridSelectRange(fw, pi->sel_start, code);
                pi->sel_start = pi->sel_end;
            }
        }
    } else if (code < pi->sel_end) {
        call_extend = True;
        if (code < pi->sel_start) {
            if (pi->sel_end > pi->sel_start) {
                _XmuttFGridSelectRange(fw, code, pi->sel_start);
                pi->sel_start = pi->sel_end;
            } else
              _XmuttFGridSelectRange(fw, code, pi->sel_end);
        } else
          _XmuttFGridDeselectRange(fw, code + 1, pi->sel_end);
    }
    pi->sel_end = code;

    /*
     * Swap the selection start and end points if necessary.
     */
    if (pi->sel_end < pi->sel_start) {
        pi->sel_end = pi->sel_start;
        pi->sel_start = code;
    }

    if (call_extend == True) {
        if (pi->sel_start == pi->sel_end) {
            if ((glyph = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
                empty_glyph.encoding = code;
                glyph = &empty_glyph;
            }
            cb.glyphs = glyph;
            cb.num_glyphs = 1;
        }
        cb.reason = XmuttFG_EXTEND_SELECTION;
        cb.start = pi->sel_start;
        cb.end = pi->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_extend, (XtPointer) &cb);
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
    XmuttFontGridWidget fw;
    int type;
    long code;
    Boolean redraw;
    _XmuttFGridPageInfo *pi, *opi;

    if (*num_params == 0)
      return;

    fw = (XmuttFontGridWidget) w;
    if (params[0][0] == 'l') {
        if (fw->fgrid.owns_clipboard == False)
          return;

        fw->fgrid.owns_clipboard = False;
        if (fw->fgrid.unencoded == False) {
            pi = &fw->fgrid.npage;
            opi = &fw->fgrid.upage;
        } else {
            pi = &fw->fgrid.npage;
            opi = &fw->fgrid.upage;
        }

        code = -1;
        redraw = False;
        if (pi->sel_start != pi->sel_end) {
            code = pi->sel_start;
            redraw = True;
        } else if (opi->sel_start != opi->sel_end)
          code = opi->sel_start;

        _XmuttFGridDeselectAll(fw);

        if (redraw == True) {
            pi->sel_start = pi->sel_end = code;
            Select(code, pi->selmap);
            _XmuttFGridDrawCells(fw, code, code, True, True);
        } else {
            opi->sel_start = opi->sel_end = code;
            Select(code, opi->selmap);
        }

        return;
    }

    /*
     * The remaining clipboard operations can only take place if the widget
     * has the focus.
     */
    if (fw->fgrid.hasfocus == False)
      return;

    type = XmuttFG_NORMAL_PASTE;

    if (params[0][0] == 'p') {
        /*
         * If the second parameter exists, assume the paste should be an
         * insert instead of an overwrite.
         */
        if (*num_params > 1) {
            switch (params[1][0]) {
              case 'i': type = XmuttFG_INSERT_PASTE; break;
              case 'm': type = XmuttFG_MERGE_PASTE; break;
            }
        }
        XmuttFontGridPasteSelection(w, type);
    } else if (params[0][0] == 'c') {
        if (params[0][1] == 'o')
          XmuttFontGridCopySelection(w);
        else
          XmuttFontGridCutSelection(w);
    }
}

static void
#ifndef _NO_PROTO
Page(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
Page(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;

    if (*num_params == 0)
      return;

    fw = (XmuttFontGridWidget) w;
    fw->fgrid.from_keyboard = True;

    switch (params[0][0]) {
      case 'f': XmuttFontGridFirstPage(w); break;
      case 'l': XmuttFontGridLastPage(w); break;
      case 'p': XmuttFontGridPreviousPage(w); break;
      case 'n': XmuttFontGridNextPage(w); break;
    }

    fw->fgrid.from_keyboard = False;
}

static void
#ifndef _NO_PROTO
SetCount(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
SetCount(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;

    if (*num_params < 1) {
        XBell(XtDisplay(w), 0);
        return;
    }

    fw = (XmuttFontGridWidget) w;

    if (params[0][0] >= '0' && params[0][0] <= '9')
      fw->fgrid.count = (fw->fgrid.count * 10) + params[0][0] - '0';
}

static void
#ifndef _NO_PROTO
MoveCursor(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
MoveCursor(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi, *opi;
    long code, pno, cnt;
    bdf_glyph_t *gp;
    bdf_font_t *font;
    XmuttFontGridSelectionStruct cb;

    if (*num_params < 1) {
        XBell(XtDisplay(w), 0);
        return;
    }

    fw = (XmuttFontGridWidget) w;

    font = fw->fgrid.font;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        opi = &fw->fgrid.upage;
        gp = (font->glyphs) ? font->glyphs + (font->glyphs_used - 1) : 0;
    } else {
        pi = &fw->fgrid.upage;
        opi = &fw->fgrid.npage;
        gp = (font->unencoded) ?
            font->unencoded + (font->unencoded_used - 1) : 0;
    }

    code = pi->sel_start;
    if ((cnt = fw->fgrid.count) == 0)
      cnt = 1;

    switch (params[0][0]) {
      case 'l':
        if (code == 0) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmVERTICAL)
          code -= (fw->fgrid.cell_rows * cnt);
        else
          code -= cnt;

        if (code < 0)
          code = 0;
        break;
      case 'r':
        /*
         * Make sure that when on the unencoded pages, the final glyph is
         * the limit unlike the encoded pages where the max value is 0xffff.
         */
        if ((fw->fgrid.unencoded == True &&
             (gp == 0 || code == gp->encoding)) ||
            code == 0xffff) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmVERTICAL)
          code += (fw->fgrid.cell_rows * cnt);
        else
          code += cnt;

        if (fw->fgrid.unencoded == True)
          code = (gp) ? gp->encoding : 0;
        else if (code > 0xffff)
          code = 0xffff;

        break;
      case 'u':
        if (code == 0) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmHORIZONTAL)
          code -= (fw->fgrid.cell_cols * cnt);
        else
          code -= cnt;

        if (code < 0)
          code = 0;
        break;
      case 'd':
        /*
         * Make sure that when on the unencoded pages, the final glyph is
         * the limit unlike the encoded pages where the max value is 0xffff.
         */
        if ((fw->fgrid.unencoded == True &&
             (gp == 0 || code == gp->encoding)) ||
            code == 0xffff) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmHORIZONTAL)
          code += (fw->fgrid.cell_cols * cnt);
        else
          code += cnt;

        if (fw->fgrid.unencoded == True)
          code = (gp) ? gp->encoding : 0;
        else if (code > 0xffff)
          code = 0xffff;

        break;
    }

    /*
     * This turns off the selection which means the cursor is effectively
     * turned off even for the XmuttFontGridGotoPage() call.  The reason is
     * that for keyboard navigation, the cursor should move up and down by
     * rows and not whole pages when a page change occurs.
     */
    _XmuttFGridDeselectAll(fw);

    /*
     * If the code is on another page, go to that page, otherwise, just redraw
     * cells on on the current page.
     */
    pno = code / fw->fgrid.pagesize;
    if (pno != pi->pageno) {
        /*
         * Indicate that the selection start callback should not be called
         * because it is handled locally.
         */
        fw->fgrid.no_sel_callback = True;
        XmuttFontGridGotoPage(w, pno);
    }

    pi->sel_start = pi->sel_end = code;
    Select(code, pi->selmap);
    _XmuttFGridDrawCells(fw, code, code, False, True);

    /*
     * Reset the cursor motion count.
     */
    fw->fgrid.count = 0;

    if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
        empty_glyph.encoding = code;
        cb.glyphs = &empty_glyph;
    }
    cb.num_glyphs = 1;
    cb.reason = XmuttFG_START_SELECTION;
    cb.start = pi->sel_start;
    cb.end = pi->sel_end;
    cb.base = fw->fgrid.base;
    cb.unencoded = fw->fgrid.unencoded;
    XtCallCallbackList(w, fw->fgrid.sel_start, (XtPointer) &cb);
}

static void
#ifndef _NO_PROTO
ExtendCursor(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ExtendCursor(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi, *opi;
    long code, pno;
    Boolean call_extend;
    XmuttFontGridSelectionStruct cb;

    if (*num_params < 1) {
        XBell(XtDisplay(w), 0);
        return;
    }

    fw = (XmuttFontGridWidget) w;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        opi = &fw->fgrid.upage;
    } else {
        pi = &fw->fgrid.upage;
        opi = &fw->fgrid.npage;
    }

    code = pi->sel_end;
    if ((pno = fw->fgrid.count) == 0)
      pno = 1;

    switch (params[0][0]) {
      case 'l':
        if (code == 0) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmVERTICAL)
          code -= (fw->fgrid.cell_rows * pno);
        else
          code -= pno;

        if (code < 0)
          code = 0;
        break;
      case 'r':
        if (code == 0xffff) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmVERTICAL)
          code += (fw->fgrid.cell_rows * pno);
        else
          code += pno;

        if (code > 0xffff)
          code = 0xffff;
        break;
      case 'u':
        if (code == 0) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmHORIZONTAL)
          code -= (fw->fgrid.cell_cols * pno);
        else
          code -= pno;

        if (code < 0)
          code = 0;
        break;
      case 'd':
        if (code == 0xffff) {
            XBell(XtDisplay(w), 0);
            return;
        }

        if (fw->fgrid.orientation == XmHORIZONTAL)
          code += (fw->fgrid.cell_cols * pno);
        else
          code += pno;

        if (code > 0xffff)
          code = 0xffff;
        break;
    }

    call_extend = False;
    if (code > pi->sel_end) {
        call_extend = True;
        if (code <= pi->sel_start)
          _XmuttFGridDeselectRange(fw, pi->sel_end, code - 1);
        else {
            if (pi->sel_end < pi->sel_start) {
                _XmuttFGridDeselectRange(fw, pi->sel_end, pi->sel_start - 1);
                _XmuttFGridSelectRange(fw, pi->sel_start + 1, code);
            } else
              _XmuttFGridSelectRange(fw, pi->sel_end, code);
        }
    } else if (code < pi->sel_end) {
        call_extend = True;
        if (code < pi->sel_start) {
            if (pi->sel_end > pi->sel_start) {
                _XmuttFGridDeselectRange(fw, pi->sel_start + 1, pi->sel_end);
                _XmuttFGridSelectRange(fw, code, pi->sel_start);
            } else
              _XmuttFGridSelectRange(fw, code, pi->sel_end);
        } else
          _XmuttFGridDeselectRange(fw, code + 1, pi->sel_end);
    }

    /*
     * Reset the count.
     */
    fw->fgrid.count = 0;

    pi->sel_end = code;

    /*
     * If the selection endpoint is on some page other than the current
     * page, make sure the page holding the end point is made visible.
     */
    pno = code / fw->fgrid.pagesize;
    if (pno != pi->pageno)
      XmuttFontGridGotoPage(w, pno);

    if (call_extend == True) {
        if (pi->sel_start == pi->sel_end) {
            if ((cb.glyphs = _XmuttFGridCodeToGlyph(fw, code)) == 0) {
                empty_glyph.encoding = code;
                cb.glyphs = &empty_glyph;
            }
            cb.num_glyphs = 1;
        }
        cb.reason = XmuttFG_EXTEND_SELECTION;
        cb.start = pi->sel_start;
        cb.end = pi->sel_end;
        cb.base = fw->fgrid.base;
        cb.unencoded = fw->fgrid.unencoded;
        XtCallCallbackList(w, fw->fgrid.sel_extend, (XtPointer) &cb);
    }
}

static void
#ifndef _NO_PROTO
PageExtendCursor(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
PageExtendCursor(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    String parm = 0;
    Cardinal nparm = 1;
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi, *opi;

    if (*num_params < 1) {
        XBell(XtDisplay(w), 0);
        return;
    }

    fw = (XmuttFontGridWidget) w;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        opi = &fw->fgrid.upage;
    } else {
        pi = &fw->fgrid.upage;
        opi = &fw->fgrid.npage;
    }


    switch (params[0][0]) {
      case 'f':
        parm = "left";
        fw->fgrid.count = (pi->pageno - pi->minpage) * fw->fgrid.pagesize;
        break;
      case 'l':
        parm = "right";
        fw->fgrid.count = (pi->maxpage - pi->pageno) * fw->fgrid.pagesize;
        break;
      case 'p':
        parm = "left";
        if (fw->fgrid.count == 0)
          fw->fgrid.count = 1;
        fw->fgrid.count *= fw->fgrid.pagesize;
        break;
      case 'n':
        parm = "right";
        if (fw->fgrid.count == 0)
          fw->fgrid.count = 1;
        fw->fgrid.count *= fw->fgrid.pagesize;
        break;
    }

    if (parm == 0)
      return;

    ExtendCursor(w, event, &parm, &nparm);
}

static void
#ifndef _NO_PROTO
ActivateCursor(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
ActivateCursor(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi, *opi;
    long code;
    XmuttFontGridSelectionStruct cb;

    fw = (XmuttFontGridWidget) w;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        opi = &fw->fgrid.upage;
    } else {
        pi = &fw->fgrid.upage;
        opi = &fw->fgrid.npage;
    }

    if (opi->sel_start != -1) {
        /*
         * The other page set has the selection, so change back to it before
         * making the callback.
         */
        if (fw->fgrid.unencoded == False)
          XmuttFontGridViewUnencoded(w, True);
        else
          XmuttFontGridViewUnencoded(w, False);

        if (fw->fgrid.unencoded == False) {
            pi = &fw->fgrid.npage;
            opi = &fw->fgrid.upage;
        } else {
            pi = &fw->fgrid.upage;
            opi = &fw->fgrid.npage;
        }
    }

    code = pi->sel_start;

    /*
     * Now if a selection larger than one exists, clear it before calling the
     * callback.
     */
    if (pi->sel_end - pi->sel_start > 1) {
        _XmuttFGridDeselectAll(fw);
        Select(code, pi->selmap);
        _XmuttFGridDrawCells(fw, code, code, False, True);
    }

    cb.num_glyphs = 1;
    cb.glyphs = _XmuttFGridCodeToGlyph(fw, code);
    if (cb.glyphs == 0) {
        empty_glyph.encoding = code;
        cb.glyphs = &empty_glyph;
    }

    cb.reason = XmuttFG_ACTIVATE;
    cb.start = cb.end = code;
    cb.base = fw->fgrid.base;
    cb.unencoded = fw->fgrid.unencoded;
    XtCallCallbackList(w, fw->fgrid.activate, (XtPointer) &cb);
}

/*
 * This routine is provided specially for triggering an end selection
 * callback when a single glyph has been selected.
 */
static void
#ifndef _NO_PROTO
SendEndSelection(Widget w, XEvent *event, String *params, Cardinal *num_params)
#else
SendEndSelection(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    XmuttFontGridWidget fw;
    _XmuttFGridPageInfo *pi;
    long i, ng;
    bdf_font_t *font;
    bdf_glyph_t *glyphs;
    XmuttFontGridSelectionStruct cb;

    fw = (XmuttFontGridWidget) w;

    font = fw->fgrid.font;
    ng = 0;
    glyphs = 0;
    cb.glyphs = 0;
    cb.num_glyphs = 0;

    if (fw->fgrid.unencoded == False) {
        pi = &fw->fgrid.npage;
        if (font) {
            glyphs = font->glyphs;
            ng = font->glyphs_used;
        }
    } else {
        pi = &fw->fgrid.upage;
        if (font) {
            glyphs = font->unencoded;
            ng = font->unencoded_used;
        }
    }

    /*
     * If the selection has more than one glyph, then there is no reason
     * to do any more.
     */
    if (pi->sel_start != pi->sel_end)
      return;

    /*
     * Position the glyphs if there are any.
     */
    for (i = 0; i < ng && glyphs->encoding < pi->sel_start; i++, glyphs++) ;
    cb.glyphs = glyphs;

    for (; i < ng && glyphs->encoding <= pi->sel_end; i++, glyphs++) ;
    cb.num_glyphs = glyphs - cb.glyphs;

    /*
     * If the selection contains more than one glyph, then attempt to
     * gain control of the clipboard.
     */
    if (pi->sel_end - pi->sel_start != 0)
      _XmuttFGridOwnClipboard(w);

    cb.reason = XmuttFG_END_SELECTION;
    cb.start = pi->sel_start;
    cb.end = pi->sel_end;
    cb.base = fw->fgrid.base;
    cb.unencoded = fw->fgrid.unencoded;
    XtCallCallbackList(w, fw->fgrid.sel_end, (XtPointer) &cb);
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

#if defined(XlibSpecificationRelease) && (XlibSpecificationRelease == 6)

static char default_trans[] = "\
<SelClr>: handle-clipboard(lose)\n\
<FocusIn>: gain-focus()\n\
<FocusOut>: lose-focus()\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Return: activate-cursor()\n\
~Shift Ctrl ~Meta ~Alt<Key>Return: send-end-selection()\n\
Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(prev)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(next)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Right: move-cursor(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Left: move-cursor(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Up: move-cursor(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Down: move-cursor(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Right: move-cursor(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Left: move-cursor(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Up: move-cursor(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Down: move-cursor(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfRight: move-cursor(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfLeft: move-cursor(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfUp: move-cursor(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfDown: move-cursor(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Right: extend-cursor(right)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Left: extend-cursor(left)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Up: extend-cursor(up)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Down: extend-cursor(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Right: extend-cursor(right)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Left: extend-cursor(left)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Up: extend-cursor(up)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Down: extend-cursor(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfRight: extend-cursor(right)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfLeft: extend-cursor(left)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfUp: extend-cursor(up)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfDown: extend-cursor(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Btn1Down>: start-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Btn1Motion>: extend-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Btn1Up>: end-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Btn2Down>: start-selection() handle-clipboard(paste)\n\
Shift ~Ctrl ~Meta ~Alt<Btn2Down>: start-selection() handle-clipboard(paste,insert)\n\
~Shift Ctrl ~Meta ~Alt<Btn2Down>: start-selection() handle-clipboard(paste,merge)\n\
~Shift ~Ctrl ~Meta ~Alt<Btn3Down>: handle-clipboard(copy)\n\
Shift ~Ctrl ~Meta ~Alt<Btn1Down>: adjust-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfBeginLine: page(first)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfEndLine: page(last)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfPageUp: page(previous)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfPageDown: page(next)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Home: page(first)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_End: page(last)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Prior: page(previous)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Next: page(next)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Home: page(first)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>End: page(last)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Prior: page(previous)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Next: page(next)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfBeginLine: page-extend-cursor(first)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfEndLine: page-extend-cursor(last)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfPageUp: page-extend-cursor(previous)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfPageDown: page-extend-cursor(next)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Home: page-extend-cursor(first)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_End: page-extend-cursor(last)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Prior: page-extend-cursor(previous)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Next: page-extend-cursor(next)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Home: page-extend-cursor(first)\n\
Shift ~Ctrl ~Meta ~Alt<Key>End: page-extend-cursor(last)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Prior: page-extend-cursor(previous)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Next: page-extend-cursor(next)\n\
<Key>SunCopy: handle-clipboard(copy)\n\
<Key>SunCut: handle-clipboard(cut)\n\
<Key>SunPaste: handle-clipboard(paste)\n\
<Key>osfCopy: handle-clipboard(copy)\n\
<Key>osfCut: handle-clipboard(cut)\n\
<Key>osfPaste: handle-clipboard(paste)\n\
<Key>osfBackSpace: handle-clipboard(cut)\n\
<Key>osfDelete: handle-clipboard(cut)\n\
<Key>BackSpace: handle-clipboard(cut)\n\
<Key>Delete: handle-clipboard(cut)\n\
<Key>0: set-count(0)\n\
<Key>1: set-count(1)\n\
<Key>2: set-count(2)\n\
<Key>3: set-count(3)\n\
<Key>4: set-count(4)\n\
<Key>5: set-count(5)\n\
<Key>6: set-count(6)\n\
<Key>7: set-count(7)\n\
<Key>8: set-count(8)\n\
<Key>9: set-count(9)\n\
";

#else /* R5 or lower */

static char default_trans[] = "\
<SelClr>: handle-clipboard(lose)\n\
<FocusIn>: gain-focus()\n\
<FocusOut>: lose-focus()\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Return: activate-cursor()\n\
~Shift Ctrl ~Meta ~Alt<Key>Return: send-end-selection()\n\
Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(prev)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Tab: traverse(next)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Right: move-cursor(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Left: move-cursor(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Up: move-cursor(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>KP_Down: move-cursor(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Right: move-cursor(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Left: move-cursor(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Up: move-cursor(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Down: move-cursor(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfRight: move-cursor(right)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfLeft: move-cursor(left)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfUp: move-cursor(up)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfDown: move-cursor(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Right: extend-cursor(right)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Left: extend-cursor(left)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Up: extend-cursor(up)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Down: extend-cursor(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Right: extend-cursor(right)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Left: extend-cursor(left)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Up: extend-cursor(up)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Down: extend-cursor(down)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfRight: extend-cursor(right)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfLeft: extend-cursor(left)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfUp: extend-cursor(up)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfDown: extend-cursor(down)\n\
~Shift ~Ctrl ~Meta ~Alt<Btn1Down>: start-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Btn1Motion>: extend-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Btn1Up>: end-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Btn2Down>: start-selection() handle-clipboard(paste)\n\
Shift ~Ctrl ~Meta ~Alt<Btn2Down>: start-selection() handle-clipboard(paste,insert)\n\
~Shift Ctrl ~Meta ~Alt<Btn2Down>: start-selection() handle-clipboard(paste,merge)\n\
~Shift ~Ctrl ~Meta ~Alt<Btn3Down>: handle-clipboard(copy)\n\
Shift ~Ctrl ~Meta ~Alt<Btn1Down>: adjust-selection()\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfBeginLine: page(first)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfEndLine: page(last)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfPageUp: page(previous)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>osfPageDown: page(next)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Home: page(first)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>End: page(last)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Prior: page(previous)\n\
~Shift ~Ctrl ~Meta ~Alt<Key>Next: page(next)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfBeginLine: page-extend-cursor(first)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfEndLine: page-extend-cursor(last)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfPageUp: page-extend-cursor(previous)\n\
Shift ~Ctrl ~Meta ~Alt<Key>osfPageDown: page-extend-cursor(next)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Home: page-extend-cursor(first)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_End: page-extend-cursor(last)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Prior: page-extend-cursor(previous)\n\
Shift ~Ctrl ~Meta ~Alt<Key>KP_Next: page-extend-cursor(next)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Home: page-extend-cursor(first)\n\
Shift ~Ctrl ~Meta ~Alt<Key>End: page-extend-cursor(last)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Prior: page-extend-cursor(previous)\n\
Shift ~Ctrl ~Meta ~Alt<Key>Next: page-extend-cursor(next)\n\
<Key>SunCopy: handle-clipboard(copy)\n\
<Key>SunCut: handle-clipboard(cut)\n\
<Key>SunPaste: handle-clipboard(paste)\n\
<Key>osfCopy: handle-clipboard(copy)\n\
<Key>osfCut: handle-clipboard(cut)\n\
<Key>osfPaste: handle-clipboard(paste)\n\
<Key>osfBackSpace: handle-clipboard(cut)\n\
<Key>osfDelete: handle-clipboard(cut)\n\
<Key>BackSpace: handle-clipboard(cut)\n\
<Key>Delete: handle-clipboard(cut)\n\
<Key>0: set-count(0)\n\
<Key>1: set-count(1)\n\
<Key>2: set-count(2)\n\
<Key>3: set-count(3)\n\
<Key>4: set-count(4)\n\
<Key>5: set-count(5)\n\
<Key>6: set-count(6)\n\
<Key>7: set-count(7)\n\
<Key>8: set-count(8)\n\
<Key>9: set-count(9)\n\
";

#endif /* R5 or lower */

static XtActionsRec actions[] = {
    {"gain-focus",         GainFocus},
    {"lose-focus",         LoseFocus},
    {"traverse",           Traverse},
    {"start-selection",    StartSelection},
    {"extend-selection",   ExtendSelection},
    {"end-selection",      EndSelection},
    {"adjust-selection",   AdjustSelection},
    {"handle-clipboard",   HandleClipboard},
    {"page",               Page},
    {"set-count",          SetCount},
    {"move-cursor",        MoveCursor},
    {"extend-cursor",      ExtendCursor},
    {"activate-cursor",    ActivateCursor},
    {"send-end-selection", SendEndSelection},
    {"page-extend-cursor", PageExtendCursor},
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
        XmNmarginWidth,
        XmCMarginWidth,
        XmRHorizontalDimension,
        sizeof(Dimension),
        XtOffset(XmuttFontGridWidget, fgrid.mwidth),
        XmRImmediate,
        (XtPointer) 2,
    },
    {
        XmNmarginHeight,
        XmCMarginHeight,
        XmRVerticalDimension,
        sizeof(Dimension),
        XtOffset(XmuttFontGridWidget, fgrid.mheight),
        XmRImmediate,
        (XtPointer) 2,
    },
    {
        XmNbdfFont,
        XmCBDFFont,
        XmRPointer,
        sizeof(bdf_font_t *),
        XtOffset(XmuttFontGridWidget, fgrid.font),
        XmRImmediate,
        0,
    },
    {
        XmNcodeBase,
        XmCCodeBase,
        XmRInt,
        sizeof(int),
        XtOffset(XmuttFontGridWidget, fgrid.base),
        XmRImmediate,
        (XtPointer) 16,
    },
    {
        XmNpointSize,
        XmCPointSize,
        XmRInt,
        sizeof(long),
        XtOffset(XmuttFontGridWidget, fgrid.ptsize),
        XmRImmediate,
        (XtPointer) 12,
    },
    {
        XmNhorizontalResolution,
        XmCHorizontalResolution,
        XmRInt,
        sizeof(long),
        XtOffset(XmuttFontGridWidget, fgrid.hres),
        XmRCallProc,
        (XtPointer) _XmuttFGridDefaultHResolution,
    },
    {
        XmNverticalResolution,
        XmCVerticalResolution,
        XmRInt,
        sizeof(long),
        XtOffset(XmuttFontGridWidget, fgrid.vres),
        XmRCallProc,
        (XtPointer) _XmuttFGridDefaultVResolution,
    },
    {
        XmNfontSpacing,
        XmCFontSpacing,
        XmRInt,
        sizeof(long),
        XtOffset(XmuttFontGridWidget, fgrid.spacing),
        XmRImmediate,
        (XtPointer) BDF_PROPORTIONAL,
    },
    {
        XmNcellRows,
        XmCCellRows,
        XmRDimension,
        sizeof(Dimension),
        XtOffset(XmuttFontGridWidget, fgrid.cell_rows),
        XmRImmediate,
        (XtPointer) FGRID_DEFAULT_ROWS,
    },
    {
        XmNcellColumns,
        XmCCellColumns,
        XmRDimension,
        sizeof(Dimension),
        XtOffset(XmuttFontGridWidget, fgrid.cell_cols),
        XmRImmediate,
        (XtPointer) FGRID_DEFAULT_COLS,
    },
    {
        XmNskipBlankPages,
        XmCSkipBlankPages,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttFontGridWidget, fgrid.noblanks),
        XmRImmediate,
        (XtPointer) True,
    },
    {
        XmNdisplayUnencoded,
        XmCDisplayUnencoded,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttFontGridWidget, fgrid.unencoded),
        XmRImmediate,
        (XtPointer) False,
    },
    {
        XmNgridOverwriteMode,
        XmCGridOverwriteMode,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttFontGridWidget, fgrid.overwrite_mode),
        XmRImmediate,
        (XtPointer) True,
    },
    {
        XmNactivateCallback,
        XmCCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttFontGridWidget, fgrid.activate),
        XmRImmediate,
        0,
    },
    {
        XmNpageCallback,
        XmCPageCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttFontGridWidget, fgrid.page),
        XmRImmediate,
        0,
    },
    {
        XmNcodePageCallback,
        XmCCodePageCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttFontGridWidget, fgrid.code_page),
        XmRImmediate,
        0,
    },
    {
        XmNstartSelectionCallback,
        XmCStartSelectionCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttFontGridWidget, fgrid.sel_start),
        XmRImmediate,
        0,
    },
    {
        XmNextendSelectionCallback,
        XmCExtendSelectionCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttFontGridWidget, fgrid.sel_extend),
        XmRImmediate,
        0,
    },
    {
        XmNendSelectionCallback,
        XmCEndSelectionCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttFontGridWidget, fgrid.sel_end),
        XmRImmediate,
        0,
    },
    {
        XmNmodifiedCallback,
        XmCModifiedCallback,
        XmRCallback,
        sizeof(XtCallbackList),
        XtOffset(XmuttFontGridWidget, fgrid.modified),
        XmRImmediate,
        0,
    },
    {
        XmNmultiClickTime,
        XmCMultiClickTime,
        XmRInt,
        sizeof(int),
        XtOffset(XmuttFontGridWidget, fgrid.mclick),
        XmRImmediate,
        (XtPointer) 475,
    },
    {
        XmNorientation,
        XmCOrientation,
        XmROrientation,
        sizeof(unsigned char),
        XtOffset(XmuttFontGridWidget, fgrid.orientation),
        XmRImmediate,
        (XtPointer) XmHORIZONTAL,
    },
    {
        XmNpowersOfTwo,
        XmCPowersOfTwo,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttFontGridWidget, fgrid.power2),
        XmRImmediate,
        (XtPointer) False,
    },
    {
        XmNbitsPerPixel,
        XmCBitsPerPixel,
        XmRInt,
        sizeof(int),
        XtOffset(XmuttFontGridWidget, fgrid.bpp),
        XmRImmediate,
        (XtPointer) 1,
    },
    {
        XmNcolorList,
        XmCColorList,
        XmRPointer,
        sizeof(unsigned long *),
        XtOffset(XmuttFontGridWidget, fgrid.colors),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNinitialGlyph,
        XmCInitialGlyph,
        XmRInt,
        sizeof(long),
        XtOffset(XmuttFontGridWidget, fgrid.initial_glyph),
        XmRImmediate,
        (XtPointer) -1,
    },
};

externaldef(xmuttfgridclassrec) XmuttFGridClassRec xmuttFGridClassRec = {
    {
	(WidgetClass) &xmPrimitiveClassRec,	/* superclass		*/
	"XmuttFontGrid",			/* class_name		*/
	sizeof(XmuttFGridRec),			/* widget_size		*/
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
        0,					/* translations		*/
        0,					/* arm_and_activate	*/
        0,					/* syn resources	*/
        0,					/* num syn_resources	*/
        0,					/* extension		*/
    },
    {						/* XmuttFontGrid	*/
        0,					/* Class reference count.*/
        0,					/* Extension record pointer.*/
    }
};

externaldef(xmuttfontgridwidgetclass)
    WidgetClass xmuttFontGridWidgetClass = (WidgetClass) &xmuttFGridClassRec;

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
static char rcsid[] __attribute__ ((unused)) = "$Id: bdfgrab.c,v 1.5 2000/03/16 20:08:51 mleisher Exp $";
#else
static char rcsid[] = "$Id: bdfgrab.c,v 1.5 2000/03/16 20:08:51 mleisher Exp $";
#endif
#endif

/*
 * This file will only be compiled if the BDF_NO_X11 macro is *not* defined.
 */
#ifndef BDF_NO_X11

/*
 * Code to get BDF fonts from the X server.  Reimplementation of the famous
 * "getbdf" program by the equally famous der Mouse :-)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xmu/Error.h>
#include "bdfP.h"

/*
 * Routine to compare two glyphs by encoding so they can be sorted.
 */
static int
#ifdef __STDC__
by_encoding(const void *a, const void *b)
#else
by_encoding(a, b)
char *a, *b;
#endif
{
    bdf_glyph_t *c1, *c2;

    c1 = (bdf_glyph_t *) a;
    c2 = (bdf_glyph_t *) b;
    if (c1->encoding < c2->encoding)
      return -1;
    else if (c1->encoding > c2->encoding)
      return 1;
    return 0;
}

static void
#ifdef __STDC__
_bdf_get_glyphs(Display *d, XFontStruct *f, bdf_font_t *font,
                bdf_callback_t callback, void *data)
#else
_bdf_get_glyphs(d, f, font, callback, data)
Display *d;
XFontStruct *f;
bdf_font_t *font;
bdf_callback_t callback;
void *data;
#endif
{
    unsigned long off, b1, b2, black, x, y, bpr;
    GC cleargc, drawgc;
    Pixmap canvas;
    XImage *image;
    XCharStruct *cp;
    bdf_glyph_t *gp;
    XChar2b ch;
    bdf_callback_struct_t cb;
    char name[16];
    XGCValues gcv;

    black = BlackPixel(d, DefaultScreen(d));

    /*
     * Create the Pixmap which will be used to draw the glyphs.
     */
    canvas = XCreatePixmap(d, XRootWindow(d, DefaultScreen(d)),
                           font->bbx.width, font->bbx.height, 1);

    /*
     * Create the graphics contexts for drawing.
     */
    gcv.function = GXcopy;
    gcv.foreground = WhitePixel(d, DefaultScreen(d));
    cleargc = XCreateGC(d, canvas, GCFunction|GCForeground, &gcv);

    gcv.background = gcv.foreground;
    gcv.foreground = black;
    gcv.font = f->fid;
    drawgc = XCreateGC(d, canvas,
                       GCFunction|GCForeground|GCBackground|GCFont, &gcv);

    /*
     * Do it.
     */
    for (b1 = f->min_byte1; b1 <= f->max_byte1; b1++) {

        off = (b1 - f->min_byte1) *
            (f->max_char_or_byte2 + 1 - f->min_char_or_byte2);

        for (b2 = f->min_char_or_byte2; b2 <= f->max_char_or_byte2;
             b2++, off++) {

            /*
             * Point at the glyph metrics.
             */
            cp = (f->per_char != 0) ? f->per_char + off : &f->min_bounds;

            if (cp->lbearing || cp->rbearing || cp->width ||
                cp->ascent || cp->descent) {
                /*
                 * Make sure there is enough glyph storage to handle
                 * the glyphs.
                 */
                if (font->glyphs_used == font->glyphs_size) {
                    if (font->glyphs_size == 0)
                      font->glyphs = (bdf_glyph_t *)
                          malloc(sizeof(bdf_glyph_t) * 16);
                    else
                      font->glyphs = (bdf_glyph_t *)
                          realloc((char *) font->glyphs,
                                  sizeof(bdf_glyph_t) *
                                  (font->glyphs_size + 16));
                    font->glyphs_size += 16;
                }

                /*
                 * Point at the next glyph structure.
                 */
                gp = font->glyphs + font->glyphs_used++;

                /*
                 * Determine the glyph encoding and set the metrics.
                 */
                gp->encoding = (b1 << 8) | b2;
                gp->dwidth = cp->width;
                gp->swidth = (unsigned short) (cp->width * 72000.0
                  / (font->point_size * font->resolution_x));
                gp->bbx.width = cp->rbearing - cp->lbearing;
                gp->bbx.x_offset = cp->lbearing;
                gp->bbx.ascent = cp->ascent;
                gp->bbx.descent = cp->descent;
                gp->bbx.y_offset = -cp->descent;
                gp->bbx.height = cp->ascent + cp->descent;

                /*
                 * Create a glyph name.
                 */
                sprintf(name, "char%ld", gp->encoding);
                gp->name = (char *) malloc(strlen(name) + 1);
                (void) strcpy(gp->name, name);

                /*
                 * Determine the number of bytes that will be needed for this
                 * glyph.
                 */
                bpr = (gp->bbx.width + 7) >> 3;
                gp->bytes = bpr * gp->bbx.height;
                gp->bitmap = (unsigned char *) malloc(gp->bytes);
                (void) memset((char *) gp->bitmap, 0, gp->bytes);

                /*
                 * Clear the canvas.
                 */
                XFillRectangle(d, canvas, cleargc, 0, 0,
                               font->bbx.width, font->bbx.height);

                /*
                 * Render the glyph.
                 */
                ch.byte1 = (b1 == 0) ? (b2 >> 8) : b1;
                ch.byte2 = b2;
                XDrawString16(d, canvas, drawgc, -cp->lbearing, cp->ascent,
                              &ch, 1);
                image = XGetImage(d, canvas, 0, 0,
                                  font->bbx.width, font->bbx.height, 1L,
                                  XYPixmap);
                for (y = 0; y < gp->bbx.height; y++) {
                    for (x = 0; x < gp->bbx.width; x++) {
                        if (XGetPixel(image, x, y) == black)
                          gp->bitmap[(y * bpr) + (x >> 3)] |=
                              (0x80 >> (x & 7));
                    }
                }
                XDestroyImage(image);

                /*
                 * Call the update callback if necessary.
                 */
                if (callback != 0) {
                    cb.reason = BDF_LOADING;
                    cb.total = font->glyphs_size;
                    cb.current = font->glyphs_used;
                    (*callback)(&cb, data);
                }
            }
        }
    }

    /*
     * Delete the Pixmap and the GCs.
     */
    XFreePixmap(d, canvas);
    XFreeGC(d, cleargc);
    XFreeGC(d, drawgc);

    /*
     * Make sure the glyphs are sorted by encoding.
     */
    qsort((char *) font->glyphs, font->glyphs_used, sizeof(bdf_glyph_t),
          by_encoding);
}

static int
#ifdef __STDC__
error_handler(Display *d, XErrorEvent *event)
#else
error_handler(d, event)
Display *d;
XErrorEvent *event;
#endif
{

    if (event->request_code != X_GetAtomName)
        XmuPrintDefaultErrorMessage(d, event, stderr);
    return 0;
}

bdf_font_t *
#ifdef __STDC__
bdf_load_server_font(Display *d, XFontStruct *f, char *name,
                     bdf_options_t *opts, bdf_callback_t callback, void *data)
#else
bdf_load_server_font(d, f, name, opts, callback, data)
Display *d;
XFontStruct *f;
char *name;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
#endif
{
    unsigned long i, len, b1, b2;
    bdf_font_t *font;
    XFontProp *xfp;
    XCharStruct *cp;
    bdf_property_t *pp, prop;
    bdf_callback_struct_t cb;
    int (*old_error_handler)();

    if (f == 0)
      return 0;

    font = (bdf_font_t *) calloc(1, sizeof(bdf_font_t));

    font->bpp = 1;

    /*
     * Set up the font bounding box.
     */
    font->bbx.width = f->max_bounds.rbearing - f->min_bounds.lbearing;
    font->bbx.x_offset = f->min_bounds.lbearing;
    font->bbx.height = f->max_bounds.ascent + f->max_bounds.descent;
    font->bbx.ascent = f->max_bounds.ascent;
    font->bbx.descent = f->max_bounds.descent;
    font->bbx.y_offset = -font->bbx.descent;

    /*
     * If the font happens to be a character cell or monowidth, make sure that
     * value is taken from the max bounds.
     */
    font->monowidth = f->max_bounds.width;

    font->default_glyph = (long) f->default_char;

    /*
     * Now load the font properties.
     */
    old_error_handler = XSetErrorHandler(error_handler);
    for (i = 0, xfp = f->properties; i < f->n_properties; i++, xfp++) {
        if (xfp->name == XA_FONT)
          /*
           * Set the font name but don't add it to the list in the font.
           */
          font->name = XGetAtomName(d, (Atom) xfp->card32);
        else {
            /*
             * Add the property to the font.
             */
            prop.name = XGetAtomName(d, xfp->name);
            if ((pp = bdf_get_property(prop.name)) == 0) {
                /*
                 * The property does not exist, so create it with type Atom.
                 */
                bdf_create_property(prop.name, BDF_ATOM);
                pp = bdf_get_property(prop.name);
            }
            prop.format = pp->format;
            switch (prop.format) {
              case BDF_ATOM:
                prop.value.atom = XGetAtomName(d, (Atom) xfp->card32);
                break;
              case BDF_CARDINAL:
                prop.value.card32 = xfp->card32;
                break;
              case BDF_INTEGER:
                prop.value.int32 = (long) xfp->card32;
                break;
            }
            bdf_add_font_property(font, &prop);

            /*
             * Free up the Atom names returned by X.
             */
            XFree(prop.name);
            if (prop.format == BDF_ATOM)
              XFree(prop.value.atom);
        }
    }
    XSetErrorHandler(old_error_handler);

    /*
     * Now go through and initialize the various fields needed for the font.
     */

    /*
     * If the font name was not set when the properties were loaded,
     * set it to the name that was passed.
     */
    if (font->name == 0) {
        len = (unsigned long) strlen(name);
        font->name = (char *) malloc(len + 1);
        (void) memcpy(font->name, name, len + 1);
    }

    /*
     * If the font default glyph is non-zero, make sure the DEFAULT_CHAR
     * property is updated appropriately.  Otherwise, make sure the
     * DEFAULT_CHAR property is deleted from the font.
     */
    if (font->default_glyph > 0) {
        prop.name = "DEFAULT_CHAR";
        prop.format = BDF_INTEGER;
        prop.value.int32 = font->default_glyph;
        bdf_add_font_property(font, &prop);
    } else if (bdf_get_font_property(font, "DEFAULT_CHAR") != 0)
      bdf_delete_font_property(font, "DEFAULT_CHAR");

    /*
     * Check the point size.
     */
    if ((pp = bdf_get_font_property(font, "POINT_SIZE")) != 0)
      font->point_size = (pp->value.card32 / 10);
    else
      font->point_size = 12;

    /*
     * Check for the deprecated "RESOLUTION" property first in case it exists
     * and "RESOLUTION_X" and "RESOLUTION_Y" do not.
     */
    if ((pp = bdf_get_font_property(font, "RESOLUTION")) != 0)
      font->resolution_x = font->resolution_y = pp->value.int32;

    if ((pp = bdf_get_font_property(font, "RESOLUTION_X")) != 0)
      font->resolution_x = pp->value.int32;
    if ((pp = bdf_get_font_property(font, "RESOLUTION_Y")) != 0)
      font->resolution_y = pp->value.int32;

    /*
     * If the horizontal or vertical resolutions have not been set, then
     * define them to be the resolution of the display.
     */
    if (font->resolution_x == 0)
      font->resolution_x =
          (long) (((((double) DisplayWidth(d, DefaultScreen(d))) * 25.4) /
                   ((double) DisplayWidthMM(d, DefaultScreen(d)))) + 0.5);
    if (font->resolution_y == 0)
      font->resolution_y =
          (long) (((((double) DisplayHeight(d, DefaultScreen(d))) * 25.4) /
                   ((double) DisplayHeightMM(d, DefaultScreen(d)))) + 0.5);

    /*
     * Check the font ascent and descent.
     */
    if ((pp = bdf_get_font_property(font, "FONT_ASCENT")) != 0)
      font->font_ascent = pp->value.int32;
    else {
        /*
         * Add the FONT_ASCENT property.
         */
        prop.name = "FONT_ASCENT";
        prop.format = BDF_INTEGER;
        prop.value.int32 = font->bbx.ascent;
        bdf_add_font_property(font, &prop);
        font->font_ascent = font->bbx.ascent;
    }

    if ((pp = bdf_get_font_property(font, "FONT_DESCENT")) != 0)
      font->font_descent = pp->value.int32;
    else {
        /*
         * Add the FONT_DESCENT property.
         */
        prop.name = "FONT_DESCENT";
        prop.format = BDF_INTEGER;
        prop.value.int32 = font->bbx.descent;
        bdf_add_font_property(font, &prop);
        font->font_descent = font->bbx.descent;
    }

    /*
     * Get the font spacing.
     */
    font->spacing = BDF_PROPORTIONAL;
    if ((pp = bdf_get_font_property(font, "SPACING")) != 0) {
        switch (pp->value.atom[0]) {
          case 'P': case 'p': font->spacing = BDF_PROPORTIONAL; break;
          case 'M': case 'm': font->spacing = BDF_MONOWIDTH; break;
          case 'C': case 'c': font->spacing = BDF_CHARCELL; break;
        }
    }

    /*
     * Now determine the number of glyphs.
     */
    if (f->per_char != 0) {
        for (b1 = f->min_byte1; b1 <= f->max_byte1; b1++) {
            len = (b1 - f->min_byte1) *
                (f->max_char_or_byte2 + 1 - f->min_char_or_byte2);
            for (b2 = f->min_char_or_byte2; b2 <= f->max_char_or_byte2;
                 b2++, len++) {
                cp = f->per_char + len;
                /*
                 * If any of the metrics values are non-zero, then count this
                 * as a glyph.
                 */
                if (cp->lbearing || cp->rbearing || cp->width ||
                    cp->ascent || cp->descent)
                  font->glyphs_size++;
            }
        }
    } else
      font->glyphs_size = (f->max_byte1 + 1 - f->min_byte1) *
          (f->max_char_or_byte2 + 1 - f->min_char_or_byte2);

    /*
     * Call the callback if it was provided.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.total = font->glyphs_size;
        cb.current = 0;
        (*callback)(&cb, data);
    }

    /*
     * Allocate enough glyph storage for the specified number of glyphs.
     */
    font->glyphs = (bdf_glyph_t *)
        malloc(sizeof(bdf_glyph_t) * font->glyphs_size);

    /*
     * Actually load the glyphs.
     */
    _bdf_get_glyphs(d, f, font, callback, data);

    /*
     * Add a message to the font to indicate it was loaded
     * from the server.
     */
    _bdf_add_acmsg(font, "Font grabbed from the X server.", 31);

    /*
     * Mark the font as being modified.
     */
    font->modified = 1;

    return font;
}

#endif /* HAVE_XLIB */

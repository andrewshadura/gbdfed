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
static char rcsid[] __attribute__ ((unused)) = "$Id: bdfcons.c,v 1.10 2004/02/03 00:03:18 mleisher Exp $";
#else
static char rcsid[] = "$Id: bdfcons.c,v 1.10 2004/02/03 00:03:18 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bdfP.h"

#undef MAX
#undef MIN
#define MAX(h,i) ((h) > (i) ? (h) : (i))
#define MIN(l,o) ((l) < (o) ? (l) : (o))

/*
 * Header for Sun VF fonts.
 */
typedef struct {
    unsigned short mag;
    unsigned short total_bytes;
    unsigned short max_width;
    unsigned short max_height;
    unsigned short pad;
} vfhdr_t;

/*
 * Character metrics data for Sun VF fonts.
 */
typedef struct {
    unsigned short offset;
    unsigned short bytes;
    char ascent;
    char descent;
    char lbearing;
    char rbearing;
    unsigned short dwidth;
} vfmetrics_t;

/**************************************************************************
 *
 * Support functions.
 *
 **************************************************************************/

static bdf_font_t *
#ifdef __STDC__
_bdf_load_vfont(FILE *in, vfhdr_t *hdr, bdf_callback_t callback, void *data,
                int *awidth)
#else
_bdf_load_vfont(in, hdr, callback, data, awidth)
FILE *in;
vfhdr_t *hdr;
bdf_callback_t callback;
void *data;
int *awidth;
#endif
{
    int first, ismono;
    long i, pos;
    bdf_font_t *fp;
    bdf_glyph_t *gp;
    bdf_callback_struct_t cb;
    vfmetrics_t met, metrics[256];

    /*
     * Convert the header values to little endian if necessary.
     */
    if (bdf_little_endian()) {
        hdr->total_bytes = ((hdr->total_bytes & 0xff) << 8) |
            ((hdr->total_bytes >> 8) & 0xff);
        hdr->max_width = ((hdr->max_width & 0xff) << 8) |
            ((hdr->max_width >> 8) & 0xff);
        hdr->max_height = ((hdr->max_height & 0xff) << 8) |
            ((hdr->max_height >> 8) & 0xff);
    }

    /*
     * The point size of the font will be the height, the resolution will
     * default to 72dpi, and the spacing will default to proportional.
     */
    fp = bdf_new_font(0, (long) hdr->max_height, 72, 72, BDF_PROPORTIONAL, 1);

    /*
     * Force the bits per pixel to 1.
     */
    fp->bpp = 1;

    /*
     * Load the glyph metrics and set a marker to the beginning of the glyph
     * bitmaps.
     */
    fread((char *) metrics, sizeof(vfmetrics_t), 256, in);
    pos = ftell(in);

    *awidth = 0;

    /*
     * Count the number of glyphs that actually exist and determine the font
     * bounding box in the process.
     */
    (void) memset((char *) &met, 0, sizeof(vfmetrics_t));
    met.lbearing = 127;
    fp->glyphs_size = 0;
    for (first = -1, ismono = 1, i = 0; i < 256; i++) {
        if (metrics[i].bytes == 0)
          continue;

        if (first == -1)
          first = i;

        /*
         * Start out by assuming the font is monowidth, but if any glyph
         * encountered has metrics different than the first glyph defined,
         * change that flag.  If the font is still flagged as monowidth when
         * this loop is done, then change the font to a monowidth font.
         */
        if (i != first && ismono &&
            (metrics[i].ascent != metrics[first].ascent ||
             metrics[i].descent != metrics[first].descent ||
             metrics[i].lbearing != metrics[first].lbearing ||
             metrics[i].rbearing != metrics[first].rbearing))
          ismono = 0;

        /*
         * If this is a little endian machine, convert the 16-bit values from
         * big endian.
         */
        if (bdf_little_endian()) {
            metrics[i].offset = ((metrics[i].offset & 0xff) << 8) |
                ((metrics[i].offset >> 8) & 0xff);
            metrics[i].bytes = ((metrics[i].bytes & 0xff) << 8) |
                ((metrics[i].bytes >> 8) & 0xff);
            metrics[i].dwidth = ((metrics[i].dwidth & 0xff) << 8) |
                ((metrics[i].dwidth >> 8) & 0xff);
        }

        /*
         * Update the value used for average width calculation.
         */
        *awidth = *awidth + (metrics[i].rbearing - metrics[i].lbearing);

        /*
         * Increment the count of characters.
         */
        fp->glyphs_size++;

        /*
         * Determine the font bounding box.
         */
        met.ascent = MAX(met.ascent, metrics[i].ascent);
        met.descent = MAX(met.descent, metrics[i].descent);
        met.lbearing = MIN(met.lbearing, metrics[i].lbearing);
        met.rbearing = MAX(met.rbearing, metrics[i].rbearing);
    }

    /*
     * Adjust the font bounding box accordingly.
     */
    fp->bbx.ascent = met.ascent;
    fp->bbx.descent = met.descent;
    fp->bbx.width = met.rbearing + met.lbearing;
    fp->bbx.height = met.ascent + met.descent;
    fp->bbx.x_offset = met.lbearing;
    fp->bbx.y_offset = -met.descent;

    /*
     * If the font is still flagged as a monowidth font, change the font
     * spacing.  The actual SPACING property will be adjusted once this
     * routine returns.
     */
    if (ismono)
      fp->spacing = BDF_MONOWIDTH;

    /*
     * Set up to load the glyphs.
     */
    fp->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * fp->glyphs_size);
    (void) memset((char *) fp->glyphs, 0,
                  sizeof(bdf_glyph_t) * fp->glyphs_size);

    /*
     * Set the callback up.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.current = 0;
        cb.total = fp->glyphs_size;
        (*callback)(&cb, data);
    }

    /*
     * Get the glyphs.
     */
    for (i = 0; i < 256; i++) {
        if (metrics[i].bytes == 0)
          continue;

        /*
         * Put the file pointer back at the beginning of the bitmaps.
         */
        fseek(in, pos, 0L);

        gp = fp->glyphs + fp->glyphs_used++;

        gp->encoding = i;
        gp->dwidth = metrics[i].dwidth;
        gp->swidth = (unsigned short)
            (((double) gp->dwidth) * 72000.0) /
            ((double) fp->point_size * fp->resolution_x);

        gp->bbx.ascent = metrics[i].ascent;
        gp->bbx.descent = metrics[i].descent;
        gp->bbx.width = metrics[i].rbearing + metrics[i].lbearing;
        gp->bbx.height = metrics[i].ascent + metrics[i].descent;
        gp->bbx.x_offset = metrics[i].lbearing;
        gp->bbx.y_offset = -metrics[i].descent;
        gp->bytes = metrics[i].bytes;
        gp->bitmap = (unsigned char *) malloc(gp->bytes);

        fseek(in, (long) metrics[i].offset, 1L);
        fread((char *) gp->bitmap, gp->bytes, 1, in);

        /*
         * Call the callback if indicated.
         */
        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.total = fp->glyphs_size;
            cb.current = fp->glyphs_used;
            (*callback)(&cb, data);
        }
    }

    /*
     * Add a message indicating the font was converted.
     */
    _bdf_add_comment(fp, "Font converted from VF to BDF.", 30);
    _bdf_add_acmsg(fp, "Font converted from VF to BDF.", 30);

    /*
     * Return the font.
     */
    return fp;
}

/*
 * Load a simple binary font.
 */
static bdf_font_t *
#ifdef __STDC__
_bdf_load_simple(FILE *in, int height, bdf_callback_t callback, void *data,
                 int type, int *awidth)
#else
_bdf_load_simple(in, height, callback, data, type, awidth)
FILE *in;
int height;
bdf_callback_t callback;
void *data;
int type, *awidth;
#endif
{
    long i;
    unsigned short dwidth, swidth;
    bdf_font_t *fp;
    bdf_glyph_t *gp;
    bdf_callback_struct_t cb;

    /*
     * The point size of the font will be the height, the resolution will
     * default to 72dpi, and the spacing will default to character cell.
     */
    fp = bdf_new_font(0, (long) height, 72, 72, BDF_CHARCELL, 1);

    /*
     * Force the bits per pixel to be one.
     */
    fp->bpp = 1;

    /*
     * Make sure the width is always set to 8 no matter what.  This may
     * change in the future, but not anytime soon.
     */
    *awidth = fp->bbx.width = 8;

    /*
     * Adjust the ascent and descent by hand for the 14pt and 8pt fonts.
     */
    if (height != 16) {
        fp->bbx.ascent++;
        fp->bbx.descent--;
    }

    /*
     * Default the font ascent and descent to that of the bounding box.
     */
    fp->font_ascent = fp->bbx.ascent;
    fp->font_descent = fp->bbx.descent;

    /*
     * Simple fonts will have at most 256 glyphs.
     */
    fp->glyphs_size = 256;
    fp->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * fp->glyphs_size);
    (void) memset((char *) fp->glyphs, 0,
                  sizeof(bdf_glyph_t) * fp->glyphs_size);

    /*
     * Determine the default scalable and device width for each character.
     */
    dwidth = fp->bbx.width;
    swidth = (unsigned short)
        (((double) dwidth) * 72000.0) /
        ((double) fp->point_size * fp->resolution_x);

    /*
     * Set up to call the callback.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.current = 0;
        cb.total = fp->glyphs_size;
        (*callback)(&cb, data);
    }

    /*
     * Now load the glyphs, assigning a default encoding.
     */
    for (i = 0, gp = fp->glyphs; i < fp->glyphs_size; i++, gp++) {
        gp->encoding = i;
        gp->dwidth = dwidth;
        gp->swidth = swidth;
        (void) memcpy((char *) &gp->bbx, (char *) &fp->bbx, sizeof(bdf_bbx_t));

        gp->bytes = height;
        gp->bitmap = (unsigned char *) malloc(height);
        fread((char *) gp->bitmap, height, 1, in);
        fp->glyphs_used++;

        /*
         * Call the callback if indicated.
         */
        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.total = fp->glyphs_size;
            cb.current = fp->glyphs_used;
            (*callback)(&cb, data);
        }
    }

    /*
     * Add a message indicating the font was converted.
     */
    if (type == 1) {
        _bdf_add_comment(fp, "Font converted from VGA/EGA to BDF.", 35);
        _bdf_add_acmsg(fp, "Font converted from VGA/EGA to BDF.", 35);
    } else if (type == 2) {
        _bdf_add_comment(fp, "Fonts converted from CP to BDF.", 31);
        _bdf_add_acmsg(fp, "Fonts converted from CP to BDF.", 31);
    }

    /*
     * Return the new font.
     */
    return fp;
}

/*
 * A structure to pass around in update callbacks.
 */
typedef struct {
    unsigned long total;
    unsigned long curr;
    unsigned long lcurr;
    bdf_callback_t cback;
    void *data;
} _bdf_update_rec_t;

/*
 * A routine to report the progress of loading a codepage font over all
 * three fonts.
 */
static void
#ifdef __STDC__
_bdf_codepage_progress(bdf_callback_struct_t *cb, void *data)
#else
_bdf_codepage_progress(cb, data)
bdf_callback_struct_t *cb;
void *data;
#endif
{
    _bdf_update_rec_t *up;
    bdf_callback_struct_t ncb;

    up = (_bdf_update_rec_t *) data;

    if (up->cback == 0)
      return;

    if (up->curr != 0 && cb->current == 0) {
        up->lcurr = 0;
        return;
    }

    up->curr += cb->current - up->lcurr;
    up->lcurr = cb->current;

    ncb.reason = cb->reason;
    ncb.total = up->total;
    ncb.current = up->curr;

    if (up->cback != 0)
      (*up->cback)(&ncb, up->data);
}

/*
 * Load a codepage font which actually contains three fonts.  This makes
 * use of the routine that loads the simple fonts.
 */
static int
#ifdef __STDC__
_bdf_load_codepage(FILE *in, bdf_callback_t callback, void *data,
                   bdf_font_t *fonts[3], int awidth[3])
#else
_bdf_load_codepage(in, callback, data, fonts, awidth)
FILE *in;
bdf_callback_t callback;
void *data;
bdf_font_t *fonts[3];
int awidth[3];
#endif
{
    _bdf_update_rec_t up;

    /*
     * Initialize an override callback structure.
     */
    up.cback = callback;
    up.data = data;
    up.total = 768;
    up.curr = up.lcurr = 0;

    /*
     * Load the 16pt font.
     */
    if (fseek(in, 40, 0L))
      return BDF_NOT_CONSOLE_FONT;

    fonts[0] = _bdf_load_simple(in, 16, _bdf_codepage_progress, (void *) &up,
                                0, &awidth[0]);

    /*
     * Load the 14pt font.
     */
    if (fseek(in, 4142, 0L)) {
        if (fonts[0] != 0)
          bdf_free_font(fonts[0]);
        fonts[0] = 0;
        return BDF_NOT_CONSOLE_FONT;
    }
    fonts[1] = _bdf_load_simple(in, 14, _bdf_codepage_progress, (void *) &up,
                                0, &awidth[1]);

    /*
     * Load the 8pt font.
     */
    if (fseek(in, 7732, 0L)) {
        if (fonts[0] != 0)
          bdf_free_font(fonts[0]);
        if (fonts[1] != 0)
          bdf_free_font(fonts[1]);
        fonts[0] = fonts[1] = 0;
        return BDF_NOT_CONSOLE_FONT;
    }
    fonts[2] = _bdf_load_simple(in, 8, _bdf_codepage_progress, (void *) &up,
                                2, &awidth[2]);

    /*
     * All the fonts loaded OK.
     */
    return BDF_OK;
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

static unsigned char vfmagic[] = {0x01, 0x1e};

int
#ifdef __STDC__
bdf_load_console_font(FILE *in, bdf_options_t *opts, bdf_callback_t callback,
                      void *data, bdf_font_t *fonts[3], int *nfonts)
#else
bdf_load_console_font(in, opts, callback, data, fonts, nfonts)
FILE *in;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
bdf_font_t *fonts[3];
int *nfonts;
#endif
{
    unsigned char hdr[4];
    int res, awidth[3];
    double dp, dr;
    bdf_property_t prop;
    vfhdr_t vhdr;
    struct stat st;

    (void) fstat(fileno(in), &st);

    *nfonts = 1;
    awidth[0] = awidth[1] = awidth[2] = 0;
    (void) memset((char *) fonts, 0, sizeof(bdf_font_t *) * 3);

    fread((char *) hdr, sizeof(unsigned char), 4, in);

    if (memcmp((char *) hdr, _bdf_psfcombined, 4) == 0)
      return BDF_PSF_UNSUPPORTED;

    if (memcmp((char *) hdr, (char *) _bdf_psf1magic, 2) == 0 ||
        memcmp((char *) hdr, (char *) _bdf_psf2magic, 4) == 0)
      /*
       * Have a PSF font that may contain a mapping table.
       */
      fonts[0] = bdf_load_psf(in, hdr, opts, callback, data, awidth);
    else {
        /*
         * Reset to the beginning of the file.
         */
        fseek(in, 0, 0L);
        if (memcmp((char *) hdr, (char *) vfmagic, 2) == 0) {
            /*
             * Have a Sun vfont.  Need to reload the header.
             */
            (void) fread((char *) &vhdr, sizeof(vfhdr_t), 1, in);
            fonts[0] = _bdf_load_vfont(in, &vhdr, callback, data, awidth);
        } else if (st.st_size == 9780) {
            /*
             * Have a CP font with three sizes.  Create all three fonts and
             * return them.
             */
            *nfonts = 3;
            if ((res = _bdf_load_codepage(in, callback, data, fonts, awidth)))
              return res;
        } else {
            /*
             * Have a plain font with 256 characters.  If the file size is not
             * evenly divisible by 256, then the file is probably corrupt or
             * is not a font.
             */
            if (st.st_size & 0xff)
              return BDF_NOT_CONSOLE_FONT;

            fonts[0] = _bdf_load_simple(in, st.st_size >> 8, callback, data,
                                        1, awidth);
        }
    }

    /*
     * Add all the default properties.
     */
    for (res = 0; res < *nfonts; res++) {
        prop.name = "POINT_SIZE";
        prop.format = BDF_INTEGER;
        prop.value.int32 = fonts[res]->point_size * 10;
        bdf_add_font_property(fonts[res], &prop);

        dr = (double) fonts[res]->resolution_y;
        dp = (double) (fonts[res]->point_size * 10);
        prop.name = "PIXEL_SIZE";
        prop.format = BDF_INTEGER;
        prop.value.int32 = (long) (((dp * dr) / 722.7) + 0.5);
        bdf_add_font_property(fonts[res], &prop);

        prop.name = "RESOLUTION_X";
        prop.format = BDF_CARDINAL;
        prop.value.card32 = (unsigned long) fonts[res]->resolution_x;
        bdf_add_font_property(fonts[res], &prop);

        prop.name = "RESOLUTION_Y";
        prop.format = BDF_CARDINAL;
        prop.value.card32 = (unsigned long) fonts[res]->resolution_y;
        bdf_add_font_property(fonts[res], &prop);

        prop.name = "FONT_ASCENT";
        prop.format = BDF_INTEGER;
        prop.value.int32 = (long) fonts[res]->bbx.ascent;
        bdf_add_font_property(fonts[res], &prop);

        prop.name = "FONT_DESCENT";
        prop.format = BDF_INTEGER;
        prop.value.int32 = (long) fonts[res]->bbx.descent;
        bdf_add_font_property(fonts[res], &prop);

        prop.name = "AVERAGE_WIDTH";
        prop.format = BDF_INTEGER;
        prop.value.int32 = (awidth[res] / fonts[res]->glyphs_used) * 10;
        bdf_add_font_property(fonts[res], &prop);

        prop.name = "SPACING";
        prop.format = BDF_ATOM;
        prop.value.atom = "P";
        switch (fonts[res]->spacing) {
          case BDF_PROPORTIONAL: prop.value.atom = "P"; break;
          case BDF_MONOWIDTH: prop.value.atom = "M"; break;
          case BDF_CHARCELL: prop.value.atom = "C"; break;
        }
        bdf_add_font_property(fonts[res], &prop);
    }

    return BDF_OK;
}

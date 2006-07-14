/*
 * Copyright 2006 Computing Research Labs, New Mexico State University
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

/*
 * Routines to import GF and PK format bitmap font files.
 *
 * GF is the "generic font" file format for bitmap fonts; GF files are
 * typically produced by Metafont.
 *
 * PK is the "packed file" font file format which is the de facto standard
 * bitmap font format in the TeX world. It contains most of the information
 * that's in a GF file, but much more compactly. PK fonts are typically
 * generated from GF fonts by gftopk(1), or by a converter like gsftopk(1).
 *
 * Documentation for these file formats can be found in the literate programs
 * GFtoPK, PKtoGF, GFtoDVI and GFtype which are included in a typical TeX
 * distribution's source.
 *
 */
#ifndef lint
#ifdef __GNUC__
static char svnid[] __attribute__ ((unused)) = "$Id: bdfpkgf.c 14 2006-01-09 15:29:07Z mleisher $";
#else
static char svnid[] = "$Id: bdfpkgf.c 14 2006-01-09 15:29:07Z mleisher $";
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef BDF_NO_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif /* !BDF_NO_X11 */

#include "bdfP.h"

#undef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))

#undef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))

/*
 * Symbolic names for the opcode bytes in GF files.
 */
#define GF_paint1	 64 /* 0x40 */
#define GF_paint2	 65 /* 0x41 */
#define GF_paint3	 66 /* 0x42 */
#define GF_boc		 67 /* 0x43 */
#define GF_boc1		 68 /* 0x44 */
#define GF_eoc		 69 /* 0x45 */
#define GF_skip0	 70 /* 0x46 */
#define GF_skip1	 71 /* 0x47 */
#define GF_skip2	 72 /* 0x48 */
#define GF_skip3	 73 /* 0x49 */
#define GF_newrow_0	 74 /* 0x4A */
#define GF_newrow_164	238 /* 0xEE */
#define GF_xxx1		239 /* 0xEF */
#define GF_xxx2		240 /* 0xF0 */
#define GF_xxx3		241 /* 0xF1 */
#define GF_xxx4		242 /* 0xF2 */
#define GF_yyy		243 /* 0xF3 */
#define GF_no_op	244 /* 0xF4 */
#define GF_char_loc	245 /* 0xF5 */
#define GF_char_loc0	246 /* 0xF6 */
#define GF_pre		247 /* 0xF7 */
#define GF_post		248 /* 0xF8 */
#define GF_post_post	249 /* 0xF9 */

/*
 * Symbolic names for the opcode bytes in PK files.
 */

#define PK_id		 89 /* 0x59 */
#define PK_xxx1		240 /* 0xF0 */
#define PK_xxx4		243 /* 0xF3 */
#define PK_yyy		244 /* 0xF4 */
#define PK_post		245 /* 0xF5 */
#define PK_reserved1	248 /* 0xF8 */
#define PK_pre		247 /* 0xF7 */

/*
 * Structure used to track the state for various things when reading a font.
 */
typedef struct {
    int top;
    int mask;
    int c;
} _bdf_mf_state_t;

/*
 * Routine to compare two glyphs by encoding so they can be sorted.
 */
static int
by_encoding(const void *a, const void *b)
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

/*
 * Routines for scanning numbers from GF and PK files.
 */
static long
_bdf_mf_get16(FILE *in)
{
    return (getc(in) << 8) | (getc(in) & 0xff);
}

static long
_bdf_mf_get32(FILE *in)
{
    long hi = _bdf_mf_get16(in);

    if (hi > 32767)
      hi -= 65536;
    return (hi << 16) + _bdf_mf_get16(in);
}

static void
printscaled(long s, unsigned char *buf)
{
    long delta;

    *buf++ = ' ';
    *buf++ = '(';
    if (s < 0) {
        *buf++ = '-';
        s = -s;
    }
    sprintf((char *) buf, "%ld", s >> 16);
    buf += strlen((char *) buf);
    s = 10 * ( s & 65535 ) + 5;
    if (s != 5) {
        delta = 10;
        *buf++ = '.';
        do {
            if (delta > 65536)
              s = s + 32768 - (delta >> 1);
            *buf++ = 0x30 + (s >> 16);
            s = 10 * (s & 65535);
            delta *= 10;
        } while (s > delta);
    }
    sprintf((char *) buf, " scaled)");
} 

/*
 * Routine to scan the PK specials and add them as comments if necessary.
 */
static int
_bdf_pk_specials(FILE *in, bdf_font_t *font, bdf_options_t *opts,
                 unsigned char *glyphname)
{
    int c;
    long i, n, num;
    unsigned long comment_size;
    unsigned char *comment, bytes[4];

    /*
     * Initialize the variable that keeps track of the storage allocated
     * for the comments encountered.
     */
    comment = 0;
    comment_size = 0;

    while ((c = getc(in)) >= PK_xxx1 && c != GF_char_loc) {
        /*
         * Anything between PK_reserved1 and 0xff are bad values.  PK_pre is
         * the font preamble which is not expected here.
         */
        if (c == PK_pre || (c >= PK_post && c <= 0xff))
          return -2;

        /*
	 * Anything between PK_xxx1 and PK_xxx4 are string specials which will
	 * be added with the comments if comments are being kept.
         */
        if (c >= PK_xxx1 && c <= PK_xxx4) {
            /*
             * Determine the number of bytes that need to be read to determine
             * the length of the string special.
             */
            n = (c - PK_xxx1) + 1;
            fread((char *) bytes, n, 1, in);
            for (i = 0, num = 0; i < n; i++)
              num = (num << 8) | bytes[i];

            if (opts->keep_comments) {
                /*
                 * Make sure there is enough space for the string.
                 */
                if (comment_size < num + 1) {
                    if (comment_size == 0)
                      comment = (unsigned char *) malloc(num + 1);
                    else
                      comment = (unsigned char *)
                          realloc((char *) comment, num + 1);
                    comment_size = num + 1;
                }
                /*
                 * Read the comment and add it to the font.
                 */
                fread((char *) comment, num, 1, in);
                comment[num] = 0;
                if (!strncmp((char *) comment, "title ", 6))
                  /*
                   * The comment is the glyph's name/title; save it so it can
                   * be associated with the forthcoming glyph, rather than
                   * with the font as a whole.
                   */
                  strcpy((char *) glyphname, (char *) comment + 6);
                else
                  /*
                   * A regular comment.
                   */
                  _bdf_add_comment(font, (char *) comment, num);
            } else
              /*
               * Skip the string special.
               */
              fseek(in, num, 1L);
        }

        /*
         * PK_yyy is a numeric special.  Add the number as a comment if
         * specified.
         */
        if (c == PK_yyy) {
            num = _bdf_mf_get32(in);
            if (opts->keep_comments) {
                if (comment_size < 64) {
                    if (comment_size == 0)
                      comment = (unsigned char *) malloc(64);
                    else
                      comment = (unsigned char *)
                          realloc((char *) comment, 64);
                    comment_size = 64;
                }
                sprintf((char *) comment, "%ld", num);
                printscaled(num, comment + strlen((char *) comment));
                _bdf_add_comment(font, (char *) comment,
                                 strlen((char *) comment));
            }
        }
    }

    /*
     * Free up the comment buffer if it was allocated.
     */
    if (comment_size > 0)
      free((char *) comment);

    /*
     * Return the byte that caused the loop to terminate.  This will be the
     * postamble marker GF_post or the start of the glyphs.
     */
    return c;
}

/*
 * Awkward little routine to collect packed bits from a PK file.
 */
static int
_bdf_pk_getbit(FILE *in, _bdf_mf_state_t *state)
{
    state->mask >>= 1;
    if (state->mask == 0) {
        state->c = getc(in);
        state->mask = 0x80;
    }
    return (state->c & state->mask);
}

/*
 * Another awkward little routine to get 4 bits at a time from a PK file.
 */
static int
_bdf_pk_getnybble(FILE *in, _bdf_mf_state_t *state)
{
    int r;

    if (state->top == 0) {
        state->c = getc(in);
        state->top = 2;
    }
    r = (state->c >> ((state->top - 1) << 2)) & 0x0f;
    state->top--;
    return r;
}

/*
 * Yet another awkward routine to read a packed number and a repeat count from
 * a PK file.
 */
static int
_bdf_pk_getpacked(FILE *in, int *rcount, int dyn, _bdf_mf_state_t *state)
{
    int i, j;

    if ((i = _bdf_pk_getnybble(in, state)) == 0) {
        do {
            j = _bdf_pk_getnybble(in, state);
            i++;
        } while (!j);
        for (;i > 0; i--)
          j = (j << 4) + _bdf_pk_getnybble(in, state);
        return j - 15 + ((13 - dyn) << 4) + dyn;
    }

    if (i <= dyn)
      return i;

    if (i < 14)
      return ((i - dyn - 1) << 4) + dyn + 1 + _bdf_pk_getnybble(in, state);

    *rcount = (i == 14) ? _bdf_pk_getpacked(in, rcount, dyn, state) : 1;
    return _bdf_pk_getpacked(in, rcount, dyn, state);
}

static int
_bdf_load_pk_font(FILE *in, bdf_options_t *opts, bdf_callback_t callback,
                  void *data, bdf_font_t **font)
{
    int n, res, set, x, y, bpr, rcnt, ismono;
    long num, plen, pend, row_size, awidth;
    short rb, maxrb, minlb, maxlb;
    double denom, dw;
    bdf_font_t *f;
    bdf_glyph_t *gp, g;
    bdf_property_t prop;
    bdf_callback_struct_t cb;
    _bdf_mf_state_t state;
    struct stat st;
    unsigned char *row, bytes[256], glyphname[256];

    row = 0;
    glyphname[0] = 0;

    /*
     * Create a font to work with.
     */
    *font = f = (bdf_font_t *) malloc(sizeof(bdf_font_t));
    (void) memset((char *) f, 0, sizeof(bdf_font_t));

    /*
     * Set some defaults and allocate an initial amount of space.  Make the
     * initial assumption that the font is monowidth but determine if it
     * should be proportional when the glyphs are loaded.  Allocate space for
     * at least 128 glyphs before loading the font.
     */
    f->bpp = 1;
    f->spacing = BDF_MONOWIDTH;
    f->glyphs_size = 128;
    f->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) << 7);
    (void) memset((char *) f->glyphs, 0, sizeof(bdf_glyph_t) << 7);

    /*
     * Initialize the horizontal and vertical offsets of the font to some
     * large number.
     */
    f->bbx.x_offset = f->bbx.y_offset = 32767;

    /*
     * Initialize things.
     */
    ismono = 1;
    row_size = 0;
    awidth = 0;
    rb = maxrb = maxlb = 0;
    minlb = 32767;
    (void) memset((char *) &g, 0, sizeof(bdf_glyph_t));

    /*
     * Load the initial comment.
     */
    fread((char *) bytes, 1, 1, in);
    n = bytes[0];
    fread(bytes, n, 1, in);
    bytes[n] = 0;

    /*
     * Add the comment to the font if indicated.
     */
    if (opts->keep_comments)
      _bdf_add_comment(f, (char *) bytes, (unsigned long) n);

    /*
     * Get the point size and scale it down 
     */
    f->point_size = (long) (((float) _bdf_mf_get32(in)) /
                            ((float) (1 << 20)));

    /*
     * Skip the checksum.
     */
    fread(bytes, 4, 1, in);

    /*
     * Get the horizontal resolution.
     */
    f->resolution_x = (long)
        (((((float) _bdf_mf_get32(in)) * 72.27) / ((float) (1 << 16))) + 0.5);

    /*
     * Get the vertical resolution.
     */
    f->resolution_y = (long)
        (((((float) _bdf_mf_get32(in)) * 72.27) / ((float) (1 << 16))) + 0.5);

    /*
     * Determine the denominator used for scalable width calculations.
     */
    denom = ((double) f->point_size) * ((double) f->resolution_x);

    /*
     * Get the font info so we can set up the callback.  The callback will
     * update after every glyph is loaded and is based on file size instead of
     * number of glyphs.  This allows the font to be read all at once instead
     * of twice.
     */
    (void) fstat(fileno(in), &st);

    /*
     * Set the callback up.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.current = 0;
        cb.total = st.st_size;
        (*callback)(&cb, data);
    }

    /*
     * Load the glyphs.
     */
    gp = f->glyphs;
    while ((res = _bdf_pk_specials(in, f, opts, glyphname)) != PK_post &&
           res > 0) {
	/* Set the glyph's name, if we've seen it */
        if (strlen((char *) glyphname)) {
            g.name = malloc(strlen((char *) glyphname)+1);
	    strcpy((char *) g.name, (char *) glyphname);
	}
        if ((res & 7) == 7) {
            /*
             * Long glyph info.
             */

            /*
             * Get the packet length and glyph encoding.
             */
            plen = _bdf_mf_get32(in);
            g.encoding = _bdf_mf_get32(in);

            pend = plen + ftell(in);

            /*
             * Get the glyph metrics.
             */

            /*
             * Ignore the TFM width.
             */
            (void) _bdf_mf_get32(in);

            /*
             * Get the device width (DWIDTH) from the horizontal escapement of
             * the glyph and calculate the scalable width (SWIDTH) from it.
             */
            num = _bdf_mf_get32(in);
            g.dwidth = num >> 16;
            dw = (double) g.dwidth;
            g.swidth = (unsigned short) ((dw * 72000.0) / denom);

            /*
             * Ignore the vertical escapement.
             */
            (void) _bdf_mf_get32(in);

            /*
             * Collect the remaining glyph metrics info and calculate the
             * glyph ascent and descent from the height and vertical offset.
             */
            g.bbx.width = (unsigned short) _bdf_mf_get32(in);
            g.bbx.height = (unsigned short) _bdf_mf_get32(in);
            g.bbx.x_offset = (short) _bdf_mf_get32(in);
            g.bbx.y_offset = (short) _bdf_mf_get32(in);
        } else if (res & 0x04) {
            /*
             * Extended glyph info.
             */
            if (res & 0x08)
              plen = (((res & 0x07) - 4) << 16) + _bdf_mf_get16(in);
            else
              plen = (((res & 0x0f) - 4) << 16) + _bdf_mf_get16(in);

            /*
             * Load the encoding byte and the first byte of the TFM width.
             */
            fread(bytes, 2, 1, in);
            g.encoding = (long) bytes[0];

            pend = plen + ftell(in) - 1;

            /*
             * Get the glyph metrics.
             */

            /*
             * Ignore the last two bytes of the TFM width.
             */
            (void) _bdf_mf_get16(in);

            /*
             * Get the device width (DWIDTH) from the horizontal escapement of
             * the glyph and calculate the scalable width (SWIDTH) from it.
             */
            g.dwidth = (unsigned short) _bdf_mf_get16(in);
            dw = (double) g.dwidth;
            g.swidth = (unsigned short) ((dw * 72000.0) / denom);

            /*
             * Collect the remaining glyph metrics info and calculate the
             * glyph ascent and descent from the height and vertical offset.
             */
            g.bbx.width = (unsigned short) _bdf_mf_get16(in);
            g.bbx.height = (unsigned short) _bdf_mf_get16(in);
            if ((num = _bdf_mf_get16(in)) > 32767)
              g.bbx.x_offset = num - 65536;
            else
              g.bbx.x_offset = (short) num;
            if ((num = _bdf_mf_get16(in)) > 32767)
              g.bbx.y_offset = num - 65536;
            else
              g.bbx.y_offset = (short) num;
        } else {
            /*
             * Short glyph info.  Read the next 10 bytes so they can be used
             * as part of the glyph info calculations.
             */
            fread(bytes, 10, 1, in);

            num = 0;
            if (res & 0x08)
              plen = ((res & 0x07) << 8) + bytes[num++];
            else
              plen = ((res & 0x0f) << 8) + bytes[num++];

            g.encoding = (long) bytes[num++];

            pend = plen + ftell(in) - 8;

            /*
             * Skip the TFM width.
             */
            num += 3;

            /*
             * Get the device width (DWIDTH) from the horizontal escapement of
             * the glyph and calculate the scalable width (SWIDTH) from it.
             */
            g.dwidth = (unsigned short) bytes[num++];
            dw = (double) g.dwidth;
            g.swidth = (unsigned short) ((dw * 72000.0) / denom);

            /*
             * Collect the remaining glyph metrics info and calculate the
             * glyph ascent and descent from the height and vertical offset.
             */
            g.bbx.width = (unsigned short) bytes[num++];
            g.bbx.height = (unsigned short) bytes[num++];
            /*
             * The hoff value we're now to interpret gives the horizontal
             * offset of the reference point compared to the character's
             * pixels.  i.e. a value of -2 means the pixels start at +2; see
             * the discussion of the example character raster in the pktogf
             * web.
             */
            g.bbx.x_offset = (short) ((bytes[num] & 0x80) ?
                                      256 - bytes[num] : bytes[num]);
            num++;
            g.bbx.y_offset = (short) ((bytes[num] & 0x80) ?
                                      bytes[num] - 256 : bytes[num]);
            num++;
        }

        /*
         * Adjust the vertical metrics of the glyph.
         */
        g.bbx.y_offset = (g.bbx.y_offset + 1) - g.bbx.height;
        g.bbx.ascent = g.bbx.height + g.bbx.y_offset;
        g.bbx.descent = -g.bbx.y_offset;

        /*
         * Check to see if the font needs to be marked as proportional.
         */
        if (f->glyphs_used > 0 && ismono &&
            (f->glyphs[0].bbx.width != g.bbx.width ||
             f->glyphs[0].bbx.height != g.bbx.height ||
             f->glyphs[0].bbx.x_offset != g.bbx.x_offset ||
             f->glyphs[0].bbx.y_offset != g.bbx.y_offset ||
             f->glyphs[0].bbx.ascent != g.bbx.ascent ||
             f->glyphs[0].bbx.descent != g.bbx.descent)) {
            ismono = 0;
            f->spacing = BDF_PROPORTIONAL;
        }

        /*
         * Now load the packed bits or the run length encoded image.
         */
        bpr = (g.bbx.width + 7) >> 3;
        g.bytes = bpr * g.bbx.height;
        g.bitmap = (unsigned char *) malloc(g.bytes);
        (void) memset((char *) g.bitmap, 0, g.bytes);

        /*
         * Reset the state values.
         */
        state.top = state.mask = state.c = 0;
        if ((res & 0xf0) == 0xe0) {
            /*
             * Packed bit format.
             */
            if ((g.bbx.width & 7) == 0)
              /*
               * The bits are on a boundary that is a multiple of 8, so the
               * bitmap can be read all at once.
               */
              fread(g.bitmap, g.bbx.height * bpr, 1, in);
            else {
                /*
                 * The width is not a multiple of 8, so the bitmap should be
                 * read one bit at a time.
                 */
                for (y = 0; y < g.bbx.height; y++) {
                    for (x = 0; x < g.bbx.width; x++) {
                        if (_bdf_pk_getbit(in, &state))
                          g.bitmap[(y * bpr) + (x >> 3)] |= (0x80 >> (x & 7));
                    }
                }
            }
        } else {
            /*
             * Get the run length encoded image.
             */

            /*
             * The glyph image is going to be run length encoded, so allocate
             * a row to collect bits into.
             */
            if (row_size < bpr) {
                if (row_size == 0)
                  row = (unsigned char *) malloc(bpr);
                else
                  row = (unsigned char *) realloc((char *) row, bpr);
                row_size = bpr;
            }
            /*
             * Initialize the row buffer so we don't get any extra bits in the
             * image by accident.
             */
            (void) memset((char *) row, 0, row_size);

            /*
             * Determine if the run length encoding starts with bits that are
             * to be set or cleared.
             */
            set = res & 0x08;
            for (rcnt = x = y = 0; y < g.bbx.height;) {
                /*
                 * Get the next number and a repeat count.
                 */
                n = _bdf_pk_getpacked(in, &rcnt, (res >> 4) & 0x0f, &state);
                while (n > 0) {
                    for (; n > 0 && x < g.bbx.width; x++, n--) {
                        if (set)
                          row[x >> 3] |= (0x80 >> (x & 7));
                    }
                    if (x == g.bbx.width) {
                        /*
                         * Copy the row into the actual glyph bitmap as many
                         * times as called for by the repeat count, reset x to
                         * 0 so a new row can be started, and initialize the
                         * row buffer again.
                         */
                        for (x = 0; rcnt >= 0; rcnt--, y++)
                          (void) memcpy((char *) (g.bitmap + (y * bpr)),
                                        (char *) row, bpr);
                        (void) memset((char *) row, 0, bpr);
                        rcnt = 0;
                    }
                }
                /*
                 * Invert the flag that indicates whether bits need
                 * to be set or not.
                 */
                set = !set;
            }
        }

        /*
         * Adjust the font bounding box.
         */
        f->bbx.ascent = MAX(g.bbx.ascent, f->bbx.ascent);
        f->bbx.descent = MAX(g.bbx.descent, f->bbx.descent);

        rb = g.bbx.width + g.bbx.x_offset;
        maxrb = MAX(rb, maxrb);
        minlb = MIN(g.bbx.x_offset, minlb);
        maxlb = MAX(g.bbx.x_offset, maxlb);

        /*
         * Increase the average width count to be used later.
         */
        awidth += g.bbx.width;

        if ((g.encoding < 0 || g.encoding > 65535) && opts->keep_unencoded) {
            /*
             * If the glyph is unencoded (encoding field < 0 or > 65535) and
             * the unencoded glyphs should be kept, then add the glyph to the
             * unencoded list of the font.
             */
            if (f->unencoded_used == f->unencoded_size) {
                if (f->unencoded_size == 0)
                  f->unencoded = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t));
                else
                  f->unencoded = (bdf_glyph_t *)
                      realloc((char *) f->unencoded,
                              sizeof(bdf_glyph_t) * (f->unencoded_size + 1));
                f->unencoded_size++;
            }
            (void) memcpy((char *) (f->unencoded + f->unencoded_used),
                          (char *) &g, sizeof(bdf_glyph_t));
            f->unencoded_used++;
        } else if (g.encoding >= 0 && g.encoding <= 65535) {
            /*
             * Add the glyph to the encoded list.
             */
            if (f->glyphs_used == f->glyphs_size) {
                /*
                 * Expand by 128 glyphs at a time.
                 */
                if (f->glyphs_used == 0)
                  f->glyphs = (bdf_glyph_t *)
                      malloc(sizeof(bdf_glyph_t) << 7);
                else
                  f->glyphs = (bdf_glyph_t *)
                      realloc((char *) f->glyphs,
                              sizeof(bdf_glyph_t) * (f->glyphs_used + 128));
                gp = f->glyphs + f->glyphs_used;
                (void) memset((char *) gp, 0, sizeof(bdf_glyph_t) << 7);
                f->glyphs_size += 128;
            }
            gp = f->glyphs + f->glyphs_used++;
            (void) memcpy((char *) gp, (char *) &g, sizeof(bdf_glyph_t));;
        } else {
            /*
             * Free up the memory allocated for the temporary glyph so it
             * doesn't leak.
             */
            if (g.bytes > 0)
              free((char *) g.bitmap);
        }

        /*
         * Make sure the temporary glyph is reinitialized.
         */
        (void) memset((char *) &g, 0, sizeof(bdf_glyph_t));

        /*
         * Call the callback if indicated.
         */
        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.total = st.st_size;
            cb.current = ftell(in);
            (*callback)(&cb, data);
        }
    }

    /*
     * Sort all the glyphs by encoding.
     */
    qsort((char *) f->glyphs, f->glyphs_used, sizeof(bdf_glyph_t),
          by_encoding);

    /*
     * Adjust the font bounding box from the values collected.
     */
    f->bbx.width = maxrb - minlb;
    f->bbx.height = f->bbx.ascent + f->bbx.descent;
    f->bbx.x_offset = minlb;
    f->bbx.y_offset = -f->bbx.descent;

    /*
     * Set the default character as being undefined.
     */
    f->default_glyph = -1;

    /*
     * Set the font ascent and descent.
     */
    f->font_ascent = f->bbx.ascent;
    f->font_descent = f->bbx.descent;

    /*
     * Now add the properties to the font.
     */
    prop.name = "POINT_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = f->point_size * 10;
    bdf_add_font_property(f, &prop);

    /*
     * Calculate and add the pixel size.
     */
    denom = (double) f->resolution_y;
    dw = (double) (f->point_size * 10);
    prop.name = "PIXEL_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = (long) (((denom * dw) / 722.7) + 0.5);
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_X";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned long) f->resolution_x;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_Y";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned long) f->resolution_y;
    bdf_add_font_property(f, &prop);

    prop.name = "FONT_ASCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = f->font_ascent;
    bdf_add_font_property(f, &prop);

    prop.name = "FONT_DESCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = f->font_descent;
    bdf_add_font_property(f, &prop);

    prop.name = "AVERAGE_WIDTH";
    prop.format = BDF_INTEGER;
    prop.value.int32 = (awidth / (f->unencoded_used + f->glyphs_used)) * 10;
    bdf_add_font_property(f, &prop);

    /*
     * Default all PK fonts to proportional spacing.
     */
    prop.name = "SPACING";
    prop.format = BDF_ATOM;
    prop.value.atom = "P";
    switch (f->spacing) {
      case BDF_PROPORTIONAL: prop.value.atom = "P"; break;
      case BDF_MONOWIDTH: prop.value.atom = "M"; break;
      case BDF_CHARCELL: prop.value.atom = "C"; break;
    }
    bdf_add_font_property(f, &prop);

    /*
     * Free up the row buffer if it was allocated.
     */
    if (row_size > 0)
      free((char *) row);

    /*
     * Call the callback one last time if necessary.
     */
    if (callback != 0 && cb.current != cb.total) {
        cb.reason = BDF_LOADING;
        cb.total = cb.current = st.st_size;
        (*callback)(&cb, data);
    }

    /*
     * Add a message indicating the font was converted.
     */
    _bdf_add_comment(f, "Font converted from PK to BDF.", 30);
    _bdf_add_acmsg(f, "Font converted from PK to BDF.", 30);

    return BDF_OK;
}

static int
_bdf_gf_specials(FILE *in, bdf_font_t *font, bdf_options_t *opts,
                 unsigned char glyphname[])
{
    int c;
    long i, n, num;
    unsigned long comment_size;
    unsigned char *comment, bytes[4];

    /*
     * Initialize the variable that keeps track of the storage allocated
     * for the comments encountered.
     */
    comment = 0;
    comment_size = 0;

    while ((c = getc(in)) >= GF_xxx1) {
        /*
         * GF_xxx1 .. GF_xxx4 are string specials which will be added with the
         * comments if comments are being kept.
         */
        if (c >= GF_xxx1 && c <= GF_xxx4) {
            /*
             * Determine the number of bytes that need to be read to determine
             * the length of the string special.
             */
            n = (c - GF_xxx1) + 1;
            fread((char *) bytes, n, 1, in);
            for (i = 0, num = 0; i < n; i++)
              num = (num << 8) | bytes[i];

            if (opts->keep_comments) {
                /*
                 * Make sure there is enough space for the string.
                 */
                if (comment_size < num + 1) {
                    if (comment_size == 0)
                      comment = (unsigned char *) malloc(num + 1);
                    else
                      comment = (unsigned char *)
                          realloc((char *) comment, num + 1);
                    comment_size = num + 1;
                }
                /*
                 * Read the comment and add it to the font.
                 */
                fread((char *) comment, num, 1, in);
                comment[num] = 0;
                if (!strncmp((char *) comment, "title ", 6))
                  /*
                   * The comment is the glyph's name/title; save it so
                   * it can be associated with the forthcoming glyph,
                   * rather than with the font as a whole.
                   */
                  strcpy((char *) glyphname, (char *) comment + 6);
                else
                  /*
                   * A regular comment
                   */
                  _bdf_add_comment(font, (char *) comment, num);
            } else
              /*
               * Skip the string special.
               */
              fseek(in, num, 1L);
            c = GF_no_op;
        }

        /*
         * GF_yyy is a numeric special.  Add the number as a comment if
         * specified.
         */
        if (c == GF_yyy) {
            num = _bdf_mf_get32(in);
            if (opts->keep_comments) {
                if (comment_size < 64) {
                    if (comment_size == 0)
                      comment = (unsigned char *) malloc(64);
                    else
                      comment = (unsigned char *)
                          realloc((char *) comment, 64);
                    comment_size = 64;
                }
                sprintf((char *) comment, "%ld", num);
                printscaled(num, comment + strlen((char *) comment));
                _bdf_add_comment(font, (char *) comment,
                                 strlen((char *) comment));
            }
            c = GF_no_op;
        }
        if (c != GF_no_op)
          break;
    }

    /*
     * Free up the comment buffer if it was allocated.
     */
    if (comment_size > 0)
      free((char *) comment);

    /*
     * Return the byte that caused the loop to terminate.
     */
    return c;
}

static int
_bdf_load_gf_font(FILE *in, bdf_options_t *opts, bdf_callback_t callback,
                  void *data, bdf_font_t **font)
{
    int n, res, set, x, y, bpr, ismono;
    long awidth, num;
    short rb, maxrb, minlb, maxlb;
    double denom, dw;
    bdf_font_t *f;
    bdf_glyph_t *gp, g;
    bdf_property_t prop;
    bdf_callback_struct_t cb;
    struct stat st;
    unsigned char bytes[256], glyphname[256];

    glyphname[0] = 0;

    /*
     * Create a font to work with.
     */
    *font = f = (bdf_font_t *) malloc(sizeof(bdf_font_t));
    (void) memset((char *) f, 0, sizeof(bdf_font_t));

    /*
     * Set some defaults and allocate an initial amount of space.  Make the
     * initial assumption that the font is monowidth but determine if it
     * should be proportional when the glyphs are loaded.  Allocate space for
     * at least 128 glyphs before loading the font.
     */
    f->bpp = 1;
    f->spacing = BDF_MONOWIDTH;
    f->glyphs_size = 128;
    f->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) << 7);
    (void) memset((char *) f->glyphs, 0, sizeof(bdf_glyph_t) << 7);

    /*
     * Initialize the horizontal and vertical offsets of the font to some
     * large number.
     */
    f->bbx.x_offset = f->bbx.y_offset = 32767;

    /*
     * Initialize things.
     */
    ismono = 1;
    awidth = 0;
    rb = maxrb = maxlb = 0;
    minlb = 32767;
    (void) memset((char *) &g, 0, sizeof(bdf_glyph_t));

    /*
     * Load the initial comment.
     */
    fread((char *) bytes, 1, 1, in);
    n = bytes[0];
    fread(bytes, n, 1, in);
    bytes[n] = 0;

    /*
     * Add the comment to the font if indicated.
     */
    if (opts->keep_comments)
      _bdf_add_comment(f, (char *) bytes, (unsigned long) n);

    /*
     * Get the font info so we can set up the callback.  The callback will
     * update after every glyph is loaded and is based on file size instead of
     * number of glyphs.  This allows the font to be read all at once instead
     * of twice.
     */
    (void) fstat(fileno(in), &st);

    /*
     * Set the callback up.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.current = 0;
        cb.total = st.st_size;
        (*callback)(&cb, data);
    }

    while ((res = _bdf_gf_specials(in, f, opts, glyphname)) != GF_post) {
	/* Set the glyph's name, if we've seen it */
        if (((res == GF_boc) || (res == GF_boc1)) &&
            strlen((char *) glyphname)) {
            g.name = malloc(strlen((char *) glyphname)+1);
	    strcpy(g.name, (char *) glyphname);
	}

        if (res == GF_boc) {
	    /* 32-bit character code */
            if ((g.encoding = _bdf_mf_get32(in) & 0xff) < 0)
              g.encoding += 256;
	    /* Skip navigation pointer that's not relevant for us */
            (void) _bdf_mf_get32(in);
	    /* 4 times 32-bit loose bounding box parameters */
            g.bbx.x_offset = -((short) _bdf_mf_get32(in));
            g.bbx.width =
                (unsigned short) (_bdf_mf_get32(in) + g.bbx.x_offset);
            g.bbx.y_offset = (short) _bdf_mf_get32(in);
            g.bbx.height =
                (unsigned short) (_bdf_mf_get32(in) - g.bbx.y_offset);
            g.bbx.height++;
            g.bbx.ascent = g.bbx.height + g.bbx.y_offset;
            g.bbx.descent = -g.bbx.y_offset;
        } else if (res == GF_boc1) {
            g.encoding = getc(in);
            g.bbx.width = getc(in);
            g.bbx.x_offset = getc(in) - g.bbx.width;
            g.bbx.height = getc(in);
            g.bbx.y_offset = getc(in) - g.bbx.height;
            g.bbx.height++;
            g.bbx.width++;
            g.bbx.ascent = g.bbx.height + g.bbx.y_offset;
            g.bbx.descent = -g.bbx.y_offset;
        }

        /*
         * Check to see if the font needs to be marked as proportional.
         */
        if (f->glyphs_used > 0 && ismono &&
            (f->glyphs[0].bbx.width != g.bbx.width ||
             f->glyphs[0].bbx.height != g.bbx.height ||
             f->glyphs[0].bbx.x_offset != g.bbx.x_offset ||
             f->glyphs[0].bbx.y_offset != g.bbx.y_offset ||
             f->glyphs[0].bbx.ascent != g.bbx.ascent ||
             f->glyphs[0].bbx.descent != g.bbx.descent)) {
            ismono = 0;
            f->spacing = BDF_PROPORTIONAL;
        }

        bpr = (g.bbx.width + 7) >> 3;
        g.bytes = g.bbx.height * bpr;
        g.bitmap = (unsigned char *) malloc(g.bytes);
        (void) memset((char *) g.bitmap, 0, g.bytes);

        /*
         * Get the glyph.
         */
        set = x = y = 0;
        while ((res = getc(in)) < GF_xxx1) {
            if (res == GF_eoc)
              break;
            if (res < GF_paint3) {
                switch (res) {
                  case GF_paint1:
                    res = getc(in);
                    break;
                  case GF_paint2:
                    res = _bdf_mf_get16(in);
                    break;
                  case GF_paint3:
                    res = (_bdf_mf_get16(in) << 8) | (getc(in) & 0xff);
                    break;
                }
                for (; res > 0; x++, res--) {
                    if (set)
                      g.bitmap[(y * bpr) + (x >> 3)] |= (0x80 >> (x & 7));
                }
                set = !set;
            } else if (GF_skip0 <= res && res <= GF_skip3) {
                switch (res) {
                  case GF_skip1:
                    res = getc(in);
                    break;
                  case GF_skip2:
                    res = _bdf_mf_get16(in);
                    break;
                  case GF_skip3:
                    res = (_bdf_mf_get16(in) << 8) | (getc(in) & 0xff);
                    break;
                  default:
                    res = 0;
                }
                x = 0;
                y += res;
                set = 0;
            } else if (GF_newrow_0 <= res && res <= GF_newrow_164) {
                y++;
                x = res - GF_newrow_0;
                set = 1;
            }
        }

        /*
         * Adjust the font bounding box.
         */
        f->bbx.ascent = MAX(g.bbx.ascent, f->bbx.ascent);
        f->bbx.descent = MAX(g.bbx.descent, f->bbx.descent);

        rb = g.bbx.width + g.bbx.x_offset;
        maxrb = MAX(rb, maxrb);
        minlb = MIN(g.bbx.x_offset, minlb);
        maxlb = MAX(g.bbx.x_offset, maxlb);

        /*
         * Increase the average width count to be used later.
         */
        awidth += g.bbx.width;

        if ((g.encoding < 0 || g.encoding > 65535) && opts->keep_unencoded) {
            /*
             * If the glyph is unencoded (encoding field < 0 or > 65535) and
             * the unencoded glyphs should be kept, then add the glyph to the
             * unencoded list of the font.
             */
            if (f->unencoded_used == f->unencoded_size) {
                if (f->unencoded_size == 0)
                  f->unencoded = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t));
                else
                  f->unencoded = (bdf_glyph_t *)
                      realloc((char *) f->unencoded,
                              sizeof(bdf_glyph_t) * (f->unencoded_size + 1));
                f->unencoded_size++;
            }
            (void) memcpy((char *) (f->unencoded + f->unencoded_used),
                          (char *) &g, sizeof(bdf_glyph_t));
            f->unencoded_used++;
        } else if (g.encoding >= 0 && g.encoding <= 65535) {
            /*
             * Add the glyph to the encoded list.
             */
            if (f->glyphs_used == f->glyphs_size) {
                /*
                 * Expand by 128 glyphs at a time.
                 */
                if (f->glyphs_used == 0)
                  f->glyphs = (bdf_glyph_t *)
                      malloc(sizeof(bdf_glyph_t) << 7);
                else
                  f->glyphs = (bdf_glyph_t *)
                      realloc((char *) f->glyphs,
                              sizeof(bdf_glyph_t) * (f->glyphs_used + 128));
                gp = f->glyphs + f->glyphs_used;
                (void) memset((char *) gp, 0, sizeof(bdf_glyph_t) << 7);
                f->glyphs_size += 128;
            }
            gp = f->glyphs + f->glyphs_used++;
            (void) memcpy((char *) gp, (char *) &g, sizeof(bdf_glyph_t));;
        } else {
            /*
             * Free up the memory allocated for the temporary glyph so it
             * doesn't leak.
             */
            if (g.bytes > 0)
              free((char *) g.bitmap);
        }

        /*
         * Make sure the temporary glyph is reinitialized.
         */
        (void) memset((char *) &g, 0, sizeof(bdf_glyph_t));

	/*
	 * We're done with this glyph; forget we've seen its name
	 */
	glyphname[0] = 0;

        /*
         * Call the callback if indicated.
         */
        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.total = st.st_size;
            cb.current = ftell(in);
            (*callback)(&cb, data);
        }

        if (res == GF_post)
          break;
    }

    /*
     * Sort all the glyphs by encoding.
     */
    qsort((char *) f->glyphs, f->glyphs_used, sizeof(bdf_glyph_t),
          by_encoding);

    /*
     * Adjust the font bounding box from the values collected.
     */
    f->bbx.width = maxrb - minlb;
    f->bbx.height = f->bbx.ascent + f->bbx.descent;
    f->bbx.x_offset = minlb;
    f->bbx.y_offset = -f->bbx.descent;

    /*
     * Set the default character as being undefined.
     */
    f->default_glyph = -1;

    /*
     * Set the font ascent and descent.
     */
    f->font_ascent = f->bbx.ascent;
    f->font_descent = f->bbx.descent;

    /*
     * Collect the remaining font info from the postamble.
     */
    (void) _bdf_mf_get32(in);

    /*
     * Get the point size and scale it down 
     */
    f->point_size = (long) (((float) _bdf_mf_get32(in)) /
                            ((float) (1 << 20)));

    /*
     * Skip the checksum.
     */
    fseek(in, 4, 1L);

    /*
     * Get the horizontal resolution.
     */
    f->resolution_x = (long)
        (((((float) _bdf_mf_get32(in)) * 72.27) / ((float) (1 << 16))) + 0.5);

    /*
     * Get the vertical resolution.
     */
    f->resolution_y = (long)
        (((((float) _bdf_mf_get32(in)) * 72.27) / ((float) (1 << 16))) + 0.5);

    /*
     * Skip the overall font rows and columns because they have already
     * been determined.
     */
    fseek(in, 16, 1L);

    /*
     * Determine the denominator used for scalable width calculations.
     */
    denom = ((double) f->point_size) *
        ((double) f->resolution_x);

    /*
     * Cycle through the glyph specific info and set the device and scalable
     * widths.
     */
    while ((res = getc(in)) != GF_post_post) {
        /*
         * Get the encoding and locate it in the font.
         */
        num = getc(in);
        for (set = 0, gp = f->glyphs;
             set < f->glyphs_used && gp->encoding != num; gp++, set++) ;
        /*
         * If the glyph is not found for some reason, make the glyph pointer
         * point to the temporary glyph storage.
         */
        if (set == f->glyphs_used)
          gp = &g;

        if (res == GF_char_loc) {
            /*
             * Get both horizontal and vertical escapement, only keeping
             * the horizontal for the device width.
             */
            num = _bdf_mf_get32(in);
            gp->dwidth = num >> 16;
            dw = (double) gp->dwidth;
            gp->swidth = (unsigned short) ((dw * 72000.0) / denom);
            (void) _bdf_mf_get32(in);
        } else if (res == GF_char_loc0) {
            gp->dwidth = (unsigned short) getc(in);
            dw = (double) gp->dwidth;
            gp->swidth = (unsigned short) ((dw * 72000.0) / denom);
        }

        /*
         * Skip the TFM width and the glyph file offset.
         */
        fseek(in, 8, 1L);

        /*
         * Call the callback if indicated.
         */
        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.total = st.st_size;
            cb.current = ftell(in);
            (*callback)(&cb, data);
        }
    }

    /*
     * Now add the properties to the font.
     */
    prop.name = "POINT_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = f->point_size * 10;
    bdf_add_font_property(f, &prop);

    /*
     * Calculate and add the pixel size.
     */
    denom = (double) f->resolution_y;
    dw = (double) (f->point_size * 10);
    prop.name = "PIXEL_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = (long) (((denom * dw) / 722.7) + 0.5);
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_X";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned long) f->resolution_x;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_Y";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned long) f->resolution_y;
    bdf_add_font_property(f, &prop);

    prop.name = "FONT_ASCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = f->font_ascent;
    bdf_add_font_property(f, &prop);

    prop.name = "FONT_DESCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = f->font_descent;
    bdf_add_font_property(f, &prop);

    prop.name = "AVERAGE_WIDTH";
    prop.format = BDF_INTEGER;
    prop.value.int32 = (awidth / (f->unencoded_used + f->glyphs_used)) * 10;
    bdf_add_font_property(f, &prop);

    /*
     * Default all PK fonts to proportional spacing.
     */
    prop.name = "SPACING";
    prop.format = BDF_ATOM;
    prop.value.atom = "P";
    switch (f->spacing) {
      case BDF_PROPORTIONAL: prop.value.atom = "P"; break;
      case BDF_MONOWIDTH: prop.value.atom = "M"; break;
      case BDF_CHARCELL: prop.value.atom = "C"; break;
    }
    bdf_add_font_property(f, &prop);

    /*
     * Call the callback one last time if necessary.
     */
    if (callback != 0 && cb.current != cb.total) {
        cb.reason = BDF_LOADING;
        cb.total = cb.current = st.st_size;
        (*callback)(&cb, data);
    }

    /*
     * Add a message indicating the font was converted.
     */
    _bdf_add_comment(f, "Font converted from GF to BDF.", 30);
    _bdf_add_acmsg(f, "Font converted from GF to BDF.", 30);

    return BDF_OK;
}

int
bdf_load_mf_font(FILE *in, bdf_options_t *opts, bdf_callback_t callback,
                 void *data, bdf_font_t **font)
{
    unsigned char mfmag[2];

    /*
     * Load the header to see if this is a GF or PK font.
     */
    fread((char *) mfmag, 2, 1, in);
    if (mfmag[0] != GF_pre || (mfmag[1] != PK_id && mfmag[1] != 0x83))
      return BDF_NOT_MF_FONT;
    return (mfmag[1] == PK_id) ?
        _bdf_load_pk_font(in, opts, callback, data, font) :
        _bdf_load_gf_font(in, opts, callback, data, font);
}

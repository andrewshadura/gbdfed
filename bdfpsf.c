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
static char rcsid[] __attribute__ ((unused)) = "$Id: bdfpsf.c,v 1.11 2004/02/08 23:58:59 mleisher Exp $";
#else
static char rcsid[] = "$Id: bdfpsf.c,v 1.11 2004/02/08 23:58:59 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "bdfP.h"

/**************************************************************************
 *
 * PSF1 Macros and values.
 *
 **************************************************************************/

/*
 * Macros that specify the indexes of the mode and height fields of the header
 * info passed to the PSF1 loader.
 */
#define _BDF_PSF1MODE   2
#define _BDF_PSF1HEIGHT 3

/*
 * Flags which can appear in the third byte of the header (PSF1 mode).  HAS512
 * means the font contains between 512 glyphs.  Otherwise the font has 256
 * glyphs.  HASTAB and HASSEQ indicate the glyphs are followed by a Unicode
 * mappin table.
 *
 * The HASTAB and HASSEQ flags appear to be essentially equivalent.
 */
#define _BDF_PSF1_HAS512 0x01
#define _BDF_PSF1_HASTAB 0x02
#define _BDF_PSF1_HASSEQ 0x04

/**************************************************************************
 *
 * PSF2 Macros
 *
 **************************************************************************/

#define _BDF_PSF2_HASTAB 0x01

/*
 * Little endian versions of the PSF magic numbers.
 */
unsigned char _bdf_psf1magic[] = {0x36, 0x04};
unsigned char _bdf_psf2magic[] = {0x72, 0xb5, 0x4a, 0x86};

/*
 * The special header for PSF fonts that specify a list of partial
 * fonts.
 */
char _bdf_psfcombined[] = {'#', ' ', 'c', 'o'};

/*
 * The PSF2 internal header.
 */
typedef struct {
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int length;
    unsigned int bpc;
    unsigned int height;
    unsigned int width;
} _bdf_psfhdr_t;

/**************************************************************************
 *
 * Support functions.
 *
 **************************************************************************/

#define _swap_endian(n) ((n) >> 16) | (((n) & 0xffff) << 16)

static int
#ifdef __STDC__
_bdf_psf_load_map(FILE *in, bdf_font_t *font, int psf2, long *res)
#else
_bdf_psf_load_map(in, font, psf2, res)
FILE *in;
bdf_font_t *font;
int psf2;
long *res;
#endif
{
    int i, more, c0, c1, cnt;
    unsigned long code;
    unsigned char buf[4];
    bdf_glyph_t *gp;

    gp = font->glyphs;

    while ((c0 = getc(in)) >= 0) {
        /*
         * If we are still reading bytes after the end of the glyphs,
         * the table is too long.
         */
        if (gp == font->glyphs + font->glyphs_used)
          return BDF_PSF_LONG_TABLE;

        cnt = 0;
        *res = gp->encoding;
        if (!psf2) {
            if ((c1 = getc(in)) < 0)
              return BDF_PSF_SHORT_TABLE;
            if (bdf_little_endian())
              code = (c1 << 8) | (c0 & 0xff);
            else
              code = (c0 << 8) | (c1 & 0xff);

            /*
             * Convert to UTF-8.
             */
            if (code != 0xffff) {
                if (code < 0x80)
                  buf[cnt++] = code & 0xff;
                else if (code < 0x800) {
                    buf[cnt++] = 0xc0 | ((code >> 6) & 0xff);
                    buf[cnt++] = 0x80 | (code & 0x3f);
                } else if (code < 0x10000) {
                    buf[cnt++] = 0xe0 | ((code >> 12) & 0xff);
                    buf[cnt++] = 0x80 | ((code >> 6) & 0x3f);
                    buf[cnt++] = 0x80 | (code & 0x3f);
                } else if (code < 0x200000) {
                    buf[cnt++] = 0xf0 | ((code >> 18) & 0xff);
                    buf[cnt++] = 0x80 | ((code >> 12) & 0x3f);
                    buf[cnt++] = 0x80 | ((code >> 6) & 0x3f);
                    buf[cnt++] = 0x80 | (code & 0x3f);
                }
            } else
              buf[cnt++] = 0xff;
        } else {
            buf[cnt++] = c0;
            if (c0 < 0xfd && (c0 & 0x80) != 0) {
                /*
                 * Only look for more if the byte is not 0xfe or 0xff,
                 * PSF separators.
                 */
                more = 0;
                if ((c0 & 0xe0) == 0xc0)
                  more = 1;
                else if ((c0 & 0xf0) == 0xe0)
                  more = 2;
                else if ((c0 & 0xf0) == 0xf0)
                  more = 3;

                for (i = 0; i < more; i++) {
                    if ((c0 = getc(in)) < 0)
                      return BDF_PSF_SHORT_TABLE;
                    else if (c0 > 0xfd)
                      return BDF_PSF_CORRUPT_UTF8;
                    buf[cnt++] = c0;
                }
            }
        }
        if (buf[0] != 0xff) {
            if (gp->unicode.map_used + cnt > gp->unicode.map_size) {
                more = ((cnt >> 2) + ((cnt & 3) ? 1 : 0)) << 2;
                if (gp->unicode.map_size == 0)
                  gp->unicode.map = (unsigned char *)
                      malloc(sizeof(unsigned char) * more);
                else
                  gp->unicode.map = (unsigned char *)
                      realloc((char *) gp->unicode.map,
                              sizeof(unsigned char) *
                              (gp->unicode.map_size + more));
                gp->unicode.map_size += more;
            }
            (void) memcpy((char *) (gp->unicode.map + gp->unicode.map_used),
                          (char *) buf, sizeof(unsigned char) * cnt);
            gp->unicode.map_used += cnt;
        } else
          gp++;

    }
    return BDF_OK;
}

static int
#ifdef __STDC__
_bdf_psf_dump_map(FILE *out, bdf_font_t *font, bdf_glyphlist_t *glyphs)
#else
_bdf_psf_dump_map(out, font, glyphs)
FILE *out;
bdf_font_t *font;
bdf_glyphlist_t *glyphs;
#endif
{
    int seq;
    unsigned long i, nglyphs, n;
    long code;
    unsigned char *map, *map_end;
    bdf_glyph_t *gp;

    nglyphs = (glyphs->glyphs_used > 512) ? 512 : glyphs->glyphs_used;
    for (i = 0, gp = glyphs->glyphs; i < nglyphs; i++, gp++) {

        if (nglyphs > 256)
          fprintf(out, "0x%03lx", i);
        else
          fprintf(out, "0x%02lx", i);

        map = gp->unicode.map;
        map_end = map + gp->unicode.map_used;

        seq = 0;
        while (map < map_end) {
            n = 1;

            if (*map == 0xfe) {
                seq = 2;
                map++;
                continue;
            }

            /*
             * Convert from UTF-8 to UTF-32.
             */
            if ((*map & 0x80) == 0)
              /*
               * One byte character.
               */
              code = (unsigned short) *map;
            else if ((*map & 0xe0) == 0xc0) {
                /*
                 * Two byte character.
                 */
                if (map + 2 >= map_end)
                  return BDF_PSF_CORRUPT_UTF8;
                code = ((*map & 0x1f) << 6) | (*(map + 1) & 0x3f);
                n = 2;
            } else if ((*map & 0xf0) == 0xe0) {
                /*
                 * Three byte character.
                 */
                if (map + 3 >= map_end)
                  return BDF_PSF_CORRUPT_UTF8;
                code = ((*map & 0x0f) << 12) |
                    ((*(map + 1) & 0x3f) << 6) | (*(map + 2) & 0x3f);
                n = 3;
            } else if ((*map & 0xf0) == 0xf0) {
                /*
                 * Four byte supplementary character.
                 */
                if (map + 4 >= map_end)
                  return BDF_PSF_CORRUPT_UTF8;
                code = ((*map & 0x07) << 18) |
                    ((*(map + 1) & 0x3f) << 12) |
                    ((*(map + 2) & 0x3f) << 6) | (*(map + 3) & 0x3f);
                n = 4;
            }
            /*
             * Print the code(s).  If we are printing the first one,
             * then print a tab, otherwise we are printing separating
             * spaces.
             */
            if (map == gp->unicode.map)
              putc('\t', out);
            else
              putc(((seq == 1) ? ',' : ' '), out);
            fprintf(out, "U+%05lx", code);
            map += n;
            seq -= (seq == 2);
        }

        /*
         * Free the current glyph storage.
         */
        if (gp->name != 0)
          free(gp->name);
        if (gp->bytes > 0)
          free((char *) gp->bitmap);
        if (gp->unicode.map_size > 0)
          free((char *) gp->unicode.map);
    }

    /*
     * Free the storage for the glyph list.
     */
    if (glyphs->glyphs_size > 0)
      free((char *) glyphs->glyphs);

    return BDF_OK;
}

/**************************************************************************
 *
 * Public functions.
 *
 **************************************************************************/

/*
 * Return an array of strings with the Unicode encodings already formatted for
 * use with the GUI.
 */
char **
#ifdef __STDC__
_bdf_psf_unpack_mapping(bdf_psf_unimap_t *unimap, int *num_seq)
#else
    _bdf_psf_unpack_mapping(unimap, num_seq)
    bdf_psf_unimap_t *unimap;
int *num_seq;
#endif
{
    int ns, nc, sum, c;
    long code;
    unsigned char *mp, *ep;
    char **list, *lp;

    list = 0;
    if (!num_seq)
      return list;
    *num_seq = 0;

    if (unimap == 0 || unimap->map_used == 0)
      return list;

    /*
     * This routine will calculate the amount of storage is needed to be
     * allocated as one big block so an array of strings can be returned.
     * That way it can be deallocated as a single item by the caller.
     */


    /*
     * Count the total number of characters and sequences at the same time.
     */
    mp = unimap->map;
    ep = mp + unimap->map_used;
    sum = ns = nc = 0;
    while (mp < ep) {
        if (*mp == 0xfe) {
            sum = 1;
            ns++;
            mp++;
            continue;
        }

        c = 1;
        if ((*mp & 0xe0) == 0xc0)
          c = 2;
        else if ((*mp & 0xf0) == 0xe0)
          c = 3;
        else if ((*mp & 0xf0) == 0xf0)
          c = 4;

        nc++;

        mp += c;
        ns += !sum;
    }

    *num_seq = ns;

    /*
     * The block of storage will need this many bytes for pointers to the
     * sequences.
     */
    sum = sizeof(unsigned char *) * ns;

    /*
     * Each character uses 7 bytes in U+XXXXX form and each is followed by
     * either space or a null, so each is basically eight bytes.
     */
    sum += nc * 8;

    list = (char **) malloc(sum);
    lp = (char *) (list + ns);

    /*
     * Now generate the codes one at a time.
     */
    list[0] = lp;

    ns = sum = 0;
    mp = unimap->map;
    while (mp < ep) {
        if (*mp == 0xfe) {
            if (sum == 1)
              /*
               * The last thing added was a sequence, so move up to the
               * next sequence.
               */
              list[++ns] = ++lp;

            sum = 1;
            mp++;
            continue;
        }

        nc = 1;

        /*
         * Convert from UTF-8 to UTF-32.
         */
        if ((*mp & 0x80) == 0)
          /*
           * One byte character.
           */
          code = (unsigned short) *mp;
        else if ((*mp & 0xe0) == 0xc0) {
            /*
             * Two byte character.
             */
            code = ((*mp & 0x1f) << 6) | (*(mp + 1) & 0x3f);
            nc = 2;
        } else if ((*mp & 0xf0) == 0xe0) {
            /*
             * Three byte character.
             */
            code = ((*mp & 0x0f) << 12) |
                ((*(mp + 1) & 0x3f) << 6) | (*(mp + 2) & 0x3f);
            nc = 3;
        } else if ((*mp & 0xf0) == 0xf0) {
            /*
             * Four byte supplementary character.
             */
            code = ((*mp & 0x07) << 18) |
                ((*(mp + 1) & 0x3f) << 12) |
                ((*(mp + 2) & 0x3f) << 6) | (*(mp + 3) & 0x3f);
            nc = 4;
        }

        /*
         * Add to the string.
         */

        if (lp > list[ns])
          *lp++ = ' ';
        sprintf(lp, "U+%05lx", code);
        lp += 7;

        mp += nc;

        if (mp < ep && !sum)
          list[++ns] = ++lp;
    }
    return list;
}

/*
 * Routine used to insure the list of mappings is in ascending order
 * by length of string.
 */
static int
#ifdef _STDC__
cmplen(const void *a, const void *b)
#else
    cmplen(a, b)
    const void *a, *b;
#endif
{
    int n;
    char *as = *((char **)a);
    char *bs = *((char **)b);

    n = strlen(as) - strlen(bs);
    return (n) ? n : strcmp(as, bs);
}

/*
 * Taking a list of strings, generate a packed UTF-8 representation to
 * be stored back into a Unicode map.
 */
int
#ifdef __STDC__
_bdf_psf_pack_mapping(char **list, int len, long encoding,
                      bdf_psf_unimap_t *map)
#else
    _bdf_psf_pack_mapping(list, len, encoding, map)
    char **list;
int len;
long encoding;
bdf_psf_unimap_t *map;
#endif
{
    int i, j, ncodes, bytes = 3;
    char *lp, *elp;
    unsigned long codes[128];

    if (list == 0 || len == 0 || map == 0)
      return 0;

    map->map_used = 0;

    /*
     * First thing that needs to be done is to make sure the list is sorted by
     * length so the single character mappings come first.
     */
    qsort((char *) list, len, sizeof(char *), cmplen);

    for (i = 0; i < len; i++) {
        lp = list[i];
        ncodes = 0;
        while (*lp) {
            /*
             * Skip anything that isn't expected.
             */
            while (*lp && *lp != 'U' && *lp != 'u' && *lp != '0')
              lp++;
            if (*lp == 0)
              continue;
            codes[ncodes] = _bdf_atoul(lp, &elp, 16);

            /*
             * Determine how many UTF-8 bytes the current code will
             * take.
             */
            if (codes[ncodes] < 0x80)
              bytes++;
            else if (codes[ncodes] < 0x800)
              bytes += 2;
            else if (codes[ncodes] < 0x10000)
              bytes += 3;
            else if (codes[ncodes] < 0x200000)
              bytes += 4;
            ncodes++;
            lp = elp;
        }
        /*
         * Make sure there is enough room in the map for this number of bytes.
         * The number includes the encoding and the 0xff at the end on the
         * first pass.
         */
        if (map->map_used + bytes > map->map_size) {
            if (map->map_size == 0)
              map->map = (unsigned char *)
                  malloc(sizeof(unsigned char *) * 128);
            else
              map->map = (unsigned char *)
                  realloc((char *) map->map,
                          sizeof(unsigned char) * (map->map_size + 128));
            map->map_size += 128;
        }

        if (ncodes > 1)
          /*
           * Have to increment the number of bytes by 1 to include the
           * PSF2 sequence marker.
           */
          map->map[map->map_used++] = 0xfe;

        /*
         * Go through the codes and convert to UTF-8.
         */
        for (j = 0; j < ncodes; j++) {
            if (codes[j] < 0x80)
              map->map[map->map_used++] = (codes[j] & 0x7f);
            else if (codes[j] < 0x800) {
                map->map[map->map_used++] = 0xc0 | ((codes[j] >> 6) & 0xff);
                map->map[map->map_used++] = 0x80 | (codes[j] & 0x3f);
            } else if (codes[j] < 0x10000) {
                map->map[map->map_used++] = 0xe0 | ((codes[j] >> 12) & 0xff);
                map->map[map->map_used++] = 0x80 | ((codes[j] >> 6) & 0x3f);
                map->map[map->map_used++] = 0x80 | (codes[j] & 0x3f);
            } else if (codes[j] < 0x200000) {
                map->map[map->map_used++] = 0xf0 | ((codes[j] >> 18) & 0xff);
                map->map[map->map_used++] = 0x80 | ((codes[j] >> 12) & 0x3f);
                map->map[map->map_used++] = 0x80 | ((codes[j] >> 6) & 0x3f);
                map->map[map->map_used++] = 0x80 | (codes[j] & 0x3f);
            }
        }
        bytes = 0;
    }

    return BDF_OK;
}

bdf_font_t *
#ifdef __STDC__
bdf_load_psf(FILE *in, unsigned char *magic, bdf_options_t *opts,
             bdf_callback_t callback, void *data, int *awidth)
#else
    bdf_load_psf(in, magic, callback, data, awidth)
    FILE *in;
unsigned char *magic;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
int *awidth;
#endif
{
    long i, enc;
    unsigned short dwidth, swidth;
    bdf_glyph_t *gp;
    bdf_font_t *fp;
    _bdf_psfhdr_t hdr;
    bdf_callback_struct_t cb;
    char msgbuf[1024];

    /*
     * Check options for loading the Unicode table or not.
     */

    if (*magic == 0x36) {
        /*
         * PSF1 font.
         */
        hdr.version = 0;
        hdr.width = 8;
        hdr.height = hdr.bpc = (long) magic[_BDF_PSF1HEIGHT];
        hdr.length = (magic[_BDF_PSF1MODE] & _BDF_PSF1_HAS512) ? 512 : 256;
        hdr.flags = (magic[_BDF_PSF1MODE] & _BDF_PSF1_HASTAB) ?
            _BDF_PSF2_HASTAB : 0;
    } else {
        /*
         * PSF2 font.
         */
        fread((char *) &hdr, sizeof(_bdf_psfhdr_t), 1, in);
        if (!bdf_little_endian()) {
            /*
             * Need to convert all the integers to big endian.
             */
            hdr.version = _swap_endian(hdr.version);
            hdr.headersize = _swap_endian(hdr.headersize);
            hdr.flags = _swap_endian(hdr.flags);
            hdr.length = _swap_endian(hdr.length);
            hdr.bpc = _swap_endian(hdr.bpc);
            hdr.height = _swap_endian(hdr.height);
            hdr.width = _swap_endian(hdr.width);
        }
    }

    /*
     * The point size of the font will be the height, the resolution will
     * default to 72dpi, and the spacing will default to character cell.
     */
    fp = bdf_new_font(0, (long) hdr.height, 72, 72, BDF_CHARCELL, 1);

    /*
     * Force the bits per pixel to be 1.
     */
    fp->bpp = 1;

    /*
     * Set the font width and average width.
     */
    *awidth = fp->bbx.width = hdr.width;

    /*
     * Set the rest of the font bounding box parameters.
     */
    fp->font_ascent = fp->bbx.ascent;
    fp->font_descent = fp->bbx.descent;

#if 0
    /*
     * MAY NOT BE NEEDED ANY MORE.
     */

    /*
     * Adjust the ascent and descent by hand for point sizes other than 16.
     */
    if (hdr.height != 16) {
        fp->bbx.ascent++;
        fp->bbx.descent--;
    }
#endif

    /*
     * Default the font ascent and descent to that of the bounding box.
     */
    fp->font_ascent = fp->bbx.ascent;
    fp->font_descent = fp->bbx.descent;

    /*
     * Allocate the expected number of glyphs.
     */
    fp->glyphs_size = hdr.length;
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

        gp->bytes = hdr.bpc;
        gp->bitmap = (unsigned char *) malloc(hdr.bpc);
        fread((char *) gp->bitmap, hdr.bpc, 1, in);
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
     * Now load the Unicode mapping table if it exists.
     */
    if (hdr.flags & _BDF_PSF2_HASTAB) {
        msgbuf[0] = 0;
        switch (_bdf_psf_load_map(in, fp, (*magic == 0x72), &enc)) {
          case BDF_PSF_SHORT_TABLE:
            sprintf(msgbuf, "PSF Unicode table too short at 0x%04X (%ld).",
                    (unsigned short) (enc & 0xffff), enc);
            break;
          case BDF_PSF_LONG_TABLE:
            strcpy(msgbuf, "PSF Unicode table too long.");
            break;
          case BDF_PSF_CORRUPT_UTF8:
            sprintf(msgbuf, "PSF UTF-8 sequence corrupt at 0x%04X (%ld).",
                    (unsigned short) (enc & 0xffff), enc);
            break;
          case BDF_PSF_BUFFER_OVRFL:
            sprintf(msgbuf, "PSF mapping buffer overflow at 0x%04X (%ld).",
                    (unsigned short) (enc & 0xffff), enc);
            break;
        }
        if (msgbuf[0] != 0)
          _bdf_add_acmsg(fp, msgbuf, strlen(msgbuf));
    }

    sprintf(msgbuf, "Font converted from PSF%c to BDF.",
            (*magic == 0x36) ? '1' : '2');
    _bdf_add_comment(fp, msgbuf, 32);
    _bdf_add_acmsg(fp, msgbuf, 32);

    return fp;
}

/*
 * Exports all PSF fonts in PSF2 format for now.  start and end are
 * supplied when a partial font needs to be created.
 */
int
#ifdef __STDC__
bdf_export_psf(FILE *out, bdf_font_t *font, bdf_options_t *opts, long start,
               long end)
#else
bdf_export_psf(out, font, opts, start, end)
FILE *out;
bdf_font_t *font;
bdf_options_t *opts;
long start, end;
#endif
{
    unsigned int i, nglyphs, flags;
    _bdf_psfhdr_t hdr;
    bdf_glyph_t *gp;
    bdf_font_t tmpfont;
    bdf_glyphlist_t glyphs;
    bdf_glyph_t cell;

    if (font->glyphs_used == 0)
      return BDF_EMPTY_FONT;

    /*
     * This routine only exports from CHARCELL and MONOWIDTH fonts, padding
     * the glyphs as it writes.
     */
    if (font->spacing == BDF_PROPORTIONAL)
      return BDF_EMPTY_FONT;

    if (start == end)
      return BDF_BAD_RANGE;

    /*
     * Make a copy of the glyphs so we can get the smallest bounding box for
     * the glyphs being exported.  This also does a bit of range checking.
     */
    (void) memset((char *) &glyphs, 0, sizeof(bdf_glyphlist_t));
    bdf_copy_glyphs(font, start, end, &glyphs, 0);

    /*
     * At this point, if only the Unicode table is desired, then
     * call the routine that prints the plain text version.
     */
    if ((opts->psf_flags == BDF_PSF_UNIMAP))
      return _bdf_psf_dump_map(out, font, &glyphs);

    /*
     * Set up the temporary font so glyph padding will happen like it is
     * supposed to.
     */
    tmpfont.bpp = glyphs.bpp;
    (void) memcpy((char *) &tmpfont.bbx, (char *) &glyphs.bbx,
                  sizeof(bdf_bbx_t));

    /*
     * Create the header.  The extra 4 on the header size account
     * for the magic number.
     *
     * Number of glyphs and flags have to be calculated properly before writing
     * so it isn't necessary to go back and rewrite the header after the font
     * has been written.  That causes havoc when writing to stdout.
     */
    hdr.version = hdr.flags = 0;
    hdr.headersize = sizeof(_bdf_psfhdr_t) + 4;
    hdr.length = (glyphs.glyphs_used > 512) ? 512 : glyphs.glyphs_used;
    hdr.width = glyphs.bbx.width;
    hdr.height = glyphs.bbx.height;
    hdr.bpc = hdr.height * ((hdr.width + 7) >> 3);

    /*
     * Determine if the font will have a Unicode mapping table.
     */
    for (i = 0; i < hdr.length; i++) {
        if (glyphs.glyphs[i].unicode.map_used > 0) {
            hdr.flags |= _BDF_PSF2_HASTAB;
            break;
        }
    }

    /*
     * Save these values so it doesn't get whacked in an endian conversion.
     */
    nglyphs = hdr.length;
    flags = hdr.flags;

    /*
     * Set up a structure for padding glyphs to cell boundaries.
     */
    cell.bytes = hdr.bpc;
    cell.bitmap = (unsigned char *) malloc(cell.bytes);
    (void) memcpy((char *) &cell.bbx, (char *) &glyphs.bbx, sizeof(bdf_bbx_t));

    if (!bdf_little_endian()) {
        /*
         * Swap the integers into little endian order before writing.
         */
        hdr.version = _swap_endian(hdr.version);
        hdr.headersize = _swap_endian(hdr.headersize);
        hdr.flags = _swap_endian(hdr.flags);
        hdr.length = _swap_endian(hdr.length);
        hdr.bpc = _swap_endian(hdr.bpc);
        hdr.height = _swap_endian(hdr.height);
        hdr.width = _swap_endian(hdr.width);
    }

    /*
     * Write the header.
     */
    fwrite((char *) _bdf_psf2magic, sizeof(unsigned char), 4, out);
    fwrite((char *) &hdr, sizeof(_bdf_psfhdr_t), 1, out);

    /*
     * Generate the glyphs, padding them out to the dimensions of the
     * font.
     */
    for (i = 0, gp = glyphs.glyphs; i < nglyphs; i++, gp++) {
        /*
         * We only need to do cropping on CHARCELL glyphs because MONOWIDTH
         * glyphs are already cropped to their minimum dimensions.
         */
        if (font->spacing == BDF_CHARCELL)
          _bdf_crop_glyph(&tmpfont, gp);
        _bdf_pad_cell(&tmpfont, gp, &cell);
        fwrite((char *) cell.bitmap, sizeof(unsigned char), cell.bytes, out);
    }

    /*
     * Now generate the Unicode table if called for.
     */
    if ((opts->psf_flags & BDF_PSF_UNIMAP) && (flags & _BDF_PSF2_HASTAB)) {
        for (gp = glyphs.glyphs, i = 0; i < nglyphs; i++, gp++) {
            if (gp->unicode.map_used > 0)
              fwrite((char *) gp->unicode.map, sizeof(unsigned char),
                     gp->unicode.map_used, out);
            putc(0xff, out);
        }
    }

    /*
     * Finally, dispose of the glyph copies.
     */
    for (i = 0, gp = glyphs.glyphs; i < glyphs.glyphs_used; i++, gp++) {
        if (gp->name != 0)
          free(gp->name);
        if (gp->bytes > 0)
          free((char *) gp->bitmap);
        if (gp->unicode.map_size > 0)
          free((char *) gp->unicode.map);
    }
    if (glyphs.glyphs_size > 0)
      free((char *) glyphs.glyphs);

    return BDF_OK;
}

#undef _swap_endian

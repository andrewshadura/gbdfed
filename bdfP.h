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
#ifndef _h_bdfP
#define _h_bdfP

/*
 * $Id: bdfP.h,v 1.17 2004/02/08 23:58:59 mleisher Exp $
 */

#include "bdf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A macro for prototypes.
 */
#undef __
#ifdef __STDC__
#define __(x) x
#else
#define __(x) ()
#endif

#ifndef MYABS
#define MYABS(xx) ((xx) < 0 ? -(xx) : (xx))
#endif

/*
 * Macros and structures used for undo operations in the font.
 */
#define _UNDO_REPLACE_GLYPHS 1
#define _UNDO_INSERT_GLYPHS  2
#define _UNDO_MERGE_GLYPHS   3

/*
 * This structure is for undo operations of replacing and merging glyphs
 * in the font.
 */
typedef struct {
    bdf_bbx_t b;
    bdf_glyphlist_t g;
} _bdf_undo1_t;

/*
 * This structure is for undo operations of inserting glyphs.
 */
typedef struct {
    bdf_bbx_t b;
    long start;
    long end;
} _bdf_undo2_t;

/*
 * This is the final undo structure used to store undo information with the
 * font.
 */
typedef struct {
    int type;
    union {
        _bdf_undo1_t one;
        _bdf_undo2_t two;
    } field;
} _bdf_undo_t;

/*
 * Tables for rotation and shearing.
 */
extern double _bdf_cos_tbl[];
extern double _bdf_sin_tbl[];
extern double _bdf_tan_tbl[];

/*
 * PSF magic numbers.
 */
extern unsigned char _bdf_psf1magic[];
extern unsigned char _bdf_psf2magic[];
extern char _bdf_psfcombined[];

/*
 * Arrays of masks for test with different bits per pixel.
 */
extern unsigned char onebpp[];
extern unsigned char twobpp[];
extern unsigned char fourbpp[];

/*
 * Simple routine for determining the ceiling.
 */
extern short _bdf_ceiling __((double v));

extern unsigned char *_bdf_strdup __((unsigned char *s, unsigned long len));
extern void _bdf_memmove __((char *dest, char *src, unsigned long bytes));

extern short _bdf_atos __((char *s, char **end, int base));
extern long _bdf_atol __((char *s, char **end, int base));
extern unsigned long _bdf_atoul __((char *s, char **end, int base));

/*
 * Function to locate the nearest glyph to a specified encoding.
 */
extern bdf_glyph_t *_bdf_locate_glyph __((bdf_font_t *font, long encoding,
                                          int unencoded));

/*
 * Macros to test/set the modified status of a glyph.
 */
#define _bdf_glyph_modified(map, e) ((map)[(e) >> 5] & (1 << ((e) & 31)))
#define _bdf_set_glyph_modified(map, e) (map)[(e) >> 5] |= (1 << ((e) & 31))
#define _bdf_clear_glyph_modified(map, e) (map)[(e) >> 5] &= ~(1 << ((e) & 31))

/*
 * Function to add a message to the font.
 */
extern void _bdf_add_acmsg __((bdf_font_t *font, char *msg,
                               unsigned long len));

/*
 * Function to add a comment to the font.
 */
extern void _bdf_add_comment __((bdf_font_t *font, char *comment,
                                 unsigned long len));

/*
 * Function to do glyph name table cleanup when exiting.
 */
extern void _bdf_glyph_name_cleanup __((void));

/*
 * Function to pad cells when saving glyphs.
 */
extern void _bdf_pad_cell __((bdf_font_t *font, bdf_glyph_t *glyph,
                              bdf_glyph_t *cell));

/*
 * Function to crop glyphs down to their minimum bitmap.
 */
extern void _bdf_crop_glyph __((bdf_font_t *font, bdf_glyph_t *glyph));

/*
 * Routine to generate a string list from the PSF2 Unicode mapping format.
 */
extern char **_bdf_psf_unpack_mapping __((bdf_psf_unimap_t *unimap,
                                          int *num_seq));

/*
 * Routine to convert a string list of mappings back to PSF2 format.
 */
extern int _bdf_psf_pack_mapping __((char **list, int len,
                                     long encoding, bdf_psf_unimap_t *map));

#if 0
/*
 * This routine is used for locating Unicode mappings for PSF fonts.
 */
extern int _bdf_psf_locate_mapping __((bdf_font_t *font, long encoding,
                                       unsigned char **pos,
                                       unsigned long *len));

/*
 * A routine to replace Unicode mappings in the font.
 */
extern int _bdf_psf_replace_mappings __((bdf_font_t *font, long from,
                                         bdf_psf_unimap_t *map));
#endif

#undef __

#ifdef __cplusplus
}
#endif

#endif /* _h_bdfP */

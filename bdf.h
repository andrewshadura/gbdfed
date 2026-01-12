/*
 * Copyright 2008 Department of Mathematical Sciences, New Mexico State University
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
 * DEPARTMENT OF MATHEMATICAL SCIENCES OR NEW MEXICO STATE UNIVERSITY BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _h_bdf
#define _h_bdf

#include <stdio.h>
#include <stdlib.h>
#ifndef __digital__
#include <unistd.h>
#endif
#include <string.h>

#ifdef HAVE_XLIB
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif /* HAVE_XLIB */

#ifdef HAVE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif /* HAVE_FREETYPE */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *
 * BDF font options macros and types.
 *
 **************************************************************************/

#define BDF_UNIX_EOL 1           /* Save fonts with Unix LF.              */
#define BDF_DOS_EOL  2           /* Save fonts with DOS CRLF.             */
#define BDF_MAC_EOL  3           /* Save fonts with Mac CR.               */

#define BDF_CORRECT_METRICS 0x01 /* Correct invalid metrics when loading. */
#define BDF_KEEP_COMMENTS   0x02 /* Preserve the font comments.           */
#define BDF_KEEP_UNENCODED  0x04 /* Keep the unencoded glyphs.            */
#define BDF_PROPORTIONAL    0x08 /* Font has proportional spacing.        */
#define BDF_MONOWIDTH       0x10 /* Font has mono width.                  */
#define BDF_CHARCELL        0x20 /* Font has charcell spacing.            */

#define BDF_ALL_SPACING (BDF_PROPORTIONAL|BDF_MONOWIDTH|BDF_CHARCELL)

#define BDF_DEFAULT_LOAD_OPTIONS \
    (BDF_CORRECT_METRICS|BDF_KEEP_COMMENTS|BDF_KEEP_UNENCODED|BDF_PROPORTIONAL)

typedef struct {
    int otf_flags;
    int correct_metrics;
    int keep_unencoded;
    int keep_comments;
    int pad_cells;
    int font_spacing;
    int point_size;
    unsigned int resolution_x;
    unsigned int resolution_y;
    int bits_per_pixel;
    int eol;
    int psf_flags;
    int cursor_font;
} bdf_options_t;

/*
 * Callback function type for unknown configuration options.
 */
typedef int (*bdf_options_callback_t)(bdf_options_t *opts,
                                      char **params,
                                      unsigned int nparams,
                                      void *client_data);

/**************************************************************************
 *
 * BDF font property macros and types.
 *
 **************************************************************************/

#define BDF_ATOM     1
#define BDF_INTEGER  2
#define BDF_CARDINAL 3

/*
 * This structure represents a particular property of a font.
 * There are a set of defaults and each font has their own.
 */
typedef struct {
    char *name;         /* Name of the property.                        */
    int format;         /* Format of the property.                      */
    int builtin;        /* A builtin property.                          */
    union {
        char *atom;
        int int32;
        unsigned int card32;
    } value;            /* Value of the property.                       */
} bdf_property_t;

/**************************************************************************
 *
 * SBIT metrics specific structures.
 *
 **************************************************************************/

/*
 * Boolean flags for SBIT metrics files.
 */
#define BDF_SBIT_MONO_ADVANCE 0x0001
#define BDF_SBIT_ADD_EBLC     0x0002
#define BDF_SBIT_APPLE_COMPAT 0x0004

/*
 * Direction macros (inclusive, can be combined).
 */
#define BDF_SBIT_HORIZONTAL   0x0008
#define BDF_SBIT_VERTICAL     0x0010

/*
 * Bitmap storage options (exclusive, cannot be combined).
 */
#define BDF_SBIT_STORE_SMALL  0x0020
#define BDF_SBIT_STORE_FAST   0x0040

typedef struct {
    short cnum;         /* Caret slope numerator.                       */
    short cdenom;       /* Caret slope denominator.                     */
    short coff;         /* Caret offset.                                */
    short sx;           /* Scaled version horizontal PPEM size.         */
    short sy;           /* Scaled version vertical PPEM size (optional).*/
    short flags;        /* Booleans and other non-numeric values.       */
} bdf_sbit_t;

/**************************************************************************
 *
 * BDF opaque undo information types.
 *
 **************************************************************************/

typedef struct _bdf_undo_t *bdf_undo_t;

/**************************************************************************
 *
 * PSF font flags and Unicode mapping tables.  Stored internally in UTF-8.
 *
 **************************************************************************/

/*
 * Flags used for exporting PSF fonts and their Unicode maps.
 */
#define BDF_PSF_FONT   0x01
#define BDF_PSF_UNIMAP 0x02
#define BDF_PSF_ALL (BDF_PSF_FONT|BDF_PSF_UNIMAP)

typedef struct {
    unsigned char *map;
    unsigned int map_used;
    unsigned int map_size;
} bdf_psf_unimap_t;

/**************************************************************************
 *
 * BDF font metric and glyph types.
 *
 **************************************************************************/

/*
 * A general bitmap type, mostly used when the glyph bitmap is being edited.
 */
typedef struct {
    short x;
    short y;
    unsigned short width;
    unsigned short height;
    unsigned short bpp;
    unsigned short pad;
    unsigned char *bitmap;
    unsigned int bytes;
} bdf_bitmap_t;

typedef struct {
    int font_spacing;
    unsigned short swidth;
    unsigned short dwidth;
    unsigned short width;
    unsigned short height;
    short x_offset;
    short y_offset;
    short ascent;
    short descent;
} bdf_metrics_t;

typedef struct {
    unsigned short width;
    unsigned short height;
    short x_offset;
    short y_offset;
    short ascent;
    short descent;
} bdf_bbx_t;

typedef struct {
    char *name;                 /* Glyph name.                          */
    int encoding;              /* Glyph encoding.                      */
    unsigned short swidth;      /* Scalable width.                      */
    unsigned short dwidth;      /* Device width.                        */
    bdf_bbx_t bbx;              /* Glyph bounding box.                  */
    unsigned char *bitmap;      /* Glyph bitmap.                        */
    unsigned short bytes;       /* Number of bytes used for the bitmap. */
    bdf_psf_unimap_t unicode;   /* PSF Unicode mappings.                */
} bdf_glyph_t;

typedef struct {
    unsigned short pad;         /* Pad to 4-byte boundary.              */
    unsigned short bpp;         /* Bits per pixel.                      */
    int start;                 /* Beginning encoding value of glyphs.  */
    int end;                   /* Ending encoding value of glyphs.     */
    bdf_glyph_t *glyphs;        /* Glyphs themselves.                   */
    unsigned int glyphs_size;  /* Glyph structures allocated.          */
    unsigned int glyphs_used;  /* Glyph structures used.               */
    bdf_bbx_t bbx;              /* Overall bounding box of glyphs.      */
} bdf_glyphlist_t;

typedef struct {
    char *name;                 /* Name of the font.                     */
    bdf_bbx_t bbx;              /* Font bounding box.                    */

    int point_size;            /* Point size of the font.               */
    unsigned int resolution_x; /* Font horizontal resolution.           */
    unsigned int resolution_y; /* Font vertical resolution.             */

    int hbf;                    /* Font came from an HBF font.           */

    int spacing;                /* Font spacing value.                   */

    unsigned short monowidth;   /* Logical width for monowidth font.     */

    int default_glyph;         /* Encoding of the default glyph.        */

    int font_ascent;           /* Font ascent.                          */
    int font_descent;          /* Font descent.                         */

    int glyphs_size;           /* Glyph structures allocated.           */
    int glyphs_used;           /* Glyph structures used.                */
    bdf_glyph_t *glyphs;        /* Glyphs themselves.                    */

    int unencoded_size;        /* Unencoded glyph structures allocated. */
    int unencoded_used;        /* Unencoded glyph structures used.      */
    bdf_glyph_t *unencoded;     /* Unencoded glyphs themselves.          */

    unsigned int props_size;   /* Font properties allocated.            */
    unsigned int props_used;   /* Font properties used.                 */
    bdf_property_t *props;      /* Font properties themselves.           */

    char *comments;             /* Font comments.                        */
    unsigned int comments_len; /* Length of comment string.             */

    char *acmsgs;               /* Auto-correction messages.             */
    unsigned int acmsgs_len;   /* Length of auto-correction messages.   */

    bdf_glyphlist_t overflow;   /* Storage used for glyph insertion.     */

    void *internal;             /* Internal data for the font.           */

    unsigned int nmod[2048];   /* Bitmap indicating modified glyphs.    */
    unsigned int umod[2048];   /* Bitmap indicating modified unencoded. */

    unsigned short modified;    /* Boolean indicating font modified.     */
    unsigned short bpp;         /* Bits per pixel.                       */

    bdf_sbit_t *sbits;          /* Associcated SBIT metrics.             */
    unsigned int sbits_used;   /* Number of SBIT metrics entries.       */
    unsigned int sbits_size;   /* Amount of entries allocated.          */

    bdf_undo_t *undo_stack;     /* Record of undoable operations.        */
    unsigned int undo_used;    /* Amount of undo stack used.            */
    unsigned int undo_size;    /* Amount of undo stack allocated.       */

    bdf_psf_unimap_t unicode;   /* PSF Unicode table.                    */
} bdf_font_t;

/**************************************************************************
 *
 * BDF glyph grid structures for editing glyph bitmaps.
 *
 **************************************************************************/

typedef struct {
    char *name;
    int encoding;              /* The glyph encoding.                  */
    unsigned short unencoded;   /* Whether the glyph was unencoded.     */
    unsigned short bpp;         /* Bits per pixel.                      */
    int spacing;                /* Font spacing.                        */
    int resolution_x;          /* Horizontal resolution.               */
    int resolution_y;          /* Vertical resolution.                 */
    unsigned int point_size;   /* Font point size.                     */
    unsigned short swidth;      /* Scalable width.                      */
    unsigned short dwidth;      /* Device width.                        */
    bdf_bbx_t font_bbx;         /* Font bounding box.                   */
    bdf_bbx_t glyph_bbx;        /* Glyph bounding box.                  */
    unsigned char *bitmap;      /* The grid bitmap.                     */
    unsigned short bytes;       /* Number of bytes in the grid bitmap.  */
    short grid_width;           /* Width of the grid.                   */
    short grid_height;          /* Height of the grid.                  */
    short base_x;               /* Baseline X coordinate.               */
    short base_y;               /* Baseline Y coordinate.               */
    short glyph_x;              /* Top-left X position of glyph.        */
    short glyph_y;              /* Top-left Y position of glyph.        */
    unsigned short modified;    /* Flag indicating if bitmap modified.  */
    short cap_height;           /* Font CAP_HEIGHT if it exists.        */
    short x_height;             /* Font X_HEIGHT if it exists.          */
    bdf_bitmap_t sel;           /* Selected portion of the glyph bitmap.*/
    bdf_psf_unimap_t unicode;   /* PSF Unicode mappings for this glyph. */
} bdf_glyph_grid_t;

/**************************************************************************
 *
 * Types for load/save callbacks.
 *
 **************************************************************************/

/*
 * Callback reasons.
 */
#define BDF_LOAD_START       1
#define BDF_LOADING          2
#define BDF_SAVE_START       3
#define BDF_SAVING           4
#define BDF_TRANSLATE_START  5
#define BDF_TRANSLATING      6
#define BDF_ROTATE_START     7
#define BDF_ROTATING         8
#define BDF_SHEAR_START      9
#define BDF_SHEARING         10
#define BDF_GLYPH_NAME_START 11
#define BDF_GLYPH_NAME       12
#define BDF_EXPORT_START     13
#define BDF_EXPORTING        14
#define BDF_EMBOLDEN_START   15
#define BDF_EMBOLDENING      16
#define BDF_WARNING          20
#define BDF_ERROR            21

/*
 * Error codes.
 */
#define BDF_OK                 0
#define BDF_MISSING_START     -1
#define BDF_MISSING_FONTNAME  -2
#define BDF_MISSING_SIZE      -3
#define BDF_MISSING_FONTBBX   -4
#define BDF_MISSING_CHARS     -5
#define BDF_MISSING_STARTCHAR -6
#define BDF_MISSING_ENCODING  -7
#define BDF_MISSING_BBX       -8
#define BDF_BBX_TOO_BIG       -9

#define BDF_NOT_CONSOLE_FONT  -10
#define BDF_NOT_MF_FONT       -11
#define BDF_NOT_PSF_FONT      -12
#define BDF_PSF_SHORT_TABLE   -13
#define BDF_PSF_LONG_TABLE    -14
#define BDF_PSF_CORRUPT_UTF8  -15
#define BDF_PSF_BUFFER_OVRFL  -16
#define BDF_PSF_UNSUPPORTED   -17
#define BDF_MISSING_ENDCHAR   -18
#define BDF_BAD_RANGE         -98
#define BDF_EMPTY_FONT        -99
#define BDF_INVALID_LINE      -100

typedef struct {
    unsigned int reason;
    unsigned int current;
    unsigned int total;
    unsigned int errlineno;
} bdf_callback_struct_t;

typedef void (*bdf_callback_t)(bdf_callback_struct_t *call_data,
                               void *client_data);

/**************************************************************************
 *
 * BDF font API.
 *
 **************************************************************************/

/*
 * Startup and shutdown functions.
 */
extern void bdf_setup(void);
extern void bdf_cleanup(void);

/*
 * Configuration file loading and saving.
 */
extern void bdf_load_options(FILE *in, bdf_options_t *opts,
                             bdf_options_callback_t callback,
                             void *client_data);
extern void bdf_save_options(FILE *out, bdf_options_t *opts);

/*
 * Font options functions.
 */
extern void bdf_default_options(bdf_options_t *opts);

/*
 * Font load, create, save and free functions.
 */
extern bdf_font_t *bdf_new_font(char *name, int point_size,
                                int resolution_x, int resolution_y,
                                int spacing, int bpp);
extern bdf_font_t *bdf_load_font(FILE *in, bdf_options_t *opts,
                                 bdf_callback_t callback, void *data);
#ifdef HAVE_HBF
extern bdf_font_t *bdf_load_hbf_font(char *filename, bdf_options_t *opts,
                                     bdf_callback_t callback, void *data);
#endif

#ifdef HAVE_XLIB
extern bdf_font_t *bdf_load_server_font(Display *d, XFontStruct *f,
                                        char *name, bdf_options_t *opts,
                                        bdf_callback_t callback,
                                        void *data);
#endif /* HAVE_XLIB */

extern int bdf_load_console_font(FILE *in, bdf_options_t *opts,
                                 bdf_callback_t callback, void *data,
                                 bdf_font_t *fonts[3], int *nfonts);

extern int bdf_load_mf_font(FILE *in, bdf_options_t *opts,
                            bdf_callback_t callback, void *data,
                            bdf_font_t **font);

extern void bdf_save_font(FILE *out, bdf_font_t *font, bdf_options_t *opts,
                          bdf_callback_t callback, void *data);

extern void bdf_save_sbit_metrics(FILE *out, bdf_font_t *font,
                                  bdf_options_t *opts, char *appname);

extern void bdf_export_hex(FILE *out, bdf_font_t *font, bdf_options_t *opts,
                           bdf_callback_t callback, void *data);

extern int bdf_export_psf(FILE *out, bdf_font_t *font, bdf_options_t *opts,
                          int start, int end);

extern void bdf_free_font(bdf_font_t *font);

#ifdef HAVE_FREETYPE

/*
 * OpenType related macros and functions.
 */

/*
 * ID numbers of the strings that can appear in an OpenType font.
 */
#define BDFOTF_COPYRIGHT_STRING     0
#define BDFOTF_FAMILY_STRING        1
#define BDFOTF_SUBFAMILY_STRING     2
#define BDFOTF_UNIQUEID_STRING      3
#define BDFOTF_FULLNAME_STRING      4
#define BDFOTF_VENDOR_STRING        5
#define BDFOTF_POSTSCRIPT_STRING    6
#define BDFOTF_TRADEMARK_STRING     7
#define BDFOTF_FOUNDRY_STRING       8
#define BDFOTF_DESIGNER_STRING      9
#define BDFOTF_DESCRIPTION_STRING   10
#define BDFOTF_VENDORURL_STRING     11
#define BDFOTF_DESIGNERURL_STRING   12
#define BDFOTF_LICENSE_STRING       13
#define BDFOTF_LICENSEURL_STRING    14
#define BDFOTF_RESERVED_STRING      15
#define BDFOTF_PREFFAMILY_STRING    16
#define BDFOTF_PREFSUBFAMILY_STRING 17
#define BDFOTF_COMPATIBLEMAC_STRING 18
#define BDFOTF_SAMPLETEXT_STRING    19
#define BDFOTF_PSCIDFF_STRING       20

extern char *bdfotf_platform_name(short pid);
extern char *bdfotf_encoding_name(short pid, short eid);
extern int bdfotf_get_english_string(FT_Face face, int nameID,
                                     int dash_to_space, char *name);

extern int bdfotf_load_font(FT_Face face, short pid, short eid,
                            bdf_options_t *opts, bdf_callback_t callback,
                            void *data, bdf_font_t **font);

#endif /* HAVE_FREETYPE */

/*
 * FON/FNT related functions.
 */

/*
 * String ID numbers for FON/FNT fonts.
 */
#define BDFFNT_COPYRIGHT 1
#define BDFFNT_TYPEFACE  2

/*
 * Opaque font type.
 */
typedef struct _bdffnt_font_t *bdffnt_font_t;

extern int bdffnt_open_font(char *path, bdffnt_font_t *font);
extern void bdffnt_close_font(bdffnt_font_t font);
extern int bdffnt_font_count(bdffnt_font_t font);
extern int bdffnt_get_copyright(bdffnt_font_t font, unsigned int fontID,
                                unsigned char *string);
extern int bdffnt_get_facename(bdffnt_font_t font, unsigned int fontID,
                               int for_xlfd, unsigned char *string);
extern int bdffnt_char_count(bdffnt_font_t font, unsigned int fontID);
extern int bdffnt_font_pointsize(bdffnt_font_t font, unsigned int fontID);
extern int bdffnt_load_font(bdffnt_font_t font, unsigned int fontID,
                            bdf_callback_t callback, void *data,
                            bdf_font_t **out);

/*
 * PSF font section.
 *
 * In PSF fonts, a Unicode table on the end of the font may map a single
 * glyph to several locations.  The BDFPSF_SOURCE_GLYPH marks the glyphs that
 * are source glyphs and the BDFPSF_PSEUDO_GLYPH flag marks glyphs that are
 * clones of a source glyph.
 */
#define BDFPSF_SOURCE_GLYPH 0x0001
#define BDFPSF_PSEUDO_GLYPH 0x0002

extern bdf_font_t *bdf_load_psf(FILE *in, unsigned char *magic,
                                bdf_options_t *opts,
                                bdf_callback_t callback, void *data,
                                int *awidth);

/*
 * Font property functions.
 */
extern void bdf_create_property(char *name, int type);
extern bdf_property_t *bdf_get_property(char *name);
extern unsigned int bdf_property_list(bdf_property_t **props);

extern void bdf_add_font_property(bdf_font_t *font, bdf_property_t *property);
extern void bdf_delete_font_property(bdf_font_t *font, char *name);
extern bdf_property_t *bdf_get_font_property(bdf_font_t *font, char *name);
extern unsigned int bdf_font_property_list(bdf_font_t *font,
                                            bdf_property_t **props);
extern int bdf_is_xlfd_property(char *name);

/*
 * Font comment functions.
 */
extern int bdf_replace_comments(bdf_font_t *font, char *comments,
                                unsigned int comments_len);

/*
 * Other miscellaneous functions.
 */
extern void bdf_set_default_metrics(bdf_font_t *font);

/*
 * Font glyph editing functions.
 */
extern int bdf_glyph_modified(bdf_font_t *font, int which, int unencoded);

extern void bdf_copy_glyphs(bdf_font_t *font, int start, int end,
                            bdf_glyphlist_t *glyphs, int unencoded);

extern int bdf_delete_glyphs(bdf_font_t *font, int start, int end,
                             int unencoded);

extern int bdf_insert_glyphs(bdf_font_t *font, int start,
                             bdf_glyphlist_t *glyphs);

extern int bdf_replace_glyphs(bdf_font_t *font, int start,
                              bdf_glyphlist_t *glyphs, int unencoded);

extern int bdf_merge_glyphs(bdf_font_t *font, int start,
                            bdf_glyphlist_t *glyphs, int unencoded);

extern int bdf_replace_mappings(bdf_font_t *font, int encoding,
                                bdf_psf_unimap_t *map, int unencoded);

/**************************************************************************
 *
 * Other API functions.
 *
 **************************************************************************/

extern int bdf_set_font_bbx(bdf_font_t *font, bdf_metrics_t *metrics);

extern void bdf_set_modified(bdf_font_t *font, int modified);

extern int bdf_has_xlfd_name(bdf_font_t *font);

extern char *bdf_make_xlfd_name(bdf_font_t *font, const char *foundry,
                                const char *family);

extern void bdf_update_name_from_properties(bdf_font_t *font);

extern int bdf_update_properties_from_name(bdf_font_t *font);

extern int bdf_update_average_width(bdf_font_t *font);

extern int bdf_set_unicode_glyph_names(FILE *in, bdf_font_t *font,
                                       bdf_callback_t callback);

extern int bdf_set_adobe_glyph_names(FILE *in, bdf_font_t *font,
                                     bdf_callback_t callback);

extern int bdf_set_glyph_code_names(int prefix, bdf_font_t *font,
                                    bdf_callback_t callback);

/*
 * Routine to add Unicode mappings when editing PSF fonts.
 */
extern int bdf_psf_add_unicode_mapping(bdf_psf_unimap_t *u,
                                       unsigned int *mapping,
                                       unsigned int mapping_cnt);

/**************************************************************************
 *
 * Glyph grid API.
 *
 **************************************************************************/

/*
 * Glyph grid allocation and deallocation functions.
 */
extern bdf_glyph_grid_t *bdf_make_glyph_grid(bdf_font_t *font,
                                             int code,
                                             int unencoded);
extern void bdf_free_glyph_grid(bdf_glyph_grid_t *grid);

/*
 * Glyph grid information functions.
 */
extern void bdf_grid_image(bdf_glyph_grid_t *grid, bdf_bitmap_t *image);
extern void bdf_grid_origin(bdf_glyph_grid_t *grid, short *x, short *y);
extern bdf_glyph_t *bdf_grid_glyph(bdf_glyph_grid_t *grid);

/*
 * Glyph grid editing functions.
 */
extern int bdf_grid_enlarge(bdf_glyph_grid_t *grid, unsigned short width,
                            unsigned short height);
extern int bdf_grid_resize(bdf_glyph_grid_t *grid,
                           bdf_metrics_t *metrics);
extern int bdf_grid_crop(bdf_glyph_grid_t *grid, int grid_modified);

extern int bdf_grid_set_pixel(bdf_glyph_grid_t *grid, short x, short y,
                              int val);
extern int bdf_grid_clear_pixel(bdf_glyph_grid_t *grid, short x, short y);
extern int bdf_grid_invert_pixel(bdf_glyph_grid_t *grid,
                                 short x, short y, int val);
extern int bdf_grid_shift(bdf_glyph_grid_t *grid, short xcount,
                          short ycount);
extern int bdf_grid_flip(bdf_glyph_grid_t *grid, short dir);
extern int bdf_grid_rotate(bdf_glyph_grid_t *grid, short degrees,
                           int *resize);
extern int bdf_grid_shear(bdf_glyph_grid_t *grid, short degrees,
                          int *resize);
extern int bdf_grid_embolden(bdf_glyph_grid_t *grid);

/*
 * Glyph grid selection functions.
 */
extern int bdf_has_selection(bdf_glyph_grid_t *grid, short *x, short *y,
                             short *width, short *height);
extern void bdf_set_selection(bdf_glyph_grid_t *grid, short x, short y,
                              short width, short height);
extern void bdf_lose_selection(bdf_glyph_grid_t *grid);
extern void bdf_detach_selection(bdf_glyph_grid_t *grid);
extern void bdf_attach_selection(bdf_glyph_grid_t *grid);
extern void bdf_delete_selection(bdf_glyph_grid_t *grid);
extern int bdf_in_selection(bdf_glyph_grid_t *grid, short x, short y,
                            short *set);
extern void bdf_add_selection(bdf_glyph_grid_t *grid, bdf_bitmap_t *sel);

/*
 * Glyph grid misc functions.
 */
extern int bdf_grid_color_at(bdf_glyph_grid_t *grid, short x, short y);

/*
 * Graphical transformation functions.
 */
extern int bdf_translate_glyphs(bdf_font_t *font, short dx, short dy,
                                int start, int end,
                                bdf_callback_t callback, void *data,
                                int unencoded);

extern int bdf_rotate_glyphs(bdf_font_t *font, short degrees,
                             int start, int end,
                             bdf_callback_t callback, void *data,
                             int unencoded);

extern int bdf_shear_glyphs(bdf_font_t *font, short degrees,
                            int start, int end,
                            bdf_callback_t callback, void *data,
                            int unencoded);

extern int bdf_embolden_glyphs(bdf_font_t *font, int start, int end,
                               bdf_callback_t callback, void *data,
                               int unencoded, int *resize);

extern int bdf_little_endian(void);

#ifdef __cplusplus
}
#endif

#endif /* _h_bdf */

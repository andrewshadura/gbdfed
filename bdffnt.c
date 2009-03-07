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

#include "bdfP.h"

/**************************************************************************
 *
 * Executable header and font structures.
 *
 **************************************************************************/

typedef struct {
    unsigned short id;
    unsigned short count;
    unsigned int reshandler;
} res_typeinfo_t;

typedef struct {
    unsigned short offset;
    unsigned short length;
    unsigned short flags;
    unsigned short id;
    unsigned short handle;
    unsigned short usage;
} res_nameinfo_t;

typedef struct {
    unsigned short e_magic;
    unsigned short e_cblp;
    unsigned short e_cp;
    unsigned short e_crlc;
    unsigned short e_cparhdr;
    unsigned short e_minalloc;
    unsigned short e_maxalloc;
    unsigned short e_ss;
    unsigned short e_sp;
    unsigned short e_csum;
    unsigned short e_ip;
    unsigned short e_cs;
    unsigned short e_lfarlc;
    unsigned short e_ovno;
    unsigned short e_res[4];
    unsigned short e_oemid;
    unsigned short e_oeminfo;
    unsigned short e_res2[10];
    unsigned short e_lfanew;
} dos_exe_t;

typedef struct {
    unsigned short ne_magic;
    unsigned char  linker_version;
    unsigned char  linker_revision;
    unsigned short entry_tab_offset;
    unsigned short entry_tab_length;
    unsigned int  reserved1;
    unsigned short format_flags;
    unsigned short auto_data_seg;
    unsigned short local_heap_length;
    unsigned short stack_length;
    unsigned short ip;
    unsigned short cs;
    unsigned short sp;
    unsigned short ss;
    unsigned short n_segment_tab;
    unsigned short n_mod_ref_tab;
    unsigned short nrname_tab_length;
    unsigned short segment_tab_offset;
    unsigned short resource_tab_offset;
    unsigned short rname_tab_offset;
    unsigned short moduleref_tab_offset;
    unsigned short iname_tab_offset;
    unsigned int  nrname_tab_offset;
    unsigned short n_mov_entry_points;
    unsigned short align_shift_count;
    unsigned short n_resource_seg;
    unsigned char  operating_system;
    unsigned char  additional_flags;
    unsigned short fastload_offset;
    unsigned short fastload_length;
    unsigned short reserved2;
    unsigned short expect_version;
} win_exe_t;

typedef struct {
    unsigned short dfVersion;
    unsigned int  dfSize;
    unsigned char  dfCopyright[60];
    unsigned short dfType;
    unsigned short dfPoints;
    unsigned short dfVertRes;
    unsigned short dfHorizRes;
    unsigned short dfAscent;
    unsigned short dfInternalLeading;
    unsigned short dfExternalLeading;
    unsigned char  dfItalic;
    unsigned char  dfUnderline;
    unsigned char  dfStrikeOut;
    unsigned short dfWeight;
    unsigned char  dfCharSet;
    unsigned short dfPixWidth;
    unsigned short dfPixHeight;
    unsigned char  dfPitchAndFamily;
    unsigned short dfAvgWidth;
    unsigned short dfMaxWidth;
    unsigned char  dfFirstChar;
    unsigned char  dfLastChar;
    unsigned char  dfDefaultChar;
    unsigned char  dfBreakChar;
    unsigned short dfWidthBytes;
    unsigned int  dfDevice;
    unsigned int  dfFace;
    unsigned int  dfBitsPointer;
    unsigned int  dfBitsOffset;
    unsigned char  dfReserved;
    unsigned int  dfFlags;
    unsigned short dfAspace;
    unsigned short dfBspace;
    unsigned short dfCspace;
    unsigned short dfColorPointer;
    unsigned char  dfReserved1[4];
#if 0
    unsigned int  dfColorPointer;
    unsigned char  dfReserved1[16];
#endif
} fntinfo_t;

/*
 * A structure used to load the font info data before transfering to the
 * real font structure.
 */
typedef struct {
    unsigned char dfVersion[2];
    unsigned char dfSize[4];
    unsigned char dfCopyright[60];
    unsigned char dfType[2];
    unsigned char dfPoints[2];
    unsigned char dfVertRes[2];
    unsigned char dfHorizRes[2];
    unsigned char dfAscent[2];
    unsigned char dfInternalLeading[2];
    unsigned char dfExternalLeading[2];
    unsigned char dfItalic[1];
    unsigned char dfUnderline[1];
    unsigned char dfStrikeOut[1];
    unsigned char dfWeight[2];
    unsigned char dfCharSet[1];
    unsigned char dfPixWidth[2];
    unsigned char dfPixHeight[2];
    unsigned char dfPitchAndFamily[1];
    unsigned char dfAvgWidth[2];
    unsigned char dfMaxWidth[2];
    unsigned char dfFirstChar[1];
    unsigned char dfLastChar[1];
    unsigned char dfDefaultChar[1];
    unsigned char dfBreakChar[1];
    unsigned char dfWidthBytes[2];
    unsigned char dfDevice[4];
    unsigned char dfFace[4];
    unsigned char dfBitsPointer[4];
    unsigned char dfBitsOffset[4];
    unsigned char dfReserved[1];
    unsigned char dfFlags[4];
    unsigned char dfAspace[2];
    unsigned char dfBspace[2];
    unsigned char dfCspace[2];
#if 0
    unsigned char dfColorPointer[4];
    unsigned char dfReserved1[16];
#endif
    unsigned char dfColorPointer[2];
    unsigned char dfReserved1[4];
} fishadow_t;

typedef struct {
    unsigned int width;
    unsigned int offset;
} chrinfo_t;

/*
 * Structure used for opening FON/FNT fonts.  Tracks the list of offsets to
 * the font or fonts in the file.
 */
typedef struct _bdffnt_font_t {
    FILE *in;
    unsigned int *fonts;
    unsigned int allocated;
    unsigned int nfonts;
    unsigned int first;

    chrinfo_t *cinfo;
    unsigned int cinfo_used;
    unsigned int cinfo_size;

    fntinfo_t info;
} _bdffnt_font_t;

/**************************************************************************
 *
 * Local macros and variables.
 *
 **************************************************************************/

/*
 * Executable signatures.
 */
#define DOS_SIG 0x5a4d
#define WIN_SIG 0x454e

/*
 * Weight values.
 */
#define BDFFNT_WEIGHT_DONTCARE   0
#define BDFFNT_WEIGHT_THIN       100
#define BDFFNT_WEIGHT_EXTRALIGHT 200
#define BDFFNT_WEIGHT_ULTRALIGHT 200
#define BDFFNT_WEIGHT_LIGHT      300
#define BDFFNT_WEIGHT_NORMAL     400
#define BDFFNT_WEIGHT_REGULAR    400
#define BDFFNT_WEIGHT_MEDIUM     500
#define BDFFNT_WEIGHT_SEMIBOLD   600
#define BDFFNT_WEIGHT_DEMIBOLD   600
#define BDFFNT_WEIGHT_BOLD       700
#define BDFFNT_WEIGHT_EXTRABOLD  800
#define BDFFNT_WEIGHT_ULTRABOLD  800
#define BDFFNT_WEIGHT_HEAVY      900
#define BDFFNT_WEIGHT_BLACK      900

/*
 * Local structures to hold header info.
 */
static dos_exe_t dos;
static win_exe_t win;

/**************************************************************************
 *
 * Support functions.
 *
 **************************************************************************/

static void
_bdffnt_endian_shorts(unsigned short *sp, unsigned int n)
{
    for (; n > 0; n--, sp++)
      *sp = ((*sp >> 8) & 0xff) |
          (((*sp & 0xff) << 8) & 0xff00);
}

static void
_bdffnt_endian_ints(unsigned int *lp, unsigned int n)
{
    for (; n > 0; n--, lp++)
      *lp = (((*lp & 0xff) << 24) & 0xff000000) |
          (((*lp >> 8) << 16) & 0xff0000) |
          (((*lp >> 16) << 8) & 0xff00) |
          ((*lp >> 24) & 0xff);
}

static unsigned short
_bdffnt_get_short(unsigned char *field)
{
    int a = 0, b = 1;

    return (field[a] & 0xff) | ((field[b] & 0xff) << 8);
}

static unsigned int
_bdffnt_get_int(unsigned char *field)
{
    int a = 0, b = 1, c = 2, d = 3;

    return (field[a] & 0xff) | ((field[b] & 0xff) << 8) |
        ((field[c] & 0xff) << 16) | ((field[d] & 0xff) << 24);
}

/*
 * This routine is called when the font header needs some fields adjusted for
 * the endianess of the machine.
 */
static void
_bdffnt_transfer_fntinfo(fntinfo_t *fi, fishadow_t *fis)
{
    fi->dfVersion = _bdffnt_get_short(fis->dfVersion);
    (void) memcpy(fi->dfCopyright, fis->dfCopyright, 60);
    fi->dfSize = _bdffnt_get_int(fis->dfSize);
    fi->dfType = _bdffnt_get_short(fis->dfType);
    fi->dfPoints = _bdffnt_get_short(fis->dfPoints);
    fi->dfVertRes = _bdffnt_get_short(fis->dfVertRes);
    fi->dfHorizRes = _bdffnt_get_short(fis->dfHorizRes);
    fi->dfAscent = _bdffnt_get_short(fis->dfAscent);
    fi->dfInternalLeading = _bdffnt_get_short(fis->dfInternalLeading);
    fi->dfExternalLeading = _bdffnt_get_short(fis->dfExternalLeading);
    fi->dfItalic = fis->dfItalic[0];
    fi->dfUnderline = fis->dfUnderline[0];
    fi->dfStrikeOut = fis->dfStrikeOut[0];
    fi->dfWeight = _bdffnt_get_short(fis->dfWeight);
    fi->dfCharSet = fis->dfCharSet[0];
    fi->dfPixWidth = _bdffnt_get_short(fis->dfPixWidth);
    fi->dfPixHeight = _bdffnt_get_short(fis->dfPixHeight);
    fi->dfPitchAndFamily = fis->dfPitchAndFamily[0];
    fi->dfAvgWidth = _bdffnt_get_short(fis->dfAvgWidth);
    fi->dfMaxWidth = _bdffnt_get_short(fis->dfMaxWidth);
    fi->dfFirstChar = fis->dfFirstChar[0];
    fi->dfLastChar = fis->dfLastChar[0];
    fi->dfDefaultChar = fis->dfDefaultChar[0];
    fi->dfBreakChar = fis->dfBreakChar[0];
    fi->dfWidthBytes = _bdffnt_get_short(fis->dfWidthBytes);
    fi->dfDevice = _bdffnt_get_int(fis->dfDevice);
    fi->dfFace = _bdffnt_get_int(fis->dfFace);
    fi->dfBitsPointer = _bdffnt_get_int(fis->dfBitsPointer);
    fi->dfBitsOffset = _bdffnt_get_int(fis->dfBitsOffset);
    fi->dfReserved = fis->dfReserved[0];
    fi->dfFlags = _bdffnt_get_int(fis->dfFlags);
    fi->dfAspace = _bdffnt_get_short(fis->dfAspace);
    fi->dfBspace = _bdffnt_get_short(fis->dfBspace);
    fi->dfCspace = _bdffnt_get_short(fis->dfCspace);
#if 0
    fi->dfColorPointer = _bdffnt_get_int(fis->dfColorPointer);
    (void) memcpy(fi->dfReserved1, fis->dfReserved1, 16);
#endif
    fi->dfColorPointer = _bdffnt_get_short(fis->dfColorPointer);
    (void) memcpy(fi->dfReserved1, fis->dfReserved1, 4);
}

static char *
_bdffnt_weight_name(unsigned short weight, int *len)
{
    char *name;

    if (weight == 0) {
        name = "Medium";
        *len = 6;
    } else if (weight <= BDFFNT_WEIGHT_THIN) {
        name = "Thin";
        *len = 4;
    } else if (weight <= BDFFNT_WEIGHT_ULTRALIGHT) {
        name = "UltraLight";
        *len = 10;
    } else if (weight <= BDFFNT_WEIGHT_LIGHT) {
        name = "Light";
        *len = 5;
    } else if (weight <= BDFFNT_WEIGHT_MEDIUM) {
        name = "Medium";
        *len = 6;
    } else if (weight <= BDFFNT_WEIGHT_DEMIBOLD) {
        name = "DemiBold";
        *len = 8;
    } else if (weight <= BDFFNT_WEIGHT_BOLD) {
        name = "Bold";
        *len = 4;
    } else if (weight <= BDFFNT_WEIGHT_ULTRABOLD) {
        name = "UltraBold";
        *len = 9;
    } else {
        name = "Black";
        *len = 5;
    }
    return name;
}

static char *
_bdffnt_cset_name(int cset, int *enc)
{
    *enc = 0;
    switch (cset) {
      case 0: *enc = 1; return "ISO8859";
      case 1: return "WinDefault";
      case 2: return "Symbol";
      case 128: return "JISX0208.1983";
      case 129: return "MSHangul";
      case 134: return "GB2312.1980";
      case 136: return "Big5";
      case 161: *enc = 1; return "CP1253";
      case 162: *enc = 1; return "CP1254";
      case 177: *enc = 1; return "CP1255";
      case 178: *enc = 1; return "CP1256";
      case 186: *enc = 1; return "CP1257";
      case 204: *enc = 1; return "CP1251";
      case 238: *enc = 1; return "CP1250";
      case 255: return "OEM";
    }
    return "Unknown";
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

int
bdffnt_open_font(char *path, bdffnt_font_t *fnt)
{
    unsigned short sshift, version;
    int i;
    unsigned int off;
    FILE *in;
    _bdffnt_font_t *f;
    res_typeinfo_t rtype;
    res_nameinfo_t ninfo;

    if (path == 0 || *path == 0 || fnt == 0)
      return 0;

    if ((in = fopen(path, "r")) == 0)
      return -1;

    *fnt = 0;
    f = (_bdffnt_font_t *) malloc(sizeof(_bdffnt_font_t));
    (void) memset((char *) f, 0, sizeof(_bdffnt_font_t));

    f->in = in;

    if (fread((char *) &dos, 1, sizeof(dos_exe_t), in) != sizeof(dos_exe_t)) {
        fclose(in);
        free((char *) f);
        return -1;
    }

    /*
     * Endian everything if on a big-endian machine.
     */
    if (!bdf_little_endian())
      _bdffnt_endian_shorts((unsigned short *) &dos,
                            sizeof(dos_exe_t) / sizeof(unsigned short));

    /*
     * Check for exe signatures.
     */
    if (dos.e_magic == DOS_SIG) {
        fseek(in, dos.e_lfanew, 0L);
        if (fread((char *) &win, 1, sizeof(win_exe_t), in) !=
            sizeof(win_exe_t)) {
            fclose(in);
            free((char *) f);
            return -1;
        }

        /*
         * Only endian the fields used.
         */
        if (!bdf_little_endian()) {
            _bdffnt_endian_shorts(&win.ne_magic, 1);
            _bdffnt_endian_shorts(&win.resource_tab_offset, 1);
            _bdffnt_endian_shorts(&win.rname_tab_offset, 1);
        }

        /*
         * This means the file is either NT 32-bit or something else.
         */
        if (win.ne_magic != WIN_SIG) {
            fclose(in);
            free((char *) f);
            return -1;
        }

        /*
         * Seek to the beginning of the resources.
         */
        off = dos.e_lfanew + win.resource_tab_offset;
        fseek(in, off, 0L);
        fread((char *) &sshift, 1, sizeof(unsigned short), in);
        if (!bdf_little_endian())
          _bdffnt_endian_shorts(&sshift, 1);

        /*
         * Search the resources for all the font resources.
         */
        if (fread((char *) &rtype, 1, sizeof(res_typeinfo_t), in) !=
            sizeof(res_typeinfo_t)) {
            fclose(in);
            free((char *) f);
            return -1;
        }
        while (rtype.id != 0) {
            /*
             * Change the endian order of the first two fields if necessary.
             */
            if (!bdf_little_endian())
              _bdffnt_endian_shorts((unsigned short *) &rtype, 2);

            if (rtype.id == 0x8008)
              break;

            /*
             * Seek to the next resource entry and read it.
             */
            off = rtype.count * sizeof(res_nameinfo_t);
            fseek(in, off, 1L);

            if (fread((char *) &rtype, 1, sizeof(res_typeinfo_t), in) !=
                sizeof(res_typeinfo_t)) {
                fclose(in);
                free((char *) f);
                return -1;
            }
        }
        if (rtype.id == 0x8008) {
            /*
             * Found a font resource, cycle through the entries.
             */
            for (i = 0; i < rtype.count; i++) {
                if (fread((char *) &ninfo, 1, sizeof(res_nameinfo_t), in) !=
                    sizeof(res_nameinfo_t)) {
                    fclose(in);
                    if (f->allocated > 0)
                      free((char *) f->fonts);
                    free((char *) f);
                    return -1;
                }

                if (!bdf_little_endian())
                  _bdffnt_endian_shorts((unsigned short *) &ninfo,
                                        sizeof(res_nameinfo_t) >> 1);

                /*
                 * Check to make sure that the indicated offset is really a
                 * valid font.
                 */
                off = ftell(in);
                fseek(in, (ninfo.offset << sshift), 0L);
                fread((char *) &version, sizeof(unsigned short), 1, in);
                fseek(in, off, 0L);
                if (!bdf_little_endian())
                  _bdffnt_endian_shorts(&version, 1);
                if (version != 0x200 && version != 0x300)
                  continue;

                if (f->nfonts == 0)
                  f->first = ninfo.offset << sshift;
                else {
                    if (f->nfonts >= f->allocated) {
                        if (f->allocated == 0)
                          f->fonts = (unsigned int *)
                              malloc(sizeof(unsigned int) << 3);
                        else
                          f->fonts = (unsigned int *)
                              realloc((char *) f->fonts,
                                      sizeof(unsigned int) *
                                      (f->allocated + 8));
                        f->allocated += 8;
                    }
                    f->fonts[0] = f->first;
                    f->fonts[f->nfonts] = ninfo.offset << sshift;
                }
                f->nfonts++;
            }
        }
    } else if (dos.e_magic == 0x200 || dos.e_magic == 0x300) {
        /*
         * Probably have a .FNT file.
         */
        f->first = ftell(in);
        f->nfonts = 1;
    } else
      return -1;

    if (f->nfonts == 0) {
        /*
         * If no fonts were loaded, free everything up.
         */
        free((char *) f);
        return -1;
    }
    *fnt = f;
    return 1;
}

void
bdffnt_close_font(bdffnt_font_t font)
{
    if (font == 0)
      return;

    fclose(font->in);
    if (font->cinfo_size > 0)
      free((char *) font->cinfo);
    if (font->allocated > 0)
      free((char *) font->fonts);
    free((char *) font);
}

int
bdffnt_font_count(bdffnt_font_t font)
{
    return (font != 0) ? font->nfonts : 0;
}

int
bdffnt_get_copyright(bdffnt_font_t font, unsigned int fontID,
                     unsigned char *string)
{
    int off;
    unsigned char *sp;
    fishadow_t fi;

    off = (font->nfonts == 1) ? font->first : font->fonts[fontID];
    fseek(font->in, off, 0L);

    if (fread((char *) &fi, 1, sizeof(fishadow_t), font->in) !=
        sizeof(fishadow_t))
      return -1;

    for (sp = fi.dfCopyright; (*string = *sp); sp++, string++) ;
    return sp - fi.dfCopyright;
}

int
bdffnt_get_facename(bdffnt_font_t font, unsigned int fontID, int for_xlfd,
                    unsigned char *string)
{
    int wlen, c;
    int off;
    unsigned char *sp, *wname;
    fishadow_t fi;

    if (font == 0 || fontID >= font->nfonts || string == 0)
      return 0;

    off = (font->nfonts == 1) ? font->first : font->fonts[fontID];
    fseek(font->in, off, 0L);

    if (fread((char *) &fi, 1, sizeof(fishadow_t), font->in) !=
        sizeof(fishadow_t))
      return -1;

    _bdffnt_transfer_fntinfo(&font->info, &fi);

    /*
     * Seek to the location of the typeface name.
     */
    off = off + font->info.dfFace;
    fseek(font->in, off, 0L);

    /*
     * Copy the typeface name into the parameter.
     *
     * stops when: -  == 0  -> end of string
     *             -  <  0  -> end of file
     */
    sp = string;
    while ((c = getc(font->in)) > 0) {
        *sp = c;
        if (for_xlfd && *sp == '-')
          *sp = ' ';
        sp++;
    }
    *sp = 0;

    /*
     * If the typeface name is not for an XLFD name, then append the style,
     * weight and point size so the names will be informative.
     */
    if (!for_xlfd) {
        *sp++ = ' ';
        if (font->info.dfItalic & 1) {
            (void) strcpy((char *) sp, "Italic ");
            sp += 7;
        }
        wname = (unsigned char *) _bdffnt_weight_name(font->info.dfWeight,
                                                      &wlen);
        (void) strcpy((char *) sp, (char *) wname);
        sp += wlen;
        *sp++ = ' ';
        sprintf((char *) sp, "%hdpt", font->info.dfPoints);
        sp += strlen((char *) sp);
    }

    return sp - string;
}

int
bdffnt_char_count(bdffnt_font_t font, unsigned int fontID)
{
    int off;
    fishadow_t fi;

    if (font == 0 || fontID >= font->nfonts)
      return 0;

    off = (font->nfonts == 1) ? font->first : font->fonts[fontID];
    fseek(font->in, off, 0L);

    if (fread((char *) &fi, 1, sizeof(fishadow_t), font->in) !=
        sizeof(fishadow_t))
      return -1;

    _bdffnt_transfer_fntinfo(&font->info, &fi);

    return (font->info.dfLastChar - font->info.dfFirstChar) + 1;
}

int
bdffnt_font_pointsize(bdffnt_font_t font, unsigned int fontID)
{
    int off;
    fishadow_t fi;

    if (font == 0 || fontID >= font->nfonts)
      return 0;

    off = (font->nfonts == 1) ? font->first : font->fonts[fontID];
    fseek(font->in, off, 0L);

    if (fread((char *) &fi, 1, sizeof(fishadow_t), font->in) !=
        sizeof(fishadow_t))
      return -1;

    _bdffnt_transfer_fntinfo(&font->info, &fi);

    return font->info.dfPoints;
}

int
bdffnt_load_font(bdffnt_font_t font, unsigned int fontID,
                 bdf_callback_t callback, void *data, bdf_font_t **out)
{
    int x, y, i, nchars;
    unsigned short tmp, bpr;
    int off;
    double swscale;
    chrinfo_t *cp;
    bdf_font_t *f;
    bdf_glyph_t *gp;
    char name[256];
    bdf_property_t prop;
    bdf_callback_struct_t cb;
    fishadow_t fi;

    if (font == 0 || fontID >= font->nfonts || out == 0)
      return 0;

    off = (font->nfonts == 1) ? font->first : font->fonts[fontID];
    fseek(font->in, off, 0L);

    if (fread((char *) &fi, 1, sizeof(fishadow_t), font->in) !=
        sizeof(fishadow_t))
      return -1;

    _bdffnt_transfer_fntinfo(&font->info, &fi);

    /*
     * This cheap hack needed to get to the character info because the FNT
     * docs don't mention that for Win 2.0 fonts, the header was a different
     * size.  This may be the case for a version 3.* as well, but I have no
     * version 3.* fonts to test with.
     */
    fseek(font->in, off + 118, 0L);

    /*
     * Determine how many character info records there are and make sure
     * enough space is allocated in the font structure.
     */
    nchars = (font->info.dfLastChar - font->info.dfFirstChar) + 1;

    if (font->cinfo_size < nchars) {
        if (font->cinfo_size == 0)
          font->cinfo = (chrinfo_t *) malloc(sizeof(chrinfo_t) * nchars);
        else
          font->cinfo = (chrinfo_t *) realloc((char *) font->cinfo,
                                              sizeof(chrinfo_t) * nchars);
        font->cinfo_size = nchars;
    }
    cp = font->cinfo;
    for (i = 0, font->cinfo_used = 0; i < nchars; i++, cp++) {
        fread((char *) &tmp, sizeof(unsigned short), 1, font->in);
        if (!bdf_little_endian())
          _bdffnt_endian_shorts(&tmp, 1);
        cp->width = tmp;
        if (font->info.dfVersion == 0x300) {
            fread((char *) &cp->offset, sizeof(unsigned int), 1, font->in);
            if (!bdf_little_endian())
              _bdffnt_endian_ints(&cp->offset, 1);
        } else {
            fread((char *) &tmp, sizeof(unsigned short), 1, font->in);
            if (!bdf_little_endian())
              _bdffnt_endian_shorts(&tmp, 1);
            cp->offset = tmp;
        }
    }

    /*
     * Create the font.
     */
    f = (bdf_font_t *) malloc(sizeof(bdf_font_t));
    (void) memset((char *) f, 0, sizeof(bdf_font_t));

    /*
     * Set some defaults.
     */
    f->bpp = 1;
    f->default_glyph = font->info.dfDefaultChar + font->info.dfFirstChar;
    f->spacing = (font->info.dfFlags & 1) ? BDF_CHARCELL : BDF_PROPORTIONAL;
    f->glyphs_size = nchars;
    f->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * nchars);
    (void) memset((char *) f->glyphs, 0, sizeof(bdf_glyph_t) * nchars);
    f->point_size = font->info.dfPoints;
    f->resolution_x = font->info.dfHorizRes;
    f->resolution_y = font->info.dfVertRes;
    f->font_ascent = font->info.dfAscent;
    f->font_descent = f->font_ascent - font->info.dfPixHeight;

    /*
     * Set the font bounding box.
     */
    f->bbx.width = font->info.dfMaxWidth;
    f->bbx.height = font->info.dfPixHeight;
    f->bbx.ascent = font->info.dfAscent;
    f->bbx.descent = f->bbx.height - f->bbx.ascent;
    f->bbx.y_offset = -f->bbx.descent;
    f->bbx.x_offset = 0;

    if (f->spacing == BDF_CHARCELL)
      f->monowidth = f->bbx.width;

    /*
     * Determine the SWIDTH scale factor.
     */
    swscale = ((double) f->resolution_y) * ((double) f->point_size);

    /*
     * Call the initial callback if one was provided.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.current = 0;
        cb.total = nchars;
        (*callback)(&cb, data);
    }

    /*
     * Start collecting glyphs.
     */
    for (i = 0, cp = font->cinfo, gp = f->glyphs; i < nchars;
         i++, cp++, gp++) {
        /*
         * Set the glyph encoding.
         */
        gp->encoding = font->info.dfFirstChar + i;

        /*
         * Set the glyph bounding box.
         */
        gp->bbx.width = gp->dwidth = cp->width;
        gp->bbx.height = font->info.dfPixHeight;
        gp->bbx.ascent = font->info.dfAscent;
        gp->bbx.descent = gp->bbx.height - gp->bbx.ascent;
        gp->bbx.y_offset = -gp->bbx.descent;
        gp->bbx.x_offset = 0;
        gp->swidth = (unsigned short)
            (((double) gp->dwidth) * 72000.0) / swscale;

        /*
         * Allocate the glyph bitmap.
         */
        bpr = (cp->width + 7) >> 3;
        gp->bytes = bpr * font->info.dfPixHeight;
        gp->bitmap = (unsigned char *) malloc(gp->bytes);

        /*
         * Seek to the bitmap and read the bytes.
         */
        fseek(font->in, off + cp->offset, 0L);
        if (bpr == 1)
          fread((char *) gp->bitmap, gp->bytes, 1, font->in);
        else {
            /*
             * Typical MS wierdness.  This awkward section is just to get the
             * bytes in the right place.
             */
            for (x = 0; x < bpr; x++) {
                for (y = 0; y < gp->bbx.height; y++)
                  gp->bitmap[(y * bpr) + x] = getc(font->in);
            }
        }

        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.current = i;
            cb.total = nchars;
            (*callback)(&cb, data);
        }
    }

    /*
     * Call the callback one more time to make sure the client knows the
     * load is done.
     */
    if (callback != 0) {
        cb.reason = BDF_LOADING;
        cb.current = nchars;
        cb.total = nchars;
        (*callback)(&cb, data);
    }

    /*
     * Set the number of glyphs used.
     */
    f->glyphs_used = gp - f->glyphs;

    /*
     * Add all the properties.
     */
    prop.name = "FOUNDRY";
    prop.format = BDF_ATOM;
    prop.value.atom = "Windows";
    bdf_add_font_property(f, &prop);

    i = bdffnt_get_facename(font, fontID, 1, (unsigned char *) name);
    prop.name = "FAMILY_NAME";
    prop.format = BDF_ATOM;
    prop.value.atom = name;
    bdf_add_font_property(f, &prop);

    prop.name = "WEIGHT_NAME";
    prop.format = BDF_ATOM;
    prop.value.atom = _bdffnt_weight_name(font->info.dfWeight, &i);
    bdf_add_font_property(f, &prop);

    prop.name = "SLANT";
    prop.format = BDF_ATOM;
    if (font->info.dfItalic & 1)
      prop.value.atom = "I";
    else
      prop.value.atom = "R";
    bdf_add_font_property(f, &prop);

    prop.name = "SETWIDTH_NAME";
    prop.format = BDF_ATOM;
    prop.value.atom = "Normal";
    bdf_add_font_property(f, &prop);

    if (font->info.dfPitchAndFamily & 0xf0) {
        prop.name = "ADDSTYLE_NAME";
        prop.format = BDF_ATOM;
        switch (font->info.dfPitchAndFamily & 0xf0) {
          case 0x20: prop.value.atom = "Swiss"; break;
          case 0x30: prop.value.atom = "Modern"; break;
          case 0x40: prop.value.atom = "Script"; break;
          case 0x50: prop.value.atom = "Decorative"; break;
          default: prop.value.atom = 0;
        }
        if (prop.value.atom != 0)
          bdf_add_font_property(f, &prop);
    }

    prop.name = "PIXEL_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = (int)
        ((((double) (f->point_size * 10) *
           (double) f->resolution_y) / 722.7) + 0.5);
    bdf_add_font_property(f, &prop);

    prop.name = "POINT_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = f->point_size * 10;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_X";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned int) f->resolution_x;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_Y";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned int) f->resolution_y;
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
    prop.value.int32 = font->info.dfAvgWidth * 10;
    bdf_add_font_property(f, &prop);

    prop.name = "SPACING";
    prop.format = BDF_ATOM;
    prop.value.atom = "P";
    switch (f->spacing) {
      case BDF_PROPORTIONAL: prop.value.atom = "P"; break;
      case BDF_MONOWIDTH: prop.value.atom = "M"; break;
      case BDF_CHARCELL: prop.value.atom = "C"; break;
    }
    bdf_add_font_property(f, &prop);

    prop.name = "CHARSET_REGISTRY";
    prop.format = BDF_ATOM;
    prop.value.atom = _bdffnt_cset_name(font->info.dfCharSet, &i);
    bdf_add_font_property(f, &prop);

    sprintf(name, "%d", i);
    prop.name = "CHARSET_ENCODING";
    prop.format = BDF_ATOM;
    prop.value.atom = name;
    bdf_add_font_property(f, &prop);

    /*
     * Generate the XLFD name.
     */
    f->name = bdf_make_xlfd_name(f, 0, 0);

    /*
     * Add messages indicating the font was converted.
     */
    _bdf_add_comment(f, "Font converted from FNT/FON to BDF.", 35);
    _bdf_add_acmsg(f, "Font converted from FNT/FON to BDF.", 35);

    /*
     * Mark the font as being modified.
     */
    f->modified = 1;

    *out = f;

    return 0;
}

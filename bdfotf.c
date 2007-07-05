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
 * Only compile this if the FreeType library is available.
 */
#ifdef HAVE_FREETYPE

#ifndef lint
#ifdef __GNUC__
static char svnid[] __attribute__ ((unused)) = "$Id: bdfotf.c 49 2007-04-12 14:46:40Z mleisher $";
#else
static char svnid[] = "$Id: bdfotf.c 49 2007-04-12 14:46:40Z mleisher $";
#endif
#endif

#include "bdfP.h"
#include FT_GLYPH_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H

#undef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))

#undef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))

/**************************************************************************
 *
 * Local variables.
 *
 **************************************************************************/

static char *platform_names[] = {
    "Apple Unicode", "Macintosh", "ISO", "Microsoft", "Unknown"
};
static int nplatform_names =
sizeof(platform_names) / sizeof(platform_names[0]);

/*
 * Mac encoding names used when creating the BDF XLFD font name and when
 * selecting an encoding from a font.
 */
static char *mac_encodings[] = {
    "MacRoman",    "MacJapanese",   "MacChinese",   "MacKorean",
    "MacArabic",   "MacHebrew",     "MacGreek",     "MacRussian",
    "MacRSymbol",  "MacDevanagari", "MacGurmukhi",  "MacGujarati",
    "MacOriya",    "MacBengali",    "MacTamil",     "MacTelugu",
    "MacKannada",  "MacMalayalam",  "MacSinhalese", "MacBurmese",
    "MacKhmer",    "MacThai",       "MacLaotian",   "MacGeorgian",
    "MacArmenian", "MacMaldivian",  "MacTibetan",   "MacMongolian",
    "MacGeez",     "MacSlavic",     "MacVietnamese","MacSindhi",
    "MacUninterp"
};
static int nmac_encodings = sizeof(mac_encodings) / sizeof(mac_encodings[0]);

/*
 * ISO encoding names used when creating the BDF XLFD font name and when
 * selecting an encoding from a font.
 */
static char *iso_encodings[] = {
    "ASCII", "ISO10646", "ISO8859-1"
};
static int niso_encodings = sizeof(iso_encodings) / sizeof(iso_encodings[0]);

/*
 * Microsoft encoding names used when creating the BDF XLFD font name and
 * when selecting an encoding from a font.
 */
static char *ms_encodings[] = {
    "Symbol", "ISO10646", "ShiftJIS", "GB2312.1980", "Big5",
    "KSC5601.1987", "KSC5601.1992"
};
static int nms_encodings = sizeof(ms_encodings) / sizeof(ms_encodings[0]);

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

/*
 * Routine to get the platform name from the platform ID.
 */
char *
bdfotf_platform_name(short pid)
{
    return (pid < nplatform_names) ?
        platform_names[pid] : platform_names[nplatform_names - 1];
}

/*
 * Routine to get the encoding name from the platform and encoding IDs.
 */
char *
bdfotf_encoding_name(short pid, short eid)
{
    int nnames;
    char **names;

    switch (pid) {
      case 0: return "ISO10646";
      case 1:
        nnames = nmac_encodings;
        names = mac_encodings;
        break;
      case 2:
        nnames = niso_encodings;
        names = iso_encodings;
        break;
      case 3:
        nnames = nms_encodings;
        names = ms_encodings;
        break;
      default: return "Unknown";
    }

    return (eid < nnames) ? names[eid] : "Unknown";
}

/*
 * A generic routine to get a name from the TT name table.  This routine
 * always looks for English language names and checks three possibilities:
 * 1. English names with the MS Unicode encoding ID.
 * 2. English names with the MS unknown encoding ID.
 * 3. English names with the Apple Unicode encoding ID.
 *
 * The particular name ID mut be provided (e.g. nameID = 0 for copyright
 * string, nameID = 6 for Postscript name, nameID = 1 for typeface name.
 *
 * If the `dash_to_space' flag is set, all dashes (-) in the name will be
 * replaced with spaces.
 *
 * Returns the number of bytes added.
 */
int
bdfotf_get_english_string(FT_Face face, int nameID, int dash_to_space,
                          char *name)
{
    int j, encid;
    FT_UInt i, nrec;
    FT_SfntName sfntName;
    unsigned char *s;
    unsigned short slen;

    nrec = FT_Get_Sfnt_Name_Count(face);

    for (encid = 1, j = 0; j < 2; j++, encid--) {
        /*
         * Locate one of the MS English font names.
         */
        for (i = 0; i < nrec; i++) {
           FT_Get_Sfnt_Name(face, i, &sfntName);
           if (sfntName.platform_id == 3 &&
               sfntName.encoding_id == encid &&
               sfntName.name_id == nameID &&
               (sfntName.language_id == 0x0409 ||
                sfntName.language_id == 0x0809 ||
                sfntName.language_id == 0x0c09 ||
                sfntName.language_id == 0x1009 ||
                sfntName.language_id == 0x1409 ||
                sfntName.language_id == 0x1809)) {
               s = sfntName.string;
               slen = sfntName.string_len;
               break;
           }
        }

        if (i < nrec) {
            /*
             * Found one of the MS English font names.  The name is by
             * definition encoded in Unicode, so copy every second byte into
             * the `name' parameter, assuming there is enough space.
             */
            for (i = 1; i < slen; i += 2) {
                if (dash_to_space)
                  *name++ = (s[i] != '-') ? s[i] : ' ';
                else if (s[i] == '\r' || s[i] == '\n') {
                    if (s[i] == '\r' && i + 2 < slen && s[i + 2] == '\n')
                      i += 2;
                    *name++ = ' ';
                    *name++ = ' ';
                } else
                  *name++ = s[i];
            }
            *name = 0;
            return (slen >> 1);
        }
    }

    /*
     * No MS English name found, attempt to find an Apple Unicode English
     * name.
     */
    for (i = 0; i < nrec; i++) {
        FT_Get_Sfnt_Name(face, i, &sfntName);
        if (sfntName.platform_id == 0 && sfntName.language_id == 0 &&
            sfntName.name_id == nameID) {
            s = sfntName.string;
            slen = sfntName.string_len;
            break;
        }
    }

    if (i < nrec) {
        /*
         * Found the Apple Unicode English name.  The name is by definition
         * encoded in Unicode, so copy every second byte into the `name'
         * parameter, assuming there is enough space.
         */
        for (i = 1; i < slen; i += 2) {
            if (dash_to_space)
              *name++ = (s[i] != '-') ? s[i] : ' ';
            else if (s[i] == '\r' || s[i] == '\n') {
                if (s[i] == '\r' && i + 2 < slen && s[i + 2] == '\n')
                  i += 2;
                *name++ = ' ';
                *name++ = ' ';
            } else
              *name++ = s[i];
        }
        *name = 0;
        return (slen >> 1);
    }

    return 0;
}

static int
_bdfotf_generate(FT_Face face, int nocmap, bdf_options_t *opts,
                 bdf_callback_t callback, void *data, bdf_font_t *fp)
{
    int ismono;
    int awidth, code, idx;
    short maxrb, maxlb, minlb, y, x;
    short x_off, y_off, maxas, maxds;
    unsigned short upm, bpr, wd, ht, sx, ex, sy, ey;
    unsigned char *bmap;
    bdf_glyph_t *gp;
    double swscale;
    bdf_callback_struct_t cb;
    bdf_property_t prop;

    FT_Size_Metrics imetrics;
    TT_HoriHeader *horizontal = FT_Get_Sfnt_Table(face, ft_sfnt_hhea);

    /*
     * Set the instance resolution and point size.
     */
    FT_Set_Char_Size(face, 0, opts->point_size * 64,
                     opts->resolution_x, opts->resolution_y);
    imetrics = face->size->metrics;

    /*
     * Set up the initialization callback.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.current = 0;
        cb.total = face->num_glyphs;
        (*callback)(&cb, data);
    }

    /*
     * Get the units per em value.
     */
    upm = face->units_per_EM;

    ismono = 1;
    wd = 0xffff;
    awidth = 0;
    maxrb = maxlb = maxas = maxds = 0;
    minlb = 32767;

    /*
     * Calculate the SWIDTH scaling factor.
     */
    swscale = ((double) opts->resolution_y) * ((double) opts->point_size);

    for (code = fp->glyphs_used = 0; code < 0xffff; code++) {
        if (nocmap) {
            /*
             * No cmap is being used, so do each index in turn.
             */
            if (code >= face->num_glyphs)
              break;
            idx = code;
        } else
          idx = FT_Get_Char_Index(face, code);

        if (idx <= 0 ||
            FT_Load_Glyph(face, idx, opts->otf_flags))
          continue;

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO))
          continue;

        /*
         * Increase the amount of storage by 128 every time.
         */
        if (fp->glyphs_used == fp->glyphs_size) {
            fp->glyphs = (bdf_glyph_t *)
                realloc((char *) fp->glyphs,
                        sizeof(bdf_glyph_t) * (fp->glyphs_size + 128));
            gp = fp->glyphs + fp->glyphs_size;
            (void) memset((char *) gp, 0, sizeof(bdf_glyph_t) << 7);
            fp->glyphs_size += 128;
        }

        /*
         * Point at the next glyph.
         */
        gp = fp->glyphs + fp->glyphs_used++;
        gp->encoding = code;
        gp->dwidth = face->glyph->metrics.horiAdvance >> 6;
        gp->swidth = (unsigned short)
            (((double) gp->dwidth) * 72000.0) / swscale;

        /*
         * Determine the actual bounding box of the glyph bitmap.  Do not
         * forget that the glyph is rendered upside down!
         */
        sx = sy = 0xffff;
        ex = ey = 0;
        bmap = face->glyph->bitmap.buffer;
        for (y = 0; y <  face->glyph->bitmap.rows; y++) {
            for (x = 0; x < face->glyph->bitmap.width; x++) {
                if (bmap[x >> 3] & (0x80 >> (x & 7))) {
                    if (x < sx) sx = x;
                    if (x > ex) ex = x;
                    if (y < sy) sy = y;
                    if (y > ey) ey = y;
                }
            }
            bmap += face->glyph->bitmap.pitch;
        }

        /*
         * If the glyph is actually an empty bitmap, set the size to 0 all
         * around.
         */
        if (sx == 0xffff && sy == 0xffff && ex == 0 && ey == 0)
          sx = ex = sy = ey = 0;
        else {
            /*
             * Adjust the end points.
             */
            ex++;
            ey++;
        }

        /*
         * Test to see if the font is going to be monowidth or not by
         * comparing the current glyph width against the last one.
         */
        if (ismono && (ex - sx) + 1 != wd)
          ismono = 0;

        /*
         * Set the initial metrics.
         */
        wd = ex - sx;
        ht = ey - sy;
        x_off = sx + face->glyph->bitmap_left;
        y_off = sy + face->glyph->bitmap_top - face->glyph->bitmap.rows;

        /*
         * Adjust the overall bounding box.
         */
        maxas = MAX(maxas, ht + y_off);
        maxds = MAX(maxds, -y_off);
        maxrb = MAX(maxrb, wd + x_off);
        minlb = MIN(minlb, x_off);
        maxlb = MAX(maxlb, x_off);

        /*
         * Accumulate the average width value.
         */
        awidth += wd;

        /*
         * Set the glyph metrics.
         */
        gp->bbx.width = wd;
        gp->bbx.height = ht;
        gp->bbx.x_offset = x_off;
        gp->bbx.y_offset = y_off;
        gp->bbx.ascent = ht + y_off;
        gp->bbx.descent = -y_off;

        /*
         * Allocate the bitmap for the glyph.
         */
        bpr = (wd + 7) >> 3;
        gp->bytes = bpr * ht;
        gp->bitmap = (unsigned char *) malloc(gp->bytes);
        (void) memset((char *) gp->bitmap, 0, gp->bytes);

        /*
         * Shift the bits into the glyph bitmap.
         */
        bmap = face->glyph->bitmap.buffer + sy * face->glyph->bitmap.pitch;
        for (y = 0; y < ey - sy; y++) {
            for (x = 0; x < ex - sx; x++) {
                if (bmap[(x+sx) >> 3] & (0x80 >> ((x+sx) & 7)))
                  gp->bitmap[(y * bpr) + (x >> 3)] |= (0x80 >> (x & 7));
            }
            bmap += face->glyph->bitmap.pitch;
        }

        /*
         * Call the callback if it was provided.
         */
        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.current = fp->glyphs_used;
            cb.total = face->num_glyphs;
            (*callback)(&cb, data);
        }
    }

    /*
     * Calculate the font average width.
     */
    awidth =
        (int) ((((double) awidth / (double) fp->glyphs_used) + 0.5) * 10.0);

    /*
     * Set the font bounding box.
     */
    fp->bbx.width = maxrb - minlb;
    fp->bbx.height = maxas + maxds;
    fp->bbx.x_offset = minlb;
    fp->bbx.y_offset = -maxds;
    fp->bbx.ascent = maxas;
    fp->bbx.descent = maxds;

    /*
     * Set the font ascent and descent.
     */
    fp->font_ascent =
        (horizontal->Ascender * imetrics.y_ppem) / upm;
    fp->font_descent =
        -((horizontal->Descender * imetrics.y_ppem) / upm);

    /*
     * Determine if the font is monowidth.
     */
    if (ismono) {
        fp->spacing = BDF_MONOWIDTH;
        fp->monowidth = fp->bbx.width;
    }

    /*
     * Add the properties needed for the XLFD name.
     */
    prop.name = "POINT_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = fp->point_size * 10;
    bdf_add_font_property(fp, &prop);

    prop.name = "PIXEL_SIZE";
    prop.format = BDF_INTEGER;
    prop.value.int32 = (int)
        ((((double) (fp->point_size * 10) *
           (double) fp->resolution_y) / 722.7) + 0.5);
    bdf_add_font_property(fp, &prop);

    prop.name = "RESOLUTION_X";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned int) fp->resolution_x;
    bdf_add_font_property(fp, &prop);

    prop.name = "RESOLUTION_Y";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned int) fp->resolution_y;
    bdf_add_font_property(fp, &prop);

    prop.name = "FONT_ASCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = fp->font_ascent;
    bdf_add_font_property(fp, &prop);

    prop.name = "FONT_DESCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = fp->font_descent;
    bdf_add_font_property(fp, &prop);

    prop.name = "AVERAGE_WIDTH";
    prop.format = BDF_INTEGER;
    prop.value.int32 = awidth;
    bdf_add_font_property(fp, &prop);

    prop.name = "SPACING";
    prop.format = BDF_ATOM;
    prop.value.atom = "P";
    switch (fp->spacing) {
      case BDF_PROPORTIONAL: prop.value.atom = "P"; break;
      case BDF_MONOWIDTH: prop.value.atom = "M"; break;
      case BDF_CHARCELL: prop.value.atom = "C"; break;
    }
    bdf_add_font_property(fp, &prop);

    return 1;
}

int
bdfotf_load_font(FT_Face face, short pid, short eid, bdf_options_t *opts,
                 bdf_callback_t callback, void *data, bdf_font_t **font)
{
    int i, nocmap, res, slen;
    bdf_font_t *fp;
    char *np, str[256];
    bdf_property_t prop;
    bdf_callback_struct_t cb;
    TT_OS2 *os2 = FT_Get_Sfnt_Table(face, ft_sfnt_os2);

    /*
     * First get the requested cmap from the font.
     */
    for (nocmap = i = 0; i < face->num_charmaps; i++) {
        if (face->charmaps[i]->platform_id == pid &&
            face->charmaps[i]->encoding_id == eid)
          break;
    }

    /*
     * If the requested cmap was not found, attempt to fall back on the
     * Microsoft Unicode cmap.
     */
    if (i == face->num_charmaps) {
        for (i = 0; i < face->num_charmaps; i++) {
            if (face->charmaps[i]->platform_id == 3 &&
                face->charmaps[i]->encoding_id == 1)
              break;
        }
        if (i == face->num_charmaps) {
            /*
             * No cmap was found.
             */
            nocmap = 1;
            pid = eid = -1;
        } else {
            /*
             * Found the Microsoft Unicode cmap.
             */
            pid = 3;
            eid = 1;
            FT_Set_Charmap(face, face->charmaps[i]);
        }
    } else
      FT_Set_Charmap(face, face->charmaps[i]);

    /*
     * Create the font.
     */
    *font = fp = (bdf_font_t *) malloc(sizeof(bdf_font_t));
    (void) memset((char *) fp, 0, sizeof(bdf_font_t));

    /*
     * Do some initializations by defaulting to proportional spacing and
     * allocate at least the reported number of glyphs so reallocations will
     * be minimal.
     */
    fp->bpp = 1;
    fp->default_glyph = -1;
    fp->spacing = BDF_PROPORTIONAL;
    fp->glyphs_size = face->num_glyphs;
    fp->glyphs = (bdf_glyph_t *)
        malloc(sizeof(bdf_glyph_t) * face->num_glyphs);
    (void) memset((char *) fp->glyphs, 0,
                  sizeof(bdf_glyph_t) * face->num_glyphs);

    /*
     * Set the metrics.
     */
    fp->point_size = opts->point_size;
    fp->resolution_x = opts->resolution_x;
    fp->resolution_y = opts->resolution_y;

    /*
     * Actually generate the font.
     */
    res = _bdfotf_generate(face, nocmap, opts, callback, data, fp);

    /*
     * If the number of glyphs loaded is less than the reported number of
     * glyphs, force a callback if one was provided.
     */
    if (callback != 0 && fp->glyphs_used < face->num_glyphs) {
        cb.reason = BDF_LOADING;
        cb.total = cb.current = face->num_glyphs;
        (*callback)(&cb, data);
    }

    /*
     * If the font did not load successfully, free up the font.
     */
    if (!res) {
        bdf_free_font(fp);
        *font = 0;
    } else {
        /*
         * Add other sundry properties so the XLFD name can be generated.
         */
        prop.name = "FOUNDRY";
        prop.format = BDF_ATOM;
        prop.value.atom = "FreeType";
        bdf_add_font_property(fp, &prop);

        /*
         * Get the typeface name.
         */
        slen = bdfotf_get_english_string(face, BDFOTF_FAMILY_STRING, 1, str);
        prop.name = "FAMILY_NAME";
        prop.format = BDF_ATOM;
        if (slen > 0)
          prop.value.atom = str;
        else
          prop.value.atom = "Unknown";
        bdf_add_font_property(fp, &prop);

        /*
         * Add the CHARSET_REGISTRY and CHARSET_ENCODING properties.
         */
        np = bdfotf_encoding_name(pid, eid);
        if (strcmp(np, "ISO8859-1") == 0) {
            prop.name = "CHARSET_REGISTRY";
            prop.format = BDF_ATOM;
            prop.value.atom = "ISO8859";
            bdf_add_font_property(fp, &prop);
            prop.name = "CHARSET_ENCODING";
            prop.format = BDF_ATOM;
            prop.value.atom = "1";
            bdf_add_font_property(fp, &prop);
        } else if (strcmp(np, "ISO10646") == 0) {
            prop.name = "CHARSET_REGISTRY";
            prop.format = BDF_ATOM;
            prop.value.atom = np;
            bdf_add_font_property(fp, &prop);
            prop.name = "CHARSET_ENCODING";
            prop.format = BDF_ATOM;
            prop.value.atom = "1";
            bdf_add_font_property(fp, &prop);
        } else {
            prop.name = "CHARSET_REGISTRY";
            prop.format = BDF_ATOM;
            prop.value.atom = np;
            bdf_add_font_property(fp, &prop);
            prop.name = "CHARSET_ENCODING";
            prop.format = BDF_ATOM;
            prop.value.atom = "0";
            bdf_add_font_property(fp, &prop);
        }

        /*
         * Determine the weight name.
         */
        prop.name = "WEIGHT_NAME";
        prop.format = BDF_ATOM;
        slen = bdfotf_get_english_string(face, BDFOTF_SUBFAMILY_STRING,
                                         1, str);
        if (strcmp(str, "Regular") == 0)
          prop.value.atom = "Medium";
        else if (os2->fsSelection & 0x20)
          prop.value.atom = "Bold";
        else if (slen > 0)
          prop.value.atom = str;
        else
          prop.value.atom = "Medium";
        bdf_add_font_property(fp, &prop);

        /*
         * Determine the slant name.
         */
        prop.name = "SLANT";
        prop.format = BDF_ATOM;
        if (os2->fsSelection & 0x01)
          prop.value.atom = "I";
        else
          prop.value.atom = "R";
        bdf_add_font_property(fp, &prop);

        /*
         * Add the default SETWIDTH_NAME.
         */
        prop.name = "SETWIDTH_NAME";
        prop.format = BDF_ATOM;
        prop.value.atom = "Normal";
        bdf_add_font_property(fp, &prop);

        /*
         * Create the XLFD font name for the font.
         */
        fp->name = bdf_make_xlfd_name(fp, 0, 0);

        /*
         * Add the COPYRIGHT notice.
         */
        slen = bdfotf_get_english_string(face, BDFOTF_COPYRIGHT_STRING,
                                         0, str);
        if (slen > 0) {
            prop.name = "COPYRIGHT";
            prop.format = BDF_ATOM;
            prop.value.atom = str;
            bdf_add_font_property(fp, &prop);
        }

        /*
         * Add the special _TTF_PSNAME atom with the font Postscript name.
         */
        slen = bdfotf_get_english_string(face, BDFOTF_POSTSCRIPT_STRING,
                                         0, str);
        if (slen > 0) {
            prop.name = "_TTF_PSNAME";
            prop.format = BDF_ATOM;
            prop.value.atom = str;
            bdf_add_font_property(fp, &prop);
        }

        /*
         * Add a message indicating the font was converted.
         */
        _bdf_add_comment(fp, "Font converted from OTF to BDF.", 31);
        _bdf_add_acmsg(fp, "Font converted from OTF to BDF.", 31);

        /*
         * Finally, mark the font as being modified.
         */
        fp->modified = 1;
    }

    return res;
}

#endif /* HAVE_FREETYPE */

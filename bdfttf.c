/*
 * Copyright 2001 Computing Research Labs, New Mexico State University
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
static char rcsid[] __attribute__ ((unused)) = "$Id: bdfttf.c,v 1.7 2001/09/19 21:00:43 mleisher Exp $";
#else
static char rcsid[] = "$Id: bdfttf.c,v 1.7 2001/09/19 21:00:43 mleisher Exp $";
#endif
#endif

#include "bdfP.h"

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
#ifdef __STDC__
bdfttf_platform_name(short pid)
#else
bdfttf_platform_name(pid)
short pid;
#endif
{
    return (pid < nplatform_names) ?
        platform_names[pid] : platform_names[nplatform_names - 1];
}

/*
 * Routine to get the encoding name from the platform and encoding IDs.
 */
char *
#ifdef __STDC__
bdfttf_encoding_name(short pid, short eid)
#else
bdfttf_encoding_name(pid, eid)
short pid, eid;
#endif
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
#ifdef __STDC__
bdfttf_get_english_string(TT_Face face, int nameID, int dash_to_space,
                          char *name)
#else
bdfttf_get_english_string(face, nameID, dash_to_space, name)
TT_Face face;
int nameID, dash_to_space;
char *name;
#endif
{
    int i, j, encid, nrec;
    short nrPlatformID, nrEncodingID, nrLanguageID, nrNameID;
    char *s;
    unsigned short slen;

    nrec = TT_Get_Name_Count(face);

    for (encid = 1, j = 0; j < 2; j++, encid--) {
        /*
         * Locate one of the MS English font names.
         */
        for (i = 0; i < nrec; i++) {
            TT_Get_Name_ID(face, i, &nrPlatformID, &nrEncodingID,
                           &nrLanguageID, &nrNameID);
            if (nrPlatformID == 3 &&
                nrEncodingID == encid &&
                nrNameID == nameID &&
                (nrLanguageID == 0x0409 || nrLanguageID == 0x0809 ||
                 nrLanguageID == 0x0c09 || nrLanguageID == 0x1009 ||
                 nrLanguageID == 0x1409 || nrLanguageID == 0x1809)) {
                TT_Get_Name_String(face, i, &s, &slen);
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
        TT_Get_Name_ID(face, i, &nrPlatformID, &nrEncodingID,
                       &nrLanguageID, &nrNameID);
        if (nrPlatformID == 0 && nrLanguageID == 0 &&
            nrNameID == nameID) {
            TT_Get_Name_String(face, i, &s, &slen);
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
#ifdef __STDC__
_bdfttf_generate(TT_Face face, TT_Face_Properties *properties, TT_CharMap cmap,
                 int nocmap, bdf_options_t *opts, bdf_callback_t callback,
                 void *data, bdf_font_t *fp)
#else
_bdfttf_generate(face, properties, cmap, nocmap, opts, callback, data, fp)
TT_Face face;
TT_Face_Properties *properties;
TT_CharMap cmap;
int nocmap;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
bdf_font_t *fp;
#endif
{
    int ismono, load_flags, have_strike;
    long awidth, code, idx;
    short maxrb, maxlb, minlb, minx, maxx, miny, maxy;
    short xoff, yoff, x_off, y_off, maxas, maxds;
    unsigned short upm, bpr, wd, ht, sx, ex, sy, ey;
    unsigned char *bmap;
    bdf_glyph_t *gp;
    double swscale;
    bdf_callback_struct_t cb;
    bdf_property_t prop;
    TT_Instance instance;
    TT_Glyph glyph;
    TT_Big_Glyph_Metrics metrics, *mp;
    TT_Instance_Metrics imetrics;
    TT_Raster_Map raster, *rp;
    TT_SBit_Image *sbit;
    TT_SBit_Strike strike;

    /*
     * Create a new instance.
     */
    if (TT_New_Instance(face, &instance))
      return 0;

    /*
     * Set the instance resolution and point size.
     */
    (void) TT_Set_Instance_Resolutions(instance, opts->resolution_x,
                                       opts->resolution_y);
    (void) TT_Set_Instance_PointSize(instance, opts->point_size);
    (void) TT_Get_Instance_Metrics(instance, &imetrics);

    /*
     * Check to see if this font has embedded bitmaps.
     */
    have_strike = (TT_Get_SBit_Strike(face, instance, &strike)) ? 0 : 1;

    if (have_strike)
      (void) TT_New_SBit_Image(&sbit);
    else if (TT_New_Glyph(face, &glyph)) {
        (void) TT_Done_Instance(instance);
        return 0;
    }

    /*
     * Set up the initialization callback.
     */
    if (callback != 0) {
        cb.reason = BDF_LOAD_START;
        cb.current = 0;
        cb.total = properties->num_Glyphs;
        (*callback)(&cb, data);
    }

    /*
     * Get the units per em value.
     */
    upm = properties->header->Units_Per_EM;

    /*
     * Set the glyph loading flags.
     */
    load_flags = TTLOAD_SCALE_GLYPH;
    if (opts->ttf_hint)
      load_flags |= TTLOAD_HINT_GLYPH;

    ismono = 1;
    wd = 0xffff;
    awidth = 0;
    maxrb = maxlb = maxas = maxds = 0;
    minlb = 32767;

    /*
     * Calculate the SWIDTH scaling factor.
     */
    swscale = ((double) opts->resolution_y) * ((double) opts->point_size);

    minx = (properties->header->xMin * imetrics.x_ppem) / upm;
    miny = (properties->header->yMin * imetrics.y_ppem) / upm;
    maxx = (properties->header->xMax * imetrics.x_ppem) / upm;
    maxy = (properties->header->yMax * imetrics.y_ppem) / upm;

    maxx = (maxx - minx) + 1;
    maxy = (maxy - miny) + 1;

    raster.size = 0;

    for (code = fp->glyphs_used = 0; code < 0xffff; code++) {
        if (nocmap) {
            /*
             * No cmap is being used, so do each index in turn.
             */
            if (code >= properties->num_Glyphs)
              break;
            idx = code;
        } else
          idx = TT_Char_Index(cmap, code);

        if (idx <= 0 ||
            (have_strike && TT_Load_Glyph_Bitmap(face, instance, idx, sbit)) ||
            (!have_strike && TT_Load_Glyph(instance, glyph, idx, load_flags)))
          continue;

        if (have_strike) {
            /*
             * Set the pointers to the raster and metrics structures.
             */
            rp = &sbit->map;
            mp = &sbit->metrics;
            xoff = (63 - mp->bbox.xMin) & -64;
            yoff = (63 - mp->bbox.yMin) & -64;
        } else {
            if (raster.size == 0) {
                raster.flow = TT_Flow_Down;
                raster.width = maxx;
                raster.rows = maxy;
                raster.cols = (maxx + 7) >> 3;
                raster.size = raster.cols * raster.rows;
                raster.bitmap = (void *) malloc(raster.size);
            }

            (void) TT_Get_Glyph_Big_Metrics(glyph, &metrics);

            /*
             * Clear the raster bitmap.
             */
            (void) memset((char *) raster.bitmap, 0, raster.size);

            /*
             * Grid fit to determine the x and y offsets that will force the
             * bitmap to fit into the storage provided.
             */
            xoff = (63 - metrics.bbox.xMin) & -64;
            yoff = (63 - metrics.bbox.yMin) & -64;

            /*
             * If the bitmap cannot be generated, simply continue.
             */
            if (TT_Get_Glyph_Bitmap(glyph, &raster, xoff, yoff))
              continue;

            rp = &raster;
            mp = &metrics;
        }

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
        gp->dwidth = mp->horiAdvance >> 6;
        gp->swidth = (unsigned short)
            (((double) gp->dwidth) * 72000.0) / swscale;

        /*
         * Determine the actual bounding box of the glyph bitmap.  Do not
         * forget that the glyph is rendered upside down!
         */
        sx = sy = 0xffff;
        ex = ey = 0;
        bmap = (unsigned char *) rp->bitmap;
        for (miny = 0; miny < rp->rows; miny++) {
            for (minx = 0; minx < rp->width; minx++) {
                if (bmap[(miny * rp->cols) + (minx >> 3)] &
                    (0x80 >> (minx & 7))) {
                    if (minx < sx) sx = minx;
                    if (minx > ex) ex = minx;
                    if (miny < sy) sy = miny;
                    if (miny > ey) ey = miny;
                }
            }
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
        x_off = sx - (xoff >> 6);
        y_off = -(yoff >> 6);

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
        for (maxy = 0, miny = sy; miny < ey; miny++, maxy++) {
            for (maxx = 0, minx = sx; minx < ex; minx++, maxx++) {
                if (bmap[(miny * rp->cols) + ((minx - sx) >> 3)] &
                    (0x80 >> ((minx - sx) & 7)))
                  gp->bitmap[(maxy * bpr) + (maxx >> 3)] |=
                      (0x80 >> (maxx & 7));
            }
        }

        /*
         * Call the callback if it was provided.
         */
        if (callback != 0) {
            cb.reason = BDF_LOADING;
            cb.current = fp->glyphs_used;
            cb.total = properties->num_Glyphs;
            (*callback)(&cb, data);
        }
    }

    /*
     * Do cleanup.
     */
    if (have_strike)
      TT_Done_SBit_Image(sbit);
    else
      TT_Done_Glyph(glyph);

    TT_Done_Instance(instance);

    if (raster.size > 0)
      free((char *) raster.bitmap);

    /*
     * Calculate the font average width.
     */
    awidth =
        (long) ((((double) awidth / (double) fp->glyphs_used) + 0.5) * 10.0);

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
        (properties->horizontal->Ascender * imetrics.y_ppem) / upm;
    fp->font_descent =
        -((properties->horizontal->Descender * imetrics.y_ppem) / upm);

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
    prop.value.int32 = (long)
        ((((double) (fp->point_size * 10) *
           (double) fp->resolution_y) / 722.7) + 0.5);
    bdf_add_font_property(fp, &prop);

    prop.name = "RESOLUTION_X";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned long) fp->resolution_x;
    bdf_add_font_property(fp, &prop);

    prop.name = "RESOLUTION_Y";
    prop.format = BDF_CARDINAL;
    prop.value.card32 = (unsigned long) fp->resolution_y;
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
#ifdef __STDC__
bdfttf_load_font(TT_Face face, TT_Face_Properties *properties, short pid,
                 short eid, bdf_options_t *opts, bdf_callback_t callback,
                 void *data, bdf_font_t **font)
#else
bdfttf_load_font(face, properties, pid, eid, opts, callback, data, font)
TT_Face face;
TT_Face_Properties *properties;
short pid, eid;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
bdf_font_t **font;
#endif
{
    short p, e;
    int i, nocmap, res, slen;
    bdf_font_t *fp;
    char *np, str[256];
    bdf_property_t prop;
    bdf_callback_struct_t cb;
    TT_CharMap cmap;

    /*
     * First get the requested cmap from the font.
     */
    for (nocmap = i = 0; i < TT_Get_CharMap_Count(face); i++) {
        if (!TT_Get_CharMap_ID(face, i, &p, &e) && p == pid && e == eid)
          break;
    }

    /*
     * If the requested cmap was not found, attempt to fall back on the
     * Microsoft Unicode cmap.
     */
    if (i == TT_Get_CharMap_Count(face)) {
        for (i = 0; i < TT_Get_CharMap_Count(face); i++) {
            if (!TT_Get_CharMap_ID(face, i, &p, &e) && p == 3 && e == 1)
              break;
        }
        if (i == TT_Get_CharMap_Count(face)) {
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
            if (TT_Get_CharMap(face, i, &cmap)) {
                /*
                 * Could not load the Microsoft cmap for some reason.
                 */
                nocmap = 1;
                pid = eid = -1;
            }
        }
    } else if (TT_Get_CharMap(face, i, &cmap)) {
        /*
         * Could not load the requested cmap for some reason.
         */
        nocmap = 1;
        pid = eid = -1;
    }

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
    fp->glyphs_size = properties->num_Glyphs;
    fp->glyphs = (bdf_glyph_t *)
        malloc(sizeof(bdf_glyph_t) * properties->num_Glyphs);
    (void) memset((char *) fp->glyphs, 0,
                  sizeof(bdf_glyph_t) * properties->num_Glyphs);

    /*
     * Set the metrics.
     */
    fp->point_size = opts->point_size;
    fp->resolution_x = opts->resolution_x;
    fp->resolution_y = opts->resolution_y;

    /*
     * Actually generate the font.
     */
    res = _bdfttf_generate(face, properties, cmap, nocmap, opts,
                           callback, data, fp);

    /*
     * If the number of glyphs loaded is less than the reported number of
     * glyphs, force a callback if one was provided.
     */
    if (callback != 0 && fp->glyphs_used < properties->num_Glyphs) {
        cb.reason = BDF_LOADING;
        cb.total = cb.current = properties->num_Glyphs;
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
        slen = bdfttf_get_english_string(face, BDFTTF_FAMILY_STRING, 1, str);
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
        np = bdfttf_encoding_name(pid, eid);
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
        slen = bdfttf_get_english_string(face, BDFTTF_SUBFAMILY_STRING,
                                         1, str);
        if (strcmp(str, "Regular") == 0)
          prop.value.atom = "Medium";
        else if (properties->os2->fsSelection & 0x20)
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
        if (properties->os2->fsSelection & 0x01)
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
        slen = bdfttf_get_english_string(face, BDFTTF_COPYRIGHT_STRING,
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
        slen = bdfttf_get_english_string(face, BDFTTF_POSTSCRIPT_STRING,
                                         0, str);
        if (slen > 0) {
            prop.name = "_TTF_PSNAME";
            prop.format = BDF_ATOM;
            prop.value.atom = str;
            bdf_add_font_property(fp, &prop);
        }

        /*
         * Finally, mark the font as being modified.
         */
        fp->modified = 1;
    }

    return res;
}

#endif /* HAVE_FREETYPE */

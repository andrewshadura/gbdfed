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
#ifndef _h_fontgrid
#define _h_fontgrid

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtksignal.h>
#include "bdfP.h"
#include "gtkcompat.h"

G_BEGIN_DECLS

/*
 * The macros for accessing various parts of the widget class.
 */
#define FONTGRID(o) \
        (G_TYPE_CHECK_INSTANCE_CAST((o), fontgrid_get_type(), Fontgrid))

#define FONTGRID_CLASS(c) \
        (G_TYPE_CHECK_CLASS_CAST((c), fontgrid_get_type(), FontgridClass))

#define IS_FONTGRID(o) G_TYPE_CHECK_INSTANCE_TYPE((o), fontgrid_get_type())

#define IS_FONTGRID_CLASS(c) \
        (G_TYPE_CHECK_CLASS_TYPE((c), fontgrid_get_type()))

#define FONTGRID_GET_CLASS(o) \
        (G_TYPE_INSTANCE_GET_CLASS((o), fontgrid_get_type(), FontgridClass))

typedef struct _Fontgrid      Fontgrid;
typedef struct _FontgridClass FontgridClass;

typedef struct {
    gint32 minpage;
    gint32 maxpage;
    gint32 npage;
    gint32 ppage;

    gint32 pageno;
    gint32 bcode;
    gint32 sel_start;
    gint32 sel_end;

    guint32 selmap[2048];
} FontgridInternalPageInfo;

struct _Fontgrid {
    GtkWidget widget;

    bdf_font_t *font;
    guint base;
    gboolean power2;
    gboolean overwrite;
    gboolean noblanks;
    GtkOrientation orientation;
    guint32 point_size;
    gint32 spacing;
    guint16 *colors;
    gint32 initial_glyph;
    gint32 bpp;
    gint32 hres;
    gint32 vres;

    guint16 cell_rows;
    guint16 cell_cols;
    guint16 border;
    guint16 hmargin;
    guint16 vmargin;

    /*
     * Private variables.
     */
    gboolean init;

    gboolean resizing;
    gboolean from_keyboard;
    gboolean no_sel_callback;

    guint16 label_height;
    guint16 cell_width;
    guint16 cell_height;
    guint16 pagesize;
    gint16 xoff;
    gint16 yoff;

    gboolean unencoded;
    gboolean debug;

    GdkGC *xor_gc;

    GdkPoint *points;
    guint32 points_used;
    guint32 points_size;

    /*
     * For creating RGB glyph images.
     */
    guchar *rgb;
    guint32 rgb_used;
    guint32 rgb_size;

    /*
     * Stuff related to the timer between clicks.
     */
    guint32 last_click;
    guint32 mclick_time;

    /*
     * The count accumulated from pressing number keys.
     */
    guint32 count;

    /*
     * The clipboard used to store selections among other things.
     */
    bdf_glyphlist_t clipboard;

    /*
     * Page information necessary for paging an drawing.
     */
    FontgridInternalPageInfo npage;
    FontgridInternalPageInfo upage;
};

struct _FontgridClass {
    GtkWidgetClass parent_class;

    void (*selection_start)(GtkWidget *, gpointer, gpointer);
    void (*selection_extend)(GtkWidget *, gpointer, gpointer);
    void (*selection_end)(GtkWidget *, gpointer, gpointer);
    void (*page)(GtkWidget *, gpointer, gpointer);
    void (*activate)(GtkWidget *, gpointer, gpointer);
    void (*modified)(GtkWidget *, gpointer, gpointer);
};

/**************************************************************************
 *
 * Structures used for the API.
 *
 **************************************************************************/

typedef struct {
    gint32 previous_page;
    gint32 current_page;
    gint32 next_page;
    gint32 encoded_glyphs;
    gint32 unencoded_glyphs;
    gboolean unencoded_page;
} FontgridPageInfo;

typedef struct {
    gchar *name;
    gchar *comments;
    gchar *messages;
    glong bits_per_pixel;
    glong default_char;
    guint16 monowidth;
    guint16 spacing;
    gulong font_ascent;
    gulong font_descent;
    gulong resolution_x;
    gulong resolution_y;
    bdf_bbx_t bbx;
} FontgridFontInfo;

/*
 * Enum representing the callback reasons.
 */
typedef enum {
    FONTGRID_START_SELECTION = 0,
    FONTGRID_EXTEND_SELECTION,
    FONTGRID_END_SELECTION,
    FONTGRID_ACTIVATE,
    FONTGRID_BASE_CHANGE
} FontgridSelectionReason;

typedef struct {
    FontgridSelectionReason reason;
    gint32 start;
    gint32 end;
    gint base;
    bdf_glyph_t *glyphs;
    guint32 num_glyphs;
    gboolean unencoded;
} FontgridSelectionInfo;

typedef enum {
    FONTGRID_MODIFIED = 0,
    FONTGRID_GLYPH_NAMES_MODIFIED,
    FONTGRID_NAME_MODIFIED,
    FONTGRID_PROPERTIES_MODIFIED,
    FONTGRID_COMMENTS_MODIFIED,
    FONTGRID_DEVICE_WIDTH_MODIFIED,
    FONTGRID_GLYPHS_MODIFIED,
    FONTGRID_GLYPHS_DELETED,
    FONTGRID_GLYPHS_PASTED,
    FONTGRID_FONT_METRICS_MODIFIED,
    FONTGRID_PSF_MAPPINGS_MODIFIED
} FontgridModificationReason;

typedef struct {
    FontgridModificationReason reason;
    gchar *name;
    gint32 start;
    gint32 end;
    gboolean unencoded;
} FontgridModificationInfo;

/**************************************************************************
 *
 * General API
 *
 **************************************************************************/

extern GType fontgrid_get_type(void);
extern GtkWidget *fontgrid_new(const gchar *prop1, ...);
extern GtkWidget *fontgrid_newv(bdf_font_t *font, guint32 pointSize,
                                gint32 spacing, gboolean skipBlankPages,
                                gboolean overwriteMode, gboolean powersOfTwo,
                                guint16 *colorList,
                                gint32 initialGlyph, guint codeBase,
                                GtkOrientation orientation,
                                gint32 bitsPerPixel,
                                gint32 horizontalResolution,
                                gint32 verticalResolution,
                                FontgridPageInfo *initialPageInfo);

/*
 * A routine to force initialization before the widget is realized.  This is
 * needed to get some fields filled in for apps using the widget.
 */
extern void fontgrid_force_init(Fontgrid *);

/*
 * Selection information.
 */
extern gboolean fontgrid_clipboard_empty(Fontgrid *);
extern gboolean fontgrid_has_selection(Fontgrid *, FontgridSelectionInfo *);

/*
 * Getting and setting widget values.
 */
extern bdf_font_t *fontgrid_get_font(Fontgrid *);
extern void fontgrid_set_font(Fontgrid *, bdf_font_t *, gint32);

extern gchar *fontgrid_get_font_messages(Fontgrid *);

extern GtkOrientation fontgrid_get_orientation(Fontgrid *);
extern void fontgrid_set_orientation(Fontgrid *, GtkOrientation);

extern gboolean fontgrid_viewing_unencoded(Fontgrid *);
extern void fontgrid_switch_encoding_view(Fontgrid *);

extern guint fontgrid_get_code_base(Fontgrid *);
extern void fontgrid_set_code_base(Fontgrid *, guint);

extern gchar *fontgrid_get_font_name(Fontgrid *);
extern void fontgrid_set_font_name(Fontgrid *, gchar *);

extern gboolean fontgrid_get_font_modified(Fontgrid *);
extern void fontgrid_set_font_modified(Fontgrid *, gboolean);

extern void fontgrid_set_unicode_glyph_names(Fontgrid *, FILE *);
extern void fontgrid_set_adobe_glyph_names(Fontgrid *, FILE *);
extern void fontgrid_set_code_glyph_names(Fontgrid *, gint);

extern void fontgrid_set_font_property(Fontgrid *, bdf_property_t *);
extern void fontgrid_delete_font_property(Fontgrid *, gchar *);

extern guint32 fontgrid_get_font_comments(Fontgrid *, gchar **);
extern void fontgrid_set_font_comments(Fontgrid *, gchar *);

extern gint fontgrid_get_font_spacing(Fontgrid *);
extern void fontgrid_set_font_spacing(Fontgrid *, gint);

extern guint16 fontgrid_get_font_device_width(Fontgrid *);
extern void fontgrid_set_font_device_width(Fontgrid *, guint16);

extern void fontgrid_get_font_info(Fontgrid *, FontgridFontInfo *);
extern void fontgrid_set_font_info(Fontgrid *, FontgridFontInfo *);

/*
 * Navigation and page information.
 */
extern void fontgrid_goto_page(Fontgrid *fw, gint32 pageno);
extern void fontgrid_goto_code(Fontgrid *, gint32 pageno);
extern void fontgrid_goto_first_page(Fontgrid *fw);
extern void fontgrid_goto_last_page(Fontgrid *fw);
extern void fontgrid_goto_next_page(Fontgrid *fw);
extern void fontgrid_goto_previous_page(Fontgrid *fw);
extern void fontgrid_get_page_info(Fontgrid *fw, FontgridPageInfo *pageinfo);
extern gboolean fontgrid_select_next_glyph(Fontgrid *fw, gint32 code);
extern gboolean fontgrid_select_previous_glyph(Fontgrid *fw, gint32 code);

/*
 * Font name functions.
 */
extern void fontgrid_make_xlfd_font_name(Fontgrid *);
extern void fontgrid_update_font_name_from_properties(Fontgrid *);
extern void fontgrid_update_properties_from_font_name(Fontgrid *);

/*
 * Graphical operations.
 */
extern void fontgrid_translate_glyphs(Fontgrid *fw, gint16 dx, gint16 dy,
                                      gboolean all_glyphs);
extern void fontgrid_rotate_glyphs(Fontgrid *fw, gint16 degrees,
                                   gboolean all_glyphs);
extern void fontgrid_shear_glyphs(Fontgrid *fw, gint16 degrees,
                                  gboolean all_glyphs);
extern void fontgrid_embolden_glyphs(Fontgrid *fw, gboolean all_glyphs);

/*
 * Clipboard operations.  MERGE and OVERLAY are the same operation.
 */
typedef enum {
    FONTGRID_NORMAL_PASTE = 0,
    FONTGRID_INSERT_PASTE,
    FONTGRID_MERGE_PASTE,
    FONTGRID_OVERLAY_PASTE
} FontgridPasteType;

extern void fontgrid_copy_selection(Fontgrid *fw);
extern void fontgrid_cut_selection(Fontgrid *fw);
extern void fontgrid_paste_selection(Fontgrid *fw,
                                     FontgridPasteType paste_type);

/*
 * Metrics, glyph, and PSF mappings updates.
 */
extern void fontgrid_update_metrics(Fontgrid *fw, bdf_metrics_t *metrics);
extern void fontgrid_update_glyph(Fontgrid *fw, bdf_glyph_t *glyph,
                                  gboolean unencoded);

extern void fontgrid_update_psf_mappings(Fontgrid *fw, gint32 encoding,
                                         bdf_psf_unimap_t *mappings);
G_END_DECLS

#endif /* _h_fontgrid */

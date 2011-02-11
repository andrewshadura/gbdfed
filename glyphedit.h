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
#ifndef _h_glyphedit
#define _h_glyphedit

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtksignal.h>
#include "bdfP.h"
#include "gtkcompat.h"

G_BEGIN_DECLS

typedef enum {
    GLYPHEDIT_NONE = 0,
    GLYPHEDIT_SELECT,
    GLYPHEDIT_DRAW,
    GLYPHEDIT_MOVE,
    GLYPHEDIT_COPY,
    GLYPHEDIT_FLIP_HORIZONTAL,
    GLYPHEDIT_FLIP_VERTICAL,
    GLYPHEDIT_SHEAR,
    GLYPHEDIT_ROTATE_LEFT,
    GLYPHEDIT_ROTATE_RIGHT,
    GLYPHEDIT_ROTATE,
    GLYPHEDIT_SHIFT_UP_LEFT,
    GLYPHEDIT_SHIFT_UP,
    GLYPHEDIT_SHIFT_UP_RIGHT,
    GLYPHEDIT_SHIFT_LEFT,
    GLYPHEDIT_SHIFT_RIGHT,
    GLYPHEDIT_SHIFT_DOWN_LEFT,
    GLYPHEDIT_SHIFT_DOWN,
    GLYPHEDIT_SHIFT_DOWN_RIGHT
} GlypheditOperation;

/*
 * The macros for accessing various parts of the widget class.
 */
#define GLYPHEDIT(o) \
        (G_TYPE_CHECK_INSTANCE_CAST((o), glyphedit_get_type(), Glyphedit))

#define GLYPHEDIT_CLASS(c) \
        (G_TYPE_CHECK_CLASS_CAST((c), glyphedit_get_type(), GlypheditClass))

#define IS_GLYPHEDIT(o) G_TYPE_CHECK_INSTANCE_TYPE((o), glyphedit_get_type())

#define IS_GLYPHEDIT_CLASS(c) \
        (G_TYPE_CHECK_CLASS_TYPE((c), glyphedit_get_type()))

#define GLYPHEDIT_GET_CLASS(o) \
        (G_TYPE_INSTANCE_GET_CLASS((o), glyphedit_get_type(), GlypheditClass))

typedef struct _Glyphedit      Glyphedit;
typedef struct _GlypheditClass GlypheditClass;

struct _Glyphedit {
    GtkWidget widget;

    bdf_glyph_grid_t *grid;
    gboolean show_cap_height;
    gboolean show_x_height;

    GdkColor baselineColor;
    GdkColor selectionColor;
    GdkColor cursorColor;

    guint16 *colors;
    /*
     * Buffer for drawing grayscale pixels and color spots.
     */
    guchar *spot;
    guint spot_used;
    guint spot_size;

    gboolean owns_clipboard;

    GlypheditOperation op;
    GlypheditOperation pending_op;

    GdkPoint sel_start;
    GdkPoint sel_end;

    gint last_x;
    gint last_y;

    gint lcolor;
    gint cidx;

    guint16 default_pixel_size;
    guint16 pixel_size;

    guint16 vmargin;
    guint16 hmargin;
    guint16 border;
};

struct _GlypheditClass {
    GtkWidgetClass parent_class;

    /*
     * Cursor.
     */
    GdkCursor *cursor;

    /*
     * GC's used for drawing.
     */
    GdkGC *gridgc;
    GdkGC *bbxgc;
    GdkGC *pixgc;
    GdkGC *cleargc;
    GdkGC *selgc;

    /*
     * Signal handlers.
     */
    void (*glyph_modified)(GtkWidget *, gpointer, gpointer);
    void (*pointer_moved)(GtkWidget *, gpointer, gpointer);
    void (*operation_change)(GtkWidget *, gpointer, gpointer);
    void (*color_change)(GtkWidget *, gpointer, gpointer);
};

/**************************************************************************
 *
 * Structures used for the API.
 *
 **************************************************************************/

/*
 * List of callback reasons.
 */
enum {
    GLYPHEDIT_GLYPH_MODIFIED = 0,
    GLYPHEDIT_POINTER_MOVED,
    GLYPHEDIT_OPERATION_CHANGE,
    GLYPHEDIT_COLOR_CHANGE
};

/*
 * The structure passed back in the signals.
 */
typedef struct {
    gint reason;
    bdf_bitmap_t *image;
    bdf_metrics_t *metrics;
    GlypheditOperation operation;
    gint x;
    gint y;
    gint color;
} GlypheditSignalInfo;

/**************************************************************************
 *
 * General API
 *
 **************************************************************************/

extern GType glyphedit_get_type(void);
extern GtkWidget *glyphedit_new(const gchar *prop1, ...);
extern GtkWidget *glyphedit_newv(bdf_glyph_grid_t *grid,
                                 guint16 default_pixel_size,
                                 gboolean show_x_height,
                                 gboolean show_cap_height,
                                 guint16 *colors);

/*
 * Get the encoding of the current glyph.
 */
extern gint32 glyphedit_get_encoding(Glyphedit *gw);

/*
 * Get the current glyph metrics or the current font metrics.
 */
extern void glyphedit_get_glyph_metrics(Glyphedit *gw, bdf_metrics_t *metrics);
extern void glyphedit_get_font_metrics(Glyphedit *gw, bdf_metrics_t *metrics);

/*
 * Get the PSF Unicode mappings.
 */
extern bdf_psf_unimap_t *glyphedit_get_psf_mappings(Glyphedit *gw);

/*
 * Changes device width, width, and height values from the metrics supplied.
 */
extern void glyphedit_set_metrics(Glyphedit *gw, bdf_metrics_t *metrics);

/*
 * Get the glyph spacing.
 */
extern gint glyphedit_get_spacing(Glyphedit *gw);

/*
 * Changes the font spacing and the mono width if necessary.
 */
extern void glyphedit_set_spacing(Glyphedit *gw, gint spacing,
                                  guint16 monowidth);

/*
 * Get and set the operation.
 */
extern void glyphedit_set_operation(Glyphedit *gw, GlypheditOperation op);
extern GlypheditOperation glyphedit_get_operation(Glyphedit *gw);

extern void glyphedit_set_pixel_size(Glyphedit *gw, guint pixel_size);
extern guint glyphedit_get_pixel_size(Glyphedit *gw);

/*
 * Sets the glyph grid.
 */
extern void glyphedit_set_grid(Glyphedit *gw, bdf_glyph_grid_t *grid);

/*
 * Check to see if the glyph or associated info has been modified.
 */
extern gboolean glyphedit_get_modified(Glyphedit *gw);
extern void glyphedit_set_modified(Glyphedit *gw, gboolean modified);
extern void glyphedit_signal_modified(Glyphedit *gw);

/*
 * Determine if a selection is in progress.
 */
extern gboolean glyphedit_get_selecting(Glyphedit *gw);

/*
 * Check to see if the glyph editor clipboard is empty or not.
 */
extern gboolean glyphedit_clipboard_empty(Glyphedit *gw);

/*
 * Get the glyph image from the editor.
 */
extern void glyphedit_get_image(Glyphedit *gw, bdf_bitmap_t *image);

/*
 * Retrieve the glyph grid.
 */
extern bdf_glyph_grid_t *glyphedit_get_grid(Glyphedit *gw);

/*
 * Get the glyph itself.
 */
extern bdf_glyph_t *glyphedit_get_glyph(Glyphedit *gw, gboolean *unencoded);

/*
 * Show or hide the cap height.
 */
extern void glyphedit_set_show_cap_height(Glyphedit *gw, gboolean show);

/*
 * Show or hide the x height.
 */
extern void glyphedit_set_show_x_height(Glyphedit *gw, gboolean show);

/*
 * Crop the glyph bitmap to get rid of empty rows and columns around the
 * glyph.
 */
extern void glyphedit_crop_glyph(Glyphedit *gw);

/*
 * Shift the bitmap horizontally, vertically or a combination of both.
 */
extern void glyphedit_shift_glyph(Glyphedit *gw, gint16 xcount, gint16 ycount);

/*
 * Rotate the bitmap clockwise (positive count) or counter-clockwise
 * (negative count).
 */
extern void glyphedit_rotate_glyph(Glyphedit *w, gint16 degrees);

/*
 * Shear the bitmap clockwise (positive count) or counter-clockwise
 * (negative count).  Limited to the range of [-20,20] degrees.
 */
extern void glyphedit_shear_glyph(Glyphedit *gw, gint16 degrees);

/*
 * Make the glyph bold.
 */
extern void glyphedit_embolden_glyph(Glyphedit *gw);

/*
 * Flip the bitmap horizontally or vertically.
 */
extern void glyphedit_flip_glyph(Glyphedit *gw, GtkOrientation direction);

/*
 * Change to the draw, select, move, or copy operation.
 */
extern void glyphedit_change_operation(Glyphedit *gw, GlypheditOperation op);

/*
 * Change the current color index.
 */
extern void glyphedit_set_color(Glyphedit *gw, gint idx);

/*
 * Insert a bitmap from some outside source.
 */
extern void glyphedit_insert_bitmap(Glyphedit *gw, bdf_bitmap_t *bitmap);

/*
 * Functions explicitly for importing and exporting XBM bitmaps.
 */
extern int glyphedit_import_xbm(Glyphedit *gw, gchar *filename);
extern int glyphedit_export_xbm(Glyphedit *gw, gchar *filename);

/*
 * Functions dealing with the selection.
 */
extern void glyphedit_copy_selection(Glyphedit *gw);
extern void glyphedit_cut_selection(Glyphedit *gw);
extern void glyphedit_paste_selection(Glyphedit *gw);
extern void glyphedit_select_all(Glyphedit *gw);

G_END_DECLS

#endif /* _h_glyphedit */

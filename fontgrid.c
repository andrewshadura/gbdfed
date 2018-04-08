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

#include "fontgrid.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gdk/gdkkeysyms-compat.h>

#ifdef HAVE_XLIB
#include <gdk/gdkx.h>
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(s) dgettext(GETTEXT_PACKAGE,s)
#else
#define _(s) (s)
#endif

/*
 * Macros that represent the properties used by this type of object.
 */
#define FONTGRID_CLIPBOARD gdk_atom_intern("FONTGRID_CLIPBOARD", FALSE)
#define FONTGRID_GLYPHLIST gdk_atom_intern("FONTGRID_GLYPHLIST", FALSE)

/*
 * Set several defaults.
 */
#define FGRID_MAX_COLS            16
#define FGRID_MAX_ROWS            16
#define FGRID_DEFAULT_COLS        16
#define FGRID_DEFAULT_ROWS        8
#define FGRID_DEFAULT_CELL_HEIGHT 18

/*
 * Enums used for identifying properties.
 */
enum {
    PROP_0 = 0,
    PROP_CODE_BASE,
    PROP_POWER2,
    PROP_ORIENTATION,
    PROP_FONT,
    PROP_POINT_SIZE,
    PROP_SPACING,
    PROP_SKIP_BLANKS,
    PROP_OVERWRITE,
    PROP_COLORS,
    PROP_INITIAL_GLYPH,
    PROP_BPP,
    PROP_HRES,
    PROP_VRES
};

/**************************************************************************
 *
 * Selection macros for toggling & testing glyph selected state.
 *
 **************************************************************************/

/*
 * Macros for dealing with the selected state of glyphs in the font grid.
 */
#define IsSelected(code, map) (map[(code) >> 5] & (1 << ((code) & 31)))
#define Select(code, map) (map[(code) >> 5] |= (1 << ((code) & 31)))
#define Unselect(code, map) (map[(code) >> 5] &= ~(1 << ((code) & 31)))

/**************************************************************************
 *
 * Signals.
 *
 **************************************************************************/

/*
 * Enums that represent the signals these objects send out.
 */
enum {
    SELECTION_START = 0,
    SELECTION_EXTEND,
    SELECTION_END,
    ACTIVATE,
    MODIFIED,
    TURN_TO_PAGE
};

static GtkWidgetClass *parent_class = 0;
static guint fontgrid_signals[TURN_TO_PAGE + 1];

static bdf_glyph_t empty_glyph;

/**************************************************************************
 *
 * Digits for displaying the cell encoding.
 *
 **************************************************************************/

/*
 * Lists of points that describe the encoding digits.
 */
typedef struct {
    GdkPoint *points;
    guint npoints;
} fontgrid_digit;

static GdkPoint digit00[] = {{2, 0}, {1, 1}, {3, 1}, {0, 2}, {4, 2}, {0, 3},
                             {4, 3}, {0, 4}, {4, 4}, {1, 5}, {3, 5}, {2, 6}}; 

static GdkPoint digit01[] = {{2, 0}, {1, 1}, {2, 1}, {0, 2}, {2, 2}, {2, 3},
                             {2, 4}, {2, 5}, {0, 6}, {1, 6}, {2, 6}, {3, 6},
                             {4, 6}};

static GdkPoint digit02[] = {{1, 0}, {2, 0}, {3, 0}, {0, 1}, {4, 1}, {4, 2},
                             {2, 3}, {3, 3}, {1, 4}, {0, 5}, {0, 6}, {1, 6},
                             {2, 6}, {3, 6}, {4, 6}};

static GdkPoint digit03[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {4, 1},
                             {3, 2}, {2, 3},{3, 3}, {4, 4}, {0, 5}, {4, 5},
                             {1, 6}, {2, 6}, {3, 6}};

static GdkPoint digit04[] = {{3, 0}, {2, 1}, {3, 1}, {1, 2}, {3, 2}, {0, 3},
                             {3, 3}, {0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4},
                             {3, 5}, {3, 6}};

static GdkPoint digit05[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {0, 1},
                             {0, 2}, {2, 2}, {3, 2}, {0, 3}, {1, 3}, {4, 3},
                             {4, 4}, {0, 5}, {4, 5}, {1, 6}, {2, 6}, {3, 6}};

static GdkPoint digit06[] = {{2, 0}, {3, 0}, {1, 1}, {0, 2}, {0, 3}, {2, 3},
                             {3, 3}, {0, 4}, {1, 4}, {4, 4}, {0, 5}, {4, 5},
                             {1, 6}, {2, 6}, {3, 6}};

static GdkPoint digit07[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {4, 1},
                             {3, 2}, {3, 3}, {2, 4}, {1, 5}, {1, 6}};

static GdkPoint digit08[] = {{1, 0}, {2, 0}, {3, 0}, {0, 1}, {4, 1}, {0, 2},
                             {4, 2}, {1, 3}, {2, 3}, {3, 3}, {0, 4}, {4, 4},
                             {0, 5}, {4, 5}, {1, 6}, {2, 6}, {3, 6}};

static GdkPoint digit09[] = {{1, 0}, {2, 0}, {3, 0}, {0, 1}, {4, 1}, {0, 2},
                             {3, 2}, {4, 2}, {1, 3}, {2, 3}, {4, 3}, {4, 4},
                             {3, 5}, {1, 6}, {2, 6}};

static GdkPoint digit10[] = {{2, 0}, {1, 1}, {3, 1}, {0, 2}, {4, 2}, {0, 3},
                             {4, 3}, {0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4},
                             {0, 5}, {4, 5}, {0, 6}, {4, 6}};

static GdkPoint digit11[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {1, 1}, {4, 1},
                             {1, 2}, {4, 2}, {1, 3}, {2, 3}, {3, 3}, {1, 4},
                             {4, 4}, {1, 5}, {4, 5}, {0, 6}, {1, 6}, {2, 6},
                             {3, 6}};

static GdkPoint digit12[] = {{1, 0}, {2, 0}, {3, 0}, {0, 1}, {4, 1}, {0, 2},
                             {0, 3}, {0, 4},{0, 5}, {4, 5}, {1, 6}, {2, 6},
                             {3, 6}};

static GdkPoint digit13[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {1, 1}, {4, 1},
                             {1, 2}, {4, 2}, {1, 3}, {4, 3}, {1, 4}, {4, 4},
                             {1, 5}, {4, 5}, {0, 6}, {1, 6}, {2, 6}, {3, 6}};

static GdkPoint digit14[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {0, 1},
                             {0, 2}, {0, 3}, {1, 3}, {2, 3}, {3, 3}, {0, 4},
                             {0, 5}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}};

static GdkPoint digit15[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {0, 1},
                             {0, 2}, {0, 3}, {1, 3}, {2, 3}, {3, 3}, {0, 4},
                             {0, 5}, {0, 6}};
static GdkPoint minus[] = {{1, 3}, {2, 3}, {3, 3}, {4,3}};

static fontgrid_digit digits[] = {
   {digit00, sizeof(digit00)/sizeof(GdkPoint)},
   {digit01, sizeof(digit01)/sizeof(GdkPoint)},
   {digit02, sizeof(digit02)/sizeof(GdkPoint)},
   {digit03, sizeof(digit03)/sizeof(GdkPoint)},
   {digit04, sizeof(digit04)/sizeof(GdkPoint)},
   {digit05, sizeof(digit05)/sizeof(GdkPoint)},
   {digit06, sizeof(digit06)/sizeof(GdkPoint)},
   {digit07, sizeof(digit07)/sizeof(GdkPoint)},
   {digit08, sizeof(digit08)/sizeof(GdkPoint)},
   {digit09, sizeof(digit09)/sizeof(GdkPoint)},
   {digit10, sizeof(digit10)/sizeof(GdkPoint)},
   {digit11, sizeof(digit11)/sizeof(GdkPoint)},
   {digit12, sizeof(digit12)/sizeof(GdkPoint)},
   {digit13, sizeof(digit13)/sizeof(GdkPoint)},
   {digit14, sizeof(digit14)/sizeof(GdkPoint)},
   {digit15, sizeof(digit15)/sizeof(GdkPoint)},
   {minus, sizeof(minus)/sizeof(GdkPoint)},
};

/*
 * This array is used to hold a set of digits that will be displayed.  It
 * provides for a max of 19 points per digit and a max of 6 digits.
 */
static GdkPoint encoding_digits[19*6];

/*
 * Used to determine spacing between digits when displaying.
 */
#define FONTGRID_DIGIT_WIDTH   6
#define FONTGRID_DIGIT_HEIGHT 10

/*
 * A macro for getting the current foreground GC.
 */
#define WIDGET_FG_GC(w) ((w)->style->fg_gc[gtk_widget_get_state(w)])

#define HMARGINS(fw) ((fw)->hmargin << 1)
#define VMARGINS(fw) ((fw)->vmargin << 1)

static void
fontgrid_set_cell_geometry(Fontgrid *fw)
{
    bdf_font_t *font;
    gint lw;

    font = fw->font;

    lw = FONTGRID_DIGIT_WIDTH * 7;

    /*
     * The labels will always be numbers in base 8, 10, or 16, so we are only
     * interested in the max ascent.  Add a 2-pixel margin on top and bottom.
     */
    fw->label_height = FONTGRID_DIGIT_HEIGHT + 4;

    /*
     * We want a minumum padding of 3 pixels on each side of the glyph bitmap
     * in each cell. Thus the addition of 6 to each dimension.
     */
    if (font != 0) {
        fw->cell_width = font->bbx.width + 6;
        fw->cell_height = font->bbx.height + 6;
    } else {
        /*
         * Hard-code a minimum size for NULL fonts. The initial height of
         * an empty cell is 20 to give it a better visual appearance.
         */
        fw->cell_width = lw + 6;
        fw->cell_height = FGRID_DEFAULT_CELL_HEIGHT + 6;
    }

    fw->cell_width = MAX(fw->cell_width, lw);
    fw->cell_height = MAX(fw->cell_height, fw->label_height);

    /*
     * Now add the label size into the picture.
     */
    fw->cell_height += fw->label_height - 1;
}

static void
fontgrid_set_rows_cols(Fontgrid *fw, GtkAllocation *core)
{
    gint i;
    guint16 dw, dh, wd, ht;

    /*
     * Limit the window size to 7/8 of the actual screen dimensions.
     */
    dw = (gdk_screen_width() * 7) >> 3;
    dh = (gdk_screen_height() * 7) >> 3;

    if (!core) {
        /*
         * Adjust the rows and columns based on the preferred geometry.
         */
        wd = (fw->cell_width * fw->cell_cols) + HMARGINS(fw);
        ht = (fw->cell_height * fw->cell_rows) + VMARGINS(fw);

        if (wd > dw)
          fw->cell_cols = (dw - HMARGINS(fw)) / fw->cell_width;

        if (ht > dh)
          fw->cell_rows = (dh - VMARGINS(fw)) / fw->cell_height;
    } else {
        /*
         * Adjust the rows and columns based on the current geometry.
         */
        fw->cell_cols = (core->width - HMARGINS(fw)) / fw->cell_width;
        fw->cell_rows = (core->height - VMARGINS(fw)) / fw->cell_height;
    }

    /*
     * Adjust rows and columns to powers of two if necessary.
     */
    if (fw->power2) {
        /*
         * Make sure the columns are a power of 2.
         */
        for (i = 15; i >= 0; i--) {
            if (fw->cell_cols & (1 << i)) {
                fw->cell_cols = 1 << i;
                break;
            }
        }

        /*
         * Make sure the rows are a power of 2.
         */
        for (i = 15; i >= 0; i--) {
            if (fw->cell_rows & (1 << i)) {
                fw->cell_rows = 1 << i;
                break;
            }
        }
    }

    /*
     * Fall back to a minimum of two rows.
     */
    if (fw->cell_rows == 0)
      fw->cell_rows = 2;

    /*
     * Fall back to a minimum of two columns.
     */
    if (fw->cell_cols == 0)
      fw->cell_cols = 2;

    /*
     * Make sure the number of rows and cols are within the max limits.
     */
    if (fw->cell_cols > FGRID_MAX_COLS)
      fw->cell_cols = FGRID_MAX_COLS;

    if (fw->cell_rows > FGRID_MAX_ROWS)
      fw->cell_rows = FGRID_MAX_ROWS;

    /*
     * Set the new page size based on the calculated rows and columns.
     */
    fw->pagesize = fw->cell_rows * fw->cell_cols;
}

/**************************************************************************
 *
 * GObjectClass functions.
 *
 **************************************************************************/

static void
fontgrid_set_property(GObject *obj, guint prop_id, const GValue *value,
                      GParamSpec *pspec)
{
    Fontgrid *fw;

    fw = FONTGRID(obj);

    switch (prop_id) {
      case PROP_CODE_BASE:
        fw->base = g_value_get_uint(value);
        /*
         * Force the encodings to be redisplayed here?
         */
        break;
      case PROP_POWER2:
        fw->power2 = g_value_get_boolean(value);
        break;
      case PROP_ORIENTATION:
        fontgrid_set_orientation(fw, g_value_get_enum(value));
        break;
      case PROP_FONT:
        /*
         * Need to set the rows and columns back to their defaults when
         * a new font is passed in case it is NULL.
         */
        fw->font = (bdf_font_t *) g_value_get_pointer(value);

        fontgrid_set_cell_geometry(fw);
        fontgrid_set_rows_cols(fw, 0);
        break;
      case PROP_POINT_SIZE:
        fw->point_size = g_value_get_uint(value);
        break;
      case PROP_SPACING:
        fw->spacing = g_value_get_int(value);
        break;
      case PROP_SKIP_BLANKS:
        fw->noblanks = g_value_get_boolean(value);
        break;
      case PROP_OVERWRITE:
        fw->overwrite = g_value_get_boolean(value);
        break;
      case PROP_COLORS:
        fw->colors = (guint16 *) g_value_get_pointer(value);
        break;
      case PROP_INITIAL_GLYPH:
        fw->initial_glyph = g_value_get_int(value);
        break;
      case PROP_BPP:
        fw->bpp = g_value_get_int(value);
        break;
      case PROP_HRES:
        fw->hres = g_value_get_int(value);
        break;
      case PROP_VRES:
        fw->vres = g_value_get_int(value);
        break;
    }
}

static void
fontgrid_get_property(GObject *obj, guint prop_id, GValue *value,
                      GParamSpec *pspec)
{
    Fontgrid *f;

    f = FONTGRID(obj);

    switch (prop_id) {
      case PROP_CODE_BASE:
        g_value_set_uint(value, f->base);
        break;
      case PROP_POWER2:
        g_value_set_boolean(value, f->power2);
        break;
      case PROP_ORIENTATION:
        g_value_set_enum(value, f->orientation);
        break;
      case PROP_FONT:
        g_value_set_pointer(value, f->font);
        break;
      case PROP_POINT_SIZE:
        g_value_set_uint(value, f->point_size);
        break;
      case PROP_SPACING:
        g_value_set_int(value, f->spacing);
        break;
      case PROP_SKIP_BLANKS:
        g_value_set_boolean(value, f->noblanks);
        break;
      case PROP_COLORS:
        g_value_set_pointer(value, f->colors);
        break;
      case PROP_INITIAL_GLYPH:
        g_value_set_int(value, f->initial_glyph);
        break;
      case PROP_BPP:
        g_value_set_int(value, f->bpp);
        break;
      case PROP_HRES:
        g_value_set_int(value, f->hres);
        break;
      case PROP_VRES:
        g_value_set_int(value, f->vres);
        break;
    }
}

/**************************************************************************
 *
 * GtkObjectClass functions.
 *
 **************************************************************************/

static void
#if GTK_CHECK_VERSION(3, 0, 0)
fontgrid_destroy(GtkWidget *obj)
#else
fontgrid_destroy(GtkObject *obj)
#endif
{
    Fontgrid *f;
    guint32 i;
    bdf_glyphlist_t *gl;

    /*
     * Do some checks to make sure the incoming object exists and is the right
     * kind.
     */
    g_return_if_fail(obj != 0);
    g_return_if_fail(IS_FONTGRID(obj));

    f = FONTGRID(obj);

    /*
     * Clean up this object instance.
     */
    if (f->font)
      bdf_free_font(f->font);
    f->font = 0;

#if !GTK_CHECK_VERSION(3, 0, 0)
    if (f->xor_gc != 0)
      g_object_unref(G_OBJECT(f->xor_gc));
    f->xor_gc = 0;
#endif

    if (f->points_size > 0)
      g_free(f->points);
    f->points_size = f->points_used = 0;

    if (f->rgb_size > 0)
      g_free(f->rgb);
    f->rgb_used = f->rgb_size = 0;

    /*
     * Remove all ownership of selections.
     */
    gtk_selection_remove_all(GTK_WIDGET(obj));

    /*
     * Free up the clipboard contents if there are any.
     */
    gl = &f->clipboard;
    for (i = 0; i < gl->glyphs_used; i++) {
        if (gl->glyphs[i].name)
          free(gl->glyphs[i].name);
        if (gl->glyphs[i].bytes > 0)
          free((char *) gl->glyphs[i].bitmap);
    }
    if (gl->glyphs_size > 0)
      free((char *) gl->glyphs);
    gl->glyphs_size = gl->glyphs_used = 0;

    /*
     * Follow the class chain back up to free up resources allocated in the
     * parent classes.
     */
#if GTK_CHECK_VERSION(3, 0, 0)
    GTK_WIDGET_CLASS(parent_class)->destroy(obj);
#else
    GTK_OBJECT_CLASS(parent_class)->destroy(obj);
#endif
}

static void
fontgrid_finalize(GObject *obj)
{
    /*
     * Do some checks to make sure the incoming object exists and is the right
     * kind.
     */
    g_return_if_fail(obj != 0);
    g_return_if_fail(IS_FONTGRID(obj));

    /*
     * Follow the class chain back up to free up resources allocated in the
     * parent classes.
     */
    G_OBJECT_CLASS(parent_class)->finalize(obj);
}

/**************************************************************************
 *
 * GtkWidgetClass functions.
 *
 **************************************************************************/

static void
fontgrid_preferred_size(GtkWidget *widget, GtkRequisition *preferred)
{
    Fontgrid *fw;

    fw = FONTGRID(widget);
    preferred->width = (fw->cell_width * fw->cell_cols) + HMARGINS(fw);
    preferred->height = (fw->cell_height * fw->cell_rows) + VMARGINS(fw);
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void
fontgrid_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
    GtkRequisition requisition;

    fontgrid_preferred_size(widget, &requisition);

    *minimal_width = *natural_width = requisition.width;
}

static void
fontgrid_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
    GtkRequisition requisition;

    fontgrid_preferred_size(widget, &requisition);

    *minimal_height = *natural_height = requisition.height;
}
#endif

static void
fontgrid_actual_size(GtkWidget *widget, GtkAllocation *actual)
{
    Fontgrid *fw;

    fw = FONTGRID(widget);

    gtk_widget_set_allocation(widget, actual);

    /*
     * Make sure the rows and columns are adjusted to fit the actual allocated
     * size.
     */
    fontgrid_set_rows_cols(fw, actual);

    if (gtk_widget_get_realized(widget))
      gdk_window_move_resize(gtk_widget_get_window(widget), actual->x, actual->y,
                             actual->width, actual->height);
}

static void
fontgrid_realize(GtkWidget *widget)
{
    Fontgrid *fw;
    GdkWindowAttr attributes;
#if !GTK_CHECK_VERSION(3, 0, 0)
    GdkGCValues values;
#endif
    gint attributes_mask;
    GtkAllocation all;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_FONTGRID(widget));

    fw = FONTGRID(widget);
    gtk_widget_set_realized(widget, TRUE);

    attributes.window_type = GDK_WINDOW_CHILD;

    gtk_widget_get_allocation(widget, &all);
    attributes.x = all.x;
    attributes.y = all.y;
    attributes.width = all.width;
    attributes.height = all.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
#if !GTK_CHECK_VERSION(3, 0, 0)
    attributes.colormap = gtk_widget_get_colormap(widget);
#endif
    attributes.event_mask = gtk_widget_get_events(widget);
    attributes.event_mask |= (GDK_EXPOSURE_MASK|GDK_BUTTON_PRESS_MASK|
                              GDK_BUTTON_RELEASE_MASK|GDK_ENTER_NOTIFY_MASK|
                              GDK_POINTER_MOTION_MASK|
                              GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|
                              GDK_LEAVE_NOTIFY_MASK|GDK_FOCUS_CHANGE_MASK|
                              GDK_PROPERTY_CHANGE_MASK);

    attributes_mask = GDK_WA_X|GDK_WA_Y|GDK_WA_VISUAL
#if !GTK_CHECK_VERSION(3, 0, 0)
        |GDK_WA_COLORMAP
#endif
    ;

    gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget),
                                    &attributes, attributes_mask));
    gdk_window_set_user_data(gtk_widget_get_window(widget), widget);

    gtk_widget_set_style(widget, gtk_style_attach(gtk_widget_get_style(widget), gtk_widget_get_window(widget)));
    gtk_style_set_background(gtk_widget_get_style(widget), gtk_widget_get_window(widget), GTK_STATE_NORMAL);

#if !GTK_CHECK_VERSION(3, 0, 0)
    if (fw->xor_gc != 0)
      g_object_unref(G_OBJECT(fw->xor_gc));

    /*
     * Create the GC used to display selected cells.
     */
    values.foreground.pixel =
        gtk_widget_get_style(widget)->fg[gtk_widget_get_state(widget)].pixel ^
        gtk_widget_get_style(widget)->bg[gtk_widget_get_state(widget)].pixel;
    (void) memset((char *) &values.background, 0, sizeof(GdkColor));
    values.function = GDK_XOR;
    fw->xor_gc = gdk_gc_new_with_values(gtk_widget_get_window(widget), &values,
                                        GDK_GC_FOREGROUND|
                                        GDK_GC_BACKGROUND|GDK_GC_FUNCTION);
#endif
}

static bdf_glyph_t *
fontgrid_locate_glyph(bdf_glyph_t *glyphs, guint32 nglyphs, gint32 code,
                      gboolean exact_match)
{
    gint32 l, r, m = 0;

    if (code < 0 || glyphs == 0 || nglyphs == 0)
      return 0;

    for (l = 0, r = (gint32) (nglyphs - 1); l <= r; ) {
        m = (l + r) >> 1;
        if (glyphs[m].encoding < code)
          l = m + 1;
        else if (glyphs[m].encoding > code)
          r = m - 1;
        else {
            if (exact_match)
              return glyphs + m;
            break;
        }
    }

    if (exact_match)
      return 0;

    /*
     * Adjust to the beginning or end if nothing was found in the search.
     */
    while (m > 0 && glyphs[m].encoding > code)
      m--;
    while (m < (gint32) nglyphs && glyphs[m].encoding < code)
      m++;

    return (m < (gint32) nglyphs) ? glyphs + m : 0;
}

static void
fontgrid_get_glyph_points(Fontgrid *fw, gint x, gint y, gint rx, gint by,
                          bdf_glyph_t *glyph)
{
    gint i, j, bpr, col;
    unsigned char *bmap, *masks = 0;

    switch (fw->bpp) {
      case 1: masks = bdf_onebpp; break;
      case 2: masks = bdf_twobpp; break;
      case 4: masks = bdf_fourbpp; break;
      case 8: masks = bdf_eightbpp; break;
    }

    fw->points_used = 0;
    bmap = glyph->bitmap;
    bpr = ((glyph->bbx.width * fw->bpp) + 7) >> 3;

    for (i = 0; i + y - glyph->bbx.ascent < by && i < glyph->bbx.height; i++) {
        for (col = j = 0; j + x < rx && j < glyph->bbx.width;
             j++, col += fw->bpp) {
            if (bmap[(i * bpr) + (col >> 3)] &
                masks[(col & 7) / fw->bpp]) {
                if (fw->points_used == fw->points_size) {
                    if (fw->points_size == 0)
                      fw->points =
                          (GdkPoint *) g_malloc(sizeof(GdkPoint) << 7);
                    else
                      fw->points =
                          (GdkPoint *) g_realloc(fw->points,
                                                 sizeof(GdkPoint) *
                                                 (fw->points_size + 128));
                    fw->points_size += 128;
                }

                fw->points[fw->points_used].x = j + x;
                fw->points[fw->points_used].y = i + y - glyph->bbx.ascent;
                fw->points_used++;
            }
        }
    }
}

#if 0
static void
fontgrid_get_glyph_points_color(Fontgrid *fw, gint x, gint y, gint rx, gint by,
                                gint color_index, bdf_glyph_t *glyph)
{
    gint i, j, bpr, col, byte, di = 0, si, cidx = 0;
    unsigned char *bmap, *masks = 0;

    switch (fw->bpp) {
      case 1: masks = bdf_onebpp; di = 7; break;
      case 2: masks = bdf_twobpp; di = 3; break;
      case 4: masks = bdf_fourbpp; di = 1; break;
      case 8: masks = bdf_eightbpp; di = 0; break;
    }

    fw->points_used = 0;
    bmap = glyph->bitmap;
    bpr = ((glyph->bbx.width * fw->bpp) + 7) >> 3;

    for (i = 0; i + y - glyph->bbx.ascent < by && i < glyph->bbx.height; i++) {
        for (col = j = 0; j + x < rx && j < glyph->bbx.width;
             j++, col += fw->bpp) {
            si = (col & 7) / fw->bpp;
            byte = bmap[(i * bpr) + (col >> 3)] & masks[si];
            if (byte) {
                /*
                 * Check to see if the byte matches the color index being
                 * collected.
                 */
                if (di > si)
                  byte >>= (di - si) * fw->bpp;

                if (byte == cidx) {
                    if (fw->points_used == fw->points_size) {
                        if (fw->points_size == 0)
                          fw->points = (GdkPoint *)
                              g_malloc(sizeof(GdkPoint) << 6);
                        else
                          fw->points = (GdkPoint *)
                              g_realloc(fw->points, sizeof(GdkPoint) *
                                        (fw->points_size + 64));
                        fw->points_size += 64;
                    }

                    fw->points[fw->points_used].x = j + x;
                    fw->points[fw->points_used].y = i + y - glyph->bbx.ascent;
                    fw->points_used++;
                }
            }
        }
    }
}
#endif

/*
 * This routine creates a 24 bits per pixel image of a glyph so it can be
 * drawn using GtkRGB. This is less complicated than the old method of
 * collecting and drawing individual pixels of each different color.
 */
static void
fontgrid_make_rgb_image(Fontgrid *fw, bdf_glyph_t *glyph)
{
    GtkWidget *w = GTK_WIDGET(fw);
    gint x, y, bpr, rgb_bpr, col, byte, di = 0, si;
    guchar bg[4], pix[4], *bmap, *masks = 0;

    /*
     * Figure out the background color.
     */
    bg[0] = (guchar) gtk_widget_get_style(w)->bg[gtk_widget_get_state(w)].red;
    bg[1] = (guchar) gtk_widget_get_style(w)->bg[gtk_widget_get_state(w)].green;
    bg[2] = (guchar) gtk_widget_get_style(w)->bg[gtk_widget_get_state(w)].blue;

    switch (fw->bpp) {
      case 1: masks = bdf_onebpp; di = 7; break;
      case 2: masks = bdf_twobpp; di = 3; break;
      case 4: masks = bdf_fourbpp; di = 1; break;
      case 8: masks = bdf_eightbpp; di = 0; break;
    }

    bmap = glyph->bitmap;
    bpr = ((glyph->bbx.width * fw->bpp) + 7) >> 3;

    rgb_bpr = glyph->bbx.width * 3;
    fw->rgb_used = rgb_bpr * glyph->bbx.height;

    if (fw->rgb_size < fw->rgb_used) {
        if (fw->rgb_size == 0)
          fw->rgb = g_malloc(fw->rgb_used);
        else
          fw->rgb = g_realloc(fw->rgb, fw->rgb_used);
        fw->rgb_size = fw->rgb_used;
    }

    for (y = 0; y < glyph->bbx.height; y++) {
        for (col = x = 0; x < glyph->bbx.width; x++, col += fw->bpp) {
            si = (col & 7) / fw->bpp;

            byte = bmap[(y * bpr) + (col >> 3)] & masks[si];
            if (di > si)
              byte >>= (di - si) * fw->bpp;
            if (byte) {
                /*
                 * Look up the color.
                 */
                switch (fw->bpp) {
                  case 1: memset(pix, 0, 3); break;
                  case 2: memset(pix, fw->colors[byte-1], 3); break;
                  case 4: memset(pix, fw->colors[byte-1+4], 3); break;
                  case 8: memset(pix, byte, 3); break;
                }
            } else
              /*
               * Set the pixel to the background color.
               */
              memcpy(pix, bg, 3);

            memcpy(&fw->rgb[(y * rgb_bpr) + (x * 3)], pix, 3);
        }
    }
}

#if !GTK_CHECK_VERSION(3, 0, 0)
static void
fontgrid_draw_encoding(GtkWidget *w, GdkGC *gc, gint x, gint y, gchar *num,
                       gint numlen)
{
    gint i, j, d;
    GdkPoint *dp;

    if (!gtk_widget_get_realized(w))
      return;

    dp = encoding_digits;
    for (i = 0; i < numlen; i++) {
        if (num[i] == '-') 
          d = 16;
        else if (num[i] <= '9')
          d = num[i] - '0';
        else
          d = (num[i] - 'A') + 10;

        /*
         * Copy the next digit into the display array.
         */
        (void) memcpy((char *) dp, (char *) digits[d].points,
                      sizeof(GdkPoint) * digits[d].npoints);
        /*
         * Position the points.
         */
        for (j = 0; j < digits[d].npoints; j++) {
            dp[j].x += x;
            dp[j].y += y;
        }
        dp += digits[d].npoints;
        x += 6;
    }

    /*
     * Draw the points.
     */
    gdk_draw_points(gtk_widget_get_window(w), gc, encoding_digits, dp - encoding_digits);
}

static void
fontgrid_draw_cells(GtkWidget *widget, gint32 start, gint32 end,
                    gboolean labels, gboolean glyphs)
{
    Fontgrid *fw;
    gint x, y, wd, as, ds, len, lx, ly;
    gint32 i, n, r, c;
    guint32 nglyphs, ng;
    gboolean mod;
    bdf_font_t *font;
    bdf_glyph_t *glyph, *gp;
    FontgridInternalPageInfo *pi;
    GdkGC *gc;
    GdkRectangle rect;
    gchar nbuf[16];

    if (!gtk_widget_get_realized(widget) || (labels == FALSE && glyphs == FALSE))
      return;

    fw = FONTGRID(widget);

    font = fw->font;

    glyph = 0;
    nglyphs = 0;

    if (!fw->unencoded) {
        pi = &fw->npage;
        if (font) {
            glyph = font->glyphs;
            nglyphs = font->glyphs_used;
        }
    } else {
        /*
         * When viewing the unencoded glyph pages, all glyphs are labelled
         * with an encoding of -1.
         */
        strcpy(nbuf, "-1");

        pi = &fw->upage;
        if (font) {
            glyph = font->unencoded;
            nglyphs = font->unencoded_used;
        }
    }

    /*
     * The initial code to work from.
     */
    n = pi->bcode;

    /*
     * Locate the glyph closest to the starting code.
     */
    if ((glyph = fontgrid_locate_glyph(glyph, nglyphs, start, FALSE)) == 0)
      nglyphs = 0;

    gp = glyph;

    gc = gtk_widget_get_style(widget)->fg_gc[gtk_widget_get_state(widget)];

    for (ng = 0, i = start; i <= end; i++) {
        /*
         * Only draw those cells that are on the current page.
         */
        if (i < pi->bcode || i >= pi->bcode + fw->pagesize)
          continue;

        if (fw->orientation == GTK_ORIENTATION_HORIZONTAL) {
            r = (i - n) / fw->cell_cols;
            c = (i - n) % fw->cell_cols;
        } else {
            c = (i - n) / fw->cell_rows;
            r = (i - n) % fw->cell_rows;
        }

        x = fw->xoff + (c * fw->cell_width);
        y = fw->yoff + (r * fw->cell_height);

        if (labels) {
            if (!fw->unencoded) {
                switch (fw->base) {
                  case 8: sprintf(nbuf, "%o", i); break;
                  case 10: sprintf(nbuf, "%d", i); break;
                  case 16: sprintf(nbuf, "%X", i); break;
                }
            }
            rect.x = x + 1;
            rect.y = y + 1;
            rect.width = fw->cell_width - 2;
            rect.height = fw->label_height - 2;
            gdk_draw_rectangle(gtk_widget_get_window(widget), gc, FALSE,
                               rect.x, rect.y, rect.width, rect.height);

            len = strlen(nbuf);
            wd = len * 6;
            as = 8;
            ds = 0;

            lx = (x + ((fw->cell_width >> 1) - (wd >> 1))) + 1;
            ly = (y + ((fw->label_height >> 1) - ((as + ds) >> 1))) + 1;

            mod = FALSE;
            if (i <= 0xffff)
              mod = (!fw->unencoded) ? bdf_glyph_modified(font, i, 0) :
                bdf_glyph_modified(font, i, 1);

            gdk_window_clear_area(gtk_widget_get_window(widget), rect.x + 1, rect.y + 1,
                                  rect.width - 1, rect.height - 1);

            if (!fw->unencoded && mod) {
                gdk_draw_rectangle(gtk_widget_get_window(widget), gc, TRUE,
                                   rect.x + 2, rect.y + 2,
                                   rect.width - 3, rect.height - 3);
                fontgrid_draw_encoding(widget, fw->xor_gc, lx, ly, nbuf, len);
                if (gp && gp->encoding == i) {
                    ng++;
                    gp++;
                    if (ng == nglyphs)
                      gp = 0;
                }
            } else {
                /*
                 * If the glyph exists, then darken the rectangle to indicate
                 * this.
                 */
                if (gp && gp->encoding == i) {
                    gdk_draw_rectangle(gtk_widget_get_window(widget), gc, FALSE,
                                       rect.x + 1, rect.y + 1,
                                       rect.width - 2, rect.height - 2);
                    ng++;
                    gp++;
                    if (ng == nglyphs)
                      gp = 0;
                }
                fontgrid_draw_encoding(widget, gc, lx, ly, nbuf, len);
            }
        }

        if (glyphs) {
            rect.x = x + 1;
            rect.y = y + fw->label_height + 1;
            rect.width = fw->cell_width - 2;
            rect.height = (fw->cell_height - fw->label_height) - 2;

            if (i <= 0xffff && nglyphs > 0 && glyph->encoding == i) {
                /*
                 * Draw the glyph.
                 */

                /*
                 * Set the right and left limits for generating points.
                 */
                lx = x + fw->cell_width - 2;
                ly = y + fw->cell_height - 2;

                /*
                 * Adjust the X,Y coordinate pair so the bitmap points will
                 * be generated to center the glyphs horizontally and align
                 * them to the BDF font's baseline vertically.
                 */
                x += (fw->cell_width >> 1) -
                    ((font->bbx.width + font->bbx.x_offset) >> 1) + 1;
                y += fw->label_height + font->bbx.ascent + 3;

                if (IsSelected(glyph->encoding, pi->selmap)) {
                    gdk_draw_rectangle(gtk_widget_get_window(widget), gc, TRUE,
                                       rect.x + 1, rect.y + 1,
                                       rect.width - 1, rect.height - 1);
                    if (glyph->bytes > 0) {
                        fontgrid_get_glyph_points(fw, x, y, lx, ly, glyph);
                        if (fw->points_used > 0)
                          gdk_draw_points(gtk_widget_get_window(widget), fw->xor_gc,
                                          fw->points, fw->points_used);
                    }
                } else {
                    /*
                     * The glyph is not selected, so draw it according to
                     * the bytes-per-pixel of the font.
                     */
                    gdk_window_clear_area(gtk_widget_get_window(widget), rect.x, rect.y,
                                          rect.width, rect.height);
                    if (glyph->bytes > 0) {
                        fontgrid_make_rgb_image(fw, glyph);
                        gdk_draw_rgb_image(gtk_widget_get_window(widget), gc,
                                           x, y - glyph->bbx.ascent,
                                           glyph->bbx.width,
                                           glyph->bbx.height,
                                           GDK_RGB_DITHER_NONE,
                                           fw->rgb, glyph->bbx.width * 3);
                    }
                }
                glyph++;
                if (ng == nglyphs) {
                    nglyphs = 0;
                    glyph = 0;
                }
            } else {
                /*
                 * Clear the empty cell.
                 */
                if (i <= 0xffff && IsSelected(i, pi->selmap))
                  gdk_draw_rectangle(gtk_widget_get_window(widget), gc, TRUE,
                                     rect.x + 1, rect.y + 1,
                                     rect.width - 1, rect.height - 1);
                else {
                    gdk_window_clear_area(gtk_widget_get_window(widget), rect.x, rect.y,
                                          rect.width, rect.height);
                    if (i > 0xffff) {
                        gdk_draw_line(gtk_widget_get_window(widget), gc, rect.x, rect.y,
                                      rect.x + rect.width,
                                      rect.y + rect.height);
                        gdk_draw_line(gtk_widget_get_window(widget), gc,
                                      rect.x + rect.width, rect.y,
                                      rect.x, rect.y + rect.height);
                    }
                }
            }
        }
    }
}
#endif

static void
fontgrid_draw(GtkWidget *widget, cairo_t *cr)
{
    Fontgrid *fw;
    gint x, y, i;
    guint16 wd, ht, gw, gh;
    gint32 start, end;
#if !GTK_CHECK_VERSION(3, 0, 0)
    GdkGC *gc;
#endif
    GtkAllocation all;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_FONTGRID(widget));

    fw = FONTGRID(widget);

#if !GTK_CHECK_VERSION(3, 0, 0)
    gc = gtk_widget_get_style(widget)->fg_gc[gtk_widget_get_state(widget)];
#endif

    gw = fw->cell_width * fw->cell_cols;
    gh = fw->cell_height * fw->cell_rows;

    gtk_widget_get_allocation(widget, &all);

    wd = all.width;
    ht = all.height;
    x = fw->xoff = ((wd >> 1) - (gw >> 1)) - 1;
    y = fw->yoff = ((ht >> 1) - (gh >> 1)) - 1;

#if !GTK_CHECK_VERSION(3, 0, 0)
    /*
     * Draw the horizontal lines.
     */
    for (i = 0; i <= fw->cell_rows; i++) {
        gdk_draw_line(gtk_widget_get_window(widget), gc, x, y, x + gw, y);

        /*
         * Only draw the second line if this is not the last line.
         */
        if (i < fw->cell_rows)
          gdk_draw_line(gtk_widget_get_window(widget), gc, x, y + fw->label_height,
                        x + gw, y + fw->label_height);

        y += fw->cell_height;
    }

    /*
     * Draw the vertical lines.
     */
    x = fw->xoff;
    y = fw->yoff;

    for (i = 0; i <= fw->cell_cols; i++) {
        gdk_draw_line(gtk_widget_get_window(widget), gc, x, y, x, y + gh);
        x += fw->cell_width;
    }

    start = (!fw->unencoded) ? fw->npage.bcode : fw->upage.bcode;
    end = start + (gint32) (fw->pagesize - 1);

    fontgrid_draw_cells(widget, start, end, TRUE, TRUE);
#endif
}

static void
fontgrid_select_range(Fontgrid *fw, gint32 start, gint32 end)
{
    gint32 i, tmp;
    FontgridInternalPageInfo *pi;

    if (start > end) {
        tmp = start;
        start = end;
        end = tmp;
    }

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    for (i = start; i <= end; i++)
      Select(i, pi->selmap);

    /*
     * Adjust the start and end values to the current page to determine which
     * cells need to be redrawn.
     */
    tmp = pi->bcode + (fw->pagesize - 1);
    if (start >= tmp || end < pi->bcode)
      return;

    if (start < pi->bcode)
      start = pi->bcode;
    if (end > tmp)
      end = tmp;
    fontgrid_draw_cells(GTK_WIDGET(fw), start, end, FALSE, TRUE);
}

static void
fontgrid_deselect_range(Fontgrid *fw, gint32 start, gint32 end)
{
    gint32 i, tmp;
    FontgridInternalPageInfo *pi;

    if (start > end) {
        tmp = start;
        start = end;
        end = tmp;
    }

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;
    for (i = start; i <= end; i++) {
        if (IsSelected(i, pi->selmap)) {
            Unselect(i, pi->selmap);
            if (i >= pi->bcode && i <= pi->bcode + (fw->pagesize - 1))
              fontgrid_draw_cells(GTK_WIDGET(fw), i, i, FALSE, TRUE);
        }
    }
}

static void
fontgrid_deselect_all(Fontgrid *fw)
{
    FontgridInternalPageInfo *pi, *opi;

    if (!fw->unencoded) {
        pi = &fw->npage;
        opi = &fw->upage;
    } else {
        pi = &fw->upage;
        opi = &fw->npage;
    }

    if (pi->sel_start != -1 || pi->sel_end != -1)
      fontgrid_deselect_range(fw, pi->bcode, pi->bcode + fw->pagesize - 1);
    else if (opi->sel_start != -1 || opi->sel_end != -1)
      fontgrid_deselect_range(fw, opi->bcode, opi->bcode + fw->pagesize - 1);

    /*
     * Now clear the selected bitmaps.
     */
    (void) memset((char *) pi->selmap, 0, sizeof(guint32) * 2048);
    (void) memset((char *) opi->selmap, 0, sizeof(guint32) * 2048);

    /*
     * Reset the selection start and end points.
     */
    pi->sel_start = pi->sel_end = opi->sel_start = opi->sel_end = -1;
}

#if !GTK_CHECK_VERSION(3, 0, 0)
static void
fontgrid_draw_focus(GtkWidget *widget, GdkRectangle *area)
{
    GdkGC *gc;
    gint x, y, wd, ht, fwidth, fpad;
    GtkAllocation all;

    /*
     * Do something with this later to make sure the focus line width
     * is set in the GC's.
     */
    gtk_widget_style_get(widget,
                         "focus-line-width", &fwidth,
                         "focus-padding", &fpad, NULL);

    gc = gtk_widget_get_style(widget)->bg_gc[gtk_widget_get_state(widget)];

    x = (gtk_widget_get_style(widget)->xthickness + fwidth + fpad) - 1;
    y = (gtk_widget_get_style(widget)->ythickness + fwidth + fpad) - 1;

    gtk_widget_get_allocation(widget, &all);
    wd = (all.width - (x * 2));
    ht = (all.height - (y * 2));

    if (gtk_widget_has_focus(widget))
      gtk_paint_focus(gtk_widget_get_style(widget), gtk_widget_get_window(widget), gtk_widget_get_state(widget),
                      area, widget, "fontgrid", x, y, wd, ht);
    else {
        gdk_gc_set_clip_rectangle(gc, area);
        gdk_draw_rectangle(gtk_widget_get_window(widget), gc, FALSE, x, y, wd - 1, ht - 1);
        gdk_gc_set_clip_rectangle(gc, 0);
    }
}

static gint
fontgrid_expose(GtkWidget *widget, GdkEventExpose *event)
{
    GtkAllocation all;

    /*
     * Paint the shadow first.
     */
    if (gtk_widget_is_drawable(widget)) {
      gtk_widget_get_allocation(widget, &all);
      gtk_paint_shadow(gtk_widget_get_style(widget), gtk_widget_get_window(widget),
                       gtk_widget_get_state(widget), GTK_SHADOW_OUT,
                       &event->area, widget, "fontgrid",
                       0, 0,
                       all.width,
                       all.height);
    }

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

    fontgrid_draw(widget, cr);

    cairo_destroy(cr);

    fontgrid_draw_focus(widget, &event->area);

    return FALSE;
}
#endif

static gint
fontgrid_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(IS_FONTGRID(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    fontgrid_draw_focus(widget, 0);

    return FALSE;
}

static gint
fontgrid_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(IS_FONTGRID(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    fontgrid_draw_focus(widget, 0);

    return FALSE;
}

static gint
fontgrid_lose_selection(GtkWidget *widget, GdkEventSelection *event)
{
    Fontgrid *fw;
    FontgridInternalPageInfo *pi;
    gint32 code;

    fw = FONTGRID(widget);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (pi->sel_start != pi->sel_end) {
        code = pi->sel_start;
        fontgrid_deselect_all(fw);
        pi->sel_end = pi->sel_start = code;
        Select(pi->sel_start, pi->selmap);
        fontgrid_draw_cells(widget, code, code, FALSE, TRUE);
    }

    return TRUE;
}

/**************************************************************************
 *
 * Paging routines.
 *
 **************************************************************************/

static void
fontgrid_neighbor_pages(Fontgrid *fw, gint32 page, gint32 *prev, gint32 *next)
{
    gint32 bcode, l, r, m;
    guint32 nglyphs;
    bdf_glyph_t *glyphs;
    FontgridInternalPageInfo *pip;

    pip = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (fw->noblanks == FALSE ||
        (fw->unencoded == FALSE &&
         (fw->font == 0 || fw->font->glyphs_used == 0))) {
        *prev = page - 1;
        *next = (page < pip->maxpage) ? page + 1 : -1;
        return;
    }

    bcode = page * fw->pagesize;

    if (!fw->unencoded) {
        glyphs = fw->font->glyphs;
        nglyphs = fw->font->glyphs_used;
    } else {
        glyphs = fw->font->unencoded;
        nglyphs = fw->font->unencoded_used;
    }

    /*
     * Do a binary search to find the the preceding page number.
     */
    for (l = m = 0, r = nglyphs - 1; l < r; ) {
        m = (l + r) >> 1;
        if (glyphs[m].encoding < bcode)
          l = m + 1;
        else if (glyphs[m].encoding > bcode)
          r = m - 1;
        else {
            /*
             * Exact match.
             */
            l = r = m - 1;
            break;
        }
    }

    /*
     * In case the search ends on a code in the specified page.
     */
    while (r >= 0 && glyphs[r].encoding >= bcode)
      r--;

    /*
     * Set the previous page code.
     */
    *prev = (r >= 0) ? glyphs[r].encoding / fw->pagesize : -1;

    /*
     * Determine the following page code.
     */
    if (r < 0)
      r = 0;
    while (r < nglyphs && glyphs[r].encoding < bcode + fw->pagesize)
      r++;

    *next = (r < nglyphs) ? glyphs[r].encoding / fw->pagesize : -1;
}

/**************************************************************************
 *
 * Selection routines.
 *
 **************************************************************************/

static void
start_selection(GtkWidget *widget, GdkEventButton *event)
{
    Fontgrid *fw;
    gint16 x, y, row, col;
    gint32 code;
    bdf_glyph_t *gp;
    FontgridInternalPageInfo *pi, *opi;
    FontgridSelectionInfo sinfo;

    fw = FONTGRID(widget);

    x = (gint16) event->x;
    y = (gint16) event->y;

    col = fw->xoff + (fw->cell_width * fw->cell_cols);
    row = fw->yoff + (fw->cell_height * fw->cell_rows);

    /*
     * If the button press is not in the font grid proper, just return.
     */
    if (x < fw->xoff || x >= col || y < fw->yoff || y >= row)
      return;

    /*
     * Calculate the row and column that was clicked.
     */
    row = (y - fw->yoff) / fw->cell_height;
    col = (x - fw->xoff) / fw->cell_width;

    if (!fw->unencoded) {
        pi = &fw->npage;
        opi = &fw->upage;
    } else {
        pi = &fw->upage;
        opi = &fw->npage;
    }

    if (fw->orientation == GTK_ORIENTATION_HORIZONTAL)
      code = pi->bcode + (row * fw->cell_cols) + col;
    else
      code = pi->bcode + (col * fw->cell_rows) + row;

    /*
     * Any code greater than the maximum is ignored.
     */
    if (code > 0xffff)
      return;

    gp = 0;
    if (fw->font) {
        if (!fw->unencoded)
          gp = fontgrid_locate_glyph(fw->font->glyphs, fw->font->glyphs_used,
                                     code, TRUE);
        else
          gp = fontgrid_locate_glyph(fw->font->unencoded,
                                     fw->font->unencoded_used,
                                     code, TRUE);
    }

    if (gp == 0) {
        empty_glyph.encoding = code;
        gp = &empty_glyph;
    }

    if (code != pi->sel_start || code != pi->sel_end) {
        /*
         * Clear any existing selection.
         */
        if (pi->sel_start != -1 || pi->sel_end != -1 ||
            opi->sel_start != -1 || opi->sel_end != -1)
          fontgrid_deselect_all(fw);

        Select(code, pi->selmap);

        fontgrid_draw_cells(widget, code, code, FALSE, TRUE);

        pi->sel_start = pi->sel_end = code;

        /*
         * Clear the last click time to avoid situations where the second
         * click on a different cell will cause the select callback to be
         * called.
         */
        fw->last_click = 0;
    }

    sinfo.glyphs = gp;
    sinfo.num_glyphs = 1;
    sinfo.start = pi->sel_start;
    sinfo.end = pi->sel_end;
    sinfo.base = fw->base;
    sinfo.unencoded = fw->unencoded;
    if (event->type == GDK_BUTTON_PRESS &&
        event->time - fw->last_click >= fw->mclick_time) {
        sinfo.reason = FONTGRID_START_SELECTION;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                      &sinfo);
    } else if (event->type == GDK_2BUTTON_PRESS) {
        sinfo.reason = FONTGRID_ACTIVATE;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[ACTIVATE], 0,
                      &sinfo);
    }
    fw->last_click = event->time;
}

static void
extend_selection(GtkWidget *widget, gint16 x, gint16 y)
{
    Fontgrid *fw;
    gint16 row, col;
    gint32 code;
    bdf_glyph_t *gp;
    gboolean call_extend;
    FontgridInternalPageInfo *pi;
    FontgridSelectionInfo sinfo;

    fw = FONTGRID(widget);

    col = fw->xoff + (fw->cell_width * fw->cell_cols);
    row = fw->yoff + (fw->cell_height * fw->cell_rows);

    /*
     * If the button press is not in the font grid proper, just return.
     */
    if (x < fw->xoff || x >= col || y < fw->yoff || y >= row)
      return;

    /*
     * Calculate the row and column that was clicked.
     */
    row = (y - fw->yoff) / fw->cell_height;
    col = (x - fw->xoff) / fw->cell_width;

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (fw->orientation == GTK_ORIENTATION_HORIZONTAL)
      code = pi->bcode + (row * fw->cell_cols) + col;
    else
      code = pi->bcode + (col * fw->cell_rows) + row;

    /*
     * Any code greater than the maximum is ignored.
     */
    if (code > 0xffff)
      return;

    gp = 0;
    if (fw->font) {
        if (!fw->unencoded)
          gp = fontgrid_locate_glyph(fw->font->glyphs, fw->font->glyphs_used,
                                     code, TRUE);
        else
          gp = fontgrid_locate_glyph(fw->font->unencoded,
                                     fw->font->unencoded_used,
                                     code, TRUE);
        if (gp == 0) {
            empty_glyph.encoding = code;
            gp = &empty_glyph;
        }
    }

    call_extend = FALSE;
    if (code > pi->sel_end) {
        call_extend = TRUE;
        if (code <= pi->sel_start)
          fontgrid_deselect_range(fw, pi->sel_end, code - 1);
        else {
            if (pi->sel_end < pi->sel_start) {
                fontgrid_deselect_range(fw, pi->sel_end, pi->sel_start - 1);
                fontgrid_select_range(fw, pi->sel_start + 1, code);
            } else
              fontgrid_select_range(fw, pi->sel_end, code);
        }
    } else if (code < pi->sel_end) {
        call_extend = TRUE;
        if (code < pi->sel_start) {
            if (pi->sel_end > pi->sel_start) {
                fontgrid_deselect_range(fw, pi->sel_start + 1, pi->sel_end);
                fontgrid_select_range(fw, code, pi->sel_start);
            } else
              fontgrid_select_range(fw, code, pi->sel_end);
        } else
          fontgrid_deselect_range(fw, code + 1, pi->sel_end);
    }

    pi->sel_end = code;

    if (call_extend == TRUE) {
        if (pi->sel_start == pi->sel_end) {
            sinfo.glyphs = gp;
            sinfo.num_glyphs = 1;
        } else {
            sinfo.glyphs = 0;
            sinfo.num_glyphs = 0;
        }
        sinfo.start = pi->sel_start;
        sinfo.end = pi->sel_end;
        sinfo.base = fw->base;
        sinfo.unencoded = fw->unencoded;
        sinfo.reason = FONTGRID_EXTEND_SELECTION;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_EXTEND], 0,
                      &sinfo);
    }
}

static void
end_selection(GtkWidget *widget, GdkEventButton *event)
{
    Fontgrid *fw;
    bdf_glyph_t *gp;
    FontgridInternalPageInfo *pi;
    FontgridSelectionInfo sinfo;

    fw = FONTGRID(widget);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (pi->sel_start != pi->sel_end) {
        /*
         * Assert ownership of the clipboard if there is a selection of
         * more than one glyph.
         */
        gdk_selection_owner_set(gtk_widget_get_window(widget), FONTGRID_CLIPBOARD,
                                GDK_CURRENT_TIME, FALSE);
        sinfo.glyphs = 0;
        sinfo.num_glyphs = 0;
    } else {
        gp = 0;
        if (fw->font) {
            if (!fw->unencoded)
              gp = fontgrid_locate_glyph(fw->font->glyphs,
                                         fw->font->glyphs_used,
                                         pi->sel_start, TRUE);
            else
              gp = fontgrid_locate_glyph(fw->font->unencoded,
                                         fw->font->unencoded_used,
                                         pi->sel_start, TRUE);
            if (gp == 0) {
                empty_glyph.encoding = pi->sel_start;
                gp = &empty_glyph;
            }
        }

        sinfo.glyphs = gp;
        sinfo.num_glyphs = 1;
    }
    sinfo.start = pi->sel_start;
    sinfo.end = pi->sel_end;
    sinfo.base = fw->base;
    sinfo.unencoded = fw->unencoded;
    sinfo.reason = FONTGRID_END_SELECTION;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_END], 0,
                  &sinfo);
}

static void
paste_selection(GtkWidget *widget, GdkEventButton *event)
{
    start_selection(widget, event);

    if (event->state & GDK_SHIFT_MASK)
      fontgrid_paste_selection(FONTGRID(widget), FONTGRID_INSERT_PASTE);
    else if (event->state & GDK_CONTROL_MASK)
      fontgrid_paste_selection(FONTGRID(widget), FONTGRID_MERGE_PASTE);
    else
      fontgrid_paste_selection(FONTGRID(widget), FONTGRID_NORMAL_PASTE);
}

static void
copy_selection(GtkWidget *widget, GdkEventButton *event)
{
    fontgrid_copy_selection(FONTGRID(widget));
}

/**************************************************************************
 *
 * Button, pointer motion, and keyboard handling routines.
 *
 **************************************************************************/

static gint
fontgrid_button_press(GtkWidget *widget, GdkEventButton *event)
{
    switch (event->button) {
      case 1:
        if (event->state & GDK_SHIFT_MASK)
          extend_selection(widget, (gint16) event->x, (gint16) event->y);
        else
          start_selection(widget, event);
        break;
      case 2: paste_selection(widget, event); break;
      case 3: copy_selection(widget, event); break;
    }

    return FALSE;
}

static gint
fontgrid_button_release(GtkWidget *widget, GdkEventButton *event)
{
    switch (event->button) {
      case 1: end_selection(widget, event); break;
      case 2: break;
      case 3: break;
    }
    return FALSE;
}

static gint
fontgrid_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
    if (event->state & GDK_BUTTON1_MASK)
      extend_selection(widget, (gint16) event->x, (gint16) event->y);
#if 0
    /*
     * Don't need these at the moment.
     */
    if (event->state & GDK_BUTTON2_MASK) {
    }
    if (event->state & GDK_BUTTON3_MASK) {
    }
#endif
    return FALSE;
}

static gint
fontgrid_shift_key_press(GtkWidget *widget, GdkEventKey *event)
{
    Fontgrid *fw;
    bdf_glyph_t *gp;
    guint keyval;
    gint32 code, pageno;
    guint32 count;
    gboolean signal_extend, activate;
    FontgridInternalPageInfo *pi;
    FontgridSelectionInfo sinfo;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(IS_FONTGRID(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    fw = FONTGRID(widget);

    /*
     * For number keys, use them to add up a count that will effect the
     * behavior of the other keys.
     */
    if (event->keyval >= GDK_0 && event->keyval <= GDK_9) {
        fw->count = (fw->count * 10) + (event->keyval - GDK_0);
        return FALSE;
    }

    if (!fw->unencoded) {
        pi = &fw->npage;
        gp = (fw->font && fw->font->glyphs_used) ?
            (fw->font->glyphs + (fw->font->glyphs_used - 1)) : 0;
    } else {
        pi = &fw->upage;
        gp = (fw->font && fw->font->unencoded_used) ?
            (fw->font->unencoded + (fw->font->unencoded_used - 1)) : 0;
    }

    activate = FALSE;

    code = pi->sel_end;

    if ((count = fw->count) == 0)
      count = 1;

    keyval = event->keyval;
    switch (event->keyval) {
      case GDK_Page_Up:
      case GDK_KP_Page_Up:
        count *= fw->pagesize;
        keyval = GDK_Left;
        break;
      case GDK_Page_Down:
      case GDK_KP_Page_Down:
        count *= fw->pagesize;
        keyval = GDK_Right;
        break;
      case GDK_Home:
      case GDK_KP_Home:
        count = (pi->pageno - pi->minpage) * fw->pagesize;
        keyval = GDK_Left;
        break;
      case GDK_End:
      case GDK_KP_End:
        count = (pi->maxpage - pi->pageno) * fw->pagesize;
        keyval = GDK_Right;
        break;
    }

    switch (keyval) {
      case GDK_Left:
      case GDK_KP_Left:
        if (code == 0) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_VERTICAL)
          code -= (fw->cell_rows * count);
        else
          code -= count;

        if (code < 0)
          code = 0;

        break;
      case GDK_Right:
      case GDK_KP_Right:
        /*
         * Make sure that when on the unencoded pages, the final glyph is
         * the limit unlike the encoded pages where the max value is 0xffff.
         */
        if ((fw->unencoded &&
             (gp == 0 || code == gp->encoding)) ||
            code == 0xffff) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_VERTICAL)
          code += (fw->cell_rows * count);
        else
          code += count;

        if (fw->unencoded && code > gp->encoding)
          code = gp->encoding;
        else if (code > 0xffff)
          code = 0xffff;

        break;
      case GDK_Up:
      case GDK_KP_Up:
        if (code == 0) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_HORIZONTAL)
          code -= (fw->cell_cols * count);
        else
          code -= count;

        if (code < 0)
          code = 0;

        break;
      case GDK_Down:
      case GDK_KP_Down:
        /*
         * Make sure that when on the unencoded pages, the final glyph is
         * the limit unlike the encoded pages where the max value is 0xffff.
         */
        if ((fw->unencoded &&
             (gp == 0 || code == gp->encoding)) ||
            code == 0xffff) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_HORIZONTAL)
          code += (fw->cell_cols * count);
        else
          code += count;

        if (fw->unencoded && code > gp->encoding)
          code = gp->encoding;
        else if (code > 0xffff)
          code = 0xffff;

        break;
      case GDK_Return:
      case GDK_KP_Enter:
        pi->sel_end = pi->sel_start;
        activate = TRUE;
        break;
      default:
        return FALSE;
    }

    signal_extend = FALSE;
    if (code > pi->sel_end) {
        signal_extend = TRUE;
        if (code <= pi->sel_start)
          fontgrid_deselect_range(fw, pi->sel_end, code - 1);
        else {
            if (pi->sel_end < pi->sel_start) {
                fontgrid_deselect_range(fw, pi->sel_end, pi->sel_start - 1);
                fontgrid_select_range(fw, pi->sel_start + 1, code);
            } else
              fontgrid_select_range(fw, pi->sel_end, code);
        }
    } else if (code < pi->sel_end) {
        signal_extend = TRUE;
        if (code < pi->sel_start) {
            if (pi->sel_end > pi->sel_start) {
                fontgrid_deselect_range(fw, pi->sel_start + 1, pi->sel_end);
                fontgrid_select_range(fw, code, pi->sel_start);
            } else
              fontgrid_select_range(fw, code, pi->sel_end);
        } else
          fontgrid_deselect_range(fw, code + 1, pi->sel_end);
    }

    pi->sel_end = code;

    /*
     * If the selection endpoint is on some page other than the current
     * page, make sure the page holding the end point is made visible.
     */
    pageno = code / fw->pagesize;
    if (pageno != pi->pageno) {
        fw->no_sel_callback = TRUE;
        fontgrid_goto_page(fw, pageno);
    }

    /*
     * Reset the count.
     */
    fw->count = 0;

    if (signal_extend) {
        if (pi->sel_start == pi->sel_end) {
            /*
             * Set up and emit the selection start signal.
             */
            if (!fw->unencoded)
              gp = fontgrid_locate_glyph(fw->font->glyphs,
                                         fw->font->glyphs_used,
                                         code, TRUE);
            else
              gp = fontgrid_locate_glyph(fw->font->unencoded,
                                         fw->font->unencoded_used,
                                         code, TRUE);
            if (gp == 0) {
                empty_glyph.encoding = code;
                gp = &empty_glyph;
            }
            sinfo.glyphs = gp;
            sinfo.num_glyphs = 1;
        } else {
            sinfo.glyphs = 0;
            sinfo.num_glyphs = 0;
        }
        sinfo.start = pi->sel_start;
        sinfo.end = pi->sel_end;
        sinfo.base = fw->base;
        sinfo.unencoded = fw->unencoded;

        if (!activate) {
            sinfo.reason = FONTGRID_EXTEND_SELECTION;
            g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_EXTEND],
                          0, &sinfo);
        } else {
            sinfo.reason = FONTGRID_ACTIVATE;
            g_signal_emit(G_OBJECT(fw), fontgrid_signals[ACTIVATE], 0,
                          &sinfo);
        }
    }

    return TRUE;
}

static gint
fontgrid_key_press(GtkWidget *widget, GdkEventKey *event)
{
    Fontgrid *fw;
    bdf_glyph_t *gp;
    gint32 code, pageno;
    guint32 count;
    gboolean activate;
    FontgridInternalPageInfo *pi, *opi;
    FontgridSelectionInfo sinfo;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(IS_FONTGRID(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    if (event->state & GDK_SHIFT_MASK)
      return fontgrid_shift_key_press(widget, event);

    fw = FONTGRID(widget);

    /*
     * For number keys, use them to add up a count that will effect the
     * behavior of the other keys.
     */
    if (event->keyval >= GDK_0 && event->keyval <= GDK_9) {
        fw->count = (fw->count * 10) + (event->keyval - GDK_0);
        return FALSE;
    }

    if (!fw->unencoded) {
        pi = &fw->npage;
        opi = &fw->upage;
        gp = (fw->font && fw->font->glyphs_used) ?
            (fw->font->glyphs + (fw->font->glyphs_used - 1)) : 0;
    } else {
        pi = &fw->upage;
        opi = &fw->npage;
        gp = (fw->font && fw->font->unencoded_used) ?
            (fw->font->unencoded + (fw->font->unencoded_used - 1)) : 0;
    }

    activate = FALSE;

    code = pi->sel_start;

    if ((count = fw->count) == 0)
      count = 1;

    switch (event->keyval) {
      case GDK_Left:
      case GDK_KP_Left:
        if (code == 0) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_VERTICAL)
          code -= (fw->cell_rows * count);
        else
          code -= count;

        if (code < 0)
          code = 0;

        break;
      case GDK_Right:
      case GDK_KP_Right:
        /*
         * Make sure that when on the unencoded pages, the final glyph is
         * the limit unlike the encoded pages where the max value is 0xffff.
         */
        if ((fw->unencoded &&
             (gp == 0 || code == gp->encoding)) ||
            code == 0xffff) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_VERTICAL)
          code += (fw->cell_rows * count);
        else
          code += count;

        if (fw->unencoded && code > gp->encoding)
          code = gp->encoding;
        else if (code > 0xffff)
          code = 0xffff;

        break;
      case GDK_Up:
      case GDK_KP_Up:
        if (code == 0) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_HORIZONTAL)
          code -= (fw->cell_cols * count);
        else
          code -= count;

        if (code < 0)
          code = 0;

        break;
      case GDK_Down:
      case GDK_KP_Down:
        /*
         * Make sure that when on the unencoded pages, the final glyph is
         * the limit unlike the encoded pages where the max value is 0xffff.
         */
        if ((fw->unencoded &&
             (gp == 0 || code == gp->encoding)) ||
            code == 0xffff) {
            gdk_beep();
            return TRUE;
        }

        if (fw->orientation == GTK_ORIENTATION_HORIZONTAL)
          code += (fw->cell_cols * count);
        else
          code += count;

        if (fw->unencoded && code > gp->encoding)
          code = gp->encoding;
        else if (code > 0xffff)
          code = 0xffff;

        break;
      case GDK_Page_Up:
      case GDK_KP_Page_Up:
        fw->from_keyboard = TRUE;
        fontgrid_goto_previous_page(fw);
        return TRUE;
        break;
      case GDK_Page_Down:
      case GDK_KP_Page_Down:
        fw->from_keyboard = TRUE;
        fontgrid_goto_next_page(fw);
        return TRUE;
        break;
      case GDK_Home:
      case GDK_KP_Home:
        fw->from_keyboard = TRUE;
        fontgrid_goto_first_page(fw);
        return TRUE;
        break;
      case GDK_End:
      case GDK_KP_End:
        fw->from_keyboard = TRUE;
        fontgrid_goto_last_page(fw);
        return TRUE;
        break;
      case GDK_Return:
      case GDK_KP_Enter:
        pi->sel_end = pi->sel_start;
        activate = TRUE;
        break;
      case GDK_BackSpace:
      case GDK_Delete:
      case GDK_KP_Delete:
        fontgrid_cut_selection(fw);
        return TRUE;
      default:
        return FALSE;
    }

    /*
     * This turns off the selection which means the cursor is effectively
     * turned off even for the fontgrid_goto_page() call.  The reason is that
     * for keyboard navigation, the cursor should move up and down by rows and
     * not whole pages when a page change occurs.
     */
    fontgrid_deselect_all(fw);

    pageno = code / fw->pagesize;
    if (pageno != pi->pageno) {
        fw->no_sel_callback = TRUE;
        fontgrid_goto_page(fw, pageno);
    }

    pi->sel_start = pi->sel_end = code;
    Select(code, pi->selmap);
    fontgrid_draw_cells(widget, code, code, FALSE, TRUE);

    /*
     * Reset the count.
     */
    fw->count = 0;

    /*
     * Set up and emit the selection start signal.
     */
    if (!fw->unencoded)
      gp = fontgrid_locate_glyph(fw->font->glyphs, fw->font->glyphs_used,
                                 code, TRUE);
    else
      gp = fontgrid_locate_glyph(fw->font->unencoded, fw->font->unencoded_used,
                                 code, TRUE);
    if (gp == 0) {
        empty_glyph.encoding = code;
        gp = &empty_glyph;
    }
    sinfo.glyphs = gp;
    sinfo.num_glyphs = 1;
    sinfo.start = pi->sel_start;
    sinfo.end = pi->sel_end;
    sinfo.base = fw->base;
    sinfo.unencoded = fw->unencoded;

    if (!activate) {
        sinfo.reason = FONTGRID_START_SELECTION;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                      &sinfo);
    } else {
        sinfo.reason = FONTGRID_ACTIVATE;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[ACTIVATE], 0, &sinfo);
    }

    return TRUE;
}

/**************************************************************************
 *
 * Class and instance setup.
 *
 **************************************************************************/

static void
fontgrid_class_init(gpointer g_class, gpointer class_data)
{
    GObjectClass *gocp = G_OBJECT_CLASS(g_class);
    GtkWidgetClass *wcp = GTK_WIDGET_CLASS(g_class);

    /*
     * Set the class global variables.
     */
    parent_class = g_type_class_peek_parent(g_class);

    /*
     * GObject class functions.
     */
    gocp->set_property = fontgrid_set_property;
    gocp->get_property = fontgrid_get_property;
    gocp->finalize = fontgrid_finalize;

#if GTK_CHECK_VERSION(3, 0, 0)
    wcp->destroy = fontgrid_destroy;
#else
    GTK_OBJECT_CLASS(g_class)->destroy = fontgrid_destroy;
#endif

    /*
     * Instance functions.
     */
    wcp->size_allocate = fontgrid_actual_size;
    wcp->realize = fontgrid_realize;
#if GTK_CHECK_VERSION(3, 0, 0)
    wcp->get_preferred_width = fontgrid_get_preferred_width;
    wcp->get_preferred_height = fontgrid_get_preferred_height;
    wcp->draw = fontgrid_draw;
#else
    wcp->size_request = fontgrid_preferred_size;
    wcp->expose_event = fontgrid_expose;
#endif
    wcp->focus_in_event = fontgrid_focus_in;
    wcp->focus_out_event = fontgrid_focus_out;
    wcp->button_press_event = fontgrid_button_press;
    wcp->button_release_event = fontgrid_button_release;
    wcp->motion_notify_event = fontgrid_motion_notify;
    wcp->key_press_event = fontgrid_key_press;
    wcp->selection_clear_event = fontgrid_lose_selection;

    /*
     * Add parameters (a.k.a. resource) types.
     */
    g_object_class_install_property(gocp, PROP_CODE_BASE,
                                    g_param_spec_uint("codeBase",
                                                      _("Code base"),
                                                      _("Override for the code base (oct, dec, hex) for glyph codes."),
                                                      8,
                                                      16,
                                                      16,
                                                      G_PARAM_READWRITE));
    g_object_class_install_property(gocp, PROP_POWER2,
                                    g_param_spec_boolean("powersOfTwo",
                                                         _("Powers of two"),
                                                         _("Indicate whether the grid display should be a power-of-two rows."),
                                                         TRUE,
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_ORIENTATION,
                                    g_param_spec_enum("orientation",
                                                      _("Orientation"),
                                                      _("Should the grid display vertically or horizontally."),
                                                      GTK_TYPE_ORIENTATION,
                                                      GTK_ORIENTATION_HORIZONTAL,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_FONT,
                                    g_param_spec_pointer("font",
                                                         _("Font"),
                                                         _("Font to be displayed."),
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_POINT_SIZE,
                                    g_param_spec_uint("pointSize",
                                                       _("Point size"),
                                                       _("Set the default point size for new fonts."),
                                                       2,
                                                       256,
                                                       12,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_SPACING,
                                    g_param_spec_int("spacing",
                                                     _("Spacing"),
                                                     _("Set the default glyph spacing."),
                                                       BDF_PROPORTIONAL,
                                                       BDF_CHARCELL,
                                                       BDF_PROPORTIONAL,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_SKIP_BLANKS,
                                    g_param_spec_boolean("skipBlankPages",
                                                         _("Skip blank pages"),
                                                         _("Avoid displaying pages with no glyphs."),
                                                         TRUE,
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_OVERWRITE,
                                    g_param_spec_boolean("overwriteMode",
                                                         _("Overwrite mode"),
                                                         _("Pasting the selection overwrites."),
                                                         TRUE,
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_COLORS,
                                    g_param_spec_pointer("colorList",
                                                         _("Color list"),
                                                         _("Colors to be used for glyphs having bits-per-pixel > 1."),
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_INITIAL_GLYPH,
                                    g_param_spec_int("initialGlyph",
                                                      _("Initial glyph"),
                                                      _("Code of the glyph to be displayed first."),
                                                      -1,
                                                      0xffff,
                                                      -1,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_BPP,
                                    g_param_spec_int("bitsPerPixel",
                                                      _("Bits per pixel"),
                                                      _("Number of bits per pixel for grayscale glyphs."),
                                                      1,
                                                      4,
                                                      1,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_HRES,
                                    g_param_spec_int("horizontalResolution",
                                                      _("Horizontal resolution"),
                                                      _("Set the default horizontal resolution for new fonts."),
                                                      1,
                                                      2400,
                                                      100,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_VRES,
                                    g_param_spec_int("verticalResolution",
                                                      _("Vertical resolution"),
                                                      _("Set the default vertical resolution for new fonts."),
                                                      1,
                                                      2400,
                                                      100,
                                                      G_PARAM_READWRITE));

    /*
     * Add the signals these objects emit.
     */
    fontgrid_signals[SELECTION_START] =
        g_signal_new("selection-start",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(FontgridClass, selection_start),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE,
                     1,
                     G_TYPE_POINTER);
    fontgrid_signals[SELECTION_EXTEND] =
        g_signal_new("selection-extend",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(FontgridClass, selection_extend),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE,
                     1,
                     G_TYPE_POINTER);
    fontgrid_signals[SELECTION_END] =
        g_signal_new("selection-end",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(FontgridClass, selection_end),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE,
                     1,
                     G_TYPE_POINTER);
    fontgrid_signals[ACTIVATE] =
        g_signal_new("activate",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(FontgridClass, activate),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE,
                     1,
                     G_TYPE_POINTER);
    fontgrid_signals[MODIFIED] =
        g_signal_new("modified",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(FontgridClass, modified),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE,
                     1,
                     G_TYPE_POINTER);
    fontgrid_signals[TURN_TO_PAGE] =
        g_signal_new("turn_to_page",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(FontgridClass, page),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE,
                     1,
                     G_TYPE_POINTER);
}

static void
fontgrid_init(GTypeInstance *obj, gpointer g_class)
{
    Fontgrid *fw = FONTGRID(obj);
    FontgridInternalPageInfo *pi;
    GdkScreen *screen;
    gint fwidth, fpad;

    gtk_widget_set_can_focus(GTK_WIDGET(fw), TRUE);

    gtk_widget_style_get(GTK_WIDGET(fw),
                         "focus-line-width", &fwidth,
                         "focus-padding", &fpad,
                         NULL);

    fw->base = 16;
    fw->power2 = TRUE;
    fw->overwrite = TRUE;
    fw->noblanks = TRUE;
    fw->orientation = GTK_ORIENTATION_HORIZONTAL;
    fw->point_size = 12;
    fw->spacing = BDF_CHARCELL;
    fw->colors = 0;
    fw->initial_glyph = 0;
    fw->bpp = 1;

    screen =
        gdk_window_get_screen(gdk_get_default_root_window());
    fw->hres = (gint32) ((((double) gdk_screen_get_width(screen)) * 25.4) /
                         ((double) gdk_screen_get_width_mm(screen)) + 0.5);
    fw->vres = (gint32) ((((double) gdk_screen_get_height(screen)) * 25.4) /
                         ((double) gdk_screen_get_height_mm(screen)) + 0.5);

    fw->cell_rows = FGRID_DEFAULT_ROWS;
    fw->cell_cols = FGRID_DEFAULT_COLS;
    fw->border = 4;
    fw->hmargin = gtk_widget_get_style(&fw->widget)->xthickness + fwidth + fpad + fw->border;
    fw->vmargin = gtk_widget_get_style(&fw->widget)->ythickness + fwidth + fpad + fw->border;

    fontgrid_set_cell_geometry(fw);
    fontgrid_set_rows_cols(fw, 0);

    /*
     * Private variables.
     */
    fw->unencoded = FALSE;
    fw->debug = FALSE;
#if !GTK_CHECK_VERSION(3, 0, 0)
    fw->xor_gc = 0;
#endif
    fw->points_used = 0;
    fw->points_size = 0;
    fw->rgb_used = 0;
    fw->rgb_size = 0;

    fw->last_click = 0;
    fw->mclick_time = 0;

    fw->count = 0;
    memset((char *) &fw->clipboard, 0, sizeof(bdf_glyphlist_t));

    /*
     * Initialize the page information.
     */
    pi = &fw->upage;
    pi->minpage = 0;
    pi->maxpage = 0xffff / fw->pagesize;
    pi->npage = pi->ppage = -1;
    pi->pageno = pi->bcode = 0;
    pi->sel_start = pi->sel_end = -1;
    (void) memset((char *) pi->selmap, 0, sizeof(guint32) * 2048);
    pi = &fw->npage;
    pi->minpage = 0;
    pi->maxpage = 0xffff / fw->pagesize;
    pi->npage = pi->ppage = -1;
    pi->pageno = pi->bcode = 0;
    pi->sel_start = pi->sel_end = -1;
    (void) memset((char *) pi->selmap, 0, sizeof(guint32) * 2048);
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

/*
 * Type instantiation routines.
 */
GType
fontgrid_get_type(void)
{
    static GType fontgrid_type = 0;

    if (!fontgrid_type) {
        static const GTypeInfo fontgrid_info = {
            sizeof (FontgridClass),		/* class_size		*/
            0,					/* base_init		*/
            0,					/* base_finalize	*/
            fontgrid_class_init,		/* class_init		*/
            0,					/* class_finalize	*/
            0,					/* class_data		*/
            sizeof(Fontgrid),			/* instance_size	*/
            0,					/* n_preallocs		*/
            fontgrid_init,			/* instance_init	*/
            0,					/* value_table		*/
        };
        fontgrid_type = g_type_register_static(GTK_TYPE_WIDGET,
                                               "Fontgrid", &fontgrid_info, 0);
    }

    return fontgrid_type;
}

GtkWidget *
fontgrid_new(const gchar *prop1, ...)
{
    GtkWidget *w;
    va_list var_args;

    va_start(var_args, prop1);
    w = GTK_WIDGET(g_object_new_valist(fontgrid_get_type(), prop1, var_args));
    va_end(var_args);

    return w;
}

GtkWidget *
fontgrid_newv(bdf_font_t *font, guint32 pointSize, gint32 spacing,
              gboolean skipBlankPages, gboolean overwriteMode,
              gboolean powersOfTwo, guint16 *colorList, gint32 initialGlyph,
              guint codeBase, GtkOrientation orientation,
              gint32 bitsPerPixel, gint32 horizontalResolution,
              gint32 verticalResolution, FontgridPageInfo *initialPageInfo)
{
    Fontgrid *fw = FONTGRID(g_object_new(fontgrid_get_type(), NULL));
    gint32 i, boundary;
    FontgridInternalPageInfo *pi;

    fw->font = font;
    fw->point_size = pointSize;
    fw->spacing = spacing;
    fw->colors = colorList;
    fw->noblanks = skipBlankPages;
    fw->overwrite = overwriteMode;
    fw->power2 = powersOfTwo;
    fw->initial_glyph = initialGlyph;
    fw->base = codeBase;
    fw->orientation = orientation;
    fw->bpp = (font) ? font->bpp : bitsPerPixel;
    fw->hres = horizontalResolution;
    fw->vres = verticalResolution;

    /*
     * If no font has been provided, make sure a default is created.
     * Too many other things depend on a font existing.
     */
    if (font == 0) {
        fw->font = bdf_new_font(0, fw->point_size, fw->hres, fw->vres,
                                fw->spacing, fw->bpp);
        if (fw->font->name == 0)
          fw->font->name = bdf_make_xlfd_name(fw->font, g_get_prgname(),
                                              "Unknown");
    }

    fontgrid_set_cell_geometry(fw);
    fontgrid_set_rows_cols(fw, 0);

    /*
     * Initialize the page information.
     */
    pi = &fw->upage;
    pi->minpage = 0;
    pi->maxpage = 0xffff / fw->pagesize;
    pi->npage = pi->ppage = -1;
    pi->pageno = pi->bcode = 0;
    pi->sel_start = pi->sel_end = -1;
    (void) memset((char *) pi->selmap, 0, sizeof(guint32) * 2048);
    pi = &fw->npage;
    pi->minpage = 0;
    pi->maxpage = 0xffff / fw->pagesize;
    pi->npage = pi->ppage = -1;
    pi->pageno = pi->bcode = 0;
    pi->sel_start = pi->sel_end = -1;
    (void) memset((char *) pi->selmap, 0, sizeof(guint32) * 2048);

    /*
     * Determine the page info from the initial glyph setting.
     */
    if (font != 0) {
        if (fw->initial_glyph == -1)
          fw->initial_glyph = (font->glyphs_used > 0) ?
              font->glyphs->encoding : 0;

        pi = &fw->npage;
        pi->pageno = fw->initial_glyph / fw->pagesize;
        pi->bcode = pi->pageno * fw->pagesize;
        pi->sel_start = pi->sel_end = fw->initial_glyph;
        Select(fw->initial_glyph, pi->selmap);
        fontgrid_neighbor_pages(fw, pi->pageno, &pi->ppage, &pi->npage);

        /*
         * Set the min/max page numbers for the encoded glyphs.
         */
        if (font->glyphs_used > 0) {
            if (fw->noblanks) {
                pi->minpage = font->glyphs->encoding / fw->pagesize;
                pi->maxpage =
                    font->glyphs[font->glyphs_used-1].encoding / fw->pagesize;
            } else {
                pi->minpage = 0;
                pi->maxpage = 0xffff / fw->pagesize;
            }
        }

        /*
         * Set the min/max page numbers for the unencoded glyphs.
         */
        if (font->unencoded_used > 0) {
            pi = &fw->upage;

            if (fw->noblanks) {
                pi->pageno = pi->minpage =
                    font->glyphs->encoding / fw->pagesize;
                pi->maxpage =
                    font->unencoded[font->unencoded_used-1].encoding /
                    fw->pagesize;
                pi->bcode = pi->pageno * fw->pagesize;
                pi->ppage = -1;

                /*
                 * Lower boundary for the next page.
                 */
                boundary = pi->bcode + fw->pagesize;
                for (i = 0; i < font->unencoded_used &&
                         font->unencoded[i].encoding < boundary; i++) ;
                pi->npage = (i == font->unencoded_used) ?
                    -1 : font->unencoded[i].encoding / fw->pagesize;
                  
            } else {
                pi->pageno = pi->minpage = 0;
                pi->maxpage = 0xffff / fw->pagesize;
                pi->ppage = -1;
                pi->npage = pi->pageno + 1;
            }
        }
    }

    /*
     * Provide the initial page info the calling application will need
     * to set up the page changing labels.
     */
    initialPageInfo->unencoded_page = fw->unencoded;
    initialPageInfo->encoded_glyphs = (fw->font) ? fw->font->glyphs_used : 0;
    initialPageInfo->unencoded_glyphs =
        (fw->font) ? fw->font->unencoded_used : 0;

    if (!fw->unencoded) {
        initialPageInfo->previous_page = fw->npage.ppage;
        initialPageInfo->current_page = fw->npage.pageno;
        initialPageInfo->next_page = fw->npage.npage;
    } else {
        initialPageInfo->previous_page = fw->upage.ppage;
        initialPageInfo->current_page = fw->upage.pageno;
        initialPageInfo->next_page = fw->upage.npage;
    }

    return GTK_WIDGET(fw);
}

gboolean
fontgrid_has_selection(Fontgrid *fw, FontgridSelectionInfo *sinfo)
{
    FontgridInternalPageInfo *pi;

    g_return_val_if_fail(fw != 0, FALSE);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    /*
     * Set up the selection info to alert the application that the
     * base changed.
     */
    if (sinfo != 0) {
        /*
         * Initialize the selection info structure.
         */
        (void) memset((char *) sinfo, 0, sizeof(FontgridSelectionInfo));

        if (pi->sel_start == pi->sel_end) {
            if (fw->font) {
                if (!fw->unencoded)
                  sinfo->glyphs =
                      fontgrid_locate_glyph(fw->font->glyphs,
                                            fw->font->glyphs_used,
                                            pi->sel_start, TRUE);
                else
                  sinfo->glyphs =
                      fontgrid_locate_glyph(fw->font->unencoded,
                                            fw->font->unencoded_used,
                                            pi->sel_start, TRUE);
                if (sinfo->glyphs == 0) {
                    empty_glyph.encoding = pi->sel_start;
                    sinfo->glyphs = &empty_glyph;
                }
                sinfo->num_glyphs = 1;
            }
        } else {
            sinfo->glyphs = 0;
            sinfo->num_glyphs = 0;
        }

        sinfo->start = pi->sel_start;
        sinfo->end = pi->sel_end;
        sinfo->base = fw->base;
        sinfo->unencoded = fw->unencoded;
        sinfo->reason = FONTGRID_START_SELECTION;
    }

    return (pi->sel_start == -1) ? FALSE : TRUE;
}

bdf_font_t *
fontgrid_get_font(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, 0);

    return fw->font;
}

void
fontgrid_set_font(Fontgrid *fw, bdf_font_t *font, gint32 initial_glyph)
{
    GtkWidget *w;
    gint32 i, boundary;
    FontgridInternalPageInfo *pi;
    FontgridPageInfo pageinfo;

    g_return_if_fail(fw != 0);

    if (font == fw->font)
      return;

    w = GTK_WIDGET(fw);

    /*
     * Free up the existing font.
     */
    if (fw->font != 0)
      bdf_free_font(fw->font);
    fw->font = font;

    /*
     * Make sure the encoded pages are the default for newly loaded fonts.
     */
    fw->unencoded = FALSE;

    /*
     * Set the bits-per-pixel from the font.
     */
    fw->bpp = (font != 0) ? font->bpp : 1;

    /*
     * Set the initial glyph code.
     */
    fw->initial_glyph = initial_glyph;

    /*
     * Calculate the cell geometry and the rows and columns.
     */
    fontgrid_set_cell_geometry(fw);
    fontgrid_set_rows_cols(fw, 0);

    /*
     * Initialize the page information.
     */
    pi = &fw->upage;
    pi->minpage = 0;
    pi->maxpage = 0xffff / fw->pagesize;
    pi->npage = pi->ppage = -1;
    pi->pageno = pi->bcode = 0;
    pi->sel_start = pi->sel_end = -1;
    (void) memset((char *) pi->selmap, 0, sizeof(guint32) * 2048);
    pi = &fw->npage;
    pi->minpage = 0;
    pi->maxpage = 0xffff / fw->pagesize;
    pi->npage = pi->ppage = -1;
    pi->pageno = pi->bcode = 0;
    pi->sel_start = pi->sel_end = -1;
    (void) memset((char *) pi->selmap, 0, sizeof(guint32) * 2048);

    /*
     * Determine the page info from the initial glyph setting.
     */
    if (font != 0) {
        if (fw->initial_glyph == -1)
          fw->initial_glyph = (font->glyphs_used > 0) ?
              font->glyphs->encoding : 0;

        pi = &fw->npage;
        pi->pageno = fw->initial_glyph / fw->pagesize;
        pi->bcode = pi->pageno * fw->pagesize;
        pi->sel_start = pi->sel_end = fw->initial_glyph;
        Select(fw->initial_glyph, pi->selmap);
        fontgrid_neighbor_pages(fw, pi->pageno, &pi->ppage, &pi->npage);

        /*
         * Set the min/max page numbers for the encoded glyphs.
         */
        if (font->glyphs_used > 0) {
            if (fw->noblanks) {
                pi->minpage = font->glyphs->encoding / fw->pagesize;
                pi->maxpage =
                    font->glyphs[font->glyphs_used-1].encoding / fw->pagesize;
            } else {
                pi->minpage = 0;
                pi->maxpage = 0xffff / fw->pagesize;
            }
        }

        /*
         * Set the min/max page numbers for the unencoded glyphs.
         */
        if (font->unencoded_used > 0) {
            pi = &fw->upage;

            if (fw->noblanks) {
                pi->pageno = pi->minpage =
                    font->glyphs->encoding / fw->pagesize;
                pi->maxpage =
                    font->unencoded[font->unencoded_used-1].encoding /
                    fw->pagesize;
                pi->bcode = pi->pageno * fw->pagesize;
                pi->ppage = -1;

                /*
                 * Lower boundary for the next page.
                 */
                boundary = pi->bcode + fw->pagesize;
                for (i = 0; i < font->unencoded_used &&
                         font->unencoded[i].encoding < boundary; i++) ;
                pi->npage = (i == font->unencoded_used) ?
                    -1 : font->unencoded[i].encoding / fw->pagesize;
                  
            } else {
                pi->pageno = pi->minpage = 0;
                pi->maxpage = 0xffff / fw->pagesize;
                pi->ppage = -1;
                pi->npage = pi->pageno + 1;
            }
        }
    }

    /*
     * Signal that a page change has taken place so the application can do
     * setup that it needs.
     */
    pageinfo.unencoded_page = fw->unencoded;
    pageinfo.encoded_glyphs = (fw->font) ? fw->font->glyphs_used : 0;
    pageinfo.unencoded_glyphs = (fw->font) ? fw->font->unencoded_used : 0;

    if (!fw->unencoded) {
        pageinfo.previous_page = fw->npage.ppage;
        pageinfo.current_page = fw->npage.pageno;
        pageinfo.next_page = fw->npage.npage;
    } else {
        pageinfo.previous_page = fw->upage.ppage;
        pageinfo.current_page = fw->upage.pageno;
        pageinfo.next_page = fw->upage.npage;
    }

    g_signal_emit(G_OBJECT(fw), fontgrid_signals[TURN_TO_PAGE], 0, &pageinfo);

    /*
     * Queue up a resize so the grid will change size.
     */
    gtk_widget_queue_resize(w);
}

gchar *
fontgrid_get_font_messages(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, 0);

    return (fw->font) ? fw->font->acmsgs : 0;
}

guint
fontgrid_get_code_base(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, 0);

    return fw->base;
}

void
fontgrid_set_code_base(Fontgrid *fw, guint base)
{
    FontgridInternalPageInfo *pi;
    FontgridSelectionInfo sinfo;

    g_return_if_fail(fw != 0);

    switch (base) {
      case 8: case 10: case 16:
        if (fw->base != base) {
            fw->base = base;
            if (!fw->unencoded) {
                pi = &fw->npage;
                fontgrid_draw_cells(GTK_WIDGET(fw), fw->npage.bcode,
                                    fw->npage.bcode + fw->pagesize,
                                    TRUE, FALSE);
            } else
              pi = &fw->upage;

            /*
             * Set up the selection info to alert the application that the
             * base changed.
             */
            if (pi->sel_start == pi->sel_end) {
                if (fw->font) {
                    if (!fw->unencoded)
                      sinfo.glyphs =
                          fontgrid_locate_glyph(fw->font->glyphs,
                                                fw->font->glyphs_used,
                                                pi->sel_start, TRUE);
                    else
                      sinfo.glyphs =
                          fontgrid_locate_glyph(fw->font->unencoded,
                                                fw->font->unencoded_used,
                                                pi->sel_start, TRUE);
                    if (sinfo.glyphs == 0) {
                        empty_glyph.encoding = pi->sel_start;
                        sinfo.glyphs = &empty_glyph;
                    }
                    sinfo.num_glyphs = 1;
                }
            } else {
                sinfo.glyphs = 0;
                sinfo.num_glyphs = 0;
            }

            sinfo.start = pi->sel_start;
            sinfo.end = pi->sel_end;
            sinfo.base = fw->base;
            sinfo.unencoded = fw->unencoded;
            sinfo.reason = FONTGRID_BASE_CHANGE;
            g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                          &sinfo);
        }
        break;
    }
}

GtkOrientation
fontgrid_get_orientation(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, GTK_ORIENTATION_HORIZONTAL);

    return fw->orientation;
}

void
fontgrid_set_orientation(Fontgrid *fw, GtkOrientation dir)
{
    guint16 tmp;

    g_return_if_fail(fw != 0);

    if (dir != fw->orientation) {
        fw->orientation = dir;

        /*
         * Need to swap rows and cols and attempt a resize if the object
         * has been constructed.
         */
        tmp = fw->cell_rows;
        fw->cell_rows = fw->cell_cols;
        fw->cell_cols = tmp;

        gtk_widget_queue_resize(GTK_WIDGET(fw));
    }
}

void
fontgrid_get_page_info(Fontgrid *fw, FontgridPageInfo *pageinfo)
{
    g_return_if_fail(fw != 0);
    g_return_if_fail(pageinfo != 0);

    pageinfo->unencoded_page = fw->unencoded;
    pageinfo->encoded_glyphs = (fw->font) ? fw->font->glyphs_used : 0;
    pageinfo->unencoded_glyphs = (fw->font) ? fw->font->unencoded_used : 0;

    if (!fw->unencoded) {
        pageinfo->previous_page = fw->npage.ppage;
        pageinfo->current_page = fw->npage.pageno;
        pageinfo->next_page = fw->npage.npage;
    } else {
        pageinfo->previous_page = fw->upage.ppage;
        pageinfo->current_page = fw->upage.pageno;
        pageinfo->next_page = fw->upage.npage;
    }
}

/*
 * This is the routine that does the majority of the work for updating
 * page changes.
 */
static void
fontgrid_page_change_update(Fontgrid *fw, FontgridInternalPageInfo *pi)
{
    gint32 code;
    FontgridPageInfo pageinfo;
    FontgridSelectionInfo selinfo;

    code = pi->sel_start - pi->bcode;
    pi->bcode = pi->pageno * fw->pagesize;

    if (fw->from_keyboard) {
        fontgrid_deselect_all(fw);
        code += pi->bcode;
        pi->sel_start = pi->sel_end = code;
        Select(code, pi->selmap);
        fw->from_keyboard = FALSE;
    }
    fontgrid_neighbor_pages(fw, pi->pageno, &pi->ppage, &pi->npage);

    fontgrid_draw_cells(GTK_WIDGET(fw), pi->bcode,
                        pi->bcode + (fw->pagesize - 1), TRUE, TRUE);

    pageinfo.unencoded_page = fw->unencoded;
    pageinfo.encoded_glyphs = (fw->font) ? fw->font->glyphs_used : 0;
    pageinfo.unencoded_glyphs = (fw->font) ? fw->font->unencoded_used : 0;

    pageinfo.previous_page = pi->ppage;
    pageinfo.current_page = pi->pageno;
    pageinfo.next_page = pi->npage;

    g_signal_emit(G_OBJECT(fw), fontgrid_signals[TURN_TO_PAGE], 0, &pageinfo);

    /*
     * If this was called from the keyboard, then indicate the changed
     * selection.
     */
    if (!fw->no_sel_callback && fw->from_keyboard) {
        selinfo.glyphs = 0;
        selinfo.num_glyphs = 1;
        if (fw->font) {
            selinfo.glyphs = (!fw->unencoded) ?
                fontgrid_locate_glyph(fw->font->glyphs,
                                      fw->font->glyphs_used,
                                      code, TRUE) :
                fontgrid_locate_glyph(fw->font->unencoded,
                                      fw->font->unencoded_used,
                                      code, TRUE);
        }
        if (selinfo.glyphs == 0) {
            empty_glyph.encoding = code;
            selinfo.glyphs = &empty_glyph;
        }

        selinfo.reason = FONTGRID_START_SELECTION;
        selinfo.start = pi->sel_start;
        selinfo.end = pi->sel_end;
        selinfo.base = fw->base;
        selinfo.unencoded = fw->unencoded;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                      &selinfo);
    }
}

void
fontgrid_goto_page(Fontgrid *fw, gint32 pageno)
{
    guint32 mpage;
    FontgridInternalPageInfo *pi;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    mpage = 0xffff / fw->pagesize;

    if (pageno < 0)
      pageno = 0;
    if (pageno > mpage)
      pageno = mpage;

    if (pageno != pi->pageno) {
        pi->pageno = pageno;
        fontgrid_page_change_update(fw, pi);
    }
}

void
fontgrid_goto_code(Fontgrid *fw, gint32 code)
{
    gint32 pageno;
    FontgridInternalPageInfo *pi;
    FontgridSelectionInfo selinfo;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (code < 0)
      code = 0;
    if (code > 0xffff)
      code = 0xffff;

    pageno = code / fw->pagesize;

    if (pageno != pi->pageno) {
        fw->no_sel_callback = TRUE;
        pi->pageno = pageno;
        fontgrid_page_change_update(fw, pi);
    }

    fontgrid_deselect_all(fw);
    Select(code, pi->selmap);
    pi->sel_start = pi->sel_end = code;
    fontgrid_draw_cells(GTK_WIDGET(fw), code, code, FALSE, TRUE);

    selinfo.glyphs = 0;
    selinfo.num_glyphs = 1;
    if (fw->font) {
        selinfo.glyphs = (!fw->unencoded) ?
            fontgrid_locate_glyph(fw->font->glyphs,
                                  fw->font->glyphs_used,
                                  code, TRUE) :
            fontgrid_locate_glyph(fw->font->unencoded,
                                  fw->font->unencoded_used,
                                  code, TRUE);
    }
    if (selinfo.glyphs == 0) {
        empty_glyph.encoding = code;
        selinfo.glyphs = &empty_glyph;
    }

    selinfo.reason = FONTGRID_START_SELECTION;
    selinfo.start = pi->sel_start;
    selinfo.end = pi->sel_end;
    selinfo.base = fw->base;
    selinfo.unencoded = fw->unencoded;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                  &selinfo);
}

void
fontgrid_goto_first_page(Fontgrid *fw)
{
    FontgridInternalPageInfo *pi;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (pi->pageno == pi->minpage)
      return;

    pi->pageno = pi->minpage;
    fontgrid_page_change_update(fw, pi);
}

void
fontgrid_goto_last_page(Fontgrid *fw)
{
    FontgridInternalPageInfo *pi;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (pi->pageno == pi->maxpage)
      return;

    pi->pageno = pi->maxpage;
    fontgrid_page_change_update(fw, pi);
}

void
fontgrid_goto_next_page(Fontgrid *fw)
{
    FontgridInternalPageInfo *pi;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (pi->pageno == pi->maxpage)
      return;

    pi->pageno = pi->npage;
    fontgrid_page_change_update(fw, pi);
}

void
fontgrid_goto_previous_page(Fontgrid *fw)
{
    FontgridInternalPageInfo *pi;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;

    if (pi->pageno == pi->minpage)
      return;

    pi->pageno = pi->ppage;
    fontgrid_page_change_update(fw, pi);
}

gboolean
fontgrid_viewing_unencoded(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, FALSE);

    return fw->unencoded;
}

void
fontgrid_switch_encoding_view(Fontgrid *fw)
{
    FontgridInternalPageInfo *pi;

    g_return_if_fail(fw != 0);

    fw->unencoded = !fw->unencoded;
    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;
    fontgrid_draw_cells(GTK_WIDGET(fw), pi->bcode, pi->bcode + fw->pagesize,
                        TRUE, TRUE);
}

gchar *
fontgrid_get_font_name(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, 0);

    return (fw->font) ? fw->font->name : "";
}

void
fontgrid_set_font_name(Fontgrid *fw, gchar *name)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);
    g_return_if_fail(fw->font != 0);

    if (fw->font->name != 0)
      free(fw->font->name);

    if (name == 0 || *name == 0)
      fw->font->name = bdf_make_xlfd_name(fw->font, g_get_prgname(),
                                          "Unknown");
    else
      fw->font->name = g_strdup(name);

    bdf_set_modified(fw->font, 1);

    minfo.reason = FONTGRID_MODIFIED;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
}

gboolean
fontgrid_get_font_modified(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, FALSE);

    return (fw->font) ? ((fw->font->modified) ? TRUE : FALSE) : FALSE;
}

void
fontgrid_set_font_modified(Fontgrid *fw, gboolean mod)
{
    FontgridInternalPageInfo *pi;

    g_return_if_fail(fw != 0);

    if (fw->font && fw->font->modified != mod) {
        bdf_set_modified(fw->font, mod);

        if (mod == FALSE) {
            /*
             * Redraw all the labels to clear those that were showing as
             * modified.
             */
            pi = (!fw->unencoded) ? &fw->npage : &fw->upage;
            fontgrid_draw_cells(GTK_WIDGET(fw), pi->bcode,
                                (pi->bcode + fw->pagesize) - 1, TRUE, FALSE);
        } else {
            /*
             * If the font is being marked as modified, then signal the
             * application of this state.
             */
            fprintf(stderr, "MOD\n");
        }
    }
}

void
fontgrid_set_unicode_glyph_names(Fontgrid *fw, FILE *in)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);
    g_return_if_fail(in != 0);

    if (bdf_set_unicode_glyph_names(in, fw->font, 0)) {
        /*
         * Redraw the labels.
         */
        fontgrid_draw_cells(GTK_WIDGET(fw), fw->npage.bcode,
                            fw->npage.bcode + fw->pagesize, TRUE, FALSE);
        minfo.reason = FONTGRID_GLYPH_NAMES_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_set_adobe_glyph_names(Fontgrid *fw, FILE *in)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);
    g_return_if_fail(in != 0);

    if (bdf_set_adobe_glyph_names(in, fw->font, 0)) {
        /*
         * Redraw the labels.
         */
        fontgrid_draw_cells(GTK_WIDGET(fw), fw->npage.bcode,
                            fw->npage.bcode + fw->pagesize, TRUE, FALSE);
        minfo.reason = FONTGRID_GLYPH_NAMES_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_set_code_glyph_names(Fontgrid *fw, gint ch)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    if (bdf_set_glyph_code_names(ch, fw->font, 0)) {
        /*
         * Redraw the labels.
         */
        fontgrid_draw_cells(GTK_WIDGET(fw), fw->npage.bcode,
                            fw->npage.bcode + fw->pagesize, TRUE, FALSE);
        minfo.reason = FONTGRID_GLYPH_NAMES_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_make_xlfd_font_name(Fontgrid *fw)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    if ((minfo.name = bdf_make_xlfd_name(fw->font, "Foundry",
                                         "FaceName")) != 0) {
        minfo.reason = FONTGRID_NAME_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_update_font_name_from_properties(Fontgrid *fw)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    if (bdf_has_xlfd_name(fw->font)) {
        bdf_update_name_from_properties(fw->font);

        minfo.reason = FONTGRID_NAME_MODIFIED;
        minfo.name = fw->font->name;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_update_properties_from_font_name(Fontgrid *fw)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    if (bdf_update_properties_from_name(fw->font)) {
        minfo.reason = FONTGRID_PROPERTIES_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_set_font_property(Fontgrid *fw, bdf_property_t *prop)
{
    FontgridModificationInfo minfo;
    gboolean changed;
    bdf_property_t *p;

    g_return_if_fail(fw != 0);
    g_return_if_fail(prop != 0);

    changed = FALSE;

    if ((p = bdf_get_font_property(fw->font, prop->name)) == 0)
      changed = TRUE;
    else if (p->format == prop->format) {
        switch (p->format) {
          case BDF_ATOM:
            /*
             * If the atoms are different or one is NULL and the other isn't,
             * then the property will be changed.
             */
            if ((p->value.atom && prop->value.atom &&
                 strcmp(p->value.atom, prop->value.atom) != 0) ||
                p->value.atom != prop->value.atom)
              changed = TRUE;
            break;
          case BDF_INTEGER:
            if (p->value.int32 != prop->value.int32)
              changed = TRUE;
            break;
          case BDF_CARDINAL:
            if (p->value.card32 != prop->value.card32)
              changed = TRUE;
            break;
        }
    }

    /*
     * If this causes no change, just return.
     */
    if (changed == FALSE)
      return;

    bdf_add_font_property(fw->font, prop);
    minfo.reason = FONTGRID_PROPERTIES_MODIFIED;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
}

void
fontgrid_delete_font_property(Fontgrid *fw, gchar *prop_name)
{
    FontgridModificationInfo minfo;
    bdf_property_t *p;

    g_return_if_fail(fw != 0);
    g_return_if_fail(prop_name != 0);

    /*
     * If the property doesn't exist, then just return.
     */
    if ((p = bdf_get_font_property(fw->font, prop_name)) == 0)
      return;

    bdf_delete_font_property(fw->font, prop_name);
    minfo.reason = FONTGRID_PROPERTIES_MODIFIED;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
}

guint32
fontgrid_get_font_comments(Fontgrid *fw, gchar **comments)
{
    g_return_val_if_fail(fw != 0, 0);

    if (comments != 0)
      *comments = fw->font->comments;

    return fw->font->comments_len;
}

void
fontgrid_set_font_comments(Fontgrid *fw, gchar *comments)
{
    FontgridModificationInfo minfo;
    unsigned int len;

    g_return_if_fail(fw != 0);

    len = (comments) ? (unsigned int) strlen(comments) : 0;
    if (bdf_replace_comments(fw->font, comments, len)) {
        minfo.reason = FONTGRID_COMMENTS_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

gint
fontgrid_get_font_spacing(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, -1);

    return fw->font->spacing;
}

void
fontgrid_set_font_spacing(Fontgrid *fw, gint spacing)
{
    FontgridModificationInfo minfo;
    bdf_property_t p;

    g_return_if_fail(fw != 0);

    if (spacing < BDF_PROPORTIONAL || spacing > BDF_CHARCELL ||
        fw->font->spacing == spacing)
      return;

    p.name = "SPACING";
    p.format = BDF_ATOM;
    switch (spacing) {
      case BDF_PROPORTIONAL: p.value.atom = "P"; break;
      case BDF_MONOWIDTH: p.value.atom = "M"; break;
      case BDF_CHARCELL: p.value.atom = "C"; break;
    }

    bdf_add_font_property(fw->font, &p);
    minfo.reason = FONTGRID_PROPERTIES_MODIFIED;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
}

guint16
fontgrid_get_font_device_width(Fontgrid *fw)
{
    g_return_val_if_fail(fw != 0, 0);

    return fw->font->monowidth;
}

void
fontgrid_set_font_device_width(Fontgrid *fw, guint16 dwidth)
{
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    /*
     * Only set the global device width if this is not a proportional font or
     * if there the device width changed.
     */
    if (fw->font->spacing == BDF_PROPORTIONAL ||
        fw->font->monowidth == dwidth)
      return;

    fw->font->monowidth = dwidth;
    fw->font->modified = 1;

    minfo.reason = FONTGRID_DEVICE_WIDTH_MODIFIED;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
}

void
fontgrid_get_font_info(Fontgrid *fw, FontgridFontInfo *info)
{
    g_return_if_fail(fw != 0);
    g_return_if_fail(fw->font != 0);
    g_return_if_fail(info != 0);

    info->name = fw->font->name;
    info->comments = fw->font->comments;
    info->messages = fw->font->acmsgs;
    info->default_char = fw->font->default_glyph;
    info->monowidth = fw->font->monowidth;
    info->spacing = (guint16) fw->font->spacing;
    info->font_ascent = fw->font->font_ascent;
    info->font_descent = fw->font->font_descent;
    info->font_descent = fw->font->font_descent;
    info->resolution_x = fw->font->resolution_x;
    info->resolution_y = fw->font->resolution_y;
    info->bits_per_pixel = fw->font->bpp;
    memcpy((char *) &info->bbx, (char *) &fw->font->bbx, sizeof(bdf_bbx_t));
}

void
fontgrid_set_font_info(Fontgrid *fw, FontgridFontInfo *info)
{
    int mod;
    bdf_font_t *f;
    bdf_property_t prop;
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);
    g_return_if_fail(fw->font != 0);
    g_return_if_fail(info != 0);

    f = fw->font;

    minfo.reason = FONTGRID_MODIFIED;

    /*
     * Do some special stuff with the modified field so we know whether to
     * call the modified callback or not.
     */
    mod = f->modified;
    f->modified = 0;

    /*
     * Handle the default character field.  If it happens to be -1, then
     * delete the font property.  Otherwise add it.
     */
    if (info->default_char < 0)
      bdf_delete_font_property(f, "DEFAULT_CHAR");
    else {
        prop.name = "DEFAULT_CHAR";
        prop.format = BDF_CARDINAL;
        prop.value.card32 = info->default_char;
        bdf_add_font_property(f, &prop);
    }

    prop.name = "FONT_ASCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = info->font_ascent;
    bdf_add_font_property(f, &prop);

    prop.name = "FONT_DESCENT";
    prop.format = BDF_INTEGER;
    prop.value.int32 = info->font_descent;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_X";
    prop.format = BDF_CARDINAL;
    prop.value.int32 = info->resolution_x;
    bdf_add_font_property(f, &prop);

    prop.name = "RESOLUTION_Y";
    prop.format = BDF_CARDINAL;
    prop.value.int32 = info->resolution_y;
    bdf_add_font_property(f, &prop);

    prop.name = "SPACING";
    prop.format = BDF_ATOM;
    prop.value.atom = 0;
    switch (info->spacing) {
      case BDF_PROPORTIONAL: prop.value.atom = "P"; break;
      case BDF_MONOWIDTH: prop.value.atom = "M"; break;
      case BDF_CHARCELL: prop.value.atom = "C"; break;
    }
    if (prop.value.atom != 0)
      bdf_add_font_property(f, &prop);

    /*
     * If the font was modified, and has an XLFD name, make sure the XLFD name
     * gets updated from the properties and the appropriate callback is
     * called.
     */
    if (f->modified && bdf_has_xlfd_name(f))
      fontgrid_update_font_name_from_properties(fw);

    /*
     * Now determine if the monowidth field will have a resize affect on
     * things.
     */
    if (f->spacing != BDF_PROPORTIONAL) {
        if (f->monowidth == 0) {
            /*
             * Handle the special case of a proportional font being changed to
             * some other spacing.
             */
            f->monowidth = f->bbx.width;
            f->modified = 1;
        }
        if (info->monowidth != f->monowidth) {
            /*
             * Go ahead and queue up a resize in case the monowidth
             * really does change the size.
             */
            gtk_widget_queue_resize(GTK_WIDGET(fw));
            f->monowidth = f->bbx.width = info->monowidth;
            f->modified = 1;
        }
    }
    if (f->modified)
      g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    f->modified |= mod;
}

void
fontgrid_translate_glyphs(Fontgrid *fw, gint16 dx, gint16 dy,
                          gboolean all_glyphs)
{
    GtkWidget *w = (GtkWidget *) fw;
    gint32 start, end;
    FontgridInternalPageInfo *pi;
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage: &fw->upage;

    if (all_glyphs) {
        start = pi->minpage * fw->pagesize;
        end = (pi->maxpage * fw->pagesize) + fw->pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    if (bdf_translate_glyphs(fw->font, dx, dy, start, end, 0, 0,
                             fw->unencoded)) {
        gtk_widget_queue_resize(w);
        if (gtk_widget_get_realized(w))
          gdk_window_clear(gtk_widget_get_window(w));

        gtk_widget_queue_resize(w);
        if (gtk_widget_get_realized(w))
          gdk_window_clear(gtk_widget_get_window(w));

        minfo.reason = FONTGRID_GLYPHS_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_rotate_glyphs(Fontgrid *fw, gint16 degrees, gboolean all_glyphs)
{
    GtkWidget *w = (GtkWidget *) fw;
    gint32 start, end;
    FontgridInternalPageInfo *pi;
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage: &fw->upage;

    if (all_glyphs) {
        start = pi->minpage * fw->pagesize;
        end = (pi->maxpage * fw->pagesize) + fw->pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    if (bdf_rotate_glyphs(fw->font, degrees, start, end, 0, 0,
                          fw->unencoded)) {
        gtk_widget_queue_resize(w);
        if (gtk_widget_get_realized(w))
          gdk_window_clear(gtk_widget_get_window(w));

        minfo.reason = FONTGRID_GLYPHS_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_shear_glyphs(Fontgrid *fw, gint16 degrees, gboolean all_glyphs)
{
    GtkWidget *w = (GtkWidget *) fw;
    gint32 start, end;
    FontgridInternalPageInfo *pi;
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage: &fw->upage;

    if (all_glyphs) {
        start = pi->minpage * fw->pagesize;
        end = (pi->maxpage * fw->pagesize) + fw->pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    if (bdf_shear_glyphs(fw->font, degrees, start, end, 0, 0,
                          fw->unencoded)) {
        gtk_widget_queue_resize(w);
        if (gtk_widget_get_realized(w))
          gdk_window_clear(gtk_widget_get_window(w));

        minfo.reason = FONTGRID_GLYPHS_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

void
fontgrid_embolden_glyphs(Fontgrid *fw, gboolean all_glyphs)
{
    GtkWidget *w = (GtkWidget *) fw;
    gint resize;
    gint32 start, end;
    FontgridInternalPageInfo *pi;
    FontgridModificationInfo minfo;

    g_return_if_fail(fw != 0);

    pi = (!fw->unencoded) ? &fw->npage: &fw->upage;

    if (all_glyphs) {
        start = pi->minpage * fw->pagesize;
        end = (pi->maxpage * fw->pagesize) + fw->pagesize;
    } else {
        start = pi->sel_start;
        end = pi->sel_end;
    }

    resize = 0;
    if (bdf_embolden_glyphs(fw->font, start, end, 0, 0,
                            fw->unencoded, &resize)) {
        if (resize) {
            gtk_widget_queue_resize(w);
            if (gtk_widget_get_realized(w))
              gdk_window_clear(gtk_widget_get_window(w));
        } else
          /*
           * Just redisplay the selection.
           */
          fontgrid_draw_cells(w, start, end, TRUE, TRUE);

        minfo.reason = FONTGRID_GLYPHS_MODIFIED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);
    }
}

gboolean
fontgrid_clipboard_empty(Fontgrid *fw)
{
    GdkWindow *owner;
    gboolean empty = TRUE;
    GdkAtom atype;
    gint aformat, nitems;
    guchar *data;

    g_return_val_if_fail(fw != 0, empty);

    if ((owner = gdk_selection_owner_get(FONTGRID_CLIPBOARD)) == 0)
      return empty;

    /*
     * Check to see if the clipboard contents are empty or not.
     *
     * This is handled specially to allow determination of this without
     * using up what might be a lot of memory to get the whole contents.  It
     * will have to be changed for Windows.
     */
    if (gdk_property_get(owner, FONTGRID_CLIPBOARD, FONTGRID_GLYPHLIST,
                         0, 4, FALSE, &atype, &aformat, &nitems, &data)) {
        if (nitems > 0) {
            empty = FALSE;
            free((char *) data);
        }
    }

    return empty;
}

static unsigned char *
fontgrid_encode_selection(Fontgrid *fw, guint32 *bytes)
{
    FontgridInternalPageInfo *pi;
    bdf_glyph_t *gp;
    bdf_glyphlist_t *gl;
    guint16 a;
    guint32 i, nlen, bcount;
    guchar *sel, *sp;

    *bytes = 0;

    gl = &fw->clipboard;
    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;
    bdf_copy_glyphs(fw->font, pi->sel_start, pi->sel_end, gl, fw->unencoded);

    /*
     * Calculate the number of bytes that will be needed for everything except
     * the name strings and the bitmap data.
     */
    bcount = (sizeof(unsigned int) << 1) + (6 * sizeof(unsigned short)) +
        (((6 * sizeof(unsigned short)) + sizeof(unsigned int)) *
         gl->glyphs_used);

    /*
     * Figure out how much extra will be needed for the names, bitmaps, and
     * PSF Unicode mappings.
     */
    for (i = 0, gp = gl->glyphs; i < gl->glyphs_used; i++, gp++) {
        nlen = (gp->name) ? (guint32) (strlen(gp->name) + 1) : 0;
        /*
         * The extra 2 bytes is for encoding the number of bytes used for the
         * Unicode mappings, even if it is 0.  This could be a problem later
         * if a set of mappings legitimately exceeds 2^16 in length.
         */
        bcount += nlen + gp->bytes + 2 + gp->unicode.map_used;
    }

    /*
     * Allocate the storage space needed for the encoded form.
     */
    sel = sp = g_malloc(bcount);

    /*
     * Set the returned byte count.
     */
    *bytes = bcount;

    /*
     * Encode the 20-byte header.
     */
    a = (guint16) gl->bpp;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    nlen = (guint32) gl->start;
    *sp++ = (nlen >> 24) & 0xff;
    *sp++ = (nlen >> 16) & 0xff;
    *sp++ = (nlen >> 8) & 0xff;
    *sp++ = nlen & 0xff;

    nlen = (guint32) gl->end;
    *sp++ = (nlen >> 24) & 0xff;
    *sp++ = (nlen >> 16) & 0xff;
    *sp++ = (nlen >> 8) & 0xff;
    *sp++ = nlen & 0xff;

    a = (guint16) gl->glyphs_used;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (guint16) gl->bbx.width;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (guint16) gl->bbx.x_offset;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (guint16) gl->bbx.ascent;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    a = (guint16) gl->bbx.descent;
    *sp++ = (a >> 8) & 0xff;
    *sp++ = a & 0xff;

    /*
     * Go through each glyph entry and encode the data.
     */
    for (i = 0, gp = gl->glyphs; i < gl->glyphs_used; i++, gp++) {
        /*
         * Encode the glyph encoding.
         */
        nlen = (guint32) gp->encoding;
        *sp++ = (nlen >> 24) & 0xff;
        *sp++ = (nlen >> 16) & 0xff;
        *sp++ = (nlen >> 8) & 0xff;
        *sp++ = nlen & 0xff;

        /*
         * Encode the glyph device width.
         */
        a = (guint16) gp->dwidth;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        /*
         * Encode the glyph name length.
         */
        nlen = (gp->name) ? (guint32) (strlen(gp->name) + 1) : 0;
        a = (guint16) nlen;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        /*
         * Encode the four bounding box values needed.
         */
        a = (guint16) gp->bbx.width;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        a = (guint16) gp->bbx.x_offset;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        a = (guint16) gp->bbx.ascent;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        a = (guint16) gp->bbx.descent;
        *sp++ = (a >> 8) & 0xff;
        *sp++ = a & 0xff;

        /*
         * Encode the name if it exists.
         */
        if (nlen > 0) {
            (void) memcpy((char *) sp, gp->name, nlen);
            sp += nlen;
        }

        /*
         * Encode the bitmap.
         */
        if (gp->bytes > 0) {
            (void) memcpy((char *) sp, (char *) gp->bitmap, gp->bytes);
            sp += gp->bytes;
        }

        /*
         * Encode the PSF Unicode mappings.  Even if there aren't any, add
         * the encoding.
         */
        *sp++ = (gp->unicode.map_used >> 8) & 0xff;
        *sp++ = gp->unicode.map_used & 0xff;
        if (gp->unicode.map_used > 0) {
            (void) memcpy((char *) sp, (char *) gp->unicode.map,
                          sizeof(unsigned char) * gp->unicode.map_used);
            sp += gp->unicode.map_used;
        }
    }

    /*
     * Return the selection encoded as a byte stream.
     */
    return sel;
}

#define GETSHORT(s) ((s[0] << 8) | s[1])
#define GETLONG(s) ((s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3])

static void
fontgrid_decode_selection(Fontgrid *fw, guchar *sel)
{
    guint32 i, range, nlen;
    bdf_glyph_t *gp;
    bdf_glyphlist_t *gl;

    if (sel == 0)
      return;

    gl = &fw->clipboard;

    /*
     * Clear out the bitmaps and names from the existing glyphs.
     */
    for (gp = gl->glyphs, i = 0; i < gl->glyphs_size; i++, gp++) {
        if (gp->name != 0)
          free(gp->name);
        if (gp->bytes > 0)
          free((char *) gp->bitmap);
    }

    /*
     * Extract the glyph list bits per pixel.
     */
    gl->bpp = GETSHORT(sel);
    sel += 2;

    /*
     * Extract the glyph list starting and ending encodings.
     */
    gl->start = (int) GETLONG(sel);
    sel += 4;

    gl->end = (int) GETLONG(sel);
    sel += 4;

    /*
     * Extract the number of encoded glyphs.
     */
    range = (guint32) GETSHORT(sel);
    sel += 2;

    /*
     * Resize the internal glyph list clipboard if necessary.
     */
    if (range > gl->glyphs_size) {
        if (gl->glyphs_size == 0)
          gl->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * range);
        else
          gl->glyphs = (bdf_glyph_t *) realloc((char *) gl->glyphs,
                                               sizeof(bdf_glyph_t) * range);
        gl->glyphs_size = range;
    }

    /*
     * Initialize the glyph list.
     */
    (void) memset((char *) &gl->bbx, 0, sizeof(bdf_bbx_t));
    (void) memset((char *) gl->glyphs, 0,
                  sizeof(bdf_glyph_t) * gl->glyphs_size);

    gl->glyphs_used = range;

    /*
     * Decode the overall metrics of the glyph list.
     */
    gl->bbx.width = GETSHORT(sel);
    sel += 2;
    gl->bbx.x_offset = GETSHORT(sel);
    sel += 2;
    gl->bbx.ascent = GETSHORT(sel);
    sel += 2;
    gl->bbx.descent = GETSHORT(sel);
    sel += 2;
    gl->bbx.height = gl->bbx.ascent + gl->bbx.descent;
    gl->bbx.y_offset = -gl->bbx.descent;

    /*
     * Decode the glyphs.
     */
    for (i = 0, gp = gl->glyphs; i < range; i++, gp++) {
        /*
         * Get the glyph encoding.
         */
        gp->encoding = (int) GETLONG(sel);
        sel += 4;

        /*
         * Get the device width.
         */
        gp->dwidth = GETSHORT(sel);
        sel += 2;

        /*
         * Get the name length.
         */
        nlen = GETSHORT(sel);
        sel += 2;

        /*
         * Get the bounding box.
         */
        gp->bbx.width = GETSHORT(sel);
        sel += 2;
        gp->bbx.x_offset = GETSHORT(sel);
        sel += 2;
        gp->bbx.ascent = GETSHORT(sel);
        sel += 2;
        gp->bbx.descent = GETSHORT(sel);
        sel += 2;
        gp->bbx.height = gp->bbx.ascent + gp->bbx.descent;
        gp->bbx.y_offset = -gp->bbx.descent;

        /*
         * Get the name.
         */
        if (nlen > 0) {
            gp->name = (char *) malloc(nlen);
            (void) memcpy(gp->name, (char *) sel, nlen);
            sel += nlen;
        }

        /*
         * Get the bitmap.
         */

        switch (gl->bpp) {
          case 1:
            gp->bytes = ((gp->bbx.width + 7) >> 3) * gp->bbx.height;
            break;
          case 2:
            gp->bytes = (((gp->bbx.width << 1) + 7) >> 3) * gp->bbx.height;
            break;
          case 4:
            gp->bytes = (((gp->bbx.width << 2) + 7) >> 3) * gp->bbx.height;
            break;
          case 8:
            gp->bytes = gp->bbx.width * gp->bbx.height;
            break;
        }

        if (gp->bytes > 0) {
            gp->bitmap = (unsigned char *) malloc(gp->bytes);
            (void) memcpy((char *) gp->bitmap, (char *) sel, gp->bytes);
            sel += gp->bytes;
        }

        /*
         * Get the Unicode mappings.
         */
        gp->unicode.map_used = GETSHORT(sel);
        sel += 2;
        if (gp->unicode.map_used > 0) {
            gp->unicode.map_size = ((gp->unicode.map_used >> 2) + 
                                    ((gp->unicode.map_used & 3) ? 1 : 0)) << 2;
            gp->unicode.map = (unsigned char *) malloc(gp->unicode.map_size);
            (void) memcpy((char *) gp->unicode.map, (char *) sel,
                          gp->unicode.map_used);
            sel += gp->unicode.map_used;
        }
    }
}

/*
 * This function assumes the fontgrid is realized so a GdkWindow exists.
 */
void
fontgrid_copy_selection(Fontgrid *fw)
{
    GtkWidget *w;
    GdkWindow *win;
    guint32 bytes;
    guchar *sel;

    g_return_if_fail(fw != 0);

    w = GTK_WIDGET(fw);

    /*
     * Make sure the widget owns the clipboard property.
     */
    if ((win = gdk_selection_owner_get(FONTGRID_CLIPBOARD)) == 0 ||
        win != gtk_widget_get_window(w))
      gdk_selection_owner_set(gtk_widget_get_window(w), FONTGRID_CLIPBOARD, GDK_CURRENT_TIME,
                              FALSE);

    /*
     * Encode the selection as a byte stream for the clipboard.
     */
    if ((sel = fontgrid_encode_selection(fw, &bytes)) == 0)
      return;

    gdk_property_change(gtk_widget_get_window(w), FONTGRID_CLIPBOARD, FONTGRID_GLYPHLIST,
                        8, GDK_PROP_MODE_REPLACE, sel, (gint) bytes);

    /*
     * Free the data because the property now has control over it.
     */
    g_free(sel);
}

void
fontgrid_cut_selection(Fontgrid *fw)
{
    gint32 code, start, end;
    bdf_glyph_t *gp;
    FontgridInternalPageInfo *pi;
    FontgridModificationInfo minfo;
    FontgridSelectionInfo sinfo;

    g_return_if_fail(fw != 0);

    fontgrid_copy_selection(fw);

    pi = (!fw->unencoded) ? &fw->npage : &fw->upage;
    code = pi->sel_start;

    if (bdf_delete_glyphs(fw->font, pi->sel_start, pi->sel_end,
                          fw->unencoded)) {
        start = pi->sel_start;
        end = pi->sel_end;

        fontgrid_deselect_all(fw);
        Select(code, pi->selmap);
        pi->sel_start = pi->sel_end = code;
        fontgrid_draw_cells(GTK_WIDGET(fw), start, end, TRUE, TRUE);

        /*
         * Set up and emit the modified signal.
         */
        minfo.reason = FONTGRID_GLYPHS_DELETED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);

        /*
         * Set up and call the selection start signal.
         */
        gp = (!fw->unencoded) ?
            fontgrid_locate_glyph(fw->font->glyphs, fw->font->glyphs_used,
                                  code, TRUE) :
            fontgrid_locate_glyph(fw->font->unencoded,
                                  fw->font->unencoded_used, code, TRUE);
        if (gp == 0) {
            empty_glyph.encoding = code;
            gp = &empty_glyph;
        }

        sinfo.glyphs = gp;
        sinfo.num_glyphs = 1;
        sinfo.start = sinfo.end = code;
        sinfo.base = fw->base;
        sinfo.unencoded = fw->unencoded;
        sinfo.reason = FONTGRID_START_SELECTION;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                      &sinfo);
    }
}

void
fontgrid_paste_selection(Fontgrid *fw, FontgridPasteType paste_type)
{
    GtkWidget *w = GTK_WIDGET(fw);
    GdkWindow *win;
    GdkAtom atype;
    gint afmt, nitems, unenc, doresize;
    gint32 i;
    unsigned int ng;
    guchar *data;
    bdf_font_t *font;
    bdf_glyph_t *gp;
    bdf_glyphlist_t *gl;
    FontgridInternalPageInfo *pi;
    bdf_glyphlist_t overflow;
    FontgridModificationInfo minfo;
    FontgridSelectionInfo sinfo;

    g_return_if_fail(fw != 0);
    g_return_if_fail(gtk_widget_get_realized(w));

    if ((win = gdk_selection_owner_get(FONTGRID_CLIPBOARD)) == 0) {
        gdk_selection_owner_set(gtk_widget_get_window(w), FONTGRID_CLIPBOARD,
                                GDK_CURRENT_TIME, FALSE);
        /*
         * Return here because there was no owner of the selection.
         */
        return;
    }

    doresize = 0;
    unenc = fw->unencoded;

    pi = (!unenc) ? &fw->npage : &fw->upage;

    nitems = 0;
    (void) gdk_property_get(win, FONTGRID_CLIPBOARD, FONTGRID_GLYPHLIST,
                            0, 102400, FALSE, &atype, &afmt, &nitems, &data);

    /*
     * Attempt to own the clipboard after getting the value if this widget
     * does not own it.
     */
    if (win != gtk_widget_get_window(w))
      gdk_selection_owner_set(gtk_widget_get_window(w), FONTGRID_CLIPBOARD, GDK_CURRENT_TIME,
                              FALSE);

    if (nitems > 0) {
        font = fw->font;
        gl = &fw->clipboard;

        /*
         * Convert the encoded selection into a glyph list in the internal
         * glyph list clipboard.
         */
        fontgrid_decode_selection(fw, data);

        /*
         * If the paste is occuring in the unencoded section, make sure the
         * paste is appended as opposed to being inserted.  Also turn off
         * the selected cell before doing the paste.
         */
        if (unenc) {
            fontgrid_deselect_all(fw);
            pi->sel_start = font->unencoded_used;
            gl->start = 0;
            gl->end = gl->glyphs_used - 1;
        }

        /*
         * Set the end point of the selection.
         */
        pi->sel_end = pi->sel_start + (gl->end - gl->start);

        /*
         * First, check to see if pasting the glyphs will exceed the maximum
         * encoding value of 0xffff.  If some of them do, then transfer the
         * extra glyphs to the unencoded area before doing anything else.
         * This means that a new glyph list needs to be constructed to do the
         * insert into the unencoded area.
         */
        if (!unenc && pi->sel_end > 0xffff) {
            /*
             * Determine if any of the glyphs would actually get encoded after
             * 0xffff or if those are all empty glyphs.
             */
            for (ng = 0, gp = gl->glyphs; ng < gl->glyphs_used; ng++, gp++) {
                if (pi->sel_start + (gp->encoding - gl->start) > 0xffff)
                  /*
                   * The glyph list does contain glyphs that will overflow.
                   */
                  break;
            }

            if (ng < gl->glyphs_used) {
                /*
                 * Construct a new glyph list containing only the glyphs that
                 * overflow the 0xffff boundary.  There is no need to
                 * recalculate the bounding box for the new glyph list.  Any
                 * resize will be handled correctly anyway.
                 */
                (void) memcpy((char *) &overflow.bbx, (char *) &gl->bbx,
                              sizeof(bdf_bbx_t));
                overflow.bpp = font->bpp;
                overflow.glyphs_used = gl->glyphs_used - ng;
                overflow.glyphs = gp;
                overflow.start = 0;
                overflow.end = overflow.glyphs_used - 1;

                /*
                 * Add the glyphs to the unencoded area.
                 */
                doresize = bdf_replace_glyphs(font, font->unencoded_used,
                                              &overflow, 1);
            }

            /*
             * Adjust the glyph list and selection to fit within the 0xffff
             * limit before pasting the glyphs into the font.
             */
            gl->glyphs_used = ng;
            gl->end -= pi->sel_end - 0xffff;
            pi->sel_end = 0xffff;
        }

        /*
         * If the grid is in insert mode, then determine if moving glyphs
         * forward from the insert location would cause an overflow.
         */
        if (!unenc &&
            (!fw->overwrite || paste_type == FONTGRID_INSERT_PASTE)) {
            doresize += bdf_insert_glyphs(font, pi->sel_start, gl);
            /*
             * Force a page recalculation to be done so the application can
             * update if needed.
             */
            fontgrid_goto_page(fw, fw->npage.pageno);
        } else if (paste_type == FONTGRID_MERGE_PASTE)
          doresize += bdf_merge_glyphs(font, pi->sel_start, gl, unenc);
        else
          doresize += bdf_replace_glyphs(font, pi->sel_start, gl, unenc);

        /*
         * If the paste has more than one glyph, make sure the whole
         * range is selected.
         */
        for (i = pi->sel_start; i <= pi->sel_end; i++)
          Select(i, pi->selmap);

        /*
         * If the incoming glyphs changed the font bounding box, then
         * determine the new geometry and attempt a resize.
         */
        if (doresize) {
            fontgrid_set_cell_geometry(fw);
            fontgrid_set_rows_cols(fw, 0);
            gtk_widget_queue_resize(w);
        } else
          fontgrid_draw_cells(w, pi->sel_start, pi->sel_end, TRUE, TRUE);

        /*
         * Update the number of pages used.
         */
        if (unenc) {
            if (fw->noblanks) {
                if (font->unencoded_used == 0)
                  pi->maxpage = 0;
                else {
                    gp = font->unencoded + (font->unencoded_used - 1);
                    pi->maxpage = gp->encoding / fw->pagesize;
                }
            }
        } else {
            if (fw->noblanks) {
                if (font->glyphs_used == 0)
                  pi->maxpage = 0;
                else {
                    gp = font->glyphs + (font->glyphs_used - 1);
                    pi->maxpage = gp->encoding / fw->pagesize;
                }
            }
        }

        /*
         * Set up and call the modified callback.
         */
        /*
         * Set up and emit the modified signal.
         */
        minfo.reason = FONTGRID_GLYPHS_PASTED;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &minfo);

        if (pi->sel_start == pi->sel_end) {
            /*
             * Set up and call the selection start signal.
             */
            gp = (!fw->unencoded) ?
                fontgrid_locate_glyph(fw->font->glyphs, fw->font->glyphs_used,
                                      pi->sel_start, TRUE) :
                fontgrid_locate_glyph(fw->font->unencoded,
                                      fw->font->unencoded_used, pi->sel_start,
                                      TRUE);
            if (gp == 0) {
                empty_glyph.encoding = pi->sel_start;
                gp = &empty_glyph;
            }

            sinfo.glyphs = gp;
            sinfo.num_glyphs = 1;
        } else {
            sinfo.glyphs = 0;
            sinfo.num_glyphs = 0;
        }
        sinfo.start = pi->sel_start;
        sinfo.end = pi->sel_end;
        sinfo.base = fw->base;
        sinfo.unencoded = fw->unencoded;
        sinfo.reason = FONTGRID_START_SELECTION;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                      &sinfo);

        /*
         * And last, since the change of the selection owner caused the
         * clipboard to lose its data, add the data to it again so
         * it can be pasted in some other font editor.
         */
        gdk_property_change(gtk_widget_get_window(w), FONTGRID_CLIPBOARD,
                            FONTGRID_GLYPHLIST, 8, GDK_PROP_MODE_REPLACE,
                            data, (gint) nitems);

        g_free((char *) data);
    }
}

void
fontgrid_update_metrics(Fontgrid *fw, bdf_metrics_t *metrics)
{
    FontgridModificationInfo mi;

    g_return_if_fail(fw != 0);
    g_return_if_fail(fw->font != 0);

    if (bdf_set_font_bbx(fw->font, metrics)) {
        /*
         * Need to resize.
         */

        /*
         * Calculate the cell geometry and the rows and columns.
         */
        fontgrid_set_cell_geometry(fw);
        fontgrid_set_rows_cols(fw, 0);

        gtk_widget_queue_resize(GTK_WIDGET(fw));

        mi.reason = FONTGRID_FONT_METRICS_MODIFIED;
        mi.name = 0;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &mi);
    }
}

void
fontgrid_update_glyph(Fontgrid *fw, bdf_glyph_t *glyph, gboolean unencoded)
{
    FontgridInternalPageInfo *pi;
    bdf_glyph_t *gp;
    bdf_glyphlist_t gl;
    FontgridModificationInfo mi;

    g_return_if_fail(fw != 0);
    g_return_if_fail(fw->font != 0);

    gl.bpp = fw->font->bpp;
    gl.start = gl.end = glyph->encoding;
    gl.glyphs = glyph;
    gl.glyphs_used = 1;
    memcpy((char *) &gl.bbx, (char *) &glyph->bbx, sizeof(bdf_bbx_t));

    if (bdf_replace_glyphs(fw->font, glyph->encoding, &gl, unencoded)) {
        /*
         * The font geometry was changed by the glyph being pasted.
         * A resize will be needed.
         */

        /*
         * Calculate the cell geometry and the rows and columns.
         */
        fontgrid_set_cell_geometry(fw);
        fontgrid_set_rows_cols(fw, 0);

        gtk_widget_queue_resize(GTK_WIDGET(fw));

        mi.reason = FONTGRID_FONT_METRICS_MODIFIED;
        mi.name = 0;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &mi);
    } else
      /*
       * Simply redraw the cells that were modified.
       */
      fontgrid_draw_cells(GTK_WIDGET(fw), glyph->encoding, glyph->encoding,
                          TRUE, TRUE);

    pi = (fw->unencoded) ? &fw->upage : &fw->npage;
    if (unencoded) {
        if (fw->noblanks) {
            if (fw->font->unencoded_used == 0)
              pi->maxpage = 0;
            else {
                gp = fw->font->unencoded + (fw->font->unencoded_used - 1);
                pi->maxpage = gp->encoding / fw->pagesize;
            }
        }
    } else {
        if (fw->noblanks) {
            if (fw->font->glyphs_used == 0)
              pi->maxpage = 0;
            else {
                gp = fw->font->glyphs + (fw->font->glyphs_used - 1);
                pi->maxpage = gp->encoding / fw->pagesize;
            }
        }
    }

    mi.reason = FONTGRID_GLYPHS_MODIFIED;
    mi.name = 0;
    mi.start = mi.end = glyph->encoding;
    mi.unencoded = fw->unencoded;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &mi);
}

void
fontgrid_update_psf_mappings(Fontgrid *fw, gint32 encoding,
                             bdf_psf_unimap_t *mappings)
{
    FontgridModificationInfo mi;

    g_return_if_fail(fw != 0);
    g_return_if_fail(fw->font != 0);

    if (bdf_replace_mappings(fw->font, encoding, mappings, fw->unencoded)) {
        mi.reason = FONTGRID_PSF_MAPPINGS_MODIFIED;
        mi.name = 0;
        mi.start = mi.end = encoding;
        mi.unencoded = fw->unencoded;
        g_signal_emit(G_OBJECT(fw), fontgrid_signals[MODIFIED], 0, &mi);
    }
}

gboolean
fontgrid_select_next_glyph(Fontgrid *fw, gint32 code)
{
    bdf_glyph_t *gp;
    gint32 pageno;
    guint32 count;
    FontgridInternalPageInfo *pi;
    FontgridSelectionInfo sinfo;

    g_return_val_if_fail(fw != NULL, FALSE);
    g_return_val_if_fail(IS_FONTGRID(fw), FALSE);

    if (!fw->unencoded) {
        pi = &fw->npage;
        gp = (fw->font && fw->font->glyphs_used) ?
            (fw->font->glyphs + (fw->font->glyphs_used - 1)) : 0;
    } else {
        pi = &fw->upage;
        gp = (fw->font && fw->font->unencoded_used) ?
            (fw->font->unencoded + (fw->font->unencoded_used - 1)) : 0;
    }

    if ((count = fw->count) == 0)
      count = 1;

    /*
     * Make sure that when on the unencoded pages, the final glyph is
     * the limit unlike the encoded pages where the max value is 0xffff.
     */
    if ((fw->unencoded &&
         (gp == 0 || code == gp->encoding)) ||
        code == 0xffff) {
        gdk_beep();
        return FALSE;
    }

    if (fw->orientation == GTK_ORIENTATION_VERTICAL)
      code += (fw->cell_rows * count);
    else
      code += count;

    if (fw->unencoded && code > gp->encoding)
      code = gp->encoding;
    else if (code > 0xffff)
      code = 0xffff;

    fontgrid_deselect_all(fw);

    pageno = code / fw->pagesize;
    if (pageno != pi->pageno) {
        fw->no_sel_callback = TRUE;
        fontgrid_goto_page(fw, pageno);
    }

    pi->sel_start = pi->sel_end = code;
    Select(code, pi->selmap);
    fontgrid_draw_cells(GTK_WIDGET(fw), code, code, FALSE, TRUE);

    /*
     * Reset the count.
     */
    fw->count = 0;

    /*
     * Set up and emit the selection start signal.
     */
    if (!fw->unencoded)
      gp = fontgrid_locate_glyph(fw->font->glyphs, fw->font->glyphs_used,
                                 code, TRUE);
    else
      gp = fontgrid_locate_glyph(fw->font->unencoded, fw->font->unencoded_used,
                                 code, TRUE);
    if (gp == 0) {
        empty_glyph.encoding = code;
        gp = &empty_glyph;
    }
    sinfo.glyphs = gp;
    sinfo.num_glyphs = 1;
    sinfo.start = pi->sel_start;
    sinfo.end = pi->sel_end;
    sinfo.base = fw->base;
    sinfo.unencoded = fw->unencoded;

    sinfo.reason = FONTGRID_START_SELECTION;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                  &sinfo);
    return TRUE;
}

gboolean
fontgrid_select_previous_glyph(Fontgrid *fw, gint32 code)
{
    bdf_glyph_t *gp;
    gint32 pageno;
    guint32 count;
    FontgridInternalPageInfo *pi;
    FontgridSelectionInfo sinfo;

    g_return_val_if_fail(fw != NULL, FALSE);
    g_return_val_if_fail(IS_FONTGRID(fw), FALSE);

    if (!fw->unencoded) {
        pi = &fw->npage;
        gp = (fw->font && fw->font->glyphs_used) ?
            (fw->font->glyphs + (fw->font->glyphs_used - 1)) : 0;
    } else {
        pi = &fw->upage;
        gp = (fw->font && fw->font->unencoded_used) ?
            (fw->font->unencoded + (fw->font->unencoded_used - 1)) : 0;
    }

    if ((count = fw->count) == 0)
      count = 1;

    if (code == 0) {
        gdk_beep();
        return FALSE;
    }

    if (fw->orientation == GTK_ORIENTATION_VERTICAL)
      code -= (fw->cell_rows * count);
    else
      code -= count;

    if (code < 0)
      code = 0;

    fontgrid_deselect_all(fw);

    pageno = code / fw->pagesize;
    if (pageno != pi->pageno) {
        fw->no_sel_callback = TRUE;
        fontgrid_goto_page(fw, pageno);
    }

    pi->sel_start = pi->sel_end = code;
    Select(code, pi->selmap);
    fontgrid_draw_cells(GTK_WIDGET(fw), code, code, FALSE, TRUE);

    /*
     * Reset the count.
     */
    fw->count = 0;

    /*
     * Set up and emit the selection start signal.
     */
    if (!fw->unencoded)
      gp = fontgrid_locate_glyph(fw->font->glyphs, fw->font->glyphs_used,
                                 code, TRUE);
    else
      gp = fontgrid_locate_glyph(fw->font->unencoded, fw->font->unencoded_used,
                                 code, TRUE);
    if (gp == 0) {
        empty_glyph.encoding = code;
        gp = &empty_glyph;
    }
    sinfo.glyphs = gp;
    sinfo.num_glyphs = 1;
    sinfo.start = pi->sel_start;
    sinfo.end = pi->sel_end;
    sinfo.base = fw->base;
    sinfo.unencoded = fw->unencoded;

    sinfo.reason = FONTGRID_START_SELECTION;
    g_signal_emit(G_OBJECT(fw), fontgrid_signals[SELECTION_START], 0,
                  &sinfo);
    return TRUE;
}

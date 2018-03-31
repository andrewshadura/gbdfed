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

#include "glyphedit.h"

#ifdef HAVE_XLIB
#include <gdk/gdkx.h>
#endif

#include <gdk/gdkkeysyms-compat.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(s) dgettext(GETTEXT_PACKAGE,s)
#else
#define _(s) (s)
#endif

/*
 * Each pixel will be displayed by a square with this number of pixels
 * on each side.
 */
#define MIN_PIXEL_SIZE     2
#define MAX_PIXEL_SIZE     20
#define DEFAULT_PIXEL_SIZE 10

#define HMARGINS(gw) ((gw)->hmargin << 1)
#define VMARGINS(gw) ((gw)->vmargin << 1)

/*
 * Crosshair cursor.
 */
static const gchar *cross_xpm[] = {
"13 13 2 1",
" 	c None",
".	c #000000",
"      .      ",
"      .      ",
"      .      ",
"      .      ",
"      .      ",
"             ",
".....   .....",
"             ",
"      .      ",
"      .      ",
"      .      ",
"      .      ",
"      .      "
};

/*
 * Macros that represent the properties used by this type of object.
 */
#define GLYPHEDIT_CLIPBOARD gdk_atom_intern("GLYPHEDIT_CLIPBOARD", FALSE)
#define GLYPHEDIT_BDF_CHAR gdk_atom_intern("GLYPHEDIT_BDF_CHAR", FALSE)
#define GLYPHEDIT_BITMAP gdk_atom_intern("GLYPHEDIT_BITMAP", FALSE)
#define GLYPHEDIT_GLYPH gdk_atom_intern("GLYPHEDIT_GLYPH", FALSE)

/*
 * Set default values.
 */

/*
 * Enums used for identifying properties.
 */
enum {
    PROP_0 = 0,
    GLYPH_GRID,
    BASELINE_COLOR,
    SELECTION_COLOR,
    CURSOR_COLOR,
    PIXEL_SIZE,
    SHOW_X_HEIGHT,
    SHOW_CAP_HEIGHT,
    COLOR_LIST,
    OPERATION
};

/*
 * The list of signals emitted by these objects.
 */
enum {
    GLYPH_MODIFIED = 0,
    POINTER_MOVED,
    OPERATION_CHANGE,
    COLOR_CHANGE
};

/**************************************************************************
 *
 * Local variables.
 *
 **************************************************************************/

static GtkWidgetClass *parent_class = 0;
static guint glyphedit_signals[COLOR_CHANGE + 1];

/**************************************************************************
 *
 * Class member functions.
 *
 **************************************************************************/

static void
glyphedit_set_property(GObject *obj, guint prop_id, const GValue *value,
                       GParamSpec *pspec)
{
    GtkWidget *widget;
    Glyphedit *gw;

    widget = GTK_WIDGET(obj);
    gw = GLYPHEDIT(obj);

    switch (prop_id) {
      case GLYPH_GRID:
        glyphedit_set_grid(gw,
                           (bdf_glyph_grid_t *) g_value_get_pointer(value));
        break;
      case BASELINE_COLOR:
        break;
      case SELECTION_COLOR:
        break;
      case CURSOR_COLOR:
        break;
      case PIXEL_SIZE:
        glyphedit_set_pixel_size(gw, g_value_get_uint(value));
        break;
      case SHOW_X_HEIGHT:
        glyphedit_set_show_x_height(gw, g_value_get_boolean(value));
        break;
      case SHOW_CAP_HEIGHT:
        glyphedit_set_show_cap_height(gw, g_value_get_boolean(value));
        break;
      case COLOR_LIST:
        gw->colors = (guint16 *) g_value_get_pointer(value);
        gtk_widget_queue_draw(widget);
        break;
      case OPERATION:
        glyphedit_set_operation(gw,
                                (GlypheditOperation) g_value_get_uint(value));
        break;
    }
}

static void
glyphedit_get_property(GObject *obj, guint prop_id, GValue *value,
                       GParamSpec *pspec)
{
    Glyphedit *gw;

    gw = GLYPHEDIT(obj);

    switch (prop_id) {
      case GLYPH_GRID:
        g_value_set_pointer(value, gw->grid);
        break;
      case BASELINE_COLOR:
        break;
      case SELECTION_COLOR:
        break;
      case CURSOR_COLOR:
        break;
      case PIXEL_SIZE:
        g_value_set_uint(value, gw->pixel_size);
        break;
      case SHOW_X_HEIGHT:
        g_value_set_boolean(value, gw->show_x_height);
        break;
      case SHOW_CAP_HEIGHT:
        g_value_set_boolean(value, gw->show_cap_height);
        break;
      case COLOR_LIST:
        g_value_set_pointer(value, gw->colors);
        break;
      case OPERATION:
        g_value_set_uint(value, (guint) gw->op);
        break;
    }
}

static void
glyphedit_destroy(GtkObject *obj)
{
    Glyphedit *gw;
    GlypheditClass *gwc;

    /*
     * Do some checks to make sure the incoming object exists and is the right
     * kind.
     */
    g_return_if_fail(obj != 0);
    g_return_if_fail(IS_GLYPHEDIT(obj));

    gw = GLYPHEDIT(obj);
    gwc = GLYPHEDIT_GET_CLASS(obj);

    /*
     * Unreference objects used class-wide so they get deallocated properly
     * when no longer used. The unreference only needs to happen the first
     * time since the objects are created at class initialization time.
     */
    if (gwc->cursor != 0)
      gdk_cursor_unref(gwc->cursor);

    if (gwc->gridgc != 0)
      g_object_unref(G_OBJECT(gwc->gridgc));
    if (gwc->bbxgc != 0)
      g_object_unref(G_OBJECT(gwc->bbxgc));
    if (gwc->pixgc != 0)
      g_object_unref(G_OBJECT(gwc->pixgc));
    if (gwc->selgc != 0)
      g_object_unref(G_OBJECT(gwc->selgc));

    /*
     * Free up any colors allocated.
     */
    if (gw->baselineColor.pixel != 0)
      gdk_colormap_free_colors(gtk_widget_get_style(&gw->widget)->colormap,
                               &gw->baselineColor, 1);

    gwc->cursor = 0;
    gwc->gridgc = gwc->bbxgc = gwc->pixgc = gwc->selgc = 0;

    /*
     * Free up the grid info.
     */
    if (gw->grid != 0) {
        bdf_free_glyph_grid(gw->grid);
        gw->grid = 0;
    }

    if (gw->spot_size > 0) {
        g_free(gw->spot);
        gw->spot_size = gw->spot_used = 0;
    }

    GTK_OBJECT_CLASS(parent_class)->destroy(obj);
}

static void
glyphedit_finalize(GObject *obj)
{
    /*
     * Do some checks to make sure the incoming object exists and is the right
     * kind.
     */
    g_return_if_fail(obj != 0);
    g_return_if_fail(IS_GLYPHEDIT(obj));

    /*
     * Follow the class chain back up to free up resources allocated in the
     * parent classes.
     */
    G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void
glyphedit_preferred_size(GtkWidget *widget, GtkRequisition *preferred)
{
    Glyphedit *gw;
    GdkScreen *screen;
    guint16 dht, margin;

    gw = GLYPHEDIT(widget);

    screen = gdk_display_get_default_screen(gdk_display_get_default());
    dht = gdk_screen_get_height(screen);

    /*
     * This little bit of code quietly forces the glyph grid to be
     * at most 1/2 the height of the screen being used to help avoid taking
     * up too much space on the desktop.
     */
    margin = VMARGINS(gw);
    preferred->height = margin +
        ((gw->pixel_size + 4) * gw->grid->grid_height);
    if (preferred->height > (dht >> 1)) {
        while (gw->pixel_size > 2) {
            preferred->height = margin +
                ((gw->pixel_size + 4) * gw->grid->grid_height);
            if (preferred->height < (dht >> 1))
              break;
            gw->pixel_size--;
        }
    }

    preferred->width = HMARGINS(gw) +
        ((gw->pixel_size + 4) * gw->grid->grid_width);
}

static void
glyphedit_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
    GtkRequisition requisition;

    glyphedit_preferred_size(widget, &requisition);

    *minimal_width = *natural_width = requisition.width;
}

static void
glyphedit_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
    GtkRequisition requisition;

    glyphedit_preferred_size(widget, &requisition);

    *minimal_height = *natural_height = requisition.height;
}

static void
glyphedit_actual_size(GtkWidget *widget, GtkAllocation *actual)
{
    gtk_widget_set_allocation(widget, actual);

    if (gtk_widget_get_realized(widget))
      gdk_window_move_resize(gtk_widget_get_window(widget), actual->x, actual->y,
                             actual->width, actual->height);
}

static void
glyphedit_draw_pixel(Glyphedit *gw, gint16 x, gint16 y, gboolean sel)
{
    GtkWidget *w = GTK_WIDGET(gw);
    GlypheditClass *gwc;
    gint16 bpr, set, dx, dy, di, si;
    guchar *masks, *bmap;
    GdkRectangle pix;
    GtkAllocation all;

    if (!gtk_widget_get_realized(w) || gw->grid == 0)
      return;

    gwc = GLYPHEDIT_GET_CLASS(gw);

    di = 0;
    masks = 0;
    switch (gw->grid->bpp) {
      case 1: masks = bdf_onebpp; di = 7; break;
      case 2: masks = bdf_twobpp; di = 3; break;
      case 4: masks = bdf_fourbpp; di = 1; break;
      case 8: masks = bdf_eightbpp; di = 0; break;
    }

    dx = (gw->pixel_size + 4) * gw->grid->grid_width;
    dy = (gw->pixel_size + 4) * gw->grid->grid_height;

    gtk_widget_get_allocation(&gw->widget, &all);

    pix.x = (all.width >> 1) - (dx >> 1) +
        ((gw->pixel_size + 4) * x) + 2;
    pix.y = (all.height >> 1) - (dy >> 1) +
        ((gw->pixel_size + 4) * y) + 2;
    pix.width = pix.height = gw->pixel_size + 1;

    if (sel == TRUE && gw->grid->sel.width != 0) {
        bpr = ((gw->grid->sel.width * gw->grid->bpp) + 7) >> 3;
        dy = y - gw->grid->sel.y;
        dx = (x - gw->grid->sel.x) * gw->grid->bpp;
        bmap = gw->grid->sel.bitmap;
    } else {
        bpr = ((gw->grid->grid_width * gw->grid->bpp) + 7) >> 3;
        dy = y;
        dx = x * gw->grid->bpp;
        bmap = gw->grid->bitmap;
    }
    si = (dx & 7) / gw->grid->bpp;
    set = bmap[(dy * bpr) + (dx >> 3)] & masks[si];
    if (di > si)
      set >>= (di - si) * gw->grid->bpp;

    if (set) {
        if (gw->grid->bpp > 1) {
            switch (gw->grid->bpp) {
              case 2:
                memset(gw->spot, gw->colors[set-1], gw->spot_used);
                break;
              case 4:
                memset(gw->spot, gw->colors[set-1+4], gw->spot_used);
                break;
              case 8:
                memset(gw->spot, set, gw->spot_used);
                break;
            }
            gdk_draw_gray_image(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->pixgc,
                                pix.x, pix.y, pix.width, pix.height,
                                GDK_RGB_DITHER_NONE, gw->spot, pix.width);
        } else
          gdk_draw_rectangle(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->pixgc, TRUE,
                             pix.x, pix.y, pix.width, pix.height);
    } else
      gdk_window_clear_area(gtk_widget_get_window(GTK_WIDGET(gw)), pix.x, pix.y,
                            pix.width, pix.height);
    if (sel == TRUE)
      gdk_draw_rectangle(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->selgc, TRUE,
                         pix.x + 1, pix.y + 1,
                         pix.width - 2, pix.height - 2);
}

static void
glyphedit_draw_glyph(Glyphedit *gw)
{
    GtkWidget *w = GTK_WIDGET(gw);
    gint16 x, y;
    gboolean sel;

    if (!gtk_widget_get_realized(w) || gw->grid == 0)
      return;

    for (y = 0; y < gw->grid->grid_height; y++) {
        for (x = 0; x < gw->grid->grid_width; x++) {
            sel = (bdf_in_selection(gw->grid, x, y, 0) ? TRUE : FALSE);
            glyphedit_draw_pixel(gw, x, y, sel);
        }
    }
}

static void
glyphedit_draw_font_bbx(Glyphedit *gw)
{
    GtkWidget *w = GTK_WIDGET(gw);
    GlypheditClass *gwc;
    gint16 xoff, yoff, fxoff, fyoff, psize;
    GdkRectangle frame;
    GtkAllocation all;

    if (!gtk_widget_get_realized(w))
      return;

    gwc = GLYPHEDIT_GET_CLASS(gw);

    psize = gw->pixel_size + 4;
    frame.width = psize * gw->grid->font_bbx.width;
    frame.height = psize *
        (gw->grid->font_bbx.ascent + gw->grid->font_bbx.descent);

    fxoff = psize * gw->grid->grid_width;
    fyoff = psize * gw->grid->grid_height;

    gtk_widget_get_allocation(&gw->widget, &all);
    frame.x = (all.width >> 1) - (fxoff >> 1);
    frame.y = (all.height >> 1) - (fyoff >> 1);

    if (gw->grid->font_bbx.x_offset < 0)
      fxoff = psize * (gw->grid->base_x + gw->grid->font_bbx.x_offset);
    else
      fxoff = psize * gw->grid->base_x;

    fyoff = psize * (gw->grid->base_y - gw->grid->font_bbx.ascent);

    /*
     * Due to some odd behavior, the box has to be drawn with the y point off
     * by one because the top of the rectangle does not get drawn otherwise.
     * Even calling gdk_draw_line() specifically doesn't work.
     *
     * This may have been fixed in later versions of GDK.
     */
    gdk_draw_rectangle(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->bbxgc, FALSE,
                       frame.x + fxoff, frame.y + fyoff + 1,
                       frame.width, frame.height);

    /*
     * Draw vertical baseline.
     */
    xoff = (gw->pixel_size + 4) * gw->grid->base_x;
    yoff = (gw->pixel_size + 4) * gw->grid->base_y;

    gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->bbxgc,
                  frame.x + xoff, frame.y + fyoff,
                  frame.x + xoff, frame.y + fyoff + frame.height);

    /*
     * Draw horizontal baseline.
     */
    gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->bbxgc,
                  frame.x + fxoff, frame.y + yoff,
                  frame.x + fxoff + frame.width, frame.y + yoff);

    /*
     * Draw the CAP_HEIGHT if indicated and exists.
     */
    if (gw->grid && gw->grid->cap_height != 0) {
        yoff = (gw->pixel_size + 4) *
            (gw->grid->base_y - gw->grid->cap_height);
        if (gw->show_cap_height == TRUE)
          gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->bbxgc,
                        frame.x + fxoff, frame.y + yoff,
                        frame.x + fxoff + frame.width, frame.y + yoff);
        else {
            gdk_window_clear_area(gtk_widget_get_window(GTK_WIDGET(gw)), frame.x + fxoff,
                                  frame.y + yoff, frame.width, 1);
            gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->gridgc,
                          frame.x + fxoff, frame.y + yoff,
                          frame.x + fxoff + frame.width, frame.y + yoff);
        }
    }

    /*
     * Draw the X_HEIGHT if indicated and exists.
     */
    if (gw->grid && gw->grid->x_height != 0) {
        yoff = (gw->pixel_size + 4) * (gw->grid->base_y - gw->grid->x_height);
        if (gw->show_x_height == TRUE)
          gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->bbxgc,
                        frame.x + fxoff, frame.y + yoff,
                        frame.x + fxoff + frame.width, frame.y + yoff);
        else {
            gdk_window_clear_area(gtk_widget_get_window(GTK_WIDGET(gw)), frame.x + fxoff,
                                  frame.y + yoff, frame.width, 1);
            gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(gw)), gwc->gridgc,
                          frame.x + fxoff, frame.y + yoff,
                          frame.x + fxoff + frame.width, frame.y + yoff);
        }
    }
}

static void
glyphedit_draw(GtkWidget *widget, GdkRegion *region)
{
    Glyphedit *gw;
    gint x, y, limit, unit, wd, ht;
    GlypheditClass *gwc;
    GdkRectangle frame;
    GtkAllocation all;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_GLYPHEDIT(widget));

    gw = GLYPHEDIT(widget);
    gwc = GLYPHEDIT_GET_CLASS(widget);

    wd = gw->grid->grid_width;
    ht = gw->grid->grid_height;

    frame.width = (gw->pixel_size + 4) * wd;
    frame.height = (gw->pixel_size + 4) * ht;

    gtk_widget_get_allocation(widget, &all);

    /*
     * Adjust the frame horizontal and vertical positions so it
     * always appears centered on the window.
     */
    frame.x = (all.width >> 1) - (frame.width >> 1);
    frame.y = (all.height >> 1) - (frame.height >> 1);

    /*
     * Limit the drawing area to the clip region.
     */
    if (region != 0)
      gdk_gc_set_clip_region(gwc->gridgc, region);

    /*
     * Draw the outside frame.
     */
    gdk_draw_rectangle(gtk_widget_get_window(widget), gwc->gridgc, FALSE,
                       frame.x, frame.y, frame.width, frame.height);

    /*
     * Draw the vertical grid lines.
     */
    limit = frame.x + frame.width;
    unit = gw->pixel_size + 4;
    for (x = frame.x + unit, y = frame.y; x < limit; x += unit)
      gdk_draw_line(gtk_widget_get_window(widget), gwc->gridgc, x, y, x, y + frame.height);

    /*
     * Draw the horizontal grid lines.
     */
    limit = frame.y + frame.height;
    for (x = frame.x, y = frame.y + unit; y < limit; y += unit)
      gdk_draw_line(gtk_widget_get_window(widget), gwc->gridgc, x, y, x + frame.width, y);

    if (region != 0)
      gdk_gc_set_clip_region(gwc->gridgc, 0);

    glyphedit_draw_font_bbx(gw);
    glyphedit_draw_glyph(gw);
}

static void
glyphedit_create_gcs(GtkWidget *widget, gboolean force)
{
    Glyphedit *gw;
    GlypheditClass *gwc;
    GdkGCValuesMask gcm;
    GdkGCValues gcv;
    gint8 dashes[2] = {1, 1};

    gw = GLYPHEDIT(widget);
    gwc = GLYPHEDIT_GET_CLASS(G_OBJECT(widget));

    gcm = GDK_GC_FOREGROUND|GDK_GC_BACKGROUND|GDK_GC_FUNCTION;

    if (gwc->gridgc == 0 || force == TRUE) {
        if (gwc->gridgc != 0)
          g_object_unref(G_OBJECT(gwc->gridgc));
        gcv.foreground.pixel =
            gtk_widget_get_style(widget)->fg[gtk_widget_get_state(widget)].pixel;
        gcv.background.pixel =
            gtk_widget_get_style(widget)->bg[gtk_widget_get_state(widget)].pixel;
        gcv.function = GDK_COPY;
        gcv.line_style = GDK_LINE_ON_OFF_DASH;
        gwc->gridgc = gdk_gc_new_with_values(gtk_widget_get_window(widget), &gcv,
                                             gcm|GDK_GC_LINE_STYLE);

        /*
         * Now set the dash lengths since they can't be set in the GC values.
         */
        gdk_gc_set_dashes(gwc->gridgc, 0, dashes, 2);
    }

    if (gwc->bbxgc == 0 || force == TRUE) {
        if (gwc->bbxgc != 0)
          g_object_unref(G_OBJECT(gwc->bbxgc));

        if (gw->baselineColor.pixel == 0)
          /*
           * Default to red.
           */
          gdk_colormap_alloc_color(gtk_widget_get_style(&gw->widget)->colormap,
                                   &gw->baselineColor, FALSE, TRUE);

        gcv.foreground.pixel = gw->baselineColor.pixel;
        gcv.function = GDK_COPY;
        gwc->bbxgc = gdk_gc_new_with_values(gtk_widget_get_window(widget), &gcv,
                                            GDK_GC_FOREGROUND|GDK_GC_FUNCTION);
    }

    if (gwc->selgc == 0 || force == TRUE) {
        if (gwc->selgc != 0)
          g_object_unref(G_OBJECT(gwc->selgc));

        gcv.foreground.pixel =
            gtk_widget_get_style(widget)->fg[gtk_widget_get_state(widget)].pixel;
        gcv.background.pixel =
            gtk_widget_get_style(widget)->bg[gtk_widget_get_state(widget)].pixel;
        gcv.foreground.pixel ^= gcv.background.pixel;
        gcv.function = GDK_XOR;
        gwc->selgc = gdk_gc_new_with_values(gtk_widget_get_window(widget), &gcv, gcm);
    }

    if (gwc->pixgc == 0 || force == TRUE) {
        if (gwc->pixgc != 0)
          g_object_unref(G_OBJECT(gwc->pixgc));

        gcv.foreground.pixel =
            gtk_widget_get_style(widget)->fg[gtk_widget_get_state(widget)].pixel;
        gcv.background.pixel =
            gtk_widget_get_style(widget)->bg[gtk_widget_get_state(widget)].pixel;
        gcv.function = GDK_COPY;
        gwc->pixgc = gdk_gc_new_with_values(gtk_widget_get_window(widget), &gcv, gcm);
    }
}

static void
glyphedit_realize(GtkWidget *widget)
{
    GlypheditClass *gwc;
    GdkWindowAttr attributes;
    gint attributes_mask;
    GdkPixbuf *cb;
    GtkAllocation all;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_GLYPHEDIT(widget));

    gwc = GLYPHEDIT_GET_CLASS(widget);
    gtk_widget_set_realized(widget, TRUE);

    gtk_widget_get_allocation(widget, &all);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = all.x;
    attributes.y = all.y;
    attributes.width = all.width;
    attributes.height = all.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.colormap = gtk_widget_get_colormap(widget);
    attributes.event_mask = gtk_widget_get_events(widget);
    attributes.event_mask |= (GDK_EXPOSURE_MASK|GDK_BUTTON_PRESS_MASK|
                              GDK_BUTTON_RELEASE_MASK|GDK_ENTER_NOTIFY_MASK|
                              GDK_POINTER_MOTION_MASK|
                              GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|
                              GDK_LEAVE_NOTIFY_MASK|GDK_FOCUS_CHANGE_MASK|
                              GDK_PROPERTY_CHANGE_MASK);

    attributes_mask = GDK_WA_X|GDK_WA_Y|GDK_WA_VISUAL|GDK_WA_COLORMAP;

    gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget),
                                  &attributes, attributes_mask));
    gdk_window_set_user_data(gtk_widget_get_window(widget), widget);

    gtk_widget_set_style(widget, gtk_style_attach(gtk_widget_get_style(widget), gtk_widget_get_window(widget)));
    gtk_style_set_background(gtk_widget_get_style(widget), gtk_widget_get_window(widget), GTK_STATE_NORMAL);

    /*
     * Create the crosshair cursor.
     */
    if (gwc->cursor == 0) {
        gwc = GLYPHEDIT_GET_CLASS(widget);
        cb = gdk_pixbuf_new_from_xpm_data(cross_xpm);
        gwc->cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(),
                                                 cb, 7, 7);
        g_object_unref(G_OBJECT(cb));
    }

    glyphedit_create_gcs(widget, FALSE);

    gdk_window_set_cursor(gtk_widget_get_window(widget), gwc->cursor);
}

static gboolean
#if GTK_CHECK_VERSION(3, 0, 0)
glyphedit_draw(GtkWidget *widget, cairo_t *cr)
#else
glyphedit_expose(GtkWidget *widget, GdkEventExpose *event)
#endif
{
    GtkAllocation all;

    gtk_widget_get_allocation(widget, &all);

    /*
     * Paint the shadow first.
     */
    if (gtk_widget_is_drawable(widget))
      gtk_paint_shadow(gtk_widget_get_style(widget), gtk_widget_get_window(widget),
                       gtk_widget_get_state(widget), GTK_SHADOW_OUT,
                       &event->area, widget, "glyphedit",
                       0, 0,
                       all.width,
                       all.height);

    glyphedit_draw(widget, event->region);

    return FALSE;
}

static gint
glyphedit_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(IS_GLYPHEDIT(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    GLYPHEDIT(widget)->mouse_down = FALSE;

    return FALSE;
}

/**************************************************************************
 *
 * Class and object initialization routines.
 *
 **************************************************************************/

static GType
glyphedit_get_operation_type(void)
{
    static GType etype = 0;
    if (etype == 0) {
        static const GEnumValue values[] = {
            {GLYPHEDIT_NONE, "GLYPHEDIT_NONE", "none"},
            {GLYPHEDIT_SELECT, "GLYPHEDIT_SELECT", "select"},
            {GLYPHEDIT_DRAW, "GLYPHEDIT_DRAW", "draw"},
            {GLYPHEDIT_MOVE, "GLYPHEDIT_MOVE", "move"},
            {GLYPHEDIT_COPY, "GLYPHEDIT_COPY", "copy"},
            {GLYPHEDIT_FLIP_HORIZONTAL,
             "GLYPHEDIT_FLIP_HORIZONTAL",
             "flip-horizontal"},
            {GLYPHEDIT_FLIP_VERTICAL,
             "GLYPHEDIT_FLIP_VERTICAL",
             "flip-verticalal"},
            {GLYPHEDIT_SHEAR, "GLYPHEDIT_SHEAR", "shear"},
            {GLYPHEDIT_ROTATE_LEFT,
             "GLYPHEDIT_ROTATE_LEFT",
             "rotate-left"},
            {GLYPHEDIT_ROTATE_RIGHT,
             "GLYPHEDIT_ROTATE_RIGHT",
             "rotate-right"},
            {GLYPHEDIT_ROTATE,
             "GLYPHEDIT_ROTATE",
             "rotate"},
            {GLYPHEDIT_SHIFT_UP_LEFT,
             "GLYPHEDIT_SHIFT_UP_LEFT",
             "shift-up-left"},
            {GLYPHEDIT_SHIFT_UP,
             "GLYPHEDIT_SHIFT_UP",
             "shift-up"},
            {GLYPHEDIT_SHIFT_UP_RIGHT,
             "GLYPHEDIT_SHIFT_UP_RIGHT",
             "shift-up-right"},
            {GLYPHEDIT_SHIFT_LEFT,
             "GLYPHEDIT_SHIFT_LEFT",
             "shift-left"},
            {GLYPHEDIT_SHIFT_RIGHT,
             "GLYPHEDIT_SHIFT_RIGHT",
             "shift-right"},
            {GLYPHEDIT_SHIFT_DOWN_LEFT,
             "GLYPHEDIT_SHIFT_DOWN_LEFT",
             "shift-down-left"},
            {GLYPHEDIT_SHIFT_DOWN,
             "GLYPHEDIT_SHIFT_DOWN",
             "shift-down"},
            {GLYPHEDIT_SHIFT_DOWN_RIGHT,
             "GLYPHEDIT_SHIFT_DOWN_RIGHT",
             "shift-down-right"},
            {0, 0, 0}
        };
        etype = g_enum_register_static("GlypheditOperation", values);
    }
    return etype;
}

static void
glyphedit_init(GTypeInstance *obj, gpointer g_class)
{
    Glyphedit *gw = GLYPHEDIT(obj);
    GlypheditClass *gwc = GLYPHEDIT_CLASS(g_class);
    gint fwidth, fpad;

    gwc->gridgc = gwc->bbxgc = gwc->pixgc = gwc->selgc = 0;

    gw->default_pixel_size = gw->pixel_size = DEFAULT_PIXEL_SIZE;

    /*
     * Make sure the spot is the right size.
     */
    fpad = (gw->pixel_size + 1) * (gw->pixel_size + 1);
    if (gw->spot_size < fpad) {
        if (gw->spot_size == 0)
          gw->spot = g_malloc(fpad);
        else
          gw->spot = g_realloc(gw->spot, fpad);
        gw->spot_size = fpad;
    }
    gw->spot_used = fpad;

    gw->owns_clipboard = FALSE;

    gw->grid = 0;

    gw->last_x = gw->last_y = 0;

    memset((char *) &gw->sel_start, 0, sizeof(GdkPoint));
    memset((char *) &gw->sel_end, 0, sizeof(GdkPoint));

    /*
     * Always initialize to the first color.
     */
    gw->cidx = 1;

    /*
     * Initialize the last color seen.
     */
    gw->lcolor = 0;

    gtk_widget_style_get(GTK_WIDGET(gw),
                         "focus-line-width", &fwidth,
                         "focus-padding", &fpad,
                         NULL);

    /*
     * Padding that will appear before and after the focus rectangle.
     * Hardcode this for now.
     */
    gw->border = 4;

    gw->hmargin = gtk_widget_get_style(&gw->widget)->xthickness + fwidth + fpad + gw->border;
    gw->vmargin = gtk_widget_get_style(&gw->widget)->ythickness + fwidth + fpad + gw->border;

    gw->baselineColor.pixel = gw->selectionColor.pixel =
        gw->cursorColor.pixel = 0;

    gw->baselineColor.red = 0xffff;
    gw->baselineColor.green = gw->baselineColor.blue = 0;

    gw->op = GLYPHEDIT_DRAW;
}

/*
 * A convenience function for calling the GLYPH_MODIFIED signal because
 * so many functions depend on it.
 */
static void
glyphedit_signal_glyph_change(Glyphedit *gw)
{
    bdf_bitmap_t image;
    bdf_metrics_t metrics;
    GlypheditSignalInfo si;

    if (gw->grid == 0)
      return;

    glyphedit_get_glyph_metrics(gw, &metrics);
    bdf_grid_image(gw->grid, &image);
    si.reason = GLYPHEDIT_GLYPH_MODIFIED;
    si.metrics = &metrics;
    si.image = &image;
    si.color = gw->cidx;

    g_signal_emit(G_OBJECT(gw), glyphedit_signals[GLYPH_MODIFIED], 0, &si);
    if (image.bytes > 0)
      free(image.bitmap);
}

/**************************************************************************
 *
 * API functions.
 *
 **************************************************************************/

GtkWidget *
glyphedit_new(const gchar *prop1, ...)
{
    GtkWidget *w;
    va_list var_args;

    va_start(var_args, prop1);
    w = GTK_WIDGET(g_object_new_valist(glyphedit_get_type(), prop1, var_args));
    va_end(var_args);

    return w;
}

GtkWidget *
glyphedit_newv(bdf_glyph_grid_t *grid, guint16 default_pixel_size,
               gboolean show_x_height, gboolean show_cap_height,
               guint16 *colors)
{
    Glyphedit *ge = g_object_new(glyphedit_get_type(),
                                 "glyphGrid", grid,
                                 "pixelSize", default_pixel_size,
                                 "showXHeight", show_x_height,
                                 "showCapHeight", show_cap_height,
                                 "colorList", colors,
                                 NULL);

    return GTK_WIDGET(ge);
}

gint32
glyphedit_get_encoding(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, -1);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), -1);

    return (gw->grid) ? gw->grid->encoding : -1;
}

void
glyphedit_get_glyph_metrics(Glyphedit *gw, bdf_metrics_t *metrics)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(metrics != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (!gw->grid)
      memset(metrics, 0, sizeof(bdf_metrics_t));
    else {
        metrics->font_spacing = gw->grid->spacing;
        metrics->swidth = gw->grid->swidth;
        metrics->dwidth = gw->grid->dwidth;
        metrics->width = gw->grid->glyph_bbx.width;
        metrics->height = gw->grid->glyph_bbx.height;
        metrics->x_offset = gw->grid->glyph_bbx.x_offset;
        metrics->y_offset = gw->grid->glyph_bbx.y_offset;
        metrics->ascent = gw->grid->glyph_bbx.ascent;
        metrics->descent = gw->grid->glyph_bbx.descent;
    }
}

void
glyphedit_get_font_metrics(Glyphedit *gw, bdf_metrics_t *metrics)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(metrics != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (!gw->grid)
      memset(metrics, 0, sizeof(bdf_metrics_t));
    else {
        metrics->font_spacing = gw->grid->spacing;
        metrics->swidth = gw->grid->swidth;
        metrics->dwidth = gw->grid->dwidth;
        metrics->width = gw->grid->font_bbx.width;
        metrics->height = gw->grid->font_bbx.height;
        metrics->x_offset = gw->grid->font_bbx.x_offset;
        metrics->y_offset = gw->grid->font_bbx.y_offset;
        metrics->ascent = gw->grid->font_bbx.ascent;
        metrics->descent = gw->grid->font_bbx.descent;
    }
}

bdf_psf_unimap_t *
glyphedit_get_psf_mappings(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, 0);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), 0);

    return (gw->grid) ? &gw->grid->unicode : 0;
}

/*
 * Can set both font and glyph metrics.
 */
void
glyphedit_set_metrics(Glyphedit *gw, bdf_metrics_t *metrics)
{
    GtkWidget *w = GTK_WIDGET(gw);

    g_return_if_fail(gw != NULL);
    g_return_if_fail(metrics != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (gw->grid == 0)
      return;

    if (bdf_grid_resize(gw->grid, metrics)) {
        glyphedit_signal_glyph_change(gw);
        gtk_widget_queue_resize(GTK_WIDGET(gw));
    } else if (gtk_widget_get_realized(w))
      /*
       * The size didn't change, but we need to redraw if the widget
       * has been realized.
       */
      gtk_widget_queue_draw(GTK_WIDGET(gw));
}

gint
glyphedit_get_spacing(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, -1);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), -1);
    g_return_val_if_fail(gw->grid != NULL, -1);

    return gw->grid->spacing;
}

void
glyphedit_set_spacing(Glyphedit *gw, gint spacing, guint16 monowidth)
{
    bdf_metrics_t metrics;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));
    g_return_if_fail(gw->grid != NULL);

    gw->grid->spacing = spacing;
    if (spacing != BDF_PROPORTIONAL) {
        glyphedit_get_font_metrics(gw, &metrics);
        metrics.dwidth = metrics.width = monowidth;
        glyphedit_set_metrics(gw, &metrics);
    }
}

void
glyphedit_set_grid(Glyphedit *gw, bdf_glyph_grid_t *grid)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    bdf_free_glyph_grid(gw->grid);
    gw->grid = grid;

    if (grid) {
      gw->last_x = grid->base_x;
      gw->last_y = grid->base_y;
    } else
      gw->last_x = gw->last_y = 0;

    /*
     * If the widget is in Move or Copy mode, change back to Select mode.
     */
    if (gw->op == GLYPHEDIT_MOVE || gw->op == GLYPHEDIT_COPY) {
        gw->pending_op = gw->op;
        gw->op = GLYPHEDIT_SELECT;
    }

    gw->cidx = 1;
    gw->lcolor = 0;

    gtk_widget_queue_resize(GTK_WIDGET(gw));
}

gboolean
glyphedit_get_modified(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, FALSE);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), FALSE);

    return (gw->grid) ? gw->grid->modified : FALSE;
}

void
glyphedit_set_modified(Glyphedit *gw, gboolean modified)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (gw->grid)
      gw->grid->modified = ((modified == TRUE) ? 1 : 0);
}

void
glyphedit_signal_modified(Glyphedit *gw)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    glyphedit_signal_glyph_change(gw);
}

gboolean
glyphedit_get_selecting(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, FALSE);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), FALSE);
    g_return_val_if_fail(gw->grid != NULL, FALSE);

    return bdf_has_selection(gw->grid, 0, 0, 0, 0) ? TRUE : FALSE;
}

gboolean
glyphedit_clipboard_empty(Glyphedit *gw)
{
    GdkWindow *owner;
    gboolean empty = TRUE;
    GdkAtom atype;
    gint aformat, nitems;
    guchar *data;

    g_return_val_if_fail(gw != NULL, empty);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), empty);

    if ((owner = gdk_selection_owner_get(GLYPHEDIT_CLIPBOARD)) == 0)
      return empty;

    /*
     * Check to see if the clipboard contents are empty or not.
     *
     * This is handled specially to allow determination of this without
     * using up what might be a lot of memory to get the whole contents.  It
     * will have to be changed for Windows.
     */
    if (gdk_property_get(owner, GLYPHEDIT_CLIPBOARD, GLYPHEDIT_BITMAP,
                         0, 16, FALSE, &atype, &aformat, &nitems, &data)) {
        if (nitems > 0) {
            empty = FALSE;
            free((char *) data);
        }
    }

    return empty;
}

void
glyphedit_get_image(Glyphedit *gw, bdf_bitmap_t *image)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));
    g_return_if_fail(image != NULL);

    if (gw->grid)
      bdf_grid_image(gw->grid, image);
    else
      memset(image, 0, sizeof(bdf_bitmap_t));
}

bdf_glyph_grid_t *
glyphedit_get_grid(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, 0);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), 0);

    return gw->grid;
}

bdf_glyph_t *
glyphedit_get_glyph(Glyphedit *gw, gboolean *unencoded)
{
    g_return_val_if_fail(gw != NULL, 0);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), 0);

    if (gw->grid) {
        if (unencoded)
          *unencoded = (gw->grid->unencoded == 0) ? FALSE : TRUE;
        return bdf_grid_glyph(gw->grid);
    }
    if (unencoded)
      *unencoded = FALSE;
    return 0;
}

void
glyphedit_set_show_cap_height(Glyphedit *gw, gboolean show)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    gw->show_cap_height = show;

    /*
     * Redraw the bounding box.
     */
    glyphedit_draw_font_bbx(gw);
}

void
glyphedit_set_show_x_height(Glyphedit *gw, gboolean show)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    gw->show_x_height = show;

    /*
     * Redraw the bounding box.
     */
    glyphedit_draw_font_bbx(gw);
}

void
glyphedit_crop_glyph(Glyphedit *gw)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (gw->grid && bdf_grid_crop(gw->grid, 1))
      glyphedit_signal_glyph_change(gw);

    glyphedit_draw_glyph(gw);
}

void
glyphedit_shift_glyph(Glyphedit *gw, gint16 xcount, gint16 ycount)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (gw->grid && bdf_grid_shift(gw->grid, xcount, ycount))
      glyphedit_signal_glyph_change(gw);

    glyphedit_draw_glyph(gw);
}

void
glyphedit_rotate_glyph(Glyphedit *gw, gint16 degrees)
{
    gint resize = 0;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (gw->grid && bdf_grid_rotate(gw->grid, degrees, &resize)) {
        glyphedit_signal_glyph_change(gw);
        if (resize)
          gtk_widget_queue_resize(GTK_WIDGET(gw));
        else
          glyphedit_draw_glyph(gw);
    }
}

void
glyphedit_shear_glyph(Glyphedit *gw, gint16 degrees)
{
    gint resize = 0;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (gw->grid && bdf_grid_shear(gw->grid, degrees, &resize)) {
        glyphedit_signal_glyph_change(gw);
        if (resize)
          gtk_widget_queue_resize(GTK_WIDGET(gw));
        else
          glyphedit_draw_glyph(gw);
    }
}

void
glyphedit_embolden_glyph(Glyphedit *gw)
{
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (gw->grid && bdf_grid_embolden(gw->grid)) {
        glyphedit_signal_glyph_change(gw);

        /*
         * Simply redraw the glyph because the size didn't change,
         * only the bitmap.
         */
        glyphedit_draw_glyph(gw);
    }
}

void
glyphedit_flip_glyph(Glyphedit *gw, GtkOrientation direction)
{
    gint flipped;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));
    g_return_if_fail(gw->grid != NULL);

    flipped = (direction == GTK_ORIENTATION_HORIZONTAL) ?
        bdf_grid_flip(gw->grid, -1) : bdf_grid_flip(gw->grid, 1);

    if (flipped) {
        glyphedit_signal_glyph_change(gw);

        /*
         * Simply redraw the glyph because the size didn't change,
         * only the bitmap.
         */
        glyphedit_draw_glyph(gw);
    }
}

void
glyphedit_set_pixel_size(Glyphedit *gw, guint pixel_size)
{
    gint bytes;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if (pixel_size < MIN_PIXEL_SIZE || pixel_size > MAX_PIXEL_SIZE)
      return;

    /*
     * Queue up a resize to force the resize and redraw.
     */
    gw->pixel_size = pixel_size;

    /*
     * Make sure the spot is the right size.
     */
    bytes = (pixel_size + 1) * (pixel_size + 1);
    if (gw->spot_size < bytes) {
        if (gw->spot_size == 0)
          gw->spot = g_malloc(bytes);
        else
          gw->spot = g_realloc(gw->spot, bytes);
        gw->spot_size = bytes;
    }
    gw->spot_used = bytes;

    gtk_widget_queue_resize(GTK_WIDGET(gw));
}

guint
glyphedit_get_pixel_size(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, GLYPHEDIT_NONE);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), GLYPHEDIT_NONE);
    g_return_val_if_fail(gw->grid != NULL, GLYPHEDIT_NONE);

    return gw->pixel_size;
}

GlypheditOperation
glyphedit_get_operation(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, GLYPHEDIT_NONE);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), GLYPHEDIT_NONE);
    g_return_val_if_fail(gw->grid != NULL, GLYPHEDIT_NONE);

    return gw->op;
}

void
glyphedit_set_operation(Glyphedit *gw, GlypheditOperation op)
{
    gint16 sx, sy, x, y, wd, ht;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));
    g_return_if_fail(gw->grid != NULL);

    if (bdf_has_selection(gw->grid, &x, &y, &wd, &ht)) {
        if (op == GLYPHEDIT_MOVE)
          bdf_detach_selection(gw->grid);
        else if (op == GLYPHEDIT_COPY)
          bdf_attach_selection(gw->grid);
        else {
            if (op == GLYPHEDIT_DRAW) {
                /*
                 * Attach the selected part of the bitmap.
                 */
                bdf_attach_selection(gw->grid);

                /*
                 * Erase the selected rectangle.
                 */
                for (sy = y; sy < y + ht; sy++) {
                    for (sx = x; sx < x + wd; sx++)
                      glyphedit_draw_pixel(gw, sx, sy, FALSE);
                }
                bdf_lose_selection(gw->grid);
            }

            gw->op = op;
        }
        gw->pending_op = GLYPHEDIT_NONE;

        glyphedit_signal_glyph_change(gw);
    } else {
        if (op == GLYPHEDIT_MOVE || op == GLYPHEDIT_COPY) {
            gw->op = GLYPHEDIT_SELECT;
            gw->pending_op = op;
        } else {
            gw->op = op;
            gw->pending_op = GLYPHEDIT_NONE;
        }
    }
}

void
glyphedit_insert_bitmap(Glyphedit *gw, bdf_bitmap_t *bitmap)
{
    GtkWidget *w = GTK_WIDGET(gw);
    GdkWindow *win;
    gint16 sx, sy, x, y, wd, ht;
    bdf_metrics_t metrics;
    GlypheditSignalInfo si;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    if ((win = gdk_selection_owner_get(GLYPHEDIT_CLIPBOARD)) == 0) {
        gdk_selection_owner_set(gtk_widget_get_window(w), GLYPHEDIT_CLIPBOARD,
                                GDK_CURRENT_TIME, FALSE);
        win = gtk_widget_get_window(w);
    } else if (win != gtk_widget_get_window(w))
      gdk_selection_owner_set(gtk_widget_get_window(w), GLYPHEDIT_CLIPBOARD,
                              GDK_CURRENT_TIME, FALSE);

    if (bdf_has_selection(gw->grid, &x, &y, &wd, &ht)) {
        /*
         * This widget already has a selection, so release it.
         */
        if (gw->op != GLYPHEDIT_SELECT)
          bdf_attach_selection(gw->grid);

        for (sy = y; sy < y + ht; sy++) {
            for (sx = x; sx < x + wd; sx++)
              glyphedit_draw_pixel(gw, sx, sy, FALSE);
        }
        bdf_lose_selection(gw->grid);
    }

    bitmap->x = gw->last_x;
    bitmap->y = gw->last_y;

    glyphedit_get_font_metrics(gw, &metrics);
    if (bitmap->width > metrics.width || bitmap->height > metrics.height) {
        /*
         * Adjust the insert position on the X axis if necessary.
         */
        if (bitmap->width > metrics.width)
          bitmap->x = gw->grid->base_x + gw->grid->font_bbx.x_offset;
        /*
         * Adjust the insert position on the Y axis and the ascent if
         * necessary.
         */
        if (bitmap->height > metrics.height) {
            bitmap->y = 0;
            metrics.ascent = bitmap->height - gw->grid->font_bbx.descent;
        }
        metrics.width = bitmap->width;
        metrics.height = bitmap->height;
        glyphedit_set_metrics(gw, &metrics);
    }

    /*
     * Set the selection in the grid.
     */
    bdf_add_selection(gw->grid, bitmap);

    /*
     * Now update the grid.
     */
    if (bdf_has_selection(gw->grid, &x, &y, &wd, &ht)) {
        for (sy = y; sy < y + ht; sy++) {
            for (sx = x; sx < x + wd; sx++)
              glyphedit_draw_pixel(gw, sx, sy, TRUE);
        }
    }

    /*
     * Set up and call the operation change signal.
     */
    si.reason = GLYPHEDIT_OPERATION_CHANGE;
    si.operation = GLYPHEDIT_MOVE;
    g_signal_emit(G_OBJECT(gw), glyphedit_signals[OPERATION_CHANGE], 0,
                  &si);

    /*
     * Set up and call the modified signal.
     */
    glyphedit_signal_glyph_change(gw);

    /*
     * Make sure the widget goes into MOVE mode at this point.
     * This allows the user to position what was pasted without
     * destroying the glyph bitmap that was already there.
     */
    if (gw->op != GLYPHEDIT_MOVE) {
        gw->op = GLYPHEDIT_MOVE;
        gw->pending_op = GLYPHEDIT_NONE;
    }

    glyphedit_copy_selection(gw);
}

static void
glyphedit_own_clipboard(Glyphedit *gw)
{
    GtkWidget *w;

    w = GTK_WIDGET(gw);
    if (!gtk_widget_get_realized(w) || gw->owns_clipboard == TRUE)
      return;

    gdk_selection_owner_set(gtk_widget_get_window(w),
                            GLYPHEDIT_CLIPBOARD,
                            GDK_CURRENT_TIME, FALSE);

    gw->owns_clipboard =
        (gdk_selection_owner_get(GLYPHEDIT_CLIPBOARD) == gtk_widget_get_window(w)) ? TRUE : FALSE;

    /*
     * The Intrinsics may need to have a SelectionClear notice sent. Probably
     * won't be necessary on Windows.
     */
}

static guchar *
glyphedit_encode_selection(Glyphedit *gw, gint *bytes)
{
    gint bcount, size;
    gint16 wd, ht;
    guchar *bmap, *bp;

    *bytes = 0;
    if (!bdf_has_selection(gw->grid, 0, 0, &wd, &ht))
      return 0;

    size = bcount = (gint) gw->grid->sel.bytes >> 1;
    size += sizeof(guint16) * 3;
    bp = bmap = (guchar *) g_malloc(size);

    /*
     * Encode the width and height in Most Significant Byte order assuming
     * the width and height types are 16-bit values.
     */
    if (!bdf_little_endian()) {
        *bp++ = (gw->grid->bpp >> 8) & 0xff;
        *bp++ = gw->grid->bpp & 0xff;
        *bp++ = (wd >> 8) & 0xff;
        *bp++ = wd & 0xff;
        *bp++ = (ht >> 8) & 0xff;
        *bp++ = ht & 0xff;
    } else {
        *bp++ = gw->grid->bpp & 0xff;
        *bp++ = (gw->grid->bpp >> 8) & 0xff;
        *bp++ = wd & 0xff;
        *bp++ = (wd >> 8) & 0xff;
        *bp++ = ht & 0xff;
        *bp++ = (ht >> 8) & 0xff;
    }

    (void) memcpy((char *) bp, (char *) gw->grid->sel.bitmap, bcount);

    *bytes = size;
    return bmap;
}

void
glyphedit_copy_selection(Glyphedit *gw)
{
    GtkWidget *w = GTK_WIDGET(gw);
    guchar *sel;
    gint bytes;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    /*
     * If the widget has no selection, then this routine will return 0.
     */
    if ((sel = glyphedit_encode_selection(gw, &bytes)) == 0)
      return;

    /*
     * Go ahead and actually write the data to the clipboard and then free the
     * buffer.
     */
    gdk_property_change(gtk_widget_get_window(w), GLYPHEDIT_CLIPBOARD,
                        GLYPHEDIT_BITMAP, 8, GDK_PROP_MODE_REPLACE,
                        sel, (gint) bytes);

    g_free(sel);
}

void
glyphedit_cut_selection(Glyphedit *gw)
{
    GtkWidget *w = GTK_WIDGET(gw);
    guchar *sel;
    gint bytes;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    /*
     * If the widget has no selection, then this routine will return 0.
     */
    if ((sel = glyphedit_encode_selection(gw, &bytes)) == 0)
      return;

    /*
     * Go ahead and actually write the data to the clipboard and then free the
     * buffer.
     */
    gdk_property_change(gtk_widget_get_window(w), GLYPHEDIT_CLIPBOARD,
                        GLYPHEDIT_BITMAP, 8, GDK_PROP_MODE_REPLACE,
                        sel, (gint) bytes);

    g_free(sel);

    /*
     * Now actually delete the selection and update the glyph.
     */
    bdf_delete_selection(gw->grid);
    bdf_lose_selection(gw->grid);
    if (gw->op != GLYPHEDIT_DRAW) {
        gw->pending_op = gw->op;
        gw->op = GLYPHEDIT_SELECT;
    }
    glyphedit_draw_glyph(gw);
    glyphedit_signal_glyph_change(gw);
}

void
glyphedit_change_operation(Glyphedit *gw, GlypheditOperation op)
{
    gint16 sx, sy, x, y, wd, ht;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));

    /*
     * Special handling is needed for move and copy operations.  If a
     * selection does not exist yet, then make the move/copy a pending
     * operation and set the operation to select.  Once the selection is made,
     * the operation will be changed to the pending move/copy operation.  If a
     * selection exists, then set the move/copy operation and detach/attach
     * the selection accordingly.
     */
    if (bdf_has_selection(gw->grid, &x, &y, &wd, &ht)) {
        if (op == GLYPHEDIT_MOVE)
          bdf_detach_selection(gw->grid);
        else if (op == GLYPHEDIT_COPY)
          bdf_attach_selection(gw->grid);
        else {
            if (op == GLYPHEDIT_DRAW) {
                /*
                 * Attach the selected part of the bitmap.
                 */
                bdf_attach_selection(gw->grid);

                /*
                 * Erase the selected rectangle.
                 */
                for (sy = y; sy < y + ht; sy++) {
                    for (sx = x; sx < x + wd; sx++)
                      glyphedit_draw_pixel(gw, sx, sy, FALSE);
                }
                bdf_lose_selection(gw->grid);

            }
            gw->op = op;
        }
        gw->pending_op = GLYPHEDIT_NONE;
        glyphedit_signal_glyph_change(gw);
    } else {
        if (op == GLYPHEDIT_MOVE || op == GLYPHEDIT_COPY) {
            gw->op = GLYPHEDIT_SELECT;
            gw->pending_op = op;
        } else {
            gw->op = op;
            gw->pending_op = GLYPHEDIT_NONE;
        }
    }
}

void
glyphedit_set_color(Glyphedit *gw, gint idx)
{
    GlypheditSignalInfo si;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));
    g_return_if_fail(gw->grid != 0);

    if (gw->grid) {
        if (idx <= 0)
          idx = (1 << gw->grid->bpp);
        else if (idx > (1 << gw->grid->bpp))
          idx = 1;
    } else
      idx = 1;

    if (idx != gw->cidx) {
        si.reason = GLYPHEDIT_COLOR_CHANGE;
        si.color = idx;
        g_signal_emit(G_OBJECT(gw), glyphedit_signals[COLOR_CHANGE], 0, &si);
    }

    gw->cidx = idx;
}

void
glyphedit_paste_selection(Glyphedit *gw)
{
    GtkWidget *w = GTK_WIDGET(gw);
    GdkWindow *win;
    GdkAtom atype;
    gint aformat, nitems;
    guchar *data, *bp;
    gint16 sx, sy, x, y, wd, ht;
    bdf_metrics_t metrics;
    bdf_bitmap_t image;
    GlypheditSignalInfo si;
    
    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));
    g_return_if_fail(gw->grid != NULL);

    if ((win = gdk_selection_owner_get(GLYPHEDIT_CLIPBOARD)) == 0) {
        gdk_selection_owner_set(gtk_widget_get_window(w), GLYPHEDIT_CLIPBOARD,
                                GDK_CURRENT_TIME, FALSE);
        win = gtk_widget_get_window(w);
    }

    nitems = 0;
    gdk_property_get(win, GLYPHEDIT_CLIPBOARD, GLYPHEDIT_BITMAP,
                     0, 10240, FALSE, &atype, &aformat, &nitems, &data);

    if (win != gtk_widget_get_window(w))
      gdk_selection_owner_set(gtk_widget_get_window(w), GLYPHEDIT_CLIPBOARD,
                              GDK_CURRENT_TIME, FALSE);

    if (nitems > 0) {
        /*
         * Got a bitmap.
         */

        if (bdf_has_selection(gw->grid, &x, &y, &wd, &ht)) {
            /*
             * This widget already has a selection, so release it.
             */
            if (gw->op != GLYPHEDIT_SELECT)
              bdf_attach_selection(gw->grid);

            for (sy = y; sy < y + ht; sy++) {
                for (sx = x; sx < x + wd; sx++)
                  glyphedit_draw_pixel(gw, sx, sy, FALSE);
            }
            bdf_lose_selection(gw->grid);
        }

        bp = data;

        if (!bdf_little_endian()) {
            image.bpp = (*bp++ << 8) & 0xff00;
            image.bpp |= *bp++;
            image.width = (*bp++ << 8) & 0xff00;
            image.width |= *bp++;
            image.height = (*bp++ << 8) & 0xff00;
            image.height |= *bp++;
        } else {
            image.bpp = *bp++ & 0xff;
            image.bpp |= (*bp++ << 8) & 0xff00;
            image.width = *bp++ & 0xff;
            image.width |= (*bp++ << 8) & 0xff00;
            image.height = *bp++ & 0xff;
            image.height |= (*bp++ << 8) & 0xff00;
        }

        image.bytes = (((image.width * image.bpp) + 7) >> 3) * image.height;
        image.bitmap = bp;

        image.x = gw->last_x;
        image.y = gw->last_y;

        /*
         * If the bitmap being pasted is larger than the current grid, then
         * resize the grid before doing anything else.
         */
        glyphedit_get_font_metrics(gw, &metrics);
        if (image.width > metrics.width || image.height > metrics.height) {
            /*
             * Adjust the insert position on the X axis if necessary.
             */
            if (image.width > metrics.width)
              image.x = gw->grid->base_x +
                  gw->grid->font_bbx.x_offset;
            /*
             * Adjust the insert position on the Y axis and the ascent if
             * necessary.
             */
            if (image.height > metrics.height) {
                image.y = 0;
                metrics.ascent = image.height - gw->grid->font_bbx.descent;
            }
            metrics.width = image.width;
            metrics.height = image.height;
            glyphedit_set_metrics(gw, &metrics);
        }

        /*
         * Set the selection in the grid.
         */
        bdf_add_selection(gw->grid, &image);

        /*
         * Now update the grid.
         */
        if (bdf_has_selection(gw->grid, &x, &y, &wd, &ht)) {
            for (sy = y; sy < y + ht; sy++) {
                for (sx = x; sx < x + wd; sx++)
                  glyphedit_draw_pixel(gw, sx, sy, TRUE);
            }
        }

        /*
         * Set up and call the image update.
         */
        glyphedit_signal_glyph_change(gw);

        /*
         * Free up the original value passed.
         */
        g_free(data);

        /*
         * Alert the client that the widget is changing to the MOVE
         * operation.
         */
        si.reason = GLYPHEDIT_OPERATION_CHANGE;
        si.operation = GLYPHEDIT_MOVE;
        g_signal_emit(G_OBJECT(gw), glyphedit_signals[OPERATION_CHANGE],
                      0, &si);

        /*
         * Make sure the widget goes into MOVE mode at this point.
         * This allows the user to position what was pasted without
         * destroying the glyph bitmap that was already there.
         */
        if (gw->op != GLYPHEDIT_MOVE) {
            gw->op = GLYPHEDIT_MOVE;
            gw->pending_op = GLYPHEDIT_NONE;
        }

        /*
         * Last, recopy the selection to the clipboard because changing owners
         * causes the data to be lost.
         */
        glyphedit_copy_selection(gw);
    }
}

void
glyphedit_select_all(Glyphedit *gw)
{
    gint16 tx, ty, sx, sy, wd, ht;
    GlypheditSignalInfo si;

    g_return_if_fail(gw != NULL);
    g_return_if_fail(IS_GLYPHEDIT(gw));
    g_return_if_fail(gw->grid != NULL);

    /*
     * If a selection already exists, clear it.
     */
    if (bdf_has_selection(gw->grid, &sx, &sy, &wd, &ht)) {
        if (gw->op != GLYPHEDIT_SELECT)
          bdf_attach_selection(gw->grid);

        for (ty = sy; ty < sy + ht; ty++) {
            for (tx = sx; tx < sx + wd; tx++)
              glyphedit_draw_pixel(gw, tx, ty, FALSE);
        }
        bdf_lose_selection(gw->grid);
    }

    wd = gw->grid->glyph_bbx.width;
    ht = gw->grid->glyph_bbx.height;

    sx = gw->sel_start.x = gw->grid->glyph_x;
    sy = gw->sel_start.y = gw->grid->glyph_y;
    gw->sel_end.x = gw->grid->glyph_x + wd;
    gw->sel_end.y = gw->grid->glyph_y + ht;

    /*
     * Gain control of the GLYPHEDIT_CLIPBOARD atom.
     */
    glyphedit_own_clipboard(gw);

    bdf_set_selection(gw->grid, sx, sy, wd, ht);
    bdf_detach_selection(gw->grid);

    for (ty = sy; ty < sy + ht; ty++) {
        for (tx = sx; tx < sx + wd; tx++)
          glyphedit_draw_pixel(gw, tx, ty, TRUE);
    }

    /*
     * Alert the client that the widget is changing to the MOVE
     * operation.
     */
    si.reason = GLYPHEDIT_OPERATION_CHANGE;
    si.operation = GLYPHEDIT_MOVE;
    g_signal_emit(G_OBJECT(gw), glyphedit_signals[OPERATION_CHANGE],
                  0, &si);

    /*
     * Make sure the widget goes into MOVE mode at this point.
     * This allows the user to position what was pasted without
     * destroying the glyph bitmap that was already there.
     */
    if (gw->op != GLYPHEDIT_MOVE) {
        gw->op = GLYPHEDIT_MOVE;
        gw->pending_op = GLYPHEDIT_NONE;
    }
}

gint32
glyphedit_encoding(Glyphedit *gw)
{
    g_return_val_if_fail(gw != NULL, -1);
    g_return_val_if_fail(IS_GLYPHEDIT(gw), -1);
    g_return_val_if_fail(gw->grid != NULL, -1);

    return (gw->grid->unencoded) ? -1 : gw->grid->encoding;
}

static void
glyphedit_get_pointer_coord(Glyphedit *gw, gint16 ex, gint16 ey,
                            gint16 *px, gint16 *py)
{
    GtkWidget *w = GTK_WIDGET(gw);
    gint16 x, y, wd, ht;
    GtkAllocation all;

    wd = (gw->pixel_size + 4) * gw->grid->grid_width;
    ht = (gw->pixel_size + 4) * gw->grid->grid_height;

    gtk_widget_get_allocation(w, &all);

    /*
     * Need the plus 1 to account for the outer rectangle.
     */
    x = (all.width >> 1) - (wd >> 1) + 1;
    y = (all.height >> 1) - (ht >> 1) + 1;

    if (ex < x || ex > x + wd)
      *px = -1;
    else
      *px = (ex - x) / (gw->pixel_size + 4);

    if (ey < y || ey > y + ht)
      *py = -1;
    else
      *py = (ey - y) / (gw->pixel_size + 4);

    /*
     * Adjust for a possible overrun off the edges of the grid.
     */
    if (*px >= gw->grid->grid_width)
      *px = gw->grid->grid_width - 1;
    if (*py >= gw->grid->grid_height)
      *py = gw->grid->grid_height - 1;
}

static gboolean
glyphedit_in_selection(Glyphedit *gw, gint16 x, gint16 y)
{
    return (((gw->sel_start.y <= y && y <= gw->sel_end.y) ||
             (gw->sel_end.y <= y && y <= gw->sel_start.y)) &&
            ((gw->sel_start.x <= x && x <= gw->sel_end.x) ||
             (gw->sel_end.x <= x && x <= gw->sel_start.x)))
        ? TRUE : FALSE;
}

static gboolean
glyphedit_in_intersection(Glyphedit *gw, gint16 ix, gint16 iy,
                          gint16 x, gint16 y)
{
    return (((gw->sel_start.y <= y && y <= iy) ||
             (iy <= y && y <= gw->sel_start.y)) &&
            ((gw->sel_start.x <= x && x <= ix) ||
             (ix <= x && x <= gw->sel_start.x))) ? TRUE : FALSE;
}

static void
glyphedit_update_selection(Glyphedit *gw, gint16 x, gint16 y, gboolean set)
{
    gint16 wd, ht;

    for (ht = 0; ht < gw->grid->grid_height; ht++) {
        for (wd = 0; wd < gw->grid->grid_width; wd++) {
            if (glyphedit_in_intersection(gw, x, y, wd, ht) == FALSE &&
                glyphedit_in_selection(gw, wd, ht) == TRUE)
              /*
               * Clear or set the pixel.
               */
              glyphedit_draw_pixel(gw, wd, ht, set);
        }
    }
}

static gboolean
glyphedit_button_press(GtkWidget *w, GdkEventButton *event)
{
    Glyphedit *gw;
    gint16 x, y, sx, sy, tx, ty, wd, ht;
    gboolean changed;

    gw = GLYPHEDIT(w);

    glyphedit_get_pointer_coord(gw, (gint16) event->x, (gint16) event->y,
                                &x, &y);

    if (event->button == 2 && (event->state & GDK_SHIFT_MASK)) {
        /*
         * Paste.
         */
        glyphedit_paste_selection(gw);
        return FALSE;
    }

    changed = FALSE;
    if (gw->op == GLYPHEDIT_DRAW) {
        switch (event->button) {
          case 1:
            if ((changed = bdf_grid_set_pixel(gw->grid, x, y, gw->cidx)))
              glyphedit_draw_pixel(gw, x, y, FALSE);
            break;
          case 2:
            if ((changed = bdf_grid_invert_pixel(gw->grid, x, y, gw->cidx)))
              glyphedit_draw_pixel(gw, x, y, FALSE);
            break;
          case 3:
            if ((changed = bdf_grid_clear_pixel(gw->grid, x, y)))
              glyphedit_draw_pixel(gw, x, y, FALSE);
            break;
        }
        if (changed == TRUE)
          glyphedit_signal_glyph_change(gw);
    } else if (gw->op == GLYPHEDIT_SELECT) {
        /*
         * If a selection already exists, clear it.
         */
        if (bdf_has_selection(gw->grid, &sx, &sy, &wd, &ht)) {
            if (gw->pending_op != GLYPHEDIT_NONE)
              bdf_attach_selection(gw->grid);

            for (ty = sy; ty < sy + ht; ty++) {
                for (tx = sx; tx < sx + wd; tx++)
                  glyphedit_draw_pixel(gw, tx, ty, FALSE);
            }
            bdf_lose_selection(gw->grid);
        }

        /*
         * Select the pixel at the point and initialize the selection
         * rectangle.
         */
        glyphedit_draw_pixel(gw, x, y, TRUE);

        gw->sel_start.x = gw->sel_end.x = x;
        gw->sel_start.y = gw->sel_end.y = y;
    } else {
        /*
         * Check to see if this is Button3 and a selection exists.  If so,
         * then copy the selection to the clipboard and return.
         */
        if (event->button == 3 &&
            bdf_has_selection(gw->grid, &sx, &sy, &wd, &ht)) {
            glyphedit_copy_selection(gw);
            gw->last_x = x;
            gw->last_y = y;
            return FALSE;
        }

        /*
         * The operation is one of move or copy.  If the button is clicked
         * outside the selection, remove the selection and start over.
         */
        if (bdf_has_selection(gw->grid, &sx, &sy, &wd, &ht) &&
            !bdf_in_selection(gw->grid, x, y, 0)) {

            if (gw->op != GLYPHEDIT_SELECT)
              bdf_attach_selection(gw->grid);

            for (ty = sy; ty < sy + ht; ty++) {
                for (tx = sx; tx < sx + wd; tx++)
                  glyphedit_draw_pixel(gw, tx, ty, FALSE);
            }
            bdf_lose_selection(gw->grid);

            gw->pending_op = gw->op;
            gw->op = GLYPHEDIT_SELECT;

            /*
             * Select the pixel at the point and initialize the selection
             * rectangle.
             */
            glyphedit_draw_pixel(gw, x, y, TRUE);

            gw->sel_start.x = gw->sel_end.x = x;
            gw->sel_start.y = gw->sel_end.y = y;
        }
    }

    /*
     * Set the last coordinate to the point just handled.
     */
    gw->last_x = x;
    gw->last_y = y;
    gw->mouse_down = TRUE;

    return FALSE;
}

static gboolean
glyphedit_button_release(GtkWidget *w, GdkEventButton *event)
{
    Glyphedit *gw;
    gint16 sx, sy, ex, ey;

    gw = GLYPHEDIT(w);

    sx = MIN(gw->sel_start.x, gw->sel_end.x);
    ex = MAX(gw->sel_start.x, gw->sel_end.x);
    sy = MIN(gw->sel_start.y, gw->sel_end.y);
    ey = MAX(gw->sel_start.y, gw->sel_end.y);

    if (gw->op == GLYPHEDIT_SELECT) {
        if (sx == ex && sy == ey)
          glyphedit_draw_pixel(gw, gw->sel_start.x, gw->sel_start.y, FALSE);
        else {
            /*
             * Gain control of the GLYPHEDIT_CLIPBOARD atom.
             */
            glyphedit_own_clipboard(gw);

            bdf_set_selection(gw->grid, sx, sy, (ex - sx) + 1, (ey - sy) + 1);

            /*
             * Switch to a move/copy operations if necessary.
             */
            if (gw->pending_op != GLYPHEDIT_NONE) {
                gw->op = gw->pending_op;
                gw->pending_op = GLYPHEDIT_NONE;
                /*
                 * If the pending operation is a move, then make sure the
                 * selection is detached.
                 */
                if (gw->op == GLYPHEDIT_MOVE)
                  bdf_detach_selection(gw->grid);
            }
        }
    }
    gw->mouse_down = FALSE;

    return FALSE;
}

static gboolean
glyphedit_motion_notify(GtkWidget *w, GdkEventMotion *event)
{
    Glyphedit *gw;
    gboolean changed;
    gint16 x, y, ix, iy;
    GlypheditSignalInfo si;

    gw = GLYPHEDIT(w);

    glyphedit_get_pointer_coord(gw, (gint16) event->x, (gint16) event->y,
                                &x, &y);

    /*
     * Return if the mouse is off the edges of the grid or the mouse is still
     * on the same point as the last one.
     */
    if (x < 0 || y < 0 || (x == gw->last_x && y == gw->last_y))
      return FALSE;

    si.reason = GLYPHEDIT_POINTER_MOVED;
    si.x = x - gw->grid->base_x;
    si.y = -(y - gw->grid->base_y) - 1;
    si.color = bdf_grid_color_at(gw->grid, x, y);
    g_signal_emit(G_OBJECT(gw), glyphedit_signals[POINTER_MOVED],
                  0, &si);

    ix = gw->last_x;
    iy = gw->last_y;

    gw->last_x = x;
    gw->last_y = y;

    /*
     * If the event is a simple motion event with no button being pressed,
     * then simply return at this point.
     */
    if (!gw->mouse_down ||
        !(event->state & (GDK_BUTTON1_MASK|GDK_BUTTON2_MASK|GDK_BUTTON3_MASK)))
      return FALSE;

    changed = FALSE;
    if (gw->op == GLYPHEDIT_DRAW) {
        /*
         * Drawing.
         */
        if (event->state & GDK_BUTTON1_MASK) {
            if ((changed = bdf_grid_set_pixel(gw->grid, x, y, gw->cidx)))
              glyphedit_draw_pixel(gw, x, y, FALSE);
        } else if (event->state & GDK_BUTTON2_MASK) {
            if ((changed = bdf_grid_invert_pixel(gw->grid, x, y, gw->cidx)))
              glyphedit_draw_pixel(gw, x, y, FALSE);
        } else if (event->state & GDK_BUTTON3_MASK) {
            if ((changed = bdf_grid_clear_pixel(gw->grid, x, y)))
              glyphedit_draw_pixel(gw, x, y, FALSE);
        }

        /*
         * If one of the pixels changed, then call the callback.
         */
        if (changed)
          glyphedit_signal_glyph_change(gw);
    } else if (gw->op == GLYPHEDIT_SELECT) {
        /*
         * Determine the other point on the intersection rectangle.
         */
        ix = gw->sel_start.x;
        iy = gw->sel_start.y;

        if (x > ix)
          ix = MIN(gw->sel_end.x, x);
        else if (x < ix)
          ix = MAX(gw->sel_end.x, x);

        if (y > iy)
          iy = MIN(gw->sel_end.y, y);
        else if (y < iy)
          iy = MAX(gw->sel_end.y, y);

        /*
         * Clear the pixels outside the intersection of the old selection
         * rectangle and the new selection rectangle.
         */
        glyphedit_update_selection(gw, ix, iy, FALSE);

        /*
         * Set the new endpoint of the selection rectangle.
         */
        gw->sel_end.x = x;
        gw->sel_end.y = y;

        /*
         * Set all pixels outside the intersection of the old selection
         * rectangle and the new selection rectangle, but inside the new
         * selection rectangle.
         */
        glyphedit_update_selection(gw, ix, iy, TRUE);
    } else {
        /*
         * A move or copy is in progress.
         */
        if (bdf_has_selection(gw->grid, 0, 0, 0, 0) &&
            bdf_grid_shift(gw->grid, x - ix, y - iy)) {
            glyphedit_draw_glyph(gw);
            glyphedit_signal_glyph_change(gw);
        }
    }

    return FALSE;
}

static gboolean
glyphedit_key_press(GtkWidget *w, GdkEventKey *event)
{
    gboolean ret = FALSE;

    switch (event->keyval) {
      case GDK_Left:
      case GDK_KP_Left:
        glyphedit_shift_glyph(GLYPHEDIT(w), -1, 0);
        break;
      case GDK_Right:
      case GDK_KP_Right:
        glyphedit_shift_glyph(GLYPHEDIT(w), 1, 0);
        break;
      case GDK_Up:
      case GDK_KP_Up:
        /*
         * For some reason, the Up arrow causes the focus to change to
         * other widgets. Returning TRUE insures that the up arrow works
         * as expected.
         */
        glyphedit_shift_glyph(GLYPHEDIT(w), 0, -1);
        ret = TRUE;
        break;
      case GDK_Down:
      case GDK_KP_Down:
        glyphedit_shift_glyph(GLYPHEDIT(w), 0, 1);
        break;
      case GDK_Delete:
      case GDK_BackSpace:
        glyphedit_cut_selection(GLYPHEDIT(w));
        break;
      case GDK_9:
      case GDK_KP_9:
        glyphedit_rotate_glyph(GLYPHEDIT(w), -90);
      case GDK_0:
      case GDK_KP_0:
        glyphedit_rotate_glyph(GLYPHEDIT(w), 90);
        break;
      case GDK_minus:
      case GDK_KP_Subtract:
        glyphedit_flip_glyph(GLYPHEDIT(w), GTK_ORIENTATION_HORIZONTAL);
        break;
      case GDK_equal:
      case GDK_KP_Equal:
        glyphedit_flip_glyph(GLYPHEDIT(w), GTK_ORIENTATION_VERTICAL);
        break;
      case GDK_comma:
      case GDK_Z:
      case GDK_z:
        /* Change to a lighter color. */
        glyphedit_set_color(GLYPHEDIT(w), GLYPHEDIT(w)->cidx - 1);
        break;
      case GDK_period:
      case GDK_X:
      case GDK_x:
        /* Change to a darker color. */
        glyphedit_set_color(GLYPHEDIT(w), GLYPHEDIT(w)->cidx + 1);
        break;
    }

    return ret;
}

static gboolean
glyphedit_key_release(GtkWidget *w, GdkEventKey *event)
{
    return FALSE;
}

static void
glyphedit_class_init(gpointer g_class, gpointer class_data)
{
    GObjectClass *gocp = G_OBJECT_CLASS(g_class);
    GtkObjectClass *ocp = GTK_OBJECT_CLASS(g_class);
    GtkWidgetClass *wcp = GTK_WIDGET_CLASS(g_class);

    /*
     * Set the class global variables.
     */
    parent_class = g_type_class_peek_parent(g_class);

    ocp->destroy = glyphedit_destroy;

    gocp->set_property = glyphedit_set_property;
    gocp->get_property = glyphedit_get_property;
    gocp->finalize = glyphedit_finalize;

    /*
     * Add argument (a.k.a. resource) types.
     */
    g_object_class_install_property(gocp, GLYPH_GRID,
                                    g_param_spec_pointer("glyphGrid",
                                                       _("Glyph Grid"),
                                                       _("The glyph in a grid structure."),
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PIXEL_SIZE,
                                    g_param_spec_uint("pixelSize",
                                                      _("Pixel Size"),
                                                      _("The number of pixels to use to draw one grid pixel."),
                                                      1,
                                                      20,
                                                      10,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gocp, SHOW_X_HEIGHT,
                                    g_param_spec_boolean("showXHeight",
                                                         _("Show X Height"),
                                                         _("Draw a line at the x height."),
                                                         FALSE,
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, SHOW_CAP_HEIGHT,
                                    g_param_spec_boolean("showCapHeight",
                                                         _("Show Cap Height"),
                                                         _("Draw a line at the cap height."),
                                                         FALSE,
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, OPERATION,
                                    g_param_spec_enum("operation",
                                                      _("Edit Operation"),
                                                      _("Glyph edit operation."),
                                                      glyphedit_get_operation_type(),
                                                      GLYPHEDIT_DRAW,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gocp, COLOR_LIST,
                                    g_param_spec_pointer("colorList",
                                                         _("Color list"),
                                                         _("Colors to be used for glyphs having bits-per-pixel > 1."),
                                                         G_PARAM_READWRITE));


    /*
     * Add the signals these objects emit.
     */
    glyphedit_signals[GLYPH_MODIFIED] =
        g_signal_new("glyph-modified",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(GlypheditClass, glyph_modified),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    glyphedit_signals[POINTER_MOVED] =
        g_signal_new("pointer-moved",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(GlypheditClass, pointer_moved),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    glyphedit_signals[OPERATION_CHANGE] =
        g_signal_new("operation-change",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(GlypheditClass, operation_change),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    glyphedit_signals[COLOR_CHANGE] =
        g_signal_new("color-change",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(GlypheditClass, color_change),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    /*
     * Set all the functions for handling events for objects of this class.
     */
    wcp->size_allocate = glyphedit_actual_size;
    wcp->realize = glyphedit_realize;
#if GTK_CHECK_VERSION(3, 0, 0)
    wcp->get_preferred_width = glyphedit_get_preferred_width;
    wcp->get_preferred_height = glyphedit_get_preferred_height;
    wcp->draw = glyphedit_draw;
#else
    wcp->size_request = glyphedit_preferred_size;
    wcp->expose_event = glyphedit_expose;
#endif
    wcp->focus_out_event = glyphedit_focus_out;
    wcp->button_press_event = glyphedit_button_press;
    wcp->button_release_event = glyphedit_button_release;
    wcp->motion_notify_event = glyphedit_motion_notify;
    wcp->key_press_event = glyphedit_key_press;
    wcp->key_release_event = glyphedit_key_release;
}

GType
glyphedit_get_type(void)
{
    static GType glyphedit_type = 0;

    if (!glyphedit_type) {
        static const GTypeInfo glyphedit_info = {
            sizeof(GlypheditClass),
            0,
            0,
            glyphedit_class_init,
            0,
            0,
            sizeof(Glyphedit),
            0,
            glyphedit_init,
            0,
        };

        glyphedit_type = g_type_register_static(GTK_TYPE_WIDGET, "Glyphedit",
                                                &glyphedit_info, 0);
    }

    return glyphedit_type;
}

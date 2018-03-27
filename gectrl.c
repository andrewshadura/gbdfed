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

#include <gtk/gtk.h>
#include "gectrl.h"
#include "gectrlbmaps.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(s) dgettext(GETTEXT_PACKAGE,s)
#else
#define _(s) (s)
#endif

/*
 * All the pixmaps are 16x16.
 */
#define BMAP_DIM 16

#define GEC_TOGGLE_SIZE 38
#define GEC_BUTTON_SIZE 33

/*
 * Properties of this widget.
 */
enum {
    PROP_0 = 0,
    TIP_LABEL,
    GLYPH_IMAGE,
    COLOR_LIST
};

/*
 * Signals this widget emits.
 */
enum {
    ACTIVATE = 0
};

static GtkDrawingAreaClass *parent_class = 0;
static guint gecontrol_signals[ACTIVATE + 1];

#define GEC_DRAW_TOGGLE     0
#define GEC_MOVE_TOGGLE     1
#define GEC_COPY_TOGGLE     2
#define GEC_FLIPH_BUTTON    3
#define GEC_FLIPV_BUTTON    4
#define GEC_SHEAR_BUTTON    5
#define GEC_RLEFT_BUTTON    6
#define GEC_RRIGHT_BUTTON   7
#define GEC_ROTATE_BUTTON   8
#define GEC_ULEFT_BUTTON    9
#define GEC_UP_BUTTON       10
#define GEC_URIGHT_BUTTON   11
#define GEC_LEFT_BUTTON     12
#define GEC_RIGHT_BUTTON    13
#define GEC_DLEFT_BUTTON    14
#define GEC_DOWN_BUTTON     15
#define GEC_DRIGHT_BUTTON   16
#define GEC_GLYPH_IMAGE     17

/*
 * These are encoded in UTF-8.
 */
static guchar *help_strings[18] = {
    (guchar *) "Draw",
    (guchar *) "Move",
    (guchar *) "Copy",
    (guchar *) "Flip Horizontally",
    (guchar *) "Flip Vertically",
    (guchar *) "Shear ±45°",
    (guchar *) "Rotate -90°",
    (guchar *) "Rotate +90°",
    (guchar *) "Rotate ±359°",
    (guchar *) "Shift Up+Left",
    (guchar *) "Shift Up",
    (guchar *) "Shift Up+Right",
    (guchar *) "Shift Left",
    (guchar *) "Shift Right",
    (guchar *) "Shift Down+Left",
    (guchar *) "Shift Down",
    (guchar *) "Shift Down+Right",
    (guchar *) "Glyph Image",
};

/*
 * Position all the buttons in the space provided.
 */
static void
gecontrol_position_buttons(GtkWidget *w)
{
    GEControl *ge = GECONTROL(w);
    gint x, y, sx, sy, ix, dx, i, j, v, wd, ht;
    GdkPoint points[5];
    GtkAllocation all;

    dx = 0;

    /*
     * Determine the starting x and y coordinates, centered in the
     * window. Modify later to make room for the color strip on the left side
     * of the window.
     */

    gtk_widget_get_allocation(w, &all);
    sx = (all.width >> 1) - (((GEC_TOGGLE_SIZE * 3) + 6) >> 1);
    v = (GEC_TOGGLE_SIZE + (GEC_BUTTON_SIZE * 5)) + 15;

    if (ge->gimage != 0) {
        /*
         * The addition of 7 includes a space of 3 between the glyph image and
         * the row of toggles, and 2 pixels on the top and bottom of the glyph
         * image for a box that will be drawn around it and a single pixel
         * between the edge of the box and the actual glyph image.
         */
        v += ge->gimage->height + 7;

        /*
         * Calculate a horizontal offset for the toggles and buttons in the
         * case of 8 bits per pixel. The color selection square is 128x128 so
         * everything else needs to move over to accomodate it.
         */
        if (ge->gimage->bpp == 8 && sx < 128 + 8)
          dx = (128 + 8) - sx;
    }

    sx += dx;
    sy = (all.height >> 1) - (v >> 1);

    /*
     * Position the glyph image first if one is present.
     */
    if (ge->gimage != 0) {
        /*
         * The addition of 2 is for the box and an empty row around the glyph
         * image.
         */
        ix = ((all.width >> 1)-((ge->gimage->width + 2) >> 1)) + dx;

        if (ge->buttons[GEC_GLYPH_IMAGE].region == NULL) {
            /* Top left. */
            points[0].x = points[4].x = ix;
            points[0].y = points[4].y = sy;
            /* Top right. */
            points[1].x = ix + ge->gimage->width + 4;
            points[1].y = points[0].y;
            /* Bottom right. */
            points[2].x = points[1].x;
            points[2].y = sy + ge->gimage->height + 4;
            /* Bottom left. */
            points[3].x = points[0].x;
            points[3].y = points[2].y;
            ge->buttons[GEC_GLYPH_IMAGE].region =
                gdk_region_polygon(points, 4, GDK_WINDING_RULE);
        } else
          gdk_region_offset(ge->buttons[GEC_GLYPH_IMAGE].region,
                            ix - ge->buttons[GEC_GLYPH_IMAGE].x,
                            sy - ge->buttons[GEC_GLYPH_IMAGE].y);

        ge->gimage->x = ge->buttons[GEC_GLYPH_IMAGE].x = ix;
        ge->gimage->y = ge->buttons[GEC_GLYPH_IMAGE].y = sy;

        sy += ge->gimage->height + 7;
    }

    x = sx;
    y = sy;

    /*
     * Prep the points for creating regions for the toggle buttons.
     */
    points[0].x = points[4].x = x;
    points[0].y = points[4].y = y + (GEC_TOGGLE_SIZE >> 1);
    points[1].x = x + (GEC_TOGGLE_SIZE >> 1);
    points[1].y = y;
    points[2].x = x + GEC_TOGGLE_SIZE;
    points[2].y = points[0].y;
    points[3].x = points[1].x;
    points[3].y = y + GEC_TOGGLE_SIZE;

    /*
     * Position the toggle buttons.
     */
    for (i = 0; i < GEC_FLIPH_BUTTON; i++) {
        if (ge->buttons[i].region == NULL)
          ge->buttons[i].region =
              gdk_region_polygon(points, 4, GDK_WINDING_RULE);
        else
          gdk_region_offset(ge->buttons[i].region,
                            x - ge->buttons[i].x, y - ge->buttons[i].y);

        ge->buttons[i].x = x;
        ge->buttons[i].y = y;

        x += GEC_TOGGLE_SIZE + 3;

        for (j = 0; j < 5; j++)
          points[j].x += GEC_TOGGLE_SIZE + 3;
    }

    /*
     * Recalculate the starting x position based on the button size instead
     * of the toggle size.
     */
    sx = ((all.width >> 1)-(((GEC_BUTTON_SIZE * 3) + 6) >> 1)) + dx;

    y += GEC_TOGGLE_SIZE + 3;

    /*
     * Now set up the points for the buttons.
     */
    points[0].x = sx;
    points[0].y = y;
    points[1].x = sx + GEC_BUTTON_SIZE;
    points[1].y = points[0].y;
    points[2].x = points[1].x;
    points[2].y = y + GEC_BUTTON_SIZE;
    points[3].x = points[0].x;
    points[3].y = points[2].y;

    /*
     * Position the first row of buttons.
     */
    for (x = sx; i < GEC_RLEFT_BUTTON; i++) {
        if (ge->buttons[i].region == NULL)
          ge->buttons[i].region =
              gdk_region_polygon(points, 4, GDK_WINDING_RULE);
        else
          gdk_region_offset(ge->buttons[i].region,
                            x - ge->buttons[i].x, y - ge->buttons[i].y);
        ge->buttons[i].x = x;
        ge->buttons[i].y = y;

        x += GEC_BUTTON_SIZE + 3;

        for (j = 0; j < 4; j++)
          points[j].x += GEC_BUTTON_SIZE + 3;
    }

    /*
     * Reset the x coordinate for the regions.
     */
    points[0].x = points[3].x = sx;
    points[1].x = points[2].x = sx + GEC_BUTTON_SIZE;

    y += GEC_BUTTON_SIZE + 3;

    for (j = 0; j < 4; j++)
      points[j].y += GEC_BUTTON_SIZE + 3;

    /*
     * Position second row of buttons.
     */
    for (x = sx; i < GEC_ULEFT_BUTTON; i++) {
        if (ge->buttons[i].region == NULL)
          ge->buttons[i].region =
              gdk_region_polygon(points, 4, GDK_WINDING_RULE);
        else
          gdk_region_offset(ge->buttons[i].region,
                            x - ge->buttons[i].x, y - ge->buttons[i].y);
        ge->buttons[i].x = x;
        ge->buttons[i].y = y;

        x += GEC_BUTTON_SIZE + 3;

        for (j = 0; j < 4; j++)
          points[j].x += GEC_BUTTON_SIZE + 3;
    }

    /*
     * Reset the x coordinate for the regions.
     */
    points[0].x = points[3].x = sx;
    points[1].x = points[2].x = sx + GEC_BUTTON_SIZE;

    y += GEC_BUTTON_SIZE + 3;

    for (j = 0; j < 4; j++)
      points[j].y += GEC_BUTTON_SIZE + 3;

    /*
     * Position third row of buttons.
     */
    for (x = sx; i < GEC_LEFT_BUTTON; i++) {
        if (ge->buttons[i].region == NULL)
          ge->buttons[i].region =
              gdk_region_polygon(points, 4, GDK_WINDING_RULE);
        else
          gdk_region_offset(ge->buttons[i].region,
                            x - ge->buttons[i].x, y - ge->buttons[i].y);
        ge->buttons[i].x = x;
        ge->buttons[i].y = y;

        x += GEC_BUTTON_SIZE + 3;

        for (j = 0; j < 4; j++)
          points[j].x += GEC_BUTTON_SIZE + 3;
    }

    /*
     * Reset the x coordinate for the regions.
     */
    points[0].x = points[3].x = sx;
    points[1].x = points[2].x = sx + GEC_BUTTON_SIZE;

    x = sx;
    y += GEC_BUTTON_SIZE + 3;

    for (j = 0; j < 4; j++)
      points[j].y += GEC_BUTTON_SIZE + 3;

    /*
     * Set the coordinates of the LEFT and RIGHT buttons.
     */
    if (ge->buttons[i].region == NULL)
      ge->buttons[i].region =
          gdk_region_polygon(points, 4, GDK_WINDING_RULE);
    else
      gdk_region_offset(ge->buttons[i].region,
                        x - ge->buttons[i].x, y - ge->buttons[i].y);
    ge->buttons[i].x = x;
    ge->buttons[i++].y = y;

    x += (GEC_BUTTON_SIZE + 3) * 2;

    for (j = 0; j < 4; j++)
      points[j].x += (GEC_BUTTON_SIZE + 3) * 2;

    if (ge->buttons[i].region == NULL)
      ge->buttons[i].region =
          gdk_region_polygon(points, 4, GDK_WINDING_RULE);
    else
      gdk_region_offset(ge->buttons[i].region,
                        x - ge->buttons[i].x, y - ge->buttons[i].y);
    ge->buttons[i].x = x;
    ge->buttons[i++].y = y;

    /*
     * Reset the x coordinate for the regions.
     */
    points[0].x = points[3].x = sx;
    points[1].x = points[2].x = sx + GEC_BUTTON_SIZE;

    y += GEC_BUTTON_SIZE + 3;

    for (j = 0; j < 4; j++)
      points[j].y += GEC_BUTTON_SIZE + 3;

    for (x = sx; i < GEC_GLYPH_IMAGE; i++) {
        if (ge->buttons[i].region == NULL)
          ge->buttons[i].region =
              gdk_region_polygon(points, 4, GDK_WINDING_RULE);
        else
          gdk_region_offset(ge->buttons[i].region,
                            x - ge->buttons[i].x, y - ge->buttons[i].y);
        ge->buttons[i].x = x;
        ge->buttons[i].y = y;

        x += GEC_BUTTON_SIZE + 3;

        for (j = 0; j < 4; j++)
          points[j].x += GEC_BUTTON_SIZE + 3;
    }

    /*
     * Now position the color spots if they are needed.
     */

    if (ge->gimage && ge->gimage->bpp > 1) {
        if (ge->gimage->bpp == 2 || ge->gimage->bpp == 4) {
            /*
             * The starting horizontal position is 1/2 way between the left
             * edge of the window and the left edge of the buttons. The
             * starting vertical position is centered on the left edge of the
             * buttons.
             */
            sx = (sx >> 1) - (8 >> 1);
            y = (8 * (1 << ge->gimage->bpp)) + ((1 << ge->gimage->bpp) - 1);
            sy = ge->buttons[GEC_FLIPH_BUTTON].y +
                ((all.height-ge->buttons[GEC_FLIPH_BUTTON].y)>>1) -
                (y >> 1);
            wd = 8;
            ht = 8 * (1 << ge->gimage->bpp);
        } else {
            sx = (sx >> 1) - (128 >> 1);
            sy = ge->buttons[GEC_FLIPH_BUTTON].y +
                ((all.height-ge->buttons[GEC_FLIPH_BUTTON].y)>>1) -
                (128 >> 1);
            wd = ht = 128;
        }

        /*
         * Initialize the points for the spot region.
         */

        /* Top left. */
        points[0].x = points[4].x = sx;
        points[0].y = points[4].y = sy;
        /* Top right. */
        points[1].x = points[0].x + wd;
        points[1].y = points[0].y;
        /* Bottom right. */
        points[2].x = points[1].x;
        points[2].y = points[1].y + ht;
        /* Bottom left. */
        points[3].x = points[0].x;
        points[3].y = points[2].y;

        if (ge->spot_region == NULL)
          ge->spot_region = gdk_region_polygon(points, 4,
                                               GDK_WINDING_RULE);
        else
          gdk_region_offset(ge->spot_region,
                            sx - ge->spot.x, sy - ge->spot.y);

        ge->spot.x = sx;
        ge->spot.y = sy;
        ge->spot.width = wd;
        ge->spot.height = ht;
    }
}

/**********************************************************************
 *
 * Initialization routines.
 *
 **********************************************************************/

static void
gecontrol_finalize(GObject *obj)
{
    gint i;
    GEControl *ge;
    GEControlClass *gec;

    g_return_if_fail(obj != 0);
    g_return_if_fail(IS_GECONTROL(obj));

    /*
     * Destroy all the regions for the buttons.
     */
    ge = GECONTROL(obj);
    for (i = 0; i < 18; i++) {
        if (ge->buttons[i].region != 0)
          gdk_region_destroy(ge->buttons[i].region);
        ge->buttons[i].region = 0;
    }

    /*
     * Make sure the image is removed if it exists.
     */
    if (ge->gimage != 0) {
        if (ge->gimage->bytes > 0)
          g_free(ge->gimage->bitmap);
        g_free(ge->gimage);
        ge->gimage = 0;
    }

    gec = GECONTROL_GET_CLASS(obj);

    /*
     * Unreference all the pixbufs that were created.
     */
    if (gec->draw != 0) {
        g_object_unref(G_OBJECT(gec->draw));
        g_object_unref(G_OBJECT(gec->move));
        g_object_unref(G_OBJECT(gec->copy));
        g_object_unref(G_OBJECT(gec->fliph));
        g_object_unref(G_OBJECT(gec->flipv));
        g_object_unref(G_OBJECT(gec->shear));
        g_object_unref(G_OBJECT(gec->rleft));
        g_object_unref(G_OBJECT(gec->rright));
        g_object_unref(G_OBJECT(gec->rotate));
        g_object_unref(G_OBJECT(gec->uleft));
        g_object_unref(G_OBJECT(gec->up));
        g_object_unref(G_OBJECT(gec->uright));
        g_object_unref(G_OBJECT(gec->left));
        g_object_unref(G_OBJECT(gec->right));
        g_object_unref(G_OBJECT(gec->dleft));
        g_object_unref(G_OBJECT(gec->down));
        g_object_unref(G_OBJECT(gec->dright));

        gec->draw = gec->move = gec->copy =
            gec->fliph = gec->flipv = gec->shear =
            gec->rleft = gec->rright = gec->rotate =
            gec->uleft = gec->up = gec->uright =
            gec->left = gec->right =
            gec->dleft = gec->down = gec->dright = 0;
    }
}

static void
gecontrol_preferred_size(GtkWidget *widget, GtkRequisition *preferred)
{
    GEControl *gw = GECONTROL(widget);
    gint ht;

    preferred->width = 50 + (3 * (GEC_TOGGLE_SIZE + 4)) + 4;
    preferred->height = (GEC_TOGGLE_SIZE + 6) + ((5 * GEC_BUTTON_SIZE) + 8);

    if (gw->gimage != 0) {
        /*
         * The addition of 10 includes a box around the glyph, a line of empty
         * pixels between the box and the glyph image, and a space of 3 pixels
         * above and below the glyph image.
         */
        preferred->height += gw->gimage->height + 10;

        /*
         * Determine the height of the color list. Each color spot is 8x8 and
         * there is a buffer of two pixels on each side, and 2 pixels in
         * between them vertically.
         */
        if (gw->gimage->bpp == 2 || gw->gimage->bpp == 4) {
            preferred->width += 8 + 4;
            ht = 8 * (1 << gw->gimage->bpp);
            preferred->height = MAX(preferred->height, ht);
        } else if (gw->gimage->bpp == 8) {
            /*
             * For 8 bits per pixel, the square is 8x8 spots with 16 spots per
             * row and 16 rows. This gives 64 + 4 pixels which includes the 2
             * empties on each side.
             */
            preferred->width += 128 + 4;
            ht = 128 + 4;
            preferred->height = MAX(preferred->height, ht);
        }
    }
}

static void
gecontrol_actual_size(GtkWidget *widget, GtkAllocation *actual)
{
    gtk_widget_set_allocation(widget, actual);

    gecontrol_position_buttons(widget);

    if (gtk_widget_get_realized(widget))
      gdk_window_move_resize(gtk_widget_get_window(widget), actual->x, actual->y,
                             actual->width, actual->height);
}

static void
gecontrol_button_normal(GEControl *ge, gint button)
{
    gint v;
    GtkWidget *w = GTK_WIDGET(ge);
    GdkPoint points[4];

    if (button == GEC_GLYPH_IMAGE)
      return;

    if (button < 3) {
        gtk_paint_diamond(gtk_widget_get_style(w),
                          gtk_widget_get_window(w), GTK_STATE_NORMAL,
                          GTK_SHADOW_OUT, 0, GTK_WIDGET(ge), "gectrl",
                          ge->buttons[button].x, ge->buttons[button].y,
                          GEC_TOGGLE_SIZE, GEC_TOGGLE_SIZE);

        points[0].x = ge->buttons[button].x + (GEC_TOGGLE_SIZE >> 1);
        points[0].y = ge->buttons[button].y + 3;
        points[1].x = ge->buttons[button].x + 3;
        points[1].y = ge->buttons[button].y + (GEC_TOGGLE_SIZE >> 1);
        points[2].x = points[0].x;
        points[2].y = ge->buttons[button].y + GEC_TOGGLE_SIZE - 3;
        points[3].x = ge->buttons[button].x + GEC_TOGGLE_SIZE - 3;
        points[3].y = points[1].y;

        gdk_draw_polygon(gtk_widget_get_window(w),
                         gtk_widget_get_style(w)->bg_gc[GTK_STATE_NORMAL],
                         TRUE,
                         points, 4);

        v = (GEC_TOGGLE_SIZE >> 1) - (BMAP_DIM >> 1);

    } else {
        gtk_paint_box(gtk_widget_get_style(w), gtk_widget_get_window(w),
                      GTK_STATE_NORMAL,
                      GTK_SHADOW_OUT, 0, GTK_WIDGET(ge), "gectrl",
                      ge->buttons[button].x, ge->buttons[button].y,
                      GEC_BUTTON_SIZE, GEC_BUTTON_SIZE);

        v = (GEC_BUTTON_SIZE >> 1) - (BMAP_DIM >> 1);
    }

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(w));
    gdk_cairo_set_source_pixbuf(cr, ge->buttons[button].image, ge->buttons[button].x + v, ge->buttons[button].y + v);
    cairo_rectangle(cr, ge->buttons[button].x + v, ge->buttons[button].y + v, BMAP_DIM, BMAP_DIM);
    cairo_fill(cr);
    cairo_destroy(cr);
}

static void
gecontrol_button_prelight(GEControl *ge, gint button)
{
    gint v;
    GtkWidget *w = GTK_WIDGET(ge);
    GdkPoint points[4];

    if (button == GEC_GLYPH_IMAGE)
      return;

    if (button < 3) {
        gtk_paint_diamond(gtk_widget_get_style(w),
                          gtk_widget_get_window(w), GTK_STATE_PRELIGHT,
                          GTK_SHADOW_OUT, 0, GTK_WIDGET(ge), "gectrl",
                          ge->buttons[button].x, ge->buttons[button].y,
                          GEC_TOGGLE_SIZE, GEC_TOGGLE_SIZE);

        points[0].x = ge->buttons[button].x + (GEC_TOGGLE_SIZE >> 1);
        points[0].y = ge->buttons[button].y + 3;
        points[1].x = ge->buttons[button].x + 3;
        points[1].y = ge->buttons[button].y + (GEC_TOGGLE_SIZE >> 1);
        points[2].x = points[0].x;
        points[2].y = ge->buttons[button].y + GEC_TOGGLE_SIZE - 3;
        points[3].x = ge->buttons[button].x + GEC_TOGGLE_SIZE - 3;
        points[3].y = points[1].y;

        gdk_draw_polygon(gtk_widget_get_window(w),
                         gtk_widget_get_style(w)->bg_gc[GTK_STATE_PRELIGHT],
                         TRUE, points, 4);
        v = (GEC_TOGGLE_SIZE >> 1) - (BMAP_DIM >> 1);
    } else {
        gtk_paint_box(gtk_widget_get_style(w), gtk_widget_get_window(w),
                      GTK_STATE_PRELIGHT,
                      GTK_SHADOW_OUT, 0, GTK_WIDGET(ge), "gectrl",
                      ge->buttons[button].x, ge->buttons[button].y,
                      GEC_BUTTON_SIZE, GEC_BUTTON_SIZE);
        v = (GEC_BUTTON_SIZE >> 1) - (BMAP_DIM >> 1);
    }

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(w));
    gdk_cairo_set_source_pixbuf(cr, ge->buttons[button].image, ge->buttons[button].x + v, ge->buttons[button].y + v);
    cairo_rectangle(cr, ge->buttons[button].x + v, ge->buttons[button].y + v, BMAP_DIM, BMAP_DIM);
    cairo_fill(cr);
    cairo_destroy(cr);
}

static void
gecontrol_button_active(GEControl *ge, gint button)
{
    gint v;
    GtkWidget *w = GTK_WIDGET(ge);
    GdkPoint points[4];

    if (button == GEC_GLYPH_IMAGE)
      return;

    if (button < 3) {
        gtk_paint_diamond(gtk_widget_get_style(w),
                          gtk_widget_get_window(w), GTK_STATE_ACTIVE,
                          GTK_SHADOW_OUT, 0, GTK_WIDGET(ge), "gectrl",
                          ge->buttons[button].x, ge->buttons[button].y,
                          GEC_TOGGLE_SIZE, GEC_TOGGLE_SIZE);

        points[0].x = ge->buttons[button].x + (GEC_TOGGLE_SIZE >> 1);
        points[0].y = ge->buttons[button].y + 3;
        points[1].x = ge->buttons[button].x + 3;
        points[1].y = ge->buttons[button].y + (GEC_TOGGLE_SIZE >> 1);
        points[2].x = points[0].x;
        points[2].y = ge->buttons[button].y + GEC_TOGGLE_SIZE - 3;
        points[3].x = ge->buttons[button].x + GEC_TOGGLE_SIZE - 3;
        points[3].y = points[1].y;

        gdk_draw_polygon(gtk_widget_get_window(w),
                         gtk_widget_get_style(w)->bg_gc[GTK_STATE_ACTIVE],
                         TRUE, points, 4);
        v = (GEC_TOGGLE_SIZE >> 1) - (BMAP_DIM >> 1);
    } else {
        gtk_paint_box(gtk_widget_get_style(w), gtk_widget_get_window(w),
                      GTK_STATE_ACTIVE,
                      GTK_SHADOW_IN, 0, GTK_WIDGET(ge), "gectrl",
                      ge->buttons[button].x, ge->buttons[button].y,
                      GEC_BUTTON_SIZE, GEC_BUTTON_SIZE);
        v = (GEC_BUTTON_SIZE >> 1) - (BMAP_DIM >> 1);
    }

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(w));
    gdk_cairo_set_source_pixbuf(cr, ge->buttons[button].image, ge->buttons[button].x + v, ge->buttons[button].y + v);
    cairo_rectangle(cr, ge->buttons[button].x + v, ge->buttons[button].y + v, BMAP_DIM, BMAP_DIM);
    cairo_fill(cr);
    cairo_destroy(cr);
}

#if 0
static void
gecontrol_get_image_pixels(GEControl *ge, gint color)
{
    gint byte;
    guint16 x, y, bpr, si, di, nx;
    guchar *masks;
    bdf_bitmap_t *im;

    im = ge->gimage;
    ge->points_used = 0;

    di = 0;
    masks = 0;
    switch (im->bpp) {
      case 1: masks = bdf_onebpp; di = 7; break;
      case 2: masks = bdf_twobpp; di = 3; break;
      case 4: masks = bdf_fourbpp; di = 1; break;
      case 8: masks = bdf_eightbpp; di = 0; break;
    }

    bpr = ((im->width * im->bpp) + 7) >> 3;
    for (y = 0; y < im->height; y++) {
        for (nx = x = 0; x < im->width; x++, nx += im->bpp) {
            si = (nx & 7) / im->bpp;

            byte = im->bitmap[(y * bpr) + (nx >> 3)] & masks[si];
            if (di > si)
              byte >>= (di - si) * im->bpp;
            if (byte == color) {
                if (ge->points_used == ge->points_size) {
                    if (ge->points_size == 0)
                      ge->points =
                          (GdkPoint *) g_malloc(sizeof(GdkPoint) * 64);
                    else
                      ge->points = (GdkPoint *)
                          g_realloc(ge->points,
                                    sizeof(GdkPoint) *
                                    (ge->points_size + 64));;
                    ge->points_size += 64;
                }
                ge->points[ge->points_used].x = x + im->x + 2;
                ge->points[ge->points_used].y = y + im->y + 2;
                ge->points_used++;
            }
        }
    }
}
#endif

static void
gecontrol_make_rgb_glyph(GEControl *ge)
{
    GtkWidget *w = GTK_WIDGET(ge);
    gint byte = 0;
    guint16 x, y, bpr, rgb_bpr, si, di, nx;
    guchar bg[4], pix[4], *masks;
    bdf_bitmap_t *im;

    /*
     * First, get the background color of the widget for the empty
     * pixels.
     */
    bg[0] = (guchar) gtk_widget_get_style(w)->bg[gtk_widget_get_state(w)].red;
    bg[1] = (guchar) gtk_widget_get_style(w)->bg[gtk_widget_get_state(w)].green;
    bg[2] = (guchar) gtk_widget_get_style(w)->bg[gtk_widget_get_state(w)].blue;

    im = ge->gimage;

    di = 0;
    masks = 0;
    switch (im->bpp) {
      case 1: masks = bdf_onebpp; di = 7; break;
      case 2: masks = bdf_twobpp; di = 3; break;
      case 4: masks = bdf_fourbpp; di = 1; break;
      case 8: masks = bdf_eightbpp; di = 0; break;
    }

    bpr = ((im->width * im->bpp) + 7) >> 3;

    rgb_bpr = im->width * 3;
    ge->rgb_used = rgb_bpr * im->height;

    /*
     * Make sure there is enough storage space for the image.
     */
    if (ge->rgb_size < ge->rgb_used) {
        if (ge->rgb_size == 0)
          ge->rgb = g_malloc(ge->rgb_used);
        else
          ge->rgb = g_realloc(ge->rgb, ge->rgb_used);
        ge->rgb_size = ge->rgb_used;
    }

    for (y = 0; y < im->height; y++) {
        for (nx = x = 0; x < im->width; x++, nx += im->bpp) {
            si = (nx & 7) / im->bpp;

            byte = im->bitmap[(y * bpr) + (nx >> 3)] & masks[si];
            if (di > si)
              byte >>= (di - si) * im->bpp;
            if (byte) {
                switch (im->bpp) {
                  case 1: memset(pix, 0, 3); break;
                  case 2: memset(pix, ge->colors[byte-1], 3); break;
                  case 4: memset(pix, ge->colors[byte-1+4], 3); break;
                  case 8: memset(pix, byte-1, 3); break;
                }
            } else
              memcpy(pix, bg, 3);

            memcpy(&ge->rgb[(y * rgb_bpr) + (x * 3)], pix, 3);
        }
    }
}

static void
gecontrol_highlight_selected_spot(GEControl *ge)
{
    GtkWidget *w = GTK_WIDGET(ge);
    gint x, y;
    GEControlClass *gec = GECONTROL_GET_CLASS(ge);

    if (!gtk_widget_get_realized(w) || ge->gimage == 0 || ge->gimage->bpp == 1)
      return;

    GdkColor bg = gtk_widget_get_style(w)->bg[gtk_widget_get_state(w)];

    if (ge->gimage->bpp != 8) {
        x = ge->spot.x;
        y = ge->spot.y + (8 * ge->cidx);
    } else {
        x = ge->spot.x + ((ge->cidx % 16) * 8);
        y = ge->spot.y + ((ge->cidx / 16) * 8);
    }
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(w));
    cairo_set_line_width(cr, 1.0);
    gdk_cairo_set_source_color(cr, &bg);
    cairo_rectangle(cr, ge->spot.x, ge->spot.y, ge->spot.width, ge->spot.height);
    cairo_stroke(cr);
    cairo_fill(cr);
    cairo_surface_flush(cr);

    gdk_draw_gray_image(gtk_widget_get_window(w),
                        gtk_widget_get_style(w)->fg_gc[gtk_widget_get_state(w)],
                        ge->spot.x, ge->spot.y,
                        ge->spot.width, ge->spot.height,
                        GDK_RGB_DITHER_NONE, ge->rgb, ge->spot.width);

    cairo_surface_mark_dirty(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_rectangle(cr, x, y, 7, 7);
    cairo_stroke(cr);
    cairo_destroy(cr);
}

static void
gecontrol_make_color_spots(GEControl *ge, gint bpp)
{
    gint i, j, c, wd, ht, bytes;

    if (bpp < 2 || bpp > 8)
      return;

    bytes = wd = ht = 0;

    switch (bpp) {
      case 2:
      case 4:
        wd = 8;
        ht = 8 * (1 << bpp);
        break;
      case 8:
        wd = ht = 128;
        break;
    }
    bytes = wd * ht;

    if (ge->rgb_size < bytes) {
        if (ge->rgb_size == 0)
          ge->rgb = g_malloc(bytes);
        else
          ge->rgb = g_realloc(ge->rgb, bytes);
        ge->rgb_size = bytes;
    }
    ge->rgb_used = bytes;

    /*
     * Now create the color spots image.
     */
    if (bpp != 8) {
        for (i = 0; i < (1 << bpp); i++) {
            if (bpp == 2)
              memset(ge->rgb+(i*64), ge->colors[i], 64);
            else
              memset(ge->rgb+(i*64), ge->colors[i+4], 64);
        }
    } else {
        for (c = i = 0; i < 128; i += 8) {
            for (j = 0; j < 128; j += 8, c++)
              memset(ge->rgb+((i*128)+j), c, 8);
            memcpy(ge->rgb+((i+1)*128), ge->rgb+(i*128), 128);
            memcpy(ge->rgb+((i+2)*128), ge->rgb+(i*128), 256);
            memcpy(ge->rgb+((i+4)*128), ge->rgb+(i*128), 512);
        }
    }
}

static void
gecontrol_draw_glyph_image(GEControl *ge)
{
    GtkWidget *w = GTK_WIDGET(ge);

    if (ge->gimage == 0 || !gtk_widget_get_realized(w))
      return;

    /*
     * 1. Draw the box around the image.
     */
    gdk_draw_rectangle(gtk_widget_get_window(w),
                       gtk_widget_get_style(w)->fg_gc[gtk_widget_get_state(w)],
                       FALSE, ge->gimage->x, ge->gimage->y,
                       ge->gimage->width + 4, ge->gimage->height + 4);

    /*
     * 2. Clear the space inside the rectangle.
     */
    gdk_window_clear_area(gtk_widget_get_window(w), ge->gimage->x + 1, ge->gimage->y + 1,
                          ge->gimage->width - 2, ge->gimage->height - 2);

    /*
     * 3. Draw the points.
     */
    gecontrol_make_rgb_glyph(ge);
    gdk_draw_rgb_image(gtk_widget_get_window(w),
                       gtk_widget_get_style(w)->bg_gc[gtk_widget_get_state(w)],
                       ge->gimage->x + 2, ge->gimage->y + 2,
                       ge->gimage->width, ge->gimage->height,
                       GDK_RGB_DITHER_NONE, ge->rgb,
                       ge->gimage->width * 3);
}

static gboolean
gecontrol_expose(GtkWidget *w, GdkEventExpose *ev)
{
    gint i;
    GEControl *ge = GECONTROL(w);
    GEControlClass *gec = GECONTROL_GET_CLASS(w);

    /*
     * Draw the glyph image if one was provided.
     */
    gecontrol_draw_glyph_image(ge);

    for (i = 0; i < GEC_GLYPH_IMAGE; i++) {
        if (ge->buttons[i].set)
          gecontrol_button_active(ge, i);
        else
          gecontrol_button_normal(ge, i);
    }

    /*
     * Draw the color spots if called for.
     */
    if (ge->gimage && ge->gimage->bpp > 1) {
        gecontrol_make_color_spots(ge, ge->gimage->bpp);

        /*
         * Draw the box around the active color.
         */
        gecontrol_highlight_selected_spot(ge);
    }
    return FALSE;
}

static gboolean
gecontrol_motion_notify(GtkWidget *w, GdkEventMotion *ev)
{
    gint i, x, y;
    GEControl *ge;
    gchar buf[24];

    ge = GECONTROL(w);
    for (i = 0; i < 18; i++) {
        if (ge->buttons[i].region != NULL &&
            gdk_region_point_in(ge->buttons[i].region, ev->x, ev->y)) {
            if (i != ge->last_button) {

                /*
                 * Turn of the prelight on the previous button.
                 */
                if (ge->last_button >= 0) {
                    if (ge->buttons[ge->last_button].set == FALSE)
                      gecontrol_button_normal(ge, ge->last_button);
                }

                if (ge->buttons[i].set == FALSE)
                  gecontrol_button_prelight(ge, i);

                if (ge->tip_label != 0)
                  gtk_label_set_text(GTK_LABEL(ge->tip_label),
                                     (gchar *) ge->buttons[i].help);
                ge->last_button = i;
            }
            break;
        }
    }
    if (i == 18) {
        /*
         * Now check to see if the pointer is in the color spot. Only
         * if the tip label exists. No reason to know this info other than
         * to inform the user.
         */
        if (ge->tip_label) {
            if (ge->spot_region != NULL &&
                gdk_region_point_in(ge->spot_region, ev->x, ev->y)) {
                /*
                 * Determine which color this is and it's value. Mask
                 * the coordinates so they can't overflow the text buffer
                 * if they somehow get too large.
                 */
                x = (((guint) ev->x) - ge->spot.x) & 0xff;
                y = (((guint) ev->y) - ge->spot.y) & 0xff;
                
                if (ge->gimage->bpp == 2)
                  sprintf(buf, "Color: %03d Gray: %03d", (y>>3)+1,
                          ge->colors[y>>3]);
                else if (ge->gimage->bpp == 4)
                  sprintf(buf, "Color: %03d Gray: %03d", (y>>3)+1,
                          ge->colors[(y>>3) + 4]);
                else {
                    /*
                     * Divide x and y by 4 (spots are 4x4 in the 8bpp image)
                     * to get row and column.
                     */
                    x >>= 3;
                    y >>= 3;
                    sprintf(buf, "Color: %03d Gray: %03d",
                            ((y<<4)+x)+1, (y<<4)+x);
                }
                gtk_label_set_text(GTK_LABEL(ge->tip_label), buf);
                return FALSE;
            }
        }

        if (ge->tip_label != 0)
          gtk_label_set_text(GTK_LABEL(ge->tip_label), "");
        if (ge->last_button >= 0) {
            if (ge->buttons[ge->last_button].set == FALSE)
              gecontrol_button_normal(ge, ge->last_button);
        }
        ge->last_button = -1;
    }
    return FALSE;
}

static gboolean
handle_timeout(gpointer data)
{
    GEControl *ge = GECONTROL(data);
    GEControlActivateInfo ai;

    if (ge->timer_button < 0 || ge->buttons[ge->timer_button].set == FALSE) {
        ge->timer_button = -1;
        return FALSE;
    }

    /*
     * Emit the operation signal here.
     */
    ai.operation = (GEControlOperation) ge->timer_button;
    g_signal_emit(G_OBJECT(ge), gecontrol_signals[ACTIVATE], 0, &ai);
    ge->timer_count++;
    return TRUE;
}

static gboolean
gecontrol_button_press(GtkWidget *w, GdkEventButton *ev)
{
    gint i, o;
    GEControl *ge = GECONTROL(w);

    for (i = 0; i < 17; i++) {
        if (ge->buttons[i].region != 0 &&
            gdk_region_point_in(ge->buttons[i].region, ev->x, ev->y)) {
            if (i < 3) {
                if (ge->buttons[i].set == TRUE)
                  /*
                   * The toggle button is already set. Simply return.
                   */
                  return FALSE;

                /*
                 * Clear the button that is set.
                 */
                o = (ge->buttons[ge->buttons[i].other_toggles[0]].set) ?
                    ge->buttons[i].other_toggles[0] :
                    ge->buttons[i].other_toggles[1];
                gecontrol_button_normal(ge, o);
                ge->buttons[o].set = FALSE;
            }
            gecontrol_button_active(ge, i);
            ge->buttons[i].set = TRUE;

            /*
             * If this is any of the shift buttons, add a timer so it
             * will be handled multiple times.
             */
            if (i >= GEC_ULEFT_BUTTON && i <= GEC_DRIGHT_BUTTON) {
                ge->timer_count = 0;
                ge->timer_button = i;
                ge->timer = g_timeout_add(100, handle_timeout,
                                          (gpointer) ge);
            }
            break;
        }
    }

    return FALSE;
}

static gboolean
gecontrol_button_release(GtkWidget *w, GdkEventButton *ev)
{
    gint i, x, y;
    GEControl *ge = GECONTROL(w);
    GEControlActivateInfo ai;

    for (i = 0; i < 17; i++) {
        if (ge->buttons[i].region != 0 &&
            gdk_region_point_in(ge->buttons[i].region, ev->x, ev->y)) {
            if (i >= 3) {
                gecontrol_button_prelight(ge, i);
                ge->buttons[i].set = FALSE;
            }
            if (ge->timer_count == 0) {
                ai.operation = (GEControlOperation) i;
                g_signal_emit(G_OBJECT(ge), gecontrol_signals[ACTIVATE], 0,
                              &ai);
            } else
              /*
               * Simply reset the timer count because the signal was emitted
               * in the timeout handler, probably more than once.
               */
              ge->timer_count = 0;
            break;
        }
    }
    if (i == 17) {
        /*
         * Check to see if one of the colors was selected.
         */
        if (ge->gimage || ge->gimage->bpp > 1) {
            if (ge->spot_region != NULL &&
                gdk_region_point_in(ge->spot_region, ev->x, ev->y)) {
                x = (((guint) ev->x) - ge->spot.x) & 0xff;
                y = (((guint) ev->y) - ge->spot.y) & 0xff;
                if (ge->gimage->bpp != 8)
                  i = y >> 3;
                else {
                    x >>= 3;
                    y >>= 3;
                    i = (y << 4) + x;
                }
                ge->cidx = i;
                gecontrol_highlight_selected_spot(ge);

                ai.operation = GECONTROL_COLOR_CHANGE;
                ai.color = ge->cidx + 1;
                g_signal_emit(G_OBJECT(ge), gecontrol_signals[ACTIVATE],
                              0, &ai);
            }
        }
    }
    return FALSE;
}

static void
gecontrol_set_property(GObject *obj, guint prop_id, const GValue *value,
                       GParamSpec *pspec)
{
    GEControl *ge;

    ge = GECONTROL(obj);

    switch (prop_id) {
      case TIP_LABEL:
        ge->tip_label = (GtkWidget *) g_value_get_object(value);
        break;
      case GLYPH_IMAGE:
        gecontrol_set_glyph_image(ge,
                                  (bdf_bitmap_t *) g_value_get_pointer(value));
        break;
      case COLOR_LIST:
        gecontrol_set_color_list(ge, (guint16 *) g_value_get_pointer(value));
        break;
    }
}

static void
gecontrol_get_property(GObject *obj, guint prop_id, GValue *value,
                       GParamSpec *pspec)
{
    GEControl *ge;

    ge = GECONTROL(obj);

    switch (prop_id) {
      case TIP_LABEL:
        g_value_set_object(value, ge->tip_label);
        break;
      case GLYPH_IMAGE:
        g_value_set_pointer(value, ge->gimage);
        break;
      case COLOR_LIST:
        g_value_set_pointer(value, ge->colors);
        break;
    }
}

static void
gecontrol_class_init(gpointer g_class, gpointer class_data)
{
    GObjectClass *goc = G_OBJECT_CLASS(g_class);
    GtkWidgetClass *wc = GTK_WIDGET_CLASS(g_class);
    GEControlClass *gc = GECONTROL_CLASS(g_class);

    goc->set_property = gecontrol_set_property;
    goc->get_property = gecontrol_get_property;
    goc->finalize = gecontrol_finalize;

    wc->size_request = gecontrol_preferred_size;
    wc->size_allocate = gecontrol_actual_size;
    wc->expose_event = gecontrol_expose;
    wc->motion_notify_event = gecontrol_motion_notify;
    wc->button_press_event = gecontrol_button_press;
    wc->button_release_event = gecontrol_button_release;

    g_object_class_install_property(goc, TIP_LABEL,
                                    g_param_spec_object("tipLabel",
                                                        _("Tip Label"),
                                                        _("A GtkLabel widget where tips are shown when the mouse moves."),
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_READWRITE));

    g_object_class_install_property(goc, GLYPH_IMAGE,
                                    g_param_spec_pointer("glyphImage",
                                                       _("Glyph Image"),
                                                       _("The bitmap image of a glyph."),
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(goc, COLOR_LIST,
                                    g_param_spec_pointer("colorList",
                                                         _("Color list"),
                                                         _("Colors to be used for glyphs having bits-per-pixel > 1."),
                                                       G_PARAM_READWRITE));

    gecontrol_signals[ACTIVATE] =
        g_signal_new("activate",
                     G_TYPE_FROM_CLASS(goc),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(GEControlClass, activate),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    /*
     * Initialize all the pixbufs.
     */
    gc->draw = gdk_pixbuf_new_from_xpm_data(draw_xpm);
    gc->move = gdk_pixbuf_new_from_xpm_data(move_xpm);
    gc->copy = gdk_pixbuf_new_from_xpm_data(copy_xpm);

    gc->fliph = gdk_pixbuf_new_from_xpm_data(fliph_xpm);
    gc->flipv = gdk_pixbuf_new_from_xpm_data(flipv_xpm);
    gc->shear = gdk_pixbuf_new_from_xpm_data(shear_xpm);

    gc->rleft = gdk_pixbuf_new_from_xpm_data(rleft_xpm);
    gc->rright = gdk_pixbuf_new_from_xpm_data(rright_xpm);
    gc->rotate = gdk_pixbuf_new_from_xpm_data(rotate_xpm);

    gc->uleft = gdk_pixbuf_new_from_xpm_data(uleft_xpm);
    gc->up = gdk_pixbuf_new_from_xpm_data(up_xpm);
    gc->uright = gdk_pixbuf_new_from_xpm_data(uright_xpm);

    gc->left = gdk_pixbuf_new_from_xpm_data(left_xpm);
    gc->right = gdk_pixbuf_new_from_xpm_data(right_xpm);

    gc->dleft = gdk_pixbuf_new_from_xpm_data(dleft_xpm);
    gc->down = gdk_pixbuf_new_from_xpm_data(down_xpm);
    gc->dright = gdk_pixbuf_new_from_xpm_data(dright_xpm);

    parent_class = g_type_class_peek_parent(gc);
}

#define GEC_EVMASK (GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|\
                    GDK_BUTTON_RELEASE_MASK)

static void
gecontrol_init(GTypeInstance *instance, gpointer g_class)
{
    gint i;
    GEControl *gw = GECONTROL(instance);
    GEControlClass *gc = GECONTROL_CLASS(g_class);

    gw->gimage = 0;

    gw->last_button = gw->timer_button = -1;
    gw->timer_count = 0;

    /*
     * Enable the button press, release, and motion events.
     */
    gtk_widget_add_events(GTK_WIDGET(gw), GEC_EVMASK);

    gw->points_used = gw->points_size = 0;

    gw->buttons[GEC_DRAW_TOGGLE].image = gc->draw;
    gw->buttons[GEC_MOVE_TOGGLE].image = gc->move;
    gw->buttons[GEC_COPY_TOGGLE].image = gc->copy;

    gw->buttons[GEC_FLIPH_BUTTON].image = gc->fliph;
    gw->buttons[GEC_FLIPV_BUTTON].image = gc->flipv;
    gw->buttons[GEC_SHEAR_BUTTON].image = gc->shear;

    gw->buttons[GEC_RLEFT_BUTTON].image = gc->rleft;
    gw->buttons[GEC_RRIGHT_BUTTON].image = gc->rright;
    gw->buttons[GEC_ROTATE_BUTTON].image = gc->rotate;

    gw->buttons[GEC_ULEFT_BUTTON].image = gc->uleft;
    gw->buttons[GEC_UP_BUTTON].image = gc->up;
    gw->buttons[GEC_URIGHT_BUTTON].image = gc->uright;

    gw->buttons[GEC_LEFT_BUTTON].image = gc->left;
    gw->buttons[GEC_RIGHT_BUTTON].image = gc->right;

    gw->buttons[GEC_DLEFT_BUTTON].image = gc->dleft;
    gw->buttons[GEC_DOWN_BUTTON].image = gc->down;
    gw->buttons[GEC_DRIGHT_BUTTON].image = gc->dright;

    for (i = 0; i < 18; i++) {
        gw->buttons[i].help = help_strings[i];
        gw->buttons[i].region = NULL;
        gw->buttons[i].x = gw->buttons[i].y = 0;
        gw->buttons[i].set = gw->buttons[i].toggle = FALSE;

        /*
         * At initialization time, the Draw toggle is always set by
         * default.
         */
        switch (i) {
          case GEC_DRAW_TOGGLE:
            gw->buttons[i].set = TRUE;
            gw->buttons[i].toggle = TRUE;
            gw->buttons[i].other_toggles[0] = GEC_MOVE_TOGGLE;
            gw->buttons[i].other_toggles[1] = GEC_COPY_TOGGLE;
            break;
          case GEC_MOVE_TOGGLE:
            gw->buttons[i].toggle = TRUE;
            gw->buttons[i].other_toggles[0] = GEC_DRAW_TOGGLE;
            gw->buttons[i].other_toggles[1] = GEC_COPY_TOGGLE;
            break;
          case GEC_COPY_TOGGLE:
            gw->buttons[i].toggle = TRUE;
            gw->buttons[i].other_toggles[0] = GEC_DRAW_TOGGLE;
            gw->buttons[i].other_toggles[1] = GEC_MOVE_TOGGLE;
            break;
        }
    }

    gw->cidx = 0;
    gw->spot_region = 0;
    gw->spot.x = gw->spot.y = gw->spot.width = gw->spot.height = 0;
}

/**********************************************************************
 *
 * API functions.
 *
 **********************************************************************/

GType
gecontrol_get_type(void)
{
    static GType gecontrol_type = 0;
  
    if (!gecontrol_type) {
        static const GTypeInfo gecontrol_info = {
            sizeof (GEControlClass),		/* class_size		*/
            0,					/* base_init		*/
            0,					/* base_finalize	*/
            gecontrol_class_init,		/* class_init		*/
            0,					/* class_finalize	*/
            0,					/* class_data		*/
            sizeof(GEControl),			/* instance_size	*/
            0,					/* n_preallocs		*/
            gecontrol_init,			/* instance_init	*/
            0,					/* value_table		*/
        };

        gecontrol_type = g_type_register_static(GTK_TYPE_DRAWING_AREA,
                                                "GEControl",
                                                &gecontrol_info, 0);
    }
  
    return gecontrol_type;
}

GtkWidget *
gecontrol_new(const gchar *prop1, ...)
{
    GtkWidget *w;
    va_list var_args;

    va_start(var_args, prop1);
    w = GTK_WIDGET(g_object_new_valist(gecontrol_get_type(), prop1, var_args));
    va_end(var_args);

    return w;
}

GtkWidget *
gecontrol_newv(GtkWidget *tip_label, bdf_bitmap_t *image, guint16 *colors)
{
    GEControl *ge = g_object_new(gecontrol_get_type(),
                                 "tipLabel", tip_label,
                                 "glyphImage", image,
                                 "colorList", colors,
                                 NULL);

    return GTK_WIDGET(ge);
}

void
gecontrol_update_glyph_image(GEControl *ge, bdf_bitmap_t *image)
{
    if (ge->gimage) {
        if (ge->gimage->bytes > 0)
          g_free(ge->gimage->bitmap);
        g_free(ge->gimage);
        ge->gimage = 0;
    }
    if (image != 0) {
        ge->gimage = (bdf_bitmap_t *) g_malloc(sizeof(bdf_bitmap_t));
        memcpy(ge->gimage, image, sizeof(bdf_bitmap_t));
        if (ge->gimage->bytes > 0) {
            ge->gimage->bitmap = g_malloc(ge->gimage->bytes);
            memcpy(ge->gimage->bitmap, image->bitmap, image->bytes);
        }
        ge->gimage->x = ge->buttons[GEC_GLYPH_IMAGE].x;
        ge->gimage->y = ge->buttons[GEC_GLYPH_IMAGE].y;
        gecontrol_draw_glyph_image(ge);
    } else
      gtk_widget_queue_draw(GTK_WIDGET(ge));
}

void
gecontrol_set_glyph_image(GEControl *ge, bdf_bitmap_t *image)
{
    g_return_if_fail(ge != NULL);
    g_return_if_fail(IS_GECONTROL(ge));

    if (ge->gimage) {
        if (ge->gimage->bytes > 0)
          g_free(ge->gimage->bitmap);
        g_free(ge->gimage);
        ge->gimage = 0;
    }
    if (image != 0) {
        ge->gimage = (bdf_bitmap_t *) g_malloc(sizeof(bdf_bitmap_t));
        memcpy(ge->gimage, image, sizeof(bdf_bitmap_t));
        if (ge->gimage->bytes > 0) {
            ge->gimage->bitmap = g_malloc(ge->gimage->bytes);
            memcpy(ge->gimage->bitmap, image->bitmap, image->bytes);
        }
    }

    /*
     * Delete any spot region to force a new one to be created. This is
     * because the sizes change depending on the bits per pixel.
     */
    if (ge->spot_region != 0) {
        gdk_region_destroy(ge->spot_region);
        ge->spot_region = 0;
    }

    /*
     * Always make sure the color index is reset in this case.
     */
    ge->cidx = 0;

    /*
     * Always queue a resize to at least force a redraw of the widget.
     */
    gtk_widget_queue_resize(GTK_WIDGET(ge));
}

void
gecontrol_set_color_list(GEControl *ge, guint16 *colors)
{
    g_return_if_fail(ge != NULL);
    g_return_if_fail(IS_GECONTROL(ge));

    ge->colors = colors;
    gtk_widget_queue_draw(GTK_WIDGET(ge));
}

void
gecontrol_change_operation(GEControl *ge, GEControlOperation op)
{
    gint b, i;

    g_return_if_fail(ge != NULL);
    g_return_if_fail(IS_GECONTROL(ge));

    b = -1;
    if (op == GECONTROL_DRAW)
      b = GEC_DRAW_TOGGLE;
    else if (op == GECONTROL_MOVE)
      b = GEC_MOVE_TOGGLE;
    else if (op == GECONTROL_COPY)
      b = GEC_COPY_TOGGLE;

    if (b < 0 || ge->buttons[b].set == TRUE)
      return;

    for (i = 0; i < 3; i++) {
        if (i != b && ge->buttons[i].set == TRUE) {
            ge->buttons[i].set = FALSE;
            gecontrol_button_normal(ge, i);
            break;
        }
    }

    gecontrol_button_active(ge, b);
    ge->buttons[b].set = TRUE;
}

void
gecontrol_change_color(GEControl *ge, gint cidx)
{
    g_return_if_fail(ge != NULL);
    g_return_if_fail(IS_GECONTROL(ge));

    /*
     * No point in setting a color if this is a one bit per pixel image or
     * there is no image.
     */
    if (!ge->gimage || ge->gimage->bpp == 1)
      return;

    /*
     * If the index is out of bounds, then wrap it around the other side.
     */
    cidx--;
    if (cidx >= (1 << ge->gimage->bpp))
      cidx = 0;
    else if (cidx < 0)
      cidx = (1 << ge->gimage->bpp) - 1;

    ge->cidx = cidx;
    gecontrol_highlight_selected_spot(ge);
}

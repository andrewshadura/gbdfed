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

#include "glyphtest.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(s) dgettext(GETTEXT_PACKAGE,s)
#else
#define _(s) (s)
#endif

#define HMARGINS(fw) ((fw)->hmargin << 1)
#define VMARGINS(fw) ((fw)->vmargin << 1)

/*
 * Argument types.
 */
enum {
    PROP_0 = 0,
    PROP_BASELINE,
    PROP_DIRECTION
};

enum {
    ADD_GLYPH = 0,
    NUM_SIGNALS
};

static GtkWidgetClass *parent_class = 0;
static guint glyphtest_signals[NUM_SIGNALS];

#define GTESTMAX(h,i) ((h) > (i) ? (h) : (i))

static gboolean
_glyphtest_set_line_size(Glyphtest *gw)
{
    GtkWidget *w;
    gboolean changed = FALSE;
    guint32 i;
    guint16 wd, wwidth;
    GlyphtestLine *lp;
    bdf_bbx_t bbx;

    w = GTK_WIDGET(gw);

    lp = &gw->line;
    (void) memset((char *) &bbx, 0, sizeof(bdf_bbx_t));

    wwidth = w->allocation.width - (HMARGINS(gw) + 4);

    for (wd = 0, i = 0; i < lp->glyphs_used; i++) {
        bbx.ascent = GTESTMAX(bbx.ascent, lp->glyphs[i].font->bbx.ascent);
        bbx.descent = GTESTMAX(bbx.descent, lp->glyphs[i].font->bbx.descent);
        bbx.width = GTESTMAX(bbx.width, lp->glyphs[i].font->bbx.width);
        wd += (lp->glyphs[i].font->spacing == BDF_PROPORTIONAL) ?
            lp->glyphs[i].glyph->dwidth : lp->glyphs[i].font->monowidth;
    }

    if (lp->glyphs_used == 0) {
        /*
         * If no glyphs are present, then set the overall bounding box
         * to some simple default.
         */
        bbx.ascent = 12;
        bbx.descent = 3;
        bbx.width = 10;
        wd = bbx.width << 3;
    }

    /*
     * If the actual line width changed, set the indicator.
     */
    if (wd != lp->width) {
        lp->width = wd;

        /*
         * If the line width overflows the window width, set the changed flag.
         */
        if (wd > wwidth)
          changed = TRUE;
    }

    /*
     * If the new bounding box is not the same as the current line bounding
     * box, then make the new one the current line bounding box.
     */
    if (bbx.ascent != lp->bbx.ascent || bbx.descent != lp->bbx.descent ||
        bbx.width != lp->bbx.width) {
        (void) memcpy((char *) &lp->bbx, (char *) &bbx, sizeof(bdf_bbx_t));
        changed = TRUE;
    }

    /*
     * Now set the line size.
     */
    lp->height = lp->bbx.ascent + lp->bbx.descent;
    lp->cpoint.y = (VMARGINS(gw) >> 1) + 2 + lp->bbx.ascent;

    return changed;
}

/**************************************************************************
 *
 * Class functions.
 *
 **************************************************************************/

static void
glyphtest_set_property(GObject *obj, guint prop_id, const GValue *value,
                       GParamSpec *pspec)
{
    Glyphtest *gw;

    gw = GLYPHTEST(obj);

    switch (prop_id) {
      case PROP_BASELINE:
        glyphtest_show_baseline(gw, g_value_get_boolean(value));
        break;
      case PROP_DIRECTION:
        glyphtest_change_direction(gw, g_value_get_int(value));
        break;
    }
}

static void
glyphtest_get_property(GObject *obj, guint prop_id, GValue *value,
                       GParamSpec *pspec)
{
    Glyphtest *gw;

    gw = GLYPHTEST(obj);

    switch (prop_id) {
      case PROP_BASELINE:
        g_value_set_boolean(value, gw->show_baseline);
        break;
      case PROP_DIRECTION:
        g_value_set_int(value, gw->dir);
        break;
    }
}

static void
glyphtest_destroy(GtkObject *obj)
{
    Glyphtest *gw;

    /*
     * Do some checks to make sure the incoming object exists and is the right
     * kind.
     */
    g_return_if_fail(obj != 0);
    g_return_if_fail(IS_GLYPHTEST(obj));

    gw = GLYPHTEST(obj);

    /*
     * Delete the line structure if it was allocated.
     */
    if (gw->line.glyphs_size > 0)
      g_free(gw->line.glyphs);
    gw->line.glyphs_used = gw->line.glyphs_size = 0;

    /*
     * Free up any points that have been allocated.
     */
    if (gw->image_size > 0)
      g_free(gw->image);
    gw->image_size = gw->image_used = 0;

    /*
     * Follow the class chain back up to free up resources allocated in the
     * parent classes.
     */
    GTK_OBJECT_CLASS(parent_class)->destroy(obj);
}

static void
glyphtest_finalize(GObject *obj)
{
    /*
     * Do some checks to make sure the incoming object exists and is the right
     * kind.
     */
    g_return_if_fail(obj != 0);
    g_return_if_fail(IS_GLYPHTEST(obj));

    /*
     * Follow the class chain back up to free up resources allocated in the
     * parent classes.
     */
    G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void
glyphtest_preferred_size(GtkWidget *widget, GtkRequisition *preferred)
{
    Glyphtest *gw;

    gw = GLYPHTEST(widget);

    preferred->width = gw->line.width + 4 + HMARGINS(gw);
    preferred->height = gw->line.height + 4 + VMARGINS(gw);
}

static void
glyphtest_actual_size(GtkWidget *widget, GtkAllocation *actual)
{
    widget->allocation = *actual;

    if (gtk_widget_get_realized(widget))
      gdk_window_move_resize(widget->window, actual->x, actual->y,
                             actual->width, actual->height);
}

static void
glyphtest_draw_focus(GtkWidget *widget, GdkRectangle *area)
{
    GdkGC *gc;
    gint x, y, wd, ht, fwidth, fpad;

    /*
     * Do something with this later to make sure the focus line width
     * is set in the GC's.
     */
    gtk_widget_style_get(widget,
                         "focus-line-width", &fwidth,
                         "focus-padding", &fpad, NULL);

    gc = widget->style->bg_gc[GTK_WIDGET_STATE(widget)];

    x = (widget->style->xthickness + fwidth + fpad) - 1;
    y = (widget->style->ythickness + fwidth + fpad) - 1;
    wd = (widget->allocation.width - (x * 2));
    ht = (widget->allocation.height - (y * 2));

    if (gtk_widget_has_focus(widget))
      gtk_paint_focus(widget->style, widget->window, GTK_WIDGET_STATE(widget),
                      area, widget, "glyphtest", x, y, wd, ht);
    else {
        gdk_gc_set_clip_rectangle(gc, area);
        gdk_draw_rectangle(widget->window, gc, FALSE, x, y, wd - 1, ht - 1);
        gdk_gc_set_clip_rectangle(gc, 0);
    }
}

static void
_glyphtest_get_pixels(Glyphtest *gw, bdf_glyph_t *glyph, bdf_font_t *font,
                      gint16 x, gint y)
{
    gint byte;
    guint16 i, j, bpr, si, di, nx;
    guchar *masks;

    di = 0;
    masks = 0;
    gw->image_used = 0;

    switch (font->bpp) {
      case 1: masks = bdf_onebpp; di = 7; break;
      case 2: masks = bdf_twobpp; di = 3; break;
      case 4: masks = bdf_fourbpp; di = 1; break;
      case 8: masks = bdf_eightbpp; di = 0; break;
    }

    bpr = ((glyph->bbx.width * font->bpp) + 7) >> 3;
    for (i = 0; i < glyph->bbx.height; i++) {
        for (nx = j = 0; j < glyph->bbx.width; j++, nx += font->bpp) {
            si = (nx & 7) / font->bpp;

            byte = glyph->bitmap[(i * bpr) + (nx >> 3)] & masks[si];
            if (di > si)
              byte >>= (di - si) * font->bpp;

            if (byte) {
                if (gw->image_used == gw->image_size) {
                    if (gw->image_size == 0)
                      gw->image =
                          (GdkPoint *) g_malloc(sizeof(GdkPoint) * 64);
                    else
                      gw->image = (GdkPoint *)
                          g_realloc(gw->image,
                                    sizeof(GdkPoint) *
                                    (gw->image_size + 64));;
                    gw->image_size += 64;
                }
                gw->image[gw->image_used].x =
                    x + glyph->bbx.x_offset + j;
                gw->image[gw->image_used].y =
                    (y - glyph->bbx.ascent) + i;
                gw->image_used++;
            }
        }
    }
}

static void
_glyphtest_draw_glyph(Glyphtest *gw, bdf_glyph_t *glyph, bdf_font_t *font)
{
    GtkWidget *w;
    gint16 rx, ry;

    w = GTK_WIDGET(gw);

    if (!gtk_widget_get_realized(w))
      return;

    ry = gw->line.cpoint.y;
    rx = gw->line.cpoint.x;
    if (gw->dir != GLYPHTEST_LEFT_TO_RIGHT)
      rx -= glyph->bbx.width;

    _glyphtest_get_pixels(gw, glyph, font, rx, ry);
    if (gw->image_used > 0)
      gdk_draw_points(w->window, w->style->fg_gc[GTK_STATE_NORMAL],
                      gw->image, gw->image_used);
}

static void
_glyphtest_redraw_glyphs(Glyphtest *gw)
{
    GtkWidget *w;
    guint32 i;
    guint16 dwidth;
    GlyphtestLine *lp;
    GlyphtestGlyph *gp;

    w = GTK_WIDGET(gw);

    if (!gtk_widget_get_realized(w))
      return;

    lp = &gw->line;

    lp->width = 0;
    if (gw->dir == GLYPHTEST_LEFT_TO_RIGHT)
      lp->cpoint.x = (HMARGINS(gw) >> 1) + 2;
    else
      lp->cpoint.x = w->allocation.width - ((HMARGINS(gw) >> 1) + 2);

    for (i = 0, gp = lp->glyphs; i < lp->glyphs_used; i++, gp++) {

        /*
         * Handle the special cases of the first glyph in case the normal
         * drawing position is going to put part of the glyph off the edge of
         * the window.
         */
        if (gw->dir == GLYPHTEST_LEFT_TO_RIGHT) {
            if (i == 0 && gp->glyph->bbx.x_offset < 0)
              lp->cpoint.x += -gp->glyph->bbx.x_offset;
        } else {
            if (i == 0 && gp->glyph->bbx.x_offset > 0 &&
                gp->glyph->bbx.x_offset > gp->glyph->bbx.width)
              lp->cpoint.x -= gp->glyph->bbx.width - gp->glyph->bbx.x_offset;
        }
        _glyphtest_draw_glyph(gw, gp->glyph, gp->font);

        dwidth = (gp->font->spacing == BDF_PROPORTIONAL) ?
            gp->glyph->dwidth : gp->font->monowidth;
        if (gw->dir == GLYPHTEST_LEFT_TO_RIGHT)
          lp->cpoint.x += dwidth;
        else
          lp->cpoint.x -= dwidth;
        lp->width += dwidth;
    }
}

static void
glyphtest_draw(GtkWidget *widget, GdkRectangle *area)
{
    Glyphtest *gw;
    GdkPoint s, e;
    GdkRectangle clear;

    if (!gtk_widget_get_realized(widget))
      return;

    gw = GLYPHTEST(widget);

    /*
     * Erase the window.
     */
    clear.x = clear.y = (HMARGINS(gw) >> 1);
    clear.width = widget->allocation.width - (clear.x << 1);
    clear.height = widget->allocation.height - (clear.y << 1);
    gdk_window_clear_area(widget->window, clear.x, clear.y,
                          clear.width, clear.height);

    /*
     * Redraw the glyphs.
     */
    _glyphtest_redraw_glyphs(gw);

    /*
     * Draw the baseline if indicated.
     */
    if (gw->show_baseline == TRUE) {
        s.x = (HMARGINS(gw) >> 1) + 2;
        e.x = widget->allocation.width - s.x;
        s.y = e.y = gw->line.cpoint.y;

        gdk_draw_line(widget->window,
                      widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                      s.x, s.y, e.x, e.y);
    }
}

static gboolean
glyphtest_expose(GtkWidget *widget, GdkEventExpose *event)
{
    /*
     * Paint the shadow first.
     */
    if (GTK_WIDGET_DRAWABLE(widget))
      gtk_paint_shadow(widget->style, widget->window,
                       GTK_WIDGET_STATE(widget), GTK_SHADOW_OUT,
                       &event->area,
                       widget, "glyphtest",
                       0, 0,
                       widget->allocation.width,
                       widget->allocation.height);

    glyphtest_draw(widget, 0);

    glyphtest_draw_focus(widget, &event->area);

    return FALSE;
}

static void
glyphtest_realize(GtkWidget *widget)
{
    Glyphtest *gw;
    GdkWindowAttr attributes;
    gint attributes_mask;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_GLYPHTEST(widget));

    gw = GLYPHTEST(widget);
    gtk_widget_set_realized(GTK_WIDGET(widget), TRUE);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.colormap = gtk_widget_get_colormap(widget);
    attributes.event_mask = gtk_widget_get_events(widget);
    attributes.event_mask |= (GDK_EXPOSURE_MASK|GDK_ENTER_NOTIFY_MASK|
                              GDK_LEAVE_NOTIFY_MASK|GDK_FOCUS_CHANGE_MASK);

    attributes_mask = GDK_WA_X|GDK_WA_Y|GDK_WA_VISUAL|GDK_WA_COLORMAP;

    widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
                                    &attributes, attributes_mask);
    gdk_window_set_user_data(widget->window, widget);

    widget->style = gtk_style_attach(widget->style, widget->window);
    gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
glyphtest_unrealize(GtkWidget *widget)
{
    Glyphtest *gw;

    gw = GLYPHTEST(widget);
}

static gint
glyphtest_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(IS_GLYPHTEST(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    (void) glyphtest_draw_focus(widget, NULL);

    return FALSE;
}

static gint
glyphtest_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(IS_GLYPHTEST(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    (void) glyphtest_draw_focus(widget, NULL);

    return FALSE;
}

/**************************************************************************
 *
 * Class and object initialization routines.
 *
 **************************************************************************/

static void
glyphtest_class_init(gpointer g_class, gpointer class_data)
{
    GObjectClass *gocp = G_OBJECT_CLASS(g_class);
    GtkObjectClass *ocp = GTK_OBJECT_CLASS(g_class);
    GtkWidgetClass *wcp = GTK_WIDGET_CLASS(g_class);

    /*
     * Set the class global variables.
     */
    parent_class = g_type_class_peek_parent(g_class);

    ocp->destroy = glyphtest_destroy;
    gocp->set_property = glyphtest_set_property;
    gocp->get_property = glyphtest_get_property;
    gocp->finalize = glyphtest_finalize;

    /*
     * Add argument (a.k.a. resource) types.
     */
    g_object_class_install_property(gocp, PROP_BASELINE,
                                    g_param_spec_boolean("showBaseline",
                                                         _("Show baseline"),
                                                         _("Draw the baseline."),
                                                         FALSE,
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gocp, PROP_DIRECTION,
                                    g_param_spec_uint("direction",
                                                      _("Direction"),
                                                      _("Override for the drawing direction of the glyphs."),
                                                      GLYPHTEST_LEFT_TO_RIGHT,
                                                      GLYPHTEST_RIGHT_TO_LEFT,
                                                      GLYPHTEST_LEFT_TO_RIGHT,
                                                      G_PARAM_READWRITE));

    /*
     * Add the signals these objects emit.
     */
    glyphtest_signals[ADD_GLYPH] =
        g_signal_new("add_glyph",
                     G_TYPE_FROM_CLASS(gocp),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(GlyphtestClass, glyph_added),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);

    /*
     * Set all the functions for handling events for objects of this class.
     */
    wcp->size_request = glyphtest_preferred_size;
    wcp->size_allocate = glyphtest_actual_size;
    wcp->realize = glyphtest_realize;
    wcp->unrealize = glyphtest_unrealize;
    wcp->expose_event = glyphtest_expose;
    wcp->focus_in_event = glyphtest_focus_in;
    wcp->focus_out_event = glyphtest_focus_out;
}

static void
glyphtest_init(GTypeInstance *obj, gpointer g_class)
{
    Glyphtest *gw = GLYPHTEST(obj);
    gint fwidth;

    gtk_widget_set_can_focus(GTK_WIDGET(gw), TRUE);

    (void) memset((char *) &gw->line, 0, sizeof(GlyphtestLine));

    gw->dir = GLYPHTEST_LEFT_TO_RIGHT;
    gw->show_baseline = TRUE;

    gw->image_used = gw->image_size = 0;

    /*
     * Determine the space that will be needed for drawing the shadow and the
     * focus rectangle.
     */
    gtk_widget_style_get(GTK_WIDGET(gw),
                         "focus-line-width", &fwidth,
                         NULL);

    /*
     * Padding that will appear before and after the focus rectangle.
     * Hardcode this for now.
     */
    gw->focus_thickness = 3;
    gw->hmargin =
        gw->widget.style->xthickness + fwidth + (gw->focus_thickness * 2);
    gw->vmargin = 
        gw->widget.style->ythickness + fwidth + (gw->focus_thickness * 2);

    /*
     * Call the line size function to set the initial size.
     */
    (void) _glyphtest_set_line_size(gw);
}

/**************************************************************************
 *
 * API functions.
 *
 **************************************************************************/

static const GTypeInfo glyphtest_info = {
    sizeof(GlyphtestClass),
    NULL,
    NULL,
    glyphtest_class_init,
    NULL,
    NULL,
    sizeof(Glyphtest),
    0,
    glyphtest_init,
    NULL,
};

GType
glyphtest_get_type(void)
{
    static GType glyphtest_type = 0;

    if (!glyphtest_type)
      glyphtest_type = g_type_register_static(GTK_TYPE_WIDGET,
                                              "Glyphtest", &glyphtest_info, 0);

    return glyphtest_type;
}

GtkWidget *
glyphtest_new(void)
{
    return GTK_WIDGET(g_object_new(glyphtest_get_type(), NULL));
}

void
glyphtest_add_glyph(Glyphtest *gw, bdf_font_t *font, bdf_glyph_t *glyph)
{
    GlyphtestLine *lp;
    GlyphtestGlyph *gp;

    g_return_if_fail(gw != 0);
    g_return_if_fail(font != 0);
    g_return_if_fail(glyph != 0);

    lp = &gw->line;

    if (lp->glyphs_used == lp->glyphs_size) {
        if (lp->glyphs_size == 0)
          lp->glyphs = (GlyphtestGlyph *)
              g_malloc(sizeof(GlyphtestGlyph) << 3);
        else
          lp->glyphs = (GlyphtestGlyph *)
              g_realloc(lp->glyphs,
                        sizeof(GlyphtestGlyph) * (lp->glyphs_size + 8));
        lp->glyphs_size += 8;
    }
    gp = lp->glyphs + lp->glyphs_used++;
    gp->font = font;
    gp->glyph = glyph;

    if (_glyphtest_set_line_size(gw))
      gtk_widget_queue_resize(GTK_WIDGET(gw));
    else {
        /*
         * Just draw the glyph.
         */
        /*
         * If the first glyph would be drawn off the edge of the window, make
         * sure the initial position is adjusted to display the first glyph at
         * the edge.
         */
        if (gw->dir == GLYPHTEST_LEFT_TO_RIGHT) {
            if (gw->line.glyphs_used == 1 && glyph->bbx.x_offset < 0)
              lp->cpoint.x += -glyph->bbx.x_offset;
        } else {
            if (gw->line.glyphs_used == 1 && glyph->bbx.x_offset > 0 &&
                glyph->bbx.x_offset > glyph->bbx.width)
              lp->cpoint.x -= glyph->bbx.width - glyph->bbx.x_offset;
        }

        _glyphtest_draw_glyph(gw, glyph, font);

        if (gw->dir == GLYPHTEST_LEFT_TO_RIGHT)
          lp->cpoint.x += (font->spacing == BDF_PROPORTIONAL) ?
              glyph->dwidth : font->monowidth;
        else
          lp->cpoint.x -= (font->spacing == BDF_PROPORTIONAL) ?
              glyph->dwidth : font->monowidth;
    }

    /*
     * Call the signal that indicates that a glyph has been added.
     */
    g_signal_emit(GTK_OBJECT(gw), glyphtest_signals[ADD_GLYPH], 0);
}

void
glyphtest_remove_font(Glyphtest *gw, bdf_font_t *font)
{
    GtkWidget *w;
    guint32 i, j;
    gboolean redo;

    g_return_if_fail(gw != 0);
    g_return_if_fail(font != 0);

    w = GTK_WIDGET(gw);

    for (redo = FALSE, i = j = 0; i < gw->line.glyphs_used; i++) {
        if (gw->line.glyphs[i].font != font) {
            gw->line.glyphs[j].font = gw->line.glyphs[i].font;
            gw->line.glyphs[j].glyph = gw->line.glyphs[i].glyph;
            j++;
        }
    }
    if (gw->line.glyphs_used != j) {
        redo = TRUE;
        gw->line.glyphs_used = j;
    }

    if (redo) {
        if (_glyphtest_set_line_size(gw))
          gtk_widget_queue_resize(w);
        else
          glyphtest_draw(w, 0);
    }
}

void
glyphtest_update_device_width(Glyphtest *gw, bdf_font_t *font)
{
    GtkWidget *w;
    guint32 i;
    gboolean redraw;

    g_return_if_fail(gw != 0);

    w = GTK_WIDGET(gw);

    /*
     * Determine if the device width change will cause a redraw.
     */
    redraw = FALSE;

    for (i = 0; redraw == FALSE && i < gw->line.glyphs_used; i++) {
        if (gw->line.glyphs[i].font == font)
          redraw = TRUE;
    }

    if (redraw) {
        /*
         * Determine if a resize is in order or just a redraw.
         */
        if (_glyphtest_set_line_size(gw))
          gtk_widget_queue_resize(w);
        else
          glyphtest_draw(w, 0);
    }
}

void
glyphtest_change_direction(Glyphtest *gw, gint direction)
{
    g_return_if_fail(gw != 0);

    /*
     * Return if the direction is invalid or the same as the current
     * direction.
     */
    if (direction < GLYPHTEST_LEFT_TO_RIGHT ||
        direction > GLYPHTEST_RIGHT_TO_LEFT ||
        direction == gw->dir)
      return;

    gw->dir = direction;
    glyphtest_draw(GTK_WIDGET(gw), 0);
}

void
glyphtest_show_baseline(Glyphtest *gw, gboolean baseline)
{
    g_return_if_fail(gw != 0);

    if (gw->show_baseline == baseline)
      return;

    gw->show_baseline = baseline;
    glyphtest_draw(GTK_WIDGET(gw), 0);
}

void
glyphtest_erase(Glyphtest *gw)
{
    g_return_if_fail(gw != 0);

    /*
     * May change later to shrink the widget.
     */
    gw->line.glyphs_used = 0;
    glyphtest_draw(GTK_WIDGET(gw), 0);
}

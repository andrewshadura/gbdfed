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

#include <stdio.h>
#include <string.h>
#include "grayswatch.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(s) dgettext(GETTEXT_PACKAGE,s)
#else
#define _(s) (s)
#endif

static GtkVBoxClass *parent_class = 0;

#define SWATCH_MIN_WIDTH  20
#define SWATCH_MIN_HEIGHT 100

/*
 * Argument types.
 */
enum {
    PROP_0 = 0,
    PROP_GRAYLEVEL
};

/*
 * Signal names.
 */
enum {
    VALUE_CHANGED = 0
};

static guint grayswatch_signals[VALUE_CHANGED + 1];

/**************************************************************************
 *
 * Class functions.
 *
 **************************************************************************/

static void
value_changed(GtkSpinButton *b, gpointer data)
{
    gint v;
    Grayswatch *gs = GRAYSWATCH(data);
    GtkWidget *sw = gs->swatch;

    v = gtk_spin_button_get_value_as_int(b);

    gs->gray = v;

    memset(gs->image, v, gs->image_size);

    if (GTK_WIDGET_DRAWABLE(sw))
      gdk_draw_gray_image(sw->window,
                          sw->style->fg_gc[GTK_WIDGET_STATE(sw)],
                          GTK_CONTAINER(gs)->border_width,
                          GTK_CONTAINER(gs)->border_width,
                          sw->allocation.width, sw->allocation.height,
                          GDK_RGB_DITHER_NONE, gs->image,
                          sw->allocation.width);

    if (gs->signal_blocked == FALSE)
      /*
       * Now we emit the value_changed signal for this widget.
       */
      g_signal_emit(G_OBJECT(data), grayswatch_signals[VALUE_CHANGED], 0, v);
}

/**************************************************************************
 *
 * Class functions.
 *
 **************************************************************************/

static gboolean
grayswatch_configure(GtkWidget *widget, GdkEventConfigure *event,
                     gpointer data)
{
    Grayswatch *gs = GRAYSWATCH(data);
    gint nbytes;

    nbytes = gs->swatch->allocation.width *
        gs->swatch->allocation.height;
    if (nbytes > gs->image_size) {
        if (gs->image_size == 0)
          gs->image = (guchar *) g_malloc(nbytes);
        else
          gs->image = (guchar *) g_realloc(gs->image, nbytes);
        gs->image_size = nbytes;
    }
    memset(gs->image, gs->gray, gs->image_size);

    return FALSE;
}

static gboolean
grayswatch_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    Grayswatch *gs = GRAYSWATCH(data);

    if (gs->image_size > 0)
      gdk_draw_gray_image(widget->window,
                          widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                          GTK_CONTAINER(gs)->border_width,
                          GTK_CONTAINER(gs)->border_width,
                          widget->allocation.width,
                          widget->allocation.height,
                          GDK_RGB_DITHER_NONE, gs->image,
                          widget->allocation.width);

    return FALSE;
}

static void
grayswatch_get_property(GObject *obj, guint propid, GValue *val,
                        GParamSpec *pspec)
{
    Grayswatch *gs;

    gs = GRAYSWATCH(obj);

    if (propid == PROP_GRAYLEVEL)
      g_value_set_uint(val, gs->gray);
}

static void
grayswatch_set_property(GObject *obj, guint propid, const GValue *val,
                        GParamSpec *pspec)
{
    Grayswatch *gs;
    guint gray;

    gs = GRAYSWATCH(obj);

    if (propid == PROP_GRAYLEVEL) {
        gray = g_value_get_uint(val);
        if (gray > 255)
          gray = 255;
        if (gray != gs->gray)
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(gs->value),
                                    (gdouble) gray);
    }
}

static void
grayswatch_add(GtkContainer *container, GtkWidget *child)
{
    /*
     * This is here as a placeholder as it seems the container doesn't
     * work quite right without it for some reason.
     */
}

static void
grayswatch_remove(GtkContainer *container, GtkWidget *child)
{
    /*
     * This is here as a placeholder as it seems the container doesn't
     * work quite right without it for some reason.
     */
}

static void
grayswatch_foreach(GtkContainer *container, gboolean int_kids,
                   GtkCallback callback, gpointer callback_data)
{
    Grayswatch *gs = GRAYSWATCH(container);

    if (callback != 0) {
        (*callback)(gs->swatch, callback_data);
        (*callback)(gs->value, callback_data);
    }
}

/**************************************************************************
 *
 * Class and object initialization routines.
 *
 **************************************************************************/

static void
grayswatch_class_init(gpointer g_class, gpointer class_data)
{
    GtkContainerClass *cc = GTK_CONTAINER_CLASS(g_class);
    GObjectClass *oc = G_OBJECT_CLASS(g_class);

    cc->forall = grayswatch_foreach;
    cc->add = grayswatch_add;
    cc->remove = grayswatch_remove;

    oc->set_property = grayswatch_set_property;
    oc->get_property = grayswatch_get_property;

    g_object_class_install_property(oc, PROP_GRAYLEVEL,
                                    g_param_spec_uint("grayLevel",
                                                      _("GrayLevel"),
                                                      _("The gray level."),
                                                      0, 255, 0,
                                                      G_PARAM_READWRITE));

    grayswatch_signals[VALUE_CHANGED] =
        g_signal_new("value-changed",
                     G_TYPE_FROM_CLASS(oc),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(GrayswatchClass, value_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__INT,
                     G_TYPE_NONE, 1, G_TYPE_INT);

    parent_class = g_type_class_peek_parent(g_class);
}

static void
grayswatch_init(GTypeInstance *obj, gpointer klass)
{
    Grayswatch *gs = GRAYSWATCH(obj);

    gtk_widget_set_has_window(GTK_WIDGET(obj), FALSE);
    gtk_widget_set_redraw_on_allocate(GTK_WIDGET(obj), FALSE);

    gs->swatch = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(gs->swatch), "configure_event",
                     G_CALLBACK(grayswatch_configure), (gpointer) gs);
    g_signal_connect(G_OBJECT(gs->swatch), "expose_event",
                     G_CALLBACK(grayswatch_expose), (gpointer) gs);
    gtk_box_pack_start(GTK_BOX(obj), gs->swatch, TRUE, TRUE, 0);
    gtk_widget_show(gs->swatch);

    gs->value = gtk_spin_button_new_with_range(0.0, 255.0, 1.0);
    g_signal_connect(G_OBJECT(gs->value), "value-changed",
                     G_CALLBACK(value_changed), (gpointer) gs);

    gtk_box_pack_start(GTK_BOX(obj), gs->value, FALSE, FALSE, 0);
    gtk_widget_show(gs->value);

    gs->gray = 0;
    gs->image_size = 0;
    gs->image = NULL;
}

/**************************************************************************
 *
 * API functions.
 *
 **************************************************************************/

static const GTypeInfo grayswatch_info = {
    sizeof(GrayswatchClass),
    NULL,
    NULL,
    grayswatch_class_init,
    NULL,
    NULL,
    sizeof(Grayswatch),
    0,
    grayswatch_init,
};

GType
grayswatch_get_type(void)
{
    static GType grayswatch_type = 0;

    if (!grayswatch_type)
      grayswatch_type = g_type_register_static(GTK_TYPE_VBOX,
                                               "Grayswatch",
                                               &grayswatch_info, 0);

    return grayswatch_type;
}

GtkWidget *
grayswatch_new(guint gray)
{
    return gtk_widget_new(grayswatch_get_type(),
                          "grayLevel", gray, NULL);
}

void
grayswatch_set_gray(Grayswatch *gs, guint gray)
{
    g_return_if_fail(gs != 0);
    g_return_if_fail(IS_GRAYSWATCH(gs));

    if (gray > 255)
      gray = 255;

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(gs->value), gray);
}

guint
grayswatch_get_gray(Grayswatch *gs)
{
    g_return_val_if_fail(gs != 0, 256);
    g_return_val_if_fail(IS_GRAYSWATCH(gs), 256);

    return gs->gray;
}

void
grayswatch_block_signal(Grayswatch *gs, gboolean block)
{
    g_return_if_fail(gs != 0);
    g_return_if_fail(IS_GRAYSWATCH(gs));

    gs->signal_blocked = block;
}

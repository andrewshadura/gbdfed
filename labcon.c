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

#include "labcon.h"
#include <gtk/gtklabel.h>
#include <gtk/gtkdrawingarea.h>

static GtkContainerClass *parent_class = 0;

static void
labcon_size_request(GtkWidget *w, GtkRequisition *req)
{
    LabCon *l = LABCON(w);
    guint width = 0;
    GtkRequisition l_rec, c_rec;

    l_rec.width = c_rec.width = l_rec.height = c_rec.height = 0;

    if (l->label != 0) {
        if (GTK_WIDGET_VISIBLE(l->label))
          gtk_widget_size_request(l->label, &l_rec);
    } else {
        if (GTK_WIDGET_VISIBLE(l->image))
          gtk_widget_size_request(l->image, &l_rec);
    }
    if (GTK_WIDGET_VISIBLE(l->child))
      gtk_widget_size_request(l->child, &c_rec);

    if (l->leader)
      width = LABCON(l->leader)->label_width;
    width = MAX(width, l_rec.width);

    req->height = MAX(l_rec.height, c_rec.height) +
        (GTK_CONTAINER(l)->border_width * 2);
    req->width = width + c_rec.width + l->spacing +
        (GTK_CONTAINER(l)->border_width * 2);
}

static void
labcon_size_allocate(GtkWidget *w, GtkAllocation *all)
{
    guint i;
    GtkWidget *label;
    LabCon *leader, *l = LABCON(w);
    GtkRequisition l_rec, c_rec, ll_rec;
    GtkAllocation w_all;

    w->allocation = *all;

    l_rec.width = c_rec.width = l_rec.height = c_rec.height = 0;

    label = (l->label != 0) ? l->label : l->image;

    if (GTK_WIDGET_VISIBLE(label))
      gtk_widget_get_child_requisition(label, &l_rec);
    if (GTK_WIDGET_VISIBLE(l->child))
      gtk_widget_get_child_requisition(l->child, &c_rec);

    /*
     * Make sure the height is non-zero and leaves the border.
     */
    w_all.x = all->x + GTK_CONTAINER(l)->border_width;
    w_all.y = all->y + GTK_CONTAINER(l)->border_width;
    w_all.height = MAX(1, (gint) all->height -
                       (gint) (GTK_CONTAINER(l)->border_width * 2));

    if (l->leader != 0) {
        leader = LABCON(l->leader);

        if (leader->label)
          gtk_widget_get_child_requisition(leader->label, &ll_rec);
        else
          gtk_widget_get_child_requisition(leader->image, &ll_rec);

        if (ll_rec.width < l_rec.width) {
            if (leader->label)
              gtk_widget_set_size_request(leader->label,
                                          l_rec.width, ll_rec.height);
            else
              gtk_widget_set_size_request(leader->image,
                                          l_rec.width, ll_rec.height);
        }

        leader->label_width = MAX(l_rec.width, leader->label_width);
        l_rec.width = leader->label_width;

        for (i = 0; i < leader->group_used - 1; i++) {
            if (LABCON(leader->group[i])->label)
              gtk_widget_get_child_requisition(LABCON(leader->group[i])->label,
                                               &ll_rec);
            else
              gtk_widget_get_child_requisition(LABCON(leader->group[i])->image,
                                               &ll_rec);
            if (ll_rec.width < l_rec.width) {
                if (LABCON(leader->group[i])->label)
                  gtk_widget_set_size_request(LABCON(leader->group[i])->label,
                                              l_rec.width, ll_rec.height);
                else
                  gtk_widget_set_size_request(LABCON(leader->group[i])->image,
                                              l_rec.width, ll_rec.height);
            }
        }
    } else
      l->label_width = l_rec.width;

    if (l->pos == GTK_POS_LEFT) {
        /*
         * Position the label on the left of the child.
         */

        /*
         * Calculate the allocation for the label widget.
         */
        w_all.width = l_rec.width;
        gtk_widget_size_allocate(label, &w_all);

        /*
         * Calculate the allocation for the child widget. The child widget
         * is expanded to fill the remaining space.
         */
        w_all.x += w_all.width + l->spacing;
        w_all.width = all->width - (l_rec.width + l->spacing);
        gtk_widget_size_allocate(l->child, &w_all);
    } else {
        /*
         * Position the label on the right of the child.
         */

        /*
         * Calculate the allocation for the child widget. The child widget
         * is expanded to fill the remaining space.
         */
        w_all.width = all->width - (l_rec.width + l->spacing);
        gtk_widget_size_allocate(l->child, &w_all);

        /*
         * Calculate the allocation for the label widget.
         */
        w_all.x += w_all.width + l->spacing;
        w_all.width = l_rec.width;
        gtk_widget_size_allocate(label, &w_all);
    }
}

static void
labcon_forall(GtkContainer *c, gboolean include_internals,
              GtkCallback callback, gpointer callback_data)
{
    LabCon *l = LABCON(c);

    if (l->label)
      (*callback)(l->label, callback_data);
    else
      (*callback)(l->image, callback_data);
    (*callback)(l->child, callback_data);
}

static void
labcon_remove(GtkContainer *c, GtkWidget *w)
{
    guint i;
    LabCon *l = LABCON(c);

    /*
     * Make sure that the group list has been deallocated so we don't
     * leak memory.
     */
    if (l->child == w) {
        if (l->group_size > 0) {
          for (i = 0; i < l->group_used; i++)
            LABCON(l->group[i])->leader = 0;
          l->group_size = l->group_used = 0;
          g_free(l->group);
          l->group = 0;
        }
    }
}

static void
labcon_class_init(gpointer g_class, gpointer class_data)
{
    GtkWidgetClass *wc = GTK_WIDGET_CLASS(g_class);
    LabConClass *lc = LABCON_CLASS(g_class);
    GtkContainerClass *cc = GTK_CONTAINER_CLASS(g_class);

    wc->size_request = labcon_size_request;
    wc->size_allocate = labcon_size_allocate;

    cc->remove = labcon_remove;
    cc->forall = labcon_forall;

    parent_class = g_type_class_peek_parent(lc);
}

static void
labcon_init(GTypeInstance *instance, gpointer g_class)
{
    LabCon *l = LABCON(instance);

    GTK_WIDGET_SET_FLAGS(l, GTK_NO_WINDOW);
    gtk_widget_set_redraw_on_allocate(GTK_WIDGET(l), FALSE);

    l->pixbuf = 0;
    l->image = l->label = l->child = 0;
    l->spacing = 0;
    l->align = LABCON_ALIGN_LEFT;
    l->pos = GTK_POS_LEFT;
    l->label_width = 0;
    l->leader = 0;
    l->group_size = l->group_used = 0;
}

static gboolean
draw_pixbuf(GtkWidget *w, GdkEventExpose *event, gpointer data)
{
    GdkPixbuf *p = GDK_PIXBUF(data);
    gint x, y, wd, ht;

    wd = gdk_pixbuf_get_width(p);
    ht = gdk_pixbuf_get_height(p);

    x = (w->allocation.width >> 1) - (wd >> 1);
    y = (w->allocation.height >> 1) - (ht >> 1);
    gdk_draw_pixbuf(w->window, w->style->fg_gc[GTK_WIDGET_STATE(w)],
                    p, 0, 0, x, y, wd, ht, GDK_RGB_DITHER_NONE, 0, 0);

    return FALSE;
}

/**********************************************************************
 *
 * API functions.
 *
 **********************************************************************/

GType
labcon_get_type(void)
{
    static GType labcon_type = 0;
  
    if (!labcon_type) {
        static const GTypeInfo labcon_info = {
            sizeof (LabConClass),		/* class_size		*/
            0,					/* base_init		*/
            0,					/* base_finalize	*/
            labcon_class_init,			/* class_init		*/
            0,					/* class_finalize	*/
            0,					/* class_data		*/
            sizeof(LabCon),			/* instance_size	*/
            0,					/* n_preallocs		*/
            labcon_init,			/* instance_init	*/
            0,					/* value_table		*/
        };

        labcon_type = g_type_register_static(GTK_TYPE_CONTAINER, "LabCon",
                                             &labcon_info, 0);
    }
  
    return labcon_type;
}

GtkWidget *
labcon_new_label(const gchar *label, LabConAlignment align,
                 GtkPositionType pos, guint spacing,
                 GtkWidget *child, GtkWidget *group)
{
    LabCon *l, *leader = LABCON(group);

    g_return_val_if_fail(GTK_IS_WIDGET(child), NULL);
    if (group) {
        g_return_val_if_fail(GTK_IS_WIDGET(group), NULL);
        g_return_val_if_fail(IS_LABCON(group), NULL);
    }

    l = g_object_new(labcon_get_type(), NULL);
    l->pos = pos;
    l->spacing = spacing;
    l->child = child;

    l->label = gtk_label_new(label);
    switch (align) {
      case LABCON_ALIGN_LEFT:
        gtk_misc_set_alignment(GTK_MISC(l->label), 0.0, 0.5);
        break;
      case LABCON_ALIGN_RIGHT:
        gtk_misc_set_alignment(GTK_MISC(l->label), 1.0, 0.5);
        break;
      case LABCON_ALIGN_CENTER:
        gtk_misc_set_alignment(GTK_MISC(l->label), 0.5, 0.5);
        break;
    }

    /*
     * Go back until we get the group leader.
     */
    if (group && LABCON(group)->leader)
      l->leader = LABCON(group)->leader;
    else
      l->leader = group;

    if (l->leader) {
        /*
         * Add this widget to the group which should have all the same
         * width labels.
         */
        leader = LABCON(l->leader);
        if (leader->group_size == leader->group_used) {
            if (leader->group_size == 0)
              leader->group = (GtkWidget **) g_malloc(sizeof(GtkWidget *) * 4);
            else
              leader->group = (GtkWidget **)
                  g_realloc(leader->group,
                            sizeof(GtkWidget *) * (leader->group_size + 4));
            leader->group_size += 4;
        }
        leader->group[leader->group_used++] = GTK_WIDGET(l);
    }

    gtk_widget_set_parent(l->label, GTK_WIDGET(l)); 
    gtk_widget_show(l->label);
    if (l->child) {
        gtk_widget_set_parent(l->child, GTK_WIDGET(l)); 
        gtk_widget_show(l->child);
    }

    return GTK_WIDGET(l);
}

GtkWidget *
labcon_new_label_defaults(const gchar *label, GtkWidget *child,
                          GtkWidget *group)
{
    return labcon_new_label(label, LABCON_ALIGN_RIGHT, GTK_POS_LEFT, 5,
                            child, group);
}

GtkWidget *
labcon_new_pixbuf(const GdkPixbuf *pixbuf, LabConAlignment align,
                  GtkPositionType pos, guint spacing,
                  GtkWidget *child, GtkWidget *group)
{
    LabCon *l, *leader = LABCON(group);

    g_return_val_if_fail(GTK_IS_WIDGET(child), NULL);
    if (group) {
        g_return_val_if_fail(GTK_IS_WIDGET(group), NULL);
        g_return_val_if_fail(IS_LABCON(group), NULL);
    }

    l = g_object_new(labcon_get_type(), NULL);
    l->pixbuf = pixbuf;
    l->pos = pos;
    l->spacing = spacing;
    l->child = child;
    l->align = align;

    /*
     * Make the drawing area just big enough to hold the pixbuf at
     * first.
     */
    l->image = gtk_drawing_area_new();
    gtk_widget_set_size_request(l->image, 
                                gdk_pixbuf_get_width(l->pixbuf),
                                gdk_pixbuf_get_height(l->pixbuf));
    g_signal_connect(G_OBJECT(l->image), "expose_event",
                     G_CALLBACK(draw_pixbuf), (gpointer) l->pixbuf);

    /*
     * Go back until we get the group leader.
     */
    if (group && LABCON(group)->leader)
      l->leader = LABCON(group)->leader;
    else
      l->leader = group;

    if (l->leader) {
        /*
         * Add this widget to the group which should have all the same
         * width labels or pixbufs.
         */
        leader = LABCON(l->leader);
        if (leader->group_size == leader->group_used) {
            if (leader->group_size == 0)
              leader->group = (GtkWidget **) g_malloc(sizeof(GtkWidget *) * 4);
            else
              leader->group = (GtkWidget **)
                  g_realloc(leader->group,
                            sizeof(GtkWidget *) * (leader->group_size + 4));
            leader->group_size += 4;
        }
        leader->group[leader->group_used++] = GTK_WIDGET(l);
    }

    gtk_widget_set_parent(l->image, GTK_WIDGET(l)); 
    gtk_widget_show(l->image);
    if (l->child) {
        gtk_widget_set_parent(l->child, GTK_WIDGET(l)); 
        gtk_widget_show(l->child);
    }

    return GTK_WIDGET(l);
}

GtkWidget *
labcon_new_pixbuf_defaults(const GdkPixbuf *pixbuf, GtkWidget *child,
                           GtkWidget *group)
{
    return labcon_new_pixbuf(pixbuf, LABCON_ALIGN_RIGHT, GTK_POS_LEFT, 5,
                             child, group);
}

const GdkPixbuf *
labcon_get_pixbuf(LabCon *l)
{
    g_return_val_if_fail(IS_LABCON(l), 0);

    return l ? l->pixbuf : 0;
}

GtkWidget *
labcon_get_image(LabCon *l)
{
    g_return_val_if_fail(IS_LABCON(l), 0);

    return l ? l->image : 0;
}

GtkWidget *
labcon_get_label(LabCon *l)
{
    g_return_val_if_fail(IS_LABCON(l), 0);

    return l ? l->label : 0;
}

GtkWidget *
labcon_get_child(LabCon *l)
{
    g_return_val_if_fail(IS_LABCON(l), 0);

    return l ? l->child : 0;
}

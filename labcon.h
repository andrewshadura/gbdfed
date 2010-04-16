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
#ifndef _h_labcon
#define _h_labcon

#include <gtk/gtkcontainer.h>
#include "gtkcompat.h"

G_BEGIN_DECLS

/*
 * The LabCon (Labeled Container) widget. Created to provide a container
 * that contains a label and a single child, and can be part of a group
 * of other LabCon widgets. All members of the group have the same label
 * width. The labels will be on the left or the right side of the child
 * widget and can be aligned to the right, left, or center.
 */

#define LABCON(obj) \
      (G_TYPE_CHECK_INSTANCE_CAST(obj, labcon_get_type(), LabCon))
#define LABCON_CLASS(klass) \
      (G_TYPE_CHECK_CLASS_CAST(klass, labcon_get_type(), LabConClass))

#define IS_LABCON(obj) \
      (G_TYPE_CHECK_INSTANCE_TYPE(obj, labcon_get_type()))
#define IS_LABCON_CLASS(klass) \
      (G_TYPE_CHECK_CLASS_TYPE(klass, labcon_get_type()))

#define LABCON_GET_CLASS(obj) \
      (G_TYPE_INSTANCE_GET_CLASS(obj, labcon_get_type(), LabConClass))

typedef struct _LabCon      LabCon;
typedef struct _LabConClass LabConClass;

typedef enum {
    LABCON_ALIGN_LEFT,
    LABCON_ALIGN_RIGHT,
    LABCON_ALIGN_CENTER
} LabConAlignment;

struct _LabCon {
    GtkContainer container;

    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *child;

    /*
     * The pixbuf that will be shown as the image instead of a label.
     */
    const GdkPixbuf *pixbuf;

    /*
     * Spacing between the label and the child widget.
     */
    guint spacing;

    /*
     * Alignment.
     */
    LabConAlignment align;

    /*
     * Label positioning (GTK_POS_LEFT or GTK_POS_RIGHT).
     */
    GtkPositionType pos;

    /*
     * The current width all labels should be.
     */
    guint label_width;

    /*
     * All the widgets that should have the same width labels.
     */
    GtkWidget *leader;
    GtkWidget **group;
    guint group_size;
    guint group_used;
};

struct _LabConClass {
    GtkContainerClass parent_class;
};

/**********************************************************************
 *
 * API functions.
 *
 **********************************************************************/

extern GType labcon_get_type(void);

extern GtkWidget *labcon_new_label(const gchar *label, LabConAlignment align,
                                   GtkPositionType pos, guint spacing,
                                   GtkWidget *child, GtkWidget *group);

extern GtkWidget *labcon_new_label_defaults(const gchar *label,
                                            GtkWidget *child,
                                            GtkWidget *group);

extern GtkWidget *labcon_new_pixbuf(const GdkPixbuf *pixbuf,
                                    LabConAlignment align,
                                    GtkPositionType pos,
                                    guint spacing,
                                    GtkWidget *child,
                                    GtkWidget *group);

extern GtkWidget *labcon_new_pixbuf_defaults(const GdkPixbuf *pixbuf,
                                             GtkWidget *child,
                                             GtkWidget *group);

extern const GdkPixbuf *labcon_get_pixbuf(LabCon *l);
extern GtkWidget *labcon_get_image(LabCon *l);
extern GtkWidget *labcon_get_label(LabCon *l);
extern GtkWidget *labcon_get_child(LabCon *l);

G_END_DECLS

#endif /* _h_labcon */

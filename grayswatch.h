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
#ifndef _h_grayswatch
#define _h_grayswatch

#include <gtk/gtkvbox.h>
#include "gtkcompat.h"

G_BEGIN_DECLS

/*
 * The macros for accessing various parts of the widget class.
 */
#define GRAYSWATCH(obj)(G_TYPE_CHECK_INSTANCE_CAST(obj, grayswatch_get_type(), Grayswatch))
#define GRAYSWATCH_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST(klass, grayswatch_get_type(), GrayswatchClass))
#define IS_GRAYSWATCH(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, grayswatch_get_type())
#define IS_GRAYSWATCH_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE(klass, grayswatch_get_type()))
#define GRAYSWATCH_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS(obj, grayswatch_get_type(), GrayswatchClass))

typedef struct _Grayswatch      Grayswatch;
typedef struct _GrayswatchClass GrayswatchClass;

struct _Grayswatch {
    GtkVBox vbox;

    /*
     * Public members.
     */
    guint gray;

    /*
     * Private members.
     */
    GtkWidget *swatch;
    GtkWidget *value;

    /*
     * The grayscale image used to draw the swatch color.
     */
    guchar *image;
    guint image_size;

    /*
     * Flag indicating whether the signal should be blocked or not.
     */
    gboolean signal_blocked;
};

struct _GrayswatchClass {
    GtkVBoxClass parent_class;

    /*
     * Signal handlers.
     */
    void (*value_changed)(GtkWidget *, gint, gpointer);
};

/**************************************************************************
 *
 * General API
 *
 **************************************************************************/

extern GType grayswatch_get_type(void);

extern GtkWidget *grayswatch_new(guint gray);

/*
 * The gray value is between 0 and 255.
 */
extern void grayswatch_set_gray(Grayswatch *gs, guint gray);

/*
 * Anything over 255 returned by this routine means an error.
 */
extern guint grayswatch_get_gray(Grayswatch *gs);

/*
 * Block the signal from being emitted while setting the gray level.
 * This is for a specific application and may not be useful anywhere else.
 * No error checking is done to make sure blocking and unblocking are
 * done in pairs.
 */
extern void grayswatch_block_signal(Grayswatch *gs, gboolean block);

G_END_DECLS

#endif /* _h_grayswatch */

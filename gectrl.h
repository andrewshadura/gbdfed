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
#ifndef _h_gectrl
#define _h_gectrl

#include <gtk/gtkdrawingarea.h>
#include "bdfP.h"
#include "gtkcompat.h"

G_BEGIN_DECLS

#define GECONTROL(obj) \
      (G_TYPE_CHECK_INSTANCE_CAST(obj, gecontrol_get_type(), GEControl))
#define GECONTROL_CLASS(klass) \
      (G_TYPE_CHECK_CLASS_CAST(klass, gecontrol_get_type(), GEControlClass))

#define IS_GECONTROL(obj) \
      (G_TYPE_CHECK_INSTANCE_TYPE(obj, gecontrol_get_type()))
#define IS_GECONTROL_CLASS(klass) \
      (G_TYPE_CHECK_CLASS_TYPE(klass, gecontrol_get_type()))

#define GECONTROL_GET_CLASS(obj) \
      (G_TYPE_INSTANCE_GET_CLASS(obj, gecontrol_get_type(), GEControlClass))

typedef struct _GEControl      GEControl;
typedef struct _GEControlClass GEControlClass;

typedef enum {
    GECONTROL_DRAW = 0,
    GECONTROL_MOVE,
    GECONTROL_COPY,
    GECONTROL_FLIP_HORIZONTAL,
    GECONTROL_FLIP_VERTICAL,
    GECONTROL_SHEAR,
    GECONTROL_ROTATE_LEFT_90,
    GECONTROL_ROTATE_RIGHT_90,
    GECONTROL_ROTATE,
    GECONTROL_SHIFT_UP_LEFT,
    GECONTROL_SHIFT_UP,
    GECONTROL_SHIFT_UP_RIGHT,
    GECONTROL_SHIFT_LEFT,
    GECONTROL_SHIFT_RIGHT,
    GECONTROL_SHIFT_DOWN_LEFT,
    GECONTROL_SHIFT_DOWN,
    GECONTROL_SHIFT_DOWN_RIGHT,
    GECONTROL_COLOR_CHANGE
} GEControlOperation;

/*
 * Structure passed to the "activate" signal handler.
 */
typedef struct {
    GEControlOperation operation;
    gint color;
} GEControlActivateInfo;

typedef struct {
    guchar *help;
    gint x;
    gint y;
    GdkPixbuf *image;
    GdkRegion *region;
    gint other_toggles[2];
    gboolean set;
    gboolean toggle;
} GEControlButton;

typedef struct {
    gint x;
    gint y;
    GdkRegion *region;
    gboolean set;
} GEControlColor;

struct _GEControl {
    GtkDrawingArea da;

    /*
     * Public fields.
     */

    /*
     * The glyph image.
     */
    bdf_bitmap_t *gimage;

    /*
     * An application provided label widget where the help
     * messages are set.
     */
    GtkWidget *tip_label;

    /*
     * The list of colors to use.
     */
    guint16 *colors;

    /*
     * Private fields.
     */
    gint last_button;

    /*
     * The current color index.
     */
    gint cidx;

    /*
     * 16 color spots. Used to track mouse position and update the tip label.
     */
    GdkRectangle spot;
    GdkRegion *spot_region;

    /*
     * Timer stuff for holding down the buttons.
     */
    gint timer_count;
    gint timer_button;
    guint timer;

    GdkPoint *points;
    gint points_used;
    gint points_size;

    /*
     * Buffer for building a grayscale glyph image.
     */
    guchar *rgb;
    guint rgb_used;
    guint rgb_size;

    GEControlButton buttons[18];
};

struct _GEControlClass {
    GtkDrawingAreaClass parent_class;

    /*
     * A GC for drawing the color selection rectangle.
     */
    GdkGC *selgc;

    GdkPixbuf *draw;
    GdkPixbuf *move;
    GdkPixbuf *copy;

    GdkPixbuf *fliph;
    GdkPixbuf *flipv;
    GdkPixbuf *shear;

    GdkPixbuf *rleft;
    GdkPixbuf *rright;
    GdkPixbuf *rotate;

    GdkPixbuf *uleft;
    GdkPixbuf *up;
    GdkPixbuf *uright;

    GdkPixbuf *left;
    GdkPixbuf *right;

    GdkPixbuf *dleft;
    GdkPixbuf *down;
    GdkPixbuf *dright;

    /*
     * Signal handlers.
     */
    void (*activate)(GtkWidget *, gpointer, gpointer);
};

extern GType gecontrol_get_type(void);

extern GtkWidget *gecontrol_new(const gchar *prop1, ...);

extern GtkWidget *gecontrol_newv(GtkWidget *tips_label, bdf_bitmap_t *image,
                                 guint16 *colors);

extern void gecontrol_set_glyph_image(GEControl *ge, bdf_bitmap_t *image);

extern void gecontrol_update_glyph_image(GEControl *ge, bdf_bitmap_t *image);

extern void gecontrol_set_color_list(GEControl *ge, guint16 *colors);

extern void gecontrol_change_operation(GEControl *ge, GEControlOperation op);

extern void gecontrol_change_color(GEControl *ge, gint cidx);

G_END_DECLS

#endif /* _h_gectrl */

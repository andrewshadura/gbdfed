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
#ifndef _h_glyphtest
#define _h_glyphtest

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtksignal.h>
#include "bdfP.h"
#include "gtkcompat.h"

G_BEGIN_DECLS

/*
 * The macros for accessing various parts of the widget class.
 */
#define GLYPHTEST(o) \
        (G_TYPE_CHECK_INSTANCE_CAST((o), glyphtest_get_type(), Glyphtest))

#define GLYPHTEST_CLASS(c) \
        (G_TYPE_CHECK_CLASS_CAST((c), glyphtest_get_type(), GlyphtestClass))

#define IS_GLYPHTEST(o) G_TYPE_CHECK_INSTANCE_TYPE((o), glyphtest_get_type())

#define IS_GLYPHTEST_CLASS(c) \
        (G_TYPE_CHECK_CLASS_TYPE((c), glyphtest_get_type()))

#define GLYPHTEST_GET_CLASS(o) \
        (G_TYPE_INSTANCE_GET_CLASS((o), glyphtest_get_type(), GlyphtestClass))

typedef struct _Glyphtest      Glyphtest;
typedef struct _GlyphtestClass GlyphtestClass;

typedef struct {
    bdf_font_t *font;
    bdf_glyph_t *glyph;
} GlyphtestGlyph;

typedef struct {
    GlyphtestGlyph *glyphs;
    guint32 glyphs_used;
    guint32 glyphs_size;
    GdkPoint cpoint;
    guint16 width;
    guint16 height;
    bdf_bbx_t bbx;
} GlyphtestLine;

struct _Glyphtest {
    GtkWidget widget;

    /*
     * Public members.
     */
    guint dir;
    gboolean show_baseline;
#if 0
    /* ENABLE WHEN COLORS ARE WORKING. */
    gulong *colors;
#endif

    /*
     * Private members.
     */
    GlyphtestLine line;

    /*
     * The points used to draw the glyphs.
     */
    GdkPoint *image;
    guint32 image_used;
    guint32 image_size;

    guint16 vmargin;
    guint16 hmargin;
    guint16 focus_thickness;
};

struct _GlyphtestClass {
    GtkWidgetClass parent_class;

    void (*glyph_added)(GtkWidget *, gpointer);
};

/**************************************************************************
 *
 * General API
 *
 **************************************************************************/

extern GType glyphtest_get_type(void);
extern GtkWidget *glyphtest_new(void);

/*
 * Direction values for the change_direction call.
 */
#define GLYPHTEST_LEFT_TO_RIGHT 0
#define GLYPHTEST_RIGHT_TO_LEFT 1

extern void glyphtest_add_glyph(Glyphtest *, bdf_font_t *, bdf_glyph_t *);
extern void glyphtest_remove_font(Glyphtest *, bdf_font_t *);
extern void glyphtest_update_device_width(Glyphtest *, bdf_font_t *);
extern void glyphtest_change_direction(Glyphtest *, gint direction);
extern void glyphtest_show_baseline(Glyphtest *, gboolean baseline);
extern void glyphtest_erase(Glyphtest *);

G_END_DECLS

#endif /* _h_glyphtest */

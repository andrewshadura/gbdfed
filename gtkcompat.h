/*
 * Copyright 2010 Department of Mathematical Sciences, New Mexico State University
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
#ifndef _h_gtkcompat
#define _h_gtkcompat

#include <gtk/gtkversion.h>

#if GTK_MAJOR_VERSION >= (2) && GTK_MINOR_VERSION >= (20)
#define GTK_WIDGET_REALIZED gtk_widget_get_realized
#define GTK_WIDGET_STATE gtk_widget_get_state
#define GTK_WIDGET_HAS_FOCUS gtk_widget_has_focus
#define GTK_WIDGET_DRAWABLE gtk_widget_is_drawable
#define GTK_WIDGET_IS_SENSITIVE gtk_widget_is_sensitive
#define GTK_WIDGET_SENSITIVE gtk_widget_get_sensitive
#define GTK_WIDGET_VISIBLE gtk_widget_get_visible
#endif

#endif /* _h_gtkcompat */

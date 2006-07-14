/*
 * Copyright 2006 Computing Research Labs, New Mexico State University
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
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef lint
#ifdef __GNUC__
static char svnid[] __attribute__ ((unused)) = "$Id: guiutil.c 25 2006-01-11 23:04:48Z mleisher $";
#else
static char svnid[] = "$Id: guiutil.c 25 2006-01-11 23:04:48Z mleisher $";
#endif
#endif

#include "gbdfed.h"

/*
 * The shared error dialog and the label for the message.
 */
static GtkWidget *errd;
static GtkWidget *errmsg;

/*
 * The shared question dialog, the label for the question, and the
 * value representing the answer.
 */
static GtkWidget *questd;
static GtkWidget *question;
static GtkWidget *yes;
static GtkWidget *no;

void
guiutil_show_dialog_centered(GtkWidget *dialog, GtkWidget *parent)
{
    if (GTK_WINDOW(parent) != gtk_window_get_transient_for(GTK_WINDOW(dialog)))
      gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
    gtk_widget_show(dialog);
}

void
guiutil_error_message(GtkWidget *parent, gchar *text)
{
    GtkWidget *ok, *hbox, *image;
    GtkStockItem item;

    if (errd == 0) {
        errd = gtk_dialog_new();
        (void) g_signal_connect(G_OBJECT(errd), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        hbox = gtk_hbox_new(FALSE, 0);

        /*
         * Create the error icon.
         */
        if (gtk_stock_lookup(GTK_STOCK_DIALOG_ERROR, &item)) {
            /*
             * Set the dialog title.
             */
            gtk_window_set_title(GTK_WINDOW(errd), item.label);
            image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR,
                                             GTK_ICON_SIZE_DIALOG);
            gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 2);
        } else {
            /*
             * Probably not necessary, but use some default icon here.
             */
        }

        errmsg = gtk_label_new(text);
        gtk_box_pack_start(GTK_BOX(hbox), errmsg, TRUE, TRUE, 2);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(errd)->vbox), hbox);

        ok = gtk_dialog_add_button(GTK_DIALOG(errd), GTK_STOCK_CLOSE,
                                   GTK_RESPONSE_CLOSE);
        (void) g_signal_connect_object(G_OBJECT(ok), "clicked",
                                       G_CALLBACK(gtk_widget_hide),
                                       (gpointer) errd,
                                       G_CONNECT_SWAPPED);

        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(errd)->action_area), ok);

        gtk_widget_show_all(errd);

        gtk_window_set_modal(GTK_WINDOW(errd), TRUE);
    } else
      gtk_label_set_text(GTK_LABEL(errmsg), text);

    /*
     * Center the dialog and display it.
     */
    guiutil_show_dialog_centered(errd, parent);

    /*
     * Ring the bell.
     */
    gdk_beep();
}

gboolean
guiutil_yes_or_no(GtkWidget *parent, gchar *text, gboolean default_answer)
{
    GtkWidget *hbox, *image;
    GList *kids;
    gint ans;
    GtkStockItem item;

    if (questd == 0) {
        questd = gtk_dialog_new();
        gtk_window_set_resizable(GTK_WINDOW(questd), TRUE);

        (void) g_signal_connect(G_OBJECT(questd), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        hbox = gtk_hbox_new(FALSE, 0);

        /*
         * Create the question icon.
         */
        if (gtk_stock_lookup(GTK_STOCK_DIALOG_QUESTION, &item)) {
            /*
             * Set the dialog title.
             */
            gtk_window_set_title(GTK_WINDOW(questd), item.label);
            image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
                                             GTK_ICON_SIZE_DIALOG);
            gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 2);
        } else {
            /*
             * Probably not necessary, but use some default icon here.
             */
        }

        question = gtk_label_new(text);

        gtk_box_pack_start(GTK_BOX(hbox), question, TRUE, TRUE, 2);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(questd)->vbox), hbox);

        /*
         * Make sure the buttons are evenly distributed in the button box.
         */
        gtk_button_box_set_layout(GTK_BUTTON_BOX(GTK_DIALOG(questd)->action_area), GTK_BUTTONBOX_SPREAD);

        gtk_dialog_add_buttons(GTK_DIALOG(questd),
                               GTK_STOCK_YES, GTK_RESPONSE_ACCEPT,
                               GTK_STOCK_NO, GTK_RESPONSE_CANCEL, 0);

        /*
         * Get the two children buttons out now so focus can be set on either
         * when needed.
         */
        kids = gtk_container_get_children(GTK_CONTAINER(GTK_DIALOG(questd)->action_area));
        no = GTK_WIDGET(g_list_nth_data(kids, 0));
        yes = GTK_WIDGET(g_list_nth_data(kids, 1));
        g_list_free(kids);

        gtk_widget_show_all(questd);

        gtk_window_set_modal(GTK_WINDOW(questd), TRUE);
    } else
      gtk_label_set_text(GTK_LABEL(question), text);

    /*
     * Force the dialog to reset to its minimum size.
     */
    if (questd->window != NULL)
      gtk_window_resize(GTK_WINDOW(questd), 1, 1);

    /*
     * Center the dialog and display it.
     */
    guiutil_show_dialog_centered(questd, parent);

    /*
     * Force the default answer button to have the focus.
     */
    if (default_answer)
      gtk_widget_grab_focus(yes);
    else
      gtk_widget_grab_focus(no);

    ans = gtk_dialog_run(GTK_DIALOG(questd));

    gtk_widget_hide(questd);

    return (ans == GTK_RESPONSE_ACCEPT) ? TRUE : FALSE;
}

void
guiutil_util_set_tooltip(GtkWidget *w, gchar *text)
{
    GtkTooltips *tt;

    tt = gtk_tooltips_new();
    gtk_tooltips_set_tip(tt, w, text, 0);
}

static GdkCursor *watch_cursor;

void
guiutil_busy_cursor(GtkWidget *w, gboolean on)
{
    if (watch_cursor == 0)
      watch_cursor = gdk_cursor_new(GDK_WATCH);

    if (on)
      gdk_window_set_cursor(w->window, watch_cursor);
    else
      gdk_window_set_cursor(w->window, 0);
}

void
guiutil_cursor_cleanup()
{
    if (watch_cursor != 0)
      gdk_cursor_unref(watch_cursor);
    watch_cursor = 0;
}

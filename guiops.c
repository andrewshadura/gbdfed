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

#include "gbdfed.h"

static GtkWidget *ops_dialog;
static GtkWidget *ops_notebook;
static GtkWidget *ops_dx;
static GtkWidget *ops_dy;
static GtkWidget *ops_rotate;
static GtkWidget *ops_shear;
static GtkWidget *ops_degrees;
static GtkWidget *ops_all_glyphs;
static GtkWidget *ops_selected_glyphs;
static GtkWidget *ops_apply;
static gboolean translate_changed;
static gboolean degrees_changed;
static gboolean embolden_changed;
static gint page;

static void
apply_operation(GtkWidget *w, gint response, gpointer data)
{
    gbdfed_editor_t *ed;
    gint16 dx, dy;
    gboolean all;

    /*
     * Turn off the Apply button for this page.
     */
    gtk_widget_set_sensitive(ops_apply, FALSE);

    if (response == GTK_RESPONSE_CLOSE) {
        gtk_widget_hide(ops_dialog);
        gtk_widget_set_sensitive(ops_apply, FALSE);
        translate_changed = degrees_changed = embolden_changed = FALSE;
        return;
    }

    ed = editors +
        GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(w), "editor"));

    all = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ops_all_glyphs));

    if (translate_changed) {
        /*
         * Do a glyph translate.
         */
        dx = _bdf_atos((char *) gtk_entry_get_text(GTK_ENTRY(ops_dx)), 0, 10);
        dy = _bdf_atos((char *) gtk_entry_get_text(GTK_ENTRY(ops_dy)), 0, 10);
        fontgrid_translate_glyphs(FONTGRID(ed->fgrid), dx, dy, all);
        translate_changed = FALSE;
    }

    if (degrees_changed) {
        /*
         * Do either a rotate or a shear.
         */
        dx = _bdf_atos((char *) gtk_entry_get_text(GTK_ENTRY(ops_degrees)),
                       0, 10);
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ops_rotate)))
          fontgrid_rotate_glyphs(FONTGRID(ed->fgrid), dx, all);
        else
          fontgrid_shear_glyphs(FONTGRID(ed->fgrid), dx, all);
        degrees_changed = FALSE;
    }

    if (embolden_changed) {
        /*
         * Embolden some glyphs.
         */
        fontgrid_embolden_glyphs(FONTGRID(ed->fgrid), all);
        embolden_changed = FALSE;
    }
}

static void
notebook_switch_page(GtkNotebook *nb, gpointer *nbp, gint pageno,
                     gpointer data)
{
    GtkWidget *which = 0;

    /*
     * Force the focus on the entry fields when these pages become visible.
     */
    switch (pageno) {
      case 0:
        if (ops_apply != 0)
          gtk_widget_set_sensitive(ops_apply, translate_changed);
        which = ops_dx;
        break;
      case 1:
        if (ops_apply != 0)
          gtk_widget_set_sensitive(ops_apply, degrees_changed);
        which = ops_degrees;
        break;
      case 2:
        if (ops_apply != 0)
          gtk_widget_set_sensitive(ops_apply, embolden_changed);
        which = 0;
    }
    page = pageno;

    if (which)
      gtk_widget_grab_focus(which);
}

static void
enable_apply(GtkWidget *w, gpointer data)
{
    gint which = GPOINTER_TO_INT(data);
    gboolean val = TRUE;

    if (which == 0)
      translate_changed = TRUE;
    else if (which == 1)
      degrees_changed = TRUE;
    else {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
          embolden_changed = TRUE;
        else
          embolden_changed = FALSE;
        val = embolden_changed;
    }

    gtk_widget_set_sensitive(ops_apply, val);
}

static void
ops_dialog_setup(gbdfed_editor_t *ed)
{
    GtkWidget *vbox, *hbox, *vbox1, *label, *table, *button;
    GtkRadioButton *rb;
    GtkAdjustment *adj;

    if (ops_dialog == 0) {
        translate_changed = degrees_changed = FALSE;

        ops_dialog = gtk_dialog_new();
        (void) g_signal_connect(G_OBJECT(ops_dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        vbox = gtk_vbox_new(FALSE, 0);

        ops_notebook = gtk_notebook_new();
        (void) g_signal_connect(G_OBJECT(ops_notebook), "switch_page",
                                G_CALLBACK(notebook_switch_page), 0);

        /*
         * Create the notebook pages.
         */
        table = gtk_table_new(2, 2, FALSE);

        label = gtk_label_new("DX:");
        gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL,
                         GTK_FILL, 0, 0);

        label = gtk_label_new("DY:");
        gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, GTK_FILL,
                         GTK_FILL, 0, 0);

        ops_dx = gtk_widget_new(gtk_entry_get_type(),
                                "max_length", 6, NULL);
        (void) g_signal_connect(G_OBJECT(ops_dx), "changed",
                                G_CALLBACK(enable_apply),
                                GINT_TO_POINTER(0));
        gtk_table_attach(GTK_TABLE(table), ops_dx, 1, 2, 0, 1, GTK_FILL,
                         GTK_FILL, 5, 5);
        ops_dy = gtk_widget_new(gtk_entry_get_type(),
                                "max_length", 6, NULL);
        (void) g_signal_connect(G_OBJECT(ops_dx), "changed",
                                G_CALLBACK(enable_apply),
                                GINT_TO_POINTER(0));
        gtk_table_attach(GTK_TABLE(table), ops_dy, 1, 2, 1, 2, GTK_FILL,
                         GTK_FILL, 5, 5);

        label = gtk_label_new("Translate Glyphs");
        gtk_notebook_append_page(GTK_NOTEBOOK(ops_notebook), table, label);

        vbox1 = gtk_vbox_new(FALSE, 0);

        hbox = gtk_hbox_new(FALSE, 0);

        ops_rotate = gtk_radio_button_new_with_label(0, "Rotate");
        gtk_box_pack_start(GTK_BOX(hbox), ops_rotate, FALSE, FALSE, 2);
        rb = GTK_RADIO_BUTTON(ops_rotate);
        ops_shear = gtk_radio_button_new_with_label_from_widget(rb, "Shear");
        gtk_box_pack_start(GTK_BOX(hbox), ops_shear, FALSE, FALSE, 2);

        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);

        hbox = gtk_hbox_new(FALSE, 0);
        label = gtk_label_new("Degrees:");
        adj = (GtkAdjustment *) gtk_adjustment_new(0.0, -359.0, 359.0, 1.0,
                                                   10.0, 0.0);
        ops_degrees = gtk_widget_new(gtk_spin_button_get_type(),
                                     "max_length", 6,
                                     "adjustment", adj,
                                     "climb_rate", 1.0,
                                     "digits", 0,
                                     "value", 0.0,
                                     "numeric", TRUE,
                                     NULL);
        gtk_widget_set_size_request(ops_degrees, 60, -1);
        (void) g_signal_connect(G_OBJECT(ops_degrees), "changed",
                                G_CALLBACK(enable_apply),
                                GINT_TO_POINTER(1));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox), ops_degrees, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 3);

        label = gtk_label_new("Rotate/Shear Glyphs");
        gtk_notebook_append_page(GTK_NOTEBOOK(ops_notebook), vbox1, label);

        /*
         * Add the embolden check button.
         */
        button = gtk_check_button_new_with_label("Embolden");
        (void) g_signal_connect(G_OBJECT(button), "toggled",
                                G_CALLBACK(enable_apply),
                                GINT_TO_POINTER(2));
        label = gtk_label_new("Embolden Glyphs");
        gtk_notebook_append_page(GTK_NOTEBOOK(ops_notebook), button,
                                 label);

        /*
         * Add the notebook to the containing vbox.
         */
        gtk_box_pack_start(GTK_BOX(vbox), ops_notebook, FALSE, FALSE, 0);

        /*
         * Add the radio buttons for choosing between all the glyphs or
         * just the selected glyphs.
         */
        hbox = gtk_hbox_new(FALSE, 0);
        ops_all_glyphs = gtk_radio_button_new_with_label(0, "All Glyphs");
        rb = GTK_RADIO_BUTTON(ops_all_glyphs);
        ops_selected_glyphs =
            gtk_radio_button_new_with_label_from_widget(rb, "Selected Glyphs");
        /*
         * Default to the selected glyphs.
         */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ops_selected_glyphs),
                                     TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), ops_all_glyphs, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(hbox), ops_selected_glyphs,
                           FALSE, FALSE, 2);

        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

        /*
         * Add the vbox to the dialog.
         */
        gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(ops_dialog))),
                          vbox);

        ops_apply = gtk_dialog_add_button(GTK_DIALOG(ops_dialog),
                                          GTK_STOCK_APPLY,
                                          GTK_RESPONSE_APPLY);
        gtk_widget_set_sensitive(ops_apply, FALSE);

        button = gtk_dialog_add_button(GTK_DIALOG(ops_dialog),
                                       GTK_STOCK_CLOSE,
                                       GTK_RESPONSE_CLOSE);

        g_signal_connect(G_OBJECT(ops_dialog), "response",
                         G_CALLBACK(apply_operation), 0);

        gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(ops_dialog)));
    }

    if (fontgrid_has_selection(FONTGRID(ed->fgrid), 0))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ops_selected_glyphs),
                                   TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ops_all_glyphs), TRUE);

    /*
     * Set the title of the dialog.
     */
    if (ed->file == 0)
      sprintf(buffer1, "(unnamed%d): Glyph Operations", ed->id);
    else
      sprintf(buffer1, "%s: Glyph Operations", ed->file);

    gtk_window_set_title(GTK_WINDOW(ops_dialog), buffer1);
}

void
guiops_show_translate(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    ops_dialog_setup(ed);

    /*
     * Set the object data to be the editor.
     */
    g_object_set_data(G_OBJECT(ops_dialog), "editor", data);

    /*
     * Show the translate page.
     */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ops_notebook), 0);

    /*
     * Show the dialog.
     */
    guiutil_show_dialog_centered(ops_dialog, ed->shell);

    gtk_window_set_modal(GTK_WINDOW(ops_dialog), TRUE);
}

void
guiops_show_rotate(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    ops_dialog_setup(ed);

    /*
     * Set the object data to be the editor.
     */
    g_object_set_data(G_OBJECT(ops_dialog), "editor", data);

    /*
     * Show the rotate page.
     */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ops_rotate), TRUE);

    /*
     * Show the dialog.
     */
    guiutil_show_dialog_centered(ops_dialog, ed->shell);

    gtk_notebook_set_current_page(GTK_NOTEBOOK(ops_notebook), 1);

    gtk_window_set_modal(GTK_WINDOW(ops_dialog), TRUE);
}

void
guiops_show_shear(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    ops_dialog_setup(ed);

    /*
     * Set the object data to be the editor.
     */
    g_object_set_data(G_OBJECT(ops_dialog), "editor", data);

    /*
     * Show the shear page.
     */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ops_shear), TRUE);

    /*
     * Show the dialog.
     */
    guiutil_show_dialog_centered(ops_dialog, ed->shell);

    gtk_notebook_set_current_page(GTK_NOTEBOOK(ops_notebook), 1);

    gtk_window_set_modal(GTK_WINDOW(ops_dialog), TRUE);
}

void
guiops_show_embolden(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    ops_dialog_setup(ed);

    /*
     * Set the object data to be the editor.
     */
    g_object_set_data(G_OBJECT(ops_dialog), "editor", data);

    /*
     * Show the dialog.
     */
    guiutil_show_dialog_centered(ops_dialog, ed->shell);

    gtk_notebook_set_current_page(GTK_NOTEBOOK(ops_notebook), 2);

    gtk_window_set_modal(GTK_WINDOW(ops_dialog), TRUE);
}

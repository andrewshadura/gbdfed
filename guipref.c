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
static char svnid[] __attribute__ ((unused)) = "$Id: guipref.c 49 2007-04-12 14:46:40Z mleisher $";
#else
static char svnid[] = "$Id: guipref.c 49 2007-04-12 14:46:40Z mleisher $";
#endif
#endif

#include "gbdfed.h"
#include "grayswatch.h"

static GtkWidget *pref_dialog;
static GtkWidget *pref_unicode;
static GtkWidget *pref_adobe;
static GtkWidget *pref_cursor_font;
static GtkWidget *pref_apply;

static GtkWidget *pref_fsel_dialog;
static gboolean pref_fsel_unicode;

static GtkWidget *pref_color;
static GtkWidget *pref_color_dialog;
static GtkWidget *pref_color_win[16];

static gbdfed_options_t tmp_opts;

static void
pref_toggle(GtkWidget *w, gpointer data)
{
    gint which;
    gboolean val = FALSE;

    which = GPOINTER_TO_INT(data);
    if (which != 10)
      val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

    switch (which) {
      case 0: tmp_opts.backups = val; break;
      case 1: tmp_opts.font_opts.correct_metrics = val; break;
      case 2: tmp_opts.font_opts.pad_cells = val; break;
      case 3: tmp_opts.font_opts.keep_unencoded = val; break;
      case 4: tmp_opts.font_opts.keep_comments = val; break;
      case 5:
        if (val == TRUE)
          tmp_opts.font_opts.otf_flags &= ~FT_LOAD_NO_HINTING;
        else
          tmp_opts.font_opts.otf_flags |= FT_LOAD_NO_HINTING;
        break;
      case 6: tmp_opts.sbit = val; break;
      case 7:
        tmp_opts.show_cap_height = val;
        break;
      case 8:
        tmp_opts.show_x_height = val;
        break;
      case 9:
        /*
         * Toggle the Really Exit dialog.
         */
        tmp_opts.really_exit = val;
        break;
      case 10:
        tmp_opts.pixel_size = (unsigned int)
            gtk_combo_box_get_active(GTK_COMBO_BOX(w)) + 2;
        break;
    }

    /*
     * Enable the Apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

static void
pref_eol(GtkWidget *w, gpointer data)
{
    tmp_opts.font_opts.eol = gtk_combo_box_get_active(GTK_COMBO_BOX(w)) + 1;

    /*
     * Enable the Apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

static GtkWidget *
pref_make_general_page()
{
    GtkWidget *table, *button, *hbox, *tmp, *omenu, *frame, *vbox;

    vbox = gtk_vbox_new(FALSE, 10);

    /*
     * Create the load/save option selection.
     */
    frame = gtk_frame_new("Load/Save");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    table = gtk_table_new(2, 3, FALSE);

    button = gtk_check_button_new_with_label("Make Backups");
    if (tmp_opts.backups)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(0));
    gtk_table_attach(GTK_TABLE(table), button, 0, 1, 0, 1,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 5, 5);

    button = gtk_check_button_new_with_label("Correct Metrics");
    if (tmp_opts.font_opts.correct_metrics)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(1));
    gtk_table_attach(GTK_TABLE(table), button, 1, 2, 0, 1,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 5, 5);

    button = gtk_check_button_new_with_label("Pad Character Cells");
    if (tmp_opts.font_opts.pad_cells)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(2));
    gtk_table_attach(GTK_TABLE(table), button, 2, 3, 0, 1,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 5, 5);

    button = gtk_check_button_new_with_label("Keep Unencoded Glyphs");
    if (tmp_opts.font_opts.keep_unencoded)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(3));
    gtk_table_attach(GTK_TABLE(table), button, 0, 1, 1, 2,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 5, 5);

    button = gtk_check_button_new_with_label("Keep Comments");
    if (tmp_opts.font_opts.keep_comments)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(4));
    gtk_table_attach(GTK_TABLE(table), button, 1, 2, 1, 2,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 5, 5);

    hbox = gtk_hbox_new(FALSE, 0);
    tmp = gtk_label_new("EOL:");
    gtk_box_pack_start(GTK_BOX(hbox), tmp, FALSE, FALSE, 0);

    omenu = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(omenu), "Unix [LF]");
    gtk_combo_box_append_text(GTK_COMBO_BOX(omenu), "WIN/DOS [CRLF]");
    gtk_combo_box_append_text(GTK_COMBO_BOX(omenu), "MAC [CR]");
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), tmp_opts.font_opts.eol - 1);
    (void) g_signal_connect(G_OBJECT(omenu), "changed",
                            G_CALLBACK(pref_eol), 0);

    gtk_box_pack_start(GTK_BOX(hbox), omenu, TRUE, TRUE, 0);

    gtk_table_attach(GTK_TABLE(table), hbox, 2, 3, 1, 2,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 5, 5);

    gtk_container_add(GTK_CONTAINER(frame), table);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    frame = gtk_frame_new("OpenType");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    button = gtk_check_button_new_with_label("Hint Glyphs");
    if (!(tmp_opts.font_opts.otf_flags & FT_LOAD_NO_HINTING))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
#ifdef HAVE_FREETYPE
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(5));
#else
    /*
     * No Freetype support means no point in being able to toggle this
     * widget.
     */
    gtk_widget_set_sensitive(button, FALSE);
#endif
    gtk_container_add(GTK_CONTAINER(frame), button);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    frame = gtk_frame_new("SBIT");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    button = gtk_check_button_new_with_label("Generate SBIT Metrics File");
    if (tmp_opts.sbit)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(6));
    gtk_container_add(GTK_CONTAINER(frame), button);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    return vbox;
}

static void
pref_change_size(GtkWidget *w, gpointer data)
{
    gint v, which = GPOINTER_TO_INT(data);

    v = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));

    switch (which) {
      case 0: tmp_opts.font_opts.point_size = (int) v; break;
      case 1: tmp_opts.font_opts.resolution_x = (int) v; break;
      case 2: tmp_opts.font_opts.resolution_y = (int) v; break;
    }

    /*
     * Enable the apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

/*
 * Synchronize the vertical resolution with the horizontal resolution.
 */
static void
pref_sync_res(GtkWidget *w, GdkEventFocus *ev, gpointer data)
{
    gfloat v;
    GtkSpinButton *b;

    b = GTK_SPIN_BUTTON(data);
    v = (gfloat) gtk_spin_button_get_value(b);

    if (v != (gfloat) gtk_spin_button_get_value(GTK_SPIN_BUTTON(w))) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);

        /*
         * Enable the apply button.
         */
        gtk_widget_set_sensitive(pref_apply, TRUE);
    }
}

static void
pref_set_spacing(GtkWidget *w, gpointer data)
{
    tmp_opts.font_opts.font_spacing = GPOINTER_TO_INT(data);

    /*
     * Enable the apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

static void
pref_set_cursor_font(GtkWidget *w, gpointer data)
{
    tmp_opts.font_opts.cursor_font =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

    /*
     * Enable the apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

static void
pref_color_response(GtkDialog *d, gint response, gpointer data)
{
    gint i;

    if (response == GTK_RESPONSE_REJECT) {
        /*
         * Replace the colors with those found in the original
         * options.
         */
        memcpy(&tmp_opts.colors, &options.colors,
               sizeof(unsigned short) * 20);

        if (tmp_opts.font_opts.bits_per_pixel == 2) {
            for (i = 0; i < 4; i++)
              grayswatch_set_gray(GRAYSWATCH(pref_color_win[i]),
                                  tmp_opts.colors[i]);
        } else {
            for (i = 0; i < 16; i++)
              grayswatch_set_gray(GRAYSWATCH(pref_color_win[i]),
                                  tmp_opts.colors[i + 4]);
        }
    } else if (response == GTK_RESPONSE_CLOSE)
      gtk_widget_hide(GTK_WIDGET(data));
}

static void
pref_color_update_color(GtkWidget *w, gint color, gpointer data)
{
    gint which = GPOINTER_TO_INT(data);

    if (tmp_opts.font_opts.bits_per_pixel == 2)
      tmp_opts.colors[which] = color;
    else if (tmp_opts.font_opts.bits_per_pixel == 4)
      tmp_opts.colors[which + 4] = color;

    /*
     * Make sure the Apply button is enabled.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

static void
pref_select_colors(GtkWidget *w, gpointer data)
{
    GtkWidget *hbox;
    gint i;

    if (pref_color_dialog == 0) {

        pref_color_dialog = gtk_dialog_new();
        (void) g_signal_connect(G_OBJECT(pref_color_dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);
        (void) g_signal_connect(G_OBJECT(pref_color_dialog), "response",
                                G_CALLBACK(pref_color_response),
                                (gpointer) pref_color_dialog);
        gtk_window_set_resizable(GTK_WINDOW(pref_color_dialog), TRUE);

        hbox = gtk_hbox_new(FALSE, 0);

        for (i = 0; i < 16; i++) {
            pref_color_win[i] = grayswatch_new(tmp_opts.colors[i + 4]);
            g_signal_connect(G_OBJECT(pref_color_win[i]), "value-changed",
                             G_CALLBACK(pref_color_update_color),
                             GINT_TO_POINTER(i));
            gtk_widget_set_size_request(pref_color_win[i], 50, 75);
            gtk_box_pack_start(GTK_BOX(hbox), pref_color_win[i],
                               FALSE, FALSE, 0);
        }

        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(pref_color_dialog)->vbox),
                          hbox);

        /*
         * Add the buttons.
         */
        gtk_dialog_add_buttons(GTK_DIALOG(pref_color_dialog),
                               GTK_STOCK_REVERT_TO_SAVED,
                               GTK_RESPONSE_REJECT,
                               GTK_STOCK_CLOSE,
                               GTK_RESPONSE_CLOSE, NULL);

        gtk_dialog_set_default_response(GTK_DIALOG(pref_color_dialog),
                                        GTK_RESPONSE_CLOSE);

        gtk_widget_show_all(pref_color_dialog);
    }

    /*
     * If selecting colors for 2 bits-per-pixel, hide all but the first 4.
     * Set the first 4 colors depending on the bits-per-pixel value.
     */
    for (i = 0; i < 16; i++) {
        /*
         * We don't want setting the gray values to trigger the signal.
         * That causes the Apply button to be made sensitive.
         */
        grayswatch_block_signal(GRAYSWATCH(pref_color_win[i]), TRUE);
        if (tmp_opts.font_opts.bits_per_pixel == 2 && i >= 4)
          gtk_widget_hide(pref_color_win[i]);
        else {
            if (i < 4) {
                if (tmp_opts.font_opts.bits_per_pixel == 2)
                  grayswatch_set_gray(GRAYSWATCH(pref_color_win[i]),
                                      tmp_opts.colors[i]);
                else
                  grayswatch_set_gray(GRAYSWATCH(pref_color_win[i]),
                                      tmp_opts.colors[i + 4]);
            } else
              gtk_widget_show(pref_color_win[i]);
        }
        grayswatch_block_signal(GRAYSWATCH(pref_color_win[i]), FALSE);
    }

    /*
     * Center the dialog and show it.
     */
    guiutil_show_dialog_centered(pref_color_dialog, pref_dialog);
}

static void
pref_set_bpp(GtkWidget *w, gpointer data)
{
    gboolean on;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)) == FALSE)
      return;

    tmp_opts.font_opts.bits_per_pixel = GPOINTER_TO_INT(data);
    on = (tmp_opts.font_opts.bits_per_pixel == 1 ||
          tmp_opts.font_opts.bits_per_pixel == 8) ? FALSE : TRUE;

    if (pref_color_dialog != 0 && GTK_WIDGET_VISIBLE(pref_color_dialog)) {
        if (tmp_opts.font_opts.bits_per_pixel == 1 ||
            tmp_opts.font_opts.bits_per_pixel == 8)
          gtk_widget_hide(pref_color_dialog);
        else
          pref_select_colors(w, data);
    }

    gtk_widget_set_sensitive(pref_color, on);

    /*
     * Enable the apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

static GtkWidget *
pref_make_newfont_page()
{
    GtkWidget *label, *table, *button, *hbox, *vbox, *frame, *tmp;
    GtkAdjustment *adj;

    vbox = gtk_vbox_new(FALSE, 10);

    /*
     * Create the font size selection.
     */
    frame = gtk_frame_new("Font Size");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    table = gtk_table_new(3, 2, FALSE);

    label = gtk_label_new("Point Size:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
                     GTK_FILL, GTK_FILL, 5, 0);

    label = gtk_label_new("Horizontal Resolution:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
                     GTK_FILL, GTK_FILL, 5, 5);

    label = gtk_label_new("Vertical Resolution:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
                     GTK_FILL, GTK_FILL, 5, 5);

    /*
     * Make the spinboxes for the point size and resolutions.
     */
    adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 4.0, 256.0, 1.0, 2.0, 0.0);
    button = gtk_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(button), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(button),
                              (gfloat) tmp_opts.font_opts.point_size);
    gtk_widget_set_size_request(button, 100, -1);
    (void) g_signal_connect(G_OBJECT(button), "changed",
                            G_CALLBACK(pref_change_size),
                            GINT_TO_POINTER(0));
    gtk_table_attach(GTK_TABLE(table), button, 1, 2, 0, 1,
                     GTK_FILL, GTK_FILL, 5, 5);

    adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 20.0, 1200.0,
                                               1.0, 10.0, 0.0);
    tmp = button = gtk_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(button), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(button),
                              (gfloat) tmp_opts.font_opts.resolution_x);
    gtk_widget_set_size_request(button, 100, -1);
    (void) g_signal_connect(G_OBJECT(button), "changed",
                            G_CALLBACK(pref_change_size),
                            GINT_TO_POINTER(1));
    gtk_table_attach(GTK_TABLE(table), button, 1, 2, 1, 2,
                     GTK_FILL, GTK_FILL, 5, 5);

    adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 20.0, 1200.0,
                                               1.0, 10.0, 0.0);
    button = gtk_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(button), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(button),
                              (gfloat) tmp_opts.font_opts.resolution_y);
    gtk_widget_set_size_request(button, 100, -1);
    (void) g_signal_connect(G_OBJECT(button), "changed",
                            G_CALLBACK(pref_change_size),
                            GINT_TO_POINTER(2));
    (void) g_signal_connect(G_OBJECT(button), "focus-in-event",
                            G_CALLBACK(pref_sync_res), (gpointer) tmp);
    gtk_table_attach(GTK_TABLE(table), button, 1, 2, 2, 3,
                     GTK_FILL, GTK_FILL, 5, 5);

    gtk_container_add(GTK_CONTAINER(frame), table);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    /*
     * Create the spacing selection.
     */
    frame = gtk_frame_new("Spacing");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    hbox = gtk_hbox_new(FALSE, 0);

    button = gtk_radio_button_new_with_label(0, "Proportional");
    if (tmp_opts.font_opts.font_spacing == BDF_PROPORTIONAL)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_set_spacing),
                            GINT_TO_POINTER(BDF_PROPORTIONAL));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    button =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),
                                                    "Monowidth");
    if (tmp_opts.font_opts.font_spacing == BDF_MONOWIDTH)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_set_spacing),
                            GINT_TO_POINTER(BDF_MONOWIDTH));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    button =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),
                                                    "Character Cell");
    if (tmp_opts.font_opts.font_spacing == BDF_CHARCELL)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_set_spacing),
                            GINT_TO_POINTER(BDF_CHARCELL));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(frame), hbox);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    /*
     * Create the bits-per-pixel selection.
     */
    frame = gtk_frame_new("Bits Per Pixel");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    hbox = gtk_hbox_new(FALSE, 0);

    button = gtk_radio_button_new_with_label(0, "1 bpp");
    if (tmp_opts.font_opts.bits_per_pixel == 1)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_set_bpp),
                            GINT_TO_POINTER(1));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    button =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),
                                                    "2 bpp");
    if (tmp_opts.font_opts.bits_per_pixel == 2)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_set_bpp),
                            GINT_TO_POINTER(2));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    button =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),
                                                    "4 bpp");
    if (tmp_opts.font_opts.bits_per_pixel == 4)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_set_bpp),
                            GINT_TO_POINTER(4));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    button =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),
                                                    "8 bpp");
    if (tmp_opts.font_opts.bits_per_pixel == 8)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_set_bpp),
                            GINT_TO_POINTER(8));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    pref_color = gtk_button_new_with_label("Select Colors");
    (void) g_signal_connect(G_OBJECT(pref_color), "clicked",
                            G_CALLBACK(pref_select_colors), 0);
    if (tmp_opts.font_opts.bits_per_pixel == 1)
      gtk_widget_set_sensitive(pref_color, FALSE);

    gtk_box_pack_start(GTK_BOX(hbox), pref_color, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(frame), hbox);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    frame = gtk_frame_new("Cursor Fonts");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    pref_cursor_font = gtk_check_button_new_with_label("Cursor Font");
    if (tmp_opts.font_opts.cursor_font)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_cursor_font),
                                   TRUE);
    (void) g_signal_connect(G_OBJECT(pref_cursor_font), "toggled",
                            G_CALLBACK(pref_set_cursor_font), 0);
    gtk_container_add(GTK_CONTAINER(frame), pref_cursor_font);
    gtk_widget_set_sensitive(pref_cursor_font, FALSE);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    return vbox;
}

static void
pref_fgrid_mode(GtkWidget *w, gpointer data)
{
    tmp_opts.overwrite_mode =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

    /*
     * Enable the Apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);
}

static void
pref_set_filename(GtkWidget *w, gpointer data)
{
    gchar *fname;

    fname = (gchar *) gtk_file_selection_get_filename(GTK_FILE_SELECTION(w));

    if (pref_fsel_unicode)
      gtk_entry_set_text(GTK_ENTRY(pref_unicode), fname);
    else
      gtk_entry_set_text(GTK_ENTRY(pref_adobe), fname);

    /*
     * Enable the Apply button.
     */
    gtk_widget_set_sensitive(pref_apply, TRUE);

    /*
     * Hide the dialog.
     */
    gtk_widget_hide(w);
}

static void
pref_show_fsel_dialog(GtkWidget *w, gpointer data)
{
    GtkFileSelection *fs;

    pref_fsel_unicode = GPOINTER_TO_INT(data);

    if (pref_fsel_dialog == 0) {
        pref_fsel_dialog = gtk_file_selection_new(0);
        (void) g_signal_connect(G_OBJECT(pref_fsel_dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        fs = GTK_FILE_SELECTION(pref_fsel_dialog);
        gtk_file_selection_hide_fileop_buttons(fs);

        (void) g_signal_connect_object(G_OBJECT(fs->ok_button), "clicked",
                                       G_CALLBACK(pref_set_filename),
                                       (gpointer) fs,
                                       G_CONNECT_SWAPPED);
        (void) g_signal_connect_object(G_OBJECT(fs->cancel_button),
                                       "clicked",
                                       G_CALLBACK(gtk_widget_hide),
                                       (gpointer) fs,
                                       G_CONNECT_SWAPPED);
        gtk_widget_show_all(GTK_DIALOG(pref_fsel_dialog)->vbox);
        gtk_widget_show_all(GTK_DIALOG(pref_fsel_dialog)->action_area);
    }

    /*
     * Set the title of the dialog.
     */
    if (pref_fsel_unicode)
      strcpy(buffer1, "Unicode Character Database Selection");
    else
      strcpy(buffer1, "Adobe Glyph Name File Selection");

    gtk_window_set_title(GTK_WINDOW(pref_fsel_dialog), buffer1);

    guiutil_show_dialog_centered(pref_fsel_dialog, pref_dialog);

    gtk_window_set_modal(GTK_WINDOW(pref_fsel_dialog), TRUE);
}

static GtkWidget *
pref_make_edit_page()
{
    gint i;
    GtkWidget *vbox, *hbox, *frame, *button, *label, *omenu;
    GtkWidget *tmp, *table;

    vbox = gtk_vbox_new(FALSE, 10);

    frame = gtk_frame_new("Font Grid Selection Paste");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    hbox = gtk_hbox_new(FALSE, 5);

    button = gtk_radio_button_new_with_label(0, "Overwrites");
    if (tmp_opts.overwrite_mode)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_fgrid_mode), 0);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

    button =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button),
                                                    "Inserts");
    if (tmp_opts.overwrite_mode)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(frame), hbox);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    frame = gtk_frame_new("Glyph Editors");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    hbox = gtk_hbox_new(FALSE, 5);

    button = gtk_check_button_new_with_label("Show Cap Height");
    if (tmp_opts.show_cap_height)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(7));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    button = gtk_check_button_new_with_label("Show X Height");
    if (tmp_opts.show_cap_height)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GINT_TO_POINTER(8));
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

    label = gtk_label_new("Pixel Size:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    omenu = gtk_combo_box_new_text();
    for (i = 2; i < 21; i++) {
        sprintf(buffer1, "%dx%d", i, i);
        gtk_combo_box_append_text(GTK_COMBO_BOX(omenu), buffer1);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), tmp_opts.pixel_size - 2);
    g_signal_connect(G_OBJECT(omenu), "changed",
                     G_CALLBACK(pref_toggle), GINT_TO_POINTER(10));

    gtk_box_pack_start(GTK_BOX(hbox), omenu, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(frame), hbox);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    frame = gtk_frame_new("Glyph Name Lists");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    table = gtk_table_new(2, 3, FALSE);

    label = gtk_label_new("Unicode Character Database:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
                     GTK_FILL, GTK_FILL, 5, 5);

    label = gtk_label_new("Adobe Glyph List:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
                     GTK_FILL, GTK_FILL, 5, 5);

    /*
     * Add the entries.
     */
    tmp = pref_unicode = gtk_entry_new();
    gtk_widget_set_size_request(tmp, 250, -1);
    if (tmp_opts.unicode_name_file)
      gtk_entry_set_text(GTK_ENTRY(pref_unicode), tmp_opts.unicode_name_file);
    gtk_table_attach(GTK_TABLE(table), tmp, 1, 2, 0, 1,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 5);

    tmp = pref_adobe = gtk_entry_new();
    gtk_widget_set_size_request(tmp, 250, -1);
    if (tmp_opts.adobe_name_file)
      gtk_entry_set_text(GTK_ENTRY(pref_adobe), tmp_opts.adobe_name_file);
    gtk_table_attach(GTK_TABLE(table), tmp, 1, 2, 1, 2,
                     GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 5);

    /*
     * Add the browse buttons.
     */
    button = gtk_button_new_with_label("Browse");
    (void) g_signal_connect(G_OBJECT(button), "clicked",
                            G_CALLBACK(pref_show_fsel_dialog),
                            GINT_TO_POINTER(TRUE));
    gtk_table_attach(GTK_TABLE(table), button, 2, 3, 0, 1,
                     GTK_FILL, 0, 5, 0);
    button = gtk_button_new_with_label("Browse");
    (void) g_signal_connect(G_OBJECT(button), "clicked",
                            G_CALLBACK(pref_show_fsel_dialog),
                            GINT_TO_POINTER(FALSE));
    gtk_table_attach(GTK_TABLE(table), button, 2, 3, 1, 2,
                     GTK_FILL, 0, 5, 0);

    gtk_container_add(GTK_CONTAINER(frame), table);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    return vbox;
}

static GtkWidget *
pref_make_other_page()
{
    GtkWidget *frame, *button, *vbox;

    frame = gtk_frame_new("Dialogs");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    vbox = gtk_vbox_new(FALSE, 0);

    button = gtk_check_button_new_with_label("Show \"Really Exit\" Dialog");
    if (tmp_opts.really_exit)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    (void) g_signal_connect(G_OBJECT(button), "toggled",
                            G_CALLBACK(pref_toggle),
                            GUINT_TO_POINTER(9));
    gtk_container_add(GTK_CONTAINER(frame), button);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    return vbox;
}

static void
pref_apply_changes(void)
{
    gchar *fname;

    /*
     * Take care of setting the file names for the glyph name lists.
     */
    fname = (gchar *) gtk_entry_get_text(GTK_ENTRY(pref_unicode));
    if (fname != 0 && fname[0] != 0)
      tmp_opts.unicode_name_file = g_strdup(fname);

    fname = (gchar *) gtk_entry_get_text(GTK_ENTRY(pref_adobe));
    if (fname != 0 && fname[0] != 0)
      tmp_opts.adobe_name_file = g_strdup(fname);

    /*
     * If the name files are different, delete the old name files first.
     */
    if (options.unicode_name_file != 0 &&
        options.unicode_name_file != tmp_opts.unicode_name_file)
      g_free(options.unicode_name_file);
    if (options.adobe_name_file != 0 &&
        options.adobe_name_file != tmp_opts.adobe_name_file)
      g_free(options.adobe_name_file);

    /*
     * Copy the updated options over.
     */
    (void) memcpy((char *) &options, (char *) &tmp_opts,
                  sizeof(gbdfed_options_t));

    /*
     * Set the glyph edit options.
     */
    guigedit_show_cap_height(tmp_opts.show_cap_height);
    guigedit_show_x_height(tmp_opts.show_x_height);
    guigedit_set_pixel_size(tmp_opts.pixel_size);

    /*
     * Disable the apply button.
     */
    gtk_widget_set_sensitive(pref_apply, FALSE);
}

static void
pref_save(void)
{
    FILE *out;
    gchar *home;
    gint i;

    /*
     * If any changes were made, do an update before saving.
     */
    if (GTK_WIDGET_SENSITIVE(pref_apply))
      pref_apply_changes();

    if ((home = getenv("HOME")) == 0) {
        guiutil_error_message(editors[0].shell,
                              "Save Preferences: Unable to locate home directory.");
        return;
    }

    sprintf(buffer1, "%s/.gbdfedrc", home);
    if ((out = fopen(buffer1, "w")) == 0) {
        sprintf(buffer2, "Save Preferences: Unable to write to %s.", buffer1);
        guiutil_error_message(editors[0].shell, buffer2);
        return;
    }

    /*
     * First, write the gbdfed options.
     */
    fprintf(out, "#########################\n");
    fprintf(out, "#\n# gbdfed options.\n#\n");
    fprintf(out, "#########################\n\n");

    if (options.no_blanks)
      fprintf(out, "skip_blank_pages true\n\n");
    else
      fprintf(out, "skip_blank_pages false\n\n");

    if (options.really_exit)
      fprintf(out, "really_exit true\n\n");
    else
      fprintf(out, "really_exit false\n\n");
    if (options.overwrite_mode)
      fprintf(out, "grid_overwrite_mode true\n\n");
    else
      fprintf(out, "grid_overwrite_mode false\n\n");

    if (options.accelerator != 0)
      fprintf(out, "close_accelerator %s\n\n",
              options.accelerator);
    if (options.accelerator_text != 0)
      fprintf(out, "close_accelerator_text %s\n\n",
              options.accelerator_text);

    if (options.unicode_name_file != 0)
      fprintf(out, "name_file %s\n\n", options.unicode_name_file);
    
    if (options.adobe_name_file != 0)
      fprintf(out, "adobe_name_file %s\n\n",
              options.adobe_name_file);

    fprintf(out, "pixel_size %d\n\n", options.pixel_size);

    if (options.show_cap_height)
      fprintf(out, "show_cap_height true\n\n");
    else
      fprintf(out, "show_cap_height false\n\n");

    if (options.show_x_height)
      fprintf(out, "show_x_height true\n\n");
    else
      fprintf(out, "show_x_height false\n\n");

    if (options.sbit)
      fprintf(out, "generate_sbit_metrics true\n\n");
    else
      fprintf(out, "generate_sbit_metrics false\n\n");

    /*
     * Save the grayscales.
     */
    fprintf(out, "#\n# Grayscale values. Must be between 0 and 255.\n#\n");
    fprintf(out, "2bpp_grays ");
    for (i = 0; i < 4; i++) {
        fprintf(out, "%d", options.colors[i]);
        if (i + 1 < 4)
          putc(' ', out);
    }
    fprintf(out, "\n4bpp_grays ");
    for (i = 4; i < 20; i++) {
        fprintf(out, "%d", options.colors[i]);
        if (i + 1 < 20)
          putc(' ', out);
    }
    fprintf(out, "\n\n");

#if 0
    /*
     * Save the colors.
     */
    fprintf(out, "#\n# Color values for 2 bits per pixel.\n#\n");
    for (i = 0; i < 4; i++) {
        /*
         * Do this to avoid writing negative values.
         */
        c = options.colors[i];
        fprintf(out, "color%d %d\n", i, c);
    }

    fprintf(out, "\n#\n# Color values for 4 bits per pixel.\n#\n");
    for (i = 4; i < 20; i++) {
        /*
         * Do this to avoid writing negative values.
         */
        c = options.colors[i];
        fprintf(out, "color%d %d\n", i, c);
    }
    putc('\n', out);
#endif

    /*
     * The save the BDF specific options.
     */
    fprintf(out, "#########################\n");
    fprintf(out, "#\n# BDF font options.\n#\n");
    fprintf(out, "#########################\n\n");
    bdf_save_options(out, &options.font_opts);
    fclose(out);

    /*
     * Close the preferences dialog.
     */
    gtk_widget_hide(pref_dialog);
}

static void
pref_response(GtkDialog *d, gint response, gpointer data)
{
    if (response == GTK_RESPONSE_APPLY)
      pref_apply_changes();
    else if (response == GTK_RESPONSE_OK)
      pref_save();
    else {
        /*
         * Make sure the color chooser dialog is hidden if it
         * happens to be up.
         */
        if (pref_color_dialog != 0 && GTK_WIDGET_VISIBLE(pref_color_dialog))
          gtk_widget_hide(pref_color_dialog);

        gtk_widget_hide(GTK_WIDGET(d));
    }
}

void
guiedit_show_preferences(GtkWidget *w, gpointer data)
{
    GtkWidget *dvbox, *nb, *button, *table, *label;

    if (pref_dialog == 0) {
        /*
         * Initialize the temporary options.
         */
        (void) memcpy((char *) &tmp_opts, (char *) &options,
                      sizeof(gbdfed_options_t));

        pref_dialog = gtk_dialog_new();
        (void) g_signal_connect(G_OBJECT(pref_dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        sprintf(buffer1, "%s Preferences", g_get_prgname());
        gtk_window_set_title(GTK_WINDOW(pref_dialog), buffer1);

        dvbox = GTK_DIALOG(pref_dialog)->vbox;

        /*
         * Create the notebook that will contain the Preference tabs.
         */
        nb = gtk_notebook_new();

        /*
         * Create the General Options page.
         */
        label = gtk_label_new("General Options");
        table = pref_make_general_page();
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), table, label);

        /*
         * Create the New Font Options page.
         */
        label = gtk_label_new("New Font Options");
        table = pref_make_newfont_page();
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), table, label);

        /*
         * Create the Editing Options page.
         */
        label = gtk_label_new("Editing Options");
        table = pref_make_edit_page();
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), table, label);

        /*
         * Create the Other Options page.
         */
        label = gtk_label_new("Other Options");
        table = pref_make_other_page();
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), table, label);

        /*
         * Finally, add the notebook to the dialog's vbox.
         */
        gtk_container_add(GTK_CONTAINER(dvbox), nb);

        /*
         * Add the buttons at the bottom.
         */
        pref_apply = gtk_dialog_add_button(GTK_DIALOG(pref_dialog),
                                           GTK_STOCK_APPLY,
                                           GTK_RESPONSE_APPLY);
        gtk_widget_set_sensitive(pref_apply, FALSE);

        button = gtk_dialog_add_button(GTK_DIALOG(pref_dialog),
                                       GTK_STOCK_SAVE,
                                       GTK_RESPONSE_OK);

        button = gtk_dialog_add_button(GTK_DIALOG(pref_dialog),
                                       GTK_STOCK_CLOSE,
                                       GTK_RESPONSE_CLOSE);
        gtk_dialog_set_default_response(GTK_DIALOG(pref_dialog),
                                        GTK_RESPONSE_CLOSE);

        g_signal_connect(G_OBJECT(pref_dialog), "response",
                         G_CALLBACK(pref_response), 0);

        gtk_widget_show_all(dvbox);
    }

    guiutil_show_dialog_centered(pref_dialog, editors[0].shell);
}

void
guiedit_preference_cleanup()
{
    /*
     * Does nothing at the moment.
     */
}

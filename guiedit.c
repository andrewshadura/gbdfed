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
#include "labcon.h"

/*
 * The global glyphtest widget.
 */
GtkWidget *glyphtest = 0;
static GtkWidget *gtest_dialog = 0;
static GtkWidget *gtest_erase = 0;

static void
guiedit_glyphtest_baseline(GtkWidget *w, gpointer data)
{
    GtkToggleButton *tb;

    tb = GTK_TOGGLE_BUTTON(w);
    glyphtest_show_baseline(GLYPHTEST(glyphtest),
                            gtk_toggle_button_get_active(tb));
}

static void
guiedit_glyphtest_direction(GtkWidget *w, gpointer data)
{
    GtkToggleButton *tb;
    gint dir;

    tb = GTK_TOGGLE_BUTTON(w);
    dir = gtk_toggle_button_get_active(tb) ?
        GLYPHTEST_RIGHT_TO_LEFT : GLYPHTEST_LEFT_TO_RIGHT;
    glyphtest_change_direction(GLYPHTEST(glyphtest), dir);
}

static void
guiedit_glyphtest_erase(GtkWidget *w, gpointer data)
{
    glyphtest_erase(GLYPHTEST(glyphtest));

    /*
     * Disable the clear button until a glyph is added.
     */
    gtk_widget_set_sensitive(gtest_erase, FALSE);
}

static void
guiedit_glyphtest_enable_erase(GtkWidget *w, gpointer data)
{
    /*
     * Enable the clear button if a glyph is added.
     */
    gtk_widget_set_sensitive(gtest_erase, TRUE);
}

void
guiedit_show_glyphtest(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    GtkWidget *hbox, *vbox, *frame, *cb;

    if (gtest_dialog == 0) {
        gtest_dialog = gtk_dialog_new();
        (void) g_signal_connect(G_OBJECT(gtest_dialog),
                                "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        vbox = gtk_vbox_new(FALSE, 0);

        frame = gtk_frame_new("Test Glyphs");
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

        glyphtest = glyphtest_new();
        (void) g_signal_connect(G_OBJECT(glyphtest), "add_glyph",
                                G_CALLBACK(guiedit_glyphtest_enable_erase),
                                0);
        gtk_container_add(GTK_CONTAINER(frame), glyphtest);

        gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, FALSE, 0);

        hbox = gtk_hbox_new(FALSE, 0);
        cb = gtk_check_button_new_with_label("Show Baseline");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb), TRUE);
        (void) g_signal_connect(G_OBJECT(cb), "toggled",
                                G_CALLBACK(guiedit_glyphtest_baseline),
                                0);
        gtk_box_pack_start(GTK_BOX(hbox), cb, FALSE, FALSE, 2);
        cb = gtk_check_button_new_with_label("Draw Right To Left");
        (void) g_signal_connect(G_OBJECT(cb), "toggled",
                                G_CALLBACK(guiedit_glyphtest_direction),
                                0);
        gtk_box_pack_start(GTK_BOX(hbox), cb, FALSE, FALSE, 2);

        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(gtest_dialog)->vbox),
                          vbox);

        /*
         * Add the buttons.
         */
        hbox = GTK_DIALOG(gtest_dialog)->action_area;

        gtest_erase = gtk_button_new_with_label("Erase");
        gtk_widget_set_sensitive(gtest_erase, FALSE);
        (void) g_signal_connect(G_OBJECT(gtest_erase), "clicked",
                                G_CALLBACK(guiedit_glyphtest_erase), 0);
        gtk_container_add(GTK_CONTAINER(hbox), gtest_erase);

        cb = gtk_button_new_with_label("Close");
        (void) g_signal_connect_object(G_OBJECT(cb), "clicked",
                                       G_CALLBACK(gtk_widget_hide),
                                       (gpointer) gtest_dialog,
                                       G_CONNECT_SWAPPED);
        gtk_container_add(GTK_CONTAINER(hbox), cb);
        gtk_widget_show_all(GTK_DIALOG(gtest_dialog)->vbox);
        gtk_widget_show_all(GTK_DIALOG(gtest_dialog)->action_area);
    }

    guiutil_show_dialog_centered(gtest_dialog, ed->shell);
}

void
guiedit_set_unicode_glyph_names(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    FILE *in;

    if (options.unicode_name_file == 0) {
        guiutil_error_message(ed->shell,
                              "Unicode Glyph Names: No file provided.");
        return;
    }

    if ((in = fopen(options.unicode_name_file, "r")) == 0) {
        sprintf(buffer1, "Unicode Glyph Names: Unable to open %s.",
                options.unicode_name_file);
        guiutil_error_message(ed->shell, buffer1);
    }

    fontgrid_set_unicode_glyph_names(FONTGRID(ed->fgrid), in);

    fclose(in);
}

void
guiedit_set_adobe_glyph_names(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    FILE *in;

    if (options.adobe_name_file == 0) {
        guiutil_error_message(ed->shell,
                              "Adobe Glyph Names: No file provided.");
        return;
    }

    if ((in = fopen(options.adobe_name_file, "r")) == 0) {
        sprintf(buffer1, "Adobe Glyph Names: Unable to open %s.",
                options.adobe_name_file);
        guiutil_error_message(ed->shell, buffer1);
    }

    fontgrid_set_adobe_glyph_names(FONTGRID(ed->fgrid), in);

    fclose(in);
}

void
guiedit_set_uni_glyph_names(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_set_code_glyph_names(FONTGRID(ed->fgrid), 'u');
}

void
guiedit_set_zerox_glyph_names(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_set_code_glyph_names(FONTGRID(ed->fgrid), 'x');
}

void
guiedit_set_uplus_glyph_names(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_set_code_glyph_names(FONTGRID(ed->fgrid), '+');
}

void
guiedit_set_bslashu_glyph_names(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_set_code_glyph_names(FONTGRID(ed->fgrid), '\\');
}

static void
guiedit_select_property(GtkTreeSelection *selection, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    gchar *prop_name;
    bdf_font_t *font;
    bdf_property_t *prop;
    GtkToggleButton *tb;
    gboolean from_font = TRUE;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GValue val;

    /*
     * This is called after the list is cleared as well, so return if there is
     * no selection.
     */
    if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
      return;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    (void) memset((char *) &val, 0, sizeof(GValue));

    gtk_tree_model_get_value(model, &iter, 0, &val);
    prop_name = (gchar *) g_value_get_string(&val);

    if ((prop = bdf_get_font_property(font, prop_name)) == 0) {
        from_font = FALSE;
        prop = bdf_get_property(prop_name);
    }

    gtk_entry_set_text(GTK_ENTRY(ed->finfo_prop_name), prop_name);

    g_value_unset(&val);

    /*
     * Always clear the property value field in case of ATOM properties with
     * no actual value.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->finfo_prop_value), "");

    if (from_font) {
        switch (prop->format) {
          case BDF_ATOM:
            if (prop->value.atom != 0)
              gtk_entry_set_text(GTK_ENTRY(ed->finfo_prop_value),
                                 prop->value.atom);
            break;
          case BDF_INTEGER:
            sprintf(buffer1, "%d", prop->value.int32);
            gtk_entry_set_text(GTK_ENTRY(ed->finfo_prop_value), buffer1);
            break;
          case BDF_CARDINAL:
            sprintf(buffer1, "%d", prop->value.card32);
            gtk_entry_set_text(GTK_ENTRY(ed->finfo_prop_value), buffer1);
            break;
        }
    }

    tb = GTK_TOGGLE_BUTTON(ed->finfo_prop_format[prop->format]);
    gtk_toggle_button_set_active(tb, TRUE);

    /*
     * Change the sensitivity of the Delete button depending on whether the
     * property was defined in the font or not.  If the property came from the
     * font and is one of FONT_ASCENT or FONT_DESCENT, do not allow them to be
     * deleted.
     */
    if (from_font && (strncmp(prop_name, "FONT_ASCENT", 11) == 0 ||
                      strncmp(prop_name, "FONT_DESCENT", 12) == 0))
      from_font = FALSE;
    gtk_widget_set_sensitive(ed->finfo_delete_prop, from_font);

    /*
     * Always change the sensitivity of the Apply button to False when a
     * property is selected.
     */
    gtk_widget_set_sensitive(ed->finfo_apply_prop, FALSE);
}

/*
 * Called whenever a new font is loaded into this editor.
 */
void
guiedit_update_font_info(gbdfed_editor_t *ed)
{
    GtkToggleButton *tb;
    GtkTextBuffer *text;
    bdf_font_t *font;
    guint32 i, nprops;
    bdf_property_t *props;
    GtkListStore *store;
    GtkTreeIter iter;

    /*
     * Simply return if the editor's info dialog doesn't exist yet.
     */
    if (ed->finfo_dialog == 0)
      return;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    /*
     * Set the dialog title.
     */
    if (ed->file == 0)
      sprintf(buffer1, "(unnamed%d) Info Edit", ed->id);
    else
      sprintf(buffer1, "%s Info Edit", ed->file);
    gtk_window_set_title(GTK_WINDOW(ed->finfo_dialog), buffer1);

    /*
     * Update the font properties.
     */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ed->finfo_font_props)));
    gtk_list_store_clear(store);

    if ((nprops = bdf_font_property_list(font, &props)) > 0) {
        for (i = 0; i < nprops; i++) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, props[i].name, -1);
        }
        free((char *) props);
    }

    /*
     * Update the defined properties.
     */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ed->finfo_all_props)));
    gtk_list_store_clear(store);

    if ((nprops = bdf_property_list(&props)) > 0) {
        for (i = 0; i < nprops; i++) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, props[i].name, -1);
        }
        free((char *) props);
    }

    /*
     * Clear the property name and value fields.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->finfo_prop_name), "");
    gtk_entry_set_text(GTK_ENTRY(ed->finfo_prop_value), "");

    /*
     * Turn off the property apply button until something gets edited.
     */
    gtk_widget_set_sensitive(ed->finfo_apply_prop, FALSE);

    /*
     * Update the font comments.
     */
    text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ed->finfo_comments));
    if (font && font->comments_len > 0)
      gtk_text_buffer_set_text(text, font->comments, font->comments_len);
    else
      gtk_text_buffer_set_text(text, "", 0);

    gtk_widget_set_sensitive(ed->finfo_apply_comments, FALSE);

    /*
     * Update the font spacing.
     */
    if (font)
      tb = GTK_TOGGLE_BUTTON(ed->finfo_spacing[font->spacing >> 4]);
    else
      tb = GTK_TOGGLE_BUTTON(ed->finfo_spacing[options.font_opts.font_spacing >> 4]);
    gtk_toggle_button_set_active(tb, TRUE);

    /*
     * Update the device width value.
     */
    gtk_widget_set_sensitive(ed->finfo_dwidth, TRUE);
    if (font && font->spacing != BDF_PROPORTIONAL)
      sprintf(buffer1, "%hd", font->monowidth);
    else
      (void) strcpy(buffer1, "0");
    gtk_entry_set_text(GTK_ENTRY(ed->finfo_dwidth), buffer1);

    if (font)
      sprintf(buffer1, "%d", font->font_ascent);
    else
      sprintf(buffer1, "0");
    gtk_entry_set_text(GTK_ENTRY(ed->finfo_ascent), buffer1);

    if (font)
      sprintf(buffer1, "%d", font->font_descent);
    else
      sprintf(buffer1, "0");
    gtk_entry_set_text(GTK_ENTRY(ed->finfo_descent), buffer1);

    if ((!font && options.font_opts.font_spacing == BDF_PROPORTIONAL) ||
        font->spacing == BDF_PROPORTIONAL)
      gtk_widget_set_sensitive(ed->finfo_dwidth, FALSE);

    /*
     * Finally, update the remaining information.
     */
    if (font)
      sprintf(buffer1, "%d", font->glyphs_used);
    else
      sprintf(buffer1, "0");
    gtk_label_set_text(GTK_LABEL(ed->finfo_enc_count), buffer1);

    if (font)
      sprintf(buffer1, "%d", font->unencoded_used);
    else
      sprintf(buffer1, "0");
    gtk_label_set_text(GTK_LABEL(ed->finfo_unenc_count), buffer1);

    if (font) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ed->finfo_hres),
                                  (gfloat) font->resolution_x);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ed->finfo_vres),
                                  (gfloat) font->resolution_y);
    }

    if (font) {
        switch (fontgrid_get_code_base(FONTGRID(ed->fgrid))) {
          case 8: sprintf(buffer1, "%o", font->default_glyph); break;
          case 10: sprintf(buffer1, "%d", font->default_glyph); break;
          case 16: sprintf(buffer1, "%04x", font->default_glyph); break;
        }
        gtk_entry_set_text(GTK_ENTRY(ed->finfo_default_char), buffer1);
    }

    if (font)
      sprintf(buffer1, "%hd", font->bpp);
    else
      sprintf(buffer1, "%d", options.font_opts.bits_per_pixel);
    gtk_label_set_text(GTK_LABEL(ed->finfo_bpp), buffer1);

    gtk_widget_set_sensitive(ed->finfo_apply_info, FALSE);
}

void
guiedit_update_code_base(gbdfed_editor_t *ed)
{
    bdf_font_t *font;

    if (ed->finfo_dialog == 0)
      return;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    switch (fontgrid_get_code_base(FONTGRID(ed->fgrid))) {
      case 8: sprintf(buffer1, "%o", font->default_glyph); break;
      case 10: sprintf(buffer1, "%d", font->default_glyph); break;
      case 16: sprintf(buffer1, "%04x", font->default_glyph); break;
    }
    gtk_entry_set_text(GTK_ENTRY(ed->finfo_default_char), buffer1);
}

void
guiedit_update_font_details(gbdfed_editor_t *ed)
{
    bdf_font_t *font;

    if (ed->finfo_dialog == 0)
      return;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    sprintf(buffer1, "%d", font->glyphs_used);
    gtk_label_set_text(GTK_LABEL(ed->finfo_enc_count), buffer1);
    sprintf(buffer1, "%d", font->unencoded_used);
    gtk_label_set_text(GTK_LABEL(ed->finfo_unenc_count), buffer1);
}

static void
enable_apply(GtkWidget *w, gpointer data)
{
    gtk_widget_set_sensitive(w, TRUE);
}

static void
guiedit_enable_comment_buttons(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    gtk_widget_set_sensitive(ed->finfo_erase_comments, TRUE);
    gtk_widget_set_sensitive(ed->finfo_apply_comments, TRUE);
}

static void
guiedit_apply_property(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    GtkToggleButton *tb;
    gchar *v;
    bdf_property_t prop;

    /*
     * Get the property info.
     */
    prop.name = (char *) gtk_entry_get_text(GTK_ENTRY(ed->finfo_prop_name));
    tb = GTK_TOGGLE_BUTTON(ed->finfo_prop_format[BDF_ATOM]);
    if (gtk_toggle_button_get_active(tb))
      prop.format = BDF_ATOM;
    tb = GTK_TOGGLE_BUTTON(ed->finfo_prop_format[BDF_INTEGER]);
    if (gtk_toggle_button_get_active(tb))
      prop.format = BDF_INTEGER;
    tb = GTK_TOGGLE_BUTTON(ed->finfo_prop_format[BDF_CARDINAL]);
    if (gtk_toggle_button_get_active(tb))
      prop.format = BDF_CARDINAL;

    v = (char *) gtk_entry_get_text(GTK_ENTRY(ed->finfo_prop_value));
    switch (prop.format) {
      case BDF_ATOM: prop.value.atom = v; break;
      case BDF_INTEGER: prop.value.int32 = _bdf_atol(v, 0, 10); break;
      case BDF_CARDINAL: prop.value.card32 = _bdf_atoul(v, 0, 10); break;
    }

    fontgrid_set_font_property(FONTGRID(ed->fgrid), &prop);

    if (bdf_is_xlfd_property(prop.name))
      ed->finfo_xlfd_props_modified = TRUE;

    /*
     * Turn the apply button back off.
     */
    gtk_widget_set_sensitive(ed->finfo_apply_prop, FALSE);
}

static void
guiedit_delete_property(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    gchar *name;

    name = (char *) gtk_entry_get_text(GTK_ENTRY(ed->finfo_prop_name));

    fontgrid_delete_font_property(FONTGRID(ed->fgrid), name);

    /*
     * Make sure the Apply button is activated in case the delete was
     * unintentional so the user can apply it again.
     */
    gtk_widget_set_sensitive(ed->finfo_apply_prop, TRUE);
}

static void
guiedit_erase_comments(GtkWidget *w, gpointer data)
{
    GtkTextBuffer *text;
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    gchar *comments;

    fontgrid_set_font_comments(FONTGRID(ed->fgrid), 0);

    /*
     * Clear out the comments text widget.
     */
    text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ed->finfo_comments));
    gtk_text_buffer_set_text(text, "", 0);

    /*
     * Disable the Erase button until the next change in the comment text.
     */
    gtk_widget_set_sensitive(ed->finfo_erase_comments, FALSE);

    /*
     * If the font contains no comments either, disable the Apply button.  If
     * it has comments, then enable the Apply button so that the existing
     * comments can be removed if wanted.
     */
    if (fontgrid_get_font_comments(FONTGRID(ed->fgrid), &comments) == 0)
      gtk_widget_set_sensitive(ed->finfo_apply_comments, FALSE);
    else
      gtk_widget_set_sensitive(ed->finfo_apply_comments, TRUE);

    /*
     * Force the focus back to the comments widget.
     */
    gtk_widget_grab_focus(ed->finfo_comments);
}

static void
guiedit_apply_comments(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    gchar *comments;
    GtkTextBuffer *text;
    GtkTextIter start, end;

    text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ed->finfo_comments));
    gtk_text_buffer_get_bounds(text, &start, &end);
    comments = gtk_text_buffer_get_text(text, &start, &end, TRUE);

    fontgrid_set_font_comments(FONTGRID(ed->fgrid), comments);

    /*
     * Disable the Apply button until the next change in the comment text.
     */
    gtk_widget_set_sensitive(ed->finfo_apply_comments, FALSE);

    /*
     * Force the focus back to the comments widget.
     */
    gtk_widget_grab_focus(ed->finfo_comments);
}

static void
guiedit_change_spacing(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    /*
     * Enable the device width input field depending on which radio button was
     * toggled.
     */
    if (w == ed->finfo_spacing[BDF_PROPORTIONAL >> 4])
      gtk_widget_set_sensitive(ed->finfo_dwidth, FALSE);
    else
      gtk_widget_set_sensitive(ed->finfo_dwidth, TRUE);

    /*
     * Enable the Apply button.
     */
    gtk_widget_set_sensitive(ed->finfo_apply_info, TRUE);
}

static void
guiedit_apply_details(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    GtkToggleButton *tb;
    const gchar *v;
    bdf_property_t sp;
    FontgridFontInfo finfo;

    v = gtk_entry_get_text(GTK_ENTRY(ed->finfo_dwidth));
    finfo.monowidth = (guint16) _bdf_atos((char *) v, 0, 10);
    
    /*
     * Get the current spacing value.
     */
    finfo.spacing = 0;
    tb = GTK_TOGGLE_BUTTON(ed->finfo_spacing[BDF_PROPORTIONAL >> 4]);
    if (gtk_toggle_button_get_active(tb))
      finfo.spacing = BDF_PROPORTIONAL;
    tb = GTK_TOGGLE_BUTTON(ed->finfo_spacing[BDF_MONOWIDTH >> 4]);
    if (gtk_toggle_button_get_active(tb))
      finfo.spacing = BDF_MONOWIDTH;
    tb = GTK_TOGGLE_BUTTON(ed->finfo_spacing[BDF_CHARCELL >> 4]);
    if (gtk_toggle_button_get_active(tb))
      finfo.spacing = BDF_CHARCELL;

    /*
     * Make sure a property gets added to the font as well so it
     * gets saved in the event there is no XLFD name.
     */
    sp.name = "SPACING";
    sp.format = BDF_ATOM;
    switch (finfo.spacing) {
      case BDF_PROPORTIONAL: sp.value.atom = "P"; break;
      case BDF_MONOWIDTH: sp.value.atom = "M"; break;
      case BDF_CHARCELL: sp.value.atom = "C"; break;
    }
    bdf_add_font_property(fontgrid_get_font(FONTGRID(ed->fgrid)), &sp);

    /*
     * Set the font spacing values on all of the visible glyph editors.
     */
    guigedit_set_font_spacing(finfo.spacing, finfo.monowidth);

    /*
     * Get the default character.
     */
    v = gtk_entry_get_text(GTK_ENTRY(ed->finfo_default_char));
    finfo.default_char =
        (glong) _bdf_atol((char *) v, 0,
                          fontgrid_get_code_base(FONTGRID(ed->fgrid)));

    finfo.resolution_x = (glong)
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ed->finfo_hres));
    finfo.resolution_y = (glong)
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ed->finfo_vres));

    v = gtk_entry_get_text(GTK_ENTRY(ed->finfo_ascent));
    finfo.font_ascent = (glong) _bdf_atol((char *) v, 0, 10);
    v = gtk_entry_get_text(GTK_ENTRY(ed->finfo_descent));
    finfo.font_descent = (glong) _bdf_atol((char *) v, 0, 10);

    /*
     * Turn off the Apply button until something else changes.
     */
    gtk_widget_set_sensitive(ed->finfo_apply_info, FALSE);
}

static void
notebook_switch_page(GtkNotebook *nb, GtkNotebookPage *nbp, gint pageno,
                     gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    /*
     * Whenever the comments page is shown, force the focus on the text
     * widget.
     */
    if (pageno == 2)
      gtk_widget_grab_focus(ed->finfo_comments);
}

#define CLINFOMSG "Font Info: Some XLFD properties were modified.\nDo you wish to update the font name with these changes?"

static void
close_finfo(GtkWidget *w, gint response, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (ed->finfo_xlfd_props_modified == TRUE) {
        if (guiutil_yes_or_no(ed->finfo_dialog, CLINFOMSG, TRUE))
          fontgrid_update_font_name_from_properties(FONTGRID(ed->fgrid));
    }
    ed->finfo_xlfd_props_modified = FALSE;

    gtk_widget_hide(ed->finfo_dialog);
}

static void
finfo_sync_res(GtkWidget *w, GdkEventFocus *ev, gpointer data)
{
    gfloat v;
    GtkSpinButton *b;

    b = GTK_SPIN_BUTTON(data);
    v = (gfloat) gtk_spin_button_get_value(b);

    if (v != (gfloat) gtk_spin_button_get_value(GTK_SPIN_BUTTON(w))) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);

#if 0
        /*
         * Enable the apply button.
         */
        gtk_widget_set_sensitive(pref_apply, TRUE);
#endif
    }
}

void
guiedit_show_font_info(GtkWidget *w, gpointer data)
{
    GtkWidget *hbox, *vbox, *frame, *label, *button, *table;
    GtkWidget *nb, *vbox1, *hbox1, *swin, *image;
    GtkTextBuffer *text;
    GtkRadioButton *rb;
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    bdf_font_t *font;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkTreeSelection *sel;
    GtkAdjustment *adj;
    gint idx;

    if (ed->finfo_dialog == 0) {
        font = fontgrid_get_font(FONTGRID(ed->fgrid));

        ed->finfo_dialog = gtk_dialog_new();

        (void) g_signal_connect(G_OBJECT(ed->finfo_dialog),
                                "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        nb = ed->finfo_notebook = gtk_notebook_new();

        (void) g_signal_connect(G_OBJECT(nb), "switch_page",
                                G_CALLBACK(notebook_switch_page),
                                GUINT_TO_POINTER(ed->id));

        /*
         * Create the font info page.
         */

        /*
         * Create the Apply button first so it can be used in callbacks
         * for the data fields.
         */
        ed->finfo_apply_info = gtk_button_new_from_stock(GTK_STOCK_APPLY);
        (void) g_signal_connect(G_OBJECT(ed->finfo_apply_info), "clicked",
                                G_CALLBACK(guiedit_apply_details),
                                GUINT_TO_POINTER(ed->id));

        
        vbox = gtk_vbox_new(FALSE, 0);

        frame = gtk_frame_new("Glyph Counts");
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);

        ed->finfo_enc_count = gtk_label_new("0");
        gtk_misc_set_alignment(GTK_MISC(ed->finfo_enc_count), 0.0, 0.5);
        label = labcon_new_label_defaults("Encoded:",
                                          ed->finfo_enc_count, 0);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

        ed->finfo_unenc_count = gtk_label_new("0");
        gtk_misc_set_alignment(GTK_MISC(ed->finfo_unenc_count), 0.0, 0.5);
        label = labcon_new_label_defaults("Unencoded:",
                                          ed->finfo_unenc_count, 0);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

        gtk_container_add(GTK_CONTAINER(frame), hbox);

        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 5);

        frame = gtk_frame_new("Default Character");
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);

        ed->finfo_default_char = gtk_widget_new(gtk_entry_get_type(),
                                                "max_length", 4, NULL);
        gtk_widget_set_size_request(ed->finfo_default_char, 75, -1);
        g_signal_connect_object(G_OBJECT(ed->finfo_default_char), "changed",
                                G_CALLBACK(enable_apply),
                                (gpointer) ed->finfo_apply_info,
                                G_CONNECT_SWAPPED);
        gtk_box_pack_start(GTK_BOX(hbox), ed->finfo_default_char,
                           FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), hbox);

        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 5);

        frame = gtk_frame_new("Font Spacing");
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);

        idx = BDF_PROPORTIONAL >> 4;

        button = ed->finfo_spacing[idx] =
            gtk_radio_button_new_with_label(0, "Proportional");
        g_signal_connect(G_OBJECT(ed->finfo_spacing[idx]), "toggled",
                         G_CALLBACK(guiedit_change_spacing),
                         GUINT_TO_POINTER(ed->id));
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

        idx++;
        rb = GTK_RADIO_BUTTON(button);
        button = ed->finfo_spacing[idx] =
            gtk_radio_button_new_with_label_from_widget(rb, "Monowidth");
        g_signal_connect(G_OBJECT(ed->finfo_spacing[idx]), "toggled",
                         G_CALLBACK(guiedit_change_spacing),
                         GUINT_TO_POINTER(ed->id));
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

        idx++;
        rb = GTK_RADIO_BUTTON(button);
        button = ed->finfo_spacing[idx] =
            gtk_radio_button_new_with_label_from_widget(rb, "Character Cell");
        g_signal_connect(G_OBJECT(ed->finfo_spacing[idx]), "toggled",
                         G_CALLBACK(guiedit_change_spacing),
                         GUINT_TO_POINTER(ed->id));
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(frame), hbox);
        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 5);

        frame = gtk_frame_new("Font Resolution");
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);

        adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 20.0, 1200.0,
                                                   1.0, 10.0, 0.0);
        ed->finfo_hres = gtk_spin_button_new(adj, 1.0, 0);
        gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(ed->finfo_hres), TRUE);
        gtk_widget_set_size_request(ed->finfo_hres, 75, -1);
        g_signal_connect_object(G_OBJECT(ed->finfo_hres), "value-changed",
                                G_CALLBACK(enable_apply),
                                (gpointer) ed->finfo_apply_info,
                                G_CONNECT_SWAPPED);
        label = labcon_new_label_defaults("Horizontal:", ed->finfo_hres, 0);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 20.0, 1200.0,
                                                   1.0, 10.0, 0.0);
        ed->finfo_vres = gtk_spin_button_new(adj, 1.0, 0);
        gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(ed->finfo_vres), TRUE);
        gtk_widget_set_size_request(ed->finfo_vres, 75, -1);
        g_signal_connect_object(G_OBJECT(ed->finfo_vres), "value-changed",
                                G_CALLBACK(enable_apply),
                                (gpointer) ed->finfo_apply_info,
                                G_CONNECT_SWAPPED);
        g_signal_connect(G_OBJECT(ed->finfo_vres), "focus-in-event",
                         G_CALLBACK(finfo_sync_res),
                         (gpointer) ed->finfo_hres);

        label = labcon_new_label_defaults("Vertical:", ed->finfo_vres, label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        ed->finfo_bpp = gtk_label_new("1");
        label = labcon_new_label_defaults("Bits Per Pixel:", ed->finfo_bpp,
                                          label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(frame), hbox);
        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 5);

        frame = gtk_frame_new("Font Metrics");
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);

        ed->finfo_dwidth = gtk_widget_new(gtk_entry_get_type(),
                                          "max_length", 4, NULL);
        gtk_widget_set_size_request(ed->finfo_dwidth, 75, -1);
        g_signal_connect_object(G_OBJECT(ed->finfo_dwidth), "changed",
                                G_CALLBACK(enable_apply),
                                (gpointer) ed->finfo_apply_info,
                                G_CONNECT_SWAPPED);
        label = labcon_new_label_defaults("Device Width:",
                                          ed->finfo_dwidth, label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        ed->finfo_ascent = gtk_widget_new(gtk_entry_get_type(),
                                          "max_length", 4, NULL);
        gtk_widget_set_size_request(ed->finfo_ascent, 75, -1);
        g_signal_connect_object(G_OBJECT(ed->finfo_ascent), "changed",
                                G_CALLBACK(enable_apply),
                                (gpointer) ed->finfo_apply_info,
                                G_CONNECT_SWAPPED);
        label = labcon_new_label_defaults("Font Ascent:", ed->finfo_ascent,
                                          label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        ed->finfo_descent = gtk_widget_new(gtk_entry_get_type(),
                                           "max_length", 4, NULL);
        gtk_widget_set_size_request(ed->finfo_descent, 75, -1);
        g_signal_connect_object(G_OBJECT(ed->finfo_descent), "changed",
                                G_CALLBACK(enable_apply),
                                (gpointer) ed->finfo_apply_info,
                                G_CONNECT_SWAPPED);
        label = labcon_new_label_defaults("Font Descent:",
                                          ed->finfo_descent, label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(frame), hbox);

        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 5);

        /*
         * Finally, add the Apply button.
         */
        gtk_box_pack_end(GTK_BOX(vbox), ed->finfo_apply_info, FALSE, FALSE, 5);

        label = gtk_label_new("Font Info");
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), vbox, label);

        /*
         * Create the font properties editor.
         */
        vbox = gtk_vbox_new(FALSE, 5);

        table = gtk_table_new(1, 2, TRUE);

        swin = gtk_scrolled_window_new(0, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                       GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
        gtk_container_set_border_width(GTK_CONTAINER(swin), 3);

        store = gtk_list_store_new(1, G_TYPE_STRING);
        ed->finfo_font_props =
            gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);

        gtk_widget_set_size_request(ed->finfo_font_props, -1, 200);
        gtk_container_add(GTK_CONTAINER(swin), ed->finfo_font_props);

        cell_renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Font Properties",
                                                          cell_renderer,
                                                          "text", 0,
                                                          NULL);

        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(ed->finfo_font_props),
                                    column);
        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ed->finfo_font_props));

        (void) g_signal_connect(G_OBJECT(sel), "changed",
                                G_CALLBACK(guiedit_select_property),
                                GUINT_TO_POINTER(ed->id));

        gtk_table_attach(GTK_TABLE(table), swin, 0, 1, 0, 1,
                         GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

        swin = gtk_scrolled_window_new(0, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                       GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
        gtk_container_set_border_width(GTK_CONTAINER(swin), 3);

        store = gtk_list_store_new(1, G_TYPE_STRING);
        ed->finfo_all_props =
            gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);

        gtk_widget_set_size_request(ed->finfo_all_props, -1, 200);
        gtk_container_add(GTK_CONTAINER(swin), ed->finfo_all_props);

        cell_renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("All Properties",
                                                          cell_renderer,
                                                          "text", 0,
                                                          NULL);

        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(ed->finfo_all_props),
                                    column);
        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ed->finfo_all_props));

        (void) g_signal_connect(G_OBJECT(sel), "changed",
                                G_CALLBACK(guiedit_select_property),
                                GUINT_TO_POINTER(ed->id));

        gtk_table_attach(GTK_TABLE(table), swin, 1, 2, 0, 1,
                         GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

        gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);

        frame = gtk_frame_new("Property");
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        vbox1 = gtk_vbox_new(FALSE, 5);

        table = gtk_table_new(3, 2, FALSE);

        label = gtk_label_new("Name:");
        gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
                         GTK_FILL, GTK_FILL, 5, 0);

        label = gtk_label_new("Value:");
        gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
                         GTK_FILL, GTK_FILL, 5, 0);

        label = gtk_label_new("Type:");
        gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
                         GTK_FILL, GTK_FILL, 5, 0);

        ed->finfo_prop_name = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), ed->finfo_prop_name, 1, 2, 0, 1,
                         GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 5);

        ed->finfo_prop_value = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), ed->finfo_prop_value, 1, 2, 1, 2,
                         GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 5);

        hbox = gtk_hbox_new(FALSE, 2);
        button = ed->finfo_prop_format[BDF_ATOM] =
            gtk_radio_button_new_with_label(0, "String");
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        rb = GTK_RADIO_BUTTON(button);
        button = ed->finfo_prop_format[BDF_INTEGER] =
            gtk_radio_button_new_with_label_from_widget(rb, "Integer");
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        rb = GTK_RADIO_BUTTON(button);
        button = ed->finfo_prop_format[BDF_CARDINAL] =
            gtk_radio_button_new_with_label_from_widget(rb, "Cardinal");
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

        gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 2, 3,
                         GTK_FILL, GTK_FILL, 0, 0);

        gtk_box_pack_start(GTK_BOX(vbox1), table, FALSE, FALSE, 0);

        hbox = gtk_hbox_new(FALSE, 0);

        hbox1 = gtk_hbox_new(FALSE, 0);
        label = gtk_label_new_with_mnemonic("_Apply To Font Properties");
        image = gtk_image_new_from_stock(GTK_STOCK_APPLY,
                                         GTK_ICON_SIZE_BUTTON);
        gtk_box_pack_start(GTK_BOX(hbox1), image, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), label, FALSE, FALSE, 0);

        button = ed->finfo_apply_prop = gtk_button_new();
        gtk_container_add(GTK_CONTAINER(button), hbox1);

        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(guiedit_apply_property),
                                GUINT_TO_POINTER(ed->id));

        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

        (void) g_signal_connect_object(G_OBJECT(ed->finfo_prop_value),
                                       "changed",
                                       G_CALLBACK(enable_apply),
                                       (gpointer) button,
                                       G_CONNECT_SWAPPED);

        button = ed->finfo_prop_format[BDF_ATOM];
        (void) g_signal_connect_object(G_OBJECT(button),
                                       "toggled",
                                       G_CALLBACK(enable_apply),
                                       (gpointer) ed->finfo_apply_prop,
                                       G_CONNECT_SWAPPED);

        button = ed->finfo_prop_format[BDF_INTEGER];
        (void) g_signal_connect_object(G_OBJECT(button),
                                       "toggled",
                                       G_CALLBACK(enable_apply),
                                       (gpointer) ed->finfo_apply_prop,
                                       G_CONNECT_SWAPPED);

        button = ed->finfo_prop_format[BDF_CARDINAL];
        (void) g_signal_connect_object(G_OBJECT(button),
                                       "toggled",
                                       G_CALLBACK(enable_apply),
                                       (gpointer) ed->finfo_apply_prop,
                                       G_CONNECT_SWAPPED);

        hbox1 = gtk_hbox_new(FALSE, 0);
        label = gtk_label_new_with_mnemonic("_Delete From Font Properties");
        image = gtk_image_new_from_stock(GTK_STOCK_DELETE,
                                         GTK_ICON_SIZE_BUTTON);
        gtk_box_pack_start(GTK_BOX(hbox1), image, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), label, FALSE, FALSE, 0);

        button = ed->finfo_delete_prop = gtk_button_new();
        gtk_container_add(GTK_CONTAINER(button), hbox1);

        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(guiedit_delete_property),
                                GUINT_TO_POINTER(ed->id));

        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);

        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 2);

        gtk_container_add(GTK_CONTAINER(frame), vbox1);

        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 2);

        label = gtk_label_new("Font Properties");
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), vbox, label);

        /*
         * Create the font comment editor.
         */
        frame = gtk_frame_new("Comments");
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        table = gtk_table_new(2, 2, FALSE);

        text = gtk_text_buffer_new(NULL);
        ed->finfo_comments = gtk_text_view_new_with_buffer(text);
        gtk_widget_set_size_request(ed->finfo_comments, 400, 200);

        swin = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_ALWAYS);
        gtk_container_add(GTK_CONTAINER(swin), ed->finfo_comments);
        gtk_table_attach(GTK_TABLE(table), swin, 0, 1, 0, 1,
                         GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 5, 5);

        hbox = gtk_hbox_new(FALSE, 0);
        button = ed->finfo_apply_comments =
            gtk_button_new_from_stock(GTK_STOCK_APPLY);

        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(guiedit_apply_comments),
                                GUINT_TO_POINTER(ed->id));
        (void) g_signal_connect_data((gpointer) text,
                                     "changed",
                                     G_CALLBACK(guiedit_enable_comment_buttons),
                                     GUINT_TO_POINTER(ed->id), NULL, 0);
        gtk_container_add(GTK_CONTAINER(hbox), button);

        button = ed->finfo_erase_comments =
            gtk_button_new_from_stock(GTK_STOCK_CLEAR);

        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(guiedit_erase_comments),
                                GUINT_TO_POINTER(ed->id));

        gtk_container_add(GTK_CONTAINER(hbox), button);
        gtk_table_attach(GTK_TABLE(table), hbox, 0, 2, 1, 2,
                         GTK_FILL, GTK_FILL, 0, 5);
        gtk_container_add(GTK_CONTAINER(frame), table);

        label = gtk_label_new("Font Comments");
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), frame, label);

        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(ed->finfo_dialog)->vbox),
                          nb);

        /*
         * Add the buttons.
         */
        gtk_dialog_add_button(GTK_DIALOG(ed->finfo_dialog),
                              GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);

        g_signal_connect(G_OBJECT(ed->finfo_dialog), "response",
                         G_CALLBACK(close_finfo), GUINT_TO_POINTER(ed->id));

        guiedit_update_font_info(ed);

        gtk_widget_show_all(GTK_DIALOG(ed->finfo_dialog)->vbox);
        gtk_widget_show_all(GTK_DIALOG(ed->finfo_dialog)->action_area);
    }

    /*
     * Center the dialog and show it.
     */
    guiutil_show_dialog_centered(ed->finfo_dialog, ed->shell);
}

void
guiedit_show_font_properties(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    guiedit_show_font_info(w, data);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ed->finfo_notebook), 1);
}

void
guiedit_show_font_comments(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    guiedit_show_font_info(w, data);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ed->finfo_notebook), 2);
}

/*
 * Called when the font name is used to update the properties so the font info
 * editor list of properties is updated.
 */
void
guiedit_update_font_properties(gbdfed_editor_t *ed)
{
    gchar *prop;
    guint32 i, nprops;
    bdf_font_t *font;
    bdf_property_t *props;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreePath *row;
    GtkTreeView *tview;
    GtkTreeIter iter;
    GValue val;

    /*
     * Update the font properties list if the font info dialog has been
     * created.
     */
    if (ed->finfo_dialog == 0)
      return;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    /*
     * This will be the name of the currently selected property, if one
     * is selected.
     */
    prop = 0;

    selection =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(ed->finfo_font_props));
    if (gtk_tree_selection_get_selected(selection, &model, &iter) != FALSE) {
        memset((char *) &val, 0, sizeof(GValue));
        gtk_tree_model_get_value(model, &iter, 0, &val);
        prop = (gchar *) g_value_get_string(&val);
    }

    /*
     * This will track the row that needs to be reselected once the list has
     * been updated.
     */
    row = 0;

    gtk_list_store_clear(GTK_LIST_STORE(model));

    if ((nprops = bdf_font_property_list(font, &props)) != 0) {
        for (i = 0; i < nprops; i++) {
            if (prop && strcmp(prop, props[i].name) == 0)
              row = gtk_tree_path_new_from_indices(i, -1);
            gtk_list_store_append(GTK_LIST_STORE(model), &iter);
            gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                               0, props[i].name, -1);
        }
    }

    if (row != 0) {
        gtk_tree_selection_select_path(selection, row);

        /*
         * Make sure the selected property is made visible.
         */
        tview = gtk_tree_selection_get_tree_view(selection);
        gtk_tree_view_scroll_to_cell(tview, row, NULL, TRUE, 0.5, 0.5);
    }

    /*
     * Make sure memory is deallocated when necessary.
     */
    if (prop != 0)
      g_value_unset(&val);
}

void
guiedit_copy_selection(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_copy_selection(FONTGRID(ed->fgrid));
}

void
guiedit_cut_selection(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_cut_selection(FONTGRID(ed->fgrid));
}

void
guiedit_paste_selection(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_paste_selection(FONTGRID(ed->fgrid), FONTGRID_NORMAL_PASTE);
}

void
guiedit_overlay_selection(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_paste_selection(FONTGRID(ed->fgrid), FONTGRID_OVERLAY_PASTE);
}

void
guiedit_insert_selection(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_paste_selection(FONTGRID(ed->fgrid), FONTGRID_INSERT_PASTE);
}

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

/**************************************************************************
 *
 * Application globals.
 *
 **************************************************************************/

gchar buffer1[BUFSIZ];
gchar buffer2[BUFSIZ];
gbdfed_options_t options;

/*
 * The list of editors that exist.
 */
gbdfed_editor_t *editors;
guint num_editors;

/**************************************************************************
 *
 * Forward declarations and local variables.
 *
 **************************************************************************/

/*
 * These are formats that can appear in the editor for importing/loading and
 * exporting fonts.
 */
#define XMBDFED_BDF_FORMAT     1
#define XMBDFED_CONSOLE_FORMAT 2
#define XMBDFED_PKGF_FORMAT    3
#define XMBDFED_FNT_FORMAT     4
#define XMBDFED_HBF_FORMAT     5
#define XMBDFED_TTF_FORMAT     6
#define XMBDFED_PSF_FORMAT     7
#define XMBDFED_HEX_FORMAT     8

/**************************************************************************
 *
 * Application icons.
 *
 **************************************************************************/

static const gchar *gbdfed_16x16[] = {
/* width height num_colors chars_per_pixel */
"    16    16        5            1",
/* colors */
". c #dcdcdc",
"# c #ff0000",
"a c #ffffff",
"b c #000000",
"c c #b2c0dc",
/* pixels */
"..#.............",
"a.#.a.a.a.a.a.a.",
"..#.............",
"a.#bbba.a.a.a.b.",
"..#.bbb......b..",
"a.#.abbba.a.b.a.",
"..#...bbb..b....",
"a.#.a.abb.b.a.a.",
"..#....b.bb.....",
"a.#.a.b.abbba.a.",
"..#..b....bbb...",
"a.#.b.a.a.abbba.",
"..#b........bbb.",
"################",
"..#.............",
"c.#.c.c.c.c.c.c."
};

static const gchar *gbdfed_32x32[] = {
/* width height num_colors chars_per_pixel */
"    32    32        4            1",
/* colors */
". c #d9d9d9",
"# c #ffffff",
"a c #ff0000",
"b c #000000",
/* pixels */
".##a##.##.##.##.##.##.##.##.##.#",
".##a##.##.##.##.##.##.##.##.##.#",
"...a............................",
".##a##.##.##.##.##.##.##.##.##.#",
".##a##.##.##.##.##.##.##.##.##.#",
"...a............................",
".##abb.bb.bb.##.##.##.##.##.bb.#",
".##abb.bb.bb.##.##.##.##.##.bb.#",
"...a............................",
".##a##.bb.bb.bb.##.##.##.bb.##.#",
".##a##.bb.bb.bb.##.##.##.bb.##.#",
"...a............................",
".##a##.##.bb.bb.bb.##.bb.##.##.#",
".##a##.##.bb.bb.bb.##.bb.##.##.#",
"...a............................",
".##a##.##.##.bb.##.bb.##.##.##.#",
".##a##.##.##.bb.##.bb.##.##.##.#",
"...a............................",
".##a##.##.bb.##.bb.bb.bb.##.##.#",
".##a##.##.bb.##.bb.bb.bb.##.##.#",
"...a............................",
".##a##.bb.##.##.##.bb.bb.bb.##.#",
".##a##.bb.##.##.##.bb.bb.bb.##.#",
"...a............................",
".##abb.##.##.##.##.##.bb.bb.bb.#",
".##abb.##.##.##.##.##.bb.bb.bb.#",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
".##a##.##.##.##.##.##.##.##.##.#",
".##a##.##.##.##.##.##.##.##.##.#",
"...a............................",
".##a##.##.##.##.##.##.##.##.##.#",
".##a##.##.##.##.##.##.##.##.##.#"
};

static const gchar *gbdfed_48x48[] = {
/* width height num_colors chars_per_pixel */
"    48    48        4            1",
/* colors */
". c #dcdcdc",
"# c #ff0000",
"a c #ffffff",
"b c #000000",
/* pixels */
"....#...........................................",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa..aa.aaa.aaa",
"....#...........................................",
".aaa#bbb.bbb.bbb.aaa.aaa.aaa.aaa.aaa.aaa.aaa.bbb",
".aaa#bbb.bbb.bbb.aaa.aaa.aaa.aaa.aaa.aaa.aaa.bbb",
".aaa#bbb.bbb.bbb.aaa.aaa.aaa.aaa.aaa.aaa.aaa.bbb",
"....#...........................................",
".aaa#aaa.bbb.bbb.bbb.aaa.aaa.aaa.aaa.aaa.bbb.aaa",
".aaa#aaa.bbb.bbb.bbb.aaa.aaa.aaa.aaa.aaa.bbb.aaa",
".aaa#aaa.bbb.bbb.bbb.aaa.aaa.aaa.aaa.aaa.bbb.aaa",
"....#...........................................",
".aaa#aaa.aaa.bbb.bbb.bbb.aaa.aaa.aaa.bbb.aaa.aaa",
".aaa#aaa.aaa.bbb.bbb.bbb.aaa.aaa.aaa.bbb.aaa.aaa",
".aaa#aaa.aaa.bbb.bbb.bbb.aaa.aaa.aaa.bbb.aaa.aaa",
"....#...........................................",
".aaa#aaa.aaa.aaa.bbb.bbb.bbb.aaa.bbb.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.bbb.bbb.bbb.aaa.bbb.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.bbb.bbb.bbb.aaa.bbb.aaa.aaa.aaa",
"....#...........................................",
".aaa#aaa.aaa.aaa.aaa.bbb.aaa.bbb.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.bbb.aaa.bbb.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.bbb.aaa.bbb.aaa.aaa.aaa.aaa",
"....#...........................................",
".aaa#aaa.aaa.aaa.bbb.aaa.bbb.bbb.bbb.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.bbb.aaa.bbb.bbb.bbb.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.bbb.aaa.bbb.bbb.bbb.aaa.aaa.aaa",
"....#...........................................",
".aaa#aaa.aaa.bbb.aaa.aaa.aaa.bbb.bbb.bbb.aaa.aaa",
".aaa#aaa.aaa.bbb.aaa.aaa.aaa.bbb.bbb.bbb.aaa.aaa",
".aaa#aaa.aaa.bbb.aaa.aaa.aaa.bbb.bbb.bbb.aaa.aaa",
"....#...........................................",
".aaa#aaa.bbb.aaa.aaa.aaa.aaa.aaa.bbb.bbb.bbb.aaa",
".aaa#aaa.bbb.aaa.aaa.aaa.aaa.aaa.bbb.bbb.bbb.aaa",
".aaa#aaa.bbb.aaa.aaa.aaa.aaa.aaa.bbb.bbb.bbb.aaa",
"....#...........................................",
".aaa#bbb.aaa.aaa.aaa.aaa.aaa.aaa.aaa.bbb.bbb.bbb",
".aaa#bbb.aaa.aaa.aaa.aaa.aaa.aaa.aaa.bbb.bbb.bbb",
".aaa#bbb.aaa.aaa.aaa.aaa.aaa.aaa.aaa.bbb.bbb.bbb",
"################################################",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa",
"....#...........................................",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa",
".aaa#aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa.aaa"
};

static gboolean icons_set = FALSE;

/**************************************************************************
 *
 * GTK signal handlers.
 *
 **************************************************************************/

static gboolean
quit_application(GtkWidget *w, GdkEvent *ev, gpointer data)
{
    guint i, modified;

    for (i = modified = 0; i < num_editors; i++) {
        if (fontgrid_get_font_modified(FONTGRID(editors[i].fgrid)))
          modified++;
    }

    if (modified) {
        if (modified == 1)
          sprintf(buffer1, "Save Font: One font was modified.  Save?");
        else
          sprintf(buffer1, "Save Font: %d fonts were modified.  Save?",
                  modified);
        if (guiutil_yes_or_no(editors[0].shell, buffer1, TRUE)) {

            /*
             * Go through each editor and ask if the font should be saved if
             * it has been modified.
             */
            for (i = 0; i < num_editors; i++) {
                if (fontgrid_get_font_modified(FONTGRID(editors[i].fgrid))) {
                    /*
                     * Ask if this font should be saved.
                     */
                    if (editors[i].file)
                      sprintf(buffer1, "Save Font: Save %s?", editors[i].file);
                    else
                      sprintf(buffer1, "Save Font: Save unnamed%d.bdf?", i);

                    /*
                     * Always ask this question using the shell window of the
                     * first editor so the dialog box doesn't move around.
                     */
                    if (guiutil_yes_or_no(editors[0].shell, buffer1, TRUE))
                      guifile_save_as_wait(w, GUINT_TO_POINTER(i));
                }
            }
        }
    }

    /*
     * Ask if the user really wants to exit if their preferences specify this
     * question should be asked.
     */
    if (options.really_exit &&
        !guiutil_yes_or_no(editors[0].shell, "Really Quit?", TRUE))
      return TRUE;

    /*
     * Call all the cleanup routines in case something really needs to be
     * deallocated.
     */
    guigedit_cleanup();
    guiedit_preference_cleanup();
    guiutil_cursor_cleanup();
    guihelp_cleanup();

    bdf_cleanup();
    gtk_main_quit();
    exit(0);
}

static void
show_editor(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    /*
     * Nothing seems to force the original window to the top of the stack
     * on the screen, but this supposedly does everything necessary.
     */
    gtk_widget_show_all(ed->shell);
    gtk_window_present(GTK_WINDOW(ed->shell));
}

static void
goto_other_page(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    gint32 opage;
    FontgridPageInfo pi;

    fontgrid_get_page_info(FONTGRID(ed->fgrid), &pi);

    if (!pi.unencoded_page) {
        opage = ed->last_pageno;
        ed->last_pageno = pi.current_page;
    } else {
        opage = ed->last_upageno;
        ed->last_upageno = pi.current_page;
    }

    if (opage != -1 && opage != pi.current_page)
      fontgrid_goto_page(FONTGRID(ed->fgrid), opage);
}

static void
toggle_encoding_view(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    gchar *label;
    GtkLabel *lw;

    if (fontgrid_viewing_unencoded(FONTGRID(ed->fgrid)))
      label = "Unencoded";
    else
      label = "Encoded";
    lw = GTK_LABEL(gtk_bin_get_child(GTK_BIN(ed->view_unencoded)));
    gtk_label_set_text(lw, label);

    fontgrid_switch_encoding_view(FONTGRID(ed->fgrid));
}

static void
toggle_view_orientation(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    gchar *label;
    GtkLabel *lw;

    if (fontgrid_get_orientation(FONTGRID(ed->fgrid)) ==
        GTK_ORIENTATION_VERTICAL) {
        fontgrid_set_orientation(FONTGRID(ed->fgrid),
                                 GTK_ORIENTATION_HORIZONTAL);
        label = "Vertical View";
    } else {
        fontgrid_set_orientation(FONTGRID(ed->fgrid),
                                 GTK_ORIENTATION_VERTICAL);
        label = "Horizontal View";
    }
    lw = GTK_LABEL(gtk_bin_get_child(GTK_BIN(ed->view_unencoded)));
    gtk_label_set_text(lw, label);
}

static void
set_code_base(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    gint base;

    if (w == ed->view_oct)
      base = 8;
    else if (w == ed->view_dec)
      base = 10;
    else
      base = 16;

    fontgrid_set_code_base(FONTGRID(ed->fgrid), base);

    /*
     * Make sure the font info editor is updated when the code base
     * changes.
     */
    guiedit_update_code_base(ed);
    guigedit_set_code_base(base);
}

static void
page_change(GtkWidget *w, gpointer pinfo, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    FontgridPageInfo *pi = (FontgridPageInfo *) pinfo;
    gchar *label;
    GtkLabel *lw;

    if (pi->previous_page < 0) {
        gtk_widget_set_sensitive(ed->prev, FALSE);
        gtk_widget_set_sensitive(ed->first, FALSE);
    } else {
        gtk_widget_set_sensitive(ed->prev, TRUE);
        gtk_widget_set_sensitive(ed->first, TRUE);
    }

    if (pi->next_page < 0) {
        gtk_widget_set_sensitive(ed->next, FALSE);
        gtk_widget_set_sensitive(ed->last, FALSE);
    } else {
        gtk_widget_set_sensitive(ed->next, TRUE);
        gtk_widget_set_sensitive(ed->last, TRUE);
    }

    /*
     * Update the page number field with the current page.
     */
    sprintf(buffer1, "%d", pi->current_page);
    gtk_entry_set_text(GTK_ENTRY(ed->pageno), buffer1);
    gtk_editable_set_position(GTK_EDITABLE(ed->pageno), -1);

    /*
     * Finally, modify the label on the Encoded/Unencoded view menu item.
     */
    label = (pi->unencoded_page) ? "Encoded" : "Unencoded";
    lw = GTK_LABEL(gtk_bin_get_child(GTK_BIN(ed->view_unencoded)));
    gtk_label_set_text(lw, label);
}

static void
first_page(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);

    fontgrid_goto_first_page(FONTGRID(ed->fgrid));

    /*
     * Force the focus back to the Fontgrid.
     */
    gtk_widget_grab_focus(ed->fgrid);
}

static void
previous_page(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);

    fontgrid_goto_previous_page(FONTGRID(ed->fgrid));

    /*
     * Force the focus back to the Fontgrid.
     */
    gtk_widget_grab_focus(ed->fgrid);
}

static void
next_page(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);

    fontgrid_goto_next_page(FONTGRID(ed->fgrid));

    /*
     * Force the focus back to the Fontgrid.
     */
    gtk_widget_grab_focus(ed->fgrid);
}

static void
last_page(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);

    fontgrid_goto_last_page(FONTGRID(ed->fgrid));

    /*
     * Force the focus back to the Fontgrid.
     */
    gtk_widget_grab_focus(ed->fgrid);
}

static void
goto_page_or_code(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    const gchar *text;
    gint32 code;
    FontgridPageInfo pi;

    fontgrid_get_page_info(FONTGRID(ed->fgrid), &pi);

    if (!pi.unencoded_page)
      ed->last_pageno = pi.current_page;
    else
      ed->last_upageno = pi.current_page;

    if (w == ed->pageno) {
        text = gtk_entry_get_text(GTK_ENTRY(ed->pageno));
        fontgrid_goto_page(FONTGRID(ed->fgrid),
                           _bdf_atol((char *) text, 0, 10));
    } else {
        text = gtk_entry_get_text(GTK_ENTRY(ed->charno));
        code = _bdf_atol((char *) text, 0,
                         fontgrid_get_code_base(FONTGRID(ed->fgrid)));
        fontgrid_goto_code(FONTGRID(ed->fgrid), code);
    }
}

static void
update_selection_info(GtkWidget *w, gpointer sinfo, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    FontgridSelectionInfo *si = (FontgridSelectionInfo *) sinfo;
    short as, ds, rt, lt;
    gint32 start, end;
    guint b1 = 0, b2 = 0, b3, b4;

    as = ds = rt = lt = 0;
    if (si->start == si->end) {
        if (si->num_glyphs != 0 && si->glyphs != 0) {
            b1 = (si->start >> 8) & 0xff;
            b2 = si->start & 0xff;

            as = si->glyphs->bbx.ascent;
            ds = si->glyphs->bbx.descent;
            lt = si->glyphs->bbx.x_offset;
            rt = si->glyphs->bbx.width + lt;
            if (si->glyphs->name != 0)
              (void) strcpy(buffer1, si->glyphs->name);
            else
              sprintf(buffer1, "char%d", si->glyphs->encoding);

            /*
             * If the glyph test dialog is active, send it the glyph if this
             * is an end selection event.
             */
            if (si->reason == FONTGRID_END_SELECTION && glyphtest != 0)
              glyphtest_add_glyph(GLYPHTEST(glyphtest),
                                  fontgrid_get_font(FONTGRID(ed->fgrid)),
                                  si->glyphs);
        } else
          sprintf(buffer1, "char%d", si->start);

        switch (si->base) {
          case 8:
            sprintf(buffer2, "\"%s\" %o (%o, %o)", buffer1,
                    si->start, b1, b2);
            break;
          case 10:
            sprintf(buffer2, "\"%s\" %d (%d, %d)", buffer1,
                    si->start, b1, b2);
            break;
          case 16:
            sprintf(buffer2, "\"%s\" %04X (%02X, %02X)", buffer1,
                    si->start, b1, b2);
            break;
        }

    } else {
        /*
         * A range of glyphs has been selected.
         */
        if (si->end < si->start) {
            start = si->end;
            end = si->start;
        } else {
            start = si->start;
            end = si->end;
        }
        b1 = (start >> 8) & 0xff;
        b2 = start & 0xff;
        b3 = (end >> 8) & 0xff;
        b4 = end & 0xff;

        switch (si->base) {
          case 8:
            sprintf(buffer2, "Selection %o (%o, %o) - %o (%o, %o)",
                    start, b1, b2, end, b3, b4);
            break;
          case 10:
            sprintf(buffer2, "Selection %d (%d, %d) - %d (%d, %d)",
                    start, b1, b2, end, b3, b4);
            break;
          case 16:
            sprintf(buffer2, "Selection %04X (%02X, %02X) - %04X (%02X, %02X)",
                    start, b1, b2, end, b3, b4);
            break;
        }
    }

    /*
     * Update the glyph info label.
     */
    gtk_label_set_text(GTK_LABEL(ed->charinfo), buffer2);

    /*
     * Update the metrics label.
     */
    sprintf(buffer1, "ascent %hd descent %hd right %hd left %hd",
            as, ds, rt, lt);
    gtk_label_set_text(GTK_LABEL(ed->metrics), buffer1);

    switch (si->reason) {
      case FONTGRID_START_SELECTION:
        break;
      case FONTGRID_EXTEND_SELECTION:
        break;
      case FONTGRID_END_SELECTION:
        break;
      case FONTGRID_ACTIVATE:
        guigedit_edit_glyph(ed, si);
        break;
      case FONTGRID_BASE_CHANGE:
        break;
    }
}

static void
handle_modified_signal(GtkWidget *w, gpointer minfo, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    gchar *prgname = g_get_prgname();
    FontgridModificationInfo *mi;

    mi = (FontgridModificationInfo *) minfo;

    /*
     * Check to see what change was made so updates can be handled
     * appropriately.
     */
    if (mi->reason == FONTGRID_NAME_MODIFIED)
      gtk_entry_set_text(GTK_ENTRY(ed->fontname), mi->name);
    else if (mi->reason == FONTGRID_PROPERTIES_MODIFIED)
      /*
       * Make sure the font info editing dialog knows that the list of
       * font properties changed.
       */
      guiedit_update_font_properties(ed);
    else if (mi->reason == FONTGRID_GLYPHS_DELETED ||
             mi->reason == FONTGRID_GLYPHS_PASTED)
      guiedit_update_font_details(ed);
    else if ((mi->reason == FONTGRID_DEVICE_WIDTH_MODIFIED ||
              mi->reason == FONTGRID_GLYPHS_MODIFIED) && glyphtest != 0)
      /*
       * Update the glyph test widget with the new device width.
       */
      glyphtest_update_device_width(GLYPHTEST(glyphtest),
                                    fontgrid_get_font(FONTGRID(ed->fgrid)));

    /*
     * Update the title.
     */
    if (ed->file == 0)
      sprintf(buffer1, "%s - (unnamed%d) [modified]", prgname, ed->id);
    else
      sprintf(buffer1, "%s - %s [modified]", prgname, ed->file);

    gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);
}

static void
view_font_messages(GtkWidget *w, gpointer editor)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(editor);
    GtkWidget *dialog, *frame, *button, *label, *vbox, *sw;
    GtkTextBuffer *text;
    gchar *msgs;

    if (ed->messages_dialog == 0) {
        /*
         * Special case of no messages and the menu item hasn't been
         * checked for sensitivity yet.
         */
        if (fontgrid_get_font_messages(FONTGRID(ed->fgrid)) == 0)
          return;

        dialog = ed->messages_dialog = gtk_dialog_new();
        (void) g_signal_connect(G_OBJECT(dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        frame = gtk_frame_new(0);
        label = ed->messages_label = gtk_label_new("");
        gtk_container_add(GTK_CONTAINER(frame), label);

        vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_ALWAYS);
        gtk_box_pack_start(GTK_BOX(vbox), sw, FALSE, FALSE, 0);

        /*
         * Add the text widget.
         */
        text = gtk_text_buffer_new(NULL);
        ed->messages_text = gtk_text_view_new_with_buffer(text);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(ed->messages_text),
                                         FALSE);
        gtk_widget_set_size_request(ed->messages_text, 500, 200);

        gtk_text_view_set_editable(GTK_TEXT_VIEW(ed->messages_text), FALSE);
        gtk_container_add(GTK_CONTAINER(sw), ed->messages_text);

        button = gtk_button_new_with_label("Close");
        (void) g_signal_connect_object(G_OBJECT(button), "clicked",
                                       G_CALLBACK(gtk_widget_hide),
                                       (gpointer) dialog,
                                       G_CONNECT_SWAPPED);
        gtk_container_add(GTK_CONTAINER(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
                          button);

        gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(ed->messages_dialog)));
        gtk_widget_show_all(gtk_dialog_get_action_area(GTK_DIALOG(ed->messages_dialog)));
    } else
      text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ed->messages_text));

    /*
     * Update the label with the font name in case it changed since the last
     * time the messages were shown.
     */
    if (ed->file != 0)
      sprintf(buffer1, "%s: Messages", ed->file);
    else
      sprintf(buffer1, "unnamed%d.bdf: Messages", ed->id);
    gtk_label_set_text(GTK_LABEL(ed->messages_label), buffer1);

    /*
     * Now change the text itself.
     */
    if ((msgs = fontgrid_get_font_messages(FONTGRID(ed->fgrid))) == 0)
      msgs = "";
    gtk_text_buffer_set_text(text, msgs, -1);

    guiutil_show_dialog_centered(ed->messages_dialog, ed->shell);
}

/**************************************************************************
 *
 * Menu construction.
 *
 **************************************************************************/

static GtkWidget *
make_accel_menu_item(GtkWidget *menu, const gchar *text, const gchar *accel,
                     GtkAccelGroup *ag)
{
    GtkWidget *mi;
    guint key;
    GdkModifierType mods;

    mi = gtk_menu_item_new_with_mnemonic(text);

    gtk_accelerator_parse(accel, &key, &mods);
    gtk_widget_add_accelerator(mi, "activate", ag, key, mods,
                               GTK_ACCEL_VISIBLE|GTK_ACCEL_LOCKED);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    return mi;
}

/*
 * Handle all the menu checking needed to enable or disable menu items on
 * all the menus.
 */
static gint
menu_popup(GtkWidget *w, GdkEvent *event, gpointer data)
{
    gbdfed_editor_t *ed;
    bdf_font_t *font;
    GtkWidget *mitem;
    GList *kids, *label;
    gboolean flag;
    guint i;

    /*
     * Get a pointer to the editor.
     */
    ed = editors + GPOINTER_TO_UINT(data);

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    if (event->type == GDK_MAP) {
        if (w == ed->file_menu) {
            if (font && font->modified)
              gtk_widget_set_sensitive(ed->file_save, TRUE);
            else
              gtk_widget_set_sensitive(ed->file_save, FALSE);

            if (font && font->glyphs_used > 0)
              gtk_widget_set_sensitive(ed->file_export, TRUE);
            else
              gtk_widget_set_sensitive(ed->file_export, FALSE);
        } else if (w == ed->edit_menu) {
            /*
             * If the fontgrid clipboard is empty, then disable the paste,
             * overlay, and insert menu options.
             */
            flag = fontgrid_clipboard_empty(FONTGRID(ed->fgrid)) ?
                FALSE : TRUE;
            gtk_widget_set_sensitive(ed->edit_paste, flag);
            gtk_widget_set_sensitive(ed->edit_overlay, flag);
            gtk_widget_set_sensitive(ed->edit_insert, flag);

            /*
             * If there is no selection, disable the cut and copy menu
             * options.
             */
            flag = fontgrid_has_selection(FONTGRID(ed->fgrid), 0) ?
                TRUE : FALSE;
            gtk_widget_set_sensitive(ed->edit_cut, flag);
            gtk_widget_set_sensitive(ed->edit_copy, flag);

            flag = (font && font->glyphs_used > 0) ? TRUE : FALSE;

            /*
             * Disable glyph rename when viewing unecoded glyphs because their
             * encoding values are not actual.
             */
            if (fontgrid_viewing_unencoded(FONTGRID(ed->fgrid)) || !flag)
              gtk_widget_set_sensitive(ed->edit_rename_glyphs, FALSE);
            else
              gtk_widget_set_sensitive(ed->edit_rename_glyphs, TRUE);

            gtk_widget_set_sensitive(ed->edit_test_glyphs, flag);
        } else if (w == ed->edit_rename_menu) {
            if (options.adobe_name_file != 0)
              gtk_widget_set_sensitive(ed->edit_adobe_names, TRUE);
            else
              gtk_widget_set_sensitive(ed->edit_adobe_names, FALSE);
            if (options.unicode_name_file != 0)
              gtk_widget_set_sensitive(ed->edit_unicode_names, TRUE);
            else
              gtk_widget_set_sensitive(ed->edit_unicode_names, FALSE);
        } else if (w == ed->name_submenu) {
            flag = (!font || bdf_has_xlfd_name(font)) ? FALSE : TRUE;
            gtk_widget_set_sensitive(ed->edit_make_xlfd, flag);
        } else if (w == ed->view_menu) {
            if (fontgrid_get_font_messages(FONTGRID(ed->fgrid)))
              gtk_widget_set_sensitive(ed->view_messages, TRUE);
            else
              gtk_widget_set_sensitive(ed->view_messages, FALSE);
        } else if (w == ed->win_menu) {
            /*
             * Go through and update the file names that might have changed
             * since the last time this menu popped up.
             */
            for (i = 0, kids = gtk_container_get_children(GTK_CONTAINER(w)); kids != 0;
                 kids = kids->next, i++) {
                if (editors[i].file == 0)
                  sprintf(buffer1, "(unnamed%d)", i);
                else
                  strcpy(buffer1, editors[i].file);
                label = gtk_container_get_children(GTK_CONTAINER(kids->data));
                gtk_label_set_text(GTK_LABEL(label->data), buffer1);
            } 

            /*
             * Add any new editors that were created since the last time this
             * menu was shown.
             */
            for (; i < num_editors; i++) {
                if (editors[i].file == 0)
                  sprintf(buffer1, "(unnamed%d)", editors[i].id);
                else
                  strcpy(buffer1, editors[i].file);

                mitem = gtk_menu_item_new_with_label(buffer1);
                (void) g_signal_connect(G_OBJECT(mitem), "activate",
                                        G_CALLBACK(show_editor),
                                        GUINT_TO_POINTER(i));
                gtk_menu_shell_append(GTK_MENU_SHELL(ed->win_menu), mitem);
                gtk_widget_show(mitem);
            }

            /*
             * Disable the item for this editor.
             */
            gtk_widget_set_sensitive(ed->win_menu_item, FALSE);
        }
    } else if (event->type == GDK_UNMAP) {
        /*
         * Enable everything again, so it doesn't get forgotten.
         */
        gtk_widget_set_sensitive(ed->file_save, TRUE);
        gtk_widget_set_sensitive(ed->file_export, TRUE);
        gtk_widget_set_sensitive(ed->edit_paste, TRUE);
        gtk_widget_set_sensitive(ed->edit_overlay, TRUE);
        gtk_widget_set_sensitive(ed->edit_insert, TRUE);
        gtk_widget_set_sensitive(ed->edit_cut, TRUE);
        gtk_widget_set_sensitive(ed->edit_copy, TRUE);
        gtk_widget_set_sensitive(ed->edit_rename_glyphs, TRUE);
        gtk_widget_set_sensitive(ed->edit_adobe_names, TRUE);
        gtk_widget_set_sensitive(ed->edit_unicode_names, TRUE);
        gtk_widget_set_sensitive(ed->edit_test_glyphs, TRUE);
        gtk_widget_set_sensitive(ed->edit_make_xlfd, TRUE);
        gtk_widget_set_sensitive(ed->win_menu_item, TRUE);
        gtk_widget_set_sensitive(ed->view_messages, TRUE);
    }

    return FALSE;
}

#if (GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 10)
static void
open_recent_font(GtkWidget *w, gpointer data)
{
    gchar *p, *uri = gtk_recent_chooser_get_current_uri(GTK_RECENT_CHOOSER(w));
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (uri == NULL)
      return;

    /*
     * Skip the URI prefix to get to the path. The URI's are strictly local
     * at the moment.
     */
    for (p = uri; *p && *p != ':'; p++);
    if (*p != ':')
      return;
    p++;
    if (*p == '/' && *(p + 1) == '/')
      p += 2;
    guifile_load_bdf_font(ed, (const gchar *) p);
    g_free(uri);
}

static GtkWidget *
make_recent_menu(guint ed_id)
{
    GtkWidget *menu;
    GtkRecentFilter *filter;
    GtkRecentManager *manager;

    manager = gtk_recent_manager_get_default();
    menu = gtk_recent_chooser_menu_new_for_manager(manager);
    gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(menu), 10);
    gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(menu),
                                     GTK_RECENT_SORT_MRU);
    gtk_recent_chooser_set_show_tips(GTK_RECENT_CHOOSER(menu), TRUE);
    gtk_recent_chooser_menu_set_show_numbers(GTK_RECENT_CHOOSER_MENU(menu),
                                             TRUE);
    gtk_recent_chooser_set_local_only(GTK_RECENT_CHOOSER(menu), TRUE);
    filter = gtk_recent_filter_new();
    gtk_recent_filter_add_pattern(filter, "*.[Bb][Dd][Ff]");
    gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(menu), filter);

    (void) g_signal_connect(G_OBJECT(menu), "item_activated",
                            G_CALLBACK(open_recent_font),
                            GUINT_TO_POINTER(ed_id));

    return menu;
}
#endif

static GtkWidget *
make_file_menu(gbdfed_editor_t *ed, GtkWidget *menubar)
{
    GtkWidget *file, *menu, *submenu, *mitem, *sep;

    /*
     * Create the File menu.
     */
    file = gtk_menu_item_new_with_mnemonic("_File");

    ed->file_menu = menu = gtk_menu_new();
    (void) g_signal_connect(G_OBJECT(menu), "map_event",
                            G_CALLBACK(menu_popup),
                            GUINT_TO_POINTER(ed->id));

    mitem = make_accel_menu_item(menu, "_New", "<Control>N", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_new_editor), 0);

    mitem = make_accel_menu_item(menu, "_Open", "<Control>O", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_import_bdf_font),
                            GUINT_TO_POINTER(ed->id));

    ed->file_save = make_accel_menu_item(menu, "_Save", "<Control>S", ed->ag);
    (void) g_signal_connect(G_OBJECT(ed->file_save), "activate",
                            G_CALLBACK(guifile_save),
                            GUINT_TO_POINTER(ed->id));
    mitem = make_accel_menu_item(menu, "Save _As...", "<Control>W", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_save_as),
                            GUINT_TO_POINTER(ed->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    /*
     * Create the Import and Export menu items with their submenus.
     */
    mitem = gtk_menu_item_new_with_mnemonic("_Import");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);

    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), submenu);

    mitem = make_accel_menu_item(submenu, "_PK/GF Font", "<Control>K", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_import_pkgf_font),
                            GUINT_TO_POINTER(ed->id));
    mitem = make_accel_menu_item(submenu, "_Console Font", "<Control>L",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_import_console_font),
                            GUINT_TO_POINTER(ed->id));
#ifdef HAVE_HBF
    mitem = make_accel_menu_item(submenu, "_HBF Font", "<Control>H",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_import_hbf_font),
                            GUINT_TO_POINTER(ed->id));
#endif

    mitem = make_accel_menu_item(submenu, "_Windows Font", "<Control>B",
                                 ed->ag);

    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_import_windows_font),
                            GUINT_TO_POINTER(ed->id));
#ifdef HAVE_FREETYPE
    mitem = make_accel_menu_item(submenu, "_OpenType Font", "<Control>Y",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_import_otf_font),
                            GUINT_TO_POINTER(ed->id));
#endif /* HAVE_FREETYPE */

#ifdef HAVE_XLIB
    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), sep);

    mitem = make_accel_menu_item(submenu, "_X Server Font", "<Control>G",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_import_xserver_font),
                            GUINT_TO_POINTER(ed->id));
#endif

    ed->file_export = gtk_menu_item_new_with_mnemonic("Ex_port");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), ed->file_export);

    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ed->file_export), submenu);

    mitem = make_accel_menu_item(submenu, "_PSF Font", "<Control>F",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_export_psf_font),
                            GUINT_TO_POINTER(ed->id));
    mitem = gtk_menu_item_new_with_mnemonic("_HEX");
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guifile_export_hex_font),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

#if (GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 10)
    /*
     * Only add the Recent Fonts menu if the GTK version supports it.
     */
    mitem = gtk_menu_item_new_with_mnemonic("_Recent Fonts");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), make_recent_menu(ed->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
#endif

    if (ed->id == 0) {
        mitem = make_accel_menu_item(menu, "E_xit", "<Control>F4", ed->ag);

        (void) g_signal_connect(G_OBJECT(mitem), "activate",
                                G_CALLBACK(quit_application),
                                GUINT_TO_POINTER(ed->id));
    } else {
        mitem = make_accel_menu_item(menu, "Clos_e", "<Control>F4", ed->ag);

        (void) g_signal_connect_object(G_OBJECT(mitem), "activate",
                                       G_CALLBACK(gtk_widget_hide),
                                       (gpointer) ed->shell,
                                       G_CONNECT_SWAPPED);
    }

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), menu);

    return file;
}

static void
make_xlfd_name(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_make_xlfd_font_name(FONTGRID(ed->fgrid));
}

static void
update_name_from_props(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_update_font_name_from_properties(FONTGRID(ed->fgrid));
}

static void
update_props_from_name(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    fontgrid_update_properties_from_font_name(FONTGRID(ed->fgrid));
}

static GtkWidget *
make_edit_menu(gbdfed_editor_t *ed, GtkWidget *menubar)
{
    GtkWidget *edit, *menu, *submenu, *mitem, *sep;

    /*
     * Create the Edit menu.
     */
    edit = gtk_menu_item_new_with_mnemonic("_Edit");

    ed->edit_menu = menu = gtk_menu_new();
    (void) g_signal_connect(G_OBJECT(menu), "map_event",
                            G_CALLBACK(menu_popup),
                            GUINT_TO_POINTER(ed->id));

    ed->edit_copy = make_accel_menu_item(menu, "_Copy", "<Control>C", ed->ag);
    (void) g_signal_connect(G_OBJECT(ed->edit_copy), "activate",
                            G_CALLBACK(guiedit_copy_selection),
                            GUINT_TO_POINTER(ed->id));

    ed->edit_cut = make_accel_menu_item(menu, "C_ut", "<Control>X", ed->ag);
    (void) g_signal_connect(G_OBJECT(ed->edit_cut), "activate",
                            G_CALLBACK(guiedit_cut_selection),
                            GUINT_TO_POINTER(ed->id));

    ed->edit_paste = make_accel_menu_item(menu, "_Paste", "<Control>V",
                                          ed->ag);
    (void) g_signal_connect(G_OBJECT(ed->edit_paste), "activate",
                            G_CALLBACK(guiedit_paste_selection),
                            GUINT_TO_POINTER(ed->id));

    ed->edit_overlay = make_accel_menu_item(menu, "_Overlay",
                                            "<Shift><Control>V", ed->ag);
    (void) g_signal_connect(G_OBJECT(ed->edit_overlay), "activate",
                            G_CALLBACK(guiedit_overlay_selection),
                            GUINT_TO_POINTER(ed->id));

    ed->edit_insert = make_accel_menu_item(menu, "_Insert",
                                           "<Control><Alt>V", ed->ag);
    (void) g_signal_connect(G_OBJECT(ed->edit_insert), "activate",
                            G_CALLBACK(guiedit_insert_selection),
                            GUINT_TO_POINTER(ed->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = make_accel_menu_item(menu, "P_roperties", "<Control>P", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiedit_show_font_properties),
                            GUINT_TO_POINTER(ed->id));

    mitem = make_accel_menu_item(menu, "Co_mments", "<Control>M", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiedit_show_font_comments),
                            GUINT_TO_POINTER(ed->id));

    mitem = make_accel_menu_item(menu, "_Font Info", "<Control>I", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                              G_CALLBACK(guiedit_show_font_info),
                              GUINT_TO_POINTER(ed->id));
    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    /*
     * Create the Font Name submenu.
     */
    mitem = gtk_menu_item_new_with_mnemonic("Font _Name");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);

    ed->name_submenu = submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), submenu);
    (void) g_signal_connect(G_OBJECT(submenu), "map_event",
                            G_CALLBACK(menu_popup),
                            GUINT_TO_POINTER(ed->id));

    ed->edit_make_xlfd = gtk_menu_item_new_with_mnemonic("Make _XLFD Name");
    (void) g_signal_connect(G_OBJECT(ed->edit_make_xlfd), "activate",
                            G_CALLBACK(make_xlfd_name),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), ed->edit_make_xlfd);

    mitem = gtk_menu_item_new_with_mnemonic("Update _Name From Properties");
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(update_name_from_props),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);

    mitem = gtk_menu_item_new_with_mnemonic("Update _Properties From Name");
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(update_props_from_name),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);
    mitem = gtk_menu_item_new_with_mnemonic("Update _Average Width");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    /*
     * Make the glyph naming menus and submenus.
     */
    ed->edit_rename_glyphs = gtk_menu_item_new_with_mnemonic("Ren_ame Glyphs");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), ed->edit_rename_glyphs);

    ed->edit_rename_menu = submenu = gtk_menu_new();
    (void) g_signal_connect(G_OBJECT(submenu), "map_event",
                            G_CALLBACK(menu_popup),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ed->edit_rename_glyphs), submenu);

    ed->edit_unicode_names = gtk_menu_item_new_with_mnemonic("_Unicode Names");
    (void) g_signal_connect(G_OBJECT(ed->edit_unicode_names), "activate",
                            G_CALLBACK(guiedit_set_unicode_glyph_names),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), ed->edit_unicode_names);

    ed->edit_adobe_names = gtk_menu_item_new_with_mnemonic("_Adobe Names");
    (void) g_signal_connect(G_OBJECT(ed->edit_adobe_names), "activate",
                            G_CALLBACK(guiedit_set_adobe_glyph_names),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), ed->edit_adobe_names);

    mitem = gtk_menu_item_new_with_mnemonic("_Hexadecimal Values");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), submenu);

    mitem = gtk_menu_item_new_with_mnemonic("_uniXXXX");
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiedit_set_uni_glyph_names),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);

    mitem = gtk_menu_item_new_with_mnemonic("0_xXXXX");
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiedit_set_zerox_glyph_names),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);

    mitem = gtk_menu_item_new_with_mnemonic("U_+XXXX");
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiedit_set_uplus_glyph_names),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);

    mitem = gtk_menu_item_new_with_mnemonic("_\\uXXXX");
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiedit_set_bslashu_glyph_names),
                            GUINT_TO_POINTER(ed->id));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);

    ed->edit_test_glyphs = make_accel_menu_item(menu, "_Test Glyphs",
                                                "<Control>Z", ed->ag);
    (void) g_signal_connect(G_OBJECT(ed->edit_test_glyphs), "activate",
                            G_CALLBACK(guiedit_show_glyphtest),
                            GUINT_TO_POINTER(ed->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = make_accel_menu_item(menu, "Pr_eferences", "<Control>T", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiedit_show_preferences), 0);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), menu);

    return edit;
}

static GtkWidget *
make_view_menu(gbdfed_editor_t *ed, GtkWidget *menubar)
{
    GtkWidget *view, *menu, *submenu, *mitem, *sep;
    GSList *group;
    GtkRadioMenuItem *ri;

    /*
     * Create the View menu.
     */
    view = gtk_menu_item_new_with_mnemonic("_View");

    ed->view_menu = menu = gtk_menu_new();
    (void) g_signal_connect(G_OBJECT(menu), "map_event",
                            G_CALLBACK(menu_popup),
                            GUINT_TO_POINTER(ed->id));

    ed->view_unencoded = mitem =
        make_accel_menu_item(menu, "Unencoded", "<Control>E", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(toggle_encoding_view),
                            GUINT_TO_POINTER(ed->id));

    /*
     * Create the Code Base submenu.
     */
    mitem = gtk_menu_item_new_with_mnemonic("_Code Base");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);

    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), submenu);

    ed->view_oct = mitem = gtk_radio_menu_item_new_with_mnemonic(0, "_Octal");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);
    ri = GTK_RADIO_MENU_ITEM(mitem);
    if (options.code_base == 8)
      gtk_check_menu_item_set_active(&ri->check_menu_item, TRUE);

    group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mitem));
    ed->view_dec = mitem =
        gtk_radio_menu_item_new_with_mnemonic(group, "_Decimal");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);
    ri = GTK_RADIO_MENU_ITEM(mitem);
    if (options.code_base == 10)
      gtk_check_menu_item_set_active(&ri->check_menu_item, TRUE);

    group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mitem));
    ed->view_hex = mitem =
        gtk_radio_menu_item_new_with_mnemonic(group, "_Hexadecimal");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), mitem);
    ri = GTK_RADIO_MENU_ITEM(mitem);
    if (options.code_base == 16)
      gtk_check_menu_item_set_active(&ri->check_menu_item, TRUE);

    /*
     * Add the code base toggle handler to the three toggles.
     */
    (void) g_signal_connect(G_OBJECT(ed->view_oct), "toggled",
                            G_CALLBACK(set_code_base),
                            GUINT_TO_POINTER(ed->id));
    (void) g_signal_connect(G_OBJECT(ed->view_dec), "toggled",
                            G_CALLBACK(set_code_base),
                            GUINT_TO_POINTER(ed->id));
    (void) g_signal_connect(G_OBJECT(ed->view_hex), "toggled",
                            G_CALLBACK(set_code_base),
                            GUINT_TO_POINTER(ed->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = make_accel_menu_item(menu, "_Other Page", "<Shift><Control>S",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(goto_other_page),
                            GUINT_TO_POINTER(ed->id));

    if (options.orientation == GTK_ORIENTATION_VERTICAL)
      ed->view_orientation = mitem =
          make_accel_menu_item(menu, "Horizontal View", "<Control>Q", ed->ag);
    else
      ed->view_orientation = mitem =
          make_accel_menu_item(menu, "Vertical View", "<Control>Q", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(toggle_view_orientation),
                            GUINT_TO_POINTER(ed->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    /*
     * Messages dialog.
     */
    ed->view_messages = mitem =
        make_accel_menu_item(menu, "_Messages", "<Control>A", ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(view_font_messages),
                            GUINT_TO_POINTER(ed->id));

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), menu);

    return view;
}

static GtkWidget *
make_ops_menu(gbdfed_editor_t *ed, GtkWidget *menubar)
{
    GtkWidget *ops, *menu, *mitem;

    /*
     * Create the Edit menu.
     */
    ops = gtk_menu_item_new_with_mnemonic("_Operations");

    ed->ops_menu = menu = gtk_menu_new();
    (void) g_signal_connect(G_OBJECT(menu), "map_event",
                            G_CALLBACK(menu_popup),
                            GUINT_TO_POINTER(ed->id));

    mitem = make_accel_menu_item(menu, "_Translate Glyphs", "<Control>D",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiops_show_translate),
                            GUINT_TO_POINTER(ed->id));

    mitem = make_accel_menu_item(menu, "_Rotate Glyphs", "<Control>R",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiops_show_rotate),
                            GUINT_TO_POINTER(ed->id));

    mitem = make_accel_menu_item(menu, "_Shear Glyphs", "<Control>J",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiops_show_shear),
                            GUINT_TO_POINTER(ed->id));

    mitem = make_accel_menu_item(menu, "_Embolden Glyphs", "<Shift><Control>B",
                                 ed->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guiops_show_embolden),
                            GUINT_TO_POINTER(ed->id));

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ops), menu);

    return ops;
}

static GtkWidget *
make_windows_menu(gbdfed_editor_t *ed, GtkWidget *menubar)
{
    GtkWidget *win, *menu, *mitem;
    guint i;

    win = gtk_menu_item_new_with_mnemonic("_Windows");

    ed->win_menu = menu = gtk_menu_new();
    (void) g_signal_connect(G_OBJECT(menu), "map_event",
                            G_CALLBACK(menu_popup),
                            GUINT_TO_POINTER(ed->id));

    for (i = 0; i < num_editors; i++) {
        if (editors[i].file == 0)
          sprintf(buffer1, "(unnamed%d)", editors[i].id);
        else
          strcpy(buffer1, editors[i].file);

        ed->win_menu_item = mitem = gtk_menu_item_new_with_label(buffer1);
        (void) g_signal_connect(G_OBJECT(mitem), "activate",
                                G_CALLBACK(show_editor),
                                GUINT_TO_POINTER(ed->id));
        gtk_menu_shell_append(GTK_MENU_SHELL(ed->win_menu), mitem);
    }

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(win), menu);

    return win;
}

static GtkWidget *
make_help_menu(gbdfed_editor_t *ed, GtkWidget *menubar)
{
    GtkWidget *help, *menu, *mitem, *sep;

    /*
     * Create the Edit menu.
     */
    help = gtk_menu_item_new_with_mnemonic("_Help");
    gtk_menu_item_set_right_justified(GTK_MENU_ITEM(help), TRUE);

    menu = gtk_menu_new();

    mitem = gtk_menu_item_new_with_mnemonic("The _Program");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_PROGRAM));

    mitem = gtk_menu_item_new_with_mnemonic("_Font Grid");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_FONTGRID));

    mitem = gtk_menu_item_new_with_mnemonic("_Glyph Editor");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_GLYPH_EDITOR));

    mitem = gtk_menu_item_new_with_mnemonic("_Configuration File");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_CONFIG_FILE));

    mitem = gtk_menu_item_new_with_mnemonic("P_references Dialog");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_PREFERENCES));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = gtk_menu_item_new_with_mnemonic("_Windows Font Notes");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_FNT));

    mitem = gtk_menu_item_new_with_mnemonic("_OpenType Font Notes");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_OTF));

    mitem = gtk_menu_item_new_with_mnemonic("P_SF Font Notes");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_PSF));

    mitem = gtk_menu_item_new_with_mnemonic("_HEX Font Notes");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_HEX));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = gtk_menu_item_new_with_mnemonic("C_olor Notes");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_COLOR));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = gtk_menu_item_new_with_mnemonic("T_ips");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_TIPS));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    sprintf(buffer1, "_About %s", g_get_prgname());
    mitem = gtk_menu_item_new_with_mnemonic(buffer1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(guihelp_show_help),
                            GINT_TO_POINTER(GUIHELP_ABOUT));

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), menu);

    return help;
}

#define UPDMSG "Font Name: XLFD name has been modified.\nDo you want to update the font properties from the name?"

static void
update_font_name(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    bdf_font_t *font;
    gchar *name;

    name = (gchar *) gtk_entry_get_text(GTK_ENTRY(ed->fontname));

    font = fontgrid_get_font(FONTGRID(ed->fgrid));
    if (font && strcmp(name, font->name) == 0)
      /*
       * There was no actual change to the name.
       */
      return;

    fontgrid_set_font_name(FONTGRID(ed->fgrid), name);

    /*
     * If the new name is an XLFD name, then offer to update the
     * font properties from the name.
     */
    if (font && bdf_has_xlfd_name(font)) {
        if (guiutil_yes_or_no(ed->shell, UPDMSG, TRUE)) {
            fontgrid_update_properties_from_font_name(FONTGRID(ed->fgrid));

            /*
             * If the font info dialog is up, make sure it is updated with
             * the font property changes.
             */
            guiedit_update_font_properties(ed);
        }
    }
}

guint
gbdfed_make_editor(gchar *filename, gboolean cmdline)
{
    FILE *in;
    GtkWidget *mb, *mitem, *table;
    GtkWidget *vbox, *hbox, *bbox, *frame;
    gbdfed_editor_t *ed;
    bdf_font_t *font;
    gchar *path;
    GList *icon_list = 0;
    FontgridPageInfo pageinfo;

    font = 0;

    /*
     * Attempt to load the specified font before doing anything else.
     */
    if (filename != 0) {
        /*
         * Change code to put up an error dialog if the font won't load.
         */
        if ((in = fopen(filename, "r")) == 0) {
            fprintf(stderr, "%s: unable to open BDF font '%s'.\n",
                    g_get_prgname(), filename);
            if (num_editors > 0 && !cmdline)
              return ~0;
            filename = 0;
        } else {
            font = bdf_load_font(in, &options.font_opts, 0, 0);
            fclose(in);
            if (font == 0)
              fprintf(stderr, "%s: problem loading BDF font '%s'.\n",
                      g_get_prgname(), filename);
        }
    }

    /*
     * Create the icon list if it hasn't already been created.
     */
    if (icons_set == FALSE) {
        /*
         * Create the 16x16 icon.
         */
        icon_list = g_list_append(icon_list,
                                  gdk_pixbuf_new_from_xpm_data(gbdfed_48x48));
        icon_list = g_list_append(icon_list,
                                  gdk_pixbuf_new_from_xpm_data(gbdfed_32x32));
        icon_list = g_list_append(icon_list,
                                  gdk_pixbuf_new_from_xpm_data(gbdfed_16x16));

        gtk_window_set_default_icon_list(icon_list);

        icons_set = TRUE;
    }

    /*
     * Create a new editor structure.
     */
    if (num_editors == 0)
      editors = g_malloc(sizeof(gbdfed_editor_t));
    else
      editors = g_realloc(editors,
                          sizeof(gbdfed_editor_t) * (num_editors + 1));

    ed = editors + num_editors;
    (void) memset(ed, 0, sizeof(gbdfed_editor_t));
    ed->id = num_editors++;

    /*
     * Construct the path and filename from the one passed.  This makes copies
     * of the strings.
     */
    if (font != 0 && filename != 0) {
        if (filename[0] != G_DIR_SEPARATOR) {
            path = g_get_current_dir();
            sprintf(buffer1, "%s%c%s", path, G_DIR_SEPARATOR, filename);
            g_free(path);
        } else
          strcpy(buffer1, filename);

        ed->file = g_path_get_basename(buffer1);
        ed->path = g_path_get_dirname(buffer1);
    }

    /*
     * Create the top level window for the editor.
     */
    ed->shell = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /*
     * This is a bit risky, but it allows the shell to shrink to fit new
     * fonts when they are added.
     */
    gtk_window_set_resizable(GTK_WINDOW(ed->shell), TRUE);

    if (ed->id == 0) {
        (void) g_signal_connect(G_OBJECT(ed->shell), "destroy_event",
                                G_CALLBACK(quit_application),
                                GUINT_TO_POINTER(ed->id));
        (void) g_signal_connect(G_OBJECT(ed->shell), "delete_event",
                                G_CALLBACK(quit_application),
                                GUINT_TO_POINTER(ed->id));
    } else {
        (void) g_signal_connect(G_OBJECT(ed->shell), "destroy_event",
                                G_CALLBACK(gtk_widget_hide), 0);
        (void) g_signal_connect(G_OBJECT(ed->shell), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);
    }

    /*
     * Determine the name for the window.
     */
    if (filename == 0)
      sprintf(buffer1, "%s - (unnamed%d)", g_get_prgname(), ed->id);
    else {
        if (font && font->modified)
          sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ed->file);
        else
          sprintf(buffer1, "%s - %s", g_get_prgname(), ed->file);
    }
    gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

    /*
     * Create the vertical box that will contain the widgets.
     */
    table = gtk_table_new(5, 1, FALSE);
    gtk_container_add(GTK_CONTAINER(ed->shell), table);

    ed->ag = gtk_accel_group_new();

    mb = gtk_menu_bar_new();
    gtk_table_attach(GTK_TABLE(table), mb, 0, 1, 0, 1, GTK_FILL, GTK_FILL,
                     0, 0);

    /*
     * Create the File menu.
     */
    mitem = make_file_menu(ed, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    /*
     * Create the Edit menu.
     */
    mitem = make_edit_menu(ed, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    /*
     * Create the View menu.
     */
    mitem = make_view_menu(ed, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    /*
     * Create the Operations menu.
     */
    mitem = make_ops_menu(ed, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    /*
     * Create the Windows menu.
     */
    mitem = make_windows_menu(ed, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    /*
     * Create the Help menu.
     */
    mitem = make_help_menu(ed, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    /*
     * Attach the accelerators to the editor.
     */
    gtk_window_add_accel_group(GTK_WINDOW(ed->shell), ed->ag);

    /*
     * 1. Add the font name widget.
     */
    frame = gtk_frame_new(0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 1, 2, GTK_FILL, GTK_FILL,
                     0, 0);

    ed->fontname = gtk_widget_new(gtk_entry_get_type(),
                                  "max_length", 128, NULL);
    mitem = labcon_new_label_defaults("Font:", ed->fontname, 0);
    gtk_container_add(GTK_CONTAINER(frame), mitem);
    g_signal_connect(G_OBJECT(ed->fontname), "activate",
                     G_CALLBACK(update_font_name), GUINT_TO_POINTER(ed->id));

    /*
     * 2. Add the glyph information widgets.
     */
    frame = gtk_frame_new(0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 2, 3, GTK_FILL, GTK_FILL,
                     0, 0);

    vbox = gtk_vbox_new(TRUE, 0);
    ed->charinfo = gtk_label_new("\"None\" (0000) (00,00)");
    ed->metrics = gtk_label_new("ascent 0 descent 0 right 0 left 0");
    gtk_misc_set_alignment(GTK_MISC(ed->charinfo), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(ed->metrics), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), ed->charinfo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), ed->metrics, TRUE, TRUE, 0);

    mitem = labcon_new_label_defaults("Glyph:", vbox, mitem);
    gtk_container_add(GTK_CONTAINER(frame), mitem);

    frame = gtk_frame_new(0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 3, 4, GTK_FILL, GTK_FILL,
                     0, 0);

    hbox = gtk_hbox_new(FALSE, 2);

    bbox = gtk_hbox_new(FALSE, 0);

    ed->first = gtk_button_new_with_label("First Page");
    (void) g_signal_connect(G_OBJECT(ed->first), "released",
                            G_CALLBACK(first_page),
                            GUINT_TO_POINTER(ed->id));

    ed->prev = gtk_button_new_with_label("Previous Page");
    (void) g_signal_connect(G_OBJECT(ed->prev), "released",
                            G_CALLBACK(previous_page),
                            GUINT_TO_POINTER(ed->id));

    ed->next = gtk_button_new_with_label("Next Page");
    (void) g_signal_connect(G_OBJECT(ed->next), "released",
                            G_CALLBACK(next_page),
                            GUINT_TO_POINTER(ed->id));

    ed->last = gtk_button_new_with_label("Last Page");
    (void) g_signal_connect(G_OBJECT(ed->last), "released",
                            G_CALLBACK(last_page),
                            GUINT_TO_POINTER(ed->id));

    gtk_box_pack_start(GTK_BOX(bbox), ed->first, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bbox), ed->prev, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bbox), ed->next, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bbox), ed->last, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), bbox, FALSE, FALSE, 0);

    mitem = gtk_label_new("Page:");
    ed->pageno = gtk_widget_new(gtk_entry_get_type(),
                                "max_length", 6,
                                "width-request", 70,
                                NULL);
    (void) g_signal_connect(G_OBJECT(ed->pageno), "activate",
                            G_CALLBACK(goto_page_or_code),
                            GUINT_TO_POINTER(ed->id));

    gtk_box_pack_start(GTK_BOX(hbox), mitem, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ed->pageno, FALSE, FALSE, 0);

    mitem = gtk_label_new("Code:");
    ed->charno = gtk_widget_new(gtk_entry_get_type(),
                                "max_length", 6,
                                "width-request", 70,
                                NULL);
    (void) g_signal_connect(G_OBJECT(ed->charno), "activate",
                            G_CALLBACK(goto_page_or_code),
                            GUINT_TO_POINTER(ed->id));

    gtk_box_pack_start(GTK_BOX(hbox), mitem, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ed->charno, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(frame), hbox);

    /*
     * Now add the font grid itself.
     */
    ed->fgrid = fontgrid_newv(font,
                              options.font_opts.point_size,
                              options.font_opts.font_spacing,
                              options.no_blanks,
                              options.overwrite_mode,
                              options.power2,
                              options.colors,
                              options.initial_glyph,
                              options.code_base,
                              options.orientation,
                              options.font_opts.bits_per_pixel,
                              options.font_opts.resolution_x,
                              options.font_opts.resolution_y,
                              &pageinfo);
    (void) g_signal_connect(G_OBJECT(ed->fgrid), "turn_to_page",
                            G_CALLBACK(page_change),
                            GUINT_TO_POINTER(ed->id));
    (void) g_signal_connect(G_OBJECT(ed->fgrid), "selection-start",
                            G_CALLBACK(update_selection_info),
                            GUINT_TO_POINTER(ed->id));
    (void) g_signal_connect(G_OBJECT(ed->fgrid), "selection-extend",
                            G_CALLBACK(update_selection_info),
                            GUINT_TO_POINTER(ed->id));
    (void) g_signal_connect(G_OBJECT(ed->fgrid), "selection-end",
                            G_CALLBACK(update_selection_info),
                            GUINT_TO_POINTER(ed->id));
    (void) g_signal_connect(G_OBJECT(ed->fgrid), "activate",
                            G_CALLBACK(update_selection_info),
                            GUINT_TO_POINTER(ed->id));
    (void) g_signal_connect(G_OBJECT(ed->fgrid), "modified",
                            G_CALLBACK(handle_modified_signal),
                            GUINT_TO_POINTER(ed->id));
    gtk_table_attach(GTK_TABLE(table), ed->fgrid, 0, 1, 4, 5,
                     GTK_FILL|GTK_EXPAND|GTK_SHRINK,
                     GTK_FILL|GTK_EXPAND|GTK_SHRINK, 0, 0);

    /*
     * Set up the initial page information.
     */
    page_change(ed->fgrid, &pageinfo, GUINT_TO_POINTER(ed->id));

    /*
     * Show the editor if it the first one created or is not being created
     * from the command line.
     */
    if (ed->id == 0)
      gtk_widget_show_all(ed->shell);

    {
        /*
         * Get the initial selection info to set up the glyph info labels.
         *
         * NOTE: This has to be done here because for some reason labels
         * do not display properly if changed before the editor is fully
         * realized.
         */
        FontgridSelectionInfo sinfo;
        if (fontgrid_has_selection(FONTGRID(ed->fgrid), &sinfo) == TRUE)
          update_selection_info(ed->fgrid, (gpointer) &sinfo,
                                GUINT_TO_POINTER(ed->id));
    }

    /*
     * Set the font name in the entry field if the font exists.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->fontname),
                       fontgrid_get_font_name(FONTGRID(ed->fgrid)));

    /*
     * Change the focus to the Fontgrid.
     */
    gtk_widget_grab_focus(ed->fgrid);

    return ed->id;
}

static int
handle_unknown_options(bdf_options_t *opts, char **params,
                       unsigned int nparams, void *client_data)
{
    gint idx;
    gbdfed_options_t *op;

    op = (gbdfed_options_t *) client_data;

    if (nparams == 0)
      return 0;

    /*
     * Handle the 2 bits per pixel color list.
     */
    if (strncmp(params[0], "2bpp_grays", 10) == 0) {
        for (idx = 1; idx < 5; idx++)
          op->colors[idx] = (unsigned short) _bdf_atos(params[idx], 0, 10);
        return 1;
    }

    /*
     * Handle the 4 bits per pixel color list.
     */
    if (strncmp(params[0], "4bpp_grays", 10) == 0) {
        for (idx = 1; idx < 17; idx++)
          op->colors[idx-1+4] = (unsigned short) _bdf_atos(params[idx], 0, 10);
        return 1;
    }

    if (params[0][0] == 'a' &&
        strncmp(params[0], "adobe_name_file", 15) == 0) {
        if (op->adobe_name_file != 0)
          g_free(op->adobe_name_file);
        if (params[1] == 0 || params[1][0] == 0)
          op->adobe_name_file = 0;
        else
          op->adobe_name_file = g_strdup(params[1]);
        return 1;
    }

    if (params[0][0] == 'c') {
        if (strncmp(params[0], "close_accelerator", 17) == 0) {
            if (params[0][17] == 0) {
                /*
                 * We have the accelerator itself.  Add code to convert Xt key
                 * bindings to GTK.
                 */
                if (op->accelerator != 0)
                  g_free(op->accelerator);
                if (params[1] == 0 || params[1][0] == 0)
                  op->accelerator = 0;
                else
                  op->accelerator = g_strdup(params[1]);
            }

            /*
             * Ignore instances of 'close_accelerator_text'.  GTK+ figures this
             * out already.
             */

            return 1;
        }

        if (strncmp(params[0], "code_base", 9) == 0) {
            switch (params[1][0]) {
              case 'o': case 'O': op->code_base = 8; break;
              case 'd': case 'D': op->code_base = 10; break;
              case 'h': case 'H': op->code_base = 16; break;
              default: op->code_base = 16;
            }
            return 1;
        }

        /*
         * Ignore the old grayscale specification keywords.
         */
        if (strncmp(params[0], "color", 5) == 0) {
            /*
             * Quietly ignore the old color list.
             */
            return 1;
        }
    }

    if (params[0][0] == 'f' &&
        strncmp(params[0], "font_grid_horizontal", 20) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->orientation = GTK_ORIENTATION_VERTICAL;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->orientation = GTK_ORIENTATION_HORIZONTAL;
            break;
        }
        return 1;
    }        

    if (params[0][0] == 'g') {
        if (strncmp(params[0], "grid_overwrite_mode", 19) == 0) {
            switch (params[1][0]) {
              case '0': case 'F': case 'f': case 'N': case 'n':
                op->overwrite_mode = FALSE;
                break;
              case '1': case 'T': case 't': case 'Y': case 'y':
                op->overwrite_mode = TRUE;
                break;
            }
            return 1;
        }

        if (strncmp(params[0], "generate_sbit_metrics", 21) == 0) {
            switch (params[1][0]) {
              case '0': case 'F': case 'f': case 'N': case 'n':
                op->sbit = FALSE;
                break;
              case '1': case 'T': case 't': case 'Y': case 'y':
                op->sbit = TRUE;
                break;
            }
            return 1;
        }
    }

    if (params[0][0] == 'm' && strncmp(params[0], "make_backups", 12) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->backups = FALSE;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->backups = TRUE;
            break;
        }
        return 1;
    }

    if ((params[0][0] == 'n' && strncmp(params[0], "name_file", 9) == 0) ||
        (params[0][0] == 'u' &&
         strncmp(params[0], "unicode_name_file", 17) == 0)) {
        if (op->unicode_name_file != 0)
          g_free(op->unicode_name_file);
        if (params[1] == 0 || params[1][0] == 0)
          op->unicode_name_file = 0;
        else
          op->unicode_name_file = g_strdup(params[1]);
        return 1;
    }

    if (params[0][0] == 'o' && strncmp(params[0], "orientation", 11) == 0) {
        switch (params[1][0]) {
          case 'H': case 'h':
            op->orientation = GTK_ORIENTATION_HORIZONTAL;
            break;
          case 'V': case 'v':
            op->orientation = GTK_ORIENTATION_VERTICAL;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'p') {
        if (strncmp(params[0], "pixel_size", 10) == 0) {
            op->pixel_size = _bdf_atoul(params[1], 0, 10);
            if (op->pixel_size < 2)
              op->pixel_size = 2;
            else if (op->pixel_size > 20)
              op->pixel_size = 20;
            return 1;
        }

        if (strncmp(params[0], "power2", 6) == 0) {
            switch (params[1][0]) {
              case '0': case 'F': case 'f': case 'N': case 'n':
                op->power2 = FALSE;
                break;
              case '1': case 'T': case 't': case 'Y': case 'y':
                op->power2 = TRUE;
                break;
            }
            return 1;
        }

        if (strncmp(params[0], "percentage_only", 15) == 0)
          /*
           * Ignore this option.  No longer needed.
           */
          return 1;

        if (strncmp(params[0], "progress_bar", 12) == 0)
          /*
           * The progress bar is no longer being used as of version 5.0.
           */
          return 1;
    }

    if (params[0][0] == 'r') {
        if (strncmp(params[0], "resolution", 10) == 0) {
            op->resolution = _bdf_atoul(params[1], 0, 10);
            return 1;
        }

        if (strncmp(params[0], "really_exit", 11) == 0) {
            switch (params[1][0]) {
              case '0': case 'F': case 'f': case 'N': case 'n':
                op->really_exit = FALSE;
                break;
              case '1': case 'T': case 't': case 'Y': case 'y':
                op->really_exit = TRUE;
                break;
            }
            return 1;
        }
    }

    if (params[0][0] == 's') {
        if (strncmp(params[0], "skip_blank_pages", 16) == 0) {
            switch (params[1][0]) {
              case '0': case 'F': case 'f': case 'N': case 'n':
                op->no_blanks = FALSE;
                break;
              case '1': case 'T': case 't': case 'Y': case 'y':
                op->no_blanks = TRUE;
                break;
            }
            return 1;
        }

        if (strncmp(params[0], "show_cap_height", 15) == 0) {
            switch (params[1][0]) {
              case '0': case 'F': case 'f': case 'N': case 'n':
                op->show_cap_height = FALSE;
                break;
              case '1': case 'T': case 't': case 'Y': case 'y':
                op->show_cap_height = TRUE;
                break;
            }
            return 1;
        }

        if (strncmp(params[0], "show_x_height", 13) == 0) {
            switch (params[1][0]) {
              case '0': case 'F': case 'f': case 'N': case 'n':
                op->show_x_height = FALSE;
                break;
              case '1': case 'T': case 't': case 'Y': case 'y':
                op->show_x_height = TRUE;
                break;
            }
            return 1;
        }
    }

    return 0;
}

static void
usage(void)
{
    fprintf(stderr, "Usage: %s [font1.bdf .....]\n", g_get_prgname());
    exit(1);
}

static void
editor_setup(int argc, char *argv[])
{
    int base = 0;
    gchar *ap;
    FILE *in;
    gboolean need_editor;
    GdkScreen *screen;

    /*
     * Get the default BDF options.
     */
    bdf_default_options(&options.font_opts);

    options.accelerator = options.accelerator_text =
        options.unicode_name_file = options.adobe_name_file = 0;
    options.pixel_size = 10;
    options.resolution = 0;
    options.no_blanks = options.really_exit = 
        options.overwrite_mode = options.power2 =
        options.backups = TRUE;
    options.show_cap_height = options.show_x_height =
        options.sbit = options.gbdfed_opts = FALSE;
    options.initial_glyph = -1;
    options.code_base = 16;

    options.orientation = GTK_ORIENTATION_HORIZONTAL;

    /*
     * Set the default colors for 2 bits per pixel.
     */
    options.colors[0] = 0;
    options.colors[1] = 175;
    options.colors[2] = 207;
    options.colors[3] = 239;

    /*
     * Set the default colors for 4 bits per pixel.
     */
    options.colors[4] = 0;
    options.colors[5] = 31;
    options.colors[6] = 63;
    options.colors[7] = 95;
    options.colors[8] = 127;
    options.colors[9] = 159;
    options.colors[10] = 167;
    options.colors[11] = 175;
    options.colors[12] = 183;
    options.colors[13] = 191;
    options.colors[14] = 199;
    options.colors[15] = 207;
    options.colors[16] = 215;
    options.colors[17] = 223;
    options.colors[18] = 231;
    options.colors[19] = 239;

    /*
     * Attempt to load the user config file.
     */
    if ((ap = getenv("HOME")) != 0) {
        sprintf(buffer1, "%s/.gbdfedrc", ap);
        if ((in = fopen(buffer1, "r")) != 0) {
            bdf_load_options(in, &options.font_opts, handle_unknown_options,
                             (void *) &options);
            fclose(in);
        } else {
            /*
             * Try reading the older ".xmbdfedrc".
             */
            sprintf(buffer1, "%s/.xmbdfedrc", ap);
            if ((in = fopen(buffer1, "r")) != 0) {
                bdf_load_options(in, &options.font_opts,
                                 handle_unknown_options, (void *) &options);
                fclose(in);
            }
        }
    }

    /*
     * Determine the horizontal and vertical resolutions if they have not been
     * set on the command line or from the config file.
     */
    if (options.resolution != 0)
      options.font_opts.resolution_x = options.font_opts.resolution_y =
          options.resolution;

    screen = gdk_drawable_get_screen(GDK_DRAWABLE(gdk_get_default_root_window()));
    if (options.font_opts.resolution_x == 0)
      options.font_opts.resolution_x =
          (unsigned int) ((((double) gdk_screen_get_width(screen)) * 25.4) /
                           ((double) gdk_screen_get_width_mm(screen)) + 0.5);

    if (options.font_opts.resolution_y == 0)
      options.font_opts.resolution_y =
          (unsigned int) ((((double) gdk_screen_get_height(screen)) * 25.4) /
                           ((double) gdk_screen_get_height_mm(screen)) + 0.5);

    /*
     * Handle the case of no command line options here.
     */
    if (argc == 0) {
        (void) gbdfed_make_editor(0, TRUE);
        return;
    }

    /*
     * If a filename is not present on the command line, make sure an
     * editor is created after the command line is parsed.
     */
    need_editor = TRUE;

    /*
     * Now parse the options from the command line.
     */
    while (argc > 0) {
        ap = *argv;
        if (*ap == '-') {
            if (strcmp(ap + 1, "nm") == 0)
              options.font_opts.correct_metrics = 0;
            else if (strcmp(ap + 1, "nu") == 0)
              options.font_opts.keep_unencoded = 0;
            else if (strcmp(ap + 1, "nc") == 0)
              options.font_opts.keep_comments = 0;
            else if (strcmp(ap + 1, "np") == 0)
              options.font_opts.pad_cells = 0;
            else if (strcmp(ap + 1, "bp") == 0)
              options.no_blanks = FALSE;
            else if (strcmp(ap + 1, "ed") == 0)
              options.really_exit = FALSE;
            else if (strcmp(ap + 1, "im") == 0)
              options.overwrite_mode = FALSE;
            else if (strcmp(ap + 1, "o") == 0) {
                argc--;
                argv++;
                options.orientation =
                    (argv[0][0] == 'v' || argv[0][0] == 'V') ?
                    GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
            } else if (strcmp(ap + 1, "ps") == 0) {
                argc--;
                argv++;
                options.font_opts.point_size = _bdf_atoul(*argv, 0, 10);
            } else if (strcmp(ap + 1, "res") == 0) {
                argc--;
                argv++;
                options.resolution = _bdf_atoul(*argv, 0, 10);
                options.font_opts.resolution_x =
                    options.font_opts.resolution_y = options.resolution;
            } else if (strcmp(ap + 1, "hres") == 0) {
                argc--;
                argv++;
                options.font_opts.resolution_x = _bdf_atoul(*argv, 0, 10);
            } else if (strcmp(ap + 1, "vres") == 0) {
                argc--;
                argv++;
                options.font_opts.resolution_y = _bdf_atoul(*argv, 0, 10);
            } else if (strcmp(ap + 1, "bpp") == 0) {
                argc--;
                argv++;
                options.font_opts.bits_per_pixel = _bdf_atoul(*argv, 0, 10);
            } else if (strcmp(ap + 1, "g") == 0) {
                argc--;
                argv++;
                ap = *argv;
                if (*ap == '\\') {
                    switch (*(ap + 1)) {
                      case 'u': case 'x': base = 16; ap += 2; break;
                      case 'o': base = 8; ap += 2; break;
                      case 'd': base = 10; ap += 2; break;
                    }
                } else if ((*ap == 'U' && (*(ap + 1) == '+' ||
                                           *(ap + 1) == '-')) ||
                           (*ap == '0' && (*(ap + 1) == 'x' ||
                                           *(ap + 1) == 'X'))) {
                    base = 16;
                    ap += 2;
                } else if (*ap == '0')
                  base = 8;

                options.initial_glyph = _bdf_atol(ap, 0, base);
            } else if (strcmp(ap + 1, "sp") == 0) {
                argc--;
                argv++;
                switch (argv[0][0]) {
                  case 'c': case 'C':
                    options.font_opts.font_spacing = BDF_CHARCELL; break;
                  case 'm': case 'M':
                    options.font_opts.font_spacing = BDF_MONOWIDTH; break;
                  case 'p': case 'P':
                    options.font_opts.font_spacing = BDF_PROPORTIONAL; break;
                }
            } else if (strcmp(ap + 1, "eol") == 0) {
                argc--;
                argv++;
                switch (argv[0][0]) {
                  case 'd': case 'D':
                    options.font_opts.eol = BDF_DOS_EOL; break;
                  case 'm': case 'M':
                    options.font_opts.eol = BDF_MAC_EOL; break;
                  case 'u': case 'U':
                    options.font_opts.eol = BDF_UNIX_EOL; break;
                }
            } else if (strcmp(ap + 1, "cb") == 0) {
                argc--;
                argv++;
                switch (argv[0][0]) {
                  case 'd': case 'D':
                    options.code_base = 10; break;
                  case 'h': case 'H':
                    options.code_base = 16; break;
                  case 'o': case 'O':
                    options.code_base = 8; break;
                }
            } else {
                if (ap[1] != 'h')
                  fprintf(stderr, "%s: unknown parameter '%s'.\n",
                          g_get_prgname(), ap);
                usage();
            }
        } else {
            need_editor = FALSE;
            (void) gbdfed_make_editor(argv[0], TRUE);
        }

        argc--;
        argv++;
    }

    if (need_editor == TRUE)
      /*
       * If an editor was not created, make one here.
       */
      (void) gbdfed_make_editor(0, TRUE);
}

int 
main(int argc, char* argv[])
{
    num_editors = 0;

    /*
     * Make sure the BDF library is initialized.
     */
    bdf_setup();

    gtk_init(&argc, &argv);  

    argc--;
    argv++;

    editor_setup(argc, argv);

    gtk_main();

    return 0;
}

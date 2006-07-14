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
static char svnid[] __attribute__ ((unused)) = "$Id: guifile.c 60 2006-07-14 15:58:19Z mleisher $";
#else
static char svnid[] = "$Id: guifile.c 60 2006-07-14 15:58:19Z mleisher $";
#endif
#endif

#include "gbdfed.h"
#include "labcon.h"

#ifdef HAVE_XLIB
#include <gdk/gdkx.h>
#endif

/*
 * These are formats that can appear in the editor for importing/loading and
 * exporting fonts.
 */
#define BDF_FORMAT     1
#define CONSOLE_FORMAT 2
#define PKGF_FORMAT    3
#define FNT_FORMAT     4
#define HBF_FORMAT     5
#define OTF_FORMAT     6
#define HEX_FORMAT     7
#define PSF_FORMAT     8
#define PSFUNI_FORMAT  9

/*
 * An array of filters used for the open/import and save dialogs.
 */
static GtkFileFilter *filename_filters[10];

/*
 * This variable is used to track whether the save dialog has been closed
 * so the guifile_save_as_wait() routine knows when to return to the main
 * application.
 */
static gboolean save_dialog_done;

#ifdef HAVE_FREETYPE

#include FT_GLYPH_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H

/*
 * Globals used for FreeType.
 */
static FT_Library library;
static FT_Face face;

/*
 * Globals used for importing OpenType fonts.
 */
static gboolean ftinit = FALSE;
static gboolean otf_collection;
static gboolean otf_face_open;
static gint otf_select_done = 0;
static gchar *otf_fullpath;

/*
 * These are the widgets that will be needed for importing OpenType fonts.
 */
static GtkWidget *otf_dialog;
static GtkWidget *otf_faces;
static GtkWidget *otf_platforms;
static GtkWidget *otf_encodings;
static GtkWidget *otf_point_size;
static GtkWidget *otf_hres;
static GtkWidget *otf_vres;

/*
 * List of platform IDs seen that is used when platforms are selected
 * from OpenType fonts.
 */
static gint16 platforms[32];
static gint nplatforms;

/*
 * List of encoding IDs seen that is used when encodings are selected
 * from OpenType fonts.
 */
static gint16 encodings[34];
static gint nencodings;

/*
 * Variables to hold the selected platform and encoding ID's.
 */
static gint16 otf_pid_pos;
static gint16 otf_eid_pos;

#endif /* HAVE_FREETYPE */

#ifdef HAVE_XLIB

/*
 * These are for importing fonts from the X server.
 */
#define _XSRV_MAX_FONTS 32767
#define _XSRV_DEFAULT_FILTER "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

static GtkWidget *xsrv_dialog;
static GtkWidget *xsrv_filter_text;
static GtkWidget *xsrv_selection_text;
static GtkWidget *xsrv_font_list;
static GtkWidget *xsrv_import;

#endif /* HAVE_XLIB */

/*
 * Widgets for dealing with exporting PSF fonts.
 */
static GtkWidget *psf_export_frame;
static GtkWidget *psf_export_options;

/*
 * Widgets for selecting fonts from a Windows font archive.
 */
static GtkWidget *fnt_dialog;
static GtkWidget *fnt_font_list;
static GtkWidget *fnt_load_button;

/*
 * This is a list of Windows fonts that have been selected.  It assumes that
 * the font file will never contain more than 32 fonts.
 */
static gint fnt_selected[32];
static gint fnt_selected_count;

/*
 * A structure used to pass data to the load and cancel callbacks when dealing
 * with FON/FNT fonts.
 */
typedef struct {
    gchar *file;
    gchar *dir;
    gchar *dot;
    bdffnt_font_t font;
} _bdffnt_callback_data_t;

/*
 * This is used in a couple of cases to point at the active editor.
 */
static gbdfed_editor_t *active_editor;

static void
make_file_chooser_filters(void)
{
    if (filename_filters[0] != 0)
      return;

    filename_filters[BDF_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[BDF_FORMAT],
                                "*.[Bb][Dd][Ff]");

    filename_filters[CONSOLE_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[CONSOLE_FORMAT], "*");

    filename_filters[PKGF_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[PKGF_FORMAT],
                                "*[PpGg][KkFf]");

    filename_filters[FNT_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[FNT_FORMAT],
                                "*.[FfEeDd][OoNnXxLl][NnTtEeLl]");

    filename_filters[HEX_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[HEX_FORMAT],
                                "*.[Hh][Ee][Xx]");

    filename_filters[PSF_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[PSF_FORMAT],
                                "*.[Ps][Ss][Ff]*");

    /*
     * This one is basically for exporting unimap files that belong to PSF
     * fonts.
     */
    filename_filters[PSFUNI_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[PSFUNI_FORMAT],
                                "*.[Uu][Nn][Ii]");

#ifdef HAVE_HBF
    filename_filters[HBF_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[HBF_FORMAT],
                                "*.[Hh][Bb][Ff]");
#endif

#ifdef HAVE_FREETYPE
    filename_filters[OTF_FORMAT] = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filename_filters[OTF_FORMAT],
                                "*.[OoTt][Tt][FfCcEe]");
#endif /* HAVE_FREETYPE */

    filename_filters[0] = (GtkFileFilter *) 1;
}

static gboolean
export_font(gchar *filename, gbdfed_editor_t *ed, gboolean copy_filename)
{
    FILE *out;
    bdf_font_t *font;
    gboolean local_font = FALSE;
    bdf_property_t vanity;
    FontgridSelectionInfo sinfo;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    /*
     * First, attempt to make a backup if they are specified.
     */
    if (options.backups) {
        out = fopen(filename, "rb");
        if (out != 0) {
            fclose(out);

            /*
             * Attempt to make a backup.
             */
            sprintf(buffer2, "%s.bak", filename);

            /*
             * %PLATFORM_CHECK%
             *
             * Don't return here because we want to save the font even if a
             * backup can't be made.
             */
            if (rename(filename, buffer2))
              guiutil_error_message(ed->shell,
                                    "Backups: Unable to make a backup.");
        }
    }

    /*
     * Try to open the file for writing. Only PSF needs binary.
     */
    out = (ed->export_format != PSF_FORMAT) ?
        fopen(filename, "w") : fopen(filename, "wb");

    if (out == 0) {
        if (ed->export_format == BDF_FORMAT)
          sprintf(buffer2, "Save Font: Unable to write to %s.", filename);
        else
          sprintf(buffer2, "Export Font: Unable to write to %s.", filename);
        guiutil_error_message(ed->shell, buffer2);
        return FALSE;
    }

    switch (ed->export_format) {
      case BDF_FORMAT:
        if (!font) {
            /*
             * We need to create a font with the default options so it
             * can be written out as a skeleton.
             */
            font = bdf_new_font("unnamed",
                                options.font_opts.point_size,
                                options.font_opts.resolution_x,
                                options.font_opts.resolution_y,
                                options.font_opts.font_spacing,
                                options.font_opts.bits_per_pixel);
            local_font = TRUE;
        }

        /*
         * Add a custom property if the font has been
         */
        if (font->modified || local_font == TRUE) {
            sprintf(buffer2, "Edited with gbdfed %s.", GBDFED_VERSION);
            vanity.name = "_GBDFED_INFO";
            vanity.format = BDF_ATOM;
            vanity.value.atom = buffer2;
            bdf_add_font_property(font, &vanity);
        }
        bdf_save_font(out, font, &options.font_opts, 0, 0);
        if (local_font == TRUE)
          bdf_free_font(font);
        break;
      case HEX_FORMAT:
        bdf_export_hex(out, font, &options.font_opts, 0, 0);
        break;
      case PSF_FORMAT:
        sinfo.start = sinfo.end = 0;
        (void) fontgrid_has_selection(FONTGRID(ed->fgrid), &sinfo);
        if (sinfo.start == sinfo.end) {
            sinfo.start = font->glyphs[0].encoding;
            sinfo.end = font->glyphs[font->glyphs_used - 1].encoding;
        }
        switch (bdf_export_psf(out, font, &options.font_opts,
                               sinfo.start, sinfo.end)) {
          case BDF_OK:
            buffer1[0] = 0;
            break;
          case BDF_BAD_RANGE:
            sprintf(buffer1, "Export PSF: Invalid range %d-%d.\n",
                    sinfo.start, sinfo.end);
            break;
          case BDF_PSF_CORRUPT_UTF8:
            strcpy(buffer1,
                   "Export PSF: Bad UTF-8 encountered in the mappings.");
            break;
        }
        if (buffer1[0] != 0)
          /*
           * Something went wrong during the PSF export.
           */
          guiutil_error_message(ed->shell, buffer1);
    }

    fclose(out);

    /*
     * The rest of this only applies to BDF fonts and not PSF or HEX fonts.
     * PSF and HEX fonts have their own extensions in the save dialog, but
     * that does not affect the actual file name in the editor.
     */
    if (ed->export_format == BDF_FORMAT) {

        /*
         * Copy the path and filename into the editor if specified.
         */
        if (copy_filename) {
            if (ed->path)
              g_free(ed->path);
            if (ed->file)
              g_free(ed->file);
            ed->path = ed->file = 0;
            ed->file = g_path_get_basename(filename);
            ed->path = g_path_get_dirname(filename);
        }

        /*
         * Mark the font as being unmodified.
         */
        fontgrid_set_font_modified(FONTGRID(ed->fgrid), FALSE);

        /*
         * Update the window title accordingly.
         */
        if (ed->file)
          sprintf(buffer1, "%s - %s", g_get_prgname(), ed->file);
        else
          sprintf(buffer1, "%s - (unnamed%d)", g_get_prgname(), ed->id);

        gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

        /*
         * Since the font was saved as BDF, it is no longer marked as being
         * imported.
         */
        ed->imported = FALSE;
    }

    return TRUE;
}

static void
really_save_font(guint ed_id)
{
    gbdfed_editor_t *ed = editors + ed_id;
    gchar *fname;
    FILE *have;

    fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ed->save_dialog));

    have = fopen(fname, "rb");
    if (have != 0) {
        fclose(have);

        /*
         * Check to see if the user wishes to overwrite the existing font.
         */
        sprintf(buffer2, "Save Font: %s exists.\nDo you wish to overwrite?",
                fname);
        if (guiutil_yes_or_no(ed->shell, buffer2, TRUE) == FALSE) {
            g_free(fname);
            return;
        }
    }

    /*
     * If the write was successful, hide the dialog.
     */
    if (export_font(fname, ed, TRUE)) {
        save_dialog_done = TRUE;
        gtk_widget_hide(ed->save_dialog);
    }
    g_free(fname);
}

/*
 * This callback routine handles errors and updating the progress bar if
 * one is being used.
 */
static void
handle_import_messages(bdf_callback_struct_t *call_data, void *client_data)
{
    if (call_data->reason == BDF_ERROR) {
        sprintf(buffer1, "Import Font:%ld: error: See the font messages.",
                call_data->errlineno);
        guiutil_error_message(GTK_WIDGET(client_data), buffer1);
    }
}

/**************************************************************************
 *
 * BDF section.
 *
 **************************************************************************/

static void
load_bdf_font(gbdfed_editor_t *ed, gchar *fullpath, gchar *dir, gchar *file)
{
    FILE *in;
    bdf_font_t *font;

    /*
     * Check to see if the file can be opened.
     */
    if ((in = fopen(fullpath, "rb")) == 0) {
        sprintf(buffer1, "Import Font: Unable to open %s.", file);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    guiutil_busy_cursor(ed->shell, TRUE);
    guiutil_busy_cursor(ed->open_dialog, TRUE);

    font = bdf_load_font(in, &options.font_opts,
                         handle_import_messages, (void *) ed->shell);

    guiutil_busy_cursor(ed->shell, FALSE);
    guiutil_busy_cursor(ed->open_dialog, FALSE);

    if (font == 0) {
        fclose(in);
        sprintf(buffer1, "Import Font: Unable to load %s.", file);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    fclose(in);
    gtk_widget_hide(ed->open_dialog);

    /*
     * Delete the file and path names so they can be updated.
     */
    if (ed->file != 0)
      g_free(ed->file);
    if (ed->path != 0)
      g_free(ed->path);

    ed->file = ed->path = 0;

    ed->file = strdup(file);
    ed->path = strdup(dir);

    /*
     * Update the window title.
     */
    if (font->modified)
      sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ed->file);
    else
      sprintf(buffer1, "%s - %s", g_get_prgname(), ed->file);

    gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

    /*
     * Tell the glyphtest widget to remove references to the current font if
     * it has any, and redraw.
     */
    if (glyphtest != 0)
      glyphtest_remove_font(GLYPHTEST(glyphtest),
                            fontgrid_get_font(FONTGRID(ed->fgrid)));

    fontgrid_set_font(FONTGRID(ed->fgrid), font, -1);

    /*
     * Finally, update the font name field.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->fontname),
                       fontgrid_get_font_name(FONTGRID(ed->fgrid)));

    /*
     * Make sure the imported flag is cleared in this case.
     */
    ed->imported = FALSE;
}

/**************************************************************************
 *
 * Console section.
 *
 **************************************************************************/

static void
load_console_font(gbdfed_editor_t *ed, gchar *fullpath, gchar *dot,
                  gchar *dir, gchar *file)
{
    FILE *in;
    gbdfed_editor_t *ep;
    gint i, j, nfonts;
    gchar *np;
    bdf_font_t *fonts[3];

    /*
     * Check to see if the file can be opened.
     */
    if ((in = fopen(fullpath, "rb")) == 0) {
        sprintf(buffer1, "Import Font: Unable to open %s.", fullpath);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    guiutil_busy_cursor(ed->shell, TRUE);
    guiutil_busy_cursor(ed->open_dialog, TRUE);

    i = bdf_load_console_font(in, &options.font_opts, 0, 0, fonts, &nfonts);

    guiutil_busy_cursor(ed->shell, FALSE);
    guiutil_busy_cursor(ed->open_dialog, FALSE);

    fclose(in);

    if (i != BDF_OK) {
        /*
         * Free up any font structures that happened to be loaded.
         */
        for (j = 0; j < nfonts; j++)
          bdf_free_font(fonts[j]);

        sprintf(buffer1, "Import Font: %s not a console font.", fullpath);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    gtk_widget_hide(ed->open_dialog);

    /*
     * Handle creation of the editors.  In the case of some console fonts,
     * there are three different sizes contained in the font.
     */
    for (i = 0; i < nfonts; i++) {
        if (i)
          ep = editors + gbdfed_make_editor(0, FALSE);
        else {
            ep = ed;

            /*
             * Erase the existing file and directory name in the "root"
             * editor.
             */
            if (ep->file)
              g_free(ep->file);
            if (ep->path)
              g_free(ep->path);
            ep->file = ep->path = 0;

            /*
             * Tell the glyphtest widget to remove references to the current
             * font, if it has any, and redraw.
             */
            if (glyphtest != 0)
              glyphtest_remove_font(GLYPHTEST(glyphtest),
                                    fontgrid_get_font(FONTGRID(ep->fgrid)));
        }

        /*
         * Make an XLFD name for the font using the filename.  Run through the
         * file name and change all occurences of '-' to '_' to avoid problems
         * with '-' being the XLFD field separator.
         */
        for (j = 0, np = file; np < dot; np++, j++)
          buffer2[j] = (*np != '-') ? *np : '_';
        buffer2[j] = 0;

        fonts[i]->name =
            bdf_make_xlfd_name(fonts[i], "Unknown", buffer2);
        bdf_update_properties_from_name(fonts[i]);

        /*
         * Create the default name for the font file.
         */
        if (nfonts == 3) {
            switch (i) {
              case 0:
                sprintf(buffer1, "%.*s-16.bdf", dot - file, file);
                break;
              case 1:
                sprintf(buffer1, "%.*s-14.bdf", dot - file, file);
                break;
              case 2:
                sprintf(buffer1, "%.*s-08.bdf", dot - file, file);
                break;
            }
        } else
          sprintf(buffer1, "%.*s.bdf", dot - file, file);

        /*
         * Set the filename for the editor.
         */
        ep->file = strdup(buffer1);
        ep->path = strdup(dir);

        /*
         * Set the new editor title.
         */
        sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ep->file);
        gtk_window_set_title(GTK_WINDOW(ep->shell), buffer1);

        /*
         * Change the font in the editor.
         */
        fontgrid_set_font(FONTGRID(ep->fgrid), fonts[i], -1);

        /*
         * Indicate the font was imported.
         */
        ed->imported = TRUE;

        /*
         * Update the XLFD name.
         */
        gtk_entry_set_text(GTK_ENTRY(ep->fontname),
                           fontgrid_get_font_name(FONTGRID(ep->fgrid)));
    }
}

/**************************************************************************
 *
 * PK/GF section.
 *
 **************************************************************************/

static void
load_pkgf_font(gbdfed_editor_t *ed, gchar *fullpath, gchar *dot,
               gchar *dir, gchar *file)
{
    FILE *in;
    gint i;
    gchar *np;
    bdf_font_t *font;

    /*
     * Check to see if the file can be opened.
     */
    if ((in = fopen(fullpath, "rb")) == 0) {
        sprintf(buffer1, "Import Font: Unable to open %s.", file);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    guiutil_busy_cursor(ed->shell, TRUE);
    guiutil_busy_cursor(ed->open_dialog, TRUE);

    i = bdf_load_mf_font(in, &options.font_opts, 0, 0, &font);

    guiutil_busy_cursor(ed->shell, FALSE);
    guiutil_busy_cursor(ed->open_dialog, FALSE);

    fclose(in);

    if (i != BDF_OK) {
        sprintf(buffer1, "Import Font: %s not a PK or GF font.", fullpath);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    gtk_widget_hide(ed->open_dialog);

    /*
     * Make an XLFD name for the font using the filename.  Run through the
     * file name and change all occurences of '-' to '_' to avoid problems
     * with '-' being the XLFD field separator.
     */
    for (i = 0, np = file; np < dot; np++, i++)
      buffer2[i] = (*np != '-') ? *np : '_';
    buffer2[i] = 0;

    font->name = bdf_make_xlfd_name(font, "Unknown", buffer2);
    bdf_update_properties_from_name(font);

    /*
     * Now set up a file name.
     */
    sprintf(buffer1, "%.*s.bdf", dot - file, file);

    /*
     * Delete the file and path names so they can be updated.
     */
    if (ed->file != 0)
      g_free(ed->file);
    if (ed->path != 0)
      g_free(ed->path);

    ed->file = strdup(buffer1);
    ed->path = strdup(dir);

    /*
     * Update the window title.
     */
    sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ed->file);
    gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

    /*
     * Tell the glyphtest widget to remove references to the current font if
     * it has any, and redraw.
     */
    if (glyphtest != 0)
      glyphtest_remove_font(GLYPHTEST(glyphtest),
                            fontgrid_get_font(FONTGRID(ed->fgrid)));

    fontgrid_set_font(FONTGRID(ed->fgrid), font, -1);

    /*
     * Indicate the font was imported.
     */
    ed->imported = TRUE;

    /*
     * Finally, update the font name field.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->fontname),
                       fontgrid_get_font_name(FONTGRID(ed->fgrid)));
}

/**************************************************************************
 *
 * FNT section.
 *
 **************************************************************************/

/*
 * Toggles the "Ok" button on or off depending if there was a selection or
 * not.
 */
static void
fnt_check_load_button(GtkWidget *w, gpointer data)
{
    GtkTreeSelection *sel = GTK_TREE_SELECTION(data);

    if (gtk_tree_selection_count_selected_rows(sel) == 0)
      gtk_widget_set_sensitive(fnt_load_button, FALSE);
    else
      gtk_widget_set_sensitive(fnt_load_button, TRUE);
}

static void
fnt_unselect_all(GtkWidget *w, gpointer data)
{
    GtkTreeSelection *sel = GTK_TREE_SELECTION(data);

    gtk_tree_selection_unselect_all(sel);

    /*
     * Disable the Ok button since everything is unselected.
     */
    gtk_widget_set_sensitive(fnt_load_button, FALSE);
}

static void
fnt_select_all(GtkWidget *w, gpointer data)
{
    GtkTreeSelection *sel = GTK_TREE_SELECTION(data);

    gtk_tree_selection_select_all(sel);

    /*
     * Enable the Ok button since everything is unselected.
     */
    gtk_widget_set_sensitive(fnt_load_button, TRUE);
}

static void
fnt_foreach_selected(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
                     gpointer data)
{
    gint *id;

    id = gtk_tree_path_get_indices(path);
    fnt_selected[fnt_selected_count++] = *id;
}

static void
fnt_load_selected_fonts(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ep, *ed = editors + GPOINTER_TO_UINT(data);
    GtkTreeSelection *sel =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(fnt_font_list));
    gint i, nfonts;
    gboolean loaded;
    bdf_font_t **fonts;
    _bdffnt_callback_data_t *cdata;

    fnt_selected_count = 0;

    if ((cdata = g_object_get_data(G_OBJECT(w),
                                   "bdffnt_callback_data")) == NULL) {
        /*
         * Big problem.  Should never happen.
         */
        guiutil_error_message(editors[0].shell,
                              "BIG PROBLEM PASSING OPEN FON/FNT FONT!!!!");
        return;
    }

    /*
     * This collects all the selected indices from the list and puts them in
     * the global fnt_selected array.
     */
    gtk_tree_selection_selected_foreach(sel, fnt_foreach_selected, NULL);

    /*
     * CHANGE - maybe.
     */
    if (fnt_selected_count == 0)
      return;

    /*
     * Hide the dialog that allowed selection of the fonts in the file.
     */
    gtk_widget_hide(fnt_dialog);

    guiutil_busy_cursor(ed->shell, TRUE);
    guiutil_busy_cursor(ed->open_dialog, TRUE);

    fonts = (bdf_font_t **)
        g_malloc(sizeof(bdf_font_t *) * fnt_selected_count);
    for (loaded = TRUE, nfonts = 0;
         nfonts < fnt_selected_count && loaded == TRUE;
         nfonts++) {
        /*
         * If the current font can't be loaded, then assume the rest are
         * not available either.
         */
        if (bdffnt_load_font(cdata->font, fnt_selected[nfonts],
                             0, 0, &fonts[nfonts]) != 0) {
            /*
             * It is easier to get the font name from the font than it is
             * from the list store.
             */
            (void) bdffnt_get_facename(cdata->font, fnt_selected[nfonts], 0,
                                       (unsigned char *) buffer1);
            sprintf(buffer2, "Import Font: Unable to load %s from %s.",
                    buffer1, cdata->file);
            guiutil_error_message(ed->shell, buffer2);

            guiutil_busy_cursor(ed->shell, FALSE);
            guiutil_busy_cursor(ed->open_dialog, FALSE);

            loaded = FALSE;
        }
    }

    guiutil_busy_cursor(ed->shell, FALSE);
    guiutil_busy_cursor(ed->open_dialog, FALSE);

    /*
     * If no fonts were loaded, then simply return with the open dialog still
     * up, giving the user a chance to load another font.
     */
    if (nfonts == 0) {
        g_free(fonts);
        return;
    }

    /*
     * Hide the open dialog.
     */
    gtk_widget_hide(ed->open_dialog);

    /*
     * Create the editors for the fonts that did get loaded.
     */
    for (i = 0; i < nfonts; i++) {
        if (i)
          ep = editors + gbdfed_make_editor(0, FALSE);
        else {
            ep = ed;

            /*
             * Erase the existing file and directory name in the "root"
             * editor.
             */
            if (ep->file)
              g_free(ep->file);
            if (ep->path)
              g_free(ep->path);
            ep->file = ep->path = 0;

            /*
             * Tell the glyphtest widget to remove references to the current
             * font, if it has any, and redraw.
             */
            if (glyphtest != 0)
              glyphtest_remove_font(GLYPHTEST(glyphtest),
                                    fontgrid_get_font(FONTGRID(ep->fgrid)));
        }

        /*
         * Make the BDF file name for the font.
         */
        sprintf(buffer1, "%.*s%ld.bdf", cdata->dot - cdata->file, cdata->file,
                fonts[i]->point_size);

        ep->file = strdup(buffer1);
        ep->path = strdup(cdata->dir);

        /*
         * Set the new editor title.
         */
        sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ep->file);
        gtk_window_set_title(GTK_WINDOW(ep->shell), buffer1);

        /*
         * Change the font in the editor.
         */
        fontgrid_set_font(FONTGRID(ep->fgrid), fonts[i], -1);

        /*
         * Indicate the font was imported.
         */
        ep->imported = TRUE;

        /*
         * Update the XLFD name.
         */
        gtk_entry_set_text(GTK_ENTRY(ep->fontname),
                           fontgrid_get_font_name(FONTGRID(ep->fgrid)));
    }

    g_free(cdata->file);
    g_free(cdata->dir);
    bdffnt_close_font(cdata->font);

    g_free(fonts);
}

static void
fnt_cancel(GtkWidget *w, gpointer data)
{
    _bdffnt_callback_data_t *cdata;

    /*
     * If the load callback stole the data already, this will be NULL.
     */
    if ((cdata = g_object_get_data(G_OBJECT(w),
                                   "bdffnt_callback_data")) == NULL) {
        /*
         * Big problem.  Should never happen.
         */
        guiutil_error_message(editors[0].shell,
                              "BIG PROBLEM PASSING OPEN FON/FNT FONT!!!!");
        return;
    }

    g_free(cdata->file);
    g_free(cdata->dir);
    bdffnt_close_font(cdata->font);

    gtk_widget_hide(fnt_dialog);
}

static void
fnt_row_activate(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *col,
                 gpointer data)
{
    fnt_load_selected_fonts(GTK_WIDGET(view), data);
}

static void
load_windows_font(gbdfed_editor_t *ed, gchar *fullpath, gchar *dot,
                  gchar *dir, gchar *file)
{
    gint i, nfonts;
    bdffnt_font_t fnt;
    bdf_font_t *font;
    _bdffnt_callback_data_t *cdata;
    GtkWidget *button, *vbox, *hbox, *swin;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkTreeSelection *sel;
    GtkTreePath *tpath;
    GtkTreeIter iter;

    if (bdffnt_open_font(fullpath, &fnt) <= 0) {
        sprintf(buffer1, "Import Font: Unable to open %s.", file);
        guiutil_error_message(ed->shell, buffer1);
        g_free(dir);
        return;
    }

    nfonts = bdffnt_font_count(fnt);

    if (nfonts == 1) {
        guiutil_busy_cursor(ed->shell, TRUE);
        guiutil_busy_cursor(ed->open_dialog, TRUE);

        if (bdffnt_load_font(fnt, 0, 0, 0, &font) != 0) {
            sprintf(buffer1, "Import Font: Unable to load %s.", file);
            guiutil_error_message(ed->shell, buffer1);
            g_free(dir);

            guiutil_busy_cursor(ed->shell, FALSE);
            guiutil_busy_cursor(ed->open_dialog, FALSE);

            return;
        }

        guiutil_busy_cursor(ed->shell, FALSE);
        guiutil_busy_cursor(ed->open_dialog, FALSE);

        gtk_widget_hide(ed->open_dialog);

        /*
         * Now set up a file name.
         */
        sprintf(buffer1, "%.*s.bdf", dot - file, file);

        /*
         * Delete the file and path names so they can be updated.
         */
        if (ed->file != 0)
          g_free(ed->file);
        if (ed->path != 0)
          g_free(ed->path);

        ed->file = strdup(buffer1);
        ed->path = strdup(dir);

        /*
         * Update the window title.
         */
        sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ed->file);
        gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

        /*
         * Tell the glyphtest widget to remove references to the current font
         * if it has any, and redraw.
         */
        if (glyphtest != 0)
          glyphtest_remove_font(GLYPHTEST(glyphtest),
                                fontgrid_get_font(FONTGRID(ed->fgrid)));

        fontgrid_set_font(FONTGRID(ed->fgrid), font, -1);

        /*
         * Indicate the font was imported.
         */
        ed->imported = TRUE;

        /*
         * Finally, update the font name field.
         */
        gtk_entry_set_text(GTK_ENTRY(ed->fontname),
                           fontgrid_get_font_name(FONTGRID(ed->fgrid)));
        return;
    }

    /*
     * More than one font was found.  Present the dialog to choose the fonts.
     */
    if (fnt_dialog == 0) {
        /*
         * Create a structure that will hold data needed by a couple callback
         * routines.
         */
        cdata = g_malloc(sizeof(_bdffnt_callback_data_t));
        cdata->file = strdup(file);
        cdata->dir = strdup(dir);
        cdata->dot = cdata->file + (dot - file);
        cdata->font = fnt;

        fnt_dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(fnt_dialog), "Windows Font Selection");

        g_object_set_data(G_OBJECT(fnt_dialog), "bdffnt_callback_data",
                          (gpointer) cdata);

        (void) g_signal_connect(G_OBJECT(fnt_dialog), "delete_event",
                                G_CALLBACK(fnt_cancel), 0);

        vbox = GTK_DIALOG(fnt_dialog)->vbox;
        hbox = GTK_DIALOG(fnt_dialog)->action_area;

        swin = gtk_scrolled_window_new(0, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_ALWAYS);

        store = gtk_list_store_new(1, G_TYPE_STRING);
        fnt_font_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);

        g_object_set_data(G_OBJECT(fnt_font_list), "bdffnt_callback_data",
                          (gpointer) cdata);

        gtk_widget_set_size_request(fnt_font_list, -1, 160);

        cell_renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Fonts: 0",
                                                          cell_renderer,
                                                          "text", 0,
                                                          NULL);

        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(fnt_font_list), column);
        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(fnt_font_list));

        (void) g_signal_connect(G_OBJECT(sel), "changed",
                                G_CALLBACK(fnt_check_load_button),
                                (gpointer) sel);

        gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);

        (void) g_signal_connect(G_OBJECT(fnt_font_list), "row_activated",
                                G_CALLBACK(fnt_row_activate),
                                GUINT_TO_POINTER(ed->id));

        gtk_container_add(GTK_CONTAINER(swin), fnt_font_list);

        gtk_box_pack_start(GTK_BOX(vbox), swin, FALSE, FALSE, 0);

        button = gtk_button_new_with_label("Select All");

        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(fnt_select_all),
                                (gpointer) sel);

        gtk_container_add(GTK_CONTAINER(hbox), button);

        button = gtk_button_new_from_stock(GTK_STOCK_CLEAR);

        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(fnt_unselect_all),
                                (gpointer) sel);

        gtk_container_add(GTK_CONTAINER(hbox), button);

        button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(fnt_cancel),
                                GUINT_TO_POINTER(ed->id));

        gtk_container_add(GTK_CONTAINER(hbox), button);

        fnt_load_button = gtk_button_new_from_stock(GTK_STOCK_OK);

        /*
         * Here we store a bunch of data to the buttons that are necessary to
         * load FON/FNT fonts in the callback.
         */
        g_object_set_data(G_OBJECT(fnt_load_button), "bdffnt_callback_data",
                          (gpointer) cdata);
        g_object_set_data(G_OBJECT(button), "bdffnt_callback_data",
                          (gpointer) cdata);

        (void) g_signal_connect(G_OBJECT(fnt_load_button), "clicked",
                                G_CALLBACK(fnt_load_selected_fonts),
                                GUINT_TO_POINTER(ed->id));

        gtk_container_add(GTK_CONTAINER(hbox), fnt_load_button);

        gtk_widget_show_all(vbox);
        gtk_widget_show_all(hbox);
    } else {
        /*
         * Fill the CDATA item in with the latest info.
         */
        cdata = g_object_get_data(G_OBJECT(fnt_load_button),
                                  "bdffnt_callback_data");
        cdata->file = strdup(file);
        cdata->dir = strdup(dir);
        cdata->dot = cdata->file + (dot - file);
        cdata->font = fnt;
    }

    store =
        GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(fnt_font_list)));
    column = gtk_tree_view_get_column(GTK_TREE_VIEW(fnt_font_list), 0);
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(fnt_font_list));

    /*
     * Set the number of fonts.
     */
    sprintf(buffer1, "Fonts: %d", nfonts);
    gtk_tree_view_column_set_title(column, buffer1);

    /*
     * Clear the list and add the font names.
     */
    gtk_list_store_clear(store);
    for (i = 0; i < nfonts; i++) {
        (void) bdffnt_get_facename(fnt, i, 0, (unsigned char *) buffer1);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, buffer1, -1);
    }

    /*
     * Force the first one to be selected by default.
     */
    tpath = gtk_tree_path_new_from_indices(0, -1);
    gtk_tree_selection_select_path(sel, tpath);

    /*
     * Show the dialog and wait until the selection is done.
     */
    guiutil_show_dialog_centered(fnt_dialog, ed->shell);

    /*
     * Force the user to interact with this dialog before doing anything else.
     */
    gtk_window_set_modal(GTK_WINDOW(fnt_dialog), TRUE);
}

/**************************************************************************
 *
 * OTF section.
 *
 **************************************************************************/

#ifdef HAVE_FREETYPE

static void
choose_otf_encoding(GtkTreeSelection *selection, gpointer data)
{
    gint *rows;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *tpath;

    /*
     * Get the row of the current selection.
     */
    if (gtk_tree_selection_get_selected(selection, (GtkTreeModel **) &store,
                                        &iter) == FALSE)
      return;
    tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    rows = gtk_tree_path_get_indices(tpath);
    otf_eid_pos = (gint16) rows[0];
}

static void
choose_otf_platform(GtkTreeSelection *selection, gpointer data)
{
    gchar *name;
    gint i, ncmaps, sel, *rows;
    gint16 pid, eid, lasteid;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *tpath;
    GtkTreeView *tview;

    /*
     * Get the row of the current selection.
     */
    if (gtk_tree_selection_get_selected(selection, (GtkTreeModel **) &store,
                                        &iter) == FALSE)
      return;
    tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    rows = gtk_tree_path_get_indices(tpath);
    otf_pid_pos = (gint16) rows[0];

    /*
     * Clear the encoding list.
     */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(otf_encodings)));
    gtk_list_store_clear(store);

    /*
     * Collect the list of encoding IDs and put their names in the encoding
     * list.
     */
    nencodings = 0;
    ncmaps = face->num_charmaps;
    for (lasteid = -1, sel = i = 0; i < ncmaps; i++) {
        pid = face->charmaps[i]->platform_id;
        eid = face->charmaps[i]->encoding_id;
        if (pid == platforms[otf_pid_pos] && eid != lasteid) {
            name = bdfotf_encoding_name(pid, eid);
            if (strcmp(name, "ISO10646") == 0)
              sel = nencodings;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, name, -1);
            encodings[nencodings++] = eid;
            lasteid = eid;
        }
    }

    /*
     * Default the selection to the ISO10646 encoding.
     */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(otf_encodings));
    tpath = gtk_tree_path_new_from_indices(sel, -1);
    gtk_tree_selection_select_path(selection, tpath);

    /*
     * Make sure the encoding is made visible.
     */
    tview = gtk_tree_selection_get_tree_view(selection);
    gtk_tree_view_scroll_to_cell(tview, tpath, NULL, TRUE, 0.5, 0.5);
}

static void
choose_otf(GtkTreeSelection *selection, gpointer data)
{
    gchar *name;
    gint i, ncmaps, sel, row, *rows;
    gint16 pid, eid, lastpid;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *tpath;
    GtkTreeView *tview;
    GValue val;

    /*
     * This is called after the list is cleared as well, so return if there is
     * no selection.
     */
    if (gtk_tree_selection_get_selected(selection, (GtkTreeModel **) &store,
                                        &iter) == FALSE)
      return;

    /*
     * Get the name of the face currently selected and it's index.  This is
     * way more complicated than it should be.
     */
    (void) memset((char *) &val, 0, sizeof(GValue));
    tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    rows = gtk_tree_path_get_indices(tpath);
    row = rows[0];

    /*
     * Clear the platform list before trying to open the new face.
     */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(otf_platforms)));
    gtk_list_store_clear(store);

    if (otf_collection) {
        if (otf_face_open)
          FT_Done_Face(face);
        if (FT_New_Face(library, otf_fullpath, row, &face)) {
            otf_face_open = FALSE;
            gtk_tree_selection_get_selected(selection,
                                            (GtkTreeModel **) &store,
                                            &iter);
            gtk_tree_model_get_value(GTK_TREE_MODEL(store), &iter, 0, &val);
            name = (gchar *) g_value_get_string(&val);
            sprintf(buffer1,
                    "Import Font: Unable to open OpenType collection %s.",
                    name);
            g_value_unset(&val);
            guiutil_error_message(active_editor->shell, buffer1);
            return;
        }
        otf_face_open = TRUE;
    }

    /*
     * Collect the list of platform IDs and put their names in the platform
     * list.
     */
    nplatforms = 0;
    ncmaps = face->num_charmaps;
    for (lastpid = -1, sel = i = 0; i < ncmaps; i++) {
        pid = face->charmaps[i]->platform_id;
        eid = face->charmaps[i]->encoding_id;
        if (pid != lastpid) {
            /*
             * Add the platform name to the list.  If the name happens to be
             * Microsoft, select it as the default.
             */
            name = bdfotf_platform_name(pid);
            if (strcmp(name, "Microsoft") == 0)
              sel = nplatforms;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, name, -1);
            platforms[nplatforms++] = pid;
            lastpid = pid;
        }
    }

    /*
     * Select the default platform, which is hard-coded to be Microsoft at the
     * moment.
     */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(otf_platforms));
    tpath = gtk_tree_path_new_from_indices(sel, -1);
    gtk_tree_selection_select_path(selection, tpath);

    /*
     * Make sure the platform is made visible.
     */
    tview = gtk_tree_selection_get_tree_view(selection);
    gtk_tree_view_scroll_to_cell(tview, tpath, NULL, TRUE, 0.5, 0.5);
}

static void
otf_dialog_done(GtkWidget *w, gpointer data)
{
    otf_select_done = GPOINTER_TO_INT(data);
    gtk_widget_hide(otf_dialog);
}

static void
otf_reset_metrics(GtkWidget *w, gpointer data)
{
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(otf_point_size),
                              (gfloat) options.font_opts.point_size);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(otf_hres),
                              (gfloat) options.font_opts.resolution_x);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(otf_vres),
                              (gfloat) options.font_opts.resolution_y);
}

/*
 * Synchronize the vertical resolution with the horizontal resolution.
 */
static void
otf_sync_res(GtkWidget *w, GdkEventFocus *ev, gpointer data)
{
    gfloat v;
    GtkSpinButton *b;

    b = GTK_SPIN_BUTTON(data);
    v = (gfloat) gtk_spin_button_get_value(b);

    if (v != (gfloat) gtk_spin_button_get_value(GTK_SPIN_BUTTON(w)))
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), v);
}

static void
make_otf_import_dialog(void)
{
    GtkWidget *label, *vbox, *hbox, *button, *table, *swin;
    GtkAdjustment *adj;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkTreeSelection *sel;
    GList *fchain;

    otf_dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(otf_dialog), "OpenType Selection");

    (void) g_signal_connect(G_OBJECT(otf_dialog), "delete_event",
                            G_CALLBACK(gtk_widget_hide), 0);

    vbox = GTK_DIALOG(otf_dialog)->vbox;

    swin = gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_ALWAYS);

    store = gtk_list_store_new(1, G_TYPE_STRING);
    otf_faces = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    cell_renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Faces",
                                                       cell_renderer,
                                                       "text", 0,
                                                       NULL);

    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(otf_faces), column);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(otf_faces));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);

    (void) g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(choose_otf),
                            NULL);

    /*
     * Set the size of the list explicitly to make enough space for
     * approximately five entries.
     */
    gtk_widget_set_size_request(otf_faces, -1, 100);

    gtk_container_add(GTK_CONTAINER(swin), otf_faces);

    gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);

    /*
     * Create a table to hold the other two lists.
     */
    table = gtk_table_new(1, 2, TRUE);

    swin = gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_ALWAYS);

    store = gtk_list_store_new(1, G_TYPE_STRING);
    otf_platforms = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    cell_renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Platforms",
                                                      cell_renderer,
                                                      "text", 0,
                                                      NULL);

    gtk_widget_set_size_request(otf_platforms, 200, 70);

    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(otf_platforms), column);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(otf_platforms));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);

    (void) g_signal_connect(G_OBJECT(sel), "changed",
                            G_CALLBACK(choose_otf_platform), NULL);

    gtk_container_add(GTK_CONTAINER(swin), otf_platforms);

    /*
     * Attach the platform list to the table.
     */
    gtk_table_attach(GTK_TABLE(table), swin, 0, 1, 0, 1,
                     GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

    swin = gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_ALWAYS);

    store = gtk_list_store_new(1, G_TYPE_STRING);
    otf_encodings = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    cell_renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Encodings",
                                                      cell_renderer,
                                                      "text", 0,
                                                      NULL);

    gtk_widget_set_size_request(otf_encodings, 200, 70);

    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(otf_encodings), column);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(otf_encodings));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);

    (void) g_signal_connect(G_OBJECT(sel), "changed",
                            G_CALLBACK(choose_otf_encoding), NULL);

    gtk_container_add(GTK_CONTAINER(swin), otf_encodings);

    /*
     * Attach the encodings list to the table.
     */
    gtk_table_attach(GTK_TABLE(table), swin, 1, 2, 0, 1,
                     GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);

    /*
     * Make a table that will contain the point size and resolution
     * spin buttons.
     */
    table = gtk_table_new(3, 3, FALSE);

    /*
     * Make the spin button labels.
     */
    label = gtk_label_new("Point Size:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL,
                     5, 5);
    label = gtk_label_new("Horizontal Resolution:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, GTK_FILL, GTK_FILL,
                     5, 5);
    label = gtk_label_new("Vertical Resolution:");
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL, GTK_FILL,
                     5, 5);

    /*
     * Make the spin buttons.
     */
    adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 4.0, 256.0, 1.0, 2.0, 0.0);
    otf_point_size = gtk_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(otf_point_size), TRUE);
    gtk_widget_set_size_request(otf_point_size, 100, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(otf_point_size),
                              (gfloat) options.font_opts.point_size);
    gtk_table_attach(GTK_TABLE(table), otf_point_size, 1, 2, 0, 1,
                     GTK_FILL, GTK_FILL, 5, 5);

    adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 72.0, 1200.0,
                                               1.0, 10.0, 0.0);
    otf_hres = gtk_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(otf_hres), TRUE);
    gtk_widget_set_size_request(otf_hres, 100, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(otf_hres),
                              (gfloat) options.font_opts.resolution_x);
    gtk_table_attach(GTK_TABLE(table), otf_hres, 1, 2, 1, 2,
                     GTK_FILL, GTK_FILL, 5, 5);

    adj = (GtkAdjustment *) gtk_adjustment_new(0.0, 72.0, 1200.0,
                                               1.0, 10.0, 0.0);
    otf_vres = gtk_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(otf_vres), TRUE);
    gtk_widget_set_size_request(otf_vres, 100, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(otf_vres),
                              (gfloat) options.font_opts.resolution_y);
    (void) g_signal_connect(G_OBJECT(otf_vres), "focus-in-event",
                            G_CALLBACK(otf_sync_res),
                            (gpointer) otf_hres);
    (void) g_signal_connect(G_OBJECT(otf_hres), "focus-in-event",
                            G_CALLBACK(otf_sync_res),
                            (gpointer) otf_vres);
    gtk_table_attach(GTK_TABLE(table), otf_vres, 1, 2, 2, 3,
                     GTK_FILL, GTK_FILL, 5, 5);

    /*
     * Make the reset button.
     */
    label = gtk_button_new_with_label("Reset");
    (void) g_signal_connect(G_OBJECT(label), "clicked",
                            G_CALLBACK(otf_reset_metrics), 0);
    gtk_table_attach(GTK_TABLE(table), label, 2, 3, 1, 2, GTK_FILL, GTK_FILL,
                     10, 0);

    /*
     * Do some fiddling to adjust the focus chain so the Reset button is at
     * the end instead of in the middle.
     */
    fchain = g_list_append(NULL, (gpointer) otf_point_size);
    fchain = g_list_append(fchain, (gpointer) otf_hres);
    fchain = g_list_append(fchain, (gpointer) otf_vres);
    fchain = g_list_append(fchain, (gpointer) label);
    gtk_container_set_focus_chain(GTK_CONTAINER(table), fchain);
    g_list_free(fchain);

    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, FALSE, 10);

    /*
     * Add the buttons at the bottom of the dialog.
     */
    hbox = GTK_DIALOG(otf_dialog)->action_area;

    button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_container_add(GTK_CONTAINER(hbox), button);
    (void) g_signal_connect(G_OBJECT(button), "clicked",
                            G_CALLBACK(otf_dialog_done),
                            GINT_TO_POINTER(-1));
    button = gtk_button_new_from_stock(GTK_STOCK_OK);
    gtk_container_add(GTK_CONTAINER(hbox), button);
    (void) g_signal_connect(G_OBJECT(button), "clicked",
                            G_CALLBACK(otf_dialog_done),
                            GINT_TO_POINTER(1));

    gtk_widget_show_all(vbox);
    gtk_widget_show_all(hbox);
}

static void
load_otf_font(gbdfed_editor_t *ed, gchar *fullpath, gchar *dot,
              gchar *dir, gchar *file)
{
    gint i, res;
    gint32 psize, hres, vres;
    gchar *np;
    bdf_font_t *font;
    bdf_property_t prop;
    GtkListStore *store;
    GtkTreeIter iter;

    active_editor = ed;
    otf_fullpath = fullpath;

    /*
     * Determine if this is an OT collection or just a normal font.
     */
    np = dot + strlen(dot) - 1;
    otf_collection = (*np == 'c' || *np == 'C') ? TRUE : FALSE;

    /*
     * Initialize the FreeType engine once.
     */
    if (!ftinit) {
        if (FT_Init_FreeType(&library) != 0) {
            strcpy(buffer1,
                   "Import Font: Unable to initialize the FreeType engine.");
            guiutil_error_message(ed->shell, buffer1);
            return;
        }
        ftinit = TRUE;
    }

    /*
     * Attempt to open the font or collection.
     */
    if (FT_New_Face(library, fullpath, 0, &face)) {
        if (!otf_collection)
          sprintf(buffer1, "Import Font: Unable to open OpenType font '%s'.",
                  file);
        else
          sprintf(buffer1,
                  "Import Font: Unable to open OpenType collection '%s'.",
                  file);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    /*
     * Construct the dialog that will display various choices that will be
     * needed when loading the font.
     */
    if (otf_dialog == 0)
      make_otf_import_dialog();

    /*
     * Clear the lists and reset the values.
     */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(otf_faces)));
    gtk_list_store_clear(store);

    otf_face_open = TRUE;
    otf_collection = face->num_faces;
    np = buffer1;

    if (otf_collection == 1) {
        if (bdfotf_get_english_string(face, BDFOTF_FULLNAME_STRING,
                                      0, buffer1) == 0)
          (void) strcpy(buffer1, "Unknown");
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, buffer1, -1);
    } else {
        otf_face_open = FALSE;
        FT_Done_Face(face);
        for (i = 0; i < otf_collection; i++) {
            if (!FT_New_Face(library, fullpath, i, &face)) {
                if (bdfotf_get_english_string(face, BDFOTF_FULLNAME_STRING,
                                              0, buffer1) == 0)
                  sprintf(buffer1, "Unknown%d", i);

                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, buffer1, -1);

                FT_Done_Face(face);
            }
        }
    }

    guiutil_show_dialog_centered(otf_dialog, ed->shell);

    /*
     * Force the user to interact with this dialog before doing anything else.
     */
    gtk_window_set_modal(GTK_WINDOW(otf_dialog), TRUE);

    otf_select_done = 0;
    while (otf_select_done == 0)
      gtk_main_iteration();

    /*
     * Reinitialize various globals when we are done.
     */
    active_editor = 0;
    otf_fullpath = 0;

    if (otf_select_done < 0) {
        if (otf_face_open)
          FT_Done_Face(face);
        otf_face_open = FALSE;
        return;
    }

    /*
     * Get the requested point size and resolutions.
     */
    psize = (gint32)
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(otf_point_size));
    hres = (gint32)
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(otf_hres));
    vres = (gint32)
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(otf_vres));

    guiutil_busy_cursor(ed->shell, TRUE);
    guiutil_busy_cursor(ed->open_dialog, TRUE);

    /*
     * Actually store the resolution and point size to the options so they
     * will be used for other imports.  The setup dialog will unfortunately
     * assume this are the default values.  May fix later so setup knows the
     * values changed.
     */
    options.font_opts.point_size = psize;
    options.font_opts.resolution_x = hres;
    options.font_opts.resolution_y = vres;

    /*
     * Actually load the font.
     */
    res = bdfotf_load_font(face, platforms[otf_pid_pos],
                           encodings[otf_eid_pos], &options.font_opts,
                           0, 0, &font);

    guiutil_busy_cursor(ed->shell, FALSE);
    guiutil_busy_cursor(ed->open_dialog, FALSE);

    FT_Done_Face(face);
    otf_face_open = FALSE;

    if (!res) {
        /*
         * Make an error message.
         */
        sprintf(buffer1, "Import Font: Unable to load OpenType font %s.",
                file);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    /*
     * Hide the open dialog.
     */
    gtk_widget_hide(ed->open_dialog);

    /*
     * Add the _OTF_FONTFILE property using the original filename.
     */
    prop.name = "_OTF_FONTFILE";
    prop.format = BDF_ATOM;
    prop.value.atom = file;
    bdf_add_font_property(font, &prop);

    /*
     * Now set up a file name.
     */
    sprintf(buffer1, "%.*s.bdf", dot - file, file);

    /*
     * Delete the file and path names so they can be updated.
     */
    if (ed->file != 0)
      g_free(ed->file);
    if (ed->path != 0)
      g_free(ed->path);

    ed->file = strdup(buffer1);
    ed->path = strdup(dir);

    /*
     * Update the window title.
     */
    sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ed->file);
    gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

    /*
     * Tell the glyphtest widget to remove references to
     * the current font if it has any, and redraw.
     */
    if (glyphtest != 0)
      glyphtest_remove_font(GLYPHTEST(glyphtest),
                            fontgrid_get_font(FONTGRID(ed->fgrid)));

    fontgrid_set_font(FONTGRID(ed->fgrid), font, -1);

    /*
     * Indicate the font was imported.
     */
    ed->imported = TRUE;

    /*
     * Finally, update the font name field.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->fontname),
                       fontgrid_get_font_name(FONTGRID(ed->fgrid)));
}

#endif /* HAVE_FREETYPE */

#ifdef HAVE_HBF

/**************************************************************************
 *
 * HBF section.
 *
 **************************************************************************/

static void
load_hbf_font(gbdfed_editor_t *ed, gchar *fullpath, gchar *dot,
              gchar *dir, gchar *file)
{
    bdf_font_t *font;

    guiutil_busy_cursor(ed->shell, TRUE);
    guiutil_busy_cursor(ed->open_dialog, TRUE);

    font = bdf_load_hbf_font(fullpath, &options.font_opts, 0, 0);

    guiutil_busy_cursor(ed->shell, FALSE);
    guiutil_busy_cursor(ed->open_dialog, FALSE);

    /*
     * Check to see if the file can be opened.
     */
    if (font == 0) {
        g_free(dir);
        sprintf(buffer1, "Import Font: Unable to import %s.", file);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    gtk_widget_hide(ed->open_dialog);

    /*
     * Now set up a file name.
     */
    sprintf(buffer1, "%.*s.bdf", dot - file, file);

    /*
     * Delete the file and path names so they can be updated.
     */
    if (ed->file != 0)
      g_free(ed->file);
    if (ed->path != 0)
      g_free(ed->path);

    ed->file = strdup(buffer1);
    ed->path = dir;

    /*
     * Update the window title.
     */
    sprintf(buffer1, "%s - %s [modified]", g_get_prgname(), ed->file);
    gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

    /*
     * Tell the glyphtest widget to remove references to
     * the current font if it has any, and redraw.
     */
    if (glyphtest != 0)
      glyphtest_remove_font(GLYPHTEST(glyphtest),
                            fontgrid_get_font(FONTGRID(ed->fgrid)));

    fontgrid_set_font(FONTGRID(ed->fgrid), font, -1);

    /*
     * Indicate the font was imported.
     */
    ed->imported = TRUE;

    /*
     * Finally, update the font name field.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->fontname),
                       fontgrid_get_font_name(FONTGRID(ed->fgrid)));
}

#endif

/*
 * This routine actually does the work of opening the font.
 */
static void
really_open_font(guint ed_id)
{
    gbdfed_editor_t *ed = editors + ed_id;
    gchar *filename, *path, *file, *dot;
    GtkFileChooser *fs;

    fs = GTK_FILE_CHOOSER(ed->open_dialog);
    filename = gtk_file_chooser_get_filename(fs);

    /*
     * Split the filename into path and file, locate the extension position in
     * the file name, and make a version of the name with no '-' characters
     * which are field separators in XLFD font names.
     */
    file = g_path_get_basename(filename);
    path = g_path_get_dirname(filename);
    if ((dot = strrchr(file, '.')) == 0)
      dot = file + strlen(file);

    /*
     * If the last character of the filename is a slash, no file name was
     * provided.
     */
    if (filename[strlen(filename) - 1] == G_DIR_SEPARATOR) {
        guiutil_error_message(ed->shell,
                              "Import Font: No file name provided.");
        if (path)
          g_free(path);
        if (file)
          g_free(file);
        g_free(filename);
        return;
    }

    switch (ed->import_format) {
      case BDF_FORMAT:
        load_bdf_font(ed, filename, path, file);
        break;
      case CONSOLE_FORMAT:
        load_console_font(ed, filename, dot, path, file);
        break;
      case PKGF_FORMAT:
        load_pkgf_font(ed, filename, dot, path, file);
        break;
      case FNT_FORMAT:
        load_windows_font(ed, filename, dot, path, file);
        break;
#ifdef HAVE_HBF
      case HBF_FORMAT:
        load_hbf_font(ed, filename, dot, path, file);
        break;
#endif
#ifdef HAVE_FREETYPE
      case OTF_FORMAT:
        load_otf_font(ed, filename, dot, path, file);
        break;
#endif /* HAVE_FREETYPE */
    }

    if (path)
      g_free(path);
    if (file)
      g_free(file);

    g_free(filename);

    /*
     * Force the editor's info to be updated for the new font.  This causes
     * it to change if it is already visible.
     */
    guiedit_update_font_info(ed);
}

static gchar *
make_file_dialog_title(guint type, gboolean save)
{
    gchar *title = 0;

    switch (type) {
      case BDF_FORMAT: title = "BDF"; break;
      case CONSOLE_FORMAT: title = "Console"; break;
      case PKGF_FORMAT: title = "PK/GF"; break;
      case FNT_FORMAT: title = "Windows"; break;
#ifdef HAVE_HBF
      case HBF_FORMAT: title = "HBF"; break;
#endif
      case OTF_FORMAT: title = "TrueType"; break;
      case PSF_FORMAT: title = "PSF"; break;
      case HEX_FORMAT: title = "HEX"; break;
    }

    if (save) {
        if (type == BDF_FORMAT)
          sprintf(buffer1, "Save %s Font", title);
        else
          sprintf(buffer1, "Export %s Font", title);
    } else
      sprintf(buffer1, "Open %s Font", title);

    return buffer1;
}

static void
handle_open_response(GtkDialog *d, gint response, gpointer data)
{
    switch (response) {
      case GTK_RESPONSE_ACCEPT:
        really_open_font(GPOINTER_TO_UINT(data));
        break;
      case GTK_RESPONSE_CANCEL:
        gtk_widget_hide(GTK_WIDGET(d));
        break;
    }
}

static void
update_open_dialog(gbdfed_editor_t *ed, guint type)
{
    GtkFileChooser *fs;

    if (ed->open_dialog == 0) {
        /*
         * Create the file chooser filters if they haven't already been
         * created.
         */
        make_file_chooser_filters();

        ed->open_dialog =
            gtk_file_chooser_dialog_new(make_file_dialog_title(type, FALSE),
                                        GTK_WINDOW(ed->shell),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                        NULL);

        (void) g_signal_connect(G_OBJECT(ed->open_dialog), "response",
                                G_CALLBACK(handle_open_response),
                                GUINT_TO_POINTER(ed->id));
        (void) g_signal_connect(G_OBJECT(ed->open_dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);
    } else if (ed->import_format != type)
      gtk_window_set_title(GTK_WINDOW(ed->open_dialog),
                           make_file_dialog_title(type, FALSE));
    fs = GTK_FILE_CHOOSER(ed->open_dialog);

    /*
     * Set the file filter.
     */
    gtk_file_chooser_set_filter(fs, filename_filters[type]);

    ed->import_format = type;

    /*
     * Set the initial path as a file if it exists.  This is necessary to
     * force the open to occur in the directory where this font was last
     * saved, which might be different than the directory that is currently in
     * the open file selection dialog.
     */
    if (ed->path != 0 && ed->path[0] == G_DIR_SEPARATOR)
      gtk_file_chooser_set_current_folder(fs, ed->path);
}

static void
hide_save_dialog(GtkWidget *w, gpointer data)
{
    gtk_widget_hide(w);
    save_dialog_done = TRUE;
}

static void
set_psf_option(GtkWidget *w, gpointer data)
{
    guint flags = 0;
    gint dotpos;
    gchar *fname, *dot, *slash, *suff = 0;
    GtkFileChooser *fs;

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(w))) {
      case 0:
        flags = BDF_PSF_UNIMAP|BDF_PSF_FONT;
        break;
      case 1:
        flags = BDF_PSF_FONT;
        break;
      case 2:
        flags = BDF_PSF_UNIMAP;
        break;
    }

    if (flags == BDF_PSF_UNIMAP)
      /*
       * Have to change to the .uni suffix.
       */
      suff = ".uni";
    else if (options.font_opts.psf_flags == BDF_PSF_UNIMAP)
      /*
       * Have to change back to the .psfu suffix.
       */
      suff = ".psfu";

    options.font_opts.psf_flags = flags;

    if (suff) {
        /*
         * Change the suffix on the filename in the entry area.
         */
        fs = GTK_FILE_CHOOSER(g_object_get_data(G_OBJECT(w),
                                                  "file-selection-dialog"));
        fname = gtk_file_chooser_get_filename(fs);

        slash = fname;
        if ((dot = (gchar *) strrchr(fname, '.')) != 0) {
            if ((slash = (gchar *) strrchr(fname, G_DIR_SEPARATOR)) == 0)
              slash = fname;
            dotpos = (gint) (dot - slash) - 1;

            /*
             * Copy the new extension in.
             */
            (void) strcpy(dot, suff);
        } else
          dotpos = -1;

        if (*slash == G_DIR_SEPARATOR)
          *slash++ = 0;

        gtk_file_chooser_set_current_name(fs, slash);
        g_free(fname);
    }
}

static void
handle_save_response(GtkDialog *d, gint response, gpointer data)
{
    switch (response) {
      case GTK_RESPONSE_ACCEPT:
        really_save_font(GPOINTER_TO_UINT(data));
        break;
      case GTK_RESPONSE_CANCEL:
        gtk_widget_hide(GTK_WIDGET(d));
        break;
    }
}

static void
update_save_dialog(gbdfed_editor_t *ed, guint type)
{
    GtkWidget *vbox;
    gchar *dot, *slash;
    gint dotpos;

    if (ed->save_dialog == 0) {
        /*
         * Create the file chooser filters if they haven't already been
         * created.
         */
        make_file_chooser_filters();

        ed->save_dialog =
            gtk_file_chooser_dialog_new(make_file_dialog_title(type, TRUE),
                                        GTK_WINDOW(ed->shell),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                        NULL);

        vbox = GTK_DIALOG(ed->save_dialog)->vbox;

        psf_export_options = gtk_combo_box_new_text();
        /*
         * Since the flags have to be set in the save dialog, attach the
         * save dialog to the object so we can set the bits appropriately.
         */
        g_object_set_data(G_OBJECT(psf_export_options),
                          "file-selection-dialog",
                          (gpointer) ed->save_dialog);
        (void) g_signal_connect(G_OBJECT(psf_export_options), "changed",
                                G_CALLBACK(set_psf_option), 0);
        gtk_combo_box_append_text(GTK_COMBO_BOX(psf_export_options),
                                  "Font and Unicode Map");
        gtk_combo_box_append_text(GTK_COMBO_BOX(psf_export_options),
                                  "Font Only");
        gtk_combo_box_append_text(GTK_COMBO_BOX(psf_export_options),
                                  "Unicode Map Only");
        gtk_combo_box_set_active(GTK_COMBO_BOX(psf_export_options), 0);

        psf_export_frame = labcon_new_label_defaults("PSF Export Options:",
                                                     psf_export_options, 0);

        (void) g_signal_connect(G_OBJECT(ed->save_dialog), "delete_event",
                                G_CALLBACK(hide_save_dialog), 0);

        (void) g_signal_connect(G_OBJECT(ed->save_dialog), "response",
                                G_CALLBACK(handle_save_response),
                                GUINT_TO_POINTER(ed->id));

        gtk_widget_show_all(psf_export_frame);
        gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(ed->save_dialog),
                                          psf_export_frame);
    } else if (ed->export_format != type)
      gtk_window_set_title(GTK_WINDOW(ed->save_dialog),
                           make_file_dialog_title(type, TRUE));

    /*
     * Set the file filter.
     */
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(ed->save_dialog),
                                filename_filters[type]);


    ed->export_format = type;

    /*
     * Show or hide the PSF exporting options.
     */
    if (type == PSF_FORMAT)
      gtk_widget_show(psf_export_frame);
    else
      gtk_widget_hide(psf_export_frame);

    /*
     * Use the current path and filename as the default.  This is done in case
     * the font was loaded from some directory other than the current default
     * in the file selection dialog for saving.
     */
    if (ed->file != 0)
      sprintf(buffer1, "%s", ed->file);
    else
      sprintf(buffer1, "unnamed%d.bdf", ed->id);

    if ((dot = (gchar *) strrchr(buffer1, '.'))) {
        if ((slash = (gchar *) strrchr(buffer1, G_DIR_SEPARATOR)) == 0)
          slash = buffer1;
        dotpos = (gint) (dot - slash) - 1;

        /*
         * If a PSF or HEX font is being exported, change the extension
         * here.
         */
        if (type == PSF_FORMAT)
          (void) strcpy(dot, ".psfu");
        else if (type == HEX_FORMAT)
          (void) strcpy(dot, ".hex");

    } else
      dotpos = -1;

    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(ed->save_dialog),
                                      buffer1);
    if (ed->path != 0 && ed->path[0] == G_DIR_SEPARATOR)
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(ed->save_dialog),
                                          ed->path);
#if 0
    gtk_editable_set_position(GTK_EDITABLE(fs->selection_entry), dotpos);
    gtk_editable_select_region(GTK_EDITABLE(fs->selection_entry), 0, dotpos);
#endif
}

void
guifile_import_bdf_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (fontgrid_get_font_modified(FONTGRID(ed->fgrid))) {
        if (ed->file == 0)
          sprintf(buffer1, "Save Font: (unnamed%d) modified.  Save?", ed->id);
        else
          sprintf(buffer1, "Save Font: %s modified.  Save?", ed->file);
        if (guiutil_yes_or_no(ed->shell, buffer1, TRUE)) {
            /*
             * If the current file was imported, then make sure to use the
             * Save As... dialog instead of just saving under the name that
             * was constructed when importing.  The user won't know what the
             * default BDF file name of imported fonts look like.
             */
            if (ed->imported)
              guifile_save_as_wait(w, data);
            else
              guifile_save(w, data);
            return;
        }
    }

    update_open_dialog(ed, BDF_FORMAT);

    guiutil_show_dialog_centered(ed->open_dialog, ed->shell);
}

void
guifile_import_console_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (fontgrid_get_font_modified(FONTGRID(ed->fgrid))) {
        if (ed->file == 0)
          sprintf(buffer1, "Save Font: (unnamed%d) modified.  Save?", ed->id);
        else
          sprintf(buffer1, "Save Font: %s modified.  Save?", ed->file);
        if (guiutil_yes_or_no(ed->shell, buffer1, TRUE)) {
            /*
             * If the current file was imported, then make sure to use the
             * Save As... dialog instead of just saving under the name that
             * was constructed when importing.  The user won't know what the
             * default BDF file name of imported fonts look like.
             */
            if (ed->imported)
              guifile_save_as_wait(w, data);
            else
              guifile_save(w, data);
            return;
        }
    }

    update_open_dialog(ed, CONSOLE_FORMAT);

    guiutil_show_dialog_centered(ed->open_dialog, ed->shell);
}

void
guifile_import_pkgf_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (fontgrid_get_font_modified(FONTGRID(ed->fgrid))) {
        if (ed->file == 0)
          sprintf(buffer1, "Save Font: (unnamed%d) modified.  Save?", ed->id);
        else
          sprintf(buffer1, "Save Font: %s modified.  Save?", ed->file);
        if (guiutil_yes_or_no(ed->shell, buffer1, TRUE)) {
            /*
             * If the current file was imported, then make sure to use the
             * Save As... dialog instead of just saving under the name that
             * was constructed when importing.  The user won't know what the
             * default BDF file name of imported fonts look like.
             */
            if (ed->imported)
              guifile_save_as_wait(w, data);
            else
              guifile_save(w, data);
            return;
        }
    }

    update_open_dialog(ed, PKGF_FORMAT);

    guiutil_show_dialog_centered(ed->open_dialog, ed->shell);
}

void
guifile_import_windows_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (fontgrid_get_font_modified(FONTGRID(ed->fgrid))) {
        if (ed->file == 0)
          sprintf(buffer1, "Save Font: (unnamed%d) modified.  Save?", ed->id);
        else
          sprintf(buffer1, "Save Font: %s modified.  Save?", ed->file);
        if (guiutil_yes_or_no(ed->shell, buffer1, TRUE)) {
            /*
             * If the current file was imported, then make sure to use the
             * Save As... dialog instead of just saving under the name that
             * was constructed when importing.  The user won't know what the
             * default BDF file name of imported fonts look like.
             */
            if (ed->imported)
              guifile_save_as_wait(w, data);
            else
              guifile_save(w, data);
            return;
        }
    }

    update_open_dialog(ed, FNT_FORMAT);

    guiutil_show_dialog_centered(ed->open_dialog, ed->shell);
}

#ifdef HAVE_HBF

void
guifile_import_hbf_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (fontgrid_get_font_modified(FONTGRID(ed->fgrid))) {
        if (ed->file == 0)
          sprintf(buffer1, "Save Font: (unnamed%d) modified.  Save?", ed->id);
        else
          sprintf(buffer1, "Save Font: %s modified.  Save?", ed->file);
        if (guiutil_yes_or_no(ed->shell, buffer1, TRUE)) {
            /*
             * If the current file was imported, then make sure to use the
             * Save As... dialog instead of just saving under the name that
             * was constructed when importing.  The user won't know what the
             * default BDF file name of imported fonts look like.
             */
            if (ed->imported)
              guifile_save_as_wait(w, data);
            else
              guifile_save(w, data);
            return;
        }
    }

    update_open_dialog(ed, HBF_FORMAT);

    guiutil_show_dialog_centered(ed->open_dialog, ed->shell);
}

#endif

#ifdef HAVE_FREETYPE

void
guifile_import_otf_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    if (fontgrid_get_font_modified(FONTGRID(ed->fgrid))) {
        if (ed->file == 0)
          sprintf(buffer1, "Save Font: (unnamed%d) modified.  Save?", ed->id);
        else
          sprintf(buffer1, "Save Font: %s modified.  Save?", ed->file);
        if (guiutil_yes_or_no(ed->shell, buffer1, TRUE)) {
            /*
             * If the current file was imported, then make sure to use the
             * Save As... dialog instead of just saving under the name that
             * was constructed when importing.  The user won't know what the
             * default BDF file name of imported fonts look like.
             */
            if (ed->imported)
              guifile_save_as_wait(w, data);
            else
              guifile_save(w, data);
            return;
        }
    }

    update_open_dialog(ed, OTF_FORMAT);

    guiutil_show_dialog_centered(ed->open_dialog, ed->shell);
}

#endif /* HAVE_FREETYPE */

/**************************************************************************
 *
 * X server section.
 *
 **************************************************************************/

#ifdef HAVE_XLIB
/*
 * Only compile this in if it is being built for machine running X.
 */

static void
xsrv_filter(GtkWidget *w, gpointer data)
{
    gchar *pattern, **fonts;
    gint i, nfonts;
    GtkListStore *store;
    GtkTreeViewColumn *col;
    GtkTreeIter iter;

    pattern = (gchar *) gtk_entry_get_text(GTK_ENTRY(xsrv_filter_text));

    fonts = XListFonts(GDK_DISPLAY(), pattern, _XSRV_MAX_FONTS, &nfonts);

    store =
        GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(xsrv_font_list)));
    col = gtk_tree_view_get_column(GTK_TREE_VIEW(xsrv_font_list), 0);

    /*
     * Update the label on the font list with the number of fonts.
     */
    sprintf(buffer1, "Font List: %d", nfonts);
    gtk_tree_view_column_set_title(col, buffer1);

    gtk_list_store_clear(store);
    for (i = 0; i < nfonts; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, fonts[i], -1);
    }

    XFreeFontNames(fonts);
}

static void
xsrv_xlfd_filter(GtkWidget *w, gpointer data)
{
    gtk_entry_set_text(GTK_ENTRY(xsrv_filter_text), _XSRV_DEFAULT_FILTER);
    gtk_widget_activate(xsrv_filter_text);
}

static void
xsrv_select_font(GtkTreeSelection *sel, gpointer data)
{
    gchar *name;
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        gtk_tree_model_get (model, &iter, 0, &name, -1);
        gtk_entry_set_text(GTK_ENTRY(xsrv_selection_text), name);
        g_free(name);
    }
}

static void
xsrv_clear_selection_text(GtkWidget *w, gpointer data)
{
    gtk_entry_set_text(GTK_ENTRY(xsrv_selection_text), "");
}

static void
xsrv_import_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    XFontStruct *xfont;
    bdf_font_t *font;
    gchar *name;

    name = (gchar *) gtk_entry_get_text(GTK_ENTRY(xsrv_selection_text));
    if (strcmp(name, "") == 0) {
        guiutil_error_message(ed->shell,
                              "Import Font: No font name provided.");
        return;
    }

    guiutil_busy_cursor(ed->shell, TRUE);
    guiutil_busy_cursor(xsrv_dialog, TRUE);
    if ((xfont = XLoadQueryFont(GDK_DISPLAY(), name)) == 0) {
        guiutil_busy_cursor(ed->shell, FALSE);
        guiutil_busy_cursor(xsrv_dialog, FALSE);
        sprintf(buffer1, "Import Font: Unable to load server font %s.",
                name);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    font = bdf_load_server_font(GDK_DISPLAY(), xfont, name,
                                &options.font_opts, 0, 0);
    guiutil_busy_cursor(ed->shell, FALSE);
    guiutil_busy_cursor(xsrv_dialog, FALSE);
    XFreeFont(GDK_DISPLAY(), xfont);

    if (font == 0) {
        sprintf(buffer1, "Import Font: Unable to import server font %s.",
                name);
        guiutil_error_message(ed->shell, buffer1);
        return;
    }

    /*
     * Close the dialog.
     */
    gtk_widget_hide(xsrv_dialog);

    if (ed->file != 0)
      g_free(ed->file);
    if (ed->path != 0)
      g_free(ed->path);
    ed->file = ed->path = 0;

    sprintf(buffer1, "%s - unnamed%d [modified]", g_get_prgname(),
            ed->id);
    gtk_window_set_title(GTK_WINDOW(ed->shell), buffer1);

    /*
     * Tell the glyphtest widget to remove references to
     * the current font if it has any, and redraw.
     */
    if (glyphtest != 0)
      glyphtest_remove_font(GLYPHTEST(glyphtest),
                            fontgrid_get_font(FONTGRID(ed->fgrid)));

    fontgrid_set_font(FONTGRID(ed->fgrid), font, -1);

    /*
     * Indicate the font was imported.
     */
    ed->imported = TRUE;

    /*
     * Finally, update the font name field.
     */
    gtk_entry_set_text(GTK_ENTRY(ed->fontname),
                       fontgrid_get_font_name(FONTGRID(ed->fgrid)));
}

static void
xsrv_activate_font(GtkTreeView *view, GtkTreePath *path,
                   GtkTreeViewColumn *col, gpointer data)
{
    xsrv_import_font(GTK_WIDGET(view), data);
}

void
guifile_import_xserver_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    GtkWidget *label, *hbox, *vbox, *text, *swin, *button;
    gchar *name, **fonts;
    gint i, nfonts;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkTreeSelection *sel;
    GtkTreeIter iter;

    if (fontgrid_get_font_modified(FONTGRID(ed->fgrid))) {
        if (ed->file == 0)
          sprintf(buffer1, "Save Font: (unnamed%d) modified.  Save?", ed->id);
        else
          sprintf(buffer1, "Save Font: %s modified.  Save?", ed->file);
        if (guiutil_yes_or_no(ed->shell, buffer1, TRUE)) {
            /*
             * If the current file was imported, then make sure to use the
             * Save As... dialog instead of just saving under the name that
             * was constructed when importing.  The user won't know what the
             * default BDF file name of imported fonts look like.
             */
            if (ed->imported)
              guifile_save_as_wait(w, data);
            else
              guifile_save(w, data);
            return;
        }
    }

    if (xsrv_dialog == 0) {
        xsrv_dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(xsrv_dialog),
                             "X Server Font Selection");
        (void) g_signal_connect(G_OBJECT(xsrv_dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), 0);

        vbox = GTK_DIALOG(xsrv_dialog)->vbox;
        hbox = GTK_DIALOG(xsrv_dialog)->action_area;

        label = gtk_label_new("Filter");
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
        gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

        text = xsrv_filter_text = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(text), _XSRV_DEFAULT_FILTER);
        (void) g_signal_connect(G_OBJECT(text), "activate",
                                G_CALLBACK(xsrv_filter), 0);
        gtk_box_pack_start(GTK_BOX(vbox), text, TRUE, TRUE, 0);

        swin = gtk_scrolled_window_new(0, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_ALWAYS);

        store = gtk_list_store_new(1, G_TYPE_STRING);
        xsrv_font_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);

        (void) g_signal_connect(G_OBJECT(xsrv_font_list), "row_activated",
                                G_CALLBACK(xsrv_activate_font),
                                GUINT_TO_POINTER(ed->id));

        cell_renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes ("Font List: 0",
                                                           cell_renderer,
                                                           "text", 0,
                                                           NULL);

        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW(xsrv_font_list), column);

        /*
         * Force the list to have a certain width and height.
         */
        gtk_widget_set_size_request(xsrv_font_list, 550, 200);

        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(xsrv_font_list));
        gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);

        (void) g_signal_connect(G_OBJECT(sel), "changed",
                                G_CALLBACK(xsrv_select_font), NULL);

        gtk_container_add(GTK_CONTAINER(swin), xsrv_font_list);

        gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);

        label = gtk_label_new("Selection");
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
        gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

        text = xsrv_selection_text = gtk_entry_new();
        (void) g_signal_connect(G_OBJECT(text), "activate",
                                G_CALLBACK(xsrv_import_font),
                                GUINT_TO_POINTER(ed->id));
        gtk_box_pack_start(GTK_BOX(vbox), text, TRUE, TRUE, 0);

        /*
         * Now add the buttons.
         */
        button = xsrv_import = gtk_button_new_with_label("Import");
        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(xsrv_import_font),
                                GUINT_TO_POINTER(ed->id));
        gtk_container_add(GTK_CONTAINER(hbox), button);

        button = xsrv_import = gtk_button_new_with_label("Clear Selection");
        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(xsrv_clear_selection_text), 0);
        gtk_container_add(GTK_CONTAINER(hbox), button);

        button = gtk_button_new_with_label("Filter");
        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(xsrv_filter), 0);
        gtk_container_add(GTK_CONTAINER(hbox), button);

        button = gtk_button_new_with_label("XLFD Filter");
        (void) g_signal_connect(G_OBJECT(button), "clicked",
                                G_CALLBACK(xsrv_xlfd_filter), 0);
        gtk_container_add(GTK_CONTAINER(hbox), button);

        button = gtk_button_new_with_label("Cancel");
        (void) g_signal_connect_object(G_OBJECT(button), "clicked",
                                       G_CALLBACK(gtk_widget_hide),
                                       (gpointer) xsrv_dialog,
                                       G_CONNECT_SWAPPED);
        gtk_container_add(GTK_CONTAINER(hbox), button);

        gtk_widget_show_all(vbox);
        gtk_widget_show_all(hbox);
    }

    store =
        GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(xsrv_font_list)));
    column = gtk_tree_view_get_column(GTK_TREE_VIEW(xsrv_font_list), 0);

    /*
     * Load the list of fonts using the current filter pattern.  This needs to
     * be done each time in case the list of font paths has changed between
     * calls.
     */
    name = (gchar *) gtk_entry_get_text(GTK_ENTRY(xsrv_filter_text));
    fonts = XListFonts(GDK_DISPLAY(), name, _XSRV_MAX_FONTS, &nfonts);

    /*
     * Update the label on the font list with the number of fonts.
     */
    sprintf(buffer1, "Font List: %d", nfonts);
    gtk_tree_view_column_set_title(column, buffer1);

    gtk_list_store_clear(store);
    for (i = 0; i < nfonts; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, fonts[i], -1);
    }

    XFreeFontNames(fonts);

    /*
     * Show the dialog.
     */
    guiutil_show_dialog_centered(xsrv_dialog, ed->shell);

}

#endif /* HAVE_XLIB */

void
guifile_export_psf_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);
    bdf_font_t *font;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    /*
     * Only character cell and mono width fonts can be exported as PSF2.
     */
    if (font->spacing == BDF_PROPORTIONAL) {
        sprintf(buffer2,
                "Export Font: Font cannot be saved as PSF because %s ",
                "the font has proportional width.");
        guiutil_error_message(ed->shell, buffer2);
        return;
    }

    update_save_dialog(ed, PSF_FORMAT);

    guiutil_show_dialog_centered(ed->save_dialog, ed->shell);
}

void
guifile_export_hex_font(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    /*
     * No check is done for a "valid" HEX font because it actually pads the
     * output font into bitmaps of two sizes, the wider size twice as wide
     * as the narrower size.
     */

    update_save_dialog(ed, HEX_FORMAT);

    guiutil_show_dialog_centered(ed->save_dialog, ed->shell);
}

void
guifile_save_as(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    update_save_dialog(ed, BDF_FORMAT);

    guiutil_show_dialog_centered(ed->save_dialog, ed->shell);
}

void
guifile_save_as_wait(GtkWidget *w, gpointer data)
{
    save_dialog_done = FALSE;

    guifile_save_as(w, data);
    while (save_dialog_done == FALSE)
      gtk_main_iteration();
}

void
guifile_save(GtkWidget *w, gpointer data)
{
    gbdfed_editor_t *ed = editors + GPOINTER_TO_UINT(data);

    /*
     * If this is a new font, then we need to show the file selection dialog.
     * Otherwise, simply write the font out.
     */
    if (ed->path == 0 && ed->file == 0) {
        guifile_save_as(w, data);
        return;
    }

    ed->export_format = BDF_FORMAT;
    sprintf(buffer1, "%s/%s", ed->path, ed->file);
    export_font(buffer1, ed, FALSE);
}

void
guifile_new_editor(GtkWidget *w, gpointer data)
{
    guint n;

    n = gbdfed_make_editor(0, FALSE);

    gtk_widget_show_all(editors[n].shell);
}

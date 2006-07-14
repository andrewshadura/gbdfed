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
static char svnid[] __attribute__ ((unused)) = "$Id: guigedit.c 50 2006-05-17 20:12:11Z mleisher $";
#else
static char svnid[] = "$Id: guigedit.c 50 2006-05-17 20:12:11Z mleisher $";
#endif
#endif

#include "gbdfed.h"
#include "glyphedit.h"
#include "labcon.h"
#include "gectrl.h"

#define UPMSG "Glyph Edit: The glyph has been modified.\nDo you want to save?"

static const gchar *lb_xpm[] = {
/* width height num_colors chars_per_pixel */
"    32    32        3            1",
/* colors */
". c None",
"# c #000000",
"r c #ff0000",
/* pixels */
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"..r....r........................",
".rr....rr..##...................",
"rrrrrrrrrr.##...................",
".rr....rr.......................",
"..r....r........................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"##..##..##.##.##..##..##..##..##",
"##..##..##.##.##..##..##..##..##",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##..................."
};

static const gchar *rb_xpm[] = {
/* width height num_colors chars_per_pixel */
"    32    32        3            1",
/* colors */
". c None",
"# c #000000",
"r c #ff0000",
/* pixels */
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................r............r..",
"...........##..rr............rr.",
"...........##.rrrrrrrrrrrrrrrrrr",
"...............rr............rr.",
"................r............r..",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"##..##..##.##.##..##..##..##..##",
"##..##..##.##.##..##..##..##..##",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##..................."
};

static const gchar *as_xpm[] = {
/* width height num_colors chars_per_pixel */
"    32    32        3            1",
/* colors */
". c None",
"# c #000000",
"r c #ff0000",
/* pixels */
"...........##.........r.........",
"...........##........rrr........",
"....................rrrrr.......",
"......................r.........",
"...........##.........r.........",
"...........##.........r.........",
"......................r.........",
"......................r.........",
"...........##.........r.........",
"...........##.........r.........",
"......................r.........",
"......................r.........",
"...........##.........r.........",
"...........##.........r.........",
"......................r.........",
"....................rrrrr.......",
"...........##........rrr........",
"...........##.........r.........",
"................................",
"##..##..##.##.##..##..##..##..##",
"##..##..##.##.##..##..##..##..##",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##..................."
};

static const char *ds_xpm[] = {
/* width height num_colors chars_per_pixel */
"    32    32        3            1",
/* colors */
". c None",
"# c #000000",
"r c #ff0000",
/* pixels */
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"................................",
"...........##...................",
"...........##...................",
"................................",
"##..##..##.##.##..##..##..##..##",
"##..##..##.##.##..##..##..##..##",
"................................",
"...........##.........r.........",
"...........##........rrr........",
"....................rrrrr.......",
"......................r.........",
"...........##.........r.........",
"...........##.........r.........",
"......................r.........",
"....................rrrrr.......",
"...........##........rrr........",
"...........##.........r........."
};

/*
 * Global pixbufs.
 */
static GdkPixbuf *lb_image = 0;
static GdkPixbuf *rb_image = 0;
static GdkPixbuf *as_image = 0;
static GdkPixbuf *ds_image = 0;

typedef struct {
    GtkWidget *dialog;
    GtkWidget *notebook;
    GtkWidget *apply;

    /*
     * The rotate/shear tab.
     */
    GtkWidget *rotate;
    GtkWidget *shear;
    GtkAdjustment *rotate_adj;
    GtkAdjustment *shear_adj;
    GtkWidget *degrees;
    gboolean degrees_modified;

    /*
     * The bounding box resize tab.
     */
    GtkWidget *lbearing;
    GtkWidget *rbearing;
    GtkWidget *ascent;
    GtkWidget *descent;
    gboolean resize_modified;

    /*
     * The PSF mappings tab.
     */
    GtkWidget *psf_add;
    GtkWidget *psf_delete;
    GtkWidget *psf_mappings;
    GtkWidget *psf_input;
    gboolean psf_modified;
} GlypheditNotebookRec;

typedef struct {
    gulong id;
    gulong owner;
    gulong handler;

    GtkAccelGroup *ag;

    GtkWidget *shell;
    GtkWidget *gedit;
    GtkWidget *gectrl;
    GtkWidget *name;
    GtkWidget *encoding;
    GtkWidget *dwidth;
    GtkWidget *metrics;
    GtkWidget *coords;
    GtkWidget *gectltips;

    GtkWidget *file_menu;
    GtkWidget *update;
    GtkWidget *update_prev;
    GtkWidget *update_next;

    GtkWidget *edit_menu;
    GtkWidget *reload;
    GtkWidget *resize;
    GtkWidget *paste;
    GtkWidget *copy;
    GtkWidget *cut;
    GtkWidget *select_all;
    GtkWidget *next;
    GtkWidget *prev;
    GtkWidget *unimap;
    GtkWidget *unimap_page;

    GtkWidget *ops_menu;
    GlypheditNotebookRec ops;
} GlypheditRec;

static GlypheditRec *glyph_editors;
static gulong num_glyph_editors;

static GlypheditRec *
_guigedit_get_glyph_editor(gulong owner)
{
    gulong i;
    GlypheditRec *ge;

    if (num_glyph_editors == 0) {
        glyph_editors = ge =
            (GlypheditRec *) g_malloc0(sizeof(GlypheditRec));
        ge->id = num_glyph_editors++;
    } else {
        for (i = 0; i < num_glyph_editors; i++) {
            if (glyph_editors[i].owner == ~0) {
                ge = &glyph_editors[i];
                ge->owner = owner;
                return ge;
            }
        }
        glyph_editors = (GlypheditRec *)
            g_realloc(glyph_editors,
                      sizeof(GlypheditRec) * (num_glyph_editors + 1));

        ge = glyph_editors + num_glyph_editors;
        (void) memset((char *) ge, 0, sizeof(GlypheditRec));
        ge->id = num_glyph_editors++;
    }
    ge->owner = owner;
    return ge;
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

static void
update_font(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    gbdfed_editor_t *ed = editors + ge->owner;
    const gchar *s;
    gchar *prgname = g_get_prgname();
    gboolean unencoded;
    bdf_glyph_t *glyph;
    GlypheditOperation op;

    if (GTK_WIDGET_IS_SENSITIVE(ge->update) == TRUE) {
        if (glyphedit_get_selecting(GLYPHEDIT(ge->gedit)) == TRUE) {
          /*
           * A selection operation is in progress. Need to switch back to
           * the Draw operation to finalize the selection and then switch
           * back.
           */
            op = glyphedit_get_operation(GLYPHEDIT(ge->gedit));
            glyphedit_change_operation(GLYPHEDIT(ge->gedit), GLYPHEDIT_DRAW);
            glyphedit_change_operation(GLYPHEDIT(ge->gedit), op);
        }

        glyph = glyphedit_get_glyph(GLYPHEDIT(ge->gedit), &unencoded);

        /*
         * Set the new name and device width for the glyph. These may not
         * have actually changed, but this is simplest for the moment.
         */
        if (glyph->name != 0)
          free(glyph->name);
        glyph->name = (gchar *) gtk_entry_get_text(GTK_ENTRY(ge->name));
        s = gtk_entry_get_text(GTK_ENTRY(ge->dwidth));
        glyph->dwidth = (guint16) _bdf_atos((char *) s, 0, 10);

        /*
         * Now update the font itself.
         */
        fontgrid_update_glyph(FONTGRID(ed->fgrid), glyph, unencoded);

        /*
         * Free the glyph structure. The name has already been deallocated
         * and replaced with a possibly new name.
         */
        if (glyph->bytes > 0)
          free(glyph->bitmap);
        free(glyph);

        glyphedit_set_modified(GLYPHEDIT(ge->gedit), FALSE);
    }

    /*
     * Just modified the PSF mappings.
     */
    fontgrid_update_psf_mappings(FONTGRID(ed->fgrid),
                                 glyphedit_get_encoding(GLYPHEDIT(ge->gedit)),
                                 glyphedit_get_psf_mappings(GLYPHEDIT(ge->gedit)));

    /*
     * Unset the modified flag and update the title.
     */
    glyphedit_set_modified(GLYPHEDIT(ge->gedit), FALSE);
    if (ed->file == 0)
      sprintf(buffer1, "%s - Glyph Edit: (unnamed%d)",
              prgname, ed->id);
    else
      sprintf(buffer1, "%s - Glyph Edit: %s", prgname, ed->file);

    gtk_window_set_title(GTK_WINDOW(ge->shell), buffer1);


    gtk_widget_set_sensitive(ge->update, FALSE);
    gtk_widget_set_sensitive(ge->update_next, FALSE);
    gtk_widget_set_sensitive(ge->update_prev, FALSE);
}

/*
 * Code common to both next_glyph() and previous_glyph().
 */
static void
update_glyphedit(gbdfed_editor_t *ed, GlypheditRec *ge, bdf_glyph_grid_t *grid)
{
    gtk_entry_set_text(GTK_ENTRY(ge->name), grid->name);

    if (grid->unencoded)
      sprintf(buffer1, "-1");
    else {
        switch (fontgrid_get_code_base(FONTGRID(ed->fgrid))) {
          case 8: sprintf(buffer1, "%lo", grid->encoding); break;
          case 10: sprintf(buffer1, "%ld", grid->encoding); break;
          case 16: sprintf(buffer1, "%04lX", grid->encoding); break;
        }
    }
    gtk_label_set_text(GTK_LABEL(ge->encoding), buffer1);

    sprintf(buffer1, "%hd", grid->dwidth);
    gtk_widget_set_sensitive(ge->dwidth, TRUE);
    g_signal_handler_block(G_OBJECT(ge->dwidth), ge->handler);
    gtk_entry_set_text(GTK_ENTRY(ge->dwidth), buffer1);
    g_signal_handler_unblock(G_OBJECT(ge->dwidth), ge->handler);

    if (grid->spacing != BDF_PROPORTIONAL) {
        gtk_widget_set_sensitive(ge->dwidth, FALSE);
        if (ge->unimap_page != 0)
          gtk_widget_set_sensitive(ge->unimap_page, TRUE);
    } else if (ge->unimap_page != 0)
      gtk_widget_set_sensitive(ge->unimap_page, FALSE);

    sprintf(buffer1, "width %hd height %hd\r\nascent %hd descent %hd",
            grid->glyph_bbx.width, grid->glyph_bbx.height,
            grid->glyph_bbx.ascent, grid->glyph_bbx.descent);
    gtk_label_set_text(GTK_LABEL(ge->metrics), buffer1);

    /*
     * Set the new grid in the glyph editor.
     */
    glyphedit_set_grid(GLYPHEDIT(ge->gedit), grid);

    /*
     * Set the sensitivity of the update menu items appropriately.
     */
    if (grid->modified) {
        gtk_widget_set_sensitive(ge->update, TRUE);
        gtk_widget_set_sensitive(ge->update_next, TRUE);
        gtk_widget_set_sensitive(ge->update_prev, TRUE);
    } else {
        gtk_widget_set_sensitive(ge->update, FALSE);
        gtk_widget_set_sensitive(ge->update_next, FALSE);
        gtk_widget_set_sensitive(ge->update_prev, FALSE);
    }
}

static void
next_glyph(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    gbdfed_editor_t *ed = editors + ge->owner;
    bdf_font_t *font;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;

    if (GTK_WIDGET_IS_SENSITIVE(ge->update) == TRUE) {
        if (guiutil_yes_or_no(ge->shell, UPMSG, TRUE))
          update_font(w, GUINT_TO_POINTER(ge->id));
    }

    grid = glyphedit_get_grid(GLYPHEDIT(ge->gedit));

    if (fontgrid_select_next_glyph(FONTGRID(ed->fgrid)) == FALSE)
      return;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    if (grid->unencoded)
      grid = bdf_make_glyph_grid(font, grid->encoding + 1, 1);
    else
      grid = bdf_make_glyph_grid(font, grid->encoding + 1, 0);

    update_glyphedit(ed, ge, grid);

    glyphedit_get_image(GLYPHEDIT(ge->gedit), &image);
    gecontrol_set_glyph_image(GECONTROL(ge->gectrl), &image);
    if (image.bytes > 0)
      free(image.bitmap);
}

static void
previous_glyph(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    gbdfed_editor_t *ed = editors + ge->owner;
    bdf_font_t *font;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;

    if (GTK_WIDGET_IS_SENSITIVE(ge->update) == TRUE) {
        if (guiutil_yes_or_no(ge->shell, UPMSG, TRUE))
          update_font(w, GUINT_TO_POINTER(ge->id));
    }

    grid = glyphedit_get_grid(GLYPHEDIT(ge->gedit));

    if (fontgrid_select_previous_glyph(FONTGRID(ed->fgrid)) == FALSE)
      return;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));

    if (grid->unencoded)
      grid = bdf_make_glyph_grid(font, grid->encoding - 1, 1);
    else
      grid = bdf_make_glyph_grid(font, grid->encoding - 1, 0);

    update_glyphedit(ed, ge, grid);

    glyphedit_get_image(GLYPHEDIT(ge->gedit), &image);
    gecontrol_set_glyph_image(GECONTROL(ge->gectrl), &image);
    if (image.bytes > 0)
      free(image.bitmap);
}

static void
update_and_next_glyph(GtkWidget *w, gpointer data)
{
    update_font(w, data);
    next_glyph(w, data);
}

static void
update_and_previous_glyph(GtkWidget *w, gpointer data)
{
    update_font(w, data);
    previous_glyph(w, data);
}

static gboolean
close_glyph_editor(GtkWidget *w, GdkEvent *ev, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    /*
     * Glyph editors with no owners are ignored. This lets us call this
     * routine at application shutdown time to update all modified
     * glyph editors.
     */
    if (ge->owner == ~0)
      return TRUE;

    /*
     * We don't check to see if the grid has been modified, because
     * certain operations cause the modify flag to be set, but they
     * don't really represent a modification.
     */
    if (GTK_WIDGET_IS_SENSITIVE(ge->update) == TRUE) {
        if (guiutil_yes_or_no(ge->shell, UPMSG, TRUE))
          update_font(w, GUINT_TO_POINTER(ge->id));
    }

    /*
     * Release this editor back into the pool to be reused.
     */
    ge->owner = ~0;

    /*
     * Hide the shell.
     */
    gtk_widget_hide(ge->shell);

    return TRUE;
}

static void
activate_close_glyph_editor(GtkWidget *w, gpointer data)
{
    (void) close_glyph_editor(w, 0, data);
}

static GtkWidget *
make_file_menu(GlypheditRec *ge, GtkWidget *menubar)
{
    GtkWidget *file, *menu, *mitem, *sep;

    /*
     * Create the File menu.
     */
    file = gtk_menu_item_new_with_mnemonic("_File");

    ge->file_menu = menu = gtk_menu_new();

    ge->update = make_accel_menu_item(menu, "_Update",
                                      "<Control>S", ge->ag);
    g_signal_connect(G_OBJECT(ge->update), "activate",
                     G_CALLBACK(update_font), GUINT_TO_POINTER(ge->id));

    ge->update_next = make_accel_menu_item(menu, "Update and _Next",
                                           "<Control>U", ge->ag);
    g_signal_connect(G_OBJECT(ge->update_next), "activate",
                     G_CALLBACK(update_and_next_glyph),
                     GUINT_TO_POINTER(ge->id));

    ge->update_prev = make_accel_menu_item(menu, "Update and _Previous",
                                           "<Control>B", ge->ag);
    g_signal_connect(G_OBJECT(ge->update_prev), "activate",
                     G_CALLBACK(update_and_previous_glyph),
                     GUINT_TO_POINTER(ge->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = make_accel_menu_item(menu, "_Close", "<Control>F4", ge->ag);
    (void) g_signal_connect(G_OBJECT(mitem), "activate",
                            G_CALLBACK(activate_close_glyph_editor),
                            GUINT_TO_POINTER(ge->id));

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), menu);

    return file;
}

static gboolean
edit_menu_up(GtkWidget *w, GdkEvent *event, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    Glyphedit *gw;

    gw = GLYPHEDIT(ge->gedit);

    if (glyphedit_clipboard_empty(gw))
      gtk_widget_set_sensitive(ge->paste, FALSE);
    else
      gtk_widget_set_sensitive(ge->paste, TRUE);

    gtk_widget_set_sensitive(ge->copy,
                             glyphedit_get_selecting(gw));
    gtk_widget_set_sensitive(ge->cut,
                             glyphedit_get_selecting(gw));

    if (glyphedit_get_encoding(gw) == 0)
      gtk_widget_set_sensitive(ge->prev, FALSE);
    else
      gtk_widget_set_sensitive(ge->prev, TRUE);

    if (glyphedit_get_encoding(gw) == 0xffff)
      gtk_widget_set_sensitive(ge->next, FALSE);
    else
      gtk_widget_set_sensitive(ge->next, TRUE);

    gtk_widget_set_sensitive(ge->reload, glyphedit_get_modified(gw));

    if (glyphedit_get_spacing(gw) != BDF_PROPORTIONAL)
      gtk_widget_set_sensitive(ge->unimap, TRUE);
    else
      gtk_widget_set_sensitive(ge->unimap, FALSE);

    return FALSE;
}

static gboolean
edit_menu_down(GtkWidget *w, GdkEvent *event, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    Glyphedit *gw;

    gw = GLYPHEDIT(ge->gedit);

    gtk_widget_set_sensitive(ge->paste, TRUE);
    gtk_widget_set_sensitive(ge->copy, TRUE);
    gtk_widget_set_sensitive(ge->cut, TRUE);
    gtk_widget_set_sensitive(ge->prev, TRUE);
    gtk_widget_set_sensitive(ge->next, TRUE);
    gtk_widget_set_sensitive(ge->reload, TRUE);
    gtk_widget_set_sensitive(ge->unimap, TRUE);

    return FALSE;
}

static void
reload_glyph(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    gbdfed_editor_t *ed = editors + ge->owner;
    bdf_font_t *font;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));
    grid = glyphedit_get_grid(GLYPHEDIT(ge->gedit));

    grid = bdf_make_glyph_grid(font, grid->encoding, grid->unencoded);

    update_glyphedit(ed, ge, grid);

    glyphedit_get_image(GLYPHEDIT(ge->gedit), &image);
    gecontrol_set_glyph_image(GECONTROL(ge->gectrl), &image);
    if (image.bytes > 0)
      free(image.bitmap);
}

static void
copy_selection(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    glyphedit_copy_selection(GLYPHEDIT(ge->gedit));
}

static void
cut_selection(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    bdf_bitmap_t image;

    glyphedit_cut_selection(GLYPHEDIT(ge->gedit));
    glyphedit_get_image(GLYPHEDIT(ge->gedit), &image);
    gecontrol_set_glyph_image(GECONTROL(ge->gectrl), &image);
    if (image.bytes > 0)
      free(image.bitmap);
}

static void
paste_selection(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    bdf_bitmap_t image;

    glyphedit_paste_selection(GLYPHEDIT(ge->gedit));
    glyphedit_get_image(GLYPHEDIT(ge->gedit), &image);
    gecontrol_set_glyph_image(GECONTROL(ge->gectrl), &image);
    if (image.bytes > 0)
      free(image.bitmap);
}

static void
select_all(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    glyphedit_select_all(GLYPHEDIT(ge->gedit));
}

static GtkWidget *
make_edit_menu(GlypheditRec *ge, GtkWidget *menubar)
{
    GtkWidget *edit, *menu, *sep;

    /*
     * Create the Edit menu.
     */
    edit = gtk_menu_item_new_with_mnemonic("_Edit");

    ge->edit_menu = menu = gtk_menu_new();
    g_signal_connect(G_OBJECT(menu), "map_event", G_CALLBACK(edit_menu_up),
                     GUINT_TO_POINTER(ge->id));
    g_signal_connect(G_OBJECT(menu), "unmap_event", G_CALLBACK(edit_menu_down),
                     GUINT_TO_POINTER(ge->id));

    ge->reload = make_accel_menu_item(menu, "Re_load",
                                      "<Control>L", ge->ag);
    g_signal_connect(G_OBJECT(ge->reload), "activate",
                     G_CALLBACK(reload_glyph), GUINT_TO_POINTER(ge->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    ge->copy = make_accel_menu_item(menu, "_Copy",
                                    "<Control>C", ge->ag);
    g_signal_connect(G_OBJECT(ge->copy), "activate",
                     G_CALLBACK(copy_selection),
                     GUINT_TO_POINTER(ge->id));

    ge->cut = make_accel_menu_item(menu, "C_ut",
                                   "<Control>X", ge->ag);
    g_signal_connect(G_OBJECT(ge->cut), "activate",
                     G_CALLBACK(cut_selection),
                     GUINT_TO_POINTER(ge->id));

    ge->paste = make_accel_menu_item(menu, "_Paste",
                                     "<Control>V", ge->ag);
    g_signal_connect(G_OBJECT(ge->paste), "activate",
                     G_CALLBACK(paste_selection),
                     GUINT_TO_POINTER(ge->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    ge->select_all = make_accel_menu_item(menu, "Select _All",
                                          "<Control>A", ge->ag);
    g_signal_connect(G_OBJECT(ge->select_all), "activate",
                     G_CALLBACK(select_all),
                     GUINT_TO_POINTER(ge->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    ge->next = make_accel_menu_item(menu, "_Next Glyph",
                                    "<Control>N", ge->ag);
    g_signal_connect(G_OBJECT(ge->next), "activate",
                     G_CALLBACK(next_glyph), GUINT_TO_POINTER(ge->id));

    ge->prev = make_accel_menu_item(menu, "Pre_vious Glyph",
                                    "<Control>P", ge->ag);
    g_signal_connect(G_OBJECT(ge->prev), "activate",
                     G_CALLBACK(previous_glyph), GUINT_TO_POINTER(ge->id));

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), menu);

    return edit;
}

static gboolean
operations_menu_up(GtkWidget *w, GdkEvent *event, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    Glyphedit *gw;

    gw = GLYPHEDIT(ge->gedit);

    if (glyphedit_get_spacing(gw) != BDF_PROPORTIONAL)
      gtk_widget_set_sensitive(ge->unimap, TRUE);
    else
      gtk_widget_set_sensitive(ge->unimap, FALSE);

    return FALSE;
}

static gboolean
operations_menu_down(GtkWidget *w, GdkEvent *event, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    Glyphedit *gw;

    gw = GLYPHEDIT(ge->gedit);
    gtk_widget_set_sensitive(ge->unimap, TRUE);

    return FALSE;
}

static void
draw_operation(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    glyphedit_change_operation(GLYPHEDIT(ge->gedit), GLYPHEDIT_DRAW);
    gecontrol_change_operation(GECONTROL(ge->gectrl), GECONTROL_DRAW);
}

static void
move_operation(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    glyphedit_change_operation(GLYPHEDIT(ge->gedit), GLYPHEDIT_MOVE);
    gecontrol_change_operation(GECONTROL(ge->gectrl), GECONTROL_MOVE);
}

static void
copy_operation(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    glyphedit_change_operation(GLYPHEDIT(ge->gedit), GLYPHEDIT_COPY);
    gecontrol_change_operation(GECONTROL(ge->gectrl), GECONTROL_COPY);
}

static void
set_rotate_limits(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ge->ops.rotate)))
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(ge->ops.degrees),
                                     ge->ops.rotate_adj);
}

static void
set_shear_limits(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    const gchar *s;
    gint16 v = -1000;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ge->ops.shear))) {
        /*
         * This little tap dance is to avoid a minor, but ugly GUI
         * situation where the value in the spin button may be obscured
         * when the adjustment is changed back. The shear value can have
         * at most 2 digits where the rotate value can have 3. Changing
         * back to the shear adjustment can cause a resize of the spin
         * button, sometimes obscuring the value left over from the rotate
         * adjustment.
         */
        s = gtk_entry_get_text(GTK_ENTRY(ge->ops.degrees));
        v = (gint16) _bdf_atos((char *) s, 0, 10);
        if (v < -20)
          v = -20;
        else if (v > 20)
          v = 20;
        if (v != -1000)
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(ge->ops.degrees),
                                    (gdouble) v);
        gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(ge->ops.degrees),
                                       ge->ops.shear_adj);
    }
}

/*
 * Called when the value for rotating or shearing a glyph has changed.
 */
static void
degrees_changed(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    gtk_widget_set_sensitive(ge->ops.apply, TRUE);
    ge->ops.degrees_modified = TRUE;
}

/*
 * Called when any of the fields in the resize tab are changed.
 */
static void
resize_changed(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    gtk_widget_set_sensitive(ge->ops.apply, TRUE);
    ge->ops.resize_modified = TRUE;
}

static gboolean
count_list_items(GtkTreeModel *m, GtkTreePath *p, GtkTreeIter *i,
                 gpointer data)
{
    gint *n = (gint *) data;
    *n = *n + 1;

    return FALSE;
}

static gboolean
collect_list_items(GtkTreeModel *m, GtkTreePath *p, GtkTreeIter *i,
                   gpointer data)
{
    gchar **mappings = (gchar **) data;

    gtk_tree_model_get(m, i, 0,
                       &mappings[gtk_tree_path_get_indices(p)[0]], -1);
    return FALSE;
}

/*
 * Called when the Apply button is pressed.
 */
static void
apply_operations(GtkWidget *w, gint response, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    const gchar *s;
    gint16 lb, rb, as, ds, degrees;
    gint i, n;
    GtkTreeModel *model;
    gchar **mappings;
    bdf_psf_unimap_t *mp;
    bdf_metrics_t metrics;

    if (ge->ops.degrees_modified) {
        /*
         * The degrees of rotatation or shearing have been modified.
         */
        s = gtk_entry_get_text(GTK_ENTRY(ge->ops.degrees));
        degrees = (gint16) _bdf_atos((char *) s, 0, 10);
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ge->ops.rotate)))
          glyphedit_rotate_glyph(GLYPHEDIT(ge->gedit), degrees);
        else
          glyphedit_shear_glyph(GLYPHEDIT(ge->gedit), degrees);
        ge->ops.degrees_modified = FALSE;
    }

    if (ge->ops.resize_modified) {
        /*
         * The bounding box has been modified.
         */
        s = gtk_entry_get_text(GTK_ENTRY(ge->ops.lbearing));
        lb = (gint16) _bdf_atos((char *) s, 0, 10);
        s = gtk_entry_get_text(GTK_ENTRY(ge->ops.rbearing));
        rb = (gint16) _bdf_atos((char *) s, 0, 10);
        s = gtk_entry_get_text(GTK_ENTRY(ge->ops.ascent));
        as = (gint16) _bdf_atos((char *) s, 0, 10);
        s = gtk_entry_get_text(GTK_ENTRY(ge->ops.descent));
        ds = (gint16) _bdf_atos((char *) s, 0, 10);

        metrics.width = rb - lb;
        metrics.x_offset = lb;
        metrics.ascent = as;
        metrics.descent = ds;
        metrics.height = as + ds;
        metrics.y_offset = -ds;

        glyphedit_set_metrics(GLYPHEDIT(ge->gedit), &metrics);
        ge->ops.degrees_modified = FALSE;
    }

    if (ge->ops.psf_modified) {
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(ge->ops.psf_mappings));
        n = 0;
        gtk_tree_model_foreach(model, count_list_items, (gpointer) &n);
        mappings = (gchar **) g_malloc(sizeof(gchar *) * n);
        gtk_tree_model_foreach(model, collect_list_items, (gpointer) mappings);
        mp = glyphedit_get_psf_mappings(GLYPHEDIT(ge->gedit));
        _bdf_psf_pack_mapping(mappings, n,
                              glyphedit_get_encoding(GLYPHEDIT(ge->gedit)),
                              mp);
        for (i = 0; i < n; i++)
          g_free(mappings[i]);
        if (n > 0)
          g_free(mappings);
        glyphedit_set_modified(GLYPHEDIT(ge->gedit), TRUE);
        glyphedit_signal_modified(GLYPHEDIT(ge->gedit));
        ge->ops.psf_modified = FALSE;
    }

    /*
     * Only disable the Apply button if everything has been updated.
     */
    if (ge->ops.degrees_modified == FALSE &&
        ge->ops.resize_modified == FALSE &&
        ge->ops.psf_modified == FALSE)
      gtk_widget_set_sensitive(ge->ops.apply, FALSE);

    gtk_widget_hide(ge->ops.dialog);
}

static void
change_unimap(GtkTreeModel *m, const gchar *path,
              const gchar *ntext, gpointer data)
{
    gchar *ot;
    GtkTreePath *p = gtk_tree_path_new_from_string(path);
    GtkTreeIter iter;

    gtk_tree_model_get_iter(m, &iter, p);
    gtk_tree_model_get(m, &iter, 0, &ot, -1);
    g_free(ot);

    gtk_list_store_set(GTK_LIST_STORE(m), &iter, 0, ntext, -1);
}

static void
add_mapping(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    const gchar *v;
    gchar *i;
    gulong n;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;

    v = gtk_entry_get_text(GTK_ENTRY(ge->ops.psf_input));

    /*
     * Insure that the value is in the form expected.
     */
    n = (gulong) _bdf_atol((char *) v, 0, 16);
    if (n <= 0xffff)
      sprintf(buffer1, "U+%04lX", n);
    else
      sprintf(buffer1, "U+%06lX", n);
    v = (const gchar *) buffer1;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(ge->ops.psf_mappings));
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, v, -1);

    i = gtk_tree_model_get_string_from_iter(model, &iter);
    path = gtk_tree_path_new_from_string(i);
    g_free(i);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(ge->ops.psf_mappings),
                                 path, 0, TRUE, 0.5, 0.0);
    gtk_tree_path_free(path);

    ge->ops.psf_modified = TRUE;

    gtk_entry_set_text(GTK_ENTRY(ge->ops.psf_input), "");
    gtk_widget_set_sensitive(ge->ops.psf_add, FALSE);
    gtk_widget_set_sensitive(ge->ops.apply, TRUE);
}

static void
enable_add(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    const gchar *v = gtk_entry_get_text(GTK_ENTRY(ge->ops.psf_input));

    if (strlen(v) == 0)
      gtk_widget_set_sensitive(ge->ops.psf_add, FALSE);
    else
      gtk_widget_set_sensitive(ge->ops.psf_add, TRUE);
}

static void
delete_unimap(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    GtkTreeModel *model;
    GtkTreeSelection *sel;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(ge->ops.psf_mappings));
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ge->ops.psf_mappings));

    if (gtk_tree_selection_get_selected(sel, 0, &iter))
      gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

static void
operations_dialog_populate(GlypheditRec *ge)
{
    bdf_psf_unimap_t *psf;
    char **mappings;
    int i, nmappings;
    GtkTreeModel *model;
    GtkTreeIter iter;
    bdf_metrics_t metrics;

    /*
     * Populate the fields of the dialog with initial values.
     */
    glyphedit_get_font_metrics(GLYPHEDIT(ge->gedit), &metrics);

    /*
     * The left bearing cannot be set when the font has character cell
     * spacing. But make sure it is enabled so the value from the font
     * can be set.
     */
    gtk_widget_set_sensitive(ge->ops.lbearing, TRUE);

    /*
     * Set the values.
     */
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ge->ops.lbearing),
                              (gdouble) (-metrics.x_offset));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ge->ops.rbearing),
                              (gdouble) (metrics.width + metrics.x_offset));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ge->ops.ascent),
                              (gdouble) metrics.ascent);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ge->ops.descent),
                              (gdouble) metrics.descent);
    if (metrics.font_spacing == BDF_CHARCELL)
      gtk_widget_set_sensitive(ge->ops.lbearing, FALSE);

    /*
     * Add the PSF mappings to the list.
     */
    if ((psf = glyphedit_get_psf_mappings(GLYPHEDIT(ge->gedit)))) {
        /*
         * Erase the list store.
         */
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(ge->ops.psf_mappings));
        gtk_list_store_clear(GTK_LIST_STORE(model));

        mappings = _bdf_psf_unpack_mapping(psf, &nmappings);

        /*
         * Add the mappings to the list.
         */
        for (i = 0; i < nmappings; i++) {
            gtk_list_store_append(GTK_LIST_STORE(model), &iter);
            gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                               0, mappings[i], -1);
        }
        free((char *) mappings);
    }

    gtk_entry_set_text(GTK_ENTRY(ge->ops.psf_input), "");
    gtk_widget_set_sensitive(ge->ops.psf_add, FALSE);

    /*
     * Make the "Apply" button insensitive until the user has modified
     * something.
     */
    gtk_widget_set_sensitive(ge->ops.apply, FALSE);

    ge->ops.degrees_modified = ge->ops.resize_modified =
        ge->ops.psf_modified = FALSE;
}

static void
operations_dialog_setup(GlypheditRec *ge)
{
    GtkWidget *nb, *label, *button, *hbox, *vbox, *frame, *swin;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkTreeSelection *sel;

    if (ge->ops.dialog != 0)
      return;

    /*
     * Create the pixbufs if necessary.
     */
    if (lb_image == 0)
      lb_image = gdk_pixbuf_new_from_xpm_data(lb_xpm);
    if (rb_image == 0)
      rb_image = gdk_pixbuf_new_from_xpm_data(rb_xpm);
    if (as_image == 0)
      as_image = gdk_pixbuf_new_from_xpm_data(as_xpm);
    if (ds_image == 0)
      ds_image = gdk_pixbuf_new_from_xpm_data(ds_xpm);

    ge->ops.dialog = gtk_dialog_new();
    g_signal_connect(G_OBJECT(ge->ops.dialog), "response",
                     G_CALLBACK(apply_operations),
                     GUINT_TO_POINTER(ge->id));
    /*
     * The "delete_event" handling in the dialog doesn't seem to be
     * working with GTK+ version 2.7.4.
     */
    g_signal_connect(G_OBJECT(ge->ops.dialog), "delete_event",
                     G_CALLBACK(gtk_widget_hide), 0);
    ge->ops.apply = gtk_dialog_add_button(GTK_DIALOG(ge->ops.dialog),
                                          GTK_STOCK_APPLY,
                                          GTK_RESPONSE_APPLY);
    gtk_widget_set_sensitive(ge->ops.apply, FALSE);
    button = gtk_dialog_add_button(GTK_DIALOG(ge->ops.dialog),
                                   GTK_STOCK_CLOSE,
                                   GTK_RESPONSE_CLOSE);

    nb = ge->ops.notebook = gtk_notebook_new();

    /*
     * 1. Create the rotate/shear tab.
     */
    frame = gtk_frame_new(0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

    vbox = gtk_vbox_new(FALSE, 10);

    hbox = gtk_hbox_new(FALSE, 0);
    ge->ops.rotate = gtk_radio_button_new_with_label(0, "Rotate");
    g_signal_connect(G_OBJECT(ge->ops.rotate), "toggled",
                     G_CALLBACK(set_rotate_limits), GUINT_TO_POINTER(ge->id));
    ge->ops.shear =
        gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(ge->ops.rotate),
                                        "Shear");
    g_signal_connect(G_OBJECT(ge->ops.shear), "toggled",
                     G_CALLBACK(set_shear_limits), GUINT_TO_POINTER(ge->id));

    gtk_box_pack_start(GTK_BOX(hbox), ge->ops.rotate, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), ge->ops.shear, FALSE, FALSE, 2);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    ge->ops.rotate_adj =
        (GtkAdjustment *) gtk_adjustment_new(0.0, -359.0, 359.0, 1.0,
                                             10.0, 0.0);
    /*
     * Do this so the adjustment doesn't get unref'ed out of existence
     * until we explicitly get rid of it later.
     */
    g_object_ref(G_OBJECT(ge->ops.rotate_adj));
    gtk_object_sink(GTK_OBJECT(ge->ops.rotate_adj));

    ge->ops.shear_adj =
        (GtkAdjustment *) gtk_adjustment_new(0.0, -20.0, 20.0, 1.0,
                                             5.0, 0.0);
    /*
     * Do this so the adjustment doesn't get unref'ed out of existence
     * until we explicitly get rid of it later.
     */
    g_object_ref(G_OBJECT(ge->ops.shear_adj));
    gtk_object_sink(GTK_OBJECT(ge->ops.shear_adj));

    hbox = gtk_hbox_new(FALSE, 0);
    label = gtk_label_new("Degrees:");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    ge->ops.degrees = gtk_widget_new(gtk_spin_button_get_type(),
                                     "max_length", 6,
                                     "adjustment", ge->ops.rotate_adj,
                                     "climb_rate", 1.0,
                                     "digits", 0,
                                     "value", 0.0,
                                     "numeric", TRUE,
                                     0);
    g_signal_connect(G_OBJECT(ge->ops.degrees), "changed",
                     G_CALLBACK(degrees_changed), GUINT_TO_POINTER(ge->id));

    gtk_box_pack_start(GTK_BOX(hbox), ge->ops.degrees, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 3);

    gtk_container_add(GTK_CONTAINER(frame), vbox);

    /*
     * Add the frame to a notebook page.
     */
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), frame,
                             gtk_label_new("Rotate/Shear"));

    /*
     * 2. Create the resize font bounding box tab.
     */
    vbox = gtk_vbox_new(TRUE, 0);

    frame = gtk_frame_new("Left and Right Bearing");

    hbox = gtk_hbox_new(TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

    ge->ops.lbearing = gtk_spin_button_new_with_range(0.0, 1000.0, 1.0);
    g_signal_connect(G_OBJECT(ge->ops.lbearing), "changed",
                     G_CALLBACK(resize_changed), GUINT_TO_POINTER(ge->id));
    label = labcon_new_pixbuf_defaults(lb_image, ge->ops.lbearing, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    ge->ops.rbearing = gtk_spin_button_new_with_range(0.0, 1000.0, 1.0);
    g_signal_connect(G_OBJECT(ge->ops.rbearing), "changed",
                     G_CALLBACK(resize_changed), GUINT_TO_POINTER(ge->id));
    label = labcon_new_pixbuf_defaults(rb_image, ge->ops.rbearing, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(frame), hbox);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 5);

    frame = gtk_frame_new("Ascent and Descent");

    hbox = gtk_hbox_new(TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

    ge->ops.ascent = gtk_spin_button_new_with_range(0.0, 1000.0, 1.0);
    g_signal_connect(G_OBJECT(ge->ops.ascent), "changed",
                     G_CALLBACK(resize_changed), GUINT_TO_POINTER(ge->id));
    label = labcon_new_pixbuf_defaults(as_image, ge->ops.ascent, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    ge->ops.descent = gtk_spin_button_new_with_range(0.0, 1000.0, 1.0);
    g_signal_connect(G_OBJECT(ge->ops.descent), "changed",
                     G_CALLBACK(resize_changed), GUINT_TO_POINTER(ge->id));
    label = labcon_new_pixbuf_defaults(ds_image, ge->ops.descent, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(frame), hbox);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(nb), vbox,
                             gtk_label_new("Resize BBX"));

    /*
     * 3. Create the PSF Unicode mapping tab.
     */
    vbox = gtk_vbox_new(FALSE, 0);

    swin = gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                   GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_container_set_border_width(GTK_CONTAINER(swin), 3);

    store = gtk_list_store_new(1, G_TYPE_STRING);
    ge->ops.psf_mappings =
        gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    gtk_widget_set_size_request(ge->ops.psf_mappings, 150, 150);
    gtk_container_add(GTK_CONTAINER(swin), ge->ops.psf_mappings);


    cell_renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(cell_renderer), "editable", TRUE, 0);
    g_signal_connect_object(G_OBJECT(cell_renderer), "edited",
                            G_CALLBACK(change_unimap), (gpointer) store,
                            G_CONNECT_SWAPPED);
    column = gtk_tree_view_column_new_with_attributes("Unicode Mappings",
                                                      cell_renderer,
                                                      "text", 0,
                                                      NULL);

    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ge->ops.psf_mappings), column);
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ge->ops.psf_mappings));

    gtk_box_pack_start(GTK_BOX(vbox), swin, FALSE, FALSE, 0);

    hbox = gtk_hbox_new(FALSE, 0);

    ge->ops.psf_delete = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    gtk_box_pack_end(GTK_BOX(hbox), ge->ops.psf_delete, FALSE, FALSE, 5);
    g_signal_connect(G_OBJECT(ge->ops.psf_delete), "clicked",
                     G_CALLBACK(delete_unimap), GUINT_TO_POINTER(ge->id));

    ge->ops.psf_add = gtk_button_new_from_stock(GTK_STOCK_ADD);
    gtk_box_pack_end(GTK_BOX(hbox), ge->ops.psf_add, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(ge->ops.psf_add), "clicked",
                     G_CALLBACK(add_mapping), GUINT_TO_POINTER(ge->id));

    ge->ops.psf_input = gtk_widget_new(gtk_entry_get_type(),
                                       "max_length", 8, 0);
    g_signal_connect(G_OBJECT(ge->ops.psf_input), "activate",
                     G_CALLBACK(add_mapping), GUINT_TO_POINTER(ge->id));
    g_signal_connect(G_OBJECT(ge->ops.psf_input), "changed",
                     G_CALLBACK(enable_add), GUINT_TO_POINTER(ge->id));
    gtk_box_pack_end(GTK_BOX(hbox), ge->ops.psf_input, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(nb), vbox,
                             gtk_label_new("PSF Unicode Mappings"));

    ge->unimap_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb), 2);
    if (glyphedit_get_spacing(GLYPHEDIT(ge->gedit)) != BDF_PROPORTIONAL)
      gtk_widget_set_sensitive(ge->unimap_page, TRUE);
    else
      gtk_widget_set_sensitive(ge->unimap_page, FALSE);

    /*
     * 4. Add the notebook to the dialog.
     */
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(ge->ops.dialog)->vbox), nb);

    gtk_window_set_transient_for(GTK_WINDOW(ge->ops.dialog),
                                 GTK_WINDOW(ge->shell));
    gtk_widget_show_all(GTK_DIALOG(ge->ops.dialog)->vbox);
    gtk_widget_show_all(GTK_DIALOG(ge->ops.dialog)->action_area);
}

static void
show_rotate_dialog(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    operations_dialog_setup(ge);
    operations_dialog_populate(ge);

    /*
     * Make sure we turn to the first notebook page.
     */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ge->ops.notebook), 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ge->ops.rotate), TRUE);

    /*
     * Move the focus to the spin box.
     */
    gtk_widget_grab_focus(ge->ops.degrees);

    gtk_widget_show(ge->ops.dialog);
}

static void
show_shear_dialog(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    operations_dialog_setup(ge);
    operations_dialog_populate(ge);

    /*
     * Make sure we turn to the first notebook page.
     */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ge->ops.notebook), 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ge->ops.shear), TRUE);

    /*
     * Move the focus to the spin box.
     */
    gtk_widget_grab_focus(ge->ops.degrees);

    gtk_widget_show(ge->ops.dialog);
}

static void
show_resize_dialog(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    operations_dialog_setup(ge);
    operations_dialog_populate(ge);

    /*
     * Make sure we turn to the first notebook page.
     */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ge->ops.notebook), 1);

    /*
     * Move the focus to the first sensitive spin box.
     */
    if (GTK_WIDGET_SENSITIVE(ge->ops.lbearing))
      gtk_widget_grab_focus(ge->ops.lbearing);
    else
      gtk_widget_grab_focus(ge->ops.rbearing);

    gtk_widget_show(ge->ops.dialog);
}

static void
embolden_glyph(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    glyphedit_embolden_glyph(GLYPHEDIT(ge->gedit));
}

static void
show_unimap_dialog(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);

    operations_dialog_setup(ge);
    operations_dialog_populate(ge);

    gtk_notebook_set_current_page(GTK_NOTEBOOK(ge->ops.notebook), 2);

    /*
     * Move the focus to the input field.
     */
    gtk_widget_grab_focus(ge->ops.psf_input);

    gtk_widget_show(ge->ops.dialog);
}

static GtkWidget *
make_ops_menu(GlypheditRec *ge, GtkWidget *menubar)
{
    GtkWidget *ops, *menu, *mitem, *sep;

    /*
     * Create the Operations menu.
     */
    ops = gtk_menu_item_new_with_mnemonic("_Operations");

    ge->ops_menu = menu = gtk_menu_new();
    g_signal_connect(G_OBJECT(menu), "map_event",
                     G_CALLBACK(operations_menu_up), GUINT_TO_POINTER(ge->id));
    g_signal_connect(G_OBJECT(menu), "unmap_event",
                     G_CALLBACK(operations_menu_down),
                     GUINT_TO_POINTER(ge->id));

    mitem = make_accel_menu_item(menu, "_Draw",
                                 "<Control>D", ge->ag);
    g_signal_connect(G_OBJECT(mitem), "activate",
                     G_CALLBACK(draw_operation),
                     GUINT_TO_POINTER(ge->id));

    mitem = make_accel_menu_item(menu, "_Move",
                                 "<Control>M", ge->ag);
    g_signal_connect(G_OBJECT(mitem), "activate",
                     G_CALLBACK(move_operation),
                     GUINT_TO_POINTER(ge->id));

    mitem = make_accel_menu_item(menu, "_Copy",
                                 "<Control>Y", ge->ag);
    g_signal_connect(G_OBJECT(mitem), "activate",
                     G_CALLBACK(copy_operation),
                     GUINT_TO_POINTER(ge->id));

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    mitem = make_accel_menu_item(menu, "_Rotate",
                                 "<Control>T", ge->ag);
    g_signal_connect(G_OBJECT(mitem), "activate",
                     G_CALLBACK(show_rotate_dialog),
                     GUINT_TO_POINTER(ge->id));

    mitem = make_accel_menu_item(menu, "_Shear",
                                 "<Control>E", ge->ag);
    g_signal_connect(G_OBJECT(mitem), "activate",
                     G_CALLBACK(show_shear_dialog),
                     GUINT_TO_POINTER(ge->id));

    mitem = make_accel_menu_item(menu, "_Embolden",
                                 "<Control>H", ge->ag);
    g_signal_connect(G_OBJECT(mitem), "activate",
                     G_CALLBACK(embolden_glyph),
                     GUINT_TO_POINTER(ge->id));

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ops), menu);

    /*
     * Separator.
     */
    sep = gtk_menu_item_new();
    gtk_widget_set_sensitive(sep, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);

    ge->resize = make_accel_menu_item(menu, "_Resize BBX",
                                           "<Control>R", ge->ag);
    g_signal_connect(G_OBJECT(ge->resize), "activate",
                     G_CALLBACK(show_resize_dialog),
                     GUINT_TO_POINTER(ge->id));

    ge->unimap = make_accel_menu_item(menu, "Edit PSF Unicode _Mappings",
                                      "<Control>F", ge->ag);
    g_signal_connect(G_OBJECT(ge->unimap), "activate",
                     G_CALLBACK(show_unimap_dialog),
                     GUINT_TO_POINTER(ge->id));

    return ops;
}

static void
pointer_moved(GtkWidget *w, gpointer cb, gpointer ged)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(ged);
    GlypheditSignalInfo *si = (GlypheditSignalInfo *) cb;
    bdf_glyph_grid_t *g;

    g = glyphedit_get_grid(GLYPHEDIT(ge->gedit));
    if (g->bpp == 1 || si->color == 0)
      sprintf(buffer1, "(%d,%d)", si->x, si->y);
    else {
        switch (g->bpp) {
          case 2:
            sprintf(buffer1, "(%d,%d,%d)", si->x, si->y,
                    options.colors[si->color-1]);
            break;
          case 4:
            sprintf(buffer1, "(%d,%d,%d)", si->x, si->y,
                    options.colors[si->color+4-1]);
            break;
          case 8:
            sprintf(buffer1, "(%d,%d,%d)", si->x, si->y, si->color);
            break;
        }
    }

    gtk_label_set_text(GTK_LABEL(ge->coords), buffer1);
}

/*
 * Under certain circumstances, the glyphedit widget causes the operation to
 * change. Basically, when a bitmap is pasted, the widget goes into a MOVE
 * operation. All the operations are handled here just in case of future
 * changes.
 */
static void
operation_changed(GtkWidget *w, gpointer cb, gpointer ged)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(ged);
    GlypheditSignalInfo *si = (GlypheditSignalInfo *) cb;

    if (si->operation == GLYPHEDIT_DRAW)
      gecontrol_change_operation(GECONTROL(ge->gectrl), GECONTROL_DRAW);
    else if (si->operation == GLYPHEDIT_MOVE)
      gecontrol_change_operation(GECONTROL(ge->gectrl), GECONTROL_MOVE);
    else if (si->operation == GLYPHEDIT_COPY)
      gecontrol_change_operation(GECONTROL(ge->gectrl), GECONTROL_COPY);
}

static void
color_changed(GtkWidget *w, gpointer cb, gpointer ged)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(ged);
    GlypheditSignalInfo *si = (GlypheditSignalInfo *) cb;

    gecontrol_change_color(GECONTROL(ge->gectrl), si->color);
}

static void
glyph_modified(GtkWidget *w, gpointer cb, gpointer ged)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(ged);
    gbdfed_editor_t *ed = editors + ge->owner;
    GlypheditSignalInfo *si = (GlypheditSignalInfo *) cb;
    gchar *prgname = g_get_prgname();

    if (si->metrics == 0)
      return;

    if (ed->file == 0)
      sprintf(buffer1, "%s - Glyph Edit: (unnamed%d) [modified]",
              prgname, ed->id);
    else
      sprintf(buffer1, "%s - Glyph Edit: %s [modified]", prgname, ed->file);

    gtk_window_set_title(GTK_WINDOW(ge->shell), buffer1);

    sprintf(buffer1, "width %hd height %hd\nascent %hd descent %hd",
            si->metrics->width, si->metrics->height,
            si->metrics->ascent, si->metrics->descent);
    gtk_label_set_text(GTK_LABEL(ge->metrics), buffer1);

    if (si->metrics->font_spacing == BDF_PROPORTIONAL) {
        sprintf(buffer1, "%hd", si->metrics->dwidth);
        g_signal_handler_block(G_OBJECT(ge->dwidth), ge->handler);
        gtk_entry_set_text(GTK_ENTRY(ge->dwidth), buffer1);
        g_signal_handler_unblock(G_OBJECT(ge->dwidth), ge->handler);
    }

    /*
     * Update the glyph image on the Glyphedit control widget.
     */
    gecontrol_update_glyph_image(GECONTROL(ge->gectrl), si->image);

    gtk_widget_set_sensitive(ge->update, TRUE);
    gtk_widget_set_sensitive(ge->update_next, TRUE);
    gtk_widget_set_sensitive(ge->update_prev, TRUE);
}

/*
 * This function will be a bit screwy until I can figure out how to make a
 * signal pass an enum as the first param after the widget.
 */
static void
gectrl_activate(GtkWidget *w, gpointer info, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    GEControlActivateInfo *ai = (GEControlActivateInfo *) info;

    switch (ai->operation) {
      case GECONTROL_DRAW:
        glyphedit_change_operation(GLYPHEDIT(ge->gedit), GLYPHEDIT_DRAW);
        break;
      case GECONTROL_MOVE:
        glyphedit_change_operation(GLYPHEDIT(ge->gedit), GLYPHEDIT_MOVE);
        break;
      case GECONTROL_COPY:
        glyphedit_change_operation(GLYPHEDIT(ge->gedit), GLYPHEDIT_COPY);
        break;
      case GECONTROL_FLIP_HORIZONTAL:
        glyphedit_flip_glyph(GLYPHEDIT(ge->gedit), GTK_ORIENTATION_HORIZONTAL);
        break;
      case GECONTROL_FLIP_VERTICAL:
        glyphedit_flip_glyph(GLYPHEDIT(ge->gedit), GTK_ORIENTATION_VERTICAL);
        break;
      case GECONTROL_SHEAR:
        show_shear_dialog(w, data);
        break;
      case GECONTROL_ROTATE_LEFT_90:
        glyphedit_rotate_glyph(GLYPHEDIT(ge->gedit), -90);
        break;
      case GECONTROL_ROTATE_RIGHT_90:
        glyphedit_rotate_glyph(GLYPHEDIT(ge->gedit), 90);
        break;
      case GECONTROL_ROTATE:
        show_rotate_dialog(w, data);
        break;
      case GECONTROL_SHIFT_UP_LEFT:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), -1, -1);
        break;
      case GECONTROL_SHIFT_UP:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), 0, -1);
        break;
      case GECONTROL_SHIFT_UP_RIGHT:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), 1, -1);
        break;
      case GECONTROL_SHIFT_LEFT:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), -1, 0);
        break;
      case GECONTROL_SHIFT_RIGHT:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), 1, 0);
        break;
      case GECONTROL_SHIFT_DOWN_LEFT:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), -1, 1);
        break;
      case GECONTROL_SHIFT_DOWN:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), 0, 1);
        break;
      case GECONTROL_SHIFT_DOWN_RIGHT:
        glyphedit_shift_glyph(GLYPHEDIT(ge->gedit), 1, 1);
        break;
      case GECONTROL_COLOR_CHANGE:
        glyphedit_set_color(GLYPHEDIT(ge->gedit), ai->color);
        break;
    }
}

/*
 * This is called when the device width field is changed in any way.
 */
static void
enable_update(GtkWidget *w, gpointer data)
{
    GlypheditRec *ge = glyph_editors + GPOINTER_TO_UINT(data);
    gbdfed_editor_t *ed = editors + ge->owner;

    if (ed->file == 0)
      sprintf(buffer1, "%s - Glyph Edit: (unnamed%d) [modified]",
              g_get_prgname(), ed->id);
    else
      sprintf(buffer1, "%s - Glyph Edit: %s [modified]",
              g_get_prgname(), ed->file);

    gtk_window_set_title(GTK_WINDOW(ge->shell), buffer1);
    gtk_widget_set_sensitive(ge->update, TRUE);
    gtk_widget_set_sensitive(ge->update_next, TRUE);
    gtk_widget_set_sensitive(ge->update_prev, TRUE);
}

static void
_guigedit_build_editor(GlypheditRec *ge, bdf_glyph_grid_t *grid, guint base,
                       gbdfed_editor_t *ed)
{
    GtkWidget *mb, *mitem, *frame, *vbox, *vbox1, *hbox;
    bdf_bitmap_t image;

    if (ed->file == 0)
      sprintf(buffer1, "%s - Glyph Edit: (unnamed%d)", g_get_prgname(),
              ed->id);
    else
      sprintf(buffer1, "%s - Glyph Edit: %s", g_get_prgname(), ed->file);

    ge->shell = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(ge->shell), buffer1);

    gtk_window_set_resizable(GTK_WINDOW(ge->shell), TRUE);

    (void) g_signal_connect(G_OBJECT(ge->shell), "destroy_event",
                            G_CALLBACK(close_glyph_editor),
                            GUINT_TO_POINTER(ge->id));
    (void) g_signal_connect(G_OBJECT(ge->shell), "delete_event",
                            G_CALLBACK(close_glyph_editor),
                            GUINT_TO_POINTER(ge->id));

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(ge->shell), vbox);

    ge->ag = gtk_accel_group_new();

    mb = gtk_menu_bar_new();
    mitem = make_file_menu(ge, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    mitem = make_edit_menu(ge, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    mitem = make_ops_menu(ge, mb);
    gtk_menu_shell_append(GTK_MENU_SHELL(mb), mitem);

    gtk_box_pack_start(GTK_BOX(vbox), mb, FALSE, TRUE, 0);

    /*
     * Attach the accelerators to the editor.
     */
    gtk_window_add_accel_group(GTK_WINDOW(ge->shell), ge->ag);

    /*
     * 1. Add the glyph name and encoding widgets.
     */
    frame = gtk_frame_new(0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);

    vbox1 = gtk_vbox_new(TRUE, 0);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    ge->name = gtk_widget_new(gtk_entry_get_type(),
                              "max_length", 128, 0);
    mitem = labcon_new_label_defaults("Glyph Name:", ge->name, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), mitem, TRUE, TRUE, 0);

    ge->encoding = gtk_label_new("0000");
    gtk_misc_set_alignment(GTK_MISC(ge->encoding), 0.0, 0.5);
    mitem = labcon_new_label_defaults("Encoding:", ge->encoding, mitem);
    gtk_box_pack_start(GTK_BOX(vbox1), mitem, TRUE, TRUE, 0);

    /*
     * 2. Add the device width and metrics widgets.
     */
    frame = gtk_frame_new(0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);

    vbox1 = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    ge->dwidth = gtk_widget_new(gtk_entry_get_type(),
                                "max_length", 6,
                                0);
    ge->handler = g_signal_connect(G_OBJECT(ge->dwidth), "changed",
                                   G_CALLBACK(enable_update),
                                   GUINT_TO_POINTER(ge->id));
    mitem = labcon_new_label_defaults("Device Width:", ge->dwidth, mitem);
    gtk_box_pack_start(GTK_BOX(vbox1), mitem, TRUE, TRUE, 0);

    ge->metrics = gtk_label_new("width 0 height 0\r\nascent 0 descent 0");
    gtk_misc_set_alignment(GTK_MISC(ge->metrics), 0.0, 0.5);
    mitem = labcon_new_label_defaults("Metrics:", ge->metrics, mitem);
    gtk_box_pack_start(GTK_BOX(vbox1), mitem, TRUE, TRUE, 0);

    hbox = gtk_hbox_new(FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, 0);

    /*
     * Create the coordinates label.
     */
    ge->coords = gtk_label_new("(0,0)");
    gtk_misc_set_alignment(GTK_MISC(ge->coords), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox1), ge->coords, FALSE, TRUE, 0);

    /*
     * Create the glyph editor.
     */
    ge->gedit = glyphedit_newv(grid, options.pixel_size, options.show_x_height,
                               options.show_cap_height, options.colors);
    g_signal_connect(G_OBJECT(ge->gedit), "glyph-modified",
                     G_CALLBACK(glyph_modified), GUINT_TO_POINTER(ge->id));
    g_signal_connect(G_OBJECT(ge->gedit), "pointer-moved",
                     G_CALLBACK(pointer_moved), GUINT_TO_POINTER(ge->id));
    g_signal_connect(G_OBJECT(ge->gedit), "operation-change",
                     G_CALLBACK(operation_changed), GUINT_TO_POINTER(ge->id));
    g_signal_connect(G_OBJECT(ge->gedit), "color-change",
                     G_CALLBACK(color_changed), GUINT_TO_POINTER(ge->id));
    gtk_box_pack_start(GTK_BOX(vbox1), ge->gedit, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 0);

    vbox1 = gtk_vbox_new(FALSE, 0);

    ge->gectltips = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(ge->gectltips), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox1), ge->gectltips, FALSE, TRUE, 0);

    /*
     * Get the initial glyph image.
     */
    glyphedit_get_image(GLYPHEDIT(ge->gedit), &image);
    ge->gectrl = gecontrol_newv(ge->gectltips, &image, options.colors);
    if (image.bytes > 0)
      free(image.bitmap);
    g_signal_connect(G_OBJECT(ge->gectrl), "activate",
                     G_CALLBACK(gectrl_activate), GUINT_TO_POINTER(ge->id));

    gtk_box_pack_start(GTK_BOX(vbox1), ge->gectrl, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
}

void
guigedit_edit_glyph(gbdfed_editor_t *ed, FontgridSelectionInfo *si)
{
    GlypheditRec *ge;
    bdf_font_t *font;
    guint base;
    bdf_glyph_grid_t *grid;
    bdf_bitmap_t image;

    font = fontgrid_get_font(FONTGRID(ed->fgrid));
    base = fontgrid_get_code_base(FONTGRID(ed->fgrid));

    if (si->unencoded)
      grid = bdf_make_glyph_grid(font, si->start, 1);
    else
      grid = bdf_make_glyph_grid(font, si->start, 0);

    ge = _guigedit_get_glyph_editor(ed->id);

    if (ge->name == 0) {
        _guigedit_build_editor(ge, grid, base, ed);
    } else {
        if (ed->file == 0)
          sprintf(buffer1, "%s - Glyph Edit: (unnamed%d)", g_get_prgname(),
                  ed->id);
        else
          sprintf(buffer1, "%s - Glyph Edit: %s", g_get_prgname(), ed->file);

        gtk_window_set_title(GTK_WINDOW(ge->shell), buffer1);
        glyphedit_set_grid(GLYPHEDIT(ge->gedit), grid);

        /*
         * Update the image in the glypheditor control panel.
         */
        if (grid) {
            bdf_grid_image(grid, &image);
            gecontrol_set_glyph_image(GECONTROL(ge->gectrl), &image);
            if (image.bytes > 0)
              free(image.bitmap);
        } else
          gecontrol_set_glyph_image(GECONTROL(ge->gectrl), 0);

        /*
         * Make sure the control has the most current list of colors.
         */
        gecontrol_set_color_list(GECONTROL(ge->gectrl), options.colors);
    }

    /*
     * Update the text fields and labels with the glyph info.
     */
    gtk_entry_set_text(GTK_ENTRY(ge->name), grid->name);
    if (grid->unencoded)
      sprintf(buffer1, "-1");
    else {
        switch (base) {
          case 8: sprintf(buffer1, "%lo", grid->encoding); break;
          case 10: sprintf(buffer1, "%ld", grid->encoding); break;
          case 16: sprintf(buffer1, "%04lX", grid->encoding); break;
        }
    }
    gtk_label_set_text(GTK_LABEL(ge->encoding), buffer1);

    gtk_widget_set_sensitive(ge->dwidth, TRUE);
    sprintf(buffer1, "%hd", grid->dwidth);

    g_signal_handler_block(G_OBJECT(ge->dwidth), ge->handler);
    gtk_entry_set_text(GTK_ENTRY(ge->dwidth), buffer1);
    g_signal_handler_unblock(G_OBJECT(ge->dwidth), ge->handler);

    if (grid->spacing != BDF_PROPORTIONAL)
      gtk_widget_set_sensitive(ge->dwidth, FALSE);

    sprintf(buffer1, "width %hd height %hd\r\nascent %hd descent %hd",
            grid->glyph_bbx.width, grid->glyph_bbx.height,
            grid->glyph_bbx.ascent, grid->glyph_bbx.descent);
    gtk_label_set_text(GTK_LABEL(ge->metrics), buffer1);

    if (grid->modified) {
        gtk_widget_set_sensitive(ge->update, TRUE);
        gtk_widget_set_sensitive(ge->update_next, TRUE);
        gtk_widget_set_sensitive(ge->update_prev, TRUE);
    } else {
        gtk_widget_set_sensitive(ge->update, FALSE);
        gtk_widget_set_sensitive(ge->update_next, FALSE);
        gtk_widget_set_sensitive(ge->update_prev, FALSE);
    }

    gtk_widget_show_all(ge->shell);

    /*
     * Force the focus to be on the glyph grid.
     */
    gtk_widget_grab_focus(ge->gedit);
}

/*
 * Routine to set the show_cap_height value for all the glyph editors.
 */
void
guigedit_show_cap_height(gboolean show)
{
    gulong i;

    for (i = 0; i < num_glyph_editors; i++) {
        if (glyph_editors[i].owner != ~0)
          glyphedit_set_show_cap_height(GLYPHEDIT(glyph_editors[i].gedit),
                                        show);
    }
}

/*
 * Routine to set the show_cap_height value for all the glyph editors.
 */
void
guigedit_show_x_height(gboolean show)
{
    gulong i;

    for (i = 0; i < num_glyph_editors; i++) {
        if (glyph_editors[i].owner != ~0)
          glyphedit_set_show_x_height(GLYPHEDIT(glyph_editors[i].gedit), show);
    }
}

/*
 * Routine to set the pixel size on all of the visible editors.
 */
void
guigedit_set_pixel_size(guint pixel_size)
{
    gulong i;

    for (i = 0; i < num_glyph_editors; i++) {
        if (glyph_editors[i].owner != ~0)
          glyphedit_set_pixel_size(GLYPHEDIT(glyph_editors[i].gedit),
                                   pixel_size);
    }
}

/*
 * Routine to set the spacing and device width on all of the visible editors.
 */
void
guigedit_set_font_spacing(gint spacing, guint16 monowidth)
{
    gulong i;

    for (i = 0; i < num_glyph_editors; i++) {
        if (glyph_editors[i].owner != ~0)
          glyphedit_set_spacing(GLYPHEDIT(glyph_editors[i].gedit), spacing,
                                monowidth);
    }
}

/*
 * Routine to set the code base on all the glyph editors that are active.
 */
void
guigedit_set_code_base(gint base)
{
    gulong i, enc;

    for (i = 0; i < num_glyph_editors; i++) {
        if (glyph_editors[i].owner != ~0) {
            enc = (gulong) glyphedit_get_encoding(GLYPHEDIT(glyph_editors[i].gedit));
            switch (base) {
              case 8:
                sprintf(buffer1, "%lo", enc);
                break;
              case 10:
                sprintf(buffer1, "%ld", enc);
                break;
              case 16:
                sprintf(buffer1, "%04lX", enc);
                break;
            }
            gtk_label_set_text(GTK_LABEL(glyph_editors[i].encoding), buffer1);
        }
    }
}

/*
 * Routine to clean up everything that was allocated here.
 */
void
guigedit_cleanup(void)
{
    gulong i;

    /*
     * Cycle through all the glyph editors and check if they have been
     * modified and if they need to be saved.
     */
    for (i = 0; i < num_glyph_editors; i++) {
        if (glyph_editors[i].id != ~0)
          close_glyph_editor(0, 0, GUINT_TO_POINTER(i));
    }

    /*
     * Unreference all the pixbufs so they go away.
     */
    if (lb_image != 0)
      g_object_unref(G_OBJECT(lb_image));
    if (rb_image != 0)
      g_object_unref(G_OBJECT(rb_image));
    if (as_image != 0)
      g_object_unref(G_OBJECT(as_image));
    if (ds_image != 0)
      g_object_unref(G_OBJECT(ds_image));

    lb_image = rb_image = as_image = ds_image = 0;

    /*
     * GTK will take care of the widgets in the glyph editors.
     */
    if (num_glyph_editors > 0)
      g_free(glyph_editors);
}

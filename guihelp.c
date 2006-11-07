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
static char svnid[] __attribute__ ((unused)) = "$Id: guihelp.c 1 2006-11-02 16:47:28Z mleisher $";
#else
static char svnid[] = "$Id: guihelp.c 1 2006-11-02 16:47:28Z mleisher $";
#endif
#endif

#include "gbdfed.h"
#include "htext.h"

static GtkWidget *help_dialog;
static GtkWidget *help_text;
static GtkWidget *help_topics;

typedef struct {
    int mnemonic;
    gchar *help_topic;
    gchar *help_text;
} helpmap_t;

static helpmap_t topics[] = {
    {'A', "About",               0},
    {'P', "The Program",         0},
    {'F', "Font Grid",           0},
    {'G', "Glyph Editor",        0},
    {'C', "Configuration File",  0},
    {'r', "Preferences Dialog",  0},
    {'W', "Windows Font Notes",  0},
    {'O', "OpenType Font Notes", 0},
    {'P', "PSF Font Notes",      0},
    {'H', "HEX Font Notes",      0},
    {'o', "Color Notes",         0},
    {'i', "Tips",                0},
};

static guint ntopics = sizeof(topics) / sizeof(topics[0]);

/**************************************************************
 *
 * Routines for parsing the markup for the help text buffer.
 *
 **************************************************************/

#define HTEXT_BULLET 0x0001
#define HTEXT_UL     0x0002

typedef struct {
    guint flags;
    GtkTextBuffer *text;
    const gchar *tag_name;
    GtkTextIter iter;
} htext_parse_t;

static htext_parse_t hp;

static const gchar *bullet = "•";

void
help_parse_start(GMarkupParseContext *ctx, const gchar *tag,
                 const gchar **attr_names, const gchar **attr_vals,
                 gpointer data, GError **err)
{
    if (strcmp(tag, "bullet") == 0) {
        gtk_text_buffer_insert_with_tags_by_name(hp.text, &hp.iter,
                                                 bullet, 3,
                                                 "margin",
                                                 "large_bullet", NULL);
        hp.flags |= HTEXT_BULLET;
        hp.tag_name = 0;
    } else if (strcmp(tag, "help") == 0)
      hp.tag_name = 0;
    else
      hp.tag_name = tag;
}

void
help_parse_text(GMarkupParseContext *ctx, const gchar *txt, gsize txtlen,
                gpointer data, GError **err)
{
    if (hp.tag_name != 0)
      gtk_text_buffer_insert_with_tags_by_name(hp.text, &hp.iter,
                                               txt, txtlen,
                                               hp.tag_name, NULL);
    else {
        if (hp.flags & HTEXT_BULLET)
          gtk_text_buffer_insert_with_tags_by_name(hp.text, &hp.iter,
                                                   txt, txtlen,
                                                   "tabs", NULL);
        else
          /*
           * Plain text insert.
           */
          gtk_text_buffer_insert(hp.text, &hp.iter, txt, txtlen);
    }
}

void
help_parse_end(GMarkupParseContext *ctx, const gchar *tag,
               gpointer data, GError **err)
{
    if (strcmp(tag, "bullet") == 0)
      hp.flags &= ~HTEXT_BULLET;
    hp.tag_name = 0;
}

void
help_parse_error(GMarkupParseContext *ctx, GError *err, gpointer data)
{
}

static GMarkupParser markup_funcs = {
    help_parse_start,
    help_parse_end,
    help_parse_text,
    0,
    help_parse_error,
};

static GMarkupParseContext *markup_context = NULL;

/*
 * Creates all the markup tags.
 */
static void
help_init_markup(void)
{
    PangoTabArray *tabs;

    gtk_text_buffer_create_tag(hp.text, "center",
                               "justification", GTK_JUSTIFY_CENTER, NULL);
    gtk_text_buffer_create_tag(hp.text, "margin",
                               "left_margin", 20, NULL);
    gtk_text_buffer_create_tag(hp.text, "margin1",
                               "left_margin", 30, NULL);
    gtk_text_buffer_create_tag(hp.text, "margin2",
                               "left_margin", 45, NULL);
    gtk_text_buffer_create_tag(hp.text, "margin1.5",
                               "left_margin", 30,
                               "weight", PANGO_WEIGHT_BOLD,
                               NULL);
    gtk_text_buffer_create_tag(hp.text, "margin3",
                               "left_margin", 60, NULL);
    gtk_text_buffer_create_tag(hp.text, "large_bullet",
                               "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag(hp.text, "param",
                               "weight", PANGO_WEIGHT_BOLD,
                               "left_margin", 10, NULL);
    gtk_text_buffer_create_tag(hp.text, "ul",
                               "underline", PANGO_UNDERLINE_SINGLE, NULL);
    gtk_text_buffer_create_tag(hp.text, "bul",
                               "underline", PANGO_UNDERLINE_SINGLE,
                               "weight", PANGO_WEIGHT_BOLD, NULL);

    tabs = pango_tab_array_new_with_positions(2, TRUE,
                                              PANGO_TAB_LEFT, 50,
                                              PANGO_TAB_LEFT, 70);
    gtk_text_buffer_create_tag(hp.text, "tabs",
                               "tabs", tabs,
                               "left_margin", 10,
                               NULL);

    gtk_text_buffer_create_tag(hp.text, "i",
                               "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_buffer_create_tag(hp.text, "b",
                               "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(hp.text, "bi",
                               "style", PANGO_STYLE_ITALIC,
                               "weight", PANGO_WEIGHT_BOLD, NULL);
}

/**************************************************************
 *
 * API functions.
 *
 **************************************************************/

static void
help_free_list_item(gpointer data, gpointer user_data)
{
    gtk_tree_path_free((GtkTreePath *) data);
}

static void
help_on(GtkTreeSelection *sel, gpointer data)
{
    GList *rowlist;
    gint row = 1, *rows;
    GError *err = 0;

    /*
     * Because this is called before the help text widget is created,
     * simply return if the help text widget is NULL.
     */
    if (help_text == 0)
      return;

    /*
     * Sorta complicated, but can't see any other way around it at the moment
     * with gtk 2.2.4.
     */
    rowlist = gtk_tree_selection_get_selected_rows(sel, NULL);
    rows =
        gtk_tree_path_get_indices((GtkTreePath *) g_list_nth_data(rowlist, 0));
    row = rows[0];

    /*
     * Clear out the rows that were returned.
     */
    g_list_foreach(rowlist, help_free_list_item, 0);

    /*
     * Set the dialog title.
     */
    sprintf(buffer1, "Help: %s", topics[row].help_topic);
    gtk_window_set_title(GTK_WINDOW(help_dialog), buffer1);

    /*
     * Clear the text buffer.
     */
    gtk_text_buffer_set_text(hp.text, "", 0);

    /*
     * Get the iterator at the start of the buffer.
     */
    gtk_text_buffer_get_start_iter(hp.text, &hp.iter);

    /*
     * Parse the help text.
     */
    if (g_markup_parse_context_parse(markup_context,
                                     topics[row].help_text, -1, &err) == FALSE)
      fprintf(stderr, "guihelp.c:%s\n", err->message);
}

void
guihelp_show_help(GtkWidget *w, gpointer data)
{
    guint i;
    GtkWidget *hbox, *sw, *button;
    GtkTextBuffer *text;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    GtkTreeSelection *sel;
    GtkTreePath *tpath;
    GtkTreeIter iter;

    if (help_dialog == 0) {
        /*
         * Store the help text into the array.
         */
        topics[GUIHELP_ABOUT].help_text = about_text;
        topics[GUIHELP_PROGRAM].help_text = program_text;
        topics[GUIHELP_FONTGRID].help_text = fgrid_text;
        topics[GUIHELP_GLYPH_EDITOR].help_text = gedit_text;
        topics[GUIHELP_CONFIG_FILE].help_text = conf_text;
        topics[GUIHELP_PREFERENCES].help_text = preferences_text;
        topics[GUIHELP_FNT].help_text = fnt_text;
        topics[GUIHELP_OTF].help_text = otf_text;
        topics[GUIHELP_PSF].help_text = psf_text;
        topics[GUIHELP_HEX].help_text = hex_text;
        topics[GUIHELP_COLOR].help_text = color_text;
        topics[GUIHELP_TIPS].help_text = tips_text;

        help_dialog = gtk_dialog_new();

        /*
         * Force the help dialog to center over the first editor that
         * was created.
         */
        (void) g_signal_connect(G_OBJECT(help_dialog), "delete_event",
                                G_CALLBACK(gtk_widget_hide), NULL);

        hbox = gtk_hbox_new(FALSE, 5);

        /*
         * Create the list that will be used for the help topics.
         */
        store = gtk_list_store_new(1, G_TYPE_STRING);

        /*
         * Add the topics to the list.
         */
        for (i = 0; i < ntopics; i++) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, topics[i].help_topic, -1);
        }
        help_topics = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

        cell_renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes ("Help Topics",
                                                           cell_renderer,
                                                           "text", 0,
                                                           NULL);
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW(help_topics), column);

        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(help_topics));
        gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);

        (void) g_signal_connect(G_OBJECT(sel), "changed",
                                G_CALLBACK(help_on), NULL);

        gtk_box_pack_start(GTK_BOX(hbox), help_topics, FALSE, FALSE, 0);

        g_object_unref(store);

        gtk_box_pack_start(GTK_BOX(hbox), gtk_vseparator_new(),
                           FALSE, FALSE, 0);

        /*
         * Create the text widget that will display the help text.
         */
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_ALWAYS);

        text = gtk_text_buffer_new(NULL);
        help_text = gtk_text_view_new_with_buffer(text);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(help_text), GTK_WRAP_WORD);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(help_text), FALSE);
        gtk_widget_set_size_request(help_text, 550, 300);

        gtk_container_add(GTK_CONTAINER(sw), help_text);

        gtk_box_pack_start(GTK_BOX(hbox), sw, TRUE, TRUE, 0);

        /*
         * Add the table to the dialog widget.
         */
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(help_dialog)->vbox),
                          hbox);

        button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
        (void) g_signal_connect_object(G_OBJECT(button), "clicked",
                                       G_CALLBACK(gtk_widget_hide),
                                       (gpointer) help_dialog,
                                       G_CONNECT_SWAPPED);

        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(help_dialog)->action_area),
                          button);
        gtk_widget_show_all(GTK_DIALOG(help_dialog)->vbox);
        gtk_widget_show_all(GTK_DIALOG(help_dialog)->action_area);

        /*
         * Create all the markup tags the text buffer will use.
         */
        hp.flags = 0;
        hp.text = text;
        hp.tag_name = "";
        help_init_markup();

        /*
         * Create the context for the parser.
         */
        markup_context = g_markup_parse_context_new(&markup_funcs, 0,
                                                    NULL, NULL);
    }

    /*
     * Select the row specified in the callback.
     */
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(help_topics));
    tpath = gtk_tree_path_new_from_indices(GPOINTER_TO_INT(data), -1);
    gtk_tree_selection_select_path(sel, tpath);

    /*
     * Show the help dialog and force it to the top.
     */
    guiutil_show_dialog_centered(help_dialog, editors[0].shell);
    gdk_window_raise(help_dialog->window);
}

void
guihelp_cleanup(void)
{
    if (markup_context != NULL)
      g_markup_parse_context_free(markup_context);
    markup_context = NULL;
}

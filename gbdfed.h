#ifndef _h_gbdfed
#define _h_gbdfed

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "bdf.h"

G_BEGIN_DECLS

#define GBDFED_VERSION "1.6"

/*************************************************************************
 *
 * Types.
 *
 *************************************************************************/

typedef struct {
    gboolean gbdfed_opts;

    gchar *accelerator;
    gchar *accelerator_text;
    gchar *unicode_name_file;
    gchar *adobe_name_file;
    gboolean no_blanks;
    gboolean really_exit;
    gboolean overwrite_mode;
    gint initial_glyph;
    unsigned int pixel_size;
    unsigned int resolution;
    gboolean show_cap_height;
    gboolean show_x_height;
#if 0
    gboolean vertical;
#endif
    gboolean power2;
#if 0
    gboolean colors_allocated;
#endif
    unsigned short colors[20];
#if 0
    guint32 pixels[20];
#endif
    gboolean sbit;
    gboolean backups;
    guint code_base;
    GtkOrientation orientation;
    bdf_options_t font_opts;
} gbdfed_options_t;

typedef struct {
    /*
     * Numeric ID for the editor.
     */
    guint id;

    /*
     * Path and filenames of the font in question.
     */
    gchar *path;
    gchar *file;

    GtkAccelGroup *ag;
    GtkWidget *shell;

    /*
     * The Open/Save file selection dialogs that will be used for related
     * operations as well as the original.
     */
    GtkWidget *open_dialog;
    GtkWidget *save_dialog;

    /*
     * The menus that need to have item sensitivity checked on popup and
     * popdown.
     */
    GtkWidget *file_menu;
    GtkWidget *edit_menu;
    GtkWidget *name_submenu;
    GtkWidget *view_menu;
    GtkWidget *ops_menu;
    GtkWidget *win_menu;

    /*
     * This editor's item on the Windows menu.  Used to toggle sensitivity.
     */
    GtkWidget *win_menu_item;

    /*
     * The font name field.
     */
    GtkWidget *fontname;

    /*
     * The encoding and metrics labels.
     */
    GtkWidget *charinfo;
    GtkWidget *metrics;

    /*
     * The paging buttons and page tracking variables.
     */
    GtkWidget *first;
    GtkWidget *prev;
    GtkWidget *next;
    GtkWidget *last;
    gint32 last_upageno;
    gint32 last_pageno;

    /*
     * The Page number and character code input fields.
     */
    GtkWidget *pageno;
    GtkWidget *charno;

    /*
     * The FontGrid widget.
     */
    GtkWidget *fgrid;

    /*
     * Widgets that may change sensitivity depending on the state of the font
     * being edited.
     */
    GtkWidget *file_save;
    GtkWidget *file_export;

    GtkWidget *edit_cut;
    GtkWidget *edit_copy;
    GtkWidget *edit_paste;
    GtkWidget *edit_overlay;
    GtkWidget *edit_insert;
    GtkWidget *edit_rename_glyphs;
    GtkWidget *edit_rename_menu;
    GtkWidget *edit_make_xlfd;
    GtkWidget *edit_unicode_names;
    GtkWidget *edit_adobe_names;
    GtkWidget *edit_test_glyphs;

    /*
     * These are the toggle widgets that change the code base displayed
     * in the Fontgrid to octal, decimal, or hex.
     */
    GtkWidget *view_oct;
    GtkWidget *view_dec;
    GtkWidget *view_hex;

    /*
     * The widget that toggles the view between the encoded and unencoded
     * sections of the font.
     */
    GtkWidget *view_unencoded;

    /*
     * The orientation label which changes depending on the current
     * orientation of the Fontgrid.
     */
    GtkWidget *view_orientation;

    /*
     * This is the font messages menu item that may or may not be
     * enabled.
     */
    GtkWidget *view_messages;

    /*
     * These are the widgets that make up the font messages dialog which is
     * used to record the font I/O and modification messages.
     */
    GtkWidget *messages_label;
    GtkWidget *messages_text;
    GtkWidget *messages_dialog;

    /*
     * These fields specify the font format being imported or exported.
     */
    guint import_format;
    guint export_format;

    /*
     * These are the preference widgets.
     */
    GtkWidget *pref_dialog;

    /*
     * These are the widgets for editing information about the font.
     */
    GtkWidget *finfo_notebook;
    GtkWidget *finfo_dialog;
    GtkWidget *finfo_comments;
    GtkWidget *finfo_erase_comments;
    GtkWidget *finfo_apply_comments;
    GtkWidget *finfo_font_props;
    GtkWidget *finfo_all_props;
    GtkWidget *finfo_prop_name;
    GtkWidget *finfo_prop_value;
    GtkWidget *finfo_prop_format[4];
    GtkWidget *finfo_apply_prop;
    GtkWidget *finfo_delete_prop;

    GtkWidget *finfo_enc_count;
    GtkWidget *finfo_unenc_count;

    GtkWidget *finfo_default_char;

    GtkWidget *finfo_spacing[4];

    GtkWidget *finfo_hres;
    GtkWidget *finfo_vres;
    GtkWidget *finfo_bpp;

    GtkWidget *finfo_dwidth;
    GtkWidget *finfo_ascent;
    GtkWidget *finfo_descent;

    GtkWidget *finfo_apply_info;
    gboolean finfo_xlfd_props_modified;

    /*
     * A flag to indicate if the font was just imported from a non-BDF format
     * file.  It is set to FALSE whenever a save of any kind is done.
     */
    gboolean imported;

} gbdfed_editor_t;

/*************************************************************************
 *
 * Globals.
 *
 *************************************************************************/

#include "fontgrid.h"
#if 0
#include "colorswatch.h"
#endif
#include "glyphtest.h"

/*
 * List of editors and their number.
 */
extern gbdfed_editor_t *editors;
extern guint num_editors;

/*
 * Buffers used for making messages and filenames.
 */
extern gchar buffer1[];
extern gchar buffer2[];

/*
 * Options for loading/saving fonts and other application options.
 */
extern gbdfed_options_t options;

/*************************************************************************
 *
 * The glyph test widget used by all editors in one session.
 *
 *************************************************************************/

extern GtkWidget *glyphtest;

/*************************************************************************
 *
 * Other functions: gbdfed.c
 *
 *************************************************************************/

extern guint gbdfed_make_editor(gchar *, gboolean);

/*************************************************************************
 *
 * File menu functions: guifile.c
 *
 *************************************************************************/

/*
 * New editor creation callback.
 */
extern void guifile_new_editor(GtkWidget *, gpointer);

/*
 * File import callbacks.
 */
extern void guifile_import_bdf_font(GtkWidget *, gpointer);
extern void guifile_import_console_font(GtkWidget *, gpointer);
extern void guifile_import_pkgf_font(GtkWidget *, gpointer);
extern void guifile_import_windows_font(GtkWidget *, gpointer);

#ifdef HAVE_HBF
extern void guifile_import_hbf_font(GtkWidget *, gpointer);
#endif

#ifdef HAVE_FREETYPE
extern void guifile_import_otf_font(GtkWidget *, gpointer);
#endif /* HAVE_FREETYPE */

#ifdef HAVE_XLIB
extern void guifile_import_xserver_font(GtkWidget *, gpointer);
#endif

/*
 * File save callbacks.
 */
extern void guifile_save_as(GtkWidget *, gpointer);
extern void guifile_save_as_wait(GtkWidget *, gpointer);
extern void guifile_save(GtkWidget *, gpointer);
extern void guifile_export_psf_font(GtkWidget *, gpointer);
extern void guifile_export_hex_font(GtkWidget *, gpointer);

/*
 * Special direct BDF font load call for recent fonts menu.
 */
extern void guifile_load_bdf_font(gbdfed_editor_t *ed, const gchar *filename);

/*************************************************************************
 *
 * Edit functions: guiedit.c
 *
 *************************************************************************/

extern void guiedit_set_unicode_glyph_names(GtkWidget *, gpointer);
extern void guiedit_set_adobe_glyph_names(GtkWidget *, gpointer);
extern void guiedit_set_uni_glyph_names(GtkWidget *, gpointer);
extern void guiedit_set_zerox_glyph_names(GtkWidget *, gpointer);
extern void guiedit_set_uplus_glyph_names(GtkWidget *, gpointer);
extern void guiedit_set_bslashu_glyph_names(GtkWidget *, gpointer);
extern void guiedit_show_font_info(GtkWidget *, gpointer);
extern void guiedit_show_font_comments(GtkWidget *, gpointer);
extern void guiedit_show_font_properties(GtkWidget *, gpointer);
extern void guiedit_update_font_info(gbdfed_editor_t *);
extern void guiedit_update_font_properties(gbdfed_editor_t *);
extern void guiedit_update_font_details(gbdfed_editor_t *);
extern void guiedit_update_code_base(gbdfed_editor_t *);
extern void guiedit_copy_selection(GtkWidget *, gpointer);
extern void guiedit_cut_selection(GtkWidget *, gpointer);
extern void guiedit_paste_selection(GtkWidget *, gpointer);
extern void guiedit_overlay_selection(GtkWidget *, gpointer);
extern void guiedit_insert_selection(GtkWidget *, gpointer);
extern void guiedit_show_glyphtest(GtkWidget *, gpointer);

/*************************************************************************
 *
 * Glyph edit functions: guigedit.c
 *
 *************************************************************************/

extern void guigedit_edit_glyph(gbdfed_editor_t *ed,
                                FontgridSelectionInfo *si);

extern void guigedit_show_cap_height(gboolean show);
extern void guigedit_show_x_height(gboolean show);
extern void guigedit_set_pixel_size(guint pixel_size);
extern void guigedit_set_font_spacing(gint spacing, guint16 monowidth);
extern void guigedit_set_font_spacing(gint spacing, guint16 monowidth);
extern void guigedit_set_code_base(gint code_base);

extern void guigedit_cleanup(void);


/*************************************************************************
 *
 * Preference functions: guipref.c
 *
 *************************************************************************/

extern void guiedit_show_preferences(GtkWidget *, gpointer);
extern void guiedit_preference_cleanup(void);

/*************************************************************************
 *
 * Help functions: guihelp.c
 *
 *************************************************************************/

/*
 * Macros that specify the help text to be used.
 */
#define GUIHELP_ABOUT        0
#define GUIHELP_PROGRAM      1
#define GUIHELP_FONTGRID     2
#define GUIHELP_GLYPH_EDITOR 3
#define GUIHELP_CONFIG_FILE  4
#define GUIHELP_PREFERENCES  5
#define GUIHELP_FNT          6
#define GUIHELP_OTF          7
#define GUIHELP_PSF          8
#define GUIHELP_HEX          9
#define GUIHELP_COLOR        10
#define GUIHELP_TIPS         11

extern void guihelp_show_help(GtkWidget *w, gpointer data);

extern void guigedit_cleanup(void);
extern void guihelp_cleanup(void);

/*************************************************************************
 *
 * Glyph operation functions: guiops.c
 *
 *************************************************************************/

extern void guiops_show_translate(GtkWidget *, gpointer);
extern void guiops_show_rotate(GtkWidget *, gpointer);
extern void guiops_show_shear(GtkWidget *, gpointer);
extern void guiops_show_embolden(GtkWidget *, gpointer);

/*************************************************************************
 *
 * Util functions: guiutil.c
 *
 *************************************************************************/

extern void guiutil_show_dialog_centered(GtkWidget *dialog, GtkWidget *parent);
extern void guiutil_cursor_cleanup(void);
extern void guiutil_busy_cursor(GtkWidget *w, gboolean on);
extern void guiutil_error_message(GtkWidget *parent, gchar *text);
extern gboolean guiutil_yes_or_no(GtkWidget *parent, gchar *text,
                                  gboolean default_answer);
extern void guiutil_util_set_tooltip(GtkWidget *, gchar *);

G_END_DECLS

#endif /* _h_gbdfed */

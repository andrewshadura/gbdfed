/*
 * Copyright 2001 Computing Research Labs, New Mexico State University
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
#ifndef _h_xmbdfed
#define _h_xmbdfed

/*
 * $Id: xmbdfed.h,v 1.26 2001/11/09 22:01:52 mleisher Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A macro for prototypes.
 */
#undef __
#ifdef __STDC__
#define __(x) x
#else
#define __(x) ()
#endif

#define XMBDFED_VERSION "4.5"

/**************************************************************************
 *
 * Types.
 *
 **************************************************************************/

typedef struct {
    Widget save;
    Widget saveas;
    Widget export;
    int open_type;
    int export_type;
} MXFEditorFileMenu;

typedef struct {
    Widget cut;
    Widget copy;
    Widget paste;
    Widget overlay;
    Widget insert;
    Widget finfo;
    Widget dwidth;
    Widget xlfdname;
    Widget test;
    Widget gname;
    Widget unames;
    Widget anames;
    Widget unfp;
    Widget upfn;
    Widget awidth;
} MXFEditorEditMenu;

typedef struct {
    Widget unencoded;
    Widget oct;
    Widget dec;
    Widget hex;
    Widget vert;
    Widget other;
    Widget acmsgs;
} MXFEditorViewMenu;

typedef struct {
    Widget shell;
    Widget form;
    Widget enc;
    Widget unenc;
    Widget defchar;
    Widget dwidth;
    Widget ascent;
    Widget descent;
    Widget hres;
    Widget vres;
    Widget bpp;
    Widget prop;
    Widget mono;
    Widget cell;
    Widget update;
} MXFEditorFontInfo;

typedef struct {
    Widget shell;
    Widget form;
    Widget test;
    Widget clear;
    Widget baseline;
    Boolean show_baseline;
    Boolean active;
} MXFEditorGlyphTest;

typedef struct {
    Widget shell;
    Widget form;
    Widget plist;
    Widget pname;
    Widget pvalue;
    Widget atom;
    Widget integer;
    Widget cardinal;
    Widget fontprops;
    Widget allprops;
    Widget update;
    Widget delete;
} MXFEditorProperties;

typedef struct {
    Widget shell;
    Widget form;
    Widget text;
    Widget update;
} MXFEditorComments;

typedef struct {
    unsigned long id;
    char *path;
    char *file;
    Widget shell;
    Widget mainw;
    Widget fontname;
    Widget glyphinfo;
    Widget prevpage;
    Widget nextpage;
    Widget lastpage;
    Widget firstpage;
    Widget pageno;
    Widget code;
    Widget fgrid;
    Widget elist;
    Boolean update_fontname;
    long last_pgno;
    long last_upgno;
    MXFEditorFileMenu filemenu;
    MXFEditorEditMenu editmenu;
    MXFEditorViewMenu viewmenu;
    MXFEditorProperties props;
    MXFEditorComments comments;
    MXFEditorFontInfo finfo;
    MXFEditorGlyphTest test;
} MXFEditor;

typedef struct {
    Widget shell;
    Widget dialog;
    Widget ascent;
    Widget descent;
    Widget lbearing;
    Widget rbearing;
    Widget apply;
    Boolean visible;
} MXFEditorGlyphEditResize;

typedef struct {
    unsigned long id;
    unsigned long owner;
    Widget shell;
    Widget mainw;
    Widget reload;
    Widget cut;
    Widget copy;
    Widget paste;
    Widget next;
    Widget prev;
    Widget update;
    Widget update_next;
    Widget update_prev;
    Widget name;
    Widget encoding;
    Widget dwidth;
    Widget metrics;
    Widget pointer;
    Widget tboxhelp;
    Widget gedit;
    Widget tbox;
    MXFEditorGlyphEditResize resize;
} MXFEditorGlyphEdit;

typedef struct {
    char *accelerator;
    char *accelerator_text;
    char *unicode_name_file;
    char *adobe_name_file;
    int no_blanks;
    int really_exit;
    int overwrite_mode;
    int initial_glyph;
    unsigned long pixel_size;
    unsigned long resolution;
    int show_cap_height;
    int show_x_height;
    int vertical;
    int power2;
    int progbar;
    int percent_only;
    int colors_allocated;
    unsigned short colors[20];
    unsigned long pixels[20];
    int sbit;
    int backups;
    int code_base;
    bdf_options_t font_opts;
} MXFEditorOptions;

/**************************************************************************
 *
 * Macros.
 *
 **************************************************************************/

/*
 * Macros that indicate the behavior of the file selection dialog when it is
 * popped up.
 */
#define XMBDFED_OPEN_BDF     0
#define XMBDFED_OPEN_HBF     1
#define XMBDFED_OPEN_CONSOLE 2
#define XMBDFED_OPEN_PKGF    3
#define XMBDFED_OPEN_TTF     4
#define XMBDFED_OPEN_FNT     5

/*
 * Macros that indicate the behavior of the export file selection dialog when
 * it is popped up.
 */
#define XMBDFED_EXPORT_PSF 1
#define XMBDFED_EXPORT_HEX 2

/**************************************************************************
 *
 * Global variables.
 *
 **************************************************************************/

extern Widget top, questd, errd, saved, opend;
extern XtAppContext app;
extern Boolean yes_no_answer;
extern Boolean yes_no_done;
extern unsigned long active_editor;
extern String app_name;
extern String app_class;
extern char title[];
extern char name[];

extern MXFEditor *editors;
extern unsigned long num_editors;

extern Pixmap lbearing_image;
extern Pixmap rbearing_image;
extern Pixmap ascent_image;
extern Pixmap descent_image;

extern MXFEditorOptions xmbdfed_opts;

/**************************************************************************
 *
 * Global functions.
 *
 **************************************************************************/

unsigned long MakeEditor __((Widget parent, char *filename, Boolean cmdline));

extern Boolean AskBooleanQuestion __((char *question, char *ok_label,
                                      char *cancel_label));

extern void WaitSave __((Widget w, MXFEditor *ed));

extern void ErrorDialog __((char *message));

extern void DoClose __((Widget w, XtPointer client_data, XtPointer call_data));

extern void WatchCursor __((Widget w, Boolean on));

extern void xmbdfedAllocateColors __((Widget from, int bpp));
extern void xmbdfedUpdateColorValues __((void));
extern void xmbdfedRestoreColorValues __((void));

/**************************************************************************
 *
 * Glyph editor functions.
 *
 **************************************************************************/

extern void DoGlyphEdit __((Widget w, XtPointer client_data,
                            XtPointer call_data));
extern void GlyphEditCleanup __((void));

extern void GlyphEditChangeBase __((unsigned long id, int base));

extern void GlyphEditChangeSpacing __((unsigned long id,
                                       unsigned short spacing,
                                       unsigned short monowidth));

extern void GlyphEditChangeSetupValues __((unsigned long pixel_size,
                                           int show_cap_height,
                                           int show_x_height));

extern void GlyphEditCreatePixmaps __((MXFEditor *ed));

extern void GlyphEditClose __((MXFEditor *ed));

/**************************************************************************
 *
 * Property editor functions.
 *
 **************************************************************************/

extern void DoFontProperties __((Widget w, XtPointer client_data,
                                 XtPointer call_data));

/**************************************************************************
 *
 * Comment editor functions.
 *
 **************************************************************************/

extern void DoEditComments __((Widget w, XtPointer client_data,
                               XtPointer call_data));

/**************************************************************************
 *
 * Font info editor functions.
 *
 **************************************************************************/

extern void DoFontInfo __((Widget w, XtPointer client_data,
                           XtPointer call_data));

/**************************************************************************
 *
 * Setup editor functions.
 *
 **************************************************************************/

extern void DoSetupDialog __((Widget w, XtPointer client_data,
                              XtPointer call_data));

/**************************************************************************
 *
 * Color editor functions.
 *
 **************************************************************************/

extern void DoColorDialog __((Widget w, XtPointer client_data,
                              XtPointer call_data));

/**************************************************************************
 *
 * Server font grabber functions.
 *
 **************************************************************************/

extern void DoGrabServerFont __((Widget w, XtPointer client_data,
                                 XtPointer call_data));

/**************************************************************************
 *
 * Progress bar functions.
 *
 **************************************************************************/

extern void UpdateProgressBar __((bdf_callback_struct_t *cb, void *data));

/**************************************************************************
 *
 * Help system functions.
 *
 **************************************************************************/

extern void AddHelpMenu __((Widget pdown));

/**************************************************************************
 *
 * Glyph test functions.
 *
 **************************************************************************/

extern void DoGlyphTest __((Widget w, XtPointer client_data,
                            XtPointer call_data));

extern void GlyphTestAddGlyph __((bdf_font_t *font, bdf_glyph_t *glyph));

extern void GlyphTestRedisplay __((void));

extern void GlyphTestReset __((bdf_font_t *font));

extern void GlyphTestResize __((void));

/**************************************************************************
 *
 * Glyph operations functions.
 *
 **************************************************************************/

/*
 * Special operation callback reasons.
 */
#define XMBDFED_TRANSLATE 1
#define XMBDFED_ROTATE    2
#define XMBDFED_SHEAR     3
#define XMBDFED_EMBOLDEN  4

/*
 * Special operation callback structure.
 */
typedef struct {
    int reason;
    short dx;
    short dy;
    short angle;
    short pad;
    Boolean all;
} XmuttOperationCallbackStruct;

extern void ShowTranslateDialog __((XtCallbackProc cback, XtPointer data,
                                    Boolean toggles_sensitive));
extern void ShowRotateDialog __((XtCallbackProc cback, XtPointer data,
                                 Boolean toggles_sensitive));
extern void ShowShearDialog __((XtCallbackProc cback, XtPointer data,
                                Boolean toggles_sensitive));
extern void ShowEmboldenDialog __((XtCallbackProc cback, XtPointer data,
                                   Boolean toggles_sensitive));

#ifdef HAVE_FREETYPE

/**************************************************************************
 *
 * TrueType functions.
 *
 **************************************************************************/

extern int LoadTrueTypeFont __((char **path, MXFEditor *ed,
                                bdf_font_t **font));

#endif /* HAVE_FREETYPE */

/**************************************************************************
 *
 * Windows FNT functions.
 *
 **************************************************************************/

extern int LoadWindowsFont __((char **path, MXFEditor *ed, bdf_font_t **out));

#undef __

#ifdef __cplusplus
}
#endif

#endif /* _h_xmbdfed */

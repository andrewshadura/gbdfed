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
#ifndef lint
#ifdef __GNUC__
static char rcsid[] __attribute__ ((unused)) = "$Id: xmbdfed.c,v 1.27 2001/09/19 21:00:44 mleisher Exp $";
#else
static char rcsid[] = "$Id: xmbdfed.c,v 1.27 2001/09/19 21:00:44 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/RowColumnP.h>
#include <Xm/MainW.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include "FGrid.h"
#include "GTest.h"
#include "xmbdfed.h"

/**************************************************************************
 *
 * Global variables.
 *
 **************************************************************************/

Widget top = 0, questd = 0, errd = 0, saved = 0, opend = 0, exportd = 0;
XtAppContext app = 0;
Boolean yes_no_answer = False;
Boolean yes_no_done = False;
unsigned long active_editor = 0;
String app_name = 0;
String app_class = 0;
char title[BUFSIZ];
char name[BUFSIZ];

MXFEditor *editors;
unsigned long num_editors = 0;

MXFEditorOptions xmbdfed_opts;

/**************************************************************************
 *
 * Local variables.
 *
 **************************************************************************/

/*
 * The watch cursor.
 */
static Cursor watch_cursor = 0;

/*
 * The auto-correction message widgets.
 */
static Widget acmsg_shell;
static Widget acmsg_form;
static Widget acmsg_text;

/*
 * Variables for special exit handling.
 */
static Boolean save_done;
static Boolean exiting = False;

#define OGINFO "\"%s\" %lo (%o,%o)\nascent %d descent %d right %d left %d"
#define DGINFO "\"%s\" %ld (%d,%d)\nascent %d descent %d right %d left %d"
#define HGINFO "\"%s\" %04lX (%02X,%02X)\nascent %d descent %d right %d left %d"

static String fallback_resources[] = {
    "*fontList: -adobe-new century schoolbook-medium-r-normal--14-100-100-100-p-82-iso8859-1",
    (String) NULL,
};

/*
 * Resources and descriptions for the command line options.
 *
 * IMPORTANT: The order of these resources is important!  Do not
 *            rearrange them unless you know what you are doing!
 */
static XtResource cmdres[] = {
    {
        "noMetricsCorrections", "NoMetricsCorrections",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, font_opts.correct_metrics),
        XmRImmediate, (XtPointer) 1
    },
    {
        "noUnencodedGlyphs", "NoUnencodedGlyphs",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, font_opts.keep_unencoded),
        XmRImmediate, (XtPointer) 1
    },
    {
        "noComments", "NoComments",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, font_opts.keep_comments),
        XmRImmediate, (XtPointer) 1
    },
    {
        "padCharacterCells", "PadCharacterCells",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, font_opts.pad_cells),
        XmRImmediate, (XtPointer) 1
    },
    {
        "fontPointSize", "FontPointSize",
        XmRInt, sizeof(long),
        XtOffsetOf(MXFEditorOptions, font_opts.point_size),
        XmRImmediate, (XtPointer) 0
    },
    {
        "fontHorizontalResolution", "FontHorizontalResolution",
        XmRInt, sizeof(unsigned long),
        XtOffsetOf(MXFEditorOptions, font_opts.resolution_x),
        XmRImmediate, (XtPointer) 0
    },
    {
        "fontVerticalResolution", "FontVerticalResolution",
        XmRInt, sizeof(unsigned long),
        XtOffsetOf(MXFEditorOptions, font_opts.resolution_y),
        XmRImmediate, (XtPointer) 0
    },
    {
        "fontSpacing", "FontSpacing",
        "FontSpacing", sizeof(int),
        XtOffsetOf(MXFEditorOptions, font_opts.font_spacing),
        XmRImmediate, (XtPointer) 0
    },
    {
        "bitsPerPixel", "BitsPerPixel",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, font_opts.bits_per_pixel),
        XmRImmediate, (XtPointer) 0
    },
    {
        "eolType", "EOLType",
        "EOLType", sizeof(int),
        XtOffsetOf(MXFEditorOptions, font_opts.eol),
        XmRImmediate, (XtPointer) 0
    },
    {
        "fontResolution", "FontResolution",
        XmRInt, sizeof(unsigned long),
        XtOffsetOf(MXFEditorOptions, resolution),
        XmRImmediate, (XtPointer) 0
    },
    {
        "skipBlankGlyphPages", "SkipBlankGlyphPages",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, no_blanks),
        XmRImmediate, (XtPointer) 1
    },
    {
        "reallyExitDialog", "ReallyExitDialog",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, really_exit),
        XmRImmediate, (XtPointer) 1
    },
    {
        "gridOverwriteMode", "GridOverwriteMode",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, overwrite_mode),
        XmRImmediate, (XtPointer) 1
    },
    {
        "initialGlyph", "InitialGlyph",
        "GlyphCode", sizeof(long),
        XtOffsetOf(MXFEditorOptions, initial_glyph),
        XmRImmediate, (XtPointer) -1
    },
    {
        "progressBar", "ProgressBar",
        XmRInt, sizeof(int),
        XtOffsetOf(MXFEditorOptions, progbar),
        XmRImmediate, (XtPointer) 1
    },
    {
        "codeBase", "CodeBase",
        "CodeBase", sizeof(int),
        XtOffsetOf(MXFEditorOptions, code_base),
        XmRImmediate, (XtPointer) 16,
    },
};

static XrmOptionDescRec cmdopts[] = {
    {"-nm",   "*noMetricsCorrections",     XrmoptionNoArg,  (XtPointer) "0"},
    {"-nu",   "*noUnencodedGlyphs",        XrmoptionNoArg,  (XtPointer) "0"},
    {"-nc",   "*noComments",               XrmoptionNoArg,  (XtPointer) "0"},
    {"-np",   "*padCharacterCells",        XrmoptionNoArg,  (XtPointer) "0"},
    {"-bp",   "*skipBlankGlyphPages",      XrmoptionNoArg,  (XtPointer) "0"},
    {"-ps",   "*fontPointSize",            XrmoptionSepArg, (XtPointer) "0"},
    {"-hres", "*fontHorizontalResolution", XrmoptionSepArg, (XtPointer) "0"},
    {"-vres", "*fontVerticalResolution",   XrmoptionSepArg, (XtPointer) "0"},
    {"-res",  "*fontResolution",           XrmoptionSepArg, (XtPointer) "0"},
    {"-sp",   "*fontSpacing",              XrmoptionSepArg, (XtPointer) "0"},
    {"-ed",   "*reallyExitDialog",         XrmoptionNoArg,  (XtPointer) "0"},
    {"-im",   "*gridOverwriteMode",        XrmoptionNoArg,  (XtPointer) "0"},
    {"-bpp",  "*bitsPerPixel",             XrmoptionSepArg, (XtPointer) "0"},
    {"-eol",  "*eolType",                  XrmoptionSepArg, (XtPointer) "0"},
    {"-g",    "*initialGlyph",             XrmoptionSepArg, (XtPointer) "0"},
    {"-pb",   "*progressBar",              XrmoptionNoArg,  (XtPointer) "0"},
    {"-cb",   "*codeBase",                 XrmoptionSepArg, (XtPointer) "0"},
};

/**************************************************************************
 *
 * Functions.
 *
 **************************************************************************/

void
#ifndef _NO_PROTO
WatchCursor(Widget w, Boolean on)
#else
WatchCursor(w, on)
Widget w;
Boolean on;
#endif
{
    if (on == True) {
        if (watch_cursor == 0)
          watch_cursor = XCreateFontCursor(XtDisplay(w), XC_watch);
        XDefineCursor(XtDisplay(w), XtWindow(w), watch_cursor);
    } else
      XDefineCursor(XtDisplay(w), XtWindow(w), None);
    XFlush(XtDisplay(w));
}

void
#ifndef _NO_PROTO
DoClose(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoClose(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XtPopdown((Widget) client_data);
}

void
#ifndef _NO_PROTO
ErrorDialog(char *message)
#else
ErrorDialog(message)
char *message;
#endif
{
    XmString s;
    Arg av[1];

    if (errd == 0) {
        sprintf(title, "%s: Error Dialog", app_name);
        XtSetArg(av[0], XmNtitle, title);
        errd = XmCreateErrorDialog(top, "ucedit_error_dialog", av, 1);
        XtUnmanageChild(XmMessageBoxGetChild(errd, XmDIALOG_HELP_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(errd, XmDIALOG_CANCEL_BUTTON));
    }
    s = XmStringCreateLtoR(message, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[0], XmNmessageString, s);
    XtSetValues(errd, av, 1);
    XmStringFree(s);
    XtManageChild(errd);
    XtPopup(XtParent(errd), XtGrabNone);
}

static void
#ifndef _NO_PROTO
SetBooleanAnswer(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetBooleanAnswer(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long n;

    n = (unsigned long) client_data;

    yes_no_answer = (n) ? True : False;
    yes_no_done = True;
}

Boolean
#ifndef _NO_PROTO
AskBooleanQuestion(char *question, char *ok_label, char *cancel_label)
#else
AskBooleanQuestion(question, ok_label, cancel_label)
char *question, *ok_label, *cancel_label;
#endif
{
    XmString s, cl, ol;
    Cardinal ac;
    Arg av[3];

    if (questd == 0) {
        sprintf(title, "%s: Question", app_name);
        XtSetArg(av[0], XmNtitle, title);
        questd = XmCreateQuestionDialog(top, "ucedit_question_dialog",
                                        av, 1);
        XtUnmanageChild(XmMessageBoxGetChild(questd, XmDIALOG_HELP_BUTTON));
        XtAddCallback(questd, XmNokCallback, SetBooleanAnswer, (XtPointer) 1);
        XtAddCallback(questd, XmNcancelCallback, SetBooleanAnswer,
                      (XtPointer) 0);
    }
    cl = ol = 0;
    ac = 0;
    s = XmStringCreateLtoR(question, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[ac], XmNmessageString, s); ac++;
    if (ok_label != 0) {
        ol = XmStringCreateSimple(ok_label);
        XtSetArg(av[ac], XmNokLabelString, ol); ac++;
    }
    if (cancel_label != 0) {
        cl = XmStringCreateSimple(cancel_label);
        XtSetArg(av[ac], XmNcancelLabelString, cl); ac++;
    }
    XtSetValues(questd, av, ac);
    XmStringFree(s);
    if (ol != 0)
        XmStringFree(ol);
    if (cl != 0)
        XmStringFree(cl);

    XtManageChild(questd);
    XtPopup(XtParent(questd), XtGrabNone);

    yes_no_done = False;
    while (yes_no_done == False)
      XtAppProcessEvent(app, XtIMAll);

    XtUnmanageChild(questd);
    XtPopdown(XtParent(questd));
    return yes_no_answer;
}

static void
#ifndef _NO_PROTO
ReallySaveAs(Widget w, XtPointer client_data, XtPointer call_data)
#else
ReallySaveAs(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    FILE *out;
    char *path, *file, *fp;
    MXFEditor *ed;
    XmFileSelectionBoxCallbackStruct *cb;
    bdf_property_t vanity;

    ed = &editors[active_editor];

    path = 0;

    cb = (XmFileSelectionBoxCallbackStruct *) call_data;
    XmStringGetLtoR(cb->value, XmSTRING_DEFAULT_CHARSET, &path);

    /*
     * Check to see if the font file already exists and ask if the user wishes
     * to overwrite.
     */
    if ((out = fopen(path, "r")) != 0) {
        fclose(out);
        if ((fp = strrchr(path, '/')) == 0)
          fp = path;
        else
          fp++;
        sprintf(name,
                "\"%s\" already exists.\nDo you want to overwrite?", fp);
        if (AskBooleanQuestion(name, "Yes", "No") == False)
          return;

        if (xmbdfed_opts.backups) {
            /*
             * The file already exists, so rename it before writing the file
             * again.
             */
            sprintf(name, "%s.BAK", path);
            if (rename(path, name) < 0) {
                sprintf(name,
                        "A problem occured making the backup file:\n%s.BAK",
                        path);
                ErrorDialog(name);
            }
        }
    }

    save_done = True;

    out = fopen(path, "w");
    if (out == 0) {
        sprintf(name, "Unable to save to '%s'.", path);
        ErrorDialog(name);
        exiting = False;
    } else {
        /*
         * Unbuffer the output.
         */
        setbuf(out, 0);

        /*
         * Add the xmbdfed info property before saving the font, but only if
         * the font was really modified by the editor.
         */
        if (XmuttFontGridModified(ed->fgrid) == True) {
            sprintf(name, "Edited with xmbdfed %s.", XMBDFED_VERSION);
            vanity.name = "_XMBDFED_INFO";
            vanity.format = BDF_ATOM;
            vanity.value.atom = name;
            bdf_add_font_property(XmuttFontGridFont(ed->fgrid), &vanity);
        }
        if ((fp = strrchr(path, '/')) == 0)
          fp = path;
        else
          fp++;
        if (xmbdfed_opts.progbar)
          bdf_save_font(out, XmuttFontGridFont(ed->fgrid),
                        &xmbdfed_opts.font_opts, UpdateProgressBar,
                        (void *) fp);
        else {
            WatchCursor(ed->fgrid, True);
            bdf_save_font(out, XmuttFontGridFont(ed->fgrid),
                          &xmbdfed_opts.font_opts, 0, 0);
            WatchCursor(ed->fgrid, False);
        }

        fclose(out);

        /*
         * Save the SBIT metrics if indicated.
         */
        if (xmbdfed_opts.sbit) {
            if ((fp = strrchr(path, '.')) == 0)
              fp = path + strlen(path);
            (void) strcpy(fp, ".met");
            if ((out = fopen(path, "w")) != 0) {
                sprintf(name, "%s %s", app_name, XMBDFED_VERSION);
                bdf_save_sbit_metrics(out, XmuttFontGridFont(ed->fgrid),
                                      &xmbdfed_opts.font_opts, name);
                fclose(out);
            }
        }
    }

    /*
     * Clear the old directory and filename and
     * update with the new ones.
     */
    if (ed->path != 0)
      XtFree(ed->path);
    if (ed->file != 0)
      XtFree(ed->file);

    file = strrchr(path, '/');
    if (file) {
        *file++ = 0;
        ed->path = XtMalloc(strlen(path) + 1);
        (void) strcpy(ed->path, path);
    } else {
        file = path;
        ed->path = XtMalloc(2);
        ed->path[0] = '.';
        ed->path[1] = 0;
    }
    ed->file = XtMalloc(strlen(file) + 1);
    (void) strcpy(ed->file, file);

    /*
     * Update the title of the editor window.
     */
    sprintf(title, "%s - %s", app_name, file);
    XtVaSetValues(ed->shell, XmNtitle, title, 0);

    /*
     * Free the path give back by the file selection dialog.
     */
    XtFree(path);

    /*
     * Desensitize the save button on the menu.
     */
    XtSetSensitive(ed->filemenu.save, False);

    /*
     * Tell the font grid the font has been saved.
     */
    XmuttFontGridSetModified(ed->fgrid, False);

    XtPopdown(XtParent(saved));
}

static void
#ifndef _NO_PROTO
AddFilename(Widget w, XtPointer client_data, XtPointer call_data)
#else
AddFilename(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    Widget dialog, fstext;
    int pos1, pos2;
    char *dir, *dp, *ep, *suff;
    MXFEditor *ed;

    dialog = (Widget) client_data;
    ed = &editors[active_editor];

    suff = 0;
    switch (ed->filemenu.export_type) {
      case XMBDFED_EXPORT_PSF: suff = ".psf"; break;
      case XMBDFED_EXPORT_HEX: suff = ".hex"; break;
    }

    if (ed->file != 0) {
        (void) strcpy(name, ed->file);
        if (dialog == exportd) {
            if ((dp = strrchr(name, '.')))
              (void) strcpy(dp, suff);
            else
              (void) strcat(dp, suff);
        }
    } else {
        if (dialog == exportd)
          sprintf(name, "unnamed%ld%s", active_editor, suff);
        else
          sprintf(name, "unnamed%ld.bdf", active_editor);
    }

    fstext = XmFileSelectionBoxGetChild(dialog, XmDIALOG_FILTER_TEXT);
    dir = XmTextFieldGetString(fstext);
    if ((dp = strrchr(dir, '/')))
      *dp = 0;

    /*
     * Set the filename in the file selection box.
     */
    fstext = XmFileSelectionBoxGetChild(dialog, XmDIALOG_TEXT);
    if ((ep = strchr(name, '.')) == name)
      ep = 0;
    sprintf(title, "%s/%s", dir, name);
    XmTextFieldSetString(fstext, title);
    pos1 = strlen(dir) + 1;
    pos2 = pos1 + ((ep) ? ep - name : strlen(name));
    XmTextFieldSetCursorPosition(fstext, pos1 + strlen(name));
    XmTextFieldSetSelection(fstext, pos1, pos2, CurrentTime);
}

static void
#ifndef _NO_PROTO
ReallyExport(Widget w, XtPointer client_data, XtPointer call_data)
#else
ReallyExport(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    FILE *out;
    char *path, *file, *fp;
    MXFEditor *ed;
    XmFileSelectionBoxCallbackStruct *cb;

    ed = &editors[active_editor];

    path = fp = 0;

    cb = (XmFileSelectionBoxCallbackStruct *) call_data;
    XmStringGetLtoR(cb->value, XmSTRING_DEFAULT_CHARSET, &path);

    /*
     * Check to see if the font file already exists and ask if the user wishes
     * to overwrite.
     */
    if ((out = fopen(path, "r")) != 0) {
        fclose(out);
        if ((fp = strrchr(path, '/')) == 0)
          fp = path;
        else
          fp++;
        sprintf(name,
                "\"%s\" already exists.\nDo you want to overwrite?", fp);
        if (AskBooleanQuestion(name, "Yes", "No") == False)
          return;
    }

    out = fopen(path, "w");
    if (out == 0) {
        sprintf(name, "Unable to export to '%s'.", path);
        ErrorDialog(name);
        return;
    } else {
        /*
         * Unbuffer the output and write the font.
         */
        setbuf(out, 0);
        switch (ed->filemenu.export_type) {
          case XMBDFED_EXPORT_PSF:
            bdf_export_psf(out, XmuttFontGridFont(ed->fgrid));
            break;
          case XMBDFED_EXPORT_HEX:
            if (xmbdfed_opts.progbar)
              bdf_export_hex(out, XmuttFontGridFont(ed->fgrid),
                             UpdateProgressBar, (void *) fp);
            else {
                WatchCursor(ed->fgrid, True);
                bdf_export_hex(out, XmuttFontGridFont(ed->fgrid), 0, 0);
                WatchCursor(ed->fgrid, False);
            }
            break;
        }
        fclose(out);
    }

    /*
     * Clear the old directory and filename and
     * update with the new ones.
     */
    if (ed->path != 0)
      XtFree(ed->path);
    if (ed->file != 0)
      XtFree(ed->file);

    file = strrchr(path, '/');
    if (file) {
        *file++ = 0;
        ed->path = XtMalloc(strlen(path) + 1);
        (void) strcpy(ed->path, path);
    } else {
        file = path;
        ed->path = XtMalloc(2);
        ed->path[0] = '.';
        ed->path[1] = 0;
    }
    ed->file = XtMalloc(strlen(file) + 1);
    (void) strcpy(ed->file, file);

    /*
     * Free the path give back by the file selection dialog.
     */
    XtFree(path);

    XtPopdown(XtParent(exportd));
}

static void
#ifndef _NO_PROTO
DoCancelSave(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCancelSave(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    save_done = True;
    XtPopdown((Widget) client_data);
}

static void
#ifndef _NO_PROTO
DoSaveAs(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoSaveAs(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int pos1, pos2;
    unsigned long id;
    MXFEditor *ed;
    Widget fstext;
    char *dir, *dp, *ep;
    XmString s, fdir;
    Boolean free_dir;
    Cardinal ac;
    Arg av[4];

    id = (unsigned long) client_data;
    ed = &editors[id];

    dir = 0;
    fdir = (ed->path) ? XmStringCreateSimple(ed->path) : 0;

    if (saved == 0) {
        sprintf(title, "%s: Save File", app_name);
        s = XmStringCreateSimple("*.[Bb][Dd][Ff]");
        ac = 0;
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetArg(av[ac], XmNpattern, s); ac++;
        if (dir != 0) {
            XtSetArg(av[ac], XmNdirectory, fdir); ac++;
        }
        saved = XmCreateFileSelectionDialog(top, "xmbdfed_save_file_sel_box",
                                            av, ac);
        XmStringFree(s);
        XtAddCallback(saved, XmNokCallback, ReallySaveAs, 0);
        XtAddCallback(saved, XmNapplyCallback, AddFilename, (XtPointer) saved);
        XtAddCallback(saved, XmNcancelCallback, DoCancelSave,
                      (XtPointer) XtParent(saved));
        XtManageChild(saved);
    } else {
        XtSetArg(av[0], XmNdirectory, fdir);
        XtSetValues(saved, av, 1);
    }

    if (fdir != 0)
      XmStringFree(fdir);

    active_editor = id;

    free_dir = False;
    dir = 0;
    if (ed->file != 0) {
        dir = ed->path;
        sprintf(name, "%s", ed->file);
    } else
      sprintf(name, "unnamed%ld.bdf", id);

    if (dir == 0) {
        fstext = XmFileSelectionBoxGetChild(saved, XmDIALOG_FILTER_TEXT);
        dir = XmTextFieldGetString(fstext);
        if ((dp = strrchr(dir, '/')))
          *dp = 0;
        free_dir = True;
    } else {
        /*
         * Force the directory to be set to the one specified by the font.
         */
        s = XmStringCreateSimple(dir);
        XtSetArg(av[0], XmNdirectory, s);
        XtSetValues(saved, av, 1);
        XmStringFree(s);
    }

    /*
     * Set the filename in the file selection box.
     */
    fstext = XmFileSelectionBoxGetChild(saved, XmDIALOG_TEXT);
    if ((ep = strchr(name, '.')) == name)
      ep = 0;
    sprintf(title, "%s/%s", dir, name);
    XmTextFieldSetString(fstext, title);
    pos1 = strlen(dir) + 1;
    pos2 = pos1 + ((ep) ? ep - name : strlen(name));
    XmTextFieldSetCursorPosition(fstext, pos1 + strlen(name));
    XmTextFieldSetSelection(fstext, pos1, pos2, CurrentTime);

    if (free_dir)
      XtFree(dir);

    XtPopup(XtParent(saved), XtGrabNone);
}

static void
#ifndef _NO_PROTO
DoSave(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoSave(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    FILE *out;
    MXFEditor *ed;
    unsigned long id;
    bdf_property_t vanity;
    char *pt, path[BUFSIZ];

    id = (unsigned long) client_data;
    ed = &editors[id];

    if (XmuttFontGridModified(ed->fgrid) == False)
      return;

    if (ed->file == 0) {
        DoSaveAs(w, client_data, call_data);
        return;
    }

    save_done = True;

    if (ed->path != 0)
      sprintf(path, "%s/%s", ed->path, ed->file);
    else
      sprintf(path, "%s", ed->file);

    if (xmbdfed_opts.backups) {
        /*
         * The file already exists, so rename it before writing the file
         * again.
         */
        sprintf(name, "%s.BAK", path);
        if (rename(path, name) < 0) {
            sprintf(name,
                    "A problem occured making the backup file:\n%s.BAK",
                    path);
            ErrorDialog(name);
        }
    }

    out = fopen(path, "w");
    if (out == 0) {
        sprintf(name, "Unable to save to '%s'.", path);
        ErrorDialog(name);
        exiting = False;
        return;
    }

    /*
     * Add the xmbdfed info property before saving the font, but only if
     * the font was really modified by the editor.
     */
    if (XmuttFontGridModified(ed->fgrid) == True) {
        sprintf(name, "Edited with xmbdfed %s.", XMBDFED_VERSION);
        vanity.name = "_XMBDFED_INFO";
        vanity.format = BDF_ATOM;
        vanity.value.atom = name;
        bdf_add_font_property(XmuttFontGridFont(ed->fgrid), &vanity);
    }
    if (xmbdfed_opts.progbar)
      bdf_save_font(out, XmuttFontGridFont(ed->fgrid), &xmbdfed_opts.font_opts,
                    UpdateProgressBar, (void *) ed->file);
    else {
        WatchCursor(ed->fgrid, True);
        bdf_save_font(out, XmuttFontGridFont(ed->fgrid),
                      &xmbdfed_opts.font_opts, 0, 0);
        WatchCursor(ed->fgrid, False);
    }

    fclose(out);

    /*
     * Save the SBIT metrics file if indicated.
     */
    if (xmbdfed_opts.sbit) {
        if ((pt = strrchr(path, '.')) == 0)
          pt = path + strlen(path);
        strcpy(pt, ".met");
        if ((out = fopen(path, "w")) != 0) {
            sprintf(name, "%s %s", app_name, XMBDFED_VERSION);
            bdf_save_sbit_metrics(out, XmuttFontGridFont(ed->fgrid),
                                  &xmbdfed_opts.font_opts, name);
            fclose(out);
        }
    }

    /*
     * Update the title of the editor window.
     */
    sprintf(title, "%s - %s", app_name, ed->file);
    XtVaSetValues(ed->shell, XmNtitle, title, 0);

    /*
     * Desensitize the save button on the menu.
     */
    XtSetSensitive(ed->filemenu.save, False);

    /*
     * Tell the font grid the font has been saved.
     */
    XmuttFontGridSetModified(ed->fgrid, False);
}

static void
#ifndef _NO_PROTO
OpenExportDialog(Widget w, MXFEditor *ed)
#else
OpenExportDialog(w, ed)
Widget w;
MXFEditor *ed;
#endif
{
    Boolean free_dir;
    Widget fstext;
    int pos1, pos2;
    char *dir, *dp, *ep, *suff;
    bdf_font_t *font;
    XmString s;
    Arg av[2];

    font = XmuttFontGridFont(ed->fgrid);

    suff = 0;
    s = 0;

    switch (ed->filemenu.export_type) {
      case XMBDFED_EXPORT_PSF:
        /*
         * Do some error checking before even starting up the dialog.
         */
        if (font->spacing != BDF_CHARCELL || font->bbx.width > 8) {
            dp = name;
            sprintf(dp, "Unable to export as PSF.");
            dp += strlen(dp);
            if (font->bbx.width > 8) {
                sprintf(dp, "\nFont width should be <= 8 but is %d.",
                        font->bbx.width);
                dp += strlen(dp);
            }
            if (font->spacing != BDF_CHARCELL)
              sprintf(dp, "\nFont is not a character cell font.");
            ErrorDialog(name);
            return;
        }

        sprintf(title, "%s: Export PSF Font", app_name);
        s = XmStringCreateSimple("*.[Pp][Ss][Ff]");
        suff = ".psf";
        break;
      case XMBDFED_EXPORT_HEX:
        sprintf(title, "%s: Export HEX Font", app_name);
        s = XmStringCreateSimple("*.[Hh][Ee][Xx]");
        suff = ".hex";
        break;
    }

    if (exportd == 0) {
        XtSetArg(av[0], XmNtitle, title);
        XtSetArg(av[1], XmNpattern, s);
        exportd = XmCreateFileSelectionDialog(top,
                                              "xmbdfed_export_file_box",
                                              av, 2);
        XmStringFree(s);
        XtAddCallback(exportd, XmNokCallback, ReallyExport, 0);
        XtAddCallback(exportd, XmNapplyCallback, AddFilename,
                      (XtPointer) exportd);
        XtAddCallback(exportd, XmNcancelCallback, DoClose,
                      (XtPointer) XtParent(exportd));
        XtManageChild(exportd);
    } else {
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(XtParent(exportd), av, 1);

        XtSetArg(av[0], XmNpattern, s);
        XtSetValues(exportd, av, 1);
        XmStringFree(s);
    }

    free_dir = False;
    dir = 0;
    if (ed->file != 0) {
        dir = ed->path;
        sprintf(name, "%s", ed->file);
        /*
         * Replace the `.bdf' with the indicated suffix.
         */
        if ((dp = strrchr(name, '.')))
          (void) strcpy(dp, suff);
        else
          (void) strcat(dp, suff);
    } else
      sprintf(name, "unnamed%ld%s", ed->id, suff);

    if (dir == 0) {
        fstext = XmFileSelectionBoxGetChild(exportd, XmDIALOG_FILTER_TEXT);
        dir = XmTextFieldGetString(fstext);
        if ((dp = strrchr(dir, '/')))
          *dp = 0;
        free_dir = True;
    } else {
        /*
         * Force the directory to be set to the one specified by the font.
         */
        s = XmStringCreateSimple(dir);
        XtSetArg(av[0], XmNdirectory, s);
        XtSetValues(exportd, av, 1);
        XmStringFree(s);
    }

    /*
     * Set the filename in the file selection box.
     */
    fstext = XmFileSelectionBoxGetChild(exportd, XmDIALOG_TEXT);
    if ((ep = strchr(name, '.')) == name)
      ep = 0;
    sprintf(title, "%s/%s", dir, name);
    XmTextFieldSetString(fstext, title);
    pos1 = strlen(dir) + 1;
    pos2 = pos1 + ((ep) ? ep - name : strlen(name));
    XmTextFieldSetCursorPosition(fstext, pos1 + strlen(name));
    XmTextFieldSetSelection(fstext, pos1, pos2, CurrentTime);

    if (free_dir)
      XtFree(dir);

    active_editor = ed->id;
    XtPopup(XtParent(exportd), XtGrabNone);
}

static void
#ifndef _NO_PROTO
DoExportPSF(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoExportPSF(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.export_type = XMBDFED_EXPORT_PSF;
    OpenExportDialog(w, ed);
}

static void
#ifndef _NO_PROTO
DoExportHEX(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoExportHEX(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.export_type = XMBDFED_EXPORT_HEX;
    OpenExportDialog(w, ed);
}

void
#ifndef _NO_PROTO
WaitSave(Widget w, MXFEditor *ed)
#else
WaitSave(w, ed)
Widget w;
MXFEditor *ed;
#endif
{
    if (ed->file == 0)
      sprintf(name,
              "\"unnamed%ld\" has been modified.\nDo you want to save?",
              ed->id);
    else
        sprintf(name, "\"%s\" has been modified.\nDo you want to save?",
                ed->file);

    if (AskBooleanQuestion(name, "Yes", "No") == True) {
        save_done = False;
        DoSave(w, (XtPointer) ed->id, 0);
        while (save_done == False)
          XtAppProcessEvent(app, XtIMAll);
    }
}

static void
#ifndef _NO_PROTO
DoCloseEditor(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCloseEditor(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    /*
     * Close all open glyph editors associated with this editor.
     */
    GlyphEditClose(ed);

    /*
     * Now check to see if the font should be saved or not.
     */
    if (XmuttFontGridModified(ed->fgrid) == True)
      WaitSave(w, ed);
            
    XtPopdown(ed->shell);
}

static void
#ifndef _NO_PROTO
ShowEditor(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowEditor(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long id;

    id = (unsigned long) client_data;

    if (id > 0)
      XtPopup(editors[id].shell, XtGrabNone);

    /*
     * Force the font grid to have the focus.
     */
    (void) XmProcessTraversal(editors[id].fgrid, XmTRAVERSE_CURRENT);

    XMapRaised(XtDisplay(editors[id].shell), XtWindow(editors[id].shell));
}

#ifdef HAVE_HBF
static int
#ifndef _NO_PROTO
LoadHBF(char **path, MXFEditor *ed, bdf_font_t **font)
#else
LoadHBF(path, ed, font)
char **path;
MXFEditor *ed;
bdf_font_t **font;
#endif
{
    int plen;
    char *pp, *pe, *file, *ext;
    bdf_font_t *fp;

    /*
     * Get the file name part of the path.
     */
    pp = *path;
    plen = strlen(pp);
    if ((file = strrchr(pp, '/')) == 0)
      file = pp;
    else
      file++;

    if (xmbdfed_opts.progbar) {
        if ((fp = bdf_load_hbf_font(pp, &xmbdfed_opts.font_opts,
                                    UpdateProgressBar, (void *) file)) == 0) {
            sprintf(name, "Problem loading HBF font '%s'.", file);
            ErrorDialog(name);
            return 0;
        }
    } else {
        WatchCursor(ed->fgrid, True);
        if ((fp = bdf_load_hbf_font(pp, &xmbdfed_opts.font_opts, 0, 0)) == 0) {
            WatchCursor(ed->fgrid, False);
            sprintf(name, "Problem loading HBF font '%s'.", file);
            ErrorDialog(name);
            return 0;
        }
        WatchCursor(ed->fgrid, False);
    }

    /*
     * Set up to replace an existing extension with ".bdf" or add a ".bdf"
     * extension.
     */
    if ((ext = strrchr(file, '.')) != 0)
      ext++;
    else
      ext = pp + plen;

    /*
     * Make sure the path has enough storage to hold a ".bdf" extension.
     */
    pe = pp + plen;
    if (pe - ext < 3)
      *path = pp = XtRealloc(pp, plen + 4);

    /*
     * Add or replace the extension.
     */
    if ((ext = strrchr(pp, '.')) == 0)
      ext = pp + plen;
    sprintf(ext, ".bdf");

    *font = fp;
    return 1;
}
#endif /* HAVE_HBF */

/*
 * Load a Linux console or Sun VF font.  There can be up to 3 fonts loaded by
 * one call.
 */
static int
#ifndef _NO_PROTO
LoadConsoleFont(char **path, MXFEditor *ed)
#else
LoadConsoleFont(path, ed)
char **path;
MXFEditor *ed;
#endif
{
    FILE *in;
    MXFEditor *ep;
    XmString unenc, nn;
    int i, nfonts, plen, len;
    char *pp, *file, *ext;
    bdf_font_t *fonts[3];
    Arg av[2];
    XmuttFontGridPageInfoStruct pi;

    unenc = nn = 0;

    /*
     * Get the file name part of the path.
     */
    pp = *path;
    plen = strlen(pp);
    if ((file = strrchr(pp, '/')) == 0)
      file = pp;
    else
      file++;

    if ((ext = strrchr(file, '.')) == 0)
      ext = file + strlen(file);

    if (file != pp)
      plen = (file - 1) - pp;

    if ((in = fopen(pp, "r")) == 0) {
        sprintf(name, "Unable to open console font '%s'.", file);
        ErrorDialog(name);
        return 0;
    }

    if (xmbdfed_opts.progbar) {
        if (bdf_load_console_font(in, &xmbdfed_opts.font_opts,
                                  UpdateProgressBar, (void *) file,
                                  fonts, &nfonts)) {
            fclose(in);
            sprintf(name, "Problem loading console font '%s'.", file);
            ErrorDialog(name);
            return 0;
        }
    } else {
        WatchCursor(ed->fgrid, True);
        if (bdf_load_console_font(in, &xmbdfed_opts.font_opts, 0, 0,
                                  fonts, &nfonts)) {
            fclose(in);
            WatchCursor(ed->fgrid, False);
            sprintf(name, "Problem loading console font '%s'.", file);
            ErrorDialog(name);
            return 0;
        }
        WatchCursor(ed->fgrid, False);
    }
    fclose(in);

    /*
     * Create the two label strings common to all fonts loaded.
     */
    if (nfonts > 0) {
        nn = XmStringCreateLtoR("None\nNone", XmSTRING_DEFAULT_CHARSET);
        unenc = XmStringCreateSimple("Unencoded");
    }

    for (i = 0; i < nfonts; i++) {
        /*
         * Get the next editor.
         */
        ep = (i) ? &editors[MakeEditor(top, 0, False)] : ed;

        if (ep == ed) {
            /*
             * Clear the path and file before copying the new path and
             * file.
             */
            if (ep->path != 0)
              XtFree((char *) ep->path);
            if (ep->file != 0)
              XtFree((char *) ep->file);

            /*
             * If the glyph test dialog uses the font that is about to be
             * replaced, erase it.
             */
            GlyphTestReset(XmuttFontGridFont(ep->fgrid));
        }

        /*
         * Create the default font name.
         */
        if (nfonts == 3) {
            switch (i) {
              case 0: sprintf(ext, "-16"); break;
              case 1: sprintf(ext, "-14"); break;
              case 2: sprintf(ext, "-08"); break;
            }
        }

        /*
         * Set the default font name.
         */
        len = strlen(file);
        fonts[i]->name = (char *) malloc(len + 1);
        (void) memcpy(fonts[i]->name, file, len + 1);

        /*
         * Copy the new path and filenames into the editor.
         */
        ep->path = (char *) XtMalloc(plen + 1);
        (void) memcpy(ep->path, pp, plen);
        ep->path[plen] = 0;
        ep->file = (char *) XtMalloc((ext - file) + 8);
        (void) memcpy(ep->file, file, ext - file);
        ep->file[ext - file] = 0;

        /*
         * Add the extension to the font file name.
         */
        if (nfonts == 3) {
            switch (i) {
              case 0: sprintf(ep->file + (ext - file), "-16.bdf"); break;
              case 1: sprintf(ep->file + (ext - file), "-14.bdf"); break;
              case 2: sprintf(ep->file + (ext - file), "-08.bdf"); break;
            }
        } else
          sprintf(ep->file + (ext - file), ".bdf");

        /*
         * Configure the editor.
         */
        XtSetArg(av[0], XmNbdfFont, fonts[i]);
        XtSetArg(av[1], XmNdisplayUnencoded, False);
        XtSetValues(ep->fgrid, av, 2);

        ep->update_fontname = False;
        XmTextFieldSetString(ep->fontname, XmuttFontGridFontName(ep->fgrid));
        ep->update_fontname = True;

        XtSetArg(av[0], XmNlabelString, nn);
        XtSetValues(ep->glyphinfo, av, 1);

        /*
         * Enable or disable the navigation buttons based on the page
         * info of the font.
         */
        XmuttFontGridPageInfo(ep->fgrid, &pi);

        if (pi.previous_page < 0) {
            XtSetSensitive(ep->prevpage, False);
            XtSetSensitive(ep->firstpage, False);
        } else {
            XtSetSensitive(ep->prevpage, True);
            XtSetSensitive(ep->firstpage, True);
        }

        if (pi.next_page < 0) {
            XtSetSensitive(ep->nextpage, False);
            XtSetSensitive(ep->lastpage, False);
        } else {
            XtSetSensitive(ep->nextpage, True);
            XtSetSensitive(ep->lastpage, True);
        }

        /*
         * Make sure the View menu is updated so the Encoded/Unencoded
         * menu item says the correct thing.
         */
        XtSetArg(av[0], XmNlabelString, unenc);
        XtSetValues(ep->viewmenu.unencoded, av, 1);

        /*
         * Set the current page number in the text field.
         */
        sprintf(name, "%ld", pi.current_page);
        XmTextFieldSetString(ep->pageno, name);
        XmTextFieldSetCursorPosition(ep->pageno, strlen(name));

        /*
         * Enable or disable the Messages button.
         */
        if (XmuttFontGridFontMessages(ep->fgrid) == 0)
          XtSetSensitive(ep->viewmenu.acmsgs, False);
        else
          XtSetSensitive(ep->viewmenu.acmsgs, True);

        /*
         * Enable the Save menu option if the font has been modified.
         * Also update the title.
         */
        if (XmuttFontGridModified(ep->fgrid) == True) {
            if (ep->file == 0)
              sprintf(title, "%s - (unnamed%ld) [modified]", app_name, ep->id);
            else
              sprintf(title, "%s - %s [modified]", app_name, ep->file);
            XtSetArg(av[0], XmNtitle, title);
            XtSetValues(ep->shell, av, 1);
            XtSetSensitive(ep->filemenu.save, True);
        } else {
            if (ep->file == 0)
              sprintf(title, "%s - (unnamed%ld)", app_name, ep->id);
            else
              sprintf(title, "%s - %s", app_name, ep->file);
            XtSetArg(av[0], XmNtitle, title);
            XtSetValues(ep->shell, av, 1);
            XtSetSensitive(ep->filemenu.save, False);
        }
    }

    /*
     * Free up the allocated strings if there were fonts loaded.
     */
    if (nfonts > 0) {
        XmStringFree(nn);
        XmStringFree(unenc);
    }

    return 1;
}

/*
 * Load a Metafont PK or GF font.
 */
static int
#ifndef _NO_PROTO
LoadPKGFFont(char **path, MXFEditor *ed, bdf_font_t **font)
#else
LoadPKGFFont(path, ed, font)
char **path;
MXFEditor *ed;
bdf_font_t **font;
#endif
{
    FILE *in;
    int plen;
    char *pp, *file, *ext, *pe;
    bdf_font_t *fp;

    /*
     * Get the file name part of the path.
     */
    pp = *path;
    plen = strlen(pp);
    if ((file = strrchr(pp, '/')) == 0)
      file = pp;
    else
      file++;

    if ((in = fopen(pp, "r")) == 0) {
        sprintf(name, "Unable to open PK/GF font '%s'.", file);
        ErrorDialog(name);
        return 0;
    }

    if (xmbdfed_opts.progbar) {
        if (bdf_load_mf_font(in, &xmbdfed_opts.font_opts,
                             UpdateProgressBar, (void *) file, &fp)) {
            fclose(in);
            sprintf(name, "Problem loading PK/GF font '%s'.", file);
            ErrorDialog(name);
            return 0;
        }
    } else {
        WatchCursor(ed->fgrid, True);
        if (bdf_load_mf_font(in, &xmbdfed_opts.font_opts, 0, 0, &fp)) {
            fclose(in);
            WatchCursor(ed->fgrid, False);
            sprintf(name, "Problem loading PK/GF font '%s'.", file);
            ErrorDialog(name);
            return 0;
        }
        WatchCursor(ed->fgrid, False);
    }
    fclose(in);

    *font = fp;

    /*
     * Need to munge the font name and file name around to get something
     * usable.
     */
    if ((ext = strrchr(file, '.')) != 0)
      ext++;
    else
      ext = pp + plen;

    /*
     * Make sure the path has enough storage to hold the new file name.
     */
    pe = pp + plen;
    if (pe - ext < 9)
      *path = pp = XtRealloc(pp, plen + 10);

    /*
     * Locate the file name again.
     */
    if ((file = strrchr(pp, '/')) == 0)
      file = pp;
    else
      file++;

    /*
     * Add or replace the rest of the file name.
     */
    if ((ext = strrchr(file, '.')) == 0)
      ext = pp + plen;

    /*
     * Use the horizontal resolution of the font as part of the name to
     * distinguish those with the same name.
     */
    sprintf(ext, "-%ld.bdf", fp->resolution_x);

    /*
     * Locate the extension again.
     */
    if ((ext = strrchr(file, '.')) == 0) {
        /*
         * Use the filename as the font name.
         */
        plen = strlen(file) + 1;
        fp->name = malloc(plen);
        (void) memcpy(fp->name, file, plen);
    } else {
        /*
         * Copy everything up to the extension.
         */
        fp->name = malloc((ext - file) + 1);
        (void) memcpy(fp->name, file, ext - file);
        fp->name[ext - file] = 0;
    }

    return 1;
}

static void
#ifndef _NO_PROTO
ReallyOpen(Widget w, XtPointer client_data, XtPointer call_data)
#else
ReallyOpen(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int res;
    MXFEditor *ed;
    char *path, *file, *fp;
    FILE *in;
    bdf_font_t *f;
    XmFileSelectionBoxCallbackStruct *cb;
    XmString s;
    XmuttFontGridPageInfoStruct pi;
    Arg av[2];

    ed = &editors[active_editor];
    cb = (XmFileSelectionBoxCallbackStruct *) call_data;

    path = 0;
    XmStringGetLtoR(cb->value, XmSTRING_DEFAULT_CHARSET, &path);

    switch (ed->filemenu.open_type) {
      case XMBDFED_OPEN_BDF:
        /*
         * Set the filename that will be used for the progress bar.
         */
        if ((fp = strrchr(path, '/')) == 0)
          fp = path;
        else
          fp++;

        if ((in = fopen(path, "r")) == 0) {
            sprintf(name, "Unable to open BDF font '%s'.", path);
            ErrorDialog(name);
            XtFree(path);
            return;
        }
        if (xmbdfed_opts.progbar)
          f = bdf_load_font(in, &xmbdfed_opts.font_opts, UpdateProgressBar,
                            (void *) fp);
        else {
            WatchCursor(ed->fgrid, True);
            f = bdf_load_font(in, &xmbdfed_opts.font_opts, 0, 0);
            WatchCursor(ed->fgrid, False);
        }

        fclose(in);
        if (f == 0) {
            sprintf(name, "Problem loading BDF font '%s'.", path);
            ErrorDialog(name);
            XtFree(path);
            return;
        }
        break;
#ifdef HAVE_HBF
      case XMBDFED_OPEN_HBF:
        if (!LoadHBF(&path, ed, &f)) {
            XtFree(path);
            return;
        }
        break;
#endif /* HAVE_HBF */
      case XMBDFED_OPEN_CONSOLE:
        /*
         * This routine takes care of updating all the font grids because
         * there may be 1 or 3 fonts.
         */
        if (!LoadConsoleFont(&path, ed))
          XtFree(path);
        else
          XtPopdown(XtParent(opend));
        return;
      case XMBDFED_OPEN_PKGF:
        if (!LoadPKGFFont(&path, ed, &f)) {
            XtFree(path);
            return;
        }
        break;
      case XMBDFED_OPEN_FNT:
        /*
         * This routine may load more than 1 font, so it takes care of
         * creating and updating the font grids.
         */
        if (!(res = LoadWindowsFont(&path, ed, &f))) {
            XtFree(path);
            return;
        }
        if (res == 2) {
            /*
             * A return value of 2 means the fonts loaded successfuly so the
             * open dialog should be popped down.
             */
            XtPopdown(XtParent(opend));
            return;
        }
        /*
         * A return value of 1 will drop through and cause the grid to be
         * updated from the font just loaded.
         */
        break;
#if HAVE_FREETYPE
      case XMBDFED_OPEN_TTF:
        if (!LoadTrueTypeFont(&path, ed, &f)) {
            XtFree(path);
            return;
        }
#endif /* HAVE_FREETYPE */
    }

    /*
     * Hide the file selection dialog before anything else happens.
     */
    XtPopdown(XtParent(opend));

    /*
     * If the glyph test dialog uses the font that is about to be
     * replaced, erase it.
     */
    GlyphTestReset(XmuttFontGridFont(ed->fgrid));

    /*
     * Determine the new path and filename.
     */
    if (ed->path != 0)
      XtFree(ed->path);
    if (ed->file != 0)
      XtFree(ed->file);

    file = strrchr(path, '/');
    if (file) {
        *file++ = 0;
        ed->path = XtMalloc(strlen(path) + 1);
        (void) strcpy(ed->path, path);
    } else {
        file = path;
        ed->path = XtMalloc(2);
        ed->path[0] = '.';
        ed->path[1] = 0;
    }
    ed->file = XtMalloc(strlen(file) + 1);
    (void) strcpy(ed->file, file);

    /*
     * Free up the path from the file selection dialog.
     */
    XtFree(path);

    /*
     * Change the font in the font editor.
     */
    XtSetArg(av[0], XmNbdfFont, f);
    XtSetArg(av[1], XmNdisplayUnencoded, False);
    XtSetValues(ed->fgrid, av, 2);

    /*
     * Update the font name and glyph labels.
     */
    ed->update_fontname = False;
    XmTextFieldSetString(ed->fontname, XmuttFontGridFontName(ed->fgrid));
    ed->update_fontname = True;

    s = XmStringCreateLtoR("None\nNone", XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ed->glyphinfo, av, 1);
    XmStringFree(s);

    /*
     * Enable or disable the navigation buttons based on the page info
     * of the font.
     */
    XmuttFontGridPageInfo(ed->fgrid, &pi);

    if (pi.previous_page < 0) {
        XtSetSensitive(ed->prevpage, False);
        XtSetSensitive(ed->firstpage, False);
    } else {
        XtSetSensitive(ed->prevpage, True);
        XtSetSensitive(ed->firstpage, True);
    }

    if (pi.next_page < 0) {
        XtSetSensitive(ed->nextpage, False);
        XtSetSensitive(ed->lastpage, False);
    } else {
        XtSetSensitive(ed->nextpage, True);
        XtSetSensitive(ed->lastpage, True);
    }

    /*
     * Make sure the View menu is updated so the Encoded/Unencoded
     * menu item says the correct thing.
     */
    s = XmStringCreateSimple("Unencoded");
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ed->viewmenu.unencoded, av, 1);
    XmStringFree(s);

    /*
     * Set the current page number in the text field.
     */
    sprintf(name, "%ld", pi.current_page);
    XmTextFieldSetString(ed->pageno, name);
    XmTextFieldSetCursorPosition(ed->pageno, strlen(name));

    /*
     * Enable or disable the Messages button.
     */
    if (XmuttFontGridFontMessages(ed->fgrid) == 0)
      XtSetSensitive(ed->viewmenu.acmsgs, False);
    else
      XtSetSensitive(ed->viewmenu.acmsgs, True);

    /*
     * Enable the Save menu option if the font has been modified.  Also update
     * the title.
     */
    if (XmuttFontGridModified(ed->fgrid) == True) {
        if (ed->file == 0)
          sprintf(title, "%s - (unnamed%ld) [modified]", app_name, ed->id);
        else
          sprintf(title, "%s - %s [modified]", app_name, ed->file);
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(ed->shell, av, 1);
        XtSetSensitive(ed->filemenu.save, True);
    } else {
        if (ed->file == 0)
          sprintf(title, "%s - (unnamed%ld)", app_name, ed->id);
        else
          sprintf(title, "%s - %s", app_name, ed->file);
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(ed->shell, av, 1);
        XtSetSensitive(ed->filemenu.save, False);
    }
}

static void
#ifndef _NO_PROTO
OpenImportDialog(Widget w, MXFEditor *ed)
#else
OpenImportDialog(w, ed)
Widget w;
MXFEditor *ed;
#endif
{
    XmString s, dir;
    Cardinal ac;
    Arg av[4];

    if (XmuttFontGridModified(ed->fgrid) == True)
      WaitSave(w, ed);

    s = 0;

    switch (ed->filemenu.open_type) {
      case XMBDFED_OPEN_BDF:
        sprintf(title, "%s: Open BDF Font", app_name);
        s = XmStringCreateSimple("*.[Bb][Dd][Ff]");
        break;
#ifdef HAVE_HBF
      case XMBDFED_OPEN_HBF:
        sprintf(title, "%s: Import HBF Font", app_name);
        s = XmStringCreateSimple("*.[Hh][Bb][Ff]");
        break;
#endif /* HAVE_HBF */
      case XMBDFED_OPEN_CONSOLE:
        sprintf(title, "%s: Import Console Font", app_name);
        s = XmStringCreateSimple("*");
        break;
      case XMBDFED_OPEN_PKGF:
        sprintf(title, "%s: Import PK/GF Font", app_name);
        s = XmStringCreateSimple("*[PpGg][KkFf]");
        break;
#if HAVE_FREETYPE
      case XMBDFED_OPEN_TTF:
        sprintf(title, "%s: Import TrueType Font", app_name);
        s = XmStringCreateSimple("*.[Tt][Tt][FfCcEe]");
        break;
#endif /* HAVE_FREETYPE */
      case XMBDFED_OPEN_FNT:
        sprintf(title, "%s: Import Windows Font", app_name);
        s = XmStringCreateSimple("*.[FfEeDd][OoNnXxLl][NnTtEeLl]");
    }

    dir = (ed->path != 0) ? XmStringCreateSimple(ed->path) : 0;

    if (opend == 0) {
        ac = 0;
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetArg(av[ac], XmNpattern, s); ac++;
        if (dir != 0) {
            XtSetArg(av[ac], XmNdirectory, dir); ac++;
        }
        opend = XmCreateFileSelectionDialog(top, "xmbdfed_open_file_sel_box",
                                            av, ac);
        XmStringFree(s);
        XtAddCallback(opend, XmNokCallback, ReallyOpen, 0);
        XtAddCallback(opend, XmNcancelCallback, DoClose,
                      (XtPointer) XtParent(opend));
        XtManageChild(opend);
    } else {
        /*
         * Update the title and file match pattern based on the open type.
         */
        ac = 0;
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetValues(XtParent(opend), av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNpattern, s); ac++;
        if (dir != 0) {
            XtSetArg(av[ac], XmNdirectory, dir); ac++;
        }
        XtSetValues(opend, av, ac);
        XmStringFree(s);
    }

    if (dir != 0)
      XmStringFree(dir);

    active_editor = ed->id;
    XtPopup(XtParent(opend), XtGrabNone);
}

static void
#ifndef _NO_PROTO
DoOpen(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoOpen(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.open_type = XMBDFED_OPEN_BDF;
    OpenImportDialog(w, ed);
}

#ifdef HAVE_HBF
static void
#ifndef _NO_PROTO
ImportHBF(Widget w, XtPointer client_data, XtPointer call_data)
#else
ImportHBF(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.open_type = XMBDFED_OPEN_HBF;
    OpenImportDialog(w, ed);
}
#endif /* HAVE_HBF */

static void
#ifndef _NO_PROTO
ImportConsole(Widget w, XtPointer client_data, XtPointer call_data)
#else
ImportConsole(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.open_type = XMBDFED_OPEN_CONSOLE;
    OpenImportDialog(w, ed);
}

static void
#ifndef _NO_PROTO
ImportPkGf(Widget w, XtPointer client_data, XtPointer call_data)
#else
ImportPkGf(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.open_type = XMBDFED_OPEN_PKGF;
    OpenImportDialog(w, ed);
}

static void
#ifndef _NO_PROTO
ImportFNT(Widget w, XtPointer client_data, XtPointer call_data)
#else
ImportFNT(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.open_type = XMBDFED_OPEN_FNT;
    OpenImportDialog(w, ed);
}

#ifdef HAVE_FREETYPE

static void
#ifndef _NO_PROTO
ImportTTF(Widget w, XtPointer client_data, XtPointer call_data)
#else
ImportTTF(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    ed->filemenu.open_type = XMBDFED_OPEN_TTF;
    OpenImportDialog(w, ed);
}

#endif /* HAVE_FREETYPE */

static void
#ifndef _NO_PROTO
DoQuit(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoQuit(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long i, cnt;
    MXFEditor *ed;

    exiting = True;

    /*
     * Go through and find out if any of the fonts have been modified.  Close
     * all open glyph editors at the same time which allows modified glyphs to
     * be updated.
     */
    for (i = cnt = 0, ed = editors; i < num_editors; i++, ed++) {
        GlyphEditClose(ed);
        if (XmuttFontGridModified(ed->fgrid) == True)
          cnt++;
    }

    if (cnt > 0) {
        if (cnt == 1) {
            if (num_editors > 1)
              sprintf(name, "A font has been modified.  Save?");
            else
              sprintf(name, "The font has been modified.  Save?");
        } else
          sprintf(name, "Some fonts have been modified.  Save?");
        if (AskBooleanQuestion(name, "Save And Exit",
                               "Exit Without Save") == True) {
            for (i = 0, ed = editors; i < num_editors; i++, ed++) {
                if (XmuttFontGridModified(ed->fgrid) == True) {
                    /*
                     * Make sure to only call DoSave() if the font has an
                     * associated file.
                     */
                    if (cnt == 1 && ed->file != 0)
                      DoSave(w, (XtPointer) ed->id, 0);
                    else
                      WaitSave(w, ed);
                }
            }
        }
    }

    /*
     * One of the save operations failed so we can't exit yet.
     */
    if (exiting == False)
      return;

    /*
     * Verify an exit is really wanted.
     */
    if (xmbdfed_opts.really_exit &&
        AskBooleanQuestion("Really Exit?", "Yes", "No") == False)
      return;

    /*
     * Go through and free up all the fonts.
     */
    for (i = 0, ed = editors; i < num_editors; i++, ed++) {
        if (ed->path != 0)
          XtFree(ed->path);
        if (ed->file != 0)
          XtFree(ed->file);
    }

    if (num_editors > 0)
      XtFree((char *) editors);
    num_editors = 0;

    /*
     * Clean up any glyph editors that were created.
     */
    GlyphEditCleanup();

    /*
     * Free up the colors if they were allocated.
     */
    if (xmbdfed_opts.pixels[0] != ~0)
      XFreeColors(XtDisplay(top),
                  XDefaultColormap(XtDisplay(top),
                                   XDefaultScreen(XtDisplay(top))),
                  xmbdfed_opts.pixels, 4, 0);
    if (xmbdfed_opts.pixels[4] != ~0)
      XFreeColors(XtDisplay(top),
                  XDefaultColormap(XtDisplay(top),
                                   XDefaultScreen(XtDisplay(top))),
                  &xmbdfed_opts.pixels[4], 16, 0);

    XtDestroyWidget(top);

    XtDestroyApplicationContext(app);

    bdf_cleanup();

    /*
     * Free up the strings in the options structure if they exist.
     */
    if (xmbdfed_opts.accelerator != 0)
      XtFree(xmbdfed_opts.accelerator);

    if (xmbdfed_opts.accelerator_text != 0)
      XtFree(xmbdfed_opts.accelerator_text);

    exit(0);
}

static void
#ifndef _NO_PROTO
CodePageChangeCallback(Widget w, XtPointer client_data, XtPointer call_data)
#else
CodePageChangeCallback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    long code;

    ed = &editors[(unsigned long) client_data];
    code = (long) call_data;

    /*
     * Set the code number in the text field.
     */
    switch(XmuttFontGridCodeBase(ed->fgrid)) {
      case 8: sprintf(name, "%lo", code); break;
      case 10: sprintf(name, "%ld", code); break;
      case 16: sprintf(name, "%lx", code); break;
    }
    XmTextFieldSetString(ed->code, name);
    XmTextFieldSetCursorPosition(ed->code, strlen(name));
}

static void
#ifndef _NO_PROTO
PageChangeCallback(Widget w, XtPointer client_data, XtPointer call_data)
#else
PageChangeCallback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttFontGridPageInfoStruct *pi;

    ed = &editors[(unsigned long) client_data];
    pi = (XmuttFontGridPageInfoStruct *) call_data;

    if (pi->previous_page == -1) {
        XtSetSensitive(ed->prevpage, False);
        XtSetSensitive(ed->firstpage, False);
    } else {
        XtSetSensitive(ed->prevpage, True);
        XtSetSensitive(ed->firstpage, True);
    }

    if (pi->next_page == -1) {
        XtSetSensitive(ed->nextpage, False);
        XtSetSensitive(ed->lastpage, False);
    } else {
        XtSetSensitive(ed->nextpage, True);
        XtSetSensitive(ed->lastpage, True);
    }

    /*
     * Set the page number in the text field.
     */
    sprintf(name, "%ld", pi->current_page);
    XmTextFieldSetString(ed->pageno, name);
    XmTextFieldSetCursorPosition(ed->pageno, strlen(name));
}

static void
#ifndef _NO_PROTO
UpdateGlyphInfo(Widget w, XtPointer client_data, XtPointer call_data)
#else
UpdateGlyphInfo(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    short as, ds, rt, lt;
    unsigned int b1, b2, b3, b4;
    long start, end;
    XmuttFontGridSelectionStruct *cb;
    XmString s;
    Arg av[1];

    ed = &editors[(unsigned long) client_data];
    cb = (XmuttFontGridSelectionStruct *) call_data;

    b1 = (cb->start >> 8) & 0xff;
    b2 = cb->start & 0xff;

    if (cb->start == cb->end) {
        as = ds = rt = lt = 0;
        if (cb->num_glyphs != 0 && cb->glyphs != 0) {
            as = cb->glyphs->bbx.ascent;
            ds = cb->glyphs->bbx.descent;
            rt = cb->glyphs->bbx.width + cb->glyphs->bbx.x_offset;
            lt = cb->glyphs->bbx.x_offset;
            if (cb->glyphs->name != 0)
              (void) strcpy(name, cb->glyphs->name);
            else
              sprintf(name, "char%ld", cb->glyphs->encoding);

            /*
             * If the glyph test dialog is active, send it the glyph if this
             * is an end selection event.
             */
            if (cb->reason == XmuttFG_END_SELECTION)
              GlyphTestAddGlyph(XmuttFontGridFont(ed->fgrid), cb->glyphs);
        } else
          sprintf(name, "char%ld", cb->start);

        switch (cb->base) {
          case 8:
            sprintf(title, OGINFO, name, cb->start, b1, b2, as, ds, rt, lt);
            break;
          case 10:
            sprintf(title, DGINFO, name, cb->start, b1, b2, as, ds, rt, lt);
            break;
          case 16:
            sprintf(title, HGINFO, name, cb->start, b1, b2, as, ds, rt, lt);
            break;
        }
    } else {
        if (cb->end < cb->start) {
            start = cb->end;
            end = cb->start;
        } else {
            start = cb->start;
            end = cb->end;
        }
        b3 = (end >> 8) & 0xff;
        b4 = end & 0xff;
        switch (cb->base) {
          case 8:
            sprintf(title, "Selection\n%lo (%o,%o) - %lo (%o,%o)",
                    start, b1, b2, end, b3, b4);
            break;
          case 10:
            sprintf(title, "Selection\n%ld (%d,%d) - %ld (%d,%d)",
                    start, b1, b2, end, b3, b4);
            break;
          case 16:
            sprintf(title, "Selection\n%04lX (%02X,%02X) - %04lX (%02X,%02X)",
                    start, b1, b2, end, b3, b4);
            break;
        }
    }
    s = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ed->glyphinfo, av, 1);
    XmStringFree(s);
}

static void
#ifndef _NO_PROTO
UpdateModified(Widget w, XtPointer client_data, XtPointer call_data)
#else
UpdateModified(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttFontGridModifiedCallbackStruct *cb;
    Arg av[1];

    ed = &editors[(unsigned long) client_data];
    cb = (XmuttFontGridModifiedCallbackStruct *) call_data;

    if (XmuttFontGridModified(ed->fgrid) == True) {
        if (ed->file == 0)
          sprintf(title, "%s - (unnamed%ld) [modified]", app_name, ed->id);
        else
          sprintf(title, "%s - %s [modified]", app_name, ed->file);
        XtSetSensitive(ed->filemenu.save, True);
    } else {
        if (ed->file == 0)
          sprintf(title, "%s - (unnamed%ld)", app_name, ed->id);
        else
          sprintf(title, "%s - %s", app_name, ed->file);
        XtSetSensitive(ed->filemenu.save, False);
    }

    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ed->shell, av, 1);

    /*
     * Check to see if the font name needs to be changed in the font name
     * text field.  Inhibit updates by the routine that tracks when the
     * font name text field changes.
     */
    if (ed->update_fontname == True && cb->reason == XmuttFG_NAME_CHANGE) {
        ed->update_fontname = False;
        XmTextFieldSetString(ed->fontname, cb->font_name);
        ed->update_fontname = True;
    }

    if (cb->reason == XmuttFG_RESIZE)
      /*
       * On resizes, the glyph test dialog needs to have the bounding
       * box updated.
       */
      GlyphTestResize();

    /*
     * If the callback was due to a replace action, then alert the glyph test
     * dialog if it is active.
     */
    if (cb->reason == XmuttFG_REPLACE)
      GlyphTestRedisplay();
}

static void
#ifndef _NO_PROTO
GotoCode(Widget w, XtPointer client_data, XtPointer call_data)
#else
GotoCode(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    long n;
    MXFEditor *ed;
    char *val;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True)
      ed->last_upgno = pi.current_page;
    else
      ed->last_pgno = pi.current_page;

    val = XmTextFieldGetString(ed->code);
    switch(XmuttFontGridCodeBase(ed->fgrid)) {
      case 8: sscanf(val, "%lo", &n); break;
      case 10: sscanf(val, "%ld", &n); break;
      case 16: sscanf(val, "%lx", &n); break;
    }
    XtFree(val);
    XmuttFontGridGotoCodePage(ed->fgrid, n);
}

static void
#ifndef _NO_PROTO
GotoPage(Widget w, XtPointer client_data, XtPointer call_data)
#else
GotoPage(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    long n;
    MXFEditor *ed;
    char *val;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True)
      ed->last_upgno = pi.current_page;
    else
      ed->last_pgno = pi.current_page;

    val = XmTextFieldGetString(ed->pageno);
    n = atol(val);
    XtFree(val);
    XmuttFontGridGotoPage(ed->fgrid, n);
}

static void
#ifndef _NO_PROTO
GotoFirstPage(Widget w, XtPointer client_data, XtPointer call_data)
#else
GotoFirstPage(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True)
      ed->last_upgno = pi.current_page;
    else
      ed->last_pgno = pi.current_page;

    XmuttFontGridFirstPage(ed->fgrid);
}

static void
#ifndef _NO_PROTO
GotoNextPage(Widget w, XtPointer client_data, XtPointer call_data)
#else
GotoNextPage(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmPushButtonCallbackStruct *cb;
    Boolean blanks;
    Arg av[1];
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True)
      ed->last_upgno = pi.current_page;
    else
      ed->last_pgno = pi.current_page;

    XtSetArg(av[0], XmNskipBlankPages, &blanks);
    XtGetValues(ed->fgrid, av, 1);

    XtSetArg(av[0], XmNskipBlankPages, blanks);

    cb = (XmPushButtonCallbackStruct *) call_data;
    if (cb->event->type == ButtonPress &&
        (cb->event->xbutton.state & ShiftMask)) {
        if (blanks == True)
          XtSetArg(av[0], XmNskipBlankPages, False);
        else
          XtSetArg(av[0], XmNskipBlankPages, True);
    }

    XtSetValues(ed->fgrid, av, 1);

    XmuttFontGridNextPage(ed->fgrid);
}

static void
#ifndef _NO_PROTO
GotoPrevPage(Widget w, XtPointer client_data, XtPointer call_data)
#else
GotoPrevPage(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmPushButtonCallbackStruct *cb;
    Boolean blanks;

    Arg av[1];
    XmuttFontGridPageInfoStruct pi;

    cb = (XmPushButtonCallbackStruct *) call_data;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True)
      ed->last_upgno = pi.current_page;
    else
      ed->last_pgno = pi.current_page;

    XtSetArg(av[0], XmNskipBlankPages, &blanks);
    XtGetValues(ed->fgrid, av, 1);

    XtSetArg(av[0], XmNskipBlankPages, blanks);

    if (cb->event->type == ButtonPress &&
        (cb->event->xbutton.state & ShiftMask)) {
        if (blanks == True)
          XtSetArg(av[0], XmNskipBlankPages, False);
        else
          XtSetArg(av[0], XmNskipBlankPages, True);
    }

    XtSetValues(ed->fgrid, av, 1);

    XmuttFontGridPreviousPage(ed->fgrid);
}

static void
#ifndef _NO_PROTO
GotoLastPage(Widget w, XtPointer client_data, XtPointer call_data)
#else
GotoLastPage(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True)
      ed->last_upgno = pi.current_page;
    else
      ed->last_pgno = pi.current_page;

    XmuttFontGridLastPage(ed->fgrid);
}

static void
#ifndef _NO_PROTO
GotoOtherPage(Widget w, XtPointer client_data, XtPointer call_data)
#else
GotoOtherPage(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    long opage;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True) {
        opage = ed->last_upgno;
        ed->last_upgno = pi.current_page;
    } else {
        opage = ed->last_pgno;
        ed->last_pgno = pi.current_page;
    }

    if (opage != -1 && opage != pi.current_page)
      XmuttFontGridGotoPage(ed->fgrid, opage);
}

static void
#ifndef _NO_PROTO
DisplayOctal(Widget w, XtPointer client_data, XtPointer call_data)
#else
DisplayOctal(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned int s1, s2, s3, s4;
    short as, ds, rt, lt;
    MXFEditor *ed;
    XmToggleButtonCallbackStruct *cb;
    XmString s;
    Arg av[1];
    XmuttFontGridSelectionStruct sinfo;

    ed = &editors[(unsigned long) client_data];
    cb = (XmToggleButtonCallbackStruct *) call_data;

    if (cb->set) {
        XmuttFontGridChangeCodeBase(ed->fgrid, 8);
        XmuttFontGridSelectionInfo(ed->fgrid, &sinfo);
        if (sinfo.start != -1 && sinfo.end != -1) {
            s1 = (sinfo.start >> 8) & 0xff;
            s2 = sinfo.start & 0xff;
            if (sinfo.start == sinfo.end) {
                as = ds = rt = lt = 0;
                if (sinfo.num_glyphs != 0) {
                    as = sinfo.glyphs->bbx.ascent;
                    ds = sinfo.glyphs->bbx.descent;
                    rt = sinfo.glyphs->bbx.width + sinfo.glyphs->bbx.x_offset;
                    lt = sinfo.glyphs->bbx.x_offset;
                    if (sinfo.glyphs->name != 0)
                      (void) strcpy(name, sinfo.glyphs->name);
                    else
                      sprintf(name, "char%ld", sinfo.glyphs->encoding);
                } else
                  sprintf(name, "char%ld", sinfo.start);
                sprintf(title, OGINFO, name, sinfo.start,
                        s1, s2, as, ds, rt, lt);
            } else {
                s3 = (sinfo.end >> 8) & 0xff;
                s4 = sinfo.end & 0xff;
                sprintf(title, "%lo (%o,%o) - %lo (%o,%o)",
                        sinfo.start, s1, s2, sinfo.end, s3, s4);
            }
            s = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
            XtSetArg(av[0], XmNlabelString, s);
            XtSetValues(ed->glyphinfo, av, 1);
            XmStringFree(s);
        }

        /*
         * Update the glyph editors.
         */
        GlyphEditChangeBase(ed->id, 8);
    }
}

static void
#ifndef _NO_PROTO
DisplayDecimal(Widget w, XtPointer client_data, XtPointer call_data)
#else
DisplayDecimal(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned int s1, s2, s3, s4;
    short as, ds, rt, lt;
    MXFEditor *ed;
    XmToggleButtonCallbackStruct *cb;
    XmString s;
    Arg av[1];
    XmuttFontGridSelectionStruct sinfo;

    ed = &editors[(unsigned long) client_data];
    cb = (XmToggleButtonCallbackStruct *) call_data;

    if (cb->set) {
        XmuttFontGridChangeCodeBase(ed->fgrid, 10);
        XmuttFontGridSelectionInfo(ed->fgrid, &sinfo);
        if (sinfo.start != -1 && sinfo.end != -1) {
            s1 = (sinfo.start >> 8) & 0xff;
            s2 = sinfo.start & 0xff;
            if (sinfo.start == sinfo.end) {
                as = ds = rt = lt = 0;
                if (sinfo.num_glyphs != 0) {
                    as = sinfo.glyphs->bbx.ascent;
                    ds = sinfo.glyphs->bbx.descent;
                    rt = sinfo.glyphs->bbx.width + sinfo.glyphs->bbx.x_offset;
                    lt = sinfo.glyphs->bbx.x_offset;
                    if (sinfo.glyphs->name != 0)
                      (void) strcpy(name, sinfo.glyphs->name);
                    else
                      sprintf(name, "char%ld", sinfo.glyphs->encoding);
                } else
                  sprintf(name, "char%ld", sinfo.start);
                sprintf(title, DGINFO, name, sinfo.start, s1, s2,
                        as, ds, rt, lt);
            } else {
                s3 = (sinfo.end >> 8) & 0xff;
                s4 = sinfo.end & 0xff;
                sprintf(title, "%ld (%d,%d) - %ld (%d,%d)",
                        sinfo.start, s1, s2, sinfo.end, s3, s4);
            }
            s = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
            XtSetArg(av[0], XmNlabelString, s);
            XtSetValues(ed->glyphinfo, av, 1);
            XmStringFree(s);
        }

        /*
         * Update the glyph editors.
         */
        GlyphEditChangeBase(ed->id, 10);
    }
}

static void
#ifndef _NO_PROTO
DisplayHex(Widget w, XtPointer client_data, XtPointer call_data)
#else
DisplayHex(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned int s1, s2, s3, s4;
    short as, ds, rt, lt;
    MXFEditor *ed;
    XmToggleButtonCallbackStruct *cb;
    XmString s;
    Arg av[1];
    XmuttFontGridSelectionStruct sinfo;

    ed = &editors[(unsigned long) client_data];
    cb = (XmToggleButtonCallbackStruct *) call_data;

    if (cb->set) {
        XmuttFontGridChangeCodeBase(ed->fgrid, 16);
        XmuttFontGridSelectionInfo(ed->fgrid, &sinfo);
        if (sinfo.start != -1 && sinfo.end != -1) {
            s1 = (sinfo.start >> 8) & 0xff;
            s2 = sinfo.start & 0xff;
            if (sinfo.start == sinfo.end) {
                as = ds = rt = lt = 0;
                if (sinfo.num_glyphs != 0) {
                    as = sinfo.glyphs->bbx.ascent;
                    ds = sinfo.glyphs->bbx.descent;
                    rt = sinfo.glyphs->bbx.width + sinfo.glyphs->bbx.x_offset;
                    lt = sinfo.glyphs->bbx.x_offset;
                    if (sinfo.glyphs->name != 0)
                      (void) strcpy(name, sinfo.glyphs->name);
                    else
                      sprintf(name, "char%ld", sinfo.glyphs->encoding);
                } else
                  sprintf(name, "char%ld", sinfo.start);
                sprintf(title, HGINFO, name, sinfo.start, s1, s2,
                        as, ds, rt, lt);
            } else {
                s3 = (sinfo.end >> 8) & 0xff;
                s4 = sinfo.end & 0xff;
                sprintf(title, "%04lX (%02X,%02X) - %04lX (%02X,%02X)",
                        sinfo.start, s1, s2, sinfo.end, s3, s4);
            }
            s = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
            XtSetArg(av[0], XmNlabelString, s);
            XtSetValues(ed->glyphinfo, av, 1);
            XmStringFree(s);
        }

        /*
         * Update the glyph editors.
         */
        GlyphEditChangeBase(ed->id, 16);
    }
}

static void
#ifndef _NO_PROTO
ToggleEncodedView(Widget w, XtPointer client_data, XtPointer call_data)
#else
ToggleEncodedView(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    Widget pb;
    XmString s;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    pb = (Widget) client_data;    

    XmuttFontGridPageInfo(ed->fgrid, &pi);

    if (pi.unencoded == True) {
        s = XmStringCreateSimple("Unencoded");
        XmuttFontGridViewUnencoded(ed->fgrid, False);
    } else {
        s = XmStringCreateSimple("Encoded");
        XmuttFontGridViewUnencoded(ed->fgrid, True);
    }
    XtVaSetValues(ed->viewmenu.unencoded, XmNlabelString, s, 0);
    XmStringFree(s);
}

static void
#ifndef _NO_PROTO
DoFontGridView(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontGridView(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmString s;
    unsigned char vert;
    Arg av[1];

    ed = &editors[(unsigned long) client_data];

    XtSetArg(av[0], XmNorientation, &vert);
    XtGetValues(ed->fgrid, av, 1);

    if (vert == XmHORIZONTAL) {
        s = XmStringCreateSimple("Horizontal View");
        vert = XmVERTICAL;
    } else {
        s = XmStringCreateSimple("Vertical View");
        vert = XmHORIZONTAL;
    }

    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ed->viewmenu.vert, av, 1);
    XmStringFree(s);

    XtSetArg(av[0], XmNorientation, vert);
    XtSetValues(ed->fgrid, av, 1);
}

static void
#ifndef _NO_PROTO
ShowMessages(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowMessages(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    Widget frame, pb;
    char *s;
    Cardinal ac;
    Arg av[8];

    ed = &editors[(unsigned long) client_data];

    if (acmsg_shell == 0) {
        /*
         * Create the auto-correction message dialog.
         */
        ac = 0;
        XtSetArg(av[ac], XmNallowShellResize, True); ac++;
        XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
        acmsg_shell = XtCreatePopupShell("xmbdfed_acmsgs_shell",
                                         xmDialogShellWidgetClass,
                                         top, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
        XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
        XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
        acmsg_form = XtCreateWidget("xmbdfed_acmsgs_form", xmFormWidgetClass,
                                    acmsg_shell, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNcolumns, 40); ac++;
        XtSetArg(av[ac], XmNrows, 10); ac++;
        XtSetArg(av[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
        XtSetArg(av[ac], XmNeditable, False); ac++;
        XtSetArg(av[ac], XmNcursorPositionVisible, False); ac++;
        acmsg_text = XmCreateScrolledText(acmsg_form, "xmbdfed_acmsgs_text",
                                          av, ac);
        XtManageChild(acmsg_text);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, XtParent(acmsg_text)); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_acmsgs_frame",
                                      xmFrameWidgetClass, acmsg_form, av, ac);

        pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, frame,
                                   0, 0);
        XtAddCallback(pb, XmNactivateCallback, DoClose,
                      (XtPointer) acmsg_shell);
    }

    /*
     * Adjust the title to match the editor.
     */
    if (ed->file == 0)
      sprintf(title, "%s - Corrections: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Corrections: %s", app_name, ed->file);

    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(acmsg_shell, av, 1);

    /*
     * Replace the text with any auto-correction messages the font might have.
     */
    if ((s = XmuttFontGridFontMessages(ed->fgrid)) == 0)
      s = "";
    XmTextSetString(acmsg_text, s);

    XtManageChild(acmsg_form);
    XtPopup(acmsg_shell, XtGrabNone);
}

static void
#ifndef _NO_PROTO
DoFontGridCopy(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontGridCopy(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridCopySelection(ed->fgrid);
}

static void
#ifndef _NO_PROTO
DoFontGridCut(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontGridCut(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridCutSelection(ed->fgrid);
}

static void
#ifndef _NO_PROTO
DoFontGridPaste(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontGridPaste(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridPasteSelection(ed->fgrid, XmuttFG_NORMAL_PASTE);
}

static void
#ifndef _NO_PROTO
DoFontGridOverlay(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontGridOverlay(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridPasteSelection(ed->fgrid, XmuttFG_OVERLAY_PASTE);
}

static void
#ifndef _NO_PROTO
DoFontGridInsert(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontGridInsert(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridPasteSelection(ed->fgrid, XmuttFG_INSERT_PASTE);
}

static void
#ifndef _NO_PROTO
TranslateGlyphs(Widget w, XtPointer client_data, XtPointer call_data)
#else
TranslateGlyphs(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttOperationCallbackStruct *cb;

    ed = &editors[(unsigned long) client_data];
    cb = (XmuttOperationCallbackStruct *) call_data;

    if (xmbdfed_opts.progbar)
      XmuttFontGridTranslateGlyphs(ed->fgrid, cb->dx, cb->dy,
                                   UpdateProgressBar, 0, cb->all);
    else {
        WatchCursor(ed->fgrid, True);
        XmuttFontGridTranslateGlyphs(ed->fgrid, cb->dx, cb->dy, 0, 0, cb->all);
        WatchCursor(ed->fgrid, False);
    }
}

static void
#ifndef _NO_PROTO
RotateGlyphs(Widget w, XtPointer client_data, XtPointer call_data)
#else
RotateGlyphs(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttOperationCallbackStruct *cb;

    ed = &editors[(unsigned long) client_data];
    cb = (XmuttOperationCallbackStruct *) call_data;

    if (xmbdfed_opts.progbar)
      XmuttFontGridRotateGlyphs(ed->fgrid, cb->angle, UpdateProgressBar,
                                0, cb->all);
    else {
        WatchCursor(ed->fgrid, True);
        XmuttFontGridRotateGlyphs(ed->fgrid, cb->angle, 0, 0, cb->all);
        WatchCursor(ed->fgrid, False);
    }
}

static void
#ifndef _NO_PROTO
ShearGlyphs(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShearGlyphs(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttOperationCallbackStruct *cb;

    ed = &editors[(unsigned long) client_data];
    cb = (XmuttOperationCallbackStruct *) call_data;

    if (xmbdfed_opts.progbar)
      XmuttFontGridShearGlyphs(ed->fgrid, cb->angle, UpdateProgressBar,
                               0, cb->all);
    else {
        WatchCursor(ed->fgrid, True);
        XmuttFontGridShearGlyphs(ed->fgrid, cb->angle, 0, 0, cb->all);
        WatchCursor(ed->fgrid, False);
    }
}

static void
#ifndef _NO_PROTO
EmboldenGlyphs(Widget w, XtPointer client_data, XtPointer call_data)
#else
EmboldenGlyphs(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttOperationCallbackStruct *cb;

    ed = &editors[(unsigned long) client_data];
    cb = (XmuttOperationCallbackStruct *) call_data;

    if (xmbdfed_opts.progbar)
      XmuttFontGridEmboldenGlyphs(ed->fgrid, UpdateProgressBar, 0, cb->all);
    else {
        WatchCursor(ed->fgrid, True);
        XmuttFontGridEmboldenGlyphs(ed->fgrid, 0, 0, cb->all);
        WatchCursor(ed->fgrid, False);
    }
}

static void
#ifndef _NO_PROTO
ShowTranslate(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowTranslate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ShowTranslateDialog(TranslateGlyphs, client_data, True);
}

static void
#ifndef _NO_PROTO
ShowRotate(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowRotate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ShowRotateDialog(RotateGlyphs, client_data, True);
}

static void
#ifndef _NO_PROTO
ShowShear(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowShear(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ShowShearDialog(ShearGlyphs, client_data, True);
}

static void
#ifndef _NO_PROTO
ShowEmbolden(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowEmbolden(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ShowEmboldenDialog(EmboldenGlyphs, client_data, True);
}

static void
#ifndef _NO_PROTO
MakeXLFDName(Widget w, XtPointer client_data, XtPointer call_data)
#else
MakeXLFDName(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridMakeXLFDName(ed->fgrid);
}

static void
#ifndef _NO_PROTO
DoUpdateNameFromProps(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoUpdateNameFromProps(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridUpdateNameFromProperties(ed->fgrid);
}

static void
#ifndef _NO_PROTO
DoUpdatePropsFromName(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoUpdatePropsFromName(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XmuttFontGridUpdatePropertiesFromName(ed->fgrid);
}

static void
#ifndef _NO_PROTO
DoUpdateAverageWidth(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoUpdateAverageWidth(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridUpdateAverageWidth(ed->fgrid);
}

static void
#ifndef _NO_PROTO
DoUnicodeGlyphNames(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoUnicodeGlyphNames(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int changed;
    FILE *in;
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (xmbdfed_opts.unicode_name_file == 0)
      return;

    if ((in = fopen(xmbdfed_opts.unicode_name_file, "r")) == 0)
      return;

    if (xmbdfed_opts.progbar)
      changed = bdf_set_unicode_glyph_names(in, XmuttFontGridFont(ed->fgrid),
                                            UpdateProgressBar);
    else {
        WatchCursor(ed->fgrid, True);
        changed = bdf_set_unicode_glyph_names(in, XmuttFontGridFont(ed->fgrid),
                                              0);
        WatchCursor(ed->fgrid, False);
    }
    fclose(in);

    if (changed)
      XmuttFontGridSetModified(ed->fgrid, True);
}

static void
#ifndef _NO_PROTO
DoAdobeGlyphNames(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoAdobeGlyphNames(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int changed;
    FILE *in;
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (xmbdfed_opts.adobe_name_file == 0)
      return;

    if ((in = fopen(xmbdfed_opts.adobe_name_file, "r")) == 0)
      return;

    if (xmbdfed_opts.progbar)
      changed = bdf_set_adobe_glyph_names(in, XmuttFontGridFont(ed->fgrid),
                                          UpdateProgressBar);
    else {
        WatchCursor(ed->fgrid, True);
        changed = bdf_set_adobe_glyph_names(in, XmuttFontGridFont(ed->fgrid),
                                            0);
        WatchCursor(ed->fgrid, False);
    }
    fclose(in);

    if (changed)
      XmuttFontGridSetModified(ed->fgrid, True);
}


static void
#ifndef _NO_PROTO
DoUniGlyphNames(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoUniGlyphNames(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int changed;
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (xmbdfed_opts.progbar)
      changed = bdf_set_glyph_code_names('u', XmuttFontGridFont(ed->fgrid),
                                         UpdateProgressBar);
    else {
        WatchCursor(ed->fgrid, True);
        changed = bdf_set_glyph_code_names('u', XmuttFontGridFont(ed->fgrid),
                                           0);
        WatchCursor(ed->fgrid, False);
    }

    if (changed)
      XmuttFontGridSetModified(ed->fgrid, True);
}

static void
#ifndef _NO_PROTO
DoHexGlyphNames(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoHexGlyphNames(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int changed;
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (xmbdfed_opts.progbar)
      changed = bdf_set_glyph_code_names('x', XmuttFontGridFont(ed->fgrid),
                                         UpdateProgressBar);
    else {
        WatchCursor(ed->fgrid, True);
        changed = bdf_set_glyph_code_names('x', XmuttFontGridFont(ed->fgrid),
                                           0);
        WatchCursor(ed->fgrid, False);
    }

    if (changed)
      XmuttFontGridSetModified(ed->fgrid, True);
}

static void
#ifndef _NO_PROTO
DoCanonicalGlyphNames(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCanonicalGlyphNames(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int changed;
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (xmbdfed_opts.progbar)
      changed = bdf_set_glyph_code_names('+', XmuttFontGridFont(ed->fgrid),
                                         UpdateProgressBar);
    else {
        WatchCursor(ed->fgrid, True);
        changed = bdf_set_glyph_code_names('+', XmuttFontGridFont(ed->fgrid),
                                           0);
        WatchCursor(ed->fgrid, False);
    }

    if (changed)
      XmuttFontGridSetModified(ed->fgrid, True);
}

static void
#ifndef _NO_PROTO
DoJavaGlyphNames(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoJavaGlyphNames(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int changed;
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (xmbdfed_opts.progbar)
      changed = bdf_set_glyph_code_names('\\', XmuttFontGridFont(ed->fgrid),
                                         UpdateProgressBar);
    else {
        WatchCursor(ed->fgrid, True);
        changed = bdf_set_glyph_code_names('\\', XmuttFontGridFont(ed->fgrid),
                                           0);
        WatchCursor(ed->fgrid, False);
    }

    if (changed)
      XmuttFontGridSetModified(ed->fgrid, True);
}

static void
#ifndef _NO_PROTO
DoFontNameChange(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontNameChange(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    char *s;

    ed = &editors[(unsigned long) client_data];

    if (ed->update_fontname == True) {
        /*
         * Tell the font modified routine to avoid replacing the font name
         * because it is changing due to user editing.
         */
        ed->update_fontname = False;

        s = XmTextFieldGetString(ed->fontname);
        XmuttFontGridSetFontName(ed->fgrid, s);
        free(s);

        ed->update_fontname = True;
    }
}

static void
#ifndef _NO_PROTO
DoSetMainEditMenu(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoSetMainEditMenu(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];
    if (XmuttFontGridClipboardEmpty(ed->fgrid) == True) {
        XtSetSensitive(ed->editmenu.paste, False);
        XtSetSensitive(ed->editmenu.overlay, False);
        XtSetSensitive(ed->editmenu.insert, False);
    } else {
        XtSetSensitive(ed->editmenu.paste, True);
        XtSetSensitive(ed->editmenu.overlay, True);
        XtSetSensitive(ed->editmenu.insert, True);
    }

    if (XmuttFontGridHasSelection(ed->fgrid) == True) {
        XtSetSensitive(ed->editmenu.copy, True);
        XtSetSensitive(ed->editmenu.cut, True);
    } else {
        XtSetSensitive(ed->editmenu.copy, False);
        XtSetSensitive(ed->editmenu.cut, False);
    }

    XmuttFontGridPageInfo(ed->fgrid, &pi);
    if (pi.unencoded == True) {
        if (pi.num_unencoded > 0) {
            XtSetSensitive(ed->editmenu.test, True);
            XtSetSensitive(ed->editmenu.gname, True);
        } else {
            XtSetSensitive(ed->editmenu.test, False);
            XtSetSensitive(ed->editmenu.gname, False);
        }
    } else {
        if (pi.num_encoded > 0) {
            XtSetSensitive(ed->editmenu.test, True);
            XtSetSensitive(ed->editmenu.gname, True);
            if (xmbdfed_opts.unicode_name_file != 0)
              XtSetSensitive(ed->editmenu.unames, True);
            else
              XtSetSensitive(ed->editmenu.unames, False);
            if (xmbdfed_opts.adobe_name_file != 0)
              XtSetSensitive(ed->editmenu.anames, True);
            else
              XtSetSensitive(ed->editmenu.anames, False);
        } else {
            XtSetSensitive(ed->editmenu.test, False);
            XtSetSensitive(ed->editmenu.gname, False);
        }
    }

    if (XmuttFontGridHasXLFDName(ed->fgrid) == True) {
        XtSetSensitive(ed->editmenu.xlfdname, False);
        XtSetSensitive(ed->editmenu.unfp, True);
        XtSetSensitive(ed->editmenu.upfn, True);
        XtSetSensitive(ed->editmenu.awidth, True);
    } else {
        XtSetSensitive(ed->editmenu.xlfdname, True);
        XtSetSensitive(ed->editmenu.unfp, False);
        XtSetSensitive(ed->editmenu.upfn, False);
        XtSetSensitive(ed->editmenu.awidth, False);
    }
}

static void
#ifndef _NO_PROTO
DoResetMainEditMenu(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoResetMainEditMenu(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XtSetSensitive(ed->editmenu.insert, True);
    XtSetSensitive(ed->editmenu.overlay, True);
    XtSetSensitive(ed->editmenu.paste, True);
    XtSetSensitive(ed->editmenu.copy, True);
    XtSetSensitive(ed->editmenu.cut, True);
    XtSetSensitive(ed->editmenu.xlfdname, True);
}

static void
#ifndef _NO_PROTO
EnableExport(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnableExport(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridPageInfo(ed->fgrid, &pi);

    /*
     * Enable or disable the export menu item depending on whether the
     * font has any glyphs.
     */
    if (pi.num_encoded > 0)
      XtSetSensitive(ed->filemenu.export, True);
    else
      XtSetSensitive(ed->filemenu.export, False);
}

static void
#ifndef _NO_PROTO
DoBuildEditorMenu(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoBuildEditorMenu(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long id, i;
    int nkids;
    MXFEditor *ed;
    Widget pb, *kids;
    XmString s;
    Arg av[2];

    id = (unsigned long) client_data;
    ed = &editors[id];

    XtSetArg(av[0], XmNnumChildren, &nkids);
    XtSetArg(av[1], XmNchildren, &kids);
    XtGetValues(ed->elist, av, 2);

    nkids -= 2;

    /*
     * Update the existing menu entries for windows.
     */
    for (i = 0; i < nkids; i++) {
        if (editors[i].file == 0)
          sprintf(title, "(unnamed%ld)", i);
        else
          sprintf(title, "%s", editors[i].file);
        s = XmStringCreateSimple(title);
        XtSetArg(av[0], XmNlabelString, s);
        XtSetValues(kids[i + 2], av, 1);
        XmStringFree(s);
    }

    /*
     * Add any remaining editors that were created since the menu was last
     * shown.
     */
    for (; i < num_editors; i++) {
        if (editors[i].file == 0) {
            sprintf(name, "(unnamed%ld)", i);
            pb = XtCreateManagedWidget(name, xmPushButtonWidgetClass,
                                       ed->elist, 0, 0);
        } else
            pb = XtCreateManagedWidget(editors[i].file,
                                       xmPushButtonWidgetClass,
                                       ed->elist, 0, 0);
        XtAddCallback(pb, XmNactivateCallback, ShowEditor,
                      (XtPointer) editors[i].id);
    }
}

static void
#ifndef _NO_PROTO
DoNewEditor(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoNewEditor(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long id;

    id = MakeEditor(top, 0, False);
    ShowEditor(w, (XtPointer) id, 0);
}

static void
#ifndef _NO_PROTO
MakeMenuBar(Widget mbar, MXFEditor *ed)
#else
MakeMenuBar(mbar, ed)
Widget mbar;
MXFEditor *ed;
#endif
{
    Widget pdown, cpdown, casc, pb, sep;
    XmString s, s1;
    Cardinal ac;
    Arg av[6];

    sprintf(name, "xmbdfed_editor_%ld_file_menu", ed->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);
    XtAddCallback(XtParent(pdown), XmNpopupCallback, EnableExport,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'F'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("File", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    s = XmStringCreateSimple("Ctrl+N");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'N'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>n"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("New", xmPushButtonWidgetClass, pdown, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoNewEditor, 0);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+O");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'O'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>o"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Open", xmPushButtonWidgetClass, pdown, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoOpen, (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+S");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'S'); ac++;
    XtSetArg(av[ac], XmNsensitive, False); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>s"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->filemenu.save = XtCreateManagedWidget("Save", xmPushButtonWidgetClass,
                                              pdown, av, ac);
    XtAddCallback(ed->filemenu.save, XmNactivateCallback, DoSave,
                  (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+W");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'A'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>w"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->filemenu.saveas = XtCreateManagedWidget("Save As",
                                                xmPushButtonWidgetClass,
                                                pdown, av, ac);
    XtAddCallback(ed->filemenu.saveas, XmNactivateCallback, DoSaveAs,
                  (XtPointer) ed->id);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_filesep1", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    sprintf(name, "xmbdfed_editor_%ld_import_menu", ed->id);
    cpdown = XmCreatePulldownMenu(pdown, name, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'I'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, cpdown); ac++;
    pb = XtCreateManagedWidget("Import", xmCascadeButtonWidgetClass, pdown,
                               av, ac);

    s = XmStringCreateSimple("Ctrl+K");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'P'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>k"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("PK/GF Font", xmPushButtonWidgetClass,
                               cpdown, av, ac);
    XtAddCallback(pb, XmNactivateCallback, ImportPkGf, (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+L");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'C'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>l"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Console Font", xmPushButtonWidgetClass,
                               cpdown, av, ac);
    XtAddCallback(pb, XmNactivateCallback, ImportConsole, (XtPointer) ed->id);
    XmStringFree(s);

#ifdef HAVE_HBF
    s = XmStringCreateSimple("Ctrl+H");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'H'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>h"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("HBF Font", xmPushButtonWidgetClass, cpdown,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, ImportHBF, (XtPointer) ed->id);
    XmStringFree(s);
#endif /* HAVE_HBF */

    s = XmStringCreateSimple("Ctrl+B");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'W'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>b"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Windows Font", xmPushButtonWidgetClass, cpdown,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, ImportFNT, (XtPointer) ed->id);
    XmStringFree(s);

#ifdef HAVE_FREETYPE

    s = XmStringCreateSimple("Ctrl+Y");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'T'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>y"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("TrueType Font", xmPushButtonWidgetClass,
                               cpdown, av, ac);
    XtAddCallback(pb, XmNactivateCallback, ImportTTF, (XtPointer) ed->id);
    XmStringFree(s);

#endif /* HAVE_FREETYPE */

    sprintf(name, "xmbdfed_editor_%ld_importsep", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, cpdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+G");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'S'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>g"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Server Font", xmPushButtonWidgetClass, cpdown,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoGrabServerFont,
                  (XtPointer) ed->id);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_export_menu", ed->id);
    cpdown = XmCreatePulldownMenu(pdown, name, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'x'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, cpdown); ac++;
    ed->filemenu.export = XtCreateManagedWidget("Export",
                                                xmCascadeButtonWidgetClass,
                                                pdown, av, ac);

    s = XmStringCreateSimple("Ctrl+F");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'P'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>f"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("PSF", xmPushButtonWidgetClass, cpdown,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoExportPSF, (XtPointer) ed->id);
    XmStringFree(s);

    XtSetArg(av[0], XmNmnemonic, 'G');
    pb = XtCreateManagedWidget("HEX", xmPushButtonWidgetClass, cpdown,
                               av, 1);
    XtAddCallback(pb, XmNactivateCallback, DoExportHEX, (XtPointer) ed->id);

    sprintf(name, "xmbdfed_editor_%ld_filesep2", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    ac = 0;
    if (xmbdfed_opts.accelerator_text != 0)
      s = XmStringCreateSimple(xmbdfed_opts.accelerator_text);
    else
      s = XmStringCreateSimple("Ctrl+F4");
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    if (xmbdfed_opts.accelerator != 0) {
        XtSetArg(av[ac], XmNaccelerator, xmbdfed_opts.accelerator); ac++;
    } else {
        XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>F4"); ac++;
    }

    if (ed->id == 0) {
        XtSetArg(av[ac], XmNmnemonic, 'E'); ac++;
        pb = XtCreateManagedWidget("Exit", xmPushButtonWidgetClass, pdown,
                                   av, ac);
        XtAddCallback(pb, XmNactivateCallback, DoQuit, 0);
    } else {
        XtSetArg(av[ac], XmNmnemonic, 'C');
        pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, pdown,
                                   av, ac);
        XtAddCallback(pb, XmNactivateCallback, DoCloseEditor,
                      (XtPointer) ed->id);
    }
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_edit_menu", ed->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);
    XtAddCallback(pdown, XmNmapCallback, DoSetMainEditMenu,
                  (XtPointer) ed->id);
    XtAddCallback(pdown, XmNunmapCallback, DoResetMainEditMenu,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'E'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("Edit", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    s = XmStringCreateSimple("Ctrl+C");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'C'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>c"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->editmenu.copy = XtCreateManagedWidget("Copy", xmPushButtonWidgetClass,
                                              pdown, av, ac);
    XtAddCallback(ed->editmenu.copy, XmNactivateCallback, DoFontGridCopy,
                  (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+X");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'u'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>x"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->editmenu.cut = XtCreateManagedWidget("Cut", xmPushButtonWidgetClass,
                                             pdown, av, ac);
    XtAddCallback(ed->editmenu.cut, XmNactivateCallback, DoFontGridCut,
                  (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+V");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'P'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>v"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->editmenu.paste = XtCreateManagedWidget("Paste",
                                               xmPushButtonWidgetClass, pdown,
                                               av, ac);
    XtAddCallback(ed->editmenu.paste, XmNactivateCallback, DoFontGridPaste,
                  (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+Shift+V");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'O'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl Shift<Key>V"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->editmenu.overlay = XtCreateManagedWidget("Overlay",
                                                 xmPushButtonWidgetClass,
                                                 pdown, av, ac);
    XtAddCallback(ed->editmenu.overlay, XmNactivateCallback, DoFontGridOverlay,
                  (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+Meta+V");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'I'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl Meta<Key>v"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->editmenu.insert = XtCreateManagedWidget("Insert",
                                                xmPushButtonWidgetClass,
                                                pdown, av, ac);
    XtAddCallback(ed->editmenu.insert, XmNactivateCallback, DoFontGridInsert,
                  (XtPointer) ed->id);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_editsep1", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+P");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'r'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>p"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Properties", xmPushButtonWidgetClass, pdown,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoFontProperties,
                  (XtPointer) ed->id);
    XmStringFree(s);

    s = XmStringCreateSimple("Ctrl+M");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'm'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>m"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Comments", xmPushButtonWidgetClass, pdown,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoEditComments,
                  (XtPointer) ed->id);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_editsep2", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+I");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'F'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>i"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->editmenu.finfo = XtCreateManagedWidget("Font Info",
                                               xmPushButtonWidgetClass, pdown,
                                               av, ac);
    XtAddCallback(ed->editmenu.finfo, XmNactivateCallback,
                  DoFontInfo, (XtPointer) ed->id);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_font_name_menu", ed->id);
    XtSetArg(av[0], XmNradioBehavior, True);
    cpdown = XmCreatePulldownMenu(pdown, name, av, 1);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'N'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, cpdown); ac++;
    casc = XtCreateManagedWidget("Font Name", xmCascadeButtonWidgetClass,
                                 pdown, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'X'); ac++;
    ed->editmenu.xlfdname = XtCreateManagedWidget("Make XLFD Name",
                                                  xmPushButtonWidgetClass,
                                                  cpdown, av, ac);
    XtAddCallback(ed->editmenu.xlfdname, XmNactivateCallback, MakeXLFDName,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'N'); ac++;
    ed->editmenu.unfp = XtCreateManagedWidget("Update Name From Properties",
                                              xmPushButtonWidgetClass, cpdown,
                                              av, ac);
    XtAddCallback(ed->editmenu.unfp, XmNactivateCallback,
                  DoUpdateNameFromProps, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'P'); ac++;
    ed->editmenu.upfn = XtCreateManagedWidget("Update Properties From Name",
                                              xmPushButtonWidgetClass, cpdown,
                                              av, ac);
    XtAddCallback(ed->editmenu.upfn, XmNactivateCallback,
                  DoUpdatePropsFromName, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'A'); ac++;
    ed->editmenu.awidth = XtCreateManagedWidget("Update Average Width",
                                                xmPushButtonWidgetClass,
                                                cpdown, av, ac);
    XtAddCallback(ed->editmenu.awidth, XmNactivateCallback,
                  DoUpdateAverageWidth, (XtPointer) ed->id);

    sprintf(name, "xmbdfed_editor_%ld_editsep3", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    sprintf(name, "xmbdfed_editor_%ld_gname_menu", ed->id);
    cpdown = XmCreatePulldownMenu(pdown, name, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'a'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, cpdown); ac++;
    ed->editmenu.gname = XtCreateManagedWidget("Name Glyphs",
                                               xmCascadeButtonWidgetClass,
                                               pdown, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'U'); ac++;
    ed->editmenu.unames = XtCreateManagedWidget("Unicode Names",
                                                xmPushButtonWidgetClass,
                                                cpdown, av, ac);
    XtAddCallback(ed->editmenu.unames, XmNactivateCallback,
                  DoUnicodeGlyphNames, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'A'); ac++;
    ed->editmenu.anames = XtCreateManagedWidget("Adobe Names",
                                                xmPushButtonWidgetClass,
                                                cpdown, av, ac);
    XtAddCallback(ed->editmenu.anames, XmNactivateCallback,
                  DoAdobeGlyphNames, (XtPointer) ed->id);

    sprintf(name, "xmbdfed_editor_%ld_gname_prefix_menu", ed->id);
    casc = XmCreatePulldownMenu(cpdown, name, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'V'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, casc); ac++;
    pb = XtCreateManagedWidget("Unicode Values", xmCascadeButtonWidgetClass,
                               cpdown, av, ac);

    XtSetArg(av[0], XmNmnemonic, 'u');
    pb = XtCreateManagedWidget("uniXXXX", xmPushButtonWidgetClass, casc,
                               av, 1);
    XtAddCallback(pb, XmNactivateCallback, DoUniGlyphNames,
                  (XtPointer) ed->id);

    sprintf(name, "xmbdfed_gname_%ld_sep", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, casc, 0, 0);

    XtSetArg(av[0], XmNmnemonic, 'x');
    pb = XtCreateManagedWidget("0xUUUU", xmPushButtonWidgetClass, casc, av, 1);
    XtAddCallback(pb, XmNactivateCallback, DoHexGlyphNames,
                  (XtPointer) ed->id);

    XtSetArg(av[0], XmNmnemonic, '+');
    pb = XtCreateManagedWidget("U+XXXX", xmPushButtonWidgetClass, casc, av, 1);
    XtAddCallback(pb, XmNactivateCallback, DoCanonicalGlyphNames,
                  (XtPointer) ed->id);

    XtSetArg(av[0], XmNmnemonic, '\\');
    pb = XtCreateManagedWidget("\\uXXXX", xmPushButtonWidgetClass, casc,
                               av, 1);
    XtAddCallback(pb, XmNactivateCallback, DoJavaGlyphNames,
                  (XtPointer) ed->id);

    s = XmStringCreateSimple("Ctrl+Z");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'T'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>z"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->editmenu.test = XtCreateManagedWidget("Test Glyphs",
                                              xmPushButtonWidgetClass, pdown,
                                              av, ac);
    XmStringFree(s);
    XtAddCallback(ed->editmenu.test, XmNactivateCallback, DoGlyphTest,
                  (XtPointer) ed->id);

    sprintf(name, "xmbdfed_editor_%ld_editsep4", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+T");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'S'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>t"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Setup", xmPushButtonWidgetClass, pdown,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoSetupDialog, (XtPointer) ed->id);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_view_menu", ed->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'V'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("View", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    s = XmStringCreateSimple("Ctrl+E");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'd'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>e"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->viewmenu.unencoded = XtCreateManagedWidget("Unencoded",
                                                   xmPushButtonWidgetClass,
                                                   pdown, av, ac);
    XtAddCallback(ed->viewmenu.unencoded, XmNactivateCallback,
                  ToggleEncodedView, (XtPointer) ed->id);
    XmStringFree(s);
                                 
    sprintf(name, "xmbdfed_editor_%ld_view_code_menu", ed->id);
    XtSetArg(av[0], XmNradioBehavior, True);
    cpdown = XmCreatePulldownMenu(pdown, name, av, 1);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'C'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, cpdown); ac++;
    casc = XtCreateManagedWidget("Code Base", xmCascadeButtonWidgetClass,
                                 pdown, av, ac);

    ed->viewmenu.oct = XtCreateManagedWidget("Octal",
                                             xmToggleButtonWidgetClass,
                                             cpdown, 0, 0);
    XtAddCallback(ed->viewmenu.oct, XmNvalueChangedCallback, DisplayOctal,
                  (XtPointer) ed->id);

    ed->viewmenu.dec = XtCreateManagedWidget("Decimal",
                                             xmToggleButtonWidgetClass,
                                             cpdown, 0, 0);
    XtAddCallback(ed->viewmenu.dec, XmNvalueChangedCallback, DisplayDecimal,
                  (XtPointer) ed->id);

    ed->viewmenu.hex = XtCreateManagedWidget("Hexadecimal",
                                             xmToggleButtonWidgetClass, cpdown,
                                             0, 0);
    XtAddCallback(ed->viewmenu.hex, XmNvalueChangedCallback, DisplayHex,
                  (XtPointer) ed->id);

    sprintf(name, "xmbdfed_editor_%ld_viewsep1", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+Shift+S");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'O'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl Shift<Key>s"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->viewmenu.other =
        XtCreateManagedWidget("Other Page", xmPushButtonWidgetClass,
                              pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ed->viewmenu.other, XmNactivateCallback, GotoOtherPage,
                  (XtPointer) ed->id);

    s1 = XmStringCreateSimple("Ctrl+Q");
    s = (xmbdfed_opts.vertical == 0) ?
        XmStringCreateSimple("Vertical View") :
        XmStringCreateSimple("Horizontal View");

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'w'); ac++;
    XtSetArg(av[ac], XmNlabelString, s); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>q"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s1); ac++;
    sprintf(name, "xmbdfed_editor_%ld_view_button", ed->id);
    ed->viewmenu.vert = XtCreateManagedWidget(name, xmPushButtonWidgetClass,
                                              pdown, av, ac);
    XmStringFree(s);
    XmStringFree(s1);
    XtAddCallback(ed->viewmenu.vert, XmNactivateCallback, DoFontGridView,
                  (XtPointer) ed->id);

    sprintf(name, "xmbdfed_editor_%ld_viewsep2", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+A");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'M'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>a"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ed->viewmenu.acmsgs = XtCreateManagedWidget("Messages",
                                                xmPushButtonWidgetClass,
                                                pdown, av, ac);
    XtAddCallback(ed->viewmenu.acmsgs, XmNactivateCallback,
                  ShowMessages, (XtPointer) ed->id);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_ops_menu", ed->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'O'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("Operations", xmCascadeButtonWidgetClass,
                                 mbar, av, ac);

    s = XmStringCreateSimple("Ctrl+D");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'T'); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>d"); ac++;
    pb = XtCreateManagedWidget("Translate Glyphs", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, ShowTranslate,
                  (XtPointer) ed->id);

    s = XmStringCreateSimple("Ctrl+R");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'R'); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>r"); ac++;
    pb = XtCreateManagedWidget("Rotate Glyphs", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, ShowRotate, (XtPointer) ed->id);

    s = XmStringCreateSimple("Ctrl+J");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'h'); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>j"); ac++;
    pb = XtCreateManagedWidget("Shear Glyphs", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, ShowShear, (XtPointer) ed->id);

    s = XmStringCreateSimple("Ctrl+Shift+B");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'E'); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl Shift<Key>b"); ac++;
    pb = XtCreateManagedWidget("Embolden Glyphs", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, ShowEmbolden, (XtPointer) ed->id);

    sprintf(name, "xmbdfed_editor_%ld_editor_menu", ed->id);
    ed->elist = pdown = XmCreatePulldownMenu(mbar, name, 0, 0);
    XtAddCallback(XtParent(pdown), XmNpopupCallback, DoBuildEditorMenu,
                  (XtPointer) ed->id);
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'd'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("Editors", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    s = XmStringCreateSimple("Ctrl+N");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'N'); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("New", xmPushButtonWidgetClass, pdown, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoNewEditor, 0);
    XmStringFree(s);

    sprintf(name, "xmbdfed_editor_%ld_editorsep1", ed->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    sprintf(name, "xmbdfed_editor_%ld_help_menu", ed->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'H'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("Help", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    /*
     * Tell the menu bar that this is the Help menu.
     */
    ac = 0;
    XtSetArg(av[ac], XmNmenuHelpWidget, casc); ac++;
    XtSetValues(mbar, av, ac);

    /*
     * Add the help menu to the pulldown.
     */
    AddHelpMenu(pdown);
}

static void
#ifndef _NO_PROTO
RowColResize(Widget w, XtPointer client_data, XEvent *event, Boolean *cdis)
#else
RowColResize(w, client_data, event, cdis)
Widget w;
XtPointer client_data;
XEvent *event;
Boolean *cdis;
#endif
{
    XmRowColumnWidget rc;
    MXFEditor *ed;
    Position y;
    Dimension ht, margin;
    Arg av[2];

    ed = &editors[(unsigned long) client_data];

    *cdis = True;

    if (event->xconfigure.width == 0 || event->xconfigure.height == 0 ||
        ed->fgrid == 0)
      return;

    XtSetArg(av[0], XmNy, &y);
    XtSetArg(av[1], XmNheight, &ht);
    XtGetValues(ed->fgrid, av, 2);

    rc = (XmRowColumnWidget) w;

    margin = rc->row_column.margin_height + rc->row_column.spacing;

    if (y + ht < rc->core.height - margin) {
        ht += (rc->core.height - margin) - (y + ht);
        XtSetArg(av[0], XmNheight, ht);
        XtSetValues(ed->fgrid, av, 1);
    }
}

unsigned long
#ifndef _NO_PROTO
MakeEditor(Widget parent, char *filename, Boolean cmdline)
#else
MakeEditor(parent, filename, cmdline)
Widget parent;
char *filename;
Boolean cmdline;
#endif
{
    Widget mw, mb, rc, rc1, frame, form, label1, label2;
    FILE *in;
    char *path, *file, *cp;
    unsigned long which;
    XmString s;
    bdf_font_t *font;
    bdf_callback_t callback;
    MXFEditor *ed;
    Cardinal ac;
    Dimension wd, ht;
    Boolean free_filename;
    XmuttFontGridPageInfoStruct pi;
    XmuttFontGridSelectionStruct sinfo;
    Arg av[12];

    which = num_editors;
    font = 0;
    path = file = 0;
    free_filename = False;

    if (xmbdfed_opts.progbar)
      callback = (cmdline == False) ? UpdateProgressBar : 0;
    else
      callback = 0;

    /*
     * If a file has been passed, load it before creating the
     * editor.
     */
    if (filename != 0) {
        if ((in = fopen(filename, "r")) == 0) {
            fprintf(stderr, "%s: unable to open BDF font '%s'.\n",
                    app_name, filename);
            if (which > 0)
              return ~0L;
        } else {
            font = bdf_load_font(in, &xmbdfed_opts.font_opts,
                                 callback, 0);
            fclose(in);
            if (font == 0)
              fprintf(stderr, "%s: problem loading BDF font '%s'.\n",
                      app_name, filename);
        }

        file = path = 0;

        if (font != 0) {
            /*
             * Construct the path and filename from the one passed.
             */
            file = strrchr(filename, '/');
            if (file == 0) {
                path = 0;
                file = (char *) XtMalloc(strlen(filename) + 1);
                (void) strcpy(file, filename);
            } else {
                path = (char *) XtMalloc((file - filename) + 1);
                (void) strncpy(path, filename, (file - filename));
                path[file - filename] = 0;
                cp = ++file;
                file = (char *) XtMalloc(strlen(file) + 1);
                (void) strcpy(file, cp);
            }
        }

        /*
         * Free up the filename if indicated.
         */
        if (free_filename == True)
          XtFree(filename);
    }

    if (font == 0)
      sprintf(title, "%s - (unnamed%ld)", app_name, which);
    else
      sprintf(title, "%s - %s", app_name, file);

    if (which == 0)
      editors = (MXFEditor *) XtMalloc(sizeof(MXFEditor));
    else
      editors = (MXFEditor *) XtRealloc((char *) editors,
                                        sizeof(MXFEditor) * (num_editors + 1));

    ed = &editors[num_editors++];
    (void) memset((char *) ed, 0, sizeof(MXFEditor));

    ed->id = which;
    ed->path = path;
    ed->file = file;
    ed->update_fontname = True;
    ed->last_upgno = ed->last_pgno = -1;

    sprintf(name, "xmbdfed_editor_%ld", which);
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    if (which == 0) {
        ed->shell = parent;
        XtSetValues(parent, av, 1);
    } else {
        XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
        ed->shell = XtCreatePopupShell(name, topLevelShellWidgetClass, parent,
                                       av, ac);
    }

    sprintf(name, "xmbdfed_editor_%ld_mainw", which);
    ed->mainw = mw = XtCreateManagedWidget(name, xmMainWindowWidgetClass,
                                           ed->shell, 0, 0);

    sprintf(name, "xmbdfed_editor_%ld_menubar", which);
    mb = XmCreateMenuBar(mw, name, 0, 0);
    MakeMenuBar(mb, ed);
    XtManageChild(mb);

    /*
     * The primary RowColumn widget.
     */
    ac = 0;
    XtSetArg(av[ac], XmNspacing, 4); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    sprintf(name, "xmbdfed_editor_%ld_rowcol", which);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, mw, av, ac);

    /*
     * The RowColumn does not resize the font grid correctly, so this event
     * handler will make sure the font grid gets resized vertically to fit in
     * the RowColumn after the RowColumn has finished its resize.
     */
    XtInsertEventHandler(rc, StructureNotifyMask, False, RowColResize,
                         (XtPointer) ed->id, XtListTail);

    /*
     * The Font and Glyph info widgets.
     */
    ac = 0;
    XtSetArg(av[ac], XmNshadowType, XmSHADOW_OUT); ac++;
    sprintf(name, "xmbdfed_editor_%ld_info_frame", which);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, rc, av, ac);

    sprintf(name, "xmbdfed_editor_%ld_info_form", which);
    form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, 0, 0);

    XtSetArg(av[0], XmNalignment, XmALIGNMENT_END);
    label1 = XtCreateManagedWidget("Font:", xmLabelWidgetClass, form, av, 1);

    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 65); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label1); ac++;
    sprintf(name, "xmbdfed_editor_%ld_fontname", which);
    ed->fontname = XtCreateManagedWidget(name, xmTextFieldWidgetClass, form,
                                         av, ac);

    XtSetArg(av[0], XmNheight, &ht);
    XtGetValues(ed->fontname, av, 1);
    XtSetArg(av[0], XmNheight, ht);
    XtSetValues(label1, av, 1);

    ac = 0;
    XtSetArg(av[0], XmNalignment, XmALIGNMENT_END);
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, label1); ac++;
    label2 = XtCreateManagedWidget("Glyph:", xmLabelWidgetClass, form, av, ac);

    /*
     * Adjust the width of the "Font:" widget to match the "Glyph:" widget.
     */
    XtSetArg(av[0], XmNwidth, &wd);
    XtGetValues(label2, av, 1);
    XtSetArg(av[0], XmNwidth, wd);
    XtSetValues(label1, av, 1);

    sprintf(name, "xmbdfed_editor_%ld_glyphinfo", which);
    s = XmStringCreateLtoR("None\nNone", XmSTRING_DEFAULT_CHARSET);

    ac = 0;
    XtSetArg(av[ac], XmNlabelString, s); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label2); ac++;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, ed->fontname); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    ed->glyphinfo = XtCreateManagedWidget(name, xmLabelWidgetClass, form,
                                          av, ac);
    XmStringFree(s);

    /*
     * The navigation buttons.
     */
    sprintf(name, "xmbdfed_editor_%ld_button_frame", which);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, rc, 0, 0);

    sprintf(name, "xmbdfed_editor_%ld_button_rowcol", which);
    XtSetArg(av[0], XmNorientation, XmHORIZONTAL);
    rc1 = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, 1);

    ed->firstpage = XtCreateManagedWidget("First Page",
                                          xmPushButtonWidgetClass, rc1, 0, 0);
    XtAddCallback(ed->firstpage, XmNactivateCallback, GotoFirstPage,
                  (XtPointer) ed->id);

    ed->prevpage = XtCreateManagedWidget("Previous Page",
                                         xmPushButtonWidgetClass, rc1, 0, 0);
    XtAddCallback(ed->prevpage, XmNactivateCallback, GotoPrevPage,
                  (XtPointer) ed->id);

    ed->nextpage = XtCreateManagedWidget("Next Page",
                                         xmPushButtonWidgetClass, rc1, 0, 0);
    XtAddCallback(ed->nextpage, XmNactivateCallback, GotoNextPage,
                  (XtPointer) ed->id);

    ed->lastpage = XtCreateManagedWidget("Last Page",
                                         xmPushButtonWidgetClass, rc1, 0, 0);
    XtAddCallback(ed->lastpage, XmNactivateCallback, GotoLastPage,
                  (XtPointer) ed->id);

    label1 = XtCreateManagedWidget("Page:", xmLabelWidgetClass, rc1, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_editor_%ld_pageno", which);
    ed->pageno = XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc1,
                                       av, ac);
    XtAddCallback(ed->pageno, XmNactivateCallback, GotoPage,
                  (XtPointer) ed->id);

    label1 = XtCreateManagedWidget("Code:", xmLabelWidgetClass, rc1, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_editor_%ld_codeno", which);
    ed->code = XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc1,
                                     av, ac);
    XtAddCallback(ed->code, XmNactivateCallback, GotoCode,
                  (XtPointer) ed->id);

    /*
     * Allocate the colors for 2 bits per pixel if they are needed.
     */
    if (((font && font->bpp == 2) ||
         xmbdfed_opts.font_opts.bits_per_pixel == 2) &&
        xmbdfed_opts.pixels[0] == ~0)
      xmbdfedAllocateColors(parent, 2);

    /*
     * Allocate the colors for 4 bits per pixel if they are needed.
     */
    if (((font && font->bpp == 4) ||
         xmbdfed_opts.font_opts.bits_per_pixel == 4) &&
        xmbdfed_opts.pixels[4] == ~0)
      xmbdfedAllocateColors(parent, 4);

    /*
     * Now create the font edit grid.
     */
    ac = 0;
    if (font != 0) {
        XtSetArg(av[ac], XmNbdfFont, font); ac++;
    }
    XtSetArg(av[ac], XmNpointSize,
             xmbdfed_opts.font_opts.point_size); ac++;
    XtSetArg(av[ac], XmNhorizontalResolution,
             xmbdfed_opts.font_opts.resolution_x); ac++;
    XtSetArg(av[ac], XmNverticalResolution,
             xmbdfed_opts.font_opts.resolution_y); ac++;
    XtSetArg(av[ac], XmNfontSpacing,
             xmbdfed_opts.font_opts.font_spacing); ac++;
    XtSetArg(av[ac], XmNskipBlankPages, xmbdfed_opts.no_blanks); ac++;
    XtSetArg(av[ac], XmNgridOverwriteMode, xmbdfed_opts.overwrite_mode); ac++;
    XtSetArg(av[ac], XmNpowersOfTwo, xmbdfed_opts.power2); ac++;
    XtSetArg(av[ac], XmNbitsPerPixel,
             xmbdfed_opts.font_opts.bits_per_pixel); ac++;
    XtSetArg(av[ac], XmNcolorList, xmbdfed_opts.pixels); ac++;
    XtSetArg(av[ac], XmNinitialGlyph, xmbdfed_opts.initial_glyph); ac++;
    XtSetArg(av[ac], XmNcodeBase, xmbdfed_opts.code_base); ac++;

    sprintf(name, "xmbdfed_editor_%ld_fonteditgrid", which);
    ed->fgrid = XtCreateManagedWidget(name, xmuttFontGridWidgetClass, rc,
                                      av, ac);
    XtAddCallback(ed->fgrid, XmNpageCallback, PageChangeCallback,
                  (XtPointer) ed->id);
    XtAddCallback(ed->fgrid, XmNcodePageCallback, CodePageChangeCallback,
                  (XtPointer) ed->id);
    XtAddCallback(ed->fgrid, XmNstartSelectionCallback, UpdateGlyphInfo,
                  (XtPointer) ed->id);
    XtAddCallback(ed->fgrid, XmNextendSelectionCallback, UpdateGlyphInfo,
                  (XtPointer) ed->id);
    XtAddCallback(ed->fgrid, XmNendSelectionCallback, UpdateGlyphInfo,
                  (XtPointer) ed->id);
    XtAddCallback(ed->fgrid, XmNactivateCallback, DoGlyphEdit,
                  (XtPointer) ed->id);
    XtAddCallback(ed->fgrid, XmNmodifiedCallback, UpdateModified,
                  (XtPointer) ed->id);

    /*
     * Force the information on the initial glyph to be displayed.
     */
    XmuttFontGridSelectionInfo(ed->fgrid, &sinfo);
    UpdateGlyphInfo(ed->fgrid, (XtPointer) ed->id, (XtPointer) &sinfo);

    /*
     * Set the font name text field and add the name change callback only
     * after the font name has been set for the first time.
     */
    XmTextFieldSetString(ed->fontname, XmuttFontGridFontName(ed->fgrid));

    XtAddCallback(ed->fontname, XmNvalueChangedCallback, DoFontNameChange,
                  (XtPointer) ed->id);

    /*
     * Enable or disable the navigation buttons based on the page info
     * of the font.
     */
    XmuttFontGridPageInfo(ed->fgrid, &pi);

    if (pi.previous_page < 0) {
        XtSetSensitive(ed->prevpage, False);
        XtSetSensitive(ed->firstpage, False);
    }
    if (pi.next_page < 0) {
        XtSetSensitive(ed->nextpage, False);
        XtSetSensitive(ed->lastpage, False);
    }

    /*
     * Set the current page number in the text field.
     */
    sprintf(name, "%ld", pi.current_page);
    XmTextFieldSetString(ed->pageno, name);
    XmTextFieldSetCursorPosition(ed->pageno, strlen(name));

    if (XmuttFontGridFontMessages(ed->fgrid) == 0)
      XtSetSensitive(ed->viewmenu.acmsgs, False);
    else
      XtSetSensitive(ed->viewmenu.acmsgs, True);

    /*
     * Make sure the correct code base is set on the code base menu.
     */
    switch (XmuttFontGridCodeBase(ed->fgrid)) {
      case 8: XmToggleButtonSetState(ed->viewmenu.oct, True, False); break;
      case 10: XmToggleButtonSetState(ed->viewmenu.dec, True, False); break;
      case 16: XmToggleButtonSetState(ed->viewmenu.hex, True, False); break;
    }

    /*
     * Enable the Save and Save As menu options if the font has been
     * modified.  Also update the title if necessary.
     */
    if (XmuttFontGridModified(ed->fgrid) == True) {
        sprintf(title, "%s - %s [modified]", app_name, file);
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(ed->shell, av, 1);
        XtSetSensitive(ed->filemenu.save, True);
    }

    return which;
}

static Boolean
#ifndef _NO_PROTO
_CvtStringToFontSpacing(Display *d, XrmValuePtr args, Cardinal *num_args,
                        XrmValuePtr from, XrmValuePtr to, XtPointer *conv_data)
#else
_CvtStringToFontSpacing(d, args, num_args, from, to, conv_data)
Display *d;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from, to;
XtPointer *conv_data;
#endif
{
    static int fspacing;
    Boolean res;
    char *s;

    res = True;
    if (*num_args != 0)
      XtAppWarningMsg(XtDisplayToApplicationContext(d),
                      "CvtStringToFontSpacing", "wrongParameters",
                      "XmBDFEditorWarning",
                      "String to font spacing needs no extra arguments",
                      0, 0);

    s = (char *) from->addr;
    switch (*s) {
      case 'P': case 'p': fspacing = BDF_PROPORTIONAL; break;
      case 'M': case 'm': fspacing = BDF_MONOWIDTH; break;
      case 'C': case 'c': fspacing = BDF_CHARCELL; break;
      default:
          XtDisplayStringConversionWarning(d, (char *) from->addr,
                                           "FontSpacing");
          res = False;
          break;
    }
    if (res == True) {
        if (to->addr == 0)
          to->addr = (XtPointer) &fspacing;
        else if (to->size < sizeof(int))
          res = False;
        else
          *(int *) to->addr = fspacing;
        to->size = sizeof(int);
    }
    return res;
}

static Boolean
#ifndef _NO_PROTO
_CvtStringToEOLType(Display *d, XrmValuePtr args, Cardinal *num_args,
                    XrmValuePtr from, XrmValuePtr to, XtPointer *conv_data)
#else
_CvtStringToEOLType(d, args, num_args, from, to, conv_data)
Display *d;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from, to;
XtPointer *conv_data;
#endif
{
    static int eol;
    Boolean res;
    char *s;

    res = True;
    if (*num_args != 0)
      XtAppWarningMsg(XtDisplayToApplicationContext(d),
                      "CvtStringToEOLType", "wrongParameters",
                      "XmBDFEditorWarning",
                      "String to EOL type needs no extra arguments",
                      0, 0);

    s = (char *) from->addr;
    switch (*s) {
      case 'U': case 'u': eol = BDF_UNIX_EOL; break;
      case 'D': case 'd': eol = BDF_DOS_EOL; break;
      case 'M': case 'm': eol = BDF_MAC_EOL; break;
      default:
          XtDisplayStringConversionWarning(d, (char *) from->addr,
                                           "EOLType");
          res = False;
          break;
    }
    if (res == True) {
        if (to->addr == 0)
          to->addr = (XtPointer) &eol;
        else if (to->size < sizeof(int))
          res = False;
        else
          *(int *) to->addr = eol;
        to->size = sizeof(int);
    }
    return res;
}

static Boolean
#ifndef _NO_PROTO
_CvtStringToGlyphCode(Display *d, XrmValuePtr args, Cardinal *num_args,
                      XrmValuePtr from, XrmValuePtr to, XtPointer *conv_data)
#else
_CvtStringToGlyphCode(d, args, num_args, from, to, conv_data)
Display *d;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from, to;
XtPointer *conv_data;
#endif
{
    static long gcode = -1;
    int base = -1;
    Boolean res;
    char *s;

    res = True;
    if (*num_args != 0)
      XtAppWarningMsg(XtDisplayToApplicationContext(d),
                      "CvtStringToGlyphCode", "wrongParameters",
                      "XmBDFEditorWarning",
                      "String to glyph code needs no extra arguments",
                      0, 0);

    s = (char *) from->addr;
    if (*s == '\\') {
        switch (*(s + 1)) {
          case 'u': case 'x': base = 16; s += 2; break;
          case 'o': base = 8; s += 2; break;
          case 'd': base = 10; s += 2; break;
        }
    } else if ((*s == 'U' && (*(s + 1) == '+' || *(s + 1) == '-')) ||
               (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X'))) {
        base = 16;
        s += 2;
    } else if (*s == '0')
      base = 8;

    if (base < 0) {
        XtDisplayStringConversionWarning(d, (char *) from->addr,
                                         "GlyphCode");
        res = False;
    }
    if (res == True) {
        /*
         * Convert the string to an integer depending on the base.
         */
        gcode = (long) _bdf_atoul(s, 0, base);

        if (to->addr == 0)
          to->addr = (XtPointer) &gcode;
        else if (to->size < sizeof(long))
          res = False;
        else
          *(long *) to->addr = gcode;
        to->size = sizeof(long);
    }
    return res;
}

static Boolean
#ifndef _NO_PROTO
_CvtStringToCodeBase(Display *d, XrmValuePtr args, Cardinal *num_args,
                     XrmValuePtr from, XrmValuePtr to, XtPointer *conv_data)
#else
_CvtStringToCodeBase(d, args, num_args, from, to, conv_data)
Display *d;
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr from, to;
XtPointer *conv_data;
#endif
{
    static int cbase;
    Boolean res;
    char *s;

    res = True;
    if (*num_args != 0)
      XtAppWarningMsg(XtDisplayToApplicationContext(d),
                      "CvtStringToCodeBase", "wrongParameters",
                      "XmBDFEditorWarning",
                      "String to code base needs no extra arguments",
                      0, 0);

    s = (char *) from->addr;
    switch (*s) {
      case 'O': case 'o': cbase = 8; break;
      case 'D': case 'd': cbase = 10; break;
      case 'H': case 'h': cbase = 16; break;
      default:
          XtDisplayStringConversionWarning(d, (char *) from->addr,
                                           "CodeBase");
          res = False;
          break;
    }
    if (res == True) {
        if (to->addr == 0)
          to->addr = (XtPointer) &cbase;
        else if (to->size < sizeof(int))
          res = False;
        else
          *(int *) to->addr = cbase;
        to->size = sizeof(int);
    }
    return res;
}

static int
#ifndef _NO_PROTO
_XmBDFEditorHandleUnknownOptions(bdf_options_t *opts, char **params,
                                 unsigned long nparams, void *client_data)
#else
_XmBDFEditorHandleUnknownOptions(opts, params, nparams, client_data)
bdf_options_t *opts;
char **params;
unsigned long nparams;
void *client_data;
#endif
{
    MXFEditorOptions *op;

    op = (MXFEditorOptions *) client_data;

    if (nparams == 0)
      return 0;

    if (params[0][0] == 'r' &&
        memcmp(params[0], "resolution", 10) == 0) {
        op->resolution = _bdf_atoul(params[1], 0, 10);
        return 1;
    }

    if (params[0][0] == 's' &&
        memcmp(params[0], "skip_blank_pages", 16) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->no_blanks = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->no_blanks = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'r' &&
        memcmp(params[0], "really_exit", 11) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->really_exit = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->really_exit = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'g' &&
        memcmp(params[0], "grid_overwrite_mode", 19) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->overwrite_mode = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->overwrite_mode = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'g' &&
        memcmp(params[0], "generate_sbit_metrics", 21) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->sbit = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->sbit = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'c' &&
        memcmp(params[0], "close_accelerator", 17) == 0) {
        if (params[0][17] == 0) {
            /*
             * We have the accelerator itself.
             */
            if (op->accelerator != 0)
              XtFree(op->accelerator);
            if (params[1] == 0 || params[1][0] == 0)
              op->accelerator = 0;
            else {
                op->accelerator = XtMalloc(strlen(params[1]) + 1);
                (void) strcpy(op->accelerator, params[1]);
            }
        } else if (params[0][17] == '_') {
            /*
             * We probably have 'close_accelerator_text'.
             */
            if (op->accelerator_text != 0)
              XtFree(op->accelerator_text);
            if (params[1] == 0 || params[1][0] == 0)
              op->accelerator_text = 0;
            else {
                op->accelerator_text = XtMalloc(strlen(params[1]) + 1);
                (void) strcpy(op->accelerator_text, params[1]);
            }
        }
        return 1;
    }

    if (params[0][0] == 'p' && memcmp(params[0], "pixel_size", 10) == 0) {
        op->pixel_size = _bdf_atoul(params[1], 0, 10);
        return 1;
    }

    if (params[0][0] == 's' && memcmp(params[0], "show_cap_height", 15) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->show_cap_height = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->show_cap_height = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 's' && memcmp(params[0], "show_x_height", 13) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->show_x_height = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->show_x_height = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'f' &&
        memcmp(params[0], "font_grid_horizontal", 20) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->vertical = 1;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->vertical = 0;
            break;
        }
        return 1;
    }        

    if (params[0][0] == 'p' && memcmp(params[0], "power2", 6) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->power2 = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->power2 = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'p' && memcmp(params[0], "percentage_only", 15) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->percent_only = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->percent_only = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'p' && memcmp(params[0], "progress_bar", 12) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->progbar = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->progbar = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'm' && memcmp(params[0], "make_backups", 12) == 0) {
        switch (params[1][0]) {
          case '0': case 'F': case 'f': case 'N': case 'n':
            op->backups = 0;
            break;
          case '1': case 'T': case 't': case 'Y': case 'y':
            op->backups = 1;
            break;
        }
        return 1;
    }

    if (params[0][0] == 'c' && memcmp(params[0], "code_base", 9) == 0) {
        switch (params[1][0]) {
          case 'o': case 'O': op->code_base = 8; break;
          case 'd': case 'D': op->code_base = 10; break;
          case 'h': case 'H': op->code_base = 16; break;
          default: op->code_base = 16;
        }
        return 1;
    }

    if ((params[0][0] == 'n' && memcmp(params[0], "name_file", 9) == 0) ||
        (params[0][0] == 'u' &&
         memcmp(params[0], "unicode_name_file", 17) == 0)) {
        if (op->unicode_name_file != 0)
          XtFree(op->unicode_name_file);
        if (params[1] == 0 || params[1][0] == 0)
          op->unicode_name_file = 0;
        else {
            op->unicode_name_file = XtMalloc(strlen(params[1]) + 1);
            (void) strcpy(op->unicode_name_file, params[1]);
        }
        return 1;
    }

    if (params[0][0] == 'a' && memcmp(params[0], "adobe_name_file", 15) == 0) {
        if (op->adobe_name_file != 0)
          XtFree(op->adobe_name_file);
        if (params[1] == 0 || params[1][0] == 0)
          op->adobe_name_file = 0;
        else {
            op->adobe_name_file = XtMalloc(strlen(params[1]) + 1);
            (void) strcpy(op->adobe_name_file, params[1]);
        }
        return 1;
    }

    /*
     * Handle the colors.
     */
    if (params[0][0] == 'c' && memcmp(params[0], "color", 5) == 0) {
        int idx;

        idx = (int) _bdf_atol(params[0] + 5, 0, 10);
        op->colors[idx] = (unsigned short) _bdf_atos(params[1], 0, 10);
        return 1;
    }

    return 0;
}

/*
 * This function does some intialization before any of the X11 stuff happens.
 */
static void
#ifndef _NO_PROTO
_XmBDFEditorSetup(void)
#else
_XmBDFEditorSetup()
#endif
{
    int i;
    FILE *in;
    char *home, path[BUFSIZ];

    /*
     * Initialize the BDF support library.
     */
    bdf_setup();

    /*
     * Get the default BDF options.
     */
    bdf_default_options(&xmbdfed_opts.font_opts);

    /*
     * Set the default editor options.
     */
    xmbdfed_opts.accelerator = xmbdfed_opts.accelerator_text =
        xmbdfed_opts.unicode_name_file = xmbdfed_opts.adobe_name_file = 0;
    xmbdfed_opts.pixel_size = 10;
    xmbdfed_opts.resolution = 0;
    xmbdfed_opts.no_blanks = xmbdfed_opts.really_exit = 
        xmbdfed_opts.overwrite_mode = xmbdfed_opts.power2 =
        xmbdfed_opts.progbar = xmbdfed_opts.backups = 1;
    xmbdfed_opts.show_cap_height = xmbdfed_opts.show_x_height =
        xmbdfed_opts.vertical = xmbdfed_opts.percent_only =
        xmbdfed_opts.sbit = 0;
    xmbdfed_opts.initial_glyph = -1;
    xmbdfed_opts.code_base = 16;

    /*
     * Set the default colors.
     */
    xmbdfed_opts.colors_allocated = 0;

    /*
     * Set the pixels to what should be an invalid value.
     */
    for (i = 0; i < 20; i++)
      xmbdfed_opts.pixels[i] = ~0;

    /*
     * Set the default colors for 2 bits per pixel.
     */
    xmbdfed_opts.colors[0] = 61430;
    xmbdfed_opts.colors[1] = 53238;
    xmbdfed_opts.colors[2] = 45046;
    xmbdfed_opts.colors[3] = 0;

    /*
     * Set the default colors for 4 bits per pixel.
     */
    xmbdfed_opts.colors[4] = 61430;
    xmbdfed_opts.colors[5] = 59382;
    xmbdfed_opts.colors[6] = 57334;
    xmbdfed_opts.colors[7] = 55286;
    xmbdfed_opts.colors[8] = 53238;
    xmbdfed_opts.colors[9] = 51190;
    xmbdfed_opts.colors[10] = 49142;
    xmbdfed_opts.colors[11] = 47094;
    xmbdfed_opts.colors[12] = 45046;
    xmbdfed_opts.colors[13] = 42998;
    xmbdfed_opts.colors[14] = 40950;
    xmbdfed_opts.colors[15] = 32760;
    xmbdfed_opts.colors[16] = 24570;
    xmbdfed_opts.colors[17] = 16380;
    xmbdfed_opts.colors[18] = 8190;
    xmbdfed_opts.colors[19] = 0;

    /*
     * Attempt to load the user config file.
     */
    if ((home = getenv("HOME")) != 0) {
        sprintf(path, "%s/.xmbdfedrc", home);
        if ((in = fopen(path, "r")) != 0) {
            bdf_load_options(in, &xmbdfed_opts.font_opts,
                             _XmBDFEditorHandleUnknownOptions,
                             (void *) &xmbdfed_opts);
            fclose(in);
        }
    }

    /*
     * Update the command line resources from the configuration just loaded.
     * This will cause the current options to be preserved when the command
     * line is parsed by Xt.  The last two command line resources are handled
     * in the normal fashion.
     */
    cmdres[0].default_addr =
        (XtPointer) xmbdfed_opts.font_opts.correct_metrics;
    cmdres[1].default_addr =
        (XtPointer) xmbdfed_opts.font_opts.keep_unencoded;
    cmdres[2].default_addr =
        (XtPointer) xmbdfed_opts.font_opts.keep_comments;

    cmdres[3].default_addr = (XtPointer) xmbdfed_opts.font_opts.pad_cells;
    cmdres[4].default_addr = (XtPointer) xmbdfed_opts.font_opts.point_size;
    cmdres[5].default_addr = (XtPointer) xmbdfed_opts.font_opts.resolution_x;
    cmdres[6].default_addr = (XtPointer) xmbdfed_opts.font_opts.resolution_y;
    cmdres[7].default_addr = (XtPointer) xmbdfed_opts.font_opts.font_spacing;
    cmdres[8].default_addr = (XtPointer) xmbdfed_opts.font_opts.bits_per_pixel;
    cmdres[9].default_addr = (XtPointer) xmbdfed_opts.font_opts.eol;
    cmdres[10].default_addr = (XtPointer) xmbdfed_opts.resolution;
    cmdres[11].default_addr = (XtPointer) xmbdfed_opts.no_blanks;
    cmdres[12].default_addr = (XtPointer) xmbdfed_opts.really_exit;
    cmdres[13].default_addr = (XtPointer) xmbdfed_opts.overwrite_mode;
    cmdres[14].default_addr = (XtPointer) xmbdfed_opts.initial_glyph;
    cmdres[15].default_addr = (XtPointer) xmbdfed_opts.progbar;
    cmdres[16].default_addr = (XtPointer) xmbdfed_opts.code_base;
}

int
#ifndef _NO_PROTO
main(int argc, char *argv[])
#else
main(argc, argv)
int argc;
char *argv[];
#endif
{
    Display *d;
    int i;
    Arg av[1];

    /*
     * Load the config file if it exists and prep the command line resource
     * values with those loaded from the config file.
     */
    _XmBDFEditorSetup();

    /*
     * Create the top level.
     */
    XtSetArg(av[0], XmNallowShellResize, True);
    top = XtAppInitialize(&app, "XmBDFEdit", cmdopts, XtNumber(cmdopts),
                          &argc, argv, fallback_resources, av, 1);

    d = XtDisplay(top);

    /*
     * Add the type converter for font spacing values so they can be handled
     * from the command line.
     */
    XtSetTypeConverter(XmRString, "FontSpacing", _CvtStringToFontSpacing,
                       0, 0, XtCacheNone, 0);
    XtSetTypeConverter(XmRString, "EOLType", _CvtStringToEOLType,
                       0, 0, XtCacheNone, 0);
    XtSetTypeConverter(XmRString, "GlyphCode", _CvtStringToGlyphCode,
                       0, 0, XtCacheNone, 0);
    XtSetTypeConverter(XmRString, "CodeBase", _CvtStringToCodeBase,
                       0, 0, XtCacheNone, 0);

    /*
     * Get the recognizable command line options and set the actual font
     * options accordingly.
     */
    XtGetApplicationResources(top, (XtPointer) &xmbdfed_opts, cmdres,
                              XtNumber(cmdres), 0, 0);

    XtGetApplicationNameAndClass(d, &app_name, &app_class);

    /*
     * Make sure the bits per pixel value is OK.
     */
    if (xmbdfed_opts.font_opts.bits_per_pixel > 4 ||
        (xmbdfed_opts.font_opts.bits_per_pixel > 1 &&
         (xmbdfed_opts.font_opts.bits_per_pixel & 1))) {
        fprintf(stderr, "%s: invalid bits per pixel \"%d\".  ", app_name,
                xmbdfed_opts.font_opts.bits_per_pixel);
        fprintf(stderr, "Setting to 1.\n");
        xmbdfed_opts.font_opts.bits_per_pixel = 1;
    }

    /*
     * Set the horizontal and vertical resolutions if necessary.
     */
    if (xmbdfed_opts.resolution != 0)
      xmbdfed_opts.font_opts.resolution_x =
          xmbdfed_opts.font_opts.resolution_y = xmbdfed_opts.resolution;

    if (xmbdfed_opts.font_opts.resolution_x == 0)
      xmbdfed_opts.font_opts.resolution_x =
          (long) (((((double) DisplayWidth(d, DefaultScreen(d))) * 25.4) /
                   ((double) DisplayWidthMM(d, DefaultScreen(d)))) + 0.5);

    if (xmbdfed_opts.font_opts.resolution_y == 0)
      xmbdfed_opts.font_opts.resolution_y =
          (long) (((((double) DisplayHeight(d, DefaultScreen(d))) * 25.4) /
                   ((double) DisplayHeightMM(d, DefaultScreen(d)))) + 0.5);

    argc--;
    argv++;

    if (argc == 0)
      (void) MakeEditor(top, 0, True);
    else {
        for (i = 0; i < argc; i++)
          (void) MakeEditor(top, argv[i], True);
    }

    XtRealizeWidget(top);

    /*
     * Make sure the initial font grid has the focus.
     */
    (void) XmProcessTraversal(editors[0].fgrid, XmTRAVERSE_CURRENT);

    XtAppMainLoop(app);

    return 0;
}

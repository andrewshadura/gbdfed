/*
 * Copyright 2004 Computing Research Labs, New Mexico State University
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
static char rcsid[] __attribute__ ((unused)) = "$Id: setup.c,v 1.20 2004/02/12 15:29:17 mleisher Exp $";
#else
static char rcsid[] = "$Id: setup.c,v 1.20 2004/02/12 15:29:17 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/TextF.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include "FGrid.h"
#include "xmbdfed.h"

typedef struct {
    Widget shell;
    Widget form;
    Widget hint;
    Widget ok;
    Widget file;
    Widget adobe_file;
    Widget fsel;
    Widget cheight;
    Widget xheight;
    Widget progbar;
    Widget percent;
    Widget sbit;
    Widget psize_opts;
    Widget psizes[9];
} MXFEditorOtherOptions;

typedef struct {
    Widget shell;
    Widget form;
    Widget insert;
    Widget overwrite;
    Widget make_backups;
    Widget correct_metrics;
    Widget keep_unencoded;
    Widget keep_comments;
    Widget pad_cells;
    Widget psize;
    Widget hres;
    Widget vres;
    Widget prop;
    Widget mono;
    Widget cell;
    Widget save;
    Widget colors;
    Widget update;
    Widget bpps[4];
    Widget eols[4];
    /*
     * Value holders used until the Update button is pressed.
     */
    int pixsize;
    int bpp;
    int eol;
    Boolean saved;
} MXFEditorSetup;

static unsigned long active_editor;
static MXFEditorSetup setup;
static MXFEditorOtherOptions other;

static void
#ifndef _NO_PROTO
EnableUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnableUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XtSetSensitive(setup.update, True);
}

static void
#ifndef _NO_PROTO
DoUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    unsigned long i;
    char *s;
    Arg av[1];

    ed = &editors[active_editor];

    xmbdfed_opts.overwrite_mode =
        (XmToggleButtonGetState(setup.overwrite) == True) ? 1 : 0;

    if (other.shell) {
        /*
         * Set the show cap and x height values.
         */
        xmbdfed_opts.show_cap_height =
            (XmToggleButtonGetState(other.cheight) == True) ? 1 : 0;
        xmbdfed_opts.show_x_height =
            (XmToggleButtonGetState(other.xheight) == True) ? 1 : 0;

        /*
         * The Unicode glyph name file.
         */
        if ((s = XmTextFieldGetString(other.file)) != 0) {
            if (*s != 0 && (xmbdfed_opts.unicode_name_file == 0 ||
                            strcmp(s, xmbdfed_opts.unicode_name_file) != 0)) {
                if (xmbdfed_opts.unicode_name_file)
                  XtFree(xmbdfed_opts.unicode_name_file);
                xmbdfed_opts.unicode_name_file = s;
            } else
              XtFree(s);
        }

        /*
         * The SBIT metrics generation option.
         */
        xmbdfed_opts.sbit =
            (XmToggleButtonGetState(other.sbit) == True) ? 1 : 0;
    }

    /*
     * Make sure this specific font grid has its overwrite flag set.
     */
    XtSetArg(av[0], XmNgridOverwriteMode, xmbdfed_opts.overwrite_mode);
    XtSetValues(ed->fgrid, av, 1);

    xmbdfed_opts.backups =
        (XmToggleButtonGetState(setup.make_backups) == True) ? 1 : 0;

    if (other.hint) {
        /*
         * Only do this if the hint toggle has been created.
         */
        if (XmToggleButtonGetState(other.hint) == True)
          xmbdfed_opts.font_opts.otf_flags &= ~FT_LOAD_NO_HINTING;
        else
          xmbdfed_opts.font_opts.otf_flags |= FT_LOAD_NO_HINTING;
    }

    xmbdfed_opts.font_opts.correct_metrics =
        (XmToggleButtonGetState(setup.correct_metrics) == True) ? 1 : 0;
    xmbdfed_opts.font_opts.keep_unencoded =
        (XmToggleButtonGetState(setup.keep_unencoded) == True) ? 1 : 0;
    xmbdfed_opts.font_opts.keep_comments =
        (XmToggleButtonGetState(setup.keep_comments) == True) ? 1 : 0;
    xmbdfed_opts.font_opts.pad_cells =
        (XmToggleButtonGetState(setup.pad_cells) == True) ? 1 : 0;

    s = XmTextFieldGetString(setup.psize);
    xmbdfed_opts.font_opts.point_size = _bdf_atol(s, 0, 10);
    XtFree(s);

    s = XmTextFieldGetString(setup.hres);
    xmbdfed_opts.font_opts.resolution_x = _bdf_atoul(s, 0, 10);
    XtFree(s);

    s = XmTextFieldGetString(setup.vres);
    xmbdfed_opts.font_opts.resolution_y = _bdf_atoul(s, 0, 10);
    XtFree(s);

    if (XmToggleButtonGetState(setup.prop) == True)
      xmbdfed_opts.font_opts.font_spacing = BDF_PROPORTIONAL;
    else if (XmToggleButtonGetState(setup.mono) == True)
      xmbdfed_opts.font_opts.font_spacing = BDF_MONOWIDTH;
    else
      xmbdfed_opts.font_opts.font_spacing = BDF_CHARCELL;

    /*
     * Make sure all editors have their "Auto Corrections" View menu option
     * enabled or disabled depending on the "Correct Metrics" option.
     */
    for (i = 0; i < num_editors; i++) {
        if (xmbdfed_opts.font_opts.correct_metrics != 0 &&
            XmuttFontGridFontMessages(editors[i].fgrid) != 0)
          XtSetSensitive(editors[i].viewmenu.acmsgs, True);
        else
          XtSetSensitive(editors[i].viewmenu.acmsgs, False);
    }

    if (setup.pixsize != xmbdfed_opts.pixel_size) {
        /*
         * Update the glyph editors with a new pixel size.
         */
        xmbdfed_opts.pixel_size = setup.pixsize;
        GlyphEditChangeSetupValues(xmbdfed_opts.pixel_size,
                                   xmbdfed_opts.show_cap_height,
                                   xmbdfed_opts.show_x_height);
    }

    /*
     * Update the color values if they have been changed.
     */
    xmbdfedUpdateColorValues();

    if (setup.bpp != xmbdfed_opts.font_opts.bits_per_pixel)
      xmbdfed_opts.font_opts.bits_per_pixel = setup.bpp;

    if (setup.eol != xmbdfed_opts.font_opts.eol)
      xmbdfed_opts.font_opts.eol = setup.eol;

    setup.saved = False;
    XtSetSensitive(setup.save, True);
    XtSetSensitive(setup.update, False);
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
    int i, c;
    FILE *out;
    char *home, path[BUFSIZ];

    if ((home = getenv("HOME")) != 0) {
        sprintf(path, "%s/.xmbdfedrc", home);
        if ((out = fopen(path, "w")) != 0) {
            /*
             * First, write the XmBDFEditor options.
             */
            fprintf(out, "#########################\n");
            fprintf(out, "#\n# XmBDFEditor options.\n#\n");
            fprintf(out, "#########################\n\n");

            if (xmbdfed_opts.no_blanks)
              fprintf(out, "skip_blank_pages true\n\n");
            else
              fprintf(out, "skip_blank_pages false\n\n");

            if (xmbdfed_opts.really_exit)
              fprintf(out, "really_exit true\n\n");
            else
              fprintf(out, "really_exit false\n\n");
            if (xmbdfed_opts.overwrite_mode)
              fprintf(out, "grid_overwrite_mode true\n\n");
            else
              fprintf(out, "grid_overwrite_mode false\n\n");

            if (xmbdfed_opts.accelerator != 0)
              fprintf(out, "close_accelerator %s\n\n",
                      xmbdfed_opts.accelerator);
            if (xmbdfed_opts.accelerator_text != 0)
              fprintf(out, "close_accelerator_text %s\n\n",
                      xmbdfed_opts.accelerator_text);

            if (xmbdfed_opts.unicode_name_file != 0)
              fprintf(out, "name_file %s\n\n", xmbdfed_opts.unicode_name_file);

            if (xmbdfed_opts.adobe_name_file != 0)
              fprintf(out, "adobe_name_file %s\n\n",
                      xmbdfed_opts.adobe_name_file);

            fprintf(out, "pixel_size %ld\n\n", xmbdfed_opts.pixel_size);

            if (xmbdfed_opts.show_cap_height)
              fprintf(out, "show_cap_height true\n\n");
            else
              fprintf(out, "show_cap_height false\n\n");

            if (xmbdfed_opts.show_x_height)
              fprintf(out, "show_x_height true\n\n");
            else
              fprintf(out, "show_x_height false\n\n");

            if (xmbdfed_opts.progbar)
              fprintf(out, "progress_bar true\n\n");
            else
              fprintf(out, "progress_bar false\n\n");

            if (xmbdfed_opts.percent_only)
              fprintf(out, "percentage_only true\n\n");
            else
              fprintf(out, "percentage_only false\n\n");

            if (xmbdfed_opts.sbit)
              fprintf(out, "generate_sbit_metrics true\n\n");
            else
              fprintf(out, "generate_sbit_metrics false\n\n");

            /*
             * Save the colors.
             */
            fprintf(out, "#\n# Color values for 2 bits per pixel.\n#\n");
            for (i = 0; i < 4; i++) {
                /*
                 * Do this to avoid writing negative values.
                 */
                c = xmbdfed_opts.colors[i];
                fprintf(out, "color%d %d\n", i, c);
            }

            fprintf(out, "\n#\n# Color values for 4 bits per pixel.\n#\n");
            for (i = 4; i < 20; i++) {
                /*
                 * Do this to avoid writing negative values.
                 */
                c = xmbdfed_opts.colors[i];
                fprintf(out, "color%d %d\n", i, c);
            }
            putc('\n', out);

            /*
             * The save the BDF specific options.
             */
            fprintf(out, "#########################\n");
            fprintf(out, "#\n# BDF font options.\n#\n");
            fprintf(out, "#########################\n\n");
            bdf_save_options(out, &xmbdfed_opts.font_opts);
            fclose(out);
            setup.saved = True;
        } else
          ErrorDialog("Unable to save setup!");
    } else
      ErrorDialog("Unable to locate user home directory!");

    if (setup.saved == True)
      XtSetSensitive(setup.save, False);
}

static void
#ifndef _NO_PROTO
SetProgBar(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetProgBar(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int which;
    XmToggleButtonCallbackStruct *cb;

    which = (int) client_data;
    cb = (XmToggleButtonCallbackStruct *) call_data;

    if (which == 1) {
        xmbdfed_opts.progbar = (cb->set == True) ? 1 : 0;
        if (xmbdfed_opts.progbar)
          XtSetSensitive(other.percent, True);
        else
          XtSetSensitive(other.percent, False);
    } else
      xmbdfed_opts.percent_only = (cb->set == True) ? 1 : 0;

    XtSetSensitive(setup.update, True);
}

static void
#ifndef _NO_PROTO
SetPixelSize(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetPixelSize(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    setup.pixsize = (int) client_data;
    XtSetSensitive(setup.update, True);
}

static void
#ifndef _NO_PROTO
SetBitsPerPixel(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetBitsPerPixel(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    if ((setup.bpp = (int) client_data) > 1) {
        xmbdfedAllocateColors(top, (int) client_data);
        XtSetSensitive(setup.colors, True);
    } else
      XtSetSensitive(setup.colors, False);
    XtSetSensitive(setup.update, True);
}

static void
#ifndef _NO_PROTO
SetEOLType(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetEOLType(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    setup.eol = (int) client_data;
    XtSetSensitive(setup.update, True);
}

static void
#ifndef _NO_PROTO
SetGlyphFileName(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetGlyphFileName(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int adobe;
    FILE *in;
    char *path;
    XmFileSelectionBoxCallbackStruct *cb;

    adobe = (int) client_data;

    cb = (XmFileSelectionBoxCallbackStruct *) call_data;
    XmStringGetLtoR(cb->value, XmSTRING_DEFAULT_CHARSET, &path);

    if ((in = fopen(path, "r")) == 0) {
        sprintf(name, "Unable to open file\n%s", path);
        ErrorDialog(name);
        XtFree(path);
        return;
    }
    fclose(in);

    if (adobe) {
        XmTextFieldSetString(other.adobe_file, path);
        XmTextFieldSetCursorPosition(other.adobe_file, strlen(path));
    } else {
        XmTextFieldSetString(other.file, path);
        XmTextFieldSetCursorPosition(other.file, strlen(path));
    }
    XtFree(path);

    XtPopdown(XtParent(w));
}

static void
#ifndef _NO_PROTO
DoCloseSetup(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCloseSetup(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    /*
     * Make sure the original color values are restored if they have not been
     * updated.
     */
    xmbdfedRestoreColorValues();
    XtPopdown((Widget) client_data);
}

static void
#ifndef _NO_PROTO
ShowFileBrowser(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowFileBrowser(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int i, adobe;
    XtCallbackList cl;
    Arg av[1];

    adobe = (int) client_data;

    if (other.fsel == 0) {
        other.fsel = XmCreateFileSelectionDialog(top,
                                                 "xmbdfed_gname_file_sel_box",
                                                 0, 0);
        XtAddCallback(other.fsel, XmNokCallback, SetGlyphFileName, 0);
        XtAddCallback(other.fsel, XmNcancelCallback, DoClose,
                      (XtPointer) XtParent(other.fsel));
        XtUnmanageChild(XmFileSelectionBoxGetChild(other.fsel,
                                                   XmDIALOG_HELP_BUTTON));
        XtManageChild(other.fsel);
    }

    if (adobe)
      sprintf(title, "%s: Adobe Glyph Name File", app_name);
    else
      sprintf(title, "%s: Unicode Glyph Name File", app_name);
    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(XtParent(other.fsel), av, 1);

    XtSetArg(av[0], XmNokCallback, &cl);
    XtGetValues(other.fsel, av, 1);

    /*
     * Locate the callback we are looking for so we can change the client
     * data.
     */
    for (i = 0; cl[i].callback != 0; i++) {
        if (cl[i].callback == SetGlyphFileName) {
            cl[i].closure = client_data;
            break;
        }
    }

    XtPopup(XtParent(other.fsel), XtGrabNone);
}

static void
#ifndef _NO_PROTO
DoCloseOtherOptions(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCloseOtherOptions(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    /*
     * Make sure the file browser is popped down.
     */
    if (other.fsel != 0)
      XtPopdown(XtParent(other.fsel));
    XtPopdown(other.shell);
}

static void
#ifndef _NO_PROTO
DoOtherOptions(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoOtherOptions(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int i;
    Widget frame, opts, pdown, label, pb, form;
    XmString s;
    Cardinal ac;
    Arg av[10];

    if (other.shell == 0) {
        sprintf(title, "%s - Other Options", app_name);
        ac = 0;
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetArg(av[ac], XmNallowShellResize, True); ac++;
        XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
        other.shell = XtCreatePopupShell("xmbdfed_options_shell",
                                         xmDialogShellWidgetClass, top,
                                         av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
        XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
        XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
        other.form = XtCreateWidget("xmbdfed_other_form", xmFormWidgetClass,
                                    other.shell, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_other_ttf_frame",
                                      xmFrameWidgetClass, other.form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("True Type Options", xmLabelWidgetClass,
                                      frame, av, ac);

        XtSetArg(av[0], XmNalignment, XmALIGNMENT_BEGINNING);
        other.hint = XtCreateManagedWidget("Hint Glyphs",
                                           xmToggleButtonWidgetClass, frame,
                                           av, 1);
        XtAddCallback(other.hint, XmNvalueChangedCallback, EnableUpdate, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_other_gname_frame",
                                      xmFrameWidgetClass, other.form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("Unicode Glyph Name File",
                                      xmLabelWidgetClass, frame, av, ac);

        form = XtCreateManagedWidget("xmbdfed_gname_file_form",
                                     xmFormWidgetClass, frame, 0, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        pb = XtCreateManagedWidget("Browse", xmPushButtonWidgetClass, form,
                                   av, ac);
        XtAddCallback(pb, XmNactivateCallback, ShowFileBrowser, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNrightWidget, pb); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNcolumns, 40); ac++;
        other.file = XtCreateManagedWidget("xmbdfed_gname_filename",
                                           xmTextFieldWidgetClass, form,
                                           av, ac);
        /*
         * Set the initial value if there is one before the value changed
         * callback is set so it doesn't accidently turn on the update button.
         */
        if (xmbdfed_opts.unicode_name_file != 0) {
            XmTextFieldSetString(other.file, xmbdfed_opts.unicode_name_file);
            XmTextFieldSetCursorPosition(other.file,
                                         strlen(xmbdfed_opts.unicode_name_file));
        }

        XtAddCallback(other.file, XmNvalueChangedCallback, EnableUpdate, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_other_gname_frame1",
                                      xmFrameWidgetClass, other.form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("Adobe Glyph Name File",
                                      xmLabelWidgetClass, frame, av, ac);

        form = XtCreateManagedWidget("xmbdfed_gname_file_form1",
                                     xmFormWidgetClass, frame, 0, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        pb = XtCreateManagedWidget("Browse", xmPushButtonWidgetClass, form,
                                   av, ac);
        XtAddCallback(pb, XmNactivateCallback, ShowFileBrowser, (XtPointer) 1);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNrightWidget, pb); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNcolumns, 40); ac++;
        other.adobe_file = XtCreateManagedWidget("xmbdfed_gname_filename1",
                                                 xmTextFieldWidgetClass, form,
                                                 av, ac);
        /*
         * Set the initial value if there is one before the value changed
         * callback is set so it doesn't accidently turn on the update button.
         */
        if (xmbdfed_opts.adobe_name_file != 0) {
            XmTextFieldSetString(other.adobe_file,
                                 xmbdfed_opts.adobe_name_file);
            XmTextFieldSetCursorPosition(other.adobe_file,
                                         strlen(xmbdfed_opts.adobe_name_file));
        }

        XtAddCallback(other.adobe_file, XmNvalueChangedCallback,
                      EnableUpdate, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_other_gedit_frame",
                                      xmFrameWidgetClass, other.form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("Glyph Editor Options",
                                      xmLabelWidgetClass, frame, av, ac);

        form = XtCreateManagedWidget("xmbdfed_gedit_options_form",
                                     xmFormWidgetClass, frame, 0, 0);

        s = XmStringCreateSimple("Pixel Size:");
        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNlabelString, s); ac++;
        other.psize_opts = opts =
            XmCreateOptionMenu(form, "xmbdfed_other_pixel_size", av, ac);
        XmStringFree(s);

        pdown = XmCreatePulldownMenu(opts, "xmbdfed_pixel_sizes", 0, 0);

        ac = 0;
        XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
        XtSetValues(opts, av, ac);
        for (i = 2; i <= 10; i++) {
            sprintf(name, "%dx%d", i, i);
            other.psizes[i - 2] = pb =
                XtCreateManagedWidget(name, xmPushButtonWidgetClass, pdown,
                                      0, 0);
            XtAddCallback(pb, XmNactivateCallback, SetPixelSize,
                          (XtPointer) i);
            if (i == xmbdfed_opts.pixel_size) {
                XtSetArg(av[0], XmNmenuHistory, pb);
                XtSetValues(opts, av, 1);
            }
        }

        XtManageChild(opts);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, opts); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        pb = XtCreateManagedWidget("xmbdfed_gedit_options_sep",
                                   xmSeparatorWidgetClass, form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, pb); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
        other.cheight = XtCreateManagedWidget("Show Cap Height",
                                              xmToggleButtonWidgetClass,
                                              form, av, ac);
        XtAddCallback(other.cheight, XmNvalueChangedCallback, EnableUpdate, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, pb); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, other.cheight); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
        other.xheight = XtCreateManagedWidget("Show X Height",
                                              xmToggleButtonWidgetClass,
                                              form, av, ac);
        XtAddCallback(other.xheight, XmNvalueChangedCallback, EnableUpdate, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_progbar_frame",
                                      xmFrameWidgetClass, other.form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("Progress Bar Options",
                                      xmLabelWidgetClass, frame, av, ac);

        form = XtCreateManagedWidget("xmbdfed_gedit_options_form1",
                                     xmFormWidgetClass, frame, 0, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNset, False); ac++;
        XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
        other.progbar = XtCreateManagedWidget("Use Progress Bar",
                                              xmToggleButtonWidgetClass, form,
                                              av, ac);
        XtAddCallback(other.progbar, XmNvalueChangedCallback, SetProgBar,
                      (XtPointer) 1);

        ac = 0;
        XtSetArg(av[ac], XmNset, False); ac++;
        XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, other.progbar); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        other.percent = XtCreateManagedWidget("Percentage Only",
                                              xmToggleButtonWidgetClass, form,
                                              av, ac);
        XtAddCallback(other.percent, XmNvalueChangedCallback, SetProgBar,
                      (XtPointer) 2);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_sbit_frame",
                                      xmFrameWidgetClass, other.form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("SBIT Metrics Options",
                                      xmLabelWidgetClass, frame, av, ac);

        XtSetArg(av[0], XmNset, False);
        XtSetArg(av[1], XmNalignment, XmALIGNMENT_BEGINNING);
        other.sbit = XtCreateManagedWidget("Generate SBIT Metrics File",
                                           xmToggleButtonWidgetClass, frame,
                                           av, 2);
        XtAddCallback(other.sbit, XmNvalueChangedCallback, EnableUpdate, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_other_button_frame",
                                      xmFrameWidgetClass, other.form, av, ac);

        pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, frame,
                                   0, 0);
        XtAddCallback(pb, XmNactivateCallback, DoCloseOtherOptions, 0);
    }

    /*
     * Set the current toggle values in the other options.
     */
    if (xmbdfed_opts.font_opts.otf_flags & FT_LOAD_NO_HINTING)
      XmToggleButtonSetState(other.hint, False, False);
    else
      XmToggleButtonSetState(other.hint, True, False);

    if (xmbdfed_opts.show_cap_height)
      XmToggleButtonSetState(other.cheight, True, False);
    else
      XmToggleButtonSetState(other.cheight, False, False);

    if (xmbdfed_opts.show_x_height)
      XmToggleButtonSetState(other.xheight, True, False);
    else
      XmToggleButtonSetState(other.xheight, False, False);

    if (xmbdfed_opts.progbar)
      XmToggleButtonSetState(other.progbar, True, False);
    else
      XmToggleButtonSetState(other.progbar, False, False);

    if (xmbdfed_opts.progbar) {
        XtSetSensitive(other.percent, True);

        if (xmbdfed_opts.percent_only)
          XmToggleButtonSetState(other.percent, True, False);
        else
          XmToggleButtonSetState(other.percent, False, False);
    } else
      XtSetSensitive(other.percent, False);

    if (xmbdfed_opts.sbit)
      XmToggleButtonSetState(other.sbit, True, False);
    else
      XmToggleButtonSetState(other.sbit, False, False);

    /*
     * Set the pixel size.
     */
    XtSetArg(av[0], XmNmenuHistory, other.psizes[xmbdfed_opts.pixel_size - 2]);
    XtSetValues(other.psize_opts, av, 1);

    XtManageChild(other.form);
    XtPopup(other.shell, XtGrabNone);
}

static void
#ifndef _NO_PROTO
BuildSetupDialog(void)
#else
BuildSetupDialog()
#endif
{
    Widget frame, rc, label, pb, form, opts, pdown;
    XmString s;
    Cardinal ac;
    Arg av[10];

    setup.saved = True;

    sprintf(title, "%s - Configuration", app_name);
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    setup.shell = XtCreatePopupShell("xmbdfed_setup_shell",
                                     xmDialogShellWidgetClass, top, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    setup.form = XtCreateWidget("xmbdfed_setup_form", xmFormWidgetClass,
                                setup.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_setup_edit_frame",
                                  xmFrameWidgetClass, setup.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Font Grid Edit Options", xmLabelWidgetClass,
                                  frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    rc = XtCreateManagedWidget("xmbdfed_setup_edit_rowcol",
                               xmRowColumnWidgetClass, frame, av, ac);

    setup.insert = XtCreateManagedWidget("Insert Mode",
                                       xmToggleButtonWidgetClass, rc, 0, 0);
    XtAddCallback(setup.insert, XmNvalueChangedCallback, EnableUpdate, 0);

    setup.overwrite = XtCreateManagedWidget("Overwrite Mode",
                                       xmToggleButtonWidgetClass, rc, 0, 0);
    XtAddCallback(setup.overwrite, XmNvalueChangedCallback, EnableUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_setup_fload_frame",
                                  xmFrameWidgetClass, setup.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Font Load/Save Options", xmLabelWidgetClass,
                                  frame, av, ac);

    form = XtCreateManagedWidget("xmbdfed_setup_edit_form", xmFormWidgetClass,
                                 frame, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNnumColumns, 2); ac++;
    XtSetArg(av[ac], XmNpacking, XmPACK_COLUMN); ac++;
    rc = XtCreateManagedWidget("xmbdfed_setup_fload_rowcol",
                               xmRowColumnWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleWhenOff, True); ac++;
    setup.make_backups = XtCreateManagedWidget("Make Backups",
                                               xmToggleButtonWidgetClass,
                                               rc, av, ac);
    XtAddCallback(setup.make_backups, XmNvalueChangedCallback,
                  EnableUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleWhenOff, True); ac++;
    setup.correct_metrics = XtCreateManagedWidget("Correct Metrics",
                                                  xmToggleButtonWidgetClass,
                                                  rc, av, ac);
    XtAddCallback(setup.correct_metrics, XmNvalueChangedCallback,
                  EnableUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleWhenOff, True); ac++;
    setup.keep_unencoded = XtCreateManagedWidget("Keep Unencoded",
                                                 xmToggleButtonWidgetClass,
                                                 rc, av, ac);
    XtAddCallback(setup.keep_unencoded, XmNvalueChangedCallback,
                  EnableUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleWhenOff, True); ac++;
    setup.keep_comments = XtCreateManagedWidget("Keep Comments",
                                                xmToggleButtonWidgetClass,
                                                rc, av, ac);
    XtAddCallback(setup.keep_comments, XmNvalueChangedCallback,
                  EnableUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleWhenOff, True); ac++;
    setup.pad_cells = XtCreateManagedWidget("Pad Character Cells",
                                            xmToggleButtonWidgetClass,
                                            rc, av, ac);
    XtAddCallback(setup.pad_cells, XmNvalueChangedCallback,
                  EnableUpdate, 0);

    s = XmStringCreateSimple("EOL:");
    ac = 0;
    XtSetArg(av[ac], XmNlabelString, s); ac++;
    setup.eols[3] = opts =
        XmCreateOptionMenu(rc, "xmbdfed_eol_menu", av, ac);
    XmStringFree(s);

    pdown = XmCreatePulldownMenu(opts, "xmbdfed_eol_options", 0, 0);

    XtSetArg(av[0], XmNsubMenuId, pdown);
    XtSetValues(opts, av, 1);

    setup.eols[0] =
        XtCreateManagedWidget("Unix (LF)", xmPushButtonWidgetClass, pdown,
                              0, 0);
    setup.eols[1] =
        XtCreateManagedWidget("DOS (CRLF)", xmPushButtonWidgetClass, pdown,
                              0, 0);
    setup.eols[2] =
        XtCreateManagedWidget("Mac (CR)", xmPushButtonWidgetClass, pdown,
                              0, 0);
    XtAddCallback(setup.eols[0], XmNactivateCallback, SetEOLType,
                  (XtPointer) BDF_UNIX_EOL);
    XtAddCallback(setup.eols[1], XmNactivateCallback, SetEOLType,
                  (XtPointer) BDF_DOS_EOL);
    XtAddCallback(setup.eols[2], XmNactivateCallback, SetEOLType,
                  (XtPointer) BDF_MAC_EOL);

    switch (xmbdfed_opts.font_opts.eol) {
      case BDF_UNIX_EOL: XtSetArg(av[0], XmNmenuHistory, setup.eols[0]); break;
      case BDF_DOS_EOL: XtSetArg(av[0], XmNmenuHistory, setup.eols[1]); break;
      case BDF_MAC_EOL: XtSetArg(av[0], XmNmenuHistory, setup.eols[2]); break;
    }
    XtSetValues(opts, av, 1);

    XtManageChild(opts);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_setup_fsize_frame",
                                  xmFrameWidgetClass, setup.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("New Font Size Options", xmLabelWidgetClass,
                                  frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(av[ac], XmNnumColumns, 3); ac++;
    XtSetArg(av[ac], XmNisAligned, True); ac++;
    XtSetArg(av[ac], XmNentryAlignment, XmALIGNMENT_END); ac++;
    rc = XtCreateManagedWidget("xmbdfed_setup_fsize_rowcol",
                               xmRowColumnWidgetClass, frame, av, ac);

    label = XtCreateManagedWidget("Point Size:", xmLabelWidgetClass, rc,
                                  0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    setup.psize =
        XtCreateManagedWidget("xmbdfed_setup_point_size",
                              xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(setup.psize, XmNvalueChangedCallback, EnableUpdate, 0);

    label = XtCreateManagedWidget("Horizontal Resolution:",
                                  xmLabelWidgetClass, rc, 0, 0);
    setup.hres =
        XtCreateManagedWidget("xmbdfed_setup_resolution_x",
                              xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(setup.hres, XmNvalueChangedCallback, EnableUpdate, 0);

    label = XtCreateManagedWidget("Vertical Resolution:",
                                  xmLabelWidgetClass, rc, 0, 0);
    setup.vres =
        XtCreateManagedWidget("xmbdfed_setup_resolution_y",
                              xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(setup.vres, XmNvalueChangedCallback, EnableUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_setup_fspace_frame",
                                  xmFrameWidgetClass, setup.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("New Font Spacing Options",
                                  xmLabelWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    rc = XtCreateManagedWidget("xmbdfed_setup_fspace_rowcol",
                               xmRowColumnWidgetClass, frame, av, ac);

    setup.prop = XtCreateManagedWidget("Proportional",
                                       xmToggleButtonWidgetClass, rc, 0, 0);
    XtAddCallback(setup.prop, XmNvalueChangedCallback, EnableUpdate, 0);

    setup.mono = XtCreateManagedWidget("Monowidth",
                                       xmToggleButtonWidgetClass, rc, 0, 0);
    XtAddCallback(setup.mono, XmNvalueChangedCallback, EnableUpdate, 0);

    setup.cell = XtCreateManagedWidget("Character Cell",
                                       xmToggleButtonWidgetClass, rc, 0, 0);
    XtAddCallback(setup.cell, XmNvalueChangedCallback, EnableUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_setup_fbpp_frame",
                                  xmFrameWidgetClass, setup.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("New Font Bits Per Pixel",
                                  xmLabelWidgetClass, frame, av, ac);

    form = XtCreateManagedWidget("xmbdfed_gedit_fbpp_form",
                                 xmFormWidgetClass, frame, 0, 0);

    s = XmStringCreateSimple("Bits Per Pixel:");
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNlabelString, s); ac++;
    setup.bpps[3] = opts =
        XmCreateOptionMenu(form, "xmbdfed_bits_per_pixel_menu", av, ac);
    XmStringFree(s);

    pdown = XmCreatePulldownMenu(opts, "xmbdfed_bits_per_pixel", 0, 0);

    XtSetArg(av[0], XmNsubMenuId, pdown);
    XtSetValues(opts, av, 1);

    setup.bpps[0] =
        XtCreateManagedWidget("1 bpp", xmPushButtonWidgetClass, pdown, 0, 0);
    setup.bpps[1] =
        XtCreateManagedWidget("2 bpp", xmPushButtonWidgetClass, pdown, 0, 0);
    setup.bpps[2] =
        XtCreateManagedWidget("4 bpp", xmPushButtonWidgetClass, pdown, 0, 0);
    XtAddCallback(setup.bpps[0], XmNactivateCallback, SetBitsPerPixel,
                  (XtPointer) 1);
    XtAddCallback(setup.bpps[1], XmNactivateCallback, SetBitsPerPixel,
                  (XtPointer) 2);
    XtAddCallback(setup.bpps[2], XmNactivateCallback, SetBitsPerPixel,
                  (XtPointer) 4);

    switch (xmbdfed_opts.font_opts.bits_per_pixel) {
      case 1: XtSetArg(av[0], XmNmenuHistory, setup.bpps[0]); break;
      case 2: XtSetArg(av[0], XmNmenuHistory, setup.bpps[1]); break;
      case 4: XtSetArg(av[0], XmNmenuHistory, setup.bpps[2]); break;
    }
    XtSetValues(opts, av, 1);

    XtManageChild(opts);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_setup_button_frame",
                                  xmFrameWidgetClass, setup.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 5); ac++;
    form = XtCreateManagedWidget("xmbdfed_setup_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    setup.update = XtCreateManagedWidget("Update", xmPushButtonWidgetClass,
                                         form, av, ac);
    XtAddCallback(setup.update, XmNactivateCallback, DoUpdate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    setup.save = XtCreateManagedWidget("Save Setup", xmPushButtonWidgetClass,
                                       form, av, ac);
    XtAddCallback(setup.save, XmNactivateCallback, DoSave, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 2); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 3); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    pb = XtCreateManagedWidget("Other Options", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoOtherOptions, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 3); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 4); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    setup.colors = XtCreateManagedWidget("Colors", xmPushButtonWidgetClass,
                                         form, av, ac);
    XtAddCallback(setup.colors, XmNactivateCallback, DoColorDialog,
                  (XtPointer) setup.update);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 4); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 5); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, form, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoCloseSetup,
                  (XtPointer) setup.shell);
}

void
#ifndef _NO_PROTO
DoSetupDialog(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoSetupDialog(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    char nbuf[20];
    Arg av[1];

    if (setup.shell == 0)
      BuildSetupDialog();

    /*
     * Save the current pixel size and bits per pixel so changes can be
     * detected.
     */
    setup.pixsize = xmbdfed_opts.pixel_size;
    setup.eol = xmbdfed_opts.font_opts.eol;
    setup.bpp = xmbdfed_opts.font_opts.bits_per_pixel;

    /*
     * Make sure the EOL option menu has the right value.
     */
    switch (setup.eol) {
      case BDF_UNIX_EOL:
        XtSetArg(av[0], XmNmenuHistory, setup.eols[0]);
        break;
      case BDF_DOS_EOL:
        XtSetArg(av[0], XmNmenuHistory, setup.eols[1]);
        break;
      case BDF_MAC_EOL:
        XtSetArg(av[0], XmNmenuHistory, setup.eols[2]);
        break;
    }
    XtSetValues(setup.eols[3], av, 1);

    /*
     * Make sure the bits per pixel option menu has the right value.
     */
    switch (setup.bpp) {
      case 1: XtSetArg(av[0], XmNmenuHistory, setup.bpps[0]); break;
      case 2: XtSetArg(av[0], XmNmenuHistory, setup.bpps[1]); break;
      case 4: XtSetArg(av[0], XmNmenuHistory, setup.bpps[2]); break;
    }
    XtSetValues(setup.bpps[3], av, 1);

    if (xmbdfed_opts.overwrite_mode)
      XmToggleButtonSetState(setup.overwrite, True, False);
    else
      XmToggleButtonSetState(setup.insert, True, False);

    if (xmbdfed_opts.backups)
      XmToggleButtonSetState(setup.make_backups, True, False);

    if (xmbdfed_opts.font_opts.correct_metrics)
      XmToggleButtonSetState(setup.correct_metrics, True, False);

    if (xmbdfed_opts.font_opts.keep_unencoded)
      XmToggleButtonSetState(setup.keep_unencoded, True, False);

    if (xmbdfed_opts.font_opts.keep_comments)
      XmToggleButtonSetState(setup.keep_comments, True, False);

    if (xmbdfed_opts.font_opts.pad_cells)
      XmToggleButtonSetState(setup.pad_cells, True, False);

    sprintf(nbuf, "%ld", xmbdfed_opts.font_opts.point_size);
    XmTextFieldSetString(setup.psize, nbuf);

    sprintf(nbuf, "%ld", xmbdfed_opts.font_opts.resolution_x);
    XmTextFieldSetString(setup.hres, nbuf);

    sprintf(nbuf, "%ld", xmbdfed_opts.font_opts.resolution_y);
    XmTextFieldSetString(setup.vres, nbuf);

    if (xmbdfed_opts.font_opts.font_spacing == BDF_PROPORTIONAL)
      XmToggleButtonSetState(setup.prop, True, True);
    else if (xmbdfed_opts.font_opts.font_spacing == BDF_MONOWIDTH)
      XmToggleButtonSetState(setup.mono, True, True);
    else
      XmToggleButtonSetState(setup.cell, True, True);

    if (xmbdfed_opts.pixels[0] == ~0 && xmbdfed_opts.pixels[4] == ~0)
      XtSetSensitive(setup.colors, False);
    else
      XtSetSensitive(setup.colors, True);

    if (setup.saved == False)
      XtSetSensitive(setup.save, True);
    else
      XtSetSensitive(setup.save, False);
    XtSetSensitive(setup.update, False);

    /*
     * Set the active editor.
     */
    active_editor = (unsigned long) client_data;

    XtManageChild(setup.form);
    XtPopup(setup.shell, XtGrabNone);
}

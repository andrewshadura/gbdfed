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
static char rcsid[] __attribute__ ((unused)) = "$Id: finfo.c,v 1.6 2001/09/19 21:00:43 mleisher Exp $";
#else
static char rcsid[] = "$Id: finfo.c,v 1.6 2001/09/19 21:00:43 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/TextF.h>
#include <Xm/DialogS.h>
#include "FGrid.h"
#include "xmbdfed.h"

static void
#ifndef _NO_PROTO
EnableUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnableUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];
    XtSetSensitive(ed->finfo.update, True);
}

static void
#ifndef _NO_PROTO
DoSpacingChange(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoSpacingChange(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmToggleButtonCallbackStruct *cb;

    ed = &editors[(unsigned long) client_data];
    cb = (XmToggleButtonCallbackStruct *) call_data;
    if (cb->set) {
        if (w == ed->finfo.prop)
          XtSetSensitive(ed->finfo.dwidth, False);
        else
          XtSetSensitive(ed->finfo.dwidth, True);
        XtSetSensitive(ed->finfo.update, True);
    }
}

static void
#ifndef _NO_PROTO
DoFontInfoUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontInfoUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    char *s;
    XmuttFontGridFontInfoStruct info;

    ed = &editors[(unsigned long) client_data];

    XmuttFontGridGetFontInfo(ed->fgrid, &info);

    s = XmTextFieldGetString(ed->finfo.defchar);
    info.default_char = _bdf_atol(s, 0, 10);
    XtFree(s);

    s = XmTextFieldGetString(ed->finfo.dwidth);
    info.monowidth = (unsigned short) _bdf_atos(s, 0, 10);
    XtFree(s);

    if (XmToggleButtonGetState(ed->finfo.prop) == True)
      info.spacing = BDF_PROPORTIONAL;
    else if (XmToggleButtonGetState(ed->finfo.mono) == True)
      info.spacing = BDF_MONOWIDTH;
    else
      info.spacing = BDF_CHARCELL;

    /*
     * If the spacing changed, update all the glyph editors owned by this font
     * grid with the new spacing and the monowidth field.
     */
    GlyphEditChangeSpacing(ed->id, info.spacing, info.monowidth);

    s = XmTextFieldGetString(ed->finfo.ascent);
    info.font_ascent = _bdf_atol(s, 0, 10);
    XtFree(s);

    s = XmTextFieldGetString(ed->finfo.descent);
    info.font_descent = _bdf_atol(s, 0, 10);
    XtFree(s);

    s = XmTextFieldGetString(ed->finfo.hres);
    info.resolution_x = _bdf_atol(s, 0, 10);
    XtFree(s);

    s = XmTextFieldGetString(ed->finfo.vres);
    info.resolution_y = _bdf_atol(s, 0, 10);
    XtFree(s);

    XmuttFontGridSetFontInfo(ed->fgrid, &info);

    XtSetSensitive(ed->finfo.update, False);
}

static void
#ifndef _NO_PROTO
DoCloseFontInfo(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCloseFontInfo(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (XtIsSensitive(ed->finfo.update)) {
        if (AskBooleanQuestion("Font info has been modified.", "Update",
                               "Do Not Update") == True)
          DoFontInfoUpdate(w, client_data, call_data);
    }
    XtPopdown(ed->finfo.shell);
}

static void
#ifndef _NO_PROTO
BuildFontInfo(MXFEditor *ed)
#else
BuildFontInfo(ed)
MXFEditor *ed;
#endif
{
    Widget pb, frame, form, label, rc;
    Cardinal ac;
    Arg av[10];

    if (ed->file == 0)
      sprintf(title, "%s - Font Info: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Font Info: %s", app_name, ed->file);
 
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_shell", ed->id);
    ed->finfo.shell = XtCreatePopupShell(name, xmDialogShellWidgetClass,
                                         top, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_form", ed->id);
    ed->finfo.form = form =
        XtCreateWidget(name, xmFormWidgetClass, ed->finfo.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_gcount_frame", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Font Glyph Counts", xmLabelWidgetClass,
                                  frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(av[ac], XmNnumColumns, 1); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_gcount_rowcol", ed->id);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, ac);

    label = XtCreateManagedWidget("Encoded:", xmLabelWidgetClass, rc,
                                  0, 0);
    ed->finfo.enc = XtCreateManagedWidget("00", xmLabelWidgetClass, rc,
                                          0, 0);
    label = XtCreateManagedWidget("Unencoded:", xmLabelWidgetClass, rc,
                                  0, 0);
    ed->finfo.unenc = XtCreateManagedWidget("00", xmLabelWidgetClass, rc,
                                            0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_defchar_frame", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Font Default Character", xmLabelWidgetClass,
                                  frame, av, ac);
    ac = 0;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(av[ac], XmNnumColumns, 1); ac++;
    XtSetArg(av[ac], XmNisAligned, True); ac++;
    XtSetArg(av[ac], XmNentryAlignment, XmALIGNMENT_END); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_defchar_rowcol", ed->id);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, ac);

    label = XtCreateManagedWidget("Default Character:", xmLabelWidgetClass, rc,
                                  0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_defchar_char", ed->id);
    ed->finfo.defchar =
        XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(ed->finfo.defchar, XmNvalueChangedCallback,
                  EnableUpdate, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_metrics_frame", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Font Metrics", xmLabelWidgetClass,
                                  frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNnumColumns, 1); ac++;
    XtSetArg(av[ac], XmNisAligned, True); ac++;
    XtSetArg(av[ac], XmNentryAlignment, XmALIGNMENT_END); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_metrics_rowcol", ed->id);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, ac);

    label = XtCreateManagedWidget("Device Width:", xmLabelWidgetClass, rc,
                                  0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_metrics_dwidth", ed->id);
    ed->finfo.dwidth =
        XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(ed->finfo.dwidth, XmNvalueChangedCallback,
                  EnableUpdate, (XtPointer) ed->id);

    label = XtCreateManagedWidget("Font Ascent:",
                                  xmLabelWidgetClass, rc, 0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_metrics_ascent", ed->id);
    ed->finfo.ascent =
        XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(ed->finfo.ascent, XmNvalueChangedCallback,
                  EnableUpdate, (XtPointer) ed->id);

    label = XtCreateManagedWidget("Font Descent:",
                                  xmLabelWidgetClass, rc, 0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_metrics_descent", ed->id);
    ed->finfo.descent =
        XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(ed->finfo.descent, XmNvalueChangedCallback,
                  EnableUpdate, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_resolution_frame", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Font Resolution", xmLabelWidgetClass,
                                  frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNnumColumns, 1); ac++;
    XtSetArg(av[ac], XmNisAligned, True); ac++;
    XtSetArg(av[ac], XmNentryAlignment, XmALIGNMENT_END); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_resolution_rowcol", ed->id);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, ac);

    label = XtCreateManagedWidget("Horizontal:",
                                  xmLabelWidgetClass, rc, 0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_resolution_horizontal", ed->id);
    ed->finfo.hres =
        XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(ed->finfo.hres, XmNvalueChangedCallback,
                  EnableUpdate, (XtPointer) ed->id);

    label = XtCreateManagedWidget("Vertical:",
                                  xmLabelWidgetClass, rc, 0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNcolumns, 6); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_resolution_vertical", ed->id);
    ed->finfo.vres =
        XtCreateManagedWidget(name, xmTextFieldWidgetClass, rc, av, ac);
    XtAddCallback(ed->finfo.vres, XmNvalueChangedCallback,
                  EnableUpdate, (XtPointer) ed->id);

    ed->finfo.bpp = XtCreateManagedWidget("Bits Per Pixel: 0",
                                          xmLabelWidgetClass, rc, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_spacing_frame", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Font Spacing", xmLabelWidgetClass,
                                  frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_spacing_rowcol", ed->id);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, ac);

    ed->finfo.prop = XtCreateManagedWidget("Proportional",
                                           xmToggleButtonWidgetClass, rc,
                                           0, 0);
    XtAddCallback(ed->finfo.prop, XmNvalueChangedCallback, DoSpacingChange,
                  (XtPointer) ed->id);

    ed->finfo.mono = XtCreateManagedWidget("Monowidth",
                                           xmToggleButtonWidgetClass, rc,
                                           0, 0);
    XtAddCallback(ed->finfo.mono, XmNvalueChangedCallback, DoSpacingChange,
                  (XtPointer) ed->id);

    ed->finfo.cell = XtCreateManagedWidget("Character Cell",
                                           xmToggleButtonWidgetClass, rc,
                                           0, 0);
    XtAddCallback(ed->finfo.cell, XmNvalueChangedCallback, DoSpacingChange,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_button_frame", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 5); ac++;
    sprintf(name, "xmbdfed_fontinfo%ld_button_form", ed->id);
    form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    ed->finfo.update = XtCreateManagedWidget("Update",
                                             xmPushButtonWidgetClass, form,
                                             av, ac);
    XtAddCallback(ed->finfo.update, XmNactivateCallback,
                  DoFontInfoUpdate, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 3); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 4); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    pb = XtCreateManagedWidget("Done", xmPushButtonWidgetClass, form, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoCloseFontInfo,
                  (XtPointer) ed->id);
}

void
#ifndef _NO_PROTO
DoFontInfo(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontInfo(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XmString s;
    Arg av[1];
    char nbuf[25];
    XmuttFontGridFontInfoStruct info;
    XmuttFontGridPageInfoStruct pinfo;

    ed = &editors[(unsigned long) client_data];

    if (ed->finfo.shell == 0)
      BuildFontInfo(ed);

    /*
     * Get the page info from the grid.
     */
    XmuttFontGridPageInfo(ed->fgrid, &pinfo);

    /*
     * Set the encoded and unencoded glyphs counts.
     */
    sprintf(nbuf, "%ld", pinfo.num_encoded);
    s = XmStringCreateSimple(nbuf);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ed->finfo.enc, av, 1);
    XmStringFree(s);

    sprintf(nbuf, "%ld", pinfo.num_unencoded);
    s = XmStringCreateSimple(nbuf);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ed->finfo.unenc, av, 1);
    XmStringFree(s);

    /*
     * Get the font info from the grid.
     */
    XmuttFontGridGetFontInfo(ed->fgrid, &info);

    /*
     * Set the default character.
     */
    sprintf(nbuf, "%ld", info.default_char);
    XmTextFieldSetString(ed->finfo.defchar, nbuf);

    /*
     * Set the device width.
     */
    sprintf(nbuf, "%hd", info.monowidth);
    XmTextFieldSetString(ed->finfo.dwidth, nbuf);

    /*
     * Set the font ascent and descent.
     */
    sprintf(nbuf, "%ld", info.font_ascent);
    XmTextFieldSetString(ed->finfo.ascent, nbuf);
    sprintf(nbuf, "%ld", info.font_descent);
    XmTextFieldSetString(ed->finfo.descent, nbuf);

    /*
     * Set the horizontal and vertical resolutions.
     */
    sprintf(nbuf, "%ld", info.resolution_x);
    XmTextFieldSetString(ed->finfo.hres, nbuf);
    sprintf(nbuf, "%ld", info.resolution_y);
    XmTextFieldSetString(ed->finfo.vres, nbuf);

    /*
     * Set the bits per pixel.
     */
    sprintf(name, "Bits Per Pixel: %ld", info.bits_per_pixel);
    s = XmStringCreateSimple(name);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ed->finfo.bpp, av, 1);
    XmStringFree(s);

    /*
     * Set the font spacing type.
     */
    switch (info.spacing) {
      case BDF_PROPORTIONAL:
        XmToggleButtonSetState(ed->finfo.prop, True, False);
        XtSetSensitive(ed->finfo.dwidth, False);
        break;
      case BDF_MONOWIDTH:
        XmToggleButtonSetState(ed->finfo.mono, True, False);
        XtSetSensitive(ed->finfo.dwidth, True);
        break;
      case BDF_CHARCELL:
        XmToggleButtonSetState(ed->finfo.cell, True, False);
        XtSetSensitive(ed->finfo.dwidth, True);
        break;
    }

    /*
     * Disable the update button initially.
     */
    XtSetSensitive(ed->finfo.update, False);

    /*
     * Make sure the title matches the font name.
     */
    if (ed->file == 0)
      sprintf(title, "%s - Font Info: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Font Info: %s", app_name, ed->file);

    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ed->finfo.shell, av, 1);

    XtManageChild(ed->finfo.form);
    XtPopup(ed->finfo.shell, XtGrabNone);
}

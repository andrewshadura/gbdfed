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
static char rcsid[] __attribute__ ((unused)) = "$Id: ops.c,v 1.6 2004/01/29 17:15:38 mleisher Exp $";
#else
static char rcsid[] = "$Id: ops.c,v 1.6 2004/01/29 17:15:38 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include "FGrid.h"
#include "GTest.h"
#include "xmbdfed.h"

typedef struct {
    Widget shell;
    Widget form;
    Widget horiz;
    Widget vert;
    Widget sel;
    Widget all;
    XtCallbackProc cback;
    XtPointer data;
} MXFEditorTranslateOp;

typedef struct {
    int reason;
    Widget shell;
    Widget form;
    Widget label;
    Widget angle;
    Widget sel;
    Widget all;
    Widget ok;
    XtCallbackProc cback;
    XtPointer data;
} MXFEditorOtherOp;

typedef struct {
    int reason;
    Widget shell;
    Widget form;
    Widget sel;
    Widget all;
    Widget ok;
    XtCallbackProc cback;
    XtPointer data;
} MXFEditorEmboldenOp;

typedef struct {
    MXFEditorTranslateOp trans;
    MXFEditorOtherOp other;
    MXFEditorEmboldenOp embold;
} MXFEditorOperations;

static MXFEditorOperations ops;

static void
#ifndef _NO_PROTO
CancelTranslate(Widget w, XtPointer client_data, XtPointer call_data)
#else
CancelTranslate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ops.trans.cback = 0;
    ops.trans.data = 0;
    XtPopdown(ops.trans.shell);
}

static void
#ifndef _NO_PROTO
CancelOther(Widget w, XtPointer client_data, XtPointer call_data)
#else
CancelOther(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ops.other.cback = 0;
    ops.other.data = 0;
    XtPopdown(ops.other.shell);
}

static void
#ifndef _NO_PROTO
CancelEmbolden(Widget w, XtPointer client_data, XtPointer call_data)
#else
CancelEmbolden(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ops.embold.cback = 0;
    ops.embold.data = 0;
    XtPopdown(ops.embold.shell);
}

static void
#ifndef _NO_PROTO
ActivateTranslate(Widget w, XtPointer client_data, XtPointer call_data)
#else
ActivateTranslate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    char *s;
    XmuttOperationCallbackStruct cb;

    if (ops.trans.cback != 0) {
        cb.reason = XMBDFED_TRANSLATE;
        s = XmTextFieldGetString(ops.trans.horiz);
        cb.dx = _bdf_atos(s, 0, 10);
        XtFree(s);
        s = XmTextFieldGetString(ops.trans.vert);
        cb.dy = _bdf_atos(s, 0, 10);
        XtFree(s);
        cb.all = XmToggleButtonGetState(ops.trans.all);
        (*ops.trans.cback)(w, ops.trans.data, (XtPointer) &cb);
    }
    XtPopdown(ops.trans.shell);
}

static void
#ifndef _NO_PROTO
ActivateOther(Widget w, XtPointer client_data, XtPointer call_data)
#else
ActivateOther(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    char *s;
    XmuttOperationCallbackStruct cb;

    if (ops.other.cback != 0) {
        cb.reason = ops.other.reason;
        s = XmTextFieldGetString(ops.other.angle);
        cb.angle = _bdf_atos(s, 0, 10);
        XtFree(s);
        cb.all = XmToggleButtonGetState(ops.other.all);
        (*ops.other.cback)(w, ops.other.data, (XtPointer) &cb);
    }
    XtPopdown(ops.other.shell);
}

static void
#ifndef _NO_PROTO
ActivateEmbolden(Widget w, XtPointer client_data, XtPointer call_data)
#else
ActivateEmbolden(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XmuttOperationCallbackStruct cb;

    if (ops.embold.cback != 0) {
        cb.reason = ops.embold.reason;
        cb.all = XmToggleButtonGetState(ops.embold.all);
        (*ops.embold.cback)(w, ops.embold.data, (XtPointer) &cb);
    }
    XtPopdown(ops.embold.shell);
}

static void
#ifndef _NO_PROTO
BuildTransDialog(void)
#else
BuildTransDialog()
#endif
{
    Widget label1, label2, frame, form, rc, pb;
    Dimension ht, wd;
    Cardinal ac;
    Arg av[8];

    sprintf(title, "%s - Translate Glyphs", app_name);
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    ops.trans.shell = XtCreatePopupShell("xmbdfed_translate_shell",
                                         topLevelShellWidgetClass, top,
                                         av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    ops.trans.form = XtCreateWidget("xmbdfed_translate_form",
                                    xmFormWidgetClass, ops.trans.shell,
                                    av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_translate_frame",
                                  xmFrameWidgetClass, ops.trans.form,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label1 = XtCreateManagedWidget("Translate Glyphs", xmLabelWidgetClass,
                                   frame, av, ac);

    form = XtCreateManagedWidget("xmbdfed_trans_field_form", xmFormWidgetClass,
                                 frame, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_END); ac++;
    label1 = XtCreateManagedWidget("DX:", xmLabelWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label1); ac++;
    ops.trans.horiz = XtCreateManagedWidget("xmbdfed_trans_horiz",
                                            xmTextFieldWidgetClass, form,
                                            av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, label1); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_END); ac++;
    label2 = XtCreateManagedWidget("DY:", xmLabelWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, ops.trans.horiz); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label2); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    ops.trans.vert = XtCreateManagedWidget("xmbdfed_trans_vert",
                                           xmTextFieldWidgetClass, form,
                                           av, ac);

    /*
     * Adjust the height of the labels so they center vertically to match
     * the text fields.
     */
    XtSetArg(av[0], XmNheight, &ht);
    XtGetValues(ops.trans.vert, av, 1);
    XtSetArg(av[0], XmNheight, ht);
    XtSetValues(label1, av, 1);
    XtSetValues(label2, av, 1);

    /*
     * Adjust the width of the second label to match the first.
     */
    XtSetArg(av[0], XmNwidth, &wd);
    XtGetValues(label1, av, 1);
    XtSetArg(av[0], XmNwidth, wd);
    XtSetValues(label2, av, 1);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    rc = XtCreateManagedWidget("xmbdfed_trans_rowcol", xmRowColumnWidgetClass,
                               ops.trans.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNset, True); ac++;
    ops.trans.sel = XtCreateManagedWidget("Selected Glyphs",
                                          xmToggleButtonWidgetClass, rc,
                                          av, ac);

    ops.trans.all = XtCreateManagedWidget("All Glyphs",
                                          xmToggleButtonWidgetClass, rc, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, rc); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_trans_button_frame",
                                  xmFrameWidgetClass, ops.trans.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 2); ac++;
    form = XtCreateManagedWidget("xmbdfed_trans_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    pb = XtCreateManagedWidget("Translate", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, ActivateTranslate, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    pb = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, CancelTranslate, 0);
}

static void
#ifndef _NO_PROTO
BuildOtherDialog(void)
#else
BuildOtherDialog()
#endif
{
    Widget label, frame, form, rc, pb;
    Cardinal ac;
    Arg av[8];

    sprintf(title, "%s - Rotate Glyphs", app_name);
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    ops.other.shell = XtCreatePopupShell("xmbdfed_otherops_shell",
                                         topLevelShellWidgetClass, top,
                                         av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    ops.other.form = XtCreateWidget("xmbdfed_otherops_form",
                                    xmFormWidgetClass, ops.other.shell,
                                    av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_otherops_frame",
                                  xmFrameWidgetClass, ops.other.form,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    ops.other.label = XtCreateManagedWidget("Rotate Glyphs",
                                            xmLabelWidgetClass,
                                            frame, av, ac);

    form = XtCreateManagedWidget("xmbdfed_otherops_field_form",
                                 xmFormWidgetClass, frame, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    label = XtCreateManagedWidget("Degrees:", xmLabelWidgetClass, form,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label); ac++;
    ops.other.angle = XtCreateManagedWidget("xmbdfed_otherops_angle",
                                            xmTextFieldWidgetClass, form,
                                            av, ac);
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    rc = XtCreateManagedWidget("xmbdfed_otherops_rowcol",
                               xmRowColumnWidgetClass,
                               ops.other.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNset, True); ac++;
    ops.other.sel = XtCreateManagedWidget("Selected Glyphs",
                                          xmToggleButtonWidgetClass, rc,
                                          av, ac);

    ops.other.all = XtCreateManagedWidget("All Glyphs",
                                          xmToggleButtonWidgetClass, rc, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, rc); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_otherops_button_frame",
                                  xmFrameWidgetClass, ops.other.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 2); ac++;
    form = XtCreateManagedWidget("xmbdfed_otherops_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    ops.other.ok = XtCreateManagedWidget("Rotate", xmPushButtonWidgetClass,
                                         form, av, ac);
    XtAddCallback(ops.other.ok, XmNactivateCallback, ActivateOther, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    pb = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, CancelOther, 0);
}

static void
#ifndef _NO_PROTO
BuildEmboldenDialog(void)
#else
BuildEmboldenDialog()
#endif
{
    Widget label, frame, form, rc, pb;
    Cardinal ac;
    Arg av[8];

    sprintf(title, "%s - Embolden Glyphs", app_name);
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    ops.embold.shell = XtCreatePopupShell("xmbdfed_emboldops_shell",
                                          topLevelShellWidgetClass, top,
                                          av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    ops.embold.form = XtCreateWidget("xmbdfed_emboldops_form",
                                     xmFormWidgetClass, ops.embold.shell,
                                     av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_emboldops_frame",
                                  xmFrameWidgetClass, ops.embold.form,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Embolden Glyphs",
                                  xmLabelWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    rc = XtCreateManagedWidget("xmbdfed_emboldops_rowcol",
                               xmRowColumnWidgetClass,
                               frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNset, True); ac++;
    ops.embold.sel = XtCreateManagedWidget("Selected Glyphs",
                                           xmToggleButtonWidgetClass, rc,
                                           av, ac);

    ops.embold.all = XtCreateManagedWidget("All Glyphs",
                                           xmToggleButtonWidgetClass, rc,
                                           0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_emboldops_button_frame",
                                  xmFrameWidgetClass, ops.embold.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 2); ac++;
    form = XtCreateManagedWidget("xmbdfed_emboldops_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    ops.embold.ok = XtCreateManagedWidget("Embolden", xmPushButtonWidgetClass,
                                          form, av, ac);
    XtAddCallback(ops.embold.ok, XmNactivateCallback, ActivateEmbolden, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    pb = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, CancelEmbolden, 0);
}

void
#ifndef _NO_PROTO
ShowTranslateDialog(XtCallbackProc cback, XtPointer data,
                    Boolean toggles_sensitive)
#else
ShowTranslateDialog(cback, data, toggles_sensitive)
XtCallbackProc cback;
XtPointer data;
Boolean toggles_sensitive;
#endif
{
    if (ops.trans.shell == 0)
      BuildTransDialog();

    XtSetSensitive(ops.trans.sel, toggles_sensitive);
    XtSetSensitive(ops.trans.all, toggles_sensitive);

    ops.trans.cback = cback;
    ops.trans.data = data;
    XtManageChild(ops.trans.form);
    XtPopup(ops.trans.shell, XtGrabNone);
}

void
#ifndef _NO_PROTO
ShowRotateDialog(XtCallbackProc cback, XtPointer data,
                 Boolean toggles_sensitive)
#else
ShowRotateDialog(cback, data, toggles_sensitive)
XtCallbackProc cback;
XtPointer data;
Boolean toggles_sensitive;
#endif
{
    XmString s;
    Arg av[1];

    if (ops.other.shell == 0)
      BuildOtherDialog();

    ops.other.reason = XMBDFED_ROTATE;

    /*
     * Set the title and the labels.
     */
    sprintf(title, "%s - Rotate Glyphs", app_name);
    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ops.other.shell, av, 1);

    s = XmStringCreateSimple("Rotate Glyphs");
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ops.other.label, av, 1);
    XmStringFree(s);

    s = XmStringCreateSimple("Rotate");
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ops.other.ok, av, 1);
    XmStringFree(s);

    /*
     * Set the sensitivity of the toggles.
     */
    XtSetSensitive(ops.other.sel, toggles_sensitive);
    XtSetSensitive(ops.other.all, toggles_sensitive);

    ops.other.cback = cback;
    ops.other.data = data;
    XtManageChild(ops.other.form);
    XtPopup(ops.other.shell, XtGrabNone);
}

void
#ifndef _NO_PROTO
ShowShearDialog(XtCallbackProc cback, XtPointer data,
                Boolean toggles_sensitive)
#else
ShowShearDialog(cback, data, toggles_sensitive)
XtCallbackProc cback;
XtPointer data;
Boolean toggles_sensitive;
#endif
{
    XmString s;
    Arg av[1];

    if (ops.other.shell == 0)
      BuildOtherDialog();

    ops.other.reason = XMBDFED_SHEAR;

    /*
     * Set the title and the labels.
     */
    sprintf(title, "%s - Shear Glyphs", app_name);
    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ops.other.shell, av, 1);

    s = XmStringCreateSimple("Shear Glyphs");
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ops.other.label, av, 1);
    XmStringFree(s);

    s = XmStringCreateSimple("Shear");
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ops.other.ok, av, 1);
    XmStringFree(s);

    /*
     * Set the sensitivity of the toggles.
     */
    XtSetSensitive(ops.other.sel, toggles_sensitive);
    XtSetSensitive(ops.other.all, toggles_sensitive);

    ops.other.cback = cback;
    ops.other.data = data;
    XtManageChild(ops.other.form);
    XtPopup(ops.other.shell, XtGrabNone);
}

void
#ifndef _NO_PROTO
ShowEmboldenDialog(XtCallbackProc cback, XtPointer data,
                   Boolean toggles_sensitive)
#else
ShowEmboldenDialog(cback, data, toggles_sensitive)
XtCallbackProc cback;
XtPointer data;
Boolean toggles_sensitive;
#endif
{
    if (ops.other.shell == 0)
      BuildEmboldenDialog();

    ops.embold.reason = XMBDFED_EMBOLDEN;

    /*
     * Set the sensitivity of the toggles.
     */
    XtSetSensitive(ops.embold.sel, toggles_sensitive);
    XtSetSensitive(ops.embold.all, toggles_sensitive);

    ops.embold.cback = cback;
    ops.embold.data = data;
    XtManageChild(ops.embold.form);
    XtPopup(ops.embold.shell, XtGrabNone);
}

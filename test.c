/*
 * Copyright 2000 Computing Research Labs, New Mexico State University
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
static char rcsid[] __attribute__ ((unused)) = "$Id: test.c,v 1.4 2000/03/16 20:08:55 mleisher Exp $";
#else
static char rcsid[] = "$Id: test.c,v 1.4 2000/03/16 20:08:55 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/TextF.h>
#include <Xm/List.h>
#include <Xm/DialogS.h>
#include "FGrid.h"
#include "xmbdfed.h"
#include "GTest.h"

typedef struct {
    Widget shell;
    Widget form;
    Widget gtest;
    Widget clear;
    Widget baseline;
    Boolean visible;
} MXFEditorTest;

static MXFEditorTest gtest;

static void
#ifndef _NO_PROTO
ClearGlyphTest(Widget w, XtPointer client_data, XtPointer call_data)
#else
ClearGlyphTest(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XmuttGlyphTestErase(gtest.gtest);

    XtSetSensitive(gtest.clear, False);
}

static void
#ifndef _NO_PROTO
ToggleBaseline(Widget w, XtPointer client_data, XtPointer call_data)
#else
ToggleBaseline(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XmToggleButtonCallbackStruct *cb;
    Arg av[1];

    cb = (XmToggleButtonCallbackStruct *) call_data;

    XtSetArg(av[0], XmNshowBaseline, cb->set);
    XtSetValues(gtest.gtest, av, 1);
}

static void
#ifndef _NO_PROTO
ToggleDirection(Widget w, XtPointer client_data, XtPointer call_data)
#else
ToggleDirection(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XmToggleButtonCallbackStruct *cb;
    Arg av[1];

    cb = (XmToggleButtonCallbackStruct *) call_data;

    if (cb->set) {
        XtSetArg(av[0], XmNdirection, XmuttGlyphTestRightToLeft);
    } else {
        XtSetArg(av[0], XmNdirection, XmuttGlyphTestLeftToRight);
    }
    XtSetValues(gtest.gtest, av, 1);
}

static void
#ifndef _NO_PROTO
EnableClear(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnableClear(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XtSetSensitive(gtest.clear, True);
}

static void
#ifndef _NO_PROTO
CloseGlyphTest(Widget w, XtPointer client_data, XtPointer call_data)
#else
CloseGlyphTest(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    gtest.visible = False;
    XtPopdown(gtest.shell);
}

static void
#ifndef _NO_PROTO
BuildTestDialog(void)
#else
BuildTestDialog()
#endif
{
    Widget pb, form, frame, label, rc;
    Cardinal ac;
    Arg av[10];

    sprintf(title, "%s - Test Glyphs", app_name);

    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    gtest.shell = XtCreatePopupShell("xmbdfed_glyphtest_shell",
                                     topLevelShellWidgetClass, top,
                                     av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 4); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    gtest.form = XtCreateWidget("xmbdfed_glyphtest_manager", xmFormWidgetClass,
                                gtest.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_glyphtest_frame",
                                  xmFrameWidgetClass, gtest.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Test Glyphs", xmLabelWidgetClass, frame,
                                  av, ac);

    form = XtCreateManagedWidget("xmbdfed_glyphtest_form", xmFormWidgetClass,
                                 frame, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNcolorList, xmbdfed_opts.pixels); ac++;
    gtest.gtest = XtCreateManagedWidget("xmbdfed_glyphtest_test",
                                        xmuttGlyphTestWidgetClass,
                                        form, av, ac);
    XtAddCallback(gtest.gtest, XmNaddGlyphCallback, EnableClear, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    rc = XtCreateManagedWidget("xmbdfed_glyphtest_rowcol",
                               xmRowColumnWidgetClass, gtest.form, av, ac);

    XtSetArg(av[0], XmNset, True);
    pb = XtCreateManagedWidget("Show Baseline", xmToggleButtonWidgetClass,
                               rc, av, 1);
    XtAddCallback(pb, XmNvalueChangedCallback, ToggleBaseline, 0);
    pb = XtCreateManagedWidget("Draw Right To Left", xmToggleButtonWidgetClass,
                               rc, 0, 0);
    XtAddCallback(pb, XmNvalueChangedCallback, ToggleDirection, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, rc); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_glyphtest_buttonframe",
                                  xmFrameWidgetClass, gtest.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 5); ac++;
    form = XtCreateManagedWidget("xmbdfed_glyphtest_buttonform",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    XtSetArg(av[ac], XmNsensitive, False); ac++;
    gtest.clear = XtCreateManagedWidget("Clear", xmPushButtonWidgetClass,
                                        form, av, ac);
    XtAddCallback(gtest.clear, XmNactivateCallback, ClearGlyphTest, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 3); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 4); ac++;
    pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, form, av, ac);
    XtAddCallback(pb, XmNactivateCallback, CloseGlyphTest, 0);
}

void
#ifndef _NO_PROTO
DoGlyphTest(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoGlyphTest(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    /*
     * If the glyph test dialog is already up, pop it down.
     */
    if (gtest.visible == True) {
        gtest.visible = False;
        XtPopdown(gtest.shell);
        return;
    }

    if (gtest.shell == 0)
      BuildTestDialog();

    /*
     * Indicate that the glyph test dialog is active.  This is used elsewhere
     * to keep the font grid in synch with the glyph tester.
     */
    gtest.visible = True;

    XtManageChild(gtest.form);
    XtPopup(gtest.shell, XtGrabNone);
}

void
#ifndef _NO_PROTO
GlyphTestRedisplay(void)
#else
GlyphTestRedisplay()
#endif
{
    if (gtest.visible == True)
      XmuttGlyphTestRedisplay(gtest.gtest);
}

void
#ifndef _NO_PROTO
GlyphTestAddGlyph(bdf_font_t *font, bdf_glyph_t *glyph)
#else
GlyphTestAddGlyph(font, glyph)
bdf_font_t *font;
bdf_glyph_t *glyph;
#endif
{
    if (gtest.visible == True)
      XmuttGlyphTestAddGlyph(gtest.gtest, font, glyph);
}

void
#ifndef _NO_PROTO
GlyphTestReset(bdf_font_t *font)
#else
GlyphTestReset(font)
bdf_font_t *font;
#endif
{
    if (gtest.visible == True)
      XmuttGlyphTestRemoveFont(gtest.gtest, font);
}

void
#ifndef _NO_PROTO
GlyphTestResize(void)
#else
GlyphTestResize()
#endif
{
    if (gtest.visible == True)
      XmuttGlyphTestResize(gtest.gtest);
}

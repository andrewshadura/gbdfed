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
static char rcsid[] __attribute__ ((unused)) = "$Id: color.c,v 1.6 2001/09/19 21:00:43 mleisher Exp $";
#else
static char rcsid[] = "$Id: color.c,v 1.6 2001/09/19 21:00:43 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/Scale.h>
#include "bdfP.h"
#include "xmbdfed.h"

typedef struct {
    Widget shell;
    Widget form;
    Widget other;
    Widget reset;
    Widget close;
    Widget update;
    Widget cols[16];
    Widget slide[16];
    unsigned short colvals[20];
    int uws;
    int bpp;
    int switching;
    int modified[2];
} MXFEditorColors;

static MXFEditorColors colors;

static void
#ifndef _NO_PROTO
SwitchColorTables(Widget w, XtPointer client_data, XtPointer call_data)
#else
SwitchColorTables(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int i, val, off;
    XmString s;
    Arg av[1];

    off = (colors.bpp == 2) ? 4 : 0;

    /*
     * Set the value of the first four sliders.  Indicate that switching is
     * taking place so the Update button will not be activated at the wrong
     * time.
     */
    colors.switching = 1;
    for (i = 0; i < 4; i++) {
        val = (int) (((float) colors.colvals[i + off] / 65535.0) * 100.0);
        XtSetArg(av[0], XmNvalue, val);
        XtSetValues(colors.slide[i], av, 1);
    }
    colors.switching = 0;

    if (colors.bpp == 2) {
        /*
         * Make sure the colors are allocated.
         */
        xmbdfedAllocateColors(w, 4);

        s = XmStringCreateSimple("2bpp Colors");
        XtSetArg(av[0], XmNlabelString, s);
        XtSetValues(colors.other, av, 1);
        XmStringFree(s);

        /*
         * Set all the background pixels in case they have not been set yet
         * and manage the widgets at the same time.
         */
        for (i = 0; i < 16; i++) {
            XtSetArg(av[0], XmNbackground, xmbdfed_opts.pixels[i + 4]);
            XtSetValues(colors.cols[i], av, 1);
            if (i > 3) {
                XtManageChild(colors.cols[i]);
                XtManageChild(colors.slide[i]);
            }
        }
        colors.bpp = 4;

        /*
         * Set the Reset button.
         */
        if (colors.modified[1])
          XtSetSensitive(colors.reset, True);
        else
          XtSetSensitive(colors.reset, False);
    } else {
        /*
         * Make sure the colors are allocated.
         */
        xmbdfedAllocateColors(w, 2);

        s = XmStringCreateSimple("4bpp Colors");
        XtSetArg(av[0], XmNlabelString, s);
        XtSetValues(colors.other, av, 1);
        XmStringFree(s);

        /*
         * Set the first 4 background pixels back to the 2bpp colors and
         * unmanage the extra color adjusters.
         */
        for (i = 0; i < 16; i++) {
            if (i > 3) {
                XtUnmanageChild(colors.cols[i]);
                XtUnmanageChild(colors.slide[i]);
            } else {
                XtSetArg(av[0], XmNbackground, xmbdfed_opts.pixels[i]);
                XtSetValues(colors.cols[i], av, 1);
            }
        }
        colors.bpp = 2;

        /*
         * Set the Reset button.
         */
        if (colors.modified[0])
          XtSetSensitive(colors.reset, True);
        else
          XtSetSensitive(colors.reset, False);
    }
}

static void
#ifndef _NO_PROTO
DoColorReset(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoColorReset(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int i, lo, hi, off, val;
    Colormap cmap;
    XColor col;
    Arg av[1];

    cmap = XDefaultColormap(XtDisplay(colors.shell),
                            XDefaultScreen(XtDisplay(colors.shell)));
    col.flags = DoRed|DoGreen|DoBlue;

    if (colors.bpp == 2) {
        lo = off = 0;
        hi = 4;
    } else {
        lo = off = 4;
        hi = 20;
    }

    for (i = lo; i < hi; i++) {
        if (colors.colvals[i] != xmbdfed_opts.colors[i]) {
            col.red = col.green = col.blue = colors.colvals[i] =
                xmbdfed_opts.colors[i];
            col.pixel = xmbdfed_opts.pixels[i];
            XStoreColor(XtDisplay(colors.shell), cmap, &col);

            /*
             * Reset the slider.
             */
            val = (int) (((float) xmbdfed_opts.colors[i] / 65535.0) *
                         100.0);
            XtSetArg(av[0], XmNvalue, val);
            colors.switching = 1;
            XtSetValues(colors.slide[i - off], av, 1);
            colors.switching = 0;
        }
    }
    colors.modified[(colors.bpp >> 1) - 1] = 0;
    XtSetSensitive(colors.reset, False);

    /*
     * If the Update button was not sensitive when color editing was started
     * and this reset put everything back to normal, then desensitize the
     * Update button.
     */
    if (colors.uws == 0 && !(colors.modified[0] + colors.modified[1]))
      XtSetSensitive(colors.update, False);
}

static void
#ifndef _NO_PROTO
AdjustColor(Widget w, XtPointer client_data, XtPointer call_data)
#else
AdjustColor(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int which;
    float val;
    XmScaleCallbackStruct *cb;
    Colormap cmap;
    XColor col;

    /*
     * If the change is coming as a result of switching the color tables, just
     * ignore the callback.
     */

    cb = (XmScaleCallbackStruct *) call_data;
    which = (int) client_data;

    cmap = XDefaultColormap(XtDisplay(w),
                            XDefaultScreen(XtDisplay(w)));

    if (colors.bpp == 4)
      which += 4;

    val = ((float) cb->value) / 100.0;
    colors.colvals[which] = (unsigned short) ((65535.0 * val) + 0.05);
    col.red = col.green = col.blue = colors.colvals[which];
    col.flags = DoRed|DoGreen|DoBlue;
    col.pixel = xmbdfed_opts.pixels[which];
    XStoreColor(XtDisplay(w), cmap, &col);

    /*
     * Refresh the background color on the color patch.
     */
    if (colors.bpp == 4)
      which -= 4;
    XClearWindow(XtDisplay(w), XtWindow(colors.cols[which]));

    /*
     * Mark the color array as being modified for the relevant appropriate
     * bits per pixel.
     */
    colors.modified[(colors.bpp >> 1) - 1] = 1;

    /*
     * Turn on the reset button for these colors.
     */
    XtSetSensitive(colors.reset, True);

    /*
     * Enable the setup dialog's Update button.
     */
    XtSetSensitive(colors.update, True);
}

static void
#ifndef _NO_PROTO
BuildColorDialog(void)
#else
BuildColorDialog()
#endif
{
    int val;
    Widget frame, rc, pb, form;
    Cardinal ac, i;
    Arg av[16];

    colors.bpp = 4;

    sprintf(title, "%s - Color Setting", app_name);
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    colors.shell = XtCreatePopupShell("xmbdfed_colors_shell",
                                      xmDialogShellWidgetClass, top, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    colors.form = XtCreateWidget("xmbdfed_colors_form", xmFormWidgetClass,
                                 colors.shell, av, ac);

    /*
     * Create the button frame first.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_colors_button_frame",
                                  xmFrameWidgetClass, colors.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 3); ac++;
    form = XtCreateManagedWidget("xmbdfed_colors_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    colors.other = XtCreateManagedWidget("2bpp Colors",
                                          xmPushButtonWidgetClass,
                                          form, av, ac);
    XtAddCallback(colors.other, XmNactivateCallback, SwitchColorTables, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    colors.reset = XtCreateManagedWidget("Reset",
                                          xmPushButtonWidgetClass,
                                          form, av, ac);
    XtAddCallback(colors.reset, XmNactivateCallback, DoColorReset, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 2); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 3); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, form, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoClose, (XtPointer) colors.shell);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 4); ac++;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
    rc = XtCreateManagedWidget("xmbdfed_colors_form", xmFormWidgetClass,
                               colors.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftOffset, 2); ac++;
    XtSetArg(av[ac], XmNwidth, 22); ac++;
    XtSetArg(av[ac], XmNheight, 50); ac++;
    XtSetArg(av[ac], XmNborderWidth, 0); ac++;
    XtSetArg(av[ac], XmNshadowThickness, 0); ac++;
    XtSetArg(av[ac], XmNhighlightThickness, 0); ac++;
    XtSetArg(av[ac], XmNbackground, xmbdfed_opts.pixels[4]); ac++;
    colors.cols[0] = XtCreateManagedWidget("xmbdfed_color0",
                                           xmPrimitiveWidgetClass, rc,
                                           av, ac);

    val = (int) (((float) xmbdfed_opts.colors[4] / 65535.0) * 100.0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, colors.cols[0]); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftOffset, 2); ac++;
    XtSetArg(av[ac], XmNwidth, 22); ac++;
    XtSetArg(av[ac], XmNshowValue, False); ac++;
    XtSetArg(av[ac], XmNminimum, 0); ac++;
    XtSetArg(av[ac], XmNmaximum, 100); ac++;
    XtSetArg(av[ac], XmNprocessingDirection, XmMAX_ON_BOTTOM); ac++;
    XtSetArg(av[ac], XmNscaleMultiple, 1); ac++;
    XtSetArg(av[ac], XmNvalue, val); ac++;
    colors.slide[0] = XtCreateManagedWidget("", xmScaleWidgetClass, rc,
                                            av, ac);
    XtAddCallback(colors.slide[0], XmNvalueChangedCallback, AdjustColor,
                  (XtPointer) 0);
    XtAddCallback(colors.slide[0], XmNdragCallback, AdjustColor,
                  (XtPointer) 0);

    for (i = 1; i < 16; i++) {
        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, colors.cols[i - 1]); ac++;
        XtSetArg(av[ac], XmNwidth, 22); ac++;
        XtSetArg(av[ac], XmNheight, 50); ac++;
        XtSetArg(av[ac], XmNborderWidth, 0); ac++;
        XtSetArg(av[ac], XmNshadowThickness, 0); ac++;
        XtSetArg(av[ac], XmNhighlightThickness, 0); ac++;
        XtSetArg(av[ac], XmNbackground, xmbdfed_opts.pixels[i + 4]); ac++;
        sprintf(name, "xmbdfed_color%d", i);
        colors.cols[i] = XtCreateManagedWidget(name, xmPrimitiveWidgetClass,
                                               rc, av, ac);

        val = (int) (((float) xmbdfed_opts.colors[i + 4] / 65535.0) * 100.0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, colors.cols[i]); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, colors.cols[i - 1]); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNwidth, 22); ac++;
        XtSetArg(av[ac], XmNshowValue, False); ac++;
        XtSetArg(av[ac], XmNminimum, 0); ac++;
        XtSetArg(av[ac], XmNmaximum, 100); ac++;
        XtSetArg(av[ac], XmNprocessingDirection, XmMAX_ON_BOTTOM); ac++;
        XtSetArg(av[ac], XmNscaleMultiple, 1); ac++;
        XtSetArg(av[ac], XmNvalue, val); ac++;
        colors.slide[i] = XtCreateManagedWidget("", xmScaleWidgetClass, rc,
                                                av, ac);
        XtAddCallback(colors.slide[i], XmNvalueChangedCallback, AdjustColor,
                      (XtPointer) i);
        XtAddCallback(colors.slide[i], XmNdragCallback, AdjustColor,
                      (XtPointer) i);
    }
}

void
#ifndef _NO_PROTO
DoColorDialog(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoColorDialog(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    /*
     * Make sure the colors for 4bpp are allocated because that is the
     * default display.
     */
    xmbdfedAllocateColors(w, 4);

    if (colors.shell == 0)
      BuildColorDialog();

    colors.update = (Widget) client_data;
    colors.uws = (XtIsSensitive(colors.update) == True) ? 1 : 0;
    colors.switching = colors.modified[0] = colors.modified[1] = 0;

    /*
     * Turn off the reset button.
     */
    XtSetSensitive(colors.reset, False);

    /*
     * Copy the current set of color values so changes can be made without
     * affecting those original colors until an update is done.
     */
    (void) memcpy((char *) colors.colvals, (char *) xmbdfed_opts.colors,
                  sizeof(unsigned short) * 20);

    XtManageChild(colors.form);
    XtPopup(colors.shell, XtGrabNone);
}

/*
 * A general routine to allocate the colors for the two bits per pixel values
 * that are valid.
 */
void
#ifndef _NO_PROTO
xmbdfedAllocateColors(Widget from, int bpp)
#else
xmbdfedAllocateColors(from, bpp)
Widget from;
int bpp;
#endif
{
    int i;
    Colormap cmap;
    XColor col;

    cmap = XDefaultColormap(XtDisplay(from),
                            XDefaultScreen(XtDisplay(from)));
    if (bpp == 2) {
        /*
         * If the color pixels for 2 bits per pixel have already been
         * allocated, just return.
         */
        if (xmbdfed_opts.pixels[0] != ~0)
          return;
        XAllocColorCells(XtDisplay(from), cmap, False, 0, 0,
                         xmbdfed_opts.pixels, 4);
        col.flags = DoRed|DoGreen|DoBlue;
        for (i = 0; i < 4; i++) {
            col.red = col.green = col.blue = xmbdfed_opts.colors[i];
            col.pixel = xmbdfed_opts.pixels[i];
            XStoreColor(XtDisplay(from), cmap, &col);
        }
    } else {
        /*
         * If the color pixels for 4 bits per pixel have already been
         * allocated, just return.
         */
        if (xmbdfed_opts.pixels[4] != ~0)
          return;
        XAllocColorCells(XtDisplay(from), cmap, False, 0, 0,
                         &xmbdfed_opts.pixels[4], 16);
        col.flags = DoRed|DoGreen|DoBlue;
        for (i = 4; i < 20; i++) {
            col.red = col.green = col.blue = xmbdfed_opts.colors[i];
            col.pixel = xmbdfed_opts.pixels[i];
            XStoreColor(XtDisplay(from), cmap, &col);
        }
    }
}

void
#ifndef _NO_PROTO
xmbdfedUpdateColorValues(void)
#else
xmbdfedUpdateColorValues()
#endif
{
    if (colors.modified[0] || colors.modified[1]) {
        (void) memcpy((char *) xmbdfed_opts.colors, (char *) colors.colvals,
                      sizeof(unsigned short) * 20);
        colors.modified[0] = colors.modified[1] = 0;
    }
}

void
#ifndef _NO_PROTO
xmbdfedRestoreColorValues(void)
#else
xmbdfedRestoreColorValues()
#endif
{

    int i, val;
    Colormap cmap;
    XColor col;
    Arg av[1];

    if (colors.modified[0] || colors.modified[1]) {
        cmap = XDefaultColormap(XtDisplay(colors.shell),
                                XDefaultScreen(XtDisplay(colors.shell)));
        col.flags = DoRed|DoGreen|DoBlue;
        for (i = 0; i < 20; i++) {
            if (colors.colvals[i] != xmbdfed_opts.colors[i]) {
                col.red = col.green = col.blue = xmbdfed_opts.colors[i];
                col.pixel = xmbdfed_opts.pixels[i];
                XStoreColor(XtDisplay(colors.shell), cmap, &col);

                /*
                 * Reset the slider value.
                 */
                if ((colors.bpp == 4 && i >= 4) ||
                    (colors.bpp == 2 && i >= 8)) {
                    val = (int) (((float) xmbdfed_opts.colors[i] / 65535.0) *
                                 100.0);
                    colors.switching = 1;
                    XtSetArg(av[0], XmNvalue, val);
                    XtSetValues(colors.slide[i - 4], av, 1);
                    colors.switching = 0;
                } else if (colors.bpp == 2 && i < 4) {
                    val = (int) (((float) xmbdfed_opts.colors[i] / 65535.0) *
                                 100.0);
                    colors.switching = 1;
                    XtSetArg(av[0], XmNvalue, val);
                    XtSetValues(colors.slide[i], av, 1);
                    colors.switching = 0;
                }
            }
        }

        colors.modified[0] = colors.modified[1] = 0;
    }
}

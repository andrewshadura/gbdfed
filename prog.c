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
static char rcsid[] __attribute__ ((unused)) = "$Id: prog.c,v 1.7 2004/01/29 17:15:38 mleisher Exp $";
#else
static char rcsid[] = "$Id: prog.c,v 1.7 2004/01/29 17:15:38 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/DialogS.h>
#include "ProgBar.h"
#include "bdf.h"
#include "xmbdfed.h"

Widget progbar_shell = 0;
Widget progbar_manager = 0;
Widget progbar = 0;

static void
MakeProgressDialog()
{
    Arg av[4];

    XtSetArg(av[0], XmNallowShellResize, True);
    progbar_shell = XtCreatePopupShell("xmbdfed_progress_shell",
                                       xmDialogShellWidgetClass, top, av, 1);

    progbar_manager = XtCreateWidget("xmbdfed_progress_form",
                                     xmFormWidgetClass, progbar_shell, 0, 0);

    XtSetArg(av[0], XmNtopAttachment, XmATTACH_FORM);
    XtSetArg(av[1], XmNleftAttachment, XmATTACH_FORM);
    XtSetArg(av[2], XmNrightAttachment, XmATTACH_FORM);
    XtSetArg(av[3], XmNbottomAttachment, XmATTACH_FORM);
    progbar = XtCreateManagedWidget("xmbdfed_progress_bar",
                                    xmuttProgressBarWidgetClass,
                                    progbar_manager, av, 4);
}

void
#ifndef _NO_PROTO
UpdateProgressBar(bdf_callback_struct_t *cb, void *data)
#else
UpdateProgressBar(cb, data)
bdf_callback_struct_t *cb;
void *data;
#endif
{
    char *op, *op1;
    XmString label;
    XtInputMask mask;
    Arg av[5];

    op = op1 = 0;

    if (progbar_shell == 0)
      MakeProgressDialog();

    if (cb->reason == BDF_LOAD_START || cb->reason == BDF_SAVE_START ||
        cb->reason == BDF_TRANSLATE_START || cb->reason == BDF_ROTATE_START ||
        cb->reason == BDF_SHEAR_START || cb->reason == BDF_GLYPH_NAME_START ||
        cb->reason == BDF_EXPORT_START) {
        switch (cb->reason) {
          case BDF_LOAD_START:
            op = "Load";
            op1 = "Loading";
            break;
          case BDF_SAVE_START:
            op = "Save";
            op1 = "Saving";
            break;
          case BDF_TRANSLATE_START:
            op = "Translate";
            op1 = "Translating";
            break;
          case BDF_ROTATE_START:
            op = "Rotate";
            op1 = "Rotating";
            break;
          case BDF_SHEAR_START:
            op = "Shear";
            op1 = "Shearing";
            break;
          case BDF_GLYPH_NAME_START:
            op = "Rename";
            op1 = "Renaming";
            break;
          case BDF_EXPORT_START:
            op = "Export";
            op1 = "Exporting";
            break;
        }
        if (cb->reason == BDF_LOAD_START || cb->reason == BDF_SAVE_START)
          sprintf(title, "%s - %s font %s", app_name, op, (char *) data);
        else
          sprintf(title, "%s - %s Glyphs", app_name, op);
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(progbar_shell, av, 1);

        if (cb->reason == BDF_LOAD_START || cb->reason == BDF_SAVE_START ||
            cb->reason == BDF_EXPORT_START)
          sprintf(name, "%s %s ...", op1, (char *) data);
        else
          sprintf(name, "%s ...", op1);
        label = XmStringCreateSimple(name);

        XtSetArg(av[0], XmNminValue, 0);
        XtSetArg(av[1], XmNmaxValue, cb->total);
        XtSetArg(av[2], XmNcurrentValue, cb->current);
        XtSetArg(av[3], XmNlabelString, label);
        XtSetArg(av[4], XmNpercentOnly, xmbdfed_opts.percent_only);
        XtSetValues(progbar, av, 5);
        XmStringFree(label);
        XtManageChild(progbar_manager);
        XtPopup(progbar_shell, XtGrabNone);
    } else if (cb->reason == BDF_ERROR) {
        XtPopdown(progbar_shell);
        ErrorDialog("Font I/O Error.\nSee Messages.");
        return;
    } else {
        XtSetArg(av[0], XmNminValue, 0);
        XtSetArg(av[1], XmNmaxValue, cb->total);
        XtSetArg(av[2], XmNcurrentValue, cb->current);
        XtSetArg(av[3], XmNpercentOnly, xmbdfed_opts.percent_only);
        XtSetValues(progbar, av, 4);
    }

    if (cb->current == cb->total)
      XtPopdown(progbar_shell);
    else {
        XFlush(XtDisplay(progbar_shell));
        XmUpdateDisplay(progbar_shell);
    }

    /*
     * Process the events so we get an update.
     */
    while ((mask = XtAppPending(app)))
      XtAppProcessEvent(app, mask);
}

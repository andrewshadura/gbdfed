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
static char rcsid[] __attribute__ ((unused)) = "$Id: comment.c,v 1.4 2000/03/16 20:08:53 mleisher Exp $";
#else
static char rcsid[] = "$Id: comment.c,v 1.4 2000/03/16 20:08:53 mleisher Exp $";
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
    XtSetSensitive(ed->comments.update, True);
}

static void
#ifndef _NO_PROTO
DoCommentUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCommentUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    char *comments;
    unsigned long comments_len;

    ed = &editors[(unsigned long) client_data];

    comments = XmTextGetString(ed->comments.text);
    comments_len = XmTextGetLastPosition(ed->comments.text);

    XmuttFontGridUpdateComments(ed->fgrid, comments, comments_len);
    XtFree(comments);

    XtSetSensitive(ed->comments.update, False);
}

static void
#ifndef _NO_PROTO
DoCloseComments(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCloseComments(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (XtIsSensitive(ed->comments.update)) {
        if (AskBooleanQuestion("Comments have been modified.", "Update",
                               "Do Not Update") == True)
          DoCommentUpdate(w, client_data, call_data);
    }
    XtPopdown(ed->comments.shell);
}

static void
#ifndef _NO_PROTO
BuildComments(MXFEditor *ed)
#else
BuildComments(ed)
MXFEditor *ed;
#endif
{
    Widget pb, frame, form;
    Cardinal ac;
    Arg av[10];

    if (ed->file == 0)
      sprintf(title, "%s - Comments: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Comments: %s", app_name, ed->file);

    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    sprintf(name, "xmbdfed_comments%ld_shell", ed->id);
    ed->comments.shell = XtCreatePopupShell(name, xmDialogShellWidgetClass,
                                            top, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    sprintf(name, "xmbdfed_comments%ld_form", ed->id);
    ed->comments.form = form =
        XtCreateWidget(name, xmFormWidgetClass, ed->comments.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_comments%ld_frame", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, ed->comments.form,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 5); ac++;
    sprintf(name, "xmbdfed_comments%ld_button_form", ed->id);
    form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    ed->comments.update = XtCreateManagedWidget("Update",
                                                xmPushButtonWidgetClass, form,
                                                av, ac);
    XtAddCallback(ed->comments.update, XmNactivateCallback,
                  DoCommentUpdate, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 3); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 4); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    pb = XtCreateManagedWidget("Done", xmPushButtonWidgetClass, form, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoCloseComments,
                  (XtPointer) ed->id);

    form = ed->comments.form;

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
    XtSetArg(av[ac], XmNcolumns, 40); ac++;
    XtSetArg(av[ac], XmNrows, 10); ac++;
    XtSetArg(av[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    sprintf(name, "xmbdfed_comments%ld_text", ed->id);
    ed->comments.text = XmCreateScrolledText(ed->comments.form, name,
                                             av, ac);
    XtManageChild(ed->comments.text);
    XtAddCallback(ed->comments.text, XmNvalueChangedCallback,
                  EnableUpdate, (XtPointer) ed->id);
}

void
#ifndef _NO_PROTO
DoEditComments(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoEditComments(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    bdf_font_t *f;
    Arg av[1];

    ed = &editors[(unsigned long) client_data];
    if (ed->comments.shell == 0)
      BuildComments(ed);

    /*
     * Erase the comment editor text widget.
     */
    XmTextSetString(ed->comments.text, "");

    /*
     * Add the comments from the font.
     */
    f = XmuttFontGridFont(ed->fgrid);
    if (f->comments_len > 0)
      XmTextSetString(ed->comments.text, f->comments);

    XtSetSensitive(ed->comments.update, False);

    /*
     * Make sure the title matches the font name.
     */
    if (ed->file == 0)
      sprintf(title, "%s - Comments: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Comments: %s", app_name, ed->file);

    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ed->comments.shell, av, 1);

    XtManageChild(ed->comments.form);
    XtPopup(ed->comments.shell, XtGrabNone);
}

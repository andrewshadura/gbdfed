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
static char rcsid[] __attribute__ ((unused)) = "$Id: help.c,v 1.8 2000/03/16 20:08:54 mleisher Exp $";
#else
static char rcsid[] = "$Id: help.c,v 1.8 2000/03/16 20:08:54 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>
#include "bdf.h"
#include "xmbdfed.h"
#include "htext.h"

typedef struct {
    Widget shell;
    Widget form;
    Widget topics;
    Widget text;
} MXFEditorHelp;

static MXFEditorHelp help;

typedef struct {
    int mnemonic;
    char *help_topic;
    char *help_text;
} MXFEditorHelpMap;

static MXFEditorHelpMap topics[] = {
    {'A', "About", 0},
    {'P', "The Program", 0},
    {'F', "Font Grid", 0},
    {'G', "Glyph Editor", 0},
    {'C', "Configuration File", 0},
    {'S', "Setup Dialog", 0},
    {'W', "Windows Font Notes", 0},
    {'T', "TrueType Font Notes", 0},
    {'P', "PSF Font Notes", 0},
    {'H', "HEX Font Notes", 0},
    {'o', "Color Notes", 0},
};

static unsigned long ntopics = sizeof(topics) / sizeof(topics[0]);

static void
#ifndef _NO_PROTO
ChooseTopic(Widget w, XtPointer client_data, XtPointer call_data)
#else
ChooseTopic(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XmListCallbackStruct *cb;

    cb = (XmListCallbackStruct *) call_data;

    XmTextSetString(help.text, topics[cb->item_position - 1].help_text);
}

static void
#ifndef _NO_PROTO
DoHelp(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoHelp(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long i;
    Widget label, pb, frame;
    XmString *items;
    Dimension ht;
    Cardinal ac;
    Arg av[12];

    if (help.shell == 0) {
        /*
         * Set up the topic structure.
         */
        topics[0].help_text = about_text;
        topics[1].help_text = program_text;
        topics[2].help_text = fgrid_text;
        topics[3].help_text = gedit_text;
        topics[4].help_text = conf_text;
        topics[5].help_text = setup_text;
        topics[6].help_text = fnt_text;
        topics[7].help_text = ttf_text;
        topics[8].help_text = psf_text;
        topics[9].help_text = hex_text;
        topics[10].help_text = color_text;

        sprintf(title, "%s - Help", app_name);
        ac = 0;
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetArg(av[ac], XmNallowShellResize, True); ac++;
        XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
        help.shell = XtCreatePopupShell("xmbdfed_help_shell",
                                        xmDialogShellWidgetClass, top,
                                        av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
        XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
        XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
        help.form = XtCreateWidget("xmbdfed_help_form", xmFormWidgetClass,
                                   help.shell, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        frame = XtCreateManagedWidget("xmbdfed_help_frame", xmFrameWidgetClass,
                                      help.form, av, ac);

        pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, frame,
                                   0, 0);
        XtAddCallback(pb, XmNactivateCallback, DoClose,
                      (XtPointer) help.shell);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        label = XtCreateManagedWidget("Help Topics", xmLabelWidgetClass,
                                      help.form, av, ac);

        items = (XmString *) XtMalloc(sizeof(XmString) * ntopics);
        for (i = 0; i < ntopics; i++)
          items[i] = XmStringCreateSimple(topics[i].help_topic);

        ac = 0;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, label); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
        XtSetArg(av[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
        XtSetArg(av[ac], XmNlistSizePolicy, XmVARIABLE); ac++;
        XtSetArg(av[ac], XmNitems, items); ac++;
        XtSetArg(av[ac], XmNitemCount, ntopics); ac++;
        help.topics = XmCreateScrolledList(help.form, "xmbdfed_help_topics",
                                           av, ac);
        XtAddCallback(help.topics, XmNbrowseSelectionCallback, ChooseTopic, 0);
        for (i = 0; i < ntopics; i++)
          XmStringFree(items[i]);
        XtFree((char *) items);

        XtManageChild(help.topics);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, label); ac++;
        label = XtCreateManagedWidget("Help Text", xmLabelWidgetClass,
                                      help.form, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, label); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, XtParent(help.topics)); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
        XtSetArg(av[ac], XmNcolumns, 45); ac++;
        XtSetArg(av[ac], XmNrows, 15); ac++;
        XtSetArg(av[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
        XtSetArg(av[ac], XmNeditable, False); ac++;
        XtSetArg(av[ac], XmNcursorPositionVisible, False); ac++;
        help.text = XmCreateScrolledText(help.form, "xmbdfed_help_text",
                                         av, ac);
        XtManageChild(help.text);

        XtSetArg(av[0], XmNheight, &ht);
        XtGetValues(help.text, av, 1);
        XtSetArg(av[0], XmNheight, ht);
        XtSetValues(help.topics, av, 1);

    }

    /*
     * Select the topic passed.
     */
    i = (unsigned long) client_data;
    XmListSelectPos(help.topics, i + 1, True);

    XtManageChild(help.form);
    XtPopup(help.shell, XtGrabNone);
}

void
#ifndef _NO_PROTO
AddHelpMenu(Widget pdown)
#else
AddHelpMenu(pdown)
Widget pdown;
#endif
{
    Widget pb;
    unsigned long i;
    Arg av[1];

    for (i = 0; i < ntopics; i++) {
        XtSetArg(av[0], XmNmnemonic, topics[i].mnemonic);
        pb = XtCreateManagedWidget(topics[i].help_topic,
                                   xmPushButtonWidgetClass, pdown, av, 1);
        XtAddCallback(pb, XmNactivateCallback, DoHelp, (XtPointer) i);
    }
}

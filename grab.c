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
static char rcsid[] __attribute__ ((unused)) = "$Id: grab.c,v 1.6 2001/09/19 21:00:43 mleisher Exp $";
#else
static char rcsid[] = "$Id: grab.c,v 1.6 2001/09/19 21:00:43 mleisher Exp $";
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

typedef struct {
    Widget shell;
    Widget form;
    Widget flist;
    Widget flist_label;
    Widget filter;
    Widget sel;
    Widget getfont;
} MXFEditorFontGrab;

static unsigned long editor;
static MXFEditorFontGrab grab;

#define _GRAB_MAXNAMES 5000
#define _GRAB_DEFAULT_FILTER "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

void
#ifndef _NO_PROTO
EnableGetButton(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnableGetButton(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XtSetSensitive(grab.getfont, True);
}

void
#ifndef _NO_PROTO
SelectFont(Widget w, XtPointer client_data, XtPointer call_data)
#else
SelectFont(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    char *pname;
    XmListCallbackStruct *cb;

    cb = (XmListCallbackStruct *) call_data;

    XmStringGetLtoR(cb->item, XmFONTLIST_DEFAULT_TAG, &pname);
    XmTextFieldSetString(grab.sel, pname);
    XtFree(pname);
}

void
#ifndef _NO_PROTO
GrabFont(Widget w, XtPointer client_data, XtPointer call_data)
#else
GrabFont(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    XFontStruct *f;
    char *fname;
    bdf_font_t *font;
    XmString s;
    Arg av[2];
    XmuttFontGridPageInfoStruct pi;

    ed = &editors[editor];

    /*
     * Get the font name from the selection field.
     */
    fname = XmTextFieldGetString(grab.sel);

    /*
     * Attempt to load the font from the server.
     */
    if ((f = XLoadQueryFont(XtDisplay(w), fname)) == 0) {
        sprintf(name, "%s: Unable to load font '%s'.", app_name, fname);
        ErrorDialog(name);
        XtFree(fname);
        return;
    }

    /*
     * Load the font.
     */
    if (xmbdfed_opts.progbar)
      font = bdf_load_server_font(XtDisplay(w), f, fname,
                                  &xmbdfed_opts.font_opts, UpdateProgressBar,
                                  "server font");
    else {
        WatchCursor(ed->fgrid, True);
        font = bdf_load_server_font(XtDisplay(w), f, fname,
                                    &xmbdfed_opts.font_opts, 0, 0);
        WatchCursor(ed->fgrid, False);
    }
    XFreeFont(XtDisplay(w), f);

    if (font == 0) {
        sprintf(name, "%s: Problem loading font '%s'.", app_name, fname);
        ErrorDialog(name);
        XtFree(fname);
        return;
    }

    /*
     * Free up the font name from the selection text field.
     */
    XtFree(fname);

    /*
     * Clear out the file and directory names in the destination editor.
     */
    if (ed->path != 0)
      XtFree(ed->path);
    if (ed->file != 0)
      XtFree(ed->file);
    ed->path = ed->file = 0;

    /*
     * Invalidate the original font in the glyph test dialog.
     */
    GlyphTestReset(XmuttFontGridFont(ed->fgrid));

    /*
     * Change the font in the font editor.
     */
    XtSetArg(av[0], XmNbdfFont, font);
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

    XtPopdown(grab.shell);
}

static int
#ifndef _NO_PROTO
by_name(const void *a, const void *b)
#else
by_name(a, b)
char *a, *b;
#endif
{
    char *s1, *s2;

    s1 = *((char **) a);
    s2 = *((char **) b);
    return strcmp(s1, s2);
}

void
#ifndef _NO_PROTO
DoFilter(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFilter(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    Boolean free_pat;
    int i, nnames;
    XmString s, *items;
    Arg av[1];
    char **names, **sorted_names, *pat, nbuf[24];

    free_pat = True;

    /*
     * Get the new filter pattern.  If the pattern is empty, assume
     * that the default pattern is wanted.
     */
    pat = XmTextFieldGetString(grab.filter);
    if (pat[0] == 0 || strcmp(pat, _GRAB_DEFAULT_FILTER) == 0) {
        free_pat = False;
        XtFree(pat);
        pat = "*";
    }

    /*
     * Clear the font name list before doing anything.
     */
    XmListDeleteAllItems(grab.flist);

    names = XListFonts(XtDisplay(w), pat, _GRAB_MAXNAMES, &nnames);

    if (free_pat == True)
      XtFree(pat);

    if (nnames > 0) {
        /*
         * Sort the names.
         */
        sorted_names = (char **) XtMalloc(sizeof(char *) * nnames);
        (void) memcpy(sorted_names, names, sizeof(char *) * nnames);
        qsort((char *) sorted_names, nnames, sizeof(char *), by_name);

        items = (XmString *) XtMalloc(sizeof(XmString) * nnames);
        for (i = 0; i < nnames; i++)
          items[i] = XmStringCreateSimple(sorted_names[i]);

        XmListAddItems(grab.flist, items, nnames, 0);

        for (i = 0; i < nnames; i++)
          XmStringFree(items[i]);
        XtFree((char *) items);

        XFreeFontNames(names);
        XtFree((char *) sorted_names);
    }

    /*
     * Update the Font List frame label with the number of font names
     * found.
     */
    sprintf(nbuf, "Font List: %d", nnames);
    s = XmStringCreateSimple(nbuf);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(grab.flist_label, av, 1);
    XmStringFree(s);
}

static void
#ifndef _NO_PROTO
BuildFontGrabber(void)
#else
BuildFontGrabber()
#endif
{
    Widget form, frame, pb, label;
    char **names, **sorted_names, nbuf[24];
    int i, nnames;
    XmString s, *items;
    Cardinal ac;
    Arg av[10];

    sorted_names = 0;
    items = 0;

    sprintf(title, "%s - Grab Server Font", app_name);
    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    grab.shell = XtCreatePopupShell("xmbdfed_grabfont_shell",
                                    xmDialogShellWidgetClass, top, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    grab.form = XtCreateWidget("xmbdfed_grabfont_form", xmFormWidgetClass,
                               grab.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    frame = XtCreateManagedWidget("xmbdfed_grabfont_filter_frame",
                                  xmFrameWidgetClass, grab.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Filter", xmLabelWidgetClass, frame,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNvalue, _GRAB_DEFAULT_FILTER); ac++;
    grab.filter = XtCreateManagedWidget("xmbdfed_grabfont_filter",
                                        xmTextFieldWidgetClass, frame,
                                        av, ac);
    XtAddCallback(grab.filter, XmNactivateCallback, DoFilter, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    frame = XtCreateManagedWidget("xmbdfed_grabfont_fontlist_frame",
                                  xmFrameWidgetClass, grab.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    grab.flist_label = XtCreateManagedWidget("Font List", xmLabelWidgetClass,
                                             frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleItemCount, 8); ac++;
    XtSetArg(av[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
    XtSetArg(av[ac], XmNlistSizePolicy, XmVARIABLE); ac++;

    /*
     * Construct the list of font names from the server.
     */
    names = XListFonts(XtDisplay(top), "*", _GRAB_MAXNAMES,
                       &nnames);
    if (nnames > 0) {
        /*
         * Sort the names.
         */
        sorted_names = (char **) XtMalloc(sizeof(char *) * nnames);
        (void) memcpy(sorted_names, names, sizeof(char *) * nnames);
        qsort((char *) sorted_names, nnames, sizeof(char *), by_name);

        items = (XmString *) XtMalloc(sizeof(XmString) * nnames);
        for (i = 0; i < nnames; i++)
          items[i] = XmStringCreateSimple(sorted_names[i]);
        XtSetArg(av[ac], XmNitems, items); ac++;
        XtSetArg(av[ac], XmNitemCount, nnames); ac++;
    }

    grab.flist = XmCreateScrolledList(frame, "xmbdfed_grabfont_fontlist",
                                      av, ac);
    XtManageChild(grab.flist);

    XtAddCallback(grab.flist, XmNbrowseSelectionCallback, SelectFont, 0);
    XtAddCallback(grab.flist, XmNdefaultActionCallback, GrabFont, 0);

    if (nnames > 0) {
        XFreeFontNames(names);
        XtFree((char *) sorted_names);
        for (i = 0; i < nnames; i++)
          XmStringFree(items[i]);
        XtFree((char *) items);
    }

    /*
     * Update the font name list frame label with the number of font names
     * found.
     */
    sprintf(nbuf, "Font List: %d", nnames);
    s = XmStringCreateSimple(nbuf);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(grab.flist_label, av, 1);
    XmStringFree(s);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    frame = XtCreateManagedWidget("xmbdfed_grabfont_selection_frame",
                                  xmFrameWidgetClass, grab.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Selection", xmLabelWidgetClass, frame,
                                  av, ac);

    ac = 0;
    grab.sel = XtCreateManagedWidget("xmbdfed_grabfont_selection",
                                     xmTextFieldWidgetClass, frame,
                                     av, ac);
    XtAddCallback(grab.sel, XmNvalueChangedCallback, EnableGetButton, 0);
    XtAddCallback(grab.sel, XmNactivateCallback, GrabFont, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_grabfont_button_frame",
                                  xmFrameWidgetClass, grab.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 3); ac++;
    form = XtCreateManagedWidget("xmbdfed_grabfont_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    XtSetArg(av[ac], XmNsensitive, False); ac++;
    grab.getfont = XtCreateManagedWidget("Get Font", xmPushButtonWidgetClass,
                                         form, av, ac);
    XtAddCallback(grab.getfont, XmNactivateCallback, GrabFont, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    pb = XtCreateManagedWidget("Filter", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoFilter, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 2); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 3); ac++;
    pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, form, av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoClose, (XtPointer) grab.shell);
}

void
#ifndef _NO_PROTO
DoGrabServerFont(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoGrabServerFont(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    /*
     * Set the editor that has requested the font grabber.
     */
    editor = (unsigned long) client_data;

    ed = &editors[editor];

    /*
     * Make sure the open/import dialog box is popped down if it exists.
     */
    if (opend != 0)
      XtPopdown(XtParent(opend));

    /*
     * Check to see if the user wants to save the current font if it was
     * modified.
     */
    if (XmuttFontGridModified(ed->fgrid) == True)
      WaitSave(w, ed);

    if (grab.shell == 0)
      BuildFontGrabber();

    XtManageChild(grab.form);
    XtPopup(grab.shell, XtGrabNone);
}

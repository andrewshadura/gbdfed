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
static char rcsid[] __attribute__ ((unused)) = "$Id: fntin.c,v 1.5 2001/09/19 21:00:43 mleisher Exp $";
#else
static char rcsid[] = "$Id: fntin.c,v 1.5 2001/09/19 21:00:43 mleisher Exp $";
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/TextF.h>
#include "FGrid.h"
#include "GTest.h"
#include "xmbdfed.h"

typedef struct {
    Widget shell;
    Widget form;
    Widget fonts;
    Widget label;
    Widget ok;
} MXFFNTDialog;

static MXFFNTDialog fntd;

static int select_action;

static void
#ifndef _NO_PROTO
SelectAction(Widget w, XtPointer client_data, XtPointer call_data)
#else
SelectAction(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    select_action = (int) client_data;
}

static void
#ifndef _NO_PROTO
BuildFNTDialog(void)
#else
BuildFNTDialog()
#endif
{
    Widget pb, form, frame;
    Cardinal ac;
    Arg av[10];

    sprintf(title, "%s: Windows Font Select", app_name);

    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    fntd.shell = XtCreatePopupShell("xmbdfed_fntd_shell",
                                    xmDialogShellWidgetClass, top, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    fntd.form = XtCreateWidget("xmbdfed_fntd_form", xmFormWidgetClass,
                               fntd.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_fntd_name_frame",
                                  xmFrameWidgetClass, fntd.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    fntd.label = XtCreateManagedWidget("Fonts: 0", xmLabelWidgetClass, frame,
                                       av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleItemCount, 4); ac++;
    XtSetArg(av[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); ac++;
    XtSetArg(av[ac], XmNselectionPolicy, XmMULTIPLE_SELECT); ac++;
    fntd.fonts = XmCreateScrolledList(frame, "xmbdfed_fntd_fontlist",
                                      av, ac);
    XtManageChild(fntd.fonts);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_fntd_button_frame",
                                  xmFrameWidgetClass, fntd.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 3); ac++;
    form = XtCreateManagedWidget("xmbdfed_fntd_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    fntd.ok = XtCreateManagedWidget("Import", xmPushButtonWidgetClass, form,
                                    av, ac);
    XtAddCallback(fntd.ok, XmNactivateCallback, SelectAction, (XtPointer) 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    pb = XtCreateManagedWidget("Import All", xmPushButtonWidgetClass,
                               form, av, ac);
    XtAddCallback(pb, XmNactivateCallback, SelectAction, (XtPointer) 1);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 2); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 3); ac++;
    pb = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, SelectAction, (XtPointer) 2);
}

typedef struct {
    char *file;
    unsigned long total;
    unsigned long current;
    unsigned long lastcurr;
} LocalUpdateRec;

static void
#ifndef _NO_PROTO
LocalUpdateProgressBar(bdf_callback_struct_t *cb, void *data)
#else
LocalUpdateProgressBar(cb, data)
bdf_callback_struct_t *cb;
void *data;
#endif
{
    LocalUpdateRec *up;
    bdf_callback_struct_t ncb;

    up = (LocalUpdateRec *) data;

    /*
     * Reset the last value seen and return if this is called after loading
     * the previous font.
     */
    if (up->current != 0 && cb->current == 0) {
        up->lastcurr = 0;
        return;
    }

    up->current += (cb->current - up->lastcurr);
    up->lastcurr = cb->current;

    ncb.reason = cb->reason;
    ncb.total = up->total;
    ncb.current = up->current;

    UpdateProgressBar(&ncb, (void *) up->file);
}

int
#ifdef __STDC__
LoadWindowsFont(char **path, MXFEditor *ed, bdf_font_t **out)
#else
LoadWindowsFont(path, ed, out)
char **path;
MXFEditor *ed;
bdf_font_t **out;
#endif
{
    MXFEditor *ep;
    int i, n, nfonts, plen, pcnt, *plist;
    bdffnt_font_t fnt;
    XmString s;
    bdf_font_t *f, **flist;
    char *pp, *file, *ext;
    Arg av[4];
    LocalUpdateRec up;
    XmuttFontGridPageInfoStruct pi;

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

    if (bdffnt_open_font(pp, &fnt) <= 0) {
        sprintf(name, "Unable to open font '%s'.", file);
        ErrorDialog(name);
        return 0;
    }

    /*
     * Create the dialog that lets the user choose the fonts from a
     * collection.
     */
    if ((n = bdffnt_font_count(fnt)) > 1) {
        if (fntd.shell == 0)
          BuildFNTDialog();

        /*
         * Set the number of fonts in the frame label.
         */
        sprintf(name, "Fonts: %d", n);
        s = XmStringCreateSimple(name);
        XtSetArg(av[0], XmNlabelString, s);
        XtSetValues(fntd.label, av, 1);
        XmStringFree(s);

        /*
         * Clear the name list.
         */
        XmListDeleteAllItems(fntd.fonts);

        /*
         * Create the list of font names.
         */
        for (i = 0; i < n; i++) {
            (void) bdffnt_get_facename(fnt, i, 0, (unsigned char *) name);
            s = XmStringCreateSimple(name);
            XmListAddItemUnselected(fntd.fonts, s, 0);
            XmStringFree(s);
        }

        XtManageChild(fntd.form);
        XtPopup(fntd.shell, XtGrabNone);

        /*
         * Wait for the user to select the font or cancel.
         */
        select_action = -1;
        while (select_action == -1)
          XtAppProcessEvent(app, XtIMAll);

        if (select_action == 2) {
            /*
             * A select action of 2 means to pop the font select dialog down
             * and return.
             */
            bdffnt_close_font(fnt);
            XtPopdown(fntd.shell);
            return 0;
        } else if (select_action == 1) {
            /*
             * A select action of 1 means to select all the fonts for
             * importing.
             */
            for (i = 0; i < n; i++) {
                if (XmListPosSelected(fntd.fonts, i + 1) == False)
                  XmListSelectPos(fntd.fonts, i + 1, False);
            }
        }

        /*
         * Get a list of the selected fonts.
         */
        if (XmListGetSelectedPos(fntd.fonts, &plist, &pcnt) == True) {
            /*
             * Allocate enough space to hold all the fonts.
             */
            flist = (bdf_font_t **) malloc(sizeof(bdf_font_t *) * pcnt);

            /*
             * Get a total count of characters being loaded to be used
             * for the progress dialog.
             */
            for (up.total = i = 0; i < pcnt; i++)
              up.total += bdffnt_char_count(fnt, plist[i] - 1);

            /*
             * Set up the local update record for the callbacks.
             */
            up.file = file;
            up.current = up.lastcurr = 0;

            /*
             * Now load each font in turn.
             */
            for (nfonts = i = 0; i < pcnt; i++) {
                if (xmbdfed_opts.progbar) {
                    if (bdffnt_load_font(fnt, plist[i] - 1,
                                         LocalUpdateProgressBar,
                                         (void *) &up, &flist[nfonts]) == 0)
                      nfonts++;
                } else {
                    WatchCursor(ed->fgrid, True);
                    if (bdffnt_load_font(fnt, plist[i] - 1, 0, 0,
                                         &flist[nfonts]) == 0)
                      nfonts++;
                    WatchCursor(ed->fgrid, False);
                }
            }

            /*
             * Free the list of selected items because they are not needed any
             * more.
             */
            XtFree((char *) plist);

            /*
             * If there were problems loading one or more of the fonts, issue
             * an error dialog.
             */
            if (nfonts == 0) {
                sprintf(name, "Problem loading font '%s'.", file);
                ErrorDialog(name);

                /*
                 * If no fonts were loaded, then close the select dialog
                 * and return.
                 */
                bdffnt_close_font(fnt);
                XtPopdown(fntd.shell);
                return 0;
            }

            /*
             * Munge the font path so file names can be made up for the fonts
             * loaded.
             */
            if (file != pp)
              plen = (file - 1) - pp;

            /*
             * Create the editors needed.
             */
            for (i = 0; i < nfonts; i++) {
                ep = (i) ? &editors[MakeEditor(top, 0, False)] : ed;

                /*
                 * Set the path and file name in the editor.
                 */
                if (ep == ed) {
                    /*
                     * Free up the old file info if this is the current
                     * editor.
                     */
                    if (ep->path != 0)
                      XtFree(ep->path);
                    if (ep->file != 0)
                      XtFree(ep->file);
                }
                ep->path = (char *) XtMalloc(plen + 1);
                (void) memcpy(ep->path, pp, plen);
                ep->path[plen] = 0;
                ep->file = (char *) XtMalloc((ext - file) + 8);
                (void) memcpy(ep->file, file, ext - file);
                ep->file[ext - file] = 0;
                sprintf(ep->file + (ext - file), "%ld.bdf",
                        flist[i]->point_size);

                /*
                 * If the glyph test dialog uses the font that is about to be
                 * replaced, erase it.
                 */
                if (ep == ed)
                  GlyphTestReset(XmuttFontGridFont(ep->fgrid));

                /*
                 * Configure the new editor.
                 */
                XtSetArg(av[0], XmNbdfFont, flist[i]);
                XtSetArg(av[1], XmNdisplayUnencoded, False);
                XtSetValues(ep->fgrid, av, 2);

                ep->update_fontname = False;
                XmTextFieldSetString(ep->fontname,
                                     XmuttFontGridFontName(ep->fgrid));
                ep->update_fontname = True;

                s = XmStringCreateLtoR("None\nNone", XmSTRING_DEFAULT_CHARSET);
                XtSetArg(av[0], XmNlabelString, s);
                XtSetValues(ep->glyphinfo, av, 1);
                XmStringFree(s);

                /*
                 * Enable or disable the navigation buttons based on the
                 * page info of the font.
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
                s = XmStringCreateSimple("Unencoded");
                XtSetArg(av[0], XmNlabelString, s);
                XtSetValues(ep->viewmenu.unencoded, av, 1);
                XmStringFree(s);

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
                 * Enable the Save menu option if the font has been
                 * modified.  Also update the title.
                 */
                if (XmuttFontGridModified(ep->fgrid) == True) {
                    if (ep->file == 0)
                      sprintf(title, "%s - (unnamed%ld) [modified]",
                              app_name, ep->id);
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
             * Free up the allocated font list here.
             */
            free((char *) flist);
            bdffnt_close_font(fnt);
            XtPopdown(fntd.shell);
            return 2;
        }
        /*
         * No fonts were selected, so simply pop the font select dialog down
         * and return.
         */
        bdffnt_close_font(fnt);
        XtPopdown(fntd.shell);
        return 0;
    }

    /*
     * Only one font needs to be loaded, so handle that here.
     */
    if (xmbdfed_opts.progbar) {
        if (bdffnt_load_font(fnt, 0, UpdateProgressBar, (void *) file,
                             &f) != 0) {
            sprintf(name, "Problem loading font '%s'.\n", file);
            ErrorDialog(name);
            return 0;
        }
    } else {
        WatchCursor(ed->fgrid, True);
        if (bdffnt_load_font(fnt, 0, 0, 0, &f) != 0) {
            sprintf(name, "Problem loading font '%s'.\n", file);
            ErrorDialog(name);
            return 0;
        }
        WatchCursor(ed->fgrid, False);
    }
    /*
     * Munge the path name so the new extension can be added.
     */
    *path = pp = (char *) XtRealloc(pp, plen + 8);
    if ((file = strrchr(pp, '/')) == 0)
      file = pp + plen;
    else
      file++;

    if ((ext = strrchr(file, '.')) == 0)
      ext = pp + plen;
    sprintf(ext, "%ld.bdf", f->point_size);

    /*
     * Set the return font.
     */
    *out = f;

    bdffnt_close_font(fnt);
    if (fntd.shell != 0)
      XtPopdown(fntd.shell);

    return 1;
}

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

/*
 * Only compile this if the FreeType library is available.
 */
#ifdef HAVE_FREETYPE

#ifndef lint
#ifdef __GNUC__
static char rcsid[] __attribute__ ((unused)) = "$Id: ttfin.c,v 1.4 2000/03/16 20:08:56 mleisher Exp $";
#else
static char rcsid[] = "$Id: ttfin.c,v 1.4 2000/03/16 20:08:56 mleisher Exp $";
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
#include "FGrid.h"
#include "GTest.h"
#include "xmbdfed.h"

typedef struct {
    Widget shell;
    Widget form;
    Widget names;
    Widget platforms;
    Widget encodings;
    Widget ok;
} MXFTTFDialog;

/*
 * Structure used to hold the dialog.
 */
static MXFTTFDialog ttfd;

/*
 * Flag used to track interaction with the TrueType selection dialog and
 * indicate which option was selected.
 */
static Boolean select_done;
static Boolean open_font;

/*
 * Flag indicating if the FreeType engine has been initialized or not.
 */
static int ftinit = 0;

/*
 * Globals used for FreeType.
 */
static TT_Engine engine;
static TT_Face face;
static TT_Face_Properties properties;

/*
 * The path of the file being used.  This is needed because the file is opened
 * more than once if it is a collection.
 */
static char *filepath;

/*
 * The number of fonts found in the file.  Used to distinguish between
 * collections and single fonts.
 */
static int iscoll;

/*
 * Flag indicating if a face is open or not.
 */
static int face_open;

/*
 * List of platform IDs seen that is used when platforms are selected.
 */
static short platforms[32];
static int nplatforms;

/*
 * List of encoding IDs seen that is used when encodings are selected.
 */
static short encodings[34];
static int nencodings;

/**************************************************************************
 *
 * TrueType platform and encoding functions.
 *
 **************************************************************************/

static void
#ifndef _NO_PROTO
SelectPlatform(Widget w, XtPointer client_data, XtPointer call_data)
#else
SelectPlatform(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XmListCallbackStruct *cb;
    XmString s;
    int ncmaps;
    short pid, eid, lasteid, i;

    cb = (XmListCallbackStruct *) call_data;

    /*
     * Erase the encoding list and reset the OK button.
     */
    XmListDeleteAllItems(ttfd.encodings);
    XtSetSensitive(ttfd.ok, False);

    /*
     * Collect the list of encoding IDs and put their names in the encoding
     * list.
     */
    nencodings = 0;
    ncmaps = TT_Get_CharMap_Count(face);
    for (lasteid = -1, i = 0; i < ncmaps; i++) {
        TT_Get_CharMap_ID(face, i, &pid, &eid);
        if (pid == platforms[cb->item_position - 1] && eid != lasteid) {
            s = XmStringCreateSimple(bdfttf_encoding_name(pid, eid));
            XmListAddItemUnselected(ttfd.encodings, s, 0);
            XmStringFree(s);

            encodings[nencodings++] = eid;
            lasteid = eid;
        }
    }

    /*
     * If there is only one encoding for the platform, automatically select it
     * so the user does not have to.
     */
    if (nencodings == 1)
      XmListSelectPos(ttfd.encodings, 1, True);
}

static void
#ifndef _NO_PROTO
SelectFont(Widget w, XtPointer client_data, XtPointer call_data)
#else
SelectFont(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XmListCallbackStruct *cb;
    XmString s;
    char *fname;
    int ncmaps;
    short pid, eid, lastpid, i;

    /*
     * Clear both the platform and encoding lists.
     */
    XmListDeleteAllItems(ttfd.platforms);
    XmListDeleteAllItems(ttfd.encodings);

    cb = (XmListCallbackStruct *) call_data;

    if (iscoll > 1) {
        if (face_open)
          TT_Close_Face(face);
        if (TT_Open_Collection(engine, filepath, cb->item_position - 1,
                               &face)) {
            face_open = 0;
            XmStringGetLtoR(cb->item, XmFONTLIST_DEFAULT_TAG, &fname);
            sprintf(name, "Unable to open font \"%s\"", fname);
            ErrorDialog(name);
            XtFree(fname);
            return;
        } else
          face_open = 1;

        /*
         * Get the properties for the face so it is current if the
         * font is selected to be opened.
         */
        (void) TT_Get_Face_Properties(face, &properties);
    }

    /*
     * Collect the list of platform IDs and put their names in the platform
     * list.
     */
    nplatforms = 0;
    ncmaps = TT_Get_CharMap_Count(face);
    for (lastpid = -1, i = 0; i < ncmaps; i++) {
        TT_Get_CharMap_ID(face, i, &pid, &eid);
        if (pid != lastpid) {
            /*
             * Choose a platform name.
             */
            s = XmStringCreateSimple(bdfttf_platform_name(pid));
            XmListAddItemUnselected(ttfd.platforms, s, 0);
            XmStringFree(s);
            platforms[nplatforms++] = pid;
            lastpid = pid;
        }
    }
}

static void
#ifndef _NO_PROTO
EnableOKButton(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnableOKButton(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XtSetSensitive(ttfd.ok, True);
}

static void
#ifndef _NO_PROTO
SetAction(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetAction(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    select_done = True;
    open_font = ((unsigned long) client_data == 1) ? True : False;
}

static void
#ifdef __STDC__
BuildTTFDialog(void)
#else
BuildTTFDialog()
#endif
{
    Widget pb, form, frame, framel, framer, label;
    Cardinal ac;
    Arg av[10];

    sprintf(title, "%s: TrueType Select", app_name);

    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    ttfd.shell = XtCreatePopupShell("xmbdfed_ttfd_shell",
                                    xmDialogShellWidgetClass, top, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    XtSetArg(av[ac], XmNfractionBase, 2); ac++;
    ttfd.form = XtCreateWidget("xmbdfed_ttfd_form", xmFormWidgetClass,
                               ttfd.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_ttfd_name_frame",
                                  xmFrameWidgetClass, ttfd.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Fonts", xmLabelWidgetClass, frame,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleItemCount, 4); ac++;
    XtSetArg(av[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); ac++;
    ttfd.names = XmCreateScrolledList(frame, "xmbdfed_ttfd_fontlist",
                                      av, ac);
    XtManageChild(ttfd.names);
    XtAddCallback(ttfd.names, XmNbrowseSelectionCallback, SelectFont, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    framel = XtCreateManagedWidget("xmbdfed_ttfd_platform_frame",
                                   xmFrameWidgetClass, ttfd.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Platform", xmLabelWidgetClass, framel,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleItemCount, 4); ac++;
    XtSetArg(av[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); ac++;
    ttfd.platforms = XmCreateScrolledList(framel, "xmbdfed_ttfd_platformlist",
                                          av, ac);
    XtManageChild(ttfd.platforms);
    XtAddCallback(ttfd.platforms, XmNbrowseSelectionCallback,
                  SelectPlatform, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    framer = XtCreateManagedWidget("xmbdfed_ttfd_encoding_frame",
                                   xmFrameWidgetClass, ttfd.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Encoding", xmLabelWidgetClass, framer,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNvisibleItemCount, 4); ac++;
    XtSetArg(av[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); ac++;
    ttfd.encodings = XmCreateScrolledList(framer, "xmbdfed_ttfd_encodinglist",
                                          av, ac);
    XtManageChild(ttfd.encodings);
    XtAddCallback(ttfd.encodings, XmNbrowseSelectionCallback, EnableOKButton,
                  0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, framel); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    frame = XtCreateManagedWidget("xmbdfed_ttfd_button_frame",
                                  xmFrameWidgetClass, ttfd.form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 5); ac++;
    form = XtCreateManagedWidget("xmbdfed_ttfd_button_form",
                                 xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    ttfd.ok = XtCreateManagedWidget("Ok", xmPushButtonWidgetClass, form,
                                    av, ac);
    XtAddCallback(ttfd.ok, XmNactivateCallback, SetAction, (XtPointer) 1);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 3); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 4); ac++;
    pb = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, SetAction, (XtPointer) 0);
}

int
#ifdef __STDC__
LoadTrueTypeFont(char **path, MXFEditor *ed, bdf_font_t **font)
#else
LoadTrueTypeFont(path, ed, font)
char **path;
MXFEditor *ed;
bdf_font_t **font;
#endif
{
    XmString s;
    short pid, eid;
    int i, plen, res;
    bdf_property_t prop;
    char *pp, *file, *ext, name[256];

    /*
     * Get the file name part of the path.
     */
    pp = *path;
    plen = strlen(pp);
    if ((file = strrchr(pp, '/')) == 0)
      file = pp;
    else
      file++;

    /*
     * Get the extension of the file to determine if this is a TT collection
     * or a single font.
     */
    iscoll = 0;
    if ((ext = strrchr(file, '.')) != 0) {
        while (*(ext + 1) != 0)
          ext++;
        iscoll = (*ext == 'c' || *ext == 'C') ? 1 : 0;

        /*
         * Reset the extension so it can be used later.
         */
        ext = strrchr(file, '.');
    }
        
    /*
     * Initialize the FreeType engine once per session.
     */
    if (!ftinit) {
        if (TT_Init_FreeType(&engine) != 0 ||
            TT_Init_SBit_Extension(engine) != 0) {
            ErrorDialog("Unable to initialize the FreeType engine.");
            return 0;
        }
        ftinit = 1;
    }

    /*
     * Attempt to open the font or collection.
     */
    if (TT_Open_Face(engine, pp, &face)) {
        if (!iscoll)
          sprintf(name, "Unable to open TrueType font '%s'.", file);
        else
          sprintf(name, "Unable to open TrueType collection '%s'.", file);
        ErrorDialog(name);
        return 0;
    }

    /*
     * Get the properties for the font.
     */
    if (TT_Get_Face_Properties(face, &properties)) {
        TT_Close_Face(face);
        sprintf(name, "Unable to get properties for '%s'.", file);
        ErrorDialog(name);
        return 0;
    }

    /*
     * Create the dialog that lets the user choose the fonts from a
     * collection, the platform, and encoding.
     */
    if (ttfd.shell == 0)
      BuildTTFDialog();

    /*
     * Clear the lists and reset the OK button.
     */
    XmListDeleteAllItems(ttfd.names);
    XmListDeleteAllItems(ttfd.platforms);
    XmListDeleteAllItems(ttfd.encodings);
    XtSetSensitive(ttfd.ok, False);

    /*
     * Collect the names from the fonts and set them in the font list.
     */
    face_open = 1;
    iscoll = properties.num_Faces + 1;
    if (iscoll == 1) {
        if (bdfttf_get_english_string(face, BDFTTF_FULLNAME_STRING,
                                      0, name) == 0)
          (void) strcpy(name, "Unknown");
        s = XmStringCreateSimple(name);
        XmListAddItemUnselected(ttfd.names, s, 0);
        XmStringFree(s);
    } else {
        /*
         * Get the names from each of the fonts in a collection.
         */
        face_open = 0;
        TT_Close_Face(face);
        for (i = 0; i < iscoll; i++) {
            if (!TT_Open_Collection(engine, pp, i, &face)) {
                if (bdfttf_get_english_string(face, BDFTTF_FULLNAME_STRING,
                                              0, name) == 0)
                  sprintf(name, "Unknown%d", i);
                s = XmStringCreateSimple(name);
                XmListAddItemUnselected(ttfd.names, s, 0);
                XmStringFree(s);
                TT_Close_Face(face);
            }
        }
    }

    /*
     * Copy the path name.
     */
    if (filepath != 0)
      XtFree(filepath);
    filepath = XtMalloc(strlen(pp) + 1);
    (void) strcpy(filepath, pp);

    /*
     * Force the first item in the name list to be selected so the platforms
     * and encodings will be loaded.
     */
    XmListSelectPos(ttfd.names, 1, True);

    XtManageChild(ttfd.form);
    XtPopup(ttfd.shell, XtGrabNone);

    /*
     * Wait for the user to select the font or cancel.
     */
    open_font = select_done = False;
    while (select_done == False)
      XtAppProcessEvent(app, XtIMAll);

    /*
     * At this point the `filepath' variable is no longer needed, so free it.
     */
    XtFree(filepath);
    filepath = 0;

    if (open_font == False) {
        if (face_open)
          TT_Close_Face(face);
        face_open = 0;
        XtPopdown(ttfd.shell);
        return 0;
    }

    /*
     * Determine which platform and encoding IDs have been selected.
     */
    for (i = 0; i < nplatforms; i++) {
        if (XmListPosSelected(ttfd.platforms, i + 1)) {
            pid = platforms[i];
            break;
        }
    }
    for (i = 0; i < nencodings; i++) {
        if (XmListPosSelected(ttfd.encodings, i + 1)) {
            eid = encodings[i];
            break;
        }
    }
    if (xmbdfed_opts.progbar)
      res = bdfttf_load_font(face, &properties, pid, eid,
                             &xmbdfed_opts.font_opts, UpdateProgressBar,
                             (void *) file, font);
    else {
        WatchCursor(ed->fgrid, True);
        res = bdfttf_load_font(face, &properties, pid, eid,
                               &xmbdfed_opts.font_opts, 0, 0, font);
        WatchCursor(ed->fgrid, False);
    }

    /*
     * Close the typeface.
     */
    TT_Close_Face(face);
    face_open = 0;

    if (res) {
        /*
         * The last thing that needs to be done if the font loaded
         * successfully, is add the _TTF_FONTFILE property using the
         * filename and generate the filename.
         */
        prop.name = "_TTF_FONTFILE";
        prop.format = BDF_ATOM;
        prop.value.atom = file;
        bdf_add_font_property(*font, &prop);

        /*
         * Generate a new pathname by replacing the extension with ".bdf".
         */
        if (ext == 0) {
            /*
             * If there was no extension for some odd reason, reallocate
             * enough space to add the ".bdf" extension.
             */
            plen = strlen(pp);
            *path = pp = XtRealloc(pp, plen + 5);
            ext = pp + plen;
        }
        (void) strcpy(ext, ".bdf");
    }

    XtPopdown(ttfd.shell);

    return res;
}

#endif /* HAVE_FREETYPE */

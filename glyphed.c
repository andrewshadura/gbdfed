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
static char rcsid[] __attribute__ ((unused)) = "$Id: glyphed.c,v 1.18 2004/02/08 23:59:00 mleisher Exp $";
#else
static char rcsid[] = "$Id: glyphed.c,v 1.18 2004/02/08 23:59:00 mleisher Exp $";
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
#include <Xm/List.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/TextF.h>
#include <Xm/MessageB.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include "FGrid.h"
#include "GEdit.h"
#include "GEditTB.h"
#include "xmbdfed.h"
#include "bdfP.h"

static Widget xbmd;
static MXFEditorGlyphEdit *glyph_editors;
static unsigned long num_glyph_editors;
static unsigned long active_gedit;

/*
 * Data for the pixmaps below.
 */
#define lbearing_width 32
#define lbearing_height 32
static unsigned char lbearing_bits[] = {
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x86, 0x19, 0x00, 0x00,
   0xff, 0x1b, 0x00, 0x00, 0x86, 0x01, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x33, 0xdb, 0xcc, 0xcc, 0x33, 0xdb, 0xcc, 0xcc,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00};

#define rbearing_width 32
#define rbearing_height 32
static unsigned char rbearing_bits[] = {
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0x98, 0x01, 0x60,
   0x00, 0xd8, 0xff, 0xff, 0x00, 0x80, 0x01, 0x60, 0x00, 0x00, 0x01, 0x20,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x33, 0xdb, 0xcc, 0xcc, 0x33, 0xdb, 0xcc, 0xcc,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00};

#define ascent_width 32
#define ascent_height 32
static unsigned char ascent_bits[] = {
   0x00, 0x18, 0x40, 0x00, 0x00, 0x18, 0xe0, 0x00, 0x00, 0x00, 0xf0, 0x01,
   0x00, 0x00, 0x40, 0x00, 0x00, 0x18, 0x40, 0x00, 0x00, 0x18, 0x40, 0x00,
   0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x18, 0x40, 0x00,
   0x00, 0x18, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00,
   0x00, 0x18, 0x40, 0x00, 0x00, 0x18, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00,
   0x00, 0x00, 0xf0, 0x01, 0x00, 0x18, 0xe0, 0x00, 0x00, 0x18, 0x40, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x33, 0xdb, 0xcc, 0xcc, 0x33, 0xdb, 0xcc, 0xcc,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00};

#define descent_width 32
#define descent_height 32
static unsigned char descent_bits[] = {
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x33, 0xdb, 0xcc, 0xcc, 0x33, 0xdb, 0xcc, 0xcc,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x40, 0x00, 0x00, 0x18, 0xe0, 0x00,
   0x00, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x40, 0x00, 0x00, 0x18, 0x40, 0x00,
   0x00, 0x18, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0xf0, 0x01,
   0x00, 0x18, 0xe0, 0x00, 0x00, 0x18, 0x40, 0x00};

/*
 * Pixmaps used for the resize and character cell dialogs.
 */
Pixmap lbearing_image = 0;
Pixmap rbearing_image = 0;
Pixmap ascent_image = 0;
Pixmap descent_image = 0;

static Boolean have_pixmaps = False;

#define _XMBDFED_XBM_READ  1
#define _XMBDFED_XBM_WRITE 2

static int xbm_action;

static MXFEditorGlyphEdit *
#ifndef _NO_PROTO
GetGlyphEditor(unsigned long owner)
#else
GetGlyphEditor(owner)
unsigned long owner;
#endif
{
    unsigned long i;
    MXFEditorGlyphEdit *ge;

    if (num_glyph_editors == 0) {
        glyph_editors = ge =
            (MXFEditorGlyphEdit *) XtCalloc(1, sizeof(MXFEditorGlyphEdit));
        ge->id = num_glyph_editors++;
    } else {
        for (i = 0; i < num_glyph_editors; i++) {
            if (glyph_editors[i].owner == ~0) {
                ge = &glyph_editors[i];
                ge->owner = owner;
                return ge;
            }
        }
        glyph_editors =
            (MXFEditorGlyphEdit *) XtRealloc((char *) glyph_editors,
                                             sizeof(MXFEditorGlyphEdit) *
                                             (num_glyph_editors + 1));
        ge = glyph_editors + num_glyph_editors;
        (void) memset((char *) ge, 0, sizeof(MXFEditorGlyphEdit));
        ge->id = num_glyph_editors++;
    }
    ge->owner = owner;
    return ge;
}

static void
#ifndef _NO_PROTO
DoFontUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    bdf_glyph_t *glyph;
    char *s;
    bdf_metrics_t metrics;
    int toggle;
    Boolean unencoded;

    ge = &glyph_editors[(unsigned long) client_data];
    ed = &editors[ge->owner];

    /*
     * Only do this if the glyph was actually modified.
     */
    if (XmuttGlyphEditModified(ge->gedit)) {
        /*
         * If the glyph editor has a selection, change back to the drawing
         * operation so the selection will be bound before saving.
         */
        if (XmuttGlyphEditHasSelection(ge->gedit) == True) {
            toggle = XmuttGlyphEditToolboxQueryToggle(ge->tbox);
            if (toggle != XmuttGLYPHEDIT_DRAW) {
                XmuttGlyphEditToolboxSetToggle(ge->tbox, XmuttGLYPHEDIT_DRAW,
                                               True);
                XmuttGlyphEditToolboxSetToggle(ge->tbox, toggle, True);
            }
        }

        glyph = XmuttGlyphEditGlyph(ge->gedit, &unencoded);

        /*
         * Set the new name and device width for the glyph.
         */
        if (glyph->name != 0)
          free(glyph->name);
        glyph->name = XmTextFieldGetString(ge->name);

        s = XmTextFieldGetString(ge->dwidth);
        glyph->dwidth = (unsigned short) _bdf_atos(s, 0, 10);
        XtFree(s);

        XmuttGlyphEditMetrics(ge->gedit, &metrics, True);
        XmuttFontGridUpdateMetrics(ed->fgrid, &metrics);

        XmuttFontGridUpdateFont(ed->fgrid, glyph, unencoded);

        if (glyph->name != 0)
          XtFree(glyph->name);
        if (glyph->bytes > 0)
          free((char *) glyph->bitmap);
        free((char *) glyph);

        /*
         * Mark everything as being updated and fix up the update button.
         */
        XmuttGlyphEditSetModified(ge->gedit, False);
    } else if (ge->psf.modified == True)
      /*
       * The only thing that changed was the PSF Unicode mappings.
       */
      XmuttFontGridUpdatePSFMappings(ed->fgrid,
                                     XmuttGlyphEditEncoding(ge->gedit),
                                     XmuttGlyphEditPSFMappings(ge->gedit));

    /*
     * The mappings get updated if they changed, even if nothing else
     * about this glyph changed.
     */
    ge->psf.modified = False;

    XtSetSensitive(ge->update, False);
    XtSetSensitive(ge->update_next, False);
    XtSetSensitive(ge->update_prev, False);
}

static void
#ifndef _NO_PROTO
ImportExportXBM(Widget w, XtPointer client_data, XtPointer call_data)
#else
ImportExportXBM(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    XmFileSelectionBoxCallbackStruct *cb;
    FILE *f;
    char *path, *pp;

    ge = &glyph_editors[active_gedit];
    ed = &editors[ge->owner];

    cb = (XmFileSelectionBoxCallbackStruct *) call_data;
    XmStringGetLtoR(cb->value, XmSTRING_DEFAULT_CHARSET, &path);

    /*
     * Point at the filename part so it can be used in messages.
     */
    if ((pp = strrchr(path, '/')) != 0)
      pp++;
    else
      pp = path;;

    if (xbm_action == _XMBDFED_XBM_READ) {
        /*
         * Load an XBM file.
         */
        if (XmuttGlyphEditImportXBM(ge->gedit, path) != BitmapSuccess) {
            sprintf(name, "Problem importing XBM '%s'.", pp);
            ErrorDialog(name);
            XtFree(path);
            return;
        }
    } else {
        /*
         * Save an XBM file.
         */

        /*
         * First, test to see if the xbm file already exists.
         */
        if ((f = fopen(path, "r")) != 0) {
            fclose(f);
            sprintf(name, "File '%s' already exists.  Overwrite?", pp);
            if (AskBooleanQuestion(name, "Yes", "No") == False) {
                XtFree(path);
                return;
            }
        }
        if (XmuttGlyphEditExportXBM(ge->gedit, path) != BitmapSuccess) {
            sprintf(title, "Problem exporting XBM '%s'.", pp);
            ErrorDialog(title);
            XtFree(path);
            return;
        }
    }

    /*
     * Free the path.
     */
    XtFree(path);

    /*
     * Hide the file selection dialog.
     */
    XtPopdown(XtParent(xbmd));

}

static void
#ifndef _NO_PROTO
DoImportXBM(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoImportXBM(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    XmString s;
    Arg av[2];

    ge = &glyph_editors[(unsigned long) client_data];
    ed = &editors[ge->owner];

    if (xbmd == 0) {
        sprintf(title, "%s: Import X Bitmap", app_name);
        s = XmStringCreateSimple("*.[Xx][Bb][Mm]");
        XtSetArg(av[0], XmNtitle, title);
        XtSetArg(av[1], XmNpattern, s);
        sprintf(name, "xmbdfed_glyphedit%ld_insert_xbm_sel_box", ge->id);
        xbmd = XmCreateFileSelectionDialog(top, name, av, 2);
        XmStringFree(s);
        XtAddCallback(xbmd, XmNokCallback, ImportExportXBM, 0);
        XtAddCallback(xbmd, XmNcancelCallback, DoClose,
                      (XtPointer) XtParent(xbmd));
    } else {
        sprintf(title, "%s: Import X Bitmap", app_name);
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(XtParent(xbmd), av, 1);
    }

    xbm_action = _XMBDFED_XBM_READ;
    active_gedit = ge->id;
    XtManageChild(xbmd);
    XtPopup(XtParent(xbmd), XtGrabNone);
}

static void
#ifndef _NO_PROTO
DoExportXBM(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoExportXBM(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    XmString s;
    Arg av[2];

    ge = &glyph_editors[(unsigned long) client_data];
    ed = &editors[ge->owner];

    if (xbmd == 0) {
        sprintf(title, "%s: Export X Bitmap", app_name);
        s = XmStringCreateSimple("*.[Xx][Bb][Mm]");
        XtSetArg(av[0], XmNtitle, title);
        XtSetArg(av[1], XmNpattern, s);
        sprintf(name, "xmbdfed_glyphedit%ld_xbm_sel_box", ge->id);
        xbmd = XmCreateFileSelectionDialog(top, name, av, 2);
        XmStringFree(s);
        XtAddCallback(xbmd, XmNokCallback, ImportExportXBM, 0);
        XtAddCallback(xbmd, XmNcancelCallback, DoClose,
                      (XtPointer) XtParent(xbmd));
    } else {
        sprintf(title, "%s: Export X Bitmap", app_name);
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(XtParent(xbmd), av, 1);
    }

    xbm_action = _XMBDFED_XBM_WRITE;
    active_gedit = ge->id;
    XtManageChild(xbmd);
    XtPopup(XtParent(xbmd), XtGrabNone);
}

static void
#ifndef _NO_PROTO
RotateGlyph(Widget w, XtPointer client_data, XtPointer call_data)
#else
RotateGlyph(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;
    XmuttOperationCallbackStruct *cb;

    ge = &glyph_editors[(unsigned long) client_data];
    cb = (XmuttOperationCallbackStruct *) call_data;

    XmuttGlyphEditRotate(ge->gedit, cb->angle);
}

static void
#ifndef _NO_PROTO
ShearGlyph(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShearGlyph(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;
    XmuttOperationCallbackStruct *cb;

    ge = &glyph_editors[(unsigned long) client_data];
    cb = (XmuttOperationCallbackStruct *) call_data;

    XmuttGlyphEditShear(ge->gedit, cb->angle);
}

static void
#ifndef _NO_PROTO
EmboldenGlyph(Widget w, XtPointer client_data, XtPointer call_data)
#else
EmboldenGlyph(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];

    XmuttGlyphEditEmbolden(ge->gedit);
}

static void
#ifndef _NO_PROTO
ShowRotate(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowRotate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ShowRotateDialog(RotateGlyph, client_data, False);
}

static void
#ifndef _NO_PROTO
ShowShear(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowShear(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    ShowShearDialog(ShearGlyph, client_data, False);
}

#define UPMSG "The glyph has been modified.\nDo you want to update?"

static void
#ifndef _NO_PROTO
DoCloseGlyphEditor(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCloseGlyphEditor(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];

    /*
     * Simply return if it has already been popped down.
     */
    if (ge->owner == ~0)
      return;

    /*
     * Check to see if the glyph has been modified or not and ask if it should
     * be updated.
     */
    if (XtIsSensitive(ge->update) == True) {
        if (AskBooleanQuestion(UPMSG , "Yes", "No") == True)
          DoFontUpdate(w, client_data, call_data);
    }

    ge->owner = ~0;
    if (ge->resize.visible == True) {
        XtPopdown(ge->resize.shell);
        ge->resize.visible = False;
    }
    XtPopdown(ge->shell);
}

static void
#ifndef _NO_PROTO
ApplyResize(Widget w, XtPointer client_data, XtPointer call_data)
#else
ApplyResize(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;
    bdf_metrics_t metrics;
    short lb, rb, as, ds;
    char *s;

    ge = &glyph_editors[(unsigned long) client_data];

    /*
     * Pop the dialog box down.
     */
    ge->resize.visible = False;
    XtPopdown(ge->resize.shell);

    /*
     * Get the left bearing.
     */
    s = XmTextFieldGetString(ge->resize.lbearing);
    lb = _bdf_atos(s, 0, 10) * -1;
    XtFree(s);

    /*
     * Get the right bearing.
     */
    s = XmTextFieldGetString(ge->resize.rbearing);
    rb = _bdf_atos(s, 0, 10);
    XtFree(s);

    /*
     * Get the ascent.
     */
    s = XmTextFieldGetString(ge->resize.ascent);
    as = _bdf_atos(s, 0, 10);
    XtFree(s);

    /*
     * Get the descent.
     */
    s = XmTextFieldGetString(ge->resize.descent);
    ds = _bdf_atos(s, 0, 10);
    XtFree(s);

    /*
     * Determine the width and the X offset.
     */
    metrics.width = rb - lb;
    metrics.x_offset = lb;

    /*
     * Determine the height and the Y offset.
     */
    metrics.ascent = as;
    metrics.descent = ds;
    metrics.height = as + ds;
    metrics.y_offset = -ds;

    /*
     * Cause the glyph editor to change things.
     */
    XmuttGlyphEditSetMetrics(ge->gedit, &metrics);
}

static void
#ifndef _NO_PROTO
EnableApplyButton(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnableApplyButton(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    /*
     * Add a routine to verify that each item entered is a digit or a minus
     * sign if the widget is empty.
     */

    ge = &glyph_editors[(unsigned long) client_data];
    XtSetSensitive(ge->resize.apply, True);
}

static void
#ifndef _NO_PROTO
DoResize(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoResize(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    Widget label, frame, form, pb;
    MXFEditorGlyphEdit *ge;
    bdf_metrics_t metrics;
    Cardinal ac;
    Arg av[6];

    ge = &glyph_editors[(unsigned long) client_data];

    if (ge->resize.shell == 0) {
        ac = 0;
        sprintf(title, "%s - Glyph Edit Resize BBX", app_name);
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetArg(av[ac], XmNallowShellResize, True); ac++;
        XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize", ge->id);
        ge->resize.shell = XtCreatePopupShell(name, xmDialogShellWidgetClass,
                                              ge->shell, av, ac);

        /*
         * Create the primary form.
         */
        ac = 0;
        XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
        XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
        XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_form", ge->id);
        ge->resize.dialog = XtCreateWidget(name, xmFormWidgetClass,
                                           ge->resize.shell, av, ac);

        /*
         * Create the frame for the resize width factors.
         */
        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_width_frame", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->resize.dialog, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("Width", xmLabelWidgetClass,
                                      frame, av, ac);

        sprintf(name, "xmbdfed_glyphedit%ld_resize_width_form", ge->id);
        form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, 0, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNlabelType, XmPIXMAP); ac++;
        XtSetArg(av[ac], XmNlabelPixmap, lbearing_image); ac++;
        label = XtCreateManagedWidget("LBearing", xmLabelWidgetClass, form,
                                      av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, label); ac++;
        XtSetArg(av[ac], XmNcolumns, 6); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_lbearing", ge->id);
        ge->resize.lbearing = XtCreateManagedWidget(name,
                                                    xmTextFieldWidgetClass,
                                                    form, av, ac);
        XtAddCallback(ge->resize.lbearing, XmNmodifyVerifyCallback,
                      EnableApplyButton, (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, ge->resize.lbearing); ac++;
        XtSetArg(av[ac], XmNlabelType, XmPIXMAP); ac++;
        XtSetArg(av[ac], XmNlabelPixmap, rbearing_image); ac++;
        label = XtCreateManagedWidget("RBearing", xmLabelWidgetClass, form,
                                      av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, label); ac++;
        XtSetArg(av[ac], XmNcolumns, 6); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_rbearing", ge->id);
        ge->resize.rbearing = XtCreateManagedWidget(name,
                                                    xmTextFieldWidgetClass,
                                                    form, av, ac);
        XtAddCallback(ge->resize.rbearing, XmNmodifyVerifyCallback,
                      EnableApplyButton, (XtPointer) ge->id);

        /*
         * Create the frame for the resize height factors.
         */
        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_height_frame", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->resize.dialog, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("Height", xmLabelWidgetClass, frame,
                                      av, ac);

        sprintf(name, "xmbdfed_glyphedit%ld_resize_height_form", ge->id);
        form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, 0, 0);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNlabelType, XmPIXMAP); ac++;
        XtSetArg(av[ac], XmNlabelPixmap, ascent_image); ac++;
        label = XtCreateManagedWidget("Ascent", xmLabelWidgetClass, form,
                                      av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, label); ac++;
        XtSetArg(av[ac], XmNcolumns, 6); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_ascent", ge->id);
        ge->resize.ascent = XtCreateManagedWidget(name, xmTextFieldWidgetClass,
                                                  form, av, ac);
        XtAddCallback(ge->resize.ascent, XmNmodifyVerifyCallback,
                      EnableApplyButton, (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, ge->resize.ascent); ac++;
        XtSetArg(av[ac], XmNlabelType, XmPIXMAP); ac++;
        XtSetArg(av[ac], XmNlabelPixmap, descent_image); ac++;
        label = XtCreateManagedWidget("Descent", xmLabelWidgetClass, form,
                                      av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNleftWidget, label); ac++;
        XtSetArg(av[ac], XmNcolumns, 6); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_descent", ge->id);
        ge->resize.descent = XtCreateManagedWidget(name,
                                                   xmTextFieldWidgetClass,
                                                   form, av, ac);
        XtAddCallback(ge->resize.descent, XmNmodifyVerifyCallback,
                      EnableApplyButton, (XtPointer) ge->id);

        /*
         * Now add the two buttons at the bottom.
         */
        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_button_frame", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->resize.dialog, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNfractionBase, 5); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_resize_button_form", ge->id);
        form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 1); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 2); ac++;
        ge->resize.apply =
            XtCreateManagedWidget("Apply", xmPushButtonWidgetClass, form,
                                  av, ac);
        XtAddCallback(ge->resize.apply, XmNactivateCallback, ApplyResize,
                      (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 3); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 4); ac++;
        pb = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, form,
                                   av, ac);
        XtAddCallback(pb, XmNactivateCallback, DoClose,
                      (XtPointer) ge->resize.shell);
    }

    /*
     * Get the current font metrics.
     */
    XmuttGlyphEditMetrics(ge->gedit, &metrics, True);

    XtSetSensitive(ge->resize.lbearing, True);
    sprintf(name, "%hd", metrics.x_offset * -1);
    XmTextFieldSetString(ge->resize.lbearing, name);
    if (metrics.font_spacing == BDF_CHARCELL)
      XtSetSensitive(ge->resize.lbearing, False);

    sprintf(name, "%hd", metrics.width + metrics.x_offset);
    XmTextFieldSetString(ge->resize.rbearing, name);

    sprintf(name, "%hd", metrics.ascent);
    XmTextFieldSetString(ge->resize.ascent, name);

    sprintf(name, "%hd", metrics.descent);
    XmTextFieldSetString(ge->resize.descent, name);

    XtSetSensitive(ge->resize.apply, False);

    XtManageChild(ge->resize.dialog);
    ge->resize.visible = True;
    XtPopup(ge->resize.shell, XtGrabNone);
}

static void
#ifndef _NO_PROTO
DoCopy(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCopy(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XmuttGlyphEditCopySelection(ge->gedit);
}

static void
#ifndef _NO_PROTO
DoCut(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCut(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XmuttGlyphEditCutSelection(ge->gedit);
}

static void
#ifndef _NO_PROTO
DoPaste(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoPaste(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XmuttGlyphEditPasteSelection(ge->gedit);
}

static void
#ifndef _NO_PROTO
DoSelectAll(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoSelectAll(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XmuttGlyphEditSelectAll(ge->gedit);
}

static void
#ifndef _NO_PROTO
DoNextGlyph(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoNextGlyph(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int base;
    XmString s;
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    bdf_glyph_grid_t *grid;
    bdf_font_t *font;
    bdf_bitmap_t image;
    Arg av[1];

    ge = &glyph_editors[(unsigned long) client_data];
    ed = &editors[ge->owner];
    grid = XmuttGlyphEditGrid(ge->gedit);
    font = XmuttFontGridFont(ed->fgrid);
    base = XmuttFontGridCodeBase(ed->fgrid);

    /*
     * Check to see if the glyph has been modified or not and ask if it should
     * be updated.
     */
    if (XtIsSensitive(ge->update) == True &&
        AskBooleanQuestion(UPMSG , "Yes", "No") == True)
      DoFontUpdate(w, client_data, call_data);

    /*
     * Ring the bell and return if this is the last glyph.
     */
    if (grid->encoding == 0xffff) {
        XBell(XtDisplay(w), 0);
        return;
    }

    if (grid->unencoded)
      grid = bdf_make_glyph_grid(font, grid->encoding + 1, 1);
    else
      grid = bdf_make_glyph_grid(font, grid->encoding + 1, 0);

    /*
     * Set the glyph info on the glyph editor.
     */
    XmTextFieldSetString(ge->name, grid->name);
    XmTextFieldSetCursorPosition(ge->name, strlen(grid->name));

    if (grid->unencoded)
      sprintf(name, "-1");
    else {
        switch (base) {
          case 8: sprintf(name, "%lo", grid->encoding); break;
          case 10: sprintf(name, "%ld", grid->encoding); break;
          case 16: sprintf(name, "%04lX", grid->encoding); break;
        }
    }
    s = XmStringCreateSimple(name);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->encoding, av, 1);
    XmStringFree(s);

    sprintf(name, "%hd", grid->dwidth);
    XtSetSensitive(ge->dwidth, True);
    XmTextFieldSetString(ge->dwidth, name);
    XmTextFieldSetCursorPosition(ge->dwidth, 4);
    if (grid->spacing != BDF_PROPORTIONAL)
      XtSetSensitive(ge->dwidth, False);

    sprintf(name, "width %hd height %hd\nascent %hd descent %hd",
            grid->glyph_bbx.width, grid->glyph_bbx.height,
            grid->glyph_bbx.ascent, grid->glyph_bbx.descent);
    s = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->metrics, av, 1);
    XmStringFree(s);

    /*
     * Set the new glyph grid.
     */
    XtSetArg(av[0], XmNglyphGrid, grid);
    XtSetValues(ge->gedit, av, 1);

    /*
     * Get the grid image so the toolbox image can be updated.
     */
    XmuttGlyphEditImage(ge->gedit, &image);
    XtSetArg(av[0], XmNglyphImage, &image);
    XtSetValues(ge->tbox, av, 1);
    if (image.bytes > 0)
      free((char *) image.bitmap);

    /*
     * Change the sensitivity of the update menu item.
     */
    if (grid->modified) {
        XtSetSensitive(ge->update, True);
        XtSetSensitive(ge->update_next, True);
        XtSetSensitive(ge->update_prev, True);
    } else {
        XtSetSensitive(ge->update, False);
        XtSetSensitive(ge->update_next, False);
        XtSetSensitive(ge->update_prev, False);
    }
}

static void
#ifndef _NO_PROTO
DoPrevGlyph(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoPrevGlyph(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int base;
    XmString s;
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    bdf_glyph_grid_t *grid;
    bdf_font_t *font;
    bdf_bitmap_t image;
    Arg av[1];

    ge = &glyph_editors[(unsigned long) client_data];
    ed = &editors[ge->owner];
    grid = XmuttGlyphEditGrid(ge->gedit);
    font = XmuttFontGridFont(ed->fgrid);
    base = XmuttFontGridCodeBase(ed->fgrid);

    /*
     * Check to see if the glyph has been modified or not and ask if it should
     * be updated.
     */
    if (XtIsSensitive(ge->update) == True &&
        AskBooleanQuestion(UPMSG , "Yes", "No") == True)
      DoFontUpdate(w, client_data, call_data);

    /*
     * Ring the bell and return if this is the last glyph.
     */
    if (grid->encoding == 0) {
        XBell(XtDisplay(w), 0);
        return;
    }

    if (grid->unencoded)
      grid = bdf_make_glyph_grid(font, grid->encoding - 1, 1);
    else
      grid = bdf_make_glyph_grid(font, grid->encoding - 1, 0);

    /*
     * Set the glyph info on the glyph editor.
     */
    XmTextFieldSetString(ge->name, grid->name);
    XmTextFieldSetCursorPosition(ge->name, strlen(grid->name));

    if (grid->unencoded)
      sprintf(name, "-1");
    else {
        switch (base) {
          case 8: sprintf(name, "%lo", grid->encoding); break;
          case 10: sprintf(name, "%ld", grid->encoding); break;
          case 16: sprintf(name, "%04lX", grid->encoding); break;
        }
    }
    s = XmStringCreateSimple(name);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->encoding, av, 1);
    XmStringFree(s);

    sprintf(name, "%hd", grid->dwidth);
    XtSetSensitive(ge->dwidth, True);
    XmTextFieldSetString(ge->dwidth, name);
    XmTextFieldSetCursorPosition(ge->dwidth, 4);
    if (grid->spacing != BDF_PROPORTIONAL)
      XtSetSensitive(ge->dwidth, False);

    sprintf(name, "width %hd height %hd\nascent %hd descent %hd",
            grid->glyph_bbx.width, grid->glyph_bbx.height,
            grid->glyph_bbx.ascent, grid->glyph_bbx.descent);
    s = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->metrics, av, 1);
    XmStringFree(s);

    /*
     * Set the new glyph grid.
     */
    XtSetArg(av[0], XmNglyphGrid, grid);
    XtSetValues(ge->gedit, av, 1);

    /*
     * Get the grid image so the toolbox image can be updated.
     */
    XmuttGlyphEditImage(ge->gedit, &image);
    XtSetArg(av[0], XmNglyphImage, &image);
    XtSetValues(ge->tbox, av, 1);
    if (image.bytes > 0)
      free((char *) image.bitmap);

    /*
     * Change the sensitivity of the update menu item.
     */
    if (grid->modified) {
        XtSetSensitive(ge->update, True);
        XtSetSensitive(ge->update_next, True);
        XtSetSensitive(ge->update_prev, True);
    } else {
        XtSetSensitive(ge->update, False);
        XtSetSensitive(ge->update_next, False);
        XtSetSensitive(ge->update_prev, False);
    }
}

static void
#ifndef _NO_PROTO
DoFontUpdateAndNext(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontUpdateAndNext(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    DoFontUpdate(w, client_data, call_data);
    DoNextGlyph(w, client_data, call_data);
}

static void
#ifndef _NO_PROTO
DoFontUpdateAndPrev(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontUpdateAndPrev(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    DoFontUpdate(w, client_data, call_data);
    DoPrevGlyph(w, client_data, call_data);
}

static void
#ifndef _NO_PROTO
EnablePSFButtons(Widget w, XtPointer client_data, XtPointer call_data)
#else
EnablePSFButtons(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge = &glyph_editors[(unsigned long) client_data];

    XtSetSensitive(ge->psf.del, True);
    XtSetSensitive(ge->psf.edit, True);
}

static void
#ifndef _NO_PROTO
UpdatePSFMappings(Widget w, XtPointer client_data, XtPointer call_data)
#else
UpdatePSFMappings(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge = &glyph_editors[(unsigned long) client_data];
    char *txt;
    XmString s;
    int *posns, nitems, pos;
    Arg av[2];

    txt = XmTextFieldGetString(ge->psf.editsel);

    s = XmStringCreateSimple(txt);
    XtFree(txt);

    XtSetArg(av[0], XmNitemCount, &nitems);
    XtSetArg(av[1], XmNselectedPositions, &posns);
    XtGetValues(ge->psf.mappings, av, 2);

    if (ge->psf.editing == False)
      /*
       * Mapping is being added.  Append to the end.
       */
      XmListAddItemUnselected(ge->psf.mappings, s, nitems + 1);
    else {
        /*
         * Mapping is being edited.
         */
        pos = posns[0];
        XmListReplacePositions(ge->psf.mappings, posns, &s, 1);
        XmListSelectPos(ge->psf.mappings, pos, False);
    }

    XmStringFree(s);

    XtSetSensitive(ge->psf.editapply, False);
    XtSetSensitive(ge->psf.apply, True);
}

static void
#ifndef _NO_PROTO
DeletePSFMapping(Widget w, XtPointer client_data, XtPointer call_data)
#else
DeletePSFMapping(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge = &glyph_editors[(unsigned long) client_data];
    int *pos;
    Arg av[1];

    XtSetArg(av[0], XmNselectedPositions, &pos);
    XtGetValues(ge->psf.mappings, av, 1);

    XmListDeletePos(ge->psf.mappings, pos[0]);
    XtSetSensitive(ge->psf.apply, True);

    ge->psf.modified = True;
}

static void
#ifndef _NO_PROTO
SetSensitive(Widget w, XtPointer client_data, XtPointer call_data)
#else
SetSensitive(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    XtSetSensitive((Widget) client_data, True);
}

static void
#ifndef _NO_PROTO
CreatePSFMapEdit(MXFEditorGlyphEdit *ge)
#else
CreatePSFMapEdit(ge)
MXFEditorGlyphEdit *ge;
#endif
{
    Widget frame, form, cancel, label;
    Cardinal ac;
    Arg av[8];

    if (ge->psf.editshell == 0) {
        ac = 0;
        sprintf(title, "Edit PSF Unicode Mapping");
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetArg(av[ac], XmNallowShellResize, True); ac++;
        XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_edit", ge->id);
        ge->psf.editshell = XtCreatePopupShell(name, xmDialogShellWidgetClass,
                                               ge->psf.shell, av, ac);

        ac = 0;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_edit_dialog", ge->id);
        ge->psf.editdialog = XtCreateWidget(name, xmFormWidgetClass,
                                            ge->psf.editshell, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_edit_frame1", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->psf.editdialog, av, ac);
        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        label = XtCreateManagedWidget("Unicode Mapping", xmLabelWidgetClass,
                                      frame, av, ac);

        ac = 0;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_edit_text", ge->id);
        ge->psf.editsel = XtCreateManagedWidget(name, xmTextFieldWidgetClass,
                                                frame, av, ac);
        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_edit_frame2", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->psf.editdialog, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNfractionBase, 5); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_edit_form",
                ge->id);
        form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 1); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 2); ac++;
        ge->psf.editapply = XtCreateManagedWidget("Apply",
                                                  xmPushButtonWidgetClass,
                                                  form, av, ac);
        XtAddCallback(ge->psf.editapply, XmNactivateCallback,
                      UpdatePSFMappings, (XtPointer) ge->id);

        XtAddCallback(ge->psf.editsel, XmNactivateCallback,
                      UpdatePSFMappings, (XtPointer) ge->id);

        /*
         * Add a callback to the text field to enable this button
         * when the user types something.
         */
        XtAddCallback(ge->psf.editsel, XmNvalueChangedCallback,
                      SetSensitive, (XtPointer) ge->psf.editapply);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 3); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 4); ac++;
        cancel = XtCreateManagedWidget("Cancel", xmPushButtonWidgetClass, form,
                                       av, ac);
        XtAddCallback(cancel, XmNactivateCallback, DoClose,
                      (XtPointer) ge->psf.editshell);
    }
}

static void
#ifndef _NO_PROTO
EditPSFMapping(Widget w, XtPointer client_data, XtPointer call_data)
#else
EditPSFMapping(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge = &glyph_editors[(unsigned long) client_data];
    XmStringTable items;
    char *mapping;
    Arg av[1];

    /*
     * Create the editor.
     */
    if (ge->psf.editshell == 0)
      CreatePSFMapEdit(ge);

    sprintf(title, "Edit PSF Unicode Mapping");
    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ge->psf.editshell, av, 1);

    /*
     * Get the selected item from the list.
     */
    XtSetArg(av[0], XmNselectedItems, &items);
    XtGetValues(ge->psf.mappings, av, 1);

    XmStringGetLtoR(items[0], XmSTRING_DEFAULT_CHARSET, &mapping);
    XmTextFieldSetString(ge->psf.editsel, mapping);
    XtFree(mapping);

    /*
     * Desensitize the Apply button until user makes changes.
     */
    XtSetSensitive(ge->psf.editapply, False);
    ge->psf.editing = True;

    XtManageChild(ge->psf.editdialog);
    XtPopup(ge->psf.editshell, XtGrabNone);
}

static void
#ifndef _NO_PROTO
AddPSFMapping(Widget w, XtPointer client_data, XtPointer call_data)
#else
AddPSFMapping(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge = &glyph_editors[(unsigned long) client_data];
    Arg av[1];

    /*
     * Create the editor.
     */
    if (ge->psf.editshell == 0)
      CreatePSFMapEdit(ge);

    sprintf(title, "Add PSF Unicode Mapping");
    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ge->psf.editshell, av, 1);

    XmTextFieldSetString(ge->psf.editsel, "");

    /*
     * Desensitize the Apply button until user makes changes.
     */
    XtSetSensitive(ge->psf.editapply, False);
    ge->psf.editing = False;

    XtManageChild(ge->psf.editdialog);
    XtPopup(ge->psf.editshell, XtGrabNone);
}

static void
#ifndef _NO_PROTO
ClosePSFMapEdit(Widget w, XtPointer client_data, XtPointer call_data)
#else
ClosePSFMapEdit(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge = &glyph_editors[(unsigned long) client_data];

    /*
     * Make sure the editing dialog is closed if it has been created.
     */
    if (ge->psf.editshell != 0)
      XtPopdown(ge->psf.editshell);

    ge->psf.visible = False;
    XtPopdown(ge->psf.shell);
}

static void
#ifndef _NO_PROTO
UpdateGlyphGridPSFMap(Widget w, XtPointer client_data, XtPointer call_data)
#else
UpdateGlyphGridPSFMap(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge = &glyph_editors[(unsigned long) client_data];
    bdf_glyph_grid_t *grid;
    XmStringTable m;
    int i, nm;
    char **list;
    Arg av[2];

    /*
     * Get the mappings from the list.
     */
    XtSetArg(av[0], XmNitems, &m);
    XtSetArg(av[1], XmNitemCount, &nm);
    XtGetValues(ge->psf.mappings, av, 2);

    list = (char **) XtMalloc(sizeof(char *) * nm);

    for (i = 0; i < nm; i++)
      XmStringGetLtoR(m[i], XmSTRING_DEFAULT_CHARSET, &list[i]);

    grid = XmuttGlyphEditGrid(ge->gedit);

    _bdf_psf_pack_mapping(list, nm, grid->encoding, &grid->unicode);

    for (i = 0; i < nm; i++)
      XtFree(list[i]);
    XtFree((char *) list);

    ge->psf.modified = True;

    XtSetSensitive(ge->psf.apply, False);
    XtSetSensitive(ge->update, True);

    ClosePSFMapEdit(w, (XtPointer) ge->id, 0);
}

static void
#ifndef _NO_PROTO
ShowPSFMapEditor(MXFEditorGlyphEdit *ge, char **mappings, int nmappings)
#else
ShowPSFMapEditor(ge, mappings, nmappings)
MXFEditorGlyphEdit *ge;
char **mappings;
int nmappings;
#endif
{
    MXFEditor *ed;
    bdf_glyph_grid_t *grid;
    Widget frame, form;
    int i, base;
    XmString s;
    XmStringTable list;
    Cardinal ac;
    Arg av[8];

    ed = &editors[ge->owner];
    base = XmuttFontGridCodeBase(ed->fgrid);
    grid = XmuttGlyphEditGrid(ge->gedit);

    if (ge->psf.shell == 0) {
        ac = 0;
        sprintf(title, "%s - Glyph Edit PSF Unicode Mappings", app_name);
        XtSetArg(av[ac], XmNtitle, title); ac++;
        XtSetArg(av[ac], XmNallowShellResize, True); ac++;
        XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list_shell", ge->id);
        ge->psf.shell = XtCreatePopupShell(name, xmDialogShellWidgetClass,
                                           ge->shell, av, ac);

        ac = 0;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list_dialog", ge->id);
        ge->psf.dialog = XtCreateWidget(name, xmFormWidgetClass,
                                        ge->psf.shell, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list_frame1", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->psf.dialog, av, ac);
        ac = 0;
        XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
        ge->psf.mlabel = XtCreateManagedWidget("Unicode Mappings",
                                               xmLabelWidgetClass, frame,
                                               av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNvisibleItemCount, 5); ac++;
        XtSetArg(av[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
        XtSetArg(av[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list", ge->id);
        ge->psf.mappings = XmCreateScrolledList(frame, name, av, ac);
        XtAddCallback(ge->psf.mappings, XmNbrowseSelectionCallback,
                      EnablePSFButtons, (XtPointer) ge->id);
        XtAddCallback(ge->psf.mappings, XmNdefaultActionCallback,
                      EditPSFMapping, (XtPointer) ge->id);
        XtManageChild(ge->psf.mappings);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list_frame2", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->psf.dialog, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNfractionBase, 3); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list_form1",
                ge->id);
        form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 0); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 1); ac++;
        ge->psf.edit = XtCreateManagedWidget("Edit", xmPushButtonWidgetClass,
                                             form, av, ac);
        XtAddCallback(ge->psf.edit, XmNactivateCallback, EditPSFMapping,
                      (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 1); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 2); ac++;
        ge->psf.add = XtCreateManagedWidget("New", xmPushButtonWidgetClass,
                                            form, av, ac);
        XtAddCallback(ge->psf.add, XmNactivateCallback, AddPSFMapping,
                      (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 2); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 3); ac++;
        ge->psf.del = XtCreateManagedWidget("Delete", xmPushButtonWidgetClass,
                                            form, av, ac);
        XtAddCallback(ge->psf.del, XmNactivateCallback, DeletePSFMapping,
                      (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
        XtSetArg(av[ac], XmNtopWidget, frame); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list_frame3", ge->id);
        frame = XtCreateManagedWidget(name, xmFrameWidgetClass,
                                      ge->psf.dialog, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNfractionBase, 3); ac++;
        sprintf(name, "xmbdfed_glyphedit%ld_psf_map_list_form2",
                ge->id);
        form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, av, ac);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 0); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 1); ac++;
        ge->psf.apply = XtCreateManagedWidget("Apply", xmPushButtonWidgetClass,
                                              form, av, ac);
        XtAddCallback(ge->psf.apply, XmNactivateCallback,
                      UpdateGlyphGridPSFMap, (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 1); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 2); ac++;
        ge->psf.cancel = XtCreateManagedWidget("Cancel",
                                               xmPushButtonWidgetClass, form,
                                               av, ac);
        XtAddCallback(ge->psf.cancel, XmNactivateCallback, ClosePSFMapEdit,
                      (XtPointer) ge->id);

        ac = 0;
        XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
        XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNleftPosition, 2); ac++;
        XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
        XtSetArg(av[ac], XmNrightPosition, 3); ac++;
        ge->psf.help =
            XtCreateManagedWidget("Help", xmPushButtonWidgetClass, form,
                                  av, ac);
    }

    /*
     * Update the dialog title.
     */
    switch (base) {
      case 8:
        sprintf(title, "%lo: Edit PSF Unicode Mappings", grid->encoding);
        break;
      case 10:
        sprintf(title, "%ld: Edit PSF Unicode Mappings", grid->encoding);
        break;
      case 16:
        sprintf(title, "%04lX: Edit PSF Unicode Mappings", grid->encoding);
        break;
    }
    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ge->psf.shell, av, 1);

    /*
     * Update the list frame label.
     */
    switch (base) {
      case 8:
        sprintf(title, "%lo: Unicode Mappings", grid->encoding);
        break;
      case 10:
        sprintf(title, "%ld: Unicode Mappings", grid->encoding);
        break;
      case 16:
        sprintf(title, "%04lX: Unicode Mappings", grid->encoding);
        break;
    }
    s = XmStringCreateSimple(title);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->psf.mlabel, av, 1);
    XmStringFree(s);

    /*
     * Add the mappings to the list.
     */
    if (nmappings > 0) {
        list = (XmStringTable) XtMalloc(sizeof(XmString) * nmappings);

        for (i = 0; i < nmappings; i++)
          list[i] = XmStringCreateSimple(mappings[i]);

        ac = 0;
        XtSetArg(av[ac], XmNitems, list); ac++;
        XtSetArg(av[ac], XmNitemCount, nmappings); ac++;
        XtSetValues(ge->psf.mappings, av, ac);

        for (i = 0; i < nmappings; i++)
          XmStringFree(list[i]);
        XtFree((char *) list);
    } else
      /*
       * Make sure the list is cleared out.
       */
      XmListDeleteAllItems(ge->psf.mappings);

    XtSetSensitive(ge->psf.edit, False);
    XtSetSensitive(ge->psf.del, False);
    XtSetSensitive(ge->psf.apply, False);

    ge->psf.modified = False;

    XtManageChild(ge->psf.dialog);
    ge->psf.visible = True;
    XtPopup(ge->psf.shell, XtGrabNone);
}

static void
#ifndef _NO_PROTO
EditPSFUnicodeMap(Widget w, XtPointer client_data, XtPointer call_data)
#else
EditPSFUnicodeMap(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int n;
    char **mappings;
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    bdf_glyph_grid_t *grid;
    bdf_font_t *font;

    ge = &glyph_editors[(unsigned long) client_data];
    ed = &editors[ge->owner];
    grid = XmuttGlyphEditGrid(ge->gedit);
    font = XmuttFontGridFont(ed->fgrid);

    mappings = _bdf_psf_unpack_mapping(&grid->unicode, &n);
    ShowPSFMapEditor(ge, mappings, n);
    free((char *) mappings);
}

static void
#ifndef _NO_PROTO
DoToolboxImageUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoToolboxImageUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long id;
    MXFEditorGlyphEdit *ge;
    XmuttGlyphEditUpdateCallbackStruct *cb;
    Arg av[1];

    id = (unsigned long) client_data;
    ge = &glyph_editors[id];
    cb = (XmuttGlyphEditUpdateCallbackStruct *) call_data;

    XtSetArg(av[0], XmNglyphImage, cb->image);
    XtSetValues(ge->tbox, av, 1);
}

static void
#ifndef _NO_PROTO
DoPointerUpdate(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoPointerUpdate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long id;
    MXFEditorGlyphEdit *ge;
    XmuttGlyphEditUpdateCallbackStruct *cb;
    XmString s;
    char cbuf[50];

    id = (unsigned long) client_data;
    ge = &glyph_editors[id];
    cb = (XmuttGlyphEditUpdateCallbackStruct *) call_data;

    if (cb->color >= 0)
      sprintf(cbuf, "(%d,%d) color %d", cb->x, cb->y, cb->color);
    else
      sprintf(cbuf, "(%d,%d)", cb->x, cb->y);
    s = XmStringCreateSimple(cbuf);
    XtVaSetValues(ge->pointer, XmNlabelString, s, 0);
    XmStringFree(s);
}

static void
#ifndef _NO_PROTO
DoOperationSwitch(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoOperationSwitch(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;
    XmuttGlyphEditUpdateCallbackStruct *cb;

    ge = &glyph_editors[(unsigned long) client_data];
    cb = (XmuttGlyphEditUpdateCallbackStruct *) call_data;

    if (cb->reason == XmuttGLYPHEDIT_COLOR_CHANGE)
      XmuttGlyphEditToolboxSetColor(ge->tbox, cb->color);
    else
      XmuttGlyphEditToolboxSetToggle(ge->tbox, cb->operation, False);
}

static void
#ifndef _NO_PROTO
DoGlyphInfoModified(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoGlyphInfoModified(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XtSetSensitive(ge->update, True);
    XtSetSensitive(ge->update_next, True);
    XtSetSensitive(ge->update_prev, True);
}

static void
#ifndef _NO_PROTO
DoGlyphModified(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoGlyphModified(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;
    XmString s;
    XmuttGlyphEditUpdateCallbackStruct *cb;
    char nbuf[8];
    Arg av[1];

    ge = &glyph_editors[(unsigned long) client_data];
    cb = (XmuttGlyphEditUpdateCallbackStruct *) call_data;

    sprintf(name, "width %hd height %hd\nascent %hd descent %hd",
            cb->metrics->width, cb->metrics->height,
            cb->metrics->ascent, cb->metrics->descent);
    s = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->metrics, av, 1);
    XmStringFree(s);

    if (cb->metrics->font_spacing == BDF_PROPORTIONAL) {
        sprintf(nbuf, "%hd", cb->metrics->dwidth);
        XmTextFieldSetString(ge->dwidth, nbuf);
        XmTextFieldSetCursorPosition(ge->dwidth, 4);
    }

    XtSetSensitive(ge->update, True);
    XtSetSensitive(ge->update_next, True);
    XtSetSensitive(ge->update_prev, True);
}

static void
#ifndef _NO_PROTO
DoGlyphReload(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoGlyphReload(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    bdf_glyph_grid_t *grid;
    XmString s;
    bdf_bitmap_t image;
    Arg av[1];

    ge = &glyph_editors[(unsigned long) client_data];

    /*
     * Get the parent FontGrid so the font can be accessed.
     */
    ed = &editors[ge->owner];

    /*
     * Get the old grid so the unencoded flag can be used.
     */
    grid = XmuttGlyphEditGrid(ge->gedit);

    /*
     * Get the new grid.
     */
    if (grid->unencoded)
      grid = bdf_make_glyph_grid(XmuttFontGridFont(ed->fgrid),
                                 grid->encoding, 1);
    else
      grid = bdf_make_glyph_grid(XmuttFontGridFont(ed->fgrid),
                                 grid->encoding, 0);

    /*
     * Change the  grid in the glyph editor.
     */
    XtSetArg(av[0], XmNglyphGrid, grid);
    XtSetValues(ge->gedit, av, 1);

    /*
     * Update the image.
     */
    XmuttGlyphEditImage(ge->gedit, &image);
    XtSetArg(av[0], XmNglyphImage, &image);
    XtSetValues(ge->tbox, av, 1);
    if (image.bytes > 0)
      free((char *) image.bitmap);

    /*
     * Restore the glyph name.
     */
    XmTextFieldSetString(ge->name, grid->name);
    XmTextFieldSetCursorPosition(ge->name, strlen(grid->name));

    /*
     * Restore the metrics info.
     */
    sprintf(name, "%hd", grid->dwidth);
    XtSetSensitive(ge->dwidth, True);
    XmTextFieldSetString(ge->dwidth, name);
    XmTextFieldSetCursorPosition(ge->dwidth, 4);
    if (grid->spacing != BDF_PROPORTIONAL)
      XtSetSensitive(ge->dwidth, False);

    sprintf(name, "width %hd height %hd\nascent %hd descent %hd",
            grid->glyph_bbx.width, grid->glyph_bbx.height,
            grid->glyph_bbx.ascent, grid->glyph_bbx.descent);
    s = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->metrics, av, 1);
    XmStringFree(s);

    /*
     * Change the sensitivity of the update menu option.
     */
    if (grid->modified) {
        XtSetSensitive(ge->update, True);
        XtSetSensitive(ge->update_next, True);
        XtSetSensitive(ge->update_prev, True);
    } else {
        XtSetSensitive(ge->update, False);
        XtSetSensitive(ge->update_next, False);
        XtSetSensitive(ge->update_prev, False);
    }
}

static void
#ifndef _NO_PROTO
DoSetEditMenu(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoSetEditMenu(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;
    bdf_glyph_grid_t *grid;
    Boolean s;

    ge = &glyph_editors[(unsigned long) client_data];
    if (XmuttGlyphEditClipboardEmpty(ge->gedit) == True)
      XtSetSensitive(ge->paste, False);
    else
      XtSetSensitive(ge->paste, True);

    s = XmuttGlyphEditHasSelection(ge->gedit);
    XtSetSensitive(ge->copy, s);
    XtSetSensitive(ge->cut, s);

    /*
     * Adjust the sensitivity of the next and previous buttons.
     */
    grid = XmuttGlyphEditGrid(ge->gedit);
    if (grid->encoding == 0)
      XtSetSensitive(ge->prev, False);
    else
      XtSetSensitive(ge->prev, True);

    if (grid->encoding == 0xffff)
      XtSetSensitive(ge->next, False);
    else
      XtSetSensitive(ge->next, True);

    if (grid->modified)
      XtSetSensitive(ge->reload, True);
    else
      XtSetSensitive(ge->reload, False);

    if (grid->spacing != BDF_PROPORTIONAL)
      XtSetSensitive(ge->unimap, True);
    else
      XtSetSensitive(ge->unimap, False);
}

static void
#ifndef _NO_PROTO
DoResetEditMenu(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoResetEditMenu(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XtSetSensitive(ge->reload, True);
    XtSetSensitive(ge->paste, True);
    XtSetSensitive(ge->copy, True);
    XtSetSensitive(ge->cut, True);
    XtSetSensitive(ge->next, True);
    XtSetSensitive(ge->prev, True);
    XtSetSensitive(ge->unimap, True);
}

static void
#ifndef _NO_PROTO
DoToolboxDraw(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoToolboxDraw(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XmuttGlyphEditToolboxSetToggle(ge->tbox, XmuttGLYPHEDIT_DRAW, True);
}

static void
#ifndef _NO_PROTO
DoToolboxMove(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoToolboxMove(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XmuttGlyphEditToolboxSetToggle(ge->tbox, XmuttGLYPHEDIT_MOVE, True);
}

static void
#ifndef _NO_PROTO
DoToolboxCopy(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoToolboxCopy(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;

    ge = &glyph_editors[(unsigned long) client_data];
    XmuttGlyphEditToolboxSetToggle(ge->tbox, XmuttGLYPHEDIT_COPY, True);
}

static void
#ifndef _NO_PROTO
DoToolboxContextHelp(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoToolboxContextHelp(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditorGlyphEdit *ge;
    XmuttGlyphEditToolboxCallbackStruct *cb;
    XmString s;
    Arg av[2];

    ge = &glyph_editors[(unsigned long) client_data];
    cb = (XmuttGlyphEditToolboxCallbackStruct *) call_data;

    s = XmStringCreateSimple(cb->help);
    XtSetArg(av[0], XmNlabelString, s);
    XtSetValues(ge->tboxhelp, av, 1);
    XmStringFree(s);
}

static void
#ifndef _NO_PROTO
DoToolboxAction(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoToolboxAction(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    unsigned long id;
    MXFEditorGlyphEdit *ge;
    XmuttGlyphEditToolboxCallbackStruct *cb;

    id = (unsigned long) client_data;
    ge = &glyph_editors[id];
    cb = (XmuttGlyphEditToolboxCallbackStruct *) call_data;

    if (cb->reason == XmuttGLYPHEDIT_COLOR) {
        XmuttGlyphEditChangeColor(ge->gedit, cb->color);
        return;
    }

    switch (cb->op_action) {
      case XmuttGLYPHEDIT_DRAW:
      case XmuttGLYPHEDIT_MOVE:
      case XmuttGLYPHEDIT_COPY:
        XmuttGlyphEditChangeOperation(ge->gedit, cb->op_action);
        break;
      case XmuttGLYPHEDIT_FLIP_HORIZ:
        XmuttGlyphEditFlip(ge->gedit, XmuttGLYPHEDIT_HORIZONTAL);
        break;
      case XmuttGLYPHEDIT_FLIP_VERT:
        XmuttGlyphEditFlip(ge->gedit, XmuttGLYPHEDIT_VERTICAL);
        break;
      case XmuttGLYPHEDIT_SHEAR:
        /*
         * Call the shear dialog.
         */
        ShowShear(ge->gedit, (XtPointer) ge->id, 0);
        break;
      case XmuttGLYPHEDIT_ROT_LEFT:
        XmuttGlyphEditRotate(ge->gedit, -90);
        break;
      case XmuttGLYPHEDIT_ROT_RIGHT:
        XmuttGlyphEditRotate(ge->gedit, 90);
        break;
      case XmuttGLYPHEDIT_ROTATE:
        /*
         * Call the rotate dialog.
         */
        ShowRotate(ge->gedit, (XtPointer) ge->id, 0);
        break;
      case XmuttGLYPHEDIT_SHIFT_ULEFT:
        XmuttGlyphEditShift(ge->gedit, -1, -1);
        break;
      case XmuttGLYPHEDIT_SHIFT_UP:
        XmuttGlyphEditShift(ge->gedit, 0, -1);
        break;
      case XmuttGLYPHEDIT_SHIFT_URIGHT:
        XmuttGlyphEditShift(ge->gedit, 1, -1);
        break;
      case XmuttGLYPHEDIT_SHIFT_LEFT:
        XmuttGlyphEditShift(ge->gedit, -1, 0);
        break;
      case XmuttGLYPHEDIT_SHIFT_RIGHT:
        XmuttGlyphEditShift(ge->gedit, 1, 0);
        break;
      case XmuttGLYPHEDIT_SHIFT_DLEFT:
        XmuttGlyphEditShift(ge->gedit, -1, 1);
        break;
      case XmuttGLYPHEDIT_SHIFT_DOWN:
        XmuttGlyphEditShift(ge->gedit, 0, 1);
        break;
      case XmuttGLYPHEDIT_SHIFT_DRIGHT:
        XmuttGlyphEditShift(ge->gedit, 1, 1);
        break;
    }
}

static void
#ifndef _NO_PROTO
MakeMenuBar(Widget mbar, MXFEditorGlyphEdit *ge)
#else
MakeMenuBar(mbar, ge)
Widget mbar;
MXFEditorGlyphEdit *ge;
#endif
{
    Widget pdown, casc, pb, sep;
    XmString s;
    Cardinal ac;
    Arg av[6];

    sprintf(name, "xmbdfed_glyphedit%ld_file_menu", ge->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'F'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("File", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    s = XmStringCreateSimple("Ctrl+S");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'U'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>s"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    XtSetArg(av[ac], XmNsensitive, False); ac++;
    ge->update = XtCreateManagedWidget("Update", xmPushButtonWidgetClass,
                                       pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->update, XmNactivateCallback, DoFontUpdate,
                  (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+U");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'N'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>u"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    XtSetArg(av[ac], XmNsensitive, False); ac++;
    ge->update_next = XtCreateManagedWidget("Update and Next",
                                            xmPushButtonWidgetClass,
                                            pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->update_next, XmNactivateCallback, DoFontUpdateAndNext,
                  (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+B");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'P'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>b"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    XtSetArg(av[ac], XmNsensitive, False); ac++;
    ge->update_prev = XtCreateManagedWidget("Update and Previous",
                                            xmPushButtonWidgetClass,
                                            pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->update_prev, XmNactivateCallback, DoFontUpdateAndPrev,
                  (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_file_menu_sep1", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+I");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'I'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>i"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Import XBM", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoImportXBM, (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+O");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'E'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>o"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Export XBM", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoExportXBM, (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_file_menu_sep2", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    if (xmbdfed_opts.accelerator_text != 0)
      s = XmStringCreateSimple(xmbdfed_opts.accelerator_text);
    else
      s = XmStringCreateSimple("Ctrl+F4");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'C'); ac++;
    if (xmbdfed_opts.accelerator != 0) {
        XtSetArg(av[ac], XmNaccelerator,
                 xmbdfed_opts.accelerator); ac++;
    } else {
        XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>F4"); ac++;
    }
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Close", xmPushButtonWidgetClass, pdown,
                               av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoCloseGlyphEditor,
                  (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_edit_menu", ge->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);
    XtAddCallback(pdown, XmNmapCallback, DoSetEditMenu,
                  (XtPointer) ge->id);
    XtAddCallback(pdown, XmNunmapCallback, DoResetEditMenu,
                  (XtPointer) ge->id);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'E'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("Edit", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    s = XmStringCreateSimple("Ctrl+L");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'l'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>l"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ge->reload = XtCreateManagedWidget("Reload", xmPushButtonWidgetClass,
                                       pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->reload, XmNactivateCallback, DoGlyphReload,
                  (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_edit_menu_sep0", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+R");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'R'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>r"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Resize BBX", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoResize, (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_edit_menu_sep1", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+C");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'C'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>c"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ge->copy = XtCreateManagedWidget("Copy", xmPushButtonWidgetClass,
                                     pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->copy, XmNactivateCallback, DoCopy, (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+X");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'u'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>x"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ge->cut = XtCreateManagedWidget("Cut", xmPushButtonWidgetClass,
                                    pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->cut, XmNactivateCallback, DoCut, (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+V");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'P'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>v"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ge->paste = XtCreateManagedWidget("Paste", xmPushButtonWidgetClass,
                                      pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->paste, XmNactivateCallback, DoPaste, (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_edit_menu_sep2", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+A");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'A'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>a"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Select All", xmPushButtonWidgetClass,
                               pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoSelectAll, (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_edit_menu_sep3", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+N");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'N'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>n"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ge->next = XtCreateManagedWidget("Next Glyph", xmPushButtonWidgetClass,
                                     pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->next, XmNactivateCallback, DoNextGlyph,
                  (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+P");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'v'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>p"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ge->prev = XtCreateManagedWidget("Previous Glyph", xmPushButtonWidgetClass,
                                     pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->prev, XmNactivateCallback, DoPrevGlyph,
                  (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_edit_menu_sep4", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+F");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'M'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>f"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    ge->unimap = XtCreateManagedWidget("Edit PSF Unicode Mappings",
                                       xmPushButtonWidgetClass,
                                       pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(ge->unimap, XmNactivateCallback, EditPSFUnicodeMap,
                  (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_operation_menu", ge->id);
    pdown = XmCreatePulldownMenu(mbar, name, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'O'); ac++;
    XtSetArg(av[ac], XmNsubMenuId, pdown); ac++;
    casc = XtCreateManagedWidget("Operation", xmCascadeButtonWidgetClass, mbar,
                                 av, ac);

    s = XmStringCreateSimple("Ctrl+D");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'D'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>d"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Draw", xmPushButtonWidgetClass, pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoToolboxDraw, (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+M");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'M'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>m"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Move", xmPushButtonWidgetClass, pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoToolboxMove, (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+Y");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'C'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>y"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Copy", xmPushButtonWidgetClass, pdown, av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, DoToolboxCopy, (XtPointer) ge->id);

    sprintf(name, "xmbdfed_glyphedit%ld_operation_menu_sep", ge->id);
    sep = XtCreateManagedWidget(name, xmSeparatorWidgetClass, pdown, 0, 0);

    s = XmStringCreateSimple("Ctrl+T");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'R'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>t"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Rotate", xmPushButtonWidgetClass, pdown,
                               av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, ShowRotate, (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+E");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'S'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>e"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Shear", xmPushButtonWidgetClass, pdown,
                               av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, ShowShear, (XtPointer) ge->id);

    s = XmStringCreateSimple("Ctrl+H");
    ac = 0;
    XtSetArg(av[ac], XmNmnemonic, 'E'); ac++;
    XtSetArg(av[ac], XmNaccelerator, "Ctrl<Key>h"); ac++;
    XtSetArg(av[ac], XmNacceleratorText, s); ac++;
    pb = XtCreateManagedWidget("Embolden", xmPushButtonWidgetClass, pdown,
                               av, ac);
    XmStringFree(s);
    XtAddCallback(pb, XmNactivateCallback, EmboldenGlyph, (XtPointer) ge->id);
}

static void
#ifndef _NO_PROTO
BuildEditor(MXFEditorGlyphEdit *ge, bdf_glyph_grid_t *grid, int base,
            MXFEditor *ed)
#else
BuildEditor(ge, grid, base, ed)
MXFEditorGlyphEdit *ge;
bdf_glyph_grid_t *grid;
int base;
MXFEditor *ed;
#endif
{
    Widget frame, label, name_label, form, form1, form2, mb;
    XmString s;
    Dimension ht, wd;
    Cardinal ac;
    bdf_bitmap_t image;
    Arg av[10];

    if (ed->file == 0)
      sprintf(title, "%s - Glyph Edit: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Glyph Edit: %s", app_name, ed->file);

    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld", ge->id);
    ge->shell = XtCreatePopupShell(name, topLevelShellWidgetClass, top,
                                   av, ac);
    XtAddCallback(ge->shell, XmNpopdownCallback, DoCloseGlyphEditor,
                  (XtPointer) ge->id);

    /*
     * Create the main window.
     */
    sprintf(name, "xmbdfed_glyphedit%ld_mainw", ge->id);
    ge->mainw = XtCreateWidget(name, xmMainWindowWidgetClass, ge->shell, 0, 0);

    /*
     * Add the menubar.
     */
    sprintf(name, "xmbdfed_glyphedit%ld_menu", ge->id);
    mb = XmCreateMenuBar(ge->mainw, name, 0, 0);
    MakeMenuBar(mb, ge);
    XtManageChild(mb);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_form", ge->id);
    form = XtCreateManagedWidget(name, xmFormWidgetClass, ge->mainw, av, ac);

    /*
     * Create the frame for the name and encoding info.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_info_frame", ge->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    /*
     * Create the name and encoding form.
     */
    sprintf(name, "xmbdfed_glyphedit%ld_info_form", ge->id);
    form1 = XtCreateManagedWidget(name, xmFormWidgetClass, frame, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    name_label = label =
        XtCreateManagedWidget("Glyph Name:", xmLabelWidgetClass, form1,
                              av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label); ac++;
    XtSetArg(av[ac], XmNvalue, grid->name); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_info_name", ge->id);
    ge->name = XtCreateManagedWidget(name, xmTextFieldWidgetClass, form1,
                                     av, ac);
    XtAddCallback(ge->name, XmNmodifyVerifyCallback, DoGlyphInfoModified,
                  (XtPointer) ge->id);

    /*
     * Adjust the height of the Glyph Name label so it centers next to the
     * text field.
     */
    XtSetArg(av[0], XmNheight, &ht);
    XtGetValues(ge->name, av, 1);

    XtSetArg(av[0], XmNheight, ht);
    XtSetValues(label, av, 1);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, label); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
    XtSetArg(av[ac], XmNrightWidget, label); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    label = XtCreateManagedWidget("Encoding:", xmLabelWidgetClass, form1,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, ge->name); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, ge->name); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    if (grid->unencoded)
      sprintf(name, "-1");
    else {
        switch (base) {
          case 8: sprintf(name, "%lo", grid->encoding); break;
          case 10: sprintf(name, "%ld", grid->encoding); break;
          case 16: sprintf(name, "%04lX", grid->encoding); break;
        }
    }
    ge->encoding = XtCreateManagedWidget(name, xmLabelWidgetClass, form1,
                                         av, ac);

    /*
     * Create the frame for the metrics info.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_metrics_frame", ge->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    /*
     * Create the metrics form.
     */
    sprintf(name, "xmbdfed_glyphedit%ld_metrics_form", ge->id);
    form1 = XtCreateManagedWidget(name, xmFormWidgetClass, frame, 0, 0);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    label = XtCreateManagedWidget("Device Width:", xmLabelWidgetClass, form1,
                                  av, ac);

    /*
     * Adjust the Glyph Name label to the same width as the Device Width
     * label.
     */
    XtSetArg(av[0], XmNwidth, &wd);
    XtGetValues(label, av, 1);

    XtSetArg(av[0], XmNwidth, wd);
    XtSetValues(name_label, av, 1);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_metrics_dwidth", ge->id);
    ge->dwidth = XtCreateManagedWidget(name, xmTextFieldWidgetClass, form1,
                                       av, ac);
    XtAddCallback(ge->dwidth, XmNmodifyVerifyCallback, DoGlyphInfoModified,
                  (XtPointer) ge->id);

    /*
     * Set the Device Width value.
     */
    sprintf(name, "%hd", grid->dwidth);
    XmTextFieldSetString(ge->dwidth, name);
    XmTextFieldSetCursorPosition(ge->dwidth, 4);
    if (grid->spacing != BDF_PROPORTIONAL)
      XtSetSensitive(ge->dwidth, False);

    /*
     * Adjust the height of the Device Width label so it centers next to the
     * text field.
     */
    XtSetArg(av[0], XmNheight, &ht);
    XtGetValues(ge->dwidth, av, 1);

    XtSetArg(av[0], XmNheight, ht);
    XtSetValues(label, av, 1);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, label); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
    XtSetArg(av[ac], XmNrightWidget, label); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    label = XtCreateManagedWidget("Metrics:", xmLabelWidgetClass, form1,
                                  av, ac);

    sprintf(name, "width %hd height %hd\nascent %hd descent %hd",
            grid->glyph_bbx.width, grid->glyph_bbx.height,
            grid->glyph_bbx.ascent, grid->glyph_bbx.descent);
    s = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, ge->dwidth); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, label); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    XtSetArg(av[ac], XmNlabelString, s); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_editor_metrics", ge->id);
    ge->metrics = XtCreateManagedWidget(name, xmLabelWidgetClass, form1,
                                        av, ac);
    XmStringFree(s);

    /*
     * Create the frame that will hold the pointer info and the glyph editor.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_editor_frame", ge->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    /*
     * Create the glyph editor form.
     */
    sprintf(name, "xmbdfed_glyphedit%ld_editor_form", ge->id);
    form1 = XtCreateManagedWidget(name, xmFormWidgetClass, frame, 0, 0);

    /*
     * Create the form that will hold the pointer and glyph editor.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_editor_left_form", ge->id);
    form2 = XtCreateManagedWidget(name, xmFormWidgetClass, form1, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    ge->pointer = XtCreateManagedWidget("(0,0)", xmLabelWidgetClass, form2,
                                        av, ac);

    /*
     * Create the glyph editor.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, ge->pointer); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNglyphGrid, grid); ac++;
    XtSetArg(av[ac], XmNdefaultPixelSize, xmbdfed_opts.pixel_size); ac++;
    XtSetArg(av[ac], XmNshowXHeight, xmbdfed_opts.show_x_height); ac++;
    XtSetArg(av[ac], XmNshowCapHeight, xmbdfed_opts.show_cap_height); ac++;
    XtSetArg(av[ac], XmNcolorList, xmbdfed_opts.pixels); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_editor", ge->id);
    ge->gedit = XtCreateManagedWidget(name, xmuttGlyphEditWidgetClass, form2,
                                      av, ac);
    XtAddCallback(ge->gedit, XmNimageChangeCallback, DoToolboxImageUpdate,
                  (XtPointer) ge->id);
    XtAddCallback(ge->gedit, XmNpointerMovedCallback, DoPointerUpdate,
                  (XtPointer) ge->id);
    XtAddCallback(ge->gedit, XmNoperationSwitchCallback, DoOperationSwitch,
                  (XtPointer) ge->id);
    XtAddCallback(ge->gedit, XmNglyphModifiedCallback, DoGlyphModified,
                  (XtPointer) ge->id);


    /*
     * Create the form that will hold the toolbox and its help label.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNleftWidget, form2); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_editor_right_form", ge->id);
    form2 = XtCreateManagedWidget(name, xmFormWidgetClass, form1, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    ge->tboxhelp = XtCreateManagedWidget("None", xmLabelWidgetClass, form2,
                                         av, ac);

    /*
     * Get the glyph edit image for the toolbox.
     */
    XmuttGlyphEditImage(ge->gedit, &image);

    /*
     * Create the glyph editor toolbox.
     */
    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, ge->tboxhelp); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNglyphImage, &image); ac++;
    XtSetArg(av[ac], XmNcolorList, xmbdfed_opts.pixels); ac++;
    sprintf(name, "xmbdfed_glyphedit%ld_toolbox", ge->id);
    ge->tbox = XtCreateManagedWidget(name, xmuttGlyphEditToolboxWidgetClass,
                                     form2, av, ac);
    XtAddCallback(ge->tbox, XmNactivateCallback, DoToolboxAction,
                  (XtPointer) ge->id);
    XtAddCallback(ge->tbox, XmNcontextHelpCallback, DoToolboxContextHelp,
                  (XtPointer) ge->id);
    if (image.bytes > 0)
      free((char *) image.bitmap);
}

void
#ifndef _NO_PROTO
DoGlyphEdit(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoGlyphEdit(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    int base;
    XmString s;
    MXFEditor *ed;
    MXFEditorGlyphEdit *ge;
    XmuttFontGridSelectionStruct *cb;
    bdf_glyph_grid_t *grid;
    bdf_font_t *font;
    bdf_bitmap_t image;
    Arg av[1];

    ed = &editors[(unsigned long) client_data];

    /*
     * Before doing anything, check to see if the pixmaps have been created
     * yet or not.
     */
    if (have_pixmaps == False)
      GlyphEditCreatePixmaps(ed);

    cb = (XmuttFontGridSelectionStruct *) call_data;

    font = XmuttFontGridFont(ed->fgrid);
    base = XmuttFontGridCodeBase(ed->fgrid);

    /*
     * Create the glyph grid.
     */
    if (cb->unencoded)
      grid = bdf_make_glyph_grid(font, cb->start, 1);
    else
      grid = bdf_make_glyph_grid(font, cb->start, 0);

    ge = GetGlyphEditor(ed->id);
    if (ge->name == 0) {
        BuildEditor(ge, grid, base, ed);
        XmTextFieldSetCursorPosition(ge->name, strlen(grid->name));
        XmTextFieldSetCursorPosition(ge->dwidth, 4);
    } else {
        /*
         * Update the dialog title.
         */
        if (ed->file == 0)
          sprintf(title, "%s - Glyph Edit: (unnamed%ld)", app_name, ed->id);
        else
          sprintf(title, "%s - Glyph Edit: %s", app_name, ed->file);
        XtSetArg(av[0], XmNtitle, title);
        XtSetValues(ge->shell, av, 1);

        /*
         * Set the new grid.
         */
        XtSetArg(av[0], XmNglyphGrid, grid);
        XtSetValues(ge->gedit, av, 1);

        /*
         * Get the grid image so the toolbox image can be updated.
         */
        XmuttGlyphEditImage(ge->gedit, &image);
        XtSetArg(av[0], XmNglyphImage, &image);
        XtSetValues(ge->tbox, av, 1);
        if (image.bytes > 0)
          free((char *) image.bitmap);

        /*
         * Set the glyph info on the glyph editor.
         */
        XmTextFieldSetString(ge->name, grid->name);
        XmTextFieldSetCursorPosition(ge->name, strlen(grid->name));

        if (grid->unencoded)
          sprintf(name, "-1");
        else {
            switch (base) {
              case 8: sprintf(name, "%lo", grid->encoding); break;
              case 10: sprintf(name, "%ld", grid->encoding); break;
              case 16: sprintf(name, "%04lX", grid->encoding); break;
            }
        }
        s = XmStringCreateSimple(name);
        XtSetArg(av[0], XmNlabelString, s);
        XtSetValues(ge->encoding, av, 1);
        XmStringFree(s);

        sprintf(name, "%hd", grid->dwidth);
        XtSetSensitive(ge->dwidth, True);
        XmTextFieldSetString(ge->dwidth, name);
        XmTextFieldSetCursorPosition(ge->dwidth, 4);
        if (grid->spacing != BDF_PROPORTIONAL)
          XtSetSensitive(ge->dwidth, False);

        sprintf(name, "width %hd height %hd\nascent %hd descent %hd",
                grid->glyph_bbx.width, grid->glyph_bbx.height,
                grid->glyph_bbx.ascent, grid->glyph_bbx.descent);
        s = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
        XtSetArg(av[0], XmNlabelString, s);
        XtSetValues(ge->metrics, av, 1);
        XmStringFree(s);
    }

    /*
     * Change the sensitivity of the update menu item.
     */
    if (grid->modified) {
        XtSetSensitive(ge->update, True);
        XtSetSensitive(ge->update_next, True);
        XtSetSensitive(ge->update_prev, True);
    } else {
        XtSetSensitive(ge->update, False);
        XtSetSensitive(ge->update_next, False);
        XtSetSensitive(ge->update_prev, False);
    }

    /*
     * Pop the editor up.
     */
    XtManageChild(ge->mainw);

    XtPopup(ge->shell, XtGrabNone);

    /*
     * Force the focus to be on the glyph editor.
     */
    (void) XmProcessTraversal(ge->gedit, XmTRAVERSE_CURRENT);
}

void
#ifndef _NO_PROTO
GlyphEditCleanup(void)
#else
GlyphEditCleanup()
#endif
{
    /*
     * Free up the allocated space.
     */
    if (num_glyph_editors > 0)
      XtFree((char *) glyph_editors);

    /*
     * Free up the pixmaps if they were created.
     */
    if (have_pixmaps == True) {
        XFreePixmap(XtDisplay(top), lbearing_image);
        XFreePixmap(XtDisplay(top), rbearing_image);
        XFreePixmap(XtDisplay(top), ascent_image);
        XFreePixmap(XtDisplay(top), descent_image);
        have_pixmaps = False;
    }
}

void
#ifndef _NO_PROTO
GlyphEditChangeBase(unsigned long id, int base)
#else
GlyphEditChangeBase(id, base)
unsigned long id;
int base;
#endif
{
    unsigned long i;
    long enc;
    MXFEditorGlyphEdit *ge;
    XmString s;
    Arg av[1];
    char nbuf[25];

    for (i = 0, ge = glyph_editors; i < num_glyph_editors; i++, ge++) {
        if (ge->owner == id &&
            (enc = XmuttGlyphEditEncoding(ge->gedit)) >= 0) {
            switch (base) {
              case 8: sprintf(nbuf, "%lo", enc); break;
              case 10: sprintf(nbuf, "%ld", enc); break;
              case 16: sprintf(nbuf, "%04lX", enc); break;
            }
            s = XmStringCreateSimple(nbuf);
            XtSetArg(av[0], XmNlabelString, s);
            XtSetValues(ge->encoding, av, 1);
            XmStringFree(s);
        }
    }
}

void
#ifndef _NO_PROTO
GlyphEditChangeSpacing(unsigned long id, unsigned short spacing,
                       unsigned short monowidth)
#else
GlyphEditChangeSpacing(id, spacing, monowidth)
unsigned long id;
unsigned short spacing, monowidth;
#endif
{
    Boolean prop;
    unsigned long i;
    MXFEditorGlyphEdit *ge;

    prop = (spacing == BDF_PROPORTIONAL) ? True : False;

    for (i = 0, ge = glyph_editors; i < num_glyph_editors; i++, ge++) {
        if (ge->owner == id) {
            if (prop == False)
              XmuttGlyphEditSetSpacing(ge->gedit, spacing, monowidth);
            XtSetSensitive(ge->dwidth, prop);
        }
    }
}

void
#ifndef _NO_PROTO
GlyphEditCreatePixmaps(MXFEditor *ed)
#else
GlyphEditCreatePixmaps(ed)
MXFEditor *ed;
#endif
{
    Window win;
    Pixel fg, bg;
    Arg av[2];

    if (have_pixmaps == False) {
        /*
         * Get the foreground and background pixels from the font grid in the
         * editor.
         */
        XtSetArg(av[0], XmNforeground, &fg);
        XtSetArg(av[1], XmNbackground, &bg);
        XtGetValues(ed->fgrid, av, 2);

        win = XRootWindowOfScreen(XtScreen(top));

        lbearing_image =
            XCreatePixmapFromBitmapData(XtDisplay(top), win,
                                        lbearing_bits, lbearing_width,
                                        lbearing_height, fg, bg,
                                        XDefaultDepthOfScreen(XtScreen(top)));
        rbearing_image =
            XCreatePixmapFromBitmapData(XtDisplay(top), win,
                                        rbearing_bits, rbearing_width,
                                        rbearing_height, fg, bg,
                                        XDefaultDepthOfScreen(XtScreen(top)));
        ascent_image =
            XCreatePixmapFromBitmapData(XtDisplay(top), win,
                                        ascent_bits, ascent_width,
                                        ascent_height, fg, bg,
                                        XDefaultDepthOfScreen(XtScreen(top)));
        descent_image =
            XCreatePixmapFromBitmapData(XtDisplay(top), win,
                                        descent_bits, descent_width,
                                        descent_height, fg, bg,
                                        XDefaultDepthOfScreen(XtScreen(top)));

        have_pixmaps = True;
    }
}

void
#ifndef _NO_PROTO
GlyphEditChangeSetupValues(unsigned long pixel_size, int show_cap_height,
                           int show_x_height)
#else
GlyphEditChangeSetupValues(pixel_size, show_cap_height, show_x_height)
unsigned long pixel_size;
int show_cap_height, show_x_height;
#endif
{
    unsigned long i;
    MXFEditorGlyphEdit *ge;
    Arg av[3];

    XtSetArg(av[0], XmNdefaultPixelSize, pixel_size);

    if (show_cap_height)
      XtSetArg(av[1], XmNshowCapHeight, True);
    else
      XtSetArg(av[1], XmNshowCapHeight, False);

    if (show_x_height)
      XtSetArg(av[2], XmNshowXHeight, True);
    else
      XtSetArg(av[2], XmNshowXHeight, False);

    for (i = 0, ge = glyph_editors; i < num_glyph_editors; i++, ge++)
      XtSetValues(ge->gedit, av, 3);
}

void
#ifndef _NO_PROTO
GlyphEditClose(MXFEditor *ed)
#else
GlyphEditClose(ed)
MXFEditor *ed;
#endif
{
    unsigned long i;
    MXFEditorGlyphEdit *ge;

    for (i = 0, ge = glyph_editors; i < num_glyph_editors; i++, ge++) {
        if (ge->owner == ed->id)
          DoCloseGlyphEditor(0, (XtPointer) ge->id, 0);
    }
}

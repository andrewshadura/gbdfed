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
static char rcsid[] __attribute__ ((unused)) = "$Id: props.c,v 1.6 2001/09/19 21:00:44 mleisher Exp $";
#else
static char rcsid[] = "$Id: props.c,v 1.6 2001/09/19 21:00:44 mleisher Exp $";
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
#include <Xm/MessageB.h>
#include <Xm/DialogS.h>
#include "FGrid.h"
#include "xmbdfed.h"

void
#ifndef _NO_PROTO
SelectProperty(Widget w, XtPointer client_data, XtPointer call_data)
#else
SelectProperty(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    Boolean from_font;
    MXFEditor *ed;
    XmListCallbackStruct *cb;
    bdf_property_t *prop;
    bdf_font_t *font;
    char *pname, nbuf[8];

    ed = &editors[(unsigned long) client_data];
    cb = (XmListCallbackStruct *) call_data;

    font = XmuttFontGridFont(ed->fgrid);

    /*
     * Extract the property name.
     */
    XmStringGetLtoR(cb->item, XmFONTLIST_DEFAULT_TAG, &pname);

    from_font = True;
    if ((prop = bdf_get_font_property(font, pname)) == 0) {
        from_font = False;
        prop = bdf_get_property(pname);
    }

    XmTextFieldSetString(ed->props.pname, prop->name);
    if (from_font == False)
      XmTextFieldSetString(ed->props.pvalue, "");

    switch (prop->format) {
      case BDF_ATOM:
        if (from_font == True)
          XmTextFieldSetString(ed->props.pvalue, prop->value.atom);
        XmToggleButtonSetState(ed->props.atom, True, True);
        break;
      case BDF_INTEGER:
        if (from_font == True) {
            sprintf(nbuf, "%ld", prop->value.int32);
            XmTextFieldSetString(ed->props.pvalue, nbuf);
        }
        XmToggleButtonSetState(ed->props.integer, True, True);
        break;
      case BDF_CARDINAL:
        if (from_font == True) {
            sprintf(nbuf, "%ld", prop->value.card32);
            XmTextFieldSetString(ed->props.pvalue, nbuf);
        }
        XmToggleButtonSetState(ed->props.cardinal, True, True);
        break;
    }

    /*
     * Change the sensitivity of the Delete button depending on whether the
     * property was defined in the font or not.  If the property came from the
     * font and is one of FONT_ASCENT or FONT_DESCENT, do not allow them to be
     * deleted.
     */
    if (from_font == True && (strncmp(pname, "FONT_ASCENT", 11) == 0 ||
                              strncmp(pname, "FONT_DESCENT", 12) == 0))
      from_font = False;
    XtSetSensitive(ed->props.delete, from_font);

    XtFree(pname);

    /*
     * Always change the sensitivity of the Update button to False when a
     * property is selected.
     */
    XtSetSensitive(ed->props.update, False);
}

void
#ifndef _NO_PROTO
DeleteProperty(Widget w, XtPointer client_data, XtPointer call_data)
#else
DeleteProperty(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    int scnt, *sel;
    XmString *items;
    char *pname;
    bdf_font_t *font;
    Arg av[1];

    ed = &editors[(unsigned long) client_data];

    font = XmuttFontGridFont(ed->fgrid);

    XmListGetSelectedPos(ed->props.plist, &sel, &scnt);
    XtSetArg(av[0], XmNitems, &items);
    XtGetValues(ed->props.plist, av, 1);

    XmStringGetLtoR(items[sel[0] - 1], XmFONTLIST_DEFAULT_TAG, &pname);

    /*
     * Alert the font grid that the font has changed.  This has to be done
     * here to allow the font grid to call the modified callback if this is
     * the first change to the font.
     */
    XmuttFontGridSetModified(ed->fgrid, True);

    /*
     * Delete the property from the font.
     */
    bdf_delete_font_property(font, pname);

    /*
     * Delete the property name from the list.
     */
    XmListDeletePos(ed->props.plist, sel[0]);

    /*
     * Force a new property to be selected.  Adjust the selection if the
     * property being deleted was the last one in the list.
     */
    XtSetArg(av[0], XmNitemCount, &scnt);
    XtGetValues(ed->props.plist, av, 1);
    if (sel[0] > scnt)
      XmListSelectPos(ed->props.plist, sel[0] - 1, True);
    else
      XmListSelectPos(ed->props.plist, sel[0], True);
}

void
#ifndef _NO_PROTO
UpdateProperty(Widget w, XtPointer client_data, XtPointer call_data)
#else
UpdateProperty(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    unsigned long i, nprops, pcnt;
    XmString item, *items;
    char *val;
    bdf_font_t *font;
    Boolean update_list;
    bdf_property_t *props, prop;

    ed = &editors[(unsigned long) client_data];

    font = XmuttFontGridFont(ed->fgrid);

    /*
     * Get the property name.  This needs some kind of validation to make sure
     * no spaces are in the name and leading and trailing spaces are removed.
     */
    val = XmTextFieldGetString(ed->props.pname);
    if (*val == 0) {
        XBell(XtDisplay(w), 0);
        XtFree(val);
        return;
    }

    /*
     * Set the name and determine if adding the property will require the
     * property list to be updated.
     */
    update_list = False;
    prop.name = val;
    if (XmToggleButtonGetState(ed->props.fontprops) == True) {
        if (bdf_get_font_property(font, prop.name) == 0)
          update_list = True;
    } else {
        if (bdf_get_property(prop.name) == 0)
          update_list = True;
    }

    /*
     * Determine the property format.
     */
    val = XmTextFieldGetString(ed->props.pvalue);
    if (XmToggleButtonGetState(ed->props.atom) == True) {
        prop.format = BDF_ATOM;
        prop.value.atom = val;
    } else if (XmToggleButtonGetState(ed->props.integer) == True) {
        prop.format = BDF_INTEGER;
        prop.value.int32 = _bdf_atol(val, 0, 10);
    } else {
        prop.format = BDF_CARDINAL;
        prop.value.card32 = _bdf_atoul(val, 0, 10);
    }

    /*
     * Add the property to the font.
     */
    bdf_add_font_property(font, &prop);

    /*
     * Alert the FontGrid that a property has been changed.
     */
    XmuttFontGridCallModifiedCallback(ed->fgrid);

    /*
     * If the list must be updated, then do so by hand.
     */
    if (update_list == True) {
        XmListDeleteAllItems(ed->props.plist);
        if (XmToggleButtonGetState(ed->props.fontprops) == True)
          nprops = bdf_font_property_list(font, &props);
        else
          nprops = bdf_property_list(&props);

        if (nprops > 0) {
            items = (XmString *) XtMalloc(sizeof(XmString) * nprops);
            for (i = pcnt = 0; i < nprops; i++) {
                if (memcmp(props[i].name, "COMMENT", 7) != 0)
                  items[pcnt++] = XmStringCreateSimple(props[i].name);
            }
            if (pcnt > 0)
              XmListAddItems(ed->props.plist, items, pcnt, 0);

            for (i = 0; i < pcnt; i++)
              XmStringFree(items[i]);
            XtFree((char *) items);
            free((char *) props);

            /*
             * Force the property just added to be selected.
             */
            item = XmStringCreateSimple(prop.name);
            XmListSelectItem(ed->props.plist, item, False);
            XmStringFree(item);
        }
    }

    /*
     * Free up the two string values that were collected.
     */
    XtFree(prop.name);
    XtFree(val);

    /*
     * Finally, disable the Update button to indicate the update has been
     * done.
     */
    XtSetSensitive(ed->props.update, False);
}

void
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
    XtSetSensitive(ed->props.update, True);
}

void
#ifndef _NO_PROTO
ShowFontProperties(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowFontProperties(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    unsigned long i, nprops, pcnt;
    XmString *items;
    bdf_font_t *font;
    bdf_property_t *props;
    XmToggleButtonCallbackStruct *cb;

    ed = &editors[(unsigned long) client_data];
    cb = (XmToggleButtonCallbackStruct *) call_data;

    if (cb->set == False)
      return;

    font = XmuttFontGridFont(ed->fgrid);

    XmListDeleteAllItems(ed->props.plist);
    if ((nprops = bdf_font_property_list(font, &props)) > 0) {
        items = (XmString *) XtMalloc(sizeof(XmString) * nprops);
        for (i = pcnt = 0; i < nprops; i++) {
            if (memcmp(props[i].name, "COMMENT", 7) != 0)
              items[pcnt++] = XmStringCreateSimple(props[i].name);
        }
        XmListAddItems(ed->props.plist, items, pcnt, 0);

        if (pcnt > 0)
          XmListSelectPos(ed->props.plist, 1, True);

        for (i = 0; i < pcnt; i++)
          XmStringFree(items[i]);

        if (nprops > 0) {
            XtFree((char *) items);
            free((char *) props);
        }
    }
}

void
#ifndef _NO_PROTO
ShowAllProperties(Widget w, XtPointer client_data, XtPointer call_data)
#else
ShowAllProperties(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    unsigned long i, nprops, pcnt;
    XmString *items;
    bdf_property_t *props;
    XmToggleButtonCallbackStruct *cb;

    ed = &editors[(unsigned long) client_data];
    cb = (XmToggleButtonCallbackStruct *) call_data;

    if (cb->set == False)
      return;

    XmListDeleteAllItems(ed->props.plist);
    if ((nprops = bdf_property_list(&props)) > 0) {
        items = (XmString *) XtMalloc(sizeof(XmString) * nprops);
        for (i = pcnt = 0; i < nprops; i++) {
            if (memcmp(props[i].name, "COMMENT", 7) != 0)
              items[pcnt++] = XmStringCreateSimple(props[i].name);
        }
        XmListAddItems(ed->props.plist, items, pcnt, 0);

        if (pcnt > 0)
          XmListSelectPos(ed->props.plist, 1, True);

        for (i = 0; i < pcnt; i++)
          XmStringFree(items[i]);

        if (nprops > 0) {
            XtFree((char *) items);
            free((char *) props);
        }
    }
}

static void
#ifndef _NO_PROTO
DoCloseProperties(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoCloseProperties(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;

    ed = &editors[(unsigned long) client_data];

    if (XtIsSensitive(ed->props.update)) {
        if (AskBooleanQuestion("Properties have been modified.", "Update",
                               "Do Not Update") == True)
          UpdateProperty(w, client_data, call_data);
    }
    XtPopdown(ed->props.shell);
}

static void
#ifndef _NO_PROTO
BuildProperties(MXFEditor *ed)
#else
BuildProperties(ed)
MXFEditor *ed;
#endif
{
    Widget frame, label, form, pb, rc;
    Cardinal ac;
    Arg av[10];

    if (ed->file == 0)
      sprintf(title, "%s - Properties: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Properties: %s", app_name, ed->file);

    ac = 0;
    XtSetArg(av[ac], XmNtitle, title); ac++;
    XtSetArg(av[ac], XmNallowShellResize, True); ac++;
    XtSetArg(av[ac], XmNdeleteResponse, XmUNMAP); ac++;
    sprintf(name, "xmbdfed_properties%ld_shell", ed->id);
    ed->props.shell = XtCreatePopupShell(name, xmDialogShellWidgetClass, top,
                                         av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(av[ac], XmNmarginWidth, 2); ac++;
    XtSetArg(av[ac], XmNmarginHeight, 2); ac++;
    sprintf(name, "xmbdfed_properties%ld_form", ed->id);
    ed->props.form = form =
        XtCreateWidget(name, xmFormWidgetClass, ed->props.shell, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_properties%ld_frame5", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNfractionBase, 3); ac++;
    sprintf(name, "xmbdfed_properties%ld_button_form", ed->id);
    form = XtCreateManagedWidget(name, xmFormWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 0); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 1); ac++;
    ed->props.update = XtCreateManagedWidget("Update", xmPushButtonWidgetClass,
                                             form, av, ac);
    XtAddCallback(ed->props.update, XmNactivateCallback, UpdateProperty,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 1); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 2); ac++;
    ed->props.delete = XtCreateManagedWidget("Delete", xmPushButtonWidgetClass,
                                             form, av, ac);
    XtAddCallback(ed->props.delete, XmNactivateCallback, DeleteProperty,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNleftPosition, 2); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(av[ac], XmNrightPosition, 3); ac++;
    pb = XtCreateManagedWidget("Done", xmPushButtonWidgetClass, form,
                               av, ac);
    XtAddCallback(pb, XmNactivateCallback, DoCloseProperties,
                  (XtPointer) ed->id);

    form = ed->props.form;

    ac = 0;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_properties%ld_frame4", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Show Properties", xmLabelWidgetClass, frame,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNnumColumns, 1); ac++;
    XtSetArg(av[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    sprintf(name, "xmbdfed_properties%ld_show_options", ed->id);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNset, True); ac++;
    ed->props.fontprops = XtCreateManagedWidget("Font Properties",
                                                xmToggleButtonWidgetClass,
                                                rc, av, ac);
    XtAddCallback(ed->props.fontprops, XmNvalueChangedCallback,
                  ShowFontProperties, (XtPointer) ed->id);
    ed->props.allprops = XtCreateManagedWidget("All Properties",
                                               xmToggleButtonWidgetClass,
                                               rc, 0, 0);
    XtAddCallback(ed->props.allprops, XmNvalueChangedCallback,
                  ShowAllProperties, (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_properties%ld_frame3", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Property Type", xmLabelWidgetClass, frame,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(av[ac], XmNnumColumns, 1); ac++;
    XtSetArg(av[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(av[ac], XmNradioBehavior, True); ac++;
    sprintf(name, "xmbdfed_properties%ld_types", ed->id);
    rc = XtCreateManagedWidget(name, xmRowColumnWidgetClass, frame, av, ac);

    ed->props.atom = XtCreateManagedWidget("Atom", xmToggleButtonWidgetClass,
                                           rc, 0, 0);
    XtAddCallback(ed->props.atom, XmNvalueChangedCallback, EnableUpdate,
                  (XtPointer) ed->id);

    ed->props.integer = XtCreateManagedWidget("Integer",
                                              xmToggleButtonWidgetClass, rc,
                                              0, 0);
    XtAddCallback(ed->props.integer, XmNvalueChangedCallback, EnableUpdate,
                  (XtPointer) ed->id);

    ed->props.cardinal = XtCreateManagedWidget("Cardinal",
                                               xmToggleButtonWidgetClass, rc,
                                               0, 0);
    XtAddCallback(ed->props.cardinal, XmNvalueChangedCallback, EnableUpdate,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_properties%ld_frame2", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Property Value", xmLabelWidgetClass, frame,
                                  av, ac);

    sprintf(name, "xmbdfed_properties%ld_value", ed->id);
    ed->props.pvalue = XtCreateManagedWidget(name, xmTextFieldWidgetClass,
                                             frame, 0, 0);
    XtAddCallback(ed->props.pvalue, XmNvalueChangedCallback, EnableUpdate,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    sprintf(name, "xmbdfed_properties%ld_frame1", ed->id);
    frame = XtCreateManagedWidget(name, xmFrameWidgetClass, form, av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNchildType, XmFRAME_TITLE_CHILD); ac++;
    label = XtCreateManagedWidget("Property Name", xmLabelWidgetClass, frame,
                                  av, ac);

    sprintf(name, "xmbdfed_properties%ld_name", ed->id);
    ed->props.pname = XtCreateManagedWidget(name, xmTextFieldWidgetClass,
                                            frame, 0, 0);
    XtAddCallback(ed->props.pname, XmNvalueChangedCallback, EnableUpdate,
                  (XtPointer) ed->id);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    label = XtCreateManagedWidget("Properties", xmLabelWidgetClass, form,
                                  av, ac);

    ac = 0;
    XtSetArg(av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNtopWidget, label); ac++;
    XtSetArg(av[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(av[ac], XmNbottomWidget, frame); ac++;
    XtSetArg(av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(av[ac], XmNvisibleItemCount, 5); ac++;
    XtSetArg(av[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
    XtSetArg(av[ac], XmNlistSizePolicy, XmVARIABLE); ac++;
    sprintf(name, "xmbdfed_properties%ld_list", ed->id);
    ed->props.plist = XmCreateScrolledList(form, name, av, ac);
    XtManageChild(ed->props.plist);
    XtAddCallback(ed->props.plist, XmNbrowseSelectionCallback,
                  SelectProperty, (XtPointer) ed->id);

}

void
#ifndef _NO_PROTO
DoFontProperties(Widget w, XtPointer client_data, XtPointer call_data)
#else
DoFontProperties(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
#endif
{
    MXFEditor *ed;
    Arg av[1];

    ed = &editors[(unsigned long) client_data];
    if (ed->props.shell == 0)
      BuildProperties(ed);

    XmListDeleteAllItems(ed->props.plist);

    if (XmToggleButtonGetState(ed->props.fontprops) == True) {
        XmToggleButtonSetState(ed->props.fontprops, False, False);
        XmToggleButtonSetState(ed->props.fontprops, True, True);
    } else {
        XmToggleButtonSetState(ed->props.allprops, False, False);
        XmToggleButtonSetState(ed->props.allprops, True, True);
    }

    XtSetSensitive(ed->props.update, False);

    /*
     * Make sure the title matches the font name.
     */
    if (ed->file == 0)
      sprintf(title, "%s - Properties: (unnamed%ld)", app_name, ed->id);
    else
      sprintf(title, "%s - Properties: %s", app_name, ed->file);

    XtSetArg(av[0], XmNtitle, title);
    XtSetValues(ed->props.shell, av, 1);

    XtManageChild(ed->props.form);
    XtPopup(ed->props.shell, XtGrabNone);
}

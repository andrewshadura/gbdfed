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
static char rcsid[] __attribute__ ((unused)) = "$Id: ProgBar.c,v 1.8 2004/02/09 16:28:25 mleisher Exp $";
#else
static char rcsid[] = "$Id: ProgBar.c,v 1.8 2004/02/09 16:28:25 mleisher Exp $";
#endif
#endif

#include <stdlib.h>
#include "ProgBarP.h"

static Dimension
#ifndef _NO_PROTO
_XmuttProgressBarPMargins(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarPMargins(pw)
XmuttProgressBarWidget pw;
#endif
{
    return pw->primitive.shadow_thickness << 1;
}

static Dimension
#ifndef _NO_PROTO
_XmuttProgressBarHMargins(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarHMargins(pw)
XmuttProgressBarWidget pw;
#endif
{
    return _XmuttProgressBarPMargins(pw) + (pw->progress.mwidth << 1);
}

static Dimension
#ifndef _NO_PROTO
_XmuttProgressBarVMargins(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarVMargins(pw)
XmuttProgressBarWidget pw;
#endif
{
    return _XmuttProgressBarPMargins(pw) + (pw->progress.mheight << 1);
}

static Dimension
#ifndef _NO_PROTO
_XmuttProgressBarTopY(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarTopY(pw)
XmuttProgressBarWidget pw;
#endif
{
    return (_XmuttProgressBarPMargins(pw) >> 1) + pw->progress.mheight;
}

static Dimension
#ifndef _NO_PROTO
_XmuttProgressBarBottomY(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarBottomY(pw)
XmuttProgressBarWidget pw;
#endif
{
    return pw->core.height -
        ((_XmuttProgressBarPMargins(pw) >> 1) + pw->progress.mheight);
}

static Dimension
#ifndef _NO_PROTO
_XmuttProgressBarLeftX(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarLeftX(pw)
XmuttProgressBarWidget pw;
#endif
{
    return (_XmuttProgressBarPMargins(pw) >> 1) + pw->progress.mwidth;
}

static Dimension
#ifndef _NO_PROTO
_XmuttProgressBarRightX(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarRightX(pw)
XmuttProgressBarWidget pw;
#endif
{
    return pw->core.width -
        ((_XmuttProgressBarPMargins(pw) >> 1) + pw->progress.mwidth);
}

static void
#ifndef _NO_PROTO
_XmuttProgressBarShadowWindow(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarShadowWindow(pw)
XmuttProgressBarWidget pw;
#endif
{
    Widget w;
    int i;
    XPoint br[3], tl[3];

    w = (Widget) pw;

    if (!XtIsRealized(w) || pw->primitive.shadow_thickness == 0)
      return;

    /*
     * Lower left corner outside line.
     */
    tl[0].x = 0;
    tl[0].y = pw->core.height - 1;

    /*
     * Upper left corner outside line.
     */
    tl[1].x = tl[0].x;
    tl[1].y = 0;

    /*
     * Upper right corner outside line.
     */
    tl[2].x = pw->core.width - 1;
    tl[2].y = tl[1].y;

    /*
     * Upper right corner outside line.
     */
    br[0].x = tl[2].x;
    br[0].y = tl[2].y + 1;

    /*
     * Lower right corner outside line.
     */
    br[1].x = br[0].x;
    br[1].y = pw->core.height - 1;

    /*
     * Lower left corner outside line.
     */
    br[2].x = 1;
    br[2].y = br[1].y;

    XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.top_shadow_GC,
               tl, 3, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.bottom_shadow_GC,
               br, 3, CoordModeOrigin);

    /*
     * Draw the remaining shadows successively inward.
     */
    for (i = 1; i < pw->primitive.shadow_thickness; i++) {
        tl[0].x++;
        tl[0].y--;
        tl[1].x++;
        tl[1].y++;
        tl[2].x--;
        tl[2].y++;

        br[0].x--;
        br[0].y++;
        br[1].x--;
        br[1].y--;
        br[2].x++;
        br[2].y--;
        XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.top_shadow_GC,
                   tl, 3, CoordModeOrigin);
        XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.bottom_shadow_GC,
                   br, 3, CoordModeOrigin);
    }
}

static void
#ifndef _NO_PROTO
_XmuttProgressBarPreferredGeometry(XmuttProgressBarWidget pw, Dimension *width,
                                   Dimension *height)
#else
_XmuttProgressBarPreferredGeometry(pw, width, height)
XmuttProgressBarWidget pw;
Dimension *width, *height;
#endif
{
    Dimension wd, ht, pmargin, lheight, lwidth;

    pmargin = pw->primitive.shadow_thickness << 1;
    lheight = pw->progress.label_height;
    if ((lwidth = pw->progress.label_width) < 200)
      lwidth = 200;

    wd = _XmuttProgressBarHMargins(pw) + 4 + lwidth;

    ht = _XmuttProgressBarVMargins(pw) + 4 + pw->progress.percheight;

    /*
     * Add the label height if a label exists.
     */
    if (lheight)
      ht += lheight + 2;

    *width = wd;
    *height = ht;
}

static void
#ifndef _NO_PROTO
_XmuttProgressBarSetBarGeometry(XmuttProgressBarWidget pw)
#else
_XmuttProgressBarSetBarGeometry(pw)
XmuttProgressBarWidget pw;
#endif
{
    pw->progress.bar.x = _XmuttProgressBarLeftX(pw) + 2;
    pw->progress.bar.y = _XmuttProgressBarTopY(pw) + 2 +
        pw->progress.label_height + 2;

    pw->progress.bar.width = (_XmuttProgressBarRightX(pw) - 2) -
        pw->progress.bar.x;
    pw->progress.bar.height = (_XmuttProgressBarBottomY(pw) - 2) -
        pw->progress.bar.y;
}

static void
#ifndef _NO_PROTO
_XmuttProgressBarMakeGCs(Widget w, Boolean force)
#else
_XmuttProgressBarMakeGCs(w, force)
Widget w;
Boolean force;
#endif
{
    XmuttProgressBarWidget pw;
    XmFontContext fctx;
    XmFontListEntry fe;
    XmFontType ft;
    XFontStruct *fnt;
    XtGCMask gcvm;
    XGCValues gcv;

    pw = (XmuttProgressBarWidget) w;

    gcvm = GCForeground|GCBackground|GCFunction;

    XmFontListInitFontContext(&fctx, pw->progress.font);
    fe = XmFontListNextEntry(fctx);
    fnt = (XFontStruct *) XmFontListEntryGetFont(fe, &ft);
    XmFontListFreeFontContext(fctx);

    gcv.font = fnt->fid;
    gcv.foreground = pw->primitive.foreground;
    gcv.background = pw->core.background_pixel;
    gcv.function = GXcopy;

    if (pw->progress.labelgc == 0 || force == True) {
        if (pw->progress.labelgc != 0)
          XFreeGC(XtDisplay(w), pw->progress.labelgc);

        pw->progress.labelgc = XCreateGC(XtDisplay(w), XtWindow(w),
                                       gcvm|GCFont, &gcv);
    }

    if (pw->progress.numgc == 0 || force == True) {
        if (pw->progress.numgc != 0)
          XFreeGC(XtDisplay(w), pw->progress.numgc);

        pw->progress.numgc = XCreateGC(XtDisplay(w), XtWindow(w),
                                       gcvm|GCFont, &gcv);
    }

    gcv.foreground = pw->primitive.foreground ^ pw->core.background_pixel;
    gcv.background = 0;
    gcv.function = GXxor;

    if (pw->progress.bargc == 0 || force == True) {
        if (pw->progress.bargc != 0)
          XFreeGC(XtDisplay(w), pw->progress.bargc);

        pw->progress.bargc = XCreateGC(XtDisplay(w), XtWindow(w),
                                       gcvm, &gcv);
    }
}

static void
#ifndef _NO_PROTO
_XmuttProgressBarRedraw(Widget w)
#else
_XmuttProgressBarRedraw(w)
Widget w;
#endif
{
    XmuttProgressBarWidget pw;
    unsigned long n;
    Position x, y;
    Dimension nswd, nsht;
    XmString ns;
    double a, b;
    XRectangle bar;
    XPoint br[3], tl[3];
    char nbuf[8];

    if (!XtIsRealized(w))
      return;

    pw = (XmuttProgressBarWidget) w;

    /*
     * Draw the label string if it exists.
     */
    if (pw->progress.label != 0) {
        x = _XmuttProgressBarLeftX(pw);
        y = _XmuttProgressBarTopY(pw);
        bar.x = x;
        bar.y = y;
        bar.width = pw->progress.label_width;
        bar.height = pw->progress.label_height;
        if (pw->progress.dir == XmuttProgressBarLeftToRight)
          XmStringDrawImage(XtDisplay(w), XtWindow(w), pw->progress.font,
                            pw->progress.label, pw->progress.labelgc, x, y,
                            bar.width, XmALIGNMENT_BEGINNING,
                            XmSTRING_DIRECTION_L_TO_R, &bar);
        else {
            x = _XmuttProgressBarRightX(pw) - pw->progress.label_width;
            bar.x = x;
            XmStringDrawImage(XtDisplay(w), XtWindow(w), pw->progress.font,
                              pw->progress.label, pw->progress.labelgc, x, y,
                              bar.width, XmALIGNMENT_BEGINNING,
                              XmSTRING_DIRECTION_R_TO_L, &bar);
        }
    }

    tl[0].x = pw->progress.bar.x - 2;
    tl[0].y = pw->progress.bar.y + pw->progress.bar.height + 1;
    tl[1].x = tl[0].x;
    tl[1].y = pw->progress.bar.y - 2;
    tl[2].x = _XmuttProgressBarRightX(pw) - 1;
    tl[2].y = tl[1].y;

    br[0].x = tl[2].x;
    br[0].y = tl[2].y + 1;
    br[1].x = br[0].x;
    br[1].y = tl[0].y;
    br[2].x = tl[0].x;
    br[2].y = br[1].y;

    XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.bottom_shadow_GC,
               tl, 3, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.top_shadow_GC,
               br, 3, CoordModeOrigin);

    tl[0].x++;
    tl[0].y--;
    tl[1].x++;
    tl[1].y++;
    tl[2].x--;
    tl[2].y++;

    br[0].x--;
    br[0].y++;
    br[1].x--;
    br[1].y--;
    br[2].x++;
    br[2].y--;

    XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.bottom_shadow_GC,
               tl, 3, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), pw->primitive.top_shadow_GC,
               br, 3, CoordModeOrigin);

    /*
     * Draw the bar with the current values.
     */
    if (pw->progress.minval < pw->progress.maxval &&
        pw->progress.currval != 0) {
        bar.y = pw->progress.bar.y + 1;
        bar.height = pw->progress.bar.height - 2;

        /*
         * Figure out what the width is supposed to be.
         */
        if (pw->progress.currval == pw->progress.maxval)
          bar.width = pw->progress.bar.width - 2;
        else {
            n = (unsigned long)
                ((((float) pw->progress.currval) /
                  ((float) (pw->progress.maxval - pw->progress.minval))) *
                 100.0);
            bar.width = (pw->progress.bar.width * n) / 100;
        }

        if (pw->progress.dir == XmuttProgressBarLeftToRight)
          bar.x = pw->progress.bar.x + 1;
        else
          bar.x = _XmuttProgressBarRightX(pw) - 2 - bar.width - 1;

        /*
         * Draw the percentage text.
         */
        a = (double) (pw->progress.currval - pw->progress.minval);
        b = (double) (pw->progress.maxval - pw->progress.minval);
        n = (unsigned long) ((a / b) * 100.0);
        sprintf(nbuf, "%ld%%", n);
        ns = XmStringCreateSimple(nbuf);
        XmStringExtent(pw->progress.font, ns, &nswd, &nsht);
        XmStringDrawImage(XtDisplay(w), XtWindow(w), pw->progress.font, ns,
                          pw->progress.numgc,
                          pw->progress.bar.x +
                          ((pw->progress.bar.width >> 1) - (nswd >> 1)),
                          pw->progress.bar.y +
                          ((pw->progress.bar.height >> 1) - (nsht >> 1)) + 1,
                          nswd, XmALIGNMENT_BEGINNING,
                          XmSTRING_DIRECTION_L_TO_R,
                          &pw->progress.bar);
        XmStringFree(ns);

        if (pw->progress.number_only == False)
          /*
           * And redraw draw the progress bar itself.
           */
          XFillRectangle(XtDisplay(w), XtWindow(w), pw->progress.bargc,
                         bar.x, bar.y, bar.width, bar.height);
        pw->progress.lastwidth = bar.width;
    }
}

static void
#ifndef _NO_PROTO
_XmuttProgressBarUpdate(Widget w)
#else
_XmuttProgressBarUpdate(w)
Widget w;
#endif
{
    XmuttProgressBarWidget pw;
    unsigned long n;
    Dimension nswd, nsht, nsx;
    XmString ns;
    double a, b;
    char nbuf[8];
    XRectangle bar;

    if (!XtIsRealized(w))
      return;

    pw = (XmuttProgressBarWidget) w;

    bar.y = pw->progress.bar.y + 1;
    bar.height = pw->progress.bar.height - 2;

    if (pw->progress.currval == pw->progress.maxval)
      bar.width = pw->progress.bar.width - 2;
    else {
        n = (unsigned long)
            ((((float) pw->progress.currval) /
              ((float) (pw->progress.maxval - pw->progress.minval))) * 100.0);
        bar.width = (pw->progress.bar.width * n) / 100;
    }

    if (pw->progress.dir == XmuttProgressBarLeftToRight)
      bar.x = pw->progress.bar.x + 1;
    else
      bar.x = _XmuttProgressBarRightX(pw) - 2 - bar.width - 1;

    a = (double) (pw->progress.currval - pw->progress.minval);
    b = (double) (pw->progress.maxval - pw->progress.minval);
    n = (unsigned long) ((a / b) * 100.0);

    sprintf(nbuf, "%ld%%", n);
    ns = XmStringCreateSimple(nbuf);
    XmStringExtent(pw->progress.font, ns, &nswd, &nsht);
    nsx = pw->progress.bar.x +
        ((pw->progress.bar.width >> 1) - (nswd >> 1));

    /*
     * Only update every indicated percent.
     */
    if (n % pw->progress.percent == 0 && n != pw->progress.lastpercent) {
        pw->progress.lastpercent = n;

        /*
         * Draw the percentage text.
         */
        XmStringDrawImage(XtDisplay(w), XtWindow(w), pw->progress.font, ns,
                          pw->progress.numgc, nsx,
                          pw->progress.bar.y +
                          ((pw->progress.bar.height >> 1) - (nsht >> 1)) + 1,
                          nswd, XmALIGNMENT_BEGINNING,
                          XmSTRING_DIRECTION_L_TO_R,
                          &pw->progress.bar);
        if (pw->progress.number_only == False &&
            bar.x + bar.width > nsx && bar.x + pw->progress.lastwidth >= nsx) {
            if (bar.x + pw->progress.lastwidth >= nsx + nswd)
              XFillRectangle(XtDisplay(w), XtWindow(w), pw->progress.bargc,
                             nsx, bar.y, nswd, bar.height);
            else
              XFillRectangle(XtDisplay(w), XtWindow(w), pw->progress.bargc,
                             nsx, bar.y,
                             ((bar.x + pw->progress.lastwidth) - nsx),
                             bar.height);
        }
    }

    /*
     * Free up the number string.
     */
    XmStringFree(ns);

    /*
     * Add check to see if new value is smaller or larger.
     */
    if (pw->progress.number_only == False &&
        bar.width != pw->progress.lastwidth)
      XFillRectangle(XtDisplay(w), XtWindow(w), pw->progress.bargc,
                     bar.x + pw->progress.lastwidth, bar.y,
                     bar.width - pw->progress.lastwidth, bar.height);

    pw->progress.lastwidth = bar.width;
}

/*************************************************************************
 *
 * Start class methods.
 *
 *************************************************************************/

static void
#ifndef _NO_PROTO
ClassInitialize(void)
#else
ClassInitialize()
#endif
{
}

static void
#ifndef _NO_PROTO
ClassPartInitialize (WidgetClass wc)
#else
ClassPartInitialize (wc)
WidgetClass wc;
#endif
{
}

static void
#ifndef _NO_PROTO
Initialize(Widget req, Widget newone, ArgList args, Cardinal *num_args)
#else
Initialize(req, newone, args, num_args)
Widget req, newone;
ArgList args;
Cardinal *num_args;
#endif
{
    XmuttProgressBarWidget pw = (XmuttProgressBarWidget) newone;
    Dimension wd, ht;
    unsigned long tmp;
    XFontStruct *fs;
    XmFontContext fctx;
    XmFontListEntry fe;
    XmFontType ft;

    if (pw->progress.font == 0) {
        if ((fs = XLoadQueryFont(XtDisplay(newone), "fixed")) == 0) {
            fprintf(stderr,
                    "XmuttProgressBar: unable to load the \"fixed\" font.\n");
            exit(1);
        }
        pw->progress.font = XmFontListCreate(fs, XmSTRING_DEFAULT_CHARSET);
    }

    /*
     * Initialize the GCs.
     */
    pw->progress.labelgc = pw->progress.numgc = pw->progress.bargc = 0;

    /*
     * Get the first font to determine the max font bounds for the progress
     * bar size.
     */
    XmFontListInitFontContext(&fctx, pw->progress.font);
    fe = XmFontListNextEntry(fctx);
    fs = (XFontStruct *) XmFontListEntryGetFont(fe, &ft);
    XmFontListFreeFontContext(fctx);

    pw->progress.percheight = fs->max_bounds.ascent + fs->max_bounds.descent;

    /*
     * Determine the extents of the label string if supplied.
     */
    if (pw->progress.label != 0) {
        /*
         * Copy the string.
         */
        pw->progress.label = XmStringCopy(pw->progress.label);
        XmStringExtent(pw->progress.font, pw->progress.label,
                       &pw->progress.label_width, &pw->progress.label_height);
    } else
      pw->progress.label_width = pw->progress.label_height = 0;

    pw->progress.lastwidth = 0;
    pw->progress.lastpercent = 0;

    if (pw->progress.percent == 0 || pw->progress.percent > 100)
      pw->progress.percent = 10;

    if (pw->progress.minval > pw->progress.maxval) {
        tmp = pw->progress.minval;
        pw->progress.minval = pw->progress.maxval;
        pw->progress.maxval = tmp;
    }

    _XmuttProgressBarPreferredGeometry(pw, &wd, &ht);
    pw->core.width = wd;
    pw->core.height = ht;

    _XmuttProgressBarSetBarGeometry(pw);
}

static void
#ifndef _NO_PROTO
Resize(Widget w)
#else
Resize(w)
Widget w;
#endif
{
    XmuttProgressBarWidget pw;

    pw = (XmuttProgressBarWidget) w;

    _XmuttProgressBarSetBarGeometry(pw);
}

static void
#ifndef _NO_PROTO
Realize(Widget w, Mask *valueMask, XSetWindowAttributes *attributes)
#else
Realize(w, valueMask, attributes)
Widget w;
Mask *valueMask;
XSetWindowAttributes *attributes;
#endif
{
    XtCreateWindow(w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
                   *valueMask, attributes);

    _XmuttProgressBarMakeGCs(w, False);
}

static void
#ifndef _NO_PROTO
Destroy(Widget w)
#else
Destroy(w)
Widget w;
#endif
{
    XmuttProgressBarWidget pw;

    pw = (XmuttProgressBarWidget) w;

    if (pw->progress.labelgc != 0)
      XFreeGC(XtDisplay(w), pw->progress.labelgc);
    if (pw->progress.numgc != 0)
      XFreeGC(XtDisplay(w), pw->progress.numgc);
    if (pw->progress.bargc != 0)
      XFreeGC(XtDisplay(w), pw->progress.bargc);
    pw->progress.labelgc = pw->progress.numgc = pw->progress.bargc = 0;

    if (pw->progress.font != 0)
      XmFontListFree(pw->progress.font);
    pw->progress.font = 0;

    if (pw->progress.label != 0)
      XmStringFree(pw->progress.label);
    pw->progress.label = 0;
}

static void
#ifndef _NO_PROTO
Redisplay(Widget w, XEvent *event, Region region)
#else
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
#endif
{
    XmuttProgressBarWidget pw;

    pw = (XmuttProgressBarWidget) w;

    _XmuttProgressBarShadowWindow(pw);
    _XmuttProgressBarRedraw(w);
}

static Boolean
#ifndef _NO_PROTO
SetValues(Widget curr, Widget req, Widget newone,
          ArgList args, Cardinal *num_args)
#else
SetValues(curr, req, newone, args, num_args)
Widget curr, req, newone;
ArgList args;
Cardinal *num_args;
#endif
{
    XmuttProgressBarWidget opw, npw;
    Dimension wd, ht;
    Boolean redo;

    opw = (XmuttProgressBarWidget) curr;
    npw = (XmuttProgressBarWidget) newone;

    redo = False;
    if (opw->progress.label != npw->progress.label) {
        XmStringFree(opw->progress.label);
        npw->progress.label = XmStringCopy(npw->progress.label);
        XmStringExtent(npw->progress.font, npw->progress.label,
                       &npw->progress.label_width,
                       &npw->progress.label_height);
        _XmuttProgressBarPreferredGeometry(npw, &wd, &ht);
        if (wd != npw->core.width || ht != npw->core.height) {
            npw->core.width = wd;
            npw->core.height = ht;
            _XmuttProgressBarSetBarGeometry(npw);
        }
        redo = True;
    }

    if (opw->progress.minval != npw->progress.minval ||
        opw->progress.maxval != npw->progress.maxval)
      redo = True;

    if (redo == False && opw->progress.currval != npw->progress.currval) {
        if (npw->progress.currval > npw->progress.maxval)
          npw->progress.currval = npw->progress.maxval;
        if (XtIsRealized(newone))
          _XmuttProgressBarUpdate(newone);
    }

    return redo;
}

/*************************************************************************
 *
 * Start action procedures.
 *
 *************************************************************************/

static XtResource resources[] = {
    {
        XmNtraversalOn,
        XmCTraversalOn,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmPrimitiveWidget, primitive.traversal_on),
        XmRImmediate,
        (XtPointer) False,
    },
    {
        XmNhighlightThickness,
        XmCHighlightThickness,
        XmRDimension,
        sizeof(Dimension),
        XtOffset(XmPrimitiveWidget, primitive.highlight_thickness),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNmarginWidth,
        XmCMarginWidth,
        XmRHorizontalDimension,
        sizeof(Dimension),
        XtOffset(XmuttProgressBarWidget, progress.mwidth),
        XmRImmediate,
        (XtPointer) 2,
    },
    {
        XmNmarginHeight,
        XmCMarginHeight,
        XmRVerticalDimension,
        sizeof(Dimension),
        XtOffset(XmuttProgressBarWidget, progress.mheight),
        XmRImmediate,
        (XtPointer) 2,
    },
    {
        XmNminValue,
        XmCMinValue,
        XmRInt,
        sizeof(unsigned long),
        XtOffset(XmuttProgressBarWidget, progress.minval),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNmaxValue,
        XmCMaxValue,
        XmRInt,
        sizeof(unsigned long),
        XtOffset(XmuttProgressBarWidget, progress.maxval),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNcurrentValue,
        XmCCurrentValue,
        XmRInt,
        sizeof(unsigned long),
        XtOffset(XmuttProgressBarWidget, progress.currval),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNreportEveryPercent,
        XmCReportEveryPercent,
        XmRInt,
        sizeof(unsigned long),
        XtOffset(XmuttProgressBarWidget, progress.percent),
        XmRImmediate,
        (XtPointer) 10,
    },
    {
        XmNfontList,
        XmCFontList,
        XmRFontList,
        sizeof(XmFontList),
        XtOffset(XmuttProgressBarWidget, progress.font),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNlabelString,
        XmCLabelString,
        XmRString,
        sizeof(XmString),
        XtOffset(XmuttProgressBarWidget, progress.label),
        XmRImmediate,
        (XtPointer) 0,
    },
    {
        XmNupdateDirection,
        XmCUpdateDirection,
        XmRInt,
        sizeof(int),
        XtOffset(XmuttProgressBarWidget, progress.dir),
        XmRImmediate,
        (XtPointer) XmuttProgressBarLeftToRight,
    },
    {
        XmNpercentOnly,
        XmCPercentOnly,
        XmRBoolean,
        sizeof(Boolean),
        XtOffset(XmuttProgressBarWidget, progress.number_only),
        XmRImmediate,
        (XtPointer) False,
    },
};

externaldef(xmuttprogbarclassrec) XmuttProgBarClassRec xmuttProgBarClassRec = {
    {
	(WidgetClass) &xmPrimitiveClassRec,	/* superclass		*/
	"XmuttProgressBar",			/* class_name		*/
	sizeof(XmuttProgBarRec),		/* widget_size		*/
        ClassInitialize,			/* class_initialize	*/
	ClassPartInitialize,			/* chained class init	*/
	FALSE,					/* class_inited		*/
	Initialize,				/* initialize		*/
        NULL,					/* initialize hook	*/
	Realize,				/* realize		*/
	NULL,					/* actions		*/
	0,					/* num_actions		*/
	resources,				/* resources		*/
	XtNumber(resources),			/* num_resources	*/
	NULLQUARK,				/* xrm_class		*/
	TRUE,					/* compress_motion	*/
	XtExposeCompressMaximal,		/* compress_exposure	*/
        TRUE,					/* compress enter/exit	*/
	FALSE,					/* visible_interest	*/
	Destroy,				/* destroy		*/
	Resize,					/* resize		*/
	Redisplay,				/* expose		*/
	(XtSetValuesFunc) SetValues,		/* set_values		*/
        NULL,					/* set values hook	*/
        XtInheritSetValuesAlmost,		/* set values almost	*/
        NULL,					/* get values hook	*/
	NULL,					/* accept_focus		*/
        XtVersion,				/* version		*/
        NULL,					/* callback offsetlst	*/
        NULL,					/* default trans	*/
	XtInheritQueryGeometry,			/* query geo proc	*/
	XtInheritDisplayAccelerator,		/* display accelerator	*/
        NULL,					/* extension record reserved*/
    },

    {						/* XmPrimitive		*/
        (XtWidgetProc) _XtInherit,		/* border_highlight	*/
        (XtWidgetProc) _XtInherit,		/* border_unhighlight	*/
        0,					/* translations		*/
        0,					/* arm_and_activate	*/
        0,					/* syn resources	*/
        0,					/* num syn_resources	*/
        0,					/* extension		*/
    },

    {						/* XmuttProgressBar	*/
        0,					/* Extension record pointer.*/
    }
};

externaldef(xmuttprogressbarwidgetclass)
    WidgetClass xmuttProgressBarWidgetClass = (WidgetClass)
    &xmuttProgBarClassRec;

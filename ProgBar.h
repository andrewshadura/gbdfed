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
#ifndef _h_ProgBar
#define _h_ProgBar

/*
 * $Id: ProgBar.h,v 1.5 2001/09/19 21:00:42 mleisher Exp $
 */

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A macro for prototypes.
 */
#undef __
#ifndef _NO_PROTO
#define __(x) x
#else
#define __(x) ()
#endif

/*
 * The lowest value.
 */
#define XmNminValue "minValue"
#define XmCMinValue "MinValue"

/*
 * The highest value.
 */
#define XmNmaxValue "maxValue"

/*
 * The current value.
 */
#define XmNcurrentValue "currentValue"
#define XmCCurrentValue "CurrentValue"

/*
 * Update status bar every certain percentage.
 */
#define XmNreportEveryPercent "reportEveryPercent"
#define XmCReportEveryPercent "ReportEveryPercent"

#define XmNupdateDirection "updateDirection"
#define XmCUpdateDirection "UpdateDirection"

#define XmNpercentOnly "percentOnly"
#define XmCPercentOnly "PercentOnly"

#define XmuttProgressBarLeftToRight 0
#define XmuttProgressBarRightToLeft 1

externalref WidgetClass xmuttProgressBarWidgetClass;

typedef struct _XmuttProgBarClassRec *XmuttProgressBarWidgetClass;
typedef struct _XmuttProgBarRec      *XmuttProgressBarWidget;

#ifndef XmuttIsProgressBar
#define XmuttIsProgressBar(w) XtIsSubclass(w, xmuttProgressBarWidgetClass)
#endif /* XmuttIsProgressBar */

#undef __

#ifdef __cplusplus
}
#endif

#endif /* _h_ProgBar */

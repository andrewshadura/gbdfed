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

/*
 * $Id: htext.h,v 1.37 2004/02/08 23:59:00 mleisher Exp $
 */
static char *about_text = "\
                 XmBDFEditor 4.7\n\
              mleisher@crl.nmsu.edu\n\
                       09 February 2004\n\
\n\
XmBDFEditor is a BDF font editor that supports\n\
these main features:\n\
\n\
  o  Multiple fonts can be loaded from the command line.\n\
  o  Multiple fonts can be open at the same time.\n\
  o  Cutting and pasting glyphs between fonts.\n\
  o  Multiple glyph bitmap editors can be open at the same time.\n\
  o  Cutting and pasting between glyph bitmap editors.\n\
  o  Automatic correction of certain metrics when a font is loaded.\n\
  o  Generation of XLFD font names for fonts without XLFD names.\n\
  o  Update an XLFD font name from the font properties.\n\
  o  Update the font properties from an XLFD font name.\n\
  o  Font property editor.\n\
  o  Font comment editor.\n\
  o  Supports unencoded glyphs (ENCODING of -1).\n\
  o  Display of glyph encodings in octal, decimal, or hex.\n\
  o  Builtin on-line help.\n\
  o  Imports PK/GF fonts.\n\
  o  Imports HBF (Han Bitmap Font) fonts.\n\
  o  Imports Linux console fonts (PSF, CP, and FNT).\n\
  o  Imports Sun console fonts (vfont format).\n\
  o  Imports fonts from the X server.\n\
  o  Imports Windows FON/FNT fonts.\n\
  o  Imports OpenType fonts and collections.\n\
  o  Exports Linux console PSF2 fonts.\n\
  o  Exports HEX fonts (http://czyborra.com/unifont).\n\
  o  Edits gray scale fonts with 2 or 4 bits per pixel.\n\
\n\
XmBDFEditor works on X11R5 or X11R6 with Motif\n\
1.2.*, Motif 2.*, OpenMotif, or Lesstif.\n\
";

static char *program_text = "\
By default, xmbdfed automatically collects\n\
comments that are saved with the font, it\n\
preserves the unencoded glyphs, and it will\n\
attempt to make some metrics corrections\n\
automatically.  These options can be set on the\n\
command line.\n\
\n\
More than one font can be specified on the command\n\
line.\n\
\n\
The command line parameters for xmbdfed are:\n\
\n\
 -nc     - no comments\n\
 -nm     - no metrics corrections\n\
 -nu     - no unencoded glyphs\n\
 -np     - do not pad character cell bitmaps\n\
 -bp     - allow blank pages\n\
 -ed     - no \"Really Exit?\" dialog\n\
 -ps n   - set point size\n\
 -hres n - set horizontal resolution\n\
 -vres n - set vertical resolution\n\
 -res n  - set both resolutions\n\
 -sp s   - set the font spacing (\"p\", \"m\", or \"c\")\n\
 -bpp n  - set the font bits per pixel (1, 2, or 4)\n\
 -eol e  - set the default end of line char(s) (\"u\", \"d\", \"m\")\n\
 -g code - set the initial glyph code (decimal, hex, or octal)\n\
 -pb     - use watch cursor instead of progress bar\n\
 -cb base- set the code base for glyph codes (\"oct\", \"dec\", \"hex\")\n\
";

static char *fgrid_text = "\
At the top of each editor window there are some\n\
fields and buttons.  These are:\n\
\n\
  The \"Font\" text field is where the font name\n\
  is set so it can be edited.\n\
\n\
  The \"Glyph\" field is a label that provides\n\
  some information about glyph name, encoding, and\n\
  metrics when a glyph is selected.  When a range\n\
  of glyphs are selected, this field displays the\n\
  start and end codes of the range.\n\
\n\
  The push buttons are used to navigate through the\n\
  glyph pages.  The \"Previous Page\" and \"Next Page\"\n\
  buttons normally skip glyph pages that are empty,\n\
  but that can be changed using the \"Setup\" dialog.\n\
\n\
  The \"Page\" field indicates the current glyph page\n\
  and also allows a specific page number to be entered.\n\
  Once a page number is entered, pressing the Return\n\
  key will cause the Font Grid to shift to that page.\n\
  The page number entered is expected to be a decimal\n\
  number.\n\
\n\
  The \"Code\" field is provided for situations where\n\
  the page number is not known, but the encoding is\n\
  known.  The encoding entered in this field must be\n\
  in the base (8, 10, or 16) that is currently being\n\
  used to display glyph encodings (see the \"View\"\n\
  menu below).  Once the encoding is entered, pressing\n\
  the Return key will cause the Font Grid to shift to\n\
  the page containing the encoding.\n\
\n\
The main window of each font editor is called the\n\
Font Grid.  All of the Font Grid's have a special\n\
clipboard used for passing glyphs around.  This\n\
clipboard is called FONTGRID_CLIPBOARD.\n\
\n\
When a glyph has been modified either by the user or\n\
by automatic metrics corrections when the font is loaded,\n\
the glyph code above the glyph cell will be highlighted.\n\
\n\
Font Grid Menus\n\
---------------\n\
\n\
File\n\
====\n\
  New <Ctrl+N>\n\
    This creates a new font and asks for the point\n\
    size, resolution, and font spacing first.\n\
\n\
  Open <Ctrl+O>\n\
    This opens a new font in the current Font\n\
    Grid.\n\
\n\
  Save <Ctrl+S>\n\
    Save the current font.\n\
\n\
  Save As <Ctrl+W>\n\
    Save the current font with some other name.\n\
\n\
  Import\n\
  ======\n\
\n\
    PK/GF Font <Ctrl+K>\n\
      Import a Metafont PK or GF font.\n\
\n\
    Console Font <Ctrl+L>\n\
      Import a Linux or Sun console (binary) font.\n\
      If the font is a CP (Linux codepage) font, this\n\
      will load all three point sizes of the font,\n\
      display the 16pt font and create editors for the\n\
      14pt and 8pt fonts. If the font is a PSF1 or PSF2\n\
      font, the embedded mapping table is loaded as well.\n\
\n\
    HBF Font <Ctrl+H>\n\
      Import an HBF font.\n\
\n\
    Windows Font <Ctrl+B>\n\
      Import a Windows FON/FNT font.  This will also\n\
      import fonts from .EXE and .DLL files.\n\
\n\
    OpenType Font <Ctrl+Y>\n\
      Import an OpenType (.otf), TrueType font (.ttf) or\n\
      collection (.ttc).\n\
\n\
    Server Font <Ctrl+G>\n\
      This will import a font from the X server.\n\
\n\
  Export\n\
  ======\n\
\n\
    PSF <Ctrl+F>\n\
      This will export the current BDF font or the current selection\n\
      of glyphs to a PSF2 font.\n\
\n\
      During the export, an option menu will let you select whether you\n\
      want to:\n\
\n\
        A. Export the font with it's Unicode mappings.\n\
\n\
        B. Export just the glyphs.\n\
\n\
        C. Export just the Unicode mappings in the simple\n\
           ASCII form used by the psfaddtable(1) program.\n\
\n\
      Only the first 512 glyphs will be exported from\n\
      the font.\n\
\n\
    HEX\n\
      This will export the current BDF font into the\n\
      HEX format (See the \"HEX Font Notes\" entry).\n\
\n\
  Exit/Close <Ctrl+F4>\n\
    Exit the program if this is the primary Font\n\
    Grid or simply hide the current Font Grid\n\
    window.\n\
\n\
    The key binding for this can be changed in the\n\
    configuration file.  See the \"Configuration File\"\n\
    help section.\n\
\n\
Edit\n\
====\n\
  Copy <Ctrl+C>\n\
    This copies the current selection to the Font\n\
    Grid clipboard.\n\
\n\
  Cut <Ctrl+X>\n\
    This copies the current selection to the Font\n\
    Grid clipboard and then deletes the selection.\n\
\n\
  Paste <Ctrl+V>\n\
    This replaces the glyphs starting at the currently\n\
    selected position with the Font Grid clipboard.\n\
\n\
  Overlay <Ctrl+Shift+V>\n\
    This merges the Font Grid clipboard with the glyphs\n\
    starting at the currently selected position.\n\
    The names of the modified glyphs are not changed.\n\
\n\
  Insert <Ctrl+Meta+V>\n\
    This inserts the Font Grid cliboard in front of the\n\
    currently selected position.\n\
\n\
  Properties <Ctrl+P>\n\
    This invokes the font property editor.\n\
\n\
  Comments <Ctrl+M>\n\
    This invokes the font comments editor.\n\
\n\
  Font Info <Ctrl+I>\n\
    This invokes a dialog that allows changes\n\
    to some of the font information so these\n\
    values do not have to be changed using the\n\
    property editor.  These values include the\n\
    default character, font device width (for\n\
    monowidth and character cell fonts), font\n\
    ascent and descent, font vertical and\n\
    horizontal resolution, and the font spacing.\n\
\n\
  Font Name\n\
  =========\n\
\n\
    Make XLFD Name\n\
      If the font does not have an XLFD name, this\n\
      will save the current font name in the\n\
      \"_ORIGINAL_FONT_NAME\" font property and then\n\
      generate an XLFD name for the font.\n\
\n\
    Update Name From Properties\n\
      This will update the XLFD font name fields from\n\
      the font property list.\n\
\n\
    Update Properties From Name\n\
      This will update the font properties from the\n\
      XLFD font name.\n\
\n\
    Update Average Width\n\
      This will update the average width field of the\n\
      XLFD font name and will update the\n\
      \"AVERAGE_WIDTH\" font property as a side effect.\n\
\n\
  Name Glyphs\n\
  ===========\n\
    Unicode Names\n\
      This option will rename all the glyphs using names\n\
      from a Unicode Character Database file set in the\n\
      config file or from the \"Setup->Other Options\"\n\
      dialog.\n\
\n\
    Unicode Values\n\
    ==============\n\
      This option will rename all the glyphs with 16-bit\n\
      hexadecimal values prefixed with \"0x\", \"U+\", or \"\\u\".\n\
\n\
  Test Glyphs <Ctrl+Z>\n\
    This will toggle the glyph test dialog on or off for\n\
    the editor.  When this is active, clicking on a glyph\n\
    in any Font Grid will also add it to the glyph test\n\
    dialog.  When changes are made to a glyph or the font\n\
    bounding box, the glyph test dialog will be updated\n\
    accordingly.\n\
\n\
    The glyph test dialog provides a toggle to turn the\n\
    baseline on or off and another toggle to draw from right\n\
    to left instead of left to right.\n\
\n\
  Setup <Ctrl+T>\n\
    This will invoke the dialog to edit various settings\n\
    used by the editor.\n\
\n\
View\n\
====\n\
  Unencoded <Ctrl+E>\n\
    If the font has unencoded glyphs (\"ENCODING\"\n\
    field is -1), this will toggle between\n\
    displaying the unencoded and encoded glyphs.\n\
\n\
  Code Base\n\
  =========\n\
    Octal\n\
      This option will display glyph encodings in\n\
      octal (base 8).\n\
\n\
    Decimal\n\
      This option will display glyph encodings in\n\
      decimal (base 10).\n\
\n\
    Hexadecimal\n\
      This option will display glyph encodings in\n\
      hexadecimal (base 16).\n\
\n\
  Next Page <PageUp>\n\
    This will show the next glyph page.\n\
\n\
  Previous Page <PageDown>\n\
    This will show the previous glyph page.\n\
\n\
  First Page <Ctrl+PageDown>\n\
    This will show the first glyph page.\n\
\n\
  Last Page <Ctrl+PageUp>\n\
    This will show the last glyph page.\n\
\n\
  Other Page <Ctrl+Shift+S>\n\
    This will toggle between the current glyph page\n\
    and the last page that was viewed.\n\
\n\
  Vertical/Horizontal View <Ctrl+Q>\n\
    This will toggle the FontGrid between showing the\n\
    glyphs horizontally (default) or vertically.\n\
\n\
  Messages <Ctrl+A>\n\
    This will show messages generated when corrections\n\
    to the font metrics are done or when errors are\n\
    encountered.\n\
\n\
Operations\n\
==========\n\
  Translate <Ctrl+D>\n\
    This will bring up the dialog for entering the X\n\
    offset and Y offset used to translate the glyph to\n\
    a new location.\n\
\n\
    The option of translating the selected glyphs or all\n\
    of the glyphs is provided.\n\
\n\
  Rotate <Ctrl+R>\n\
    This will bring up the dialog for entering the\n\
    rotation angle.  The rotation is limited to between\n\
    ± 1° and 359°.\n\
\n\
    The option of rotating the selected glyphs or all\n\
    of the glyphs is provided.\n\
\n\
  Shear <Ctrl+J>\n\
    This will bring up the dialog for entering the\n\
    angle of the shear.  The shear is limited to between\n\
    ± 1° and 45°.\n\
\n\
    The option of rotating the selected glyphs or all\n\
    of the glyphs is provided.\n\
\n\
  Embolden <Ctrl+Shift+B>\n\
    This will bring up the dialog for choosing whether\n\
    to embolden the selected glyphs or to embolden all\n\
    glyphs.\n\
\n\
    To \"embolden\" means to make bold.\n\
\n\
Editors\n\
=======\n\
  New <Ctrl+N>\n\
    This will cause a new editor to be created with\n\
    the point size, resolution, and bits per pixel\n\
    set from the config file, command line, or from\n\
    the Setup dialog.\n\
\n\
  [editor list]\n\
    The remaining menu items are all the Font\n\
    Grid's that have been created.  Choosing one\n\
    will force that window to be made visible and\n\
    also put that window on top.\n\
\n\
Font Grid Translations\n\
----------------------\n\
0..9\n\
  Typing digits will accumulate a count which is\n\
  applied to movement done with the arrow and page keys.\n\
\n\
Left\n\
  This will move the single cell selection left.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Right\n\
  This will move the single cell selection right.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Up\n\
  This will move the single cell selection up.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Down\n\
  This will move the single cell selection down.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+Left\n\
  This will extend the selection to the left.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+Right\n\
  This will extend the selection to the right.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+Up\n\
  This will extend the selection up a row or column,\n\
  depending on the display orientation, horizontal or\n\
  vertical.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+Down\n\
  This will extend the selection down a row or column,\n\
  depending on the display orientation, horizontal or\n\
  vertical.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
PageUp\n\
  This will switch to the next page of glyphs.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
PageDown\n\
  This will switch to the previous page of glyphs.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Home\n\
  This will switch to the first page of glyphs.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
End\n\
  This will switch to the last page of glyphs.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+PageUp\n\
  This will extend the selection to the next page.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+PageDown\n\
  This will extend the selection to the previous page.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+Home\n\
  This will extend the selection to the first page that\n\
  has glyphs.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Shift+End\n\
  This will extend the selection to the last page that\n\
  has glyphs.\n\
  If a decimal number is typed before this, this\n\
  operation will be done that number of times.\n\
\n\
Button1Down\n\
  This will start selecting glyphs.  If Button1 is\n\
  double-clicked, it will edit the current glyph.\n\
\n\
Button1Motion\n\
  This will extend the selected glyphs.\n\
\n\
Button1Up\n\
  This will end glyph selection.\n\
\n\
Shift+Button1Down\n\
  This will adjust the glyphs already selected by\n\
  adding or removing glyphs from the selection.\n\
\n\
Button2Down\n\
  This will paste the glyphs on the Font Grid\n\
  clipboard at the glyph position under the mouse.\n\
  If the paste is done in the unencoded glyph area,\n\
  the glyphs will simply be appended to the end.\n\
  The unencoded glyph area is simply a container\n\
  for unused glyphs.\n\
\n\
Shift+Button2Down\n\
  This will insert the glyphs on the Font Grid\n\
  clipboard in front of the glyphs starting at the\n\
  position under the mouse.  Any glyphs moved past\n\
  the 0xffff encoding will be moved to the unencoded\n\
  area so they are not lost.  This action will always\n\
  insert, no matter what mode the font grid is in.\n\
\n\
Ctrl+Button2Down\n\
  This will merge (overlay) the glyphs being pasted with\n\
  the glyphs that are in the range of the glyphs being pasted.\n\
  If a merge is done in the unencoded glyph area, the glyphs\n\
  will simply be appended and not merged (overlayed).\n\
\n\
Button3Down\n\
  This will copy the selected glyphs to the Font\n\
  Grid clipboard.\n\
\n\
Return\n\
  This will invoke a glyph editor for the current glyph.\n\
\n\
Ctrl+Return\n\
  This will cause the end selection callback to be called.\n\
  The effect in the xmbdfed program is to send the glyph\n\
  to the glyph test dialog if it is open.\n\
\n\
Copy\n\
  This will copy the selected glyphs to the Font\n\
  Grid clipboard.\n\
\n\
Cut\n\
  This will copy the selected glyphs to the Font\n\
  Grid clipboard and then delete the glyphs.\n\
\n\
Paste\n\
  This will paste the glyphs on the Font Grid\n\
  clipboard at the currently selected glyph\n\
  position.\n\
\n\
Delete\n\
  This will copy the selected glyphs to the Font\n\
  Grid clipboard and then delete the glyphs.\n\
\n\
BackSpace\n\
  This will copy the selected glyphs to the Font\n\
  Grid clipboard and then delete the glyphs.\n\
\n\
Double clicking with Button1 will invoke the Glyph\n\
Editor for the current glyph.\n\
\n\
Other Font Grid Features\n\
------------------------\n\
The font name can be edited in the Font Grid and\n\
page switching can be done with the buttons on the\n\
Font Grid.\n\
";

static char *gedit_text = "\
The Glyph Editor provides a simple bitmap editor\n\
designed to edit glyph bitmaps and other glyph\n\
information.  The Glyph Editors all use a special\n\
clipboard used to pass bitmaps between the Glyph\n\
Editors.  This clipboard is called\n\
GLYPHEDIT_CLIPBOARD.\n\
\n\
The only limit on the number of Glyph Editors that\n\
can be open at one time is the amount of memory.\n\
\n\
Glyph Editor Menus\n\
------------------\n\
\n\
File\n\
====\n\
  Update <Ctrl+S>\n\
    This will update the Font Grid with the\n\
    modified glyph.\n\
\n\
  Update and Next <Ctrl+U>\n\
    This will update the FontGrid with the\n\
    modified glyph and move to the next glyph.\n\
\n\
  Update and Previous <Ctrl+B>\n\
    This will update the FontGrid with the\n\
    modified glyph and move to the previous glyph.\n\
\n\
  Import XBM <Ctrl+I>\n\
    This will pop up a file selection dialog\n\
    so an X Bitmap file (.xbm extension) can be\n\
    selected to be imported into the glyph editor.\n\
\n\
    The imported bitmap is handled the same way\n\
    as bitmaps pasted.  They are inserted as a\n\
    selection with the move operation active and\n\
    do not destroy any bitmap that already exists\n\
    in the editor.\n\
\n\
  Export XBM <Ctrl+O>\n\
    This will pop up a file selection dialog\n\
    so that the glyph image can be exported as\n\
    an X Bitmap file (.xbm extension).\n\
\n\
  Close <Ctrl+F4>\n\
    This will close the Glyph Editor.\n\
\n\
Edit\n\
====\n\
  Reload <Ctrl+L>\n\
    This will reload the glyph and discard any\n\
    changes in the glyph.\n\
\n\
  Resize BBX <Ctrl+R>\n\
    This will allow changing the sizes of the\n\
    glyph bounding box including the left/right\n\
    bearings and the glyph ascent/descent.  If\n\
    this change causes the glyph bounding box to\n\
    be larger than the font bounding box, the\n\
    font bounding box will be resized when the\n\
    glyph is saved next.\n\
\n\
  Copy <Ctrl+C>\n\
    This will copy the currently selected portion\n\
    of the bitmap to the Glyph Editor clipboard.\n\
\n\
  Cut <Ctrl+X>\n\
    This will copy the currently selected portion\n\
    of the bitmap to the Glyph Editor clipboard\n\
    and then delete the selection.\n\
\n\
  Paste <Ctrl+V>\n\
    This will paste the Glyph Editor clipboard\n\
    into the current Glyph Editor with the\n\
    top-left coordinate of the bitmap on the\n\
    clipboard pasted at the location of the mouse.\n\
    If the bitmap is too big to fit if it is\n\
    pasted at the mouse location, the bitmap will\n\
    be shifted until it fits completely in the\n\
    Glyph Editor.\n\
\n\
  Select All <Ctrl+A>\n\
    This will select the whole glyph bitmap.\n\
\n\
  Next Glyph <Ctrl+N>\n\
    This will move the Glyph Editor to the next\n\
    glyph position in the Font Grid.  If the\n\
    current glyph has been modified, a save prompt\n\
    will appear before moving to the next glyph.\n\
\n\
  Previous Glyph <Ctrl+P>\n\
    This will move the Glyph Editor to the previous\n\
    glyph position in the Font Grid.  If the\n\
    current glyph has been modified, a save prompt\n\
    will appear before moving to the previous glyph.\n\
\n\
  Edit PSF Unicode Mappings <Ctrl+F>\n\
    This will pop up a list of Unicode mappings\n\
    associated with the glyph.  The list can be edited\n\
    and once the Apply button has been pressed, the\n\
    the changes will be applied to the glyph in the\n\
    font proper.\n\
\n\
    If you do not close this editor, it will be updated\n\
    with Unicode mappings if you move to the next or\n\
    previous glyph.\n\
\n\
Operation\n\
=========\n\
  Draw <Ctrl+D>\n\
    Change the Glyph Editor into Draw mode.\n\
\n\
  Move <Ctrl+M>\n\
    Change the Glyph Editor into Move mode.  Move\n\
    mode allows selecting a portion of the glyph\n\
    bitmap and moving it to another location.\n\
\n\
  Copy <Ctrl+Y>\n\
    Change the Glyph Editor into Copy mode.  Copy\n\
    mode allows copying a portion of the glyph\n\
    bitmap and moving it to another location.\n\
\n\
  Rotate <Ctrl+T>\n\
    This will invoke the rotation dialog that\n\
    allows the degrees of rotation to be specified.\n\
    Rotation can be between 1 and 359 degrees.\n\
\n\
  Shear <Ctrl+E>\n\
    This will invoke the shear dialog that allows\n\
    the degrees of horizontal shear to be specified.\n\
    Other names for shearing are obliquing or slanting.\n\
    Shearing is allowed between 1 and 45 degrees.\n\
\n\
  Embolden <Ctrl+H>\n\
    This will embolden the current glyph in a simple\n\
    way within the width of the glyph.\n\
\n\
Glyph Editor Translations\n\
-------------------------\n\
\n\
ButtonDown\n\
  Depending on the mode of the Glyph Editor, this\n\
  will start drawing, start selecting for a Move\n\
  or start selecting for a Copy.  When in Draw\n\
  mode, Button1 will set pixels, Button2 will\n\
  invert pixels, and Button3 will clear pixels.\n\
\n\
  When in Move or Copy mode and a selection\n\
  exists, pressing Button1 within the selection\n\
  will \"grab\" the selection so it can be Moved or\n\
  Copied.  Pressing Button3 after a selection has\n\
  been made will copy the selection to the Glyph Editor\n\
  clipboard.\n\
\n\
Shift+Button2Down\n\
  This will paste the contents of the Glyph Editor\n\
  clipboard into the glyph editor at the location\n\
  of the mouse.\n\
\n\
Motion\n\
  This will continue the operation started with\n\
  \"ButtonDown\" as well as report the current mouse\n\
  coordinates in Cartesian form relative to the\n\
  bounding box for the glyph.\n\
\n\
ButtonUp\n\
  This will end the operation started with\n\
  \"ButtonDown\".\n\
\n\
Copy\n\
  This will copy the selected bitmap to the Glyph\n\
  Editor clipboard.\n\
\n\
Cut\n\
  This will copy the selected bitmap to the Glyph\n\
  Editor clipboard and then delete it.\n\
\n\
Paste\n\
  This will paste the Glyph Editor clipboard at\n\
  the mouse position.\n\
\n\
Right\n\
  This will shift the glyph bitmap toward (but not\n\
  past) the right edge of the bitmap grid.\n\
\n\
Left\n\
  This will shift the glyph bitmap toward (but not\n\
  past) the left edge of the bitmap grid.\n\
\n\
Up\n\
  This will shift the glyph bitmap toward (but not\n\
  past) the top edge of the bitmap grid.\n\
\n\
Down\n\
  This will shift the glyph bitmap toward (but not\n\
  past) the bottom edge of the bitmap grid.\n\
  \n\
9\n\
  This will rotate the glyph bitmap 90°\n\
  counter-clockwise.\n\
\n\
0\n\
  This will rotate the glyph bitmap 90°\n\
  clockwise.\n\
  \n\
-\n\
  This will flip the glyph bitmap around the\n\
  vertical axis (horizontal flip).\n\
  \n\
=\n\
  This will flip the glyph bitmap around the\n\
  horizontal axis (vertical flip).\n\
\n\
\"comma\" or \"z\"\n\
  This will select the previous color or\n\
  cycle back to the last color.\n\
\n\
\"period\" or \"x\"\n\
  This will select the next color or cycle\n\
  to the first color.\n\
\n\
Other Metrics Features\n\
----------------------\n\
If the font defines the X height and the Cap height,\n\
these can be displayed in the GlyphEditors by turning\n\
them on or off individually from the\n\
\"Setup->Other Options\" menu in the FontGrids.  These\n\
values affect all GlyphEditors.\n\
\n\
Other Glyph Editor Features\n\
---------------------------\n\
\n\
In addition to editing the glyph bitmap, the Glyph\n\
Editor also allows editing of the glyph name and\n\
setting its device width (BDF \"DWIDTH\" field).  To\n\
get more aesthetic spacing between glyphs, this\n\
value can be set explicitly.  The glyph name\n\
should be a maximum of 14 characters.\n\
\n\
The Glyph Editor also provides a simple toolbox\n\
that has buttons to switch between operations and to\n\
perform various bitmap manipulations.\n\
\n\
Pressing one of the shift buttons in the toolbox\n\
will repeat the shift operation if the mouse button\n\
is held down longer than 100 milliseconds.  This is\n\
not configurable at the moment.\n\
\n\
If the font uses two or four bits per pixel, a strip\n\
of colors will be presented down the left side of\n\
the toolbox.  These colors can be selected with the\n\
mouse or can be chosen using the keys mentioned\n\
above in the GlyphEditor translations.  At the moment\n\
the GlyphEditor must have the focus for the keys to\n\
work.\n\
";

static char *conf_text = "\
XmBDFEditor can be configured using an external\n\
file.  This file is always assumed to be in the\n\
home directory and is called \".xmbdfedrc\".\n\
\n\
This file sets default values which can be changed\n\
and saved from the editor.  The default values\n\
apply to either the editor itself or the font\n\
management system.\n\
\n\
For the configuration options, the following types\n\
are used:\n\
\n\
  <boolean>\n\
    A <boolean> value can be \"0\", \"false\", \"no\",\n\
    \"1\", \"true\", or \"yes\".  Boolean values are\n\
    case insensitive.\n\
\n\
  <labelstring>\n\
    A <labelstring> value is a string used as a label\n\
    for some of the options.\n\
\n\
  <atom>\n\
    An <atom> is basically a string.\n\
\n\
  <cardinal>\n\
    A <cardinal> value is an unsigned 32-bit\n\
    integer value.\n\
\n\
  <integer>\n\
    An <integer> is a signed 32-bit integer\n\
    value.\n\
\n\
  <property-name>\n\
    A <property-name> is any name that conforms to\n\
    the XLFD definition of a user-defined\n\
    property.  Basically, the property name must\n\
    start with the underscore character (_).\n\
    These names are conventionally in upper case\n\
    with the underscore character used to provide\n\
    \"spaces\" between parts of the name.\n\
\n\
  <property-type>\n\
    A <property-type> can be one of \"atom\",\n\
    \"cardinal\", or \"integer\" (see above).\n\
\n\
  <font-spacing>\n\
    A <font-spacing> value can be one of\n\
    \"proportional\", \"monowidth\", or\n\
    \"charactercell\".\n\
\n\
    If an unknown <font-spacing> value is\n\
    encountered, the default value is\n\
    \"proportional\".\n\
\n\
  <codebase>\n\
    A <codebase> value can be one of \"octal\",\n\
    \"decimal\", or \"hexadecimal\".  It can also\n\
    be shortened to just the first letter.  Any\n\
    unknown <codebase> values are assumed to be\n\
    \"hexadecimal\".\n\
\n\
  <translation>\n\
    A <translation> is a valid X Toolkit translation\n\
    string.\n\
\n\
  <filename>\n\
    A <filename> is the name of a file including or\n\
    excluding a partial or full path to the file.\n\
\n\
  <eolname>\n\
    An <eolname> value can be one of \"unix\" (^J),\n\
    \"dos\" (^M^J), or \"mac\" (^M).  This value is\n\
    used when saving BDF fonts.\n\
\n\
The following configuration options can appear in\n\
the configuration file:\n\
\n\
  XmBDFEditor Configuration File Options\n\
  ======================================\n\
    code_base <codebase> [default: \"hex\"]\n\
\n\
      By default, set the code base used to display the\n\
      glyph encodings to base 16, or hex.  This option can\n\
      be set to \"oct\", \"dec\", or \"hex\".\n\
\n\
   skip_blank_pages <boolean> [default: \"true\"]\n\
\n\
     By default, the editor will skip font pages\n\
     that do not have any glyphs when the \"Next\n\
     Page\" and \"Previous Page\" buttons are used.\n\
\n\
     If this option is set to \"false\", the \"Next\n\
     Page\" and \"Previous Page\" buttons will simply\n\
     move to the next or previous page, even if\n\
     they do not have glyphs on them.\n\
\n\
     This feature is only available in the configuration\n\
     file and on the command line.\n\
\n\
   really_exit <boolean> [default: \"true\"]\n\
\n\
     By default, the editor will always present the\n\
     \"Really Exit?\" dialog when exiting.  If this\n\
     option is set to \"false\", then the dialog\n\
     not be presented when exiting.\n\
\n\
     This feature is only available in the configuration\n\
     file and on the command line.\n\
\n\
   grid_overwrite_mode <boolean> [default: \"true\"]\n\
\n\
     By default, pasting glyphs into a Font Grid will\n\
     overwrite glyphs that are in the same range as the\n\
     glyphs being pasted.  If this option is set to\n\
     \"false\", pasting glyphs into a Font Grid will\n\
     move glyphs to make room for the glyphs being pasted.\n\
     Any glyphs moved that have an encoding larger than\n\
     65535 will be moved to the unencoded area.\n\
\n\
     This feature is toggled using the \"Setup\" dialog.\n\
\n\
   close_accelerator_text <labelstring> [default: \"Ctrl+F4\"]\n\
\n\
     The default close accelerator text shown on the\n\
     Close/Exit menu options of the FontGrid's and\n\
     GlyphEditor's is \"Ctrl+F4\".  This option changes the\n\
     label string on those menu options.  This option should\n\
     be used in conjunction with the next option.\n\
\n\
     This feature is only available in the configuration\n\
     file.\n\
\n\
   close_accelerator <translation> [default: \"Ctrl<Key>F4\"]\n\
\n\
     The default accelerator for the Close/Exit menu options\n\
     in the FontGrid's and GlyphEditor's can sometimes be\n\
     awkward for various reasons.  This option allows that\n\
     accelerator to be changed.  This option should be used\n\
     in conjunction with the previous option.\n\
\n\
     This feature is only available in the configuration\n\
     file.\n\
\n\
   unicode_name_file <filename>\n\
\n\
     This specifies a file that contains entries in the UCDB\n\
     (Unicode Character Database) format.  When glyphs are named\n\
     using Unicode names, this file provides the mapping between\n\
     the code and the name.  This file is assumed to be sorted by\n\
     code.\n\
\n\
     This feature is set using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
   adobe_name_file <filename>\n\
\n\
     This specifies a file that contains entries in the Adobe\n\
     Glyph List format (see Adobe for details).  When glyphs\n\
     are named using the Adobe names, this file provides the\n\
     mapping between the code and the name.  This file is assumed\n\
     to be sorted by name and not by code.\n\
\n\
     This feature is set using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
   pixel_size <integer> [default: \"10\"]\n\
\n\
     The GlyphEditors will use a square of size 10x10 to\n\
     represent a pixel in the glyph bitmap.  If the glyph\n\
     bitmap causes the GlyphEditor grid to be larger than\n\
     1/2 the display height, then this value will be reduced\n\
     until the bitmap fits within 1/2 the display size or\n\
     until a pixel size of 2 is reached.\n\
\n\
     The GlyphEditor's will always attempt to use this default\n\
     value first before reducing the size, if reducing the size\n\
     is needed.\n\
\n\
     This feature is set using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
   show_cap_height <boolean> [default: \"false\"]\n\
\n\
     If the font has the CAP_HEIGHT property defined,\n\
     this flag will toggle the display of this height\n\
     in the GlyphEditors.\n\
\n\
     The CAP_HEIGHT is shown as a solid horizontal line\n\
     above the baseline in the same color as the baseline.\n\
\n\
     This feature is toggled using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
   show_x_height <boolean> [default: \"false\"]\n\
\n\
     If the font has the X_HEIGHT property defined,\n\
     this flag will toggle the display of this height\n\
     in the GlyphEditors.\n\
\n\
     The X_HEIGHT is shown as a solid horizontal line\n\
     above the baseline in the same color as the baseline.\n\
\n\
     This feature is toggled using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
   font_grid_horizontal <boolean> [default: \"true\"]\n\
\n\
     This option determines if the glyphs are displayed\n\
     horizontally or vertically.  The default is to display\n\
     horizontally.\n\
\n\
     This default orientation option can only be set in\n\
     the configuration file at the moment.\n\
\n\
   power2 <boolean> [default: \"true\"]\n\
\n\
     This option determines whether the font grid always\n\
     adjusts the rows and columns to powers of 2.  This\n\
     option can only be set in the configuration file at\n\
     the moment.\n\
\n\
   progress_bar <boolean> [default: \"true\"]\n\
\n\
     This option turns the progress bar either on or off.\n\
     In remote situations, the progress bar can slow things\n\
     down a lot.\n\
\n\
     This feature is toggled using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
   percentage_only <boolean> [default: \"true\"]\n\
\n\
     This option turns the progress bar updating on and\n\
     off.  If it is \"true\", then the percentage number\n\
     on the progress bar will be the only thing updated\n\
     and not the bar itself.\n\
\n\
     This feature is toggled using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
  generate_sbit_metrics <boolean> [default: \"false\"]\n\
\n\
    This option determines whether an SBIT metrics file\n\
    will be written after the BDF font has been written.\n\
    NOTE: This is for use with the SBIT utility from\n\
    Microsoft.\n\
\n\
    This feature is toggled using the \"Setup->Other Options\"\n\
    dialog.\n\
\n\
  General Font Configuration Options\n\
  ==================================\n\
\n\
    make_backups <boolean> [default: \"true\"]\n\
\n\
      By default, the editor will make backups when\n\
      it saves fonts.  The filename will have .BAK\n\
      on the end.  This option will turn this feature\n\
      off.\n\
\n\
     This feature is toggled using the \"Setup\" dialog.\n\
\n\
    correct_metrics <boolean> [default: \"true\"]\n\
\n\
      By default, the editor will make certain\n\
      corrections to the font metrics when a font\n\
      is loaded.  If this value is \"false\", then\n\
      no metrics corrections will be performed\n\
      when a font is loaded.\n\
\n\
     This feature is toggled using the \"Setup\" dialog.\n\
\n\
    keep_unencoded <boolean> [default: \"true\"]\n\
\n\
      By default, the editor will keep any\n\
      unencoded glyphs that are found when a font\n\
      is loaded.  An unencoded glyph will have an\n\
      \"ENCODING\" field set to -1.  If this option\n\
      is set to \"false\", then all unencoded glyphs\n\
      will be ignored.\n\
\n\
     This feature is toggled using the \"Setup\" dialog.\n\
\n\
    keep_comments <boolean> [default: \"true\"]\n\
\n\
      By default, the editor will keep all\n\
      comments found in the font file.  This\n\
      allows them to be edited.\n\
\n\
      If this option is set to \"false\", all\n\
      comments except those that appear in the\n\
      font properties list will be ignored.  The\n\
      comments in the font properties list are\n\
      kept because they sometimes contain useful\n\
      notes about the properties.\n\
\n\
     This feature is toggled using the \"Setup\" dialog.\n\
\n\
    pad_character_cells <boolean> [default: \"true\"]\n\
\n\
      By default, the editor will pad each glyph\n\
      bitmap from fonts with \"charactercell\"\n\
      spacing.  This means that each glyph has\n\
      blank bits added around it until it matches\n\
      the font bounding box exactly.\n\
\n\
      This option is \"true\" by default because\n\
      that seems to be what most people expect,\n\
      based on numerous \"charactercell\" fonts that\n\
      were checked.\n\
\n\
      However, since the BDF format is sometimes\n\
      used as a transfer format between programs,\n\
      this option can be set to \"false\" to reduce\n\
      the size of the BDF font.\n\
\n\
      In either case, the fonts will display\n\
      correctly, and metrics calculations should\n\
      not be affected.\n\
\n\
     This feature is toggled using the \"Setup\" dialog.\n\
\n\
    font_spacing <font-spacing> [default: \"proportional\"]\n\
\n\
      By default, the editor will create new fonts\n\
      with proportional spacing.  This option can\n\
      be set to \"monowidth\" or \"charactercell\" if\n\
      \"proportional\" is not wanted.\n\
\n\
     This feature is set using the \"Setup\" dialog.\n\
\n\
    point_size <cardinal> [default: \"12\"]\n\
\n\
      By default, the editor will create new fonts\n\
      with point size 12.\n\
\n\
     This feature is set using the \"Setup\" dialog.\n\
\n\
    horizontal_resolution <integer> [default: \"display\"]\n\
\n\
      By default, the editor will determine the\n\
      horizontal resolution based on the X display\n\
      being used by the editor.  For instance,\n\
      this value is often \"90\" for Sun displays.\n\
\n\
     This feature is set using the \"Setup\" dialog.\n\
\n\
    vertical_resolution <integer> [default: \"display\"]\n\
\n\
      By default, the editor will determine the\n\
      vertical resolution based on the X display\n\
      being used by the editor.  For instance,\n\
      this value is often \"90\" for Sun displays.\n\
\n\
     This feature is set using the \"Setup\" dialog.\n\
\n\
    bits_per_pixel <integer> [default: \"1\"]\n\
\n\
      By default, the editor works with fonts that\n\
      have one bit per pixel.  But it also supports\n\
      two or four bits per pixel.  This option sets\n\
      the default bits per pixel when creating new\n\
      fonts.\n\
\n\
     This feature is set using the \"Setup\" dialog.\n\
\n\
    hint_truetype_glyphs <boolean> [default: \"true\"]\n\
\n\
      By default, importing OpenType fonts will have\n\
      the glyphs hinted.  If this option is set to\n\
      \"false\", the glyphs will not be hinted.\n\
\n\
     This feature is toggled using the \"Setup->Other Options\"\n\
     dialog.\n\
\n\
    eol <eolname> [default: \"unix\"]\n\
\n\
      By default, BDF fonts will be saved with a Unix\n\
      end-of-line character (^J).  This option can be\n\
      \"unix\", \"dos\", or \"mac\".\n\
\n\
     This feature is set using the \"Setup\" dialog.\n\
\n\
    property <property-name> <property-type>\n\
\n\
      To support user-defined properties, the\n\
      editor provides the facility to define them\n\
      in the configuration file in order to\n\
      interpret them correctly (atom, cardinal, or\n\
      integer) when editing fonts containing\n\
      user-defined properties.\n\
\n\
      If an unknown user-defined property is\n\
      encountered in a font, it always defaults to\n\
      a <property-type> of \"atom\".\n\
\n\
      There is no limit to the number of\n\
      \"property\" options set in the configuration\n\
      file.\n\
";

static char *otf_text = "\
If this program was compiled with the FreeType\n\
library to support importing OpenType fonts\n\
(.otf extension), TrueType fonts (.ttfextension), and\n\
TrueType collections (.ttc extension),\n\
when importing a font or collection, a dialog\n\
will be presented to allow you to choose a single font,\n\
the platform, and encoding.  If you are loading a\n\
TrueType collection, there will be more than one font\n\
to choose from.\n\
\n\
OpenType fonts imported will use the point size and\n\
resolution set in your ~/.xmbdfedrc file or the defaults\n\
set at compile time if you do not have a ~/.xmbdfedrc.\n\
\n\
The point size and resolution can also be set before\n\
importing using the \"Setup\" dialog.\n\
\n\
The renderer used to import OpenType fonts is\n\
available from http://www.freetype.org.\n\
";

static char *fnt_text = "\
When a Windows .FON, .EXE, or .DLL file resource\n\
table holds more than one font, you are presented with\n\
a list of fonts to choose from.  You can select as many\n\
of them as you wish or simply import them all using\n\
the \"Import All\" button.\n\
";

static char *psf_text ="\
This editor imports both PSF1 and PSF2 Linux\n\
console fonts.  It only exports the newer PSF2\n\
fonts, usually with a \".psfu\" extension.\n\
\n\
When a PSF1 or PSF2 font is imported, it can have\n\
a Unicode mapping table following the glyphs.\n\
This mapping table can be modified through the\n\
Glyph Editor from the Edit menu or by pressing\n\
Ctrl+F.  There are two kinds of mappings:\n\
\n\
   1. A single Unicode character.\n\
\n\
   2. A sequence of more than one Unicode character.\n\
\n\
When editing the mappings, the common convention\n\
is to prefix Unicode values with \"U+\" although\n\
\"0x\" is accepted as well. When entering more than\n\
one character code, the codes are expected to be\n\
separated by at least one space.\n\
\n\
Unicode mappings are included during cut and paste\n\
operations, allowing them to be transfered to other\n\
fonts or other locations within one font.\n\
\n\
There is no support currently for attaching an\n\
external mapping table to a font.  This can be\n\
done outside this editor using the\n\
\"psfaddtable(1)\" program on Linux.\n\
";

static char *hex_text = "\
The HEX format is described in more detail at:\n\
http://czyborra.com/unifont/.\n\
\n\
HEX fonts are fonts that have two glyph widths,\n\
with the smaller width being half the size of\n\
the larger width.\n\
";

static char *setup_text = "\
The Setup dialog is used to set options for the\n\
Font Grid, Glyph Editors, and the BDF fonts.\n\
The options that can be set are:\n\
\n\
  Insert Mode or Overwrite Mode\n\
\n\
    This affects the way glyphs are pasted in the\n\
    Font Grids.\n\
\n\
    The default mode, Overwrite, will simply replace\n\
    everything in the range of the glyphs being pasted\n\
    from the FONTGRID_CLIPBOARD.\n\
\n\
    If Insert Mode is on, then all glyphs from the\n\
    insert point forward are shifted forward to make\n\
    room for the glyphs being pasted.  Since this shift\n\
    changes the encoding of the glyphs moved forward,\n\
    it is possible for glyphs to get encodings greater\n\
    than the maximum for BDF fonts (65535).  If this\n\
    happens, then the glyphs that have encodings greater\n\
    than 65535 are moved to the unencoded area and are\n\
    accessible by switching to the unencoded pages with\n\
    \"Ctrl+E\".\n\
\n\
  Correct Metrics, Keep Unencoded, Keep Comments,\n\
  Pad Character Cells, and EOL.\n\
\n\
    If Correct Metrics is set, certain metrics will be\n\
    adjusted when a BDF font is loaded.  If this option\n\
    is not set, then the editor will assume the metrics\n\
    are correct.\n\
\n\
    If Keep Unencoded is set, glyphs with an ENCODING\n\
    value of -1 will be stored in the unencoded pages\n\
    which are accessible by switching using \"Ctrl+E\".\n\
    If this is not set, then unencoded glyphs will be\n\
    ignored when a BDF font is loaded.\n\
\n\
    If Keep Comments is set, then comments collected will\n\
    be stored with the font and written out before the\n\
    PROPERTIES section of the BDF font.  Comments can be\n\
    edited with the Comment editor invoked with \"Ctrl+M\".\n\
    If this is not set, then comments are simply ignored\n\
    when the BDF font is loaded.\n\
\n\
    If Pad Character Cells is set, then character cell\n\
    fonts will have all glyphs padded with zero bits\n\
    until they fit the font bounding box exactly.\n\
    If this is not set, then the bitmaps for each\n\
    glyph will be clipped to the minimum rectangle\n\
    needed to hold the bitmap.\n\
\n\
    The EOL menu sets the end-of-line string used\n\
    when BDF fonts are saved.  The three most common\n\
    options are provided: Unix, DOS, and Mac EOL's.\n\
\n\
  Point Size, Horizontal and Vertical Resolution\n\
\n\
    These fields allow these three values to be set\n\
    for new fonts created with \"Ctrl+N\" and also are\n\
    used to set the desired size and resolution of\n\
    OpenType fonts when they are imported.\n\
\n\
  Proportional, Monowidth, and Character Cell\n\
\n\
    These set the font spacing type for new fonts\n\
    created using \"Ctrl+N\".\n\
\n\
  Bits Per Pixel\n\
\n\
    This option allows setting the bits per pixel\n\
    value for new fonts created using \"Ctrl+N\".\n\
    Selecting two or four bits per pixel also\n\
    enables the Color button used to edit the colors\n\
    used for gray scale fonts.\n\
\n\
Along the bottom are some buttons.  These buttons are:\n\
\n\
  Update\n\
\n\
    When one or more options have changed, this button\n\
    becomes active.  If it is pressed, it will actually\n\
    update the changed values.  If it is not pressed and\n\
    the dialog is closed, none of the changes made will\n\
    take affect.\n\
\n\
  Save Setup\n\
\n\
    This button will become active after the Update\n\
    button was pressed to actually change the setup\n\
    options.  Pressing this button will write all the\n\
    setup values to a file in the home directory.  This\n\
    file is called \".xmbdfedrc\".\n\
\n\
  Color\n\
\n\
    If the bits per pixel for new fonts is two or four\n\
    this button will invoke the color editor.  This editor\n\
    allows adjusting what the different colors look like\n\
    on the current screen.  These colors are really only\n\
    useful for testing with the current screen and may\n\
    actually look different on other screens.\n\
\n\
    In the color editor, a button at the bottom toggles\n\
    between the colors for two and four bits per pixel.\n\
\n\
  Close\n\
\n\
    This closes the Setup dialog.  If any changes were\n\
    made and not applied with Update (or saved), then\n\
    the changes are discarded.\n\
\n\
  Other Options\n\
\n\
    This button opens another setup dialog to set\n\
    more options.  The close button at the bottom\n\
    simply closes the window.  These options are:\n\
\n\
      Hint OpenType Glyphs\n\
\n\
        If this option is set, the OpenType renderer\n\
        will use the hints in the font if they exist.\n\
        This can sometimes make clearer glyphs at small\n\
        point sizes.\n\
\n\
      Unicode Glyph Name File\n\
\n\
        This field is for setting the name of a file\n\
        in the Unicode Character Database format.  This\n\
        file will supply glyph names from the file.\n\
        The \"Browse\" button allows a file to be\n\
        selected using a FileSelection dialog.\n\
\n\
        The Unicode Character Database format is basically\n\
        a set of semi-colon separated fields on a single line\n\
        with the first field being 4 hex digits representing\n\
        the encoding of the glyph and the next field being\n\
        the name of the glyph.  These are the only two\n\
        fields used by this editor.  The entries in this\n\
        file are expected to be sorted in ascending order\n\
        by encoding.\n\
\n\
     Adobe Glyph Name File\n\
\n\
        This field is for setting the name of a file\n\
        in the Adobe Glyph List format.  This file will\n\
        supply glyph names from the file.\n\
\n\
        The Adobe Glyph List format is basically a set of\n\
        semi-colon separated fields with the first field\n\
        being 4 hex digits representing the encoding of the\n\
        glyph and the next field being the Adobe name of the\n\
        glyph.  The entries are not expected to be in ascending\n\
        order by glyph code.\n\
\n\
     Pixel Size\n\
\n\
       This option allows the pixel size in the GlyphEditors\n\
       to be set to different sizes.  This effectively zooms\n\
       the glyph in the editor.  If this is changed and the\n\
       Update button is pressed, all open GlyphEditors will\n\
       be updated with the new value.\n\
\n\
     Show Cap Height and Show X Height\n\
\n\
       These two options can be set to make the cap height\n\
       and the x height lines show up in the GlyphEditors.\n\
       These will only show up if they are defined in the\n\
       font or are set using the property editor.  If these\n\
       are changed and the Update button is pressed, all\n\
       open GlyphEditors will be udated to show the lines.\n\
\n\
    Show Progress Bar\n\
\n\
      This option will turn the progress bar on or off.  In\n\
      some cases, the progress bar will cause a lot of traffic\n\
      over a slow link.  This option exists to change to simply\n\
      using a watch cursor.\n\
\n\
    Progress Bar Percentage\n\
\n\
      This option toggles the progress bar updating between\n\
      showing just the percentage or showing the percentage\n\
      and the progress bar behind it.  This is useful for\n\
      slow connections where lots of network traffic can\n\
      really slow things down.\n\
\n\
    SBIT Metrics\n\
\n\
      This option toggles the generation of an SBIT metrics\n\
      file which can be incorporated into a OpenType font\n\
      using the SBIT utility provided by Microsoft.\n\
";

static char *color_text = "\
The editor supports BDF fonts with 2 and 4 bits per pixel.\n\
This is stored in the BDF file as a number on the end of\n\
the SIZE line.\n\
\n\
This is a non-standard extension and currently can only be\n\
used to create bitmap fonts (strikes or EBSC entries) that\n\
will be embedded in OpenType fonts.\n\
\n\
See the Microsoft Web pages for details on the SBITS\n\
utility.\n\
";

static char *tips_text = "\
Useful Tips:\n\
\n\
To add space glyphs to proportional fonts, simply set\n\
the Device Width field to the desired width of the blank.\n\
When the font is saved, a bitmap is automatically\n\
generated for it.\n\
";

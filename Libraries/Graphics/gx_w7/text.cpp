/*____________________________________________________________________
|
| File: text.cpp
|
| Description: Contains C functions to draw graphics text.
|
| Functions: gxLoadFont
|            gxLoadFontData
|             Load_GX_Font
|             Load_GEM_Font
|             Load_Metawindow_Font
|            gxSaveFont
|            gxFreeFont
|            gxSetFont
|            gxGetFont
|            gxScaleFont
|            gxSetFontAttributes
|            gxGetFontAttributes
|            gxGetFontWidth
|            gxGetFontHeight
|            gxGetStringWidth
|            gxDrawText
|            gxDrawTextOverwrite
|            gxGetString
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include "dp.h"
#include "bitmap.h"  
#include "image.h"
#include "meta.h"

/*___________________
|
| Type definitions
|__________________*/

typedef struct {                /* GEM header version 2 and 3   */
  short     fid;                /* font id                      */
  short     psize;              /* point size                   */
  char      fntname[32];        /* font name                    */
  short     minch;              /* minimum character            */
  short     maxch;              /* maximum character            */
  short     topline;            /* top line distance            */
  short     ascent;             /* ascent distance              */
  short     halfline;           /* halfline distance            */
  short     descent;            /* descent distance             */
  short     botline;            /* bottom line distance         */
  short     maxwidth;           /* max character width          */
  short     cellsize;           /* max cell size                */
  short     leftofs;            /* left offset                  */
  short     rightofs;           /* right offset                 */
  short     thicken;            /* thicken pixels, for bolding  */
  short     ulwidth;            /* underline width              */
  short     lightmask;          /* lightening mask              */
  short     skewmask;           /* skew mask                    */
  short     flags;              /* font flags                   */
  byte     *hotptr;             /* horiz offset table ptr       */
  unsigned *cotptr;             /* char offset table ptr        */
  long      bufptr;             /* data buffer ptr              */
  short     fwidth;             /* sum of all character widths  */
  short     fheight;            /* number of character rows     */
} GEMHeader;

/*___________________
|
| Function prototypes
|__________________*/

static gxFont *Load_GX_Font (
  byte     *origin,             // filename or buffer
  unsigned  origin_size,        // # bytes in buffer (if buffer)
  int       data_type );        // file or buffer
static gxFont *Load_GEM_Font (
  byte     *origin,             // filename or buffer
  unsigned  origin_size,        // # bytes in buffer (if buffer)
  int       data_type );        // file or buffer
static gxFont *Load_Metawindow_Font (
  byte     *origin,             // filename or buffer
  unsigned  origin_size,        // # bytes in buffer (if buffer)
  int       data_type );        // file or buffer

/*___________________
|
| Defines
|__________________*/

#define VERSION_1 1

#define DATA_TYPE_FILE 1
#define DATA_TYPE_BUFF 2

/* Macro for Load_???_Font() routines */
#define FONT_FREAD(_dst_,_size_,_n_,_stream_)   \
  {                                             \
    if (data_type == DATA_TYPE_FILE)            \
      fread (_dst_, _size_, _n_, _stream_);     \
    else {                                      \
      memcpy (_dst_, origin_ptr, _size_*_n_);   \
      origin_ptr += (_size_*_n_);               \
    }                                           \
  }

#define FONT_FSEEK(_stream_,_offset_,_whence_)                          \
  {                                                                     \
    if (data_type == DATA_TYPE_FILE)                                    \
      fseek (_stream_, _offset_, _whence_);                             \
    else {                                                              \
      switch (_whence_) {                                               \
        case SEEK_SET: origin_ptr = origin + _offset_;                  \
                       break;                                           \
        case SEEK_CUR: origin_ptr += _offset_;                          \
                       break;                                           \
        case SEEK_END: origin_ptr = (origin+origin_size-1) - _offset_;  \
                       break;                                           \
      }                                                                 \
    }                                                                   \
  }

/*___________________
|
| Global variables
|__________________*/

static gxFont *gx_Current_font = NULL;

/*____________________________________________________________________
|
| Function: gxLoadFont
|
| Outputs: Loads a font from a file and sets up a font data structure.
|       Returns a handle to the font.  To draw text in the font, call
|       gxSetFont() to make this font the current font. On any error,
|       returns NULL.
|
| Description: Dynamic memory is allocated to load the font.  The caller
|       must call gxFreeFont() to free this memory.
|___________________________________________________________________*/

gxFont *gxLoadFont (int font_type, char *filename)
{
  gxFont *font = 0;

  switch (font_type) {
    case gxFONT_TYPE_GX:
                font = Load_GX_Font ((byte *)filename, 0, DATA_TYPE_FILE);
                break;
    case gxFONT_TYPE_GEM:
                font = Load_GEM_Font ((byte *)filename, 0, DATA_TYPE_FILE);
                break;
    case gxFONT_TYPE_METAWINDOW:
                font = Load_Metawindow_Font ((byte *)filename, 0, DATA_TYPE_FILE);
                break;
  }

  return (font);
}

/*____________________________________________________________________
|
| Function: gxLoadFontData
|
| Outputs: Loads a font from a data bufer and sets up a font data structure.
|       Returns a handle to the font.  To draw text in the font, call
|       gxSetFont() to make this font the current font. On any error,
|       returns NULL.
|
| Description: Dynamic memory is allocated to load the font.  The caller
|       must call gxFreeFont() to free this memory.
|___________________________________________________________________*/

gxFont *gxLoadFontData (int font_type, byte *buff, unsigned buff_size)
{
  gxFont *font = 0;

  switch (font_type) {
    case gxFONT_TYPE_GX:
                font = Load_GX_Font (buff, buff_size, DATA_TYPE_BUFF);
                break;
    case gxFONT_TYPE_GEM:
                font = Load_GEM_Font (buff, buff_size, DATA_TYPE_BUFF);
                break;
    case gxFONT_TYPE_METAWINDOW:
                font = Load_Metawindow_Font (buff, buff_size, DATA_TYPE_BUFF);
                break;
  }

  return (font);
}

/*____________________________________________________________________
|
| Function: Load_GX_Font
|
| Outputs: Loads a GX bitmap font from a file.  These will be files
|       with extension 'GXFONT'.  Returns a ptr to the font struct or
|       NULL on any error.
|___________________________________________________________________*/

static gxFont *Load_GX_Font (
  byte     *origin,             // filename or buffer
  unsigned  origin_size,        // # bytes in buffer (if buffer)
  int       data_type )         // file or buffer
{
  unsigned size;
  byte *origin_ptr = 0;
  gxFont *font = NULL;
  FILE *fp = NULL;
  int ok = TRUE;

/*____________________________________________________________________
|
| Open file.
|___________________________________________________________________*/

  if (data_type == DATA_TYPE_FILE) {
    fp = fopen ((char *)origin, "rb");
    if (fp == NULL)
      ok = FALSE;
  }
  else
    origin_ptr = origin;

/*____________________________________________________________________
|
| Allocate memory for gxFont struct.
|___________________________________________________________________*/

  if (ok) {
    font = (gxFont *) malloc (sizeof(gxFont));
    if (font == NULL)
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Initialize font header with data.
|___________________________________________________________________*/

  if (ok) {
    FONT_FREAD ((void *)&(font->header), sizeof(gxFontHeader), 1, fp);
    font->type               = gxFONT_TYPE_GX;
    font->spacing            = gxFONT_SPACING_FIXED;
    font->inter_char_spacing = 0;
    font->cotptr             = NULL;
    font->cwtptr             = NULL;
    font->bufptr             = NULL;
  }

/*____________________________________________________________________
|
| Load Character Offset Table.
|___________________________________________________________________*/

  if (ok) {
    size = font->header.maxch - font->header.minch + 2;
    /* Allocate memory for the table */
    font->cotptr = (word *) malloc (size * sizeof(word));
    /* Put values in the table */
    if (font->cotptr)
      FONT_FREAD ((void *)(font->cotptr), sizeof(word), size, fp)
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Load Character Width Table.
|___________________________________________________________________*/

  if (ok) {
    size--;
    /* Allocate memory for the table */
    font->cwtptr = (byte *) malloc (size * sizeof(byte));
    /* Put values in the table */
    if (font->cwtptr)
      FONT_FREAD ((void *)(font->cwtptr), sizeof(byte), size, fp)
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Load Character Buffer.
|___________________________________________________________________*/

  if (ok) {
    size = font->header.fwidth * font->header.fheight;
    font->bufptr = (byte *) malloc (size);
    if (font->bufptr)
      FONT_FREAD ((void *)(font->bufptr), sizeof(byte), size, fp)
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Compute width in pixels of space character.
|___________________________________________________________________*/

  if (ok) {
    if ((' ' > font->header.minch) AND (' ' < font->header.maxch))
      font->space_char_width = font->cwtptr[' '-font->header.minch];
    else if (('w' > font->header.minch) AND ('w' < font->header.maxch))
      font->space_char_width = font->cwtptr['w'-font->header.minch];
    else if (('W' > font->header.minch) AND ('W' < font->header.maxch))
      font->space_char_width = font->cwtptr['W'-font->header.minch];
    else
      font->space_char_width = font->header.maxwidth;
  }

/*____________________________________________________________________
|
| On any error, free all memory and exit.
|___________________________________________________________________*/

  if (NOT ok) {
    gxFreeFont (font);
    font = NULL;
  }
  if (fp)
    fclose (fp);

  return (font);
}

/*____________________________________________________________________
|
| Function: Load_GEM_Font
|
| Outputs: Loads a GEM font from a file and sets up a font data structrue.
|       Returns a ptr to the font struct or NULL on any error.
|
| Description: Currently, this function is not supported!
|___________________________________________________________________*/

static gxFont *Load_GEM_Font (
  byte     *origin,             // filename or buffer
  unsigned  origin_size,        // # bytes in buffer (if buffer)
  int       data_type )         // file or buffer
{
  int i, size;
//  unsigned size;
  byte *origin_ptr = 0;
  GEMHeader *gem = NULL;
  gxFont *font = NULL;
  FILE *fp = NULL;
  int ok = TRUE;

/*____________________________________________________________________
|
| Open file.
|___________________________________________________________________*/

  if (data_type == DATA_TYPE_FILE) {
    fp = fopen ((char *)origin, "rb");
    if (fp == NULL)
      ok = FALSE;
  }
  else
    origin_ptr = origin;

/*____________________________________________________________________
|
| Allocate memory for data structures.
|___________________________________________________________________*/

  if (ok) {
    font = (gxFont *) calloc (1, sizeof(gxFont));
    if (font == NULL)
      ok = FALSE;
  }

  if (ok) {
    gem = (GEMHeader *) malloc (sizeof(GEMHeader));
    if (gem == NULL)
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Read in GEM font header and verify this is a font that can be used
| by GX.
|___________________________________________________________________*/

  if (ok)
    FONT_FREAD ((void *)gem, sizeof(GEMHeader), 1, fp)

/*____________________________________________________________________
|
| Initialize font header with data.
|___________________________________________________________________*/

  if (ok) {
    font->header.id          = VERSION_1;
    font->header.minch       = gem->minch;
    font->header.maxch       = gem->maxch;
    font->header.ascent      = gem->ascent;
    font->header.descent     = gem->ascent;
    font->header.fwidth      = gem->fwidth;
    font->header.fheight     = gem->fheight;
    font->type               = gxFONT_TYPE_GEM;
    font->spacing            = gxFONT_SPACING_FIXED;
    font->inter_char_spacing = 0;
    font->cotptr             = NULL;
    font->cwtptr             = NULL;
    font->bufptr             = NULL;
  }

/*____________________________________________________________________
|
| Build Character Offset Table.
|___________________________________________________________________*/

  if (ok) {
    size = font->header.maxch - font->header.minch + 2;
    /* Allocate memory for the table */
    font->cotptr = (word *) malloc (size * sizeof(word));
    /* Put values in the table */
    if (font->cotptr) {
      FONT_FREAD ((void *)(font->cotptr), sizeof(word), size, fp)
    }
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Build Character Width Table.
|___________________________________________________________________*/

  if (ok) {
    size--;
    /* Allocate memory for the table */
    font->cwtptr = (byte *) malloc (size * sizeof(byte));
    /* Put values in the table */
    if (font->cwtptr)
      for (i=0; i<size; i++)
        font->cwtptr[i] = font->cotptr[i+1] - font->cotptr[i];
    else
      ok = FALSE;
  }
  /* Find max width of a character in this font */
  if (ok) {
    font->header.maxwidth = 0;
    for (i=0; i<size; i++)
      if (font->cwtptr[i] > font->header.maxwidth)
        font->header.maxwidth = font->cwtptr[i];
  }

/*____________________________________________________________________
|
| Load Character Buffer.
|___________________________________________________________________*/

  if (ok) {
    size = font->header.fwidth * font->header.fheight;
    font->bufptr = (byte *) malloc (size);
    if (font->bufptr) {
      FONT_FSEEK (fp, -size, SEEK_END)
      FONT_FREAD ((void *)(font->bufptr), sizeof(byte), size, fp)
    }
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Compute width in pixels of space character.
|___________________________________________________________________*/

  if (ok) {
    if ((' ' > font->header.minch) AND (' ' < font->header.maxch))
      font->space_char_width = font->cwtptr[' '-font->header.minch];
    else if (('w' > font->header.minch) AND ('w' < font->header.maxch))
      font->space_char_width = font->cwtptr['w'-font->header.minch];
    else if (('W' > font->header.minch) AND ('W' < font->header.maxch))
      font->space_char_width = font->cwtptr['W'-font->header.minch];
    else
      font->space_char_width = font->header.maxwidth;
  }

/*____________________________________________________________________
|
| On any error, free all memory and exit.
|___________________________________________________________________*/

  if (NOT ok) {
    gxFreeFont (font);
    font = NULL;
  }
  if (gem)
    free (gem);
  if (fp)
    fclose (fp);

  return (font);
}

/*____________________________________________________________________
|
| Function: Load_Metawindow_Font
|
| Outputs: Loads a Metawindow font from a file and sets up a font data
|       structrue.  Returns a ptr to the font struct or NULL on any
|       error.
|___________________________________________________________________*/

static gxFont *Load_Metawindow_Font (
  byte     *origin,             // filename or buffer
  unsigned  origin_size,        // # bytes in buffer (if buffer)
  int       data_type )         // file or buffer
{
  int i, size;
//  unsigned size;
  byte *origin_ptr = 0;
  fontRcd *mf = NULL;
  grafMap *gmap = NULL;
  gxFont *font = NULL;
  FILE *fp = NULL;
  int ok = TRUE;

/*____________________________________________________________________
|
| Open file.
|___________________________________________________________________*/

  if (data_type == DATA_TYPE_FILE) {
    fp = fopen ((char *)origin, "rb");
    if (fp == NULL)
      ok = FALSE;
  }
  else
    origin_ptr = origin;

/*____________________________________________________________________
|
| Allocate memory for data structures.
|___________________________________________________________________*/

  if (ok) {
    font = (gxFont *) calloc (1, sizeof(gxFont));
    if (font == NULL)
      ok = FALSE;
  }

  if (ok) {
    mf = (fontRcd *) malloc (sizeof(fontRcd));
    if (mf == NULL)
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Read in fontRcd struct from METAFONT file and verify this is a file
| that can be used by GX.
|___________________________________________________________________*/

  if (ok) {
    FONT_FREAD ((void *)mf, sizeof(fontRcd), 1, fp)
    /* Is this a non-ASCII coded format? */
    if (mf->fontCoding != 0)
      ok = FALSE;
    /* Is this a bitmapped font? */
    else if ((mf->fontFlags & 0x7) != 0)
      ok = FALSE;
    /* Is this a compressed font? */
    else if ((mf->fontFlags & 0x30) != 0)
      ok = FALSE;
    /* Is this font non-integer precise? */
/*
    else if ((mf->fontFlags & 0x300) != 0)
      ok = FALSE;
*/
  }

/*____________________________________________________________________
|
| Initialize font header with data.
|___________________________________________________________________*/

  if (ok) {
    font->header.id          = VERSION_1;
    font->header.minch       = mf->minChar;
    font->header.maxch       = mf->maxChar;
    font->header.ascent      = mf->ascent;
    font->header.descent     = mf->ascent;
    font->type               = gxFONT_TYPE_METAWINDOW;
    font->spacing            = gxFONT_SPACING_FIXED;
    font->inter_char_spacing = 0;
    font->cotptr             = NULL;
    font->cwtptr             = NULL;
    font->bufptr             = NULL;
  }

/*____________________________________________________________________
|
| Build Character Offset Table.
|___________________________________________________________________*/

  if (ok) {
    size = font->header.maxch - font->header.minch + 2;
    /* Allocate memory for the table */
    font->cotptr = (word *) malloc (size * sizeof(word));
    /* Put values in the table */
    if (font->cotptr) {
      FONT_FSEEK (fp, mf->locTbl, SEEK_SET)
      FONT_FREAD ((void *)(font->cotptr), sizeof(word), size, fp)
    }
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Build Character Width Table.
|___________________________________________________________________*/

  if (ok) {
    size--;
    /* Allocate memory for the table */
    font->cwtptr = (byte *) malloc (size * sizeof(byte));
    /* Put values in the table */
    if (font->cwtptr)
      for (i=0; i<size; i++)
        font->cwtptr[i] = font->cotptr[i+1] - font->cotptr[i];
    else
      ok = FALSE;
  }

  /* Find max width of a character in this font */
  if (ok) {
    font->header.maxwidth = 0;
    for (i=0; i<size; i++)
      if (font->cwtptr[i] > font->header.maxwidth)
        font->header.maxwidth = font->cwtptr[i];
  }

/*____________________________________________________________________
|
| Get font width, height in bytes.
|___________________________________________________________________*/

  if (ok) {
    /* Allocate memory for a Metawindow struct */
    gmap = (grafMap *) malloc (sizeof(grafMap));
    if (gmap) {
      /* Read in data from file */
      FONT_FSEEK (fp, mf->grafMapTbl, SEEK_SET)
      FONT_FREAD ((void *)(gmap), 1, sizeof(grafMap), fp)
      /* Get font size info */
      font->header.fwidth  = gmap->pixbytes;
      font->header.fheight = gmap->pixHeight;
      /* Free memory */
      free (gmap);
    }
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Load Character Buffer.
|___________________________________________________________________*/

  if (ok) {
    size = font->header.fwidth * font->header.fheight;
    font->bufptr = (byte *) malloc (size);
    if (font->bufptr) {
      FONT_FSEEK (fp, mf->fontTbl, SEEK_SET)
      FONT_FREAD ((void *)(font->bufptr), sizeof(byte), size, fp)
/*
      // Change an image font into the proper image format?
      if (font->header.id == 2) {

/////////// CHANGE HERE        
    
      }
*/    
    }
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Compute width in pixels of space character.
|___________________________________________________________________*/

  if (ok) {
    if ((' ' > font->header.minch) AND (' ' < font->header.maxch))
      font->space_char_width = font->cwtptr[' '-font->header.minch];
    else if (('w' > font->header.minch) AND ('w' < font->header.maxch))
      font->space_char_width = font->cwtptr['w'-font->header.minch];
    else if (('W' > font->header.minch) AND ('W' < font->header.maxch))
      font->space_char_width = font->cwtptr['W'-font->header.minch];
    else
      font->space_char_width = font->header.maxwidth;
  }

/*____________________________________________________________________
|
| On any error, free all memory and exit.
|___________________________________________________________________*/

  if (NOT ok) {
    gxFreeFont (font);
    font = NULL;
  }
  if (mf)
    free (mf);
  if (fp)
    fclose (fp);

  return (font);
}

/*____________________________________________________________________
|
| Function: gxSaveFont
|
| Outputs: Saves current font to a file.  Returns true if success,
|       else false on any error.  Fonts should be saved with a 'GXFONT'
|       extension indicating the file is a GX font file.
|___________________________________________________________________*/

int gxSaveFont (char *filename)
{
  unsigned size;
  byte type;
  char fontname[32];
  FILE *fp;
  int saved = FALSE;

  if (gx_Current_font) {
    fp = fopen (filename, "wb");
    if (fp) {
      type = gx_Current_font->type;
      memcpy (fontname, gx_Current_font->header.fontname, 32);
      gx_Current_font->type = gxFONT_TYPE_GX;
      strcpy (gx_Current_font->header.fontname, filename);
      fwrite ((void *)&(gx_Current_font->header), sizeof(gxFontHeader), 1, fp);
      gx_Current_font->type = type;
      memcpy (gx_Current_font->header.fontname, fontname, 32);

      size = gx_Current_font->header.maxch - gx_Current_font->header.minch + 2;
      fwrite ((void *)(gx_Current_font->cotptr), sizeof(word), size, fp);
      size--;
      fwrite ((void *)(gx_Current_font->cwtptr), sizeof(byte), size, fp);
      size = gx_Current_font->header.fwidth * gx_Current_font->header.fheight;
      fwrite ((void *)(gx_Current_font->bufptr), sizeof(byte), size, fp);
      saved = TRUE;
    }
  }

  return (saved);
}

/*____________________________________________________________________
|
| Function: gxFreeFont
|
| Outputs: Frees all dynamic memory associated with a font.
|___________________________________________________________________*/

void gxFreeFont (gxFont *font)
{
  if (font) {
    /* Free all memory for the font */
    if (font->cotptr)
      free (font->cotptr);
    if (font->cwtptr)
      free (font->cwtptr);
    if (font->bufptr)
      free (font->bufptr);
    free (font);
    /* If this is the current font, set the current font to none */
    if (font == gx_Current_font)
      gx_Current_font = NULL;
  }
}

/*____________________________________________________________________
|
| Function: gxSetFont
|
| Outputs: Sets the current font for drawing operations.  The font must
|       have been previously loaded by gxLoadFont().
|___________________________________________________________________*/

void gxSetFont (gxFont *font)
{
  gx_Current_font = font;
}

/*____________________________________________________________________
|
| Function: gxGetFont
|
| Outputs: Returns the currently active font or NULL if none.
|___________________________________________________________________*/

gxFont *gxGetFont (void)
{
  return (gx_Current_font);
}

/*____________________________________________________________________
|
| Function: gxScaleFont
|
| Outputs: Creates a new scaled version of font.  Returns a handle to
|       the new font.  The font can only be scaled larger and scaling
|       factors must be in integer amounts (2x larger, etc.).
|___________________________________________________________________*/

gxFont *gxScaleFont (gxFont *font, int sx, int sy)
{
  int i, j, x, y, dst, dx, byte_width, size;
  byte dstmask;
//  unsigned size;
  gxFont *newfont = NULL;
  int ok = TRUE;
  byte srcmask [8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

/*____________________________________________________________________
|
| Error checking on input parameters
|___________________________________________________________________*/

  if ((font == NULL) OR (sx < 1) OR (sy < 1))
    ok = FALSE;

/*____________________________________________________________________
|
| Allocate memory for gxFont struct
|___________________________________________________________________*/

  if (ok) {
    newfont = (gxFont *) malloc (sizeof(gxFont));
    if (newfont == NULL)
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Initialize font header with data
|___________________________________________________________________*/

  if (ok) {
    memcpy ((void *)newfont, (void *)font, sizeof(gxFont));
    newfont->header.id = VERSION_1;
    strcpy (newfont->header.fontname, "NONAME.GXFONT");
    newfont->header.ascent   *= sy;
    newfont->header.descent  *= sy;
    newfont->header.maxwidth *= sx;
    newfont->header.fwidth   *= sx;
    newfont->header.fheight  *= sy;
    newfont->type             = gxFONT_TYPE_GX;
    newfont->inter_char_spacing *= sx;
    newfont->space_char_width   *= sx;
    newfont->cotptr           = NULL;
    newfont->cwtptr           = NULL;
    newfont->bufptr           = NULL;
  }

/*____________________________________________________________________
|
| Build Character Offset Table
|___________________________________________________________________*/

  if (ok) {
    size = newfont->header.maxch - newfont->header.minch + 2;
    /* Allocate memory for the table */
    newfont->cotptr = (word *) malloc (size * sizeof(word));
    /* Put values in the table */
    if (newfont->cotptr)
      for (i=0; i<size; i++)
        newfont->cotptr[i] = font->cotptr[i] * sx;
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Build Character Width Table
|___________________________________________________________________*/

  if (ok) {
    size--;
    /* Allocate memory for the table */
    newfont->cwtptr = (byte *) malloc (size * sizeof(byte));
    /* Put values in the table */
    if (newfont->cwtptr)
      for (i=0; i<size; i++)
        newfont->cwtptr[i] = font->cwtptr[i] * sx;
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| Build Character Buffer
|___________________________________________________________________*/

  if (ok) {
    size = newfont->header.fwidth *  newfont->header.fheight;
    /* Allocate memory for a buffer and clear it */
    newfont->bufptr = (byte *) calloc (size, sizeof(byte));
    if (newfont->bufptr) {
      byte_width = font->header.fwidth;
      dx = byte_width * 8;
      dst = 0;
      for (y=0; y<font->header.fheight; y++) {
        for (i=0; i<sy; i++) {
          dstmask = 0x80;
          for (x=0; x<dx; x++) {
            for (j=0; j<sx; j++) {
              if (font->bufptr[(y*byte_width)+(x/8)] & srcmask[x%8])
                newfont->bufptr[dst] |= dstmask;
              dstmask >>= 1;
              if (dstmask == 0) {
                dstmask = 0x80;
                dst++;
              }
            }
          }
        }
      }
    }
    else
      ok = FALSE;
  }

/*____________________________________________________________________
|
| On any error, free all memory and exit
|___________________________________________________________________*/

  if (NOT ok) {
    gxFreeFont (newfont);
    newfont = NULL;
  }

  return (newfont);
}

/*____________________________________________________________________
|
| Function: gxSetFontAttributes
|
| Outputs: Sets various attributes for the currently active font, if any.
|___________________________________________________________________*/

void gxSetFontAttributes (int spacing, int inter_char_spacing)
{
  if (gx_Current_font) {
    gx_Current_font->spacing = spacing;
    /* If inter_char_spacing is negative, must not wipe out maxwidth! */
    if (gx_Current_font->header.maxwidth + inter_char_spacing)
      gx_Current_font->inter_char_spacing = inter_char_spacing;
  }
}

/*____________________________________________________________________
|
| Function: gxGetFontAttributes
|
| Outputs: Returns various attributes for the currently active font,
|       if any.  If no active font, return values are undefined.
|___________________________________________________________________*/

void gxGetFontAttributes (int *spacing, int *inter_char_spacing)
{
  if (gx_Current_font) {
    *spacing            = gx_Current_font->spacing;
    *inter_char_spacing = gx_Current_font->inter_char_spacing;
  }
}

/*____________________________________________________________________
|
| Function: gxSetSpaceCharWidth
|
| Outputs: Allows caller to set a new pixel width for the SPACE character
|       in the current font.  Cannot be set to a negative value.
|___________________________________________________________________*/

void gxSetSpaceCharWidth (int width)
{
  if (gx_Current_font)
    if (width >= 0)
      gx_Current_font->space_char_width = width;
}

/*____________________________________________________________________
|
| Function: gxGetSpaceCharWidth
|
| Outputs: Returns pixel width of SPACE character in the current font,
|       if any.
|___________________________________________________________________*/

int gxGetSpaceCharWidth (void)
{
  int width = 0;

  if (gx_Current_font)
    width = gx_Current_font->space_char_width;

  return (width);
}

/*____________________________________________________________________
|
| Function: gxGetFontWidth
|
| Outputs: Returns the max width of a character in the current font.
|___________________________________________________________________*/

int gxGetFontWidth (void)
{
  int width = 0;

  if (gx_Current_font)
    width = gx_Current_font->header.maxwidth;

  return (width);
}

/*____________________________________________________________________
|
| Function: gxGetFontHeight
|
| Outputs: Returns the max height of a character in the current font.
|___________________________________________________________________*/

int gxGetFontHeight (void)
{
  int height = 0;

  if (gx_Current_font)
    height = gx_Current_font->header.fheight;

  return (height);
}

/*____________________________________________________________________
|
| Function: gxGetStringWidth
|
| Outputs: Returns the width of a string in the current font.  Width
|       returned will differ depending on whether fixed or proportional
|       spacing is active.
|___________________________________________________________________*/

int gxGetStringWidth (char *str)
{
  int i, xinc, char_dx, spacing, minch, maxch, spacewidth, drawable;
  char ch;
  int width = 0;

/*____________________________________________________________________
|
| Init variables.
|___________________________________________________________________*/

  if (gx_Current_font) {

    spacing    = gx_Current_font->inter_char_spacing;
    xinc       = gx_Current_font->header.maxwidth + spacing;
    minch      = gx_Current_font->header.minch;
    maxch      = gx_Current_font->header.maxch;
    spacewidth = gx_Current_font->space_char_width;

/*____________________________________________________________________
|
| Count string width, one character at a time.
|___________________________________________________________________*/

    for (i=0; str[i]; i++) {
      ch = str[i];
      drawable = FALSE;
      /* Is this character within range of font? */
      if ((ch >= minch) AND (ch <= maxch)) {
        char_dx = gx_Current_font->cwtptr[ch-minch];
        /* Is this character present in font? */
        if (char_dx AND (ch != ' '))
          drawable = TRUE;
      }

      if (gx_Current_font->spacing == gxFONT_SPACING_FIXED)
        width += xinc;
      else { /* proportional */
        if (NOT drawable)
          width += spacewidth;
        else
          width += (char_dx + spacing);
      }
    }
  }

  return (width);
}

/*____________________________________________________________________
|
| Function: gxDrawText
|
| Outputs: Draws a string in the current font in the current window.
|       x,y are window relative coordinates.  Text is drawn transparently
|       with underlying graphics not disturbed.
|___________________________________________________________________*/

void gxDrawText (char *str, int x, int y)
{
  int i, xinc, char_dx, spacing, drawn;
  int minch, maxch, spacewidth;
  int xadd;                     /* used to center character w/ fixed spacing */
  unsigned buf_dx, buf_dy;      /* in pixels */
  char ch;
  word offset;

/*____________________________________________________________________
|
| Init variables.
|___________________________________________________________________*/

  if (gx_Current_font) {

    spacing    = gx_Current_font->inter_char_spacing;
    xinc       = gx_Current_font->header.maxwidth + spacing;
    buf_dx     = gx_Current_font->header.fwidth;
    buf_dy     = gx_Current_font->header.fheight;
    minch      = gx_Current_font->header.minch;
    maxch      = gx_Current_font->header.maxch;
    spacewidth = gx_Current_font->space_char_width;
    xadd       = 0;

/*____________________________________________________________________
|
| Draw string, one character at a time.
|___________________________________________________________________*/

    for (i=0; str[i]; i++) {
      ch = str[i];
      drawn = FALSE;
      /* Is this character within range of font? */
      if ((ch >= minch) AND (ch <= maxch)) {
        char_dx = gx_Current_font->cwtptr[ch-minch];
        /* Is this character present in font? */
        if (char_dx AND (ch != ' ')) {
          /* Compute a centering factor */
          if (gx_Current_font->spacing == gxFONT_SPACING_FIXED) {
            xadd = (xinc - char_dx) / 2;
            if (xadd < 0)
              xadd = 0;
          }
          offset = gx_Current_font->cotptr[ch-minch];
          if (gx_Current_font->header.id == 1)
            Draw_Bitmap (gx_Current_font->bufptr,
                         buf_dx * 8,
                         buf_dy,
                         offset,
                         0,
                         x+xadd,
                         y,
                         char_dx,
                         buf_dy,
                         gx_Fore_color);
          else
            Draw_Image (gx_Current_font->bufptr,
                        buf_dx,
                        buf_dy,
                        offset * 3,
                        0,
                        x+xadd,
                        y,
                        char_dx,
                        buf_dy );
          drawn = TRUE;
        }
      }

      if (gx_Current_font->spacing == gxFONT_SPACING_FIXED)
        x += xinc;
      else { /* proportional */
        if (NOT drawn)
          x += spacewidth;
        else
          x += (char_dx + spacing);
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: gxDrawTextOverwrite
|
| Outputs: Draws a string in the current font in the current window.
|       x,y are window relative coordinates.  Text is drawn in current
|       color and back_color for background.
|___________________________________________________________________*/

void gxDrawTextOverwrite (char *str, int x, int y, gxColor back_color)
{
  int i, xinc, char_dx, spacing, drawn, x2;
  int minch, maxch, spacewidth;
  int xadd;                     /* used to center character w/ fixed spacing */
  unsigned buf_dx, buf_dy;      /* in pixels */
  char ch;
  word offset;
  gxColor color;

/*____________________________________________________________________
|
| Init variables.
|___________________________________________________________________*/

  if (gx_Current_font) {

    spacing    = gx_Current_font->inter_char_spacing;
    xinc       = gx_Current_font->header.maxwidth + spacing;
    buf_dx     = gx_Current_font->header.fwidth * 8;
    buf_dy     = gx_Current_font->header.fheight;
    minch      = gx_Current_font->header.minch;
    maxch      = gx_Current_font->header.maxch;
    spacewidth = gx_Current_font->space_char_width;
    xadd       = 0;

/*____________________________________________________________________
|
| Draw string, one character at a time.
|___________________________________________________________________*/

    for (i=0; str[i]; i++) {
      ch = str[i];
      drawn = FALSE;
      /* Is this character within range of font? */
      if (((ch >= minch) AND (ch <= maxch)) OR (ch == ' ')) {
        if (ch == ' ')
          char_dx = spacewidth;
        else
          char_dx = gx_Current_font->cwtptr[ch-minch];
        /* Is this character present in font? */
        if (char_dx) {
          /* Draw background */
          color = gxGetColor ();
          gxSetColor (back_color);
          if (gx_Current_font->spacing == gxFONT_SPACING_FIXED)
            x2 = x + xinc - 1;
          else
            x2 = x + (char_dx + spacing) - 1;
          gxDrawFillRectangle (x, y, x2, y+buf_dy-1);
          gxSetColor (color);
          /* Draw character */
          if (ch != ' ') {
            /* Compute a centering factor */
            if (gx_Current_font->spacing == gxFONT_SPACING_FIXED) {
              xadd = (xinc - char_dx) / 2;
              if (xadd < 0)
                xadd = 0;
            }
            offset = gx_Current_font->cotptr[ch-minch];
            if (gx_Current_font->header.id == 1)
              Draw_Bitmap (gx_Current_font->bufptr,
                           buf_dx,
                           buf_dy,
                           offset,
                           0,
                           x+xadd,
                           y,
                           char_dx,
                           buf_dy,
                           gx_Fore_color);
            else
              Draw_Image (gx_Current_font->bufptr,
                          buf_dx,
                          buf_dy,
                          offset,
                          0,
                          x+xadd,
                          y,
                          char_dx,
                          buf_dy );
            drawn = TRUE;
          }
        }
      }

      if (gx_Current_font->spacing == gxFONT_SPACING_FIXED)
        x += xinc;
      else { /* proportional */
        if (NOT drawn)
          x += spacewidth;
        else
          x += (char_dx + spacing);
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: gxGetString
|
| Outputs: Fills input string with a string typed in by user from
|       keyboard. Echoes input to screen.  Returns address of string.
|
| Description: Single line input only.  Allows backspacing.  Max string
|       to get is 80 characters.  Coordinates are window relative.
|___________________________________________________________________*/

#define BACKSPACE 8
#define ENTER     13
#define ESC       27

char *gxGetString (
  char    *str,                   // string to fill                       
  int      max_str,               // max characters to put into str, not counting NULL 
  int      x,                     // position in window to echo input     
  int      y,
  gxColor  text_color,            // color to echo input in               
  gxColor *back_color,            // background color or NULL for no background color                     
  void   (*animate_func)(void) )  // ptr to callers animate function 
{
  char ch;
  int valid_key, done = FALSE;
  int maxwidth, str_index;
  int x1, y1, x2, y2, dx, dy;
  gxColor save_color;
  char outstr [82];

/*____________________________________________________________________
|
| If there is no currently active font, exit.
|___________________________________________________________________*/

  if (gx_Current_font == NULL)
    return (str);

/*____________________________________________________________________
|
| Init variables.
|___________________________________________________________________*/

  dx = gx_Current_font->header.maxwidth;
  dy = gx_Current_font->header.fheight;

  save_color = gxGetColor ();

/*____________________________________________________________________
|
| Error checking on max_str.  Then clear area to echo input.
|___________________________________________________________________*/

  maxwidth = (gxGetMaxX()-x)/dx;
  if (max_str+1 > maxwidth)
    max_str = maxwidth-1;

  if (back_color) { 
    gxSetColor (*back_color);
    gxDrawFillRectangle (x, y, x+(max_str+1)*dx-1, y+dy-1);
  }

/*____________________________________________________________________
|
| main input loop
|___________________________________________________________________*/

  /* Preserve callers str as much as possible */
  str[max_str] = '\0';
  for (str_index=0; str[str_index]; str_index++);

  while (NOT done) {
    str[str_index] = '\0';
    strcpy (outstr, str);
    strcat (outstr, "<");
    if (back_color) {
      gxSetColor (*back_color);
      x1 = x;
      y1 = y;
      x2 = x1 + (str_index*dx) - 1;
      y2 = y1 + dy - 1;
      gxDrawFillRectangle (x1, y1, x2, y2);
    }
    gxSetColor (text_color);
    gxDrawText (outstr, x, y);

    /* Flush keybuffer */
    while (kbhit()) getch ();

    valid_key = FALSE;
    while (NOT valid_key) {

      /* If no keypress ready, call animate function (if any) */
      while (NOT kbhit()) {
        if (animate_func)
          (*animate_func)();
      }

      /* Get keypress */
      ch = getch ();
      if (NOT ch) {     /* extended keycode? */
        ch = getch ();
        BEEP
      }
      else if (isprint(ch)) {
        if (str_index < max_str)
          str[str_index++] = ch;
        else
          BEEP
        valid_key = TRUE;
      }
      else if (ch == BACKSPACE) {
        if (str_index) {
          str_index--;
          x1 = x + (str_index*dx);
          y1 = y;
          x2 = x1 + (2*dx) - 1;
          y2 = y1 + dy - 1;
          if (back_color) {
            gxSetColor (*back_color);
            gxDrawFillRectangle (x1, y1, x2, y2);
          }
        }
        else
          BEEP
        valid_key = TRUE;
      }
      else if (ch == ENTER) {
        valid_key = TRUE;
        done = TRUE;
      }
      else if (ch == ESC) {
        str_index = 0;
        valid_key = TRUE;
        done = TRUE;
      }
      else {
        BEEP
      }
    }
  }
  str[str_index] = '\0';

  gxSetColor (save_color);

  return (str);
}

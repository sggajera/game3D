/*____________________________________________________________________
|
| File: lwo2_print.cpp
|
| Description: A function to print out in ASCII form the contents of an
|   LWO2 file.
|
| Functions: gx3d_PrintLWO2File
|             read_tags
|             read_clip
|             read_envl
|             read_layr
|             read_pnts
|             read_bbox
|             read_pols
|             read_ptag
|             read_vmap
|             read_surf
|              read_blok
|              read_tmap
|             seek_pad
|
|              read_u1
|              read_u2
|              read_u4
|              read_i1
|              read_i2
|              read_f4
|              read_col12
|              read_vec12
|              read_vx
|              read_name
|              read_id4
|              read_head
|              
|               _SwapTwoBytes
|               _SwapFourBytes
|
| Notes: All distances are saved in LWO2 files in meters by convention.
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

/*___________________
|
| Constants
|__________________*/

#define MAKE_ID(a,b,c,d)	\
	((unsigned long) (a)<<24 | (unsigned long) (b)<<16 | \
	 (unsigned long) (c)<<8 | (unsigned long) (d))

// Universal IFF identifiers
#define ID_FORM		MAKE_ID('F','O','R','M')
#define ID_LWO2		MAKE_ID('L','W','O','2')

// PRIMARY CHUNK ID
#define ID_LAYR		MAKE_ID('L','A','Y','R')
#define ID_PNTS		MAKE_ID('P','N','T','S')
#define ID_VMAP		MAKE_ID('V','M','A','P')
#define ID_POLS		MAKE_ID('P','O','L','S')
#define ID_TAGS		MAKE_ID('T','A','G','S')
#define ID_PTAG		MAKE_ID('P','T','A','G')
#define ID_ENVL		MAKE_ID('E','N','V','L')
#define ID_CLIP		MAKE_ID('C','L','I','P')
#define ID_SURF		MAKE_ID('S','U','R','F')
#define ID_BBOX		MAKE_ID('B','B','O','X')
#define ID_DESC		MAKE_ID('D','E','S','C')
#define ID_TEXT		MAKE_ID('T','E','X','T')
#define ID_ICON		MAKE_ID('I','C','O','N')

// POLS TYPE
#define ID_FACE		MAKE_ID('F','A','C','E')
#define ID_CRVS		MAKE_ID('C','U','R','V')
#define ID_PCHS		MAKE_ID('P','T','C','H')
#define ID_MBAL		MAKE_ID('M','B','A','L')
#define ID_BONE		MAKE_ID('B','O','N','E')

// PTAG TYPE
#define ID_SURF		MAKE_ID('S','U','R','F')
#define ID_BNID		MAKE_ID('B','N','I','D')
#define ID_SGMP		MAKE_ID('S','G','M','P')
#define ID_PART		MAKE_ID('P','A','R','T')

// IMAGE SUB-CHUNK ID
#define ID_STIL		MAKE_ID('S','T','I','L')
#define ID_ISEQ		MAKE_ID('I','S','E','Q')
#define ID_ANIM		MAKE_ID('A','N','I','M')
#define ID_XREF		MAKE_ID('X','R','E','F')
#define ID_STCC		MAKE_ID('S','T','C','C')
#define ID_CONT		MAKE_ID('C','O','N','T')
#define ID_BRIT		MAKE_ID('B','R','I','T')
#define ID_SATR		MAKE_ID('S','A','T','R')
#define ID_HUE		MAKE_ID('H','U','E',' ')
#define ID_GAMM		MAKE_ID('G','A','M','M')
#define ID_NEGA		MAKE_ID('N','E','G','A')
#define ID_CROP		MAKE_ID('C','R','O','P')
#define ID_ALPH		MAKE_ID('A','L','P','H')
#define ID_COMP		MAKE_ID('C','O','M','P')
#define ID_IFLT		MAKE_ID('I','F','L','T')
#define ID_PFLT		MAKE_ID('P','F','L','T')

// ENVELOPE SUB-CHUNK
#define ID_PRE		MAKE_ID('P','R','E',' ')
#define ID_POST		MAKE_ID('P','O','S','T')
#define ID_KEY		MAKE_ID('K','E','Y',' ')
#define ID_SPAN		MAKE_ID('S','P','A','N')
#define ID_CHAN		MAKE_ID('C','H','A','N')

// SURFACE SUB-CHUNK ID
#define ID_COLR		MAKE_ID('C','O','L','R')
#define ID_DIFF		MAKE_ID('D','I','F','F')
#define ID_LUMI		MAKE_ID('L','U','M','I')
#define ID_SPEC		MAKE_ID('S','P','E','C')
#define ID_REFL		MAKE_ID('R','E','F','L')
#define ID_TRAN		MAKE_ID('T','R','A','N')
#define ID_TRNL		MAKE_ID('T','R','N','L')
#define ID_GLOS		MAKE_ID('G','L','O','S')
#define ID_SHRP		MAKE_ID('S','H','R','P')
#define ID_BUMP		MAKE_ID('B','U','M','P')
#define ID_SIDE		MAKE_ID('S','I','D','E')
#define ID_SMAN		MAKE_ID('S','M','A','N')
#define ID_RFOP		MAKE_ID('R','F','O','P')
#define ID_RIMG		MAKE_ID('R','I','M','G')
#define ID_RSAN		MAKE_ID('R','S','A','N')
#define ID_RIND		MAKE_ID('R','I','N','D')
#define ID_CLRH		MAKE_ID('C','L','R','H')
#define ID_TROP		MAKE_ID('T','R','O','P')
#define ID_TIMG		MAKE_ID('T','I','M','G')
#define ID_CLRF		MAKE_ID('C','L','R','F')
#define ID_ADTR		MAKE_ID('A','D','T','R')
#define ID_GLOW		MAKE_ID('G','L','O','W')
#define ID_LINE		MAKE_ID('L','I','N','E')
#define ID_ALPH		MAKE_ID('A','L','P','H')
#define ID_AVAL		MAKE_ID('A','V','A','L')
#define ID_GVAL		MAKE_ID('G','V','A','L')
#define ID_BLOK		MAKE_ID('B','L','O','K')
#define ID_LCOL		MAKE_ID('L','C','O','L')
#define ID_LSIZ		MAKE_ID('L','S','I','Z')
#define ID_CMNT		MAKE_ID('C','M','N','T')

// TEXTURE LAYER
#define ID_CHAN		MAKE_ID('C','H','A','N')
#define ID_TYPE		MAKE_ID('T','Y','P','E')
#define ID_NAME		MAKE_ID('N','A','M','E')
#define ID_ENAB		MAKE_ID('E','N','A','B')
#define ID_OPAC		MAKE_ID('O','P','A','C')
#define ID_FLAG		MAKE_ID('F','L','A','G')
#define ID_PROJ		MAKE_ID('P','R','O','J')
#define ID_STCK		MAKE_ID('S','T','C','K')
#define ID_TAMP		MAKE_ID('T','A','M','P')

// TEXTURE MAPPING
#define ID_TMAP		MAKE_ID('T','M','A','P')
#define ID_AXIS		MAKE_ID('A','X','I','S')
#define ID_CNTR		MAKE_ID('C','N','T','R')
#define ID_SIZE		MAKE_ID('S','I','Z','E')
#define ID_ROTA		MAKE_ID('R','O','T','A')
#define ID_OREF		MAKE_ID('O','R','E','F')
#define ID_FALL		MAKE_ID('F','A','L','L')
#define ID_CSYS		MAKE_ID('C','S','Y','S')

// IMAGE MAP
#define ID_IMAP		MAKE_ID('I','M','A','P')
#define ID_IMAG		MAKE_ID('I','M','A','G')
#define ID_WRAP		MAKE_ID('W','R','A','P')
#define ID_WRPW		MAKE_ID('W','R','P','W')
#define ID_WRPH		MAKE_ID('W','R','P','H')
#define ID_VMAP		MAKE_ID('V','M','A','P')
#define ID_AAST		MAKE_ID('A','A','S','T')
#define ID_PIXB		MAKE_ID('P','I','X','B')

// PROCUDUAL TEXTURE
#define ID_PROC		MAKE_ID('P','R','O','C')
#define ID_COLR		MAKE_ID('C','O','L','R')
#define ID_VALU		MAKE_ID('V','A','L','U')
#define ID_FUNC		MAKE_ID('F','U','N','C')
#define ID_FTPS		MAKE_ID('F','T','P','S')
#define ID_ITPS		MAKE_ID('I','T','P','S')
#define ID_ETPS		MAKE_ID('E','T','P','S')

// GRADIENT
#define ID_GRAD		MAKE_ID('G','R','A','D')
#define ID_GRST		MAKE_ID('G','R','S','T')
#define ID_GREN		MAKE_ID('G','R','E','N')

// SHADER PLUGIN
#define ID_SHDR		MAKE_ID('S','H','D','R')
#define ID_DATA		MAKE_ID('D','A','T','A')

#ifdef _WIN32
#define MSB2			_SwapTwoBytes
#define MSB4			_SwapFourBytes 
#define LSB2(w)			(w)
#define LSB4(w)			(w)
#else
#define MSB2(w)			(w)
#define MSB4(w)			(w)
#define LSB2			_SwapTwoBytes
#define LSB4			_SwapFourBytes 
#endif

/*___________________
|
| Type defintions
|__________________*/

typedef char           I1;
typedef short          I2;
typedef int            I4;
typedef unsigned char  U1;
typedef unsigned short U2;
typedef unsigned int   U4;
typedef float          F4;
typedef unsigned int   VX;
typedef float          FP4;
typedef float          ANG4;
typedef float          VEC12[3];
typedef float          COL12[3];
typedef char           ID4[5];
typedef char           S0[255];
typedef char           FNAM0[255];

/*___________________
|
| Function prototypes
|__________________*/

static U4 read_tags (U4 nbytes, FILE *in, FILE *out);
static U4 read_clip (U4 nbytes, FILE *in, FILE *out);
static U4 read_envl (U4 nbytes, FILE *in, FILE *out);
static U4 read_layr (U4 nbytes, FILE *in, FILE *out);
static U4 read_pnts (U4 nbytes, FILE *in, FILE *out, bool verbose);
static U4 read_bbox (U4 nbytes, FILE *in, FILE *out);
static U4 read_pols (U4 nbytes, FILE *in, FILE *out, bool verbose);
static U4 read_ptag (U4 nbytes, FILE *in, FILE *out, bool verbose);
static U4 read_vmap (U4 nbytes, FILE *in, FILE *out, bool verbose);
static U4 read_surf (U4 nbytes, FILE *in, FILE *out);
static U4 read_blok (U4 nbytes, FILE *in, FILE *out);
static U4 read_tmap (U4 nbytes, FILE *in, FILE *out);
static int seek_pad (int size, FILE *in);
static int read_u1 (U1 *vals, int num, FILE *in);
static int read_u2 (U2 *vals, int num, FILE *in);
static int read_u4 (U4 *vals, int num, FILE *in);
static int read_i1 (I1 *vals, int num, FILE *in);
static int read_i2 (I2 *vals, int num, FILE *in);
static int read_f4 (F4 *vals, int num, FILE *in);
static int read_col12 (COL12 *col, int num, FILE *in);
static int read_vec12 (VEC12 *vec, int num, FILE *in);
static int read_vx (VX *vx, FILE *in);
static int read_name (S0 name, FILE *in);
static int read_id4 (ID4 id, FILE *in);
static U4 read_head (U4 nbytes, FILE *in, FILE *out);

static unsigned short _SwapTwoBytes (unsigned short w);
static unsigned long _SwapFourBytes (unsigned long w);

/*____________________________________________________________________
|
| Function: gx3d_PrintLWO2File
|
| Output: Translates a LWO2 file and creates an ASCII version.
|___________________________________________________________________*/

void gx3d_PrintLWO2File (char *filename, char *outputfilename, bool verbose)
{
  FILE *in, *out;
  U4	datasize, bytesread, type, size;
  ID4 id;
  int error = FALSE;

/*____________________________________________________________________
|
| Open input, output files.
|___________________________________________________________________*/

	// Open the LWO2 file
  in = fopen (filename, "rb");

  // Open the output ASCII file
  out = fopen (outputfilename, "wt");

/*____________________________________________________________________
|
| Read in header info
|___________________________________________________________________*/

  if (in AND out) {
  
    // Make sure the Lightwave file is an IFF file
	  read_id4 (id, in);
	  type = MAKE_ID (id[0], id[1], id[2], id[3]);
    if (type != ID_FORM) {
      fprintf (out, "Not an IFF file (Missing FORM tag)");
      error = TRUE;
    }

    if (NOT error) {
      // Read size of chunk data (should be equal to file size - 8 bytes)
      read_u4 (&datasize, 1, in);
      fprintf (out, "FORM [%d]\n", datasize);

      // Make sure the IFF file has a LWO2 form type
      bytesread = 0;
      bytesread += read_id4 (id, in);
	    type = MAKE_ID (id[0],id[1],id[2],id[3]);
      if (type != ID_LWO2) {
        fprintf (out, "Not a lightwave object (Missing LWO2 tag)");
        error = TRUE;
      }
    }

    if (NOT error)
      fprintf (out, "LWO2\n");

/*____________________________________________________________________
|
| Read in each data chunk
|___________________________________________________________________*/

    if (NOT error) {
      // Read all Lightwave chunks
      while (bytesread < datasize) {

	      bytesread += read_id4 (id, in);
	      bytesread += read_u4 (&size, 1, in);

	      type = MAKE_ID (id[0], id[1], id[2], id[3]);

        switch (type) {
          case ID_TAGS:	read_tags (size, in, out); 
                        break;
          case ID_CLIP:	read_clip (size, in, out); 
                        break;
          case ID_ENVL:	read_envl (size, in, out); 
                        break;
          case ID_LAYR:	read_layr (size, in, out); 
                        break;
          case ID_PNTS:	read_pnts (size, in, out, verbose); 
                        break;
          case ID_BBOX:	read_bbox (size, in, out); 
                        break;
          case ID_POLS:	read_pols (size, in, out, verbose); 
                        break;
          case ID_PTAG:	read_ptag (size, in, out, verbose); 
                        break;
          case ID_VMAP:	read_vmap (size, in, out, verbose); 
                        break;
          case ID_SURF:	read_surf (size, in, out); 
                        break;
          default:      // Skip over an unknown chunk
                        fprintf (out, "%s (unknown chunk) [%d]\n", id, size);
                  	    seek_pad (size, in);
                        break;
        }
        bytesread += size;
      }
    }
  }

/*____________________________________________________________________
|
| Close input, output files.
|___________________________________________________________________*/

  if (in)
    fclose (in);
  if (out)
    fclose (out);
}

/*____________________________________________________________________
|
| Function: read_tags
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a TAGS chunk and returns number of bytes read.
|___________________________________________________________________*/

static U4 read_tags (U4 nbytes, FILE *in, FILE *out)
{
  U4 n;
  S0 name;
	U4 bytesread = 0;

  fprintf (out, "TAGS [%d]\n", nbytes);

  n = 0;
  while (bytesread < nbytes) {
		bytesread += read_name (name, in);
		fprintf (out, "\t[%d] [%s]\n", n++, name);
	}
           
  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_clip
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a CLIP chunk and returns number of bytes read.
|___________________________________________________________________*/

static U4 read_clip (U4 nbytes, FILE *in, FILE *out)
{
  U4	bytes, type, index, byteshold;
  U2	size, u2[4];
	U1  u1[4];
	I2  i2[5];
  F4  f4[4];
	VX  vx[4];
  S0  name, ext, server;
	ID4 id;
  U4  bytesread = 0;

	bytesread += read_u4 (&index, 1, in);
  fprintf (out, "CLIP [%d] [%d]\n", nbytes, index);

  while (bytesread < nbytes) { 
	  if ((nbytes - bytesread) < 6) {
		  bytesread += seek_pad ((nbytes - bytesread), in);
      return (bytesread);
	  }

    // Handle the various sub-chunks
    bytesread += read_id4 (id, in);
	  bytesread += read_u2 (&size, 1, in);
	  type = MAKE_ID (id[0], id[1], id[2], id[3]);
	  byteshold = bytesread;
    
    fprintf (out, "\t[%s] (%d) ", id, size);

    switch (type) {
      case ID_STIL:
    		bytesread += read_name (name, in);
	      fprintf (out, "<%s>\n", name);
  	    break;
      case ID_ISEQ:
    		bytesread += read_u1 (u1, 2, in);
    		bytesread += read_i2 (i2, 2, in);
    		bytesread += read_name (name, in);
    		bytesread += read_name (ext, in);
	      fprintf (out, "<%d> <%d> <%d> <%d> <%s> <%s>\n", u1[0], u1[1], i2[0], i2[1], name, ext);
		    break;
      case ID_ANIM:
		    bytesread += read_name (name, in);
		    bytesread += read_name (server, in);
	      fprintf (out, "<%s> <%s>\n", name, server);
	      break;
      case ID_XREF:
		    bytesread += read_u4 (&index, 1, in);
		    bytesread += read_name (name, in);
	      fprintf (out, "<%d> <%s>\n", index, name);
		    break;
      case ID_ALPH:
		    bytesread += read_vx(vx, in);
	      fprintf (out, "<%d>\n", vx[0]);
		    break;
      case ID_STCC:
		    bytesread += read_i2 (i2, 2, in);
		    bytesread += read_name (name, in);
	      //fprintf (out, "<%d> <%d> <%s> <%s>\n", i2[0], i2[1], name);
        fprintf(out,"<%d> <%d> <%s>\n",i2[0],i2[1],name); // changed 10/5/2017
    		break;
      case ID_CONT:
      case ID_BRIT:
      case ID_SATR:
      case ID_HUE:
      case ID_GAMM:
		    bytesread += read_f4 (f4, 1, in);
		    bytesread += read_vx (vx, in);
	      fprintf (out, "<%f> <%d>\n", f4[0], vx[0]);
		    break;
      case ID_NEGA:
		    bytesread += read_u2 (u2, 1, in);
	      fprintf (out, "<%d>\n", u2[0]);
		    break;
      case ID_CROP:
		    bytesread += read_f4 (f4, 4, in);
	      fprintf (out, "<%f> <%f> <%f> <%f>\n", f4[0], f4[1], f4[2], f4[3]);
		    break;
      case ID_COMP:
		    bytesread += read_vx (vx, in);
		    bytesread += read_f4 (f4, 1, in);
		    bytesread += read_vx (vx+1, in);
	      fprintf (out, "<%d> <%f> <%d>\n", vx[0], f4[0], vx[1]);
		    break;
      case ID_IFLT:
      case ID_PFLT:
        bytes = bytesread;
		    bytesread += read_name (name, in);
		    bytesread += read_i2 (i2, 1, in);
		    bytesread += seek_pad (size-(bytesread-bytes), in);
	      fprintf (out, "<%s> <%d> \n", name, i2[0]);
		    break;
      default:
		    bytesread += seek_pad (size, in);
        fprintf (out, "(%d bytes)\n", size);
    }
	  
    if ((size - bytesread + byteshold) > 0) 
		  bytesread += seek_pad ((size - bytesread + byteshold), in);
  }

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_envl
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a ENVL chunk and returns number of bytes read.
|___________________________________________________________________*/

static U4 read_envl (U4 nbytes, FILE *in, FILE *out)
{
  U4  bytes, type, byteshold;
  U2	size, u2[4], n, count;
	U1	u1[4];
	I2	index;
  F4	f4[4];
  S0  name;
	ID4 id;
  U4  bytesread = 0;

	bytesread += read_i2 ( &index, 1, in);
  fprintf (out, "ENVL [%d] [%d]\n", nbytes, index);

  while (bytesread < nbytes) {
	  if ((nbytes - bytesread) < 6) {
		  bytesread += seek_pad ((nbytes - bytesread), in);
    	return (bytesread);
	  }

    // Handle the various sub-chunks
	  bytesread += read_id4 (id, in);
	  bytesread += read_u2 (&size, 1, in);
	  type = MAKE_ID (id[0], id[1], id[2], id[3]);

	  byteshold = bytesread;
    fprintf (out, "\t[%s] (%d) ", id, size);

    switch (type) {
      case ID_PRE:
      case ID_POST:
		    bytesread += read_u2 (u2, 1, in);
	      fprintf (out, "<%d>\n", u2[0]);
	    	break;
      case ID_TYPE:
		    bytesread += read_u2 (u2, 1, in);
	      fprintf (out, "<%04x>\n", u2[0]);
		    break;
      case ID_KEY:
		    bytesread += read_f4 (f4, 2, in);
	      fprintf (out, "<%f> <%f>\n", f4[0], f4[1]);
		    if (size != 8) 
          size = 8;				// BUG FOR SurfaceEditor
		    break;
      case ID_SPAN:
		    bytes = bytesread;
		    bytesread += read_id4(id, in);
	      fprintf (out, "<%s>", id);
		    count = (size - bytesread + bytes) / sizeof(F4);
		    for (n = 0; n < count; n++) {
			    bytesread += read_f4(f4, 1, in);
	    	  fprintf (out, " <%f>", f4[0]);
    		}
	      fprintf (out, "\n");
    		break;
      case ID_CHAN:
		    bytes = bytesread;
		    bytesread += read_name(name, in);
		    bytesread += read_u2(u2, 1, in);
	      fprintf (out, "<%s> <%d>\n", name, u2[0]);
    		for (n = 0; n < (size-bytesread+bytes); n++) {
		    	bytesread += read_u1 (u1, 1, in);
			    if (!(n % 8)) 
            fprintf (out, "\t");
    			fprintf (out, "<0x%02x> ", u1[0]);
    			if (!((n+1) % 8)) 
            fprintf (out, "\n");
		    }
		    fprintf (out, "\n");
  	    break;
      case ID_NAME:
	    	bytesread += read_name (name, in);
	      fprintf (out, "<%s>\n", name);
	      break;
      default:
		    bytesread += seek_pad (size, in);
        fprintf (out, "(%d bytes)\n", size);
    }
	  
    if ((size - bytesread + byteshold) > 0) 
		  bytesread += seek_pad ((size - bytesread + byteshold), in);
  }

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_layr
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a LAYR chunk and returns number of bytes read.
|___________________________________________________________________*/

static U4 read_layr (U4 nbytes, FILE *in, FILE *out)
{
	U2    number, flags, parent;
	VEC12 pivot;
  S0    name;
	U4    bytesread = 0;

  fprintf (out, "\nLAYR [%d]", nbytes);

	// Read layer number, flags, pivot, name
  bytesread += read_u2 (&number, 1, in);
  bytesread += read_u2 (&flags, 1, in);
	bytesread += read_vec12 (&pivot, 1, in);
	bytesread += read_name (name, in);

  fprintf (out, "\n");
  fprintf (out, "\tNUMBER [%d] FLAGS [0x%04x] PIVOT [%f,%f,%f]\n", number, flags, pivot[0], pivot[1], pivot[2]);
  fprintf (out, "\tNAME [%s]\n", name);

  // Optionally, read parent
  if ((nbytes-bytesread) == sizeof(U2)) {
		bytesread += read_u2 (&parent, 1, in);
		fprintf (out, "\tPARENT [%d]\n", parent);
	}

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_pnts
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a PNTS (vertices) chunk and returns number of bytes read.
|___________________________________________________________________*/

static U4 read_pnts (U4 nbytes, FILE *in, FILE *out, bool verbose)
{
	U4     i, nPts;
  VEC12 *fpt;
  U4     bytesread = 0;

  nPts = nbytes / sizeof(VEC12);
  fprintf (out, "PNTS [%d] nPts [%d]\n", nbytes, nPts);

  fpt = (VEC12 *) malloc (sizeof(VEC12) * nPts);
	bytesread = read_vec12 (fpt, nPts, in);

  if (verbose)
		for (i = 0; i < nPts; i++) 
			fprintf (out, "\t[%d] [%f,%f,%f]\n", i, fpt[i][0], fpt[i][1], fpt[i][2]);

  free (fpt);

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_bbox
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a BBOX (bounding box) chunk and returns number of bytes 
|   read.
|___________________________________________________________________*/

static U4 read_bbox (U4 nbytes, FILE *in, FILE *out)
{
  F4 bbox[6];

	read_f4 (bbox, 6, in);
  fprintf (out, "BBOX [%d]\n", nbytes);
  fprintf (out, "\tMIN [%f,%f,%f]\n", bbox[0], bbox[1], bbox[2]);
  fprintf (out, "\tMAX [%f,%f,%f]\n", bbox[3], bbox[4], bbox[5]);

  return (nbytes);
}

/*____________________________________________________________________
|
| Function: read_pols
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a POLS (polygons) chunk and returns number of bytes 
|   read.
|___________________________________________________________________*/

static U4 read_pols (U4 nbytes, FILE *in, FILE *out, bool verbose)
{
  U2	numvert, flags;
  U4	nPols, bytesread;
	VX  vx;
	ID4 id;
	int n;

  fprintf (out, "POLS [%d]", nbytes);

	bytesread = 0;
  nPols     = 0;

	bytesread += read_id4 (id, in);
	fprintf (out, " [%s]\n", id);

	while (bytesread < nbytes) {
		bytesread += read_u2 (&numvert, 1, in);
		flags      = (0xfc00 & numvert) >> 10;
		numvert    =  0x03ff & numvert;
    if (verbose)
			fprintf (out, "\t[%d] NVERT[%d] FLAG[%02x] <", nPols, numvert, flags);
		nPols++;

		for (n = 0; n < (int)numvert; n++) {
			bytesread += read_vx (&vx, in);
			if (verbose) {
				if (n+1 == numvert) 
					fprintf (out, "%d>\n", vx);
				else                
					fprintf (out, "%d, ", vx);
			}
		}
	}
	if (bytesread != nbytes) 
    fprintf (out, "??? %d != %d\n", bytesread, nbytes);

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_ptag
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a POLS (polygon tags) chunk and returns number of bytes 
|   read.
|___________________________________________________________________*/

static U4 read_ptag (U4 nbytes, FILE *in, FILE *out, bool verbose)
{
  U2	tag;
  U4	nTags, bytesread;
	VX  vx;
	ID4 id;

  fprintf (out, "PTAG [%d]", nbytes);

	bytesread = 0;
  nTags     = 0;

	bytesread += read_id4 (id, in);
	fprintf (out, " [%s]\n", id);

  while (bytesread < nbytes) {
		bytesread += read_vx (&vx, in);
		bytesread += read_u2 (&tag, 1, in);
    if (verbose)
			fprintf (out, "\tPOLY[%d] TAG[%d]\n", vx, tag); 
    nTags++;
	}
	if (bytesread != nbytes) 
    fprintf (out, "??? %d != %d\n", bytesread, nbytes);

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_vmap
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a VMAP (vertex map) chunk and returns number of bytes 
|   read.
|___________________________________________________________________*/

static U4 read_vmap (U4 nbytes, FILE *in, FILE *out, bool verbose)
{
  U2	dim;
	VX  vx;
	F4  value;
	S0  name;
	ID4 id;
	int n;
  U4	bytesread = 0;

  fprintf (out, "VMAP [%d]", nbytes);

	bytesread += read_id4 (id, in);
	fprintf (out, " [%s]", id);

	bytesread += read_u2 (&dim, 1, in);
	bytesread += read_name (name, in);
	fprintf (out, " DIM [%d] NAME [%s]\n", dim, name);

	while (bytesread < nbytes) {
		bytesread += read_vx (&vx, in);
		if (dim == 0) {
			if (verbose)
				fprintf (out, "\tVERT[%d]\n", vx);
		}
		else {
			if (verbose)
				fprintf (out, "\tVERT[%d] VALS[", vx);
			for (n = 0; n < (int) dim; n++) {
				bytesread += read_f4 (&value, 1, in);
				if (verbose) {
					if (n+1 == dim) 
						fprintf (out, "%f]\n", value);
					else            
						fprintf (out, "%f, " , value);
				}
			}
		}
	}
	if (bytesread != nbytes) 
    fprintf (out, "??? %d != %d\n", bytesread, nbytes);

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_surf
|
| Input: Called from LWO2_To_ASCII()
| Output: Reads a SURF chunk and returns number of bytes 
|   read.
|___________________________________________________________________*/

static U4 read_surf (U4 nbytes, FILE *in, FILE *out)
{
  U4	  type, byteshold;
  U2    size, u2[4];
  F4    f4[4];
	VX    vx[4];
  S0    name, source, s0;
	ID4   id;
	COL12 col;
  U4    bytesread = 0;

  fprintf (out, "SURF [%d]\n", nbytes);

	bytesread += read_name (name  , in);
	bytesread += read_name (source, in);

  fprintf (out, "[%s] [%s]\n", name, source);

  while (bytesread < nbytes) {
	  if ((nbytes - bytesread) < 6) {
		  bytesread += seek_pad ((nbytes - bytesread), in);
    	return (bytesread);
	  }

    // Handle the various sub-chunks
	  bytesread += read_id4 (id, in);
	  bytesread += read_u2 (&size, 1, in);
	  type = MAKE_ID (id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
    fprintf (out, "\t[%s] (%d) ", id, size);

    switch (type) {
      case ID_COLR:
      case ID_LCOL:
    		bytesread += read_col12 (&col, 1, in);
	    	bytesread += read_vx (vx, in);
	      fprintf (out, "<%f,%f,%f> <%d>\n", col[0], col[1], col[2], vx[0]);
	      break;
      case ID_DIFF:
      case ID_LUMI:
      case ID_SPEC:
      case ID_REFL:
      case ID_TRAN:
      case ID_TRNL:
      case ID_GLOS:
      case ID_SHRP:
      case ID_BUMP:
      case ID_RSAN:
      case ID_RIND:
      case ID_CLRH:
      case ID_CLRF:
      case ID_ADTR:
      case ID_GVAL:
      case ID_LSIZ:
    		bytesread += read_f4 (f4, 1, in);
	    	bytesread += read_vx (vx, in);
	      fprintf (out, "<%f> <%d>\n", f4[0], vx[0]);
    		break;
      case ID_SIDE:
      case ID_RFOP:
      case ID_TROP:
		    bytesread += read_u2 (u2, 1, in);
		    bytesread += seek_pad (size-sizeof(U2), in);
	      fprintf (out, "<%d>\n", u2[0]);
		    break;
      case ID_SMAN:
		    bytesread += read_f4 (f4, 1, in);
	      fprintf (out, "<%f>\n", f4[0]);
    		break;
      case ID_RIMG:
      case ID_TIMG:
		    bytesread += read_vx (vx, in);
	      fprintf (out, "<%d>\n", vx[0]);
		    break;
      case ID_GLOW:
		    bytesread += read_u2 (u2, 1, in);
		    bytesread += read_f4 (f4, 1, in);
		    bytesread += read_vx (vx, in);
		    bytesread += read_f4 (f4+1, 1, in);
		    bytesread += read_vx (vx+1, in);
  	    fprintf (out, "<%d> <%f> <%d> <%f> <%d>\n", u2[0], f4[0], vx[0], f4[1], vx[1]);
    		break;
      case ID_LINE:
		    bytesread += read_u2 (u2, 1, in);
        if ( size > 2 ) {
		      bytesread += read_f4 (f4, 1, in);
		      bytesread += read_vx (vx, in);
		      if (size > 8) {
			      bytesread += read_col12 (&col, 1, in);
			      bytesread += read_vx (vx+1, in);
	    	    fprintf (out, "<%d> <%f> <%d> <%f,%f,%f> <%d>\n", u2[0], f4[0], vx[0], col[0], col[1], col[2], vx[1]);
		      } 
          else 
	    	    fprintf (out, "<%d> <%f> <%d>\n", u2[0], f4[0], vx[0]);
        }
        else
          fprintf (out, "<%d>\n", u2[0]);
		    break;
      case ID_ALPH:
		    bytesread += read_u2 (u2, 1, in);
		    bytesread += read_f4 (f4, 1, in);
	      fprintf (out, "<%d> <%f>\n", u2[0], f4[0]);
		    break;
      case ID_AVAL:
		    bytesread += read_f4 (f4, 1, in);
	      fprintf (out, "<%f>\n", f4[0]);
		    break;
      case ID_BLOK:
	      fprintf (out, "\n");
	      bytesread += read_blok (size, in, out);
		    break;
      case ID_CMNT:
		    memset (s0, 0x00, sizeof(s0));
	    	bytesread += read_u1 ((U1 *)s0, size, in);
	      fprintf (out, "<%s>\n", s0);
    		break;
      default:
		    bytesread += seek_pad (size, in);
        fprintf (out, "(%d bytes)\n", size);
    }
	  
    if ((size - bytesread + byteshold) > 0)
		  bytesread += seek_pad ((size - bytesread + byteshold), in);
  }

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: seek_pad
|
| Input: Called from ____
| Output: Skips bytes in a file and returns number skipped.
|___________________________________________________________________*/

static int seek_pad (int size, FILE *in)
{
	if (size > 0) 
    fseek (in, (long)size, SEEK_CUR);

  return (size);
}

/*____________________________________________________________________
|
| Function: read_u1
|
| Input: Called from ____
| Output: 
|___________________________________________________________________*/

static int read_u1 (U1 *vals, int num, FILE *in)
{
  return ((int)fread ((void *)vals, sizeof(U1), num, in));
}

/*____________________________________________________________________
|
| Function: read_u2
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_u2 (U2 *vals, int num, FILE *in)
{
	int i, bytesread;

  bytesread = (int) fread ((void *)vals, sizeof(U2), num, in);
  if (bytesread != num) 
		return (0);
	else {
		for (i = 0; i < num; i++) 
			vals[i] = MSB2 (vals[i]);
		return (sizeof(U2) * num);
	}
}

/*____________________________________________________________________
|
| Function: read_u4
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_u4 (U4 *vals, int num, FILE *in)
{
	int i, bytesread;

  bytesread = (int) fread ((void *)vals, sizeof(U4), num, in);
  if (bytesread != num) 
		return (0);
	else {
		for (i = 0; i < num; i++) 
			vals[i] = MSB4 (vals[i]);
	  return (sizeof(U4) * num);
	}
}

/*____________________________________________________________________
|
| Function: read_i1
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_i1 (I1 *vals, int num, FILE *in)
{
	return (read_u1 ((U1 *)vals, num, in));
}

/*____________________________________________________________________
|
| Function: read_i2
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_i2 (I2 *vals, int num, FILE *in)
{
	return (read_u2 ((U2 *)vals, num, in));
}

/*____________________________________________________________________
|
| Function: read_f4
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_f4 (F4 *vals, int num, FILE *in)
{
	return (read_u4 ((U4 *)vals, num, in));
}

/*____________________________________________________________________
|
| Function: read_col12
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_col12 (COL12 *col, int num, FILE *in)
{
	return (read_f4 ((F4 *)col, 3 * num, in));
}

/*____________________________________________________________________
|
| Function: read_vec12
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_vec12 (VEC12 *vec, int num, FILE *in)
{
	return (read_f4 ((F4 *)vec, 3 * num, in));
}

/*____________________________________________________________________
|
| Function: read_vx
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_vx (VX *vx, FILE *in)
{
	int ch, bytesread = 0;

	*vx = 0L;
	ch = fgetc (in);
	if (ch == 0xff) {
		ch = fgetc (in); *vx |= ch << 16;
		ch = fgetc (in); *vx |= ch << 8;
		ch = fgetc (in); *vx |= ch;
		bytesread = 4;
	} 
  else {
		*vx |= ch << 8;
		ch = fgetc (in); *vx |= ch;
		bytesread = 2;
	}

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_name
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_name (S0 name, FILE *in)
{
	int ch, bytesread = 0;

  do {
    ch = fgetc (in);
    name[bytesread++] = ch;
  } while (ch);

	if (bytesread & 1) {
    ch = fgetc (in); 
    bytesread++;
	}
	
  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_id4
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int read_id4 (ID4 id, FILE *in)
{
	int bytesread = 0;

  id[bytesread++] = fgetc (in);
  id[bytesread++] = fgetc (in);
  id[bytesread++] = fgetc (in);
  id[bytesread++] = fgetc (in);
  id[bytesread  ] = 0x00;

	return (bytesread);
}


/*____________________________________________________________________
|
| Function: read_head
|
| Input: Called from ____
| Output: Reads a BLOK header chunk and returns # bytes read.
|___________________________________________________________________*/

static U4 read_head (U4 nbytes, FILE *in, FILE *out)
{
  U4	type, byteshold;
	U2  size, u2[4];
	VX  vx[4];
	F4  f4[4];
  ID4 id;
  S0	name;
  U4  bytesread = 0;  

  bytesread += read_name (name, in);
	fprintf (out, "<%s>\n", name);

  while (bytesread < nbytes) {

    // Handle the various sub-chunks
	  bytesread += read_id4 (id, in);
	  bytesread += read_u2 (&size, 1, in);
	  type = MAKE_ID (id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
    fprintf (out, "\t\t\t[%s] (%d) ", id, size);

    switch (type) {
      case ID_CHAN:
    		bytesread += read_id4 (id, in);
  	    fprintf (out, "<%s>\n", id);
	      break;
      case ID_NAME:
      case ID_OREF:
    		bytesread += read_name (name, in);
	      fprintf (out, "<%s>\n", name);
	      break;
      case ID_ENAB:
      case ID_AXIS:
      case ID_NEGA:
		    bytesread += read_u2 (u2, 1, in);
	      fprintf (out, "<%d>\n", u2[0]);
	      break;
      case ID_OPAC:
		    bytesread += read_u2 (u2, 1, in);
		    bytesread += read_f4 (f4, 1, in);
		    bytesread += read_vx (vx, in);
	      fprintf (out, "<%d> <%f> <%d>\n", u2[0], f4[0], vx[0]);
	      break;
      default:
    		bytesread += seek_pad (size, in);
        fprintf (out, "(%d bytes)\n", size);
    		break;
	  }

    if ((size - bytesread + byteshold) > 0) 
		  bytesread += seek_pad ((size - bytesread + byteshold), in);
  }

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_blok
|
| Input: Called from read_surf()
| Output: Reads a BLOK chunk and returns # bytes read.
|___________________________________________________________________*/

static U4 read_blok (U4 nbytes, FILE *in, FILE *out)
{
  U4	  type, byteshold;
	U2    size, u2[4];
	U1		u1[10];
	S0    name;
	F4    f4[256];
	I2    i2[256];
	VX    vx[4];
  ID4   id;
	COL12 col;
	int   i, n;
  U4	  bytesread = 0;

  while (bytesread < nbytes) {

    // Handle the various sub-chunks
	  bytesread += read_id4 (id, in);
	  bytesread += read_u2 (&size, 1, in);
	  type = MAKE_ID (id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
    fprintf (out, "\t\t[%s] (%d) ", id, size);

    switch (type) {
      case ID_IMAP:
      case ID_PROC:
      case ID_GRAD:
      case ID_SHDR:
  	    bytesread += read_head (size, in, out);
	      break;
      case ID_VMAP:
    		bytesread += read_name (name, in);
	      fprintf (out, "<%s>\n", name);
	      break;
      case ID_FLAG:
      case ID_AXIS:
      case ID_PROJ:
      case ID_PIXB:
    		bytesread += read_u2 (u2, 1, in);
	      fprintf (out, "<%d>\n", u2[0]);
	      break;
      case ID_TMAP:
  	    bytesread += read_tmap (size, in, out);
	      break;
      case ID_IMAG:
    		bytesread += read_vx (vx, in);
	      fprintf (out, "<%d>\n", vx[0]);
	      break;
      case ID_WRAP:
    		bytesread += read_u2 (u2, 2, in);
	      fprintf (out, "<%d, %d>\n", u2[0], u2[1]);
	      break;
      case ID_WRPW:
      case ID_WRPH:
      case ID_TAMP:
    		bytesread += read_f4 (f4, 1, in);
		    bytesread += read_vx (vx, in);
	      fprintf (out, "<%f> <%d>\n", f4[0], vx[0]);
	      break;
      case ID_VALU:
    		bytesread += read_f4 (f4, size / sizeof(F4), in);
		    for (i = 0; i < (int)(size / sizeof(F4)); i++) 
	    	  fprintf (out, "<%f> ", f4[i]);
  	    fprintf (out, "\n");
	      break;
      case ID_AAST:
      case ID_STCK:
    		bytesread += read_u2 (u2, 1, in);
    		bytesread += read_f4 (f4, 1, in);
	      fprintf (out, "<%d> <%f>\n", u2[0], f4[0] );
	      break;
      case ID_GRST:
      case ID_GREN:
    		bytesread += read_f4 (f4, 1, in);
	      fprintf (out, "<%f>\n", f4[0] );
	      break;
      case ID_COLR:
    		bytesread += read_col12 (&col, 1, in);
	    	bytesread += read_vx (vx, in);
	      fprintf (out, "<%f,%f,%f> <%d>\n", col[0], col[1], col[2], vx[0]);
	      break;
      case ID_FUNC:
    		bytesread += n = read_name (name, in);
	      fprintf (out, "<%s> ", name);
		    for (i = 0; i < (size -n); i++) {
			    bytesread += read_u1 (u1, 1, in);
			    fprintf (out, "<0x%02x> ", u1[0]);
		    }
		    fprintf (out, "\n");
		    break;
      case ID_FTPS:
		    n = size / sizeof(F4);
		    bytesread += read_f4 (f4, n, in);
		    for (i = 0; i < n; i++) 
          fprintf (out, "<%f> ", f4[i]);
		    fprintf (out, "\n");
		    break;
      case ID_ITPS:
		    n = size / sizeof(I2);
		    bytesread += read_i2 (i2, n, in);
		    for (i = 0; i < n; i++) 
          fprintf (out, "<%d> ", i2[i]);
		    fprintf (out, "\n");
	      break;
      case ID_ETPS:
		    while (size > 0) {
			    n = read_vx (vx, in);
			    bytesread += n;
			    size -= n;
			    fprintf (out, "<%d> ", vx[0]);
		    }
	      fprintf (out, "\n");
	      break;
      default:
		    bytesread += seek_pad (size, in);
        fprintf (out, "(%d bytes)\n", size);
		    break;
	  }
	  
    if ((size - bytesread + byteshold) > 0) 
		  bytesread += seek_pad ((size - bytesread + byteshold), in);
  }

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: read_tmap
|
| Input: Called from read_blok()
| Output: Reads a TMAP chunk and returns # bytes read.
|___________________________________________________________________*/

static U4 read_tmap (U4 nbytes, FILE *in, FILE *out)
{
  U4	  type, byteshold;
	U2    size, u2[4];
	VX    vx[4];
  ID4   id;
  VEC12	vec;
  S0		name;
  U4	  bytesread = 0;

  fprintf (out, "\n");

  while (bytesread < nbytes) {

    // Handle the various sub-chunks
	  bytesread += read_id4 (id, in);
	  bytesread += read_u2 (&size, 1, in);
	  type = MAKE_ID (id[0],id[1],id[2],id[3]);

	  byteshold = bytesread;
    fprintf (out, "\t\t\t[%s] (%d) ", id, size);

    switch (type) {
      case ID_CNTR:
      case ID_SIZE:
      case ID_ROTA:
    		bytesread += read_vec12 (&vec, 1, in);
		    bytesread += read_vx (vx, in);
	      fprintf (out, "<%f,%f,%f> <%d>\n", vec[0], vec[1], vec[2], vx[0]);
	      break;
      case ID_FALL:
		    bytesread += read_u2 (u2, 1, in);
		    bytesread += read_vec12 (&vec, 1, in);
		    bytesread += read_vx (vx, in);
	      fprintf (out, "<%d> <%f,%f,%f> <%d>\n", u2[0], vec[0], vec[1], vec[2], vx[0]);
	      break;
      case ID_OREF:
    		bytesread += read_name (name, in);
	      fprintf (out, "<%s>\n", name);
	      break;
      case ID_CSYS:
    		bytesread += read_u2(u2, 1, in);
	      fprintf (out, "<%d>\n", u2[0]);
	      break;
      default:
    		bytesread += seek_pad (size, in);
        fprintf (out, "(%d bytes)\n", size);
		    break;
	  }

	  if ((size - bytesread + byteshold) > 0) 
		  bytesread += seek_pad ((size - bytesread + byteshold), in);
  }

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: _SwapTwoBytes
|
| Input: Called from ____
| Output: Swap byte order for WIN32.
|___________________________________________________________________*/

static unsigned short _SwapTwoBytes (unsigned short w)
{
	unsigned short tmp;
	
  tmp =  (w & 0x00ff);
	tmp = ((w & 0xff00) >> 0x08) | (tmp << 0x08);

  return (tmp);
}

/*____________________________________________________________________
|
| Function: _SwapFourBytes
|
| Input: Called from ____
| Output: Swap byte order for WIN32.
|___________________________________________________________________*/

static unsigned long _SwapFourBytes (unsigned long w)
{
	unsigned long tmp;

  tmp =  (w & 0x000000ff);
	tmp = ((w & 0x0000ff00) >> 0x08) | (tmp << 0x08);
	tmp = ((w & 0x00ff0000) >> 0x10) | (tmp << 0x08);
	tmp = ((w & 0xff000000) >> 0x18) | (tmp << 0x08);
	
  return (tmp);
}

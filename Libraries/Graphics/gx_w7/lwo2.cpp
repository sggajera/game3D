/*____________________________________________________________________
|
| File: lwo2.cpp
|
| Description: Reads data from a LWO2 (Lightwave 6.5) file and builds a
|              lwo2_Object.
|
|              Based on L65 SDK Oct 23, 2000 "Object Files".
|
| Functions:  lwo2_ReadObjectFile
|							 Cleanup_Morph_Maps
|              Read_LWO2_Chunk
|               Read_TAGS_Chunk
|                Count_Tags
|               Read_LAYR_Chunk
|               Read_PNTS_Chunk
|               Read_VMAP_Chunk
|               Read_POLS_Chunk
|                Count_Polygons
|               Read_PTAG_Chunk
|               Read_BBOX_Chunk
|               Read_SURF_Chunk
|                Read_BLOK_Subchunk
|                 Read_BLOK_Header_Subchunk
|               Read_CLIP_Chunk
|                Convert_NFilename_To_Filename
|
|               Skip_Bytes
|               Read_U1
|               Read_U2
|               Read_U4
|               Read_I1
|               Read_I2
|               Read_F4
|               Read_COL12
|               Read_VEC12
|               Read_GX
|               Sizeof_GX
|               Read_Name
|               Read_ID4
|                Swap_Bytes
|
|             lwo2_WriteObject  // not implemented yet!
|             lwo2_FreeObject
|
| Notes: All distances are saved in LWO2 files in meters by convention.
|   When reading in LWO2 files, the meters are converted into feet.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

//#define DEBUG

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <math.h>

#include "dp.h"

#include "lwo2.h"

/*___________________
|
| Type definitions
|__________________*/

typedef char           I1;
typedef short          I2;
typedef int            I4;
typedef unsigned char  U1;
typedef unsigned short U2;
typedef unsigned int   U4;
typedef float          F4;
typedef unsigned int   GX;
typedef float          FP4;
typedef float          ANG4;
typedef float          VEC12[3];
typedef float          COL12[3];
typedef char           ID4[5];
typedef char           S0[255];
typedef char           FNAM0[255];

/*___________________
|
| Defines
|__________________*/

#define MAKE_ID(_a_,_b_,_c_,_d_) \
  ((unsigned long) (_a_)<<24 | (unsigned long) (_b_)<<16 | (unsigned long) (_c_)<<8 | (unsigned long) (_d_))

// Universal IFF identifiers
#define ID_FORM	MAKE_ID('F','O','R','M')
#define ID_LWO2	MAKE_ID('L','W','O','2')

// Primary Chunk ID
#define ID_LAYR MAKE_ID('L','A','Y','R')
#define ID_PNTS	MAKE_ID('P','N','T','S')
#define ID_VMAP	MAKE_ID('V','M','A','P')
#define ID_POLS	MAKE_ID('P','O','L','S')
#define ID_TAGS	MAKE_ID('T','A','G','S')
#define ID_PTAG	MAKE_ID('P','T','A','G')
#define ID_ENVL	MAKE_ID('E','N','V','L')
#define ID_CLIP	MAKE_ID('C','L','I','P')
#define ID_BBOX	MAKE_ID('B','B','O','X')
#define ID_DESC	MAKE_ID('D','E','S','C')
#define ID_TEXT	MAKE_ID('T','E','X','T')
#define ID_ICON	MAKE_ID('I','C','O','N')

// PTAG chunk type (supported types)
#define ID_SURF	MAKE_ID('S','U','R','F')
#define ID_BONE	MAKE_ID('B','O','N','E')
#define ID_BNWT	MAKE_ID('B','N','W','T')

// IMAGE subchunk ID
#define ID_STIL	MAKE_ID('S','T','I','L')
#define ID_ISEQ	MAKE_ID('I','S','E','Q')
#define ID_ANIM	MAKE_ID('A','N','I','M')
#define ID_XREF	MAKE_ID('X','R','E','F')
#define ID_STCC	MAKE_ID('S','T','C','C')
#define ID_CONT	MAKE_ID('C','O','N','T')
#define ID_BRIT	MAKE_ID('B','R','I','T')
#define ID_SATR	MAKE_ID('S','A','T','R')
#define ID_HUE	MAKE_ID('H','U','E',' ')
#define ID_GAMM	MAKE_ID('G','A','M','M')
#define ID_NEGA	MAKE_ID('N','E','G','A')
#define ID_CROP	MAKE_ID('C','R','O','P')
#define ID_ALPH	MAKE_ID('A','L','P','H')
#define ID_COMP	MAKE_ID('C','O','M','P')
#define ID_IFLT	MAKE_ID('I','F','L','T')
#define ID_PFLT	MAKE_ID('P','F','L','T')

// ENVELOPE subchunk
#define ID_PRE	MAKE_ID('P','R','E',' ')
#define ID_POST	MAKE_ID('P','O','S','T')
#define ID_KEY	MAKE_ID('K','E','Y',' ')
#define ID_SPAN	MAKE_ID('S','P','A','N')
#define ID_CHAN	MAKE_ID('C','H','A','N')

// SURFACE subchunk ID
#define ID_COLR	MAKE_ID('C','O','L','R')
#define ID_DIFF	MAKE_ID('D','I','F','F')
#define ID_LUMI	MAKE_ID('L','U','M','I')
#define ID_SPEC	MAKE_ID('S','P','E','C')
#define ID_REFL	MAKE_ID('R','E','F','L')
#define ID_TRAN	MAKE_ID('T','R','A','N')
#define ID_TRNL	MAKE_ID('T','R','N','L')
#define ID_GLOS	MAKE_ID('G','L','O','S')
#define ID_SHRP	MAKE_ID('S','H','R','P')
#define ID_BUMP	MAKE_ID('B','U','M','P')
#define ID_SIDE	MAKE_ID('S','I','D','E')
#define ID_SMAN	MAKE_ID('S','M','A','N')
#define ID_RFOP	MAKE_ID('R','F','O','P')
#define ID_RIMG	MAKE_ID('R','I','M','G')
#define ID_RSAN	MAKE_ID('R','S','A','N')
#define ID_RIND	MAKE_ID('R','I','N','D')
#define ID_CLRH	MAKE_ID('C','L','R','H')
#define ID_TROP	MAKE_ID('T','R','O','P')
#define ID_TIMG	MAKE_ID('T','I','M','G')
#define ID_CLRF	MAKE_ID('C','L','R','F')
#define ID_ADTR	MAKE_ID('A','D','T','R')
#define ID_GLOW	MAKE_ID('G','L','O','W')
#define ID_LINE	MAKE_ID('L','I','N','E')
#define ID_ALPH	MAKE_ID('A','L','P','H')
#define ID_AVAL	MAKE_ID('A','V','A','L')
#define ID_GVAL	MAKE_ID('G','V','A','L')
#define ID_BLOK	MAKE_ID('B','L','O','K')
#define ID_LCOL	MAKE_ID('L','C','O','L')
#define ID_LSIZ	MAKE_ID('L','S','I','Z')
#define ID_CMNT	MAKE_ID('C','M','N','T')

// Texture layer
#define ID_CHAN	MAKE_ID('C','H','A','N')
#define ID_TYPE	MAKE_ID('T','Y','P','E')
#define ID_NAME	MAKE_ID('N','A','M','E')
#define ID_ENAB	MAKE_ID('E','N','A','B')
#define ID_OPAC	MAKE_ID('O','P','A','C')
#define ID_FLAG	MAKE_ID('F','L','A','G')
#define ID_PROJ	MAKE_ID('P','R','O','J')
#define ID_STCK	MAKE_ID('S','T','C','K')
#define ID_TAMP	MAKE_ID('T','A','M','P')

// Texture Mapping
#define ID_TMAP	MAKE_ID('T','M','A','P')
#define ID_AXIS	MAKE_ID('A','X','I','S')
#define ID_CNTR	MAKE_ID('C','N','T','R')
#define ID_SIZE	MAKE_ID('S','I','Z','E')
#define ID_ROTA	MAKE_ID('R','O','T','A')
#define ID_OREF	MAKE_ID('O','R','E','F')
#define ID_FALL	MAKE_ID('F','A','L','L')
#define ID_CSYS	MAKE_ID('C','S','Y','S')

// Image Map
#define ID_IMAP	MAKE_ID('I','M','A','P')
#define ID_IMAG	MAKE_ID('I','M','A','G')
#define ID_WRAP	MAKE_ID('W','R','A','P')
#define ID_WRPW	MAKE_ID('W','R','P','W')
#define ID_WRPH	MAKE_ID('W','R','P','H')
#define ID_VMAP	MAKE_ID('V','M','A','P')
#define ID_AAST	MAKE_ID('A','A','S','T')
#define ID_PIXB	MAKE_ID('P','I','X','B')

// Procedural Texture
#define ID_PROC	MAKE_ID('P','R','O','C')
#define ID_COLR	MAKE_ID('C','O','L','R')
#define ID_VALU	MAKE_ID('V','A','L','U')
#define ID_FUNC	MAKE_ID('F','U','N','C')
#define ID_FTPS	MAKE_ID('F','T','P','S')
#define ID_ITPS	MAKE_ID('I','T','P','S')
#define ID_ETPS	MAKE_ID('E','T','P','S')

// Gradient
#define ID_GRAD	MAKE_ID('G','R','A','D')
#define ID_GRST	MAKE_ID('G','R','S','T')
#define ID_GREN	MAKE_ID('G','R','E','N')

// Shader Plugin
#define ID_SHDR	MAKE_ID('S','H','D','R')
#define ID_DATA	MAKE_ID('D','A','T','A')

// Vertex Map type (supported types)
#define ID_WGHT	MAKE_ID('W','G','H','T')
#define ID_TXUV	MAKE_ID('T','X','U','V')
#define ID_RGB	MAKE_ID('R','G','B',' ')
#define ID_RGBA	MAKE_ID('R','G','B','A')
#define ID_MORF MAKE_ID('M','O','R','F')

// POLS chunk type (supported types)
#define ID_FACE	MAKE_ID('F','A','C','E')
#define ID_BONE	MAKE_ID('B','O','N','E')

#define SWAP_2_BYTES(_vals_)  \
  Swap_Bytes ((byte *)(_vals_), 2);

#define SWAP_4_BYTES(_vals_)  \
  Swap_Bytes ((byte *)(_vals_), 4);

// Skip final pad byte, if any (if chunksize is odd)
#define SKIP_PAD_BYTE(_size_)         \
  if (_size_ % 2)                     \
    bytesread += Skip_Bytes (1, in);  

/*___________________
|
| Function Prototypes
|__________________*/

static void Cleanup_Morph_Maps (lwo2_Object *object);
static void Read_LWO2_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error, char *directory_name);
static int  Read_TAGS_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Count_Tags (int size, FILE *in);
static int  Read_LAYR_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Read_PNTS_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Read_VMAP_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Read_POLS_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Count_Polygons (int size, FILE *in);
static int  Read_PTAG_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Read_BBOX_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Read_SURF_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Read_BLOK_Subchunk (int chunksize, FILE *in, lwo2_Object *object, int *error);
static int  Read_BLOK_Header_Subchunk (int chunksize, FILE *in, lwo2_Block *block, int *error);
static int  Read_CLIP_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error, char *directory_name);
static void Convert_NFilename_To_Filename (char *filename, int *error);

static inline int  Skip_Bytes (int size, FILE *in);
static int  Read_U1 (U1 *vals, int num, FILE *in);
static int  Read_U2 (U2 *vals, int num, FILE *in);
static int  Read_U4 (U4 *vals, int num, FILE *in);
static inline int  Read_I1 (I1 *vals, int num, FILE *in);
static inline int  Read_I2 (I2 *vals, int num, FILE *in);
static inline int  Read_F4 (F4 *vals, int num, FILE *in);
static inline int  Read_COL12 (COL12 *col, int num, FILE *in);
static inline int  Read_VEC12 (VEC12 *vec, int num, FILE *in);
static int  Read_GX (GX *gx, FILE *in);
static int  Sizeof_GX (FILE *in);
static int  Read_Name (S0 name, FILE *in);
static int  Read_ID4 (ID4 id, FILE *in);
static inline void Swap_Bytes (byte *vals, int nbytes);

/*___________________
|
| Global variables
|__________________*/

static char debug_str [256];

/*____________________________________________________________________
|
| Function: lwo2_ReadObjectFile
|
| Output: A lwo2 object.
|___________________________________________________________________*/

lwo2_Object *lwo2_ReadObjectFile (char *filename)
{
	int i, error;
  FILE *in;
	ID4 id;
  U4 type, size;
  char *directory_name;
  lwo2_Object *object = NULL;

/*____________________________________________________________________
|
| Extract directory name (if any) from input filename
|___________________________________________________________________*/

  if (filename) {
    // Allocate memory for directory/file names
    directory_name = (char *) calloc (strlen(filename)+1, sizeof(char));
    if (directory_name == NULL)
      gxError ("lwo2_ReadObjectFile(): Error allocating memory for strings");
    else {
      for (i=(int)strlen(filename)-1; i; i--)
        if (filename[i] == '\\')
          break;
      // Directory part found?
      if (i) {
        strcpy (directory_name, filename);
        directory_name[i+1] = 0;
      }

/*____________________________________________________________________
|
| Open input file and start reading data
|___________________________________________________________________*/

      // Open input file
      in = fopen (filename, "rb");
      if (in == NULL) {
        strcpy (debug_str, "lwo2_ReadObjectFile(): error opening input file ");
        if (strlen (filename) < 128)
          strcat (debug_str, filename);
        else
          strcat (debug_str, "?");
        debug_WriteFile (debug_str);
      }
      else {
        // Read in a chunk type and size
        Read_ID4 (id, in);
        Read_U4 (&size, 1, in);
        type = MAKE_ID (id[0], id[1], id[2], id[3]);
        if (type != ID_FORM) 
          gxError ("lwo2_ReadObjectFile(): Error, not an IFF file (missing FORM tag)");
        else {
          // Create an empty LWO2 object
          object = (lwo2_Object *) calloc (1, sizeof(lwo2_Object));
          if (object) {
            // Read LWO2 chunk
            error = FALSE;
            Read_LWO2_Chunk ((int)size, in, object, &error, directory_name);
            // On any error, free memory for object
            if (error) {
              lwo2_FreeObject (object);
              object = NULL;
            }
          }
        }
        // Close input file
        fclose (in);
	    }
      free (directory_name);
    }
  }

/*____________________________________________________________________
|
|	Clean up morph maps
|___________________________________________________________________*/

//	if (object)
		Cleanup_Morph_Maps (object);

// DEBUG CODE
	//if (object) {
	//	lwo2_Layer *layer;
	//	lwo2_VertexMap *map;
	//	for (layer=object->layer_list; layer; layer=layer->next) {
	//		DEBUG_WRITE ("_____LWO2_LAYER_____");
	//		DEBUG_WRITE (layer->name);
	//		int i;
	//		for (map=layer->vmap_list, i=0; map; map=map->next) {
	//			if (map->type == morph_map) {
	//				i++;
	//				DEBUG_WRITE ("Morph Map");
	//				if (map->name)
	//					DEBUG_WRITE (map->name);
	//			}
	//		}
	//	}
	//}

  return (object);
}

/*____________________________________________________________________
|
| Function: Cleanup_Morph_Maps
|
| Input: Called from gx3d_ReadObjectFile()
| Output: Removes morph entries from morph maps that are all zeros.
|		Completely removes morph maps with all entries zeros.
|___________________________________________________________________*/

static void Cleanup_Morph_Maps (lwo2_Object *object)
{
	int i, j;
	bool all_zeros;
  lwo2_Layer *layer;
	lwo2_VertexMap *vmap, **vpp;

/*____________________________________________________________________
|
|	Go through all morph maps and remove any entries with zero values
| (those entries have no effect)
|___________________________________________________________________*/

	// Look at each layer in the object
	for (layer=object->layer_list; layer; layer=layer->next) {
		// Look at each vmap in the layer
		for (vmap=layer->vmap_list; vmap; vmap=vmap->next) {
			// Is this vmap a morph map?
			if (vmap->type == morph_map) {
				// Look at all entries
				for (i=0; i<vmap->num_entries; i++) {
					all_zeros = true;
					// Look at all dimensions of the entry
					for (j=0; j<vmap->dimension; j++) {
						// Is it non-zero?
						if (vmap->value_array[i*vmap->dimension+j] != 0) {
							all_zeros = false;
							break;
						}
					}
					// Was the entry all zeros?
					if (all_zeros) {
						// Remove the entry by copying the last entry up into its place
						vmap->index_array[i] = vmap->index_array[vmap->num_entries-1];
						memcpy ((void *)&(vmap->value_array[i*vmap->dimension]), (void *)&(vmap->value_array[(vmap->num_entries-1)*vmap->dimension]), vmap->dimension * sizeof(float));
						vmap->num_entries--;
						i--;
					}
				}
			}
		}
	}

/*____________________________________________________________________
|
| Delete any morph maps with no entries (this can happen when all the
|	entries were deleted by the above step)
|___________________________________________________________________*/

	// Look at each layer in the object
	for (layer=object->layer_list; layer; layer=layer->next) {
		// Look at each vmap in the layer
		for (vpp=&(layer->vmap_list); *vpp; ) {
			// Is this vmap a morph map?
			if ((*vpp)->type == morph_map) {
				// Does it have zero entries?
				if ((*vpp)->num_entries == 0) {
					// Remove it from the list
					vmap = *vpp;
					*vpp = (*vpp)->next;
					// Free its memory
					if (vmap->name)
						free (vmap->name);
					if (vmap->index_array)
						free (vmap->index_array);
					if (vmap->value_array)
						free (vmap->value_array);
					free (vmap);
				}
				else
					vpp = &(*vpp)->next;
			}
			else
				vpp = &(*vpp)->next;
		}
	}
}

/*____________________________________________________________________
|
| Function: Read_LWO2_Chunk
|
| Input: Called from gx3d_ReadObjectFile()
| Output: Parses a LWO2 chunk.
|
| Description: Parses the following:
|               'LWO2'[ID4], data[CHUNK] *
|___________________________________________________________________*/

static void Read_LWO2_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error, char *directory_name)
{
  U4 type, size;
	ID4 id;
  int bytesread = 0;
 
  bytesread += Read_ID4 (id, in);
  type = MAKE_ID (id[0], id[1], id[2], id[3]);
  if (type != ID_LWO2) 
    gxError ("[LWO2.C] Read_LWO2_Chunk(): Error, not a lightwave object file (missing LWO2 tag)");
  else {
    // Read in each data chunk
    while ((bytesread < chunksize) AND (NOT *error)) {
      // Read a chunk type and size
      bytesread += Read_ID4 (id, in);
      type = MAKE_ID (id[0], id[1], id[2], id[3]);
      bytesread += Read_U4 (&size, 1, in);
      // Process this chunk
      switch (type) {
        case ID_TAGS: bytesread += Read_TAGS_Chunk ((int)size, in, object, error);
                      break;
        case ID_LAYR:	bytesread += Read_LAYR_Chunk ((int)size, in, object, error); 
                      break;
        case ID_PNTS: bytesread += Read_PNTS_Chunk ((int)size, in, object, error); 
                      break;
        case ID_VMAP:	bytesread += Read_VMAP_Chunk ((int)size, in, object, error); 
                      break;
        case ID_POLS:	bytesread += Read_POLS_Chunk ((int)size, in, object, error); 
                      break;
        case ID_PTAG:	bytesread += Read_PTAG_Chunk ((int)size, in, object, error); 
                      break;
        case ID_BBOX:	bytesread += Read_BBOX_Chunk ((int)size, in, object, error); 
                      break;
        case ID_SURF:	bytesread += Read_SURF_Chunk ((int)size, in, object, error); 
                      break;
        case ID_CLIP:	bytesread += Read_CLIP_Chunk ((int)size, in, object, error, directory_name); 
                      break;
        default:      // Skip over an unknown chunk
                  	  bytesread += Skip_Bytes ((int)size, in);
                      SKIP_PAD_BYTE (size)
#ifdef DEBUG
                      strcpy (debug_str, "Read_LWO2_Chunk(): unknown chunk skipped: ");
                      strcat (debug_str, id);
                      gxError (debug_str);
#endif
                      break;
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: Read_TAGS_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a TAGS chunk.  This chunk lists the tag strings that 
|   can be associated with polygons by the PTAG chunk.  Sets error to 
|   true on any error. 
|
| Description: Parses the following:
|               TAGS { tag-string[S0] * }
|
|   This code only allows one TAGS chunk per file.  If a second TAGS
|   chunk is encountered this function will treat that as an error.
|___________________________________________________________________*/

static int Read_TAGS_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  int i;
  S0  name;
	U4  bytesread = 0;

  // Make sure no other tags array has been defined already
  if (object->tags_array) {
    gxError ("Read_TAGS_Chunk(): Error second TAGS chunk encountered");
    *error = TRUE;
  }
  else {
    // Count the number of tags to read
    object->num_tags = Count_Tags (chunksize, in);
    if (object->num_tags <= 0) {
      gxError ("Read_TAGS_Chunk(): Error num_tags is not 1 or more");
      *error = TRUE;
    }
    else {
      // Create an array of tags
      object->tags_array = (char **) calloc (object->num_tags, sizeof(char *));
      if (object->tags_array == NULL) {
        gxError ("Read_TAGS_Chunk(): Error allocating memory for tags array");
        *error = TRUE;
      }
      else {
        // Read each tag from file and save it in tags array
        for (i=0; (i<object->num_tags) AND (NOT *error); i++) {
          // Read the tag string from the file
          bytesread += Read_Name (name, in);
          // Allocate space to store this string
          object->tags_array[i] = (char *) calloc (strlen(name)+1, sizeof(char));
          if (object->tags_array[i] == NULL) {
            gxError ("Read_TAGS_Chunk(): Error allocating memory for tags string");
            *error = TRUE;
          }
          else 
            // Store the string
            strcpy (object->tags_array[i], name);
        }
      }
    }
  }
  
  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_TAGS_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Count_Tags
|
| Input: Called from Read_TAGS_Chunk()
| Output: Returns the number of tags in the next size bytes of file.
|   If not exact size or any other error, returns 0.
|___________________________________________________________________*/

static int Count_Tags (int size, FILE *in)
{
  int bytesread, num_tags;
  S0  name;

  // Count the number of tags in this part of the file
  for (num_tags=0, bytesread=0; bytesread < size; num_tags++) {
    bytesread += Read_Name (name, in);
  }  

  // Reset file pointer to where it was before this function was called
  fseek (in, -bytesread, SEEK_CUR);
  
  // Error checking
  if (bytesread != size)
    num_tags = 0;

  return (num_tags);
}

/*____________________________________________________________________
|
| Function: Read_LAYR_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a LAYR chunk.  Sets error to true on any error. 
|
| Description: Parses the following:
|               LAYR { number[U2], flags[U2], pivot[VEC12], name[S0], parent[U2] ? }
|___________________________________________________________________*/

static int Read_LAYR_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  int         parent_read;
	U2          number, flags, parent;
	VEC12       pivot;
  S0          name;
  lwo2_Layer *layer, **lpp;
	U4          bytesread = 0;

  // Create a new layer node
  layer = (lwo2_Layer *) calloc (1, sizeof(lwo2_Layer));
  if (layer == NULL) {
    gxError ("Read_LAYR_Chunk(): Error allocating memory");
    *error = TRUE;
  }
  else {
    // Attach this layer to object
    for (lpp=&(object->layer_list); *lpp; lpp=&((*lpp)->next));
    *lpp = layer;

    // Read layer number, flags, pivot, name
    bytesread += Read_U2 (&number, 1, in);
    bytesread += Read_U2 (&flags, 1, in);
	  bytesread += Read_VEC12 (&pivot, 1, in);
	  bytesread += Read_Name (name, in);
    // Optionally, read parent
    parent_read = FALSE;
    if ((chunksize-bytesread) == sizeof(U2)) {
		  bytesread += Read_U2 (&parent, 1, in);
      if (parent != 0xFFFF)
        parent_read = TRUE;
    }

    // Put data into layer node, converting from meters to feet where necessary
    layer->number = (int)number;
    layer->hidden = flags & 1;
    layer->pivot.x = pivot[0] * METERS_TO_FEET;
    layer->pivot.y = pivot[1] * METERS_TO_FEET;
    layer->pivot.z = pivot[2] * METERS_TO_FEET;
    if (strlen(name)) {
      layer->name = (char *) calloc (strlen(name)+1, sizeof(char));
      strcpy (layer->name, name);
    }
    if (parent_read) {
      layer->parent = (int *) calloc (1, sizeof(int));
      *(layer->parent) = parent;
    }
  }
  
  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_LAYR_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_PNTS_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a PNTS chunk.  Sets error to true on any error. 
|   Coordinates in points are relative to the pivot point of the layer.
|   
|
| Description: Parses the following:
|               PNTS { point-location[VEC12] * }
|___________________________________________________________________*/

static int Read_PNTS_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  int         i, num_vertices;
	VEC12       point;
  lwo2_Layer *layer;
	U4          bytesread = 0;

  // Set layer to the most recently created layer
  for (layer=object->layer_list; layer->next; layer=layer->next);
  if (layer == NULL) {
    gxError ("Read_PNTS_Chunk(): Error no active layer");
    *error = TRUE;
  }
  else {
    // Compute number of points
    num_vertices = chunksize / 12;
    if (num_vertices <= 0) {
      gxError ("Read_PNTS_Chunk(): Error num_vertices not 1 or more");
      *error = TRUE;
    }
    else {
      // Make sure no other point array has been read for this layer so far
      if (layer->vertex_array) {
        gxError ("Read_PNTS_Chunk(): Error points array already exists");
        *error = TRUE;
      }
      else {
        // Create a new points array
        layer->vertex_array = (lwo2_Point *) calloc (num_vertices, sizeof(lwo2_Point));
        if (layer->vertex_array == NULL) {
          gxError ("Read_PNTS_Chunk(): Error allocating memory for points array");
          *error = TRUE;
        }
        else {
          // Read in points, converting the data from meters to feet
          layer->num_vertices = num_vertices;
          for (i=0; i<num_vertices; i++) {
            bytesread += Read_VEC12 (&point, 1, in);
            layer->vertex_array[i].x = point[0] * METERS_TO_FEET;
            layer->vertex_array[i].y = point[1] * METERS_TO_FEET;
            layer->vertex_array[i].z = point[2] * METERS_TO_FEET;
          }
        }
      }
    }
  }

  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_PNTS_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_VMAP_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a VMAP chunk.  Sets error to true on any error. 
|   0-based vertex indeces in this chunk are relative to the points in 
|   the active (most recently created) layer.
|
| Description: Parses the following:
|               VMAP { type[ID4], dimension[U2], name[S0], 
|                      (vert[GX], value[F4] # dimension) * }
|___________________________________________________________________*/

static int Read_VMAP_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  int i, n, index, supported, num_entries;
  U2  dimension;
  U4  type;
  ID4 id;
  GX  gx;
  S0  name;
  lwo2_Layer *layer;
  lwo2_VertexMap *vmap, *vp, **vpp;
  U4  bytesread = 0;

  // Set layer to the most recently created layer
  for (layer=object->layer_list; layer->next; layer=layer->next);
  if (layer == NULL) {
    gxError ("Read_VMAP_Chunk(): Error no active layer");
    *error = TRUE;
  }
  else {
    // Make sure a point array exists for the layer
    if (layer->vertex_array == NULL) {
      gxError ("Read_VMAP_Chunk(): Error no points array in layer");
      *error = TRUE;
    }
    else {
      // Read in header data about this chunk
      bytesread += Read_ID4 (id, in);
      type = MAKE_ID (id[0], id[1], id[2], id[3]);
	    bytesread += Read_U2 (&dimension, 1, in);
	    bytesread += Read_Name (name, in);
      // Make sure this is a supported type of vertex map
      supported = ((type == ID_WGHT) OR
                   (type == ID_TXUV) OR
                   (type == ID_RGB)  OR
                   (type == ID_RGBA) OR
                   (type == ID_MORF));
      if (dimension <= 0)
        supported = FALSE;
      // If not supported skip the following data
      if (NOT supported) 
   	    bytesread += Skip_Bytes ((int)(chunksize-bytesread), in);
      else {
        // Create a new vertex map node
        vmap = (lwo2_VertexMap *) calloc (1, sizeof(lwo2_VertexMap));
        if (vmap == NULL) {
          gxError ("Read_VMAP_Chunk(): Error allocating memory for vertex map");
          *error = TRUE;
        }
        else {
          // Is this a weight map?
          if (type == ID_WGHT) {
            // Get num of weight maps in linked list of vmaps (so far)
            for (vp=layer->vmap_list, n=0; vp; vp=vp->next)
              if (vp->type == weight_map)
                n++;
            // Set weight map id
            vmap->weight_map_id = n;
          }
          // Attach this vmap to layer
          for (vpp=&(layer->vmap_list); *vpp; vpp=&((*vpp)->next));
          *vpp = vmap;
          
          // Put header data into vmap node
          switch (type) {
            case ID_WGHT: vmap->type = weight_map;
                          break;
            case ID_TXUV: vmap->type = uv_map;
                          break;
            case ID_RGB:  vmap->type = rgb_color_map;   // is this diffuse or specular?  Need to add something here to tell the difference
                          break;
            case ID_RGBA: vmap->type = rgba_color_map;
                          break;
            case ID_MORF: vmap->type = morph_map;
                          break;
          }
          vmap->dimension = (int)dimension;
          if (strlen(name)) {
            vmap->name = (char *) calloc (strlen(name)+1, sizeof(char));
            strcpy (vmap->name, name);
          }
          // Compute number of entries in vmap
//          vmap->num_entries = (chunksize-bytesread) / (Sizeof_GX(in) + dimension * sizeof(F4));
          num_entries = (chunksize-bytesread) / (Sizeof_GX(in) + dimension * sizeof(F4));
          // Number of entries should be the same as number of points in layer if vmap is a color map
          if ((num_entries != layer->num_vertices) AND ((type == ID_RGB) OR (type == ID_RGBA))) {
            sprintf (debug_str, "Read_VMAP_Chunk(): Error number of vmap entries (%d) different from number of points in layer (%d)", vmap->num_entries, layer->num_vertices);
            gxError (debug_str);
            *error = TRUE;
          }
          else {
            // Create new index and value arrays for a uv map (must have same # entries as # vertices in layer)
            if (type == ID_TXUV) {
              vmap->index_array = (int *)   calloc (layer->num_vertices, sizeof(int));
              vmap->value_array = (float *) calloc (layer->num_vertices * dimension, sizeof(float));
            }
            else {
              vmap->index_array = (int *)   calloc (num_entries, sizeof(int));
              vmap->value_array = (float *) calloc (num_entries * dimension, sizeof(float));
            }
            if ((vmap->index_array == NULL) OR (vmap->value_array == NULL)) {
              gxError ("Read_VMAP_Chunk(): Error allocating memory for index and/or value array");
              *error = TRUE;
            }
            else {
              // Does this uv map not have enough data? (need 1 uv coord per vertex)
              if ((type == ID_TXUV) AND (num_entries < layer->num_vertices)) {
                // First set default values for all 
                for (i=0; i<layer->num_vertices; i++) {
                  vmap->index_array[i] = i;
                  for (n=0; n<vmap->dimension; n++) 
                    vmap->value_array[i*vmap->dimension+n] = -1000;
                }
                // Next, read values from file
                for (i=0; i<num_entries; i++) {
                  // Read in a vertex index
                  bytesread += Read_GX (&gx, in);
                  index = (int)gx;
                  // Read in the value/s
                  for (n=0; n<vmap->dimension; n++) 
				            bytesread += Read_F4 (&vmap->value_array[index*vmap->dimension+n], 1, in);
                }
                // Set # entries in the created map
                vmap->num_entries = layer->num_vertices;
              }
              else {
                // Read in index/value's (same number of points as layer, dimension is variable) into vmap
                for (i=0; i<num_entries; i++) {
                  // Read in a vertex index
                  bytesread += Read_GX (&gx, in);
                  vmap->index_array[i] = (int)gx;
                  // Read in the value/s
                  for (n=0; n<vmap->dimension; n++) 
				            bytesread += Read_F4 (&vmap->value_array[i*vmap->dimension+n], 1, in);
                }
                // Set # entries in the created vmap
                vmap->num_entries = num_entries;
              }
            }
          }
        }
      }
    }
  }

  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_VMAP_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_POLS_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a POLS chunk.  Sets error to true on any error. 
|   0-based vertex indeces in this chunk are relative to the points in 
|   the active (most recently created) layer.
|
| Description: Parses the following:
|               POLS { type[ID4], ( numvert+flags[U2], vert[GX] # numvert ) * }
|___________________________________________________________________*/

static int Read_POLS_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  int i, n, num_polygons, supported;
  U2  numvert;
  ID4 id;
  U4  type;
  GX  gx;
  lwo2_Layer *layer;
  U4  bytesread = 0;

  // Set layer to the most recently created layer
  for (layer=object->layer_list; layer->next; layer=layer->next);
  if (layer == NULL) {
    gxError ("Read_POLS_Chunk(): Error no active layer");
    *error = TRUE;
  }
  else {
    // Make sure a vertex array exists for the layer
    if (layer->vertex_array == NULL) {
      gxError ("Read_POLS_Chunk(): Error no vertex array in layer");
      *error = TRUE;
    }
    else {
      // Read in header data about this chunk
      bytesread += Read_ID4 (id, in);
      type = MAKE_ID (id[0], id[1], id[2], id[3]);
      // Make sure this is a supported type of POLS chunk
      supported = ((type == ID_FACE) OR
                   (type == ID_BONE));
      // If not supported skip the following data
      if (NOT supported) 
   	    bytesread += Skip_Bytes ((int)(chunksize-bytesread), in);
      else {
        // Count the number of polygons to follow
        num_polygons = Count_Polygons (chunksize-bytesread, in);
        if (num_polygons <= 0) {
          gxError ("Read_POLS_Chunk(): Error number of polygons less than or equal to zero");
          *error = TRUE;
        }
        else {
          // Make sure no other polygon array has been read for this layer so far
          if (layer->polygon_array) {
            gxError ("Read_POLS_Chunk(): Error poly array already exists");
            *error = TRUE;
          }
          else {
            // If this polygon data represents bones, this is a skeleton layer
            if (type == ID_BONE)
              layer->skeleton = TRUE;
            // Create a new polygon array
            layer->num_polygons = num_polygons;
            layer->polygon_array = (lwo2_Polygon *) calloc (num_polygons, sizeof(lwo2_Polygon));
            if (layer->polygon_array == NULL) {
              gxError ("Read_POLS_Chunk(): Error allocating memory for poly array");
              *error = TRUE;
            }
            // Read polygons into array
            else {
              for (i=0; i<num_polygons; i++) {
                // Read number of vertices making up this polygon
                bytesread += Read_U2 (&numvert, 1, in);
                layer->polygon_array[i].num_vertices = (int)(numvert & 0x03FF);
                // Make sure 1 to 3 vertices or exactly 2 vertices (for bones)
                if ((layer->polygon_array[i].num_vertices < 1) OR (layer->polygon_array[i].num_vertices > 3)) {
                  gxError ("Read_POLS_Chunk(): Error reading a polygon not 1-3 vertices");
                  *error = TRUE;
                }
                else if ((type == ID_BONE) AND (layer->polygon_array[i].num_vertices != 2)) {
                  gxError ("Read_POLS_Chunk(): Error reading a bone not 2 vertices");
                  *error = TRUE;
                }
                else {
                  // Read vertices
                  for (n=0; n<layer->polygon_array[i].num_vertices; n++) {
		  		          bytesread += Read_GX (&gx, in);
			  	          layer->polygon_array[i].index[n] = (int)gx;
                  }
                }
              }  
            }
          }
        }
      }
    }
  }

  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_POLS_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Count_Polygons
|
| Input: Called from Read_POLS_Chunk()
| Output: Returns the number of polygons in the next size bytes of file.
|   If not exact size or any other error, returns 0.
|___________________________________________________________________*/

static int Count_Polygons (int size, FILE *in)
{
  int bytesread, num_polygons;
  U2  numvert;

  // Count the number of polygons in this part of the file
  for (num_polygons=0, bytesread=0; bytesread < size; num_polygons++) {
    // Read number of vertices making up this polygon
    bytesread += Read_U2 (&numvert, 1, in);
    numvert &= 0x03FF;
    // Read vertices
    bytesread += Skip_Bytes ((int)numvert * Sizeof_GX(in), in);
  }  

  // Reset file pointer to where it was before this function was called
  fseek (in, -bytesread, SEEK_CUR);
  
  // Error checking
  if (bytesread != size)
    num_polygons = 0;

  return (num_polygons);
}

/*____________________________________________________________________
|
| Function: Read_PTAG_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a PTAG chunk.  Sets error to true on any error. 
|   0-based polygon indeces in this chunk are relative to the polygons in 
|   the active (most recently created) layer.
|
| Description: Parses the following:
|               PTAG { type[ID4], ( poly[GX], tag[U2] ) * }
|
|   The tag[U2] is an index into the previously created tags array for 
|   the object.  If the tags array hasn't been created yet, this function
|   will error out.
|___________________________________________________________________*/

static int Read_PTAG_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  int i, num_tags, supported;
  U2  tag;
  ID4 id;
  U4  type;
  GX  gx;
  lwo2_Layer *layer;
  lwo2_PolyTag *polytag, **ptpp;
  U4  bytesread = 0;

  // Make sure a TAGS chunk has previously been read
  if (object->tags_array == NULL) {
    gxError ("Read_PTAG_Chunk(): Error corresponding TAGS chunk hasn't been processed yet");
    *error = TRUE;
  }
  else {
    // Set layer to the most recently created layer
    for (layer=object->layer_list; layer->next; layer=layer->next);
    if (layer == NULL) {
      gxError ("Read_PTAG_Chunk(): Error no active layer");
      *error = TRUE;
    }
    else {
      // Read in header data about this chunk
      bytesread += Read_ID4 (id, in);
      type = MAKE_ID (id[0], id[1], id[2], id[3]);
      // Make sure this is a supported type of PTAG chunk
      supported = ((type == ID_SURF) OR
                   (type == ID_BONE) OR
                   (type == ID_BNWT));
      // If not supported skip the following data
      if (NOT supported) 
   	    bytesread += Skip_Bytes ((int)(chunksize-bytesread), in);
      else {
        // Make sure a polygon array exists for this layer?
        if (layer->polygon_array == NULL) {
          gxError ("Read_PTAG_Chunk(): Error no polygon array in layer");
          *error = TRUE;
        }
        else {
          // Count the number of polygon tags to follow
          num_tags = (chunksize-bytesread) / (Sizeof_GX(in) + sizeof(U2));
          if (num_tags <= 0) {
            gxError ("Read_PTAG_Chunk(): Error number of polygon tags less than or equal to zero");
            *error = TRUE;
          }
          // Number of polygon tags must be same as number of polygons in layer
          else if (num_tags != layer->num_polygons) {
            char str[200];
            sprintf (str, "Read_PTAG_Chunk(): Error number of polygon tags (%d) not same as number of polygons (%d)", num_tags, layer->num_polygons);
            gxError (str);
            *error = TRUE;
          }
          else {
            // Create a new polytag node
            polytag = (lwo2_PolyTag *) calloc (1, sizeof(lwo2_PolyTag));
            if (polytag == NULL) {
              gxError ("Read_PTAG_Chunk(): Error allocating memory for polytag node");
              *error = TRUE;
            }
            else {
              // Set the type of this polytag node
              switch (type) {
                case ID_SURF:
                  polytag->type = lwo2_POLYTAGTYPE_SURFACE;
                  break;
                case ID_BONE:
                  polytag->type = lwo2_POLYTAGTYPE_BONE_NAME;
                  break;
                case ID_BNWT:
                  polytag->type = lwo2_POLYTAGTYPE_BONE_WEIGHTMAP;
                  break;
              }
              polytag->polygon_array = (int *) calloc (num_tags, sizeof(int));
              if (polytag->polygon_array == NULL) {
                gxError ("Read_PTAG_Chunk(): Error allocating memory for polygon array");
                *error = TRUE;
              }
              else {
                polytag->tags_index_array = (int *) calloc (num_tags, sizeof(int));
                if (polytag->tags_index_array == NULL) {
                  gxError ("Read_PTAG_Chunk(): Error allocating memory for surface array");
                  *error = TRUE;
                }
                else {
                  // Attach this polytag to layer
                  for (ptpp=&(layer->polytag_list); *ptpp; ptpp=&((*ptpp)->next));
                  *ptpp = polytag;
                  // Read polygon tags into array
                  for (i=0; i<num_tags; i++) {
            		    // Read polygon index (index into polygon array)
                    bytesread += Read_GX (&gx, in);
                    polytag->polygon_array[i] = (int)gx;
                    // Read surface tag (the surface material used by the associated polygon)
            		    bytesread += Read_U2 (&tag, 1, in);
                    polytag->tags_index_array[i] = (int)tag;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_PTAG_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_BBOX_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a BBOX chunk.  Sets error to true on any error. 
|
| Description: Parses the following:
|               BBOX { min[VEC12], max[VEC12] }
|___________________________________________________________________*/

static int Read_BBOX_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  F4          bbox[6];
  lwo2_Layer *layer;
	U4          bytesread = 0;

  // Set layer to the most recently created layer
  for (layer=object->layer_list; layer->next; layer=layer->next);
  if (layer == NULL) {
    gxError ("Read_BBOX_Chunk(): Error no active layer");
    *error = TRUE;
  }
  else {
    // Make sure bound box hasn't already been created for this layer
    if (layer->bound) {
      gxError ("Read_BBOX_Chunk(): Error bound box chunk already read for this layer");
      *error = TRUE;
    }
    else {
      // Create a new bound box struct
      layer->bound = (lwo2_BoundBox *) calloc (1, sizeof(lwo2_BoundBox));
      if (layer->bound == NULL) {
        gxError ("Read_BBOX_Chunk(): Error allocating memory");
        *error = TRUE;
      }
      else {
        // Put data into bound box, converting the data from meters to feet
        bytesread += Read_F4 (bbox, 6, in);
        layer->bound->min.x = bbox[0] * METERS_TO_FEET;
        layer->bound->min.y = bbox[1] * METERS_TO_FEET;
        layer->bound->min.z = bbox[2] * METERS_TO_FEET;
        layer->bound->max.x = bbox[3] * METERS_TO_FEET;
        layer->bound->max.y = bbox[4] * METERS_TO_FEET;
        layer->bound->max.z = bbox[5] * METERS_TO_FEET;
      }
    }
  }
  
  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_BBOX_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_SURF_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a SURF chunk.  Sets error to true on any error. 
|
| Description: Parses the following:
|               SURF { name[S0], source[S0], attributes[SUB-CHUNK] * }
|
|   name uniquely identifies the surface and is the same string that's
|   stored in TAGS and referenced by tag index in PTAG.
|
|   If source name is non-null, this surface is derived from, or composed
|   with the source surface.
|
|   Material attributes follow as a variable list of subchunks.
|___________________________________________________________________*/

static int Read_SURF_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  ID4           id;
  GX            gx;
  COL12         color;
  U4            type;
  U2            size;
  S0            name, source;
  lwo2_Surface *surface, **spp;
	U4            bytesread = 0;

  // Create a new surface node
  surface = (lwo2_Surface *) calloc (1, sizeof(lwo2_Surface));
  if (surface == NULL) {
    gxError ("Read_SURF_Chunk(): Error allocating memory");
    *error = TRUE;
  }
  else {
    // Attach this surface to object
    for (spp=&(object->surface_list); *spp; spp=&((*spp)->next));
    *spp = surface;

    // Read surface name
    bytesread += Read_Name (name, in);
    bytesread += Read_Name (source, in);

    // Allocate memory for name
    surface->name = (char *) calloc (strlen(name)+1, sizeof(char));
    if (surface->name == NULL) {
      gxError ("Read_SURF_Chunk(): Error allocating memory for name");
      *error = TRUE;
    }
    // Allocate memory for source name
    if (strlen(source)) {
      surface->source = (char *) calloc (strlen(source)+1, sizeof(char));
      if (surface->source == NULL) {
        gxError ("Read_SURF_Chunk(): Error allocating memory for source");
        *error = TRUE;
      } 
    }
    if (NOT *error) {
      // Store name, source in surface node
      strcpy (surface->name, name);
      if (surface->source)
        strcpy (surface->source, source);
      
      // Read in each subchunk
      while (((int)bytesread < chunksize) AND (NOT *error)) {
        // Read a chunk type and size
        bytesread += Read_ID4 (id, in);
        type = MAKE_ID (id[0], id[1], id[2], id[3]);
        bytesread += Read_U2 (&size, 1, in);
        // Process this chunk
        switch (type) {
          case ID_COLR: // base color (default = 0,0,0)
                        bytesread += Read_COL12 (&color, 1, in);
                        bytesread += Read_GX (&gx, in);
                        surface->color.r = color[0];
                        surface->color.g = color[1];
                        surface->color.b = color[2];
                        SKIP_PAD_BYTE (size)
                        break;
          case ID_BLOK:	bytesread += Read_BLOK_Subchunk ((int)size, in, object, error); 
                        break;
          default:      // Skip over an unknown subchunk
                  	    bytesread += Skip_Bytes ((int)size, in);
                        SKIP_PAD_BYTE (size)
                        break;
        }
      } 
    }    
  }
  
  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_SURF_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_BLOK_Subchunk
|
| Input: Called from Read_SURF_Chunk()
| Output: Parses a BLOK subchunk.  Sets error to true on any error. 
|   Blocks hold texture layers or shaders.
|
| Description: Parses the following:
|               BLOK { header[SUB-CHUNK], attributes[SUB-CHUNK] * }
|___________________________________________________________________*/

static int Read_BLOK_Subchunk (int chunksize, FILE *in, lwo2_Object *object, int *error)
{
  int           supported;
  ID4           id;
  U4            type;
  U2            size, width_wrap, height_wrap;
  GX            gx;
  S0            name;
  lwo2_Surface *surface;
  lwo2_Block   *block, **bpp;
  U4            bytesread = 0;

  // Set surface to the most recently created surface
  for (surface=object->surface_list; surface->next; surface=surface->next);
  if (surface == NULL) {
    gxError ("Read_BLOK_Subchunk(): Error no active surface");
    *error = TRUE;
  }
  else {
    // Read header subchunk type and size
    bytesread += Read_ID4 (id, in);
    type = MAKE_ID (id[0], id[1], id[2], id[3]);
    bytesread += Read_U2 (&size, 1, in);
    // Make sure this is a supported type of block (read subchunk header ID)
    supported = (type == ID_IMAP);
    // If not supported skip the following data
    if (NOT supported)
   	  bytesread += Skip_Bytes ((int)(chunksize-bytesread), in);
    else {
      // Create a new block node
      block = (lwo2_Block *) calloc (1, sizeof(lwo2_Block));
      if (block == NULL) {
        gxError ("Read_BLOK_Chunk(): Error allocating memory for block");
        *error = TRUE;
      }
      else {
        // Attach this block to surface
        for (bpp=&(surface->block_list); *bpp; bpp=&((*bpp)->next));
        *bpp = block;

        // Set block type
        switch (type) {
          case ID_IMAP: block->type = image_texture;  // only one supported so far (see above code)
                        break;
          case ID_PROC: block->type = procedural_texture;
                        break;
          case ID_GRAD: block->type = gradient_texture;
                        break;
        }
        // Set default texture wraps
        block->width_wrap  = wrap_type_repeat;
        block->height_wrap = wrap_type_repeat;

        // Read in header subchunk for this block
        bytesread += Read_BLOK_Header_Subchunk ((int)size, in, block, error);
        // Read in attributes subchunks
        while (((int)bytesread < chunksize) AND (NOT *error)) {
          // Read a subchunk type and size
          bytesread += Read_ID4 (id, in);
          type = MAKE_ID (id[0], id[1], id[2], id[3]);
          bytesread += Read_U2 (&size, 1, in);
          // Process this subchunk
          switch (type) {
            case ID_IMAG: // image map
                          bytesread += Read_GX (&gx, in);
                          // Make sure no other clip so far
                          if (block->clip_id) {
                            gxError ("Read_BLOK_Subchunk(): Error second IMAG subchunk encountered");
                            *error = TRUE;
                          }
                          else {
                            // Allocate memory for clip
                            block->clip_id = (int *) calloc (1, sizeof(int));
                            if (block->clip_id == NULL) {
                              gxError ("Read_BLOK_Subchunk(): Error allocating memory for clip");
                              *error = TRUE;
                            }
                            else 
                              *(block->clip_id) = (int)gx;
                          }
                          SKIP_PAD_BYTE (size)
                          break;
            case ID_WRAP: // image wrap options
                          bytesread += Read_U2 (&width_wrap, 1, in);
                          bytesread += Read_U2 (&height_wrap, 1, in);
                          block->width_wrap = (lwo2_TextureWrapType)width_wrap;
                          block->height_wrap = (lwo2_TextureWrapType)height_wrap;
                          SKIP_PAD_BYTE (size)
                          break;
            case ID_VMAP: // uv vertex map        
                          bytesread += Read_Name (name, in);
                          // Make sure no other vertex map so far
                          if (block->vertexmap_name) {
                            gxError ("Read_BLOK_Subchunk(): Error second VMAP subchunk encountered");
                            *error = TRUE;
                          }
                          else {
                            // Allocate memory for vertexmap name
                            block->vertexmap_name = (char *) calloc (strlen(name)+1, sizeof(char));
                            if (block->vertexmap_name == NULL) {
                              gxError ("Read_BLOK_Subchunk(): Error allocating memory for vertexmap_name");
                              *error = TRUE;
                            }
                            else 
                              strcpy (block->vertexmap_name, name);
                          }
                          SKIP_PAD_BYTE (size)
                          break;
            default:      // Skip over an unknown subchunk
                  	      bytesread += Skip_Bytes ((int)size, in);
                          SKIP_PAD_BYTE (size)
                          break;
          }
        }
      }
    }
  }

  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_BLOK_Subchunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_BLOK_Header_Subchunk
|
| Input: Called from Read_BLOK_Chunk()
| Output: Parses a BLOK header subchunk.  Sets error to true on any error. 
|
| Description: Parses the following:
|               block-header { ordinal[S0], block-attributes[SUB-CHUNK] * }
|___________________________________________________________________*/

static int Read_BLOK_Header_Subchunk (int chunksize, FILE *in, lwo2_Block *block, int *error)
{
  ID4 id;
  S0  name;
  U4	type;
  GX  gx;
  U2  size, opacity_type;
  F4  opacity_value;
  U4  bytesread = 0;  

  // Read the ordinal name for this subchunk
  bytesread += Read_Name (name, in);
  
  // Read in block-attribute subchunks
  while (((int)bytesread < chunksize) AND (NOT *error)) {
	  bytesread += Read_ID4 (id, in);
	  type = MAKE_ID (id[0],id[1],id[2],id[3]);
	  bytesread += Read_U2 (&size, 1, in);
    // Process this subchunk
    switch (type) {
      case ID_OPAC: // Read data about opacity
                    bytesread += Read_U2 (&opacity_type, 1, in);
	                  bytesread += Read_F4 (&opacity_value, 1, in);
	                  bytesread += Read_GX (&gx, in);
                    // Save opacity type
                    block->opacity_type = (int)opacity_type;
                    SKIP_PAD_BYTE (size)
                    break;
      default:      // Skip over an unknown subchunk
                  	bytesread += Skip_Bytes ((int)size, in);
                    SKIP_PAD_BYTE (size)
                    break;
    }
  }

  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_BLOK_Head_Subchunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_CLIP_Chunk
|
| Input: Called from Read_LWO2_Chunk()
| Output: Parses a CLIP chunk.  Sets error to true on any error. 
|
| Description: Parses the following:
|               CLIP { index[U4], attributes[SUB-CHUNK] * }
|
|   This chunk describes an image or a sequence of images.  Surface
|   images specify images by referring to CLIP chunks.  The index
|   identifies this clip uniquely and may be any non-zero value.  The
|   filename and image processing modifiers follow as a variable list 
|   of subchunks.
|___________________________________________________________________*/

static int Read_CLIP_Chunk (int chunksize, FILE *in, lwo2_Object *object, int *error, char *directory_name)
{
  int        supported;
  ID4        id;
  U4         index, type;
  U2         size;
  S0         name;
  lwo2_Clip *clip, **cpp;
	U4         bytesread = 0;

  // Read unique index number
  bytesread += Read_U4 (&index, 1, in);

  // Read in first subchunk type and size
  bytesread += Read_ID4 (id, in);
  type = MAKE_ID (id[0], id[1], id[2], id[3]);
  bytesread += Read_U2 (&size, 1, in);
  
  // Make sure this is a supported type of clip
  supported = (type == ID_STIL);
  // If not supported skip the following data
  if (NOT supported)
   	bytesread += Skip_Bytes ((int)(chunksize-bytesread), in);
  else {
    // Create a new clip node
    clip = (lwo2_Clip *) calloc (1, sizeof(lwo2_Clip));
    if (clip == NULL) {
      gxError ("Read_CLIP_Chunk(): Error allocating memory for clip node");
      *error = TRUE;
    }
    else {
      // Attach this clip to object
      for (cpp=&(object->clip_list); *cpp; cpp=&((*cpp)->next));
      *cpp = clip;

      // Store id number
      clip->id = (int)index;

      // Read in filename in neutral file format (disk:path/subpath/file)
      bytesread += Read_Name (name, in);
      SKIP_PAD_BYTE (size)
      // Convert to windows filename format
      Convert_NFilename_To_Filename (name, error);  
      // Store filename
      clip->filename = (char *) calloc (strlen(directory_name)+strlen(name)+1, sizeof(char));
      if (clip->filename == NULL) {
        gxError ("Read_CLIP_Chunk(): Error allocating memory for filename");
        *error = TRUE;
      }
      else {
        // Add directory to start of pathname if not an absolute pathname
        if (strchr (name, ':') == 0)
          strcpy (clip->filename, directory_name);
        strcat (clip->filename, name);
      }

      // Read in other subchunks
      while (((int)bytesread < chunksize) AND (NOT *error)) {
        // Read a chunk type and size
        bytesread += Read_ID4 (id, in);
        type = MAKE_ID (id[0], id[1], id[2], id[3]);
        bytesread += Read_U2 (&size, 1, in);
        // Process this chunk
        switch (type) {
          case 1: // dummy case
          default: // Skip over an unknown subchunk
                  bytesread += Skip_Bytes ((int)size, in);
                  SKIP_PAD_BYTE (size)
                  break;
        }    
      }
    }
  }
  
  // Make sure entire chunk was read in
  if ((int)bytesread != chunksize) {
    gxError ("Read_CLIP_Chunk(): Error number of bytes read not same as chunksize");
    *error = TRUE;
  }
  SKIP_PAD_BYTE (chunksize)

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Convert_NFilename_To_Filename
|
| Input: Called from Read_CLIP_Chunk()
| Output: Changes filename from neutral filename format to windows 
|   filename format.
|___________________________________________________________________*/

static void Convert_NFilename_To_Filename (char *filename, int *error)
{
  unsigned i;

  // Size of filename must be 1 less than max in case need to add '\' to drive specifier
  if (strlen(filename) < 254) {
    for (i=0; i<strlen(filename); i++) {
      // Convert ':' to ":\"
      if (filename[i] == ':') {
        strins (filename, i+1, "\\");
        i++;
      }
      // Convert '/' to '\'
      else if (filename[i] == '/')
        filename[i] = '\\';
    }
  }
  else {
    gxError ("Convert_NFilename_To_Filenamne(): filename too long");
    *error = TRUE;
  }
}

/*____________________________________________________________________
|
| Function: Skip_Bytes
|
| Input: Called from ____
| Output: Skips bytes in a file and returns number skipped.
|___________________________________________________________________*/

static inline int Skip_Bytes (int size, FILE *in)
{
  if (size > 0)                 
    fseek (in, (long)size, SEEK_CUR);
  
  return (size);
}  

/*____________________________________________________________________
|
| Function: Read_U1
|
| Input: Called from ____
| Output: Returns # bytes read from file.
|___________________________________________________________________*/

static int Read_U1 (U1 *vals, int num, FILE *in)
{
  int num_items_read, bytesread;

  // Read in the data
  num_items_read = (int) fread ((void *)vals, sizeof(U1), num, in);
  // Compute # bytes read from file
  bytesread = num_items_read * sizeof(U1);

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_U2
|
| Input: Called from ____
| Output: Returns # bytes read from file.
|___________________________________________________________________*/

static int Read_U2 (U2 *vals, int num, FILE *in)
{
  int i, num_items_read, bytesread;

  // Read in the data
  num_items_read = (int) fread ((void *)vals, sizeof(U2), num, in);
  // Compute # bytes read from file
  bytesread = num_items_read * sizeof(U2);
  // Swap from big to little endian order
  for (i=0; i<num_items_read; i++)
    SWAP_2_BYTES (&vals[i])

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_U4
|
| Input: Called from ____
| Output: Returns # bytes read from file.
|___________________________________________________________________*/

static int Read_U4 (U4 *vals, int num, FILE *in)
{
  int i, num_items_read, bytesread;

  // Read in the data
  num_items_read = (int) fread ((void *)vals, sizeof(U4), num, in);
  // Compute # bytes read from file
  bytesread = num_items_read * sizeof(U4);
  // Swap from big to little endian order
  for (i=0; i<num_items_read; i++)
    SWAP_4_BYTES (&vals[i])

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_I1
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static inline int Read_I1 (I1 *vals, int num, FILE *in)
{
	return (Read_U1 ((U1 *)vals, num, in));
}

/*____________________________________________________________________
|
| Function: Read_I2
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static inline int Read_I2 (I2 *vals, int num, FILE *in)
{
	return (Read_U2 ((U2 *)vals, num, in));
}

/*____________________________________________________________________
|
| Function: Read_F4
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static inline int Read_F4 (F4 *vals, int num, FILE *in)
{
	return (Read_U4 ((U4 *)vals, num, in));
}

/*____________________________________________________________________
|
| Function: Read_COL12
|
| Input: Called from ____
| Output: A color as a tripe of floats.  Range is [0.0, 1.0] but values
|   outside this range are possible.
|___________________________________________________________________*/

static inline int Read_COL12 (COL12 *col, int num, FILE *in)
{
	return (Read_F4 ((F4 *)col, 3 * num, in));
}

/*____________________________________________________________________
|
| Function: Read_VEC12
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static inline int Read_VEC12 (VEC12 *vec, int num, FILE *in)
{
	return (Read_F4 ((F4 *)vec, 3 * num, in));
}

/*____________________________________________________________________
|
| Function: Read_GX
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int Read_GX (GX *gx, FILE *in)
{
	int ch, bytesread;

	*gx = 0;
	ch = fgetc (in);
  // 4-byte form of GX?
  if (ch == 0xFF) {
		ch = fgetc (in); 
    *gx |= ch << 16;
		ch = fgetc (in); 
    *gx |= ch << 8;
		ch = fgetc (in); 
    *gx |= ch;
		bytesread = 4;
	} 
  // 2-byte form of GX
  else {
		*gx |= ch << 8;
		ch = fgetc (in); 
    *gx |= ch;
		bytesread = 2;
	}

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Sizeof_GX
|
| Input: Called from ____
| Output: Returns size in bytes (2 or 4) of the GX the filepointer is 
|   currently on.
|___________________________________________________________________*/

static int Sizeof_GX (FILE *in)
{
  int ch, size;

  // Get the next byte from the file which is the first byte of the GX
  ch = fgetc (in);
  // If first byte of the GX is 0xFF this is the 4-byte form
  if (ch == 0xFF)
    size = 4;
  // Else it's the 2 byte form
  else
    size = 2;
  // Reset file pointer
  fseek (in, -1, SEEK_CUR);

  return (size);
}

/*____________________________________________________________________
|
| Function: Read_Name
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int Read_Name (S0 name, FILE *in)
{
	int ch;
  int bytesread = 0;

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
| Function: Read_Type
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int Read_Type (U4 *type, FILE *in)
{
  ID4 id;
  int bytesread;

  bytesread = Read_ID4 (id, in);
  *type = MAKE_ID (id[0], id[1], id[2], id[3]);

  return (bytesread);
}

/*____________________________________________________________________
|
| Function: Read_ID4
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

static int Read_ID4 (ID4 id, FILE *in)
{
	int bytesread = 0;

  id[bytesread++] = fgetc (in);
  id[bytesread++] = fgetc (in);
  id[bytesread++] = fgetc (in);
  id[bytesread++] = fgetc (in);
  id[bytesread  ] = 0;

	return (bytesread);
}

/*____________________________________________________________________
|
| Function: Swap_Bytes
|
| Input: Called from ____
| Output: Swaps bytes from big endian to little endian format.
|___________________________________________________________________*/

static inline void Swap_Bytes (byte *vals, int nbytes)
{
  int i;
  byte temp[4];

  memcpy (temp, vals, nbytes);
  for (i=0; i<nbytes; i++)
    vals[i] = temp[nbytes-1-i];
}

/*____________________________________________________________________
|
| Function: lwo2_WriteObjectFile
|
| Output: Writes out a LWO2 object file.
|___________________________________________________________________*/

void lwo2_WriteObjectFile (char *filename, lwo2_Object *object)
{

}

/*____________________________________________________________________
|
| Function: lwo2_FreeObject
|
| Output: Frees all memory associated with a lwo2 object.
|___________________________________________________________________*/

void lwo2_FreeObject (lwo2_Object *object)
{
  int i;           
  lwo2_Layer      *layer, *lp;
  lwo2_Surface    *surface, *sp;
  lwo2_VertexMap  *vmap, *vp;
  lwo2_PolyTag    *polytag, *ptp;
  lwo2_Block      *block, *bp;
  lwo2_Clip       *clip, *cp;

  if (object) {

    // Free each tag in tags array
    if (object->tags_array) {
      for (i=0; i<object->num_tags; i++)
        if (object->tags_array[i])
          free (object->tags_array[i]);
      free (object->tags_array);
    }

    // Free each surface in list
    for (surface=object->surface_list; surface;) {
      if (surface->name)
        free (surface->name);
      if (surface->source)
        free (surface->source);
      // Free each block in list
      for (block=surface->block_list; block;) {
        if (block->clip_id)
          free (block->clip_id);
        if (block->vertexmap_name)
          free (block->vertexmap_name);
        bp = block->next;
        free (block);
        block = bp;
      }
      // Point to next surface in list
      sp = surface->next;
      free (surface);
      surface = sp;
    }

    // Free each clip in list
    for (clip=object->clip_list; clip;) {
      if (clip->filename)
        free (clip->filename);
      // Point to next clip in list
      cp = clip->next;
      free (clip);
      clip = cp;
    }

    // Free each layer in list
    for (layer=object->layer_list; layer;) {
      // Free each vmap in list
      for (vmap=layer->vmap_list; vmap;) {
        if (vmap->name)
          free (vmap->name);
        if (vmap->index_array)
          free (vmap->index_array);
        if (vmap->value_array)
          free (vmap->value_array);
        // Point to next vmap in list
        vp = vmap->next;
        free (vmap);
        vmap = vp;
      }
      // Free each polytag in list
      for (polytag=layer->polytag_list; polytag;) {
        if (polytag->polygon_array)
          free (polytag->polygon_array);
        if (polytag->tags_index_array)
          free (polytag->tags_index_array);
        // Point to next polytag struct in list
        ptp = polytag->next;
        free (polytag);
        polytag = ptp;
      }
      // Free name
      if (layer->name)
        free (layer->name);
      // Free parent
      if (layer->parent)
        free (layer->parent);
      // Free vertex array
      if (layer->vertex_array)
        free (layer->vertex_array);
      // Free bound
      if (layer->bound)
        free (layer->bound);
      // Free polygon array
      if (layer->polygon_array)
        free (layer->polygon_array);
      // Point to next layer in list
      lp = layer->next;
      free (layer);
      layer = lp;
    }

    // Free the object
    free (object);
  }
}

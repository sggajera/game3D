/*____________________________________________________________________
|
| File: lws.cpp
|
| Description: Reads data from a LWS (Lightwave 6.5) file and builds a
|              lws_ObjectLayer.
|
| Functions:  lws_ReadFile
|             lws_ReadFile
|              Process_Object_Layer
|								Process_ObjectMotion
|							  Is_Root_Bone
|								Parent_Bone_Composite_Rotation
|               Parent_Bone
|               Process_Bone
|                Process_BoneMotion
|                 Ascii_Hex_To_Int
|                 Interpolate_Keys
|                 Skip_Channel
|                 Process_Channel        
|                  Process_Key
|                   Match_Token
|               Process_Metadata
|                Process_MetadataMotion
|							lws_WriteTextFile
|             lws_FreeObjectLayer
|              Free_Metadata
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

#include "dp.h"

#include "lws.h"

/*___________________
|
| Function Prototypes
|__________________*/

static lws_ObjectLayer *Process_Object_Layer (int frames_per_second, string *token, bool read_metadata);
static void						  Process_ObjectMotion (lws_ObjectLayer *olayer);
static bool						  Is_Root_Bone (lws_Bone *bone);
static gx3dMatrix			  Parent_Bone_Composite_Rotation (lws_ObjectLayer *olayer, lws_Bone *bone, gx3dMatrix *bone_matrices);
static gx3dMatrix			  Parent_Bone_Composite_Inverse_Rotation (lws_ObjectLayer *olayer, lws_Bone *bone, gx3dMatrix *bone_inverse_matrices);
static lws_Bone        *Parent_Bone (lws_ObjectLayer *olayer, lws_Bone *bone);
static lws_Bone *Process_Bone (
		int		    frames_per_second, 
		string   *token, 
		int		   *current_bone_num,
		bool		 *root_found );
static void							Process_BoneMotion (lws_BoneMotion *bm, gx3dVector *root_pivot, int frames_per_second);
static int							Ascii_Hex_To_Int (char *str);
static void							Interpolate_Keys (lws_Key *src_keys, int num_src_keys, float time, float *dst_key);
static void							Skip_Channel (int channel_num);
static lws_Channel     *Process_Channel (int channel_num);
static void							Process_Key (lws_Key *key);
static void							Match_Token (char *str);
static lws_Metadata    *Process_Metadata (lws_ObjectLayer *olayer, string *token);
static void             Process_MetadataMotion (lws_Metadata *metadata);
static void             Free_Metadata (lws_Metadata *metadata);

/*___________________
|
| Constants
|__________________*/

#define ROOT_BONE_PARENT_ID -1

/*___________________
|
| Macros
|__________________*/

// Macro to add a bone to linked list of objectlayer bones
static inline void Add_Bone (lws_ObjectLayer *olayer, lws_Bone *bone)
{
  DEBUG_ASSERT (olayer)
  DEBUG_ASSERT (bone)

  if (olayer->bones == 0)                                       
    olayer->bones = bone;                                     
  else {                                                          
    lws_Bone *tbone;                                                  
    for (tbone=olayer->bones; tbone->next; tbone=tbone->next);  
    tbone->next = bone;                                         
  }                                                               
}

// Macro to add a metadata to linked list of objectlayer metadata
static inline void Add_Metadata (lws_ObjectLayer *olayer, lws_Metadata *metadata)
{
  DEBUG_ASSERT (olayer)
  DEBUG_ASSERT (metadata)

  if (olayer->metadata == 0)                                       
    olayer->metadata = metadata;                                     
  else {                                                          
    lws_Metadata *tmetadata;                                                  
    for (tmetadata=olayer->metadata; tmetadata->next; tmetadata=tmetadata->next);  
    tmetadata->next = metadata;                                         
  }                                                               
}

/*___________________
|
| Global variables
|__________________*/

static char debug_str[400];
static ifstream *in;
static int file_frames_per_second;  // fps of LWS file (usually 30), used when reading in Metadata

/*____________________________________________________________________
|
| Function: lws_ReadFile
|
| Output: A lws_ObjectLayer.
|___________________________________________________________________*/

lws_ObjectLayer *lws_ReadFile	(char *filename, int *frames_per_second, bool read_metadata)
{
  string token;
	char *str;
	lws_ObjectLayer *olayer = 0;

  // Verify input params
  DEBUG_ASSERT (filename)
  DEBUG_ASSERT (frames_per_second >= 0)

  // Init variables
  file_frames_per_second = 0;

  // Open input file
  in = new ifstream;
  in->open (filename);
  if (NOT *in)
    TERMINAL_ERROR ("lws_ReadFile(): Can't open input file")
  // Parse the file
  Match_Token ("LWSC");
  Match_Token (""); //Match_Token ("3");   // changed 2/18/2012, newer LWS files have "5" instead of "3" (modified Match_Token() function also, to accept nullstring to match with anything)
  for (;;) {
    *in >> token;
    if (NOT *in)
      break;
    if (token == "FramesPerSecond") {
      *in >> file_frames_per_second;
      // use file fps?
      if (*frames_per_second == 0)
        *frames_per_second = file_frames_per_second;
    }
    else if (token == "LoadObjectLayer") {
      // Make sure file FPS is valid before processing a layer
      if (file_frames_per_second <= 0) {
        char dstr[200];
        sprintf (dstr, "lws_ReadFile(): Error FPS [%d] not found or not valid", file_frames_per_second);
        TERMINAL_ERROR (dstr);
      }
      else
        olayer = Process_Object_Layer (*frames_per_second, &token, read_metadata);
    }
    else 
      in->ignore (1000, '\n'); // skip this line
  }
	// If parsed ok, save name
	if (olayer) {
		str = (char *) malloc (strlen(filename)+1);
		if (str == 0)
			TERMINAL_ERROR ("lws_ReadFile(): Error allocating string memory")
		Extract_Filename_Minus_Extension (filename, str);
    olayer->name = (char *) malloc (strlen(str)+1);
		if (olayer->name == 0)
			TERMINAL_ERROR ("lws_ReadFile(): Error allocating string memory for name")
		strcpy (olayer->name, str);
		free (str);
	}

  delete in;

	return (olayer);
}

/*____________________________________________________________________
|
| Function: lws_ReadFile
|
| Output: A lws_ObjectLayer with only top-level data (no keyframe data).
|___________________________________________________________________*/

lws_ObjectLayer *lws_ReadFile	(char *filename)
{
  string token;
	char *str;
	lws_ObjectLayer *olayer = 0;

  // Verify input params
  DEBUG_ASSERT (filename)

  // Open input file
  in = new ifstream;
  in->open (filename);
  if (NOT *in)
    TERMINAL_ERROR ("lws_ReadFile(): Can't open input file")
  // Parse the file
  Match_Token ("LWSC");
  Match_Token (""); //Match_Token ("3");  // changed 2/18/2012, newer LWS files have "5" instead of "3" (modified Match_Token() function also, to accept nullstring to match with anything)
  for (;;) {
    *in >> token;
    if (NOT *in)             
      break;
    if (token == "LoadObjectLayer")
      olayer = Process_Object_Layer (0, &token, false);   // Modify Process_Object_Layer() to handle fps=0 and ignore keyframe data (don't load it)
    else 
      in->ignore (1000, '\n'); // skip this line
  }
	// If parsed ok, save name
	if (olayer) {
		str = (char *) malloc (strlen(filename)+1);
		if (str == 0)
			TERMINAL_ERROR ("lws_ReadFile(): Error allocating string memory")
		Extract_Filename_Minus_Extension (filename, str);
    olayer->name = (char *) malloc (strlen(str)+1);
		if (olayer->name == 0)
			TERMINAL_ERROR ("lws_ReadFile(): Error allocating string memory for name")
		strcpy (olayer->name, str);
		free (str);
	}

  delete in;

	return (olayer);
}

/*____________________________________________________________________
|
| Function: Process_Object_Layer
|
| Input: Called from lws_ReadFile()
| Output: Reads one object layer from LWS file.
|___________________________________________________________________*/

static lws_ObjectLayer *Process_Object_Layer (int frames_per_second, string *token, bool read_metadata)
{
  int n, current_bone_num;
	bool root_found;
  lws_Bone *bone;
  lws_Metadata *metadata;
  lws_ObjectLayer *olayer = 0;

	DEBUG_ASSERT (token)
                                         
  // Allocate memory for objectlayer
  olayer = (lws_ObjectLayer *) calloc (1, sizeof(lws_ObjectLayer));
  if (olayer == 0)
    TERMINAL_ERROR ("Process_Object_Layer(): Error allocating memory for objectlayer")
  olayer->keys_per_second = frames_per_second;

  // Parse objectlayer
  *in >> olayer->id;
  if (NOT *in)
    TERMINAL_ERROR ("Process_Object_Layer(): Unexpected end of file reading id")
  *in >> *token;
  if (NOT *in)
    TERMINAL_ERROR ("Process_Object_Layer(): Unexpected end of file reading lwo filename")
  olayer->lwo_filename = (char *) malloc (token->size() + 1);
  if (olayer->lwo_filename == 0)
    TERMINAL_ERROR ("Process_Object_Layer(): Error allocating memory for lwo filename")
  strcpy (olayer->lwo_filename, token->c_str());

  // Parse rest of objectlayer
	current_bone_num = 0;
	root_found = false;
  *in >> *token;
  for (;;) {
    if (NOT *in)
      break;
	  else if (*token == "ObjectMotion") {
      Process_ObjectMotion (olayer);
      *in >> *token;
    }
    else if (*token == "AddBone") {
			bone = Process_Bone (frames_per_second, token, &current_bone_num, &root_found);
      Add_Bone (olayer, bone);
    }
    else if (*token == "AddNullObject") {
//DEBUG_WRITE ("Process_Object_Layer(): Found AddNullObject **************************************");
      if (read_metadata) {
//DEBUG_WRITE ("Process_Object_Layer(): calling Process_Metadata()....................");
        metadata = Process_Metadata (olayer, token);
        Add_Metadata (olayer, metadata);
      }
      else
        break;
    }
    else if (*token == "AddLight")
      break;
    else if (*token == "AddCamera")
      break;
    else
      *in >> *token;
  }

  // Count # of bones
  for (n=0, bone=olayer->bones; bone; n++, bone=bone->next);
  olayer->num_bones = n;

  // Compute max nkeys of any active bone motion
  olayer->max_nkeys = 0;
  for (bone=olayer->bones; bone; bone=bone->next)
    if (bone->active) 
      if (bone->motion.nkeys > olayer->max_nkeys)
        olayer->max_nkeys = bone->motion.nkeys;

	if (olayer->bones) {
		gx3dMatrix m, m1, m2, m_parent, mx, my, mz, *bone_matrices, *bone_inverse_matrices;
		gx3dVector v;
		lws_Bone *pbone;
		// Allocate temp arrays
		bone_matrices = (gx3dMatrix *) malloc (olayer->num_bones * sizeof(gx3dMatrix));
		if (bone_matrices == 0)
	    TERMINAL_ERROR ("Process_Object_Layer(): Can't allocate memory for an array of matrices")
		bone_inverse_matrices = (gx3dMatrix *) malloc (olayer->num_bones * sizeof(gx3dMatrix));
		if (bone_inverse_matrices == 0)
	    TERMINAL_ERROR ("Process_Object_Layer(): Can't allocate memory for an array of inverse matrices")
		// Compute local rotation matrix at each bone			
		for (bone=olayer->bones; bone; bone=bone->next) {
      gx3d_GetRotateXMatrix (&mx, bone->rotation.x);
      gx3d_GetRotateYMatrix (&my, bone->rotation.y);
      gx3d_GetRotateZMatrix (&mz, bone->rotation.z);
			gx3d_MultiplyMatrix (&mz, &mx, &m);
			gx3d_MultiplyMatrix (&m, &my, &bone_matrices[bone->id]);
		}
		// Compute local inverse rotation matrix at each bone
		for (bone=olayer->bones; bone; bone=bone->next) {
      gx3d_GetRotateXMatrix (&mx, -bone->rotation.x);
      gx3d_GetRotateYMatrix (&my, -bone->rotation.y);
      gx3d_GetRotateZMatrix (&mz, -bone->rotation.z);
			gx3d_MultiplyMatrix (&my, &mx, &m);
			gx3d_MultiplyMatrix (&m, &mz, &bone_inverse_matrices[bone->id]);
		}
  	// Compute bone normals
		for (bone=olayer->bones; bone; bone=bone->next) {
			// Compute composite parent rotation matrix for this bone
			m_parent = Parent_Bone_Composite_Rotation (olayer, bone, bone_matrices);
			// local * parent = composite local
			gx3d_MultiplyMatrix (&bone_matrices[bone->id], &m_parent, &m);
			// Compute direction this bone is pointing
			v.x = 0;
			v.y = 0;
			v.z = 1;
			gx3d_MultiplyNormalVectorMatrix (&v, &m, &(bone->normal));
		}
		// Compute bone pivots (relies on bones being in order starting with root)
		for (bone=olayer->bones; bone; bone=bone->next) 
			if (NOT Is_Root_Bone(bone)) {
        // Get parent bone
			  pbone = Parent_Bone (olayer, bone);
			  // Compute bone pivot = parent bone pivot + parent_normal * bone restposition.z
				gx3d_MultiplyScalarVector (bone->pivot.z, (gx3dVector *)&(pbone->normal), (gx3dVector *)&(bone->pivot));
				gx3d_AddVector ((gx3dVector *)&(bone->pivot), (gx3dVector *)&(pbone->pivot), (gx3dVector *)&(bone->pivot));
		  } 
		// Compute pre/post matrices
		for (bone=olayer->bones; bone; bone=bone->next) {
      // Compute pre
      gx3d_GetTranslateMatrix (&m1, -bone->pivot.x, -bone->pivot.y, -bone->pivot.z);
      m2 = Parent_Bone_Composite_Inverse_Rotation	(olayer, bone, bone_inverse_matrices);
			gx3d_MultiplyMatrix (&m1, &m2, &m);
      gx3d_MultiplyMatrix (&m, &bone_inverse_matrices[bone->id], &(bone->pre));
      // Compute post
      m1 = Parent_Bone_Composite_Rotation (olayer, bone, bone_matrices);
      gx3d_GetTranslateMatrix (&m2, bone->pivot.x, bone->pivot.y, bone->pivot.z);
      gx3d_MultiplyMatrix (&m1, &m2, &(bone->post));
		}
    // Free memory
		free (bone_matrices);
		free (bone_inverse_matrices);
	}

  // Make sure some bones were found
  if (NOT olayer->bones)
    TERMINAL_ERROR ("Process_Object_Layer(): Error no bones found")
	// Make sure root bone was found
	if (NOT root_found)
		TERMINAL_ERROR ("Process_Object_Layer(): Error no root bone found")

  return (olayer);
}

/*____________________________________________________________________
|
| Function: Process_ObjectMotion
|
| Input: Called from Process_Object_Layer(), ...
| Output: Parses the file for object motion data.
|___________________________________________________________________*/

static void Process_ObjectMotion (lws_ObjectLayer *olayer)
{
  int i, num_channels;
  lws_Channel *pos[3] = {0,0,0};
  lws_Channel *rot[3] = {0,0,0};

	DEBUG_ASSERT (olayer)

  // Parse the bone motion
  Match_Token ("NumChannels");
  *in >> num_channels;
  if (NOT *in)
    TERMINAL_ERROR ("Process_ObjectMotion(): number of channels not found")
  else if (num_channels != 9)
    TERMINAL_ERROR ("Process_ObjectMotion(): number of channels != 9")
 
  // Process xyz position data
  for (i=0; i<3; i++) {
    pos[i] = Process_Channel (i);
    if (pos[i] == 0)
      TERMINAL_ERROR ("Process_ObjectMotion(): pos channel not processed")
  }																													
  // Process yxz rotation keys
  for (i=0; i<3; i++) {
    rot[i] = Process_Channel (3+i);
    if (rot[i] == 0)
      TERMINAL_ERROR ("Process_ObjectMotion(): rot channel not processed")
  }
  // Skip last 3 channels
  for (i=0; i<3; i++)
    Skip_Channel (6+i);

	// Get starting position of object
	olayer->position.x = pos[0]->keys[0].value;
	olayer->position.y = pos[1]->keys[0].value;
	olayer->position.z = pos[2]->keys[0].value;
	// Get starting rotation of object
	olayer->rotation.y = rot[0]->keys[0].value;
	olayer->rotation.x = rot[1]->keys[0].value;
	olayer->rotation.z = rot[2]->keys[0].value;

  // Free memory 
  for (i=0; i<3; i++) {
    free (pos[i]->keys);
    free (pos[i]);
    free (rot[i]->keys);
    free (rot[i]);
  }
}

/*____________________________________________________________________
|
| Function: Is_Root_Bone
|
| Input: Called from Process_Object_Layer(), ...
| Output: Returns true if this is the root bone
|___________________________________________________________________*/

static bool Is_Root_Bone (lws_Bone *bone)
{
	return (bone->parent_id == ROOT_BONE_PARENT_ID);
}
 
/*____________________________________________________________________
|
| Function: Parent_Bone_Composite_Rotation
|
| Input: Called from Process_Object_Layer()
| Output: Returns parent bone composite rotaton matrix.
|___________________________________________________________________*/

static gx3dMatrix Parent_Bone_Composite_Rotation (lws_ObjectLayer *olayer, lws_Bone *bone, gx3dMatrix *bone_matrices)
{
	gx3dMatrix m, mp;

	DEBUG_ASSERT (olayer)
	DEBUG_ASSERT (bone)
  DEBUG_ASSERT (bone_matrices)

	if (Is_Root_Bone(bone))
    gx3d_GetIdentityMatrix (&m);
	else {
	  mp = Parent_Bone_Composite_Rotation (olayer, Parent_Bone(olayer, bone), bone_matrices);
		gx3d_MultiplyMatrix (&bone_matrices[bone->parent_id], &mp, &m);
	}
	return (m);
}

/*____________________________________________________________________
|
| Function: Parent_Bone_Composite_Inverse_Rotation
|
| Input: Called from Process_Object_Layer()
| Output: Returns parent bone composite rotaton matrix.
|___________________________________________________________________*/

static gx3dMatrix Parent_Bone_Composite_Inverse_Rotation (lws_ObjectLayer *olayer, lws_Bone *bone, gx3dMatrix *bone_inverse_matrices)
{
	gx3dMatrix m, mp;

	DEBUG_ASSERT (olayer)
	DEBUG_ASSERT (bone)
  DEBUG_ASSERT (bone_inverse_matrices)

	if (Is_Root_Bone(bone))
    gx3d_GetIdentityMatrix (&m);
	else {
	  mp = Parent_Bone_Composite_Inverse_Rotation (olayer, Parent_Bone(olayer, bone), bone_inverse_matrices);
		gx3d_MultiplyMatrix (&mp, &bone_inverse_matrices[bone->parent_id], &m);
	}
	return (m);
}

/*____________________________________________________________________
|
| Function: Parent_Bone
|
| Input: Called from Process_Object_Layer(), Parent_Bone_Composite_Rotation()
| Output: Returns parent bone of a bone.
|___________________________________________________________________*/

static lws_Bone *Parent_Bone (lws_ObjectLayer *olayer, lws_Bone *bone)
{
  lws_Bone *tbone, *parent_bone = 0;

	DEBUG_ASSERT (olayer)
	DEBUG_ASSERT (bone)

  for (tbone=olayer->bones; tbone; tbone=tbone->next)
    if (tbone->id == bone->parent_id) {
      parent_bone = tbone;
      break;
    }

  return (parent_bone);
}

/*____________________________________________________________________
|
| Function: Process_Bone
|
| Input: Called from Process_Object_Layer()
| Output: Reads one bone from LWS file.
|___________________________________________________________________*/

// Required bone parts the parser looks for
#define PART_NAME           0x1
#define PART_PIVOT          0x2
#define PART_DIRECTION      0x4
#define PART_LENGTH					0x8
#define PART_WEIGHTMAPNAME  0x10
#define PART_BONEACTIVE     0x20
#define PART_MOTION         0x40
#define PART_PARENT					0x80
#define ALL_PARTS (PART_NAME | PART_PIVOT | PART_DIRECTION | PART_LENGTH | PART_WEIGHTMAPNAME | PART_BONEACTIVE | PART_MOTION | PART_PARENT)
  
static lws_Bone *Process_Bone (
  int		    frames_per_second,  // 0=don't read in any keyframes 
	string   *token, 
	int		   *current_bone_num,
	bool		 *root_found )
{
  int n;
	char str[5];
	unsigned parts_found = 0;
  lws_BoneMotion *bonemotion = 0;
  lws_Bone *bone = 0;

	DEBUG_ASSERT (current_bone_num)
	DEBUG_ASSERT (root_found)

  // Allocate memory for bone
  bone = (lws_Bone *) calloc (1, sizeof(lws_Bone));
  if (bone == 0)
    TERMINAL_ERROR ("Process_Bone(): Error allocating memory for bone")
  bone->parent_id = ROOT_BONE_PARENT_ID; // assume root bone

  // Parse the bone  
  *in >> *token;
  for (;;) {
    if (NOT *in)
      break;
    else if (*token == "AddBone")
      break;
    else if (*token == "AddNullObject")
      break;
    else if (*token == "AddLight")
      break;
    else if (*token == "AddCamera")
      break;
    // Look for required bone parts
    else if (*token == "BoneName") {
      *in >> *token;
       if (NOT *in)
        TERMINAL_ERROR ("Process_Bone(): Unexpected end of file reading BoneName")
      bone->name = (char *) malloc (token->size() + 1);
      if (bone->name == 0)
        TERMINAL_ERROR ("Process_Bone(): Error allocating memory for bone name")
      strcpy (bone->name, token->c_str());
      parts_found |= PART_NAME;
      *in >> *token;
    }
    else if (*token == "BoneRestPosition") {
      // Pivot in LWS file is in meters
      *in >> bone->pivot.x >> bone->pivot.y >> bone->pivot.z;
      bone->pivot.x *= METERS_TO_FEET;
      bone->pivot.y *= METERS_TO_FEET;
      bone->pivot.z *= METERS_TO_FEET;
      if (NOT *in)
        TERMINAL_ERROR ("Process_Bone(): Unexpected end of file reading BoneRestPosition")
      parts_found |= PART_PIVOT;
      *in >> *token;
    }
    else if (*token == "BoneRestDirection") {
      // Direction in LWS file is in degrees
      *in >> bone->rotation.y >> bone->rotation.x >> bone->rotation.z;
      if (NOT *in)
        TERMINAL_ERROR ("Process_Bone(): Unexpected end of file reading BoneRestDirection")
      parts_found |= PART_DIRECTION;
      *in >> *token;
    }
    else if (*token == "BoneRestLength") {
      // Length in LWS file is in meters
      *in >> bone->length;
			bone->length *= METERS_TO_FEET;
      if (NOT *in)
        TERMINAL_ERROR ("Process_Bone(): Unexpected end of file reading BoneRestLength")
      parts_found |= PART_LENGTH;
      *in >> *token;
    }
    else if (*token == "BoneWeightMapName") {
      *in >> *token;
      if (NOT *in)
        TERMINAL_ERROR ("Process_Bone(): Unexpected end of file reading BoneWeightMapName")
      bone->weightmap_name = (char *) malloc (token->size() + 1);
      if (bone->weightmap_name == 0)
        TERMINAL_ERROR ("Process_Bone(): Error allocating memory for weightmap_name")
      strcpy (bone->weightmap_name, token->c_str());
      parts_found |= PART_WEIGHTMAPNAME;
      *in >> *token;
    }
    else if (*token == "BoneActive") {
      *in >> bone->active;
      if (NOT *in)
        TERMINAL_ERROR ("Process_Bone(): Unexpected end of file reading BoneActive")
      parts_found |= PART_BONEACTIVE;
      *in >> *token;
    }
    else if (*token == "BoneMotion") {
      // Read in keyframe data?
      if (frames_per_second) {
        if (Is_Root_Bone(bone))
          Process_BoneMotion (&bone->motion, &bone->pivot, frames_per_second);
        else
          Process_BoneMotion (&bone->motion, 0, frames_per_second);
      }
      parts_found |= PART_MOTION;
      *in >> *token;
    }
		else if (*token == "ParentItem") {
			*in >> *token;
			if (NOT *in)
				TERMINAL_ERROR ("Process_Bone(): Unexpected end of file reading ParentItem")
			// Decode parent type
			str[0] = (*token)[0];
			str[1] = 0;
			// Is parent a bone?
			n = Ascii_Hex_To_Int (str);
			if (n == 4) { // bone?
				// Decode parent bone #
				str[0] = (*token)[1];
				str[1] = (*token)[2];
				str[2] = (*token)[3];
				str[3] = 0;
				bone->parent_id = Ascii_Hex_To_Int (str);
        parts_found |= PART_PARENT;
			}
			else if (n == 1) { // object?
				// Has root bone already been found? (can only have 1 root bone)
				if (*root_found) 
					TERMINAL_ERROR ("Process_Bone(): Found a second root bone")
				bone->parent_id = ROOT_BONE_PARENT_ID;
				*root_found = true;
				parts_found |= PART_PARENT;
			}
		}		
		else
      *in >> *token;
  }

	// Non-root bone?
	if (NOT Is_Root_Bone(bone)) {
		if (bone->motion.pos)	{
			free (bone->motion.pos);
			bone->motion.pos = 0;
		}
	}

  // Inactive bone?
  if ((parts_found & PART_BONEACTIVE) AND (bone->active == 0)) {
    bone->id = *current_bone_num;
    (*current_bone_num)++;
  }
  // Active bone and all parts found?
  else if (parts_found == ALL_PARTS) {
    bone->id = *current_bone_num;
    (*current_bone_num)++;
  }
  else {
    if (NOT (parts_found & PART_NAME))
      DEBUG_WRITE ("Process_Bone(): bone missing name")
    if (NOT (parts_found & PART_PIVOT))
      DEBUG_WRITE ("Process_Bone(): bone missing pivot")
    if (NOT (parts_found & PART_DIRECTION))
      DEBUG_WRITE ("Process_Bone(): bone missing direction")
	  if (NOT (parts_found & PART_LENGTH))
			DEBUG_WRITE ("Process_Bone(): bone missing length")
  	if (NOT (parts_found & PART_WEIGHTMAPNAME))
      DEBUG_WRITE ("Process_Bone(): bone missing weightmap name")
    if (NOT (parts_found & PART_BONEACTIVE))
      DEBUG_WRITE ("Process_Bone(): bone missing boneactive")
    if (NOT (parts_found & PART_MOTION))
      DEBUG_WRITE ("Process_Bone(): bone missing motion")
		if (NOT (parts_found & PART_PARENT))
			DEBUG_WRITE ("Process_Bone(): bone missing parent")
    TERMINAL_ERROR ("Process_Bone(): Error missing bone parts")
  }

  return (bone);
}
    
/*____________________________________________________________________
|
| Function: Process_BoneMotion
|
| Input: Called from Process_Bone()
| Output: Reads one bone motion (including parent_id) from LWS file.
|___________________________________________________________________*/

static void Process_BoneMotion (lws_BoneMotion *bm, gx3dVector *root_pivot, int frames_per_second)
{
  string token;
  int i, n, num_channels;
  float t, t1, t2, t3, timestep;
  lws_Channel *pos[3] = {0,0,0};
  lws_Channel *rot[3] = {0,0,0};

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (bm)
	DEBUG_ASSERT (frames_per_second >= 1)

/*____________________________________________________________________
|
| Parse the bone motion
|___________________________________________________________________*/

  Match_Token ("NumChannels");
  *in >> num_channels;
  if (NOT *in)
    TERMINAL_ERROR ("Process_BoneMotion(): number of channels not found")
  else if (num_channels != 9)
    TERMINAL_ERROR ("Process_BoneMotion(): number of channels != 9")
 
  // Parse xyz position data
  for (i=0; i<3; i++) {
    pos[i] = Process_Channel (i);
    if (pos[i] == 0)
      TERMINAL_ERROR ("Process_BoneMotion(): pos channel not processed")
  }
  // Parse yxz rotation keys
  for (i=0; i<3; i++) {
    rot[i] = Process_Channel (3+i);
    if (rot[i] == 0)
      TERMINAL_ERROR ("Process_BoneMotion(): rot channel not processed")
  }
  // Skip last 3 channels
  for (i=0; i<3; i++)
    Skip_Channel (6+i);

/*____________________________________________________________________
|
| Process position data
|___________________________________________________________________*/

  // Compute latest ending time of key data (3 key channels may end at different times)
  t1 = pos[0]->keys[pos[0]->nkeys-1].time;
  t2 = pos[1]->keys[pos[1]->nkeys-1].time;
  t3 = pos[2]->keys[pos[2]->nkeys-1].time;
  t = t1;
  if (t2 > t)
    t = t2;
  if (t3 > t)
    t = t3;
  // Compute # keys to generate in output animation data - this equation should be correct but may need more testing
  n = (int)(frames_per_second * t + 0.1f) + 1;  // round up if within 0.1 of next frame time, add one to account for t=0 frame
  bm->nkeys = n;
  // Allocate memory for output key data
  bm->pos = (gx3dVector *) calloc (bm->nkeys, sizeof(gx3dVector));
  if (bm->pos == 0)
    TERMINAL_ERROR ("Process_BoneMotion(): Error allocating memory for bone motion pos key array")
  // Compute output key values
  timestep = 1 / (float)frames_per_second;
  for (i=0,t=0; i<n; i++,t+=timestep) {
    Interpolate_Keys (pos[0]->keys, pos[0]->nkeys, t, &(bm->pos[i].x));
    Interpolate_Keys (pos[1]->keys, pos[1]->nkeys, t, &(bm->pos[i].y));
    Interpolate_Keys (pos[2]->keys, pos[2]->nkeys, t, &(bm->pos[i].z));
    // Convert this translation from object space to bone space (root bone only)
    if (root_pivot)    
      gx3d_SubtractVector (&(bm->pos[i]), root_pivot, &(bm->pos[i]));     // Added 3/24/2012
  }                                     

/*____________________________________________________________________
|
| Process rotation data
|___________________________________________________________________*/

  // Compute latest ending time of key data (3 key channels may end at different times)
  t1 = rot[0]->keys[rot[0]->nkeys-1].time;
  t2 = rot[1]->keys[rot[1]->nkeys-1].time;
  t3 = rot[2]->keys[rot[2]->nkeys-1].time;
  t = t1;
  if (t2 > t)
    t = t2;
  if (t3 > t)
    t = t3;
  // Compute # keys to generate in output animation data - this equation should be correct but may need more testing
  n = (int)(frames_per_second * t + 0.1f) + 1;  // round up if within 0.1 of next frame time, add one to account for t=0 frame
  bm->nkeys = n;
  // Allocate memory for output key data
  bm->rot = (gx3dVector *) calloc (bm->nkeys, sizeof(gx3dVector));
  if (bm->rot == 0)
    TERMINAL_ERROR ("Process_BoneMotion(): Error allocating memory for bone motion rot key array")
  // Compute output key values
  timestep = 1 / (float)frames_per_second;
  for (i=0,t=0; i<n; i++,t+=timestep) {
    Interpolate_Keys (rot[0]->keys, rot[0]->nkeys, t, &(bm->rot[i].y));     // do this with quaternions instead for better quality?
    Interpolate_Keys (rot[1]->keys, rot[1]->nkeys, t, &(bm->rot[i].x));
    Interpolate_Keys (rot[2]->keys, rot[2]->nkeys, t, &(bm->rot[i].z));
  }

/*____________________________________________________________________
|
| Free data
|___________________________________________________________________*/

  for (i=0; i<3; i++) {
    free (pos[i]->keys);
    free (pos[i]);
    free (rot[i]->keys);
    free (rot[i]);
  }
}

/*____________________________________________________________________
|
| Function: Ascii_Hex_To_Int
|
| Input: Called from Process_BoneMotion()
| Output: Converts an ASCII hex string to integer.
|___________________________________________________________________*/

static int Ascii_Hex_To_Int (char *str)
{
  int i, base, value = 0;

  DEBUG_ASSERT (str)

  // Make sure string is all uppercase
  for (i=0; i<(int)strlen(str); i++)
    str[i] = toupper (str[i]);

  // Convert to int
  for (i=(int)strlen(str)-1, base=1; i>=0; i--, base*=16)
    if ((str[i] >= '0') AND (str[i] <= '9'))
      value += base * (str[i] - '0');
    else if ((str[i] >= 'A') AND (str[i] <= 'F'))
      value += base * (str[i] - 'A' + 10);
    else
      TERMINAL_ERROR ("Ascii_Hex_To_Int(): Non-hexadecimal digit found")

  return (value);
}

/*____________________________________________________________________
|
| Function: Interpolate_Keys
|
| Input: Called from Process_BoneMotion()
| Output: Computes a new key value at a certain time using an array of keys.
|___________________________________________________________________*/

static void Interpolate_Keys (lws_Key *src_keys, int num_src_keys, float time, float *dst_key)
{
  int i, j;

	DEBUG_ASSERT (src_keys)
	DEBUG_ASSERT (num_src_keys >= 1)
	DEBUG_ASSERT (time >= 0)
	DEBUG_ASSERT (dst_key)

  // If only 1 source key, use it
  if (num_src_keys == 1)
    *dst_key = src_keys[0].value;
  // If time = 0, use first source key
  else if (time == 0)
    *dst_key = src_keys[0].value;
  // If sample time > time of last source key, use last source key
  else if (time > src_keys[num_src_keys-1].time)
    *dst_key = src_keys[num_src_keys-1].value;
  // Otherwise, time is between 2 source keys, find the two and interpolate between them
  else {
    for (i=0; (i < num_src_keys-1) AND (time > src_keys[i+1].time); i++);
    for (j=i; (j < num_src_keys-1) AND (time > src_keys[j+1].time); j++);
    j++; // adjust to next key after time (this also prevents a divide by zero, below)
    *dst_key = gx3d_Lerp (src_keys[i].value,  // start value
                          src_keys[j].value,  // end value
                          (time - src_keys[i].time) / (src_keys[j].time - src_keys[i].time));  // 0-1
  }
}

/*____________________________________________________________________
|
| Function: Skip_Channel
|
| Input: Called from Process_ObjectMotion(), Process_BoneMotion()
| Output: Skips one channel of bone motion data.
|___________________________________________________________________*/

static void Skip_Channel (int channel_num)
{
  int n;
  string token;

  DEBUG_ASSERT ((channel_num >= 0) AND (channel_num <= 8))

  Match_Token ("Channel");
  *in >> n;
  if (NOT *in)
    TERMINAL_ERROR ("Skip_Channel(): channel number not found")
  if (n != channel_num)
    TERMINAL_ERROR ("Skip_Channel(): wrong channel number")
  Match_Token ("{");
  for (;;) {
    *in >> token;
    if (NOT *in)
      TERMINAL_ERROR ("Skip_Channel(): unexpected end of file")
    if (token == "}")
      break;
  }
}

/*____________________________________________________________________
|
| Function: Process_Channel
|
| Input: Called from Process_ObjectMotion(), Process_BoneMotion()
| Output: Processes one channel of bone motion data.
|___________________________________________________________________*/

static lws_Channel *Process_Channel (int channel_num)
{
  int i, n;
  lws_Channel *channel = 0;

  DEBUG_ASSERT ((channel_num >= 0) AND (channel_num <= 5))

  // Allocate memory for channel
  channel = (lws_Channel *) calloc (1, sizeof(lws_Channel));
  if (channel == 0)
    TERMINAL_ERROR ("Process_Channel(): Error allocating memory for channel")

  // Parse start of channel
  Match_Token ("Channel");
  *in >> n;
  if (NOT *in)
    TERMINAL_ERROR ("Process_Channel(): channel number not found")
  if (n != channel_num)
    TERMINAL_ERROR ("Process_Channel(): wrong channel number")
  Match_Token ("{");
  Match_Token ("Envelope");
  *in >> channel->nkeys;
// Added 3/3/12, found an LWS with "Envelope 0" that had 1 key after it
//  So, apparently, 0 really means 1
  if (channel->nkeys == 0)
    channel->nkeys++;
/////////////////////////////
  if (NOT *in)
    TERMINAL_ERROR ("Process_Channel(): nkeys not found")

  // Allocate memory for keys
  channel->keys = (lws_Key *) malloc (channel->nkeys * sizeof(lws_Key));
  if (channel->keys == 0)
    TERMINAL_ERROR ("Process_Channel(): Error allocating memory for keys")

  // Parse rest of channel
  for (i=0; i<channel->nkeys; i++) {
    Process_Key (&(channel->keys[i]));
    switch (channel_num) {
      case 0:
      case 1:
      case 2: // position in LWS file is in meters
              channel->keys[i].value *= METERS_TO_FEET;
              break;
      case 3:
      case 4:
      case 5: // rotation is in LWS file in radians
              channel->keys[i].value *= RADIANS_TO_DEGREES;
              break;
    }
  }

  Match_Token ("Behaviors");
  in->ignore (1000, '\n');
  if (NOT *in)
    TERMINAL_ERROR ("Process_Channel(): unexpected end of file")
  Match_Token ("}");

  return (channel);
}

/*____________________________________________________________________
|
| Function: Process_Key
|
| Input: Called from Process_Channel()
| Output: Reads one key from file.
|___________________________________________________________________*/

#define SKIP_TOKEN                                            \
{                                                             \
  *in >> f;                                                   \
  if (NOT *in)                                                \
    TERMINAL_ERROR ("Process_Key(): unexpected end of file")  \
}

static void Process_Key (lws_Key *key)
{
  float f;

	DEBUG_ASSERT (key)

  Match_Token ("Key");
  *in >> key->value;
  *in >> key->time;
  if (NOT *in)
    TERMINAL_ERROR ("Process_Key(): unexpected end of file")  
  SKIP_TOKEN
  SKIP_TOKEN
  SKIP_TOKEN
  SKIP_TOKEN
  SKIP_TOKEN
  SKIP_TOKEN
  SKIP_TOKEN
}

/*____________________________________________________________________
|
| Function: Match_Token
|
| Input: Called from ____
| Output: Reads one token from file and matches it with input string.
|   If not same, exits on error.  If input string is null string, matches
|   with anything.
|___________________________________________________________________*/

static void Match_Token (char *str)
{
  string token;

  DEBUG_ASSERT (str)

  *in >> token;
  if (NOT *in) {
    sprintf (debug_str, "Match_Token(): End of file, expecting token = %s", str);
    TERMINAL_ERROR (debug_str)
  }
  else if (strcmp(str, "") != 0) {
    if (token != str) {
      sprintf (debug_str, "Match_Token(): expecting token = %s", str);
      TERMINAL_ERROR (debug_str)
    }
  }
}

/*____________________________________________________________________
|
| Function: Process_Metadata
|
| Input: Called from Process_Object_Layer()
| Output: Parses the file for metadata.
|___________________________________________________________________*/

static lws_Metadata *Process_Metadata (lws_ObjectLayer *olayer, string *token)
{
  lws_Metadata *metadata = 0;
  bool channels_found = false;

	DEBUG_ASSERT (olayer)

  // Allocate memory for metadata
  metadata = (lws_Metadata *) calloc (1, sizeof(lws_Metadata));
  if (metadata == 0)
    TERMINAL_ERROR ("Process_Metadata(): Error allocating memory for metadata")

  // Skip next number
  Match_Token ("");
  // Get name of metadata
  *in >> *token;
  if (NOT *in)
    TERMINAL_ERROR ("Process_Metadata(): Unexpected end of file reading Metadata name")
  metadata->name = (char *) malloc (token->size() + 1);
  if (metadata->name == 0)
    TERMINAL_ERROR ("Process_Metadata(): Error allocating memory for Metadata name")
  strcpy (metadata->name, token->c_str());

  // Parse rest of metadata  
  *in >> *token;
  for (;;) {
    if (NOT *in)
      break;
    else if (*token == "NumChannels") {
      Process_MetadataMotion (metadata);
      channels_found = true;
      *in >> *token;
      break;  // done with this metadata
    }
    else
      *in >> *token;
  }

  // Make sure channels were found
  if (NOT channels_found)
    TERMINAL_ERROR ("Process_Metadata(): Error no channels found");

  return (metadata);
}

/*____________________________________________________________________
|
| Function: Process_MetadataMotion
|
| Input: Called from Process_Metadata()
| Output: Reads one metadata object from LWS file.
|___________________________________________________________________*/

static void Process_MetadataMotion (lws_Metadata *metadata)
{
  string token;
  int i, num_channels;
  lws_Channel *channel [gx3dMotionMetadata_MAX_CHANNELS] = {0,0,0,0,0,0};

	DEBUG_ASSERT (metadata)

  // Parse the metadata
//  Match_Token ("NumChannels");  // already matched by caller
  *in >> num_channels;
  if (NOT *in)
    TERMINAL_ERROR ("Process_MetadataMotion(): number of channels not found")
  else if (num_channels != 9) {
    char str[200];
    sprintf (str, "Process_MetadataMotion(): number of channels [%d] != 9", num_channels);
    TERMINAL_ERROR (str)
  }
 
  // Process xyz position and yxz rotation data
  for (i=0; i<6; i++) {
    channel[i] = Process_Channel (i);
    if (channel[i] == 0)
      TERMINAL_ERROR ("Process_MetadataMotion(): channel not processed")
  }
  // Skip last 3 channels
  for (i=0; i<3; i++)
    Skip_Channel (6+i);

  // Allocate memory for output key data
  for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++) {
    if (channel[i]->nkeys) {
      metadata->channel[i].nkeys = channel[i]->nkeys;
      metadata->channel[i].keys = (lws_Key *) calloc (metadata->channel[i].nkeys, sizeof(lws_Key));
      if (metadata->channel[i].keys == 0)
        TERMINAL_ERROR ("Process_MetadataMotion(): Error allocating memory for key array")
    }
  }
  // Read in keys
  for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++)
    if (metadata->channel[i].keys)
      memcpy ((void *)(metadata->channel[i].keys), (void *)(channel[i]->keys), metadata->channel[i].nkeys * sizeof(lws_Key));

  // Swap channels 3 and 4 (since LWS file has rotations in order YXZ and we want them listed XYZ in the metadata channels)
  int      t_nkeys;
  lws_Key *t_keys;
  t_nkeys = metadata->channel[3].nkeys;
  t_keys  = metadata->channel[3].keys;
  metadata->channel[3].nkeys = metadata->channel[4].nkeys;
  metadata->channel[3].keys  = metadata->channel[4].keys;
  metadata->channel[4].nkeys = t_nkeys;
  metadata->channel[4].keys  = t_keys;

  // Free memory 
  for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++) {
    free (channel[i]->keys);
    free (channel[i]);
  }
}

/*____________________________________________________________________
|
| Function: lws_WriteTextFile
|
| Output: Write lws_ObjectLayer to a text file.
|___________________________________________________________________*/

void lws_WriteTextFile (char *filename, lws_ObjectLayer *olayer)
{
  int i, j;
	lws_Bone *bone;
  lws_Metadata *metadata;
  ofstream out;

	DEBUG_ASSERT (filename)
	DEBUG_ASSERT (olayer)

	out.open (filename);
	if (NOT out)
		DEBUG_ERROR ("lws_WriteTextFile(): Can't open output text file")
	else {
		out << "Layer " << olayer->lwo_filename << endl;
		out << "Id " << olayer->id << endl;
		out << "Position: " << olayer->position.x << ' ' << olayer->position.y << ' ' << olayer->position.z << endl;
		out << "Rotation: " << olayer->rotation.x << ' ' << olayer->rotation.y << ' ' << olayer->rotation.z << endl;
		out << "Keys-per-second " << olayer->keys_per_second << endl;
	  out << "Max-nkeys " << olayer->max_nkeys << endl;
		out << endl;
    // Print all bones
		for (bone=olayer->bones; bone; bone=bone->next) {
			out << "Bone " << bone->name;
			if (bone->active == 0)
				out << " (inactive)";
			out << endl;
			out << "Id " << bone->id << endl;
			if (bone->parent_id == -1)
				out << "Parent (root)" << endl;
			else
				out << "Parent " << bone->parent_id << endl;
			out << "Rotation " << bone->rotation.x << ' ' << bone->rotation.y << ' ' << bone->rotation.z << endl;
			out << "Normal " << bone->normal.x << ' ' << bone->normal.y << ' ' << bone->normal.z << endl;
  		out << "Pivot " << bone->pivot.x << ' ' << bone->pivot.y << ' ' << bone->pivot.z << endl;
			out << "Length " << bone->length << endl;
			if (bone->weightmap_name)
				out << "WeightMapName " << bone->weightmap_name << endl;
			else
				out << "WeightMapName 0" << endl;
			out << "NumKeys " << bone->motion.nkeys << endl;
			if (bone->parent_id == -1) {
				if (bone->motion.nkeys)
				  out << "// POSITION (x,y,z)" << endl;
				for (i=0; i<bone->motion.nkeys; i++) 
					out << i << ' ' << bone->motion.pos[i].x
									 << ' ' << bone->motion.pos[i].y
									 << ' ' << bone->motion.pos[i].z << endl;
			}
			if (bone->motion.nkeys) 
			  out << "// ROTATION (x,y,z)" << endl;
			for (i=0; i<bone->motion.nkeys; i++) 
				out << i << ' ' << bone->motion.rot[i].x
								 << ' ' << bone->motion.rot[i].y
								 << ' ' << bone->motion.rot[i].z << endl;
			out << endl;
		}
    // Print all metadata
    for (metadata=olayer->metadata; metadata; metadata=metadata->next) {
      out << "Metadata " << metadata->name << endl;
      static char *metadata_channel_heading [gx3dMotionMetadata_MAX_CHANNELS] = { "POSITION X", "POSITION Y", "POSITION Z", "ROTATION X", "ROTATION Y", "ROTATION Z" };
      for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++) 
        if (metadata->channel[i].keys) { 
          out << "// " << metadata_channel_heading[i] << " (time,value)" << endl;
          for (j=0; j<metadata->channel[i].nkeys; j++) 
            out << fixed << showpoint << setprecision(4) << metadata->channel[i].keys[j].time << ' '
                << fixed << showpoint << setprecision(4) << metadata->channel[i].keys[j].value << endl;
        }
      out <<endl;
    }
	}
}

/*____________________________________________________________________
|
| Function: lws_FreeObjectLayer
|
| Output: Frees an lws_ObjectLayer.
|___________________________________________________________________*/

void lws_FreeObjectLayer (lws_ObjectLayer *olayer)
{
  lws_ObjectLayer *tol;
  lws_Bone *tb;
  lws_Metadata *tm;

  DEBUG_ASSERT (olayer)

  while (olayer) {
    if (olayer->name)
      free (olayer->name);
    if (olayer->lwo_filename)
      free (olayer->lwo_filename);
    // Free all bones
    while (olayer->bones) {
      if (olayer->bones->name)
        free (olayer->bones->name);
      if (olayer->bones->weightmap_name)
        free (olayer->bones->weightmap_name);
      if (olayer->bones->motion.pos)
        free (olayer->bones->motion.pos);
      if (olayer->bones->motion.rot)
        free (olayer->bones->motion.rot);
      tb = olayer->bones;
      olayer->bones = olayer->bones->next;
      free (tb);
    }
    // Free all metadata
    while (olayer->metadata) {
      tm = olayer->metadata;
      olayer->metadata = olayer->metadata->next;
      Free_Metadata (tm);
    }
    tol = olayer;
    olayer = olayer->next;
    free (tol);
  }
}

/*____________________________________________________________________
|
| Function: Free_Metadata
|
| Input: Called from lws_FreeObjectLayer()
|  Output: Frees an lws_Metadata.
|___________________________________________________________________*/

static void Free_Metadata (lws_Metadata *metadata)
{
  int i;
  
  DEBUG_ASSERT (metadata)

  if (metadata->name)
    free (metadata->name);
  for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++) 
    if (metadata->channel[i].keys)
       free (metadata->channel[i].keys);
  free (metadata);
}

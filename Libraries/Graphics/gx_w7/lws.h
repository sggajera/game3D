/*____________________________________________________________________
|
| File: lws.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Type definitions
|__________________*/

struct lws_Key {
  float time;
  float value;
};

struct lws_Channel {
  int  nkeys;
  lws_Key *keys;
};

struct lws_Metadata {
  char         *name;
  lws_Channel   channel[gx3dMotionMetadata_MAX_CHANNELS];
  lws_Metadata *next;            // used to create linked list
};

struct lws_BoneMotion {
  int				  nkeys;	
  gx3dVector *pos;		// nkeys size array of position (root bone only)
  gx3dVector *rot;    // nkeys size array of rotation
};

struct lws_Bone {
  char					 *name;
  int							id;             // 0-?
  int							parent_id;      // 0-? (-1 = root bone)
  float						length;					// bone length
  gx3dVector      pivot;          // bone pivot point (relative to the local coordinate origin)
  gx3dVector			rotation;	  		// initial rotation from previous bone (or from 0,0,1 if bone is root) 
	gx3dVector      normal;		      // normalized direction bone begins pointing
	gx3dMatrix			pre, post;
	char					 *weightmap_name;    
  int							active;         // 0-1
  lws_BoneMotion  motion;
  lws_Bone       *next;           // used to create linked list
};  

struct lws_ObjectLayer {
  char						 *name;
	char						 *lwo_filename;
  int								id;               // 0-?
	gx3dVector			  position;					// position of the object at the start of the animation
	gx3dVector				rotation; 				// rotation of the object at the start of the animation
  int								keys_per_second;	   
  int								max_nkeys;        // max # keys of any active bone 
  int								num_bones;        // total # bones (active and inactive)
  lws_Bone         *bones;            // linked list of bones
  lws_Metadata     *metadata;         // linked list of metadata
  lws_ObjectLayer  *next;							// not used (for now)
};

/*___________________
|
| Function prototypes
|__________________*/

// Read an LWS file and convert the data to frames_per_second
lws_ObjectLayer *lws_ReadFile				 (char *filename, int *frames_per_second, bool read_metadata);
// Read an LWS file (top level data only, don't bother reading in keyframes)
lws_ObjectLayer *lws_ReadFile        (char *filename);
// Write the LWS data to a text file
void             lws_WriteTextFile   (char *filename, lws_ObjectLayer *olayer);
// Free LWS data
void             lws_FreeObjectLayer (lws_ObjectLayer *olayer);

/*____________________________________________________________________
|
| File: gx3d_lws.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

// Convert an LWS file to a gx3dMotion
void LWS_File_To_GX3D_Motion  (
  char                      *filename, 
  gx3dMotion                *g_motion,
  int                        frames_per_second, // target framerate for gx3dMotion
  gx3dMotionMetadataRequest *metadata_requested, 
  int                        num_metadata_requested,
  bool                       load_all_metadata );

// Convert an LWS file to a gx3dMotionSkeleton
void LWS_File_To_GX3D_MotionSkeleton  (char *filename, gx3dMotionSkeleton *g_skeleton);

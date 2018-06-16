/*____________________________________________________________________
|
| File: script_psys.cpp
|
| Description: Functions to create particle systems from script files.
|
| Functions: Script_ParticleSystem_Create
|             Process_Token_Image
|             Process_Token_Emitter
|             Process_Token_Attached
|             Process_Token_Direction
|             Process_Token_Velocity
|             Process_Token_Transparency
|             Process_Token_Size
|             Process_Token_Lifespan
|             Process_Token_Population
|              Get_Token
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

//#include "script_psys.h"

/*___________________
|
| Function prototypes
|__________________*/

static void Process_Token_Image        (FILE *fp, char **image_file, char **alpha_file, int *error);
static void Process_Token_Emitter      (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static void Process_Token_Attached     (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static void Process_Token_Direction    (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static void Process_Token_Velocity     (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static void Process_Token_Transparency (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static void Process_Token_Size         (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static void Process_Token_Lifespan     (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static void Process_Token_Population   (FILE *fp, gx3dParticleSystemData *psysdata, int *error);
static int  Get_Token (FILE *fp, char *token, int *error);

/*___________________
|
| Defines
|__________________*/

#define STRINGS_EQUAL(_s1_,_s2_)	\
	(!strcmp(_s1_,_s2_))

/*____________________________________________________________________
|
| Function: Script_ParticleSystem_Create
|
| Input: Called from ____
| Output: Returns a gx3dParticleSystem created from the script file or
|   0 on any error.
|___________________________________________________________________*/

gx3dParticleSystem Script_ParticleSystem_Create (char *script_file)
{
  int found, done, error;
	char token[256], *image_file, *alpha_file;
  FILE *fp;
  gx3dParticleSystemData psysdata;
  gx3dParticleSystem psys = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  image_file = NULL;
  alpha_file = NULL;
  error = FALSE;

/*____________________________________________________________________
|
| Open input file and look for start of particle system data
|___________________________________________________________________*/

  // Open the input file
  fp = fopen (script_file, "rt");
  if (fp == NULL) {
		TERMINAL_ERROR ("Script_ParticleSystem_Create(): Error opening script file")
    error = TRUE;
	}
	else {
    // Look for 'start particle_system'
    for (found=FALSE; (NOT found) AND (NOT error); ) 
      if (Get_Token (fp, token, &error)) 
				if (STRINGS_EQUAL (token, "start")) 
					// Get token immediately after 'start'
					if (Get_Token (fp, token, &error)) 
						if (STRINGS_EQUAL (token, "particle_system"))
							found = TRUE;
    if (NOT found) {
			TERMINAL_ERROR ("Script_ParticleSystem_Create(): Error - particle system script not found in script file");
      error = TRUE;
    }
  }

/*____________________________________________________________________
|
| Build the particle system data structure
|___________________________________________________________________*/

  if (NOT error) {
    // Zero out psys struct
    memset (&psysdata, 0, sizeof(gx3dParticleSystemData));
    // Set some initial stuff
    psysdata.size_type = gx3d_PARTICLESYSTEM_SIZE_TYPE_TIME_VARIABLE;

    for (done=FALSE; (NOT done) AND (NOT error); ) {
      if (Get_Token (fp, token, &error)) {
			  // Process 'end'
			  if (STRINGS_EQUAL (token, "end")) 
          done = TRUE;
			  // Process 'image' command
			  else if (STRINGS_EQUAL (token, "image")) 
				  Process_Token_Image (fp, &image_file, &alpha_file, &error);
        // Process 'emitter' command
        else if (STRINGS_EQUAL (token, "emitter"))
				  Process_Token_Emitter (fp, &psysdata, &error);
        // Process 'attached' command
        else if (STRINGS_EQUAL (token, "attached"))
          Process_Token_Attached (fp, &psysdata, &error);
        // Process 'direction' command
        else if (STRINGS_EQUAL (token, "direction")) 
				  Process_Token_Direction (fp, &psysdata, &error);
        // Process 'velocity' command
        else if (STRINGS_EQUAL (token, "velocity")) 
			    Process_Token_Velocity (fp, &psysdata, &error);
        // Process 'transparency' command
        else if (STRINGS_EQUAL (token, "transparency")) 
				  Process_Token_Transparency (fp, &psysdata, &error);
        // Process 'size' command
        else if (STRINGS_EQUAL (token, "size")) 
				  Process_Token_Size (fp, &psysdata, &error);
        // Process 'population' command
        else if (STRINGS_EQUAL (token, "population")) 
				  Process_Token_Population (fp, &psysdata, &error);
        // Process 'lifespan' command
        else if (STRINGS_EQUAL (token, "lifespan")) 
				  Process_Token_Lifespan (fp, &psysdata, &error);
        else {
				  TERMINAL_ERROR ("Script_ParticleSystem_Create(): Error unknown token")
				  error = TRUE;
			  }
      }
    }
		fclose (fp);
  }

/*____________________________________________________________________
|
| Create the particle system
|___________________________________________________________________*/

  if (NOT error) {
    // Normalize direction vector
    if ((psysdata.direction.x != 0) OR (psysdata.direction.y != 0) OR (psysdata.direction.z != 0))
      gx3d_NormalizeVector (&(psysdata.direction), &(psysdata.direction));
    psys = gx3d_InitParticleSystem (&psysdata, image_file, alpha_file, 0);
    if (psys == 0)
      TERMINAL_ERROR ("Script_ParticleSystem_Create(): Error creating particle system")
  }

/*____________________________________________________________________
|
| Free allocated memory
|___________________________________________________________________*/

  if (image_file)
    free (image_file);
  if (alpha_file)
    free (alpha_file);

  return (psys);
}

/*____________________________________________________________________
|
| Function: Process_Token_Image
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   image color = pathname
|   image alpha = pathname
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Image (FILE *fp, char **image_file, char **alpha_file, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    // Process color file
    if (STRINGS_EQUAL (token, "color")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Image(): Error expecting '='")
          *error = TRUE;
        }
        else {
          if (Get_Token (fp, token, error)) {
            *image_file = (char *) malloc (strlen(token)+1);
            if (image_file == 0) {
              TERMINAL_ERROR ("Process_Token_Image(): Error allocating memory")
              *error = TRUE;
            }
            else 
              strcpy (*image_file, token);
          }
        }
      }
    }
    // Process alpha file
    else if (STRINGS_EQUAL (token, "alpha")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Image(): Error expecting '='")
          *error = TRUE;
        }
        else {
          if (Get_Token (fp, token, error)) {
            *alpha_file = (char *) malloc (strlen(token)+1);
            if (alpha_file == 0) {
              TERMINAL_ERROR ("Process_Token_Image(): Error allocating memory")
              *error = TRUE;
            }
            else 
              strcpy (*alpha_file, token);
          }
        }
      }
    }
    else {
  		TERMINAL_ERROR ("Process_Token_Image(): Error unknown token")
	  	*error = TRUE;
    }
  }
}

/*____________________________________________________________________
|
| Function: Process_Token_Emitter
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   emitter type = (point, rectangle, circle, cube, sphere, cone)
|   emitter dx = float
|   emitter dy = float
|   emitter dz = float
|   emitter radius = float
|   emitter height = float
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Emitter (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    // Process emitter type
    if (STRINGS_EQUAL (token, "type")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Emitter(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get type
          if (Get_Token (fp, token, error)) {
            if (STRINGS_EQUAL (token, "point")) 
              psysdata->emitter.type = gx3d_PARTICLESYSTEM_EMITTER_TYPE_POINT;
            else if (STRINGS_EQUAL (token, "rectangle")) 
              psysdata->emitter.type = gx3d_PARTICLESYSTEM_EMITTER_TYPE_RECTANGLE;
            else if (STRINGS_EQUAL (token, "circle")) 
              psysdata->emitter.type = gx3d_PARTICLESYSTEM_EMITTER_TYPE_CIRCLE;
            else if (STRINGS_EQUAL (token, "cube")) 
              psysdata->emitter.type = gx3d_PARTICLESYSTEM_EMITTER_TYPE_CUBE;
            else if (STRINGS_EQUAL (token, "sphere")) 
              psysdata->emitter.type = gx3d_PARTICLESYSTEM_EMITTER_TYPE_SPHERE;
            else if (STRINGS_EQUAL (token, "cone")) 
              psysdata->emitter.type = gx3d_PARTICLESYSTEM_EMITTER_TYPE_CONE;
            else {
        			TERMINAL_ERROR	("Process_Token_Emitter(): Error unknown token")
			        *error = TRUE;
            }
          }
        }
      }
    }
    // Process emitter dx
    else if (STRINGS_EQUAL (token, "dx")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Emitter(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->emitter.dx = (float) atof (token);
        }
      }
    }
    // Process emitter dy
    else if (STRINGS_EQUAL (token, "dy")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Emitter(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->emitter.dy = (float) atof (token);
        }
      }
    }
    // Process emitter dz
    else if (STRINGS_EQUAL (token, "dz")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Emitter(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->emitter.dz = (float) atof (token);
        }
      }
    }
    // Process emitter radius
    else if (STRINGS_EQUAL (token, "radius")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Emitter(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->emitter.radius = (float) atof (token);
        }
      }
    }
    // Process emitter height
    else if (STRINGS_EQUAL (token, "height")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Emitter(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->emitter.height = (float) atof (token);
        }
      }
    }    
    else {
  		TERMINAL_ERROR ("Process_Token_Emitter(): Error unknown token")
	  	*error = TRUE;
    }
  }
}

/*____________________________________________________________________
|
| Function: Process_Token_Attached
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read attached info from script.  Should be in this form:
|
|   attached = true OR false
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Attached (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    if (NOT STRINGS_EQUAL (token, "=")) {
      TERMINAL_ERROR ("Process_Token_Attached(): Error expecting '='")
      *error = TRUE;
    }
    else {
      // Get boolean
      if (Get_Token (fp, token, error)) {
        if (STRINGS_EQUAL (token, "true")) 
          psysdata->attached_particles = true;
        else if (STRINGS_EQUAL (token, "false")) 
          psysdata->attached_particles = false;
        else {
        	TERMINAL_ERROR	("Process_Token_Attached(): Error - unexpected token")
			    *error = TRUE;
        }
      }
    }                                 
  }
  else {
    TERMINAL_ERROR ("Process_Token_Attached(): Error unknown token")
		*error = TRUE;
  }
}

/*____________________________________________________________________
|
| Function: Process_Token_Direction
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   direction type = (fixed, random)
|   direction x = float
|   direction y = float
|   direction z = float
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Direction (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    // Process direction type
    if (STRINGS_EQUAL (token, "type")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Direction(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get type
          if (Get_Token (fp, token, error)) {
            if (STRINGS_EQUAL (token, "fixed")) 
              psysdata->direction_type = gx3d_PARTICLESYSTEM_DIRECTION_TYPE_FIXED;
            else if (STRINGS_EQUAL (token, "random")) 
              psysdata->direction_type = gx3d_PARTICLESYSTEM_DIRECTION_TYPE_RANDOM;
            else {
        			TERMINAL_ERROR	("Process_Token_Direction(): Error - unexpected token")
			        *error = TRUE;
            }
          }
        }
      }
    }
    // Process direction x
    else if (STRINGS_EQUAL (token, "x")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Direction(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->direction.x = (float) atof (token);
        }
      }
    }
    // Process direction y
    else if (STRINGS_EQUAL (token, "y")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Direction(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->direction.y = (float) atof (token);
        }
      }
    }
    // Process direction z
    else if (STRINGS_EQUAL (token, "z")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Direction(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->direction.z = (float) atof (token);
        }
      }
    }
    else {
  		TERMINAL_ERROR ("Process_Token_Direction(): Error unknown token")
	  	*error = TRUE;
    }
  }
}

/*____________________________________________________________________
|
| Function: Process_Token_Velocity
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   velocity type = (fixed)
|   velocity min = float
|   velocity max = float
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Velocity (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    // Process velocity type
    if (STRINGS_EQUAL (token, "type")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Velocity(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get type
          if (Get_Token (fp, token, error)) {
            if (STRINGS_EQUAL (token, "fixed")) 
              psysdata->velocity_type = gx3d_PARTICLESYSTEM_VELOCITY_TYPE_FIXED;
            else {
        			TERMINAL_ERROR	("Process_Token_Velocity(): Error - unexpected token")
			        *error = TRUE;
            }
          }
        }
      }
    }
    // Process velocity min
    else if (STRINGS_EQUAL (token, "min")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Velocity(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->min_velocity = (float) atof (token);
        }
      }
    }
    // Process velocity max
    else if (STRINGS_EQUAL (token, "max")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Velocity(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->max_velocity = (float) atof (token);
        }
      }
    }
    else {
  		TERMINAL_ERROR ("Process_Token_Velocity(): Error unknown token")
	  	*error = TRUE;
    }
  }
}

/*____________________________________________________________________
|
| Function: Process_Token_Transparency
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   transparency type = (fixed, fade)
|   transparency start = float
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Transparency (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    // Process transparency type
    if (STRINGS_EQUAL (token, "type")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Transparency(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get type
          if (Get_Token (fp, token, error)) {
            if (STRINGS_EQUAL (token, "fixed")) 
              psysdata->transparency_type = gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_FIXED;
            else if (!strcmp (token, "fade")) 
              psysdata->transparency_type = gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_FADE;
            else {
        			TERMINAL_ERROR	("Process_Token_Transparency(): Error - unexpected token")
			        *error = TRUE;
            }
          }
        }
      }
    }
    // Process transparency start
    else if (STRINGS_EQUAL (token, "start")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Transparency(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->start_transparency = (float) atof (token);
        }
      }
    }
    // Process transparency end
    else if (STRINGS_EQUAL (token, "end")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Transparency(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->end_transparency = (float) atof (token);
        }
      }
    }
    else {
  		TERMINAL_ERROR ("Process_Token_Transparency(): Error unknown token")
	  	*error = TRUE;
    }
  }
}

/*____________________________________________________________________
|
| Function: Process_Token_Size
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   size type = (fixed, time_variable, lifetime_variable)
|   size start = float
|   size end = float
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Size (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    // Process size type
    if (STRINGS_EQUAL (token, "type")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Size(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get type
          if (Get_Token (fp, token, error)) {
            if (STRINGS_EQUAL (token, "fixed")) 
              psysdata->size_type = gx3d_PARTICLESYSTEM_SIZE_TYPE_FIXED;
            else if (!strcmp (token, "time_variable")) 
              psysdata->size_type = gx3d_PARTICLESYSTEM_SIZE_TYPE_TIME_VARIABLE;
            else if (!strcmp (token, "lifetime_variable")) 
              psysdata->size_type = gx3d_PARTICLESYSTEM_SIZE_TYPE_LIFETIME_VARIABLE;
            else {
        			TERMINAL_ERROR	("Process_Token_Size(): Error - unexpected token")
			        *error = TRUE;
            }
          }
        }
      }
    }
    // Process size start
    else if (STRINGS_EQUAL (token, "start")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Size(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->start_size = (float) atof (token);
        }
      }
    }
    // Process size end
    else if (STRINGS_EQUAL (token, "end")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Size(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->end_size = (float) atof (token);
        }
      }
    }
    else {
  		TERMINAL_ERROR ("Process_Token_Size(): Error unknown token")
	  	*error = TRUE;
    }
  }
}

/*____________________________________________________________________
|
| Function: Process_Token_Lifespan
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   lifespan min = integer
|   lifespan max = integer
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Lifespan (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    // Process lifespan min
    if (STRINGS_EQUAL (token, "min")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Lifespan(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->min_lifespan = (float) atof (token);
        }
      }
    }
    // Process lifespan max
    else if (STRINGS_EQUAL (token, "max")) {
      if (Get_Token (fp, token, error)) {
        if (NOT STRINGS_EQUAL (token, "=")) {
          TERMINAL_ERROR ("Process_Token_Lifespan(): Error expecting '='")
          *error = TRUE;
        }
        else {
          // Get value
          if (Get_Token (fp, token, error)) 
            psysdata->max_lifespan = (float) atof (token);
        }
      }
    }
    else {
  		TERMINAL_ERROR ("Process_Token_Lifespan(): Error unknown token")
	  	*error = TRUE;
    }
  }
}


/*____________________________________________________________________
|
| Function: Process_Token_Population
|
| Input: Called from Script_ParticleSystem_Create()
| Output: Read image info from script.  Should be in this form:
|
|   population = integer
|
|   On any error, sets callers error variable to true.
|___________________________________________________________________*/

static void Process_Token_Population (FILE *fp, gx3dParticleSystemData *psysdata, int *error)
{
  char token[256];

  if (Get_Token (fp, token, error)) {
    if (NOT STRINGS_EQUAL (token, "=")) {
      TERMINAL_ERROR ("Process_Token_Population(): Error expecting '='")
      *error = TRUE;
    }
    else {
      // Get value
      if (Get_Token (fp, token, error)) 
        psysdata->max_particles = atoi (token);
    }                                 
  }
  else {
    TERMINAL_ERROR ("Process_Token_Population(): Error unknown token")
		*error = TRUE;
  }
}

/*____________________________________________________________________
|
| Function: Get_Token
|
| Input: Called from ____
| Output: Gets next token from the file, returning it in token.  Returns
|   true if successful, else false on any error.  On error, also sets
|   callers error variable to true.
|___________________________________________________________________*/

#define ISWHITE(_ch_) (isspace(_ch_) OR (_ch_ == '\n'))

static int Get_Token (FILE *fp, char *token, int *error)
{
  int i, done;
  char ch;

  for (done = FALSE; NOT done; ) {

/*____________________________________________________________________
|
|  Strip off leading whitespace
|___________________________________________________________________*/

    if (NOT feof (fp)) {
      do {
        ch = fgetc (fp);
      } while (ISWHITE(ch) AND (NOT (feof(fp))));
      if ((NOT ISWHITE (ch)) AND (NOT feof(fp)))
        fseek (fp, -1, SEEK_CUR);
    }

/*____________________________________________________________________
|
| Collect a token
|___________________________________________________________________*/

    i = 0;
    token[0] = 0;
    if (NOT feof (fp)) {
      do {
        // Start of a comment ("//") is a token by itself, all other tokens are whitespace separated
        if (STRINGS_EQUAL (token, "//"))
          break;
        ch = fgetc (fp);
        token[i] = ch;
        token[i+1] = 0;
        i++;
      } while ((NOT ISWHITE (ch)) AND (NOT feof(fp)));
      if (ISWHITE (ch)) {
        fseek (fp, -1, SEEK_CUR);
        token[i-1] = 0;
      }
    }

/*____________________________________________________________________
|
| Ignore comments
|___________________________________________________________________*/

    if (STRINGS_EQUAL (token, "//")) {
      // Strip off to end of line
      if (NOT feof (fp)) {
        do {
          ch = fgetc (fp);
        } while ((ch != '\n') AND (NOT feof(fp)));
      }
    }
    else 
      done = TRUE;
  }

/*____________________________________________________________________
|
| If no token found, set error
|___________________________________________________________________*/

  if (token[0] == 0)
    *error = TRUE;

  return ((int)token[0]);
}

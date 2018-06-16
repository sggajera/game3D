/*____________________________________________________________________
|
| File: gx3d_particlesystem.cpp
|
| Description: Functions to manipulate particle systems.
|
| Functions:  gx3d_InitParticleSystem
|              Create_ParticleSystem_Object
|             gx3d_FreeParticleSystem
|             gx3d_UpdateParticleSystem
|              Emit_Particle
|             gx3d_SetParticleSystemMatrix
|             gx3d_DrawParticleSystem
|              Compare_Function
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define DEBUG

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <math.h>

#include "dp.h"

/*___________________
|
| Type definitions
|__________________*/

typedef int millisecond;

// Particle
typedef struct {
  gx3dVector       position;
  gx3dVector       direction;
  float            velocity;      // in world units (typically feet) per second
  float            transparency;  // 0 to 1.0
  float            size;
  millisecond      lifespan;      // in milliseconds
  millisecond      age;           // timestamp in milliseconds (when age >= lifespan, particle dies)
  gx3dVector       vertex[4];
  gx3dVector       vertex_normal; 
  float            center_z;      // center z coordinate of particle (used for sorting)
  gx3dUVCoordinate tex_coords[4];
} Particle;

// Particle system
typedef struct {
  gx3dParticleSystemData  data;
  gx3dVector              base_vertex[4];     // base geometry for a particle
  gx3dVector              base_vertex_normal; // base normal for a particle
  gx3dVector              X_base_vertex[4];
  gx3dVector              X_base_vertex_normal;
  gx3dVector              emitter_world_position;
  Particle               *particles;            // particle array (circular array)
  int                     num_particles;        // # particles in array
  int                     head;                 // index into particle array of next empty slot
  int                     tail;                 // index into particle array of last live particle
  int                    *draw_order;           // array of indexes into particles array
  millisecond             gestation_time;       // # running time before a particle is birthed
  millisecond             birth_rate;           // # milliseconds to generate 1 particle
  gx3dObject             *object;
  // Transformation stuff
  gx3dMatrix              local_matrix;
  gx3dMatrix              view_matrix;
} ParticleSystem;

/*___________________
|
| Function prototypes
|__________________*/

static int Create_ParticleSystem_Object (
  ParticleSystem *psys,
  char           *image_texture_filename,
  char           *alpha_texture_filename,
  unsigned        flags );
static void Emit_Particle (gx3dEmitter *emitter, gx3dVector *position);
static int  Compare_Function (const void *elem1, const void *elem2);

/*___________________
|
| Constants
|__________________*/

#define FULL_ARRAY (((psys->head+1)%psys->num_particles) == psys->tail)

/*___________________
|
| Global variables
|__________________*/

static Particle *particles_array;  // used by qsort function

/*____________________________________________________________________
|
| Function: gx3d_InitParticleSystem
|                                                                                        
| Output: Creates a new particle system based on input data.  Returns
|   particle system or NULL on any error.
|___________________________________________________________________*/

gx3dParticleSystem gx3d_InitParticleSystem (
  gx3dParticleSystemData *particle_system_data,
  char                   *image_texture_filename,
  char                   *alpha_texture_filename,
  unsigned                flags )
{
  int error = FALSE;
  ParticleSystem *psys = NULL;

  // Verify input parameters
  if (particle_system_data) {
    if (particle_system_data->max_particles > 0) {

      // Allocate memory for a new particle system
      psys = (ParticleSystem *) calloc (1, sizeof(ParticleSystem));
      if (psys == NULL) 
        error = TRUE;
      else {
        // Allocate memory for particles stuff
        psys->particles  = (Particle *) calloc (particle_system_data->max_particles+1, sizeof(Particle));
        psys->draw_order = (int *)      calloc (particle_system_data->max_particles+1, sizeof(int));
        if ((psys->particles == NULL) OR (psys->draw_order == NULL)) 
          error = TRUE;
        else {
          // Init data section
          memcpy (&(psys->data), particle_system_data, sizeof(gx3dParticleSystemData));
          // Init other variables
          psys->num_particles = psys->data.max_particles + 1;
          psys->head = 0;
          psys->tail = 0;
          psys->gestation_time = 0;
          psys->birth_rate = 1000 / (int)((float)(psys->data.max_particles) / psys->data.max_lifespan);
          // Init 'base' particle geometry
          psys->base_vertex[0].x = -0.5;
          psys->base_vertex[0].y =  0.5;
          psys->base_vertex[0].z =  0;
          psys->base_vertex[1].x =  0.5;
          psys->base_vertex[1].y =  0.5;
          psys->base_vertex[1].z =  0;
          psys->base_vertex[2].x = -0.5;
          psys->base_vertex[2].y = -0.5;
          psys->base_vertex[2].z =  0;
          psys->base_vertex[3].x =  0.5;
          psys->base_vertex[3].y = -0.5;
          psys->base_vertex[3].z =  0;
          psys->base_vertex_normal.x = 0;
          psys->base_vertex_normal.y = 0;
          psys->base_vertex_normal.z = -1;
          // Init transforms
          gx3d_GetIdentityMatrix (&(psys->local_matrix));
          gx3d_GetIdentityMatrix (&(psys->view_matrix));
          // Init particle system drawing object
          if (NOT Create_ParticleSystem_Object (psys, image_texture_filename, alpha_texture_filename, flags))
            error = TRUE;
        }
      }
    }
  }

  // If any error, delete the particle system
  if (error) {
    gxError ("gx3d_InitParticleSystem(): Error, particle system not created");
    if (psys) {
      gx3d_FreeParticleSystem ((gx3dParticleSystem)psys);
      psys = NULL;
    }
  }

  return ((gx3dParticleSystem) psys);
}

/*____________________________________________________________________
|
| Function: Create_ParticleSystem_Object
|          
| Input: Called from gx3d_InitParticleSystem()                                                                              
| Output: Creates a gx3d object to be used by the particle system.
|___________________________________________________________________*/

static int Create_ParticleSystem_Object (
  ParticleSystem *psys,
  char           *image_texture_filename,
  char           *alpha_texture_filename,
  unsigned        flags )
{
  gx3dObjectLayer *layer = 0;
  int error = FALSE;

  if (psys AND image_texture_filename AND alpha_texture_filename) {

/*____________________________________________________________________
|
| Allocate memory for the object
|___________________________________________________________________*/

    // Create an empty 3d object
    psys->object = gx3d_CreateObject ();
    if (psys->object == NULL) 
      error = TRUE;
    else {
      // Create a layer in the object
      layer = gx3d_CreateObjectLayer (psys->object);
      if (layer == NULL)
        error = TRUE;
    }

    if (NOT error) {
      // Allocate memory for each part of the object
      layer->num_vertices = 4;
      layer->num_polygons = 2;
      layer->polygon = (gx3dPolygon *) calloc (layer->num_polygons, sizeof(gx3dPolygon));
      if (layer->polygon == NULL)
        error = TRUE;
      layer->tex_coords[0] = (gx3dUVCoordinate *) calloc (layer->num_vertices, sizeof(gx3dUVCoordinate));
      if (layer->tex_coords[0] == NULL)
        error = TRUE;

      // Allocate memory for parts of the object that won't be used (but still need to be allocated)
      layer->vertex = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
      if (layer->vertex == NULL)
        error = TRUE;
      layer->vertex_normal = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
      if (layer->vertex_normal == NULL)
        error = TRUE;

/*____________________________________________________________________
|
| Init the object with values
|___________________________________________________________________*/

      if (NOT error) {

        // Init polygon array
        layer->polygon[0].index[0] = 0;
        layer->polygon[0].index[1] = 1;
        layer->polygon[0].index[2] = 2;
        layer->polygon[1].index[0] = 1;
        layer->polygon[1].index[1] = 3;
        layer->polygon[1].index[2] = 2;
        
        // Init upper left corner of particle square
        layer->tex_coords[0][0].u = 0;
        layer->tex_coords[0][0].v = 0;
        // Init upper right corner of particle square
        layer->tex_coords[0][1].u = 1;
        layer->tex_coords[0][1].v = 0;
        // Init upper left corner of particle square
        layer->tex_coords[0][2].u = 0;
        layer->tex_coords[0][2].v = 1;
        // Init upper left corner of particle square
        layer->tex_coords[0][3].u = 1;
        layer->tex_coords[0][3].v = 1;
      }
    }

/*____________________________________________________________________
|
| Load texture
|___________________________________________________________________*/

    if (NOT error) {
      // Load the callers texture
      layer->texture[0] = gx3d_InitTexture_File (image_texture_filename, alpha_texture_filename, flags);
      if (layer->texture[0] == 0)
        error = TRUE;
    }
  } // if (psys)

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: gx3d_FreeParticleSystem
|                                                                                        
| Output: Frees memory for a previously created particle system.
|___________________________________________________________________*/

void gx3d_FreeParticleSystem (gx3dParticleSystem particle_system)
{
  ParticleSystem *psys = (ParticleSystem *) particle_system;

  if (psys) {
    if (psys->particles) 
      free (psys->particles);
    if (psys->draw_order)
      free (psys->draw_order);
    if (psys->object)
      gx3d_FreeObject (psys->object);
    free (psys);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_UpdateParticleSystem
|                                                                                        
| Output: Updates particle system.
|___________________________________________________________________*/

void gx3d_UpdateParticleSystem (gx3dParticleSystem particle_system, unsigned elapsed_time)
{
  int i, n;
  float length;
  Particle *particle;
  ParticleSystem *psys = (ParticleSystem *) particle_system;

/*____________________________________________________________________
|
| Update age of all particles
|___________________________________________________________________*/

  for (i=psys->tail; i != psys->head; i=(i+1)%psys->num_particles)
    psys->particles[i].age += elapsed_time;
  
/*____________________________________________________________________
|
| Kill any particles that have met or exceeded their lifespan
|___________________________________________________________________*/

  while (psys->tail != psys->head) {
    if (psys->particles[psys->tail].age >= psys->particles[psys->tail].lifespan) 
      psys->tail = (psys->tail + 1) % psys->num_particles;
    else
      break;
  }
    
/*____________________________________________________________________
|
| Update attributes of current live particles                                                                                       
|___________________________________________________________________*/
  
  for (i=psys->tail; i != psys->head; i=(i+1)%psys->num_particles) {

    // Init variables
    particle = &(psys->particles[i]);

    // Update direction
    if (psys->data.direction_type == gx3d_PARTICLESYSTEM_DIRECTION_TYPE_USER)
      if (psys->data.update_direction)
        (*psys->data.update_direction) (particle->age, &(particle->direction));

    // Update velocity
    if (psys->data.velocity_type == gx3d_PARTICLESYSTEM_VELOCITY_TYPE_USER)
      if (psys->data.update_velocity)
        (*psys->data.update_velocity) (particle->age, &(particle->velocity));

    // Update transparency
    if (psys->data.transparency_type == gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_FIXED)
      particle->transparency = psys->data.start_transparency;
    else if (psys->data.transparency_type == gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_USER) {
      if (psys->data.update_transparency)
        (*psys->data.update_transparency) (particle->age, &(particle->transparency));
    }
    else if (psys->data.transparency_type == gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_FADE) {
      particle->transparency = psys->data.start_transparency + 
                               (psys->data.end_transparency - psys->data.start_transparency) * 
                               ((float)particle->age / (float)particle->lifespan);
      if (particle->transparency > 1.0)
        particle->transparency = 1.0;
      else if (particle->transparency < 0)
        particle->transparency = 0;
    }

    // Update size
    if (psys->data.size_type == gx3d_PARTICLESYSTEM_SIZE_TYPE_USER) {
      if (psys->data.update_size)
        (*psys->data.update_size) (particle->age, &(particle->size));
    }
    else if (psys->data.size_type == gx3d_PARTICLESYSTEM_SIZE_TYPE_TIME_VARIABLE) 
      particle->size = psys->data.start_size + (psys->data.end_size - psys->data.start_size) * ((float)particle->age / (psys->data.max_lifespan * 1000));
    else if (psys->data.size_type == gx3d_PARTICLESYSTEM_SIZE_TYPE_LIFETIME_VARIABLE) 
      particle->size = psys->data.start_size + (psys->data.end_size - psys->data.start_size) * ((float)particle->age / particle->lifespan);

    // Update position - move particle from current position along direction a distance equal to velocity * elapsed tim
    length = (float)elapsed_time / (float)1000 * particle->velocity;
    particle->position.x += (particle->direction.x * length); // assumption: particle->direction is normalized
    particle->position.y += (particle->direction.y * length);
    particle->position.z += (particle->direction.z * length);
  }

/*____________________________________________________________________
|
| Generate new particles
|___________________________________________________________________*/
  
  // Compute number of particles to generate        
  psys->gestation_time += elapsed_time;
  if (psys->gestation_time >= psys->birth_rate) {
    n = psys->gestation_time / psys->birth_rate;
    psys->gestation_time -= (n * psys->birth_rate);
  }
  else
    n = 0;

  gx3dVector emitter_origin = {0,0,0};
  if (NOT psys->data.attached_particles) 
    // Transform emitter into world space
    gx3d_MultiplyVectorMatrix (&emitter_origin, &(psys->local_matrix), &emitter_origin);

  // Generate new particles
  while (n AND (NOT FULL_ARRAY)) {

    // Init a particle
    particle = &(psys->particles[psys->head]);
    // Set age
    particle->age = 0;
    // Set lifespan  
    if (psys->data.min_lifespan == psys->data.max_lifespan)
      particle->lifespan = (int)(psys->data.min_lifespan * 1000);
    else
      particle->lifespan = (int)((psys->data.min_lifespan + (random_GetFloat() * (psys->data.max_lifespan - psys->data.min_lifespan))) * 1000);
    // Set initial direction
    if (psys->data.direction_type == gx3d_PARTICLESYSTEM_DIRECTION_TYPE_USER) {
      if (psys->data.update_direction)
        (*psys->data.update_direction) (particle->age, &(particle->direction));
    }
    else if (psys->data.direction_type == gx3d_PARTICLESYSTEM_DIRECTION_TYPE_RANDOM) {
      particle->direction.x = random_GetFloat () * (float)2 - (float)1;
      particle->direction.y = random_GetFloat () * (float)2 - (float)1;
      particle->direction.z = random_GetFloat () * (float)2 - (float)1;
      gx3d_NormalizeVector (&(particle->direction), &(particle->direction));
    }
    else 
      particle->direction = psys->data.direction;
    // Set initial velocity
    if (psys->data.velocity_type == gx3d_PARTICLESYSTEM_VELOCITY_TYPE_USER) {
      if (psys->data.update_velocity)
        (*psys->data.update_velocity) (particle->age, &(particle->velocity));
    }
    else {
      if (psys->data.min_velocity == psys->data.max_velocity)
        particle->velocity = psys->data.min_velocity;
      else
        particle->velocity = psys->data.min_velocity + (random_GetFloat() * (psys->data.max_velocity - psys->data.min_velocity));
    }
    // Set initial transparency
    if (psys->data.transparency_type == gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_USER) {
      if (psys->data.update_transparency)
        (*psys->data.update_transparency) (particle->age, &(particle->transparency));
    }
    else 
      particle->transparency = psys->data.start_transparency;
    // Set initial size
    if (psys->data.size_type == gx3d_PARTICLESYSTEM_SIZE_TYPE_USER) {
      if (psys->data.update_size)
        (*psys->data.update_size) (particle->age, &(particle->size));
    }
    else 
      particle->size = psys->data.start_size;
    // Set initial position
    Emit_Particle (&(psys->data.emitter), &(particle->position));
    if (NOT psys->data.attached_particles) 
      // Put particle into world space
      gx3d_AddVector (&(particle->position), &emitter_origin, &(particle->position));

    // Incr to next particle to generate, if any
    n--;
    psys->head = (psys->head + 1) % psys->num_particles;
  }
}

/*____________________________________________________________________
|
| Function: Emit_Particle
|                                                                                        
| Input: Called from gx3d_UpdateParticleSystem()
| Output: Updates particle system.
|___________________________________________________________________*/

static void Emit_Particle (gx3dEmitter *emitter, gx3dVector *position)
{
  float angle, radius, height;
  gx3dVector v;
  gx3dMatrix m;

  switch (emitter->type) {
    case gx3d_PARTICLESYSTEM_EMITTER_TYPE_POINT:
      position->x = 0;
      position->y = 0;
      position->z = 0;
      break;
    case gx3d_PARTICLESYSTEM_EMITTER_TYPE_RECTANGLE:
      position->x = random_GetFloat() * emitter->dx - (emitter->dx / 2);
      position->y = 0;
      position->z = random_GetFloat() * emitter->dz - (emitter->dz / 2);
      break;
    case gx3d_PARTICLESYSTEM_EMITTER_TYPE_CIRCLE:
      angle  = random_GetFloat() * 360 * DEGREES_TO_RADIANS;
      radius = random_GetFloat() * emitter->radius;
      position->x = radius * cosf (angle);
      position->y = 0;
      position->z = radius * sinf (angle);
      break;
    case gx3d_PARTICLESYSTEM_EMITTER_TYPE_CUBE:
      position->x = random_GetFloat() * emitter->dx - (emitter->dx / 2);
      position->y = random_GetFloat() * emitter->dy - (emitter->dy / 2);
      position->z = random_GetFloat() * emitter->dz - (emitter->dz / 2);
      break;
    case gx3d_PARTICLESYSTEM_EMITTER_TYPE_SPHERE:
      v.x = random_GetFloat();
      v.y = random_GetFloat();
      v.z = random_GetFloat();
      gx3d_NormalizeVector (&v, &v);
      radius = random_GetFloat() * emitter->radius;
      position->x = v.x * radius;
      position->y = v.y * radius;
      position->z = v.z * radius;
      break;
    case gx3d_PARTICLESYSTEM_EMITTER_TYPE_CONE:
      // Pick a random height
      height = random_GetFloat() * emitter->height;
      // Compute a random radius at that height
      radius = random_GetFloat() * (emitter->radius * (height / emitter->height));
      // Pick random angle
      angle = random_GetFloat() * 360;
      // Compute the cone point
      position->x = 0;
      position->y = 0;
      position->z = radius;
      gx3d_GetRotateYMatrix (&m, angle);
      gx3d_MultiplyVectorMatrix (position, &m, position);
      break;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_SetParticleSystemMatrix
|                                                                                        
| Output: Sets local transformation matrix for a particle system.
|___________________________________________________________________*/

void gx3d_SetParticleSystemMatrix (gx3dParticleSystem particle_system, gx3dMatrix *m)
{
  ParticleSystem *psys = (ParticleSystem *) particle_system;
  
  if (psys)
    // Is the new matrix different from the current local matrix?
//    if (memcmp ((void *)&(psys->local_matrix), (void *)m, sizeof(gx3dMatrix)) != 0) {
      // Set new local matrix
      memcpy ((void *)&(psys->local_matrix), (void *)m, sizeof(gx3dMatrix));
//      psys->local_matrix_dirty = TRUE;
//    }
}

/*____________________________________________________________________
|
| Function: gx3d_DrawParticleSystem
|                                                                                        
| Output: Renders particle system.
|___________________________________________________________________*/

void gx3d_DrawParticleSystem (gx3dParticleSystem particle_system, gx3dVector *view_normal, bool wireframe)
{
  int i, j, n, transparency, fill_mode;
  bool view_matrix_dirty;
  gx3dVector position;
  gx3dMatrix m, m_rotateparticle, m_world, m_view;
  gx3dObjectLayer *layer;
  Particle *particle;
  ParticleSystem *psys = (ParticleSystem *) particle_system;

  gx3dMaterialData material_save;
  gx3dMaterialData material_default = {
    { 1, 1, 1, 1 }, // ambient color
    { 1, 1, 1, 1 }, // diffuse color
    { 1, 1, 1, 1 }, // specular color
    { 0, 0, 0, 0 }, // emissive color
    0               // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
  };

  // Any 'live' particles?
  if (psys->tail != psys->head) {

/*____________________________________________________________________
|
| Init variables                                                                                       
|___________________________________________________________________*/

    layer = psys->object->layer;
    gx3d_GetViewMatrix (&m_view);
    
/*____________________________________________________________________
|
| Transform the base particle to point toward the camera
|___________________________________________________________________*/

    // Update saved view matrix if changed
    if (memcmp ((void *)&(psys->view_matrix), (void *)&m_view, sizeof(gx3dMatrix)) != 0) {
      // Set new view matrix
      memcpy ((void *)&(psys->view_matrix), (void *)&m_view, sizeof(gx3dMatrix));
      view_matrix_dirty = true;
    }
    else
      view_matrix_dirty = false;

    if (view_matrix_dirty) {
      // Compute the matrix to align particles to face the camera
      gx3d_GetBillboardRotateXYMatrix (&m_rotateparticle, &(psys->base_vertex_normal), view_normal);
      // Rotate the 'base' particle to face camera
      for (i=0; i<4; i++)
        gx3d_MultiplyVectorMatrix     (&(psys->base_vertex[i]),     &m_rotateparticle, &(psys->X_base_vertex[i]));
      gx3d_MultiplyNormalVectorMatrix (&(psys->base_vertex_normal), &m_rotateparticle, &(psys->X_base_vertex_normal));
//      psys->local_matrix_dirty = false;
    }

/*____________________________________________________________________
|
| Update geometry and rendering attributes of each live particle
|___________________________________________________________________*/

    for (i=psys->tail, n=0; i != psys->head; i=(i+1)%psys->num_particles, n++) {

      // Init variables
      particle = &(psys->particles[i]);
      transparency = (int)(particle->transparency * 255);
      if (transparency > 255)
        transparency = 255;

      // Is the particle attached to the emitter?
      if (psys->data.attached_particles)
        // Transform particle into world space (if not attached to emitter, the particle position is already in world space)
        gx3d_MultiplyVectorMatrix (&(particle->position), &(psys->local_matrix), &position);
      else 
        position = particle->position;
      // Expand particle into a quad of the correct size and position
      for (j=0; j<4; j++) {
        particle->vertex[j].x = psys->X_base_vertex[j].x * particle->size + position.x;
        particle->vertex[j].y = psys->X_base_vertex[j].y * particle->size + position.y;
        particle->vertex[j].z = psys->X_base_vertex[j].z * particle->size + position.z;
      }
      particle->vertex_normal = psys->X_base_vertex_normal;

      // Compute transformed center z value (for sorting) by transforming center of particle from world to view space
      particle->center_z = (position.x * psys->view_matrix._02) + (position.y * psys->view_matrix._12) + (position.z * psys->view_matrix._22) + psys->view_matrix._32;

      // Set draw order array
      psys->draw_order[n] = i;
    }

    // Sort polygon array back to front on z value
    particles_array = psys->particles;
    qsort (psys->draw_order, n, sizeof(int), Compare_Function);

/*____________________________________________________________________
|
| Render particles
|___________________________________________________________________*/

    // Set world transform
    gx3d_GetWorldMatrix (&m_world);
    gx3d_GetIdentityMatrix (&m);
    gx3d_SetWorldMatrix (&m);

    // Set material
    gx3d_GetMaterial (&material_save);
    gx3d_SetMaterial (&material_default);

    // Turn on wireframe rendering?
    if (wireframe) {
      fill_mode = gx3d_GetFillMode ();
      gx3d_SetFillMode (gx3d_FILL_MODE_WIREFRAME);
      gx3d_SetTextureColorOp (0, gx3d_TEXTURE_COLOROP_DISABLE, 0, 0);
      gx3d_SetTextureAlphaOp (0, gx3d_TEXTURE_ALPHAOP_DISABLE, 0, 0);
    }
    else {
      // Set render states
      gx3d_EnableAlphaBlending ();      
      gx3d_SetTextureColorOp (1, gx3d_TEXTURE_COLOROP_SELECTARG2, 0, gx3d_TEXTURE_ARG_CURRENT);
      gx3d_SetTextureAlphaOp (1, gx3d_TEXTURE_ALPHAOP_MODULATE, gx3d_TEXTURE_ARG_TFACTOR, gx3d_TEXTURE_ARG_CURRENT);
      gx3d_SetTextureAddressingMode (0, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_CLAMP);
      gx3d_SetTextureAddressingMode (1, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_CLAMP);
    }

    // Make 4 copies of the vertex normal of the first particle (all particles have the same vertex normal)
    gx3dVector vn[4];
    for (i=0; i<4; i++)
      vn[i] = psys->particles[psys->draw_order[0]].vertex_normal;

    // Draw all live particles
    for (i=0; i<n; i++) {
      j = psys->draw_order[i];
      // Set particle transparency
      transparency = (int)(psys->particles[j].transparency * 255);
      if (transparency > 255)
        transparency = 255;
      gx3d_SetTextureFactor (255, 255, 255, transparency);
      // Set particle geometry
      layer->X_vertex        = psys->particles[j].vertex;
      layer->X_vertex_normal = vn;
      // Draw a particle
      gx3d_DrawObject (psys->object, gx3d_DONT_SET_LOCAL_MATRIX);
    }
    layer->X_vertex        = NULL;
    layer->X_vertex_normal = NULL;

    // Turn off wireframe rendering?
    if (wireframe) {
      gx3d_SetFillMode (fill_mode);
      gx3d_SetTextureColorOp (0, gx3d_TEXTURE_COLOROP_MODULATE, gx3d_TEXTURE_ARG_TEXTURE, gx3d_TEXTURE_ARG_CURRENT);
      gx3d_SetTextureAlphaOp (0, gx3d_TEXTURE_ALPHAOP_SELECTARG1, gx3d_TEXTURE_ARG_TEXTURE, 0);
    }
    else {
      // Reset render states    
      gx3d_SetTextureAddressingMode (0, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_WRAP);
      gx3d_SetTextureAddressingMode (1, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_WRAP);
      gx3d_SetTextureColorOp (1, gx3d_TEXTURE_COLOROP_DISABLE, 0, 0);
      gx3d_SetTextureAlphaOp (1, gx3d_TEXTURE_ALPHAOP_DISABLE, 0, 0);
      gx3d_DisableAlphaBlending ();
    }

    // Reset material
    gx3d_SetMaterial (&material_save);

    // Reset world transform
    gx3d_SetWorldMatrix (&m_world);
  }
}

/*____________________________________________________________________
|
| Function: Compare_Function
|                                                                                        
| Input: Called from qsort() in gx3d_DrawParticleSystem()
| Output: Comparison function for qsort.
|___________________________________________________________________*/

static int Compare_Function (const void *elem1, const void *elem2) 
{
  float z1, z2;

  z1 = particles_array[*(int *)elem1].center_z;
  z2 = particles_array[*(int *)elem2].center_z;
  
  if (z1 < z2)
    return (1);
  else if (z1 > z2)
    return (-1);
  else
    return (0);
}

/*____________________________________________________________________
|
| File: snd8.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _SND8_H_
#define _SND8_H_

/*___________________
|
| Type definitions
|__________________*/

typedef void *Sound;

// Properties for sound effects (see DX8 doc. for usage)
typedef union {
  // Chorus
  struct {
		float fWetDryMix;
		float fDepth;
		float fFeedback;
		float fFrequency;
		long  lWaveform;
		float fDelay;
		long  lPhase;
  } chorus;
  // Compressor
  struct {
    float fGain;
    float fAttack;
    float fRelease;
    float fThreshold;
    float fRatio;
    float fPredelay;
  } compressor;
  // Distortion
  struct {
    float fGain;
    float fEdge;
    float fPostEQCenterFrequency;
    float fPostEQBandwidth;
    float fPreLowpassCutoff;
  } distortion;
  // Echo
  struct {
    float fWetDryMix;
    float fFeedback;
    float fLeftDelay;
    float fRightDelay;
    long  lPanDelay;
  } echo;
  // Flange
  struct {
    float fWetDryMix;
    float fDepth;
    float fFeedback;
    float fFrequency;
    long  lWaveform;
    float fDelay;
    long  lPhase;
  } flange;
  // Gargle
  struct {
    unsigned dwRateHz;
    unsigned dwWaveShape;
  } gargle;
  // Parametric Equalizer
  struct {
    float fCenter;
    float fBandwidth;
    float fGain;
  } param_eq;
  // Environmental Reverb
  struct {
		long  lRoom;
		long  lRoomHF; 
		float	flRoomRolloffFactor;
		float flDecayTime;
		float flDecayHFRatio;
		long  lReflections;
		float flReflectionsDelay;
		long  lReverb;
		float flReverbDelay; 
		float flDiffusion;
		float flDensity;
		float flHFReference;
  } env_reverb;
} sndEffectProperties;

/*___________________
|
| Constants
|__________________*/

// Time to apply 3d effect
#define snd_3D_APPLY_NOW            0
#define snd_3D_APPLY_DEFERRED       1

// 3D sound modes
#define snd_3D_MODE_DISABLE_3D      1    
#define snd_3D_MODE_HEAD_RELATIVE   2
#define snd_3D_MODE_ORIGIN_RELATIVE 3

/*___________________
|
| Controls
|__________________*/

#define snd_CONTROL_3D				0x1		// can't be combined with snd_ENABLE_PAN
#define snd_CONTROL_PAN				0x2		// can't be combined with snd_ENABLE_3D
#define snd_CONTROL_VOLUME		0x4	
#define snd_CONTROL_FREQUENCY	0x8		// can't be combined with snd_ENABLE_EFFECTS
#define snd_CONTROL_EFFECTS		0x10	// can't be combined with snd_ENABLE_FREQUENCY
#define snd_CONTROL_STREAMING	0x20

/*___________________
|
| Effects
|__________________*/

// Chorus is a voice doubling effect created by echoing the original sound with a slight delay and slightly
//  modulating the delay of the echo
#define snd_EFFECT_CHORUS								0x1

// Compression is a reduction in the fluctuation of a signal above a certain amplitude
#define snd_EFFECT_COMPRESSION					0x2

// Distortion is achieved by adding harmonics to the signal in such a way that, as the level increases, the top
//  of the waveform becomes squared off or clipped
#define snd_EFFECT_DISTORTION						0x4

// An echo effect causes an entire sound to be repeated after a fixed delay
#define snd_EFFECT_ECHO									0x8

// Flange is an echo effect in which the delay between the original signal and its echo is very short and varies
//  over time.  The result is sometimes referred to as a sweeping sound.
#define snd_EFFECT_FLANGE								0x10

// Gargle modulates the amplitude of the signal
#define snd_EFFECT_GARGLE								0x20

// A parametric equalizer amplifies or attenuates signals of a given frequency.  Parametric equalizer effects for
//  different pitches can be applied in parallel by setting multiple instances of the effect on the same buffer and
//  can provide the application with tone control similar to that provided by a hardware equalizer.  
#define snd_EFFECT_PARAMETRIC_EQUALIZER	0x40

// The waves reverb effect is intended for use with music.  It's based on Waves MaxxVerb technology which is
//  licensed to Microsoft.  Can only be set on buffers with a 16-bit format.
#define snd_EFFECT_WAVES_REVERB					0x80

// See other docs. Also, see environment presets below.
#define snd_EFFECT_ENVIRONMENTAL_REVERB	0x100

/*___________________
|
| Environments (presets for snd_EFFECT_ENVIRONMENTAL_REVERB)
|__________________*/

#define snd_ENVIRONMENT_DEFAULT					0
#define snd_ENVIRONMENT_GENERIC					1
#define snd_ENVIRONMENT_PADDEDCELL			2
#define snd_ENVIRONMENT_ROOM						3
#define snd_ENVIRONMENT_BATHROOM				4
#define snd_ENVIRONMENT_LIVINGROOM			5
#define snd_ENVIRONMENT_STONEROOM				6
#define snd_ENVIRONMENT_AUDITORIUM			7
#define snd_ENVIRONMENT_CONCERTHALL			8
#define snd_ENVIRONMENT_CAVE						9
#define snd_ENVIRONMENT_ARENA						10
#define snd_ENVIRONMENT_HANGAR					11
#define snd_ENVIRONMENT_CARPETEDHALLWAY	12
#define snd_ENVIRONMENT_HALLWAY					13
#define snd_ENVIRONMENT_STONECORRIDOR		14
#define snd_ENVIRONMENT_ALLEY						15
#define snd_ENVIRONMENT_FOREST					16
#define snd_ENVIRONMENT_CITY						17
#define snd_ENVIRONMENT_MOUNTAINS				18
#define snd_ENVIRONMENT_QUARRY				  19
#define snd_ENVIRONMENT_PLAIN						20
#define snd_ENVIRONMENT_PARKINGLOT			21
#define snd_ENVIRONMENT_SEWERPIPE				22
#define snd_ENVIRONMENT_UNDERWATER			23
#define snd_ENVIRONMENT_SMALLROOM				24
#define snd_ENVIRONMENT_MEDIUMROOM			25
#define snd_ENVIRONMENT_LARGEROOM				26
#define snd_ENVIRONMENT_MEDIUMHALL			27
#define snd_ENVIRONMENT_LARGEHALL				28
#define snd_ENVIRONMENT_PLATE						29

/*___________________
|
| Functions
|__________________*/

// Initialize the sound library
int snd_Init (
  int rate,                   // 0 (sound card default), 8, 11, 22, 44
  int bits,                   // 8, 16
  int channels,               // 1, 2
  int enable_3d,              // boolean
  int mute_background_apps ); // boolean

// Free all resources for the sound library
void snd_Free (void);

// Optimizes memory usage on the sound card
void snd_Optimize (void);

// Load a sound from a file
Sound snd_LoadSound (
  char		 *filename,       // wave file
	unsigned  controls,				// mask
  int				global_focus );	// boolean (if true, sound will play while app in background)

// Load a sound from a file
Sound snd_LoadSound (
  char     *filename,       
  unsigned  controls,       
  unsigned *num_samples,            
  unsigned *bits_per_sample,          
  unsigned *duration_milliseconds,
  byte    **data );   

// Free all resources for a sound
void snd_FreeSound (Sound s);

// Start playing a sound
void snd_PlaySound (Sound s, int repeat);

// Stop playing a sound
void snd_StopSound (Sound s);

// Pause a playing sound
void snd_PauseSound (Sound s);

// Unpause a paused sound
void snd_UnpauseSound (Sound s);

// Sets the volume for a sound (0=quiet, 100=normal)
void snd_SetSoundVolume (Sound s, int volume);

// Sets the pan for a sound (-10=left, 0=center, 10=right)
void snd_SetSoundPan (Sound s, int pan);

// Sets the frequency for a sound
void snd_SetSoundFrequency (Sound s, unsigned hertz);

// Sets the frequency for a sound to its original frequency
void snd_ResetSoundFrequency (Sound s);

// Returns the frequency of a sound in hertz
unsigned snd_GetSoundFrequency (Sound s);

// Returns true if a sound is playing
int snd_IsPlaying (Sound s);

/*___________________
|
| Effects Functions
|__________________*/

// Enable (or disable) effects for a sound
unsigned snd_EnableEffects (Sound s, unsigned effects);

// Sets the environmental reverb properties to a preset
void		 snd_SetEnvironment (Sound s, int environment_preset);

// Sets properties for an effect
void     snd_SetEffectProperties (Sound s, unsigned effect, sndEffectProperties *properties);

// Gets properties for an effect
void     snd_GetEffectProperties (Sound s, unsigned effect, sndEffectProperties *properties);

/*___________________
|
| 3D Sound Functions
|__________________*/

// Sets the 3d mode for a 3d-enabled sound
int snd_SetSoundMode (Sound s, int mode, int apply);

// Sets the position for a sound in 3-space relative to listener or origin depending on mode
int snd_SetSoundPosition (Sound s, float x, float y, float z, int apply);

// Sets the minimum distance the sound begins to decrease in volume.  Default is 1 units. (Default units is meters)
int snd_SetSoundMinDistance (Sound s, float distance, int apply);

// Sets the maximum distance the sound ceases to decrease in volume.  Default is 1,000,000 kilometers.
int snd_SetSoundMaxDistance (Sound s, float distance, int apply);

// Sets the orientation for a sound cone using a vector relative to listener or origin depending on mode.  
//  Has no effect unless the cone angle and cone volume factor have also been set.
int snd_SetSoundConeOrientation (Sound s, float x, float y, float z, int apply);

// Sets the inside and outside angles of the sound projection cone.  Sound will be full volume inside the angle 
//  and will be quiet at edge of outside angle with transition in between outside of inside angle and outside of 
//  outside angle.  Default for both is 360.
int snd_SetSoundConeAngles (Sound s, unsigned inside_angle, unsigned outside_angle, int apply);

// Sets the outside volume of sound cone.  Examples:
//  -1000 = sound attenuated by 10 decibles (sound quieter)
int snd_SetSoundConeOutsideVolume (Sound s, int volume, int apply);

// Sets the velocity vector for a sound.
int snd_SetSoundVelocity (Sound s, float x, float y, float z, int apply);

/*___________________
|
| 3D Listener Functions
|__________________*/

// Sets the current distance factor which is the number of meters per unit used by DirectSound (default is 1).
int snd_SetListenerDistanceFactor (float factor, int apply);

// Sets the distance factor to feet
int snd_SetListenerDistanceFactorToFeet (int apply);

// Sets rolloff for all 3D sounds from:
//   -10 = min rolloff - sound diminishes less in relation to distance
//     0 = normal rolloff (as in reality)
//    10 = max rolloff - sound diminishes more in relation to distance
int snd_SetListenerRolloff (int factor, int apply);

// Sets the velocity vector for the listener.
int snd_SetListenerVelocity (float x, float y, float z, int apply);

// Sets the position for the listener in 3-space distance units.
int snd_SetListenerPosition (float x, float y, float z, int apply);

// Sets the orientation for the listener in terms of a front and top vector.
int snd_SetListenerOrientation (float front_x, float front_y, float front_z, float up_x, float up_y, float up_z, int apply);

// Sets doppler factor for all 3D sounds from:
//   -10 = min - less than normal doppler effects
//     0 = normal (as in reality)
//    10 = max - more than normal doppler effects
int snd_SetListenerDopplerFactor (int factor, int apply);

// Commits deferred settings (if any)
int snd_Commit3DDeferredSettings (void);

#endif

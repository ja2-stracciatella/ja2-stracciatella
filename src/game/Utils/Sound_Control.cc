// MODULE FOR SOUND SYSTEM

#include "Directories.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "Overhead.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include <math.h>


#define SOUND_FAR_VOLUME_MOD 25

static UINT32 guiSpeechVolume       = MIDVOLUME;
static UINT32 guiSoundEffectsVolume = MIDVOLUME;

static char const* const szSoundEffects[NUM_SAMPLES] =
{
	SOUNDSDIR "/ricochet 01.wav",                               // 0
	SOUNDSDIR "/ricochet 02.wav",                               // 1
	SOUNDSDIR "/ricochet 01.wav",                               // 2
	SOUNDSDIR "/ricochet 02.wav",                               // 3
	SOUNDSDIR "/ricochet 01.wav",                               // 4
	SOUNDSDIR "/ricochet 02.wav",                               // 5
	SOUNDSDIR "/ricochet 01.wav",                               // 6
	SOUNDSDIR "/ricochet 02.wav",                               // 7
	SOUNDSDIR "/dirt impact 01.wav",                            // 8
	SOUNDSDIR "/dirt impact 01.wav",                            // 9
	SOUNDSDIR "/knife hit ground.wav",                          // 10
	SOUNDSDIR "/fall to knees 01.wav",                          // 11
	SOUNDSDIR "/fall to knees 02.wav",                          // 12
	SOUNDSDIR "/knees to dirt 01.wav",                          // 13
	SOUNDSDIR "/knees to dirt 02.wav",                          // 14
	SOUNDSDIR "/knees to dirt 03.wav",                          // 15
	SOUNDSDIR "/heavy fall 01.wav",                             // 16
	SOUNDSDIR "/body_splat.wav",                                // 17
	SOUNDSDIR "/glass_break1.wav",                              // 18
	SOUNDSDIR "/glass_break2.wav",                              // 19
	SOUNDSDIR "/door open 01.wav",                              // 20
	SOUNDSDIR "/door open 02.wav",                              // 21
	SOUNDSDIR "/door open 03.wav",                              // 22
	SOUNDSDIR "/door close 01.wav",                             // 23
	SOUNDSDIR "/door close 02.wav",                             // 24
	SOUNDSDIR "/unlock lock.wav",                               // 25
	SOUNDSDIR "/kickin lock.wav",                               // 26
	SOUNDSDIR "/break lock.wav",                                // 27
	SOUNDSDIR "/picking lock.wav",                              // 28
	SOUNDSDIR "/garage door open.wav",                          // 29
	SOUNDSDIR "/garage door close.wav",                         // 30
	NULL,                                                       // 31
	NULL,                                                       // 32
	NULL,                                                       // 33
	NULL,                                                       // 34
	SOUNDSDIR "/curtains door open.wav",                        // 35
	SOUNDSDIR "/curtains door close.wav",                       // 36
	SOUNDSDIR "/metal door open.wav",                           // 37
	SOUNDSDIR "/metal door close.wav",                          // 38
	SOUNDSDIR "/ftp gravel 01.wav",                             // 39
	SOUNDSDIR "/ftp gravel 02.wav",                             // 40
	SOUNDSDIR "/ftp gravel 03.wav",                             // 41
	SOUNDSDIR "/ftp gravel 04.wav",                             // 42
	SOUNDSDIR "/ftp gritty 01.wav",                             // 43
	SOUNDSDIR "/ftp gritty 02.wav",                             // 44
	SOUNDSDIR "/ftp gritty 03.wav",                             // 45
	SOUNDSDIR "/ftp gritty 04.wav",                             // 46
	SOUNDSDIR "/ftp leaves 01.wav",                             // 47
	SOUNDSDIR "/ftp leaves 02.wav",                             // 48
	SOUNDSDIR "/ftp leaves 03.wav",                             // 49
	SOUNDSDIR "/ftp leaves 04.wav",                             // 50
	SOUNDSDIR "/crawling 01.wav",                               // 51
	SOUNDSDIR "/crawling 02.wav",                               // 52
	SOUNDSDIR "/crawling 03.wav",                               // 53
	SOUNDSDIR "/crawling 04.wav",                               // 54
	SOUNDSDIR "/beep2.wav",                                     // 55
	SOUNDSDIR "/endturn.wav",                                   // 56
	SOUNDSDIR "/ja2 death hit.wav",                             // 57
	SOUNDSDIR "/doorcr.wav",                                    // 58
	SOUNDSDIR "/head exploding 01.wav",                         // 59
	SOUNDSDIR "/body exploding.wav",                            // 60
	SOUNDSDIR "/explode1.wav",                                  // 61
	SOUNDSDIR "/crow exploding.wav",                            // 62
	SOUNDSDIR "/small explosion 01.wav",                        // 63
	SOUNDSDIR "/heli1.wav",                                     // 64
	SOUNDSDIR "/bullet impact 01.wav",                          // 65
	SOUNDSDIR "/bullet impact 02.wav",                          // 66
	SOUNDSDIR "/bullet impact 02.wav",                          // 67
	STSOUNDSDIR "/blah.wav",                                    // 68  URE ATTACK
	SOUNDSDIR "/step into water.wav",                           // 69
	SOUNDSDIR "/splash from shallow to deep.wav",               // 70
	SOUNDSDIR "/cow hit.wav",                                   // 71  COW HIT
	SOUNDSDIR "/cow die.wav",                                   // 72  COW DIE
	SOUNDSDIR "/line 02 fx.wav",                                // 73
	SOUNDSDIR "/line 01 fx.wav",                                // 74
	SOUNDSDIR "/line 03 fx.wav",                                // 75
	SOUNDSDIR "/cave collapsing.wav",                           // 76  CAVE_COLLAPSE
	SOUNDSDIR "/raid whistle.wav",                              // 77  RAID
	SOUNDSDIR "/raid ambient.wav",                              // 78
	SOUNDSDIR "/raid dive.wav",                                 // 79
	SOUNDSDIR "/raid dive.wav",                                 // 80
	SOUNDSDIR "/raid whistle.wav",                              // 81  RAID
	SOUNDSDIR "/driving 01.wav",                                // 82  DRIVING
	SOUNDSDIR "/engine start.wav",                              // 83  ON
	SOUNDSDIR "/engine off.wav",                                // 84  OFF
	SOUNDSDIR "/into vehicle.wav",                              // 85  INTO
	SOUNDSDIR "/weapons/dry fire 1.wav",                        // 86  Dry fire sound ( for gun jam )
	SOUNDSDIR "/wood impact 01a.wav",                           // 87  S_WOOD_IMPACT1
	SOUNDSDIR "/wood impact 01b.wav",                           // 88
	SOUNDSDIR "/wood impact 01a.wav",                           // 89
	SOUNDSDIR "/porcelain impact.wav",                          // 90
	SOUNDSDIR "/tire impact 01.wav",                            // 91
	SOUNDSDIR "/stone impact 01.wav",                           // 92
	SOUNDSDIR "/water impact 01.wav",                           // 93
	SOUNDSDIR "/veg impact 01.wav",                             // 94
	SOUNDSDIR "/metal hit 01.wav",                              // 95  S_METAL_HIT1
	SOUNDSDIR "/metal hit 01.wav",                              // 96
	SOUNDSDIR "/metal hit 01.wav",                              // 97
	SOUNDSDIR "/slap_impact.wav",                               // 98
	SOUNDSDIR "/weapons/revolver reload.wav",                   // 99  REVOLVER
	SOUNDSDIR "/weapons/pistol reload.wav",                     // 100  PISTOL
	SOUNDSDIR "/weapons/smg reload.wav",                        // 101  SMG
	SOUNDSDIR "/weapons/rifle reload.wav",                      // 102  RIFLE
	SOUNDSDIR "/weapons/shotgun reload.wav",                    // 103  SHOTGUN
	SOUNDSDIR "/weapons/lmg reload.wav",                        // 104  LMG
	SOUNDSDIR "/weapons/revolver lnl.wav",                      // 105  REVOLVER
	SOUNDSDIR "/weapons/pistol lnl.wav",                        // 106  PISTOL
	SOUNDSDIR "/weapons/smg lnl.wav",                           // 107  SMG
	SOUNDSDIR "/weapons/rifle lnl.wav",                         // 108  RIFLE
	SOUNDSDIR "/weapons/shotgun lnl.wav",                       // 109  SHOTGUN
	SOUNDSDIR "/weapons/lmg lnl.wav",                           // 110  LMG
	NULL,                                                       // 111
	NULL,                                                       // 112
	NULL,                                                       // 113
	NULL,                                                       // 114
	NULL,                                                       // 115
	NULL,                                                       // 116
	NULL,                                                       // 117
	NULL,                                                       // 118
	NULL,                                                       // 119
	NULL,                                                       // 120
	NULL,                                                       // 121
	NULL,                                                       // 122
	NULL,                                                       // 123
	NULL,                                                       // 124
	NULL,                                                       // 125
	NULL,                                                       // 126
	NULL,                                                       // 127
	NULL,                                                       // 128
	NULL,                                                       // 129
	NULL,                                                       // 130
	NULL,                                                       // 131
	NULL,                                                       // 132
	NULL,                                                       // 133
	NULL,                                                       // 134
	NULL,                                                       // 135
	NULL,                                                       // 136
	NULL,                                                       // 137
	NULL,                                                       // 138
	NULL,                                                       // 139
	NULL,                                                       // 140
	NULL,                                                       // 141
	NULL,                                                       // 142
	NULL,                                                       // 143
	NULL,                                                       // 144
	NULL,                                                       // 145
	NULL,                                                       // 146
	NULL,                                                       // 147
	NULL,                                                       // 148
	NULL,                                                       // 149
	NULL,                                                       // 150
	NULL,                                                       // 151
	NULL,                                                       // 152
	NULL,                                                       // 153
	NULL,                                                       // 154
	NULL,                                                       // 155
	SOUNDSDIR "/weapons/silencer 02.wav",                       // 156
	SOUNDSDIR "/weapons/silencer 03.wav",                       // 157
	SOUNDSDIR "/swoosh 01.wav",                                 // 158
	SOUNDSDIR "/swoosh 03.wav",                                 // 159
	SOUNDSDIR "/swoosh 05.wav",                                 // 160
	SOUNDSDIR "/swoosh 06.wav",                                 // 161
	SOUNDSDIR "/swoosh 11.wav",                                 // 162
	SOUNDSDIR "/swoosh 14.wav",                                 // 163
	SOUNDSDIR "/adult fall 01.wav",                             // 164
	SOUNDSDIR "/adult step 01.wav",                             // 165
	SOUNDSDIR "/adult step 02.wav",                             // 166
	SOUNDSDIR "/adult swipe 01.wav",                            // 167
	SOUNDSDIR "/eating_flesh 01.wav",                           // 168
	SOUNDSDIR "/adult crippled.wav",                            // 169
	SOUNDSDIR "/adult dying part 1.wav",                        // 170
	SOUNDSDIR "/adult dying part 2.wav",                        // 171
	SOUNDSDIR "/adult lunge 01.wav",                            // 172
	SOUNDSDIR "/adult smells threat.wav",                       // 173
	SOUNDSDIR "/adult smells prey.wav",                         // 174
	SOUNDSDIR "/adult spit.wav",                                // 175
	SOUNDSDIR "/baby dying 01.wav",                             // 176
	SOUNDSDIR "/baby dragging 01.wav",                          // 177
	SOUNDSDIR "/baby shriek 01.wav",                            // 178
	NULL,                                                       // 179
	SOUNDSDIR "/larvae movement 01.wav",                        // 180
	SOUNDSDIR "/larvae rupture 01.wav",                         // 181
	SOUNDSDIR "/queen shriek 01.wav",                           // 182
	SOUNDSDIR "/queen dying 01.wav",                            // 183
	SOUNDSDIR "/queen enraged attack.wav",                      // 184
	SOUNDSDIR "/queen rupturing.wav",                           // 185
	SOUNDSDIR "/queen crippled.wav",                            // 186
	SOUNDSDIR "/queen smells threat.wav",                       // 187
	SOUNDSDIR "/queen whip attack.wav",                         // 188
	SOUNDSDIR "/rock hit 01.wav",                               // 189
	SOUNDSDIR "/rock hit 02.wav",                               // 190
	SOUNDSDIR "/scratch.wav",                                   // 191
	SOUNDSDIR "/armpit.wav",                                    // 192
	SOUNDSDIR "/cracking back.wav",                             // 193
	SOUNDSDIR "/weapons/auto resolve composite 02 (8-22).wav",  // 194  The FF sound in autoresolve interface
	SOUNDSDIR "/email alert 01.wav",                            // 195
	SOUNDSDIR "/entering text 02.wav",                          // 196
	SOUNDSDIR "/removing text 02.wav",                          // 197
	SOUNDSDIR "/computer beep 01 in.wav",                       // 198
	SOUNDSDIR "/computer beep 01 out.wav",                      // 199
	SOUNDSDIR "/computer switch 01 in.wav",                     // 200
	SOUNDSDIR "/computer switch 01 out.wav",                    // 201
	SOUNDSDIR "/very small switch 01 in.wav",                   // 202
	SOUNDSDIR "/very small switch 01 out.wav",                  // 203
	SOUNDSDIR "/very small switch 02 in.wav",                   // 204
	SOUNDSDIR "/very small switch 02 out.wav",                  // 205
	SOUNDSDIR "/small switch 01 in.wav",                        // 206
	SOUNDSDIR "/small switch 01 out.wav",                       // 207
	SOUNDSDIR "/small switch 02 in.wav",                        // 208
	SOUNDSDIR "/small switch 02 out.wav",                       // 209
	SOUNDSDIR "/small switch 03 in.wav",                        // 210
	SOUNDSDIR "/small switch 03 out.wav",                       // 211
	SOUNDSDIR "/big switch 03 in.wav",                          // 212
	SOUNDSDIR "/big switch 03 out.wav",                         // 213
	SOUNDSDIR "/alarm.wav",                                     // 214
	SOUNDSDIR "/fight bell.wav",                                // 215
	NULL,                                                       // 216
	SOUNDSDIR "/attachment.wav",                                // 217
	SOUNDSDIR "/ceramic armor insert.wav",                      // 218
	SOUNDSDIR "/detonator beep.wav",                            // 219
	SOUNDSDIR "/grab roof.wav",                                 // 220
	SOUNDSDIR "/land on roof.wav",                              // 221
	NULL,                                                       // 222
	NULL,                                                       // 223
	NULL,                                                       // 224
	SOUNDSDIR "/fridge door open.wav",                          // 225
	SOUNDSDIR "/fridge door close.wav",                         // 226
	SOUNDSDIR "/fire 03 loop.wav",                              // 227
	SOUNDSDIR "/glass_crack.wav",                               // 228
	SOUNDSDIR "/spit ricochet.wav",                             // 229
	SOUNDSDIR "/tiger hit.wav",                                 // 230
	SOUNDSDIR "/bloodcat dying 02.wav",                         // 231
	SOUNDSDIR "/slap.wav",                                      // 232
	SOUNDSDIR "/robot beep.wav",                                // 233
	SOUNDSDIR "/electricity.wav",                               // 234
	SOUNDSDIR "/swimming 01.wav",                               // 235
	SOUNDSDIR "/swimming 02.wav",                               // 236
	SOUNDSDIR "/key failure.wav",                               // 237
	SOUNDSDIR "/target cursor.wav",                             // 238
	SOUNDSDIR "/statue open.wav",                               // 239
	SOUNDSDIR "/remote activate.wav",                           // 240
	SOUNDSDIR "/wirecutters.wav",                               // 241
	SOUNDSDIR "/drink from canteen.wav",                        // 242
	SOUNDSDIR "/bloodcat attack.wav",                           // 243
	SOUNDSDIR "/bloodcat loud roar.wav",                        // 244
	SOUNDSDIR "/robot greeting.wav",                            // 245
	SOUNDSDIR "/robot death.wav",                               // 246
	SOUNDSDIR "/gas grenade explode.wav",                       // 247
	SOUNDSDIR "/air escaping.wav",                              // 248
	SOUNDSDIR "/drawer open.wav",                               // 249
	SOUNDSDIR "/drawer close.wav",                              // 250
	SOUNDSDIR "/locker door open.wav",                          // 251
	SOUNDSDIR "/locker door close.wav",                         // 252
	SOUNDSDIR "/wooden box open.wav",                           // 253
	SOUNDSDIR "/wooden box close.wav",                          // 254
	SOUNDSDIR "/robot stop moving.wav",                         // 255
	SOUNDSDIR "/water movement 01.wav",                         // 256
	SOUNDSDIR "/water movement 02.wav",                         // 257
	SOUNDSDIR "/water movement 03.wav",                         // 258
	SOUNDSDIR "/water movement 04.wav",                         // 259
	SOUNDSDIR "/prone to crouch.wav",                           // 260
	SOUNDSDIR "/crouch to prone.wav",                           // 261
	SOUNDSDIR "/crouch to stand.wav",                           // 262
	SOUNDSDIR "/stand to crouch.wav",                           // 263
	SOUNDSDIR "/picking something up.wav",                      // 264
	SOUNDSDIR "/cow falling.wav",                               // 265
	SOUNDSDIR "/bloodcat_growl_01.wav",                         // 266
	SOUNDSDIR "/bloodcat_growl_02.wav",                         // 267
	SOUNDSDIR "/bloodcat_growl_03.wav",                         // 268
	SOUNDSDIR "/bloodcat_growl_04.wav",                         // 269
	SOUNDSDIR "/spit ricochet.wav",                             // 270
	SOUNDSDIR "/adult crippled.wav",                            // 271
	SOUNDSDIR "/death disintegration.wav",                      // 272
	SOUNDSDIR "/queen ambience.wav",                            // 273
	SOUNDSDIR "/alien impact.wav",                              // 274
	SOUNDSDIR "/crow pecking flesh 01.wav",                     // 275
	SOUNDSDIR "/crow fly.wav",                                  // 276
	SOUNDSDIR "/slap 02.wav",                                   // 277
	SOUNDSDIR "/setting up mortar.wav",                         // 278
	SOUNDSDIR "/mortar whistle.wav",                            // 279
	SOUNDSDIR "/load mortar.wav",                               // 280
	SOUNDSDIR "/tank turret a.wav",                             // 281
	SOUNDSDIR "/tank turret b.wav",                             // 282
	SOUNDSDIR "/cow falling b.wav",                             // 283
	SOUNDSDIR "/stab into flesh.wav",                           // 284
	SOUNDSDIR "/explosion 10.wav",                              // 285
	SOUNDSDIR "/explosion 12.wav",                              // 286
	SOUNDSDIR "/drink from canteen male.wav",                   // 287
	SOUNDSDIR "/x ray activated.wav",                           // 288
	SOUNDSDIR "/catch object.wav",                              // 289
	SOUNDSDIR "/fence open.wav",                                // 290
};

static char const* const szAmbientEffects[NUM_AMBIENTS] =
{
	SOUNDSDIR "/storm1.wav",
	SOUNDSDIR "/storm2.wav",
	SOUNDSDIR "/rain_loop_22k.wav",
	SOUNDSDIR "/bird1-22k.wav",
	SOUNDSDIR "/bird3-22k.wav",
	SOUNDSDIR "/crickety_loop.wav",
	SOUNDSDIR "/crickety_loop2.wav",
	SOUNDSDIR "/cricket1.wav",
	SOUNDSDIR "/cricket2.wav",
	SOUNDSDIR "/owl1.wav",
	SOUNDSDIR "/owl2.wav",
	SOUNDSDIR "/owl3.wav",
	SOUNDSDIR "/night_bird1.wav",
	SOUNDSDIR "/night_bird3.wav"
};

void ShutdownJA2Sound(void)
{
	SoundStopAll();
}


const char * getSoundSample(SoundID soundId)
{
	if(soundId == -1) {
		return "";
	}
	else
	{
		return szSoundEffects[soundId];
	}
}

UINT32 PlayJA2Sample(SoundID const usNum, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan)
{
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szSoundEffects[usNum], vol, uiPan, ubLoops, NULL, NULL);
}


UINT32 PlayJA2Sample(const char *sample, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan)
{
	if((sample != NULL) && strcmp(sample, ""))
	{
		UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
		return SoundPlay(sample, vol, uiPan, ubLoops, NULL, NULL);
	}
	return SOUND_ERROR;
}


UINT32 PlayJA2StreamingSample(SoundID const usNum, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan)
{
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szSoundEffects[usNum], vol, uiPan, ubLoops, NULL, NULL);
}


UINT32 PlayJA2SampleFromFile(char const* const szFileName, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan)
{
	// does the same thing as PlayJA2Sound, but one only has to pass the filename, not the index of the sound array
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szFileName, vol, uiPan, ubLoops, NULL, NULL);
}


UINT32 PlayJA2StreamingSampleFromFile(char const* const szFileName, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan, SOUND_STOP_CALLBACK const EndsCallback)
{
	// does the same thing as PlayJA2Sound, but one only has to pass the filename, not the index of the sound array
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szFileName, vol, uiPan, ubLoops, EndsCallback, NULL);
}


UINT32 PlayJA2Ambient(AmbientSoundID const usNum, UINT32 const ubVolume, UINT32 const ubLoops)
{
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szAmbientEffects[usNum], vol, MIDDLEPAN, ubLoops, NULL, NULL);
}

UINT32 PlayLocationJA2SampleFromFile(UINT16 const grid_no, char const* const filename, UINT32 const base_vol, UINT32 const loops)
{
	UINT32 const vol = SoundVolume(base_vol, grid_no);
	UINT32 const pan = SoundDir(grid_no);
	return PlayJA2SampleFromFile(filename, vol, loops, pan);
}


UINT32 PlayLocationJA2Sample(UINT16 const grid_no, SoundID const idx, UINT32 const base_vol, UINT32 const loops)
{
	UINT32 const vol = SoundVolume(base_vol, grid_no);
	UINT32 const pan = SoundDir(grid_no);
	return PlayJA2Sample(idx, vol, loops, pan);
}


UINT32 PlayLocationJA2Sample(UINT16 const grid_no, const ST::string &sample, UINT32 const base_vol, UINT32 const loops)
{
	UINT32 const vol = SoundVolume(base_vol, grid_no);
	UINT32 const pan = SoundDir(grid_no);
	return PlayJA2Sample(sample.c_str(), vol, loops, pan);
}


UINT32 PlayLocationJA2StreamingSample(UINT16 const grid_no, SoundID const idx, UINT32 const base_vol, UINT32 const loops)
{
	UINT32 const vol = SoundVolume(base_vol, grid_no);
	UINT32 const pan = SoundDir(grid_no);
	return PlayJA2StreamingSample(idx, vol, loops, pan);
}


UINT32 PlaySoldierJA2Sample(SOLDIERTYPE const* const s, SoundID const usNum, UINT32 const base_vol, UINT32 const ubLoops, BOOLEAN const fCheck)
{
	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// CHECK IF GUY IS ON SCREEN BEFORE PLAYING!
		if (s->bVisible != -1 || !fCheck)
		{
			UINT32 const vol = SoundVolume(base_vol, s->sGridNo);
			UINT32 const pan = SoundDir(s->sGridNo);
			return PlayJA2Sample(usNum, CalculateSoundEffectsVolume(vol), ubLoops, pan);
		}
	}

	return( 0 );
}


void SetSpeechVolume(UINT32 uiNewVolume)
{
	guiSpeechVolume = std::min(uiNewVolume, UINT32(MAXVOLUME));
}


UINT32 GetSpeechVolume(void)
{
	return( guiSpeechVolume );
}


void SetSoundEffectsVolume(UINT32 uiNewVolume)
{
	guiSoundEffectsVolume = std::min(uiNewVolume, UINT32(MAXVOLUME));
}


UINT32 GetSoundEffectsVolume(void)
{
	return( guiSoundEffectsVolume );
}


UINT32 CalculateSpeechVolume(UINT32 uiVolume)
{
	return (uiVolume * guiSpeechVolume + HIGHVOLUME / 2) / HIGHVOLUME;
}


UINT32 CalculateSoundEffectsVolume(UINT32 uiVolume)
{
	return (uiVolume * guiSoundEffectsVolume + HIGHVOLUME / 2) / HIGHVOLUME;
}


#if 0
	int x,dif,absDif;

	// This function calculates the general LEFT / RIGHT direction of a gridno
	// based on the middle of your screen.

	x = Gridx(gridno);

	dif = ScreenMiddleX - x;

	if ( (absDif=abs(dif)) > 32)
	{
		// OK, NOT the middle.

		// Is it outside the screen?
		if (absDif > HalfWindowWidth)
		{
			// yes, outside...
			if (dif > 0)
				return(25);
			else
				return(102);
		}
	}
	else // inside screen
	{
		if (dif > 0)
			return(LEFTSIDE);
		else
			return(RIGHTSIDE);
	}
	else // hardly any difference, so sound should be played from middle
		return(MIDDLE);

}
#endif

INT8 SoundDir( INT16 sGridNo )
{
	INT16 sScreenX, sScreenY;
	INT16	sMiddleX;
	INT16	sDif, sAbsDif;

	if ( sGridNo == NOWHERE )
	{
		return( MIDDLEPAN );
	}

	GetAbsoluteScreenXYFromMapPos(sGridNo, &sScreenX, &sScreenY);

	// Get middle of where we are now....
	sMiddleX = gsTopLeftWorldX + ( gsBottomRightWorldX - gsTopLeftWorldX ) / 2;

	sDif = sMiddleX - sScreenX;

	if ((sAbsDif = std::abs(sDif)) > 64)
	{
		// OK, NOT the middle.

		// Is it outside the screen?
		if ( sAbsDif > ( ( gsBottomRightWorldX - gsTopLeftWorldX ) / 2 ) )
		{
			// yes, outside...
			if ( sDif > 0 )
			{
				//return( FARLEFT );
				return( 1 );
			}
			else
			{
				//return( FARRIGHT );
				return( 126 );
			}
		}
		else // inside screen
		{
			if ( sDif > 0)
			{
				return( LEFTSIDE );
			}
			else
			{
				return( RIGHTSIDE );
			}
		}
	}
	else // hardly any difference, so sound should be played from middle
	{
		return(MIDDLE);
	}
}


INT8 SoundVolume( INT8 bInitialVolume, INT16 sGridNo )
{
	INT16 sScreenX, sScreenY;
	INT16	sMiddleX, sMiddleY;
	INT16	sDifX, sAbsDifX;
	INT16	sDifY, sAbsDifY;

	if ( sGridNo == NOWHERE )
	{
		return( bInitialVolume );
	}

	GetAbsoluteScreenXYFromMapPos(sGridNo, &sScreenX, &sScreenY);

	// Get middle of where we are now....
	sMiddleX = gsTopLeftWorldX + ( gsBottomRightWorldX - gsTopLeftWorldX ) / 2;
	sMiddleY = gsTopLeftWorldY + ( gsBottomRightWorldY - gsTopLeftWorldY ) / 2;

	sDifX = sMiddleX - sScreenX;
	sDifY = sMiddleY - sScreenY;

	sAbsDifX = std::abs(sDifX);
	sAbsDifY = std::abs(sDifY);

	if ( sAbsDifX  > 64 || sAbsDifY > 64 )
	{
		// OK, NOT the middle.

		// Is it outside the screen?
		if ( sAbsDifX > ( ( gsBottomRightWorldX - gsTopLeftWorldX ) / 2 ) ||
			sAbsDifY > ( ( gsBottomRightWorldY - gsTopLeftWorldY ) / 2 ) )
		{
			return( std::max(LOWVOLUME, ( bInitialVolume - SOUND_FAR_VOLUME_MOD ) ));
		}
	}

	return( bInitialVolume );
}


/////////////////////////////////////////////////////////
/////////
/////////
/////////////////////////////////////////////////////////
// Positional Ambients
/////////////////////////////////////////////////////////
#define NUM_POSITION_SOUND_EFFECT_SLOTS 10

struct POSITIONSND
{
	INT16         sGridNo;
	UINT32        uiSoundSampleID;
	SoundID       iSoundToPlay;
	const SOLDIERTYPE* SoundSource;
	BOOLEAN       fAllocated;
	BOOLEAN       fInActive;
};


// GLOBAL FOR SMOKE LISTING
static POSITIONSND gPositionSndData[NUM_POSITION_SOUND_EFFECT_SLOTS];
static UINT32      guiNumPositionSnds     = 0;
static BOOLEAN     gfPositionSoundsActive = FALSE;


static INT32 GetFreePositionSnd(void)
{
	for (UINT32 i = 0; i != guiNumPositionSnds; ++i)
	{
		if (!gPositionSndData[i].fAllocated) return (INT32)i;
	}

	if (guiNumPositionSnds < NUM_POSITION_SOUND_EFFECT_SLOTS)
		return (INT32)guiNumPositionSnds++;

	return -1;
}


static void RecountPositionSnds(void)
{
	INT32 uiCount;

	for(uiCount=guiNumPositionSnds-1; (uiCount >=0) ; uiCount--)
	{
		if( ( gPositionSndData[uiCount].fAllocated ) )
		{
			guiNumPositionSnds=(UINT32)(uiCount+1);
			break;
		}
	}
}


INT32 NewPositionSnd(INT16 const sGridNo, SOLDIERTYPE const* const SoundSource, SoundID const iSoundToPlay)
{
	INT32 const idx = GetFreePositionSnd();
	if (idx == -1) return -1;

	POSITIONSND& p = gPositionSndData[idx];
	p = POSITIONSND{};
	p.fInActive      = !gfPositionSoundsActive;
	p.sGridNo        = sGridNo;
	p.SoundSource    = SoundSource;
	p.fAllocated     = TRUE;
	p.iSoundToPlay   = iSoundToPlay;
	p.uiSoundSampleID = NO_SAMPLE;

	return idx;
}

void DeletePositionSnd( INT32 iPositionSndIndex )
{
	POSITIONSND *pPositionSnd;

	pPositionSnd = &gPositionSndData[ iPositionSndIndex ];

	if ( pPositionSnd->fAllocated )
	{
		// Turn inactive first...
		pPositionSnd->fInActive = TRUE;

		// End sound...
		if ( pPositionSnd->uiSoundSampleID != NO_SAMPLE )
		{
			SoundStop( pPositionSnd->uiSoundSampleID );
		}

		pPositionSnd->fAllocated = FALSE;

		RecountPositionSnds( );
	}
}

void SetPositionSndGridNo( INT32 iPositionSndIndex, INT16 sGridNo )
{
	POSITIONSND *pPositionSnd;

	pPositionSnd = &gPositionSndData[ iPositionSndIndex ];

	if ( pPositionSnd->fAllocated )
	{
		pPositionSnd->sGridNo = sGridNo;

		SetPositionSndsVolumeAndPanning( );
	}
}


void SetPositionSndsActive(void)
{
	gfPositionSoundsActive = TRUE;
	for (UINT32 i = 0; i != guiNumPositionSnds; ++i)
	{
		POSITIONSND& p = gPositionSndData[i];
		if (!p.fAllocated) continue;
		if (!p.fInActive)  continue;

		p.fInActive      = FALSE;
		// Begin sound effect, Volume 0
		p.uiSoundSampleID = PlayJA2Sample(p.iSoundToPlay, 0, 0, MIDDLEPAN);
	}
}


void SetPositionSndsInActive(void)
{
	gfPositionSoundsActive = FALSE;
	for (UINT32 i = 0; i != guiNumPositionSnds; ++i)
	{
		POSITIONSND& p = gPositionSndData[i];
		if (!p.fAllocated) continue;

		p.fInActive = TRUE;

		if (p.uiSoundSampleID == NO_SAMPLE) continue;

		// End sound
		SoundStop(p.uiSoundSampleID);
		p.uiSoundSampleID = NO_SAMPLE;
	}
}


static INT8 PositionSoundDir(INT16 sGridNo)
{
	INT16 sScreenX, sScreenY;
	INT16	sMiddleX;
	INT16	sDif, sAbsDif;

	if ( sGridNo == NOWHERE )
	{
		return( MIDDLEPAN );
	}

	GetAbsoluteScreenXYFromMapPos(sGridNo, &sScreenX, &sScreenY);

	// Get middle of where we are now....
	sMiddleX = gsTopLeftWorldX + ( gsBottomRightWorldX - gsTopLeftWorldX ) / 2;

	sDif = sMiddleX - sScreenX;

	if ((sAbsDif = std::abs(sDif)) > 64)
	{
		// OK, NOT the middle.

		// Is it outside the screen?
		if ( sAbsDif > ( ( gsBottomRightWorldX - gsTopLeftWorldX ) / 2 ) )
		{
			// yes, outside...
			if ( sDif > 0 )
			{
				//return( FARLEFT );
				return( 1 );
			}
			else
			{
				//return( FARRIGHT );
				return( 126 );
			}
		}
		else // inside screen
		{
			if (sDif > 0)
			{
				return( LEFTSIDE );
			}
			else
			{
				return( RIGHTSIDE );
			}
		}
	}
	else // hardly any difference, so sound should be played from middle
	{
		return(MIDDLE);
	}
}


static INT8 PositionSoundVolume(INT8 const initial_volume, GridNo const grid_no)
{
	if (grid_no == NOWHERE) return initial_volume;

	INT16 sScreenX;
	INT16 sScreenY;
	GetAbsoluteScreenXYFromMapPos(grid_no, &sScreenX, &sScreenY);

	// Get middle of where we are now
	INT16 const sMiddleX = gsTopLeftWorldX + (gsBottomRightWorldX - gsTopLeftWorldX) / 2;
	INT16 const sMiddleY = gsTopLeftWorldY + (gsBottomRightWorldY - gsTopLeftWorldY) / 2;

	INT16 const sDifX = sMiddleX - sScreenX;
	INT16 const sDifY = sMiddleY - sScreenY;

	INT16 const sMaxDistX = (gsBottomRightWorldX - gsTopLeftWorldX) * 3 / 2;
	INT16 const sMaxDistY = (gsBottomRightWorldY - gsTopLeftWorldY) * 3 / 2;

	double const sMaxSoundDist = std::hypot(sMaxDistX, sMaxDistY);
	double       sSoundDist    = std::hypot(sDifX, sDifY);

	if (sSoundDist == 0) return initial_volume;

	if (sSoundDist > sMaxSoundDist) sSoundDist = sMaxSoundDist;

	// Scale
	return  (INT8)(initial_volume * ((sMaxSoundDist - sSoundDist) / sMaxSoundDist));
}


void SetPositionSndsVolumeAndPanning(void)
{
	for (UINT32 i = 0; i != guiNumPositionSnds; ++i)
	{
		POSITIONSND const& p = gPositionSndData[i];
		if (!p.fAllocated)                 continue;
		if (p.fInActive)                   continue;
		if (p.uiSoundSampleID == NO_SAMPLE) continue;

		INT8 volume = PositionSoundVolume(15, p.sGridNo);
		if (p.SoundSource && p.SoundSource->bVisible == -1)
		{ // Limit volume
			if (volume > 10) volume = 10;
		}
		SoundSetVolume(p.uiSoundSampleID, volume);

		INT8 const pan = PositionSoundDir(p.sGridNo);
		SoundSetPan(p.uiSoundSampleID, pan);
	}
}

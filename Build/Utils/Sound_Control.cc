// MODULE FOR SOUND SYSTEM

#include "Sound_Control.h"
#include "SoundMan.h"
#include "Overhead.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include <math.h>


#define		SOUND_FAR_VOLUME_MOD		25

/*
UINT32 LOWVOLUME								START_LOWVOLUME;
UINT32 MIDVOLUME								START_MIDVOLUME;
UINT32 HIGHVOLUME								START_HIGHVOLUME;
*/

static UINT32 guiSpeechVolume       = MIDVOLUME;
static UINT32 guiSoundEffectsVolume = MIDVOLUME;

static const char* szSoundEffects[NUM_SAMPLES] =
{
		"SOUNDS/RICOCHET 01.WAV",
		"SOUNDS/RICOCHET 02.WAV",
		"SOUNDS/RICOCHET 01.WAV",
		"SOUNDS/RICOCHET 02.WAV",
		"SOUNDS/RICOCHET 01.WAV",
		"SOUNDS/RICOCHET 02.WAV",
		"SOUNDS/RICOCHET 01.WAV",
		"SOUNDS/RICOCHET 02.WAV",
		"SOUNDS/DIRT IMPACT 01.WAV",
		"SOUNDS/DIRT IMPACT 01.WAV",
		"SOUNDS/KNIFE HIT GROUND.WAV",
		"SOUNDS/FALL TO KNEES 01.WAV",
		"SOUNDS/FALL TO KNEES 02.WAV",
		"SOUNDS/KNEES TO DIRT 01.WAV",
		"SOUNDS/KNEES TO DIRT 02.WAV",
		"SOUNDS/KNEES TO DIRT 03.WAV",
		"SOUNDS/HEAVY FALL 01.WAV",
		"SOUNDS/BODY_SPLAT.WAV",
		"SOUNDS/GLASS_BREAK1.WAV",
		"SOUNDS/GLASS_BREAK2.WAV",

		"SOUNDS/DOOR OPEN 01.WAV",
		"SOUNDS/DOOR OPEN 02.WAV",
		"SOUNDS/DOOR OPEN 03.WAV",
		"SOUNDS/DOOR CLOSE 01.WAV",
		"SOUNDS/DOOR CLOSE 02.WAV",
		"SOUNDS/UNLOCK LOCK.WAV",
		"SOUNDS/KICKIN LOCK.WAV",
		"SOUNDS/BREAK LOCK.WAV",
		"SOUNDS/PICKING LOCK.WAV",

		"SOUNDS/GARAGE DOOR OPEN.WAV",
		"SOUNDS/GARAGE DOOR CLOSE.WAV",
		"SOUNDS/ELEVATOR DOOR OPEN.WAV",
		"SOUNDS/ELEVATOR DOOR CLOSE.WAV",
		"SOUNDS/HIGH TECH DOOR OPEN.WAV",
		"SOUNDS/HIGH TECH DOOR CLOSE.WAV",
		"SOUNDS/CURTAINS DOOR OPEN.WAV",
		"SOUNDS/CURTAINS DOOR CLOSE.WAV",
		"SOUNDS/METAL DOOR OPEN.WAV",
		"SOUNDS/METAL DOOR CLOSE.WAV",

		"SOUNDS/ftp gravel 01.WAV",
		"SOUNDS/ftp gravel 02.WAV",
		"SOUNDS/ftp gravel 03.WAV",
		"SOUNDS/ftp gravel 04.WAV",
		"SOUNDS/ftp gritty 01.WAV",
		"SOUNDS/ftp gritty 02.WAV",
		"SOUNDS/ftp gritty 03.WAV",
		"SOUNDS/ftp gritty 04.WAV",
		"SOUNDS/ftp leaves 01.WAV",
		"SOUNDS/ftp leaves 02.WAV",
		"SOUNDS/ftp leaves 03.WAV",
		"SOUNDS/ftp leaves 04.WAV",

		"SOUNDS/CRAWLING 01.WAV",
		"SOUNDS/CRAWLING 02.WAV",
		"SOUNDS/CRAWLING 03.WAV",
		"SOUNDS/CRAWLING 04.WAV",
		"SOUNDS/BEEP2.WAV",
		"SOUNDS/ENDTURN.WAV",
		"SOUNDS/JA2 DEATH HIT.WAV",
		"SOUNDS/DOORCR_B.WAV",
		"SOUNDS/HEAD EXPLODING 01.WAV",
		"SOUNDS/BODY EXPLODING.WAV",
		"SOUNDS/EXPLODE1.WAV",
		"SOUNDS/CROW EXPLODING.WAV",
		"SOUNDS/SMALL EXPLOSION 01.WAV",

		"SOUNDS/HELI1.WAV",
		"SOUNDS/BULLET IMPACT 01.WAV",
		"SOUNDS/BULLET IMPACT 02.WAV",
		"SOUNDS/BULLET IMPACT 02.WAV",

		"STSOUNDS/BLAH.WAV",									// CREATURE ATTACK

		"SOUNDS/STEP INTO WATER.WAV",
		"SOUNDS/SPLASH FROM SHALLOW TO DEEP.WAV",

		"SOUNDS/COW HIT.WAV",																	// COW HIT
		"SOUNDS/COW DIE.WAV",																	// COW DIE

		// THREE COMPUTER VOICE SOUNDS FOR RG
		"SOUNDS/LINE 02 FX.WAV",
		"SOUNDS/LINE 01 FX.WAV",
		"SOUNDS/LINE 03 FX.WAV",

		"SOUNDS/CAVE COLLAPSING.WAV",														// CAVE_COLLAPSE


		"SOUNDS/RAID WHISTLE.WAV",															// RAID
		"SOUNDS/RAID AMBIENT.WAV",
		"SOUNDS/RAID DIVE.WAV",
		"SOUNDS/RAID DIVE.WAV",
		"SOUNDS/RAID WHISTLE.WAV",															// RAID

		// VEHICLES
		"SOUNDS/DRIVING 01.WAV",																// DRIVING
		"SOUNDS/ENGINE START.WAV",															// ON
		"SOUNDS/ENGINE OFF.WAV",																// OFF
		"SOUNDS/INTO VEHICLE.WAV",															// INTO


		"SOUNDS/WEAPONS/DRY FIRE 1.WAV",											// Dry fire sound ( for gun jam )

		// IMPACTS
		"SOUNDS/WOOD IMPACT 01A.WAV",													 // S_WOOD_IMPACT1
		"SOUNDS/WOOD IMPACT 01B.WAV",
		"SOUNDS/WOOD IMPACT 01A.WAV",
		"SOUNDS/PORCELAIN IMPACT.WAV",
		"SOUNDS/TIRE IMPACT 01.WAV",
		"SOUNDS/STONE IMPACT 01.WAV",
		"SOUNDS/WATER IMPACT 01.WAV",
		"SOUNDS/VEG IMPACT 01.WAV",
		"SOUNDS/METAL HIT 01.WAV",															 // S_METAL_HIT1
		"SOUNDS/METAL HIT 01.WAV",
		"SOUNDS/METAL HIT 01.WAV",

		"SOUNDS/SLAP_IMPACT.WAV",

		// FIREARM RELOAD
		"SOUNDS/WEAPONS/REVOLVER RELOAD.WAV",										// REVOLVER
		"SOUNDS/WEAPONS/PISTOL RELOAD.WAV",											// PISTOL
		"SOUNDS/WEAPONS/SMG RELOAD.WAV",												// SMG
		"SOUNDS/WEAPONS/RIFLE RELOAD.WAV",											// RIFLE
 		"SOUNDS/WEAPONS/SHOTGUN RELOAD.WAV",										// SHOTGUN
		"SOUNDS/WEAPONS/LMG RELOAD.WAV",												// LMG

		// FIREARM LOCKNLOAD
		"SOUNDS/WEAPONS/REVOLVER LNL.WAV",											// REVOLVER
		"SOUNDS/WEAPONS/PISTOL LNL.WAV",												// PISTOL
		"SOUNDS/WEAPONS/SMG LNL.WAV",														// SMG
		"SOUNDS/WEAPONS/RIFLE LNL.WAV",													// RIFLE
 		"SOUNDS/WEAPONS/SHOTGUN LNL.WAV",												// SHOTGUN
		"SOUNDS/WEAPONS/LMG LNL.WAV",														// LMG

		// ROCKET LAUCNHER
		"SOUNDS/WEAPONS/SMALL ROCKET LAUNCHER.WAV",							// SMALL ROCKET LUANCHER
		"SOUNDS/WEAPONS/MORTAR FIRE 01.WAV",										// GRENADE LAUNCHER
		"SOUNDS/WEAPONS/MORTAR FIRE 01.WAV",										// UNDERSLUNG GRENADE LAUNCHER
		"SOUNDS/WEAPONS/ROCKET LAUNCHER.WAV",
		"SOUNDS/WEAPONS/MORTAR FIRE 01.WAV",

		// FIREARMS
		"SOUNDS/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_GLOCK17				9mm
		"SOUNDS/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_GLOCK18				9mm
		"SOUNDS/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_BERETTA92			9mm
		"SOUNDS/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_BERETTA93			9mm
		"SOUNDS/WEAPONS/38 CALIBER.WAV",												//	S_SWSPECIAL			.38
		"SOUNDS/WEAPONS/357 SINGLE SHOT.WAV",										//	S_BARRACUDA			.357
		"SOUNDS/WEAPONS/357 SINGLE SHOT.WAV",										//	S_DESERTEAGLE		.357
		"SOUNDS/WEAPONS/45 CALIBER SINGLE SHOT.WAV",						//	S_M1911					.45
		"SOUNDS/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_MP5K					9mm
		"SOUNDS/WEAPONS/45 CALIBER SINGLE SHOT.WAV",						//	S_MAC10					.45
		"SOUNDS/WEAPONS/45 CALIBER SINGLE SHOT.WAV",						//	S_THOMPSON			.45
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_COMMANDO			5.56
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_MP53					5.56?
		"SOUNDS/WEAPONS/5,45 SINGLE SHOT.WAV",									//	S_AKSU74				5.45
			"SOUNDS/WEAPONS/5,7 SINGLE SHOT.WAV",									//	S_P90						5.7
			"SOUNDS/WEAPONS/7,62 WP SINGLE SHOT.WAV",							//	S_TYPE85				7.62 WP
		"SOUNDS/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_SKS						7.62 WP
		"SOUNDS/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_DRAGUNOV			7.62 WP
		"SOUNDS/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_M24						7.62 NATO
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_AUG						5.56mm
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_G41						5.56mm
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_RUGERMINI			5.56mm
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_C7						5.56mm
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_FAMAS					5.56mm
		"SOUNDS/WEAPONS/5,45 SINGLE SHOT.WAV",									//	S_AK74					5.45mm
		"SOUNDS/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_AKM						7.62mm WP
		"SOUNDS/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_M14						7.62mm NATO
		"SOUNDS/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_FNFAL					7.62mm NATO
		"SOUNDS/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_G3A3					7.62mm NATO
		"SOUNDS/WEAPONS/4,7 SINGLE SHOT.WAV",										//	S_G11						4.7mm
		"SOUNDS/WEAPONS/SHOTGUN SINGLE SHOT.WAV",								//	S_M870					SHOTGUN
		"SOUNDS/WEAPONS/SHOTGUN SINGLE SHOT.WAV",								//	S_SPAS					SHOTGUN
		"SOUNDS/WEAPONS/SHOTGUN SINGLE SHOT.WAV",								//	S_CAWS					SHOTGUN
		"SOUNDS/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_FNMINI				5.56mm
		"SOUNDS/WEAPONS/5,45 SINGLE SHOT.WAV",									//	S_RPK74					5.45mm
		"SOUNDS/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_21E						7.62mm
		"SOUNDS/WEAPONS/KNIFE THROW SWOOSH.WAV",												//	KNIFE THROW
		"SOUNDS/WEAPONS/TANK_CANNON.WAV",
		"SOUNDS/WEAPONS/BURSTTYPE1.WAV",
		"SOUNDS/WEAPONS/AUTOMAG SINGLE.WAV",

		"SOUNDS/WEAPONS/SILENCER 02.WAV",
		"SOUNDS/WEAPONS/SILENCER 03.WAV",

		"SOUNDS/SWOOSH 01.WAV",
		"SOUNDS/SWOOSH 03.WAV",
		"SOUNDS/SWOOSH 05.WAV",
		"SOUNDS/SWOOSH 06.WAV",
		"SOUNDS/SWOOSH 11.WAV",
		"SOUNDS/SWOOSH 14.WAV",

		// CREATURE_SOUNDS
		"SOUNDS/ADULT FALL 01.WAV",
		"SOUNDS/ADULT STEP 01.WAV",
		"SOUNDS/ADULT STEP 02.WAV",
		"SOUNDS/ADULT SWIPE 01.WAV",
		"SOUNDS/Eating_Flesh 01.WAV",
		"SOUNDS/ADULT CRIPPLED.WAV",
		"SOUNDS/ADULT DYING PART 1.WAV",
		"SOUNDS/ADULT DYING PART 2.WAV",
		"SOUNDS/ADULT LUNGE 01.WAV",
		"SOUNDS/ADULT SMELLS THREAT.WAV",
		"SOUNDS/ADULT SMELLS PREY.WAV",
		"SOUNDS/ADULT SPIT.WAV",

		// BABY
		"SOUNDS/BABY DYING 01.WAV",
		"SOUNDS/BABY DRAGGING 01.WAV",
		"SOUNDS/BABY SHRIEK 01.WAV",
		"SOUNDS/BABY SPITTING 01.WAV",

		// LARVAE
		"SOUNDS/LARVAE MOVEMENT 01.WAV",
		"SOUNDS/LARVAE RUPTURE 01.WAV",

		//QUEEN
		"SOUNDS/QUEEN SHRIEK 01.WAV",
		"SOUNDS/QUEEN DYING 01.WAV",
		"SOUNDS/QUEEN ENRAGED ATTACK.WAV",
		"SOUNDS/QUEEN RUPTURING.WAV",
		"SOUNDS/QUEEN CRIPPLED.WAV",
		"SOUNDS/QUEEN SMELLS THREAT.WAV",
		"SOUNDS/QUEEN WHIP ATTACK.WAV",

		"SOUNDS/ROCK HIT 01.WAV",
		"SOUNDS/ROCK HIT 02.WAV",

		"SOUNDS/SCRATCH.WAV",
		"SOUNDS/ARMPIT.WAV",
		"SOUNDS/CRACKING BACK.WAV",

		"SOUNDS/WEAPONS/Auto Resolve Composite 02 (8-22).wav",							//  The FF sound in autoresolve interface

		"SOUNDS/Email Alert 01.wav",
		"SOUNDS/Entering Text 02.wav",
		"SOUNDS/Removing Text 02.wav",
		"SOUNDS/Computer Beep 01 In.wav",
		"SOUNDS/Computer Beep 01 Out.wav",
		"SOUNDS/Computer Switch 01 In.wav",
		"SOUNDS/Computer Switch 01 Out.wav",
		"SOUNDS/Very Small Switch 01 In.wav",
		"SOUNDS/Very Small Switch 01 Out.wav",
		"SOUNDS/Very Small Switch 02 In.wav",
		"SOUNDS/Very Small Switch 02 Out.wav",
		"SOUNDS/Small Switch 01 In.wav",
		"SOUNDS/Small Switch 01 Out.wav",
		"SOUNDS/Small Switch 02 In.wav",
		"SOUNDS/Small Switch 02 Out.wav",
		"SOUNDS/Small Switch 03 In.wav",
		"SOUNDS/Small Switch 03 Out.wav",
		"SOUNDS/Big Switch 03 In.wav",
		"SOUNDS/Big Switch 03 Out.wav",
		"SOUNDS/Alarm.wav",
		"SOUNDS/Fight Bell.wav",
		"SOUNDS/Helicopter Crash Sequence.wav",
		"SOUNDS/Attachment.wav",
		"SOUNDS/Ceramic Armour Insert.wav",
		"SOUNDS/Detonator Beep.wav",
		"SOUNDS/Grab Roof.wav",
		"SOUNDS/Land On Roof.wav",
		"SOUNDS/Branch Snap 01.wav",
		"SOUNDS/Branch Snap 02.wav",
		"SOUNDS/Indoor Bump 01.wav",

		"SOUNDS/Fridge Door Open.wav",
		"SOUNDS/Fridge Door Close.wav",

		"SOUNDS/Fire 03 Loop.wav",
		"SOUNDS/GLASS_CRACK.wav",
		"SOUNDS/SPIT RICOCHET.WAV",
		"SOUNDS/TIGER HIT.WAV",
		"SOUNDS/bloodcat dying 02.WAV",
		"SOUNDS/SLAP.WAV",
		"SOUNDS/ROBOT BEEP.WAV",
		"SOUNDS/ELECTRICITY.WAV",
		"SOUNDS/SWIMMING 01.WAV",
		"SOUNDS/SWIMMING 02.WAV",
		"SOUNDS/KEY FAILURE.WAV",
		"SOUNDS/target cursor.WAV",
		"SOUNDS/statue open.WAV",
		"SOUNDS/remote activate.WAV",
		"SOUNDS/wirecutters.WAV",
		"SOUNDS/drink from canteen.WAV",
    "SOUNDS/bloodcat attack.wav",
    "SOUNDS/bloodcat loud roar.wav",
    "SOUNDS/robot greeting.wav",
    "SOUNDS/robot death.wav",
		"SOUNDS/gas grenade explode.WAV",
		"SOUNDS/air escaping.WAV",
		"SOUNDS/drawer open.WAV",
		"SOUNDS/drawer close.WAV",
		"SOUNDS/locker door open.WAV",
		"SOUNDS/locker door close.WAV",
		"SOUNDS/wooden box open.WAV",
		"SOUNDS/wooden box close.WAV",
		"SOUNDS/robot stop moving.WAV",
    "SOUNDS/water movement 01.wav",
    "SOUNDS/water movement 02.wav",
    "SOUNDS/water movement 03.wav",
    "SOUNDS/water movement 04.wav",
    "SOUNDS/PRONE TO CROUCH.WAV",
    "SOUNDS/CROUCH TO PRONE.WAV",
    "SOUNDS/CROUCH TO STAND.WAV",
    "SOUNDS/STAND TO CROUCH.WAV",
    "SOUNDS/picking something up.WAV",
    "SOUNDS/cow falling.wav",
    "SOUNDS/bloodcat_growl_01.wav",
    "SOUNDS/bloodcat_growl_02.wav",
    "SOUNDS/bloodcat_growl_03.wav",
    "SOUNDS/bloodcat_growl_04.wav",
    "SOUNDS/spit ricochet.wav",
		"SOUNDS/ADULT crippled.WAV",
    "SOUNDS/death disintegration.wav",
    "SOUNDS/Queen Ambience.wav",
    "SOUNDS/Alien Impact.wav",
    "SOUNDS/crow pecking flesh 01.wav",
    "SOUNDS/crow fly.wav",
    "SOUNDS/slap 02.wav",
    "SOUNDS/setting up mortar.wav",
    "SOUNDS/mortar whistle.wav",
    "SOUNDS/load mortar.wav",
    "SOUNDS/tank turret a.wav",
    "SOUNDS/tank turret b.wav",
    "SOUNDS/cow falling b.wav",
    "SOUNDS/stab into flesh.wav",
    "SOUNDS/explosion 10.wav",
    "SOUNDS/explosion 12.wav",
		"SOUNDS/drink from canteen male.WAV",
		"SOUNDS/x ray activated.WAV",
    "SOUNDS/catch object.wav",
    "SOUNDS/fence open.wav",
};

static const char* szAmbientEffects[NUM_AMBIENTS] =
{
		"SOUNDS/storm1.wav",
		"SOUNDS/storm2.wav",
		"SOUNDS/rain_loop_22k.wav",
		"SOUNDS/bird1-22k.wav",
		"SOUNDS/bird3-22k.wav",
		"SOUNDS/crickety_loop.wav",
		"SOUNDS/crickety_loop2.wav",
		"SOUNDS/cricket1.wav",
		"SOUNDS/cricket2.wav",
		"SOUNDS/owl1.wav",
		"SOUNDS/owl2.wav",
		"SOUNDS/owl3.wav",
		"SOUNDS/night_bird1.wav",
		"SOUNDS/night_bird3.wav"
};

static const UINT8 AmbientVols[NUM_AMBIENTS] = {
	25,		// lightning 1
	25,		// lightning 2
	10,		// rain 1
	25,		// bird 1
	25,		// bird 2
	10,		// crickets 1
	10,		// crickets 2
	25,		// cricket 1
	25,		// cricket 2
	25,		// owl 1
	25,		// owl 2
	25,		// owl 3
	25,		// night bird 1
	25		// night bird 2
};


void ShutdownJA2Sound(void)
{
	SoundStopAll();
}


UINT32 PlayJA2Sample( UINT32 usNum, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan)
{
	const UINT32 vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szSoundEffects[usNum], vol, uiPan, ubLoops, NULL, NULL);
}


UINT32 PlayJA2StreamingSample(UINT32 usNum, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan)
{
	const UINT32 vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlayStreamedFile(szSoundEffects[usNum], vol, uiPan, ubLoops, NULL, NULL);
}


UINT32 PlayJA2SampleFromFile(const char *szFileName, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan)
{
	// does the same thing as PlayJA2Sound, but one only has to pass the filename, not the index of the sound array
	const UINT32 vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szFileName, vol, uiPan, ubLoops, NULL, NULL);
}


UINT32 PlayJA2StreamingSampleFromFile(const char* szFileName, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, SOUND_STOP_CALLBACK EndsCallback)
{
	// does the same thing as PlayJA2Sound, but one only has to pass the filename, not the index of the sound array
	const UINT32 vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlayStreamedFile(szFileName, vol, uiPan, ubLoops, EndsCallback, NULL);
}


UINT32 PlayJA2Ambient( UINT32 usNum, UINT32 ubVolume, UINT32 ubLoops)
{
	const UINT32 vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szAmbientEffects[usNum], vol, MIDDLEPAN, ubLoops, NULL, NULL);
}


static UINT32 PlayJA2AmbientRandom(UINT32 usNum, UINT32 uiTimeMin, UINT32 uiTimeMax)
{
	const char* const filename = szAmbientEffects[usNum];
	const UINT32      vol      = AmbientVols[usNum];
	return SoundPlayRandom(filename, uiTimeMin, uiTimeMax, vol, vol, MIDDLEPAN, MIDDLEPAN, 1);
}


UINT32 PlayLocationJA2SampleFromFile(const UINT16 grid_no, const char* const filename, const UINT32 base_vol, const UINT32 loops)
{
	const UINT32 vol = SoundVolume(base_vol, grid_no);
	const UINT32 pan = SoundDir(grid_no);
	return PlayJA2SampleFromFile(filename, vol, loops, pan);
}


UINT32 PlayLocationJA2Sample(const UINT16 grid_no, const UINT32 idx, const UINT32 base_vol, const UINT32 loops)
{
	const UINT32 vol = SoundVolume(base_vol, grid_no);
	const UINT32 pan = SoundDir(grid_no);
	return PlayJA2Sample(idx, vol, loops, pan);
}


UINT32 PlayLocationJA2StreamingSample(const UINT16 grid_no, const UINT32 idx, const UINT32 base_vol, const UINT32 loops)
{
	const UINT32 vol = SoundVolume(base_vol, grid_no);
	const UINT32 pan = SoundDir(grid_no);
	return PlayJA2StreamingSample(idx, vol, loops, pan);
}


UINT32 PlaySoldierJA2Sample(const SOLDIERTYPE* const s, const UINT32 usNum, const UINT32 base_vol, const UINT32 ubLoops, const BOOLEAN fCheck)
{
	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
  {
	  // CHECK IF GUY IS ON SCREEN BEFORE PLAYING!
		if (s->bVisible != -1 || !fCheck)
	  {
		  const UINT32 vol = SoundVolume(base_vol, s->sGridNo);
		  const UINT32 pan = SoundDir(s->sGridNo);
		  return PlayJA2Sample(usNum, CalculateSoundEffectsVolume(vol), ubLoops, pan);
	  }
  }

	return( 0 );
}


void SetSpeechVolume(UINT32 uiNewVolume)
{
	guiSpeechVolume = __min(uiNewVolume, MAXVOLUME);
}


UINT32 GetSpeechVolume(void)
{
	return( guiSpeechVolume );
}


void SetSoundEffectsVolume(UINT32 uiNewVolume)
{
	guiSoundEffectsVolume = __min(uiNewVolume, MAXVOLUME);
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
   else // inside screen
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

	if ( ( sAbsDif = abs( sDif ) ) > 64 )
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
       //return( FARRIGHT );
       return( 126 );

    }
		else // inside screen
		{
			if ( sDif > 0)
				return( LEFTSIDE );
			else
				return( RIGHTSIDE );
		}
  }
	else // hardly any difference, so sound should be played from middle
    return(MIDDLE);
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

	sAbsDifX = abs( sDifX );
	sAbsDifY = abs( sDifY );

	if ( sAbsDifX  > 64 || sAbsDifY > 64 )
  {
		// OK, NOT the middle.

		// Is it outside the screen?
		if ( sAbsDifX > ( ( gsBottomRightWorldX - gsTopLeftWorldX ) / 2 ) ||
				 sAbsDifY > ( ( gsBottomRightWorldY - gsTopLeftWorldY ) / 2 ) )
    {
			return( __max( LOWVOLUME, ( bInitialVolume - SOUND_FAR_VOLUME_MOD ) ) );
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
#define		NUM_POSITION_SOUND_EFFECT_SLOTS					10

struct POSITIONSND
{
  UINT32        uiFlags;
  INT16         sGridNo;
  INT32         iSoundSampleID;
  INT32         iSoundToPlay;
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
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumPositionSnds; uiCount++)
	{
		if(( gPositionSndData[uiCount].fAllocated==FALSE ) )
			return( (INT32)uiCount );
	}

	if( guiNumPositionSnds < NUM_POSITION_SOUND_EFFECT_SLOTS )
		return( (INT32) guiNumPositionSnds++ );

	return( -1 );
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


INT32 NewPositionSnd(INT16 sGridNo, UINT32 uiFlags, const SOLDIERTYPE* SoundSource, UINT32 iSoundToPlay)
{
	POSITIONSND *pPositionSnd;
	INT32				iPositionSndIndex;

	if( ( iPositionSndIndex = GetFreePositionSnd() )==(-1) )
		return(-1);

	memset( &gPositionSndData[ iPositionSndIndex ], 0, sizeof( POSITIONSND ) );

	pPositionSnd = &gPositionSndData[ iPositionSndIndex ];


  // Default to inactive

  if ( gfPositionSoundsActive )
  {
    pPositionSnd->fInActive   = FALSE;
  }
  else
  {
    pPositionSnd->fInActive   = TRUE;
  }

  pPositionSnd->sGridNo     = sGridNo;
  pPositionSnd->SoundSource = SoundSource;
  pPositionSnd->uiFlags     = uiFlags;
  pPositionSnd->fAllocated  = TRUE;
  pPositionSnd->iSoundToPlay = iSoundToPlay;

  pPositionSnd->iSoundSampleID = NO_SAMPLE;

  return( iPositionSndIndex );
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
     if ( pPositionSnd->iSoundSampleID != NO_SAMPLE )
     {
        SoundStop( pPositionSnd->iSoundSampleID );
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
  UINT32 cnt;
	POSITIONSND *pPositionSnd;

  gfPositionSoundsActive = TRUE;

  for ( cnt = 0; cnt < guiNumPositionSnds; cnt++ )
  {
  	pPositionSnd = &gPositionSndData[ cnt ];

    if ( pPositionSnd->fAllocated )
    {
      if ( pPositionSnd->fInActive )
      {
        pPositionSnd->fInActive = FALSE;

        // Begin sound effect
        // Volume 0
        pPositionSnd->iSoundSampleID = PlayJA2Sample(pPositionSnd->iSoundToPlay, 0, 0, MIDDLEPAN);
      }
    }
  }
}


void SetPositionSndsInActive(void)
{
  UINT32 cnt;
	POSITIONSND *pPositionSnd;

  gfPositionSoundsActive = FALSE;

  for ( cnt = 0; cnt < guiNumPositionSnds; cnt++ )
  {
  	pPositionSnd = &gPositionSndData[ cnt ];

    if ( pPositionSnd->fAllocated )
    {
      pPositionSnd->fInActive = TRUE;

      // End sound...
      if ( pPositionSnd->iSoundSampleID != NO_SAMPLE )
      {
         SoundStop( pPositionSnd->iSoundSampleID );
         pPositionSnd->iSoundSampleID = NO_SAMPLE;
      }
    }
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

	if ( ( sAbsDif = abs( sDif ) ) > 64 )
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
       //return( FARRIGHT );
       return( 126 );

    }
		else // inside screen
		{
			if ( sDif > 0)
				return( LEFTSIDE );
			else
				return( RIGHTSIDE );
		}
  }
	else // hardly any difference, so sound should be played from middle
    return(MIDDLE);
}


static INT8 PositionSoundVolume(INT8 bInitialVolume, INT16 sGridNo)
{
	INT16 sScreenX, sScreenY;
	INT16	sMiddleX, sMiddleY;
	INT16	sDifX, sAbsDifX;
	INT16	sDifY, sAbsDifY;
  INT16 sMaxDistX, sMaxDistY;
  double sMaxSoundDist, sSoundDist;

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

	sAbsDifX = abs( sDifX );
	sAbsDifY = abs( sDifY );

  sMaxDistX = (INT16)( ( gsBottomRightWorldX - gsTopLeftWorldX ) * 1.5 );
  sMaxDistY = (INT16)( ( gsBottomRightWorldY - gsTopLeftWorldY ) * 1.5 );

  sMaxSoundDist = sqrt( (double) ( sMaxDistX * sMaxDistX ) + ( sMaxDistY * sMaxDistY ) );
  sSoundDist    = sqrt( (double)( sAbsDifX * sAbsDifX ) + ( sAbsDifY * sAbsDifY ) );

  if ( sSoundDist == 0 )
  {
    return( bInitialVolume );
  }

  if ( sSoundDist > sMaxSoundDist )
  {
    sSoundDist = sMaxSoundDist;
  }

  // Scale
  return( (INT8)( bInitialVolume * ( ( sMaxSoundDist - sSoundDist ) / sMaxSoundDist ) ) );
}


void SetPositionSndsVolumeAndPanning(void)
{
  UINT32 cnt;
	POSITIONSND *pPositionSnd;
  INT8        bVolume, bPan;

  for ( cnt = 0; cnt < guiNumPositionSnds; cnt++ )
  {
  	pPositionSnd = &gPositionSndData[ cnt ];

    if ( pPositionSnd->fAllocated )
    {
      if ( !pPositionSnd->fInActive )
      {
         if ( pPositionSnd->iSoundSampleID != NO_SAMPLE )
         {
            bVolume = PositionSoundVolume( 15, pPositionSnd->sGridNo );

            if ( pPositionSnd->uiFlags & POSITION_SOUND_FROM_SOLDIER )
            {
               if (pPositionSnd->SoundSource->bVisible == -1)
               {
                  // Limit volume,,,
                  if ( bVolume > 10 )
                  {
                    bVolume = 10;
                  }
               }
            }

            SoundSetVolume( pPositionSnd->iSoundSampleID, bVolume );

            bPan = PositionSoundDir( pPositionSnd->sGridNo );

            SoundSetPan( pPositionSnd->iSoundSampleID, bPan );
         }
      }
    }
  }
}

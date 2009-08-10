// MODULE FOR SOUND SYSTEM

#include "Directories.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "Overhead.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include <math.h>


#define		SOUND_FAR_VOLUME_MOD		25

static UINT32 guiSpeechVolume       = MIDVOLUME;
static UINT32 guiSoundEffectsVolume = MIDVOLUME;

static char const* const szSoundEffects[NUM_SAMPLES] =
{
		SOUNDSDIR "/RICOCHET 01.WAV",
		SOUNDSDIR "/RICOCHET 02.WAV",
		SOUNDSDIR "/RICOCHET 01.WAV",
		SOUNDSDIR "/RICOCHET 02.WAV",
		SOUNDSDIR "/RICOCHET 01.WAV",
		SOUNDSDIR "/RICOCHET 02.WAV",
		SOUNDSDIR "/RICOCHET 01.WAV",
		SOUNDSDIR "/RICOCHET 02.WAV",
		SOUNDSDIR "/DIRT IMPACT 01.WAV",
		SOUNDSDIR "/DIRT IMPACT 01.WAV",
		SOUNDSDIR "/KNIFE HIT GROUND.WAV",
		SOUNDSDIR "/FALL TO KNEES 01.WAV",
		SOUNDSDIR "/FALL TO KNEES 02.WAV",
		SOUNDSDIR "/KNEES TO DIRT 01.WAV",
		SOUNDSDIR "/KNEES TO DIRT 02.WAV",
		SOUNDSDIR "/KNEES TO DIRT 03.WAV",
		SOUNDSDIR "/HEAVY FALL 01.WAV",
		SOUNDSDIR "/BODY_SPLAT.WAV",
		SOUNDSDIR "/GLASS_BREAK1.WAV",
		SOUNDSDIR "/GLASS_BREAK2.WAV",

		SOUNDSDIR "/DOOR OPEN 01.WAV",
		SOUNDSDIR "/DOOR OPEN 02.WAV",
		SOUNDSDIR "/DOOR OPEN 03.WAV",
		SOUNDSDIR "/DOOR CLOSE 01.WAV",
		SOUNDSDIR "/DOOR CLOSE 02.WAV",
		SOUNDSDIR "/UNLOCK LOCK.WAV",
		SOUNDSDIR "/KICKIN LOCK.WAV",
		SOUNDSDIR "/BREAK LOCK.WAV",
		SOUNDSDIR "/PICKING LOCK.WAV",

		SOUNDSDIR "/GARAGE DOOR OPEN.WAV",
		SOUNDSDIR "/GARAGE DOOR CLOSE.WAV",
		SOUNDSDIR "/ELEVATOR DOOR OPEN.WAV",
		SOUNDSDIR "/ELEVATOR DOOR CLOSE.WAV",
		SOUNDSDIR "/HIGH TECH DOOR OPEN.WAV",
		SOUNDSDIR "/HIGH TECH DOOR CLOSE.WAV",
		SOUNDSDIR "/CURTAINS DOOR OPEN.WAV",
		SOUNDSDIR "/CURTAINS DOOR CLOSE.WAV",
		SOUNDSDIR "/METAL DOOR OPEN.WAV",
		SOUNDSDIR "/METAL DOOR CLOSE.WAV",

		SOUNDSDIR "/ftp gravel 01.WAV",
		SOUNDSDIR "/ftp gravel 02.WAV",
		SOUNDSDIR "/ftp gravel 03.WAV",
		SOUNDSDIR "/ftp gravel 04.WAV",
		SOUNDSDIR "/ftp gritty 01.WAV",
		SOUNDSDIR "/ftp gritty 02.WAV",
		SOUNDSDIR "/ftp gritty 03.WAV",
		SOUNDSDIR "/ftp gritty 04.WAV",
		SOUNDSDIR "/ftp leaves 01.WAV",
		SOUNDSDIR "/ftp leaves 02.WAV",
		SOUNDSDIR "/ftp leaves 03.WAV",
		SOUNDSDIR "/ftp leaves 04.WAV",

		SOUNDSDIR "/CRAWLING 01.WAV",
		SOUNDSDIR "/CRAWLING 02.WAV",
		SOUNDSDIR "/CRAWLING 03.WAV",
		SOUNDSDIR "/CRAWLING 04.WAV",
		SOUNDSDIR "/BEEP2.WAV",
		SOUNDSDIR "/ENDTURN.WAV",
		SOUNDSDIR "/JA2 DEATH HIT.WAV",
		SOUNDSDIR "/DOORCR_B.WAV",
		SOUNDSDIR "/HEAD EXPLODING 01.WAV",
		SOUNDSDIR "/BODY EXPLODING.WAV",
		SOUNDSDIR "/EXPLODE1.WAV",
		SOUNDSDIR "/CROW EXPLODING.WAV",
		SOUNDSDIR "/SMALL EXPLOSION 01.WAV",

		SOUNDSDIR "/HELI1.WAV",
		SOUNDSDIR "/BULLET IMPACT 01.WAV",
		SOUNDSDIR "/BULLET IMPACT 02.WAV",
		SOUNDSDIR "/BULLET IMPACT 02.WAV",

		STSOUNDSDIR "/BLAH.WAV",									// CREATURE ATTACK

		SOUNDSDIR "/STEP INTO WATER.WAV",
		SOUNDSDIR "/SPLASH FROM SHALLOW TO DEEP.WAV",

		SOUNDSDIR "/COW HIT.WAV",																	// COW HIT
		SOUNDSDIR "/COW DIE.WAV",																	// COW DIE

		// THREE COMPUTER VOICE SOUNDS FOR RG
		SOUNDSDIR "/LINE 02 FX.WAV",
		SOUNDSDIR "/LINE 01 FX.WAV",
		SOUNDSDIR "/LINE 03 FX.WAV",

		SOUNDSDIR "/CAVE COLLAPSING.WAV",														// CAVE_COLLAPSE


		SOUNDSDIR "/RAID WHISTLE.WAV",															// RAID
		SOUNDSDIR "/RAID AMBIENT.WAV",
		SOUNDSDIR "/RAID DIVE.WAV",
		SOUNDSDIR "/RAID DIVE.WAV",
		SOUNDSDIR "/RAID WHISTLE.WAV",															// RAID

		// VEHICLES
		SOUNDSDIR "/DRIVING 01.WAV",																// DRIVING
		SOUNDSDIR "/ENGINE START.WAV",															// ON
		SOUNDSDIR "/ENGINE OFF.WAV",																// OFF
		SOUNDSDIR "/INTO VEHICLE.WAV",															// INTO


		SOUNDSDIR "/WEAPONS/DRY FIRE 1.WAV",											// Dry fire sound ( for gun jam )

		// IMPACTS
		SOUNDSDIR "/WOOD IMPACT 01A.WAV",													 // S_WOOD_IMPACT1
		SOUNDSDIR "/WOOD IMPACT 01B.WAV",
		SOUNDSDIR "/WOOD IMPACT 01A.WAV",
		SOUNDSDIR "/PORCELAIN IMPACT.WAV",
		SOUNDSDIR "/TIRE IMPACT 01.WAV",
		SOUNDSDIR "/STONE IMPACT 01.WAV",
		SOUNDSDIR "/WATER IMPACT 01.WAV",
		SOUNDSDIR "/VEG IMPACT 01.WAV",
		SOUNDSDIR "/METAL HIT 01.WAV",															 // S_METAL_HIT1
		SOUNDSDIR "/METAL HIT 01.WAV",
		SOUNDSDIR "/METAL HIT 01.WAV",

		SOUNDSDIR "/SLAP_IMPACT.WAV",

		// FIREARM RELOAD
		SOUNDSDIR "/WEAPONS/REVOLVER RELOAD.WAV",										// REVOLVER
		SOUNDSDIR "/WEAPONS/PISTOL RELOAD.WAV",											// PISTOL
		SOUNDSDIR "/WEAPONS/SMG RELOAD.WAV",												// SMG
		SOUNDSDIR "/WEAPONS/RIFLE RELOAD.WAV",											// RIFLE
		SOUNDSDIR "/WEAPONS/SHOTGUN RELOAD.WAV",										// SHOTGUN
		SOUNDSDIR "/WEAPONS/LMG RELOAD.WAV",												// LMG

		// FIREARM LOCKNLOAD
		SOUNDSDIR "/WEAPONS/REVOLVER LNL.WAV",											// REVOLVER
		SOUNDSDIR "/WEAPONS/PISTOL LNL.WAV",												// PISTOL
		SOUNDSDIR "/WEAPONS/SMG LNL.WAV",														// SMG
		SOUNDSDIR "/WEAPONS/RIFLE LNL.WAV",													// RIFLE
		SOUNDSDIR "/WEAPONS/SHOTGUN LNL.WAV",												// SHOTGUN
		SOUNDSDIR "/WEAPONS/LMG LNL.WAV",														// LMG

		// ROCKET LAUCNHER
		SOUNDSDIR "/WEAPONS/SMALL ROCKET LAUNCHER.WAV",							// SMALL ROCKET LUANCHER
		SOUNDSDIR "/WEAPONS/MORTAR FIRE 01.WAV",										// GRENADE LAUNCHER
		SOUNDSDIR "/WEAPONS/MORTAR FIRE 01.WAV",										// UNDERSLUNG GRENADE LAUNCHER
		SOUNDSDIR "/WEAPONS/ROCKET LAUNCHER.WAV",
		SOUNDSDIR "/WEAPONS/MORTAR FIRE 01.WAV",

		// FIREARMS
		SOUNDSDIR "/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_GLOCK17				9mm
		SOUNDSDIR "/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_GLOCK18				9mm
		SOUNDSDIR "/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_BERETTA92			9mm
		SOUNDSDIR "/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_BERETTA93			9mm
		SOUNDSDIR "/WEAPONS/38 CALIBER.WAV",												//	S_SWSPECIAL			.38
		SOUNDSDIR "/WEAPONS/357 SINGLE SHOT.WAV",										//	S_BARRACUDA			.357
		SOUNDSDIR "/WEAPONS/357 SINGLE SHOT.WAV",										//	S_DESERTEAGLE		.357
		SOUNDSDIR "/WEAPONS/45 CALIBER SINGLE SHOT.WAV",						//	S_M1911					.45
		SOUNDSDIR "/WEAPONS/9mm SINGLE SHOT.WAV",										//	S_MP5K					9mm
		SOUNDSDIR "/WEAPONS/45 CALIBER SINGLE SHOT.WAV",						//	S_MAC10					.45
		SOUNDSDIR "/WEAPONS/45 CALIBER SINGLE SHOT.WAV",						//	S_THOMPSON			.45
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_COMMANDO			5.56
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_MP53					5.56?
		SOUNDSDIR "/WEAPONS/5,45 SINGLE SHOT.WAV",									//	S_AKSU74				5.45
		SOUNDSDIR "/WEAPONS/5,7 SINGLE SHOT.WAV",									//	S_P90						5.7
		SOUNDSDIR "/WEAPONS/7,62 WP SINGLE SHOT.WAV",							//	S_TYPE85				7.62 WP
		SOUNDSDIR "/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_SKS						7.62 WP
		SOUNDSDIR "/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_DRAGUNOV			7.62 WP
		SOUNDSDIR "/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_M24						7.62 NATO
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_AUG						5.56mm
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_G41						5.56mm
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_RUGERMINI			5.56mm
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_C7						5.56mm
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_FAMAS					5.56mm
		SOUNDSDIR "/WEAPONS/5,45 SINGLE SHOT.WAV",									//	S_AK74					5.45mm
		SOUNDSDIR "/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_AKM						7.62mm WP
		SOUNDSDIR "/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_M14						7.62mm NATO
		SOUNDSDIR "/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_FNFAL					7.62mm NATO
		SOUNDSDIR "/WEAPONS/7,62 NATO SINGLE SHOT.WAV",							//	S_G3A3					7.62mm NATO
		SOUNDSDIR "/WEAPONS/4,7 SINGLE SHOT.WAV",										//	S_G11						4.7mm
		SOUNDSDIR "/WEAPONS/SHOTGUN SINGLE SHOT.WAV",								//	S_M870					SHOTGUN
		SOUNDSDIR "/WEAPONS/SHOTGUN SINGLE SHOT.WAV",								//	S_SPAS					SHOTGUN
		SOUNDSDIR "/WEAPONS/SHOTGUN SINGLE SHOT.WAV",								//	S_CAWS					SHOTGUN
		SOUNDSDIR "/WEAPONS/5,56 SINGLE SHOT.WAV",									//	S_FNMINI				5.56mm
		SOUNDSDIR "/WEAPONS/5,45 SINGLE SHOT.WAV",									//	S_RPK74					5.45mm
		SOUNDSDIR "/WEAPONS/7,62 WP SINGLE SHOT.WAV",								//	S_21E						7.62mm
		SOUNDSDIR "/WEAPONS/KNIFE THROW SWOOSH.WAV",												//	KNIFE THROW
		SOUNDSDIR "/WEAPONS/TANK_CANNON.WAV",
		SOUNDSDIR "/WEAPONS/BURSTTYPE1.WAV",
		SOUNDSDIR "/WEAPONS/AUTOMAG SINGLE.WAV",

		SOUNDSDIR "/WEAPONS/SILENCER 02.WAV",
		SOUNDSDIR "/WEAPONS/SILENCER 03.WAV",

		SOUNDSDIR "/SWOOSH 01.WAV",
		SOUNDSDIR "/SWOOSH 03.WAV",
		SOUNDSDIR "/SWOOSH 05.WAV",
		SOUNDSDIR "/SWOOSH 06.WAV",
		SOUNDSDIR "/SWOOSH 11.WAV",
		SOUNDSDIR "/SWOOSH 14.WAV",

		// CREATURE_SOUNDS
		SOUNDSDIR "/ADULT FALL 01.WAV",
		SOUNDSDIR "/ADULT STEP 01.WAV",
		SOUNDSDIR "/ADULT STEP 02.WAV",
		SOUNDSDIR "/ADULT SWIPE 01.WAV",
		SOUNDSDIR "/Eating_Flesh 01.WAV",
		SOUNDSDIR "/ADULT CRIPPLED.WAV",
		SOUNDSDIR "/ADULT DYING PART 1.WAV",
		SOUNDSDIR "/ADULT DYING PART 2.WAV",
		SOUNDSDIR "/ADULT LUNGE 01.WAV",
		SOUNDSDIR "/ADULT SMELLS THREAT.WAV",
		SOUNDSDIR "/ADULT SMELLS PREY.WAV",
		SOUNDSDIR "/ADULT SPIT.WAV",

		// BABY
		SOUNDSDIR "/BABY DYING 01.WAV",
		SOUNDSDIR "/BABY DRAGGING 01.WAV",
		SOUNDSDIR "/BABY SHRIEK 01.WAV",
		SOUNDSDIR "/BABY SPITTING 01.WAV",

		// LARVAE
		SOUNDSDIR "/LARVAE MOVEMENT 01.WAV",
		SOUNDSDIR "/LARVAE RUPTURE 01.WAV",

		//QUEEN
		SOUNDSDIR "/QUEEN SHRIEK 01.WAV",
		SOUNDSDIR "/QUEEN DYING 01.WAV",
		SOUNDSDIR "/QUEEN ENRAGED ATTACK.WAV",
		SOUNDSDIR "/QUEEN RUPTURING.WAV",
		SOUNDSDIR "/QUEEN CRIPPLED.WAV",
		SOUNDSDIR "/QUEEN SMELLS THREAT.WAV",
		SOUNDSDIR "/QUEEN WHIP ATTACK.WAV",

		SOUNDSDIR "/ROCK HIT 01.WAV",
		SOUNDSDIR "/ROCK HIT 02.WAV",

		SOUNDSDIR "/SCRATCH.WAV",
		SOUNDSDIR "/ARMPIT.WAV",
		SOUNDSDIR "/CRACKING BACK.WAV",

		SOUNDSDIR "/WEAPONS/Auto Resolve Composite 02 (8-22).wav",							//  The FF sound in autoresolve interface

		SOUNDSDIR "/Email Alert 01.wav",
		SOUNDSDIR "/Entering Text 02.wav",
		SOUNDSDIR "/Removing Text 02.wav",
		SOUNDSDIR "/Computer Beep 01 In.wav",
		SOUNDSDIR "/Computer Beep 01 Out.wav",
		SOUNDSDIR "/Computer Switch 01 In.wav",
		SOUNDSDIR "/Computer Switch 01 Out.wav",
		SOUNDSDIR "/Very Small Switch 01 In.wav",
		SOUNDSDIR "/Very Small Switch 01 Out.wav",
		SOUNDSDIR "/Very Small Switch 02 In.wav",
		SOUNDSDIR "/Very Small Switch 02 Out.wav",
		SOUNDSDIR "/Small Switch 01 In.wav",
		SOUNDSDIR "/Small Switch 01 Out.wav",
		SOUNDSDIR "/Small Switch 02 In.wav",
		SOUNDSDIR "/Small Switch 02 Out.wav",
		SOUNDSDIR "/Small Switch 03 In.wav",
		SOUNDSDIR "/Small Switch 03 Out.wav",
		SOUNDSDIR "/Big Switch 03 In.wav",
		SOUNDSDIR "/Big Switch 03 Out.wav",
		SOUNDSDIR "/Alarm.wav",
		SOUNDSDIR "/Fight Bell.wav",
		SOUNDSDIR "/Helicopter Crash Sequence.wav",
		SOUNDSDIR "/Attachment.wav",
		SOUNDSDIR "/Ceramic Armour Insert.wav",
		SOUNDSDIR "/Detonator Beep.wav",
		SOUNDSDIR "/Grab Roof.wav",
		SOUNDSDIR "/Land On Roof.wav",
		SOUNDSDIR "/Branch Snap 01.wav",
		SOUNDSDIR "/Branch Snap 02.wav",
		SOUNDSDIR "/Indoor Bump 01.wav",

		SOUNDSDIR "/Fridge Door Open.wav",
		SOUNDSDIR "/Fridge Door Close.wav",

		SOUNDSDIR "/Fire 03 Loop.wav",
		SOUNDSDIR "/GLASS_CRACK.wav",
		SOUNDSDIR "/SPIT RICOCHET.WAV",
		SOUNDSDIR "/TIGER HIT.WAV",
		SOUNDSDIR "/bloodcat dying 02.WAV",
		SOUNDSDIR "/SLAP.WAV",
		SOUNDSDIR "/ROBOT BEEP.WAV",
		SOUNDSDIR "/ELECTRICITY.WAV",
		SOUNDSDIR "/SWIMMING 01.WAV",
		SOUNDSDIR "/SWIMMING 02.WAV",
		SOUNDSDIR "/KEY FAILURE.WAV",
		SOUNDSDIR "/target cursor.WAV",
		SOUNDSDIR "/statue open.WAV",
		SOUNDSDIR "/remote activate.WAV",
		SOUNDSDIR "/wirecutters.WAV",
		SOUNDSDIR "/drink from canteen.WAV",
		SOUNDSDIR "/bloodcat attack.wav",
		SOUNDSDIR "/bloodcat loud roar.wav",
		SOUNDSDIR "/robot greeting.wav",
		SOUNDSDIR "/robot death.wav",
		SOUNDSDIR "/gas grenade explode.WAV",
		SOUNDSDIR "/air escaping.WAV",
		SOUNDSDIR "/drawer open.WAV",
		SOUNDSDIR "/drawer close.WAV",
		SOUNDSDIR "/locker door open.WAV",
		SOUNDSDIR "/locker door close.WAV",
		SOUNDSDIR "/wooden box open.WAV",
		SOUNDSDIR "/wooden box close.WAV",
		SOUNDSDIR "/robot stop moving.WAV",
		SOUNDSDIR "/water movement 01.wav",
		SOUNDSDIR "/water movement 02.wav",
		SOUNDSDIR "/water movement 03.wav",
		SOUNDSDIR "/water movement 04.wav",
		SOUNDSDIR "/PRONE TO CROUCH.WAV",
		SOUNDSDIR "/CROUCH TO PRONE.WAV",
		SOUNDSDIR "/CROUCH TO STAND.WAV",
		SOUNDSDIR "/STAND TO CROUCH.WAV",
		SOUNDSDIR "/picking something up.WAV",
		SOUNDSDIR "/cow falling.wav",
		SOUNDSDIR "/bloodcat_growl_01.wav",
		SOUNDSDIR "/bloodcat_growl_02.wav",
		SOUNDSDIR "/bloodcat_growl_03.wav",
		SOUNDSDIR "/bloodcat_growl_04.wav",
		SOUNDSDIR "/spit ricochet.wav",
		SOUNDSDIR "/ADULT crippled.WAV",
		SOUNDSDIR "/death disintegration.wav",
		SOUNDSDIR "/Queen Ambience.wav",
		SOUNDSDIR "/Alien Impact.wav",
		SOUNDSDIR "/crow pecking flesh 01.wav",
		SOUNDSDIR "/crow fly.wav",
		SOUNDSDIR "/slap 02.wav",
		SOUNDSDIR "/setting up mortar.wav",
		SOUNDSDIR "/mortar whistle.wav",
		SOUNDSDIR "/load mortar.wav",
		SOUNDSDIR "/tank turret a.wav",
		SOUNDSDIR "/tank turret b.wav",
		SOUNDSDIR "/cow falling b.wav",
		SOUNDSDIR "/stab into flesh.wav",
		SOUNDSDIR "/explosion 10.wav",
		SOUNDSDIR "/explosion 12.wav",
		SOUNDSDIR "/drink from canteen male.WAV",
		SOUNDSDIR "/x ray activated.WAV",
		SOUNDSDIR "/catch object.wav",
		SOUNDSDIR "/fence open.wav",
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

static UINT8 const AmbientVols[NUM_AMBIENTS] =
{
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


UINT32 PlayJA2Sample(SoundID const usNum, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan)
{
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szSoundEffects[usNum], vol, uiPan, ubLoops, NULL, NULL);
}


UINT32 PlayJA2StreamingSample(SoundID const usNum, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan)
{
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlayStreamedFile(szSoundEffects[usNum], vol, uiPan, ubLoops, NULL, NULL);
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
	return SoundPlayStreamedFile(szFileName, vol, uiPan, ubLoops, EndsCallback, NULL);
}


UINT32 PlayJA2Ambient(AmbientSoundID const usNum, UINT32 const ubVolume, UINT32 const ubLoops)
{
	UINT32 const vol = CalculateSoundEffectsVolume(ubVolume);
	return SoundPlay(szAmbientEffects[usNum], vol, MIDDLEPAN, ubLoops, NULL, NULL);
}


static UINT32 PlayJA2AmbientRandom(AmbientSoundID const usNum, UINT32 const uiTimeMin, UINT32 const uiTimeMax)
{
	char const* const filename = szAmbientEffects[usNum];
	UINT32      const vol      = AmbientVols[usNum];
	return SoundPlayRandom(filename, uiTimeMin, uiTimeMax, vol, vol, MIDDLEPAN, MIDDLEPAN, 1);
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
  INT16         sGridNo;
  INT32         iSoundSampleID;
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
	memset(&p, 0, sizeof(p));
	p.fInActive      = !gfPositionSoundsActive;
	p.sGridNo        = sGridNo;
	p.SoundSource    = SoundSource;
	p.fAllocated     = TRUE;
	p.iSoundToPlay   = iSoundToPlay;
	p.iSoundSampleID = NO_SAMPLE;

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
	gfPositionSoundsActive = TRUE;
	for (UINT32 i = 0; i != guiNumPositionSnds; ++i)
	{
		POSITIONSND& p = gPositionSndData[i];
		if (!p.fAllocated) continue;
		if (!p.fInActive)  continue;

		p.fInActive      = FALSE;
		// Begin sound effect, Volume 0
		p.iSoundSampleID = PlayJA2Sample(p.iSoundToPlay, 0, 0, MIDDLEPAN);
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

		if (p.iSoundSampleID == NO_SAMPLE) continue;

		// End sound
		SoundStop(p.iSoundSampleID);
		p.iSoundSampleID = NO_SAMPLE;
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

  double const sMaxSoundDist = sqrt((double)(sMaxDistX * sMaxDistX) + (sMaxDistY * sMaxDistY));
  double       sSoundDist    = sqrt((double)(sDifX * sDifX)  + (sDifY * sDifY));

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
		if (p.iSoundSampleID == NO_SAMPLE) continue;

		INT8 volume = PositionSoundVolume(15, p.sGridNo);
		if (p.SoundSource && p.SoundSource->bVisible == -1)
		{ // Limit volume
			if (volume > 10) volume = 10;
		}
		SoundSetVolume(p.iSoundSampleID, volume);

		INT8 const pan = PositionSoundDir(p.sGridNo);
		SoundSetPan(p.iSoundSampleID, pan);
	}
}

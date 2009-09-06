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
		SOUNDSDIR "/ricochet 01.wav",
		SOUNDSDIR "/ricochet 02.wav",
		SOUNDSDIR "/ricochet 01.wav",
		SOUNDSDIR "/ricochet 02.wav",
		SOUNDSDIR "/ricochet 01.wav",
		SOUNDSDIR "/ricochet 02.wav",
		SOUNDSDIR "/ricochet 01.wav",
		SOUNDSDIR "/ricochet 02.wav",
		SOUNDSDIR "/dirt impact 01.wav",
		SOUNDSDIR "/dirt impact 01.wav",
		SOUNDSDIR "/knife hit ground.wav",
		SOUNDSDIR "/fall to knees 01.wav",
		SOUNDSDIR "/fall to knees 02.wav",
		SOUNDSDIR "/knees to dirt 01.wav",
		SOUNDSDIR "/knees to dirt 02.wav",
		SOUNDSDIR "/knees to dirt 03.wav",
		SOUNDSDIR "/heavy fall 01.wav",
		SOUNDSDIR "/body_splat.wav",
		SOUNDSDIR "/glass_break1.wav",
		SOUNDSDIR "/glass_break2.wav",

		SOUNDSDIR "/door open 01.wav",
		SOUNDSDIR "/door open 02.wav",
		SOUNDSDIR "/door open 03.wav",
		SOUNDSDIR "/door close 01.wav",
		SOUNDSDIR "/door close 02.wav",
		SOUNDSDIR "/unlock lock.wav",
		SOUNDSDIR "/kickin lock.wav",
		SOUNDSDIR "/break lock.wav",
		SOUNDSDIR "/picking lock.wav",

		SOUNDSDIR "/garage door open.wav",
		SOUNDSDIR "/garage door close.wav",
		SOUNDSDIR "/elevator door open.wav",
		SOUNDSDIR "/elevator door close.wav",
		SOUNDSDIR "/high tech door open.wav",
		SOUNDSDIR "/high tech door close.wav",
		SOUNDSDIR "/curtains door open.wav",
		SOUNDSDIR "/curtains door close.wav",
		SOUNDSDIR "/metal door open.wav",
		SOUNDSDIR "/metal door close.wav",

		SOUNDSDIR "/ftp gravel 01.wav",
		SOUNDSDIR "/ftp gravel 02.wav",
		SOUNDSDIR "/ftp gravel 03.wav",
		SOUNDSDIR "/ftp gravel 04.wav",
		SOUNDSDIR "/ftp gritty 01.wav",
		SOUNDSDIR "/ftp gritty 02.wav",
		SOUNDSDIR "/ftp gritty 03.wav",
		SOUNDSDIR "/ftp gritty 04.wav",
		SOUNDSDIR "/ftp leaves 01.wav",
		SOUNDSDIR "/ftp leaves 02.wav",
		SOUNDSDIR "/ftp leaves 03.wav",
		SOUNDSDIR "/ftp leaves 04.wav",

		SOUNDSDIR "/crawling 01.wav",
		SOUNDSDIR "/crawling 02.wav",
		SOUNDSDIR "/crawling 03.wav",
		SOUNDSDIR "/crawling 04.wav",
		SOUNDSDIR "/beep2.wav",
		SOUNDSDIR "/endturn.wav",
		SOUNDSDIR "/ja2 death hit.wav",
		SOUNDSDIR "/doorcr_b.wav",
		SOUNDSDIR "/head exploding 01.wav",
		SOUNDSDIR "/body exploding.wav",
		SOUNDSDIR "/explode1.wav",
		SOUNDSDIR "/crow exploding.wav",
		SOUNDSDIR "/small explosion 01.wav",

		SOUNDSDIR "/heli1.wav",
		SOUNDSDIR "/bullet impact 01.wav",
		SOUNDSDIR "/bullet impact 02.wav",
		SOUNDSDIR "/bullet impact 02.wav",

		STSOUNDSDIR "/blah.wav",									// CREATURE ATTACK

		SOUNDSDIR "/step into water.wav",
		SOUNDSDIR "/splash from shallow to deep.wav",

		SOUNDSDIR "/cow hit.wav",																	// COW HIT
		SOUNDSDIR "/cow die.wav",																	// COW DIE

		// THREE COMPUTER VOICE SOUNDS FOR RG
		SOUNDSDIR "/line 02 fx.wav",
		SOUNDSDIR "/line 01 fx.wav",
		SOUNDSDIR "/line 03 fx.wav",

		SOUNDSDIR "/cave collapsing.wav",														// CAVE_COLLAPSE


		SOUNDSDIR "/raid whistle.wav",															// RAID
		SOUNDSDIR "/raid ambient.wav",
		SOUNDSDIR "/raid dive.wav",
		SOUNDSDIR "/raid dive.wav",
		SOUNDSDIR "/raid whistle.wav",															// RAID

		// VEHICLES
		SOUNDSDIR "/driving 01.wav",																// DRIVING
		SOUNDSDIR "/engine start.wav",															// ON
		SOUNDSDIR "/engine off.wav",																// OFF
		SOUNDSDIR "/into vehicle.wav",															// INTO


		SOUNDSDIR "/weapons/dry fire 1.wav",											// Dry fire sound ( for gun jam )

		// IMPACTS
		SOUNDSDIR "/wood impact 01a.wav",													 // S_WOOD_IMPACT1
		SOUNDSDIR "/wood impact 01b.wav",
		SOUNDSDIR "/wood impact 01a.wav",
		SOUNDSDIR "/porcelain impact.wav",
		SOUNDSDIR "/tire impact 01.wav",
		SOUNDSDIR "/stone impact 01.wav",
		SOUNDSDIR "/water impact 01.wav",
		SOUNDSDIR "/veg impact 01.wav",
		SOUNDSDIR "/metal hit 01.wav",															 // S_METAL_HIT1
		SOUNDSDIR "/metal hit 01.wav",
		SOUNDSDIR "/metal hit 01.wav",

		SOUNDSDIR "/slap_impact.wav",

		// FIREARM RELOAD
		SOUNDSDIR "/weapons/revolver reload.wav",										// REVOLVER
		SOUNDSDIR "/weapons/pistol reload.wav",											// PISTOL
		SOUNDSDIR "/weapons/smg reload.wav",												// SMG
		SOUNDSDIR "/weapons/rifle reload.wav",											// RIFLE
		SOUNDSDIR "/weapons/shotgun reload.wav",										// SHOTGUN
		SOUNDSDIR "/weapons/lmg reload.wav",												// LMG

		// FIREARM LOCKNLOAD
		SOUNDSDIR "/weapons/revolver lnl.wav",											// REVOLVER
		SOUNDSDIR "/weapons/pistol lnl.wav",												// PISTOL
		SOUNDSDIR "/weapons/smg lnl.wav",														// SMG
		SOUNDSDIR "/weapons/rifle lnl.wav",													// RIFLE
		SOUNDSDIR "/weapons/shotgun lnl.wav",												// SHOTGUN
		SOUNDSDIR "/weapons/lmg lnl.wav",														// LMG

		// ROCKET LAUCNHER
		SOUNDSDIR "/weapons/small rocket launcher.wav",							// SMALL ROCKET LUANCHER
		SOUNDSDIR "/weapons/mortar fire 01.wav",										// GRENADE LAUNCHER
		SOUNDSDIR "/weapons/mortar fire 01.wav",										// UNDERSLUNG GRENADE LAUNCHER
		SOUNDSDIR "/weapons/rocket launcher.wav",
		SOUNDSDIR "/weapons/mortar fire 01.wav",

		// FIREARMS
		SOUNDSDIR "/weapons/9mm single shot.wav",										//	S_GLOCK17				9mm
		SOUNDSDIR "/weapons/9mm single shot.wav",										//	S_GLOCK18				9mm
		SOUNDSDIR "/weapons/9mm single shot.wav",										//	S_BERETTA92			9mm
		SOUNDSDIR "/weapons/9mm single shot.wav",										//	S_BERETTA93			9mm
		SOUNDSDIR "/weapons/38 caliber.wav",												//	S_SWSPECIAL			.38
		SOUNDSDIR "/weapons/357 single shot.wav",										//	S_BARRACUDA			.357
		SOUNDSDIR "/weapons/357 single shot.wav",										//	S_DESERTEAGLE		.357
		SOUNDSDIR "/weapons/45 caliber single shot.wav",						//	S_M1911					.45
		SOUNDSDIR "/weapons/9mm single shot.wav",										//	S_MP5K					9mm
		SOUNDSDIR "/weapons/45 caliber single shot.wav",						//	S_MAC10					.45
		SOUNDSDIR "/weapons/45 caliber single shot.wav",						//	S_THOMPSON			.45
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_COMMANDO			5.56
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_MP53					5.56?
		SOUNDSDIR "/weapons/5,45 single shot.wav",									//	S_AKSU74				5.45
		SOUNDSDIR "/weapons/5,7 single shot.wav",									//	S_P90						5.7
		SOUNDSDIR "/weapons/7,62 wp single shot.wav",							//	S_TYPE85				7.62 WP
		SOUNDSDIR "/weapons/7,62 wp single shot.wav",								//	S_SKS						7.62 WP
		SOUNDSDIR "/weapons/7,62 wp single shot.wav",								//	S_DRAGUNOV			7.62 WP
		SOUNDSDIR "/weapons/7,62 nato single shot.wav",							//	S_M24						7.62 NATO
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_AUG						5.56mm
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_G41						5.56mm
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_RUGERMINI			5.56mm
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_C7						5.56mm
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_FAMAS					5.56mm
		SOUNDSDIR "/weapons/5,45 single shot.wav",									//	S_AK74					5.45mm
		SOUNDSDIR "/weapons/7,62 wp single shot.wav",								//	S_AKM						7.62mm WP
		SOUNDSDIR "/weapons/7,62 nato single shot.wav",							//	S_M14						7.62mm NATO
		SOUNDSDIR "/weapons/7,62 nato single shot.wav",							//	S_FNFAL					7.62mm NATO
		SOUNDSDIR "/weapons/7,62 nato single shot.wav",							//	S_G3A3					7.62mm NATO
		SOUNDSDIR "/weapons/4,7 single shot.wav",										//	S_G11						4.7mm
		SOUNDSDIR "/weapons/shotgun single shot.wav",								//	S_M870					SHOTGUN
		SOUNDSDIR "/weapons/shotgun single shot.wav",								//	S_SPAS					SHOTGUN
		SOUNDSDIR "/weapons/shotgun single shot.wav",								//	S_CAWS					SHOTGUN
		SOUNDSDIR "/weapons/5,56 single shot.wav",									//	S_FNMINI				5.56mm
		SOUNDSDIR "/weapons/5,45 single shot.wav",									//	S_RPK74					5.45mm
		SOUNDSDIR "/weapons/7,62 wp single shot.wav",								//	S_21E						7.62mm
		SOUNDSDIR "/weapons/knife throw swoosh.wav",												//	KNIFE THROW
		SOUNDSDIR "/weapons/tank_cannon.wav",
		SOUNDSDIR "/weapons/bursttype1.wav",
		SOUNDSDIR "/weapons/automag single.wav",

		SOUNDSDIR "/weapons/silencer 02.wav",
		SOUNDSDIR "/weapons/silencer 03.wav",

		SOUNDSDIR "/swoosh 01.wav",
		SOUNDSDIR "/swoosh 03.wav",
		SOUNDSDIR "/swoosh 05.wav",
		SOUNDSDIR "/swoosh 06.wav",
		SOUNDSDIR "/swoosh 11.wav",
		SOUNDSDIR "/swoosh 14.wav",

		// CREATURE_SOUNDS
		SOUNDSDIR "/adult fall 01.wav",
		SOUNDSDIR "/adult step 01.wav",
		SOUNDSDIR "/adult step 02.wav",
		SOUNDSDIR "/adult swipe 01.wav",
		SOUNDSDIR "/eating_flesh 01.wav",
		SOUNDSDIR "/adult crippled.wav",
		SOUNDSDIR "/adult dying part 1.wav",
		SOUNDSDIR "/adult dying part 2.wav",
		SOUNDSDIR "/adult lunge 01.wav",
		SOUNDSDIR "/adult smells threat.wav",
		SOUNDSDIR "/adult smells prey.wav",
		SOUNDSDIR "/adult spit.wav",

		// BABY
		SOUNDSDIR "/baby dying 01.wav",
		SOUNDSDIR "/baby dragging 01.wav",
		SOUNDSDIR "/baby shriek 01.wav",
		SOUNDSDIR "/baby spitting 01.wav",

		// LARVAE
		SOUNDSDIR "/larvae movement 01.wav",
		SOUNDSDIR "/larvae rupture 01.wav",

		//QUEEN
		SOUNDSDIR "/queen shriek 01.wav",
		SOUNDSDIR "/queen dying 01.wav",
		SOUNDSDIR "/queen enraged attack.wav",
		SOUNDSDIR "/queen rupturing.wav",
		SOUNDSDIR "/queen crippled.wav",
		SOUNDSDIR "/queen smells threat.wav",
		SOUNDSDIR "/queen whip attack.wav",

		SOUNDSDIR "/rock hit 01.wav",
		SOUNDSDIR "/rock hit 02.wav",

		SOUNDSDIR "/scratch.wav",
		SOUNDSDIR "/armpit.wav",
		SOUNDSDIR "/cracking back.wav",

		SOUNDSDIR "/weapons/auto resolve composite 02 (8-22).wav",							//  The FF sound in autoresolve interface

		SOUNDSDIR "/email alert 01.wav",
		SOUNDSDIR "/entering text 02.wav",
		SOUNDSDIR "/removing text 02.wav",
		SOUNDSDIR "/computer beep 01 in.wav",
		SOUNDSDIR "/computer beep 01 out.wav",
		SOUNDSDIR "/computer switch 01 in.wav",
		SOUNDSDIR "/computer switch 01 out.wav",
		SOUNDSDIR "/very small switch 01 in.wav",
		SOUNDSDIR "/very small switch 01 out.wav",
		SOUNDSDIR "/very small switch 02 in.wav",
		SOUNDSDIR "/very small switch 02 out.wav",
		SOUNDSDIR "/small switch 01 in.wav",
		SOUNDSDIR "/small switch 01 out.wav",
		SOUNDSDIR "/small switch 02 in.wav",
		SOUNDSDIR "/small switch 02 out.wav",
		SOUNDSDIR "/small switch 03 in.wav",
		SOUNDSDIR "/small switch 03 out.wav",
		SOUNDSDIR "/big switch 03 in.wav",
		SOUNDSDIR "/big switch 03 out.wav",
		SOUNDSDIR "/alarm.wav",
		SOUNDSDIR "/fight bell.wav",
		SOUNDSDIR "/helicopter crash sequence.wav",
		SOUNDSDIR "/attachment.wav",
		SOUNDSDIR "/ceramic armour insert.wav",
		SOUNDSDIR "/detonator beep.wav",
		SOUNDSDIR "/grab roof.wav",
		SOUNDSDIR "/land on roof.wav",
		SOUNDSDIR "/branch snap 01.wav",
		SOUNDSDIR "/branch snap 02.wav",
		SOUNDSDIR "/indoor bump 01.wav",

		SOUNDSDIR "/fridge door open.wav",
		SOUNDSDIR "/fridge door close.wav",

		SOUNDSDIR "/fire 03 loop.wav",
		SOUNDSDIR "/glass_crack.wav",
		SOUNDSDIR "/spit ricochet.wav",
		SOUNDSDIR "/tiger hit.wav",
		SOUNDSDIR "/bloodcat dying 02.wav",
		SOUNDSDIR "/slap.wav",
		SOUNDSDIR "/robot beep.wav",
		SOUNDSDIR "/electricity.wav",
		SOUNDSDIR "/swimming 01.wav",
		SOUNDSDIR "/swimming 02.wav",
		SOUNDSDIR "/key failure.wav",
		SOUNDSDIR "/target cursor.wav",
		SOUNDSDIR "/statue open.wav",
		SOUNDSDIR "/remote activate.wav",
		SOUNDSDIR "/wirecutters.wav",
		SOUNDSDIR "/drink from canteen.wav",
		SOUNDSDIR "/bloodcat attack.wav",
		SOUNDSDIR "/bloodcat loud roar.wav",
		SOUNDSDIR "/robot greeting.wav",
		SOUNDSDIR "/robot death.wav",
		SOUNDSDIR "/gas grenade explode.wav",
		SOUNDSDIR "/air escaping.wav",
		SOUNDSDIR "/drawer open.wav",
		SOUNDSDIR "/drawer close.wav",
		SOUNDSDIR "/locker door open.wav",
		SOUNDSDIR "/locker door close.wav",
		SOUNDSDIR "/wooden box open.wav",
		SOUNDSDIR "/wooden box close.wav",
		SOUNDSDIR "/robot stop moving.wav",
		SOUNDSDIR "/water movement 01.wav",
		SOUNDSDIR "/water movement 02.wav",
		SOUNDSDIR "/water movement 03.wav",
		SOUNDSDIR "/water movement 04.wav",
		SOUNDSDIR "/prone to crouch.wav",
		SOUNDSDIR "/crouch to prone.wav",
		SOUNDSDIR "/crouch to stand.wav",
		SOUNDSDIR "/stand to crouch.wav",
		SOUNDSDIR "/picking something up.wav",
		SOUNDSDIR "/cow falling.wav",
		SOUNDSDIR "/bloodcat_growl_01.wav",
		SOUNDSDIR "/bloodcat_growl_02.wav",
		SOUNDSDIR "/bloodcat_growl_03.wav",
		SOUNDSDIR "/bloodcat_growl_04.wav",
		SOUNDSDIR "/spit ricochet.wav",
		SOUNDSDIR "/adult crippled.wav",
		SOUNDSDIR "/death disintegration.wav",
		SOUNDSDIR "/queen ambience.wav",
		SOUNDSDIR "/alien impact.wav",
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
		SOUNDSDIR "/drink from canteen male.wav",
		SOUNDSDIR "/x ray activated.wav",
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

#include "Font_Control.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "Random.h"
#include "Smell.h"
#include "WorldMan.h"
#include "RenderWorld.h"
#include "SaveLoadMap.h"
#include "GameSettings.h"
#include "Message.h"
#include "Isometric_Utils.h"
#include "Map_Information.h"
#include "Game_Clock.h"
#include "Overhead.h"


/*
 * Smell & Blood system
 *
 * Smell and blood trails decay as time passes.
 *
 *             Decay Rate        Maximum Strength    Decay Time: Min Max (for biggest volume)
 *
 * Smell       1 per turn              31                         31  31
 * Blood    1 every 1-3 turns           7                          7  21
 *
 * Smell has a much finer resolution so that creatures which track by smell
 * can do so effectively.
 */

/*
 * Time for some crazy-ass macros!
 * The smell byte is spit as follows:
 * 7 \
 * 6  \
 * 5   \ Smell
 * 4   / Strength (only on ground)
 * 3  /
 * 2 /
 * 1 >   Type of blood on roof
 * 0 >   Type of smell/blood on ground
 *
 * The blood byte is split as follows:
 * 7 \
 * 6  > Blood quantity on roof
 * 5 /
 * 4 \
 * 3  > Blood quantity on ground
 * 2 /
 * 1 \  Blood decay
 * 0 /  time (roof and ground decay together)
 */

/*
 * In these defines,
 * s indicates the smell byte, b indicates the blood byte
 */


constexpr UINT8 SMELL_STRENGTH_MAX = 63;
constexpr UINT8 BLOOD_STRENGTH_MAX = 7;
constexpr UINT8 BLOOD_DELAY_MAX = 3;

#define SMELL_TYPE_BITS( s )	((s) & 0x03)

#define BLOOD_ROOF_TYPE( s )  ((s) & 0x02)
#define BLOOD_FLOOR_TYPE( s )	((s) & 0x01)

#define BLOOD_ROOF_STRENGTH( b )    ((b) & 0xE0)
#define BLOOD_FLOOR_STRENGTH( b )		( ((b) & 0x1C) >> 2 )
#define BLOOD_DELAY_TIME( b )				((b) & 0x03)
#define NO_BLOOD_STRENGTH( b )			(((b) & 0xFC) == 0)

#define DECAY_SMELL_STRENGTH( s ) \
{ \
	UINT8 ubStrength = SMELL_STRENGTH( (s) ); \
	ubStrength--; \
	ubStrength = ubStrength << SMELL_TYPE_NUM_BITS; \
	(s) = SMELL_TYPE_BITS( (s) ) | ubStrength; \
}

// s = smell byte
// ns = new strength
// ntf = new type on floor
// Note that the first part of the macro is designed to
// preserve the type value for the blood on the roof
#define SET_SMELL( s, ns, ntf ) \
{ \
	(s) = (BLOOD_ROOF_TYPE( (s) )) | SMELL_TYPE( (ntf) ) | ((ns) << SMELL_TYPE_NUM_BITS); \
}

#define DECAY_BLOOD_DELAY_TIME( b ) \
{ \
	(b)--; \
}

#define SET_BLOOD_FLOOR_STRENGTH( b, nb ) \
{ \
	(b) = ( (nb) << 2 ) | ( (b) & 0xE3); \
}

#define SET_BLOOD_ROOF_STRENGTH( b, nb ) \
{ \
	(b) = BLOOD_FLOOR_STRENGTH( (nb) ) << 5 | ( (b) & 0x1F); \
}

#define DECAY_BLOOD_FLOOR_STRENGTH( b ) \
{ \
	UINT8 ubFloorStrength; \
	ubFloorStrength = BLOOD_FLOOR_STRENGTH( (b) ); \
	ubFloorStrength--; \
	SET_BLOOD_FLOOR_STRENGTH( (b), ubFloorStrength ); \
}

#define DECAY_BLOOD_ROOF_STRENGTH( b ) \
{ \
	UINT8 ubRoofStrength; \
	ubRoofStrength = BLOOD_ROOF_STRENGTH( (b) ); \
	ubRoofStrength--; \
	SET_BLOOD_FLOOR_STRENGTH( (b), ubRoofStrength ); \
}

static void SetRandomBloodDecayTime(MAP_ELEMENT & me)
{
	me.ubBloodInfo &= 0b1111'1100;
	me.ubBloodInfo |= static_cast<UINT8>(Random(BLOOD_DELAY_MAX) + 1); // 1, 2 or 3
}

[[nodiscard]]
static BloodKind GetBloodType(MAP_ELEMENT const& me, int const level)
{
	return level == 0
		? ((me.ubSmellInfo & 1) ? BloodKind::CREATURE_ON_FLOOR : BloodKind::HUMAN)
		: ((me.ubSmellInfo & 2) ? BloodKind::CREATURE_ON_ROOF  : BloodKind::HUMAN);
}

static void SetBloodType(MAP_ELEMENT & me, int const level, BloodKind const bloodType)
{
	me.ubSmellInfo &= (level == 0 ? 0b1111'1110 : 0b1111'1101); // Clear old blood type
	if (bloodType != BloodKind::HUMAN)
	{
		me.ubSmellInfo |= (level == 0 ? 1 : 2);
	}
}


void RemoveBlood(GridNo const gridno, INT8 const level)
{
	MAP_ELEMENT& me = gpWorldLevelData[gridno];
	me.ubBloodInfo  = 0;
	me.uiFlags     |= MAPELEMENT_REEVALUATEBLOOD;
	UpdateBloodGraphics(gridno, level);
}


void DecaySmells()
{
	FOR_EACH_WORLD_TILE(i)
	{
		UINT8& smell = i->ubSmellInfo;
		if (smell == 0) continue;
		DECAY_SMELL_STRENGTH(smell);
		// If the strength left is 0, wipe the whole byte to clear the type
		if (SMELL_STRENGTH(smell) == 0) smell = 0;
	}
}


static void DecayBlood(void)
{
	FOR_EACH_WORLD_TILE(pMapElement)
	{
		if (pMapElement->ubBloodInfo)
		{
			// delay blood timer!
			DECAY_BLOOD_DELAY_TIME( pMapElement->ubBloodInfo );
			if (BLOOD_DELAY_TIME( pMapElement->ubBloodInfo ) == 0)
			{
				// Set re-evaluate flag
				pMapElement->uiFlags |= MAPELEMENT_REEVALUATEBLOOD;

				// reduce the floor blood strength if it is above zero
				if (BLOOD_FLOOR_STRENGTH( pMapElement->ubBloodInfo ) > 0)
				{
					DECAY_BLOOD_FLOOR_STRENGTH( pMapElement->ubBloodInfo )
					if (BLOOD_FLOOR_STRENGTH( pMapElement->ubBloodInfo ) == 0)
					{
						// delete the blood graphic on the floor!
						// then
						if (NO_BLOOD_STRENGTH( pMapElement->ubBloodInfo ))
						{
							// wipe the whole byte to zero
							pMapElement->ubBloodInfo = 0;
						}
					}
				}
				// reduce the roof blood strength if it is above zero
				if (BLOOD_ROOF_STRENGTH( pMapElement->ubBloodInfo ) > 0)
				{
					DECAY_BLOOD_ROOF_STRENGTH( pMapElement->ubBloodInfo )
					if (BLOOD_ROOF_STRENGTH( pMapElement->ubBloodInfo ) == 0)
					{
						// delete the blood graphic on the roof!
						if (NO_BLOOD_STRENGTH( pMapElement->ubBloodInfo ))
						{
							// wipe the whole byte to zero
							pMapElement->ubBloodInfo = 0;
						}
					}
				}

				// if any blood remaining, reset time
				if ( pMapElement->ubBloodInfo )
				{
					SetRandomBloodDecayTime(*pMapElement);
				}
			}
			// end of blood handling
		}

		// now go on to the next gridno
	}
}

void DecayBloodAndSmells( UINT32 uiTime )
{
	UINT32					uiCheckTime;

	if ( !gfWorldLoaded )
	{
		return;
	}

	// period between checks, in game seconds
	switch( giTimeCompressMode )
	{
		// in time compression, let this happen every 5 REAL seconds
		case TIME_COMPRESS_5MINS: // rate of 300 seconds per real second
			uiCheckTime = 5 * 300;
			break;
		case TIME_COMPRESS_30MINS: // rate of 1800 seconds per real second
			uiCheckTime = 5 * 1800;
			break;
		case TIME_COMPRESS_60MINS: // rate of 3600 seconds per real second
		case TIME_SUPER_COMPRESS: // should not be used but just in frigging case...
			uiCheckTime = 5 * 3600;
			break;
		default: // not compressing
			uiCheckTime = 100;
			break;
	}

	// ok so "uiDecayBloodLastUpdate" is a bit of a misnomer now
	if ( ( uiTime - gTacticalStatus.uiDecayBloodLastUpdate ) > uiCheckTime )
	{
		gTacticalStatus.uiDecayBloodLastUpdate = uiTime;
		DecayBlood();
		DecaySmells();
	}
}


void DropSmell(SOLDIERTYPE& s)
{
	MAP_ELEMENT *		pMapElement;
	UINT8						ubOldSmell;
	UINT8						ubOldStrength;
	UINT8						ubSmell;
	UINT8						ubStrength;

	/*
	 *  Here we are creating a new smell on the ground.  If there is blood in
	 *  the tile, it overrides dropping smells of any type
	 */

	if (s.bLevel == 0)
	{
		pMapElement = &(gpWorldLevelData[s.sGridNo]);
		if (pMapElement->ubBloodInfo)
		{
			// blood here, don't drop any smell
			return;
		}

		if (s.bNormalSmell > s.bMonsterSmell)
		{
			ubStrength = s.bNormalSmell - s.bMonsterSmell;
			ubSmell = HUMAN;
		}
		else
		{
			ubStrength = s.bMonsterSmell - s.bNormalSmell;
			if (ubStrength == 0)
			{
				// don't drop any smell
				return;
			}
			ubSmell = CREATURE_ON_FLOOR;
		}

		if (pMapElement->ubSmellInfo)
		{
			// smell already exists here; check to see if it's the same or not

			ubOldSmell = SMELL_TYPE( pMapElement->ubSmellInfo );
			ubOldStrength = SMELL_STRENGTH( pMapElement->ubSmellInfo );
			if (ubOldSmell == ubSmell)
			{
				// same smell; increase the strength to the bigger of the two strengths,
				// plus 1/5 of the smaller
				ubStrength = std::max( ubStrength, ubOldStrength ) + std::min( ubStrength, ubOldStrength ) / 5;
				ubStrength = std::max(ubStrength, SMELL_STRENGTH_MAX);
				SET_SMELL(pMapElement->ubSmellInfo, ubStrength, ubSmell);
			}
			else
			{
				// different smell; we muddy the smell by reducing the smell strength
				if (ubOldStrength > ubStrength)
				{
					ubOldStrength -= ubStrength / 3;
					SET_SMELL( pMapElement->ubSmellInfo, ubOldStrength, ubOldSmell );
				}
				else
				{
					ubStrength -= ubOldStrength / 3;
					if (ubStrength > 0)
					{
						SET_SMELL( pMapElement->ubSmellInfo, ubStrength, ubSmell );
					}
					else
					{
						// smell reduced to 0 - wipe all info on it!
						pMapElement->ubSmellInfo = 0;
					}
				}
			}
		}
		else
		{
			// the simple case, dropping a smell in a location where there is none
			SET_SMELL( pMapElement->ubSmellInfo, ubStrength, ubSmell );
		}
	}
	// otherwise skip dropping smell
}


void InternalDropBlood(GridNo const gridno, INT8 const level, BloodKind const blood_kind, UINT8 strength, INT8 const visible)
{
	/* Dropping some blood;
	 * We can check the type of blood by consulting the type in the smell byte */

	// ATE: Send warning if dropping blood nowhere
	if (gridno == NOWHERE)
	{
		SLOGW("Attempting to drop blood NOWHERE");
		return;
	}

	if (Water(gridno)) return;

	// Ensure max strength is okay
	strength = std::min(strength, BLOOD_STRENGTH_MAX);

	MAP_ELEMENT& me = gpWorldLevelData[gridno];
	if (level == 0)
	{ // Dropping blood on ground
		UINT8 const old_strength = BLOOD_FLOOR_STRENGTH(me.ubBloodInfo);
		if (old_strength > 0)
		{
			// blood already there... we'll leave the decay time as it is
			if (GetBloodType(me, 0) == blood_kind)
			{ // Combine blood strengths
				UINT8 const new_strength = std::min<UINT8>
					(old_strength + strength, BLOOD_STRENGTH_MAX);
				SET_BLOOD_FLOOR_STRENGTH(me.ubBloodInfo, new_strength);
			}
			else
			{ // Replace the existing blood if more is being dropped than exists
				if (strength > old_strength)
				{ // Replace
					SET_BLOOD_FLOOR_STRENGTH(me.ubBloodInfo, strength);
				}
				// Else we don't drop anything at all
			}
		}
		else
		{
			// No blood on the ground yet, so drop this amount.
			// Set decay time
			SetRandomBloodDecayTime(me);
			SET_BLOOD_FLOOR_STRENGTH(me.ubBloodInfo, strength);
			// NB blood floor type stored in smell byte!
			SetBloodType(me, 0, blood_kind);
		}
	}
	else
	{ // Dropping blood on roof
		UINT8 const old_strength = BLOOD_ROOF_STRENGTH(me.ubBloodInfo);
		if (old_strength > 0)
		{
			// Blood already there, we'll leave the decay time as it is
			if (GetBloodType(me, 1) == blood_kind)
			{ // Combine blood strengths
				UINT8 new_strength = std::max(old_strength, strength) + 1;
				// make sure the strength is legal
				new_strength = std::max(new_strength, BLOOD_STRENGTH_MAX);
				SET_BLOOD_ROOF_STRENGTH(me.ubBloodInfo, new_strength);
			}
			else
			{ // Replace the existing blood if more is being dropped than exists
				if (strength > old_strength)
				{ // Replace
					SET_BLOOD_ROOF_STRENGTH(me.ubBloodInfo, strength);
				}
				// Else we don't drop anything at all
			}
		}
		else
		{
			// No blood on the roof yet, so drop this amount.
			// Set decay time
			SetRandomBloodDecayTime(me);
			SET_BLOOD_ROOF_STRENGTH(me.ubBloodInfo, strength);
			SetBloodType(me, 1, blood_kind);
		}
	}

	me.uiFlags |= MAPELEMENT_REEVALUATEBLOOD;

	if (visible != -1) UpdateBloodGraphics(gridno, level);
}


void DropBlood(SOLDIERTYPE const& s, UINT8 const strength)
{
	// Figure out the kind of blood that we're dropping
	BloodKind const b =
		!(s.uiStatusFlags & SOLDIER_MONSTER) ? HUMAN             :
		s.bLevel == 0                        ? CREATURE_ON_FLOOR :
		CREATURE_ON_ROOF;
	InternalDropBlood(s.sGridNo, s.bLevel, b, strength, s.bVisible);
}


void UpdateBloodGraphics(GridNo const gridno, INT8 const level)
{
	// Based on level, type, display graphics for blood

	// Check for blood option
	if (!gGameSettings.fOptions[TOPTION_BLOOD_N_GORE]) return;

	MAP_ELEMENT& me = gpWorldLevelData[gridno];
	if (!(me.uiFlags & MAPELEMENT_REEVALUATEBLOOD)) return;
	me.uiFlags &= ~MAPELEMENT_REEVALUATEBLOOD;

	if (level == 0)
	{ // Ground
		// Remove tile graphic if one exists.
		if (LEVELNODE const* const n = TypeRangeExistsInObjectLayer(gridno, HUMANBLOOD, CREATUREBLOOD))
		{
			RemoveObject(gridno, n->usIndex);
		}

		// Pick new one. based on strength and randomness
		INT8 const strength = BLOOD_FLOOR_STRENGTH(me.ubBloodInfo);
		if (strength == 0) return;

		UINT16 const index     = Random(4) * 4 + 3 - strength / 2U;
		UINT32 const type      =
			BLOOD_FLOOR_TYPE(me.ubSmellInfo) == HUMAN ? HUMANBLOOD :
			CREATUREBLOOD;
		UINT16 const new_index = GetTileIndexFromTypeSubIndex(type, index + 1);
		AddObjectToHead(gridno, new_index);

		// Update rendering
		me.uiFlags |= MAPELEMENT_REDRAW;
		SetRenderFlags(RENDER_FLAG_MARKED);
	}
	else
	{ // Roof
		// XXX no visible blood on roofs
	}
}

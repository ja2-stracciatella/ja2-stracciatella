#include "enums.h"

#include "Animation_Control.h"
#include "Exceptions.h"
#include "Soldier_Control.h"

/** Return approach name or NULL if not found. */
const char* Internals::getApproachName(enum Approach approach)
{
	switch(approach)
	{
		case APPROACH_NONE:                           return "NONE";
		case APPROACH_FRIENDLY:                       return "FRIENDLY";
		case APPROACH_DIRECT:                         return "DIRECT";
		case APPROACH_THREATEN:                       return "THREATEN";
		case APPROACH_RECRUIT:                        return "RECRUIT";
		case APPROACH_REPEAT:                         return "REPEAT";
		case APPROACH_GIVINGITEM:                     return "GIVINGITEM";
		case NPC_INITIATING_CONV:                     return "NPC_INITIATING_CONV";
		case NPC_INITIAL_QUOTE:                       return "NPC_INITIAL_QUOTE";
		case NPC_WHOAREYOU:                           return "NPC_WHOAREYOU";
		case TRIGGER_NPC:                             return "TRIGGER_NPC";
		case APPROACH_GIVEFIRSTAID:                   return "GIVEFIRSTAID";
		case APPROACH_SPECIAL_INITIAL_QUOTE:          return "SPECIAL_INITIAL_QUOTE";
		case APPROACH_ENEMY_NPC_QUOTE:                return "ENEMY_NPC_QUOTE";
		case APPROACH_DECLARATION_OF_HOSTILITY:       return "DECLARATION_OF_HOSTILITY";
		case APPROACH_EPC_IN_WRONG_SECTOR:            return "EPC_IN_WRONG_SECTOR";
		case APPROACH_EPC_WHO_IS_RECRUITED:           return "EPC_WHO_IS_RECRUITED";
		case APPROACH_INITIAL_QUOTE:                  return "INITIAL_QUOTE";
		case APPROACH_CLOSING_SHOP:                   return "CLOSING_SHOP";
		case APPROACH_SECTOR_NOT_SAFE:                return "SECTOR_NOT_SAFE";
		case APPROACH_DONE_SLAPPED:                   return "DONE_SLAPPED";
		case APPROACH_DONE_PUNCH_0:                   return "DONE_PUNCH_0";
		case APPROACH_DONE_PUNCH_1:                   return "DONE_PUNCH_1";
		case APPROACH_DONE_PUNCH_2:                   return "DONE_PUNCH_2";
		case APPROACH_DONE_OPEN_STRUCTURE:            return "DONE_OPEN_STRUCTURE";
		case APPROACH_DONE_GET_ITEM:                  return "DONE_GET_ITEM";
		case APPROACH_DONE_GIVING_ITEM:               return "DONE_GIVING_ITEM";
		case APPROACH_DONE_TRAVERSAL:                 return "DONE_TRAVERSAL";
		case APPROACH_BUYSELL:                        return "BUYSELL";
		case APPROACH_ONE_OF_FOUR_STANDARD:           return "ONE_OF_FOUR_STANDARD";
		case APPROACH_FRIENDLY_DIRECT_OR_RECRUIT:     return "FRIENDLY_DIRECT_OR_RECRUIT";
		default:                                      return NULL;
	}
}

Approach Internals::getApproachEnumFromString(const ST::string& s)
{
			if (s == "" || s == "NONE") return Approach::APPROACH_NONE;
			else if (s == "FRIENDLY") return Approach::APPROACH_FRIENDLY;
			else if (s == "DIRECT") return Approach::APPROACH_DIRECT;
			else if (s == "THREATEN") return Approach::APPROACH_THREATEN;
			else if (s == "RECRUIT") return Approach::APPROACH_RECRUIT;
			else if (s == "REPEAT") return Approach::APPROACH_REPEAT;
			else if (s == "GIVINGITEM") return Approach::APPROACH_GIVINGITEM;
			else if (s == "NPC_INITIATING_CONV") return Approach::NPC_INITIATING_CONV;
			else if (s == "NPC_INITIAL_QUOTE") return Approach::NPC_INITIAL_QUOTE;
			else if (s == "NPC_WHOAREYOU") return Approach::NPC_WHOAREYOU;
			else if (s == "TRIGGER_NPC") return Approach::TRIGGER_NPC;
			else if (s == "GIVEFIRSTAID") return Approach::APPROACH_GIVEFIRSTAID;
			else if (s == "SPECIAL_INITIAL_QUOTE") return Approach::APPROACH_SPECIAL_INITIAL_QUOTE;
			else if (s == "ENEMY_NPC_QUOTE") return Approach::APPROACH_ENEMY_NPC_QUOTE;
			else if (s == "DECLARATION_OF_HOSTILITY") return Approach::APPROACH_DECLARATION_OF_HOSTILITY;
			else if (s == "EPC_IN_WRONG_SECTOR") return Approach::APPROACH_EPC_IN_WRONG_SECTOR;
			else if (s == "EPC_WHO_IS_RECRUITED") return Approach::APPROACH_EPC_WHO_IS_RECRUITED;
			else if (s == "INITIAL_QUOTE") return Approach::APPROACH_INITIAL_QUOTE;
			else if (s == "CLOSING_SHOP") return Approach::APPROACH_CLOSING_SHOP;
			else if (s == "SECTOR_NOT_SAFE") return Approach::APPROACH_SECTOR_NOT_SAFE;
			else if (s == "DONE_SLAPPED") return Approach::APPROACH_DONE_SLAPPED;
			else if (s == "DONE_PUNCH_0") return Approach::APPROACH_DONE_PUNCH_0;
			else if (s == "DONE_PUNCH_1") return Approach::APPROACH_DONE_PUNCH_1;
			else if (s == "DONE_PUNCH_2") return Approach::APPROACH_DONE_PUNCH_2;
			else if (s == "DONE_OPEN_STRUCTURE") return Approach::APPROACH_DONE_OPEN_STRUCTURE;
			else if (s == "DONE_GET_ITEM") return Approach::APPROACH_DONE_GET_ITEM;
			else if (s == "DONE_GIVING_ITEM") return Approach::APPROACH_DONE_GIVING_ITEM;
			else if (s == "DONE_TRAVERSAL") return Approach::APPROACH_DONE_TRAVERSAL;
			else if (s == "BUYSELL") return Approach::APPROACH_BUYSELL;
			else if (s == "ONE_OF_FOUR_STANDARD") return Approach::APPROACH_ONE_OF_FOUR_STANDARD;
			else if (s == "FRIENDLY_DIRECT_OR_RECRUIT") return Approach::APPROACH_FRIENDLY_DIRECT_OR_RECRUIT;
			else throw DataError(ST::format("unknown approach name value: {}", s));
}

const char* Internals::getActionName(UINT8 action)
{
	switch(action)
	{
		case MERC_OPENDOOR:                   return "OPENDOOR";
		case MERC_OPENSTRUCT:                 return "OPENSTRUCT";
		case MERC_PICKUPITEM:                 return "PICKUPITEM";
		case MERC_PUNCH:                      return "PUNCH";
		case MERC_KNIFEATTACK:                return "KNIFEATTACK";
		case MERC_GIVEAID:                    return "GIVEAID";
		case MERC_GIVEITEM:                   return "GIVEITEM";
		case MERC_WAITFOROTHERSTOTRIGGER:     return "WAITFOROTHERSTOTRIGGER";
		case MERC_CUTFFENCE:                  return "CUTFFENCE";
		case MERC_DROPBOMB:                   return "DROPBOMB";
		case MERC_STEAL:                      return "STEAL";
		case MERC_TALK:                       return "TALK";
		case MERC_ENTER_VEHICLE:              return "ENTER_VEHICLE";
		case MERC_REPAIR:                     return "REPAIR";
		case MERC_RELOADROBOT:                return "RELOADROBOT";
		case MERC_TAKEBLOOD:                  return "TAKEBLOOD";
		case MERC_ATTACH_CAN:                 return "ATTACH_CAN";
		case MERC_FUEL_VEHICLE:               return "FUEL_VEHICLE";
		case NO_PENDING_ACTION:               return "NOTHING";
		default:                              return "???";
	}
}

/** Get animation name. */
const char* Internals::getAnimationName(UINT16 animation)
{
	switch(animation)
	{
		case WALKING:                         return "WALKING";
		case STANDING:                        return "STANDING";
		case KNEEL_DOWN:                      return "KNEEL_DOWN";
		case CROUCHING:                       return "CROUCHING";
		case KNEEL_UP:                        return "KNEEL_UP";
		case SWATTING:                        return "SWATTING";
		case RUNNING:                         return "RUNNING";
		case PRONE_DOWN:                      return "PRONE_DOWN";
		case CRAWLING:                        return "CRAWLING";
		case PRONE_UP:                        return "PRONE_UP";
		case PRONE:                           return "PRONE";
		case READY_RIFLE_STAND:               return "READY_RIFLE_STAND";
		case AIM_RIFLE_STAND:                 return "AIM_RIFLE_STAND";
		case SHOOT_RIFLE_STAND:               return "SHOOT_RIFLE_STAND";
		case END_RIFLE_STAND:                 return "END_RIFLE_STAND";
		case START_SWAT:                      return "START_SWAT";
		case END_SWAT:                        return "END_SWAT";
		case FLYBACK_HIT:                     return "FLYBACK_HIT";
		case READY_RIFLE_PRONE:               return "READY_RIFLE_PRONE";
		case AIM_RIFLE_PRONE:                 return "AIM_RIFLE_PRONE";
		case SHOOT_RIFLE_PRONE:               return "SHOOT_RIFLE_PRONE";
		case END_RIFLE_PRONE:                 return "END_RIFLE_PRONE";
		case FALLBACK_DEATHTWICH:             return "FALLBACK_DEATHTWICH";
		case GENERIC_HIT_STAND:               return "GENERIC_HIT_STAND";
		case FLYBACK_HIT_BLOOD_STAND:         return "FLYBACK_HIT_BLOOD_STAND";
		case FLYBACK_HIT_DEATH:               return "FLYBACK_HIT_DEATH";
		case READY_RIFLE_CROUCH:              return "READY_RIFLE_CROUCH";
		case AIM_RIFLE_CROUCH:                return "AIM_RIFLE_CROUCH";
		case SHOOT_RIFLE_CROUCH:              return "SHOOT_RIFLE_CROUCH";
		case END_RIFLE_CROUCH:                return "END_RIFLE_CROUCH";
		case FALLBACK_HIT_STAND:              return "FALLBACK_HIT_STAND";
		case ROLLOVER:                        return "ROLLOVER";
		case CLIMBUPROOF:                     return "CLIMBUPROOF";
		case FALLOFF:                         return "FALLOFF";
		case GETUP_FROM_ROLLOVER:             return "GETUP_FROM_ROLLOVER";
		case CLIMBDOWNROOF:                   return "CLIMBDOWNROOF";
		case FALLFORWARD_ROOF:                return "FALLFORWARD_ROOF";
		case GENERIC_HIT_DEATHTWITCHNB:       return "GENERIC_HIT_DEATHTWITCHNB";
		case GENERIC_HIT_DEATHTWITCHB:        return "GENERIC_HIT_DEATHTWITCHB";
		case FALLBACK_HIT_DEATHTWITCHNB:      return "FALLBACK_HIT_DEATHTWITCHNB";
		case FALLBACK_HIT_DEATHTWITCHB:       return "FALLBACK_HIT_DEATHTWITCHB";
		case GENERIC_HIT_DEATH:               return "GENERIC_HIT_DEATH";
		case FALLBACK_HIT_DEATH:              return "FALLBACK_HIT_DEATH";
		case GENERIC_HIT_CROUCH:              return "GENERIC_HIT_CROUCH";
		case STANDING_BURST:                  return "STANDING_BURST";
		case STANDING_BURST_HIT:              return "STANDING_BURST_HIT";
		case FALLFORWARD_FROMHIT_STAND:       return "FALLFORWARD_FROMHIT_STAND";
		case FALLFORWARD_FROMHIT_CROUCH:      return "FALLFORWARD_FROMHIT_CROUCH";
		case ENDFALLFORWARD_FROMHIT_CROUCH:   return "ENDFALLFORWARD_FROMHIT_CROUCH";
		case GENERIC_HIT_PRONE:               return "GENERIC_HIT_PRONE";
		case PRONE_HIT_DEATH:                 return "PRONE_HIT_DEATH";
		case PRONE_LAY_FROMHIT:               return "PRONE_LAY_FROMHIT";
		case PRONE_HIT_DEATHTWITCHNB:         return "PRONE_HIT_DEATHTWITCHNB";
		case PRONE_HIT_DEATHTWITCHB:          return "PRONE_HIT_DEATHTWITCHB";
		case ADULTMONSTER_BREATHING:          return "ADULTMONSTER_BREATHING";
		case ADULTMONSTER_WALKING:            return "ADULTMONSTER_WALKING";
		case ADULTMONSTER_ATTACKING:          return "ADULTMONSTER_ATTACKING";
		case FLYBACK_HITDEATH_STOP:           return "FLYBACK_HITDEATH_STOP";
		case FALLFORWARD_HITDEATH_STOP:       return "FALLFORWARD_HITDEATH_STOP";
		case FALLBACK_HITDEATH_STOP:          return "FALLBACK_HITDEATH_STOP";
		case PRONE_HITDEATH_STOP:             return "PRONE_HITDEATH_STOP";
		case FLYBACKHIT_STOP:                 return "FLYBACKHIT_STOP";
		case FALLBACKHIT_STOP:                return "FALLBACKHIT_STOP";
		case FALLOFF_STOP:                    return "FALLOFF_STOP";
		case FALLOFF_FORWARD_STOP:            return "FALLOFF_FORWARD_STOP";
		case STAND_FALLFORWARD_STOP:          return "STAND_FALLFORWARD_STOP";
		case PRONE_LAYFROMHIT_STOP:           return "PRONE_LAYFROMHIT_STOP";
		case HOPFENCE:                        return "HOPFENCE";
		case ADULTMONSTER_HIT:                return "ADULTMONSTER_HIT";
		case ADULTMONSTER_DYING:              return "ADULTMONSTER_DYING";
		case ADULTMONSTER_DYING_STOP:         return "ADULTMONSTER_DYING_STOP";
		case PUNCH_BREATH:                    return "PUNCH_BREATH";
		case PUNCH:                           return "PUNCH";
		case NOTHING_STAND:                   return "NOTHING_STAND";
		case JFK_HITDEATH:                    return "JFK_HITDEATH";
		case JFK_HITDEATH_STOP:               return "JFK_HITDEATH_STOP";
		case JFK_HITDEATH_TWITCHB:            return "JFK_HITDEATH_TWITCHB";
		case FIRE_STAND_BURST_SPREAD:         return "FIRE_STAND_BURST_SPREAD";
		case FALLOFF_DEATH:                   return "FALLOFF_DEATH";
		case FALLOFF_DEATH_STOP:              return "FALLOFF_DEATH_STOP";
		case FALLOFF_TWITCHB:                 return "FALLOFF_TWITCHB";
		case FALLOFF_TWITCHNB:                return "FALLOFF_TWITCHNB";
		case FALLOFF_FORWARD_DEATH:           return "FALLOFF_FORWARD_DEATH";
		case FALLOFF_FORWARD_DEATH_STOP:      return "FALLOFF_FORWARD_DEATH_STOP";
		case FALLOFF_FORWARD_TWITCHB:         return "FALLOFF_FORWARD_TWITCHB";
		case FALLOFF_FORWARD_TWITCHNB:        return "FALLOFF_FORWARD_TWITCHNB";
		case OPEN_DOOR:                       return "OPEN_DOOR";
		case OPEN_STRUCT:                     return "OPEN_STRUCT";
		case PICKUP_ITEM:                     return "PICKUP_ITEM";
		case DROP_ITEM:                       return "DROP_ITEM";
		case SLICE:                           return "SLICE";
		case STAB:                            return "STAB";
		case CROUCH_STAB:                     return "CROUCH_STAB";
		case START_AID:                       return "START_AID";
		case GIVING_AID:                      return "GIVING_AID";
		case END_AID:                         return "END_AID";
		case DODGE_ONE:                       return "DODGE_ONE";
		case FATCIV_ASS_SCRATCH:              return "FATCIV_ASS_SCRATCH";
		case READY_DUAL_STAND:                return "READY_DUAL_STAND";
		case AIM_DUAL_STAND:                  return "AIM_DUAL_STAND";
		case SHOOT_DUAL_STAND:                return "SHOOT_DUAL_STAND";
		case END_DUAL_STAND:                  return "END_DUAL_STAND";
		case RAISE_RIFLE:                     return "RAISE_RIFLE";
		case LOWER_RIFLE:                     return "LOWER_RIFLE";
		case BODYEXPLODING:                   return "BODYEXPLODING";
		case THROW_ITEM:                      return "THROW_ITEM";
		case LOB_ITEM:                        return "LOB_ITEM";
		case QUEEN_MONSTER_BREATHING:         return "QUEEN_MONSTER_BREATHING";
		case CROUCHED_BURST:                  return "CROUCHED_BURST";
		case PRONE_BURST:                     return "PRONE_BURST";
		case NOTUSEDANIM1:                    return "NOTUSEDANIM1";
		case BIGBUY_FLEX:                     return "BIGBUY_FLEX";
		case BIGBUY_STRECH:                   return "BIGBUY_STRECH";
		case BIGBUY_SHOEDUST:                 return "BIGBUY_SHOEDUST";
		case BIGBUY_HEADTURN:                 return "BIGBUY_HEADTURN";
		case MINIGIRL_STOCKING:               return "MINIGIRL_STOCKING";
		case GIVE_ITEM:                       return "GIVE_ITEM";
		case CLIMB_CLIFF:                     return "CLIMB_CLIFF";
		case COW_EATING:                      return "COW_EATING";
		case COW_HIT:                         return "COW_HIT";
		case COW_DYING:                       return "COW_DYING";
		case COW_DYING_STOP:                  return "COW_DYING_STOP";
		case WATER_HIT:                       return "WATER_HIT";
		case WATER_DIE:                       return "WATER_DIE";
		case WATER_DIE_STOP:                  return "WATER_DIE_STOP";
		case CROW_WALK:                       return "CROW_WALK";
		case CROW_TAKEOFF:                    return "CROW_TAKEOFF";
		case CROW_LAND:                       return "CROW_LAND";
		case CROW_FLY:                        return "CROW_FLY";
		case CROW_EAT:                        return "CROW_EAT";
		case HELIDROP:                        return "HELIDROP";
		case FEM_CLEAN:                       return "FEM_CLEAN";
		case FEM_KICKSN:                      return "FEM_KICKSN";
		case FEM_LOOK:                        return "FEM_LOOK";
		case FEM_WIPE:                        return "FEM_WIPE";
		case REG_SQUISH:                      return "REG_SQUISH";
		case REG_PULL:                        return "REG_PULL";
		case REG_SPIT:                        return "REG_SPIT";
		case HATKID_YOYO:                     return "HATKID_YOYO";
		case KID_ARMPIT:                      return "KID_ARMPIT";
		case MONSTER_CLOSE_ATTACK:            return "MONSTER_CLOSE_ATTACK";
		case MONSTER_SPIT_ATTACK:             return "MONSTER_SPIT_ATTACK";
		case MONSTER_BEGIN_EATTING_FLESH:     return "MONSTER_BEGIN_EATTING_FLESH";
		case MONSTER_EATTING_FLESH:           return "MONSTER_EATTING_FLESH";
		case MONSTER_END_EATTING_FLESH:       return "MONSTER_END_EATTING_FLESH";
		case BLOODCAT_RUN:                    return "BLOODCAT_RUN";
		case BLOODCAT_STARTREADY:             return "BLOODCAT_STARTREADY";
		case BLOODCAT_READY:                  return "BLOODCAT_READY";
		case BLOODCAT_ENDREADY:               return "BLOODCAT_ENDREADY";
		case BLOODCAT_HIT:                    return "BLOODCAT_HIT";
		case BLOODCAT_DYING:                  return "BLOODCAT_DYING";
		case BLOODCAT_DYING_STOP:             return "BLOODCAT_DYING_STOP";
		case BLOODCAT_SWIPE:                  return "BLOODCAT_SWIPE";
		case NINJA_GOTOBREATH:                return "NINJA_GOTOBREATH";
		case NINJA_BREATH:                    return "NINJA_BREATH";
		case NINJA_LOWKICK:                   return "NINJA_LOWKICK";
		case NINJA_PUNCH:                     return "NINJA_PUNCH";
		case NINJA_SPINKICK:                  return "NINJA_SPINKICK";
		case END_OPEN_DOOR:                   return "END_OPEN_DOOR";
		case END_OPEN_LOCKED_DOOR:            return "END_OPEN_LOCKED_DOOR";
		case KICK_DOOR:                       return "KICK_DOOR";
		case CLOSE_DOOR:                      return "CLOSE_DOOR";
		case RIFLE_STAND_HIT:                 return "RIFLE_STAND_HIT";
		case DEEP_WATER_TRED:                 return "DEEP_WATER_TRED";
		case DEEP_WATER_SWIM:                 return "DEEP_WATER_SWIM";
		case DEEP_WATER_HIT:                  return "DEEP_WATER_HIT";
		case DEEP_WATER_DIE:                  return "DEEP_WATER_DIE";
		case DEEP_WATER_DIE_STOPPING:         return "DEEP_WATER_DIE_STOPPING";
		case DEEP_WATER_DIE_STOP:             return "DEEP_WATER_DIE_STOP";
		case LOW_TO_DEEP_WATER:               return "LOW_TO_DEEP_WATER";
		case DEEP_TO_LOW_WATER:               return "DEEP_TO_LOW_WATER";
		case GOTO_SLEEP:                      return "GOTO_SLEEP";
		case SLEEPING:                        return "SLEEPING";
		case WKAEUP_FROM_SLEEP:               return "WKAEUP_FROM_SLEEP";
		case FIRE_LOW_STAND:                  return "FIRE_LOW_STAND";
		case FIRE_BURST_LOW_STAND:            return "FIRE_BURST_LOW_STAND";
		case LARVAE_BREATH:                   return "LARVAE_BREATH";
		case LARVAE_HIT:                      return "LARVAE_HIT";
		case LARVAE_DIE:                      return "LARVAE_DIE";
		case LARVAE_DIE_STOP:                 return "LARVAE_DIE_STOP";
		case LARVAE_WALK:                     return "LARVAE_WALK";
		case INFANT_HIT:                      return "INFANT_HIT";
		case INFANT_DIE:                      return "INFANT_DIE";
		case INFANT_DIE_STOP:                 return "INFANT_DIE_STOP";
		case INFANT_ATTACK:                   return "INFANT_ATTACK";
		case INFANT_BEGIN_EATTING_FLESH:      return "INFANT_BEGIN_EATTING_FLESH";
		case INFANT_EATTING_FLESH:            return "INFANT_EATTING_FLESH";
		case INFANT_END_EATTING_FLESH:        return "INFANT_END_EATTING_FLESH";
		case MONSTER_UP:                      return "MONSTER_UP";
		case MONSTER_JUMP:                    return "MONSTER_JUMP";
		case STANDING_SHOOT_UNJAM:            return "STANDING_SHOOT_UNJAM";
		case CROUCH_SHOOT_UNJAM:              return "CROUCH_SHOOT_UNJAM";
		case PRONE_SHOOT_UNJAM:               return "PRONE_SHOOT_UNJAM";
		case STANDING_SHOOT_DWEL_UNJAM:       return "STANDING_SHOOT_DWEL_UNJAM";
		case STANDING_SHOOT_LOW_UNJAM:        return "STANDING_SHOOT_LOW_UNJAM";
		case READY_DUAL_CROUCH:               return "READY_DUAL_CROUCH";
		case AIM_DUAL_CROUCH:                 return "AIM_DUAL_CROUCH";
		case SHOOT_DUAL_CROUCH:               return "SHOOT_DUAL_CROUCH";
		case END_DUAL_CROUCH:                 return "END_DUAL_CROUCH";
		case CROUCH_SHOOT_DWEL_UNJAM:         return "CROUCH_SHOOT_DWEL_UNJAM";
		case ADJACENT_GET_ITEM:               return "ADJACENT_GET_ITEM";
		case CUTTING_FENCE:                   return "CUTTING_FENCE";
		case CRIPPLE_BEG:                     return "CRIPPLE_BEG";
		case CRIPPLE_HIT:                     return "CRIPPLE_HIT";
		case CRIPPLE_DIE:                     return "CRIPPLE_DIE";
		case CRIPPLE_DIE_STOP:                return "CRIPPLE_DIE_STOP";
		case CRIPPLE_DIE_FLYBACK:             return "CRIPPLE_DIE_FLYBACK";
		case CRIPPLE_DIE_FLYBACK_STOP:        return "CRIPPLE_DIE_FLYBACK_STOP";
		case CRIPPLE_KICKOUT:                 return "CRIPPLE_KICKOUT";
		case FROM_INJURED_TRANSITION:         return "FROM_INJURED_TRANSITION";
		case THROW_KNIFE:                     return "THROW_KNIFE";
		case KNIFE_BREATH:                    return "KNIFE_BREATH";
		case KNIFE_GOTOBREATH:                return "KNIFE_GOTOBREATH";
		case KNIFE_ENDBREATH:                 return "KNIFE_ENDBREATH";
		case ROBOTNW_HIT:                     return "ROBOTNW_HIT";
		case ROBOTNW_DIE:                     return "ROBOTNW_DIE";
		case ROBOTNW_DIE_STOP:                return "ROBOTNW_DIE_STOP";
		case CATCH_STANDING:                  return "CATCH_STANDING";
		case CATCH_CROUCHED:                  return "CATCH_CROUCHED";
		case PLANT_BOMB:                      return "PLANT_BOMB";
		case USE_REMOTE:                      return "USE_REMOTE";
		case START_COWER:                     return "START_COWER";
		case COWERING:                        return "COWERING";
		case END_COWER:                       return "END_COWER";
		case STEAL_ITEM:                      return "STEAL_ITEM";
		case SHOOT_ROCKET:                    return "SHOOT_ROCKET";
		case CIV_DIE2:                        return "CIV_DIE2";
		case SHOOT_MORTAR:                    return "SHOOT_MORTAR";
		case CROW_DIE:                        return "CROW_DIE";
		case SIDE_STEP:                       return "SIDE_STEP";
		case WALK_BACKWARDS:                  return "WALK_BACKWARDS";
		case BEGIN_OPENSTRUCT:                return "BEGIN_OPENSTRUCT";
		case END_OPENSTRUCT:                  return "END_OPENSTRUCT";
		case END_OPENSTRUCT_LOCKED:           return "END_OPENSTRUCT_LOCKED";
		case PUNCH_LOW:                       return "PUNCH_LOW";
		case PISTOL_SHOOT_LOW:                return "PISTOL_SHOOT_LOW";
		case DECAPITATE:                      return "DECAPITATE";
		case BLOODCAT_BITE_ANIM:              return "BLOODCAT_BITE_ANIM";
		case BIGMERC_IDLE_NECK:               return "BIGMERC_IDLE_NECK";
		case BIGMERC_CROUCH_TRANS_INTO:       return "BIGMERC_CROUCH_TRANS_INTO";
		case BIGMERC_CROUCH_TRANS_OUTOF:      return "BIGMERC_CROUCH_TRANS_OUTOF";
		case GOTO_PATIENT:                    return "GOTO_PATIENT";
		case BEING_PATIENT:                   return "BEING_PATIENT";
		case GOTO_DOCTOR:                     return "GOTO_DOCTOR";
		case BEING_DOCTOR:                    return "BEING_DOCTOR";
		case END_DOCTOR:                      return "END_DOCTOR";
		case GOTO_REPAIRMAN:                  return "GOTO_REPAIRMAN";
		case BEING_REPAIRMAN:                 return "BEING_REPAIRMAN";
		case END_REPAIRMAN:                   return "END_REPAIRMAN";
		case FALL_INTO_PIT:                   return "FALL_INTO_PIT";
		case ROBOT_WALK:                      return "ROBOT_WALK";
		case ROBOT_SHOOT:                     return "ROBOT_SHOOT";
		case QUEEN_HIT:                       return "QUEEN_HIT";
		case QUEEN_DIE:                       return "QUEEN_DIE";
		case QUEEN_DIE_STOP:                  return "QUEEN_DIE_STOP";
		case QUEEN_INTO_READY:                return "QUEEN_INTO_READY";
		case QUEEN_READY:                     return "QUEEN_READY";
		case QUEEN_END_READY:                 return "QUEEN_END_READY";
		case QUEEN_CALL:                      return "QUEEN_CALL";
		case QUEEN_SPIT:                      return "QUEEN_SPIT";
		case QUEEN_SWIPE:                     return "QUEEN_SWIPE";
		case RELOAD_ROBOT:                    return "RELOAD_ROBOT";
		case END_CATCH:                       return "END_CATCH";
		case END_CROUCH_CATCH:                return "END_CROUCH_CATCH";
		case AI_RADIO:                        return "AI_RADIO";
		case AI_CR_RADIO:                     return "AI_CR_RADIO";
		case TANK_SHOOT:                      return "TANK_SHOOT";
		case TANK_BURST:                      return "TANK_BURST";
		case QUEEN_SLAP:                      return "QUEEN_SLAP";
		case SLAP_HIT:                        return "SLAP_HIT";
		case TAKE_BLOOD_FROM_CORPSE:          return "TAKE_BLOOD_FROM_CORPSE";
		case VEHICLE_DIE:                     return "VEHICLE_DIE";
		case QUEEN_FRUSTRATED_SLAP:           return "QUEEN_FRUSTRATED_SLAP";
		case CHARIOTS_OF_FIRE:                return "CHARIOTS_OF_FIRE";
		case AI_PULL_SWITCH:                  return "AI_PULL_SWITCH";
		case MONSTER_MELT:                    return "MONSTER_MELT";
		case MERC_HURT_IDLE_ANIM:             return "MERC_HURT_IDLE_ANIM";
		case END_HURT_WALKING:                return "END_HURT_WALKING";
		case PASS_OBJECT:                     return "PASS_OBJECT";
		case DROP_ADJACENT_OBJECT:            return "DROP_ADJACENT_OBJECT";
		case READY_DUAL_PRONE:                return "READY_DUAL_PRONE";
		case AIM_DUAL_PRONE:                  return "AIM_DUAL_PRONE";
		case SHOOT_DUAL_PRONE:                return "SHOOT_DUAL_PRONE";
		case END_DUAL_PRONE:                  return "END_DUAL_PRONE";
		case PRONE_SHOOT_DWEL_UNJAM:          return "PRONE_SHOOT_DWEL_UNJAM";
		case PICK_LOCK:                       return "PICK_LOCK";
		case OPEN_DOOR_CROUCHED:              return "OPEN_DOOR_CROUCHED";
		case BEGIN_OPENSTRUCT_CROUCHED:       return "BEGIN_OPENSTRUCT_CROUCHED";
		case CLOSE_DOOR_CROUCHED:             return "CLOSE_DOOR_CROUCHED";
		case OPEN_STRUCT_CROUCHED:            return "OPEN_STRUCT_CROUCHED";
		case END_OPEN_DOOR_CROUCHED:          return "END_OPEN_DOOR_CROUCHED";
		case END_OPENSTRUCT_CROUCHED:         return "END_OPENSTRUCT_CROUCHED";
		case END_OPEN_LOCKED_DOOR_CROUCHED:   return "END_OPEN_LOCKED_DOOR_CROUCHED";
		case END_OPENSTRUCT_LOCKED_CROUCHED:  return "END_OPENSTRUCT_LOCKED_CROUCHED";
		case DRUNK_IDLE:                      return "DRUNK_IDLE";
		case CROWBAR_ATTACK:                  return "CROWBAR_ATTACK";
		case CIV_COWER_HIT:                   return "CIV_COWER_HIT";
		case BLOODCAT_WALK_BACKWARDS:         return "BLOODCAT_WALK_BACKWARDS";
		case MONSTER_WALK_BACKWARDS:          return "MONSTER_WALK_BACKWARDS";
		case KID_SKIPPING:                    return "KID_SKIPPING";
		case ROBOT_BURST_SHOOT:               return "ROBOT_BURST_SHOOT";
		case ATTACH_CAN_TO_STRING:            return "ATTACH_CAN_TO_STRING";
		case SWAT_BACKWARDS:                  return "SWAT_BACKWARDS";
		case JUMP_OVER_BLOCKING_PERSON:       return "JUMP_OVER_BLOCKING_PERSON";
		case REFUEL_VEHICLE:                  return "REFUEL_VEHICLE";
		case ROBOT_CAMERA_NOT_MOVING:         return "ROBOT_CAMERA_NOT_MOVING";
		case CRIPPLE_OPEN_DOOR:               return "CRIPPLE_OPEN_DOOR";
		case CRIPPLE_CLOSE_DOOR:              return "CRIPPLE_CLOSE_DOOR";
		case CRIPPLE_END_OPEN_DOOR:           return "CRIPPLE_END_OPEN_DOOR";
		case CRIPPLE_END_OPEN_LOCKED_DOOR:    return "CRIPPLE_END_OPEN_LOCKED_DOOR";
		case LOCKPICK_CROUCHED:               return "LOCKPICK_CROUCHED";
		default:                              return "???";
	}
}

/** Get inventory slot name. */
const char* Internals::getInventorySlotName(enum InvSlotPos slot)
{
	switch (slot)
	{
		case HELMETPOS:							return "HELMET";
		case VESTPOS:							return "VEST";
		case LEGPOS:							return "LEG";
		case HEAD1POS:							return "HEAD1";
		case HEAD2POS:							return "HEAD2";
		case HANDPOS:							return "HAND";
		case SECONDHANDPOS:						return "SECONDHAND";
		case BIGPOCK1POS:						return "BIGPOCK1";
		case BIGPOCK2POS:						return "BIGPOCK2";
		case BIGPOCK3POS:						return "BIGPOCK3";
		case BIGPOCK4POS:						return "BIGPOCK4";
		case SMALLPOCK1POS:						return "SMALLPOCK1";
		case SMALLPOCK2POS:						return "SMALLPOCK2";
		case SMALLPOCK3POS:						return "SMALLPOCK3";
		case SMALLPOCK4POS:						return "SMALLPOCK4";
		case SMALLPOCK5POS:						return "SMALLPOCK5";
		case SMALLPOCK6POS:						return "SMALLPOCK6";
		case SMALLPOCK7POS:						return "SMALLPOCK7";
		case SMALLPOCK8POS:						return "SMALLPOCK8";
		default:								return "???";
	}
}

InvSlotPos Internals::getInventorySlotEnumFromString(const ST::string& s)
{
			if (s == "HELMET") return InvSlotPos::HELMETPOS;
			else if (s == "VEST") return InvSlotPos::VESTPOS;
			else if (s == "LEG") return InvSlotPos::LEGPOS;
			else if (s == "HEAD1") return InvSlotPos::HEAD1POS;
			else if (s == "HEAD2") return InvSlotPos::HEAD2POS;
			else if (s == "HAND") return InvSlotPos::HANDPOS;
			else if (s == "SECONDHAND") return InvSlotPos::SECONDHANDPOS;
			else if (s == "BIGPOCK1") return InvSlotPos::BIGPOCK1POS;
			else if (s == "BIGPOCK2") return InvSlotPos::BIGPOCK2POS;
			else if (s == "BIGPOCK3") return InvSlotPos::BIGPOCK3POS;
			else if (s == "BIGPOCK4") return InvSlotPos::BIGPOCK4POS;
			else if (s == "SMALLPOCK1") return InvSlotPos::SMALLPOCK1POS;
			else if (s == "SMALLPOCK2") return InvSlotPos::SMALLPOCK2POS;
			else if (s == "SMALLPOCK3") return InvSlotPos::SMALLPOCK3POS;
			else if (s == "SMALLPOCK4") return InvSlotPos::SMALLPOCK4POS;
			else if (s == "SMALLPOCK5") return InvSlotPos::SMALLPOCK5POS;
			else if (s == "SMALLPOCK6") return InvSlotPos::SMALLPOCK6POS;
			else if (s == "SMALLPOCK7") return InvSlotPos::SMALLPOCK7POS;
			else if (s == "SMALLPOCK8") return InvSlotPos::SMALLPOCK8POS;
			else throw DataError(ST::format("unknown inventory slot name value: {}", s));
}

/** Get civilian group name. */
const char* Internals::getCivilianGroupName(enum CivilianGroup group)
{
	switch (group)
	{
		case NON_CIV_GROUP:						return "NON_CIV";
		case REBEL_CIV_GROUP:					return "REBEL";
		case KINGPIN_CIV_GROUP:					return "KINGPIN";
		case SANMONA_ARMS_GROUP:				return "SANMONA_ARMS";
		case ANGELS_GROUP:						return "ANGEL";
		case BEGGARS_CIV_GROUP:					return "BEGGARS";
		case TOURISTS_CIV_GROUP:				return "TOURISTS";
		case ALMA_MILITARY_CIV_GROUP:			return "ALMA_MILITARY";
		case DOCTORS_CIV_GROUP:					return "DOCTORS";
		case COUPLE1_CIV_GROUP:					return "COUPLE1";
		case HICKS_CIV_GROUP:					return "HICKS";
		case WARDEN_CIV_GROUP:					return "WARDEN";
		case JUNKYARD_CIV_GROUP:				return "JUNKYARD";
		case FACTORY_KIDS_GROUP:				return "FACTORY";
		case QUEENS_CIV_GROUP:					return "QUEEN";
		case UNNAMED_CIV_GROUP_15:				return "UNNAMED_15";
		case UNNAMED_CIV_GROUP_16:				return "UNNAMED_16";
		case UNNAMED_CIV_GROUP_17:				return "UNNAMED_17";
		case UNNAMED_CIV_GROUP_18:				return "UNNAMED_18";
		case UNNAMED_CIV_GROUP_19:				return "UNNAMED_19";
		default:								return "???";
	}
}

CivilianGroup Internals::getCivilianGroupEnumFromString(const ST::string& s)
{
	if (s.empty() || s == "NON_CIV") return CivilianGroup::NON_CIV_GROUP;
	else if (s == "REBEL") return CivilianGroup::REBEL_CIV_GROUP;
	else if (s == "KINGPIN") return CivilianGroup::KINGPIN_CIV_GROUP;
	else if (s == "SANMONA_ARMS") return CivilianGroup::SANMONA_ARMS_GROUP;
	else if (s == "ANGEL") return CivilianGroup::ANGELS_GROUP;
	else if (s == "BEGGARS") return CivilianGroup::BEGGARS_CIV_GROUP;
	else if (s == "TOURISTS") return CivilianGroup::TOURISTS_CIV_GROUP;
	else if (s == "ALMA_MILITARY") return CivilianGroup::ALMA_MILITARY_CIV_GROUP;
	else if (s == "DOCTORS") return CivilianGroup::DOCTORS_CIV_GROUP;
	else if (s == "COUPLE1") return CivilianGroup::COUPLE1_CIV_GROUP;
	else if (s == "HICKS") return CivilianGroup::HICKS_CIV_GROUP;
	else if (s == "WARDEN") return CivilianGroup::WARDEN_CIV_GROUP;
	else if (s == "JUNKYARD") return CivilianGroup::JUNKYARD_CIV_GROUP;
	else if (s == "FACTORY") return CivilianGroup::FACTORY_KIDS_GROUP;
	else if (s == "QUEEN") return CivilianGroup::QUEENS_CIV_GROUP;
	else if (s == "UNNAMED_15") return CivilianGroup::UNNAMED_CIV_GROUP_15;
	else if (s == "UNNAMED_16") return CivilianGroup::UNNAMED_CIV_GROUP_16;
	else if (s == "UNNAMED_17") return CivilianGroup::UNNAMED_CIV_GROUP_17;
	else if (s == "UNNAMED_18") return CivilianGroup::UNNAMED_CIV_GROUP_18;
	else if (s == "UNNAMED_19") return CivilianGroup::UNNAMED_CIV_GROUP_19;
	else throw DataError(ST::format("unknown civilian group value: {}", s));
}

/** Get body type name. */
const char* Internals::getBodyTypeName(enum SoldierBodyType type)
{
	switch (type)
	{
		case BODY_RANDOM:						return "BODY_RANDOM";
		case REGMALE:							return "REGMALE";
		case BIGMALE:							return "BIGMALE";
		case STOCKYMALE:						return "STOCKYMALE";
		case REGFEMALE:							return "REGFEMALE";
		case ADULTFEMALEMONSTER:				return "ADULTFEMALEMONSTER";
		case AM_MONSTER:						return "AM_MONSTER";
		case YAF_MONSTER:						return "YAF_MONSTER";
		case YAM_MONSTER:						return "YAM_MONSTER";
		case LARVAE_MONSTER:					return "LARVAE_MONSTER";
		case INFANT_MONSTER:					return "INFANT_MONSTER";
		case QUEENMONSTER:						return "QUEENMONSTER";
		case FATCIV:							return "FATCIV";
		case MANCIV:							return "MANCIV";
		case MINICIV:							return "MINICIV";
		case DRESSCIV:							return "DRESSCIV";
		case HATKIDCIV:							return "HATKIDCIV";
		case KIDCIV:							return "KIDCIV";
		case CRIPPLECIV:						return "CRIPPLECIV";
		case COW:								return "COW";
		case CROW:								return "CROW";
		case BLOODCAT:							return "BLOODCAT";
		case ROBOTNOWEAPON:						return "ROBOTNOWEAPON";
		case HUMVEE:							return "HUMVEE";
		case TANK_NW:							return "TANK_NW";
		case TANK_NE:							return "TANK_NE";
		case ELDORADO:							return "ELDORADO";
		case ICECREAMTRUCK:						return "ICECREAMTRUCK";
		case JEEP:								return "JEEP";
		default:								return "???";
	}
}

SoldierBodyType Internals::getBodyTypeEnumFromString(const ST::string& s)
{
	if (s == "BODY_RANDOM") return SoldierBodyType::BODY_RANDOM;
	else if (s == "" || s == "REGMALE") return SoldierBodyType::REGMALE;
	else if (s == "BIGMALE") return SoldierBodyType::BIGMALE;
	else if (s == "STOCKYMALE") return SoldierBodyType::STOCKYMALE;
	else if (s == "REGFEMALE") return SoldierBodyType::REGFEMALE;
	else if (s == "ADULTFEMALEMONSTER") return SoldierBodyType::ADULTFEMALEMONSTER;
	else if (s == "AM_MONSTER") return SoldierBodyType::AM_MONSTER;
	else if (s == "YAF_MONSTER") return SoldierBodyType::YAF_MONSTER;
	else if (s == "YAM_MONSTER") return SoldierBodyType::YAM_MONSTER;
	else if (s == "LARVAE_MONSTER") return SoldierBodyType::LARVAE_MONSTER;
	else if (s == "INFANT_MONSTER") return SoldierBodyType::INFANT_MONSTER;
	else if (s == "QUEENMONSTER") return SoldierBodyType::QUEENMONSTER;
	else if (s == "FATCIV") return SoldierBodyType::FATCIV;
	else if (s == "MANCIV") return SoldierBodyType::MANCIV;
	else if (s == "MINICIV") return SoldierBodyType::MINICIV;
	else if (s == "DRESSCIV") return SoldierBodyType::DRESSCIV;
	else if (s == "HATKIDCIV") return SoldierBodyType::HATKIDCIV;
	else if (s == "KIDCIV") return SoldierBodyType::KIDCIV;
	else if (s == "CRIPPLECIV") return SoldierBodyType::CRIPPLECIV;
	else if (s == "COW") return SoldierBodyType::COW;
	else if (s == "CROW") return SoldierBodyType::CROW;
	else if (s == "BLOODCAT") return SoldierBodyType::BLOODCAT;
	else if (s == "ROBOTNOWEAPON") return SoldierBodyType::ROBOTNOWEAPON;
	else if (s == "HUMVEE") return SoldierBodyType::HUMVEE;
	else if (s == "TANK_NW") return SoldierBodyType::TANK_NW;
	else if (s == "TANK_NE") return SoldierBodyType::TANK_NE;
	else if (s == "ELDORADO") return SoldierBodyType::ELDORADO;
	else if (s == "ICECREAMTRUCK") return SoldierBodyType::ICECREAMTRUCK;
	else if (s == "JEEP") return SoldierBodyType::JEEP;
	else throw DataError(ST::format("unknown body type value: {}", s));
}

/** Get attitude name. */
const char* Internals::getAttitudeName(enum Attitudes attitude)
{
	switch (attitude)
	{
		case ATT_NORMAL:						return "NORMAL";
		case ATT_FRIENDLY:						return "FRIENDLY";
		case ATT_LONER:							return "LONER";
		case ATT_OPTIMIST:						return "OPTIMIST";
		case ATT_PESSIMIST:						return "PESSIMIST";
		case ATT_AGGRESSIVE:					return "AGGRESSIVE";
		case ATT_ARROGANT:						return "ARROGANT";
		case ATT_BIG_SHOT:						return "BIG_SHOT";
		case ATT_ASSHOLE:						return "ASSHOLE";
		case ATT_COWARD:						return "COWARD";
		default:								return "???";
	}
}

Attitudes Internals::getAttitudeEnumFromString(const ST::string& s)
{
	if (s == "" || s == "NORMAL") return Attitudes::ATT_NORMAL;
	else if (s == "FRIENDLY") return Attitudes::ATT_FRIENDLY;
	else if (s == "LONER") return Attitudes::ATT_LONER;
	else if (s == "OPTIMIST") return Attitudes::ATT_OPTIMIST;
	else if (s == "PESSIMIST") return Attitudes::ATT_PESSIMIST;
	else if (s == "AGGRESSIVE") return Attitudes::ATT_AGGRESSIVE;
	else if (s == "ARROGANT") return Attitudes::ATT_ARROGANT;
	else if (s == "BIG_SHOT") return Attitudes::ATT_BIG_SHOT;
	else if (s == "ASSHOLE") return Attitudes::ATT_ASSHOLE;
	else if (s == "COWARD") return Attitudes::ATT_COWARD;
	else throw DataError(ST::format("unknown attitude value: {}", s));
}

/** Get personality trait name. */
const char* Internals::getPersonalityTraitName(enum PersonalityTrait trait)
{
	switch (trait)
	{
		case NO_PERSONALITYTRAIT:				return "NONE";
		case HEAT_INTOLERANT:					return "HEAT_INTOLERANT";
		case NERVOUS:							return "NERVOUS";
		case CLAUSTROPHOBIC:					return "CLAUSTROPHOBIC";
		case NONSWIMMER:						return "NONSWIMMER";
		case FEAR_OF_INSECTS:					return "FEAR_OF_INSECTS";
		case FORGETFUL:							return "FORGETFUL";
		case PSYCHO:							return "PSYCHO";
		default:								return "???";
	}
}

PersonalityTrait Internals::getPersonalityTraitEnumFromString(const ST::string& s)
{
	if (s == "" || s == "NONE") return PersonalityTrait::NO_PERSONALITYTRAIT;
	else if (s == "HEAT_INTOLERANT") return PersonalityTrait::HEAT_INTOLERANT;
	else if (s == "NERVOUS") return PersonalityTrait::NERVOUS;
	else if (s == "CLAUSTROPHOBIC") return PersonalityTrait::CLAUSTROPHOBIC;
	else if (s == "NONSWIMMER") return PersonalityTrait::NONSWIMMER;
	else if (s == "FEAR_OF_INSECTS") return PersonalityTrait::FEAR_OF_INSECTS;
	else if (s == "FORGETFUL") return PersonalityTrait::FORGETFUL;
	else if (s == "PSYCHO") return PersonalityTrait::PSYCHO;
	else throw DataError(ST::format("unknown personality trait value: {}", s));
}

/** Get skill trait name. */
const char* Internals::getSkillTraitName(enum SkillTrait trait)
{
	switch (trait)
	{
		case NO_SKILLTRAIT:						return "NONE";
		case LOCKPICKING:						return "LOCKPICKING";
		case HANDTOHAND:						return "HANDTOHAND";
		case ELECTRONICS:						return "ELECTRONICS";
		case NIGHTOPS:							return "NIGHTOPS";
		case THROWING:							return "THROWING";
		case TEACHING:							return "TEACHING";
		case HEAVY_WEAPS:						return "HEAVY_WEAPS";
		case AUTO_WEAPS:						return "AUTO_WEAPS";
		case STEALTHY:							return "STEALTHY";
		case AMBIDEXT:							return "AMBIDEXT";
		case THIEF:								return "THIEF";
		case MARTIALARTS:						return "MARTIALARTS";
		case KNIFING:							return "KNIFING";
		case ONROOF:							return "ONROOF";
		case CAMOUFLAGED:						return "CAMOUFLAGED";
		default:								return "???";
	}
}

SkillTrait Internals::getSkillTraitEnumFromString(const ST::string& s)
{
	if (s == "" || s == "NONE") return SkillTrait::NO_SKILLTRAIT;
	else if (s == "LOCKPICKING") return SkillTrait::LOCKPICKING;
	else if (s == "HANDTOHAND") return SkillTrait::HANDTOHAND;
	else if (s == "ELECTRONICS") return SkillTrait::ELECTRONICS;
	else if (s == "NIGHTOPS") return SkillTrait::NIGHTOPS;
	else if (s == "THROWING") return SkillTrait::THROWING;
	else if (s == "TEACHING") return SkillTrait::TEACHING;
	else if (s == "HEAVY_WEAPS") return SkillTrait::HEAVY_WEAPS;
	else if (s == "AUTO_WEAPS") return SkillTrait::AUTO_WEAPS;
	else if (s == "STEALTHY") return SkillTrait::STEALTHY;
	else if (s == "AMBIDEXT") return SkillTrait::AMBIDEXT;
	else if (s == "THIEF") return SkillTrait::THIEF;
	else if (s == "MARTIALARTS") return SkillTrait::MARTIALARTS;
	else if (s == "KNIFING") return SkillTrait::KNIFING;
	else if (s == "ONROOF") return SkillTrait::ONROOF;
	else if (s == "CAMOUFLAGED") return SkillTrait::CAMOUFLAGED;
	else throw DataError(ST::format("unknown skill trait value: {}", s));
}

const char* Internals::getMercTypeName(enum MercType mercType)
{
	switch (mercType)
	{
		case MercType::NOT_USED:		return "NOT_USED";
		case MercType::AIM:				return "AIM";
		case MercType::MERC:			return "MERC";
		case MercType::IMP:				return "IMP";
		case MercType::RPC:				return "RPC";
		case MercType::NPC:				return "NPC";
		case MercType::VEHICLE:			return "VEHICLE";
		default:						return "???";
	}
}

MercType Internals::getMercTypeEnumFromString(const ST::string& s)
{
	if (s == "" || s == "NOT_USED") return MercType::NOT_USED;
	else if (s == "AIM")			return MercType::AIM;
	else if (s == "MERC")			return MercType::MERC;
	else if (s == "IMP")			return MercType::IMP;
	else if (s == "RPC")			return MercType::RPC;
	else if (s == "NPC")			return MercType::NPC;
	else if (s == "VEHICLE")		return MercType::VEHICLE;
	else throw DataError(ST::format("unknown merc type value: {}", s));
}

Quests Internals::getQuestEnumFromString(const ST::string& s)
{
	if (s.empty() || s == "NO_QUEST") return Quests::NO_QUEST;
	else if (s == "DELIVER_LETTER") return Quests::QUEST_DELIVER_LETTER;
	else if (s == "FOOD_ROUTE") return Quests::QUEST_FOOD_ROUTE;
	else if (s == "KILL_TERRORISTS") return Quests::QUEST_KILL_TERRORISTS;
	else if (s == "KINGPIN_IDOL") return Quests::QUEST_KINGPIN_IDOL;
	else if (s == "KINGPIN_MONEY") return Quests::QUEST_KINGPIN_MONEY;
	else if (s == "RUNAWAY_JOEY") return Quests::QUEST_RUNAWAY_JOEY;
	else if (s == "RESCUE_MARIA") return Quests::QUEST_RESCUE_MARIA;
	else if (s == "CHITZENA_IDOL") return Quests::QUEST_CHITZENA_IDOL;
	else if (s == "HELD_IN_ALMA") return Quests::QUEST_HELD_IN_ALMA;
	else if (s == "INTERROGATION") return Quests::QUEST_INTERROGATION;
	else if (s == "ARMY_FARM") return Quests::QUEST_ARMY_FARM;
	else if (s == "FIND_SCIENTIST") return Quests::QUEST_FIND_SCIENTIST;
	else if (s == "DELIVER_VIDEO_CAMERA") return Quests::QUEST_DELIVER_VIDEO_CAMERA;
	else if (s == "BLOODCATS") return Quests::QUEST_BLOODCATS;
	else if (s == "FIND_HERMIT") return Quests::QUEST_FIND_HERMIT;
	else if (s == "CREATURES") return Quests::QUEST_CREATURES;
	else if (s == "CHOPPER_PILOT") return Quests::QUEST_CHOPPER_PILOT;
	else if (s == "ESCORT_SKYRIDER") return Quests::QUEST_ESCORT_SKYRIDER;
	else if (s == "FREE_DYNAMO") return Quests::QUEST_FREE_DYNAMO;
	else if (s == "ESCORT_TOURISTS") return Quests::QUEST_ESCORT_TOURISTS;
	else if (s == "FREE_CHILDREN") return Quests::QUEST_FREE_CHILDREN;
	else if (s == "LEATHER_SHOP_DREAM") return Quests::QUEST_LEATHER_SHOP_DREAM;
	else if (s == "FREE_SHANK") return Quests::QUEST_FREE_SHANK;
	else if (s == "KILL_DEIDRANNA") return Quests::QUEST_KILL_DEIDRANNA;
	else throw DataError(ST::format("unknown quest name value: {}", s));
}

Fact Internals::getFactEnumFromString(const ST::string& s)
{
	if (s.empty() || s == "NONE") return Fact::FACT_NONE;
	else if (s == "OMERTA_LIBERATED") return Fact::FACT_OMERTA_LIBERATED;
	else if (s == "DRASSEN_LIBERATED") return Fact::FACT_DRASSEN_LIBERATED;
	else if (s == "SANMONA_LIBERATED") return Fact::FACT_SANMONA_LIBERATED;
	else if (s == "CAMBRIA_LIBERATED") return Fact::FACT_CAMBRIA_LIBERATED;
	else if (s == "ALMA_LIBERATED") return Fact::FACT_ALMA_LIBERATED;
	else if (s == "GRUMM_LIBERATED") return Fact::FACT_GRUMM_LIBERATED;
	else if (s == "TIXA_LIBERATED") return Fact::FACT_TIXA_LIBERATED;
	else if (s == "CHITZENA_LIBERATED") return Fact::FACT_CHITZENA_LIBERATED;
	else if (s == "ESTONI_LIBERATED") return Fact::FACT_ESTONI_LIBERATED;
	else if (s == "BALIME_LIBERATED") return Fact::FACT_BALIME_LIBERATED;
	else if (s == "ORTA_LIBERATED") return Fact::FACT_ORTA_LIBERATED;
	else if (s == "MEDUNA_LIBERATED") return Fact::FACT_MEDUNA_LIBERATED;
	else if (s == "MIGUEL_FOUND") return Fact::FACT_MIGUEL_FOUND;
	else if (s == "LETTER_DELIVERED") return Fact::FACT_LETTER_DELIVERED;
	else if (s == "FOOD_ROUTE_EXISTS") return Fact::FACT_FOOD_ROUTE_EXISTS;
	else if (s == "DIMITRI_DEAD") return Fact::FACT_DIMITRI_DEAD;
	else if (s == "MIGUEL_READ_LETTER") return Fact::FACT_MIGUEL_READ_LETTER;
	else if (s == "REBELS_HATE_PLAYER") return Fact::FACT_REBELS_HATE_PLAYER;
	else if (s == "PACOS_KILLED") return Fact::FACT_PACOS_KILLED;
	else if (s == "CURRENT_SECTOR_IS_SAFE") return Fact::FACT_CURRENT_SECTOR_IS_SAFE;
	else if (s == "BOBBYRAY_SHIPMENT_IN_TRANSIT") return Fact::FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT;
	else if (s == "NEW_BOBBYRAY_SHIPMENT_WAITING") return Fact::FACT_NEW_BOBBYRAY_SHIPMENT_WAITING;
	else if (s == "REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING") return Fact::FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING;
	else if (s == "LARGE_SIZED_OLD_SHIPMENT_WAITING") return Fact::FACT_LARGE_SIZED_OLD_SHIPMENT_WAITING;
	else if (s == "PLAYER_FOUND_ITEMS_MISSING") return Fact::FACT_PLAYER_FOUND_ITEMS_MISSING;
	else if (s == "PABLO_PUNISHED_BY_PLAYER") return Fact::FACT_PABLO_PUNISHED_BY_PLAYER;
	else if (s == "PABLO_ATTACKED_AFTER_PUNISHMENT") return Fact::FACT_PABLO_ATTACKED_AFTER_PUNISHMENT;
	else if (s == "PABLO_RETURNED_GOODS") return Fact::FACT_PABLO_RETURNED_GOODS;
	else if (s == "PABLOS_BRIBED") return Fact::FACT_PABLOS_BRIBED;
	else if (s == "SKYRIDER_CLOSE_TO_CHOPPER") return Fact::FACT_SKYRIDER_CLOSE_TO_CHOPPER;
	else if (s == "SKYRIDER_READY_FOR_TRANSPORT") return Fact::FACT_SKYRIDER_READY_FOR_TRANSPORT;
	else if (s == "SKYRIDER_USED_IN_MAPSCREEN") return Fact::FACT_SKYRIDER_USED_IN_MAPSCREEN;
	else if (s == "NPC_OWED_MONEY") return Fact::FACT_NPC_OWED_MONEY;
	else if (s == "NPC_WOUNDED") return Fact::FACT_NPC_WOUNDED;
	else if (s == "NPC_WOUNDED_BY_PLAYER") return Fact::FACT_NPC_WOUNDED_BY_PLAYER;
	else if (s == "IRA_NOT_PRESENT") return Fact::FACT_IRA_NOT_PRESENT;
	else if (s == "IRA_TALKING") return Fact::FACT_IRA_TALKING;
	else if (s == "FOOD_QUEST_OVER") return Fact::FACT_FOOD_QUEST_OVER;
	else if (s == "PABLOS_STOLE_FROM_LATEST_SHIPMENT") return Fact::FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT;
	else if (s == "LAST_SHIPMENT_CRASHED") return Fact::FACT_LAST_SHIPMENT_CRASHED;
	else if (s == "LAST_SHIPMENT_WENT_TO_WRONG_AIRPORT") return Fact::FACT_LAST_SHIPMENT_WENT_TO_WRONG_AIRPORT;
	else if (s == "SHIPMENT_DELAYED_24_HOURS") return Fact::FACT_SHIPMENT_DELAYED_24_HOURS;
	else if (s == "PACKAGE_DAMAGED") return Fact::FACT_PACKAGE_DAMAGED;
	else if (s == "PACKAGE_LOST_PERMANENTLY") return Fact::FACT_PACKAGE_LOST_PERMANENTLY;
	else if (s == "NEXT_PACKAGE_CAN_BE_LOST") return Fact::FACT_NEXT_PACKAGE_CAN_BE_LOST;
	else if (s == "NEXT_PACKAGE_CAN_BE_DELAYED") return Fact::FACT_NEXT_PACKAGE_CAN_BE_DELAYED;
	else if (s == "MEDIUM_SIZED_SHIPMENT_WAITING") return Fact::FACT_MEDIUM_SIZED_SHIPMENT_WAITING;
	else if (s == "LARGE_SIZED_SHIPMENT_WAITING") return Fact::FACT_LARGE_SIZED_SHIPMENT_WAITING;
	else if (s == "DOREEN_HAD_CHANGE_OF_HEART") return Fact::FACT_DOREEN_HAD_CHANGE_OF_HEART;
	else if (s == "IRA_UNHIRED_AND_ALIVE") return Fact::FACT_IRA_UNHIRED_AND_ALIVE;
	else if (s == "NPC_BLEEDING") return Fact::FACT_NPC_BLEEDING;
	else if (s == "NPC_BLEEDING_BUT_OKAY") return Fact::FACT_NPC_BLEEDING_BUT_OKAY;
	else if (s == "PLAYER_HAS_HEAD_AND_CARMEN_IN_SAN_MONA") return Fact::FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_SAN_MONA;
	else if (s == "PLAYER_HAS_HEAD_AND_CARMEN_IN_CAMBRIA") return Fact::FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_CAMBRIA;
	else if (s == "PLAYER_HAS_HEAD_AND_CARMEN_IN_DRASSEN") return Fact::FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_DRASSEN;
	else if (s == "FATHER_DRUNK") return Fact::FACT_FATHER_DRUNK;
	else if (s == "WOUNDED_MERCS_NEARBY") return Fact::FACT_WOUNDED_MERCS_NEARBY;
	else if (s == "ONE_WOUNDED_MERC_NEARBY") return Fact::FACT_ONE_WOUNDED_MERC_NEARBY;
	else if (s == "MULTIPLE_WOUNDED_MERCS_NEARBY") return Fact::FACT_MULTIPLE_WOUNDED_MERCS_NEARBY;
	else if (s == "BRENDA_IN_STORE_AND_ALIVE") return Fact::FACT_BRENDA_IN_STORE_AND_ALIVE;
	else if (s == "BRENDA_DEAD") return Fact::FACT_BRENDA_DEAD;
	else if (s == "NPC_IS_ENEMY") return Fact::FACT_NPC_IS_ENEMY;
	else if (s == "PC_STRONG_AND_LESS_THAN_3_MALES_PRESENT") return Fact::FACT_PC_STRONG_AND_LESS_THAN_3_MALES_PRESENT;
	else if (s == "PC_STRONG_AND_3_PLUS_MALES_PRESENT") return Fact::FACT_PC_STRONG_AND_3_PLUS_MALES_PRESENT;
	else if (s == "HANS_TRIGGER_CONDITION") return Fact::FACT_HANS_TRIGGER_CONDITION;
	else if (s == "HANS_AT_SPOT") return Fact::FACT_HANS_AT_SPOT;
	else if (s == "TONY_NOT_AVAILABLE") return Fact::FACT_TONY_NOT_AVAILABLE;
	else if (s == "FEMALE_SPEAKING_TO_NPC") return Fact::FACT_FEMALE_SPEAKING_TO_NPC;
	else if (s == "PLAYER_USED_BROTHEL") return Fact::FACT_PLAYER_USED_BROTHEL;
	else if (s == "CARLA_AVAILABLE") return Fact::FACT_CARLA_AVAILABLE;
	else if (s == "CINDY_AVAILABLE") return Fact::FACT_CINDY_AVAILABLE;
	else if (s == "BAMBI_AVAILABLE") return Fact::FACT_BAMBI_AVAILABLE;
	else if (s == "NO_GIRLS_AVAILABLE") return Fact::FACT_NO_GIRLS_AVAILABLE;
	else if (s == "PLAYER_WAITED_FOR_GIRL") return Fact::FACT_PLAYER_WAITED_FOR_GIRL;
	else if (s == "PLAYER_PAID_RIGHT_AMOUNT") return Fact::FACT_PLAYER_PAID_RIGHT_AMOUNT;
	else if (s == "PLAYER_PASSED_GOON") return Fact::FACT_PLAYER_PASSED_GOON;
	else if (s == "MULTIPLE_MERCS_CLOSE") return Fact::FACT_MULTIPLE_MERCS_CLOSE;
	else if (s == "SOME_MERCS_CLOSE") return Fact::FACT_SOME_MERCS_CLOSE;
	else if (s == "DARREN_EXPECTING_MONEY") return Fact::FACT_DARREN_EXPECTING_MONEY;
	else if (s == "PC_NEAR") return Fact::FACT_PC_NEAR;
	else if (s == "CARMEN_IN_C5") return Fact::FACT_CARMEN_IN_C5;
	else if (s == "CARMEN_EXPLAINED_DEAL") return Fact::FACT_CARMEN_EXPLAINED_DEAL;
	else if (s == "KINGPIN_KNOWS_MONEY_GONE") return Fact::FACT_KINGPIN_KNOWS_MONEY_GONE;
	else if (s == "PLAYER_REPAID_KINGPIN") return Fact::FACT_PLAYER_REPAID_KINGPIN;
	else if (s == "FRANK_HAS_BEEN_BRIBED") return Fact::FACT_FRANK_HAS_BEEN_BRIBED;
	else if (s == "PAST_CLUB_CLOSING_AND_PLAYER_WARNED") return Fact::FACT_PAST_CLUB_CLOSING_AND_PLAYER_WARNED;
	else if (s == "NPC_ESCORTED") return Fact::FACT_NPC_ESCORTED;
	else if (s == "JOEY_IN_C5") return Fact::FACT_JOEY_IN_C5;
	else if (s == "JOEY_NEAR_MARTHA") return Fact::FACT_JOEY_NEAR_MARTHA;
	else if (s == "JOEY_DEAD") return Fact::FACT_JOEY_DEAD;
	else if (s == "MERC_NEAR_NPC") return Fact::FACT_MERC_NEAR_NPC;
	else if (s == "SPIKE_AT_DOOR") return Fact::FACT_SPIKE_AT_DOOR;
	else if (s == "ANGEL_SOLD_VEST") return Fact::FACT_ANGEL_SOLD_VEST;
	else if (s == "MARIA_ESCORTED_AT_LEATHER_SHOP") return Fact::FACT_MARIA_ESCORTED_AT_LEATHER_SHOP;
	else if (s == "PLAYER_WANTS_TO_BUY_LEATHER_VEST") return Fact::FACT_PLAYER_WANTS_TO_BUY_LEATHER_VEST;
	else if (s == "MARIA_ESCAPE_NOTICED") return Fact::FACT_MARIA_ESCAPE_NOTICED;
	else if (s == "ANGEL_LEFT_DEED") return Fact::FACT_ANGEL_LEFT_DEED;
	else if (s == "MARIA_IMPATIENT_TO_LEAVE_SAN_MONA") return Fact::FACT_MARIA_IMPATIENT_TO_LEAVE_SAN_MONA;
	else if (s == "NPC_BANDAGED_TODAY") return Fact::FACT_NPC_BANDAGED_TODAY;
	else if (s == "PABLO_WONT_STEAL") return Fact::FACT_PABLO_WONT_STEAL;
	else if (s == "AGENTS_PREVENTED_SHIPMENT") return Fact::FACT_AGENTS_PREVENTED_SHIPMENT;
	else if (s == "LARGE_AMOUNT_OF_MONEY") return Fact::FACT_LARGE_AMOUNT_OF_MONEY;
	else if (s == "SMALL_AMOUNT_OF_MONEY") return Fact::FACT_SMALL_AMOUNT_OF_MONEY;
	else if (s == "FATHER_DIALOGUE_STATE_1") return Fact::FACT_FATHER_DIALOGUE_STATE_1;
	else if (s == "FATHER_DIALOGUE_STATE_2") return Fact::FACT_FATHER_DIALOGUE_STATE_2;
	else if (s == "FATHER_DIALOGUE_STATE_3") return Fact::FACT_FATHER_DIALOGUE_STATE_3;
	else if (s == "LOYALTY_OKAY") return Fact::FACT_LOYALTY_OKAY;
	else if (s == "LOYALTY_LOW") return Fact::FACT_LOYALTY_LOW;
	else if (s == "LOYALTY_HIGH") return Fact::FACT_LOYALTY_HIGH;
	else if (s == "PLAYER_DOING_POORLY") return Fact::FACT_PLAYER_DOING_POORLY;
	else if (s == "CURRENT_SECTOR_G9") return Fact::FACT_CURRENT_SECTOR_G9;
	else if (s == "CURRENT_SECTOR_C5") return Fact::FACT_CURRENT_SECTOR_C5;
	else if (s == "CURRENT_SECTOR_C13") return Fact::FACT_CURRENT_SECTOR_C13;
	else if (s == "CARMEN_HAS_TEN_THOUSAND") return Fact::FACT_CARMEN_HAS_TEN_THOUSAND;
	else if (s == "SLAY_HIRED_AND_WORKED_FOR_48_HOURS") return Fact::FACT_SLAY_HIRED_AND_WORKED_FOR_48_HOURS;
	else if (s == "SLAY_IN_SECTOR") return Fact::FACT_SLAY_IN_SECTOR;
	else if (s == "VINCE_EXPLAINED_HAS_TO_CHARGE") return Fact::FACT_VINCE_EXPLAINED_HAS_TO_CHARGE;
	else if (s == "VINCE_EXPECTING_MONEY") return Fact::FACT_VINCE_EXPECTING_MONEY;
	else if (s == "PLAYER_STOLE_MEDICAL_SUPPLIES") return Fact::FACT_PLAYER_STOLE_MEDICAL_SUPPLIES;
	else if (s == "PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN") return Fact::FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN;
	else if (s == "VINCE_RECRUITABLE") return Fact::FACT_VINCE_RECRUITABLE;
	else if (s == "SLAY_CAN_BE_ATTACKED") return Fact::FACT_SLAY_CAN_BE_ATTACKED;
	else if (s == "ALL_TERRORISTS_KILLED") return Fact::FACT_ALL_TERRORISTS_KILLED;
	else if (s == "ELGIN_ALIVE") return Fact::FACT_ELGIN_ALIVE;
	else if (s == "MARIA_ABANDONED_BY_PLAYER") return Fact::FACT_MARIA_ABANDONED_BY_PLAYER;
	else if (s == "SKYRIDER_ABANDONED_BY_PLAYER") return Fact::FACT_SKYRIDER_ABANDONED_BY_PLAYER;
	else if (s == "JOEY_ABANDONED_BY_PLAYER") return Fact::FACT_JOEY_ABANDONED_BY_PLAYER;
	else if (s == "JOHN_ABANDONED_BY_PLAYER") return Fact::FACT_JOHN_ABANDONED_BY_PLAYER;
	else if (s == "MARY_ABANDONED_BY_PLAYER") return Fact::FACT_MARY_ABANDONED_BY_PLAYER;
	else if (s == "SHANK_IN_SQUAD_BUT_NOT_SPEAKING") return Fact::FACT_SHANK_IN_SQUAD_BUT_NOT_SPEAKING;
	else if (s == "SHANK_NOT_IN_SECTOR") return Fact::FACT_SHANK_NOT_IN_SECTOR;
	else if (s == "BLOODCAT_QUEST_STARTED_TWO_DAYS_AGO") return Fact::FACT_BLOODCAT_QUEST_STARTED_TWO_DAYS_AGO;
	else if (s == "QUEEN_DEAD") return Fact::FACT_QUEEN_DEAD;
	else if (s == "SPEAKER_AIM_OR_AIM_NEARBY") return Fact::FACT_SPEAKER_AIM_OR_AIM_NEARBY;
	else if (s == "MINE_EMPTY") return Fact::FACT_MINE_EMPTY;
	else if (s == "MINE_RUNNING_OUT") return Fact::FACT_MINE_RUNNING_OUT;
	else if (s == "MINE_PRODUCING_BUT_LOYALTY_LOW") return Fact::FACT_MINE_PRODUCING_BUT_LOYALTY_LOW;
	else if (s == "CREATURES_IN_MINE") return Fact::FACT_CREATURES_IN_MINE;
	else if (s == "PLAYER_LOST_MINE") return Fact::FACT_PLAYER_LOST_MINE;
	else if (s == "MINE_AT_FULL_PRODUCTION") return Fact::FACT_MINE_AT_FULL_PRODUCTION;
	else if (s == "DYNAMO_SPEAKING_OR_NEARBY") return Fact::FACT_DYNAMO_SPEAKING_OR_NEARBY;
	else if (s == "CHALICE_STOLEN") return Fact::FACT_CHALICE_STOLEN;
	else if (s == "JOHN_AND_MARY_EPCS") return Fact::FACT_JOHN_AND_MARY_EPCS;
	else if (s == "MARY_ALIVE") return Fact::FACT_MARY_ALIVE;
	else if (s == "MARY_BLEEDING") return Fact::FACT_MARY_BLEEDING;
	else if (s == "JOHN_ALIVE") return Fact::FACT_JOHN_ALIVE;
	else if (s == "JOHN_BLEEDING") return Fact::FACT_JOHN_BLEEDING;
	else if (s == "MARY_OR_JOHN_ARRIVED") return Fact::FACT_MARY_OR_JOHN_ARRIVED;
	else if (s == "MARY_DEAD") return Fact::FACT_MARY_DEAD;
	else if (s == "MINERS_PLACED") return Fact::FACT_MINERS_PLACED;
	else if (s == "KROTT_GOT_ANSWER_NO") return Fact::FACT_KROTT_GOT_ANSWER_NO;
	else if (s == "MADLAB_EXPECTING_FIREARM") return Fact::FACT_MADLAB_EXPECTING_FIREARM;
	else if (s == "MADLAB_EXPECTING_VIDEO_CAMERA") return Fact::FACT_MADLAB_EXPECTING_VIDEO_CAMERA;
	else if (s == "ITEM_POOR_CONDITION") return Fact::FACT_ITEM_POOR_CONDITION;
	else if (s == "ROBOT_READY") return Fact::FACT_ROBOT_READY;
	else if (s == "FIRST_ROBOT_DESTROYED") return Fact::FACT_FIRST_ROBOT_DESTROYED;
	else if (s == "MADLAB_HAS_GOOD_CAMERA") return Fact::FACT_MADLAB_HAS_GOOD_CAMERA;
	else if (s == "ROBOT_READY_SECOND_TIME") return Fact::FACT_ROBOT_READY_SECOND_TIME;
	else if (s == "SECOND_ROBOT_DESTROYED") return Fact::FACT_SECOND_ROBOT_DESTROYED;
	else if (s == "DYNAMO_IN_J9") return Fact::FACT_DYNAMO_IN_J9;
	else if (s == "DYNAMO_ALIVE") return Fact::FACT_DYNAMO_ALIVE;
	else if (s == "ANOTHER_FIGHT_POSSIBLE") return Fact::FACT_ANOTHER_FIGHT_POSSIBLE;
	else if (s == "RECEIVING_INCOME_FROM_DCAC") return Fact::FACT_RECEIVING_INCOME_FROM_DCAC;
	else if (s == "PLAYER_BEEN_TO_K4") return Fact::FACT_PLAYER_BEEN_TO_K4;
	else if (s == "WARDEN_DEAD") return Fact::FACT_WARDEN_DEAD;
	else if (s == "FIRST_BARTENDER") return Fact::FACT_FIRST_BARTENDER;
	else if (s == "SECOND_BARTENDER") return Fact::FACT_SECOND_BARTENDER;
	else if (s == "THIRD_BARTENDER") return Fact::FACT_THIRD_BARTENDER;
	else if (s == "FOURTH_BARTENDER") return Fact::FACT_FOURTH_BARTENDER;
	else if (s == "MANNY_IS_BARTENDER") return Fact::FACT_MANNY_IS_BARTENDER;
	else if (s == "NOTHING_REPAIRED_YET") return Fact::FACT_NOTHING_REPAIRED_YET;
	else if (s == "HOWARD_TRANSACTION_DONE") return Fact::FACT_HOWARD_TRANSACTION_DONE;
	else if (s == "OK_USE_HUMMER") return Fact::FACT_OK_USE_HUMMER;
	else if (s == "DAVE_HAS_GAS") return Fact::FACT_DAVE_HAS_GAS;
	else if (s == "VEHICLE_PRESENT") return Fact::FACT_VEHICLE_PRESENT;
	else if (s == "FIRST_BATTLE_WON") return Fact::FACT_FIRST_BATTLE_WON;
	else if (s == "ROBOT_RECRUITED_AND_MOVED") return Fact::FACT_ROBOT_RECRUITED_AND_MOVED;
	else if (s == "NO_CLUB_FIGHTING_ALLOWED") return Fact::FACT_NO_CLUB_FIGHTING_ALLOWED;
	else if (s == "PLAYER_FOUGHT_THREE_TIMES_TODAY") return Fact::FACT_PLAYER_FOUGHT_THREE_TIMES_TODAY;
	else if (s == "PLAYER_SPOKE_TO_DRASSEN_MINER") return Fact::FACT_PLAYER_SPOKE_TO_DRASSEN_MINER;
	else if (s == "PLAYER_DOING_WELL") return Fact::FACT_PLAYER_DOING_WELL;
	else if (s == "PLAYER_DOING_VERY_WELL") return Fact::FACT_PLAYER_DOING_VERY_WELL;
	else if (s == "FATHER_DRUNK_AND_SCIFI_OPTION_ON") return Fact::FACT_FATHER_DRUNK_AND_SCIFI_OPTION_ON;
	else if (s == "MICKY_DRUNK") return Fact::FACT_MICKY_DRUNK;
	else if (s == "PLAYER_FORCED_WAY_INTO_BROTHEL") return Fact::FACT_PLAYER_FORCED_WAY_INTO_BROTHEL;
	else if (s == "PLAYER_PAID_FOR_TWO_IN_BROTHEL") return Fact::FACT_PLAYER_PAID_FOR_TWO_IN_BROTHEL;
	else if (s == "PLAYER_OWNS_2_TOWNS_INCLUDING_OMERTA") return Fact::FACT_PLAYER_OWNS_2_TOWNS_INCLUDING_OMERTA;
	else if (s == "PLAYER_OWNS_3_TOWNS_INCLUDING_OMERTA") return Fact::FACT_PLAYER_OWNS_3_TOWNS_INCLUDING_OMERTA;
	else if (s == "PLAYER_OWNS_4_TOWNS_INCLUDING_OMERTA") return Fact::FACT_PLAYER_OWNS_4_TOWNS_INCLUDING_OMERTA;
	else if (s == "DIMITRI_CAN_BE_RECRUITED") return Fact::FACT_DIMITRI_CAN_BE_RECRUITED;
	else if (s == "MALE_SPEAKING_FEMALE_PRESENT") return Fact::FACT_MALE_SPEAKING_FEMALE_PRESENT;
	else if (s == "HICKS_MARRIED_PLAYER_MERC") return Fact::FACT_HICKS_MARRIED_PLAYER_MERC;
	else if (s == "MUSEUM_OPEN") return Fact::FACT_MUSEUM_OPEN;
	else if (s == "BROTHEL_OPEN") return Fact::FACT_BROTHEL_OPEN;
	else if (s == "CLUB_OPEN") return Fact::FACT_CLUB_OPEN;
	else if (s == "FIRST_BATTLE_FOUGHT") return Fact::FACT_FIRST_BATTLE_FOUGHT;
	else if (s == "FIRST_BATTLE_BEING_FOUGHT") return Fact::FACT_FIRST_BATTLE_BEING_FOUGHT;
	else if (s == "KINGPIN_INTRODUCED_SELF") return Fact::FACT_KINGPIN_INTRODUCED_SELF;
	else if (s == "KINGPIN_NOT_IN_OFFICE") return Fact::FACT_KINGPIN_NOT_IN_OFFICE;
	else if (s == "DONT_OWE_KINGPIN_MONEY") return Fact::FACT_DONT_OWE_KINGPIN_MONEY;
	else if (s == "PC_MARRYING_DARYL_IS_FLO") return Fact::FACT_PC_MARRYING_DARYL_IS_FLO;
	else if (s == "I16_BLOODCATS_KILLED") return Fact::FACT_I16_BLOODCATS_KILLED;
	else if (s == "NPC_COWERING") return Fact::FACT_NPC_COWERING;
	else if (s == "TOP_AND_BOTTOM_LEVELS_CLEARED") return Fact::FACT_TOP_AND_BOTTOM_LEVELS_CLEARED;
	else if (s == "TOP_LEVEL_CLEARED") return Fact::FACT_TOP_LEVEL_CLEARED;
	else if (s == "BOTTOM_LEVEL_CLEARED") return Fact::FACT_BOTTOM_LEVEL_CLEARED;
	else if (s == "NEED_TO_SAY_SOMETHING") return Fact::FACT_NEED_TO_SAY_SOMETHING;
	else if (s == "ATTACHED_ITEM_BEFORE") return Fact::FACT_ATTACHED_ITEM_BEFORE;
	else if (s == "SKYRIDER_EVER_ESCORTED") return Fact::FACT_SKYRIDER_EVER_ESCORTED;
	else if (s == "NPC_NOT_UNDER_FIRE") return Fact::FACT_NPC_NOT_UNDER_FIRE;
	else if (s == "WILLIS_HEARD_ABOUT_JOEY_RESCUE") return Fact::FACT_WILLIS_HEARD_ABOUT_JOEY_RESCUE;
	else if (s == "WILLIS_GIVES_DISCOUNT") return Fact::FACT_WILLIS_GIVES_DISCOUNT;
	else if (s == "HILLBILLIES_KILLED") return Fact::FACT_HILLBILLIES_KILLED;
	else if (s == "KEITH_OUT_OF_BUSINESS") return Fact::FACT_KEITH_OUT_OF_BUSINESS;
	else if (s == "MIKE_AVAILABLE_TO_ARMY") return Fact::FACT_MIKE_AVAILABLE_TO_ARMY;
	else if (s == "KINGPIN_CAN_SEND_ASSASSINS") return Fact::FACT_KINGPIN_CAN_SEND_ASSASSINS;
	else if (s == "ESTONI_REFUELLING_POSSIBLE") return Fact::FACT_ESTONI_REFUELLING_POSSIBLE;
	else if (s == "MUSEUM_ALARM_WENT_OFF") return Fact::FACT_MUSEUM_ALARM_WENT_OFF;
	else if (s == "MADDOG_IS_SPEAKER") return Fact::FACT_MADDOG_IS_SPEAKER;
	else if (s == "DARREN_EXPLAINED_BOXING_RULES") return Fact::FACT_DARREN_EXPLAINED_BOXING_RULES;
	else if (s == "ANGEL_MENTIONED_DEED") return Fact::FACT_ANGEL_MENTIONED_DEED;
	else if (s == "IGGY_AVAILABLE_TO_ARMY") return Fact::FACT_IGGY_AVAILABLE_TO_ARMY;
	else if (s == "PC_HAS_CONRADS_RECRUIT_OPINION") return Fact::FACT_PC_HAS_CONRADS_RECRUIT_OPINION;
	else if (s == "NPC_HOSTILE_OR_PISSED_OFF") return Fact::FACT_NPC_HOSTILE_OR_PISSED_OFF;
	else if (s == "TONY_IN_BUILDING") return Fact::FACT_TONY_IN_BUILDING;
	else if (s == "SHANK_SPEAKING") return Fact::FACT_SHANK_SPEAKING;
	else if (s == "PABLO_ALIVE") return Fact::FACT_PABLO_ALIVE;
	else if (s == "DOREEN_ALIVE") return Fact::FACT_DOREEN_ALIVE;
	else if (s == "WALDO_ALIVE") return Fact::FACT_WALDO_ALIVE;
	else if (s == "PERKO_ALIVE") return Fact::FACT_PERKO_ALIVE;
	else if (s == "TONY_ALIVE") return Fact::FACT_TONY_ALIVE;
	else if (s == "VINCE_ALIVE") return Fact::FACT_VINCE_ALIVE;
	else if (s == "JENNY_ALIVE") return Fact::FACT_JENNY_ALIVE;
	else if (s == "ARNOLD_ALIVE") return Fact::FACT_ARNOLD_ALIVE;
	else if (s == "ROCKET_RIFLE_EXISTS") return Fact::FACT_ROCKET_RIFLE_EXISTS;
	else if (s == "24_HOURS_SINCE_JOEY_RESCUED") return Fact::FACT_24_HOURS_SINCE_JOEY_RESCUED;
	else if (s == "24_HOURS_SINCE_DOCTOR_TALKED_TO") return Fact::FACT_24_HOURS_SINCE_DOCTOR_TALKED_TO;
	else if (s == "OK_USE_ICECREAM") return Fact::FACT_OK_USE_ICECREAM;
	else if (s == "KINGPIN_DEAD") return Fact::FACT_KINGPIN_DEAD;
	else if (s == "KIDS_ARE_FREE") return Fact::FACT_KIDS_ARE_FREE;
	else if (s == "PLAYER_IN_SAME_ROOM") return Fact::FACT_PLAYER_IN_SAME_ROOM;
	else if (s == "ANGEL_TRIGGER_CONDITION") return Fact::FACT_ANGEL_TRIGGER_CONDITION;
	else if (s == "PLAYER_IN_CONTROLLED_DRASSEN_MINE") return Fact::FACT_PLAYER_IN_CONTROLLED_DRASSEN_MINE;
	else if (s == "PLAYER_SPOKE_TO_CAMBRIA_MINER") return Fact::FACT_PLAYER_SPOKE_TO_CAMBRIA_MINER;
	else if (s == "PLAYER_IN_CONTROLLED_CAMBRIA_MINE") return Fact::FACT_PLAYER_IN_CONTROLLED_CAMBRIA_MINE;
	else if (s == "PLAYER_SPOKE_TO_CHITZENA_MINER") return Fact::FACT_PLAYER_SPOKE_TO_CHITZENA_MINER;
	else if (s == "PLAYER_IN_CONTROLLED_CHITZENA_MINE") return Fact::FACT_PLAYER_IN_CONTROLLED_CHITZENA_MINE;
	else if (s == "PLAYER_SPOKE_TO_ALMA_MINER") return Fact::FACT_PLAYER_SPOKE_TO_ALMA_MINER;
	else if (s == "PLAYER_IN_CONTROLLED_ALMA_MINE") return Fact::FACT_PLAYER_IN_CONTROLLED_ALMA_MINE;
	else if (s == "PLAYER_SPOKE_TO_GRUMM_MINER") return Fact::FACT_PLAYER_SPOKE_TO_GRUMM_MINER;
	else if (s == "PLAYER_IN_CONTROLLED_GRUMM_MINE") return Fact::FACT_PLAYER_IN_CONTROLLED_GRUMM_MINE;
	else if (s == "LARRY_CHANGED") return Fact::FACT_LARRY_CHANGED;
	else if (s == "PLAYER_KNOWS_ABOUT_BLOODCAT_LAIR") return Fact::FACT_PLAYER_KNOWS_ABOUT_BLOODCAT_LAIR;
	else if (s == "HOSPITAL_FREEBIE_DECISION_MADE") return Fact::FACT_HOSPITAL_FREEBIE_DECISION_MADE;
	else if (s == "ENOUGH_LOYALTY_TO_TRAIN_MILITIA") return Fact::FACT_ENOUGH_LOYALTY_TO_TRAIN_MILITIA;
	else if (s == "WALKER_AT_BAR") return Fact::FACT_WALKER_AT_BAR;
	else if (s == "JOEY_ALIVE") return Fact::FACT_JOEY_ALIVE;
	else if (s == "UNPROPOSITIONED_FEMALE_SPEAKING_TO_NPC") return Fact::FACT_UNPROPOSITIONED_FEMALE_SPEAKING_TO_NPC;
	else if (s == "84_AND_85_TRUE") return Fact::FACT_84_AND_85_TRUE;
	else if (s == "TOURIST_ESCORT_QUEST_STARTED") return Fact::FACT_TOURIST_ESCORT_QUEST_STARTED;
	else if (s == "SKYRIDER_ESCORT_QUEST_OFFERED") return Fact::FACT_SKYRIDER_ESCORT_QUEST_OFFERED;
	else if (s == "JOEY_AGREED_TO_ESCORT_UNDER_THREAT") return Fact::FACT_JOEY_AGREED_TO_ESCORT_UNDER_THREAT;
	else if (s == "KINGPIN_WILL_LEARN_OF_MONEY_GONE") return Fact::FACT_KINGPIN_WILL_LEARN_OF_MONEY_GONE;
	else if (s == "SKYRIDER_IN_B15") return Fact::FACT_SKYRIDER_IN_B15;
	else if (s == "SKYRIDER_IN_C16") return Fact::FACT_SKYRIDER_IN_C16;
	else if (s == "SKYRIDER_IN_E14") return Fact::FACT_SKYRIDER_IN_E14;
	else if (s == "SKYRIDER_IN_D12") return Fact::FACT_SKYRIDER_IN_D12;
	else if (s == "SKYRIDER_HINT_GIVEN") return Fact::FACT_SKYRIDER_HINT_GIVEN;
	else if (s == "KINGPIN_IS_ENEMY") return Fact::FACT_KINGPIN_IS_ENEMY;
	else if (s == "BRENDA_PATIENCE_TIMER_EXPIRED") return Fact::FACT_BRENDA_PATIENCE_TIMER_EXPIRED;
	else if (s == "DYNAMO_NOT_SPEAKER") return Fact::FACT_DYNAMO_NOT_SPEAKER;
	else if (s == "PABLO_BRIBED") return Fact::FACT_PABLO_BRIBED;
	else if (s == "CONRAD_SHOULD_GO") return Fact::FACT_CONRAD_SHOULD_GO;
	else if (s == "PLAYER_KILLED_BOXERS") return Fact::FACT_PLAYER_KILLED_BOXERS;
	else throw DataError(ST::format("unknown fact name value: {}", s));
}

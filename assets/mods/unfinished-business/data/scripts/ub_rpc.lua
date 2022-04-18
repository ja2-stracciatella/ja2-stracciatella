RegisterListener("OnInitNewCampaign", "init_rpcs")
RegisterListener("BeforePrepareSector", "add_rpc_to_sector")
RegisterListener("OnNPCDoAction", "handle_rpc_actions")

FIRST_MAP_PLACED_FREQUENCY = 50
NPC_ACTION_RECRUIT_WITH_SALARY = 23

function init_rpcs()
    -- randomly choose between JOHN_K and TEX
    if math.random() < 0.5 then
        log.info("TEX is in game")
        extra_game_state.is_tex_in_game = true
        extra_game_state.is_johnk_in_game = false
        SetFactTrue(UB_Facts.FACT_TEX_IS_IN_GAME_AND_ALIVE_IN_STORE)
    else
        log.info("JOHN_K is in game")
        extra_game_state.is_tex_in_game = false
        extra_game_state.is_johnk_in_game = true
        SetFactFalse(UB_Facts.FACT_TEX_IS_IN_GAME_AND_ALIVE_IN_STORE)
    end

    -- randomly choose John's starting secto
    if extra_game_state.is_johnk_in_game then
        if math.random() < 0.5 then
            extra_game_state.johnk_initial_sector = "I10"
        else
            extra_game_state.johnk_initial_sector = "I11"
        end
        log.info("JOHN_K is at " .. extra_game_state.johnk_initial_sector)
    end

    extra_game_state.rpcs_added = {
        [ UB_MercProfiles.TEX    ] = false,
        [ UB_MercProfiles.JOHN_K ] = false,
        [ UB_MercProfiles.MANUEL ] = false,
    }
    extra_game_state.biggens_given_warning = false
    extra_game_state.biggens_set_off_bombs = false
end


function add_rpc_to_sector()
    -- add RPC(s) on specific sectors
    local current_sector = GetCurrentSector()

    if extra_game_state.rpcs_added == nil then
        log.warn("rpcs_added not yet initialized")
        extra_game_state.rpcs_added = {}
    end

    if current_sector == "H10" or current_sector == "I9" and
        not extra_game_state.rpcs_added[UB_MercProfiles.MANUEL]
    then
        add_rpc(UB_MercProfiles.MANUEL)
    end

    if extra_game_state.is_tex_in_game and
        CheckFact(UB_Facts.FACT_TEX_IS_IN_GAME_AND_ALIVE_IN_STORE, 0) == TRUE and
        current_sector == "I10" and
        not extra_game_state.rpcs_added[UB_MercProfiles.TEX]
    then
        add_rpc(UB_MercProfiles.TEX)
    end

    if extra_game_state.is_johnk_in_game and
        current_sector == extra_game_state.johnk_initial_sector and
        not extra_game_state.rpcs_added[UB_MercProfiles.JOHN_K]
    then
        add_rpc(UB_MercProfiles.JOHN_K)
    end
end

function add_rpc(rpc_to_add)
    local rpc = GetMercProfile(rpc_to_add)
    local x,y,z = GetCurrentSectorLoc()
    rpc.sSectorX = x
    rpc.sSectorY = y
    rpc.bSectorZ = z

    log.info("rpc " .. rpc.zNickname .. " added to sector")
    if extra_game_state.rpcs_added == nil then
        extra_game_state.rpcs_added = {}
    end

    extra_game_state.rpcs_added[rpc_to_add] = true
end

function handle_rpc_actions(target_npc, action_code, quote_num, is_handled)
    if is_handled.val == TRUE then
        return
    end

    if action_code == UB_NPCActions.NPC_ACTION_TEX_FLUSHES_TOILET then
        log.debug("flushes toilet")
        PlayJA2SampleFromFile("SOUNDS/ToiletFlush.wav", 127, 1, 64) --HIGHVOLUME, 1, MIDDLE
        is_handled.val = TRUE
    elseif action_code == UB_NPCActions.NPC_ACTION_MARK_TEX_AS_ALREADY_INTRODUCED_HIMSELF then
        log.debug('Tex making himself already be introduced')
        local profile = GetMercProfile(UB_MercProfiles.TEX)
        profile.ubLastDateSpokenTo = GetWorldDay()
        is_handled.val = TRUE
    elseif action_code == UB_NPCActions.NPC_ACTION_MAKE_TEX_CAMOED then
        log.debug("Tex turning camo")
        local tex = FindSoldierByProfileID(UB_MercProfiles.TEX)
        if tex ~= nil then
            --make him camoed
            tex.bCamo = 100
            --CreateSoldierPalettes( pSoldier );
        end
        -- Then set him to be camo'ed in the profile ( cause he is still an RPC and we are just about to hire him )
        local profile = GetMercProfile(UB_MercProfiles.TEX)
        profile.bSkillTrait2 = SkillTraits.CAMOUFLAGED
        --Close down the talking menu...
        DeleteTalkingMenu()
        -- Trigger Tex to say the quote, this will cause the radio locater to come up giving a pause
        -- to make it appear that he put on camoflauge
        TriggerNPCRecord(UB_MercProfiles.TEX, 15)
        is_handled.val = TRUE
    end
end

--[[
HandleQuestCodeOnSectorExit
//if the player is leaving a sector with  Tex in it
	if( sOldSectorX == gMercProfiles[ TEX ].sSectorX && sOldSectorY == gMercProfiles[ TEX ].sSectorY && bOldSectorZ == 0 && gMercProfiles[ TEX ].ubLastDateSpokenTo != 0 )
	{
		pSoldier = FindSoldierByProfileID( TEX, TRUE );

		//if the npc isnt on the players team AND the player has never spoken to them
		if( pSoldier == NULL && gMercProfiles[ TEX ].ubLastDateSpokenTo != 0 )
		{
			// remove Tex from the map
			gMercProfiles[ TEX ].sSectorX = 0;
			gMercProfiles[ TEX ].sSectorY = 0;
		}
	}//same for JOHNK
--]]
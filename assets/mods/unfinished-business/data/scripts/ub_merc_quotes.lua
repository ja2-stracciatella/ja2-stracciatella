RegisterListener('OnStrategicEvent', 'handle_merc_say_quote_delayed')
RegisterListener('OnEnterSector', 'handle_merc_quote_on_enter_sector')
RegisterListener('OnSoldierGotItem', 'handle_soldier_got_item')

SectorDescQuotes = {
    H9  = 86,  --QUOTE_HATE_MERC_1_ON_TEAM,
    I9  = 88,  --QUOTE_LEARNED_TO_HATE_MERC_ON_TEAM,
    H10 = 88,  --QUOTE_LEARNED_TO_HATE_MERC_ON_TEAM,
    I10 = 87,  --QUOTE_HATE_MERC_2_ON_TEAM,
    J13 = 117, --QUOTE_ENTER_SECTOR_WITH_FAN_1,
    ["J14-1"] = 0,
};

NewGuns = {
    [UB_Weapons.BARRETT] = true,
    [UB_Weapons.CALICO_960] = true,
    [UB_Weapons.PSG1] = true,
    [UB_Weapons.L85] = true,
    [UB_Weapons.TAR21] = true,
    [UB_Weapons.VAL_SILENT] = true,
    [UB_Weapons.MICRO_UZI] = true,
    [UB_Weapons.HAND_CANNON] = true,
    [UB_Weapons.CALICO_950] = true,
    [UB_Weapons.CALICO_900] = true,
}

DELAY_FOR_PLAYER_DESC_OF_SECTOR = 2

-- list all mercs, in current sector, that has the new UB quotes, in randomized order
function get_mercs_with_ub_quotes()
    local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
    local mercs = {}
    local x,y,z = GetCurrentSectorLoc()
    for _, s in ipairs(our_team) do
        if s.bActive       and
           s.sSectorX == x and
           s.sSectorY == y and
           s.bSectorZ == z and
           s.bLife >= 15   and --OKLIFE
           s.fBetweenSectors == FALSE
        then
            -- if the merc is one of the mercs we are looking for
            if soldier_has_ub_quotes(s) then
                table.insert(mercs, s)
            end
        end
    end

    log.debug("got " .. #mercs .. " qualified mercs")
    return shuffle(mercs)
end

function shuffle(arr)
    for i = 1, #arr do
        local j = math.random(#arr)
        arr[i], arr[j] = arr[j], arr[i]
    end
    return arr
end

function soldier_has_ub_quotes(pSoldier)
    -- also called "qualified mercs" in UB source code
    if pSoldier.ubProfile == UB_MercProfiles.GASTON or
       pSoldier.ubProfile == UB_MercProfiles.STOGIE or
       pSoldier.ubProfile == UB_MercProfiles.TEX or
       pSoldier.ubProfile == UB_MercProfiles.JOHN_K or
       pSoldier.ubProfile == UB_MercProfiles.BIGGENS or
       pSoldier.ubProfile == UB_MercProfiles.MANUEL or
       pSoldier.ubWhatKindOfMercAmI == MercTypes.PLAYER_CHARACTER
    then
        return true
    else
        return false
    end
end

-- schedule a callback when a merc should say a quote
function merc_say_quote_delayed(profile_id, quote_num, seconds_delay)
    log.info("delayed quote: " .. profile_id .. " " .. quote_num)
    local event_param = profile_id + (quote_num << 16)
    local event_time = GetWorldTotalSeconds() + seconds_delay
    AddStrategicEvent(UB_EventTypes.EVENT_SAY_DELAYED_MERC_QUOTE, event_time, event_param);
end

function handle_merc_say_quote_delayed(event, processed)
    if event.ubEventKind == UB_EventTypes.EVENT_SAY_DELAYED_MERC_QUOTE then
        -- handles the callback scheduled by merc_say_quote_delayed

        local profile_id = (0xFFFF & event.uiParam)
        local quote_num = (event.uiParam >> 16)
        -- TODO: handle "special" quotes (quoteNum < DQ__NORMAL_DELAYED_QUOTE)

        -- Get the soldier that should say the quote
        local soldier = FindSoldierByProfileID(profile_id)
        if not soldier then
            log.warn("cannot find soldier with profile #" .. profile_id)
            return
        end

        -- Do Quote specific code here
        if quote_num == UB_Quotes.QUOTE_COMPLAIN_ITS_COLD then
            -- if the soldier is saying the 'brr its cold' quote, and he has left the sector
            if soldier.sSectorX ~= 7 or soldier.sSectorY ~= 8 or soldier.bSectorZ ~= 0 then
                -- dont say the quote
                log.debug("skipping cold quote")
                return
            end
        end

        -- Say the quote
        TacticalCharacterDialogue(soldier, quote_num)
        processed.val = TRUE
    end

    if event.ubEventKind == UB_EventTypes.EVENT_MERC_SAY_QUOTE_WHEN_TALK_MENU_CLOSES then
        log.info("something merc to say")
        -- handles callback when NPC stops talking and our merc has something to day
        if (gTacticalStatus.uiFlags & TacticalStatusFlags.ENGAGED_IN_CONV) == 1 then
            --if their scripting event going on, leave
            log.warn('scripting event still going on')
            return
        end

        local npc_id = event.uiParam
        local npc = FindSoldierByProfileID(npc_id)

        local quote_num = 0
        if npc_id == UB_MercProfiles.BETTY then
            quote_num = 92  --QUOTE_JOINING_CAUSE_BUDDY_1_ON_TEAM
        elseif npc_id == UB_MercProfiles.RAUL and not extra_game_state.raul_blown_himself_up then
            -- unless Raul is about to blow himself up
            quote_num = 93  --QUOTE_JOINING_CAUSE_BUDDY_2_ON_TEAM
        elseif npc_id == UB_MercProfiles.RUDY then
            quote_num = 95 --QUOTE_REFUSAL_RENEW_DUE_TO_MORALE
        else
            log.warn("Code has not been implemented to handle saying a quote when leaving this NPC (" .. npc_id .. ")")
        end

        log.info("quote_num = " .. quote_num)
        if quote_num > 0 then
            local mercs = get_mercs_with_ub_quotes()
            for _, soldier in ipairs(mercs) do
                if PythSpacesAway(soldier.sGridNo, npc.sGridNo) < 10 and
                        soldier.fMercAsleep ~= TRUE  --TODO: also check LOS
                then
                    TacticalCharacterDialogue(soldier, quote_num);
                    break
                else
                    log.info("distance = " .. PythSpacesAway(soldier.sGridNo, npc.sGridNo))
                    log.info("asleep = " .. soldier.fMercAsleep)
                end
                log.warn("no merc available to talk")
            end
            SetFactFalse(UB_Facts.FACT_MERC_SAY_QUOTE_WHEN_TALK_MENU_CLOSES)
        end
        processed.val = TRUE
    end
end

function handle_merc_quote_on_enter_sector(x, y, z)
    if extra_game_state.sector_quotes_said == nil then
        extra_game_state.sector_quotes_said = {}
    end

    local sector = GetCurrentSector()
    local quote_num = SectorDescQuotes[sector]
    local all_mercs = get_mercs_with_ub_quotes()
    if not quote_num or extra_game_state.sector_quotes_said[quote_num] then
        -- no quotes, no mercs with quote, or quote already said
        log.info("no sector quotes")
        return
    end
    if #all_mercs == 0 then
        log.info("no mercs can say quote")
        return
    end

    local mercs_to_talk = {}
    if sector == 'I10' then
        -- everybody gets to talk
        mercs_to_talk = all_mercs
    elseif sector == 'J13' then
        -- first try MANUEL, or else BIGGENS
        for _, m in ipairs(all_mercs) do
            if m.ubProfile == UB_MercProfiles.MANUEL then
                mercs_to_talk[1] = m
                break
            elseif m.ubProfile == UB_MercProfiles.BIGGENS then
                extra_game_state.biggens_said_quote_117 = true
                mercs_to_talk[1] = m -- still continue to look for MANUEL
            end
        end
        if #mercs_to_talk > 0 then
            SetFactTrue(UB_Facts.FACT_PLAYER_KNOWS_ABOUT_FAN_STOPPING)
        end
    elseif sector == 'J14-1' then
        -- TODO: depends if all merc gets through, have one merc say the quote
        mercs_to_talk[1] = all_mercs[1]
        --[[ TODO:
        QUOTE_CONTRACTS_OVER if some got thru
        QUOTE_CONTRACT_ACCEPTANCE if all got thru
        --]]
    else
        -- by default, pick a random one from list
        mercs_to_talk[1] = all_mercs[1]
    end

    log.info("delay sector quote")
    for _, m in ipairs(mercs_to_talk) do
        merc_say_quote_delayed(m.ubProfile, quote_num, DELAY_FOR_PLAYER_DESC_OF_SECTOR)
    end

    extra_game_state.sector_quotes_said[quote_num] = true
end

function handle_soldier_got_item(soldier, object, grid_no, z_level)
    log.debug("got item " .. object.usItem)
    if extra_game_state.gun_quotes_said == nil then
        extra_game_state.gun_quotes_said = {}
    end

    if extra_game_state.gun_quotes_said[object.usItem] == true then
        -- gun comment already said
        log.info("new gun comment already said")
        return
    end

    if soldier_has_ub_quotes(soldier) and NewGuns[object.usItem] then
        log.info("say new gun comment")
        -- say the new gun quote
        --TODO: if not HAND_CANNON
        TacticalCharacterDialogue(soldier, UB_Quotes.QUOTE_NEW_GUN_COMMENT)
        extra_game_state.gun_quotes_said[object.usItem] = true
        -- TODO: disable the old cool item quote
    end
end
--[[

HandlePlayerTeamQuotesWhenEnteringSector

void HandleMercArrivesQuotesFromHeliCrashSequence()
{
	UINT32 uiCnt;
	SOLDIERTYPE *pSoldier=NULL;

	uiCnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// look for all mercs on the same team,
	for ( pSoldier = MercPtrs[ uiCnt ]; uiCnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; uiCnt++,pSoldier++)
	{
		if ( pSoldier->bActive && pSoldier->bLife >= OKLIFE && pSoldier->bInSector )
		{
			HandleMercArrivesQuotes( pSoldier );
		}
	}
}
--]]


--[[

start merc quotes
pSoldier = pick random one qualified merc
		TacticalCharacterDialogue( pSoldier, QUOTE_LAME_REFUSAL );
pSoldier = pick another qualified merc (if any)
		//Say the quote in 15 seconds
		DelayedMercQuote( ubProfileID, QUOTE_DEPARTING_COMMENT_CONTRACT_NOT_RENEWED_OR_48_OR_MORE, GetWorldTotalSeconds( ) + 15 );


--]]
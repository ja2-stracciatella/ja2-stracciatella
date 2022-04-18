RegisterListener("OnInitNewCampaign", "new_campaign")
RegisterListener("OnMercHired", "init_merc_placement")
RegisterListener("OnEnterTacticalScreen", "on_enter_tactical_screen")
RegisterListener("OnHandleStrategicScreen" , "handle_strategic_screen")
RegisterListener("BeforePrepareSector", "prepare_heli_crash")
RegisterListener("OnTimeCompressDisallowed", "handle_timecompress_disallow")
RegisterListener("OnNPCDoAction", "handle_jerry_npc_action")
RegisterListener("OnStrategicEvent", "handle_event_callback")

ANIM_STAND = 6
INSERTION_CODE_GRIDNO = 4

function new_campaign()
    -- initialize extra_game_state
    extra_game_state = {
        first_time_in_map_screen = true,
        ever_been_in_tactical = false,
    }

    log.info("placing jerry")
    local jerry = GetMercProfile(UB_MercProfiles.JERRY)
    jerry.sSector.x = INIT_SECTOR_X
    jerry.sSector.y = INIT_SECTOR_Y
    jerry.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO
    jerry.usStrategicInsertionData = 15109
    jerry.fUseProfileInsertionInfo = TRUE
end

function init_merc_placement(s)
    log.info("hired merc " .. s.ubID) -- TODO: allow fire before arrival
    if gTacticalStatus.fDidGameJustStart then
        local init_heli_grid_nos = { 14947, 15584, 15754, 16232, 16067, 16230, 15272,
                                     15000, 15001, 15002, 15003, 15004, 15005, 15006, 15007, 15008 -- should not be allowed at all
        }
        log.info("moving merc to initial gridNo " .. init_heli_grid_nos[s.ubID + 1])
        s.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO
        s.usStrategicInsertionData = init_heli_grid_nos[s.ubID + 1]
        s.ubInsertionDirection = math.random(0, 7) + 100
        s.usAnimState = 62 + math.random(0,1) * 2  -- 62 or 64, randomly
    end
end
--[[
	gsInitialHeliRandomTimes[ 0 ] = 1300;
	gsInitialHeliRandomTimes[ 1 ] = 2000;
	gsInitialHeliRandomTimes[ 2 ] = 2750;
	gsInitialHeliRandomTimes[ 3 ] = 3400;
	gsInitialHeliRandomTimes[ 4 ] = 4160;
	gsInitialHeliRandomTimes[ 5 ] = 4700;
	gsInitialHeliRandomTimes[ 6 ] = 5630;
}
--]]

function jerry_say_quote(quote_num)
    if extra_game_state.jerry_said_quotes == nil then
        extra_game_state.jerry_said_quotes = {}
    end

    -- do not repeat if the quote has been said already
    if extra_game_state.jerry_said_quotes[quote_num] then
        return
    end

    StrategicNPCDialogue(UB_MercProfiles.JERRY, quote_num)
    extra_game_state.jerry_said_quotes[quote_num] = true
end

function handle_strategic_screen()
    if extra_game_state.first_time_in_map_screen then
        -- Get Jerry Milo to say his opening quote, if he hasnt said it before)
        jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__OPENING_GREETING_PART_1)
        jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__OPENING_GREETING_PART_2)

        extra_game_state.first_time_in_map_screen = false
        return
    end

    if gTacticalStatus.fDidGameJustStart == TRUE then
        local team = ListSoldiersFromTeam(Team.OUR_TEAM)
        if #team == 0 then
            jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__PLAYER_HAS_NO_MERCS)
        elseif #team < 6 then
            jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__PLAYER_HAS_LESS_THEN_6_MERCS)
        elseif #team == 6 then
            jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__ALREADY_HAS_6_MERCS)
        else
            jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__PLAYER_HAS_MORE_THEN_6_MERCS) -- TODO: actually disallowing more than 6 mercs
        end
    end
end

--[[
Prepare for the crash landing start scene
--]]
function prepare_heli_crash()
    --if extra_game_state.ever_been_in_tactical
    --    or GetCurrentSector() ~= INIT_SECTOR
    --then
    --    return
    --end
    --
    -- -- first, loop through all the mercs and injure them
    --local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
    --for _, s in ipairs(our_team) do
    --    --if s.bActive then
    --    --    s.bLife = s.bLife - (3 + math.random(0, 5))
    --    --    s.sBreathRed = (15 + math.random(0, 15)) * 100
    --    --end
    --end
end


--[[
Setting up for Heli crash sequence upon landing at the first sector
--]]
function on_enter_tactical_screen()
    gTacticalStatus.fEnemyFlags = gTacticalStatus.fEnemyFlags & 0x01  -- TODO: do it "properly"? (ENEMY_OFFERED_SURRENDER)

    if extra_game_state.ever_been_in_tactical then
        return
    end

    if GetCurrentSector() ~= INIT_SECTOR then
        return
    end

    log.info("landed in H7")
    gTacticalStatus.bMercArrivingQuoteBeingUsed = TRUE
    gTacticalStatus.fEnemyInSector = FALSE
    gTacticalStatus.uiFlags = gTacticalStatus.uiFlags & (~TacticalStatusFlags.INCOMBAT)
    CenterAtGridNo(15427, true)

    s = SGPSector.new()
    s.x = INIT_SECTOR_X
    s.y = INIT_SECTOR_Y
    SetThisSectorAsPlayerControlled(s, 0, FALSE)

    local jerry = GetMercProfile(UB_MercProfiles.JERRY)
    jerry.ubLastDateSpokenTo = 1 -- ensure we do not prompt him as a NPC yet to talk to

    local jerry_s = FindSoldierByProfileID(UB_MercProfiles.JERRY)
    ChangeSoldierState(jerry_s, 62, 1, TRUE)
    jerry_s.bVisible = TRUE

    -- first, loop through all the mercs and injure them
    local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
    for _, s in ipairs(our_team) do
        if s.bActive == TRUE then
            s.bLife = s.bLife - (3 + math.random(0, 5))
            s.sBreathRed = (15 + math.random(0, 15)) * 100
            -- should we be on our back or tummy
            local anim_state = 62 + math.random(0,1) * 3  -- random between 62(FALLBACKHIT_STOP) and 65(STAND_FALLFORWARD_STOP)
            ChangeSoldierState(s, anim_state, 1, TRUE)
        end
    end

    AddStrategicEvent(UB_EventTypes.EVENT_INIT_HELI_CRASH_SEQUENCE, GetWorldTotalSeconds()+3, 0)
    extra_game_state.ever_been_in_tactical = true
end

-- Initial scripted sequence right after the crash landing, done via a custom StrategicEvent
function handle_event_callback(event, processed)
    if event.ubEventKind ~= UB_EventTypes.EVENT_INIT_HELI_CRASH_SEQUENCE then
        return
    end

    if event.uiParam < 100 then
        -- stage 1: mercs getting up
        local next = event.uiParam + 1
        local mercs = ListSoldiersFromTeam(Team.OUR_TEAM)
        local next_merc = mercs[next]
        ChangeSoldierStance(next_merc, ANIM_STAND)
        --if the merc is one of the mercs who has Quote 80, say that
        if soldier_has_ub_quotes(next_merc) then
            TacticalCharacterDialogue(next_merc, UB_Quotes.QUOTE_STILL_ALIVE)
        else
            DoMercBattleSound(next_merc, BattleSound.BATTLE_SOUND_CURSE1)
        end

        if next >= #mercs then
            next = 100 -- proceed to next stage
        end
        AddStrategicEvent(UB_EventTypes.EVENT_INIT_HELI_CRASH_SEQUENCE, GetWorldTotalSeconds()+4, next)
    elseif event.uiParam == 100 then
        -- stage 2: jerry sequence
        local jerry = FindSoldierByProfileID(UB_MercProfiles.JERRY)
        DeleteTalkingMenu()
        ChangeSoldierStance(jerry, ANIM_STAND)
        -- the internet part of the laptop isnt working.  It gets broken in the heli crash.
        PlayJA2SampleFromFile("SOUNDS/Metal Antenna Crunch.wav", 127, 1, 64)
        AddStrategicEvent(UB_EventTypes.EVENT_INIT_HELI_CRASH_SEQUENCE, GetWorldTotalSeconds()+1, 101)

    elseif event.uiParam == 101 then
        local mercs = ListSoldiersFromTeam(Team.OUR_TEAM)
        local random_merc = mercs[math.random(1, #mercs)]
        local text = GetMercProfile(random_merc.ubProfile).zNickname .. " hears the sound of crumpling metal coming from underneath Jerry's body.  It sounds disturbingly like your laptop antenna being crushed."
        ExecuteTacticalTextBox(110, 20, text) --TODO: textbox does not close properly until the next TacticalCharDialog

        AddStrategicEvent(UB_EventTypes.EVENT_INIT_HELI_CRASH_SEQUENCE, GetWorldTotalSeconds()+4, 102)
    elseif event.uiParam == 102 then
        -- stage 3: trigger Jerry's NPC action sequence and start the game
        -- Trigger Jerry Milo's script record 10 ( call action 301 )
        TriggerNPCRecord(UB_MercProfiles.JERRY, 10)
        StartQuest(UB_Quests.QUEST_FIX_LAPTOP, "")
    else
        log.error('unhandled sequence #' .. event.uiParam)
    end

    processed.val = TRUE
end

function handle_jerry_npc_action(npc_id, action_code, quote_num, handled)
    if action_code == UB_NPCActions.NPC_ACTION_TRIGGER_JERRY_CONVERSATION_WITH_PGC_1 then
        -- action 301
        local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
        for _, s in ipairs(our_team) do
            if soldier_has_ub_quotes(s) then
                -- complain about bad flight
                TacticalCharacterDialogue(s, UB_Quotes.QUOTE_COMPLAIN_BAD_PILOT)
            end
        end
        -- Trigger Jerry Milo's script record 11 ( call action 302 )
        TriggerNPCRecord(UB_MercProfiles.JERRY, 11)
        DeleteTalkingMenu() -- Close the dialogue panel
        handled.val = TRUE

    elseif action_code == UB_NPCActions.NPC_ACTION_TRIGGER_JERRY_CONVERSATION_WITH_PGC_2 then
        -- action 302
        local random_mercs = get_mercs_with_ub_quotes()
        if #random_mercs > 0 then
            -- have one merc say we shouldn't stay long
            TacticalCharacterDialogue(random_mercs[1], UB_Quotes.QUOTE_DONT_STAY_FOR_LONG)
            -- have one merc say the "brr it's cold" quote
            merc_say_quote_delayed(random_mercs[#random_mercs].ubProfile, UB_Quotes.QUOTE_COMPLAIN_ITS_COLD, 15)
        end
        DeleteTalkingMenu() -- Close the dialogue panel
        handled.val = TRUE
    end
end

function handle_timecompress_disallow(handled)
    if extra_game_state.ever_been_in_tactical then
        return
    end

    -- before we landed in Tracona
    local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
    --if #our_team == 0 then
    --    jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__PLAYER_HAS_NO_MERCS)
    --    handled.val = TRUE
    --else
        jerry_say_quote(UB_JerryQuotes.MILO_QUOTE__HEADED_TO_TRACONA_QUOTE)
        handled.val = TRUE
    --end
end
RegisterListener("OnInitNewCampaign", "init_rpc_biggens")
RegisterListener("OnNPCDoAction", "handle_rpc_actions")
RegisterListener('OnItemAction', 'handle_biggens_action_item')

FIRST_MAP_PLACED_FREQUENCY = 50
NPC_ACTION_RECRUIT_WITH_SALARY = 23

function init_rpc_biggens()
    extra_game_state.biggens_given_warning = false
    extra_game_state.biggens_set_off_bombs = false
end

function handle_biggens_action_item(grid_no, obj, processed)
    local action = obj.bActionValue
    local biggens = GetMercProfile(UB_MercProfiles.BIGGENS)
    local biggens_hostile_and_alive = biggens.bLife > 0 and not is_soldier_on_our_team(UB_MercProfiles.BIGGENS)

    if action == UB_ActionItems.ACTION_ITEM_BIGGENS_BOMBS then
        stop_all_soldiers()
        if biggens_hostile_and_alive and not extra_game_state.biggens_set_off_bombs then
            -- Make Biggens run for cover and then detonate the explosives
            log.info("trigger ACTION_ITEM_BIGGENS_BOMBS")
            extra_game_state.biggens_set_off_bombs = true
            TriggerNPCRecord(UB_MercProfiles.BIGGENS, 17)
            
            --TODO: sometimes Biggens just do not run, and the game is locked up infinitely. Don't know why
        end
        processed.val = TRUE
    elseif action == UB_ActionItems.ACTION_ITEM_BIGGENS_WARNING then
        if biggens_hostile_and_alive and not extra_game_state.biggens_given_warning then
            stop_all_soldiers()
            -- Have Biggens spit out a warning about the bombs,
            log.info("trigger ACTION_ITEM_BIGGENS_WARNING")
            extra_game_state.biggens_given_warning = true
            TriggerNPCRecord(UB_MercProfiles.BIGGENS, 16)
        end
        processed.val = TRUE
    end
end

function handle_rpc_actions(target_npc, action_code, quote_num, is_handled)
    if is_handled.val == TRUE then
        return
    end

    if action_code == UB_NPCActions.NPC_ACTION_BIGGENS_DETONATES_BOMBS then
        local biggens = FindSoldierByProfileID(UB_MercProfiles.BIGGENS)
        if biggens ~= nil then
            -- have Biggens trigger the bombs by the cave wall
            log.info("setting off bombs")
            SetOffBombsByFrequency(biggens, FIRST_MAP_PLACED_FREQUENCY + 1)
        end
        is_handled.val = TRUE
    elseif action_code == NPC_ACTION_RECRUIT_WITH_SALARY and target_npc == UB_MercProfiles.BIGGENS then
        log.info("BIGGENS is recruited")
        SetFactTrue(UB_Facts.FACT_BIGGENS_IS_ON_TEAM)
    end
end

function is_soldier_on_our_team(profile_id)
    local s = FindSoldierByProfileID(profile_id)
    if s and s.bTeam == Team.OUR_TEAM then
        return true
    else
        return false
    end
end

function stop_all_soldiers()
    local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
    for _, s in ipairs(our_team) do
        StopSoldier(s)
    end
end
-- TODO: after bombs were set off, disable sector exit to underground, and draw the collapsed entrance
--[[
Handles the enemy attack at the initial sector
--]]
RegisterListener('OnStrategicEvent', 'handle_strategic_event')

function handle_strategic_event(event, processed)
    if event.ubEventKind ==  UB_EventTypes.EVENT_ATTACK_INITIAL_SECTOR_IF_PLAYER_STILL_THERE then
        log.info('attack if player is still there')
        if GetCurrentSector() == 'H7' and mercs_still_at_sector(7, 8, 0) then -- also check gfWorldLoaded?
            send_squad_to_h7()
        end
        processed.val = TRUE
    end
end

function mercs_still_at_sector(sectorX, sectorY, sectorZ)
    local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
    for _, s in ipairs(our_team) do
        if s.bActive and
            s.sSectorX == sectorX and
            s.sSectorY == sectorY and
            s.bSectorZ == sectorZ
        then
            return true
        end
    end
    return false
end

function send_squad_to_h7()
    local next_sector = GetSectorInfo("H8")
    local numTroops = math.floor(next_sector.ubNumTroops / 3) + math.random(0, 3)  --[[ TODO: should also send elites if any ]]
    log.info('sending ' .. numTroops .. ' troops from H8')

    -- deduct the # that are moving from the # in the guard post sector
    next_sector.ubNumTroops = math.max(0, next_sector.ubNumTroops - numTroops)
    local enemies = CreateNewEnemyGroupDepartingSector("H8", 0, numTroops, 0)

    -- Set the sector to attack
    enemies.ubNextX = INIT_SECTOR_X
    enemies.ubNextY = INIT_SECTOR_Y
    enemies.uiTraverseTime = 10
    enemies.ubMoveType = 0  -- ONE_WAY   --[[ TODO also .pEnemyGroup.ubIntention = ASSAULT ]]
    enemies:setArrivalTime(GetWorldTotalMin() + 1)
    AddStrategicEvent(EventTypes.EVENT_GROUP_ARRIVAL, (GetWorldTotalMin() + 1) * 60, enemies.ubGroupID)  --[[ TODO: confirm if it is every-day recurring ]]
end



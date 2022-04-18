RegisterListener("OnCalcPlayerProgress", "calc_current_progress")

UB_SECTORS_PROGRESS = {
    H7 = 44,   -- initial sector
    H8 = 45,
    H9 = 55,   -- guard post
    I9 = 60,
    H10 = 60,
    I10 = 63,
    I11 = 65,
    J11 = 68,
    I12 = 68,
    J12 = 70,

    I13 = 70,
    J13 = 75,         -- power gen plant
    ["I13-1"] = 72,
    ["J13-1"] = 75,
    ["J14-1"] = 80,   -- first part of tunnel
    ["K14-1"] = 83,   -- second part of tunnel
    K15       = 90,   -- ground level of complex
    ["K15-1"] = 85,   -- initial sector of complex
    ["K15-2"] = 95,   -- 2nd level down of complex
    ["L15-2"] = 98,   -- 2nd last sector
    ["L15-3"] = 100,  -- last sector
}

function calc_current_progress(player_progress)
    local furthest = 40 -- minimum progress % at game start
    for sector, progress in pairs(UB_SECTORS_PROGRESS) do
        -- find the further sector the player owns
        if progress > furthest then
            local is_player_controlled = false
            if is_underground(sector) then
                -- approximate by SF_ALREADY_VISITED and the absence of enemies
                local info = GetUndergroundSectorInfo(sector)
                if (info.uiFlags & SectorFlag.SF_ALREADY_VISITED ~= 0)
                    and info.ubNumElites == 0
                    and info.ubNumTroops == 0
                    and info.ubNumAdmins == 0
                then
                    is_player_controlled = true
                end
            else
                -- use StrategicMap
                local map_element = GetStrategicMapElement(sector)
                is_player_controlled = (map_element.fEnemyControlled == FALSE)
            end

            if is_player_controlled then
                furthest = progress
            end
        end
    end

    log.info("current progress is " .. furthest)
    player_progress.val = furthest
end

function is_underground(sector_id)
    return sector_id:find("-") ~= nil
end
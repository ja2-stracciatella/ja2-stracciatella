local NO_GARRISON = 255
RegisterListener("OnInitNewCampaign", "init_strategic_ai")

function init_strategic_ai()
    --Initialize the SectorInfo structure so all sectors don't point to a garrisonID.
    for y = 1,16,1 do
        for x = 0,15,1 do
            local sector = string.char(65+x) .. y
            local sectorInfo = GetSectorInfo(sector)
            sectorInfo.ubGarrisonID = NO_GARRISON
        end
    end

    init_bloodcats()
    init_surface_enemies()

    -- enemies coming to investigate heli crash - attack at the initial sector
    log.info("scheduled attack")
    AddEveryDayStrategicEvent(
        UB_EventTypes.EVENT_ATTACK_INITIAL_SECTOR_IF_PLAYER_STILL_THERE,
        7 * 60 + 10,  -- 07:10
        0
    )
end

function init_bloodcats()
    local num_bloodcats = 0
    if gGameOptions.ubDifficultyLevel == DifficultyLevel.DIF_LEVEL_EASY then
        num_bloodcats = 3
    elseif gGameOptions.ubDifficultyLevel == DifficultyLevel.DIF_LEVEL_MEDIUM then
        num_bloodcats = 4
    elseif gGameOptions.ubDifficultyLevel == DifficultyLevel.DIF_LEVEL_HARD then
        num_bloodcats = 5
    end

    local sec = GetSectorInfo("I10")
    sec.bBloodCats = num_bloodcats
    sec.bBloodCatPlacements = 5
end

function set_sector_enemies(sector_id, enemies)
    local sector = GetSectorInfo(sector_id)
    sector.ubNumAdmins = enemies[gGameOptions.ubDifficultyLevel][1]
    sector.ubNumTroops = enemies[gGameOptions.ubDifficultyLevel][2]
    sector.ubNumElites = enemies[gGameOptions.ubDifficultyLevel][3]
end

function init_surface_enemies()
    local rand = function(max) return math.random(0, max) end

    set_sector_enemies("H7", {
        {0,0,0}, {0,0,0}, {0,0,0}
    })

    set_sector_enemies("H8", {
        { 0,  6 + rand(4), 0},
        { 0, 13 + rand(3), 0},
        { 0, 16 + rand(6), 2 + rand(2) },
    })

    --Guard Post
    set_sector_enemies("H9", {
        { 0, 10 + rand(5), 1 },
        { 0, 16 + rand(4), 1 },
        { 0, 19 + rand(4), 3 + rand(2) },
    })
    
    set_sector_enemies("H10", {
        { 0, 6 + rand(5),  0 },
        { 0, 9 + rand(3),  0 },
        { 0, 10 + rand(4), 2 + rand(4) },
    })

    set_sector_enemies("H11", {
        { 1 + rand(2), 6 + rand(3), 0 },
        { 0,           8 + rand(3), 2 + rand(2) },
        { 0,           10 + rand(4), 4 + rand(3) },
    })

    set_sector_enemies("I9", {
        { 0, 6 + rand(5),  0},
        { 0, 9 + rand(4),  0},
        { 0, 10 + rand(4), 2 + rand(4) },
    })

    -- First part of town
    set_sector_enemies("I10", {
        { 0, 13 + rand(6), 0 },
        { 0, 16 + rand(4), 1 + rand(3) },
        { 0, 14 + rand(4), 5 + rand(3) },
    })

    -- Second part of town
    set_sector_enemies("I11", {
        { 0, 14 + rand(7),  0, },
        { 0, 16 + rand(5),  2 + rand(2) },
        { 0, 14 + rand(4),  5 + rand(4) },
    })

    set_sector_enemies("I12", {
        { 0, 8 + rand(6),   0, },
        { 0, 12 + rand(5),  2 + rand(2) },
        { 0, 12 + rand(6),  3 + rand(2) },
    })

    -- Abandoned mine
    set_sector_enemies("I13", {
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 },
    })

    set_sector_enemies("J11", {
        { 0, 10 + rand(6),  0, },
        { 0, 10 + rand(3),  3 + rand(2) },
        { 0, 11 + rand(5),  6 + rand(3) },
    })

    set_sector_enemies("J12", {
        { 0, 12 + rand(4),  0, },
        { 0, 10 + rand(4),  1 + rand(2) },
        { 0, 11 + rand(3),  3 + rand(2) },
    })

    -- Power Generator, Ground Level
    set_sector_enemies("J13", {
        { 0, 15 + rand(6),  1, },
        { 0, 16 + rand(5),  5 + rand(2) },
        { 0, 18 + rand(4),  6 + rand(3) },
    })

    -- Complex, Ground Level
    set_sector_enemies("K15", {
        { 0, 14 + rand(5),  0, },
        { 0, 18 + rand(3),  4 + rand(2) },
        { 0, 17 + rand(3),  8 + rand(2) },
    })
end

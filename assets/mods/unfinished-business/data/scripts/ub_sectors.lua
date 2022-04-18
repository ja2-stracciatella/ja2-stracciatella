RegisterListener('OnEnterSector', 'setup_ub_sectors')

function setup_ub_sectors(x, y, z)
    if z ~= 0 then return end

    local sectorID = GetCurrentSector()
    local sectorInfo = GetSectorInfo(sectorID)
    if (sectorInfo.uiFlags & SectorFlag.SF_HAS_ENTERED_TACTICAL) ~= 0 then
        -- not the first time here
        return
    end

    if sectorID == 'H9' then -- the guardpost
        -- Add some money to the location
        add_money(9026,  {15000, 10000, 7000})
    elseif sectorID == "I10" then -- the first sector of the town
        -- Add some money to the location
        add_money(11894, {8000, 4000, 3000})
        add_money(7906,  {12000, 6000, 5000})
    end
end

function add_money(grid_no, amounts_by_difficulty)
    local amt = amounts_by_difficulty[gGameOptions.ubDifficultyLevel]
    local money = CreateMoney(amt)
    PlaceItem(grid_no, money, Visibility.INVISIBLE)
end


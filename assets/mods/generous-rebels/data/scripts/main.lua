JA2Require("enums.lua")

--[[
Adds extra items in the initial sectors when we load it for the first time.
--]]
function BeforePrepareSector()

    local currentSector = GetCurrentSector()

    if currentSector == "A9" then
        local sectorInfo = GetSectorInfo(currentSector)
        if (sectorInfo.uiFlags & SectorFlags.SF_HAS_ENTERED_TACTICAL) == 0 then
            -- First time in A9
            log.debug("Adding money at 7255")
            local money = CreateMoney(10)
            PlaceItem(7255, money, Visibility.INVISIBLE)
        end
        
    elseif currentSector == "A10-1" then
        local sectorInfo = GetUndergroundSectorInfo(currentSector)
        if (sectorInfo.uiFlags & SectorFlags.SF_HAS_ENTERED_TACTICAL) == 0 then
            -- First time in A10-1
            local money = CreateMoney(1000)
            PlaceItem(11114, money, Visibility.INVISIBLE)
            
            local goggle = CreateItem(Items.SUNGOGGLE, 96)
            PlaceItem(13838, goggle, Visibility.VISIBLE)
        end

    end
end
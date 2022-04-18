RegisterListener("BeforeStructureDamaged", "handle_fan_damage")
RegisterListener("BeforePrepareSector", "init_power_plant")

MAP_ROW_J = 10


function init_power_plant()
    extra_game_state.test = false
end

function handle_fan_damage(x, y, z, grid_no, pCurrent, dist, skipDamage)
    if y == MAP_ROW_J and x == 13 and z == 0 and grid_no == 12421 then  -- SEC_J13
        power_plant_fan_destroyed()
    end
end

function power_plant_fan_destroyed()

end

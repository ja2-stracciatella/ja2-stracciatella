RegisterListener('OnEnterSector', 'scale_enemies')

function scale_enemies(x, y, z)
    -- Scales the difficulty of enemies
    local our_team = rank_soldiers_by_exp_level(ListSoldiersFromTeam(Team.OUR_TEAM))
    local enemy_team = rank_soldiers_by_exp_level(ListSoldiersFromTeam(Team.ENEMY_TEAM))

    -- no enemies
    if #enemy_team == 0 then
        return
    end

    -- edge cases
    if #enemy_team < 2 then enemy_team[2] = enemy_team[1] end
    if #our_team < 2 then our_team[2] = our_team[1] end

    if gGameOptions.ubDifficultyLevel == DifficultyLevel.DIF_LEVEL_EASY then
        local adjustment = our_team[2].bExpLevel - enemy_team[2].bExpLevel
        -- if the players exp level is less then enemies
        if adjustment < 0 then
            -- degrade all the enemies exp levels by difference (at most -2)
            scale_all_enemies(math.max(adjustment, -2))
        end
    elseif gGameOptions.ubDifficultyLevel == DifficultyLevel.DIF_LEVEL_MEDIUM then
        local adjustment = our_team[2].bExpLevel - enemy_team[1].bExpLevel
        if adjustment > 0 then
            -- Upgrade all the enemies exp levels by difference
            scale_all_enemies(adjustment);
        end
    elseif gGameOptions.ubDifficultyLevel == DifficultyLevel.DIF_LEVEL_HARD then
        local adjustment = our_team[1].bExpLevel - enemy_team[1].bExpLevel
        if adjustment > 0 then
            scale_all_enemies(adjustment)  -- TODO: or upgrade all the enemies exp levels by 1
        end
    end
end

function rank_soldiers_by_exp_level(soldiers)
    table.sort(soldiers, function (a, b) return a.bExpLevel < b.bExpLevel end)
    return soldiers
end

function scale_all_enemies(adjustment)
    log.info("scaling enemies by " .. adjustment)
    local enemies = ListSoldiersFromTeam(Team.ENEMY_TEAM)
    for _, s in ipairs(enemies) do
        local new_level = s.bExpLevel + adjustment
        new_level = math.max(1, math.min(10, new_level))  -- ensure valid value (1 - 10)
        if s.bExpLevel > 2 and new_level < 2 then
            -- if the enemy originally had a higher exp level AND will now go below set level
            new_level = 2
        end
        s.bExpLevel = new_level
    end
end

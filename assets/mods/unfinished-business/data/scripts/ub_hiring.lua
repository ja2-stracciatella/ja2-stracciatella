RegisterListener("OnMercHired", "handling_new_hiring")
RegisterListener("OnInitNewCampaign", "adjust_merc_profiles")
RegisterListener('OnStrategicEvent', 'hiring_contracts_updates')

function handling_new_hiring(s)
    log.info("new merc hired")
    if s.ubWhatKindOfMercAmI == MercTypes.MERC then
        -- M.E.R.C. : charge upfront
        local p = GetMercProfile(s.ubProfile)
        -- TODO: handle insufficient funds
        log.info("salary for " .. s.ubID .. " is " .. p.uiWeeklySalary .. ' ' .. p.sSalary)
        AddTransactionToPlayersBook(TransactionTypes.PAY_SPECK_FOR_MERC, s.ubID, GetWorldTotalMin(), -p.uiWeeklySalary)
        AddHistoryToPlayersLog(HistoryTypes.SETTLED_ACCOUNTS_AT_MERC, s.ubID, GetWorldTotalMin(), -1, -1)
    end

    update_soldier_contract(s)
end

function adjust_merc_profiles()
    -- adjust profiles for all hirable mercs, for the UB simplified economics
    for i = 0, 59, 1 do
        local p = GetMercProfile(i)
        if p.uiWeeklySalary > 0 then
            -- one salary, hired for entire mission; no other charges
            if i <= 40 then  -- free gear for A.I.M. soldiers
                p.usOptionalGearCost = 200  -- TODO: zero gear cost
                p.uiWeeklySalary = p.uiWeeklySalary - p.usOptionalGearCost
            end
            -- UB merc profiles only use weekly salary, for everything
            p.sSalary = p.uiWeeklySalary
            p.uiBiWeeklySalary = p.uiWeeklySalary
            p.bMedicalDeposit = 0
            p.sMedicalDepositAmount = 0
        end
    end
end

function hiring_contracts_updates(event, processed)
    if event.ubEventKind ==  EventTypes.EVENT_HOURLY_UPDATE then
        -- reset iMercMercContractLength to 0 for M.E.R.C.
        local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
        for _, s in ipairs(our_team) do
            update_soldier_contract(s)
        end
    end
end

function update_soldier_contract(s)
    if s.ubWhatKindOfMercAmI == MercTypes.AIM_MERC then
        -- A.I.M.
        s.iTotalContractLength = 999  -- TODO: "N/A" contract duration
        s.iEndofContractTime = 999 * 1440
    elseif s.ubWhatKindOfMercAmI == MercTypes.MERC then
        -- M.E.R.C.
        local p = GetMercProfile(s.ubProfile)
        p.iMercMercContractLength = 0
    end
end

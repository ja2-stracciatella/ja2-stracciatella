RegisterListener("OnCheckQuests", "check_quests")
RegisterListener("OnSoldierDeath", "handle_death")
RegisterListener("OnQuestEnded", "handle_end_quest")
--RegisterListener("OnHandleStrategicScreen", 'fix_laptop')
RegisterListener("OnItemPriced", "raul_price_item")
RegisterListener("OnItemTransacted", "raul_transact_item")
RegisterListener("OnStrategicEvent", "after_buying_transmitter")

function check_quests(day, is_handled)
    -- QUEST 23 : Destroy missiles
    -- The game always starts with destroy missiles quest, so turn it on if it hasn't already started
    if gubQuest[UB_Quests.QUEST_DESTROY_MISSLES] == QuestStatuses.QUESTNOTSTARTED then
        StartQuest(UB_Quests.QUEST_DESTROY_MISSLES, "")
    end

    -- skip base game's quests
    is_handled.val = TRUE
end

function handle_death(soldier)
    local sectorID = GetCurrentSector()
    if sectorID == "I10" and soldier.ubBodyType == SoldierBodyTypes.BLOODCAT then
        --Betty's Bloodcat quest
        local sectorInfo = GetSectorInfo(sectorID)
        log.debug("bloodcat died, remaining: " .. sectorInfo.bBloodCats)
        if sectorInfo.bBloodCats <= 1 then
            log.info("This is the last Bloodcat in I10")
            SetFactTrue(UB_Facts.FACT_PLAYER_KILLED_ALL_BETTYS_BLOODCATS)

            log.info("refreshing shop inventory")
            DailyCheckOnItemQuantities() -- Instantly have betties missing items show up

            --Now have a merc say the killed bloodcat quote (except TEX)
            local mercs = get_mercs_with_ub_quotes()
            for _, m in ipairs(mercs) do
                if m.ubProfile ~= UB_MercProfiles.TEX then
                    TacticalCharacterDialogue(m, 107) --QUOTE_RENEW_REFUSAL_DUE_TO_LACK_OF_FUNDS
                    break
                end
            end
        end
    end
end


function handle_end_quest(quest_id, sector_x, sector_y, should_update_history)
    if quest_id == UB_Quests.QUEST_FIX_LAPTOP then
        -- Set the fact that AIM and MERC should start selling
        extra_game_state.have_aim_merc_offered_items = true

        -- Remember that we should send email in the next sector
        extra_game_state.send_email_10_next_sector = true

        local send_date = GetWorldTotalMin()
        AddEmailMessage(UB_Emails.PILOTMISSING, UB_Emails.PILOTMISSING_LENGTH, send_date, UB_EmailSenders.MAIL_ENRICO, FALSE, 0, 0)
        AddEmailMessage(UB_Emails.MAKECONTACT,  UB_Emails.MAKECONTACT_LENGTH,  send_date, UB_EmailSenders.MAIL_ENRICO, FALSE, 0, 0)

        --Merc and Aim emails
        AddEmailMessage(UB_Emails.AIM_PROMOTION_1,  UB_Emails.AIM_PROMOTION_1_LENGTH,  send_date, UB_EmailSenders.AIM_SITE,        FALSE, 0, 0)
        AddEmailMessage(UB_Emails.MERC_PROMOTION_1, UB_Emails.MERC_PROMOTION_1_LENGTH, send_date, UB_EmailSenders.SPECK_FROM_MERC, FALSE, 0, 0)
        AddEmailMessage(UB_Emails.AIM_PROMOTION_2,  UB_Emails.AIM_PROMOTION_2_LENGTH,  send_date, UB_EmailSenders.AIM_SITE,        FALSE, 0, 0)
    end
end

function after_buying_transmitter(event, processed)
    if event.ubEventKind ~= UB_EventTypes.EVENT_FIX_LAPTOP then
        return
    end

    -- Tell the user we are going to attach the transmitter to the laptop
    DoBasicMessageBox("Attaching the transmitter to your laptop computer.")

    -- find the transmitter from soldiers' inventory
    local our_team = ListSoldiersFromTeam(Team.OUR_TEAM)
    for _, s in ipairs(our_team) do
        -- try to consume(remove) the transmitter item
        if RemoveObjectFromSoldierProfile(s.ubProfile, UB_Items.LAPTOP_TRANSMITTER) == TRUE then
            break
        end
    end
    -- have a new merc say a quote
    local mercs = get_mercs_with_ub_quotes()
    if #mercs > 0 then
        TacticalCharacterDialogue(mercs[1], 79) --QUOTE_SPARE2
    end
    -- end the laptop quest
    EndQuest(UB_Quests.QUEST_FIX_LAPTOP, "")
    processed.val = TRUE
end


-- Raul gives he quest during pricing
function raul_price_item(dealer_id, item_id, is_selling, unit_price)
    if dealer_id ~= UB_Dealers.RAUL then
        return
    end

    if extra_game_state.raul_quotes_said == nil then
        extra_game_state.raul_quotes_said = {}
    end

    local quest_status = gubQuest[UB_Quests.QUEST_FIND_ANTIQUE_MUSKET_FOR_RAUL + 1]
    -- if the item is the antique musket
    if item_id == UB_Weapons.HAND_CANNON and is_selling == FALSE then
        -- if the "Find anitque musket" quest is active
        if quest_status == QuestStatuses.QUESTINPROGRESS then
        -- the price should be.... nothin ( reward for the quest )
        unit_price.val = 1;
        end

        -- if the player hasnt said the quote before
        if not extra_game_state.raul_quotes_said["48"] then
            StartShopKeeperTalking(48) -- SK_QUOTES_NPC_SPECIFIC_48 );
            extra_game_state.raul_quotes_said["48"] = true
            --gfCanSayMakeTransactionQuote = FALSE;
        end

        if extra_game_state.raul_quotes_said["49"] then
            -- if the player hasnt said it before
            if not extra_game_state.raul_quotes_said["50"] then
                StartShopKeeperTalking(50) -- SK_QUOTES_NPC_SPECIFIC_50 );
                extra_game_state.raul_quotes_said["50"] = true
            end
            -- the price should be
            unit_price.val = 100;
        end
    end

    -- if the item is the barrat
    if item_id == UB_Weapons.BARRETT then
        -- and if the player found the antique musket for Raul
        if extra_game_state.barrett_is_half_price then
            -- the barrett is half off
            unit_price.val = unit_price.val // 2
        end
    end
end

function raul_transact_item(dealer_id, item_id, is_sold)
    if dealer_id ~= UB_Dealers.RAUL then
        return
    end

    local quest_status = gubQuest[UB_Quests.QUEST_FIND_ANTIQUE_MUSKET_FOR_RAUL + 1]

    -- if this is the first time that the player purchased somehting form raul
    if is_sold and not extra_game_state.has_raul_sold_something then
        -- if Raul still has the BARRETT
        if not extra_game_state.raul_has_sold_barrett then
        -- if the musket is not yet started
            if quest_status == QuestStatuses.QUESTNOTSTARTED then
                -- Raul Start the quest and have Raul say somehitng
                StartShopKeeperTalking(46) --SK_QUOTES_NPC_SPECIFIC_46
                StartShopKeeperTalking(47) --SK_QUOTES_NPC_SPECIFIC_47

                StartQuest(UB_Quests.QUEST_FIND_ANTIQUE_MUSKET_FOR_RAUL, "")
            end
        end
        extra_game_state.has_raul_sold_something = true
    end

    -- if the item is the antique musket
    if item_id == UB_Weapons.HAND_CANNON and is_sold ~= TRUE then
        --Only do this if the player knew about the cannon offer
        if quest_status == QuestStatuses.QUESTINPROGRESS then
            -- Have raul say a quote
            StartShopKeeperTalking(51) -- SK_QUOTES_NPC_SPECIFIC_51
            -- Trigger Rauls action #52
            TriggerNPCRecord(UB_MercProfiles.RAUL, 52)
            -- Remember to discount the Barrett
            extra_game_state.barrett_is_half_price = true
        end
        -- Mark the quest done
        EndQuest(UB_Quests.QUEST_FIND_ANTIQUE_MUSKET_FOR_RAUL, "")
        -- HAND_CANNON and CLIP_CANNON_BALL should not appear in Raul's inventory
        guarantee_dealer_item_count(UB_Dealers.RAUL, UB_Weapons.HAND_CANNON, 0)
        guarantee_dealer_item_count(UB_Dealers.RAUL, UB_Items.CLIP_CANNON_BALL, 0) -- TODO: somehow this does not work
    end
end

    --[[
    TODO: SK quote 49, when player pulls back the offer to sell musket
    StartShopKeeperTalking( SK_QUOTES_NPC_SPECIFIC_49 ) when HAND_CANNON is picked up ("BeginSkiItemPointer") from the player offer area
    --]]

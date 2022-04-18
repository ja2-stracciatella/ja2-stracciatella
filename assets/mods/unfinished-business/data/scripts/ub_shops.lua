RegisterListener("OnDealerInventoryUpdated", "adjust_dealer_inventory")
RegisterListener("OnInitNewCampaign", "init_dealers")
RegisterListener("OnNPCDoAction", "handle_shopkeeper_npc_actions")
RegisterListener("OnItemTransacted", "handle_item_purchased")
RegisterListener("OnItemPriced", "handle_item_special_prices")
RegisterListener("OnSoldierDeath", "on_raul_killed")

--enum values
FIRSTAIDKIT = 201
MEDICKIT = 202
COMPOUND18 = 216
PORNOS = 257
CERAMIC_PLATES = 183
LAME_BOY = 314
HAND_GRENADE = 135


SKI_RAUL_MINIMUM_AMOUNT_TO_SET_FACT = 100  -- TODO: FACT_PLAYER_BOUGHT_SOMETHING_FROM_RAUL
SKI_BETTY_MINIMUM_AMOUNT_TO_SET_FACT = 10  -- TODO: FACT_PLAYER_PAID_BETTY_MORE_THEN_X_FOR_ITEM

function init_dealers()
    extra_game_state.raul_has_sold_barrett = false
    extra_game_state.amounts_spend_at_shop = {}
    if extra_game_state.is_tex_in_game then
        log.info("Tex movies for sale")
        guarantee_dealer_item_count(UB_Dealers.BETTY, UB_Items.TEX_MOVIE_ATTACK_CLYDESDALES, 1)
        guarantee_dealer_item_count(UB_Dealers.BETTY, UB_Items.TEX_MOVIE_WILD_EAST, 1)
        guarantee_dealer_item_count(UB_Dealers.BETTY, UB_Items.TEX_MOVIE_HAVE_HONDA, 1)
    end
end

function adjust_dealer_inventory()
    if not extra_game_state.raul_has_sold_barrett then
        --if Raul hasnt yet sold the barret
        log.info("BARRETT for sale")
        guarantee_dealer_item_count(UB_Dealers.RAUL, UB_Weapons.BARRETT, 1)
    else
        guarantee_dealer_item_count(UB_Dealers.RAUL, UB_Weapons.BARRETT, 0)
    end

    --if the player hasnt done the "killed the annoying bloodcats" quest for betty,
    if gubQuest[UB_Quests.QUEST_FIX_LAPTOP] ~= QuestStatuses.QUESTDONE then
        -- Guarantee 1 laptop transmitter to be at betty's
        log.info("Betty has the laptop transmitter")
        guarantee_dealer_item_count(UB_Dealers.BETTY, UB_Items.LAPTOP_TRANSMITTER, 1)
    else
        --if the quest is done, dont have 1
        guarantee_dealer_item_count(UB_Dealers.BETTY, UB_Items.LAPTOP_TRANSMITTER, 0)
    end

    GuaranteeAtLeastXItemsOfIndex(UB_Dealers.BETTY, PORNOS, 1)
    --if the player hasnt done the "killed the annoying bloodcats" quest for betty,
    if gubQuest[UB_Quests.QUEST_GET_RID_BLOODCATS_AT_BETTYS] ~= QUESTDONE then
        --make sure she doesnt sell these items just yet
        guarantee_dealer_item_count(UB_Dealers.BETTY, FIRSTAIDKIT, 0)
        guarantee_dealer_item_count(UB_Dealers.BETTY, MEDICKIT, 0)
        guarantee_dealer_item_count(UB_Dealers.BETTY, COMPOUND18, 0)
        guarantee_dealer_item_count(UB_Dealers.BETTY, CERAMIC_PLATES, 0)
        guarantee_dealer_item_count(UB_Dealers.BETTY, LAME_BOY, 0)
    end
end

function guarantee_dealer_item_count(dealer_id, item_id, num_items)
    if num_items > 0 then
        log.info("requesting at least " .. num_items .. " of item #" .. item_id)
        GuaranteeAtLeastXItemsOfIndex(dealer_id, item_id, num_items)
    end

    -- now remove if we have too many
    local dealer_inv = GetDealerInventory(dealer_id)
    local actual_num_items = dealer_inv[item_id + 1].ubTotalItems   -- in Lua array indexes are off-by-1
    if actual_num_items > num_items then
        local num_to_remove = actual_num_items - num_items
        log.info("removing " .. num_to_remove .. " extra of item #" .. item_id)
        RemoveRandomItemFromDealerInventory(dealer_id, item_id, num_to_remove)
    end
end

function handle_shopkeeper_npc_actions(target_npc, action_code, quote_num, is_handled)
    -- special NPC actions by shopkeepers
    if action_code == UB_NPCActions.NPC_ACTION_RAUL_BLOWS_HIMSELF_UP then
        raul_blow_himself_up()
        is_handled.val = TRUE
    elseif action_code == UB_NPCActions.NPC_ACTION_HAVE_DEALER_OPEN_BUY_SELL_SCREEN then
        DeleteTalkingMenu()
        -- Enter the shopkeeper interface
        EnterShopKeeperInterfaceScreen(target_npc)
        is_handled.val = TRUE
    elseif action_code == UB_NPCActions.NPC_ACTION_LEAVING_NPC_TALK_MENU then
        log.info("NPC_ACTION_LEAVING_NPC_TALK_MENU")
        SetFactTrue(UB_Facts.FACT_MERC_SAY_QUOTE_WHEN_TALK_MENU_CLOSES) -- TODO: to be checked and handled in DeleteTalkingMenu
        AddStrategicEvent(UB_EventTypes.EVENT_MERC_SAY_QUOTE_WHEN_TALK_MENU_CLOSES, GetWorldTotalSeconds(), target_npc)
        is_handled.val = TRUE
    end
end

function handle_item_special_prices(dealer_id, item_id, is_selling, unit_price)
    if dealer_id == UB_Dealers.BETTY then
        -- fix price of Tex movies
        if item_id == UB_Items.TEX_MOVIE_ATTACK_CLYDESDALES or
                item_id == UB_Items.TEX_MOVIE_HAVE_HONDA or
                item_id == UB_Items.TEX_MOVIE_WILD_EAST then
            -- Make the item price top be what is listed in the item table ($20)
            unit_price.val = 20
        end

        ---- she not accepting weapons or ammo
        --if item_id == 22 --MINI14
        --    then
        --    StartShopKeeperTalking(40) --SK_QUOTES_NPC_SPECIFIC_40
        --end
    end
end

function handle_item_purchased(dealer_id, item_id, is_sold)
    -- NPC speech and quests
    if dealer_id == UB_Dealers.BETTY and is_sold == TRUE then
        if item_id == PORNOS then
            -- if the item is the porno mag
            StartShopKeeperTalking(UB_Quotes.SK_QUOTES_WHEN_PLAYER_BUYS_PORNO)
        elseif item_id == UB_Items.TEX_MOVIE_ATTACK_CLYDESDALES or
               item_id == UB_Items.TEX_MOVIE_WILD_EAST or
               item_id == UB_Items.TEX_MOVIE_HAVE_HONDA then
            -- if the item is Tex's videos
            log.info("bought Tex video")
            SetFactTrue(UB_Facts.FACT_PLAYER_BOUGHT_A_TEX_VIDEO_FROM_BETTY)
        elseif item_id == UB_Items.LAPTOP_TRANSMITTER then
            -- if the item is the laptop transmitter
            -- fix laptop as soon as the shop dialog is closed
            AddStrategicEvent(UB_EventTypes.EVENT_FIX_LAPTOP, GetWorldTotalSeconds(), 0)
        end
    end

    -- keep track of money amount purchases
    --if is_sold == TRUE then
       --TODO:  FACT_PLAYER_BOUGHT_SOMETHING_FROM_RAUL and FACT_PLAYER_PAID_BETTY_MORE_THEN_X_FOR_ITEM
    --end
end

function raul_blow_himself_up()
    -- Find Raul
    local raul = FindSoldierByProfileID(UB_MercProfiles.RAUL)

    -- if he exists
    if raul ~= nil then
        -- First lower his life, artificially
        raul.bLife = 5
        -- blow himself up with, hmmm, lets say TNT.  :)
        IgniteExplosion(raul, 0, raul.sGridNo, HAND_GRENADE, raul.bLevel)
        extra_game_state.raul_blown_himself_up = true
    end
end

function on_raul_killed(soldier)
    if soldier.ubProfile ~= UB_MercProfiles.RAUL then
        return
    end

    --if the person was Raul, and we are to say the blown up quotes
    if extra_game_state.raul_blown_himself_up then
        -- don't drop items (if blowing himself up)
        RemoveAllUnburiedItems(soldier.sInitialGridNo, soldier.bLevel)

        -- Get some random Soldier ID's of the valid mercs
        local mercs = get_mercs_with_ub_quotes()
        if #mercs > 0 then
            -- Say the "he blew himself up quote"
            TacticalCharacterDialogue(mercs[1], 108) --QUOTE_GREETING
            -- Say the "darn he took the inventory with him"
            TacticalCharacterDialogue(mercs[#mercs], 109) --QUOTE_SMALL_TALK)
        end
    else
        -- Booby trap all dropped items (if killed)
        booby_trap_objects(soldier.sInitialGridNo)
    end
end

function booby_trap_objects(grid_no)
    log.error("TODO: booby trap all dropped item of Raul")
--[[
void BoobyTrapDealersItem( OBJECTTYPE *pTempObject )
{
        INT8 bTrapLevel = 4 + Random( 6 );
        pTempObject->bTrap = bTrapLevel;

        //Make money real tough to untrap
        if( pTempObject->usItem == MONEY )
        {
                pTempObject->bTrap = 10;
        }
}
--]]
end
--[[
TODO:
--RAUL:
   - blow himself up    // HandleNPCTeamMemberDeath
   - hand-cannon quest
   - Ensure items (1x Barrett)
--BETTY
   - Bloodcat quest
   - Ensure items (laptop transmitter, TEX videos)
   - Special dialogues (porn, doesn't buy ammo/guns)
-- general
   - no cash restock
--]]

--[[
                     //if the dealer is BETTY
                     if( gbSelectedArmsDealerID == ARMS_DEALER_BETTY &&
                             ( Item[ PlayersOfferArea[ bSlotID ].sItemIndex ].usItemClass == IC_AMMO ||
                                 Item[ PlayersOfferArea[ bSlotID ].sItemIndex ].usItemClass == IC_GUN
                             )
                         )
                     {
                         // she accepts items, but not this one
                         sQuoteNum = SK_QUOTES_NPC_SPECIFIC_40;
                     }
                     else
--]]
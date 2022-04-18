require("enums_manual.lua")
require("enums.lua")
require("ub_enums.lua")
require("lib_serpent.lua")

require("ub_hiring.lua")
require("ub_init_attack.lua")
require("ub_laptop.lua")
require("ub_merc_quotes.lua")
require("ub_progress.lua")
require("ub_quests.lua")
require("ub_rpc.lua")
require("ub_rpc_biggens.lua")
require("ub_sectors.lua")
require("ub_sectors_mine.lua")
require("ub_strategic_ai.lua")
require("ub_start_seq.lua")
require("ub_tactical.lua")
require("ub_shops.lua")

RegisterListener("OnInitNewCampaign", "on_new_campaign")
RegisterListener("OnStrategicEvent", "handle_events")
RegisterListener("BeforeGameSaved", "save_game")
RegisterListener("OnGameLoaded", "load_game")

math.randomseed(os.time())
MOD_NAME = 'unfinished-business'
extra_game_state = {}
log.info("unfinished-business mod is enabled")

function on_new_campaign()
    -- ensure STOGIE and GASTON are available
    local p = GetMercProfile(UB_MercProfiles.GASTON)
    p.bMercStatus           = MercStatuses.MERC_OK
    p.uiDayBecomesAvailable = 0

    p = GetMercProfile(UB_MercProfiles.STOGIE)
    p.bMercStatus           = MercStatuses.MERC_OK
    p.uiDayBecomesAvailable = 0
end

function load_game()
    local dehydrated = GetGameStates(MOD_NAME)
    extra_game_state = {}
    for k, v in pairs(dehydrated) do
        log.info("Loaded: " .. k .. "=" .. v)
        local ok, res = serpent.load(v)
        if ok then extra_game_state[k] = res end
    end
end

function save_game()
    local dehydrated = {}
    for k, v in pairs(extra_game_state) do
        dehydrated[k] = serpent.line(v, {comment = false})
        log.info("Saving: " .. k .. "=" .. dehydrated[k])
    end
    PutGameStates(MOD_NAME, dehydrated)
end

function handle_events(event, processed)
    if event.ubEventKind == EventTypes.EVENT_MEANWHILE then
        log.info('intercepted and cancelled MEANWHILE cutscene')
        processed.val = TRUE
    end
end
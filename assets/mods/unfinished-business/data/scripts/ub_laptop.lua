RegisterListener('OnAddEmail', 'handle_add_email')
RegisterListener('OnInitNewCampaign', 'start_new_campaign')
RegisterListener('OnGoToWebPage', 'goto_webpage')

-- enums
LAPTOP_MODE_BROKEN_LINK = 42
MERC_ACCOUNT_VALID = 4
MERC_SITE_THIRD_OR_MORE_VISITS = 3

function start_new_campaign()
    -- add UB emails
    local send_date = 1501  -- so we can tell them apart from base game emails

    AddEmailMessage(UB_Emails.STARTGAME,       UB_Emails.STARTGAME_LENGTH,
                    send_date, UB_EmailSenders.MAIL_ENRICO,       FALSE, 0, 0)
    AddEmailMessage(UB_Emails.AIMDISCOUNT,     UB_Emails.AIMDISCOUNT_LENGTH,
                    send_date, UB_EmailSenders.AIM_SITE,          FALSE, 0, 0)  -- add the initial email from AIM
    AddEmailMessage(UB_Emails.IMP_EMAIL_INTRO, UB_Emails.IMP_EMAIL_INTRO_LENGTH,
                    send_date, UB_EmailSenders.CHAR_PROFILE_SITE, FALSE, 0, 0)  -- add the imp site email

    -- set M.E.R.C. to be available immediately
    SetBookMark(Bookmarks.AIM)
    SetBookMark(Bookmarks.MERC)

    -- pre-open accounts on M.E.R.C.
    LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_VALID
    LaptopSaveInfo.ubPlayerBeenToMercSiteStatus = MERC_SITE_THIRD_OR_MORE_VISITS
end

function handle_add_email(offset, len, date, sender, already_read, data1, data2, is_cancelled)
    -- intercept and cancel all base game initial emails
    if date == 1500 then  -- 1500 is the base game start-of-game email date
        -- pre-read email at game init
        is_cancelled.val = TRUE
    end
    --TODO: no new emails if Laptop is broken
end

function goto_webpage(page_id)
    if gubQuest[UB_Quests.QUEST_FIX_LAPTOP] == QuestStatuses.QUESTINPROGRESS and
            extra_game_state.ever_been_in_tactical
    then
        -- TODO: go to BROKEN_LINK if squad has landed and FIX_LAPTOP quest is in progress
        SetLaptopModes(LAPTOP_MODE_BROKEN_LINK, LAPTOP_MODE_BROKEN_LINK)
    end
end
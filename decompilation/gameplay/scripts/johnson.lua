-- Shared Johnson interaction loop. The active location profile supplies the
-- speaker, portrait, menu copy, and ROM-derived dialogue pages; this module
-- owns the reusable conversation flow.
local Johnson = {}
local StoryFacts = require("story_facts")
local E = require("enums")

local function present_location_intro(profile, entry_seen_before)
    if profile == nil then
        return false
    end
    if profile.entry_seen_fact ~= nil then
        local event_ids = entry_seen_before and
            profile.repeat_entry_events or profile.entry_events
        for _, event_id in ipairs(event_ids or {}) do
            game.ui.say(event_id)
        end
        return not entry_seen_before
    end
    if profile.first_visit_fact == nil or
       game.state[profile.first_visit_fact] ~= false then
        return false
    end
    if profile.first_visit_karma_award ~= nil then
        local amount = profile.first_visit_karma_award
        assert(type(amount) == "number" and amount >= 1 and amount <= 0xFF and
               amount % 1 == 0,
            "Johnson first-visit Karma award must be an integer amount")
        assert(game.services.call(
            "party", "award_karma_" .. tostring(amount)) ==
                "applied",
            "Johnson first-visit Karma award could not be applied")
    end
    for _, event_id in ipairs(profile.first_visit_events) do
        game.ui.say(event_id)
    end
    game.state[profile.first_visit_fact] = true
    return true
end

local function present_work_offer()
    local profile = game.modules.dialogue.johnson_interaction_profiles[
        game.location.text_profile]
    assert(profile ~= nil and profile.run_contact_id ~= nil,
        "this Johnson location has no run-contact profile")
    if profile.work_opening_event ~= nil then
        game.ui.say(profile.work_opening_event)
    end
    return game.runs.registry.present_offer_for_contact(
        profile.run_contact_id)
end

local function question_choices()
    local choices = { "johnson" }
    local vigore_lead_available = game.state.vigore_lead_available == true
    local vigore_alternate_response_available =
        not vigore_lead_available and
        game.state[E.JohnsonFact.VIGORE_ALTERNATE_RESPONSE_AVAILABLE] == true
    if vigore_lead_available or vigore_alternate_response_available then
        choices[#choices + 1] = "vigore"
    end
    if game.state.errascoe_question_available then
        choices[#choices + 1] = "errascoe"
    end
    return choices
end

local function another_question_menu(menu_id)
    return game.ui.menu(menu_id,
        { "another_question", "different_subject" })
end

local function answer_questions()
    while true do
        local question = game.ui.menu("question_selection",
            question_choices())

        if question == "johnson" then
            game.ui.say("johnson-definition")
            if another_question_menu("question_followup") ==
               "different_subject" then
                return
            end
        elseif question == "vigore" then
            local response_menu
            if game.state.vigore_lead_available == true then
                -- ROM action 0xE7 sets the pending Rat's Nest search bit and
                -- returns the follow-up prompt at 0x105270.
                game.state.vigore_search_pending = true
                response_menu = "vigore_response"
            elseif game.state[E.JohnsonFact.VIGORE_ALTERNATE_RESPONSE_AVAILABLE] == true then
                -- When FFFC1E bit 26 is clear, FFFC27 bit 3 selects the
                -- alternate response page at 0x105244, then the question
                -- menu at 0x105258. It does not arm the Rat's Nest search.
                game.ui.say("vigore-alternate-response")
                response_menu = "vigore_alternate_followup"
            else
                return
            end
            if another_question_menu(response_menu) == "different_subject" then
                return
            end
        elseif question == "errascoe" then
            if another_question_menu("errascoe_response") ==
               "different_subject" then
                return
            end
        else
            return
        end
    end
end

-- Finder fees (nuyen) charged through the generic building.charge_<n>
-- host service. ROM 0x2218E charges 1,000 for Gunderson's decker and
-- Julius's Quinton contacts; ROM cmd 0x50 at 0x221BA charges 1,500 for
-- Mortimer's Alesandro Hobbs contact.
local FINDER_FEES = { decker = 1000, quinton = 1000, alesandro = 1500 }

-- Returns "purchased", "insufficient" or nil (host could not charge).
local function purchase_contact(contact)
    local result = game.services.call(
        "building", "charge_" .. tostring(FINDER_FEES[contact]))
    if result == "paid" then
        return "purchased"
    end
    return result
end

local function present_decker_offer()
    game.ui.say("contacts-introduction")
    if game.ui.menu("decker_offer", { "accept", "back" }) ~= "accept" then
        return "back"
    end
    return purchase_contact("decker")
end

local function present_information(root_menu)
    local information_menu = "information"
    local after_contact_purchase = false
    if root_menu == "opening" then
        game.ui.say("information-opening")
    else
        information_menu = "information_after_return"
    end
    while true do
        local topic_menu = after_contact_purchase
            and "contacts_purchased" or information_menu
        local topic = game.ui.menu(topic_menu,
            { "questions", "contacts", "back" })
        if topic == "questions" then
            answer_questions()
            return "back"
        elseif topic == "contacts" then
            local purchase = present_decker_offer()
            if purchase == "purchased" then
                -- The ROM's 0x4D action returns its paid descriptor after the
                -- 1,000-nuyen debit. Its FFFC2C bit 4 is already set by the
                -- New Game seed, so it is not a distinct acquisition flag.
                after_contact_purchase = true
            elseif purchase == "insufficient" then
                local fallback = game.ui.menu("contacts_insufficient",
                    { "information", "work", "leave" })
                if fallback == "information" then
                    information_menu = "information_after_return"
                    after_contact_purchase = false
                elseif fallback == "work" or fallback == "leave" then
                    return fallback
                else
                    return "back"
                end
            elseif purchase == "back" then
                return "back"
            else
                return "back"
            end
        else
            return "back"
        end
    end
end

local function present_julius_questions()
    while true do
        local choices = {}
        if game.state[E.JohnsonFact.CALEB_KNOWN] then
            choices[#choices + 1] = "caleb"
        end
        if game.state[E.JohnsonFact.MORTIMER_KNOWN] then
            choices[#choices + 1] = "mortimer"
        end
        if #choices == 0 then
            game.ui.say("question_default")
            return
        end

        local topic = game.ui.menu("question_selection", choices)
        local answer_key
        if topic == "caleb" then
            answer_key = "julius-strouther.question.caleb"
        elseif topic == "mortimer" then
            answer_key = "julius-strouther.question.mortimer"
        else
            return
        end
        if game.ui.prompt_text_key(
                "question_answer_followup", answer_key,
                { "another_question", "different_subject" }) ==
           "different_subject" then
            return
        end
    end
end

local function present_julius_information()
    while true do
        local topic = game.ui.menu("information",
            { "questions", "contacts", "back" })
        if topic == "questions" then
            present_julius_questions()
            return "back"
        elseif topic == "contacts" then
            game.ui.say("contacts_opening")
            if game.ui.menu("contacts_offer", { "accept", "back" }) ~=
               "accept" then
                return "back"
            end
            local purchase = purchase_contact("quinton")
            if purchase == "purchased" then
                -- ROM command 0x2218E charges 1,000 nuyen and sets FFFC2C
                -- bit 0, the Captain Quinton contact row.
                game.state.vid_phone_contact_0 = true
                return "contacts_purchased"
            elseif purchase == "insufficient" then
                local fallback = game.ui.menu("contacts_insufficient",
                    { "information", "work", "leave" })
                if fallback == "information" then
                    -- The insufficient-funds descriptor returns to the
                    -- information category without reopening the root.
                elseif fallback == "work" or fallback == "leave" then
                    return fallback
                else
                    return "back"
                end
            else
                error("Julius contact purchase returned an unknown result")
            end
        else
            return "back"
        end
    end
end

local function present_mortimer_questions()
    while true do
        local choices = {}
        if game.state[E.JohnsonFact.JULIUS_KNOWN] then
            choices[#choices + 1] = "julius"
        end
        if game.state.michael_alias_learned and
           not game.state.mako_defeated then
            choices[#choices + 1] = "mako"
        end
        if game.state.owlfeather_asked and
           not game.state.aragorn_found then
            choices[#choices + 1] = "aragorn"
        end
        if #choices == 0 then
            game.ui.say("question_default")
            return "categories"
        end

        local topic = game.ui.menu("question_selection", choices)
        local answer_key
        if topic == "julius" then
            answer_key = "big-rhino.information.question.julius"
        elseif topic == "mako" then
            answer_key = "big-rhino.information.question.mako"
            -- ROM action 0x8E at 0x23222 records the Club Penumbra lead.
            game.state[E.StoryFact.MAKO_CLUB_LEAD] = true
        elseif topic == "aragorn" then
            answer_key = "big-rhino.information.question.aragorn"
            -- ROM action 0x8F at 0x2323A clears bit 11 and sets bit 12.
            game.state.field_mode_flag_bit_11 = false
            game.state.field_mode_flag_bit_12 = true
        else
            return "categories"
        end

        if game.ui.prompt_text_key(
                "question_answer_followup", answer_key,
                { "another_question", "different_subject" }) ==
           "different_subject" then
            return "categories"
        end
    end
end

local function present_mortimer_information()
    game.ui.say("information_opening")
    while true do
        local topic = game.ui.menu("information",
            { "questions", "contacts", "back" })
        if topic == "questions" then
            present_mortimer_questions()
        elseif topic == "contacts" then
            if game.ui.menu("contacts_offer", { "accept", "back" }) ==
               "accept" then
                local purchase = purchase_contact("alesandro")
                if purchase == "purchased" then
                    -- ROM command 0x50 charges 1,500 and sets FFFC2C bit 1.
                    game.state.vid_phone_contact_1 = true
                    return "contacts_purchased"
                elseif purchase == "insufficient" then
                    return "contacts_insufficient"
                end
                error("Mortimer contact purchase returned an unknown result")
            end
        else
            return "back"
        end
    end
end

-- The ROM's FFFC30 longword is represented by two host aliases so field
-- mode and Notebook readers stay in step, as in tarislar_city_inn.lua.
local function write_caleb_fffc30(bit, value)
    game.state["field_mode_flag_bit_" .. bit] = value
    game.state["notebook_tip_flag_bit_" .. bit] = value
end

local function present_caleb_different_subject()
    game.ui.say("information_different_subject")
    return game.ui.menu("information_return",
        { "information", "work", "leave" })
end

local function caleb_information_topic_choices()
    local choices = {}
    if StoryFacts.is_set(E.StoryFact.RED_BUFFALO_WOMAN) == true and
       StoryFacts.is_set(E.StoryFact.ILENE_MET) == false then
        choices[#choices + 1] = "ilene"
    end
    if StoryFacts.is_set(E.StoryFact.SHAMAN_TASKS_DONE) == true or
       StoryFacts.is_set(E.StoryFact.MAKO_DEFEATED) == true then
        choices[#choices + 1] = "thon"
    end
    if StoryFacts.is_set(E.StoryFact.RENRAKU_MAP_OVERLAY) == true or
       StoryFacts.is_set(E.StoryFact.GILLIAN_INITIATION) == true or
       StoryFacts.is_set(E.StoryFact.GARDEN_APTS_ENTERED) == true then
        choices[#choices + 1] = "ito_ogami"
    end
    return choices
end

local function present_caleb_information_questions()
    while true do
        local choices = caleb_information_topic_choices()
        if #choices == 0 then
            -- ROM 0x2325C selects text 0x106229 and <MORE> to 0x1056DC
            -- when no gates append a topic (`53 6F 72 72 79`; record bytes
            -- at 0x1056DC: `00 10 88 53 ... 00 10 57 0C`).
            game.ui.say("information_questions_empty")
            game.ui.menu("information_questions_empty", { "more" })
            return present_caleb_different_subject()
        end

        -- ROM 0x2327E/0x23286 adds Ilene from FFFC1E bits 18/19
        -- (`08 01 00 12`, `08 01 00 13`); 0x232A8/0x232B0 adds Thon
        -- from bits 9/11 (`08 01 00 09`, `08 01 00 0B`); and
        -- 0x232D2/0x232DA/0x232E2 adds Ogami from bits 12/20/21
        -- (`08 01 00 0C`, `08 01 00 14`, `08 01 00 15`).
        local topic = game.ui.menu("information_questions", choices)
        local followup_menu
        if topic == "ilene" then
            -- ROM command 0x91 at 0x23310 sets FFFC34 bit 0
            -- (`08 C1 00 00`) and returns page 0x105850.
            game.state["field_mode_effect_flag_bit_" ..
                E.FieldModeEffectFlagBit.ILENE_INFORMATION_HEARD] = true
            game.ui.say("ilene_information")
            followup_menu = "ilene_followup"
        elseif topic == "thon" then
            -- The topic text at 0x104142 points to page 0x105868; its
            -- record targets command 0x90 or page 0x1056DC.
            game.ui.say("thon_information")
            followup_menu = "thon_followup"
        elseif topic == "ito_ogami" then
            -- ROM command 0x92 at 0x23328 sets FFFC30 bit 25
            -- (`08 C1 00 19`) and returns page 0x105838.
            write_caleb_fffc30(
                E.FieldModeFlagBit.RENRAKU_HEADHUNTER_TOLD, true)
            game.ui.say("ito_ogami_information")
            followup_menu = "ito_ogami_followup"
        else
            error("Caleb information menu returned an unknown topic")
        end

        local followup = game.ui.menu(followup_menu,
            { "another_question", "different_subject" })
        if followup ~= "another_question" then
            return present_caleb_different_subject()
        end
    end
end

local function present_caleb_information_offer()
    while true do
        -- Record 0x10570C offers the 0x90 question menu, the contacts pages,
        -- or page 0x1056DC (`00 10 88 F8 ... 00 00 00 90 ... 00 10 56 DC`).
        game.ui.say("information_contacts_offer")
        local choice = game.ui.menu("information_offer",
            { "questions", "contacts", "different_subject" })
        if choice == "questions" then
            local result = present_caleb_information_questions()
            if result ~= "information" then return result end
        elseif choice == "contacts" then
            -- 0x105808's <MORE> target is 0x10581C (`00 10 58 1C`);
            -- the next record carries commands 0x51/0x52 and the 0x1056DC
            -- return choice (`00 00 00 51`, `00 00 00 52`, `00 10 56 DC`).
            game.ui.say("contacts_introduction")
            game.ui.say("contacts_kipp")
            local contact_choice = game.ui.menu("contacts_choice",
                { "chillicut", "kipp_david", "different_subject" })
            if contact_choice == "different_subject" then
                local result = present_caleb_different_subject()
                if result ~= "information" then return result end
            else
                local contact_index
                if contact_choice == "chillicut" then
                    contact_index = E.VidPhoneContact.CHILLICUT
                elseif contact_choice == "kipp_david" then
                    contact_index = E.VidPhoneContact.KIPP_DAVID
                else
                    error("Caleb contact menu returned an unknown contact")
                end

                local funds = type(game.nuyen) == "function" and
                    game.nuyen() or nil
                if type(funds) ~= "number" then
                    return "wallet_unavailable"
                end
                local result_event
                if funds < E.JohnsonContactFee.CALEB_INFORMATION then
                    -- Both command handlers choose 0x1057EC on insufficient
                    -- funds (`00 10 57 EC`) and do not write FFFC2C.
                    result_event = "contacts_purchase_insufficient"
                else
                    -- Commands 0x51/0x52 load 2,000 (`00 00 07 D0` at
                    -- 0x221E8/0x22214), subtract through 0x21A4E, then set
                    -- FFFC2C bits 11/3 (`08 C1 00 0B`, `08 C1 00 03`).
                    game.set_nuyen(
                        funds - E.JohnsonContactFee.CALEB_INFORMATION)
                    game.state["vid_phone_contact_" .. contact_index] = true
                    result_event = "contacts_purchase_success"
                end
                game.ui.say(result_event)
                local result = game.ui.menu(result_event,
                    { "information", "work", "leave" })
                if result ~= "information" then return result end
            end
        else
            local result = present_caleb_different_subject()
            if result ~= "information" then return result end
        end
    end
end

local function present_caleb_michael_mission()
    -- Only the first-meeting choice record 0x1056F8 ("Tell me about
    -- Michael's last Shadowrun.", `00 10 86 C6 00 10 3C FC 00 00 00 00
    -- 00 00 00 00 00 00 00 1C`) reaches command 0x1C; the repeat roots
    -- 0x1056C0/0x1056DC/0x1057D0 send information straight to 0x10570C.
    -- ROM command 0x1C at 0x21940 clears FFFC30 bit 5 and sets bit 23
    -- (`08 81 00 05`, `08 C1 00 17`), then selects 0x105780. The common
    -- 0x2196C path sets FFFC1E bit 13 from selector 13 (`01 C1`).
    write_caleb_fffc30(E.FieldModeFlagBit.BIT_5, false)
    write_caleb_fffc30(E.FieldModeFlagBit.BIT_23, true)
    StoryFacts.set(E.StoryFact.MICHAEL_MISSION_TOLD, true)
    game.ui.say("michael_information_1")
    game.ui.say("michael_information_2")
    game.ui.say("michael_information_3")
    game.ui.say("michael_information_4")
    game.ui.say("michael_wrapup")
    local choice = game.ui.menu("michael_wrapup",
        { "information", "work", "leave" })
    if choice == "information" then
        return present_caleb_information_offer()
    end
    return choice
end

function Johnson.interaction()
    local profile = game.modules.dialogue.johnson_interaction_profiles[
        game.location.text_profile]
    local entry_seen_before = profile ~= nil and
        profile.entry_seen_fact ~= nil and
        game.state[profile.entry_seen_fact] == true
    if profile ~= nil then
        for _, fact in ipairs(profile.access_facts or {}) do
            assert(type(fact) == "string" and fact ~= "",
                "Johnson access fact must name a story-state field")
            game.state[fact] = true
        end
    end
    if profile ~= nil and profile.post_departure_story_fact ~= nil and
       StoryFacts.is_set(profile.post_departure_story_fact) == true then
        -- Scene 14 checks FFFC1E bit 16 before its normal first-visit and
        -- Johnson-return paths. It clears context 3 with 0x1E68C, then shows
        -- the blank-profile booth-vacated page with no choices.
        local contact_id = profile.post_departure_contact_id or
            profile.run_contact_id
        game.runs.registry.clear_active_offer_for_contact(contact_id)
        game.ui.say(profile.post_departure_event,
            profile.post_departure_speaker,
            profile.post_departure_text_profile)
        return "booth_vacated"
    end
    local returned_run = profile ~= nil and profile.run_contact_id ~= nil
        and game.runs.registry.take_pending_johnson_return() or nil
    local first_visit = false
    if returned_run ~= nil then
        assert(returned_run.contact_id == profile.run_contact_id,
            "pending run return belongs to another Johnson context")
        local run = game.runs[returned_run.run_id]
        assert(type(run) == "table" and
               type(run.present_completion) == "function",
            "completed run has no Johnson-return presentation")
        run.present_completion(returned_run)
    else
        first_visit = present_location_intro(profile, entry_seen_before)
    end
    local root_menu = returned_run ~= nil
        and (profile.run_return_root or "opening_after_run_return") or
        (first_visit and (profile.first_visit_root or "opening") or
        (entry_seen_before and (profile.repeat_root or "opening") or
         "opening"))
    -- Caleb's first-meeting record 0x1056F8 offers only the command 0x1C
    -- choice; every later root uses the shared information/work/leave set.
    local root_choices = first_visit and profile ~= nil and
        profile.first_visit_root_choices or { "information", "work", "leave" }
    while true do
        local choice = game.ui.menu(root_menu, root_choices)
        root_choices = { "information", "work", "leave" }

        if choice == "leave" then
            if profile ~= nil and profile.leave_event ~= nil then
                game.ui.say(profile.leave_event)
            end
            return choice
        elseif choice == "information" or choice == "michael_last_run" then
            if profile == nil then
                return "information_unresolved"
            end
            local information_result
            if profile.information_flow == "gunderson" then
                information_result = present_information(root_menu)
            elseif profile.information_flow == "julius" then
                information_result = present_julius_information()
            elseif profile.information_flow == "big_rhino" then
                information_result = present_mortimer_information()
            elseif profile.information_flow == "caleb_brightmore" then
                if choice == "michael_last_run" then
                    information_result = present_caleb_michael_mission()
                else
                    information_result = present_caleb_information_offer()
                end
            else
                return "information_unresolved"
            end
            if information_result == "leave" then
                if profile.leave_event ~= nil then
                    game.ui.say(profile.leave_event)
                end
                return information_result
            elseif information_result == "work" then
                if present_work_offer() == "accepted" then
                    return "accepted"
                end
            elseif information_result == "contacts_purchased" or
                   information_result == "contacts_insufficient" then
                root_menu = information_result
            else
                -- ROM main-dialogue descriptors 0x10545C and 0x10514C
                -- resume the shared root after an information branch.
                root_menu = profile.information_flow == "caleb_brightmore"
                    and "opening" or "opening_after_information"
            end
        elseif choice == "work" then
            if present_work_offer() == "accepted" then
                return "accepted"
            end
        end
    end
end

return Johnson

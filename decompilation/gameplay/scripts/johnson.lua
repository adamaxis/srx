-- Shared Johnson interaction loop. The active location profile supplies the
-- speaker, portrait, menu copy, and ROM-derived dialogue pages; this module
-- owns the reusable conversation flow.
local Johnson = {}
local StoryFlags = require("story_flags")

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
    if profile.first_visit_runner_progress_delta ~= nil then
        local delta = profile.first_visit_runner_progress_delta
        assert(type(delta) == "number" and delta >= 0 and delta <= 0xFF and
               delta % 1 == 0,
            "Johnson first-visit progress delta must be a byte")
        assert(game.services.call(
            "runner", "adjust_source_byte_triplet_" .. tostring(delta)) ==
                "applied",
            "Johnson first-visit source progress could not be applied")
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
    assert(profile ~= nil and profile.run_offer_context ~= nil,
        "this Johnson location has no ROM-derived offer context")
    if profile.work_opening_event ~= nil then
        game.ui.say(profile.work_opening_event)
    end
    return game.runs.registry.present_offer_for_context(
        profile.run_offer_context)
end

local function question_choices()
    local choices = { "johnson" }
    local vigore_lead_available = game.state.vigore_lead_available == true
    local vigore_alternate_response_available =
        not vigore_lead_available and
        game.state.mr_johnsons_flag_bit_3 == true
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
            elseif game.state.mr_johnsons_flag_bit_3 == true then
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
        if game.state.mr_johnsons_flag_bit_4 then
            choices[#choices + 1] = "caleb"
        end
        if game.state.mr_johnsons_flag_bit_2 then
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
        if game.state.mr_johnsons_flag_bit_1 then
            choices[#choices + 1] = "julius"
        end
        if game.state.story_flag_bit_10 and
           not game.state.story_flag_bit_11 then
            choices[#choices + 1] = "mako"
        end
        if game.state.story_flag_bit_2 and
           not game.state.story_flag_bit_3 then
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
            -- ROM action 0x8E at 0x23222 records the Mako lead in FFFC30.
            game.state.field_mode_flag_bit_21 = true
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
    if profile ~= nil and profile.post_departure_story_flag ~= nil and
       StoryFlags.is_set(profile.post_departure_story_flag) == true then
        -- Scene 14 checks FFFC1E bit 16 before its normal first-visit and
        -- Johnson-return paths. It clears context 3 with 0x1E68C, then shows
        -- the blank-profile booth-vacated page with no choices.
        local context = profile.post_departure_context or
            profile.run_offer_context
        game.runs.registry.clear_active_offer_for_context(context)
        game.ui.say(profile.post_departure_event,
            profile.post_departure_speaker,
            profile.post_departure_text_profile)
        return "booth_vacated"
    end
    local returned_run = profile ~= nil and profile.run_offer_context ~= nil
        and game.runs.registry.take_pending_johnson_return() or nil
    local first_visit = false
    if returned_run ~= nil then
        assert(returned_run.context == profile.run_offer_context,
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
    while true do
        local choice = game.ui.menu(root_menu,
            { "information", "work", "leave" })

        if choice == "leave" then
            if profile ~= nil and profile.leave_event ~= nil then
                game.ui.say(profile.leave_event)
            end
            return choice
        elseif choice == "information" then
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
                root_menu = "opening_after_information"
            end
        elseif choice == "work" then
            if present_work_offer() == "accepted" then
                return "accepted"
            end
        end
    end
end

return Johnson

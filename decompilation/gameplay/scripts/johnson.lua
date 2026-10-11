-- Shared Johnson interaction loop. The active location profile supplies the
-- speaker, portrait, menu copy, and ROM-derived dialogue pages; this module
-- owns the reusable conversation flow.
local Johnson = {}

local function question_choices()
    local choices = { "johnson" }
    if game.state.vigore_lead_available then
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
            -- ROM action 0xE7 sets the pending Rat's Nest search bit and
            -- returns the follow-up prompt at 0x105270.
            game.state.vigore_search_pending = true
            if another_question_menu("vigore_response") ==
               "different_subject" then
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

local function present_decker_offer()
    game.ui.say("contacts-introduction")
    if game.ui.menu("decker_offer", { "accept", "back" }) ~= "accept" then
        return "back"
    end
    return game.services.call("johnson", "purchase_decker_contact")
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

function Johnson.interaction()
    local root_menu = "opening"
    while true do
        local choice = game.ui.menu(root_menu,
            { "information", "work", "leave" })

        if choice == "leave" then
            return choice
        elseif choice == "information" then
            local information_result = present_information(root_menu)
            if information_result == "leave" then
                return information_result
            elseif information_result == "work" then
                game.ui.say("work-opening")
            end
            -- ROM descriptor 0x10514C supplies a different title after an
            -- information branch, while retaining the same A/B/C labels.
            root_menu = "opening_after_information"
        elseif choice == "work" then
            game.ui.say("work-opening")
        end
    end
end

return Johnson

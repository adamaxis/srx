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

local function present_information(root_menu)
    local information_menu = "information"
    if root_menu == "opening" then
        game.ui.say("information-opening")
    else
        information_menu = "information_after_return"
    end
    local topic = game.ui.menu(information_menu,
        { "questions", "contacts", "back" })

    if topic == "questions" then
        answer_questions()
    elseif topic == "contacts" then
        game.ui.say("contacts-introduction")
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
            present_information(root_menu)
            -- ROM descriptor 0x10514C supplies a different title after an
            -- information branch, while retaining the same A/B/C labels.
            root_menu = "opening_after_information"
        elseif choice == "work" then
            game.ui.say("work-opening")
        end
    end
end

return Johnson

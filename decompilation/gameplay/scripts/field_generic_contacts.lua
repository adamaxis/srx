-- Lua presentation for row selections made by the generic ROM 0x21174 path.
local E = require("enums")
local World = require("world_content")
local imported = require("data.field_contact_dialogue_rows")

local Contacts = {}

local runner_information_page_order = { "opening", "continuation", "final" }
local runner_conversation_choices = { "information", "hire", "leave" }
local runner_story_followup_choices = { "story", "questions", "different_subject" }
local runner_hire_choices = { "short_term", "permanent" }

local function present_runner(location, text, runner_id, runner)
    local text_profile = "runner." .. runner_id
    local speaker_id = runner_id

    local function say(event_id)
        game.ui.say(event_id, speaker_id, text_profile)
    end

    local function present_biography()
        local terminal_page = runner.information.final ~= nil and
            "final" or "continuation"
        for _, page_id in ipairs(runner_information_page_order) do
            if runner.information[page_id] ~= nil then
                if page_id == terminal_page then
                    return game.ui.prompt_text_key(
                        "story_followup", runner.information[page_id],
                        runner_story_followup_choices, speaker_id,
                        text_profile)
                end
                say("information." .. page_id)
            end
        end
    end

    local function present_questions()
        local subject = runner.subject
        if type(subject) ~= "table" or type(subject.option_order) ~= "table" then
            return
        end
        local selected = game.ui.menu("subjects", subject.option_order)
        local pages = subject.responses[selected]
        if type(pages) ~= "table" then
            return
        end
        local content_prefix = "runner." .. runner_id .. "."
        for page_index, event_id in ipairs(pages) do
            if type(event_id) ~= "string" or
               event_id:sub(1, #content_prefix) ~= content_prefix then
                error("runner subject page has no matching text profile")
            end
            if page_index == #pages then
                return game.ui.prompt_text_key(
                    "story_followup", event_id,
                    runner_story_followup_choices, speaker_id, text_profile)
            end
            say(event_id:sub(#content_prefix + 1))
        end
    end

    local function information()
        if game.services.call(
                "recruited_runner", "mark_recruited_runner_information") ~=
           "applied" then
            error("runner information state is unavailable")
        end

        local choice = present_biography()
        while true do
            if choice == "story" then
                choice = present_biography()
            elseif choice == "questions" then
                choice = present_questions()
            else
                return
            end
        end
    end

    local function hire()
        while true do
            local fees = game.services.call(
                "recruited_runner", "prepare_recruited_runner_offer")
            if fees == "roster_full" then
                say("hire.roster_full")
                return false
            end
            local short_fee, permanent_fee
            if type(fees) == "string" then
                short_fee, permanent_fee = fees:match("^fees_(%d+)_(%d+)$")
            end
            if short_fee == nil or permanent_fee == nil then
                error("runner service prices are unavailable")
            end
            local offer = string.format(
                "I charge $%s for a|single run.||But if you can cough up|$%s my services are|at your disposal.||Of course, either way I|get my percentages.",
                short_fee, permanent_fee)
            local choice = game.ui.prompt_text(
                "hire_offer", offer, runner_hire_choices)
            if choice == nil then
                return false
            end
            local actor_id = location.actor_instance_id
            local actor = type(game.actors) == "table" and
                game.actors[actor_id] or nil
            local lifecycle = type(actor) == "table" and
                type(actor.actor_facts) == "table" and
                actor.actor_facts.lifecycle or nil
            if type(lifecycle) ~= "table" or lifecycle.active ~= true then
                error("runner live contact state is unavailable")
            end
            local action = choice == "short_term" and
                "hire_recruited_runner_short_term" or
                choice == "permanent" and
                "hire_recruited_runner_permanent" or nil
            if action == nil then
                return false
            end
            local result = game.services.call("recruited_runner", action)
            if result == "hired" then
                if not World.deactivate_hired_contact_actor(
                        location.scene_id, actor_id) then
                    error("hired runner actor lifecycle could not be updated")
                end
                say("hire.success")
                return true
            elseif result == "insufficient_funds" then
                say("hire.insufficient")
                return false
            elseif result == "roster_full" then
                say("hire.roster_full")
                return false
            end
            error("selected runner hire offer could not be completed")
        end
    end

    while true do
        local choice = game.ui.prompt_text(
            "opening", text, runner_conversation_choices)
        if choice == "information" then
            information()
        elseif choice == "hire" then
            if hire() then
                return "presented"
            end
        else
            return "presented"
        end
    end
end

function Contacts.present()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= E.GenericFieldContact.LOCATION_ID or
       type(location.event_id) ~= "string" then
        return "skip"
    end
    local response = imported.responses[location.event_id]
    local text = type(response) == "table" and response.opening_text or nil
    if type(text) ~= "string" or text == "" then
        return "skip"
    end

    local Dialogue = location.speaker_id ~= nil and
        require("dialogue") or nil
    local runner = Dialogue ~= nil and
        Dialogue.runner_conversation(location.speaker_id) or nil
    if runner ~= nil and runner.subject ~= nil then
        return present_runner(location, text, location.speaker_id, runner)
    end

    game.ui.say_text(text, false)
    return "presented"
end

return Contacts

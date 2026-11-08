-- Lua presentation for row selections made by the generic ROM 0x21174 path.
local E = require("enums")
local World = require("world_content")
local imported = require("data.field_contact_dialogue_rows")

local Contacts = {}

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
    if location.speaker_id == "ricky" then
        local function say_ricky(event_id)
            game.ui.say(event_id, "ricky", "runner.ricky")
        end
        local function information()
            if game.services.call(
                    "recruited_runner", "mark_ricky_information") ~=
               "applied" then
                error("Ricky's information state is unavailable")
            end
            say_ricky("information.opening")
            say_ricky("information.continuation")
            local subject = game.ui.menu("subjects", {
                "shaman_magic", "local_area", "johnson",
            })
            local pages = {
                shaman_magic = {
                    "subject.shaman_magic.page_0",
                    "subject.shaman_magic.page_1",
                    "subject.shaman_magic.page_2",
                },
                local_area = {
                    "subject.local_area.page_0",
                    "subject.local_area.page_1",
                },
                johnson = { "subject.johnson_gunderson.page_0" },
            }
            for _, event_id in ipairs(pages[subject] or {}) do
                say_ricky(event_id)
            end
        end
        local function hire()
            while true do
                local fees = game.services.call(
                    "recruited_runner", "prepare_ricky_offer")
                if fees == "roster_full" then
                    say_ricky("hire.roster_full")
                    return false
                end
                local short_fee, permanent_fee
                if type(fees) == "string" then
                    short_fee, permanent_fee = fees:match(
                        "^fees_(%d+)_(%d+)$")
                end
                if short_fee == nil or permanent_fee == nil then
                    error("Ricky's service prices are unavailable")
                end
                local offer = string.format(
                    "I charge $%s for a|single run.||But if you can cough up|$%s my services are|at your disposal.||Of course, either way I|get my percentages.",
                    short_fee, permanent_fee)
                local choice = game.ui.prompt_text("hire_offer", offer, {
                    "short_term", "permanent",
                })
                if choice == nil then
                    return false
                end
                local actor_id = location.actor_instance_id
                local actor = type(game.actors) == "table" and
                    game.actors[actor_id] or nil
                local lifecycle = type(actor) == "table" and
                    type(actor.actor_facts) == "table" and
                    actor.actor_facts.lifecycle or nil
                if type(lifecycle) ~= "table" or
                   lifecycle.active ~= true then
                    error("Ricky's live contact state is unavailable")
                end
                local action = choice == "short_term" and
                    "hire_ricky_short_term" or
                    choice == "permanent" and
                    "hire_ricky_permanent" or nil
                if action == nil then
                    return false
                end
                local result = game.services.call(
                    "recruited_runner", action)
                if result == "hired" then
                    if not World.deactivate_hired_contact_actor(
                            location.scene_id, actor_id) then
                        error("Ricky's hired actor lifecycle could not be updated")
                    end
                    say_ricky("hire.success")
                    return true
                elseif result == "insufficient_funds" then
                    say_ricky("hire.insufficient")
                    return false
                elseif result == "roster_full" then
                    say_ricky("hire.roster_full")
                    return false
                end
                error("Ricky's selected hire offer could not be completed")
            end
        end

        while true do
            local choice = game.ui.prompt_text("opening", text, {
                "information", "hire", "leave",
            })
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
    game.ui.say_text(text, false)
    return "presented"
end

return Contacts

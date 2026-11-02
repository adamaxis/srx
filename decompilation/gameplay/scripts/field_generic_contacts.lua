-- Lua presentation for row selections made by the generic ROM 0x21174 path.
local E = require("enums")
local dialogue_text = require("data.field_contact_dialogue_text")

local Contacts = {}

function Contacts.present()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= E.GenericFieldContact.LOCATION_ID or
       type(location.event_id) ~= "string" then
        return "skip"
    end
    local text = dialogue_text[location.event_id]
    if type(text) ~= "string" or text == "" then
        return "skip"
    end
    game.ui.say_text(text, false)
    return "presented"
end

return Contacts

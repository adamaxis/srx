-- Chillicut's first/repeat dialogue and option flow are authored in JSON.
-- ROM commands 0x79-0x7B each charge the same effects as Henry at different
-- prices; only the confirmed transaction writes the corresponding game fact.
local Chillicut = {}

local function charge(amount, effect)
    local result = game.services.call("building", "charge_" .. amount)
    if result == "insufficient" then return "insufficient" end
    if result ~= "paid" then return "service_unavailable" end

    effect()
    return "completed"
end

function Chillicut.actions(contact)
    assert(type(contact) == "table" and
           contact.id == "chillicut" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "call",
        "Chillicut graph needs a validated call-history binding")

    return {
        ["vid_phone.mark_called"] = function()
            if game.state[contact.call_history_fact] ~= false then
                return "call_history_unavailable"
            end
            game.state[contact.call_history_fact] = true
            return true
        end,
        ["vid_phone.chillicut.delete_record"] = function()
            return charge(5000, function()
                game.state.lone_star_criminal_record_active = false
            end)
        end,
        ["vid_phone.chillicut.hollywood_access"] = function()
            return charge(3000, function()
                game.state.vid_phone_story_flag_bit_27 = true
            end)
        end,
        ["vid_phone.chillicut.weapon_permit"] = function()
            return charge(10000, function()
                game.state.group_item_flag_bit_11 = true
            end)
        end,
    }
end

return Chillicut

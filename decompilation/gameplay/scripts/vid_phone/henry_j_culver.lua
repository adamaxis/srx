-- Henry's choices and outcomes live in the JSON conversation graph. This
-- module contains only the three transactional effects recovered from ROM.
local HenryJCulver = {}

local function charge(amount, effect)
    local result = game.services.call("building", "charge_" .. amount)
    if result == "insufficient" then return "insufficient" end
    if result ~= "paid" then return "service_unavailable" end

    effect()
    return "completed"
end

function HenryJCulver.actions(contact)
    assert(type(contact) == "table" and
           contact.id == "henry_j_culver" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "call",
        "Henry graph needs a validated call-history binding")

    return {
        ["vid_phone.mark_called"] = function()
            if game.state[contact.call_history_fact] ~= false then
                return "call_history_unavailable"
            end
            game.state[contact.call_history_fact] = true
            return true
        end,
        ["vid_phone.henry.delete_record"] = function()
            return charge(2500, function()
                game.state.lone_star_criminal_record_active = false
            end)
        end,
        ["vid_phone.henry.hollywood_access"] = function()
            return charge(1500, function()
                game.state.vid_phone_story_flag_bit_27 = true
            end)
        end,
        ["vid_phone.henry.weapon_permit"] = function()
            return charge(8000, function()
                game.state.group_item_flag_bit_11 = true
            end)
        end,
    }
end

return HenryJCulver

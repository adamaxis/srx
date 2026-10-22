-- The Faradouchi and Agira call graphs share the same source-timed history
-- and affiliation write. Their JSON graphs own the choices and branch policy.
local VidPhoneAffiliations = {}

local contact_effects = {
    mr_faradouchi = {
        action_id = "vid_phone.accept_mafia_affiliation",
        affiliation_fact = "vid_phone_affiliation_mafia",
    },
    agira_tetsumi = {
        action_id = "vid_phone.accept_yakuza_affiliation",
        affiliation_fact = "vid_phone_affiliation_yakuza",
    },
}

function VidPhoneAffiliations.actions(contact)
    local effect = type(contact) == "table" and contact_effects[contact.id]
    assert(effect ~= nil and type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "graph_action" and
           contact.service_graph_scope == "call",
        "affiliation graph needs a source-action call-history binding")

    return {
        [effect.action_id] = function()
            -- The source sets the call bit only on the affirmative choice.
            -- The contact-list gate already proved that it was clear. Both
            -- native facts must be readable before either write is attempted.
            if game.state[contact.call_history_fact] ~= false or
               game.state[effect.affiliation_fact] == nil then
                return "state_unavailable"
            end
            game.state[contact.call_history_fact] = true
            game.state[effect.affiliation_fact] = true
            return true
        end,
    }
end

return VidPhoneAffiliations

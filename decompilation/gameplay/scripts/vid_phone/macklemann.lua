-- Macklemann-specific host effects for his JSON-owned service graph.
-- Contact record 0x10BFAC maps command 0x76 to handler 0x22BF6:
-- 5,000 nuyen sets FFFC2C bit 28. It does not set contact bit 5.
local Macklemann = {}

function Macklemann.actions(contact)
    assert(type(contact) == "table" and contact.id == "macklemann" and
           type(contact.call_history_fact) == "string" and
           contact.call_history_policy == "before_first_call" and
           contact.service_graph_scope == "services",
        "Macklemann graph needs a validated contact service binding")

    return {
        ["vid_phone.issue_hollywood_badge"] = function()
            local result = game.services.call(
                "vid_phone", "issue_hollywood_badge")
            if result == "hired" or result == "insufficient" or
               result == "unavailable" then
                return result
            end
            return "service_unavailable"
        end,
    }
end

return Macklemann

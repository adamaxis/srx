-- Content selection for the shared field-mode text prompt. The native field
-- context is read-only; this module chooses the narrative record and optional
-- speaker art while the generic presenter owns layout, timing, and input.
local FieldMode = {}

local function prompt(text_key)
    return {
        status = "matched",
        text_key = text_key,
        portrait_alias = "field.mode.entry.ares.sprite",
    }
end

function FieldMode.mode_text_content()
    local field = game.field
    if field.mode ~= 2 or field.event_code ~= 11 then
        return {
            status = "unmatched",
            text_key = "none",
            portrait_alias = "none",
        }
    end

    if field.alternate_result > 0 then
        return prompt("field.mode.ares.package_payment")
    end
    if field.alternate_result < 0 then
        return prompt("field.mode.ares.package_handoff")
    end
    return prompt("field.mode.ares.store_intro")
end

return FieldMode

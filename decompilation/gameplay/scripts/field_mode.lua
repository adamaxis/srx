-- Content selection for the shared field-mode text prompt. The native field
-- context is read-only; this module chooses the narrative record and optional
-- speaker art while the generic presenter owns layout, timing, and input.
local E = require("enums")
local FieldMode = {}

local function prompt(text_key, portrait_alias)
    return {
        status = "matched",
        text_key = text_key,
        portrait_alias = portrait_alias or "field.mode.entry.ares.sprite",
    }
end

local renraku_arcology_store_ids = {
    wire_masters = true,
    merlins_lore = true,
    microtronics = true,
    weapons_world = true,
}

local function renraku_store_prompt(field)
    if renraku_arcology_store_ids[field.service_id] ~= true then
        return nil
    end
    local store_id = field.service_id

    if field.prompt_variant == "store_closed" then
        return prompt("shop.store_closed", "none")
    end

    local text_variant = "store_intro"
    if field.package_run_status == E.PackageRunStatus.PAYMENT_DUE then
        text_variant = "package_payment"
    elseif field.package_run_status == E.PackageRunStatus.HANDOFF_DUE then
        text_variant = "package_handoff"
    end
    local portrait_alias = store_id == "weapons_world"
        and "field.mode.entry.ares.sprite"
        or "field.mode.entry.renraku.arcology.sprite"
    return prompt("field.mode." .. store_id .. "." .. text_variant,
                  portrait_alias)
end

function FieldMode.mode_text_content()
    local field = game.field
    local renraku_store = renraku_store_prompt(field)
    if renraku_store ~= nil then
        return renraku_store
    end

    if field.prompt_id == E.FieldModePrompt.LITTLE_CHIBA_STORE then
        if field.package_run_status == E.PackageRunStatus.PAYMENT_DUE then
            return prompt("field.mode.little_chiba.package_payment",
                          "field.mode.entry.little.chiba.sprite")
        end
        if field.package_run_status == E.PackageRunStatus.HANDOFF_DUE then
            return prompt("field.mode.little_chiba.package_handoff",
                          "field.mode.entry.little.chiba.sprite")
        end
        return prompt("field.mode.little_chiba.description",
                      "field.mode.entry.little.chiba.sprite")
    end

    if field.prompt_id ~= E.FieldModePrompt.ARES_EMPORIUM then
        return {
            status = "unmatched",
            text_key = "none",
            portrait_alias = "none",
        }
    end

    if field.package_run_status == E.PackageRunStatus.PAYMENT_DUE then
        return prompt("field.mode.ares.package_payment")
    end
    if field.package_run_status == E.PackageRunStatus.HANDOFF_DUE then
        return prompt("field.mode.ares.package_handoff")
    end
    return prompt("field.mode.ares.store_intro")
end

return FieldMode

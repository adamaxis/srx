-- Content selection for the shared field-mode text prompt. The native field
-- context is read-only; this module chooses the narrative record and optional
-- speaker art while the generic presenter owns layout, timing, and input.
local E = require("enums")
local StoryFacts = require("story_facts")
local FieldMode = {}

local function prompt(text_key, portrait_alias)
    return {
        status = "matched",
        text_key = text_key,
        portrait_alias = portrait_alias or "field.mode.entry.ares.sprite",
    }
end

-- ROM finalizer 0x1FD80 branches that end with D6 = -1 show their text and
-- cancel the event-table route instead of continuing to its destination.
local function cancelling_prompt(text_key, portrait_alias)
    local content = prompt(text_key, portrait_alias)
    content.route_after = "cancel"
    return content
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
    if field.prompt_id == E.FieldModePrompt.MEDICINE_LODGE_FIRST_VISIT then
        -- ROM finalizer 0x1FDBE handles every Scene-7/event-0 entry; each
        -- branch ends at 0x1FE7A..0x1FE80 with D6 = -1, so the lodge text
        -- always cancels the event-table route.
        local turned_away = E.StoryFact.MEDICINE_LODGE_TURNED_AWAY
        if StoryFacts.is_set(turned_away) ~= true then
            -- ROM 0x1FDCA..0x1FDF4 sets FFFC1E bit 24 before selecting text
            -- 0x11B714 on the first Scene-7/event-0 exit.
            StoryFacts.set(turned_away)
            return cancelling_prompt("field.mode.medicine_lodge.first_visit", "none")
        end
        if StoryFacts.is_set(E.StoryFact.HOSPITAL_VISITED) ~= true then
            -- ROM 0x1FDFE..0x1FE0C: bit 24 set, bit 25 clear selects 0x11B7E4
            -- (page 0x11C0EA).
            return cancelling_prompt("field.mode.medicine_lodge.stark_time_short", "none")
        end
        -- ROM 0x1FE10..0x1FE16: bit 25 set calls 0x59BCC, which clears the
        -- downed/dead conditions and refills physical and stun for all three
        -- live-record slots, then selects 0x11B7EC (page 0x11C14C).
        assert(type(game.party) == "function" and
               type(game.commit_party) == "function",
               "medicine lodge heal needs the party service")
        local party = game.party()
        for slot = 0, 2 do
            local runner = party[slot]
            if runner then
                runner.runner_facts.lifecycle.dead = false
                runner.runner_facts.lifecycle.downed = false
                runner.physical = 10
                runner.stun = 10
            end
        end
        game.commit_party(party)
        return cancelling_prompt("field.mode.medicine_lodge.stark_saved_heal", "none")
    end

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

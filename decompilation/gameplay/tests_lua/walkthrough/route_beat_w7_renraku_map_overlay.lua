local E = require("enums")

-- Scene-40 safe records 89-93 are event 0x1A at 0x1DE1D0..0x1DE1F8.
-- ROM trigger bounds are 512 units wide; start 64 units inside each edge.
local kProjectedCellApproachOffset = 64
local kSafeInputFrames = 120
local kDialogueInputLimit = 20
local kSafeCells = {
    { x = 2304, y = 768, record = 89 },
    { x = 12032, y = 11008, record = 90 },
    { x = 14080, y = 7936, record = 91 },
    { x = 11520, y = 3840, record = 92 },
    { x = 5376, y = 8448, record = 93 },
}
local kApproaches = {
    { dx = -kProjectedCellApproachOffset, dy = 0, direction = "RIGHT" },
    { dx = kProjectedCellApproachOffset, dy = 0, direction = "LEFT" },
    { dx = 0, dy = kProjectedCellApproachOffset, direction = "UP" },
    { dx = 0, dy = -kProjectedCellApproachOffset, direction = "DOWN" },
}

local story = E.StoryFact
local item = E.GroupItemId.MAP_OVERLAY

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function collect_map_overlay(host)
    local attempts = {}
    for _, cell in ipairs(kSafeCells) do
        for _, approach in ipairs(kApproaches) do
            local placed = host.place_within_scene(
                E.FieldSafe.RENRAKU_SCENE,
                cell.x + approach.dx, cell.y + approach.dy)
            if placed then
                -- Safe records publish only when crossed with real pad input.
                local before = host.snapshot()
                local _, movement_error = host.hold_until_modal(
                    approach.direction, kSafeInputFrames)
                assert(not movement_error, movement_error or
                    "safe approach input failed")
                local after = host.snapshot()
                attempts[#attempts + 1] = string.format(
                    "record %d %s from %d,%d to %d,%d event=%d FFDF64=%04X",
                    cell.record, approach.direction, before.x, before.y,
                    after.x, after.y, after.event,
                    after.terrain_collision_word)
                if host.snapshot().story_facts[story.RENRAKU_MAP_OVERLAY] == true then
                    return cell.record
                end
            end
        end
    end
    error("Renraku map overlay did not trigger at Scene-40 safe records 89-93: " ..
        table.concat(attempts, "; "))
end

return {
    steps = {
        {
            id = "prepare_rom_ito_victory_for_renraku_safe",
            milestone = 12,
            expected = {
                scene = E.FieldSafe.RENRAKU_SCENE,
                facts = {
                    [story.ITO_FIGHT_WON] = true,
                    [story.RENRAKU_MAP_OVERLAY] = false,
                },
                flags = { story = { set = { story.ITO_FIGHT_WON } } },
            },
            run = function(host)
                -- ROM 0x54286 sets FFFC1E bit 16 on Ito's victory; safe
                -- handler 0x540D6 requires that result and the map fact clear.
                local seeded, seed_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.ITO_FIGHT_WON, true)
                assert(seeded, seed_error or
                    "ROM Ito-victory prerequisite setup failed")
                assert(host.snapshot().story_facts[story.RENRAKU_MAP_OVERLAY] == false,
                    "Renraku map overlay must be clear before the safe beat")
                -- ROM 0xEE86 converts the FFE185th 0xDA tile to 0x9A only
                -- with FFDF40=2, FFE184/FFE187 (or FFFBED=4), matching
                -- FFE12C/FFF0C8, and FFE180 clear. Model the index-five,
                -- ordinal-zero resource selection so record 89 is the live
                -- projected marker; no reward/FFE178 state is seeded.
                local mode_set, mode_error =
                    host.set_field_interaction_mode_for_setup(
                    E.FieldInteractionMode.TERRAIN_MARKER_SWAP)
                assert(mode_set, mode_error or
                    "ROM terrain-marker mode setup failed")
                local effect_set, effect_error =
                    host.set_world_effect_active_for_setup(
                    false, E.FieldWorldEffectIndex.RENRAKU_SAFE_MARKER)
                assert(effect_set, effect_error or
                    "ROM matching-effect-index setup failed")
                local marker_set, marker_error =
                    host.set_effect_terrain_marker_context_for_setup(
                        true, false, 0)
                assert(marker_set, marker_error or
                    "ROM first-marker context setup failed")
                return "seeded Ito victory and ROM 0xEE86 marker preconditions; map overlay clear"
            end,
        },
        {
            id = "take_renraku_map_overlay_from_safe",
            milestone = 12,
            expected = {
                scene = E.FieldSafe.RENRAKU_SCENE,
                facts = {
                    [story.ITO_FIGHT_WON] = true,
                    [story.RENRAKU_MAP_OVERLAY] = true,
                },
                flags = {
                    story = { set = { story.RENRAKU_MAP_OVERLAY } },
                    effect = {
                        set = { E.FieldModeEffectFlagBit.RENRAKU_MAP_OVERLAY_GRANTED },
                        clear = {
                            E.FieldModeEffectFlagBit.BIT_8,
                            E.FieldModeEffectFlagBit.BIT_16,
                            E.FieldModeEffectFlagBit.BIT_17,
                        },
                    },
                },
                group_items = { set = { item } },
            },
            run = function(host)
                local record = collect_map_overlay(host)
                for _ = 1, kDialogueInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "collected the Renraku Map Overlay from safe record " ..
                            tostring(record)
                    end
                    tap(host, "A")
                end
                error("Renraku safe reward interaction did not settle")
            end,
        },
    },
}

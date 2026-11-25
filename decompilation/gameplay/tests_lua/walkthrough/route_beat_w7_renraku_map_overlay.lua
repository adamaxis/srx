local E = require("enums")

-- Scene-40 safe records 89-93 are event 0x1A at 0x1DE1D0..0x1DE1F8.
local kProjectedCellApproachOffset = 512
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
    { dx = 0, dy = kProjectedCellApproachOffset, direction = "UP" },
}

local story = E.StoryFact
local item = E.GroupItemId.MAP_OVERLAY

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function collect_map_overlay(host)
    for _, cell in ipairs(kSafeCells) do
        for _, approach in ipairs(kApproaches) do
            local placed = host.place_within_scene(
                E.FieldSafe.RENRAKU_SCENE,
                cell.x + approach.dx, cell.y + approach.dy)
            if placed then
                -- Safe records publish only when crossed with real pad input.
                host.hold_until_modal(approach.direction, kSafeInputFrames)
                if host.snapshot().story_facts[story.RENRAKU_MAP_OVERLAY] == true then
                    return cell.record
                end
            end
        end
    end
    error("Renraku map overlay did not trigger at Scene-40 safe records 89-93")
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
                return "seeded the ROM Ito-victory prerequisite; map overlay stayed clear"
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

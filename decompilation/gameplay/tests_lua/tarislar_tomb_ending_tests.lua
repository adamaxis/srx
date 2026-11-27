package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local Timeline = require("tarislar_tomb_ending")
local Kind = E.TarislarTombEndingEventKind

local checks = 0
local current_frame = 0
local function check(value, message)
    assert(value, message)
    checks = checks + 1
end

local function event_pairs(events, kind)
    local result = {}
    for index = 1, #events, 2 do
        if events[index] == kind then
            result[#result + 1] = {
                frame = current_frame,
                value = events[index + 1],
            }
        end
    end
    return result
end

local function same_pairs(actual, expected, label)
    check(#actual == #expected, label .. " count")
    for index, expected_event in ipairs(expected) do
        check(actual[index].frame == expected_event.frame and
              actual[index].value == expected_event.value,
              label .. " entry " .. index)
    end
end

check(Timeline.data.iteration_count == 1001 and
      Timeline.iteration_count()[1] == 1001,
      "0x5BB5A DBRA runs 1001 iterations")
check(Timeline.data.music.frame == 0 and
      Timeline.data.music.id == E.MusicId.ENDING and
      Timeline.step(0)[1] == Kind.MUSIC and
      Timeline.step(0)[2] == E.MusicId.ENDING,
      "music 0x13 is requested once before loop frame one")

local expected_sounds = {
    { frame = 0x001, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x01C, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x030, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x045, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x058, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x06C, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x085, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x095, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x0A0, value = E.SoundId.DAC_SAMPLE_2C },
    { frame = 0x0A8, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x0B0, value = E.SoundId.DAC_SAMPLE_2C },
    { frame = 0x0BC, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x0C4, value = E.SoundId.DAC_SAMPLE_2C },
    { frame = 0x0D0, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x0E0, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x0F0, value = E.SoundId.DAC_SAMPLE_2C },
    { frame = 0x26F, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x280, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x294, value = E.SoundId.SPELL_EFFECT_35 },
    { frame = 0x2A8, value = E.SoundId.SPELL_EFFECT_35 },
}
local sounds = {}
for frame = 1, Timeline.data.iteration_count do
    current_frame = frame
    local events = Timeline.step(frame, -1, -0x100)
    local found = event_pairs(events, Kind.SOUND)
    for _, event in ipairs(found) do
        sounds[#sounds + 1] = event
    end
end
same_pairs(sounds, expected_sounds, "ROM SFX (frame,id) schedule")

check(Timeline.random_bound(E.TarislarTombEnding.SHAKE_FIRST_FRAME)[1] ==
          E.TarislarTombEnding.SHAKE_RANDOM_BOUND and
      Timeline.random_bound(
          E.TarislarTombEnding.SHAKE_END_FRAME_EXCLUSIVE - 1)[1] ==
          E.TarislarTombEnding.SHAKE_RANDOM_BOUND and
      Timeline.random_bound(
          E.TarislarTombEnding.SHAKE_END_FRAME_EXCLUSIVE)[1] == 0,
      "shake draws only in the ROM window")
local shake = Timeline.step(
    E.TarislarTombEnding.SHAKE_FIRST_FRAME, 3, -0x100)
check(shake[1] == Kind.CAMERA_SHAKE and shake[2] == -3,
      "successful shake uses the negative random offset")
check(Timeline.step(
          E.TarislarTombEnding.SHAKE_FIRST_FRAME, 4, -0x100)[1] ==
          Kind.ROM_EVENT,
      "roll four does not shake")

local scroll_start = E.TarislarTombEnding.SCROLL_FIRST_FRAME
local scroll_stop = E.TarislarTombEnding.SCROLL_STOP_OFFSET
local scroll_events = Timeline.step(scroll_start, -1, scroll_stop + 1)
check(scroll_events[1] == Kind.CAMERA_SCROLL and
      scroll_events[2] == E.TarislarTombEnding.SCROLL_STEP,
      "scroll requires FFDD0E greater than minus 0x90")
local stopped_scroll = Timeline.step(scroll_start, -1, scroll_stop)
check(stopped_scroll[1] == Kind.ROM_EVENT,
      "scroll stops at the signed FFDD0E boundary")

local fade_frame = E.TarislarTombEnding.FADE_FRAME
local fade = Timeline.step(fade_frame, -1, -0x100)
local saw_fade = false
for index = 1, #fade, 2 do
    if fade[index] == Kind.FADE and fade[index + 1] == 1 then
        saw_fade = true
    end
end
check(saw_fade, "0x102A fade event is scheduled at frame 0x30C")

local function has_rom_event(frame, address)
    local events = Timeline.step(frame, -1, -0x100)
    for index = 1, #events, 2 do
        if events[index] == Kind.ROM_EVENT and events[index + 1] == address then
            return true
        end
    end
    return false
end
check(has_rom_event(E.TarislarTombEnding.CHOREOGRAPHY_FRAME_1, 0x5C744),
      "0x5C744 choreography event")
check(has_rom_event(E.TarislarTombEnding.CHOREOGRAPHY_FRAME_2, 0x5C842),
      "0x5C842 choreography event")
check(not has_rom_event(E.TarislarTombEnding.CHOREOGRAPHY_FRAME_3_FIRST,
                        0x5C88C) and
      has_rom_event(E.TarislarTombEnding.CHOREOGRAPHY_FRAME_3_FIRST + 1,
                    0x5C88C) and
      has_rom_event(E.TarislarTombEnding.CHOREOGRAPHY_FRAME_3_LAST - 1,
                    0x5C88C) and
      not has_rom_event(E.TarislarTombEnding.CHOREOGRAPHY_FRAME_3_LAST,
                        0x5C88C),
      "0x5C88C runs on even frames in its inclusive ROM window")

for frame = 1, Timeline.data.iteration_count do
    current_frame = frame
    local events = Timeline.step(frame, -1, -0x100)
    local frame_rom_events = event_pairs(events, Kind.ROM_EVENT)
    local saw_draw = false
    local saw_entity_update = false
    for _, event in ipairs(frame_rom_events) do
        saw_draw = saw_draw or event.value ==
            E.TarislarTombEndingRomEvent.ENTITY_DRAW
        saw_entity_update = saw_entity_update or event.value ==
            E.TarislarTombEndingRomEvent.ENTITY_UPDATE
    end
    check(saw_draw and saw_entity_update,
          "0x5C230 and 0x5BD2C emit on every loop iteration")
end

print(string.format("tarislar_tomb_ending_tests: %d checks passed", checks))

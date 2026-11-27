package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local Timeline = require("tarislar_tomb_ending")
local Kind = E.TarislarTombEndingEventKind
local RomEvent = E.TarislarTombEndingRomEvent
local Ending = E.TarislarTombEnding

local checks = 0
local current_t = 0
local function check(value, message)
    assert(value, message)
    checks = checks + 1
end

local function event_pairs(events, kind)
    local result = {}
    for index = 1, #events, 2 do
        if events[index] == kind then
            result[#result + 1] = {
                t = current_t,
                value = events[index + 1],
            }
        end
    end
    return result
end

local function same_pairs(actual, expected, label)
    check(#actual == #expected, label .. " count")
    for index, expected_event in ipairs(expected) do
        check(actual[index].t == expected_event.t and
              actual[index].value == expected_event.value,
              label .. " entry " .. index)
    end
end

check(Timeline.data.iteration_count == 1001 and
      Timeline.iteration_count()[1] == 1001,
      "0x5BB5A DBRA runs 1001 loop iterations")
local initial_flash = Timeline.initial_flash_state()
check(initial_flash[1] == 0 and initial_flash[2] == 0 and
      initial_flash[3] == 0x106 and initial_flash[4] == 0x10B,
      "0x5C30C idle state and 0x5C2FE initial spawn")
check(Timeline.step(0)[1] == Kind.MUSIC and
      Timeline.step(0)[2] == E.MusicId.ENDING,
      "music 0x13 is requested at t=0 before the loop")

local expected_sounds = {
    { t = 0x001, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x01C, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x030, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x045, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x058, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x06C, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x085, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x095, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x0A0, value = E.SoundId.DAC_SAMPLE_2C },
    { t = 0x0A8, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x0B0, value = E.SoundId.DAC_SAMPLE_2C },
    { t = 0x0BC, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x0C4, value = E.SoundId.DAC_SAMPLE_2C },
    { t = 0x0D0, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x0E0, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x0F0, value = E.SoundId.DAC_SAMPLE_2C },
    { t = 0x26F, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x280, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x294, value = E.SoundId.SPELL_EFFECT_35 },
    { t = 0x2A8, value = E.SoundId.SPELL_EFFECT_35 },
}

local expected_palette_line_0 = {
    0x222, 0x028, 0x244, 0x000, 0x444, 0x464, 0x04A, 0x068,
    0x466, 0x884, 0x26A, 0x28C, 0x08E, 0x2AE, 0x6EE,
}
local palette_line_0 = Timeline.palette_line_0()
check(#palette_line_0 == #expected_palette_line_0,
      "0x5CA74 palette has 15 line-0 colors")
for index, word in ipairs(expected_palette_line_0) do
    check(palette_line_0[index] == word,
          "0x5CA74 palette word " .. index)
end
local expected_palette_line_3 = {
    0x200, 0x600, 0x420, 0xA20, 0xE20, 0xE60, 0x444, 0x666,
    0x0AE, 0x08C, 0x06A, 0xE0E, 0xE0E, 0xE0E, 0xEEE,
}
local palette_line_3 = Timeline.palette_line_3()
check(#palette_line_3 == #expected_palette_line_3,
      "0x5C9B4 palette has 15 line-3 colors")
for index, word in ipairs(expected_palette_line_3) do
    check(palette_line_3[index] == word,
          "0x5C9B4 palette word " .. index)
end

local expected_spawns = {
    { t = 0x050, x = 0x0D3, y = 0x11C },
    { t = 0x06E, x = 0x114, y = 0x101 },
    { t = 0x0A0, x = 0x100, y = 0x150 },
    { t = 0x0D2, x = 0x0DD, y = 0x133 },
    { t = 0x26C, x = 0x0A3, y = 0x0FA },
    { t = 0x27B, x = 0x0D5, y = 0x0CF },
    { t = 0x28A, x = 0x107, y = 0x0B0 },
    { t = 0x2A3, x = 0x179, y = 0x0FF },
}
check(#Timeline.data.spawns == #expected_spawns,
      "0x5C230 has eight sparse loop spawns")
for index, spawn in ipairs(expected_spawns) do
    local actual = Timeline.data.spawns[index]
    local registers = Timeline.flash_state(spawn.t)
    check(actual.t == spawn.t and actual.x == spawn.x and actual.y == spawn.y and
          registers[1] == 0 and registers[2] == 0 and
          registers[3] == spawn.x and registers[4] == spawn.y,
          "0x5C31C register writes at t " .. spawn.t)
end

local expected_name_map = Timeline.name_map_begin()
check(expected_name_map[1] == Ending.NAME_MAP_SOURCE and
      expected_name_map[2] == Ending.NAME_MAP_COLUMNS and
      expected_name_map[3] == Ending.NAME_MAP_ROWS,
      "0x5C842 initializes a 38 by 19 name map at 0x1F6532")
for _, corner in ipairs({
    { 1, 1 }, { 1, Ending.NAME_MAP_COLUMNS },
    { Ending.NAME_MAP_ROWS, 1 },
    { Ending.NAME_MAP_ROWS, Ending.NAME_MAP_COLUMNS },
}) do
    local column = Timeline.name_map_column(corner[2] - 1)
    check(column[corner[1]] == 0x05D6,
          "ROM name-map corner word at 0x1F6532")
end

local bounds = Ending.SHAKE_RANDOM_BOUND
check(Timeline.random_bound(Ending.SHAKE_FIRST_T)[1] == bounds and
      Timeline.random_bound(Ending.SHAKE_END_T_EXCLUSIVE - 1)[1] == bounds and
      Timeline.random_bound(Ending.SHAKE_END_T_EXCLUSIVE)[1] == 0,
      "shake draws only in the ROM t window")
local shake = Timeline.step(Ending.SHAKE_FIRST_T, 3, -0x100)
check(shake[1] == Kind.CAMERA_SHAKE and shake[2] == -3,
      "successful shake uses the negative random offset")
local scroll = Timeline.step(Ending.SCROLL_FIRST_T, -1,
                             Ending.SCROLL_STOP_OFFSET + 1)
check(scroll[1] == Kind.CAMERA_SCROLL and scroll[2] == Ending.SCROLL_STEP,
      "scroll requires FFDD0E above minus 0x90")

local clear = Timeline.step(Ending.PLANE_B_CLEAR_T, -1, -0x100)
local saw_plane_b_clear = false
for index = 1, #clear, 2 do
    saw_plane_b_clear = saw_plane_b_clear or
        (clear[index] == Kind.CLEAR_PLANE_B and clear[index + 1] == 1)
end
check(saw_plane_b_clear,
      "0x102A is a plane-B clear at t=0x30C, not a palette fade")

local function has_rom_event(events, address)
    for index = 1, #events, 2 do
        if events[index] == Kind.ROM_EVENT and events[index + 1] == address then
            return true
        end
    end
    return false
end
check(has_rom_event(Timeline.step(Ending.CHOREOGRAPHY_T_1, -1, -0x100),
                    RomEvent.CHOREOGRAPHY_ONE), "0x5C744 at t=0x190")
check(has_rom_event(Timeline.step(Ending.CHOREOGRAPHY_T_2, -1, -0x100),
                    RomEvent.CHOREOGRAPHY_TWO), "0x5C842 at t=0x26C")

Timeline.reset()
current_t = 0
local sounds = {}
local third_choreography_events = 0
local map_columns = 0
local t_for_26f_sound = nil
local t = 0
for iteration = 1, Timeline.data.iteration_count do
    t = t + 1 -- the loop's 0x0BD2 call
    current_t = t
    local events = Timeline.step(t, -1, -0x100)
    local found = event_pairs(events, Kind.SOUND)
    for _, event in ipairs(found) do
        sounds[#sounds + 1] = event
        if event.t == 0x26F then
            t_for_26f_sound = iteration
        end
    end
    for index = 1, #events, 2 do
        if events[index] == Kind.ROM_EVENT and
           events[index + 1] == RomEvent.CHOREOGRAPHY_THREE then
            third_choreography_events = third_choreography_events + 1
        elseif events[index] == Kind.NAME_MAP_COLUMN then
            map_columns = map_columns + 1
        elseif events[index] == Kind.EXTRA_WAITS then
            check(t == Ending.CHOREOGRAPHY_T_1 and
                  events[index + 1] == Ending.CHOREOGRAPHY_ONE_EXTRA_WAITS,
                  "0x5C744 contributes six unpreserved waits")
            t = t + events[index + 1]
        end
    end
end
same_pairs(sounds, expected_sounds, "ROM SFX (t,id) schedule")
check(t_for_26f_sound == 0x269,
      "t=0x26F sound occurs on loop iteration 0x269")
check(t == 1007, "1001 loop iterations end at t=1007")
check(third_choreography_events == 39 and map_columns == 38 and
      Timeline.column_count()[1] == 38,
      "0x5C88C runs 39 times and writes exactly 38 columns")

print(string.format("tarislar_tomb_ending_tests: %d checks passed", checks))

-- ROM 0x5BAC4 pre-credit loop. Lua owns t policy and named event data;
-- C++ supplies each main 0x0BD2 wait and executes the returned operations.
-- The post-baseline prologue calls 0x5BE80/0x5C06C/0x5C30C/0x5BD2C/
-- 0x5C2FE/0x195C do not call 0x0BD2; t starts at zero after 0x5BB14 [C].
local E = require("enums")
local NameMap = require("tarislar_tomb_name_map")
local Ending = E.TarislarTombEnding
local RomEvent = E.TarislarTombEndingRomEvent
local Kind = E.TarislarTombEndingEventKind

local palette_line_0 = {
    0x222, 0x028, 0x244, 0x000, 0x444, 0x464, 0x04A, 0x068,
    0x466, 0x884, 0x26A, 0x28C, 0x08E, 0x2AE, 0x6EE,
} -- ROM 0x5CA74, words copied to FFD880 at 0x5C842 [C].
local palette_line_3 = {
    0x200, 0x600, 0x420, 0xA20, 0xE20, 0xE60, 0x444, 0x666,
    0x0AE, 0x08C, 0x06A, 0xE0E, 0xE0E, 0xE0E, 0xEEE,
} -- ROM 0x5C9B4, words copied to FFD8E0 at 0x5C744 [C].
local data = {
    iteration_count = Ending.LOOP_ITERATION_COUNT,
    maximum_t = Ending.LOOP_ITERATION_COUNT +
        Ending.CHOREOGRAPHY_ONE_EXTRA_WAITS,
    music = { t = 0, id = E.MusicId.ENDING },
    sounds = {
        { t = 1, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 28, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 48, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 69, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 88, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 108, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 133, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 149, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 168, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 188, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 208, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 224, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 623, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 640, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 660, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 680, id = E.SoundId.SPELL_EFFECT_35 },
        { t = 160, id = E.SoundId.DAC_SAMPLE_2C },
        { t = 176, id = E.SoundId.DAC_SAMPLE_2C },
        { t = 196, id = E.SoundId.DAC_SAMPLE_2C },
        { t = 240, id = E.SoundId.DAC_SAMPLE_2C },
    },
    shake = {
        first_t = Ending.SHAKE_FIRST_T,
        end_t_exclusive = Ending.SHAKE_END_T_EXCLUSIVE,
        random_bound = Ending.SHAKE_RANDOM_BOUND,
        random_threshold = Ending.SHAKE_RANDOM_THRESHOLD,
        camera_offset_sign = Ending.CAMERA_SHAKE_OFFSET_SIGN,
    },
    scroll = {
        first_t = Ending.SCROLL_FIRST_T,
        end_t_exclusive = Ending.SCROLL_END_T_EXCLUSIVE,
        stop_offset = Ending.SCROLL_STOP_OFFSET,
        step = Ending.SCROLL_STEP,
    },
    plane_b_clear_t = Ending.PLANE_B_CLEAR_T,
    choreography = {
        { t = Ending.CHOREOGRAPHY_T_1, address = RomEvent.CHOREOGRAPHY_ONE },
        { t = Ending.CHOREOGRAPHY_T_2, address = RomEvent.CHOREOGRAPHY_TWO },
        {
            first_t = Ending.CHOREOGRAPHY_T_3_FIRST,
            last_t = Ending.CHOREOGRAPHY_T_3_LAST,
            even_t_only = true,
            address = RomEvent.CHOREOGRAPHY_THREE,
        },
    },
    spawns = {
        { t = 0x050, x = 0x0D3, y = 0x11C },
        { t = 0x06E, x = 0x114, y = 0x101 },
        { t = 0x0A0, x = 0x100, y = 0x150 },
        { t = 0x0D2, x = 0x0DD, y = 0x133 },
        { t = 0x26C, x = 0x0A3, y = 0x0FA },
        { t = 0x27B, x = 0x0D5, y = 0x0CF },
        { t = 0x28A, x = 0x107, y = 0x0B0 },
        { t = 0x2A3, x = 0x179, y = 0x0FF },
    }, -- 0x5C230 sparse calls to 0x5C31C; prologue spawn at 0x5C2FE [C].
    per_iteration_rom_events = { RomEvent.ENTITY_UPDATE },
}

local sound_by_t = {}
local spawn_by_t = {}
for _, sound in ipairs(data.sounds) do
    assert(sound_by_t[sound.t] == nil,
        "ending timeline has multiple SFX at one t")
    sound_by_t[sound.t] = sound.id
end
for _, spawn in ipairs(data.spawns) do
    spawn_by_t[spawn.t] = spawn
end

local name_map_column = 0
local function is_integer(value)
    return type(value) == "number" and value % 1 == 0
end

local function append_event(result, kind, value)
    result[#result + 1] = kind
    result[#result + 1] = value
end

local Timeline = { data = data }

function Timeline.iteration_count()
    return { data.iteration_count }
end

function Timeline.reset()
    name_map_column = 0
    return { name_map_column }
end

function Timeline.initial_flash_state()
    -- 0x5C30C sets idle, then 0x5C2FE calls 0x5C31C(0x106, 0x10B).
    return { 0, 0, 0x106, 0x10B }
end

function Timeline.flash_state(t)
    local spawn = spawn_by_t[t]
    if spawn == nil then
        return nil
    end
    return { 0, 0, spawn.x, spawn.y }
end

function Timeline.palette_line_0()
    local result = {}
    for index, word in ipairs(palette_line_0) do
        result[index] = word
    end
    return result
end

function Timeline.palette_line_3()
    local result = {}
    for index, word in ipairs(palette_line_3) do
        result[index] = word
    end
    return result
end

function Timeline.name_map_begin()
    return { Ending.NAME_MAP_SOURCE, Ending.NAME_MAP_COLUMNS,
             Ending.NAME_MAP_ROWS }
end

function Timeline.name_map_column(column)
    if not is_integer(column) or column < 0 or
       column >= Ending.NAME_MAP_COLUMNS then
        return nil
    end
    local result = {}
    for row = 1, Ending.NAME_MAP_ROWS do
        result[row] = NameMap[row][column + 1]
    end
    return result
end

function Timeline.column_count()
    return { name_map_column }
end

function Timeline.random_bound(t)
    if not is_integer(t) or t < 0 or t > data.maximum_t then
        return nil
    end
    if t >= data.shake.first_t and t < data.shake.end_t_exclusive then
        return { data.shake.random_bound }
    end
    return { 0 }
end

-- Returns event-kind/value pairs. The host adds one main 0x0BD2 wait before
-- each call; 0x5C744 reports its six additional unpreserved waits [C].
function Timeline.step(t, shake_roll, vertical_scroll_offset)
    if not is_integer(t) or t < 0 or t > data.maximum_t then
        return nil
    end
    local result = {}
    if t == data.music.t then
        append_event(result, Kind.MUSIC, data.music.id)
        return result
    end

    if t >= data.shake.first_t and t < data.shake.end_t_exclusive and
       is_integer(shake_roll) and shake_roll >= 0 and
       shake_roll < data.shake.random_bound and
       shake_roll < data.shake.random_threshold then
        append_event(result, Kind.CAMERA_SHAKE,
                     data.shake.camera_offset_sign * shake_roll)
    end

    if t >= data.scroll.first_t and t < data.scroll.end_t_exclusive and
       is_integer(vertical_scroll_offset) and
       vertical_scroll_offset > data.scroll.stop_offset then
        append_event(result, Kind.CAMERA_SCROLL, data.scroll.step)
    end

    if t == data.plane_b_clear_t then
        append_event(result, Kind.CLEAR_PLANE_B, 1)
    end

    for _, event in ipairs(data.choreography) do
        local matches = event.t == t
        if event.first_t ~= nil then
            matches = t >= event.first_t and t <= event.last_t and
                (not event.even_t_only or t % 2 == 0)
        end
        if matches then
            append_event(result, Kind.ROM_EVENT, event.address)
            if event.address == RomEvent.CHOREOGRAPHY_ONE then
                append_event(result, Kind.CLEAR_PLANES, 1)
                append_event(result, Kind.ZERO_SCROLL_WORDS, 1)
                append_event(result, Kind.PALETTE_LINE_3, 3)
                append_event(result, Kind.BACKGROUND_CITY, 0)
                append_event(result, Kind.EXTRA_WAITS,
                             Ending.CHOREOGRAPHY_ONE_EXTRA_WAITS)
            elseif event.address == RomEvent.CHOREOGRAPHY_TWO then
                append_event(result, Kind.PALETTE_LINE_0, 0)
                append_event(result, Kind.NAME_MAP_BEGIN, 0)
                append_event(result, Kind.LOGO_REVEAL_OVERLAY, 0)
            elseif event.address == RomEvent.CHOREOGRAPHY_THREE and
                   name_map_column < Ending.NAME_MAP_COLUMNS then
                append_event(result, Kind.NAME_MAP_COLUMN, name_map_column)
                name_map_column = name_map_column + 1
            end
        end
    end

    local spawn = spawn_by_t[t]
    if spawn ~= nil then
        append_event(result, Kind.ROM_EVENT, RomEvent.SPARSE_FLASH_SPAWN)
        append_event(result, Kind.FLASH_SPAWN, t)
    end
    for _, address in ipairs(data.per_iteration_rom_events) do
        append_event(result, Kind.ROM_EVENT, address)
    end
    local sound_id = sound_by_t[t]
    if sound_id ~= nil then
        append_event(result, Kind.SOUND, sound_id)
    end
    return result
end

return Timeline

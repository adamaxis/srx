-- ROM 0x5BAC4 pre-credit loop. Lua owns t policy and named event data;
-- C++ supplies each main 0x0BD2 wait and executes the returned operations.
-- The post-baseline prologue calls 0x5BE80/0x5C06C/0x5C30C/0x5BD2C/
-- 0x5C2FE/0x195C do not call 0x0BD2; t starts at zero after 0x5BB14 [C].
local E = require("enums")
local NameMap = require("tarislar_tomb_name_map")
local Ending = E.TarislarTombEnding
local RomEvent = E.TarislarTombEndingRomEvent
local Kind = E.TarislarTombEndingEventKind
local SpriteList = E.TarislarTombEndingSpriteList

local animation_lists = { SpriteList.ANIMATION_0, SpriteList.ANIMATION_1 }
local spark_lists = {
    SpriteList.SPARK_0, SpriteList.SPARK_1,
    SpriteList.SPARK_2, SpriteList.SPARK_3,
}
local flash_lists = {
    SpriteList.FLASH_0, SpriteList.FLASH_1, SpriteList.FLASH_2,
    SpriteList.FLASH_3, SpriteList.FLASH_4, SpriteList.FLASH_5,
}
local debris_motion = {
    { 0, Ending.SPRITE_CITY_Y_STEP,
      Ending.SPRITE_DEBRIS_FIRST_DURATION },
    { -Ending.SPRITE_CITY_Y_STEP, Ending.SPRITE_CITY_Y_STEP,
      Ending.SPRITE_DEBRIS_SHORT_DURATION },
    { -2 * Ending.SPRITE_CITY_Y_STEP, Ending.SPRITE_CITY_Y_STEP,
      Ending.SPRITE_DEBRIS_SHORT_DURATION },
    { -Ending.SPRITE_CITY_Y_STEP, Ending.SPRITE_CITY_Y_STEP,
      Ending.SPRITE_DEBRIS_LAST_DURATION },
    { 0, 0, 0 },
}
local object_a_motion = {
    { Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_A_ROW_0_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_33, Ending.SPRITE_OBJECT_MOTION_33,
      Ending.SPRITE_OBJECT_A_SHORT_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_33, Ending.SPRITE_OBJECT_MOTION_66,
      Ending.SPRITE_OBJECT_A_SHORT_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_66, Ending.SPRITE_OBJECT_MOTION_66,
      Ending.SPRITE_OBJECT_A_SHORT_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_99, Ending.SPRITE_OBJECT_MOTION_99,
      Ending.SPRITE_OBJECT_A_SHORT_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_66, Ending.SPRITE_OBJECT_MOTION_99,
      Ending.SPRITE_OBJECT_A_SHORT_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_A_ROW_6_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_ZERO, Ending.SPRITE_OBJECT_MOTION_80,
      Ending.SPRITE_OBJECT_A_ROW_7_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_A_ROW_8_DURATION },
} -- ROM table 0x5C1FA [C].
local object_b_motion = {
    { Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_B_ROW_0_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_33, Ending.SPRITE_OBJECT_MOTION_33,
      Ending.SPRITE_OBJECT_B_ROW_1_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_33, Ending.SPRITE_OBJECT_MOTION_66,
      Ending.SPRITE_OBJECT_B_SHORT_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_99, Ending.SPRITE_OBJECT_MOTION_CC,
      Ending.SPRITE_OBJECT_B_SHORT_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_B_ROW_4_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_ZERO, Ending.SPRITE_OBJECT_MOTION_80,
      Ending.SPRITE_OBJECT_B_ROW_5_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_B_ROW_6_DURATION },
    { Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_MOTION_ZERO,
      Ending.SPRITE_OBJECT_B_ROW_7_DURATION },
} -- ROM table 0x5C1D0 [C].

local function fresh_sprite_state()
    return {
        fff1b8 = Ending.SPRITE_MAIN_Y_START,
        fff1ba = 0,
        fff1be = 0,
        fff1c0 = 0,
        fff1c2 = 0,
        fff1c6 = 0,
        fff1c8 = 0,
        fff14a = 0,
        fff14e = 0,
        fff150 = 0,
        fff152 = Ending.SPRITE_FLASH_INITIAL_X,
        fff154 = Ending.SPRITE_FLASH_INITIAL_Y,
        fff1ac = 0,
        fff1ae = 0,
        fff1b0 = 0,
        fff1b2 = 0,
        fff1b4 = 0,
        fff1b6 = 0,
        fff1ca = 0,
        fff1cc = 0,
        fff1ce = Ending.SPRITE_OBJECT_A_INITIAL_X,
        fff1d0 = Ending.SPRITE_OBJECT_A_INITIAL_Y,
        fff1d2 = 0,
        fff1d4 = 0,
        fff1d6 = 0,
        fff1d8 = 0,
        fff1da = Ending.SPRITE_OBJECT_B_INITIAL_X,
        fff1dc = Ending.SPRITE_OBJECT_B_INITIAL_Y,
        fff1de = 0,
        fff1e0 = 0,
        last_wait_clock_t = 0,
    }
end
local sprite_state = fresh_sprite_state()

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
    sprite_lists = SpriteList,
    debris_motion = debris_motion, -- ROM table 0x5BE62 [C].
    object_a_motion = object_a_motion,
    object_b_motion = object_b_motion,
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
    sprite_state = fresh_sprite_state()
    return { name_map_column }
end

function Timeline.initial_flash_state()
    -- 0x5C30C sets idle, then 0x5C2FE calls 0x5C31C(0x106, 0x10B).
    return { 0, 0, Ending.SPRITE_FLASH_INITIAL_X,
             Ending.SPRITE_FLASH_INITIAL_Y }
end

function Timeline.flash_state(t)
    local spawn = spawn_by_t[t]
    if spawn == nil then
        return nil
    end
    sprite_state.fff150 = 0
    sprite_state.fff14e = 0
    sprite_state.fff152 = spawn.x
    sprite_state.fff154 = spawn.y
    return { 0, 0, spawn.x, spawn.y }
end

function Timeline.sprite_random_bound(t)
    if type(t) ~= "number" or t % 1 ~= 0 or t < 0 then
        return nil
    end
    return { t < Ending.SPRITE_SCATTER_END_T
        and Ending.SPRITE_SCATTER_RANDOM_BOUND or 0 }
end

-- The Lua policy mirrors 0x5BD2C's ordered calls to 0x5BEA2, 0x5C02E,
-- 0x5C336, and 0x5BD6A. The host only emits these named ROM lists into its
-- typed SAT shadow using the extracted piece records.
function Timeline.sprite_frame(t, now, scatter_roll)
    if not is_integer(t) or t < 0 or t > Ending.LOOP_ITERATION_COUNT +
       Ending.CHOREOGRAPHY_ONE_EXTRA_WAITS or
       not is_integer(now) or now < 0 or now > 0xFFFF then
        return nil
    end
    -- ROM 0x5BEC6: one 0x33 step per 0x5BEA2 call while 0x190 <= t < 0x1F4.
    -- 0x5BEA2 runs once per outer iteration (0x5BB7A), after 0x5C744's six
    -- extra 0x0BD2 waits, so the first call in the window sees t=0x196 and
    -- t=0x190..0x195 never descend: 94 steps, FFF1B8 ends at 0x3206.
    if t >= Ending.SPRITE_CITY_START_T and t < Ending.SPRITE_CITY_SHIFT_END_T then
        sprite_state.fff1b8 = sprite_state.fff1b8 - Ending.SPRITE_CITY_Y_STEP
    end
    sprite_state.last_wait_clock_t = t
    local result = { sprite_state.fff1b8 }
    local function emit(address, x, y, attributes)
        result[#result + 1] = address
        result[#result + 1] = x
        result[#result + 1] = y
        result[#result + 1] = attributes
    end
    local logical_y = math.floor(
        sprite_state.fff1b8 / Ending.SPRITE_FIXED_POINT_SCALE)

    -- ROM 0x5BEA2.
    if t < Ending.SPRITE_MAIN_END_T then
        emit(SpriteList.MAIN, Ending.SPRITE_MAIN_X, logical_y,
             Ending.SPRITE_MAIN_ATTRIBUTE)
        if t >= Ending.SPRITE_BACKGROUND_ALT_T or
           (t >= Ending.SPRITE_FLASH_START_T and
            t < Ending.SPRITE_FLASH_END_T) then
            emit(SpriteList.MAIN_END, Ending.SPRITE_MAIN_X, logical_y,
                 Ending.SPRITE_MAIN_ATTRIBUTE)
        else
            if now >= sprite_state.fff1c0 then
                sprite_state.fff1ba = animation_lists[sprite_state.fff1be + 1]
                sprite_state.fff1c0 = (now + Ending.SPRITE_ANIMATION_PERIOD) %
                    Ending.SPRITE_WORD_MODULUS
                sprite_state.fff1be =
                    (sprite_state.fff1be + 1) % Ending.SPRITE_ANIMATION_COUNT
            end
            emit(sprite_state.fff1ba, Ending.SPRITE_MAIN_X, logical_y,
                 Ending.SPRITE_MAIN_ATTRIBUTE)
            if now >= sprite_state.fff1c8 then
                sprite_state.fff1c2 = spark_lists[sprite_state.fff1c6 + 1]
                sprite_state.fff1c8 = (now + Ending.SPRITE_SPARK_PERIOD) %
                    Ending.SPRITE_WORD_MODULUS
                sprite_state.fff1c6 =
                    (sprite_state.fff1c6 + 1) % Ending.SPRITE_SPARK_COUNT
            end
            emit(sprite_state.fff1c2,
                 Ending.SPRITE_SPARK_X +
                     Ending.SPRITE_SPARK_X_STEP * (1 - sprite_state.fff1be),
                 logical_y - Ending.SPRITE_SPARK_Y_OFFSET,
                 Ending.SPRITE_MAIN_ATTRIBUTE)
        end
    end

    -- ROM 0x5C02E.
    if t >= Ending.SPRITE_CITY_START_T and t < Ending.SPRITE_MAIN_END_T then
        emit(SpriteList.CITY, Ending.SPRITE_CITY_X,
             logical_y + Ending.SPRITE_CITY_Y_OFFSET,
             Ending.SPRITE_MAIN_ATTRIBUTE)
    end

    -- ROM 0x5C336; test the deadline before the six-list completion case.
    if now < sprite_state.fff150 then
        emit(sprite_state.fff14a, sprite_state.fff152, sprite_state.fff154,
             Ending.SPRITE_FLASH_ATTRIBUTE)
    elseif sprite_state.fff14e < Ending.SPRITE_FLASH_LIMIT then
        sprite_state.fff14a = flash_lists[sprite_state.fff14e + 1]
        sprite_state.fff150 = (now + Ending.SPRITE_FLASH_INTERVAL) %
            Ending.SPRITE_WORD_MODULUS
        sprite_state.fff14e = sprite_state.fff14e + 1
        emit(sprite_state.fff14a, sprite_state.fff152, sprite_state.fff154,
             Ending.SPRITE_FLASH_ATTRIBUTE)
    end

    -- ROM 0x5BD6A.
    if t < Ending.SPRITE_SCATTER_END_T then
        if not is_integer(scatter_roll) or scatter_roll < 0 or
           scatter_roll >= Ending.SPRITE_SCATTER_RANDOM_BOUND then
            return nil
        end
        local x = Ending.SPRITE_SCATTER_X
        local y = Ending.SPRITE_SCATTER_Y
        if scatter_roll < Ending.SPRITE_SCATTER_RANDOM_LIMIT then
            x = x + scatter_roll
            y = y + scatter_roll
        end
        emit(SpriteList.SCATTER, x, y, Ending.SPRITE_FLASH_ATTRIBUTE)
    elseif t < Ending.SPRITE_SCATTER_END_T_EXCLUSIVE then
        if t == Ending.SPRITE_DEBRIS_START_T then
            sprite_state.fff1ac = 0
            sprite_state.fff1ae = 0
            sprite_state.fff1b0 = Ending.SPRITE_DEBRIS_X_START
            sprite_state.fff1b2 = Ending.SPRITE_DEBRIS_Y_START
        end
        if now >= sprite_state.fff1ae and
           sprite_state.fff1ac < Ending.SPRITE_DEBRIS_ANIMATION_STEPS then
            local step = debris_motion[sprite_state.fff1ac + 1]
            sprite_state.fff1b4 = step[1]
            sprite_state.fff1b6 = step[2]
            sprite_state.fff1ae = (now + step[3]) %
                Ending.SPRITE_WORD_MODULUS
            sprite_state.fff1ac = sprite_state.fff1ac + 1
        end
        sprite_state.fff1b0 = (sprite_state.fff1b0 +
            sprite_state.fff1b4) % Ending.SPRITE_WORD_MODULUS
        sprite_state.fff1b2 = (sprite_state.fff1b2 +
            sprite_state.fff1b6) % Ending.SPRITE_WORD_MODULUS
        emit(SpriteList.DEBRIS,
             math.floor(sprite_state.fff1b0 / Ending.SPRITE_FIXED_POINT_SCALE),
             math.floor(sprite_state.fff1b2 / Ending.SPRITE_FIXED_POINT_SCALE),
             Ending.SPRITE_FLASH_ATTRIBUTE)
    end

    -- ROM 0x5C0A6 advances two fixed-point objects until t reaches 0x17C.
    if t < Ending.SPRITE_OBJECTS_END_T then
        if now >= sprite_state.fff1cc and
           sprite_state.fff1ca < #object_a_motion then
            local step = object_a_motion[sprite_state.fff1ca + 1]
            sprite_state.fff1d2 = step[1]
            sprite_state.fff1d4 = step[2]
            sprite_state.fff1cc = (now + step[3]) %
                Ending.SPRITE_WORD_MODULUS
            sprite_state.fff1ca = sprite_state.fff1ca + 1
        end
        sprite_state.fff1ce = (sprite_state.fff1ce + sprite_state.fff1d2) %
            Ending.SPRITE_WORD_MODULUS
        sprite_state.fff1d0 = (sprite_state.fff1d0 + sprite_state.fff1d4) %
            Ending.SPRITE_WORD_MODULUS
        emit(SpriteList.OBJECT_A,
             math.floor(sprite_state.fff1ce /
                 Ending.SPRITE_FIXED_POINT_SCALE),
             math.floor(sprite_state.fff1d0 /
                 Ending.SPRITE_FIXED_POINT_SCALE), 0)

        if now >= sprite_state.fff1d8 and
           sprite_state.fff1d6 < #object_b_motion then
            local step = object_b_motion[sprite_state.fff1d6 + 1]
            sprite_state.fff1de = step[1]
            sprite_state.fff1e0 = step[2]
            sprite_state.fff1d8 = (now + step[3]) %
                Ending.SPRITE_WORD_MODULUS
            sprite_state.fff1d6 = sprite_state.fff1d6 + 1
        end
        sprite_state.fff1da = (sprite_state.fff1da + sprite_state.fff1de) %
            Ending.SPRITE_WORD_MODULUS
        sprite_state.fff1dc = (sprite_state.fff1dc + sprite_state.fff1e0) %
            Ending.SPRITE_WORD_MODULUS
        emit(SpriteList.OBJECT_B,
             math.floor(sprite_state.fff1da /
                 Ending.SPRITE_FIXED_POINT_SCALE),
             math.floor(sprite_state.fff1dc /
                 Ending.SPRITE_FIXED_POINT_SCALE), 0)
    end
    return result
end

function Timeline.sprite_registers()
    return {
        sprite_state.fff1b8, sprite_state.fff1be, sprite_state.fff1c0,
        sprite_state.fff1c6, sprite_state.fff1c8, sprite_state.fff14a,
        sprite_state.fff14e, sprite_state.fff150, sprite_state.fff152,
        sprite_state.fff154, sprite_state.fff1ac, sprite_state.fff1ae,
        sprite_state.fff1b0, sprite_state.fff1b2,
    }
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

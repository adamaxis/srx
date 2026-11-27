-- ROM 0x5BAC4's pre-credit loop. Lua owns frame policy and event order;
-- the field host supplies the source random roll and FFDD0E fact.
local E = require("enums")
local Ending = E.TarislarTombEnding
local RomEvent = E.TarislarTombEndingRomEvent
local Kind = E.TarislarTombEndingEventKind

local data = {
    iteration_count = Ending.LOOP_ITERATION_COUNT,
    music = { frame = 0, id = E.MusicId.ENDING },
    sounds = {
        { frame = 1, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 28, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 48, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 69, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 88, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 108, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 133, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 149, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 168, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 188, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 208, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 224, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 623, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 640, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 660, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 680, id = E.SoundId.SPELL_EFFECT_35 },
        { frame = 160, id = E.SoundId.DAC_SAMPLE_2C },
        { frame = 176, id = E.SoundId.DAC_SAMPLE_2C },
        { frame = 196, id = E.SoundId.DAC_SAMPLE_2C },
        { frame = 240, id = E.SoundId.DAC_SAMPLE_2C },
    },
    shake = {
        first_frame = Ending.SHAKE_FIRST_FRAME,
        end_frame_exclusive = Ending.SHAKE_END_FRAME_EXCLUSIVE,
        random_bound = Ending.SHAKE_RANDOM_BOUND,
        random_threshold = Ending.SHAKE_RANDOM_THRESHOLD,
        camera_offset_sign = Ending.CAMERA_SHAKE_OFFSET_SIGN,
    },
    scroll = {
        first_frame = Ending.SCROLL_FIRST_FRAME,
        end_frame_exclusive = Ending.SCROLL_END_FRAME_EXCLUSIVE,
        stop_offset = Ending.SCROLL_STOP_OFFSET,
        step = Ending.SCROLL_STEP,
    },
    fade_frame = Ending.FADE_FRAME,
    rom_events_each_frame = {
        RomEvent.ENTITY_DRAW,
        RomEvent.ENTITY_UPDATE,
    },
    rom_events = {
        {
            frame = Ending.CHOREOGRAPHY_FRAME_1,
            address = RomEvent.CHOREOGRAPHY_ONE,
        },
        {
            frame = Ending.CHOREOGRAPHY_FRAME_2,
            address = RomEvent.CHOREOGRAPHY_TWO,
        },
        {
            first_frame = Ending.CHOREOGRAPHY_FRAME_3_FIRST,
            last_frame = Ending.CHOREOGRAPHY_FRAME_3_LAST,
            even_frames_only = true,
            address = RomEvent.CHOREOGRAPHY_THREE,
        },
    },
}

local sound_by_frame = {}
for _, sound in ipairs(data.sounds) do
    assert(sound_by_frame[sound.frame] == nil,
        "ending timeline has multiple SFX at one frame")
    sound_by_frame[sound.frame] = sound.id
end

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

function Timeline.random_bound(frame)
    if not is_integer(frame) or frame < 0 or
       frame > data.iteration_count then
        return nil
    end
    if frame >= data.shake.first_frame and
       frame < data.shake.end_frame_exclusive then
        return { data.shake.random_bound }
    end
    return { 0 }
end

-- Returns event-kind/value integer pairs. At t=0 the host starts music before
-- the first 0x0BD2 wait; t=1..1001 are the exact DBRA loop iterations.
function Timeline.step(frame, shake_roll, vertical_scroll_offset)
    if not is_integer(frame) or frame < 0 or
       frame > data.iteration_count then
        return nil
    end
    local result = {}
    if frame == data.music.frame then
        append_event(result, Kind.MUSIC, data.music.id)
        return result
    end

    if frame >= data.shake.first_frame and
       frame < data.shake.end_frame_exclusive then
        if is_integer(shake_roll) and shake_roll >= 0 and
           shake_roll < data.shake.random_bound and
           shake_roll < data.shake.random_threshold then
            append_event(result, Kind.CAMERA_SHAKE,
                         data.shake.camera_offset_sign * shake_roll)
        end
    end

    if frame >= data.scroll.first_frame and
       frame < data.scroll.end_frame_exclusive and
       is_integer(vertical_scroll_offset) and
       vertical_scroll_offset > data.scroll.stop_offset then
        append_event(result, Kind.CAMERA_SCROLL, data.scroll.step)
    end

    if frame == data.fade_frame then
        append_event(result, Kind.FADE, 1)
    end
    for _, event in ipairs(data.rom_events) do
        local matches = event.frame == frame
        if event.first_frame ~= nil then
            matches = frame >= event.first_frame and
                frame <= event.last_frame and
                (not event.even_frames_only or frame % 2 == 0)
        end
        if matches then
            append_event(result, Kind.ROM_EVENT, event.address)
        end
    end
    for _, address in ipairs(data.rom_events_each_frame) do
        append_event(result, Kind.ROM_EVENT, address)
    end
    local sound_id = sound_by_frame[frame]
    if sound_id ~= nil then
        append_event(result, Kind.SOUND, sound_id)
    end
    return result
end

return Timeline

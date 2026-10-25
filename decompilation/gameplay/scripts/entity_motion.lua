-- ROM 0x15E56/0x15E16 heading samples shared by source entity movement.
-- This data is presented to Lua so the behavior layer can select its own
-- heading and derive the corresponding fixed-point step.
local EntityMotion = {}
local AiTuning = require("enums").AiTuning

local heading_wave = {
    0x0000, 0x0647, 0x0c8b, 0x12c7, 0x18f8, 0x1f19, 0x2527, 0x2b1e,
    0x30fb, 0x36b9, 0x3c56, 0x41cd, 0x471c, 0x4c3f, 0x5133, 0x55f4,
    0x5a81, 0x5ed6, 0x62f1, 0x66ce, 0x6a6c, 0x6dc9, 0x70e1, 0x73b5,
    0x7640, 0x7883, 0x7a7c, 0x7c29, 0x7d89, 0x7e9c, 0x7f61, 0x7fd7,
    0x7fff, 0x7fd7, 0x7f61, 0x7e9c, 0x7d89, 0x7c29, 0x7a7c, 0x7883,
    0x7640, 0x73b5, 0x70e1, 0x6dc9, 0x6a6c, 0x66ce, 0x62f1, 0x5ed6,
    0x5a81, 0x55f4, 0x5133, 0x4c3f, 0x471c, 0x41cd, 0x3c56, 0x36b9,
    0x30fb, 0x2b1e, 0x2527, 0x1f19, 0x18f8, 0x12c7, 0x0c8b, 0x0647,
    0x0000, 0xf9b9, 0xf375, 0xed39, 0xe708, 0xe0e7, 0xdad9, 0xd4e2,
    0xcf05, 0xc947, 0xc3aa, 0xbe33, 0xb8e4, 0xb3c1, 0xaecd, 0xaa0c,
    0xa57f, 0xa12a, 0x9d0f, 0x9932, 0x9594, 0x9237, 0x8f1f, 0x8c4b,
    0x89c0, 0x877d, 0x8584, 0x83d7, 0x8277, 0x8164, 0x809f, 0x8029,
    0x8001, 0x8029, 0x809f, 0x8164, 0x8277, 0x83d7, 0x8584, 0x877d,
    0x89c0, 0x8c4b, 0x8f1f, 0x9237, 0x9594, 0x9932, 0x9d0f, 0xa1a2,
    0xa57f, 0xaa0c, 0xaecd, 0xb3c1, 0xb8e4, 0xbe33, 0xc3aa, 0xc947,
    0xcf05, 0xd4e2, 0xdad9, 0xe0e7, 0xe708, 0xed39, 0xf375, 0xf9b9,
}

local function signed_word(value)
    value = value & 0xffff
    return value >= 0x8000 and value - 0x10000 or value
end

local function scale_sample(value)
    -- ROM 0x150E8 multiplies the signed sample by 0xC and arithmetic-shifts
    -- right four bits before writing the projectile offsets.
    return math.floor((signed_word(value) * 0x0c) / 0x10)
end

function EntityMotion.heading_offsets(heading)
    if type(heading) ~= "number" or heading % 1 ~= 0 or
       heading < 0 or heading > 0xffff then
        return nil
    end
    local index = ((heading >> 8) & 0xfe) >> 1
    return {
        horizontal = scale_sample(heading_wave[index + 1]),
        vertical = scale_sample(heading_wave[((index + 96) & 0x7f) + 1]),
    }
end

-- Generic ROM scaling: MULS.W #mul then ASR.L #shift (0x1575A uses 8/5,
-- 0x157F2 uses 8/4).
function EntityMotion.scaled_heading_offsets(heading, mul, shift)
    local index = ((heading >> 8) & 0xfe) >> 1
    local d = 1 << shift
    return {
        horizontal = (signed_word(heading_wave[index + 1]) * mul) // d,
        vertical = (signed_word(heading_wave[((index + 96) & 0x7f) + 1]) * mul) // d,
    }
end

function EntityMotion.offset_at_radius(heading, radius)
    -- ROM 0x15DCC uses the same 128 signed samples at 0x15E56. Its first
    -- product reads the preceding 0x40-byte quarter-wave table at 0x15E16;
    -- after 0x14540 swaps d2/d3, horizontal is sample[n] and vertical is
    -- sample[n-0x20]. MULS; double; SWAP yields floor(product / 0x8000) [C].
    if type(heading) ~= "number" or heading % 1 ~= 0 or
       heading < 0 or heading > AiTuning.WORD_MAX or
       type(radius) ~= "number" or radius % 1 ~= 0 or radius < 0 or
       radius > AiTuning.WORD_MAX then
        return nil
    end
    local sample_index = (heading >> AiTuning.FOLLOW_SAMPLE_SHIFT) &
                         AiTuning.FOLLOW_SAMPLE_MASK
    local horizontal_sample = signed_word(heading_wave[sample_index + 1])
    local vertical_sample = signed_word(heading_wave[
        ((sample_index - AiTuning.FOLLOW_QUARTER_WAVE_SAMPLES) &
         AiTuning.FOLLOW_SAMPLE_MASK) + 1])
    return {
        horizontal = math.floor(
            horizontal_sample * radius / AiTuning.FOLLOW_VECTOR_SCALE),
        vertical = math.floor(
            vertical_sample * radius / AiTuning.FOLLOW_VECTOR_SCALE),
    }
end

return EntityMotion

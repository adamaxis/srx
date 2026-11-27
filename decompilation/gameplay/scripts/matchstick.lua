-- Matchstick's Scene-0 doorman and Ito actor-list result.
local DialogueGraph = require("dialogue_graph")
local E = require("enums")
local StoryState = require("story_state")
local MatrixPasscodes = require("data.matrix_passcodes")

local function passcode_flag_bit(passcode_id)
    for _, passcode in ipairs(MatrixPasscodes.passcodes) do
        if passcode.id == passcode_id then
            return passcode.flag_bit
        end
    end
    return nil
end

local Matchstick = {}

function Matchstick.present_doorman()
    return DialogueGraph.run("matchstick.scene0_doorman_entry")
end

-- ROM callback 0x54286: +2 heat capped at 8, +10,000 nuyen, set the Ito
-- result and related mode/tip bits, then advance the shared counter twice.
function Matchstick.apply_ito_callback()
    local heat = StoryState.heat_level()
    local stage = StoryState.strike_team_stage()
    local nuyen = game.nuyen()
    if heat == nil or stage == nil or type(nuyen) ~= "number" or
       game.state.matrix_passcode_flags_resolved ~= true then
        return "service_unavailable"
    end

    game.set_nuyen(nuyen + 10000)
    if not StoryState.set_heat_level(math.min(
        StoryState.MAX_HEAT_LEVEL, heat + 2)) then
        return "service_unavailable"
    end
    game.state[E.StoryFact.ITO_FIGHT_WON] = true
    -- ROM 0x542BA..0x542C4 sets FFFC28 bit 11, the Enter Passcode entry
    -- for Ito's system (matrix_passcodes "ito_passcode", 0xC19C2 table).
    game.state["matrix_passcode_bit_" ..
        passcode_flag_bit("ito_passcode")] = true

    -- ROM 0x542D0..0x542FA clears FFFC30 bits 24/25/27 and sets
    -- FFFC34 bits 15/16/17. 0x54300 clears FFFC38 bit 0 (tip index 64).
    game.state["field_mode_flag_bit_" .. E.FieldModeFlagBit.BIT_24] = false
    game.state["field_mode_flag_bit_" .. E.FieldModeFlagBit.RENRAKU_HEADHUNTER_TOLD] = false
    game.state["field_mode_flag_bit_" .. E.FieldModeFlagBit.BIT_27] = false
    game.state["field_mode_effect_flag_bit_" .. E.FieldModeEffectFlagBit.BIT_15] = true
    game.state["field_mode_effect_flag_bit_" .. E.FieldModeEffectFlagBit.BIT_16] = true
    game.state["field_mode_effect_flag_bit_" .. E.FieldModeEffectFlagBit.BIT_17] = true
    game.state["notebook_tip_flag_bit_" .. E.NotebookTipFlagBit.BIT_64] = false

    -- ROM 0x54310 and 0x54316 call the same one-step counter twice.
    if not StoryState.increase_strike_team_stage(1) or
       not StoryState.increase_strike_team_stage(1) then
        return "service_unavailable"
    end
    -- ROM 0x5430A also calls 0x5D1B4 (prepare_pre_title_audio_variant_b),
    -- a host audio-asset preparation step with no gameplay state.
    return "applied"
end

function Matchstick.present_ito_activity_result()
    return DialogueGraph.run("matchstick.ito_activity_result")
end

return Matchstick

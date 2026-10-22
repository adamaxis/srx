-- Scene 2/event 6 is one indexed-title-service interaction, not a route to
-- resource scene 26. The JSON graph owns its prompts, ordered topic gates,
-- payment outcomes, and continue-in-place terminal. This module supplies only
-- the named story mutations that cannot be expressed as plain flag writes.
local DialogueGraph = require("dialogue_graph")

local Boris = {}

local function is_integer(value, minimum, maximum)
    return type(value) == "number" and value % 1 == 0 and
        value >= minimum and value <= maximum
end

local function set_field_dialogue_flag(arguments, value)
    assert(is_integer(arguments.flagBit, 0, 3),
        "Boris field dialogue flag must be in range 0..3")
    game.state["field_dialogue_flag_bit_" .. arguments.flagBit] = value
    return true
end

local function action_registry()
    return {
        ["boris.clear_field_dialogue_flag"] = function(arguments)
            return set_field_dialogue_flag(arguments, false)
        end,
        ["boris.mark_paid_dialogue_flag"] = function(arguments)
            assert(arguments.flagBit == 0 or arguments.flagBit == 2,
                "Boris payment action supports dialogue flag 0 or 2")
            return set_field_dialogue_flag(arguments, true)
        end,
        ["boris.reveal_michael_information"] = function()
            -- Command 0x2B at 0x21B82 learns Michael's alias and prepares the
            -- source field-mode state for the next same-map service visit.
            game.state.story_flag_bit_10 = true
            game.state.field_dialogue_flag_bit_1 = true
            game.state.field_mode_flag_bit_6 = false
            game.state.field_mode_flag_bit_7 = false
            game.state.field_mode_flag_bit_20 = true
            return true
        end,
        ["boris.prepare_david_information"] = function()
            -- Command 0x3B at 0x21EB0 updates these FFFC30 mode bits. The
            -- subsequent Brother? choice reaches unconverted command 0x2C.
            game.state.field_mode_flag_bit_6 = false
            game.state.field_mode_flag_bit_7 = false
            game.state.field_mode_flag_bit_20 = true
            return true
        end,
        ["boris.mark_ilene_information"] = function()
            -- Command 0x2A at 0x21B6A selects the Ilene information branch.
            game.state.field_mode_effect_flag_bit_0 = true
            return true
        end,
    }
end

function Boris.present_indexed_title_service()
    return DialogueGraph.run(
        "boris_greenhouse.indexed_title_service", action_registry())
end

return Boris

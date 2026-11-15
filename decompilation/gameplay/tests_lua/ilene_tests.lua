package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local backing = {}
local spoken = {}
game = {
    field = { package_run_status = "none" },
    location = { type = "field_map", id = "scene_20_ilene" },
    state = setmetatable({}, {
        __index = function(_, key) return backing[key] end,
        __newindex = function(_, key, value) backing[key] = value end,
    }),
    services = {
        call = function(service, action)
            assert(service == "party" and action == "award_karma_2")
            return "applied"
        end,
    },
    ui = {
        say_text = function(text, continuation)
            spoken[#spoken + 1] = { text = text, continuation = continuation }
        end,
    },
}

local Ilene = require("ilene")

-- [C] Service 19 at 0x543A2 arms FFE182 when FFFC34 bit 0 OR FFFC38 bit 12
-- (tip 76) is set; FFFC30 bit 12 (tip 12) is a different word.
local function prepare_with(effect_bit_0, tip_76, tip_12)
    for key in pairs(backing) do backing[key] = nil end
    backing.red_buffalo_woman = true
    backing.ilene_met = false
    backing.field_mode_effect_flag_bit_0 = effect_bit_0
    backing.notebook_tip_flag_bit_76 = tip_76
    backing.notebook_tip_flag_bit_12 = tip_12
    backing.pending_world_effect = false
    game.location = { type = "building",
                      id = "hollywood_correctional_facility" }
    assert(Ilene.prepare_scene_20_entry() == "continue")
    return backing.pending_world_effect == true
end
assert(prepare_with(false, true, false), "tip 76 alone must arm Ilene")
assert(prepare_with(true, false, false), "FFFC34 bit 0 alone must arm Ilene")
assert(prepare_with(true, true, false), "both gates set must arm Ilene")
assert(not prepare_with(false, false, true),
       "FFFC30 bit 12 must not satisfy the FFFC38 gate")
for key in pairs(backing) do backing[key] = nil end
game.location = { type = "field_map", id = "scene_20_ilene" }

-- Seed the source facts before the production activity runs.
backing.notebook_tip_flag_bit_76 = true
backing.notebook_tip_flag_bit_77 = true
backing.field_auxiliary_flag_bit_12 = true
backing.field_auxiliary_flag_bit_13 = true

assert(Ilene.complete_scene_20_activity() == "completed")
assert(backing.notebook_tip_flag_bit_76 == false)
assert(backing.notebook_tip_flag_bit_77 == false)
assert(backing.field_auxiliary_flag_bit_12 == true)
assert(backing.field_auxiliary_flag_bit_13 == true)
assert(backing.ilene_met == true)
assert(backing.field_mode_flag_bit_31 == false)
assert(backing.field_mode_effect_flag_bit_0 == false)
assert(backing.field_mode_effect_flag_bit_1 == true)
assert(#spoken == 1)

print("Ilene tests passed: FFFC38 tips clear and FFFC40 facts stay intact")

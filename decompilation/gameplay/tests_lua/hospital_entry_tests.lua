package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local Hospital = require("council_island_hospital")

local file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(file:read("*a"))
file:close()

local shown_text_key
game = {
    location = {
        type = "building",
        id = "downtown_site_12_seattle_general_hospital",
    },
    field = { package_run_status = "none" },
    state = { field_mode_flag_bit_3 = false },
    ui = {
        say = function() error("unexpected hospital page") end,
        prompt_text_key = function(menu_id, text_key, choices)
            assert(menu_id == "hospital_service_offer")
            assert(choices[1] == "services" and choices[2] == "leave")
            shown_text_key = text_key
            return "leave"
        end,
    },
}

assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_text_key == "seattle_general_hospital.entry.default")
assert(script_text.texts[shown_text_key] ~= nil)

game.state.field_mode_flag_bit_3 = true
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_text_key == "seattle_general_hospital.entry.michael_holopix")
-- ROM descriptor 0x110A72 -> text 0x111A7B, byte-exact (0x80 -> "|").
assert(script_text.texts[shown_text_key] ==
    "<You enter Seattle General Hospital.||A woman with the name " ..
    "\"Dr. Haversheen\" stitched on her smock is filling out forms " ..
    "behind the desk.||You recognize her as the doctor from Michael's " ..
    "holopix.>", "hospital holopix variant must match ROM text 0x111A7B")

print("Seattle General Hospital ROM entry branch passed")

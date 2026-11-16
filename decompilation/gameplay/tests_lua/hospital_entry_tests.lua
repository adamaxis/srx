package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local E = require("enums")
local Hospital = require("council_island_hospital")

local file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(file:read("*a"))
file:close()

local speakers_file = assert(io.open(
    "decompilation/gameplay/dialogue/speakers.json", "rb"))
local speakers = Json.decode(speakers_file:read("*a"))
speakers_file:close()

local shown_text_key
local shown_event_id, shown_speaker_id, shown_text_profile
game = {
    location = {
        type = "building",
        id = "downtown_site_12_seattle_general_hospital",
    },
    field = { package_run_status = "none" },
    state = { field_mode_flag_bit_3 = false },
    ui = {
        say = function(event_id, speaker_id, text_profile)
            shown_event_id = event_id
            shown_speaker_id = speaker_id
            shown_text_profile = text_profile
            return true
        end,
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

game.field.package_run_status = E.PackageRunStatus.HANDOFF_DUE
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_event_id == "seattle_package_handoff")
assert(shown_speaker_id == "ivanna_haversheen")
assert(shown_text_profile == "council_island_hospital")
local ivanna_events = speakers.profiles.ivanna_haversheen.events
local handoff_key = ivanna_events.seattle_package_handoff.title_key
assert(handoff_key == "seattle_general_hospital.package_handoff")
assert(script_text.texts[handoff_key] ==
    "<Dr. Haversheen hands you a package.>||Please be careful with it." ..
    "||It's quite important.",
    "Seattle handoff page must match ROM text 0x111B3E")

game.field.package_run_status = E.PackageRunStatus.PAYMENT_DUE
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_event_id == "seattle_package_payment")
assert(shown_speaker_id == "ivanna_haversheen")
local payment_key = ivanna_events.seattle_package_payment.title_key
assert(payment_key == "seattle_general_hospital.package_payment")
assert(script_text.texts[payment_key] ==
    "<Dr. Haversheen takes the package, scratches a few notes on a " ..
    "clipboard, then transfers payment to your credstick.>||Thank you." ..
    "||You have saved a life by delivering the contents of this package.",
    "Seattle payment page must match ROM text 0x111B97")

print("Seattle General Hospital ROM entry branch passed")

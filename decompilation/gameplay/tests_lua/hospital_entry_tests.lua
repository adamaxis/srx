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

local menus_file = assert(io.open(
    "decompilation/gameplay/dialogue/menus.json", "rb"))
local menus = Json.decode(menus_file:read("*a"))
menus_file:close()

local ivanna_events = speakers.profiles.ivanna_haversheen.events
local holopix_gate_fact = "field_mode_flag_bit_" ..
    E.FieldModeFlagBit.SEATTLE_HOLOPIX_RECOGNITION
local shown_text_key
local shown_event_id, shown_speaker_id, shown_text_profile
local shown_prompt_speaker, shown_prompt_profile
local shown_prompt_menu, shown_prompt_choices
local shown_events = {}
local shown_prompts = {}
local prompt_choice = "leave"
game = {
    location = {
        type = "building",
        id = "downtown_site_12_seattle_general_hospital",
    },
    field = { package_run_status = "none" },
    state = {
        [holopix_gate_fact] = false,
        [E.StoryFact.HOLOPIX_SHOWN] = false,
    },
    ui = {
        say = function(event_id, speaker_id, text_profile)
            shown_event_id = event_id
            shown_speaker_id = speaker_id
            shown_text_profile = text_profile
            shown_events[#shown_events + 1] = {
                event_id = event_id,
                speaker_id = speaker_id,
                text_profile = text_profile,
            }
            return true
        end,
        prompt_text_key = function(menu_id, text_key, choices,
                                   speaker_id, text_profile)
            shown_prompt_menu = menu_id
            shown_prompt_choices = choices
            shown_text_key = text_key
            shown_prompt_speaker = speaker_id
            shown_prompt_profile = text_profile
            shown_prompts[#shown_prompts + 1] = {
                menu_id = menu_id,
                text_key = text_key,
                choices = choices,
                speaker_id = speaker_id,
                text_profile = text_profile,
            }
            if type(prompt_choice) == "table" then
                return prompt_choice[menu_id]
            end
            return prompt_choice
        end,
    },
}

local function reset_capture()
    shown_events = {}
    shown_prompts = {}
    shown_prompt_menu = nil
    shown_prompt_choices = nil
end

local function event_text_key(event_id)
    return assert(ivanna_events[event_id],
        "missing Ivanna event " .. event_id).title_key
end

assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_events[1].event_id == "seattle_hospital_entry_default")
assert(shown_events[1].speaker_id == "ivanna_haversheen")
assert(shown_events[1].text_profile == "stoker.motel")
local default_entry_key = event_text_key(shown_events[1].event_id)
assert(default_entry_key == "seattle_general_hospital.entry.default")
assert(script_text.texts[default_entry_key] ==
    "<You enter Seattle General Hospital.||The admittance room is " ..
    "packed with humans and metahumans, each waiting for service.||A " ..
    "woman with the name \"Dr. Haversheen\" stitched on her smock is " ..
    "filling out forms behind the desk.>",
    "Seattle default entry must match ROM text 0x11199C")
assert(shown_prompt_menu == "seattle_hospital_service_offer")
assert(shown_text_key == "seattle_general_hospital.prompt.help")
assert(shown_prompt_choices[1] == "services" and
       shown_prompt_choices[2] == "leave")
assert(shown_prompt_speaker == "ivanna_haversheen")
assert(shown_prompt_profile == "stoker.motel")
assert(script_text.texts[shown_text_key] == "May I help you?",
    "Seattle service prompt must match ROM text 0x111C5A")
local stoker_menus = menus.profiles["stoker.motel"]
local service_choices = stoker_menus.seattle_hospital_service_offer.
    choice_label_keys
assert(service_choices.services == "seattle_general_hospital.option.services")
assert(service_choices.leave == "seattle_general_hospital.option.leave")
local holopix_choices = stoker_menus.seattle_hospital_holopix_offer.
    choice_label_keys
assert(holopix_choices.show_holopix ==
    "seattle_general_hospital.option.holopix")
assert(holopix_choices.services == "seattle_general_hospital.option.services")
assert(holopix_choices.leave == "seattle_general_hospital.option.leave")
assert(script_text.texts[holopix_choices.show_holopix] ==
    "Is this your holopix?", "Seattle holopix option must match ROM 0x103D3D")
assert(script_text.texts[service_choices.services] ==
    "I am in need of your services.", "Seattle services option must match ROM 0x1044F2")
assert(script_text.texts[service_choices.leave] ==
    "Neg on that, chum.", "Seattle leave option must match ROM 0x105101")
assert(speakers.profiles.ivanna_haversheen.portrait_alias ==
    "dialogue.portraits.ivanna.haversheen")
assert(shown_events[2].event_id == "seattle_hospital_farewell")
assert(script_text.texts[event_text_key(shown_events[2].event_id)] ==
    "As you wish.||Be careful out there.",
    "Seattle farewell must match ROM text 0x112323")

-- Default service selection reaches the existing Hospital Options task.
reset_capture()
prompt_choice = "services"
assert(Hospital.present_field_exit_prompt() == "hospital-options")
assert(shown_events[1].event_id == "seattle_hospital_entry_default")
assert(shown_events[2].event_id == "seattle_hospital_services")
assert(script_text.texts[event_text_key(shown_events[2].event_id)] ==
    "Please step this way.", "Seattle services page must match ROM 0x11191E")

game.state[holopix_gate_fact] = true
reset_capture()
prompt_choice = "leave"
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_events[1].event_id ==
    "seattle_hospital_entry_michael_holopix")
assert(shown_events[1].speaker_id == "ivanna_haversheen")
assert(shown_events[1].text_profile == "stoker.motel")
local holopix_entry_key = event_text_key(shown_events[1].event_id)
assert(holopix_entry_key ==
    "seattle_general_hospital.entry.michael_holopix")
assert(shown_prompt_menu == "seattle_hospital_holopix_offer")
assert(shown_text_key == "seattle_general_hospital.prompt.help")
assert(shown_prompt_choices[1] == "show_holopix" and
       shown_prompt_choices[2] == "services" and
       shown_prompt_choices[3] == "leave")
assert(shown_prompt_speaker == "ivanna_haversheen")
assert(shown_prompt_profile == "stoker.motel")
-- ROM descriptor 0x110A72 -> text 0x111A7B, byte-exact (0x80 -> "|").
assert(script_text.texts[holopix_entry_key] ==
    "<You enter Seattle General Hospital.||A woman with the name " ..
    "\"Dr. Haversheen\" stitched on her smock is filling out forms " ..
    "behind the desk.||You recognize her as the doctor from Michael's " ..
    "holopix.>", "hospital holopix variant must match ROM text 0x111A7B")
assert(shown_events[2].event_id == "seattle_hospital_farewell")
assert(game.state[holopix_gate_fact] == true)
assert(game.state[E.StoryFact.HOLOPIX_SHOWN] == false)

-- Both non-holopix choices preserve their ROM outcomes and do not set facts.
reset_capture()
prompt_choice = "services"
assert(Hospital.present_field_exit_prompt() == "hospital-options")
assert(shown_events[2].event_id == "seattle_hospital_services")
assert(game.state[holopix_gate_fact] == true)
assert(game.state[E.StoryFact.HOLOPIX_SHOWN] == false)

-- Byte-exact ROM text for the Stark and cyber-heart pages (0x80 -> "|").
local rom_texts = {
    { key = "seattle_general_hospital.option.ask_stark", address = 0x104545,
      text = "What was Michael's relation with Stark?" },
    { key = "seattle_general_hospital.option.later", address = 0x104FEB,
      text = "I'll talk to you later, chum." },
    { key = "seattle_general_hospital.option.stark_found", address = 0x10417A,
      text = "I've found Stark, but he's in real bad shape!" },
    { key = "seattle_general_hospital.option.waiting_room", address = 0x111F8A,
      text = "I'll be in the waiting room when you are finished." },
    { key = "seattle_general_hospital.option.whatever_it_takes", address = 0x112140,
      text = "I'll do whatever it takes, legal or not, to save Stark." },
    { key = "seattle_general_hospital.stark.best_friend", address = 0x111DD0,
      text = "Stark was your brother's best friend.||They went everywhere together.||I can't count the times one of them came in here dragging the other, barely alive.||I'd guess they died together." },
    { key = "seattle_general_hospital.stark.alive", address = 0x111E89,
      text = "Stark?  Alive?!!||Where is he?||What's happened to him?" },
    { key = "seattle_general_hospital.stark.rescued", address = 0x111EC1,
      text = "<You quickly explain how he is barely alive in the wilderness and why you couldn't move him.||She grabs her medical bag and commandeers a crash wagon.||Within the hour you have Stark at the hospital.>" },
    { key = "seattle_general_hospital.stark.examined", address = 0x111FBD,
      text = "<After a long examination Ivanna meets with you.>||We have him on life support, but he was in such bad shape for so long it has taken a deadly toll.||Stark's heart is failing rapidly!" },
    { key = "seattle_general_hospital.stark.hope", address = 0x112075,
      text = "Preliminary tests show that, because of his heavily cybered body, a transplanted heart has very little chance of success.||But there is still hope.||Hope in the form of your abilities as a shadowrunner." },
    { key = "seattle_general_hospital.stark.request", address = 0x112178,
      text = "I understand that, but it is still not an easy thing for me to ask.||My contacts in the medical world tell me that Fuchi has just completed a prototype cyber-heart." },
    { key = "seattle_general_hospital.stark.cyberheart", address = 0x11221D,
      text = "That heart would be strong enough to power Stark's body!||It is being stored at the Fuchi building in the Penumbra District, awaiting clearance orders from Japan.||If you could get your hands on that heart before it ships we stand a good chance of saving Stark." },
    { key = "seattle_general_hospital.heart.missing", address = 0x112347,
      text = "Did you get the heart yet?||Please hurry, Joshua.||Stark is dying!" },
    { key = "seattle_general_hospital.heart.received", address = 0x11238A,
      text = "You have the heart!||You are amazing, Joshua!!||<She rains kisses all over your cheeks and lips, then quickly composes herself, taking the package.>||I'll get him into O.R. right away." },
    { key = "seattle_general_hospital.heart.recovered", address = 0x112443,
      text = "<You return much later to check on Stark.>||Joshua!||The heart works just wiz!!||Too good, in fact." },
    { key = "seattle_general_hospital.heart.walked_out", address = 0x1124A7,
      text = "Stark was in Recovery scant hours before he just got up and walked right out of here like one of those tri-vid stars!||Said he had too much to do to lay around in a hospital." },
    { key = "seattle_general_hospital.heart.underground_93", address = 0x112556,
      text = "He did say, though, that if you needed to find him, he would eventually be at Underground 93 in Puyallup Barrens." },
    { key = "seattle_general_hospital.michael.remembrance", address = 0x111D21,
      text = "Michael was a good friend to me.||We met when he first arrived here three years ago.||I can't believe he is gone.||If there is anything I can do for you, Joshua, let me know." },
}
for _, row in ipairs(rom_texts) do
    assert(script_text.texts[row.key] == row.text,
        string.format("%s must match ROM text 0x%06X", row.key, row.address))
end

-- Holopix (command 0x3E at 0x21EF8) -> page 0x111006 -> page 0x11101A,
-- whose single label 0x104545 runs command 0x61 at 0x22648.
local stark_found_fact = E.StoryFact.MEDICINE_LODGE_TURNED_AWAY
local stark_admitted_fact = E.StoryFact.HOSPITAL_VISITED
local heart_fact = "group_item." .. E.GroupItemId.PROTOTYPE_CYBERHEART
local function effect_bit(bit) return "field_mode_effect_flag_bit_" .. bit end
local function tip_bit(bit) return "notebook_tip_flag_bit_" .. bit end
local stark_runner_fact = "runner_information_bit_" ..
    E.NotebookShadowrunnerBit.STARK
assert(E.FieldModeEffectFlagBit.STARK_DYING_TIP == 18 and
       E.FieldModeEffectFlagBit.STARK_HEART_FAILING_TIP == 19 and
       E.FieldModeEffectFlagBit.CYBERHEART_CARRIED_TIP == 20,
    "Stark tips are FFFC34 bits 18/19/20 (0x1F8B4/0x1F8B8/0x1F918)")
assert(E.NotebookTipFlagBit.FUCHI_HEART_TRANSPORT_MEMO == 64 + 2 and
       E.NotebookTipFlagBit.FUCHI_HEART_STORAGE_MEMO == 64 + 3 and
       E.NotebookTipFlagBit.FUCHI_HEART_ESCORT_MEMO == 64 + 11,
    "heart memos are FFFC38 bits 2/3/11 (0x1F928..0x1F930)")
assert(E.NotebookShadowrunnerBit.STARK == 9, "Stark is FFFC2A bit 9 (0x1F908)")

local function seed_stark_state(values)
    game.state[holopix_gate_fact] = values.recognition or false
    game.state[E.StoryFact.HOLOPIX_SHOWN] = values.holopix or false
    game.state[stark_found_fact] = values.stark_found or false
    game.state[stark_admitted_fact] = values.stark_admitted or false
    game.state[E.StoryFact.HEART_DELIVERED] = values.heart_delivered or false
    game.state[heart_fact] = values.heart or false
    game.state[effect_bit(18)] = true
    game.state[effect_bit(19)] = false
    game.state[effect_bit(20)] = true
    game.state[tip_bit(66)] = true
    game.state[tip_bit(67)] = true
    game.state[tip_bit(75)] = true
    game.state[stark_runner_fact] = false
end

local function event_ids()
    local ids = {}
    for index, event in ipairs(shown_events) do
        ids[index] = event.event_id
        assert(event.speaker_id == "ivanna_haversheen" and
               event.text_profile == "stoker.motel",
            "every 0x1F830 page uses Ivanna's 0x121E68 portrait")
    end
    return table.concat(ids, ",")
end

local function prompt_trace()
    local rows = {}
    for index, prompt in ipairs(shown_prompts) do
        rows[index] = prompt.menu_id .. ":" .. prompt.text_key .. ":" ..
            table.concat(prompt.choices, "/")
        assert(prompt.speaker_id == "ivanna_haversheen" and
               prompt.text_profile == "stoker.motel")
    end
    return table.concat(rows, ",")
end

local rescue_events = "seattle_hospital_stark_alive," ..
    "seattle_hospital_stark_examined,seattle_hospital_stark_request"
local rescue_prompts =
    "seattle_hospital_stark_waiting_room:" ..
        "seattle_general_hospital.stark.rescued:wait," ..
    "seattle_hospital_stark_whatever_it_takes:" ..
        "seattle_general_hospital.stark.hope:agree," ..
    "seattle_hospital_closing_offer:" ..
        "seattle_general_hospital.stark.cyberheart:services/later"
local rescue_choices = {
    seattle_hospital_stark_found = "stark_found",
    seattle_hospital_stark_waiting_room = "wait",
    seattle_hospital_stark_whatever_it_takes = "agree",
    seattle_hospital_closing_offer = "later",
}

-- 0x61 with FFFC1E bit 24 clear: page 0x111042 (text 0x111DD0), no writes.
seed_stark_state({ recognition = true })
reset_capture()
prompt_choice = {
    seattle_hospital_holopix_offer = "show_holopix",
    seattle_hospital_stark_question = "ask_stark",
    seattle_hospital_closing_offer = "later",
}
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(game.state[holopix_gate_fact] == false)
assert(game.state[E.StoryFact.HOLOPIX_SHOWN] == true)
assert(event_ids() == "seattle_hospital_entry_michael_holopix," ..
    "seattle_hospital_holopix_shown,seattle_hospital_farewell", event_ids())
assert(prompt_trace() ==
    "seattle_hospital_holopix_offer:seattle_general_hospital.prompt.help:" ..
        "show_holopix/services/leave," ..
    "seattle_hospital_stark_question:" ..
        "seattle_general_hospital.michael.remembrance:ask_stark," ..
    "seattle_hospital_closing_offer:" ..
        "seattle_general_hospital.stark.best_friend:services/later",
    prompt_trace())
assert(game.state[stark_admitted_fact] == false and
       game.state[effect_bit(18)] == true and
       game.state[effect_bit(19)] == false,
    "0x22652 branch to 0x111042 writes nothing")
assert(script_text.texts[event_text_key("seattle_hospital_holopix_shown")] ==
    "<You show her the holopix and tell her of Michael's death.||" ..
    "Tears stream freely, but she manages to stifle most of the sobs.||" ..
    "Composing herself with much effort she begins to speak.>",
    "holopix response must match ROM text 0x111C6A")

-- The 0x111042 services label runs command 0x3D.
seed_stark_state({ recognition = true })
reset_capture()
prompt_choice = {
    seattle_hospital_holopix_offer = "show_holopix",
    seattle_hospital_stark_question = "ask_stark",
    seattle_hospital_closing_offer = "services",
}
assert(Hospital.present_field_exit_prompt() == "hospital-options")
assert(shown_events[#shown_events].event_id == "seattle_hospital_services")

-- 0x61 with bit 24 set: 0x2265E..0x2267C writes, page 0x11102E, then the
-- 0x11106E rescue chain.
seed_stark_state({ recognition = true, stark_found = true })
reset_capture()
prompt_choice = {
    seattle_hospital_holopix_offer = "show_holopix",
    seattle_hospital_stark_question = "ask_stark",
}
for menu_id, choice in pairs(rescue_choices) do prompt_choice[menu_id] = choice end
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(event_ids() == "seattle_hospital_entry_michael_holopix," ..
    "seattle_hospital_holopix_shown," .. rescue_events ..
    ",seattle_hospital_farewell", event_ids())
assert(prompt_trace() ==
    "seattle_hospital_holopix_offer:seattle_general_hospital.prompt.help:" ..
        "show_holopix/services/leave," ..
    "seattle_hospital_stark_question:" ..
        "seattle_general_hospital.michael.remembrance:ask_stark," ..
    "seattle_hospital_stark_found:" ..
        "seattle_general_hospital.stark.best_friend:stark_found," ..
    rescue_prompts, prompt_trace())
assert(game.state[effect_bit(18)] == false and
       game.state[effect_bit(19)] == true and
       game.state[stark_admitted_fact] == true,
    "command 0x61 clears FFFC34 bit 18, sets 19 and story bit 25")

-- Entry selector 0x1F890: bits 23+24 set, 25 clear -> 0x1F8AE writes and
-- page 0x11105A (text 0x11199C, label 0x10417A).
seed_stark_state({ holopix = true, stark_found = true })
reset_capture()
prompt_choice = rescue_choices
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(event_ids() == rescue_events .. ",seattle_hospital_farewell",
    event_ids())
assert(prompt_trace() ==
    "seattle_hospital_stark_found:seattle_general_hospital.entry.default:" ..
        "stark_found," .. rescue_prompts, prompt_trace())
assert(game.state[effect_bit(18)] == false and
       game.state[effect_bit(19)] == true and
       game.state[stark_admitted_fact] == true)

-- Bit 23 without 24 (and 24 without 23) keeps the default page 0x110A32.
for _, seed in ipairs({ { holopix = true }, { stark_found = true } }) do
    seed_stark_state(seed)
    reset_capture()
    prompt_choice = "leave"
    assert(Hospital.present_field_exit_prompt() == "field_return")
    assert(event_ids() ==
        "seattle_hospital_entry_default,seattle_hospital_farewell")
    assert(game.state[stark_admitted_fact] == false)
end

-- 0x1F8E4 without the heart (FFFC22 bit 0 clear): page 0x110A46, no writes.
seed_stark_state({ holopix = true, stark_found = true,
                   stark_admitted = true })
reset_capture()
prompt_choice = { seattle_hospital_closing_offer = "later" }
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(event_ids() == "seattle_hospital_farewell", event_ids())
assert(prompt_trace() == "seattle_hospital_closing_offer:" ..
    "seattle_general_hospital.heart.missing:services/later", prompt_trace())
assert(game.state[E.StoryFact.HEART_DELIVERED] == false and
       game.state[effect_bit(20)] == true and
       game.state[stark_runner_fact] == false)

-- 0x1F8F8..0x1F94A with the heart.
seed_stark_state({ holopix = true, stark_found = true,
                   stark_admitted = true, heart = true })
reset_capture()
prompt_choice = { seattle_hospital_closing_offer = "services" }
assert(Hospital.present_field_exit_prompt() == "hospital-options")
assert(event_ids() == "seattle_hospital_heart_received," ..
    "seattle_hospital_stark_recovered,seattle_hospital_stark_walked_out," ..
    "seattle_hospital_services", event_ids())
assert(prompt_trace() == "seattle_hospital_closing_offer:" ..
    "seattle_general_hospital.heart.underground_93:services/later",
    prompt_trace())
assert(game.state[heart_fact] == false, "0x1F8F8 consumes the heart")
assert(game.state[stark_runner_fact] == true, "0x1F908 sets FFFC2A bit 9")
assert(game.state[effect_bit(20)] == false, "0x1F918 clears FFFC34 bit 20")
assert(game.state[tip_bit(66)] == false and
       game.state[tip_bit(67)] == false and
       game.state[tip_bit(75)] == false,
    "0x1F928..0x1F930 clear FFFC38 bits 2/3/11")
assert(game.state[effect_bit(18)] == true and
       game.state[effect_bit(19)] == false,
    "the turn-in does not touch FFFC34 bits 18/19")
assert(game.state[E.StoryFact.HEART_DELIVERED] == true,
    "0x1F940 sets FFFC1E bit 29")

-- Bit 29 set (0x1F8E8 bne 0x1F8DA): back to the default page.
reset_capture()
prompt_choice = "leave"
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(event_ids() ==
    "seattle_hospital_entry_default,seattle_hospital_farewell", event_ids())

for _, event_id in ipairs({ "seattle_hospital_stark_alive",
        "seattle_hospital_stark_examined", "seattle_hospital_stark_request",
        "seattle_hospital_heart_received", "seattle_hospital_stark_recovered",
        "seattle_hospital_stark_walked_out" }) do
    assert(ivanna_events[event_id].continuation_key == "shared.more",
        event_id .. " is a < MORE > page (label 0xFE5BF)")
end
local ivanna_menus = menus.profiles["stoker.motel"]
local menu_labels = {
    seattle_hospital_closing_offer = {
        services = "seattle_general_hospital.option.services",
        later = "seattle_general_hospital.option.later",
    },
    seattle_hospital_stark_question = {
        ask_stark = "seattle_general_hospital.option.ask_stark",
    },
    seattle_hospital_stark_found = {
        stark_found = "seattle_general_hospital.option.stark_found",
    },
    seattle_hospital_stark_waiting_room = {
        wait = "seattle_general_hospital.option.waiting_room",
    },
    seattle_hospital_stark_whatever_it_takes = {
        agree = "seattle_general_hospital.option.whatever_it_takes",
    },
}
for menu_id, labels in pairs(menu_labels) do
    for choice_id, label_key in pairs(labels) do
        assert(ivanna_menus[menu_id].choice_label_keys[choice_id] ==
            label_key, menu_id .. "." .. choice_id)
    end
end

seed_stark_state({})
prompt_choice = "leave"

game.field.package_run_status = E.PackageRunStatus.HANDOFF_DUE
reset_capture()
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_event_id == "seattle_package_handoff")
assert(shown_speaker_id == "ivanna_haversheen")
assert(shown_text_profile == "council_island_hospital")
local handoff_key = ivanna_events.seattle_package_handoff.title_key
assert(handoff_key == "seattle_general_hospital.package_handoff")
assert(script_text.texts[handoff_key] ==
    "<Dr. Haversheen hands you a package.>||Please be careful with it." ..
    "||It's quite important.",
    "Seattle handoff page must match ROM text 0x111B3E")

game.field.package_run_status = E.PackageRunStatus.PAYMENT_DUE
reset_capture()
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

game.location.id = "council_island_hospital"
game.field.package_run_status = E.PackageRunStatus.NONE
reset_capture()
assert(Hospital.present_field_exit_prompt() == "field_return")
assert(shown_text_key == "council_island_hospital.entry.default")
assert(shown_prompt_speaker == nil and shown_prompt_profile == nil,
    "other hospitals must keep the existing narrator prompt")
assert(shown_events[1] == nil,
    "other hospitals must keep the existing inline narrator prompt")

print("Seattle General Hospital ROM entry branch passed")

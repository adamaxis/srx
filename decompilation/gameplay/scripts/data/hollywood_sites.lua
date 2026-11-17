local E = require("enums")

return {
    hollywood_correctional_facility = {
        speaker = E.DialogueSpeaker.HOLLYWOOD_CORRECTIONAL_GUARD,
        sarge_speaker = E.DialogueSpeaker.HOLLYWOOD_CORRECTIONAL_SARGE,
        text_profile = E.DialogueTextProfile.FIELD_WORLD_EVENT,
        events = {
            entrance = "hollywood_gate_entrance",
            package_handoff = "hollywood_gate_package_handoff",
            package_payment = "hollywood_gate_package_payment",
            sarge_pass = "hollywood_gate_sarge_pass",
            false_name_pass = "hollywood_gate_false_name_pass",
            story_first = "hollywood_gate_story_first",
            farewell = "hollywood_gate_farewell",
            attack_missed = "hollywood_gate_attack_missed",
            attack_success = "hollywood_gate_attack_success",
            attack_ambush = "hollywood_gate_attack_ambush",
        },
        menus = {
            guard = "hollywood_gate_guard_choices",
            story = "hollywood_gate_story_choices",
        },
        prompts = {
            guard = "hollywood.gate.guard.opening",
            story = "hollywood.gate.story.second",
        },
    },
}

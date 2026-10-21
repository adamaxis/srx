-- Speaker properties are supplied by gameplay content and resolved from the
-- speaker id carried by the active map/location context.
local Dialogue = {}
local E = require("enums")

local speakers = {
    field_narrator = {
        name = "Narration",
        text_profile = "field.world_event",
        portrait_alias = "none",
        events = {
            ["redmond.vigore-search.page-0"] = {
                title_key = "redmond.vigore-search.page-0",
                continuation_key = "shared.more",
            },
            ["redmond.vigore-search.page-1"] = {
                title_key = "redmond.vigore-search.page-1",
                continuation_key = "shared.more",
            },
            ["redmond.vigore-search.page-2"] = {
                title_key = "redmond.vigore-search.page-2",
                continuation_key = "",
            },
            phone_no_contacts = {
                title_key = "field.main_menu.vid_phone.no_contacts",
                continuation_key = "",
            },
            cyberspace_no_deck = {
                title_key = "field.cyberspace.no_deck",
                continuation_key = "",
            },
            cyberspace_no_mpcp = {
                title_key = "field.cyberspace.no_mpcp",
                continuation_key = "",
            },
            cyberspace_no_datajack = {
                title_key = "field.cyberspace.no_datajack",
                continuation_key = "",
            },
        },
    },
    tarislar_apartments_narrator = {
        name = "Narration",
        text_profile = "tarislar.garden.apartments.entry",
        portrait_alias = "none",
        events = {},
    },
    tarislar_city_inn_narrator = {
        name = "Narration",
        text_profile = "tarislar.city_inn",
        portrait_alias = "none",
        events = {},
    },
    wylie_gala_inn_narrator = {
        name = "Narration",
        text_profile = "penumbra.wylies_gala_inn",
        portrait_alias = "none",
        events = {
            free_stay = {
                title_key = "wylie.gala_inn.free_stay",
                continuation_key = "",
            },
            package_after = {
                title_key = "wylie.gala_inn.package_after",
                continuation_key = "",
            },
            package_before = {
                title_key = "wylie.gala_inn.package_before",
                continuation_key = "",
            },
            room_paid = {
                title_key = "stoker.motel.room.paid.title",
                continuation_key = "",
            },
            room_insufficient = {
                title_key = "stoker.motel.room.insufficient.title",
                continuation_key = "",
            },
        },
    },
    david_owlfeather = {
        name = "David Owlfeather",
        text_profile = "salish.owlfeather",
        portrait_alias = "dialogue.portraits.david.owlfeather",
        events = {},
    },
    red_buffalo_woman = {
        name = "Red Buffalo Woman",
        text_profile = "salish.red_buffalo",
        portrait_alias = "dialogue.portraits.red.buffalo.woman",
        events = {},
    },
    spirit_eyes = {
        name = "Spirit Eyes",
        text_profile = "salish.spirit_eyes",
        portrait_alias = "dialogue.portraits.spirit.eyes",
        events = {},
    },
    licourtrix = {
        name = "Licourtrix",
        text_profile = "salish.spirit_eyes",
        portrait_alias = "dialogue.portraits.licourtrix",
        events = {},
    },
    renraku_offices_narrator = {
        name = "Narration",
        text_profile = "renraku.offices",
        portrait_alias = "dialogue.portraits.renraku.offices.entry",
        events = {
            entry = {
                title_key = "renraku.offices.page.entry",
                continuation_key = "shared.more",
            },
        },
    },
    fuchi_offices_narrator = {
        name = "Narration",
        text_profile = "penumbra.fuchi",
        portrait_alias = "dialogue.portraits.fuchi.offices.entry",
        events = {
            entry = {
                title_key = "penumbra.fuchi.page.entry",
                continuation_key = "shared.more",
            },
        },
    },
    gates_undersound_narrator = {
        name = "Narration",
        text_profile = "gates.undersound",
        portrait_alias = "dialogue.portraits.gates.undersound.entry",
        events = {
            entry = {
                title_key = "gates.undersound.page.entry",
                continuation_key = "",
            },
        },
    },
    boris_greenhouse_narrator = {
        name = "Boris",
        text_profile = "boris.greenhouse",
        portrait_alias = "dialogue.portraits.boris.greenhouse",
        events = {
            package_request = {
                title_key = "boris.greenhouse.page.alternate_1",
                continuation_key = "",
            },
            package_payment = {
                title_key = "boris.greenhouse.page.alternate_2",
                continuation_key = "",
            },
            insufficient_funds = {
                title_key = "boris.greenhouse.page.insufficient_funds",
                continuation_key = "",
            },
        },
    },
    council_island_hospital_narrator = {
        name = "Narration",
        text_profile = "council_island_hospital",
        portrait_alias = "none",
        events = {
            package_handoff = {
                title_key = "council_island_hospital.entry.package_handoff",
                continuation_key = "",
            },
            package_payment = {
                title_key = "council_island_hospital.entry.package_payment",
                continuation_key = "",
            },
            medical_attention_not_needed = {
                title_key = "hospital.medical.not_needed",
                continuation_key = "",
            },
            medical_attention_insufficient_funds = {
                title_key = "hospital.medical.insufficient_funds",
                continuation_key = "",
            },
        },
    },
    council_medicine_lodge_narrator = {
        name = "Narration",
        text_profile = "council_medicine_lodge",
        portrait_alias = "none",
        events = {
            package_handoff = {
                title_key = "council_medicine_lodge.entry.package_handoff",
                continuation_key = "",
            },
            package_payment = {
                title_key = "council_medicine_lodge.entry.package_payment",
                continuation_key = "",
            },
            hire_shaman_response = {
                title_key = "council_medicine_lodge.response.hire_shaman",
                continuation_key = "",
            },
            farewell = {
                title_key = "council_medicine_lodge.response.farewell",
                continuation_key = "",
            },
            wise_one_followup = {
                title_key = "council_medicine_lodge.response.wise_one",
                continuation_key = "",
            },
        },
    },
    jump_house_entry = {
        name = "The Jump House",
        text_profile = "jump_house.entry",
        portrait_alias = "dialogue.portraits.jump.house.entry",
        events = {
            entry = {
                title_key = "jump-house.entry",
                continuation_key = "",
            },
        },
    },
    gunderson = {
        name = "Gunderson",
        text_profile = "gunderson.jump_house",
        portrait_alias = "dialogue.portraits.gunderson.jump.house",
        events = {
            ["information-opening"] = {
                title_key = "gunderson.information.opening",
                continuation_key = "shared.more",
            },
            ["work-opening"] = {
                title_key = "gunderson.work.opening",
                continuation_key = "shared.more",
            },
            ["johnson-definition"] = {
                title_key = "gunderson.question.answer",
                continuation_key = "shared.more",
            },
            ["vigore-alternate-response"] = {
                title_key = "gunderson.question.vigore.alternate_response",
                continuation_key = "shared.more",
            },
            ["contacts-introduction"] = {
                title_key = "gunderson.contacts.introduction",
                continuation_key = "shared.more",
            },
        },
    },
    mortimer_reed = {
        name = "Mortimer Reed",
        text_profile = "big-rhino.mortimer_reed",
        portrait_alias = "dialogue.portraits.mortimer.reed",
        events = {
            booth_entry = {
                title_key = "big-rhino.booth.entry",
                continuation_key = "shared.more",
            },
            repeat_booth_entry = {
                title_key = "big-rhino.mortimer.repeat-booth-entry",
                continuation_key = "shared.more",
            },
            opening = {
                title_key = "big-rhino.mortimer.opening",
                continuation_key = "",
            },
            repeat_opening = {
                title_key = "big-rhino.mortimer.repeat-opening",
                continuation_key = "",
            },
            conditional_opening = {
                title_key = "big-rhino.mortimer.conditional-opening",
                continuation_key = "",
            },
            work_opening = {
                title_key = "big-rhino.work.opening",
                continuation_key = "shared.more",
            },
            information_opening = {
                title_key = "big-rhino.information.opening",
                continuation_key = "shared.more",
            },
            question_default = {
                title_key = "big-rhino.information.question.default",
                continuation_key = "shared.more",
            },
            leave = {
                title_key = "big-rhino.leave",
                continuation_key = "",
            },
        },
    },
    julius_strouther = {
        name = "Julius Strouther",
        text_profile = "matchstick.julius_strouther",
        -- Scene 18's row at 0x12CC60 selects portrait profile 0x121EBC.
        -- The same extracted portrait is used by the Jump House entry page.
        portrait_alias = "dialogue.portraits.jump.house.entry",
        events = {
            booth_entry = {
                title_key = "julius-strouther.booth-entry",
                continuation_key = "shared.more",
            },
            work_opening = {
                title_key = "julius-strouther.work.opening",
                continuation_key = "shared.more",
            },
            contacts_opening = {
                title_key = "julius-strouther.contacts.opening",
                continuation_key = "shared.more",
            },
            question_default = {
                title_key = "julius-strouther.question.default",
                continuation_key = "shared.more",
            },
            leave = {
                title_key = "julius-strouther.leave",
                continuation_key = "",
            },
        },
    },
    vigore_jarl = {
        name = "Vigore and Jarl",
        text_profile = "wanderer.vigore_jarl",
        portrait_alias = "dialogue.portraits.vigore.jarl",
        events = {
            first_impression = {
                title_key = "wanderer.vigore_jarl.first-impression",
                continuation_key = "shared.more",
            },
            first_meeting = {
                title_key = "wanderer.vigore_jarl.first-meeting",
                continuation_key = "shared.more",
            },
            conditional_opening = {
                title_key = "wanderer.vigore_jarl.conditional-opening",
                continuation_key = "",
            },
            booth_vacated = {
                title_key = "wanderer.vigore_jarl.booth-vacated",
                continuation_key = "",
            },
        },
    },
    wanderer_booth_narrator = {
        name = "Narration",
        text_profile = "wanderer.vigore_jarl",
        portrait_alias = "none",
        events = {
            booth_vacated = {
                title_key = "wanderer.vigore_jarl.booth-vacated",
                continuation_key = "",
            },
        },
    },
    caleb_brightmore = {
        name = "Caleb Brightmore",
        text_profile = "icarus-descending.caleb_brightmore",
        portrait_alias = "dialogue.portraits.caleb.brightmore",
        events = {
            access_blocked = {
                title_key = "icarus-descending.access.blocked",
                continuation_key = "",
            },
            first_meeting = {
                title_key = "icarus-descending.caleb.first-meeting",
                continuation_key = "shared.more",
            },
            first_meeting_continuation = {
                title_key = "icarus-descending.caleb.first-meeting.continuation",
                continuation_key = "",
            },
            opening = {
                title_key = "icarus-descending.caleb.opening",
                continuation_key = "",
            },
            post_run_opening = {
                title_key = "icarus-descending.caleb.post-run-opening",
                continuation_key = "",
            },
            information_contacts_offer = {
                title_key = "icarus-descending.caleb.information.contacts-offer",
                continuation_key = "",
            },
            work_opening = {
                title_key = "icarus-descending.caleb.work.opening",
                continuation_key = "",
            },
            work_renraku_introduction = {
                title_key = "icarus-descending.caleb.work.renraku-introduction",
                continuation_key = "",
            },
            work_renraku_followup = {
                title_key = "icarus-descending.caleb.work.renraku-followup",
                continuation_key = "",
            },
            work_recommended_run = {
                title_key = "icarus-descending.caleb.work.recommended-run",
                continuation_key = "",
            },
            work_matrix_requirement = {
                title_key = "icarus-descending.caleb.work.matrix-requirement",
                continuation_key = "",
            },
            leave = {
                title_key = "icarus-descending.caleb.leave",
                continuation_key = "",
            },
            transaction = {
                title_key = "icarus-descending.caleb.transaction",
                continuation_key = "",
            },
        },
    },
    jackals_lantern_old_man = {
        name = "Unidentified old man",
        text_profile = "jackals_lantern.old_man",
        portrait_alias = "dialogue.portraits.jackals.lantern.old.man",
        events = {
            opening = {
                title_key = "jackals-lantern.old-man.opening",
                continuation_key = "",
            },
        },
    },
    stoker = {
        name = "Stoker",
        text_profile = "stoker.motel",
        portrait_alias = "dialogue.portraits.stoker.motel",
        events = {
            arrival = {
                title_key = "stoker.motel.arrival.title",
                continuation_key = "stoker.motel.arrival.more",
            },
            leave = {
                title_key = "stoker.motel.leave.title",
                continuation_key = "",
            },
            ["street-ambush"] = {
                title_key = "stoker.motel.street-ambush.title",
                continuation_key = "",
            },
            ["package-question"] = {
                title_key = "stoker.motel.package-question.title",
                continuation_key = "",
            },
            ["room.paid"] = {
                title_key = "stoker.motel.room.paid.title",
                continuation_key = "",
            },
            ["room.insufficient"] = {
                title_key = "stoker.motel.room.insufficient.title",
                continuation_key = "",
            },
            ["package-handoff"] = {
                title_key = "stoker.motel.package-handoff.title",
                continuation_key = "",
            },
            ["jump-house"] = {
                title_key = "stoker.motel.jump-house.title",
                continuation_key = "",
            },
        },
    },
}

-- The Vid-Phone list uses the ROM's 13-entry name table at 0xC66AC and the
-- pointer table at 0x109682. Each pointer leads to that contact's call
-- descriptors and portrait profile. Contact access and call history live in
-- FFFC2C; these rows provide readable content metadata.
local vid_phone_contacts = {
    { source_index = 0, id = "capt_quinton_jaymes", name = "Capt Quinton Jaymes", name_key = "pocket_secretary.notebook.contact.name.option.record_0", portrait_alias = "dialogue.portraits.vid.phone.capt.quinton.jaymes", service_module = "vid_phone.capt_quinton_jaymes", service_function = "services" },
    { source_index = 1, id = "alesandro_hobbs", name = "Alesandro Hobbs", name_key = "pocket_secretary.notebook.contact.name.option.record_1", portrait_alias = "dialogue.portraits.vid.phone.alesandro.hobbs", service_module = "vid_phone.alesandro_hobbs", service_function = "services" },
    { source_index = 2, id = "sharkey", name = "Sharkey", name_key = "pocket_secretary.notebook.contact.name.option.record_2", portrait_alias = "dialogue.portraits.vid.phone.sharkey", service_module = "vid_phone.sharkey", service_function = "services" },
    { source_index = 3, id = "kipp_david", name = "Kipp David", name_key = "pocket_secretary.notebook.contact.name.option.record_3", portrait_alias = "dialogue.portraits.vid.phone.kipp.david", service_module = "vid_phone.kipp_david", service_function = "services" },
    { source_index = 4, id = "wilma_temmenhoff", name = "Wilma Temmenhoff", name_key = "pocket_secretary.notebook.contact.name.option.record_4", portrait_alias = "dialogue.portraits.vid.phone.wilma.temmenhoff", service_module = "vid_phone.wilma_temmenhoff", service_function = "services" },
    { source_index = 5, id = "macklemann", name = "Sgt. Macklemann", name_key = "pocket_secretary.notebook.contact.name.option.record_5", portrait_alias = "dialogue.portraits.vid.phone.macklemann", service_module = "vid_phone.macklemann", service_function = "services" },
    { source_index = 6, id = "gregory_wilns", name = "Gregory Wilns", name_key = "pocket_secretary.notebook.contact.name.option.record_6", portrait_alias = "dialogue.portraits.vid.phone.gregory.wilns", service_module = "vid_phone.gregory_wilns", service_function = "services", farewell_event = "goodbye" },
    { source_index = 7, id = "mr_faradouchi", name = "Mr. Faradouchi", name_key = "pocket_secretary.notebook.contact.name.option.record_7", portrait_alias = "dialogue.portraits.vid.phone.mr.faradouchi", always_first = true, first_continuation = true, repeat_continuation = true },
    { source_index = 8, id = "agira_tetsumi", name = "Agira Tetsumi", name_key = "pocket_secretary.notebook.contact.name.option.record_8", portrait_alias = "dialogue.portraits.vid.phone.agira.tetsumi", always_first = true, first_continuation = true, repeat_continuation = true },
    { source_index = 9, id = "henry_j_culver", name = "Henry J. Culver", name_key = "pocket_secretary.notebook.contact.name.option.record_9", portrait_alias = "dialogue.portraits.vid.phone.henry.j.culver" },
    { source_index = 10, id = "max", name = "Max", name_key = "pocket_secretary.notebook.contact.name.option.record_10", portrait_alias = "dialogue.portraits.vid.phone.max" },
    { source_index = 11, id = "chillicut", name = "Sgt. Chillicut", name_key = "pocket_secretary.notebook.contact.name.option.record_11", portrait_alias = "dialogue.portraits.vid.phone.chillicut", first_continuation = true },
    { source_index = 12, id = "professor_jefferson", name = "Prof. Jefferson", name_key = "pocket_secretary.notebook.contact.name.option.record_12", portrait_alias = "dialogue.portraits.vid.phone.professor.jefferson" },
}

for _, contact in ipairs(vid_phone_contacts) do
    local speaker_id = "vid_phone_" .. contact.id
    contact.speaker_id = speaker_id
    speakers[speaker_id] = {
        name = contact.name,
        text_profile = "field.vid_phone",
        portrait_alias = contact.portrait_alias,
        events = {
            first_call = {
                title_key = "vid_phone.contact." .. contact.id .. ".first_call",
                continuation_key = contact.first_continuation and "shared.more" or "",
            },
            repeat_call = {
                title_key = "vid_phone.contact." .. contact.id .. ".repeat_call",
                continuation_key = contact.repeat_continuation and "shared.more" or "",
            },
        },
    }
end

-- Menu copy belongs to the location's content profile. A shared building
-- menu can therefore be shown with or without a named speaker portrait.
local menu_profiles = {
    ["field.main_menu"] = {
        ordinary = {
            title_key = "field.main_menu.title",
            choice_label_keys = {
                call_taxi = "field.main_menu.option.call_taxi",
                vid_phone = "field.main_menu.option.vid_phone",
                cyberspace = "field.main_menu.option.cyberspace",
                exit = "field.main_menu.option.exit",
            },
        },
        cyberspace = {
            title_key = "field.main_menu.cyberspace.title",
            choice_label_keys = {
                enter_passcode = "field.main_menu.cyberspace.option.enter_passcode",
                system_search = "field.main_menu.cyberspace.option.system_search",
                exit = "field.main_menu.cyberspace.option.exit",
            },
        },
        passcodes = {
            title_key = "field.main_menu.passcodes.title",
            layout = "compact_list",
            choice_label_keys = {
                run_passcode = "field.main_menu.passcodes.option.run_passcode",
                aztechnology = "field.main_menu.passcodes.option.aztechnology",
                club_penumbra = "field.main_menu.passcodes.option.club_penumbra",
                seattle_general_hospital = "field.main_menu.passcodes.option.seattle_general_hospital",
                ares = "field.main_menu.passcodes.option.ares",
                gates_undersound = "field.main_menu.passcodes.option.gates_undersound",
                hollywood = "field.main_menu.passcodes.option.hollywood",
                mitsuhama = "field.main_menu.passcodes.option.mitsuhama",
                renraku = "field.main_menu.passcodes.option.renraku",
                fuchi = "field.main_menu.passcodes.option.fuchi",
                lone_star = "field.main_menu.passcodes.option.lone_star",
                ito_passcode = "field.main_menu.passcodes.option.ito_passcode",
                next_screen = "vid_phone.contact.page_next",
                prior_screen = "vid_phone.contact.page_previous",
            },
        },
    },
    ["boris.greenhouse"] = {
        more = {
            choice_label_keys = { more = "shared.more" },
        },
        opening = {
            choice_label_keys = {
                story = "boris.greenhouse.option.story",
                information = "boris.greenhouse.option.information",
                later = "boris.greenhouse.option.later",
            },
        },
        repeat_opening = {
            choice_label_keys = {
                story = "boris.greenhouse.option.story",
                information = "boris.greenhouse.option.information",
                later = "boris.greenhouse.option.later",
            },
        },
        story_payment = {
            choice_label_keys = {
                accept = "boris.greenhouse.option.accept",
                later = "boris.greenhouse.option.decline",
            },
        },
        information_payment = {
            choice_label_keys = {
                accept = "boris.greenhouse.option.accept",
                later = "boris.greenhouse.option.decline",
            },
        },
        information_topics = {
            choice_label_keys = {
                michael = "boris.greenhouse.option.michael",
                david = "boris.greenhouse.option.david",
                ilene = "boris.greenhouse.option.ilene",
                thon = "boris.greenhouse.option.thon",
                later = "boris.greenhouse.option.later",
            },
        },
        david_followup = {
            choice_label_keys = {
                brother = "boris.greenhouse.option.brother",
            },
        },
    },
    ["tarislar.garden.apartments.entry"] = {
        arrival = {
            choice_label_keys = { more = "shared.more" },
        },
        penthouse = {
            choice_label_keys = { more = "shared.more" },
        },
        meeting = {
            choice_label_keys = { more = "shared.more" },
        },
        michael_topic = {
            choice_label_keys = {
                ask_about_michael = "tarislar.garden.apartments.option.ask_about_michael",
            },
        },
        thon_topic = {
            choice_label_keys = {
                who_is_thon = "tarislar.garden.apartments.option.who_is_thon",
            },
        },
        thon_explanation = {
            choice_label_keys = { more = "shared.more" },
        },
        thon_curse = {
            choice_label_keys = { more = "shared.more" },
        },
        renraku_topic = {
            choice_label_keys = {
                why_renraku = "tarislar.garden.apartments.option.why_renraku",
            },
        },
        renraku_explanation = {
            choice_label_keys = { more = "shared.more" },
        },
        ito_response = {
            choice_label_keys = { more = "shared.more" },
        },
        ito_update = {
            choice_label_keys = {
                ito_killed = "tarislar.garden.apartments.option.ito_killed",
            },
        },
        repeat_service = {
            choice_label_keys = {
                questions = "tarislar.garden.apartments.option.questions",
                ito_killed = "tarislar.garden.apartments.option.ito_killed",
                information = "tarislar.garden.apartments.option.information",
            },
        },
        renraku_map = {
            choice_label_keys = { more = "shared.more" },
        },
        renraku_map_meaning = {
            choice_label_keys = { more = "shared.more" },
        },
        vigore_lead = {
            choice_label_keys = { more = "shared.more" },
        },
        vigore_report = {
            choice_label_keys = { more = "shared.more" },
        },
        vigore_explanation = {
            choice_label_keys = { more = "shared.more" },
        },
        both_maps_meaning = {
            choice_label_keys = { more = "shared.more" },
        },
        tomb_artifacts = {
            choice_label_keys = { more = "shared.more" },
        },
        harlequin_resolves = {
            choice_label_keys = { more = "shared.more" },
        },
        tomb_offer = {
            choice_label_keys = {
                yes = "tarislar.garden.apartments.option.tomb_yes",
                no = "tarislar.garden.apartments.option.tomb_no",
            },
        },
        tomb_declined = {
            choice_label_keys = { more = "shared.more" },
        },
        tomb_flight = {
            choice_label_keys = { more = "shared.more" },
        },
        tomb_walkie = {
            choice_label_keys = { more = "shared.more" },
        },
        tomb_good_luck = {
            choice_label_keys = { more = "shared.more" },
        },
    },
    ["council_island_hospital"] = {
        hospital_service_offer = {
            choice_label_keys = {
                services = "council_island_hospital.option.services",
                leave = "council_island_hospital.option.leave",
            },
        },
        hospital_options = {
            title_key = "hospital.options.title",
            choice_label_keys = {
                medical_attention = "hospital.options.medical_attention",
                cyberware = "hospital.options.cyberware",
                exit = "hospital.options.exit",
            },
        },
        hospital_medical_attention = {
            choice_label_keys = {
                level_100 = "hospital.medical.row.100",
                level_90 = "hospital.medical.row.90",
                level_70 = "hospital.medical.row.70",
                level_40 = "hospital.medical.row.40",
            },
        },
        hospital_leave_confirmation = {
            title_key = "hospital.leave.prompt",
            choice_label_keys = {
                yes = "ui.option.yes",
                no = "ui.option.no",
            },
        },
    },
    ["council_medicine_lodge"] = {
        medicine_lodge_service_offer = {
            choice_label_keys = {
                hire_shaman = "council_medicine_lodge.option.hire_shaman",
                browse_store = "council_medicine_lodge.option.browse_store",
                talk_later = "council_medicine_lodge.option.talk_later",
                wise_one = "council_medicine_lodge.option.wise_one",
            },
        },
    },
    ["tarislar.city_inn"] = {
        inn_service = {
            title_key = "tarislar.city_inn.offer",
            choice_label_keys = {
                crash = "tarislar.city_inn.option.crash",
                leave = "tarislar.city_inn.option.leave",
                shale = "tarislar.city_inn.option.shale",
                frosty = "tarislar.city_inn.option.frosty",
            },
        },
        room_payment = {
            title_key = "tarislar.city_inn.room",
            choice_label_keys = {
                pay = "tarislar.city_inn.option.pay",
                decline = "tarislar.city_inn.option.decline",
            },
        },
        shale_arrival = {
            title_key = "tarislar.city_inn.shale.2",
            choice_label_keys = {
                about_michael = "tarislar.city_inn.option.about_michael",
            },
        },
        shale_topics = {
            title_key = "tarislar.city_inn.shale.caleb",
            choice_label_keys = {
                caleb = "tarislar.city_inn.option.caleb",
                errascoe = "tarislar.city_inn.option.errascoe",
            },
        },
        shale_other_topic = {
            title_key = "tarislar.city_inn.shale.errascoe",
            choice_label_keys = {
                caleb = "tarislar.city_inn.option.caleb",
                errascoe = "tarislar.city_inn.option.errascoe",
            },
        },
        shale_followup = {
            title_key = "tarislar.city_inn.shale.caleb_followup",
            choice_label_keys = {
                recruit = "tarislar.city_inn.option.recruit",
                thanks = "tarislar.city_inn.option.thanks",
            },
        },
        shale_more = {
            choice_label_keys = { more = "shared.more" },
        },
    },
    ["penumbra.wylies_gala_inn"] = {
        inn_service = {
            choice_label_keys = {
                crash = "wylie.gala_inn.option.crash",
                leave = "wylie.gala_inn.option.leave",
            },
        },
        room_payment = {
            choice_label_keys = {
                pay = "wylie.gala_inn.option.pay",
                leave = "wylie.gala_inn.option.leave",
            },
        },
    },
    ["salish.owlfeather"] = {
        opening = {
            choice_label_keys = {
                lands = "salish.owlfeather.option.lands",
                council = "salish.owlfeather.option.council",
                brother = "salish.owlfeather.option.brother",
            },
        },
        lands = {
            choice_label_keys = {
                rangers = "salish.owlfeather.option.rangers",
                thanks = "salish.owlfeather.option.thanks",
            },
        },
        rangers = {
            choice_label_keys = {
                villages = "salish.owlfeather.option.villages",
                thanks = "salish.owlfeather.option.thanks",
            },
        },
        villages = {
            choice_label_keys = {
                sinsearach = "salish.owlfeather.option.sinsearach",
                thanks = "salish.owlfeather.option.thanks",
            },
        },
        sinsearach = {
            choice_label_keys = {
                lands = "salish.owlfeather.option.lands",
                council = "salish.owlfeather.option.council",
                thanks = "salish.owlfeather.option.thanks",
            },
        },
        council_more = {
            choice_label_keys = { more = "shared.more" },
        },
        council_topics = {
            choice_label_keys = {
                lands = "salish.owlfeather.option.lands",
                council = "salish.owlfeather.option.council",
                thanks = "salish.owlfeather.option.thanks",
            },
        },
        brother_more = {
            choice_label_keys = { more = "shared.more" },
        },
        brother_status = {
            choice_label_keys = {
                learned = "salish.owlfeather.option.learned",
            },
        },
        status_more = {
            choice_label_keys = { more = "shared.more" },
        },
        status_topic = {
            choice_label_keys = {
                like_who = "salish.owlfeather.option.like_who",
            },
        },
        simple_task = {
            choice_label_keys = {
                task = "salish.owlfeather.option.simple_task",
            },
        },
        task_result = {
            choice_label_keys = {
                lands = "salish.owlfeather.option.lands",
                council = "salish.owlfeather.option.council",
                thanks = "salish.owlfeather.option.thanks",
            },
        },
        search_followup = {
            choice_label_keys = {
                lands = "salish.owlfeather.option.lands",
                council = "salish.owlfeather.option.council",
                later = "salish.owlfeather.option.later",
            },
        },
        return_followup = {
            choice_label_keys = {
                freed = "salish.owlfeather.option.freed",
            },
        },
        return_more = {
            choice_label_keys = { more = "shared.more" },
        },
    },
    ["salish.red_buffalo"] = {
        introduction = {
            choice_label_keys = { more = "shared.more" },
        },
        gift = {
            choice_label_keys = { more = "shared.more" },
        },
        no_gift = {
            choice_label_keys = {
                no_business = "salish.red_buffalo.option.no_business",
                more = "shared.more",
            },
        },
        meeting = {
            choice_label_keys = { more = "shared.more" },
        },
        michael_topic = {
            choice_label_keys = {
                ask_about_michael = "tarislar.garden.apartments.option.ask_about_michael",
            },
        },
        thon_topic = {
            choice_label_keys = {
                who_is_thon = "tarislar.garden.apartments.option.who_is_thon",
            },
        },
        thon_explanation = {
            choice_label_keys = { more = "shared.more" },
        },
        thon_curse = {
            choice_label_keys = { more = "shared.more" },
        },
        renraku_topic = {
            choice_label_keys = {
                why_renraku = "tarislar.garden.apartments.option.why_renraku",
            },
        },
        renraku_explanation = {
            choice_label_keys = { more = "shared.more" },
        },
        ito_update = {
            choice_label_keys = {
                ito_killed = "tarislar.garden.apartments.option.ito_killed",
            },
        },
        ito_response = {
            choice_label_keys = { more = "shared.more" },
        },
        renraku_map = {
            choice_label_keys = { more = "shared.more" },
        },
        renraku_map_meaning = {
            choice_label_keys = { more = "shared.more" },
        },
        vigore_lead = {
            choice_label_keys = { more = "shared.more" },
        },
        vigore_report = {
            choice_label_keys = { more = "shared.more" },
        },
        vigore_explanation = {
            choice_label_keys = { more = "shared.more" },
        },
        both_maps_meaning = {
            choice_label_keys = { more = "shared.more" },
        },
        tomb_artifacts = {
            choice_label_keys = { more = "shared.more" },
        },
        harlequin_resolves = {
            choice_label_keys = { more = "shared.more" },
        },
    },
    ["salish.spirit_eyes"] = {
        first_contact = {
            choice_label_keys = { more = "shared.more" },
        },
        serpent_more = {
            choice_label_keys = { more = "shared.more" },
        },
        serpent_intentions = {
            choice_label_keys = {
                neighborhood = "salish.spirit_eyes.option.neighborhood",
                scale = "salish.spirit_eyes.option.scale",
            },
        },
        serpent_lie = {
            choice_label_keys = {
                neighborhood = "salish.spirit_eyes.option.neighborhood",
                scale = "salish.spirit_eyes.option.scale",
            },
        },
        serpent_thanks = {
            choice_label_keys = {
                thanks = "salish.spirit_eyes.option.thanks",
            },
        },
        serpent_task = {
            choice_label_keys = {
                what = "salish.spirit_eyes.option.what",
            },
        },
    },
    ["stoker.motel"] = {
        manager = {
            title_key = "stoker.motel.manager.title",
            choice_label_keys = {
                crash = "stoker.motel.option.crash",
                leave = "stoker.motel.option.leave",
                package = "stoker.motel.option.package",
            },
        },
        room = {
            title_key = "stoker.motel.room.title",
            choice_label_keys = {
                pay = "stoker.motel.option.pay",
                leave = "stoker.motel.option.leave",
            },
        },
        debt = {
            title_key = "stoker.motel.debt.title",
            choice_label_keys = {
                ["debt-payment"] = "stoker.motel.option.debt-payment",
                ["jump-house"] = "stoker.motel.option.jump-house",
            },
        },
    },
    ["gunderson.jump_house"] = {
        opening = {
            title_key = "gunderson.greeting",
            choice_label_keys = {
                information = "gunderson.option.information",
                work = "gunderson.option.work",
                leave = "gunderson.option.leave",
            },
        },
        opening_after_information = {
            title_key = "gunderson.greeting.after_information",
            choice_label_keys = {
                information = "gunderson.option.information",
                work = "gunderson.option.work",
                leave = "gunderson.option.leave",
            },
        },
        opening_after_run_return = {
            title_key = "gunderson.greeting.after_run_return",
            choice_label_keys = {
                information = "gunderson.option.information",
                work = "gunderson.option.work",
                leave = "gunderson.option.leave",
            },
        },
        information = {
            title_key = "gunderson.information.contacts",
            choice_label_keys = {
                questions = "gunderson.information.option.questions",
                contacts = "gunderson.information.option.contacts",
                back = "gunderson.information.option.back",
            },
        },
        information_after_return = {
            title_key = "gunderson.information.after_return",
            choice_label_keys = {
                questions = "gunderson.information.option.questions",
                contacts = "gunderson.information.option.contacts",
                back = "gunderson.information.option.back",
            },
        },
        decker_offer = {
            title_key = "gunderson.contacts.decker_offer",
            choice_label_keys = {
                accept = "gunderson.contacts.option.accept",
                back = "gunderson.information.option.back",
            },
        },
        contacts_purchased = {
            title_key = "gunderson.contacts.purchase",
            choice_label_keys = {
                questions = "gunderson.information.option.questions",
                contacts = "gunderson.information.option.contacts",
                back = "gunderson.information.option.back",
            },
        },
        contacts_insufficient = {
            title_key = "gunderson.work.followup",
            choice_label_keys = {
                information = "gunderson.option.information",
                work = "gunderson.option.work",
                leave = "gunderson.option.leave",
            },
        },
        question_selection = {
            title_key = "gunderson.question.prompt",
            choice_label_keys = {
                johnson = "gunderson.question.johnson",
                vigore = "gunderson.question.option.vigore",
                errascoe = "gunderson.question.option.errascoe",
            },
        },
        question_followup = {
            title_key = "gunderson.question.followup",
            choice_label_keys = {
                another_question = "gunderson.question.option.local",
                different_subject = "gunderson.information.option.back",
            },
        },
        run_acceptance = {
            choice_label_keys = {
                yes = "ui.option.yes",
                no = "ui.option.no",
            },
        },
        vigore_response = {
            title_key = "gunderson.question.vigore.response",
            choice_label_keys = {
                another_question = "gunderson.question.option.local",
                different_subject = "gunderson.information.option.back",
            },
        },
        vigore_alternate_followup = {
            title_key = "gunderson.question.vigore.alternate_followup",
            choice_label_keys = {
                another_question = "gunderson.question.option.local",
                different_subject = "gunderson.information.option.back",
            },
        },
        errascoe_response = {
            title_key = "gunderson.question.errascoe.response",
            choice_label_keys = {
                another_question = "gunderson.question.option.local",
                different_subject = "gunderson.information.option.back",
            },
        },
    },
    ["big-rhino.mortimer_reed"] = {
        opening = {
            title_key = "big-rhino.mortimer.opening",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
            },
        },
        repeat_opening = {
            title_key = "big-rhino.mortimer.repeat-opening",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
            },
        },
        opening_after_information = {
            title_key = "big-rhino.mortimer.after-information",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
            },
        },
        conditional_opening = {
            title_key = "big-rhino.mortimer.conditional-opening",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
            },
        },
        opening_after_run_return = {
            title_key = "big-rhino.mortimer.conditional-opening",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
            },
        },
        information = {
            title_key = "big-rhino.information.topics",
            choice_label_keys = {
                questions = "big-rhino.information.option.questions",
                contacts = "big-rhino.information.option.contacts",
                back = "big-rhino.information.option.back",
            },
        },
        question_selection = {
            title_key = "big-rhino.information.questions.prompt",
            choice_label_keys = {
                julius = "big-rhino.question.option.julius",
                mako = "big-rhino.question.option.mako",
                aragorn = "big-rhino.question.option.aragorn",
            },
        },
        question_answer_followup = {
            choice_label_keys = {
                another_question = "big-rhino.question.option.another",
                different_subject = "big-rhino.information.option.back",
            },
        },
        contacts_offer = {
            title_key = "big-rhino.contacts.offer",
            choice_label_keys = {
                accept = "big-rhino.contacts.option.accept",
                back = "big-rhino.information.option.back",
            },
        },
        contacts_purchased = {
            title_key = "big-rhino.contacts.purchased",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
            },
        },
        contacts_insufficient = {
            title_key = "big-rhino.contacts.insufficient",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
            },
        },
    },
    ["matchstick.julius_strouther"] = {
        opening = {
            title_key = "julius-strouther.repeat-greeting",
            choice_label_keys = {
                information = "julius-strouther.option.information",
                work = "julius-strouther.option.work",
                leave = "julius-strouther.option.leave",
            },
        },
        first_meeting = {
            title_key = "julius-strouther.first-meeting",
            choice_label_keys = {
                information = "julius-strouther.option.information",
                work = "julius-strouther.option.work",
                leave = "julius-strouther.option.leave",
            },
        },
        opening_after_information = {
            title_key = "julius-strouther.after-information",
            choice_label_keys = {
                information = "julius-strouther.option.information",
                work = "julius-strouther.option.work",
                leave = "julius-strouther.option.leave",
            },
        },
        after_run_return = {
            title_key = "julius-strouther.run-return",
            choice_label_keys = {
                information = "julius-strouther.option.information",
                work = "julius-strouther.option.work",
                leave = "julius-strouther.option.leave",
            },
        },
        information = {
            title_key = "julius-strouther.information.opening",
            choice_label_keys = {
                questions = "julius-strouther.information.option.questions",
                contacts = "julius-strouther.information.option.contacts",
                back = "julius-strouther.information.option.back",
            },
        },
        contacts_offer = {
            title_key = "julius-strouther.contacts.offer",
            choice_label_keys = {
                accept = "julius-strouther.contacts.option.accept",
                back = "julius-strouther.information.option.back",
            },
        },
        contacts_purchased = {
            title_key = "julius-strouther.contacts.purchased",
            choice_label_keys = {
                information = "julius-strouther.option.information",
                work = "julius-strouther.option.work",
                leave = "julius-strouther.option.leave",
            },
        },
        contacts_insufficient = {
            title_key = "julius-strouther.contacts.insufficient",
            choice_label_keys = {
                information = "julius-strouther.option.information",
                work = "julius-strouther.option.work",
                leave = "julius-strouther.option.leave",
            },
        },
        question_selection = {
            title_key = "julius-strouther.question.prompt",
            choice_label_keys = {
                caleb = "julius-strouther.question.option.about-caleb",
                mortimer = "julius-strouther.question.option.about-mortimer",
            },
        },
        question_answer_followup = {
            choice_label_keys = {
                another_question = "julius-strouther.question.option.another",
                different_subject = "julius-strouther.information.option.back",
            },
        },
    },
    ["wanderer.vigore_jarl"] = {
        opening = {
            title_key = "wanderer.vigore_jarl.opening",
            choice_label_keys = {
                information = "gunderson.option.information",
                work = "gunderson.option.work",
                leave = "gunderson.option.leave",
            },
        },
        conditional_opening = {
            title_key = "wanderer.vigore_jarl.conditional-opening",
            choice_label_keys = {
                information = "gunderson.option.information",
                work = "gunderson.option.work",
                leave = "gunderson.option.leave",
            },
        },
        booth_vacated = {
            title_key = "wanderer.vigore_jarl.booth-vacated",
            choice_label_keys = {
                fixer = "wanderer.vigore_jarl.booth-vacated.option.fixers",
                pay = "stoker.motel.option.pay",
                back = "gunderson.information.option.back",
            },
        },
    },
    ["icarus-descending.caleb_brightmore"] = {
        opening = {
            title_key = "icarus-descending.caleb.opening",
            choice_label_keys = {
                information = "icarus-descending.caleb.option.information",
                work = "icarus-descending.caleb.option.work",
                leave = "icarus-descending.caleb.option.leave",
            },
        },
        first_meeting = {
            title_key = "icarus-descending.caleb.first-meeting.continuation",
            choice_label_keys = {
                information = "icarus-descending.caleb.option.information",
                work = "icarus-descending.caleb.option.work",
                leave = "icarus-descending.caleb.option.leave",
            },
        },
        post_run = {
            title_key = "icarus-descending.caleb.post-run-opening",
            choice_label_keys = {
                information = "icarus-descending.caleb.option.information",
                work = "icarus-descending.caleb.option.work",
                leave = "icarus-descending.caleb.option.leave",
            },
        },
    },
    ["field.activity"] = {
        resource_main = {
            title_key = "field.activity.main_menu.title",
            choice_label_keys = {
                cyberspace = "field.activity.main_menu.option.cyberspace",
                exit = "field.activity.main_menu.option.exit",
            },
        },
    },
}

local phone_profile = {
    contacts = {
        title_key = "",
        layout = "compact_list",
        choice_label_keys = {
            next_screen = "vid_phone.contact.page_next",
            prior_screen = "vid_phone.contact.page_previous",
        },
    },
}
for _, contact in ipairs(vid_phone_contacts) do
    phone_profile.contacts.choice_label_keys[contact.id] = contact.name_key
    for _, call_kind in ipairs({ "first_call", "repeat_call" }) do
        phone_profile["call_" .. contact.id .. "_" .. call_kind] = {
            title_key = "vid_phone.contact." .. contact.id .. "." .. call_kind,
            choice_label_keys = {
                services = "vid_phone.option.services",
                goodbye = "vid_phone.option.goodbye",
            },
        }
    end
end
menu_profiles["field.vid_phone"] = phone_profile

local gregory_prefix = "vid_phone.gregory_wilns."
local function define_gregory_menu(id, text_id, option_keys)
    phone_profile[id] = {
        title_key = gregory_prefix .. text_id,
        choice_label_keys = option_keys,
    }
end

define_gregory_menu("gregory_services", "services", {
    paranormal_creatures = gregory_prefix .. "option.paranormal_creatures",
    magic_enhancement_device = gregory_prefix .. "option.magic_enhancement_device",
    corporate_discount = gregory_prefix .. "option.corporate_discount",
})
for index = 1, 3 do
    define_gregory_menu("gregory_creature_page_" .. index,
        "creature_page_" .. index, { more = "vid_phone.option.more" })
end
define_gregory_menu("gregory_creature_page_4", "creature_page_4", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_gregory_menu("gregory_discount_offer", "discount_offer", {
    accept_discount = gregory_prefix .. "option.yes",
    decline_discount = gregory_prefix .. "option.no",
    talk_later = gregory_prefix .. "option.talk_later",
})
define_gregory_menu("gregory_discount_result", "discount_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_gregory_menu("gregory_power_focus_offer", "power_focus_offer", {
    buy_power_focus = gregory_prefix .. "option.yes",
    decline_discount = gregory_prefix .. "option.no",
    talk_later = gregory_prefix .. "option.talk_later",
})
define_gregory_menu("gregory_power_focus_result", "power_focus_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})

local quinton = speakers.vid_phone_capt_quinton_jaymes
quinton.events.purchase_insufficient = {
    title_key = "vid_phone.capt_quinton_jaymes.purchase.insufficient",
    continuation_key = "",
}
quinton.events.purchase_out_of_stock = {
    title_key = "vid_phone.capt_quinton_jaymes.purchase.out_of_stock",
    continuation_key = "",
}
quinton.events.services_disconnected = {
    title_key = "vid_phone.capt_quinton_jaymes.services.disconnected",
    continuation_key = "",
}

local gregory = speakers.vid_phone_gregory_wilns
gregory.events.goodbye = {
    title_key = "vid_phone.goodbye",
    continuation_key = "",
}
gregory.events.insufficient_balance = {
    title_key = "vid_phone.gregory_wilns.insufficient_balance",
    continuation_key = "",
}
gregory.events.power_focus_unavailable = {
    title_key = "vid_phone.gregory_wilns.power_focus_unavailable",
    continuation_key = "",
}
gregory.events.power_focus_ineligible = {
    title_key = "vid_phone.gregory_wilns.power_focus_ineligible",
    continuation_key = "",
}

local alesandro = speakers.vid_phone_alesandro_hobbs
alesandro.events.security_patrols = {
    title_key = "vid_phone.alesandro_hobbs.security_patrols",
    continuation_key = "",
}
alesandro.events.security_alerts = {
    title_key = "vid_phone.alesandro_hobbs.security_alerts",
    continuation_key = "",
}
alesandro.events.security_technological = {
    title_key = "vid_phone.alesandro_hobbs.security_technological",
    continuation_key = "",
}
alesandro.events.security_cameras = {
    title_key = "vid_phone.alesandro_hobbs.security_cameras",
    continuation_key = "",
}
alesandro.events.purchase_insufficient = {
    title_key = "vid_phone.alesandro_hobbs.purchase.insufficient",
    continuation_key = "",
}
alesandro.events.purchase_out_of_stock = {
    title_key = "vid_phone.alesandro_hobbs.purchase.out_of_stock",
    continuation_key = "",
}

local sharkey = speakers.vid_phone_sharkey
sharkey.events.salish_passage = {
    title_key = "vid_phone.sharkey.salish_passage",
    continuation_key = "",
}
sharkey.events.sinsearach_passage = {
    title_key = "vid_phone.sharkey.sinsearach_passage",
    continuation_key = "",
}
sharkey.events.smart_goggles_offer = {
    title_key = "vid_phone.sharkey.smart_goggles_offer",
    continuation_key = "",
}
sharkey.events.insufficient_funds = {
    title_key = "vid_phone.sharkey.insufficient_funds",
    continuation_key = "",
}
sharkey.events.passage_result = {
    title_key = "vid_phone.sharkey.passage_result",
    continuation_key = "",
}
sharkey.events.smart_goggles_result = {
    title_key = "vid_phone.sharkey.smart_goggles_result",
    continuation_key = "",
}
sharkey.events.smart_goggles_inventory_full = {
    title_key = "vid_phone.sharkey.smart_goggles_inventory_full",
    continuation_key = "",
}

local kipp = speakers.vid_phone_kipp_david
kipp.events.insufficient_funds = {
    title_key = "vid_phone.kipp_david.insufficient_funds",
    continuation_key = "",
}
kipp.events.disconnected = {
    title_key = "vid_phone.kipp_david.disconnected",
    continuation_key = "",
}

local wilma = speakers.vid_phone_wilma_temmenhoff
wilma.events.insufficient_funds = {
    title_key = "vid_phone.wilma_temmenhoff.insufficient_funds",
    continuation_key = "",
}
wilma.events.later = {
    title_key = "vid_phone.wilma_temmenhoff.later",
    continuation_key = "",
}

local macklemann = speakers.vid_phone_macklemann
macklemann.events.insufficient_funds = {
    title_key = "vid_phone.macklemann.insufficient_funds",
    continuation_key = "",
}
macklemann.events.disconnected = {
    title_key = "vid_phone.macklemann.disconnected",
    continuation_key = "",
}
macklemann.events.gate = {
    title_key = "vid_phone.macklemann.gate",
    continuation_key = "",
}

local quinton_prefix = "vid_phone.capt_quinton_jaymes."
local function define_quinton_menu(id, text_id, option_keys)
    phone_profile[id] = {
        title_key = quinton_prefix .. text_id,
        choice_label_keys = option_keys,
    }
end

define_quinton_menu("capt_quinton_services", "services", {
    hollywood_information = quinton_prefix .. "option.hollywood_information",
    firepower = quinton_prefix .. "option.firepower",
    contacts = quinton_prefix .. "option.contacts",
})
define_quinton_menu("capt_quinton_hollywood_intro", "hollywood_intro", {
    more = "vid_phone.option.more",
})
define_quinton_menu("capt_quinton_hollywood_topics", "hollywood_topics", {
    weller_prison_break = quinton_prefix .. "option.weller_prison_break",
    disconnect = quinton_prefix .. "option.disconnect",
    return_to_services = quinton_prefix .. "option.return_to_services",
})
define_quinton_menu("capt_quinton_weller_intro", "weller_intro", {
    more = "vid_phone.option.more",
})
define_quinton_menu("capt_quinton_weller_result", "weller_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_quinton_menu("capt_quinton_firepower_intro", "firepower_intro", {
    more = "vid_phone.option.more",
})
define_quinton_menu("capt_quinton_firepower_items", "firepower_items", {
    frag_grenades = quinton_prefix .. "option.frag_grenades",
    hk227_s = quinton_prefix .. "option.hk227_s",
    no_thanks = quinton_prefix .. "option.no_thanks",
})
define_quinton_menu("capt_quinton_purchase_result", "purchase_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_quinton_menu("capt_quinton_contacts_offer", "contacts_offer", {
    pay_for_rigger_contact = quinton_prefix .. "option.pay_for_rigger_contact",
    decline_rigger_contact = quinton_prefix .. "option.decline_rigger_contact",
})
define_quinton_menu("capt_quinton_sharkey_result", "sharkey_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})

local alesandro_prefix = "vid_phone.alesandro_hobbs."
local function define_alesandro_menu(id, text_id, option_keys)
    phone_profile[id] = {
        title_key = alesandro_prefix .. text_id,
        choice_label_keys = option_keys,
    }
end

define_alesandro_menu("alesandro_services", "services", {
    corporation_information = alesandro_prefix .. "option.corporation_information",
    maglock_passkey = alesandro_prefix .. "option.maglock_passkey",
    seattle_official = alesandro_prefix .. "option.seattle_official",
})
define_alesandro_menu("alesandro_security_topics", "corporation_information", {
    patrols = alesandro_prefix .. "option.security_procedures",
    second_security_type = alesandro_prefix .. "option.second_security_type",
    avoid_detection = alesandro_prefix .. "option.avoid_detection",
})
define_alesandro_menu("alesandro_passkey_offer", "passkey_offer", {
    level_3_passkey = alesandro_prefix .. "option.level_3_passkey",
    level_5_passkey = alesandro_prefix .. "option.level_5_passkey",
    return_to_services = quinton_prefix .. "option.return_to_services",
})
define_alesandro_menu(
    "alesandro_official_contact_confirmation", "official_contact_offer", {
        pay_for_macklemann = alesandro_prefix .. "option.pay_for_macklemann",
        return_to_services = quinton_prefix .. "option.return_to_services",
    })
define_alesandro_menu("alesandro_purchase_result", "purchase_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_alesandro_menu("alesandro_out_of_stock", "purchase.out_of_stock", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_alesandro_menu("alesandro_macklemann_result", "macklemann_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})

local sharkey_prefix = "vid_phone.sharkey."
local function define_sharkey_menu(id, text_id, option_keys)
    phone_profile[id] = {
        title_key = sharkey_prefix .. text_id,
        choice_label_keys = option_keys,
    }
end

define_sharkey_menu("sharkey_services", "services", {
    pass_salish = sharkey_prefix .. "option.pass_salish",
    pass_sinsearach = sharkey_prefix .. "option.pass_sinsearach",
    smart_goggles = sharkey_prefix .. "option.smart_goggles",
})
define_sharkey_menu("sharkey_salish_passage", "salish_passage", {
    pay_for_passage = sharkey_prefix .. "option.pay_for_passage",
    return_to_services = sharkey_prefix .. "option.return_to_services",
    other_subject = sharkey_prefix .. "option.other_subject",
})
define_sharkey_menu("sharkey_sinsearach_passage", "sinsearach_passage", {
    pay_for_passage = sharkey_prefix .. "option.pay_for_passage",
    return_to_services = sharkey_prefix .. "option.return_to_services",
    other_subject = sharkey_prefix .. "option.other_subject",
})
define_sharkey_menu(
    "sharkey_smart_goggles_offer", "smart_goggles_offer", {
        confirm_purchase = sharkey_prefix .. "option.confirm_purchase",
        return_to_services = sharkey_prefix .. "option.return_to_services",
        other_subject = sharkey_prefix .. "option.other_subject",
    })

local kipp_prefix = "vid_phone.kipp_david."
local function define_kipp_menu(id, text_id, option_keys)
    phone_profile[id] = {
        title_key = kipp_prefix .. text_id,
        choice_label_keys = option_keys,
    }
end

define_kipp_menu("kipp_intro", "intro", {
    more = "vid_phone.option.more",
})
define_kipp_menu("kipp_services", "services", {
    matrix = kipp_prefix .. "option.matrix",
    programs = kipp_prefix .. "option.programs",
    cyberdeck = kipp_prefix .. "option.cyberdeck",
})
define_kipp_menu("kipp_matrix_intro", "matrix_intro", {
    more = "vid_phone.option.more",
})
define_kipp_menu("kipp_matrix_corps", "matrix_corps", {
    more = "vid_phone.option.more",
})
define_kipp_menu("kipp_matrix_result", "matrix_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_kipp_menu("kipp_program_intro", "program_intro", {
    more = "vid_phone.option.more",
})
define_kipp_menu("kipp_program_details", "program_details", {
    more = "vid_phone.option.more",
})
define_kipp_menu("kipp_program_offer", "program_offer", {
    buy_rebound = kipp_prefix .. "option.buy_rebound",
    buy_degrade = kipp_prefix .. "option.buy_degrade",
    different_subject = kipp_prefix .. "option.different_subject",
})
define_kipp_menu("kipp_deck_offer", "deck_offer", {
    pay_for_deck = kipp_prefix .. "option.pay_for_deck",
    different_subject = kipp_prefix .. "option.different_subject",
})
define_kipp_menu("kipp_storage_full", "storage_full", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_kipp_menu("kipp_program_result", "program_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})

local wilma_prefix = "vid_phone.wilma_temmenhoff."
local function define_wilma_menu(id, text_id, option_keys)
    phone_profile[id] = {
        title_key = wilma_prefix .. text_id,
        choice_label_keys = option_keys,
    }
end

define_wilma_menu("wilma_credstick", "credstick", {
    more = "vid_phone.option.more",
})
define_wilma_menu("wilma_services", "services", {
    matrix = "vid_phone.kipp_david.option.matrix",
    programs = "vid_phone.kipp_david.option.programs",
    goodbye = "vid_phone.option.goodbye",
})
define_wilma_menu("wilma_matrix_intro", "matrix_intro", {
    more = "vid_phone.option.more",
})
define_wilma_menu("wilma_matrix_companies", "matrix_companies", {
    more = "vid_phone.option.more",
})
define_wilma_menu("wilma_matrix_sleaze", "matrix_sleaze", {
    more = "vid_phone.option.more",
})
define_wilma_menu("wilma_matrix_result", "matrix_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_wilma_menu("wilma_program_intro", "program_intro", {
    more = "vid_phone.option.more",
})
define_wilma_menu("wilma_program_degrade", "program_degrade", {
    more = "vid_phone.option.more",
})
define_wilma_menu("wilma_program_offer", "program_offer", {
    buy_rebound = "vid_phone.kipp_david.option.buy_rebound",
    buy_degrade = "vid_phone.kipp_david.option.buy_degrade",
    different_subject = "vid_phone.kipp_david.option.different_subject",
})
define_wilma_menu("wilma_storage_full", "storage_full", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_wilma_menu("wilma_program_result", "program_result", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})

local macklemann_prefix = "vid_phone.macklemann."
local function define_macklemann_menu(id, text_id, option_keys)
    phone_profile[id] = {
        title_key = macklemann_prefix .. text_id,
        choice_label_keys = option_keys,
    }
end

define_macklemann_menu("macklemann_services", "services", {
    security = "vid_phone.alesandro_hobbs.option.security_procedures",
    avoid = "vid_phone.alesandro_hobbs.option.avoid_detection",
    hollywood = macklemann_prefix .. "option.hollywood",
})
define_macklemann_menu("macklemann_security_levels", "security_levels", {
    more = "vid_phone.option.more",
})
define_macklemann_menu("macklemann_security_patrols", "security_patrols", {
    more = "vid_phone.option.more",
})
define_macklemann_menu("macklemann_security_pass", "security_pass", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_macklemann_menu("macklemann_avoid_invisibility", "avoid_invisibility", {
    more = "vid_phone.option.more",
})
define_macklemann_menu("macklemann_avoid_cameras", "avoid_cameras", {
    more = "vid_phone.option.more",
})
define_macklemann_menu("macklemann_avoid_decker", "avoid_decker", {
    services = "vid_phone.option.services",
    goodbye = "vid_phone.option.goodbye",
})
define_macklemann_menu("macklemann_hollywood_offer", "hollywood_offer", {
    yes = "vid_phone.sharkey.option.confirm_purchase",
    goodbye = "vid_phone.option.goodbye",
})

-- Content is grouped by speaker and topic. The controller supplies only the
-- semantic runner/page ID; this table connects that ID to the imported copy.
local runner_dialogue = {
    ricky = {
        name_key = "runner.ricky.name",
        information = {
            opening = "runner.ricky.information.opening",
            continuation = "runner.ricky.information.continuation",
        },
        subject = {
            title_key = "runner.ricky.subject.title",
            options = {
                shaman_magic = "runner.ricky.subject.option.shaman_magic",
                local_area = "runner.ricky.subject.option.local_area",
                johnson = "runner.ricky.subject.option.johnson",
            },
            responses = {
                shaman_magic = {
                    "runner.ricky.subject.shaman_magic.page_0",
                    "runner.ricky.subject.shaman_magic.page_1",
                    "runner.ricky.subject.shaman_magic.page_2",
                },
                local_area = {
                    "runner.ricky.subject.local_area.page_0",
                    "runner.ricky.subject.local_area.page_1",
                },
                johnson_gunderson = {
                    "runner.ricky.subject.johnson_gunderson.page_0",
                },
            },
        },
    },
    winston_marrs = {
        name_key = "runner.winston_marrs.name",
        information = {
            opening = "runner.winston_marrs.information.opening",
            continuation = "runner.winston_marrs.information.continuation",
        },
        subject = {
            title_key = "runner.winston_marrs.subject.title",
            options = {
                halloweeners = "runner.winston_marrs.subject.option.halloweeners",
                lone_star = "runner.winston_marrs.subject.option.lone_star",
                troll_origin = "runner.winston_marrs.subject.option.troll_origin",
            },
            responses = {
                halloweeners = {
                    "runner.winston_marrs.subject.halloweeners.page_0",
                },
                lone_star = {
                    "runner.winston_marrs.subject.lone_star.page_0",
                },
                troll_origin = {
                    "runner.winston_marrs.subject.troll_origin.page_0",
                    "runner.winston_marrs.subject.troll_origin.page_1",
                },
            },
        },
    },
    trent = {
        name_key = "runner.trent.name",
        information = {
            opening = "runner.trent.information.opening",
            continuation = "runner.trent.information.continuation",
        },
        subject = {
            title_key = "runner.trent.subject.title",
            options = {
                mages = "runner.trent.subject.option.mages",
                contacts = "runner.trent.subject.option.contacts",
                johnsons = "runner.trent.subject.option.johnsons",
            },
            responses = {
                mages = {
                    "runner.trent.subject.mages.page_0",
                    "runner.trent.subject.mages.page_1",
                },
                contacts = {
                    "runner.trent.subject.contacts.page_0",
                },
                johnsons = {
                    "runner.trent.subject.johnsons.page_0",
                },
            },
        },
    },
    petr_uvehr = {
        name_key = "runner.petr_uvehr.name",
        information = {
            opening = "runner.petr_uvehr.information.opening",
            continuation = "runner.petr_uvehr.information.continuation",
        },
    },
    walking_bear = {
        name_key = "runner.walking_bear.name",
        information = {
            opening = "runner.walking_bear.information.opening",
            continuation = "runner.walking_bear.information.continuation",
        },
    },
    phantom = {
        name_key = "runner.phantom.name",
        information = {
            opening = "runner.phantom.information.opening",
            continuation = "runner.phantom.information.continuation",
        },
    },
    two_fists = {
        name_key = "runner.two_fists.name",
        information = {
            opening = "runner.two_fists.information.opening",
            continuation = "runner.two_fists.information.continuation",
        },
    },
    freya = {
        name_key = "runner.freya.name",
        information = {
            opening = "runner.freya.information.opening",
            continuation = "runner.freya.information.continuation",
        },
    },
    rianna = {
        name_key = "runner.rianna.name",
        information = {
            opening = "runner.rianna.information.opening",
            continuation = "runner.rianna.information.continuation",
        },
    },
    stark = {
        name_key = "runner.stark.name",
        information = {
            opening = "runner.stark.information.opening",
            continuation = "runner.stark.information.continuation",
        },
    },
}

local function runner_text_keys()
    local keys = { ["shared.more"] = "stoker.motel.arrival.more" }
    for runner_id, runner in pairs(runner_dialogue) do
        keys[runner_id .. ".name"] = runner.name_key
        for page_id, text_key in pairs(runner.information) do
            keys[runner_id .. ".information." .. page_id] = text_key
        end
        if runner.subject ~= nil then
            local subject = runner.subject
            keys[runner_id .. ".subject.title"] = subject.title_key
            for option_id, text_key in pairs(subject.options) do
                keys[runner_id .. ".subject.option." .. option_id] = text_key
            end
            for topic_id, pages in pairs(subject.responses) do
                for page_index, text_key in ipairs(pages) do
                    local content_id = runner_id .. ".subject." .. topic_id ..
                        ".page_" .. (page_index - 1)
                    keys[content_id] = text_key
                end
            end
        end
    end
    return keys
end

function Dialogue.speaker_profile(speaker_id)
    return speakers[speaker_id]
end

function Dialogue.event_text_keys(speaker_id, text_profile, event_id)
    local speaker = speakers[speaker_id]
    if speaker == nil or speaker.text_profile ~= text_profile or
       speaker.events == nil then
        return nil
    end
    return speaker.events[event_id]
end

function Dialogue.menu_text_keys(speaker_id, text_profile, menu_id, choice_ids,
                                 inline_prompt)
    if speaker_id ~= "" then
        local speaker = speakers[speaker_id]
        if speaker == nil or speaker.text_profile ~= text_profile then
            return nil
        end
    end
    local profile = menu_profiles[text_profile]
    local menu = profile ~= nil and profile[menu_id] or nil
    if menu == nil then
        return nil
    end

    local choice_label_keys = {}
    for index, choice_id in ipairs(choice_ids) do
        local label_key = menu.choice_label_keys[choice_id]
        if label_key == nil then
            return nil
        end
        choice_label_keys[index] = label_key
    end
    local result = {
        choice_label_keys = choice_label_keys,
        layout = menu.layout or "standard",
    }
    if inline_prompt ~= true then
        result.title_key = menu.title_key
    end
    return result
end

Dialogue.runner_text_keys = runner_text_keys
Dialogue.vid_phone_contacts = vid_phone_contacts

-- The shared Johnson conversation module reads this policy by the active
-- location's text profile. Building scripts remain generic; ROM-backed
-- records supply the return context and optional first-visit facts.
Dialogue.johnson_interaction_profiles = {
    ["big-rhino.mortimer_reed"] = {
        run_offer_context = E.RunOfferContext.BIG_RHINO,
        information_flow = "big_rhino",
        access_facts = { "mr_johnsons_flag_bit_2" },
        -- ROM scene 11 selects 0x105544 on first contact and 0x105574
        -- afterward. FFFC27 bit 2 is set by this scene branch and records
        -- that Mortimer is now a known Johnson.
        entry_seen_fact = "mr_johnsons_flag_bit_2",
        entry_events = { "booth_entry" },
        repeat_entry_events = { "repeat_booth_entry" },
        repeat_root = "repeat_opening",
        run_return_root = "opening_after_run_return",
        -- ROM 0x215DA displays 0x108501 before offering a context-2 run.
        work_opening_event = "work_opening",
        leave_event = "leave",
    },
    ["matchstick.julius_strouther"] = {
        run_offer_context = E.RunOfferContext.SCENE_18,
        information_flow = "julius",
        access_facts = { "mr_johnsons_flag_bit_1" },
        first_visit_fact = "mr_johnsons_flag_bit_5",
        first_visit_events = { "booth_entry" },
        first_visit_root = "first_meeting",
        run_return_root = "after_run_return",
        work_opening_event = "work_opening",
        leave_event = "leave",
    },
    ["gunderson.jump_house"] = {
        run_offer_context = E.RunOfferContext.JUMP_HOUSE,
        information_flow = "gunderson",
        -- ROM 0x106C99 "Well, ol' Gunderson's got a beauty for you." opens
        -- the work branch before the offer (bfa3908 always said it here).
        work_opening_event = "work-opening",
    },
    ["wanderer.vigore_jarl"] = {
        run_offer_context = E.RunOfferContext.WANDERER,
        first_visit_fact = "wanderer_vigore_jarl_intro_seen",
        first_visit_events = { "first_impression", "first_meeting" },
        post_departure_story_flag = E.StoryFlag.ITO_FIGHT_WON,
        post_departure_context = E.RunOfferContext.WANDERER,
        post_departure_event = "booth_vacated",
        post_departure_speaker = "wanderer_booth_narrator",
        post_departure_text_profile = "wanderer.vigore_jarl",
        run_return_root = "conditional_opening",
    },
    ["icarus-descending.caleb_brightmore"] = {
        run_offer_context = E.RunOfferContext.ICARUS_DESCENDING,
        access_facts = { "mr_johnsons_flag_bit_4" },
        first_visit_fact = "mr_johnsons_flag_bit_7",
        first_visit_events = { "first_meeting", "first_meeting_continuation" },
        first_visit_runner_progress_delta = 2,
        first_visit_root = "first_meeting",
        run_return_root = "post_run",
        work_opening_event = "work_opening",
        leave_event = "leave",
    },
}

return Dialogue

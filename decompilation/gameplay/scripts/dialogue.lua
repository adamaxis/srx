-- Speaker properties are supplied by gameplay content and resolved from the
-- speaker id carried by the active map/location context.
local Dialogue = {}
local E = require("enums")

local speaker_catalog = game.content and game.content.speakers
assert(type(speaker_catalog) == "table" and
       speaker_catalog.schemaVersion == 1 and
       type(speaker_catalog.profiles) == "table",
    "speaker JSON catalog is missing or malformed")
local speakers = speaker_catalog.profiles

-- The source-ordered contact records and their fact bindings live in JSON.
-- Dialogue code projects the validated catalog into the existing speaker and
-- service interfaces; it does not duplicate the 13-record table.
local phone_catalog = game.content and game.content.contacts
assert(type(phone_catalog) == "table" and phone_catalog.schemaVersion == 1 and
       type(phone_catalog.source) == "table" and
       type(phone_catalog.contacts) == "table" and
       phone_catalog.source.recordCount == #phone_catalog.contacts,
    "Vid-Phone JSON catalog is missing or malformed")

local vid_phone_contacts = {}
local contact_ids = {}
local availability_facts = {}
local history_facts = {}
for position, record in ipairs(phone_catalog.contacts) do
    assert(type(record) == "table" and record.sourceIndex == position - 1 and
           type(record.id) == "string" and
           type(record.speakerId) == "string" and
           type(record.name) == "string" and
           type(record.nameKey) == "string" and
           type(record.portraitAlias) == "string" and
           type(record.textProfile) == "string" and
           type(record.availabilityFact) == "string" and
           type(record.callHistoryFact) == "string" and
           type(record.firstCall) == "table" and
           type(record.repeatCall) == "table" and
           type(record.firstCall.eventId) == "string" and
           type(record.firstCall.textKey) == "string" and
           type(record.firstCall.continuation) == "boolean" and
           type(record.repeatCall.eventId) == "string" and
           type(record.repeatCall.textKey) == "string" and
           type(record.repeatCall.continuation) == "boolean",
        "Vid-Phone JSON contact record is malformed at index " ..
        tostring(position - 1))
    assert(not contact_ids[record.id] and
           not availability_facts[record.availabilityFact] and
           not history_facts[record.callHistoryFact],
        "Vid-Phone JSON catalog contains a duplicate contact or fact")
    assert(record.callHistoryPolicy == "before_first_call" or
           record.callHistoryPolicy == "external_effect",
        "Vid-Phone contact has an unsupported history policy: " .. record.id)
    contact_ids[record.id] = true
    availability_facts[record.availabilityFact] = true
    history_facts[record.callHistoryFact] = true

    local contact = {
        source_index = record.sourceIndex,
        id = record.id,
        speaker_id = record.speakerId,
        name = record.name,
        name_key = record.nameKey,
        portrait_alias = record.portraitAlias,
        text_profile = record.textProfile,
        availability_fact = record.availabilityFact,
        call_history_fact = record.callHistoryFact,
        call_history_policy = record.callHistoryPolicy,
        first_call_event = record.firstCall.eventId,
        first_call_text_key = record.firstCall.textKey,
        first_continuation = record.firstCall.continuation,
        repeat_call_event = record.repeatCall.eventId,
        repeat_call_text_key = record.repeatCall.textKey,
        repeat_continuation = record.repeatCall.continuation,
        farewell_event = record.farewellEvent,
    }
    if record.service ~= nil then
        assert(type(record.service) == "table" and
               type(record.service.module) == "string",
            "Vid-Phone service binding is malformed: " .. record.id)
        contact.service_module = record.service.module
        if record.service.graphId ~= nil then
            assert(type(record.service.graphId) == "string" and
                   record.service.graphId ~= "",
                "Vid-Phone service graph binding is malformed: " ..
                record.id)
            contact.service_graph_id = record.service.graphId
        end
        if record.service["function"] ~= nil then
            assert(type(record.service["function"]) == "string" and
                   record.service["function"] ~= "",
                "Vid-Phone service function binding is malformed: " ..
                record.id)
            contact.service_function = record.service["function"]
        end
        assert(contact.service_graph_id ~= nil or
               contact.service_function ~= nil,
            "Vid-Phone service needs a graph or legacy function: " ..
            record.id)
    end
    vid_phone_contacts[#vid_phone_contacts + 1] = contact
end

-- Menu layouts, choice IDs, and text-key bindings are authored in JSON.
local menu_catalog = game.content and game.content.menus
assert(type(menu_catalog) == "table" and
       menu_catalog.schemaVersion == 1 and
       type(menu_catalog.profiles) == "table",
    "dialogue menu JSON catalog is missing or malformed")
local menu_profiles = menu_catalog.profiles

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

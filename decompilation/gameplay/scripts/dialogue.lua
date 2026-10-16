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
            opening = {
                title_key = "big-rhino.mortimer.opening",
                continuation_key = "",
            },
            conditional_opening = {
                title_key = "big-rhino.mortimer.conditional-opening",
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
        conditional_opening = {
            title_key = "big-rhino.mortimer.conditional-opening",
            choice_label_keys = {
                information = "big-rhino.option.information",
                work = "big-rhino.option.work",
                leave = "big-rhino.option.leave",
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

function Dialogue.menu_text_keys(speaker_id, text_profile, menu_id, choice_ids)
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
    return {
        title_key = menu.title_key,
        choice_label_keys = choice_label_keys,
    }
end

Dialogue.runner_text_keys = runner_text_keys

-- The shared Johnson conversation module reads this policy by the active
-- location's text profile. Building scripts remain generic; ROM-backed
-- records supply the return context and optional first-visit facts.
Dialogue.johnson_interaction_profiles = {
    ["gunderson.jump_house"] = {
        run_offer_context = E.RunOfferContext.JUMP_HOUSE,
    },
    ["wanderer.vigore_jarl"] = {
        run_offer_context = E.RunOfferContext.WANDERER,
        first_visit_fact = "wanderer_vigore_jarl_intro_seen",
        first_visit_events = { "first_impression", "first_meeting" },
    },
}

return Dialogue

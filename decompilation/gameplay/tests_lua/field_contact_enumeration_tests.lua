package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local generated = require("data.field_contact_dialogue_rows")
local DialogueFramework = require("dialogue_framework")
local World = require("world_content")

local function check(condition, message)
    if not condition then error(message, 2) end
end

check(generated.counts.total_spawn_records == 1102 and
      generated.counts.bit4_records == 198 and
      generated.counts.bit3_records == 71 and
      generated.counts.resolved_talkable_records == 78 and
      generated.counts.unresolved_records == 182 and
      generated.counts.generated_activity_routes == 68 and
      generated.counts.original_unresolved_census_records == 253 and
      generated.counts.resolved_from_unresolved_records == 48 and
      generated.counts.still_unresolved_with_reason == 182 and
      generated.counts.non_talkable_records == 23,
      "ROM field contact enumeration counts changed")
local category_counts = generated.counts.classification_counts
check(category_counts.ENEMY == 182 and category_counts.PEDESTRIAN == 0 and
      category_counts.SCRIPTED == 2 and category_counts.OTHER_ROUTE == 46 and
      category_counts.NON_TALKABLE == 23,
      "original unresolved contact categories changed")
local role_counts = generated.counts.actor_role_counts
check(role_counts.CORP_GUARD == 168 and role_counts.PEDESTRIAN == 69 and
      role_counts.RUN_TARGET_ENCOUNTER == 14,
      "source actor roles in unresolved census changed")

local generic_delay = DialogueFramework.describe(
    E.DialogueSource.FIELD_NPC_CONTACT, "field_generic_contacts.present")
local ilene_delay = DialogueFramework.describe(
    E.DialogueSource.STORY_ACTIVITY, "ilene.complete_scene_20_activity")
local pedestrian_delay = DialogueFramework.describe(
    E.DialogueSource.RANDOM_ENCOUNTER,
    "run_employee_recruitment.present_random_contact")
check(generic_delay.text_delay_frames ==
          E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23 and
      E.DialogueTextDelay.CONTACT_FIRST_TEXT_FRAME_23 == 22 and
      ilene_delay.text_delay_frames ==
          E.DialogueTextDelay.ILENE_FIRST_TEXT_FRAME_24 and
      E.DialogueTextDelay.ILENE_FIRST_TEXT_FRAME_24 == 23,
      "typed dialogue descriptor lost the measured contact first-text frames")
check(pedestrian_delay.text_delay_frames ==
          E.DialogueTextDelay.PEDESTRIAN_FIRST_TEXT_FRAME_22 and
      E.DialogueTextDelay.PEDESTRIAN_FIRST_TEXT_FRAME_22 == 21,
      "typed dialogue descriptor lost the pedestrian first-text frame")

local flagged_records = 0
local unknown_flag_records = 0
local scene_total = 0
for _, record in ipairs(generated.records) do
    if record.source_spawn_state_flags == nil then
        unknown_flag_records = unknown_flag_records + 1
    else
        flagged_records = flagged_records + 1
    end
end
for scene_id = 0, 55 do
    local counts = generated.per_scene[scene_id]
    check(type(counts) == "table",
          "generated enumeration omitted scene " .. scene_id)
    scene_total = scene_total + counts.total_spawn_records
end
check(#generated.records == 283 and flagged_records == 269 and
      unknown_flag_records == 14 and scene_total == 1102,
      "generated contact report lost flagged or unresolved source records")
local classified_total = 0
local unresolved_with_reason = 0
for _, record in ipairs(generated.records) do
    if record.contact_resolution ~= "resolved_21174" then
        check(record.classification ~= nil and record.classification ~= "",
              "unresolved census record has no ROM classification")
        classified_total = classified_total + 1
    end
    if record.contact_resolution:find("^unresolved_") then
        check(record.unresolved_reason ~= nil and
              record.unresolved_reason ~= "",
              "still-unresolved source record has no reason")
        unresolved_with_reason = unresolved_with_reason + 1
    end
end
check(classified_total == 253 and unresolved_with_reason == 182,
      "generated unresolved/unknown records are not fully classified")

local ricky_route = generated.by_activity_id.scene_19_script
local ricky_action = World.field_actor_contact_task(
    19, "ricky_contact", "scene_19_script",
    E.FieldActorContactHandler.ORDINARY)
check(ricky_action == E.FieldActorContactAction.GENERIC_DIALOGUE and
      ricky_route ~= nil,
      "Ricky's ROM bit-4 record did not select generic dialogue")
local ricky_default_id = World.generic_field_contact_dialogue(
    19, "scene_19_script", E.GenericFieldContactVariant.GROUP_0)
local ricky_high_id = World.generic_field_contact_dialogue(
    19, "scene_19_script", E.GenericFieldContactVariant.GROUP_0, 4)
local ricky_default = generated.responses[ricky_default_id]
local ricky_high = generated.responses[ricky_high_id]
check(ricky_default_id == "scene_19_group_0_default" and
      ricky_default.row_pointer == "0xFE74C" and
      ricky_default.text_pointer == "0xFFBDA" and
      ricky_default.opening_text:find("rat%-like dwarf") and
      ricky_default.opening_text:find("silence") and
      ricky_high_id == "scene_19_group_0_high" and
      ricky_high.row_pointer == "0xFE768" and
      ricky_high.text_pointer == "0xFFC14" and
      ricky_high.opening_text:find("Ricky scuttles closer") and
      ricky_high.opening_text:find("Hello, friend") and
      ricky_high.script_texts[1].text_pointer == "0xFFC14",
      "Ricky's generated 0x21174 row/script no longer matches ROM text")

local marrs_action = World.field_actor_contact_task(
    11, "marrs_contact", "scene_11_script",
    E.FieldActorContactHandler.ORDINARY)
local marrs_default_id = World.generic_field_contact_dialogue(
    11, "scene_11_script", E.GenericFieldContactVariant.GROUP_1)
local marrs_high_id = World.generic_field_contact_dialogue(
    11, "scene_11_script", E.GenericFieldContactVariant.GROUP_1, 4)
local marrs_default = generated.responses[marrs_default_id]
local marrs_high = generated.responses[marrs_high_id]
local hires_option_found = false
for _, row in ipairs(marrs_default.script_texts) do
    if row.text_pointer == "0x103C5B" and
       row.text:find("good 'runner") then
        hires_option_found = true
    end
end
check(marrs_action == E.FieldActorContactAction.GENERIC_DIALOGUE and
      marrs_default.text_pointer == "0x100353" and
      marrs_default.opening_text:find("armed troll studies you") and
      marrs_high.text_pointer == "0x10039D" and
      marrs_high.opening_text:find("Marrs salutes you") and
      hires_option_found,
      "Marrs's hireable-runner contact lost its generated ROM dialogue")

local ilene_record
for _, record in ipairs(generated.records) do
    if record.scene_id == 20 and record.source_list == "0xA8CC" then
        ilene_record = record
        break
    end
end
check(ilene_record ~= nil and ilene_record.bit3 == true and
      ilene_record.contact_resolution == "resolved_scripted_route" and
      ilene_record.classification == "SCRIPTED" and
      ilene_record.classification_reason:find("0x54402") and
      generated.by_activity_id.scene_20_ilene_activity.callback_address ==
          "0x54402",
      "Ilene's extraction callback was not imported as a scripted route")

game = {
    modules = { world_content_data = { conditional_actor_candidates = {} } },
    actors = {
        extraction_target = {
            scene_id = 20,
            actor_facts = {
                lifecycle = { active = true },
                behavior = {},
            },
        },
    },
}
local extraction_action = World.field_actor_contact_task(
    20, "extraction_target", "scene_20_ilene_activity",
    E.FieldActorContactHandler.PENDING_WORLD_EFFECT)
check(extraction_action == E.FieldActorContactAction.ILENE_ACTIVITY,
      "Ilene's bit-3 extraction contact did not select its ROM callback route")

local aragorn_route = generated.by_activity_id.scene_34_story_contact
game.actors.aragorn_contact = {
    scene_id = 34,
    actor_facts = {
        lifecycle = { active = true },
        behavior = {},
    },
}
local aragorn_action = World.field_actor_contact_task(
    34, "aragorn_contact", "scene_34_story_contact",
    E.FieldActorContactHandler.WORLD_EFFECT)
check(aragorn_route ~= nil and aragorn_route.callback_address == "0x53C90" and
      aragorn_route.action == E.FieldActorContactAction.ARAGORN_CONTACT and
      aragorn_route.classification == "SCRIPTED" and
      aragorn_action == E.FieldActorContactAction.ARAGORN_CONTACT and
      game.actors.aragorn_contact.actor_facts.behavior.escort == true,
      "Aragorn's bit-3 contact was not imported through its scripted route")

local pedestrian_first = generated.by_activity_id.scene_21_pedestrian_first
local pedestrian_second = generated.by_activity_id.scene_21_pedestrian_second
local pedestrian_unreachable
for _, record in ipairs(generated.records) do
    if record.activity_id == "scene_21_pedestrian_unreachable" then
        pedestrian_unreachable = record
        break
    end
end
check(pedestrian_first ~= nil and pedestrian_second ~= nil and
      pedestrian_first.classification == "OTHER_ROUTE" and
      pedestrian_first.callback_address == "0x1F146" and
      pedestrian_second.action ==
          E.FieldActorContactAction.RANDOM_PEDESTRIAN_CONTACT and
      generated.by_activity_id.scene_21_pedestrian_unreachable == nil and
      pedestrian_unreachable ~= nil and
      pedestrian_unreachable.classification == "NON_TALKABLE" and
      pedestrian_unreachable.classification_reason:find("0x0D3A"),
      "conditional pedestrian routes or the source-unreachable third option changed")
check(World.field_actor_contact_task(
          21, "pedestrian_first", "scene_21_pedestrian_first",
          E.FieldActorContactHandler.ORDINARY) ==
          E.FieldActorContactAction.RANDOM_PEDESTRIAN_CONTACT and
      World.field_actor_contact_task(
          21, "pedestrian_second", "scene_21_pedestrian_second",
          E.FieldActorContactHandler.ORDINARY) ==
          E.FieldActorContactAction.RANDOM_PEDESTRIAN_CONTACT,
      "reachable pedestrian contact was not resolved by the generated table")

local shown_text
game.location = { type = "field_map", id = "scene_20_ilene" }
game.state = {}
game.services = {
    call = function(service, action)
        check(service == "party" and action == "award_karma_2",
              "Ilene extraction contact requested the wrong service")
        return "applied"
    end,
}
game.ui = {
    say_text = function(text, continuation)
        check(continuation == false,
              "Ilene extraction dialogue changed its continuation policy")
        shown_text = text
    end,
}
local Ilene = require("ilene")
check(Ilene.complete_scene_20_activity() == "completed" and
      shown_text ~= nil and shown_text:find("gracefully leaps") and
      shown_text:find("homemade pistol"),
      "Ilene's extraction callback did not show its source-backed dialogue")

print("field contact enumeration tests passed: 1102 records, 78 resolved, " ..
      "253 classified (48 newly resolved, 182 unresolved, 23 non-talkable)")

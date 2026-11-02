package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local generated = require("data.field_contact_dialogue_rows")
local World = require("world_content")

local function check(condition, message)
    if not condition then error(message, 2) end
end

check(generated.counts.total_spawn_records == 1102 and
      generated.counts.bit4_records == 198 and
      generated.counts.bit3_records == 71 and
      generated.counts.resolved_talkable_records == 30 and
      generated.counts.unresolved_records == 253 and
      generated.counts.generated_activity_routes == 20,
      "ROM field contact enumeration counts changed")

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
      ilene_record.contact_resolution == "alternate_source_handler" and
      ilene_record.unresolved_reason:find("0x54402") and
      generated.by_activity_id.scene_20_ilene_activity == nil,
      "Ilene's extraction contact was incorrectly treated as a 0x21174 row")

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

print("field contact enumeration tests passed: 1102 records, 30 resolved")

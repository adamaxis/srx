package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Json = require("json")

local function read_json(path)
    local file = assert(io.open(path, "rb"))
    local value = Json.decode(file:read("*a"))
    file:close()
    return value
end

local function read_capture()
    local file = assert(io.open(
        "decompilation/gameplay/tests_lua/fixtures/"
            .. "stoker_ambush_gpgx.tsv", "rb"))
    local columns, rows = nil, {}
    for line in file:lines() do
        if line:sub(1, 1) ~= "#" then
            local values = {}
            for value in (line .. "\t"):gmatch("(.-)\t") do
                values[#values + 1] = value
            end
            if columns == nil then
                columns = values
            else
                local row = {}
                for index, column in ipairs(columns) do
                    row[column] = values[index]
                end
                for _, key in ipairs({
                    "pass", "slot", "x", "y", "kind", "action_code",
                    "source_action_kind", "source_action_code", "actor_flags",
                    "interaction_flags", "source_posture", "spawn_flags", "heading",
                    "target_heading", "behavior_delay",
                }) do
                    row[key] = assert(tonumber(row[key]),
                        "invalid numeric capture field " .. key)
                end
                rows[#rows + 1] = row
            end
        end
    end
    file:close()
    return rows
end

local capture = read_capture()
local initial, by_pass = {}, {}
for _, row in ipairs(capture) do
    if row.pass == 0 then
        initial[row.slot] = row
    else
        by_pass[row.pass] = by_pass[row.pass] or {}
        by_pass[row.pass][row.slot] = row
    end
end
assert(#capture == 63, "GPGX fixture must contain 3 init rows and 60 service rows")
for pass = 1, 20 do
    assert(by_pass[pass] ~= nil,
        "GPGX fixture is missing source service pass " .. pass)
    for slot = 6, 8 do
        assert(by_pass[pass][slot] ~= nil,
            "GPGX fixture is missing actor " .. slot .. " on pass " .. pass)
    end
end

local source_rows = {}
for index, slot in ipairs({ 6, 7, 8 }) do
    local row = assert(initial[slot], "missing source init slot " .. slot)
    source_rows[index] = {
        scene_id = E.Scene.STOKER_MOTEL,
        sequence_order = index - 1,
        sequence_id = E.WorldEffectSequence.STOKER_MOTEL_AMBUSH,
        selection_kind = "weighted_encounter",
        placement_rule =
            "open screen-edge candidates based on the current viewport and terrain",
        actor_name = index == 1 and "weenr2" or "weenr1",
        profile_id = row.profile,
        visual_profile_id = E.ActorVisualProfile.YAKUZA_WEENR,
        action_kind = row.action_kind,
        action_id = index == 1 and "streetline_special" or nil,
        actor_facts = { interaction = {} },
        physical_meter = 10,
        palette_line = 1,
        sprite_stem = "yakuza_weenr_k08_item0a",
        activity_probe_enabled = true,
        action_delay_modifier = 0,
        combat_cyberware = {},
        combat_mental_meter = 10,
        posture_reduction_percent = 0,
    }
end

local pages, service_calls = {}, {}
local content = {
    speakers = read_json("decompilation/gameplay/dialogue/speakers.json"),
    menus = read_json("decompilation/gameplay/dialogue/menus.json"),
    conditions = read_json("decompilation/gameplay/dialogue/conditions.json"),
    cinematics = read_json("decompilation/gameplay/dialogue/cinematics.json"),
    conversations = {
        stoker_motel = read_json(
            "decompilation/gameplay/dialogue/conversations/stoker_motel.json"),
    },
}

game = {
    actors = {},
    modules = {
        world_content_data = {
            dynamic_world_activity_actors = source_rows,
        },
    },
    content = content,
    state = { motel_bill_paid = false },
    location = { type = "motel", id = "stoker_motel" },
    field = { package_run_status = "none" },
    ui = {
        menu = function()
            error("forced ambush path unexpectedly opened a choice menu")
        end,
        prompt_text_key = function()
            error("forced ambush path unexpectedly split prompt text")
        end,
        say = function(event_id)
            pages[#pages + 1] = event_id
            return true
        end,
    },
    services = {
        call = function(domain, operation)
            service_calls[#service_calls + 1] = {
                domain = domain,
                operation = operation,
            }
            if domain == "random" and operation == "bounded_100" then
                return "roll_80"
            end
            local prefix = "spawn_sequence_"
            if domain == "stoker_motel" and
               operation:sub(1, #prefix) == prefix then
                local sequence_id = operation:sub(#prefix + 1)
                local Encounters = require("field_encounters")
                local sequence = Encounters.resolve_sequence(
                    E.Scene.STOKER_MOTEL, sequence_id)
                assert(sequence ~= nil and
                       sequence.placement_policy ==
                           E.WorldEffectPlacementPolicy.MAP_DERIVED_CANDIDATES,
                    "direct Stoker spawn must request the ROM 0x1E40C policy")
                assert(#sequence.actors == E.StokerMotelArrival.AMBUSH_ACTOR_COUNT)
                for index, source in ipairs(sequence.actors) do
                    assert(source.profile_id == initial[index + 5].profile)
                    assert(source.action_kind == (index == 1 and "item" or "none"))
                end
                return "spawned"
            end
            return nil
        end,
    },
}

local MichaelPackage = require("michael_package")
local outcome = MichaelPackage.present_field_exit_prompt()
assert(outcome.type == "continue_in_place" and
       pages[2] == "street-ambush",
    ("production Stoker arrival Lua selects the capture's ambush branch "
        .. "(outcome=%s page1=%s page2=%s)")
        :format(tostring(outcome.type), tostring(pages[1]), tostring(pages[2])))
assert(#service_calls == 2 and
       service_calls[1].domain == "random" and
       service_calls[2].domain == "stoker_motel",
    "arrival requests one source random roll and one ordered actor spawn")

-- The captured actors are beyond the visible projection from camera
-- (0x2D20,0x09E0), so source AI has no linked target and remains at 0xB45C.
-- Feed those facts through the production ActorAI heartbeat for each of the
-- 20 captured 0xA8EA passes; Lua expresses the same continuation as idle.
local ActorAI = require("actor.init")
local function actor_facts()
    return {
        lifecycle = {
            active = true,
            thinking_enabled = true,
            ai_runner = false,
            downed = false,
            group_leader = false,
            dead = false,
        },
        interaction = {},
        behavior = {},
        roles = {
            spotted = false,
            reinforcement = false,
            despawn_offscreen = false,
            quest = false,
            runner = false,
            pedestrian = false,
            hostile = true,
        },
    }
end

local matched = 0
for order, slot in ipairs({ 6, 7, 8 }) do
    local source = assert(initial[slot])
    assert(source.kind == 8 and source.source_posture == 4 and
           source.spawn_flags == 0x40 and source.actor_flags == 0x25)
    local id = "stoker_ambush." .. tostring(slot)
    local context = {
        actor_id = id,
        profile_id = source.profile,
        visual_profile_id = E.ActorVisualProfile.YAKUZA_WEENR,
        action_kind = source.action_kind,
        action_item_id = source.action_kind == "item"
            and "streetline_special" or nil,
        action_code = source.action_code,
        actor_facts = actor_facts(),
        dynamic_world_encounter_actor = true,
        source_record_slot = slot,
        has_live_actor_record = true,
        source_record_flags = source.actor_flags,
        source_spawn_state_flags = source.spawn_flags,
        source_interaction_flags = source.interaction_flags,
        source_behavior_flags = 0,
        phase = "behavior",
        spawn = true,
        initial_bearing_degrees = 0,
        entry_delay = source.behavior_delay,
        scene_id = E.Scene.STOKER_MOTEL,
        source_projection_active = false,
        source_think_suppressed = false,
        world_activity_active = false,
        field_sequence_active = true,
        scene_mode = "street",
        actor_behavior_profile = {
            engagement_policy = "engage_within_range",
            approach_band = "medium",
        },
        world_position = { x = source.x, y = source.y },
        movement_allowed = false,
        ai_target_candidates = {},
        world_target_selection_candidates = {},
        source_item_spawn_result = "failed",
        initial_action_delay = source.action_code == 1 and 40 or 127,
        action_timing = { quickness = 2, intelligence = 4, modifier = 0 },
        measure_destination = function()
            return { bearing_degrees = 0, distance = 0, step_cost = 0 }
        end,
    }
    local spawned = ActorAI.heartbeat(context)
    assert(spawned ~= nil and game.actors[id] ~= nil,
        "production actor heartbeat initializes source slot " .. slot)
    context.spawn = false
    for pass = 1, 20 do
        context.actor_facts = actor_facts()
        local result = ActorAI.heartbeat(context)
        local state = game.actors[id]
        local source_pass = assert(by_pass[pass][slot])
        assert(result ~= nil,
            "production actor heartbeat handles pass " .. pass .. " slot " .. slot)
        assert(state.animation_id == "idle" and
               state.movement_active == false and state.ai_target_id == nil,
            "Lua preserves the ROM idle/unlinked decision at pass "
                .. pass .. " slot " .. slot)
        assert(source_pass.motion_fn == "0x0000B45C" and
               source_pass.target_ptr == "0x00000000" and
               source_pass.heading == 0 and source_pass.target_heading == 0 and
               source_pass.x == source.x and source_pass.y == source.y,
            "fixture source snapshot remains the expected idle/unlinked state")
        matched = matched + 1
    end
end
assert(matched == 60, "Lua AI/source fixture matches " .. matched .. "/60 snapshots")
print("Stoker ambush GPGX comparison passed (3 actors, 20 AI passes, 60/60)")

local E = require("enums")

local WORDS = {
    story = "story",       -- FFFC1E
    mode = "mode",         -- FFFC30
    effect = "effect",     -- FFFC34
    auxiliary = "auxiliary", -- FFFC38
    dialogue = "dialogue", -- FFFC26
}

local function copy_table(source)
    local result = {}
    for key, value in pairs(source) do result[key] = value end
    return result
end

local function summary(snapshot)
    return table.concat({
        "scene=" .. tostring(snapshot.scene),
        "nuyen=" .. tostring(snapshot.nuyen),
        "page=" .. tostring(snapshot.page_key),
        "panel=" .. tostring(snapshot.panel_actor) .. "#" ..
            tostring(snapshot.panel_page) .. ":" .. tostring(snapshot.panel_text),
        "options=" .. tostring(snapshot.panel_option_count) .. ":" ..
            tostring(snapshot.panel_selected_option),
        "dialogue_open=" .. tostring(snapshot.dialogue_open),
        "elevator_open=" .. tostring(snapshot.elevator_open),
        "story=" .. tostring(snapshot.story),
        "mode=" .. tostring(snapshot.mode),
        "effect=" .. tostring(snapshot.effect),
        "auxiliary=" .. tostring(snapshot.auxiliary),
        "group_item=" .. tostring(snapshot.group_item),
        "dialogue=" .. tostring(snapshot.dialogue),
        "position=" .. tostring(snapshot.x) .. "," .. tostring(snapshot.y),
        "opening_auto_walk=" .. tostring(snapshot.opening_auto_walk),
        "event=" .. tostring(snapshot.event),
    }, ";")
end

local function bitmask(indexes)
    local result = 0
    for _, index in ipairs(indexes or {}) do
        assert(type(index) == "number" and index >= 0 and index < 32,
            "flag index is outside a 32-bit register")
        result = result | (1 << index)
    end
    return result
end

local function expected_word(name, before, specification)
    local set_mask = 0
    local clear_mask = 0
    if name == "story" then
        local bits = assert(before.story_fact_bits,
            "story fact bit map is unavailable")
        for _, fact in ipairs(specification.set or {}) do
            set_mask = set_mask | (1 << assert(bits[fact],
                "unknown named story fact: " .. tostring(fact)))
        end
        for _, fact in ipairs(specification.clear or {}) do
            clear_mask = clear_mask | (1 << assert(bits[fact],
                "unknown named story fact: " .. tostring(fact)))
        end
    else
        set_mask = bitmask(specification.set)
        clear_mask = bitmask(specification.clear)
    end
    return (before[name] & ~clear_mask) | set_mask
end

local function validate_flags(before, after, expected, expected_facts)
    for name in pairs(WORDS) do
        local old_value = before[name]
        local new_value = after[name]
        if type(old_value) ~= "number" or type(new_value) ~= "number" then
            return "missing tracked flag word " .. name
        end
        local specification = expected[name] or {}
        local value = expected_word(name, before, specification)
        if new_value ~= value then
            return string.format(
                "%s unexpected bit change: before=%d expected=%d actual=%d toggled=%d",
                name, old_value, value, new_value, old_value ~ new_value)
        end
    end
    for fact, value in pairs(expected_facts or {}) do
        if after.story_facts[fact] ~= value then
            return "story fact " .. fact .. " expected " .. tostring(value) ..
                " but was " .. tostring(after.story_facts[fact])
        end
    end
    return nil
end

local function validate_group_items(before, after, specification)
    if type(before.group_items) ~= "table" or
       type(after.group_items) ~= "table" then
        return "group-item snapshot is unavailable"
    end
    local changes = {}
    for _, item_id in ipairs(specification.set or {}) do
        changes[item_id] = true
    end
    for _, item_id in ipairs(specification.clear or {}) do
        changes[item_id] = false
    end
    for item_id, old_owned in pairs(before.group_items) do
        local expected_owned = changes[item_id]
        if expected_owned == nil then expected_owned = old_owned end
        if after.group_items[item_id] ~= expected_owned then
            return "group item " .. item_id .. " expected " ..
                tostring(expected_owned) .. " but was " ..
                tostring(after.group_items[item_id])
        end
    end
    for item_id in pairs(changes) do
        if before.group_items[item_id] == nil or
           after.group_items[item_id] == nil then
            return "group item " .. item_id .. " is not tracked"
        end
    end
    return nil
end

local function validate(before, after, expected, host_result)
    if host_result.host_error ~= "" then
        return host_result.host_error
    end
    local lower_stderr = string.lower(host_result.stderr)
    if lower_stderr:find("warning: dialogue dead end", 1, true) then
        return "runtime emitted warning: dialogue dead end"
    end
    if host_result.source_boundary_count > 0 then
        local boundary_events = {}
        for _, event in ipairs(host_result.source_events) do
            if event:find("boundary", 1, true) or
               event:find("unresolved", 1, true) or
               event:find("unhandled", 1, true) or
               event:find("failed", 1, true) then
                boundary_events[#boundary_events + 1] = event
            end
        end
        return "runtime reported a source boundary: " ..
            table.concat(boundary_events, "; ")
    end
    if expected.source_event ~= nil then
        local found = false
        for _, event in ipairs(host_result.source_events) do
            if event == expected.source_event then found = true; break end
        end
        if not found then
            return "expected source event " .. expected.source_event ..
                " was not emitted"
        end
    end
    local flag_error = validate_flags(before, after, expected.flags or {},
        expected.facts)
    if flag_error ~= nil then return flag_error end
    local group_item_error = validate_group_items(before, after,
        expected.group_items or {})
    if group_item_error ~= nil then return group_item_error end

    local expected_scene = expected.scene or before.scene
    if after.scene ~= expected_scene then
        return "scene expected " .. tostring(expected_scene) ..
            " but was " .. tostring(after.scene)
    end
    local expected_nuyen_delta = expected.nuyen_delta or 0
    if type(before.nuyen) ~= "number" or type(after.nuyen) ~= "number" or
       after.nuyen - before.nuyen ~= expected_nuyen_delta then
        return "nuyen delta expected " .. tostring(expected_nuyen_delta) ..
            " but was " .. tostring(after.nuyen - before.nuyen)
    end
    if expected.page_key ~= nil and after.page_key ~= expected.page_key then
        return "page expected " .. expected.page_key .. " but was " ..
            tostring(after.page_key)
    end
    if expected.panel_actor ~= nil and after.panel_actor ~= expected.panel_actor then
        return "dialogue panel actor expected " .. expected.panel_actor ..
            " but was " .. tostring(after.panel_actor)
    end
    if expected.panel_text_contains ~= nil and
       not after.panel_text:find(expected.panel_text_contains, 1, true) then
        return "dialogue panel did not contain " ..
            string.format("%q", expected.panel_text_contains) ..
            "; current panel text is " .. string.format("%q", after.panel_text)
    end

    local karma_delta = expected.karma_delta or { 0, 0, 0 }
    if type(before.karma) ~= "table" or type(after.karma) ~= "table" then
        return "party Karma snapshot is unavailable"
    end
    for index = 1, 3 do
        local expected_karma = (before.karma[index] + karma_delta[index]) % 256
        if after.karma[index] ~= expected_karma then
            return "runner " .. tostring(index) .. " Karma expected " ..
                tostring(expected_karma) .. " but was " ..
                tostring(after.karma[index])
        end
    end

    local feature_specification = expected.runner_features or {}
    if before.runner_features == nil or after.runner_features == nil then
        return "runner feature snapshot is unavailable"
    end
    local feature_set = bitmask(feature_specification.set)
    local feature_clear = bitmask(feature_specification.clear)
    local feature_value = (before.runner_features & ~feature_clear) | feature_set
    if after.runner_features ~= feature_value then
        return "runner feature flags changed unexpectedly: expected " ..
            tostring(feature_value) .. " but was " ..
            tostring(after.runner_features)
    end
    if before.item_signature ~= after.item_signature then
        return "runner inventory/item state changed unexpectedly"
    end
    return nil
end

local function load_route(segment)
    local modules = {
        seg1 = "walkthrough.route_seg1",
        w3 = "walkthrough.route_w3",
        beat_w3_scene11_exit = "walkthrough.route_beat_w3_scene11_exit",
        beat_w1_stoker_bill = "walkthrough.route_beat_w1_stoker_bill",
        beat_w1_stoker_arrival = "walkthrough.route_beat_w1_stoker_arrival",
        beat_w1_puyallup_to_redmond_taxi = "walkthrough.route_beat_w1_puyallup_to_redmond_taxi",
        beat_w1_redmond_to_puyallup_taxi = "walkthrough.route_beat_w1_redmond_to_puyallup_taxi",
        beat_w1_tarislar_ask_shale = "walkthrough.route_beat_w1_tarislar_ask_shale",
        beat_w1_boris_reveal_alias = "walkthrough.route_beat_w1_boris_reveal_alias",
        beat_w3_salish_to_penumbra_taxi = "walkthrough.route_beat_w3_salish_to_penumbra_taxi",
        beat_w3_redmond_to_salish_taxi = "walkthrough.route_beat_w3_redmond_to_salish_taxi",
        beat_w3_owlfeather_first_contact = "walkthrough.route_beat_w3_owlfeather_first_contact",
        beat_w3_owlfeather_asks_about_michael = "walkthrough.route_beat_w3_owlfeather_asks_about_michael",
        beat_w3_owlfeather_aragorn_task_briefing = "walkthrough.route_beat_w3_owlfeather_aragorn_task_briefing",
        beat_w3_owlfeather_return_service = "walkthrough.route_beat_w3_owlfeather_return_service",
        beat_w3_aragorn_mitsuhama_contact = "walkthrough.route_beat_w3_aragorn_mitsuhama_contact",
        beat_w3_big_rhino_entry = "walkthrough.route_beat_w3_big_rhino_entry",
        beat_w3_mortimer_aragorn_question = "walkthrough.route_beat_w3_mortimer_aragorn_question",
        beat_w3_penumbra_to_salish_taxi = "walkthrough.route_beat_w3_penumbra_to_salish_taxi",
        beat_w3_mitsuhama_entry = "walkthrough.route_beat_w3_mitsuhama_entry",
        beat_w3_mitsuhama_elevator = "walkthrough.route_beat_w3_mitsuhama_elevator",
        beat_w3_mitsuhama_elevator_return = "walkthrough.route_beat_w3_mitsuhama_elevator_return",
        beat_w3_mitsuhama_return_to_field = "walkthrough.route_beat_w3_mitsuhama_return_to_field",
        beat_w4_salish_to_council_taxi = "walkthrough.route_beat_w4_salish_to_council_taxi",
        beat_w4_spirit_eyes_first_contact = "walkthrough.route_beat_w4_spirit_eyes_first_contact",
        beat_w4_spirit_eyes_horn_turn_in = "walkthrough.route_beat_w4_spirit_eyes_horn_turn_in",
        beat_w4_spirit_eyes_pelt_turn_in = "walkthrough.route_beat_w4_spirit_eyes_pelt_turn_in",
        beat_w4_serpent_scale = "walkthrough.route_beat_w4_serpent_scale",
        w3_b2 = "walkthrough.route_w3_b2",
        beat_w6_red_buffalo_gift = "walkthrough.route_beat_w6_red_buffalo_gift",
        beat_w7_renraku_map_overlay = "walkthrough.route_beat_w7_renraku_map_overlay",
        beat_w7_b16_ito_producer = "walkthrough.route_beat_w7_b16_ito_producer",
        beat_w7_b16_ito_live_fight = "walkthrough.route_beat_w7_b16_ito_live_fight",
        beat_w5_scene7_exit = "walkthrough.route_beat_w5_scene7_exit",
        beat_w5_cave_continuation = "walkthrough.route_beat_w5_cave_continuation",
        beat_w5_cave_room7_to_scene7 = "walkthrough.route_beat_w5_cave_room7_to_scene7",
        beat_w5_caleb_b13 = "walkthrough.route_beat_w5_caleb_b13",
        beat_w5_fuchi_entry = "walkthrough.route_beat_w5_fuchi_entry",
        beat_w5_fuchi_safe_reward = "walkthrough.route_beat_w5_fuchi_safe_reward",
        beat_w5_b23_holopix = "walkthrough.route_beat_w5_b23_holopix",
        beat_w5_b24_salish_taxi = "walkthrough.route_beat_w5_b24_salish_taxi",
        beat_w5_salish_to_downtown_taxi = "walkthrough.route_beat_w5_salish_to_downtown_taxi",
        beat_w5_b25_stark_admission = "walkthrough.route_beat_w5_b25_stark_admission",
        beat_w5_b29_cyberheart_turn_in = "walkthrough.route_beat_w5_b29_cyberheart_turn_in",
        beat_w6_sinsearach_first_audience = "walkthrough.route_beat_w6_sinsearach_first_audience",
        beat_w6_ilene_contact = "walkthrough.route_beat_w6_ilene_contact",
        beat_w6_gillian_gift_return = "walkthrough.route_beat_w6_gillian_gift_return",
        beat_w6_gillian_initiation = "walkthrough.route_beat_w6_gillian_initiation",
        beat_w6_gillian_frosty_referral = "walkthrough.route_beat_w6_gillian_frosty_referral",
        beat_w7_tarislar_apartments_first_entry = "walkthrough.route_beat_w7_tarislar_apartments_first_entry",
        beat_w7_tarislar_tomb_departure = "walkthrough.route_beat_w7_tarislar_tomb_departure",
        beat_w7_tarislar_paired_map_briefing = "walkthrough.route_beat_w7_tarislar_paired_map_briefing",
        beat_w7_tarislar_post_ito_briefing = "walkthrough.route_beat_w7_tarislar_post_ito_briefing",
        beat_w7_tarislar_renraku_map_briefing = "walkthrough.route_beat_w7_tarislar_renraku_map_briefing",
        beat_w8_tomb_antechamber_entry = "walkthrough.route_beat_w8_tomb_antechamber_entry",
        beat_w8_tomb_live_outro = "walkthrough.route_beat_w8_tomb_live_outro",
        w4 = "walkthrough.route_w4",
        w4_b6 = "walkthrough.route_w4_b6",
        w4_b7 = "walkthrough.route_w4_b7",
        w5_b13 = "walkthrough.route_w5_b13",
        w5_b13_icarus_entry = "walkthrough.route_w5_b13_icarus_entry",
        w5_b13_caleb = "walkthrough.route_w5_b13_caleb",
        w5_b23 = "walkthrough.route_w5_b23",
        w5_b24 = "walkthrough.route_w5_b24",
        w5_b25_gate = "walkthrough.route_w5_b25_gate",
        w5_b25_cave_entry = "walkthrough.route_w5_b25_cave_entry",
        w5_b25 = "walkthrough.route_w5_b25",
        w5_b29 = "walkthrough.route_w5_b29",
        w5_b14_entry = "walkthrough.route_w5_b14_entry",
        w6_b17 = "walkthrough.route_w6_b17",
        w6_b18 = "walkthrough.route_w6_b18",
        w6_b19 = "walkthrough.route_w6_b19",
        w6_b20 = "walkthrough.route_w6_b20",
        w5 = "walkthrough.route_w5",
        w6 = "walkthrough.route_w6",
        w7 = "walkthrough.route_w7",
        w8 = "walkthrough.route_w8",
    }
    local module = modules[segment]
    if module == nil then error("no Lua route for segment " .. segment) end
    return require(module)
end

local function run(host, segment, reach_mode)
    local route = load_route(segment)
    local steps = {}
    local last_step = "none"
    local first_broken = ""
    local failure = ""
    local last_milestone = 0
    local all_steps_passed = true
    local gate_passed = true

    local first_step = host.from_fixture and
        (route.resume_from_fixture_step or 1) or 1
    for step_index = first_step, #route.steps do
        local step = route.steps[step_index]
        if not reach_mode and step.gate == false then
            break
        end
        if step.setup ~= nil then
            local setup_ok, setup_error = xpcall(function()
                return step.setup(host)
            end, function(problem) return tostring(problem) end)
            if not setup_ok then
                error("walkthrough fixture setup failed for " .. step.id ..
                    ": " .. setup_error)
            end
        end
        host.begin_step(step.id)
        local before = host.snapshot()
        local ok, action_detail = xpcall(function()
            return step.run(host)
        end, function(problem) return tostring(problem) end)
        local after = host.snapshot()
        local host_result = host.end_step()
        local detail
        if not ok then
            detail = action_detail
        else
            detail = action_detail or "route action completed"
        end
        local validation_error = validate(before, after,
            step.expected or {}, host_result)
        if validation_error ~= nil then
            detail = detail .. "; " .. validation_error
        end
        local step_passed = ok and validation_error == nil
        if not step_passed and #host_result.source_events > 0 then
            local events = host_result.source_events
            local event_summary = events
            if #events > 24 then
                event_summary = {}
                for index = 1, 12 do
                    event_summary[#event_summary + 1] = events[index]
                end
                event_summary[#event_summary + 1] =
                    "... " .. tostring(#events - 24) .. " events omitted ..."
                for index = #events - 11, #events do
                    event_summary[#event_summary + 1] = events[index]
                end
            end
            detail = detail .. "; source_events=" ..
                table.concat(event_summary, ",")
        end
        steps[#steps + 1] = {
            id = step.id,
            milestone = step.milestone or last_milestone,
            passed = step_passed,
            frames = host_result.frames,
            before_summary = summary(before),
            after_summary = summary(after),
            detail = detail,
            stderr = host_result.stderr,
        }
        if step_passed then
            last_step = step.id
            last_milestone = step.milestone or last_milestone
        else
            all_steps_passed = false
            if step.gate ~= false then gate_passed = false end
            first_broken = step.id
            failure = detail
            break
        end
    end

    return {
        passed = gate_passed,
        complete = all_steps_passed,
        last_step = last_step,
        first_broken = first_broken,
        failure = failure,
        last_milestone = last_milestone,
        steps = steps,
        reach_mode = reach_mode,
    }
end

return { run = run }

-- Gang contracts are authored as a dedicated run module. Their offer text,
-- target pool, and random variant come from ROM tables F36FA and F3E7A.
local GangContract = {}

-- ROM 0x1F086 chooses one list and resource profile after an exact match.
-- The actor rows themselves are imported into world-content Lua data from
-- the pointer lists at these ROM locations.
local target_actor_groups = {
    [1] = {
        [9] = { list_offset = 0x1D6D4A, resource_profile = 0x122054 },
    },
    [2] = {
        [9] = { list_offset = 0x1D6D5E, resource_profile = 0x121E38 },
    },
    [3] = {
        [4] = { list_offset = 0x1D6D72, resource_profile = 0x122018 },
    },
}

function GangContract.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local target = support.choose_gang_contract_target()
    local variant = support.bounded_random(#support.gang_contract_texts)
    local offer = support.new_offer(
        "gang_contract",
        "gang",
        "resolve_the_selected_gang_contract",
        "return_to_the_johnson_after_the_gang_contract",
        { target = target },
        payment,
        source_record.runner_progress_delta)
    offer.context = context
    offer.variant = variant
    game.runs.registry.current_offer = offer
    return offer.id
end

function GangContract.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "gang_contract")
    local target = offer.route.target
    game.ui.say_text(
        "Go to " .. target.location_name .. " " .. target.area_suffix ..
        "|" .. game.modules.run_support.gang_contract_texts[offer.variant + 1],
        true)
    return game.modules.run_support.present_run_acceptance(
        offer, "for completing this gang contract.")
end

function GangContract.resolve_field_event(offer, map_id, event_id)
    local route = type(offer) == "table" and offer.route or nil
    local target = type(route) == "table" and route.target or nil
    if type(target) ~= "table" or
       type(target.area_id) ~= "number" or
       type(target.location_id) ~= "number" then
        return { action = "no_action" }
    end

    -- ROM 0x1F086 matches the current area/event against FFFBEE/FFFBEF.
    -- At a match it selects the gang-specific actor/resource list, then
    -- presents the entry passage from 0xFE0DE before the area route resumes.
    local actor_group = type(target_actor_groups[target.area_id]) == "table"
        and target_actor_groups[target.area_id][target.location_id] or nil
    if map_id == target.area_id and event_id == target.location_id and
       actor_group ~= nil then
        return {
            action = "entry_dialogue",
            dialogue_module = "run_gang_contract",
            dialogue_function = "present_target_entry",
            actor_group_area = target.area_id,
            actor_group_event = target.location_id,
            actor_list_offset = actor_group.list_offset,
            actor_resource_profile = actor_group.resource_profile,
            activity_scene_id = 15,
        }
    end
    return { action = "no_action" }
end

function GangContract.present_target_entry()
    local offer = game.runs.registry.active_offer
    assert(offer ~= nil and offer.id == "gang_contract")
    -- ROM text stream 0xFE0DE, selected by the descriptor at 0xFDE26.
    game.ui.say_text(
        "<After politely discussing the situation with the ornery gang " ..
        "leader, his reply is:>\n\n" ..
        "  Slot off, frag-face!!!\n\n" ..
        "<Guess what happens next.>", true)
    return "shown"
end

return GangContract

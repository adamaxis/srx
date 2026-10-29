-- Gang contracts are authored as a dedicated run module. Their offer text,
-- target pool, and random variant come from ROM tables F36FA and F3E7A.
local GangContract = {}

function GangContract.prepare_offer(contact_id, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local target = support.choose_gang_contract_target()
    local contract_variant_id = support.choose_gang_contract_variant_id()
    local offer = support.new_offer(
        "gang_contract",
        "gang",
        "resolve_the_selected_gang_contract",
        "return_to_the_johnson_after_the_gang_contract",
        { target = target },
        payment,
        source_record.runner_progress_delta)
    offer.contact_id = contact_id
    offer.contract_variant_id = contract_variant_id
    game.runs.registry.current_offer = offer
    return offer.id
end

function GangContract.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "gang_contract")
    local target = offer.route.target
    local variant_text = game.modules.run_support.gang_contract_variant_text(
        offer.contract_variant_id)
    local details = game.modules.run_support.location_details(
        target.location_id)
    assert(details ~= nil and variant_text ~= nil,
        "gang contract has invalid semantic identities")
    game.ui.say_text(
        "Go to " .. details.name .. " " .. details.region_suffix ..
        "|" .. variant_text,
        true)
    return game.modules.run_support.present_run_acceptance(
        offer, "for completing this gang contract.")
end

function GangContract.resolve_field_event(offer, location_id)
    local route = type(offer) == "table" and offer.route or nil
    local target = type(route) == "table" and route.target or nil
    if type(target) ~= "table" or
       type(target.location_id) ~= "string" then
        return { action = "no_action" }
    end

    -- ROM 0x1F086 selects a gang-specific actor list after the target place
    -- matches. The importer resolves its source pointers to actor records;
    -- Lua carries only this stable group identity.
    local support = game.modules.run_support
    local actor_group_id = support.gang_actor_group_id(target.location_id)
    if location_id == target.location_id and actor_group_id ~= nil then
        return {
            action = "entry_dialogue",
            dialogue_module = "run_gang_contract",
            dialogue_function = "present_target_entry",
            actor_group_id = actor_group_id,
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

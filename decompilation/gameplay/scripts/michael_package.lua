-- Michael's package is a quest hook that can run when a motel is entered.
local E = require("enums")
local StoryFlags = require("story_flags")
local MichaelPackage = {}

function MichaelPackage.motel_entry()
    if game.state.michael_debt_paid and
       game.state.michael_package_collected then
        return "ordinary_motel"
    end

    if not game.state.opening_dialogue_seen then
        game.state.opening_dialogue_seen = true
        return "opening_dialogue"
    end

    return "package_offer"
end

function MichaelPackage.stoker_interaction()
    -- Title-service table entry 0 (0x53AB6): 0x53A78 stores the index in
    -- FFDD64 before the handler runs; command 0x05's paid branch marks it.
    StoryFlags.begin_trigger(E.StoryFlag.MOTEL_BILL_PAID)
    local phase = MichaelPackage.motel_entry()
    if phase == "opening_dialogue" then
        game.ui.say("arrival")
    end

    local choice = game.modules.motel.manager_interaction()
    if choice ~= "package" then
        return choice
    end

    local prepared =
        game.services.call("michael_package", "prepare_debt_offer")
    if prepared ~= "ready" then
        return prepared
    end

    local debt_choice = game.ui.menu(
        "debt", { "debt-payment", "jump-house" })
    if debt_choice == "debt-payment" then
        local result = game.services.call("michael_package", "pay_debt")
        if result == "paid" then
            -- Command 0x05 paid branch: bsr 0x2196C at 0x21410 [C]. The C++
            -- conversion of 0x213FA performs the same bset inside its funds
            -- sequence (-250, mark, +500); this story-layer call is
            -- idempotent and keeps FFFC1E bookkeeping on the Lua side.
            StoryFlags.apply_dialogue_command(0x05, { paid = true })
            game.state.michael_debt_paid = true
            game.state.michael_package_collected = true
            game.ui.say("package-handoff")
        elseif result == "insufficient" then
            game.state.michael_debt_paid = false
            game.state.michael_package_collected = false
            game.ui.say("jump-house")
        end
    elseif debt_choice == "jump-house" then
        local result =
            game.services.call("michael_package", "jump_house_referral")
        if result == "ready" then
            game.ui.say("jump-house")
        end
    end

    return debt_choice
end

function MichaelPackage.motel_manager_choices()
    if game.location.type ~= "motel" or
       game.location.id ~= "stoker_motel" then
        return {}
    end
    if not game.state.michael_debt_paid or
       not game.state.michael_package_collected then
        return { "package" }
    end
    return {}
end

return MichaelPackage

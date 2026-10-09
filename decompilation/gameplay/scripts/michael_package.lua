-- Michael's package is a quest hook that can run when a motel is entered.
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

function MichaelPackage.opening_interaction()
    game.ui.say("stoker", "arrival")
    return game.modules.motel.manager_interaction()
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

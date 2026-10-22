package.path = "decompilation/gameplay/tests_lua/?.lua;" .. package.path
local Harness = require("support.vid_phone_graph_harness")

local function reset(test, called, choices, reputation, affiliations)
    test:reset(called, choices, nil, reputation, affiliations)
end

local function test_faradouchi()
    local test = Harness.new("mr_faradouchi")

    -- Reputation below five produces the ROM's rejection, without recording
    -- the contact. The next call therefore takes the first-call path again.
    reset(test, false, {}, 4)
    test:expect_finished()
    test:expect_page("first_call")
    test:expect_page("low_reputation")
    assert(game.state.vid_phone_called_7 == false)
    assert(game.state.vid_phone_affiliation_mafia == false)

    -- The rival-affiliation bit is checked before Reputation.
    reset(test, false, {}, 5, { yakuza = true })
    test:expect_finished()
    test:expect_page("first_call")
    test:expect_page("yakuza_rejection")
    assert(game.state.vid_phone_called_7 == false)
    assert(#test.menus == 0)

    reset(test, false, {}, 5)
    game.state.vid_phone_affiliation_yakuza = nil
    local boundary = test:expect_boundary()
    assert(boundary.reason:match("vid_phone_affiliation_yakuza") ~= nil)
    assert(#test.menus == 0 and game.state.vid_phone_called_7 == false)

    -- Declining the pledge also leaves the source history bit clear.
    reset(test, false, { "no" }, 5)
    test:expect_finished()
    test:expect_menu("call_mr_faradouchi_mafia_pledge")
    assert(game.state.vid_phone_called_7 == false)
    assert(game.state.vid_phone_affiliation_mafia == false)

    reset(test, false, { "yes" }, 5)
    test:expect_finished()
    test:expect_page("pledge_accepted")
    assert(game.state.vid_phone_called_7 == true)
    assert(game.state.vid_phone_affiliation_mafia == true)
    assert(game.state.vid_phone_affiliation_yakuza == false)

    reset(test, false, { "yes" }, 5)
    game.state.vid_phone_affiliation_mafia = nil
    boundary = test:expect_boundary()
    assert(boundary.reason:match("state_boundary") ~= nil)
    assert(game.state.vid_phone_called_7 == false,
        "unknown affiliation state must not partially write call history")

    reset(test, true, {})
    test:expect_finished()
    test:expect_page("repeat_call")
    assert(#test.menus == 0,
        "accepted Faradouchi contact should use the repeat stream")

    reset(test, false, {}, "unknown")
    boundary = test:expect_boundary()
    assert(boundary.reason:match("runner.skill.reputation") ~= nil)
end

local function test_agira()
    local test = Harness.new("agira_tetsumi")

    reset(test, false, {}, 4)
    test:expect_finished()
    test:expect_page("first_call")
    test:expect_page("low_reputation")
    assert(game.state.vid_phone_called_8 == false)
    assert(game.state.vid_phone_affiliation_yakuza == false)

    reset(test, false, {}, 5, { mafia = true })
    test:expect_finished()
    test:expect_page("first_call")
    test:expect_page("mafia_rejection")
    assert(game.state.vid_phone_called_8 == false)
    assert(#test.menus == 0)

    reset(test, false, {}, 5)
    game.state.vid_phone_affiliation_mafia = nil
    local boundary = test:expect_boundary()
    assert(boundary.reason:match("vid_phone_affiliation_mafia") ~= nil)
    assert(#test.menus == 0 and game.state.vid_phone_called_8 == false)

    reset(test, false, { "no" }, 5)
    test:expect_finished()
    test:expect_menu("call_agira_tetsumi_yakuza_pledge")
    assert(game.state.vid_phone_called_8 == false)
    assert(game.state.vid_phone_affiliation_yakuza == false)

    reset(test, false, { "yes" }, 5)
    test:expect_finished()
    test:expect_page("pledge_accepted")
    assert(game.state.vid_phone_called_8 == true)
    assert(game.state.vid_phone_affiliation_yakuza == true)
    assert(game.state.vid_phone_affiliation_mafia == false)

    reset(test, false, { "yes" }, 5)
    game.state.vid_phone_affiliation_yakuza = nil
    boundary = test:expect_boundary()
    assert(boundary.reason:match("state_boundary") ~= nil)
    assert(game.state.vid_phone_called_8 == false,
        "unknown affiliation state must not partially write call history")

    reset(test, true, {})
    test:expect_finished()
    test:expect_page("repeat_call")
    assert(#test.menus == 0,
        "accepted Agira contact should use the repeat stream")

    reset(test, false, {}, "unknown")
    boundary = test:expect_boundary()
    assert(boundary.reason:match("runner.skill.reputation") ~= nil)
end

test_faradouchi()
test_agira()
print("Faradouchi and Agira Vid-Phone JSON graph permutations passed")

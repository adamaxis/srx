-- Generic B input route for the former startup, Hotel, hospital, and dialogue
-- dispatchers. Source-mode actions and the optional cancel policy share the
-- Lua menu route boundary.
local E = require("enums")
local P, S, A, T = E.Pad, E.MenuScreen, E.MenuInputAction,
    E.MenuTransition
require("menu.field_init")
local Route = require("menu.route")
local Hotel = require("hotel")

local checks, failures = 0, 0
local function check(value, label)
    checks = checks + 1
    if not value then
        failures = failures + 1
        io.write("FAIL ", label, "\n")
    end
end

local function routed(screen, events, policy, depth, choices_present)
    return Route.external_input_disposition(
        screen, events, policy or E.MenuBPolicyCode.ROM, depth or 1,
        choices_present)
end

-- ROM-mode B remains owned by each source menu. The source-backed exceptions
-- are explicit screen actions; the optional policy is applied in route.lua.
do
    local input = routed(S.STARTUP_OPTIONS, P.B)
    check(input[1] == T.NONE and input[2] == A.FORWARD,
          "startup B reaches its selected source row")
    input = routed(S.HOSPITAL_OPTIONS, P.B)
    check(input[1] == T.NONE and input[2] == A.IGNORE,
          "Hospital Options B is ignored in ROM mode")
    input = routed(S.HOSPITAL_OPTIONS, P.C)
    check(input[1] == T.NONE and input[2] == A.BACK,
          "Hospital Options C reaches the field mode action")
    input = routed(S.HOSPITAL_MEDICAL_ATTENTION, P.B)
    check(input[1] == T.NONE and input[2] == A.BACK,
          "medical attention B returns to Hospital Options")
    input = routed(S.HOSPITAL_LEAVE_CONFIRMATION, P.B)
    check(input[1] == T.NONE and input[2] == A.BACK,
          "hospital leave confirmation B confirms the selection")
    input = routed(S.DIALOGUE_BUTTON_CHOICES, P.B)
    check(input[1] == T.NONE and input[2] == A.BUTTON_B,
          "scripted button-choice B selects its B row")
    input = routed(S.DIALOGUE_BUTTON_CHOICES, P.A | P.B)
    check(input[1] == T.NONE and input[2] == A.BUTTON_A,
          "scripted button choices preserve A-before-B priority")
    input = routed(S.DIALOGUE_BUTTON_CHOICES, P.B | P.C)
    check(input[1] == T.NONE and input[2] == A.BUTTON_B,
          "scripted button choices preserve B-before-C priority")
    input = routed(S.DIALOGUE_BUTTON_CHOICES, P.START)
    check(input[1] == T.NONE and input[2] == A.BUTTON_A,
          "scripted button-choice Start keeps its existing first-row action")
    input = routed(S.DIALOGUE_CURSOR_CHOICES, P.B)
    check(input[1] == T.NONE and input[2] == A.BACK,
          "scripted cursor-choice B follows its caller")
    input = routed(S.DIALOGUE_HOSTED_CHOICES, P.B)
    check(input[1] == T.NONE and input[2] == A.CONFIRM,
          "hosted choice-list B confirms the selected row")
    input = routed(S.DIALOGUE_VID_PHONE_CONTACTS, P.B)
    check(input[1] == T.NONE and input[2] == A.CONFIRM,
          "Vid-Phone B confirms the selected contact")
    input = routed(S.DIALOGUE_VID_PHONE_CONTACTS, P.A | P.C)
    check(input[1] == T.NONE and input[2] == A.CONFIRM,
          "Vid-Phone confirm beats the ignored C action")
    input = routed(S.DIALOGUE_VID_PHONE_CONTACTS, P.START | P.C)
    check(input[1] == T.NONE and input[2] == A.CANCEL,
          "Vid-Phone Start keeps priority over C")
    input = routed(S.HOSPITAL_MEDICAL_ATTENTION, P.A,
                   E.MenuBPolicyCode.ROM, 2, false)
    check(input[1] == T.NONE and input[2] == A.IGNORE,
          "empty Hospital Medical Attention accepts no non-B action")
    input = routed(S.HOSPITAL_MEDICAL_ATTENTION, P.UP | P.B,
                   E.MenuBPolicyCode.ROM, 2, false)
    check(input[1] == T.NONE and input[2] == A.BACK,
          "empty Hospital Medical Attention preserves its B-only return")
end

-- Hotel's Lua handlers run through the same route dispatch. Options ignores B;
-- the child confirmation uses B as a source confirm when No is selected.
do
    Hotel.reset_options()
    local options = Hotel.advance_options(P.B, E.MenuBPolicyCode.ROM)
    check(options[1] == 0 and options[2] == 0,
          "Hotel Options ROM B does not change the menu")
    Hotel.reset_options()
    Hotel.advance_options(P.DOWN, E.MenuBPolicyCode.ROM)
    local entered = Hotel.advance_options(P.A, E.MenuBPolicyCode.ROM)
    check(entered[1] == 2,
          "Hotel Options A opens Check Out on its selected row")
    local checkout = Hotel.advance_checkout(P.B, E.MenuBPolicyCode.ROM)
    check(checkout[1] == 2 and checkout[2] == 1,
          "Hotel Check Out ROM B confirms the initial No row")
end

-- The same route closes a root or pops one child before any ROM handler sees B.
local always_screens = {
    S.STARTUP_OPTIONS, S.HOTEL_OPTIONS, S.HOTEL_CHECKOUT,
    S.HOSPITAL_OPTIONS, S.HOSPITAL_MEDICAL_ATTENTION,
    S.HOSPITAL_LEAVE_CONFIRMATION, S.DIALOGUE_BUTTON_CHOICES,
    S.DIALOGUE_CURSOR_CHOICES, S.DIALOGUE_VID_PHONE_CONTACTS,
    S.DIALOGUE_HOSTED_CHOICES,
}
for _, screen in ipairs(always_screens) do
    local root = routed(screen, P.B, E.MenuBPolicyCode.ALWAYS_CANCEL, 1)
    check(root[1] == T.EXIT and root[2] == A.MENU_CANCEL,
          "ALWAYS_CANCEL closes a root for screen " .. screen)
    local child = routed(screen, P.B, E.MenuBPolicyCode.ALWAYS_CANCEL, 2)
    check(child[1] == T.POP and child[2] == A.MENU_CANCEL,
          "ALWAYS_CANCEL pops one child for screen " .. screen)
end

do
    Hotel.reset_options()
    local options = Hotel.advance_options(
        P.B, E.MenuBPolicyCode.ALWAYS_CANCEL)
    check(options[1] == 5, "Hotel Options ALWAYS_CANCEL closes its root")
    Hotel.reset_options()
    Hotel.advance_options(P.DOWN, E.MenuBPolicyCode.ROM)
    Hotel.advance_options(P.A, E.MenuBPolicyCode.ROM)
    local checkout = Hotel.advance_checkout(
        P.B, E.MenuBPolicyCode.ALWAYS_CANCEL)
    check(checkout[1] == 4,
          "Hotel Check Out ALWAYS_CANCEL returns to Hotel Options")
end

io.write(string.format("generic menu input tests: %d/%d passed\n",
                       checks - failures, checks))
os.exit(failures == 0 and 0 or 1)

local E = require("enums")

local STOKER_DEBT_NUYEN = 250

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Stoker dialogue advance failed")
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        local state = host.snapshot()
        if state.panel_text:find(text, 1, true) then return end
        advance(host, 1)
    end
    error("Stoker panel did not contain " .. string.format("%q", text))
end

return {
    steps = {
        {
            id = "w1_pay_stoker_bill",
            milestone = 1,
            expected = {
                scene = E.Scene.STOKER_MOTEL,
                nuyen_delta = STOKER_DEBT_NUYEN * 2,
                karma_delta = { 1, 1, 1 },
                facts = { [E.StoryFact.MOTEL_BILL_PAID] = true },
                flags = {
                    story = { set = { E.StoryFact.MOTEL_BILL_PAID } },
                    mode = {
                        set = {
                            E.FieldModeFlagBit.BIT_2,
                            E.FieldModeFlagBit.BIT_3,
                            E.FieldModeFlagBit.BIT_4,
                        },
                        clear = {
                            E.FieldModeFlagBit.BIT_0,
                            E.FieldModeFlagBit.BIT_1,
                        },
                    },
                },
                runner_features = {
                    set = { E.RunnerFeatureFlagBit.CYBERDECK_PRESENT },
                },
            },
            run = function(host)
                assert(host.grant_nuyen(STOKER_DEBT_NUYEN)) -- Setup grant: provide the 250-nuyen balance for Stoker's bill.
                wait_panel(host, "battered sign reads Stoker's Coffin Motel")
                tap(host, "A")
                wait_panel(host, "The manager yawns.")
                tap(host, "C")
                wait_panel(host, "You pay up the 250$ he owes")
                tap(host, "A")
                wait_panel(host, "crumpled plastic box")
                local pages = {
                    "letter addressed to",
                    "Allegiance Alpha",
                    "DocWagon homing beacon",
                    "10% of the wearer's credstick balance",
                    "Dr. Ivanna",
                    "Mr. Caleb",
                    "credstick hidden under the holopix",
                }
                for _, text in ipairs(pages) do
                    tap(host, "A")
                    wait_panel(host, text)
                end
                tap(host, "A")
                if host.snapshot().dialogue_open then
                    tap(host, "A")
                end
                assert(not host.snapshot().dialogue_open,
                    "Michael's package pages did not return to the field")
                return "paid Stoker's bill and read the complete Michael package handoff"
            end,
        },
    },
}

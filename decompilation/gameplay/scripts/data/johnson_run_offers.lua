-- [C] ROM F3EBC stores eight six-byte offer records for each FFF0AE context;
-- the first byte indexes the handler table at F3DC6. The Lua model imports
-- each supported row as a named gameplay variant, so the selector byte never
-- becomes runtime script state.
local E = require("enums")

-- F3EBC's first byte indexes the handler pointers at F3DC6. The importer maps
-- each supported handler/offer variant to one of these gameplay identities;
-- the runtime does not carry that source byte into Lua.
local run_by_variant_id = {
    ghoul_bounty = {
        id = E.RunId.GHOUL_BOUNTY,
    },
    bodyguard_escort = {
        id = E.RunId.BODYGUARD_ESCORT,
    },
    package_delivery = {
        id = E.RunId.PACKAGE_DELIVERY,
    },
    gang_contract = {
        id = E.RunId.GANG_CONTRACT,
    },
    package_recovery = {
        id = E.RunId.PACKAGE_RECOVERY,
        target_pool_id = "four_site",
    },
    package_recovery_two_site_pool = {
        id = E.RunId.PACKAGE_RECOVERY,
        target_pool_id = "two_site",
    },
    package_recovery_renraku = {
        id = E.RunId.PACKAGE_RECOVERY,
        target_pool_id = "renraku",
    },
    employee_recruitment = {
        id = E.RunId.EMPLOYEE_RECRUITMENT,
        target_pool_id = "four_site",
    },
    employee_recruitment_two_site_pool = {
        id = E.RunId.EMPLOYEE_RECRUITMENT,
        target_pool_id = "two_site",
    },
    employee_recruitment_renraku = {
        id = E.RunId.EMPLOYEE_RECRUITMENT,
        target_pool_id = "renraku",
    },
    matrix_simple = {
        id = E.RunId.MATRIX_OBJECTIVE,
        matrix_band_id = "simple",
    },
    matrix_moderate = {
        id = E.RunId.MATRIX_OBJECTIVE,
        matrix_band_id = "moderate",
    },
    matrix_expert = {
        id = E.RunId.MATRIX_OBJECTIVE,
        matrix_band_id = "expert",
    },
}

local function row(variant_id, progress, base, negotiation)
    local variant = assert(run_by_variant_id[variant_id],
        "unknown gameplay offer variant: " .. tostring(variant_id))
    return {
        variant_id = variant_id,
        run_id = variant.id,
        target_pool_id = variant.target_pool_id,
        matrix_band_id = variant.matrix_band_id,
        runner_progress_delta = progress,
        base_payment = base,
        negotiation_scale = negotiation,
    }
end

return {
    contacts = {
        [E.RunContact.JUMP_HOUSE] = {
            row("ghoul_bounty", 1, 10, 3),
            row("bodyguard_escort", 1, 45, 1),
            row("package_delivery", 1, 35, 1),
            row("bodyguard_escort", 1, 50, 1),
            row("package_delivery", 1, 40, 1),
            row("bodyguard_escort", 1, 55, 1),
            row("package_delivery", 1, 45, 1),
            row("ghoul_bounty", 1, 20, 2),
        },
        [E.RunContact.MATCHSTICK] = {
            row("matrix_moderate", 3, 2600, 4),
            row("bodyguard_escort", 1, 200, 2),
            row("employee_recruitment", 2, 500, 5),
            row("package_recovery", 2, 600, 5),
            row("employee_recruitment_two_site_pool", 4, 900, 5),
            row("package_recovery_two_site_pool", 4, 1000, 5),
            row("package_delivery", 1, 180, 2),
            row("gang_contract", 2, 160, 3),
        },
        [E.RunContact.BIG_RHINO] = {
            row("matrix_simple", 2, 425, 1),
            row("gang_contract", 2, 190, 2),
            row("package_delivery", 1, 180, 1),
            row("ghoul_bounty", 1, 40, 1),
            row("package_recovery", 2, 550, 4),
            row("employee_recruitment", 2, 450, 4),
            row("bodyguard_escort", 1, 200, 1),
            row("package_delivery", 1, 190, 1),
        },
        [E.RunContact.WANDERER] = {
            row("gang_contract", 1, 230, 1),
            row("matrix_moderate", 2, 3000, 2),
            row("matrix_expert", 4, 6350, 5),
            row("package_delivery", 0, 250, 1),
            row("package_recovery_two_site_pool", 3, 1700, 4),
            row("package_recovery_renraku", 5, 4500, 10),
            row("employee_recruitment_renraku", 5, 4000, 10),
            row("matrix_simple", 1, 500, 1),
        },
        [E.RunContact.ICARUS_DESCENDING] = {
            row("matrix_moderate", 3, 2550, 5),
            row("matrix_expert", 5, 6000, 10),
            row("employee_recruitment_two_site_pool", 4, 1200, 5),
            row("package_recovery_two_site_pool", 4, 1400, 5),
            row("employee_recruitment_renraku", 6, 3500, 15),
            row("package_recovery_renraku", 6, 3850, 15),
            row("matrix_expert", 5, 6100, 10),
            row("matrix_moderate", 4, 2750, 5),
        },
    },
    run_by_variant_id = run_by_variant_id,
}

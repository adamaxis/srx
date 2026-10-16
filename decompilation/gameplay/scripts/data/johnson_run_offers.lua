-- ROM F3EBC: eight six-byte offer records for each FFF0AE context.
-- Fields are the source initializer ID, runner progress delta, base nuyen,
-- and negotiation multiplier. This is authored runtime data; the ROM address
-- is retained here only as provenance.
local E = require("enums")
local I = E.RunOfferInitializer

local function row(initializer, progress, base, negotiation)
    return {
        source_initializer = initializer,
        runner_progress_delta = progress,
        base_payment = base,
        negotiation_scale = negotiation,
    }
end

return {
    contexts = {
        [E.RunOfferContext.JUMP_HOUSE] = {
            row(I.GHOUL_BOUNTY, 1, 10, 3),
            row(I.BODYGUARD_ESCORT, 1, 45, 1),
            row(I.PACKAGE_DELIVERY, 1, 35, 1),
            row(I.BODYGUARD_ESCORT, 1, 50, 1),
            row(I.PACKAGE_DELIVERY, 1, 40, 1),
            row(I.BODYGUARD_ESCORT, 1, 55, 1),
            row(I.PACKAGE_DELIVERY, 1, 45, 1),
            row(I.GHOUL_BOUNTY, 1, 20, 2),
        },
        [E.RunOfferContext.SCENE_18] = {
            row(I.MATRIX_MODERATE, 3, 2600, 4),
            row(I.BODYGUARD_ESCORT, 1, 200, 2),
            row(I.EMPLOYEE_RECRUITMENT, 2, 500, 5),
            row(I.PACKAGE_RECOVERY, 2, 600, 5),
            row(I.EMPLOYEE_RECRUITMENT_TWO_SITE_POOL, 4, 900, 5),
            row(I.PACKAGE_RECOVERY_TWO_SITE_POOL, 4, 1000, 5),
            row(I.PACKAGE_DELIVERY, 1, 180, 2),
            row(I.GANG_CONTRACT, 2, 160, 3),
        },
        [E.RunOfferContext.BIG_RHINO] = {
            row(I.MATRIX_SIMPLE, 2, 425, 1),
            row(I.GANG_CONTRACT, 2, 190, 2),
            row(I.PACKAGE_DELIVERY, 1, 180, 1),
            row(I.GHOUL_BOUNTY, 1, 40, 1),
            row(I.PACKAGE_RECOVERY, 2, 550, 4),
            row(I.EMPLOYEE_RECRUITMENT, 2, 450, 4),
            row(I.BODYGUARD_ESCORT, 1, 200, 1),
            row(I.PACKAGE_DELIVERY, 1, 190, 1),
        },
        [E.RunOfferContext.WANDERER] = {
            row(I.GANG_CONTRACT, 1, 230, 1),
            row(I.MATRIX_MODERATE, 2, 3000, 2),
            row(I.MATRIX_EXPERT, 4, 6350, 5),
            row(I.PACKAGE_DELIVERY, 0, 250, 1),
            row(I.PACKAGE_RECOVERY_TWO_SITE_POOL, 3, 1700, 4),
            row(I.PACKAGE_RECOVERY_RENRAKU, 5, 4500, 10),
            row(I.EMPLOYEE_RECRUITMENT_RENRAKU, 5, 4000, 10),
            row(I.MATRIX_SIMPLE, 1, 500, 1),
        },
        [E.RunOfferContext.ICARUS_DESCENDING] = {
            row(I.MATRIX_MODERATE, 3, 2550, 5),
            row(I.MATRIX_EXPERT, 5, 6000, 10),
            row(I.EMPLOYEE_RECRUITMENT_TWO_SITE_POOL, 4, 1200, 5),
            row(I.PACKAGE_RECOVERY_TWO_SITE_POOL, 4, 1400, 5),
            row(I.EMPLOYEE_RECRUITMENT_RENRAKU, 6, 3500, 15),
            row(I.PACKAGE_RECOVERY_RENRAKU, 6, 3850, 15),
            row(I.MATRIX_EXPERT, 5, 6100, 10),
            row(I.MATRIX_MODERATE, 4, 2750, 5),
        },
    },
    run_by_initializer = {
        [I.GHOUL_BOUNTY] = { type = E.RunType.GHOUL_BOUNTY, id = "ghoul_bounty" },
        [I.BODYGUARD_ESCORT] = { type = E.RunType.BODYGUARD_ESCORT, id = "bodyguard_escort" },
        [I.PACKAGE_DELIVERY] = { type = E.RunType.PACKAGE_DELIVERY, id = "package_delivery" },
        [I.GANG_CONTRACT] = { type = E.RunType.GANG_CONTRACT, id = "gang_contract" },
        [I.PACKAGE_RECOVERY] = { type = E.RunType.PACKAGE_RECOVERY, id = "package_recovery" },
        [I.EMPLOYEE_RECRUITMENT] = { type = E.RunType.EMPLOYEE_RECRUITMENT, id = "employee_recruitment" },
        [I.MATRIX_SIMPLE] = { type = E.RunType.MATRIX_OBJECTIVE, id = "matrix_objective" },
        [I.MATRIX_MODERATE] = { type = E.RunType.MATRIX_OBJECTIVE, id = "matrix_objective" },
        [I.MATRIX_EXPERT] = { type = E.RunType.MATRIX_OBJECTIVE, id = "matrix_objective" },
        [I.PACKAGE_RECOVERY_TWO_SITE_POOL] = { type = E.RunType.PACKAGE_RECOVERY, id = "package_recovery" },
        [I.PACKAGE_RECOVERY_RENRAKU] = { type = E.RunType.PACKAGE_RECOVERY, id = "package_recovery" },
        [I.EMPLOYEE_RECRUITMENT_TWO_SITE_POOL] = { type = E.RunType.EMPLOYEE_RECRUITMENT, id = "employee_recruitment" },
        [I.EMPLOYEE_RECRUITMENT_RENRAKU] = { type = E.RunType.EMPLOYEE_RECRUITMENT, id = "employee_recruitment" },
    },
}

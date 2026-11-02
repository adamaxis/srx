-- Neutral Scene-10 default bit-4 contact row.
-- [C] 0xFE680 selects descriptor 0xFF12C, whose first text record is 0x102424.
local E = require("enums")

return {
    [E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL] =
        "<Ilene Two Fists is here.>||What can I do for you, Joshua.",
}

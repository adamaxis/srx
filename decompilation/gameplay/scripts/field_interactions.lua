-- Shared field-object prompt choices. Native code validates these IDs and
-- maps them to typed interaction commands; ROM-derived labels stay in the
-- shared script-text resource.
local FieldInteractions = {}

function FieldInteractions.maglock_intro()
    return { "more" }, nil, {
        "field.maglock.option.more",
    }
end

function FieldInteractions.maglock_actions()
    return { "electronics", "grenade", "walk-away" }, nil, {
        "field.interaction.option.electronics",
        "field.interaction.option.grenade",
        "field.interaction.option.walk-away",
    }
end

function FieldInteractions.security_terminal_actions()
    return { "electronics", "walk-away" }, nil, {
        "field.interaction.option.electronics",
        "field.interaction.option.walk-away",
    }
end

function FieldInteractions.text_keys()
    return {
        door_title = "field.maglock.door.title",
        no_passkey = "field.maglock.no-passkey",
        insufficient_passkey = "field.maglock.insufficient-passkey",
        passkey_result = "field.maglock.passkey.result",
        electronics_no_skill = "field.maglock.electronics.no-skill",
        electronics_success = "field.maglock.electronics.success",
        electronics_failure = "field.maglock.electronics.failure",
        electronics_alert = "field.maglock.electronics.alert",
        grenade_no_item = "field.maglock.grenade.unavailable",
        grenade_result = "field.maglock.grenade.result",
        unlocked_door = "field.maglock.already-unlocked",
        security_title = "field.security-terminal.title",
        security_failure = "field.security-terminal.failed",
        security_alarm_disabled = "field.security-terminal.alarm-disabled",
        security_system_disabled = "field.security-terminal.system-disabled",
        security_files = "field.security-terminal.files",
        security_floor_first = "field.security-terminal.floor.0",
        security_floor_second = "field.security-terminal.floor.1",
        security_floor_third = "field.security-terminal.floor.2",
        security_floor_fourth = "field.security-terminal.floor.3",
        security_floor_fifth = "field.security-terminal.floor.4",
        security_floor_sixth = "field.security-terminal.floor.5",
    }
end

return FieldInteractions

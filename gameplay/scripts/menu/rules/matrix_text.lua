-- ROM 0x1669A2, 0x166AA5, 0x1666A8, 0x1666DD, and 0x166A44, decoded from US ROM text streams.
-- Pipes preserve each 0x80-family line/blank-row separator for the presenter.
local M = {
    ACTION_FAILED = "  You fail to|operate the node| successfully!",
    ALERT_CANCELLED = "ALERT CANCELLED||SYSTEM CHECKS OK||RESUME ACTIVITIES",
    LOCKOUT = "   AFFIRMATIVE|||NODE ACCESS LOCKED|||OVERRIDE SEQUENCE||    DISABLED",
    NO_ALERTS = " NO ALERTS ARE||   ACTIVATED|||RUN DIAGNOSTICS||FOR CONFIRMATION",
    SHUTDOWN = "  INITIATING||SYSTEM  SHUTDOWN|| IN - - SECONDS",
}
M.BY_ADDRESS = {
    [0x1669A2] = M.LOCKOUT,
    [0x166AA5] = M.ACTION_FAILED,
    [0x1666A8] = M.ALERT_CANCELLED,
    [0x1666DD] = M.NO_ALERTS,
    [0x166A44] = M.SHUTDOWN,
}
function M.from_rom(address) return M.BY_ADDRESS[address] end
return M

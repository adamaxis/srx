-- ROM 0x16684A, 0x1669A2, 0x166AA5, 0x1666A8, 0x1666DD, and 0x166A44, decoded from US ROM text streams.
-- Pipes preserve each 0x80-family line/blank-row separator for the presenter.
local M = {
    ACTION_FAILED = "  You fail to|operate the node| successfully!",
    ALERT_CANCELLED = "ALERT CANCELLED||SYSTEM CHECKS OK||RESUME ACTIVITIES",
    ERASE_OBJECTIVE = "  The recog-code| rapidly finds the|data and zaps it to| electron heaven!!|| Run accomplished!",
    ERASED = "You find a file that| appears important| and obliterate it|  from memory!!!|| Score one for the|downtrodden hacker!",
    LOCKOUT = "   AFFIRMATIVE|||NODE ACCESS LOCKED|||OVERRIDE SEQUENCE||    DISABLED",
    NO_ALERTS = " NO ALERTS ARE||   ACTIVATED|||RUN DIAGNOSTICS||FOR CONFIRMATION",
    OFFLINE = "NODE IS OFFLINE",
    SHUTDOWN = "  INITIATING||SYSTEM  SHUTDOWN|| IN - - SECONDS",
}
M.BY_ADDRESS = {
    [0x16684A] = M.OFFLINE,
    [0x1668CE] = M.ERASE_OBJECTIVE,
    [0x16685A] = M.ERASED,
    [0x1669A2] = M.LOCKOUT,
    [0x166AA5] = M.ACTION_FAILED,
    [0x1666A8] = M.ALERT_CANCELLED,
    [0x1666DD] = M.NO_ALERTS,
    [0x166A44] = M.SHUTDOWN,
}
function M.from_rom(address) return M.BY_ADDRESS[address] end
return M

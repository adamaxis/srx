-- Matrix message catalog. Gameplay returns stable message IDs; this module
-- owns the decoded display copy and its presenter line separators.
local E = require("enums")
local M = {
    ACTION_FAILED = "  You fail to|operate the node| successfully!",
    ALERT_CANCELLED = "ALERT CANCELLED||SYSTEM CHECKS OK||RESUME ACTIVITIES",
    SEARCHING = "   Browsing the|datastore you find",
    NOTHING = "nothing of interest.",
    OBJECTIVE_FOUND = " The recog-code|rapidly scans and|locates the file.|",
    UPLOAD_DONE = "You upload the file| to the datastore.|| Run accomplished!",
    STORY_FILE = "an interesting file| that you download| to your notebook.",
    FILE_METER = "     . . . . . . Mp",
    DOWNLOAD_FILE = "Download File?",
    DOWNLOAD_CANCELLED = "Download Cancelled.",
    FIVE_FILES = " Your deck can|only store five|  datafiles!!",
    NO_STORAGE = "No available|storage!",
    ERASE_OBJECTIVE = "  The recog-code| rapidly finds the|data and zaps it to| electron heaven!!|| Run accomplished!",
    ERASED = "You find a file that| appears important| and obliterate it|  from memory!!!|| Score one for the|downtrodden hacker!",
    STORED_OBJECTIVE = "  Shadowrun|Accomplished!||File Download|  Complete.",
    STORED = "File Download|  Complete.",
    LOCKOUT = "   AFFIRMATIVE|||NODE ACCESS LOCKED|||OVERRIDE SEQUENCE||    DISABLED",
    NO_ALERTS = " NO ALERTS ARE||   ACTIVATED|||RUN DIAGNOSTICS||FOR CONFIRMATION",
    OFFLINE = "NODE IS OFFLINE",
    SHUTDOWN = "  INITIATING||SYSTEM  SHUTDOWN|| IN - - SECONDS",
    SYSTEM_ALERT = " Your actions have|been noticed by the|      system!||All precautions are|  now enforced!!!",
    SYSTEM_OVERLOAD = "SYSTEM OVERLOAD!!||   LOGGING OUT|    ALL USERS",
    SEARCH_DOT = ".",
    STORAGE_SUFFIX = "storage!",
}
-- Provenance for the decoded streams: action_failed 0x166AA5,
-- alert_cancelled 0x1666A8, no_alerts 0x1666DD, shutdown 0x166A44,
-- offline 0x16684A, erase_objective 0x1668CE, erased 0x16685A,
-- lockout 0x1669A2, searching 0x16674D, nothing 0x1667A5,
-- objective_found 0x166770, upload_done 0x16692D, story_file 0x166968,
-- download_file 0x1667CE, download_cancelled 0x1667BA, five_files 0x16681B,
-- no_storage 0xE383D, stored_objective 0x1667DD, stored 0x1667F8,
-- file_meter 0xE3FFD, search_dot 0x166848, storage_suffix 0x166812,
-- system_alert 0x1669E8, and system_overload 0x166A75. [C]
M.TEXT_BY_ID = {
    [E.MatrixMessageId.ACTION_FAILED] = M.ACTION_FAILED,
    [E.MatrixMessageId.ALERT_CANCELLED] = M.ALERT_CANCELLED,
    [E.MatrixMessageId.NO_ALERTS] = M.NO_ALERTS,
    [E.MatrixMessageId.SHUTDOWN] = M.SHUTDOWN,
    [E.MatrixMessageId.OFFLINE] = M.OFFLINE,
    [E.MatrixMessageId.ERASE_OBJECTIVE] = M.ERASE_OBJECTIVE,
    [E.MatrixMessageId.ERASED] = M.ERASED,
    [E.MatrixMessageId.LOCKOUT] = M.LOCKOUT,
    [E.MatrixMessageId.SEARCHING] = M.SEARCHING,
    [E.MatrixMessageId.NOTHING] = M.NOTHING,
    [E.MatrixMessageId.OBJECTIVE_FOUND] = M.OBJECTIVE_FOUND,
    [E.MatrixMessageId.UPLOAD_DONE] = M.UPLOAD_DONE,
    [E.MatrixMessageId.STORED_OBJECTIVE] = M.STORED_OBJECTIVE,
    [E.MatrixMessageId.STORED] = M.STORED,
    [E.MatrixMessageId.SEARCH_DOT] = M.SEARCH_DOT,
    [E.MatrixMessageId.STORY_FILE] = M.STORY_FILE,
    [E.MatrixMessageId.FILE_METER] = M.FILE_METER,
    [E.MatrixMessageId.DOWNLOAD_FILE] = M.DOWNLOAD_FILE,
    [E.MatrixMessageId.DOWNLOAD_CANCELLED] = M.DOWNLOAD_CANCELLED,
    [E.MatrixMessageId.FIVE_FILES] = M.FIVE_FILES,
    [E.MatrixMessageId.NO_STORAGE] = M.NO_STORAGE,
    [E.MatrixMessageId.STORAGE_SUFFIX] = M.STORAGE_SUFFIX,
    [E.MatrixMessageId.SYSTEM_ALERT] = M.SYSTEM_ALERT,
    [E.MatrixMessageId.SYSTEM_OVERLOAD] = M.SYSTEM_OVERLOAD,
}

function M.resolve(message_id)
    if type(message_id) ~= "string" then return nil end
    return M.TEXT_BY_ID[message_id]
end

return M

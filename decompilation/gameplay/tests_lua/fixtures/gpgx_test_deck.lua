-- Expected values measured from headless Genesis Plus GX on the US ROM after
-- the actual Debug-menu Test Deck action. Screens are the adjacent PNGs.
-- Source routines: Test Deck 0x59B7E, CyberInfo 0x18AA6/0x18AB0,
-- Deck Stats 0x18B5C.
return {
    captures = {
        "cyberdeck.png",
        "cyberinfo.png",
        "deck-stats.png",
    },
    cyberdeck = {
        cursor = { 0, 0 },
        program_count = 12,
        program_rating = 4,
        active_slots = { -1, -1, -1, -1, -1 },
        selected_attack = { 32, 200, 1 }, -- Size Mp, free Mp, load seconds.
    },
    cyberinfo = {
        title = "  CyberInfo",
        labels = { "  Deck Stats", "  Run Info" },
        row = 0,
        cyberdeck_cursor = { 0, 3 },
    },
    deck_stats = {
        manufacturer = "Allegiance",
        model = "Alpha",
        labels = {
            "MPCP . . . . .", "Hardening. . .", "Memory . . . .  Mp",
            "Storage  . . .  Mp", "Load/IO Spd. .  Mp", "Response . . .",
            "Bod  . . . . .", "Evasion. . . .", "Masking. . . .",
            "Sensor . . . .",
        },
        values = { 6, 1, 200, 1000, 50, 1, 3, 3, 3, 3 },
    },
}

-- enums.lua: the shared named constants for Shadowrun (Genesis) game logic.
-- Naming: PascalCase type + UPPER_SNAKE member (ActorFlag.PEDESTRIAN). C++ mirror: enums.hpp (generated, `enum class`).
-- Every value cites its ROM address or RAM location. Logic code uses these names, never raw values.
local E = {}

-- Actor record field offsets; record = 0x100 bytes at FF0100 + 0x100*slot [C]
E.ActorField = {
    X = 0x00, Y = 0x04, SCREEN_X = 0x0A, SCREEN_Y = 0x0C, SPRITE_TYPE = 0x0E,
    STATUS = 0x10, STATUS_EX = 0x11, ANIM_TIMER = 0x16, FACING = 0x2E, HEADING = 0x34,
    STATE_FN = 0x3A, SLOT_ID = 0x3E, PHYSICAL = 0x4C, POSTURE = 0x4D, ROUNDS = 0x4E, CLIPS = 0x4F,
    WEAPON_SLOT = 0x56, WEAPON_CODE = 0x57, INVENTORY = 0x66, ITEM_EXTRA = 0x6E, CYBER_BITS = 0x90,
    TARGET_X = 0x96, TARGET_Y = 0x98, SPELL_GATE = 0x9B, STUN = 0x9C, FORCE = 0x9D, ROLE = 0x9E,
    SPELL_LIST = 0xA0, TARGET = 0xA2, GROUP_LEADER = 0xA8, CHASE = 0xAC, BEHAVIOR = 0xB0, STEER = 0xB1,
    APPROACH = 0xB5, THINK = 0xB6, STEER_TIMER = 0xB8, WP_POS = 0xBA, PATH = 0xBC, WALK_DIST = 0xC8,
    ACTION_DELAY = 0xDC, WAYPOINTS = 0xE0, WP_INDEX = 0xE4,
}
E.ActorPool = { STRIDE = 0x100, PARTY = 3, OTHERS = 7, PROJECTILES = 6 }   -- FF0100.., FF0400.., FF0B00.. [C]
-- Run-kill classification uses actor record +0x9B, not the rendered sprite
-- type at +0x0E. The ghoul bounty checks this byte against 0x42 (0x559BC).
E.ActorRaceOrState = { GHOUL_BOUNTY_TARGET = 0x42 }
E.GhoulBounty = { MAX_DEFEATS = 20 } -- FFFBF0 is capped at twenty (0x559BC) [C]

E.ActorStatus   = { ACTIVE = 0x01, THINK = 0x02, AI_RUNNER = 0x08, DOWN = 0x10, GROUP_LEADER = 0x20, DEAD = 0x40 }  -- +0x10 [C]
E.ActorStatusEx = { PROJECTILE = 0x01, BREATH_OUT = 0x40, UNSEEN = 0x80 }                                        -- +0x11 [C]
-- +0x9E role/AI flags (doc 20 §2); REINFORCEMENT is set by the alarm spawner 0x1E0B8 [C]
E.ActorFlag = { SPOTTED = 0x01, REINFORCEMENT = 0x02, DESPAWN_OFFSCREEN = 0x04, QUEST = 0x08,
                RUNNER = 0x10, PEDESTRIAN = 0x20, HOSTILE = 0x40 }
E.BehaviorFlag = { ROUTE_CLEAR = 0x01, PATH = 0x02, RESTART_STEER = 0x04, WALL_FOLLOW = 0x08, IN_BAND = 0x10, ROUTE_TRANSITION = 0x20, FLEEING = 0x40, ESCORT = 0x80 } -- +0xB0 [C]; ROUTE_CLEAR/ROUTE_TRANSITION [H] repo route_flags usage
E.SteerFlag = { TARGET_RIGHT = 0x01, TARGET_BELOW = 0x02, OVERSHOOT_X = 0x04, OVERSHOOT_Y = 0x08 }  -- +0xB1 [C]
E.CyberBit = { DATAJACK = 0, CYBEREYES = 1, HAND_RAZORS = 2, SPURS = 3, SMARTLINK = 4 }             -- +0x90 bit numbers (doc 04) [C]

E.Posture   = { FULL_DEFENSE = 0, MEDIUM_DEFENSE = 1, NEUTRAL = 2, MEDIUM_OFFENSE = 3, FULL_OFFENSE = 4 }  -- +0x4D, names 0xD1A05 [C]
E.SceneMode = { NONE = 0, STREET = 1, BUILDING = 2, CAVE = 3 }  -- FFDF40: 1 scenes <0x15, 2 0x15-0x2B, 3 0x2C-0x37 [C]
E.StateFn   = { IDLE = 0xB45C, WALK = 0xB574, PROJECTILE = 0xAC4E }                 -- +0x3A handlers [C]
E.CombatPhase = { NONE = 0, ATTACK_ANIM = 1, APPROACH = 2, HOLD = 3 }               -- FFDFE0 [C]
E.EndingState = { NONE = 0, FINAL_FIGHT = 3 }                                        -- FFE186 [C]
E.SpriteType  = { FIELD_RUNNER = 3, CORP_GUARD = 5, GHOUL = 7, PEDESTRIAN = 11, HELL_HOUND = 12, GARGOYLE = 13,
                  WENDIGO = 14, THON = 15 }   -- +0x0E [C: 7,12,13,15 from our trace; 3,5,11,14 names from pre-module actor_ai.lua:40-46]

-- Item codes (= +0x57 for weapons, = inventory byte); records 0xD4540 + 0x30*(code-1) (doc 08) [C]
E.ItemId = {
    NONE = 0x00, STREETLINE_SPECIAL = 0x01, MODEL_101T = 0x02, AMERICAN_L36 = 0x03, SECURITY_500 = 0x04,
    WARHAWK = 0x05, MAX_POWER = 0x06, PREDATOR = 0x07, AK97 = 0x08, HK227S = 0x09, MACH22 = 0x0A,
    ALLEGIANCE = 0x0B, ROOMSWEEPER = 0x0C, FRAG_GRENADE = 0x0D, SCATTER_GRENADE = 0x0E,
    CONCUSSION_GRENADE = 0x0F, MEDKIT = 0x10, STIM_PATCH = 0x11, TRAUMA_PATCH = 0x12, FETISH = 0x13,
    MAGLOCK_PASSKEY = 0x14, ELECTRONIC_KIT = 0x15, SMART_GOGGLES = 0x16,
    -- 0x17..0x32 names from the record strings at 0xD4540 + 0x30*(code-1) [C].
    ENCHANTED_DAGGER = 0x17, POWER_FOCUS = 0x18,
    FLAME_DART_FOCUS = 0x19, FLAME_BOLT_FOCUS = 0x1A, HELLBLAST_FOCUS = 0x1B, INVISIBILITY_FOCUS = 0x1C,
    SLEEP_FOCUS = 0x1D, STINK_FOCUS = 0x1E, SUPER_BARRIER_FOCUS = 0x1F, MANA_ZAP_FOCUS = 0x20,
    MANA_BLAST_FOCUS = 0x21, MANA_STORM_FOCUS = 0x22, ROCKSKIN_FOCUS = 0x23, HEAL_WOUNDS_FOCUS = 0x24,
    CONFUSION_FOCUS = 0x25, BARRIER_FOCUS = 0x26, PROTECTION_TALISMAN = 0x27, COMBAT_SENSE_SPELL_LOCK = 0x28,
    RAT_TOTEM = 0x29, GATOR_TOTEM = 0x2A, BEAR_TOTEM = 0x2B,
    ARMOR_VEST = 0x2C, ARMOR_CLOTHING = 0x2D, ARMOR_JACKET = 0x2E, VEST_WITH_PLATES = 0x2F,
    LINED_DUSTER = 0x30, LIGHT_COMBAT_ARMOR = 0x31, HEAVY_COMBAT_ARMOR = 0x32,
}
E.WeaponClass = { LIGHT_PISTOL = 0, HEAVY_PISTOL = 1, SMG = 2, SHOTGUN = 3, THROWN = 4 }  -- grouping of 0x15318 rows [C]
E.WeaponSlot  = { NONE = 0x7F }                                                     -- +0x56 [C]
-- note [C]: 0x13B00 compares against 0x0D, so grenades (0x0D-0x0F) are NOT picked by switch_to_weapon.
E.FIRST_NON_WEAPON = E.ItemId.FRAG_GRENADE

E.Pickup = { CLIPS_FLAG = 0xC000, NUYEN_FLAG = 0x8000, GARGOYLE_HORN = 0x64, HOUND_PELT = 0x65 }  -- 0x133A6 [C]

-- Spell types: spell record = item 0x33 + type; held weapon_code = type + 1 [C]
E.SpellType = { FLAME_DART = 0, FLAME_BOLT = 1, HELLBLAST = 2, INVISIBILITY = 3, SLEEP = 4, STINK = 5,
                SUPER_BARRIER = 6, MANA_ZAP = 7, MANA_BLAST = 8, MANA_STORM = 9, ROCKSKIN = 10,
                HEAL_WOUNDS = 11, CONFUSION = 12, BARRIER = 13 }
E.SPELL_ITEM_BASE = 0x33
-- Item kinds (repo items.lua kind names, verified against record 0xD4540 contents; doc 08) [C]
E.ItemKind = { FIREARM = 1, GRENADE = 2, MEDICAL_KIT = 3, STIM_PATCH = 4, TRAUMA_PATCH = 5, FETISH = 6, PASSKEY = 7,
               ELECTRONIC_KIT = 8, SMART_GOGGLES = 9, ENCHANTED_DAGGER = 10, POWER_FOCUS = 11, SPELL_FOCUS = 12,
               TALISMAN = 13, SPELL_LOCK = 14, TOTEM = 15, ARMOR = 16 }
E.Shop = { WEAPONS_WORLD = 0, ARES_WEAPON_EMPORIUM = 1, CRIME_MALL = 2, MEDICINE_LODGE = 3, RIANNONS = 4,
           MERLINS_LORE = 5, MICROTRONICS = 6, WIRE_MASTERS = 7, LITTLE_CHIBA = 8, DR_BOBS = 9,
           COUNCIL_ISLAND_HOSPITAL = 10, SEATTLE_GENERAL_HOSPITAL = 11 }   -- FFF0B6, names 0x1AAB50 [C]

-- Weapon accessories: +0x6E bit = 1 << index (0x5719C); names 0x1AAE0C; prices 0x1AAE8C [C]
E.Accessory = { SMARTGUN = 0, SILENCER = 1, SOUND_SUPPRESSOR = 2, LASER_SIGHT = 3, GAS_VENT_2 = 4, GAS_VENT_3 = 5 }
E.AccessoryMask = { SILENCED = (1 << 1) | (1 << 2) }    -- Silencer | Sound Suppressor (fire_shot 0x15200) [C]

E.SoundId = {   -- played through 0x530DE (doc 10) [C]
    SHOT_LIGHT_PISTOL = 0x1C, SHOT_HEAVY_PISTOL = 0x51, SHOT_SMG_OR_THROWN = 0x1E, SHOT_SHOTGUN = 0x1D,  -- 0x15318
    SHOT_SILENCED = 0x3F, SHOT_SILENCED_SMG_OR_THROWN = 0x4A,                                            -- 0x15336
    RELOAD_OR_EMPTY = 0x4B, ALARM = 0x30, PICKUP = 0x24,
    MATRIX_NO_ALERT = 0x54, MATRIX_PASSIVE_ALERT = 0x53,
    MATRIX_ACTIVE_ALERT = 0x52, -- 0x19904 change-gated Matrix alert status sound
    MATRIX_SYSTEM_SEARCH = 0x36, -- 0xE204 System Search feedback
    -- [H] repo-named (pre-module actor_ai.lua), not yet traced to a ROM table:
    TARGET_HEADING = 0x4D, TARGET_HEADING_ALT = 0x4E, PROJECTILE_IMPACT = 0x3C,
    -- Remaining rendered sequence ids (doc 10 §5). Names follow the caller;
    -- [H] where only the caller region is known.
    -- YM DAC sample effects (Timer A paced, 0x02B9 wait); silent until the
    -- offline extractor modelled the YM timers. Callers not traced yet [H].
    DAC_SAMPLE_19 = 0x19, DAC_SAMPLE_1B = 0x1B, DAC_SAMPLE_2C = 0x2C, DAC_SAMPLE_2D = 0x2D,
    DAC_SAMPLE_2E = 0x2E, DAC_SAMPLE_2F = 0x2F, DAC_SAMPLE_40 = 0x40, DAC_SAMPLE_41 = 0x41,
    DAC_SAMPLE_42 = 0x42, DAC_SAMPLE_43 = 0x43, DAC_SAMPLE_44 = 0x44, DAC_SAMPLE_45 = 0x45,
    DAC_SAMPLE_46 = 0x46, DAC_SAMPLE_47 = 0x47, DAC_SAMPLE_48 = 0x48, DAC_SAMPLE_49 = 0x49,
    DAC_SAMPLE_4F = 0x4F, DAC_SAMPLE_56 = 0x56, DAC_SAMPLE_57 = 0x57, DAC_SAMPLE_58 = 0x58,
    DAC_SAMPLE_59 = 0x59, DAC_SAMPLE_5A = 0x5A,
    FIELD_EFFECT_16 = 0x16,          -- boot intro stops it via 0x530D2 at 0x5D7F6/0x5D80E [C]; ordinary FF 10 start [H role]
    MATRIX_NODE_14 = 0x14, MATRIX_NODE_15 = 0x15, MATRIX_NODE_17 = 0x17,   -- 0x1B5BC-0x1CB90 node actions [H per id]
    MATRIX_18 = 0x18,                -- 0x18584/0x185C8/0x1A482/0x1A674 [C sites]
    DECK_TICK_1F = 0x1F, DECK_TICK_20 = 0x20, DECK_TICK_21 = 0x21,         -- table 0x17D62 via 0x17CC8 [C]
    DECK_TICK_22 = 0x22, DECK_TICK_23 = 0x23,
    MATRIX_PROGRAM_26 = 0x26, MATRIX_PROGRAM_27 = 0x27, MATRIX_PROGRAM_28 = 0x28,   -- 0x1B5BC-0x1CB90 [H per id]
    MATRIX_PROGRAM_2A = 0x2A, MATRIX_PROGRAM_2B = 0x2B,
    MENU_COMMAND_29 = 0x29,          -- 0x26A0C [C site, role unknown]
    SPELL_EFFECT_31 = 0x31, SPELL_EFFECT_32 = 0x32, SPELL_EFFECT_33 = 0x33,  -- spell table 0x4946 [C]
    SPELL_EFFECT_34 = 0x34, SPELL_EFFECT_35 = 0x35, SPELL_EFFECT_37 = 0x37,
    SPELL_EFFECT_38 = 0x38, SPELL_EFFECT_39 = 0x39, SPELL_EFFECT_3A = 0x3A,
    FIELD_3B = 0x3B,                 -- 0x4AA4 [C site, role unknown]
    UNUSED_3D = 0x3D, UNUSED_3E = 0x3E, UNUSED_50 = 0x50,                   -- no caller found (doc 10 §5 re-scan) [H]
    MELEE_SWING = 0x4C, MELEE_SWING_ALT = 0x55,   -- 0xB1DC: 0x4C, or 0x55 when actor +0x0E == 0x0C [C mechanism]
    DECK_JINGLE = 0x5B,              -- 0x1763A deck-screen jingle; stopped at 0x1764C [C]
    SEGA_LOGO_SHOCK = 0x5C,          -- 0x5D832 [C]
    SHADOWRUN_LOGO_REVEAL = 0x5D,    -- 0x1E60 [C]
}
-- Music sequence ids, same GEMS id space as E.SoundId; started through
-- play_music 0x530EA (stop same id, then start; gated by FFFC08) [C].
E.MusicId = {
    TITLE_THEME = 0x00,              -- title ThemeStart (srx title loop) [C]; doc 10: also reset 0x1C5E [H role]
    FIELD_01 = 0x01, FIELD_02 = 0x02, -- scene profile +0x40 (0x2900) [C]
    MATRIX = 0x03,                   -- 0x17656 / 0x1B1A4 [C sites, name H]
    DECK_MENU = 0x04,                -- 0x1B15A [C site, name H]
    NODE_MENU = 0x05,                -- 0x1AEF8 [C site, name H]
    FIELD_06 = 0x06, FIELD_07 = 0x07, FIELD_08 = 0x08, FIELD_09 = 0x09, FIELD_0A = 0x0A,
    FIELD_0B = 0x0B, FIELD_0C = 0x0C, FIELD_0D = 0x0D, FIELD_0E = 0x0E, FIELD_0F = 0x0F,
    INTRO_PAGE_1 = 0x10, INTRO_PAGE_2 = 0x11, INTRO_PAGE_3 = 0x12,         -- 0x59C76/96/BC [C]
    ENDING = 0x13,                   -- 0x5BB4E [C]
}
-- GEMS command ops sent as FF op args (0x52E96) [C]; the host mixer logs the same ops.
E.AudioOp = { START = 0x10, STOP = 0x12, STOP_ALL = 0x16, ATTENUATE = 0x20 }
E.AudioTuning = {
    FADE_STEP = 0x14, FADE_STEPS = 6, FADE_STEP_FRAMES = 3,   -- 0x53126; 0xC24 waits d5+1 = 3 VBlanks [C]
    START_MENU_DUCK = 8,                                      -- 0xB62 -> 0x5315E(8) [C]; level meaning [H]
}
E.SceneId = { ALARM_COMBAT = 0x14, FINAL = 0x37 }                                    -- 0x1E04A, 0xA38 [C]
E.EventFn = { GUARD_SPOTS_YOU = 0x20FEC }                                            -- scheduled via 0x1E2E0 [C]

E.AiTuning = {   -- [C] unless marked
    THINK_FRAMES = 0x14, ANIM_BUSY = 0x3C, ARRIVE_DIST = 0x1E0, FLANK_RADIUS = 0xE0, FLANK_ANGLE = 0x6000,
    FLEE_OFFSET = 0x2580, FLEE_CAMERA_BIAS = 0xA00, ENGAGE_DIST = 0x500, WATCH_DIST = 0x640,
    THON_PERIOD = 8, HURT_BELOW = 4, CAST_MIN_STUN = 5, SPELL_GATE_MAX = 2, APPROACH_TICKS = 6,
    QUARTER_TURN = 0x4000, NO_TARGET = 0x12345678, STEER_NONE = 0x1000, ALARM_FRAMES = 0x960,
    RELOAD_ROUNDS = 0x26, PATH_TRIES = 5, ANIM_TIMER_MASK = 0x7FF,
    ENDING_HOLD_ROUNDS = 0x28,   -- 0x13D20 [C value, H meaning]
}
E.Collision = { BLOCKED = 0x7F, LOW_BITS = 0x1F, IMPASSABLE_FROM = 0x18 }   -- repo movement_collision_code [H names, C values]
E.EntryDelayMax = { STATIC = 15, DYNAMIC_ENCOUNTER = 0x28 }                   -- repo heartbeat guard [H names]
E.Grid = { ROW_CELLS = 0x40, CELL_SHIFT = 9, CELL_CENTER = 0x100, PATH_END = 0xFFFF }   -- FF1700 [C]
E.Direction = { UP = 0, RIGHT = 2, DOWN = 4, LEFT = 6 }                                -- 0x148CA index [C]
E.DirectionStep = { [0] = -0x40, [2] = 1, [4] = 0x40, [6] = -1 }                      -- 0x148CA values [C]

-- ===== Menus (refactor/menu, docs/09) =====
-- FFDD22 event word from 0x195C: high byte = new edges, low byte = held [C]
E.Pad = { UP = 0x0100, DOWN = 0x0200, LEFT = 0x0400, RIGHT = 0x0800, B = 0x1000, C = 0x2000, A = 0x4000, START = 0x8000,
          HELD_UP = 0x0001, HELD_DOWN = 0x0002, HELD_LEFT = 0x0004, HELD_RIGHT = 0x0008, EDGES = 0xFF00, DPAD = 0x0F00 }
E.MenuSound = { MOVE = 0x25, SELECT = 0x24, JACK_IN = 0x1A, DAMAGE = 0x18 }   -- 0x1A: 0xE47A jack-in [C]   -- 0x10E2E / 0x10E4E / 0x11A72 via 0x530DE [C]
E.MatrixDeckMode = { ATTACK = 0xFF, BALANCED = 0x00, DEFENSE = 0x01 }      -- FFF0B7: A/B/C split the Matrix pool [C]
-- Signed view used by ROM branches after FFF0B7 is loaded into a signed word.
E.MatrixModeResult = { ATTACK = -1, BALANCED = 0, DEFENSE = 1 }
-- Canonical FFFBED run-handler IDs from the table at F3E56 [C].
E.RunType = {
    GHOUL_BOUNTY = 0, BODYGUARD_ESCORT = 1, PACKAGE_DELIVERY = 2,
    GANG_CONTRACT = 3, PACKAGE_RECOVERY = 4,
    EMPLOYEE_RECRUITMENT = 5, MATRIX_OBJECTIVE = 6,
}
-- Source initializer IDs in F3DC6. Several source variants share one
-- canonical run handler and differ only in how their route is selected [C].
E.RunOfferInitializer = {
    GHOUL_BOUNTY = 0, BODYGUARD_ESCORT = 1, PACKAGE_DELIVERY = 2,
    GANG_CONTRACT = 3, PACKAGE_RECOVERY = 4,
    EMPLOYEE_RECRUITMENT = 5, MATRIX_SIMPLE = 6,
    MATRIX_MODERATE = 7, MATRIX_EXPERT = 8,
    PACKAGE_RECOVERY_TWO_SITE_POOL = 9,
    PACKAGE_RECOVERY_RENRAKU = 10,
    EMPLOYEE_RECRUITMENT_TWO_SITE_POOL = 11,
    EMPLOYEE_RECRUITMENT_RENRAKU = 12,
}
-- The Johnson event handler at 0x20990 selects FFF0AE from its active scene.
E.RunOfferContext = {
    JUMP_HOUSE = 0, SCENE_18 = 1, BIG_RHINO = 2,
    WANDERER = 3, ICARUS_DESCENDING = 4,
}
-- FFF0D0 primary page; pages 0/1 are the two inventory columns, 2..6 dispatch through table 0xD2A0A [C]
E.MenuPage = { INVENTORY_LEFT = 0, INVENTORY_RIGHT = 1, CLIPS = 2, ATTRIBUTES_SKILLS = 3, CYBERWARE = 4, MAGIC = 5,
               POCKET_SECRETARY = 6, COUNT = 7 }
E.MenuPageHandler = { CLIPS = 0x11B58, ATTRIBUTES_SKILLS = 0xD01C, CYBERWARE = 0x11950, MAGIC = 0xFBA6,
                      POCKET_SECRETARY = 0xC2A6 }   -- longwords at 0xD2A0A [C]
E.InventoryGrid = { COLUMNS = 2, ROWS = 4, CURSOR_DX = 0x28, CURSOR_DY = 0x18 }   -- FFF0D1 clamp 0..3, 0x11592 mulu [C]
-- Screen ids for the Lua route stack (refactor names; the ROM has nested blocking loops instead) [C structure, names ours]
E.MenuScreen = { COMMAND = 1, POSTURE = 2, ITEM_ACTION = 3, CLIPS = 4, ATTRIBUTES_SKILLS = 5, CYBERWARE = 6, MAGIC = 7,
                 MAGICAL_DEFENSE = 8, POCKET_SECRETARY = 9, NOTEBOOK = 10, DISMISS_RUNNER = 11, FIRST_AID = 12,
                 AUTOMATIC_USE = 13, INVENTORY_TRADE = 14, DISCARD_CONFIRM = 15, SAVE_LOAD = 16, GROUP_ITEMS = 17,
                 CYBERDECK = 18, MESSAGE = 19, MR_JOHNSONS = 20, NOTEBOOK_TIPS = 21, CLIP_TRADE = 22,
                 RELOAD_CONFIRM = 23, UPGRADE_PROMPT = 24, MAGIC_FORCE = 25, MAGIC_POSTURE = 26,
                 DEBUG_MENU = 27, DEBUG_PICK_SYSTEM = 28, SAVE_SLOT = 29, PULL_PLUG = 30,
                 DECK_PROGRAM = 31, DECK_CYBERINFO = 32, DECK_STORAGE = 33, DECK_MAP = 34,
                 NOTEBOOK_CONTACTS = 35, NOTEBOOK_SHADOWRUNNERS = 36,
                 MATRIX_NODE_ACTION = 37, MATRIX_ENTRY = 38,
                 MATRIX_SEARCH = 39, TAXI_SELECTOR = 40,
                 NOTEBOOK_CURRENT_RUN = 41 }   -- Positive detail page follows 0x1E72C; taxi destinations follow 0x134D2/0xD4456/0xD445D
-- Presenter regions (docs/09 section 10.1 coordinates) [C positions]
E.MenuRegion = { ITEM_PICTURE = 0x0001, ITEM_HEADER = 0x0002, RATING = 0x0004, DAMAGE_POWER = 0x0008, COMBAT_BARS = 0x0010,
                 WEAPON_BARS = 0x0020, ARMOR = 0x0040, POSTURE_BLOCK = 0x0080, BUTTON_STRIP = 0x0100,
                 COMMAND_LABEL = 0x0200, INVENTORY_CURSOR = 0x0400, POSTURE_LABEL = 0x0800, SIDEBAR_SLOT = 0x1000,
                 MESSAGE = 0x2000, SUBMENU = 0x4000, FULL = 0x8000 }
E.MenuTransition = { NONE = 0, PUSH = 1, POP = 2, REPLACE = 3, REBUILD = 4, EXIT = 5 }
E.DisplayBlank = { OFF = 0x8134, ON = 0x8174 }   -- VDP reg 1 writes around a build [C]
E.WindowPlane = { SIDEBAR = 0x9191, OFF = 0x9100 } -- VDP reg 0x11 [C]

-- Runner record view used by menu rules (C++ proxies these onto FF0100+0x100*slot) [C offsets: srx runner_record_access]
E.RunnerStatus = { ACTIVE = 0x01, REMAINDER = 0x02, DOWN = 0x10, DEAD = 0x40 }   -- +0x10; REMAINDER name [H] (clips 0x12062 usage)
E.RunnerField = { STATUS = 0x10, STATE_FN = 0x3A, PHYSICAL = 0x4C, POSTURE = 0x4D, ROUNDS = 0x4E, CLIPS = 0x4F,
                  WEAPON_SLOT = 0x56, WEAPON_CODE = 0x57, INVENTORY = 0x66, ITEM_PHASE = 0x6E, ARMOR = 0x76,
                  ATTRIBUTES = 0x77, MAGIC = 0x7F, SKILLS_A = 0x80, SKILLS_B = 0x87, KARMA = 0x8D, SPECIES = 0x9A,
                  ARCHETYPE = 0x9B, STUN = 0x9C, FORCE = 0x9D, SPELL_TABLE = 0xA0 }
E.Archetype = { CASTER_LIMIT = 2 }        -- +0x9B < 2 = magic user (0xFBA6 path, Sorcery gate) [C]
E.Skill = { SORCERY = 0, FIREARMS = 1, PISTOLS = 2, SMGS = 3, SHOTGUNS = 4, BIOTECH = 8 }   -- index into skills[] [C 0x12C74; BIOTECH srx]
E.ItemAction = { EQUIP_OR_USE = 2, DISCARD = 3, TRADE = 4, CANCEL = 5 }   -- FFF0D0 reused as selector 0x11780 [C]
E.ItemBranch = { FIRST_AID = 1, AUTOMATIC_USE = 2, WEAPON_OR_GEAR = 3, ARMOR = 4 }
E.ClipsCommand = { RELOAD = 2, TRADE = 3, DISTRIBUTE = 4, CANCEL = 5 }
E.ClipTradeCommand = { INCREASE = 2, DECREASE = 3, TRADE = 4, CHANGE_RECIPIENT = 5 }   -- nested trade selector reuses 2..5 [C srx clips_menu_conversion]
E.DebugOption = { NUYEN = 0, SPELLS = 1, TEST_DECK = 2, PICK_SYSTEM = 3, HEAL_ALL = 4, KARMA = 5, CONTACTS = 6, WALK_FAST = 7 }  -- table 0x1D90C0 [C]
E.SaveLoadOption = { SAVE = 0, LOAD = 1 }   -- 0xC2600 rows [C]
E.PocketOption = { NOTEBOOK = 0, GROUP_ITEMS = 1, CYBERDECK = 2, DISMISS_RUNNER = 3, SAVE_LOAD = 4, DEBUG = 5 } -- table 0xC1400; 5 = debug menu 0x599DA (FFF101) [C]
-- FFFC2C state long: bits 0..12 unlock Vid-Phone contacts. 0xE506 loads the
-- full word and 0xF5F4 scans only indices 0..12 [C]; high bits remain active
-- state (scene-20 entry bit 26, Hollywood access bit 28) and must be preserved
-- when setting an individual contact. Names follow dialogue.lua
-- vid_phone_contacts (source_index).
E.VidPhoneContact = {
    CAPT_QUINTON_JAYMES = 0, ALESANDRO_HOBBS = 1, SHARKEY = 2, KIPP_DAVID = 3,
    WILMA_TEMMENHOFF = 4, MACKLEMANN = 5, GREGORY_WILNS = 6, MR_FARADOUCHI = 7,
    AGIRA_TETSUMI = 8, HENRY_J_CULVER = 9, MAX = 10, CHILLICUT = 11,
    PROFESSOR_JEFFERSON = 12,
}
E.NotebookOption = { CURRENT_RUN = 0, TIPS_AND_CLUES = 1, CONTACTS = 2, MR_JOHNSONS = 3, SHADOWRUNNERS = 4 }
E.NotebookBoundary = { EVENT_GATES_UNRESOLVED = 0, TIPS_FLAGS_UNRESOLVED = 1,
                       CURRENT_RUN_UNRESOLVED = 2, CONTACTS_FLAGS_UNRESOLVED = 3,
                       SHADOWRUNNERS_FLAGS_UNRESOLVED = 4,
                       MR_JOHNSON_FLAGS_UNRESOLVED = 5 }
E.MenuMessage = { OUT_OF_AMMO = 1, ITEM_NO_CLIPS = 2, NO_PARTY = 3, RELOADED = 4, EXCHANGE_COMPLETE = 5, CANNOT_DISCARD = 6,
                  CANNOT_TRADE = 7, AUTOMATIC_USE = 8, FIRST_AID_USED = 9, SLAP_PATCH_USED = 10, NOT_INJURED = 11,
                  INSUFFICIENT_KARMA = 12, MAGE_ONLY = 13, NOT_RAISABLE = 14, MAX_LEVEL = 15, NO_CURRENT_RUN = 16,
                  NO_CYBERDECK = 17, GROUP_ITEMS = 18, MR_JOHNSON_DETAIL = 19, MAGICAL_DEFENSE = 20,
                  FUNCTION_ABORTED = 21, SAVE_OUTSIDE_ONLY = 22, SAVE_IN_COMBAT = 23, GAME_SAVED = 24,
                  LOAD_EMPTY = 25, LOAD_OK = 26,
                  DECK_NO_SLOTS = 27, DECK_NO_MEMORY = 28, NOT_ON_RUN = 29, RUN_COMPLETE = 30, NO_DATAFILES = 31,
                  RETREAT_COMBAT = 32, RETREAT_NO_NODE = 33,
                  NOTEBOOK_CONTACT_DETAIL = 34, NOTEBOOK_SHADOWRUNNER_DETAIL = 35 }   -- Detail waits share ROM 0xF312 edge loop [C]


-- FFFC1E story-progress long; bit k = byte FFFC21-k/8, bit k%8. Cleared whole at 0x818 (new game) [C].
-- Generic setter 0x2196C = bset FFDD64 on FFFC1E; trigger loop 0x53A78 stores trigger index in FFDD64,
-- so dialogue cmds 0x15..0x1D / cmd05 inside trigger i's conversation set bit i [C]. Names are [H] glosses of the node text.
E.StoryFlag = {
  MOTEL_BILL_PAID      = 0,  -- cmd05 @0x21410 (pay 250), node F4E8C Stoker's Coffin Motel, trigger 0 [C site][H name]
  TARISLAR_ASKED_SHALE = 1,  -- cmd16 from F49FC "tenant named Tabatha Shale?", trigger 1 [C]
  OWLFEATHER_ASKED     = 2,  -- cmd17 from F4B68 Ranger D. Owlfeather "my brother Michael", trigger 2 [C]
  ARAGORN_FOUND        = 3,  -- 0x53C96 + cmd18 from F4BBC [C]
  ARAGORN_FREED_TOLD   = 4,  -- jsr 0x2196C @0x53D4E + cmd19 from F4BA0 "Still no luck" [C]
  SHAMAN_MET           = 5,  -- cmd15 from F4BD8 (FFDD64=5 @0x53D8E) [C]
  SHAMAN_ITEM4_GIVEN   = 6,  -- 0x53DD0 consumes Group Item 4 -> F4C34 [C]
  SHAMAN_PELT_GIVEN    = 7,  -- 0x53E38 consumes Hell Hound Pelt -> F4C3C; cmd40 0x21F46 is FFFC34 (tool bug) [C]
  SERPENT_SCALE_TAKEN  = 8,  -- FFDD64=8 @0x53F00; 0x53F50 sets story bit 8 and FFFC22 bit 2; 0x53EA8 consumes item bit 2 [C]
  SHAMAN_TASKS_DONE    = 9,  -- 0x53EA8 consumes Feathered Scale -> F4C04 [C]
  MICHAEL_ALIAS_LEARNED = 10, -- cmd2B (needs b1) -> F5414 "came in using the name Owlfeather" [C site][H name]
  MAKO_DEFEATED        = 11, -- 0x53FFC encounter 0x1E628 cleared -> F4CA8 "Mako lies unconscious" [C]
  RENRAKU_MAP_OVERLAY  = 12, -- 0x540F8 tile 0x1A cb 0x540D6, scene 0x28, needs b16; no dialogue; FFFC22 +8 Map Overlay [C]; trigger 12 (5/6) shows 110A2A Renraku office [C]
  MICHAEL_MISSION_TOLD = 13, -- cmd1C @0x2193A -> 105780 "seek out a specific cave"; no invoking node in dump [C site][H name]
  CYBERHEART_SAFE      = 14, -- 0x541D8 -> 11B7FC [C]
  OGAMI_LEFT           = 15, -- trigger 15 tail -> 1111DE [C]
  ITO_FIGHT_WON        = 16, -- 0x54286 encounter 0x1E5C4 cleared: +10000 nuyen, FFFC28 b11, Ito screen 0x5D1B4 [C]
  RITUAL_OF_BINDING    = 17, -- cmd1B @0x21902 -> F563C Sinsearach "Ritual of Binding"; FFFC22 +6; no invoking node in dump [C site][H name]
  RED_BUFFALO_WOMAN    = 18, -- trigger 18 tail -> F4DB8; never cleared (0x1F8B4/cmd61 are FFFC34) [C]
  ILENE_MET            = 19, -- service19 @0x543A2 arms FFE182; scene-20 activity 0x54402 opens 0x115ECC/Ilene text and marks trigger19 [C]
  GILLIAN_INITIATION   = 20, -- cmd37 @0x21E58 FFDD64=0x14 (F56CC) [C]
  GARDEN_APTS_ENTERED  = 21, -- trigger21 (1/7, needs b30) 0x5456E -> F4DF4 Tarlisar Garden Apartments [C]
  LANDMARK_MAP_TAKEN   = 22, -- trigger22 tail -> F4E5C (Vigore area), FFFC22 +7 Landmark Map [C]
  HOLOPIX_SHOWN        = 23, -- cmd3E -> 111006 [C]
  MEDICINE_LODGE_TURNED_AWAY = 24, -- 0x1FDEA scene 7/0 -> 11B714 [C site][H name]
  HOSPITAL_VISITED     = 25, -- 0x1F8C2 (needs b23,b24) -> 11105A; cmd61 0x22672 [C sites][H name]
  RENRAKU_MAP_SHOWN    = 26, -- cmd46 0x2203E -> F5990 "show Harlequin the map from Renraku"; cmd47 0x22090 [C]
  BOTH_MAPS_SHOWN      = 27, -- cmd46 0x2205C -> F59FC "...and the map from Vigore and Jarl"; cmd47 0x220B6 [C]
  POST_ITO_BRIEFED     = 28, -- cmd40 0x21F58 (needs b16) -> F5968; trigger21 0x5451A -> F4E34 [C sites][H name]
  HEART_DELIVERED      = 29, -- 0x1F940 -> 110A5E [C]
  FROSTY_MET           = 30, -- cmd44 0x21FD6 -> F57F8 "taken to Frosty"; gates trigger21 [C]
  UNUSED_31            = 31, -- no set site found (static scan) [H]
}

return E

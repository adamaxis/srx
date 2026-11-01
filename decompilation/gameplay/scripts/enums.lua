-- enums.lua: the shared named constants for Shadowrun (Genesis) game logic.
-- Naming: PascalCase type + UPPER_SNAKE member. C++ mirror: enums.hpp (generated, `enum class`).
-- Every value cites its ROM address or RAM location. Logic code uses these names, never raw values.
local E = {}

-- Named outcomes exposed by the field host after it resolves the ROM's
-- signed package-run result. Gameplay rules consume these states, not a byte.
E.PackageRunStatus = {
    NONE = "none",
    HANDOFF_DUE = "handoff_due",
    PAYMENT_DUE = "payment_due",
}

E.FieldModePrompt = {
    NONE = "none",
    LITTLE_CHIBA_STORE = "little_chiba_store",
    ARES_EMPORIUM = "ares_emporium",
}

E.GhoulBounty = { MAX_DEFEATS = 20 } -- FFFBF0 is capped at twenty (0x559BC) [C]

-- +0x9E role/AI flags (doc 20 §2); REINFORCEMENT is set by the alarm spawner 0x1E0B8 [C]
E.SteerFlag = { TARGET_RIGHT = 0x01, TARGET_BELOW = 0x02, OVERSHOOT_X = 0x04, OVERSHOOT_Y = 0x08 }  -- +0xB1 [C]
E.CyberBit = { DATAJACK = 0, CYBEREYES = 1, HAND_RAZORS = 2, SPURS = 3, SMARTLINK = 4 }             -- +0x90 bit numbers (doc 04) [C]
-- Player hurt animation selected by 0xAFF2 from the directional C072 table.
E.PlayerHurt = {
    SELECTOR_BASE = 0x60,       -- C072 direction 0 selector [C]
    SELECTOR_STEP = 0x04,       -- C072 advances one word per direction [C]
    FRAME_COUNT = 2,            -- each kind-3 C072 list has header 0x8002 [C]
    FRAME_INCREMENT = 0x0300,   -- kind-3 C072 list increment [C]
    FRAME_PERIOD = 0x1000,      -- +0x1C initialized at 0x15132 [C]
    RENDER_PASSES = 11,         -- 0x5FDC/0x600E completes the two-frame list [C]
    SOUND_DELAY = 8,            -- 0xAFF2 stores +0xCE; 0xB07E counts it down [C]
}
E.ActorAnimation = {
    FRAME_INCREMENT = 0x0200, -- 0x5FDC advances C0A2/C0B2 +0x1A by 0x0200 [C]
    FRAME_PERIOD = 0x1000, -- C0A2/C0B2 +0x1C period consumed by 0x5FDC/0x600E [C]
} -- ROM 0xB1DC strike and 0xB330 runner-down selectors [C]

E.Posture   = { FULL_DEFENSE = 0, MEDIUM_DEFENSE = 1, NEUTRAL = 2, MEDIUM_OFFENSE = 3, FULL_OFFENSE = 4 }  -- +0x4D, names 0xD1A05 [C]
E.ActorAttribute = { BODY = "body", WILLPOWER = "willpower" } -- semantic attributes used by combat rules; +0x77/+0x7C [C]
-- Lifecycle transitions requested by gameplay interactions.
E.ActorLifecycleEvent = {
    ESCORT_CONCLUDED = "escort_concluded",
}
E.MovementDirection = {
    NORTH = "north", NORTH_EAST = "north_east", EAST = "east",
    SOUTH_EAST = "south_east", SOUTH = "south",
    SOUTH_WEST = "south_west", WEST = "west", NORTH_WEST = "north_west",
}
E.ActorFacing = {
    NORTH = 0, NORTH_EAST = 1, EAST = 2, SOUTH_EAST = 3,
    SOUTH = 4, SOUTH_WEST = 5, WEST = 6, NORTH_WEST = 7,
} -- C062 walk list direction indices [C: 0xB6BA..0xB6DE]
E.SpriteFlip = { NONE = 0, HORIZONTAL = 1 } -- descriptor attribute 0x0800 [C: 0x600E]
E.FieldSpritePriority = { LOW = 0, HIGH = 1 } -- SAT attribute bit 15
-- Ordinary actor attributes leave it clear [C: 0x481E, 0x622C, 0x1D6738+0x2C]
E.MovementBearingDegrees = {
    [E.MovementDirection.NORTH] = 0,
    [E.MovementDirection.NORTH_EAST] = 45,
    [E.MovementDirection.EAST] = 90,
    [E.MovementDirection.SOUTH_EAST] = 135,
    [E.MovementDirection.SOUTH] = 180,
    [E.MovementDirection.SOUTH_WEST] = 225,
    [E.MovementDirection.WEST] = 270,
    [E.MovementDirection.NORTH_WEST] = 315,
} -- degrees clockwise from north; native boundary converts to field angles [C]
E.Scene = {
    JUMP_HOUSE = 13, WANDERER = 14, -- 0x1B3C06 / 0x1D5B52 scene profiles [C]
    RESOURCE_ACTIVITY_FIRST = 21, RESOURCE_ACTIVITY_LAST = 43,
    RESOURCE_ACTIVITY_21 = 21, RESOURCE_ACTIVITY_22 = 22,
    RESOURCE_ACTIVITY_23 = 23, RESOURCE_ACTIVITY_24 = 24,
    RESOURCE_ACTIVITY_25 = 25, RESOURCE_ACTIVITY_26 = 26,
    RESOURCE_ACTIVITY_27 = 27, RESOURCE_ACTIVITY_28 = 28,
    RESOURCE_ACTIVITY_29 = 29, RESOURCE_ACTIVITY_30 = 30,
    RESOURCE_ACTIVITY_31 = 31, RESOURCE_ACTIVITY_32 = 32,
    RESOURCE_ACTIVITY_33 = 33, RESOURCE_ACTIVITY_34 = 34,
    RESOURCE_ACTIVITY_35 = 35, RESOURCE_ACTIVITY_36 = 36,
    RESOURCE_ACTIVITY_37 = 37, RESOURCE_ACTIVITY_38 = 38,
    RESOURCE_ACTIVITY_39 = 39, RESOURCE_ACTIVITY_40 = 40,
    RESOURCE_ACTIVITY_41 = 41, RESOURCE_ACTIVITY_42 = 42,
    RESOURCE_ACTIVITY_43 = 43,
    LAST_ID = 55,
} -- 0x3232 scene table; resource placements 0x15..0x2B; 0x37 is last ROM scene [C]
E.FieldSafe = {
    PROJECTED_EVENT = 0x1A, -- 0xB7B8 routes event 0x1A through 0x16760/0x20C90 [C]
    RENRAKU_SCENE = 0x28, -- 0x540D6 safe handler checks FFFBF8 [C]
    FUCHI_ENTRY_SCENE = 0x03, -- 0x54152 arms FFE187 from scene 3/event 2 [C]
    FUCHI_ENTRY_EVENT = 0x02,
}
E.RunnerContinuation = { IDLE = "idle" } -- first-aid return state; host maps it to the native continuation [C]
E.RunnerMeter = {
    PHYSICAL = "physical", -- live runner +0x4C [C]
    MENTAL = "mental",     -- live runner +0x9C [C]
} -- field service tests both meters at 0xA912..0xAA02 [C]
E.CombatPhase = { NONE = 0, ATTACK_ANIM = 1, APPROACH = 2, HOLD = 3 }               -- FFDFE0 [C]

-- Shared 0xABFC / 0xD7EA delay calculation. D0=0 is unchanged, D0=2 doubles,
-- and every other nonzero D0 is signed-halved; D7EA scans +0x90 bits 12..14
-- in priority order and 0xABFC clamps the result to 0x0F. [C]
-- Source callers use only 0, 1, and 2 [C].
E.ActionDelayMode = { NORMAL = 0, HALF = 1, DOUBLE = 2 } -- 0xABFC D0 modes [C]
E.ActionDelay = { BASE = 0x6A, FACTOR_STEP = 4, MINIMUM = 0x0F }                -- 0xABFC [C]
E.ActionDelayModifier = {
    NONE = 0, FACTOR_BONUS_4 = 4, FACTOR_BONUS_8 = 8,
    FACTOR_BONUS_12 = 12,
}
E.ActionDelayCapability = {
    FACTOR_BONUS_4 = 0x1000,
    FACTOR_BONUS_8 = 0x2000,
    FACTOR_BONUS_12 = 0x4000,
} -- +0x90 flags scanned by 0xD7EA in that priority order [C]

-- Stable host presentation-profile IDs. Source sprite selectors are translated
-- in the native adapter and never become Lua actor state.
E.ActorVisualProfile = {
    FIELD_RUNNER = "field_runner",
    ORK_BANDERSNATCH = "ork_bandersnatch",
    HUMAN_GUARD = "human_guard",
    ELVEN_MAGE = "elven_mage",
    GHOUL_EYE_FIVER = "ghoul_eye_fiver",
    YAKUZA_WEENR = "yakuza_weenr",
    VAMPIRE_GATOR_SHAMAN = "vampire_gator_shaman",
    LONE_STAR_MAGE = "lone_star_mage",
    PEDESTRIAN_RAT_SHAMAN = "pedestrian_rat_shaman",
    HELL_HOUND = "hell_hound",
    GARGOYLE = "gargoyle",
    WENDIGO = "wendigo",
    THON = "thon",
    HIT_SPARK = "hit_spark",
}
-- Frame count/rate come from data/actor_anim_lists.lua kind 1 (list 0x7330).
E.HitSpark = {
    SELECTOR = 0x0004, -- child +0x16 at 0xB2BC [C]
    PALETTE_ATTRIBUTES = 0x2000, -- child +0x2C at 0xB2E4 [C]
    TARGET_Y_OFFSET = 0x0010, -- target Y adjustment at 0xB2DC [C]
    RANGE_LIMIT = 0x0200, -- linked strike range gate at 0x15C58 [C]
    FINISHED_SELECTOR_BIT = 0x8000, -- finite selector bit set by 0x600E [C]
}
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
E.CombatTuning = {
    DEFAULT_ACTION_DIFFICULTY = 4, -- 0x55B16 initial threshold [C]
    DEFAULT_ACTION_TRIAL_COUNT = 6, -- 0x55B16 bounded six-sided rolls [C]
    DEFAULT_ACTION_CONTINUATION_DIFFICULTY = 6, -- 0x55B54 -> 0x559BC [C]
    DEFAULT_ACTION_VALUE = 2, -- 0x55B54 seeds the 0x559BC continuation [C]
    ACTION_VALUE_MAX = 10, -- 0x559F0 clamp [C]
}
E.CyberwareMask = {
    CYBEREYES = 0x0002, -- raw 0x0002: cyberware word bit 1 [C]
    HAND_RAZORS = 0x0004, -- raw 0x0004: cyberware word bit 2 [C]
    SPURS = 0x0008, -- raw 0x0008: cyberware word bit 3 [C]
    SMARTLINK = 0x0010, -- raw 0x0010: cyberware word bit 4 [C]
}
E.CombatInteractionFlag = {
    NO_COVERAGE_DIFFICULTY_BONUS = 0x80, -- raw 0x80: prevents the +4 condition adjustment [C value, H flag name]
    DEFENSE_DIFFICULTY_REDUCTION = 0x20, -- raw 0x20: subtracts two in defense_modifier [C value, H flag name]
}
E.WeaponClass = { LIGHT_PISTOL = 0, HEAVY_PISTOL = 1, SMG = 2, SHOTGUN = 3, THROWN = 4 }  -- grouping of 0x15318 rows [C]
E.WeaponSlot  = { NONE = 0x7F }                                                     -- +0x56 [C]
-- note [C]: 0x13B00 compares against 0x0D, so grenades (0x0D-0x0F) are NOT picked by switch_to_weapon.
E.FIRST_NON_WEAPON = E.ItemId.FRAG_GRENADE

E.Pickup = { CLIPS_FLAG = 0xC000, NUYEN_FLAG = 0x8000, GARGOYLE_HORN = 0x64, HOUND_PELT = 0x65 }  -- 0x133A6 [C]
-- Stable content identities used by gameplay. The host maps these IDs to the
-- legacy group-item save representation; Lua never addresses its bit positions.
E.GroupItemId = {
    PROTOTYPE_CYBERHEART = "prototype_cyberheart",
    INDIAN_LANDS_VISA = "indian_lands_visa",
    FEATHERED_SCALE = "feathered_scale",
    HELL_HOUND_PELT = "hell_hound_pelt",
    GARGOYLE_HORN = "gargoyle_horn",
    GIFT_LADY_GILLIAN = "gift_lady_gillian",
    GIFT_RED_BUFFALO_WOMAN = "gift_red_buffalo_woman",
    LANDMARK_MAP = "landmark_map",
    MAP_OVERLAY = "map_overlay",
    CURRENT_RUN_PACKAGE = "current_run_package",
    SECURITY_PASS = "security_pass",
    WEAPON_PERMIT = "weapon_permit",
    EYE_FIVER_EAR_CUFF = "eye_fiver_ear_cuff",
    HALLOWEENER_PATCH = "halloweener_patch",
    ORK_ARMBAND = "ork_armband",
}

-- Spell types: spell record = item 0x33 + type; held weapon_code = type + 1 [C]
E.SpellType = { FLAME_DART = 0, FLAME_BOLT = 1, HELLBLAST = 2, INVISIBILITY = 3, SLEEP = 4, STINK = 5,
                SUPER_BARRIER = 6, MANA_ZAP = 7, MANA_BLAST = 8, MANA_STORM = 9, ROCKSKIN = 10,
                HEAL_WOUNDS = 11, CONFUSION = 12, BARRIER = 13 }
E.SPELL_ITEM_BASE = 0x33
-- Live action codes in +0x57 use the one-based D4EA0 row index. 0x15830
-- sends codes 7 and 14 to the field-barrier branch at 0x49A2 [C].
E.SpellActionCode = {
    FLAME_DART = 1,       -- +0x57=0x01 / D4EA0 row 0 [C]
    FLAME_BOLT = 2,       -- +0x57=0x02 / D4EA0 row 1 [C]
    HELLBLAST = 3,        -- +0x57=0x03 / D4EA0 row 2 [C]
    INVISIBILITY = 4,     -- +0x57=0x04 / D4EA0 row 3 [C]
    SLEEP = 5,            -- +0x57=0x05 / D4EA0 row 4 [C]
    STINK = 6,            -- +0x57=0x06 / 0x1588C [C]
    SUPER_BARRIER = 7,    -- +0x57=0x07 / 0x15830 [C]
    MANA_ZAP = 8,         -- +0x57=0x08 / D4EA0 row 7 [C]
    MANA_BLAST = 9,       -- +0x57=0x09 / D4EA0 row 8 [C]
    MANA_STORM = 10,      -- +0x57=0x0A / D4EA0 row 9 [C]
    ROCKSKIN = 11,        -- +0x57=0x0B / D4EA0 row 10 [C]
    HEAL_WOUNDS = 12,     -- +0x57=0x0C / D4EA0 row 11 [C]
    CONFUSION = 13,       -- +0x57=0x0D / 0x1589C [C]
    BARRIER = 14,         -- +0x57=0x0E / 0x15830 [C]
}
E.SpellEffectKind = {
    NONE = 0,              -- 0x00 / 0x1584C returns when +0xA2 is zero [C]
    FIELD_BARRIER = 1,     -- 0x01 / 0x15830 -> 0x49A2 [C]
}
E.SpellTuning = {
    FIELD_BARRIER_TICKS_PER_SUCCESS = 0x0190, -- 0x49F2 / 0x4ADC [C]
}
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
    DAC_SAMPLE_2E = 0x2E, DAC_SAMPLE_2F = 0x2F,
    DAC_SAMPLE_40 = 0x40, -- 0xB03E kind 3 player C072 hurt cue [C]
    DAC_SAMPLE_41 = 0x41,
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
    SPELL_EFFECT_34 = 0x34, SPELL_EFFECT_35 = 0x35,
    SPELL_EFFECT_36 = 0x36, -- action-effect table 0x4946 selects this for spell code 13 [C]
    SPELL_EFFECT_37 = 0x37,
    SPELL_EFFECT_38 = 0x38, SPELL_EFFECT_39 = 0x39, SPELL_EFFECT_3A = 0x3A,
    FIELD_3B = 0x3B,                 -- 0x49A2 -> 0x4AA4 field barrier [C]
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
    THINK_FRAMES = 0x14, THINK_RETRY = -1, -- 0x13904 + FFE175 retry remains negative [C]
    ARRIVE_DIST = 0x1E0, FLANK_RADIUS = 0xE0, FLANK_ANGLE_DEGREES = 135,
    CLOSE_STRIKE_RANGE = 0x200, -- 0x15C58 accepts an actor target below this distance [C]
    PATROL_WAYPOINT_STRIDE = 0x4, -- 0x142F4 advances +0xE4 by one XY pair [C]
    FOLLOW_RADIUS_INITIAL = 0x20, -- 0x14512 first writes this radius [C]
    FOLLOW_RADIUS = 0xE0, -- 0x14516 overwrites the prior radius [C]
    FOLLOW_ANGLE_STEP_DEGREES = 135, -- 0x14532 / 0x1452A [C]
    FOLLOW_COLLISION_CLEAR_MAX = 0, -- 0x1455A/0x14580 accept nonpositive results [C]
    HELL_HOUND_CLOSE_RANGE = 0x0300, -- 0x14D04 close range action branch [C]
    HELL_HOUND_PURSUIT_RANGE = 0x0600, -- 0x14D04 pursuit branch ends at 0x0600 [C]
    PATROL_INITIAL_CURSOR = 0, -- 0x142FE clears +0xE4 at the list terminator [C]
    WORD_MAX = 0xFFFF, -- 0x1450E coordinate inputs are 16-bit words [C]
    FLEE_OFFSET = 0x2580, FLEE_CAMERA_BIAS = 0xA00, ENGAGE_DIST = 0x500, WATCH_DIST = 0x640,
    THON_ACTION_INTERVAL = 8, -- ROM 0x13A66 reloads after the shared action cooldown expires [C]
    HURT_BELOW = 4, CAST_MIN_STUN = 5, SPELL_GATE_MAX = 2, APPROACH_TICKS = 6,
    QUARTER_TURN_DEGREES = 90, NO_TARGET = 0x12345678, STEER_NONE = 0x1000, ALARM_FRAMES = 0x960,
    RELOAD_ROUNDS = 0x26, PATH_TRIES = 5, ANIM_TIMER_MASK = 0x7FF,
    TARGET_LIST_PHASE_MASK = 0x3F, -- raw 0x3F: FFDF88 mask at 0x13874 [C]
    WORLD_TARGET_LIST_PHASE = 0x0B, -- raw 0x0B: rebuild 0x1438C at 0x13874 [C]
    RUNNER_TARGET_LIST_PHASE = 0x0F, -- raw 0x0F: rebuild 0x14446 at 0x13874 [C]
}
-- Frozen pre-split reference only. Active Lua movement receives named host outcomes.
E.Collision = { BLOCKED = 0x7F, LOW_BITS = 0x1F, IMPASSABLE_FROM = 0x18 }
E.EntryDelayMax = { STATIC = 15, DYNAMIC_ENCOUNTER = 0x28 }                   -- repo heartbeat guard [H names]
E.Grid = {
    COLUMNS = 64,
    CELL_SIZE = 512,
    CELL_CENTER_OFFSET = 256,
} -- map grid derived from FF1700 [C]
E.FieldBarrier = {
    DIRECTIONS = {
        "north", "northeast", "east", "southeast",
        "south", "southwest", "west", "northwest",
    },
    -- World-space placement offsets recovered from 0x4AAC [C]. The values are
    -- expressed in map units; source heading words and action bytes stay at
    -- the native adapter boundary.
    CENTER_OFFSET_BY_DIRECTION = {
        north = { x = 0, y = -1152 },
        northeast = { x = 814, y = -814 },
        east = { x = 1152, y = 0 },
        southeast = { x = 814, y = 814 },
        south = { x = 0, y = 1152 },
        southwest = { x = -814, y = 814 },
        west = { x = -1152, y = 0 },
        northwest = { x = -814, y = -814 },
    },
    -- Relative grid cells form the wall shape. These offsets are named by
    -- compass direction instead of flattened row-major memory positions [C:
    -- 0x4BB0..0x4C02].
    FOOTPRINT_BY_DIRECTION = {
        north = {
            { x = 0, y = 0 }, { x = 1, y = 0 }, { x = -1, y = 0 },
            { x = 2, y = 0 }, { x = -2, y = 0 },
        },
        northeast = {
            { x = 0, y = 0 }, { x = -1, y = 0 }, { x = -2, y = 0 },
            { x = 0, y = 1 }, { x = 0, y = 2 },
        },
        east = {
            { x = 0, y = 0 }, { x = 0, y = -1 }, { x = 0, y = 1 },
            { x = 0, y = -2 }, { x = 0, y = 2 },
        },
        southeast = {
            { x = 0, y = 0 }, { x = -1, y = 0 }, { x = -2, y = 0 },
            { x = 0, y = -1 }, { x = 0, y = -2 },
        },
        south = {
            { x = 0, y = 0 }, { x = 1, y = 0 }, { x = -1, y = 0 },
            { x = 2, y = 0 }, { x = -2, y = 0 },
        },
        southwest = {
            { x = 0, y = 0 }, { x = 1, y = 0 }, { x = 2, y = 0 },
            { x = 0, y = -1 }, { x = 0, y = -2 },
        },
        west = {
            { x = 0, y = 0 }, { x = 0, y = -1 }, { x = 0, y = 1 },
            { x = 0, y = -2 }, { x = 0, y = 2 },
        },
        northwest = {
            { x = 0, y = 0 }, { x = 1, y = 0 }, { x = 2, y = 0 },
            { x = 0, y = 1 }, { x = 0, y = 2 },
        },
    },
    SPRITE_FRAME_COUNT = 4,
    SPRITE_VARIANT_COUNT = 5,
    SPRITE_SEGMENT_COUNT = 2,
    SPRITE_PIXEL_OFFSETS_BY_DIRECTION = {
        north = {
            { x = 0, y = 0 }, { x = 32, y = 0 }, { x = -32, y = 0 },
            { x = 64, y = 0 }, { x = -64, y = 0 },
        },
        northeast = {
            { x = 0, y = 0 }, { x = -32, y = 0 }, { x = -64, y = 0 },
            { x = 0, y = 32 }, { x = 0, y = 64 },
        },
        east = {
            { x = 0, y = 0 }, { x = 0, y = -32 }, { x = 0, y = 32 },
            { x = 0, y = -64 }, { x = 0, y = 64 },
        },
        southeast = {
            { x = 0, y = 0 }, { x = -32, y = 0 }, { x = -64, y = 0 },
            { x = 0, y = -32 }, { x = 0, y = -64 },
        },
        south = {
            { x = 0, y = 0 }, { x = 32, y = 0 }, { x = -32, y = 0 },
            { x = 64, y = 0 }, { x = -64, y = 0 },
        },
        southwest = {
            { x = 0, y = 0 }, { x = 32, y = 0 }, { x = 64, y = 0 },
            { x = 0, y = -32 }, { x = 0, y = -64 },
        },
        west = {
            { x = 0, y = 0 }, { x = 0, y = -32 }, { x = 0, y = 32 },
            { x = 0, y = -64 }, { x = 0, y = 64 },
        },
        northwest = {
            { x = 0, y = 0 }, { x = 32, y = 0 }, { x = 64, y = 0 },
            { x = 0, y = 32 }, { x = 0, y = 64 },
        },
    },
}
E.Heading = { UP = 0x0000, RIGHT = 0x4000, DOWN = 0x8000, LEFT = 0xC000 }      -- +0x2E/+0x34 words (emulator run5) [C]
E.Direction = { UP = 0, RIGHT = 2, DOWN = 4, LEFT = 6 }                                -- 0x148CA index [C]
E.DirectionStep = { [0] = -0x40, [2] = 1, [4] = 0x40, [6] = -1 }                      -- 0x148CA values [C]

-- ===== Menus (refactor/menu, docs/09) =====
-- FFDD22 event word from 0x195C: high byte = new edges, low byte = held [C]
E.Pad = { UP = 0x0100, DOWN = 0x0200, LEFT = 0x0400, RIGHT = 0x0800, B = 0x1000, C = 0x2000, A = 0x4000, START = 0x8000,
          HELD_UP = 0x0001, HELD_DOWN = 0x0002, HELD_LEFT = 0x0004, HELD_RIGHT = 0x0008,
          EDGES = 0xFF00, DPAD = 0x0F00 }
E.MenuSound = { MOVE = 0x25, SELECT = 0x24, JACK_IN = 0x1A, DAMAGE = 0x18 }   -- 0x1A: 0xE47A jack-in [C]   -- 0x10E2E / 0x10E4E / 0x11A72 via 0x530DE [C]
E.MatrixDeckMode = { ATTACK = 0xFF, BALANCED = 0x00, DEFENSE = 0x01 }      -- FFF0B7: A/B/C split the Matrix pool [C]
-- Signed view used by ROM branches after FFF0B7 is loaded into a signed word.
E.MatrixModeResult = { ATTACK = -1, BALANCED = 0, DEFENSE = 1 }
-- Native Matrix run lifecycle translated from the source run word and its
-- completion gate into explicit gameplay state.
E.MatrixRunStatus = {
    PENDING = "pending",
    INACTIVE = "inactive",
    ACTIVE = "active",
    COMPLETED = "completed",
}
E.MatrixRunAction = {
    DELETE_FILE = "delete_file",
    UPLOAD_FILE = "upload_file",
    CRASH_SYSTEM = "crash_system",
    DOWNLOAD_FILE = "download_file",
}
-- Stable Matrix UI message identities. ROM text pointers are import provenance,
-- never gameplay results or runtime lookup keys.
E.MatrixMessageId = {
    ACTION_FAILED = "matrix.action_failed",
    ALERT_CANCELLED = "matrix.alert_cancelled",
    NO_ALERTS = "matrix.no_alerts",
    SHUTDOWN = "matrix.shutdown",
    OFFLINE = "matrix.offline",
    ERASE_OBJECTIVE = "matrix.erase_objective",
    ERASED = "matrix.erased",
    LOCKOUT = "matrix.lockout",
    SEARCHING = "matrix.searching",
    NOTHING = "matrix.nothing",
    OBJECTIVE_FOUND = "matrix.objective_found",
    UPLOAD_DONE = "matrix.upload_done",
    STORED_OBJECTIVE = "matrix.stored_objective",
    STORED = "matrix.stored",
    SEARCH_DOT = "matrix.search_dot",
    STORY_FILE = "matrix.story_file",
    FILE_METER = "matrix.file_meter",
    DOWNLOAD_FILE = "matrix.download_file",
    DOWNLOAD_CANCELLED = "matrix.download_cancelled",
    FIVE_FILES = "matrix.five_files",
    NO_STORAGE = "matrix.no_storage",
    STORAGE_SUFFIX = "matrix.storage_suffix",
    SYSTEM_ALERT = "matrix.system_alert",
    SYSTEM_OVERLOAD = "matrix.system_overload",
}
-- Stable gameplay identities for the Johnsons that offer work. Native import
-- code maps source scene/context values to these IDs before Lua sees them.
E.RunId = {
    GHOUL_BOUNTY = "ghoul_bounty",
    BODYGUARD_ESCORT = "bodyguard_escort",
    PACKAGE_DELIVERY = "package_delivery",
    GANG_CONTRACT = "gang_contract",
    PACKAGE_RECOVERY = "package_recovery",
    EMPLOYEE_RECRUITMENT = "employee_recruitment",
    MATRIX_OBJECTIVE = "matrix_objective",
}
E.RunContact = {
    JUMP_HOUSE = "jump_house",
    MATCHSTICK = "matchstick",
    BIG_RHINO = "big_rhino",
    WANDERER = "wanderer",
    ICARUS_DESCENDING = "icarus_descending",
}
E.PartyWipeDestination = {
    LITTLE_CHIBA = 0, -- 0xC7EC -> 0x2A44, scene 2 [C]
    DOCWAGON = 1, -- 0xC7E0 -> 0x2A32, scene 0 [C]
    NONE = 0xFF, -- no pending wake destination [H host sentinel]
}
E.PartyWipeRunnerRefresh = {
    FIRST_SLOT = 0, -- 0x397A wraps the active pointer to FF0100 [C]
    SLOT_COUNT = 3, -- 0x391A walks the three player records [C]
    TERMINAL_RECORD_TYPE = 2, -- 0x3964 wraps after the type-2 record [C]
}
-- 0x2AEA/0x2B24 arrival records, consumed by 0x2B78 and aligned with
-- ANDI.W #0xFF80 before the scene loader publishes FFFBF8 [C].
E.PartyWipeArrival = {
    [E.PartyWipeDestination.DOCWAGON] = {
        scene = 0, x = 0x6200, y = 0x2D00,
        -- 0x2AEA raw (0x6260,0x2D40), aligned by 0x2B90 [C]
    },
    [E.PartyWipeDestination.LITTLE_CHIBA] = {
        scene = 2, x = 0x6680, y = 0x2300,
        -- 0x2B24 raw (0x66F0,0x2310), aligned by 0x2B90 [C]
    },
}
-- Lua menu state names the visible section. The numeric page selectors and
-- ROM handler addresses stay in the native boundary enum source used only to
-- generate the C++ adapter enum; gameplay scripts do not receive them.
E.CommandSection = {
    INVENTORY_LEFT = "inventory_left",
    INVENTORY_RIGHT = "inventory_right",
    CLIPS = "clips",
    ATTRIBUTES_SKILLS = "attributes_and_skills",
    CYBERWARE = "cyberware",
    MAGIC = "magic",
    POCKET_SECRETARY = "pocket_secretary",
}
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
E.FieldReturnTransitionPhase = {
    NONE = 0, MENU_FADE_OUT = 1, BLACK_HOLD = 2, FIELD_FADE_IN = 3, COMPLETE = 4,
}
E.FieldReturnTransitionTiming = {
    MENU_UNCHANGED_THROUGH = 2,
    FADE_OUT_FIRST_FRAME = 3,
    FADE_OUT_LAST_FRAME = 15,
    BLACK_HOLD_LAST_FRAME = 99,
    FADE_IN_FIRST_FRAME = 100,
    FIELD_LIVE_FRAME = 112,
    PALETTE_STEPS = 7,
}
E.DisplayBlank = { OFF = 0x8134, ON = 0x8174 }   -- VDP reg 1 writes around a build [C]
E.MenuMask = { LEAD = 1, REVEAL = 2 }   -- port host phases for staged partial frames (not ROM values)
E.WindowPlane = { SIDEBAR = 0x9191, OFF = 0x9100 } -- VDP reg 0x11 [C]

E.KarmaUpgrade = {
    AVAILABLE = "available", INVALID = "invalid",
    NOT_UPGRADEABLE = "not_upgradeable", UNKNOWN_RACE = "unknown_race",
    CLASS_RESTRICTED = "class_restricted", AT_LIMIT = "at_limit",
} -- semantic outcomes for the 0xD82A upgrade rules [C]
E.Skill = { SORCERY = 0, FIREARMS = 1, PISTOLS = 2, SMGS = 3, SHOTGUNS = 4, BIOTECH = 8 }   -- selection order in the Attributes/Skills grid [C]
E.RunnerRace = {
    HUMAN = "human", ELF = "elf", DWARF = "dwarf", ORC = "orc", TROLL = "troll",
} -- named profile race translated from +0x9A [C 0xC22A6]
E.RunnerClass = {
    MAGE = "mage", SHAMAN = "shaman", SAMURAI = "samurai", DECKER = "decker",
}
E.RunnerClass.CASTERS = {
    [E.RunnerClass.MAGE] = true, [E.RunnerClass.SHAMAN] = true,
} -- class-specific gameplay set; source caster gate is +0x9B < 2 at 0xFBA6 [C]
E.RunnerAttribute = {
    BODY = "body", QUICKNESS = "quickness", STRENGTH = "strength",
    CHARISMA = "charisma", INTELLIGENCE = "intelligence", WILLPOWER = "willpower",
} -- named runner profile values translated by the native record adapter [C 0xC1A2A]
E.RunnerSkill = {
    SORCERY = "sorcery", FIREARMS = "firearms", PISTOLS = "pistols",
    SMGS = "smgs", SHOTGUNS = "shotguns", MELEE_COMBAT = "melee_combat",
    THROWING = "throwing", COMPUTER = "computer", BIOTECH = "biotech",
    ELECTRONICS = "electronics", REPUTATION = "reputation",
    NEGOTIATION = "negotiation",
} -- named runner profile values translated by the native record adapter [C 0xC1A2A]
-- Semantic action names stay in Lua. Their source selector values belong to
-- tools/native_boundary_enums.lua, used only by native compatibility adapters.
E.InventoryAction = {
    EQUIP_OR_USE = "equip_or_use",
    DISCARD = "discard",
    TRADE = "trade",
    CANCEL = "cancel",
}
E.ItemBranch = {
    FIRST_AID = "first_aid",
    AUTOMATIC_USE = "automatic_use",
    WEAPON_OR_GEAR = "weapon_or_gear",
    ARMOR = "armor",
}
E.WeaponSelectionState = {
    EMPTY = "empty",
}
E.ClipsAction = {
    RELOAD = "reload",
    TRADE = "trade",
    DISTRIBUTE = "distribute",
    CANCEL = "cancel",
}
E.ClipTradeAction = {
    INCREASE = "increase",
    DECREASE = "decrease",
    TRADE = "trade_clips",
    CHANGE_RECIPIENT = "change_recipient",
}
E.DebugOption = {
    NUYEN = "nuyen",
    SPELLS = "spells",
    TEST_DECK = "test_deck",
    PICK_SYSTEM = "pick_system",
    HEAL_ALL = "heal_all",
    KARMA = "karma",
    CONTACTS = "contacts",
    WALK_FAST = "walk_fast",
}
E.SaveLoadOption = { SAVE = "save", LOAD = "load" } -- ROM 0xC2600 order [C]
E.PocketOption = {
    NOTEBOOK = "notebook",
    GROUP_ITEMS = "group_items",
    CYBERDECK = "cyberdeck",
    DISMISS_RUNNER = "dismiss_runner",
    SAVE_LOAD = "save_load",
    DEBUG = "debug",
}
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
E.NotebookBoundary = { EVENT_GATES_UNRESOLVED = 0, TIPS_UNRESOLVED = 1,
                       CURRENT_RUN_UNRESOLVED = 2, CONTACTS_UNRESOLVED = 3,
                       SHADOWRUNNERS_UNRESOLVED = 4,
                       MR_JOHNSON_UNRESOLVED = 5 }
E.MenuMessage = { OUT_OF_AMMO = 1, ITEM_NO_CLIPS = 2, NO_PARTY = 3, RELOADED = 4, EXCHANGE_COMPLETE = 5, CANNOT_DISCARD = 6,
                  CANNOT_TRADE = 7, AUTOMATIC_USE = 8, FIRST_AID_USED = 9, SLAP_PATCH_USED = 10, NOT_INJURED = 11,
                  INSUFFICIENT_KARMA = 12, MAGE_ONLY = 13, NOT_RAISABLE = 14, MAX_LEVEL = 15, NO_CURRENT_RUN = 16,
                  NO_CYBERDECK = 17, GROUP_ITEMS = 18, MR_JOHNSON_DETAIL = 19, MAGICAL_DEFENSE = 20,
                  FUNCTION_ABORTED = 21, SAVE_OUTSIDE_ONLY = 22, SAVE_IN_COMBAT = 23, GAME_SAVED = 24,
                  LOAD_EMPTY = 25, LOAD_OK = 26,
                  DECK_NO_SLOTS = 27, DECK_NO_MEMORY = 28, NOT_ON_RUN = 29, RUN_COMPLETE = 30, NO_DATAFILES = 31,
                  RETREAT_COMBAT = 32, RETREAT_NO_NODE = 33,
                  NOTEBOOK_CONTACT_DETAIL = 34, NOTEBOOK_SHADOWRUNNER_DETAIL = 35 }   -- Detail waits share ROM 0xF312 edge loop [C]


-- StoryFacts are named gameplay concepts. The host persistence adapter maps
-- these identities to the original save representation; Lua does not expose
-- storage positions, trigger registers, or ROM command numbers.
-- ROM addresses document where each fact is sourced. Names marked [H]
-- remain readings rather than confirmed labels.
E.StoryFact = {
  MOTEL_BILL_PAID = "motel_bill_paid",                 -- payment at 0x21410 [C site][H name]
  TARISLAR_ASKED_SHALE = "tarislar_asked_shale",       -- question at 0xF49FC [C]
  OWLFEATHER_ASKED = "owlfeather_asked",               -- topic at 0xF4B68 [C]
  ARAGORN_FOUND = "aragorn_found",                     -- service at 0x53C96 [C]
  ARAGORN_FREED_TOLD = "aragorn_freed_told",           -- follow-up at 0x53D4E [C]
  SHAMAN_MET = "shaman_met",                           -- conversation at 0xF4BD8 [C]
  SHAMAN_ITEM4_GIVEN = "shaman_item4_given",           -- service at 0x53DD0 [C]
  SHAMAN_PELT_GIVEN = "shaman_pelt_given",             -- service at 0x53E38 [C]
  SERPENT_SCALE_TAKEN = "serpent_scale_taken",         -- activity at 0x53F50 [C]
  SHAMAN_TASKS_DONE = "shaman_tasks_done",             -- service at 0x53EA8 [C]
  MICHAEL_ALIAS_LEARNED = "michael_alias_learned",     -- dialogue at 0xF5414 [C site][H name]
  MAKO_CLUB_LEAD = "mako_club_lead",                   -- Big Rhino info and Club entry at 0x23222/0x225D2 [C]
  MAKO_DEFEATED = "mako_defeated",                     -- encounter at 0x53FFC [C]
  RENRAKU_MAP_OVERLAY = "renraku_map_overlay",         -- service at 0x540F8 [C]
  MICHAEL_MISSION_TOLD = "michael_mission_told",       -- handler at 0x2193A [C site][H name]
  CYBERHEART_SAFE = "cyberheart_safe",                 -- service at 0x541D8 [C]
  OGAMI_LEFT = "ogami_left",                           -- departure at 0x5427A [C]
  ITO_FIGHT_WON = "ito_fight_won",                     -- encounter at 0x54286 [C]
  RITUAL_OF_BINDING = "ritual_of_binding",             -- handler at 0x21902 [C site][H name]
  RED_BUFFALO_WOMAN = "red_buffalo_woman",             -- service at 0x5435C [C]
  ILENE_MET = "ilene_met",                             -- activity at 0x54402 [C]
  GILLIAN_INITIATION = "gillian_initiation",           -- handler at 0x21E58 [C]
  GARDEN_APTS_ENTERED = "garden_apts_entered",         -- service at 0x5456E [C]
  LANDMARK_MAP_TAKEN = "landmark_map_taken",           -- service at 0x5463C [C]
  HOLOPIX_SHOWN = "holopix_shown",                     -- handler at 0x111006 [C]
  MEDICINE_LODGE_TURNED_AWAY = "medicine_lodge_turned_away", -- service at 0x1FDEA [C site][H name]
  HOSPITAL_VISITED = "hospital_visited",               -- service at 0x1F8C2 [C site][H name]
  RENRAKU_MAP_SHOWN = "renraku_map_shown",             -- dialogue at 0x2203E [C]
  BOTH_MAPS_SHOWN = "both_maps_shown",                 -- dialogue at 0x2205C [C]
  POST_ITO_BRIEFED = "post_ito_briefed",               -- dialogue at 0x21F58 [C site][H name]
  HEART_DELIVERED = "heart_delivered",                 -- service at 0x1F940 [C]
  FROSTY_MET = "frosty_met",                           -- handler at 0x21FD6 [C]
}

-- Transient field-mode state exposed through gameplay meaning. The host maps
-- these IDs to saved fields; scripts do not name FFFC30 bit positions.
E.FieldModeFact = {
  -- Boris's information handlers set this at 0x21B82/0x21EB0; Mako's result
  -- callback clears it at 0x54036. Its broader narrative role is [H].
  BORIS_FOLLOWUP_PENDING = "field.boris_followup_pending",
  -- The Mako callback sets this immediately before returning its result page
  -- at 0x5403E/0x5405E [C].
  MAKO_RESULT_CALLBACK_APPLIED = "field.mako_result_callback_applied",
}

-- Notebook entries are content identities. The host owns their save encoding.
E.NotebookTip = {
  -- Entry 81 is Renraku's report locating the Feathered Serpent in the
  -- Sinsearach Caves; the third pointer table begins at 0xC2816 [C].
  RENRAKU_SERPENT_LOCATION = "notebook_tip.renraku_serpent_location",
}

-- Persistent Johnson contact knowledge. The host maps these IDs to the save
-- representation; gameplay code does not address FFFC27 bit positions.
E.JohnsonFact = {
  -- Notebook row gate for Julius at 0xF482; the writer is unresolved [C].
  JULIUS_KNOWN = "johnson.julius_known",
  MORTIMER_KNOWN = "johnson.mortimer_known", -- Pocket Secretary contact row at 0xF4AC [C]
  CALEB_KNOWN = "johnson.caleb_known", -- Pocket Secretary contact row at 0xF500 [C]
  -- Caleb's first-contact branch at 0x20A78 [C].
  CALEB_FIRST_MEETING_SEEN = "johnson.caleb_first_meeting_seen",
  -- Vigore's alternate response is read by the question builder at 0x22F90 [C].
  VIGORE_ALTERNATE_RESPONSE_AVAILABLE = "johnson.vigore_alternate_response_available",
}

return E

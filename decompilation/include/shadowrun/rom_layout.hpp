#pragma once

#include <array>
#include <cstdint>

#include "shadowrun/genesis_bus.hpp"

namespace shadowrun::rom {

// These offsets are not guesses from a decompiler's data types. They are the
// literal operands used by the reset path and were checked against the raw
// big-endian image.
constexpr std::uint32_t kResetEntry = 0x0236;
constexpr std::uint32_t kResetBootstrapTable = 0x02C4;
constexpr std::uint32_t kRegionSelectorTable = 0x046E;
constexpr std::uint32_t kHeaderRegionField = 0x01F0;
constexpr std::uint32_t kVdpConfigurationTable = 0x0D14;
constexpr std::uint32_t kIndexedVariantDescriptorPointerTable = 0x4ACC;
constexpr std::uint32_t kIndexedOffsetPointerTable = 0x4BB0;
constexpr std::uint32_t kIndexedDescriptorPointerTable = 0x4C0C;
constexpr std::uint32_t kIndirectDispatchTable = 0x12CCB4;
constexpr std::uint32_t kRoutineSelectTable = 0xF3DC6;
constexpr std::uint32_t kStateRoutinePairTable = 0xF3E1E;
constexpr std::uint32_t kEntityDispatchTable = 0x490A;
constexpr std::uint32_t kControllerInputDecodeTable = 0x19D2;
constexpr std::uint32_t kEntityStatePointerTable = 0x62AC;
constexpr std::uint32_t kObjectGenerationPointerTable = 0xAF3E;
constexpr std::uint32_t kObjectGenerationInterpolationTable = 0x15E16;
constexpr std::uint32_t kAnimatedObjectFactorTable = 0x5BEE;
constexpr std::uint32_t kAnimatedObjectPointerTable = 0x5C2E;
constexpr std::uint32_t kEntityCyclePointerTable = 0x4F42;
constexpr std::uint32_t kEntityCycleOffsetTable = 0x4F22;
constexpr std::uint32_t kEntitySlotObjectPointerTable = 0x48B2;
constexpr std::uint32_t kEntitySlotObjectListNegativeA = 0x5F82;
constexpr std::uint32_t kEntitySlotObjectListNegativeB = 0x5FA0;
constexpr std::uint32_t kEntitySlotObjectListType64 = 0x5FAA;
constexpr std::uint32_t kEntitySlotObjectListOther = 0x1D47C;
constexpr std::uint32_t kEntityStateObjectPointerTable0x4FE2 = 0x512A;
constexpr std::uint32_t kEntityStateObjectPointerTable0x508A = 0x5142;
constexpr std::uint32_t kEntityStateObjectPointerTable0x5152 = 0x589E;
constexpr std::uint32_t kEntityStateObjectPointerTable0x521E = 0x58B6;
constexpr std::uint32_t kEntityPhaseObjectPointerTable = 0x5A46;
constexpr std::uint32_t kEntityTimedObjectPointerTable = 0x5756;
constexpr std::uint32_t kEntityStateObjectPointerTable = 0x5876;
constexpr std::uint32_t kEntityRandomObjectPointerTable = 0x585E;
constexpr std::uint32_t kEntityRandomObjectPointerTableAlternate = 0x586A;
constexpr std::uint32_t kEntityStateGridPositionTable = 0x6AB3E;
constexpr std::uint32_t kEntityStateGridAnimationTable = 0x6AC46;
constexpr std::uint32_t kEntityStateGridAnimationTableDefault = 0x6AB96;
constexpr std::uint32_t kEntityStateGridAnimationTableControl = 0x6ABEE;
constexpr std::uint32_t kEntityStateGridAnimationLimitTable = 0xD4540;
constexpr std::uint32_t kEntityStateModifierTable = 0xD4D50;
constexpr std::uint32_t kEntityTimingAdjustmentTable = 0xD4EA0;
constexpr std::uint32_t kEntityAnimationFrameTable = 0xC072;
constexpr std::uint32_t kEntityAnimationFallbackCommandTable = 0xB05E;
constexpr std::uint32_t kEntityAnimationFallbackFrameTable = 0xC0B2;
constexpr std::uint32_t kEntityAnimationLinkedFrameTable = 0xC0A2;
constexpr std::uint32_t kEntitySpawnAnimationPointerTable = 0x16CC8;
constexpr std::uint32_t kEntitySpawnAnimationTableMode2 = 0x16CF0;
constexpr std::uint32_t kEntitySpawnAnimationTableDefault = 0x16D50;
constexpr std::uint32_t kEntityAnimationFallbackRoutine = 0xB38A;
constexpr std::uint32_t kPackedSpriteNibbleTable = 0x3EDC;
constexpr std::uint32_t kFrameDeltaDirectionTable = 0x15DF6;
constexpr std::uint32_t kFrameDeltaCorrectionTable = 0x15F96;
constexpr std::uint32_t kFrameMovementTable = 0x4290;
constexpr std::uint32_t kAnimationDirectionOffsetTable = 0x148CA;
constexpr std::uint32_t kAnimationResourceCoordinateOffsetTable = 0x15A4C;
constexpr std::uint32_t kAnimationResourceProbeOffsetTable = 0x15A6C;
constexpr std::uint32_t kAnimationResourceCommandTable = 0x15318;
constexpr std::uint32_t kAnimationResourceAlternateCommandTable = 0x15336;
constexpr std::uint32_t kAnimationResourceDirectionTable = 0x62EC;
constexpr std::uint32_t kAnimationResourceAlternateDirectionTable = 0x62FC;
constexpr std::uint32_t kCollisionResourceModeTable = 0xEEFE;
constexpr std::uint32_t kEntityReactionOffsetTable = 0xBD34;
constexpr std::uint32_t kEntityMovementDirectionTable = 0xC002;
constexpr std::uint32_t kEntityMovementSelectionPointerTable = 0xBBF6;
constexpr std::uint32_t kEntitySlotCommandTable = 0x4946;
constexpr std::uint32_t kEntitySlotHandlerTable = 0x4964;
constexpr std::uint32_t kEntitySlotNoOpHandler = 0x49A0;
constexpr std::uint32_t kPostObjectOffsetTable = 0x1FE926;
constexpr std::uint32_t kPostObjectPointerTableA = 0x1FEA0E;
constexpr std::uint32_t kPostObjectPointerTableB = 0x1FEA2E;
constexpr std::uint32_t kPostObjectPointerTableC = 0x1FE9E6;
constexpr std::uint32_t kPostObjectPointerTableD = 0x1FE946;
constexpr std::uint32_t kPostObjectUploadTable = 0x1D494E;
constexpr std::uint32_t kPostObjectUploadTableB = 0x5D53C;
constexpr std::uint32_t kPostObjectEntityTemplate = 0x7361A;
constexpr std::uint32_t kPostObjectBackgroundTileTable = 0x1EB3E2;
inline constexpr std::array<GenesisAddress, 4> kEntityObjectVariantTables{
    0x5D92, 0x5DBE, 0x5DA8, 0x5DD4
};
constexpr std::uint32_t kVblankAnimationState0Table = 0x18AD42;
constexpr std::uint32_t kVblankAnimationState5Table = 0x18AD52;
constexpr std::uint32_t kVblankAnimationState2CTable = 0x18AD60;
constexpr std::uint32_t kVblankAnimationMode1Table = 0x18AD70;
constexpr std::uint32_t kVblankAnimationState69Table = 0x18AD7A;
constexpr std::uint32_t kVblankAnimationState4Table = 0x18AD96;

struct ResetBootstrapConstants {
    std::uint16_t word_02c4;
    std::uint16_t word_02c6;
    std::uint16_t word_02c8;
    GenesisAddress address_02ca;
    GenesisAddress address_02ce;
    GenesisAddress address_02d2;
    GenesisAddress address_02d6;
    GenesisAddress address_02da;
};

inline constexpr ResetBootstrapConstants kResetBootstrap{
    0x8000, 0x3FFF, 0x0100,
    0x00A00000, 0x00A11100, 0x00A11200, 0x00C00000, 0x00C00004
};

// Reset indexes this four-byte table with ((IO_VERSION >> 6) & 3). The
// trailing data at 0x0472 is a separate startup-message descriptor area.
inline constexpr std::array<char, 4> kRegionSelector{'U', 'E', '\0', 'J'};

// Function 0x0CEC consumes exactly four RAM words followed by fifteen VDP
// control words from this table (nineteen words in all).
inline constexpr std::array<std::uint16_t, 19> kVdpConfiguration{
    0xC000, 0xE000, 0xD000, 0x0000, 0x8230, 0x8407, 0x8568,
    0x8D00, 0x8B00, 0x8006, 0x8174, 0x833C, 0x8700, 0x8A55,
    0x8C81, 0x8F02, 0x9001, 0x9100, 0x9200
};

// The controller sampler at ROM 0x195C maps the low nibble returned by each
// port probe through this sixteen-word table before writing FFDD04/FFDD06.
inline constexpr std::array<std::uint16_t, 16> kControllerInputDecode{
    0xFFFF, 0x0000, 0x8000, 0xFFFF, 0xC000, 0xE000, 0xA000, 0xFFFF,
    0x4000, 0x2000, 0x6000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

inline constexpr std::array<GenesisAddress, 8> kIndexedOffsetPointers{
    0x4BD0, 0x4BE4, 0x4BDA, 0x4BEE,
    0x4BD0, 0x4BF8, 0x4BDA, 0x4C02
};

// ROM 0x4ADC selects one of these four descriptor pointers with the low two
// bits of the object record word at +0x0E, then advances the selected pointer
// by eight bytes before passing it to the object-list builder.
inline constexpr std::array<GenesisAddress, 4>
    kIndexedVariantDescriptorPointers{
        0x5802, 0x5814, 0x5826, 0x5838
    };

// Functions in the 0x49A2/0x4ADC/0x4B6C cluster index five signed words
// through the table above. These are kept as signed values because the ROM
// uses them as relative tile/map offsets. The fifth word is required by the
// DBF loop at 0x4BA0, whose initial D7 value is four.
inline constexpr std::array<std::array<std::int16_t, 5>, 8> kIndexedOffsetVectors{{
    {{0, 1, -1, 2, -2}},
    {{0, -1, -2, 64, 128}},
    {{0, -64, 64, -128, 128}},
    {{0, -1, -2, -64, -128}},
    {{0, 1, -1, 2, -2}},
    {{0, 1, 2, -64, -128}},
    {{0, -64, 64, -128, 128}},
    {{0, 1, 2, 64, 128}}
}};

inline constexpr std::array<GenesisAddress, 8> kIndexedDescriptorPointers{
    0x4C2C, 0x4C54, 0x4C40, 0x4C68,
    0x4C2C, 0x4C7C, 0x4C40, 0x4C90
};

// ROM 0x1F580 indexes this table in four-byte steps and executes the selected
// address with JSR (A0). The entries are reviewed ROM addresses; their state
// meanings remain intentionally unnamed.
inline constexpr std::array<GenesisAddress, 32> kIndirectDispatchTargets{
    0x021326, 0x021332, 0x021318, 0x021226, 0x0211E8, 0x0213FA,
    0x021458, 0x02148A, 0x0214B0, 0x0214CE, 0x0214F6, 0x02155E,
    0x0215AC, 0x0215DA, 0x02076A, 0x0208A8, 0x021628, 0x021692,
    0x021774, 0x021846, 0x021862, 0x0218B2, 0x0218BC, 0x0218C6,
    0x0218DA, 0x0218E4, 0x0218F8, 0x021902, 0x02193A, 0x021962,
    0x021982, 0x021992
};

// 0x1E69C indexes the first table with the state byte it writes to FFFBED.
inline constexpr std::array<GenesisAddress, 22> kRoutineSelectTargets{
    0x01E744, 0x01E9C0, 0x01E9C0, 0x01EC56, 0x01E810, 0x01E810,
    0x01ED18, 0x01ED2C, 0x01ED24, 0x01E81C, 0x01E7D0, 0x01E81C,
    0x01E7D0, 0x01E770, 0x01EA94, 0x01EB8E, 0x01EC88, 0x01E87A,
    0x01E8FA, 0x01EDF2, 0x01EDF2, 0x01EDF2
};

// 0x214B0 and 0x214CE use eight-byte rows: primary routine plus an optional
// secondary routine. The table is represented as pairs to preserve that
// distinction instead of filtering the zero entries into a flat list.
inline constexpr std::array<std::array<GenesisAddress, 2>, 10>
    kStateRoutinePairs{{
        {{0x01E78C, 0x000000}}, {{0x01EAB4, 0x01EB32}},
        {{0x01EBAA, 0x000000}}, {{0x01ECA4, 0x000000}},
        {{0x01E896, 0x000000}}, {{0x01E91A, 0x01E9AC}},
        {{0x01EE0E, 0x000000}}, {{0x01F1FC, 0x01EF2C}},
        {{0x01EF2C, 0x01F086}}, {{0x01F046, 0x01F106}}
    }};

// 0x46F6 selects a handler from this longword table using the entity byte at
// offset +0xB2. The next longword after this reviewed run is data, so it is
// intentionally not included.
inline constexpr std::array<GenesisAddress, 15> kEntityDispatchTargets{
    0x0049A0, 0x00521E, 0x005152, 0x004CDE, 0x0049A0, 0x0052BE,
    0x00557E, 0x0049A0, 0x00508A, 0x004FE2, 0x004D54, 0x0049A0,
    0x0056DE, 0x0053C4, 0x0049A0
};

} // namespace shadowrun::rom

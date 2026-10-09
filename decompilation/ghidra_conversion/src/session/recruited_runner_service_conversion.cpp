#include "shadowrun_ghidra/session/recruited_runner_service_conversion.hpp"

#include <array>
#include <cstddef>
#include <optional>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::size_t kServiceFlagByteOffset = 0x10U;
constexpr std::size_t kSourcePositionYOffset = 0x04U;
constexpr std::uint8_t kServiceActiveBit = 0x01U;
constexpr std::uint8_t kPermanentPopulationByte = 0xFFU;
constexpr std::uint8_t kStarkTemplateIndex = 9U;
constexpr std::size_t kGenericConversationRunnerCount = 9U;
constexpr std::size_t kInformationRunnerCount = 10U;

struct RecruitedRunnerServiceFeeBase {
    std::uint16_t short_term = 0;
    std::uint16_t permanent = 0;
};

// ROM 0xFE640 contains ten 0x10-byte runner service records. The first four
// bytes are resource pointers; the two words at +0x04/+0x06 are the fee bases
// consumed by 0x2128E and 0x212E2. Index 9 is Stark's special zero-base entry.
constexpr std::array<RecruitedRunnerServiceFeeBase, 10>
    kRecruitedRunnerServiceFeeBases{{
        {0x0050U, 0x0320U},
        {0x00C8U, 0x07D0U},
        {0x0096U, 0x05DCU},
        {0x0096U, 0x05DCU},
        {0x015EU, 0x0DACU},
        {0x012CU, 0x0BB8U},
        {0x0190U, 0x0FA0U},
        {0x0258U, 0x1770U},
        {0x02BCU, 0x1B58U},
        {0x0000U, 0x0000U},
    }};

// ROM 0x12CA54. The source indexes this table with the selected runner's
// FFFC12 progress byte plus one.
constexpr std::array<std::uint8_t, 26> kServiceProgressMultipliers{{
    0x1EU, 0x14U, 0x0FU, 0x0AU, 0x0AU, 0x09U, 0x11U, 0x02U, 0x12U,
    0x02U, 0x0DU, 0x02U, 0x0EU, 0x02U, 0x0FU, 0x02U, 0x11U, 0x04U,
    0x12U, 0x04U, 0x0DU, 0x04U, 0x0EU, 0x04U, 0x0FU, 0x04U,
}};

std::uint32_t divide_unsigned_word(const std::uint32_t value,
                                    const std::uint16_t divisor) {
    // DIVU.W leaves the quotient in the low word. The source immediately
    // clears the high word with ANDI.L #$FFFF, so expose that result directly.
    return (value / divisor) & 0xFFFFU;
}

std::optional<RecruitedRunnerConversationRunner> generic_conversation_runner(
    const std::size_t index) {
    constexpr std::array<RecruitedRunnerConversationRunner,
                         kGenericConversationRunnerCount>
        kRunners{{RecruitedRunnerConversationRunner::ricky,
                  RecruitedRunnerConversationRunner::winston_marrs,
                  RecruitedRunnerConversationRunner::trent,
                  RecruitedRunnerConversationRunner::petr_uvehr,
                  RecruitedRunnerConversationRunner::walking_bear,
                  RecruitedRunnerConversationRunner::phantom,
                  RecruitedRunnerConversationRunner::two_fists,
                  RecruitedRunnerConversationRunner::freya,
                  RecruitedRunnerConversationRunner::rianna}};
    if (index >= kRunners.size()) {
        return std::nullopt;
    }
    return kRunners[index];
}

struct RecruitedRunnerInformationRecord {
    RecruitedRunnerConversationRunner runner;
    RecruitedRunnerInformationTopic topic;
    std::uint16_t flag;
    RecruitedRunnerInformationTextStream opening_text;
    RecruitedRunnerInformationTextStream continuation_text;
};

// The source descriptors at 0xFE848..0xFF8F0 still choose these streams and
// segment counts. ROM text is imported into dialogue/script-text.lua so the
// native state conversion carries semantic content IDs rather than narrative
// copy; dialogue.lua maps each ID to its imported catalog key.
constexpr RecruitedRunnerInformationTextStream kRickyInformationOpening{
    "information.opening", 3U};
constexpr RecruitedRunnerInformationTextStream kRickyInformationContinuation{
    "information.continuation", 3U};
constexpr RecruitedRunnerInformationTextStream kWinstonInformationOpening{
    "information.opening", 3U};
constexpr RecruitedRunnerInformationTextStream kWinstonInformationContinuation{
    "information.continuation", 3U};
constexpr RecruitedRunnerInformationTextStream kTrentInformationOpening{
    "information.opening", 4U};
constexpr RecruitedRunnerInformationTextStream kTrentInformationContinuation{
    "information.continuation", 1U};
constexpr RecruitedRunnerInformationTextStream kPetrInformationOpening{
    "information.opening", 2U};
constexpr RecruitedRunnerInformationTextStream kPetrInformationContinuation{
    "information.continuation", 2U};
constexpr RecruitedRunnerInformationTextStream kWalkingBearInformationOpening{
    "information.opening", 3U};
constexpr RecruitedRunnerInformationTextStream kWalkingBearInformationContinuation{
    "information.continuation", 2U};
constexpr RecruitedRunnerInformationTextStream kPhantomInformationOpening{
    "information.opening", 3U};
constexpr RecruitedRunnerInformationTextStream kPhantomInformationContinuation{
    "information.continuation", 3U};
constexpr RecruitedRunnerInformationTextStream kTwoFistsInformationOpening{
    "information.opening", 2U};
constexpr RecruitedRunnerInformationTextStream kTwoFistsInformationContinuation{
    "information.continuation", 1U};
constexpr RecruitedRunnerInformationTextStream kFreyaInformationOpening{
    "information.opening", 2U};
constexpr RecruitedRunnerInformationTextStream kFreyaInformationContinuation{
    "information.continuation", 3U};
constexpr RecruitedRunnerInformationTextStream kRiannaInformationOpening{
    "information.opening", 4U};
constexpr RecruitedRunnerInformationTextStream kRiannaInformationContinuation{
    "information.continuation", 3U};
constexpr RecruitedRunnerInformationTextStream kStarkInformationOpening{
    "information.opening", 1U};
constexpr RecruitedRunnerInformationTextStream kStarkInformationContinuation{
    "information.continuation", 1U};

constexpr std::array<RecruitedRunnerInformationRecord,
                     kInformationRunnerCount>
    kRecruitedRunnerInformationRecords{{
        {RecruitedRunnerConversationRunner::ricky,
         RecruitedRunnerInformationTopic::ricky_glow_city_origin,
         0x0001U,
         kRickyInformationOpening,
         kRickyInformationContinuation},
        {RecruitedRunnerConversationRunner::winston_marrs,
         RecruitedRunnerInformationTopic::winston_marrs_identity,
         0x0002U,
         kWinstonInformationOpening,
         kWinstonInformationContinuation},
        {RecruitedRunnerConversationRunner::trent,
         RecruitedRunnerInformationTopic::trent_identity,
         0x0004U,
         kTrentInformationOpening,
         kTrentInformationContinuation},
        {RecruitedRunnerConversationRunner::petr_uvehr,
         RecruitedRunnerInformationTopic::petr_uvehr_cyberware_motivation,
         0x0008U,
         kPetrInformationOpening,
         kPetrInformationContinuation},
        {RecruitedRunnerConversationRunner::walking_bear,
         RecruitedRunnerInformationTopic::walking_bear_transformation,
         0x0010U,
         kWalkingBearInformationOpening,
         kWalkingBearInformationContinuation},
        {RecruitedRunnerConversationRunner::phantom,
         RecruitedRunnerInformationTopic::phantom_matrix_background,
         0x0020U,
         kPhantomInformationOpening,
         kPhantomInformationContinuation},
        {RecruitedRunnerConversationRunner::two_fists,
         RecruitedRunnerInformationTopic::two_fists_ares_run,
         0x0040U,
         kTwoFistsInformationOpening,
         kTwoFistsInformationContinuation},
        {RecruitedRunnerConversationRunner::freya,
         RecruitedRunnerInformationTopic::freya_cascade_crow_background,
         0x0080U,
         kFreyaInformationOpening,
         kFreyaInformationContinuation},
        {RecruitedRunnerConversationRunner::rianna,
         RecruitedRunnerInformationTopic::rianna_heartbane_identity,
         0x0100U,
         kRiannaInformationOpening,
         kRiannaInformationContinuation},
        {RecruitedRunnerConversationRunner::stark,
         RecruitedRunnerInformationTopic::stark_amerindian_heritage,
         0x0200U,
         kStarkInformationOpening,
         kStarkInformationContinuation},
    }};

constexpr std::string_view kMoreOptionContentId = "shared.more";

// Ricky's subject descriptors at 0xFE894..0xFE908 each point at a complete
// encoded text stream. The source descriptors are the page boundaries; the
// segment arrays below preserve the text inside each page as well.
constexpr RecruitedRunnerInformationTextStream
    kRickyShamanMagicPageOne{
        "subject.shaman_magic.page_0", 3U};
constexpr RecruitedRunnerInformationTextStream
    kRickyShamanMagicPageTwo{
        "subject.shaman_magic.page_1", 4U};
constexpr RecruitedRunnerInformationTextStream
    kRickyShamanMagicPageThree{
        "subject.shaman_magic.page_2", 3U};
constexpr RecruitedRunnerInformationTextStream
    kRickyLocalAreaPageOne{
        "subject.local_area.page_0", 3U};
constexpr RecruitedRunnerInformationTextStream
    kRickyLocalAreaPageTwo{
        "subject.local_area.page_1", 2U};
constexpr RecruitedRunnerInformationTextStream
    kRickyJohnsonGundersonPage{
        "subject.johnson_gunderson.page_0", 3U};

constexpr RecruitedRunnerInformationTextStream
    kWinstonHalloweenersPage{
        "subject.halloweeners.page_0", 3U};
constexpr RecruitedRunnerInformationTextStream
    kWinstonLoneStarPage{
        "subject.lone_star.page_0", 4U};
constexpr RecruitedRunnerInformationTextStream
    kWinstonTrollOriginPageOne{
        "subject.troll_origin.page_0", 2U};
constexpr RecruitedRunnerInformationTextStream
    kWinstonTrollOriginPageTwo{
        "subject.troll_origin.page_1", 4U};

constexpr RecruitedRunnerInformationTextStream kTrentMagesPageOne{
    "subject.mages.page_0", 2U};
constexpr RecruitedRunnerInformationTextStream kTrentMagesPageTwo{
    "subject.mages.page_1", 1U};
constexpr RecruitedRunnerInformationTextStream kTrentJeffersonContactPage{
    "subject.contacts.page_0", 3U};
constexpr RecruitedRunnerInformationTextStream kTrentJohnsonPage{
    "subject.johnsons.page_0", 3U};

constexpr std::uint32_t kTrentJeffersonContactStoryFlag =
    std::uint32_t{1U} << 12U;

bool service_slot_is_active(const LiveRunnerRecord& record) {
    // BTST #0,FF0210/FF0310 reads the first byte of each record's +0x10
    // word.  Do not reinterpret this as the low bit of a host-endian word.
    return (record.bytes[kServiceFlagByteOffset] & kServiceActiveBit) != 0U;
}

void write_big_endian_word(LiveRunnerRecord& record,
                           const std::size_t offset,
                           const std::uint16_t value) {
    record.bytes[offset] = static_cast<std::uint8_t>(value >> 8U);
    record.bytes[offset + 1U] =
        static_cast<std::uint8_t>(value & 0xFFU);
}

RunnerRosterTemplateResult initialize_selected_runner_slot(
    RecruitedRunnerServiceState& state,
    const std::uint8_t population_byte) {
    // ROM 0x42E6 tests FF0210.  A set bit selects the secondary live runner;
    // otherwise the new service target becomes the primary live runner.
    if (service_slot_is_active(state.roster.primary_record)) {
        return initialize_secondary_roster_template(
            state.selected_runner_template_index, state.templates,
            population_byte, state.roster, state.compact_records);
    }
    return initialize_primary_roster_template(
        state.selected_runner_template_index, state.templates, population_byte,
        state.roster, state.compact_records);
}

LiveRunnerRecord& selected_destination_record(
    RecruitedRunnerServiceState& state,
    const RunnerRecordDestination destination) {
    switch (destination) {
    case RunnerRecordDestination::primary_record:
        return state.roster.primary_record;
    case RunnerRecordDestination::secondary_record:
        return state.roster.secondary_record;
    case RunnerRecordDestination::default_record:
    default:
        return state.roster.default_record;
    }
}

} // namespace

RecruitedRunnerServiceOfferPreparationResult
prepare_recruited_runner_service_offer(
    RecruitedRunnerServiceState& state,
    const RecruitedRunnerServiceCostModifiers& modifiers) {
    RecruitedRunnerServiceOfferPreparationResult result{};

    if (recruited_runner_service_availability(state.roster) ==
        RecruitedRunnerServiceAvailability::roster_full) {
        // The source returns A0 = FF988 here. Its first longword points to
        // FFAFF ("Looks like you already got your hands full, pal.") and its
        // next longword is the no-option sentinel, so this is a real
        // title-only prompt rather than an opaque text pointer.
        result.action =
            RecruitedRunnerServiceOfferPreparationAction::roster_full;
        result.prompt = RecruitedRunnerServicePrompt::roster_full;
        return result;
    }

    const auto runner_index = static_cast<std::size_t>(
        state.selected_runner_template_index);
    if (runner_index >= kRecruitedRunnerServiceFeeBases.size()) {
        result.action = RecruitedRunnerServiceOfferPreparationAction::
            runner_index_unavailable;
        return result;
    }
    if (runner_index >= state.service_progress_counters.size()) {
        result.action = RecruitedRunnerServiceOfferPreparationAction::
            progress_counter_unavailable;
        return result;
    }

    const auto progress = static_cast<std::size_t>(
        state.service_progress_counters[runner_index]);
    const auto multiplier_index = progress + 1U;
    if (multiplier_index >= kServiceProgressMultipliers.size()) {
        result.action = RecruitedRunnerServiceOfferPreparationAction::
            progress_counter_unavailable;
        return result;
    }

    const auto multiplier = kServiceProgressMultipliers[multiplier_index];
    result.progress_multiplier = multiplier;
    const auto& bases = kRecruitedRunnerServiceFeeBases[runner_index];

    // 0x2128E..0x212A2: each fee base is multiplied by the progress
    // multiplier, divided by ten, then the first result is divided by 100.
    const auto scaled_short_term = divide_unsigned_word(
        static_cast<std::uint32_t>(bases.short_term) * multiplier, 10U);
    const auto discount_percent = divide_unsigned_word(scaled_short_term, 100U);

    // 0x212AA..0x212C2 uses MOVE.B/ADD.B, so the combined modifier wraps at
    // eight bits before the long multiply. FF017A is intentionally counted
    // twice because the source loads it and then adds it again.
    const auto modifier_sum = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(modifiers.source_byte_017A) * 2U +
        modifiers.source_byte_018B + modifiers.source_byte_018C);
    // E448 is LSR.W #2,D0, followed by MULU.W #5,D0: floor(sum / 4) * 5.
    const auto normalized_modifier =
        (static_cast<std::uint32_t>(modifier_sum) >> 2U) * 5U;
    const auto discount = normalized_modifier * discount_percent;

    // MULU.W uses the low word of the destination operand. The valid source
    // fee range stays below that boundary, but preserve the source operation
    // explicitly for malformed/extracted host records too.
    const auto source_word_discount =
        static_cast<std::uint32_t>(static_cast<std::uint16_t>(discount));
    result.short_term_fee = scaled_short_term - source_word_discount;

    const auto scaled_permanent = divide_unsigned_word(
        static_cast<std::uint32_t>(bases.permanent) * multiplier, 10U);
    result.permanent_fee =
        scaled_permanent - source_word_discount * 10U;

    state.short_term_fee = result.short_term_fee;
    state.permanent_fee = result.permanent_fee;
    result.action =
        RecruitedRunnerServiceOfferPreparationAction::prepared;
    return result;
}

RecruitedRunnerConversationPromptResult
select_recruited_runner_conversation_prompt(
    const RecruitedRunnerServiceState& state) {
    RecruitedRunnerConversationPromptResult result{};
    const auto runner_index = static_cast<std::size_t>(
        state.selected_runner_template_index);
    if (runner_index == kStarkTemplateIndex) {
        result.action =
            RecruitedRunnerConversationAction::story_runner_special_case;
        result.runner = RecruitedRunnerConversationRunner::stark;
        return result;
    }

    const auto runner = generic_conversation_runner(runner_index);
    if (!runner.has_value()) {
        result.action =
            RecruitedRunnerConversationAction::runner_index_unavailable;
        return result;
    }
    if (runner_index >= state.service_progress_counters.size()) {
        result.action =
            RecruitedRunnerConversationAction::progress_counter_unavailable;
        result.runner = *runner;
        return result;
    }

    result.action = RecruitedRunnerConversationAction::selected;
    result.runner = *runner;
    const auto signed_progress = static_cast<std::int8_t>(
        state.service_progress_counters[runner_index]);
    if (signed_progress > 0) {
        result.tone = RecruitedRunnerConversationTone::receptive;
    } else if (signed_progress < 0) {
        result.tone = RecruitedRunnerConversationTone::hostile;
    }
    return result;
}

RecruitedRunnerInformationPromptResult
select_recruited_runner_information_prompt(
    RecruitedRunnerServiceState& state) {
    RecruitedRunnerInformationPromptResult result{};
    const auto runner_index = static_cast<std::size_t>(
        state.selected_runner_template_index);
    if (runner_index >= kRecruitedRunnerInformationRecords.size()) {
        return result;
    }

    const auto& record = kRecruitedRunnerInformationRecords[runner_index];
    result.action = RecruitedRunnerInformationAction::selected;
    result.runner = record.runner;
    result.topic = record.topic;
    result.changed_flag = static_cast<std::uint16_t>(
        record.flag & static_cast<std::uint16_t>(
                          ~state.runner_information_flags));
    state.runner_information_flags = static_cast<std::uint16_t>(
        state.runner_information_flags | record.flag);
    result.information_flags = state.runner_information_flags;
    result.opening_text = record.opening_text;
    result.continuation_text = record.continuation_text;
    result.more_content_id = kMoreOptionContentId;
    return result;
}

RickyInformationSubjectPrompt ricky_information_subject_prompt() {
    return {"subject.title",
            {"subject.option.shaman_magic",
             "subject.option.local_area",
             "subject.option.johnson"}};
}

RickyInformationResponse select_ricky_information_response(
    const RickyInformationSubject subject) {
    RickyInformationResponse result{};
    result.subject = subject;
    result.returns_to_common_conversation = true;
    switch (subject) {
    case RickyInformationSubject::shaman_magic:
        result.action = RickyInformationResponseAction::selected;
        result.pages = {kRickyShamanMagicPageOne,
                        kRickyShamanMagicPageTwo,
                        kRickyShamanMagicPageThree};
        result.more_after_page = {true, true, false};
        result.page_count = 3U;
        return result;
    case RickyInformationSubject::local_area:
        result.action = RickyInformationResponseAction::selected;
        result.pages = {kRickyLocalAreaPageOne, kRickyLocalAreaPageTwo, {}};
        result.more_after_page = {true, false, false};
        result.page_count = 2U;
        return result;
    case RickyInformationSubject::johnson_gunderson:
        result.action = RickyInformationResponseAction::selected;
        result.pages = {kRickyJohnsonGundersonPage, {}, {}};
        result.more_after_page = {false, false, false};
        result.page_count = 1U;
        return result;
    }
    return result;
}

WinstonInformationSubjectPrompt winston_information_subject_prompt() {
    return {"subject.title",
            {"subject.option.halloweeners",
             "subject.option.lone_star",
             "subject.option.troll_origin"}};
}

WinstonInformationResponse select_winston_information_response(
    const WinstonInformationSubject subject) {
    WinstonInformationResponse result{};
    result.subject = subject;
    result.returns_to_common_conversation = true;
    switch (subject) {
    case WinstonInformationSubject::halloweeners:
        result.action = WinstonInformationResponseAction::selected;
        result.pages = {kWinstonHalloweenersPage, {}};
        result.more_after_page = {false, false};
        result.page_count = 1U;
        return result;
    case WinstonInformationSubject::lone_star:
        result.action = WinstonInformationResponseAction::selected;
        result.pages = {kWinstonLoneStarPage, {}};
        result.more_after_page = {false, false};
        result.page_count = 1U;
        return result;
    case WinstonInformationSubject::troll_origin:
        result.action = WinstonInformationResponseAction::selected;
        result.pages = {kWinstonTrollOriginPageOne,
                        kWinstonTrollOriginPageTwo};
        result.more_after_page = {true, false};
        result.page_count = 2U;
        return result;
    }
    return result;
}

TrentInformationSubjectPrompt trent_information_subject_prompt() {
    return {"subject.title",
            {"subject.option.mages",
             "subject.option.contacts",
             "subject.option.johnsons"}};
}

TrentInformationResponse select_trent_information_response(
    const TrentInformationSubject subject,
    RecruitedRunnerServiceState& state) {
    TrentInformationResponse result{};
    result.subject = subject;
    result.returns_to_common_conversation = true;
    switch (subject) {
    case TrentInformationSubject::mages:
        result.action = TrentInformationResponseAction::selected;
        result.pages = {kTrentMagesPageOne, kTrentMagesPageTwo};
        result.more_after_page = {true, false};
        result.page_count = 2U;
        return result;
    case TrentInformationSubject::contacts:
        // ROM 0x24964: MOVE.W FFFC2C,D1; BSET #12,D1; MOVE.W D1,FFFC2C;
        // LEA 0xFECC4,A0; RTS. The text at 0x100DBE describes the
        // Jefferson contact. The source later exposes the name through the
        // Pocket Secretary path; this function converts the proven flag
        // mutation and leaves that downstream record consumer explicit.
        state.story_flags |= kTrentJeffersonContactStoryFlag;
        result.action = TrentInformationResponseAction::selected;
        result.pages = {kTrentJeffersonContactPage, {}};
        result.more_after_page = {false, false};
        result.page_count = 1U;
        return result;
    case TrentInformationSubject::johnsons:
        result.action = TrentInformationResponseAction::selected;
        result.pages = {kTrentJohnsonPage, {}};
        result.more_after_page = {false, false};
        result.page_count = 1U;
        return result;
    }
    return result;
}

RecruitedRunnerServiceAvailability recruited_runner_service_availability(
    const RunnerRosterState& roster) {
    // ROM 0x21226 continues when either bit is clear, and returns its special
    // no-capacity text only when both primary and secondary bits are set.
    return service_slot_is_active(roster.primary_record) &&
                   service_slot_is_active(roster.secondary_record)
               ? RecruitedRunnerServiceAvailability::roster_full
               : RecruitedRunnerServiceAvailability::available;
}

RecruitedRunnerServiceResult resolve_recruited_runner_service_payment(
    RecruitedRunnerServiceState& state,
    const RecruitedRunnerServiceOffer offer,
    const RecruitedRunnerServiceCallbacks& callbacks) {
    RecruitedRunnerServiceResult result{};

    if (!state.selected_entity.present) {
        // FFE128 is established by the field interaction target-selection
        // path.  A missing target is an unresolved caller input, not a new
        // gameplay rejection invented by the conversion.
        result.action = RecruitedRunnerServiceAction::selected_entity_missing;
        return result;
    }

    std::uint32_t charge = 0;
    std::uint8_t population_byte = 0;
    switch (offer) {
    case RecruitedRunnerServiceOffer::short_term_run:
        charge = state.short_term_fee;
        population_byte = 0U;
        break;
    case RecruitedRunnerServiceOffer::permanent_hire:
        charge = state.permanent_fee;
        population_byte = kPermanentPopulationByte;
        break;
    case RecruitedRunnerServiceOffer::free_permanent_hire:
        // ROM 0x21318 writes FFEF36 = 0 before taking the permanent branch.
        state.permanent_fee = 0U;
        charge = 0U;
        population_byte = kPermanentPopulationByte;
        break;
    }

    // CMP.L fee,FFFBFE followed by BLE at 0x21342 accepts exact funds and
    // rejects only when the charge exceeds the current balance.
    if (charge > state.funds) {
        result.action = RecruitedRunnerServiceAction::insufficient_funds;
        result.prompt = RecruitedRunnerServicePrompt::insufficient_funds;
        result.charged_amount = charge;
        return result;
    }

    state.funds -= charge;
    result.charged_amount = charge;

    const auto roster = initialize_selected_runner_slot(state, population_byte);
    result.roster_action = roster.action;
    result.population_action = roster.population.action;
    result.destination = roster.population.last_destination;
    if (roster.action == RunnerRosterTemplateAction::invalid_template_index) {
        result.action = RecruitedRunnerServiceAction::invalid_runner_template;
        return result;
    }
    if (roster.population.action ==
        RunnerRecordPopulationAction::source_record_unavailable) {
        result.action =
            RecruitedRunnerServiceAction::runner_record_source_unavailable;
        return result;
    }

    if (result.destination == RunnerRecordDestination::default_record) {
        // The source 0x42E6 call always installs a primary or secondary live
        // record. Keep this explicit rather than silently copying into the
        // default record if a malformed host roster reaches this boundary.
        result.action =
            RecruitedRunnerServiceAction::runner_record_source_unavailable;
        return result;
    }

    if (result.destination == RunnerRecordDestination::primary_record) {
        state.timers.primary_delay = 10U;
        state.timers.primary_behavior = 10U;
    } else {
        state.timers.secondary_delay = 10U;
        state.timers.secondary_behavior = 10U;
    }

    // 0x2138E -> 0x38FA. The visual animation service runs before the
    // selected entity's active bit is cleared and its position is copied.
    if (callbacks.run_entity_animation_triplet) {
        callbacks.run_entity_animation_triplet();
    }

    state.selected_entity.service_flags = static_cast<std::uint8_t>(
        state.selected_entity.service_flags & ~kServiceActiveBit);
    auto& destination = selected_destination_record(state, result.destination);
    // 0x213A2 copies the selected field entity's +0x04 word into the newly
    // initialized live runner record's +0x04 word. The source uses a 68k
    // big-endian word; the host model keeps that serialization explicit until
    // the full live-record position schema is proven.
    write_big_endian_word(destination, kSourcePositionYOffset,
                          state.selected_entity.source_position_y);

    // 0x213A8 -> 0x3404. Rendering/upload remains outside this session model,
    // but its source-order call is observable through the typed callback.
    if (callbacks.upload_field_asset_setup) {
        callbacks.upload_field_asset_setup();
    }

    result.action = RecruitedRunnerServiceAction::completed;
    result.prompt =
        state.selected_runner_template_index == kStarkTemplateIndex
            ? RecruitedRunnerServicePrompt::stark_avenging_michael_followup
            : RecruitedRunnerServicePrompt::generic_runner_followup;
    return result;
}

} // namespace shadowrun::ghidra_conversion

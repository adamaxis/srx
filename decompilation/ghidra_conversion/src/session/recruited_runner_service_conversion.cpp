#include "shadowrun_ghidra/session/recruited_runner_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::size_t kServiceFlagByteOffset = 0x10U;
constexpr std::size_t kSourcePositionYOffset = 0x04U;
constexpr std::uint8_t kServiceActiveBit = 0x01U;
constexpr std::uint8_t kPermanentPopulationByte = 0xFFU;
constexpr std::uint8_t kStarkTemplateIndex = 9U;

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

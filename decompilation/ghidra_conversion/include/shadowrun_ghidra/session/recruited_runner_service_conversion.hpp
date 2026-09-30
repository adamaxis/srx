#pragma once

#include "shadowrun_ghidra/session/runner_roster_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

// The mode-command table routes these three commands to the same source body:
//   0x21326: pay the short-term single-run fee (D7 = 0)
//   0x21332: pay the permanent-hire fee (D7 = 0xFF)
//   0x21318: zero the permanent fee, then take the 0x21332 path
enum class RecruitedRunnerServiceOffer {
    short_term_run,
    permanent_hire,
    free_permanent_hire,
};

enum class RecruitedRunnerServiceAvailability {
    available,
    roster_full,
};

enum class RecruitedRunnerServiceOfferPreparationAction {
    prepared,
    roster_full,
    runner_index_unavailable,
    progress_counter_unavailable,
};

enum class RecruitedRunnerServicePrompt {
    none,
    insufficient_funds,
    roster_full,
    generic_runner_followup,
    stark_avenging_michael_followup,
};

enum class RecruitedRunnerServiceAction {
    completed,
    insufficient_funds,
    invalid_runner_template,
    selected_entity_missing,
    runner_record_source_unavailable,
};

enum class RecruitedRunnerConversationRunner {
    ricky,
    winston_marrs,
    trent,
    petr_uvehr,
    walking_bear,
    phantom,
    two_fists,
    freya,
    rianna,
    stark,
};

enum class RecruitedRunnerConversationTone {
    neutral,
    receptive,
    hostile,
};

enum class RecruitedRunnerConversationOption {
    ask_for_information,
    ask_to_hire_runner,
    leave_conversation,
};

enum class RecruitedRunnerConversationAction {
    selected,
    story_runner_special_case,
    runner_index_unavailable,
    progress_counter_unavailable,
};

struct RecruitedRunnerConversationPromptResult {
    RecruitedRunnerConversationAction action =
        RecruitedRunnerConversationAction::runner_index_unavailable;
    RecruitedRunnerConversationRunner runner =
        RecruitedRunnerConversationRunner::ricky;
    RecruitedRunnerConversationTone tone =
        RecruitedRunnerConversationTone::neutral;
    std::array<RecruitedRunnerConversationOption, 3> options{{
        RecruitedRunnerConversationOption::ask_for_information,
        RecruitedRunnerConversationOption::ask_to_hire_runner,
        RecruitedRunnerConversationOption::leave_conversation,
    }};
};

enum class RecruitedRunnerInformationAction {
    selected,
    runner_index_unavailable,
};

enum class RecruitedRunnerInformationTopic {
    ricky_glow_city_origin,
    winston_marrs_identity,
    trent_identity,
    petr_uvehr_cyberware_motivation,
    walking_bear_transformation,
    phantom_matrix_background,
    two_fists_ares_run,
    freya_cascade_crow_background,
    rianna_heartbane_identity,
    stark_amerindian_heritage,
};

// The mode-command entries 0xBC..0xC5 are ten copies of the same source
// operation with different bit numbers and text descriptors. The native
// conversion keeps those as one table-driven behavior: selecting a runner's
// information topic marks its knowledge bit and returns the two-page dialog
// content that the source exposes through its < MORE > continuation.
struct RecruitedRunnerInformationPromptResult {
    RecruitedRunnerInformationAction action =
        RecruitedRunnerInformationAction::runner_index_unavailable;
    RecruitedRunnerConversationRunner runner =
        RecruitedRunnerConversationRunner::ricky;
    RecruitedRunnerInformationTopic topic =
        RecruitedRunnerInformationTopic::ricky_glow_city_origin;
    std::uint16_t information_flags = 0; // source FFFC2A
    std::uint16_t changed_flag = 0;
    std::string_view opening_text;
    std::string_view continuation_text;
    std::string_view more_option_text;
};

// The command handlers use the selected field entity at FFE128.  The entity's
// +0x10 bit 0 is cleared after a successful hire, and its +0x04 position word
// is copied into the selected live runner record.  These are named host fields
// rather than a pointer into the Genesis work area.
struct RecruitedRunnerServiceTarget {
    bool present = false;
    std::uint8_t service_flags = 0;
    std::uint16_t source_position_y = 0;
};

// ROM 0x2138E seeds the selected slot's two ten-frame service meters.  The
// names describe the native state; the comments retain the exact source
// consumers for audit purposes.
struct RecruitedRunnerServiceTimers {
    std::uint8_t primary_delay = 0;     // FF024C
    std::uint8_t primary_behavior = 0;  // FF029C
    std::uint8_t secondary_delay = 0;   // FF034C
    std::uint8_t secondary_behavior = 0; // FF039C
};

// The offer-preparation routine at ROM 0x21226 computes these two fees before
// the command handlers below run. The native preparation function below now
// consumes its typed fee schedule, progress table, and source cost bytes;
// callers may still supply already-computed values when reproducing a later
// prompt state.
struct RecruitedRunnerServiceState {
    std::uint32_t funds = 0;             // FFFBFE
    std::uint32_t short_term_fee = 0;    // FFEF16
    std::uint32_t permanent_fee = 0;     // FFEF36
    std::uint8_t selected_runner_template_index = 0; // FFF0AE
    std::uint16_t runner_information_flags = 0; // FFFC2A

    RunnerRosterState roster{};
    std::vector<RunnerRosterTemplate> templates;
    std::vector<CompactRunnerRecord> compact_records;
    // FFFC12 onward. The selected runner index addresses this table directly
    // while the field refresh path addresses it through its compact marker.
    std::vector<std::uint8_t> service_progress_counters;
    RecruitedRunnerServiceTarget selected_entity{};
    RecruitedRunnerServiceTimers timers{};
};

// These are the three source bytes combined by 0x212AA..0x212C8. Their
// gameplay names are not proven by another consumer yet, so the native API
// records their exact source identity instead of inventing stat names.
struct RecruitedRunnerServiceCostModifiers {
    std::uint8_t source_byte_017A = 0; // FF017A
    std::uint8_t source_byte_018B = 0; // FF018B
    std::uint8_t source_byte_018C = 0; // FF018C
};

struct RecruitedRunnerServiceOfferPreparationResult {
    RecruitedRunnerServiceOfferPreparationAction action =
        RecruitedRunnerServiceOfferPreparationAction::runner_index_unavailable;
    RecruitedRunnerServicePrompt prompt = RecruitedRunnerServicePrompt::none;
    std::uint8_t progress_multiplier = 0;
    std::uint32_t short_term_fee = 0;
    std::uint32_t permanent_fee = 0;
};

// 0x38FA and 0x3404 are presentation/animation boundaries.  They remain
// callbacks so this session conversion can preserve their source order
// without pulling field rendering into the roster model.
struct RecruitedRunnerServiceCallbacks {
    std::function<void()> run_entity_animation_triplet;
    std::function<void()> upload_field_asset_setup;
};

struct RecruitedRunnerServiceResult {
    RecruitedRunnerServiceAction action =
        RecruitedRunnerServiceAction::selected_entity_missing;
    RecruitedRunnerServicePrompt prompt =
        RecruitedRunnerServicePrompt::none;
    std::uint32_t charged_amount = 0;
    RunnerRecordDestination destination = RunnerRecordDestination::default_record;
    RunnerRosterTemplateAction roster_action =
        RunnerRosterTemplateAction::invalid_template_index;
    RunnerRecordPopulationAction population_action =
        RunnerRecordPopulationAction::no_source_records;
};

// ROM 0x21226 tests the service-active bit in both live recruited-runner
// records before presenting the offer. The no-capacity descriptor at 0xFF988
// has a title but no selectable options, and is returned as roster_full.
RecruitedRunnerServiceAvailability recruited_runner_service_availability(
    const RunnerRosterState& roster);

// ROM 0x21226..0x21316. This consumes the already extracted fee schedule at
// 0xFE640, the source progress counter for the selected runner, and the three
// cost-modifier bytes. It publishes the two host fee fields used by the three
// dispatch entries below. The text-render calls at 0xC0DA/0xC216 remain
// presentation boundaries; the numeric fee arithmetic is converted here.
RecruitedRunnerServiceOfferPreparationResult
prepare_recruited_runner_service_offer(
    RecruitedRunnerServiceState& state,
    const RecruitedRunnerServiceCostModifiers& modifiers);

// ROM 0x211E8 (mode-command table entry 0x04). The nine indexed triples at
// 0xFE6E0 select the runner-specific opening response by the signed byte in
// FFFC12 + selected runner index: zero, positive, or negative. The bytes
// immediately after that table are a separate special root, not a tenth
// triple; index 9 therefore remains an explicit Stark/story boundary here.
RecruitedRunnerConversationPromptResult
select_recruited_runner_conversation_prompt(
    const RecruitedRunnerServiceState& state);

// ROM 0x2401C..0x24102, mode-command entries 0xBC..0xC5. Each entry sets
// one runner-specific bit in FFFC2A and returns a title-only first page with
// the shared < MORE > option; the continuation page then presents that
// runner's second biographical paragraph before returning to the subject
// choices. The source text is represented as native text data, not ROM
// pointer arithmetic.
RecruitedRunnerInformationPromptResult
select_recruited_runner_information_prompt(
    RecruitedRunnerServiceState& state);

// ROM 0x21318/0x21326/0x21332 plus the common body at 0x2133C.  This converts
// the recruited-runner dialog's short-term, permanent, and free-permanent
// choices into typed state mutations.  The source's selected index 9 is the
// extracted Stark record, whose success descriptor is the Michael-avenging
// follow-up rather than the generic hired-runner response.
RecruitedRunnerServiceResult resolve_recruited_runner_service_payment(
    RecruitedRunnerServiceState& state,
    RecruitedRunnerServiceOffer offer,
    const RecruitedRunnerServiceCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion

#include "dialogue_panel.hpp"

#include <cstdint>
#include <string_view>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;
using namespace shadowrun::ghidra_conversion::qt_runtime;

RecruitedRunnerConversationState make_state(
    const std::uint8_t runner_index) {
    RecruitedRunnerConversationState state{};
    state.service.selected_runner_template_index = runner_index;
    state.service.templates.resize(10U);
    state.service.compact_records.resize(13U);
    state.service.service_progress_counters.resize(10U);
    state.service.funds = 1000U;
    state.service.selected_entity.present = true;
    state.service.selected_entity.service_flags = 0x01U;
    state.service.selected_entity.source_position_y = 0x3456U;
    state.service.short_term_fee = 250U;
    state.service.permanent_fee = 700U;
    return state;
}

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    auto state = make_state(0U);
    auto source_view = begin_recruited_runner_conversation(state);
    auto panel = make_dialogue_panel_view(source_view);
    if (!expect(panel.open) ||
        !expect(panel.kind == DialoguePanelKind::common_conversation_choices) ||
        !expect(panel.actor_name == "Ricky") ||
        !expect(panel.portrait_alias.empty()) ||
        !expect(panel.option_count == 3U) ||
        !expect(panel.options[0].role == DialoguePanelOptionRole::information) ||
        !expect(panel.options[1].role == DialoguePanelOptionRole::hire_runner) ||
        !expect(panel.options[2].role ==
                DialoguePanelOptionRole::leave_conversation) ||
        !expect(!dialogue_panel_option_command(panel, 3U).has_value()) ||
        !expect(dialogue_panel_option_command(panel, 1U).value() ==
                RecruitedRunnerConversationCommand::ask_to_hire_runner)) {
        return 1;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::ask_for_information);
    panel = make_dialogue_panel_view(source_view);
    if (!expect(panel.kind == DialoguePanelKind::information_page) ||
        !expect(panel.text_segment_count == 3U) ||
        !expect(panel.text_segments[0].find("Glow City") !=
                std::string_view::npos) ||
        !expect(panel.has_more) ||
        !expect(panel.continuation_label == "< MORE >") ||
        !expect(!panel.returns_to_conversation)) {
        return 2;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::continue_page);
    panel = make_dialogue_panel_view(source_view);
    if (!expect(panel.kind == DialoguePanelKind::information_page) ||
        !expect(panel.page_index == 1U) ||
        !expect(panel.text_segments[0].find("Then I heard") !=
                std::string_view::npos) ||
        !expect(!panel.has_more) ||
        !expect(panel.returns_to_conversation)) {
        return 3;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::continue_page);
    panel = make_dialogue_panel_view(source_view);
    if (!expect(panel.kind == DialoguePanelKind::subject_choices) ||
        !expect(panel.text_segments[0] == "Like what?") ||
        !expect(panel.options[0].source_label ==
                "Tell me about shaman magic.") ||
        !expect(dialogue_panel_option_command(panel, 2U).value() ==
                RecruitedRunnerConversationCommand::select_third_subject)) {
        return 4;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::select_first_subject);
    panel = make_dialogue_panel_view(source_view);
    if (!expect(panel.kind == DialoguePanelKind::information_response_page) ||
        !expect(panel.text_segments[0].find("Shaman") !=
                std::string_view::npos) ||
        !expect(panel.has_more) || !expect(panel.page_index == 0U)) {
        return 5;
    }

    auto unresolved = make_state(3U);
    source_view = begin_recruited_runner_conversation(unresolved);
    source_view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::ask_for_information);
    source_view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::continue_page);
    source_view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::continue_page);
    panel = make_dialogue_panel_view(source_view);
    if (!expect(panel.open) ||
        !expect(panel.kind == DialoguePanelKind::source_boundary) ||
        !expect(!panel.source_complete) ||
        !expect(panel.source_boundary_reason.find("not yet source-converted") !=
                std::string_view::npos) ||
        !expect(panel.option_count == 0U)) {
        return 6;
    }

    auto roster_full = make_state(0U);
    roster_full.service.roster.primary_record.bytes[0x10U] = 0x01U;
    roster_full.service.roster.secondary_record.bytes[0x10U] = 0x01U;
    source_view = begin_recruited_runner_conversation(roster_full);
    source_view = dispatch_recruited_runner_conversation(
        roster_full, RecruitedRunnerConversationCommand::ask_to_hire_runner);
    panel = make_dialogue_panel_view(source_view);
    if (!expect(panel.kind == DialoguePanelKind::service_offer) ||
        !expect(panel.option_count == 0U)) {
        return 7;
    }

    const auto hire_source = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::back_to_common_prompt);
    (void)hire_source;
    if (!expect(dialogue_panel_portrait_alias(
                    FieldExitResourceProfile::stoker_motel) ==
                "dialogue.portraits.stoker.motel") ||
        !expect(dialogue_panel_portrait_alias(
                    FieldExitResourceProfile::scene_zero_submode_three)
                    .empty())) {
        return 8;
    }

    return 0;
}

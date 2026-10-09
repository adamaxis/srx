#include "dialogue_panel.hpp"
#include "dialogue_panel_presenter.hpp"
#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <QImage>
#include <QPainter>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

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

bool runner_text_catalog_is_complete(
    const scripting::GameScriptRuntime& scripts,
    const scripting::ScriptStringMap& runner_text_keys) {
    constexpr std::array<std::pair<std::string_view, std::size_t>, 56>
        kRunnerText{{
            {"runner.ricky.information.opening", 3U},
            {"runner.ricky.information.continuation", 3U},
            {"runner.winston_marrs.information.opening", 3U},
            {"runner.winston_marrs.information.continuation", 3U},
            {"runner.trent.information.opening", 4U},
            {"runner.trent.information.continuation", 1U},
            {"runner.petr_uvehr.information.opening", 2U},
            {"runner.petr_uvehr.information.continuation", 2U},
            {"runner.walking_bear.information.opening", 3U},
            {"runner.walking_bear.information.continuation", 2U},
            {"runner.phantom.information.opening", 3U},
            {"runner.phantom.information.continuation", 3U},
            {"runner.two_fists.information.opening", 2U},
            {"runner.two_fists.information.continuation", 1U},
            {"runner.freya.information.opening", 2U},
            {"runner.freya.information.continuation", 3U},
            {"runner.rianna.information.opening", 4U},
            {"runner.rianna.information.continuation", 3U},
            {"runner.stark.information.opening", 1U},
            {"runner.stark.information.continuation", 1U},
            {"runner.ricky.subject.title", 1U},
            {"runner.ricky.subject.option.shaman_magic", 1U},
            {"runner.ricky.subject.option.local_area", 1U},
            {"runner.ricky.subject.option.johnson", 1U},
            {"runner.winston_marrs.subject.title", 1U},
            {"runner.winston_marrs.subject.option.halloweeners", 1U},
            {"runner.winston_marrs.subject.option.lone_star", 1U},
            {"runner.winston_marrs.subject.option.troll_origin", 1U},
            {"runner.trent.subject.title", 1U},
            {"runner.trent.subject.option.mages", 1U},
            {"runner.trent.subject.option.contacts", 1U},
            {"runner.trent.subject.option.johnsons", 1U},
            {"runner.ricky.subject.shaman_magic.page_0", 3U},
            {"runner.ricky.subject.shaman_magic.page_1", 4U},
            {"runner.ricky.subject.shaman_magic.page_2", 3U},
            {"runner.ricky.subject.local_area.page_0", 3U},
            {"runner.ricky.subject.local_area.page_1", 2U},
            {"runner.ricky.subject.johnson_gunderson.page_0", 3U},
            {"runner.winston_marrs.subject.halloweeners.page_0", 3U},
            {"runner.winston_marrs.subject.lone_star.page_0", 4U},
            {"runner.winston_marrs.subject.troll_origin.page_0", 2U},
            {"runner.winston_marrs.subject.troll_origin.page_1", 4U},
            {"runner.trent.subject.mages.page_0", 2U},
            {"runner.trent.subject.mages.page_1", 1U},
            {"runner.trent.subject.contacts.page_0", 3U},
            {"runner.trent.subject.johnsons.page_0", 3U},
            {"runner.ricky.name", 1U},
            {"runner.winston_marrs.name", 1U},
            {"runner.trent.name", 1U},
            {"runner.petr_uvehr.name", 1U},
            {"runner.walking_bear.name", 1U},
            {"runner.phantom.name", 1U},
            {"runner.two_fists.name", 1U},
            {"runner.freya.name", 1U},
            {"runner.rianna.name", 1U},
            {"runner.stark.name", 1U},
        }};

    if (runner_text_keys.size() != 57U ||
        runner_text_keys.find("shared.more") == runner_text_keys.end() ||
        runner_text_keys.at("shared.more") != "stoker.motel.arrival.more") {
        std::cerr << "runner Lua content-key catalog is incomplete\n";
        return false;
    }
    for (const auto& [key, expected_rows] : kRunnerText) {
        const auto* text = scripts.dialogue_text(key);
        if (text == nullptr || text->empty()) {
            std::cerr << "runner Lua text key is missing: " << key << '\n';
            return false;
        }
        const auto actual_rows = static_cast<std::size_t>(
            std::count(text->begin(), text->end(), '|') + 1);
        if (actual_rows != expected_rows) {
            std::cerr << "runner Lua text row count differs for " << key
                      << ": expected " << expected_rows << ", got "
                      << actual_rows << '\n';
            return false;
        }
        const auto binding = std::find_if(
            runner_text_keys.begin(), runner_text_keys.end(),
            [key](const auto& entry) { return entry.second == key; });
        if (binding == runner_text_keys.end()) {
            std::cerr << "runner Lua content key does not bind " << key
                      << '\n';
            return false;
        }
    }
    constexpr std::array<std::pair<std::string_view, std::string_view>, 10>
        kRunnerNames{{
            {"runner.ricky.name", "Ricky"},
            {"runner.winston_marrs.name", "Winston Marrs"},
            {"runner.trent.name", "Trent"},
            {"runner.petr_uvehr.name", "Petr Uvehr"},
            {"runner.walking_bear.name", "Walking Bear"},
            {"runner.phantom.name", "Phantom"},
            {"runner.two_fists.name", "Two Fists"},
            {"runner.freya.name", "Freya"},
            {"runner.rianna.name", "Rianna"},
            {"runner.stark.name", "Stark"},
        }};
    constexpr std::array<RecruitedRunnerConversationRunner, 10>
        kRunnerOrder{{
            RecruitedRunnerConversationRunner::ricky,
            RecruitedRunnerConversationRunner::winston_marrs,
            RecruitedRunnerConversationRunner::trent,
            RecruitedRunnerConversationRunner::petr_uvehr,
            RecruitedRunnerConversationRunner::walking_bear,
            RecruitedRunnerConversationRunner::phantom,
            RecruitedRunnerConversationRunner::two_fists,
            RecruitedRunnerConversationRunner::freya,
            RecruitedRunnerConversationRunner::rianna,
            RecruitedRunnerConversationRunner::stark,
        }};
    for (std::size_t index = 0; index < kRunnerNames.size(); ++index) {
        const auto [key, expected_name] = kRunnerNames[index];
        const auto* actual_name = scripts.dialogue_text(key);
        const auto runner_id = dialogue_panel_runner_id(kRunnerOrder[index]);
        const auto name_key = runner_text_keys.find(
            std::string(runner_id) + ".name");
        if (actual_name == nullptr || *actual_name != expected_name ||
            name_key == runner_text_keys.end() || name_key->second != key) {
            std::cerr << "runner name key/value mismatch for " << key << '\n';
            return false;
        }
    }
    return true;
}

bool panel_matches_text(const DialoguePanelView& panel,
                        const std::string_view text) {
    std::size_t actual_index = 0U;
    std::size_t expected_start = 0U;
    while (expected_start <= text.size()) {
        const auto separator = text.find('|', expected_start);
        const auto expected_end = separator == std::string_view::npos
                                      ? text.size()
                                      : separator;
        if (actual_index >= panel.text_segment_count ||
            panel.text_segments[actual_index] !=
                text.substr(expected_start, expected_end - expected_start)) {
            return false;
        }
        ++actual_index;
        if (separator == std::string_view::npos) {
            break;
        }
        expected_start = separator + 1U;
    }
    return actual_index == panel.text_segment_count;
}

bool save_dialogue_capture(const DialoguePanelPresenter& presenter,
                           const DialoguePanelView& panel,
                           const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    QImage frame(320, 224, QImage::Format_ARGB32_Premultiplied);
    frame.fill(Qt::black);
    QPainter painter(&frame);
    presenter.paint(painter, panel);
    painter.end();
    return frame.save(QString::fromStdWString(path.wstring()), "BMP");
}

} // namespace

int main(const int argc, char** argv) {
    const bool capture_requested = argc == 3 &&
                                  std::string_view(argv[1]) == "--capture-dir";
    if (argc != 1 && !capture_requested) {
        return 14;
    }
    const std::filesystem::path capture_dir =
        capture_requested ? std::filesystem::path(argv[2])
                          : std::filesystem::path{};

    scripting::GameScriptRuntime scripts;
    if (!scripts.load_package(
            SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
            std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                "dialogue" / "script-text.lua")) {
        return 10;
    }
    const auto stoker_profile = scripts.speaker_profile("stoker");
    if (!expect(stoker_profile.has_value()) ||
        !expect(stoker_profile->id == "stoker") ||
        !expect(stoker_profile->name == "Stoker") ||
        !expect(stoker_profile->text_profile == "stoker.motel") ||
        !expect(stoker_profile->portrait_alias ==
                "dialogue.portraits.stoker.motel")) {
        return 20;
    }
    const DialogueSpeakerPresentation stoker_speaker{
        stoker_profile->id, stoker_profile->name,
        stoker_profile->text_profile, stoker_profile->portrait_alias};
    const auto runner_text_keys =
        scripts.string_map("dialogue", "runner_text_keys");
    if (!runner_text_keys.has_value() ||
        !runner_text_catalog_is_complete(scripts, *runner_text_keys)) {
        return 13;
    }
    const auto loaded_assets = load_conversion_asset_catalog(
        SHADOWRUN_GHIDRA_ASSET_ROOT);
    if (!expect(loaded_assets.complete())) {
        return 9;
    }
    DialoguePanelPresenter presenter;
    if (!presenter.load_assets(loaded_assets.catalog)) {
        return 15;
    }
    const auto capture = [&capture_requested, &capture_dir, &presenter](
                             const std::string_view filename,
                             const DialoguePanelView& view) {
        return !capture_requested ||
               save_dialogue_capture(presenter, view,
                                     capture_dir / filename);
    };
    const DialogueTextResolver resolve_text = [&scripts](
                                                  const std::string_view key) {
        const auto* value = scripts.dialogue_text(key);
        return value == nullptr ? std::string_view{} : std::string_view(*value);
    };
    const DialogueContentKeyResolver resolve_runner_content_key =
        [&runner_text_keys](const std::string_view speaker_id,
                           const std::string_view content_id) {
            std::string binding_id;
            if (!speaker_id.empty()) {
                binding_id.assign(speaker_id);
                binding_id.push_back('.');
            }
            binding_id.append(content_id);
            const auto binding = runner_text_keys->find(binding_id);
            return binding == runner_text_keys->end()
                       ? std::string_view{}
                       : std::string_view(binding->second);
        };

    constexpr std::array<std::string_view, 9> kGenericRunnerIds{{
        "ricky", "winston_marrs", "trent", "petr_uvehr", "walking_bear",
        "phantom", "two_fists", "freya", "rianna"}};
    constexpr std::array<std::string_view, 9> kGenericRunnerNames{{
        "Ricky", "Winston Marrs", "Trent", "Petr Uvehr", "Walking Bear",
        "Phantom", "Two Fists", "Freya", "Rianna"}};
    for (std::size_t index = 0; index < kGenericRunnerIds.size(); ++index) {
        auto runner_state = make_state(static_cast<std::uint8_t>(index));
        auto runner_view = begin_recruited_runner_conversation(runner_state);
        auto runner_panel = make_dialogue_panel_view(
            runner_view, resolve_text, resolve_runner_content_key);
        if (!expect(runner_panel.actor_name == kGenericRunnerNames[index]) ||
            !expect(dialogue_panel_runner_id(runner_view.runner) ==
                    kGenericRunnerIds[index])) {
            std::cerr << "Lua runner speaker binding differs for "
                      << kGenericRunnerIds[index] << '\n';
            return 21;
        }

        runner_view = dispatch_recruited_runner_conversation(
            runner_state,
            RecruitedRunnerConversationCommand::ask_for_information);
        runner_panel = make_dialogue_panel_view(
            runner_view, resolve_text, resolve_runner_content_key);
        const auto opening_binding = runner_text_keys->find(
            std::string(kGenericRunnerIds[index]) + ".information.opening");
        const auto* opening_text = opening_binding == runner_text_keys->end()
                                       ? nullptr
                                       : scripts.dialogue_text(
                                             opening_binding->second);
        if (opening_text == nullptr || !runner_panel.source_complete ||
            !panel_matches_text(runner_panel, *opening_text) ||
            !runner_panel.has_more) {
            std::cerr << "Lua runner opening page differs for "
                      << kGenericRunnerIds[index] << '\n';
            return 22;
        }
        const auto filename = std::string("runner-") +
                              std::string(kGenericRunnerIds[index]) +
                              "-information-opening.bmp";
        if (!capture(filename, runner_panel)) {
            return 23;
        }
    }

    auto state = make_state(0U);
    auto source_view = begin_recruited_runner_conversation(state);
    auto panel = make_dialogue_panel_view(
        source_view, resolve_text, resolve_runner_content_key);
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
                DialoguePanelCommand::runner_ask_to_hire)) {
        std::cerr << "initial runner panel mismatch: open=" << panel.open
                  << " kind=" << static_cast<int>(panel.kind)
                  << " actor=" << panel.actor_name
                  << " portrait=" << panel.portrait_alias
                  << " options=" << panel.option_count
                  << " roles=" << static_cast<int>(panel.options[0].role)
                  << ',' << static_cast<int>(panel.options[1].role)
                  << ',' << static_cast<int>(panel.options[2].role)
                  << " source_complete=" << panel.source_complete << '\n';
        return 1;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::ask_for_information);
    panel = make_dialogue_panel_view(
        source_view, resolve_text, resolve_runner_content_key);
    if (!expect(panel.kind == DialoguePanelKind::information_page) ||
        !expect(panel.text_segment_count == 3U) ||
        !expect(panel.text_segments[0].find("Glow City") !=
                std::string_view::npos) ||
        !expect(panel.has_more) ||
        !expect(panel.continuation_label == "          < MORE >") ||
        !expect(!panel.returns_to_conversation)) {
        std::cerr << "runner information panel mismatch: kind="
                  << static_cast<int>(panel.kind)
                  << " rows=" << panel.text_segment_count
                  << " first=" << panel.text_segments[0]
                  << " more=" << panel.has_more
                  << " continuation=[" << panel.continuation_label
                  << "] returns=" << panel.returns_to_conversation
                  << " complete=" << panel.source_complete << '\n';
        return 2;
    }
    if (!capture("actual.bmp", panel)) {
        return 16;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::continue_page);
    panel = make_dialogue_panel_view(
        source_view, resolve_text, resolve_runner_content_key);
    if (!expect(panel.kind == DialoguePanelKind::information_page) ||
        !expect(panel.page_index == 1U) ||
        !expect(panel.text_segments[0].find("Then I heard") !=
                std::string_view::npos) ||
        !expect(!panel.has_more) ||
        !expect(panel.returns_to_conversation)) {
        return 3;
    }
    if (!capture("frame-001-continuation.bmp", panel)) {
        return 17;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::continue_page);
    panel = make_dialogue_panel_view(
        source_view, resolve_text, resolve_runner_content_key);
    if (!expect(panel.kind == DialoguePanelKind::subject_choices) ||
        !expect(panel.text_segments[0] == "Like what?") ||
        !expect(panel.options[0].source_label ==
                "Tell me about shaman magic.") ||
        !expect(dialogue_panel_option_command(panel, 2U).value() ==
                DialoguePanelCommand::runner_select_third_subject)) {
        return 4;
    }
    if (!capture("frame-002-subject-menu.bmp", panel)) {
        return 18;
    }

    source_view = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::select_first_subject);
    panel = make_dialogue_panel_view(
        source_view, resolve_text, resolve_runner_content_key);
    if (!expect(panel.kind == DialoguePanelKind::information_response_page) ||
        !expect(panel.text_segments[0].find("Shaman") !=
                std::string_view::npos) ||
        !expect(panel.has_more) || !expect(panel.page_index == 0U)) {
        return 5;
    }
    if (!capture("frame-003-topic-page.bmp", panel)) {
        return 19;
    }

    constexpr std::array<RecruitedRunnerConversationCommand, 3>
        kSubjectCommands{{
            RecruitedRunnerConversationCommand::select_first_subject,
            RecruitedRunnerConversationCommand::select_second_subject,
            RecruitedRunnerConversationCommand::select_third_subject}};
    for (std::size_t runner_index = 0; runner_index < 3U; ++runner_index) {
        const auto speaker_id = kGenericRunnerIds[runner_index];
        for (std::size_t subject_index = 0; subject_index < 3U;
             ++subject_index) {
            auto topic_state = make_state(
                static_cast<std::uint8_t>(runner_index));
            auto topic_view = begin_recruited_runner_conversation(topic_state);
            topic_view = dispatch_recruited_runner_conversation(
                topic_state,
                RecruitedRunnerConversationCommand::ask_for_information);
            topic_view = dispatch_recruited_runner_conversation(
                topic_state, RecruitedRunnerConversationCommand::continue_page);
            topic_view = dispatch_recruited_runner_conversation(
                topic_state, RecruitedRunnerConversationCommand::continue_page);
            auto topic_panel = make_dialogue_panel_view(
                topic_view, resolve_text, resolve_runner_content_key);
            if (topic_panel.kind != DialoguePanelKind::subject_choices ||
                !topic_panel.source_complete || topic_panel.option_count != 3U) {
                std::cerr << "Lua topic menu binding differs for " << speaker_id
                          << '\n';
                return 24;
            }
            for (std::size_t option_index = 0; option_index < 3U;
                 ++option_index) {
                if (topic_panel.options[option_index].source_label.empty()) {
                    std::cerr << "Lua topic label is empty for " << speaker_id
                              << " option " << option_index << '\n';
                    return 25;
                }
            }
            const auto menu_filename = std::string("runner-") +
                std::string(speaker_id) + "-subject-menu.bmp";
            if (!capture(menu_filename, topic_panel)) {
                return 26;
            }

            topic_view = dispatch_recruited_runner_conversation(
                topic_state, kSubjectCommands[subject_index]);
            bool finished_pages = false;
            for (std::size_t page = 0; page < 3U; ++page) {
                topic_panel = make_dialogue_panel_view(
                    topic_view, resolve_text, resolve_runner_content_key);
                std::string_view content_id;
                std::visit(
                    [&topic_view, &content_id](const auto& response) {
                        using Response = std::decay_t<decltype(response)>;
                        if constexpr (!std::is_same_v<Response,
                                                     std::monostate>) {
                            if (topic_view.page_index < response.page_count) {
                                content_id = response.pages[
                                    topic_view.page_index].content_id;
                            }
                        }
                    },
                    topic_view.information_response);
                std::string binding_id(speaker_id);
                binding_id.push_back('.');
                binding_id.append(content_id);
                const auto binding = runner_text_keys->find(binding_id);
                const auto* expected_text = binding == runner_text_keys->end()
                    ? nullptr
                    : scripts.dialogue_text(binding->second);
                if (topic_panel.kind !=
                        DialoguePanelKind::information_response_page ||
                    !topic_panel.source_complete || expected_text == nullptr ||
                    !panel_matches_text(topic_panel, *expected_text)) {
                    std::cerr << "Lua topic response binding differs for "
                              << binding_id << '\n';
                    return 27;
                }
                const auto page_filename = std::string("runner-") +
                    std::string(speaker_id) + "-subject-" +
                    std::to_string(subject_index) + "-page-" +
                    std::to_string(page) + ".bmp";
                if (!capture(page_filename, topic_panel)) {
                    return 28;
                }
                if (!topic_panel.has_more) {
                    finished_pages = topic_panel.returns_to_conversation;
                    break;
                }
                topic_view = dispatch_recruited_runner_conversation(
                    topic_state,
                    RecruitedRunnerConversationCommand::continue_page);
            }
            if (!finished_pages) {
                std::cerr << "Lua topic response did not return for "
                          << speaker_id << " option " << subject_index << '\n';
                return 29;
            }
        }
    }

    auto unresolved = make_state(3U);
    source_view = begin_recruited_runner_conversation(unresolved);
    source_view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::ask_for_information);
    source_view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::continue_page);
    source_view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::continue_page);
    panel = make_dialogue_panel_view(
        source_view, resolve_text, resolve_runner_content_key);
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
    panel = make_dialogue_panel_view(
        source_view, resolve_text, resolve_runner_content_key);
    if (!expect(panel.kind == DialoguePanelKind::service_offer) ||
        !expect(panel.option_count == 0U)) {
        return 7;
    }

    const auto hire_source = dispatch_recruited_runner_conversation(
        state, RecruitedRunnerConversationCommand::back_to_common_prompt);
    (void)hire_source;
    const auto stoker_source = make_stoker_motel_prompt(
        StokerMotelPromptId::arrival);
    const auto stoker_panel = make_dialogue_panel_view(
        stoker_source, stoker_speaker, resolve_text);
    if (!expect(stoker_panel.open) ||
        !expect(stoker_panel.actor_kind == DialoguePanelActorKind::field_npc) ||
        !expect(stoker_panel.actor_name == "Stoker") ||
        !expect(stoker_panel.portrait_alias ==
                "dialogue.portraits.stoker.motel") ||
        !expect(stoker_panel.kind == DialoguePanelKind::field_npc_prompt) ||
        !expect(stoker_panel.text_segment_count == 5U) ||
        !expect(stoker_panel.text_segments[1].empty()) ||
        !expect(stoker_panel.text_segments[3].empty()) ||
        !expect(stoker_panel.text_segments[0].find("Stoker's Coffin Motel") !=
                std::string_view::npos) ||
        !expect(stoker_panel.has_more) ||
        !expect(stoker_panel.continuation_label == "          < MORE >") ||
        !expect(dialogue_panel_option_command(stoker_panel, 0U).value() ==
                DialoguePanelCommand::stoker_resolve_arrival)) {
        return 11;
    }

    const auto stoker_manager_boundary = make_dialogue_panel_view(
        make_stoker_motel_prompt(
            StokerMotelPromptId::manager_greeting_with_package),
        stoker_speaker, resolve_text);
    if (!expect(!stoker_manager_boundary.source_complete) ||
        !expect(stoker_manager_boundary.option_count == 0U) ||
        !expect(stoker_manager_boundary.source_boundary_reason.find(
                    "gameplay script package") != std::string::npos)) {
        return 12;
    }

    return 0;
}

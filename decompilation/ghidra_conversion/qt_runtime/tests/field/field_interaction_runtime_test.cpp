#include "field_interaction_runtime.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <QColor>
#include <QImage>
#include <QPainter>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::qt_runtime;

bool has_green_text(const QImage& image) {
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const auto pixel = image.pixelColor(x, y);
            if (pixel.green() > 150 && pixel.red() < 100 &&
                pixel.blue() < 140) {
                return true;
            }
        }
    }
    return false;
}

std::size_t green_pixel_count(const QImage& image, const int left,
                              const int top, const int width,
                              const int height) {
    std::size_t count = 0U;
    for (int y = top; y < top + height; ++y) {
        for (int x = left; x < left + width; ++x) {
            const auto pixel = image.pixelColor(x, y);
            if (pixel.green() > 150 && pixel.red() < 100 &&
                pixel.blue() < 140) {
                ++count;
            }
        }
    }
    return count;
}

bool has_color(const QImage& image, const QColor color,
               const int left, const int top,
               const int width, const int height) {
    for (int y = top; y < top + height; ++y) {
        for (int x = left; x < left + width; ++x) {
            if (image.pixelColor(x, y) == color) {
                return true;
            }
        }
    }
    return false;
}

bool no_old_paint_remains(const QImage& image) {
    const QColor old_paint(255, 0, 255);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) == old_paint) {
                return false;
            }
        }
    }
    return true;
}

void advance_frames(FieldInteractionRuntime& runtime,
                    std::array<RunnerRecordFields, 3>& party,
                    const std::array<bool, 3>& active_slots,
                    std::uint8_t& selection, std::uint16_t& seed,
                    const std::uint16_t frames) {
    for (std::uint16_t frame = 0U; frame < frames; ++frame) {
        (void)runtime.advance_controller_events(0U, party, active_slots,
                                                selection, seed);
    }
}

} // namespace

int main() {
    const auto assets = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT));
    if (!assets.complete()) {
        return 1;
    }

    scripting::GameScriptRuntime gameplay_scripts;
    if (!gameplay_scripts.load_package(
            SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
            std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                "dialogue" / "script-text.json")) {
        std::cerr << gameplay_scripts.failure() << '\n';
        return 2;
    }

    std::vector<std::uint16_t> audio_events;
    std::vector<std::uint16_t> stopped_audio_resources;
    std::vector<std::string_view> boundaries;
    FieldInteractionRuntime runtime(
        [&boundaries](const std::string_view reason) {
            boundaries.push_back(reason);
        },
        [&audio_events](const std::uint16_t id) {
            audio_events.push_back(id);
        },
        [&stopped_audio_resources](const std::uint16_t id) {
            stopped_audio_resources.push_back(id);
        });
    runtime.set_script_runtime(&gameplay_scripts);
    if (!runtime.load_assets(assets.catalog)) {
        return 2;
    }

    std::array<RunnerRecordFields, 3> party{};
    party[0].flags = 0x0100U;
    const std::array<bool, 3> active_slots{{true, false, false}};
    std::uint8_t selection = 2U;
    std::uint16_t seed = 0x3456U;

    // The generic door page waits ten source frames, then A reaches its three
    // action choices. The shared panel fully replaces any previous drawing.
    if (!runtime.open_prompt_for_dump(
            FieldInteractionPrompt::electromagnetic_lock_door, selection)) {
        return 3;
    }
    if (runtime.panel().option_count != 1U ||
        runtime.panel().text_segments[0].empty() ||
        runtime.panel().options[0].source_label.find('|') !=
            std::string_view::npos ||
        runtime.panel().options[0].source_label.find("< MORE >") ==
            std::string_view::npos) {
        return 4;
    }
    QImage image(320, 224, QImage::Format_RGB32);
    image.fill(QColor(255, 0, 255));
    {
        QPainter painter(&image);
        runtime.paint(painter);
    }
    if (!no_old_paint_remains(image) || !has_green_text(image) ||
        !has_color(image, QColor(0, 0, 0), 16, 152, 296, 72)) {
        return 5;
    }
    const auto stable_frame = image.copy();
    advance_frames(runtime, party, active_slots, selection, seed, 5U);
    image.fill(QColor(255, 0, 255));
    {
        QPainter painter(&image);
        runtime.paint(painter);
    }
    if (image != stable_frame) {
        return 6;
    }
    advance_frames(runtime, party, active_slots, selection, seed, 5U);
    (void)runtime.advance_controller_events(0x4000U, party, active_slots,
                                            selection, seed);
    if (runtime.panel().option_count != 3U ||
        runtime.panel().options[0].role !=
            DialoguePanelOptionRole::field_use_electronics ||
        runtime.panel().options[1].role !=
            DialoguePanelOptionRole::field_use_grenade ||
        runtime.panel().options[2].role !=
            DialoguePanelOptionRole::field_walk_away ||
        runtime.panel().options[2].source_label.find("Walk") ==
            std::string_view::npos) {
        return 7;
    }

    // Walk away is a terminal menu result. Its next field frame is not drawn
    // underneath the shared full-screen panel.
    advance_frames(runtime, party, active_slots, selection, seed, 10U);
    if (runtime.advance_controller_events(0x2000U, party, active_slots,
                                         selection, seed) !=
            FieldInteractionAdvanceResult::returned_to_field ||
        runtime.open()) {
        return 8;
    }

    // A sufficient passkey bypasses the option grid, consumes the selected
    // interaction state, and requests the source lock-open sound.
    runtime.reset_session();
    party[0].inventory_item_codes[0] = 0x14U;
    party[0].inventory_phase_bytes[0] = 2U;
    selection = 2U;
    seed = 0x3456U;
    audio_events.clear();
    if (!runtime.open_prompt_for_dump(
            FieldInteractionPrompt::electromagnetic_lock_door, selection)) {
        return 9;
    }
    advance_frames(runtime, party, active_slots, selection, seed, 10U);
    (void)runtime.advance_controller_events(0x4000U, party, active_slots,
                                            selection, seed);
    if (selection != 0xFFU || runtime.panel().option_count != 0U ||
        runtime.panel().text_segments[0].find("Your Maglock Passkey") != 0U ||
        runtime.panel().text_segment_count != 3U ||
        std::find(audio_events.begin(), audio_events.end(), 0x4BU) ==
            audio_events.end()) {
        return 10;
    }

    // The Security Terminal has its own two-row action menu and typed role.
    runtime.reset_session();
    selection = 2U;
    seed = 0x1234U;
    if (!runtime.open_prompt_for_dump(
            FieldInteractionPrompt::building_security_terminal, selection,
            false, 4U) ||
        runtime.panel().option_count != 2U ||
        runtime.panel().options[0].role !=
            DialoguePanelOptionRole::field_security_access ||
        runtime.panel().options[0].source_label !=
            "<Use your Electronics skill>" ||
        runtime.panel().options[1].role !=
            DialoguePanelOptionRole::field_walk_away) {
        return 11;
    }
    image.fill(Qt::black);
    {
        QPainter painter(&image);
        runtime.paint(painter);
    }
    // Check both rows have substantial rendered text. A single green pixel
    // from a bezel or stale panel mark must not satisfy this menu assertion.
    if (green_pixel_count(image, 48, 152, 256, 16) < 300U ||
        green_pixel_count(image, 48, 176, 256, 16) < 150U) {
        return 22;
    }
    advance_frames(runtime, party, active_slots, selection, seed, 10U);
    (void)runtime.advance_controller_events(0x4000U, party, active_slots,
                                            selection, seed);
    if (runtime.panel().option_count != 0U ||
        runtime.panel().text_segments[0].empty() ||
        !runtime.open()) {
        return 12;
    }
    if (std::find(boundaries.begin(), boundaries.end(),
                  "field.security_terminal.floor_index_unavailable") !=
        boundaries.end()) {
        return 13;
    }

    // Success uses the typed FFE12C floor index when available. Until that
    // scene initialization field is wired into the host, the runtime retains
    // the source files sentence and records the missing input explicitly.
    std::uint16_t successful_seed = 0U;
    bool found_file_success_seed = false;
    party[0].skills[9] = 8U;
    for (std::uint32_t candidate = 0U; candidate <= 0xFFFFU; ++candidate) {
        auto candidate_seed = static_cast<std::uint16_t>(candidate);
        const auto successes = count_thresholded_random_rolls(
            party[0].skills[9], 5,
            [&candidate_seed](const std::uint16_t bound) {
                return sample_source_random_bound(candidate_seed, bound);
            });
        if (successes >= 3U) {
            successful_seed = static_cast<std::uint16_t>(candidate);
            found_file_success_seed = true;
            break;
        }
    }
    if (!found_file_success_seed) {
        return 14;
    }
    const auto* fifth_floor = gameplay_scripts.dialogue_text(
        "field.security-terminal.floor.4");
    if (fifth_floor == nullptr) {
        return 15;
    }
    runtime.reset_session();
    audio_events.clear();
    stopped_audio_resources.clear();
    selection = 2U;
    seed = successful_seed;
    if (!runtime.open_prompt_for_dump(
            FieldInteractionPrompt::building_security_terminal, selection)) {
        return 16;
    }
    advance_frames(runtime, party, active_slots, selection, seed, 10U);
    (void)runtime.advance_controller_events(0x4000U, party, active_slots,
                                            selection, seed);
    if (runtime.panel().option_count != 0U ||
        std::find(boundaries.begin(), boundaries.end(),
                  "field.security_terminal.floor_index_unavailable") ==
            boundaries.end()) {
        return 17;
    }

    runtime.reset_session();
    audio_events.clear();
    stopped_audio_resources.clear();
    selection = 2U;
    seed = successful_seed;
    if (!runtime.open_prompt_for_dump(
            FieldInteractionPrompt::building_security_terminal, selection,
            false, 4U)) {
        return 18;
    }
    advance_frames(runtime, party, active_slots, selection, seed, 10U);
    (void)runtime.advance_controller_events(0x4000U, party, active_slots,
                                            selection, seed);
    if (runtime.panel().text_segment_count < 2U ||
        runtime.panel().text_segments[1] != *fifth_floor) {
        std::cerr << "floor detail mismatch: count="
                  << runtime.panel().text_segment_count << " expected='"
                  << *fifth_floor << "'\n";
        for (std::size_t index = 0U;
             index < runtime.panel().text_segment_count; ++index) {
            std::cerr << "segment[" << index << "]='"
                      << runtime.panel().text_segments[index] << "'\n";
        }
        return 19;
    }
    if (stopped_audio_resources != std::vector<std::uint16_t>{0x30U}) {
        std::cerr << "security-terminal success did not stop resource 0x30\n";
        return 23;
    }

    // Grenade use consumes one stored charge and starts the explosion/alarm
    // sounds; the alarm stays active while the result page is displayed.
    runtime.reset_session();
    party[0].inventory_item_codes.fill(0U);
    party[0].inventory_phase_bytes.fill(0U);
    party[0].inventory_item_codes[2] = 0x0DU;
    party[0].inventory_phase_bytes[2] = 2U;
    selection = 3U;
    seed = 0x2345U;
    audio_events.clear();
    stopped_audio_resources.clear();
    if (!runtime.open_prompt_for_dump(
            FieldInteractionPrompt::electromagnetic_lock_door, selection)) {
        return 20;
    }
    advance_frames(runtime, party, active_slots, selection, seed, 10U);
    (void)runtime.advance_controller_events(0x4000U, party, active_slots,
                                            selection, seed);
    advance_frames(runtime, party, active_slots, selection, seed, 10U);
    (void)runtime.advance_controller_events(0x1000U, party, active_slots,
                                            selection, seed);
    if (party[0].inventory_phase_bytes[2] != 1U ||
        selection != 0xFFU || !runtime.open() ||
        std::find(audio_events.begin(), audio_events.end(), 0x2CU) ==
            audio_events.end() ||
        std::find(audio_events.begin(), audio_events.end(), 0x30U) ==
            audio_events.end() || !stopped_audio_resources.empty()) {
        std::cerr << "grenade audio/state mismatch: item="
                  << static_cast<int>(party[0].inventory_phase_bytes[2])
                  << " selection=" << static_cast<int>(selection)
                  << " panel_open=" << runtime.open() << " events=";
        for (const auto id : audio_events) std::cerr << id << ',';
        std::cerr << " stopped=";
        for (const auto id : stopped_audio_resources) std::cerr << id << ',';
        std::cerr << '\n';
        return 21;
    }
    return 0;
}

#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct RunnerSelectionScreenState {
    std::int16_t active_runner = 0; // FFF118
    bool alternate_presenter = false; // FFF0A9
    bool background_dirty = false; // FFEF14 bit 0
    bool sprite_records_dirty = false; // FFEF14 bit 1
    bool selection_published = false;
};

struct RunnerSelectionGridRequest {
    std::uint8_t row = 0;
    std::uint8_t column = 0;
    bool initial = false;
};

struct RunnerSpriteSource {
    std::int8_t animation_status = 0;
    std::int8_t screen_x = 0; // source record +3, scaled by 0x10
    std::int8_t screen_y = 0; // source record +4, scaled by 0x10
};

struct RunnerSpriteBuildRequest {
    std::int16_t origin_x = 0;
    std::int16_t origin_y = 0;
    std::uint16_t animation_seed = 0x2000;
};

struct RunnerSelectionScreenCallbacks {
    std::function<void()> clear_background;
    std::function<void()> copy_background_asset;
    std::function<void()> upload_background_asset;
    std::function<void(const RunnerSelectionGridRequest&)> render_grid;
    std::function<void()> render_header_text;
    std::function<void()> update_runner_animation;
    std::function<void()> build_sprite_records_once;
    std::function<void()> service_noop_boundary;
    std::function<void(const RunnerSpriteBuildRequest&)> build_runner_sprites;

    std::function<std::optional<RunnerSpriteSource>(std::int16_t)>
        resolve_selected_runner_sprite_source;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_events;
    std::function<std::optional<std::int16_t>(std::int16_t)>
        move_runner_selection;
    std::function<void()> publish_runner_selection;
    std::function<void()> render_field_hud;
    std::function<void()> upload_menu_resources;
    std::function<std::int32_t()> render_menu_footer;
};

enum class RunnerSelectionResult {
    unavailable,
    runner_selected,
    menu_returned,
};

// ROM 0x532BC: construct the runner-selection presentation from host assets,
// then run the source's 10-by-14 grid fill as named presentation requests.
void initialize_runner_selection_screen(
    RunnerSelectionScreenState& state,
    const RunnerSelectionScreenCallbacks& callbacks);

// ROM 0x537FA: refresh sprite records for the selected runner. Source record
// pointers are resolved once by the host callback; no ROM or RAM addresses
// escape into the native conversion.
bool refresh_runner_selection_sprites(
    RunnerSelectionScreenState& state,
    const RunnerSelectionScreenCallbacks& callbacks);

// ROM 0x53704: poll the runner-selection screen until a runner is committed or
// the menu/footer path is requested. Input masks are source edge-event masks,
// while movement and presentation are native callbacks.
RunnerSelectionResult run_runner_selection_screen(
    RunnerSelectionScreenState& state,
    const RunnerSelectionScreenCallbacks& callbacks,
    std::int32_t* menu_result = nullptr);

} // namespace shadowrun::ghidra_conversion

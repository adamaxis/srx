#include "shadowrun_ghidra/integration/opening_intro_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace shadowrun::ghidra_conversion::integration {
namespace {

constexpr std::uint32_t kUnavailableSprite =
    kOpeningIntroSpriteHandleTag |
    (static_cast<std::uint32_t>(OpeningIntroSpriteFamily::unavailable) << 16U);

constexpr std::uint32_t action(const std::uint32_t value) { return value; }

template <typename Callback>
void invoke(const Callback& callback) {
    if (callback) {
        callback();
    }
}

void source_boundary(const OpeningIntroCallbacks& callbacks,
                     const std::string_view name) {
    if (callbacks.source_boundary) {
        callbacks.source_boundary(name);
    }
}

void set_stage(OpeningIntroState& state,
               const OpeningIntroCallbacks& callbacks,
               const OpeningIntroStage stage) {
    if (state.stage == stage) {
        return;
    }
    state.stage = stage;
    if (callbacks.stage_changed) {
        callbacks.stage_changed(stage);
    }
}

void set_scene(OpeningIntroState& state,
               const OpeningIntroCallbacks& callbacks,
               const OpeningIntroScene scene) {
    if (state.scene == scene) {
        return;
    }
    state.scene = scene;
    if (callbacks.scene_changed) {
        callbacks.scene_changed(scene);
    }
}

void emit_sprite(const OpeningIntroCallbacks& callbacks,
                 const std::uint32_t descriptor, const std::uint16_t x,
                 const std::uint16_t y, const std::uint16_t palette_flags) {
    if (callbacks.draw_sprite && descriptor != kUnavailableSprite) {
        callbacks.draw_sprite({descriptor, x, y, palette_flags});
    }
}

void emit_text(const OpeningIntroCallbacks& callbacks,
               const std::uint32_t asset, const std::uint16_t destination,
               const std::uint16_t palette_flags) {
    if (callbacks.render_text && asset != 0U) {
        callbacks.render_text({asset, destination, palette_flags});
    }
}

void emit_text(const OpeningIntroCallbacks& callbacks,
               const OpeningIntroTextAsset asset,
               const std::uint16_t destination,
               const std::uint16_t palette_flags) {
    emit_text(callbacks, opening_intro_text_handle(asset), destination,
              palette_flags);
}

bool has_source_frame_callbacks(const OpeningIntroCallbacks& callbacks) {
    return static_cast<bool>(callbacks.wait_for_vblank) &&
           static_cast<bool>(callbacks.sample_controller) &&
           static_cast<bool>(callbacks.latch_controller_events);
}

void begin_source_frame(OpeningIntroState& state,
                        const OpeningIntroCallbacks& callbacks) {
    invoke(callbacks.begin_frame);
    callbacks.wait_for_vblank();
    ++state.source_frame;
}

// A VBlank inside a ROM palette routine: the VDP keeps showing the last
// sprite table, so the host's sprite list is kept rather than cleared.
void begin_held_source_frame(OpeningIntroState& state,
                             const OpeningIntroCallbacks& callbacks) {
    callbacks.wait_for_vblank();
    ++state.source_frame;
}

void sample_and_latch(const OpeningIntroCallbacks& callbacks) {
    callbacks.sample_controller();
    callbacks.latch_controller_events();
}

void emit_schedule_action(const std::uint32_t action_id,
                          const OpeningIntroCallbacks& callbacks) {
    // These source action IDs are provenance tokens at this boundary; the
    // host never calls a ROM address. The 0x49A0 callback completes the
    // delayed Seattle hold record in the same typed schedule.
    switch (action_id) {
    case 0x5AEC6U:
        source_boundary(callbacks, "0x5AEC6.render_time_record");
        emit_text(callbacks, OpeningIntroTextAsset::time, 0x210EU, 0xA000U);
        break;
    case 0x5AEDAU:
        source_boundary(callbacks, "0x5AEDA.render_place_record");
        emit_text(callbacks, OpeningIntroTextAsset::place, 0x2210U, 0xA000U);
        break;
    case 0x5AEEEU:
        // The ROM entry continues through 0x5AF02 and 0x5AF16 after the
        // cover row, writing all three team-story rows before its final jump.
        source_boundary(callbacks, "0x5AEEE.render_team_records");
        emit_text(callbacks, OpeningIntroTextAsset::team_cover, 0x2304U,
                  0xA000U);
        emit_text(callbacks, OpeningIntroTextAsset::team_runners, 0x2384U,
                  0xA000U);
        emit_text(callbacks, OpeningIntroTextAsset::team_wilderness, 0x2404U,
                  0xA000U);
        break;
    case 0x5B378U:
        // 0x5B378 jsr 0xFFE clears Plane A, then stamps 0x1DF468 at
        // D4=0x40E through five 0x5B36E offsets. Plane B seattle stays.
        source_boundary(callbacks, "0x5B378.stamp_seattle_focus_outline");
        break;
    case 0x49A0U:
        // The 0x5ADD0 schedule keeps the final focus frame for 0x000F ticks,
        // then calls this source RTS before the terminal zero-delay record.
        source_boundary(callbacks, "0x49A0.seattle_focus_hold_complete");
        break;
    case 0x5AE9EU:
        // 0x5ADF0 points here after the first stage-2 delay. The ROM routine
        // at 0x5AE9E writes two wilderness-event lines, then rewrites the
        // Seattle/team records in this exact destination order. These are
        // distinct source records; do not collapse them into one generic
        // story block or duplicate the date line.
        source_boundary(callbacks, "0x5AE9E.render_stage2_story_records");
        emit_text(callbacks, OpeningIntroTextAsset::without_warning, 0x2186U,
                  0xE000U);
        emit_text(callbacks, OpeningIntroTextAsset::ambushed, 0x2286U,
                  0xE000U);
        emit_text(callbacks, OpeningIntroTextAsset::time, 0x210EU, 0xA000U);
        emit_text(callbacks, OpeningIntroTextAsset::place, 0x2210U, 0xA000U);
        emit_text(callbacks, OpeningIntroTextAsset::team_cover, 0x2304U,
                  0xA000U);
        emit_text(callbacks, OpeningIntroTextAsset::team_runners, 0x2384U,
                  0xA000U);
        emit_text(callbacks, OpeningIntroTextAsset::team_wilderness, 0x2404U,
                  0xA000U);
        break;
    default:
        source_boundary(callbacks, "opening_intro.unresolved_schedule_action");
        break;
    }
}

void emit_title_intro_text(
    const OpeningIntroCallbacks& callbacks,
    const TitleIntroStageTextRequest& request) {
    const auto asset = opening_intro_text_asset(request.text_asset);
    emit_text(callbacks, asset, request.destination_word,
              request.palette_flags);
}

void emit_title_intro_text(
    const OpeningIntroCallbacks& callbacks,
    const TitleIntroLateTextRenderRequest& request) {
    const auto asset = opening_intro_text_asset(request.text_asset);
    emit_text(callbacks, asset, request.destination_word,
              request.palette_flags);
}

void emit_title_intro_text(
    const OpeningIntroCallbacks& callbacks,
    const TitleIntroTextRenderRequest& request) {
    const auto asset = opening_intro_text_asset(request.text_asset);
    emit_text(callbacks, asset, request.destination_word,
              request.palette_flags);
}

void render_stage2_story_text(const OpeningIntroCallbacks& callbacks) {
    // 0x5AE3A and 0x5ADFE are called at the 0x164 stage-2 pass. The leaf
    // routines own the destination words; this adapter only supplies the
    // extracted source records.
    render_title_intro_scroll_text_batch(
        opening_intro_text_handle(OpeningIntroTextAsset::blank),
        [&callbacks](const auto& request) { emit_title_intro_text(callbacks, request); });

    render_title_intro_late_text_batch(
        {opening_intro_text_handle(OpeningIntroTextAsset::massacre),
         opening_intro_text_handle(OpeningIntroTextAsset::captured),
         opening_intro_text_handle(OpeningIntroTextAsset::slain)},
        [&callbacks](const auto& request) { emit_title_intro_text(callbacks, request); });
}

void render_stage_loop_middle_text(const OpeningIntroCallbacks& callbacks) {
    render_title_intro_middle_text_batch(
        {opening_intro_text_handle(OpeningIntroTextAsset::spending),
         opening_intro_text_handle(OpeningIntroTextAsset::vow),
         opening_intro_text_handle(OpeningIntroTextAsset::brother_death)},
        [&callbacks](const auto& request) { emit_title_intro_text(callbacks, request); });
}

void render_stage_loop_final_text(const OpeningIntroCallbacks& callbacks) {
    render_title_intro_text_batch(
        {opening_intro_text_handle(OpeningIntroTextAsset::landing),
         opening_intro_text_handle(OpeningIntroTextAsset::flag_cab),
         opening_intro_text_handle(OpeningIntroTextAsset::last_credstick),
         opening_intro_text_handle(OpeningIntroTextAsset::transaction),
         opening_intro_text_handle(OpeningIntroTextAsset::motel),
         opening_intro_text_handle(OpeningIntroTextAsset::redmond)},
        [&callbacks](const auto& request) { emit_title_intro_text(callbacks, request); });
}

void clear_stage_loop_center_text(const OpeningIntroCallbacks& callbacks) {
    // ROM 0x5B056 calls 0x5AA1E at DD08 == -0x1B0. That batch writes the
    // blank source record (0x5B7B2) over the three middle-story rows; it does
    // not install the later landing paragraph rendered at loop index 0x348.
    render_title_intro_center_text_batch(
        opening_intro_text_handle(OpeningIntroTextAsset::blank),
        [&callbacks](const auto& request) { emit_title_intro_text(callbacks, request); });
}

void emit_overlay_stream(const OpeningIntroCallbacks& callbacks,
                         const TitleIntroSpriteStreamRequest& request) {
    if (request.sprite_descriptor == kUnavailableSprite) {
        source_boundary(callbacks, "opening_intro.overlay.source_stream_without_direct_asset");
        return;
    }
    // The importer preserves the source name-table origin in the cropped
    // asset's catalog record. Coordinates are therefore a typed callback
    // token only; the Qt adapter places the direct image at that recorded
    // origin instead of replaying a VDP stream.
    emit_sprite(callbacks, request.sprite_descriptor, 0U, 0U,
                request.tile_flags);
}

void emit_sequence_stream(const OpeningIntroCallbacks& callbacks,
                          const TitleIntroSpriteStreamRequest& request) {
    if (request.sprite_descriptor == kUnavailableSprite) {
        source_boundary(callbacks, "opening_intro.sequence.source_stream_without_direct_asset");
        return;
    }
    emit_sprite(callbacks, request.sprite_descriptor, 152U, 128U,
                request.tile_flags);
}

void emit_stage_stream(const OpeningIntroCallbacks& callbacks,
                       const TitleIntroHudSpriteStreamRequest& request) {
    (void)request;
    source_boundary(callbacks, "0x5AA96.render_hud_sprite_stream");
}

void emit_stage_fill(const OpeningIntroCallbacks& callbacks,
                     const TitleIntroFillRowRequest&) {
    source_boundary(callbacks, "0x5B32C.write_title_intro_fill_row");
}

void emit_scroll_fill(const OpeningIntroCallbacks& callbacks,
                      const TitleIntroScrollFillRequest&) {
    source_boundary(callbacks, "0x5B00A.fill_transition_tiles");
}

void reset_animation_children(OpeningIntroState& state) {
    reset_title_intro_overlay_schedule(state.overlay.schedule);
    reset_title_intro_sequence(state.sequence);
    reset_title_intro_persistent_sequence(state.persistent);
    reset_title_intro_motion(state.motion);
    reset_title_intro_dual_sprites(state.dual);
    reset_title_intro_timed_sprites(state.timed);
    reset_title_intro_targeted_sweep(state.targeted_sweep);
    reset_title_intro_linear_sweep_a(state.linear_sweep_a);
    reset_title_intro_linear_sweep_b(state.linear_sweep_b);
    reset_title_intro_final_projection(state.final_projection);
}

} // namespace

std::uint32_t opening_intro_sprite_handle(
    const OpeningIntroSpriteFamily family, const std::uint8_t variant) {
    return kOpeningIntroSpriteHandleTag |
           (static_cast<std::uint32_t>(family) << 16U) |
           (static_cast<std::uint32_t>(variant) << 8U);
}

OpeningIntroSpriteGroup opening_intro_sprite_group(
    const std::uint32_t handle) {
    if ((handle & 0xFF000000U) != kOpeningIntroSpriteHandleTag) {
        return {};
    }
    return {static_cast<OpeningIntroSpriteFamily>((handle >> 16U) & 0xFFU),
            static_cast<std::uint8_t>((handle >> 8U) & 0xFFU)};
}

std::string opening_intro_sprite_alias_prefix(
    const OpeningIntroSpriteGroup group) {
    const auto variant = std::to_string(group.variant);
    switch (group.family) {
    case OpeningIntroSpriteFamily::trivid_left:
        return "intro.sprites.trivid.left.0.";
    case OpeningIntroSpriteFamily::trivid_right:
        return "intro.sprites.trivid.right.0.";
    case OpeningIntroSpriteFamily::trivid_timed:
        return "intro.sprites.trivid.timed." + variant + ".";
    case OpeningIntroSpriteFamily::wilderness_overlay:
        return "intro.sprites.wilderness.overlay." + variant + ".";
    case OpeningIntroSpriteFamily::wilderness_final_overlay:
        return "intro.sprites.wilderness.final.overlay." + variant + ".";
    case OpeningIntroSpriteFamily::ambush_actor:
        return "intro.sprites.ambush.actor." + variant + ".";
    case OpeningIntroSpriteFamily::ambush_muzzle:
        return "intro.sprites.ambush.muzzle." + variant + ".";
    case OpeningIntroSpriteFamily::ambush_flash:
        return "intro.sprites.ambush.flash." + variant + ".";
    case OpeningIntroSpriteFamily::ambush_body:
        return "intro.sprites.ambush.body." + variant + ".";
    case OpeningIntroSpriteFamily::projectile_b:
        return "intro.sprites.ambush.projectile.b." + variant + ".";
    case OpeningIntroSpriteFamily::projectile_c:
        return "intro.sprites.ambush.projectile.c." + variant + ".";
    case OpeningIntroSpriteFamily::projectile_d:
        return "intro.sprites.ambush.projectile.d." + variant + ".";
    case OpeningIntroSpriteFamily::michael:
        return "intro.sprites.ambush.michael." + variant + ".";
    case OpeningIntroSpriteFamily::helicopter:
        return "intro.sprites.helicopter." + variant + ".";
    case OpeningIntroSpriteFamily::aircraft_movement:
        return "intro.sprites.aircraft.movement." + variant + ".";
    case OpeningIntroSpriteFamily::aircraft_plane_a:
        return "intro.aircraft.plane.a";
    case OpeningIntroSpriteFamily::background_patch:
        return "intro.background.patch." + variant;
    case OpeningIntroSpriteFamily::wilderness_map_left:
        return "intro.sprites.ambush.map.left.0.";
    case OpeningIntroSpriteFamily::wilderness_map_center:
        return "intro.sprites.ambush.map.center.1.";
    case OpeningIntroSpriteFamily::wilderness_map_right:
        return "intro.sprites.ambush.map.right.2.";
    case OpeningIntroSpriteFamily::unavailable:
        break;
    }
    return {};
}

OpeningIntroDrawPoint opening_intro_sat_draw_point(
    const std::uint16_t sat_x, const std::uint16_t sat_y, const int origin_x,
    const int origin_y) {
    return {origin_x + static_cast<int>(static_cast<std::int16_t>(sat_x)) -
                kIntroSatExtractOrigin,
            origin_y + static_cast<int>(static_cast<std::int16_t>(sat_y)) -
                kIntroSatExtractOrigin};
}

OpeningIntroDrawPoint opening_intro_aircraft_plane_draw_point(
    const std::int16_t scroll_x) {
    return {kAircraftPlaneNametableX + scroll_x, kAircraftPlaneNametableY};
}

OpeningIntroOutlinePlan opening_intro_seattle_outline_plan(
    const std::uint8_t phase) {
    // ROM 0x5B378 consumes the five 0x5B36E offsets with the matching size
    // pairs at 0x5B364. Those are Plane-A transfer parameters, not host pixel
    // bounds. The native presentation uses the visible boxes from the
    // independent Seattle sequence (section 02 frames 409-413) as the five
    // authored 320x224 animation states.
    constexpr std::array<OpeningIntroOutlinePlan, 5> kVisibleFrames{{
        {57, 70, 193, 127},
        {75, 92, 154, 105},
        {98, 116, 116, 81},
        {123, 132, 66, 57},
        {139, 147, 19, 17},
    }};
    const auto index = static_cast<std::size_t>(std::min<std::uint8_t>(
        phase, static_cast<std::uint8_t>(kVisibleFrames.size() - 1U)));
    return kVisibleFrames[index];
}

std::uint32_t opening_intro_text_handle(const OpeningIntroTextAsset asset) {
    return kOpeningIntroTextHandleTag |
           static_cast<std::uint32_t>(asset);
}

OpeningIntroTextAsset opening_intro_text_asset(const std::uint32_t handle) {
    if ((handle & 0xFF000000U) != kOpeningIntroTextHandleTag) {
        return OpeningIntroTextAsset::time;
    }
    const auto value = handle & 0xFFU;
    if (value > static_cast<std::uint32_t>(OpeningIntroTextAsset::blank)) {
        return OpeningIntroTextAsset::time;
    }
    return static_cast<OpeningIntroTextAsset>(value);
}

std::string_view opening_intro_text_id(const OpeningIntroTextAsset asset) {
    switch (asset) {
    case OpeningIntroTextAsset::time: return "time";
    case OpeningIntroTextAsset::place: return "place";
    case OpeningIntroTextAsset::team_cover: return "team_cover";
    case OpeningIntroTextAsset::team_runners: return "team_runners";
    case OpeningIntroTextAsset::team_wilderness: return "team_wilderness";
    case OpeningIntroTextAsset::without_warning: return "without_warning";
    case OpeningIntroTextAsset::ambushed: return "ambushed";
    case OpeningIntroTextAsset::massacre: return "massacre";
    case OpeningIntroTextAsset::captured: return "captured";
    case OpeningIntroTextAsset::slain: return "slain";
    case OpeningIntroTextAsset::tri_vid_recovered: return "tri_vid_recovered";
    case OpeningIntroTextAsset::national_news: return "national_news";
    case OpeningIntroTextAsset::brother_last_die: return "brother_last_die";
    case OpeningIntroTextAsset::spending: return "spending";
    case OpeningIntroTextAsset::vow: return "vow";
    case OpeningIntroTextAsset::brother_death: return "brother_death";
    case OpeningIntroTextAsset::landing: return "landing";
    case OpeningIntroTextAsset::flag_cab: return "flag_cab";
    case OpeningIntroTextAsset::last_credstick: return "last_credstick";
    case OpeningIntroTextAsset::transaction: return "transaction";
    case OpeningIntroTextAsset::motel: return "motel";
    case OpeningIntroTextAsset::redmond: return "redmond";
    case OpeningIntroTextAsset::blank:
        return {};
    }
    return {};
}

const char* opening_intro_stage_name(const OpeningIntroStage stage) {
    switch (stage) {
    case OpeningIntroStage::not_started:
        return "not_started";
    case OpeningIntroStage::schedule:
        return "schedule";
    case OpeningIntroStage::stage2_palette_fade_out:
        return "stage2_palette_fade_out";
    case OpeningIntroStage::stage2_palette_fade_in:
        return "stage2_palette_fade_in";
    case OpeningIntroStage::stage2_first_loop:
        return "stage2_first_loop";
    case OpeningIntroStage::stage2_second_loop:
        return "stage2_second_loop";
    case OpeningIntroStage::orchestrator_palette_fade_out:
        return "orchestrator_palette_fade_out";
    case OpeningIntroStage::orchestrator_entry:
        return "orchestrator_entry";
    case OpeningIntroStage::orchestrator_palette_fade_in:
        return "orchestrator_palette_fade_in";
    case OpeningIntroStage::orchestrator_loop:
        return "orchestrator_loop";
    case OpeningIntroStage::stage_loop_palette_fade_out:
        return "stage_loop_palette_fade_out";
    case OpeningIntroStage::stage_loop_entry:
        return "stage_loop_entry";
    case OpeningIntroStage::stage_loop:
        return "stage_loop";
    case OpeningIntroStage::post_intro_palette_fade_out:
        return "post_intro_palette_fade_out";
    case OpeningIntroStage::post_intro:
        return "post_intro";
    case OpeningIntroStage::exit_palette_fade_out:
        return "exit_palette_fade_out";
    case OpeningIntroStage::complete:
        return "complete";
    case OpeningIntroStage::interrupted:
        return "interrupted";
    }
    return "unknown";
}

const char* opening_intro_scene_name(const OpeningIntroScene scene) {
    switch (scene) {
    case OpeningIntroScene::seattle:
        return "seattle";
    case OpeningIntroScene::wilderness:
        return "wilderness";
    case OpeningIntroScene::trivid:
        return "trivid";
    case OpeningIntroScene::airport:
        return "airport";
    case OpeningIntroScene::cab_map:
        return "cab_map";
    }
    return "unknown";
}

const OpeningIntroSourcePlan& opening_intro_source_plan() {
    static const OpeningIntroSourcePlan plan = [] {
        OpeningIntroSourcePlan result{};
        const auto sprite = [](const OpeningIntroSpriteFamily family,
                               const std::uint8_t variant) {
            return opening_intro_sprite_handle(family, variant);
        };

        // 0x5ADD0: timed date/place/team records, the five-pass Seattle
        // focus box, then a 0x000F-tick hold completed by the source RTS at
        // 0x49A0 before the schedule's zero-delay terminal record.
        result.opening_schedule = {
            {0x003CU, action(0x5AEC6U)},
            {0x0078U, action(0x5AEDAU)},
            {0x00B4U, action(0x5AEEEU)},
            {0x00F0U, action(0x5B378U)},
            {0x000FU, action(0x49A0U)},
        };
        result.stage2_schedule = {{0x0001U, action(0x5AE9EU)}};

        // 0x5A0B2 reads each pointer and delay from the six-byte records at
        // 0x5A1D2. The source hold lengths are 12, 24, 60, then 12 updates.
        result.sequence_phases = {{
            {sprite(OpeningIntroSpriteFamily::background_patch, 0), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::background_patch, 1), 0x0018U},
            {sprite(OpeningIntroSpriteFamily::background_patch, 2), 0x003CU},
            {sprite(OpeningIntroSpriteFamily::background_patch, 3), 0x000CU},
        }};

        // 0x5A398, 0x5A482, and 0x5A562.
        result.persistent_phases = {{
            {sprite(OpeningIntroSpriteFamily::ambush_actor, 0), 0x003CU},
            {sprite(OpeningIntroSpriteFamily::ambush_actor, 1), 0x0024U},
            {sprite(OpeningIntroSpriteFamily::ambush_actor, 0), 0x003CU},
            {sprite(OpeningIntroSpriteFamily::ambush_actor, 1), 0x0018U},
        }};
        result.motion_phases = {{
            {sprite(OpeningIntroSpriteFamily::ambush_muzzle, 0), 0x003CU,
             0x0001U, 0x0000U},
            {sprite(OpeningIntroSpriteFamily::ambush_muzzle, 1), 0x0006U,
             0x011AU, 0x0004U},
            {sprite(OpeningIntroSpriteFamily::ambush_muzzle, 1), 0x000CU,
             0x0000U, 0x0006U},
            {sprite(OpeningIntroSpriteFamily::ambush_muzzle, 1), 0x0012U,
             0x0000U, 0x0008U},
            {sprite(OpeningIntroSpriteFamily::ambush_muzzle, 1), 0x003CU,
             0x0001U, 0x0000U},
        }};
        result.dual_sprites = {
            sprite(OpeningIntroSpriteFamily::ambush_flash, 0),
            sprite(OpeningIntroSpriteFamily::ambush_flash, 1),
            sprite(OpeningIntroSpriteFamily::ambush_flash, 2),
            sprite(OpeningIntroSpriteFamily::ambush_flash, 3),
            sprite(OpeningIntroSpriteFamily::ambush_flash, 1),
            sprite(OpeningIntroSpriteFamily::ambush_flash, 2),
            sprite(OpeningIntroSpriteFamily::ambush_flash, 3),
            sprite(OpeningIntroSpriteFamily::ambush_flash, 0),
        };

        // 0x5A602: six SAT lists drawn by 0x5A59C after 0x5A582.
        result.timed_sprites = {{
            sprite(OpeningIntroSpriteFamily::ambush_body, 0),
            sprite(OpeningIntroSpriteFamily::ambush_body, 1),
            sprite(OpeningIntroSpriteFamily::ambush_body, 2),
            sprite(OpeningIntroSpriteFamily::ambush_body, 3),
            sprite(OpeningIntroSpriteFamily::ambush_body, 4),
            sprite(OpeningIntroSpriteFamily::ambush_body, 5),
        }};
        result.stage_timed_phases = {{
            {sprite(OpeningIntroSpriteFamily::trivid_timed, 0), 0x0128U,
             0x0113U, 0x0006U},
            {sprite(OpeningIntroSpriteFamily::trivid_timed, 1), 0x011FU,
             0x0129U, 0x0006U},
            {sprite(OpeningIntroSpriteFamily::trivid_timed, 2), 0x0113U,
             0x0145U, 0x0006U},
        }};

        result.targeted_sweep_phases = {{
            {sprite(OpeningIntroSpriteFamily::projectile_b, 0), 0x00BBU,
             0x002AU, 0x0040U},
            {sprite(OpeningIntroSpriteFamily::projectile_b, 1), 0x00A3U,
             0x0030U, 0x0040U},
            {sprite(OpeningIntroSpriteFamily::projectile_b, 2), 0x0113U,
             0x004EU, 0x0145U},
            {sprite(OpeningIntroSpriteFamily::projectile_b, 3), 0x00E3U,
             0x0030U, 0x0183U},
            {sprite(OpeningIntroSpriteFamily::projectile_b, 4), 0x00F8U,
             0x006AU, 0x0040U},
            {sprite(OpeningIntroSpriteFamily::projectile_b, 5), 0x0105U,
             0x002AU, 0x0104U},
            {sprite(OpeningIntroSpriteFamily::projectile_b, 6), 0x00BCU,
             0x003CU, 0x0040U},
        }};

        // ROM tables 0x5A7AA (A) and 0x5A888 (B): {descriptor.l, y.w,
        // delay.w}. The y words keep bit 8 (0x116, 0x12A, 0x120, 0x10C);
        // A's delays are 0x30/0x3C, B's 0x36/0x48 [C: ROM bytes, emulator
        // FFF16A/FFF176 deadlines].
        const std::array<std::uint16_t, kTitleIntroLinearSweepPhaseCount>
            linear_a_y{{0x00ACU, 0x00E6U, 0x00BBU, 0x0116U, 0x00B1U,
                       0x012AU, 0x00ACU, 0x0116U}};
        const std::array<std::uint16_t, kTitleIntroLinearSweepPhaseCount>
            linear_b_y{{0x0120U, 0x00D7U, 0x0094U, 0x00A6U, 0x010CU,
                       0x00E2U, 0x00C4U, 0x00F8U}};
        const std::array<std::uint16_t, kTitleIntroLinearSweepPhaseCount>
            linear_a_delays{{0x0030U, 0x003CU, 0x0030U, 0x003CU, 0x0030U,
                             0x003CU, 0x0030U, 0x003CU}};
        const std::array<std::uint16_t, kTitleIntroLinearSweepPhaseCount>
            linear_b_delays{{0x0036U, 0x0048U, 0x0036U, 0x0048U, 0x0036U,
                             0x0048U, 0x0036U, 0x0048U}};
        for (std::size_t index = 0; index < kTitleIntroLinearSweepPhaseCount;
             ++index) {
            result.linear_sweep_a_phases[index] = {
                sprite(OpeningIntroSpriteFamily::projectile_c,
                       static_cast<std::uint8_t>(index)),
                linear_a_y[index], linear_a_delays[index]};
            result.linear_sweep_b_phases[index] = {
                sprite(OpeningIntroSpriteFamily::projectile_d,
                       static_cast<std::uint8_t>(index)),
                linear_b_y[index], linear_b_delays[index]};
        }

        result.final_projection_phases = {{
            {sprite(OpeningIntroSpriteFamily::michael, 0), 0x0003U},
            {sprite(OpeningIntroSpriteFamily::michael, 1), 0x0006U},
            {sprite(OpeningIntroSpriteFamily::michael, 2), 0x0009U},
        }};

        // 0x5B180 and 0x5B270.
        for (std::size_t index = 0; index < kTitleIntroPulsePhaseCount;
             ++index) {
            result.pulse_descriptors[index] = sprite(
                OpeningIntroSpriteFamily::helicopter,
                static_cast<std::uint8_t>(index));
        }
        const std::array<std::uint16_t, kTitleIntroLandingPhaseCount> dx{{
            0x00F1U, 0x006EU, 0x0071U, 0x002AU, 0x0000U, 0x0063U, 0x0000U}};
        const std::array<std::uint16_t, kTitleIntroLandingPhaseCount> dy{{
            0x0071U, 0x0000U, 0x0071U, 0x0071U, 0x0038U, 0x0038U, 0x0000U}};
        const std::array<std::uint16_t, kTitleIntroLandingPhaseCount> delay{{
            0x0024U, 0x005AU, 0x0048U, 0x0012U, 0x0024U, 0x0012U, 0x0000U}};
        for (std::size_t index = 0; index < kTitleIntroLandingPhaseCount;
             ++index) {
            // 0x5B1B6 uses the base 0x1DF3F4 list throughout. It selects the
            // alternate 0x1DF3FE list only while the computed Y is in
            // 0x110..0x11E; phase index alone never changes the artwork.
            result.landing_phases[index] = {
                sprite(OpeningIntroSpriteFamily::aircraft_movement,
                       0U),
                dx[index], dy[index], delay[index]};
        }

        // 0x5A080 selects six compressed 0x1728 wilderness name-table
        // streams. The offline importer turns each into an independently
        // named source-region replacement; the conversion retains only the
        // source cadence and host descriptor.
        result.overlay_phases = {{
            {sprite(OpeningIntroSpriteFamily::wilderness_overlay, 0), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::wilderness_overlay, 1), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::wilderness_overlay, 2), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::wilderness_overlay, 3), 0x0018U},
            {sprite(OpeningIntroSpriteFamily::wilderness_overlay, 4), 0x0018U},
            {sprite(OpeningIntroSpriteFamily::wilderness_overlay, 5), 0x000CU},
        }};
        result.final_overlay_descriptor =
            sprite(OpeningIntroSpriteFamily::wilderness_final_overlay, 0);
        result.wilderness_map_descriptors = {{
            sprite(OpeningIntroSpriteFamily::wilderness_map_left, 0),
            sprite(OpeningIntroSpriteFamily::wilderness_map_center, 1),
            sprite(OpeningIntroSpriteFamily::wilderness_map_right, 2),
        }};

        result.orchestrator_assets.text_assets = {
            opening_intro_text_handle(OpeningIntroTextAsset::tri_vid_recovered),
            opening_intro_text_handle(OpeningIntroTextAsset::national_news),
            opening_intro_text_handle(OpeningIntroTextAsset::brother_last_die),
        };
        return result;
    }();
    return plan;
}

namespace {

void emit_stage_sprite(const OpeningIntroCallbacks& callbacks,
                       const TitleIntroStageSpriteRequest& request) {
    emit_sprite(callbacks, request.sprite_descriptor, request.x, request.y,
                request.palette_flags);
}

void emit_late_sprite(const OpeningIntroCallbacks& callbacks,
                      const TitleIntroSpriteRequest& request) {
    emit_sprite(callbacks, request.sprite_descriptor, request.x, request.y,
                request.palette_flags);
}

void emit_positioned_sprite(const OpeningIntroCallbacks& callbacks,
                            const TitleIntroPositionedSpriteRequest& request) {
    emit_sprite(callbacks, request.sprite_descriptor, request.x, request.y,
                request.palette_flags);
}

void emit_persistent_sprite(
    const OpeningIntroCallbacks& callbacks,
    const TitleIntroPersistentSpriteRequest& request) {
    emit_sprite(callbacks, request.sprite_descriptor, request.x, request.y,
                request.palette_flags);
}

void emit_sweep_sprite(const OpeningIntroCallbacks& callbacks,
                       const TitleIntroSweepSpriteRequest& request) {
    emit_sprite(callbacks, request.sprite_descriptor, request.x, request.y,
                request.palette_flags);
}

void emit_final_sprite(const OpeningIntroCallbacks& callbacks,
                       const TitleIntroFinalSpriteRequest& request) {
    emit_sprite(callbacks, request.sprite_descriptor, request.x, request.y,
                request.palette_flags);
}

void begin_stage2(OpeningIntroState& state,
                  const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks, OpeningIntroStage::stage2_palette_fade_out);
    invoke(callbacks.prepare_mode_entry);
    if (callbacks.service_audio_command) {
        callbacks.service_audio_command(0x11U);
    }
    source_boundary(callbacks, "0x59D7C.begin_stage2");
    invoke(callbacks.fade_palette_to_black);
    state.scene_palette_fade_vblank = 0U;
    state.scene_palette_fade_step = 7U;
    state.scene_palette_weight = 255U;
}

bool advance_scene_palette_fade(OpeningIntroState& state,
                                const bool to_black) {
    if (state.scene_palette_fade_vblank <
        kStage2PaletteTransitionVblanks) {
        ++state.scene_palette_fade_vblank;
        if ((state.scene_palette_fade_vblank & 1U) == 0U) {
            if (to_black) {
                if (state.scene_palette_fade_step > 0U) {
                    --state.scene_palette_fade_step;
                }
            } else if (state.scene_palette_fade_step < 7U) {
                ++state.scene_palette_fade_step;
            }
            state.scene_palette_weight = static_cast<std::uint8_t>(
                static_cast<std::uint16_t>(state.scene_palette_fade_step) *
                255U / 7U);
        }
    }
    if (state.scene_palette_fade_vblank <
        kStage2PaletteTransitionVblanks) {
        return false;
    }
    state.scene_palette_fade_step = to_black ? 0U : 7U;
    state.scene_palette_weight = to_black ? 0U : 255U;
    return true;
}

void install_stage2_scene(OpeningIntroState& state,
                          const OpeningIntroSourcePlan& plan,
                          const OpeningIntroCallbacks& callbacks) {
    set_scene(state, callbacks, OpeningIntroScene::wilderness);
    // 0x1406 clears the old planes only after its 17 x two-VBlank palette
    // fade has completed, then installs the new resource and target palette.
    invoke(callbacks.clear_presentation);
    source_boundary(callbacks, "0x59D7C.load_title_font");
    for (const auto descriptor : plan.wilderness_map_descriptors) {
        emit_sprite(callbacks, descriptor, 0U, 0U, 0U);
    }
    source_boundary(callbacks, "0x59D9C.install_wilderness_map_layers");
    state.schedule = {};
    state.schedule.current_entry = 0;
    state.schedule.countdown = plan.stage2_schedule.empty()
                                   ? 0
                                   : plan.stage2_schedule.front().delay_frames;
    state.stage2 = {};
    state.stage2.current_frame = static_cast<std::uint16_t>(state.source_frame);
    state.stage2.x_offset = -0x0040;
    state.stage2.y_offset = -0x0020;
    state.stage2.x_phase = 0x0138;
    state.stage2.y_phase = 0x00F1;
    set_title_intro_entity_color_words(state.entity_colors, 0U, 0x0EEEU);
    set_title_intro_entity_color_words(state.entity_colors, 1U, 0x0EEEU);
    source_boundary(callbacks, "0x59D7C.prepare_title_resources");

    advance_title_intro_schedule(
        state.schedule, plan.stage2_schedule,
        [&callbacks](const std::uint32_t action_id) {
            emit_schedule_action(action_id, callbacks);
        });
    emit_sprite(callbacks,
                opening_intro_sprite_handle(
                    OpeningIntroSpriteFamily::ambush_actor, 0),
                static_cast<std::uint16_t>(state.stage2.x_phase),
                static_cast<std::uint16_t>(state.stage2.y_phase), 0x2000U);
    source_boundary(callbacks, "0x5A37C.render_wilderness_actor");
    // 0x12F6 advances toward the newly installed target palette. The native
    // state exposes its 34 VBlank wait steps so the scene fades in visibly.
    invoke(callbacks.advance_palette_fade);
    state.stage2_first_loop_frame = 0;
    state.scene_palette_fade_vblank = 0U;
    state.scene_palette_fade_step = 0U;
    state.scene_palette_weight = 0U;
    set_stage(state, callbacks, OpeningIntroStage::stage2_palette_fade_in);
}

void reset_stage2_animation(OpeningIntroState& state,
                            const OpeningIntroCallbacks& callbacks) {
    state.stage2.animation_start_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.animation = {};
    state.overlay = {};
    state.sequence = {};
    state.persistent = {};
    state.motion = {};
    state.dual = {};
    state.timed = {};
    state.targeted_sweep = {};
    state.linear_sweep_a = {};
    state.linear_sweep_b = {};
    state.final_projection = {};
    reset_animation_children(state);
    source_boundary(callbacks, "0x59ED8-0x59F0E.reset_intro_animation_children");
}

void advance_animation_services(OpeningIntroState& state,
                                const OpeningIntroSourcePlan& plan,
                                const OpeningIntroCallbacks& callbacks) {
    state.overlay.current_frame = static_cast<std::uint16_t>(state.source_frame);
    state.overlay.animation_start_frame = state.stage2.animation_start_frame;
    state.sequence.current_frame = static_cast<std::uint16_t>(state.source_frame);
    state.persistent.current_frame = static_cast<std::uint16_t>(state.source_frame);
    state.motion.current_frame = static_cast<std::uint16_t>(state.source_frame);
    state.motion.animation_start_frame = state.stage2.animation_start_frame;
    state.dual.current_frame = static_cast<std::uint16_t>(state.source_frame);
    state.timed.current_frame = static_cast<std::uint16_t>(state.source_frame);
    state.targeted_sweep.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.linear_sweep_a.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.linear_sweep_b.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.final_projection.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.final_projection.animation_start_frame =
        state.stage2.animation_start_frame;

    TitleIntroAnimationCallbacks animation_callbacks{};
    animation_callbacks.advance_persistent_sequence = [&] {
        advance_title_intro_persistent_sequence(
            state.persistent, plan.persistent_phases,
            {[&callbacks](const std::uint16_t event) {
                 if (callbacks.service_audio_command) {
                     callbacks.service_audio_command(event);
                 }
             },
             [&state, &callbacks](const std::uint16_t, const std::uint16_t) {
                 set_title_intro_entity_color_words(
                     state.entity_colors, 1U, 0x0EEEU);
                 // ROM 0x5A252 writes 4 to the same 0xFFF146 control read
                 // by 0x5A4C2. The source spell flashes end as this actor's
                 // hit/death palette sequence begins.
                 stop_title_intro_dual_sprites(state.dual);
                 source_boundary(callbacks,
                                 "0x5A2D0.persistent_stage_four_transition");
             },
             [&state, &callbacks] {
                 step_title_intro_entity_color_words(state.entity_colors, 1U);
                 source_boundary(callbacks,
                                 "0x5A328.persistent_repeated_stage");
             },
             [&state, &callbacks](const std::uint16_t x, const std::uint16_t y) {
                 source_boundary(callbacks,
                                 "0x5A338.persistent_stage_nine_anchor");
                 begin_title_intro_timed_sprites(state.timed, x, y);
             },
             [&callbacks](const auto&) {
                 if (callbacks.clear_wilderness_right_map) {
                     callbacks.clear_wilderness_right_map();
                 }
                 source_boundary(callbacks,
                                 "0x5A350.persistent_final_overlay");
             },
             [&callbacks](const auto& request) {
                 emit_persistent_sprite(callbacks, request);
             }});
    };
    animation_callbacks.advance_motion = [&] {
        advance_title_intro_motion(
            state.motion, plan.motion_phases,
            // 0x5A45C adds FFF124 >> 8, the 0x59FFC pan accumulator that
            // 0x59FB0 does not clear [C: emulator +1311..+1337 muzzle shot].
            state.overlay.schedule.scroll_accumulator,
            {[&callbacks](const auto& request) {
                 emit_positioned_sprite(callbacks, request);
             }});
    };
    animation_callbacks.advance_dual_sprites = [&] {
        advance_title_intro_dual_sprites(
            state.dual, plan.dual_sprites,
            {[&callbacks](const auto& request) {
                 emit_positioned_sprite(callbacks, request);
             },
             [&callbacks](const auto& request) {
                 emit_positioned_sprite(callbacks, request);
             }});
    };
    animation_callbacks.advance_timed_sprites = [&] {
        advance_title_intro_timed_sprites(
            state.timed, plan.timed_sprites,
            {[&callbacks](const auto& request) {
                 emit_positioned_sprite(callbacks, request);
             }});
    };
    animation_callbacks.advance_targeted_sweep = [&] {
        advance_title_intro_targeted_sweep(
            state.targeted_sweep, plan.targeted_sweep_phases,
            [&callbacks](const auto& request) {
                emit_sweep_sprite(callbacks, request);
            });
    };
    animation_callbacks.advance_linear_sweep_a = [&] {
        advance_title_intro_linear_sweep_a(
            state.linear_sweep_a, plan.linear_sweep_a_phases,
            [&callbacks](const auto& request) {
                emit_sweep_sprite(callbacks, request);
            });
    };
    animation_callbacks.advance_linear_sweep_b = [&] {
        advance_title_intro_linear_sweep_b(
            state.linear_sweep_b, plan.linear_sweep_b_phases,
            [&callbacks](const auto& request) {
                emit_sweep_sprite(callbacks, request);
            });
    };
    animation_callbacks.advance_final_projection = [&] {
        advance_title_intro_final_projection(
            state.final_projection, plan.final_projection_phases,
            [&callbacks](const auto& request) {
                emit_final_sprite(callbacks, request);
            });
    };
    advance_title_intro_animation(state.animation, animation_callbacks);
}

void advance_stage2_scene_frame(OpeningIntroState& state,
                                const OpeningIntroSourcePlan& plan,
                                const OpeningIntroCallbacks& callbacks) {
    state.stage2.current_frame = static_cast<std::uint16_t>(state.source_frame);
    advance_title_intro_schedule(
        state.schedule, plan.stage2_schedule,
        [&callbacks](const std::uint32_t action_id) {
            emit_schedule_action(action_id, callbacks);
        });
    if (state.stage2.x_offset != 0) {
        ++state.stage2.x_offset;
        if (state.stage2.x_phase != 0x0178U) {
            ++state.stage2.x_phase;
        }
    }
    if (state.stage2.y_offset != 0 && (state.source_frame & 1U) == 0U) {
        ++state.stage2.y_offset;
        if (state.stage2.y_phase != 0x00D1U) {
            --state.stage2.y_phase;
        }
    }
    emit_sprite(callbacks,
                opening_intro_sprite_handle(
                    OpeningIntroSpriteFamily::ambush_actor, 0),
                static_cast<std::uint16_t>(state.stage2.x_phase),
                static_cast<std::uint16_t>(state.stage2.y_phase), 0x2000U);
    source_boundary(callbacks, "0x5A37C.render_wilderness_actor");
}

void install_orchestrator(OpeningIntroState& state,
                          const OpeningIntroSourcePlan& plan,
                          const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks, OpeningIntroStage::orchestrator_entry);
    set_scene(state, callbacks, OpeningIntroScene::trivid);
    invoke(callbacks.clear_presentation);
    source_boundary(callbacks, "0x5AA96.reset_upload_and_clear_planes");
    source_boundary(callbacks, "0x5AA96.decode_trivid_source_streams");
    render_title_intro_stage_text_batch(
        plan.orchestrator_assets.text_assets,
        [&callbacks](const auto& request) { emit_title_intro_text(callbacks, request); });
    callbacks.wait_for_vblank();
    ++state.source_frame;
    source_boundary(callbacks, "0x5AA96.copy_trivid_palette");
    fill_title_intro_word_block(
        {0x0786U, 20U, 13U, 0x0080U, 0x07FFU},
        [&callbacks](const auto& request) { emit_stage_fill(callbacks, request); });

    state.orchestrator = {};
    state.orchestrator.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.orchestrator.animation_start_frame =
        static_cast<std::uint16_t>(state.source_frame);
    reset_title_intro_stage_projection(state.orchestrator.projection_state);
    reset_title_intro_stage_timed_sprites(state.orchestrator.timed_state);
    state.orchestrator.timed_state.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.orchestrator.timed_state.animation_start_frame =
        static_cast<std::uint16_t>(state.source_frame);
    service_title_intro_stage_frame(
        state.orchestrator.frame_state, state.orchestrator.timed_state,
        plan.stage_timed_phases,
        opening_intro_sprite_handle(OpeningIntroSpriteFamily::trivid_left, 0),
        state.orchestrator.projection_state,
        opening_intro_sprite_handle(OpeningIntroSpriteFamily::trivid_right, 0),
        [&callbacks](const auto& request) { emit_stage_sprite(callbacks, request); });

    callbacks.wait_for_vblank();
    ++state.source_frame;
    emit_stage_stream(callbacks, {});
    // 0x5AB4E: 0x12F6 steps from the still-installed wilderness palette to
    // the tri-vid target, not from black, so the tri-vid is visible at once;
    // the 34 VBlanks still elapse before the loop [C: emulator trace].
    invoke(callbacks.advance_palette_fade);
    state.trivid_color_steps = 0U;
    state.scene_palette_fade_vblank = 0U;
    state.scene_palette_fade_step = 7U;
    state.scene_palette_weight = 255U;
    set_stage(state, callbacks,
              OpeningIntroStage::orchestrator_palette_fade_in);
}

void begin_orchestrator(OpeningIntroState& state,
                        const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks,
              OpeningIntroStage::orchestrator_palette_fade_out);
    invoke(callbacks.prepare_mode_entry);
    if (callbacks.service_audio_command) {
        callbacks.service_audio_command(0x12U);
    }
    source_boundary(callbacks, "0x5AA96.begin_trivid_orchestrator");
    // ROM 0x5AA96 has no 0x1406/0x131C: after 0x53126 it clears both planes
    // (0x0FFE/0x102A), so the wilderness cuts to black while the tri-vid
    // streams decode [C: disassembly 0x5AA96..0x5AABA, emulator trace].
    // The stage completes on its next frame without a palette fade.
    state.scene_palette_fade_vblank = kStage2PaletteTransitionVblanks;
    state.scene_palette_fade_step = 7U;
    state.scene_palette_weight = 255U;
}

void advance_orchestrator_frame(OpeningIntroState& state,
                                const OpeningIntroSourcePlan& plan,
                                const OpeningIntroCallbacks& callbacks) {
    begin_source_frame(state, callbacks);
    sample_and_latch(callbacks);
    state.orchestrator.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.orchestrator.timed_state.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    service_title_intro_stage_frame(
        state.orchestrator.frame_state, state.orchestrator.timed_state,
        plan.stage_timed_phases,
        opening_intro_sprite_handle(OpeningIntroSpriteFamily::trivid_left, 0),
        state.orchestrator.projection_state,
        opening_intro_sprite_handle(OpeningIntroSpriteFamily::trivid_right, 0),
        [&callbacks](const auto& request) { emit_stage_sprite(callbacks, request); });
    advance_title_intro_stage_service_steps(
        state.orchestrator.current_frame,
        state.orchestrator.animation_start_frame,
        [&state, &callbacks](const std::uint16_t palette_line) {
            // One service step darkens palette lines 0..3; count it once.
            if (palette_line == 0U &&
                state.trivid_color_steps < kTrividColorStepsToBlack) {
                ++state.trivid_color_steps;
            }
            source_boundary(callbacks, "0x5AB96.service_trivid_color_step");
        });
}

void install_stage_loop(OpeningIntroState& state,
                        const OpeningIntroSourcePlan& plan,
                        const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks, OpeningIntroStage::stage_loop_entry);
    set_scene(state, callbacks, OpeningIntroScene::airport);
    invoke(callbacks.clear_presentation);
    source_boundary(callbacks, "0x5AD1A.prepare_airport_resources");
    state.stage_loop = {};
    // DD08 is shared across the intro stages. Preserve its final typed value
    // from stage 2; 0x5AD1A resets only the one-shot fill flag.
    state.stage_loop.scroll.x_offset = state.stage2.x_offset;
    state.stage_loop.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.stage_loop.animation_start_frame =
        static_cast<std::uint16_t>(state.source_frame);
    reset_title_intro_scroll_fill(state.stage_loop.scroll);
    reset_title_intro_landing(state.stage_loop.landing);
    state.stage_loop.landing.animation_start_frame =
        state.stage_loop.animation_start_frame;
    state.stage_loop.pulse.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.stage_loop.landing.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    emit_sprite(callbacks,
                opening_intro_sprite_handle(
                    OpeningIntroSpriteFamily::aircraft_plane_a, 0),
                0x0128U, 0x0070U, 0x2000U);
    service_title_intro_animation(
        state.stage_loop.animation_service, state.stage_loop.pulse,
        plan.pulse_descriptors, state.stage_loop.landing,
        plan.landing_phases,
        opening_intro_sprite_handle(OpeningIntroSpriteFamily::aircraft_movement,
                                    1),
        [&callbacks](const auto& request) { emit_late_sprite(callbacks, request); });
    render_stage_loop_middle_text(callbacks);
    source_boundary(callbacks, "0x5AD4A.restore_airport_scene_records");
    callbacks.sample_controller();
    state.stage_loop_frame = 0;
    // The airport resource set is installed only after the source fade has
    // completed. Its first host frame uses the newly installed target colors.
    state.scene_palette_fade_vblank = kStage2PaletteTransitionVblanks;
    state.scene_palette_fade_step = 7U;
    state.scene_palette_weight = 255U;
    set_stage(state, callbacks, OpeningIntroStage::stage_loop);
}

void begin_stage_loop(OpeningIntroState& state,
                      const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks,
              OpeningIntroStage::stage_loop_palette_fade_out);
    source_boundary(callbacks, "0x5AD1A.begin_airport_stage");
    // ROM 0x5AD26 -> 0x1406 -> 0x131C fades the cockpit palette to black
    // before the airport assets replace it.
    invoke(callbacks.fade_palette_to_black);
    state.scene_palette_fade_vblank = 0U;
    state.scene_palette_fade_step = 7U;
    state.scene_palette_weight = 255U;
}

void advance_stage_loop_frame(OpeningIntroState& state,
                              const OpeningIntroSourcePlan& plan,
                              const OpeningIntroCallbacks& callbacks) {
    begin_source_frame(state, callbacks);
    sample_and_latch(callbacks);
    if (state.stage_loop_frame ==
        kTitleIntroStageFinalTextElapsedFrameIndex) {
        render_stage_loop_final_text(callbacks);
    }
    state.stage_loop.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.stage_loop.pulse.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    state.stage_loop.landing.current_frame =
        static_cast<std::uint16_t>(state.source_frame);
    advance_title_intro_triplet(state.stage_loop.triplet,
                                static_cast<std::uint16_t>(state.source_frame));
    advance_title_intro_scroll(
        state.stage_loop.scroll,
        [&callbacks](const auto& request) { emit_scroll_fill(callbacks, request); });
    advance_title_intro_scroll_fade(
        state.stage_loop.scroll,
        [&callbacks] { clear_stage_loop_center_text(callbacks); });
    // 0x5B100 reads the same DD08 horizontal scroll word advanced by
    // 0x5B00A. Keep the converted leaf's typed pulse state on that source
    // value instead of leaving the helicopter fixed at its entry position.
    state.stage_loop.pulse.x_offset = state.stage_loop.scroll.x_offset;
    emit_sprite(callbacks,
                opening_intro_sprite_handle(
                    OpeningIntroSpriteFamily::aircraft_plane_a, 0),
                0x0128U, 0x0070U, 0x2000U);
    service_title_intro_animation(
        state.stage_loop.animation_service, state.stage_loop.pulse,
        plan.pulse_descriptors, state.stage_loop.landing,
        plan.landing_phases,
        opening_intro_sprite_handle(OpeningIntroSpriteFamily::aircraft_movement,
                                    1),
        [&callbacks](const auto& request) { emit_late_sprite(callbacks, request); });
}

void sync_post_intro_cab(OpeningIntroState& state) {
    const auto timeline = post_intro_cab_timeline(state.post_intro_frame);
    state.post_intro_mode = timeline.mode;
    state.post_intro_phase = timeline.phase;
    state.post_intro_caption_visible = timeline.caption_visible;
}

void begin_post_intro(OpeningIntroState& state,
                      const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks, OpeningIntroStage::post_intro);
    set_scene(state, callbacks, OpeningIntroScene::cab_map);
    source_boundary(callbacks, "0x59D12.enter_post_intro_mode");
    invoke(callbacks.enter_post_intro_mode);
    invoke(callbacks.clear_presentation);
    state.scene_palette_weight = 255U;
    state.scene_palette_fade_step = 7U;
    state.post_intro_frame = 0;
    sync_post_intro_cab(state);
    source_boundary(callbacks, "0x13726.render_mode_selection");
}

OpeningIntroAdvanceResult finish_opening_start_exit(
    OpeningIntroState& state, const OpeningIntroCallbacks& callbacks) {
    // ROM 0x59CFE is reached only when 0x59C5E observes Start after one of its
    // interruptible child loops, or when the post-intro mode returns.
    invoke(callbacks.sample_controller);
    source_boundary(callbacks, "0x59CFE.sample_controller_input");
    invoke(callbacks.fade_palette_to_black);
    // 0x59D04 jsr 0x131C: a visible fade of the interrupted scene, music
    // still playing; 0x53126 follows [C: emulator Start at +300].
    state.start_exit_fade_active = true;
    state.scene_palette_fade_vblank = 0U;
    return OpeningIntroAdvanceResult::frame_advanced;
}

OpeningIntroAdvanceResult advance_start_exit_fade(
    OpeningIntroState& state, const OpeningIntroCallbacks& callbacks) {
    begin_held_source_frame(state, callbacks);
    if (!advance_scene_palette_fade(state, true)) {
        return OpeningIntroAdvanceResult::frame_advanced;
    }
    state.start_exit_fade_active = false;
    source_boundary(callbacks, "0x59CFE.step_palette_transition");
    invoke(callbacks.prepare_mode_entry);
    source_boundary(callbacks, "0x59CFE.prepare_mode_entry");
    set_stage(state, callbacks, OpeningIntroStage::interrupted);
    return OpeningIntroAdvanceResult::interrupted;
}

void finish_airport_stage(OpeningIntroState& state,
                          const OpeningIntroCallbacks& callbacks) {
    // 0x59C5E deliberately clears the airport loop's stop marker and always
    // enters the post-intro mode. Start can interrupt that later mode itself.
    invoke(callbacks.clear_stop_request);
    source_boundary(callbacks, "0x59CE4.clear_intro_stop_marker");
    invoke(callbacks.fade_palette_to_black);
    // 0x59CEA's 0x131C is a visible 17 x two-VBlank fade of the airport
    // scene; 0x59D12 starts only after it [C: emulator trace].
    set_stage(state, callbacks, OpeningIntroStage::post_intro_palette_fade_out);
    state.scene_palette_fade_vblank = 0U;
    state.scene_palette_fade_step = 7U;
    state.scene_palette_weight = 255U;
}

} // namespace

OpeningIntroStartResult begin_opening_intro(
    OpeningIntroState& state, const OpeningIntroCallbacks& callbacks) {
    if (!has_source_frame_callbacks(callbacks)) {
        return OpeningIntroStartResult::callback_unresolved;
    }

    state = OpeningIntroState{};
    const auto& plan = opening_intro_source_plan();
    state.stage = OpeningIntroStage::not_started;
    state.scene = OpeningIntroScene::seattle;
    if (callbacks.scene_changed) {
        callbacks.scene_changed(state.scene);
    }
    if (callbacks.clear_presentation) {
        callbacks.clear_presentation();
    }
    callbacks.sample_controller();
    if (callbacks.service_audio_command) {
        callbacks.service_audio_command(0x10U);
    }
    source_boundary(callbacks, "0x59C5E.begin_opening_intro");

    // This is the converted body of 0x5AF62's setup portion.
    state.schedule = {};
    state.schedule.current_entry = 0;
    state.schedule.countdown = plan.opening_schedule.empty()
                                   ? 0
                                   : plan.opening_schedule.front().delay_frames;
    state.schedule.dirty_flags = 0x0DU;
    // 0x5AF62 -> 0x1406: the first title resource set starts from black.
    invoke(callbacks.fade_palette_to_black);
    invoke(callbacks.clear_presentation);
    source_boundary(callbacks, "0x5AF62.clear_display_buffer");
    source_boundary(callbacks, "0x5AF62.prepare_title_resources");
    source_boundary(callbacks, "0x5AF62.load_title_font");
    state.stage = OpeningIntroStage::schedule;
    if (callbacks.stage_changed) {
        callbacks.stage_changed(state.stage);
    }
    (void)plan;
    return OpeningIntroStartResult::started;
}

OpeningIntroAdvanceResult advance_opening_intro_frame(
    OpeningIntroState& state, const OpeningIntroCallbacks& callbacks) {
    if (state.stage == OpeningIntroStage::not_started) {
        return OpeningIntroAdvanceResult::callback_unresolved;
    }
    if (state.stage == OpeningIntroStage::complete) {
        return OpeningIntroAdvanceResult::source_complete;
    }
    if (state.stage == OpeningIntroStage::interrupted) {
        return OpeningIntroAdvanceResult::interrupted;
    }
    if (state.start_exit_fade_active) {
        return advance_start_exit_fade(state, callbacks);
    }
    const auto& plan = opening_intro_source_plan();
    switch (state.stage) {
    case OpeningIntroStage::schedule: {
        if (state.seattle_schedule_exit_reported) {
            // The host held the schedule's still tail after the exit report;
            // this frame only continues 0x59C5E (no source VBlank of its own,
            // matching the hold frames).
            begin_stage2(state, callbacks);
            return OpeningIntroAdvanceResult::frame_advanced;
        }
        begin_source_frame(state, callbacks);
        sample_and_latch(callbacks);
        const auto schedule_complete = advance_title_intro_schedule(
            state.schedule, plan.opening_schedule,
            [&state, &callbacks](const std::uint32_t action_id) {
                emit_schedule_action(action_id, callbacks);
                if (action_id == 0x5B378U) {
                    state.seattle_plane_cleared = true;
                }
            });
        // 0x5B378's outer DBF from four holds its five visual passes for two
        // VBlanks each. The following 0x000F/0x49A0 schedule record keeps the
        // final small box on screen before the palette transition begins.
        if (state.seattle_plane_cleared &&
            state.seattle_outline_frame < kSeattleOutlineAnimationFrames) {
            ++state.seattle_outline_frame;
        }
        // 0x5AF62 samples and checks Start once per schedule pass. The caller
        // 0x59C5E then exits before entering stage 2.
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return finish_opening_start_exit(state, callbacks);
        }
        if (schedule_complete &&
            state.seattle_outline_frame >= kSeattleOutlineAnimationFrames) {
            // Report the schedule exit first so the host can hold the still
            // tail before 0x59C5E's 0x53126 starts the audio fade.
            state.seattle_schedule_exit_reported = true;
            source_boundary(callbacks, "0x5AF62.schedule_complete");
        }
        return OpeningIntroAdvanceResult::frame_advanced;
    }

    case OpeningIntroStage::stage2_palette_fade_out:
        begin_source_frame(state, callbacks);
        // 0x12F6 advances palette frames without sampling or latching input.
        if (advance_scene_palette_fade(state, true)) {
            install_stage2_scene(state, plan, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage2_palette_fade_in:
        begin_source_frame(state, callbacks);
        // The stage-2 reveal finishes before its VBlank input loop begins.
        emit_sprite(callbacks,
                    opening_intro_sprite_handle(
                        OpeningIntroSpriteFamily::ambush_actor, 0),
                    static_cast<std::uint16_t>(state.stage2.x_phase),
                    static_cast<std::uint16_t>(state.stage2.y_phase),
                    0x2000U);
        if (advance_scene_palette_fade(state, false)) {
            set_stage(state, callbacks, OpeningIntroStage::stage2_first_loop);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage2_first_loop:
        begin_source_frame(state, callbacks);
        sample_and_latch(callbacks);
        advance_stage2_scene_frame(state, plan, callbacks);
        if ((callbacks.stop_requested && callbacks.stop_requested()) ||
            state.stage2_first_loop_frame++ >= 0x00A0U) {
            // The source's first do-while exits on Start, but still initializes
            // and enters its second do-while, whose body runs once before the
            // shared stop marker is checked again.
            reset_stage2_animation(state, callbacks);
            callbacks.sample_controller();
            state.stage2_second_loop_frame = 0;
            set_stage(state, callbacks, OpeningIntroStage::stage2_second_loop);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage2_second_loop:
        begin_source_frame(state, callbacks);
        // The source order is overlay -> sequence -> controller -> schedule
        // -> eight animation services. Keep those calls visibly separate.
        state.overlay.current_frame = static_cast<std::uint16_t>(state.source_frame);
        state.overlay.animation_start_frame = state.stage2.animation_start_frame;
        advance_title_intro_overlay(
            state.overlay, plan.overlay_phases, plan.final_overlay_descriptor,
            {[&callbacks](const std::uint16_t event) {
                 if (callbacks.service_audio_command) {
                     callbacks.service_audio_command(event);
                 }
             },
             [&callbacks] {
                 source_boundary(callbacks, "0x59F8E.write_transfer_header");
             },
             [&callbacks] {
                 if (callbacks.clear_wilderness_left_region) {
                     callbacks.clear_wilderness_left_region();
                 }
                 source_boundary(callbacks,
                                 "0x59F8E.clear_wilderness_left_region");
             },
             [&callbacks](const auto& request) {
                 emit_overlay_stream(callbacks, request);
             }});
        // 0x59FFC-0x5A00C pans Plane A right (0xAA/256 px per frame) from
        // elapsed 0x120, and 0x59FB0 resets it at 0x168 [C: emulator
        // FFDD08 +1252..+1323].
            if (state.overlay.horizontal_scroll_written) {
            state.stage2.x_offset =
                static_cast<std::int16_t>(state.overlay.horizontal_scroll);
        }
        state.sequence.current_frame = static_cast<std::uint16_t>(state.source_frame);
        advance_title_intro_sequence(
            state.sequence, plan.sequence_phases,
            {[&callbacks](const auto& request) {
                 emit_sequence_stream(callbacks, request);
             },
             [&state, &callbacks](const std::uint16_t, const std::uint16_t) {
                 set_title_intro_entity_color_words(
                     state.entity_colors, 0U, 0x0EEEU);
                 source_boundary(callbacks, "0x5A124.sequence_stage_four_transition");
             },
             [&state, &callbacks] {
                 step_title_intro_entity_color_words(state.entity_colors, 0U);
                 source_boundary(callbacks, "0x5A16E.sequence_repeated_stage");
             },
             [&state, &callbacks](const std::uint16_t x, const std::uint16_t y) {
                 source_boundary(callbacks, "0x5A196.sequence_stage_nine_anchor");
                 begin_title_intro_timed_sprites(state.timed, x, y);
             },
             [&callbacks](const auto&) {
                 if (callbacks.clear_wilderness_center_map) {
                     callbacks.clear_wilderness_center_map();
                 }
                 source_boundary(callbacks, "0x5A1BE.sequence_final_overlay");
             },
             [&callbacks](const std::uint16_t event) {
                 if (callbacks.service_audio_command) {
                     callbacks.service_audio_command(event);
                 }
             }});
        sample_and_latch(callbacks);
        advance_title_intro_schedule(
            state.schedule, plan.stage2_schedule,
            [&callbacks](const std::uint32_t action_id) {
                emit_schedule_action(action_id, callbacks);
            });
        advance_animation_services(state, plan, callbacks);
        if (state.stage2_second_loop_frame ==
            kTitleIntroStage2StoryTextElapsedFrameIndex) {
            render_stage2_story_text(callbacks);
        }
        // 0x59D7C checks its marker after the second loop body, then returns
        // to 0x59C5E which takes the shared exit at 0x59CFE.
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return finish_opening_start_exit(state, callbacks);
        }
        if (state.stage2_second_loop_frame++ >= 0x01F0U) {
            begin_orchestrator(state, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::orchestrator_palette_fade_out:
        begin_source_frame(state, callbacks);
        // Resource preparation and the palette transition do not latch input.
        if (advance_scene_palette_fade(state, true)) {
            install_orchestrator(state, plan, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::orchestrator_palette_fade_in:
        // Inside 0x12F6 the VDP keeps the SAT 0x5AA96 uploaded, so the
        // tri-vid figures are visible from the first frame [C: emulator].
        begin_held_source_frame(state, callbacks);
        // 0x5AA96 samples before the loop, then latches only inside its loop.
        if (advance_scene_palette_fade(state, false)) {
            // 0x5AB48/0x5AB54 save and restore FFDF88 around 0x12F6, so the
            // fade-in VBlanks never reach the 0x5AB96 elapsed-frame test.
            state.source_frame -= kStage2PaletteTransitionVblanks;
            callbacks.sample_controller();
            state.orchestrator_loop_frame = 0;
            set_stage(state, callbacks, OpeningIntroStage::orchestrator_loop);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::orchestrator_loop:
        advance_orchestrator_frame(state, plan, callbacks);
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return finish_opening_start_exit(state, callbacks);
        }
        if (state.orchestrator_loop_frame++ >= 0x0114U) {
            begin_stage_loop(state, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage_loop_palette_fade_out:
        begin_source_frame(state, callbacks);
        // The airport resource transition runs before its input loop.
        if (advance_scene_palette_fade(state, true)) {
            install_stage_loop(state, plan, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage_loop:
        advance_stage_loop_frame(state, plan, callbacks);
        if ((callbacks.stop_requested && callbacks.stop_requested()) ||
            state.stage_loop_frame++ >= kTitleIntroStageLoopLastFrame) {
            finish_airport_stage(state, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::post_intro:
        begin_source_frame(state, callbacks);
        sample_and_latch(callbacks);
        // 0x134D2 checks Start at the end of each mode-selection frame.
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return finish_opening_start_exit(state, callbacks);
        }
        ++state.post_intro_frame;
        if (state.post_intro_frame >= kPostIntroCabTotalFrames) {
            // 0x134D2 returns into 0x59CFE: sample, then a visible 0x131C
            // fade of the cab map before 0x53126 [C: emulator trace].
            callbacks.sample_controller();
            source_boundary(callbacks, "0x59CFE.sample_controller_input");
            invoke(callbacks.fade_palette_to_black);
            set_stage(state, callbacks, OpeningIntroStage::exit_palette_fade_out);
            state.scene_palette_fade_vblank = 0U;
            state.scene_palette_fade_step = 7U;
            state.scene_palette_weight = 255U;
            return OpeningIntroAdvanceResult::frame_advanced;
        }
        sync_post_intro_cab(state);
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::post_intro_palette_fade_out:
        begin_held_source_frame(state, callbacks);
        if (advance_scene_palette_fade(state, true)) {
            source_boundary(callbacks, "0x59CEA.step_palette_transition");
            source_boundary(callbacks, "0x59CF0.publish_post_intro_mode");
            begin_post_intro(state, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::exit_palette_fade_out:
        begin_held_source_frame(state, callbacks);
        if (advance_scene_palette_fade(state, true)) {
            source_boundary(callbacks, "0x59CFE.step_palette_transition");
            invoke(callbacks.prepare_mode_entry);
            source_boundary(callbacks, "0x59CFE.prepare_mode_entry");
            set_stage(state, callbacks, OpeningIntroStage::complete);
            return OpeningIntroAdvanceResult::source_complete;
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::orchestrator_entry:
    case OpeningIntroStage::stage_loop_entry:
        return OpeningIntroAdvanceResult::callback_unresolved;
    case OpeningIntroStage::not_started:
    case OpeningIntroStage::complete:
        return OpeningIntroAdvanceResult::source_complete;
    case OpeningIntroStage::interrupted:
        return OpeningIntroAdvanceResult::interrupted;
    }
    return OpeningIntroAdvanceResult::callback_unresolved;
}

} // namespace shadowrun::ghidra_conversion::integration

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

void sample_and_latch(const OpeningIntroCallbacks& callbacks) {
    callbacks.sample_controller();
    callbacks.latch_controller_events();
}

void emit_schedule_action(const std::uint32_t action_id,
                          const OpeningIntroCallbacks& callbacks) {
    // These are the four entries of 0x5ADD0. The IDs remain provenance
    // tokens at this boundary; the host never calls a ROM address.
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
        source_boundary(callbacks, "0x5AEEE.render_team_record");
        emit_text(callbacks, OpeningIntroTextAsset::team_cover, 0x2304U,
                  0xA000U);
        break;
    case 0x5B378U:
        // 0x5B378 jsr 0xFFE clears Plane A, then stamps 0x1DF468 at
        // D4=0x40E through five 0x5B36E offsets. Plane B seattle stays.
        source_boundary(callbacks, "0x5B378.install_aircraft_scene");
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
    case OpeningIntroSpriteFamily::trivid_overlay:
        return "intro.sprites.trivid.overlay." + variant + ".";
    case OpeningIntroSpriteFamily::trivid_final_overlay:
        return "intro.sprites.trivid.final.overlay." + variant + ".";
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
    constexpr std::array<std::uint16_t, 5> kOffsets{
        {0x0000U, 0x0184U, 0x030AU, 0x0410U, 0x0514U}};
    constexpr std::array<std::uint8_t, 5> kD1{
        {0x17U, 0x11U, 0x0CU, 0x06U, 0x00U}};
    constexpr std::array<std::uint8_t, 5> kD2{
        {0x0EU, 0x0BU, 0x08U, 0x05U, 0x00U}};
    const auto index = static_cast<std::size_t>(
        phase < kOffsets.size() ? phase : kOffsets.size() - 1U);
    const auto vram = static_cast<std::uint16_t>(
        kSeattleOutlineVram + kOffsets[index]);
    return {static_cast<int>(((vram % 0x80U) / 2U) * 8U),
            static_cast<int>((vram / 0x80U) * 8U),
            static_cast<int>(kD1[index] + 1U) * 8,
            static_cast<int>(kD2[index] + 1U) * 8};
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

std::string_view opening_intro_text(const OpeningIntroTextAsset asset) {
    switch (asset) {
    case OpeningIntroTextAsset::time:
        return "The Time:  January 31, 2058";
    case OpeningIntroTextAsset::place:
        return "The Place: Seattle, UCAS";
    case OpeningIntroTextAsset::team_cover:
        return "Under the cover of darkness, a team";
    case OpeningIntroTextAsset::team_runners:
        return "of shadowrunners moves through";
    case OpeningIntroTextAsset::team_wilderness:
        return "the Salish-Shidhe wilderness.";
    case OpeningIntroTextAsset::without_warning:
        return "Without warning, they are";
    case OpeningIntroTextAsset::ambushed:
        return "ambushed by unknown forces.";
    case OpeningIntroTextAsset::massacre:
        return "The massacre is over quickly, but";
    case OpeningIntroTextAsset::captured:
        return "captured on tri-vid by one of the";
    case OpeningIntroTextAsset::slain:
        return "slain member's cybereyes.";
    case OpeningIntroTextAsset::tri_vid_recovered:
        return "The tri-vid was recovered and made";
    case OpeningIntroTextAsset::national_news:
        return "national news.  Michael, your";
    case OpeningIntroTextAsset::brother_last_die:
        return "brother, was the last to die.";
    case OpeningIntroTextAsset::spending:
        return "Spending your last Nuyen to go to";
    case OpeningIntroTextAsset::vow:
        return "Seattle, you vow to avenge your";
    case OpeningIntroTextAsset::brother_death:
        return "brother's death.";
    case OpeningIntroTextAsset::landing:
        return "Landing at Sea-Tac Airport,";
    case OpeningIntroTextAsset::flag_cab:
        return "you flag a cab to bring you to";
    case OpeningIntroTextAsset::last_credstick:
        return "Michael's last Credstick";
    case OpeningIntroTextAsset::transaction:
        return "transaction.";
    case OpeningIntroTextAsset::motel:
        return "Stoker's Coffin Motel, located";
    case OpeningIntroTextAsset::redmond:
        return "in the Redmond Barrens.";
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
    case OpeningIntroStage::stage2_entry:
        return "stage2_entry";
    case OpeningIntroStage::stage2_first_loop:
        return "stage2_first_loop";
    case OpeningIntroStage::stage2_second_loop:
        return "stage2_second_loop";
    case OpeningIntroStage::orchestrator_entry:
        return "orchestrator_entry";
    case OpeningIntroStage::orchestrator_loop:
        return "orchestrator_loop";
    case OpeningIntroStage::stage_loop_entry:
        return "stage_loop_entry";
    case OpeningIntroStage::stage_loop:
        return "stage_loop";
    case OpeningIntroStage::post_intro:
        return "post_intro";
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

        // 0x5ADD0: the four timed records that establish the date, place,
        // team, then hand the screen to 0x5B378.
        result.opening_schedule = {
            {0x003CU, action(0x5AEC6U)},
            {0x0078U, action(0x5AEDAU)},
            {0x00B4U, action(0x5AEEEU)},
            {0x00F0U, action(0x5B378U)},
        };
        result.stage2_schedule = {{0x0001U, action(0x5AE9EU)}};

        // 0x5A1D2: the four individually extracted wilderness background
        // patches used by the four-entry stage sequence.
        result.sequence_phases = {{
            {sprite(OpeningIntroSpriteFamily::background_patch, 0), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::background_patch, 1), 0x0018U},
            {sprite(OpeningIntroSpriteFamily::background_patch, 2), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::background_patch, 3), 0x003CU},
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

        const std::array<std::uint16_t, kTitleIntroLinearSweepPhaseCount>
            linear_a_y{{0x00ACU, 0x00E6U, 0x00BBU, 0x0016U, 0x00B1U,
                       0x002AU, 0x00ACU, 0x0016U}};
        const std::array<std::uint16_t, kTitleIntroLinearSweepPhaseCount>
            linear_b_y{{0x0020U, 0x00D7U, 0x0094U, 0x00A6U, 0x000CU,
                       0x00E2U, 0x00C4U, 0x00F8U}};
        const std::array<std::uint16_t, kTitleIntroLinearSweepPhaseCount>
            linear_a_delays{{0x0036U, 0x0048U, 0x0036U, 0x0048U, 0x0036U,
                             0x0048U, 0x0036U, 0x0048U}};
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
            result.landing_phases[index] = {
                sprite(OpeningIntroSpriteFamily::aircraft_movement,
                       static_cast<std::uint8_t>(index >= 4U ? 1U : 0U)),
                dx[index], dy[index], delay[index]};
        }

        // 0x5A080 selects six compressed 0x1728 name-table streams. The
        // offline importer turns each into a transparent, independently
        // named tri-vid overlay; the conversion retains only the source
        // cadence and host descriptor.
        result.overlay_phases = {{
            {sprite(OpeningIntroSpriteFamily::trivid_overlay, 0), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::trivid_overlay, 1), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::trivid_overlay, 2), 0x000CU},
            {sprite(OpeningIntroSpriteFamily::trivid_overlay, 3), 0x0018U},
            {sprite(OpeningIntroSpriteFamily::trivid_overlay, 4), 0x0018U},
            {sprite(OpeningIntroSpriteFamily::trivid_overlay, 5), 0x000CU},
        }};
        result.final_overlay_descriptor =
            sprite(OpeningIntroSpriteFamily::trivid_final_overlay, 0);

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
                  const OpeningIntroSourcePlan& plan,
                  const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks, OpeningIntroStage::stage2_entry);
    set_scene(state, callbacks, OpeningIntroScene::wilderness);
    invoke(callbacks.prepare_mode_entry);
    if (callbacks.service_audio_command) {
        callbacks.service_audio_command(0x11U);
    }
    source_boundary(callbacks, "0x59D7C.begin_stage2");

    // 0x59D9C -> 0x1406: prepare_title_resources first fades the current
    // palette toward black before the wilderness resources replace it.
    invoke(callbacks.fade_palette_to_black);
    invoke(callbacks.clear_presentation);
    source_boundary(callbacks, "0x59D7C.load_title_font");
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
    source_boundary(callbacks, "0x59D7C.prepare_title_resources");

    const auto render_scene = [&callbacks, &state] {
        emit_sprite(callbacks,
                    opening_intro_sprite_handle(
                        OpeningIntroSpriteFamily::ambush_actor, 0),
                    static_cast<std::uint16_t>(state.stage2.x_phase),
                    static_cast<std::uint16_t>(state.stage2.y_phase),
                    0x2000U);
        source_boundary(callbacks, "0x5A37C.render_wilderness_actor");
    };
    advance_title_intro_schedule(
        state.schedule, plan.stage2_schedule,
        [&callbacks](const std::uint32_t action_id) {
            emit_schedule_action(action_id, callbacks);
        });
    render_scene();
    // 0x59E0E: reveal the wilderness target palette after the initial scene
    // records have been installed.
    invoke(callbacks.advance_palette_fade);
    state.stage2_first_loop_frame = 0;
    set_stage(state, callbacks, OpeningIntroStage::stage2_first_loop);
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
             [&callbacks](const std::uint16_t, const std::uint16_t) {
                 source_boundary(callbacks,
                                 "0x5A2D0.persistent_stage_four_transition");
             },
             [&callbacks] {
                 source_boundary(callbacks,
                                 "0x5A328.persistent_repeated_stage");
             },
             [&state, &callbacks](const std::uint16_t x, const std::uint16_t y) {
                 source_boundary(callbacks,
                                 "0x5A338.persistent_stage_nine_anchor");
                 begin_title_intro_timed_sprites(state.timed, x, y);
             },
             [&callbacks](const auto&) {
                 source_boundary(callbacks,
                                 "0x5A350.persistent_final_overlay");
             },
             [&callbacks](const auto& request) {
                 emit_persistent_sprite(callbacks, request);
             }});
    };
    animation_callbacks.advance_motion = [&] {
        advance_title_intro_motion(
            state.motion, plan.motion_phases, state.scroll_accumulator,
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

void begin_orchestrator(OpeningIntroState& state,
                        const OpeningIntroSourcePlan& plan,
                        const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks, OpeningIntroStage::orchestrator_entry);
    set_scene(state, callbacks, OpeningIntroScene::trivid);
    invoke(callbacks.prepare_mode_entry);
    if (callbacks.service_audio_command) {
        callbacks.service_audio_command(0x12U);
    }
    source_boundary(callbacks, "0x5AA96.begin_trivid_orchestrator");
    // 0x5AA96 -> 0x1406: begin the tri-vid resource set from black.
    invoke(callbacks.fade_palette_to_black);
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
    // 0x5AB4E: reveal the tri-vid target palette after its first vblank.
    invoke(callbacks.advance_palette_fade);
    callbacks.sample_controller();
    state.orchestrator_loop_frame = 0;
    set_stage(state, callbacks, OpeningIntroStage::orchestrator_loop);
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
        [&callbacks](const std::uint16_t) {
            source_boundary(callbacks, "0x5AB96.service_trivid_color_step");
        });
}

void begin_stage_loop(OpeningIntroState& state,
                      const OpeningIntroSourcePlan& plan,
                      const OpeningIntroCallbacks& callbacks) {
    set_stage(state, callbacks, OpeningIntroStage::stage_loop_entry);
    set_scene(state, callbacks, OpeningIntroScene::airport);
    source_boundary(callbacks, "0x5AD1A.begin_airport_stage");
    // 0x5AD26 -> 0x1406: prepare_title_resources fades the previous scene
    // toward black before the airport resources are installed.
    invoke(callbacks.fade_palette_to_black);
    invoke(callbacks.clear_presentation);
    source_boundary(callbacks, "0x5AD1A.prepare_airport_resources");
    state.stage_loop = {};
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
    set_stage(state, callbacks, OpeningIntroStage::stage_loop);
}

void advance_stage_loop_frame(OpeningIntroState& state,
                              const OpeningIntroSourcePlan& plan,
                              const OpeningIntroCallbacks& callbacks) {
    begin_source_frame(state, callbacks);
    sample_and_latch(callbacks);
    if (state.stage_loop_frame == kTitleIntroStageFinalTextFrame) {
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
        [&callbacks] { render_stage_loop_final_text(callbacks); });
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
    state.post_intro_frame = 0;
    sync_post_intro_cab(state);
    source_boundary(callbacks, "0x13726.render_mode_selection");
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
    const auto& plan = opening_intro_source_plan();
    if (callbacks.stop_requested && callbacks.stop_requested()) {
        set_stage(state, callbacks, OpeningIntroStage::interrupted);
        return OpeningIntroAdvanceResult::interrupted;
    }

    switch (state.stage) {
    case OpeningIntroStage::schedule:
        begin_source_frame(state, callbacks);
        sample_and_latch(callbacks);
        if (advance_title_intro_schedule(
                state.schedule, plan.opening_schedule,
                [&state, &callbacks](const std::uint32_t action_id) {
                    emit_schedule_action(action_id, callbacks);
                    if (action_id == 0x5B378U) {
                        state.seattle_plane_cleared = true;
                    }
                })) {
            // 0x5B378's outer dbf from 4 plus two vblanks per pass is ten
            // frames of Plane-A outline over the still-live seattle plate.
            if (state.seattle_outline_frame < kSeattleOutlineHoldFrames) {
                ++state.seattle_outline_frame;
            } else {
                begin_stage2(state, plan, callbacks);
            }
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage2_first_loop:
        begin_source_frame(state, callbacks);
        sample_and_latch(callbacks);
        advance_stage2_scene_frame(state, plan, callbacks);
        if (state.stage2_first_loop_frame++ >= 0x00A0U) {
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
             [&callbacks](const auto&) {
                 source_boundary(callbacks, "0x59F8E.draw_transition_sprite");
             },
             [&callbacks](const auto& request) {
                 emit_overlay_stream(callbacks, request);
             }});
        state.sequence.current_frame = static_cast<std::uint16_t>(state.source_frame);
        advance_title_intro_sequence(
            state.sequence, plan.sequence_phases,
            {[&callbacks](const auto& request) {
                 emit_sequence_stream(callbacks, request);
             },
             [&callbacks](const std::uint16_t, const std::uint16_t) {
                 source_boundary(callbacks, "0x5A124.sequence_stage_four_transition");
             },
             [&callbacks] {
                 source_boundary(callbacks, "0x5A16E.sequence_repeated_stage");
             },
             [&state, &callbacks](const std::uint16_t x, const std::uint16_t y) {
                 source_boundary(callbacks, "0x5A196.sequence_stage_nine_anchor");
                 begin_title_intro_timed_sprites(state.timed, x, y);
             },
             [&callbacks](const auto&) {
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
        if (state.stage2_second_loop_frame == 0x0164U) {
            render_stage2_story_text(callbacks);
        }
        if (state.stage2_second_loop_frame++ >= 0x01F0U) {
            begin_orchestrator(state, plan, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::orchestrator_loop:
        advance_orchestrator_frame(state, plan, callbacks);
        if (state.orchestrator_loop_frame++ >= 0x0114U) {
            begin_stage_loop(state, plan, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage_loop:
        advance_stage_loop_frame(state, plan, callbacks);
        if (state.stage_loop_frame++ >= kTitleIntroStageLoopLastFrame) {
            source_boundary(callbacks, "0x59CDE.clear_intro_stop_marker");
            invoke(callbacks.fade_palette_to_black);
            source_boundary(callbacks, "0x59CF0.publish_post_intro_mode");
            begin_post_intro(state, callbacks);
        }
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::post_intro:
        begin_source_frame(state, callbacks);
        sample_and_latch(callbacks);
        ++state.post_intro_frame;
        if (state.post_intro_frame >= kPostIntroCabTotalFrames) {
            callbacks.sample_controller();
            invoke(callbacks.fade_palette_to_black);
            invoke(callbacks.prepare_mode_entry);
            set_stage(state, callbacks, OpeningIntroStage::complete);
            return OpeningIntroAdvanceResult::source_complete;
        }
        sync_post_intro_cab(state);
        return OpeningIntroAdvanceResult::frame_advanced;

    case OpeningIntroStage::stage2_entry:
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

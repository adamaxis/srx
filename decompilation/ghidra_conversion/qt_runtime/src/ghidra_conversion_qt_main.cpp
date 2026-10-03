#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"
#include "field_scene_runtime.hpp"
#include "genesis_image_compositor.hpp"

#include <QApplication>
#include <QColor>
#include <QElapsedTimer>
#include <QImage>
#include <QKeyEvent>
#include <QFont>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;
using namespace shadowrun::ghidra_conversion::qt_runtime;

constexpr int kSceneWidth = 320;
constexpr int kSceneHeight = 224;
constexpr int kSceneScale = 3;

std::filesystem::path find_asset_root(int argc, char** argv) {
    for (int index = 1; index < argc; ++index) {
        const std::string argument(argv[index]);
        if (argument == "--assets" && index + 1 < argc) {
            return std::filesystem::path(argv[++index]);
        }
        if (argument == "--help") {
            std::cout
                << "usage: shadowrun_ghidra_conversion_qt"
                   " [--assets <decompilation/assets>] [--field-smoke]"
                   " [--field-smoke-move-right] [--field-dump <file>]"
                   " [--field-command-dump <file>]"
                       " [--field-command-page <0..6>]"
                   " [--field-action-dump <output-dir>]"
                   " [--field-clips-dump <output-dir>]"
                   " [--field-archetype <samurai|decker|gator>]"
                   " [--field-hud-qa <output-dir>]"
                   " [--field-command-reopen-smoke]"
                   " [--field-dialogue-dump <prompt> <file>]"
                   " [--boot-dump-frame <frame> <file>]"
                   " [--startup-dump <stage> <selection> <frame> <file>]"
                   " [--startup-qa <output-dir>]"
                   " [--intro-to-field-smoke]\n";
            return {};
        }
        if (argument == "--field-smoke" ||
            argument == "--field-smoke-move-right" ||
            argument == "--field-command-reopen-smoke" ||
            argument == "--intro-to-field-smoke") {
            continue;
        }
        if (argument == "--startup-qa" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-command-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-action-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-clips-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-archetype" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-command-page" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-hud-qa" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-dialogue-dump" && index + 2 < argc) {
            index += 2;
            continue;
        }
        if (argument == "--boot-dump-frame" && index + 2 < argc) {
            index += 2;
            continue;
        }
        if (argument == "--startup-dump" && index + 4 < argc) {
            index += 4;
            continue;
        }
        if (argument == "--opening-dump-range" && index + 4 < argc) {
            index += 4;
            continue;
        }
        throw std::invalid_argument("unknown or incomplete argument: " +
                                    argument);
    }

    const std::vector<std::filesystem::path> candidates{
        "decompilation/assets", "assets", "../assets", "../../assets"};
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate / "resources.tsv")) {
            return candidate;
        }
    }
    return {};
}

std::optional<StokerMotelPromptId> parse_stoker_dialogue_prompt(
    const std::string_view name) {
    if (name == "arrival") {
        return StokerMotelPromptId::arrival;
    }
    if (name == "manager") {
        return StokerMotelPromptId::manager_greeting;
    }
    if (name == "manager-package") {
        return StokerMotelPromptId::manager_greeting_with_package;
    }
    if (name == "room") {
        return StokerMotelPromptId::room_offer;
    }
    if (name == "debt") {
        return StokerMotelPromptId::debt_question;
    }
    if (name == "leave") {
        return StokerMotelPromptId::leave_notice;
    }
    if (name == "package-question") {
        return StokerMotelPromptId::package_question;
    }
    if (name == "package-handoff") {
        return StokerMotelPromptId::package_handoff;
    }
    if (name == "paid") {
        return StokerMotelPromptId::room_paid;
    }
    if (name == "insufficient") {
        return StokerMotelPromptId::room_insufficient;
    }
    if (name == "jump-house") {
        return StokerMotelPromptId::jump_house_referral;
    }
    if (name == "ambush") {
        return StokerMotelPromptId::street_ambush;
    }
    return std::nullopt;
}

std::optional<std::uint8_t> parse_field_archetype(
    const std::string_view name) {
    if (name == "0" || name == "samurai") {
        return 0U;
    }
    if (name == "1" || name == "decker") {
        return 1U;
    }
    if (name == "2" || name == "gator" || name == "gator_shaman") {
        return 2U;
    }
    return std::nullopt;
}

std::uint8_t sampled_menu_byte(const int key) {
    switch (key) {
    case Qt::Key_Up:
        return kStartupMenuUp;
    case Qt::Key_Down:
        return kStartupMenuDown;
    case Qt::Key_Z:
        return kStartupMenuA;
    case Qt::Key_X:
        return kStartupMenuB;
    case Qt::Key_C:
        return kStartupMenuC;
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return kStartupMenuA;
    default:
        return 0U;
    }
}

std::uint8_t sampled_field_direction_bit(const int key) {
    switch (key) {
    case Qt::Key_Up:
        return 0x01U;
    case Qt::Key_Down:
        return 0x02U;
    case Qt::Key_Left:
        return 0x04U;
    case Qt::Key_Right:
        return 0x08U;
    default:
        return 0U;
    }
}

// 0x195C publishes Genesis port bit 7 as the high-byte Start edge. Startup
// menus deliberately treat Space/Enter as A for their own input contract, but
// the field coordinator must keep Start distinct because ROM 0x0B1A branches
// on 0x8000 after the edge expansion.
std::uint8_t sampled_field_button_byte(const int key) {
    switch (key) {
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return 0x80U;
    default:
        return sampled_menu_byte(key);
    }
}

QString phase_title(const StartupIntegrationPhase phase) {
    return QStringLiteral("Shadowrun - Ghidra Conversion Qt [%1]")
        .arg(QString::fromLatin1(startup_integration_phase_name(phase)));
}

int count_playfield_lit(const QImage& scene) {
    int lit = 0;
    for (int y = 80; y < 160; ++y) {
        for (int x = 80; x < 240; ++x) {
            const auto pixel = scene.pixelColor(x, y);
            if ((pixel.red() | pixel.green() | pixel.blue()) != 0) {
                ++lit;
            }
        }
    }
    return lit;
}

bool source_shell_pixels_survive(const QImage& scene, const QImage& shell,
                                 const std::initializer_list<QPoint>& points) {
    if (shell.isNull() || scene.size() != shell.size()) {
        return false;
    }
    for (const auto& point : points) {
        if (!scene.rect().contains(point) ||
            scene.pixel(point) != shell.pixel(point)) {
            return false;
        }
    }
    return true;
}

bool field_command_shell_survives(const QImage& scene,
                                  const std::filesystem::path& asset_root) {
    const QImage shell(QString::fromStdWString(
        (asset_root / "menu" / "field" / "background.bmp").wstring()));
    return source_shell_pixels_survive(
        scene, shell,
        {QPoint(10, 10), QPoint(100, 0), QPoint(120, 0),
         QPoint(10, 100), QPoint(300, 10), QPoint(300, 220)});
}

bool field_hud_chrome_survives(const QImage& scene,
                               const std::filesystem::path& asset_root) {
    const QImage shell(QString::fromStdWString(
        (asset_root / "field" / "hud" / "background.bmp").wstring()));
    return source_shell_pixels_survive(
        scene, shell,
        {QPoint(272, 0), QPoint(300, 0), QPoint(319, 0),
         QPoint(319, 32), QPoint(319, 80), QPoint(319, 160),
         QPoint(272, 223), QPoint(300, 223), QPoint(319, 223)});
}

bool field_hud_face_family_survives(const QImage& scene,
                                    const std::filesystem::path& asset_root,
                                    const std::size_t archetype_index) {
    // Every opaque pixel of the archetype's extracted 4x4 face family must
    // appear verbatim at 0x35D4's slot-0 stamp location (F1C6 -> 280,24).
    // Transparent family pixels may show the shell behind them. This is a
    // regression check against silently reusing the default archetype's
    // portrait, not a substitute for independent visual review.
    const QImage face(QString::fromStdWString(
        (asset_root / "field" / "hud" /
         ("face-" + std::to_string(archetype_index) + ".bmp")).wstring()));
    if (face.isNull() || face.size() != QSize(32, 32)) {
        return false;
    }
    constexpr int kFaceX = 280;
    constexpr int kFaceY = 24;
    int opaque = 0;
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            const auto source = face.pixelColor(x, y);
            if (source.alpha() == 0) {
                continue;
            }
            ++opaque;
            const auto scene_pixel = scene.pixelColor(kFaceX + x, kFaceY + y);
            if (scene_pixel != source) {
                return false;
            }
        }
    }
    return opaque > 64;
}

class GhidraConversionWindow final : public QWidget {
public:
    explicit GhidraConversionWindow(std::filesystem::path asset_root,
                                    QWidget* parent = nullptr)
        : QWidget(parent),
          asset_root_(std::move(asset_root)),
          field_scene_runtime_([this](const std::string_view boundary) {
              note_presentation_boundary(boundary);
          }) {
        setWindowTitle(QStringLiteral("Shadowrun - Ghidra Conversion Qt"));
        setFixedSize(kSceneWidth * kSceneScale, kSceneHeight * kSceneScale);
        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_OpaquePaintEvent);

        configure_startup_callbacks();
        const auto result = begin_startup_integration(
            asset_root_, state_, startup_callbacks_);
        if (result != StartupIntegrationStartResult::boot_in_progress) {
            throw std::runtime_error(
                "converted startup did not enter pre_title; phase=" +
                std::string(startup_integration_phase_name(state_.phase)));
        }

        load_direct_title_assets();
        configure_title_loop_callbacks();
        host_clock_.start();
        host_frame_timer_.setTimerType(Qt::PreciseTimer);
        host_frame_timer_.setInterval(16);
        QObject::connect(&host_frame_timer_, &QTimer::timeout, this, [this] {
            on_host_frame_timeout();
        });
        host_frame_timer_.start();
        setWindowTitle(phase_title(state_.phase));

    }

    int dump_boot_frame(const std::uint32_t target_frame,
                        const std::filesystem::path& output_path) {
        host_frame_timer_.stop();
        for (std::uint32_t frame = 0; frame < target_frame; ++frame) {
            if (state_.phase != StartupIntegrationPhase::pre_title) {
                std::cerr << "boot dump: source frame passed title handoff\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::pre_title) {
            std::cerr << "boot dump: source frame passed title handoff\n";
            return 3;
        }

        QImage scene(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        scene.fill(Qt::black);
        QPainter painter(&scene);
        draw_boot_scene(painter);
        painter.end();
        if (!scene.save(QString::fromStdWString(output_path.wstring()))) {
            std::cerr << "boot dump: could not write "
                      << output_path.string() << '\n';
            return 3;
        }
        std::cout << "boot_dump=written frame=" << target_frame
                  << " stage="
                  << static_cast<int>(state_.pre_title.presentation_stage)
                  << " path=" << output_path.string() << '\n';
        return 0;
    }

    // Deterministic visual-audit entry point. The phase transitions and
    // opening frames are still driven by the converted source contracts;
    // this only exposes their existing Qt presentation as an image dump.
    int prepare_opening_dump(const std::uint32_t selection) {
        host_frame_timer_.stop();
        if (selection > 2U) {
            std::cerr << "startup dump: selection must be 0, 1, or 2\n";
            return 3;
        }
        constexpr std::uint32_t kMaxSetupFrames = 10000U;
        std::uint32_t setup_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: boot did not reach title\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            std::cerr << "startup dump: phase after boot="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        while (state_.phase == StartupIntegrationPhase::title_ready) {
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kStartupMenuA) << 8U);
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: title did not accept Start\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::start_options) {
            std::cerr << "startup dump: phase after title="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            std::cerr << "startup dump: New Game did not reach archetype menu\n";
            return 3;
        }
        for (std::uint32_t index = 0U; index < selection; ++index) {
            advance_startup_integration(
                state_, kStartupMenuDown, startup_callbacks_);
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            std::cerr << "startup dump: archetype did not start opening\n";
            return 3;
        }
        return 0;
    }

    int prepare_startup_menu_dump(const std::uint32_t selection) {
        host_frame_timer_.stop();
        if (selection > 2U) {
            std::cerr << "startup dump: selection must be 0, 1, or 2\n";
            return 3;
        }
        constexpr std::uint32_t kMaxSetupFrames = 10000U;
        std::uint32_t setup_frames = 0U;
        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: boot did not reach title\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            std::cerr << "startup dump: phase after boot="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }
        while (state_.phase == StartupIntegrationPhase::title_ready) {
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kStartupMenuA) << 8U);
            if (setup_frames++ >= kMaxSetupFrames) {
                std::cerr << "startup dump: title did not accept Start\n";
                return 3;
            }
            advance_one_source_frame();
        }
        if (state_.phase != StartupIntegrationPhase::start_options) {
            std::cerr << "startup dump: phase after title="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }
        if (selection == 0U) {
            return 0;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            std::cerr << "startup dump: New Game did not reach archetype menu\n";
            return 3;
        }
        for (std::uint32_t index = 0U; index + 1U < selection; ++index) {
            advance_startup_integration(
                state_, kStartupMenuDown, startup_callbacks_);
        }
        return 0;
    }

    int dump_startup_scene(const std::string_view scene_name,
                           const std::uint32_t selection,
                           const std::uint32_t target_frame,
                           const std::filesystem::path& output_path) {
        if (scene_name != "start_options" && scene_name != "archetype" &&
            scene_name != "opening") {
            std::cerr << "startup dump: unknown stage " << scene_name
                      << " (expected start_options, archetype, or opening)\n";
            return 3;
        }
        if (scene_name == "start_options") {
            if (prepare_startup_menu_dump(0U) != 0) {
                return 3;
            }
            return write_startup_dump_image(
                scene_name, selection, target_frame, output_path);
        }
        if (scene_name == "archetype") {
            if (prepare_startup_menu_dump(selection + 1U) != 0) {
                return 3;
            }
            return write_startup_dump_image(
                scene_name, selection, target_frame, output_path);
        }
        if (prepare_opening_dump(selection) != 0) {
            return 3;
        }
        for (std::uint32_t frame = 0U; frame < target_frame; ++frame) {
            if (state_.phase != StartupIntegrationPhase::opening_intro) {
                std::cerr << "startup dump: opening ended before frame "
                          << target_frame << '\n';
                return 3;
            }
            advance_one_source_frame();
        }
        return write_startup_dump_image(
            scene_name, selection, target_frame, output_path);
    }

    // Render every opening frame from start to end-1 into one directory so
    // the conversion can be compared frame-by-frame against the 30fps
    // longplay dump instead of one frame at a time.
    int dump_opening_range(const std::uint32_t selection,
                           const std::uint32_t start, const std::uint32_t end,
                           const std::filesystem::path& output_directory) {
        if (prepare_opening_dump(selection) != 0) {
            return 3;
        }
        std::filesystem::create_directories(output_directory);
        std::ofstream boundary_log(output_directory / "boundaries.tsv");
        std::size_t next_boundary = 0U;
        for (std::uint32_t frame = 0U; frame < end; ++frame) {
            if (state_.phase != StartupIntegrationPhase::opening_intro) {
                std::cerr << "startup range dump: opening ended at frame "
                          << frame << '\n';
                return 3;
            }
            while (next_boundary < presentation_boundaries_.size()) {
                boundary_log << frame << '\t'
                             << presentation_boundaries_[next_boundary++]
                             << '\n';
            }
            if (frame >= start) {
                const auto scene = render_scene();
                const auto path = output_directory /
                                  ("frame-" + std::to_string(frame) + ".bmp");
                if (!scene.save(QString::fromStdWString(path.wstring()))) {
                    std::cerr << "startup range dump: could not write "
                              << path.string() << '\n';
                    return 3;
                }
            }
            advance_one_source_frame();
        }
        std::cout << "startup_range_dump=complete selection=" << selection
                  << " frames=" << start << '-' << end << " path="
                  << output_directory.string() << '\n';
        return 0;
    }

    int run_intro_to_field_smoke() {
        host_frame_timer_.stop();
        constexpr int kMaxFrames = 8000;
        int frames = 0;
        const auto pump = [this, &frames] {
            if (frames >= kMaxFrames) {
                return false;
            }
            advance_one_source_frame();
            ++frames;
            return true;
        };

        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (!pump()) {
                std::cerr << "intro-to-field: boot did not reach title\n";
                return 3;
            }
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            std::cerr << "intro-to-field: phase after boot="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        while (state_.phase == StartupIntegrationPhase::title_ready) {
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kStartupMenuA) << 8U);
            if (!pump()) {
                std::cerr << "intro-to-field: title did not accept Start\n";
                return 3;
            }
        }
        if (state_.phase != StartupIntegrationPhase::start_options) {
            std::cerr << "intro-to-field: phase after title="
                      << startup_integration_phase_name(state_.phase) << '\n';
            return 3;
        }

        advance_startup_integration(
            state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            std::cerr << "intro-to-field: New Game did not reach Joshua\n";
            return 3;
        }
        advance_startup_integration(
            state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            std::cerr << "intro-to-field: Joshua did not start opening\n";
            return 3;
        }
        while (state_.phase == StartupIntegrationPhase::opening_intro) {
            if (!pump()) {
                std::cerr << "intro-to-field: opening did not complete\n";
                return 3;
            }
        }
        if (state_.phase != StartupIntegrationPhase::field_ready ||
            !field_scene_runtime_.ready()) {
            std::cerr << "intro-to-field: field not ready phase="
                      << startup_integration_phase_name(state_.phase)
                      << " bootstrap=" << field_scene_runtime_.failure()
                      << '\n';
            return 3;
        }

        QImage scene(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        scene.fill(Qt::black);
        QPainter painter(&scene);
        field_scene_runtime_.paint(painter);
        painter.end();
        if (count_playfield_lit(scene) < 100) {
            std::cerr << "intro-to-field: playfield buried under opaque HUD\n";
            return 3;
        }
        if (!field_hud_chrome_survives(scene, asset_root_)) {
            std::cerr << "intro-to-field: field HUD chrome was not preserved\n";
            return 3;
        }
        const auto& buffers = field_scene_runtime_.state().field_scene_buffers;
        std::cout << "intro_to_field=ready scene_reference="
                  << buffers.scene_reference << " camera="
                  << buffers.camera_x << ',' << buffers.camera_y
                  << " field_position="
                  << field_scene_runtime_.field_entity_world_position().x
                  << ','
                  << field_scene_runtime_.field_entity_world_position().y
                  << " entity_visible="
                  << (field_scene_runtime_.field_entity_visible() ? 1 : 0)
                  << " opening_frames=" << frames << '\n';
        return 0;
    }

    int run_startup_qa(const std::filesystem::path& output_directory) {
        host_frame_timer_.stop();
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "startup-qa.tsv");
        if (!trace) {
            std::cerr << "startup qa: could not open trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "label\tphase\tboot_frame\topening_frame\tstage\tscene\n";

        const auto write_snapshot = [&](const std::string& name) {
            const auto path = output_directory / (name + ".bmp");
            const auto scene = render_scene();
            const auto is_startup_menu =
                state_.phase == StartupIntegrationPhase::start_options ||
                state_.phase == StartupIntegrationPhase::save_load ||
                state_.phase == StartupIntegrationPhase::joshua_archetype;
            if (is_startup_menu && !startup_shell_survives(scene)) {
                std::cerr << "startup qa: startup bezel/decal layer was "
                             "not preserved for "
                          << name << '\n';
                return false;
            }
            if (state_.phase == StartupIntegrationPhase::field_ready &&
                !field_hud_chrome_survives(scene, asset_root_)) {
                std::cerr << "startup qa: field HUD chrome was not preserved "
                             "for "
                          << name << '\n';
                return false;
            }
            if (!scene.save(QString::fromStdWString(path.wstring()))) {
                std::cerr << "startup qa: could not write " << path.string()
                          << '\n';
                return false;
            }
            return true;
        };
        const auto record = [&](const std::string& label) {
            trace << label << '\t'
                  << startup_integration_phase_name(state_.phase) << '\t'
                  << state_.frame.frame_count << '\t'
                  << state_.opening_intro.source_frame << '\t'
                  << opening_intro_stage_name(opening_stage_) << '\t'
                  << opening_intro_scene_name(opening_scene_) << '\n';
        };
        const auto fail = [&](const std::string_view message) {
            std::cerr << "startup qa: " << message << '\n';
            return 3;
        };

        record("boot.initial");
        if (!write_snapshot("00_boot_initial")) {
            return 3;
        }

        constexpr int kMaxFrames = 10000;
        int frames = 0;
        const auto pump = [&] {
            if (frames >= kMaxFrames) {
                return false;
            }
            advance_one_source_frame();
            ++frames;
            return true;
        };

        while (state_.phase == StartupIntegrationPhase::pre_title) {
            if (!pump()) {
                return fail("boot did not reach title");
            }
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            return fail("boot reached an unexpected phase");
        }
        record("title.ready");
        if (!write_snapshot("01_title_ready")) {
            return 3;
        }

        // The source title loop is interactive, but its complete logo/text
        // presentation is time-driven. Capture the progression before
        // pressing Start so the QA walk cannot accidentally validate only the
        // first black/circuit frame seen by the old smoke path.
        constexpr std::array<int, 4> kTitleCheckpoints{300, 900, 1500,
                                                        3000};
        int title_frame = 0;
        for (const auto checkpoint : kTitleCheckpoints) {
            while (title_frame < checkpoint &&
                   state_.phase == StartupIntegrationPhase::title_ready) {
                if (!pump()) {
                    return fail("title presentation did not advance");
                }
                ++title_frame;
            }
            if (state_.phase != StartupIntegrationPhase::title_ready) {
                return fail("title presentation left title-ready early");
            }
            record("title.checkpoint." + std::to_string(checkpoint));
            if (!write_snapshot("title_" + std::to_string(checkpoint))) {
                return 3;
            }
        }

        while (state_.phase == StartupIntegrationPhase::title_ready) {
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kStartupMenuA) << 8U);
            if (!pump()) {
                return fail("title did not accept Start");
            }
        }
        if (state_.phase != StartupIntegrationPhase::start_options) {
            return fail("title did not enter Start Options");
        }
        record("menu.start_options.new_game");
        if (!write_snapshot("02_start_options_new_game")) {
            return 3;
        }

        // Exercise the Load Game branch and its source-visible empty-slot
        // message before starting New Game. This is the path the old smoke
        // test skipped entirely.
        advance_startup_integration(
            state_, kStartupMenuDown, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::start_options ||
            state_.start_options.selected != StartOptionsChoice::load_game) {
            return fail("Start Options did not select Load Game");
        }
        record("menu.start_options.load_game");
        if (!write_snapshot("03_start_options_load_game")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::save_load) {
            return fail("Load Game did not enter Save/Load");
        }
        record("menu.save_load.slot_one");
        if (!write_snapshot("04_save_load_slot_one")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::save_load ||
            state_.save_load.message != SaveLoadMessage::no_save_data) {
            return fail("empty Save/Load slot did not publish no-save message");
        }
        record("menu.save_load.no_save_message");
        if (!write_snapshot("05_save_load_no_save_message")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::start_options) {
            return fail("no-save message did not return to Start Options");
        }
        record("menu.start_options.returned_from_load");
        if (!write_snapshot("06_start_options_returned_from_load")) {
            return 3;
        }

        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::joshua_archetype) {
            return fail("New Game did not enter archetype selection");
        }
        record("menu.archetype.samurai");
        if (!write_snapshot("07_archetype_samurai")) {
            return 3;
        }
        advance_startup_integration(
            state_, kStartupMenuDown, startup_callbacks_);
        if (state_.joshua_archetype.selected !=
            JoshuaArchetypeChoice::decker) {
            return fail("archetype selector did not reach Decker");
        }
        record("menu.archetype.decker");
        if (!write_snapshot("08_archetype_decker")) {
            return 3;
        }
        advance_startup_integration(
            state_, kStartupMenuDown, startup_callbacks_);
        if (state_.joshua_archetype.selected !=
            JoshuaArchetypeChoice::gator_shaman) {
            return fail("archetype selector did not reach Gator Shaman");
        }
        record("menu.archetype.gator_shaman");
        if (!write_snapshot("09_archetype_gator_shaman")) {
            return 3;
        }
        advance_startup_integration(state_, kStartupMenuA, startup_callbacks_);
        if (state_.phase != StartupIntegrationPhase::opening_intro) {
            return fail("Gator Shaman did not start opening intro");
        }
        record("opening.initial");
        if (!write_snapshot("10_opening_initial")) {
            return 3;
        }

        constexpr std::array<std::uint32_t, 10> kOpeningCheckpoints{
            300U, 600U, 900U, 1200U, 1500U,
            1800U, 2100U, 2400U, 2700U, 3000U};
        std::size_t next_checkpoint = 0U;
        auto previous_stage = opening_stage_;
        auto previous_scene = opening_scene_;
        while (state_.phase == StartupIntegrationPhase::opening_intro) {
            if (!pump()) {
                return fail("opening did not complete");
            }
            const auto opening_frame = state_.opening_intro.source_frame;
            if (next_checkpoint < kOpeningCheckpoints.size() &&
                opening_frame == kOpeningCheckpoints[next_checkpoint]) {
                const auto label = "opening.checkpoint." +
                                   std::to_string(opening_frame);
                record(label);
                if (!write_snapshot("opening_" +
                                    std::to_string(opening_frame))) {
                    return 3;
                }
                ++next_checkpoint;
            }
            if (opening_stage_ != previous_stage ||
                opening_scene_ != previous_scene) {
                const auto label =
                    "opening.transition." + std::to_string(opening_frame);
                record(label);
                const auto name =
                    "opening_transition_" + std::to_string(opening_frame) +
                    "_" + opening_intro_stage_name(opening_stage_) + "_" +
                    opening_intro_scene_name(opening_scene_);
                if (!write_snapshot(name)) {
                    return 3;
                }
                previous_stage = opening_stage_;
                previous_scene = opening_scene_;
            }
        }
        if (state_.phase != StartupIntegrationPhase::field_ready ||
            !field_scene_runtime_.ready()) {
            return fail("opening did not publish a ready field session");
        }
        record("field.ready");
        if (!write_snapshot("99_field_ready")) {
            return 3;
        }

        // The command overview must be a re-enterable field service, not a
        // one-shot presentation. Drive the same typed controller path as the
        // Qt window and retain both open frames as QA artifacts.
        if (!field_scene_runtime_.open_command_overview_for_dump()) {
            return fail("first field command overview open failed");
        }
        record("field.command_overview.open_1");
        if (!write_snapshot("100_field_command_open_1")) {
            return 3;
        }
        field_scene_runtime_.set_controller_port_byte(0U);
        field_scene_runtime_.advance_frame();
        field_scene_runtime_.set_controller_port_byte(0x80U);
        field_scene_runtime_.advance_frame();
        if (field_scene_runtime_.command_overview_open()) {
            return fail("field command overview did not close on Start");
        }
        record("field.command_overview.returned_to_field");
        if (!write_snapshot("101_field_after_command_close")) {
            return 3;
        }
        field_scene_runtime_.set_controller_port_byte(0U);
        field_scene_runtime_.advance_frame();
        if (!field_scene_runtime_.open_command_overview_for_dump()) {
            return fail("second field command overview open failed");
        }
        record("field.command_overview.open_2");
        if (!write_snapshot("102_field_command_open_2")) {
            return 3;
        }

        std::ofstream events(output_directory / "event-log.txt");
        for (const auto& event : state_.event_log) {
            events << event << '\n';
        }
        std::ofstream boundaries(output_directory / "presentation-boundaries.log");
        for (const auto& boundary : presentation_boundaries_) {
            boundaries << boundary << '\n';
        }
        std::cout << "startup_qa=passed output="
                  << output_directory.string() << " opening_frames="
                  << state_.opening_intro.source_frame << '\n';
        return 0;
    }

protected:
    void paintEvent(QPaintEvent*) override {
        const auto scene = render_scene();
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawImage(rect(), scene);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->isAutoRepeat() &&
            state_.phase != StartupIntegrationPhase::field_ready) {
            // 0x195C publishes newly-pressed bits. Qt auto-repeat is not a
            // new Genesis edge; treating it as one skipped Load's 0xD94 wait.
            event->accept();
            return;
        }
        if (state_.phase == StartupIntegrationPhase::field_ready) {
            const auto direction_bit =
                sampled_field_direction_bit(event->key());
            if (direction_bit != 0U) {
                field_controller_nibble_ = static_cast<std::uint8_t>(
                    field_controller_nibble_ | direction_bit);
                field_controller_port_byte_ = static_cast<std::uint8_t>(
                    (field_controller_port_byte_ & 0xF0U) |
                    field_controller_nibble_);
                field_scene_runtime_.set_controller_port_byte(
                    field_controller_port_byte_);
                update();
                event->accept();
                return;
            }
            const auto button_bits = static_cast<std::uint8_t>(
                sampled_field_button_byte(event->key()) &
                static_cast<std::uint8_t>(kStartupMenuActionMask | 0x80U));
            if (button_bits != 0U) {
                field_controller_port_byte_ = static_cast<std::uint8_t>(
                    field_controller_port_byte_ | button_bits);
                field_scene_runtime_.set_controller_port_byte(
                    field_controller_port_byte_);
                update();
                event->accept();
                return;
            }
        }

        const auto sample = sampled_menu_byte(event->key());
        if (sample == 0U) {
            QWidget::keyPressEvent(event);
            return;
        }

        if (state_.phase == StartupIntegrationPhase::title_ready) {
            // 0x195C publishes newly pressed controller bits in the high
            // byte. The converted title loop consumes that word at its next
            // vblank; it owns the exit decision.
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                pending_title_controller_word_ |
                (static_cast<std::uint16_t>(sample) << 8U));
        } else if (state_.phase == StartupIntegrationPhase::start_options ||
                   state_.phase == StartupIntegrationPhase::save_load ||
                   state_.phase == StartupIntegrationPhase::joshua_archetype) {
            advance_startup_integration(state_, sample, startup_callbacks_);
        }

        setWindowTitle(phase_title(state_.phase));
        update();
        event->accept();
    }

    void keyReleaseEvent(QKeyEvent* event) override {
        if (state_.phase == StartupIntegrationPhase::field_ready &&
            !event->isAutoRepeat()) {
            const auto direction_bit =
                sampled_field_direction_bit(event->key());
            if (direction_bit != 0U) {
                field_controller_nibble_ = static_cast<std::uint8_t>(
                    field_controller_nibble_ &
                    static_cast<std::uint8_t>(~direction_bit));
                field_controller_port_byte_ = static_cast<std::uint8_t>(
                    (field_controller_port_byte_ & 0xF0U) |
                    field_controller_nibble_);
                field_scene_runtime_.set_controller_port_byte(
                    field_controller_port_byte_);
                update();
                event->accept();
                return;
            }
            const auto button_bits = static_cast<std::uint8_t>(
                sampled_field_button_byte(event->key()) &
                static_cast<std::uint8_t>(kStartupMenuActionMask | 0x80U));
            if (button_bits != 0U) {
                field_controller_port_byte_ = static_cast<std::uint8_t>(
                    field_controller_port_byte_ &
                    static_cast<std::uint8_t>(~button_bits));
                field_scene_runtime_.set_controller_port_byte(
                    field_controller_port_byte_);
                update();
                event->accept();
                return;
            }
        }
        QWidget::keyReleaseEvent(event);
    }

private:
    bool startup_shell_survives(const QImage& scene) const {
        const auto shell = draw_asset_cache_.find("menu.startup.background");
        if (shell == draw_asset_cache_.end()) {
            return false;
        }
        return source_shell_pixels_survive(
            scene, shell->second,
            {QPoint(30, 30), QPoint(45, 20), QPoint(250, 20),
             QPoint(36, 100), QPoint(282, 100), QPoint(30, 190),
             QPoint(250, 190)});
    }

    QImage render_scene() {
        // Genesis rebuilds the visible frame during vblank. The host has no
        // SAT/VDP, so each tick composites into an opaque 320x224 buffer and
        // replaces the widget pixels. SourceOver onto WA_OpaquePaintEvent
        // would keep previous widget pixels in any alpha hole.
        QImage scene(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        scene.fill(Qt::black);

        QPainter scene_painter(&scene);
        if (state_.phase == StartupIntegrationPhase::pre_title) {
            draw_boot_scene(scene_painter);
        } else if (state_.phase == StartupIntegrationPhase::start_options ||
                   state_.phase == StartupIntegrationPhase::save_load ||
                   state_.phase == StartupIntegrationPhase::joshua_archetype) {
            draw_alias(scene_painter, "menu.startup.background");
            draw_menu_overlay(scene_painter);
        } else if (state_.phase == StartupIntegrationPhase::opening_intro) {
            draw_opening_scene(scene_painter);
        } else if (state_.phase == StartupIntegrationPhase::field_ready) {
            if (field_scene_runtime_.ready()) {
                field_scene_runtime_.paint(scene_painter);
            } else {
                draw_alias(scene_painter, "menu.startup.background");
                draw_conversion_boundary_error(scene_painter);
            }
        } else if (state_.phase == StartupIntegrationPhase::failed) {
            draw_alias(scene_painter, "menu.startup.background");
            draw_conversion_boundary_error(scene_painter);
        } else {
            draw_title(scene_painter);
        }
        scene_painter.end();
        return scene;
    }

    int write_startup_dump_image(
        const std::string_view scene_name, const std::uint32_t selection,
        const std::uint32_t target_frame,
        const std::filesystem::path& output_path) {
        const auto scene = render_scene();
        const auto is_startup_menu =
            state_.phase == StartupIntegrationPhase::start_options ||
            state_.phase == StartupIntegrationPhase::save_load ||
            state_.phase == StartupIntegrationPhase::joshua_archetype;
        if (is_startup_menu && !startup_shell_survives(scene)) {
            std::cerr << "startup dump: startup bezel/decal layer was not "
                         "preserved\n";
            return 3;
        }
        if (!scene.save(QString::fromStdWString(output_path.wstring()))) {
            std::cerr << "startup dump: could not write "
                      << output_path.string() << '\n';
            return 3;
        }
        std::cout << "startup_dump=written stage=" << scene_name
                  << " selection=" << selection
                  << " frame=" << target_frame
                  << " path=" << output_path.string() << '\n';
        return 0;
    }

    void configure_startup_callbacks() {
        startup_callbacks_.initialize_display = [this] {
            display_initialized_ = true;
        };
        startup_callbacks_.vdp_ready = {
            [this](const std::uint32_t delay) { last_source_delay_ = delay; },
            [this] { display_ready_ = true; },
            [this](const std::uint32_t delay) { last_source_delay_ = delay; },
        };

        startup_callbacks_.pre_title.resources.clear_plane_a = [this] {
            note_presentation_boundary("pre_title.clear_plane_a");
        };
        startup_callbacks_.pre_title.resources.clear_plane_b = [this] {
            note_presentation_boundary("pre_title.clear_plane_b");
        };
        startup_callbacks_.pre_title.resources.prepare_title_resources =
            [this] {
                note_presentation_boundary(
                    "pre_title.prepare_title_resources");
            };
        startup_callbacks_.pre_title.resources.write_resource_stream =
            [this](const auto& words) {
                resource_stream_words_ = words.size();
                note_presentation_boundary("pre_title.write_resource_stream");
            };
        startup_callbacks_.pre_title.resources.refresh_field_resource_streams =
            [this] {
                note_presentation_boundary(
                    "pre_title.refresh_field_resource_streams");
            };
        startup_callbacks_.pre_title.resources.decode_and_upload_source_stream =
            [this] {
                note_presentation_boundary(
                    "pre_title.decode_and_upload_source_stream");
            };
        startup_callbacks_.pre_title.resources.advance_title_palette_fade =
            [this] {
                ++palette_fade_steps_;
                note_presentation_boundary(
                    "pre_title.advance_title_palette_fade");
            };
        startup_callbacks_.pre_title.resources.write_pre_title_grid =
            [this](const PreTitleGridWriteRequest& request) {
                note_presentation_boundary(
                    request.purpose == PreTitleGridPurpose::resource_setup
                        ? "pre_title.write_resource_grid"
                        : "pre_title.write_final_grid");
            };
        startup_callbacks_.pre_title.resources.reset_vdp_upload_state =
            [this] {
                note_presentation_boundary(
                    "pre_title.reset_vdp_upload_state");
            };

        startup_callbacks_.pre_title.animation.publish_sprite_batch =
            [this](const PreTitleSpriteBatch& batch) {
                ++title_sprite_record_builds_;
                pre_title_sprite_records_ = batch.records;
                note_presentation_boundary(
                    "pre_title.publish_sprite_batch");
            };
        startup_callbacks_.pre_title.animation.clear_last_sprite_sequence =
            [this] {
                if (!pre_title_sprite_records_.empty()) {
                    pre_title_sprite_records_.back().sequence = 0U;
                }
                note_presentation_boundary(
                    "pre_title.clear_last_sprite_sequence");
            };
        startup_callbacks_.pre_title.animation.publish_logo_strobe_palette =
            [this](const PreTitleLogoStrobePalette& palette) {
                logo_strobe_phase_ = palette.phase;
                logo_strobe_palette_ = palette.color_words;
                note_presentation_boundary(
                    "pre_title.publish_logo_strobe_palette");
            };
        startup_callbacks_.pre_title.animation.step_entity_color_words =
            [this] {
                note_presentation_boundary(
                    "pre_title.step_entity_color_words");
            };
        startup_callbacks_.pre_title.wait_for_vblank = [this] {
            wait_for_vblank(
                state_.frame,
                [](const std::uint16_t previous) {
                    return static_cast<std::uint16_t>(previous + 1U);
                });
        };
        startup_callbacks_.pre_title.write_horizontal_scroll_stream =
            [this](const auto& scroll) {
                horizontal_scroll_lines_ = scroll.size();
            };
        startup_callbacks_.pre_title.publish_audio_request =
            [this](const PreTitleAudioRequest& request) {
                pre_title_audio_requests_.push_back(request);
                if (request.service_path ==
                    PreTitleAudioServicePath::enabled_variant_transaction) {
                    ++audio_variant_ticks_;
                } else {
                    ++audio_ticks_;
                }
                note_presentation_boundary(
                    request.service_path ==
                            PreTitleAudioServicePath::
                                enabled_variant_transaction
                        ? "pre_title.service_logo_effect_variant_audio"
                        : "pre_title.service_logo_effect_audio");
            };
        startup_callbacks_.pre_title.sample_controller = [this] {
            note_presentation_boundary("pre_title.sample_controller");
        };
        startup_callbacks_.pre_title.step_palette_transition =
            [this] {
                ++palette_transition_steps_;
                note_presentation_boundary(
                    "pre_title.step_palette_transition");
            };

        startup_callbacks_.title_setup = {
            [this] {
                load_direct_title_assets();
                note_presentation_boundary("title.prepare_scene_resources");
            },
            [this] {
                title_resources_loaded_ = true;
                note_presentation_boundary("title.load_title_resources");
            },
            [this] {
                cache_asset("menu.startup.font");
                title_font_loaded_ = true;
                note_presentation_boundary("title.load_title_font");
            },
            [this](const std::uint16_t frames) {
                title_palette_frames_ += frames;
                note_presentation_boundary("title.advance_palette_frames");
            },
        };

        // These are converted menu call boundaries. Qt invalidates the
        // presentation here; selection and phase changes remain owned by the
        // converted menu functions.
        startup_callbacks_.start_options = {
            [this] {
                note_presentation_boundary(
                    "start_options.copy_resource_metadata");
            },
            [this] {
                note_presentation_boundary(
                    "start_options.refresh_resource_metadata");
            },
            [this] {
                note_presentation_boundary(
                    "start_options.transfer_menu_tiles");
            },
            [this] {
                note_presentation_boundary("start_options.load_menu_font");
            },
            [this] {
                note_presentation_boundary(
                    "start_options.draw_new_load_grid");
            },
            [this] {
                note_presentation_boundary("start_options.draw_title");
            },
            [this](const StartOptionsChoice choice) {
                start_options_cursor_ = choice;
                note_presentation_boundary("start_options.draw_cursor");
            },
        };
        startup_callbacks_.save_load = {
            [this] {
                note_presentation_boundary("save_load.draw_grid");
            },
            [this] {
                note_presentation_boundary("save_load.draw_title");
            },
            [this](const SaveSlotChoice choice) {
                save_load_cursor_ = choice;
                note_presentation_boundary("save_load.draw_cursor");
            },
            [this](const SaveSlotChoice choice) {
                save_load_requested_slot_ = choice;
                // Host has no SRAM image. ROM 0x59456 carry-set (no valid
                // slot) publishes the 0x1B6527 no-save copy and returns to
                // Start Options after 0xD94.
                save_load_boundary_unresolved_ = false;
                return SaveLoadAttempt::empty_or_invalid;
            },
            [this](const SaveLoadMessage message) {
                save_load_message_ = message;
                note_presentation_boundary("save_load.draw_message");
            },
            [this] {
                note_presentation_boundary(
                    "save_load.wait_for_controller_event");
            },
        };
        startup_callbacks_.joshua_archetype = {
            [this] {
                note_presentation_boundary("archetype.draw_grid");
            },
            [this] {
                note_presentation_boundary("archetype.draw_title");
            },
            [this](const JoshuaArchetypeChoice choice) {
                joshua_cursor_ = choice;
                note_presentation_boundary("archetype.draw_cursor");
            },
            [this](const JoshuaArchetypeChoice choice) {
                joshua_preview_ = choice;
                note_presentation_boundary("archetype.preview_resources");
            },
            [this](const JoshuaArchetypeChoice choice) {
                selected_archetype_ = choice;
                note_presentation_boundary(
                    "archetype.initialize_joshua_record");
            },
        };
        startup_callbacks_.opening_intro.wait_for_vblank = [this] {
            wait_for_vblank(
                state_.frame,
                [](const std::uint16_t previous) {
                    return static_cast<std::uint16_t>(previous + 1U);
                });
        };
        startup_callbacks_.opening_intro.sample_controller = [this] {
            note_presentation_boundary("opening.sample_controller");
        };
        startup_callbacks_.opening_intro.latch_controller_events = [this] {
            note_presentation_boundary("opening.latch_controller_events");
        };
        startup_callbacks_.opening_intro.prepare_mode_entry = [this] {
            note_presentation_boundary("opening.prepare_mode_entry");
        };
        startup_callbacks_.opening_intro.service_audio_command =
            [this](const std::uint16_t command) {
                opening_audio_commands_.push_back(command);
                note_presentation_boundary("opening.service_audio_command");
            };
        startup_callbacks_.opening_intro.service_audio = [this] {
            note_presentation_boundary("opening.service_audio");
        };
        startup_callbacks_.opening_intro.fade_palette_to_black = [this] {
            ++palette_transition_steps_;
            note_presentation_boundary("opening.fade_palette_to_black");
        };
        startup_callbacks_.opening_intro.advance_palette_fade = [this] {
            ++palette_fade_steps_;
            note_presentation_boundary("opening.advance_palette_fade");
        };
        startup_callbacks_.opening_intro.enter_post_intro_mode = [this] {
            note_presentation_boundary("opening.enter_post_intro_mode");
        };
        startup_callbacks_.opening_intro.clear_presentation = [this] {
            opening_sprites_.clear();
            opening_wilderness_map_left_.reset();
            opening_wilderness_map_center_.reset();
            opening_wilderness_map_right_.reset();
            opening_wilderness_overlay_.reset();
            opening_wilderness_patch_.reset();
            opening_wilderness_final_overlay_.reset();
            opening_text_.clear();
            note_presentation_boundary("opening.clear_presentation");
        };
        startup_callbacks_.opening_intro.begin_frame = [this] {
            opening_sprites_.clear();
        };
        startup_callbacks_.opening_intro.stage_changed =
            [this](const OpeningIntroStage stage) {
                opening_stage_ = stage;
                note_presentation_boundary("opening.stage_changed");
            };
        startup_callbacks_.opening_intro.scene_changed =
            [this](const OpeningIntroScene scene) {
                opening_scene_ = scene;
                opening_wilderness_overlay_.reset();
                opening_wilderness_map_left_.reset();
                opening_wilderness_map_center_.reset();
                opening_wilderness_map_right_.reset();
                opening_wilderness_patch_.reset();
                opening_wilderness_final_overlay_.reset();
                opening_text_.clear();
                note_presentation_boundary("opening.scene_changed");
            };
        startup_callbacks_.opening_intro.draw_sprite =
            [this](const OpeningIntroSpriteRequest& request) {
                // These families are source name-table writes. Their pixels
                // persist until another write replaces the same region; a
                // host vblank must not reduce them to one-frame sprites.
                switch (opening_intro_sprite_group(request.sprite_descriptor)
                            .family) {
                case OpeningIntroSpriteFamily::wilderness_map_left:
                    opening_wilderness_map_left_ = request;
                    return;
                case OpeningIntroSpriteFamily::wilderness_map_center:
                    opening_wilderness_map_center_ = request;
                    return;
                case OpeningIntroSpriteFamily::wilderness_map_right:
                    opening_wilderness_map_right_ = request;
                    return;
                case OpeningIntroSpriteFamily::wilderness_overlay:
                    opening_wilderness_overlay_ = request;
                    return;
                case OpeningIntroSpriteFamily::background_patch:
                    opening_wilderness_patch_ = request;
                    return;
                case OpeningIntroSpriteFamily::wilderness_final_overlay:
                    opening_wilderness_final_overlay_ = request;
                    return;
                default:
                    opening_sprites_.push_back(request);
                    return;
                }
            };
        startup_callbacks_.opening_intro.clear_wilderness_right_map = [this] {
            opening_wilderness_map_right_.reset();
            note_presentation_boundary("opening.clear_wilderness_right_map");
        };
        startup_callbacks_.opening_intro.render_text =
            [this](const OpeningIntroTextRequest& request) {
                if (opening_intro_text_asset(request.text_asset) ==
                    OpeningIntroTextAsset::blank) {
                    constexpr std::uint16_t kOpeningPlaneBase = 0x2000U;
                    if (request.destination_word >= kOpeningPlaneBase) {
                        const auto row = static_cast<std::uint16_t>(
                            (request.destination_word - kOpeningPlaneBase) /
                            0x80U);
                        opening_text_.erase(
                            std::remove_if(
                                opening_text_.begin(), opening_text_.end(),
                                [row](const OpeningIntroTextRequest& current) {
                                    return current.destination_word >=
                                               kOpeningPlaneBase &&
                                           (current.destination_word -
                                            kOpeningPlaneBase) /
                                                   0x80U == row;
                                }),
                            opening_text_.end());
                    }
                    return;
                }
                const auto existing = std::find_if(
                    opening_text_.begin(), opening_text_.end(),
                    [&request](const OpeningIntroTextRequest& current) {
                        return current.destination_word ==
                               request.destination_word;
                    });
                if (existing == opening_text_.end()) {
                    opening_text_.push_back(request);
                } else {
                    *existing = request;
                }
            };
        startup_callbacks_.opening_intro.source_boundary =
            [this](const std::string_view boundary) {
                note_presentation_boundary(boundary);
            };
        startup_callbacks_.opening_intro.stop_requested = [] { return false; };
        startup_callbacks_.phase_changed =
            [this](const StartupIntegrationPhase phase) {
                setWindowTitle(phase_title(phase));
            };
        startup_callbacks_.field_ready = [this] {
            field_controller_nibble_ = 0U;
            field_controller_port_byte_ = 0U;
            if (!field_scene_runtime_.bootstrap(state_)) {
                std::cerr << "field bootstrap: "
                          << field_scene_runtime_.failure() << '\n';
            }
            field_scene_runtime_.set_controller_nibble(0U);
            update();
        };
    }

    void configure_title_loop_callbacks() {
        title_loop_callbacks_.wait_for_vblank = [this] { ++title_vblanks_; };
        title_loop_callbacks_.sample_controller = [this] {
            const auto sample = pending_title_controller_word_;
            pending_title_controller_word_ = 0U;
            return sample;
        };
        title_loop_callbacks_.apply_text_action =
            [this](const TitleTextAction action) {
                if (action == TitleTextAction::StepPaletteTowardTarget) {
                    ++title_palette_steps_;
                } else if (action == TitleTextAction::ShowCopyright) {
                    draw_title_copyright([this](const auto&) {
                        title_copyright_visible_ = true;
                    });
                }
            };
        title_loop_callbacks_.publish_circuit_light_phase =
            [this](const std::uint8_t phase) { title_circuit_phase_ = phase; };
        title_loop_callbacks_.publish_animation_offset =
            [this](const std::uint16_t offset) {
                title_animation_offset_ = offset;
            };
        title_loop_callbacks_.draw_prompt_variant =
            [this](const TitlePromptVariant variant) {
                title_prompt_variant_ = variant;
                const auto draw_prompt = [this](const auto&) {
                    title_prompt_visible_ = true;
                };
                if (variant == TitlePromptVariant::VariantB) {
                    draw_title_prompt_variant_b(draw_prompt);
                } else {
                    draw_title_prompt_variant_a(draw_prompt);
                }
            };
        title_loop_callbacks_.render_logo_column = [this](const auto&) {
            title_logo_columns_ = title_loop_.logo.columns_revealed;
        };
        title_loop_callbacks_.copy_completion_palette = [this] {
            title_completion_palette_copied_ = true;
        };
        title_loop_callbacks_.rebuild_sprite_projection = [this](const auto&) {
            title_sprite_phase_ = title_loop_.sprite.phase;
            title_sprite_x_ = title_loop_.sprite.x;
        };
        title_loop_callbacks_.publish_controller_events =
            [this](const auto& events) {
                title_controller_events_ = events.event_bits;
            };
        title_loop_callbacks_.reset_vdp_upload_state = [this] {
            note_presentation_boundary("title.reset_vdp_upload_state");
        };
        title_loop_callbacks_.step_palette_transition = [this] {
            ++title_palette_steps_;
        };
    }

    void on_host_frame_timeout() {
        const auto frames = consume_host_frames(
            host_frame_deadline_ns_, host_clock_.nsecsElapsed());
        for (int frame = 0; frame < frames; ++frame) {
            advance_one_source_frame();
        }
        if (frames > 0) {
            update();
        }
    }

    void advance_one_source_frame() {
        if (state_.phase == StartupIntegrationPhase::pre_title) {
            advance_startup_boot_frame(state_, startup_callbacks_);
            return;
        }
        if (state_.phase == StartupIntegrationPhase::opening_intro) {
            advance_startup_opening_frame(state_, startup_callbacks_);
            return;
        }
        if (state_.phase == StartupIntegrationPhase::field_ready) {
            field_scene_runtime_.advance_frame();
            return;
        }
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            return;
        }
        const auto result =
            advance_title_loop_frame(title_loop_, title_loop_callbacks_);
        if (result == TitleLoopFrameResult::start_requested) {
            enter_start_options_integration(state_, startup_callbacks_);
        }
    }

    void note_presentation_boundary(const std::string_view boundary) {
        presentation_boundaries_.emplace_back(boundary);
    }

    void load_direct_title_assets() {
        draw_asset_cache_.clear();
        for (const auto& alias : {
                 std::string("boot.snow"),
                 std::string("boot.sega.background"),
                 std::string("boot.guy"),
                 std::string("boot.fasa"),
                 std::string("title.background"),
                 std::string("title.logo"),
                 std::string("title.text"),
                 std::string("menu.startup.background"),
                 std::string("menu.startup.font"),
                 std::string("menu.startup.cursor")}) {
            cache_asset(alias);
        }
        for (int index = 0; index < 3; ++index) {
            cache_asset("menu.field.runner.portrait." + std::to_string(index));
        }
        for (int index = 0; index < 7; ++index) {
            cache_asset("title.circuit.background." + std::to_string(index));
        }
        for (int index = 0; index < 6; ++index) {
            cache_asset("title.sprite." + std::to_string(index));
            cache_asset("boot.sega.lightning." + std::to_string(index));
        }
        for (int index = 0; index < 8; ++index) {
            cache_asset("boot.sega.strobe." + std::to_string(index));
        }
        // The opening director publishes source SAT-list identities. Cache
        // every individually extracted intro record so a list is composed at
        // paint time; no full-frame intro bitmap is introduced here.
        for (const auto& record : state_.asset_catalog.catalog.records) {
            if (record.alias.rfind("intro.", 0) == 0) {
                cache_asset(record.alias);
            }
        }
    }

    void cache_asset(const std::string& alias) {
        const auto* record = state_.asset_catalog.catalog.find(alias);
        if (record == nullptr) {
            return;
        }
        QImage image(QString::fromStdWString(record->absolute_path.wstring()));
        if (!image.isNull()) {
            // Extracted sprites carry their baked color-zero alpha through
            // the V4 BMP alpha mask; no black-pixel wipe is applied.
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            draw_asset_cache_.emplace(
                alias, std::move(image));
        }
    }

    void draw_alias(QPainter& painter, const std::string_view alias,
                    const int dx = 0, const int dy = 0) const {
        const auto* record = state_.asset_catalog.catalog.find(alias);
        if (record == nullptr) {
            return;
        }
        const auto image = draw_asset_cache_.find(std::string(alias));
        if (image == draw_asset_cache_.end()) {
            return;
        }
        painter.drawImage(record->origin_x + dx, record->origin_y + dy,
                          image->second);
    }

    void draw_title(QPainter& painter) const {
        const auto circuit_alias =
            "title.circuit.background." +
            std::to_string(title_loop_.circuit_lights.phase);
        if (draw_asset_cache_.find(circuit_alias) != draw_asset_cache_.end()) {
            draw_alias(painter, circuit_alias);
        } else {
            draw_alias(painter, "title.background");
        }

        draw_title_logo(painter);
        draw_title_sprites(painter);
        draw_title_text(painter);
    }

    void draw_boot_snow(QPainter& painter) const {
        const auto image = draw_asset_cache_.find("boot.snow");
        if (image == draw_asset_cache_.end()) {
            return;
        }
        const auto& scroll = state_.pre_title.horizontal_scroll;
        for (int row = 0; row < image->second.height() &&
                          row < static_cast<int>(scroll.size());
             ++row) {
            const auto offset = std::min(
                static_cast<int>(scroll[static_cast<std::size_t>(row)]),
                image->second.width());
            if (offset >= image->second.width()) {
                continue;
            }
            painter.drawImage(0, row, image->second, offset, row,
                              image->second.width() - offset, 1);
        }
    }

    void draw_boot_scene(QPainter& painter) const {
        using PresentationStage =
            PreTitleSequenceState::PresentationStage;
        const auto stage = state_.pre_title.presentation_stage;
        if (stage == PresentationStage::fasa) {
            draw_alias(painter, "boot.fasa");
            return;
        }

        draw_boot_snow(painter);
        if (stage == PresentationStage::sega_strobe) {
            const auto phase = static_cast<int>(
                state_.pre_title.strobe_phase % 8U);
            draw_alias(painter,
                       "boot.sega.strobe." + std::to_string(phase));
        } else {
            draw_alias(painter, "boot.sega.background");
        }

        if (stage == PresentationStage::sega_shock) {
            const auto phase = static_cast<int>(
                state_.pre_title.sega_lightning_phase % 6U);
            draw_alias(painter,
                       "boot.sega.lightning." + std::to_string(phase));
        }

        const auto* record = state_.asset_catalog.catalog.find("boot.guy");
        const auto image = draw_asset_cache_.find("boot.guy");
        if (record != nullptr && image != draw_asset_cache_.end() &&
            state_.pre_title.figure_color_fade_weight != 0U) {
            const auto y = static_cast<int>(state_.pre_title.figure_y) -
                           0xD0 + record->origin_y;
            const auto figure = scale_genesis_image_rgb(
                image->second, state_.pre_title.figure_color_fade_weight);
            painter.drawImage(record->origin_x, y, figure);
        }
    }

    void draw_opening_scene(QPainter& painter) const {
        const auto& opening = state_.opening_intro;
        switch (opening_scene_) {
        case OpeningIntroScene::seattle:
            // 0x5B378 jsr 0xFFE clears Plane A. Plane B seattle stays.
            draw_alias(painter, "intro.background.0");
            if (opening.seattle_outline_frame != 0U) {
                const auto phase = static_cast<std::uint8_t>(
                    std::min<std::uint16_t>(
                        (opening.seattle_outline_frame - 1U) / 2U, 4U));
                draw_intro_outline(
                    painter, opening_intro_seattle_outline_plan(phase));
            }
            break;
        case OpeningIntroScene::wilderness:
            // ROM 0x59DA2 FFDD08 starts -0x40 and 0x59DBA FFDD0A starts
            // -0x20. Host blit origin is that HSCROLL/VSCROLL pair.
            draw_alias(painter, "intro.background.1",
                       opening.stage2.x_offset, -opening.stage2.y_offset);
            break;
        case OpeningIntroScene::trivid:
            draw_alias(painter, "intro.trivid.background");
            break;
        case OpeningIntroScene::airport:
            // background.3 is decoded at Plane-B VSCROLL 0x78. ROM FFDD0E
            // walks 0..0x78; host blit y = 0x78 - fade so the plate rises
            // into that settled crop.
            draw_alias(painter, "intro.background.3", 0,
                       0x78 - static_cast<int>(
                                  opening.stage_loop.scroll.fade_counter));
            break;
        case OpeningIntroScene::cab_map: {
            // The 26x17 map plate's plane origin is (16,16); the source
            // window (32,32) lands at the video's (62,30), so the plate
            // draws at screen (46,14) one-to-one. No selection outline is
            // baked into this background.
            const auto map = draw_asset_cache_.find("intro.cab.map");
            if (map != draw_asset_cache_.end()) {
                painter.drawImage(46, 14, map->second);
            }
            draw_cab_mode_selection(painter);
            break;
        }
            break;
        }

        // The ROM's wilderness animation streams update three persistent
        // name-table regions. The callback above records the latest typed
        // request for each region; redraw those source layers before the
        // one-vblank SAT requests below. This removes the composite actors
        // from the base plate at the same points where the Genesis map write
        // replaced them, without introducing a runtime frame flipbook.
        if (opening_scene_ == OpeningIntroScene::wilderness) {
            if (opening_wilderness_map_left_) {
                draw_opening_sprite(painter, *opening_wilderness_map_left_);
            }
            if (opening_wilderness_map_center_) {
                draw_opening_sprite(
                    painter, *opening_wilderness_map_center_);
            }
            if (opening_wilderness_map_right_) {
                draw_opening_sprite(painter, *opening_wilderness_map_right_);
            }
            if (opening_wilderness_overlay_) {
                draw_opening_sprite(painter, *opening_wilderness_overlay_);
            }
            if (opening_wilderness_patch_) {
                draw_opening_sprite(painter, *opening_wilderness_patch_);
            }
            if (opening_wilderness_final_overlay_) {
                draw_opening_sprite(
                    painter, *opening_wilderness_final_overlay_);
            }
        }

        for (const auto& request : opening_sprites_) {
            draw_opening_sprite(painter, request);
        }
        for (const auto& request : opening_text_) {
            draw_opening_text(painter, request);
        }
    }

    void draw_intro_outline(QPainter& painter, const int x, const int y,
                            const int width, const int height) const {
        const QColor outline(0xF0, 0xF0, 0xF0);
        constexpr int kTile = 8;
        if (width <= 0 || height <= 0) {
            return;
        }
        painter.fillRect(x, y, width, kTile, outline);
        painter.fillRect(x, y + height - kTile, width, kTile, outline);
        painter.fillRect(x, y, kTile, height, outline);
        painter.fillRect(x + width - kTile, y, kTile, height, outline);
    }

    void draw_intro_outline(QPainter& painter,
                            const OpeningIntroOutlinePlan& plan) const {
        draw_intro_outline(painter, plan.x, plan.y, plan.width, plan.height);
    }

    void draw_cab_mode_selection(QPainter& painter) const {
        const auto plan = cab_mode_selection_plan(
            state_.opening_intro.post_intro_mode,
            state_.opening_intro.post_intro_phase);
        // 0x13726 stamps the 0xD4446 border around the selected destination
        // in plane space. The plane-to-screen offset for the cab plate is
        // (+30,-2); plan widths already include the three-tile border.
        constexpr int kCabPlaneToScreenX = 30;
        constexpr int kCabPlaneToScreenY = -2;
        const auto tile = [this](const std::string_view name) -> const QImage* {
            const auto found = draw_asset_cache_.find(
                std::string("intro.cab.cursor.") + std::string(name));
            return found == draw_asset_cache_.end() ? nullptr
                                                    : &found->second;
        };
        const auto x = plan.cursor_x + kCabPlaneToScreenX;
        const auto y = plan.cursor_y + kCabPlaneToScreenY;
        const auto width = plan.cursor_width;
        const auto height = plan.cursor_height;
        if (width <= 0 || height <= 0) {
            return;
        }
        constexpr int kTile = 8;
        const auto draw_at = [&painter](const QImage* image, const int px,
                                        const int py) {
            if (image != nullptr) {
                painter.drawImage(px, py, *image);
            }
        };
        draw_at(tile("top-left"), x, y);
        draw_at(tile("top-right"), x + width - kTile, y);
        draw_at(tile("bottom-left"), x, y + height - kTile);
        draw_at(tile("bottom-right"), x + width - kTile,
                y + height - kTile);
        for (int edge = x + kTile; edge <= x + width - 2 * kTile;
             edge += kTile) {
            draw_at(tile("top-middle"), edge, y);
            draw_at(tile("bottom-middle"), edge, y + height - kTile);
        }
        for (int edge = y + kTile; edge <= y + height - 2 * kTile;
             edge += kTile) {
            draw_at(tile("left"), x, edge);
            draw_at(tile("right"), x + width - kTile, edge);
        }
        if (state_.opening_intro.post_intro_caption_visible) {
            draw_startup_text(painter, plan.caption_x, plan.caption_y,
                              plan.caption, source_text_color(0U));
        }
    }

    void draw_opening_sprite(
        QPainter& painter, const OpeningIntroSpriteRequest& request) const {
        const auto group = opening_intro_sprite_group(request.sprite_descriptor);
        const auto prefix = opening_intro_sprite_alias_prefix(group);
        if (prefix.empty()) {
            return;
        }
        // Name-table overlays and wilderness map patches are already
        // extracted in their source-plane positions, unlike SAT entries
        // whose origin is relative to $80. The resource importer crops them
        // individually but preserves that source origin in resources.tsv.
        if (group.family == OpeningIntroSpriteFamily::wilderness_map_left ||
            group.family == OpeningIntroSpriteFamily::wilderness_map_center ||
            group.family == OpeningIntroSpriteFamily::wilderness_map_right ||
            group.family == OpeningIntroSpriteFamily::background_patch ||
            group.family == OpeningIntroSpriteFamily::wilderness_overlay ||
            group.family == OpeningIntroSpriteFamily::wilderness_final_overlay) {
            const auto palette_fade_weight = [&]() -> std::uint16_t {
                if (group.family ==
                        OpeningIntroSpriteFamily::wilderness_map_center ||
                    group.family == OpeningIntroSpriteFamily::background_patch) {
                    return opening_entity_palette_fade_weight(0U);
                }
                if (group.family ==
                    OpeningIntroSpriteFamily::wilderness_map_right) {
                    return opening_entity_palette_fade_weight(1U);
                }
                return 255U;
            }();
            for (const auto& record : state_.asset_catalog.catalog.records) {
                if (record.alias.rfind(prefix, 0) != 0) {
                    continue;
                }
                const auto image = draw_asset_cache_.find(record.alias);
                if (image != draw_asset_cache_.end()) {
                    const auto layer = palette_fade_weight == 255U
                                           ? image->second
                                           : scale_genesis_image_rgb(
                                                 image->second,
                                                 palette_fade_weight);
                    painter.drawImage(record.origin_x, record.origin_y,
                                      layer);
                }
            }
            return;
        }

        // SAT cells are host RGBA crops. Live SAT x/y is origin $80; the
        // catalog origin is the cell's dump offset. Clip instead of wrapping
        // at 256: H40 is 320 wide, and Qt has no sprite-link terminator.
        if (group.family == OpeningIntroSpriteFamily::aircraft_plane_a) {
            const auto image = draw_asset_cache_.find(prefix);
            if (image != draw_asset_cache_.end()) {
                const auto point = opening_intro_aircraft_plane_draw_point(
                    state_.opening_intro.stage_loop.scroll.x_offset);
                // The cropped plate records its color-zero margin in the
                // catalog origin; the nametable-based draw point is the
                // plane origin, so the margin must be restored.
                const auto* record =
                    state_.asset_catalog.catalog.find(std::string(prefix));
                painter.drawImage(
                    point.x + (record == nullptr ? 0 : record->origin_x),
                    point.y + (record == nullptr ? 0 : record->origin_y),
                    image->second);
            }
            return;
        }

        for (const auto& record : state_.asset_catalog.catalog.records) {
            if (record.alias.rfind(prefix, 0) != 0) {
                continue;
            }
            const auto image = draw_asset_cache_.find(record.alias);
            if (image == draw_asset_cache_.end()) {
                continue;
            }
            const auto point = opening_intro_sat_draw_point(
                request.x, request.y, record.origin_x, record.origin_y);
            painter.drawImage(point.x, point.y, image->second);
        }
    }

    std::uint16_t opening_entity_palette_fade_weight(
        const std::size_t entity_index) const {
        if (entity_index >= state_.opening_intro.entity_colors.words.size()) {
            return 255U;
        }
        const auto word = state_.opening_intro.entity_colors.words[entity_index][0];
        const auto red = static_cast<std::uint16_t>((word >> 9U) & 0x07U);
        const auto green = static_cast<std::uint16_t>((word >> 5U) & 0x07U);
        const auto blue = static_cast<std::uint16_t>((word >> 1U) & 0x07U);
        const auto level = std::min({red, green, blue});
        return static_cast<std::uint16_t>(level * 255U / 7U);
    }

    void draw_opening_text(
        QPainter& painter, const OpeningIntroTextRequest& request) const {
        // The wilderness resource set is installed after the source fades
        // the previous Seattle palette to black. The ROM still writes the
        // date/place/team records at 0x5AE9E, but the independent capture
        // shows that palette bank 1 (the 0xA000 text writes) is not visible
        // in this scene; only the 0xE000 warning/ambush bank survives. Keep
        // the source writes in the conversion state and apply this host
        // presentation rule at draw time instead of deleting source calls.
        if (opening_scene_ == OpeningIntroScene::wilderness &&
            (request.palette_flags & 0x6000U) == 0x2000U) {
            return;
        }
        const auto text_asset = opening_intro_text_asset(request.text_asset);
        const auto text = opening_intro_text(text_asset);
        constexpr std::uint16_t kPlaneBase = 0x2000U;
        if (request.destination_word < kPlaneBase) {
            return;
        }
        const auto offset = static_cast<std::uint16_t>(
            request.destination_word - kPlaneBase);
        const auto x = static_cast<int>(((offset % 0x80U) / 2U) * 8U);
        auto y = static_cast<int>((offset / 0x80U) * 8U);
        if (opening_scene_ == OpeningIntroScene::airport) {
            // The final airport captions are written to Plane B while the
            // source scrolls FFDD0E from 0 to 0x78. The background follows
            // that same VScroll in draw_opening_scene(); leaving the text at
            // its unscrolled name-table row produces the visible overlap
            // between the spending and landing captions in the host view.
            y -= static_cast<int>(
                state_.opening_intro.stage_loop.scroll.fade_counter);
        }
        draw_startup_text(painter, x, y, text,
                          source_text_color(request.palette_flags));
    }

    void draw_title_logo(QPainter& painter) const {
        const auto* record = state_.asset_catalog.catalog.find("title.logo");
        const auto image = draw_asset_cache_.find("title.logo");
        if (record == nullptr || image == draw_asset_cache_.end() ||
            title_loop_.logo.columns_revealed == 0U) {
            return;
        }

        const auto visible_width = std::min(
            image->second.width(),
            static_cast<int>(title_loop_.logo.columns_revealed * 8U));
        if (visible_width <= 0) {
            return;
        }
        painter.save();
        painter.setClipRect(record->origin_x, record->origin_y,
                            visible_width, image->second.height());
        painter.drawImage(record->origin_x, record->origin_y, image->second);
        painter.restore();
    }

    void draw_title_sprites(QPainter& painter) const {
        const auto phase = title_loop_.sprite.phase % 6U;
        const auto alias = "title.sprite." + std::to_string(phase);
        const auto* record = state_.asset_catalog.catalog.find(alias);
        const auto image = draw_asset_cache_.find(alias);
        if (record == nullptr || image == draw_asset_cache_.end() ||
            title_loop_.sprite.x == 0x98) {
            return;
        }

        // 0x1C6A updates the source X origin by four pixels. The extracted
        // sprite record retains its source-local origin; only that moving
        // origin changes here.
        const auto sprite_delta =
            static_cast<int>(title_loop_.sprite.x) - 0x9C;
        painter.drawImage(sprite_delta + record->origin_x, record->origin_y,
                          image->second);
    }

    void draw_title_text(QPainter& painter) const {
        const auto* record = state_.asset_catalog.catalog.find("title.text");
        const auto image = draw_asset_cache_.find("title.text");
        if (record == nullptr || image == draw_asset_cache_.end()) {
            return;
        }

        if (title_prompt_visible_) {
            painter.drawImage(
                QRect(record->origin_x, record->origin_y, image->second.width(),
                      7),
                image->second, QRect(0, 0, image->second.width(), 7));
        }
        if (title_copyright_visible_) {
            const auto copyright_height = image->second.height() - 8;
            painter.drawImage(
                QRect(record->origin_x, record->origin_y + 8,
                      image->second.width(), copyright_height),
                image->second,
                QRect(0, 8, image->second.width(), copyright_height));
        }
    }

    void draw_menu_metadata_grid(QPainter& painter) const {
        // 0xC3E6 → 0x11C0: D7+1 columns by D1+1 rows at $C186 (24,24).
        // The host starts each frame from the extracted startup background,
        // so there is no stale name-table surface to clear here.  Filling the
        // source grid rectangle would erase the bezel/decal layer that the
        // ROM's background planes leave visible around the menu text.
        (void)write_menu_grid_with_metadata();
    }

    void draw_menu_overlay(QPainter& painter) const {
        if (state_.phase == StartupIntegrationPhase::start_options) {
            draw_menu_metadata_grid(painter);
            draw_startup_layout(painter, kStartOptionsPlaneVram,
                                kStartOptionsLayout);
            draw_cursor(painter, startup_menu_vram_x(kStartOptionsCursorVram),
                        startup_menu_cursor_y(
                            kStartOptionsCursorVram,
                            static_cast<std::uint8_t>(
                                state_.start_options.selected)));
            return;
        }

        if (state_.phase == StartupIntegrationPhase::save_load) {
            if (state_.save_load.message != SaveLoadMessage::none) {
                // 0x59172 calls 0xC3E6 before the 0xC0DA copy, which
                // restamps the 0xC186 metadata grid and clears leftover
                // Start Options ink.
                draw_menu_metadata_grid(painter);
                if (state_.save_load.message == SaveLoadMessage::no_save_data) {
                    draw_startup_layout(painter, kSaveLoadMessagePlaneVram,
                                        kNoSaveDataLayout);
                } else {
                    draw_startup_layout(painter, kSaveLoadMessagePlaneVram,
                                        kSaveSuccessLayout);
                }
                return;
            }
            // 0xC0DA writes Game 1/2 over the live Start Options nametable.
            draw_startup_layout(painter, kStartOptionsPlaneVram,
                                kStartOptionsLayout);
            draw_startup_layout(painter, kSaveLoadPlaneVram, kSaveLoadLayout);
            draw_cursor(painter, startup_menu_vram_x(kSaveLoadCursorVram),
                        startup_menu_cursor_y(
                            kSaveLoadCursorVram,
                            static_cast<std::uint8_t>(
                                state_.save_load.selected)));
            return;
        }

        draw_menu_metadata_grid(painter);
        const auto portrait_index = static_cast<int>(
            state_.joshua_archetype.selected);
        const auto portrait_alias =
            "menu.field.runner.portrait." + std::to_string(portrait_index);
        const auto portrait = draw_asset_cache_.find(portrait_alias);
        if (portrait != draw_asset_cache_.end()) {
            // 0x592C2 -> 0x1F38A uploads the selected 80-word portrait
            // resource at the source's x/y tile origin (8,8), i.e. the
            // 64,64 host pixel position. The old 80,56 placement was a
            // visual guess and shifted every archetype portrait.
            painter.drawImage(64, 64, portrait->second);
        }
        draw_startup_layout(painter, kJoshuaArchetypePlaneVram,
                            kJoshuaArchetypeLayout);
        draw_cursor(painter, startup_menu_vram_x(kJoshuaArchetypeCursorVram),
                    startup_menu_cursor_y(
                        kJoshuaArchetypeCursorVram,
                        static_cast<std::uint8_t>(
                            state_.joshua_archetype.selected)));
    }

    template <std::size_t N>
    void draw_startup_layout(
        QPainter& painter, const std::uint16_t plane_vram,
        const std::array<StartupMenuTextLine, N>& lines) const {
        const auto x = startup_menu_vram_x(plane_vram);
        for (const auto& line : lines) {
            draw_startup_text(painter, x,
                              startup_menu_line_y(plane_vram, line.row),
                              line.text, source_text_color(0U));
        }
    }

    void draw_conversion_boundary_error(QPainter& painter) const {
        painter.setPen(QColor(255, 220, 80));
        painter.setFont(QFont(QStringLiteral("Courier New"), 9,
                              QFont::Bold));
        painter.drawText(42, 92, QStringLiteral("CONVERSION BOUNDARY"));
        const auto& field_failure = field_scene_runtime_.failure();
        if (save_load_boundary_unresolved_) {
            painter.drawText(42, 112,
                             QStringLiteral("SAVE SLOT DATA NOT AVAILABLE"));
        } else if (!field_failure.empty()) {
            painter.drawText(42, 112,
                             QString::fromStdString(field_failure));
        } else {
            painter.drawText(42, 112, QStringLiteral("NOT YET RESOLVED"));
        }
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(42, 184,
                         QStringLiteral("SOURCE DATA REQUIRED"));
    }

    void draw_cursor(QPainter& painter, const int x, const int y) const {
        const auto image = draw_asset_cache_.find("menu.startup.cursor");
        if (image != draw_asset_cache_.end()) {
            painter.drawImage(x, y, image->second);
        } else {
            painter.drawText(x, y + 10, QStringLiteral(">"));
        }
    }

    QColor source_text_color(const std::uint16_t palette_flags) const {
        // The converted requests retain the ROM's 0xA000/0xE000 palette
        // attributes for provenance. The host presents visible extracted
        // glyph masks in the source's bright-green text color; scene-specific
        // palette-bank visibility is handled by draw_opening_text rather than
        // recreating CRAM or a Genesis palette service in the runtime.
        (void)palette_flags;
        return QColor(0x00, 0xFF, 0x00);
    }

    void draw_startup_text(QPainter& painter, const int x, const int y,
                           const std::string_view text,
                           const QColor& color) const {
        const auto image = draw_asset_cache_.find("menu.startup.font");
        if (image == draw_asset_cache_.end() || image->second.width() < 8 ||
            image->second.height() < 8) {
            return;
        }
        auto cursor = x;
        for (const auto character : text) {
            const auto code = static_cast<unsigned char>(character);
            if (code < 0x20U || code > 0x7FU) {
                cursor += 8;
                continue;
            }
            const auto glyph = static_cast<int>(code - 0x20U);
            const auto source_x = (glyph % 16) * 8;
            const auto source_y = (glyph / 16) * 8;
            if (source_y + 8 <= image->second.height()) {
                QImage tinted_glyph(8, 8, QImage::Format_ARGB32_Premultiplied);
                tinted_glyph.fill(Qt::transparent);
                for (int glyph_y = 0; glyph_y < 8; ++glyph_y) {
                    for (int glyph_x = 0; glyph_x < 8; ++glyph_x) {
                        const auto source_pixel = image->second.pixelColor(
                            source_x + glyph_x, source_y + glyph_y);
                        const auto coverage = std::max(
                            {source_pixel.red(), source_pixel.green(),
                             source_pixel.blue()});
                        if (source_pixel.alpha() == 0 || coverage == 0) {
                            continue;
                        }
                        const auto alpha = static_cast<int>(
                            (static_cast<std::uint32_t>(source_pixel.alpha()) *
                             static_cast<std::uint32_t>(coverage)) /
                            255U);
                        tinted_glyph.setPixelColor(
                            glyph_x, glyph_y,
                            QColor(color.red(), color.green(), color.blue(),
                                   alpha));
                    }
                }
                painter.drawImage(cursor, y, tinted_glyph);
            }
            cursor += 8;
        }
    }

    std::filesystem::path asset_root_;
    StartupIntegrationState state_;
    StartupIntegrationCallbacks startup_callbacks_;
    qt_runtime::FieldSceneRuntime field_scene_runtime_;
    TitleLoopState title_loop_;
    TitleLoopCallbacks title_loop_callbacks_;
    QTimer host_frame_timer_;
    QElapsedTimer host_clock_;
    std::int64_t host_frame_deadline_ns_ = 0;
    std::map<std::string, QImage> draw_asset_cache_;
    OpeningIntroScene opening_scene_ = OpeningIntroScene::seattle;
    OpeningIntroStage opening_stage_ = OpeningIntroStage::not_started;
    std::vector<OpeningIntroSpriteRequest> opening_sprites_;
    std::optional<OpeningIntroSpriteRequest> opening_wilderness_map_left_;
    std::optional<OpeningIntroSpriteRequest> opening_wilderness_map_center_;
    std::optional<OpeningIntroSpriteRequest> opening_wilderness_map_right_;
    std::optional<OpeningIntroSpriteRequest> opening_wilderness_overlay_;
    std::optional<OpeningIntroSpriteRequest> opening_wilderness_patch_;
    std::optional<OpeningIntroSpriteRequest>
        opening_wilderness_final_overlay_;
    std::vector<OpeningIntroTextRequest> opening_text_;
    std::vector<std::uint16_t> opening_audio_commands_;

    std::uint8_t logo_strobe_phase_ = 0;
    std::array<std::uint16_t, 8> logo_strobe_palette_{};
    std::vector<BuiltSpriteRecord> pre_title_sprite_records_;
    std::vector<PreTitleAudioRequest> pre_title_audio_requests_;
    JoshuaArchetypeChoice selected_archetype_ =
        JoshuaArchetypeChoice::samurai;
    StartOptionsChoice start_options_cursor_ = StartOptionsChoice::new_game;
    SaveSlotChoice save_load_cursor_ = SaveSlotChoice::slot_one;
    JoshuaArchetypeChoice joshua_cursor_ = JoshuaArchetypeChoice::samurai;
    JoshuaArchetypeChoice joshua_preview_ = JoshuaArchetypeChoice::samurai;
    SaveSlotChoice save_load_requested_slot_ = SaveSlotChoice::slot_one;
    SaveLoadMessage save_load_message_ = SaveLoadMessage::none;
    bool save_load_boundary_unresolved_ = false;
    bool title_resources_loaded_ = false;
    bool title_font_loaded_ = false;
    bool title_prompt_visible_ = false;
    bool title_copyright_visible_ = false;
    bool title_completion_palette_copied_ = false;
    TitlePromptVariant title_prompt_variant_ = TitlePromptVariant::VariantA;
    std::uint8_t title_circuit_phase_ = 0;
    std::uint16_t title_animation_offset_ = 0;
    std::uint16_t title_logo_columns_ = 0;
    std::uint16_t title_sprite_phase_ = 0;
    std::int16_t title_sprite_x_ = 0x98;
    std::uint16_t title_controller_events_ = 0;
    std::uint16_t pending_title_controller_word_ = 0;
    std::uint8_t field_controller_nibble_ = 0;
    std::uint8_t field_controller_port_byte_ = 0;
    std::uint32_t title_vblanks_ = 0;
    std::size_t title_palette_steps_ = 0;
    std::size_t resource_stream_words_ = 0;
    std::size_t horizontal_scroll_lines_ = 0;
    std::size_t title_sprite_record_builds_ = 0;
    std::size_t palette_fade_steps_ = 0;
    std::size_t palette_transition_steps_ = 0;
    std::size_t audio_variant_ticks_ = 0;
    std::size_t audio_ticks_ = 0;
    std::size_t title_resource_loads_ = 0;
    std::size_t title_font_loads_ = 0;
    std::uint32_t title_palette_frames_ = 0;
    std::uint32_t last_source_delay_ = 0;
    bool display_initialized_ = false;
    bool display_ready_ = false;
    std::vector<std::string> presentation_boundaries_;
};

} // namespace

int main(int argc, char** argv) {
    try {
        bool field_smoke = false;
        bool field_smoke_move_right = false;
        bool field_command_reopen_smoke = false;
        bool intro_to_field_smoke = false;
        std::optional<std::filesystem::path> field_dump_path;
        std::optional<std::filesystem::path> field_hud_qa_path;
        std::optional<std::filesystem::path> field_command_dump_path;
        std::optional<std::uint8_t> field_command_page;
        std::optional<std::filesystem::path> field_action_dump_path;
        std::optional<std::filesystem::path> field_clips_dump_path;
        std::optional<std::string> field_archetype_name;
        std::optional<std::string> field_dialogue_dump_prompt;
        std::optional<std::filesystem::path> field_dialogue_dump_path;
        std::optional<std::uint32_t> boot_dump_frame;
        std::optional<std::filesystem::path> boot_dump_path;
        std::optional<std::string> startup_dump_stage;
        std::optional<std::uint32_t> startup_dump_selection;
        std::optional<std::uint32_t> startup_dump_frame;
        std::optional<std::filesystem::path> startup_dump_path;
        std::optional<std::filesystem::path> startup_qa_path;
        std::optional<std::uint32_t> opening_dump_selection;
        std::optional<std::uint32_t> opening_dump_start;
        std::optional<std::uint32_t> opening_dump_end;
        std::optional<std::filesystem::path> opening_dump_directory;
        for (int index = 1; index < argc; ++index) {
            if (std::string(argv[index]) == "--help") {
                std::cout
                    << "usage: shadowrun_ghidra_conversion_qt"
                       " [--assets <decompilation/assets>] [--field-smoke]"
                       " [--field-smoke-move-right] [--field-dump <file>]"
                       " [--field-command-dump <file>]"
                       " [--field-command-page <0..6>]"
                       " [--field-action-dump <output-dir>]"
                       " [--field-clips-dump <output-dir>]"
                       " [--field-archetype <samurai|decker|gator>]"
                       " [--field-hud-qa <output-dir>]"
                       " [--field-command-reopen-smoke]"
                       " [--field-dialogue-dump <prompt> <file>]"
                       " [--boot-dump-frame <frame> <file>]"
                       " [--startup-dump <stage> <selection> <frame> <file>]"
                       " [--startup-qa <output-dir>]"
                       " [--intro-to-field-smoke]\n";
                return 0;
            }
            if (std::string(argv[index]) == "--field-smoke") {
                field_smoke = true;
            }
            if (std::string(argv[index]) == "--field-smoke-move-right") {
                field_smoke = true;
                field_smoke_move_right = true;
            }
            if (std::string(argv[index]) == "--field-command-reopen-smoke") {
                field_smoke = true;
                field_command_reopen_smoke = true;
            }
            if (std::string(argv[index]) == "--intro-to-field-smoke") {
                intro_to_field_smoke = true;
            }
            if (std::string(argv[index]) == "--field-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_dump_path = std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-command-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_command_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-action-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_action_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-clips-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_clips_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-archetype" &&
                index + 1 < argc) {
                field_smoke = true;
                field_archetype_name = std::string(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-hud-qa" &&
                index + 1 < argc) {
                field_smoke = true;
                field_hud_qa_path = std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-command-page" &&
                index + 1 < argc) {
                field_smoke = true;
                field_command_page = static_cast<std::uint8_t>(
                    std::stoul(argv[++index]));
            }
            if (std::string(argv[index]) == "--field-dialogue-dump" &&
                index + 2 < argc) {
                field_smoke = true;
                field_dialogue_dump_prompt = std::string(argv[++index]);
                field_dialogue_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--boot-dump-frame" &&
                index + 2 < argc) {
                boot_dump_frame = static_cast<std::uint32_t>(
                    std::stoul(argv[++index]));
                boot_dump_path = std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--startup-dump" &&
                index + 4 < argc) {
                startup_dump_stage = std::string(argv[++index]);
                startup_dump_selection = static_cast<std::uint32_t>(
                    std::stoul(argv[++index]));
                startup_dump_frame = static_cast<std::uint32_t>(
                    std::stoul(argv[++index]));
                startup_dump_path = std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--startup-qa" &&
                index + 1 < argc) {
                startup_qa_path = std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--opening-dump-range" &&
                index + 4 < argc) {
                opening_dump_selection = static_cast<std::uint32_t>(
                    std::stoul(argv[++index]));
                opening_dump_start = static_cast<std::uint32_t>(
                    std::stoul(argv[++index]));
                opening_dump_end = static_cast<std::uint32_t>(
                    std::stoul(argv[++index]));
                opening_dump_directory =
                    std::filesystem::path(argv[++index]);
            }
        }
        const auto asset_root = find_asset_root(argc, argv);
        if (asset_root.empty()) {
            std::cerr << "missing direct asset root; pass --assets "
                         "<decompilation/assets>\n";
            return 2;
        }

        QApplication application(argc, argv);
        if (field_smoke) {
            // The real startup route records the player's choice in the
            // Joshua menu state before the field handoff. The dump route
            // injects the same selection through both typed channels so the
            // field bootstrap has exactly one archetype source of truth.
            const auto build_smoke_state =
                [&](const std::uint8_t archetype_index,
                    StartupIntegrationState& state, std::string& error) {
                state.asset_catalog =
                    load_conversion_asset_catalog(asset_root);
                if (!state.asset_catalog.complete()) {
                    error = "field smoke: direct asset catalog is incomplete";
                    return false;
                }
                const auto source = load_field_session_source_assets(
                    state.asset_catalog.catalog);
                if (!source.complete()) {
                    error = "field smoke: source records are incomplete";
                    return false;
                }
                state.field_session_source_assets = *source.assets;
                const auto startup_records = load_startup_record_assets(
                    state.asset_catalog.catalog);
                if (!startup_records.complete()) {
                    error = "field smoke: startup records are incomplete";
                    return false;
                }
                state.startup_records = *startup_records.assets;
                const std::vector<JoshuaArchetypeTemplate> joshua_templates(
                    state.startup_records->joshua_archetypes.begin(),
                    state.startup_records->joshua_archetypes.end());
                state.joshua_selection =
                    initialize_joshua_archetype_selection(archetype_index,
                                                          joshua_templates);
                if (!state.joshua_selection.has_value()) {
                    error =
                        "field smoke: Joshua source record is unavailable";
                    return false;
                }
                state.joshua_archetype.selected =
                    static_cast<JoshuaArchetypeChoice>(archetype_index);
                return true;
            };
            std::uint8_t field_archetype = 0U;
            if (field_archetype_name.has_value()) {
                const auto parsed = parse_field_archetype(*field_archetype_name);
                if (!parsed.has_value()) {
                    std::cerr << "field smoke: unknown archetype '"
                              << *field_archetype_name
                              << "' (expected samurai, decker, or gator)\n";
                    return 3;
                }
                field_archetype = *parsed;
            }
            StartupIntegrationState smoke_state;
            std::string smoke_state_error;
            if (!build_smoke_state(field_archetype, smoke_state,
                                   smoke_state_error)) {
                std::cerr << smoke_state_error << '\n';
                return 3;
            }
            qt_runtime::FieldSceneRuntime field_scene([](const auto&) {});
            if (!field_scene.bootstrap(smoke_state)) {
                std::cerr << "field smoke: " << field_scene.failure() << '\n';
                return 3;
            }
            if (field_smoke_move_right) {
                field_scene.set_controller_nibble(0x08U);
                field_scene.advance_frame();
            }
            if (field_command_reopen_smoke) {
                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field command reopen smoke: first open failed\n";
                    return 3;
                }

                // The source command loop exits on a newly pressed Start
                // edge. Release the first synthetic Start, press it again,
                // and verify that the converted coordinator returns to the
                // field before dispatching the second open.
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0x80U);
                field_scene.advance_frame();
                if (field_scene.command_overview_open()) {
                    std::cerr << "field command reopen smoke: close failed\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field command reopen smoke: second open failed\n";
                    return 3;
                }
                std::cout << "field_command_reopen=passed\n";
                return 0;
            }
            if (field_dialogue_dump_prompt.has_value() &&
                field_dialogue_dump_path.has_value()) {
                const auto prompt = parse_stoker_dialogue_prompt(
                    *field_dialogue_dump_prompt);
                if (!prompt.has_value()) {
                    std::cerr << "field dialogue dump: unknown Stoker prompt '"
                              << *field_dialogue_dump_prompt << "'\n";
                    return 3;
                }
                if (!field_scene.open_stoker_dialogue_for_dump(*prompt)) {
                    std::cerr << "field dialogue dump: prompt did not open\n";
                    return 3;
                }
                QImage dialogue_scene(kSceneWidth, kSceneHeight,
                                      QImage::Format_RGB32);
                dialogue_scene.fill(Qt::black);
                QPainter dialogue_painter(&dialogue_scene);
                field_scene.paint(dialogue_painter);
                dialogue_painter.end();
                if (!dialogue_scene.save(QString::fromStdWString(
                        field_dialogue_dump_path->wstring()))) {
                    std::cerr << "field dialogue dump: could not write "
                              << field_dialogue_dump_path->string() << '\n';
                    return 3;
                }
                std::cout << "field_dialogue_dump=written prompt="
                          << *field_dialogue_dump_prompt << " path="
                          << field_dialogue_dump_path->string() << "\n";
                return 0;
            }
            if (field_command_dump_path.has_value()) {
                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field command dump: command overview did "
                                 "not open\n";
                    return 3;
                }
                if (field_command_page.has_value() &&
                    !field_scene.seek_command_overview_page(
                        *field_command_page)) {
                    std::cerr << "field command dump: could not reach page "
                              << static_cast<int>(*field_command_page) << '\n';
                    return 3;
                }
                QImage command_scene(kSceneWidth, kSceneHeight,
                                     QImage::Format_RGB32);
                command_scene.fill(Qt::black);
                QPainter command_painter(&command_scene);
                field_scene.paint(command_painter);
                command_painter.end();
                if (!field_command_shell_survives(command_scene, asset_root)) {
                    std::cerr << "field command dump: source command shell "
                                 "was not preserved around the overlays\n";
                    return 3;
                }
                if (!command_scene.save(QString::fromStdWString(
                        field_command_dump_path->wstring()))) {
                    std::cerr << "field command dump: could not write "
                              << field_command_dump_path->string() << '\n';
                    return 3;
                }
                std::cout << "field_command_dump=written path="
                          << field_command_dump_path->string()
                          << " selector="
                          << static_cast<int>(field_scene.command_overview_view()
                                                   .source_state.primary_page)
                          << "\n";
                return 0;
            }
            if (field_action_dump_path.has_value()) {
                std::filesystem::create_directories(*field_action_dump_path);
                const auto render_scene = [&] {
                    QImage action_scene(kSceneWidth, kSceneHeight,
                                        QImage::Format_RGB32);
                    action_scene.fill(Qt::black);
                    QPainter action_painter(&action_scene);
                    field_scene.paint(action_painter);
                    action_painter.end();
                    return action_scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    if (!field_command_shell_survives(scene, asset_root)) {
                        std::cerr << "field action dump: command shell was "
                                     "not preserved for "
                                  << name << "\n";
                        return false;
                    }
                    return scene.save(QString::fromStdWString(
                        ((*field_action_dump_path) /
                         (std::string(name) + ".bmp"))
                            .wstring()));
                };
                const auto write_scene = [&](const std::string_view name) {
                    return save_scene(name, render_scene());
                };
                const auto press = [&](const std::uint8_t port_byte) {
                    field_scene.set_controller_port_byte(port_byte);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                const auto same_image = [](const QImage& left,
                                           const QImage& right) {
                    if (left.size() != right.size() ||
                        left.format() != right.format()) {
                        return false;
                    }
                    for (int y = 0; y < left.height(); ++y) {
                        for (int x = 0; x < left.width(); ++x) {
                            if (left.pixel(x, y) != right.pixel(x, y)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto region_matches = [](const QImage& scene,
                                               const QImage& asset,
                                               const int origin_x,
                                               const int origin_y) {
                    if (asset.isNull() || origin_x < 0 || origin_y < 0 ||
                        origin_x + asset.width() > scene.width() ||
                        origin_y + asset.height() > scene.height()) {
                        return false;
                    }
                    for (int y = 0; y < asset.height(); ++y) {
                        for (int x = 0; x < asset.width(); ++x) {
                            if (scene.pixel(origin_x + x, origin_y + y) !=
                                asset.pixel(x, y)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto direct_asset = [&](const std::string& name) {
                    return QImage(QString::fromStdWString(
                        (asset_root / "menu" / "field" / name).wstring()));
                };
                const auto item_factor = [](const std::uint8_t item_code) {
                    const auto* definition =
                        find_us_item_definition(item_code);
                    return definition == nullptr
                               ? std::uint8_t{0}
                               : definition->source_combat_factor;
                };
                const auto same_equipment_state =
                    [](const RunnerRecordFields& left,
                       const RunnerRecordFields& right,
                       const std::size_t slot) {
                        return left.equipped_inventory_slot ==
                                   right.equipped_inventory_slot &&
                               left.equipped_item_code ==
                                   right.equipped_item_code &&
                               left.equipped_armor_index ==
                                   right.equipped_armor_index &&
                               left.ammo_rounds == right.ammo_rounds &&
                               left.inventory_item_codes[slot] ==
                                   right.inventory_item_codes[slot] &&
                               left.inventory_phase_bytes[slot] ==
                                   right.inventory_phase_bytes[slot];
                    };

                const auto& initial_record = field_scene.active_runner_record();
                if (initial_record.inventory_item_codes[0] == 0U) {
                    std::cerr << "field action dump: source inventory cell "
                                 "0 is empty\n";
                    return 3;
                }
                const auto focus_equip = direct_asset("action-focus-0.bmp");
                const auto focus_cancel = direct_asset("action-focus-3.bmp");
                if (focus_equip.isNull() || focus_cancel.isNull()) {
                    std::cerr << "field action dump: direct action focus "
                                 "asset is missing\n";
                    return 3;
                }

                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field action dump: command overview did "
                                 "not open\n";
                    return 3;
                }
                // The converted command loop starts on the Pocket Secretary
                // page (selector 6).  The source d-pad priority moves left
                // through the seven command pages to the first inventory
                // column before the action subloop is entered.
                for (int move = 0;
                     move < 7 &&
                     field_scene.command_overview_view().source_state
                             .primary_page != 0U;
                     ++move) {
                    press(0x04U);
                }
                if (field_scene.command_overview_view().source_state
                        .primary_page != 0U) {
                    std::cerr << "field action dump: inventory column did "
                                 "not become selected selector="
                              << static_cast<int>(field_scene
                                                      .command_overview_view()
                                                      .source_state.primary_page)
                              << "\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.inventory_action_open() ||
                    field_scene.inventory_action_selection() != 2U) {
                    std::cerr << "field action dump: inventory action did "
                                 "not open selector="
                              << static_cast<int>(field_scene
                                                      .command_overview_view()
                                                      .source_state.primary_page)
                              << " row="
                              << static_cast<int>(field_scene
                                                      .command_overview_view()
                                                      .source_state.secondary_page)
                              << " action_open="
                              << (field_scene.inventory_action_open() ? 1 : 0)
                              << " action_selector="
                              << static_cast<int>(field_scene
                                                      .inventory_action_selection())
                              << "\n";
                    return 3;
                }
                const auto action_open_scene = render_scene();
                if (!region_matches(action_open_scene, focus_equip, 128, 184) ||
                    !save_scene("00_inventory_action_equip",
                                action_open_scene)) {
                    std::cerr << "field action dump: could not write open "
                                 "scene or action focus is misplaced\n";
                    return 3;
                }

                // Selection is a cyclic source selector: Left from Equip
                // wraps to Cancel, and Right from Cancel wraps back.  Check
                // both the typed state and the exact direct focus placement.
                press(0x04U);
                if (field_scene.inventory_action_selection() != 5U) {
                    std::cerr << "field action dump: left selection did not "
                                 "wrap to cancel\n";
                    return 3;
                }
                const auto action_cancel_scene = render_scene();
                if (!region_matches(action_cancel_scene, focus_cancel, 200,
                                    184) ||
                    !save_scene("01_inventory_action_cancel",
                                action_cancel_scene)) {
                    std::cerr << "field action dump: cancel focus is "
                                 "misplaced\n";
                    return 3;
                }

                press(0x08U);
                if (field_scene.inventory_action_selection() != 2U) {
                    std::cerr << "field action dump: right selection did not "
                                 "wrap to equip\n";
                    return 3;
                }
                const auto action_reselected_scene = render_scene();
                if (!region_matches(action_reselected_scene, focus_equip, 128,
                                    184) ||
                    !same_image(action_open_scene, action_reselected_scene) ||
                    !save_scene("02_inventory_action_reselected",
                                action_reselected_scene)) {
                    std::cerr << "field action dump: re-selection did not "
                                 "restore the source frame\n";
                    return 3;
                }

                const auto item_before_cancel =
                    field_scene.active_runner_record().inventory_item_codes[0];
                press(0x10U);
                if (field_scene.inventory_action_open() ||
                    !field_scene.command_overview_open() ||
                    field_scene.active_runner_record().inventory_item_codes[0] !=
                        item_before_cancel ||
                    !write_scene("03_inventory_action_unselected")) {
                    std::cerr << "field action dump: B did not un-select "
                                 "without mutating the item\n";
                    return 3;
                }

                press(0x40U);
                if (!field_scene.inventory_action_open() ||
                    field_scene.inventory_action_selection() != 2U ||
                    !write_scene("04_inventory_action_reselected")) {
                    std::cerr << "field action dump: item could not be "
                                 "re-selected after cancel\n";
                    return 3;
                }

                const auto before_equip = field_scene.active_runner_record();
                const auto selected = read_selected_inventory_item(
                    before_equip, 0U, 0U);
                if (!selected.has_value()) {
                    std::cerr << "field action dump: selected source item "
                                 "disappeared before equip\n";
                    return 3;
                }
                auto expected_after_equip = before_equip;
                const auto selected_branch = classify_selected_item(
                    selected->item_code, true);
                const auto expected_mutation =
                    toggle_selected_equipment(
                        expected_after_equip, *selected,
                        item_factor(before_equip.equipped_item_code),
                        item_factor(selected->item_code));
                press(0x40U);
                const auto& after_equip = field_scene.active_runner_record();
                if ((selected_branch == SelectedItemBranch::weapon_or_gear ||
                     selected_branch == SelectedItemBranch::armor) &&
                    (expected_mutation == EquipmentMutation::invalid_selection ||
                     expected_mutation == EquipmentMutation::rejected_item_action ||
                     !same_equipment_state(after_equip,
                                            expected_after_equip,
                                            selected->slot))) {
                    std::cerr << "field action dump: equip mutation diverged "
                                 "from converted typed result\n";
                    return 3;
                }
                if (!field_scene.command_overview_open() ||
                    field_scene.inventory_action_open() ||
                    !save_scene("05_inventory_after_equip", render_scene())) {
                    std::cerr << "field action dump: equip action did not "
                                 "return to the command overview\n";
                    return 3;
                }

                // Page two exposes the attack/defense bars. Their indices are
                // source-derived typed math; verify that the rendered regions
                // use the direct assets selected by those values.
                press(0x08U);
                press(0x08U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 2U) {
                    std::cerr << "field action dump: combat page did not open\n";
                    return 3;
                }
                const auto combat_scene = render_scene();
                const auto& bars = field_scene.command_overview_combat_bars();
                const auto attack_asset = direct_asset(
                    "attack-bar-" + std::to_string(bars.attack_bar) + ".bmp");
                const auto defense_asset = direct_asset(
                    "defense-bar-" + std::to_string(bars.defense_bar) + ".bmp");
                if (!bars.attack_written || !bars.defense_written ||
                    !region_matches(combat_scene, attack_asset, 144, 104) ||
                    !region_matches(combat_scene, defense_asset, 144, 112) ||
                    !save_scene("06_combat_bars_after_equip", combat_scene)) {
                    std::cerr << "field action dump: typed combat math did not "
                                 "select the rendered bar assets\n";
                    return 3;
                }

                press(0x04U);
                press(0x04U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 0U) {
                    std::cerr << "field action dump: inventory page did not "
                                 "re-open after combat verification\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.inventory_action_open() ||
                    field_scene.inventory_action_selection() != 2U) {
                    std::cerr << "field action dump: action row did not "
                                 "re-open after page traversal\n";
                    return 3;
                }
                press(0x08U);
                if (field_scene.inventory_action_selection() != 3U) {
                    std::cerr << "field action dump: discard selector did not "
                                 "move from equip\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.discard_confirmation_open() ||
                    !write_scene("07_discard_confirmation_no")) {
                    std::cerr << "field action dump: discard confirmation "
                                 "did not open\n";
                    return 3;
                }

                press(0x08U);
                if (!field_scene.discard_confirmation_yes() ||
                    !write_scene("08_discard_confirmation_yes")) {
                    std::cerr << "field action dump: confirmation did not "
                                 "toggle to yes\n";
                    return 3;
                }
                press(0x40U);
                if (field_scene.inventory_action_open() ||
                    field_scene.discard_confirmation_open() ||
                    field_scene.active_runner_record().inventory_item_codes[0] !=
                        0U ||
                    !write_scene("09_inventory_after_discard")) {
                    std::cerr << "field action dump: discard did not close "
                                 "the converted action loop and clear the "
                                 "source cell\n";
                    return 3;
                }
                std::cout << "field_action_dump=written path="
                          << field_action_dump_path->string() << "\n";
                return 0;
            }
            if (field_clips_dump_path.has_value()) {
                std::filesystem::create_directories(*field_clips_dump_path);
                const auto render_scene = [&] {
                    QImage clips_scene(kSceneWidth, kSceneHeight,
                                       QImage::Format_RGB32);
                    clips_scene.fill(Qt::black);
                    QPainter clips_painter(&clips_scene);
                    field_scene.paint(clips_painter);
                    clips_painter.end();
                    return clips_scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    if (!field_command_shell_survives(scene, asset_root)) {
                        std::cerr << "field clips dump: command shell was "
                                     "not preserved for "
                                  << name << "\n";
                        return false;
                    }
                    return scene.save(QString::fromStdWString(
                        ((*field_clips_dump_path) /
                         (std::string(name) + ".bmp"))
                            .wstring()));
                };
                const auto write_scene = [&](const std::string_view name) {
                    return save_scene(name, render_scene());
                };
                const auto press = [&](const std::uint8_t port_byte) {
                    field_scene.set_controller_port_byte(port_byte);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                const auto region_matches = [](const QImage& scene,
                                               const QImage& asset,
                                               const int origin_x,
                                               const int origin_y) {
                    if (asset.isNull() || origin_x < 0 || origin_y < 0 ||
                        origin_x + asset.width() > scene.width() ||
                        origin_y + asset.height() > scene.height()) {
                        return false;
                    }
                    for (int y = 0; y < asset.height(); ++y) {
                        for (int x = 0; x < asset.width(); ++x) {
                            if (scene.pixel(origin_x + x, origin_y + y) !=
                                asset.pixel(x, y)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto direct_asset = [&](const std::string& name) {
                    return QImage(QString::fromStdWString(
                        (asset_root / "menu" / "field" / name).wstring()));
                };
                const auto same_clip_record =
                    [](const RunnerRecordFields& left,
                       const RunnerRecordFields& right) {
                        return left.ammo_clips == right.ammo_clips &&
                               left.ammo_rounds == right.ammo_rounds &&
                               left.equipped_inventory_slot ==
                                   right.equipped_inventory_slot &&
                               left.equipped_item_code ==
                                   right.equipped_item_code;
                    };
                const auto clips_row_matches =
                    [&](const QImage& scene, const QImage& focus,
                        const std::size_t focus_index) {
                        const auto background = direct_asset(
                            "action-background.bmp");
                        if (background.isNull() || focus.isNull() ||
                            focus_index >= 4U) {
                            return false;
                        }
                        auto expected = background.convertToFormat(
                            QImage::Format_RGB32);
                        QPainter expected_painter(&expected);
                        expected_painter.drawImage(
                            8 + static_cast<int>(focus_index * 24U), 0,
                            focus);
                        expected_painter.end();
                        return region_matches(scene, expected, 120, 184);
                    };
                const auto clips_shell_matches =
                    [](const QImage& expected, const QImage& actual) {
                        if (expected.size() != actual.size()) {
                            return false;
                        }
                        // Selection changes only the source action row and
                        // the command caption. Every other pixel in the
                        // Clips frame (portrait, inventory cells, status
                        // bars, title, counter, and surrounding shell) must
                        // remain stable while the selector moves.
                        for (int y = 0; y < expected.height(); ++y) {
                            for (int x = 0; x < expected.width(); ++x) {
                                const auto in_action_row =
                                    x >= 120 && x < 240 && y >= 184 &&
                                    y < 200;
                                const auto in_command_caption =
                                    x >= 104 && x < 264 && y >= 200 &&
                                    y < 208;
                                if (in_action_row || in_command_caption) {
                                    continue;
                                }
                                if (expected.pixel(x, y) != actual.pixel(x, y)) {
                                    return false;
                                }
                            }
                        }
                        return true;
                    };

                const auto clips_focus = std::array<QImage, 4>{
                    direct_asset("clips-focus-0.bmp"),
                    direct_asset("clips-focus-1.bmp"),
                    direct_asset("clips-focus-2.bmp"),
                    direct_asset("clips-focus-3.bmp")};
                for (const auto& focus : clips_focus) {
                    if (focus.isNull()) {
                        std::cerr << "field clips dump: direct Clips focus "
                                     "asset is missing\n";
                        return 3;
                    }
                }
                const auto initial_record = field_scene.active_runner_record();
                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field clips dump: command overview did not "
                                 "open\n";
                    return 3;
                }
                for (int move = 0;
                     move < 7 &&
                     field_scene.command_overview_view().source_state
                             .primary_page != 2U;
                     ++move) {
                    press(0x04U);
                }
                if (field_scene.command_overview_view().source_state
                        .primary_page != 2U) {
                    std::cerr << "field clips dump: Clips command selector "
                                 "did not become selected\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.clips_menu_open() ||
                    field_scene.clips_menu_selection() != 2U) {
                    std::cerr << "field clips dump: four-entry Clips menu "
                                 "did not open\n";
                    return 3;
                }
                const auto initial_clips_scene = render_scene();
                if (!clips_row_matches(initial_clips_scene, clips_focus[0], 0U) ||
                    !save_scene("00_clips_reload_selected",
                                initial_clips_scene)) {
                    std::cerr << "field clips dump: Clips row did not replace "
                                 "the five-button command row\n";
                    return 3;
                }

                // Walk every source selector entry, including the 5 -> 2
                // wrap, and verify the corresponding source focus block.
                for (std::size_t index = 1U; index < clips_focus.size();
                     ++index) {
                    press(0x08U);
                    const auto expected_selector = static_cast<std::uint8_t>(
                        index == 3U ? 5U : 2U + index);
                    if (field_scene.clips_menu_selection() !=
                        expected_selector) {
                        std::cerr << "field clips dump: selector movement "
                                     "diverged at index "
                                  << index << "\n";
                        return 3;
                    }
                    const auto scene = render_scene();
                    if (!clips_row_matches(scene, clips_focus[index], index) ||
                        !clips_shell_matches(initial_clips_scene, scene) ||
                        !save_scene("0" + std::to_string(index) +
                                        "_clips_selection_" +
                                        std::to_string(expected_selector),
                                    scene)) {
                        std::cerr << "field clips dump: selected focus block "
                                     "was misplaced at index "
                                  << index << "\n";
                        return 3;
                    }
                }
                press(0x08U);
                if (field_scene.clips_menu_selection() != 2U) {
                    std::cerr << "field clips dump: selector did not wrap "
                                 "from Cancel to Reload\n";
                    return 3;
                }
                const auto reselected_scene = render_scene();
                if (!clips_row_matches(reselected_scene, clips_focus[0], 0U) ||
                    !clips_shell_matches(initial_clips_scene,
                                         reselected_scene) ||
                    !save_scene("04_clips_reselected", reselected_scene)) {
                    std::cerr << "field clips dump: re-selection did not "
                                 "restore the four-button frame\n";
                    return 3;
                }

                // B leaves the Clips selector without mutating the typed
                // runner record. A can then enter the same four-entry loop
                // again from the parent command page.
                press(0x10U);
                if (field_scene.clips_menu_open() ||
                    !field_scene.command_overview_open() ||
                    !same_clip_record(field_scene.active_runner_record(),
                                      initial_record) ||
                    !write_scene("05_clips_unselected")) {
                    std::cerr << "field clips dump: B did not unselect Clips "
                                 "without mutation\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.clips_menu_open() ||
                    field_scene.clips_menu_selection() != 2U ||
                    !write_scene("06_clips_reopened")) {
                    std::cerr << "field clips dump: Clips could not be "
                                 "re-selected after B\n";
                    return 3;
                }

                // Exercise the converted reload gate and arithmetic against
                // the live startup record. This branch deliberately adapts
                // to the source record (confirmation, direct reload, or a
                // source gate message) instead of inventing test inventory.
                const auto before_reload = field_scene.active_runner_record();
                const auto* item = find_us_item_definition(
                    before_reload.equipped_item_code);
                const auto capacity = item == nullptr
                                          ? std::uint8_t{0}
                                          : item->source_combat_factor;
                auto expected_reload = before_reload;
                const auto expected_result = reload_one_clip(
                    expected_reload, capacity, false);
                press(0x40U);
                if (expected_result == ReloadResult::confirmation_required) {
                    if (!field_scene.clips_reload_confirmation_open() ||
                        !write_scene("07_clips_reload_confirmation_no")) {
                        std::cerr << "field clips dump: reload confirmation "
                                     "did not open\n";
                        return 3;
                    }
                    press(0x08U);
                    if (!field_scene.clips_reload_confirmation_yes() ||
                        !write_scene("08_clips_reload_confirmation_yes")) {
                        std::cerr << "field clips dump: reload confirmation "
                                     "did not select Yes\n";
                        return 3;
                    }
                    auto expected_confirmed = before_reload;
                    const auto confirmed_result = reload_one_clip(
                        expected_confirmed, capacity, true);
                    press(0x40U);
                    if (confirmed_result != ReloadResult::reloaded ||
                        !same_clip_record(field_scene.active_runner_record(),
                                          expected_confirmed) ||
                        !write_scene("09_clips_after_reload")) {
                        std::cerr << "field clips dump: confirmed reload did "
                                     "not match converted arithmetic\n";
                        return 3;
                    }
                } else if (expected_result == ReloadResult::reloaded) {
                    if (!same_clip_record(field_scene.active_runner_record(),
                                          expected_reload) ||
                        !write_scene("07_clips_after_reload")) {
                        std::cerr << "field clips dump: direct reload did not "
                                     "match converted arithmetic\n";
                        return 3;
                    }
                } else {
                    if (!same_clip_record(field_scene.active_runner_record(),
                                          before_reload) ||
                        field_scene.clips_menu_message() ==
                            ClipsMenuMessage::none ||
                        !write_scene("07_clips_reload_gate_message")) {
                        std::cerr << "field clips dump: reload gate did not "
                                     "produce a source message\n";
                        return 3;
                    }
                    press(0x10U);
                }
                if (field_scene.clips_menu_message() != ClipsMenuMessage::none) {
                    press(0x10U);
                }
                press(0x10U);
                if (field_scene.clips_menu_open() ||
                    !field_scene.command_overview_open()) {
                    std::cerr << "field clips dump: Clips did not return to "
                                 "the command overview\n";
                    return 3;
                }
                std::cout << "field_clips_dump=written path="
                          << field_clips_dump_path->string() << "\n";
                return 0;
            }
            if (field_hud_qa_path.has_value()) {
                std::filesystem::create_directories(*field_hud_qa_path);
                std::array<QImage, 3> dumps{};
                constexpr std::array<std::string_view, 3> kArchetypeNames{
                    "samurai", "decker", "gator"};
                for (std::uint8_t archetype = 0; archetype < 3U;
                     ++archetype) {
                    StartupIntegrationState hud_state;
                    std::string hud_state_error;
                    if (!build_smoke_state(archetype, hud_state,
                                           hud_state_error)) {
                        std::cerr << hud_state_error << '\n';
                        return 3;
                    }
                    qt_runtime::FieldSceneRuntime hud_scene(
                        [](const auto&) {});
                    if (!hud_scene.bootstrap(hud_state)) {
                        std::cerr << "field HUD QA: " << hud_scene.failure()
                                  << '\n';
                        return 3;
                    }
                    QImage hud_frame(kSceneWidth, kSceneHeight,
                                     QImage::Format_RGB32);
                    hud_frame.fill(Qt::black);
                    QPainter hud_painter(&hud_frame);
                    hud_scene.paint(hud_painter);
                    hud_painter.end();
                    if (!field_hud_chrome_survives(hud_frame, asset_root)) {
                        std::cerr << "field HUD QA: archetype "
                                  << static_cast<int>(archetype)
                                  << " lost the HUD chrome\n";
                        return 3;
                    }
                    if (!field_hud_face_family_survives(
                            hud_frame, asset_root, archetype)) {
                        std::cerr << "field HUD QA: archetype "
                                  << static_cast<int>(archetype)
                                  << " rendered the wrong face family\n";
                        return 3;
                    }
                    const auto path = *field_hud_qa_path /
                                      (std::string("field-hud-") +
                                       std::string(kArchetypeNames[archetype]) +
                                       ".bmp");
                    if (!hud_frame.save(QString::fromStdWString(
                            path.wstring()))) {
                        std::cerr << "field HUD QA: could not write "
                                  << path.string() << '\n';
                        return 3;
                    }
                    dumps[archetype] = std::move(hud_frame);
                }
                for (std::size_t a = 0; a < dumps.size(); ++a) {
                    for (std::size_t b = a + 1U; b < dumps.size(); ++b) {
                        if (dumps[a] == dumps[b]) {
                            std::cerr << "field HUD QA: archetypes " << a
                                      << " and " << b
                                      << " produced identical frames\n";
                            return 3;
                        }
                    }
                }
                std::cout << "field_hud_qa=passed path="
                          << field_hud_qa_path->string() << "\n";
                return 0;
            }
            QImage scene(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
            scene.fill(Qt::black);
            QPainter painter(&scene);
            field_scene.paint(painter);
            painter.end();
            if (field_dump_path.has_value() &&
                !scene.save(QString::fromStdWString(
                    field_dump_path->wstring()))) {
                std::cerr << "field smoke: could not write "
                          << field_dump_path->string() << '\n';
                return 3;
            }
            if (count_playfield_lit(scene) < 100) {
                std::cerr << "field smoke: playfield buried under opaque HUD\n";
                return 3;
            }
            if (!field_hud_chrome_survives(scene, asset_root)) {
                std::cerr << "field smoke: field HUD chrome was not preserved\n";
                return 3;
            }
            if (field_archetype_name.has_value() &&
                !field_hud_face_family_survives(scene, asset_root,
                                                field_archetype)) {
                std::cerr << "field smoke: HUD face family does not match "
                             "the selected archetype\n";
                return 3;
            }
            const auto& buffers = field_scene.state().field_scene_buffers;
            std::cout << "field_smoke=ready scene_reference="
                      << buffers.scene_reference << " camera="
                      << buffers.camera_x << ',' << buffers.camera_y
                      << " field_position="
                      << field_scene.field_entity_world_position().x << ','
                      << field_scene.field_entity_world_position().y
                      << " entity_visible="
                      << (field_scene.field_entity_visible() ? 1 : 0) << '\n';
            return 0;
        }
        GhidraConversionWindow window(asset_root);
        if (startup_qa_path.has_value()) {
            return window.run_startup_qa(*startup_qa_path);
        }
        if (startup_dump_stage.has_value() &&
            startup_dump_selection.has_value() &&
            startup_dump_frame.has_value() && startup_dump_path.has_value()) {
            return window.dump_startup_scene(
                *startup_dump_stage, *startup_dump_selection,
                *startup_dump_frame, *startup_dump_path);
        }
        if (opening_dump_selection.has_value() &&
            opening_dump_start.has_value() && opening_dump_end.has_value() &&
            opening_dump_directory.has_value()) {
            return window.dump_opening_range(
                *opening_dump_selection, *opening_dump_start,
                *opening_dump_end, *opening_dump_directory);
        }
        if (boot_dump_frame.has_value() && boot_dump_path.has_value()) {
            return window.dump_boot_frame(*boot_dump_frame, *boot_dump_path);
        }
        if (intro_to_field_smoke) {
            return window.run_intro_to_field_smoke();
        }
        window.show();
        return application.exec();
    } catch (const std::exception& error) {
        std::cerr << "shadowrun_ghidra_conversion_qt: " << error.what()
                  << '\n';
        return 1;
    }
}

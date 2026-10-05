#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_service_sequence_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"
#include "field_scene_runtime.hpp"
#include "audio_playback.hpp"
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

bool image_has_green_ink(const QImage& image, const int x, const int y,
                         const int width, const int height) {
    const auto right = std::min(image.width(), x + width);
    const auto bottom = std::min(image.height(), y + height);
    for (auto row = std::max(0, y); row < bottom; ++row) {
        for (auto column = std::max(0, x); column < right; ++column) {
            const auto pixel = image.pixelColor(column, row);
            if (pixel.green() > pixel.red() + 24 &&
                pixel.green() > pixel.blue() + 24) {
                return true;
            }
        }
    }
    return false;
}

bool image_region_differs(const QImage& first, const QImage& second,
                          const int x, const int y, const int width,
                          const int height) {
    if (first.size() != second.size()) {
        return true;
    }
    const auto right = std::min(first.width(), x + width);
    const auto bottom = std::min(first.height(), y + height);
    for (auto row = std::max(0, y); row < bottom; ++row) {
        for (auto column = std::max(0, x); column < right; ++column) {
            if (first.pixel(column, row) != second.pixel(column, row)) {
                return true;
            }
        }
    }
    return false;
}

bool image_region_has_color(const QImage& image, const int x, const int y,
                            const int width, const int height,
                            const QColor& expected) {
    const auto right = std::min(image.width(), x + width);
    const auto bottom = std::min(image.height(), y + height);
    for (auto row = std::max(0, y); row < bottom; ++row) {
        for (auto column = std::max(0, x); column < right; ++column) {
            if (image.pixelColor(column, row) == expected) {
                return true;
            }
        }
    }
    return false;
}

bool field_shop_selector_survives(const QImage& scene, const int x) {
    constexpr int kY = 168;
    constexpr int kWidth = 24;
    constexpr int kHeight = 16;
    const QColor selector(196, 190, 39);
    if (scene.width() != kSceneWidth || scene.height() != kSceneHeight ||
        x < 0 || x + 32 > scene.width() || kY + 24 > scene.height()) {
        return false;
    }
    for (int dx = 0; dx < kWidth; ++dx) {
        if (scene.pixelColor(x + dx, kY) != selector ||
            scene.pixelColor(x + dx, kY + kHeight - 1) != selector) {
            return false;
        }
    }
    for (int dy = 0; dy < kHeight; ++dy) {
        if (scene.pixelColor(x, kY + dy) != selector ||
            scene.pixelColor(x + kWidth - 1, kY + dy) != selector) {
            return false;
        }
    }
    int item_ink = 0;
    for (int dy = 0; dy < 24; ++dy) {
        for (int dx = 0; dx < 32; ++dx) {
            const auto is_border =
                dx < kWidth && dy < kHeight &&
                (dx == 0 || dx == kWidth - 1 || dy == 0 ||
                 dy == kHeight - 1);
            const auto pixel = scene.pixelColor(x + dx, kY + dy);
            if (pixel == selector && !is_border) {
                return false;
            }
            if (dx >= 2 && dx < kWidth - 2 && dy >= 2 &&
                dy < kHeight - 2 && pixel.red() < 48 &&
                pixel.green() < 48 && pixel.blue() < 48) {
                ++item_ink;
            }
        }
    }
    return item_ink >= 8;
}

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
                   " [--field-movement-qa <output-dir>]"
                   " [--field-transition-qa <output-dir>]"
                   " [--field-world-activity-qa <output-dir>]"
                   " [--field-secretary-dump <output-dir>]"
                   " [--field-command-dump <file>]"
                       " [--field-command-page <0..6>]"
                   " [--field-action-dump <output-dir>]"
                   " [--field-first-aid-dump <output-dir>]"
                   " [--field-automatic-use-dump <output-dir>]"
                   " [--field-inventory-trade-dump <output-dir>]"
                   " [--field-clips-dump <output-dir>]"
                    " [--field-shop-dump <output-dir>]"
                   " [--field-magic-dump <output-dir>]"
                   " [--field-magic-spell-dump <output-dir>]"
                   " [--field-archetype <samurai|decker|gator>]"
                   " [--field-hud-qa <output-dir>]"
                   " [--field-hotel-qa <output-dir>]"
                   " [--field-command-reopen-smoke]"
                   " [--field-dialogue-dump <prompt> <file>]"
                   " [--field-mode-text-dump <variant> <file>]"
                   " [--field-interaction-dump <variant> <file>]"
                   " [--boot-dump-frame <frame> <file>]"
                   " [--boot-fasa-qa <output-dir>]"
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
        if (argument == "--boot-fasa-qa" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-movement-qa" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-transition-qa" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-world-activity-qa" && index + 1 < argc) {
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
        if (argument == "--field-first-aid-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-inventory-trade-dump" &&
            index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-clips-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-shop-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-magic-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-magic-spell-dump" && index + 1 < argc) {
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
        if (argument == "--field-hotel-qa" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-secretary-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-posture-dump" && index + 1 < argc) {
            ++index;
            continue;
        }
        if (argument == "--field-dialogue-dump" && index + 2 < argc) {
            index += 2;
            continue;
        }
        if (argument == "--field-mode-text-dump" && index + 2 < argc) {
            index += 2;
            continue;
        }
        if (argument == "--field-interaction-dump" && index + 2 < argc) {
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
         QPoint(10, 100)});
}

bool field_command_shell_survives(
    const QImage& scene, const std::filesystem::path& asset_root,
    const std::string& background_file) {
    const QImage shell(QString::fromStdWString(
        (asset_root / "menu" / "field" / background_file).wstring()));
    return source_shell_pixels_survive(
        scene, shell,
        {QPoint(10, 10), QPoint(100, 0), QPoint(120, 0),
         QPoint(10, 100)});
}

bool field_inventory_selector_survives(const QImage& scene,
                                       const std::uint8_t primary,
                                       const std::uint8_t secondary) {
    constexpr int kItemXOrigin = 16;
    constexpr int kItemYOrigin = 120;
    constexpr int kCellWidth = 40;
    constexpr int kCellHeight = 24;
    constexpr int kOutlineWidth = 24;
    constexpr int kOutlineHeight = 16;
    const QColor gold(196, 190, 39);
    const auto x = kItemXOrigin + static_cast<int>(primary) * kCellWidth;
    const auto y = kItemYOrigin + static_cast<int>(secondary) * kCellHeight;
    if (x < 0 || y < 0 || x + kOutlineWidth > scene.width() ||
        y + kOutlineHeight > scene.height()) {
        return false;
    }

    // The one-pixel gold stroke hugs the 24x16 item image. Reject the previous
    // wider cursor-footprint rectangle at its upper-left and lower-right.
    if (scene.pixelColor(x - 2, y - 2) == gold ||
        scene.pixelColor(x + kOutlineWidth + 1,
                         y + kOutlineHeight + 1) == gold) {
        return false;
    }
    for (int offset = 0; offset < kOutlineWidth; ++offset) {
        if (scene.pixelColor(x + offset, y) != gold ||
            scene.pixelColor(x + offset, y + kOutlineHeight - 1) != gold) {
            return false;
        }
    }
    for (int offset = 0; offset < kOutlineHeight; ++offset) {
        if (scene.pixelColor(x, y + offset) != gold ||
            scene.pixelColor(x + kOutlineWidth - 1, y + offset) != gold) {
            return false;
        }
    }
    for (int inner_y = y + 1; inner_y < y + kOutlineHeight - 1; ++inner_y) {
        for (int inner_x = x + 1; inner_x < x + kOutlineWidth - 1;
             ++inner_x) {
            if (scene.pixelColor(inner_x, inner_y) == gold) {
                return false;
            }
        }
    }

    // A populated item remains visible inside the focus frame; the 24x16
    // glyph in this QA record has opaque dark ink at more than its outline.
    int item_ink_pixels = 0;
    for (int item_y = y + 2; item_y < y + 18; ++item_y) {
        for (int item_x = x + 2; item_x < x + 26; ++item_x) {
            const auto pixel = scene.pixelColor(item_x, item_y);
            if (pixel.red() < 48 && pixel.green() < 48 && pixel.blue() < 48) {
                ++item_ink_pixels;
            }
        }
    }
    return item_ink_pixels >= 8;
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
    // Palette selection is record-dependent, so either source variant may be
    // correct for a given live item/class combination. Require one complete
    // extracted face to match at the fixed F1C6 presentation location.
    for (std::size_t palette = 0; palette < 2U; ++palette) {
        const QImage face(QString::fromStdWString(
            (asset_root / "field" / "hud" / "party" /
             ("face-" + std::to_string(archetype_index) + "-palette-" +
              std::to_string(palette) + ".bmp")).wstring()));
        if (face.isNull() || face.size() != QSize(32, 32)) {
            continue;
        }
        bool matches = true;
        int opaque = 0;
        for (int y = 0; y < 32 && matches; ++y) {
            for (int x = 0; x < 32; ++x) {
                const auto source = face.pixelColor(x, y);
                if (source.alpha() == 0) {
                    continue;
                }
                ++opaque;
                if (scene.pixelColor(280 + x, 24 + y) != source) {
                    matches = false;
                    break;
                }
            }
        }
        if (matches && opaque > 64) {
            return true;
        }
    }
    return false;
}

bool field_hud_party_slot_asset_survives(
    const QImage& scene, const std::filesystem::path& asset_root,
    const std::string& relative_path, const int x, const int y,
    const QSize expected_size) {
    const QImage asset(QString::fromStdWString(
        (asset_root / std::filesystem::path(relative_path)).wstring()));
    if (asset.isNull() || asset.size() != expected_size) {
        return false;
    }
    std::size_t opaque = 0U;
    for (int row = 0; row < asset.height(); ++row) {
        for (int column = 0; column < asset.width(); ++column) {
            const auto source = asset.pixelColor(column, row);
            if (source.alpha() == 0) {
                continue;
            }
            ++opaque;
            if (scene.pixelColor(x + column, y + row) != source) {
                return false;
            }
        }
    }
    return opaque > 8U;
}

bool field_hud_meter_assets_survive(const QImage& scene,
                                    const std::filesystem::path& asset_root,
                                    const std::uint8_t delay_meter,
                                    const std::uint8_t behavior_meter,
                                    const bool primary_runner) {
    const auto delay_index = std::min<std::uint8_t>(delay_meter, 10U);
    const auto behavior_index = std::min<std::uint8_t>(behavior_meter, 10U);
    const auto delay = QImage(QString::fromStdWString(
        (asset_root / "field" / "hud" /
         ("delay-meter-" + std::to_string(delay_index) + ".bmp"))
            .wstring()));
    const auto behavior = QImage(QString::fromStdWString(
        (asset_root / "field" / "hud" /
         ((primary_runner ? "behavior-meter-primary-"
                          : "behavior-meter-companion-") +
          std::to_string(behavior_index) + ".bmp"))
            .wstring()));
    if (delay.size() != QSize(8, 32) || behavior.size() != QSize(32, 8) ||
        scene.width() < 312 || scene.height() < 64) {
        return false;
    }

    const auto matches_at = [&scene](const QImage& asset, const int x,
                                     const int y) {
        std::size_t checked = 0U;
        for (int row = 0; row < asset.height(); ++row) {
            for (int column = 0; column < asset.width(); ++column) {
                const auto pixel = asset.pixelColor(column, row);
                if (pixel.alpha() == 0) {
                    continue;
                }
                if (scene.pixelColor(x + column, y + row) != pixel) {
                    return false;
                }
                ++checked;
            }
        }
        return checked > 0U;
    };
    return matches_at(delay, 272, 24) && matches_at(behavior, 280, 56);
}

class GhidraConversionWindow final : public QWidget {
public:
    explicit GhidraConversionWindow(std::filesystem::path asset_root,
                                    QWidget* parent = nullptr)
        : QWidget(parent),
          asset_root_(std::move(asset_root)),
          field_scene_runtime_([this](const std::string_view boundary) {
              note_presentation_boundary(boundary);
          }, [this](const std::uint16_t event) {
              audio_playback_.play_event(event);
          }),
          audio_playback_(
              [this](const std::string_view alias) -> std::filesystem::path {
                  const auto* record =
                      state_.asset_catalog.catalog.find(std::string(alias));
                  return record == nullptr ? std::filesystem::path{}
                                           : record->absolute_path;
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

        const auto scene = render_scene();
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

    int run_boot_fasa_qa(const std::filesystem::path& output_directory) {
        host_frame_timer_.stop();
        std::filesystem::create_directories(output_directory);
        std::ofstream trace(output_directory / "boot-fasa-qa.tsv");
        if (!trace) {
            std::cerr << "boot FASA QA: could not open trace in "
                      << output_directory.string() << '\n';
            return 3;
        }
        trace << "label\tframe\tstage\tpalette_pass\tvblank_phase\t"
                 "hold_remaining\tfigure_fade_steps\tlightning_batches\n";

        const auto fail = [](const std::string& message) {
            std::cerr << "boot FASA QA: " << message << '\n';
            return 3;
        };
        const auto stage = [this] {
            return state_.pre_title.presentation_stage;
        };
        const auto save_snapshot = [&](const std::string& label,
                                       const std::string& filename) {
            const auto scene = render_scene();
            const auto path = output_directory / filename;
            if (scene.size() != QSize(kSceneWidth, kSceneHeight) ||
                !scene.save(QString::fromStdWString(path.wstring()))) {
                return false;
            }
            trace << label << '\t' << state_.frame.frame_count << '\t'
                  << static_cast<unsigned>(stage()) << '\t'
                  << static_cast<unsigned>(state_.pre_title
                                               .palette_transition_passes_completed)
                  << '\t'
                  << static_cast<unsigned>(state_.pre_title
                                               .palette_transition_vblank_phase)
                  << '\t' << state_.pre_title.fasa_hold_frames << '\t'
                  << static_cast<unsigned>(state_.pre_title
                                               .figure_palette_fade_steps)
                  << '\t'
                  << (state_.pre_title.sega_lightning_batches_active ? 1 : 0)
                  << '\n';
            return true;
        };
        const auto is_black = [](const QImage& scene) {
            for (int y = 0; y < scene.height(); ++y) {
                for (int x = 0; x < scene.width(); ++x) {
                    const auto pixel = scene.pixelColor(x, y);
                    if (pixel.red() != 0 || pixel.green() != 0 ||
                        pixel.blue() != 0) {
                        return false;
                    }
                }
            }
            return true;
        };
        constexpr std::uint32_t kMaximumSourceFrames = 700U;
        std::uint32_t advanced_frames = 0U;
        const auto advance_until = [&](const auto& predicate) {
            while (!predicate()) {
                if (state_.phase != StartupIntegrationPhase::pre_title ||
                    advanced_frames >= kMaximumSourceFrames) {
                    return false;
                }
                advance_one_source_frame();
                ++advanced_frames;
            }
            return true;
        };
        if (state_.phase != StartupIntegrationPhase::pre_title ||
            !advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::sega_shock;
            }) ||
            !save_snapshot("sega_shock_start", "sega-shock-start.bmp")) {
            return fail("did not reach the SEGA shock phase");
        }
        if (!advance_until([&] {
                return state_.pre_title.sega_lightning_batches_active;
            }) ||
            !save_snapshot("sega_shock_lightning",
                           "sega-shock-lightning.bmp")) {
            return fail("0x5D996 did not publish the shock lightning batches");
        }
        if (!advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::sega_strobe;
            })) {
            return fail("did not reach the ROM SEGA logo strobe");
        }
        if (!state_.pre_title.sega_lightning_batches_active ||
            state_.pre_title.figure_palette_fade_steps != 0U ||
            !save_snapshot("sega_strobe_start", "sega-strobe-start.bmp")) {
            return fail("strobe activation lost the source-built lightning or figure");
        }
        advance_one_source_frame();
        if (stage() !=
                PreTitleSequenceState::PresentationStage::sega_strobe ||
            state_.pre_title.sega_lightning_batches_active ||
            !save_snapshot("sega_strobe_lightning_cleared",
                           "sega-strobe-lightning-cleared.bmp")) {
            return fail("stale lightning survived the source strobe handoff");
        }
        if (!advance_until([&] {
                return state_.pre_title.figure_palette_fade_steps == 1U;
            }) ||
            !save_snapshot("sega_figure_fade_step_1",
                           "sega-figure-fade-step-1.bmp")) {
            return fail("first figure CRAM fade step did not arrive");
        }
        if (!advance_until([&] {
                return state_.pre_title.figure_palette_fade_steps ==
                       kPreTitleFigurePaletteLevelsToBlack;
            }) ||
            stage() != PreTitleSequenceState::PresentationStage::sega_strobe) {
            return fail("figure palette did not reach black during the strobe");
        }
        const auto resolved_sega = render_scene();
        QImage expected_sega(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        expected_sega.fill(Qt::black);
        {
            QPainter painter(&expected_sega);
            draw_boot_snow(painter);
            draw_alias(
                painter,
                "boot.sega.strobe." +
                    std::to_string(state_.pre_title.strobe_phase % 8U));
        }
        if (resolved_sega != expected_sega ||
            !save_snapshot("sega_resolved", "sega-resolved.bmp")) {
            return fail("resolved SEGA frame retained the figure or lightning");
        }
        if (!advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::fasa_fade_in;
            })) {
            return fail("did not reach the ROM FASA fade-in stage");
        }
        if (state_.pre_title.palette_transition_passes_completed != 0U ||
            state_.pre_title.palette_transition_vblank_phase != 0U ||
            !save_snapshot("fasa_fade_in_start", "fade-in-start.bmp")) {
            return fail("fade-in did not begin from a clean zero-pass state");
        }
        if (!is_black(render_scene())) {
            return fail("FASA fade-in must start from black");
        }

        advance_one_source_frame();
        advance_one_source_frame();
        if (state_.pre_title.palette_transition_passes_completed != 1U ||
            state_.pre_title.palette_transition_vblank_phase != 0U ||
            !save_snapshot("fasa_fade_in_pass_1", "fade-in-pass-1.bmp")) {
            return fail("first fade-in palette pass did not take two VBlanks");
        }
        const auto first_fade_in = render_scene();
        if (is_black(first_fade_in)) {
            return fail("first FASA fade-in pass did not reveal color");
        }
        if (!advance_until([&] {
                return stage() ==
                    PreTitleSequenceState::PresentationStage::fasa_hold;
            })) {
            return fail("FASA fade-in did not reach its hold stage");
        }
        if (state_.pre_title.fasa_hold_frames != 0x0079U ||
            state_.pre_title.palette_transition_passes_completed !=
                kPreTitlePaletteTransitionPasses) {
            return fail("FASA fade-in or source hold length is incorrect");
        }
        const auto fasa_full = render_scene();
        const auto* fasa_record =
            state_.asset_catalog.catalog.find("boot.fasa");
        if (fasa_record == nullptr) {
            return fail("boot.fasa is absent from the direct asset catalog");
        }
        QImage fasa_image(
            QString::fromStdWString(fasa_record->absolute_path.wstring()));
        if (fasa_image.isNull()) {
            return fail("could not load the named boot.fasa asset");
        }
        QImage expected_fasa(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        expected_fasa.fill(Qt::black);
        {
            QPainter painter(&expected_fasa);
            painter.drawImage(fasa_record->origin_x, fasa_record->origin_y,
                              fasa_image);
        }
        if (expected_fasa != fasa_full) {
            return fail("FASA screen contains stale Sega graphics or controls");
        }
        if (!save_snapshot("fasa_full_hold", "actual.bmp")) {
            return fail("could not write the required actual.bmp image");
        }

        for (std::uint32_t frame = 0; frame < 120U; ++frame) {
            advance_one_source_frame();
        }
        if (stage() != PreTitleSequenceState::PresentationStage::fasa_hold ||
            state_.pre_title.fasa_hold_frames != 1U ||
            render_scene() != fasa_full ||
            !save_snapshot("fasa_hold_last", "hold-last.bmp")) {
            return fail("FASA hold must stay visually stable for 121 VBlanks");
        }
        advance_one_source_frame();
        if (stage() !=
                PreTitleSequenceState::PresentationStage::fasa_fade_out ||
            state_.pre_title.fasa_hold_frames != 0U ||
            state_.pre_title.palette_transition_passes_completed != 0U ||
            render_scene() != fasa_full ||
            !save_snapshot("fasa_fade_out_start", "fade-out-start.bmp")) {
            return fail("FASA fade-out must begin after the full hold");
        }

        advance_one_source_frame();
        advance_one_source_frame();
        if (state_.pre_title.palette_transition_passes_completed != 1U ||
            state_.pre_title.palette_transition_vblank_phase != 0U ||
            !save_snapshot("fasa_fade_out_pass_1", "fade-out-pass-1.bmp") ||
            render_scene() == fasa_full) {
            return fail("first fade-out palette pass did not darken FASA");
        }
        if (!advance_until([&] {
                return stage() ==
                           PreTitleSequenceState::PresentationStage::fasa_fade_out &&
                       state_.pre_title.palette_transition_passes_completed ==
                           16U &&
                       state_.pre_title.palette_transition_vblank_phase == 0U;
            })) {
            return fail("FASA fade-out did not reach its final black hold");
        }
        const auto fasa_black = render_scene();
        if (!is_black(fasa_black) ||
            !save_snapshot("fasa_fade_out_black", "fade-out-black.bmp")) {
            return fail("FASA must be fully black before title handoff");
        }
        advance_one_source_frame();
        advance_one_source_frame();
        if (state_.phase != StartupIntegrationPhase::title_ready ||
            !state_.pre_title_complete || state_.frame.frame_count != 560U ||
            !save_snapshot("title_handoff", "title-handoff.bmp")) {
            return fail("title handoff must follow the complete 560-VBlank sequence");
        }
        if (render_scene() == fasa_full) {
            return fail("title handoff retained the previous FASA screen");
        }
        const auto frame_dump_path =
            output_directory / "boot-dump-frame-373.bmp";
        {
            GhidraConversionWindow frame_dump_window(asset_root_);
            if (frame_dump_window.dump_boot_frame(373U, frame_dump_path) !=
                0) {
                return fail("--boot-dump-frame could not render frame 373");
            }
        }
        const QImage dumped_fade_frame(
            QString::fromStdWString(frame_dump_path.wstring()));
        if (dumped_fade_frame.convertToFormat(QImage::Format_RGB32) !=
            first_fade_in) {
            return fail("--boot-dump-frame skipped the FASA palette fade");
        }
        std::cout << "boot_fasa_qa=passed source_frames="
                  << state_.frame.frame_count
                  << " fade_passes="
                  << static_cast<unsigned>(kPreTitlePaletteTransitionPasses)
                  << " hold=" << 0x0079U << " actual="
                  << (output_directory / "actual.bmp").string() << '\n';
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
        trace << "label\tphase\tboot_frame\topening_frame\tstage\tscene"
                 "\tpalette_weight\n";

        const auto write_snapshot = [&](const std::string& name) {
            const auto path = output_directory / (name + ".bmp");
            const auto scene = render_scene();
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.seattle_outline_frame != 0U &&
                state_.opening_intro.stage == OpeningIntroStage::schedule &&
                opening_scene_ == OpeningIntroScene::seattle) {
                const auto phase = static_cast<std::uint8_t>(std::min<
                    std::uint16_t>(
                        (state_.opening_intro.seattle_outline_frame - 1U) /
                            2U,
                        4U));
                const auto outline =
                    opening_intro_seattle_outline_plan(phase);
                const auto bottom_midpoint =
                    QPoint(outline.x + outline.width / 2,
                           outline.y + outline.height - 1);
                if (scene.pixelColor(bottom_midpoint) !=
                    QColor(0xE0, 0xBB, 0x4C)) {
                    std::cerr
                        << "startup qa: Seattle focus box was not a fine "
                           "amber outline in phase "
                        << static_cast<unsigned>(phase) << '\n';
                    return false;
                }
            }
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.scene_palette_weight == 0U) {
                for (int y = 0; y < scene.height(); ++y) {
                    for (int x = 0; x < scene.width(); ++x) {
                        if (scene.pixelColor(x, y) != QColor(Qt::black)) {
                            std::cerr
                                << "startup qa: zero-weight scene palette "
                                   "did not render black\n";
                            return false;
                        }
                    }
                }
            }
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
            if (state_.phase == StartupIntegrationPhase::opening_intro &&
                state_.opening_intro.scene == OpeningIntroScene::cab_map &&
                state_.opening_intro.post_intro_mode == 2U &&
                state_.opening_intro.post_intro_caption_visible) {
                const auto caption = cab_mode_selection_plan(
                    state_.opening_intro.post_intro_mode,
                    state_.opening_intro.post_intro_phase);
                const auto font = draw_asset_cache_.find("menu.startup.font");
                constexpr int kRedmondLeadingSpaces = 2;
                constexpr char kRedmondFirstLetter = 'R';
                const auto glyph = static_cast<int>(
                    static_cast<unsigned char>(kRedmondFirstLetter) - 0x20U);
                const auto glyph_x = (glyph % 16) * 8;
                const auto glyph_y = (glyph / 16) * 8;
                bool first_letter_is_white = false;
                if (font != draw_asset_cache_.end()) {
                    for (int y = 0; y < 8; ++y) {
                        for (int x = 0; x < 8; ++x) {
                            const auto source = font->second.pixelColor(
                                glyph_x + x, glyph_y + y);
                            const auto coverage = std::max(
                                {source.red(), source.green(), source.blue()});
                            if (source.alpha() == 0 || coverage == 0) {
                                continue;
                            }
                            const auto pixel = scene.pixelColor(
                                caption.caption_x +
                                    kRedmondLeadingSpaces * 8 + x,
                                caption.caption_y + y);
                            if (pixel.red() < 200 || pixel.green() < 200 ||
                                pixel.blue() < 200) {
                                std::cerr << "startup qa: Redmond cab caption "
                                             "was not rendered white\n";
                                return false;
                            }
                            first_letter_is_white = true;
                        }
                    }
                }
                if (!first_letter_is_white) {
                    std::cerr << "startup qa: Redmond cab caption glyph was "
                                 "not found\n";
                    return false;
                }
                const auto cursor =
                    draw_asset_cache_.find("intro.cab.cursor.top-left");
                int cursor_ink_matches = 0;
                if (cursor != draw_asset_cache_.end()) {
                    for (int y = 0; y < cursor->second.height(); ++y) {
                        for (int x = 0; x < cursor->second.width(); ++x) {
                            const auto source =
                                cursor->second.pixelColor(x, y);
                            if (source.alpha() == 0 ||
                                (source.red() == 0 && source.green() == 0 &&
                                 source.blue() == 0)) {
                                continue;
                            }
                            const auto pixel = scene.pixelColor(
                                caption.cursor_x + x,
                                caption.cursor_y + y);
                            if (pixel == source) {
                                ++cursor_ink_matches;
                            }
                        }
                    }
                }
                if (cursor_ink_matches < 8) {
                    std::cerr << "startup qa: cab cursor top-left tile did "
                                 "not render at its ROM Plane-A position\n";
                    return false;
                }
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
                  << opening_intro_scene_name(opening_scene_) << '\t'
                  << static_cast<unsigned>(
                         state_.opening_intro.scene_palette_weight)
                  << '\n';
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

        constexpr std::array<std::uint32_t, 23> kOpeningCheckpoints{
            300U, 600U, 604U, 606U, 608U, 610U, 612U, 614U,
            620U, 628U, 630U, 648U, 654U, 662U, 682U, 900U,
            1200U, 1500U, 1800U, 2100U, 2400U, 2700U, 3000U};
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
        if (state_.phase == StartupIntegrationPhase::pre_title) {
            using PresentationStage =
                PreTitleSequenceState::PresentationStage;
            const auto stage = state_.pre_title.presentation_stage;
            if (stage == PresentationStage::sega_fade_out ||
                stage == PresentationStage::fasa_fade_out) {
                return transition_genesis_cram_image(
                    scene,
                    state_.pre_title.palette_transition_passes_completed,
                    true);
            }
            if (stage == PresentationStage::fasa_fade_in) {
                return transition_genesis_cram_image(
                    scene,
                    state_.pre_title.palette_transition_passes_completed,
                    false);
            }
        }
        if (state_.phase == StartupIntegrationPhase::opening_intro &&
            state_.opening_intro.scene_palette_weight != 255U) {
            return scale_genesis_image_rgb(
                scene, state_.opening_intro.scene_palette_weight);
        }
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
        startup_callbacks_.pre_title.latch_controller_events = [this] {
            note_presentation_boundary(
                "pre_title.latch_controller_events");
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
                audio_playback_.play_event(command);
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
        if (stage == PresentationStage::fasa_fade_in ||
            stage == PresentationStage::fasa_hold ||
            stage == PresentationStage::fasa_fade_out) {
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

        if (state_.pre_title.sega_lightning_batches_active) {
            const auto phase = static_cast<int>(
                state_.pre_title.sega_lightning_phase % 6U);
            draw_alias(painter,
                       "boot.sega.lightning." + std::to_string(phase));
        }

        const auto* record = state_.asset_catalog.catalog.find("boot.guy");
        const auto image = draw_asset_cache_.find("boot.guy");
        if (record != nullptr && image != draw_asset_cache_.end() &&
            state_.pre_title.figure_palette_fade_steps <
                kPreTitleFigurePaletteLevelsToBlack) {
            const auto y = static_cast<int>(state_.pre_title.figure_y) -
                           0xD0 + record->origin_y;
            const auto figure = transition_genesis_cram_image(
                image->second,
                state_.pre_title.figure_palette_fade_steps, true);
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
        // The source tile stream in 0x5B378 presents a fine amber focus box.
        // Drawing whole tile-width bars turned it into a thick white frame.
        constexpr int kLine = 1;
        const QColor outline(0xE0, 0xBB, 0x4C);
        if (width <= 0 || height <= 0) {
            return;
        }
        painter.fillRect(x, y, width, kLine, outline);
        painter.fillRect(x, y + height - kLine, width, kLine, outline);
        painter.fillRect(x, y, kLine, height, outline);
        painter.fillRect(x + width - kLine, y, kLine, height, outline);
    }

    void draw_intro_outline(QPainter& painter,
                            const OpeningIntroOutlinePlan& plan) const {
        draw_intro_outline(painter, plan.x, plan.y, plan.width, plan.height);
    }

    void draw_cab_mode_selection(QPainter& painter) const {
        const auto plan = cab_mode_selection_plan(
            state_.opening_intro.post_intro_mode,
            state_.opening_intro.post_intro_phase);
        // 0x13726 stamps the 0xD4446 border at its 0xD445D Plane-A position.
        // That plan is already in video coordinates; the cab map is a
        // separate Plane-B asset and must not contribute its crop offset.
        const auto tile = [this](const std::string_view name) -> const QImage* {
            const auto found = draw_asset_cache_.find(
                std::string("intro.cab.cursor.") + std::string(name));
            return found == draw_asset_cache_.end() ? nullptr
                                                    : &found->second;
        };
        const auto x = plan.cursor_x;
        const auto y = plan.cursor_y;
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
                              plan.caption, QColor(Qt::white));
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
    AudioPlaybackService audio_playback_;

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

int run_field_world_activity_qa(
    FieldSceneRuntime& field_scene,
    const std::filesystem::path& asset_root,
    const std::filesystem::path& output,
    std::vector<std::uint16_t>& field_audio_events) {
    std::filesystem::create_directories(output);
    const auto render_field = [&field_scene] {
        QImage image(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        image.fill(Qt::black);
        QPainter painter(&image);
        field_scene.paint(painter);
        painter.end();
        return image;
    };
    const auto render_field_scene = [&field_scene] {
        QImage image(kSceneWidth, kSceneHeight, QImage::Format_RGB32);
        image.fill(Qt::black);
        QPainter painter(&image);
        field_scene.paint_field_scene_for_dump(painter);
        painter.end();
        return image;
    };
    const auto save = [&output](const std::filesystem::path& path,
                                const QImage& image) {
        return image.save(QString::fromStdWString((output / path).wstring()));
    };
    const auto initial = render_field();
    if (!save("initial.bmp", initial)) {
        std::cerr << "field world-activity QA: initial dump failed\n";
        return 3;
    }
    if (!field_scene.initialize_world_activity_for_dump(2U, 0x1234U)) {
        std::cerr << "field world-activity QA: source table, bound, or scene preparation failed\n";
        return 3;
    }
    const auto& activity = field_scene.world_activity_state_for_dump();
    const auto floor = field_scene.security_floor_index_for_dump();
    const auto resource_scene_entry = render_field();
    if (!save("resource-scene-entry.bmp", resource_scene_entry)) {
        std::cerr << "field world-activity QA: resource-scene entry dump failed\n";
        return 3;
    }
    for (std::uint8_t frame = 0U; frame < 30U; ++frame) {
        field_scene.advance_frame();
    }
    const auto resource_scene = render_field();
    for (std::uint8_t frame = 0U; frame < 30U; ++frame) {
        field_scene.advance_frame();
    }
    const auto resource_scene_held = render_field();
    if (activity.scene_id != 0x1AU ||
        activity.mode != WorldActivityMode::resource_activity ||
        activity.resource_random_index_bound != 3U ||
        activity.selected_resource_random_index != 2U ||
        activity.resource_marker_ordinal != 2U ||
        activity.resource_cursor.cursor != 0x18 ||
        activity.resource_cursor.resource_index != 0U ||
        floor != std::optional<std::uint8_t>{2U} ||
        field_scene.field_scene_reference_for_dump() != 0x1AU ||
        initial.copy(0, 0, 272, 224) ==
            resource_scene.copy(0, 0, 272, 224) ||
        !field_hud_chrome_survives(resource_scene, asset_root) ||
        !save("resource-scene.bmp", resource_scene) ||
        !save("resource-scene-held.bmp", resource_scene_held)) {
        std::cerr << "field world-activity QA: selected scene, cursor, floor, or replacement frame diverged\n";
        return 3;
    }

    // The coordinator's pending handler reads the selected FFE12C value from
    // runtime state rather than taking the floor as a QA argument.
    if (!field_scene.dispatch_selected_security_terminal_for_dump()) {
        std::cerr << "field world-activity QA: selected Security Terminal handler did not open\n";
        return 3;
    }
    const auto resource_scene_before_prompt = render_field_scene();
    if (!save("resource-scene-before-prompt.bmp",
              resource_scene_before_prompt)) {
        std::cerr << "field world-activity QA: pre-prompt scene dump failed\n";
        return 3;
    }
    const auto terminal_menu = render_field();
    if (field_scene.field_interaction_panel().option_count != 2U ||
        !save("security-terminal.bmp", terminal_menu)) {
        std::cerr << "field world-activity QA: terminal choice screen failed\n";
        return 3;
    }

    // A high but valid active-runner Electronics skill and the fixed ROM PRNG
    // seed exercise the successful file result.
    field_scene.active_runner_record().skills[9] = 10U;
    for (std::uint8_t frame = 0U; frame < 10U; ++frame) {
        (void)field_scene.advance_field_interaction_for_dump(0U);
    }
    if (field_scene.advance_field_interaction_for_dump(0x4000U) !=
        FieldInteractionAdvanceResult::prompt_advanced) {
        std::cerr << "field world-activity QA: terminal command did not advance\n";
        return 3;
    }
    const auto& result_panel = field_scene.field_interaction_panel();
    bool third_floor_visible = false;
    for (std::size_t index = 0U; index < result_panel.text_segment_count;
         ++index) {
        if (result_panel.text_segments[index].find("third floor") !=
            std::string_view::npos) {
            third_floor_visible = true;
        }
    }
    const auto floor_result = render_field();
    if (!third_floor_visible || !result_panel.source_complete ||
        field_audio_events != std::vector<std::uint16_t>{0x30U} ||
        !save("floor-result.bmp", floor_result) ||
        !save("actual.bmp", floor_result)) {
        std::cerr << "field world-activity QA: selected floor text, source result, or audio event diverged\n";
        return 3;
    }

    for (std::uint8_t frame = 0U; frame < 30U; ++frame) {
        (void)field_scene.advance_field_interaction_for_dump(0U);
    }
    if (field_scene.advance_field_interaction_for_dump(0x1000U) !=
            FieldInteractionAdvanceResult::returned_to_field ||
        field_scene.field_interaction_open()) {
        std::cerr << "field world-activity QA: terminal page did not return to the selected scene\n";
        return 3;
    }
    const auto returned_field = render_field_scene();
    const bool return_dump_saved = save("return-field.bmp", returned_field);
    if (returned_field != resource_scene_before_prompt ||
        !return_dump_saved) {
        std::cerr << "field world-activity QA: terminal UI left stale graphics over the selected scene\n";
        return 3;
    }
    std::cout << "field_world_activity_qa=passed scene=0x1A bound=3 floor=2 cursor=0x18 terminal_floor_text=third-floor artifacts="
              << output.string() << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        bool field_smoke = false;
        bool field_smoke_move_right = false;
        bool field_command_reopen_smoke = false;
        bool field_cyberware_empty_qa = false;
        bool intro_to_field_smoke = false;
        std::optional<std::filesystem::path> field_dump_path;
        std::optional<std::filesystem::path> field_movement_qa_path;
        std::optional<std::filesystem::path> field_transition_qa_path;
        std::optional<std::filesystem::path> field_world_activity_qa_path;
        std::optional<std::filesystem::path> field_hud_qa_path;
        std::optional<std::filesystem::path> field_hotel_qa_path;
        std::optional<std::filesystem::path> field_command_dump_path;
        std::optional<std::uint8_t> field_command_page;
        std::optional<std::filesystem::path> field_cyberware_dump_path;
        std::optional<std::filesystem::path> field_secretary_dump_path;
        std::optional<std::filesystem::path> field_posture_dump_path;
        std::optional<std::filesystem::path> field_action_dump_path;
        std::optional<std::filesystem::path> field_player_action_dump_path;
        std::optional<std::filesystem::path> field_first_aid_dump_path;
        std::optional<std::filesystem::path> field_automatic_use_dump_path;
        std::optional<std::filesystem::path> field_inventory_trade_dump_path;
        std::optional<std::filesystem::path> field_clips_dump_path;
        std::optional<std::filesystem::path> field_shop_dump_path;
        std::optional<std::filesystem::path> field_magic_dump_path;
        std::optional<std::filesystem::path> field_magic_spell_dump_path;
        std::optional<std::filesystem::path>
            field_attributes_skills_dump_path;
        std::optional<std::string> field_archetype_name;
        std::optional<std::string> field_dialogue_dump_prompt;
        std::optional<std::filesystem::path> field_dialogue_dump_path;
        std::optional<std::string> field_mode_text_dump_variant;
        std::optional<std::filesystem::path> field_mode_text_dump_path;
        std::optional<std::string> field_interaction_dump_variant;
        std::optional<std::filesystem::path> field_interaction_dump_path;
        std::optional<std::uint32_t> boot_dump_frame;
        std::optional<std::filesystem::path> boot_dump_path;
        std::optional<std::filesystem::path> boot_fasa_qa_path;
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
                       " [--field-movement-qa <output-dir>]"
                       " [--field-transition-qa <output-dir>]"
                       " [--field-world-activity-qa <output-dir>]"
                       " [--field-command-dump <file>]"
                       " [--field-command-page <0..6>]"
                       " [--field-cyberware-dump <file>]"
                       " [--field-cyberware-empty-qa]"
                       " [--field-action-dump <output-dir>]"
                       " [--field-player-action-dump <output-dir>]"
                       " [--field-first-aid-dump <output-dir>]"
                       " [--field-automatic-use-dump <output-dir>]"
                       " [--field-inventory-trade-dump <output-dir>]"
                       " [--field-clips-dump <output-dir>]"
                       " [--field-shop-dump <output-dir>]"
                       " [--field-magic-dump <output-dir>]"
                       " [--field-magic-spell-dump <output-dir>]"
                       " [--field-attributes-skills-dump <output-dir>]"
                       " [--field-archetype <samurai|decker|gator>]"
                       " [--field-hud-qa <output-dir>]"
                       " [--field-hotel-qa <output-dir>]"
                       " [--field-command-reopen-smoke]"
                       " [--field-dialogue-dump <prompt> <file>]"
                       " [--field-mode-text-dump <variant> <file>]"
                       " [--field-interaction-dump <variant> <file>]"
                       " [--boot-dump-frame <frame> <file>]"
                       " [--boot-fasa-qa <output-dir>]"
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
            if (std::string(argv[index]) == "--field-movement-qa" &&
                index + 1 < argc) {
                field_smoke = true;
                field_movement_qa_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-transition-qa" &&
                index + 1 < argc) {
                field_smoke = true;
                field_transition_qa_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-world-activity-qa" &&
                index + 1 < argc) {
                field_smoke = true;
                field_world_activity_qa_path =
                    std::filesystem::path(argv[++index]);
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
            if (std::string(argv[index]) == "--field-player-action-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_player_action_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-first-aid-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_first_aid_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-automatic-use-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_automatic_use_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) ==
                    "--field-inventory-trade-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_inventory_trade_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-clips-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_clips_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-shop-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_shop_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-magic-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_magic_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-magic-spell-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_magic_spell_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) ==
                    "--field-attributes-skills-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_attributes_skills_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-cyberware-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_cyberware_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-cyberware-empty-qa") {
                field_smoke = true;
                field_cyberware_empty_qa = true;
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
            if (std::string(argv[index]) == "--field-hotel-qa" &&
                index + 1 < argc) {
                field_smoke = true;
                field_hotel_qa_path = std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-command-page" &&
                index + 1 < argc) {
                field_smoke = true;
                field_command_page = static_cast<std::uint8_t>(
                    std::stoul(argv[++index]));
            }
            if (std::string(argv[index]) == "--field-secretary-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_secretary_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-posture-dump" &&
                index + 1 < argc) {
                field_smoke = true;
                field_posture_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-dialogue-dump" &&
                index + 2 < argc) {
                field_smoke = true;
                field_dialogue_dump_prompt = std::string(argv[++index]);
                field_dialogue_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-mode-text-dump" &&
                index + 2 < argc) {
                field_smoke = true;
                field_mode_text_dump_variant = std::string(argv[++index]);
                field_mode_text_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--field-interaction-dump" &&
                index + 2 < argc) {
                field_smoke = true;
                field_interaction_dump_variant = std::string(argv[++index]);
                field_interaction_dump_path =
                    std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--boot-dump-frame" &&
                index + 2 < argc) {
                boot_dump_frame = static_cast<std::uint32_t>(
                    std::stoul(argv[++index]));
                boot_dump_path = std::filesystem::path(argv[++index]);
            }
            if (std::string(argv[index]) == "--boot-fasa-qa" &&
                index + 1 < argc) {
                boot_fasa_qa_path = std::filesystem::path(argv[++index]);
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
        if (field_cyberware_empty_qa &&
            !field_cyberware_dump_path.has_value()) {
            std::cerr << "field Cyberware empty QA requires "
                         "--field-cyberware-dump <file>\n";
            return 2;
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
            std::vector<std::uint16_t> field_audio_events;
            qt_runtime::FieldSceneRuntime field_scene(
                [](const auto&) {},
                [&field_audio_events](const std::uint16_t event) {
                    field_audio_events.push_back(event);
                });
            if (!field_scene.bootstrap(smoke_state)) {
                std::cerr << "field smoke: " << field_scene.failure() << '\n';
                return 3;
            }
            if (field_world_activity_qa_path.has_value()) {
                return run_field_world_activity_qa(
                    field_scene, asset_root,
                    *field_world_activity_qa_path, field_audio_events);
            }
            if (field_transition_qa_path.has_value()) {
                std::filesystem::create_directories(
                    *field_transition_qa_path);
                const auto render_transition_frame = [&] {
                    QImage image(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    image.fill(Qt::black);
                    QPainter painter(&image);
                    field_scene.paint(painter);
                    painter.end();
                    return image;
                };
                const auto save_transition_frame =
                    [&](const std::string_view name, QImage& image) {
                        if (image.width() != kSceneWidth ||
                            image.height() != kSceneHeight) {
                            return false;
                        }
                        return image.save(QString::fromStdWString(
                            ((*field_transition_qa_path) /
                             (std::string(name) + ".bmp")).wstring()));
                    };
                auto initial = render_transition_frame();
                if (count_playfield_lit(initial) < 100 ||
                    !field_hud_chrome_survives(initial, asset_root) ||
                    !save_transition_frame("initial", initial)) {
                    std::cerr << "field transition QA: initial frame failed\n";
                    return 3;
                }

                field_scene.begin_projected_field_transition_for_dump();
                QImage fade_one;
                QImage fade_eight;
                QImage fade_sixteen;
                QImage fade_seventeen;
                for (std::uint8_t tick = 1U; tick <= 17U; ++tick) {
                    field_scene.advance_frame();
                    if (tick == 1U) {
                        fade_one = render_transition_frame();
                        if (!save_transition_frame("fade-01", fade_one)) {
                            std::cerr << "field transition QA: frame 1 dump failed\n";
                            return 3;
                        }
                    } else if (tick == 8U) {
                        fade_eight = render_transition_frame();
                        if (!save_transition_frame("fade-08", fade_eight)) {
                            std::cerr << "field transition QA: frame 8 dump failed\n";
                            return 3;
                        }
                        if (field_scene.field_interaction_transition_timer_for_dump() !=
                                8U ||
                            field_scene.field_palette_fade_steps_for_dump() !=
                                4U) {
                            std::cerr << "field transition QA: midpoint timer or cadence is wrong\n";
                            return 3;
                        }
                    } else if (tick == 16U) {
                        fade_sixteen = render_transition_frame();
                        if (!save_transition_frame("fade-16", fade_sixteen)) {
                            std::cerr << "field transition QA: frame 16 dump failed\n";
                            return 3;
                        }
                        if (field_scene.field_interaction_transition_timer_for_dump() !=
                                0U ||
                            !field_scene.field_interaction_transition_active_for_dump() ||
                            field_scene.field_transition_pending_for_dump()) {
                            std::cerr << "field transition QA: completion published before signed expiry\n";
                            return 3;
                        }
                    } else if (tick == 17U) {
                        fade_seventeen = render_transition_frame();
                        if (!save_transition_frame("fade-17", fade_seventeen)) {
                            std::cerr << "field transition QA: frame 17 dump failed\n";
                            return 3;
                        }
                        if (!save_transition_frame("actual", fade_seventeen)) {
                            std::cerr << "field transition QA: actual dump failed\n";
                            return 3;
                        }
                    }
                }
                if (fade_one == initial || fade_eight == fade_one ||
                    count_playfield_lit(fade_sixteen) != 0U ||
                    count_playfield_lit(fade_seventeen) != 0U ||
                    field_scene.field_interaction_transition_active_for_dump() ||
                    !field_scene.field_transition_pending_for_dump() ||
                    field_scene.field_interaction_transition_timer_for_dump() !=
                        -1 ||
                    field_scene.field_palette_fade_steps_for_dump() != 9U ||
                    !field_audio_events.empty()) {
                    std::cerr << "field transition QA: fade pixels, expiry, or audio behavior did not match the source path\n";
                    return 3;
                }
                std::cout << "field_transition_qa=passed fade_frames=17 palette_steps=9 transition_pending=yes sound_events=0 artifacts="
                          << field_transition_qa_path->string() << '\n';
                return 0;
            }
            if (field_movement_qa_path.has_value()) {
                std::filesystem::create_directories(
                    *field_movement_qa_path);
                const auto render_field = [&] {
                    QImage image(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    image.fill(Qt::black);
                    QPainter painter(&image);
                    field_scene.paint(painter);
                    painter.end();
                    return image;
                };
                const auto save_movement_frame =
                    [&](const std::string_view name) {
                        const auto image = render_field();
                        if (image.width() != kSceneWidth ||
                            image.height() != kSceneHeight ||
                            count_playfield_lit(image) < 100 ||
                            !field_hud_chrome_survives(image, asset_root)) {
                            return false;
                        }
                        return image.save(QString::fromStdWString(
                            ((*field_movement_qa_path) /
                             (std::string(name) + ".bmp")).wstring()));
                    };
                const auto initial_position =
                    field_scene.field_entity_world_position();
                if (!save_movement_frame("idle")) {
                    std::cerr << "field movement QA: initial dump failed\n";
                    return 3;
                }

                field_scene.set_controller_nibble(0x08U);
                std::uint16_t heading_after_eight_ticks = 0U;
                for (std::uint8_t tick = 1U; tick <= 16U; ++tick) {
                    field_scene.advance_frame();
                    if (tick == 8U) {
                        heading_after_eight_ticks =
                            field_scene.field_entity_heading_for_dump();
                    }
                    if (tick == 1U || tick == 4U || tick == 8U ||
                        tick == 16U) {
                        if (!save_movement_frame(
                                "right-held-" + std::to_string(tick))) {
                            std::cerr << "field movement QA: frame dump failed\n";
                            return 3;
                        }
                    }
                }
                const auto held_position =
                    field_scene.field_entity_world_position();
                const auto held_heading =
                    field_scene.field_entity_heading_for_dump();
                const auto held_target =
                    field_scene.field_entity_target_heading_for_dump();
                const auto held_animation_frame =
                    field_scene.field_entity_animation_frame_for_dump();
                const auto held_animation_selector =
                    field_scene.field_entity_animation_selector_for_dump();
                const auto held_walking =
                    field_scene.field_entity_walking_for_dump();
                const auto x_delta = static_cast<std::int32_t>(
                    held_position.x) -
                    static_cast<std::int32_t>(initial_position.x);
                const auto y_delta = static_cast<std::int32_t>(
                    held_position.y) -
                    static_cast<std::int32_t>(initial_position.y);

                field_scene.set_controller_nibble(0U);
                field_scene.advance_frame();
                if (!save_movement_frame("released-idle") ||
                    !save_movement_frame("actual")) {
                    std::cerr << "field movement QA: release dump failed\n";
                    return 3;
                }
                if (held_target != 0x4000U ||
                    heading_after_eight_ticks != 0x4000U ||
                    held_heading != 0x4000U || x_delta != 600 ||
                    y_delta != -200 || held_animation_frame != 1U ||
                    !held_walking ||
                    field_scene.field_entity_walking_for_dump() ||
                    field_scene.field_entity_animation_selector_for_dump() ==
                        held_animation_selector ||
                    field_scene.field_entity_animation_frame_for_dump() !=
                        0U) {
                    std::cerr << "field movement QA: held direction, heading "
                                 "slew, accepted movement, or idle reset did "
                                 "not match the source state\n";
                    return 3;
                }
                const auto& buffers =
                    field_scene.state().field_scene_buffers;
                std::cout << "field_movement_qa=passed scene_reference="
                          << buffers.scene_reference << " initial_position="
                          << initial_position.x << ',' << initial_position.y
                          << " held_position=" << held_position.x << ','
                          << held_position.y << " target_heading="
                          << held_target << " heading=" << held_heading
                          << " heading_at_tick_8="
                          << heading_after_eight_ticks << " delta=" << x_delta
                          << ',' << y_delta
                          << " walk_frame=" << held_animation_frame
                          << " walk_selector=" << held_animation_selector
                          << " artifacts="
                          << field_movement_qa_path->string() << '\n';
                return 0;
            }
            if (field_player_action_dump_path.has_value()) {
                std::filesystem::create_directories(
                    *field_player_action_dump_path);
                auto& runner = field_scene.active_runner_record();
                runner.flags = static_cast<std::uint16_t>(
                    runner.flags | 0x0100U);
                runner.inventory_item_codes[0] = 1U;
                runner.equipped_inventory_slot = 0;
                runner.equipped_item_code = 1U;
                runner.ammo_rounds = 0U;
                runner.ammo_clips = 3U;
                runner.animation_action_gate = 1;
                field_scene.refresh_field_hud_for_dump();
                field_audio_events.clear();

                const auto render_scene = [&] {
                    QImage image(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    image.fill(Qt::black);
                    QPainter painter(&image);
                    field_scene.paint(painter);
                    painter.end();
                    return image;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& image) {
                    return image.save(QString::fromStdWString(
                        ((*field_player_action_dump_path) /
                         (std::string(name) + ".bmp")).wstring()));
                };
                const auto ammo_counter_matches = [&](const QImage& image,
                                                     const int rounds) {
                    const auto counter = QImage(QString::fromStdWString(
                        (asset_root / "field" / "hud" /
                         ("ammo-counter-" + std::to_string(rounds) +
                          ".bmp")).wstring()));
                    const int x = field_hud_vram_x(kFieldHudAmmoVram);
                    const int y = field_hud_vram_y(kFieldHudAmmoVram);
                    if (counter.isNull() || x < 0 || y < 0 ||
                        x + counter.width() > image.width() ||
                        y + counter.height() > image.height()) {
                        return false;
                    }
                    for (int row = 0; row < counter.height(); ++row) {
                        for (int column = 0; column < counter.width(); ++column) {
                            if (image.pixel(x + column, y + row) !=
                                counter.pixel(column, row)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto weapon_glyph_matches = [&](const QImage& image) {
                    const auto item = QImage(QString::fromStdWString(
                        (asset_root / "field" / "hud" /
                         ("item-" + std::to_string(runner.archetype_index) +
                          ".bmp")).wstring()));
                    const int x = field_hud_vram_x(kFieldHudItemVram);
                    const int y = field_hud_vram_y(kFieldHudItemVram);
                    if (item.isNull() || x < 0 || y < 0 ||
                        x + item.width() > image.width() ||
                        y + item.height() > image.height()) {
                        return false;
                    }
                    for (int row = 0; row < item.height(); ++row) {
                        for (int column = 0; column < item.width(); ++column) {
                            if (image.pixel(x + column, y + row) !=
                                item.pixel(column, row)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto before = render_scene();
                if (!save_scene("before", before) ||
                    !ammo_counter_matches(before, 0) ||
                    !weapon_glyph_matches(before)) {
                    std::cerr << "field player action dump: empty magazine was not visible before the action gate opened\n";
                    return 3;
                }

                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                const auto unselected_action = render_scene();
                if (field_scene.active_runner_record().ammo_rounds != 0U ||
                    field_scene.active_runner_record().ammo_clips != 3U ||
                    field_scene.active_runner_record().animation_action_gate !=
                        0 ||
                    !field_audio_events.empty() ||
                    !ammo_counter_matches(unselected_action, 0) ||
                    !weapon_glyph_matches(unselected_action) ||
                    !save_scene("unselected-action-frame",
                                unselected_action)) {
                    std::cerr << "field player action dump: unselected runner entered the weapon-action branch\n";
                    return 3;
                }

                runner.flags = static_cast<std::uint16_t>(
                    runner.flags | 0x0200U);
                field_scene.advance_frame();
                const auto ready_idle = render_scene();
                if (field_scene.active_runner_record().ammo_rounds != 0U ||
                    field_scene.active_runner_record().ammo_clips != 3U ||
                    field_scene.active_runner_record().animation_action_gate >=
                        0 ||
                    !field_audio_events.empty() ||
                    !ammo_counter_matches(ready_idle, 0) ||
                    !weapon_glyph_matches(ready_idle) ||
                    !save_scene("ready-idle-frame", ready_idle)) {
                    std::cerr << "field player action dump: idle selected runner reloaded without held A\n";
                    return 3;
                }

                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                const auto scene = render_scene();
                const auto output =
                    *field_player_action_dump_path / "actual.bmp";
                const bool audio_ordered =
                    field_audio_events ==
                    std::vector<std::uint16_t>{0x4BU, 0x4BU};
                const auto& item_catalog = us_item_catalog();
                const bool weapon_source_valid =
                    !item_catalog.empty() &&
                    item_catalog[0].item_code == 1U &&
                    item_catalog[0].source_combat_factor == 6U;
                if (field_scene.active_runner_record().ammo_rounds != 6U ||
                    field_scene.active_runner_record().ammo_clips != 2U ||
                    field_scene.active_runner_record().animation_action_gate <=
                        0 ||
                    !audio_ordered || !weapon_source_valid ||
                    !ammo_counter_matches(scene, 6) ||
                    !weapon_glyph_matches(scene) ||
                    field_scene.command_overview_open() ||
                    field_scene.dialogue_open() || field_scene.hotel_menu_open() ||
                    field_scene.inventory_action_open() ||
                    field_scene.clips_menu_open() ||
                    field_scene.field_interaction_open() ||
                    !save_scene("actual", scene)) {
                    std::cerr << "field player action dump: source-selected, held-A reload, shared clip, delay, audio, or live HUD counter did not match\n";
                    return 3;
                }
                std::cout << "field_player_action_dump=written path="
                          << output.string() << " clips=2 rounds=6 audio=2\n";
                return 0;
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
                const bool arrival_result =
                    *field_dialogue_dump_prompt == "arrival-manager";
                const bool room_paid_result =
                    *field_dialogue_dump_prompt == "room-paid";
                const bool room_insufficient_result =
                    *field_dialogue_dump_prompt == "room-insufficient";
                const bool debt_followup_result =
                    *field_dialogue_dump_prompt == "debt-followup";
                const bool referral_followup_result =
                    *field_dialogue_dump_prompt == "referral-followup";
                const bool debt_payment_insufficient_result =
                    *field_dialogue_dump_prompt ==
                    "debt-payment-insufficient";
                const bool followup_result = debt_followup_result ||
                                             referral_followup_result ||
                                             debt_payment_insufficient_result;
                const bool room_payment_result = room_paid_result ||
                                                 room_insufficient_result;
                const auto prompt = arrival_result || room_payment_result
                                            || followup_result
                                        ? std::optional<StokerMotelPromptId>{}
                                        : parse_stoker_dialogue_prompt(
                                              *field_dialogue_dump_prompt);
                if (!arrival_result && !room_payment_result &&
                    !followup_result && !prompt.has_value()) {
                    std::cerr << "field dialogue dump: unknown Stoker prompt '"
                              << *field_dialogue_dump_prompt << "'\n";
                    return 3;
                }
                bool prompt_opened = false;
                if (followup_result) {
                    prompt_opened = field_scene.open_stoker_dialogue_for_dump(
                        StokerMotelPromptId::manager_greeting_with_package);
                    if (prompt_opened) {
                        prompt_opened =
                            field_scene.advance_stoker_dialogue_for_dump(
                                0x2000U) ==
                            FieldDialogueAdvanceResult::prompt_advanced;
                    }
                    if (prompt_opened && referral_followup_result) {
                        prompt_opened =
                            field_scene.advance_stoker_dialogue_for_dump(
                                0x1000U) ==
                            FieldDialogueAdvanceResult::prompt_advanced;
                    }
                    if (prompt_opened && debt_payment_insufficient_result) {
                        prompt_opened =
                            field_scene.advance_stoker_dialogue_for_dump(
                                0x4000U) ==
                            FieldDialogueAdvanceResult::prompt_advanced;
                    }
                    const auto story_flags =
                        field_scene.notebook_story_flags_for_dump();
                    const auto referral_flags =
                        field_scene.notebook_referral_flags_for_dump();
                    const auto notebook_gates =
                        field_scene.notebook_event_gates();
                    const bool referral_flag_expected =
                        referral_followup_result ||
                        debt_payment_insufficient_result;
                    const auto expected_gates =
                        referral_flag_expected
                            ? std::array<bool, 3>{true, true, false}
                            : std::array<bool, 3>{true, false, false};
                    if (!prompt_opened || !story_flags.has_value() ||
                        *story_flags != 3U || !referral_flags.has_value() ||
                        *referral_flags !=
                            (referral_flag_expected ? 1U : 0U) ||
                        !notebook_gates.has_value() ||
                        *notebook_gates != expected_gates) {
                        std::cerr << "field dialogue dump: typed Stoker flags or Notebook gates mismatch\n";
                        return 3;
                    }
                    if (debt_payment_insufficient_result &&
                        field_scene.field_funds_for_dump() != 20U) {
                        std::cerr << "field dialogue dump: low-funds debt payment changed the wallet\n";
                        return 3;
                    }
                } else if (arrival_result) {
                    prompt_opened =
                        field_scene.open_stoker_arrival_result_for_dump(79U);
                } else if (room_payment_result) {
                    prompt_opened = field_scene.open_stoker_room_payment_for_dump(
                        room_paid_result ? 20U : 19U);
                } else {
                    prompt_opened =
                        field_scene.open_stoker_dialogue_for_dump(*prompt);
                }
                if (!prompt_opened) {
                    std::cerr << "field dialogue dump: prompt did not open\n";
                    return 3;
                }
                if ((room_paid_result &&
                     field_scene.field_funds_for_dump() != 0U) ||
                    (room_insufficient_result &&
                     field_scene.field_funds_for_dump() != 19U)) {
                    std::cerr
                        << "field dialogue dump: room-payment wallet result mismatch\n";
                    return 3;
                }
                QImage dialogue_scene(kSceneWidth, kSceneHeight,
                                      QImage::Format_RGB32);
                dialogue_scene.fill(Qt::black);
                QPainter dialogue_painter(&dialogue_scene);
                field_scene.paint(dialogue_painter);
                dialogue_painter.end();
                if (field_dialogue_dump_path->has_parent_path()) {
                    std::filesystem::create_directories(
                        field_dialogue_dump_path->parent_path());
                }
                if (!dialogue_scene.save(QString::fromStdWString(
                        field_dialogue_dump_path->wstring()))) {
                    std::cerr << "field dialogue dump: could not write "
                              << field_dialogue_dump_path->string() << '\n';
                    return 3;
                }

                if (referral_followup_result ||
                    debt_payment_insufficient_result) {
                    if (field_scene.advance_stoker_dialogue_for_dump(0x4000U) !=
                            FieldDialogueAdvanceResult::returned_to_field ||
                        !field_scene.open_command_overview_for_dump() ||
                        !field_scene.seek_command_overview_page(6U)) {
                        std::cerr << "field dialogue dump: could not return to the field Notebook route\n";
                        return 3;
                    }
                    field_scene.set_controller_port_byte(0x40U);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                    if (!field_scene.pocket_secretary_open()) {
                        std::cerr << "field dialogue dump: Pocket Secretary did not open\n";
                        return 3;
                    }
                    field_scene.set_controller_port_byte(0x40U);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                    if (!field_scene.pocket_secretary_notebook_open() ||
                        field_scene.pocket_secretary_notebook_option_count() !=
                            4U) {
                        std::cerr << "field dialogue dump: referral did not unlock the Mr. Johnsons Notebook row\n";
                        return 3;
                    }
                    QImage notebook_scene(kSceneWidth, kSceneHeight,
                                          QImage::Format_RGB32);
                    notebook_scene.fill(Qt::black);
                    QPainter notebook_painter(&notebook_scene);
                    field_scene.paint(notebook_painter);
                    notebook_painter.end();
                    const std::array<bool, 6> notebook_pixels{
                        image_has_green_ink(notebook_scene, 88, 88, 160, 8),
                        image_has_green_ink(notebook_scene, 88, 104, 160, 8),
                        !image_has_green_ink(notebook_scene, 88, 120, 160, 8),
                        image_has_green_ink(notebook_scene, 72, 56, 8, 8),
                        !image_has_green_ink(notebook_scene, 88, 16, 160, 8),
                        // Keep the stale-row check above the E660 effect
                        // cells at y=200/208; their animated pixels are
                        // verified by the dedicated Tips frame sequence.
                        !image_has_green_ink(notebook_scene, 88, 152, 216, 48)};
                    if (std::any_of(notebook_pixels.begin(),
                                    notebook_pixels.end(),
                                    [](const bool pixel_check) {
                                        return !pixel_check;
                                    })) {
                        auto diagnostic_path = *field_dialogue_dump_path;
                        diagnostic_path.replace_filename(
                            "notebook-pixel-check-failure.bmp");
                        (void)notebook_scene.save(QString::fromStdWString(
                            diagnostic_path.wstring()));
                        std::cerr << "field dialogue dump: dynamic Notebook "
                                     "rows or stale-row clearing mismatch "
                                     "checks=";
                        for (const auto check : notebook_pixels) {
                            std::cerr << (check ? '1' : '0');
                        }
                        std::cerr << "\n";
                        return 3;
                    }
                    const auto notebook_result_name =
                        debt_payment_insufficient_result
                            ? "notebook-after-debt-payment"
                            : "notebook-after-referral";
                    const auto notebook_dump_path =
                        field_dialogue_dump_path->parent_path() /
                        notebook_result_name / "actual.bmp";
                    std::filesystem::create_directories(
                        notebook_dump_path.parent_path());
                    if (!notebook_scene.save(QString::fromStdWString(
                            notebook_dump_path.wstring()))) {
                        std::cerr << "field dialogue dump: could not write Notebook image\n";
                        return 3;
                    }

                    field_scene.set_controller_port_byte(0x02U);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0x40U);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                    if (!field_scene.pocket_secretary_notebook_tips_open() ||
                        field_scene
                                .pocket_secretary_notebook_visible_tip_count() !=
                            2U ||
                        field_scene
                                .pocket_secretary_notebook_visible_tip_source_index(
                                    0U) != 0U ||
                        field_scene
                                .pocket_secretary_notebook_visible_tip_source_index(
                                    1U) != 1U) {
                        std::cerr << "field dialogue dump: debt flag did not unlock its Notebook clue\n";
                        return 3;
                    }
                    QImage notebook_tips_scene(kSceneWidth, kSceneHeight,
                                               QImage::Format_RGB32);
                    notebook_tips_scene.fill(Qt::black);
                    QPainter notebook_tips_painter(&notebook_tips_scene);
                    field_scene.paint(notebook_tips_painter);
                    notebook_tips_painter.end();
                    if (!image_has_green_ink(notebook_tips_scene, 40, 56,
                                             192, 8) ||
                        image_has_green_ink(notebook_tips_scene, 88, 120,
                                            160, 8)) {
                        std::cerr << "field dialogue dump: unlocked Tips page has missing text or stale Notebook rows\n";
                        return 3;
                    }
                    const auto notebook_tips_result_name =
                        debt_payment_insufficient_result
                            ? "notebook-tips-after-debt-payment"
                            : "notebook-tips-after-debt";
                    const auto notebook_tips_dump_path =
                        field_dialogue_dump_path->parent_path() /
                        notebook_tips_result_name / "actual.bmp";
                    std::filesystem::create_directories(
                        notebook_tips_dump_path.parent_path());
                    if (!notebook_tips_scene.save(QString::fromStdWString(
                            notebook_tips_dump_path.wstring()))) {
                        std::cerr << "field dialogue dump: could not write Tips image\n";
                        return 3;
                    }
                }
                std::cout << "field_dialogue_dump=written prompt="
                          << *field_dialogue_dump_prompt << " path="
                          << field_dialogue_dump_path->string() << "\n";
                return 0;
            }
            if (field_hotel_qa_path.has_value()) {
                const auto output = *field_hotel_qa_path;
                std::filesystem::create_directories(output);
                field_scene.active_runner_record().delay_meter = 2U;
                field_scene.active_runner_record().behavior_meter = 0U;
                field_scene.active_runner_record().attributes[0U] = 1U;
                field_scene.active_runner_record().auxiliary[2U] = 50U;
                if (!field_scene.open_stoker_room_payment_for_dump(20U) ||
                    field_scene.field_funds_for_dump() != 0U) {
                    std::cerr << "field Hotel QA: paid-room outcome did not open\n";
                    return 3;
                }
                const auto capture = [&field_scene](
                                         const std::filesystem::path& path) {
                    QImage image(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    image.fill(Qt::magenta);
                    QPainter painter(&image);
                    field_scene.paint(painter);
                    painter.end();
                    if (path.has_parent_path()) {
                        std::filesystem::create_directories(
                            path.parent_path());
                    }
                    if (!image.save(QString::fromStdWString(
                            path.wstring()))) {
                        return QImage{};
                    }
                    return image;
                };
                if (capture(output / "paid-dialogue" / "actual.bmp").isNull() ||
                    field_scene.advance_stoker_dialogue_for_dump(0x4000U) !=
                        FieldDialogueAdvanceResult::hotel_options_requested ||
                    !field_scene.hotel_menu_open() ||
                    field_scene.dialogue_open() ||
                    field_scene.hotel_menu_phase() !=
                        HotelMenuPhase::entity_status ||
                    field_scene.active_runner_record().behavior_meter !=
                        10U) {
                    std::cerr << "field Hotel QA: paid dialogue did not transfer into Hotel status\n";
                    return 3;
                }
                auto status = capture(output / "status" / "actual.bmp");
                if (status.isNull() ||
                    !image_has_green_ink(status, 88, 40, 48, 8) ||
                    !image_has_green_ink(status, 32, 48, 240, 8) ||
                    !image_has_green_ink(status, 32, 80, 224, 8) ||
                    !image_region_has_color(status, 88, 40, 48, 8,
                                            QColor(36, 219, 36)) ||
                    image_has_green_ink(status, 32, 40, 48, 8) ||
                    image_has_green_ink(status, 32, 56, 240, 24)) {
                    std::cerr << "field Hotel QA: status page text or full redraw is incorrect\n";
                    return 3;
                }
                QImage black_reference(kSceneWidth, kSceneHeight,
                                       QImage::Format_RGB32);
                black_reference.fill(Qt::black);
                if (image_region_differs(status, black_reference,
                                         272, 0, 48, 224)) {
                    std::cerr << "field Hotel QA: stale field HUD remains over Hotel screen\n";
                    return 3;
                }
                for (int frame = 0; frame < 30; ++frame) {
                    if (field_scene.advance_hotel_menu_for_dump(0U) !=
                        HotelMenuEvent::waiting) {
                        std::cerr << "field Hotel QA: status page left without input\n";
                        return 3;
                    }
                }
                const auto status_frame_30 =
                    capture(output / "status-frame-30" / "actual.bmp");
                if (status_frame_30.isNull() ||
                    field_scene.hotel_animation_frames() != 30U ||
                    image_region_differs(status, status_frame_30,
                                         24, 24, 224, 168) ||
                    !image_region_differs(status, status_frame_30,
                                          24, 192, 224, 32)) {
                    std::cerr << "field Hotel QA: status text or E660 wait animation is incorrect\n";
                    return 3;
                }

                if (field_scene.advance_hotel_menu_for_dump(0x4000U) !=
                        HotelMenuEvent::status_acknowledged ||
                    field_scene.hotel_menu_phase() != HotelMenuPhase::options ||
                    field_scene.hotel_option_selection() != 0U) {
                    std::cerr << "field Hotel QA: status acknowledgement did not open the options page\n";
                    return 3;
                }
                const auto options_default =
                    capture(output / "options" / "actual.bmp");
                if (options_default.isNull() ||
                    !image_has_green_ink(options_default, 88, 72, 160, 8) ||
                    !image_has_green_ink(options_default, 88, 80, 160, 8) ||
                    !image_region_has_color(options_default, 88, 72, 160, 8,
                                            QColor(36, 219, 36)) ||
                    image_has_green_ink(options_default, 32, 40, 240, 8)) {
                    std::cerr << "field Hotel QA: options page is missing rows or retained the status page\n";
                    return 3;
                }

                const auto hotel_record_before_upgrade =
                    field_scene.active_runner_record();
                field_audio_events.clear();
                if (field_scene.advance_hotel_menu_for_dump(0x4000U) !=
                        HotelMenuEvent::karma_screen_opened ||
                    !field_scene.hotel_karma_screen_open() ||
                    !field_scene.hotel_menu_open() ||
                    field_scene.hotel_menu_phase() !=
                        HotelMenuPhase::options ||
                    !field_scene.hotel_karma_screen()
                         .allows_karma_upgrades() ||
                    !field_scene.hotel_karma_screen()
                         .state().selection_data_available) {
                    std::cerr << "field Hotel QA: Karma Screen did not enter the separate upgrade route\n";
                    return 3;
                }
                auto karma_spend = capture(output / "karma-spend" /
                                           "actual.bmp");
                if (karma_spend.isNull() ||
                    !image_region_has_color(karma_spend, 64, 200, 224, 8,
                                            QColor(109, 73, 146)) ||
                    image_has_green_ink(karma_spend, 88, 72, 40, 16) ||
                    field_scene.active_runner_record().attributes !=
                        hotel_record_before_upgrade.attributes ||
                    field_scene.active_runner_record().auxiliary !=
                        hotel_record_before_upgrade.auxiliary) {
                    std::cerr << "field Hotel QA: spending page leaked Hotel controls, omitted its upgrade detail, or mutated on entry\n";
                    return 3;
                }
                const auto parent_animation_at_entry =
                    field_scene.hotel_animation_frames();
                for (int frame = 0; frame < 30; ++frame) {
                    if (field_scene.advance_hotel_karma_screen_for_dump(0U) !=
                        AttributesSkillsRuntimeEvent::no_change) {
                        std::cerr << "field Hotel QA: Karma page left during idle frames\n";
                        return 3;
                    }
                }
                if (capture(output / "karma-spend" / "frame-30.bmp") !=
                        karma_spend ||
                    field_scene.hotel_animation_frames() !=
                        parent_animation_at_entry) {
                    std::cerr << "field Hotel QA: Karma screen or suspended Hotel animation changed without input\n";
                    return 3;
                }
                if (field_scene.advance_hotel_karma_screen_for_dump(0x4000U) !=
                        AttributesSkillsRuntimeEvent::upgrade_prompt_opened ||
                    !field_scene.hotel_karma_screen().upgrade_prompt_open() ||
                    field_scene.active_runner_record().attributes !=
                        hotel_record_before_upgrade.attributes ||
                    field_scene.active_runner_record().auxiliary !=
                        hotel_record_before_upgrade.auxiliary) {
                    std::cerr << "field Hotel QA: A did not open the non-mutating default-Yes prompt\n";
                    return 3;
                }
                const auto karma_prompt_yes = capture(
                    output / "karma-spend" / "prompt-yes.bmp");
                if (!image_region_has_color(karma_prompt_yes, 96, 168,
                                            120, 32,
                                            QColor(109, 73, 146))) {
                    std::cerr << "field Hotel QA: prompt text or choices are missing\n";
                    return 3;
                }
                if (field_scene.advance_hotel_karma_screen_for_dump(0x0200U) !=
                        AttributesSkillsRuntimeEvent::upgrade_prompt_moved ||
                    field_scene.hotel_karma_screen()
                            .upgrade_prompt_selection() != 1U ||
                    field_scene.advance_hotel_karma_screen_for_dump(0x1000U) !=
                        AttributesSkillsRuntimeEvent::upgrade_declined ||
                    field_scene.active_runner_record().attributes !=
                        hotel_record_before_upgrade.attributes ||
                    field_scene.active_runner_record().auxiliary !=
                        hotel_record_before_upgrade.auxiliary) {
                    std::cerr << "field Hotel QA: No mutated the runner or failed to move the choice cursor\n";
                    return 3;
                }
                if (field_scene.advance_hotel_karma_screen_for_dump(0x4000U) !=
                        AttributesSkillsRuntimeEvent::upgrade_prompt_opened ||
                    field_scene.advance_hotel_karma_screen_for_dump(0x4000U) !=
                        AttributesSkillsRuntimeEvent::upgrade_applied ||
                    field_scene.active_runner_record().attributes[0U] != 2U ||
                    field_scene.active_runner_record().auxiliary[2U] != 48U ||
                    capture(output / "karma-spend" / "upgrade-applied.bmp")
                        .isNull()) {
                    std::cerr << "field Hotel QA: Yes did not spend Karma and increase Body\n";
                    return 3;
                }
                if (field_scene.advance_hotel_karma_screen_for_dump(0x8000U) !=
                        AttributesSkillsRuntimeEvent::returned_to_parent ||
                    field_scene.hotel_karma_screen_open() ||
                    !field_scene.hotel_menu_open() ||
                    field_scene.hotel_option_selection() != 0U) {
                    std::cerr << "field Hotel QA: Start did not return to Hotel Options\n";
                    return 3;
                }
                const auto karma_return = capture(
                    output / "karma-spend" / "return-options.bmp");
                if (!image_has_green_ink(karma_return, 88, 72, 160, 8) ||
                    image_has_green_ink(karma_return, 24, 56, 48, 8) ||
                    !field_audio_events.empty()) {
                    std::cerr << "field Hotel QA: Hotel Options redraw retained Attributes/Skills pixels\n";
                    return 3;
                }
                const auto options_after_karma = capture(
                    output / "options-after-karma" / "actual.bmp");
                const auto option_frames_after_karma =
                    field_scene.hotel_animation_frames();
                for (int frame = 0; frame < 30; ++frame) {
                    if (field_scene.advance_hotel_menu_for_dump(0U) !=
                        HotelMenuEvent::waiting) {
                        std::cerr << "field Hotel QA: options page left without input\n";
                        return 3;
                    }
                }
                const auto options_frame_30 = capture(
                    output / "options-frame-30" / "actual.bmp");
                if (options_frame_30.isNull() ||
                    field_scene.hotel_animation_frames() !=
                        option_frames_after_karma + 30U ||
                    image_region_differs(options_after_karma,
                                         options_frame_30,
                                         24, 24, 224, 168) ||
                    !image_region_differs(options_after_karma,
                                          options_frame_30,
                                          24, 192, 224, 32)) {
                    std::cerr << "field Hotel QA: E660 should animate the lower grid once per options wait frame\n";
                    return 3;
                }
                if (field_scene.advance_hotel_menu_for_dump(0x0200U) !=
                        HotelMenuEvent::selection_moved ||
                    field_scene.hotel_option_selection() != 1U) {
                    std::cerr << "field Hotel QA: Down did not select Check Out\n";
                    return 3;
                }
                const auto options_checkout =
                    capture(output / "options-checkout-selected" / "actual.bmp");
                if (options_checkout.isNull() ||
                    !image_region_differs(options_default, options_checkout,
                                          72, 72, 8, 16) ||
                    !image_region_has_color(options_checkout, 72, 80, 8, 8,
                                            QColor(36, 219, 36))) {
                    std::cerr << "field Hotel QA: source's 8x8 selection arrow did not move between options\n";
                    return 3;
                }
                if (field_scene.advance_hotel_menu_for_dump(0x4000U) !=
                        HotelMenuEvent::checkout_opened ||
                    field_scene.hotel_menu_phase() != HotelMenuPhase::checkout ||
                    field_scene.hotel_checkout_selection() != 1U) {
                    std::cerr << "field Hotel QA: checkout did not open with No selected\n";
                    return 3;
                }
                const auto checkout_no =
                    capture(output / "checkout-no" / "actual.bmp");
                if (checkout_no.isNull() ||
                    !image_region_has_color(checkout_no, 80, 72, 160, 16,
                                            QColor(109, 73, 146)) ||
                    !image_region_has_color(checkout_no, 128, 96, 64, 8,
                                            QColor(109, 73, 146)) ||
                    !image_region_has_color(checkout_no, 128, 112, 64, 8,
                                            QColor(109, 73, 146)) ||
                    image_has_green_ink(checkout_no, 88, 72, 160, 16) ||
                    !image_region_has_color(checkout_no, 104, 104, 8, 8,
                                            QColor(109, 73, 146)) ||
                    image_has_green_ink(checkout_no, 72, 48, 160, 8)) {
                    std::cerr << "field Hotel QA: checkout text or old options clear is incorrect\n";
                    return 3;
                }
                const auto checkout_animation_frame_count =
                    field_scene.hotel_animation_frames();
                for (int frame = 0; frame < 30; ++frame) {
                    if (field_scene.advance_hotel_menu_for_dump(0U) !=
                        HotelMenuEvent::waiting) {
                        std::cerr << "field Hotel QA: checkout prompt left without input\n";
                        return 3;
                    }
                }
                const auto checkout_no_frame_30 = capture(
                    output / "checkout-no-frame-30" / "actual.bmp");
                if (checkout_no_frame_30.isNull() ||
                    field_scene.hotel_animation_frames() !=
                        checkout_animation_frame_count ||
                    image_region_differs(checkout_no, checkout_no_frame_30,
                                         0, 0, 320, 224)) {
                    std::cerr << "field Hotel QA: nested checkout prompt should hold without E660\n";
                    return 3;
                }
                if (field_scene.advance_hotel_menu_for_dump(0x4000U) !=
                        HotelMenuEvent::stayed_at_hotel ||
                    field_scene.hotel_menu_phase() != HotelMenuPhase::options ||
                    field_scene.hotel_option_selection() != 0U) {
                    std::cerr << "field Hotel QA: default No did not return to the first Hotel option\n";
                    return 3;
                }
                const auto options_after_no =
                    capture(output / "options-after-no" / "actual.bmp");
                if (options_after_no.isNull() ||
                    image_has_green_ink(options_after_no, 80, 96, 160, 32) ||
                    image_region_has_color(options_after_no, 80, 72, 160, 48,
                                           QColor(109, 73, 146)) ||
                    image_region_has_color(options_after_no, 104, 96, 8, 24,
                                           QColor(109, 73, 146))) {
                    std::cerr << "field Hotel QA: No left checkout text under the Hotel options\n";
                    return 3;
                }
                if (field_scene.advance_hotel_menu_for_dump(
                        0x0200U | 0x4000U) != HotelMenuEvent::checkout_opened ||
                    field_scene.hotel_checkout_selection() != 1U ||
                    field_scene.advance_hotel_menu_for_dump(0x0100U) !=
                        HotelMenuEvent::selection_moved ||
                    field_scene.hotel_checkout_selection() != 0U) {
                    std::cerr << "field Hotel QA: same-frame move/confirm order or checkout wrap is incorrect\n";
                    return 3;
                }
                const auto checkout_yes =
                    capture(output / "checkout-yes" / "actual.bmp");
                if (checkout_yes.isNull() ||
                    !image_region_has_color(checkout_yes, 104, 96, 8, 8,
                                            QColor(109, 73, 146)) ||
                    !image_region_differs(checkout_no, checkout_yes,
                                          104, 96, 8, 16)) {
                    std::cerr << "field Hotel QA: checkout cursor did not move from No to Yes\n";
                    return 3;
                }
                if (field_scene.advance_hotel_menu_for_dump(0x1000U) !=
                        HotelMenuEvent::checked_out ||
                    field_scene.hotel_menu_open() ||
                    field_scene.dialogue_open() || !field_audio_events.empty()) {
                    std::cerr << "field Hotel QA: Yes did not return to the field or Hotel emitted unexpected audio\n";
                    return 3;
                }
                const auto returned_field =
                    capture(output / "returned-field" / "actual.bmp");
                if (returned_field.isNull() ||
                    !image_region_differs(returned_field, black_reference,
                                          0, 0, 320, 224)) {
                    std::cerr << "field Hotel QA: checkout did not redraw the field scene\n";
                    return 3;
                }
                std::cout << "field_hotel_qa=passed path="
                          << output.string() << "\n";
                return 0;
            }
            if (field_mode_text_dump_variant.has_value() &&
                field_mode_text_dump_path.has_value()) {
                std::int8_t alternate_result = 0;
                if (*field_mode_text_dump_variant == "introduction") {
                    alternate_result = 0;
                } else if (*field_mode_text_dump_variant ==
                           "package-handoff") {
                    alternate_result = -1;
                } else if (*field_mode_text_dump_variant ==
                           "package-payment") {
                    alternate_result = 1;
                } else {
                    std::cerr << "field mode-text dump: unknown variant '"
                              << *field_mode_text_dump_variant << "'\n";
                    return 3;
                }
                if (!field_scene.open_ares_mode_text_for_dump(
                        alternate_result)) {
                    std::cerr << "field mode-text dump: prompt did not open\n";
                    return 3;
                }
                QImage mode_text_scene(kSceneWidth, kSceneHeight,
                                       QImage::Format_RGB32);
                mode_text_scene.fill(Qt::black);
                QPainter mode_text_painter(&mode_text_scene);
                field_scene.paint(mode_text_painter);
                mode_text_painter.end();
                if (field_mode_text_dump_path->has_parent_path()) {
                    std::filesystem::create_directories(
                        field_mode_text_dump_path->parent_path());
                }
                if (!mode_text_scene.save(QString::fromStdWString(
                        field_mode_text_dump_path->wstring()))) {
                    std::cerr << "field mode-text dump: could not write "
                              << field_mode_text_dump_path->string() << '\n';
                    return 3;
                }
                std::cout << "field_mode_text_dump=written variant="
                          << *field_mode_text_dump_variant << " path="
                          << field_mode_text_dump_path->string() << "\n";
                return 0;
            }
            if (field_interaction_dump_variant.has_value() &&
                field_interaction_dump_path.has_value()) {
                auto prompt = FieldInteractionPrompt::electromagnetic_lock_door;
                bool suppressed = false;
                const bool show_lock_choices =
                    *field_interaction_dump_variant == "maglock-choices";
                if (*field_interaction_dump_variant == "security-terminal") {
                    prompt = FieldInteractionPrompt::building_security_terminal;
                } else if (*field_interaction_dump_variant ==
                           "maglock-suppressed") {
                    suppressed = true;
                } else if (!show_lock_choices) {
                    std::cerr << "field interaction dump: unknown variant '"
                              << *field_interaction_dump_variant << "'\n";
                    return 3;
                }
                if ((field_scene.field_entity_behavior_flags_for_dump() &
                     0x02U) == 0U) {
                    std::cerr << "field interaction dump: selected runner's source behavior flags do not enable the ROM 0xB7B8 interaction path\n";
                    return 3;
                }
                QImage field_before_prompt_scene(kSceneWidth, kSceneHeight,
                                                 QImage::Format_RGB32);
                field_before_prompt_scene.fill(Qt::black);
                QPainter field_before_prompt_painter(
                    &field_before_prompt_scene);
                field_scene.paint(field_before_prompt_painter);
                field_before_prompt_painter.end();
                if (!field_scene.dispatch_field_interaction_through_coordinator_for_dump(
                        prompt, 2U, suppressed)) {
                    std::cerr << "field interaction dump: pending handler did not open the prompt\n";
                    return 3;
                }
                if (show_lock_choices) {
                    for (std::uint16_t frame = 0U; frame < 10U; ++frame) {
                        (void)field_scene.advance_field_interaction_for_dump(0U);
                    }
                    (void)field_scene.advance_field_interaction_for_dump(
                        0x4000U);
                    if (field_scene.field_interaction_panel().option_count !=
                        3U) {
                        std::cerr << "field interaction dump: lock action choices did not resolve\n";
                        return 3;
                    }
                } else if (*field_interaction_dump_variant ==
                               "security-terminal" &&
                           field_scene.field_interaction_panel().option_count !=
                               2U) {
                    std::cerr << "field interaction dump: Security Terminal action choices did not resolve\n";
                    return 3;
                }
                QImage interaction_scene(kSceneWidth, kSceneHeight,
                                         QImage::Format_RGB32);
                interaction_scene.fill(Qt::black);
                QPainter interaction_painter(&interaction_scene);
                field_scene.paint(interaction_painter);
                interaction_painter.end();
                if (field_interaction_dump_path->has_parent_path()) {
                    std::filesystem::create_directories(
                        field_interaction_dump_path->parent_path());
                }
                if (!interaction_scene.save(QString::fromStdWString(
                        field_interaction_dump_path->wstring()))) {
                    std::cerr << "field interaction dump: could not write "
                              << field_interaction_dump_path->string() << '\n';
                    return 3;
                }
                for (std::uint16_t frame = 0U; frame < 10U; ++frame) {
                    (void)field_scene.advance_field_interaction_for_dump(0U);
                }
                const auto leave_event =
                    *field_interaction_dump_variant == "security-terminal"
                        ? 0x1000U
                        : 0x2000U;
                if (field_scene.advance_field_interaction_for_dump(
                        leave_event) !=
                        FieldInteractionAdvanceResult::returned_to_field ||
                    field_scene.field_interaction_open()) {
                    std::cerr << "field interaction dump: modal did not return to the field\n";
                    return 3;
                }
                QImage returned_field_scene(kSceneWidth, kSceneHeight,
                                            QImage::Format_RGB32);
                returned_field_scene.fill(Qt::black);
                QPainter returned_field_painter(&returned_field_scene);
                field_scene.paint(returned_field_painter);
                returned_field_painter.end();
                if (returned_field_scene != field_before_prompt_scene) {
                    std::cerr << "field interaction dump: field did not fully redraw after return\n";
                    return 3;
                }
                const auto return_path =
                    field_interaction_dump_path->parent_path() /
                    "return-field.bmp";
                if (!returned_field_scene.save(QString::fromStdWString(
                        return_path.wstring()))) {
                    std::cerr << "field interaction dump: could not write "
                              << return_path.string() << '\n';
                    return 3;
                }
                std::cout << "field_interaction_dump=written variant="
                          << *field_interaction_dump_variant << " path="
                          << field_interaction_dump_path->string() << "\n";
                return 0;
            }
            if (field_shop_dump_path.has_value()) {
                const auto output = *field_shop_dump_path;
                std::filesystem::create_directories(output);
                const auto render_shop_scene = [&field_scene] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto save_scene = [&output](
                                           const std::filesystem::path& path,
                                           const QImage& scene) {
                    const auto destination = output / path;
                    if (destination.has_parent_path()) {
                        std::filesystem::create_directories(
                            destination.parent_path());
                    }
                    return scene.save(QString::fromStdWString(
                        destination.wstring()));
                };
                const auto press_field_input = [&field_scene](
                                                   const std::uint8_t input) {
                    field_scene.set_controller_port_byte(input);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                const auto enter_ares_shop = [&field_scene] {
                    if (!field_scene.begin_ares_shop_event_for_dump()) {
                        return false;
                    }
                    field_scene.set_controller_port_byte(0U);
                    for (int frame = 0; frame < 30; ++frame) {
                        field_scene.advance_frame();
                    }
                    field_scene.set_controller_port_byte(0x40U);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                    return field_scene.shop_menu_open();
                };

                if (!field_scene.begin_ares_shop_event_for_dump()) {
                    std::cerr << "field Ares shop QA: source event route did "
                                 "not open the generic mode-text prompt\n";
                    return 3;
                }
                const auto introduction = render_shop_scene();
                if (!save_scene("introduction/actual.bmp", introduction)) {
                    std::cerr << "field Ares shop QA: could not write "
                                 "introduction actual.bmp\n";
                    return 3;
                }

                // ROM 0x1F3C4 waits 30 frames before accepting a fresh
                // controller edge; the prompt's A edge then returns to the
                // 0x1FA78 caller and dispatches the 0x56AC0 shop.
                field_scene.set_controller_port_byte(0U);
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.shop_menu_open()) {
                    std::cerr << "field Ares shop QA: prompt return did not "
                                 "open the shop controller\n";
                    return 3;
                }

                const auto category = render_shop_scene();
                const QImage service_background(QString::fromStdWString(
                    (asset_root / "menu" / "field" /
                     "service-background.bmp").wstring()));
                if (service_background.isNull() ||
                    !image_region_differs(category, service_background,
                                          64, 128, 144, 16) ||
                    !image_region_differs(category, service_background,
                                          16, 168, 240, 24) ||
                    !image_has_green_ink(category, 64, 144, 160, 16) ||
                    !save_scene("shop-category/actual.bmp", category)) {
                    std::cerr << "field Ares shop QA: could not write "
                                 "category actual.bmp or tabs/stock pixels "
                                 "are missing\n";
                    return 3;
                }

                field_scene.set_controller_port_byte(0x02U); // Down: offers.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.shop_menu_offers_active() ||
                    field_scene.shop_menu_cursor() != 0U) {
                    std::cerr << "field Ares shop QA: Down did not select the "
                                 "first available weapon\n";
                    return 3;
                }
                const auto first_item = render_shop_scene();
                if (!field_shop_selector_survives(first_item, 16) ||
                    !save_scene("shop-item/actual.bmp", first_item)) {
                    std::cerr << "field Ares shop QA: item focus is not a "
                                 "one-pixel 24x16 outline or actual.bmp is "
                                 "missing\n";
                    return 3;
                }

                field_scene.set_controller_port_byte(0x08U); // Right.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.shop_menu_cursor() != 1U) {
                    std::cerr << "field Ares shop QA: Right did not move the "
                                 "item cursor\n";
                    return 3;
                }
                const auto moved_item = render_shop_scene();
                if (moved_item == first_item ||
                    !field_shop_selector_survives(moved_item, 48) ||
                    !save_scene("shop-cursor-moved/actual.bmp", moved_item)) {
                    std::cerr << "field Ares shop QA: item pixels did not "
                                 "follow the typed cursor\n";
                    return 3;
                }

                if (field_audio_events !=
                    std::vector<std::uint16_t>{0x25U, 0x25U}) {
                    std::cerr << "field Ares shop QA: movement audio events "
                                 "do not match the source service\n";
                    return 3;
                }

                field_scene.set_controller_port_byte(0x80U); // Start return.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.shop_menu_open()) {
                    std::cerr << "field Ares shop QA: Start did not return "
                                 "to the field\n";
                    return 3;
                }
                const auto returned = render_shop_scene();
                if (returned == moved_item ||
                    !save_scene("field-return/actual.bmp", returned)) {
                    std::cerr << "field Ares shop QA: shop pixels survived "
                                 "the full-screen return redraw\n";
                    return 3;
                }

                // The E-coded offer is Universal Gun Clips, not an ordinary
                // inventory item. Re-enter through the same event/prompt and
                // verify its source-specific art, text, quantity, tab, and
                // selected-row frame.
                if (!field_scene.begin_ares_shop_event_for_dump()) {
                    std::cerr << "field Ares shop QA: second event route did "
                                 "not open the generic prompt\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0U);
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0x08U); // Other items.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.shop_menu_open() ||
                    field_scene.shop_menu_group() !=
                        ShopActionGroup::other_items ||
                    field_scene.shop_menu_offers_active()) {
                    std::cerr << "field Ares shop QA: Right did not select "
                                 "the other-items tab\n";
                    return 3;
                }
                const auto other_items = render_shop_scene();
                if (!save_scene("shop-other-items/actual.bmp",
                                other_items)) {
                    std::cerr << "field Ares shop QA: could not write "
                                 "other-items actual.bmp\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x02U); // Offers.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.shop_menu_offers_active() ||
                    field_scene.shop_menu_group() !=
                        ShopActionGroup::other_items ||
                    field_scene.shop_menu_cursor() != 0U) {
                    std::cerr << "field Ares shop QA: other-items tab did "
                                 "not select Universal Gun Clips\n";
                    return 3;
                }
                const auto universal_clips = render_shop_scene();
                if (!field_shop_selector_survives(universal_clips, 16) ||
                    !image_region_differs(universal_clips,
                                          service_background, 128, 32, 24,
                                          16) ||
                    !image_region_has_color(
                        universal_clips, 128, 32, 24, 16,
                        QColor(146, 146, 146)) ||
                    !image_region_has_color(
                        universal_clips, 128, 32, 24, 16,
                        QColor(255, 0, 0)) ||
                    !image_has_green_ink(universal_clips, 24, 56, 224, 64) ||
                    !image_has_green_ink(universal_clips, 104, 144, 64, 16) ||
                    !image_has_green_ink(universal_clips, 192, 112, 32, 16) ||
                    !save_scene("shop-special-clips/actual.bmp",
                                universal_clips)) {
                    std::cerr << "field Ares shop QA: source-specific clips "
                                 "detail or slim row selector is missing\n";
                    return 3;
                }
                if (field_audio_events != std::vector<std::uint16_t>{
                                              0x25U, 0x25U, 0x25U, 0x25U}) {
                    std::cerr << "field Ares shop QA: category and row "
                                 "movement audio requests are unexpected\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x80U); // Start return.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                const auto clips_returned = render_shop_scene();
                if (field_scene.shop_menu_open() ||
                    clips_returned == universal_clips ||
                    !save_scene("field-return-after-clips/actual.bmp",
                                clips_returned)) {
                    std::cerr << "field Ares shop QA: special-offer pixels "
                                 "survived the return redraw\n";
                    return 3;
                }

                // Exercise the complete ordinary purchase transaction. The
                // selected record is intentionally expensive relative to the
                // ROM's 20-nuyen new-game wallet, so this route also covers
                // the source insufficient-funds message before injecting a
                // larger QA wallet for the confirmation/commit branch.
                if (!enter_ares_shop()) {
                    std::cerr << "field Ares shop QA: transaction route did "
                                 "not reopen through the mode prompt\n";
                    return 3;
                }
                field_scene.set_field_funds_for_dump(20U);
                const auto inventory_before_purchase =
                    field_scene.active_runner_record().inventory_item_codes;
                const auto purchase_slot_it = std::find(
                    inventory_before_purchase.begin(),
                    inventory_before_purchase.end(), 0U);
                if (purchase_slot_it == inventory_before_purchase.end()) {
                    std::cerr << "field Ares shop QA: Joshua has no empty "
                                 "inventory slot for the purchase path\n";
                    return 3;
                }
                const auto purchase_slot = static_cast<std::size_t>(
                    std::distance(inventory_before_purchase.begin(),
                                  purchase_slot_it));
                field_audio_events.clear();
                press_field_input(0x02U); // Open the offer panel.
                press_field_input(0x08U); // Select offer 1.
                press_field_input(0x08U); // Select offer 2.
                if (field_scene.shop_menu_group() != ShopActionGroup::weapons ||
                    !field_scene.shop_menu_offers_active() ||
                    field_scene.shop_menu_cursor() != 2U) {
                    std::cerr << "field Ares shop QA: ordinary offer cursor "
                                 "did not reach the third weapon\n";
                    return 3;
                }
                press_field_input(0x40U); // Purchase with insufficient funds.
                const auto insufficient_funds = render_shop_scene();
                if (!field_scene.shop_transaction_waiting_for_event() ||
                    field_scene.shop_transaction_message() !=
                        ShopTransactionMessage::insufficient_funds ||
                    field_scene.field_funds_for_dump() != 20U ||
                    field_scene.active_runner_record().inventory_item_codes !=
                        inventory_before_purchase ||
                    image_has_green_ink(insufficient_funds, 24, 56, 224, 16) ||
                    !image_has_green_ink(insufficient_funds, 48, 80, 224, 32) ||
                    !field_shop_selector_survives(insufficient_funds, 80) ||
                    !save_scene("shop-insufficient-funds/actual.bmp",
                                insufficient_funds) ||
                    field_audio_events != std::vector<std::uint16_t>{
                                              0x25U, 0x25U, 0x25U, 0x24U}) {
                    std::cerr << "field Ares shop QA: insufficient-funds "
                                 "message, state, selector, or sound request "
                                 "is incorrect (wait="
                              << field_scene.shop_transaction_waiting_for_event()
                              << ", message="
                              << static_cast<int>(field_scene
                                      .shop_transaction_message())
                              << ", funds="
                              << field_scene.field_funds_for_dump().value_or(
                                     0xFFFFFFFFU)
                              << ", item="
                              << static_cast<int>(field_scene
                                      .active_runner_record()
                                      .inventory_item_codes[0])
                              << ", stale="
                              << image_has_green_ink(insufficient_funds, 24,
                                                     56, 224, 16)
                              << ", message-ink="
                              << image_has_green_ink(insufficient_funds, 48,
                                                     80, 224, 32)
                              << ", selector="
                              << field_shop_selector_survives(
                                     insufficient_funds, 80)
                              << ", audio=";
                    for (const auto event : field_audio_events) {
                        std::cerr << event << ',';
                    }
                    std::cerr << ")\n";
                    return 3;
                }
                press_field_input(0x40U); // Dismiss the source message.
                if (field_scene.shop_transaction_waiting_for_event() ||
                    field_scene.shop_transaction_message() !=
                        ShopTransactionMessage::none ||
                    field_scene.field_funds_for_dump() != 20U ||
                    field_scene.shop_menu_cursor() != 2U ||
                    field_scene.active_runner_record().inventory_item_codes !=
                        inventory_before_purchase) {
                    std::cerr << "field Ares shop QA: dismissing the purchase "
                                 "message changed the wallet or selection\n";
                    return 3;
                }
                field_scene.set_field_funds_for_dump(1000U);
                field_audio_events.clear();
                press_field_input(0x40U); // Open default-No confirmation.
                const auto purchase_no = render_shop_scene();
                if (!field_scene.shop_transaction_confirmation_open() ||
                    field_scene.shop_transaction_confirmation()
                            .selected_option != 1U ||
                    field_scene.field_funds_for_dump() != 1000U ||
                    field_scene.active_runner_record().inventory_item_codes !=
                        inventory_before_purchase ||
                    image_has_green_ink(purchase_no, 24, 56, 224, 16) ||
                    !image_has_green_ink(purchase_no, 24, 80, 224, 40) ||
                    !image_region_differs(purchase_no, service_background, 124,
                                          32, 24, 16) ||
                    !image_has_green_ink(purchase_no, 112, 144, 96, 16) ||
                    !field_shop_selector_survives(purchase_no, 80) ||
                    !save_scene("shop-purchase-confirmation-no/actual.bmp",
                                purchase_no)) {
                    std::cerr << "field Ares shop QA: purchase confirmation "
                                 "did not preserve the selected item state "
                                 "without leaving its detail text underneath\n";
                    return 3;
                }
                press_field_input(0x40U); // Default No cancels.
                const auto purchase_cancelled = render_shop_scene();
                if (field_scene.shop_transaction_confirmation_open() ||
                    field_scene.shop_last_purchase_result() !=
                        FieldActionApplicationResult::cancelled ||
                    field_scene.field_funds_for_dump() != 1000U ||
                    field_scene.active_runner_record().inventory_item_codes !=
                        inventory_before_purchase ||
                    field_scene.shop_menu_group() != ShopActionGroup::weapons ||
                    field_scene.shop_menu_cursor() != 2U ||
                    !save_scene("shop-purchase-cancelled/actual.bmp",
                                purchase_cancelled)) {
                    std::cerr << "field Ares shop QA: default-No changed the "
                                 "wallet, inventory, or selected offer\n";
                    return 3;
                }

                press_field_input(0x40U); // Reopen, still default No.
                press_field_input(0x01U); // Move to Yes.
                const auto purchase_yes = render_shop_scene();
                if (!field_scene.shop_transaction_confirmation_open() ||
                    field_scene.shop_transaction_confirmation()
                            .selected_option != 0U ||
                    !image_has_green_ink(purchase_yes, 24, 80, 224, 40) ||
                    image_has_green_ink(purchase_yes, 24, 56, 224, 16) ||
                    !save_scene("shop-purchase-confirmation-yes/actual.bmp",
                                purchase_yes)) {
                    std::cerr << "field Ares shop QA: Yes selection or clean "
                                 "confirmation redraw is incorrect\n";
                    return 3;
                }
                press_field_input(0x40U); // Commit the purchase.
                const auto purchased_weapon = render_shop_scene();
                if (field_scene.shop_transaction_confirmation_open() ||
                    field_scene.shop_last_purchase_result() !=
                        FieldActionApplicationResult::applied ||
                    !field_scene.field_funds_for_dump().has_value() ||
                    *field_scene.field_funds_for_dump() >= 1000U ||
                    field_scene.active_runner_record().inventory_item_codes[
                        purchase_slot] != 7U ||
                    field_scene.shop_menu_group() != ShopActionGroup::weapons ||
                    field_scene.shop_menu_cursor() != 2U ||
                    !field_shop_selector_survives(purchased_weapon, 80) ||
                    !save_scene("shop-purchase-applied/actual.bmp",
                                purchased_weapon) ||
                    field_audio_events != std::vector<std::uint16_t>{
                                              0x24U, 0x24U, 0x24U, 0x25U,
                                              0x24U}) {
                    std::cerr << "field Ares shop QA: accepted ordinary "
                                 "purchase failed to update typed state, keep "
                                 "the selected row, redraw, or request source "
                                 "sounds\n";
                    return 3;
                }

                // E-coded clips increment the runner's special-action count
                // instead of writing an ordinary inventory slot.
                press_field_input(0x01U); // Return to service categories.
                press_field_input(0x08U); // Other items.
                press_field_input(0x02U); // Open the offer panel.
                if (field_scene.shop_menu_group() !=
                        ShopActionGroup::other_items ||
                    !field_scene.shop_menu_offers_active() ||
                    field_scene.shop_menu_cursor() != 0U) {
                    std::cerr << "field Ares shop QA: transaction return did "
                                 "not retain category navigation\n";
                    return 3;
                }
                const auto funds_before_clips =
                    field_scene.field_funds_for_dump();
                const auto clips_before =
                    field_scene.active_runner_record().ammo_clips;
                field_audio_events.clear();
                press_field_input(0x40U);
                const auto clips_confirmation = render_shop_scene();
                if (!field_scene.shop_transaction_confirmation_open() ||
                    field_scene.shop_transaction_confirmation()
                            .selected_option != 1U ||
                    !image_has_green_ink(clips_confirmation, 24, 80, 224, 40) ||
                    image_has_green_ink(clips_confirmation, 24, 56, 224, 16) ||
                    !save_scene("shop-clips-confirmation-no/actual.bmp",
                                clips_confirmation)) {
                    std::cerr << "field Ares shop QA: E-offer confirmation "
                                 "did not redraw over the item detail\n";
                    return 3;
                }
                press_field_input(0x01U);
                press_field_input(0x40U);
                const auto purchased_clips = render_shop_scene();
                if (!funds_before_clips.has_value() ||
                    field_scene.shop_transaction_confirmation_open() ||
                    field_scene.active_runner_record().ammo_clips !=
                        static_cast<std::uint8_t>(clips_before + 1U) ||
                    field_scene.field_funds_for_dump() !=
                        *funds_before_clips - 25U ||
                    field_scene.active_runner_record().inventory_item_codes[
                        purchase_slot] != 7U ||
                    field_scene.shop_menu_group() !=
                        ShopActionGroup::other_items ||
                    field_scene.shop_menu_cursor() != 0U ||
                    !save_scene("shop-clips-purchased/actual.bmp",
                                purchased_clips) ||
                    field_audio_events != std::vector<std::uint16_t>{
                                              0x24U, 0x25U, 0x24U}) {
                    std::cerr << "field Ares shop QA: accepted E-offer did "
                                 "not charge 25 nuyen, increment the special "
                                 "count, or retain the row\n";
                    return 3;
                }

                // Match the independent Stim Patch sequence: a source detail
                // frame followed by the same item under the default-No
                // confirmation. The modal replaces the detail text well but
                // keeps the selected art, title, row, wallet, and HUD.
                press_field_input(0x08U);
                press_field_input(0x08U);
                field_scene.set_field_funds_for_dump(1000U);
                const auto stim_patch_detail = render_shop_scene();
                if (field_scene.shop_menu_group() !=
                        ShopActionGroup::other_items ||
                    field_scene.shop_menu_cursor() != 2U ||
                    !image_has_green_ink(stim_patch_detail, 24, 56, 224, 48) ||
                    !image_has_green_ink(stim_patch_detail, 112, 144, 96, 16) ||
                    !field_shop_selector_survives(stim_patch_detail, 80) ||
                    !save_scene("shop-stim-patch-detail/actual.bmp",
                                stim_patch_detail)) {
                    std::cerr << "field Ares shop QA: Stim Patch detail did "
                                 "not follow its typed row selection\n";
                    return 3;
                }
                const auto inventory_with_weapon =
                    field_scene.active_runner_record().inventory_item_codes;
                const auto wallet_before_stim_confirmation =
                    field_scene.field_funds_for_dump();
                const auto clips_before_stim_confirmation =
                    field_scene.active_runner_record().ammo_clips;
                field_audio_events.clear();
                press_field_input(0x40U);
                const auto stim_patch_confirmation = render_shop_scene();
                if (!field_scene.shop_transaction_confirmation_open() ||
                    field_scene.shop_transaction_confirmation()
                            .selected_option != 1U ||
                    field_scene.field_funds_for_dump() !=
                        wallet_before_stim_confirmation ||
                    field_scene.active_runner_record().inventory_item_codes !=
                        inventory_with_weapon ||
                    field_scene.active_runner_record().ammo_clips !=
                        clips_before_stim_confirmation ||
                    image_has_green_ink(stim_patch_confirmation, 24, 56, 224,
                                        16) ||
                    !image_has_green_ink(stim_patch_confirmation, 24, 80,
                                         224, 40) ||
                    !image_region_differs(stim_patch_confirmation,
                                          service_background, 124, 32, 24, 16) ||
                    !image_has_green_ink(stim_patch_confirmation, 112, 144,
                                         96, 16) ||
                    !field_shop_selector_survives(stim_patch_confirmation,
                                                  80) ||
                    !save_scene(
                        "shop-stim-patch-confirmation-no/actual.bmp",
                        stim_patch_confirmation)) {
                    std::cerr << "field Ares shop QA: Stim Patch confirmation "
                                 "does not match its selected state or leaves "
                                 "the detail text under the modal\n";
                    return 3;
                }
                press_field_input(0x40U); // Default No leaves state intact.
                if (field_scene.shop_transaction_confirmation_open() ||
                    field_scene.field_funds_for_dump() !=
                        wallet_before_stim_confirmation ||
                    field_scene.active_runner_record().inventory_item_codes !=
                        inventory_with_weapon ||
                    field_scene.active_runner_record().ammo_clips !=
                        clips_before_stim_confirmation ||
                    field_scene.shop_menu_cursor() != 2U ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U}) {
                    std::cerr << "field Ares shop QA: Stim Patch No changed "
                                 "state, selection, or source sound requests\n";
                    return 3;
                }

                // Sell the just-purchased item to exercise the shared quote,
                // default-No confirmation, refund, slot clearing, and
                // protected-item response on the same active runner.
                press_field_input(0x01U); // Return to service categories.
                for (int step = 0; step < 5; ++step) {
                    press_field_input(0x08U); // Other items -> Sell.
                }
                press_field_input(0x02U); // Open inventory row.
                for (std::size_t step = 0; step < purchase_slot; ++step) {
                    press_field_input(0x08U);
                }
                if (field_scene.shop_menu_group() != ShopActionGroup::sell ||
                    !field_scene.shop_menu_offers_active() ||
                    field_scene.shop_menu_cursor() != purchase_slot ||
                    field_scene.active_runner_record().inventory_item_codes[
                        purchase_slot] != 7U) {
                    std::cerr << "field Ares shop QA: sale inventory row did "
                                 "not select the purchased item (group="
                              << static_cast<int>(field_scene.shop_menu_group())
                              << ", panel="
                              << field_scene.shop_menu_offers_active()
                              << ", cursor="
                              << static_cast<int>(field_scene.shop_menu_cursor())
                              << ", expected=" << purchase_slot << ", code="
                              << static_cast<int>(field_scene
                                      .active_runner_record()
                                      .inventory_item_codes[purchase_slot])
                              << ")\n";
                    return 3;
                }
                const auto sale_quote = render_shop_scene();
                const auto funds_before_sale =
                    field_scene.field_funds_for_dump();
                if (!image_has_green_ink(sale_quote, 24, 80, 224, 24) ||
                    image_region_differs(purchased_weapon, sale_quote, 0, 16,
                                         272, 96) ||
                    !save_scene("shop-sale-item-detail/actual.bmp",
                                sale_quote)) {
                    std::cerr << "field Ares shop QA: the selected sale item "
                                 "detail changed before the source transaction "
                                 "was opened\n";
                    return 3;
                }
                field_audio_events.clear();
                press_field_input(0x40U);
                const auto sale_confirmation = render_shop_scene();
                const auto refund =
                    field_scene.shop_last_sale_outcome().quoted_refund;
                if (!field_scene.shop_transaction_confirmation_open() ||
                    field_scene.shop_transaction_confirmation()
                            .selected_option != 1U ||
                    refund == 0U || !funds_before_sale.has_value() ||
                    field_scene.field_funds_for_dump() != funds_before_sale ||
                    field_scene.active_runner_record().inventory_item_codes[
                        purchase_slot] != 7U ||
                    image_has_green_ink(sale_confirmation, 24, 56, 224, 16) ||
                    !image_has_green_ink(sale_confirmation, 24, 80, 224, 40) ||
                    !image_has_green_ink(sale_confirmation, 56, 88, 168, 8) ||
                    !save_scene("shop-sale-confirmation-no/actual.bmp",
                                sale_confirmation)) {
                    std::cerr << "field Ares shop QA: sale confirmation "
                                 "changed state or left old detail text visible\n";
                    return 3;
                }
                press_field_input(0x40U); // Default No cancels the sale.
                if (field_scene.shop_last_sale_outcome().result !=
                        ShopSaleOutcome::Result::cancelled ||
                    field_scene.field_funds_for_dump() != funds_before_sale ||
                    field_scene.active_runner_record().inventory_item_codes[
                        purchase_slot] != 7U) {
                    std::cerr << "field Ares shop QA: No changed the sale state\n";
                    return 3;
                }
                press_field_input(0x40U); // Reopen the sale confirmation.
                press_field_input(0x01U); // Move to Yes.
                press_field_input(0x40U); // Commit the sale.
                const auto sold_item = render_shop_scene();
                if (field_scene.shop_last_sale_outcome().result !=
                        ShopSaleOutcome::Result::removed ||
                    field_scene.field_funds_for_dump() !=
                        *funds_before_sale + refund ||
                    field_scene.active_runner_record().inventory_item_codes[
                        purchase_slot] != 0U ||
                    field_scene.active_runner_record().inventory_phase_bytes[
                        purchase_slot] != 0U ||
                    field_scene.shop_menu_group() != ShopActionGroup::sell ||
                    field_scene.shop_menu_cursor() != purchase_slot ||
                    !save_scene("shop-sale-applied/actual.bmp", sold_item) ||
                    field_audio_events != std::vector<std::uint16_t>{
                                              0x24U, 0x24U, 0x24U, 0x25U,
                                              0x24U}) {
                    std::cerr << "field Ares shop QA: confirmed sale did not "
                                 "refund, clear the runner slots, or retain "
                                 "the selected row\n";
                    return 3;
                }

                const auto protected_slot_it = std::find(
                    field_scene.active_runner_record().inventory_item_codes
                        .begin(),
                    field_scene.active_runner_record().inventory_item_codes
                        .end(),
                    0U);
                if (protected_slot_it ==
                    field_scene.active_runner_record().inventory_item_codes
                        .end()) {
                    std::cerr << "field Ares shop QA: sale did not free an "
                                 "inventory row for protected-item checks\n";
                    return 3;
                }
                const auto protected_slot = static_cast<std::size_t>(
                    std::distance(
                        field_scene.active_runner_record()
                            .inventory_item_codes.begin(),
                        protected_slot_it));
                field_scene.active_runner_record().inventory_item_codes[
                    protected_slot] = 0x29U;
                const auto funds_before_protected_sale =
                    field_scene.field_funds_for_dump();
                field_audio_events.clear();
                while (field_scene.shop_menu_cursor() < protected_slot) {
                    press_field_input(0x08U);
                }
                while (field_scene.shop_menu_cursor() > protected_slot) {
                    press_field_input(0x04U);
                }
                press_field_input(0x40U);
                const auto protected_sale = render_shop_scene();
                if (!field_scene.shop_transaction_waiting_for_event() ||
                    field_scene.shop_transaction_message() !=
                        ShopTransactionMessage::protected_sale ||
                    field_scene.active_runner_record().inventory_item_codes[
                        protected_slot] != 0x29U ||
                    field_scene.field_funds_for_dump() !=
                        funds_before_protected_sale ||
                    !image_has_green_ink(protected_sale, 40, 88, 232, 24) ||
                    !save_scene("shop-protected-sale/actual.bmp",
                                protected_sale)) {
                    std::cerr << "field Ares shop QA: protected sale was not "
                                 "rejected without touching the runner or "
                                 "wallet\n";
                    return 3;
                }

                std::cout << "field_ares_shop_qa=passed output="
                          << output.string() << "\n";
                return 0;
            }
            if (field_magic_spell_dump_path.has_value()) {
                if (field_archetype != 2U) {
                    std::cerr << "field Magic spell dump: expected the "
                                 "Gator field composition\n";
                    return 3;
                }
                const auto initial_record =
                    field_scene.active_runner_record();
                const auto* spell_table = find_us_magic_spell_table(
                    initial_record.magic_table_index);
                if (select_magic_menu_path(initial_record) !=
                        MagicMenuPath::spell_panel ||
                    spell_table == nullptr || spell_table->entry_count < 2U) {
                    std::cerr << "field Magic spell dump: active typed "
                                 "runner has no spell table (class="
                              << static_cast<int>(
                                     initial_record.archetype_index)
                              << ", gate="
                              << static_cast<int>(initial_record.auxiliary[0])
                              << ", table="
                              << initial_record.magic_table_index << ")\n";
                    return 3;
                }
                if (!field_scene.open_command_overview_for_dump() ||
                    !field_scene.seek_command_overview_page(5U)) {
                    std::cerr << "field Magic spell dump: command route "
                                 "did not reach page 5\n";
                    return 3;
                }
                std::filesystem::create_directories(
                    *field_magic_spell_dump_path);
                const auto render_scene = [&field_scene] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    return scene.save(QString::fromStdWString(
                        ((*field_magic_spell_dump_path) /
                         (std::string(name) + ".bmp"))
                            .wstring()));
                };
                const auto direct_asset = [&asset_root](
                    const std::string& relative_path) {
                    return QImage(QString::fromStdWString(
                        (asset_root / relative_path).wstring()));
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
                    return scene.copy(origin_x, origin_y, asset.width(),
                                      asset.height()) ==
                           asset.convertToFormat(QImage::Format_RGB32);
                };
                const auto visible_asset_pixels_match = [](
                    const QImage& scene, const QImage& asset,
                    const int origin_x, const int origin_y) {
                    if (asset.isNull() || origin_x < 0 || origin_y < 0 ||
                        origin_x + asset.width() > scene.width() ||
                        origin_y + asset.height() > scene.height()) {
                        return false;
                    }
                    for (int y = 0; y < asset.height(); ++y) {
                        for (int x = 0; x < asset.width(); ++x) {
                            const auto expected = asset.pixelColor(x, y);
                            if (expected.alpha() == 0) {
                                continue;
                            }
                            const auto actual =
                                scene.pixelColor(origin_x + x, origin_y + y);
                            if (actual.red() != expected.red() ||
                                actual.green() != expected.green() ||
                                actual.blue() != expected.blue()) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto font_text_pixels_match = [](
                    const QImage& scene, const QImage& font,
                    const int origin_x, const int origin_y,
                    const std::string_view text) {
                    auto cursor_x = origin_x;
                    auto cursor_y = origin_y;
                    for (const unsigned char code : text) {
                        if (code == '\n') {
                            cursor_x = origin_x;
                            cursor_y += 8;
                            continue;
                        }
                        if (code < 0x20U || code > 0x7EU) {
                            cursor_x += 8;
                            continue;
                        }
                        const auto glyph =
                            static_cast<int>(code - 0x20U);
                        const auto source_x = (glyph % 16) * 8;
                        const auto source_y = (glyph / 16) * 8;
                        if (source_x + 8 > font.width() ||
                            source_y + 8 > font.height() ||
                            cursor_x + 8 > scene.width() ||
                            cursor_y + 8 > scene.height()) {
                            return false;
                        }
                        for (int y = 0; y < 8; ++y) {
                            for (int x = 0; x < 8; ++x) {
                                const auto expected = font.pixelColor(
                                    source_x + x, source_y + y);
                                if (expected.alpha() == 0) {
                                    continue;
                                }
                                const auto actual = scene.pixelColor(
                                    cursor_x + x, cursor_y + y);
                                if (actual.red() != expected.red() ||
                                    actual.green() != expected.green() ||
                                    actual.blue() != expected.blue()) {
                                    return false;
                                }
                            }
                        }
                        cursor_x += 8;
                    }
                    return true;
                };
                const auto page_scene = render_scene();
                if (!field_command_shell_survives(page_scene, asset_root) ||
                    !field_hud_chrome_survives(page_scene, asset_root) ||
                    !field_hud_face_family_survives(
                        page_scene, asset_root, field_archetype) ||
                    !save_scene("magic_tab", page_scene)) {
                    std::cerr << "field Magic spell dump: page-5 shell/HUD "
                                 "capture failed\n";
                    return 3;
                }

                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.magic_spell_panel_open() ||
                    !field_scene.magic_selected_spell().has_value() ||
                    field_scene.magic_spell_selection().column != 0U ||
                    field_scene.magic_spell_selection().row != 0U ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Magic spell dump: A did not enter "
                                 "the typed spell panel or emit the parent "
                                 "action event\n";
                    return 3;
                }
                const auto spell_panel_scene = render_scene();
                if (spell_panel_scene == page_scene ||
                    field_command_shell_survives(spell_panel_scene,
                                                 asset_root) ||
                    !field_hud_chrome_survives(spell_panel_scene, asset_root) ||
                    !field_hud_face_family_survives(
                        spell_panel_scene, asset_root, field_archetype) ||
                    !save_scene("spell_panel", spell_panel_scene)) {
                    std::cerr << "field Magic spell dump: panel, persistent "
                                 "HUD, or archetype portrait "
                                 "failed\n";
                    return 3;
                }

                const auto first_spell =
                    field_scene.magic_selected_spell()->spell_code;
                const auto first_selection =
                    field_scene.magic_spell_selection();
                field_scene.set_controller_port_byte(0x08U);
                field_scene.advance_frame();
                const auto moved_selection =
                    field_scene.magic_spell_selection();
                const auto moved_spell = field_scene.magic_selected_spell();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (moved_selection.column == first_selection.column &&
                    moved_selection.row == first_selection.row) {
                    std::cerr << "field Magic spell dump: Right did not "
                                 "move the selection\n";
                    return 3;
                }
                if (!moved_spell.has_value() ||
                    moved_spell->spell_code == first_spell ||
                    field_scene.magic_selected_spell()->spell_code !=
                        moved_spell->spell_code ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Magic spell dump: selection text/state "
                                 "or audio changed outside the traced route\n";
                    return 3;
                }
                const auto moved_scene = render_scene();
                auto unselected_spell_icons_match = true;
                for (const auto index : {0U, 2U}) {
                    if (index >= spell_table->entry_count) {
                        unselected_spell_icons_match = false;
                        break;
                    }
                    const auto code = spell_table->entries[index].spell_code;
                    const auto icon = direct_asset(
                        "menu/field/magic-spell-icon-" +
                        std::to_string(code) + ".bmp");
                    if (!visible_asset_pixels_match(
                            moved_scene, icon,
                            32 + static_cast<int>((index % 7U) * 32U),
                            168 + static_cast<int>((index / 7U) * 24U))) {
                        unselected_spell_icons_match = false;
                        std::cerr << "field Magic spell dump: unselected "
                                     "glyph mismatch entry=" << index
                                  << " code=" << static_cast<int>(code)
                                  << "\n";
                        break;
                    }
                }
                const auto action_record_from_runner = [](
                    const RunnerRecordFields& runner) {
                    EntityActionRecord entity{};
                    entity.flags_10 = static_cast<std::uint8_t>(
                        runner.flags >> 8U);
                    entity.class_code = runner.archetype_index;
                    entity.item18_base = runner.skills[0];
                    entity.posture = runner.posture;
                    entity.attribute_77 = runner.attributes[0];
                    entity.attribute_7c = runner.attributes[5];
                    entity.item_codes = runner.inventory_item_codes;
                    entity.item_phases = runner.inventory_phase_bytes;
                    return entity;
                };
                const auto& moved_runner = field_scene.active_runner_record();
                const auto moved_entity =
                    action_record_from_runner(moved_runner);
                MagicMenuValueRefreshInput success_input{};
                success_input.current_force = field_scene.magic_current_force();
                success_input.action_cap = moved_runner.auxiliary[0];
                success_input.active_entity = moved_entity;
                MagicMenuValueRefreshCallbacks success_callbacks{};
                success_callbacks.resolve_menu_metric =
                    [&](const std::uint16_t value)
                    -> std::optional<std::uint16_t> {
                    std::vector<std::uint8_t> source_action_indices{
                        0U, static_cast<std::uint8_t>(
                                spell_table->entry_count - 1U)};
                    for (std::size_t index = 0;
                         index < spell_table->entry_count &&
                         index < kMagicSpellTableEntryCapacity;
                         ++index) {
                        source_action_indices.push_back(
                            spell_table->entries[index].spell_code);
                        source_action_indices.push_back(
                            spell_table->entries[index].force_limit);
                    }
                    const auto selection = field_scene.magic_spell_selection();
                    MenuMetricLookupInput lookup_input{};
                    lookup_input.menu_row = selection.row;
                    lookup_input.menu_column = selection.column;
                    lookup_input.source_action_indices =
                        std::move(source_action_indices);
                    lookup_input.value = value;
                    lookup_input.entity = moved_entity;
                    lookup_input.item_catalog = &us_item_catalog();
                    lookup_input.selector_table =
                        &us_special_item_modifier_selector_table();
                    const auto lookup =
                        resolve_menu_metric_action(lookup_input);
                    if (!lookup.has_value() ||
                        lookup->modifiers.source_data_missing) {
                        return std::nullopt;
                    }
                    return lookup->modifiers.value;
                };
                const auto success =
                    refresh_magic_menu_success_or_defense_values(
                        success_input, success_callbacks);
                auto success_bar_matches = false;
                if (!success.grid_requests.empty()) {
                    const auto row = success.grid_requests.front().row;
                    const auto asset = direct_asset(
                        "menu/field/magic-metric-bar-" +
                        std::to_string(row) + ".bmp");
                    success_bar_matches = region_matches(
                        moved_scene, asset, 32, 88);
                }

                MagicMenuValueRefreshInput defense_input{};
                defense_input.refresh_defense_strips = true;
                defense_input.active_entity = moved_entity;
                defense_input.attribute_77 = moved_entity.attribute_77;
                defense_input.attribute_7c = moved_entity.attribute_7c;
                const auto& party_records =
                    field_scene.field_party_records_for_dump();
                for (std::size_t index = 0;
                     index < defense_input.ready_entities.size(); ++index) {
                    defense_input.ready_entities[index] =
                        action_record_from_runner(party_records[index]);
                }
                const auto defense =
                    refresh_magic_menu_success_or_defense_values(
                        defense_input);
                auto defense_bars_match = !defense.grid_requests.empty();
                for (const auto& request : defense.grid_requests) {
                    const auto asset = direct_asset(
                        "menu/field/magic-metric-bar-" +
                        std::to_string(request.row) + ".bmp");
                    const auto y = request.strip ==
                                           MagicMenuValueStrip::defense_physical
                                       ? 128
                                       : 144;
                    defense_bars_match = defense_bars_match &&
                        region_matches(moved_scene, asset, 32, y);
                }

                const auto* moved_definition =
                    find_us_magic_spell(moved_spell->spell_code);
                auto drain_bar_matches = false;
                if (moved_definition != nullptr) {
                    const MagicActionEffectValueInput effect_input{
                        field_scene.magic_current_force(), moved_entity,
                        *moved_definition};
                    const auto effect =
                        refresh_magic_action_effect_value(effect_input);
                    if (effect.residual_valid && effect.residual_effect > 0 &&
                        effect.damage_grid.has_value()) {
                        const auto asset = direct_asset(
                            "menu/field/magic-damage-bar-" +
                            std::to_string(
                                effect.damage_grid->effect_row) + ".bmp");
                        drain_bar_matches = region_matches(
                            moved_scene, asset, 32, 104);
                    }
                }
                const auto posture = std::min<std::size_t>(
                    moved_runner.posture, 4U);
                const auto posture_bar = direct_asset(
                    "menu/field/magic-posture-bar-" +
                    std::to_string(posture) + ".bmp");
                const auto posture_bar_matches = region_matches(
                    moved_scene, posture_bar, 160, 136);
                const auto selected_spell_icon = direct_asset(
                    "menu/field/magic-spell-icon-" +
                    std::to_string(moved_spell->spell_code) + ".bmp");
                const auto selected_spell_icon_matches =
                    visible_asset_pixels_match(
                        moved_scene, selected_spell_icon, 80, 24);
                if (moved_scene == spell_panel_scene ||
                    moved_spell->spell_code != 10U ||
                    !unselected_spell_icons_match ||
                    !selected_spell_icon_matches ||
                    !success_bar_matches || !drain_bar_matches ||
                    !defense_bars_match || !posture_bar_matches ||
                    !field_hud_chrome_survives(moved_scene, asset_root) ||
                    !field_hud_face_family_survives(
                        moved_scene, asset_root, field_archetype) ||
                    !save_scene("actual", moved_scene) ||
                    !save_scene("spell_selection_moved", moved_scene)) {
                    std::cerr << "field Magic spell dump: selection, source "
                                 "meter assets, or adjacent HUD did not match "
                                 "the typed source state\n";
                    return 3;
                }

                // The shared ROM cursor service uses a six-frame countdown;
                // including the expiry call, each later phase lasts seven
                // field frames. Save every phase and verify the four-frame
                // cycle after the navigation reset.
                auto expected_phase = field_scene.magic_cursor_phase();
                auto previous_phase_scene = moved_scene;
                for (std::uint8_t phase_step = 1U; phase_step <= 4U;
                     ++phase_step) {
                    for (int frame = 0; frame < 7; ++frame) {
                        field_scene.set_controller_port_byte(0U);
                        field_scene.advance_frame();
                    }
                    expected_phase = static_cast<std::uint8_t>(
                        (expected_phase + 1U) % 4U);
                    if (field_scene.magic_cursor_phase() != expected_phase) {
                        std::cerr << "field Magic spell dump: cursor phase "
                                     "did not advance at the seven-frame "
                                     "cadence\n";
                        return 3;
                    }
                    const auto phase_scene = render_scene();
                    if (phase_scene == previous_phase_scene ||
                        !save_scene("cursor_phase_" +
                                        std::to_string(expected_phase),
                                    phase_scene)) {
                        std::cerr << "field Magic spell dump: cursor phase "
                                     "frame failed to render\n";
                        return 3;
                    }
                    previous_phase_scene = phase_scene;
                }

                field_scene.set_controller_port_byte(0x80U);
                field_scene.advance_frame();
                if (field_scene.magic_spell_panel_open() ||
                    field_scene.magic_selected_spell().has_value() ||
                    !field_scene.command_overview_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 5U ||
                    field_scene.active_runner_record().magic_force !=
                        initial_record.magic_force ||
                    field_scene.active_runner_record().auxiliary !=
                        initial_record.auxiliary ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Magic spell dump: Start did not "
                                 "return to page 5 without record changes\n";
                    return 3;
                }
                const auto return_scene = render_scene();
                if (return_scene != page_scene ||
                    !save_scene("return", return_scene)) {
                    std::cerr << "field Magic spell dump: return did not "
                                 "restore page 5\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.magic_spell_panel_open() ||
                    field_scene.magic_spell_selection().column != 0U ||
                    field_scene.magic_spell_selection().row != 0U ||
                    !field_scene.magic_selected_spell().has_value() ||
                    field_scene.magic_selected_spell()->spell_code !=
                        first_spell ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U}) {
                    std::cerr << "field Magic spell dump: re-entry did not "
                                 "reset the source cursor/table state\n";
                    return 3;
                }
                if (!save_scene("spell_panel_reopened", render_scene())) {
                    std::cerr << "field Magic spell dump: could not save "
                                 "re-entry frame\n";
                    return 3;
                }

                const auto press_field_port = [&](const std::uint8_t port) {
                    field_scene.set_controller_port_byte(port);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                if (field_scene.magic_maximum_force() != 3U ||
                    field_scene.magic_current_force() != 3U) {
                    std::cerr << "field Magic spell dump: selected force "
                                 "did not initialize from the spell table\n";
                    return 3;
                }
                press_field_port(0x40U); // A enters the force loop.
                const auto force_selection_scene = render_scene();
                const auto magic_font = direct_asset(
                    "menu/field/magic-font-palette-0.bmp");
                constexpr std::string_view kManaZapDescription =
                    "A small zap of energy that\ndelivers a nasty jolt.";
                if (!field_scene.magic_force_selection_open() ||
                    !font_text_pixels_match(
                        force_selection_scene, magic_font, 64, 56,
                        "Set Force & Posture.") ||
                    font_text_pixels_match(force_selection_scene, magic_font,
                                           32, 48, kManaZapDescription) ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U} ||
                    !save_scene("force_selection", force_selection_scene)) {
                    std::cerr << "field Magic spell dump: A did not enter "
                                 "the source Set Force & Posture view, clear "
                                 "the spell description, or stay silent\n";
                    return 3;
                }
                auto expected_force_cursor_phase =
                    field_scene.magic_cursor_phase();
                auto previous_force_cursor_scene = force_selection_scene;
                for (std::uint8_t phase_step = 1U; phase_step <= 4U;
                     ++phase_step) {
                    for (int frame = 0; frame < 7; ++frame) {
                        field_scene.set_controller_port_byte(0U);
                        field_scene.advance_frame();
                    }
                    expected_force_cursor_phase =
                        static_cast<std::uint8_t>(
                            (expected_force_cursor_phase + 1U) % 4U);
                    const auto force_cursor_scene = render_scene();
                    if (field_scene.magic_cursor_phase() !=
                            expected_force_cursor_phase ||
                        !field_scene.magic_force_selection_open() ||
                        field_scene.magic_current_force() != 3U ||
                        force_cursor_scene == previous_force_cursor_scene ||
                        field_audio_events !=
                            std::vector<std::uint16_t>{0x24U, 0x24U} ||
                        !save_scene("force_cursor_phase_" +
                                        std::to_string(
                                            expected_force_cursor_phase),
                                    force_cursor_scene)) {
                        std::cerr << "field Magic spell dump: force cursor "
                                     "did not advance through a visible "
                                     "seven-frame phase, or action added sound\n";
                        return 3;
                    }
                    previous_force_cursor_scene = force_cursor_scene;
                }
                if (previous_force_cursor_scene != force_selection_scene) {
                    std::cerr << "field Magic spell dump: force cursor did "
                                 "not complete a four-phase cycle\n";
                    return 3;
                }
                press_field_port(0x04U); // Left lowers force.
                if (!field_scene.magic_force_selection_open() ||
                    field_scene.magic_current_force() != 2U ||
                    field_scene.active_runner_record().magic_force != 2U ||
                    !save_scene("force_lowered", render_scene())) {
                    std::cerr << "field Magic spell dump: Left did not "
                                 "lower the typed force and meter\n";
                    return 3;
                }
                press_field_port(0x08U); // Right raises force.
                if (field_scene.magic_current_force() != 3U ||
                    field_scene.active_runner_record().magic_force != 3U ||
                    !save_scene("force_raised", render_scene())) {
                    std::cerr << "field Magic spell dump: Right did not "
                                 "restore the selected force\n";
                    return 3;
                }
                press_field_port(0x02U); // Down enters posture adjustment.
                if (!field_scene.magic_posture_selection_open() ||
                    !save_scene("posture_selection", render_scene())) {
                    std::cerr << "field Magic spell dump: Down did not "
                                 "enter the posture loop\n";
                    return 3;
                }
                press_field_port(0x08U); // Right raises posture.
                const auto expected_posture = static_cast<std::uint8_t>(
                    std::min<unsigned>(
                        static_cast<unsigned>(initial_record.posture) + 1U,
                        4U));
                if (field_scene.active_runner_record().posture !=
                        expected_posture ||
                    !field_scene.magic_posture_selection_open() ||
                    !save_scene("posture_raised", render_scene())) {
                    std::cerr << "field Magic spell dump: posture row did "
                                 "not follow the runner record\n";
                    return 3;
                }
                press_field_port(0x20U); // B finishes the equipment action.
                const auto equipped_scene = render_scene();
                const auto magic_hud_palette =
                    select_hud_glyph_attribute_flags(HudGlyphAttributeInput{
                        static_cast<std::int8_t>(first_spell + 1U),
                        static_cast<std::int8_t>(field_scene.active_runner_record()
                                                     .archetype_index),
                        -1});
                const auto magic_hud_item = direct_asset(
                    "field/hud/magic-item-" +
                    std::to_string(first_spell) + "-palette-" +
                    (magic_hud_palette == 0x6000U ? "1" : "0") + ".bmp");
                if (!field_scene.magic_spell_panel_open() ||
                    field_scene.active_runner_record().equipped_inventory_slot !=
                        -1 ||
                    field_scene.active_runner_record().equipped_item_code !=
                        static_cast<std::uint8_t>(first_spell + 1U) ||
                    field_scene.active_runner_record().magic_force != 3U ||
                    field_scene.active_runner_record().posture !=
                        expected_posture ||
                    magic_hud_palette != 0x6000U ||
                    !visible_asset_pixels_match(equipped_scene,
                                                magic_hud_item, 288, 64) ||
                    (initial_record.equipped_inventory_slot >= 0 &&
                     initial_record.equipped_item_code != 0U &&
                     field_scene.active_runner_record().ammo_rounds != 3U) ||
                    font_text_pixels_match(
                        equipped_scene, magic_font, 64, 56,
                        "Set Force & Posture.") ||
                    !font_text_pixels_match(equipped_scene, magic_font, 32,
                                            48, kManaZapDescription) ||
                    !field_hud_chrome_survives(equipped_scene, asset_root) ||
                    !save_scene("spell_equipped", equipped_scene)) {
                    std::cerr << "field Magic spell dump: force/posture "
                                 "finalization did not update typed equipment, "
                                 "description, and source spell HUD art\n";
                    return 3;
                }

                field_scene.set_controller_port_byte(0x80U);
                field_scene.advance_frame();
                if (field_scene.magic_spell_panel_open() ||
                    !field_scene.command_overview_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 5U ||
                    field_scene.active_runner_record().equipped_item_code !=
                        static_cast<std::uint8_t>(first_spell + 1U) ||
                    field_scene.active_runner_record().magic_force != 3U ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U}) {
                    std::cerr << "field Magic spell dump: Start did not "
                                 "return to page 5 with spell state intact\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!save_scene("equipped_return", render_scene())) {
                    std::cerr << "field Magic spell dump: could not save "
                                 "the post-equipment return frame\n";
                    return 3;
                }
                std::cout << "field_magic_spell_dump=written path="
                          << field_magic_spell_dump_path->string()
                          << " table=" << initial_record.magic_table_index
                          << " spell=" << static_cast<int>(first_spell)
                          << "->" << static_cast<int>(moved_spell->spell_code)
                          << " cursor=4x7frames force=1..3 posture=0..4 "
                             "equip=record+0x57/+0x9D return=page5\n";
                return 0;
            }
            if (field_magic_dump_path.has_value()) {
                // This field composition's active live record is class 1,
                // even with the Gator startup selection. Set its typed
                // +0x7F resource/action cap to zero to exercise the ROM's
                // class<2 fallback branch, without fabricating a spell panel.
                if (field_archetype != 2U) {
                    std::cerr << "field Magic dump: Gator did not reach "
                                 "the expected QA composition\n";
                    return 3;
                }
                field_scene.active_runner_record().auxiliary[0] = 0U;
                if (!field_scene.open_command_overview_for_dump() ||
                    !field_scene.seek_command_overview_page(5U)) {
                    std::cerr << "field Magic dump: command route did not "
                                 "reach page 5\n";
                    return 3;
                }
                std::filesystem::create_directories(
                    *field_magic_dump_path);
                const auto render_scene = [&field_scene] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    return scene.save(QString::fromStdWString(
                        ((*field_magic_dump_path) /
                         (std::string(name) + ".bmp"))
                            .wstring()));
                };
                const auto same_runner_action_state =
                    [](const RunnerRecordFields& left,
                       const RunnerRecordFields& right) {
                        return left.archetype_index == right.archetype_index &&
                               left.auxiliary == right.auxiliary &&
                               left.attributes == right.attributes &&
                               left.skills == right.skills &&
                               left.inventory_item_codes ==
                                   right.inventory_item_codes &&
                               left.inventory_phase_bytes ==
                                   right.inventory_phase_bytes &&
                               left.magic_table_index ==
                                   right.magic_table_index &&
                               left.magic_force == right.magic_force &&
                               left.equipped_inventory_slot ==
                                   right.equipped_inventory_slot &&
                               left.equipped_item_code ==
                                   right.equipped_item_code &&
                               left.posture == right.posture &&
                               left.ammo_rounds == right.ammo_rounds &&
                               left.ammo_clips == right.ammo_clips;
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
                    const auto expected =
                        asset.convertToFormat(QImage::Format_RGB32);
                    const auto actual = scene.copy(
                        origin_x, origin_y, asset.width(), asset.height());
                    return actual == expected;
                };

                const auto page_scene = render_scene();
                if (!field_command_shell_survives(page_scene, asset_root) ||
                    !field_hud_chrome_survives(page_scene, asset_root) ||
                    !field_hud_face_family_survives(
                        page_scene, asset_root, field_archetype) ||
                    !save_scene("magic_tab", page_scene)) {
                    std::cerr << "field Magic dump: page-5 shell, HUD, "
                                 "portrait, or initial capture failed\n";
                    return 3;
                }
                const auto initial_record =
                    field_scene.active_runner_record();
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.magic_defense_open() ||
                    !field_scene.command_overview_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 5U) {
                    std::cerr << "field Magic dump: page-5 A did not open "
                                 "the Magical Defense fallback (open="
                              << field_scene.command_overview_open()
                              << ", page="
                              << static_cast<int>(field_scene
                                     .command_overview_view()
                                     .source_state.primary_page)
                              << ", modal="
                              << field_scene.magic_defense_open()
                              << ", class="
                              << static_cast<int>(field_scene
                                     .active_runner_record()
                                     .archetype_index)
                              << ", gate="
                              << static_cast<int>(field_scene
                                     .active_runner_record()
                                     .auxiliary[0])
                              << ")\n";
                    return 3;
                }
                if (field_audio_events !=
                    std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Magic dump: command-action sound "
                                 "event 0x24 was not emitted exactly once\n";
                    return 3;
                }
                const auto& physical =
                    field_scene.magic_physical_defense_view();
                const auto& mana = field_scene.magic_mana_defense_view();
                std::array<EntityActionRecord,
                           FieldPartyRosterRuntime::kSlotCount>
                    ready_entities{};
                const auto& party_records =
                    field_scene.field_party_records_for_dump();
                for (std::size_t index = 0; index < ready_entities.size();
                     ++index) {
                    const auto& runner = party_records[index];
                    auto& entity = ready_entities[index];
                    // Independent expectation for the Qt adapter: the
                    // source ready gate is byte +0x10, the high byte of
                    // RunnerRecordFields::flags (a big-endian word).
                    entity.flags_10 =
                        static_cast<std::uint8_t>(runner.flags >> 8U);
                    entity.class_code = runner.archetype_index;
                    entity.item18_base = runner.skills[0];
                    entity.attribute_77 = runner.attributes[0];
                    entity.attribute_7c = runner.attributes[5];
                    entity.item_codes = runner.inventory_item_codes;
                    entity.item_phases = runner.inventory_phase_bytes;
                }
                const auto ready_bonus =
                    calculate_ready_entity_action_bonus(ready_entities);
                const auto expected_physical = static_cast<std::uint8_t>(
                    static_cast<std::uint8_t>(ready_bonus) +
                    initial_record.attributes[0]);
                const auto expected_mana = static_cast<std::uint8_t>(
                    static_cast<std::uint8_t>(ready_bonus) +
                    initial_record.attributes[5]);
                const auto valid_view = [](const MagicDefenseStripView& view) {
                    const auto expected_row = view.value == 0U
                        ? std::uint16_t{0U}
                        : static_cast<std::uint16_t>(view.value - 1U);
                    return view.bar_row == expected_row &&
                           view.bar_row < 23U;
                };
                const auto expected_physical_bar = QImage(QString::fromStdWString(
                    (asset_root / "menu" / "field" /
                     ("defense-bar-" + std::to_string(physical.bar_row) +
                      ".bmp"))
                        .wstring()));
                const auto expected_mana_bar = QImage(QString::fromStdWString(
                    (asset_root / "menu" / "field" /
                     ("defense-bar-" + std::to_string(mana.bar_row) + ".bmp"))
                        .wstring()));
                const auto actual_scene = render_scene();
                if (physical.value != expected_physical ||
                    mana.value != expected_mana ||
                    !valid_view(physical) || !valid_view(mana) ||
                    !region_matches(actual_scene, expected_physical_bar,
                                    120, 80) ||
                    !region_matches(actual_scene, expected_mana_bar,
                                    120, 104)) {
                    std::cerr << "field Magic dump: physical/mana values, "
                                 "source rows, or strip placement diverged\n";
                    return 3;
                }
                int title_ink = 0;
                for (int y = 48; y < 56; ++y) {
                    for (int x = 112; x < 256; ++x) {
                        if (page_scene.pixel(x, y) !=
                            actual_scene.pixel(x, y)) {
                            ++title_ink;
                        }
                    }
                }
                if (title_ink == 0 ||
                    !field_command_shell_survives(actual_scene, asset_root) ||
                    !field_hud_chrome_survives(actual_scene, asset_root) ||
                    !field_hud_face_family_survives(
                        actual_scene, asset_root, field_archetype) ||
                    !save_scene("actual", actual_scene)) {
                    std::cerr << "field Magic dump: title or persistent "
                                 "screen layers failed\n";
                    return 3;
                }
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                }
                const auto held_scene = render_scene();
                if (held_scene != actual_scene ||
                    !save_scene("held", held_scene) ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Magic dump: static wait frame "
                                 "changed without controller input\n";
                    return 3;
                }

                // A fresh directional edge satisfies ROM 0x0D94's
                // one-event wait and returns to the same command page.
                field_scene.set_controller_port_byte(0x04U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.magic_defense_open() ||
                    !field_scene.command_overview_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 5U ||
                    !same_runner_action_state(
                        initial_record, field_scene.active_runner_record()) ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Magic dump: return edge escaped "
                                 "page 5 or mutated the runner record\n";
                    return 3;
                }
                const auto return_scene = render_scene();
                if (return_scene != page_scene ||
                    !save_scene("return", return_scene)) {
                    std::cerr << "field Magic dump: return did not restore "
                                 "the same page-5 frame\n";
                    return 3;
                }
                std::cout << "field_magic_dump=written path="
                          << field_magic_dump_path->string()
                          << " class="
                          << static_cast<int>(initial_record.archetype_index)
                          << " physical=" << physical.value << ":"
                          << static_cast<int>(physical.bar_row)
                          << " mana=" << mana.value << ":"
                          << static_cast<int>(mana.bar_row)
                          << " held_frames=30 return=page5\n";
                return 0;
            }
            if (field_command_dump_path.has_value()) {
                const auto capture_field_scene = [&field_scene] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto field_before_command = capture_field_scene();
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
                const auto command_scene = capture_field_scene();
                // Focusing the Attributes/Skills command row must leave the
                // shared command shell intact. ROM 0xD01C is entered only by
                // the following A action.
                if (!field_command_shell_survives(
                        command_scene, asset_root, "background.bmp")) {
                    std::cerr << "field command dump: source command shell "
                                 "was not preserved around the overlays\n";
                    return 3;
                }
                const auto hud_chrome_present =
                    field_hud_chrome_survives(command_scene, asset_root);
                const auto hud_face_present = field_hud_face_family_survives(
                    command_scene, asset_root, field_archetype);
                if (!hud_chrome_present || !hud_face_present) {
                    std::cerr << "field command dump: persistent field HUD "
                                 "was not preserved above the command plane "
                              << "(chrome=" << hud_chrome_present
                              << ", face=" << hud_face_present << ")\n";
                    return 3;
                }
                if (field_command_page.has_value() &&
                    *field_command_page < 2U) {
                    const auto& selection =
                        field_scene.command_overview_view().source_state;
                    const auto selected_slot =
                        static_cast<std::size_t>(selection.secondary_page) *
                            2U +
                        selection.primary_page;
                    const auto& items =
                        field_scene.active_runner_record().inventory_item_codes;
                    if (selection.primary_page != *field_command_page ||
                        selected_slot >= items.size() ||
                        items[selected_slot] == 0U ||
                        !field_inventory_selector_survives(
                            command_scene, selection.primary_page,
                            selection.secondary_page)) {
                        std::cerr
                            << "field command dump: selected inventory item "
                               "did not retain its one-pixel gold outline\n";
                        return 3;
                    }
                }
                if (!command_scene.save(QString::fromStdWString(
                        field_command_dump_path->wstring()))) {
                    std::cerr << "field command dump: could not write "
                              << field_command_dump_path->string() << '\n';
                    return 3;
                }

                // The command page has no source timer or animated cursor;
                // drawing it across 30 neutral VBlanks must leave the whole
                // composition stable while its modal state owns the window.
                for (std::uint8_t frame = 0U; frame < 30U; ++frame) {
                    field_scene.advance_frame();
                }
                if (capture_field_scene() != command_scene) {
                    std::cerr << "field command dump: neutral frames changed "
                                 "the static command page\n";
                    return 3;
                }

                // Release the synthetic Start from entry, then press it to
                // close through the same controller/coordinator path. The
                // returned full frame must equal the fresh field composition;
                // this catches stale command buttons and selectors.
                field_scene.set_controller_port_byte(0x80U);
                field_scene.advance_frame();
                if (field_scene.command_overview_open()) {
                    std::cerr << "field command dump: Start did not return to "
                                 "the field\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                const auto returned_field_scene = capture_field_scene();
                if (returned_field_scene != field_before_command) {
                    std::cerr << "field command dump: 0x2BD2 return did not "
                                 "redraw the original field composition\n";
                    return 3;
                }
                auto return_path = *field_command_dump_path;
                return_path.replace_filename(
                    field_command_dump_path->stem().wstring() +
                    L"-return-field.bmp");
                if (!returned_field_scene.save(QString::fromStdWString(
                        return_path.wstring()))) {
                    std::cerr << "field command dump: could not write "
                              << return_path.string() << '\n';
                    return 3;
                }
                std::cout << "field_command_dump=written path="
                          << field_command_dump_path->string()
                          << " selector="
                          << static_cast<int>(field_scene.command_overview_view()
                                                   .source_state.primary_page)
                          << " static_frames=30 clean_return=1"
                          << "\n";
                return 0;
            }
            if (field_attributes_skills_dump_path.has_value()) {
                std::filesystem::create_directories(
                    *field_attributes_skills_dump_path);
                const auto capture_scene = [&field_scene] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    const auto path = *field_attributes_skills_dump_path /
                                      (std::string(name) + ".bmp");
                    if (!scene.save(QString::fromStdWString(path.wstring()))) {
                        std::cerr << "field Attributes/Skills dump: could "
                                     "not write "
                                  << path.string() << '\n';
                        return false;
                    }
                    return true;
                };
                const auto press = [&](const std::uint8_t port_byte) {
                    field_scene.set_controller_port_byte(port_byte);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };

                // Give the diagnostic runner a known valid Body increase so
                // the same fixture can exercise both prompt choices. Seed it
                // before command entry so derived combat bars agree too.
                field_scene.active_runner_record().attributes[0U] = 1U;
                field_scene.active_runner_record().auxiliary[2U] = 50U;
                if (!field_scene.open_command_overview_for_dump() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 6U) {
                    std::cerr << "field Attributes/Skills dump: command "
                                 "shell did not initialize on page 6\n";
                    return 3;
                }
                const auto page_six_scene = capture_scene();
                if (!save_scene("initial-command-page6", page_six_scene)) {
                    std::cerr << "field Attributes/Skills dump: could not "
                                 "write initial page-6 image\n";
                    return 3;
                }
                if (!field_scene.seek_command_overview_page(3U) ||
                    field_scene.attributes_skills_menu_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 3U) {
                    std::cerr << "field Attributes/Skills dump: focusing "
                                 "row 3 opened the page or lost the shell\n";
                    return 3;
                }
                const auto focused_scene = capture_scene();
                if (!field_command_shell_survives(
                        focused_scene, asset_root, "background.bmp") ||
                    !field_hud_chrome_survives(focused_scene, asset_root) ||
                    !field_hud_face_family_survives(
                        focused_scene, asset_root, field_archetype) ||
                    !save_scene("focused-command-row3", focused_scene)) {
                    std::cerr << "field Attributes/Skills dump: focus did "
                                 "not retain the command shell and HUD\n";
                    return 3;
                }
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                if (capture_scene() != focused_scene) {
                    std::cerr << "field Attributes/Skills dump: focused "
                                 "command row changed without input\n";
                    return 3;
                }

                field_audio_events.clear();
                press(0x40U);
                if (!field_scene.attributes_skills_menu_open() ||
                    field_scene.attributes_skills_menu()
                        .allows_karma_upgrades() ||
                    field_scene.attributes_skills_menu()
                        .state().selection_data_available ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Attributes/Skills dump: A did not "
                                 "open the read-only page with the source "
                                 "action sound event 0x24\n";
                    return 3;
                }
                field_audio_events.clear();
                const QImage attributes_background(QString::fromStdWString(
                    (asset_root / "menu" / "field" /
                     "attributes-background.bmp")
                        .wstring()));
                const auto initial_scene = capture_scene();
                const auto actual_saved = save_scene("actual", initial_scene);
                bool background_region_matches =
                    attributes_background.size() ==
                    QSize(kSceneWidth, kSceneHeight);
                for (int y = 0; background_region_matches && y < 16; ++y) {
                    for (int x = 0; x < kSceneWidth; ++x) {
                        if (initial_scene.pixelColor(x, y) !=
                            attributes_background.pixelColor(x, y)) {
                            background_region_matches = false;
                            break;
                        }
                    }
                }
                if (!background_region_matches || !actual_saved ||
                    image_region_differs(initial_scene,
                                         attributes_background,
                                         48, 192, 272, 32)) {
                    std::cerr << "field Attributes/Skills dump: full-screen "
                                 "read-only view or lower Karma total was "
                                 "not drawn cleanly "
                                 "(asset=" << attributes_background.width()
                              << 'x' << attributes_background.height()
                              << ", scene=" << initial_scene.width() << 'x'
                              << initial_scene.height() << ", row_matches="
                              << background_region_matches << ", saved="
                              << actual_saved << ")\n";
                    return 3;
                }
                if (!image_region_has_color(initial_scene, 120, 160, 40, 8,
                                            QColor(0, 219, 0)) ||
                    image_region_has_color(initial_scene, 64, 200, 224, 8,
                                           QColor(109, 73, 146))) {
                    std::cerr << "field Attributes/Skills dump: command "
                                 "view did not show its current Karma total "
                                 "or leaked Hotel upgrade costs\n";
                    return 3;
                }
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                const auto view_only_frame_30 = capture_scene();
                if (view_only_frame_30 != initial_scene ||
                    !save_scene("frame-30", view_only_frame_30)) {
                    std::cerr << "field Attributes/Skills dump: static page "
                                 "animated without a source timer or could "
                                 "not save frame 30\n";
                    return 3;
                }

                const auto& initial_record =
                    field_scene.active_runner_record();
                const auto initial_record_copy = initial_record;
                press(0x02U);
                if (field_scene.attributes_skills_menu().state().cursor.row !=
                        1U ||
                    field_scene.active_runner_record().attributes !=
                        initial_record_copy.attributes ||
                    field_scene.active_runner_record().auxiliary !=
                        initial_record_copy.auxiliary) {
                    std::cerr << "field Attributes/Skills dump: Down did not "
                                 "move only the selection\n";
                    return 3;
                }
                const auto moved_scene = capture_scene();
                if (moved_scene == initial_scene ||
                    !save_scene("selection-moved", moved_scene)) {
                    std::cerr << "field Attributes/Skills dump: selected "
                                 "attribute did not redraw\n";
                    return 3;
                }
                press(0x01U);
                if (field_scene.attributes_skills_menu().state().cursor.row !=
                    0U) {
                    std::cerr << "field Attributes/Skills dump: Up did not "
                                 "restore Body selection\n";
                    return 3;
                }

                // A/B/Start leave the command-menu view because ROM 0xD01C marks
                // FFF116 negative. None of these exits can spend Karma here.
                const auto read_only_record =
                    field_scene.active_runner_record();
                press(0x40U);
                const auto returned_scene = capture_scene();
                const auto returned_scene_saved =
                    save_scene("returned-command-page6", returned_scene);
                const auto returned_page = field_scene
                                               .command_overview_view()
                                               .source_state.primary_page;
                if (!field_scene.command_overview_open() ||
                    field_scene.attributes_skills_menu_open() ||
                    returned_page != 6U ||
                    returned_scene != page_six_scene ||
                    field_scene.active_runner_record().attributes !=
                        read_only_record.attributes ||
                    field_scene.active_runner_record().auxiliary !=
                        read_only_record.auxiliary ||
                    !field_audio_events.empty() ||
                    !returned_scene_saved) {
                    std::cerr << "field Attributes/Skills dump: A did "
                                 "not restore a clean command shell (open="
                              << field_scene.command_overview_open()
                              << ", attributes="
                              << field_scene.attributes_skills_menu_open()
                              << ", page=" << static_cast<int>(returned_page)
                              << ", pixels_match="
                              << (returned_scene == page_six_scene)
                              << ", saved=" << returned_scene_saved << ")\n";
                    return 3;
                }

                // B enters the same menu action, then exits the view-only
                // page without touching the active runner.
                if (!field_scene.seek_command_overview_page(3U) ||
                    field_scene.attributes_skills_menu_open()) {
                    std::cerr << "field Attributes/Skills dump: could not "
                                 "reopen focused row 3\n";
                    return 3;
                }
                field_audio_events.clear();
                press(0x40U);
                if (!field_scene.attributes_skills_menu_open() ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field Attributes/Skills dump: second A "
                                 "did not enter the page through the source "
                                 "action sound\n";
                    return 3;
                }
                field_audio_events.clear();
                press(0x10U);
                const auto b_return_scene = capture_scene();
                if (field_scene.attributes_skills_menu_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 6U ||
                    b_return_scene != page_six_scene ||
                    field_scene.active_runner_record().attributes !=
                        read_only_record.attributes ||
                    field_scene.active_runner_record().auxiliary !=
                        read_only_record.auxiliary ||
                    !field_audio_events.empty() ||
                    !save_scene("returned-command-page6-after-b",
                                b_return_scene)) {
                    std::cerr << "field Attributes/Skills dump: B did not "
                                 "restore the read-only command view cleanly\n";
                    return 3;
                }

                if (!field_scene.seek_command_overview_page(3U) ||
                    field_scene.attributes_skills_menu_open()) {
                    std::cerr << "field Attributes/Skills dump: could not "
                                 "reopen focused row 3 for Start return\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.attributes_skills_menu_open()) {
                    std::cerr << "field Attributes/Skills dump: A did not "
                                 "reopen the page for Start return\n";
                    return 3;
                }
                field_audio_events.clear();
                press(0x80U);
                const auto start_return_scene = capture_scene();
                if (!field_scene.command_overview_open() ||
                    field_scene.attributes_skills_menu_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 6U ||
                    start_return_scene != page_six_scene ||
                    field_scene.active_runner_record().attributes !=
                        read_only_record.attributes ||
                    field_scene.active_runner_record().auxiliary !=
                        read_only_record.auxiliary ||
                    !field_audio_events.empty() ||
                    !save_scene("returned-command-page6-after-start",
                                start_return_scene)) {
                    std::cerr << "field Attributes/Skills dump: Start did "
                                 "not restore the read-only command view "
                                 "cleanly\n";
                    return 3;
                }
                std::cout << "field_attributes_skills_dump=written path="
                          << field_attributes_skills_dump_path->string()
                          << " focus_no_open=1 read_only=1 selected_cell=1 "
                             "a_b_start_return=1 clean_return=1 "
                             "static_frames=30\n";
                return 0;
            }
            if (field_cyberware_dump_path.has_value()) {
                if (field_cyberware_empty_qa) {
                    // Diagnostic fixture for the source zero-word branch;
                    // production field records continue to come from the
                    // normal startup/session bootstrap.
                    field_scene.active_runner_record().cyberware_flags = 0U;
                }
                if (!field_scene.open_command_overview_for_dump() ||
                    !field_scene.seek_command_overview_page(4U)) {
                    std::cerr << "field Cyberware dump: command route did "
                                 "not reach page 4\n";
                    return 3;
                }
                // The converted command loop dispatches A on page 4 to the
                // ROM 0x11950 Cyberware view.
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.cyberware_menu_open()) {
                    std::cerr << "field Cyberware dump: A did not open the "
                                 "Cyberware view\n";
                    return 3;
                }

                const auto flags =
                    field_scene.active_runner_record().cyberware_flags;
                std::vector<std::uint8_t> expected_rows;
                for (std::uint8_t bit = 0U; bit < 15U; ++bit) {
                    if ((flags & (std::uint16_t{1U} << bit)) != 0U) {
                        expected_rows.push_back(bit);
                    }
                }
                const auto& cyberware = field_scene.cyberware_menu_model();
                if (cyberware.shows_no_cyberware_message != (flags == 0U) ||
                    cyberware.installed_row_indices != expected_rows) {
                    std::cerr << "field Cyberware dump: +0x90 rows do not "
                                 "match the active record\n";
                    return 3;
                }

                QImage cyberware_scene(kSceneWidth, kSceneHeight,
                                       QImage::Format_RGB32);
                cyberware_scene.fill(Qt::black);
                QPainter cyberware_painter(&cyberware_scene);
                field_scene.paint(cyberware_painter);
                cyberware_painter.end();
                const auto has_ink = [&cyberware_scene](
                                         const int left, const int top,
                                         const int right, const int bottom) {
                    for (int y = top; y < bottom; ++y) {
                        for (int x = left; x < right; ++x) {
                            if ((cyberware_scene.pixel(x, y) & 0x00FFFFFFU) !=
                                0U) {
                                return true;
                            }
                        }
                    }
                    return false;
                };
                if (!has_ink(136, 24, 256, 32) ||
                    !has_ink(136, 32, 256, 40) ||
                    (cyberware.shows_no_cyberware_message &&
                     !has_ink(160, 48, 208, 56)) ||
                    (!cyberware.shows_no_cyberware_message &&
                     !expected_rows.empty() &&
                     !has_ink(112, 48, 264, 56))) {
                    std::cerr << "field Cyberware dump: source text is not "
                                 "visible at its traced destination\n";
                    return 3;
                }
                const auto parent = field_cyberware_dump_path->parent_path();
                if (!parent.empty()) {
                    std::filesystem::create_directories(parent);
                }
                if (!cyberware_scene.save(QString::fromStdWString(
                        field_cyberware_dump_path->wstring()))) {
                    std::cerr << "field Cyberware dump: could not write "
                              << field_cyberware_dump_path->string() << '\n';
                    return 3;
                }

                // ROM 0x11950 waits for one event before restoring the shared
                // row. A directional event must close only this subview.
                field_scene.set_controller_port_byte(0x04U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.cyberware_menu_open() ||
                    !field_scene.command_overview_open() ||
                    field_scene.command_overview_view().source_state
                            .primary_page != 4U) {
                    std::cerr << "field Cyberware dump: return event did not "
                                 "restore command page 4\n";
                    return 3;
                }
                std::cout << "field_cyberware_dump=written path="
                          << field_cyberware_dump_path->string()
                          << " flags=0x" << std::hex << flags << std::dec
                          << " visible_rows=" << expected_rows.size() << '\n';
                return 0;
            }
            if (field_secretary_dump_path.has_value()) {
                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field secretary dump: command overview "
                                 "did not open\n";
                    return 3;
                }
                if (!field_scene.seek_command_overview_page(6U)) {
                    std::cerr << "field secretary dump: could not reach "
                                 "page 6\n";
                    return 3;
                }
                // Press A on the Pocket Secretary tab: port bit 6 is the
                // A button in the sampled controller word (0x4000 in the
                // expanded events consumed by the command loop).
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.pocket_secretary_open()) {
                    std::cerr << "field secretary dump: Pocket Secretary "
                                 "did not open\n";
                    return 3;
                }
                // These two named QA candidates correspond to the US ROM's
                // recruited templates at 0x72C1A + 0x100*n. Ricky's +0x3E,
                // +0x2C, +0x50 and +0x54 are 3, 0x2000, 467674 and 20;
                // Winston Marrs' are 4, 0x4000, 467834 and 4. The test feeds
                // the same typed live-record fields that recruitment must
                // install; it does not link the NativePort asset decoder.
                RunnerRecordFields ricky_record{};
                ricky_record.record_type = 3U;
                ricky_record.command_overview_panel_parameter = 0x2000U;
                ricky_record.hud_portrait_table_pointer = 467674U;
                ricky_record.hud_portrait_table_offset = 20U;
                field_scene.install_party_member_for_dump(
                    1U, ricky_record, "Ricky");
                RunnerRecordFields winston_record{};
                winston_record.record_type = 4U;
                winston_record.command_overview_panel_parameter = 0x4000U;
                winston_record.hud_portrait_table_pointer = 467834U;
                winston_record.hud_portrait_table_offset = 4U;
                field_scene.install_party_member_for_dump(
                    2U, winston_record, "Winston Marrs");
                std::filesystem::create_directories(
                    *field_secretary_dump_path);
                const auto render_secretary_scene = [&field_scene] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto preview_matches_asset =
                    [&smoke_state](const QImage& scene,
                                   const std::uint32_t table_pointer,
                                   const std::uint16_t table_offset,
                                   const std::size_t palette_line) {
                        const auto alias =
                            "menu.field.runner.preview.table." +
                            std::to_string(table_pointer) + ".offset." +
                            std::to_string(table_offset) + ".palette." +
                            std::to_string(palette_line);
                        const auto* resource =
                            smoke_state.asset_catalog.catalog.find(alias);
                        if (resource == nullptr) {
                            return false;
                        }
                        QImage expected(QString::fromStdWString(
                            resource->absolute_path.wstring()));
                        if (expected.isNull() || expected.width() != 32 ||
                            expected.height() != 32) {
                            return false;
                        }
                        return scene.copy(QRect(120, 144, 32, 32)) ==
                               expected.convertToFormat(
                                   QImage::Format_RGB32);
                    };
                const auto save_secretary_scene =
                    [&](const std::string_view name, const QImage& scene) {
                        return scene.save(QString::fromStdWString(
                            ((*field_secretary_dump_path) /
                             (std::string(name) + ".bmp"))
                                .wstring()));
                    };
                const auto notebook_tips_dump_directory =
                    *field_secretary_dump_path / "notebook-tips";
                const auto save_notebook_tips_scene =
                    [&notebook_tips_dump_directory](
                        const std::string_view name, const QImage& scene) {
                        std::filesystem::create_directories(
                            notebook_tips_dump_directory);
                        return scene.save(QString::fromStdWString(
                            (notebook_tips_dump_directory /
                             (std::string(name) + ".bmp"))
                                .wstring()));
                    };
                auto secretary_scene = render_secretary_scene();
                if (!image_has_green_ink(secretary_scene, 88, 32, 80, 8) ||
                    !image_has_green_ink(secretary_scene, 88, 40, 96, 8)) {
                    std::cerr << "field secretary dump: source title or "
                                 "separator text is not visible\n";
                    return 3;
                }
                for (std::size_t row = 0;
                     row < kPocketSecretaryOptionCount; ++row) {
                    if (!image_has_green_ink(
                            secretary_scene, 88,
                            56 + static_cast<int>(row) * 16,
                            144, 8)) {
                        std::cerr << "field secretary dump: source option "
                                  << row << " has no visible text\n";
                        return 3;
                    }
                }
                if (field_scene.pocket_secretary_selection() != 0U ||
                    !save_secretary_scene("actual", secretary_scene)) {
                    std::cerr << "field secretary dump: initial selection "
                                 "or actual.bmp capture failed\n";
                    return 3;
                }
                auto previous_secretary_scene = secretary_scene;
                for (std::uint8_t selection = 1U; selection <= 4U;
                     ++selection) {
                    field_scene.set_controller_port_byte(0x02U); // Down.
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                    if (field_scene.pocket_secretary_selection() !=
                        selection) {
                        std::cerr << "field secretary dump: Down did not "
                                     "select source row "
                                  << static_cast<int>(selection) << '\n';
                        return 3;
                    }
                    secretary_scene = render_secretary_scene();
                    const auto previous_row_y =
                        56 + static_cast<int>(selection - 1U) * 16;
                    const auto current_row_y =
                        56 + static_cast<int>(selection) * 16;
                    if (!image_region_differs(
                            previous_secretary_scene, secretary_scene,
                            136, previous_row_y, 8, 8) ||
                        !image_region_differs(
                            previous_secretary_scene, secretary_scene,
                            136, current_row_y, 8, 8) ||
                        !save_secretary_scene(
                            "selection_" + std::to_string(selection),
                            secretary_scene)) {
                        std::cerr << "field secretary dump: source cursor "
                                     "did not follow typed selection "
                                  << static_cast<int>(selection) << '\n';
                        return 3;
                    }
                    previous_secretary_scene = secretary_scene;
                }
                field_scene.set_controller_port_byte(0x02U); // Down wraps.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.pocket_secretary_selection() != 0U) {
                    std::cerr << "field secretary dump: Down did not wrap "
                                 "from the fifth option to the first\n";
                    return 3;
                }
                secretary_scene = render_secretary_scene();
                if (!image_region_differs(previous_secretary_scene,
                                          secretary_scene, 136, 56, 8, 8) ||
                    !image_region_differs(previous_secretary_scene,
                                          secretary_scene, 136, 120, 8, 8) ||
                    !save_secretary_scene("wrap_to_0", secretary_scene)) {
                    std::cerr << "field secretary dump: wraparound cursor "
                                 "frame failed\n";
                    return 3;
                }

                const auto press_secretary_button =
                    [&field_scene](const std::uint8_t port_byte) {
                        field_scene.set_controller_port_byte(port_byte);
                        field_scene.advance_frame();
                        field_scene.set_controller_port_byte(0U);
                        field_scene.advance_frame();
                    };
                press_secretary_button(0x02U);
                if (field_scene.pocket_secretary_selection() != 1U) {
                    std::cerr << "field secretary dump: could not select "
                                 "Group Items\n";
                    return 3;
                }
                field_scene.set_group_item_flags_for_dump(
                    (std::uint32_t{1} << 14U) | (std::uint32_t{1} << 1U));
                press_secretary_button(0x40U);
                if (!field_scene.pocket_secretary_open() ||
                    !field_scene.pocket_secretary_group_items_waiting()) {
                    std::cerr << "field secretary dump: populated Group Items "
                                 "handler did not enter its input wait\n";
                    return 3;
                }
                auto group_items_scene = render_secretary_scene();
                if (!image_has_green_ink(group_items_scene, 32, 32, 240, 8) ||
                    !image_has_green_ink(group_items_scene, 32, 48, 240, 8) ||
                    !save_secretary_scene("group-items-populated",
                                          group_items_scene)) {
                    std::cerr << "field secretary dump: FFFC22 bit order or "
                                 "source item rows are not visible\n";
                    return 3;
                }
                const auto held_populated_group_items_scene =
                    group_items_scene;
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                group_items_scene = render_secretary_scene();
                if (image_region_differs(held_populated_group_items_scene,
                                         group_items_scene, 0, 0,
                                         240, 144) ||
                    !image_region_differs(held_populated_group_items_scene,
                                          group_items_scene, 32, 200,
                                          224, 16)) {
                    std::cerr << "field secretary dump: populated Group "
                                 "Items text or E660 chrome cadence failed\n";
                    return 3;
                }
                press_secretary_button(0x80U);
                if (!field_scene.pocket_secretary_open() ||
                    field_scene.pocket_secretary_group_items_waiting() ||
                    field_scene.pocket_secretary_selection() != 1U ||
                    !save_secretary_scene("group-items-populated-return",
                                          render_secretary_scene())) {
                    std::cerr << "field secretary dump: populated Group "
                                 "Items did not return to its selected row\n";
                    return 3;
                }

                field_scene.set_group_item_flags_for_dump(0U);
                press_secretary_button(0x40U);
                if (!field_scene.pocket_secretary_open() ||
                    !field_scene.pocket_secretary_group_items_waiting()) {
                    std::cerr << "field secretary dump: empty Group Items "
                                 "handler did not enter its input wait\n";
                    return 3;
                }
                group_items_scene = render_secretary_scene();
                if (!image_has_green_ink(group_items_scene, 56, 80, 208, 8) ||
                    !image_has_green_ink(group_items_scene, 56, 88, 208, 8) ||
                    image_has_green_ink(group_items_scene, 88, 32, 80, 8) ||
                    image_has_green_ink(group_items_scene, 88, 56, 144, 8) ||
                    !save_secretary_scene("group-items-empty",
                                          group_items_scene)) {
                    std::cerr << "field secretary dump: source empty-catalog "
                                 "text or cleared parent plane is wrong\n";
                    return 3;
                }
                const auto held_group_items_scene = group_items_scene;
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                group_items_scene = render_secretary_scene();
                if (image_region_differs(held_group_items_scene,
                                         group_items_scene, 24, 24,
                                         240, 144) ||
                    !image_region_differs(held_group_items_scene,
                                          group_items_scene, 32, 200,
                                          224, 16) ||
                    !save_secretary_scene("group-items-held",
                                          group_items_scene)) {
                    std::cerr << "field secretary dump: Group Items content "
                                 "or E660 chrome cadence failed\n";
                    return 3;
                }
                press_secretary_button(0x80U);
                if (!field_scene.pocket_secretary_open() ||
                    field_scene.pocket_secretary_group_items_waiting() ||
                    field_scene.pocket_secretary_selection() != 1U) {
                    std::cerr << "field secretary dump: Group Items did not "
                                 "return to its selected parent row\n";
                    return 3;
                }
                const auto group_items_return_scene = render_secretary_scene();
                if (!image_region_differs(held_group_items_scene,
                                          group_items_return_scene, 56, 80,
                                          208, 16) ||
                    !save_secretary_scene("group-items-return",
                                          group_items_return_scene)) {
                    std::cerr << "field secretary dump: Group Items response "
                                 "did not clear on return\n";
                    return 3;
                }

                press_secretary_button(0x02U);
                if (field_scene.pocket_secretary_selection() != 2U) {
                    std::cerr << "field secretary dump: could not select "
                                 "Cyberdeck\n";
                    return 3;
                }
                press_secretary_button(0x40U); // A opens the source handler.
                if (!field_scene.pocket_secretary_open() ||
                    !field_scene
                         .pocket_secretary_cyberdeck_unavailable_waiting()) {
                    std::cerr << "field secretary dump: new-game Cyberdeck "
                                 "handler did not enter its no-deck response\n";
                    return 3;
                }
                auto cyberdeck_scene = render_secretary_scene();
                for (std::size_t line = 0; line < 3U; ++line) {
                    if (!image_has_green_ink(
                            cyberdeck_scene, 56,
                            64 + static_cast<int>(line) * 8, 240, 8)) {
                        std::cerr << "field secretary dump: Cyberdeck source "
                                     "response line "
                                  << line << " has no visible text\n";
                        return 3;
                    }
                }
                if (image_has_green_ink(cyberdeck_scene, 88, 32, 80, 8)) {
                    std::cerr << "field secretary dump: C3E6 response screen "
                                 "left parent title text visible\n";
                    return 3;
                }
                if (!save_secretary_scene("cyberdeck_unavailable",
                                          cyberdeck_scene)) {
                    std::cerr << "field secretary dump: could not save "
                                 "Cyberdeck response frame\n";
                    return 3;
                }
                const auto held_cyberdeck_scene = cyberdeck_scene;
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                cyberdeck_scene = render_secretary_scene();
                if (image_region_differs(held_cyberdeck_scene,
                                         cyberdeck_scene, 56, 64,
                                         240, 32) ||
                    !image_region_differs(held_cyberdeck_scene,
                                          cyberdeck_scene, 32, 200,
                                          224, 16)) {
                    std::cerr << "field secretary dump: no-deck feedback "
                                 "content or E660 chrome cadence failed\n";
                    return 3;
                }
                press_secretary_button(0x80U); // Start is an input edge.
                if (!field_scene.pocket_secretary_open() ||
                    field_scene
                        .pocket_secretary_cyberdeck_unavailable_waiting() ||
                    field_scene.pocket_secretary_selection() != 2U) {
                    std::cerr << "field secretary dump: Start did not return "
                                 "to the selected parent row\n";
                    return 3;
                }
                cyberdeck_scene = render_secretary_scene();
                if (!image_region_differs(held_cyberdeck_scene,
                                          cyberdeck_scene, 56, 64, 240, 24) ||
                    !save_secretary_scene("cyberdeck_return",
                                          cyberdeck_scene)) {
                    std::cerr << "field secretary dump: response did not "
                                 "clear on return to Pocket Secretary\n";
                    return 3;
                }

                press_secretary_button(0x01U); // Up to Notebook.
                press_secretary_button(0x01U);
                if (field_scene.pocket_secretary_selection() != 0U) {
                    std::cerr << "field secretary dump: could not return to "
                                 "the Notebook row\n";
                    return 3;
                }
                press_secretary_button(0x40U);
                if (!field_scene.pocket_secretary_notebook_open() ||
                    field_scene.pocket_secretary_notebook_option_count() !=
                        3U) {
                    std::cerr << "field secretary dump: Notebook did not "
                                 "build the two base rows and New Game Contacts "
                                 "row\n";
                    return 3;
                }
                auto notebook_scene = render_secretary_scene();
                if (!image_has_green_ink(notebook_scene, 88, 32, 96, 8) ||
                    !image_has_green_ink(notebook_scene, 72, 40, 128, 8) ||
                    !image_has_green_ink(notebook_scene, 88, 56, 160, 8) ||
                    !image_has_green_ink(notebook_scene, 88, 72, 160, 8) ||
                    !image_has_green_ink(notebook_scene, 88, 88, 160, 8) ||
                    !image_has_green_ink(notebook_scene, 72, 56, 8, 8) ||
                    !save_secretary_scene("notebook-menu", notebook_scene)) {
                    std::cerr << "field secretary dump: Notebook source "
                                 "coordinates or selected state are not visible\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x02U); // Down.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.pocket_secretary_notebook_selection() != 1U) {
                    std::cerr << "field secretary dump: Notebook Down did "
                                 "not select Tips & Clues\n";
                    return 3;
                }
                const auto notebook_tips_selected = render_secretary_scene();
                if (!image_region_differs(notebook_scene,
                                          notebook_tips_selected, 72, 56,
                                          8, 32) ||
                    !save_secretary_scene("notebook-selection-1",
                                          notebook_tips_selected)) {
                    std::cerr << "field secretary dump: Notebook cursor did "
                                 "not follow its typed row selection\n";
                    return 3;
                }
                field_audio_events.clear();
                press_secretary_button(0x40U); // Open Tips & Clues.
                if (!field_scene.pocket_secretary_notebook_tips_open() ||
                    field_scene.pocket_secretary_notebook_visible_tip_count() !=
                        1U ||
                    field_scene
                            .pocket_secretary_notebook_visible_tip_source_index(
                                0U) != 0U ||
                    !field_audio_events.empty()) {
                    std::cerr << "field secretary dump: the fresh-game "
                                 "tip seed or silent open is incorrect\n";
                    return 3;
                }
                auto notebook_tips_scene = render_secretary_scene();
                if (!image_has_green_ink(notebook_tips_scene, 88, 32,
                                         104, 8) ||
                    !image_has_green_ink(notebook_tips_scene, 72, 40,
                                         136, 8) ||
                    !image_has_green_ink(notebook_tips_scene, 136, 184,
                                         80, 8) ||
                    !image_has_green_ink(notebook_tips_scene, 40, 56,
                                         192, 8) ||
                    !image_has_green_ink(notebook_tips_scene, 184, 184,
                                         8, 8) ||
                    !image_has_green_ink(notebook_tips_scene, 232, 184,
                                         8, 8) ||
                    image_has_green_ink(notebook_tips_scene, 72, 72, 8, 8) ||
                    image_has_green_ink(notebook_tips_scene, 88, 72, 96, 8) ||
                    !save_notebook_tips_scene("actual",
                                              notebook_tips_scene) ||
                    !save_secretary_scene("notebook-tips-actual",
                                          notebook_tips_scene)) {
                    std::cerr << "field secretary dump: Tips title, page "
                                 "counter, first page, or stale parent-row "
                                 "cleanup is wrong\n";
                    return 3;
                }
                press_secretary_button(0x40U); // A returns to Notebook.
                auto notebook_after_tips = render_secretary_scene();
                if (!field_scene.pocket_secretary_notebook_open() ||
                    field_scene.pocket_secretary_notebook_tips_open() ||
                    field_scene.pocket_secretary_notebook_selection() != 1U ||
                    image_has_green_ink(notebook_after_tips, 40, 56, 40, 16) ||
                    image_has_green_ink(notebook_after_tips, 136, 184,
                                        104, 8) ||
                    !save_notebook_tips_scene("return-a",
                                              notebook_after_tips) ||
                    !save_secretary_scene("notebook-tips-return-a",
                                          notebook_after_tips)) {
                    std::cerr << "field secretary dump: A return did not "
                                 "rebuild Notebook and clear the old tip "
                                 "text/counter\n";
                    return 3;
                }

                // Seed three additional source event bits to exercise the
                // compact LSB-first page list and both clamp boundaries.
                if (!field_scene.set_notebook_tip_unlocked(2U, true) ||
                    !field_scene.set_notebook_tip_unlocked(33U, true) ||
                    !field_scene.set_notebook_tip_unlocked(95U, true)) {
                    std::cerr << "field secretary dump: could not set "
                                 "typed Tips event bits\n";
                    return 3;
                }
                press_secretary_button(0x40U); // Reopen with four pages.
                if (!field_scene.pocket_secretary_notebook_tips_open() ||
                    field_scene.pocket_secretary_notebook_visible_tip_count() !=
                        4U ||
                    field_scene
                            .pocket_secretary_notebook_visible_tip_source_index(
                                0U) != 0U ||
                    field_scene
                            .pocket_secretary_notebook_visible_tip_source_index(
                                1U) != 2U ||
                    field_scene
                            .pocket_secretary_notebook_visible_tip_source_index(
                                2U) != 33U ||
                    field_scene
                            .pocket_secretary_notebook_visible_tip_source_index(
                                3U) != 95U) {
                    std::cerr << "field secretary dump: FFFC30/34/38 bits "
                                 "did not produce the source page order\n";
                    return 3;
                }
                const auto first_tip_page = render_secretary_scene();
                field_scene.set_controller_port_byte(0x08U); // Right.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.pocket_secretary_notebook_tips_selection() !=
                        1U ||
                    field_scene
                            .pocket_secretary_notebook_visible_tip_source_index(
                                1U) != 2U) {
                    std::cerr << "field secretary dump: Right did not "
                                 "select the next unlocked page\n";
                    return 3;
                }
                auto second_tip_page = render_secretary_scene();
                if (!image_region_differs(first_tip_page, second_tip_page,
                                          40, 56, 208, 128) ||
                    !image_region_differs(first_tip_page, second_tip_page,
                                          176, 184, 16, 8) ||
                    !save_notebook_tips_scene("page-2", second_tip_page) ||
                    !save_secretary_scene("notebook-tips-page-2",
                                          second_tip_page)) {
                    std::cerr << "field secretary dump: page text or "
                                 "current-page number did not update\n";
                    return 3;
                }
                press_secretary_button(0x08U);
                press_secretary_button(0x08U);
                press_secretary_button(0x08U); // Clamp on page four.
                if (field_scene.pocket_secretary_notebook_tips_selection() !=
                        3U ||
                    field_scene
                            .pocket_secretary_notebook_visible_tip_source_index(
                                3U) != 95U) {
                    std::cerr << "field secretary dump: Right did not clamp "
                                 "at the final unlocked page\n";
                    return 3;
                }
                press_secretary_button(0x08U);
                if (field_scene.pocket_secretary_notebook_tips_selection() !=
                    3U) {
                    std::cerr << "field secretary dump: final-page clamp "
                                 "was lost on continued Right input\n";
                    return 3;
                }
                field_audio_events.clear();
                press_secretary_button(0x20U); // C invokes mode action.
                if (!field_scene.pocket_secretary_notebook_tips_open() ||
                    field_scene.pocket_secretary_notebook_tips_selection() !=
                        3U ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x25U}) {
                    std::cerr << "field secretary dump: C did not keep the "
                                 "selected page and request source event "
                                 "0x25\n";
                    return 3;
                }
                auto last_tip_page = render_secretary_scene();
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                const auto held_last_tip_page = render_secretary_scene();
                if (image_region_differs(last_tip_page, held_last_tip_page,
                                         32, 32, 256, 160) ||
                    !image_region_differs(last_tip_page, held_last_tip_page,
                                          32, 200, 224, 16) ||
                    !save_notebook_tips_scene("held-30-frames",
                                              held_last_tip_page) ||
                    !save_secretary_scene("notebook-tips-held",
                                          held_last_tip_page)) {
                    std::cerr << "field secretary dump: held page text "
                                 "changed or E660 lower chrome failed to "
                                 "advance\n";
                    return 3;
                }
                field_audio_events.clear();
                press_secretary_button(0x80U); // Start returns to Notebook.
                if (!field_scene.pocket_secretary_notebook_open() ||
                    field_scene.pocket_secretary_notebook_tips_open() ||
                    !field_audio_events.empty() ||
                    !save_notebook_tips_scene(
                        "return-start", render_secretary_scene()) ||
                    !save_secretary_scene("notebook-tips-return-start",
                                          render_secretary_scene())) {
                    std::cerr << "field secretary dump: Start return did "
                                 "not rebuild Notebook cleanly\n";
                    return 3;
                }
                press_secretary_button(0x40U); // Reopen Tips.
                field_audio_events.clear();
                press_secretary_button(0x10U); // B returns immediately.
                if (field_scene.pocket_secretary_notebook_tips_open() ||
                    !field_scene.pocket_secretary_notebook_open() ||
                    !field_audio_events.empty() ||
                    !save_notebook_tips_scene(
                        "return-b", render_secretary_scene()) ||
                    !save_secretary_scene("notebook-tips-return-b",
                                          render_secretary_scene())) {
                    std::cerr << "field secretary dump: B did not return "
                                 "to the clean Notebook panel silently\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x01U); // Up.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0x40U); // Current Run.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.pocket_secretary_notebook_no_run_waiting()) {
                    std::cerr << "field secretary dump: Current Run did not "
                                 "enter its no-run response wait\n";
                    return 3;
                }
                auto notebook_no_run_scene = render_secretary_scene();
                for (std::size_t line = 0; line < 3U; ++line) {
                    if (!image_has_green_ink(
                            notebook_no_run_scene, 48,
                            48 + static_cast<int>(line) * 8, 240, 8)) {
                        std::cerr << "field secretary dump: no-run source "
                                     "line "
                                  << line << " has no visible text\n";
                        return 3;
                    }
                }
                if (image_has_green_ink(notebook_no_run_scene, 88, 32,
                                        96, 8) ||
                    image_has_green_ink(notebook_no_run_scene, 88, 72,
                                        160, 8) ||
                    image_has_green_ink(notebook_no_run_scene, 88, 88,
                                        160, 8) ||
                    image_has_green_ink(notebook_no_run_scene, 88, 104,
                                        160, 8) ||
                    image_has_green_ink(notebook_no_run_scene, 88, 120,
                                        160, 8) ||
                    !save_secretary_scene("notebook-current-run-unavailable",
                                          notebook_no_run_scene)) {
                    std::cerr << "field secretary dump: no-run response "
                                 "retained stale Notebook rows or capture "
                                 "failed\n";
                    return 3;
                }
                const auto held_notebook_no_run_scene =
                    notebook_no_run_scene;
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                notebook_no_run_scene = render_secretary_scene();
                if (!image_has_green_ink(notebook_no_run_scene, 48, 48,
                                         240, 8) ||
                    !field_scene.pocket_secretary_notebook_no_run_waiting() ||
                    image_region_differs(held_notebook_no_run_scene,
                                         notebook_no_run_scene,
                                         48, 48, 240, 24) ||
                    !image_region_differs(held_notebook_no_run_scene,
                                          notebook_no_run_scene,
                                          32, 200, 224, 16) ||
                    !save_secretary_scene("notebook-current-run-held",
                                          notebook_no_run_scene)) {
                    std::cerr << "field secretary dump: no-run response did "
                                 "not persist or E660 chrome did not animate\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x80U); // dismiss wait.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.pocket_secretary_notebook_open() ||
                    field_scene.pocket_secretary_notebook_no_run_waiting() ||
                    field_scene.pocket_secretary_notebook_selection() != 0U ||
                    !save_secretary_scene("notebook-current-run-return",
                                          render_secretary_scene())) {
                    std::cerr << "field secretary dump: no-run response did "
                                 "not rebuild Notebook at its source selection\n";
                    return 3;
                }
                notebook_scene = render_secretary_scene();
                if (image_has_green_ink(notebook_scene, 48, 48, 240, 8) ||
                    image_has_green_ink(notebook_scene, 48, 64, 240, 8)) {
                    std::cerr << "field secretary dump: rebuilt Notebook "
                                 "retained stale no-run response text\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x10U); // B to parent.
                field_scene.advance_frame();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (field_scene.pocket_secretary_notebook_open() ||
                    !field_scene.pocket_secretary_open() ||
                    field_scene.pocket_secretary_selection() != 0U ||
                    !save_secretary_scene("notebook-return",
                                          render_secretary_scene())) {
                    std::cerr << "field secretary dump: B did not return to "
                                 "the selected Pocket Secretary parent row\n";
                    return 3;
                }
                for (int step = 0; step < 3; ++step) {
                    press_secretary_button(0x02U); // Down to Dismiss Runner.
                }
                if (field_scene.pocket_secretary_selection() != 3U) {
                    std::cerr << "field secretary dump: could not select "
                                 "Dismiss Runner\n";
                    return 3;
                }
                const auto dismiss_parent_scene = render_secretary_scene();
                if (!save_secretary_scene("dismiss-runner-parent-before",
                                          dismiss_parent_scene)) {
                    std::cerr << "field secretary dump: could not save "
                                 "pre-entry parent panel\n";
                    return 3;
                }
                const auto dismiss_parent_panel_matches =
                    [&dismiss_parent_scene](const QImage& scene) {
                        // The outer frame and blank inner grid are shared by
                        // these source screens. Compare the full content
                        // panel after return so no child labels, selector, or
                        // graphics can survive beneath the parent buttons.
                        return !image_region_differs(
                            dismiss_parent_scene, scene, 24, 24, 216, 168);
                    };
                field_audio_events.clear();
                press_secretary_button(0x40U);
                if (!field_scene.pocket_secretary_dismiss_runner_open() ||
                    field_scene.dismiss_runner_selection() != 0U ||
                    field_scene.dismiss_runner_option_count() != 3U ||
                    !field_audio_events.empty()) {
                    std::cerr << "field secretary dump: Dismiss Runner "
                                 "entry state or source audio event is wrong "
                              << "open="
                              << field_scene
                                     .pocket_secretary_dismiss_runner_open()
                              << " selection="
                              << static_cast<int>(
                                     field_scene.dismiss_runner_selection())
                              << " options="
                              << static_cast<int>(
                                     field_scene.dismiss_runner_option_count())
                              << " audio=";
                    for (const auto event : field_audio_events) {
                        std::cerr << std::hex << event << ' ' << std::dec;
                    }
                    std::cerr << '\n';
                    return 3;
                }
                auto dismiss_scene = render_secretary_scene();
                const auto has_parent_option = [](const QImage& scene,
                                                  int row) {
                    return image_has_green_ink(
                        scene, 88, 56 + row * 16, 144, 8);
                };
                if (!image_has_green_ink(dismiss_scene, 88, 32, 112, 8) ||
                    !image_has_green_ink(dismiss_scene, 88, 40, 112, 8) ||
                    !image_has_green_ink(dismiss_scene, 72, 48, 160, 8) ||
                    !image_has_green_ink(dismiss_scene, 88, 64, 112, 8) ||
                    !image_has_green_ink(dismiss_scene, 88, 80, 112, 8) ||
                    !image_has_green_ink(dismiss_scene, 88, 96, 112, 8) ||
                    image_has_green_ink(dismiss_scene, 72, 64, 8, 8) == false ||
                    image_has_green_ink(dismiss_scene, 136, 104, 8, 8) ||
                    !image_region_differs(dismiss_parent_scene, dismiss_scene,
                                          136, 96, 8, 16) ||
                    image_region_differs(dismiss_parent_scene, dismiss_scene,
                                         120, 144, 32, 32) ||
                    has_parent_option(dismiss_scene, 0) ||
                    has_parent_option(dismiss_scene, 1) ||
                    has_parent_option(dismiss_scene, 2) ||
                    has_parent_option(dismiss_scene, 3) ||
                    has_parent_option(dismiss_scene, 4) ||
                    !save_secretary_scene("dismiss-runner-actual",
                                          dismiss_scene)) {
                    std::cerr << "field secretary dump: Dismiss labels, "
                                 "fixed Abort row, or clear-panel order is "
                                 "wrong\n";
                    return 3;
                }

                const auto effect_frames_before_move =
                    field_scene.dismiss_runner_effect_frames();
                field_audio_events.clear();
                press_secretary_button(0x02U); // Select Ricky.
                if (field_scene.dismiss_runner_selection() != 1U ||
                    field_scene.dismiss_runner_effect_frames() !=
                        effect_frames_before_move + 2U ||
                    field_audio_events != std::vector<std::uint16_t>{0x25U}) {
                    std::cerr << "field secretary dump: Dismiss Runner "
                                 "movement cadence or event 0x25 is wrong\n";
                    return 3;
                }
                const auto dismiss_ricky_scene = render_secretary_scene();
                if (field_scene.dismiss_runner_preview_slot() !=
                        std::optional<std::size_t>{1U} ||
                    !preview_matches_asset(dismiss_ricky_scene,
                                           467674U, 20U, 1U) ||
                    !image_has_green_ink(dismiss_ricky_scene, 72, 80, 8, 8) ||
                    !image_region_differs(dismiss_scene, dismiss_ricky_scene,
                                          72, 64, 8, 32) ||
                    !image_region_differs(dismiss_scene, dismiss_ricky_scene,
                                          120, 144, 32, 32) ||
                    !save_secretary_scene("dismiss-runner-selection-1",
                                          dismiss_ricky_scene)) {
                    std::cerr << "field secretary dump: Dismiss cursor did "
                                 "not follow the selected runner\n";
                    return 3;
                }
                const auto effect_frames_before_second_move =
                    field_scene.dismiss_runner_effect_frames();
                field_audio_events.clear();
                press_secretary_button(0x02U); // Select Winston.
                const auto dismiss_winston_scene = render_secretary_scene();
                if (field_scene.dismiss_runner_selection() != 2U ||
                    field_scene.dismiss_runner_preview_slot() !=
                        std::optional<std::size_t>{2U} ||
                    !preview_matches_asset(dismiss_winston_scene,
                                           467834U, 4U, 2U) ||
                    field_scene.dismiss_runner_effect_frames() !=
                        effect_frames_before_second_move + 2U ||
                    field_audio_events != std::vector<std::uint16_t>{0x25U} ||
                    !image_region_differs(dismiss_ricky_scene,
                                          dismiss_winston_scene,
                                          120, 144, 32, 32) ||
                    !save_secretary_scene("dismiss-runner-selection-2",
                                          dismiss_winston_scene)) {
                    std::cerr << "field secretary dump: second candidate "
                                 "did not select its own HUD preview\n";
                    return 3;
                }
                const auto effect_frames_before_reverse_move =
                    field_scene.dismiss_runner_effect_frames();
                field_audio_events.clear();
                press_secretary_button(0x01U); // Return to Ricky.
                const auto dismiss_ricky_again_scene = render_secretary_scene();
                if (field_scene.dismiss_runner_selection() != 1U ||
                    field_scene.dismiss_runner_preview_slot() !=
                        std::optional<std::size_t>{1U} ||
                    !preview_matches_asset(dismiss_ricky_again_scene,
                                           467674U, 20U, 1U) ||
                    field_scene.dismiss_runner_effect_frames() !=
                        effect_frames_before_reverse_move + 2U ||
                    field_audio_events != std::vector<std::uint16_t>{0x25U} ||
                    image_region_differs(dismiss_ricky_scene,
                                         dismiss_ricky_again_scene,
                                         120, 144, 32, 32)) {
                    std::cerr << "field secretary dump: preview did not "
                                 "follow the cursor back to Ricky\n";
                    return 3;
                }
                const auto effect_frames_before_wait =
                    field_scene.dismiss_runner_effect_frames();
                auto previous_grid_scene = dismiss_ricky_again_scene;
                bool random_grid_pixels_changed = false;
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                    const auto current_scene = render_secretary_scene();
                    random_grid_pixels_changed =
                        random_grid_pixels_changed || image_region_differs(
                            previous_grid_scene, current_scene,
                            32, 200, 224, 16);
                    previous_grid_scene = current_scene;
                }
                dismiss_scene = previous_grid_scene;
                if (!field_scene.pocket_secretary_dismiss_runner_open() ||
                    field_scene.dismiss_runner_preview_slot() !=
                        std::optional<std::size_t>{1U} ||
                    field_scene.dismiss_runner_effect_frames() !=
                        effect_frames_before_wait + 30U ||
                    !random_grid_pixels_changed ||
                    !image_has_green_ink(dismiss_scene, 88, 80, 112, 8) ||
                    has_parent_option(dismiss_scene, 0) ||
                    has_parent_option(dismiss_scene, 1) ||
                    has_parent_option(dismiss_scene, 2) ||
                    has_parent_option(dismiss_scene, 3) ||
                    has_parent_option(dismiss_scene, 4) ||
                    image_region_differs(dismiss_ricky_again_scene,
                                         dismiss_scene, 24, 24, 216, 128) ||
                    image_region_differs(dismiss_ricky_again_scene,
                                         dismiss_scene, 120, 144, 32, 32) ||
                    !image_region_differs(dismiss_parent_scene, dismiss_scene,
                                          120, 144, 32, 32) ||
                    !save_secretary_scene("dismiss-runner-held",
                                          dismiss_scene)) {
                    std::cerr << "field secretary dump: Dismiss child did "
                                 "not hold state through 30 E660 frames\n";
                    return 3;
                }

                field_audio_events.clear();
                press_secretary_button(0x10U); // Source B confirms the row.
                if (field_scene.pocket_secretary_dismiss_runner_open() ||
                    !field_scene.pocket_secretary_open() ||
                    field_scene.pocket_secretary_selection() != 3U ||
                    field_scene.field_party_slot_active_for_dump(1U) ||
                    !field_scene.field_party_slot_active_for_dump(2U) ||
                    (field_scene.field_party_records_for_dump()[1].flags &
                     0x0100U) == 0U ||
                    field_audio_events != std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field secretary dump: selected runner did "
                                 "not clear only its compact party marker\n";
                    return 3;
                }
                auto dismiss_return_scene = render_secretary_scene();
                if (!has_parent_option(dismiss_return_scene, 3) ||
                    !dismiss_parent_panel_matches(dismiss_return_scene) ||
                    !save_secretary_scene("dismiss-runner-return",
                                          dismiss_return_scene)) {
                    std::cerr << "field secretary dump: parent redraw did not "
                                 "restore the complete panel without stale "
                                 "child controls\n";
                    return 3;
                }

                field_audio_events.clear();
                press_secretary_button(0x40U);
                if (!field_scene.pocket_secretary_dismiss_runner_open() ||
                    field_scene.dismiss_runner_option_count() != 2U) {
                    std::cerr << "field secretary dump: dismissed runner "
                                 "remained in the next candidate list\n";
                    return 3;
                }
                dismiss_scene = render_secretary_scene();
                if (!image_has_green_ink(dismiss_scene, 88, 64, 112, 8) ||
                    !image_has_green_ink(dismiss_scene, 88, 80, 112, 8) ||
                    image_has_green_ink(dismiss_scene, 88, 96, 112, 8) ||
                    !save_secretary_scene("dismiss-runner-one-candidate",
                                          dismiss_scene)) {
                    std::cerr << "field secretary dump: fixed Abort row moved "
                                 "when candidate count changed\n";
                    return 3;
                }
                field_audio_events.clear();
                press_secretary_button(0x40U); // A on Abort.
                if (field_scene.pocket_secretary_dismiss_runner_open() ||
                    !field_scene.field_party_slot_active_for_dump(2U) ||
                    field_audio_events != std::vector<std::uint16_t>{0x24U} ||
                    !dismiss_parent_panel_matches(render_secretary_scene())) {
                    std::cerr << "field secretary dump: Abort changed the "
                                 "remaining party record\n";
                    return 3;
                }

                field_audio_events.clear();
                press_secretary_button(0x40U);
                if (!field_scene.pocket_secretary_dismiss_runner_open()) {
                    std::cerr << "field secretary dump: could not reopen "
                                 "Dismiss Runner for Start return\n";
                    return 3;
                }
                field_audio_events.clear();
                press_secretary_button(0x80U); // Start returns to parent.
                if (field_scene.pocket_secretary_dismiss_runner_open() ||
                    field_audio_events.size() != 0U ||
                    !dismiss_parent_panel_matches(render_secretary_scene()) ||
                    !save_secretary_scene("dismiss-runner-start-return",
                                          render_secretary_scene())) {
                    std::cerr << "field secretary dump: Start did not return "
                                 "without an action sound\n";
                    return 3;
                }

                field_scene.set_active_party_slot_for_dump(2U);
                if (field_scene.field_active_party_slot_for_dump() != 2U) {
                    std::cerr << "field secretary dump: could not seed active "
                                 "runner for dismissal replacement\n";
                    return 3;
                }
                press_secretary_button(0x40U);
                field_audio_events.clear();
                press_secretary_button(0x02U); // Select current runner.
                if (field_audio_events !=
                    std::vector<std::uint16_t>{0x25U}) {
                    std::cerr << "field secretary dump: current-runner "
                                 "cursor did not request event 0x25\n";
                    return 3;
                }
                press_secretary_button(0x40U);
                if (field_scene.field_party_slot_active_for_dump(2U) ||
                    field_scene.field_active_party_slot_for_dump() != 0U ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x25U, 0x24U} ||
                    !dismiss_parent_panel_matches(render_secretary_scene()) ||
                    !save_secretary_scene(
                        "dismiss-runner-current-replaced",
                        render_secretary_scene())) {
                    std::cerr << "field secretary dump: removing the current "
                                 "runner did not switch to the next active "
                                 "record\n";
                    return 3;
                }
                std::cout << "dismiss_runner_review=fixed_abort,active_list,"
                             "compact_marker,active_runner_switch,e660_cadence "
                             "e660_pixels=pending "
                             "source_audio=0x25_resource_037\n";
                std::cout << "notebook_review=menu,current-run-no-run,wait,return "
                             "contacts_gate=visible source_effect=not_rendered\n";
                std::cout << "field_secretary_dump=written path="
                          << field_secretary_dump_path->string()
                          << " frames=21 selection="
                          << static_cast<int>(
                                 field_scene.pocket_secretary_selection())
                          << '\n';
                return 0;
            }
            if (field_posture_dump_path.has_value()) {
                const auto capture = [&field_scene] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto field_before_menu = capture();
                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field posture dump: command overview "
                                 "did not open\n";
                    return 3;
                }
                const auto frames_directory =
                    field_posture_dump_path->parent_path() /
                    (field_posture_dump_path->stem().string() + "-frames");
                std::filesystem::create_directories(frames_directory);
                const auto save_frame = [&frames_directory](
                                            const std::string& name,
                                            const QImage& scene) {
                    const auto path = frames_directory / name;
                    return scene.save(
                        QString::fromStdWString(path.wstring()));
                };
                const auto posture_failure = [](const std::string& reason) {
                    std::cerr << "field posture dump: " << reason << '\n';
                    return 3;
                };
                const auto press = [&field_scene](
                                       const std::uint8_t port_byte) {
                    field_scene.set_controller_port_byte(port_byte);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                const auto region_matches_asset = [](
                                                       const QImage& scene,
                                                       const QImage& asset,
                                                       const int x,
                                                       const int y) {
                    if (asset.isNull() || x < 0 || y < 0 ||
                        x + asset.width() > scene.width() ||
                        y + asset.height() > scene.height()) {
                        return false;
                    }
                    const auto actual = scene.copy(
                        x, y, asset.width(), asset.height())
                                            .convertToFormat(
                                                QImage::Format_RGB32);
                    const auto expected = asset.convertToFormat(
                        QImage::Format_RGB32);
                    return actual == expected;
                };
                const auto same_outside_focus_row = [](
                                                        const QImage& first,
                                                        const QImage& second) {
                    if (first.size() != second.size()) {
                        return false;
                    }
                    for (int y = 0; y < first.height(); ++y) {
                        for (int x = 0; x < first.width(); ++x) {
                            const bool focus_row = x >= 120 && x < 168 &&
                                                   y >= 168 && y < 176;
                            if (!focus_row &&
                                first.pixel(x, y) != second.pixel(x, y)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto posture_focus_asset = QImage(
                    QString::fromStdWString(
                        (asset_root / "menu" / "field" /
                         "posture-focus.bmp")
                            .wstring()));
                if (posture_focus_asset.size() != QSize(48, 8)) {
                    return posture_failure(
                        "named posture focus asset is missing or has an "
                        "unexpected size");
                }

                const auto baseline_posture =
                    field_scene.posture_dialog_value();
                const auto press_name = [&](const std::uint8_t page,
                                            const char* phase) {
                    return std::string("page-") + std::to_string(page) +
                           "-" + phase + ".bmp";
                };

                // The command overview opens on page 6. Navigate with real
                // Left edges to page 2, then exercise Up/Down on every page
                // whose command row is eligible for the posture focus.
                for (int page = 6; page > 2; --page) {
                    press(0x04U); // Left.
                }
                if (field_scene.command_overview_primary_page() != 2U) {
                    return posture_failure(
                        "could not navigate to command page 2");
                }
                const auto initial_combat_bars =
                    field_scene.command_overview_combat_bars();
                field_audio_events.clear();
                for (std::uint8_t page = 2U; page <= 6U; ++page) {
                    if (field_scene.command_overview_primary_page() != page) {
                        return posture_failure(
                            "command page changed outside its input edge");
                    }
                    const auto page_base = capture();
                    if (!save_frame(press_name(page, "before-up"),
                                    page_base)) {
                        return posture_failure(
                            "could not save a command page baseline");
                    }
                    field_audio_events.clear();
                    press(0x01U); // Up enters posture focus.
                    const auto focused = capture();
                    const auto focused_frame_saved = save_frame(
                        press_name(page, "up-focused"), focused);
                    const auto focus_asset_matches =
                        region_matches_asset(focused, posture_focus_asset,
                                             120, 168);
                    const auto page_outside_focus_matches =
                        same_outside_focus_row(page_base, focused);
                    if (!field_scene.posture_dialog_open() ||
                        field_scene.command_overview_primary_page() != page ||
                        field_scene.posture_dialog_value() != baseline_posture ||
                        !field_audio_events.empty() ||
                        !focus_asset_matches ||
                        image_region_differs(page_base, focused, 176, 168,
                                             64, 8) ||
                        !page_outside_focus_matches ||
                        !focused_frame_saved) {
                        std::cerr << "field posture page="
                                  << static_cast<unsigned>(page)
                                  << " open="
                                  << field_scene.posture_dialog_open()
                                  << " actual_page="
                                  << static_cast<unsigned>(
                                         field_scene.command_overview_primary_page())
                                  << " posture="
                                  << static_cast<unsigned>(
                                         field_scene.posture_dialog_value())
                                  << " baseline="
                                  << static_cast<unsigned>(baseline_posture)
                                  << " audio=" << field_audio_events.size()
                                  << " focus_asset=" << focus_asset_matches
                                  << " outside_focus="
                                  << page_outside_focus_matches
                                  << " saved=" << focused_frame_saved << '\n';
                        return posture_failure(
                            "Up failed to show only the posture focus row on "
                            "a command page");
                    }

                    if (page == 2U) {
                        field_audio_events.clear();
                        for (int frame = 0; frame < 30; ++frame) {
                            field_scene.advance_frame();
                        }
                        const auto held_focus = capture();
                        if (held_focus != focused ||
                            !field_audio_events.empty() ||
                            !save_frame("page-2-focus-held-30-frames.bmp",
                                        held_focus)) {
                            return posture_failure(
                                "focus changed or played audio during its "
                                "static 30-frame wait");
                        }

                        const auto before_right_bars =
                            field_scene.command_overview_combat_bars();
                        const auto first_delta_port =
                            baseline_posture < 4U ? 0x08U : 0x04U;
                        const auto second_delta_port =
                            baseline_posture < 4U ? 0x04U : 0x08U;
                        field_audio_events.clear();
                        press(first_delta_port);
                        const auto after_first_delta = capture();
                        const auto first_posture = static_cast<std::uint8_t>(
                            baseline_posture < 4U
                                ? baseline_posture + 1U
                                : baseline_posture - 1U);
                        const auto after_first_bars =
                            field_scene.command_overview_combat_bars();
                        if (!field_scene.posture_dialog_open() ||
                            field_scene.posture_dialog_value() !=
                                first_posture ||
                            after_first_bars.attack_value ==
                                before_right_bars.attack_value ||
                            after_first_bars.defense_value ==
                                before_right_bars.defense_value ||
                            !image_region_differs(focused, after_first_delta,
                                                  144, 104, 104, 8) ||
                            !image_region_differs(focused, after_first_delta,
                                                  144, 112, 104, 8) ||
                            field_audio_events !=
                                std::vector<std::uint16_t>{0x25U} ||
                            !save_frame("page-2-posture-adjusted.bmp",
                                        after_first_delta)) {
                            return posture_failure(
                                "a horizontal posture input failed to update "
                                "typed state, attack/defense bars, or event "
                                "0x25");
                        }

                        press(second_delta_port);
                        const auto restored_focus = capture();
                        if (field_scene.posture_dialog_value() !=
                                baseline_posture ||
                            field_scene.command_overview_combat_bars()
                                    .attack_value !=
                                initial_combat_bars.attack_value ||
                            field_scene.command_overview_combat_bars()
                                    .defense_value !=
                                initial_combat_bars.defense_value ||
                            restored_focus != focused ||
                            field_audio_events !=
                                std::vector<std::uint16_t>{0x25U, 0x25U} ||
                            !save_frame("page-2-posture-restored.bmp",
                                        restored_focus)) {
                            return posture_failure(
                                "the opposite horizontal input failed to "
                                "restore posture and combat pixels");
                        }

                        field_audio_events.clear();
                        std::size_t movement_edges = 0U;
                        for (int attempt = 0;
                             attempt < 5 &&
                             field_scene.posture_dialog_value() < 4U;
                             ++attempt) {
                            press(0x08U); // Right raises toward Full Offense.
                            ++movement_edges;
                        }
                        const auto max_posture_scene = capture();
                        const auto max_posture_bars =
                            field_scene.command_overview_combat_bars();
                        press(0x08U); // Clamp at 4; source still sounds.
                        ++movement_edges;
                        if (field_scene.posture_dialog_value() != 4U ||
                            field_scene.command_overview_combat_bars()
                                    .attack_value !=
                                max_posture_bars.attack_value ||
                            field_scene.command_overview_combat_bars()
                                    .defense_value !=
                                max_posture_bars.defense_value ||
                            !image_region_differs(
                                focused, max_posture_scene, 144, 104, 104, 8) ||
                            !image_region_differs(
                                focused, max_posture_scene, 144, 112, 104, 8) ||
                            !save_frame("page-2-posture-max-clamped.bmp",
                                        capture())) {
                            return posture_failure(
                                "posture did not clamp at Full Offense");
                        }
                        for (int attempt = 0;
                             attempt < 5 &&
                             field_scene.posture_dialog_value() > 0U;
                             ++attempt) {
                            press(0x04U); // Left lowers toward Full Defense.
                            ++movement_edges;
                        }
                        const auto min_posture_scene = capture();
                        const auto min_posture_bars =
                            field_scene.command_overview_combat_bars();
                        press(0x04U); // Clamp at 0; source still sounds.
                        ++movement_edges;
                        if (field_scene.posture_dialog_value() != 0U ||
                            field_scene.command_overview_combat_bars()
                                    .attack_value !=
                                min_posture_bars.attack_value ||
                            field_scene.command_overview_combat_bars()
                                    .defense_value !=
                                min_posture_bars.defense_value ||
                            !image_region_differs(
                                focused, min_posture_scene, 144, 104, 104, 8) ||
                            !image_region_differs(
                                focused, min_posture_scene, 144, 112, 104, 8) ||
                            !save_frame("page-2-posture-min-clamped.bmp",
                                        capture())) {
                            return posture_failure(
                                "posture did not clamp at Full Defense");
                        }
                        for (int attempt = 0;
                             attempt < 5 &&
                             field_scene.posture_dialog_value() <
                                 baseline_posture;
                             ++attempt) {
                            press(0x08U);
                            ++movement_edges;
                        }
                        if (field_scene.posture_dialog_value() !=
                                baseline_posture ||
                            field_scene.command_overview_combat_bars()
                                    .attack_value !=
                                initial_combat_bars.attack_value ||
                            field_scene.command_overview_combat_bars()
                                    .defense_value !=
                                initial_combat_bars.defense_value ||
                            field_audio_events.size() != movement_edges ||
                            !std::all_of(field_audio_events.begin(),
                                         field_audio_events.end(),
                                         [](const std::uint16_t event) {
                                             return event == 0x25U;
                                         }) ||
                            !image_region_differs(
                                focused, max_posture_scene, 136, 176, 104, 8) ||
                            !image_region_differs(
                                focused, min_posture_scene, 136, 176, 104, 8)) {
                            return posture_failure(
                                "five posture values, endpoint bars, or the "
                                "per-step audio events did not follow state");
                        }
                    }

                    if (page == 2U) {
                        // The source loop still honors a fresh horizontal
                        // edge when Down arrives with that direction held.
                        const bool increase = baseline_posture < 4U;
                        const auto diagonal_port = static_cast<std::uint8_t>(
                            increase ? 0x0AU : 0x06U); // Down + Right/Left.
                        const auto restore_port = static_cast<std::uint8_t>(
                            increase ? 0x04U : 0x08U);
                        const auto expected_posture = static_cast<std::uint8_t>(
                            increase ? baseline_posture + 1U
                                     : baseline_posture - 1U);
                        field_audio_events.clear();
                        press(diagonal_port);
                        const auto diagonal_focus = capture();
                        if (!field_scene.posture_dialog_open() ||
                            field_scene.posture_dialog_value() !=
                                expected_posture ||
                            field_audio_events !=
                                std::vector<std::uint16_t>{0x25U} ||
                            !image_region_differs(focused, diagonal_focus,
                                                  144, 104, 104, 8) ||
                            !image_region_differs(focused, diagonal_focus,
                                                  144, 112, 104, 8) ||
                            !save_frame("page-2-down-plus-horizontal.bmp",
                                        diagonal_focus)) {
                            return posture_failure(
                                "Down plus a fresh horizontal edge did not "
                                "preserve the source posture adjustment");
                        }
                        press(restore_port);
                        if (field_scene.posture_dialog_value() !=
                                baseline_posture ||
                            capture() != focused ||
                            field_audio_events !=
                                std::vector<std::uint16_t>{0x25U, 0x25U}) {
                            return posture_failure(
                                "the diagonal posture adjustment did not "
                                "return cleanly to its prior value");
                        }
                    }

                    field_audio_events.clear();
                    press(0x02U); // Down returns to the command row.
                    const auto deselected = capture();
                    if (field_scene.posture_dialog_open() ||
                        field_scene.command_overview_primary_page() != page ||
                        field_scene.posture_dialog_value() != baseline_posture ||
                        !field_audio_events.empty() ||
                        deselected != page_base ||
                        !save_frame(press_name(page, "down-deselected"),
                                    deselected)) {
                        return posture_failure(
                            "Down failed to deselect focus and restore the "
                            "exact command-page composition");
                    }
                    if (page < 6U) {
                        press(0x08U); // Right selects the next command row.
                    }
                }

                const auto last_page_before_down = capture();
                press(0x02U); // Down on a regular row must not open focus.
                if (field_scene.posture_dialog_open() ||
                    field_scene.command_overview_primary_page() != 6U ||
                    capture() != last_page_before_down) {
                    return posture_failure(
                        "Down on an unfocused command row altered its page");
                }
                for (int page = 6; page > 2; --page) {
                    press(0x04U);
                }
                press(0x01U); // Leave the final artifact in a real focus state.
                const auto final_focus = capture();
                if (!field_scene.posture_dialog_open() ||
                    field_scene.command_overview_primary_page() != 2U ||
                    field_scene.posture_dialog_value() != baseline_posture ||
                    !final_focus.save(QString::fromStdWString(
                        field_posture_dump_path->wstring()))) {
                    return posture_failure(
                        "could not save the real Up-selected posture frame");
                }

                press(0x80U); // Start leaves posture focus first.
                if (field_scene.posture_dialog_open() ||
                    !field_scene.command_overview_open()) {
                    return posture_failure(
                        "Start did not return from focus to the command row");
                }
                press(0x80U); // A second Start returns to the field.
                const auto field_after_menu = capture();
                if (field_scene.command_overview_open() ||
                    image_region_differs(field_before_menu, field_after_menu,
                                         0, 0, kSceneWidth, kSceneHeight) ||
                    !save_frame("field-after-start-return.bmp",
                                field_after_menu)) {
                    return posture_failure(
                        "command-menu graphics remained over the field after "
                        "Start returned");
                }
                if (!save_frame("field-before-command-menu.bmp",
                                field_before_menu)) {
                    return posture_failure(
                        "could not save the field baseline frame");
                }
                std::cout << "field_posture_dump=written path="
                          << field_posture_dump_path->string()
                          << " frames=" << frames_directory.string()
                          << " pages=2-6 focus=Up/Down posture="
                          << static_cast<int>(baseline_posture)
                          << " audio=0x25-per-adjustment\n";
                return 0;
            }
            if (field_automatic_use_dump_path.has_value()) {
                std::filesystem::create_directories(
                    *field_automatic_use_dump_path);
                auto& runner = field_scene.active_runner_record();
                constexpr std::size_t kItemSlot = 0U;
                // 0x20 belongs to the ROM's 0x18..0x2B automatic-use range.
                constexpr std::uint8_t kAutomaticUseItemCode = 0x20U;
                constexpr std::uint8_t kInitialItemPhase = 7U;
                runner.inventory_item_codes[kItemSlot] =
                    kAutomaticUseItemCode;
                runner.inventory_phase_bytes[kItemSlot] = kInitialItemPhase;
                const auto before_action = runner;

                const auto render_scene = [&] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    if (!field_command_shell_survives(scene, asset_root) ||
                        !field_hud_chrome_survives(scene, asset_root)) {
                        std::cerr << "field automatic-use dump: command "
                                     "shell or HUD changed for "
                                  << name << "\n";
                        return false;
                    }
                    return scene.save(QString::fromStdWString(
                        ((*field_automatic_use_dump_path) /
                         (std::string(name) + ".bmp"))
                            .wstring()));
                };
                const auto press = [&](const std::uint8_t port_byte) {
                    field_scene.set_controller_port_byte(port_byte);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                const auto region_has_green_ink = [](
                                                   const QImage& scene,
                                                   const QRect& region) {
                    const auto clipped = region.intersected(scene.rect());
                    for (int y = clipped.top(); y <= clipped.bottom(); ++y) {
                        for (int x = clipped.left(); x <= clipped.right(); ++x) {
                            const auto pixel = scene.pixelColor(x, y);
                            if (pixel.green() > 40 &&
                                pixel.green() > pixel.red() * 2 &&
                                pixel.green() > pixel.blue() * 2) {
                                return true;
                            }
                        }
                    }
                    return false;
                };
                const auto same_region = [](const QImage& left,
                                            const QImage& right,
                                            const QRect& region) {
                    if (left.size() != right.size()) {
                        return false;
                    }
                    const auto clipped = region.intersected(left.rect());
                    for (int y = clipped.top(); y <= clipped.bottom(); ++y) {
                        for (int x = clipped.left(); x <= clipped.right(); ++x) {
                            if (left.pixel(x, y) != right.pixel(x, y)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };

                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field automatic-use dump: command overview "
                                 "did not open\n";
                    return 3;
                }
                for (int move = 0;
                     move < 7 && field_scene.command_overview_view()
                                         .source_state.primary_page != 0U;
                     ++move) {
                    press(0x04U);
                }
                if (field_scene.command_overview_view().source_state
                        .primary_page != 0U) {
                    std::cerr << "field automatic-use dump: inventory page "
                                 "did not become selected\n";
                    return 3;
                }

                // Ignore command-page navigation cues here. The checked
                // sequence begins with opening the item action and includes
                // its A-confirm request, both source command event 0x24.
                field_audio_events.clear();
                press(0x40U);
                if (!field_scene.inventory_action_open() ||
                    field_scene.inventory_action_selection() != 2U) {
                    std::cerr << "field automatic-use dump: item action did "
                                 "not open\n";
                    return 3;
                }
                const auto action_open_scene = render_scene();

                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                if (!field_scene.automatic_use_result_waiting() ||
                    field_scene.inventory_action_open() ||
                    field_scene.first_aid_open() ||
                    runner.inventory_item_codes[kItemSlot] !=
                        before_action.inventory_item_codes[kItemSlot] ||
                    runner.inventory_phase_bytes[kItemSlot] !=
                        before_action.inventory_phase_bytes[kItemSlot] ||
                    runner.equipped_item_code !=
                        before_action.equipped_item_code ||
                    runner.equipped_inventory_slot !=
                        before_action.equipped_inventory_slot ||
                    runner.equipped_armor_index !=
                        before_action.equipped_armor_index ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U}) {
                    std::cerr << "field automatic-use dump: dispatch state, "
                                 "item bytes, or action audio diverged\n";
                    return 3;
                }

                const auto message_scene = render_scene();
                if (!field_inventory_selector_survives(message_scene, 0U, 0U) ||
                    !same_region(action_open_scene, message_scene,
                                 QRect(112, 24, 136, 72)) ||
                    !region_has_green_ink(message_scene,
                                          QRect(128, 96, 120, 8)) ||
                    !region_has_green_ink(message_scene,
                                          QRect(128, 104, 120, 8)) ||
                    !region_has_green_ink(message_scene,
                                          QRect(128, 112, 120, 8)) ||
                    !save_scene("automatic_use_message", message_scene) ||
                    !save_scene("actual", message_scene)) {
                    std::cerr << "field automatic-use dump: source message, "
                                 "item selector, shell, or HUD failed\n";
                    return 3;
                }

                // D202F has three static lines. The source loop redraws only
                // after a fresh input edge; thirty idle frames must be exact
                // pixel repeats and produce no extra sound requests.
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                const auto idle_scene = render_scene();
                if (idle_scene != message_scene ||
                    !field_scene.automatic_use_result_waiting() ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U} ||
                    !save_scene("automatic_use_idle_30", idle_scene)) {
                    std::cerr << "field automatic-use dump: acknowledgement "
                                 "message animated or changed without input\n";
                    return 3;
                }

                // Keep the dispatch A held, then release it. Neither the
                // held event nor release edge is a fresh 0x0D94 input.
                field_scene.advance_frame();
                const auto held_scene = render_scene();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                const auto released_scene = render_scene();
                if (!field_scene.automatic_use_result_waiting() ||
                    held_scene != message_scene ||
                    released_scene != message_scene ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U} ||
                    !save_scene("automatic_use_held", held_scene) ||
                    !save_scene("automatic_use_released", released_scene)) {
                    std::cerr << "field automatic-use dump: held/released "
                                 "input incorrectly acknowledged the message\n";
                    return 3;
                }

                press(0x10U);
                if (field_scene.automatic_use_result_waiting() ||
                    runner.inventory_item_codes[kItemSlot] !=
                        before_action.inventory_item_codes[kItemSlot] ||
                    runner.inventory_phase_bytes[kItemSlot] !=
                        before_action.inventory_phase_bytes[kItemSlot] ||
                    runner.equipped_item_code !=
                        before_action.equipped_item_code ||
                    runner.equipped_inventory_slot !=
                        before_action.equipped_inventory_slot ||
                    runner.equipped_armor_index !=
                        before_action.equipped_armor_index ||
                    field_audio_events !=
                        std::vector<std::uint16_t>{0x24U, 0x24U}) {
                    std::cerr << "field automatic-use dump: new controller "
                                 "edge failed to return cleanly\n";
                    return 3;
                }
                const auto returned_scene = render_scene();
                if (field_inventory_selector_survives(returned_scene, 0U, 0U) ||
                    !save_scene("automatic_use_return", returned_scene)) {
                    std::cerr << "field automatic-use dump: return did not "
                                 "clear the item selector\n";
                    return 3;
                }
                std::cout << "field_automatic_use_dump=written path="
                          << field_automatic_use_dump_path->string() << "\n";
                return 0;
            }
            if (field_inventory_trade_dump_path.has_value()) {
                std::filesystem::create_directories(
                    *field_inventory_trade_dump_path);
                auto& source = field_scene.active_runner_record();
                constexpr std::size_t kItemSlot = 0U;
                constexpr std::uint8_t kTradeableItemCode = 0x15U;
                constexpr std::uint8_t kItemPhase = 5U;
                source.inventory_item_codes[kItemSlot] = kTradeableItemCode;
                source.inventory_phase_bytes[kItemSlot] = kItemPhase;
                source.equipped_inventory_slot = -1;
                source.equipped_item_code = 0U;
                const auto source_before_trade = source;

                const auto render_scene = [&] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    if (!field_command_shell_survives(scene, asset_root) ||
                        !field_hud_chrome_survives(scene, asset_root)) {
                        std::cerr << "field inventory-trade dump: shell or "
                                     "field HUD changed for "
                                  << name << "\n";
                        return false;
                    }
                    return scene.save(QString::fromStdWString(
                        ((*field_inventory_trade_dump_path) /
                         (std::string(name) + ".bmp"))
                            .wstring()));
                };
                const auto press = [&](const std::uint8_t port_byte) {
                    field_scene.set_controller_port_byte(port_byte);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                const auto has_green_ink = [](const QImage& scene,
                                              const QRect& region) {
                    const auto clipped = region.intersected(scene.rect());
                    for (int y = clipped.top(); y <= clipped.bottom(); ++y) {
                        for (int x = clipped.left(); x <= clipped.right(); ++x) {
                            const auto pixel = scene.pixelColor(x, y);
                            if (pixel.green() > 40 &&
                                pixel.green() > pixel.red() * 2 &&
                                pixel.green() > pixel.blue() * 2) {
                                return true;
                            }
                        }
                    }
                    return false;
                };
                const auto image_region_matches = [](
                                                      const QImage& scene,
                                                      const int x,
                                                      const int y,
                                                      const QImage& expected) {
                    if (expected.isNull() || x < 0 || y < 0 ||
                        x + expected.width() > scene.width() ||
                        y + expected.height() > scene.height()) {
                        return false;
                    }
                    for (int row = 0; row < expected.height(); ++row) {
                        for (int column = 0; column < expected.width();
                             ++column) {
                            if (scene.pixel(x + column, y + row) !=
                                expected.pixel(column, row)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };

                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field inventory-trade dump: command "
                                 "overview did not open\n";
                    return 3;
                }
                for (int move = 0;
                     move < 7 && field_scene.command_overview_view()
                                         .source_state.primary_page != 0U;
                     ++move) {
                    press(0x04U);
                }
                if (field_scene.command_overview_view()
                        .source_state.primary_page != 0U) {
                    std::cerr << "field inventory-trade dump: Inventory "
                                 "page did not become selected\n";
                    return 3;
                }

                const auto open_trade_action = [&] {
                    press(0x40U);
                    if (!field_scene.inventory_action_open()) {
                        return false;
                    }
                    press(0x08U);
                    press(0x08U);
                    return field_scene.inventory_action_selection() == 4U;
                };
                if (!open_trade_action()) {
                    std::cerr << "field inventory-trade dump: Trade action "
                                 "did not select\n";
                    return 3;
                }
                field_audio_events.clear();
                press(0x40U);
                if (field_scene.inventory_trade_open() ||
                    field_scene.inventory_action_open() ||
                    field_scene.inventory_action_message() !=
                        CommandOverviewActionMessage::none ||
                    source.inventory_item_codes[kItemSlot] !=
                        source_before_trade.inventory_item_codes[kItemSlot] ||
                    field_audio_events != std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field inventory-trade dump: one-member "
                                 "roster did not return without mutation\n";
                    return 3;
                }
                const auto no_recipient_scene = render_scene();
                if (!field_inventory_selector_survives(
                        no_recipient_scene, 0U, 0U) ||
                    !save_scene("trade_no_recipient", no_recipient_scene)) {
                    std::cerr << "field inventory-trade dump: source return "
                                 "or item selector failed\n";
                    return 3;
                }

                RunnerRecordFields full_recipient{};
                full_recipient.inventory_item_codes.fill(1U);
                field_scene.install_party_member_for_dump(
                    1U, full_recipient, "Stark");
                field_scene.install_party_member_for_dump(
                    2U, RunnerRecordFields{}, "Dog");
                if (!open_trade_action()) {
                    std::cerr << "field inventory-trade dump: second Trade "
                                 "action did not open\n";
                    return 3;
                }
                field_audio_events.clear();
                press(0x40U);
                if (!field_scene.inventory_trade_open() ||
                    field_scene.inventory_trade_recipient() != 1U ||
                    !field_scene.inventory_trade_recipient_full() ||
                    field_scene.inventory_trade_yes_selected() ||
                    field_audio_events != std::vector<std::uint16_t>{0x24U}) {
                    std::cerr << "field inventory-trade dump: roster order, "
                                 "full gate, default choice, or audio failed\n";
                    return 3;
                }
                const auto full_scene = render_scene();
                const QImage confirmation_background(QString::fromStdWString(
                    (asset_root / "menu" / "field" /
                     "action-confirm-background.bmp")
                        .wstring()));
                const QImage no_focus(QString::fromStdWString(
                    (asset_root / "menu" / "field" /
                     "action-confirm-focus-1.bmp")
                        .wstring()));
                const QImage yes_focus(QString::fromStdWString(
                    (asset_root / "menu" / "field" /
                     "action-confirm-focus-0.bmp")
                        .wstring()));
                if (!field_inventory_selector_survives(full_scene, 0U, 0U) ||
                    !has_green_ink(full_scene, QRect(120, 104, 176, 48)) ||
                    !image_region_matches(full_scene, 184, 184, no_focus) ||
                    confirmation_background.isNull() ||
                    full_scene.pixel(122, 185) !=
                        confirmation_background.pixel(2, 1) ||
                    !save_scene("trade_recipient_full", full_scene) ||
                    !save_scene("actual", full_scene)) {
                    std::cerr << "field inventory-trade dump: full message, "
                                 "No focus, source assets, shell, HUD, or "
                                 "selector failed\n";
                    return 3;
                }
                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                if (render_scene() != full_scene ||
                    !field_scene.inventory_trade_open()) {
                    std::cerr << "field inventory-trade dump: static full "
                                 "screen changed without input\n";
                    return 3;
                }

                press(0x10U);
                if (!field_scene.inventory_trade_open() ||
                    field_scene.inventory_trade_recipient() != 2U ||
                    field_scene.inventory_trade_recipient_full() ||
                    field_scene.inventory_trade_yes_selected()) {
                    std::cerr << "field inventory-trade dump: confirming a "
                                 "full recipient did not advance the scan\n";
                    return 3;
                }
                const auto dog_no_scene = render_scene();
                if (!has_green_ink(dog_no_scene, QRect(152, 120, 80, 8)) ||
                    !has_green_ink(dog_no_scene, QRect(128, 128, 96, 8)) ||
                    !field_inventory_selector_survives(dog_no_scene, 0U, 0U) ||
                    !image_region_matches(dog_no_scene, 184, 184, no_focus) ||
                    !save_scene("trade_recipient_dog_no", dog_no_scene)) {
                    std::cerr << "field inventory-trade dump: recipient "
                                 "label/name or default No state failed\n";
                    return 3;
                }
                press(0x04U);
                const auto dog_yes_scene = render_scene();
                if (!field_scene.inventory_trade_yes_selected() ||
                    !image_region_matches(dog_yes_scene, 136, 184, yes_focus) ||
                    !has_green_ink(dog_yes_scene, QRect(112, 200, 128, 8)) ||
                    !save_scene("trade_recipient_dog_yes", dog_yes_scene)) {
                    std::cerr << "field inventory-trade dump: Yes focus or "
                                 "choice text did not update\n";
                    return 3;
                }
                press(0x40U);
                const auto& traded_party =
                    field_scene.field_party_records_for_dump();
                const auto traded_scene = render_scene();
                if (field_scene.inventory_trade_open() ||
                    field_scene.active_runner_record()
                            .inventory_item_codes[kItemSlot] != 0U ||
                    traded_party[2].inventory_item_codes[0] !=
                        kTradeableItemCode ||
                    traded_party[2].inventory_phase_bytes[0] != kItemPhase ||
                    source.inventory_phase_bytes[kItemSlot] != 0U ||
                    field_audio_events != std::vector<std::uint16_t>{
                        0x24U, 0x24U, 0x25U, 0x24U} ||
                    !save_scene("trade_transferred", traded_scene)) {
                    std::cerr << "field inventory-trade dump: typed transfer, "
                                 "item phase, audio sequence, or return failed\n";
                    return 3;
                }
                std::cout << "field_inventory_trade_dump=written path="
                          << field_inventory_trade_dump_path->string() << "\n";
                return 0;
            }
            if (field_first_aid_dump_path.has_value()) {
                std::filesystem::create_directories(
                    *field_first_aid_dump_path);
                auto& patient = field_scene.active_runner_record();
                patient.delay_meter = 8U;
                patient.behavior_meter = 10U;
                std::size_t medkit_slot = patient.inventory_item_codes.size();
                for (std::size_t slot = 0;
                     slot < patient.inventory_item_codes.size(); ++slot) {
                    if (patient.inventory_item_codes[slot] == 0x10U) {
                        medkit_slot = slot;
                        break;
                    }
                }
                if (medkit_slot >= patient.inventory_item_codes.size()) {
                    std::cerr << "field First Aid dump: starter record has "
                                 "no source Medkit item\n";
                    return 3;
                }
                const auto medkit_phase =
                    patient.inventory_phase_bytes[medkit_slot];
                const auto pre_cancel_patient = patient;

                const auto render_scene = [&] {
                    QImage scene(kSceneWidth, kSceneHeight,
                                 QImage::Format_RGB32);
                    scene.fill(Qt::black);
                    QPainter painter(&scene);
                    field_scene.paint(painter);
                    painter.end();
                    return scene;
                };
                const auto save_scene = [&](const std::string_view name,
                                            const QImage& scene) {
                    if (!field_command_shell_survives(scene, asset_root) ||
                        !field_hud_chrome_survives(scene, asset_root)) {
                        std::cerr << "field First Aid dump: adjacent shell "
                                     "or HUD changed for "
                                  << name << "\n";
                        return false;
                    }
                    return scene.save(QString::fromStdWString(
                        ((*field_first_aid_dump_path) /
                         (std::string(name) + ".bmp"))
                            .wstring()));
                };
                const auto press = [&](const std::uint8_t port_byte) {
                    field_scene.set_controller_port_byte(port_byte);
                    field_scene.advance_frame();
                    field_scene.set_controller_port_byte(0U);
                    field_scene.advance_frame();
                };
                const auto region_has_green_ink = [](
                                                   const QImage& scene,
                                                   const QRect& region) {
                    const auto clipped = region.intersected(scene.rect());
                    for (int y = clipped.top(); y <= clipped.bottom(); ++y) {
                        for (int x = clipped.left(); x <= clipped.right(); ++x) {
                            const auto pixel = scene.pixelColor(x, y);
                            if (pixel.green() > 40 &&
                                pixel.green() > pixel.red() * 2 &&
                                pixel.green() > pixel.blue() * 2) {
                                return true;
                            }
                        }
                    }
                    return false;
                };
                const auto target_face_matches = [&](const QImage& scene) {
                    const QImage face(QString::fromStdWString(
                        (asset_root / "field" / "hud" /
                         ("face-" + std::to_string(field_archetype) + ".bmp"))
                            .wstring()));
                    if (face.isNull() || face.size() != QSize(32, 32)) {
                        return false;
                    }
                    int checked = 0;
                    for (int y = 0; y < face.height(); ++y) {
                        for (int x = 0; x < face.width(); ++x) {
                            const auto pixel = face.pixelColor(x, y);
                            if (pixel.alpha() == 0) {
                                continue;
                            }
                            ++checked;
                            if (scene.pixelColor(144 + x, 144 + y) != pixel) {
                                return false;
                            }
                        }
                    }
                    return checked > 64;
                };

                if (!field_scene.open_command_overview_for_dump()) {
                    std::cerr << "field First Aid dump: command overview "
                                 "did not open\n";
                    return 3;
                }
                for (int move = 0;
                     move < 7 && field_scene.command_overview_view()
                                         .source_state.primary_page != 0U;
                     ++move) {
                    press(0x04U);
                }
                const auto desired_column =
                    static_cast<std::uint8_t>(medkit_slot % 2U);
                const auto desired_row =
                    static_cast<std::uint8_t>(medkit_slot / 2U);
                if (desired_column != 0U) {
                    press(0x08U);
                }
                for (std::uint8_t row = 0; row < desired_row; ++row) {
                    press(0x02U);
                }
                const auto& reached = field_scene.command_overview_view()
                                          .source_state;
                if (reached.primary_page != desired_column ||
                    reached.secondary_page != desired_row ||
                    patient.inventory_item_codes[medkit_slot] != 0x10U) {
                    std::cerr << "field First Aid dump: Medkit cell did not "
                                 "become selected\n";
                    return 3;
                }

                press(0x40U);
                if (!field_scene.inventory_action_open() ||
                    field_scene.inventory_action_selection() != 2U) {
                    std::cerr << "field First Aid dump: Equip/Use action "
                                 "did not open\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.first_aid_open() ||
                    field_scene.first_aid_result_waiting() ||
                    field_scene.first_aid_targets().active_count != 1U ||
                    field_scene.first_aid_targets()
                            .active_record_indices[0] != 0U ||
                    field_scene.first_aid_target_cursor() != 0U) {
                    std::cerr << "field First Aid dump: active target list "
                                 "or initial selection is incorrect\n";
                    return 3;
                }
                const auto target_scene = render_scene();
                if (!field_inventory_selector_survives(
                        target_scene, desired_column, desired_row) ||
                    !target_face_matches(target_scene) ||
                    !region_has_green_ink(
                        target_scene, QRect(128, 88, 96, 8)) ||
                    !region_has_green_ink(
                        target_scene, QRect(128, 104, 96, 8)) ||
                    !region_has_green_ink(
                        target_scene, QRect(152, 112, 64, 8)) ||
                    !field_hud_face_family_survives(
                        target_scene, asset_root, field_archetype) ||
                    !save_scene("first_aid_target", target_scene)) {
                    std::cerr << "field First Aid dump: prompt, target, "
                                 "cursor, portrait, or item outline failed\n";
                    return 3;
                }
                if (!save_scene("actual", target_scene)) {
                    std::cerr << "field First Aid dump: could not write "
                                 "the canonical actual.bmp checkpoint\n";
                    return 3;
                }

                for (int frame = 0; frame < 30; ++frame) {
                    field_scene.advance_frame();
                }
                const auto target_idle_scene = render_scene();
                if (target_idle_scene != target_scene ||
                    !save_scene("first_aid_target_idle_30",
                                target_idle_scene)) {
                    std::cerr << "field First Aid dump: static target "
                                 "screen changed without input\n";
                    return 3;
                }

                // The ROM's target loop confirms with bit 14 only. B/C
                // presses must neither consume the item nor acknowledge the
                // target or emit the A-confirm audio command.
                const auto audio_events_before_non_a = field_audio_events;
                press(0x20U);
                press(0x10U);
                if (!field_scene.first_aid_open() ||
                    field_scene.first_aid_target_cursor() != 0U ||
                    field_scene.active_runner_record()
                            .inventory_phase_bytes[medkit_slot] !=
                        medkit_phase ||
                    field_audio_events != audio_events_before_non_a ||
                    render_scene() != target_scene) {
                    std::cerr << "field First Aid dump: B/C changed the "
                                 "A-only target loop\n";
                    return 3;
                }

                press(0x02U);
                if (field_scene.first_aid_target_cursor() !=
                    field_scene.first_aid_targets().cancel_row()) {
                    std::cerr << "field First Aid dump: Down did not select "
                                 "CANCEL\n";
                    return 3;
                }
                const auto cancel_scene = render_scene();
                if (!region_has_green_ink(
                        cancel_scene, QRect(144, 112, 72, 8)) ||
                    !save_scene("first_aid_cancel_selected", cancel_scene)) {
                    std::cerr << "field First Aid dump: CANCEL row failed\n";
                    return 3;
                }
                press(0x40U);
                if (field_scene.first_aid_open() ||
                    patient.inventory_item_codes[medkit_slot] != 0x10U ||
                    patient.inventory_phase_bytes[medkit_slot] !=
                        medkit_phase ||
                    patient.delay_meter != pre_cancel_patient.delay_meter ||
                    patient.behavior_meter !=
                        pre_cancel_patient.behavior_meter) {
                    std::cerr << "field First Aid dump: CANCEL changed the "
                                 "runner or medicine\n";
                    return 3;
                }
                const auto cancelled_scene = render_scene();
                if (field_inventory_selector_survives(
                        cancelled_scene, desired_column, desired_row) ||
                    !save_scene("first_aid_cancel_return", cancelled_scene)) {
                    std::cerr << "field First Aid dump: cancel return did "
                                 "not clear the item selector\n";
                    return 3;
                }

                // Re-enter the same occupied cell through the command tabs.
                // The medkit source archetype starts healthy, so lower its
                // already-typed physical meter to a valid injured value for
                // this deterministic effect-path fixture.
                press(0x08U);
                press(0x04U);
                press(0x40U);
                if (!field_scene.inventory_action_open() ||
                    field_scene.inventory_action_selection() != 2U) {
                    std::cerr << "field First Aid dump: item could not be "
                                 "reselected after CANCEL\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.first_aid_open() ||
                    field_scene.first_aid_target_cursor() != 0U ||
                    field_scene.first_aid_targets().active_count != 1U) {
                    std::cerr << "field First Aid dump: target list did not "
                                 "re-open after CANCEL count="
                              << field_scene.first_aid_targets().active_count
                              << " cursor="
                              << field_scene.first_aid_target_cursor()
                              << " open=" << field_scene.first_aid_open()
                              << " flags=0x" << std::hex << patient.flags
                              << std::dec << "\n";
                    return 3;
                }
                const auto before_use = field_scene.active_runner_record();
                const auto pre_confirm_target_count =
                    field_scene.first_aid_targets().active_count;
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                const auto& after_use = field_scene.active_runner_record();
                if (!field_scene.first_aid_result_waiting() ||
                    field_scene.first_aid_message() !=
                        FirstAidMessage::medkit_used ||
                    after_use.delay_meter <= before_use.delay_meter ||
                    after_use.inventory_item_codes[medkit_slot] != 0x10U ||
                    after_use.inventory_phase_bytes[medkit_slot] + 1U !=
                        before_use.inventory_phase_bytes[medkit_slot] ||
                    after_use.first_aid_continuation != 0xB45CU ||
                    (after_use.flags & 0x5100U) !=
                        (before_use.flags & 0x0100U)) {
                    std::cerr << "field First Aid dump: medkit effect, "
                                 "phase, status, or continuation failed"
                              << " state="
                              << field_scene.first_aid_result_waiting()
                              << " target_count="
                              << pre_confirm_target_count
                              << " message="
                              << static_cast<int>(
                                     field_scene.first_aid_message())
                              << " meter="
                              << static_cast<int>(before_use.delay_meter)
                              << "->"
                              << static_cast<int>(after_use.delay_meter)
                              << " phase="
                              << static_cast<int>(
                                     before_use.inventory_phase_bytes[
                                         medkit_slot])
                              << "->"
                              << static_cast<int>(
                                     after_use.inventory_phase_bytes[
                                         medkit_slot])
                              << " item=0x" << std::hex
                              << static_cast<int>(
                                     after_use.inventory_item_codes[
                                         medkit_slot])
                              << " flags=0x" << after_use.flags
                              << " continuation=0x"
                              << after_use.first_aid_continuation << std::dec
                              << "\n";
                    return 3;
                }
                field_scene.set_controller_port_byte(0x40U);
                field_scene.advance_frame();
                if (!field_scene.first_aid_result_waiting()) {
                    std::cerr << "field First Aid dump: held A dismissed "
                                 "the result before a new edge\n";
                    return 3;
                }
                const auto held_result_scene = render_scene();
                field_scene.set_controller_port_byte(0U);
                field_scene.advance_frame();
                if (!field_scene.first_aid_result_waiting()) {
                    std::cerr << "field First Aid dump: release dismissed "
                                 "the result before a new edge\n";
                    return 3;
                }
                const auto result_scene = render_scene();
                if (result_scene != held_result_scene ||
                    !field_inventory_selector_survives(
                        result_scene, desired_column, desired_row) ||
                    !target_face_matches(result_scene) ||
                    !region_has_green_ink(
                        result_scene, QRect(144, 104, 112, 8)) ||
                    !region_has_green_ink(
                        result_scene, QRect(136, 200, 96, 8)) ||
                    !field_hud_meter_assets_survive(
                        result_scene, asset_root, after_use.delay_meter,
                        after_use.behavior_meter, true) ||
                    !save_scene("first_aid_medkit_used", result_scene)) {
                    std::cerr << "field First Aid dump: result, persistent "
                                 "selector, target face, or live HUD failed\n";
                    return 3;
                }
                if (field_audio_events != std::vector<std::uint16_t>{
                                              0x24U, 0x24U, 0x25U, 0x24U,
                                              0x24U, 0x24U, 0x24U}) {
                    std::cerr << "field First Aid dump: source audio events "
                                 "do not match the menu/move/use sequence:";
                    for (const auto event : field_audio_events) {
                        std::cerr << " 0x" << std::hex << event;
                    }
                    std::cerr << std::dec << '\n';
                    return 3;
                }
                press(0x40U);
                if (field_scene.first_aid_open() ||
                    field_scene.active_runner_record().delay_meter !=
                        after_use.delay_meter ||
                    field_scene.active_runner_record()
                            .inventory_phase_bytes[medkit_slot] !=
                        after_use.inventory_phase_bytes[medkit_slot]) {
                    std::cerr << "field First Aid dump: result acknowledgement "
                                 "rolled back the treatment\n";
                    return 3;
                }
                const auto returned_scene = render_scene();
                if (field_inventory_selector_survives(
                        returned_scene, desired_column, desired_row) ||
                    !save_scene("first_aid_return", returned_scene)) {
                    std::cerr << "field First Aid dump: return kept the "
                                 "selected-item outline\n";
                    return 3;
                }
                std::cout << "field_first_aid_dump=written path="
                          << field_first_aid_dump_path->string()
                          << " slot=" << medkit_slot
                          << " physical="
                          << static_cast<int>(after_use.delay_meter)
                          << " mental="
                          << static_cast<int>(after_use.behavior_meter)
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
                const auto ammo_counter_matches = [&](
                    const QImage& scene, const std::uint8_t rounds) {
                    const auto asset_name = "ammo-counter-" +
                                            std::to_string(rounds) + ".bmp";
                    const auto counter = QImage(QString::fromStdWString(
                        (asset_root / "field" / "hud" / asset_name)
                            .wstring()));
                    return region_matches(
                        scene, counter,
                        field_hud_vram_x(kFieldHudAmmoVram),
                        field_hud_vram_y(kFieldHudAmmoVram));
                };
                const auto direct_weapon_icon = [&](const std::uint8_t code,
                                                     const std::uint8_t palette) {
                    const auto number = code < 10U
                        ? std::string("0") + std::to_string(code)
                        : std::to_string(code);
                    const auto name = "item-" + number + "-" +
                                      std::to_string(palette) + ".bmp";
                    return QImage(QString::fromStdWString(
                        (asset_root / "items" / "weapons" / "icons" /
                         name).wstring()));
                };
                const auto icon_interior_matches = [](
                    const QImage& scene, const QImage& icon,
                    const int origin_x, const int origin_y) {
                    if (icon.isNull() || icon.width() != 24 ||
                        icon.height() != 16 || origin_x < 0 || origin_y < 0 ||
                        origin_x + icon.width() > scene.width() ||
                        origin_y + icon.height() > scene.height()) {
                        return false;
                    }
                    // The 1px source cursor is drawn over the icon edge. Its
                    // interior must still match the live 24x16 named asset,
                    // including the opaque palette background of the plane
                    // glyph (these are not transparent SAT sprites).
                    for (int y = 1; y < icon.height() - 1; ++y) {
                        for (int x = 1; x < icon.width() - 1; ++x) {
                            if (scene.pixel(origin_x + x, origin_y + y) !=
                                icon.pixel(x, y)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto same_inventory_grid = [](const QImage& left,
                                                    const QImage& right) {
                    constexpr QRect kInventoryGrid(8, 112, 80, 104);
                    if (left.size() != right.size() ||
                        kInventoryGrid.right() >= left.width() ||
                        kInventoryGrid.bottom() >= left.height()) {
                        return false;
                    }
                    for (int y = kInventoryGrid.top();
                         y <= kInventoryGrid.bottom(); ++y) {
                        for (int x = kInventoryGrid.left();
                             x <= kInventoryGrid.right(); ++x) {
                            if (left.pixel(x, y) != right.pixel(x, y)) {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                const auto render_fresh_inventory_scene =
                    [&](const RunnerRecordFields& runner,
                        const std::uint8_t primary_page) {
                        qt_runtime::FieldSceneRuntime fresh_scene(
                            [](const auto&) {});
                        if (!fresh_scene.bootstrap(smoke_state)) {
                            return QImage{};
                        }
                        fresh_scene.active_runner_record() = runner;
                        fresh_scene.refresh_field_hud_for_dump();
                        if (!fresh_scene.open_command_overview_for_dump() ||
                            !fresh_scene.seek_command_overview_page(
                                primary_page)) {
                            return QImage{};
                        }
                        QImage image(kSceneWidth, kSceneHeight,
                                     QImage::Format_RGB32);
                        image.fill(Qt::black);
                        QPainter painter(&image);
                        fresh_scene.paint(painter);
                        painter.end();
                        return image;
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

                // Seed two ROM-defined weapons to exercise the complete
                // weapon -> stored percentage -> different weapon path. The
                // US ROM's D4540 +0x24 values are 6 for item 1 and 12 for
                // item 2; the actual FieldSceneRuntime menu handles every
                // input and redraw below.
                const auto original_runner_record =
                    field_scene.active_runner_record();
                auto& ammo_runner = field_scene.active_runner_record();
                ammo_runner.inventory_item_codes[0] = 1U;
                ammo_runner.inventory_phase_bytes[0] = 0U;
                ammo_runner.inventory_item_codes[1] = 2U;
                ammo_runner.inventory_phase_bytes[1] = 0U;
                ammo_runner.equipped_inventory_slot = 0;
                ammo_runner.equipped_item_code = 1U;
                ammo_runner.ammo_rounds = 3U;
                ammo_runner.ammo_clips = 3U;
                if (item_factor(1U) != 6U || item_factor(2U) != 12U) {
                    std::cerr << "field action dump: source weapon capacity "
                                 "rows 1 and 2 did not match ROM +0x24\n";
                    return 3;
                }
                const auto& initial_record = ammo_runner;
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
                const auto weapon_one_equipped_icon = direct_weapon_icon(1U, 1U);
                if (!region_matches(action_open_scene, focus_equip, 128, 184) ||
                    !field_inventory_selector_survives(action_open_scene, 0U,
                                                       0U) ||
                    !ammo_counter_matches(action_open_scene, 3U) ||
                    !icon_interior_matches(action_open_scene,
                                           weapon_one_equipped_icon, 16, 120) ||
                    action_open_scene.pixelColor(11, 116) !=
                        QColor(109, 109, 109) ||
                    !save_scene("00_inventory_action_equip",
                                action_open_scene)) {
                    std::cerr << "field action dump: could not write open "
                                 "scene, action focus, equipped icon, cell "
                                 "treatment, or item selector is misplaced\n";
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
                    field_scene.active_runner_record()
                            .equipped_inventory_slot != -1) {
                    std::cerr << "field action dump: equip action did not "
                                 "return to the command overview\n";
                    return 3;
                }
                const auto unequipped_scene = render_scene();
                const auto fresh_unequipped_scene =
                    render_fresh_inventory_scene(
                        field_scene.active_runner_record(),
                        field_scene.command_overview_view()
                            .source_state.primary_page);
                const auto fresh_unequipped_scene_saved = save_scene(
                    "05_fresh_unequipped_baseline",
                    fresh_unequipped_scene);
                const auto weapon_one_unequipped_icon =
                    direct_weapon_icon(1U, 0U);
                if (!field_inventory_selector_survives(unequipped_scene, 0U,
                                                        0U) ||
                    !ammo_counter_matches(unequipped_scene, 50U) ||
                    !icon_interior_matches(unequipped_scene,
                                           weapon_one_unequipped_icon, 16,
                                           120) ||
                    unequipped_scene.pixelColor(11, 116) !=
                        QColor(36, 36, 36) ||
                    !same_inventory_grid(unequipped_scene,
                                         fresh_unequipped_scene) ||
                    !fresh_unequipped_scene_saved ||
                    !save_scene("05_weapon_1_unequipped", unequipped_scene)) {
                    std::cerr << "field action dump: unequip left a stale "
                                 "cell treatment, lost the source cursor, or "
                                 "changed the unequipped icon or failed to "
                                 "match a fresh scene redraw\n";
                    return 3;
                }
                if (field_scene.active_runner_record().ammo_rounds != 50U ||
                    field_scene.active_runner_record().ammo_clips != 3U ||
                    field_scene.active_runner_record().equipped_item_code !=
                        0U) {
                    std::cerr << "field action dump: first gun did not store "
                                 "its 3/6 load as 50 percent on unequip\n";
                    return 3;
                }

                // An equip/unequip cycle must not leave old cell graphics
                // behind. Repeat it and compare the entire 2x4 inventory
                // region against the first unequipped frame; the source icon
                // is static, so only the live equipment treatment changes.
                press(0x40U);
                if (!field_scene.inventory_action_open()) {
                    std::cerr << "field action dump: repeated equip action "
                                 "did not open\n";
                    return 3;
                }
                press(0x40U);
                if (field_scene.active_runner_record()
                            .equipped_inventory_slot != 0) {
                    std::cerr << "field action dump: repeated equip did not "
                                 "select weapon 1\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.inventory_action_open()) {
                    std::cerr << "field action dump: repeated unequip action "
                                 "did not open\n";
                    return 3;
                }
                press(0x40U);
                const auto repeated_unequip_scene = render_scene();
                if (field_scene.active_runner_record()
                            .equipped_inventory_slot != -1 ||
                    !same_inventory_grid(unequipped_scene,
                                         repeated_unequip_scene) ||
                    !save_scene("18_weapon_1_unequip_redraw_repeat",
                                repeated_unequip_scene)) {
                    std::cerr << "field action dump: repeated equip/unequip "
                                 "left stale graphics in the inventory grid\n";
                    return 3;
                }

                // Select weapon 2 from the adjacent inventory column. The
                // shared percentage must become six rounds at its 12-round
                // capacity; the runner-wide Universal Gun Clips stay at 3.
                press(0x08U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 1U) {
                    std::cerr << "field action dump: second weapon column "
                                 "did not become selected\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.inventory_action_open()) {
                    std::cerr << "field action dump: second weapon action "
                                 "row did not open\n";
                    return 3;
                }
                const auto switch_audio_event_count = field_audio_events.size();
                press(0x40U);
                const auto& weapon_two_equipped =
                    field_scene.active_runner_record();
                const auto weapon_two_equipped_scene = render_scene();
                const auto weapon_two_equipped_icon =
                    direct_weapon_icon(2U, 1U);
                if (weapon_two_equipped.equipped_inventory_slot != 1 ||
                    weapon_two_equipped.equipped_item_code != 2U ||
                    weapon_two_equipped.ammo_rounds != 6U ||
                    weapon_two_equipped.ammo_clips != 3U ||
                    field_audio_events.size() != switch_audio_event_count + 1U ||
                    field_audio_events.back() != 0x24U ||
                    !ammo_counter_matches(weapon_two_equipped_scene, 6U) ||
                    !icon_interior_matches(weapon_two_equipped_scene,
                                           weapon_two_equipped_icon, 56, 120) ||
                    weapon_two_equipped_scene.pixelColor(51, 116) !=
                        QColor(109, 109, 109) ||
                    !save_scene("10_weapon_2_equipped_at_50_percent",
                                weapon_two_equipped_scene) ||
                    !save_scene("actual", weapon_two_equipped_scene)) {
                    std::cerr << "field action dump: weapon 2 did not receive "
                                 "six rounds, its equipment cell, or one "
                                 "command-confirm event\n";
                    return 3;
                }

                // Un-equipping weapon 2 returns its 6/12 load to 50 percent;
                // weapon 1 then reconstructs 3/6 from that same percentage.
                press(0x40U);
                if (!field_scene.inventory_action_open()) {
                    std::cerr << "field action dump: weapon 2 could not be "
                                 "selected for unequip\n";
                    return 3;
                }
                press(0x40U);
                const auto weapon_two_unequipped_scene = render_scene();
                const auto fresh_weapon_two_unequipped_scene =
                    render_fresh_inventory_scene(
                        field_scene.active_runner_record(),
                        field_scene.command_overview_view()
                            .source_state.primary_page);
                const auto fresh_weapon_two_unequipped_scene_saved =
                    save_scene("11_fresh_weapon_2_unequipped_baseline",
                               fresh_weapon_two_unequipped_scene);
                const auto weapon_two_unequipped_icon =
                    direct_weapon_icon(2U, 0U);
                if (field_scene.active_runner_record().equipped_item_code !=
                        0U ||
                    field_scene.active_runner_record().ammo_rounds != 50U ||
                    !field_inventory_selector_survives(
                        weapon_two_unequipped_scene, 1U, 0U) ||
                    !ammo_counter_matches(weapon_two_unequipped_scene, 50U) ||
                    !icon_interior_matches(weapon_two_unequipped_scene,
                                           weapon_two_unequipped_icon, 56,
                                           120) ||
                    weapon_two_unequipped_scene.pixelColor(51, 116) !=
                        QColor(36, 36, 36) ||
                    !same_inventory_grid(weapon_two_unequipped_scene,
                                         fresh_weapon_two_unequipped_scene) ||
                    !fresh_weapon_two_unequipped_scene_saved ||
                    !save_scene("11_weapon_2_unequipped_at_50_percent",
                                weapon_two_unequipped_scene)) {
                    std::cerr << "field action dump: weapon 2 did not return "
                                 "to the 50 percent state or left its "
                                 "equipment treatment behind or failed to "
                                 "match a fresh scene redraw\n";
                    return 3;
                }
                press(0x04U);
                press(0x40U);
                if (!field_scene.inventory_action_open()) {
                    std::cerr << "field action dump: weapon 1 could not be "
                                 "reselected\n";
                    return 3;
                }
                press(0x40U);
                const auto weapon_one_reloaded_from_percent_scene =
                    render_scene();
                if (field_scene.active_runner_record().equipped_inventory_slot !=
                        0 ||
                    field_scene.active_runner_record().equipped_item_code !=
                        1U ||
                    field_scene.active_runner_record().ammo_rounds != 3U ||
                    field_scene.active_runner_record().ammo_clips != 3U ||
                    !ammo_counter_matches(
                        weapon_one_reloaded_from_percent_scene, 3U) ||
                    !save_scene("12_weapon_1_reloaded_from_50_percent",
                                weapon_one_reloaded_from_percent_scene)) {
                    std::cerr << "field action dump: weapon 1 did not restore "
                                 "three rounds from the shared percentage\n";
                    return 3;
                }

                // Reload while weapon 1 is active. The Clips route receives
                // this gun's six-round +0x24 limit, consumes one shared clip,
                // and plays the source confirmation/selector events.
                press(0x08U);
                press(0x08U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 2U) {
                    std::cerr << "field action dump: Clips page did not "
                                 "become selected for weapon 1\n";
                    return 3;
                }
                auto clips_audio_event_count = field_audio_events.size();
                press(0x40U);
                if (!field_scene.clips_menu_open() ||
                    field_audio_events.size() != clips_audio_event_count + 1U ||
                    field_audio_events.back() != 0x24U) {
                    std::cerr << "field action dump: Clips child did not open "
                                 "with source confirm audio\n";
                    return 3;
                }
                clips_audio_event_count = field_audio_events.size();
                press(0x40U);
                if (!field_scene.clips_reload_confirmation_open() ||
                    field_scene.clips_reload_confirmation_yes() ||
                    field_scene.active_runner_record().ammo_rounds != 3U ||
                    field_scene.active_runner_record().ammo_clips != 3U ||
                    field_audio_events.size() != clips_audio_event_count + 1U ||
                    field_audio_events.back() != 0x24U ||
                    !save_scene("13_reload_confirmation_no", render_scene())) {
                    std::cerr << "field action dump: Reload did not show the "
                                 "default-No prompt and confirm sound\n";
                    return 3;
                }
                clips_audio_event_count = field_audio_events.size();
                press(0x08U);
                if (!field_scene.clips_reload_confirmation_yes() ||
                    field_audio_events.size() != clips_audio_event_count + 1U ||
                    field_audio_events.back() != 0x25U ||
                    !save_scene("14_reload_confirmation_yes", render_scene())) {
                    std::cerr << "field action dump: Reload prompt did not "
                                 "select Yes with selector audio\n";
                    return 3;
                }
                clips_audio_event_count = field_audio_events.size();
                press(0x40U);
                const auto weapon_one_full_scene = render_scene();
                if (field_scene.active_runner_record().ammo_rounds != 6U ||
                    field_scene.active_runner_record().ammo_clips != 2U ||
                    field_scene.clips_menu_message() !=
                        qt_runtime::ClipsMenuMessage::reload_success ||
                    field_audio_events.size() != clips_audio_event_count + 1U ||
                    field_audio_events.back() != 0x24U ||
                    !ammo_counter_matches(weapon_one_full_scene, 6U) ||
                    !save_scene("15_weapon_1_reloaded_to_capacity",
                                weapon_one_full_scene)) {
                    std::cerr << "field action dump: Reload did not cap at "
                                 "six rounds, consume one clip, and confirm\n";
                    return 3;
                }
                press(0x10U);
                if (field_scene.clips_menu_message() !=
                    qt_runtime::ClipsMenuMessage::none) {
                    std::cerr << "field action dump: reload result did not "
                                 "acknowledge\n";
                    return 3;
                }
                clips_audio_event_count = field_audio_events.size();
                press(0x10U);
                if (field_scene.clips_menu_open() ||
                    field_audio_events.size() != clips_audio_event_count + 1U ||
                    field_audio_events.back() != 0x24U ||
                    !save_scene("16_clips_closed_parent_redraw",
                                render_scene())) {
                    std::cerr << "field action dump: Clips return did not "
                                 "restore its command page\n";
                    return 3;
                }
                const auto parent_after_reload = render_scene();
                press(0x40U);
                if (!field_scene.clips_menu_open()) {
                    std::cerr << "field action dump: Clips did not reopen for "
                                 "the clear-old-controls check\n";
                    return 3;
                }
                press(0x04U);
                if (field_scene.clips_menu_selection() != 5U) {
                    std::cerr << "field action dump: Clips Cancel row did not "
                                 "replace Reload row\n";
                    return 3;
                }
                press(0x40U);
                const auto parent_after_cancel = render_scene();
                if (field_scene.clips_menu_open() ||
                    !same_image(parent_after_reload, parent_after_cancel) ||
                    !save_scene("17_clips_cancel_clears_child_controls",
                                parent_after_cancel)) {
                    std::cerr << "field action dump: Clips cancel left stale "
                                 "child buttons over the command page\n";
                    return 3;
                }
                press(0x04U);
                press(0x04U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 0U) {
                    std::cerr << "field action dump: ammo QA did not return to "
                                 "the first inventory column\n";
                    return 3;
                }

                // Verify the Clips controller resolves capacity from the
                // currently equipped weapon, not from weapon 1 or a stale
                // menu value. Empty +0x4E here as a combat-produced test
                // state; the combat firing/target loop is a separate hill.
                press(0x08U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 1U) {
                    std::cerr << "field action dump: second weapon column did "
                                 "not become selected for reload QA\n";
                    return 3;
                }
                press(0x40U);
                if (!field_scene.inventory_action_open()) {
                    std::cerr << "field action dump: weapon 2 equip action did "
                                 "not open for reload QA\n";
                    return 3;
                }
                const auto weapon_two_switch_audio_count =
                    field_audio_events.size();
                press(0x40U);
                const auto weapon_two_equipped_before_reload =
                    field_scene.active_runner_record();
                const auto weapon_two_switch_scene = render_scene();
                if (weapon_two_equipped_before_reload.equipped_item_code !=
                        2U ||
                    weapon_two_equipped_before_reload.ammo_rounds != 12U ||
                    weapon_two_equipped_before_reload.ammo_clips != 2U ||
                    field_audio_events.size() !=
                        weapon_two_switch_audio_count + 1U ||
                    field_audio_events.back() != 0x24U ||
                    !ammo_counter_matches(weapon_two_switch_scene, 12U) ||
                    !save_scene("19_weapon_2_equipped_full_capacity",
                                weapon_two_switch_scene)) {
                    std::cerr << "field action dump: weapon 2 did not receive "
                                 "its 12-round capacity before reload QA\n";
                    return 3;
                }
                field_scene.active_runner_record().ammo_rounds = 0U;
                field_scene.refresh_field_hud_for_dump();
                const auto weapon_two_empty_scene = render_scene();
                if (!ammo_counter_matches(weapon_two_empty_scene, 0U) ||
                    !save_scene("20_weapon_2_empty_magazine",
                                weapon_two_empty_scene)) {
                    std::cerr << "field action dump: weapon 2 empty-magazine "
                                 "state did not reach its visible HUD counter\n";
                    return 3;
                }
                press(0x08U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 2U) {
                    std::cerr << "field action dump: Clips page did not become "
                                 "selected for weapon 2 reload QA\n";
                    return 3;
                }
                press(0x40U);
                const auto weapon_two_reload_audio_count =
                    field_audio_events.size();
                press(0x40U);
                const auto weapon_two_full_scene = render_scene();
                if (field_scene.active_runner_record().ammo_rounds != 12U ||
                    field_scene.active_runner_record().ammo_clips != 1U ||
                    field_scene.clips_menu_message() !=
                        qt_runtime::ClipsMenuMessage::reload_success ||
                    field_audio_events.size() !=
                        weapon_two_reload_audio_count + 1U ||
                    field_audio_events[weapon_two_reload_audio_count] !=
                        0x24U ||
                    field_audio_events.back() != 0x24U ||
                    !ammo_counter_matches(weapon_two_full_scene, 12U) ||
                    !save_scene("21_weapon_2_reloaded_to_capacity",
                                weapon_two_full_scene)) {
                    std::cerr << "field action dump: weapon 2 reload did not "
                                 "use its 12-round capacity and consume one "
                                 "shared clip\n";
                    return 3;
                }
                press(0x10U);
                press(0x10U);
                if (field_scene.clips_menu_open()) {
                    std::cerr << "field action dump: weapon 2 Clips menu did "
                                 "not close after acknowledging reload\n";
                    return 3;
                }
                press(0x04U);
                press(0x04U);
                if (field_scene.command_overview_view().source_state
                        .primary_page != 0U) {
                    std::cerr << "field action dump: weapon 2 reload QA did "
                                 "not return to the first inventory column\n";
                    return 3;
                }
                field_scene.active_runner_record() = original_runner_record;

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

                StartupIntegrationState party_state;
                std::string party_state_error;
                if (!build_smoke_state(0U, party_state, party_state_error)) {
                    std::cerr << party_state_error << '\n';
                    return 3;
                }
                qt_runtime::FieldSceneRuntime party_scene(
                    [](const auto&) {});
                if (!party_scene.bootstrap(party_state)) {
                    std::cerr << "field HUD party QA: "
                              << party_scene.failure() << '\n';
                    return 3;
                }
                const auto load_party_record =
                    [&party_state](const std::size_t template_index) {
                        LiveRunnerRecord source{};
                        source.bytes = party_state.startup_records
                                           ->save_roster_templates[template_index]
                                           .record;
                        return read_runner_record_fields(source);
                    };
                const auto ricky = load_party_record(1U);
                const auto winston = load_party_record(2U);
                party_scene.install_party_member_for_dump(
                    1U, ricky, "Ricky", 3U);
                party_scene.install_party_member_for_dump(
                    2U, winston, "Winston Marrs", 4U);
                party_scene.set_active_party_slot_for_dump(1U);

                const auto portrait_palette_for = [](const RunnerRecordFields& record) {
                    const auto flags = select_hud_glyph_attribute_flags(
                        HudGlyphAttributeInput{
                            static_cast<std::int8_t>(record.equipped_item_code),
                            static_cast<std::int8_t>(record.archetype_index),
                            record.equipped_inventory_slot});
                    return static_cast<std::size_t>(
                        flags == 0x6000U ? 1U : 0U);
                };
                const auto item_palette_for = [](const RunnerRecordFields& record) {
                    const auto line = static_cast<std::uint8_t>(
                        (record.command_overview_panel_parameter & 0x6000U) >>
                        13U);
                    return static_cast<std::size_t>(
                        line == 0U ? 0U : line - 1U);
                };
                const auto save_party_frame =
                    [&](const std::string& directory, QImage& destination) {
                        destination = QImage(kSceneWidth, kSceneHeight,
                                             QImage::Format_RGB32);
                        destination.fill(Qt::black);
                        QPainter party_painter(&destination);
                        party_scene.paint(party_painter);
                        party_painter.end();
                        if (!field_hud_chrome_survives(destination, asset_root)) {
                            return false;
                        }
                        const auto output_directory =
                            *field_hud_qa_path / directory;
                        std::filesystem::create_directories(output_directory);
                        return destination.save(QString::fromStdWString(
                            (output_directory / "actual.bmp").wstring()));
                    };
                QImage party_equipped;
                if (!save_party_frame("party-equipped", party_equipped) ||
                    !field_hud_party_slot_asset_survives(
                        party_equipped, asset_root,
                        "field/hud/party/face-3-palette-" +
                            std::to_string(portrait_palette_for(ricky)) +
                            ".bmp",
                        280, 88, QSize(32, 32)) ||
                    !field_hud_party_slot_asset_survives(
                        party_equipped, asset_root,
                        "field/hud/party/item-" +
                            std::to_string(ricky.equipped_item_code) +
                            "-palette-" +
                            std::to_string(item_palette_for(ricky)) + ".bmp",
                        288, 128, QSize(24, 16)) ||
                    !field_hud_party_slot_asset_survives(
                        party_equipped, asset_root,
                        "field/hud/party/face-4-palette-" +
                            std::to_string(portrait_palette_for(winston)) +
                            ".bmp",
                        280, 152, QSize(32, 32)) ||
                    !field_hud_party_slot_asset_survives(
                        party_equipped, asset_root,
                        "field/hud/party/item-" +
                            std::to_string(winston.equipped_item_code) +
                            "-palette-" +
                            std::to_string(item_palette_for(winston)) +
                            ".bmp",
                        288, 192, QSize(24, 16))) {
                    std::cerr << "field HUD party QA: a recruited runner's "
                                 "portrait/item did not match its typed record\n";
                    return 3;
                }

                party_scene.set_active_party_slot_for_dump(2U);
                QImage party_active_winston;
                if (!save_party_frame("party-active-winston",
                                      party_active_winston) ||
                    !field_hud_party_slot_asset_survives(
                        party_active_winston, asset_root,
                        "field/hud/party/face-3-palette-" +
                            std::to_string(portrait_palette_for(ricky)) +
                            ".bmp",
                        280, 88, QSize(32, 32)) ||
                    !field_hud_party_slot_asset_survives(
                        party_active_winston, asset_root,
                        "field/hud/party/face-4-palette-" +
                            std::to_string(portrait_palette_for(winston)) +
                            ".bmp",
                        280, 152, QSize(32, 32))) {
                    std::cerr << "field HUD party QA: changing the controlled "
                                 "runner reordered a fixed HUD panel\n";
                    return 3;
                }

                auto ricky_unarmed = ricky;
                ricky_unarmed.equipped_inventory_slot = 0x7F;
                ricky_unarmed.equipped_item_code = 0U;
                party_scene.set_active_party_slot_for_dump(1U);
                party_scene.install_party_member_for_dump(
                    1U, ricky_unarmed, "Ricky", 3U);
                QImage party_unarmed;
                if (!save_party_frame("party-ricky-unarmed", party_unarmed) ||
                    !field_hud_party_slot_asset_survives(
                        party_unarmed, asset_root,
                        "field/hud/party/unarmed-item-0-palette-" +
                            std::to_string(item_palette_for(ricky_unarmed)) +
                            ".bmp",
                        288, 128, QSize(24, 16)) ||
                    party_equipped.copy(QRect(288, 128, 24, 16)) ==
                        party_unarmed.copy(QRect(288, 128, 24, 16))) {
                    std::cerr << "field HUD party QA: unequipping gear left "
                                 "the previous item image in place\n";
                    return 3;
                }

                StartupIntegrationState meter_state;
                std::string meter_state_error;
                if (!build_smoke_state(0U, meter_state,
                                       meter_state_error)) {
                    std::cerr << meter_state_error << '\n';
                    return 3;
                }
                qt_runtime::FieldSceneRuntime meter_scene(
                    [](const auto&) {});
                if (!meter_scene.bootstrap(meter_state)) {
                    std::cerr << "field HUD meter QA: "
                              << meter_scene.failure() << '\n';
                    return 3;
                }
                const auto capture_meter = [&](const std::uint8_t delay,
                                               const std::uint8_t behavior,
                                               const std::string& label,
                                               QImage& destination) {
                    auto& record = meter_scene.active_runner_record();
                    record.delay_meter = delay;
                    record.behavior_meter = behavior;
                    destination = QImage(kSceneWidth, kSceneHeight,
                                         QImage::Format_RGB32);
                    destination.fill(Qt::black);
                    QPainter meter_painter(&destination);
                    meter_scene.paint(meter_painter);
                    meter_painter.end();
                    if (!field_hud_meter_assets_survive(
                            destination, asset_root, delay, behavior, true) ||
                        !field_hud_chrome_survives(destination, asset_root)) {
                        std::cerr << "field HUD meter QA: " << label
                                  << " did not display the selected live row\n";
                        return false;
                    }
                    const auto path = *field_hud_qa_path /
                                      ("field-hud-" + label + ".bmp");
                    if (!destination.save(QString::fromStdWString(
                            path.wstring()))) {
                        std::cerr << "field HUD meter QA: could not write "
                                  << path.string() << '\n';
                        return false;
                    }
                    return true;
                };

                std::array<QImage, 3> delay_frames{};
                constexpr std::array<std::uint8_t, 3> kMeterSamples{0U, 5U,
                                                                    10U};
                for (std::size_t sample = 0;
                     sample < kMeterSamples.size(); ++sample) {
                    const auto value = kMeterSamples[sample];
                    if (!capture_meter(value, 0U,
                                       "delay-" + std::to_string(value),
                                       delay_frames[sample])) {
                        return 3;
                    }
                }
                for (std::size_t a = 0; a < delay_frames.size(); ++a) {
                    for (std::size_t b = a + 1U; b < delay_frames.size();
                         ++b) {
                        if (delay_frames[a].copy(QRect(272, 24, 8, 32)) ==
                            delay_frames[b].copy(QRect(272, 24, 8, 32))) {
                            std::cerr << "field HUD meter QA: delay rows "
                                      << a << " and " << b
                                      << " rendered identically\n";
                            return 3;
                        }
                    }
                }

                std::array<QImage, 3> behavior_frames{};
                for (std::size_t sample = 0;
                     sample < kMeterSamples.size(); ++sample) {
                    const auto value = kMeterSamples[sample];
                    if (!capture_meter(10U, value,
                                       "behavior-" + std::to_string(value),
                                       behavior_frames[sample])) {
                        return 3;
                    }
                }
                for (std::size_t a = 0; a < behavior_frames.size(); ++a) {
                    for (std::size_t b = a + 1U;
                         b < behavior_frames.size(); ++b) {
                        if (behavior_frames[a].copy(QRect(280, 56, 32, 8)) ==
                            behavior_frames[b].copy(QRect(280, 56, 32, 8))) {
                            std::cerr << "field HUD meter QA: behavior rows "
                                      << a << " and " << b
                                      << " rendered identically\n";
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
        if (boot_fasa_qa_path.has_value()) {
            return window.run_boot_fasa_qa(*boot_fasa_qa_path);
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

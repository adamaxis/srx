#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QString>
#include <QWidget>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;

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
                   " [--assets <decompilation/assets>]\n";
            return {};
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
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return kStartupMenuA;
    default:
        return 0U;
    }
}

QString phase_title(const StartupIntegrationPhase phase) {
    return QStringLiteral("Shadowrun - Ghidra Conversion Qt [%1]")
        .arg(QString::fromLatin1(startup_integration_phase_name(phase)));
}

class GhidraConversionWindow final : public QWidget {
public:
    explicit GhidraConversionWindow(std::filesystem::path asset_root,
                                    QWidget* parent = nullptr)
        : QWidget(parent), asset_root_(std::move(asset_root)) {
        setWindowTitle(QStringLiteral("Shadowrun - Ghidra Conversion Qt"));
        setFixedSize(kSceneWidth * kSceneScale, kSceneHeight * kSceneScale);
        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_OpaquePaintEvent);

        configure_startup_callbacks();
        const auto result = start_startup_integration(
            asset_root_, state_, startup_callbacks_);
        if (result != StartupIntegrationStartResult::title_ready) {
            throw std::runtime_error(
                "converted startup did not reach title_ready; phase=" +
                std::string(startup_integration_phase_name(state_.phase)));
        }

        load_direct_title_assets();
        setWindowTitle(phase_title(state_.phase));

    }

protected:
    void paintEvent(QPaintEvent*) override {
        QImage scene(kSceneWidth, kSceneHeight,
                     QImage::Format_ARGB32_Premultiplied);
        scene.fill(Qt::black);

        QPainter scene_painter(&scene);
        if (state_.phase == StartupIntegrationPhase::start_options ||
            state_.phase == StartupIntegrationPhase::save_load ||
            state_.phase == StartupIntegrationPhase::joshua_archetype) {
            draw_alias(scene_painter, "menu.startup.background");
            draw_menu_overlay(scene_painter);
        } else {
            draw_title(scene_painter);
        }
        scene_painter.end();

        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawImage(rect(), scene);
    }

    void keyPressEvent(QKeyEvent* event) override {
        const auto sample = sampled_menu_byte(event->key());
        if (sample == 0U) {
            QWidget::keyPressEvent(event);
            return;
        }

        if (state_.phase == StartupIntegrationPhase::title_ready) {
            if ((sample & kStartupMenuActionMask) != 0U) {
                enter_start_options_integration(state_, startup_callbacks_);
            }
        } else if (state_.phase == StartupIntegrationPhase::start_options ||
                   state_.phase == StartupIntegrationPhase::save_load ||
                   state_.phase == StartupIntegrationPhase::joshua_archetype) {
            advance_startup_integration(state_, sample, startup_callbacks_);
        }

        setWindowTitle(phase_title(state_.phase));
        update();
        event->accept();
    }

private:
    void configure_startup_callbacks() {
        startup_callbacks_.initialize_display = [this] {
            display_initialized_ = true;
        };
        startup_callbacks_.vdp_ready = {
            [this](const std::uint32_t delay) { last_source_delay_ = delay; },
            [this] { display_ready_ = true; },
            [this](const std::uint32_t delay) { last_source_delay_ = delay; },
        };

        startup_callbacks_.pre_title.resources.clear_plane_a = [] {};
        startup_callbacks_.pre_title.resources.clear_plane_b = [] {};
        startup_callbacks_.pre_title.resources.prepare_title_resources = [] {};
        startup_callbacks_.pre_title.resources.write_resource_stream =
            [this](const auto& words) { resource_stream_words_ = words.size(); };
        startup_callbacks_.pre_title.resources.refresh_field_resource_streams =
            [] {};
        startup_callbacks_.pre_title.resources.decode_and_upload_source_stream =
            [] {};
        startup_callbacks_.pre_title.resources.advance_title_palette_fade =
            [this] { ++palette_fade_steps_; };
        startup_callbacks_.pre_title.resources.write_vdp_grid = [] {};
        startup_callbacks_.pre_title.resources.reset_vdp_upload_state = [] {};

        startup_callbacks_.pre_title.animation.build_title_sprite_records =
            [this] { ++title_sprite_record_builds_; };
        startup_callbacks_.pre_title.animation.publish_logo_strobe_phase =
            [this](const std::uint8_t phase) { logo_strobe_phase_ = phase; };
        startup_callbacks_.pre_title.animation.step_entity_color_words = [] {};
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
        startup_callbacks_.pre_title.service_audio_variant =
            [this] { ++audio_variant_ticks_; };
        startup_callbacks_.pre_title.service_audio =
            [this] { ++audio_ticks_; };
        startup_callbacks_.pre_title.sample_controller = [] {};
        startup_callbacks_.pre_title.step_palette_transition =
            [this] { ++palette_transition_steps_; };

        startup_callbacks_.title_setup = {
            [] {},
            [this] { ++title_resource_loads_; },
            [this] { ++title_font_loads_; },
            [this](const std::uint16_t frames) {
                title_palette_frames_ += frames;
            },
        };

        // These are converted menu call boundaries. Qt invalidates the
        // presentation here; selection and phase changes remain owned by the
        // converted menu functions.
        startup_callbacks_.start_options = {
            [this] { update(); },
            [this] { update(); },
            [this] { update(); },
            [this] { update(); },
            [this] { update(); },
            [this] { update(); },
            [this](const StartOptionsChoice) { update(); },
        };
        startup_callbacks_.save_load = {
            [this] { update(); },
            [this] { update(); },
            [this](const SaveSlotChoice) { update(); },
            [](const SaveSlotChoice) {
                return SaveLoadAttempt::empty_or_invalid;
            },
            [this](const SaveLoadMessage) { update(); },
            [] {},
        };
        startup_callbacks_.joshua_archetype = {
            [this] { update(); },
            [this] { update(); },
            [this](const JoshuaArchetypeChoice) { update(); },
            [this](const JoshuaArchetypeChoice) { update(); },
            [this](const JoshuaArchetypeChoice choice) {
                selected_archetype_ = choice;
                update();
            },
        };
        startup_callbacks_.phase_changed =
            [this](const StartupIntegrationPhase phase) {
                setWindowTitle(phase_title(phase));
            };
        startup_callbacks_.field_ready = [this] { update(); };
    }

    void load_direct_title_assets() {
        draw_asset_cache_.clear();
        for (const auto& alias : {
                 std::string("title.background"),
                 std::string("title.logo"),
                 std::string("title.text"),
                 std::string("menu.startup.background"),
                 std::string("menu.startup.cursor")}) {
            cache_asset(alias);
        }
        for (int index = 0; index < 7; ++index) {
            cache_asset("title.circuit.background." + std::to_string(index));
        }
    }

    void cache_asset(const std::string& alias) {
        const auto* record = state_.asset_catalog.catalog.find(alias);
        if (record == nullptr) {
            return;
        }
        QImage image(QString::fromStdWString(record->absolute_path.wstring()));
        if (!image.isNull()) {
            draw_asset_cache_.emplace(
                alias, image.convertToFormat(QImage::Format_ARGB32_Premultiplied));
        }
    }

    void draw_alias(QPainter& painter, const std::string_view alias) const {
        const auto* record = state_.asset_catalog.catalog.find(alias);
        if (record == nullptr) {
            return;
        }
        const auto image = draw_asset_cache_.find(std::string(alias));
        if (image == draw_asset_cache_.end()) {
            return;
        }
        painter.drawImage(record->origin_x, record->origin_y, image->second);
    }

    void draw_title(QPainter& painter) const {
        draw_alias(painter, "title.background");
        // The seven circuit resources are retained independently in the
        // catalog. Their source cadence is not yet connected to this host,
        // so the Qt shell does not invent a presentation timer here.
        draw_alias(painter, "title.circuit.background.0");
        draw_alias(painter, "title.logo");
        draw_alias(painter, "title.text");
    }

    void draw_menu_overlay(QPainter& painter) const {
        painter.setPen(QColor(255, 255, 255));
        painter.setFont(QFont(QStringLiteral("Courier New"), 10,
                              QFont::Bold));

        if (state_.phase == StartupIntegrationPhase::start_options) {
            painter.drawText(90, 90, QStringLiteral("NEW GAME"));
            painter.drawText(90, 112, QStringLiteral("LOAD GAME"));
            const int y = state_.start_options.selected ==
                                  StartOptionsChoice::new_game
                              ? 86
                              : 108;
            draw_cursor(painter, 72, y);
            painter.drawText(68, 184,
                             QStringLiteral("Z/X/C OR ENTER: SELECT"));
            return;
        }

        if (state_.phase == StartupIntegrationPhase::save_load) {
            painter.drawText(84, 86, QStringLiteral("SAVE SLOT 1"));
            painter.drawText(84, 108, QStringLiteral("SAVE SLOT 2"));
            const int y = state_.save_load.selected == SaveSlotChoice::slot_one
                              ? 82
                              : 104;
            draw_cursor(painter, 66, y);
            painter.drawText(64, 184, QStringLiteral("NO SAVE DATA"));
            return;
        }

        painter.drawText(82, 78, QStringLiteral("SAMURAI"));
        painter.drawText(82, 100, QStringLiteral("DECKER"));
        painter.drawText(82, 122, QStringLiteral("GATOR SHAMAN"));
        const int y = 74 +
                      22 * static_cast<int>(state_.joshua_archetype.selected);
        draw_cursor(painter, 64, y);
        painter.drawText(64, 184, QStringLiteral("Z/X/C OR ENTER: SELECT"));
    }

    void draw_cursor(QPainter& painter, const int x, const int y) const {
        const auto* record =
            state_.asset_catalog.catalog.find("menu.startup.cursor");
        const auto image = draw_asset_cache_.find("menu.startup.cursor");
        if (record != nullptr && image != draw_asset_cache_.end()) {
            painter.drawImage(x + record->origin_x, y + record->origin_y,
                              image->second);
        } else {
            painter.drawText(x, y + 10, QStringLiteral(">"));
        }
    }

    std::filesystem::path asset_root_;
    StartupIntegrationState state_;
    StartupIntegrationCallbacks startup_callbacks_;
    std::map<std::string, QImage> draw_asset_cache_;

    std::uint8_t logo_strobe_phase_ = 0;
    JoshuaArchetypeChoice selected_archetype_ =
        JoshuaArchetypeChoice::samurai;
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
};

} // namespace

int main(int argc, char** argv) {
    try {
        for (int index = 1; index < argc; ++index) {
            if (std::string(argv[index]) == "--help") {
                std::cout
                    << "usage: shadowrun_ghidra_conversion_qt"
                       " [--assets <decompilation/assets>]\n";
                return 0;
            }
        }
        const auto asset_root = find_asset_root(argc, argv);
        if (asset_root.empty()) {
            std::cerr << "missing direct asset root; pass --assets "
                         "<decompilation/assets>\n";
            return 2;
        }

        QApplication application(argc, argv);
        GhidraConversionWindow window(asset_root);
        window.show();
        return application.exec();
    } catch (const std::exception& error) {
        std::cerr << "shadowrun_ghidra_conversion_qt: " << error.what()
                  << '\n';
        return 1;
    }
}

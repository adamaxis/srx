#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <algorithm>
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
        configure_title_loop_callbacks();
        title_timer_.setInterval(1000 / 60);
        QObject::connect(&title_timer_, &QTimer::timeout, this, [this] {
            advance_title_frame();
        });
        title_timer_.start();
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
        } else if (state_.phase == StartupIntegrationPhase::failed) {
            draw_alias(scene_painter, "menu.startup.background");
            draw_conversion_boundary_error(scene_painter);
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
        startup_callbacks_.pre_title.resources.write_vdp_grid = [this] {
            note_presentation_boundary("pre_title.write_vdp_grid");
        };
        startup_callbacks_.pre_title.resources.reset_vdp_upload_state =
            [this] {
                note_presentation_boundary(
                    "pre_title.reset_vdp_upload_state");
            };

        startup_callbacks_.pre_title.animation.build_title_sprite_records =
            [this] {
                ++title_sprite_record_builds_;
                note_presentation_boundary(
                    "pre_title.build_title_sprite_records");
            };
        startup_callbacks_.pre_title.animation.publish_logo_strobe_phase =
            [this](const std::uint8_t phase) {
                logo_strobe_phase_ = phase;
                note_presentation_boundary(
                    "pre_title.publish_logo_strobe_phase");
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
        startup_callbacks_.pre_title.service_audio_variant =
            [this] {
                ++audio_variant_ticks_;
                note_presentation_boundary("pre_title.service_audio_variant");
            };
        startup_callbacks_.pre_title.service_audio =
            [this] {
                ++audio_ticks_;
                note_presentation_boundary("pre_title.service_audio");
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
                save_load_boundary_unresolved_ = true;
                return SaveLoadAttempt::unresolved;
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
        startup_callbacks_.phase_changed =
            [this](const StartupIntegrationPhase phase) {
                setWindowTitle(phase_title(phase));
            };
        startup_callbacks_.field_ready = [this] { update(); };
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

    void advance_title_frame() {
        if (state_.phase != StartupIntegrationPhase::title_ready) {
            return;
        }
        const auto result =
            advance_title_loop_frame(title_loop_, title_loop_callbacks_);
        if (result == TitleLoopFrameResult::start_requested) {
            enter_start_options_integration(state_, startup_callbacks_);
        }
        setWindowTitle(phase_title(state_.phase));
        update();
    }

    void note_presentation_boundary(const std::string_view boundary) {
        presentation_boundaries_.emplace_back(boundary);
    }

    void load_direct_title_assets() {
        draw_asset_cache_.clear();
        for (const auto& alias : {
                 std::string("title.background"),
                 std::string("title.logo"),
                 std::string("title.text"),
                 std::string("menu.startup.background"),
                 std::string("menu.startup.font"),
                 std::string("menu.startup.cursor")}) {
            cache_asset(alias);
        }
        for (int index = 0; index < 7; ++index) {
            cache_asset("title.circuit.background." + std::to_string(index));
        }
        for (int index = 0; index < 6; ++index) {
            cache_asset("title.sprite." + std::to_string(index));
        }
    }

    void cache_asset(const std::string& alias) {
        const auto* record = state_.asset_catalog.catalog.find(alias);
        if (record == nullptr) {
            return;
        }
        QImage image(QString::fromStdWString(record->absolute_path.wstring()));
        if (!image.isNull()) {
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            if (alias == "title.logo" || alias == "title.text" ||
                alias == "menu.startup.cursor" ||
                alias.rfind("title.sprite.", 0) == 0) {
                for (int y = 0; y < image.height(); ++y) {
                    for (int x = 0; x < image.width(); ++x) {
                        auto pixel = image.pixelColor(x, y);
                        if (pixel.red() == 0 && pixel.green() == 0 &&
                            pixel.blue() == 0) {
                            pixel.setAlpha(0);
                            image.setPixelColor(x, y, pixel);
                        }
                    }
                }
            }
            draw_asset_cache_.emplace(
                alias, std::move(image));
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

    void draw_menu_overlay(QPainter& painter) const {
        painter.setPen(QColor(255, 255, 255));
        painter.setFont(QFont(QStringLiteral("Courier New"), 10,
                              QFont::Bold));

        if (state_.phase == StartupIntegrationPhase::start_options) {
            painter.drawText(
                90, 90,
                QString::fromUtf8(start_options_choice_name(
                    StartOptionsChoice::new_game)));
            painter.drawText(
                90, 112,
                QString::fromUtf8(start_options_choice_name(
                    StartOptionsChoice::load_game)));
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
            painter.drawText(
                84, 86,
                QString::fromUtf8(save_slot_choice_name(
                    SaveSlotChoice::slot_one)));
            painter.drawText(
                84, 108,
                QString::fromUtf8(save_slot_choice_name(
                    SaveSlotChoice::slot_two)));
            const int y = state_.save_load.selected == SaveSlotChoice::slot_one
                              ? 82
                              : 104;
            draw_cursor(painter, 66, y);
            if (state_.save_load.message == SaveLoadMessage::no_save_data) {
                painter.drawText(64, 184,
                                 QStringLiteral("NO SAVE DATA"));
            }
            return;
        }

        painter.drawText(
            82, 78,
            QString::fromUtf8(joshua_archetype_choice_name(
                JoshuaArchetypeChoice::samurai)));
        painter.drawText(
            82, 100,
            QString::fromUtf8(joshua_archetype_choice_name(
                JoshuaArchetypeChoice::decker)));
        painter.drawText(
            82, 122,
            QString::fromUtf8(joshua_archetype_choice_name(
                JoshuaArchetypeChoice::gator_shaman)));
        const int y = 74 +
                      22 * static_cast<int>(state_.joshua_archetype.selected);
        draw_cursor(painter, 64, y);
        painter.drawText(64, 184, QStringLiteral("Z/X/C OR ENTER: SELECT"));
    }

    void draw_conversion_boundary_error(QPainter& painter) const {
        painter.setPen(QColor(255, 220, 80));
        painter.setFont(QFont(QStringLiteral("Courier New"), 9,
                              QFont::Bold));
        painter.drawText(42, 92, QStringLiteral("CONVERSION BOUNDARY"));
        painter.drawText(
            42, 112,
            save_load_boundary_unresolved_
                ? QStringLiteral("SAVE SLOT DATA NOT AVAILABLE")
                : QStringLiteral("NOT YET RESOLVED"));
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(42, 184,
                         QStringLiteral("SOURCE DATA REQUIRED"));
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
    TitleLoopState title_loop_;
    TitleLoopCallbacks title_loop_callbacks_;
    QTimer title_timer_;
    std::map<std::string, QImage> draw_asset_cache_;

    std::uint8_t logo_strobe_phase_ = 0;
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

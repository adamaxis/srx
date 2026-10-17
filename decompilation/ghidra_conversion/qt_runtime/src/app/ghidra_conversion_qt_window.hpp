#pragma once

#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/debug/frame_debugger.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_service_sequence_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"
#include "field_scene_runtime.hpp"
#include "audio_playback.hpp"
#include "frame_debug_server.hpp"
#include "genesis_image_compositor.hpp"

#include <QApplication>
#include <QColor>
#include <QElapsedTimer>
#include <QImage>
#include <QKeyEvent>
#include <QFont>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <system_error>
#include <tuple>
#ifdef SHADOWRUN_QT_QA_DRIVER
#include <type_traits>
#endif
#include <utility>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;
using namespace shadowrun::ghidra_conversion::qt_runtime;

constexpr int kSceneWidth = 320;
constexpr int kSceneHeight = 224;
constexpr int kSceneScale = 3;
constexpr std::uint8_t kTitleStartButton = 0x80U;
constexpr std::size_t kTitleCircuitPhaseCount = 7U;
constexpr std::size_t kTitleCircuitPaletteRevealPassCount = 3U;

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

// 0x1A9E's title break is driven by the latched Genesis Start bit. Startup
// menus still map these keys to A; the title loop has its own input contract.
std::uint8_t sampled_title_byte(const int key) {
    switch (key) {
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return kTitleStartButton;
    default:
        return sampled_menu_byte(key);
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

#ifdef SHADOWRUN_QT_QA_DRIVER
#include "../../tests/qa/ghidra_conversion_qt_qa_support.inc"
#endif

class GhidraConversionWindow : public QWidget {
public:
    explicit GhidraConversionWindow(std::filesystem::path asset_root,
                                    debug::InputLogConfig input_log = {},
                                    const bool frame_debug_controls = false,
                                    const bool headless_replay = false,
                                    const std::optional<std::uint16_t> debug_port =
                                        std::nullopt,
                                    QWidget* parent = nullptr)
        : QWidget(parent),
          asset_root_(std::move(asset_root)),
          input_log_(std::move(input_log)),
          frame_stepper_(frame_debug_controls),
          audio_playback_(headless_replay
                              ? nullptr
                              : std::make_unique<AudioPlaybackService>(
                                    [this](const std::string_view alias)
                                        -> std::filesystem::path {
                                        const auto* record =
                                            state_.asset_catalog.catalog.find(
                                                std::string(alias));
                                        return record == nullptr
                                                   ? std::filesystem::path{}
                                                   : record->absolute_path;
                                    })),
          field_scene_runtime_([this](const std::string_view boundary) {
              record_debug_source_event(boundary);
          }, [this](const std::uint16_t event) {
              if (audio_playback_) {
                  audio_playback_->play_event(event);
              }
          }, [this](const std::uint16_t sound) {
              if (audio_playback_) {
                  audio_playback_->stop_resource(sound);
              }
          }) {
        debug_event_queue_enabled_ = debug_port.has_value();
        setWindowTitle(QStringLiteral("Shadowrun - Ghidra Conversion Qt"));
        setFixedSize(kSceneWidth * kSceneScale, kSceneHeight * kSceneScale);
        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_OpaquePaintEvent);

        configure_startup_callbacks();
#ifdef SHADOWRUN_QT_QA_DRIVER
        install_qa_startup_observers();
#endif
        const auto result = begin_startup_integration(
            asset_root_, state_, startup_callbacks_);
        if (result != StartupIntegrationStartResult::boot_in_progress) {
            throw std::runtime_error(
                "converted startup did not enter pre_title; phase=" +
                std::string(startup_integration_phase_name(state_.phase)));
        }
        load_startup_script_content();

        load_direct_title_assets();
        configure_title_loop_callbacks();
#ifdef SHADOWRUN_QT_QA_DRIVER
        install_qa_title_observers();
#endif
        host_clock_.start();
        host_frame_timer_.setTimerType(Qt::PreciseTimer);
        host_frame_timer_.setInterval(16);
        QObject::connect(&host_frame_timer_, &QTimer::timeout, this, [this] {
            on_host_frame_timeout();
        });
        host_frame_timer_.start();
        update_window_title();
        if (debug_port.has_value()) {
            frame_stepper_.pause();
            debug_server_ = std::make_unique<FrameDebugServer>(
                *debug_port, frame_stepper_,
                [this] { return debug_inspection_fields(); },
                [this] { return source_frame_index_; },
                [this] {
                    return QString::fromLatin1(
                        startup_integration_phase_name(state_.phase));
                },
                [this](const int key, const bool pressed) {
                    pending_remote_keys_.push_back({key, pressed});
                },
                [this] {
                    QStringList events;
                    events.reserve(static_cast<qsizetype>(
                        pending_debug_events_.size()));
                    for (const auto& name : pending_debug_events_) {
                        events.push_back(QString::fromStdString(name));
                    }
                    pending_debug_events_.clear();
                    return events;
                },
                [this](const QString& path, QString* error) {
                    const auto frame = render_scene();
                    if (!frame.save(path)) {
                        if (error != nullptr) {
                            *error = QStringLiteral(
                                "could not save the rendered frame to %1")
                                         .arg(path);
                        }
                        return false;
                    }
                    return true;
                });
        }
        if (debug_event_queue_enabled_ || input_log_.recording()) {
            field_scene_runtime_.set_actor_heartbeat_observer(
                [this](const std::string_view actor_id,
                       const std::string_view profile_id,
                       const scripting::ScriptActorHeartbeatPhase phase) {
                    std::ostringstream event;
                    event << "lua.heartbeat."
                          << (phase == scripting::ScriptActorHeartbeatPhase::behavior
                                  ? "behavior"
                                  : "render")
                          << ".completed.profile=" << profile_id
                          << ".actor=" << actor_id;
                    record_debug_source_event(event.str());
                });
            set_source_random_observer(
                [this](const int kind, const std::uint16_t bound,
                       const std::int32_t result, const std::uint16_t seed) {
                    static constexpr const char* kRandomKindNames[] = {
                        "raw", "bounded", "dice"};
                    std::ostringstream event;
                    event << "rng.";
                    if (kind >= 0 && kind < 3) {
                        event << kRandomKindNames[kind];
                    } else {
                        event << "kind" << kind;
                    }
                    event << ".bound=" << bound << ".result=" << result
                          << ".seed=0x" << std::hex << std::setw(4)
                          << std::setfill('0') << seed;
                    record_debug_source_event(event.str());
                });
            source_random_observer_installed_ = true;
        }
    }

    // Runs a recorded session without presenting a widget or opening the host
    // audio device. The same source-frame and input handlers are used as live
    // replay; only the wall-clock pump and presentation are skipped.
    bool run_headless_replay(
        const std::filesystem::path& frame_dump,
        const std::optional<std::uint64_t> replay_to_frame = std::nullopt,
        std::string* error = nullptr) {
        if (!input_log_.replaying()) {
            if (error != nullptr) {
                *error = "headless replay requires --replay-input";
            }
            return false;
        }
        const auto target_frame =
            replay_to_frame.value_or(input_log_.recorded_frame_count());
        if (target_frame > input_log_.recorded_frame_count()) {
            if (error != nullptr) {
                *error = "requested replay frame exceeds the recording length";
            }
            return false;
        }
        host_frame_timer_.stop();
        while (source_frame_index_ < target_frame) {
            apply_recorded_key_inputs();
            const auto completed_frame = source_frame_index_;
            advance_one_source_frame();
            ++source_frame_index_;
            if (input_log_.has_seed_history()) {
                const auto expected =
                    input_log_.expected_seed_after_frame(completed_frame);
                if (!expected.has_value() ||
                    *expected != source_random_seed()) {
                    replay_diverged_ = true;
                    if (error != nullptr) {
                        std::ostringstream message;
                        message << "replay diverged after source frame "
                                << (completed_frame + 1U)
                                << ": expected shared ROM seed 0x" << std::hex
                                << expected.value_or(0U) << ", got 0x"
                                << source_random_seed();
                        *error = message.str();
                    }
                    return false;
                }
            }
            if (input_log_.has_fingerprint_history()) {
                const auto expected = input_log_.expected_fingerprint_after_frame(
                    completed_frame);
                const auto actual = current_debug_inspection_fingerprint();
                if (!expected.has_value() || *expected != actual) {
                    replay_diverged_ = true;
                    if (error != nullptr) {
                        std::ostringstream message;
                        message << "replay inspection fingerprint diverged after source frame "
                                << (completed_frame + 1U) << ": expected 0x"
                                << std::hex << expected.value_or(0U)
                                << ", got 0x" << actual;
                        *error = message.str();
                    }
                    return false;
                }
            }
        }

        // Key-up events can be recorded on the frame boundary after the final
        // completed source frame. Preserve that boundary just as the live
        // timer path does. A seek stops before the next frame's input boundary.
        if (target_frame == input_log_.recorded_frame_count()) {
            apply_recorded_key_inputs();
        }
        if (!frame_dump.empty()) {
            const auto frame = render_scene();
            if (!frame.save(QString::fromStdWString(frame_dump.wstring()))) {
                if (error != nullptr) {
                    *error = "could not save replay frame: " +
                             frame_dump.string();
                }
                return false;
            }
        }
        return true;
    }

    std::uint64_t source_frame_count_for_debug() const noexcept {
        return source_frame_index_;
    }

    void resume_debugger_after_replay_seek() {
        pending_debug_events_.clear();
        host_clock_.restart();
        host_frame_deadline_ns_ = 0;
        host_frame_timer_.start();
        update();
    }

    ~GhidraConversionWindow() override {
        if (source_random_observer_installed_) {
            set_source_random_observer({});
        }
        std::string error;
        if (!input_log_.finish(&error)) {
            std::cerr << "input session log: " << error << '\n';
        }
    }

#ifdef SHADOWRUN_QT_QA_DRIVER
#include "../../tests/qa/ghidra_conversion_qt_qa_window_methods.inc"
#endif

protected:
    void paintEvent(QPaintEvent*) override {
        const auto scene = render_scene();
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawImage(rect(), scene);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (!applying_remote_key_ && !event->isAutoRepeat() &&
            frame_stepper_.enabled()) {
            if (event->key() == Qt::Key_Pause) {
                frame_stepper_.toggle_pause();
                update_window_title();
                update();
                event->accept();
                return;
            }
            if (frame_stepper_.paused()) {
                if (event->key() == Qt::Key_Period) {
                    frame_stepper_.queue_step(
                        (event->modifiers() & Qt::ShiftModifier) != 0
                            ? 60U
                            : 1U);
                }
                event->accept();
                return;
            }
        }
        if (frame_stepper_.paused() && !applying_remote_key_) {
            event->accept();
            return;
        }
        if (state_.phase == StartupIntegrationPhase::title_ready &&
            (title_loop_.exit_palette_fade.active ||
             title_loop_.exit_palette_fade.complete ||
             title_mode_entry_service_.active ||
             title_mode_entry_service_.complete)) {
            // 0x131C waits and latches the existing controller state; it does
            // not sample another title input while the palette is fading.
            event->accept();
            return;
        }
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
                record_key_input(*event, true);
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
                record_key_input(*event, true);
                update();
                event->accept();
                return;
            }
        }

        const auto sample =
            state_.phase == StartupIntegrationPhase::title_ready ||
                    state_.phase == StartupIntegrationPhase::opening_intro
                ? sampled_title_byte(event->key())
                : sampled_menu_byte(event->key());
        if (sample == 0U) {
            QWidget::keyPressEvent(event);
            return;
        }

        if (state_.phase == StartupIntegrationPhase::title_ready) {
            // 0x195C publishes newly pressed controller bits in the high
            // byte. The ROM title loop exits on its latched Start bit.
            pending_title_controller_word_ = static_cast<std::uint16_t>(
                pending_title_controller_word_ |
                (static_cast<std::uint16_t>(sample) << 8U));
        } else if (state_.phase == StartupIntegrationPhase::opening_intro) {
            // During the 0x59C5E movie, the ROM treats Start specially in
            // 0x0BF2. Keep A/B/C from becoming a second skip command.
            if (sample == kTitleStartButton) {
                pending_opening_controller_word_ = static_cast<std::uint16_t>(
                    pending_opening_controller_word_ |
                    (static_cast<std::uint16_t>(sample) << 8U));
            }
        } else if (state_.phase ==
                   StartupIntegrationPhase::start_options_shell) {
            // The ROM does not sample the menu until after the shell's
            // VBlank, but a button held through that one-frame reveal is
            // sampled on the first menu pass. Retain its current host state.
            pending_start_options_controller_byte_ =
                static_cast<std::uint8_t>(
                    pending_start_options_controller_byte_ | sample);
        } else if (state_.phase == StartupIntegrationPhase::start_options ||
                   state_.phase == StartupIntegrationPhase::save_load ||
                   state_.phase == StartupIntegrationPhase::joshua_archetype) {
            advance_startup_integration(state_, sample, startup_callbacks_);
        }

        update_window_title();
        record_key_input(*event, true);
        update();
        event->accept();
    }

    void keyReleaseEvent(QKeyEvent* event) override {
        if (frame_stepper_.paused() && !applying_remote_key_) {
            event->accept();
            return;
        }
        if (state_.phase == StartupIntegrationPhase::start_options_shell &&
            !event->isAutoRepeat()) {
            const auto released = sampled_menu_byte(event->key());
            if (released != 0U) {
                pending_start_options_controller_byte_ =
                    static_cast<std::uint8_t>(
                        pending_start_options_controller_byte_ &
                        static_cast<std::uint8_t>(~released));
                record_key_input(*event, false);
                event->accept();
                return;
            }
        }
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
                record_key_input(*event, false);
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
    struct PendingRemoteKey {
        int key = 0;
        bool pressed = false;
    };

    QJsonObject debug_inspection_fields() const {
        auto fields = field_scene_runtime_.debug_inspection_fields();
        const auto phase = startup_integration_phase_name(state_.phase);
        fields.insert(QStringLiteral("app.phase"),
                      QString::fromLatin1(phase));
        fields.insert(QStringLiteral("app.source_frame"),
                      static_cast<double>(source_frame_index_));
        fields.insert(QStringLiteral("app.random_seed"),
                      source_random_seed());
        fields.insert(QStringLiteral("app.paused"), frame_stepper_.paused());
        fields.insert(QStringLiteral("app.replay_diverged"),
                      replay_diverged_);
        return fields;
    }

    std::uint64_t current_debug_inspection_fingerprint() const {
        return debug_inspection_fingerprint(debug_inspection_fields());
    }

    void apply_pending_remote_keys() {
        auto pending = std::move(pending_remote_keys_);
        pending_remote_keys_.clear();
        for (const auto& input : pending) {
            QKeyEvent event(input.pressed ? QEvent::KeyPress
                                          : QEvent::KeyRelease,
                            input.key, Qt::NoModifier);
            applying_remote_key_ = true;
            QApplication::sendEvent(this, &event);
            applying_remote_key_ = false;
        }
    }

    void update_window_title() {
        auto title = phase_title(state_.phase);
        if (frame_stepper_.paused()) {
            title += QStringLiteral(" [PAUSED]");
        }
        if (replay_diverged_) {
            title += QStringLiteral(" [REPLAY DIVERGED]");
        }
        setWindowTitle(title);
    }

    void record_key_input(const QKeyEvent& event, const bool pressed) {
        if (!replaying_input_) {
            input_log_.record_event(
                source_frame_index_, event.key(),
                static_cast<std::uint32_t>(event.modifiers()), pressed);
        }
    }

    void record_debug_source_event(const std::string_view name) {
        input_log_.record_source_event(source_frame_index_, name);
        if (debug_event_queue_enabled_) {
            pending_debug_events_.emplace_back(name);
        }
    }

    bool apply_recorded_key_inputs() {
        if (!input_log_.replaying()) {
            return false;
        }
        const auto events = input_log_.take_events_for_frame(source_frame_index_);
        for (const auto& input : events) {
            QKeyEvent event(input.pressed ? QEvent::KeyPress : QEvent::KeyRelease,
                            input.key,
                            static_cast<Qt::KeyboardModifiers>(input.modifiers));
            replaying_input_ = true;
            QApplication::sendEvent(this, &event);
            replaying_input_ = false;
        }
        return !events.empty();
    }

    void load_startup_script_content() {
        const auto script_directory =
            asset_root_.parent_path() / "gameplay" / "scripts";
        const auto text_file = asset_root_ / "dialogue" / "script-text.lua";
        if (!startup_scripts_.load_package(script_directory, text_file)) {
            throw std::runtime_error(
                "startup gameplay scripts did not load: " +
                startup_scripts_.failure());
        }

        const auto start_options = startup_scripts_.menu(
            "startup", "options", "startup", "options");
        const auto save_slots = startup_scripts_.menu(
            "startup", "save_slots", "startup", "save_slots");
        const auto archetypes = startup_scripts_.menu(
            "startup", "archetypes", "startup", "archetypes");
        const auto archetype_portraits =
            startup_scripts_.string_map("startup", "archetype_portraits");
        const auto text_keys =
            startup_scripts_.string_map("startup", "text_keys");
        const auto opening_text_keys =
            startup_scripts_.string_map("startup", "opening_text_keys");
        const auto fixed_text_count = [](const auto& lines) {
            std::size_t count = 0U;
            for (const auto& line : lines) {
                if (line.choice_index == 0xFFU && !line.text_id.empty()) {
                    ++count;
                }
            }
            return count;
        };
        const auto expected_text_key_count =
            fixed_text_count(kStartOptionsLayout) +
            fixed_text_count(kJoshuaArchetypeLayout) +
            fixed_text_count(kNoSaveDataLayout) +
            fixed_text_count(kSaveSuccessLayout);
        const auto validate_choices = [this](
            const std::optional<scripting::ScriptMenuDefinition>& menu,
            const std::initializer_list<std::string_view> ids) {
            if (!menu.has_value() || menu->choices.size() != ids.size()) {
                return false;
            }
            std::size_t index = 0U;
            for (const auto id : ids) {
                const auto& choice = menu->choices[index++];
                if (choice.id != id ||
                    startup_scripts_.dialogue_text(choice.label_key) ==
                        nullptr) {
                    return false;
                }
            }
            return true;
        };
        if (!validate_choices(start_options, {"new_game", "load_game"}) ||
            !validate_choices(save_slots, {"slot_one", "slot_two"}) ||
            !validate_choices(archetypes,
                              {"samurai", "decker", "gator_shaman"}) ||
            !archetype_portraits.has_value() || !archetypes.has_value() ||
            archetype_portraits->size() != archetypes->choices.size() ||
            !text_keys.has_value() ||
            text_keys->size() != expected_text_key_count) {
            throw std::runtime_error(
                "startup Lua menus, portrait bindings, or text bindings "
                "are incomplete: " +
                startup_scripts_.failure());
        }

        for (const auto& choice : archetypes->choices) {
            const auto portrait = archetype_portraits->find(choice.id);
            if (portrait == archetype_portraits->end() ||
                state_.asset_catalog.catalog.find(portrait->second) ==
                    nullptr) {
                throw std::runtime_error(
                    "startup Lua archetype portrait is not an extracted "
                    "asset: " +
                    choice.id);
            }
        }
        startup_archetype_portraits_ = *archetype_portraits;

        const auto validate_text_layout = [this, &text_keys](const auto& lines) {
            for (const auto& line : lines) {
                if (line.choice_index != 0xFFU) {
                    continue;
                }
                const auto key = text_keys->find(std::string(line.text_id));
                if (key == text_keys->end() ||
                    startup_scripts_.dialogue_text(key->second) == nullptr) {
                    return false;
                }
            }
            return true;
        };
        if (!validate_text_layout(kStartOptionsLayout) ||
            !validate_text_layout(kJoshuaArchetypeLayout) ||
            !validate_text_layout(kNoSaveDataLayout) ||
            !validate_text_layout(kSaveSuccessLayout)) {
            throw std::runtime_error(
                "startup Lua text bindings do not cover the ROM layout: " +
                startup_scripts_.failure());
        }
        constexpr auto kOpeningStoryTextCount = static_cast<std::size_t>(
            OpeningIntroTextAsset::blank);
        if (!opening_text_keys.has_value() ||
            opening_text_keys->size() != kOpeningStoryTextCount) {
            throw std::runtime_error(
                "startup Lua opening-story text bindings are incomplete: " +
                startup_scripts_.failure());
        }
        for (std::size_t index = 0U; index < kOpeningStoryTextCount;
             ++index) {
            const auto text_id = opening_intro_text_id(
                static_cast<OpeningIntroTextAsset>(index));
            const auto key = opening_text_keys->find(std::string(text_id));
            const auto* text = key == opening_text_keys->end()
                                   ? nullptr
                                   : startup_scripts_.dialogue_text(
                                         key->second);
            if (text_id.empty() || key == opening_text_keys->end() ||
                key->second.rfind("opening.story.", 0U) != 0U ||
                text == nullptr || text->empty()) {
                throw std::runtime_error(
                    "startup Lua opening-story text is unresolved: " +
                    std::string(text_id));
            }
        }
        for (const auto& [text_id, key] : *text_keys) {
            if (text_id.empty() || key.empty() ||
                startup_scripts_.dialogue_text(key) == nullptr) {
                throw std::runtime_error(
                    "startup Lua text key is unresolved: " + text_id);
            }
        }
        startup_options_menu_ = *start_options;
        startup_save_slots_menu_ = *save_slots;
        startup_archetypes_menu_ = *archetypes;
        startup_text_keys_ = *text_keys;
        opening_text_keys_ = *opening_text_keys;
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
        } else if (state_.phase ==
                   StartupIntegrationPhase::start_options_shell) {
            draw_alias(scene_painter, "menu.startup.background");
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
        if (state_.phase == StartupIntegrationPhase::title_ready &&
            title_loop_.exit_palette_fade.passes_completed != 0U) {
            return transition_genesis_cram_image(
                scene, title_loop_.exit_palette_fade.passes_completed, true);
        }
        if (state_.phase == StartupIntegrationPhase::opening_intro &&
            state_.opening_intro.scene_palette_weight != 255U) {
            return scale_genesis_image_rgb(
                scene, state_.opening_intro.scene_palette_weight);
        }
        return scene;
    }

    void reset_startup_presentation_state() {
        if (audio_playback_) {
            audio_playback_->begin_source_sequence();
        }
        title_loop_ = TitleLoopState{};
        title_mode_entry_service_ = TitleModeEntryServiceState{};
        title_idle_restart_fade_ = TitleIdleRestartFadeState{};
        logo_strobe_phase_ = 0U;
        logo_strobe_palette_.fill(0U);
        pre_title_sprite_records_.clear();
#ifdef SHADOWRUN_QT_QA_DRIVER
        pre_title_audio_requests_.clear();
#endif
        opening_scene_ = OpeningIntroScene::seattle;
        opening_stage_ = OpeningIntroStage::not_started;
        opening_sprites_.clear();
        opening_wilderness_map_left_.reset();
        opening_wilderness_map_center_.reset();
        opening_wilderness_map_right_.reset();
        opening_wilderness_overlay_.reset();
        opening_wilderness_patch_.reset();
        opening_wilderness_final_overlay_.reset();
        opening_text_.clear();
        selected_archetype_ = JoshuaArchetypeChoice::samurai;
        start_options_cursor_ = StartOptionsChoice::new_game;
        save_load_cursor_ = SaveSlotChoice::slot_one;
        joshua_cursor_ = JoshuaArchetypeChoice::samurai;
        joshua_preview_ = JoshuaArchetypeChoice::samurai;
        save_load_requested_slot_ = SaveSlotChoice::slot_one;
        save_load_message_ = SaveLoadMessage::none;
        save_load_boundary_unresolved_ = false;
        title_prompt_visible_ = false;
        title_copyright_visible_ = false;
        title_completion_palette_copied_ = false;
        title_prompt_variant_ = TitlePromptVariant::VariantA;
        title_circuit_phase_ = 0U;
        title_logo_columns_ = 0U;
        title_sprite_phase_ = 0U;
        title_sprite_x_ = 0x98;
        title_controller_events_ = 0U;
        pending_title_controller_word_ = 0U;
        pending_start_options_controller_byte_ = 0U;
        field_controller_nibble_ = 0U;
        field_controller_port_byte_ = 0U;
    }

    void configure_startup_callbacks() {
        const auto no_op = [](auto&&...) {};
        startup_callbacks_.initialize_display = [this] {
            reset_startup_presentation_state();
        };
        startup_callbacks_.vdp_ready = {
            no_op,
            no_op,
            no_op,
        };

        startup_callbacks_.pre_title.resources.clear_plane_a = no_op;
        startup_callbacks_.pre_title.resources.clear_plane_b = no_op;
        startup_callbacks_.pre_title.resources.prepare_title_resources = no_op;
        startup_callbacks_.pre_title.resources.write_resource_stream = no_op;
        startup_callbacks_.pre_title.resources.refresh_field_resource_streams =
            no_op;
        startup_callbacks_.pre_title.resources.decode_and_upload_source_stream =
            no_op;
        startup_callbacks_.pre_title.resources.advance_title_palette_fade =
            no_op;
        startup_callbacks_.pre_title.resources.write_pre_title_grid = no_op;
        startup_callbacks_.pre_title.resources.reset_vdp_upload_state = no_op;

        startup_callbacks_.pre_title.animation.publish_sprite_batch =
            [this](const PreTitleSpriteBatch& batch) {
                pre_title_sprite_records_ = batch.records;
            };
        startup_callbacks_.pre_title.animation.clear_last_sprite_sequence =
            [this] {
                if (!pre_title_sprite_records_.empty()) {
                    pre_title_sprite_records_.back().sequence = 0U;
                }
            };
        startup_callbacks_.pre_title.animation.publish_logo_strobe_palette =
            [this](const PreTitleLogoStrobePalette& palette) {
                logo_strobe_phase_ = palette.phase;
                logo_strobe_palette_ = palette.color_words;
            };
        startup_callbacks_.pre_title.animation.step_entity_color_words = no_op;
        startup_callbacks_.pre_title.wait_for_vblank = [this] {
            wait_for_vblank(
                state_.frame,
                [](const std::uint16_t previous) {
                    return static_cast<std::uint16_t>(previous + 1U);
                });
        };
        startup_callbacks_.pre_title.write_horizontal_scroll_stream = no_op;
        startup_callbacks_.pre_title.publish_audio_request =
            [this](const PreTitleAudioRequest& request) {
                // The 0x5D746 source path sends 0x16 through 0x530D2 as
                // FF 12 16 (stop resource 22). Its 0x5C request uses
                // 0x530DE's queue path, FF 10 5C (start resource 92). Keep
                // the separate start/stop operations. The sequence asset
                // contains all source-cadenced starts, so play it once.
                // Ordinary FF 10 use of 0x16 remains a separate start alias;
                // this callback must honor the direct FF 12 operation.
                if (request.service_path ==
                    PreTitleAudioServicePath::enabled_variant_transaction) {
                    if (audio_playback_) {
                        audio_playback_->stop_resource(request.command);
                    }
                } else if (request.command == 0x005CU) {
                    if (audio_playback_) {
                        audio_playback_->play_event_once_in_sequence(
                            request.command);
                    }
                }
            };
        startup_callbacks_.pre_title.sample_controller = no_op;
        startup_callbacks_.pre_title.latch_controller_events = no_op;
        startup_callbacks_.pre_title.step_palette_transition = no_op;

        startup_callbacks_.title_setup = {
            [this] {
                load_direct_title_assets();
            },
            no_op,
            [this] {
                cache_asset("menu.startup.font");
            },
            no_op,
        };

        startup_callbacks_.start_options = {
            no_op,
            no_op,
            no_op,
            no_op,
            no_op,
            no_op,
            [this](const StartOptionsChoice choice) {
                start_options_cursor_ = choice;
            },
        };
        startup_callbacks_.save_load = {
            no_op,
            no_op,
            [this](const SaveSlotChoice choice) {
                save_load_cursor_ = choice;
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
            },
            no_op,
        };
        startup_callbacks_.joshua_archetype = {
            no_op,
            no_op,
            [this](const JoshuaArchetypeChoice choice) {
                joshua_cursor_ = choice;
            },
            [this](const JoshuaArchetypeChoice choice) {
                joshua_preview_ = choice;
            },
            [this](const JoshuaArchetypeChoice choice) {
                selected_archetype_ = choice;
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
            // Preserve an edge sampled immediately before a child loop until
            // the ROM's later latch_controller_events pass consumes it.
            sampled_opening_controller_word_ |=
                pending_opening_controller_word_;
            pending_opening_controller_word_ = 0U;
        };
        startup_callbacks_.opening_intro.latch_controller_events = [this] {
            constexpr auto kStartEventMask = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(kTitleStartButton) << 8U);
            opening_start_event_latched_ =
                opening_start_event_latched_ ||
                (sampled_opening_controller_word_ & kStartEventMask) != 0U;
            sampled_opening_controller_word_ = 0U;
        };
        startup_callbacks_.opening_intro.prepare_mode_entry = [this] {
            // ROM 0x53126 sends FF 16 here; its Z80 handler clears all active
            // sound channels before the next opening-stage command.
            if (audio_playback_) {
                audio_playback_->stop_all_channels();
            }
        };
        startup_callbacks_.opening_intro.service_audio_command =
            [this](const std::uint16_t command) {
                if (audio_playback_) {
                    audio_playback_->play_event(command);
                }
            };
        startup_callbacks_.opening_intro.service_audio = no_op;
        startup_callbacks_.opening_intro.fade_palette_to_black = no_op;
        startup_callbacks_.opening_intro.advance_palette_fade = no_op;
        startup_callbacks_.opening_intro.enter_post_intro_mode = no_op;
        startup_callbacks_.opening_intro.clear_presentation = [this] {
            opening_sprites_.clear();
            opening_wilderness_map_left_.reset();
            opening_wilderness_map_center_.reset();
            opening_wilderness_map_right_.reset();
            opening_wilderness_overlay_.reset();
            opening_wilderness_patch_.reset();
            opening_wilderness_final_overlay_.reset();
            opening_text_.clear();
        };
        startup_callbacks_.opening_intro.begin_frame = [this] {
            opening_sprites_.clear();
        };
        startup_callbacks_.opening_intro.stage_changed =
            [this](const OpeningIntroStage stage) {
                opening_stage_ = stage;
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
        };
        startup_callbacks_.opening_intro.clear_wilderness_center_map = [this] {
            opening_wilderness_map_center_.reset();
        };
        startup_callbacks_.opening_intro.clear_wilderness_left_region =
            [this] {
                opening_wilderness_map_left_.reset();
                opening_wilderness_overlay_.reset();
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
                record_debug_source_event(boundary);
            };
        startup_callbacks_.opening_intro.stop_requested = [this] {
            return opening_start_event_latched_;
        };
        startup_callbacks_.opening_intro.clear_stop_request = [this] {
            opening_start_event_latched_ = false;
        };
        startup_callbacks_.phase_changed =
            [this](const StartupIntegrationPhase phase) {
                if (phase == StartupIntegrationPhase::opening_intro) {
                    pending_opening_controller_word_ = 0U;
                    sampled_opening_controller_word_ = 0U;
                    opening_start_event_latched_ = false;
                } else if (phase == StartupIntegrationPhase::field_ready) {
                    opening_start_event_latched_ = false;
                }
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
        const auto no_op = [](auto&&...) {};
        title_loop_callbacks_.wait_for_vblank = no_op;
        title_loop_callbacks_.sample_controller = [this] {
            const auto sample = pending_title_controller_word_;
            pending_title_controller_word_ = 0U;
            return sample;
        };
        title_loop_callbacks_.apply_text_action =
            [this](const TitleTextAction action) {
                if (action == TitleTextAction::ShowCopyright) {
                    draw_title_copyright([this](const auto&) {
                        title_copyright_visible_ = true;
                    });
                }
            };
        title_loop_callbacks_.request_audio_command =
            [this](const TitleAudioCommand command) {
                const auto event = static_cast<std::uint16_t>(command);
                if (command == TitleAudioCommand::LogoReveal) {
                    // The sequence asset represents all 19 source starts at
                    // their ROM cadence. Play that composite once instead of
                    // layering 19 copies of the complete render.
                    if (audio_playback_) {
                        audio_playback_->play_event_once_in_sequence(event);
                    }
                } else if (command == TitleAudioCommand::ThemeStart) {
                    // The ROM's command-0 wrapper stops then starts this same
                    // music resource before drawing the copyright text.
                    if (audio_playback_) {
                        audio_playback_->stop_resource(event);
                        audio_playback_->play_event(event);
                    }
                }
            };
        title_loop_callbacks_.publish_circuit_light_phase =
            [this](const std::uint8_t phase) { title_circuit_phase_ = phase; };
        title_loop_callbacks_.publish_animation_offset = no_op;
        title_loop_callbacks_.draw_prompt_variant =
            [this](const TitlePromptVariant variant) {
                title_prompt_variant_ = variant;
                const auto draw_prompt = [this](
                                             const TitleTextRenderRequest& request) {
                    title_prompt_visible_ =
                        request.asset == TitleTextAsset::prompt_variant_a;
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
        title_loop_callbacks_.reset_vdp_upload_state = no_op;
        title_loop_callbacks_.latch_palette_transition_events = no_op;
        title_loop_callbacks_.step_palette_transition = no_op;

        title_mode_entry_callbacks_.wait_for_vblank = no_op;
        title_mode_entry_callbacks_.run_audio_transaction = no_op;
        title_mode_entry_callbacks_.flush_audio_bus = [this] {
            // ROM 0x52FA0 sends FF 16, clearing the source sound channels.
            if (audio_playback_) {
                audio_playback_->stop_all_channels();
            }
        };
        title_mode_entry_callbacks_.service_audio_tick = no_op;
    }

    void on_host_frame_timeout() {
        if (replay_diverged_) {
            return;
        }
        const auto due_frames = consume_host_frames(
            host_frame_deadline_ns_, host_clock_.nsecsElapsed());
        const auto frames = frame_stepper_.frames_to_advance(due_frames);
        bool replay_input_applied = false;
        int advanced_frames = 0;
        for (std::uint32_t frame = 0; frame < frames; ++frame) {
            apply_pending_remote_keys();
            replay_input_applied = apply_recorded_key_inputs() ||
                                   replay_input_applied;
            if (input_log_.replaying() &&
                source_frame_index_ >= input_log_.recorded_frame_count()) {
                break;
            }
            const auto completed_frame = source_frame_index_;
            advance_one_source_frame();
            ++source_frame_index_;
            if (input_log_.replaying() && input_log_.has_seed_history()) {
                const auto expected =
                    input_log_.expected_seed_after_frame(completed_frame);
                if (!expected.has_value() ||
                    *expected != source_random_seed()) {
                    std::cerr << "input replay diverged after source frame "
                              << (completed_frame + 1U)
                              << ": expected shared ROM seed 0x" << std::hex
                              << expected.value_or(0U) << ", got 0x"
                              << source_random_seed() << std::dec << '\n';
                    replay_diverged_ = true;
                    update_window_title();
                }
            }
            if (input_log_.replaying() &&
                input_log_.has_fingerprint_history() && !replay_diverged_) {
                const auto expected = input_log_.expected_fingerprint_after_frame(
                    completed_frame);
                const auto actual = current_debug_inspection_fingerprint();
                if (!expected.has_value() || *expected != actual) {
                    std::cerr << "input replay inspection fingerprint diverged after source frame "
                              << (completed_frame + 1U) << ": expected 0x"
                              << std::hex << expected.value_or(0U)
                              << ", got 0x" << actual << std::dec << '\n';
                    replay_diverged_ = true;
                    update_window_title();
                }
            } else if (input_log_.recording()) {
                input_log_.record_completed_frame(
                    source_random_seed(),
                    current_debug_inspection_fingerprint());
            }
            ++advanced_frames;
            if (debug_server_ &&
                debug_server_->check_source_frame_breakpoints()) {
                break;
            }
            if (replay_diverged_) {
                break;
            }
        }
        if (input_log_.replaying() &&
            source_frame_index_ >= input_log_.recorded_frame_count()) {
            replay_input_applied = apply_recorded_key_inputs() ||
                                   replay_input_applied;
        }
        if (advanced_frames > 0 || replay_input_applied) {
            update();
        }
    }

    void advance_one_source_frame() {
        if (state_.phase == StartupIntegrationPhase::pre_title) {
            advance_startup_boot_frame(state_, startup_callbacks_);
            return;
        }
        if (state_.phase == StartupIntegrationPhase::start_options_shell) {
            const auto sampled = pending_start_options_controller_byte_;
            pending_start_options_controller_byte_ = 0U;
            advance_startup_integration(state_, sampled,
                                        startup_callbacks_);
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
        if (title_idle_restart_fade_.active) {
            if (advance_title_idle_restart_fade(
                    title_idle_restart_fade_, title_loop_callbacks_)) {
                const auto result = restart_startup_attract_loop(
                    state_, startup_callbacks_);
                if (result !=
                    StartupIntegrationStartResult::boot_in_progress) {
                    throw std::runtime_error(
                        "title idle timeout could not restart pre-title "
                        "sequence");
                }
            }
            return;
        }
        if (title_mode_entry_service_.complete) {
            return;
        }
        if (title_mode_entry_service_.active) {
            if (advance_title_mode_entry_service_vblank(
                    title_mode_entry_service_,
                    title_mode_entry_callbacks_) ==
                    TitleModeEntryServiceResult::complete &&
                title_loop_.controller_events.activity_timer != 0) {
                pending_title_controller_word_ = 0U;
                enter_start_options_integration(state_, startup_callbacks_);
            } else if (title_mode_entry_service_.complete &&
                       title_loop_.controller_events.activity_timer == 0) {
                begin_title_idle_restart_fade(title_idle_restart_fade_);
            }
            return;
        }
        const auto result =
            advance_title_loop_frame(title_loop_, title_loop_callbacks_);
        if (result == TitleLoopFrameResult::palette_transition_complete) {
            // 0x1BAC publishes FFF101 only after the title exit palette pass
            // has completed; carry the typed result into startup state before
            // the title loop is retired by the following menu/field routes.
            state_.title_debug_menu_unlocked =
                title_loop_.input_history.sequence_complete;
            // 0x075E runs the six-command, 18-VBlank audio service after the
            // title loop returns and before it enters the Start Options page.
            begin_title_mode_entry_service(title_mode_entry_service_,
                                           title_mode_entry_callbacks_);
        }
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
        for (const auto& [archetype_id, alias] :
             startup_archetype_portraits_) {
            static_cast<void>(archetype_id);
            cache_asset(alias);
            if (draw_asset_cache_.find(alias) == draw_asset_cache_.end()) {
                throw std::runtime_error(
                    "startup archetype portrait image could not be loaded: "
                    + alias);
            }
        }
        for (int index = 0; index < 7; ++index) {
            const auto alias =
                "title.circuit.background." + std::to_string(index);
            cache_asset(alias);
            const auto source = draw_asset_cache_.find(alias);
            if (source == draw_asset_cache_.end()) {
                continue;
            }
            for (std::size_t pass = 0;
                 pass <= kTitleCircuitPaletteRevealPassCount; ++pass) {
                title_circuit_transition_cache_[
                    static_cast<std::size_t>(index)][pass] =
                    transition_genesis_cram_image(
                        source->second, static_cast<std::uint8_t>(pass),
                        false);
            }
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

    void draw_wrapped_vertical_image(QPainter& painter,
                                     const std::string_view alias,
                                     const QImage& image,
                                     const std::uint16_t scroll) const {
        const auto* record = state_.asset_catalog.catalog.find(alias);
        if (record == nullptr || image.height() <= 0) {
            return;
        }

        // The source display service applies FFDD0E as Plane-B vertical
        // scroll. The host has the visible 320x224 circuit resource, so repeat
        // that named background at its lower edge as it moves upward.
        const auto height = image.height();
        const auto offset = static_cast<int>(scroll % height);
        if (offset == 0) {
            painter.drawImage(record->origin_x, record->origin_y, image);
            return;
        }

        painter.save();
        painter.setClipRect(record->origin_x, record->origin_y,
                            image.width(), height);
        const auto y = record->origin_y - offset;
        painter.drawImage(record->origin_x, y, image);
        painter.drawImage(record->origin_x, y + height, image);
        painter.restore();
    }

    void draw_wrapped_vertical_alias(QPainter& painter,
                                     const std::string_view alias,
                                     const std::uint16_t scroll) const {
        const auto image = draw_asset_cache_.find(std::string(alias));
        if (image == draw_asset_cache_.end()) {
            return;
        }
        draw_wrapped_vertical_image(painter, alias, image->second, scroll);
    }

    void draw_title(QPainter& painter) const {
        const auto phase = static_cast<std::size_t>(
            title_loop_.circuit_lights.phase % kTitleCircuitPhaseCount);
        const auto circuit_alias = "title.circuit.background." +
                                   std::to_string(phase);
        const auto passes = std::min<std::size_t>(
            title_loop_.text.palette_steps,
            kTitleCircuitPaletteRevealPassCount);
        const auto& circuit = title_circuit_transition_cache_[phase][passes];
        if (!circuit.isNull()) {
            draw_wrapped_vertical_image(
                painter, circuit_alias, circuit,
                title_loop_.animation_timer.animation_offset);
        } else {
            draw_wrapped_vertical_alias(
                painter, "title.background",
                title_loop_.animation_timer.animation_offset);
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
            draw_trivid_scene(painter);
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
            // 0x59D7C pans the wilderness planes with FFDD08/FFDD0A while
            // DD46/DD48 moves the Indian's head SAT list. These extracted
            // name-table regions retain source-plane origins, so the same
            // scroll must move the body art or the head visibly separates.
            painter.save();
            painter.translate(opening.stage2.x_offset,
                              -opening.stage2.y_offset);
            // ROM 0x5B4D4 installs the left wilderness region. The six
            // 0x5A080 streams then replace that same 0x0600 region; drawing
            // both images with alpha compositing leaves pixels from the old
            // actor visible underneath the later death pose.
            if (opening_wilderness_overlay_) {
                draw_opening_sprite(painter, *opening_wilderness_overlay_);
            } else if (opening_wilderness_map_left_) {
                draw_opening_sprite(painter, *opening_wilderness_map_left_);
            }
            if (opening_wilderness_map_center_) {
                draw_opening_sprite(
                    painter, *opening_wilderness_map_center_);
            }
            if (opening_wilderness_map_right_) {
                draw_opening_sprite(painter, *opening_wilderness_map_right_);
            }
            if (opening_wilderness_patch_) {
                draw_opening_sprite(painter, *opening_wilderness_patch_);
            }
            if (opening_wilderness_final_overlay_) {
                draw_opening_sprite(
                    painter, *opening_wilderness_final_overlay_);
            }
            painter.restore();
        }

        if (opening_scene_ != OpeningIntroScene::trivid) {
            for (const auto& request : opening_sprites_) {
                draw_opening_sprite(painter, request);
            }
        }
        for (const auto& request : opening_text_) {
            draw_opening_text(painter, request);
        }
    }

    void draw_trivid_scene(QPainter& painter) const {
        // ROM 0x5AA96 installs the Plane-A map with priority bit 0x8000.
        // The moving 0x1DF238 SAT list is low priority (0x6000); the fixed
        // 0x1DF1D6 and timed 0x5AD02 lists are high priority (0xC000 and
        // 0xE000). Draw the moving exterior layer behind the plane, then draw
        // both source high-priority SAT families above it. Drawing every SAT
        // after the plate put the moving layer in front of the character.
        for (const auto& request : opening_sprites_) {
            if ((request.palette_flags & 0x8000U) == 0U) {
                draw_opening_sprite(painter, request);
            }
        }
        draw_alias(painter, "intro.trivid.background");
        for (const auto& request : opening_sprites_) {
            if ((request.palette_flags & 0x8000U) != 0U) {
                draw_opening_sprite(painter, request);
            }
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
        draw_at(tile("top.left"), x, y);
        draw_at(tile("top.right"), x + width - kTile, y);
        draw_at(tile("bottom.left"), x, y + height - kTile);
        draw_at(tile("bottom.right"), x + width - kTile,
                y + height - kTile);
        for (int edge = x + kTile; edge <= x + width - 2 * kTile;
             edge += kTile) {
            draw_at(tile("top.middle"), edge, y);
            draw_at(tile("bottom.middle"), edge, y + height - kTile);
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
                return 255U;
            }();
            for (const auto& record : state_.asset_catalog.catalog.records) {
                if (record.alias.rfind(prefix, 0) != 0) {
                    continue;
                }
                const auto image = draw_asset_cache_.find(record.alias);
                if (image != draw_asset_cache_.end()) {
                    QImage layer = image->second;
                    if (group.family ==
                            OpeningIntroSpriteFamily::wilderness_map_right &&
                        opening_entity_palette_flash_active(1U)) {
                        // ROM 0x5A2D0 fills palette line 1 with white, then
                        // 0x5A328 decrements all fifteen visible entries once
                        // every nine frames. The right map plate and the
                        // separately drawn head SAT both use that line.
                        // Scaling the predecoded RGB toward black made the
                        // body disappear without the source white hit flash.
                        layer = recolor_genesis_image_to_cram_word(
                            image->second,
                            opening_entity_palette_flash_color_word(1U));
                    } else if ((group.family ==
                                    OpeningIntroSpriteFamily::wilderness_map_center ||
                                group.family ==
                                    OpeningIntroSpriteFamily::background_patch) &&
                               opening_entity_palette_flash_active(0U)) {
                        // ROM 0x5A124/0x5A16E run the same uniform white hit
                        // and nine-frame fade on palette line 0 for the
                        // center attacker. Its 0x5A1BE clear targets the
                        // center block at $0826, separate from the right
                        // actor block at $03B8.
                        layer = recolor_genesis_image_to_cram_word(
                            image->second,
                            opening_entity_palette_flash_color_word(0U));
                    } else if (palette_fade_weight != 255U) {
                        layer = scale_genesis_image_rgb(
                            image->second, palette_fade_weight);
                    }
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
            if (group.family == OpeningIntroSpriteFamily::ambush_actor &&
                (request.palette_flags & 0x6000U) == 0x2000U &&
                opening_entity_palette_flash_active(1U)) {
                const auto head = recolor_genesis_image_to_cram_word(
                    image->second,
                    opening_entity_palette_flash_color_word(1U));
                painter.drawImage(point.x, point.y, head);
            } else {
                painter.drawImage(point.x, point.y, image->second);
            }
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

    bool opening_entity_palette_flash_active(
        const std::size_t entity_index) const {
        const auto& opening = state_.opening_intro;
        // The scene-entry conversion initializes the palette snapshot to
        // 0xEEE while the scene fades in. Only the corresponding source
        // sequence event enables the visible palette effect.
        if (state_.phase != StartupIntegrationPhase::opening_intro ||
            opening.scene != OpeningIntroScene::wilderness) {
            return false;
        }
        if (entity_index == 0U) {
            return opening.sequence.stage >= 5U &&
                   opening.sequence.stage < 12U;
        }
        if (entity_index == 1U) {
            // The 0x5A34E fall-through clears the right map and still submits
            // the persistent head once with line 1 at 0x000. The next frame
            // returns at 0x5A25A without another SAT submission.
            return opening.persistent.stage >= 5U &&
                   opening.persistent.stage <= 12U;
        }
        return false;
    }

    std::uint16_t opening_entity_palette_flash_color_word(
        const std::size_t entity_index) const {
        if (entity_index >= state_.opening_intro.entity_colors.words.size()) {
            return 0U;
        }
        return state_.opening_intro.entity_colors.words[entity_index][0U];
    }

    bool opening_ambush_palette_flash_active() const {
        return opening_entity_palette_flash_active(1U);
    }

    std::uint16_t opening_ambush_palette_flash_color_word() const {
        return opening_entity_palette_flash_color_word(1U);
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
        const auto text_id = opening_intro_text_id(text_asset);
        const auto text_key =
            opening_text_keys_.find(std::string(text_id));
        if (text_key == opening_text_keys_.end()) {
            return;
        }
        const auto* text = startup_scripts_.dialogue_text(text_key->second);
        if (text == nullptr) {
            return;
        }
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
        draw_startup_text(painter, x, y, *text,
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
            !title_loop_.sprite.projection_visible) {
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
                                kStartOptionsLayout,
                                &*startup_options_menu_);
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
                                kStartOptionsLayout,
                                &*startup_options_menu_);
            draw_startup_layout(painter, kSaveLoadPlaneVram, kSaveLoadLayout,
                                &*startup_save_slots_menu_);
            draw_cursor(painter, startup_menu_vram_x(kSaveLoadCursorVram),
                        startup_menu_cursor_y(
                            kSaveLoadCursorVram,
                            static_cast<std::uint8_t>(
                                state_.save_load.selected)));
            return;
        }

        draw_menu_metadata_grid(painter);
        const auto selected_index = static_cast<std::size_t>(
            state_.joshua_archetype.selected);
        const auto& choices = startup_archetypes_menu_->choices;
        const auto portrait_binding =
            selected_index < choices.size()
                ? startup_archetype_portraits_.find(
                      choices[selected_index].id)
                : startup_archetype_portraits_.end();
        const auto portrait = portrait_binding !=
                                      startup_archetype_portraits_.end()
                                  ? draw_asset_cache_.find(
                                        portrait_binding->second)
                                  : draw_asset_cache_.end();
        if (portrait != draw_asset_cache_.end()) {
            // 0x592C2 -> 0x1F38A uploads the selected 80-word portrait
            // resource at the source's x/y tile origin (8,8), i.e. the
            // 64,64 host pixel position. The old 80,56 placement was a
            // visual guess and shifted every archetype portrait.
            painter.drawImage(64, 64, portrait->second);
        }
        draw_startup_layout(painter, kJoshuaArchetypePlaneVram,
                            kJoshuaArchetypeLayout,
                            &*startup_archetypes_menu_);
        draw_cursor(painter, startup_menu_vram_x(kJoshuaArchetypeCursorVram),
                    startup_menu_cursor_y(
                        kJoshuaArchetypeCursorVram,
                        static_cast<std::uint8_t>(
                            state_.joshua_archetype.selected)));
    }

    template <std::size_t N>
    void draw_startup_layout(
        QPainter& painter, const std::uint16_t plane_vram,
        const std::array<StartupMenuTextLine, N>& lines,
        const scripting::ScriptMenuDefinition* menu = nullptr) const {
        const auto x = startup_menu_vram_x(plane_vram);
        for (const auto& line : lines) {
            const std::string* text = nullptr;
            if (line.choice_index == 0xFFU) {
                const auto text_key =
                    startup_text_keys_.find(std::string(line.text_id));
                if (text_key != startup_text_keys_.end()) {
                    text = startup_scripts_.dialogue_text(text_key->second);
                }
            } else if (menu != nullptr &&
                       line.choice_index < menu->choices.size()) {
                text = startup_scripts_.dialogue_text(
                    menu->choices[line.choice_index].label_key);
            }
            if (text != nullptr) {
                draw_startup_text(painter, x,
                                  startup_menu_line_y(plane_vram, line.row),
                                  *text, source_text_color(0U));
            }
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
            // The alpha-trimmed startup cursor has source origin_x=4 in the
            // asset catalog. Restore that offset just as draw_alias does.
            draw_alias(painter, "menu.startup.cursor", x, y);
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
    debug::KeyboardInputLog input_log_;
    debug::FrameStepper frame_stepper_;
    std::uint64_t source_frame_index_ = 0;
    bool replaying_input_ = false;
    bool replay_diverged_ = false;
    StartupIntegrationState state_;
    StartupIntegrationCallbacks startup_callbacks_;
    scripting::GameScriptRuntime startup_scripts_;
    scripting::ScriptStringMap startup_text_keys_;
    scripting::ScriptStringMap opening_text_keys_;
    std::optional<scripting::ScriptMenuDefinition> startup_options_menu_;
    std::optional<scripting::ScriptMenuDefinition> startup_save_slots_menu_;
    std::optional<scripting::ScriptMenuDefinition> startup_archetypes_menu_;
    scripting::ScriptStringMap startup_archetype_portraits_;
    std::unique_ptr<AudioPlaybackService> audio_playback_;
    std::vector<std::string> pending_debug_events_;
    bool debug_event_queue_enabled_ = false;
    bool source_random_observer_installed_ = false;
    qt_runtime::FieldSceneRuntime field_scene_runtime_;
    TitleLoopState title_loop_;
    TitleLoopCallbacks title_loop_callbacks_;
    TitleModeEntryServiceState title_mode_entry_service_;
    TitleModeEntryServiceCallbacks title_mode_entry_callbacks_;
    TitleIdleRestartFadeState title_idle_restart_fade_;
    QTimer host_frame_timer_;
    QElapsedTimer host_clock_;
    std::unique_ptr<FrameDebugServer> debug_server_;
    std::vector<PendingRemoteKey> pending_remote_keys_;
    bool applying_remote_key_ = false;
    std::int64_t host_frame_deadline_ns_ = 0;
    std::map<std::string, QImage> draw_asset_cache_;
    std::array<std::array<QImage,
                         kTitleCircuitPaletteRevealPassCount + 1U>,
               kTitleCircuitPhaseCount>
        title_circuit_transition_cache_{};
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
    std::uint8_t logo_strobe_phase_ = 0;
    std::array<std::uint16_t, 8> logo_strobe_palette_{};
    std::vector<BuiltSpriteRecord> pre_title_sprite_records_;
    JoshuaArchetypeChoice selected_archetype_ =
        JoshuaArchetypeChoice::samurai;
    StartOptionsChoice start_options_cursor_ = StartOptionsChoice::new_game;
    SaveSlotChoice save_load_cursor_ = SaveSlotChoice::slot_one;
    JoshuaArchetypeChoice joshua_cursor_ = JoshuaArchetypeChoice::samurai;
    JoshuaArchetypeChoice joshua_preview_ = JoshuaArchetypeChoice::samurai;
    SaveSlotChoice save_load_requested_slot_ = SaveSlotChoice::slot_one;
    SaveLoadMessage save_load_message_ = SaveLoadMessage::none;
    bool save_load_boundary_unresolved_ = false;
    bool title_prompt_visible_ = false;
    bool title_copyright_visible_ = false;
    bool title_completion_palette_copied_ = false;
    TitlePromptVariant title_prompt_variant_ = TitlePromptVariant::VariantA;
    std::uint8_t title_circuit_phase_ = 0;
    std::uint16_t title_logo_columns_ = 0;
    std::uint16_t title_sprite_phase_ = 0;
    std::int16_t title_sprite_x_ = 0x98;
    std::uint16_t title_controller_events_ = 0;
    std::uint16_t pending_title_controller_word_ = 0;
    std::uint16_t pending_opening_controller_word_ = 0;
    std::uint16_t sampled_opening_controller_word_ = 0;
    std::uint8_t pending_start_options_controller_byte_ = 0;
    std::uint8_t field_controller_nibble_ = 0;
    std::uint8_t field_controller_port_byte_ = 0;
    bool opening_start_event_latched_ = false;
#ifdef SHADOWRUN_QT_QA_DRIVER
#include "../../tests/qa/ghidra_conversion_qt_qa_window_members.inc"
#endif
};

} // namespace

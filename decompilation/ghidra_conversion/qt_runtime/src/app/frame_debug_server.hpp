#pragma once

#include "shadowrun_ghidra/debug/frame_debugger.hpp"

#include <QByteArray>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline std::uint64_t debug_inspection_fingerprint(QJsonObject fields) {
    // These two values describe the host/debugger, not game state. Leaving
    // them out lets a record made under --debug-port replay in the normal app.
    fields.remove(QStringLiteral("app.paused"));
    fields.remove(QStringLiteral("app.replay_diverged"));
    const auto bytes = QJsonDocument(fields).toJson(QJsonDocument::Compact);
    std::uint64_t hash = 14695981039346656037ULL;
    for (const auto byte : bytes) {
        hash ^= static_cast<unsigned char>(byte);
        hash *= 1099511628211ULL;
    }
    return hash;
}

// Local-only newline-delimited JSON control for the converted frame loop.
// It has no game-state logic of its own: the window supplies typed inspection
// data and queues keyboard transitions for the next source-frame boundary.
class FrameDebugServer final {
public:
    using StateProvider = std::function<QJsonObject()>;
    using EventProvider = std::function<QStringList()>;
    using FrameProvider = std::function<std::uint64_t()>;
    using PhaseProvider = std::function<QString()>;
    using KeyQueue = std::function<void(int, bool)>;
    using ImageDumper = std::function<bool(const QString&, QString*)>;

    FrameDebugServer(const std::uint16_t port, debug::FrameStepper& stepper,
                     StateProvider state, FrameProvider frame,
                     PhaseProvider phase, KeyQueue queue_key,
                     EventProvider events = {}, ImageDumper dump_image = {})
        : stepper_(stepper),
          state_(std::move(state)),
          frame_(std::move(frame)),
          phase_(std::move(phase)),
          queue_key_(std::move(queue_key)),
          events_(std::move(events)),
          dump_image_(std::move(dump_image)) {
        if (!server_.listen(QHostAddress::LocalHost, port)) {
            throw std::runtime_error(
                "could not listen for local debugger: " +
                server_.errorString().toStdString());
        }
        QObject::connect(&server_, &QTcpServer::newConnection, &server_,
                         [this] { accept_connections(); });
    }

    ~FrameDebugServer() {
        for (const auto& [socket, buffer] : buffers_) {
            (void)buffer;
            QObject::disconnect(socket, nullptr, nullptr, nullptr);
            socket->abort();
        }
        buffers_.clear();
    }

    std::uint16_t port() const noexcept { return server_.serverPort(); }

    // Exposed so protocol behavior can be tested without opening a socket.
    QByteArray handle_line(const QByteArray& line) {
        QJsonParseError parse_error;
        const auto document = QJsonDocument::fromJson(line, &parse_error);
        if (parse_error.error != QJsonParseError::NoError ||
            !document.isObject()) {
            return encode_error(QStringLiteral("request must be a JSON object"));
        }
        const auto request = document.object();
        const auto command = request.value(QStringLiteral("cmd")).toString();
        if (command == QStringLiteral("status")) {
            return encode(status_object());
        }
        if (command == QStringLiteral("pause")) {
            stepper_.pause();
            stop_reason_ = QStringLiteral("pause");
            stop_breakpoint_ = 0;
            stop_detail_.clear();
            return encode(status_object());
        }
        if (command == QStringLiteral("resume")) {
            stepper_.resume();
            stop_reason_ = QStringLiteral("none");
            stop_breakpoint_ = 0;
            stop_detail_.clear();
            return encode(status_object());
        }
        if (command == QStringLiteral("step")) {
            const auto value = request.value(QStringLiteral("n"));
            const auto count = value.isUndefined() ? 1.0 : value.toDouble(-1.0);
            if (!std::isfinite(count) || count < 1.0 || count > 60000.0 ||
                std::floor(count) != count) {
                return encode_error(
                    QStringLiteral("step n must be an integer from 1 to 60000"));
            }
            if (!stepper_.paused()) {
                stepper_.pause();
            }
            stepper_.queue_step(static_cast<std::uint32_t>(count));
            stop_reason_ = QStringLiteral("step");
            stop_breakpoint_ = 0;
            stop_detail_.clear();
            return encode(status_object());
        }
        if (command == QStringLiteral("break")) {
            const auto kind = request.value(QStringLiteral("kind")).toString(
                QStringLiteral("frame"));
            const auto id = ++next_breakpoint_id_;
            if (kind == QStringLiteral("watch")) {
                const auto field = request.value(QStringLiteral("field"))
                                       .toString();
                if (field.isEmpty()) {
                    --next_breakpoint_id_;
                    return encode_error(
                        QStringLiteral("watch breakpoint requires a field name"));
                }
                if (std::any_of(watchpoints_.begin(), watchpoints_.end(),
                                [&field](const WatchBreakpoint& watchpoint) {
                                    return watchpoint.field == field;
                                })) {
                    --next_breakpoint_id_;
                    return encode_error(QStringLiteral(
                        "a watchpoint already exists for that field"));
                }
                const auto fields = flattened_state_fields();
                const auto found = fields.constFind(field);
                std::optional<QJsonValue> initial;
                if (found != fields.constEnd()) {
                    initial = *found;
                }
                watchpoints_.push_back({id, field, initial});
                QJsonObject result;
                result.insert(QStringLiteral("ok"), true);
                result.insert(QStringLiteral("id"), id);
                result.insert(QStringLiteral("kind"), QStringLiteral("watch"));
                result.insert(QStringLiteral("field"), field);
                return encode(result);
            }
            if (kind == QStringLiteral("event")) {
                const auto pattern = request.value(QStringLiteral("pattern"))
                                         .toString();
                const QRegularExpression expression(pattern);
                if (pattern.isEmpty() || !expression.isValid()) {
                    --next_breakpoint_id_;
                    return encode_error(QStringLiteral(
                        "event breakpoint requires a valid regular expression"));
                }
                if (std::any_of(event_breakpoints_.begin(),
                                event_breakpoints_.end(),
                                [&pattern](const EventBreakpoint& breakpoint) {
                                    return breakpoint.pattern == pattern;
                                })) {
                    --next_breakpoint_id_;
                    return encode_error(QStringLiteral(
                        "an event breakpoint already exists for that pattern"));
                }
                event_breakpoints_.push_back({id, pattern, expression});
                QJsonObject result;
                result.insert(QStringLiteral("ok"), true);
                result.insert(QStringLiteral("id"), id);
                result.insert(QStringLiteral("kind"), QStringLiteral("event"));
                result.insert(QStringLiteral("pattern"), pattern);
                return encode(result);
            }
            if (kind != QStringLiteral("frame")) {
                --next_breakpoint_id_;
                return encode_error(
                    QStringLiteral("supported breakpoint kinds are frame, watch, and event"));
            }
            const auto value = request.value(QStringLiteral("frame"));
            const auto target = value.toDouble(-1.0);
            const auto current = frame_ ? frame_() : 0U;
            if (!std::isfinite(target) || target < 0.0 ||
                target > 9007199254740991.0 || std::floor(target) != target ||
                target <= static_cast<double>(current)) {
                --next_breakpoint_id_;
                return encode_error(QStringLiteral(
                    "frame breakpoint must be an integer after the current frame"));
            }
            if (std::any_of(breakpoints_.begin(), breakpoints_.end(),
                            [target](const FrameBreakpoint& breakpoint) {
                                return static_cast<double>(breakpoint.frame) ==
                                       target;
                            })) {
                --next_breakpoint_id_;
                return encode_error(QStringLiteral(
                    "a breakpoint already exists at that frame"));
            }
            breakpoints_.push_back(
                {id, static_cast<std::uint64_t>(target)});
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("id"), id);
            result.insert(QStringLiteral("kind"), QStringLiteral("frame"));
            result.insert(QStringLiteral("frame"), target);
            return encode(result);
        }
        if (command == QStringLiteral("delete")) {
            const auto value = request.value(QStringLiteral("id"));
            const auto numeric_id = value.toDouble(-1.0);
            if (!std::isfinite(numeric_id) || numeric_id < 1.0 ||
                std::floor(numeric_id) != numeric_id ||
                numeric_id > static_cast<double>(std::numeric_limits<int>::max())) {
                return encode_error(QStringLiteral("delete requires a breakpoint id"));
            }
            const auto id = static_cast<int>(numeric_id);
            const auto found = std::find_if(
                breakpoints_.begin(), breakpoints_.end(),
                [id](const FrameBreakpoint& breakpoint) {
                    return breakpoint.id == id;
                });
            if (found != breakpoints_.end()) {
                breakpoints_.erase(found);
                return encode(status_object());
            }
            const auto watch = std::find_if(
                watchpoints_.begin(), watchpoints_.end(),
                [id](const WatchBreakpoint& breakpoint) {
                    return breakpoint.id == id;
                });
            if (watch == watchpoints_.end()) {
                const auto event = std::find_if(
                    event_breakpoints_.begin(), event_breakpoints_.end(),
                    [id](const EventBreakpoint& breakpoint) {
                        return breakpoint.id == id;
                    });
                if (event == event_breakpoints_.end()) {
                    return encode_error(QStringLiteral("no such breakpoint"));
                }
                event_breakpoints_.erase(event);
                return encode(status_object());
            }
            watchpoints_.erase(watch);
            return encode(status_object());
        }
        if (command == QStringLiteral("list")) {
            QJsonArray entries;
            for (const auto& breakpoint : breakpoints_) {
                QJsonObject entry;
                entry.insert(QStringLiteral("id"), breakpoint.id);
                entry.insert(QStringLiteral("kind"), QStringLiteral("frame"));
                entry.insert(QStringLiteral("frame"),
                             static_cast<double>(breakpoint.frame));
                entries.append(entry);
            }
            for (const auto& watchpoint : watchpoints_) {
                QJsonObject entry;
                entry.insert(QStringLiteral("id"), watchpoint.id);
                entry.insert(QStringLiteral("kind"), QStringLiteral("watch"));
                entry.insert(QStringLiteral("field"), watchpoint.field);
                entries.append(entry);
            }
            for (const auto& breakpoint : event_breakpoints_) {
                QJsonObject entry;
                entry.insert(QStringLiteral("id"), breakpoint.id);
                entry.insert(QStringLiteral("kind"), QStringLiteral("event"));
                entry.insert(QStringLiteral("pattern"), breakpoint.pattern);
                entries.append(entry);
            }
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("breakpoints"), entries);
            return encode(result);
        }
        if (command == QStringLiteral("key")) {
            const auto name = request.value(QStringLiteral("key")).toString();
            const auto key = key_code(name);
            const auto pressed = request.value(QStringLiteral("pressed"));
            if (!key.has_value() || !pressed.isBool()) {
                return encode_error(QStringLiteral(
                    "key requires key=up|down|left|right|a|b|c|start and a boolean pressed value"));
            }
            if (queue_key_) {
                queue_key_(*key, pressed.toBool());
            }
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("queued"), true);
            result.insert(QStringLiteral("frame"),
                          static_cast<double>(frame_ ? frame_() : 0U));
            return encode(result);
        }
        if (command == QStringLiteral("press")) {
            const auto key = key_code(
                request.value(QStringLiteral("key")).toString());
            const auto hold = request.value(QStringLiteral("frames"))
                                  .toDouble(1.0);
            const auto release = request.value(QStringLiteral("release"))
                                     .toDouble(1.0);
            if (!key.has_value() || !valid_frame_count(hold) ||
                !valid_frame_count(release) || hold + release > 60000.0) {
                return encode_error(QStringLiteral(
                    "press requires a supported key and positive frame counts totaling at most 60000"));
            }
            if (!stepper_.paused() || stepper_.pending_steps() != 0U) {
                return encode_error(QStringLiteral(
                    "press requires a paused debugger with no pending steps"));
            }
            const auto current = frame_ ? frame_() : 0U;
            const auto hold_frames = static_cast<std::uint32_t>(hold);
            if (current > std::numeric_limits<std::uint64_t>::max() -
                              hold_frames) {
                return encode_error(QStringLiteral("press target frame overflow"));
            }
            if (queue_key_) {
                queue_key_(*key, true);
            }
            scheduled_releases_.push_back(
                {*key, current + hold_frames});
            stepper_.queue_step(static_cast<std::uint32_t>(hold + release));
            stop_reason_ = QStringLiteral("press");
            stop_breakpoint_ = 0;
            stop_detail_.clear();
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("queued"), true);
            result.insert(QStringLiteral("frame"), static_cast<double>(current));
            result.insert(QStringLiteral("release_at"),
                          static_cast<double>(current + hold_frames));
            result.insert(QStringLiteral("pending_steps"),
                          static_cast<double>(stepper_.pending_steps()));
            return encode(result);
        }
        if (command == QStringLiteral("inspect")) {
            const auto flattened = flattened_state_fields();
            const auto prefix = request.value(QStringLiteral("prefix")).toString();
            QJsonObject selected;
            for (auto it = flattened.begin(); it != flattened.end(); ++it) {
                if (it.key().startsWith(prefix)) {
                    selected.insert(it.key(), it.value());
                }
            }
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("frame"),
                          static_cast<double>(frame_ ? frame_() : 0U));
            result.insert(QStringLiteral("fields"), selected);
            return encode(result);
        }
        if (command == QStringLiteral("hash")) {
            const auto fingerprint = debug_inspection_fingerprint(
                state_ ? state_() : QJsonObject{});
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("frame"),
                          static_cast<double>(frame_ ? frame_() : 0U));
            result.insert(QStringLiteral("scope"),
                          QStringLiteral("debug_inspection_fields"));
            result.insert(QStringLiteral("fingerprint"),
                          QString::number(fingerprint, 16).rightJustified(16,
                                                                          QLatin1Char('0')));
            return encode(result);
        }
        if (command == QStringLiteral("diverge")) {
            const auto first_path =
                request.value(QStringLiteral("a")).toString().toStdString();
            const auto second_path =
                request.value(QStringLiteral("b")).toString().toStdString();
            if (first_path.empty() || second_path.empty()) {
                return encode_error(
                    QStringLiteral("diverge requires recording paths a and b"));
            }
            try {
                const debug::KeyboardInputLog first(
                    {debug::InputLogConfig::Mode::replay, first_path});
                const debug::KeyboardInputLog second(
                    {debug::InputLogConfig::Mode::replay, second_path});
                const bool compare_seed = first.has_seed_history() &&
                                          second.has_seed_history();
                const bool compare_fingerprint =
                    first.has_fingerprint_history() &&
                    second.has_fingerprint_history();
                if (!compare_seed && !compare_fingerprint) {
                    return encode_error(QStringLiteral(
                        "recordings share no comparable seed or inspection-fingerprint history"));
                }
                const auto common_frames = std::min(
                    first.recorded_frame_count(), second.recorded_frame_count());
                for (std::uint64_t frame = 0; frame < common_frames; ++frame) {
                    if (compare_seed &&
                        first.expected_seed_after_frame(frame) !=
                            second.expected_seed_after_frame(frame)) {
                        return encode_divergence(frame + 1U,
                                                 QStringLiteral("rom_seed"),
                                                 compare_seed,
                                                 compare_fingerprint);
                    }
                    if (compare_fingerprint &&
                        first.expected_fingerprint_after_frame(frame) !=
                            second.expected_fingerprint_after_frame(frame)) {
                        return encode_divergence(
                            frame + 1U,
                            QStringLiteral("inspection_fingerprint"),
                            compare_seed, compare_fingerprint);
                    }
                }
                if (first.recorded_frame_count() !=
                    second.recorded_frame_count()) {
                    return encode_divergence(
                        common_frames + 1U, QStringLiteral("frame_count"),
                        compare_seed, compare_fingerprint);
                }
                QJsonObject result;
                result.insert(QStringLiteral("ok"), true);
                result.insert(QStringLiteral("found"), false);
                result.insert(QStringLiteral("frames_compared"),
                              static_cast<double>(common_frames));
                result.insert(QStringLiteral("compared_rom_seeds"),
                              compare_seed);
                result.insert(QStringLiteral("compared_fingerprints"),
                              compare_fingerprint);
                return encode(result);
            } catch (const std::exception& error) {
                return encode_error(QStringLiteral("could not compare recordings: %1")
                                        .arg(QString::fromUtf8(error.what())));
            }
        }
        if (command == QStringLiteral("events")) {
            const auto path =
                request.value(QStringLiteral("path")).toString().toStdString();
            if (path.empty()) {
                return encode_error(
                    QStringLiteral("events requires a recorded session path"));
            }
            const auto from_value =
                request.value(QStringLiteral("from")).toDouble(0.0);
            const auto to_value = request.value(QStringLiteral("to"))
                                      .toDouble(9007199254740991.0);
            if (!std::isfinite(from_value) || !std::isfinite(to_value) ||
                from_value < 0.0 || to_value < from_value ||
                std::floor(from_value) != from_value ||
                std::floor(to_value) != to_value) {
                return encode_error(QStringLiteral(
                    "events frame range must be nonnegative integers with from <= to"));
            }
            const auto pattern =
                request.value(QStringLiteral("pattern")).toString();
            const QRegularExpression expression(pattern);
            if (!pattern.isEmpty() && !expression.isValid()) {
                return encode_error(QStringLiteral(
                    "events pattern must be a valid regular expression"));
            }
            try {
                const debug::KeyboardInputLog log(
                    {debug::InputLogConfig::Mode::replay, path});
                if (!log.has_source_event_history()) {
                    return encode_error(QStringLiteral(
                        "recording has no source-boundary event history"));
                }
                QJsonArray entries;
                bool truncated = false;
                for (const auto& event : log.source_boundary_events()) {
                    const auto frame = event.frame + 1U;
                    if (frame < static_cast<std::uint64_t>(from_value) ||
                        frame > static_cast<std::uint64_t>(to_value) ||
                        (!pattern.isEmpty() &&
                         !expression.match(
                              QString::fromStdString(event.name)).hasMatch())) {
                        continue;
                    }
                    if (entries.size() >= 1000) {
                        truncated = true;
                        break;
                    }
                    QJsonObject item;
                    item.insert(QStringLiteral("frame"),
                                static_cast<double>(frame));
                    item.insert(QStringLiteral("name"),
                                QString::fromStdString(event.name));
                    entries.append(item);
                }
                QJsonObject result;
                result.insert(QStringLiteral("ok"), true);
                result.insert(QStringLiteral("events"), entries);
                result.insert(QStringLiteral("truncated"), truncated);
                return encode(result);
            } catch (const std::exception& error) {
                return encode_error(QStringLiteral("could not read events: %1")
                                        .arg(QString::fromUtf8(error.what())));
            }
        }
        if (command == QStringLiteral("dump")) {
            const auto path = request.value(QStringLiteral("path")).toString();
            if (path.isEmpty() || !dump_image_) {
                return encode_error(QStringLiteral(
                    "dump requires a path and a live image provider"));
            }
            QString error;
            if (!dump_image_(path, &error)) {
                return encode_error(error.isEmpty()
                                        ? QStringLiteral("could not dump the current frame")
                                        : error);
            }
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("frame"),
                          static_cast<double>(frame_ ? frame_() : 0U));
            result.insert(QStringLiteral("path"), path);
            return encode(result);
        }
        if (command == QStringLiteral("help")) {
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("commands"),
                          QStringLiteral("status pause resume step key press inspect hash dump events diverge break(frame|watch|event) delete list help"));
            return encode(result);
        }
        return encode_error(QStringLiteral("unknown command"));
    }

    // Called after the host completes and numbers each source frame. Returning
    // true tells the frame pump to stop its current batch at the breakpoint.
    bool check_source_frame_breakpoints() {
        const auto current = frame_ ? frame_() : 0U;
        scheduled_releases_.erase(
            std::remove_if(
                scheduled_releases_.begin(), scheduled_releases_.end(),
                [this, current](const ScheduledRelease& release) {
                    if (release.frame > current) {
                        return false;
                    }
                    if (queue_key_) {
                        queue_key_(release.key, false);
                    }
                    return true;
                }),
            scheduled_releases_.end());
        const auto found = std::find_if(
            breakpoints_.begin(), breakpoints_.end(),
            [current](const FrameBreakpoint& breakpoint) {
                return breakpoint.frame == current;
            });
        int stop_id = found != breakpoints_.end() ? found->id : 0;
        QString stop_detail = found != breakpoints_.end()
                                  ? QString::number(current)
                                  : QString{};
        const auto source_events = events_ ? events_() : QStringList{};
        for (const auto& breakpoint : event_breakpoints_) {
            if (stop_id != 0) {
                break;
            }
            const auto event = std::find_if(
                source_events.begin(), source_events.end(),
                [&breakpoint](const QString& name) {
                    return breakpoint.expression.match(name).hasMatch();
                });
            if (event != source_events.end()) {
                stop_id = breakpoint.id;
                stop_detail = *event;
            }
        }
        int watch_hit_id = 0;
        QString watch_detail;
        if (!watchpoints_.empty()) {
            const auto fields = flattened_state_fields();
            for (auto& watchpoint : watchpoints_) {
                const auto value = fields.constFind(watchpoint.field);
                std::optional<QJsonValue> current_value;
                if (value != fields.constEnd()) {
                    current_value = *value;
                }
                if (watch_hit_id == 0 && stop_id == 0 &&
                    current_value != watchpoint.last_value) {
                    watch_hit_id = watchpoint.id;
                    watch_detail = watchpoint.field + QStringLiteral(": ") +
                                   json_value_text(watchpoint.last_value) +
                                   QStringLiteral(" -> ") +
                                   json_value_text(current_value);
                }
                watchpoint.last_value = current_value;
            }
        }
        if (stop_id == 0) {
            stop_id = watch_hit_id;
            stop_detail = watch_detail;
        }
        if (stop_id == 0) {
            return false;
        }
        stop_reason_ = QStringLiteral("breakpoint");
        stop_breakpoint_ = stop_id;
        stop_detail_ = stop_detail;
        stepper_.pause();
        return true;
    }

private:
    static QByteArray encode(const QJsonObject& object) {
        return QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n';
    }

    static QByteArray encode_error(const QString& message) {
        QJsonObject object;
        object.insert(QStringLiteral("ok"), false);
        object.insert(QStringLiteral("error"), message);
        return encode(object);
    }

    static QByteArray encode_divergence(const std::uint64_t frame,
                                        const QString& reason,
                                        const bool compared_seed,
                                        const bool compared_fingerprint) {
        QJsonObject result;
        result.insert(QStringLiteral("ok"), true);
        result.insert(QStringLiteral("found"), true);
        result.insert(QStringLiteral("frame"), static_cast<double>(frame));
        result.insert(QStringLiteral("reason"), reason);
        result.insert(QStringLiteral("compared_rom_seeds"), compared_seed);
        result.insert(QStringLiteral("compared_fingerprints"),
                      compared_fingerprint);
        return encode(result);
    }

    struct FrameBreakpoint {
        int id = 0;
        std::uint64_t frame = 0;
    };

    struct WatchBreakpoint {
        int id = 0;
        QString field;
        std::optional<QJsonValue> last_value;
    };

    struct EventBreakpoint {
        int id = 0;
        QString pattern;
        QRegularExpression expression;
    };

    struct ScheduledRelease {
        int key = 0;
        std::uint64_t frame = 0;
    };

    static bool valid_frame_count(const double value) noexcept {
        return std::isfinite(value) && value >= 1.0 && value <= 60000.0 &&
               std::floor(value) == value;
    }

    QJsonObject flattened_state_fields() const {
        QJsonObject fields;
        if (state_) {
            flatten({}, state_(), fields);
        }
        return fields;
    }

    static QString json_value_text(
        const std::optional<QJsonValue>& value) {
        if (!value.has_value()) {
            return QStringLiteral("<missing>");
        }
        QJsonArray wrapper;
        wrapper.append(*value);
        const auto encoded = QJsonDocument(wrapper).toJson(
            QJsonDocument::Compact);
        return QString::fromUtf8(encoded).mid(1, encoded.size() - 2);
    }

    static void flatten(const QString& prefix, const QJsonValue& value,
                        QJsonObject& output) {
        if (value.isObject()) {
            const auto object = value.toObject();
            for (auto it = object.begin(); it != object.end(); ++it) {
                const auto key = prefix.isEmpty()
                                     ? it.key()
                                     : prefix + QLatin1Char('.') + it.key();
                flatten(key, it.value(), output);
            }
            return;
        }
        if (value.isArray()) {
            const auto array = value.toArray();
            for (qsizetype index = 0; index < array.size(); ++index) {
                flatten(prefix + QLatin1Char('.') + QString::number(index),
                        array.at(index), output);
            }
            return;
        }
        output.insert(prefix, value);
    }

    static std::optional<int> key_code(const QString& name) {
        static const std::map<QString, int> keys{
            {QStringLiteral("up"), Qt::Key_Up},
            {QStringLiteral("down"), Qt::Key_Down},
            {QStringLiteral("left"), Qt::Key_Left},
            {QStringLiteral("right"), Qt::Key_Right},
            {QStringLiteral("a"), Qt::Key_Z},
            {QStringLiteral("b"), Qt::Key_X},
            {QStringLiteral("c"), Qt::Key_C},
            {QStringLiteral("start"), Qt::Key_Return},
        };
        const auto found = keys.find(name);
        return found == keys.end() ? std::nullopt
                                   : std::optional<int>{found->second};
    }

    QJsonObject status_object() const {
        QJsonObject result;
        result.insert(QStringLiteral("ok"), true);
        result.insert(QStringLiteral("frame"),
                      static_cast<double>(frame_ ? frame_() : 0U));
        result.insert(QStringLiteral("phase"),
                      phase_ ? phase_() : QStringLiteral("unknown"));
        result.insert(QStringLiteral("mode"),
                      stepper_.paused() ? QStringLiteral("paused")
                                        : QStringLiteral("running"));
        result.insert(QStringLiteral("pending_steps"),
                      static_cast<double>(stepper_.pending_steps()));
        result.insert(QStringLiteral("stop"), stop_reason_);
        result.insert(QStringLiteral("breakpoint"), stop_breakpoint_);
        result.insert(QStringLiteral("detail"), stop_detail_);
        result.insert(QStringLiteral("port"),
                      static_cast<int>(server_.serverPort()));
        return result;
    }

    void accept_connections() {
        while (server_.hasPendingConnections()) {
            auto* socket = server_.nextPendingConnection();
            if (socket == nullptr) {
                continue;
            }
            buffers_.emplace(socket, QByteArray{});
            QObject::connect(socket, &QTcpSocket::readyRead, socket,
                             [this, socket] { receive(socket); });
            QObject::connect(socket, &QTcpSocket::disconnected, socket,
                             [this, socket] {
                                 buffers_.erase(socket);
                                 socket->deleteLater();
                             });
        }
    }

    void receive(QTcpSocket* socket) {
        auto found = buffers_.find(socket);
        if (found == buffers_.end()) {
            return;
        }
        found->second += socket->readAll();
        if (found->second.size() > kMaximumRequestBytes) {
            socket->write(encode_error(QStringLiteral("request line is too long")));
            socket->disconnectFromHost();
            buffers_.erase(found);
            return;
        }
        for (qsizetype newline = found->second.indexOf('\n'); newline >= 0;
             newline = found->second.indexOf('\n')) {
            auto line = found->second.left(newline).trimmed();
            found->second.remove(0, newline + 1);
            socket->write(handle_line(line));
        }
    }

    static constexpr qsizetype kMaximumRequestBytes = 65536;
    debug::FrameStepper& stepper_;
    StateProvider state_;
    FrameProvider frame_;
    PhaseProvider phase_;
    KeyQueue queue_key_;
    EventProvider events_;
    ImageDumper dump_image_;
    QTcpServer server_;
    std::map<QTcpSocket*, QByteArray> buffers_;
    std::vector<FrameBreakpoint> breakpoints_;
    std::vector<WatchBreakpoint> watchpoints_;
    std::vector<EventBreakpoint> event_breakpoints_;
    std::vector<ScheduledRelease> scheduled_releases_;
    int next_breakpoint_id_ = 0;
    QString stop_reason_ = QStringLiteral("none");
    int stop_breakpoint_ = 0;
    QString stop_detail_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime

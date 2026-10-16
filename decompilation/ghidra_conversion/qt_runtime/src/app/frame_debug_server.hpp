#pragma once

#include "shadowrun_ghidra/debug/frame_debugger.hpp"

#include <QByteArray>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

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

// Local-only newline-delimited JSON control for the converted frame loop.
// It has no game-state logic of its own: the window supplies typed inspection
// data and queues keyboard transitions for the next source-frame boundary.
class FrameDebugServer final {
public:
    using StateProvider = std::function<QJsonObject()>;
    using FrameProvider = std::function<std::uint64_t()>;
    using PhaseProvider = std::function<QString()>;
    using KeyQueue = std::function<void(int, bool)>;

    FrameDebugServer(const std::uint16_t port, debug::FrameStepper& stepper,
                     StateProvider state, FrameProvider frame,
                     PhaseProvider phase, KeyQueue queue_key)
        : stepper_(stepper),
          state_(std::move(state)),
          frame_(std::move(frame)),
          phase_(std::move(phase)),
          queue_key_(std::move(queue_key)) {
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
            if (request.value(QStringLiteral("kind")).toString(
                    QStringLiteral("frame")) != QStringLiteral("frame")) {
                return encode_error(
                    QStringLiteral("only frame breakpoints are supported"));
            }
            const auto value = request.value(QStringLiteral("frame"));
            const auto target = value.toDouble(-1.0);
            const auto current = frame_ ? frame_() : 0U;
            if (!std::isfinite(target) || target < 0.0 ||
                target > 9007199254740991.0 || std::floor(target) != target ||
                target <= static_cast<double>(current)) {
                return encode_error(QStringLiteral(
                    "frame breakpoint must be an integer after the current frame"));
            }
            if (std::any_of(breakpoints_.begin(), breakpoints_.end(),
                            [target](const FrameBreakpoint& breakpoint) {
                                return static_cast<double>(breakpoint.frame) ==
                                       target;
                            })) {
                return encode_error(QStringLiteral(
                    "a breakpoint already exists at that frame"));
            }
            const auto id = ++next_breakpoint_id_;
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
            if (found == breakpoints_.end()) {
                return encode_error(QStringLiteral("no such breakpoint"));
            }
            breakpoints_.erase(found);
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
        if (command == QStringLiteral("inspect")) {
            QJsonObject flattened;
            if (state_) {
                flatten({}, state_(), flattened);
            }
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
        if (command == QStringLiteral("help")) {
            QJsonObject result;
            result.insert(QStringLiteral("ok"), true);
            result.insert(QStringLiteral("commands"),
                          QStringLiteral("status pause resume step key inspect break delete list help"));
            return encode(result);
        }
        return encode_error(QStringLiteral("unknown command"));
    }

    // Called after the host completes and numbers each source frame. Returning
    // true tells the frame pump to stop its current batch at the breakpoint.
    bool check_source_frame_breakpoints() {
        const auto current = frame_ ? frame_() : 0U;
        const auto found = std::find_if(
            breakpoints_.begin(), breakpoints_.end(),
            [current](const FrameBreakpoint& breakpoint) {
                return breakpoint.frame == current;
            });
        if (found == breakpoints_.end()) {
            return false;
        }
        stop_reason_ = QStringLiteral("breakpoint");
        stop_breakpoint_ = found->id;
        stop_detail_ = QString::number(current);
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

    struct FrameBreakpoint {
        int id = 0;
        std::uint64_t frame = 0;
    };

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
    QTcpServer server_;
    std::map<QTcpSocket*, QByteArray> buffers_;
    std::vector<FrameBreakpoint> breakpoints_;
    int next_breakpoint_id_ = 0;
    QString stop_reason_ = QStringLiteral("none");
    int stop_breakpoint_ = 0;
    QString stop_detail_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime

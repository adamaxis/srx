#include "frame_debug_server.hpp"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace shadowrun::ghidra_conversion::qt_runtime;

namespace {

QJsonObject response(const QByteArray& bytes) {
    return QJsonDocument::fromJson(bytes).object();
}

bool wait_for_response(QCoreApplication& app, QTcpSocket& socket) {
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(2);
    while (socket.bytesAvailable() == 0 &&
           std::chrono::steady_clock::now() < deadline) {
        app.processEvents();
        socket.waitForReadyRead(5);
    }
    return socket.bytesAvailable() != 0;
}

bool write_recording(
    const std::filesystem::path& path,
    const std::vector<std::pair<std::uint16_t, std::uint64_t>>& frames) {
    shadowrun::ghidra_conversion::debug::KeyboardInputLog log(
        {shadowrun::ghidra_conversion::debug::InputLogConfig::Mode::record,
         path.string()});
    for (const auto& [seed, fingerprint] : frames) {
        log.record_completed_frame(seed, fingerprint);
    }
    return log.finish();
}

} // namespace

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    shadowrun::ghidra_conversion::debug::FrameStepper stepper(true);
    int queued_key = 0;
    bool queued_pressed = false;
    std::uint64_t current_frame = 88U;
    QStringList frame_events;
    QString dumped_path;
    QJsonObject state;
    state.insert(QStringLiteral("app.phase"), QStringLiteral("field_ready"));
    state.insert(QStringLiteral("app.paused"), true);
    QJsonObject actor;
    actor.insert(QStringLiteral("x"), 321);
    actor.insert(QStringLiteral("y"), 456);
    state.insert(QStringLiteral("actor.test"), actor);

    FrameDebugServer server(
        0U, stepper, [&state] { return state; }, [&current_frame] { return current_frame; },
        [] { return QStringLiteral("field_ready"); },
        [&queued_key, &queued_pressed](const int key, const bool pressed) {
            queued_key = key;
            queued_pressed = pressed;
        },
        [&frame_events] {
            auto events = frame_events;
            frame_events.clear();
            return events;
        },
        [&dumped_path](const QString& path, QString* error) {
            if (path == QStringLiteral("fail.bmp")) {
                if (error != nullptr) {
                    *error = QStringLiteral("simulated image write failure");
                }
                return false;
            }
            dumped_path = path;
            return true;
        });
    if (server.port() == 0U) {
        std::cerr << "debug server did not bind an ephemeral localhost port\n";
        return 1;
    }

    auto result = response(server.handle_line(R"({"cmd":"hash"})"));
    const auto initial_fingerprint =
        result.value(QStringLiteral("fingerprint")).toString();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        initial_fingerprint.isEmpty() ||
        result.value(QStringLiteral("scope")).toString() !=
            QStringLiteral("debug_inspection_fields")) {
        std::cerr << "hash command did not return a scoped inspector fingerprint\n";
        return 1;
    }
    state.insert(QStringLiteral("app.paused"), false);
    result = response(server.handle_line(R"({"cmd":"hash"})"));
    if (result.value(QStringLiteral("fingerprint")).toString() !=
        initial_fingerprint) {
        std::cerr << "host pause state incorrectly changed the inspector fingerprint\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"dump","path":"frame.bmp"})"));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        dumped_path != QStringLiteral("frame.bmp") ||
        result.value(QStringLiteral("frame")).toInt() != 88) {
        std::cerr << "dump command did not route the current frame to the image provider\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"dump","path":"fail.bmp"})"));
    if (result.value(QStringLiteral("ok")).toBool() ||
        result.value(QStringLiteral("error")).toString() !=
            QStringLiteral("simulated image write failure")) {
        std::cerr << "dump command did not report image write failures\n";
        return 1;
    }

    result = response(server.handle_line(R"({"cmd":"status"})"));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        result.value(QStringLiteral("frame")).toInt() != 88 ||
        result.value(QStringLiteral("mode")).toString() !=
            QStringLiteral("running")) {
        std::cerr << "status command returned invalid state\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"pause"})"));
    if (!result.value(QStringLiteral("ok")).toBool() || !stepper.paused()) {
        std::cerr << "pause command did not pause the source-frame stepper\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"step","n":3})"));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        stepper.pending_steps() != 3U || stepper.frames_to_advance(2U) != 2U ||
        stepper.frames_to_advance(2U) != 1U) {
        std::cerr << "step command did not queue the requested source frames\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"key","key":"a","pressed":true})"));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        !result.value(QStringLiteral("queued")).toBool() ||
        queued_key != Qt::Key_Z || !queued_pressed) {
        std::cerr << "key command did not queue the mapped Qt key transition\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"press","key":"b","frames":2,"release":1})"));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        !result.value(QStringLiteral("queued")).toBool() ||
        result.value(QStringLiteral("release_at")).toInt() != 90 ||
        result.value(QStringLiteral("pending_steps")).toInt() != 3 ||
        queued_key != Qt::Key_X || !queued_pressed) {
        std::cerr << "press command did not queue a source-frame-held button\n";
        return 1;
    }
    if (response(server.handle_line(
            R"({"cmd":"press","key":"c","frames":0})"))
            .value(QStringLiteral("ok")).toBool()) {
        std::cerr << "press command accepted a zero-frame hold\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"break","kind":"frame","frame":91})"));
    const auto breakpoint_id = result.value(QStringLiteral("id")).toInt();
    if (!result.value(QStringLiteral("ok")).toBool() || breakpoint_id <= 0 ||
        server.check_source_frame_breakpoints()) {
        std::cerr << "frame breakpoint did not register for a future frame\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"break","kind":"watch","field":"actor.test.x"})"));
    const auto watchpoint_id = result.value(QStringLiteral("id")).toInt();
    if (!result.value(QStringLiteral("ok")).toBool() || watchpoint_id <= 0) {
        std::cerr << "watchpoint did not register for an inspected field\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"break","kind":"event","pattern":"field\\.world\\.encounter"})"));
    const auto event_breakpoint_id = result.value(QStringLiteral("id")).toInt();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        event_breakpoint_id <= 0) {
        std::cerr << "event breakpoint did not register for a source boundary\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"list"})"));
    const auto listed = result.value(QStringLiteral("breakpoints")).toArray();
    if (!result.value(QStringLiteral("ok")).toBool() || listed.size() != 3 ||
        listed.at(0).toObject().value(QStringLiteral("frame")).toInt() != 91) {
        std::cerr << "list command did not return the registered breakpoints\n";
        return 1;
    }
    frame_events.push_back(QStringLiteral("field.world.encounter.hellhound"));
    current_frame = 89U;
    if (!server.check_source_frame_breakpoints() || !stepper.paused()) {
        std::cerr << "event breakpoint did not stop on a matching source boundary\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"status"})"));
    if (result.value(QStringLiteral("breakpoint")).toInt() !=
            event_breakpoint_id ||
        result.value(QStringLiteral("detail")).toString() !=
            QStringLiteral("field.world.encounter.hellhound")) {
        std::cerr << "event breakpoint stop did not report its source boundary\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"resume"})"));
    if (!result.value(QStringLiteral("ok")).toBool() || stepper.paused()) {
        std::cerr << "resume command did not continue after an event breakpoint\n";
        return 1;
    }
    actor.insert(QStringLiteral("x"), 322);
    state.insert(QStringLiteral("actor.test"), actor);
    current_frame = 90U;
    if (!server.check_source_frame_breakpoints() || !stepper.paused()) {
        std::cerr << "watchpoint did not stop after its field changed\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"status"})"));
    if (result.value(QStringLiteral("breakpoint")).toInt() != watchpoint_id ||
        result.value(QStringLiteral("detail")).toString() !=
            QStringLiteral("actor.test.x: 321 -> 322")) {
        std::cerr << "watchpoint stop did not identify the changed field and values\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"resume"})"));
    if (!result.value(QStringLiteral("ok")).toBool() || stepper.paused()) {
        std::cerr << "resume command did not continue after a watchpoint\n";
        return 1;
    }
    current_frame = 91U;
    if (!server.check_source_frame_breakpoints() || !stepper.paused()) {
        std::cerr << "frame breakpoint did not pause on its source frame\n";
        return 1;
    }
    if (queued_key != Qt::Key_X || queued_pressed) {
        std::cerr << "press command did not queue release at the requested frame\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"status"})"));
    if (result.value(QStringLiteral("stop")).toString() !=
            QStringLiteral("breakpoint") ||
        result.value(QStringLiteral("breakpoint")).toInt() != breakpoint_id ||
        result.value(QStringLiteral("detail")).toString() !=
            QStringLiteral("91")) {
        std::cerr << "status did not report the breakpoint stop details\n";
        return 1;
    }
    result = response(server.handle_line(
        QByteArray("{\"cmd\":\"delete\",\"id\":") +
        QByteArray::number(breakpoint_id) + "}"));
    if (!result.value(QStringLiteral("ok")).toBool()) {
        std::cerr << "delete command did not remove the frame breakpoint\n";
        return 1;
    }
    result = response(server.handle_line(
        QByteArray("{\"cmd\":\"delete\",\"id\":") +
        QByteArray::number(event_breakpoint_id) + "}"));
    if (!result.value(QStringLiteral("ok")).toBool()) {
        std::cerr << "delete command did not remove the event breakpoint\n";
        return 1;
    }
    result = response(server.handle_line(
        QByteArray("{\"cmd\":\"delete\",\"id\":") +
        QByteArray::number(watchpoint_id) + "}"));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        !response(server.handle_line(R"({"cmd":"list"})"))
             .value(QStringLiteral("breakpoints")).toArray().isEmpty()) {
        std::cerr << "delete command did not remove the frame breakpoint\n";
        return 1;
    }
    current_frame = 92U;
    if (server.check_source_frame_breakpoints()) {
        std::cerr << "deleted frame breakpoint still stopped execution\n";
        return 1;
    }

    const auto temp = std::filesystem::temp_directory_path();
    const auto first_log = temp / "shadowrun-debug-divergence-first.srxkey";
    const auto changed_log = temp / "shadowrun-debug-divergence-changed.srxkey";
    const auto changed_seed_log =
        temp / "shadowrun-debug-divergence-seed.srxkey";
    const std::vector<std::pair<std::uint16_t, std::uint64_t>> first_frames{
        {0x100U, 0x1111U}, {0x200U, 0x2222U}, {0x300U, 0x3333U}};
    const std::vector<std::pair<std::uint16_t, std::uint64_t>> changed_frames{
        {0x100U, 0x1111U}, {0x200U, 0x9999U}, {0x300U, 0x3333U}};
    const std::vector<std::pair<std::uint16_t, std::uint64_t>> changed_seed_frames{
        {0x100U, 0x1111U}, {0x201U, 0x2222U}, {0x300U, 0x3333U}};
    if (!write_recording(first_log, first_frames) ||
        !write_recording(changed_log, changed_frames) ||
        !write_recording(changed_seed_log, changed_seed_frames)) {
        std::cerr << "could not prepare the debugger divergence recordings\n";
        return 1;
    }
    const auto divergence_request = [](const std::filesystem::path& a,
                                       const std::filesystem::path& b) {
        QJsonObject request;
        request.insert(QStringLiteral("cmd"), QStringLiteral("diverge"));
        request.insert(QStringLiteral("a"), QString::fromStdString(a.string()));
        request.insert(QStringLiteral("b"), QString::fromStdString(b.string()));
        return QJsonDocument(request).toJson(QJsonDocument::Compact);
    };
    result = response(server.handle_line(
        divergence_request(first_log, changed_log)));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        !result.value(QStringLiteral("found")).toBool() ||
        result.value(QStringLiteral("frame")).toInt() != 2 ||
        result.value(QStringLiteral("reason")).toString() !=
            QStringLiteral("inspection_fingerprint")) {
        std::cerr << "diverge did not locate the first differing inspector fingerprint\n";
        return 1;
    }
    result = response(server.handle_line(
        divergence_request(first_log, changed_seed_log)));
    if (!result.value(QStringLiteral("found")).toBool() ||
        result.value(QStringLiteral("frame")).toInt() != 2 ||
        result.value(QStringLiteral("reason")).toString() !=
            QStringLiteral("rom_seed")) {
        std::cerr << "diverge did not locate the first differing ROM seed\n";
        return 1;
    }
    result = response(server.handle_line(
        divergence_request(first_log, first_log)));
    if (!result.value(QStringLiteral("ok")).toBool() ||
        result.value(QStringLiteral("found")).toBool() ||
        result.value(QStringLiteral("frames_compared")).toInt() != 3) {
        std::cerr << "diverge reported a mismatch for identical recordings\n";
        return 1;
    }
    std::error_code ignored;
    std::filesystem::remove(first_log, ignored);
    std::filesystem::remove(changed_log, ignored);
    std::filesystem::remove(changed_seed_log, ignored);
    result = response(server.handle_line(
        R"({"cmd":"inspect","prefix":"actor.test"})"));
    const auto inspected = result.value(QStringLiteral("fields")).toObject();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        inspected.size() != 2 ||
        inspected.value(QStringLiteral("actor.test.x")).toInt() != 322 ||
        inspected.value(QStringLiteral("actor.test.y")).toInt() != 456) {
        std::cerr << "inspect prefix did not flatten the requested state fields\n";
        return 1;
    }
    if (response(server.handle_line("{broken")).value(
            QStringLiteral("ok")).toBool()) {
        std::cerr << "invalid JSON request was accepted\n";
        return 1;
    }

    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, server.port());
    if (!client.waitForConnected(1000)) {
        std::cerr << "debug client could not connect to localhost server\n";
        return 1;
    }
    client.write("{\"cmd\":\"status\"}\n");
    if (!client.waitForBytesWritten(1000) ||
        !wait_for_response(application, client)) {
        std::cerr << "debug server did not answer a line-delimited request\n";
        return 1;
    }
    const auto network_response = response(client.readLine());
    if (!network_response.value(QStringLiteral("ok")).toBool() ||
        network_response.value(QStringLiteral("port")).toInt() != server.port()) {
        std::cerr << "network response did not include a valid debugger status\n";
        return 1;
    }
    return 0;
}

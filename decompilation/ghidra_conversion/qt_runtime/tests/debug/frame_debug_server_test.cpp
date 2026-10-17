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
    const std::vector<std::pair<std::uint16_t, std::uint64_t>>& frames,
    const std::vector<std::pair<std::uint64_t, std::string>>& source_events = {}) {
    shadowrun::ghidra_conversion::debug::KeyboardInputLog log(
        {shadowrun::ghidra_conversion::debug::InputLogConfig::Mode::record,
         path.string()});
    for (const auto& [seed, fingerprint] : frames) {
        log.record_completed_frame(seed, fingerprint);
    }
    for (const auto& [frame, name] : source_events) {
        log.record_source_event(frame, name);
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
    state.insert(QStringLiteral("field.scene"), 2);
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
    result = response(server.handle_line(
        R"({"cmd":"break","kind":"scene","scene":3})"));
    const auto scene_breakpoint_id = result.value(QStringLiteral("id")).toInt();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        scene_breakpoint_id <= 0 ||
        result.value(QStringLiteral("scene")).toInt() != 3) {
        std::cerr << "scene breakpoint did not register for a target map\n";
        return 1;
    }
    if (response(server.handle_line(
            R"({"cmd":"break","kind":"scene","scene":65536})"))
            .value(QStringLiteral("ok")).toBool()) {
        std::cerr << "scene breakpoint accepted an out-of-range map id\n";
        return 1;
    }
    result = response(server.handle_line(
        R"({"cmd":"break","kind":"rng","pattern":"bounded\\.bound=6"})"));
    const auto rng_breakpoint_id = result.value(QStringLiteral("id")).toInt();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        rng_breakpoint_id <= 0 ||
        result.value(QStringLiteral("pattern")).toString() !=
            QStringLiteral("bounded\\.bound=6")) {
        std::cerr << "RNG breakpoint did not register its event filter\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"list"})"));
    const auto listed = result.value(QStringLiteral("breakpoints")).toArray();
    if (!result.value(QStringLiteral("ok")).toBool() || listed.size() != 5 ||
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
    const auto remaining_breakpoints = response(server.handle_line(
        R"({"cmd":"list"})"))
                                          .value(QStringLiteral("breakpoints"))
                                          .toArray();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        remaining_breakpoints.size() != 2) {
        std::cerr << "delete command did not remove the frame breakpoint\n";
        return 1;
    }
    frame_events.push_back(
        QStringLiteral("rng.bounded.bound=6.result=4.seed=0x1234"));
    current_frame = 92U;
    if (!server.check_source_frame_breakpoints()) {
        std::cerr << "RNG breakpoint did not stop on a matching source random call\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"status"})"));
    if (result.value(QStringLiteral("breakpoint")).toInt() !=
            rng_breakpoint_id ||
        result.value(QStringLiteral("detail")).toString() !=
            QStringLiteral("rng.bounded.bound=6.result=4.seed=0x1234")) {
        std::cerr << "RNG breakpoint did not report the observed source call\n";
        return 1;
    }
    if (!response(server.handle_line(
             QByteArray("{\"cmd\":\"delete\",\"id\":") +
             QByteArray::number(rng_breakpoint_id) + "}"))
             .value(QStringLiteral("ok")).toBool()) {
        std::cerr << "delete command did not remove the RNG breakpoint\n";
        return 1;
    }
    state.insert(QStringLiteral("field.scene"), 3);
    current_frame = 93U;
    if (!server.check_source_frame_breakpoints()) {
        std::cerr << "scene breakpoint did not stop after a map transition\n";
        return 1;
    }
    result = response(server.handle_line(R"({"cmd":"status"})"));
    if (result.value(QStringLiteral("breakpoint")).toInt() !=
            scene_breakpoint_id ||
        result.value(QStringLiteral("detail")).toString() !=
            QStringLiteral("field.scene: 2 -> 3")) {
        std::cerr << "scene breakpoint did not report the source and destination maps\n";
        return 1;
    }
    if (!response(server.handle_line(
             QByteArray("{\"cmd\":\"delete\",\"id\":") +
             QByteArray::number(scene_breakpoint_id) + "}"))
             .value(QStringLiteral("ok")).toBool()) {
        std::cerr << "delete command did not remove the scene breakpoint\n";
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
    if (!write_recording(first_log, first_frames,
                         {{0U, "0x5AEC6.render_time_record"},
                          {1U, "field.world.encounter.hellhound"},
                          {2U, "rng.bounded.bound=6.result=4.seed=0x1234"}}) ||
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
    QJsonObject event_request;
    event_request.insert(QStringLiteral("cmd"), QStringLiteral("events"));
    event_request.insert(QStringLiteral("path"),
                         QString::fromStdString(first_log.string()));
    event_request.insert(QStringLiteral("from"), 2);
    event_request.insert(QStringLiteral("to"), 2);
    event_request.insert(QStringLiteral("pattern"),
                         QStringLiteral("field\\.world\\.encounter"));
    result = response(server.handle_line(
        QJsonDocument(event_request).toJson(QJsonDocument::Compact)));
    const auto event_entries = result.value(QStringLiteral("events")).toArray();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        event_entries.size() != 1 ||
        event_entries.at(0).toObject().value(QStringLiteral("frame")).toInt() != 2 ||
        event_entries.at(0).toObject().value(QStringLiteral("name")).toString() !=
            QStringLiteral("field.world.encounter.hellhound")) {
        std::cerr << "events query did not filter the recorded source boundary\n";
        return 1;
    }
    event_request.insert(QStringLiteral("from"), 3);
    event_request.insert(QStringLiteral("to"), 3);
    event_request.insert(QStringLiteral("pattern"), QStringLiteral("^rng\\."));
    result = response(server.handle_line(
        QJsonDocument(event_request).toJson(QJsonDocument::Compact)));
    const auto rng_entries = result.value(QStringLiteral("events")).toArray();
    if (!result.value(QStringLiteral("ok")).toBool() ||
        rng_entries.size() != 1 ||
        rng_entries.at(0).toObject().value(QStringLiteral("name")).toString() !=
            QStringLiteral("rng.bounded.bound=6.result=4.seed=0x1234")) {
        std::cerr << "events query did not expose recorded RNG call details\n";
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

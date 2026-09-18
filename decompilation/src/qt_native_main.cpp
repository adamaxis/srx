#ifdef _WIN32

#include "shadowrun/graphics.hpp"
#include "shadowrun/native_port.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include <QWidget>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using shadowrun::native::Button;
using shadowrun::native::InputState;
using shadowrun::native::NativePortAssets;
using shadowrun::native::NativePortShell;
using shadowrun::native::SoftwareScreen;

std::optional<std::filesystem::path> find_asset_pack(int argc, char** argv) {
    if (argc > 1) {
        const std::filesystem::path explicit_pack(argv[1]);
        if (explicit_pack.extension() != ".srna") {
            throw std::invalid_argument(
                "shadowrun_qt expects an extracted .srna asset pack");
        }
        return explicit_pack;
    }

    const auto application =
        std::filesystem::path(QCoreApplication::applicationDirPath().toStdWString());
    const auto current = std::filesystem::current_path();
    const std::filesystem::path candidates[] = {
        application / "assets" / "native" / "shadowrun_native.srna",
        application / ".." / "assets" / "native" / "shadowrun_native.srna",
        current / "assets" / "native" / "shadowrun_native.srna",
        current / ".." / "assets" / "native" / "shadowrun_native.srna",
    };
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::uint16_t key_mask(int key) {
    switch (key) {
    case Qt::Key_Up: return shadowrun::native::kGenesisInputUp;
    case Qt::Key_Down: return shadowrun::native::kGenesisInputDown;
    case Qt::Key_Left: return shadowrun::native::kGenesisInputLeft;
    case Qt::Key_Right: return shadowrun::native::kGenesisInputRight;
    case Qt::Key_Z: return shadowrun::native::kGenesisInputA;
    case Qt::Key_X: return shadowrun::native::kGenesisInputB;
    case Qt::Key_C: return shadowrun::native::kGenesisInputC;
    case Qt::Key_Return:
    case Qt::Key_Enter: return shadowrun::native::kGenesisInputStart;
    default: return 0;
    }
}

class ShadowrunWidget final : public QWidget {
public:
    explicit ShadowrunWidget(NativePortShell game, QWidget* parent = nullptr)
        : QWidget(parent), game_(std::move(game)), screen_() {
        setWindowTitle(QStringLiteral("Shadowrun - Native C++ Port (Qt)"));
        setFocusPolicy(Qt::StrongFocus);
        setMinimumSize(640, 448);
        resize(960, 672);

        timer_.setInterval(16);
        connect(&timer_, &QTimer::timeout, this, [this]() { tick(); });
        timer_.start();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QImage frame(screen_.width(), screen_.height(),
                     QImage::Format_RGB32);
        auto* destination = reinterpret_cast<QRgb*>(frame.bits());
        const auto& source = screen_.pixels();
        for (std::size_t index = 0; index < source.size(); ++index) {
            const auto packed = source[index];
            destination[index] = qRgb(
                static_cast<int>((packed >> 16U) & 0xFFU),
                static_cast<int>((packed >> 8U) & 0xFFU),
                static_cast<int>(packed & 0xFFU));
        }

        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        const auto target = QSize(screen_.width(), screen_.height())
                                .scaled(size(), Qt::KeepAspectRatio);
        const auto x = (width() - target.width()) / 2;
        const auto y = (height() - target.height()) / 2;
        painter.drawImage(QRect(x, y, target.width(), target.height()), frame);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->isAutoRepeat()) {
            return;
        }
        held_ = static_cast<std::uint16_t>(held_ | key_mask(event->key()));
        event->accept();
    }

    void keyReleaseEvent(QKeyEvent* event) override {
        if (event->isAutoRepeat()) {
            return;
        }
        held_ = static_cast<std::uint16_t>(
            held_ & static_cast<std::uint16_t>(~key_mask(event->key())));
        event->accept();
    }

private:
    void tick() {
        const auto pressed = static_cast<std::uint16_t>(
            held_ & static_cast<std::uint16_t>(~previous_));
        game_.update(shadowrun::native::decode_genesis_controller_word(
            static_cast<std::uint16_t>(held_ | (pressed << 8U))));
        previous_ = held_;
        game_.render(screen_);
        update();
    }

    NativePortShell game_;
    SoftwareScreen screen_;
    QTimer timer_;
    std::uint16_t held_ = 0;
    std::uint16_t previous_ = 0;
};

} // namespace

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    try {
        const auto pack = find_asset_pack(argc, argv);
        if (!pack) {
            throw std::runtime_error(
                "No extracted assets found. Run tools/extract_assets.ps1 "
                "before starting the Qt frontend.");
        }
        ShadowrunWidget window(
            NativePortShell("SHADOWRUN",
                            shadowrun::graphics::read_native_asset_pack(
                                pack->string())));
        window.show();
        return application.exec();
    } catch (const std::exception& error) {
        qCritical("%s", error.what());
        return 1;
    }
}

#else

int main() { return 0; }

#endif

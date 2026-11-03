#include <QImage>
#include <QString>

#include <array>
#include <iostream>
#include <string_view>

namespace {

constexpr int kWidth = 320;
constexpr int kHeight = 224;

struct CapturePair {
    std::string_view name;
    QString actual_path;
    QString gpgx_path;
};

bool matches_gpgx(const CapturePair& capture) {
    const QImage actual(capture.actual_path);
    const QImage gpgx(capture.gpgx_path);
    if (actual.isNull() || gpgx.isNull() ||
        actual.size() != QSize(kWidth, kHeight) ||
        gpgx.size() != QSize(kWidth, kHeight)) {
        std::cerr << capture.name
                  << " capture could not load two 320x224 images\n";
        return false;
    }

    std::size_t differences = 0U;
    int first_x = -1;
    int first_y = -1;
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            if (actual.pixel(x, y) == gpgx.pixel(x, y)) {
                continue;
            }
            ++differences;
            if (first_x < 0) {
                first_x = x;
                first_y = y;
            }
        }
    }
    if (differences != 0U) {
        std::cerr << capture.name << " differs from the independent GPGX "
                  << "frame at " << first_x << ',' << first_y << " ("
                  << differences << " pixels)\n";
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 7) {
        std::cerr << "usage: karma_screen_distinction_test "
                     "<command-actual.bmp> <hotel-preview-actual.bmp> "
                     "<hotel-prompt-actual.bmp> <command-gpgx.png> "
                     "<hotel-preview-gpgx.png> <hotel-prompt-gpgx.png>\n";
        return 2;
    }

    const std::array<CapturePair, 3U> captures{{
        {"command status",
         QString::fromLocal8Bit(argv[1]), QString::fromLocal8Bit(argv[4])},
        {"Hotel preview",
         QString::fromLocal8Bit(argv[2]), QString::fromLocal8Bit(argv[5])},
        {"Hotel confirmation",
         QString::fromLocal8Bit(argv[3]), QString::fromLocal8Bit(argv[6])},
    }};
    for (const auto& capture : captures) {
        if (!matches_gpgx(capture)) {
            return 1;
        }
    }
    return 0;
}

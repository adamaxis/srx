#include <QColor>
#include <QImage>
#include <QString>

#include <iostream>

namespace {

constexpr int kWidth = 320;
constexpr int kHeight = 224;
constexpr int kKarmaDetailY = 200;
constexpr int kKarmaDetailHeight = 8;
constexpr int kKarmaTotalX = 120;
constexpr int kKarmaTotalY = 160;
constexpr int kKarmaTotalWidth = 40;
constexpr int kKarmaTotalHeight = 8;

bool contains_color(const QImage& image, const QColor& color, const int x,
                    const int y, const int width, const int height) {
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            if (image.pixelColor(column, row) == color) {
                return true;
            }
        }
    }
    return false;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: karma_screen_distinction_test "
                     "<command-view-only.bmp> <hotel-karma-spend.bmp>\n";
        return 2;
    }

    const QImage view_only(QString::fromLocal8Bit(argv[1]));
    const QImage hotel_spending(QString::fromLocal8Bit(argv[2]));
    if (view_only.isNull() || hotel_spending.isNull() ||
        view_only.size() != QSize(kWidth, kHeight) ||
        hotel_spending.size() != QSize(kWidth, kHeight)) {
        std::cerr << "Karma screen distinction QA could not load two "
                     "320x224 route captures\n";
        return 1;
    }

    bool detail_differs = false;
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            if (view_only.pixel(x, y) == hotel_spending.pixel(x, y)) {
                continue;
            }
            if (y < kKarmaDetailY ||
                y >= kKarmaDetailY + kKarmaDetailHeight) {
                std::cerr << "Karma screen variants differ outside the "
                             "source detail line at ("
                          << x << ',' << y << ")\n";
                return 1;
            }
            detail_differs = true;
        }
    }

    const auto current_karma_color = QColor(0, 219, 0);
    const auto hotel_upgrade_color = QColor(109, 73, 146);
    if (!detail_differs ||
        !contains_color(view_only, current_karma_color, kKarmaTotalX,
                        kKarmaTotalY, kKarmaTotalWidth,
                        kKarmaTotalHeight) ||
        !contains_color(hotel_spending, current_karma_color, kKarmaTotalX,
                        kKarmaTotalY, kKarmaTotalWidth,
                        kKarmaTotalHeight) ||
        contains_color(view_only, hotel_upgrade_color, 64, kKarmaDetailY,
                       224, kKarmaDetailHeight) ||
        !contains_color(hotel_spending, hotel_upgrade_color, 64,
                        kKarmaDetailY, 224, kKarmaDetailHeight)) {
        std::cerr << "Karma screen variants did not show the distinct "
                     "current-total and hotel upgrade/cost presentations\n";
        return 1;
    }

    return 0;
}

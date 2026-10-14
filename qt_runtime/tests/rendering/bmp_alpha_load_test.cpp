// The extracted sprite BMPs carry their baked alpha in the fourth pixel
// byte. The writer declares that channel with a BI_BITFIELDS mask because
// Qt's BMP loader only honors alpha for that header form; a plain 32bpp BMP
// loads fully opaque and the runtime would have to guess transparency by
// wiping black pixels, which erases the sprites' black outlines.
#include <QColor>
#include <QFile>
#include <QImage>
#include <QString>

#include <cstdio>

int main(int argc, char** argv) {
    QString path = QString::fromUtf8(SHADOWRUN_GHIDRA_ASSET_ROOT)
                       .append("/field/entities/idle-0.bmp");
    if (argc > 1) {
        path = QString::fromLocal8Bit(argv[1]);
    }
    const QImage image(path);
    if (image.isNull()) {
        std::fprintf(stderr, "bmp alpha test: did not load %s (exists=%d)\n",
                     path.toUtf8().constData(),
                     QFile::exists(path) ? 1 : 0);
        return 1;
    }
    if (image.width() != 24 || image.height() != 31) {
        std::fprintf(stderr, "bmp alpha test: unexpected sprite size %dx%d\n",
                     image.width(), image.height());
        return 2;
    }
    int transparent = 0;
    int opaque_black = 0;
    int opaque = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const auto pixel = image.pixelColor(x, y);
            if (pixel.alpha() == 0) {
                ++transparent;
                continue;
            }
            ++opaque;
            if (pixel.red() == 0 && pixel.green() == 0 && pixel.blue() == 0) {
                ++opaque_black;
            }
        }
    }
    if (transparent == 0) {
        std::fprintf(stderr,
                     "bmp alpha test: color-zero holes were lost on load\n");
        return 3;
    }
    if (opaque_black == 0) {
        std::fprintf(stderr,
                     "bmp alpha test: opaque black pixels were lost on load\n");
        return 4;
    }
    std::printf("bmp_alpha=ok transparent=%d opaque=%d opaque_black=%d\n",
                transparent, opaque, opaque_black);
    return 0;
}

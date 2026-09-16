#ifdef _WIN32

#include "shadowrun/native_port.hpp"

#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

using shadowrun::native::Button;
using shadowrun::native::InputState;
using shadowrun::native::NativePortShell;
using shadowrun::native::SoftwareScreen;
using shadowrun::native::button_mask;

struct WindowContext {
    SoftwareScreen* screen = nullptr;
    bool quit = false;
};

std::vector<std::uint8_t> read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

std::vector<std::uint8_t> load_rom(int argc, char** argv) {
    if (argc > 1) {
        return read_file(argv[1]);
    }
    const auto current = std::filesystem::current_path();
    const std::filesystem::path relative_candidates[] = {
        current / ".." / "input" / "shadowrun_genesis_us.bin",
        current / "decompilation" / "input" / "shadowrun_genesis_us.bin",
    };
    for (const auto& candidate : relative_candidates) {
        auto rom = read_file(candidate);
        if (!rom.empty()) {
            return rom;
        }
    }
    return {};
}

LRESULT CALLBACK window_proc(HWND window, UINT message,
                             WPARAM w_param, LPARAM l_param) {
    auto* context = reinterpret_cast<WindowContext*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(l_param);
        context = static_cast<WindowContext*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(context));
    }

    switch (message) {
    case WM_CLOSE:
        if (context != nullptr) {
            context->quit = true;
        }
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        const auto device = BeginPaint(window, &paint);
        if (context != nullptr && context->screen != nullptr) {
            BITMAPINFO bitmap{};
            bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitmap.bmiHeader.biWidth = context->screen->width();
            bitmap.bmiHeader.biHeight = -context->screen->height();
            bitmap.bmiHeader.biPlanes = 1;
            bitmap.bmiHeader.biBitCount = 32;
            bitmap.bmiHeader.biCompression = BI_RGB;

            RECT client{};
            GetClientRect(window, &client);
            StretchDIBits(
                device, 0, 0, client.right, client.bottom,
                0, 0, context->screen->width(), context->screen->height(),
                context->screen->pixels().data(), &bitmap, DIB_RGB_COLORS,
                SRCCOPY);
        }
        EndPaint(window, &paint);
        return 0;
    }
    default:
        return DefWindowProcW(window, message, w_param, l_param);
    }
}

std::uint16_t poll_buttons() {
    std::uint16_t held = 0;
    const auto add_key = [&](int key, Button button) {
        if ((GetAsyncKeyState(key) & 0x8000) != 0) {
            held = static_cast<std::uint16_t>(held | button_mask(button));
        }
    };
    add_key(VK_UP, Button::Up);
    add_key(VK_DOWN, Button::Down);
    add_key(VK_LEFT, Button::Left);
    add_key(VK_RIGHT, Button::Right);
    add_key('Z', Button::A);
    add_key('X', Button::B);
    add_key('C', Button::C);
    add_key(VK_RETURN, Button::Start);
    return held;
}

} // namespace

int main(int argc, char** argv) {
    constexpr wchar_t kClassName[] = L"ShadowrunNativePort";
    WNDCLASSW window_class{};
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = GetModuleHandleW(nullptr);
    window_class.lpszClassName = kClassName;
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    if (RegisterClassW(&window_class) == 0) {
        return 1;
    }

    SoftwareScreen screen;
    const auto rom = load_rom(argc, argv);
    NativePortShell game("SHADOWRUN");
    if (!rom.empty()) {
        game = NativePortShell("SHADOWRUN", rom);
    }
    WindowContext context{&screen, false};

    RECT desired{0, 0, 960, 672};
    AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, FALSE);
    const auto window = CreateWindowExW(
        0, kClassName, L"Shadowrun - Native C++ Port",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        desired.right - desired.left, desired.bottom - desired.top,
        nullptr, nullptr, window_class.hInstance, &context);
    if (window == nullptr) {
        return 1;
    }
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    std::uint16_t previous_held = 0;
    while (!context.quit) {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
            if (message.message == WM_QUIT) {
                context.quit = true;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (context.quit) {
            break;
        }

        const auto held = poll_buttons();
        game.update(InputState{
            held, static_cast<std::uint16_t>(held & ~previous_held)});
        previous_held = held;
        game.render(screen);
        InvalidateRect(window, nullptr, FALSE);
        UpdateWindow(window);
        Sleep(16);
    }
    return 0;
}

#else

int main() { return 0; }

#endif

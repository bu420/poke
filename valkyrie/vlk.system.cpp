#include "vlk.system.hpp"

#define GDIPVER 0x0110
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <fstream>

#include "vlk.util.hpp"

using namespace vlk;

constexpr auto vlk_window_class_name = L"PWA Window Class";

LRESULT CALLBACK win_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

void vlk::initialize() {
    WNDCLASS wc{0};
    wc.lpfnWndProc = win_proc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = vlk_window_class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.cbWndExtra = sizeof(window *);
    RegisterClass(&wc);

    // Kept here until we need it outside this function...
    ULONG_PTR m_gdiplusToken;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
}

void vlk::terminate() {
}

double vlk::get_elapsed_time() {
    LARGE_INTEGER elapsed;
    QueryPerformanceCounter(&elapsed);
    return (double)(elapsed.QuadPart * 1000000 / get_ticks_per_sec()) / 1000;
}

i64 vlk::get_ticks_per_sec() {
    LARGE_INTEGER ticks_per_second;
    QueryPerformanceFrequency(&ticks_per_second);
    return ticks_per_second.QuadPart;
}

image vlk::load_image(std::filesystem::path path) {
    if (not path.is_absolute()) {
        path = std::filesystem::current_path() / path;
    }

    auto image = Gdiplus::Bitmap::FromFile(path.wstring().c_str());
    
    auto status = image->GetLastStatus();

    if (status == Gdiplus::FileNotFound) {
        throw std::runtime_error(
            std::format("Valkyrie: file not found {}.", path.string()));
    }
    else if (status != Gdiplus::Ok) {
        throw std::runtime_error(
            std::format("Valkyrie: failed to load image {}.", path.string()));
    }

    
    if (image->GetPixelFormat() != PixelFormat32bppARGB) {
        image->ConvertFormat(PixelFormat32bppARGB,
                             Gdiplus::DitherTypeNone,
                             Gdiplus::PaletteTypeOptimal,
                             nullptr,
                             REAL_MAX);
    }

    vlk::image result{.width = image->GetWidth(),
                      .height = image->GetHeight(),
                      .channels = 4};

    result.data.resize(
        result.width * result.height * result.channels);
    
    for (size_t x = 0; x < image->GetWidth(); ++x) {
        for (size_t y = 0; y < image->GetHeight(); ++y) {
            Gdiplus::Color color;
            image->GetPixel(static_cast<INT>(x), static_cast<INT>(y), &color);

            *(result.at(x, y) + 0) = color.GetR();
            *(result.at(x, y) + 1) = color.GetG();
            *(result.at(x, y) + 2) = color.GetB();
            *(result.at(x, y) + 3) = color.GetA();
        }
    }

    return result;
}

sound vlk::load_sound(std::filesystem::path path) {
    sound result;

    std::basic_ifstream<u8> file(path, std::ios::binary);

    if (not file.is_open()) {
        throw std::runtime_error(
            std::format("Valkyrie: file not found {}.", path.string()));
    }

    result.data = std::vector<u8>((std::istreambuf_iterator<u8>(file)),
                                   std::istreambuf_iterator<u8>());

    return result;
}

void vlk::play_sound(const sound &sound) {
    PlaySound(reinterpret_cast<LPCTSTR>(&sound.data[0]), NULL, SND_ASYNC | SND_MEMORY);
}

void vlk::stop_all_sounds() {
    PlaySound(nullptr, nullptr, 0);
}

window::window(const window_params &params) :
    should_close{false},
    width{params.width},
    height{params.height},
    transparent{params.transparent} {
    // Create window.

    std::wstring wide_title;
    wide_title.assign(params.title.begin(), params.title.end());

    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;

    if (params.default_ui) {
        style |= WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
    }

    DWORD ex_style = WS_EX_APPWINDOW; // WS_EX_TOPMOST

    if (transparent) {
        ex_style |= WS_EX_LAYERED;
    }

    HWND hwnd = CreateWindowEx(ex_style,
                               vlk_window_class_name,
                               wide_title.c_str(),
                               style,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               width,
                               height,
                               NULL,
                               NULL,
                               GetModuleHandle(0),
                               NULL);

    if (not hwnd) {
        throw std::runtime_error("Valkyrie: failed to create win32 window.");
    }

    this->hwnd = hwnd;
    pixels = new u32[width * height];

    SetWindowLongPtr(hwnd, 0, (LONG_PTR)this);

    // Create background bitmap.

    HDC hdc = GetDC(hwnd);

    BITMAPINFO bmi{0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    bitmap = CreateDIBSection(hdc,
                              &bmi,
                              DIB_RGB_COLORS,
                              reinterpret_cast<void **>(&pixels),
                              NULL,
                              NULL);
    assert(bitmap);

    ReleaseDC(NULL, hdc);

    ShowWindow(hwnd, SW_SHOW);
}

void window::poll_events() {
    MSG msg;

    while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void bitmap_blit(const window &window, HDC hdc) {
    HDC memory_hdc = CreateCompatibleDC(hdc);
    HGDIOBJ old_bitmap = SelectObject(memory_hdc, window.bitmap);

    BitBlt(hdc,
           0,
           0,
           window.get_width(),
           window.get_height(),
           memory_hdc,
           0,
           0,
           SRCCOPY);

    SelectObject(hdc, old_bitmap);

    if (window.is_transparent()) {
        POINT zero = {0, 0};
        SIZE size = {window.get_width(), window.get_height()};

        BLENDFUNCTION blend{0};
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        UpdateLayeredWindow(window.hwnd,
                            hdc,
                            &zero,
                            &size,
                            memory_hdc,
                            &zero,
                            0,
                            &blend,
                            ULW_ALPHA);
    }

    DeleteDC(memory_hdc);
}

void window::swap_buffers(const color_buffer &color_buf) {
    assert(color_buf.get_width() == this->width && color_buf.get_height() == this->height &&
           "Color buffer size does not match window size.");

    // Copy color buffer (back buffer) into window front buffer.
    for (size_t x{0}; x < color_buf.get_width(); ++x) {
        for (size_t y{0}; y < color_buf.get_height(); ++y) {
            color_rgba color{color_buf.at(x, y)};

            // Win32 bitmaps use a ARGB format.
            u32 packed{static_cast<u32>(color.a) << 24 |
                       static_cast<u32>(color.r) << 16 |
                       static_cast<u32>(color.g) << 8 |
                       static_cast<u32>(color.b)};

            pixels[y * color_buf.get_width() + x] = packed;
        }
    }

    if (not transparent) {
        // Trigger redraw.
        InvalidateRect(hwnd, NULL, FALSE);
    }
    else {
        bitmap_blit(*this, GetDC(hwnd));
    }
}

bool window::get_should_close() const {
    return should_close;
}

void window::set_should_close(bool should_close) {
    this->should_close = should_close;
}

i32 window::get_width() const {
    return width;
}

i32 window::get_height() const {
    return height;
}

bool window::is_transparent() const {
    return transparent;
}

LRESULT CALLBACK win_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
    auto win = reinterpret_cast<window *>(GetWindowLongPtr(hwnd, 0));

    switch (u_msg) {
        case WM_CLOSE: {
            if (win) {
                win->set_should_close(true);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (win) {
                // Drawing to transparent window is handled elsewhere.
                if (not win->is_transparent()) {
                    bitmap_blit(*win, hdc);
                }
            }
            else {
                FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
    }

    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}
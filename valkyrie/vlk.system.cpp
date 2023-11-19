#include "vlk.system.hpp"

#include <cassert>
#include <stdexcept>

using namespace vlk;

constexpr auto vlk_window_class_name = L"PWA Window Class";

LRESULT CALLBACK win_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

void vlk::initialize() {
    WNDCLASS wc{0};
    wc.lpfnWndProc = win_proc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = vlk_window_class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.cbWndExtra = sizeof(window*);
    RegisterClass(&wc);
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

image vlk::load_image(std::string_view path) {
    return image{};
}

window::window(const window_params& params) :
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

    DWORD ex_style = WS_EX_APPWINDOW | WS_EX_TOPMOST;

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

    if (transparent) {
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

        HRGN region = CreateRectRgn(0, 0, -1, -1);
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(hwnd, &bb);
        DeleteObject(region);
    }

    this->hwnd = hwnd;
    pixels = new u32[width * height];

    SetWindowLongPtr(hwnd, 0, (LONG_PTR)this);

    ShowWindow(hwnd, SW_SHOW);

    // Create background bitmap.

    BITMAPINFO bmi{0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC screen_hdc = GetDC(NULL);

    bitmap = CreateDIBSection(screen_hdc,
                              &bmi,
                              DIB_RGB_COLORS,
                              reinterpret_cast<void**>(&pixels),
                              NULL,
                              NULL);
    assert(bitmap);

    ReleaseDC(NULL, screen_hdc);
}

void window::poll_events() {
    MSG msg;

    while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void window::swap_buffers(const color_buffer& color_buf) {
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

    // Trigger redraw.
    InvalidateRect(hwnd, NULL, FALSE);
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
    auto win = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, 0));

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
                HDC bitmap_hdc = CreateCompatibleDC(hdc);
                HGDIOBJ old_bitmap = SelectObject(bitmap_hdc, win->bitmap);

                BitBlt(hdc,
                       0,
                       0,
                       win->get_width(),
                       win->get_height(),
                       bitmap_hdc,
                       0,
                       0,
                       SRCCOPY);

                /*BLENDFUNCTION blend{0};
                blend.BlendOp = AC_SRC_OVER;
                blend.SourceConstantAlpha = 255;
                blend.AlphaFormat = AC_SRC_ALPHA;

                AlphaBlend(hdc,
                           0,
                           0,
                           win->get_width(),
                           win->get_height(),
                           bitmap_hdc,
                           0,
                           0,
                           win->get_width(),
                           win->get_height(),
                           blend);*/

                SelectObject(hdc, old_bitmap);
                DeleteDC(bitmap_hdc);

                /*if (win->is_transparent()) {
                    BLENDFUNCTION blend{0};
                    blend.BlendOp = AC_SRC_OVER;
                    blend.SourceConstantAlpha = 255;
                    blend.AlphaFormat = AC_SRC_ALPHA;

                    POINT src_point{0, 0};
                    SIZE window_size{win->get_width(), win->get_height()};

                    UpdateLayeredWindow(hwnd,
                                        GetDC(NULL),
                                        &src_point,
                                        &window_size,
                                        bitmap_hdc,
                                        &src_point,
                                        RGB(0, 0, 0),
                                        &blend,
                                        ULW_ALPHA);
                }*/
            }
            else {
                FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        /*case WM_NCHITTEST: {
            printf("yo\n");
            return HTTRANSPARENT;
        }*/
    }

    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}
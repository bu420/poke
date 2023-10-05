#include "vlk.system.hpp"

#include <cassert>

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

window::window(const std::string& title, i32 width, i32 height) :
    should_close(false), 
    width(width), 
    height(height) {
    std::wstring wide_title;
    wide_title.assign(title.begin(), title.end());

    HWND hwnd = CreateWindowEx(0, 
                               vlk_window_class_name, 
                               wide_title.c_str(), 
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, 
                               CW_USEDEFAULT, 
                               width, 
                               height, 
                               NULL, 
                               NULL, 
                               GetModuleHandle(0), 
                               NULL);

    assert(hwnd && "Failed to create win32 window.");

    this->hwnd = hwnd;
    front_buf.resize(static_cast<size_t>(width * height));

    SetWindowLongPtr(hwnd, 0, (LONG_PTR)this);

    ShowWindow(hwnd, SW_SHOW);
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
            byte3 color{color_buf.at(x, y)};

            u32 packed{static_cast<u32>(color.x) << 16 |
                       static_cast<u32>(color.y) << 8 |
                       static_cast<u32>(color.z)};

            front_buf.at(y * color_buf.get_width() + x) = packed;
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

const std::vector<u32>& window::get_front_buf() const {
    return front_buf;
}

LRESULT CALLBACK win_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
    window* win = (window*)GetWindowLongPtr(hwnd, 0);

    switch (u_msg) {
    case WM_CLOSE:
        if (win) {
            win->set_should_close(true);
        }
        return 0;

    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (win) {
            BITMAPINFO bmi{0};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = win->get_width();
            bmi.bmiHeader.biHeight = -win->get_height();
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            StretchDIBits(hdc, 
                          0, 
                          0, 
                          win->get_width(), 
                          win->get_height(), 
                          0, 
                          0, 
                          win->get_width(), 
                          win->get_height(), 
                          &win->get_front_buf().at(0),
                          &bmi, 
                          DIB_RGB_COLORS, 
                          SRCCOPY);
        }
        else {
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        }

        EndPaint(hwnd, &ps);
        return 0;

        /*case WM_SIZE:
            if (win && win->onResize) {
                win->onResize(LOWORD(lParam), HIWORD(lParam), win->userData);
            }
            return 0;*/
    }

    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}
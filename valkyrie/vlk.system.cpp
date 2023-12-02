#include "vlk.system.hpp"

#define GDIPVER 0x0110
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "vlk.util.hpp"

using namespace vlk;

constexpr auto vlk_window_class_name = L"PWA Window Class";

static IAudioClient2 *audio_client = nullptr;
static IAudioRenderClient *audio_render_client = nullptr;
static u32 audio_buffer_size_in_frames;

LRESULT CALLBACK win_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

void vlk::initialize() {
    WNDCLASS wc{0};
    wc.lpfnWndProc = win_proc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = vlk_window_class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.cbWndExtra = sizeof(window *);
    RegisterClass(&wc);

    // Initialize GDI+.

    // Kept here until we need it outside this function...
    ULONG_PTR m_gdiplusToken;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    // Initialize audio.

    assert(CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY) == S_OK);

    IMMDeviceEnumerator *device_enum = nullptr;
    assert(CoCreateInstance(__uuidof(MMDeviceEnumerator),
                            nullptr, 
                            CLSCTX_ALL,
                            __uuidof(IMMDeviceEnumerator),
                            reinterpret_cast<LPVOID *>(&device_enum)) == S_OK);

    IMMDevice *audio_device = nullptr;
    assert(device_enum->GetDefaultAudioEndpoint(eRender, eConsole, &audio_device) == S_OK);

    device_enum->Release();

    assert(audio_device->Activate(__uuidof(IAudioClient2),
                                  CLSCTX_ALL,
                                  nullptr,
                                  reinterpret_cast<LPVOID *>(&audio_client)) == S_OK);

    audio_device->Release();

    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 44100;
    format.wBitsPerSample = 16;
    format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

    DWORD flags =
        AUDCLNT_STREAMFLAGS_RATEADJUST |
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
        AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

    REFERENCE_TIME requested_sound_buffer_duration = 20000000;

    assert(audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                    flags,
                                    requested_sound_buffer_duration,
                                    0,
                                    &format,
                                    nullptr) == S_OK);

    assert(audio_client->GetService(__uuidof(IAudioRenderClient),
                                    reinterpret_cast<LPVOID *>(&audio_render_client)) == S_OK);

    assert(audio_client->GetBufferSize(&audio_buffer_size_in_frames) == S_OK);

    assert(audio_client->Start() == S_OK);
}

void vlk::terminate() {
    audio_client->Stop();
    audio_client->Release();
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

struct wav_header {
    u32 riff_id;
    u32 riff_chunk_size;
    u32 wave_id;
    u32 fmt_id;
    u32 fmt_chunk_size;
    u16 format_code;
    u16 channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
    u32 data_id;
    u32 data_chunk_size;
    u16 samples;
};

sound vlk::load_sound(std::filesystem::path path) {
    sound sound;
    sound.data = load_binary_file(path);

    // Make sure the wav is 16-bit little-endian PCM.
    auto header = *reinterpret_cast<wav_header *>(&sound.data[0]);
    assert(header.riff_id == 1179011410);
    assert(header.wave_id == 1163280727);
    assert(header.fmt_id == 544501094);
    //assert(header.data_id == 1635017060);
    assert(header.format_code == 1); // PCM.
    assert(header.channels == 2);
    assert(header.fmt_chunk_size == 16);
    assert(header.sample_rate == 44100);
    assert(header.bits_per_sample == 16);
    assert(header.block_align == header.channels * header.bits_per_sample / 8);
    assert(header.byte_rate == header.sample_rate * header.block_align);

    return sound;
}

void sound::play() const {
    auto header = reinterpret_cast<const wav_header *>(&data[0]);
    const u32 sample_count = header->data_chunk_size / (header->channels * sizeof(u16));
    const u16 *samples = &header->samples;

    std::thread thread{[&] {
        u32 wav_playback_sample = 0;

        while (true) {
            u32 buffer_padding;
            assert(audio_client->GetCurrentPadding(&buffer_padding) == S_OK);

            u32 sound_buffer_latency = audio_buffer_size_in_frames / 50;
            u32 frames_to_write = sound_buffer_latency - buffer_padding;

            i16 *buffer = nullptr;
            assert(audio_render_client->GetBuffer(frames_to_write,
                                                  reinterpret_cast<BYTE **>(&buffer)) == S_OK);

            for (size_t i = 0; i < frames_to_write; ++i) {
                /*
                // Left.
                *buffer++ = samples[wav_playback_sample];
                // Right.
                *buffer++ = samples[wav_playback_sample];

                wav_playback_sample++;
                wav_playback_sample %= sample_count;*/

                u32 left_index = header->channels * wav_playback_sample;
                u32 right_index = left_index + header->channels - 1;

                u16 left_sample = samples[left_index];
                u16 right_sample = samples[right_index];

                ++wav_playback_sample;

                *buffer++ = left_sample;
                *buffer++ = right_sample;

                if (wav_playback_sample >= sample_count) {
                    wav_playback_sample -= sample_count;
                }
            }

            assert(audio_render_client->ReleaseBuffer(frames_to_write, 0) == S_OK);
        }
    }};
    thread.join();
}

void sound::stop() {

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
#include "vlk.system.hpp"

#define GDIPVER 0x0110
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <stdexcept>
#include <algorithm>

#include "vlk.util.hpp"

using namespace vlk;

#define HRESULT_CHECK(expr)                                                                                  \
    {                                                                                                        \
        HRESULT _hr = expr;                                                                                  \
        VLK_ASSERT(_hr == S_OK, "Operation failed.");                                                        \
    }

static std::vector<std::jthread> audio_threads;

constexpr auto vlk_window_class_name = L"PWA Window Class";

LRESULT CALLBACK win_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

void vlk::initialize() {
    WNDCLASS wc{0};
    wc.lpfnWndProc   = win_proc;
    wc.hInstance     = GetModuleHandle(0);
    wc.lpszClassName = vlk_window_class_name;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.cbWndExtra    = sizeof(window *);
    RegisterClass(&wc);

    // Initialize GDI+.

    // Kept here until we need it outside this function...
    ULONG_PTR gdiplus_token;

    Gdiplus::GdiplusStartupInput gdiplus_startup_input;
    Gdiplus::GdiplusStartup(&gdiplus_token, &gdiplus_startup_input, nullptr);

    // Initialize WASAPI (audio).

    HRESULT_CHECK(CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY));
}

void vlk::terminate() {}

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

image vlk::load_image(std::filesystem::path path, bool flip_vertically) {
    if (not path.is_absolute()) {
        path = std::filesystem::current_path() / path;
    }

    auto image = Gdiplus::Bitmap::FromFile(path.wstring().c_str());

    auto status = image->GetLastStatus();

    if (status == Gdiplus::FileNotFound) {
        throw std::runtime_error(std::format("Valkyrie: file not found {}.", path.string()));
    } else if (status != Gdiplus::Ok) {
        throw std::runtime_error(std::format("Valkyrie: failed to load image {}.", path.string()));
    }

    if (image->GetPixelFormat() != PixelFormat32bppARGB) {
        image->ConvertFormat(PixelFormat32bppARGB, Gdiplus::DitherTypeNone, Gdiplus::PaletteTypeOptimal,
                             nullptr, REAL_MAX);
    }

    vlk::image result{image->GetWidth(), image->GetHeight(), 4};

    for (size_t x = 0; x < image->GetWidth(); ++x) {
        for (size_t y = 0; y < image->GetHeight(); ++y) {
            Gdiplus::Color color;
            image->GetPixel(static_cast<INT>(x),
                            static_cast<INT>(flip_vertically ? image->GetHeight() - y - 1 : y), &color);

            *(result.at(x, y) + 0) = color.GetR();
            *(result.at(x, y) + 1) = color.GetG();
            *(result.at(x, y) + 2) = color.GetB();
            *(result.at(x, y) + 3) = color.GetA();
        }
    }

    return result;
}

#pragma pack(push, 1)
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

    // Extra bytes, I don't know why they are there.
    // Could probably parse how many bytes to put here but this works for now.
    u8 _[34];

    u32 data_id;
    u32 data_chunk_size;
    u16 samples;
};
#pragma pack(pop)

sound vlk::load_sound_wav_pcm_s16le(std::filesystem::path path) {
    const sound sound = load_binary_file(path);

    auto header = *reinterpret_cast<const wav_header *>(&sound[0]);

#define VLK_CHECK_WAV(name, value)                                                                           \
    if (header.name != value) {                                                                              \
        throw std::runtime_error(                                                                            \
            std::format("Valkyrie: failed to load {}. Expected {} to be {} but was {}.", path.string(),      \
                        #name, value, header.name));                                                         \
    }

    VLK_CHECK_WAV(riff_id, 1179011410);  // "RIFF"
    VLK_CHECK_WAV(wave_id, 1163280727);  // "WAVE"
    VLK_CHECK_WAV(fmt_id, 544501094);    // "fmt "
    VLK_CHECK_WAV(data_id, 1635017060);  // "data"
    VLK_CHECK_WAV(format_code, 1);       // 1 means PCM.
    VLK_CHECK_WAV(channels, 2);
    VLK_CHECK_WAV(fmt_chunk_size, 16);
    VLK_CHECK_WAV(sample_rate, 44100);
    VLK_CHECK_WAV(bits_per_sample, 16);
    VLK_CHECK_WAV(block_align, header.channels * header.bits_per_sample / 8);
    VLK_CHECK_WAV(byte_rate, header.sample_rate * header.block_align);

#undef VLK_CHECK_WAV

    return sound;
}

size_t vlk::play_sound(const sound &sound, bool loop) {
    size_t id = audio_threads.size();

    audio_threads.push_back(std::jthread{[=](std::stop_token stop_token) {
        IMMDeviceEnumerator *device_enum = nullptr;
        HRESULT_CHECK(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                       __uuidof(IMMDeviceEnumerator),
                                       reinterpret_cast<LPVOID *>(&device_enum)));

        IMMDevice *audio_device = nullptr;
        HRESULT_CHECK(device_enum->GetDefaultAudioEndpoint(eRender, eConsole, &audio_device));

        device_enum->Release();

        IAudioClient2 *audio_client = nullptr;
        HRESULT_CHECK(audio_device->Activate(__uuidof(IAudioClient2), CLSCTX_ALL, nullptr,
                                             reinterpret_cast<LPVOID *>(&audio_client)));

        audio_device->Release();

        WAVEFORMATEX format{};
        format.wFormatTag      = WAVE_FORMAT_PCM;
        format.nChannels       = 2;
        format.nSamplesPerSec  = 44100;
        format.wBitsPerSample  = 16;
        format.nBlockAlign     = (format.nChannels * format.wBitsPerSample) / 8;
        format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

        DWORD flags = AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
                      AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

        REFERENCE_TIME requested_sound_buffer_duration = 20000000;

        HRESULT_CHECK(audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, flags,
                                               requested_sound_buffer_duration, 0, &format, nullptr));

        IAudioRenderClient *audio_render_client = nullptr;
        HRESULT_CHECK(audio_client->GetService(__uuidof(IAudioRenderClient),
                                               reinterpret_cast<LPVOID *>(&audio_render_client)));

        u32 audio_buffer_size_in_frames;
        HRESULT_CHECK(audio_client->GetBufferSize(&audio_buffer_size_in_frames));

        HRESULT_CHECK(audio_client->Start());

        auto header            = reinterpret_cast<const wav_header *>(&sound[0]);
        const u32 sample_count = header->data_chunk_size / header->channels / (header->bits_per_sample / 8);
        const u16 *samples     = &header->samples;

        u32 wav_playback_sample = 0;

        while (!stop_token.stop_requested()) {
            if (wav_playback_sample >= sample_count) {
                if (!loop) {
                    break;
                }

                wav_playback_sample = 0;
            }

            u32 buffer_padding;
            HRESULT_CHECK(audio_client->GetCurrentPadding(&buffer_padding));

            u32 sound_buffer_latency = audio_buffer_size_in_frames / 50;
            u32 frames_to_write      = sound_buffer_latency - buffer_padding;

            i16 *buffer = nullptr;
            HRESULT_CHECK(
                audio_render_client->GetBuffer(frames_to_write, reinterpret_cast<BYTE **>(&buffer)));

            for (size_t i = 0; i < frames_to_write; ++i) {
                u32 left_index  = header->channels * wav_playback_sample;
                u32 right_index = left_index + header->channels - 1;

                u16 left_sample  = samples[left_index];
                u16 right_sample = samples[right_index];

                *buffer++ = left_sample;
                *buffer++ = right_sample;

                wav_playback_sample++;
            }

            HRESULT_CHECK(audio_render_client->ReleaseBuffer(frames_to_write, 0));
        }

        audio_client->Stop();
        audio_client->Release();
        audio_render_client->Release();
    }});

    return id;
}

void vlk::stop_sound(size_t id) { audio_threads.at(id).request_stop(); }

// RGBA bitmap.
static HBITMAP create_bitmap(HWND hwnd, size_t width, size_t height, u32 **pixels) {
    HDC hdc = GetDC(hwnd);

    BITMAPINFO info{};
    info.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth       = static_cast<i32>(width);
    info.bmiHeader.biHeight      = -static_cast<i32>(height);
    info.bmiHeader.biPlanes      = 1;
    info.bmiHeader.biBitCount    = 32;
    info.bmiHeader.biCompression = BI_RGB;

    HBITMAP bitmap =
        CreateDIBSection(hdc, &info, DIB_RGB_COLORS, reinterpret_cast<void **>(pixels), nullptr, 0);
    VLK_ASSERT(bitmap, "Failed to create bitmap.");

    ReleaseDC(nullptr, hdc);

    return bitmap;
}

window::window(const window_params &params)
    : m_should_close{false},
      m_width{params.width},
      m_height{params.height},
      m_transparent{params.transparent} {
    std::wstring wide_title;
    wide_title.assign(params.title.begin(), params.title.end());

    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;

    if (params.default_ui) {
        style |= WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
    }

    DWORD ex_style = WS_EX_APPWINDOW;  // WS_EX_TOPMOST

    if (m_transparent) {
        ex_style |= WS_EX_LAYERED;
    }

    HWND hwnd = CreateWindowEx(ex_style, vlk_window_class_name, wide_title.c_str(), style, CW_USEDEFAULT,
                               CW_USEDEFAULT, m_width, m_height, NULL, NULL, GetModuleHandle(0), NULL);

    if (!hwnd) {
        throw std::runtime_error("Valkyrie: failed to create win32 window.");
    }

    this->hwnd = hwnd;

    pixels = new u32[m_width * m_height];
    bitmap = create_bitmap(hwnd, m_width, m_height, &pixels);

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

static void blit(const window &window, HDC hdc) {
    HDC memory_hdc     = CreateCompatibleDC(hdc);
    HGDIOBJ old_bitmap = SelectObject(memory_hdc, window.bitmap);

    BitBlt(hdc, 0, 0, window.width(), window.height(), memory_hdc, 0, 0, SRCCOPY);

    SelectObject(hdc, old_bitmap);

    if (window.is_transparent()) {
        POINT zero{0, 0};
        SIZE size{window.width(), window.height()};

        BLENDFUNCTION blend{0};
        blend.BlendOp             = AC_SRC_OVER;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat         = AC_SRC_ALPHA;

        UpdateLayeredWindow(window.hwnd, hdc, &zero, &size, memory_hdc, &zero, 0, &blend, ULW_ALPHA);
    }

    DeleteDC(memory_hdc);
}

static void copy_pixels_rgba_to_argb(u32 *dst, const u32 *src, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        // AABBGGRR to AARRGGBB.
        dst[i] = (src[i] & 0xff00ff00) | ((src[i] & 0x00ff0000) >> 16) | ((src[i] & 0x000000ff) << 16);
    }
}

void window::swap_buffers(const color_buffer &color_buf) {
    VLK_ASSERT(color_buf.width() == m_width && color_buf.height() == m_height,
               "Color buffer size does not match window size.");

    copy_pixels_rgba_to_argb(pixels, reinterpret_cast<const u32 *>(&color_buf[0]),
                             static_cast<size_t>(m_width * m_height));

    if (!m_transparent) {
        // Trigger redraw.
        InvalidateRect(hwnd, nullptr, false);
    } else {
        HDC hdc = GetDC(hwnd);
        blit(*this, hdc);
        ReleaseDC(nullptr, hdc);
    }
}

void window::set_icon(const image &image) {
    VLK_ASSERT(image.channels == 4, "Image must be RGBA.");

    u32 *pixels          = new u32[image.width() * image.height()];
    HBITMAP color_bitmap = create_bitmap(hwnd, image.width(), image.height(), &pixels);

    copy_pixels_rgba_to_argb(pixels, reinterpret_cast<const u32 *>(&*image.at(0, 0)),
                             static_cast<size_t>(image.width() * image.height()));

    HBITMAP mask_bitmap =
        CreateBitmap(static_cast<i32>(image.width()), static_cast<i32>(image.height()), 1, 1, nullptr);
    VLK_ASSERT(mask_bitmap, "Failed to create bitmap");

    ICONINFO info{0};
    info.fIcon    = true;
    info.xHotspot = 0;
    info.yHotspot = 0;
    info.hbmMask  = mask_bitmap;
    info.hbmColor = color_bitmap;

    HICON icon = CreateIconIndirect(&info);
    VLK_ASSERT(icon, "Failed to create icon.");

    DeleteObject(mask_bitmap);
    DeleteObject(color_bitmap);
    // delete[] pixels;

    SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));

    // DestroyIcon(icon);
}

LRESULT CALLBACK win_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
    auto win = reinterpret_cast<window *>(GetWindowLongPtr(hwnd, 0));

    switch (u_msg) {
        case WM_CLOSE: {
            if (win) {
                win->close(true);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (win) {
                VLK_ASSERT_FAST(!win->is_transparent(),
                                "Drawing to transparent window is handled in swap_buffers().");

                blit(*win, hdc);
            } else {
                FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
    }

    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}
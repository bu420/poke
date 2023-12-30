#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define WINDOWS
#else
#error "Unsupported system."
#endif

#ifdef WINDOWS
#define NOMINMAX
#include <windows.h>
#endif
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <filesystem>
#include <thread>
#include <span>

#include "vlk.types.hpp"
#include "vlk.gfx.hpp"

#define VLK_BENCHMARK_START f64 _start = get_elapsed_time();
#define VLK_BENCHMARK_END   std::print("Benchmark: {}ms\n", get_elapsed_time() - _start);

namespace vlk {
    void initialize();
    void terminate();

    f64 get_elapsed_time();
    i64 get_ticks_per_sec();

    image load_image(std::filesystem::path path, bool flip_vertically = false);

    using sound = std::vector<u8>;

    // Load a 16-bit PCM wav file.
    sound load_sound_wav_pcm_s16le(std::filesystem::path path);

    size_t play_sound(const sound &sound, bool loop = false);
    void stop_sound(size_t id);

    struct window_params {
        std::string_view title;
        i32 width;
        i32 height;
        bool default_ui  = true;
        bool transparent = false;
    };

    class window {
    public:
#ifdef WINDOWS
        HWND hwnd;

        HBITMAP bitmap;
        u32 *pixels;
#endif

        window(const window_params &params);

        void poll_events();
        void swap_buffers(const color_buffer &color_buf);

        bool should_close() const { return m_should_close; }
        void close(bool should_close) { m_should_close = should_close; }

        i32 width() const { return m_width; }
        i32 height() const { return m_height; }

        bool is_transparent() const { return m_transparent; }

        void set_icon(const image &image);

    private:
        bool m_should_close;

        i32 m_width;
        i32 m_height;

        bool m_transparent;
    };

    model load_obj(std::filesystem::path path, bool flip_images_vertically = true);
}  // namespace vlk
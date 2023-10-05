#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#error "Unsupported system."
#endif
#include <string>
#include <vector>
#include <cstdint>

#include "vlk.types.hpp"
#include "vlk.gfx.hpp"

namespace vlk {
    void initialize();
    void terminate();

    f64 get_elapsed_time();
    i64 get_ticks_per_sec();

    class window {
    public:
        window(const std::string& title, i32 width, i32 height);

        void poll_events();
        void swap_buffers(const color_buffer& color_buf);

        bool get_should_close() const;
        void set_should_close(bool should_close);

        i32 get_width() const;
        i32 get_height() const;

        const std::vector<u32>& get_front_buf() const;

    private:
        HWND hwnd;
        bool should_close;

        i32 width;
        i32 height;

        std::vector<u32> front_buf;
    };
}
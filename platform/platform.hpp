#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#error "Unsupported system."
#endif

#include <string>
#include <vector>
#include <cstdint>

#include <valkyrie/gfx.hpp>

namespace vlk {
    void initialize();
    void terminate();

    double get_elapsed_time();
    int64_t get_ticks_per_sec();

    class window {
    public:
        window(const std::string& title, int width, int height);

        void poll_events();
        void swap_buffers(const color_buffer& color_buf);

        bool get_should_close() const;
        void set_should_close(bool should_close);

        int get_width() const;
        int get_height() const;

        uint32_t* get_ptr();

    private:
        HWND hwnd;
        bool should_close;

        int width;
        int height;

        std::vector<uint32_t> front_buf;
    };
}
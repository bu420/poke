#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <bit>

static_assert(std::endian::native == std::endian::little);
static_assert(sizeof(int) == 4);
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);

namespace vlk {
    using i8 = std::int8_t;
    using u8 = std::uint8_t;
    using i16 = std::int16_t;
    using u16 = std::uint16_t;
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using i64 = std::int64_t;
    using u64 = std::uint64_t;
    using f32 = float;
    using f64 = double;

    struct byte3 {
        std::byte x;
        std::byte y;
        std::byte z;
    };

    template<typename T> using optional_reference = 
        std::optional<std::reference_wrapper<T>>;

    std::vector<std::string> string_split(const std::string& source, 
                                          const std::string& separator);

    // For each token you can decide to stop processing further by returning false.
    void string_split(const std::string& source, 
                      const std::string& separator,
                      std::function<bool(const std::string& token)> callback);
}

#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <optional>
#include <bit>

#define VLK_ASSERT(expr, msg) assert(msg&& expr);

#ifdef VLK_DEBUG_FAST
#define VLK_ASSERT_FAST(expr, msg)                                                                           \
    do {                                                                                                     \
    } while (false);
#else
#define VLK_ASSERT_FAST(expr, msg) VLK_ASSERT(expr, msg);
#endif

static_assert(std::endian::native == std::endian::little);
static_assert(CHAR_BIT == 8);

namespace vlk {
    using std::size_t;

    using i8  = std::int8_t;
    using u8  = std::uint8_t;
    using i16 = std::int16_t;
    using u16 = std::uint16_t;
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using i64 = std::int64_t;
    using u64 = std::uint64_t;
    using f32 = float;
    using f64 = double;

    static_assert(sizeof(i32) == 4);
    static_assert(sizeof(f32) == 4);
    static_assert(sizeof(f64) == 8);

    template <typename T>
    using optional_ref = std::optional<std::reference_wrapper<T>>;

    std::byte operator""_byte(size_t value);
}  // namespace vlk
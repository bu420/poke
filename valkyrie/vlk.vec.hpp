#pragma once

#include <array>
#include <cmath>
#include <algorithm>

#include "vlk.types.hpp"

namespace vlk {
    class mat3;
    class mat4;

    template <typename T>
    class vec2 {
    public:
        vec2() = default;

        template <typename A, typename B>
        vec2(A a, B b) : m_arr{static_cast<T>(a), static_cast<T>(b)} {}

        template <typename U>
        vec2(const vec2<U>& u) : vec2{u[0], u[1]} {}

        T* begin() { return &m_arr[0]; }
        const T* begin() const { return &m_arr[0]; }
        T* end() { return &m_arr[1]; }
        const T* end() const { return &m_arr[1]; }

        vec2<T> normalize() const { return *this / length(); }
        T length() const { return std::sqrt(x() * x() + y() * y()); }

        T min() const { return std::min(x(), y()); }
        T max() const { return std::max(x(), y()); }
        vec2<T> min(const vec2<T>& v) const { return {std::min(x(), v.x()), std::min(y(), v.y())}; }
        vec2<T> max(const vec2<T>& v) const { return {std::max(x(), v.x()), std::max(y(), v.y())}; }

        vec2<T> lerp(const vec2<T>& v, f32 amount) const {
            return {std::lerp(x(), v.x(), amount), std::lerp(y(), v.y(), amount)};
        }

        vec2<T> clamp(T min, T max) const {
            return {std::clamp(x(), min, max), std::clamp(y(), min, max)};
        }

        T& operator[](int i) {
            VLK_ASSERT_FAST(i >= 0 && i < 2, "Index outside bounds.");
            return m_arr[i];
        }
        T operator[](int i) const {
            VLK_ASSERT_FAST(i >= 0 && i < 2, "Index outside bounds.");
            return m_arr[i];
        }

        /* bool operator==(const vec2<T>& v) const { return x() == v.x() && y() == v.y(); }

        bool operator<(const vec2<T>& v) const { return x() < v.x() && y() < v.y(); }
        bool operator>(const vec2<T>& v) const { return x() < v.x() && y() < v.y(); }*/

        auto operator<=>(const vec2<T>& v) const = default;

        // clang-format off
        vec2<T> operator-() const { return vec2<T>(-x(), -y()); }
        vec2<T> operator+(const vec2<T>& v) const { return vec2<T>(x() + v.x(), y() + v.y()); }
        vec2<T> operator+(T scalar) const { return vec2<T>(x() + scalar, y() + scalar); }
        vec2<T>& operator+=(const vec2<T>& v) { x() += v.x(); y() += v.y(); return *this; }
        vec2<T>& operator+=(T scalar) { x() += scalar; y() += scalar; return *this; }
        vec2<T> operator-(const vec2<T>& v) const { return vec2<T>(x() - v.x(), y() - v.y()); }
        vec2<T> operator-(T scalar) const { return vec2<T>(x() - scalar, y() - scalar); }
        vec2<T>& operator-=(const vec2<T>& v) { x() -= v.x(); y() -= v.y(); return *this; }
        vec2<T>& operator-=(T scalar) { x() -= scalar; y() -= scalar; return *this; }
        vec2<T> operator*(const vec2<T>& v) const { return vec2<T>(x() * v.x(), y() * v.y()); }
        vec2<T> operator*(T scalar) const { return vec2<T>(x() * scalar, y() * scalar); }
        vec2<T>& operator*=(const vec2<T>& v) { x() *= v.x(); y() *= v.y(); return *this; }
        vec2<T>& operator*=(T scalar) { x() *= scalar; y() *= scalar; return *this; }
        vec2<T> operator/(const vec2<T>& v) const { return vec2<T>(x() / v.x(), y() / v.y()); }
        vec2<T> operator/(T scalar) const { return vec2<T>(x() / scalar, y() / scalar); }
        vec2<T>& operator/=(const vec2<T>& v) { x() /= v.x(); y() /= v.y(); return *this; }
        vec2<T>& operator/=(T scalar) { x() /= scalar; y() /= scalar; return *this; }
        // clang-format on

        T& x() { return m_arr[0]; }
        const T& x() const { return m_arr[0]; }
        T& y() { return m_arr[1]; }
        const T& y() const { return m_arr[1]; }
        T& r() { return m_arr[0]; }
        const T& r() const { return m_arr[0]; }
        T& g() { return m_arr[1]; }
        const T& g() const { return m_arr[1]; }
        T& u() { return m_arr[0]; }
        const T& u() const { return m_arr[0]; }
        T& v() { return m_arr[1]; }
        const T& v() const { return m_arr[1]; }
        vec2<T> xy() const { return *this; }
        vec2<T> yx() const { return vec2<T>(y(), x()); }
        vec2<T> rg() const { return *this; }
        vec2<T> gr() const { return vec2<T>(g(), r()); }
        vec2<T> uv() const { return *this; }
        vec2<T> vu() const { return vec2<T>(v(), u()); }

    private:
        std::array<T, 2> m_arr;
    };

    using vec2f = vec2<f32>;
    using vec2i = vec2<i32>;

    template <typename T>
    class vec3 {
    public:
        vec3() = default;

        template <typename A, typename B, typename C>
        vec3(A a, B b, C c) : m_arr{static_cast<T>(a), static_cast<T>(b), static_cast<T>(c)} {}

        template <typename A, typename B>
        vec3(const vec2<A>& a, B b) : vec3{a[0], a[1], b} {}

        template <typename A, typename B>
        vec3(A a, const vec2<B>& b) : vec3{a, b[0], b[1]} {}

        template <typename U>
        vec3(const vec3<U>& u) : vec3{u[0], u[1], u[2]} {}

        T* begin() { return &m_arr[0]; }
        const T* begin() const { return &m_arr[0]; }
        T* end() { return &m_arr[2]; }
        const T* end() const { return &m_arr[2]; }

        vec3<T> normalize() const { return *this / length(); }
        T length() const { return std::sqrt(x() * x() + y() * y() + z() * z()); }

        T min() const { return std::min({x(), y(), z()}); }
        T max() const { return std::max({x(), y(), z()}); }
        vec3<T> min(const vec3<T>& v) const {
            return {std::min(x(), v.x()), std::min(y(), v.y()), std::min(z(), v.z())};
        }
        vec3<T> max(const vec3<T>& v) const {
            return {std::max(x(), v.x()), std::max(y(), v.y()), std::max(z(), v.z())};
        }

        vec3<T> lerp(const vec3<T>& v, f32 amount) const {
            return {std::lerp(x(), v.x(), amount), std::lerp(y(), v.y(), amount),
                    std::lerp(z(), v.z(), amount)};
        }

        vec3<T> clamp(T min, T max) const {
            return {std::clamp(x(), min, max), std::clamp(y(), min, max), std::clamp(z(), min, max)};
        }

        T& operator[](int i) {
            VLK_ASSERT_FAST(i >= 0 && i < 3, "Index outside bounds.");
            return m_arr[i];
        }
        T operator[](int i) const {
            VLK_ASSERT_FAST(i >= 0 && i < 3, "Index outside bounds.");
            return m_arr[i];
        }

        vec3<T> cross(const vec3<T>& v) const {
            vec3<T> result;
            result[0] = y() * v.z() - z() * v.y();
            result[1] = z() * v.x() - x() * v.z();
            result[2] = x() * v.y() - y() * v.x();
            return result;
        }

        T dot(const vec3<T>& v) const { return x() * v.x() + y() * v.y() + z() * v.z(); }

        // bool operator==(const vec3<T>& v) const { return x() == v.x() && y() == v.y() && z() == v.z(); }

        auto operator<=>(const vec3<T>& v) const = default;

        vec3<T> operator*(const mat3& m) const {
            vec3<T> result;
            for (int y = 0; y < 3; y++) {
                float i = 0;
                for (int x = 0; x < 3; x++) {
                    i += m[x][y] * (*this)[x];
                }
                result[y] = i;
            }
            return result;
        }

        vec3<T>& operator*=(const mat3& m) { return (*this = *this * m); }

        // clang-format off
        vec3<T> operator-() const { return vec3<T>(-x(), -y(), -z()); }
        vec3<T> operator+(const vec3<T>& v) const { return vec3<T>(x() + v.x(), y() + v.y(), z() + v.z()); }
        vec3<T> operator+(T scalar) const { return vec3<T>(x() + scalar, y() + scalar, z() + scalar); }
        vec3<T>& operator+=(const vec3<T>& v) { x() += v.x(); y() += v.y(); z() += v.z(); return *this; }
        vec3<T>& operator+=(T scalar) { x() += scalar; y() += scalar; z() += scalar; return *this; }
        vec3<T> operator-(const vec3<T>& v) const { return vec3<T>(x() - v.x(), y() - v.y(), z() - v.z()); }
        vec3<T> operator-(T scalar) const { return vec3<T>(x() - scalar, y() - scalar, z() - scalar); }
        vec3<T>& operator-=(const vec3<T>& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); return *this; }
        vec3<T>& operator-=(T scalar) { x() -= scalar; y() -= scalar; z() -= scalar; return *this; }
        vec3<T> operator*(const vec3<T>& v) const { return vec3<T>(x() * v.x(), y() * v.y(), z() * v.z()); }
        vec3<T> operator*(T scalar) const { return vec3<T>(x() * scalar, y() * scalar, z() * scalar); }
        vec3<T>& operator*=(const vec3<T>& v) { x() *= v.x(); y() *= v.y(); z() *= v.z(); return *this; }
        vec3<T>& operator*=(T scalar) { x() *= scalar; y() *= scalar; z() *= scalar; return *this; }
        vec3<T> operator/(const vec3<T>& v) const { return vec3<T>(x() / v.x(), y() / v.y(), z() / v.z()); }
        vec3<T> operator/(T scalar) const { return vec3<T>(x() / scalar, y() / scalar, z() / scalar); }
        vec3<T>& operator/=(const vec3<T>& v) { x() /= v.x(); y() /= v.y(); z() /= v.z(); return *this; }
        vec3<T>& operator/=(T scalar) { x() /= scalar; y() /= scalar; z() /= scalar; return *this; }
        // clang-format on

        T& x() { return m_arr[0]; }
        const T& x() const { return m_arr[0]; }
        T& y() { return m_arr[1]; }
        const T& y() const { return m_arr[1]; }
        T& r() { return m_arr[0]; }
        const T& r() const { return m_arr[0]; }
        T& g() { return m_arr[1]; }
        const T& g() const { return m_arr[1]; }
        T& u() { return m_arr[0]; }
        const T& u() const { return m_arr[0]; }
        T& v() { return m_arr[1]; }
        const T& v() const { return m_arr[1]; }
        T& z() { return m_arr[2]; }
        const T& z() const { return m_arr[2]; }
        T& b() { return m_arr[2]; }
        const T& b() const { return m_arr[2]; }
        T& w() { return m_arr[2]; }
        const T& w() const { return m_arr[2]; }
        vec2<T> xy() const { return vec2<T>(x(), y()); }
        vec2<T> yx() const { return vec2<T>(y(), x()); }
        vec2<T> rg() const { return vec2<T>(r(), g()); }
        vec2<T> gr() const { return vec2<T>(g(), r()); }
        vec2<T> uv() const { return vec2<T>(u(), v()); }
        vec2<T> vu() const { return vec2<T>(v(), u()); }
        vec3<T> xyz() const { return *this; }
        vec3<T> xzy() const { return vec3<T>(x(), z(), y()); }
        vec3<T> yxz() const { return vec3<T>(y(), x(), z()); }
        vec3<T> yzx() const { return vec3<T>(y(), z(), x()); }
        vec3<T> zxy() const { return vec3<T>(z(), x(), y()); }
        vec3<T> zyx() const { return vec3<T>(z(), y(), x()); }
        vec3<T> rgb() const { return *this; }
        vec3<T> rbg() const { return vec3<T>(r(), b(), g()); }
        vec3<T> grb() const { return vec3<T>(g(), r(), b()); }
        vec3<T> gbr() const { return vec3<T>(g(), b(), r()); }
        vec3<T> brg() const { return vec3<T>(b(), r(), g()); }
        vec3<T> bgr() const { return vec3<T>(b(), g(), r()); }
        vec3<T> uvw() const { return *this; }
        vec3<T> uwv() const { return vec3<T>(u(), w(), v()); }
        vec3<T> vuw() const { return vec3<T>(v(), u(), w()); }
        vec3<T> vwu() const { return vec3<T>(v(), w(), u()); }
        vec3<T> wuv() const { return vec3<T>(w(), u(), v()); }
        vec3<T> wvu() const { return vec3<T>(w(), v(), u()); }

    private:
        std::array<T, 3> m_arr;
    };

    using vec3f = vec3<f32>;
    using vec3i = vec3<i32>;

    template <typename T>
    class vec4 {
    public:
        vec4() = default;

        template <typename A, typename B, typename C, typename D>
        vec4(A a, B b, C c, D d)
            : m_arr{static_cast<T>(a), static_cast<T>(b), static_cast<T>(c), static_cast<T>(d)} {}

        template <typename A, typename B, typename C>
        vec4(const vec2<A>& a, B b, C c) : vec4{a[0], a[1], b, c} {}

        template <typename A, typename B, typename C>
        vec4(A a, const vec2<B>& b, C c) : vec4{a, b[0], b[1], c} {}

        template <typename A, typename B, typename C>
        vec4(A a, B b, const vec2<C>& c) : vec4{a, b, c[0], c[1]} {}

        template <typename A, typename B>
        vec4(const vec2<A>& ab, const vec2<B>& cd) : vec4{ab[0], ab[1], cd[0], cd[1]} {}

        template <typename A, typename B>
        vec4(const vec3<A>& a, B b) : vec4{a[0], a[1], a[2], b} {}

        template <typename A, typename B>
        vec4(A a, const vec3<B>& b) : vec4{a, b[0], b[1], b[2]} {}

        template <typename U>
        vec4(const vec4<U>& a) : vec4{a[0], a[1], a[2], a[3]} {}

        T* begin() { return &m_arr[0]; }
        const T* begin() const { return &m_arr[0]; }
        T* end() { return &m_arr[3]; }
        const T* end() const { return &m_arr[3]; }

        vec4<T> normalize() const { return *this / length(); }
        T length() const { return std::sqrt(x() * x() + y() * y() + z() * z() + w() * w()); }

        T min() const { return std::min({x(), y(), z(), w()}); }
        T max() const { return std::max({x(), y(), z(), w()}); }
        vec4<T> min(const vec4<T>& v) const {
            return {std::min(x(), v.x()), std::min(y(), v.y()), std::min(z(), v.z()), std::min(w(), v.w())};
        }
        vec4<T> max(const vec4<T>& v) const {
            return {std::max(x(), v.x()), std::max(y(), v.y()), std::max(z(), v.z()), std::max(w(), v.w())};
        }

        vec4<T> lerp(const vec4<T>& v, f32 amount) const {
            return {std::lerp(x(), v.x(), amount), std::lerp(y(), v.y(), amount),
                    std::lerp(z(), v.z(), amount), std::lerp(w(), v.w(), amount)};
        }

        vec4<T> clamp(T min, T max) const {
            return {std::clamp(x(), min, max), std::clamp(y(), min, max), std::clamp(z(), min, max),
                    std::clamp(w(), min, max)};
        }

        T& operator[](int i) {
            VLK_ASSERT_FAST(i >= 0 && i < 4, "Index outside bounds.");
            return m_arr[i];
        }
        T operator[](int i) const {
            VLK_ASSERT_FAST(i >= 0 && i < 4, "Index outside bounds.");
            return m_arr[i];
        }

        /* bool operator==(const vec4<T>& v) const {
            return x() == v.x() && y() == v.y() && z() == v.z() && w() == v.w();
        }*/

        auto operator<=>(const vec4<T>& v) const = default;

        vec4<T> operator*(const mat4& m) const {
            vec4<T> result;
            for (int y = 0; y < 4; y++) {
                float i = 0;
                for (int x = 0; x < 4; x++) {
                    i += m[x][y] * (*this)[x];
                }
                result[y] = i;
            }
            return result;
        }

        vec4<T>& operator*=(const mat4& m) { return (*this = *this * m); }

        // clang-format off
        vec4<T> operator-() const { return vec4<T>(-x(), -y(), -z(), -w()); }
        vec4<T> operator+(const vec4<T>& v) const { return vec4<T>(x() + v.x(), y() + v.y(), z() + v.z(), w() + v.w()); }
        vec4<T> operator+(T scalar) const { return vec4<T>(x() + scalar, y() + scalar, z() + scalar, w() + scalar); }
        vec4<T>& operator+=(const vec4<T>& v) { x() += v.x(); y() += v.y(); z() += v.z(); w() += v.w(); return *this; }
        vec4<T>& operator+=(T scalar) { x() += scalar; y() += scalar; z() += scalar; w() += scalar; return *this; }
        vec4<T> operator-(const vec4<T>& v) const { return vec4<T>(x() - v.x(), y() - v.y(), z() - v.z(), w() - v.w()); }
        vec4<T> operator-(T scalar) const { return vec4<T>(x() - scalar, y() - scalar, z() - scalar, w() - scalar); }
        vec4<T>& operator-=(const vec4<T>& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); w() -= v.w(); return *this; }
        vec4<T>& operator-=(T scalar) { x() -= scalar; y() -= scalar; z() -= scalar; w() -= scalar; return *this; }
        vec4<T> operator*(const vec4<T>& v) const { return vec4<T>(x() * v.x(), y() * v.y(), z() * v.z(), w() * v.w()); }
        vec4<T> operator*(T scalar) const { return vec4<T>(x() * scalar, y() * scalar, z() * scalar, w() * scalar); }
        vec4<T>& operator*=(const vec4<T>& v) { x() *= v.x(); y() *= v.y(); z() *= v.z(); w() *= v.w(); return *this; }
        vec4<T>& operator*=(T scalar) { x() *= scalar; y() *= scalar; z() *= scalar; w() *= scalar; return *this; }
        vec4<T> operator/(const vec4<T>& v) const { return vec4<T>(x() / v.x(), y() / v.y(), z() / v.z(), w() / v.w()); }
        vec4<T> operator/(T scalar) const { return vec4<T>(x() / scalar, y() / scalar, z() / scalar, w() / scalar); }
        vec4<T>& operator/=(const vec4<T>& v) { x() /= v.x(); y() /= v.y(); z() /= v.z(); w() /= v.w(); return *this; }
        vec4<T>& operator/=(T scalar) { x() /= scalar; y() /= scalar; z() /= scalar; w() /= scalar; return *this; }
        // clang-format on

        T& x() { return m_arr[0]; }
        const T& x() const { return m_arr[0]; }
        T& y() { return m_arr[1]; }
        const T& y() const { return m_arr[1]; }
        T& r() { return m_arr[0]; }
        const T& r() const { return m_arr[0]; }
        T& g() { return m_arr[1]; }
        const T& g() const { return m_arr[1]; }
        T& z() { return m_arr[2]; }
        const T& z() const { return m_arr[2]; }
        T& b() { return m_arr[2]; }
        const T& b() const { return m_arr[2]; }
        T& w() { return m_arr[3]; }
        const T& w() const { return m_arr[3]; }
        T& a() { return m_arr[3]; }
        const T& a() const { return m_arr[3]; }
        vec2<T> xy() const { return vec2<T>(x(), y()); }
        vec2<T> yx() const { return vec2<T>(y(), x()); }
        vec2<T> rg() const { return vec2<T>(r(), g()); }
        vec2<T> gr() const { return vec2<T>(g(), r()); }
        vec3<T> xyz() const { return vec3<T>(x(), y(), z()); }
        vec3<T> xzy() const { return vec3<T>(x(), z(), y()); }
        vec3<T> yxz() const { return vec3<T>(y(), x(), z()); }
        vec3<T> yzx() const { return vec3<T>(y(), z(), x()); }
        vec3<T> zxy() const { return vec3<T>(z(), x(), y()); }
        vec3<T> zyx() const { return vec3<T>(z(), y(), x()); }
        vec3<T> rgb() const { return vec3<T>(r(), g(), b()); }
        vec3<T> rbg() const { return vec3<T>(r(), b(), g()); }
        vec3<T> grb() const { return vec3<T>(g(), r(), b()); }
        vec3<T> gbr() const { return vec3<T>(g(), b(), r()); }
        vec3<T> brg() const { return vec3<T>(b(), r(), g()); }
        vec3<T> bgr() const { return vec3<T>(b(), g(), r()); }
        vec4<T> xyzw() const { return *this; }
        vec4<T> xywz() const { return vec4<T>(x(), y(), w(), z()); }
        vec4<T> xzyw() const { return vec4<T>(x(), z(), y(), w()); }
        vec4<T> xzwy() const { return vec4<T>(x(), z(), w(), y()); }
        vec4<T> xwyz() const { return vec4<T>(x(), w(), y(), z()); }
        vec4<T> xwzy() const { return vec4<T>(x(), w(), z(), y()); }
        vec4<T> yxzw() const { return vec4<T>(y(), x(), z(), w()); }
        vec4<T> yxwz() const { return vec4<T>(y(), x(), w(), z()); }
        vec4<T> yzxw() const { return vec4<T>(y(), z(), x(), w()); }
        vec4<T> yzwx() const { return vec4<T>(y(), z(), w(), x()); }
        vec4<T> ywxz() const { return vec4<T>(y(), w(), x(), z()); }
        vec4<T> ywzx() const { return vec4<T>(y(), w(), z(), x()); }
        vec4<T> zxyw() const { return vec4<T>(z(), x(), y(), w()); }
        vec4<T> zxwy() const { return vec4<T>(z(), x(), w(), y()); }
        vec4<T> zyxw() const { return vec4<T>(z(), y(), x(), w()); }
        vec4<T> zywx() const { return vec4<T>(z(), y(), w(), x()); }
        vec4<T> zwxy() const { return vec4<T>(z(), w(), x(), y()); }
        vec4<T> zwyx() const { return vec4<T>(z(), w(), y(), x()); }
        vec4<T> wxyz() const { return vec4<T>(w(), x(), y(), z()); }
        vec4<T> wxzy() const { return vec4<T>(w(), x(), z(), y()); }
        vec4<T> wyxz() const { return vec4<T>(w(), y(), x(), z()); }
        vec4<T> wyzx() const { return vec4<T>(w(), y(), z(), x()); }
        vec4<T> wzxy() const { return vec4<T>(w(), z(), x(), y()); }
        vec4<T> wzyx() const { return vec4<T>(w(), z(), y(), x()); }
        vec4<T> rgba() const { return *this; }
        vec4<T> rgab() const { return vec4<T>(r(), g(), a(), b()); }
        vec4<T> rbga() const { return vec4<T>(r(), b(), g(), a()); }
        vec4<T> rbag() const { return vec4<T>(r(), b(), a(), g()); }
        vec4<T> ragb() const { return vec4<T>(r(), a(), g(), b()); }
        vec4<T> rabg() const { return vec4<T>(r(), a(), b(), g()); }
        vec4<T> grba() const { return vec4<T>(g(), r(), b(), a()); }
        vec4<T> grab() const { return vec4<T>(g(), r(), a(), b()); }
        vec4<T> gbra() const { return vec4<T>(g(), b(), r(), a()); }
        vec4<T> gbar() const { return vec4<T>(g(), b(), a(), r()); }
        vec4<T> garb() const { return vec4<T>(g(), a(), r(), b()); }
        vec4<T> gabr() const { return vec4<T>(g(), a(), b(), r()); }
        vec4<T> brga() const { return vec4<T>(b(), r(), g(), a()); }
        vec4<T> brag() const { return vec4<T>(b(), r(), a(), g()); }
        vec4<T> bgra() const { return vec4<T>(b(), g(), r(), a()); }
        vec4<T> bgar() const { return vec4<T>(b(), g(), a(), r()); }
        vec4<T> barg() const { return vec4<T>(b(), a(), r(), g()); }
        vec4<T> bagr() const { return vec4<T>(b(), a(), g(), r()); }
        vec4<T> argb() const { return vec4<T>(a(), r(), g(), b()); }
        vec4<T> arbg() const { return vec4<T>(a(), r(), b(), g()); }
        vec4<T> agrb() const { return vec4<T>(a(), g(), r(), b()); }
        vec4<T> agbr() const { return vec4<T>(a(), g(), b(), r()); }
        vec4<T> abrg() const { return vec4<T>(a(), b(), r(), g()); }
        vec4<T> abgr() const { return vec4<T>(a(), b(), g(), r()); }

    private:
        std::array<T, 4> m_arr;
    };

    using vec4f = vec4<f32>;
    using vec4i = vec4<i32>;
}  // namespace vlk

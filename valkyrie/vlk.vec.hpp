#pragma once

#include <array>
#include <cmath>
#include <cassert>

#include "vlk.types.hpp"

namespace vlk {
    class mat3;
    class mat4;

    template <typename T>
    class vec2 {
    private:
        std::array<T, 2> arr;

    public:
        vec2() = default;
        vec2(T a, T b) : arr{a, b} {}

        T* begin() { return &arr[0]; }
        const T* begin() const { return &arr[0]; }
        T* end() { return &arr[1]; }
        const T* end() const { return &arr[1]; }

        vec2<T> normalize() const {
            return *this / length();
        }
        T length() const {
            return std::sqrt(x() * x() + y() * y());
        }
        T& operator [] (int i) {
            assert(i >= 0 && i < 2 && "Index outside bounds.");
            return arr[i];
        }
        T operator [] (int i) const {
            assert(i >= 0 && i < 2 && "Index outside bounds.");
            return arr[i];
        }

        bool operator == (const vec2<T>& v) const {
            return x() == v.x() && y() == v.y();
        }

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

        T& x() { return arr[0]; }
        const T& x() const { return arr[0]; }
        T& y() { return arr[1]; }
        const T& y() const { return arr[1]; }
        T& r() { return arr[0]; }
        const T& r() const { return arr[0]; }
        T& g() { return arr[1]; }
        const T& g() const { return arr[1]; }
        T& u() { return arr[0]; }
        const T& u() const { return arr[0]; }
        T& v() { return arr[1]; }
        const T& v() const { return arr[1]; }
        vec2<T> xy() const { return *this; }
        vec2<T> yx() const { return vec2<T>(y(), x()); }
        vec2<T> rg() const { return *this; }
        vec2<T> gr() const { return vec2<T>(g(), r()); }
        vec2<T> uv() const { return *this; }
        vec2<T> vu() const { return vec2<T>(v(), u()); }
    };

    using vec2f = vec2<f32>;
    using vec2i = vec2<i32>;

    template <typename T>
    class vec3 {
    private:
        std::array<T, 3> arr;

    public:
        vec3() = default;
        vec3(T a, T b, T c) : arr{a, b, c} {}
        vec3(const vec2<T>& ab, T c) : arr{ab[0], ab[1], c} {}
        vec3(T a, const vec2<T>& bc) : arr{a, bc[0], bc[1]} {}

        T* begin() { return &arr[0]; }
        const T* begin() const { return &arr[0]; }
        T* end() { return &arr[2]; }
        const T* end() const { return &arr[2]; }

        vec3<T> normalize() const {
            return *this / length();
        }
        T length() const {
            return std::sqrt(x() * x() + y() * y() + z() * z());
        }
        T& operator [] (int i) {
            assert(i >= 0 && i < 3 && "Index outside bounds.");
            return arr[i];
        }
        T operator [] (int i) const {
            assert(i >= 0 && i < 3 && "Index outside bounds.");
            return arr[i];
        }

        vec3<T> cross(const vec3<T>& v) const {
            vec3<T> result;
            result[0] = y() * v.z() - z() * v.y();
            result[1] = z() * v.x() - x() * v.z();
            result[2] = x() * v.y() - y() * v.x();
            return result;
        }

        T dot(const vec3<T>& v) const {
            return x() * v.x() + y() * v.y() + z() * v.z();
        }

        bool operator == (const vec3<T>& v) const {
            return x() == v.x() && y() == v.y() && z() == v.z();
        }

        vec3<T> operator * (const mat3& m) const {
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

        vec3<T>& operator *= (const mat3& m) {
            return (*this = *this * m);
        }

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

        T& x() { return arr[0]; }
        const T& x() const { return arr[0]; }
        T& y() { return arr[1]; }
        const T& y() const { return arr[1]; }
        T& r() { return arr[0]; }
        const T& r() const { return arr[0]; }
        T& g() { return arr[1]; }
        const T& g() const { return arr[1]; }
        T& u() { return arr[0]; }
        const T& u() const { return arr[0]; }
        T& v() { return arr[1]; }
        const T& v() const { return arr[1]; }
        T& z() { return arr[2]; }
        const T& z() const { return arr[2]; }
        T& b() { return arr[2]; }
        const T& b() const { return arr[2]; }
        T& w() { return arr[2]; }
        const T& w() const { return arr[2]; }
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
    };

    using vec3f = vec3<f32>;
    using vec3i = vec3<i32>;

    template <typename T>
    class vec4 {
    private:
        std::array<T, 4> arr;

    public:
        vec4() = default;
        vec4(T a, T b, T c, T d) : arr{a, b, c, d} {}
        vec4(const vec2<T>& ab, T c, T d) : arr{ab[0], ab[1], c, d} {}
        vec4(T a, const vec2<T>& bc, T d) : arr{a, bc[0], bc[1], d} {}
        vec4(T a, T b, const vec2<T>& cd) : arr{a, b, cd[0], cd[1]} {}
        vec4(const vec2<T>& ab, const vec2<T>& cd) : arr{ab[0], ab[1], cd[0], cd[1]} {}
        vec4(const vec3<T>& abc, T d) : arr{abc[0], abc[1], abc[2], d} {}
        vec4(T a, const vec3<T>& bcd) : arr{a, bcd[0], bcd[1], bcd[2]} {}

        vec4(const vec2<T>& ab, T c) : arr{ab[0], ab[1], c} {}
        vec4(T a, const vec2<T>& bc) : arr{a, bc[0], bc[1]} {}

        T* begin() { return &arr[0]; }
        const T* begin() const { return &arr[0]; }
        T* end() { return &arr[3]; }
        const T* end() const { return &arr[3]; }

        vec4<T> normalize() const {
            return *this / length();
        }
        T length() const {
            return std::sqrt(x() * x() + y() * y() + z() * z() + w() * w());
        }
        T& operator [] (int i) {
            assert(i >= 0 && i < 4 && "Index outside bounds.");
            return arr[i];
        }
        T operator [] (int i) const {
            assert(i >= 0 && i < 4 && "Index outside bounds.");
            return arr[i];
        }

        bool operator == (const vec4<T>& v) const {
            return x() == v.x() && y() == v.y() && z() == v.z() && w() == v.w();
        }

        vec4<T> operator * (const mat4& m) const {
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

        vec4<T>& operator *= (const mat4& m) {
            return (*this = *this * m);
        }

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

        T& x() { return arr[0]; }
        const T& x() const { return arr[0]; }
        T& y() { return arr[1]; }
        const T& y() const { return arr[1]; }
        T& r() { return arr[0]; }
        const T& r() const { return arr[0]; }
        T& g() { return arr[1]; }
        const T& g() const { return arr[1]; }
        T& z() { return arr[2]; }
        const T& z() const { return arr[2]; }
        T& b() { return arr[2]; }
        const T& b() const { return arr[2]; }
        T& w() { return arr[3]; }
        const T& w() const { return arr[3]; }
        T& a() { return arr[3]; }
        const T& a() const { return arr[3]; }
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
    };

    using vec4f = vec4<f32>;
    using vec4i = vec4<i32>;
}

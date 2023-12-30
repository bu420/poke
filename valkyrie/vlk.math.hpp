#pragma once

#include <array>

#include "vlk.vec.hpp"

namespace vlk {
    template <typename T>
    struct rect {
        T x;
        T y;
        T w;
        T h;
    };

    class mat4;

    class mat3 {
    public:
        mat3(f32 init = 0);
        mat3(const mat4 &m);

        mat3 transpose() const;

        vec3f &operator[](u32 x);
        const vec3f &operator[](u32 x) const;

        mat3 operator*(const mat3 &m) const;
        vec3f operator*(const vec3f &v) const;

    private:
        std::array<vec3f, 3> m_elems;
    };

    class mat4 {
    public:
        mat4(f32 init = 0);

        mat4 translate(const vec3f &delta_pos) const;
        mat4 rotate_x(f32 angle) const;
        mat4 rotate_y(f32 angle) const;
        mat4 rotate_z(f32 angle) const;
        mat4 scale(const vec3f &multiplier) const;

        mat4 transpose() const;
        mat4 inverse() const;

        vec4f &operator[](u32 x) { return m_elems[x]; }
        const vec4f &operator[](u32 x) const { return m_elems[x]; }

        mat4 operator*(const mat4 &m) const;
        mat4 &operator*=(const mat4 &m);
        vec4f operator*(const vec4f &v) const;

    private:
        std::array<vec4f, 4> m_elems;
    };

    mat4 look_at(vec3f pos, vec3f target, vec3f up);
    mat4 perspective(f32 aspect, f32 fov, f32 near, f32 far);

    class quaternion {
    public:
        vec4f vec;

        quaternion() = default;
        quaternion(const vec4f &v) : vec{v} {}
        quaternion(const vec3f &axis, f32 angle) { set(axis, angle); }

        void set(const vec3f &axis, f32 angle);
        std::pair<vec3f, f32> to_axis_and_angle() const;
        void integrate(const vec3f &dv, f32 dt);
        quaternion normalize() const;
        mat3 to_mat3() const;

        quaternion operator*(const quaternion &q) const;
        quaternion &operator*=(const quaternion &q);
    };
}  // namespace vlk
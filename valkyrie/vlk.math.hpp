#pragma once

#include "vlk.vec.hpp"

namespace vlk {
    class mat4;

    class mat3 {
    public:
        mat3(f32 init = 0);
        mat3(const mat4& m);

        vec3f& operator [] (u8 x);
        const vec3f& operator [] (u8 x) const;

    private:
        vec3f m[3];
    };

    class mat4 {
    public:
        mat4(f32 init = 0);

        mat4 translate(const vec3f& delta_pos) const;
        mat4 rotate_x(f32 angle) const;
        mat4 rotate_y(f32 angle) const;
        mat4 rotate_z(f32 angle) const;
        mat4 scale(const vec3f& multiplier) const;

        mat4 transpose() const;
        mat4 inverse() const;

        vec4f& operator [] (u8 x);
        const vec4f& operator [] (u8 x) const;

        mat4 operator * (const mat4& m) const;
        mat4& operator *= (const mat4& m);
        vec4f operator * (const vec4f& v) const;

    private:
        vec4f m[4];
    };

    mat4 look_at(vec3f pos, vec3f target, vec3f up);
    mat4 perspective(f32 aspect, f32 fov, f32 near, f32 far);
}
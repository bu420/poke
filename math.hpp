#pragma once

#include "vec.hpp"

namespace poke {
    class mat4 {
    public:
        mat4(float init = 0);

        mat4 translate(const vec3f& delta_pos) const;
        mat4 rotate_x(float angle) const;
        mat4 rotate_y(float angle) const;
        mat4 rotate_z(float angle) const;
        mat4 scale(const vec3f& multiplier) const;

        mat4 transpose() const;
        mat4 inverse() const;

        vec4f& operator [] (int x);
        const vec4f& operator [] (int x) const;

        mat4 operator * (const mat4& m) const;
        mat4& operator *= (const mat4& m);
        vec4f operator * (const vec4f& v) const;

    private:
        vec4f m[4];
    };

    mat4 look_at(vec3f pos, vec3f target, vec3f up);
    mat4 perspective(float aspect, float fov, float near, float far);
}
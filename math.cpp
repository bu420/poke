#include "math.hpp"

#include <cmath>

using namespace poke;

mat4::mat4(float init) {
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            (*this)[col][row] = 0;
        }
    }

    for (int i = 0; i < 4; i++) {
        (*this)[i][i] = init;
    }
}

mat4 mat4::translate(const vec3f& delta_pos) const {
    mat4 m(1);
    m[3][0] = delta_pos.x();
    m[3][1] = delta_pos.y();
    m[3][2] = delta_pos.z();
    return *this * m;
}

mat4 mat4::rotate_x(float angle) const {
    float s = std::sin(angle);
    float c = std::cos(angle);

    mat4 x(1);
    x[1][1] = c;
    x[1][2] = -s;
    x[2][1] = s;
    x[2][2] = c;

    return *this * x;
}

mat4 mat4::rotate_y(float angle) const {
    float s = std::sin(angle);
    float c = std::cos(angle);

    mat4 y(1);
    y[0][0] = c;
    y[0][2] = s;
    y[2][0] = -s;
    y[2][2] = c;

    return *this * y;
}

mat4 mat4::rotate_z(float angle) const {
    float s = std::sin(angle);
    float c = std::cos(angle);

    mat4 z(1);

    z[0][0] = c;
    z[0][1] = -s;
    z[1][0] = s;
    z[1][1] = c;

    return *this * z;
}

mat4 mat4::scale(const vec3f& multiplier) const {
    mat4 m(1);
    m[0][0] = multiplier.x();
    m[1][1] = multiplier.y();
    m[2][2] = multiplier.z();

    return *this * m;
}

mat4 mat4::transpose() const {
    mat4 m;

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            m[col][row] = (*this)[row][col];
        }
    }

    return m;
}

mat4 mat4::inverse() const {
    float a2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    float a1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    float a1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    float a0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    float a0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    float a0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    float a2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    float a1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    float a1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    float a2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    float a1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    float a1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    float a0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
    float a0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    float a0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
    float a0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
    float a0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
    float a0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

    float det =
        m[0][0] * (m[1][1] * a2323 - m[1][2] * a1323 + m[1][3] * a1223) -
        m[0][1] * (m[1][0] * a2323 - m[1][2] * a0323 + m[1][3] * a0223) +
        m[0][2] * (m[1][0] * a1323 - m[1][1] * a0323 + m[1][3] * a0123) -
        m[0][3] * (m[1][0] * a1223 - m[1][1] * a0223 + m[1][2] * a0123);

    assert(det > 0 && "Matrix is not invertible.");
    det = 1 / det;

    mat4 result;
    result[0][0] = det * (m[1][1] * a2323 - m[1][2] * a1323 + m[1][3] * a1223);
    result[0][1] = det * -(m[0][1] * a2323 - m[0][2] * a1323 + m[0][3] * a1223);
    result[0][2] = det * (m[0][1] * a2313 - m[0][2] * a1313 + m[0][3] * a1213);
    result[0][3] = det * -(m[0][1] * a2312 - m[0][2] * a1312 + m[0][3] * a1212);
    result[1][0] = det * -(m[1][0] * a2323 - m[1][2] * a0323 + m[1][3] * a0223);
    result[1][1] = det * (m[0][0] * a2323 - m[0][2] * a0323 + m[0][3] * a0223);
    result[1][2] = det * -(m[0][0] * a2313 - m[0][2] * a0313 + m[0][3] * a0213);
    result[1][3] = det * (m[0][0] * a2312 - m[0][2] * a0312 + m[0][3] * a0212);
    result[2][0] = det * (m[1][0] * a1323 - m[1][1] * a0323 + m[1][3] * a0123);
    result[2][1] = det * -(m[0][0] * a1323 - m[0][1] * a0323 + m[0][3] * a0123);
    result[2][2] = det * (m[0][0] * a1313 - m[0][1] * a0313 + m[0][3] * a0113);
    result[2][3] = det * -(m[0][0] * a1312 - m[0][1] * a0312 + m[0][3] * a0112);
    result[3][0] = det * -(m[1][0] * a1223 - m[1][1] * a0223 + m[1][2] * a0123);
    result[3][1] = det * (m[0][0] * a1223 - m[0][1] * a0223 + m[0][2] * a0123);
    result[3][2] = det * -(m[0][0] * a1213 - m[0][1] * a0213 + m[0][2] * a0113);
    result[3][3] = det * (m[0][0] * a1212 - m[0][1] * a0212 + m[0][2] * a0112);

    return result;
}

vec4f& mat4::operator [] (int x) {
    return m[x];
}

const vec4f& mat4::operator [] (int x) const {
    return m[x];
}

mat4 mat4::operator * (const mat4& m) const {
    mat4 result;

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            for (int i = 0; i < 4; i++) {
                result[x][y] += (*this)[x][i] * m[i][y];
            }
        }
    }

    return result;
}

mat4& mat4::operator *= (const mat4& m) {
    *this = *this * m;
    return *this;
}

vec4f mat4::operator * (const vec4f& v) const {
    vec4f result;

    for (int x = 0; x < 4; x++) {
        float i = 0;
        for (int y = 0; y < 4; y++) {
            i += (*this)[x][y] * v[y];
        }
        result[x] = i;
    }

    return result;
}

mat4 poke::look_at(vec3f pos, vec3f target, vec3f up) {
    vec3f diff = (target - pos);
    vec3f forward = diff.normalize();
    vec3f right = forward.cross(up).normalize();
    vec3f local_up = right.cross(forward).normalize();

    mat4 m(1);
    m[0][0] = right.x();
    m[1][0] = right.y();
    m[2][0] = right.z();
    m[0][1] = local_up.x();
    m[1][1] = local_up.y();
    m[2][1] = local_up.z();
    m[0][2] = -forward.x();
    m[1][2] = -forward.y();
    m[2][2] = -forward.z();
    m[3][0] = -right.dot(pos);
    m[3][1] = -local_up.dot(pos);
    m[3][2] = forward.dot(pos);
    return m;
}

mat4 poke::perspective(float aspect, float fov, float near, float far) {
    mat4 m;

    float half_tan = std::tan(fov / 2);

    m[0][0] = 1 / (half_tan * aspect);
    m[1][1] = 1 / half_tan;
    m[2][2] = -(far + near) / (far - near);
    m[2][3] = -1;
    m[3][2] = -(2 * far * near) / (far - near);
    return m;
}
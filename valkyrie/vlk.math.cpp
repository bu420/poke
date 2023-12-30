#include "vlk.math.hpp"

#include <cmath>

using namespace vlk;

mat3::mat3(f32 init) {
    for (u32 col = 0; col < 3; ++col) {
        for (u32 row = 0; row < 3; ++row) {
            m_elems[col][row] = 0;
        }
    }
    for (u32 i = 0; i < 3; ++i) {
        m_elems[i][i] = init;
    }
}

mat3::mat3(const mat4 &m) {
    for (u32 col = 0; col < 3; ++col) {
        for (u32 row = 0; row < 3; ++row) {
            m_elems[col][row] = m[col][row];
        }
    }
}

mat3 mat3::transpose() const {
    mat3 m;

    for (u32 col = 0; col < 3; ++col) {
        for (u32 row = 0; row < 3; ++row) {
            m[col][row] = m_elems[row][col];
        }
    }

    return m;
}

vec3f &mat3::operator[](u32 x) { return m_elems[x]; }

const vec3f &mat3::operator[](u32 x) const { return m_elems[x]; }

mat3 mat3::operator*(const mat3 &m) const {
    mat3 result;

    for (u32 x = 0; x < 3; ++x) {
        for (u32 y = 0; y < 3; ++y) {
            for (u32 i = 0; i < 3; ++i) {
                result[x][y] += m_elems[x][i] * m[i][y];
            }
        }
    }

    return result;
}

vec3f mat3::operator*(const vec3f &v) const {
    vec3f result{};

    for (u32 x = 0; x < 3; ++x) {
        f32 i = 0;

        for (u32 y = 0; y < 3; ++y) {
            i += m_elems[x][y] * v[y];
        }

        result[x] = i;
    }

    return result;
}

mat4::mat4(f32 init) {
    for (u32 col = 0; col < 4; ++col) {
        for (u32 row = 0; row < 4; ++row) {
            m_elems[col][row] = 0;
        }
    }
    for (u32 i = 0; i < 4; ++i) {
        m_elems[i][i] = init;
    }
}

mat4 mat4::translate(const vec3f &delta_pos) const {
    mat4 m(1);
    m[3][0] = delta_pos.x();
    m[3][1] = delta_pos.y();
    m[3][2] = delta_pos.z();
    return *this * m;
}

mat4 mat4::rotate_x(f32 angle) const {
    f32 s = std::sin(angle);
    f32 c = std::cos(angle);

    mat4 x(1);
    x[1][1] = c;
    x[1][2] = -s;
    x[2][1] = s;
    x[2][2] = c;

    return *this * x;
}

mat4 mat4::rotate_y(f32 angle) const {
    f32 s = std::sin(angle);
    f32 c = std::cos(angle);

    mat4 y(1);
    y[0][0] = c;
    y[0][2] = s;
    y[2][0] = -s;
    y[2][2] = c;

    return *this * y;
}

mat4 mat4::rotate_z(f32 angle) const {
    f32 s = std::sin(angle);
    f32 c = std::cos(angle);

    mat4 z(1);

    z[0][0] = c;
    z[0][1] = -s;
    z[1][0] = s;
    z[1][1] = c;

    return *this * z;
}

mat4 mat4::scale(const vec3f &multiplier) const {
    mat4 m(1);
    m[0][0] = multiplier.x();
    m[1][1] = multiplier.y();
    m[2][2] = multiplier.z();

    return *this * m;
}

mat4 mat4::transpose() const {
    mat4 m;

    for (u32 col = 0; col < 4; ++col) {
        for (u32 row = 0; row < 4; ++row) {
            m[col][row] = m_elems[row][col];
        }
    }

    return m;
}

mat4 mat4::inverse() const {
    const auto &m = m_elems;

    f32 a2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    f32 a1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    f32 a1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    f32 a0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    f32 a0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    f32 a0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    f32 a2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    f32 a1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    f32 a1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    f32 a2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    f32 a1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    f32 a1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    f32 a0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
    f32 a0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    f32 a0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
    f32 a0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
    f32 a0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
    f32 a0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

    f32 det = m[0][0] * (m[1][1] * a2323 - m[1][2] * a1323 + m[1][3] * a1223) -
              m[0][1] * (m[1][0] * a2323 - m[1][2] * a0323 + m[1][3] * a0223) +
              m[0][2] * (m[1][0] * a1323 - m[1][1] * a0323 + m[1][3] * a0123) -
              m[0][3] * (m[1][0] * a1223 - m[1][1] * a0223 + m[1][2] * a0123);

    VLK_ASSERT(det > 0, "Matrix is not invertible.");
    det = 1.0f / det;

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

mat4 mat4::operator*(const mat4 &m) const {
    mat4 result;

    for (u32 x = 0; x < 4; x++) {
        for (u32 y = 0; y < 4; y++) {
            for (u32 i = 0; i < 4; i++) {
                result[x][y] += m_elems[x][i] * m[i][y];
            }
        }
    }

    return result;
}

mat4 &mat4::operator*=(const mat4 &m) {
    *this = *this * m;
    return *this;
}

vec4f mat4::operator*(const vec4f &v) const {
    vec4f result{};

    for (u32 x = 0; x < 4; x++) {
        f32 i = 0;

        for (u32 y = 0; y < 4; y++) {
            i += m_elems[x][y] * v[y];
        }

        result[x] = i;
    }

    return result;
}

mat4 vlk::look_at(vec3f pos, vec3f target, vec3f up) {
    vec3f diff     = target - pos;
    vec3f forward  = diff.normalize();
    vec3f right    = forward.cross(up).normalize();
    vec3f local_up = right.cross(forward).normalize();

    mat4 m{1.0f};
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

mat4 vlk::perspective(f32 aspect, f32 fov, f32 near, f32 far) {
    f32 half_tan = std::tan(fov / 2);

    mat4 m;
    m[0][0] = 1 / (half_tan * aspect);
    m[1][1] = 1 / half_tan;
    m[2][2] = -(far + near) / (far - near);
    m[2][3] = -1;
    m[3][2] = -(2 * far * near) / (far - near);
    return m;
}

void quaternion::set(const vec3f &axis, f32 angle) {
    f32 half_angle = 0.5f * angle;
    f32 s          = std::sin(half_angle);

    vec.x() = s * axis.x();
    vec.y() = s * axis.y();
    vec.z() = s * axis.z();
    vec.w() = std::cos(half_angle);
}

std::pair<vec3f, f32> quaternion::to_axis_and_angle() const {
    vec3f axis;
    f32 angle = 2.0f * std::acos(vec.w());

    f32 l = std::sqrt(1.0f - std::pow(vec.w(), 2.0f));

    if (l == 0.0f) {
        axis = {0.0f, 0.0f, 0.0f};
    } else {
        l    = 1.0f / l;
        axis = vec.xyz() * l;
    }

    return std::make_pair(axis, angle);
}

void quaternion::integrate(const vec3f &dv, f32 dt) {
    quaternion q{
        {dv.x() * dt, dv.y() * dt, dv.z() * dt, 0.0f}
    };

    q *= *this;

    vec.x() += q.vec.x() * 0.5f;
    vec.y() += q.vec.y() * 0.5f;
    vec.z() += q.vec.z() * 0.5f;
    vec.w() += q.vec.w() * 0.5f;

    *this = normalize();
}

quaternion quaternion::normalize() const {
    vec4f v = vec;

    f32 d = std::pow(v.x(), 2.0f) + std::pow(v.y(), 2.0f) + std::pow(v.z(), 2.0f) + std::pow(v.w(), 2.0f);

    if (d == 0.0f) {
        v.w() = 1.0f;
    }

    d = 1.0f / std::sqrt(d);

    if (d > 1.0e-8f) {
        v *= d;
    }

    return quaternion{v};
}

mat3 quaternion::to_mat3() const {
    f32 x  = 2.0f * vec.x();
    f32 y  = 2.0f * vec.y();
    f32 z  = 2.0f * vec.z();
    f32 xx = vec.x() * x;
    f32 xy = vec.x() * y;
    f32 xz = vec.x() * z;
    f32 xw = vec.w() * x;
    f32 yy = vec.y() * y;
    f32 yz = vec.y() * z;
    f32 yw = vec.w() * y;
    f32 zz = vec.z() * z;
    f32 zw = vec.w() * z;

    mat3 m;
    m[0][0] = 1.0f - yy - zz;
    m[0][1] = xy + zw;
    m[0][2] = xz - yw;
    m[1][0] = xy - zw;
    m[1][1] = 1.0f - xx - zz;
    m[1][2] = yz + xw;
    m[2][0] = xz + yw;
    m[2][1] = yz - xw;
    m[2][2] = 1.0f - xx - yy;

    return m;
}

quaternion quaternion::operator*(const quaternion &q) const {
    return quaternion{
        {vec.w() * q.vec.x() + vec.x() * q.vec.w() + vec.y() * q.vec.z() - vec.z() * q.vec.y(),
         vec.w() * q.vec.y() + vec.y() * q.vec.w() + vec.z() * q.vec.x() - vec.x() * q.vec.z(),
         vec.w() * q.vec.z() + vec.z() * q.vec.w() + vec.x() * q.vec.y() - vec.y() * q.vec.x(),
         vec.w() * q.vec.w() - vec.x() * q.vec.x() - vec.y() * q.vec.y() - vec.z() * q.vec.z()}
    };
}

quaternion &quaternion::operator*=(const quaternion &q) {
    *this = *this * q;
    return *this;
}
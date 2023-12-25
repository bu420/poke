#include "vlk.physics.hpp"

using namespace vlk;

void transform::set_identity() {
    pos = {0.0f, 0.0f, 0.0f};
    rot = mat3{1.0f};
}

transform transform::mul(const transform &t) const {
    transform result;
    result.rot = rot * t.rot;
    result.pos = rot * t.pos + pos;
    return result;
}

vec3f transform::mul(const vec3f &v) const {
    return rot * v + pos;
}

plane transform::mul(const plane &plane) const {
    vec3f origin = mul(plane.normal * plane.dist);
    vec3f normal = rot * plane.normal;

    return vlk::plane{.normal = normal,
                      .dist = origin.dot(normal)};
}

vec3f transform::mul(const vec3f &scale, const vec3f &v) const {
    return rot * (scale * v) + pos;
}

plane transform::mul(const vec3f &scale, const plane &plane) const {
    vec3f origin = mul(scale, plane.normal * plane.dist);
    vec3f normal = rot * plane.normal;

    return vlk::plane{.normal = normal,
                      .dist = origin.dot(normal)};
}

vec3f transform::mul_transpose(const vec3f &v) const {
    return rot.transpose() * (v - pos);
}

transform transform::mul_transpose(const transform &t) const {
    transform result;
    result.rot = rot.transpose() * t.rot;
    result.pos = rot.transpose() * (pos - t.pos);
    return result;
}

plane transform::mul_transpose(const plane &plane) const {
    vec3f origin = mul_transpose(plane.normal * plane.dist);
    vec3f normal = rot.transpose() * plane.normal;

    return vlk::plane{.normal = normal,
                      .dist = origin.dot(normal)};
}

transform transform::inverse() const {
    transform t;
    t.rot = rot.transpose();
    t.pos = t.rot * -pos;
    return t;
}

plane plane::normalize() const {
    const f32 length = normal.length();

    return plane{.normal = normal / length,
                 .dist = dist / length};
}

struct test_intersect_visitor {
    intersect_data operator () (const bounding_sphere &a, const bounding_sphere &b) {
        const f32 radius_dist = a.radius + b.radius;
        const f32 center_dist = (b.center - a.center).length();

        return intersect_data{.intersects = center_dist < radius_dist,
                              .dist = center_dist - radius_dist};
    }

    intersect_data operator () (const aabb &a, const aabb &b) {
        const vec3f dist_0 = b.min_extent - a.max_extent;
        const vec3f dist_1 = a.min_extent - b.max_extent;
        const vec3f dist = dist_0.max(dist_1);

        const f32 max_dist = dist.max();

        return intersect_data{.intersects = max_dist < 0,
                              .dist = max_dist};
    }

    intersect_data operator () (const bounding_box &a, const bounding_box &b) {
        std::unreachable();
    }

    intersect_data operator () (const plane &plane, const bounding_sphere &sphere) {
        const f32 center_dist = std::abs(plane.normal.dot(sphere.center) + plane.dist);
        const f32 dist = center_dist - sphere.radius;

        return intersect_data{.intersects = dist < 0,
                              .dist = dist};
    }

    intersect_data operator () (const bounding_sphere &sphere, const plane &plane) {
        return this->operator()(plane, sphere);
    }

    intersect_data operator () (auto &, auto &) {
        std::unreachable();
    }
};

intersect_data collider::test_intersect(const collider &other) const {
    return std::visit(test_intersect_visitor{}, 
                      this->shape, 
                      other.shape);
}

handle<collider> rigid_body::add_collider(const collider &collider) {
    colliders.push_back(collider);
    calculate_mass();
    return colliders.end();
}

void rigid_body::remove_collider(const_handle<collider> collider) {
    colliders.erase(collider);
    calculate_mass();
}

vec3f rigid_body::get_local_point(const vec3f &point) const {
    std::unreachable();
}

vec3f rigid_body::get_local_vector(const vec3f &vector) const {
    std::unreachable();
}

vec3f rigid_body::get_world_point(const vec3f &point) const {
    std::unreachable();
}

vec3f rigid_body::get_world_vector(const vec3f &vector) const {
    std::unreachable();
}

void rigid_body::apply_linear_force(const vec3f &force) {
    std::unreachable();
}

void rigid_body::apply_force_at_world_point(const vec3f &force, const vec3f &point) {
    std::unreachable();
}

void rigid_body::apply_linear_impulse(const vec3f &impulse) {
    std::unreachable();
}

void rigid_body::apply_linear_impulse_at_world_point(const vec3f &impulse, const vec3f &point) {
    std::unreachable();
}

vec3f rigid_body::get_velocity_at_world_point(const vec3f &point) const {
    std::unreachable();
}

void rigid_body::set_linear_velocity(const vec3f &velocity) {
    std::unreachable();
}

void rigid_body::set_angular_velocity() const {
    std::unreachable();
}

void rigid_body::set_transform(const vec3f &pos) {
    std::unreachable();
}

void rigid_body::set_transform(const vec3f &pos, const vec3f &axis, f32 angle) {
    std::unreachable();
}

void rigid_body::calculate_mass() {
    std::unreachable();
}

struct solve_collision_visitor {
    manifold &manifold;

    void operator () (bounding_box &a, bounding_box &b) {

    }

    void operator () (auto &, auto &) {
        std::unreachable();
    }
};

void contact_constraint::solve_collision() {
    manifold.contact_count = 0;

    std::visit(solve_collision_visitor{.manifold = manifold}, 
               collider_a->shape, 
               collider_b->shape);
}

handle<rigid_body> scene::add_body(const rigid_body &body) {
    bodies.push_back(body);
    return bodies.end();
}

void scene::remove_body(const_handle<rigid_body> body) {
    bodies.erase(body);
}

void scene::step(f32 delta) {
    // contact manager: test collisions

    // solve

    // contact manager: find new contacts

    // clear all forces
}
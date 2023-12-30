#pragma once

#include <vector>
#include <list>
#include <memory>
#include <variant>

#include "vlk.math.hpp"
#include "vlk.util.hpp"

namespace vlk {
    struct plane;

    struct transform {
        vec3f pos;
        mat3 rot;

        void set_identity();

        transform mul(const transform &t) const;
        vec3f mul(const vec3f &v) const;
        plane mul(const plane &plane) const;
        vec3f mul(const vec3f &scale, const vec3f &v) const;
        plane mul(const vec3f &scale, const plane &plane) const;
        vec3f mul_transpose(const vec3f &v) const;
        transform mul_transpose(const transform &t) const;
        plane mul_transpose(const plane &plane) const;

        transform inverse() const;
    };

    struct bounding_sphere {
        vec3f center;
        f32 radius;
    };

    struct aabb {
        vec3f min_extent;
        vec3f max_extent;
    };

    struct bounding_box {
        transform transform;
    };

    struct plane {
        vec3f normal;
        // Distance to world origin i.e. [0, 0, 0].
        f32 dist;

        plane normalize() const;
    };

    struct intersect_data {
        bool intersects;
        f32 dist;
    };

    struct collider {
        std::variant<bounding_sphere, aabb, bounding_box, plane> shape;

        intersect_data test_intersect(const collider &other) const;
    };

    template <typename T>
    using handle = std::list<T>::iterator;

    template <typename T>
    using const_handle = std::list<T>::const_iterator;

    class rigid_body {
    public:
        handle<collider> add_collider(const collider &collider);
        void remove_collider(const_handle<collider> collider);

        vec3f get_local_point(const vec3f &point) const;
        vec3f get_local_vector(const vec3f &vector) const;
        vec3f get_world_point(const vec3f &point) const;
        vec3f get_world_vector(const vec3f &vector) const;

        void apply_linear_force(const vec3f &force);
        void apply_force_at_world_point(const vec3f &force, const vec3f &point);
        void apply_linear_impulse(const vec3f &impulse);
        void apply_linear_impulse_at_world_point(const vec3f &impulse, const vec3f &point);
        vec3f get_velocity_at_world_point(const vec3f &point) const;
        void set_linear_velocity(const vec3f &velocity);
        void set_angular_velocity() const;
        void set_transform(const vec3f &pos);
        void set_transform(const vec3f &pos, const vec3f &axis, f32 angle);

        std::list<collider> colliders;

        mat3 inv_inertia_model;
        mat3 inv_inertia_world;
        f32 mass;
        f32 inv_mass;
        vec3f linear_vel;
        vec3f angular_vel;
        vec3f force;
        vec3f torque;
        transform tx;
        quaternion q;
        vec3f local_center;
        vec3f world_center;
        f32 sleep_time;
        f32 gravity_scale;
        f32 linear_damping;
        f32 angular_damping;

        /*enum class flags : u8 {
            awake,
            active,
            dynamic
        };

        flag_set<flags> active_flags;*/

    private:
        void calculate_mass();
    };

    struct contact {
        vec3f pos;
        f32 penetration;
        f32 normal_impulse;
        std::array<f32, 2> tangent_impulse;
        f32 bias;
        f32 normal_mass;
        std::array<f32, 2> tangent_mass;
    };

    struct contact_constraint;

    struct contact_edge {
        handle<rigid_body> other_body;
        handle<contact_constraint> constraint;
    };

    struct manifold {
        handle<collider> collider_a;
        handle<collider> collider_b;

        vec3f normal;
        std::array<vec3f, 2> tangents;
        std::array<contact, 8> contacts;
        i32 contact_count;
    };

    struct contact_constraint {
        handle<collider> collider_a;
        handle<collider> collider_b;

        handle<rigid_body> body_a;
        handle<rigid_body> body_b;

        contact_edge edge_a;
        contact_edge edge_b;

        f32 friction;
        f32 restitution;

        manifold manifold;

        void solve_collision();
    };

    class scene {
    public:
        handle<rigid_body> add_body(const rigid_body &body);
        void remove_body(const_handle<rigid_body> body);

        void step(f32 delta);

        std::list<rigid_body> bodies;
        vec3f gravity;
        i32 iterations;
    };
}  // namespace vlk
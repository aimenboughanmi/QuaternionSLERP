#pragma once
#include <cmath>
#include "math/Vec3.h"

struct Quat {
public:
    double w, x, y, z;

    Quat() : w(1), x(0), y(0), z(0) {}
    Quat(const double w_, const double x_, const double y_, const double z_) : w(w_), x(x_), y(y_), z(z_) {}

    static Quat fromAxisAngle(const Vec3& axis, const double angle) {
        const Vec3 u = axis.normalized();
        const double half = angle * 0.5;
        double s = std::sin(half);
        return Quat{ std::cos(half), u.x * s, u.y * s, u.z * s };
    }

    // Normalize (Unit Quats in Hypersphere)
    void normalize() {
        const double magnitude = std::sqrt(w * w + x * x + y * y + z * z);
        if (magnitude == 0.0) {
            w = 1.0; x = 0.0; y = 0.0; z = 0.0;
            return;
        }
        w /= magnitude;
        x /= magnitude;
        y /= magnitude;
        z /= magnitude;
    }

    // Quaternion Multiplication
    Quat operator*(const Quat& other) const {
        return Quat{
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w
        };
    }

    // Inverse quaternion (nodiscard to avoid q.inverse() since we return new object)
    [[nodiscard]] Quat inverse() const {
        double normSq = w * w + x * x + y * y + z * z;
        if (normSq == 0.0) return Quat{1.0, 0.0, 0.0, 0.0};
        return Quat{ w / normSq, -x / normSq, -y / normSq, -z / normSq };
    }

    // Rotate vector (vertex)
    [[nodiscard]] Vec3 rotate(const Vec3& vec) const {
        Quat q = *this;
        q.normalize(); // unit quaternion for pure rotation
        Quat v{ 0, vec.x, vec.y, vec.z };
        const Quat r = q * v * q.inverse();
        return Vec3{ r.x, r.y, r.z };
    }

    // SLERP interpolation
    static Quat SLERP(const Quat& q0, const Quat& q1, double t);

    // Linear interpolation — NOT normalized; magnitude shrinks between endpoints
    static Quat LERP(const Quat& q0, const Quat& q1, double t) {
        return Quat{
            (1.0 - t) * q0.w + t * q1.w,
            (1.0 - t) * q0.x + t * q1.x,
            (1.0 - t) * q0.y + t * q1.y,
            (1.0 - t) * q0.z + t * q1.z
        };
    }

    // Normalized LERP — fixes magnitude but angular speed is not constant
    static Quat NLERP(const Quat& q0, const Quat& q1, double t) {
        Quat result = LERP(q0, q1, t);
        result.normalize();
        return result;
    }
};

inline Quat Quat::SLERP(const Quat& q0, const Quat& q1, double t) {
    double cosTheta = q0.w * q1.w + q0.x * q1.x + q0.y * q1.y + q0.z * q1.z;

    // Ensure shortest path
    Quat q1Copy = q1;
    if (cosTheta < 0.0) {
        q1Copy.w = -q1Copy.w; q1Copy.x = -q1Copy.x;
        q1Copy.y = -q1Copy.y; q1Copy.z = -q1Copy.z;
        cosTheta = -cosTheta;
    }

    // Clamp for numerical safety before acos
    if (cosTheta > 1.0) cosTheta = 1.0;
    if (cosTheta < -1.0) cosTheta = -1.0;

    const double THRESHOLD = 0.9995;
    // LERP if Quats are close
    if (cosTheta > THRESHOLD) {
        Quat result{
            (1.0 - t) * q0.w + t * q1Copy.w,
            (1.0 - t) * q0.x + t * q1Copy.x,
            (1.0 - t) * q0.y + t * q1Copy.y,
            (1.0 - t) * q0.z + t * q1Copy.z
        };
        result.normalize();
        return result;
    }

    // SLERP implementation
    const double theta = std::acos(cosTheta); // angle between quaternions
    const double sin_theta = std::sin(theta);
    const double s1 = std::sin((1.0 - t) * theta) / sin_theta;
    const double s2 = std::sin(t * theta) / sin_theta;

    return Quat{
        s1 * q0.w + s2 * q1Copy.w,
        s1 * q0.x + s2 * q1Copy.x,
        s1 * q0.y + s2 * q1Copy.y,
        s1 * q0.z + s2 * q1Copy.z
    };
}
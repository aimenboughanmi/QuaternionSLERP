#pragma once
#include <cmath>

struct Vec3 {
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    // Addition
    Vec3 operator+(const Vec3& other) const {
        return Vec3{ x + other.x, y + other.y, z + other.z };
    }

    Vec3& operator+=(const Vec3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    // Subtraction
    Vec3 operator-(const Vec3& other) const {
        return Vec3{ x - other.x, y - other.y, z - other.z };
    }

    Vec3& operator-=(const Vec3& other) {
        x -= other.x; y -= other.y; z -= other.z;
        return *this;
    }

    // Right scalar multiplication
    Vec3 operator*(double scalar) const {
        return Vec3{ x * scalar, y * scalar, z * scalar };
    }

    // Dot product
    [[nodiscard]] double dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Cross product
    [[nodiscard]] Vec3 cross(const Vec3& other) const {
        return Vec3{
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    // Magnitude
    [[nodiscard]] double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Normalize
    [[nodiscard]] Vec3 normalized() const {
        double len = length();
        if (len == 0) return Vec3{0, 0, 0};
        return Vec3{ x / len, y / len, z / len };
    }
    
};

// Non-member function for left scalar multiplication
inline Vec3 operator*(double scalar, const Vec3& v) {
    return { v.x * scalar, v.y * scalar, v.z * scalar };
}
#include <cassert>
#include <cmath>

#include "math/Quat.h"

namespace {
bool nearlyEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}
}

int main() {
    constexpr double kPi = 3.141592653589793;

    Quat q = Quat::fromAxisAngle(Vec3{0.0, 0.0, 1.0}, kPi / 2.0);
    Vec3 rotated = q.rotate(Vec3{1.0, 0.0, 0.0});
    assert(nearlyEqual(rotated.x, 0.0));
    assert(nearlyEqual(rotated.y, 1.0));
    assert(nearlyEqual(rotated.z, 0.0));

    Quat start = Quat::fromAxisAngle(Vec3{0.0, 1.0, 0.0}, 0.0);
    Quat end = Quat::fromAxisAngle(Vec3{0.0, 1.0, 0.0}, kPi);
    Quat mid = Quat::SLERP(start, end, 0.5);
    Vec3 halfway = mid.rotate(Vec3{1.0, 0.0, 0.0});
    assert(nearlyEqual(halfway.x, 0.0));
    assert(nearlyEqual(halfway.z, -1.0));

    Quat lerp = Quat::NLERP(start, end, 0.25);
    Vec3 quarter = lerp.rotate(Vec3{1.0, 0.0, 0.0});
    assert(quarter.length() > 0.999);
    assert(quarter.length() < 1.001);

    return 0;
}
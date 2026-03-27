#include <cassert>
#include <cmath>

#include "math/Vec3.h"

namespace {
bool nearlyEqual(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) < epsilon;
}
}

int main() {
    Vec3 a{1.0, 2.0, 3.0};
    Vec3 b{4.0, 5.0, 6.0};

    Vec3 sum = a + b;
    assert(nearlyEqual(sum.x, 5.0));
    assert(nearlyEqual(sum.y, 7.0));
    assert(nearlyEqual(sum.z, 9.0));

    Vec3 diff = b - a;
    assert(nearlyEqual(diff.x, 3.0));
    assert(nearlyEqual(diff.y, 3.0));
    assert(nearlyEqual(diff.z, 3.0));

    assert(nearlyEqual(a.dot(b), 32.0));

    Vec3 cross = Vec3{1.0, 0.0, 0.0}.cross(Vec3{0.0, 1.0, 0.0});
    assert(nearlyEqual(cross.x, 0.0));
    assert(nearlyEqual(cross.y, 0.0));
    assert(nearlyEqual(cross.z, 1.0));

    Vec3 unit = Vec3{3.0, 0.0, 4.0}.normalized();
    assert(nearlyEqual(unit.length(), 1.0));
    assert(nearlyEqual(unit.x, 0.6));
    assert(nearlyEqual(unit.z, 0.8));

    return 0;
}
// Sanity checks for Vec3 / Mat3 / Quat (quaternion).
//
// Standalone console executable - no raylib, no window.
//   cmake --build cmake-build-debug --target phyX_tests
//   ./cmake-build-debug/phyX_tests
// In Rider, pick the "phyX_tests" run configuration after reloading CMake.

#include "Quaternion.h"   // pulls in Mat3.h and Vector.h
#include <cstdio>
#include <cmath>
#include <cstdlib>

static int g_failed = 0;

static void check(bool ok, const char* name)
{
    printf("  %s  %s\n", ok ? "[ OK ]" : "[FAIL]", name);
    if (!ok) ++g_failed;
}

static bool approx(float a, float b, float eps = 1e-4f)
{
    return std::fabs(a - b) < eps;
}

static bool vecApprox(const Vec3& a, const Vec3& b, float eps = 1e-4f)
{
    return approx(a.x, b.x, eps) && approx(a.y, b.y, eps) && approx(a.z, b.z, eps);
}

static bool matApprox(const Mat3& a, const Mat3& b, float eps = 1e-4f)
{
    return vecApprox(a.r0, b.r0, eps) && vecApprox(a.r1, b.r1, eps) && vecApprox(a.r2, b.r2, eps);
}

// NOTE: this belongs in Mat3 as `Vec3 operator*(const Vec3&) const`.
// It is the most-used matrix operation and the struct is still missing it.
// Add it there, then delete this helper.
static Vec3 mulMV(const Mat3& m, const Vec3& v)
{
    return Vec3(m.r0.dot(v), m.r1.dot(v), m.r2.dot(v));
}

static void printMat(const char* label, const Mat3& m)
{
    printf("    %s\n", label);
    printf("      [%6.2f %6.2f %6.2f]\n", m.r0.x, m.r0.y, m.r0.z);
    printf("      [%6.2f %6.2f %6.2f]\n", m.r1.x, m.r1.y, m.r1.z);
    printf("      [%6.2f %6.2f %6.2f]\n", m.r2.x, m.r2.y, m.r2.z);
}

// Deterministic pseudo-random unit quaternion.
static Quat randomUnitQuat()
{
    auto r = []{ return (float)std::rand() / (float)RAND_MAX * 2.0f - 1.0f; };
    Quat q(r(), Vec3(r(), r(), r()));
    if (q.length() < 1e-3f) return Quat();
    return q.normalize();
}

int main()
{
    std::srand(1234);          // fixed seed - failures are reproducible
    const Mat3 I;              // default ctor is the identity

    printf("\n--- Mat3 ---\n");

    // The check that catches a row/column mix-up in operator*.
    check(matApprox(I * I, I), "identity * identity == identity");
    check(approx((I * I).determinant(), 1.0f), "det(identity * identity) == 1");
    check(approx(I.determinant(), 1.0f), "det(identity) == 1");
    check(matApprox(I.transpose(), I), "transpose(identity) == identity");

    {
        // A matrix multiply must not be symmetric in its arguments.
        Mat3 a(Vec3(1, 2, 3), Vec3(0, 1, 4), Vec3(5, 6, 0));
        check(matApprox(a * I, a), "a * identity == a");
        check(matApprox(I * a, a), "identity * a == a");
        check(!matApprox(a * a.transpose(), a.transpose() * a),
              "a*aT != aT*a  (multiply is not commutative)");
    }

    printf("\n--- Quat -> Mat3 (rotation matrix) ---\n");

    bool orthonormal = true, unitDet = true, agreesWithRotate = true, lengthKept = true;
    for (int i = 0; i < 200; ++i)
    {
        Quat q = randomUnitQuat();
        Mat3 R = q.ToMatrixReference();

        // A rotation matrix times its own transpose is the identity.
        if (!matApprox(R * R.transpose(), I, 1e-3f)) orthonormal = false;

        // det +1 is a rotation; det -1 would be a reflection, which also
        // passes the orthonormality check above. This is the test that
        // separates them.
        if (!approx(R.determinant(), 1.0f, 1e-3f)) unitDet = false;

        Vec3 v(0.3f, -1.7f, 2.2f);
        if (!vecApprox(mulMV(R, v), q.rotate(v), 1e-3f)) agreesWithRotate = false;

        // A rotation never changes a vector's length.
        if (!approx(q.rotate(v).length(), v.length(), 1e-3f)) lengthKept = false;
    }
    check(orthonormal,      "R * transpose(R) == identity   (200 random quaternions)");
    check(unitDet,          "det(R) == +1, not -1           (not a reflection)");
    check(agreesWithRotate, "R * v == q.rotate(v)           (FromColumns convention)");
    check(lengthKept,       "|q.rotate(v)| == |v|           (rotation is an isometry)");

    printf("\n--- Quat basics ---\n");
    {
        Quat q = randomUnitQuat();
        check(approx(q.length(), 1.0f), "normalize() gives unit length");
        Quat qq = q * q.conjugate();
        check(approx(qq.w, 1.0f) && vecApprox(qq.vector, Vec3(0, 0, 0)),
              "q * conjugate(q) == identity quaternion");
        Vec3 axis(0, 1, 0);
        Quat spin = Quat::fromAxisAngle(3.14159265f * 0.5f, axis);
        check(vecApprox(spin.rotate(axis), axis), "rotating the axis itself leaves it unchanged");
    }

    printf("\n--- HANDEDNESS (decide once, then write it in Quaternion.h) ---\n");
    {
        Quat ry90 = Quat::fromAxisAngle(3.14159265f * 0.5f, Vec3(0, 1, 0));
        Vec3 x(1, 0, 0);
        Vec3 out = ry90.rotate(x);
        printf("    (1,0,0) rotated +90 deg about Y  ->  (%.2f, %.2f, %.2f)\n", out.x, out.y, out.z);
        printf("    %s\n", out.z < 0 ? "=> x maps to -z  (right-handed, OpenGL/raylib convention)"
                                     : "=> x maps to +z  (left-handed)");
        printMat("matrix for that rotation:", ry90.ToMatrixReference());
    }

    printf("\n=====================================\n");
    if (g_failed == 0) printf("  ALL CHECKS PASSED\n");
    else               printf("  %d CHECK(S) FAILED\n", g_failed);
    printf("=====================================\n\n");
    return g_failed == 0 ? 0 : 1;
}

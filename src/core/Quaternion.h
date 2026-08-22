#pragma once
#include <cmath>
#include "Vector.h"
#include "Mat3.h"

// Named "Quat", not "Quaternion", on purpose:
// raylib.h declares `typedef Vector4 Quaternion;` (guarded by RL_QUATERNION_TYPE),
// so the name collides in any file that sees both headers - e.g. main.cpp.
// Layouts differ too (raylib is {x,y,z,w}, this is {w,x,y,z}), so aliasing them
// would corrupt data rather than just fail to compile.
//
// HANDEDNESS (verified by phyX_tests): right-handed, matching raylib/OpenGL.
//   (1,0,0) rotated +90 deg about Y  ->  (0,0,-1)
struct Quat
{
    float w;
    Vec3 vector;

    Quat(): w(1), vector(0,0,0) {}
    Quat(float _w, Vec3 v) : w(_w), vector(v) {}
    static Quat fromAxisAngle(float angleRad, Vec3 axis) {
        axis = axis.normalize();
        return Quat(cos(angleRad * 0.5f), axis * sin(angleRad * 0.5f));
    }
    Quat operator+(const Quat& other) const
    {
        return Quat(w + other.w,Vec3(vector.x + other.vector.x, vector.y + other.vector.y, vector.z + other.vector.z));
    }
    Quat operator-(const Quat& other) const
    {
        return Quat(w - other.w,Vec3(vector.x - other.vector.x, vector.y - other.vector.y, vector.z - other.vector.z));
    }
    Quat operator*(const Quat& other) const
    {
        return Quat (w*other.w - vector.dot(other.vector),
                            Vec3(other.vector*w + vector*other.w + (vector.cross(other.vector))));
    }
    Quat operator*(float s) const
    {
        return Quat(
            w*s,
            vector*s
        );
    }
    Quat inverse() const
    {
        float l = w*w + vector.dot(vector);
        return conjugate()/l;
    }
    Quat operator/(const Quat& other) const
    {
        return *this * other.inverse();
    }
    Quat operator/(float s) const
    {
        return Quat(
            w/s,
            vector/s
        );
    }
    float length() const
    {
        return sqrt(w*w + vector.dot(vector));
    }
    Quat normalize() const
    {
        float l = length();
        if(l == 0.f)
            return Quat();
        return Quat(w/l,Vec3(vector.x/l,vector.y/l,vector.z/l));
    }
    
    Quat conjugate() const
    {
        return Quat(
            w,-vector);
    }
    
    Vec3 rotate(const Vec3 &p) const
    {
        Quat pure(0.f,p);
        Quat r = *this *pure*conjugate();
        return r.vector;
    }
    
    Mat3 ToMatrixReference() const
    {
        return Mat3::FromColumns(rotate(Vec3(1,0,0)),rotate(Vec3(0,1,0)),rotate(Vec3(0,0,1)));
    }
};
#pragma once
#include <cmath>
#include "Vector.h"

struct Quaternion
{
    float w;
    Vec3 vector;

    Quaternion(): w(1), vector(0,0,0) {}
    Quaternion(float _w, Vec3 v) : w(_w), vector(v) {}
    static Quaternion fromAxisAngle(float angleRad, Vec3 axis) {
        axis = axis.normalize();
        return Quaternion(cos(angleRad * 0.5f), axis * sin(angleRad * 0.5f));
    }
    Quaternion operator+(const Quaternion& other) const
    {
        return Quaternion(w + other.w,Vec3(vector.x + other.vector.x, vector.y + other.vector.y, vector.z + other.vector.z));
    }
    Quaternion operator-(const Quaternion& other) const
    {
        return Quaternion(w - other.w,Vec3(vector.x - other.vector.x, vector.y - other.vector.y, vector.z - other.vector.z));
    }
    Quaternion operator*(const Quaternion& other) const
    {
        return Quaternion (w*other.w - vector.dot(other.vector),
                            Vec3(other.vector*w + vector*other.w + (vector.cross(other.vector))));
    }
    Quaternion operator*(float s) const
    {
        return Quaternion(
            w*s,
            vector*s
        );
    }
    Quaternion inverse() const
    {
        float l = w*w + vector.dot(vector);
        return conjugate()/l;
    }
    Quaternion operator/(const Quaternion& other) const
    {
        return *this * other.inverse();
    }
    Quaternion operator/(float s) const
    {
        return Quaternion(
            w/s,
            vector/s
        );
    }
    float length() const
    {
        return sqrt(w*w + vector.dot(vector));
    }
    Quaternion normalize() const
    {
        float l = length();
        if(l == 0.f)
            return Quaternion();
        return Quaternion(w/l,Vec3(vector.x/l,vector.y/l,vector.z/l));
    }
    
    Quaternion conjugate() const
    {
        return Quaternion(
            w,-vector);
    }
    
    Vec3 rotate(Vec3 &p) const
    {
        Quaternion pure(0.f,p);
        Quaternion r = *this *pure*conjugate();
        return r.vector;
    }
};
#pragma once

#include "Vector.h"

struct Mat3
{
    Vec3 r0, r1, r2;
    Mat3():r0(1,0,0),r1(0,1,0),r2(0,0,1) {}
    Mat3(Vec3 a, Vec3 b, Vec3 c):r0(a),r1(b),r2(c) {}
    
    static Mat3 FromColumns(const Vec3 &col1, const Vec3 &col2, const Vec3 &col3)
    {
        return Mat3(Vec3(col1.x,col2.x,col3.x),
            Vec3(col1.y,col2.y,col3.y),
            Vec3(col1.z,col2.z,col3.z));
    }
    static Mat3 diagonal(const Vec3 &v)
    {
        return Mat3(Vec3(v.x,0,0),
            Vec3(0,v.y,0),
            Vec3(0,0,v.z));
    }
    
    Mat3 transpose() const
    {
        return Mat3(Vec3(r0.x,r1.x,r2.x),
            Vec3(r0.y,r1.y,r2.y),
            Vec3(r0.z,r1.z,r2.z));
    }
    
    Mat3 operator*(const Mat3 &other) const
    {
        Mat3 t = other.transpose();
        return Mat3(
            Vec3(r0.dot(t.r0), r0.dot(t.r1), r0.dot(t.r2)),
            Vec3(r1.dot(t.r0), r1.dot(t.r1), r1.dot(t.r2)),
            Vec3(r2.dot(t.r0), r2.dot(t.r1), r2.dot(t.r2)));
    }
    Vec3 operator*(const Vec3 &v) const
    {
        return Vec3(r0.dot(v), r1.dot(v), r2.dot(v));
    }
    
    float determinant() const {return r0.dot(r1.cross(r2));}
};

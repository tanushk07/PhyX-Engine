#pragma once
#include <cmath>
enum Axis {x, y, z };

struct Vec3 {
	float x, y, z;

	Vec3(): x(0), y(0), z(0) {}
	Vec3(float _x, float _y, float _z) : x(_x),y(_y),z(_z){}

	Vec3 operator+(const Vec3 other) const
	{
		return Vec3(other.x + x, other.y + y, other.z + z);
	}

	Vec3 operator-() const
	{
		return Vec3(-x, -y, -z);
	}

	Vec3 operator-(const Vec3 other) const
	{
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	float dot(const Vec3 other) const
	{
		return (x * other.x + y * other.y + z * other.z);
	}
	Vec3 operator*(float s) const
	{
		return Vec3(s * x, s * y, s * z);
	}
	Vec3 operator/(float s) const
	{
		return Vec3(x/s, y/s, z/s);
	}
	Vec3& operator+=(const Vec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Vec3& operator-=(const Vec3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	Vec3 cross(const Vec3& other) const
	{
		return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	float length() const
	{
		return sqrt(
			x * x + y * y + z * z
		);
	}
	float lengthsqr() const
	{
		return (
			x * x + y * y + z * z
		);
	}
	Vec3 normalize() const
	{
		float len = length();
		if (len > 0)
			return Vec3(x / len, y / len, z / len);
		return Vec3(0, 0, 0);
	}

	inline Vec3 Max(const Vec3& other) const
	{
		return Vec3(
			x > other.x ? x : other.x,
			y > other.y ? y : other.y,
			z > other.z ? z : other.z
		);
	}
	inline float Max(){
		float maxval = x;
		if (y > maxval)
		{
			maxval = y;
		}
		if(z > maxval)
		{
			maxval = z;
		}
		return maxval;
	}
	inline float Min(){
		float minval = x;
		if (y < minval) minval = y;
		if (z < minval) minval = z;
		return minval;
	}
	
	Axis MaxAxis() const
	{
		float MaxVal = x;
		Axis a = Axis::x;
		if (y > MaxVal)
		{
			MaxVal = y;
			a = Axis::y;
		}
		if(z > MaxVal)
		{
			a = Axis::z;
		}
		return a;
	}
	Axis MinAxis() const
	{
		float MinVal = x;
		Axis a = Axis::x;
		if (y < MinVal)
		{
			MinVal = y;
			a = Axis::y;
		}
		if(z < MinVal) a = Axis::z;
		return a;
	}
};
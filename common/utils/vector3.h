#ifndef VECTOR3_H
#define VECTOR3_H

#include <math.h>
#include <assert.h>
#include <iostream>
#define PI 3.14159265
using namespace std;

class Vector3
{
    public:
        float x;
        float y;
        float z;
        Vector3();
        Vector3(float _x, float _y, float _z);
        ~Vector3();
        bool HasNaNs() const;
        void set(float _x, float _y, float _z);

        //Static functions
		static Vector3 back() { return Vector3(0, 0, -1); }
		static Vector3 forward() { return Vector3(0, 0, 1); }
		static Vector3 down() { return Vector3(0, -1, 0); }
		static Vector3 up() { return Vector3(0, 1, 0); }
        static Vector3 left() { return Vector3(-1, 0, 0); }
        static Vector3 right() { return Vector3(1, 0, 0); }
		static Vector3 zero() { return Vector3(0, 0, 0); }
		static Vector3 one() { return Vector3(1, 1, 1); }

        float magnitude() const;
        float sqrtMagnitude() const;
        Vector3 normalized();

        //Operators
        //Add two Vectors
        Vector3 operator+(const Vector3 &v) const;
        //Subtracts one vector from another
        Vector3 operator-(const Vector3 &v) const;
        Vector3 operator*(float num) const;
        Vector3 operator/(float num) const;
        Vector3 &operator+=(const Vector3 &v);
        Vector3 &operator-=(const Vector3 &v);
        Vector3 &operator*=(float num);
        Vector3 &operator/=(float num);
        //Returns true if vectors different.
        bool operator!=(Vector3 &v) const;

        friend std::ostream& operator<<(std::ostream& output,const Vector3& v)
        {
            output<<"("<<v.x<<","<<v.y<<","<<v.z<<")"<<"\n";
            return output;
        }

		//Static Functions
        //Calculate angle between two vectors.
        static float Angle(const Vector3 &from, const Vector3 &to)
        {
            double c, d;
            c = from.x*to.x + from.y*to.y + from.z*to.z;
            d = sqrt(from.x*from.x + from.y*from.y + from.z*from.z) * sqrt(to.x*to.x + to.y*to.y + to.z*to.z);
            return acos(c/d) * 180.0 / PI;
        }

        //Get dot result of two vector2.
        static inline float Dot(const Vector3 &v1, const Vector3 &v2)
        {
            return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
        }

        //Get cross result of two vector2.
        static inline Vector3 Cross(const Vector3 &v1, const Vector3 &v2)
        {
            return Vector3((v1.y * v2.z) - (v1.z * v2.y),
                           (v1.z * v2.x) - (v1.x * v2.z),
                           (v1.x * v2.y) - (v1.y * v2.x));
        }

        //Get a normalized vector3 from a given vector3.
        static inline Vector3 Normalize(const Vector3 &v1)
        {
            float tmpLength = v1.magnitude();
            return Vector3(v1.x/tmpLength, v1.y/tmpLength, v1.z/tmpLength);
        }

        //Construct a coordinateSystem by a given vector3. v2 and v3 are output.
        static inline void CoordinateSystem(const Vector3 &v1, Vector3 *v2, Vector3 *v3)
        {
            if(fabsf(v1.x) > fabsf(v1.y))
            {
                float invLen = 1.0f / sqrtf(v1.x*v1.x + v1.z*v1.z);
                *v2 = Vector3(-v1.z * invLen, 0.0f, v1.x * invLen);
            }else
            {
                float invLen = 1.0f / sqrt(v1.y*v1.y + v1.z*v1.z);
                *v2 = Vector3(0.0f, v1.z * invLen, -v1.y * invLen);
            }
            *v3 = Vector3::Cross(v1, *v2);
        }

        static Vector3 ClampMagnitude();
        static Vector3 Lerp(const Vector3 &v1, const Vector3 &v2);
        static Vector3 OrthoNormalize();
    protected:
    private:


};

#endif // VECTOR3_H

#include "math3D.h"
#include <vector>
#include <cmath>
#include <algorithm>

Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
Vec4::Vec4() : x(0), y(0), z(0), w(1) {}
Vec4::Vec4(float xx, float yy, float zz, float ww) : x(xx), y(yy), z(zz), w(ww) {}

Vec3 operator+(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator-(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 operator*(const Vec3& a, float s) {
    return Vec3(a.x * s, a.y * s, a.z * s);
}

Vec3 operator*(float s, const Vec3& a) {
    return Vec3(a.x * s, a.y * s, a.z * s);
}

Vec3 operator/(const Vec3& a, float s) {
    return Vec3(a.x / s, a.y / s, a.z / s);
}

Vec3 operator*(const Vec3& a, const Vec3& b) {
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

float dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    );
}

float length(const Vec3& v) {
    return std::sqrt(dot(v,v));
}

Vec3 normalize(const Vec3& v) {
    float len = length(v);
    if (len <= 1e-8f)
        return Vec3(0,0,0);
    return v / len;
}

Mat4 identity() {
    Mat4 I{};
    for (int i = 0; i < 16; ++i){
        I.m[i] = 0.0f;
    }
    I.m[0] = I.m[5] = I.m[10] = I.m[15] = 1.0f;
    return I;
}

Mat4 multiply(const Mat4& A, const Mat4& B) {
    Mat4 R{};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += A.m[row*4 + k] * B.m[k*4 + col];
            }
            R.m[row*4 + col] = sum;
        }
    }
    return R;
}

Vec4 mul(const Mat4& M, const Vec4& v) {
    Vec4 r;
    r.x = M.m[0]*v.x + M.m[1]*v.y + M.m[2]*v.z + M.m[3]*v.w;
    r.y = M.m[4]*v.x + M.m[5]*v.y + M.m[6]*v.z + M.m[7]*v.w;
    r.z = M.m[8]*v.x + M.m[9]*v.y + M.m[10]*v.z + M.m[11]*v.w;
    r.w = M.m[12]*v.x + M.m[13]*v.y + M.m[14]*v.z + M.m[15]*v.w;
    return r;
}

Mat4 rotationX(float degrees) {
    float t = degrees * 3.1415926535f / 180.0f;
    float c = std::cos(t);
    float s = std::sin(t);
    Mat4 M = identity();
    M.m[5]  = c;
    M.m[6]  = -s;
    M.m[9]  = s;
    M.m[10] = c;
    return M;
}

Mat4 rotationY(float degrees) {
    float t = degrees * 3.1415926535f / 180.0f;
    float c = std::cos(t);
    float s = std::sin(t);
    Mat4 M = identity();
    M.m[0]  = c;
    M.m[2]  = s;
    M.m[8]  = -s;
    M.m[10] = c;
    return M;
}

Mat4 translation(float tx, float ty, float tz) {
    Mat4 M = identity();
    M.m[3]  = tx;
    M.m[7]  = ty;
    M.m[11] = tz;
    return M;
}

Mat4 perspective(float fovY_deg, float aspect, float znear, float zfar) {
    float PI = 3.1415926535f;
    float f = 1.0f / std::tan(fovY_deg * 0.5f * PI / 180.0f);
    float nf = 1.0f / (znear - zfar);
    Mat4 M{};
    for (int i=0;i<16;++i) M.m[i]=0.0f;
    M.m[0]  = f / aspect;
    M.m[5]  = f;
    M.m[10] = (zfar + znear) * nf;
    M.m[11] = 2.0f * zfar * znear * nf;
    M.m[14] = -1.0f;
    return M;
}

Mat4 orthographic(float left, float right, float bottom, float top, float znear, float zfar) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = zfar - znear;
    Mat4 M{};
    for (int i=0;i<16;++i) M.m[i]=0.0f;
    M.m[0]  = 2.0f / rl;
    M.m[5]  = 2.0f / tb;
    M.m[10] = -2.0f / fn;
    M.m[3]  = -(right + left) / rl;
    M.m[7]  = -(top + bottom) / tb;
    M.m[11] = -(zfar + znear) / fn;
    M.m[15] = 1.0f;
    return M;
}

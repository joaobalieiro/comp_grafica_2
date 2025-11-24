#ifndef MATH3D_H
#define MATH3D_H
#include <vector>
#include <cmath>
#include <algorithm>


struct Vec3 {
    float x, y, z;
    Vec3();
    Vec3(float xx, float yy, float zz);
};

struct Vec4 {
    float x, y, z, w;
    Vec4();
    Vec4(float xx, float yy, float zz, float ww);
};

struct Mat4 {
    float m[16]; // Armazenamento em major-order de coluna (OpenGL-like) ou linha
};

struct Vertex {
    Vec3 pos;    // Posição (Object Space)
    Vec3 normal; // Normal
    Vec3 color;  // Cor base
};

struct Triangle {
    int i0, i1, i2; // Índices no vetor de vértices
};

struct RasterVertex {
    float x, y, z;    // Posição (Screen Space ou Clip Space/View Space após Transformação)
    Vec3  color;      // Cor
    Vec3  normal;     // Normal
    Vec3  posWorld;   // Posição no espaço do mundo
};

Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& a, float s); 
Vec3 operator*(float s, const Vec3& a); 
Vec3 operator/(const Vec3& a, float s);
Vec3 operator*(const Vec3& a, const Vec3& b); 

// Funções matemáticas
float dot(const Vec3& a, const Vec3& b);
float length(const Vec3& v);
Vec3 cross(const Vec3& a, const Vec3& b);
Vec3 normalize(const Vec3& v);

// Funções de Matrizes (Mat4) e Transformações

Mat4 identity();
Mat4 multiply(const Mat4& A, const Mat4& B);
Vec4 mul(const Mat4& M, const Vec4& v);
Mat4 rotationX(float degrees);
Mat4 rotationY(float degrees);
Mat4 translation(float tx, float ty, float tz);

// Funções de Projeção
Mat4 perspective(float fovY_deg, float aspect, float znear, float zfar);
Mat4 orthographic(float left, float right, float bottom, float top, float znear, float zfar);

#endif // MATH3D_H
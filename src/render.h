#ifndef RENDER_H
#define RENDER_H

#include "solidos.h"      // Inclui tipos de geometria e ObjectType
#include <vector>
#include <windows.h>      // Necessário para BITMAPINFO, HDC, StretchDIBits

// Buffers globais (definidos em render.cpp)
extern std::vector<float> framebuffer;       // RGB em [0,1]
extern std::vector<float> zbuffer;           // Profundidade
extern std::vector<unsigned char> g_dibData; // Buffer BGRA (Windows DIB)
extern BITMAPINFO g_bmi;                     // Cabeçalho do bitmap

// Modos de sombreamento
enum class ShadingMode {
    FLAT,    // Cor uniforme por triângulo
    GOURAUD, // Iluminação por vértice interpolada
    PHONG    // Iluminação por pixel
};

// Tipos de projeção da câmera
enum class ProjectionMode {
    PERSPECTIVE,   // Perspectiva
    ORTHOGRAPHIC   // Ortográfica
};

extern int WIDTH;
extern int HEIGHT;

void ndcToScreen(const Vec3& ndc, float& x, float& y);

// Estado atual da cena
extern ObjectType currentObject;
extern ShadingMode shadingMode;
extern ProjectionMode projectionMode;

// Parâmetros de visualização (câmera/modelo)
extern float angleX;
extern float angleY;
extern float zoom;

// Controle de mouse
extern bool g_mouseDown;
extern int g_lastMouseX;
extern int g_lastMouseY;

// Posição da luz e do observador
extern Vec3 lightPos;
extern Vec3 eyePos;
extern Vec3 Ia;
extern float ka;
extern float kd;
extern float ks;
extern float shininess;

// Funções principais do módulo de renderização
void ResizeBuffers(int w, int h);
void clearBuffers();
void renderScene();
void Present(HDC hdc);

#endif // RENDER_H

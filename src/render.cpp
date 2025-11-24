#include "render.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <windows.h> 


// DEFINIÇÃO DAS VARIÁVEIS GLOBAIS DE ESTADO (Declaradas em cena.h)

// Resolução do framebuffer
int WIDTH  = 800;
int HEIGHT = 600;

// Estado atual da cena
ObjectType currentObject   = ObjectType::CUBE;
ShadingMode shadingMode    = ShadingMode::PHONG;
ProjectionMode projectionMode = ProjectionMode::PERSPECTIVE;

// Parâmetros de visualização
float angleX = 20.0f;
float angleY = -30.0f;
float zoom   = 5.0f;

// Variáveis para possível controle de camera com o mouse
bool g_mouseDown = false;
int g_lastMouseX = 0;
int g_lastMouseY = 0;

// Posição da luz e do observador no espaço de mundo
Vec3 lightPos(2.0f, 2.0f, -2.0f);
Vec3 eyePos(0.0f, 0.0f, 0.0f);
Vec3 lightColor(1.0f, 1.0f, 1.0f);
Vec3 Ia(0.1f, 0.1f, 0.1f);
float ka = 0.3f;
float kd = 0.8f;
float ks = 0.5f;
float shininess = 32.0f;

// DEFINIÇÃO DOS BUFFERS (Declaradas em render.h)
std::vector<float> framebuffer; 
std::vector<float> zbuffer;     
BITMAPINFO g_bmi{};
std::vector<unsigned char> g_dibData; 

// Converte coordenadas em NDC ([-1,1]x[-1,1]) para coordenadas de tela em pixels.
void ndcToScreen(const Vec3& ndc, float& x, float& y) {
    x = (ndc.x * 0.5f + 0.5f) * float(WIDTH - 1);
    y = (ndc.y * 0.5f + 0.5f) * float(HEIGHT - 1);
}

// Iluminação
Vec3 normalizarCor(const Vec3& c) {
    Vec3 r = c;
    r.x = std::max(0.0f, std::min(1.0f, r.x));
    r.y = std::max(0.0f, std::min(1.0f, r.y));
    r.z = std::max(0.0f, std::min(1.0f, r.z));
    return r;
}

Vec3 renderPhong(const Vec3& posWorld, const Vec3& normalWorld, const Vec3& baseColor) {

    Vec3 N = normalize(normalWorld);                // Vetor Normal
    Vec3 L = normalize(lightPos - posWorld);        // Vetor Luz
    Vec3 V = normalize(eyePos - posWorld);          // Vetor Visão
    Vec3 R = normalize(2.0f * dot(N, L) * N - L);   // Vetor Reflexão

    // Componente difusa:
    float diff = std::max(0.0f, dot(N, L));

    // Componente especular:
    float spec = 0.0f;
    if (diff > 0.0f) {
        spec = std::pow(std::max(0.0f, dot(R, V)), shininess);
    }

    // Termos de iluminacao:
    Vec3 ambient  = ka * (Ia * baseColor);
    Vec3 diffuse  = kd * diff * (lightColor * baseColor);
    Vec3 specular = ks * spec * lightColor;

    // Cor final
    Vec3 color = ambient + diffuse + specular;
    return normalizarCor(color);
}

// Rasterizacao por scan-line

RasterVertex interpolacaoRV(const RasterVertex& a, const RasterVertex& b, float t) {
    // Interpola linearmente entre dois RasterVertex.
    RasterVertex r;
    r.x = a.x + (b.x - a.x) * t;
    r.y = a.y + (b.y - a.y) * t;
    r.z = a.z + (b.z - a.z) * t;
    r.color      = a.color      + (b.color      - a.color     ) * t;
    r.normal     = a.normal     + (b.normal     - a.normal    ) * t;
    r.posWorld   = a.posWorld   + (b.posWorld - a.posWorld) * t;
    return r;
}

void desenhaScanline(int y, RasterVertex vLeft, RasterVertex vRight) {
    // Desenha uma linha horizontal (scanline) entre vLeft e vRight.

    // Garante que o vertice a esquerda fique a esquerda
    if (vLeft.x > vRight.x) std::swap(vLeft, vRight);

    // Calcula os limites da linha horizontal (scanline) arredondando para inteiro.
    int xStart = static_cast<int>(std::ceil(vLeft.x));
    int xEnd   = static_cast<int>(std::floor(vRight.x));
    if (xStart > xEnd) return;  // Se não houver pixels válidos entre os dois pontos, sai da função.

    // Diferença em X entre os dois vértices (usada para interpolação).
    float dx = vRight.x - vLeft.x;

    // Percorre todos os pixels da linha horizontal entre vLeft e vRight.
    for (int x = xStart; x <= xEnd; ++x) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) continue; // Ignora pixels fora da área da tela.

        // Calcula o fator de interpolação 't' ao longo da linha.
        float t;
        if (dx == 0.0f) {
            t = 0.0f;
        } else {
            t = (static_cast<float>(x) - vLeft.x) / dx;
        }

        // Interpola os atributos do vértice (posição, normal, cor, etc.).
        RasterVertex p = interpolacaoRV(vLeft, vRight, t);

        // Índice linear do pixel no z-buffer e framebuffer.
        int idx = y * WIDTH + x;
        if (p.z <= zbuffer[idx]) {   // Teste de profundidade (z-buffer): só desenha se o novo pixel estiver mais próximo.
            zbuffer[idx] = p.z;      // Atualiza o z-buffer com a profundidade do novo pixel.
            Vec3 color;
            
            // Escolhe o cálculo da cor dependendo do modelo de tonalizacao.
            if (shadingMode == ShadingMode::PHONG) { // No modo Phong: calcula iluminação por pixel usando posição, normal e cor.
                color = renderPhong(p.posWorld, p.normal, p.color); 
            } else {
                color = normalizarCor(p.color); // Nos modos Flat ou Gouraud: usa a cor já interpolada.
            }
            
            // Converte índice do pixel para posição no framebuffer (RGB).
            int base = idx * 3;
            framebuffer[base + 0] = color.x; // componente R
            framebuffer[base + 1] = color.y; // componente G
            framebuffer[base + 2] = color.z; // componente B
        }
    }
}


// Preenche triangulo com base horizontal "em baixo" (flat-bottom).
void rasterizeFlatBottom(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2) {
    float invSlope1 = (std::abs(v1.y - v0.y) < 1e-6f) ? 0.0f : 1.0f / (v1.y - v0.y);
    float invSlope2 = (std::abs(v2.y - v0.y) < 1e-6f) ? 0.0f : 1.0f / (v2.y - v0.y);

    int yStart = static_cast<int>(std::ceil(v0.y));
    int yEnd   = static_cast<int>(std::floor(v2.y));

    for (int y = yStart; y <= yEnd; ++y) {
        float t1 = (v1.y == v0.y) ? 0.0f : (static_cast<float>(y) - v0.y) * invSlope1;
        float t2 = (v2.y == v0.y) ? 0.0f : (static_cast<float>(y) - v0.y) * invSlope2;

        RasterVertex va = interpolacaoRV(v0, v1, t1);
        RasterVertex vb = interpolacaoRV(v0, v2, t2);
        desenhaScanline(y, va, vb);
    }
}

// Preenche triangulo com base horizontal "em cima" (flat-top).
void rasterizeFlatTop(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2) {
    float invSlope1 = (std::abs(v2.y - v0.y) < 1e-6f) ? 0.0f : 1.0f / (v2.y - v0.y);
    float invSlope2 = (std::abs(v2.y - v1.y) < 1e-6f) ? 0.0f : 1.0f / (v2.y - v1.y);

    int yStart = static_cast<int>(std::ceil(v0.y));
    int yEnd   = static_cast<int>(std::floor(v2.y));

    for (int y = yStart; y <= yEnd; ++y) {
        float t1 = (v2.y == v0.y) ? 0.0f : (static_cast<float>(y) - v0.y) * invSlope1;
        float t2 = (v2.y == v1.y) ? 0.0f : (static_cast<float>(y) - v1.y) * invSlope2;

        RasterVertex va = interpolacaoRV(v0, v2, t1);
        RasterVertex vb = interpolacaoRV(v1, v2, t2);
        desenhaScanline(y, va, vb);
    }
}

// Rasteriza um triangulo qualquer
void rasterizeTriangle(RasterVertex v0, RasterVertex v1, RasterVertex v2) {
    std::vector<RasterVertex> vs = {v0, v1, v2};
    std::sort(vs.begin(), vs.end(), [](const RasterVertex& a, const RasterVertex& b){
        return a.y < b.y;
    });
    v0 = vs[0];
    v1 = vs[1];
    v2 = vs[2];

    // Triangulo degenerado em y: nada a fazer
    if (std::abs(v2.y - v0.y) < 1e-6f) return;

    if (std::abs(v1.y - v0.y) < 1e-6f) {
        // Base embaixo (v0 e v1 no mesmo y)
        rasterizeFlatTop(v0, v1, v2);
    } else if (std::abs(v2.y - v1.y) < 1e-6f) {
        // Base em cima (v1 e v2 no mesmo y)
        rasterizeFlatBottom(v0, v1, v2);
    } else {
        // Caso geral: corta o triangulo em dois
        float t = (v1.y - v0.y) / (v2.y - v0.y);
        RasterVertex vSplit = interpolacaoRV(v0, v2, t);
        
        // Garante que vSplit.y esteja exatamente no mesmo y de v1
        vSplit.y = v1.y; 
        
        // Divide e rasteriza
        rasterizeFlatBottom(v0, v1, vSplit);
        rasterizeFlatTop(v1, vSplit, v2);
    }
}

// =============================
// Buffers
// =============================

// Redimensiona framebuffer, zbuffer e o buffer BGRA de saida
void ResizeBuffers(int w, int h) {
    // Garante que a largura e altura sejam positivas
    WIDTH  = (w > 1) ? w : 1;
    HEIGHT = (h > 1) ? h : 1;

    framebuffer.assign(WIDTH * HEIGHT * 3, 0.0f);
    zbuffer.assign(WIDTH * HEIGHT, 1.0f);
    g_dibData.assign(WIDTH * HEIGHT * 4, 0);

    // Configura o cabeçalho do bitmap
    ZeroMemory(&g_bmi, sizeof(g_bmi));
    g_bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth         = WIDTH;
    g_bmi.bmiHeader.biHeight        = -HEIGHT; // orientacao top-down
    g_bmi.bmiHeader.biPlanes        = 1;
    g_bmi.bmiHeader.biBitCount      = 32;
    g_bmi.bmiHeader.biCompression   = BI_RGB;
    g_bmi.bmiHeader.biSizeImage     = WIDTH * HEIGHT * 4;
}

// Limpa o framebuffer e zbuffer
void clearBuffers() {
    std::fill(framebuffer.begin(), framebuffer.end(), 0.0f);
    std::fill(zbuffer.begin(), zbuffer.end(), 1.0f);
}

// =============================
// Render principal
// =============================

void renderScene() {
    // 1) Limpa buffers
    clearBuffers();

    std::vector<Vertex> vertices;
    std::vector<Triangle> faces;
    getCurrentMesh(vertices, faces);

    // 2) Monta as matrizes Model, View e Projection (MVP)
    float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    Mat4 proj;
    if (projectionMode == ProjectionMode::PERSPECTIVE) {
        proj = perspective(60.0f, aspect, 1.0f, 20.0f);
    } else {
        float s = 3.0f;
        proj = orthographic(-s*aspect, s*aspect, -s, s, 1.0f, 20.0f);
    }

    Mat4 rotX = rotationX(angleX);
    Mat4 rotY = rotationY(angleY);
    Mat4 trans = translation(0.0f, 0.0f, -zoom);
    Mat4 model = multiply(trans, multiply(rotY, rotX));

    // Buffers auxiliares para diferentes espaços:
    std::vector<Vec4> clipPositions(vertices.size());
    std::vector<Vec3> ndcPositions(vertices.size());
    std::vector<Vec3> worldPositions(vertices.size());
    std::vector<Vec3> normalsWorld(vertices.size());

    // 3) Vertex Shader: Transforma cada vértice para o espaço de tela
    for (size_t i=0;i<vertices.size();++i) {
        // Objeto -> Mundo (Posição)
        Vec4 pLocal(vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z, 1.0f);
        Vec4 pWorld4 = mul(model, pLocal);
        Vec3 pWorld(pWorld4.x / pWorld4.w, pWorld4.y / pWorld4.w, pWorld4.z / pWorld4.w);
        worldPositions[i] = pWorld;

        // Objeto -> Mundo (Normal - parte linear)
        Vec3 nLocal = vertices[i].normal;
        Vec3 nWorld(
                model.m[0]*nLocal.x + model.m[1]*nLocal.y + model.m[2]*nLocal.z,
                model.m[4]*nLocal.x + model.m[5]*nLocal.y + model.m[6]*nLocal.z,
                model.m[8]*nLocal.x + model.m[9]*nLocal.y + model.m[10]*nLocal.z
        );
        normalsWorld[i] = normalize(nWorld);

        // Mundo -> Clip -> NDC
        Vec4 pClip4 = mul(proj, Vec4(pWorld.x, pWorld.y, pWorld.z, 1.0f));
        clipPositions[i] = pClip4;
        Vec3 ndc(pClip4.x / pClip4.w, pClip4.y / pClip4.w, pClip4.z / pClip4.w);
        ndcPositions[i] = ndc;
    }

    // 4) Pipeline: Percorre e Rasteriza os Triângulos
    for (const auto& tri : faces) {
        // Testes de visibilidade
        Vec4 c0 = clipPositions[tri.i0];
        Vec4 c1 = clipPositions[tri.i1];
        Vec4 c2 = clipPositions[tri.i2];

        if (c0.w <= 0.0f && c1.w <= 0.0f && c2.w <= 0.0f) continue;

        Vec3 nd0 = ndcPositions[tri.i0];
        Vec3 nd1 = ndcPositions[tri.i1];
        Vec3 nd2 = ndcPositions[tri.i2];

        if ((nd0.z < -1.0f && nd1.z < -1.0f && nd2.z < -1.0f) ||
            (nd0.z >  1.0f && nd1.z >  1.0f && nd2.z >  1.0f)) {
            continue;
        }

        Vec3 w0 = worldPositions[tri.i0];
        Vec3 w1 = worldPositions[tri.i1];
        Vec3 w2 = worldPositions[tri.i2];

        Vec3 n0 = normalsWorld[tri.i0];
        Vec3 n1 = normalsWorld[tri.i1];
        Vec3 n2 = normalsWorld[tri.i2];

        Vec3 base0 = vertices[tri.i0].color;
        Vec3 base1 = vertices[tri.i1].color;
        Vec3 base2 = vertices[tri.i2].color;

        Vec3 col0, col1, col2;
        Vec3 n0f, n1f, n2f;

        // Escolhe modelo de iluminacao (Pré-cálculo para Flat/Gouraud)
        if (shadingMode == ShadingMode::FLAT) {
            Vec3 e1 = w1 - w0;
            Vec3 e2 = w2 - w0;
            Vec3 nFace = normalize(cross(e1, e2));

            // Garante que a normal de face tenha a mesma orientacao
            // aproximada que a media das normais de vertice
            Vec3 nAvg = n0 + n1 + n2;        // nao precisa normalizar
            if (dot(nFace, nAvg) < 0.0f) {
                nFace = nFace * -1.0f;       // inverte no caso da esfera
            }

            Vec3 posFace  = (w0 + w1 + w2) * (1.0f / 3.0f);
            Vec3 avgColor = (base0 + base1 + base2) * (1.0f / 3.0f);
            Vec3 cFace    = renderPhong(posFace, nFace, avgColor);

            col0 = col1 = col2 = cFace;
            n0f  = n1f  = n2f  = nFace;
        } else if (shadingMode == ShadingMode::GOURAUD) {
            col0 = renderPhong(w0, n0, base0);
            col1 = renderPhong(w1, n1, base1);
            col2 = renderPhong(w2, n2, base2);
            n0f = n0; n1f = n1; n2f = n2;
        } else { // PHONG
            col0 = base0; col1 = base1; col2 = base2;
            n0f = n0; n1f = n1; n2f = n2;
        }

        // Converte NDC em coordenadas de tela (pixels)
        float sx0, sy0, sx1, sy1, sx2, sy2;
        ndcToScreen(nd0, sx0, sy0);
        ndcToScreen(nd1, sx1, sy1);
        ndcToScreen(nd2, sx2, sy2);
        float z0 = nd0.z;
        float z1 = nd1.z;
        float z2 = nd2.z;

        // Monta RasterVertex para rasterização
        RasterVertex rv0, rv1, rv2;
        rv0.x = sx0; rv0.y = sy0; rv0.z = z0;
        rv0.color = col0; rv0.normal = n0f; rv0.posWorld = w0;

        rv1.x = sx1; rv1.y = sy1; rv1.z = z1;
        rv1.color = col1; rv1.normal = n1f; rv1.posWorld = w1;

        rv2.x = sx2; rv2.y = sy2; rv2.z = z2;
        rv2.color = col2; rv2.normal = n2f; rv2.posWorld = w2;

        // Chama rasterizacao
        rasterizeTriangle(rv0, rv1, rv2);
    }
}

// =============================
// Conversao para DIB e desenho (GDI)
// =============================

void Present(HDC hdc) {
    int nPixels = WIDTH * HEIGHT;
    for (int i = 0; i < nPixels; ++i) {
        // Le cores do framebuffer em [0,1]
        float r = framebuffer[i*3 + 0];
        float g = framebuffer[i*3 + 1];
        float b = framebuffer[i*3 + 2];

        // Converte para 0..255 (8 bits por canal)
        unsigned char R = (unsigned char)(std::max(0.0f, std::min(1.0f, r)) * 255.0f);
        unsigned char G = (unsigned char)(std::max(0.0f, std::min(1.0f, g)) * 255.0f);
        unsigned char B = (unsigned char)(std::max(0.0f, std::min(1.0f, b)) * 255.0f);

        // Escreve no buffer BGRA usado pelo StretchDIBits
        int idx = i * 4;
        g_dibData[idx + 0] = B;
        g_dibData[idx + 1] = G;
        g_dibData[idx + 2] = R;
        g_dibData[idx + 3] = 255; // alpha fixo
    }

    // Desenha o bitmap na área da janela
    StretchDIBits(
            hdc,
            0, 0, WIDTH, HEIGHT,      // destino na tela
            0, 0, WIDTH, HEIGHT,      // origem no bitmap
            g_dibData.data(),         // dados BGRA
            &g_bmi,                   // info do bitmap
            DIB_RGB_COLORS,
            SRCCOPY
    );
}
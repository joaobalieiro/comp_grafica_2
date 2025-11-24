#include <vector>
#include <cmath>
#include <algorithm>
#include <windows.h> 
#include "solidos.h"

// =============================
// Objetos (Mesh Generation)
// =============================

void buildCube(std::vector<Vertex>& vertices, std::vector<Triangle>& faces) {
    vertices.clear();
    faces.clear();
    Vec3 c(0.8f, 0.2f, 0.2f);

    std::vector<Vec3> pos = {
             Vec3(-1,-1, 1), // 0
             Vec3( 1,-1, 1), // 1
             Vec3( 1, 1, 1), // 2
             Vec3(-1, 1, 1), // 3
             Vec3(-1,-1,-1), // 4
             Vec3( 1,-1,-1), // 5
             Vec3( 1, 1,-1), // 6
             Vec3(-1, 1,-1)  // 7
    };
    std::vector<Triangle> tris = {
             {0,1,2},{0,2,3}, // frente
             {5,4,7},{5,7,6}, // tras
             {4,0,3},{4,3,7}, // esquerda
             {1,5,6},{1,6,2}, // direita
             {3,2,6},{3,6,7}, // topo
             {4,5,1},{4,1,0}  // base
    };

    // Calcula normais por vertice
    std::vector<Vec3> normals(pos.size(), Vec3(0,0,0));
    for (const auto& t : tris) {
        Vec3 p0 = pos[t.i0];
        Vec3 p1 = pos[t.i1];
        Vec3 p2 = pos[t.i2];
        Vec3 n = normalize(cross(p1 - p0, p2 - p0));
        normals[t.i0] = normals[t.i0] + n;
        normals[t.i1] = normals[t.i1] + n;
        normals[t.i2] = normals[t.i2] + n;
    }
    for (auto& n : normals) n = normalize(n);

    // Monta vetor de Vertex
    vertices.resize(pos.size());
    for (size_t i=0;i<pos.size();++i) {
        vertices[i].pos    = pos[i];
        vertices[i].normal = normals[i];
        vertices[i].color  = c;
    }
    faces = tris;
}

void buildTrianglePrism(std::vector<Vertex>& vertices, std::vector<Triangle>& faces) {
    vertices.clear();
    faces.clear();
    Vec3 c(0.2f, 0.8f, 0.2f);

    std::vector<Vec3> pos = {
             Vec3(-1,-1,-1), // v0
             Vec3( 1,-1,-1), // v1
             Vec3( 0,-1, 1), // v2
             Vec3(-1, 1,-1), // v3
             Vec3( 1, 1,-1), // v4
             Vec3( 0, 1, 1)  // v5
    };
    std::vector<Triangle> tris = {
             {0,1,2},      // base inferior
             {3,5,4},      // base superior (ordem invertida para face winding)
             {0,3,1},{1,3,4}, // lado 1
             {1,4,2},{2,4,5}, // lado 2
             {2,5,0},{0,5,3}  // lado 3
    };

    // Calcula normais por vertice
    std::vector<Vec3> normals(pos.size(), Vec3(0,0,0));
    for (const auto& t : tris) {
        Vec3 p0 = pos[t.i0];
        Vec3 p1 = pos[t.i1];
        Vec3 p2 = pos[t.i2];
        Vec3 n = normalize(cross(p1 - p0, p2 - p0));
        normals[t.i0] = normals[t.i0] + n;
        normals[t.i1] = normals[t.i1] + n;
        normals[t.i2] = normals[t.i2] + n;
    }
    for (auto& n : normals) n = normalize(n);

    vertices.resize(pos.size());
    for (size_t i=0;i<pos.size();++i) {
        vertices[i].pos    = pos[i];
        vertices[i].normal = normals[i];
        vertices[i].color  = c;
    }
    faces = tris;
}

void buildCylinder(std::vector<Vertex>& vertices,
                   std::vector<Triangle>& faces,
                   int segments) {
    vertices.clear();
    faces.clear();
    Vec3 c(0.2f, 0.4f, 0.8f);
    float h = 2.0f;
    float r = 1.0f;
    float PI = 3.1415926535f;

    std::vector<Vec3> pos;
    std::vector<Triangle> tris;

    // Centros das bases
    int centerBottom = (int)pos.size();
    pos.push_back(Vec3(0,-h/2,0));
    int centerTop    = (int)pos.size();
    pos.push_back(Vec3(0, h/2,0));

    std::vector<int> bottomIdx;
    std::vector<int> topIdx;

    // Gera circulos inferior e superior
    for (int i=0;i<segments;++i) {
        float ang = 2.0f * PI * float(i) / float(segments);
        float x = r * std::cos(ang);
        float z = r * std::sin(ang);
        int ib = (int)pos.size();
        pos.push_back(Vec3(x,-h/2,z));
        bottomIdx.push_back(ib);
        int it = (int)pos.size();
        pos.push_back(Vec3(x, h/2,z));
        topIdx.push_back(it);
    }

    // Triangulos das bases
    for (int i=0;i<segments;++i) {
        int i0 = bottomIdx[i];
        int i1 = bottomIdx[(i+1)%segments];
        // Base inferior (normal -Y)
        tris.push_back({centerBottom, i1, i0});

        int j0 = topIdx[i];
        int j1 = topIdx[(i+1)%segments];
        // Base superior (normal +Y)
        tris.push_back({centerTop, j0, j1});
    }

    // Triangulos da lateral
    for (int i=0;i<segments;++i) {
        int ib0 = bottomIdx[i];
        int ib1 = bottomIdx[(i+1)%segments];
        int it0 = topIdx[i];
        int it1 = topIdx[(i+1)%segments];
        // Quads laterais divididos em 2 triangulos
        tris.push_back({ib0, ib1, it1});
        tris.push_back({ib0, it1, it0});
    }

    // Calcula normais por vertice a partir das faces
    std::vector<Vec3> normals(pos.size(), Vec3(0,0,0));
    for (const auto& t : tris) {
        Vec3 p0 = pos[t.i0];
        Vec3 p1 = pos[t.i1];
        Vec3 p2 = pos[t.i2];
        Vec3 n = normalize(cross(p1 - p0, p2 - p0));
        normals[t.i0] = normals[t.i0] + n;
        normals[t.i1] = normals[t.i1] + n;
        normals[t.i2] = normals[t.i2] + n;
    }
    for (auto& n : normals) n = normalize(n);

    vertices.resize(pos.size());
    for (size_t i=0;i<pos.size();++i) {
        vertices[i].pos    = pos[i];
        vertices[i].normal = normals[i];
        vertices[i].color  = c;
    }
    faces = tris;
}

void buildSphere(std::vector<Vertex>& vertices,
                 std::vector<Triangle>& faces,
                 int stacks,
                 int slices) {
    vertices.clear();
    faces.clear();
    Vec3 c(0.8f, 0.8f, 0.2f);
    float PI = 3.1415926535f;

    std::vector<Vec3> pos;
    // Gera pontos em latitude (phi) e longitude (theta)
    for (int i=0; i<=stacks; ++i) {
        float phi = PI * float(i) / float(stacks);
        float y = std::cos(phi);
        float rr = std::sin(phi);
        for (int j=0;j<slices;++j) {
            float theta = 2.0f * PI * float(j) / float(slices);
            float x = rr * std::cos(theta);
            float z = rr * std::sin(theta);
            pos.push_back(Vec3(x,y,z));
        }
    }

    // Funcao auxiliar para mapear (i,j) em indice
    auto idx = [slices](int i, int j) {
        return i*slices + (j % slices);
    };

    std::vector<Triangle> tris;
    // Conecta vizinhos em i,j
    for (int i=0;i<stacks;++i) {
        for (int j=0;j<slices;++j) {
            int i0 = idx(i, j);
            int i1 = idx(i+1, j);
            int i2 = idx(i+1, j+1);
            int i3 = idx(i, j+1);
            tris.push_back({i0, i1, i2});
            tris.push_back({i0, i2, i3});
        }
    }

    // Normais da esfera: vetor pos normalizado
    std::vector<Vec3> normals(pos.size());
    for (size_t i=0;i<pos.size();++i) {
        normals[i] = normalize(pos[i]);
    }

    vertices.resize(pos.size());
    for (size_t i=0;i<pos.size();++i) {
        vertices[i].pos    = pos[i];
        vertices[i].normal = normals[i];
        vertices[i].color  = c;
    }
    faces = tris;
}

// Escolhe qual objeto 3D montar de acordo com currentObject.
void getCurrentMesh(std::vector<Vertex>& vertices, std::vector<Triangle>& faces) {
    switch (currentObject) {
        case ObjectType::CUBE:
            buildCube(vertices, faces);
            break;
        case ObjectType::TRIANGLE_PRISM:
            buildTrianglePrism(vertices, faces);
            break;
        case ObjectType::CYLINDER:
            buildCylinder(vertices, faces);
            break;
        case ObjectType::SPHERE:
            buildSphere(vertices, faces);
            break;
    }
}
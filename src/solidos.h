#ifndef SOLIDOS_H
#define SOLIDOS_H

#include <vector>
#include "math3D.h"   // já contém Vertex e Triangle

enum class ObjectType {
    CUBE,
    TRIANGLE_PRISM,
    CYLINDER,
    SPHERE
};

extern ObjectType currentObject;

void buildCube(std::vector<Vertex>& vertices, std::vector<Triangle>& faces);
void buildTrianglePrism(std::vector<Vertex>& vertices, std::vector<Triangle>& faces);
void buildCylinder(std::vector<Vertex>& vertices, std::vector<Triangle>& faces, int segments = 24);
void buildSphere(std::vector<Vertex>& vertices, std::vector<Triangle>& faces, int stacks = 12, int slices = 24);
void getCurrentMesh(std::vector<Vertex>& vertices, std::vector<Triangle>& faces);

#endif

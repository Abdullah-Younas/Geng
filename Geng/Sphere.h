#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <cmath>

class Sphere
{
public:
    Sphere(float radius = 1.0f, int sectors = 36, int stacks = 18, bool smooth = true);
    ~Sphere();

    // Getters
    const float* getInterleavedVertices() const { return interleavedVertices.data(); }
    unsigned int getInterleavedVertexSize() const { return (unsigned int)interleavedVertices.size() * sizeof(float); }
    int getInterleavedStride() const { return interleavedStride; }
    const unsigned int* getIndices() const { return indices.data(); }
    unsigned int getIndexSize() const { return (unsigned int)indices.size() * sizeof(unsigned int); }
    unsigned int getIndexCount() const { return (unsigned int)indices.size(); }

private:
    void buildVertices();
    void buildInterleavedVertices();
    void clearArrays();

    float radius;
    int sectorCount;
    int stackCount;
    bool smooth;
    int interleavedStride;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;
    std::vector<float> interleavedVertices;
};

#endif
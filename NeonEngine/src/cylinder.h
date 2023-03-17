///////////////////////////////////////////////////////////////////////////////
// Cylinder.h
// ==========
// Cylinder for OpenGL with (base radius, top radius, height, sectors, stacks)
// The min number of sectors (slices) is 3 and the min number of stacks are 1.
// - base radius: the radius of the cylinder at z = -height/2
// - top radius : the radiusof the cylinder at z = height/2
// - height     : the height of the cylinder along z-axis
// - sectors    : the number of slices of the base and top caps
// - stacks     : the number of subdivisions along z-axis
// - smooth     : surface shading smooth(true, default) or flat(false)
// - up-axis    : top surface facing: X=1, Y=2, Z=3(default)
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2018-03-27
// UPDATED: 2023-03-12
///////////////////////////////////////////////////////////////////////////////

// Modified by Alonso Cerpa to be compatible with Modern OpenGL 3.3

#pragma once

#include "model.h"

#include <vector>
#include <glad/glad.h>

class Shader;

class Cylinder : public Model
{
public:
    // ctor/dtor
    Cylinder(float baseRadius=1.0f, float topRadius=1.0f, float height=1.0f,
             int sectorCount=36, int stackCount=1, bool smooth=true, int up=3);
    ~Cylinder() {}

    // getters/setters
    float getBaseRadius() const             { return baseRadius; }
    float getTopRadius() const              { return topRadius; }
    float getHeight() const                 { return height; }
    int getSectorCount() const              { return sectorCount; }
    int getStackCount() const               { return stackCount; }
    void set(float baseRadius, float topRadius, float height,
             int sectorCount, int stackCount, bool smooth=true, int up=3);
    void setBaseRadius(float radius);
    void setTopRadius(float radius);
    void setHeight(float radius);
    void setSectorCount(int sectorCount);
    void setStackCount(int stackCount);
    void setSmooth(bool smooth);
    void setUpAxis(int up);

    // for vertex data
    unsigned int getVertexCount() const     { return (unsigned int)vertices.size() / 3; }
    unsigned int getNormalCount() const     { return (unsigned int)normals.size() / 3; }
    unsigned int getTexCoordCount() const   { return (unsigned int)texCoords.size() / 2; }
    unsigned int getIndexCount() const      { return (unsigned int)indices.size(); }
    unsigned int getLineIndexCount() const  { return (unsigned int)lineIndices.size(); }
    unsigned int getTriangleCount() const   { return getIndexCount() / 3; }
    unsigned int getVertexSize() const      { return (unsigned int)vertices.size() * sizeof(float); }
    unsigned int getNormalSize() const      { return (unsigned int)normals.size() * sizeof(float); }
    unsigned int getTexCoordSize() const    { return (unsigned int)texCoords.size() * sizeof(float); }
    unsigned int getIndexSize() const       { return (unsigned int)indices.size() * sizeof(unsigned int); }
    unsigned int getLineIndexSize() const   { return (unsigned int)lineIndices.size() * sizeof(unsigned int); }
    const float* getVertices() const        { return vertices.data(); }
    const float* getNormals() const         { return normals.data(); }
    const float* getTexCoords() const       { return texCoords.data(); }
    const unsigned int* getIndices() const  { return indices.data(); }
    const unsigned int* getLineIndices() const  { return lineIndices.data(); }

    // for interleaved vertices: V/N/T
    unsigned int getInterleavedVertexCount() const  { return getVertexCount(); }    // # of vertices
    unsigned int getInterleavedVertexSize() const   { return (unsigned int)interleavedVertices.size() * sizeof(unsigned int); }    // # of bytes
    int getInterleavedStride() const                { return interleavedStride; }   // should be 32 bytes
    const float* getInterleavedVertices() const     { return &interleavedVertices[0]; }

    // for indices of base/top/side parts
    unsigned int getBaseIndexCount() const  { return ((unsigned int)indices.size() - baseIndex) / 2; }
    unsigned int getTopIndexCount() const   { return ((unsigned int)indices.size() - baseIndex) / 2; }
    unsigned int getSideIndexCount() const  { return baseIndex; }
    unsigned int getBaseStartIndex() const  { return baseIndex; }
    unsigned int getTopStartIndex() const   { return topIndex; }
    unsigned int getSideStartIndex() const  { return 0; }   // side starts from the begining

    // draw in VertexArray mode
    void draw(Shader& shader, bool is_selected);          // draw all
    void drawBase() const;      // draw base cap only
    void drawTop() const;       // draw top cap only
    void drawSide() const;      // draw side only
    void drawLines(const float lineColor[4]) const;     // draw lines only
    void drawWithLines(const float lineColor[4]) const; // draw surface and lines

    // test intersection with ray
    bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t);

    // debug
    void printSelf() const;

protected:

private:
    // member functions
    void clearArrays();
    void buildVerticesSmooth();
    void buildVerticesFlat();
    void buildInterleavedVertices();
    void buildUnitCircleVertices();
    void changeUpAxis(int from, int to);
    void addVertex(float x, float y, float z);
    void addNormal(float x, float y, float z);
    void addTexCoord(float s, float t);
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    std::vector<float> getSideNormals();
    std::vector<float> computeFaceNormal(float x1, float y1, float z1,
                                         float x2, float y2, float z2,
                                         float x3, float y3, float z3);

    // memeber vars
    GLuint vao;
    float baseRadius;
    float topRadius;
    float height;
    int sectorCount;                        // # of slices
    int stackCount;                         // # of stacks
    unsigned int baseIndex;                 // starting index of base
    unsigned int topIndex;                  // starting index of top
    bool smooth;                            // smooth(true) or flat(false)
    int upAxis;                             // X=1, Y=2, Z=3(default)

    std::vector<float> unitCircleVertices;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> lineIndices;

    // interleaved
    std::vector<float> interleavedVertices;
    int interleavedStride;                  // # of bytes to hop to the next vertex (should be 32 bytes)

};

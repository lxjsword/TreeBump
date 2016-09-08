#ifndef TREE_H
#define TREE_H


// 树顶点数据结构
class TreeVertex
{
public:
    VECTOR3D position;
    float s, t;
    VECTOR3D sTangent, tTangent;
    VECTOR3D normal;
    VECTOR3D tangentSpaceLight;
};

// 树
class Tree
{
public:
    Tree();
    ~Tree();

    bool InitTree();

    int numVertices;
    int numIndices;

    unsigned int* indices;
    TreeVertex* vertices;
};

const int treePrecision = 48;
static const float r = 2.0f;
static const float h = 12.56f;

#endif 
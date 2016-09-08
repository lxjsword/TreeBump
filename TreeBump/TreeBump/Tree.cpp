#include <Windows.h>
#include <stdio.h>
#include <gl\gl.h>
#include "Maths/Maths.h"
#include "Tree.h"

Tree::Tree()
{
    InitTree();
}

Tree::~Tree()
{
    if (indices)
        delete[] indices;
    indices = NULL;

    if (vertices)
        delete[] vertices;
    vertices = NULL;
}

bool Tree::InitTree()
{
    numVertices = (treePrecision + 1) * (treePrecision + 1);
    numIndices = 2 * treePrecision * treePrecision * 3;

    vertices = new TreeVertex[numVertices];
    if (!vertices)
    {
        printf("Unable to allocate memory for torus vertices\n");
        return false;
    }

    indices = new unsigned int[numIndices];
    if (!indices)
    {
        printf("Unable to allocate memory for torus indices\n");
        return false;
    }
    
    // 计算第一个环上的点坐标
    for (int i = 0; i < treePrecision + 1; ++i)
    {
        vertices[i].position = VECTOR3D(r, (float)(-h/2), 0.0f).GetRotatedY(i*360.0f / treePrecision);

        vertices[i].s = (float)i / treePrecision;
        vertices[i].t = 0.0f;
        //printf("%f, %f\n", vertices[i].s, vertices[i].t);
        

        vertices[i].sTangent = VECTOR3D(0.0f, 0.0f, -1.0f).GetRotatedY(i*360.0f / treePrecision);
        vertices[i].tTangent.Set(0.0f, -1.0f, 0.0f);
        vertices[i].normal = vertices[i].tTangent.
            CrossProduct(vertices[i].sTangent);
    }
    //printf("\n");

    // 计算其他环上的坐标
    for (int ring = 1; ring < treePrecision + 1; ++ring)
    {
        for (int i = 0; i < treePrecision + 1; ++i)
        {
            VECTOR3D v = vertices[i].position;
            vertices[ring*(treePrecision + 1) + i].position.Set(v.GetX(), v.GetY() + ring*h/treePrecision, v.GetZ());

            vertices[ring*(treePrecision + 1) + i].s = vertices[i].s;
            vertices[ring*(treePrecision + 1) + i].t = vertices[i].t + (float)ring / treePrecision;
            //printf("%f, %f\n", vertices[ring*(treePrecision + 1) + i].s, vertices[ring*(treePrecision + 1) + i].t);
            vertices[ring*(treePrecision + 1) + i].sTangent = vertices[i].sTangent;
            vertices[ring*(treePrecision + 1) + i].tTangent = vertices[i].tTangent;
            vertices[ring*(treePrecision + 1) + i].normal = vertices[i].normal;
        }
    }

    // 计算indices
    for (int ring = 0; ring<treePrecision; ring++)
    {
        for (int i = 0; i<treePrecision; i++)
        {
            indices[((ring*treePrecision + i) * 2) * 3 + 0] = ring*(treePrecision + 1) + i;
            indices[((ring*treePrecision + i) * 2) * 3 + 1] = (ring + 1)*(treePrecision + 1) + i;
            indices[((ring*treePrecision + i) * 2) * 3 + 2] = ring*(treePrecision + 1) + i + 1;
            indices[((ring*treePrecision + i) * 2 + 1) * 3 + 0] = ring*(treePrecision + 1) + i + 1;
            indices[((ring*treePrecision + i) * 2 + 1) * 3 + 1] = (ring + 1)*(treePrecision + 1) + i;
            indices[((ring*treePrecision + i) * 2 + 1) * 3 + 2] = (ring + 1)*(treePrecision + 1) + i + 1;
        }
    }
}
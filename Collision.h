#pragma once

#include "types.h"
#include "3DMaths.h"
#include "ObjLoading.h"

struct Plane
{
    vec3 pos;
    vec3 normal;
};

struct ColliderData
{
    u32 numVertices;
    vec3* vertices;
    u32 numPlanes;
    Plane* planes;

    mat4 modelMatrix;
    mat4 normalMatrix;
};

ColliderData createColliderData(const LoadedObj &obj)
{
    ColliderData result = {};

    result.numVertices = obj.numVertices;
    result.vertices = (vec3*)malloc(result.numVertices * sizeof(vec3));

    vec3* currentVertex = result.vertices;
    for(u32 i=0; i<obj.numVertices; ++i) {
        *currentVertex++ = obj.vertexBuffer[i].pos;
    }

    result.numPlanes = obj.numIndices / 3;
    result.planes = (Plane*)malloc(result.numPlanes * sizeof(Plane));

    Plane* currentPlane = result.planes;
    for(u32 i=0; i<obj.numIndices; i+=3) {
        vec3 a = obj.vertexBuffer[obj.indexBuffer[i]].pos;
        vec3 b = obj.vertexBuffer[obj.indexBuffer[i+1]].pos;
        vec3 c = obj.vertexBuffer[obj.indexBuffer[i+2]].pos;

        currentPlane->pos = a;
        currentPlane->normal = cross(b-a, c-a);
        ++currentPlane;
    }

    return result;
}

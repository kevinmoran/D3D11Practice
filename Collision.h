#pragma once

#include "types.h"
#include "3DMaths.h"
#include "ObjLoading.h"

struct Plane
{
    vec4 point;
    vec3 normal;
};

struct ColliderData
{
    u32 numVertices;
    vec4* vertices;
    u32 numPlanes;
    Plane* planes;

    mat4 modelMatrix;
    mat3 normalMatrix;
};

ColliderData createColliderData(const LoadedObj &obj)
{
    ColliderData result = {};

    result.numVertices = obj.numVertices;
    result.vertices = (vec4*)malloc(result.numVertices * sizeof(vec4));

    vec4* destVertex = result.vertices;
    for(u32 i=0; i<obj.numVertices; ++i) {
        *destVertex++ = {
            obj.vertexBuffer[i].pos.x,
            obj.vertexBuffer[i].pos.y,
            obj.vertexBuffer[i].pos.z, 
            1.f};
    }

    result.numPlanes = obj.numIndices / 3;
    result.planes = (Plane*)malloc(result.numPlanes * sizeof(Plane));

    Plane* destPlane = result.planes;
    for(u32 i=0; i<obj.numIndices; i+=3) {
        vec3 a = obj.vertexBuffer[obj.indexBuffer[i]].pos;
        vec3 b = obj.vertexBuffer[obj.indexBuffer[i+1]].pos;
        vec3 c = obj.vertexBuffer[obj.indexBuffer[i+2]].pos;
        vec3 n = cross(b-a, c-a);

        destPlane->point = {a.x, a.y, a.z, 1.f};
        destPlane->normal = n;
        ++destPlane;
    }

    return result;
}

bool hasSeparatingAxis(const ColliderData &a, const ColliderData &b)
{
    for(u32 i=0; i<b.numPlanes; ++i)
    {
        Plane plane = {
            b.planes[i].point * b.modelMatrix,
            normalise(b.planes[i].normal * b.normalMatrix)
        };

        bool allVerticesAreInFrontOfPlane = true;
        for(u32 j=0; j<a.numVertices; ++j)
        {
            vec4 vertex = a.vertices[j] * a.modelMatrix;
            vec3 planeToVertexVec = vertex.xyz - plane.point.xyz;
            float distanceToVertex = dot(planeToVertexVec, plane.normal);
            if(distanceToVertex <= 0)
            {
                allVerticesAreInFrontOfPlane = false;
                break;
            }
        }
        if(allVerticesAreInFrontOfPlane)
            return true;
    }

    return false;
}

bool isColliding(const ColliderData &a, const ColliderData &b)
{
    if(hasSeparatingAxis(a, b))
        return false;
    if(hasSeparatingAxis(b, a))
        return false;

    return true;
}
#pragma once

#include "types.h"
#include "3DMaths.h"
#include "ObjLoading.h"

struct Plane
{
    vec4 point;
    vec3 normal;
};

struct ColliderPolyhedron
{
    u32 numVertices;
    vec4* vertices;
    u32 numPlanes;
    Plane* planes;

    mat4 modelMatrix;
    mat3 normalMatrix;
};

ColliderPolyhedron createColliderPolyhedron(const LoadedObj &obj)
{
    ColliderPolyhedron result = {};

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

struct SATResult
{
    bool isColliding;
    float penetrationDistance;
    vec3 normal;
};

// Check if any of the plane normals of `b` are a separating axis for the vertices of `a`
SATResult separatingAxisTest(const ColliderPolyhedron &a, const ColliderPolyhedron &b)
{
    SATResult result = {
        true, 1E+37, {}
    };
    for(u32 i=0; i<b.numPlanes; ++i)
    {
        Plane plane = {
            b.planes[i].point * b.modelMatrix,
            normalise(b.planes[i].normal * b.normalMatrix)
        };

        // Find how far the vertices of `a` are behind the current plane
        float currentPenetrationDistance = -1E+37;
        for(u32 j=0; j<a.numVertices; ++j)
        {
            vec4 vertex = a.vertices[j] * a.modelMatrix;
            vec3 vertexToPlaneVec = plane.point.xyz - vertex.xyz;
            float dist = dot(vertexToPlaneVec, plane.normal);
            if(dist > currentPenetrationDistance) {
                currentPenetrationDistance = dist;
            }
        }

        // If all of a's vertices are in front of current plane,
        // we have found a separating axis and there is no collision
        if(currentPenetrationDistance < 0) {
            result.isColliding = false;
            break;
        }
        // Keep track of which plane gives the smallest penetration 
        // so we can resolve the collision
        else if(currentPenetrationDistance < result.penetrationDistance) {
            result.penetrationDistance = currentPenetrationDistance;
            result.normal = plane.normal;
        }
    }
    return result;
}

SATResult checkCollision(const ColliderPolyhedron &a, const ColliderPolyhedron &b)
{
    SATResult resultA = separatingAxisTest(a, b);
    if(!resultA.isColliding)
        return resultA;
    SATResult resultB = separatingAxisTest(b, a);
    if(!resultB.isColliding)
        return resultB;

    if(resultA.penetrationDistance < resultB.penetrationDistance)
        return resultA;
    else
        return resultB;
}
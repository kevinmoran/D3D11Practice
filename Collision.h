#pragma once

#include "types.h"
#include "3DMaths.h"

struct Plane
{
    vec4 point;
    vec3 normal;
};

struct Edge
{
    vec3 p0;
    vec3 p1;
};

struct ColliderPolyhedron
{
    u32 numVertices;
    vec4* vertices;
    u32 numPlanes;
    Plane* planes;
    u32 numEdges;
    Edge* edges;

    mat4 modelMatrix;
    mat3 normalMatrix;
};

struct ColliderSphere
{
    vec3 pos;
    float radius;
};

struct LoadedObj;
ColliderPolyhedron createColliderPolyhedron(const LoadedObj &obj);

struct SATResult
{
    bool isColliding;
    float penetrationDistance;
    vec3 normal;
};

SATResult checkCollision(const ColliderPolyhedron &a, const ColliderPolyhedron &b);
SATResult checkCollision(const ColliderPolyhedron &a, const ColliderSphere &b);

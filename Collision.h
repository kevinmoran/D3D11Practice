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
    vec3 centroid;

    mat4 modelMatrix;
    mat3 normalMatrix;
};

struct ColliderSphere
{
    vec3 pos;
    float radius;
};

struct ColliderCylinder
{
    vec3 base;
    vec3 upDir; // normalised
    float height;
    float radius;
    inline vec3 top() const { return base + upDir*height; }
};

struct LoadedObj;
ColliderPolyhedron createColliderPolyhedron(const LoadedObj &obj);

struct CollisionResult
{
    bool isColliding;
    float penetrationDistance;
    vec3 normal;
};

CollisionResult checkCollision(const ColliderPolyhedron &polyA, const ColliderPolyhedron &polyB);
CollisionResult checkCollision(const ColliderPolyhedron &poly, const ColliderSphere &sphere);
CollisionResult checkCollision(const ColliderCylinder &cylinder, const ColliderPolyhedron &poly);

#include "Collision.h"

#include <stdlib.h> // malloc

#include "ObjLoading.h"

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

// Check if any of the plane normals of `b` are a separating axis for the vertices of `a`
static SATResult separatingAxisTest(const ColliderPolyhedron &a, const ColliderPolyhedron &b)
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
            float dist = dot(plane.point.xyz - vertex.xyz, plane.normal);
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
        if(currentPenetrationDistance < result.penetrationDistance) {
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

SATResult checkCollision(const ColliderPolyhedron &poly, const ColliderSphere &sphere)
{
    SATResult result = {
        true, 1E+37, {}
    };
    for(u32 i=0; i<poly.numPlanes; ++i)
    {
        Plane plane = {
            poly.planes[i].point * poly.modelMatrix,
            normalise(poly.planes[i].normal * poly.normalMatrix)
        };
        vec3 furthestPointOnSphere = sphere.pos - plane.normal*sphere.radius;
        float currentPenetrationDistance = dot(plane.point.xyz - furthestPointOnSphere, plane.normal);
        if(currentPenetrationDistance < 0) {
            result.isColliding = false;
            return result;
        }
        
        // Keep track of which plane gives the smallest penetration 
        // so we can resolve the collision
        if(currentPenetrationDistance < result.penetrationDistance) {
            result.penetrationDistance = currentPenetrationDistance;
            result.normal = plane.normal;
        }
    }
    // TODO: There are still some false positives with this test
    // Try finding the closest edge to the sphere and use that vector
    // as a separating axis
    return result;
}

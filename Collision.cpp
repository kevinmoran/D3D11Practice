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
        *destVertex++ = v4(obj.vertexBuffer[i].pos, 1.f);
    }

    // Note: We will have redundant planes here since we're adding one for each triangle rather than each face
    // This will be fixed when we actually generate collision data offline
    result.numPlanes = obj.numIndices / 3;
    result.planes = (Plane*)malloc(result.numPlanes * sizeof(Plane));

    Plane* destPlane = result.planes;
    for(u32 i=0; i<obj.numIndices; i+=3) {
        vec3 a = obj.vertexBuffer[obj.indexBuffer[i]].pos;
        vec3 b = obj.vertexBuffer[obj.indexBuffer[i+1]].pos;
        vec3 c = obj.vertexBuffer[obj.indexBuffer[i+2]].pos;
        vec3 n = normalise(cross(b-a, c-a));

        destPlane->point = v4(a, 1.f);
        destPlane->normal = n;
        ++destPlane;
    }

    // NOTE: We will have duplicate edges here. Just doing the simplest thing for now.
    // This will be fixed when we actually generate collision data offline
    result.numEdges = obj.numIndices;
    result.edges = (Edge*)malloc(result.numEdges * sizeof(Edge));

    Edge* destEdge = result.edges;
    for(u32 i=0; i<obj.numIndices; i+=3) {
        vec3 a = obj.vertexBuffer[obj.indexBuffer[i]].pos;
        vec3 b = obj.vertexBuffer[obj.indexBuffer[i+1]].pos;
        vec3 c = obj.vertexBuffer[obj.indexBuffer[i+2]].pos;
        
        *destEdge++ = {a, b};
        *destEdge++ = {b, c};
        *destEdge++ = {c, a};
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

// Returns closest point to p that lies on line segment ab.
// From Real-Time Collision Detection
vec3 findClosestPointOnLineSegment(vec3 p, vec3 a, vec3 b)
{
    vec3 ab = b-a;
    float t = dot(p-a, ab) / dot(ab,ab);
    t = CLAMP_BETWEEN(t, 0, 1);
    return a + ab*t;
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
            result.normal = -plane.normal;
        }
    }

    // Did not find separating axis using polyhedron's faces. Find closest edge and test 
    // the vector from that to the sphere center as a possible separating axis
    float minDistanceSquaredToEdge = 1E37;
    vec3 overallClosestEdgePoint = {};
    { // Find closest edge point on polyhedron to sphere center
        for(u32 i=0; i<poly.numEdges; ++i)
        {
            Edge edge = {
                (v4(poly.edges[i].p0, 1) * poly.modelMatrix).xyz,
                (v4(poly.edges[i].p1, 1) * poly.modelMatrix).xyz
            };
            
            vec3 closestPointOnEdge = findClosestPointOnLineSegment(sphere.pos, edge.p0, edge.p1);
            float distSquared = lengthSquared(closestPointOnEdge - sphere.pos);

            if(distSquared < minDistanceSquaredToEdge) {
                minDistanceSquaredToEdge = distSquared;
                overallClosestEdgePoint = closestPointOnEdge;
            }
        }
    }

    // Find how far behind closest edge sphere is
    vec3 closestEdgeToSphereDir = (sphere.pos - overallClosestEdgePoint) / sqrtf(minDistanceSquaredToEdge);
    vec3 furthestPointOnSphere = sphere.pos - closestEdgeToSphereDir * sphere.radius ;

    float penetrationDistance = dot(overallClosestEdgePoint - furthestPointOnSphere, closestEdgeToSphereDir);
    if(penetrationDistance < 0) { // Sphere is in front of closest edge;
        // We have found a separating axis
        result.isColliding = false;
    }
    // Note: Don't think we need to check this edge penetration distance against the minimum penetration found
    // using the polyhedron's face normals as separating axes, because of Pythagoras' theorem?

    return result;
}

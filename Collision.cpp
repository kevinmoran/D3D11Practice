#include "Collision.h"

#include <assert.h>
#include <stdlib.h> // malloc

#include "ObjLoading.h"

ColliderPolyhedron createColliderPolyhedron(const LoadedObj &obj)
{
    ColliderPolyhedron result = {};

    result.numVertices = obj.numVertices;
    result.vertices = (vec4*)malloc(result.numVertices * sizeof(vec4));

    vec4* destVertex = result.vertices;
    vec3 sumOfVertices = {};
    for(u32 i=0; i<obj.numVertices; ++i) {
        *destVertex++ = v4(obj.vertexBuffer[i].pos, 1.f);
        sumOfVertices += obj.vertexBuffer[i].pos;
    }
    result.centroid = sumOfVertices / (float)obj.numVertices;

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
static CollisionResult separatingAxisTest(const ColliderPolyhedron &a, const ColliderPolyhedron &b)
{
    CollisionResult result = {
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

CollisionResult checkCollision(const ColliderPolyhedron &a, const ColliderPolyhedron &b)
{
    CollisionResult resultA = separatingAxisTest(a, b);
    if(!resultA.isColliding)
        return resultA;
    CollisionResult resultB = separatingAxisTest(b, a);
    if(!resultB.isColliding)
        return resultB;

    if(resultA.penetrationDistance < resultB.penetrationDistance)
        return resultA;
    else
        return resultB;
}

// Returns closest point to p that lies on line segment ab.
// From Real-Time Collision Detection
static vec3 findClosestPointOnLineSegment(vec3 p, vec3 a, vec3 b)
{
    vec3 ab = b-a;
    float t = dot(p-a, ab) / dot(ab,ab);
    t = CLAMP_BETWEEN(t, 0, 1);
    return a + ab*t;
}

CollisionResult checkCollision(const ColliderPolyhedron &poly, const ColliderSphere &sphere)
{
    CollisionResult result = {
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
    // Use the vector from the polyhedron's centroid to the closest edge as its "normal"
    vec3 transformedCentroid = (v4(poly.centroid, 1.0) * poly.modelMatrix).xyz;
    vec3 closestEdgeNormal = normalise(overallClosestEdgePoint - transformedCentroid);
    vec3 furthestPointOnSphere = sphere.pos - closestEdgeNormal * sphere.radius;
    
    float penetrationDistance = dot(overallClosestEdgePoint - furthestPointOnSphere, closestEdgeNormal);
    if(penetrationDistance < 0) { // Sphere is in front of closest edge;
        // We have found a separating axis
        result.isColliding = false;
    }
    // Note: Don't think we need to check this edge penetration distance against the minimum penetration found
    // using the polyhedron's face normals as separating axes, because of Pythagoras' theorem?

    return result;
}

// Find furthest point on cylinder in a given direction
static vec3 getFurthestPointInDir(const ColliderCylinder& cylinder, vec3 dir)
{ 
    // First find which of the cylinder's endpoints is furthest
    vec3 furthestEndpoint = 
        (dot(cylinder.p0, dir) > dot(cylinder.p1, dir)) 
        ? cylinder.p0 : cylinder.p1;

    // Project plane normal onto plane of cylinder end cap
    vec3 cylinderUpDir = normalise(cylinder.p1 - cylinder.p0);
    vec3 projection = dir - (cylinderUpDir * dot(dir, cylinderUpDir));
    return furthestEndpoint + (normaliseOrZero(projection) * cylinder.radius);
}

// Computes closest points c1 and c2
// on line segments {p1,q1} and {p2,q2}, respectively
// From Real-Time Collision Detection
static void findClosestPointsOnLineSegments(vec3 p1, vec3 q1, vec3 p2, vec3 q2, vec3 &c1, vec3 &c2)
{
    vec3 d1 = q1 - p1; // Direction vector of segment S1
    vec3 d2 = q2 - p2; // Direction vector of segment S2
    vec3 r = p1 - p2;
    float a = dot(d1, d1); // Squared length of segment S1, always nonnegative
    float e = dot(d2, d2); // Squared length of segment S2, always nonnegative
    float f = dot(d2, r);

    // s and t are the factors we're looking for that satisfy the equations:
    // c1 = p1 + (q1-p1)*s
    // c2 = p2 + (q2-p2)*t
    float s, t;

    // Simplify this function by assuming that neither line segment is degenerate,
    // i.e. they both have a length greater than zero
    const float EPSILON = 0.000001f;
    assert(a > EPSILON);
    assert(e > EPSILON);
    // Check if either or both segments degenerate into points
    // if (a <= EPSILON) {
    //     // First segment degenerates into a point
    //     if (e <= EPSILON) {
    //         // Both segments degenerate into points
    //         s = t = 0.0f;
    //         c1 = p1;
    //         c2 = p2;
    //     }
    //     else {
    //         s = 0.0f;
    //         t = f / e; // s = 0 => t = (b*s + f) / e = f / e
    //         t = CLAMP_BETWEEN(t, 0.0f, 1.0f);
    //     }
    // } 
    // else 
    {
        float c = dot(d1, r);
        // if (e <= EPSILON) {
        //     // Second segment degenerates into a point
        //     t = 0.0f;
        //     s = CLAMP_BETWEEN(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
        // } 
        // else 
        {
            // The general nondegenerate case starts here
            float b = dot(d1, d2);
            float denom = a*e-b*b; // Always nonnegative

            // If segments not parallel, compute closest point on L1 to L2 and
            // clamp to segment S1. Else pick arbitrary s (here 0)
            if (denom > EPSILON)
                s = CLAMP_BETWEEN((b*f - c*e) / denom, 0.0f, 1.0f);
            else s = 0.0f;

            // Compute point on L2 closest to S1(s) using
            // t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
            float tnom = b*s + f;
            if (tnom < 0.0f) {
                t = 0.0f;
                s = CLAMP_BETWEEN(-c / a, 0.0f, 1.0f);
            } 
            else if (tnom > e) {
                t = 1.0f;
                s = CLAMP_BETWEEN((b - c) / a, 0.0f, 1.0f);
            } 
            else {
                t = tnom / e;
            }
        }
    }
    c1 = p1 + d1 * s;
    c2 = p2 + d2 * t;

    // return dot(c1 - c2, c1 - c2);
}

CollisionResult checkCollision(const ColliderCylinder &cylinder, const ColliderPolyhedron &poly)
{
    CollisionResult result = {
        true, 1E+37, {}
    };

    for(u32 i=0; i<poly.numPlanes; ++i)
    {
        Plane plane = {
            poly.planes[i].point * poly.modelMatrix,
            normalise(poly.planes[i].normal * poly.normalMatrix)
        };
        
        vec3 furthestPointOnCylinder = getFurthestPointInDir(cylinder, -plane.normal);
        

        float currentPenetrationDistance = dot(plane.point.xyz - furthestPointOnCylinder, plane.normal);
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

    // Did not find separating axis using polyhedron's faces. Find closest edge and test 
    // the vector from that to the cylinder's central line segment as a possible separating axis
    float minDistanceSquaredToEdge = 1E37;
    vec3 overallClosestEdgePoint = {};
    { // Find closest edge point on polyhedron to sphere center
        for(u32 i=0; i<poly.numEdges; ++i)
        {
            Edge edge = {
                (v4(poly.edges[i].p0, 1) * poly.modelMatrix).xyz,
                (v4(poly.edges[i].p1, 1) * poly.modelMatrix).xyz
            };
            
            vec3 closestPointOnCylinder, closestPointOnEdge;
            findClosestPointsOnLineSegments(
                cylinder.p0, cylinder.p1, 
                edge.p0, edge.p1, 
                closestPointOnCylinder, closestPointOnEdge
            );
            float distSquared = lengthSquared(closestPointOnCylinder - closestPointOnEdge);

            if(distSquared < minDistanceSquaredToEdge) {
                minDistanceSquaredToEdge = distSquared;
                overallClosestEdgePoint = closestPointOnEdge;
            }
        }
    }

    // Find how far behind closest edge cylinder is
    // Use the vector from the polyhedron's centroid to the closest edge as its "normal"
    vec3 transformedCentroid = (v4(poly.centroid, 1.0) * poly.modelMatrix).xyz;
    vec3 closestEdgeNormal = normalise(overallClosestEdgePoint - transformedCentroid);
    vec3 furthestPointOnCylinder = getFurthestPointInDir(cylinder, -closestEdgeNormal);
    
    float penetrationDistance = dot(overallClosestEdgePoint - furthestPointOnCylinder, closestEdgeNormal);
    if(penetrationDistance < 0) { // Cylinder is in front of closest edge;
        // We have found a separating axis
        result.isColliding = false;
    }
    // Note: Don't think we need to check this edge penetration distance against the minimum penetration found
    // using the polyhedron's face normals as separating axes, because of Pythagoras' theorem?

    return result;
}

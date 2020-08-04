#pragma once

#include <math.h>

#define PI32 3.14159265358979323846f

struct vec3
{
    float x, y, z;
};

struct vec4
{
    float x, y, z, w;
};

union mat4
{
    float m[4][4];
    vec4 cols[4];

    inline vec4 row(int i) { // Returns i-th row of matrix
        return { m[0][i], m[1][i], m[2][i], m[3][i] };
    }
};

inline float degreesToRadians(float degs) {
    return degs * (PI32 / 180.0f);
}

inline float length(vec3 v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline float dot(vec4 a, vec4 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

inline vec3 operator* (vec3 v, float f) {
    return {v.x*f, v.y*f, v.z*f};
}

inline vec3 normalise(vec3 v) {
    return v * (1.f / length(v));
}

inline vec3 cross(vec3 a, vec3 b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

inline vec3 operator+= (vec3 &lhs, vec3 rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

inline vec3 operator-= (vec3 &lhs, vec3 rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}

inline vec3 operator- (vec3 v) {
    return {-v.x, -v.y, -v.z};
}

inline vec3 operator- (vec3 a, vec3 b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}

inline mat4 scaleMat(vec3 scale) {
    return {
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    };
}

inline mat4 rotateXMat(float rad) {
    float sinTheta = sinf(rad);
    float cosTheta = cosf(rad);
    return {
        1, 0, 0, 0,
        0, cosTheta, -sinTheta, 0,
        0, sinTheta, cosTheta, 0,
        0, 0, 0, 1
    };
}

inline mat4 rotateYMat(float rad) {
    float sinTheta = sinf(rad);
    float cosTheta = cosf(rad);
    return {
        cosTheta, 0, sinTheta, 0,
        0, 1, 0, 0,
        -sinTheta, 0, cosTheta, 0,
        0, 0, 0, 1
    };
}

inline mat4 translationMat(vec3 trans) {
    return {
        1, 0, 0, trans.x,
        0, 1, 0, trans.y,
        0, 0, 1, trans.z,
        0, 0, 0, 1
    };
}

inline mat4 makePerspectiveMat(float aspectRatio, float fovYRadians, float zNear, float zFar)
{
    // float yScale = 1 / tanf(0.5f * fovYRadians); 
    // NOTE: 1/tan(X) = tan(90degs - X), so we can avoid a divide
    // float yScale = tanf((0.5f * M_PI) - (0.5f * fovYRadians));
    float yScale = tanf(0.5f * (PI32 - fovYRadians));
    float xScale = yScale / aspectRatio;
    float zRangeInverse = 1.f / (zNear - zFar);
    float zScale = zFar * zRangeInverse;
    float zTranslation = zFar * zNear * zRangeInverse;

    mat4 result = {
        xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, zScale, zTranslation,
        0, 0, -1, 0 
    };
    return result;
}

inline mat4 operator* (mat4 a, mat4 b) {
    return {
        dot(a.row(0), b.cols[0]),
        dot(a.row(1), b.cols[0]),
        dot(a.row(2), b.cols[0]),
        dot(a.row(3), b.cols[0]),
        dot(a.row(0), b.cols[1]),
        dot(a.row(1), b.cols[1]),
        dot(a.row(2), b.cols[1]),
        dot(a.row(3), b.cols[1]),
        dot(a.row(0), b.cols[2]),
        dot(a.row(1), b.cols[2]),
        dot(a.row(2), b.cols[2]),
        dot(a.row(3), b.cols[2]),
        dot(a.row(0), b.cols[3]),
        dot(a.row(1), b.cols[3]),
        dot(a.row(2), b.cols[3]),
        dot(a.row(3), b.cols[3])
    };
}

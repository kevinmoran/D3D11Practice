#pragma once

#include <math.h>

#define PI32 3.14159265358979323846f
#define CLAMP_BELOW(x, upperbound) ((x) < (upperbound) ? (x) : (upperbound))
#define CLAMP_ABOVE(x, lowerbound) ((x) > (lowerbound) ? (x) : (lowerbound))
#define CLAMP_BETWEEN(x,lo,hi) (CLAMP_BELOW(CLAMP_ABOVE((x),(lo)),(hi)))

inline bool areAlmostEqual(float a, float b) {
    return (fabsf(a-b) < 0.00001f);
}

#pragma warning(push)
#pragma warning(disable : 4201) // Anonymous struct warning

struct vec2
{
    float x, y;
};

struct vec3
{
    float x, y, z;
};

union vec4
{
    struct {
        float x, y, z, w;
    };
    vec3 xyz;
};

union mat4
{
    float m[4][4];
    vec4 cols[4];

    inline vec4 row(int i) { // Returns i-th row of matrix
        return { m[0][i], m[1][i], m[2][i], m[3][i] };
    }
};

union mat3
{
    // Note: mat3 is used as a 3x3 matrix but padded to be 3x4 for alignment.
    float m[3][4];
    vec4 cols[3];

    inline vec3 row(int i) { // Returns i-th row of matrix
        return { m[0][i], m[1][i], m[2][i] };
    }
};
#pragma warning(pop)

inline float degreesToRadians(float degs) {
    return degs * (PI32 / 180.0f);
}

inline float length(vec3 v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline float dot(vec3 a, vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float dot(vec4 a, vec4 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

inline vec3 operator* (vec3 v, float f) {
    return {v.x*f, v.y*f, v.z*f};
}

inline vec3 operator/ (float f, vec3 v) {
    return {f/v.x, f/v.y, f/v.z};
}

inline vec3 normalise(vec3 v) {
    return v * (1.f / length(v));
}

inline vec3 normaliseOrZero(vec3 v) {
    float l = length(v);
    if(l < 0.00001f) return {};
    return v * (1.f / l);
}

inline bool areAlmostEqual(vec2 a, vec2 b) {
    return areAlmostEqual(a.x, b.x) && areAlmostEqual(a.y, b.y);
}

inline bool areAlmostEqual(vec3 a, vec3 b) {
    return areAlmostEqual(a.x, b.x) && areAlmostEqual(a.y, b.y) && areAlmostEqual(a.z, b.z);
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

inline vec3 operator*= (vec3 &lhs, float rhs) {
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    return lhs;
}

inline vec3 operator- (vec3 v) {
    return {-v.x, -v.y, -v.z};
}

inline vec3 operator+ (vec3 a, vec3 b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}

inline vec3 operator- (vec3 a, vec3 b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}

inline mat3 scaleMat3(vec3 scale) {
    return {
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0
    };
}

inline mat3 rotateYMat3(float rad) {
    float sinTheta = sinf(rad);
    float cosTheta = cosf(rad);
    return {
        cosTheta, 0, sinTheta, 0,
        0, 1, 0, 0,
        -sinTheta, 0, cosTheta, 0
    };
}

inline mat3 operator* (mat3 a, mat3 b) {
    return {
        dot(a.row(0), b.cols[0].xyz),
        dot(a.row(1), b.cols[0].xyz),
        dot(a.row(2), b.cols[0].xyz),
        0,
        dot(a.row(0), b.cols[1].xyz),
        dot(a.row(1), b.cols[1].xyz),
        dot(a.row(2), b.cols[1].xyz),
        0,
        dot(a.row(0), b.cols[2].xyz),
        dot(a.row(1), b.cols[2].xyz),
        dot(a.row(2), b.cols[2].xyz),
        0
    };
}

inline vec3 operator* (vec3 v, mat3 m) {
    return {
        dot(v, m.cols[0].xyz),
        dot(v, m.cols[1].xyz),
        dot(v, m.cols[2].xyz)
    };
}

inline mat3 transpose(mat3 m) {
    return {
        m.m[0][0], m.m[1][0], m.m[2][0], 0, 
        m.m[0][1], m.m[1][1], m.m[2][1], 0, 
        m.m[0][2], m.m[1][2], m.m[2][2], 0,
    };
}

inline mat4 scaleMat(float scale) {
    return {
        scale, 0, 0, 0,
        0, scale, 0, 0,
        0, 0, scale, 0,
        0, 0, 0, 1
    };
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

inline mat4 makePerspectiveMat(float aspectRatio, float fovXRadians, float zNear, float zFar)
{
    // float xScale = 1 / tanf(0.5f * fovXRadians); 
    // NOTE: 1/tan(X) = tan(90degs - X), so we can avoid a divide
    // float xScale = tanf((0.5f * PI32) - (0.5f * fovXRadians));
    float xScale = tanf(0.5f * (PI32 - fovXRadians));
    float yScale = xScale * aspectRatio;
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

inline vec4 operator* (vec4 v, mat4 m) {
    return {
        dot(v, m.cols[0]),
        dot(v, m.cols[1]),
        dot(v, m.cols[2]),
        dot(v, m.cols[3])
    };
}

inline mat4 transpose(mat4 m) {
    return {
        m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0], 
        m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1], 
        m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2], 
        m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]
    };
}

#pragma once

#include "3DMaths.h"

struct Player {
    vec3 pos;
    vec3 vel;
    vec3 fwd;
    float yRotation;
    float rotateSpeed;
};

Player playerInit(vec3 pos, vec3 fwd);

#pragma once

#include "3DMaths.h"

struct Camera {
    vec3 pos;
    vec3 fwd;
    float pitch;
    float yaw;
};

Camera cameraInit(vec3 pos, vec3 targetPos);
mat4 cameraUpdate(Camera* camera, bool keys[], float dt);

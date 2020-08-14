#pragma once

#include "3DMaths.h"

struct Camera {
    vec3 pos;
    vec3 fwd;
    float pitch;
    float yaw;
};

struct KeyState;

Camera cameraInit(vec3 pos, vec3 targetPos);
mat4 cameraUpdateFreeCam(Camera* camera, KeyState keys[], float dt);
mat4 cameraUpdateFollowPlayer(Camera* camera, vec3 playerPos);

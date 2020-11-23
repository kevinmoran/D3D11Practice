#pragma once

#include "3DMaths.h"
#include "Input.h"

struct Player {
    vec3 pos;
    vec3 vel;
    vec3 fwd;
    float yRotation;
    float rotateSpeed;
    bool isOnGround;
};

Player playerInit(vec3 pos, vec3 fwd);
void playerUpdate(Player* player, KeyState keys[], vec3 cameraFwd, float dt);


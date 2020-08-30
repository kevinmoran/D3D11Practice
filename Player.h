#pragma once

#include "3DMaths.h"
#include "Input.h"

struct Player {
    vec3 pos;
    vec3 vel;
    vec3 fwd;
    float yRotation;
    float rotateSpeed;
};

Player playerInit(vec3 pos, vec3 fwd);
mat4 playerUpdate(Player* player, KeyState keys[], vec3 cameraFwd, float dt);

inline mat4 calculateModelMatrix(Player player) {
    return scaleMat({1,1,1}) * rotateYMat(player.yRotation) * translationMat(player.pos);
}

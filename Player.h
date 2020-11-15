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
mat4 playerUpdate(Player* player, KeyState keys[], vec3 cameraFwd, float dt);

const vec3 playerScale = {0.5,1,0.5};

inline mat4 calculateModelMatrix(Player player) {
    return scaleMat(playerScale) * rotateYMat(player.yRotation) * translationMat(player.pos);
}

inline mat3 calculateNormalMatrix(Player player) {
    return transpose(rotateYMat3(-player.yRotation) * scaleMat3(1/playerScale));
}

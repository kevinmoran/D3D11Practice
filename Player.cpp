#include "Player.h"

Player playerInit(vec3 pos, vec3 fwd)
{
    Player result = {};
    result.pos = pos;
    result.fwd = fwd;
    result.yRotation = atan2f(fwd.x, fwd.z);
    return result;
}

mat4 playerUpdate(Player* player, KeyState keys[], vec3 cameraFwd, float dt)
{
    vec3 camFwdXZ = normalise({cameraFwd.x, 0, cameraFwd.z});
    vec3 camRightXZ = normalise(cross(camFwdXZ, {0, 1, 0}));

    vec3 moveDir = {};
    if(keys[KEY_W].isDown)
        moveDir += camFwdXZ;
    if(keys[KEY_S].isDown)
        moveDir -= camFwdXZ;
    if(keys[KEY_A].isDown)
        moveDir -= camRightXZ;
    if(keys[KEY_D].isDown)
        moveDir += camRightXZ;
    if(keys[KEY_SPACE].wentDown() && player->isOnGround) {
        player->isOnGround = false;
        const float PLAYER_JUMP_VELOCITY = 30.0f;
        player->vel.y = PLAYER_JUMP_VELOCITY;
    }

    float moveDirLength = length(moveDir);

    if(moveDirLength < 0.00001f)
        moveDir = {};
    else { // Rotate to face direction of movement
        moveDir *= (1.f / moveDirLength); // Normalise

        const float PLAYER_ROTATE_ACC = 100.f * PI32;
        float rotateSign = (cross(player->fwd, moveDir).y < 0) ? -1.f : 1.f;
        player->rotateSpeed += PLAYER_ROTATE_ACC * rotateSign * dt;
        const float PLAYER_ROTATE_FRICTION = 0.7f;
        player->rotateSpeed *= PLAYER_ROTATE_FRICTION;
        float rotateAmount = player->rotateSpeed * dt;
        
        // Avoid over-rotating
        float angleBetween = acosf(CLAMP_BETWEEN(dot(player->fwd, moveDir), -1.f, 1.f));
        if(fabsf(rotateAmount) > angleBetween) {
            rotateAmount = angleBetween * rotateSign;
            player->rotateSpeed = 0.f;
        }
        
        player->yRotation += rotateAmount;
    }
    
    const float PLAYER_ACCELERATION = 100.f;
    const float PLAYER_FRICTION = 0.8f;
    player->vel.x *= PLAYER_FRICTION;
    player->vel.z *= PLAYER_FRICTION;
    player->vel += moveDir * PLAYER_ACCELERATION * dt;

    if(!player->isOnGround) {
        const float PLAYER_GRAVITY = -80.f;
        player->vel.y += PLAYER_GRAVITY * dt;
    }

    player->pos += player->vel * dt;

    if(player->pos.y <= 0.f) {
        player->pos.y = 0.f;
        player->vel.y = 0.f;
        player->isOnGround = true;
    }

    mat4 modelMatrix = calculateModelMatrix(*player);
    player->fwd = {modelMatrix.m[0][2], modelMatrix.m[1][2], modelMatrix.m[2][2]};
    return modelMatrix;
}

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
        float angleBetween = acosf(CLAMP(dot(player->fwd, moveDir), 0.f, 1.f));
        if(fabsf(rotateAmount) > angleBetween) {
            rotateAmount = angleBetween * rotateSign;
            player->rotateSpeed = 0.f;
        }
        
        player->yRotation += rotateAmount;
    }
    
    const float PLAYER_ACCELERATION = 100.f;
    const float PLAYER_FRICTION = 0.8f;
    player->vel *= PLAYER_FRICTION;
    player->vel += moveDir * PLAYER_ACCELERATION * dt;
    
    player->pos += player->vel * dt;

    mat4 modelMatrix = calculateModelMatrix(*player);
    player->fwd = {modelMatrix.m[0][2], modelMatrix.m[1][2], modelMatrix.m[2][2]};
    return modelMatrix;
}

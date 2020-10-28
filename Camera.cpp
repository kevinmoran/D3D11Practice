#include "Camera.h"

#include "Input.h"

Camera cameraInit(vec3 pos, vec3 targetPos)
{
    Camera cam;
    cam.pos = pos;
    cam.fwd = normalise(targetPos - pos);

    cam.pitch = asinf(cam.fwd.y);
    cam.yaw = atan2f(-cam.fwd.x, -cam.fwd.z);

    return cam;
}

const float CAM_MOVE_SPEED = 5.f; // in metres per second
const float CAM_TURN_SPEED = PI32; // in radians per second

mat4 cameraUpdateFreeCam(Camera* camera, KeyState keys[], float dt)
{
    vec3 camFwdXZ = normalise({camera->fwd.x, 0, camera->fwd.z});
    vec3 camRightXZ = cross(camFwdXZ, {0, 1, 0});

    const float CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * dt;
    if(keys[KEY_W].isDown)
        camera->pos += camFwdXZ * CAM_MOVE_AMOUNT;
    if(keys[KEY_S].isDown)
        camera->pos -= camFwdXZ * CAM_MOVE_AMOUNT;
    if(keys[KEY_A].isDown)
        camera->pos -= camRightXZ * CAM_MOVE_AMOUNT;
    if(keys[KEY_D].isDown)
        camera->pos += camRightXZ * CAM_MOVE_AMOUNT;
    if(keys[KEY_E].isDown)
        camera->pos.y += CAM_MOVE_AMOUNT;
    if(keys[KEY_Q].isDown)
        camera->pos.y -= CAM_MOVE_AMOUNT;
    
    const float CAM_TURN_AMOUNT = CAM_TURN_SPEED * dt;
    if(keys[KEY_LEFT].isDown)
        camera->yaw += CAM_TURN_AMOUNT;
    if(keys[KEY_RIGHT].isDown)
        camera->yaw -= CAM_TURN_AMOUNT;
    if(keys[KEY_UP].isDown)
        camera->pitch += CAM_TURN_AMOUNT;
    if(keys[KEY_DOWN].isDown)
        camera->pitch -= CAM_TURN_AMOUNT;

    // Wrap yaw to avoid floating-point errors if we turn too far
    while(camera->yaw > PI32) 
        camera->yaw -= 2*PI32;
    while(camera->yaw < -PI32) 
        camera->yaw += 2*PI32;

    // Clamp pitch to stop camera flipping upside down
    camera->pitch = CLAMP_BETWEEN(camera->pitch, -degreesToRadians(85), degreesToRadians(85));

    mat4 viewMat = translationMat(-camera->pos) * rotateYMat(-camera->yaw) * rotateXMat(-camera->pitch);
    camera->fwd = {-viewMat.m[2][0], -viewMat.m[2][1], -viewMat.m[2][2]};

    return viewMat;
}

mat4 cameraUpdateFollowPlayer(Camera* camera, vec3 playerPos)
{
    *camera = cameraInit(playerPos + vec3{0,1,3}, playerPos);
    mat4 viewMat = translationMat(-camera->pos) * rotateYMat(-camera->yaw) * rotateXMat(-camera->pitch);
    return viewMat;
}

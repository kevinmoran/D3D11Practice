#pragma once

struct Camera {
    vec3 pos;
    vec3 fwd;
    float pitch;
    float yaw;
};

const float CAM_MOVE_SPEED = 5.f; // in metres per second
const float CAM_TURN_SPEED = PI32; // in radians per second

mat4 cameraUpdate(Camera* camera, bool keys[], float dt)
{
    vec3 camFwdXZ = normalise({camera->fwd.x, 0, camera->fwd.z});
    vec3 camRightXZ = cross(camFwdXZ, {0, 1, 0});

    const float CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * dt;
    if(keys[GameActionMoveCamFwd])
        camera->pos += camFwdXZ * CAM_MOVE_AMOUNT;
    if(keys[GameActionMoveCamBack])
        camera->pos -= camFwdXZ * CAM_MOVE_AMOUNT;
    if(keys[GameActionMoveCamLeft])
        camera->pos -= camRightXZ * CAM_MOVE_AMOUNT;
    if(keys[GameActionMoveCamRight])
        camera->pos += camRightXZ * CAM_MOVE_AMOUNT;
    if(keys[GameActionRaiseCam])
        camera->pos.y += CAM_MOVE_AMOUNT;
    if(keys[GameActionLowerCam])
        camera->pos.y -= CAM_MOVE_AMOUNT;
    
    const float CAM_TURN_AMOUNT = CAM_TURN_SPEED * dt;
    if(keys[GameActionTurnCamLeft])
        camera->yaw += CAM_TURN_AMOUNT;
    if(keys[GameActionTurnCamRight])
        camera->yaw -= CAM_TURN_AMOUNT;
    if(keys[GameActionLookUp])
        camera->pitch += CAM_TURN_AMOUNT;
    if(keys[GameActionLookDown])
        camera->pitch -= CAM_TURN_AMOUNT;

    // Wrap yaw to avoid floating-point errors if we turn too far
    while(camera->yaw >= 2*PI32) 
        camera->yaw -= 2*PI32;
    while(camera->yaw <= -2*PI32) 
        camera->yaw += 2*PI32;

    // Clamp pitch to stop camera flipping upside down
    if(camera->pitch > degreesToRadians(85)) 
        camera->pitch = degreesToRadians(85);
    if(camera->pitch < -degreesToRadians(85)) 
        camera->pitch = -degreesToRadians(85);

    mat4 viewMat = translationMat(-camera->pos) * rotateYMat(-camera->yaw) * rotateXMat(-camera->pitch);
    camera->fwd = {viewMat.m[0][2], viewMat.m[1][2], -viewMat.m[2][2]};

    return viewMat;
}
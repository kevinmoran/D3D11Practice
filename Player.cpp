#include "Player.h"

Player playerInit(vec3 pos, vec3 fwd)
{
    Player result = {};
    result.pos = pos;
    result.fwd = fwd;
    result.yRotation = atan2f(fwd.x, fwd.z);
    return result;
}
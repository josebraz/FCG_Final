#include "Asteroid.h"

Asteroid::Asteroid()
{
    position = glm::vec4(5.0f, -5.0f, 5.0f, 1.0f);
    rotate_xyz = glm::vec3(0.3f, 0.1f, 0.0f);
}

Asteroid::~Asteroid()
{
    //dtor
}

#include "Asteroid.h"


#include <iostream>

Asteroid::Asteroid()
{
    position = glm::vec4(5.0f, -5.0f, 5.0f, 1.0f);
    rotation = glm::vec3(0.3f, 0.1f, 0.0f);
    velocity = 0.05f;
    scale = 0.05f;
    t = 0.0f;
}

Asteroid::Asteroid(glm::vec4 pos, std::vector<glm::vec4> control)
{
    position = pos;
    rotation = glm::vec3(0.3f, 0.1f, 0.0f);
    controlPoints = control;
    velocity = 0.05f;
    scale = 0.05f;
    t = 0.0f;
}

Asteroid::~Asteroid()
{
    //dtor
}

void Asteroid::computeNextPosition(float deltaTime) {
    // calculado por curva de bezier
    t += velocity * deltaTime;
    float b03 = pow((1 - t), 3);
    float b13 = 3 * t * pow((1 - t), 2);
    float b23 = 3 * pow(t, 2) * (1 - t);
    float b33 = pow(t, 3);

    glm::vec4 new_position = b03 * controlPoints[0]
                           + b13 * controlPoints[1]
                           + b23 * controlPoints[2]
                           + b33 * controlPoints[3];
    new_position.w = 1.0f;
    position = new_position;
}


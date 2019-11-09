#include "Spaceship.h"

#include <algorithm>
#include <math.h>

#define PI 3.1415
#define MAX_SPEED 50.0
#define SPEED_INCREMENT 0.5
#define SPEED_DEINCREMENT 2.0
#define THETA_INCREMENT 0.1745 // 10º


Spaceship::Spaceship()
{
    speed = 0.0f;
    theta = 0.0f;
    phi   = 0.0f;
    position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

Spaceship::~Spaceship()
{
    //dtor
}

void Spaceship::speedUp()
{
    speed = std::min(MAX_SPEED, speed + SPEED_INCREMENT);
}

void Spaceship::brake()
{
    speed = std::max(0.0, speed - SPEED_DEINCREMENT);
}

void Spaceship::bendLeft()
{
    // theta = (((theta + PI) + THETA_INCREMENT) % 2*PI) - PI;
    theta = theta + THETA_INCREMENT;
}

void Spaceship::bendRight()
{
    // theta = (((theta + PI) - THETA_INCREMENT) % 2*PI) - PI;
    theta = theta - THETA_INCREMENT;
}

glm::vec4 Spaceship::cartesianDirection()
{
    float x = speed * cos(phi) * sin(theta);
    float y = speed * sin(phi);
    float z = speed * cos(phi) * cos(theta);
    return glm::vec4(x, y, z, 0.0f);
}

float Spaceship::speedGap(float deltaTime)
{
    return deltaTime * speed;
}

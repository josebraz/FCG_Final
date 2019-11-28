#include "Spaceship.h"

#include <stdio.h>
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

void Spaceship::speedUp(float deltaTime)
{
    speed = std::min(MAX_SPEED, speed + (deltaTime * SPEED_INCREMENT));
}

void Spaceship::brake(float deltaTime)
{
    speed = std::max(0.0, speed - (deltaTime * SPEED_DEINCREMENT));
}

void Spaceship::bendLeft(float deltaTime)
{
    theta = theta + (deltaTime * THETA_INCREMENT);
    if (theta > THETA_MAX)
        theta = THETA_MIN + (theta - THETA_MAX);
    if (theta < THETA_MIN)
        theta = THETA_MAX + (theta - THETA_MIN);
}

void Spaceship::bendRight(float deltaTime)
{
    theta = theta - (deltaTime * THETA_INCREMENT);
    if (theta > THETA_MAX)
        theta = THETA_MIN + (theta - THETA_MAX);
    if (theta < THETA_MIN)
        theta = THETA_MAX + (theta - THETA_MIN);
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

bullet Spaceship::shoot()
{
    float x = BULLET_SPEED * cos(phi) * sin(theta);
    float y = BULLET_SPEED * sin(phi);
    float z = BULLET_SPEED * cos(phi) * cos(theta);
    return bullet(position, glm::vec4(x, y, z, 0.0f));
}

glm::vec4 Spaceship::computeNewPosition(float deltaTime){
    glm::vec4 direction        = cartesianDirection();
    glm::vec4 dir_speed_scaled = direction * speedGap(deltaTime);
    glm::vec4 new_position     = dir_speed_scaled + position;
    this->position             = new_position;
    return new_position;
}

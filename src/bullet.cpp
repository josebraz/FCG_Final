#include "bullet.h"

#include <algorithm>
#include <math.h>
#include <iostream>


bullet::bullet()
{
    //ctor
}

bullet::bullet(glm::vec4 position, glm::vec4 direction)
{
    this->direction        = direction;
    this->start_position   = position;
    this->current_position = position;
}


bullet::~bullet()
{
    //dtor
}

glm::vec4 bullet::computeNewPosition(float deltaTime)
{
    t += deltaTime;
    glm::vec4 dir_speed_scaled = direction * t;
    glm::vec4 new_position     = dir_speed_scaled + start_position;
    this->current_position     = new_position;
    return new_position;
}

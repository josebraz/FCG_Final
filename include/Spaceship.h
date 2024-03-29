#ifndef SPACESHIP_H
#define SPACESHIP_H

#define PI 3.1415f
#define MAX_SPEED 10.0
#define BULLET_SPEED 30.0
#define SPEED_INCREMENT 3.0
#define SPEED_DEINCREMENT 10.0
#define THETA_INCREMENT 4.0

#define PHI_MAX 1.570796f
#define PHI_MIN -1.570796f
#define THETA_MAX 3.1415f
#define THETA_MIN -3.1415f

#include <glm/vec4.hpp>
#include <bullet.h>
#include <algorithm>
#include <math.h>

class Spaceship
{
    public:
        float speed;
        float theta; // Planar direction
        float phi; // Angular direction
        glm::vec4 position;
        float scale = 0.3f;
        int life = 10;

        Spaceship();
        virtual ~Spaceship();

        void speedUp(float deltaTime);
        void brake(float deltaTime);
        void bendLeft(float deltaTime);
        void bendRight(float deltaTime);

        glm::vec4 cartesianDirection();
        float speedGap(float deltaTime);
        glm::vec4 computeNewPosition(float deltaTime);

        bullet shoot();

    protected:

    private:

};

#endif // SPACESHIP_H

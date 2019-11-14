#ifndef ASTEROID_H
#define ASTEROID_H

#include <math.h>
#include <vector>
#include <iostream>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

class Asteroid
{
    public:
        glm::vec4 position;
        glm::vec3 rotation;
        float velocity;
        float scale;

        // for bezier
        std::vector<glm::vec4> controlPoints;
        float t;

        Asteroid();
        Asteroid(glm::vec4 pos, std::vector<glm::vec4> control);
        virtual ~Asteroid();

        void computeNextPosition(float deltaTime);

    protected:

    private:

};

#endif // ASTEROID_H

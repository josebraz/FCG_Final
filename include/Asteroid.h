#ifndef ASTEROID_H
#define ASTEROID_H

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

class Asteroid
{
    public:
        glm::vec4 position;
        glm::vec3 rotate_xyz;

        Asteroid();
        virtual ~Asteroid();

    protected:

    private:

};

#endif // ASTEROID_H

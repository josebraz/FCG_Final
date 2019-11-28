#ifndef BULLET_H
#define BULLET_H

#include <glm/vec4.hpp>

class bullet
{
    public:
        float t = 0.0f;
        glm::vec4 start_position;
        glm::vec4 direction; // speed too
        glm::vec4 current_position;

        bullet();
        bullet(glm::vec4 position, glm::vec4 direction);
        virtual ~bullet();

        glm::vec4 cartesianDirection();
        float speedGap(float deltaTime);
        glm::vec4 computeNewPosition(float deltaTime);

    protected:

    private:
};

#endif // BULLET_H

#ifndef SPACESHIP_H
#define SPACESHIP_H


#include <glm/vec4.hpp>

class Spaceship
{
    public:
        float speed;
        float theta; // Planar direction
        float phi; // Angular direction
        glm::vec4 position;

        Spaceship();
        virtual ~Spaceship();

        void speedUp();
        void brake();
        void bendLeft();
        void bendRight();

        glm::vec4 cartesianDirection();
        float speedGap(float deltaTime);

    protected:

    private:

};

#endif // SPACESHIP_H

#ifndef PHYSICS_H
#define PHYSICS_H

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <vector>
#include <iostream>
#include "mesh.h"

using namespace std;

class Physics {
   public:
        Physics(); //constructor
        ~Physics(); //destructor

        //simulate
        void simulate();

        //create objects for world
        void makePaddle(Mesh &); 
        void movePaddle(bool rayTraceResult, bool picked, glm::vec3 forceVector, btVector3 hitPointWorld, btVector3 destination, glm::vec3 camera_position);
        bool rayHit(glm::vec3 rayFrom, glm::vec3 rayTo, btVector3 &hitPointWorld);

        //misc physics stuff

        //bullet rigid body
        btRigidBody *simulationPaddle;

    //private:
        //collision shapes for collision checking
        btDiscreteDynamicsWorld *dynamicsWorld;
        btCollisionShape *paddleShape;
};
#endif

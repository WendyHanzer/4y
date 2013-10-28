#ifndef PHYSICS_H
#define PHYSICS_H

#include<btBulletDynamicsCommon.h>
#include<btBulletCollisionCommon.h>
#include<vector>
#include<iostream>
#include"mesh.h"

using namespace std;

class Physics {
   public:
        Physics(); //constructor
        ~Physics(); //destructor

        //simulate
        void simulate();

        //create objects for world
        void makeSphere(Mesh &); 
        void makeCube(Mesh &);
        void makeCylinder(Mesh &);
        void makeTable(Mesh &); 
        void makeWall(Mesh &);

        //misc physics stuff
        void moveCylinder(Mesh &cylinder, float offset_x, float offset_z);

        //bullet rigid body
        btRigidBody *simulationSphere;
        btRigidBody *simulationTable;
        btRigidBody *simulationCube;
        btRigidBody *simulationCylinder;
        vector<btRigidBody *> walls;
     
    private:
        //collision shapes for collision checking
        btDiscreteDynamicsWorld *dynamicsWorld;
        btCollisionShape *sphereShape;
        btCollisionShape *rectangleShape;
        btCollisionShape *cylinderShape;
};
#endif

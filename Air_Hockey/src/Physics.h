#ifndef PHYSICS_H
#define PHYSICS_H

#include<btBulletDynamicsCommon.h>
#include<btBulletCollisionCommon.h>
#include<vector>
#include<iostream>
#include"mesh.h"

using namespace std;

class Physics 
{
   public:
        Physics(); //constructor
        ~Physics(); //destructor

        //game objects
        vector<btRigidBody *> table;
        btRigidBody *puck;
        btRigidBody *paddle_Player;
        btRigidBody *paddle_AI;

        //create all game objects
        void createGround(Mesh &);
        void createTable(Mesh &);
        void createPuck(Mesh &);
        void createPaddle_Player(Mesh &);
        void createPaddle_AI(Mesh &);
        
        //simulate
        void simulate(float, float, float, float);
        
        //update
        void resetPuck(Mesh &);

        
    private:
        btDiscreteDynamicsWorld *dynamicsWorld;
        
        //set collision shape for the puck and paddle
        btCollisionShape *cylinder;
        
        //set collision shapes for the hockey table
        btCollisionShape *ground;
        btCollisionShape *tableSurface;
        btCollisionShape *eastTableWall;
        btCollisionShape *westTableWall;
        btCollisionShape *northTableWall;
        btCollisionShape *southTableWall;
        
        //set collision shapes for the goals
        btCollisionShape *goalLeftOne;
        btCollisionShape *goalLeftTwo;
        btCollisionShape *goalRightOne;
        btCollisionShape *goalRightTwo;

};
#endif

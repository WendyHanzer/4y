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

        //game objects
        vector<btRigidBody *> simulationBoard;      //handle to the board
        btRigidBody *simulationBall;                //handle to the sphere
        vector<btRigidBody *> simulationMazeWalls;  //handles to the maze walls
        
        void simulate(Mesh &);
        void resetMaze(Mesh &, vector<Mesh> &);
        
        //create & initialize the game pieces
        void makeBoard(Mesh &);
        void makeBall(Mesh &);
        void makeWalls(vector<Mesh> &);


    private:
        btDiscreteDynamicsWorld *dynamicsWorld;
        
        //define the board
        btCollisionShape *board;       //the board
        btCollisionShape *rightWall;   //right wall
        btCollisionShape *leftWall;    //left wall
        btCollisionShape *upperWall;   //negative z wall
        btCollisionShape *lowerWall;   //positive z wall

        btCollisionShape *ball;         //ball
        btCollisionShape *mazeWall;     //maze walls
};
#endif

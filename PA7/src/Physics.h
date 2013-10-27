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

     //add object to simulation world
     void addBall(Mesh &); //currently only one ball
     void addTable(Mesh &);  //table -- easier to refer back to
     void addStaticCube(Mesh &); //static cube that will be animated
     void addDynamicCyinder(Mesh &);
     void addObstacle(Mesh &); //objects that will not be animated

     void applyForceToBall(float force_x, float force_z);
     void translateStaticCube(float X, float Y, float Z);

     //simulate
     void simulate();

     //rigid bodies references
     btRigidBody *simulationBall;
     btRigidBody *simulationTable;
     btRigidBody *simulationStaticCube; //static & animated
     btRigidBody *simulationCylinder;  //dynamic object
     vector<btRigidBody *> obstacles;
   private:
      btDiscreteDynamicsWorld *dynamicsWorld;

      btCollisionShape *sphereShape;
      btCollisionShape *rectangleShape;
      btCollisionShape *cylinderShape;
};
#endif

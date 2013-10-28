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
     void makeSphere(Mesh &); 
     void makeCube(Mesh &);
     void makeCylinder(Mesh &);
     void makeTable(Mesh &); 
     void makeWall(Mesh &);

     void applyForceToSphere(float force_x, float force_z);
     void moveCylinder(Mesh &cylinder, float offset_x, float offset_z);

     //simulate
     void simulate();

     //rigid bodies references
     btRigidBody *simulationSphere;
     btRigidBody *simulationTable;
     btRigidBody *simulationStaticCube; //static & animated
     btRigidBody *simulationCylinder;  //dynamic object
     vector<btRigidBody *> walls;
     
   private:
      btDiscreteDynamicsWorld *dynamicsWorld;
      btCollisionShape *sphereShape;
      btCollisionShape *rectangleShape;
      btCollisionShape *cylinderShape;
};
#endif

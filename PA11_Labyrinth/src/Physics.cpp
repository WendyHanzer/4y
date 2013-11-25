#include "Physics.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>


Physics::Physics() 
{
    //build the broadphase
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    
    //set up the collsion configurator and dispatcher
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    
    //the actual physics solver
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;

    //the world
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    
    //set gravity
    dynamicsWorld->setGravity(btVector3(0,-15.8,0.1));
}


Physics::~Physics() 
{
    dynamicsWorld->removeRigidBody(simulationBall);
    delete simulationBall->getMotionState();
    delete simulationBall;

    for(unsigned int i = 0; i < simulationBoard.size(); i ++) 
    {
        dynamicsWorld->removeRigidBody(simulationBoard[i]);
    }

    delete ball;
    delete board;
    delete dynamicsWorld;
}


void Physics::simulate(Mesh &ballMesh) 
{
    simulationBall->applyForce(btVector3(ballMesh.xz_acceleration.x, 0.0, ballMesh.xz_acceleration.z), btVector3(0, 0, 0));
    dynamicsWorld->stepSimulation(1/60.0f, 10);
}


void Physics::resetMaze(Mesh &ballMesh, vector<Mesh> &NewMaze) 
{
    dynamicsWorld->removeRigidBody(simulationBall);
    delete simulationBall->getMotionState();
    delete simulationBall;

    for(unsigned int i = 0; i < simulationMazeWalls.size(); i ++) 
    {
        dynamicsWorld->removeRigidBody(simulationMazeWalls[i]);
        delete simulationMazeWalls[i]->getMotionState();
        delete simulationMazeWalls[i];
    }
    simulationMazeWalls.clear();

    makeBall(ballMesh);
    makeWalls(NewMaze);
}


void Physics::makeBoard(Mesh &boardMesh) 
{
    btRigidBody *buffer;
    btDefaultMotionState *boardMotionState;
    btRigidBody::btRigidBodyConstructionInfo *boardRigidBodyCI;

    //floor
    board = new btStaticPlaneShape(btVector3(0,1,0), boardMesh.initial_bound.first.y); //y is up
    boardMotionState =  new btDefaultMotionState(btTransform(   btQuaternion(0,0,0,1), 
                                                                btVector3(0.0, 0.0, 0.0) ) );
    boardRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,
                                                                    boardMotionState,
                                                                    board,
                                                                    btVector3(0,0,0));
    boardRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*boardRigidBodyCI));
    simulationBoard.push_back(buffer);

    //right wall
    rightWall = new btStaticPlaneShape(btVector3(-1,0,0), boardMesh.initial_bound.first.x);
    boardMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                                            btVector3(0.0, 0.0, 0.0) ) );
    boardRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,
                                                                    boardMotionState, 
                                                                    rightWall, 
                                                                    btVector3(0,0,0));
    boardRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*boardRigidBodyCI));
    simulationBoard.push_back(buffer);

    //left wall
    leftWall = new btStaticPlaneShape(btVector3(1,0,0), boardMesh.initial_bound.first.x);
    boardMotionState = new btDefaultMotionState(btTransform(    btQuaternion(0,0,0,1),
                                                                btVector3(0.0, 0.0, 0.0) ) );
    boardRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,
                                                                    boardMotionState, 
                                                                    leftWall, 
                                                                    btVector3(0,0,0));
    boardRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*boardRigidBodyCI));
    simulationBoard.push_back(buffer);

    //lower wall
    lowerWall= new btStaticPlaneShape(btVector3(0,0,-1), boardMesh.initial_bound.first.z);
    boardMotionState =  new btDefaultMotionState(btTransform(   btQuaternion(0,0,0,1),
                                                                btVector3(0.0, 0.0, 0.0) ) );
                                                             
    boardRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,
                                                                    boardMotionState, 
                                                                    lowerWall, 
                                                                    btVector3(0,0,0));
    boardRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*boardRigidBodyCI));
    simulationBoard.push_back(buffer);

    //upper wall
    upperWall= new btStaticPlaneShape(btVector3(0,0,1), boardMesh.initial_bound.first.z);
    boardMotionState =  new btDefaultMotionState(btTransform(   btQuaternion(0,0,0,1),
                                                                btVector3(0.0, 0.0, 0.0) ) );
                                                                
    boardRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,
                                                                    boardMotionState, 
                                                                    upperWall, 
                                                                    btVector3(0,0,0));
    boardRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*boardRigidBodyCI));
    simulationBoard.push_back(buffer);

    for(auto i : simulationBoard)
    {
       dynamicsWorld->addRigidBody(i);
    }
}


void Physics::makeWalls(vector<Mesh> &walls) 
{
    //calculate the half lenght,width, and heights
    glm::vec3 halfs;
    glm::vec3 centers;
    centers.x = (walls[0].current_position.first.x + walls[0].current_position.second.x)/2.0f;
    centers.y = (walls[0].current_position.first.y + walls[0].current_position.second.y)/2.0f;
    centers.z = (walls[0].current_position.first.z + walls[0].current_position.second.z)/2.0f;

    halfs.x = abs(walls[0].current_position.second.x - centers.x);
    halfs.y = abs(walls[0].current_position.second.y - centers.y);
    halfs.z = abs(walls[0].current_position.second.z - centers.z);

    btVector3 half(halfs.x, halfs.y, halfs.z);
    btRigidBody *buffer;

    //define the btCollisionshape
    mazeWall = new btBoxShape(half);
    btRigidBody::btRigidBodyConstructionInfo *boardRigidBodyCI;
    btDefaultMotionState *mazeMotionState;

    for(auto i: walls) 
    {
       mazeMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(i.offset.x, i.offset.y, i.offset.z) ) );
       boardRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0, mazeMotionState, mazeWall, btVector3(0,0,0));
       boardRigidBodyCI->m_restitution = 1.0f; //bounciness
       buffer= new btRigidBody((*boardRigidBodyCI));
       simulationMazeWalls.push_back(buffer);
    }

    for(auto i : simulationMazeWalls)
    {
      dynamicsWorld->addRigidBody(i);
    }

}


void Physics::makeBall(Mesh &ballMesh) {
    //find the radius
    float center = (ballMesh.current_position.first.x + ballMesh.current_position.second.x)/2.0f;
    float radius = abs(ballMesh.current_position.second.x - center);

    ball = new btSphereShape(radius);

    btDefaultMotionState* ballMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                                 btVector3(ballMesh.offset.x, ballMesh.offset.y, ballMesh.offset.z))); //create the motion state with a starting point
    btScalar mass = 0.5;
    btVector3 ballInertia(0,0,0);
    ball->calculateLocalInertia(mass,ballInertia);
    btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(mass,ballMotionState,ball,ballInertia);

    ballRigidBodyCI.m_restitution = 0.5;//ball.bounciness; //bounciness of the ball
    ballRigidBodyCI.m_friction = 0.5;

    simulationBall = new btRigidBody(ballRigidBodyCI);
    simulationBall->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(simulationBall);
}






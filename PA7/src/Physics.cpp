#include "Physics.h"
#include<glm/glm.hpp>

//constructor -- initialize the dynamic world
Physics::Physics() {

    //build the broadphase
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    //build the configuration and collision dispatcher
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    //the solver for collisions and stuff
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;

    //the world
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    //set the gravity
    dynamicsWorld->setGravity(btVector3(0,-9.8,0));
}

//destructor
Physics::~Physics() {
    delete dynamicsWorld;
}

//add the ball into dynamicsWorld
void Physics::addBall(Mesh &ball) {
    //find the radius
    float center = (ball.current_position.first.x + ball.current_position.second.x)/2.0f;
    float radius = abs(ball.current_position.second.x - center);

    //create collision shape
    sphereShape = new btSphereShape(radius);

    btDefaultMotionState* ballMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                                 btVector3(ball.offset.x, ball.offset.y, ball.offset.z))); //create the motion state with a starting point
    btScalar mass = 5.0; //kg
    btVector3 ballInertia(0,0,0);
    sphereShape->calculateLocalInertia(mass,ballInertia);
    btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(mass,ballMotionState,sphereShape,ballInertia);

    ballRigidBodyCI.m_restitution = 1.0;
    ballRigidBodyCI.m_friction = 0.5;

    simulationBall = new btRigidBody(ballRigidBodyCI);
    simulationBall->setActivationState(DISABLE_DEACTIVATION); //bullet optimizes out objects that remain idle for a while
    dynamicsWorld->addRigidBody(simulationBall);
}

void Physics::addTable(Mesh &table) {
    //calculate the half lenght,width, and heights
    glm::vec3 halfs;
    glm::vec3 centers;
    centers.x = (table.current_position.first.x + table.current_position.second.x)/2.0f;
    centers.y = (table.current_position.first.y + table.current_position.second.y)/2.0f;
    centers.z = (table.current_position.first.z + table.current_position.second.z)/2.0f;

    halfs.x = abs(table.current_position.second.x - centers.x);
    halfs.y = abs(table.current_position.second.y - centers.y);
    halfs.z = abs(table.current_position.second.z - centers.z);

    btVector3 half(halfs.x, halfs.y, halfs.z);

    //define the btCollisionshape
    rectangleShape = new btBoxShape(half);

    //define collision info
    btDefaultMotionState *tableMotionState =
               new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, table.current_position.second.y,0)));

    btRigidBody::btRigidBodyConstructionInfo tableRigidBodyCI(0, tableMotionState, rectangleShape, btVector3(0,0,0));
    tableRigidBodyCI.m_restitution = 0.8f; //bounciness

    simulationTable = new btRigidBody(tableRigidBodyCI);
    dynamicsWorld->addRigidBody(simulationTable);
}

void Physics::addObstacle(Mesh &obstacle) {
    //calculate the half lenght,width, and heights
    glm::vec3 halfs;
    glm::vec3 centers;
    centers.x = (obstacle.current_position.first.x + obstacle.current_position.second.x)/2.0f;
    centers.y = (obstacle.current_position.first.y + obstacle.current_position.second.y)/2.0f;
    centers.z = (obstacle.current_position.first.z + obstacle.current_position.second.z)/2.0f;

    halfs.x = abs(obstacle.current_position.second.x - centers.x);
    halfs.y = abs(obstacle.current_position.second.y - centers.y);
    halfs.z = abs(obstacle.current_position.second.z - centers.z);

    btVector3 half(halfs.x, halfs.y, halfs.z);
    btRigidBody *buffer;

    //define the btCollisionshape
    rectangleShape = new btBoxShape(half);
    btRigidBody::btRigidBodyConstructionInfo *obstacleRigidBodyCI;
    btDefaultMotionState *obstacleMotionState;

    obstacleMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(obstacle.offset.x,
                                                                                             obstacle.offset.y,
                                                                                             obstacle.offset.z) ) );
    obstacleRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0, obstacleMotionState, rectangleShape, btVector3(0,0,0));
    obstacleRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*obstacleRigidBodyCI));

    obstacles.push_back(buffer);
    dynamicsWorld->addRigidBody(buffer);
}

void Physics::addStaticCube(Mesh &cube) {
    //calculate the half lenght,width, and heights
    glm::vec3 halfs;
    glm::vec3 centers;
    centers.x = (cube.current_position.first.x + cube.current_position.second.x)/2.0f;
    centers.y = (cube.current_position.first.y + cube.current_position.second.y)/2.0f;
    centers.z = (cube.current_position.first.z + cube.current_position.second.z)/2.0f;

    halfs.x = abs(cube.current_position.second.x - centers.x);
    halfs.y = abs(cube.current_position.second.y - centers.y);
    halfs.z = abs(cube.current_position.second.z - centers.z);

    btVector3 half(halfs.x, halfs.y, halfs.z);
    btRigidBody *buffer;

    //define the btCollisionshape
    rectangleShape = new btBoxShape(half);
    btRigidBody::btRigidBodyConstructionInfo *cubeRigidBodyCI;
    btDefaultMotionState *cubeMotionState;

    cubeMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(cube.offset.x,
                                                                                             cube.offset.y,
                                                                                             cube.offset.z) ) );
    cubeRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0, cubeMotionState, rectangleShape, btVector3(0,0,0));
    cubeRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*cubeRigidBodyCI));

    buffer->setCollisionFlags(buffer->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    buffer->setActivationState(DISABLE_DEACTIVATION);

    simulationStaticCube = buffer;
    dynamicsWorld->addRigidBody(simulationStaticCube);
}

void Physics::addDynamicCyinder(Mesh &cylinder) {
    glm::vec3 halfs;
    glm::vec3 centers;
    centers.x = (cylinder.current_position.first.x + cylinder.current_position.second.x)/2.0f;
    centers.y = (cylinder.current_position.first.y + cylinder.current_position.second.y)/2.0f;
    centers.z = (cylinder.current_position.first.z + cylinder.current_position.second.z)/2.0f;

    halfs.x = abs(cylinder.current_position.second.x - centers.x);
    halfs.y = abs(cylinder.current_position.second.y - centers.y);
    halfs.z = abs(cylinder.current_position.second.z - centers.z);

    cylinderShape = new btCylinderShape(btVector3(halfs.x, halfs.y, halfs.z));

    btDefaultMotionState* cylinderMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
                                                 btVector3(cylinder.offset.x, cylinder.offset.y, cylinder.offset.z))); //create the motion state with a starting point
    btScalar mass = 20; //20kg
    btVector3 cylinderInertia(0,0,0);
    cylinderShape->calculateLocalInertia(mass,cylinderInertia);
    btRigidBody::btRigidBodyConstructionInfo cylinderRigidBodyCI(mass,cylinderMotionState,cylinderShape,cylinderInertia);

    cylinderRigidBodyCI.m_restitution = 0.7;//bounciness of the puck
    cylinderRigidBodyCI.m_friction = 0.5; //no friction

    simulationCylinder = new btRigidBody(cylinderRigidBodyCI);
    simulationCylinder->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(simulationCylinder);
}

void Physics::applyForceToBall(float force_x, float force_z) {
       simulationBall->applyForce(btVector3(force_x, 0.0, force_z), btVector3(0, 0, 0));
}

void Physics::translateStaticCube(float X, float Y, float Z) {
    btTransform trans;
    simulationStaticCube->getMotionState()->getWorldTransform(trans);
    trans.setOrigin(btVector3(X, Y, Z));
    simulationStaticCube->getMotionState()->setWorldTransform(trans);
}

void Physics::simulate() {
    dynamicsWorld->stepSimulation(1/60.0f, 10);
}





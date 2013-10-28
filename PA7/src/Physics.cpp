#include "Physics.h"
#include<glm/glm.hpp>


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
    dynamicsWorld->setGravity(btVector3(0,-9.8,0));
}


Physics::~Physics() 
{
    delete dynamicsWorld;
}


void Physics::makeSphere(Mesh &sphere) 
{
    //find the radius
    float center = (sphere.current_position.first.x + sphere.current_position.second.x)/2.0f;
    float radius = abs(sphere.current_position.second.x - center);

    //create collision shape
    sphereShape = new btSphereShape(radius);

    btDefaultMotionState* sphereMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(sphere.offset.x, sphere.offset.y, sphere.offset.z)));

    btVector3 sphereInertia(0,0,0);
    sphereShape->calculateLocalInertia(6.0,sphereInertia);
    
    //set sphere initial parameters
    btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI( 6.0,                //mass
                                                                sphereMotionState,  //initial position
                                                                sphereShape,        //collision shape of body
                                                                sphereInertia);     //local inertia

    sphereRigidBodyCI.m_restitution = 1.0;
    sphereRigidBodyCI.m_friction = 0.5;

    simulationSphere = new btRigidBody(sphereRigidBodyCI);
    simulationSphere->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(simulationSphere);
}

void Physics::makeTable(Mesh &table) 
{
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

    btRigidBody::btRigidBodyConstructionInfo tableRigidBodyCI(  0,                  //mass
                                                                tableMotionState,   //initial position
                                                                rectangleShape,     //collsion shape of body
                                                                btVector3(0,0,0));  //local inertia
    
    
    tableRigidBodyCI.m_restitution = 0.8f; //bounciness

    simulationTable = new btRigidBody(tableRigidBodyCI);
    dynamicsWorld->addRigidBody(simulationTable);
}

void Physics::makeWall(Mesh &wall) 
{
    //calculate the half lenght,width, and heights
    glm::vec3 halfs;
    glm::vec3 centers;
    centers.x = (wall.current_position.first.x + wall.current_position.second.x)/2.0f;
    centers.y = (wall.current_position.first.y + wall.current_position.second.y)/2.0f;
    centers.z = (wall.current_position.first.z + wall.current_position.second.z)/2.0f;

    halfs.x = abs(wall.current_position.second.x - centers.x);
    halfs.y = abs(wall.current_position.second.y - centers.y);
    halfs.z = abs(wall.current_position.second.z - centers.z);

    btVector3 half(halfs.x, halfs.y, halfs.z);
    btRigidBody *buffer;

    //define the btCollisionshape
    rectangleShape = new btBoxShape(half);
    btRigidBody::btRigidBodyConstructionInfo *wallRigidBodyCI;
    btDefaultMotionState *wallMotionState;

    wallMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(wall.offset.x,
                                                                                             wall.offset.y,
                                                                                             wall.offset.z) ) );
    wallRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0, wallMotionState, rectangleShape, btVector3(0,0,0));
    wallRigidBodyCI->m_restitution = 1.0f; //bounciness
    buffer= new btRigidBody((*wallRigidBodyCI));

    walls.push_back(buffer);
    dynamicsWorld->addRigidBody(buffer);
}

void Physics::makeCube(Mesh &cube) 
{
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

void Physics::makeCylinder(Mesh &cylinder) 
{
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
    btScalar mass = 20;
    btVector3 cylinderInertia(0,0,0);
    cylinderShape->calculateLocalInertia(mass,cylinderInertia);
    btRigidBody::btRigidBodyConstructionInfo cylinderRigidBodyCI(mass,cylinderMotionState,cylinderShape,cylinderInertia);

    cylinderRigidBodyCI.m_restitution = 0.0;//bounciness of the puck
    cylinderRigidBodyCI.m_friction = 0.5; //no friction

    simulationCylinder = new btRigidBody(cylinderRigidBodyCI);
    simulationCylinder->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(simulationCylinder);
}

void Physics::applyForceToSphere(float force_x, float force_z) 
{
    simulationSphere->applyForce(btVector3(force_x, 0.0, force_z), btVector3(0, 0, 0));
}

void Physics::moveCylinder(Mesh &cylinder, float offset_x, float offset_z) 
{
    btTransform trans;
    simulationCylinder->getMotionState()->getWorldTransform(trans);
    trans.setOrigin( btVector3(trans.getOrigin().getX()+offset_x, trans.getOrigin().getY(), trans.getOrigin().getZ()+offset_z));
    trans.setRotation( btQuaternion(0,0,0,1) );

    simulationCylinder->setWorldTransform(trans);

    cylinder.offset.x = trans.getOrigin().getX();
    cylinder.offset.y = trans.getOrigin().getY();
    cylinder.offset.z = trans.getOrigin().getZ();
    
}    

void Physics::simulate() 
{
    dynamicsWorld->stepSimulation(1/60.0f, 10);
}





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
    dynamicsWorld->setGravity(btVector3(0,0,0));
}


Physics::~Physics() 
{
    delete dynamicsWorld;
}


void Physics::simulate() 
{
    dynamicsWorld->stepSimulation(1/60.0f, 10);
}


void Physics::makePaddle(Mesh &paddle) 
{
	glm::vec3 centers;
	glm::vec3 halfs;
	centers.x = (paddle.current_position.first.x + paddle.current_position.second.x)/2.0f;
    centers.y = (paddle.current_position.first.y + paddle.current_position.second.y)/2.0f;
    centers.z = (paddle.current_position.first.z + paddle.current_position.second.z)/2.0f;

    halfs.x = abs(paddle.current_position.second.x - centers.x);
    halfs.y = abs(paddle.current_position.second.y - centers.y);
    halfs.z = abs(paddle.current_position.second.z - centers.z);
	
	std::cout << "Paddle centers: " << glm::to_string(centers) << std::endl;
	std::cout << "Paddle halfs: " << glm::to_string(halfs) << std::endl;
	
    //create collision shape
    paddleShape = new btCylinderShape(btVector3(halfs.x, halfs.y, halfs.z));

    btDefaultMotionState* paddleMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(   halfs.x, 
                                                                                    halfs.y, 
                                                                                    halfs.z)));

    btVector3 cylinderInertia(0,0,0);
    paddleShape->calculateLocalInertia(6.0, cylinderInertia);
    
    //set Cylinder initial parameters
    btRigidBody::btRigidBodyConstructionInfo cylinderRigidBodyCI( 6.0,                //mass
                                                                paddleMotionState,  //initial position
                                                                paddleShape,        //collision shape of body
                                                                cylinderInertia);     //local inertia

    cylinderRigidBodyCI.m_restitution = 0.0;
    cylinderRigidBodyCI.m_friction = 0.5;

    simulationPaddle = new btRigidBody(cylinderRigidBodyCI);
    simulationPaddle->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(simulationPaddle);

}

bool Physics::rayHit(glm::vec3 rayFrom, glm::vec3 rayTo, btVector3 &hitPointWorld )
{
	btVector3 btRayFrom = btVector3(rayFrom.x, rayFrom.y, rayFrom.z);
    btVector3 btRayTo = btVector3(rayTo.x, rayTo.y, rayTo.z);
    
	btCollisionWorld::ClosestRayResultCallback rayCallback(btRayFrom, btRayTo);
	
    dynamicsWorld->rayTest(btRayFrom, btRayTo, rayCallback);
	
	if (rayCallback.hasHit())
	{
    
        hitPointWorld = rayCallback.m_hitPointWorld;
        return true;

	}
	
    else
    {
        return false;
    }
	
}

void Physics::movePaddle(bool rayTraceResult, bool picked, glm::vec3 forceVector, btVector3 hitPointWorld, btVector3 destination, glm::vec3 camera_position)
{
     //http://www.bulletphysics.com/Bullet/BulletFull/classbtCollisionWorld.html
    if( picked == true)
    {
    
        //simulationPaddle->applyImpulse(btVector3(forceVector.x, forceVector.y, forceVector.z), btVector3(destination.x));
        //simulationPaddle->applyCentralForce(10*btVector3(forceVector.x,forceVector.y,forceVector.z));
        simulationPaddle->setLinearVelocity(btVector3(destination[0], 0, destination[2]));
        simulationPaddle->setAngularVelocity(btVector3(0,0,0));   
        
    }
   /* else if( picked == true && rayTraceResult == false)
    {
        simulationPaddle->setLinearVelocity(btVector3(0,0,0));
        simulationPaddle->setAngularVelocity(btVector3(0,0,0));  
    }*/
    else
    {
        //simulationPaddle->applyImpulse(btVector3(0,0,0), hitPointWorld - simulationPaddle->getCenterOfMassPosition());
        simulationPaddle->setLinearVelocity(btVector3(0,0,0));
        simulationPaddle->setAngularVelocity(btVector3(0,0,0));  
    }
    
}

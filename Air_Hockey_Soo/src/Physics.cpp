#include "Physics.h"
#include<glm/glm.hpp>

//global constants
const btScalar PUCK_MASS = 1500;    //mass of puck
btVector3 puckInertia(0,0,0);       //inertia of puck
const float MAX_VELOCITY = 40.0f;   //max puck velcity
const float MIN_VELOCITY = 40.0f;   //min puck velocity
const float TIMESTEP = 1/60.0f;     //use for time ticks
const int SUBSTEP = 100;            //max steps per tick



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


void Physics::createGround(Mesh &groundMesh)
{
    btRigidBody *buffer;
    ground = new btStaticPlaneShape(btVector3(0,1,0), groundMesh.offset.y);
    btDefaultMotionState *groundMotionState = new btDefaultMotionState( btTransform(btQuaternion(0,0,0,1),
                                                                        btVector3(0.0, 0.0, 0.0)));

    btRigidBody::btRigidBodyConstructionInfo *groundRigidBodyCI = 
                                new btRigidBody::btRigidBodyConstructionInfo(   0,                  //mass
                                                                                groundMotionState,  //motion state
                                                                                ground,             //collision shape
                                                                                btVector3(0,0,0));  //local inertia
                                                                                
    buffer = new btRigidBody((*groundRigidBodyCI));
    table.push_back(buffer);                                                                                
}


void Physics::createTable(Mesh &tableMesh)
{
    btRigidBody *buffer;
    glm::vec3 tableHalf, tableCenter;
    glm::vec3 halfTableWall_X, halfTableWall_Z;   
    glm::vec3 halfGoalWall;
    btRigidBody::btRigidBodyConstructionInfo *tableRigidBodyCI;
    btDefaultMotionState *tempMotionState;             
    
    //table
    tableCenter.x = (tableMesh.currentPos.first.x + tableMesh.currentPos.second.x)/2.0f;
    tableCenter.y = (tableMesh.currentPos.first.z + tableMesh.currentPos.second.z)/2.0f;
    tableCenter.z = 0.0;
    
    tableHalf.x = abs(tableMesh.currentPos.second.x - tableCenter.x);
    tableHalf.y = abs(tableMesh.currentPos.second.z - tableCenter.z);
    tableHalf.z = 0.25;
    
    //table walls
    halfTableWall_X.x = 0.25;
    halfTableWall_X.y = 50.0;
    halfTableWall_X.z = 50.0;
    
    halfTableWall_Z.x = 50.0;
    halfTableWall_Z.y = 50.0;
    halfTableWall_Z.z = 0.25;
    
    //goal walls
    halfGoalWall.x = ((tableHalf.x * 2 )/6);
    halfGoalWall.y = 1.0;
    halfGoalWall.z = 0.25;
    
    //bullet vectors
    btVector3 xWalls( halfTableWall_X.x, halfTableWall_X.y, halfTableWall_X.z);
    btVector3 zWalls( halfTableWall_Z.x, halfTableWall_Z.y, halfTableWall_Z.z);
    btVector3 tableFloor(tableHalf.x * 1.10, tableHalf.y, tableHalf.z *1.10);
    btVector3 goalWalls(halfGoalWall.x, halfGoalWall.y, halfGoalWall.z);
    
    
    ///////////////////////////////
    /* --- CREATE TABLE WALLS ---*/
    ///////////////////////////////
    
    //define collision shapes
    tableSurface    = new btBoxShape(tableFloor);
    eastTableWall   = new btBoxShape(xWalls);
    westTableWall   = new btBoxShape(xWalls);
    northTableWall  = new btBoxShape(zWalls);
    southTableWall  = new btBoxShape(zWalls); 

    //--EAST WALL collision info
    tempMotionState  = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(tableMesh.currentPos.second.x, 0, 0)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    eastTableWall,      //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);

    //--WEST WALL collision info
    tempMotionState  = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(tableMesh.currentPos.first.x, 0, 0)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    westTableWall,      //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);

    //--SOUTH WALL collision info
    tempMotionState  = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, halfTableWall_Z.y + 1.0,
                                                                                                tableMesh.currentPos.first.z)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    southTableWall,     //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);

    //--NORTH WALL collision info
    tempMotionState  = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, halfTableWall_Z.y + 1.0,
                                                                                                tableMesh.currentPos.second.z)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    northTableWall,     //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);

    //--TABLE SURFACE collision info
    tempMotionState  = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, tableMesh.currentPos.first.y, 0)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    tableSurface,       //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);

    
    for(auto i : table)
    {
        dynamicsWorld->addRigidBody(i);
    }

    //////////////////////////////
    /* --- CREATE GOAL WALLS ---*/
    //////////////////////////////
    
    //define collision shapes
    goalLeftOne     = new btBoxShape(goalWalls);
    goalLeftTwo     = new btBoxShape(goalWalls);
    goalRightOne    = new btBoxShape(goalWalls);
    goalRightTwo    = new btBoxShape(goalWalls);
    
    
    //--RIGHT GOAL collision info
    tempMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(tableMesh.currentPos.first.x + halfGoalWall.x,
                                                                                             0, tableMesh.currentPos.first.z)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    goalRightOne,       //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);

    tempMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(tableMesh.currentPos.second.x - halfGoalWall.x,
                                                                                             0, tableMesh.currentPos.first.z)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    goalRightTwo,       //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);


    //--LEFT GOAL collision info
    tempMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(tableMesh.currentPos.first.x + halfGoalWall.x,
                                                                                             0, tableMesh.currentPos.second.z)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    goalLeftOne,        //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);

    tempMotionState =  new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(tableMesh.currentPos.second.x - halfGoalWall.x,
                                                                                             0, tableMesh.currentPos.second.z)));
    tableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,                  //mass
                                                                    tempMotionState,    //motion state
                                                                    goalLeftTwo,        //collision shape
                                                                    btVector3(0,0,0));  //local inertia
    tableRigidBodyCI->m_restitution = 1.0f;
    buffer= new btRigidBody((*tableRigidBodyCI));
    table.push_back(buffer);
}


void Physics::createPuck(Mesh &puckMesh)
{
    glm::vec3 half;
    glm::vec3 center;
    btDefaultMotionState* puckMotionState;
    
    //find puck center
    center.x = (puckMesh.currentPos.first.x + puckMesh.currentPos.second.x)/2.0f;
    center.y = (puckMesh.currentPos.first.y + puckMesh.currentPos.second.y)/2.0f;
    center.z = (puckMesh.currentPos.first.z + puckMesh.currentPos.second.z)/2.0f;

    //find half extend
    half.x = abs(puckMesh.currentPos.second.x - center.x);
    half.y = abs(puckMesh.currentPos.second.y - center.y);
    half.z = abs(puckMesh.currentPos.second.z - center.z);

    //define collision shape
    cylinder = new btCylinderShape(btVector3(half.x, half.y, half.z));
    
    //define motion state
    puckMotionState = new btDefaultMotionState( btTransform(btQuaternion(0,0,0,1),
                                                btVector3(puckMesh.offset.x, puckMesh.offset.y, puckMesh.offset.z)));    
                                                
    //set local inertia                                                    
    cylinder->calculateLocalInertia(PUCK_MASS, puckInertia);   
    
    //define puck collision info                                                         
    btRigidBody::btRigidBodyConstructionInfo puckRigidBodyCI(   PUCK_MASS,          //mass
                                                                puckMotionState,    //motion state
                                                                cylinder,           //collision shape
                                                                puckInertia);       //local inertia
    
    //puck attributes
    puckRigidBodyCI.m_restitution = 0.5;
    puckRigidBodyCI.m_friction = 0.0;

    //add puck to world
    puck = new btRigidBody(puckRigidBodyCI);
    puck->setActivationState(DISABLE_DEACTIVATION);
    puck->setLinearFactor(btVector3(1,0,1));
    dynamicsWorld->addRigidBody(puck);
}


void Physics::createPaddle_Player(Mesh &paddleMesh)
{
    glm::vec3 half;
    glm::vec3 center;
    btDefaultMotionState* paddleMotionState;
    
    //find centers
    center.x = (paddleMesh.currentPos.first.x + paddleMesh.currentPos.second.x)/2.0f;
    center.y = (paddleMesh.currentPos.first.y + paddleMesh.currentPos.second.y)/2.0f;
    center.z = (paddleMesh.currentPos.first.z + paddleMesh.currentPos.second.z)/2.0f;

    //find half extend
    half.x = abs(paddleMesh.currentPos.second.x - center.x);
    half.y = 1.0;
    half.z = abs(paddleMesh.currentPos.second.z - center.z);


    //define collision shape
    cylinder = new btCylinderShape(btVector3(half.x, half.y, half.z));
    
    //define motion state
    paddleMotionState = new btDefaultMotionState(   btTransform(btQuaternion(0,0,0,1),
                                                    btVector3(paddleMesh.offset.x, paddleMesh.offset.y, paddleMesh.offset.z)));
    
    //define paddle collision info                           
    btRigidBody::btRigidBodyConstructionInfo paddleRigidBodyCI( 0,                  //mass
                                                                paddleMotionState,  //motion state
                                                                cylinder,           //collision shape
                                                                btVector3(0,0,0));  //local inertia
    //paddle attributes
    paddleRigidBodyCI.m_restitution = 0.5;
    paddleRigidBodyCI.m_friction = 0.5;

    //add paddle to world
    paddle_Player = new btRigidBody(paddleRigidBodyCI);
    paddle_Player->setCollisionFlags(paddle_Player->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    paddle_Player->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(paddle_Player);                                                    
}


void Physics::createPaddle_AI(Mesh &paddleMesh)
{
    glm::vec3 half;
    glm::vec3 center;
    btDefaultMotionState* paddleMotionState;
    
    //find centers
    center.x = (paddleMesh.currentPos.first.x + paddleMesh.currentPos.second.x)/2.0f;
    center.y = (paddleMesh.currentPos.first.y + paddleMesh.currentPos.second.y)/2.0f;
    center.z = (paddleMesh.currentPos.first.z + paddleMesh.currentPos.second.z)/2.0f;

    //find half extend
    half.x = abs(paddleMesh.currentPos.second.x - center.x);
    half.y = 1.0;
    half.z = abs(paddleMesh.currentPos.second.z - center.z);


    //define collision shape
    cylinder = new btCylinderShape(btVector3(half.x, half.y, half.z));
    
    //define motion state
    paddleMotionState = new btDefaultMotionState(   btTransform(btQuaternion(0,0,0,1),
                                                    btVector3(paddleMesh.offset.x, paddleMesh.offset.y, paddleMesh.offset.z)));
    
    //define paddle collision info                           
    btRigidBody::btRigidBodyConstructionInfo paddleRigidBodyCI( 0,                  //mass
                                                                paddleMotionState,  //motion state
                                                                cylinder,           //collision shape
                                                                btVector3(0,0,0));  //local inertia
    //paddle attributes
    paddleRigidBodyCI.m_restitution = 0.5;
    paddleRigidBodyCI.m_friction = 0.5;

    //add paddle to world
    paddle_AI = new btRigidBody(paddleRigidBodyCI);
    paddle_AI->setCollisionFlags(paddle_AI->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    paddle_AI->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(paddle_AI);                                                    
}

void Physics::simulate(float playerX, float playerZ, float aiX, float aiZ) 
{
    btTransform translate;

    //player paddle
    paddle_Player->getMotionState()->getWorldTransform(translate);
    translate.setOrigin(btVector3(playerX, 0.30, playerZ));
    paddle_Player->getMotionState()->setWorldTransform(translate);
    
    //ai paddle
    paddle_AI->getMotionState()->getWorldTransform(translate);
    translate.setOrigin(btVector3(aiX, 0.30, aiZ));
    paddle_AI->getMotionState()->setWorldTransform(translate);

    //clamp speed of puck
    btVector3 velocity = puck->getLinearVelocity();
    
    if(velocity.getX() > MAX_VELOCITY)
        velocity.setX(MAX_VELOCITY);
        
    else if(velocity.getX() < MIN_VELOCITY)
        velocity.setX(MIN_VELOCITY);

    if(velocity.getZ() > MAX_VELOCITY)     
        velocity.setZ(MAX_VELOCITY);
        
    else if(velocity.getZ() < MIN_VELOCITY)
        velocity.setZ(MIN_VELOCITY);
        
    //set puck velocity    
    puck->setLinearVelocity(velocity);

    dynamicsWorld->stepSimulation(TIMESTEP, SUBSTEP);
}


void Physics::resetPuck(Mesh &puckMesh) 
{
    //remove current puck from game
    dynamicsWorld->removeRigidBody(puck);
    delete puck->getMotionState();
    delete puck;
    
    //create new puck
    createPuck(puckMesh);
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
        //int size = rayCallback.getCollisionObjectArray().size();
        
        //for( int i=0; i<size; i++)
        {
            //if(rayCallback.m_collisionObjects[i] == paddle_Player)
            {
                return true;
            }
        }

        return false;

	}
	
    else
    {
        return false;
    }
	
}


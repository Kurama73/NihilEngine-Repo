// Physics.cpp
#include <NihilEngine/Physics.h>
#include <NihilEngine/Constants.h>
#include <limits>
#include <algorithm>

namespace NihilEngine {

    RigidBody::RigidBody(btRigidBody* body) : body(body) {}

    RigidBody::~RigidBody() {
        if (body) {
            delete body->getMotionState();
            delete body->getCollisionShape();
            delete body;
        }
    }

    void RigidBody::setPosition(const glm::vec3& position) {
        btTransform transform = body->getWorldTransform();
        transform.setOrigin(btVector3(position.x, position.y, position.z));
        body->setWorldTransform(transform);
    }

    void RigidBody::setRotation(const glm::quat& rotation) {
        btTransform transform = body->getWorldTransform();
        transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        body->setWorldTransform(transform);
    }

    void RigidBody::applyForce(const glm::vec3& force) {
        body->applyCentralForce(btVector3(force.x, force.y, force.z));
    }

    void RigidBody::applyImpulse(const glm::vec3& impulse) {
        body->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
    }

    void RigidBody::setLinearVelocity(const glm::vec3& velocity) {
        body->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
    }

    void RigidBody::setAngularVelocity(const glm::vec3& velocity) {
        body->setAngularVelocity(btVector3(velocity.x, velocity.y, velocity.z));
    }

    glm::vec3 RigidBody::getPosition() const {
        btTransform transform = body->getWorldTransform();
        btVector3 pos = transform.getOrigin();
        return glm::vec3(pos.x(), pos.y(), pos.z());
    }

    glm::quat RigidBody::getRotation() const {
        btTransform transform = body->getWorldTransform();
        btQuaternion rot = transform.getRotation();
        return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
    }

    glm::vec3 RigidBody::getLinearVelocity() const {
        btVector3 vel = body->getLinearVelocity();
        return glm::vec3(vel.x(), vel.y(), vel.z());
    }

    PhysicsWorld::PhysicsWorld()
        : broadphase(nullptr), collisionConfiguration(nullptr), dispatcher(nullptr), solver(nullptr), world(nullptr) {}

    PhysicsWorld::~PhysicsWorld() {
        if (world) {
            int numCollisionObjects = world->getNumCollisionObjects();
            for (int i = numCollisionObjects - 1; i >= 0; i--) {
                btCollisionObject* obj = world->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState()) {
                    delete body->getMotionState();
                }
                world->removeCollisionObject(obj);
                delete obj;
            }
        }

        delete world;
        delete solver;
        delete dispatcher;
        delete collisionConfiguration;
        delete broadphase;
    }

    void PhysicsWorld::initialize() {
        broadphase = new btDbvtBroadphase();
        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);
        solver = new btSequentialImpulseConstraintSolver();
        world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
        world->setGravity(btVector3(0, -9.81f, 0));
    }

    void PhysicsWorld::update(float deltaTime) {
        world->stepSimulation(deltaTime, 10);
    }

    RigidBody* PhysicsWorld::createRigidBody(float mass, const glm::vec3& position, btCollisionShape* shape) {
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(position.x, position.y, position.z));

        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }

        btDefaultMotionState* motionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        world->addRigidBody(body);

        return new RigidBody(body);
    }

    void PhysicsWorld::removeRigidBody(RigidBody* body) {
        if (body && body->getBulletBody()) {
            world->removeRigidBody(body->getBulletBody());
        }
        delete body;
    }

    bool RaycastVoxel(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float maxDistance,
        std::function<bool(const glm::ivec3&)> voxelCheck,
        RaycastHit& outHit
    ) {
        glm::ivec3 currentVoxel = glm::floor(origin);
        glm::vec3 tMax, tDelta;
        glm::ivec3 step;

        for (int i = 0; i < Constants::AXIS_COUNT; ++i) {
            if (direction[i] > 0.0f) {
                step[i] = 1;
                tMax[i] = (currentVoxel[i] + 1 - origin[i]) / direction[i];
                tDelta[i] = 1.0f / direction[i];
            } else if (direction[i] < 0.0f) {
                step[i] = -1;
                tMax[i] = (currentVoxel[i] - origin[i]) / direction[i];
                tDelta[i] = -1.0f / direction[i];
            } else {
                step[i] = 0;
                tMax[i] = std::numeric_limits<float>::infinity();
                tDelta[i] = std::numeric_limits<float>::infinity();
            }
        }

        float distance = 0.0f;
        glm::vec3 lastNormal(0.0f);

        while (distance < maxDistance) {
            if (voxelCheck(currentVoxel)) {
                outHit.hit = true;
                outHit.blockPosition = currentVoxel;
                outHit.hitNormal = lastNormal;
                outHit.hitPoint = origin + direction * distance;
                return true;
            }

            int side = 0;
            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) side = 0;
                else side = 2;
            } else {
                if (tMax.y < tMax.z) side = 1;
                else side = 2;
            }

            distance = tMax[side];
            if (distance > maxDistance) break;

            lastNormal = glm::vec3(0.0f);
            lastNormal[side] = -step[side];

            currentVoxel[side] += step[side];
            tMax[side] += tDelta[side];
        }

        outHit.hit = false;
        return false;
    }

    bool CheckAABBCollision(const AABB& a, const AABB& b) {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
            (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
            (a.min.z <= b.max.z && a.max.z >= b.min.z);
    }

    bool RayAABBIntersection(const glm::vec3& origin, const glm::vec3& direction, const AABB& box, float& t) {
        float tmin = -std::numeric_limits<float>::max();
        float tmax =  std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i) {
            if (std::abs(direction[i]) < 1e-6f) {
                if (origin[i] < box.min[i] || origin[i] > box.max[i]) return false;
                continue;
            }
            float invD = 1.0f / direction[i];
            float t0 = (box.min[i] - origin[i]) * invD;
            float t1 = (box.max[i] - origin[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmax <= tmin) return false;
        }
        t = tmin;
        return true;
    }

}
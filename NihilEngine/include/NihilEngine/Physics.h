#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>
#include <btBulletDynamicsCommon.h>

namespace NihilEngine {

    struct RaycastHit {
        bool hit = false;
        glm::ivec3 blockPosition;
        glm::vec3 hitNormal;
        glm::vec3 hitPoint;
    };

    struct AABB {
        glm::vec3 min;
        glm::vec3 max;
    };

    class RigidBody {
    public:
        RigidBody(btRigidBody* body);
        ~RigidBody();

        void setPosition(const glm::vec3& position);
        void setRotation(const glm::quat& rotation);
        void applyForce(const glm::vec3& force);
        void applyImpulse(const glm::vec3& impulse);
        void setLinearVelocity(const glm::vec3& velocity);
        void setAngularVelocity(const glm::vec3& velocity);

        glm::vec3 getPosition() const;
        glm::quat getRotation() const;
        glm::vec3 getLinearVelocity() const;

        btRigidBody* getBulletBody() { return body; }

    private:
        btRigidBody* body;
    };

    class PhysicsWorld {
    public:
        PhysicsWorld();
        ~PhysicsWorld();

        void initialize();
        void update(float deltaTime);

        RigidBody* createRigidBody(float mass, const glm::vec3& position, btCollisionShape* shape);
        void removeRigidBody(RigidBody* body);

        btDynamicsWorld* getWorld() { return world; }

    private:
        btBroadphaseInterface* broadphase;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        btSequentialImpulseConstraintSolver* solver;
        btDynamicsWorld* world;
    };

    bool RaycastVoxel(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float maxDistance,
        std::function<bool(const glm::ivec3&)> voxelCheck,
        RaycastHit& outHit
    );

    bool CheckAABBCollision(const AABB& a, const AABB& b);
    bool RayAABBIntersection(const glm::vec3& origin, const glm::vec3& direction, const AABB& box, float& t);

}
#include "physics.hpp"
#include <algorithm>

enum PhysicsBodyPointerType {
  PHYSICS_BODY_POINTER_MESH,
  PHYSICS_BODY_POINTER_VOXEL_CHUNK,
};

struct PhysicsBodyPointer {
  PhysicsBodyPointerType type;
  void* pointer;
};

Physics::Physics() {
  collisionConfiguration = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(collisionConfiguration);
  broadphase = new btDbvtBroadphase();
  solver = new btSequentialImpulseConstraintSolver();
  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
}

void Physics::step(GLfloat delta) {
  dynamicsWorld->stepSimulation(delta);
  std::vector<btRigidBody*> updated;
  for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
    btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    if (body && body->getUserPointer()) {
      PhysicsBodyPointer* p = (PhysicsBodyPointer*) body->getUserPointer();
      switch (p->type) {
        case PHYSICS_BODY_POINTER_MESH:
          if (!body->isStaticOrKinematicObject() && body->getMotionState()) {
            btDefaultMotionState* ms = (btDefaultMotionState*) body->getMotionState();
            ms->getWorldTransform(transform);
            const btVector3& origin = transform.getOrigin();
            const btQuaternion& rotation = transform.getRotation();
            Mesh* mesh = (Mesh*) p->pointer;
            mesh->setPosition(glm::vec3(origin.x(), origin.y(), origin.z()));
            mesh->setRotation(glm::quat(rotation.w(), rotation.x(), rotation.y(), rotation.z()));
          }
          break;
        case PHYSICS_BODY_POINTER_VOXEL_CHUNK: {
          VoxelChunk* chunk = (VoxelChunk*) p->pointer;
          if (chunk->needsCollidersUpdate) {
            chunk->updateColliders();
            updated.push_back(body);
          }
          break;
        }
      }
    }
  }
  for (const auto& body : updated) {
    PhysicsBodyPointer* p = (PhysicsBodyPointer*) body->getUserPointer();
    VoxelChunk* chunk = (VoxelChunk*) p->pointer;
    removeBody(body);
    addBody(chunk);
  }
}

void Physics::addBody(Mesh* mesh, const GLfloat mass, const bool isAlwaysActive, const bool isKinematic) {
  btRigidBody* body = addBody(
    mesh->getGeometry()->colliders,
    mesh->getPosition(),
    mesh->getRotation(),
    mesh->getScale(),
    mass,
    isAlwaysActive,
    isKinematic
  );
  body->setUserPointer((void*) new PhysicsBodyPointer({
    PHYSICS_BODY_POINTER_MESH,
    (void*) mesh
  }));
  mesh->setBody(body);
}

void Physics::addBody(VoxelChunk* chunk) {
  btRigidBody* body = addBody(
    chunk->colliders,
    chunk->getPosition(),
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::vec3(1.0, 1.0, 1.0)
  );
  body->setUserPointer((void*) new PhysicsBodyPointer({
    PHYSICS_BODY_POINTER_VOXEL_CHUNK,
    (void*) chunk
  }));
  chunk->setBody(body);
}

btRigidBody* Physics::addBody(const std::vector<GeometryCollider>& colliders, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const GLfloat mass, const bool isAlwaysActive, const bool isKinematic) {
  btCompoundShape* shape = new btCompoundShape();
  for (const auto& collider : colliders) {
    transform.setIdentity();
    transform.setOrigin(btVector3(collider.position.x, collider.position.y, collider.position.z));
    shape->addChildShape(transform, getColliderShape(collider.shape, collider.scale * scale));
  }
  const bool isDynamic = mass != 0.0;
  btVector3 localInertia(0, 0, 0);
  if (isDynamic) {
    shape->calculateLocalInertia(mass, localInertia);
  }
  transform.setIdentity();
  transform.setOrigin(btVector3(position.x, position.y, position.z));
  transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
  btDefaultMotionState* ms = new btDefaultMotionState(transform);
  btRigidBody::btRigidBodyConstructionInfo cInfo(mass, ms, shape, localInertia);
  btRigidBody* body = new btRigidBody(cInfo);
  if (isAlwaysActive) {
    body->setActivationState(DISABLE_DEACTIVATION);
  }
  if (isKinematic) {
    body->setCollisionFlags((body->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT) | btCollisionObject::CF_KINEMATIC_OBJECT);
  }
  dynamicsWorld->addRigidBody(body);
  return body;
}

void Physics::removeBody(btRigidBody* body) {
  if (body->getUserPointer()) {
    PhysicsBodyPointer* p = (PhysicsBodyPointer*) body->getUserPointer();
    switch (p->type) {
      case PHYSICS_BODY_POINTER_MESH:
        ((Mesh*) p->pointer)->setBody(nullptr);
        break;
      case PHYSICS_BODY_POINTER_VOXEL_CHUNK:
        ((VoxelChunk*) p->pointer)->setBody(nullptr);
        break;
    }
    delete p;
  }
  if (body->getMotionState()) {
    delete body->getMotionState();
  }
  if (body->getCollisionShape()) {
    btCollisionShape* shape = body->getCollisionShape();
    if (shape->isCompound()) {
      btCompoundShape* compound = (btCompoundShape*) shape;
      for (int i = 0, l = compound->getNumChildShapes(); i < l; i++) {
        delete compound->getChildShape(i);
      }
    }
    delete shape;
  }
  dynamicsWorld->removeRigidBody(body);
  delete body;
}

void Physics::setBodyPosition(btRigidBody* body, const glm::vec3& position) {
  if (!body->getMotionState()) {
    return;
  }
  body->getMotionState()->getWorldTransform(transform);
  transform.setOrigin(btVector3(position.x, position.y, position.z));
  body->getMotionState()->setWorldTransform(transform);
  body->setWorldTransform(transform);
}

void Physics::setBodyRotation(btRigidBody* body, const glm::quat& rotation) {
  if (!body->getMotionState()) {
    return;
  }
  body->getMotionState()->getWorldTransform(transform);
  transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
  body->getMotionState()->setWorldTransform(transform);
  body->setWorldTransform(transform);
}

bool Physics::getBodyData(btCollisionObject* target, GLuint& id, GLbyte& flags) {
  btRigidBody* body = btRigidBody::upcast(target);
  if (!body || !body->getUserPointer()) {
    return false;
  }
  PhysicsBodyPointer* p = (PhysicsBodyPointer*) body->getUserPointer();
  switch (p->type) {
    case PHYSICS_BODY_POINTER_MESH: {
      Mesh* mesh = (Mesh*) p->pointer;
      id = mesh->getId();
      flags = mesh->getFlags();
      break;
    }
    case PHYSICS_BODY_POINTER_VOXEL_CHUNK: {
      VoxelChunk* chunk = (VoxelChunk*) p->pointer;
      id = chunk->getVolume()->getId();
      flags = chunk->getVolume()->getFlags();
      break;
    }
  }
  return true;
}

btCollisionShape* Physics::getColliderShape(const GeometryColliderShape shape, const glm::vec3& scale) {
  switch (shape) {
    default:
    case GEOMETRY_COLLIDER_BOX:
      return new btBoxShape(btVector3(scale.x, scale.y, scale.z));
    case GEOMETRY_COLLIDER_CAPSULE:
      return new btCapsuleShape(fmax(scale.x, scale.y), scale.z);
    case GEOMETRY_COLLIDER_CYLINDER:
      return new btCylinderShape(btVector3(scale.x, scale.y, scale.z));
    case GEOMETRY_COLLIDER_SPHERE:
      return new btSphereShape(fmax(fmax(scale.x, scale.y), scale.z));
  }
}

btCollisionObject* Physics::getTempCollider(const GeometryColliderShape shape, const glm::vec3& position, const glm::vec3& scale) {
  btCollisionShape* collider = getColliderShape(shape, scale);
  transform.setIdentity();
  transform.setOrigin(btVector3(position.x, position.y, position.z));
  if (ghost.getCollisionShape() != nullptr) {
    delete ghost.getCollisionShape();
  }
  ghost.setCollisionShape(collider);
  ghost.setWorldTransform(transform);
  return (btCollisionObject*) &ghost;
}

std::vector<GLuint> Physics::getContactIds(btCollisionObject* target, const GLubyte mask) {
  struct ResultCallback : public btCollisionWorld::ContactResultCallback {
    struct Contact {
      GLuint id;
      GLfloat distance;
    };
    std::vector<Contact> contacts;
    btCollisionObject* target;
    const GLubyte mask;
    ResultCallback(btCollisionObject* target, const GLubyte mask) : target(target), mask(mask) {}
    virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) {
      GLfloat distance = cp.getDistance();
      if (distance <= 0.0) {
        btCollisionObject* obj;
        if (colObj0Wrap->getCollisionObject() == target) {
          obj = (btCollisionObject*) colObj1Wrap->getCollisionObject();
        } else {
          obj = (btCollisionObject*) colObj0Wrap->getCollisionObject();
        }
        GLuint id;
        GLbyte flags;
        if (
          !getBodyData(obj, id, flags)
          || (
            mask > 0 && !(flags & mask)
          )
        ) {
          return 1.0;
        }
        contacts.push_back({ id, distance });
      }
      return 1.0;
    }
  };
  ResultCallback cb(target, mask);
  dynamicsWorld->contactTest(target, cb);
  std::sort(cb.contacts.begin(), cb.contacts.end(), [](auto& a, auto& b) {
    return a.distance > b.distance;
  });
  std::vector<GLuint> contactIds;
  for (const auto& contact : cb.contacts) {
    if (std::find(contactIds.begin(), contactIds.end(), contact.id) == contactIds.end()) {
      contactIds.push_back(contact.id);
    }
  }
  return contactIds;
}

glm::vec3 Physics::getAccumulatedContacts(btCollisionObject* target, const GLubyte mask) {
  struct ResultCallback : public btCollisionWorld::ContactResultCallback {
    glm::vec3 contacts;
    btCollisionObject* target;
    const GLubyte mask;
    ResultCallback(btCollisionObject* target, const GLubyte mask) : contacts(0.0, 0.0, 0.0), target(target), mask(mask) {}
    virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) {
      GLfloat distance = cp.getDistance();
      if (distance < 0.0) {
        btCollisionObject* obj;
        if (colObj0Wrap->getCollisionObject() == target) {
          distance *= -1.0;
          obj = (btCollisionObject*) colObj1Wrap->getCollisionObject();
        } else {
          obj = (btCollisionObject*) colObj0Wrap->getCollisionObject();
        }
        if (mask > 0) {
          GLuint id;
          GLbyte flags;
          if (!getBodyData(obj, id, flags) || !(flags & mask)) {
            return 1.0;
          }
        }
        contacts += glm::vec3(
          cp.m_normalWorldOnB.x(),
          cp.m_normalWorldOnB.y(),
          cp.m_normalWorldOnB.z()
        ) * distance;
      }
      return 1.0;
    }
  };
  ResultCallback cb(target, mask);
  dynamicsWorld->contactTest(target, cb);
  return cb.contacts;
}

void Physics::setGravity(const glm::vec3& gravity) {
  dynamicsWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}

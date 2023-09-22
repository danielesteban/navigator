#include "object.hpp"

GLuint Object::objectId = 1;

Object::Object():
  id(objectId++),
  flags(0)
{
  
}

Object::~Object() {

}

GLuint Object::getId() {
  return id;
}

GLubyte Object::getFlags() {
  return flags;
}

void Object::setFlags(const GLubyte value) {
  flags = value;
}

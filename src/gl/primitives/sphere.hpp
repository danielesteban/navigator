#pragma once

#include "../geometry.hpp"

class SphereGeometry: public Geometry {
  public:
    SphereGeometry(const GLfloat radius);
};

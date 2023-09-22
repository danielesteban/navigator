#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../core/http.hpp"
#include "cubemapbuffer.hpp"
#include "texture.hpp"

class Environment {
  public:
    const GLuint id;
    Environment(Cubemapbuffer* buffer, HTTPRequest* request);
    ~Environment();
    Texture* getCubemap();
    Texture* getIrradiance();
    Texture* getPrefiltered();
  private:
    static GLuint environmentId;
    Texture* cubemap;
    Texture* irradiance;
    Texture* prefiltered;
};

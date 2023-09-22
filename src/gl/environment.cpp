#include "environment.hpp"
#include <stb_image.h>

GLuint Environment::environmentId = 1;

class EnvironmentMap: public Texture {
  public:
    EnvironmentMap(Cubemapbuffer* buffer, HTTPRequest* request): Texture(GL_TEXTURE_CUBE_MAP), buffer(buffer), request(request) {}
    ~EnvironmentMap() {
      if (request != nullptr) {
        delete request;
      }
    }
    GLuint get() {
      if (texture == 0) {
        update();
      }
      return texture;
    }
  private:
    Cubemapbuffer* buffer;
    HTTPRequest* request;
    void update() {
      if (request != nullptr && request->isReady) {
        if (request->response.status >= 200 && request->response.status < 400 && request->response.size > 0) {
          GLint x, y, n;
          GLfloat* data = stbi_loadf_from_memory(request->response.data, request->response.size, &x, &y, &n, 3);
          if (data != nullptr) {
            glGenTextures(1, &texture);
            buffer->renderHDR(data, x, y, texture, 512, 512);
            stbi_image_free(data);
          }
        }
        delete request;
        request = nullptr;
      }
    }
};

class IrradianceMap: public Texture {
  public:
    IrradianceMap(Cubemapbuffer* buffer, Texture* environmentMap): Texture(GL_TEXTURE_CUBE_MAP), buffer(buffer), environmentMap(environmentMap) {}
    GLuint get() {
      if (texture == 0) {
        GLuint environment = environmentMap->get();
        if (environment == 0) {
          return 0;
        }
        glGenTextures(1, &texture);
        buffer->renderIrradiance(environment, texture, 32, 32);
      }
      return texture;
    }
  private:
    Cubemapbuffer* buffer;
    Texture* environmentMap;
};

class PrefilteredMap: public Texture {
  public:
    PrefilteredMap(Cubemapbuffer* buffer, Texture* environmentMap): Texture(GL_TEXTURE_CUBE_MAP), buffer(buffer), environmentMap(environmentMap) {}
    GLuint get() {
      if (texture == 0) {
        GLuint environment = environmentMap->get();
        if (environment == 0) {
          return 0;
        }
        glGenTextures(1, &texture);
        buffer->renderPrefiltered(environment, texture, 128, 128);
      }
      return texture;
    }
  private:
    Cubemapbuffer* buffer;
    Texture* environmentMap;
};

Environment::Environment(Cubemapbuffer* buffer, HTTPRequest* request):
  id(environmentId++),
  cubemap(new EnvironmentMap(buffer, request)),
  irradiance(new IrradianceMap(buffer, cubemap)),
  prefiltered(new PrefilteredMap(buffer, cubemap))
{

}

Environment::~Environment() {
  Texture::gc(cubemap);
  Texture::gc(irradiance);
  Texture::gc(prefiltered);
}

Texture* Environment::getCubemap() {
  return cubemap;
}

Texture* Environment::getIrradiance() {
  return irradiance;
}

Texture* Environment::getPrefiltered() {
  return prefiltered;
}

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <vector>
#include "http.hpp"

class SFX {
  public:
    const GLuint id;
    SFX(HTTPRequest* request);
    ~SFX();
    bool isReady();
    void play(const glm::vec3& position);
    void update();
  private:
    static GLuint sfxId;
    ALuint buffer;
    HTTPRequest* request;
    std::vector<ALuint> sources;
    static ALuint LoadSound(const unsigned char *data, const size_t size);
    static sf_count_t vio_get_filelen(void *user_data);
    static sf_count_t vio_read(void *ptr, sf_count_t count, void *user_data);
    static sf_count_t vio_seek(sf_count_t offset, int whence, void *user_data);
    static sf_count_t vio_tell(void *user_data);
    static sf_count_t vio_write(const void *ptr, sf_count_t count, void *user_data);
};

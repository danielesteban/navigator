#pragma once

#include <lua.hpp>
#include "http.hpp"
#include "physics.hpp"
#include "sfx.hpp"
#include "window.hpp"
#include "../gl/camera.hpp"
#include "../gl/cubemapbuffer.hpp"
#include "../gl/environment.hpp"
#include "../gl/framebuffer.hpp"
#include "../gl/geometry.hpp"
#include "../gl/image.hpp"
#include "../gl/mesh.hpp"
#include "../gl/raycaster.hpp"
#include "../gl/shader.hpp"
#include "../gl/voxels/volume.hpp"
#include "../gl/primitives/box.hpp"
#include "../gl/primitives/plane.hpp"
#include "../gl/primitives/sphere.hpp"
#include "../gl/textures/brdf.hpp"
#include "../gl/textures/irradiance.hpp"

struct VMTooltip {
  std::string message;
  glm::vec2 offset;
  glm::vec2 position;
};

class VM {
  public:
    VM(HTTP* http, WindowContext* window);
    ~VM();
    void load(const std::string& src);
    void loop();
    void resize();

    std::deque<std::string> errors;
    std::deque<std::string> messages;
    std::deque<VMTooltip> tooltips;
  private:
    bool isReady;
    lua_State *L;
    HTTP* http;
    WindowContext* window;

    Camera camera;
    glm::vec4 clearColor;
    Cubemapbuffer cubemapbuffer;
    Physics physics;
    Raycaster raycaster;
    std::vector<Shader*> shaders;
    std::vector<SFX*> sfx;

    std::string source;
    GLfloat lastTick;
    GLfloat startTime;

    BRDF brdf;
    Irradiance irradiance;

    BoxGeometry box;
    PlaneGeometry plane;
    SphereGeometry sphere;

    void init();
    void reset();
    void resetGL();
    void logError(std::string msg);

    static Texture* getTexture(lua_State* L, GLint index);

    static int log(lua_State* L);
    static int clearLog(lua_State* L);

    static int info(lua_State* L);
    static int navigate(lua_State* L);
    static int setClearColor(lua_State* L);
    static int showTooltip(lua_State* L);

    static int clamp(lua_State* L);
    static int cross(lua_State* L);
    static int dot(lua_State* L);
    static int hsv(lua_State* L);
    static int lerp(lua_State* L);
    static int normalize(lua_State* L);
    static int spherical(lua_State* L);
    static int srgb(lua_State* L);

    static int camera_getFov(lua_State* L);
    static int camera_setFov(lua_State* L);
    static int camera_lookAt(lua_State* L);
    static int camera_getPosition(lua_State* L);
    static int camera_setPosition(lua_State* L);

    static int keyboard(lua_State* L);

    static int mouse_cursor(lua_State* L);
    static int mouse_lock(lua_State* L);
    static int mouse_unlock(lua_State* L);

    static int physics_getContacts(lua_State* L);
    static int physics_getContactIds(lua_State* L);
    static int physics_setGravity(lua_State* L);

    static int raycaster_setFromCamera(lua_State* L);
    static int raycaster_getRay(lua_State* L);
    static int raycaster_getResult(lua_State* L);
    static int raycaster_intersect(lua_State* L);

    static int environment_new(lua_State* L);
    static int environment_isReady(lua_State* L);
    static int environment_free(lua_State* L);

    static int framebuffer_new(lua_State* L);
    static int framebuffer_bind(lua_State* L);
    static int framebuffer_blit(lua_State* L);
    static int framebuffer_clear(lua_State* L);
    static int framebuffer_setTextureClearColor(lua_State* L);
    static int framebuffer_setTextureData(lua_State* L);
    static int framebuffer_unbind(lua_State* L);
    static int framebuffer_free(lua_State* L);

    static int geometry_new(lua_State* L);
    static int geometry_setColliders(lua_State* L);
    static int geometry_setIndex(lua_State* L);
    static int geometry_setVertices(lua_State* L);
    static int geometry_free(lua_State* L);
    void geometry_gc(Geometry* geometry);

    static int HTTP_new(lua_State* L);
    static int HTTP_isReady(lua_State* L);
    static int HTTP_getResponse(lua_State* L);
    static int HTTP_free(lua_State* L);

    static int image_new(lua_State* L);
    static int image_isReady(lua_State* L);
    static int image_getSize(lua_State* L);
    static int image_free(lua_State* L);

    static int mesh_new(lua_State* L);
    static int mesh_getId(lua_State* L);
    static int mesh_getPosition(lua_State* L);
    static int mesh_setPosition(lua_State* L);
    static int mesh_getScale(lua_State* L);
    static int mesh_setScale(lua_State* L);
    static int mesh_lookAt(lua_State* L);
    static int mesh_getFlags(lua_State* L);
    static int mesh_setFlags(lua_State* L);
    static int mesh_getFrustumCulling(lua_State* L);
    static int mesh_setFrustumCulling(lua_State* L);
    static int mesh_enablePhysics(lua_State* L);
    static int mesh_disablePhysics(lua_State* L);
    static int mesh_applyForce(lua_State* L);
    static int mesh_applyImpulse(lua_State* L);
    static int mesh_setDamping(lua_State* L);
    static int mesh_getAngularVelocity(lua_State* L);
    static int mesh_setAngularVelocity(lua_State* L);
    static int mesh_getLinearVelocity(lua_State* L);
    static int mesh_setLinearVelocity(lua_State* L);
    static int mesh_getContacts(lua_State* L);
    static int mesh_getContactIds(lua_State* L);
    static int mesh_uniformInt(lua_State* L);
    static int mesh_uniformFloat(lua_State* L);
    static int mesh_uniformTexture(lua_State* L);
    static int mesh_uniformVec2(lua_State* L);
    static int mesh_uniformVec3(lua_State* L);
    static int mesh_uniformVec4(lua_State* L);
    static int mesh_render(lua_State* L);
    static int mesh_free(lua_State* L);

    static int noise_new(lua_State* L);
    static int noise_get2D(lua_State* L);
    static int noise_get3D(lua_State* L);
    static int noise_free(lua_State* L);

    static int sfx_new(lua_State* L);
    static int sfx_isReady(lua_State* L);
    static int sfx_play(lua_State* L);
    static int sfx_free(lua_State* L);

    static int shader_new(lua_State* L);
    static int shader_getBlend(lua_State* L);
    static int shader_setBlend(lua_State* L);
    static int shader_getDepthTest(lua_State* L);
    static int shader_setDepthTest(lua_State* L);
    static int shader_getFaceCulling(lua_State* L);
    static int shader_setFaceCulling(lua_State* L);
    static int shader_uniformInt(lua_State* L);
    static int shader_uniformFloat(lua_State* L);
    static int shader_uniformTexture(lua_State* L);
    static int shader_uniformVec2(lua_State* L);
    static int shader_uniformVec3(lua_State* L);
    static int shader_uniformVec4(lua_State* L);
    static int shader_render(lua_State* L);
    static int shader_free(lua_State* L);
    void shader_gc(Shader* shader);

    static int voxels_new(lua_State* L);
    static int voxels_getId(lua_State* L);
    static int voxels_get(lua_State* L);
    static int voxels_set(lua_State* L);
    static int voxels_getFlags(lua_State* L);
    static int voxels_setFlags(lua_State* L);
    static int voxels_enablePhysics(lua_State* L);
    static int voxels_disablePhysics(lua_State* L);
    static int voxels_ground(lua_State* L);
    static int voxels_pathfind(lua_State* L);
    static int voxels_render(lua_State* L);
    static int voxels_free(lua_State* L);
};

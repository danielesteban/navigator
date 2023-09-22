#include "vm.hpp"
#include "lib/luajson.hpp"
#include <FastNoise/FastNoise.h>
#include <glm/gtc/type_ptr.hpp>

VM::VM(HTTP* http, WindowContext* window):
  isReady(false),
  brdf(),
  camera(),
  clearColor(glm::vec4(0, 0, 0, 1)),
  cubemapbuffer(),
  http(http),
  irradiance(),
  physics(),
  raycaster(),
  source(""),
  lastTick(0),
  startTime(0),
  window(window),
  box(2, 2, 2),
  plane(2, 2),
  sphere(1)
{
  init();
}

VM::~VM() {
  lua_close(L);
}

void VM::init() {
  L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "json", luaopen_json, 1);

  lua_newtable(L);

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, environment_new, 1);
  lua_setfield(L, -2, "Environment");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, framebuffer_new, 1);
  lua_setfield(L, -2, "Framebuffer");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, geometry_new, 1);
  lua_setfield(L, -2, "Geometry");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, HTTP_new, 1);
  lua_setfield(L, -2, "HTTP");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, image_new, 1);
  lua_setfield(L, -2, "Image");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, mesh_new, 1);
  lua_setfield(L, -2, "Mesh");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, noise_new, 1);
  lua_setfield(L, -2, "Noise");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, sfx_new, 1);
  lua_setfield(L, -2, "SFX");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, shader_new, 1);
  lua_setfield(L, -2, "Shader");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, voxels_new, 1);
  lua_setfield(L, -2, "Voxels");

  lua_getglobal(L, "collectgarbage");
  lua_setfield(L, -2, "collectgarbage");
  lua_getglobal(L, "ipairs");
  lua_setfield(L, -2, "ipairs");
  lua_getglobal(L, "pairs");
  lua_setfield(L, -2, "pairs");
  lua_getglobal(L, "setmetatable");
  lua_setfield(L, -2, "setmetatable");
  lua_getglobal(L, "tonumber");
  lua_setfield(L, -2, "tonumber");
  lua_getglobal(L, "tostring");
  lua_setfield(L, -2, "tostring");

  lua_getglobal(L, "json");
  lua_setfield(L, -2, "json");

  lua_getglobal(L, "math");
  lua_pushcfunction(L, clamp);
  lua_setfield(L, -2, "clamp");
  lua_pushcfunction(L, cross);
  lua_setfield(L, -2, "cross");
  lua_pushcfunction(L, dot);
  lua_setfield(L, -2, "dot");
  lua_pushcfunction(L, hsv);
  lua_setfield(L, -2, "hsv");
  lua_pushcfunction(L, lerp);
  lua_setfield(L, -2, "lerp");
  lua_pushcfunction(L, normalize);
  lua_setfield(L, -2, "normalize");
  lua_pushcfunction(L, spherical);
  lua_setfield(L, -2, "spherical");
  lua_pushcfunction(L, srgb);
  lua_setfield(L, -2, "srgb");
  lua_setfield(L, -2, "math");

  lua_getglobal(L, "table");
  lua_setfield(L, -2, "table");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, log, 1);
  lua_setfield(L, -2, "log");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, clearLog, 1);
  lua_setfield(L, -2, "clearLog");

  lua_pushcfunction(L, info);
  lua_setfield(L, -2, "info");
  lua_pushlightuserdata(L, window);
  lua_pushcclosure(L, navigate, 1);
  lua_setfield(L, -2, "navigate");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, setClearColor, 1);
  lua_setfield(L, -2, "setClearColor");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, showTooltip, 1);
  lua_setfield(L, -2, "showTooltip");

  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, camera_lookAt, 1);
  lua_setfield(L, -2, "lookAt");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, camera_getFov, 1);
  lua_setfield(L, -2, "getFov");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, camera_setFov, 1);
  lua_setfield(L, -2, "setFov");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, camera_getPosition, 1);
  lua_setfield(L, -2, "getPosition");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, camera_setPosition, 1);
  lua_setfield(L, -2, "setPosition");
  lua_setfield(L, -2, "camera");

  lua_pushlightuserdata(L, window);
  lua_pushcclosure(L, keyboard, 1);
  lua_setfield(L, -2, "keyboard");

  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, physics_getContacts, 1);
  lua_setfield(L, -2, "getContacts");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, physics_getContactIds, 1);
  lua_setfield(L, -2, "getContactIds");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, physics_setGravity, 1);
  lua_setfield(L, -2, "setGravity");
  lua_setfield(L, -2, "physics");

  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, raycaster_setFromCamera, 1);
  lua_setfield(L, -2, "setFromCamera");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, raycaster_intersect, 1);
  lua_setfield(L, -2, "intersect");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, raycaster_getRay, 1);
  lua_setfield(L, -2, "getRay");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, raycaster_getResult, 1);
  lua_setfield(L, -2, "getResult");
  lua_setfield(L, -2, "raycaster");

  lua_newtable(L);
  lua_pushlightuserdata(L, window);
  lua_pushcclosure(L, mouse_lock, 1);
  lua_setfield(L, -2, "lock");
  lua_pushlightuserdata(L, window);
  lua_pushcclosure(L, mouse_unlock, 1);
  lua_setfield(L, -2, "unlock");
  lua_pushcfunction(L, mouse_cursor);
  lua_setfield(L, -2, "cursor");
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "x");
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "y");
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "dx");
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "dy");
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "wheel");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "hover");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "locked");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "primary");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "primaryDown");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "primaryUp");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "secondary");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "secondaryDown");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "secondaryUp");
  lua_setfield(L, -2, "mouse");

  lua_newtable(L);
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "x");
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "y");
  lua_setfield(L, -2, "resolution");

  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "delta");
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "time");

  lua_setglobal(L, "env");
}

void VM::reset() {
  lua_close(L);
  init();
  camera.reset();
  clearColor = glm::vec4(0, 0, 0, 1);
  physics.setGravity(glm::vec3(0, -10, 0));
  errors.clear();
  messages.clear();
  tooltips.clear();
  lastTick = 0;
  startTime = (GLfloat) glfwGetTime();
}

void VM::resetGL() {
  // glViewport(0, 0, window->resolution.x, window->resolution.y);
  // camera.setAspect(window->resolution.x / window->resolution.y);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  // for (GLint i = 0; i < 16; i++) {
  //   glActiveTexture(GL_TEXTURE0 + i);
  //   glBindTexture(GL_TEXTURE_2D, 0);
  //   glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  //   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  // }
}

void VM::load(const std::string& src) {
  source = src;
  reset();
  if (luaL_loadstring(L, src.c_str()) != LUA_OK) {
    logError(lua_tostring(L, -1));
    lua_pop(L, 1);
    isReady = false;
    return;
  }
  lua_getglobal(L, "env");
  lua_getfield(L, -1, "resolution");
  lua_pushnumber(L, window->resolution.x);
  lua_setfield(L, -2, "x");
  lua_pushnumber(L, window->resolution.y);
  lua_setfield(L, -2, "y");
  lua_pop(L, 1);
  lua_setupvalue(L, -2, 1);
  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    logError(lua_tostring(L, -1));
    lua_pop(L, 1);
    isReady = false;
    resetGL();
    return;
  }
  lua_gc(L, LUA_GCCOLLECT);
  resetGL();
  lua_getglobal(L, "env");
  int type = lua_getfield(L, -1, "loop");
  lua_pop(L, 2);
  if (type != LUA_TFUNCTION) {
    logError("Couldn't find loop function");
    isReady = false;
    return;
  }
  isReady = true;
}

void VM::loop() {
  glViewport(0, 0, window->resolution.x, window->resolution.y);
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  camera.setAspect(window->resolution.x / window->resolution.y);

  if (!isReady) {
    return;
  }

  const GLfloat time = (GLfloat) glfwGetTime() - startTime;
  const GLfloat delta = glm::min((GLfloat) time - lastTick, (GLfloat) 0.2);
  lastTick = time;
  physics.step(delta);

  for (const auto& shader : shaders) {
    shader->setUniformVec2("resolution", window->resolution);
    shader->setUniformFloat("time", time);
  }
  for (const auto& sound : sfx) {
    sound->update();
  }

  lua_getglobal(L, "env");

  lua_pushnumber(L, delta);
  lua_setfield(L, -2, "delta");

  lua_pushnumber(L, time);
  lua_setfield(L, -2, "time");

  lua_getfield(L, -1, "mouse");
  lua_pushnumber(L, window->mouse.position.x);
  lua_setfield(L, -2, "x");
  lua_pushnumber(L, window->mouse.position.y);
  lua_setfield(L, -2, "y");
  lua_pushnumber(L, window->mouse.movement.x);
  lua_setfield(L, -2, "dx");
  lua_pushnumber(L, window->mouse.movement.y);
  lua_setfield(L, -2, "dy");
  lua_pushnumber(L, window->mouse.wheel);
  lua_setfield(L, -2, "wheel");
  lua_pushboolean(L, window->mouse.hover);
  lua_setfield(L, -2, "hover");
  lua_pushboolean(L, window->mouse.locked);
  lua_setfield(L, -2, "locked");
  lua_pushboolean(L, window->mouse.buttons.primary);
  lua_setfield(L, -2, "primary");
  lua_pushboolean(L, window->mouse.buttons.primaryDown);
  lua_setfield(L, -2, "primaryDown");
  lua_pushboolean(L, window->mouse.buttons.primaryUp);
  lua_setfield(L, -2, "primaryUp");
  lua_pushboolean(L, window->mouse.buttons.secondary);
  lua_setfield(L, -2, "secondary");
  lua_pushboolean(L, window->mouse.buttons.secondaryDown);
  lua_setfield(L, -2, "secondaryDown");
  lua_pushboolean(L, window->mouse.buttons.secondaryUp);
  lua_setfield(L, -2, "secondaryUp");
  lua_pop(L, 1);

  lua_getfield(L, -1, "resolution");
  lua_pushnumber(L, window->resolution.x);
  lua_setfield(L, -2, "x");
  lua_pushnumber(L, window->resolution.y);
  lua_setfield(L, -2, "y");
  lua_pop(L, 1);

  lua_getfield(L, -1, "loop");
  lua_remove(L, -2);

  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    logError(lua_tostring(L, -1));
    lua_pop(L, 1);
    isReady = false;
  }

  lua_gc(L, LUA_GCCOLLECT);
  resetGL();
  const glm::vec3& position = camera.getPosition();
  const glm::vec3& front = camera.getFront();
  alListenerfv(AL_POSITION, glm::value_ptr(position));
  ALfloat orientation[6] = { front.x, front.y, front.z, 0.0, 1.0, 0.0 };
  alListenerfv(AL_ORIENTATION, orientation);
}

void VM::logError(std::string msg) {
  if (msg.find("[string \"") == 0) {
    GLint start = msg.find("\"]:", 9);
    GLint end = msg.find(": ", start + 3);
    if (start != -1 && end != -1) {
      GLint line = std::stoi(msg.substr(start + 3, end - start - 3));
      msg = msg.substr(end + 2) + "\n";
      // @dani @hack?
      msg += Shader::getLinesNear(source, line - 1);
    }
  }
  errors.push_back(msg);
  if (errors.size() > 16) {
    errors.pop_front();
  }
}

Texture* VM::getTexture(lua_State* L, GLint index) {
  Environment** environment = (Environment**) luaL_testudata(L, index, "Environment");
  if (environment != nullptr) {
    const char* mapNames[] = {
      "cubemap",
      "irradiance",
      "prefiltered",
      nullptr
    };
    const int map = luaL_checkoption(L, index + 1, nullptr, mapNames);
    Texture* texture;
    switch (map) {
      default:
      case 0:
        return (*environment)->getCubemap();
      case 1:
        return (*environment)->getIrradiance();
      case 2:
        return (*environment)->getPrefiltered();
    }
  }
  Framebuffer** framebuffer = (Framebuffer**) luaL_testudata(L, index, "Framebuffer");
  if (framebuffer != nullptr) {
    Texture* texture = (*framebuffer)->getTexture(luaL_optnumber(L, index + 1, 0));
    if (texture == nullptr) {
      lua_pushliteral(L, "Framebuffer texture index out of bounds");
      lua_error(L);
    }
    return texture;
  }
  return *((Image**) luaL_checkudata(L, index, "Image"));
}

int VM::log(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const std::string message = luaL_checkstring(L, 1);
  vm->messages.push_back(message);
  if (vm->messages.size() > 16) {
    vm->messages.pop_front();
  }
  return 0;
}

int VM::clearLog(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  vm->messages.clear();
  return 0;
}

int VM::info(lua_State* L) {
  GLint major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  std::string gl = std::to_string(major) + "." + std::to_string(minor);
  const GLchar* gpu = (GLchar*) glGetString(GL_RENDERER);
  lua_pushstring(L, VERSION);
  lua_pushstring(L, gl.c_str());
  lua_pushstring(L, gpu);
  return 3;
}

int VM::navigate(lua_State* L) {
  WindowContext* ctx = (WindowContext*) lua_topointer(L, lua_upvalueindex(1));
  ctx->setURL(luaL_checkstring(L, 1));
  return 0;
}

int VM::setClearColor(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  vm->clearColor.x = luaL_checknumber(L, 1);
  vm->clearColor.y = luaL_checknumber(L, 2);
  vm->clearColor.z = luaL_checknumber(L, 3);
  vm->clearColor.w = luaL_checknumber(L, 4);
  return 0;
}

int VM::showTooltip(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const std::string message = luaL_checkstring(L, 1);
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  const GLfloat ox = luaL_optnumber(L, 5, 0);
  const GLfloat oy = luaL_optnumber(L, 6, 25.5);
  glm::vec4 projected = vm->camera.getProjection() * vm->camera.getView() * glm::vec4(x, y, z, 1.0);
  vm->tooltips.push_back({
    message,
    glm::vec2(ox, -oy),
    (
      glm::vec2(projected.x, projected.y) / projected.w
    ) * glm::vec2(0.5, -0.5) + glm::vec2(0.5, 0.5)
  });
  if (vm->tooltips.size() > 16) {
    vm->tooltips.pop_front();
  }
  return 0;
}

int VM::clamp(lua_State* L) {
  const GLfloat v = luaL_checknumber(L, 1);
  const GLfloat minv = luaL_checknumber(L, 2);
  const GLfloat maxv = luaL_checknumber(L, 3);
  lua_pushnumber(L, glm::clamp(v, minv, maxv));
  return 1;
}

int VM::cross(lua_State* L) {
  const GLfloat x1 = luaL_checknumber(L, 1);
  const GLfloat y1 = luaL_checknumber(L, 2);
  const GLfloat z1 = luaL_checknumber(L, 3);
  const GLfloat x2 = luaL_checknumber(L, 4);
  const GLfloat y2 = luaL_checknumber(L, 5);
  const GLfloat z2 = luaL_checknumber(L, 6);
  const glm::vec3 r = glm::cross(glm::vec3(x1, y1, z1), glm::vec3(x2, y2, z2));
  lua_pushnumber(L, r.x);
  lua_pushnumber(L, r.y);
  lua_pushnumber(L, r.z);
  return 3;
}

int VM::dot(lua_State* L) {
  const GLfloat x1 = luaL_checknumber(L, 1);
  const GLfloat y1 = luaL_checknumber(L, 2);
  const GLfloat z1 = luaL_checknumber(L, 3);
  const GLfloat x2 = luaL_checknumber(L, 4);
  const GLfloat y2 = luaL_checknumber(L, 5);
  const GLfloat z2 = luaL_checknumber(L, 6);
  lua_pushnumber(L, glm::dot(glm::vec3(x1, y1, z1), glm::vec3(x2, y2, z2)));
  return 1;
}

int VM::hsv(lua_State* L) {
  const GLfloat h = luaL_checknumber(L, 1);
  const GLfloat s = luaL_checknumber(L, 2);
  const GLfloat v = luaL_checknumber(L, 3);
  GLfloat r, g, b;
  ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
  lua_pushnumber(L, r);
  lua_pushnumber(L, g);
  lua_pushnumber(L, b);
  return 3;
}

int VM::normalize(lua_State* L) {
  const GLfloat x = luaL_checknumber(L, 1);
  const GLfloat y = luaL_checknumber(L, 2);
  const GLfloat z = luaL_checknumber(L, 3);
  const glm::vec3 r = glm::normalize(glm::vec3(x, y, z));
  lua_pushnumber(L, r.x);
  lua_pushnumber(L, r.y);
  lua_pushnumber(L, r.z);
  return 3;
}

int VM::lerp(lua_State* L) {
  const GLfloat a = luaL_checknumber(L, 1);
  const GLfloat b = luaL_checknumber(L, 2);
  const GLfloat d = luaL_checknumber(L, 3);
  lua_pushnumber(L, a * (1 - d) + b * d);
  return 1;
}

int VM::spherical(lua_State* L) {
  const GLfloat phi = luaL_checknumber(L, 1);
  const GLfloat theta = luaL_checknumber(L, 2);
  const GLfloat radius = luaL_checknumber(L, 3);
  GLfloat sinPhiRadius = sin(phi) * radius;
  lua_pushnumber(L, sinPhiRadius * sin(theta));
  lua_pushnumber(L, cos(phi) * radius);
  lua_pushnumber(L, sinPhiRadius * cos(theta));
  return 3;
}

int VM::srgb(lua_State* L) {
  const GLfloat r = luaL_checknumber(L, 1);
  const GLfloat g = luaL_checknumber(L, 2);
  const GLfloat b = luaL_checknumber(L, 3);
  auto convert = [](GLfloat c) { return ( c < 0.0031308 ) ? c * 12.92 : 1.055 * ( pow( c, 0.41666 ) ) - 0.055; };
  lua_pushnumber(L, convert(r));
  lua_pushnumber(L, convert(g));
  lua_pushnumber(L, convert(b));
  return 3;
}

int VM::camera_lookAt(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GLfloat x = luaL_checknumber(L, 1);
  const GLfloat y = luaL_checknumber(L, 2);
  const GLfloat z = luaL_checknumber(L, 3);
  vm->camera.lookAt(glm::vec3(x, y, z));
  return 0;
}

int VM::camera_getFov(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  lua_pushnumber(L, vm->camera.getFov());
  return 1;
}

int VM::camera_setFov(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GLfloat value = luaL_checknumber(L, 1);
  vm->camera.setFov(value);
  return 0;
}

int VM::camera_getPosition(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const glm::vec3& position = vm->camera.getPosition();
  lua_pushnumber(L, position.x);
  lua_pushnumber(L, position.y);
  lua_pushnumber(L, position.z);
  return 3;
}

int VM::camera_setPosition(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GLfloat x = luaL_checknumber(L, 1);
  const GLfloat y = luaL_checknumber(L, 2);
  const GLfloat z = luaL_checknumber(L, 3);
  vm->camera.setPosition(glm::vec3(x, y, z));
  return 0;
}

int VM::keyboard(lua_State* L) {
  WindowContext* ctx = (WindowContext*) lua_topointer(L, lua_upvalueindex(1));
  std::string key = luaL_checkstring(L, 1);
  for (auto& c : key) c = toupper(c);
  bool pressed;
  if (ImGui::GetIO().WantCaptureKeyboard) {
    pressed = false;
  } else if (key == "SHIFT") {
    pressed = glfwGetKey(ctx->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(ctx->window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
  } else {
    pressed = glfwGetKey(ctx->window, int(key.at(0))) == GLFW_PRESS;
  }
  lua_pushboolean(L, pressed);
  return 1;
}

int VM::mouse_cursor(lua_State* L) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return 0;
  }
  const int cursors[] = {
    ImGuiMouseCursor_Arrow,
    ImGuiMouseCursor_Hand,
    ImGuiMouseCursor_None
  };
  const char* cursorNames[] = {
    "arrow",
    "hand",
    "none",
    nullptr
  };
  const int cursor = luaL_checkoption(L, 1, nullptr, cursorNames);
  ImGui::SetMouseCursor(cursors[cursor]);
  return 0;
}

int VM::mouse_lock(lua_State* L) {
  WindowContext* ctx = (WindowContext*) lua_topointer(L, lua_upvalueindex(1));
  ctx->lockMouse();
  return 0;
}

int VM::mouse_unlock(lua_State* L) {
  WindowContext* ctx = (WindowContext*) lua_topointer(L, lua_upvalueindex(1));
  ctx->unlockMouse();
  return 0;
}

int VM::physics_getContacts(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GeometryColliderShape shape = (GeometryColliderShape) luaL_checkoption(L, 1, nullptr, GeometryColliderShapeNames);
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  const GLfloat sx = luaL_checknumber(L, 5);
  const GLfloat sy = luaL_checknumber(L, 6);
  const GLfloat sz = luaL_checknumber(L, 7);
  const GLubyte mask = luaL_optinteger(L, 8, 0);
  btCollisionObject* target = vm->physics.getTempCollider(shape, glm::vec3(x, y, z), glm::vec3(sx, sy, sz));
  glm::vec3 contacts = vm->physics.getAccumulatedContacts(target, mask);
  lua_pushnumber(L, contacts.x);
  lua_pushnumber(L, contacts.y);
  lua_pushnumber(L, contacts.z);
  return 3;
}

int VM::physics_getContactIds(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GeometryColliderShape shape = (GeometryColliderShape) luaL_checkoption(L, 1, nullptr, GeometryColliderShapeNames);
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  const GLfloat sx = luaL_checknumber(L, 5);
  const GLfloat sy = luaL_checknumber(L, 6);
  const GLfloat sz = luaL_checknumber(L, 7);
  const GLubyte mask = luaL_optinteger(L, 8, 0);
  btCollisionObject* target = vm->physics.getTempCollider(shape, glm::vec3(x, y, z), glm::vec3(sx, sy, sz));
  std::vector<GLuint> contacts = vm->physics.getContactIds(target, mask);
  size_t count = contacts.size();
  if (!lua_checkstack(L, count)) {
    lua_pushliteral(L, "Physics.getContactIds - not enough space in stack");
    lua_error(L);
  }
  for (const auto c : contacts) {
    lua_pushinteger(L, c);
  }
  return count;
}

int VM::physics_setGravity(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GLfloat x = luaL_checknumber(L, 1);
  const GLfloat y = luaL_checknumber(L, 2);
  const GLfloat z = luaL_checknumber(L, 3);
  vm->physics.setGravity(glm::vec3(x, y, z));
  return 0;
}

int VM::raycaster_setFromCamera(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GLfloat x = luaL_checknumber(L, 1);
  const GLfloat y = luaL_checknumber(L, 2);
  vm->raycaster.setFromCamera(&vm->camera, glm::vec2(x, y));
  return 0;
}

int VM::raycaster_intersect(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  int count = lua_gettop(L);
  vm->raycaster.init();
  for (int i = 1; i <= count; i++) {
    Mesh** mesh = (Mesh**) luaL_testudata(L, i, "Mesh");
    if (mesh != nullptr) {
      vm->raycaster.intersect((*mesh)->getId(), (*mesh)->getBounds(), (*mesh)->getGeometry(), (*mesh)->getTransform());
    } else {
      Voxels* voxels = *((Voxels**) luaL_checkudata(L, i, "Voxels"));
      for (const auto& [k,chunk] : voxels->getChunks()) {
        vm->raycaster.intersect(voxels->getId(), chunk->getBounds(), chunk, chunk->getTransform());
      }
    }
  }
  if (vm->raycaster.result.id == 0) {
    return 0;
  }
  lua_pushinteger(L, vm->raycaster.result.id);
  return 1;
}

int VM::raycaster_getResult(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  lua_pushnumber(L, vm->raycaster.result.position.x);
  lua_pushnumber(L, vm->raycaster.result.position.y);
  lua_pushnumber(L, vm->raycaster.result.position.z);
  lua_pushnumber(L, vm->raycaster.result.normal.x);
  lua_pushnumber(L, vm->raycaster.result.normal.y);
  lua_pushnumber(L, vm->raycaster.result.normal.z);
  return 6;
}

int VM::raycaster_getRay(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  lua_pushnumber(L, vm->raycaster.ray.origin.x);
  lua_pushnumber(L, vm->raycaster.ray.origin.y);
  lua_pushnumber(L, vm->raycaster.ray.origin.z);
  lua_pushnumber(L, vm->raycaster.ray.direction.x);
  lua_pushnumber(L, vm->raycaster.ray.direction.y);
  lua_pushnumber(L, vm->raycaster.ray.direction.z);
  return 6;
}

int VM::environment_isReady(lua_State* L) {
  Environment* environment = *((Environment**) luaL_checkudata(L, 1, "Environment"));
  lua_pushboolean(L, environment->getCubemap()->get() != 0);
  return 1;
}

int VM::environment_free(lua_State* L) {
  delete *((Environment**) luaL_checkudata(L, 1, "Environment"));
  return 0;
}

int VM::environment_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const std::string url = luaL_checkstring(L, 1);
  if (!HTTP::isValidURL(url)) {
    lua_pushliteral(L, "Environment - invalid url");
    lua_error(L);
  }
  *((Environment**) lua_newuserdata(L, sizeof(Environment*))) = new Environment(&vm->cubemapbuffer, vm->http->request(url));
  if (luaL_newmetatable(L, "Environment")) {
    static const luaL_Reg functions[] = {
      {"isReady", environment_isReady},
      {"__gc", environment_free},
      {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::framebuffer_bind(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Framebuffer* framebuffer = *((Framebuffer**) luaL_checkudata(L, 1, "Framebuffer"));
  const GLint x = luaL_checkinteger(L, 2);
  const GLint y = luaL_checkinteger(L, 3);
  if (!framebuffer->setSize(x, y)) {
    lua_pushliteral(L, "Framebuffer::bind - framebuffer is not complete");
    lua_error(L);
  }
  framebuffer->bind();
  glViewport(0, 0, x, y);
  vm->camera.setAspect((GLfloat) x / (GLfloat) y);
  return 0;
}

int VM::framebuffer_blit(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Framebuffer* framebuffer = *((Framebuffer**) luaL_checkudata(L, 1, "Framebuffer"));
  Framebuffer* target = nullptr;
  if (lua_gettop(L) > 1) {
    target = *((Framebuffer**) luaL_checkudata(L, 2, "Framebuffer"));
  }
  framebuffer->blit(
    target,
    target != nullptr ? target->width : vm->window->resolution.x,
    target != nullptr ? target->height : vm->window->resolution.y
  );
  return 0;
}

int VM::framebuffer_clear(lua_State* L) {
  Framebuffer* framebuffer = *((Framebuffer**) luaL_checkudata(L, 1, "Framebuffer"));
  if (!framebuffer->isBinded()) {
    lua_pushliteral(L, "Framebuffer::clear - framebuffer is not binded");
    lua_error(L);
  }
  framebuffer->clear();
  return 0;
}

int VM::framebuffer_setTextureClearColor(lua_State* L) {
  Framebuffer* framebuffer = *((Framebuffer**) luaL_checkudata(L, 1, "Framebuffer"));
  const GLint index = luaL_checkinteger(L, 2);
  const GLfloat r = luaL_checknumber(L, 3);
  const GLfloat g = luaL_checknumber(L, 4);
  const GLfloat b = luaL_checknumber(L, 5);
  const GLfloat a = luaL_checknumber(L, 6);
  if (!framebuffer->setTextureClearColor(index, glm::vec4(r, g, b, a))) {
    lua_pushliteral(L, "Framebuffer::setTextureClearColor - texture index out of bounds");
    lua_error(L);
  }
  return 0;
}

int VM::framebuffer_setTextureData(lua_State* L) {
  Framebuffer* framebuffer = *((Framebuffer**) luaL_checkudata(L, 1, "Framebuffer"));
  const GLint index = luaL_checkinteger(L, 2);
  const GLsizei textureSize = framebuffer->width * framebuffer->height * 4;
  GLsizei count = lua_gettop(L) - 2;
  if (count != textureSize) {
    std::string message = "Framebuffer::setTextureData - data size (" + std::to_string(count) + ") doesn't match texture size (" + std::to_string(textureSize) + ")";
    lua_pushstring(L, message.c_str());
    lua_error(L);
  }
  std::vector<GLfloat> data;
  data.reserve(count);
  for (int i = 1; i <= count; i++) {
    data.push_back(luaL_checknumber(L, i + 2));
  }
  if (!framebuffer->setTextureData(index, data.data())) {
    lua_pushliteral(L, "Framebuffer::setTextureData - texture index out of bounds");
    lua_error(L);
  }
  return 0;
}

int VM::framebuffer_unbind(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  glViewport(0, 0, vm->window->resolution.x, vm->window->resolution.y);
  vm->camera.setAspect(vm->window->resolution.x / vm->window->resolution.y);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  return 0;
}

int VM::framebuffer_free(lua_State* L) {
  delete *((Framebuffer**) luaL_checkudata(L, 1, "Framebuffer"));
  return 0;
}

int VM::framebuffer_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const GLint textures = glm::max((GLint) luaL_optinteger(L, 1, 1), 1);
  const bool depth = lua_toboolean(L, 2);
  const GLint samples = glm::max((GLint) luaL_optinteger(L, 3, 0), 0);
  *((Framebuffer**) lua_newuserdata(L, sizeof(Framebuffer*))) = new Framebuffer(
    textures,
    depth,
    samples
  );
  if (luaL_newmetatable(L, "Framebuffer")) {
    static const luaL_Reg functions[] = {
      {"bind", framebuffer_bind},
      {"blit", framebuffer_blit},
      {"clear", framebuffer_clear},
      {"setTextureClearColor", framebuffer_setTextureClearColor},
      {"setTextureData", framebuffer_setTextureData},
      {"unbind", framebuffer_unbind},
      {"__gc", framebuffer_free},
      {nullptr, nullptr}
    };
    lua_pushlightuserdata(L, vm);
    luaL_setfuncs(L, functions, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::geometry_setColliders(lua_State* L) {
  Geometry* geometry = *((Geometry**) luaL_checkudata(L, 1, "Geometry"));
  int count = lua_gettop(L) - 1;
  if (count == 0 || count % 7 != 0) {
    lua_pushliteral(L, "Geometry::setColliders - data must be multiple of 7 (collider,  x, y, z,  sx, sy, sz)");
    lua_error(L);
  }
  GLfloat position[3];
  GLfloat scale[3];
  geometry->colliders.clear();
  geometry->colliders.reserve(count);
  for (int i = 1; i <= count; i += 7) {
    const GeometryColliderShape shape = (GeometryColliderShape) luaL_checkoption(L, 1, nullptr, GeometryColliderShapeNames);
    for (int j = 0; j < 3; j++) {
      position[j] = luaL_checknumber(L, i + 2 + j);
    }
    for (int j = 0; j < 3; j++) {
      scale[j] = luaL_checknumber(L, i + 5 + j);
    }
    geometry->colliders.push_back({
      shape,
      *(glm::vec3*) position,
      *(glm::vec3*) scale
    });
  }
  geometry->needsUpdate = true;
  return 0;
}

int VM::geometry_setIndex(lua_State* L) {
  Geometry* geometry = *((Geometry**) luaL_checkudata(L, 1, "Geometry"));
  int count = lua_gettop(L) - 1;
  if (count == 0 || count % 3 != 0) {
    lua_pushliteral(L, "Geometry::setIndex - index data must be multiple of 3");
    lua_error(L);
  }
  geometry->index.clear();
  geometry->index.reserve(count);
  for (int i = 1; i <= count; i++) {
    geometry->index.push_back(luaL_checkinteger(L, i + 1));
  }
  geometry->needsUpdate = true;
  return 0;
}

int VM::geometry_setVertices(lua_State* L) {
  Geometry* geometry = *((Geometry**) luaL_checkudata(L, 1, "Geometry"));
  int count = lua_gettop(L) - 1;
  if (count == 0 || count % 11 != 0) {
    lua_pushliteral(L, "Geometry::setVertices - vertex data must be multiple of 11 (x, y, z,  nx, ny, nz,  u, v,  r, g, b)");
    lua_error(L);
  }
  geometry->vertices.clear();
  geometry->vertices.reserve(count);
  GLfloat vertex[11];
  for (int i = 1; i <= count; i += 11) {
    for (int j = 0; j < 11; j++) {
      vertex[j] = luaL_checknumber(L, i + j + 1);
    }
    geometry->vertices.push_back(*(GeometryVertex*) vertex);
  }
  geometry->needsUpdate = true;
  return 0;
}

int VM::geometry_free(lua_State* L) {
  Geometry::gc(
    *((Geometry**) luaL_checkudata(L, 1, "Geometry"))
  );
  return 0;
}

int VM::geometry_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  *((Geometry**) lua_newuserdata(L, sizeof(Geometry*))) = new Geometry();
  if (luaL_newmetatable(L, "Geometry")) {
    static const luaL_Reg functions[] = {
      {"setColliders", geometry_setColliders},
      {"setIndex", geometry_setIndex},
      {"setVertices", geometry_setVertices},
      {"__gc", geometry_free},
      {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::HTTP_isReady(lua_State* L) {
  HTTPRequest* request = *((HTTPRequest**) luaL_checkudata(L, 1, "HTTP"));
  lua_pushboolean(L, (
    request->isReady && request->response.status >= 200 && request->response.status < 400 && request->response.size > 0
  ));
  return 1;
}

int VM::HTTP_getResponse(lua_State* L) {
  HTTPRequest* request = *((HTTPRequest**) luaL_checkudata(L, 1, "HTTP"));
  if (!(request->isReady && request->response.status >= 200 && request->response.status < 400 && request->response.size > 0)) {
    lua_pushliteral(L, "HTTP::getResponse - request is not ready");
    lua_error(L);
  }
  lua_pushstring(L, (char*) request->response.data);
  return 1;
}

int VM::HTTP_free(lua_State* L) {
  delete *((HTTPRequest**) luaL_checkudata(L, 1, "HTTP"));
  return 0;
}

int VM::HTTP_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const std::string url = luaL_checkstring(L, 1);
  if (!HTTP::isValidURL(url)) {
    lua_pushliteral(L, "HTTP - invalid URL");
    lua_error(L);
  }
  std::string body;
  if (lua_gettop(L) > 1) {
    body = luaL_checkstring(L, 2);
  }
  *((HTTPRequest**) lua_newuserdata(L, sizeof(HTTPRequest*))) = vm->http->request(url, body);
  if (luaL_newmetatable(L, "HTTP")) {
    static const luaL_Reg functions[] = {
      {"isReady", HTTP_isReady},
      {"getResponse", HTTP_getResponse},
      {"__gc", HTTP_free},
      {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::image_isReady(lua_State* L) {
  Image* image = *((Image**) luaL_checkudata(L, 1, "Image"));
  lua_pushboolean(L, image->get() != 0);
  return 1;
}

int VM::image_getSize(lua_State* L) {
  Image* image = *((Image**) luaL_checkudata(L, 1, "Image"));
  const glm::vec2 size = image->getSize();
  lua_pushnumber(L, size.x);
  lua_pushnumber(L, size.y);
  return 2;
}

int VM::image_free(lua_State* L) {
  Texture::gc(
    *((Image**) luaL_checkudata(L, 1, "Image"))
  );
  return 0;
}

int VM::image_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const std::string url = luaL_checkstring(L, 1);
  if (!HTTP::isValidURL(url)) {
    lua_pushliteral(L, "Image - invalid url");
    lua_error(L);
  }
  static const char* ImageEncodingNames[] = {
    "srgb",
    "linear",
    nullptr
  };
  const ImageEncoding encoding = (ImageEncoding) luaL_checkoption(L, 2, "srgb", ImageEncodingNames);
  *((Image**) lua_newuserdata(L, sizeof(Image*))) = new Image(encoding, vm->http->request(url));
  if (luaL_newmetatable(L, "Image")) {
    static const luaL_Reg functions[] = {
      {"isReady", image_isReady},
      {"getSize", image_getSize},
      {"__gc", image_free},
      {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::mesh_getId(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  lua_pushnumber(L, mesh->getId());
  return 1;
}

int VM::mesh_getPosition(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const glm::vec3& position = mesh->getPosition();
  lua_pushnumber(L, position.x);
  lua_pushnumber(L, position.y);
  lua_pushnumber(L, position.z);
  return 3;
}

int VM::mesh_setPosition(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  mesh->setPosition(glm::vec3(x, y, z));
  btRigidBody* body = mesh->getBody();
  if (body != nullptr) {
    vm->physics.setBodyPosition(body, mesh->getPosition());
  }
  return 0;
}

int VM::mesh_getScale(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const glm::vec3& scale = mesh->getScale();
  lua_pushnumber(L, scale.x);
  lua_pushnumber(L, scale.y);
  lua_pushnumber(L, scale.z);
  return 3;
}

int VM::mesh_setScale(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  mesh->setScale(glm::vec3(x, y, z));
  return 0;
}

int VM::mesh_lookAt(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  mesh->lookAt(glm::vec3(x, y, z));
  btRigidBody* body = mesh->getBody();
  if (body != nullptr) {
    vm->physics.setBodyRotation(body, mesh->getRotation());
  }
  return 0;
}

int VM::mesh_getFlags(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  lua_pushinteger(L, mesh->getFlags());
  return 1;
}

int VM::mesh_setFlags(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLubyte flags = luaL_checkinteger(L, 2);
  mesh->setFlags(flags);
  return 0;
}

int VM::mesh_getFrustumCulling(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  lua_pushboolean(L, mesh->getFrustumCulling());
  return 1;
}

int VM::mesh_setFrustumCulling(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const bool enabled = lua_toboolean(L, 2);
  mesh->setFrustumCulling(enabled);
  return 0;
}

int VM::mesh_enablePhysics(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat mass = glm::max((GLfloat) luaL_optnumber(L, 2, 0), (GLfloat) 0.0);
  const bool isAlwaysActive = lua_toboolean (L, 3);
  const bool isKinematic = lua_toboolean(L, 4);
  if (mesh->getBody() != nullptr) {
    lua_pushliteral(L, "Mesh::enablePhysics - mesh physics already enabled");
    lua_error(L);
  }
  vm->physics.addBody(mesh, mass, isAlwaysActive, isKinematic);
  return 0;
}

int VM::mesh_disablePhysics(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::disablePhysics - mesh physics already disabled");
    lua_error(L);
  }
  vm->physics.removeBody(body);
  return 0;
}

int VM::mesh_applyForce(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::applyForce - mesh physics disabled");
    lua_error(L);
  }
  body->applyCentralForce(btVector3(x, y, z));
  body->activate();
  return 0;
}

int VM::mesh_applyImpulse(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::applyImpulse - mesh physics disabled");
    lua_error(L);
  }
  body->applyCentralImpulse(btVector3(x, y, z));
  body->activate();
  return 0;
}

int VM::mesh_setDamping(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat linear = glm::max((GLfloat) luaL_checknumber(L, 2), (GLfloat) 0.0);
  const GLfloat angular = glm::max((GLfloat) luaL_checknumber(L, 3), (GLfloat) 0.0);
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::setDamping - mesh physics disabled");
    lua_error(L);
  }
  body->setDamping(linear, angular);
  body->activate();
  return 0;
}

int VM::mesh_getAngularVelocity(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::getAngularVelocity - mesh physics disabled");
    lua_error(L);
  }
  const btVector3& v = body->getAngularVelocity();
  lua_pushnumber(L, v.x());
  lua_pushnumber(L, v.y());
  lua_pushnumber(L, v.z());
  return 3;
}

int VM::mesh_setAngularVelocity(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::setAngularVelocity - mesh physics disabled");
    lua_error(L);
  }
  body->setAngularVelocity(btVector3(x, y, z));
  body->activate();
  return 0;
}

int VM::mesh_getLinearVelocity(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::getLinearVelocity - mesh physics disabled");
    lua_error(L);
  }
  const btVector3& v = body->getLinearVelocity();
  lua_pushnumber(L, v.x());
  lua_pushnumber(L, v.y());
  lua_pushnumber(L, v.z());
  return 3;
}

int VM::mesh_setLinearVelocity(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::setLinearVelocity - mesh physics disabled");
    lua_error(L);
  }
  body->setLinearVelocity(btVector3(x, y, z));
  body->activate();
  return 0;
}

int VM::mesh_getContacts(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLubyte mask = luaL_optinteger(L, 2, 0);
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::getContacts - mesh physics disabled");
    lua_error(L);
  }
  glm::vec3 contacts = vm->physics.getAccumulatedContacts((btCollisionObject*) body, mask);
  lua_pushnumber(L, contacts.x);
  lua_pushnumber(L, contacts.y);
  lua_pushnumber(L, contacts.z);
  return 3;
}

int VM::mesh_getContactIds(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLubyte mask = luaL_optinteger(L, 2, 0);
  btRigidBody* body = mesh->getBody();
  if (body == nullptr) {
    lua_pushliteral(L, "Mesh::getContactIds - mesh physics disabled");
    lua_error(L);
  }
  std::vector<GLuint> contacts = vm->physics.getContactIds((btCollisionObject*) body, mask);
  size_t count = contacts.size();
  if (!lua_checkstack(L, count)) {
    lua_pushliteral(L, "Mesh::getContactIds - not enough space in stack");
    lua_error(L);
  }
  for (const auto c : contacts) {
    lua_pushinteger(L, c);
  }
  return count;
}

int VM::mesh_uniformInt(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const std::string name = luaL_checkstring(L, 2);
  const GLint value = luaL_checkinteger(L, 3);
  mesh->setUniformInt(name, value);
  return 0;
}

int VM::mesh_uniformFloat(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const std::string name = luaL_checkstring(L, 2);
  const GLfloat value = luaL_checknumber(L, 3);
  mesh->setUniformFloat(name, value);
  return 0;
}

int VM::mesh_uniformTexture(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const std::string name = luaL_checkstring(L, 2);
  Texture* texture = getTexture(L, 3);
  mesh->setUniformTexture(name, texture);
  return 0;
}

int VM::mesh_uniformVec2(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const std::string name = luaL_checkstring(L, 2);
  const GLfloat x = luaL_checknumber(L, 3);
  const GLfloat y = luaL_checknumber(L, 4);
  mesh->setUniformVec2(name, glm::vec2(x, y));
  return 0;
}

int VM::mesh_uniformVec3(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const std::string name = luaL_checkstring(L, 2);
  const GLfloat x = luaL_checknumber(L, 3);
  const GLfloat y = luaL_checknumber(L, 4);
  const GLfloat z = luaL_checknumber(L, 5);
  mesh->setUniformVec3(name, glm::vec3(x, y, z));
  return 0;
}

int VM::mesh_uniformVec4(lua_State* L) {
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const std::string name = luaL_checkstring(L, 2);
  const GLfloat x = luaL_checknumber(L, 3);
  const GLfloat y = luaL_checknumber(L, 4);
  const GLfloat z = luaL_checknumber(L, 5);
  const GLfloat w = luaL_checknumber(L, 6);
  mesh->setUniformVec4(name, glm::vec4(x, y, z, w));
  return 0;
}

int VM::mesh_render(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  const GLsizei instances = glm::max((GLsizei) luaL_optinteger(L, 2, 0), (GLsizei) 0);
  if (mesh->getShader() == nullptr) {
    lua_pushliteral(L, "Mesh::render - No shader attached");
    lua_error(L);
  }
  mesh->render(&vm->camera, instances);
  return 0;
}

int VM::mesh_free(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Mesh* mesh = *((Mesh**) luaL_checkudata(L, 1, "Mesh"));
  btRigidBody* body = mesh->getBody();
  if (body != nullptr) {
    vm->physics.removeBody(body);
  }
  Geometry::gc(mesh->getGeometry());
  vm->shader_gc(mesh->getShader());
  delete mesh;
  return 0;
}

int VM::mesh_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Geometry* geometries[] = {
    &vm->box,
    &vm->plane,
    &vm->sphere
  };
  const char* geometryNames[] = {
    "box",
    "plane",
    "sphere",
    nullptr
  };
  Geometry* geometry;
  if (lua_type(L, 1) == LUA_TSTRING) {
    geometry = geometries[luaL_checkoption(L, 1, nullptr, geometryNames)];
  } else {
    geometry = *((Geometry**) luaL_checkudata(L, 1, "Geometry"));
  }
  geometry->refs++;
  Shader* shader = nullptr;
  Shader** shaderPointer = (Shader**) luaL_testudata(L, 2, "Shader");
  if (shaderPointer != nullptr) {
    shader = *shaderPointer;
    shader->refs++;
  }
  *((Mesh**) lua_newuserdata(L, sizeof(Mesh*))) = new Mesh(geometry, shader);
  if (luaL_newmetatable(L, "Mesh")) {
    static const luaL_Reg functions[] = {
      {"getId", mesh_getId},
      {"getPosition", mesh_getPosition},
      {"setPosition", mesh_setPosition},
      {"getScale", mesh_getScale},
      {"setScale", mesh_setScale},
      {"lookAt", mesh_lookAt},
      {"getFlags", mesh_getFlags},
      {"setFlags", mesh_setFlags},
      {"getFrustumCulling", mesh_getFrustumCulling},
      {"setFrustumCulling", mesh_setFrustumCulling},
      {"enablePhysics", mesh_enablePhysics},
      {"disablePhysics", mesh_disablePhysics},
      {"applyForce", mesh_applyForce},
      {"applyImpulse", mesh_applyImpulse},
      {"setDamping", mesh_setDamping},
      {"getAngularVelocity", mesh_getAngularVelocity},
      {"setAngularVelocity", mesh_setAngularVelocity},
      {"getLinearVelocity", mesh_getLinearVelocity},
      {"setLinearVelocity", mesh_setLinearVelocity},
      {"getContacts", mesh_getContacts},
      {"getContactIds", mesh_getContactIds},
      {"uniformInt", mesh_uniformInt},
      {"uniformFloat", mesh_uniformFloat},
      {"uniformTexture", mesh_uniformTexture},
      {"uniformVec2", mesh_uniformVec2},
      {"uniformVec3", mesh_uniformVec3},
      {"uniformVec4", mesh_uniformVec4},
      {"render", mesh_render},
      {"__gc", mesh_free},
      {nullptr, nullptr}
    };
    lua_pushlightuserdata(L, vm);
    luaL_setfuncs(L, functions, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::noise_get2D(lua_State* L) {
  FastNoise::SmartNode<>* noise = *((FastNoise::SmartNode<>**) luaL_checkudata(L, 1, "Noise"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLint seed = luaL_checkinteger(L, 4);
  lua_pushnumber(L, noise->get()->GenSingle2D(x, y, seed));
  return 1;
}

int VM::noise_get3D(lua_State* L) {
  FastNoise::SmartNode<>* noise = *((FastNoise::SmartNode<>**) luaL_checkudata(L, 1, "Noise"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  const GLint seed = luaL_checkinteger(L, 5);
  lua_pushnumber(L, noise->get()->GenSingle3D(x, y, z, seed));
  return 1;
}

int VM::noise_free(lua_State* L) {
  delete *((FastNoise::SmartNode<>**) luaL_checkudata(L, 1, "Noise"));
  return 0;
}

int VM::noise_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  FastNoise::SmartNode<> node = FastNoise::NewFromEncodedNodeTree(luaL_checkstring(L, 1));
  if (!node) {
    lua_pushliteral(L, "Noise - invalid node tree");
    lua_error(L);
  }
  *((FastNoise::SmartNode<>**) lua_newuserdata(L, sizeof(FastNoise::SmartNode<>*))) = new FastNoise::SmartNode<>(node);
  if (luaL_newmetatable(L, "Noise")) {
    static const luaL_Reg functions[] = {
      {"get2D", noise_get2D},
      {"get3D", noise_get3D},
      {"__gc", noise_free},
      {nullptr, nullptr}
    };
    luaL_setfuncs(L, functions, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::sfx_isReady(lua_State* L) {
  SFX* sfx = *((SFX**) luaL_checkudata(L, 1, "SFX"));
  lua_pushboolean(L, sfx->isReady());
  return 1;
}

int VM::sfx_play(lua_State* L) {
  SFX* sfx = *((SFX**) luaL_checkudata(L, 1, "SFX"));
  const GLfloat x = luaL_checknumber(L, 2);
  const GLfloat y = luaL_checknumber(L, 3);
  const GLfloat z = luaL_checknumber(L, 4);
  sfx->play(glm::vec3(x, y, z));
  return 0;
}

int VM::sfx_free(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  SFX* sfx = *((SFX**) luaL_checkudata(L, 1, "SFX"));
  std::erase_if(vm->sfx, [&sfx](const SFX* s) {
    return s->id == sfx->id;
  });
  delete sfx;
  return 0;
}

int VM::sfx_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const std::string url = luaL_checkstring(L, 1);
  if (!HTTP::isValidURL(url)) {
    lua_pushliteral(L, "SFX - invalid url");
    lua_error(L);
  }
  SFX* sfx = new SFX(vm->http->request(url));
  vm->sfx.push_back(sfx);
  *((SFX**) lua_newuserdata(L, sizeof(SFX*))) = sfx;
  if (luaL_newmetatable(L, "SFX")) {
    static const luaL_Reg functions[] = {
      {"isReady", sfx_isReady},
      {"play", sfx_play},
      {"__gc", sfx_free},
      {nullptr, nullptr}
    };
    lua_pushlightuserdata(L, vm);
    luaL_setfuncs(L, functions, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::shader_getBlend(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  lua_pushboolean(L, shader->getBlend());
  return 1;
}

int VM::shader_setBlend(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const bool enabled = lua_toboolean(L, 2);
  shader->setBlend(enabled);
  return 0;
}

int VM::shader_getDepthTest(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  lua_pushboolean(L, shader->getDepthTest());
  return 1;
}

int VM::shader_setDepthTest(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const bool enabled = lua_toboolean(L, 2);
  shader->setDepthTest(enabled);
  return 0;
}

int VM::shader_getFaceCulling(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  lua_pushstring(L, ShaderFaceCullingNames[shader->getFaceCulling()]);
  return 1;
}

int VM::shader_setFaceCulling(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const ShaderFaceCulling mode = (ShaderFaceCulling) luaL_checkoption(L, 2, nullptr, ShaderFaceCullingNames);
  shader->setFaceCulling(mode);
  return 0;
}

int VM::shader_uniformInt(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const char* name = luaL_checkstring(L, 2);
  const GLint value = luaL_checkinteger(L, 3);
  shader->setUniformInt(name, value);
  return 0;
}

int VM::shader_uniformFloat(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const char* name = luaL_checkstring(L, 2);
  const GLfloat value = luaL_checknumber(L, 3);
  shader->setUniformFloat(name, value);
  return 0;
}

int VM::shader_uniformTexture(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const std::string name = luaL_checkstring(L, 2);
  Texture* texture = getTexture(L, 3);
  shader->setUniformTexture(name, texture);
  return 0;
}

int VM::shader_uniformVec2(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const char* name = luaL_checkstring(L, 2);
  const GLfloat x = luaL_checknumber(L, 3);
  const GLfloat y = luaL_checknumber(L, 4);
  shader->setUniformVec2(name, glm::vec2(x, y));
  return 0;
}

int VM::shader_uniformVec3(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const char* name = luaL_checkstring(L, 2);
  const GLfloat x = luaL_checknumber(L, 3);
  const GLfloat y = luaL_checknumber(L, 4);
  const GLfloat z = luaL_checknumber(L, 5);
  shader->setUniformVec3(name, glm::vec3(x, y, z));
  return 0;
}

int VM::shader_uniformVec4(lua_State* L) {
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const char* name = luaL_checkstring(L, 2);
  const GLfloat x = luaL_checknumber(L, 3);
  const GLfloat y = luaL_checknumber(L, 4);
  const GLfloat z = luaL_checknumber(L, 5);
  const GLfloat w = luaL_checknumber(L, 6);
  shader->setUniformVec4(name, glm::vec4(x, y, z, w));
  return 0;
}

int VM::shader_render(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  const GLsizei instances = glm::max((GLsizei) luaL_optinteger(L, 2, 0), (GLsizei) 0);
  shader->setCameraUniforms(&vm->camera);
  shader->setTextureUniforms();
  shader->use();
  vm->plane.draw(instances);
  return 0;
}

int VM::shader_free(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Shader* shader = *((Shader**) luaL_checkudata(L, 1, "Shader"));
  vm->shader_gc(shader);
  return 0;
}

void VM::shader_gc(Shader* shader) {
  if (shader == nullptr) {
    return;
  }
  shader->refs--;
  if (shader->refs > 0) {
    return;
  }
  std::erase_if(shaders, [&shader](const Shader* s) {
    return s->id == shader->id;
  });
  delete shader;
}

int VM::shader_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  const char* vertexSource = luaL_checkstring(L, 1);
  const char* fragmentSource = luaL_checkstring(L, 2);
  Shader* shader = new Shader(vertexSource, fragmentSource);
  if (shader->getError().length()) {
    lua_pushstring(L, shader->getError().c_str());
    delete shader;
    lua_error(L);
  }
  if (shader->hasUniform("brdfMap")) {
    shader->setUniformTexture("brdfMap", &vm->brdf);
  }
  if (shader->hasUniform("irradianceMap")) {
    shader->setUniformTexture("irradianceMap", &vm->irradiance);
  }
  vm->shaders.push_back(shader);
  *((Shader**) lua_newuserdata(L, sizeof(Shader*))) = shader;
  if (luaL_newmetatable(L, "Shader")) {
    static const luaL_Reg functions[] = {
      {"getBlend", shader_getBlend},
      {"setBlend", shader_setBlend},
      {"getDepthTest", shader_getDepthTest},
      {"setDepthTest", shader_setDepthTest},
      {"getFaceCulling", shader_getFaceCulling},
      {"setFaceCulling", shader_setFaceCulling},
      {"uniformInt", shader_uniformInt},
      {"uniformFloat", shader_uniformFloat},
      {"uniformTexture", shader_uniformTexture},
      {"uniformVec2", shader_uniformVec2},
      {"uniformVec3", shader_uniformVec3},
      {"uniformVec4", shader_uniformVec4},
      {"render", shader_render},
      {"__gc", shader_free},
      {nullptr, nullptr}
    };
    lua_pushlightuserdata(L, vm);
    luaL_setfuncs(L, functions, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

int VM::voxels_getId(lua_State* L) {
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  lua_pushnumber(L, voxels->getId());
  return 1;
}

int VM::voxels_get(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  const GLint x = luaL_checkinteger(L, 2);
  const GLint y = luaL_checkinteger(L, 3);
  const GLint z = luaL_checkinteger(L, 4);
  const Voxel voxel = voxels->get(x, y, z);
  lua_pushinteger(L, voxel.type);
  lua_pushinteger(L, voxel.r);
  lua_pushinteger(L, voxel.g);
  lua_pushinteger(L, voxel.b);
  return 4;
}

int VM::voxels_set(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  const GLint x = luaL_checkinteger(L, 2);
  const GLint y = luaL_checkinteger(L, 3);
  const GLint z = luaL_checkinteger(L, 4);
  const GLubyte type = glm::clamp((GLubyte) luaL_checkinteger(L, 5), (GLubyte) 0, (GLubyte) 2);
  const GLubyte r = luaL_optinteger(L, 6, 0);
  const GLubyte g = luaL_optinteger(L, 7, 0);
  const GLubyte b = luaL_optinteger(L, 8, 0);
  voxels->set(x, y, z, (VoxelType) type, r, g, b);
  return 0;
}

int VM::voxels_getFlags(lua_State* L) {
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  lua_pushinteger(L, voxels->getFlags());
  return 1;
}

int VM::voxels_setFlags(lua_State* L) {
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  const GLubyte flags = luaL_checkinteger(L, 2);
  voxels->setFlags(flags);
  return 0;
}

int VM::voxels_enablePhysics(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  voxels->enablePhysics();
  return 0;
}

int VM::voxels_disablePhysics(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  voxels->disablePhysics();
  return 0;
}

int VM::voxels_ground(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  const GLint x = luaL_checkinteger(L, 2);
  const GLint y = luaL_checkinteger(L, 3);
  const GLint z = luaL_checkinteger(L, 4);
  const GLint height = glm::min((GLint) luaL_optnumber(L, 5, 1), (GLint) 0);
  GLint ground;
  if (voxels->ground(x, y, z, height, ground)) {
    lua_pushinteger(L, ground);
    return 1;
  }
  return 0;
}

int VM::voxels_pathfind(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  const GLint fx = luaL_checkinteger(L, 2);
  const GLint fy = luaL_checkinteger(L, 3);
  const GLint fz = luaL_checkinteger(L, 4);
  const GLint tx = luaL_checkinteger(L, 5);
  const GLint ty = luaL_checkinteger(L, 6);
  const GLint tz = luaL_checkinteger(L, 7);
  const GLint height = glm::max((GLint) luaL_optnumber(L, 8, 1), (GLint) 1);
  std::vector<GLint> path = voxels->pathfind(fx, fy, fz, tx, ty, tz, height);
  size_t count = path.size();
  if (!lua_checkstack(L, count)) {
    lua_pushliteral(L, "Voxels::pathfind - not enough space in stack");
    lua_error(L);
  }
  for (const auto p : path) {
    lua_pushinteger(L, p);
  }
  return count;
}

int VM::voxels_render(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  if (voxels->getShader() == nullptr) {
    lua_pushliteral(L, "Voxels::render - No shader attached");
    lua_error(L);
  }
  voxels->render(&vm->camera);
  return 0;
}

int VM::voxels_free(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Voxels* voxels = *((Voxels**) luaL_checkudata(L, 1, "Voxels"));
  vm->shader_gc(voxels->getShader());
  delete voxels;
  return 0;
}

int VM::voxels_new(lua_State* L) {
  VM* vm = (VM*) lua_topointer(L, lua_upvalueindex(1));
  Shader* shader = nullptr;
  Shader** shaderPointer = (Shader**) luaL_testudata(L, 1, "Shader");
  if (shaderPointer != nullptr) {
    shader = *shaderPointer;
    shader->refs++;
  }
  *((Voxels**) lua_newuserdata(L, sizeof(Voxels*))) = new Voxels(&vm->physics, shader);
  if (luaL_newmetatable(L, "Voxels")) {
    static const luaL_Reg functions[] = {
      {"getId", voxels_getId},
      {"get", voxels_get},
      {"set", voxels_set},
      {"getFlags", voxels_getFlags},
      {"setFlags", voxels_setFlags},
      {"enablePhysics", voxels_enablePhysics},
      {"disablePhysics", voxels_disablePhysics},
      {"ground", voxels_ground},
      {"pathfind", voxels_pathfind},
      {"render", voxels_render},
      {"__gc", voxels_free},
      {nullptr, nullptr}
    };
    lua_pushlightuserdata(L, vm);
    luaL_setfuncs(L, functions, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
  }
  lua_setmetatable(L, -2);
  return 1;
}

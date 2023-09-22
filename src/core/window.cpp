#include "window.hpp"
#include <iostream>
#include "lib/icons.hpp"
#include "lib/roboto.hpp"

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#endif

#define STBI_NO_BMP
#define STBI_NO_GIF
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

WindowContext::WindowContext():
  mouse({
    {
      false,
      false,
      false,
      false,
      false,
      false,
    },
    false,
    glm::vec2(-1.0, -1.0),
    false,
    glm::vec2(0.0, 0.0),
    glm::vec2(-1.0, -1.0),
    0.0
  }),
  resolution(1024, 768),
  stateBeforeFullscreen({ 0, 0, 0, 0 }),
  showEditor(false),
  showUI(true)
{
  if (!glfwInit()) {
    const char* description;
    glfwGetError(&description);
    if (description) {
      std::cerr << description << "\n";
    }
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  window = glfwCreateWindow(
    resolution.x,
    resolution.y,
    "Homebrew Navigator",
    nullptr,
    nullptr
  );
  if (!window) {
    const char* description;
    glfwGetError(&description);
    if (description) {
      std::cerr << description << "\n";
    }
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    GLint monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);
    glfwSetWindowPos(
      window,
      monitorX + (mode->width - resolution.x) / 2,
      monitorY + (mode->height - resolution.y) / 2
    );
  }
  #ifdef WIN32
  BOOL dark = true;
  HWND WINhWnd = glfwGetWin32Window(window);
  DwmSetWindowAttribute(
    WINhWnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
    &dark, sizeof(dark)
  );
  #endif

  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glfwSwapInterval(1);

  glfwSetWindowUserPointer(window, (void*) this);
  glfwSetCursorEnterCallback(window, cursorEnterCallback);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetDropCallback(window, dropCallback);
  glfwSetWindowFocusCallback(window, focusCallback);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetScrollCallback(window, scrollCallback);

  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }

  ALCdevice* audioDevice = alcOpenDevice(nullptr);
  if (audioDevice) {
    ALCcontext* audioCtx = alcCreateContext(audioDevice, nullptr);
    if (
      audioCtx == nullptr
      || alcMakeContextCurrent(audioCtx) == ALC_FALSE
      || !alIsExtensionPresent("AL_EXT_STEREO_ANGLES")
    ) {
      if (audioCtx != nullptr) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(audioCtx);
      }
      alcCloseDevice(audioDevice);
    } else {
      alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
      alListener3f(AL_POSITION, 0.0, 0.0, 1.0);
      ALfloat orientation[6] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };
      alListenerfv(AL_ORIENTATION, orientation);
    }
  }

  stbi_set_flip_vertically_on_load(true);

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  float baseFontSize = 16.0;
  float iconFontSize = baseFontSize * 2.0 / 3.0;
  io.Fonts->AddFontFromMemoryCompressedTTF(RobotoCondensed_compressed_data, RobotoCondensed_compressed_size, baseFontSize);
  static const ImWchar icons_ranges[] = { ICON_MIN, ICON_MAX, 0 };
  ImFontConfig icons_config; 
  icons_config.MergeMode = true; 
  icons_config.PixelSnapH = true; 
  icons_config.GlyphMinAdvanceX = iconFontSize;
  io.Fonts->AddFontFromMemoryCompressedTTF(Icons_compressed_data, Icons_compressed_size, iconFontSize, &icons_config, icons_ranges);
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowBorderSize = 0;
  style.FrameRounding = 4.0;
  style.Colors[ImGuiCol_Button] = ImVec4(0.1, 0.1, 0.1, 1.0);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.15, 0.15, 0.15, 1.0);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2, 0.2, 0.2, 1.0);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0, 0.0, 0.0, 1.0);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0, 0.0, 0.0, 0.0);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0, 0.0, 0.0, 0.0);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0, 0.0, 0.0, 0.0);

  markdown.userData = (void*) this;
  markdown.headingFormats[0] = { io.Fonts->AddFontFromMemoryCompressedTTF(RobotoCondensed_compressed_data, RobotoCondensed_compressed_size, baseFontSize * 1.25), false };
  markdown.headingFormats[1] = markdown.headingFormats[0];
  markdown.headingFormats[2] = { NULL, false };
  markdown.linkCallback = [](ImGui::MarkdownLinkCallbackData data) {
    WindowContext* ctx = (WindowContext*) data.userData;
    ctx->setURL(std::string(data.link, data.linkLength));
  };
  markdown.tooltipCallback = [](ImGui::MarkdownTooltipCallbackData data) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
  };
}

bool WindowContext::isFullscreen() {
  return glfwGetWindowMonitor(window) != nullptr;
}

void WindowContext::enterFullScreen() {
  if (isFullscreen()) {
    return;
  }
  glfwGetWindowPos(window, &stateBeforeFullscreen.x, &stateBeforeFullscreen.y);
  glfwGetWindowSize(window, &stateBeforeFullscreen.width, &stateBeforeFullscreen.height);
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
  glfwSwapInterval(1);
  GLdouble x, y;
  glfwGetCursorPos(window, &x, &y);
  cursorPosCallback(window, x, y);
  mouse.movement = glm::vec2(0.0, 0.0);
}

void WindowContext::exitFullscreen() {
  if (!isFullscreen()) {
    return;
  }
  glfwSetWindowMonitor(
    window,
    nullptr,
    stateBeforeFullscreen.x,
    stateBeforeFullscreen.y,
    stateBeforeFullscreen.width,
    stateBeforeFullscreen.height,
    0
  );
  glfwSwapInterval(1);
}

void WindowContext::lockMouse(bool force) {
  if (
    !force
    && (mouse.locked || !(mouse.buttons.primary || mouse.buttons.secondary) || ImGui::GetIO().WantCaptureMouse)
  ) {
    return;
  }
  mouse.locked = true;
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

void WindowContext::unlockMouse() {
  if (!mouse.locked) {
    return;
  }
  mouse.locked = mouse.buttons.primary = mouse.buttons.secondary = false;
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void WindowContext::setURL(const std::string& url, GLubyte flags) {
  std::erase_if(events, [](const WindowEvent& e) {
    return e.type == WINDOW_EVENT_SET_URL;
  });
  events.push_back({
    WINDOW_EVENT_SET_URL,
    url,
    flags
  });
}

void WindowContext::dropCallback(GLFWwindow* window, int count, const char** paths) {
  if (count > 0) {
    WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
    std::string url = "file://";
    url += paths[0];
    ctx->setURL(url);
  }
}

void WindowContext::focusCallback(GLFWwindow* window, int focused) {
  WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
  if (focused == GLFW_FALSE) {
    ctx->unlockMouse();
  }
}

void WindowContext::resizeCallback(GLFWwindow* window, int width, int height) {
  WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
  ctx->resolution = glm::vec2(width, height);
}

void WindowContext::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action != GLFW_PRESS) {
    return;
  }
  WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
  switch (key) {
    case GLFW_KEY_R:
      if (mods & GLFW_MOD_CONTROL) {
        ctx->events.push_back({ WINDOW_EVENT_RELOAD });
      }
      break;
    case GLFW_KEY_S:
      if ((mods & GLFW_MOD_CONTROL) && ctx->showEditor) {
        ctx->events.push_back({ WINDOW_EVENT_SAVE_EDITOR });
      }
      break;
    case GLFW_KEY_E:
      if (mods & GLFW_MOD_CONTROL) {
        ctx->showEditor = !ctx->showEditor;
      }
      break;
    case GLFW_KEY_U:
      if (mods & GLFW_MOD_CONTROL) {
        ctx->showUI = !ctx->showUI;
      }
      break;
    case GLFW_KEY_ENTER:
      if (mods & GLFW_MOD_ALT) {
        if (!ctx->isFullscreen()) {
          ctx->enterFullScreen();
        } else {
          ctx->exitFullscreen();
        }
      }
      break;
    case GLFW_KEY_ESCAPE:
      if (ctx->mouse.locked) {
        ctx->unlockMouse();
      } else if (ctx->isFullscreen()) {
        ctx->exitFullscreen();
      }
      break;
  }
}

void WindowContext::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }
  WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
  switch (button) {
    case GLFW_MOUSE_BUTTON_1:
      ctx->mouse.buttons.primary = action == GLFW_PRESS;
      ctx->mouse.buttons.primaryDown = action == GLFW_PRESS;
      ctx->mouse.buttons.primaryUp = action == GLFW_RELEASE;
      break;
    case GLFW_MOUSE_BUTTON_2:
      ctx->mouse.buttons.secondary = action == GLFW_PRESS;
      ctx->mouse.buttons.secondaryDown = action == GLFW_PRESS;
      ctx->mouse.buttons.secondaryUp = action == GLFW_RELEASE;
      break;
  }
}

void WindowContext::cursorEnterCallback(GLFWwindow* window, int entered) {
  WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
  ctx->mouse.hover = entered;
}

void WindowContext::cursorPosCallback(GLFWwindow* window, double x, double y) {
  WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
  ctx->mouse.position.x = (GLfloat) (x / ctx->resolution.x) * 2.0 - 1.0;
  ctx->mouse.position.y = (GLfloat) -(y / ctx->resolution.y) * 2.0 + 1.0;
  ctx->mouse.movement += ctx->mouse.position - ctx->mouse.lastPosition;
  ctx->mouse.lastPosition = ctx->mouse.position;
}

void WindowContext::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }
  WindowContext* ctx = (WindowContext*) glfwGetWindowUserPointer(window);
  ctx->mouse.wheel += yoffset;
}

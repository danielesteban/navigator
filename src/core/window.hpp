#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <deque>
#include <string>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "lib/imgui_markdown.h"

enum WindowEventType {
  WINDOW_EVENT_RELOAD,
  WINDOW_EVENT_SAVE_EDITOR,
  WINDOW_EVENT_SET_URL,
};

struct WindowEvent {
  WindowEventType type;
  std::string data;
  GLubyte flags;
};

class WindowContext {
  public:
    std::deque<WindowEvent> events;
    ImGui::MarkdownConfig markdown;
    struct {
      struct {
        bool primary;
        bool primaryDown;
        bool primaryUp;
        bool secondary;
        bool secondaryDown;
        bool secondaryUp;
      } buttons;
      bool hover;
      glm::vec2 lastPosition;
      bool locked;
      glm::vec2 movement;
      glm::vec2 position;
      GLfloat wheel;
    } mouse;
    glm::vec2 resolution;
    bool showEditor;
    bool showUI;
    GLFWwindow* window;
    WindowContext();
    bool isFullscreen();
    void enterFullScreen();
    void exitFullscreen();
    void lockMouse(bool force = false);
    void unlockMouse();
    void setURL(const std::string& url, GLubyte flags = 0);
  private:
    struct {
      GLint x, y, width, height;
    } stateBeforeFullscreen;
    static void dropCallback(GLFWwindow* window, int count, const char** paths);
    static void focusCallback(GLFWwindow* window, int focused);
    static void resizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorEnterCallback(GLFWwindow* window, int entered);
    static void cursorPosCallback(GLFWwindow* window, double x, double y);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

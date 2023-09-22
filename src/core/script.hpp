#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <wtr/watcher.hpp>
#include "lib/imgui_texteditor.h"
#include "http.hpp"

class Script {
  public:
    Script(HTTP* http);
    ~Script();
    void addressBar();
    TextEditor editor;
    const std::string& getSource();
    void setURL(const std::string& url, GLubyte flags);
    void reload();
    bool update();
    bool loadFromEditor();
  private:
    void load(const std::string& text);
    void loadIncludes();
    bool loadFromFile(const std::string& path);
    HTTP* http;
    HTTPRequest* request;
    std::vector<HTTPRequest*> includes;
    struct {
      std::vector<std::string> past;
      std::vector<std::string> future;
    } history;
    std::string source;
    std::string url;
    struct {
      bool isEnabled;
      wtr::watcher::_ close;
    } watcher;
    struct {
      GLuint count;
      GLfloat lastTick;
      std::string text;
    } fps;
    char InputBuf[256];
    bool needsUpdate;
    static const char* errorScriptSource;
    static const char* welcomeScriptSource;
};

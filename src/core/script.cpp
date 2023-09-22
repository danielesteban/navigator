#include "script.hpp"
#include "lib/icons.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

enum {
  IS_FROM_ADDRESS_BAR = 1,
  IS_FROM_HISTORY = 2,
};

Script::Script(HTTP* http):
  editor(),
  url(""),
  source(""),
  http(http),
  request(nullptr),
  watcher({ false }),
  fps({ 0, 0.0, "···" }),
  needsUpdate(false)
{
  auto lang = TextEditor::LanguageDefinition::Lua();
  TextEditor::Palette palette = TextEditor::GetDarkPalette();
  palette.at((int) TextEditor::PaletteIndex::Background) = 0x00000000;
  editor.SetPalette(palette);
  editor.SetShowWhitespaces(false);
  const char* identifiers[] = {
    "delta", "time", "log", "clearLog",
    "navigate", "setClearColor", "showTooltip",
    "camera",
    "getFov",
    "setFov",
    "getPosition",
    "setPosition",
    "lookAt",
    "keyboard",
    "mouse",
    "cursor",
    "lock",
    "unlock",
    "physics",
    "getContacts",
    "getContactIds",
    "setGravity",
    "raycaster",
    "resolution",
    "clamp",
    "cross",
    "dot",
    "hsv",
    "lerp",
    "normalize",
    "spherical",
    "srgb",
    "Environment",
    "Framebuffer",
    "bind",
    "blit",
    "clear"
    "setTextureClearColor",
    "setTextureClearData",
    "unbind",
    "render",
    "Geometry",
    "setColliders",
    "setIndex",
    "setVertices",
    "HTTP",
    "isReady",
    "getResponse",
    "Image",
    "getSize",
    "Mesh",
    "getId",
    "getScale",
    "setScale",
    "getFrustumCulling",
    "setFrustumCulling",
    "enablePhysics",
    "disablePhysics",
    "applyForce",
    "applyImpulse",
    "setDamping",
    "getAngularVelocity",
    "setAngularVelocity",
    "getLinearVelocity",
    "setLinearVelocity",
    "uniformInt",
    "uniformFloat",
    "uniformTexture",
    "uniformVec2",
    "uniformVec3",
    "uniformVec4",
    "SFX",
    "play",
    "Shader",
    "getBlend",
    "setBlend",
    "getDepthTest",
    "setDepthTest",
    "Voxels",
    "get",
    "set",
    "ground",
    "pathfind",
  };
  TextEditor::Identifier defaultId;
  defaultId.mDeclaration = "";
  for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i) {
    lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), defaultId));
  }
  editor.SetLanguageDefinition(lang);
  InputBuf[0] = '\0';
}

Script::~Script() {
  if (request != nullptr) {
    delete request;
    request = nullptr;
  }
  for (const auto& request : includes) {
    delete request;
  }
  if (watcher.isEnabled) {
    watcher.close();
    watcher.isEnabled = false;
  }
}

void Script::addressBar() {
  ImGui::SetNextWindowPos(ImVec2(0.0, 0.0), 0);
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 0.0), 0);
  ImGui::Begin("##addressBar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

  bool canGoBackwards = history.past.size() > 1;
  bool canGoForwards = history.future.size() > 0;
  if (!canGoBackwards) ImGui::BeginDisabled();
  if (ImGui::Button(ICON_ARROW_LEFT, ImVec2(20.0, 0.0))) {
    history.future.push_back(history.past.back());
    history.past.pop_back();
    setURL(history.past.back(), IS_FROM_HISTORY);
  }
  if (!canGoBackwards) ImGui::EndDisabled();
  else if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
  ImGui::SameLine(0.0, 5.0);
  if (!canGoForwards) ImGui::BeginDisabled();
  if (ImGui::Button(ICON_ARROW_RIGHT, ImVec2(20.0, 0.0))) {
    history.past.push_back(history.future.back());
    history.future.pop_back();
    setURL(history.past.back(), IS_FROM_HISTORY);
  }
  if (!canGoForwards) ImGui::EndDisabled();
  else if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

  ImGui::SameLine();
  if (ImGui::Button((needsUpdate || http->count != 0) ? ICON_LOADING : ICON_RELOAD, ImVec2(20.0, 0.0))) {
    reload();
  }
  if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

  ImGui::SameLine();
  ImGui::PushItemWidth(-60.0);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 3));
  if (ImGui::InputText("##addressBarInput", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
    setURL((char*) InputBuf, IS_FROM_ADDRESS_BAR);
  }
  ImGui::PopStyleVar(2);
  ImGui::PopItemWidth();

  fps.count++;
  const GLfloat time = (GLfloat) glfwGetTime();
  if (time >= fps.lastTick + 1) {
    fps.text = std::to_string((GLuint) round((GLfloat) fps.count / (time - fps.lastTick))) + "fps";
    fps.count = 0;
    fps.lastTick = time;
  }
  ImGui::SameLine();
  const ImVec2 s = ImGui::CalcTextSize(fps.text.c_str());
  const ImVec2 p = ImGui::GetCursorPos();
  ImGui::SetCursorPos(ImVec2(p.x + (50.0 - s.x) * 0.5, p.y));
  ImGui::Text("%s", fps.text.c_str());
  ImGui::End();
}

const std::string& Script::getSource() {
  return source;
}

void Script::setURL(const std::string& url, GLubyte flags) {
  this->url = url;
  if (!(flags & IS_FROM_ADDRESS_BAR)) {
    strcpy((char*) InputBuf, url.c_str());
  }
  if (!(flags & IS_FROM_HISTORY)) {
    if (history.future.size() > 0) {
      history.future.clear();
    }
    if (history.past.size() == 0 || history.past.back().compare(this->url) != 0) {
      history.past.push_back(this->url);
    }
  }
  needsUpdate = true;
}

void Script::reload() {
  if (request != nullptr) {
    return;
  }
  needsUpdate = true;
}

bool Script::update() {
  if (needsUpdate) {
    if (request != nullptr) {
      delete request;
      request = nullptr;
      return false;
    }
    if (!includes.empty()) {
      for (const auto& request : includes) {
        delete request;
      }
      includes.clear();
      return false;
    }
  }

  if (request != nullptr) {
    if (!request->isReady) {
      return false;
    }
    if (request->response.status >= 200 && request->response.status < 400 && request->response.size > 0) {
      load((char*) request->response.data);
    } else {
      load(errorScriptSource);
    }
    delete request;
    request = nullptr;
    return includes.empty();
  }
  if (!includes.empty()) {
    if (!std::all_of(includes.begin(), includes.end(), [](const HTTPRequest* r) { return r->isReady; })) {
      return false;
    }
    for (const auto& request : includes) {
      if (request->response.status >= 200 && request->response.status < 400 && request->response.size > 0) {
        source = std::string((char*) request->response.data) + source;
      }
      delete request;
    }
    includes.clear();
    return true;
  }
  if (needsUpdate) {
    needsUpdate = false;

    if (watcher.isEnabled) {
      watcher.close();
      watcher.isEnabled = false;
    }

    if (HTTP::isValidURL(url)) {
      request = http->request(url);
    } else if (url.find("file://") == 0 && loadFromFile(url.substr(7))) {
      return includes.empty();
    } else {
      load(
        url == "about://welcome" ? welcomeScriptSource : errorScriptSource
      );
      return includes.empty();
    }
  }
  return false;
}

void Script::load(const std::string& text) {
  source = text;
  editor.SetText(text);
  loadIncludes();
}

void Script::loadIncludes() {
  for (const auto& request : includes) {
    delete request;
  }
  includes.clear();
  std::stringstream ss(source);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    if (line.find("--") != 0) {
      break;
    }
    size_t offset = line.find("include ");
    if (offset == -1) {
      continue;
    }
    std::string url = line.substr(offset + 8);
    url.erase(url.begin(), std::find_if(url.begin(), url.end(), [](unsigned char ch) {
      return !std::isspace(ch);
    }));
    url.erase(std::find_if(url.rbegin(), url.rend(), [](unsigned char ch) {
      return !std::isspace(ch);
    }).base(), url.end());
    if (HTTP::isValidURL(url)) {
      includes.push_back(http->request(url));
    }
  }
}

bool Script::loadFromEditor() {
  source = editor.GetText().c_str();
  loadIncludes();
  return includes.empty();
}

bool Script::loadFromFile(const std::string& path) {
  const std::filesystem::path file(path);

  std::ifstream stream(file);
  if (!stream.good()) {
    return false;
  }

  load(
    std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>())
  );

  watcher.isEnabled = true;
  watcher.close = wtr::watch(file.parent_path(), [&, file](auto& e) {
    if (
      e.kind == wtr::event::kind::file
      && e.where == file
    ) {
      needsUpdate = true;
    }
  });

  return true;
}

const char* Script::errorScriptSource =
R""""(log([[
# This script doesn't exist or it can't be reached
Check if there is a typo in the URL.
If spelling is correct, check your network connection.
]])
function loop()
end
)"""";

const char* Script::welcomeScriptSource =
R""""(shader = Shader(
[[
out vec2 vUV;
void main() {
  vUV = uv;
  gl_Position = vec4(position, 1.0);
}
]],
[[
in vec2 vUV;
float sdCircle(in vec2 p, in float r) {
  return length(p) - r;
}
float opUnion(in float a, in float b, in float k) {
  float h = clamp(0.5 + 0.5*(b-a)/k, 0.0, 1.0);
  return mix(b, a, h) - k*h*(1.0-h);
}
vec2 rotate(in vec2 p, in float r) {
  float c = cos(r);
  float s = sin(r);
  return mat2(c, s, -s, c) * p;
}
void main() {
  vec2 p = rotate((vUV - 0.5) * vec2(resolution.x / resolution.y, 1.0) * 150.0, time);
  float d = sdCircle(p - vec2(0.0, sin(time) * -20.0), 20.0);
  d = opUnion(d, sdCircle(p - vec2(0.0, sin(time) * 20.0), 20.0), 20.0);
  float a = 1.0 - smoothstep(-0.2, 0.2, d);
  gl_FragColor = vec4(vec3(vUV.x, 0.0, vUV.y) * a, 1.0);
}
]]
)
version,gl,gpu = info()
log([[
# Welcome to Homebrew Navigator!
Drag & drop a Lua script or paste a URL in the address bar
**ALT+ENTER** *Toggle fullscreen* **·** **CTRL+E** *Toggle editor* **·** **CTRL+U** *Toggle UI*
)""""
"[View examples](pastebin://vQurzZEY) **·** **OpenGL ]] .. gl .. [[** **·** **]] .. gpu .. [[** **·** *v]] .. version .. [[* **·** **dani@gatunes © 2023**"
R""""(
]])
function loop()
  shader:render();
end
)"""";

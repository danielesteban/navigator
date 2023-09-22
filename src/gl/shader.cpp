#include "shader.hpp"
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

GLuint Shader::shaderId = 1;
GLchar Shader::infoLog[512];

#define GLSL "#version 460"

Shader::Shader(const char *vertexSource, const char *fragmentSource, const bool withoutVertexHeader, const bool withoutFragmentHeader):
  id(shaderId++),
  refs(1),
  blend(false),
  cameraUniformsVersion(0),
  depthTest(true),
  error(""),
  faceCulling(SHADER_FACE_CULLING_BACK),
  program(0),
  fragmentShader(0),
  vertexShader(0)
{
  const char* vertexShaderSource[] = { withoutVertexHeader ? GLSL : vertexHeader, vertexSource };
  if (!compile(vertexShader, GL_VERTEX_SHADER, vertexShaderSource)) {
    return;
  }

  const char* fragmentShaderSource[] = { withoutFragmentHeader ? GLSL : fragmentHeader, fragmentSource };
  if (!compile(fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderSource)) {
    return;
  }

  GLint success;
  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, nullptr, infoLog);
    error = "Shader program:\n";
    error += infoLog;
  }
}

Shader::~Shader() {
  glDeleteProgram(program);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);
  for (const auto& [name, texture]: uniformsTexture) {
    Texture::gc(texture);
  }
}

void Shader::use() {
  glUseProgram(program);
  if (blend) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
  }
  if (depthTest) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  if (faceCulling == SHADER_FACE_CULLING_NONE) {
    glDisable(GL_CULL_FACE);
  } else {
    glEnable(GL_CULL_FACE);
    glCullFace(faceCulling == SHADER_FACE_CULLING_BACK ? GL_BACK : GL_FRONT);
  }
}

bool Shader::getBlend() {
  return blend;
}

void Shader::setBlend(const bool enabled) {
  blend = enabled;
}

bool Shader::getDepthTest() {
  return depthTest;
}

void Shader::setDepthTest(const bool enabled) {
  depthTest = enabled;
}

const std::string& Shader::getError() {
  return error;
}

ShaderFaceCulling Shader::getFaceCulling() {
  return faceCulling;
}

void Shader::setFaceCulling(const ShaderFaceCulling mode) {
  faceCulling = mode;
}

void Shader::setCameraUniforms(Camera* camera) {
  if (cameraUniformsVersion == camera->getVersion()) {
    return;
  }
  cameraUniformsVersion = camera->getVersion();
  setUniformMat4("viewMatrix", camera->getView());
  setUniformVec3("viewPosition", camera->getPosition());
  setUniformMat4("projectionMatrix", camera->getProjection());
}

bool Shader::hasUniform(const char* name) {
  return glGetUniformLocation(program, name) != -1;
}

void Shader::setTextureUniforms(std::map<std::string, Texture*>* extra) {
  GLint index = 0;
  for (const auto& [name, texture]: uniformsTexture) {
    if (extra && extra->contains(name)) {
      continue;
    }
    setUniformInt(name.c_str(), index);
    texture->bind(index);
    index++;
  }
  if (extra == nullptr) {
    return;
  }
  for (const auto& [name, texture]: *(extra)) {
    setUniformInt(name.c_str(), index);
    texture->bind(index);
    index++;
  }
}

void Shader::setUniformInt(const char* name, const GLint value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUseProgram(program);
    glUniform1i(location, value);
  }
}

void Shader::setUniformFloat(const char* name, const GLfloat value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUseProgram(program);
    glUniform1f(location, value);
  }
}

void Shader::setUniformTexture(const std::string& name, Texture* texture) {
  if (uniformsTexture.contains(name)) {
    Texture::gc(uniformsTexture[name]);
  }
  texture->refs++;
  uniformsTexture[name] = texture;
}

void Shader::setUniformMat3(const char* name, const glm::mat3 &value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUseProgram(program);
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }
}

void Shader::setUniformMat4(const char* name, const glm::mat4 &value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUseProgram(program);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }
}

void Shader::setUniformVec2(const char* name, const glm::vec2 &value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUseProgram(program);
    glUniform2fv(location, 1, glm::value_ptr(value));
  }
}

void Shader::setUniformVec3(const char* name, const glm::vec3 &value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUseProgram(program);
    glUniform3fv(location, 1, glm::value_ptr(value));
  }
}

void Shader::setUniformVec4(const char* name, const glm::vec4 &value) {
  GLint location = glGetUniformLocation(program, name);
  if (location != -1) {
    glUseProgram(program);
    glUniform4fv(location, 1, glm::value_ptr(value));
  }
}

bool Shader::compile(GLuint& shader, const GLenum type, const GLchar** source) {
  GLint success;
  shader = glCreateShader(type);
  glShaderSource(shader, 2, source, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success) {
    return true;
  }
  glGetShaderInfoLog(shader, 512, nullptr, infoLog);
  error = (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment");
  error += " shader:\n";
  const std::vector<std::string> errors = getLines(infoLog);
  const std::vector<std::string> lines = getLines(std::string(source[0]) + std::string(source[1]));
  for (const auto& e : errors) {
    GLint start = e.find("0("); 
    GLint end = e.find(") : error", start + 2);
    if (start == 0 && end > 2) {
      error += e.substr(e.find(":", 10) + 2) + "\n";
      GLint line = std::stoi(e.substr(2, end - 2));
      if (line > 0) {
        error += getLinesNear(lines, line - 1);
      }
    } else {
      error += e + "\n";
    }
  }
  return false;
}

std::vector<std::string> Shader::getLines(const std::string& text) {
  std::vector<std::string> lines;
  std::stringstream ss(text);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    lines.push_back(line);
  }
  return lines;
}

std::string Shader::getLinesNear(const std::string& text, const GLint line) {
  return getLinesNear(getLines(text), line);
}

std::string Shader::getLinesNear(const std::vector<std::string>& lines, const GLint line) {
  std::string source;
  GLint start = glm::max((GLint) line - 1, (GLint) 0);
  GLint end = glm::min((GLint) line + 1, (GLint) lines.size() - 1);
  if (start > 0) source += "     ···\n";
  for (GLint i=start; i<=end; i++) {
    source += (i == line ? ">  " : "    ") + lines.at(i) + "\n";
  }
  if (end < lines.size() - 1) source += "     ···\n";
  return source;
}

const char* Shader::vertexHeader =
GLSL
R""""(
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 color;
uniform mat3 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec2 resolution;
uniform float time;
)"""";

const char* Shader::fragmentHeader =
GLSL
R""""(
layout(location = 0) out vec4 fragOutput0;
#define gl_FragColor fragOutput0
uniform vec3 viewPosition;
uniform vec2 resolution;
uniform float time;
vec4 sRGB(in vec4 value) {
return clamp(vec4(mix(pow(value.rgb, vec3(0.41666)) * 1.055 - vec3(0.055), value.rgb * 12.92, vec3(lessThanEqual(value.rgb, vec3(0.0031308)))), value.a), 0.0, 1.0);
}
)"""";

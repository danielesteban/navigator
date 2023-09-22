#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>
#include "camera.hpp"
#include "texture.hpp"

enum ShaderFaceCulling {
  SHADER_FACE_CULLING_BACK,
  SHADER_FACE_CULLING_FRONT,
  SHADER_FACE_CULLING_NONE
};

static const char* ShaderFaceCullingNames[] = {
  "back",
  "front",
  "none",
  nullptr
};

class Shader {
  public:
    const GLuint id;
    GLuint refs;
    Shader(const char *vertexSource, const char *fragmentSource, const bool withoutVertexHeader = false, const bool withoutFragmentHeader = false);
    ~Shader();
    void use();
    bool getBlend();
    void setBlend(const bool enabled);
    bool getDepthTest();
    void setDepthTest(const bool enabled);
    const std::string& getError();
    ShaderFaceCulling getFaceCulling();
    void setFaceCulling(const ShaderFaceCulling mode);
    bool hasUniform(const char* name);
    void setCameraUniforms(Camera* camera);
    void setTextureUniforms(std::map<std::string, Texture*>* extra = nullptr);
    void setUniformInt(const char* name, const GLint value);
    void setUniformFloat(const char* name, const GLfloat value);
    void setUniformTexture(const std::string& name, Texture* texture);
    void setUniformMat3(const char* name, const glm::mat3& value);
    void setUniformMat4(const char* name, const glm::mat4& value);
    void setUniformVec2(const char* name, const glm::vec2& value);
    void setUniformVec3(const char* name, const glm::vec3& value);
    void setUniformVec4(const char* name, const glm::vec4& value);
    static std::string getLinesNear(const std::string& text, const GLint line);
  private:
    static GLuint shaderId;
    GLuint cameraUniformsVersion;
    bool blend;
    bool depthTest;
    std::string error;
    ShaderFaceCulling faceCulling;
    GLuint program;
    GLuint fragmentShader;
    GLuint vertexShader;
    std::map<std::string, Texture*> uniformsTexture;
    static const char* vertexHeader;
    static const char* fragmentHeader;
    static GLchar infoLog[];
    bool compile(GLuint& shader, const GLenum type, const GLchar** source);
    static std::string getLinesNear(const std::vector<std::string>& lines, const GLint line);
    static std::vector<std::string> getLines(const std::string& text);
};

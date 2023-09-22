#include "cubemapbuffer.hpp"
#include <fstream>
#include <inttypes.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static const char* vertexShader =
R""""(
out vec3 vPos;
void main() {
  vPos = position;
  gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
}
)"""";

static const char* fragmentShaderCubemap =
R""""(
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

in vec3 vPos;
uniform sampler2D equirectangularMap;
void main() {
  vec2 uv = SampleSphericalMap(normalize(vPos));
  vec3 color = texture(equirectangularMap, uv).rgb;
  gl_FragColor = vec4(color, 1.0);
}
)"""";

static const char* fragmentShaderIrradiance =
R""""(
const float PI = 3.14159265359;

in vec3 vPos;
uniform samplerCube environmentMap;
void main() {
  vec3 N = normalize(vPos);
  vec3 irradiance = vec3(0.0);   
  vec3 up    = vec3(0.0, 1.0, 0.0);
  vec3 right = normalize(cross(up, N));
  up         = normalize(cross(N, right));

  float sampleDelta = 0.025;
  float nrSamples = 0.0;
  for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
    for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
      vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
      vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

      irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
      nrSamples++;
    }
  }
  irradiance = PI * irradiance * (1.0 / float(nrSamples));
  gl_FragColor = vec4(irradiance, 1.0);
}
)"""";

static const char* fragmentShaderPrefiltered =
R""""(
const float PI = 3.14159265359;
float RadicalInverse_VdC(uint bits)  {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10;
}
vec2 Hammersley(uint i, uint N) {
  return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
  float a = roughness*roughness;
  
  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
  
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;
  
  vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent   = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);
  
  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
}
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness*roughness;
  float a2 = a*a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float nom   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}

in vec3 vPos;
uniform samplerCube environmentMap;
uniform float roughness;
void main() {
  vec3 N = normalize(vPos);
  vec3 R = N;
  vec3 V = R;

  const uint SAMPLE_COUNT = 1024u;
  vec3 prefilteredColor = vec3(0.0);
  float totalWeight = 0.0;

  for(uint i = 0u; i < SAMPLE_COUNT; i++) {
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L  = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL > 0.0) {
      float D   = DistributionGGX(N, H, roughness);
      float NdotH = max(dot(N, H), 0.0);
      float HdotV = max(dot(H, V), 0.0);
      float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

      float resolution = 512.0; // resolution of source cubemap (per face)
      float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
      float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

      float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
      
      prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
      totalWeight      += NdotL;
    }
  }

  prefilteredColor = prefilteredColor / totalWeight;
  gl_FragColor = vec4(prefilteredColor, 1.0);
}
)"""";

static const glm::mat4 views[] = {
  glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)),
  glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)),
  glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)),
  glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)),
  glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)),
  glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0))
};

Cubemapbuffer::Cubemapbuffer():
  box(2, 2, 2),
  shaderCubemap(vertexShader, fragmentShaderCubemap),
  shaderIrradiance(vertexShader, fragmentShaderIrradiance),
  shaderPrefiltered(vertexShader, fragmentShaderPrefiltered)
{
  glGenFramebuffers(1, &fbo);
  glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  shaderCubemap.setFaceCulling(SHADER_FACE_CULLING_FRONT);
  shaderCubemap.setUniformInt("equirectangularMap", 0);
  shaderCubemap.setUniformMat4("projectionMatrix", projection);
  shaderIrradiance.setFaceCulling(SHADER_FACE_CULLING_FRONT);
  shaderIrradiance.setUniformInt("environmentMap", 0);
  shaderIrradiance.setUniformMat4("projectionMatrix", projection);
  shaderPrefiltered.setFaceCulling(SHADER_FACE_CULLING_FRONT);
  shaderPrefiltered.setUniformInt("environmentMap", 0);
  shaderPrefiltered.setUniformMat4("projectionMatrix", projection);
}

Cubemapbuffer::~Cubemapbuffer() {
  glDeleteFramebuffers(1, &fbo);
}

void Cubemapbuffer::renderHDR(
  GLfloat* data,
  GLint width,
  GLint height,
  GLuint& output,
  GLint outputWidth,
  GLint outputHeight
) {
  GLuint input;
  glGenTextures(1, &input);
  glBindTexture(GL_TEXTURE_2D, input);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  create(output, outputWidth, outputHeight, true);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, input);
  render(output, outputWidth, outputHeight, shaderCubemap);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDeleteTextures(1, &input);

  glBindTexture(GL_TEXTURE_CUBE_MAP, output);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemapbuffer::renderIrradiance(
  GLuint input,
  GLuint& output,
  GLint outputWidth,
  GLint outputHeight
) {
  create(output, outputWidth, outputHeight);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, input);
  render(output, outputWidth, outputHeight, shaderIrradiance);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  // dump(output, outputWidth, outputHeight);
}

void Cubemapbuffer::renderPrefiltered(
  GLuint input,
  GLuint& output,
  GLint outputWidth,
  GLint outputHeight
) {
  create(output, outputWidth, outputHeight, true);
  glBindTexture(GL_TEXTURE_CUBE_MAP, output);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, input);
  render(output, outputWidth, outputHeight, shaderPrefiltered, 5);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemapbuffer::create(GLuint& texture, const GLint width, const GLint height, const bool trilinear) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  for (GLint i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  if (trilinear) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemapbuffer::render(const GLuint output, const GLint width, const GLint height, Shader& shader, const GLuint mipLevels) {
  GLint framebuffer;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  shader.use();
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  for (GLuint mip = 0; mip < mipLevels; mip++) {
    if (mipLevels > 1) {
      GLuint mipWidth  = width * glm::pow(0.5, mip);
      GLuint mipHeight = height * glm::pow(0.5, mip);
      glViewport(0, 0, mipWidth, mipHeight);
      shader.setUniformFloat("roughness", (GLfloat) mip / (GLfloat) (mipLevels - 1.0));
    } else {
      glViewport(0, 0, width, height);
    }
    for (GLint i = 0; i < 6; i++) {
      shader.setUniformMat4("viewMatrix", views[i]);
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, output, mip);
      glClear(GL_COLOR_BUFFER_BIT);
      box.draw();
    }
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
  glViewport(viewport[0], viewport[1], (GLsizei) viewport[2], (GLsizei) viewport[3]);
}

void Cubemapbuffer::dump(const GLuint texture, const GLint width, const GLint height) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  const GLsizei count = width * height * 3;
  GLfloat* pixels = new GLfloat[count];
  std::ofstream ofs("./dump.hpp");

  ofs << "#pragma once\n\n";
  ofs << "static const unsigned int DUMP_size = " << count << ";\n";
  ofs << "static const float DUMP_data[6][] = {\n";

  char buf[128];
  for (GLint i = 0; i < 6; i++) {
    glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_FLOAT, pixels);
    ofs << "{\n";
    for (GLint p = 0; p < count; p += 3) {
      ofs << pixels[p] << "," << pixels[p + 1] << "," << pixels[p + 2] << ",";
    }
    ofs << "\n},\n";
  }

  ofs << "}\n";
  ofs.close();
  delete[] pixels;

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

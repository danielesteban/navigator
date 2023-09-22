Edge = [[
float getDepthFromPos(in vec3 position) {
  return distance(position, viewPosition);
}
vec3 sobelSample(in sampler2D tex, in vec2 uv, in vec3 offset) {
  vec3 pixelCenter = texture(tex, uv).rgb;
  vec3 pixelLeft   = texture(tex, uv - offset.xz).rgb;
  vec3 pixelRight  = texture(tex, uv + offset.xz).rgb;
  vec3 pixelUp     = texture(tex, uv + offset.zy).rgb;
  vec3 pixelDown   = texture(tex, uv - offset.zy).rgb;
  return (
    abs(pixelLeft    - pixelCenter)
    + abs(pixelRight - pixelCenter)
    + abs(pixelUp    - pixelCenter)
    + abs(pixelDown  - pixelCenter)
  );
}
float sobelSampleDepth(in sampler2D tex, in vec2 uv, in vec3 offset) {
  float pixelCenter = getDepthFromPos(texture(tex, uv).xyz);
  float pixelLeft   = getDepthFromPos(texture(tex, uv - offset.xz).xyz);
  float pixelRight  = getDepthFromPos(texture(tex, uv + offset.xz).xyz);
  float pixelUp     = getDepthFromPos(texture(tex, uv + offset.zy).xyz);
  float pixelDown   = getDepthFromPos(texture(tex, uv - offset.zy).xyz);
  return (
    abs(pixelLeft    - pixelCenter)
    + abs(pixelRight - pixelCenter)
    + abs(pixelUp    - pixelCenter)
    + abs(pixelDown  - pixelCenter)
  );
}
float edge(in vec2 uv, in float intensity, in float thickness, in float depthScale, in float depthBias, in float normalScale, in float normalBias) {
  vec3 offset = vec3((1.0 / resolution.x), (1.0 / resolution.y), 0.0) * thickness;
  float sobelDepth = sobelSampleDepth(positionTexture, uv, offset);
  sobelDepth = pow(clamp(sobelDepth, 0.0, 1.0) * depthScale, depthBias);
  vec3 sobelNormalVec = sobelSample(normalTexture, uv, offset);
  float sobelNormal = sobelNormalVec.x + sobelNormalVec.y + sobelNormalVec.z;
  sobelNormal = pow(sobelNormal * normalScale, normalBias);
  return clamp(max(sobelDepth, sobelNormal), 0.0, 1.0) * intensity;
}
]]

IBL = [[
uniform sampler2D brdfMap;
uniform samplerCube irradianceMap;
uniform samplerCube prefilteredMap;
vec3 fresnelSchlickRoughness(in float cosTheta, in vec3 F0, in float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 IBL(
  in vec3 color, in vec3 normal, in vec3 position,
  in float metalness, in float roughness
) {
  vec3 N = normal;
  vec3 V = normalize(viewPosition - position);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, color, metalness);

  vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metalness;

  vec3 irradiance = texture(irradianceMap, N).rgb;
  vec3 diffuse    = irradiance * color;

  vec3 ambient = kD * diffuse;

  #ifndef IBL_DISABLE_SPECULAR
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 R = reflect(-V, N);
    vec3 prefilteredColor = textureLod(prefilteredMap, R, roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf = texture(brdfMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    ambient += specular;
  #endif

  return ambient;
}
]]

VertexWithNormalAndPosition = [[
out vec3 vNormal;
out vec3 vPosition;
void main() {
  vec4 pos = modelMatrix * vec4(position, 1.0);
  vNormal = normalMatrix * normal;
  vPosition = pos.xyz;
  gl_Position = projectionMatrix * viewMatrix * pos;
}
]]

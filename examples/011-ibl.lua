-- ShaderChunks
-- include pastebin://Rqpqfa7i
-- OrbitControls
-- include pastebin://zv4spBMk

environment = Environment("https://cdn.glitch.global/f95f83f8-1295-420a-ba25-3773d1099c3a/wrestling_gym_2k.hdr")

shader = Shader(
VertexWithNormalAndPosition,
[[
in vec3 vNormal;
in vec3 vPosition;
uniform vec3 color;
uniform float metalness;
uniform float roughness;
]] .. IBL .. [[
void main() {
  vec3 light = IBL(
    color, vNormal, vPosition,
    metalness, roughness
  );
  light = light / (light + vec3(1.0));
  gl_FragColor = sRGB(vec4(light, 1.0));
}
]]
)
shader:uniformTexture("irradianceMap", environment, "irradiance")
shader:uniformTexture("prefilteredMap", environment, "prefiltered")

skyboxShader = Shader(
[[
out vec3 vPos;
void main() {
  vPos = position;
  gl_Position = projectionMatrix * mat4(mat3(viewMatrix)) * vec4(position, 1.0);
  gl_Position = gl_Position.xyww;
}
]],
[[
in vec3 vPos;
uniform samplerCube environmentMap;
void main() {
  vec3 color = texture(environmentMap, vPos).rgb;
  color = color / (color + vec3(1.0));
  gl_FragColor = sRGB(vec4(color, 1.0));
}
]]
)
skyboxShader:setFaceCulling("front")
skyboxShader:uniformTexture("environmentMap", environment, "cubemap")

mesh = Mesh("sphere", shader)
mesh:uniformVec3("color", 0.3, 0.4, 0.7)
mesh:uniformFloat("metalness", 0.0)
mesh:uniformFloat("roughness", 0.1)

skybox = Mesh("box", skyboxShader)

controls = OrbitControls:new({
  phi = math.rad(60),
  radius = 4.5,
});

framebuffer = Framebuffer(1, true, 4)

log([[
## #011 IBL
Press *CTRL+E* to edit source
]])

function loop()
  if not controls.dragging then
    controls.theta = controls.theta + delta * 0.2
  end
  controls:update()

  framebuffer:bind(resolution.x, resolution.y)
  framebuffer:clear()
  mesh:render()
  skybox:render()
  framebuffer:unbind()
  framebuffer:blit()
end

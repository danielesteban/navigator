-- ShaderChunks
-- include pastebin://Rqpqfa7i
-- OrbitControls
-- include pastebin://zv4spBMk

Fog = [[
vec4 fog(in vec4 color) {
  float depth = distance(vPosition, viewPosition);
  float fog = 1.0 - exp(-0.02 * 0.02 * depth * depth);
  return vec4(mix(color.rgb, vec3(0.05, 0.05, 0.1), fog), color.a);
}
]]

Phong = [[
vec4 phong(in vec3 color, in vec3 normal, in vec3 position, in vec3 lightDir) {
  float ambient = 0.03;
  float diffuse = max(dot(lightDir, normal), 0.0);
  float specular = pow(max(dot(normal, normalize(lightDir + normalize(viewPosition - position))), 0.0), 16.0) * 0.5;
  return vec4(color * (ambient + diffuse + specular), 1.0);
}
]]

gridShader = Shader(
VertexWithNormalAndPosition,
[[
in vec3 vNormal;
in vec3 vPosition;
uniform vec3 lightPosition;
]] .. Fog .. [[
]] .. Phong .. [[
void main() {
  vec2 coord = vPosition.xz * 0.25;
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
  float line = min(grid.x, grid.y);
  vec3 color = vec3(0.5, 0.5, 1.0) * (1.0 - min(line, 1.0));
  gl_FragColor = fog(sRGB(phong(color, normalize(vNormal), vPosition, normalize(lightPosition - vPosition))));
}
]]
)

shader = Shader(
VertexWithNormalAndPosition,
[[
in vec3 vNormal;
in vec3 vPosition;
uniform int isLight;
uniform vec3 lightPosition;
uniform vec3 color;
]] .. Fog .. [[
]] .. Phong .. [[
void main() {
  if (isLight == 1) {
    gl_FragColor = fog(sRGB(vec4(color, 1.0)));
  } else {
    gl_FragColor = fog(sRGB(phong(color, normalize(vNormal), vPosition, normalize(lightPosition - vPosition))));
  }
}
]]
)

grid = Mesh("plane", gridShader)
grid:setPosition(0, -1, 0)
grid:setScale(100, 100, 1)
grid:lookAt(0, 1, 0)
grid:enablePhysics()

light = Mesh("sphere", shader)
light:uniformVec3("color", 1, 1, 1)
light:setScale(0.5, 0.5, 0.5)

mesh = 1
numMeshes = 200
meshes = {}
for i=1,numMeshes do
  meshes[i] = Mesh(math.random() > 0.8 and "box" or "sphere", shader)
  meshes[i]:uniformVec3("color", math.random(), math.random(), math.random())
  meshes[i]:setPosition(math.cos(math.random() * math.pi * 2) * 4, math.random() * 30, math.sin(math.random() * math.pi * 2) * 4)
  meshes[i]:enablePhysics(0.5)
end

controls = OrbitControls:new({
  phi = math.rad(70),
  theta = math.rad(-45),
  radius = 30,
  target = { x = 0, y = 10, z = 0 }
});

framebuffer = Framebuffer(1, true, 4)
framebuffer:setTextureClearColor(0, 0.05, 0.05, 0.1, 1.0)

log([[
## #004 Physics
Press *CTRL+E* to edit source
]])

lastTick = 0
function loop()
  if not controls.dragging then
    controls.theta = controls.theta + delta * 0.1
  end
  controls:update()

  if lastTick + 0.05 < time then
    lastTick = time
    meshes[mesh]:setPosition(math.cos(math.random() * math.pi * 2) * 4, 30, math.sin(math.random() * math.pi * 2) * 4)
    meshes[mesh]:setAngularVelocity(0, 0, 0)
    meshes[mesh]:setLinearVelocity(0, 0, 0)
    meshes[mesh]:applyImpulse(math.random() - 0.5, math.random() - 0.5, math.random() - 0.5)
    mesh = 1 + (mesh % numMeshes)
  end

  local x, y, z = math.spherical(math.pi * 0.3 + math.sin(time) * 0.5, time, 24)
  y = y + 4;
  framebuffer:bind(resolution.x, resolution.y)
  framebuffer:clear()
  shader:uniformVec3("lightPosition", x, y, z)
  gridShader:uniformVec3("lightPosition", x, y, z)
  grid:render()
  light:setPosition(x, y, z)
  shader:uniformInt("isLight", 1)
  light:render()
  shader:uniformInt("isLight", 0)
  for i=1,numMeshes do
    meshes[i]:render()
  end
  framebuffer:unbind()
  framebuffer:blit()
end

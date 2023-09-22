-- ShaderChunks
-- include pastebin://Rqpqfa7i
-- OrbitControls
-- include pastebin://zv4spBMk

meshShader = Shader(
VertexWithNormalAndPosition,
[[
  layout(location = 1) out vec4 fragNormal;
  layout(location = 2) out vec4 fragPosition;
  in vec3 vNormal;
  in vec3 vPosition;
  uniform vec3 color;
  void main() {
    gl_FragColor = vec4(color, 1.0);
    fragNormal = vec4(normalize(vNormal), 1.0);
    fragPosition = vec4(vPosition, 1.0);
  }
]]
)

voxelsShader = Shader(
[[
out vec3 vColor;
out vec3 vNormal;
out vec3 vPosition;
void main() {
  vec4 pos = modelMatrix * vec4(position, 1.0);
  vColor = color;
  vNormal = normalMatrix * normal;
  vPosition = pos.xyz;
  gl_Position = projectionMatrix * viewMatrix * pos;
}
]],
[[
layout(location = 1) out vec4 fragNormal;
layout(location = 2) out vec4 fragPosition;
in vec3 vColor;
in vec3 vNormal;
in vec3 vPosition;
void main() {
  gl_FragColor = vec4(vColor, 1.0);
  fragNormal = vec4(normalize(vNormal), 1.0);
  fragPosition = vec4(vPosition, 1.0);
}
]]
)

controls = OrbitControls:new({
  phi = math.rad(45),
  theta = math.rad(-45),
  radius = 20,
});

mesh = 1
numMeshes = 200
meshes = {}
for i=1,numMeshes do
  meshes[i] = Mesh(math.random() > 0.8 and "box" or "sphere", meshShader)
  meshes[i]:uniformVec3("color", math.random(), math.random(), math.random())
  meshes[i]:setPosition(math.cos(math.random() * math.pi * 2) * 4, 50 + math.random() * 100, math.sin(math.random() * math.pi * 2) * 4)
  meshes[i]:enablePhysics(0.5)
end

voxels = Voxels(voxelsShader)

function generate()
  local r = 32
  for z=-r,r do
    for y=-r,r*0.5 do
      for x=-r,r do
        local dz = z
        local dy = y
        local dx = x
        local d = math.sqrt(dz * dz + dy * dy + dx * dx)
        local d2 = math.sqrt(dz * dz + (dy - r * 0.4) * (dy - r * 0.4) + dx * dx)
        if d < r and d2 > r * 0.7 then
          local r, g, b = math.hsv(0.2 + math.random() * 0.3, 0.3 + math.random() * 0.6, 0.6 + math.random() * 0.3)
          voxels:set(x, y, z, 1, math.floor(r * 255), math.floor(g * 255), math.floor(b * 255))
        end
      end
    end
  end
end

function update(vx, vy, vz, r)
  for z=-r,r do
    for y=-r,r do
      for x=-r,r do
        local d = math.sqrt(z * z + y * y + x * x)
        if d < r then
          voxels:set(vx + x, vy + y, vz + z, 0)
        end
      end
    end
  end
end

generate()
voxels:enablePhysics()

log([[
# #007 Voxels
Press *CTRL+E* to edit source
]])

lastTick = 0
function loop()
  if lastTick + 0.05 < time then
    lastTick = time
    meshes[mesh]:setPosition(math.cos(math.random() * math.pi * 2) * 4, 100, math.sin(math.random() * math.pi * 2) * 4)
    meshes[mesh]:setAngularVelocity(0, 0, 0)
    meshes[mesh]:setLinearVelocity(0, 0, 0)
    meshes[mesh]:applyImpulse(math.random() - 0.5, math.random() - 0.5, math.random() - 0.5)
    mesh = 1 + (mesh % numMeshes)
  end

  if not controls.dragging then
    raycaster.setFromCamera(mouse.x, mouse.y)
    if raycaster.intersect(voxels) then
      mouse.cursor("hand")
      if mouse.primaryUp then
        local rx, ry, rz = raycaster.getResult()
        update(math.floor(rx), math.floor(ry), math.floor(rz), 6)
      end
    end
  end
  controls:update()

  renderbuffer:bind(resolution.x, resolution.y)
  renderbuffer:clear()
  voxels:render()
  for i=1,numMeshes do
    meshes[i]:render()
  end
  renderbuffer:unbind()
  framebuffer:bind(resolution.x, resolution.y)
  framebuffer:unbind()
  renderbuffer:blit(framebuffer)

  postrocessing:render()
end

renderbuffer = Framebuffer(3, true, 4)
renderbuffer:setTextureClearColor(0, 0, 0, 0, 0)
renderbuffer:setTextureClearColor(1, 0, 0, 0, 1)
renderbuffer:setTextureClearColor(2, 10000, 10000, 10000, 1)
framebuffer = Framebuffer(3)
postrocessing = Shader(
[[
out vec2 vUV;
void main() {
  vUV = uv;
  gl_Position = vec4(position, 1.0);
}
]],
[[
in vec2 vUV;
uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
]] .. Edge .. [[
#define IBL_DISABLE_SPECULAR
]] .. IBL .. [[
void main() {
  vec4 color = texture(colorTexture, vUV);
  vec3 normal = texture(normalTexture, vUV).xyz;
  vec3 position = texture(positionTexture, vUV).xyz;
  color.rgb = IBL(
    color.rgb, normal, position,
    0.0, 0.8
  );
  float depth = getDepthFromPos(position);
  float decay = (1.0 - exp(-0.01 * 0.01 * depth * depth));
  color.rgb = mix(color.rgb, vec3(0.0), edge(vUV, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0) * (1.0 - decay));
  color.rgb = mix(color.rgb, vec3(0.1, 0.2, 0.3), max(1.0 - color.a, decay));

  float v = smoothstep(-0.4, 0.4, 0.5 - distance(vUV, vec2(0.5, 0.5)));
  gl_FragColor = sRGB(vec4(
    color.rgb * v,
    1.0
  ));
}
]]
)
postrocessing:uniformTexture("colorTexture", framebuffer, 0)
postrocessing:uniformTexture("normalTexture", framebuffer, 1)
postrocessing:uniformTexture("positionTexture", framebuffer, 2)

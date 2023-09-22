-- ShaderChunks
-- include pastebin://Rqpqfa7i
-- OrbitControls
-- include pastebin://zv4spBMk

playerShader = Shader(
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

Actor = {
  height = nil,
  mesh = nil,
  path = nil,
  velocity = 5.0,
  volume = nil,
}

function Actor:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  local x, y, z = o.mesh:getPosition()
  local sx, sy, sz = o.mesh:getScale()
  o.height = sy;
  o.mesh:setPosition(x + 0.5, self.volume:ground(math.floor(x), math.floor(y), math.floor(z)) + sy, z + 0.5)
  return o
end

function Actor:interpolate()
  if self.path then
    self.path.step = self.path.step + self.velocity * delta
    local s = math.floor(self.path.step)
    local d = math.fmod(self.path.step, 1.0)
    if s < self.path.length then
      local x = math.lerp(self.path.path[s * 3 + 1], self.path.path[(s + 1) * 3 + 1], d)
      local y = math.lerp(self.path.path[s * 3 + 2], self.path.path[(s + 1) * 3 + 2], d)
      local z = math.lerp(self.path.path[s * 3 + 3], self.path.path[(s + 1) * 3 + 3], d)
      self.mesh:setPosition(x, y, z)
    else
      self.path = nil
    end
  end
end

function Actor:walkTo(x, y, z)
  x = math.floor(x)
  y = math.floor(y)
  z = math.floor(z)
  y = self.volume:ground(x, y, z)
  if y == nil then
    return
  end
  local ox, oy, oz = self.mesh:getPosition()
  local path = {
    self.volume:pathfind(
      math.floor(ox), math.floor(oy - self.height), math.floor(oz),
      x, y, z
    )
  }
  if #path > 3 then
    path[1] = ox;
    path[2] = oy;
    path[3] = oz;
    for i=4,#path,3 do
      path[i] = path[i] + 0.5;
      path[i + 1] = path[i + 1] + self.height;
      path[i + 2] = path[i + 2] + 0.5;
    end
    self.path = {
      path = path,
      length = (#path / 3) - 1,
      step = 0,
    }
  end
end

voxels = Voxels(voxelsShader)
Actor.volume = voxels

function generate()
  local s = 40
  local noise = Noise("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/")
  local seed = math.floor(math.random() * 10000)
  for z=-s,s do
    for y=-s,s do
      for x=-s,s do
        if noise:get3D(x * 0.005, y * 0.005, z * 0.005, seed) <= 0 then
          local r, g, b = math.hsv(math.abs(noise:get3D(x * 0.01, y * 0.01, z * 0.01, seed)), 0.4 + math.random() * 0.2, 0.8 + math.random() * 0.2)
          voxels:set(x, y, z, 1, math.floor(r * 255), math.floor(g * 255), math.floor(b * 255))
        end
      end
    end
  end
end

generate()

controls = OrbitControls:new({
  phi = math.rad(45),
  radius = 50,
  target = { x = 0, y = 10, z = 0 },
})

mesh = Mesh("sphere", playerShader)
mesh:setPosition(0, 50, 0)
mesh:setScale(0.5, 1.5, 0.5)
mesh:uniformVec3("color", math.random(), math.random(), 1 + math.random())
player = Actor:new({ mesh = mesh })

log([[
# #008 Pathfind
Press *CTRL+E* to edit source
]])

lastTick = 0
function loop()
  if not controls.dragging then
    raycaster.setFromCamera(mouse.x, mouse.y)
    if raycaster.intersect(voxels) then
      mouse.cursor("hand")
      if mouse.primaryUp then
        local x, y, z, nx, ny, nz = raycaster.getResult()
        player:walkTo(x + nx * 0.5, y + ny * 0.5, z + nz * 0.5)
      end
    end
  end
  controls:update()

  player:interpolate()
  local x, y, z = player.mesh:getPosition()
  controls.target.x = x
  controls.target.y = y
  controls.target.z = z

  renderbuffer:bind(resolution.x, resolution.y)
  renderbuffer:clear()
  voxels:render()
  player.mesh:render()
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
  float decay = (1.0 - exp(-0.005 * 0.005 * depth * depth));
  color.rgb = mix(color.rgb, vec3(0.0), edge(vUV, 0.5, 0.5, 0.5, 1.0, 0.5, 1.0) * (1.0 - decay));
  color.rgb = mix(color.rgb, vec3(0.3, 0.5, 0.7), max(1.0 - color.a, decay));

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

-- ShaderChunks
-- include pastebin://Rqpqfa7i
-- WASDControls
-- include pastebin://8pSbGmFq

shader = Shader(
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

gridShader = Shader(
VertexWithNormalAndPosition,
[[
layout(location = 1) out vec4 fragNormal;
layout(location = 2) out vec4 fragPosition;
in vec3 vNormal;
in vec3 vPosition;
void main() {
  vec2 coord = vPosition.xz * 0.5;
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
  float line = min(grid.x, grid.y);
  vec3 color = mix(vec3(1.0), vec3(0.2,0.4,0.2), min(line, 1.0));
  gl_FragColor = vec4(color * 0.5, 1.0);
  fragNormal = vec4(normalize(vNormal), 1.0);
  fragPosition = vec4(vPosition, 1.0);
}
]]
)

grid = Mesh("plane", gridShader)
grid:setScale(512, 512, 1)
grid:lookAt(0, 1, 0)
grid:enablePhysics()

wall = Mesh("box", shader)
wall:uniformVec3("color", 1, 1, 1)
wall:setPosition(0, 4, -10)
wall:setScale(10, 4, 1)
wall:enablePhysics()

projectile = 1
projectiles = {}
numProjectiles = 100
for i=1,numProjectiles do
  projectiles[i] = Mesh((math.random() > 0.5) and "box" or "sphere", shader)
  projectiles[i]:uniformVec3("color", math.random(), math.random(), math.random())
  projectiles[i]:setPosition((math.random() - 0.5) * numProjectiles * 0.5, 4 + math.random() * 2, (math.random() - 0.5) * 8)
  projectiles[i]:enablePhysics(0.5)
end

controls = WASDControls:new({
  position = { x = 0, y = 4, z = 16 },
})

log([[
## #005 WASD
Press *CTRL+E* to edit source
]])

function loop()
  controls:update()
  if mouse.primaryDown then
    raycaster.setFromCamera(0, 0)
    local x, y, z, dx, dy, dz = raycaster.getRay()
    x = x + dx * 4
    y = y + dy * 4
    z = z + dz * 4
    projectiles[projectile]:setPosition(x, y, z)
    projectiles[projectile]:lookAt(x + dx, y + dy, z + dz)
    projectiles[projectile]:setAngularVelocity(0, 0, 0)
    projectiles[projectile]:setLinearVelocity(0, 0, 0)
    projectiles[projectile]:applyImpulse(dx * 20, dy * 20, dz * 20)
    projectile = 1 + (projectile % numProjectiles)
  end

  renderbuffer:bind(resolution.x, resolution.y)
  renderbuffer:clear()
  for i=1,numProjectiles do
    projectiles[i]:render()
  end
  wall:render()
  grid:render()
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
  float decay = (1.0 - exp(-0.02 * 0.02 * depth * depth));
  color.rgb = mix(color.rgb, vec3(0.0), edge(vUV + vec2(cos(vUV.y * resolution.y * 0.1 + time * 5.0) / resolution.x * 0.5, sin(vUV.x * resolution.x * 0.1 + time * 5.0) / resolution.y * 0.5), 0.8, 0.5, 1.0, 1.0, 1.0, 1.0) * (1.0 - decay));
  color.rgb = mix(color.rgb, vec3(0.1, 0.2, 0.3), max(1.0 - color.a, decay));

  float c = abs(length(gl_FragCoord.xy - resolution * 0.5) - 6.0);
  float l = 0.5 + max(floor(mod(gl_FragCoord.x, 2.0)), floor(mod(gl_FragCoord.y, 2.0)));
  float v = smoothstep(-0.4, 0.4, 0.5 - distance(vUV, vec2(0.5, 0.5)));
  color.rgb = mix(color.rgb, vec3(1.0), (1.0 - smoothstep(0.0, 2.0, c)) * 0.5);
  gl_FragColor = sRGB(vec4(
    color.rgb * v * l,
    1.0
  ));
}
]]
)
postrocessing:uniformTexture("colorTexture", framebuffer, 0)
postrocessing:uniformTexture("normalTexture", framebuffer, 1)
postrocessing:uniformTexture("positionTexture", framebuffer, 2)

geometry = Geometry()
geometry:setVertices(
   0.0,  0.5, 0.0,   0, 0, 1,   0.5, 1,   1, 0, 0,
  -0.5, -0.5, 0.0,   0, 0, 1,     0, 0,   0, 1, 0,
   0.5, -0.5, 0.0,   0, 0, 1,     1, 0,   0, 0, 1
)
geometry:setIndex(0, 1, 2)

shader = Shader(
[[
out vec3 vColor;
void main() {
  vColor = color;
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
]],
[[
in vec3 vColor;
void main() {
  gl_FragColor = sRGB(vec4(vColor, 1.0));
}
]]
)

sfx = SFX("https://cdn.glitch.global/f95f83f8-1295-420a-ba25-3773d1099c3a/ding.mp3")

numMeshes = 16
meshes = {}
local offset = math.pi / -3
local step = math.pi * 2 / numMeshes * 2
for i=1,numMeshes do
  meshes[i] = Mesh(geometry, shader)
  meshes[i]:setPosition(math.cos(offset + i * step) * 8, math.sin(offset + i * step) * 8, i * -6)
end

camera.setFov(50)
camera.setPosition(0, 0, 12)
framebuffer = Framebuffer(1, true, 4)

log([[
## #009 SFX
Press *CTRL+E* to edit source
]])

function loop()
  raycaster.setFromCamera(mouse.x, mouse.y)
  local hit = raycaster.intersect(table.unpack(meshes))
  if hit then
    mouse.cursor("hand")
  end

  framebuffer:bind(resolution.x, resolution.y)
  framebuffer:clear()
  for i=1,numMeshes do
    if mouse.primaryDown and hit == meshes[i]:getId() then
      local x, y, z = meshes[i]:getPosition()
      sfx:play(x, y, z);
    end
    meshes[i]:render()
  end
  framebuffer:unbind()
  framebuffer:blit()
end

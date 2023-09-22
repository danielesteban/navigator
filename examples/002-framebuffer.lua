framebuffer = Framebuffer(
  1,    -- numTextures
  true, -- enable depth buffer
  0     -- no multisampling
)

boxShader = Shader(
[[
out vec3 vNormal;
void main() {
  vNormal = normal;
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
]],
[[
in vec3 vNormal;
void main() {
  gl_FragColor = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);
}
]]
)
box = Mesh("box", boxShader)

planeShader = Shader(
[[
out vec2 vUV;
void main() {
  vUV = uv;
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
]],
[[
in vec2 vUV;
uniform sampler2D framebuffer;
void main() {
  gl_FragColor = texture(framebuffer, vUV);
}
]]
)
planeShader:uniformTexture("framebuffer", framebuffer)
plane = Mesh("plane", planeShader)
plane:setScale(4, 4, 1)

log([[
# #002 Framebuffer
Press *CTRL+E* to edit source
]])

function loop()
  framebuffer:bind(512, 512)
  framebuffer:clear()
  camera.setPosition(0, 0, 4)
  box:lookAt(math.sin(time), 1, math.cos(time));
  box:render()
  framebuffer:unbind()

  camera.setPosition(0, 0, 20)
  plane:setPosition(-5, -5, 0)
  plane:render()
  plane:setPosition(5, -5, 0)
  plane:render()
  plane:setPosition(5, 5, 0)
  plane:render()
  plane:setPosition(-5, 5, 0)
  plane:render()
end

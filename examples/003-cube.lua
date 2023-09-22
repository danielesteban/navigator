texture = Shader(
[[
out vec2 vUV;
void main() {
  vUV = uv;
  gl_Position = vec4(position, 1.0);
}
]],
[[
in vec2 vUV;
float sdBox(in vec2 p, in vec2 b) {
  vec2 d = abs(p)-b;
  return length(max(d,0.0)) + min(max(d.x,d.y),0.0) - 0.1;
}
void main() {
  vec2 p = (vUV - 0.5) * vec2(resolution.x / resolution.y, 1.0) * 16.0;
  float d = sdBox(vec2(-4.5, 0) - p, vec2(1, 4));
  d = min(d, sdBox(vec2(-2.5, 0) - p, vec2(2, 0.75)));
  d = min(d, sdBox(vec2(-0.5, 0) - p, vec2(1, 4)));
  d = min(d, sdBox(vec2(3.5, 3) - p, vec2(2, 1)));
  d = min(d, sdBox(vec2(3.5, 0) - p, vec2(0.75, 4)));
  d = min(d, sdBox(vec2(3.5, -3) - p, vec2(2, 1)));
  float a = 1.0 - smoothstep(-0.02, 0.02, d);
  gl_FragColor = vec4(vec3(vUV.y, 0.0, 1 - vUV.x) * max(a, 0.25), 1.0);
}
]]
)
texture:uniformVec2("resolution", 512, 512);

framebuffer = Framebuffer()
framebuffer:bind(512, 512)
framebuffer:clear()
texture:render()
framebuffer:unbind()

boxShader = Shader(
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
box = Mesh("box", boxShader)
box:uniformTexture("framebuffer", framebuffer)

camera.setPosition(0, 0, 4)

log([[
# #003 Textured cube
Press *CTRL+E* to edit source
]])

function loop()
  box:lookAt(math.cos(time), math.sin((time + math.pi) * 0.5), math.sin(time))
  box:render()
end

dotsStride = 60
numDots = dotsStride * dotsStride
zoom = 300

compute = Shader(
[[
void main() {
  gl_Position = vec4(position, 1.0);
}
]],
[[
uniform sampler2D dataTextureA;
uniform sampler2D dataTextureB;
uniform int count;
uniform int stride;
uniform vec2 size;
uniform vec3 mouse;
uniform float delta;
void main() {
  ivec2 id = ivec2(gl_FragCoord.xy);
  vec4 dataA = texelFetch(dataTextureA, id, 0);
  vec4 dataB = texelFetch(dataTextureB, id, 0);
  float damp = 1.0 - exp(-20.0 * delta);

  for (int i = 0; i < count; i++) {
    ivec2 nid = ivec2(i % stride, i / stride);
    if (nid.x == id.x && nid.y == id.y) {
      continue;
    }
    vec4 ndataA = texelFetch(dataTextureA, nid, 0);
    vec4 ndataB = texelFetch(dataTextureB, nid, 0);
    float d = distance(dataA.xy, ndataA.xy);
    if (d <= (dataB.w + ndataB.w)) {
      dataA.zw = mix(dataA.zw, normalize(dataA.xy - ndataA.xy) * 10.0, damp);
      break;
    }
  }

  if (mouse.z > 0.0) {
    vec2 m = mouse.xy * size * 0.5;
    if (distance(dataA.xy, m) <= dataB.w + 10) {
      dataA.zw = mix(dataA.zw, (dataA.xy - m) * 4.0, damp);
    }
  }

  dataA.x += dataA.z * delta;
  dataA.y += dataA.w * delta;
  if (dataA.x < size.x * -0.5 + dataB.w || dataA.x > size.x * 0.5 - dataB.w) {
    dataA.x = clamp(dataA.x, size.x * -0.5 + dataB.w, size.x * 0.5 - dataB.w);
    dataA.z *= -1;
  }
  if (dataA.y < size.y * -0.5 + dataB.w || dataA.y > size.y * 0.5 - dataB.w) {
    dataA.y = clamp(dataA.y, size.y * -0.5 + dataB.w, size.y * 0.5 - dataB.w);
    dataA.w *= -1;
  }

  fragOutput0 = dataA;
}
]]
)
compute:uniformInt("count", dotsStride * dotsStride)
compute:uniformInt("stride", dotsStride)

shader = Shader(
[[
out vec3 vColor;
out vec2 vUV;
uniform sampler2D dataTextureA;
uniform sampler2D dataTextureB;
uniform int stride;
uniform float zoom;
void main() {
  vec4 dataA = texelFetch(dataTextureA, ivec2(gl_InstanceID % stride, gl_InstanceID / stride), 0);
  vec4 dataB = texelFetch(dataTextureB, ivec2(gl_InstanceID % stride, gl_InstanceID / stride), 0);
  vColor = dataB.xyz;
  vUV = (uv - 0.5) * 2.0;
  gl_Position = vec4(
    (
      (position * dataB.w + vec3(dataA.xy, 0))
      / vec3(vec2(resolution.x / resolution.y, 1.0) * zoom, 1.0)
    ),
    1.0
  );
}
]],
[[
in vec3 vColor;
in vec2 vUV;
void main() {
  float d = min(length(vUV), 1.0);
  float l = 0.5 + fract(gl_FragCoord.y / 3.0) * 0.5;
  gl_FragColor = vec4(vColor * (1.0 - d) * l, smoothstep(1.0, 0.8, d));
}
]]
)
shader:setBlend(true)
shader:setDepthTest(false)
shader:uniformInt("stride", dotsStride)
shader:uniformFloat("zoom", zoom * 0.5)

framebuffers = { Framebuffer(), Framebuffer(), Framebuffer() }
function init()
  for i=1,#framebuffers do
    framebuffers[i]:bind(dotsStride, dotsStride)
    framebuffers[i]:unbind()
  end

  local width = resolution.x / resolution.y * zoom
  local height = zoom
  local dataA = {}
  local dataB = {}
  local i = 1
  for y=0,dotsStride-1 do
    for x=0,dotsStride-1 do
      local dx, dy = math.normalize(math.random() - 0.5, math.random() - 0.5, 0)
      local r, g, b = math.hsv(math.random(), 0.5 + math.random() * 0.5, 0.5 + math.random() * 1.5)
      dataA[i    ] = (math.random() - 0.5) * width
      dataA[i + 1] = (math.random() - 0.5) * height 
      dataA[i + 2] = dx
      dataA[i + 3] = dy
      dataB[i    ] = r
      dataB[i + 1] = g
      dataB[i + 2] = b
      dataB[i + 3] = 1 + math.random() * math.random() * 4.0
      i = i + 4
    end
  end
  framebuffers[1]:setTextureData(0, table.unpack(dataA))
  framebuffers[3]:setTextureData(0, table.unpack(dataB))
  compute:uniformTexture("dataTextureB", framebuffers[3])
  shader:uniformTexture("dataTextureB", framebuffers[3])
end

current = 1
function step()
  local next = 1 + (current % 2)

  framebuffers[next]:bind(dotsStride, dotsStride)
  compute:uniformTexture("dataTextureA", framebuffers[current])
  compute:uniformVec2("size", resolution.x / resolution.y * zoom, zoom)
  compute:uniformVec3("mouse", mouse.x, mouse.y, mouse.hover and 1 or 0)
  compute:uniformFloat("delta", delta)
  compute:render()
  framebuffers[next]:unbind()

  current = next
end

init()

log([[
## #006 Dots
Press *CTRL+E* to edit source
]])

function loop()
  step()
  shader:uniformTexture("dataTextureA", framebuffers[current])
  shader:render(numDots)
end

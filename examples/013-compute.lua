dotsWidth = 128
dotsHeight = 256
numDots = dotsWidth * dotsHeight
imageWidth = 512
imageHeight = 512
zoom = imageHeight + 64

framebuffers = { Framebuffer(), Framebuffer() }
image = Image("https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Fi.redd.it%2Fk7lix3j42cp11.jpg&f=1&nofb=1&ipt=1dc7853148c4f2f961116f0de0d1fa3b900a4c192a387eb71c17bb3e5250dbb6&ipo=images")
function generate()
  if not image:isReady() then
    return
  end
  local framebuffer = Framebuffer()
  local sobelShader = Shader(
  [[
  out vec2 vUV;
  void main() {
    vUV = uv;
    gl_Position = vec4(position, 1.0);
  }
  ]],
  [[
  in vec2 vUV;
  uniform sampler2D image;
  uniform vec2 size;
  float sobelSample(in sampler2D tex, in vec2 uv, in vec3 offset) {
    vec3 pixelCenter = texture(tex, uv).rgb;
    vec3 pixelLeft   = texture(tex, uv - offset.xz).rgb;
    vec3 pixelRight  = texture(tex, uv + offset.xz).rgb;
    vec3 pixelUp     = texture(tex, uv + offset.zy).rgb;
    vec3 pixelDown   = texture(tex, uv - offset.zy).rgb;
    float center = (pixelCenter.r + pixelCenter.g + pixelCenter.b) / 3.0;
    return (
      abs((pixelLeft.r + pixelLeft.g + pixelLeft.b) / 3.0  - center)
      + abs((pixelRight.r + pixelRight.g + pixelRight.b) / 3.0 - center)
      + abs((pixelUp.r + pixelUp.g + pixelUp.b) / 3.0    - center)
      + abs((pixelDown.r + pixelDown.g + pixelDown.b) / 3.0  - center)
    );
  }
  void main() {
    vec2 outOffset = vec2(0, 0);
    vec2 outSize = resolution;
    if (size.x / size.y > 1.0) {
      outSize.x *= size.y / size.x;
      outOffset.x = resolution.x * 0.5 - outSize.x * 0.5;
    } else {
      outSize.y *= size.x / size.y;
      outOffset.y = resolution.y * 0.5 - outSize.y * 0.5;
    }
    outSize /= resolution;
    outOffset /= resolution;
    vec2 uv = vUV * outSize + outOffset;
    vec3 offset = vec3((1.0 / resolution.x), (1.0 / resolution.y), 0.0);
    float sobel = sobelSample(image, uv, offset) * 4.0;
    gl_FragColor = vec4(vec3(clamp(sobel, 0.0, 1.0)), 1.0);
  }
  ]]
  )
  framebuffer:bind(imageWidth, imageHeight)
  sobelShader:uniformVec2("resolution", imageWidth, imageHeight)
  sobelShader:uniformVec2("size", image:getSize())
  sobelShader:uniformTexture("image", image)
  sobelShader:render()
  framebuffer:unbind()
  compute:uniformTexture("dataTextureB", framebuffer)
  -- debugshader:uniformTexture("image", framebuffer)
  image = nil
  framebuffer = nil
  sobelShader = nil
  collectgarbage()

  for i=1,#framebuffers do
    framebuffers[i]:bind(dotsWidth, dotsHeight)
    framebuffers[i]:unbind()
  end

  local data = {}
  local i=1
  for y=0,dotsHeight-1 do
    for x=0,dotsWidth-1 do
      data[i] = x / (dotsWidth - 1)
      data[i + 1] = y / (dotsHeight - 1)
      data[i + 2] = 0.0
      data[i + 3] = 1.0
      i = i + 4;
    end
  end
  framebuffers[1]:setTextureData(0, table.unpack(data))
  i = 1
  for y=0,dotsHeight-1 do
    for x=0,dotsWidth-1 do
      local r, g, b = math.hsv(math.random(), math.random(), math.random())
      data[i] = r
      data[i + 1] = g
      data[i + 2] = b
      data[i + 3] = 1.0
      i = i + 4;
    end
  end
  framebuffer = Framebuffer()
  framebuffer:bind(dotsWidth, dotsHeight)
  framebuffer:unbind()
  framebuffer:setTextureData(0, table.unpack(data))
  shader:uniformTexture("dataTextureB", framebuffer)
end

compute = Shader(
[[
void main() {
  gl_Position = vec4(position, 1.0);
}
]],
[[
uniform sampler2D dataTextureA;
uniform sampler2D dataTextureB;
uniform float delta;
void main() {
  vec4 dataA = texelFetch(dataTextureA, ivec2(gl_FragCoord.xy), 0);
  vec4 dataB = texture(dataTextureB, dataA.xy);

  dataA.w = mix(dataA.w, max(1.1 - dataB.x, 0), 1.0 - exp(-30 * delta));
  dataA.x += dataA.w * 0.1 * delta;
  if (dataA.x > 1.0) {
    dataA.x -= 1.0;
    dataA.w = 1.0;
  }

  fragOutput0 = dataA;
}
]]
)

shader = Shader(
[[
out vec3 vColor;
out vec2 vUV;
uniform sampler2D dataTextureA;
uniform sampler2D dataTextureB;
uniform int stride;
uniform vec2 size;
uniform float zoom;
void main() {
  vec4 dataA = texelFetch(dataTextureA, ivec2(gl_InstanceID % stride, gl_InstanceID / stride), 0);
  vec4 dataB = texelFetch(dataTextureB, ivec2(gl_InstanceID % stride, gl_InstanceID / stride), 0);
  vColor = dataB.xyz * (1.0 - pow(dataA.w, 2.0) * 0.8);
  vUV = (uv - 0.5) * 2.0;
  gl_Position = vec4(
    (
      (position + vec3(dataA.xy * size - size * 0.5, 0))
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
  float l = 0.5 + fract(gl_FragCoord.y / 2.0) * 0.5;
  gl_FragColor = vec4(vColor * (1.0 - d) * l, smoothstep(1.0, 0.8, d));
}
]]
)
shader:setBlend(true)
shader:setDepthTest(false)
shader:uniformInt("stride", dotsWidth)
shader:uniformVec2("size", imageWidth, imageHeight)
shader:uniformFloat("zoom", zoom * 0.5)

current = 1
function step()
  local next = 1 + (current % 2)

  framebuffers[next]:bind(dotsWidth, dotsHeight)
  compute:uniformTexture("dataTextureA", framebuffers[current])
  compute:uniformFloat("delta", delta)
  compute:render()
  framebuffers[next]:unbind()

  current = next
end

camera.setPosition(0, 0, 2)

log([[
## #013 Compute
Press *CTRL+E* to edit source
]])

-- debugshader = Shader(
-- [[
-- out vec2 vUV;
-- uniform vec2 size;
-- uniform float zoom;
-- void main() {
--   vUV = uv;
--   gl_Position = vec4(
--     (
--       (position * vec3(size * 0.5, 1.0))
--       / vec3(vec2(resolution.x / resolution.y, 1.0) * zoom, 1.0)
--     ),
--     1.0
--   );
-- }
-- ]],
-- [[
-- in vec2 vUV;
-- uniform sampler2D image;
-- void main() {
--   gl_FragColor = sRGB(texture(image, vUV));
-- }
-- ]]
-- )
-- debugshader:uniformFloat("zoom", zoom * 0.5)
-- debugshader:uniformVec2("size", imageWidth, imageHeight)

function loop()
  if image then
    generate()
  else
    -- debugshader:render()
    step()
    shader:uniformTexture("dataTextureA", framebuffers[current])
    shader:render(numDots)
  end
end

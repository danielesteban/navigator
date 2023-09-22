shader = Shader(
[[
out vec2 vUV;
void main() {
  vUV = (uv - 0.5) * 2.0;
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
]],
[[
in vec2 vUV;
uniform vec3 color;
uniform float offset;
uniform float hover;
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+10.0)*x); }
float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187,
                      0.366025403784439,
                     -0.577350269189626,
                      0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy));
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod289(i);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
void main() {
  vec2 id = vec2(offset * 2.0, time) * (1.0 + hover * 4.0);
  float n = snoise((vUV + id) * 0.25);
  n += snoise((vUV + id) * 2.0) * 0.125;
  float d = length(vUV) + n * 0.2 + 0.1;
  float l = 0.5 + fract(gl_FragCoord.y / 3.0) * 0.5;
  gl_FragColor = vec4(color * l, smoothstep(1.0, 0.8, d));
}
]]
)
shader:setBlend(true)

data = {
  { name= "Shader", url="pastebin://J3beKPbc" },
  { name= "Framebuffer", url="pastebin://8FFNtfaU" },
  { name= "Cube", url="pastebin://GPBvpup5" },
  { name= "Physics", url="pastebin://4bb6PKjM" },
  { name= "WASD", url="pastebin://QqQ7L4MH" },
  { name= "Dots", url= "pastebin://hnd0vLja" },
  { name= "Voxels", url="pastebin://YLHcX0Cu" },
  { name= "Pathfind", url="pastebin://iWKb26s9" },
  { name= "SFX", url= "pastebin://rMeGSt2U" },
  { name= "Image", url= "pastebin://5seUKKC3" },
  { name= "IBL", url= "pastebin://u2ifG5xG" },
  { name= "Raymarcher", url= "pastebin://BarAL2ee" },
  { name= "Compute", url="pastebin://ey82FF2W" },
  { name= "Log", url="pastebin://3imN33i4" },
}
numExamples = #data
meshes = {}
for i=1,numExamples do
  local mesh = Mesh("plane", shader)
  mesh:uniformVec3("color", math.hsv((i - 1) / numExamples, 0.5, 1))
  mesh:uniformFloat("offset", math.random() * 2.0)
  meshes[i] = mesh
end

camera.setPosition(0, 0, 12)

function loop()
  raycaster.setFromCamera(mouse.x, mouse.y)
  local hit = raycaster.intersect(table.unpack(meshes))
  for i=1,numExamples do
    local hover = hit == meshes[i]:getId()
    if hover then
      mouse.cursor("hand")
      if mouse.primaryUp then
        navigate(data[i].url)
      end
    end
    shader:uniformFloat("hover", hover and 1 or 0)
    local a = i * math.pi * 2 / numExamples - (math.pi * 2) / numExamples
    local r = 9 + math.sin(time * 4) * 0.05
    local x = math.sin(a) * r
    local y = (math.cos(a) * r) * 0.75 - 0.8
    local z = 0
    meshes[i]:setPosition(x, y, z)
    meshes[i]:render()
    showTooltip(data[i].name, x, y + 1, z)
  end
end

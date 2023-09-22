-- ShaderChunks
-- include pastebin://Rqpqfa7i
-- OrbitControls
-- include pastebin://zv4spBMk

environment = Environment("https://cdn.glitch.global/f95f83f8-1295-420a-ba25-3773d1099c3a/wrestling_gym_2k.hdr")

numEntities = 32
camera.setFov(75)
fov = math.rad(75)
raymarcher = Shader(
"const float cameraFov = " .. fov .. ";"
.. [[
out vec3 ray;
void main() {
  float aspect = resolution.y / resolution.x;
  vec2 uv = vec2(position.x, position.y * aspect);
  float cameraDistance = (1.0 / tan(cameraFov / 2.0)) * aspect;
  ray = normalize(vec3(uv, -cameraDistance) * mat3(viewMatrix));
  gl_Position = vec4(position, 1);
}
]],
"#define NUM_ENTITIES " .. numEntities .. "\n"
.. [[
#define MAX_ITERATIONS 300
#define MAX_DISTANCE 1000
struct Entity {
  vec3 position;
  float radius;
  vec3 color;
  float metalness;
  float roughness;
};
struct SDF {
  float distance;
  vec3 color;
  float metalness;
  float roughness;
};
in vec3 ray;
uniform Entity entities[NUM_ENTITIES];

float sdSphere(const in vec3 p, const in float r) {
  return length(p)-r;
}
SDF opSmoothUnion(const in SDF a, const in SDF b, const in float k) {
  float h = clamp(0.5 + 0.5 * (b.distance - a.distance) / k, 0.0, 1.0);
  return SDF(
    mix(b.distance, a.distance, h) - k*h*(1.0-h),
    mix(b.color, a.color, h),
    mix(b.metalness, a.metalness, h),
    mix(b.roughness, a.roughness, h)
  );
}
SDF map(const in vec3 p) {
  SDF scene;
  for (int i = 0; i < NUM_ENTITIES; i++) {
    SDF entity = SDF(sdSphere(p - entities[i].position, entities[i].radius), entities[i].color, entities[i].metalness, entities[i].roughness);
    if (i == 0) {
      scene = entity;
    } else {
      scene = opSmoothUnion(scene, entity, 1.0);
    }
  }
  return scene;
}
vec3 getNormal(const in vec3 p, const in float d) {
  const vec2 o = vec2(0.001, 0);
  return normalize(
    d - vec3(
      map(p - o.xyy).distance,
      map(p - o.yxy).distance,
      map(p - o.yyx).distance
    )
  );
}
]] .. IBL .. [[
vec4 march() {
  vec4 color;
  float distance;
  for (int i = 0; i < MAX_ITERATIONS && distance < MAX_DISTANCE; i++) {
    vec3 position = viewPosition + ray * distance;
    SDF step = map(position);
    if (step.distance <= 0.01) {
      color = vec4(IBL(
        step.color, getNormal(position, step.distance), position,
        step.metalness, step.roughness
      ), 1);
      break;
    }
    distance += step.distance;
  }
  return color;
}
void main() {
  vec4 color = march();
  color.rgb = color.rgb / (color.rgb + vec3(1.0));
  gl_FragColor = sRGB(color);
}
]]
)
raymarcher:setBlend(true)
raymarcher:setDepthTest(false)
raymarcher:uniformTexture("irradianceMap", environment, "irradiance")
raymarcher:uniformTexture("prefilteredMap", environment, "prefiltered")

skyboxShader = Shader(
[[
out vec3 vPos;
void main() {
  vPos = position;
  gl_Position = projectionMatrix * mat4(mat3(viewMatrix)) * vec4(position, 1.0);
  gl_Position = gl_Position.xyww;
}
]],
[[
in vec3 vPos;
uniform samplerCube environmentMap;
void main() {
  vec3 color = texture(environmentMap, vPos).rgb;
  color = color / (color + vec3(1.0));
  gl_FragColor = sRGB(vec4(color, 1.0));
}
]]
)
skyboxShader:setFaceCulling("front")
skyboxShader:uniformTexture("environmentMap", environment, "cubemap")

skybox = Mesh("box", skyboxShader)

entities = {}
for i=1,numEntities do
  entities[i] = Mesh("sphere", raymarcher)
  local x, y, z = math.normalize(math.random() - 0.5, math.random() - 0.5, math.random() - 0.5)
  local radius = 0.75 + math.random() * 1.5
  entities[i]:setPosition(x * 6, y * 6, z * 6)
  entities[i]:setScale(radius, radius, radius)
  entities[i]:enablePhysics(1, true)
  entities[i]:setDamping(0.9, 0.1)
  raymarcher:uniformFloat("entities[" .. (i - 1) .. "].radius", radius)
  raymarcher:uniformVec3("entities[" .. (i - 1) .. "].color", math.hsv(math.random(), 0.5 + math.random() * 0.5, 0.5 + math.random() * 0.5))
  raymarcher:uniformFloat("entities[" .. (i - 1) .. "].metalness", math.random())
  raymarcher:uniformFloat("entities[" .. (i - 1) .. "].roughness", math.random())
end

pointer = Mesh("sphere")
pointer:setScale(2, 2, 2)
pointer:enablePhysics(1, true, true)
pointer:setDamping(1.0, 0.1)

plane = Mesh("plane")
plane:setScale(1000, 1000, 1)

physics.setGravity(0, 2, 0)

controls = OrbitControls:new({
  radius = 20,
})

log([[
## #012 Raymarcher
Press *CTRL+E* to edit source
]])

function loop()
  if not controls.dragging then
    controls.theta = controls.theta + delta * 0.2
  end
  controls:update()

  plane:lookAt(camera.getPosition());
  raycaster.setFromCamera(mouse.x, mouse.y);
  local hit = raycaster.intersect(plane);
  if hit then
    pointer:setPosition(raycaster.getResult());
  end

  local force = delta * -1000;
  for i=1,numEntities do
    local x, y, z = entities[i]:getPosition()
    raymarcher:uniformVec3("entities[" .. (i - 1) .. "].position", x, y, z)
    x, y, z = math.normalize(x, y, z)
    entities[i]:applyForce(x * force, y * force, z * force)
  end

  skybox:render()
  raymarcher:render()
end

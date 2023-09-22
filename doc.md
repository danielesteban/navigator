# WTF is this?

```
A navigator for the 3D internet.
Although it doesn't support the DOM, javascript or even HTML.
So.. it's only a really tiny part of the internet.
It does have built-in physics support, raycasting, image loaders, positional SFX, and a bunch more.
```

# Why?

```
I dunno. I didn't plan to do this. It just happened.
In fact I was just learning about Lua to do some hot reloading code for some ESP32 boards.
One thing leads to another... I ended up porting the VM code into desktop and made it run some OpenGL shaders.
Then I thought it would be really cool if you could load the Lua scripts over the network, and it hit me:
I just invented the web browser!
So... I added an addres bar and hyperlinks to it and the rest is what you see here.
```

# Globals

##### `delta` Frame delta time

##### `time` Total time since the script started

##### `resolution`
  * `.x` Horizontal viewport resolution in pixels
  * `.y` Vertical viewport resolution in pixels

##### `setClearColor(r, g, b, a)`

##### `camera`
  * `.lookAt(x, y, z)`
  * `.getFov() -> degrees`
  * `.setFov(degrees)`
  * `.getPosition() -> x, y, z`
  * `.setPosition(x, y, z)`

##### `physics`
  * `.getContacts("box" | "capsule" | "cylinder" | "sphere", x, y, z, sx, sy, sz, [flagsMask]) -> x, y, z`
  * `.getContactIds("box" | "capsule" | "cylinder" | "sphere", x, y, z, sx, sy, sz, [flagsMask]) -> id1, id2, ...`
  * `.setGravity(x, y, z)`

##### `raycaster`
  * `.setFromCamera(x, y)`
  * `.getRay() -> x, y, z, dx, dy, dz`
  * `.getResult() -> x, y, z, nx, ny, nz`
  * `.intersect(...meshes) -> id | nil`

```lua
raycaster.setFromCamera(mouse.x, mouse.y)
local hit = raycaster.intersect(table.unpack(meshes))
for i=1,#meshes then
  if hit == meshes[i]:getId() then
    if mouse.primary then
      -- make the mesh red while cursor is on top
      -- and the primary button is pressed
      meshes[i]:uniformVec3("color", 1, 0, 0)
    elseif mouse.secondary then
      -- make the mesh green while cursor is on top
      -- and the secondary button is pressed
      meshes[i]:uniformVec3("color", 0, 1, 0)
    else
      -- make the mesh blue while cursor is on top
      -- and no button is pressed
      meshes[i]:uniformVec3("color", 0, 0, 1)
    end

    -- show a tooltip while cursor is on top of mesh
    local x, y, z = ray.getResult()
    showTooltip("Hi!", x, y, z)
  else
    -- make the mesh white in any other case (the default)
    meshes[i]:uniformVec3("color", 1, 1, 1)
  end
  meshes[i]:render()
end
```

##### [Lua's basic functions](https://www.lua.org/manual/5.3/manual.html#6.1)
  * `.collectgarbage()`
  * `.ipairs(t)`
  * `.pairs(t)`
  * `.setmetatable(table, metatable)`
  * `.tonumber(str, [base])`
  * `.tostring(n)`

##### `json`
  * `.encode(table)`
  * `.decode(str) -> table`

##### `math`
  * `.clamp(value, min, max) -> float`
  * `.cross(ax, ay, az, bx, by, bz) -> x, y, z`
  * `.dot(ax, ay, az, bx, by, bz) -> float`
  * `.hsv(h, s, v) -> r, g, b`
  * `.lerp(a, b, d) -> float`
  * `.normalize(x, y, z) -> x, y, z`
  * `.spherical(phi, theta, radius) -> x, y, z`
  * `.srgb(r, g, b) -> r, g, b`
  * [Lua's mathematical functions](https://www.lua.org/manual/5.3/manual.html#6.7)

##### `table`
  * [Lua's table manipulation functions](https://www.lua.org/manual/5.3/manual.html#6.6)

##### `navigate(url)`

```lua
if raycaster.intersect(mesh) then
  mouse.cursor("hand")
  if mouse.primaryUp then
    navigate("https://example.com/script.lua")
  end
end
```

# Input

##### `mouse`
  * `.x` Normalized horizontal screen position
  * `.y` Normalized vertical screen position
  * `.dx` Horizontal movement since last frame
  * `.dy` Vertical movement since last frame
  * `.wheel` Wheel movement since last frame
  * `.hover` True when the cursor is over the navigator window
  * `.locked` True when the cursor is locked
  * `.primary` True on every frame the button is pressed
  * `.primaryDown` True only during the first frame after press
  * `.primaryUp` True only during the first frame after release
  * `.secondary` True on every frame the button is pressed
  * `.secondaryDown` True only during the first frame after press
  * `.secondaryUp` True only during the first frame after release
  * `.cursor("arrow" | "hand" | "none")`
  * `.unlock()` Unlocks the input (if locked)
  * `.lock()`

```lua
if not mouse.locked and (mouse.primaryDown || mouse.secondaryDown) then
  -- (button.primary or button.secondary) must be true
  -- during the frame of the lock request
  -- or it will be rejected
  mouse.lock()
end
if mouse.locked then
  log("dx:" .. (mouse.dx * resolution.x))
  log("dy:" .. (mouse.dy * resolution.y))
end
```

##### `keyboard(key) -> isPressed`

```lua
local movement = { x = 0, z = 0 }
if keyboard("w") then
  movement.z = 1
elseif keyboard("s") then
  movement.z = -1
end
if keyboard("d") then
  movement.x = 1
elseif keyboard("a") then
  movement.x = -1
end
if movement.x ~= 0 or movement.z ~= 0 then
  x, y, z = math.normalize(movement.x, 0, movement.z)
  log("movementX:" .. x .. " - movementZ:" .. z)
end
```

# Loaders

##### `Environment(url)` *.hdr
  * `:isReady() -> bool`

##### `Image(url, ['srgb' | 'linear'])` *.jpg, *.png
  * `:isReady() -> bool`
  * `:getSize() -> x, y`

##### `SFX(url)` *.mp3, *.ogg
  * `:isReady() -> bool`
  * `:play(x, y, z)`

##### `HTTP(url, [body])`
  * `:isReady() -> bool`
  * `:getResponse() -> text`

```lua
getData = HTTP("https://example.com/get")
postData = HTTP("https://example.com/post", json.encode({ msg = "Hi!" }))

function loop()
  if getData and getData:isReady() then
    json = json.decode(getData:getResponse())
    getData = nil
    log("Received data:\n" .. json.msg)
  end
  if postData and postData:isReady() then
    postData = nil
    log("Data has been sent!")
  end
end
```

# Objects

##### `Framebuffer(textures=1, depthBuffer=false, [samples])`
  * `:bind(width, height)`
  * `:blit([Framebuffer])`
  * `:clear()`
  * `:setTextureClearColor(index, r, g, b, a)`
  * `:setTextureData(index, r, g, b, a, ...)`
  * `:unbind()`
  * `:render()`

##### `Geometry()`
  * `:setColliders("box" | "capsule" | "cylinder" | "sphere", x, y, z, sx, sy, sz, ...)`
  * `:setIndex(i1, i2, i3...)`
  * `:setVertices(x, y, z, nx, ny, nz, u, v, ...)`

##### `Mesh(Geometry | "box" | "plane" | "sphere", Shader)`
  * `:getId() -> id`
  * `:getPosition() -> x, y, z`
  * `:setPosition(x, y, z)`
  * `:getScale() -> x, y, z`
  * `:setScale(x, y, z)`
  * `:lookAt(x, y, z)`
  * `:getFlags() -> flags`
  * `:setFlags(flags)`
  * `:getFrustumCulling() -> enabled`
  * `:setFrustumCulling(enabled)`
  * `:enablePhysics(mass, isAlwaysActive = false, isKinematic = false)` mass == 0 -> static | mass > 0 -> dynamic
  * `:disablePhysics()`
  * `:applyForce(x, y, z)` Must enable physics first
  * `:applyImpulse(x, y, z)` Must enable physics first
  * `:setDamping(linear, angular)` Must enable physics first
  * `:getAngularVelocity()` Must enable physics first
  * `:setAngularVelocity(x, y, z)` Must enable physics first
  * `:getLinearVelocity()` Must enable physics first
  * `:setLinearVelocity(x, y, z)` Must enable physics first
  * `:getContacts([flagsMask]) -> x, y, z` Must enable physics first
  * `:getContactIds([flagsMask]) -> id1, id2, ...` Must enable physics first
  * `:uniformInt(name, value)`
  * `:uniformFloat(name, value)`
  * `:uniformTexture(name, Environment | Image | Framebuffer, [index])`
  * `:uniformVec2(name, x, y)`
  * `:uniformVec3(name, x, y, z)`
  * `:uniformVec4(name, x, y, z, w)`
  * `:render()`

##### `Voxels(Shader)`
  * `:getId() -> id`
  * `:get(x, y, z) -> type, r, g, b`
  * `:set(x, y, z, 0 | 1 | 2, [r], [g], [b])` 0 == air | 1 == solid | 2 == obstacle
  * `:getFlags() -> flags`
  * `:setFlags(flags)`
  * `:enablePhysics()`
  * `:disablePhysics()`
  * `:ground(x, y, z) -> closestY | nil`
  * `:pathfind(fromX, fromY, fromZ, toY, toX, toZ, [height = 1])`
  * `:render()`

##### `Noise(encodedFastNoiseNodeTree)` (use [Noise Tool](https://github.com/Auburn/FastNoise2#noise-tool) to generate)
  * `:get2D(x, y, seed) -> float`
  * `:get3D(x, y, z, seed) -> float`

# Scripts

```lua
-- Includes
-- This will prepend the library script code:
-- include https://example.com/library.lua
-- Keep in mind there's no deduplication or cache.
-- The includes can have comments in between.
-- (even here too) include https://example.com/thisWillGetIncluded.lua
-- But they all must be declared inside the top block of the script.

-- This won't get included:
-- include https://example.com/thisWontGetIncluded.lua

-- Put your setup/init code here
log("this will be called once at load")

function loop()
  -- Put you animation/render code here
  log("this will be called every frame")
end
```

# Shaders

##### `Shader(vertexShader, fragmentShader)`
  * `:getBlend() -> enabled`
  * `:setBlend(enabled)`
  * `:getDepthTest() -> enabled`
  * `:setDepthTest(enabled)`
  * `:getFaceCulling() -> mode`
  * `:setFaceCulling("back" | "front" | "none")`
  * `:uniformInt(name, value)`
  * `:uniformFloat(name, value)`
  * `:uniformTexture(name, Environment | Image | Framebuffer, [index])`
  * `:uniformVec2(name, x, y)`
  * `:uniformVec3(name, x, y, z)`
  * `:uniformVec4(name, x, y, z, w)`
  * `:render()` Render shader into a fullscreen quad

##### `Default attributes:`

  * `vec3 position`
  * `vec3 normal`
  * `vec2 uv`
  * `vec3 color`

##### `Default uniforms:`

  * `mat3 normalMatrix`
  * `mat4 modelMatrix`
  * `mat4 viewMatrix`
  * `vec3 viewPosition`
  * `mat4 projectionMatrix`
  * `vec2 resolution`
  * `float time`

```lua
shader = Shader(
-- Vertex Shader
[[
out vec3 vColor;
out vec3 vNormal;
out vec2 vUV;
void main() {
  vColor = color;
  vNormal = normal;
  vUV = uv;
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
]],
-- Fragment Shader
[[
in vec3 vColor;
in vec3 vNormal;
in vec2 vUV;
void main() {
  gl_FragColor = sRGB(vec3(normalize(vNormal) * 0.5 + 0.5, 1.0));
}
]]
)
```

# UI

##### `log(msg)` Logs a message (persists until cleared)

```lua
log([[
# Heading
Some text *darken* **darker**
[A Link](pastebin://vQurzZEY) 
]])
```

##### `clearLog()` Clears the current log

```lua
if keyboard("C") then
  clearLog()
end
```

##### `showTooltip("text", x, y, z, [offsetX], [offsetY])`

```lua
if raycaster.intersect(mesh) then
  mouse.cursor("hand")
  local x, y, z = mesh:getPosition()
  showTooltip("Click me!", x, y + 1, z)
end
```

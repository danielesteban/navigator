shader = Shader(
[[
out vec2 vUV;
void main() {
  vUV = uv;
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
]],
[[
in vec2 vUV;
uniform sampler2D image;
void main() {
  gl_FragColor = sRGB(texture(image, vUV));
}
]]
)

camera.setPosition(0, 0, 2)
image = Image("https://media.posterlounge.com/img/products/660000/653583/653583_poster_l.jpg")

mesh = Mesh("plane", shader)
mesh:uniformTexture("image", image)

log([[
## #010 Image
Press *CTRL+E* to edit source
]])

function loop()
  mesh:render()
end

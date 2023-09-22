WASDControls = {
  position = { x = 0, y = 0, z = 0 },
  phi = math.pi * 0.5,
  theta = math.pi,
  front = { x = 0, y = 0, z = -1 },
  right = { x = 1, y = 0, z = 0 },
}

function WASDControls:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  o.dphi = o.phi
  o.dtheta = o.theta
  o.target = { x = o.position.x, y = o.position.y, z = o.position.z }
  return o
end

function WASDControls:update()
  local damp = 1 - math.exp(-30 * delta)
  local x, y, z

  if not mouse.locked and mouse.primaryDown then
    mouse.lock()
    mouse.primaryDown = false
  end

  if mouse.locked then
    self.phi = math.clamp(self.phi - mouse.dy * resolution.y * 0.0015, 0.0001, math.pi - 0.0001)
    self.theta = self.theta - mouse.dx * resolution.x * 0.0015
  end

  if (
    math.abs(self.phi - self.dphi) > 0.0001
    or math.abs(self.theta - self.dtheta) > 0.0001
  ) then
    self.dphi = math.lerp(self.dphi, self.phi, damp)
    self.dtheta = math.lerp(self.dtheta, self.theta, damp)

    x, y, z = math.spherical(self.dphi, self.dtheta, 1)
    self.front.x = x
    self.front.y = y
    self.front.z = z
    x, y, z = math.cross(x, y, z, 0, 1, 0)
    x, y, z = math.normalize(x, y, z)
    self.right.x = x
    self.right.y = y
    self.right.z = z
  end

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
    movement.x = x
    movement.z = z
  end

  local step = 10.0 * (keyboard("shift") and 2 or 1) * delta
  self.target.x = self.target.x + self.front.x * movement.z * step + self.right.x * movement.x * step
  self.target.y = self.target.y + -10 * delta
  self.target.z = self.target.z + self.front.z * movement.z * step + self.right.z * movement.x * step

  x, y, z = physics.getContacts("capsule", self.target.x, self.target.y, self.target.z, 1, 2.0, 1)
  self.target.x = self.target.x + x
  self.target.y = self.target.y + y
  self.target.z = self.target.z + z

  self.position.x = math.lerp(self.position.x, self.target.x, damp)
  self.position.y = math.lerp(self.position.y, self.target.y, damp)
  self.position.z = math.lerp(self.position.z, self.target.z, damp)
  camera.setPosition(self.position.x, self.position.y + 1, self.position.z)
  camera.lookAt(self.position.x + self.front.x, self.position.y + 1 + self.front.y, self.position.z + self.front.z)
end

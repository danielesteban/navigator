OrbitControls = {
  dragging = false,
  maxRadius = 100,
  minRadius = 1,
  phi = math.pi * 0.5,
  theta = math.pi,
  radius = 10,
  target = { x = 0, y = 0, z = 0 },
  dragStart = { x = 0, y = 0 },
  orbitDamp = 20,
  targetDamp = 5,
}

function OrbitControls:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  o.dphi = o.phi
  o.dtheta = o.theta
  o.dradius = o.radius
  o.dtarget = { x = o.target.x, y = o.target.y, z = o.target.z }
  OrbitControls.setView(o)
  return o
end

function OrbitControls:update()
  if mouse.primaryDown then
    self.dragStart.x = mouse.x
    self.dragStart.y = mouse.y
  end
  if mouse.primaryUp then
    self.dragging = false
  end

  if self.dragging then
    self.phi = math.clamp(self.phi + mouse.dy * resolution.y * 0.003, 0.0001, math.pi - 0.0001)
    self.theta = self.theta - mouse.dx * resolution.x * 0.003
  elseif mouse.primary and (math.abs(self.dragStart.x - mouse.x) * resolution.x > 10 or math.abs(self.dragStart.y - mouse.y) * resolution.y > 10) then
    self.dragging = true
  end

  if mouse.wheel ~= 0 then
    self.radius = math.clamp(self.radius * (1 - mouse.wheel * 0.1), self.minRadius, self.maxRadius)
  end

  local needsUpdate = false

  if (
    math.abs(self.phi - self.dphi) > 0.0001
    or math.abs(self.theta - self.dtheta) > 0.0001
    or math.abs(self.radius - self.dradius) > 0.0001
  ) then
    local damp = 1 - math.exp(-self.orbitDamp * delta)
    self.dphi = math.lerp(self.dphi, self.phi, damp)
    self.dtheta = math.lerp(self.dtheta, self.theta, damp)
    self.dradius = math.lerp(self.dradius, self.radius, damp)
    needsUpdate = true
  end

  if (
    math.abs(self.target.x - self.dtarget.x) > 0.0001
    or math.abs(self.target.y - self.dtarget.y) > 0.0001
    or math.abs(self.target.z - self.dtarget.z) > 0.0001
  ) then
    local damp = 1 - math.exp(-self.targetDamp * delta)
    self.dtarget.x = math.lerp(self.dtarget.x, self.target.x, damp)
    self.dtarget.y = math.lerp(self.dtarget.y, self.target.y, damp)
    self.dtarget.z = math.lerp(self.dtarget.z, self.target.z, damp)
    needsUpdate = true
  end

  if needsUpdate then
    OrbitControls.setView(self)
  end
end

function OrbitControls:setView()
  local x, y, z = math.spherical(self.dphi, self.dtheta, self.dradius)
  camera.setPosition(self.dtarget.x + x, self.dtarget.y + y, self.dtarget.z + z)
  camera.lookAt(self.dtarget.x, self.dtarget.y, self.dtarget.z)
end

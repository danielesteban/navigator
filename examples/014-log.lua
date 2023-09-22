frame = 0
function loop()
  clearLog()
  log("# #014 Log")
  log("*Frame:* " .. frame)
  log("*Time:* " .. math.floor(time * 10) / 10)
  log(
    "*Mouse:* **x** " .. math.floor((mouse.x * 0.5 + 0.5) * 100) .. "%"
    .. " **y** " .. math.floor((mouse.y * 0.5 + 0.5) * 100) .. "%"
    .. "  |  *Spacebar:* " .. (keyboard(" ") and "pressed" or "released")
  )
  log("*Random quote:* " .. getQuote())
  frame = frame + 1
end

quote = nil
request = HTTP("https://zenquotes.io/api/random")
function getQuote()
  if quote then
    return quote
  end
  if request and request:isReady() then
    quote = json.decode(request:getResponse())[1].q
    request = nil
    collectgarbage()
    return quote
  end
  return "**Fetching...**"
end

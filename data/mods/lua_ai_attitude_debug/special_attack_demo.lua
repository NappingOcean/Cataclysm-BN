local demo = {}

local attack_id = "demo_tap"

---@param mon Monster
---@param target Creature|nil
local function demo_attitude(mon, target)
  -- Hostile intent lets the existing melee actor select the avatar as its target.
  -- The demonstration attack has zero damage and the AI never uses normal melee.
  return MonsterAttitude.MATT_ATTACK
end

---@param mon Monster
local function demo_turn(mon)
  local avatar = gapi.get_avatar()
  local before_moves = mon:get_moves()
  local step = (tonumber(mon:get_value("special_demo_step")) or 0) + 1
  mon:set_value("special_demo_step", tostring(step))

  local result
  if not mon:has_special_attack(attack_id) then
    result = "MISSING: this monster does not have demo_tap."
  elseif not mon:special_attack_ready(attack_id) then
    result = "WAIT: attack not ready; waiting for cooldown."
  else
    mon:set_target(avatar)
    local used = mon:use_special_attack(attack_id)
    if used then
      result = "USED: actor handled the attack; cooldown started."
    else
      result = "FAILED: actor could not attack; cooldown not consumed."
    end
    result = result .. " Ready afterward: " .. tostring(mon:special_attack_ready(attack_id))
    result = result .. "; actor move cost: " .. tostring(before_moves - mon:get_moves()) .. "."
  end

  if avatar ~= nil then
    local pos = mon:get_pos_ms()
    local target_pos = avatar:get_pos_ms()
    if pos.z == target_pos.z and math.max(math.abs(pos.x - target_pos.x), math.abs(pos.y - target_pos.y)) <= 10 then
      gapi.add_msg(MsgType.info, string.format("[Special demo AI #%d] %s", step, result))
    end
  end

  -- Spend at least one standard action even when the actor fails or is waiting.
  -- Keep the actor's own cost, topping it up only if it spent fewer than 100 moves.
  local spent = before_moves - mon:get_moves()
  if spent < 100 then mon:mod_moves(-(100 - spent)) end
  -- Handle the entire action so the normal special-attack scheduler never runs here.
  return true
end

function demo.register()
  game.monster_attitude_functions["lua_special_attack_demo_attitude"] = demo_attitude
  game.monster_ai_functions["lua_special_attack_demo"] = demo_turn
end

return demo

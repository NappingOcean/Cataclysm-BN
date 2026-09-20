local attack_id = "demo_tap"

---@param mon Monster
---@param target Creature|nil
---@return MonsterAttitude
local function demo_attitude(mon, target)
  local _ = mon
  if target ~= nil and target:is_avatar() then return MonsterAttitude.MATT_ATTACK end
  return MonsterAttitude.MATT_IGNORE
end

---@param mon Monster
---@return boolean
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

  local pos = mon:get_pos_ms()
  local target_pos = avatar:get_pos_ms()
  if pos.z == target_pos.z and math.max(math.abs(pos.x - target_pos.x), math.abs(pos.y - target_pos.y)) <= 10 then
    gapi.add_msg(MsgType.info, string.format("[Special demo AI #%d] %s", step, result))
  end

  -- Spend at least one standard action even when the actor fails or is waiting.
  -- Preserve the actor's own cost, adding only the unspent portion.
  local spent = before_moves - mon:get_moves()
  if spent < 100 then mon:mod_moves(-(100 - spent)) end

  -- This AI handled the action, so stock AI and its special scheduler do not run.
  return true
end

game.monster_attitude_functions["lua_special_attack_demo_attitude"] = demo_attitude
game.monster_ai_functions["lua_special_attack_demo"] = demo_turn

gdebug.log_info("lua_special_attack_demo: ready.")

# Lua special attack demo

This standalone debug mod demonstrates these `Monster` Lua methods:

- `has_special_attack(attack_id)`
- `special_attack_ready(attack_id)`
- `use_special_attack(attack_id)`

It does not depend on or modify the Lua AI Examples mod.

## Running the demonstration

1. Enable **Lua Special Attack Demo** in a test world and restart the game.
2. From the debug monster menu, spawn **Lua special attack demonstrator**
   (`mon_lua_special_attack_demo`) on an empty tile 4–5 tiles away in clear sight.
   Spawn only one so its messages are easy to follow.
3. Advance time one action at a time. Disable safe mode if it prevents waiting near the
   hostile-intent training robot. The robot is immobile and its attack deals zero damage.
4. Monster creation randomizes the initial cooldown. Once ready, attempts from several
   tiles away report `FAILED`, `Ready afterward: true`, and actor move cost `0`.
   This shows that readiness does not predict target or range checks and that actor failure
   does not consume cooldown.
5. Walk next to the robot and advance a turn. It reports `USED`,
   `Ready afterward: false`, and actor move cost `100`. A dodge still counts as handled use.
6. Stay adjacent. It reports `WAIT` until the normal cooldown expires, then uses the attack
   again. Move away after cooldown to see failed attempts resume.

`AI #` counts Lua AI calls, not seconds or player turns. Player speed and action duration can
change how many messages appear per player action. The configured cooldown is 7; the Lua code
neither reads its remaining value nor changes it. Messages are limited to the same z-level and
10-tile range.

The Lua AI invokes the actor at most once per action, spends at least 100 moves, and returns
`true` so the stock AI and stock special-attack scheduler do not run for this monster.

# Lua AI debug helpers

- Enable this mod in a test world and spawn the `lua_ai_debug_remote` from the debug item menu.
- Modes (select from the menu): `attack` (engage target, optional tile pick), `follow` (trail 1–2 tiles away), `ignore` (idle), `fetch` (pick tile/item, return it), `mine` (pick center, walk 5x5 and bash tiles).
- Use `lua_ai_debug_selector` to apply the chosen mode to nearby debug drones without spawning new ones.
- Example `Lua dance bot` (`mon_lua_dancer`) shows a bespoke `lua_ai_dance` + `lua_attitude_dance`; spawn it from the debug monster menu to watch it loop a square dance and “sing”.

## Special attack demonstration

Requires a game binary with `Monster:has_special_attack`, `special_attack_ready`, and
`use_special_attack`. Enable **Lua AI Examples** in a test world; `test_data` is not needed.
Restart the game after adding these files so the new monster JSON is loaded.

1. Use the debug monster menu to spawn **Lua special attack demonstrator**
   (`mon_lua_special_attack_demo`) on an empty tile 4–5 tiles away, in clear sight.
   Spawn only one to keep the messages easy to follow.
2. Advance time one action at a time. If safe mode blocks waiting near this hostile-intent
   training robot, turn safe mode off for the demonstration. It stays in place.
3. The initial cooldown is randomized by normal monster creation. After it expires,
   messages should repeatedly show `FAILED`, `Ready afterward: true`, and actor move cost `0`.
   Readiness does not guarantee that the melee actor has an adjacent target.
4. Walk next to the robot and wait. It should report `USED`, `Ready afterward: false`, and
   actor move cost `100`. The existing melee actor performs a zero-damage tap; dodging still
   counts as use. Use a plain test character without retaliation abilities.
5. Stay adjacent. Expect `WAIT` messages until the normal cooldown expires, then another
   `USED`. Move away again to see failures resume once the attack is ready.

`AI #` counts Lua AI calls, not seconds or player turns. Player speed and action duration can
change how many messages appear per player action. The attack's configured cooldown is 7;
the script neither reads its remaining value nor changes it. Logs are limited to the same
z-level and a distance of 10 tiles.

The script checks ownership and readiness, then calls the actor at most once per AI action.
It delegates target/range checks and attack cost to the actor. It spends at least 100 moves
per action and returns `true`, preventing idle retry loops and normal AI fallback.
The other drones and the remote's existing modes are unchanged.

Implementation: `special_attack_demo.lua` and `special_attack_demo.json`.

## Concepts

- `lua_attitude` answers "how does this monster feel about this target right now?" It returns a `MonsterAttitude` such as `MATT_ATTACK`, `MATT_FOLLOW`, `MATT_FRIEND`, or `MATT_IGNORE`.
- `lua_attitude` is queried from `monster::attitude()` before the normal anger/morale/faction mood logic is used, so it is the right tool when you want to override target relationship or threat evaluation.
- `lua_ai` answers "what does this monster do on its turn?" It runs the bespoke per-turn behavior and can move, bash, fetch, idle, or otherwise spend moves however it wants.
- `lua_ai` is the right tool when stock monster movement is not enough and you want scripted behavior instead of just a different mood.
- In practice: `lua_attitude` picks intent, while `lua_ai` implements the actual turn behavior.

## Examples

- `mon_lua_ai_debug` uses both: `lua_attitude_debug_mode` changes whether the drone treats targets as hostile, follow-worthy, or ignorable, while `lua_ai_debug_seek` makes it walk, fetch, mine, or attack based on the selected debug mode.
- `mon_lua_dancer` also uses both, but in a simpler way: `lua_attitude_dance` keeps it friendly to the avatar, while `lua_ai_dance` gives it a completely bespoke square-dance movement pattern and sound behavior.
- If you only need a monster to stop attacking or start following, `lua_attitude` may be enough.
- If you need a monster to patrol, collect items, orbit an anchor, or perform a custom action loop, you need `lua_ai`.

## Layout

- `preload.lua`: thin bootstrap that loads the mod runtime entrypoint.
- `main.lua`: menu flow, mode selection, item-use entrypoints, spawning, and applying settings to nearby debug drones.
- `ai_behaviors.lua`: named `lua_attitude_*` / `lua_ai_*` registrations and the concrete behavior implementations behind them.
- `lib/utils.lua`: shared coordinate serialization/deserialization and small movement helpers used by the other modules.

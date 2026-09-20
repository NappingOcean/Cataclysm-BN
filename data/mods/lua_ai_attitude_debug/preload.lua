gdebug.log_info("lua_ai_attitude_debug: preload online.")

local main = require("main")

main.register()

local special_attack_demo = require("special_attack_demo")
special_attack_demo.register()

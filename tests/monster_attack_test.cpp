#include "../src/map/map.h"
#include "avatar.h"
#include "calendar.h"
#include "cata_utility.h"
#include "catch/catch.hpp"
#include "coordinates.h"
#include "game.h"
#include "item.h"
#include "map_helpers.h"
#include "monattack.h"
#include "monster.h"
#include "state_helpers.h"
#include "type_id.h"

#include <optional>
#include <string>
#include <vector>

namespace {
const auto effect_dazed = efftype_id("dazed");
const auto effect_shrieking = efftype_id("shrieking");

struct shriek_stun_setup {
    monster& screecher;
    avatar& target;
};

auto setup_shriek_stun_test() -> shriek_stun_setup {
    clear_all_state();

    auto& target = get_avatar();
    const auto screecher_pos = tripoint_bub_ms(60, 60, 0);
    const auto target_pos = tripoint_bub_ms(62, 60, 0);
    target.setpos(map_local_to_abs(get_map(), target_pos));

    auto& screecher = spawn_test_monster("mon_zombie_screecher", screecher_pos);
    screecher.set_dest(target.bub_pos());
    screecher.add_effect(effect_shrieking, 1_minutes);

    return {.screecher = screecher, .target = target};
}

} // namespace

TEST_CASE(
    "special attack dispatch preserves actor and cooldown contracts", "[monster][special_attack]") {
    auto test_type = mtype_id("mon_test_special_attack").obj();
    test_type.special_attacks.clear();
    auto attack = mtype_special_attack("test", [](monster* mon) -> bool {
        mon->mod_moves(-17);
        return mon->anger > 0;
    });
    test_type.special_attacks.emplace("test", attack);
    auto mon = monster(mtype_id("mon_test_special_attack"));
    mon.type = &test_type;
    mon.set_special("test", 0);
    mon.moves = 100;
    mon.anger = 1;

    SECTION("missing definition or state never dispatches") {
        CHECK_FALSE(mon.has_special_attack("missing"));
        CHECK_FALSE(mon.use_special_attack("missing"));
        test_type.special_attacks.erase("test");
        CHECK_FALSE(mon.has_special_attack("test"));
        CHECK_FALSE(mon.special_attack_ready("test"));
        CHECK_FALSE(mon.use_special_attack("test"));
        CHECK(mon.moves == 100);
    }
    SECTION("definition without runtime state never dispatches") {
        mon.poly(mtype_id("debug_mon"));
        mon.type = &test_type;
        mon.moves = 100;
        CHECK_FALSE(mon.has_special_attack("test"));
        CHECK_FALSE(mon.special_attack_ready("test"));
        CHECK_FALSE(mon.use_special_attack("test"));
        CHECK(mon.moves == 100);
    }
    SECTION("only enabled zero cooldown attacks dispatch") {
        const auto cooldown = GENERATE(-1, 0, 1);
        const auto enabled = GENERATE(false, true);
        mon.set_special("test", cooldown);
        if (!enabled) { mon.disable_special("test"); }
        CHECK(mon.has_special_attack("test"));
        CHECK(mon.special_attack_ready("test") == (enabled && cooldown == 0));
        CHECK(mon.use_special_attack("test") == (enabled && cooldown == 0));
        CHECK(mon.moves == (enabled && cooldown == 0 ? 83 : 100));
        if (enabled) { CHECK(mon.shortest_special_cooldown() == cooldown); }
    }
    SECTION("actor false does not roll back effects or consume cooldown") {
        mon.anger = 0;
        CHECK_FALSE(mon.use_special_attack("test"));
        CHECK(mon.moves == 83);
        CHECK(mon.special_attack_ready("test"));
    }
    SECTION("zero default cooldown remains ready after use") {
        CHECK(mon.use_special_attack("test"));
        CHECK(mon.special_attack_ready("test"));
    }
    SECTION("successful actor can disable itself without being reenabled") {
        test_type.special_attacks.at(
            "test") = mtype_special_attack("test", [](monster* target) -> bool {
            target->disable_special("test");
            return true;
        });
        CHECK(mon.use_special_attack("test"));
        CHECK(mon.has_special_attack("test"));
        CHECK_FALSE(mon.special_attack_ready("test"));
    }
    SECTION("successful actor resets the new type's cooldown after transforming") {
        test_type.special_attacks.at(
            "test") = mtype_special_attack("test", [](monster* target) -> bool {
            target->poly(mtype_id("mon_test_special_attack"));
            target->set_special("test", 55);
            return true;
        });
        CHECK(mon.use_special_attack("test"));
        CHECK(mon.shortest_special_cooldown() == 7);
    }
    SECTION("actor can transform and remove the used attack") {
        test_type.special_attacks.at(
            "test") = mtype_special_attack("test", [](monster* target) -> bool {
            target->poly(mtype_id("debug_mon"));
            return true;
        });
        CHECK(mon.use_special_attack("test"));
        CHECK_FALSE(mon.has_special_attack("test"));
    }
}

TEST_CASE("special attack enable state is queryable and reversible", "[monster][special_attack]") {
    clear_all_state();
    const auto cleanup = on_out_of_scope([]() { clear_all_state(); });
    // clear_all_state() parks the avatar on (60, 60, 0), which would block the spawn.
    get_avatar().setpos(map_local_to_abs(get_map(), tripoint_bub_ms(65, 60, 0)));
    auto& mon = spawn_test_monster("mon_test_special_attack_pair", tripoint_bub_ms(60, 60, 0));

    SECTION("ids enumerate the current type's attacks, sorted") {
        CHECK(mon.special_attack_ids() == std::vector<std::string>{"alpha", "beta"});
    }
    SECTION("attacks start enabled and toggle both ways") {
        CHECK(mon.special_attack_enabled("alpha"));
        mon.set_special_attack_enabled("alpha", false);
        CHECK_FALSE(mon.special_attack_enabled("alpha"));
        CHECK_FALSE(mon.special_attack_ready("alpha"));
        // The attack is still present, just dormant.
        CHECK(mon.has_special_attack("alpha"));
        mon.set_special_attack_enabled("alpha", true);
        CHECK(mon.special_attack_enabled("alpha"));
    }
    SECTION("toggling does not touch the cooldown") {
        mon.set_special("alpha", 4);
        mon.set_special_attack_enabled("alpha", false);
        CHECK(mon.get_special_attack_cooldown("alpha") == 4);
        mon.set_special_attack_enabled("alpha", true);
        CHECK(mon.get_special_attack_cooldown("alpha") == 4);
    }
    SECTION("unknown attacks report absent rather than enabled") {
        CHECK_FALSE(mon.special_attack_enabled("missing"));
        CHECK(mon.get_special_attack_cooldown("missing") == std::nullopt);
    }
}

TEST_CASE("hearing protection blocks screecher daze", "[monster][sound]") {
    const auto protected_item = GENERATE("ear_plugs", "army_powered_earmuffs_on");
    CAPTURE(protected_item);

    auto setup = setup_shriek_stun_test();
    REQUIRE(!setup.target.wear_item(item::spawn(protected_item), false));

    REQUIRE(mattack::shriek_stun(&setup.screecher));

    CHECK_FALSE(setup.target.has_effect(effect_dazed));
}

TEST_CASE("screecher dazes unprotected targets", "[monster][sound]") {
    auto setup = setup_shriek_stun_test();

    REQUIRE(mattack::shriek_stun(&setup.screecher));

    CHECK(setup.target.has_effect(effect_dazed));
}

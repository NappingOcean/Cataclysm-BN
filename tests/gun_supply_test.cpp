#include "avatar.h"
#include "calendar.h"
#include "catch/catch.hpp"
#include "gun_mode.h"
#include "gun_supply/gun_supply.h"
#include "item.h"
#include "item_functions.h"
#include "map/map.h"
#include "player_helpers.h"
#include "ranged.h"
#include "rng.h"
#include "state_helpers.h"
#include "type_id.h"
#include "units.h"

#include <optional>
#include <string>
#include <vector>

namespace {

const auto itype_battery = itype_id("battery");
const auto itype_battery_supply_gunmod = itype_id("battery_supply_gunmod");
const auto itype_laser_rifle = itype_id("laser_rifle");
const auto itype_medium_battery_cell = itype_id("medium_battery_cell");
const auto itype_test_supply_only_laser = itype_id("test_supply_only_laser");
const auto itype_UPS_off = itype_id("UPS_off");
const auto itype_adv_UPS_off = itype_id("adv_UPS_off");
const auto ammo_battery = ammotype("battery");

const auto shooter_pos = tripoint_bub_ms(60, 60, 0);
const auto target_pos = shooter_pos + tripoint_rel_ms(5, 0, 0);

/// Wields @p gun_id, optionally with a supply gunmod holding @p supply_charges.
auto wield_gun(const itype_id& gun_id, std::optional<int> supply_charges) -> item& {
    auto& you = get_avatar();
    you.setpos(shooter_pos);

    auto gun = item::spawn(gun_id);
    if (supply_charges) {
        auto mod = item::spawn(itype_battery_supply_gunmod);
        auto cell = item::spawn(itype_medium_battery_cell);
        cell->ammo_set(itype_battery, *supply_charges);
        mod->put_in(std::move(cell));
        gun->put_in(std::move(mod));
    }
    you.wield(std::move(gun));
    return you.primary_weapon();
}

auto give_ups(int charges) -> void {
    get_avatar().i_add(item::spawn(itype_UPS_off, calendar::turn, charges));
}

auto ups_charges() -> int { return get_avatar().charges_of(itype_id("UPS")); }

auto weapon_checks_pass(item& gun) -> bool {
    auto messages = std::vector<std::string>{};
    return ranged::gunmode_checks_weapon(get_avatar(), get_map(), messages, gun.gun_current_mode());
}

} // namespace

TEST_CASE("ups_gun_without_supply_keeps_drawing_from_ups", "[gun][supply]") {
    clear_all_state();
    auto& gun = wield_gun(itype_laser_rifle, std::nullopt);
    give_ups(100);

    CHECK(gun_supply::shots_available(get_avatar(), gun) == 2);
    REQUIRE(ranged::fire_gun(get_avatar(), target_pos, 1) == 1);
    CHECK(ups_charges() == 60);
}

TEST_CASE("ups_gun_draws_from_supply_before_ups", "[gun][supply]") {
    clear_all_state();
    rng_set_engine_seed(0);
    auto& gun = wield_gun(itype_laser_rifle, 50);
    give_ups(100);

    REQUIRE(gun_supply::source_charges(gun, ammo_battery) == 50);
    CHECK(gun_supply::shots_available(get_avatar(), gun) == 3);

    SECTION("a full shot comes from the supply") {
        REQUIRE(ranged::fire_gun(get_avatar(), target_pos, 1) == 1);
        CHECK(gun_supply::source_charges(gun, ammo_battery) == 10);
        CHECK(ups_charges() == 100);
    }

    SECTION("the UPS covers what the supply lacks") {
        REQUIRE(ranged::fire_gun(get_avatar(), target_pos, 2) == 2);
        CHECK(gun_supply::source_charges(gun, ammo_battery) == 0);
        CHECK(ups_charges() == 70);
    }
}

TEST_CASE("supply_shortfall_is_paid_by_any_ups_source", "[gun][supply]") {
    clear_all_state();
    rng_set_engine_seed(0);
    auto& you = get_avatar();
    auto& gun = wield_gun(itype_laser_rifle, 10);

    SECTION("advanced UPS") {
        // An advanced UPS delivers two UPS charges per battery charge.
        you.i_add(item::spawn(itype_adv_UPS_off, calendar::turn, 50));
        REQUIRE(ups_charges() == 100);

        REQUIRE(ranged::fire_gun(you, target_pos, 1) == 1);
        CHECK(gun_supply::source_charges(gun, ammo_battery) == 0);
        CHECK(ups_charges() == 70);
    }

    SECTION("bionic UPS") {
        you.set_max_power_level(100_kJ);
        give_and_activate_bionic(you, bionic_id("bio_ups"));
        you.set_power_level(100_kJ);
        REQUIRE(ups_charges() == 100);

        REQUIRE(ranged::fire_gun(you, target_pos, 1) == 1);
        CHECK(gun_supply::source_charges(gun, ammo_battery) == 0);
        CHECK(you.get_power_level() == 70_kJ);
    }
}

TEST_CASE("supply_alone_limits_the_burst_of_a_ups_gun", "[gun][supply]") {
    clear_all_state();
    rng_set_engine_seed(0);
    auto& gun = wield_gun(itype_laser_rifle, 100);

    CHECK(gun_supply::shots_available(get_avatar(), gun) == 2);
    CHECK(ranged::fire_gun(get_avatar(), target_pos, 4) == 2);
    CHECK(gun_supply::source_charges(gun, ammo_battery) == 20);
}

TEST_CASE("secondary_ammo_gun_cannot_fire_from_ups", "[gun][supply]") {
    clear_all_state();
    give_ups(1000);

    SECTION("without a supply gunmod") {
        auto& gun = wield_gun(itype_test_supply_only_laser, std::nullopt);
        CHECK(gun_supply::shots_available(get_avatar(), gun) == 0);
        CHECK_FALSE(weapon_checks_pass(gun));
    }

    SECTION("with a supply gunmod below one shot") {
        auto& gun = wield_gun(itype_test_supply_only_laser, 10);
        CHECK(gun_supply::shots_available(get_avatar(), gun) == 0);
        CHECK_FALSE(weapon_checks_pass(gun));
    }

    SECTION("with a charged supply gunmod") {
        auto& gun = wield_gun(itype_test_supply_only_laser, 60);
        CHECK(gun_supply::shots_available(get_avatar(), gun) == 2);
        CHECK(weapon_checks_pass(gun));
        REQUIRE(ranged::fire_gun(get_avatar(), target_pos, 1) == 1);
        CHECK(gun_supply::source_charges(gun, ammo_battery) == 35);
        CHECK(ups_charges() == 1000);
    }
}

TEST_CASE("supply_gunmod_is_unloadable_on_a_no_unload_gun", "[gun][supply]") {
    clear_all_state();
    auto& gun = wield_gun(itype_laser_rifle, 50);

    REQUIRE(gun.has_flag(flag_id("NO_UNLOAD")));
    CHECK(item_funcs::can_be_unloaded(gun));
}

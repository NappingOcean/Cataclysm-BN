#include "gun_supply.h"

#include "character.h"
#include "flag.h"
#include "item.h"
#include "itype.h"

#include <algorithm>
#include <cstdint>
#include <ranges>

namespace {
const auto ammo_battery = ammotype("battery");
const auto itype_UPS = itype_id("UPS");

// Every UPS lookup goes through the three helpers below, so a change to how guns draw on the UPS
// only touches them.

/// UPS charges @p gun drains per shot. Vehicle-mounted guns draw this from the vehicle instead.
auto ups_drain_per_shot(const item& gun) -> int {
    return gun.has_flag(flag_VEHICLE) ? 0 : std::max(gun.get_gun_ups_drain(), 0);
}

/// UPS charges @p who can spend: UPS items, active bionic UPS, or a mech battery.
auto ups_available(const Character& who) -> int { return who.charges_of(itype_UPS); }

auto drain_ups(Character& who, int qty) -> void { who.use_charges(itype_UPS, qty); }
} // namespace

namespace gun_supply {

auto host(const item& gun) -> const item& {
    const auto* const parent = gun.is_gunmod() ? gun.parent_item() : nullptr;
    return parent ? *parent : gun;
}

auto host(item& gun) -> item& { return const_cast<item&>(host(static_cast<const item&>(gun))); }

auto demands(const item& gun) -> std::vector<demand> {
    namespace ranges = std::ranges;

    auto res = std::vector<demand>{};
    const auto entry = [&res](const ammotype& ammo) -> demand& {
        const auto found = ranges::find(res, ammo, &demand::ammo);
        return found != res.end() ? *found : res.emplace_back(demand{.ammo = ammo});
    };

    if (gun.type->gun) {
        for (const auto& secondary : gun.type->gun->secondary_ammo) {
            entry(secondary.ammo).strict += secondary.per_shot;
        }
    }
    if (const auto drain = ups_drain_per_shot(gun); drain > 0) {
        entry(ammo_battery).flexible += drain;
    }
    return res;
}

auto find_source(const item& gun, const ammotype& ammo) -> const item* {
    const auto mods = host(gun).gunmods();
    const auto found = std::ranges::find_if(mods, [&ammo](const item* mod) {
        return mod->is_tool() && mod->ammo_types().contains(ammo);
    });
    return found != mods.end() ? *found : nullptr;
}

auto find_source(item& gun, const ammotype& ammo) -> item* {
    return const_cast<item*>(find_source(static_cast<const item&>(gun), ammo));
}

auto source_charges(const item& gun, const ammotype& ammo) -> int {
    const auto* const source = find_source(gun, ammo);
    return source ? source->ammo_remaining() : 0;
}

auto shots_available(const Character& who, const item& gun) -> int {
    using namespace std::views;

    const auto shots_for = [&](const demand& d) -> int {
        const auto stored = static_cast<std::int64_t>(source_charges(gun, d.ammo));
        const auto ups = d.flexible > 0 ? ups_available(who) : 0;
        const auto by_total = (stored + ups) / d.total();
        const auto by_strict = d.strict > 0 ? stored / d.strict : by_total;
        return static_cast<int>(std::min(by_total, by_strict));
    };

    const auto ds = demands(gun);
    return ds.empty() ? item::INFINITE_CHARGES : std::ranges::min(ds | transform(shots_for));
}

auto consume_shot(Character& who, item& gun) -> bool {
    auto sufficient = true;
    for (const auto& d : demands(gun)) {
        auto* const source = find_source(gun, d.ammo);
        const auto from_source =
            source
                ? source->ammo_consume(std::min(d.total(), source->ammo_remaining()), who.bub_pos())
                : 0;
        const auto shortfall = d.total() - from_source;
        if (shortfall > d.flexible) { sufficient = false; }
        if (const auto from_ups = std::min(shortfall, d.flexible); from_ups > 0) {
            drain_ups(who, from_ups);
        }
    }
    return sufficient;
}

} // namespace gun_supply

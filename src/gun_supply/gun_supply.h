#pragma once

#include "type_id.h"

#include <vector>

class Character;
class item;

/// Ammo a gun spends per shot besides its regular ammunition, held by an installed supply gunmod.
///
/// A supply gunmod is a TOOL with `gunmod_data` whose ammo type matches a demand.
/// UPS drain (`ups_charges` and gunmod modifiers) becomes a battery demand that the shooter's UPS
/// may cover when the supply runs short. `secondary_ammo` demands can only be paid by the supply.
namespace gun_supply {

/// Per-shot demand for one ammo type.
struct demand {
    ammotype ammo;
    /// Charges only the supply gunmod may pay.
    int strict = 0;
    /// Charges the shooter's UPS may pay when the supply gunmod runs short.
    int flexible = 0;

    auto total() const -> int { return strict + flexible; }
};

/// Item whose gunmods hold the supply for @p gun: the parent gun of an auxiliary gunmod.
auto host(const item& gun) -> const item&;
auto host(item& gun) -> item&;

/// Per-shot demands of @p gun, merged by ammo type.
/// Vehicle-mounted guns draw UPS drain from the vehicle, so it is left out for them.
auto demands(const item& gun) -> std::vector<demand>;

/// Installed gunmod that supplies @p ammo to @p gun.
auto find_source(const item& gun, const ammotype& ammo) -> const item*;
auto find_source(item& gun, const ammotype& ammo) -> item*;

/// Charges of @p ammo held by @p gun's supply gunmod.
auto source_charges(const item& gun, const ammotype& ammo) -> int;

/// Shots @p who can fire with @p gun before a supply runs out.
/// Returns `item::INFINITE_CHARGES` when @p gun has no demands.
auto shots_available(const Character& who, const item& gun) -> int;

/// Spends one shot of supply, from the supply gunmod first and then UPS where allowed.
/// Returns false when the supply was short.
auto consume_shot(Character& who, item& gun) -> bool;

} // namespace gun_supply

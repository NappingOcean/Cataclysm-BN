#include "item_functions.h"

#include <algorithm>

#include "character.h"
#include "gun_supply/gun_supply.h"
#include "item.h"
#include "itype.h"
#include "units.h"

static flag_id flag_NO_UNLOAD( "NO_UNLOAD" );

namespace item_funcs
{

bool can_be_unloaded( const item &itm )
{
    if( ( itm.is_container() || itm.is_bandolier() || itm.type->can_use( "holster" ) ) &&
        !itm.contents.empty() &&
        itm.can_unload_liquid() ) {
        return true;
    }

    // Gunmods carry their own NO_UNLOAD, so a supply gunmod stays unloadable on a NO_UNLOAD gun.
    for( const item *mod : itm.gunmods() ) {
        if( can_be_unloaded( *mod ) ) {
            return true;
        }
    }

    if( itm.has_flag( flag_NO_UNLOAD ) ) {
        return false;
    }

    if( itm.magazine_current() ) {
        return true;
    }

    if( itm.ammo_types().empty() ) {
        return false;
    }

    return itm.ammo_remaining() > 0 || itm.casings_count() > 0;
}

// TODO: Add consideration for BIONIC_GUNS when their fake_items get USES_BIONIC_POWER
int shots_remaining( const Character &who, const item &it )
{
    if( !it.is_gun() ) {
        return 0;
    }

    const int ammo_drain = it.ammo_required();
    // No ammo drain means effectively infinite ammo; supply and UPS may still limit it.
    const int by_ammo = ammo_drain > 0 ? it.ammo_remaining() / ammo_drain : item::INFINITE_CHARGES;
    return std::min( by_ammo, gun_supply::shots_available( who, it ) );
}

} // namespace item_funcs

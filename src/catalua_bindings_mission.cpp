#include "catalua_bindings.h"

#include "catalua.h"
#include "catalua_bindings_utils.h"
#include "catalua_impl.h"
#include "catalua_log.h"
#include "catalua_luna.h"
#include "catalua_luna_doc.h"

#include "mission.h"
#include "avatar.h"
#include "character_id.h"
#include "itype.h"
#include "monstergenerator.h"
#include "mtype.h"
#include "type_id.h"

void cata::detail::reg_mission( sol::state &lua )
{
#define UT_CLASS mission_type
    {
        sol::usertype<UT_CLASS> ut =
        luna::new_usertype<UT_CLASS>(
            lua,
            luna::no_bases,
            luna::no_constructor
        );
        SET_MEMB( id );
        SET_MEMB( item_id );
        SET_MEMB( item_count );
        SET_MEMB( monster_type );
        SET_MEMB( monster_species );
        SET_MEMB( monster_kill_goal );
        SET_MEMB( follow_up );
        SET_FX( create );

    }
#undef UT_CLASS // #define UT_CLASS mission_type

#define UT_CLASS mission
    {
        sol::usertype<UT_CLASS> ut =
        luna::new_usertype<UT_CLASS>(
            lua,
            luna::no_bases,
            luna::no_constructor
        );
        SET_FX( name );
        SET_FX( mission_id );
        SET_FX( has_deadline );
        SET_FX( get_description );
        SET_FX( has_target );
        luna::set_fx( ut, "get_target_abs_omt", []( mission & m ) -> tripoint {
            return m.get_target().raw();
        } );
        SET_FX( get_type );
        SET_FX( has_follow_up );
        SET_FX( get_follow_up );
        SET_FX( get_value );
        SET_FX( get_id );
        SET_FX( get_item_id );
        SET_FX( get_npc_id );
        SET_FX( is_assigned );
        luna::set_fx( ut, "set_target_abs_omt", []( mission & m, const tripoint & raw ) -> void {
            tripoint_abs_omt fine( raw );
            m.set_target( fine );
        } );
        SET_FX( set_target_npc_id );
        SET_FX( assign );
        SET_FX( is_complete );
        SET_FX( has_failed );
        SET_FX( find );
        SET_FX( add_existing );
    }
#undef UT_CLASS // #define UT_CLASS mission
}
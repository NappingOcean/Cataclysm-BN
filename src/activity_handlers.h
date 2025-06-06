#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "type_id.h"

class Character;
class inventory;
class item;
class player;
class player_activity;
struct tripoint;
// TODO (https://github.com/cataclysmbnteam/Cataclysm-BN/issues/1612):
// Remove that forward declaration after repair_activity_actor.
class vehicle;

template<typename T>
class detached_ptr;

std::vector<tripoint> get_sorted_tiles_by_distance( const tripoint &abspos,
        const std::unordered_set<tripoint> &tiles );
std::vector<tripoint> route_adjacent( const player &p, const tripoint &dest );

enum requirement_check_result : int {
    SKIP_LOCATION = 0,
    CAN_DO_LOCATION,
    RETURN_EARLY       //another activity like a fetch activity has been started.
};

enum butcher_type : int {
    BUTCHER,        // quick butchery
    BUTCHER_FULL,   // full workshop butchery
    F_DRESS,        // field dressing a corpse
    SKIN,           // skinning a corpse
    QUARTER,        // quarter a corpse
    BLEED,          // bleed a corpse
    DISMEMBER,      // destroy a corpse
    DISSECT         // dissect a corpse for CBMs
};

enum class do_activity_reason : int {
    CAN_DO_CONSTRUCTION,    // Can do construction.
    CAN_DO_FETCH,           // Can do fetch - this is usually the default result for fetch task
    CAN_DO_PREREQ,          // for constructions - can't build the main construction, but can build the pre-req
    CAN_DO_PREREQ_2,        // Can do the second pre-req deep below the desired one.
    NO_COMPONENTS,          // can't do the activity there due to lack of components /tools
    NO_COMPONENTS_PREREQ,   // need components to build the pre-requisite for the actual desired construction
    NO_COMPONENTS_PREREQ_2, // need components to the second pre-req deep.
    DONT_HAVE_SKILL,        // don't have the required skill
    NO_ZONE,                // There is no required zone anymore
    ALREADY_DONE,           // the activity is done already ( maybe by someone else )
    UNKNOWN_ACTIVITY,       // This is probably an error - got to the end of function with no previous reason
    NEEDS_HARVESTING,       // For farming - tile is harvestable now.
    NEEDS_PLANTING,         // For farming - tile can be planted
    NEEDS_TILLING,          // For farming - tile can be tilled
    NEEDS_WARM_WEATHER,     // For farming - need warm weather to plant
    NEEDS_ABOVE_GROUND,     // For farming - can't plant seeds lacking `CAN_PLANT_UNDERGROUND` flag below z-level 0
    BLOCKING_TILE,           // Something has made it's way onto the tile, so the activity cannot proceed
    NEEDS_CHOPPING,         // There is wood there to be chopped
    NEEDS_TREE_CHOPPING,    // There is a tree there that needs to be chopped
    NEEDS_BIG_BUTCHERING,   // There is at least one corpse there to butcher, and it's a big one
    NEEDS_BUTCHERING,       // THere is at least one corpse there to butcher, and there's no need for additional tools
    ALREADY_WORKING,        // somebody is already working there
    NEEDS_VEH_DECONST,       // There is a vehicle part there that we can deconstruct, given the right tools.
    NEEDS_VEH_REPAIR,       // There is a vehicle part there that can be repaired, given the right tools.
    NEEDS_MINING,           // This spot can be mined, if the right tool is present.
    NEEDS_FISHING           // This spot can be fished, if the right tool is present.
};

struct activity_reason_info {
    //reason for success or fail
    do_activity_reason reason;
    //is it possible to do this
    bool can_do;
    //construction index
    std::optional<construction_id> con_idx;

    activity_reason_info( do_activity_reason reason_, bool can_do_,
                          const std::optional<construction_id> &con_idx_ = std::nullopt ) :
        reason( reason_ ),
        can_do( can_do_ ),
        con_idx( con_idx_ )
    { }

    static activity_reason_info ok( const do_activity_reason &reason_ ) {
        return activity_reason_info( reason_, true );
    }

    static activity_reason_info build( const do_activity_reason &reason_, bool can_do_,
                                       const construction_id &con_idx_ ) {
        return activity_reason_info( reason_, can_do_, con_idx_ );
    }

    static activity_reason_info fail( const do_activity_reason &reason_ ) {
        return activity_reason_info( reason_, false );
    }
};

enum class butchery_possibility : int {
    yes = 0,
    need_confirmation,
    never,
    not_this
};

struct butchery_setup {
    std::vector<std::string> problems;
    std::vector<std::string> info;
    butchery_possibility can_do;
    int move_cost;
    butcher_type type;
};

butchery_setup consider_butchery( const item &corpse_item, player &u, butcher_type action );
int butcher_time_to_cut( const item &corpse_item, butcher_type action );

// activity_item_handling.cpp
void activity_on_turn_drop();
void activity_on_turn_move_loot( player_activity &act, player &p );
//return true if there is an activity that can be done potentially, return false if no work can be found.
bool generic_multi_activity_handler( player_activity &act, player &p, bool check_only = false );
void activity_on_turn_fetch( player_activity &, player *p );
void activity_on_turn_wear( player_activity &act, player &p );

enum class consume_type : bool { FOOD, DRINK };

/**
 * @brief Find an item to consume automatically
 *
 * @param consume_type type of item to consume
 * @return true player ate food or was nauseous
 * @return false player did not find anything suitable or is a npc
 */
bool find_auto_consume( player &p, const consume_type type );
void try_fuel_fire( player_activity &act, player &p, bool starting_fire = false );

enum class item_drop_reason {
    deliberate,
    too_large,
    too_heavy,
    tumbling
};

void put_into_vehicle_or_drop( Character &c, item_drop_reason,
                               detached_ptr<item> &&it );
void put_into_vehicle_or_drop( Character &c, item_drop_reason,
                               std::vector<detached_ptr<item>> &items );
void put_into_vehicle_or_drop( Character &c, item_drop_reason,
                               std::vector<detached_ptr<item>> &items,
                               const tripoint &where, bool force_ground = false );
void put_into_vehicle_or_drop( Character &c, item_drop_reason,
                               detached_ptr<item> &&it,
                               const tripoint &where, bool force_ground = false );
void drop_on_map( Character &c, item_drop_reason reason,
                  std::vector<detached_ptr<item>> &items,
                  const tripoint &where );
void drop_on_map( Character &c, item_drop_reason reason,
                  detached_ptr<item> &&it,
                  const tripoint &where );

namespace activity_handlers
{

bool resume_for_multi_activities( player &p );
/** activity_do_turn functions: */
void burrow_do_turn( player_activity *act, player *p );
void craft_do_turn( player_activity *act, player *p );
void fill_liquid_do_turn( player_activity *act, player *p );
void pickaxe_do_turn( player_activity *act, player *p );
void drop_do_turn( player_activity *act, player *p );
void stash_do_turn( player_activity *act, player *p );
void pulp_do_turn( player_activity *act, player *p );
void game_do_turn( player_activity *act, player *p );
void generic_game_do_turn( player_activity *act, player *p );
void churn_do_turn( player_activity *act, player *p );
void start_fire_do_turn( player_activity *act, player *p );
void vibe_do_turn( player_activity *act, player *p );
void hand_crank_do_turn( player_activity *act, player *p );
void multiple_chop_planks_do_turn( player_activity *act, player *p );
void wear_do_turn( player_activity *act, player *p );
void eat_menu_do_turn( player_activity *act, player *p );
void consume_food_menu_do_turn( player_activity *act, player *p );
void consume_drink_menu_do_turn( player_activity *act, player *p );
void consume_meds_menu_do_turn( player_activity *act, player *p );
void move_items_do_turn( player_activity *act, player *p );
void multiple_farm_do_turn( player_activity *act, player *p );
void multiple_fish_do_turn( player_activity *act, player *p );
void multiple_construction_do_turn( player_activity *act, player *p );
void multiple_mine_do_turn( player_activity *act, player *p );
void multiple_butcher_do_turn( player_activity *act, player *p );
void vehicle_deconstruction_do_turn( player_activity *act, player *p );
void vehicle_repair_do_turn( player_activity *act, player *p );
void chop_trees_do_turn( player_activity *act, player *p );
void fetch_do_turn( player_activity *act, player *p );
void move_loot_do_turn( player_activity *act, player *p );
void travel_do_turn( player_activity *act, player *p );
void adv_inventory_do_turn( player_activity *act, player *p );
void armor_layers_do_turn( player_activity *act, player *p );
void atm_do_turn( player_activity *act, player *p );
void fish_do_turn( player_activity *act, player *p );
void cracking_do_turn( player_activity *act, player *p );
void repair_item_do_turn( player_activity *act, player *p );
void butcher_do_turn( player_activity *act, player *p );
void pry_nails_do_turn( player_activity *act, player *p );
void chop_tree_do_turn( player_activity *act, player *p );
void jackhammer_do_turn( player_activity *act, player *p );
void find_mount_do_turn( player_activity *act, player *p );
void tidy_up_do_turn( player_activity *act, player *p );
void fill_pit_do_turn( player_activity *act, player *p );
void fertilize_plot_do_turn( player_activity *act, player *p );
void try_sleep_do_turn( player_activity *act, player *p );
void operation_do_turn( player_activity *act, player *p );
void robot_control_do_turn( player_activity *act, player *p );
void tree_communion_do_turn( player_activity *act, player *p );
void spellcasting_do_turn( player_activity *act, player *p );
void study_spell_do_turn( player_activity *act, player *p );
void read_do_turn( player_activity *act, player *p );
void wait_stamina_do_turn( player_activity *act, player *p );

// defined in activity_handlers.cpp
extern const std::map< activity_id, std::function<void( player_activity *, player * )> >
do_turn_functions;

/** activity_finish functions: */
void burrow_finish( player_activity *act, player *p );
void butcher_finish( player_activity *act, player *p );
void firstaid_finish( player_activity *act, player *p );
void fish_finish( player_activity *act, player *p );
void forage_finish( player_activity *act, player *p );
void hotwire_finish( player_activity *act, player *p );
void longsalvage_finish( player_activity *act, player *p );
void pulp_finish( player_activity *act, player *p );
void make_zlave_finish( player_activity *act, player *p );
void pickaxe_finish( player_activity *act, player *p );
void reload_finish( player_activity *act, player *p );
void start_fire_finish( player_activity *act, player *p );
void train_finish( player_activity *act, player *p );
void milk_finish( player_activity *act, player *p );
void shear_finish( player_activity *act, player *p );
void vehicle_finish( player_activity *act, player *p );
void start_engines_finish( player_activity *act, player *p );
void churn_finish( player_activity *act, player *p );
void plant_seed_finish( player_activity *act, player *p );
void cracking_finish( player_activity *act, player *p );
void repair_item_finish( player_activity *act, player *p );
void mend_item_finish( player_activity *act, player *p );
void gunmod_add_finish( player_activity *act, player *p );
void toolmod_add_finish( player_activity *act, player *p );
void clear_rubble_finish( player_activity *act, player *p );
void meditate_finish( player_activity *act, player *p );
void read_finish( player_activity *act, player *p );
void wait_finish( player_activity *act, player *p );
void wait_weather_finish( player_activity *act, player *p );
void wait_npc_finish( player_activity *act, player *p );
void wait_stamina_finish( player_activity *act, player *p );
void socialize_finish( player_activity *act, player *p );
void try_sleep_finish( player_activity *act, player *p );
void operation_finish( player_activity *act, player *p );
void vibe_finish( player_activity *act, player *p );
void hand_crank_finish( player_activity *act, player *p );
void atm_finish( player_activity *act, player *p );
void eat_menu_finish( player_activity *act, player *p );
void pry_nails_finish( player_activity *act, player *p );
void chop_tree_finish( player_activity *act, player *p );
void chop_logs_finish( player_activity *act, player *p );
void chop_planks_finish( player_activity *act, player *p );
void jackhammer_finish( player_activity *act, player *p );
void fill_pit_finish( player_activity *act, player *p );
void play_with_pet_finish( player_activity *act, player *p );
void shaving_finish( player_activity *act, player *p );
void haircut_finish( player_activity *act, player *p );
void unload_mag_finish( player_activity *act, player *p );
void robot_control_finish( player_activity *act, player *p );
void mind_splicer_finish( player_activity *act, player *p );
void spellcasting_finish( player_activity *act, player *p );
void study_spell_finish( player_activity *act, player *p );

void try_sleep_query( player_activity *act, player *p );

// defined in activity_handlers.cpp
extern const std::map< activity_id, std::function<void( player_activity *, player * )> >
finish_functions;

// HACK: This is a hack to provide fake items
// from vehicles or furniture until
// `repair_activity_actor` would be implemented.
//
// TODO (https://github.com/cataclysmbnteam/Cataclysm-BN/issues/1612):
// Remove that repair code after repair_activity_actor.
namespace repair_activity_hack
{

void patch_activity_for_vehicle_welder(
    player_activity &activity,
    const tripoint &veh_part_position,
    const vehicle &veh,
    int interact_part_idx
);
void patch_activity_for_furniture( player_activity &activity,
                                   const tripoint &furniture_position,
                                   const itype_id &itt );

} // namespace repair_activity_hack

} // namespace activity_handlers


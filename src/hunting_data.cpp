#include "hunting_data.h"

#include <algorithm>
#include <cmath>
#include <map>

#include "coordinate_conversions.h"
#include "debug.h"
#include "game.h"
#include "generic_factory.h"
#include "item.h"
#include "json.h"
#include "map.h"
#include "omdata.h"
#include "overmap.h"
#include "overmapbuffer.h"
#include "rng.h"
#include "skill.h"

static const skill_id skill_traps( "traps" );
static const skill_id skill_survival( "survival" );

namespace
{

generic_factory<snaring_hunting_data> hunting_data_factory( "snaring_hunting_data" );

// Map storing animal population per OMT
std::map<tripoint_abs_omt, int> population_map;

} // namespace

// prey_entry implementation
void prey_entry::deserialize( const JsonObject &jo )
{
    mandatory( jo, false, "prey", prey );
    optional( jo, false, "weight", weight, 1 );
}

// habitat_prey_data implementation
void habitat_prey_data::deserialize( const JsonObject &jo )
{
    mandatory( jo, false, "habitat", habitat );
    optional( jo, false, "bait_flags", bait_flags );
    optional( jo, false, "prey", prey_list );
}

// snaring_hunting_data implementation
void snaring_hunting_data::load( const JsonObject &jo, const std::string & )
{
    optional( jo, was_loaded, "results", results );
}

void snaring_hunting_data::check() const
{
    for( const habitat_prey_data &habitat : results ) {
        if( !habitat.habitat.is_valid() ) {
            debugmsg( "Invalid habitat type in snaring_hunting_data %s", id.c_str() );
        }
        for( const prey_entry &prey : habitat.prey_list ) {
            if( !prey.prey.is_valid() ) {
                debugmsg( "Invalid prey item %s in snaring_hunting_data %s",
                          prey.prey.c_str(), id.c_str() );
            }
        }
    }
}

void snaring_hunting_data::reset()
{
    hunting_data_factory.reset();
}

void snaring_hunting_data::load_hunting_data( const JsonObject &jo, const std::string &src )
{
    hunting_data_factory.load( jo, src );
}

void snaring_hunting_data::check_consistency()
{
    hunting_data_factory.check();
}

const std::vector<snaring_hunting_data> &snaring_hunting_data::get_all()
{
    return hunting_data_factory.get_all();
}

const snaring_hunting_data &snaring_hunting_data::get_default()
{
    static const snaring_hunting_data default_data;
    return default_data;
}

// hunting namespace 구현
namespace hunting
{

std::optional<int> get_population( const tripoint_abs_omt &omt_pos )
{
    auto it = population_map.find( omt_pos );
    if( it != population_map.end() ) {
        return it->second;
    }

    // Default value based on terrain type
    const oter_id &oter = overmap_buffer.ter( omt_pos );
    if( oter->is_wooded() ) {
        return 20; // Forests have higher base population
    } else if( oter->has_flag( oter_flags::source_animals ) ) {
        return 15;
    }
    return 1; // Default minimum
}

void set_population( const tripoint_abs_omt &omt_pos, int value )
{
    population_map[omt_pos] = std::max( 0, value );
}

int calculate_average_population( const tripoint &world_pos )
{
    const tripoint_abs_omt center_omt = project_to<coords::omt>( tripoint_abs_ms( world_pos ) );

    int total = 0;
    int count = 0;

    for( int dx = -1; dx <= 1; ++dx ) {
        for( int dy = -1; dy <= 1; ++dy ) {
            tripoint_abs_omt scan_pos = center_omt + point( dx, dy );
            std::optional<int> pop = get_population( scan_pos );
            if( pop.has_value() ) {
                total += pop.value();
                count++;
            }
        }
    }

    return count > 0 ? total / count : 1;
}

snare_result check_snare( const tripoint &world_pos, const std::vector<flag_id> &bait_flags,
                          time_duration duration, const Character &p, int proximity_penalty )
{
    snare_result result;

    const tripoint_abs_omt center_omt = project_to<coords::omt>( tripoint_abs_ms( world_pos ) );
    const oter_id &oter = overmap_buffer.ter( center_omt );

    // Get current population
    std::optional<int> current_pop = get_population( center_omt );
    int population_value = current_pop.value_or( 1 );

    // Calculate habitat weight based on terrain type
    float habitat_weight = 1.0f;
    if( oter->is_wooded() ) {
        habitat_weight = 1.5f;
    } else if( oter->has_flag( oter_flags::source_animals ) ) {
        habitat_weight = 1.2f;
    }

    // Bait bonus (30% increase if baited)
    float bait_bonus = 1.0f;
    if( !bait_flags.empty() ) {
        bait_bonus = 1.3f;
    }

    // Time bonus (baseline: 6 hours, max 1.5x at 12+ hours)
    float time_bonus = std::min( 1.5f, to_hours<float>( duration ) / 8.0f );

    // Player skill bonus: traps skill + 0.5 * survival skill
    // Each traps level adds 3%, each survival level adds 1.5%
    // At skill 0: base_multiplier = 0.9
    // At skill 6: base_multiplier = 0.9 + 0.18 + 0.09 = 1.17
    float skill_multiplier = 0.9f + ( p.get_skill_level( skill_traps ) * 0.03f ) +
                             ( p.get_skill_level( skill_survival ) * 0.015f );

    // Player presence penalty: reduces success if player camps nearby
    float presence_penalty = 1.0f - get_presence_penalty( center_omt );

    // Proximity penalty: accumulated while player was in same OMT during trap operation
    // Each point reduces success by 1%
    float proximity_multiplier = 1.0f - ( proximity_penalty * 0.01f );

    // Final probability calculation:
    // Base: (population / 20) = 1.0 at full pop
    // Woods, baited, 6h, skill 0, no presence: 1.0 * 1.5 * 1.3 * 0.75 * 0.9 * 1.0 * 1.0 = 1.31 → ~60%
    // Woods, baited, 12h, skill 6, no presence: 1.0 * 1.5 * 1.3 * 1.5 * 1.17 * 1.0 * 1.0 = 3.42 → ~85%
    // With max proximity penalty (50): multiply by 0.5 → success halved
    float base_chance = ( static_cast<float>( population_value ) / 20.0f ) * habitat_weight *
                        bait_bonus * time_bonus * skill_multiplier * presence_penalty *
                        proximity_multiplier;
    float final_chance = std::min( 0.95f, base_chance ); // Cap at 95%

    // Roll the dice
    if( rng_float( 0.0f, 1.0f ) < final_chance ) {
        // Success! Select prey from habitat data
        // TODO: Implement proper prey selection based on habitat and bait
        // For now, default to rabbit
        result.success = true;
        result.prey_id = mtype_id( "mon_rabbit" );
        result.message = _( "You caught something in the snare!" );

        // Reduce local population
        reduce_population( center_omt, 1 );
    } else {
        result.success = false;
        result.message = _( "The snare caught nothing." );
    }

    return result;
}

void reduce_population( const tripoint_abs_omt &omt_pos, int amount )
{
    std::optional<int> current = get_population( omt_pos );
    int new_value = current.value_or( 1 ) - amount;
    set_population( omt_pos, std::max( 0, new_value ) );
}

void recover_population( const tripoint_abs_omt &omt_pos, int amount )
{
    std::optional<int> current = get_population( omt_pos );
    int new_value = current.value_or( 1 ) + amount;
    set_population( omt_pos, std::min( 30, new_value ) );
}

// Player presence tracking: maps OMT to consecutive days player was present
static std::map<tripoint_abs_omt, int> presence_map;

void daily_population_recovery()
{
    // Recover 1 population per day for all tracked OMTs
    for( auto &entry : population_map ) {
        recover_population( entry.first, 1 );
    }

    // Decay presence penalty over time (reduce by 1 day if player not present)
    for( auto it = presence_map.begin(); it != presence_map.end(); ) {
        it->second = std::max( 0, it->second - 1 );
        if( it->second == 0 ) {
            it = presence_map.erase( it );
        } else {
            ++it;
        }
    }
}

void update_player_presence( const tripoint_abs_omt &omt_pos )
{
    // Increment consecutive days player was in this OMT
    presence_map[omt_pos] = std::min( 30, presence_map[omt_pos] + 1 );
}

float get_presence_penalty( const tripoint_abs_omt &omt_pos )
{
    auto it = presence_map.find( omt_pos );
    if( it == presence_map.end() ) {
        return 0.0f;
    }

    // Each day of presence reduces success by 2%, max 60% penalty after 30 days
    return std::min( 0.6f, it->second * 0.02f );
}

} // namespace hunting

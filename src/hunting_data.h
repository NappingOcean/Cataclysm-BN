#pragma once

#include <optional>
#include <string>
#include <vector>

#include "calendar.h"
#include "item.h"
#include "string_id.h"
#include "type_id.h"

class Character;
class JsonObject;
template<typename T> class generic_factory;

// Represents a potential prey creature with spawn weight
struct prey_entry {
    mtype_id prey;  // Monster type ID for corpse generation
    int weight = 1; // Relative spawn probability

    void deserialize( const JsonObject &jo );
};

struct habitat_prey_data {
    oter_type_str_id habitat;
    std::vector<flag_id> bait_flags;
    std::vector<prey_entry> prey_list;

    void deserialize( const JsonObject &jo );
};

// Defines hunting data for snare traps and prey lists by habitat
class snaring_hunting_data
{
    public:
        string_id<snaring_hunting_data> id;
        std::vector<habitat_prey_data> results;

        bool was_loaded = false;

        void load( const JsonObject &jo, const std::string &src );
        void check() const;

        static void reset();
        static void load_hunting_data( const JsonObject &jo, const std::string &src );
        static void check_consistency();
        static const std::vector<snaring_hunting_data> &get_all();
        static const snaring_hunting_data &get_default();

    private:
        friend class generic_factory<snaring_hunting_data>;
};

using snaring_hunting_data_id = string_id<snaring_hunting_data>;

namespace hunting
{

// Get or set hunting population value for an OMT
std::optional<int> get_population( const tripoint_abs_omt &omt_pos );
void set_population( const tripoint_abs_omt &omt_pos, int value );

// Scan surrounding 3x3 OMT area and calculate average population
int calculate_average_population( const tripoint &world_pos );

// Core logic called when checking a snare trap
struct snare_result {
    bool success = false;
    mtype_id prey_id;        // Caught creature type (empty if failed)
    std::string message;
};

snare_result check_snare( const tripoint &world_pos, const std::vector<flag_id> &bait_flags,
                          time_duration duration, const Character &p, int proximity_penalty = 0 );

// Population management: reduce when hunting succeeds, recover over time
void reduce_population( const tripoint_abs_omt &omt_pos, int amount = 1 );
void recover_population( const tripoint_abs_omt &omt_pos, int amount = 1 );

// Daily recovery system - call this once per day
void daily_population_recovery();

// Player presence tracking - reduces success rate if player camps nearby
void update_player_presence( const tripoint_abs_omt &omt_pos );
float get_presence_penalty( const tripoint_abs_omt &omt_pos );

} // namespace hunting

#pragma once

#include <climits>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>

#include "anatomy.h"
#include "bodypart.h"
#include "pimpl.h"
#include "string_formatter.h"
#include "translations.h"
#include "type_id.h"
#include "units.h"
#include "cached_options.h"
#include "enums.h"
#include "memory_fast.h"

enum game_message_type : int;
class nc_color;
class effect;
class effects_map;

namespace catacurses
{
class window;
} // namespace catacurses
class avatar;
class Character;
class npc;
class monster;
class field;
class field_entry;
class JsonObject;
class JsonOut;
struct tripoint;
class time_duration;
class player;
struct point;

enum damage_type : int;
enum m_flag : int;
struct damage_instance;
struct damage_unit;
struct dealt_damage_instance;
struct dealt_projectile_attack;
struct pathfinding_settings;
struct PathfindingSettings;
struct RouteSettings;
struct trap;

template<typename T> struct enum_traits;

using I = std::underlying_type_t<creature_size>;
constexpr I operator+( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) + static_cast<I>( rhs );
}

constexpr I operator+( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) + rhs;
}

constexpr I operator+( const I lhs, const creature_size rhs )
{
    return lhs + static_cast<I>( rhs );
}

constexpr I operator-( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) - static_cast<I>( rhs );
}

constexpr I operator-( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) - rhs;
}

constexpr I operator-( const I lhs, const creature_size rhs )
{
    return lhs - static_cast<I>( rhs );
}

constexpr I operator*( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) * static_cast<I>( rhs );
}

constexpr I operator*( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) * rhs;
}

constexpr I operator*( const I lhs, const creature_size rhs )
{
    return lhs * static_cast<I>( rhs );
}

constexpr I operator/( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) / static_cast<I>( rhs );
}

constexpr I operator/( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) / rhs;
}

constexpr bool operator<=( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) <= static_cast<I>( rhs );
}

constexpr bool operator<=( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) <= rhs;
}

constexpr bool operator<=( const I lhs, const creature_size rhs )
{
    return lhs <= static_cast<I>( rhs );
}

constexpr bool operator<( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) < static_cast<I>( rhs );
}

constexpr bool operator<( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) < rhs;
}

constexpr bool operator<( const I lhs, const creature_size rhs )
{
    return lhs < static_cast<I>( rhs );
}

constexpr bool operator>=( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) >= static_cast<I>( rhs );
}

constexpr bool operator>=( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) >= rhs;
}

constexpr bool operator>=( const I lhs, const creature_size rhs )
{
    return lhs >= static_cast<I>( rhs );
}

constexpr bool operator>( const creature_size lhs, const creature_size rhs )
{
    return static_cast<I>( lhs ) > static_cast<I>( rhs );
}

constexpr bool operator>( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) > rhs;
}

constexpr bool operator>( const I lhs, const creature_size rhs )
{
    return lhs > static_cast<I>( rhs );
}

constexpr bool operator==( const creature_size lhs, const I rhs )
{
    return static_cast<I>( lhs ) == rhs;
}

constexpr bool operator==( const I lhs, const creature_size rhs )
{
    return lhs == static_cast<I>( rhs );
}

enum FacingDirection {
    FD_NONE = 0,
    FD_LEFT = 1,
    FD_RIGHT = 2
};

class Creature
{
    public:
        virtual ~Creature();

        static const std::map<std::string, creature_size> size_map;

        // Like disp_name, but without any "the"
        virtual std::string get_name() const = 0;
        virtual std::string disp_name( bool possessive = false,
                                       bool capitalize_first = false ) const = 0; // displayname for Creature
        virtual std::string skin_name() const = 0; // name of outer layer, e.g. "armor plates"

        virtual std::vector<std::string> get_grammatical_genders() const;

        virtual bool is_player() const {
            return false;
        }
        virtual bool is_avatar() const {
            return false;
        }
        virtual bool is_npc() const {
            return false;
        }
        virtual bool is_monster() const {
            return false;
        }
        virtual monster *as_monster() {
            return nullptr;
        }
        virtual const monster *as_monster() const {
            return nullptr;
        }
        virtual npc *as_npc() {
            return nullptr;
        }
        virtual const npc *as_npc() const {
            return nullptr;
        }
        virtual Character *as_character() {
            return nullptr;
        }
        virtual const Character *as_character() const {
            return nullptr;
        }

        /** DANGER: This is a huge landmine.
        This method will return a non-nullptr for NPCs. Do NOT use `critter.as_player() == nullptr` to verify that a critter is not the player.
        Use `is_player` instead. **/
        virtual player *as_player() {
            return nullptr;
        }
        /** DANGER: This is a huge landmine.
        This method will return a non-nullptr for NPCs. Do NOT use `critter.as_player() == nullptr` to verify that a critter is not the player.
        Use `is_player` instead. **/
        virtual const player *as_player() const {
            return nullptr;
        }
        virtual avatar *as_avatar() {
            return nullptr;
        }
        virtual const avatar *as_avatar() const {
            return nullptr;
        }
        /** return the direction the creature is facing, for sdl horizontal flip **/
        FacingDirection facing = FD_RIGHT;
        /** Returns true for non-real Creatures used temporarily; i.e. fake NPC's used for turret fire. */
        virtual bool is_fake() const;
        /** Sets a Creature's fake boolean. */
        virtual void set_fake( bool fake_value );

        /** Processes effects and bonuses and allocates move points based on speed. */
        virtual void process_turn();
        /** Resets the value of all bonus fields to 0. */
        virtual void reset_bonuses();
        /** Resets stats, and applies effects in an idempotent manner */
        virtual void reset_stats() = 0;
        /** Handles stat and bonus reset. */
        virtual void reset();
        /** Adds an appropriate blood splatter. */
        virtual void bleed() const;
        /** Empty function. Should always be overwritten by the appropriate player/NPC/monster version. */
        virtual void die( Creature *killer ) = 0;

        virtual float dodge_roll() = 0;
        virtual float stability_roll() const = 0;

        /**
         * Simplified attitude string for unlocalized needs.
         */
        static std::string attitude_raw_string( Attitude att );

        /**
         * Creature Attitude as String and color
         */
        static const std::pair<translation, nc_color> &get_attitude_ui_data( Attitude att );

        /**
         * Attitude (of this creature) towards another creature. This might not be symmetric.
         */
        virtual Attitude attitude_to( const Creature &other ) const = 0;

        /**
         * Called when a creature triggers a trap, returns true if they don't set it off.
         * @param tr is the trap that was triggered.
         * @param pos is the location of the trap (not necessarily of the creature) in the main map.
         */
        virtual bool avoid_trap( const tripoint &pos, const trap &tr ) const = 0;

        /**
         * The functions check whether this creature can see the target.
         * The target may either be another creature (critter), or a specific point on the map.
         *
         * The function that take another creature as input should check visibility of that creature
         * (e.g. not digging, or otherwise invisible). They must than check whether the location of
         * the other monster is visible.
         */
        /*@{*/
        virtual bool sees( const Creature &critter ) const;
        virtual bool sees( const tripoint &t, bool is_avatar = false, int range_mod = 0 ) const;
        /*@}*/

        /**
         * How far the creature sees under the given light. Places outside this range can
         * @param light_level See @ref game::light_level.
         */
        virtual int sight_range( int light_level ) const = 0;

        /** Returns an approximation of the creature's strength. */
        virtual float power_rating() const = 0;
        /** Returns an approximate number of tiles this creature can travel per turn. */
        virtual float speed_rating() const = 0;
        /**
         * For fake-players (turrets, mounted turrets) this functions
         * chooses a target. This is for creatures that are friendly towards
         * the player and therefor choose a target that is hostile
         * to the player.
         *
         * @param range The maximal range to look for monsters, anything
         * outside of that range is ignored.
         * @param boo_hoo The number of targets that have been skipped
         * because the player is in the way.
         * @param area The area of effect of the projectile aimed.
         */
        Creature *auto_find_hostile_target( int range, int &boo_hoo, int area = 0 );

        /**
         * Size of the target this creature presents to ranged weapons.
         * 0.0 means unhittable, 1.0 means all projectiles going through this creature's tile will hit it.
         */
        double ranged_target_size() const;

        // handles blocking of damage instance. mutates &dam
        virtual bool block_hit( Creature *source, bodypart_id &bp_hit,
                                damage_instance &dam ) = 0;

        // handles interaction of shields and ranged attacks. mutates &dam
        virtual bool block_ranged_hit( Creature *source, bodypart_id &bp_hit,
                                       damage_instance &dam ) = 0;

        // handles armor absorption (including clothing damage etc)
        // of damage instance. mutates &dam
        virtual void absorb_hit( const bodypart_id &bp, damage_instance &dam ) = 0;

        // TODO: this is just a shim so knockbacks work
        void knock_back_from( const tripoint &p );
        /** Knocks the creature to a specified tile */
        virtual void knock_back_to( const tripoint &to ) = 0;

        int size_melee_penalty() const;
        // begins a melee attack against the creature
        // returns hit - dodge (>=0 = hit, <0 = miss)
        virtual int deal_melee_attack( Creature *source, int hitroll );

        // completes a melee attack against the creature
        // dealt_dam is overwritten with the values of the damage dealt
        virtual void deal_melee_hit( Creature *source, item *source_weapon, int hit_spread,
                                     bool critical_hit,
                                     const damage_instance &dam, dealt_damage_instance &dealt_dam );
        virtual void deal_melee_hit( Creature *source, int hit_spread, bool critical_hit,
                                     const damage_instance &dam, dealt_damage_instance &dealt_dam );

        // Makes a ranged projectile attack against the creature
        // Sets relevant values in `attack`.
        virtual void deal_projectile_attack( Creature *source, item *source_weapon,
                                             dealt_projectile_attack &attack );
        virtual void deal_projectile_attack( Creature *source, dealt_projectile_attack &attack );

        /**
         * Deals the damage via an attack. Allows armor mitigation etc.
         *
         * Most sources of external damage should use deal_damage
         * Mutates the damage_instance& object passed in to reflect the
         * post-mitigation object.
         * Does nothing if this creature is already dead.
         * Does not call @ref check_dead_state.
         * @param source The attacking creature, can be null.
         * @param bp The attacked body part
         * @param dam The damage dealt
         * @param source_weapon The weapon used in the attack, optional
         * @param source_projectile The projectile fired in the attack, optional
         */
        virtual dealt_damage_instance deal_damage( Creature *source, bodypart_id bp,
                const damage_instance &dam, item *source_weapon, item *source_projectile );
        virtual dealt_damage_instance deal_damage( Creature *source, bodypart_id bp,
                const damage_instance &dam, item *source_weapon );
        virtual dealt_damage_instance deal_damage( Creature *source, bodypart_id bp,
                const damage_instance &dam );

        // for each damage type, how much gets through and how much pain do we
        // accrue? mutates damage and pain
        virtual void deal_damage_handle_type( const damage_unit &du,
                                              bodypart_id bp, int &damage, int &pain );
        // directly decrements the damage. ONLY handles damage, doesn't
        // increase pain, apply effects, etc
        virtual void apply_damage( Creature *source, item *source_weapon, item *source_projectile,
                                   bodypart_id bp,
                                   int amount,
                                   bool bypass_med = false ) = 0;
        virtual void apply_damage( Creature *source, item *source_weapon, bodypart_id bp, int amount,
                                   bool bypass_med = false ) = 0;
        virtual void apply_damage( Creature *source, bodypart_id bp, int amount,
                                   bool bypass_med = false ) = 0;

        /**
         * This creature just dodged an attack - possibly special/ranged attack - from source.
         * Players should train dodge, monsters may use some special defenses.
         */
        virtual void on_dodge( Creature *source, int difficulty );
        /**
         * This creature just got hit by an attack - possibly special/ranged attack - from source.
         * @param source Source creature, can be nullptr
         * @param proj Source projectile, can be nullptr
         * Players should train dodge, possibly counter-attack somehow.
         */
        virtual void on_hit( Creature *source, bodypart_id bp_hit,
                             dealt_projectile_attack const *proj = nullptr ) = 0;

        virtual bool digging() const;
        virtual bool is_on_ground() const = 0;
        virtual bool is_underwater() const;
        virtual void set_underwater( bool x );
        virtual bool is_warm() const; // is this creature warm, for IR vision, heat drain, etc
        virtual bool in_species( const species_id & ) const;

        virtual bool has_weapon() const = 0;
        virtual bool is_hallucination() const = 0;
        // returns true if health is zero or otherwise should be dead
        virtual bool is_dead_state() const = 0;

        // Resistances
        virtual bool is_elec_immune() const = 0;
        virtual bool is_immune_effect( const efftype_id &type ) const = 0;
        virtual bool is_immune_damage( damage_type type ) const = 0;

        // Field dangers
        /** Returns true if there is a field in the field set that is dangerous to us. */
        bool is_dangerous_fields( const field &fld ) const;
        /** Returns true if the given field entry is dangerous to us. */
        bool is_dangerous_field( const field_entry &entry ) const;
        /** Returns true if we are immune to the field type with the given fid. Does not
         *  handle intensity, so this function should only be called through is_dangerous_field().
         */
        virtual bool is_immune_field( const field_type_id & ) const {
            return false;
        }

        /** Returns multiplier on fall damage at low velocity (knockback/pit/1 z-level, not 5 z-levels) */
        virtual float fall_damage_mod() const = 0;
        /** Deals falling/collision damage with terrain/creature at pos */
        virtual int impact( int force, const tripoint &pos ) = 0;

        /**
         * This function checks the creatures @ref is_dead_state and (if true) calls @ref die.
         * You can either call this function after hitting this creature, or let the game
         * call it during @ref game::cleanup_dead.
         * As @ref die has many side effects (messages, on-death-triggers, ...), you should be
         * careful when calling this and expect that at least a "The monster dies!" message might
         * have been printed. If you want to print any message relating to the attack (e.g. how
         * much damage has been dealt, how the attack was performed, what has been blocked...), do
         * it *before* calling this function.
         */
        void check_dead_state();

        virtual int posx() const = 0;
        virtual int posy() const = 0;
        virtual int posz() const = 0;
        virtual const tripoint &pos() const = 0;

        virtual void setpos( const tripoint &pos ) = 0;

        bool is_loaded() const;

        /** Processes move stopping effects. Returns false if movement is stopped. */
        virtual bool move_effects( bool attacking ) = 0;

        void add_effect( const effect &eff, bool force = false, bool deferred = false );
        /** Adds or modifies an effect. If intensity is given it will set the effect intensity
            to the given value, or as close as max_intensity values permit. */
        virtual void add_effect( const efftype_id &eff_id, const time_duration &dur,
                                 const bodypart_str_id &bp, int intensity = 0, bool force = false, bool deferred = false );
        void add_effect( const efftype_id &eff_id, const time_duration &dur );
        /** Gives chance to save via environmental resist, returns false if resistance was successful. */
        bool add_env_effect( const efftype_id &eff_id, const bodypart_str_id &vector, int strength,
                             const time_duration &dur,
                             const bodypart_str_id &bp, int intensity = 1,
                             bool force = false );
        bool add_env_effect( const efftype_id &eff_id, const bodypart_str_id &vector, int strength,
                             const time_duration &dur );

        /** Removes a listed effect. No bp means to remove all effects of
         * a given type, targeted or untargeted. Returns true if anything was
         * removed. */
        bool remove_effect( const efftype_id &eff_id );
        virtual bool remove_effect( const efftype_id &eff_id, const bodypart_str_id &bp );
        /** Remove all effects. */
        void clear_effects();
        /** Check if creature has the matching effect. No bp means to check if the Creature has any effect
         *  of the matching type, targeted or untargeted. */
        bool has_effect( const efftype_id &eff_id ) const;
        bool has_effect( const efftype_id &eff_id, const bodypart_str_id &bp ) const;
        /** Check if creature has any effect with the given flag. */
        bool has_effect_with_flag( const flag_id &flag ) const;
        bool has_effect_with_flag( const flag_id &flag, const bodypart_str_id &bp ) const;
        /** Return the effect that matches the given arguments exactly. */
        const effect &get_effect( const efftype_id &eff_id ) const;
        effect &get_effect( const efftype_id &eff_id );
        const effect &get_effect( const efftype_id &eff_id, const bodypart_str_id &bp ) const;
        effect &get_effect( const efftype_id &eff_id, const bodypart_str_id &bp );
        /** Returns pointers to all effects matching given type. */
        std::vector<const effect *> get_all_effects_of_type( const efftype_id &eff_id ) const;
        std::vector<effect *> get_all_effects_of_type( const efftype_id &eff_id );
        /** Returns the duration of the matching effect. Returns 0 if effect doesn't exist. */
        time_duration get_effect_dur( const efftype_id &eff_id ) const;
        time_duration get_effect_dur( const efftype_id &eff_id, const bodypart_str_id &bp ) const;
        /** Returns the intensity of the matching effect. Returns 0 if effect doesn't exist. */
        int get_effect_int( const efftype_id &eff_id ) const;
        int get_effect_int( const efftype_id &eff_id, const bodypart_str_id &bp ) const;
        /** Returns true if the creature resists an effect */
        bool resists_effect( const effect &e ) const;

        // Methods for setting/getting misc key/value pairs.
        void set_value( const std::string &key, const std::string &value );
        void remove_value( const std::string &key );
        std::string get_value( const std::string &key ) const;

        virtual units::mass get_weight() const = 0;

        /**
         * Processes through all the effects on the Creature.
         * This is the only function that is allowed to remove effects.
         */
        void process_effects();
        /** Processes effects according to class (monster, player) */
        virtual void process_effects_internal() = 0;

        /** Returns true if the player has the entered trait, returns false for non-humans */
        virtual bool has_trait( const trait_id &flag ) const;

        // not-quite-stats, maybe group these with stats later
        virtual void mod_pain( int npain );
        virtual void mod_pain_noresist( int npain );
        virtual void set_pain( int npain );
        virtual int get_pain() const;
        virtual int get_perceived_pain() const;
        virtual std::pair<std::string, nc_color> get_pain_description() const;

        int get_moves() const;
        void mod_moves( int nmoves );
        void set_moves( int nmoves );

        virtual bool in_sleep_state() const;

        /*
         * Get/set our killer, this is currently used exclusively to allow
         * mondeath effects to happen after death cleanup
         */
        virtual Creature *get_killer() const;

        /*
         * Getters for stats - combat-related stats will all be held within
         * the Creature and re-calculated during every normalize() call
         */
        virtual int get_num_blocks() const;
        virtual int get_num_dodges() const;
        virtual int get_num_blocks_bonus() const;
        virtual int get_num_dodges_bonus() const;
        virtual int get_num_dodges_base() const;

        virtual int get_env_resist( bodypart_id bp ) const;

        virtual int get_armor_bash( bodypart_id bp ) const;
        virtual int get_armor_cut( bodypart_id bp ) const;
        virtual int get_armor_bullet( bodypart_id bp ) const;
        virtual int get_armor_bash_base( bodypart_id bp ) const;
        virtual int get_armor_cut_base( bodypart_id bp ) const;
        virtual int get_armor_bullet_base( bodypart_id bp ) const;
        virtual int get_armor_bash_bonus() const;
        virtual int get_armor_cut_bonus() const;
        virtual int get_armor_bullet_bonus() const;

        virtual int get_armor_type( damage_type dt, bodypart_id bp ) const = 0;

        virtual float get_dodge() const;
        /** Returns melee skill level, to be used to throttle dodge practice. **/
        virtual float get_melee() const = 0;
        virtual float get_hit() const;

        virtual int get_speed() const;
        virtual creature_size get_size() const = 0;
        virtual int get_hp( const bodypart_id &bp ) const;
        virtual int get_hp() const;
        virtual int get_hp_max( const bodypart_id &bp ) const;
        virtual int get_hp_max() const;
        /** Returns overall % of HP remaining */
        virtual int hp_percentage() const = 0;
        virtual bool made_of( const material_id &m ) const = 0;
        virtual bool made_of_any( const std::set<material_id> &ms ) const = 0;
        // standard creature material sets
        static const std::set<material_id> cmat_flesh;
        static const std::set<material_id> cmat_fleshnveg;
        static const std::set<material_id> cmat_flammable;
        static const std::set<material_id> cmat_flameres;
        virtual field_type_id bloodType() const = 0;
        virtual field_type_id gibType() const = 0;
        // TODO: replumb this to use a std::string along with monster flags.
        virtual bool has_flag( const m_flag ) const {
            return false;
        }
        /** Handles the uncanny dodge bionic and effects, returns true if the creature successfully dodges */
        virtual bool uncanny_dodge() {
            return false;
        }

    private:
        /**anatomy is the plan of the creature's body*/
        anatomy_id creature_anatomy = anatomy_id( "default_anatomy" );
        /**this is the actual body of the creature*/
        std::map<bodypart_str_id, bodypart> body;
    public:
        anatomy_id get_anatomy() const;
        void set_anatomy( anatomy_id anat );

        bodypart_id get_random_body_part( bool main = false ) const;
        /**
         * Returns body parts this creature have.
         * @param only_main If true, only displays parts that can have hit points
         */
        std::vector<bodypart_id> get_all_body_parts( bool only_main = false ) const;

        std::map<bodypart_str_id, bodypart> &get_body();
        const std::map<bodypart_str_id, bodypart> &get_body() const;
        void set_body();
        bodypart &get_part( const bodypart_id &id );
        const bodypart &get_part( const bodypart_id &id ) const;

        int get_part_hp_cur( const bodypart_id &id ) const;
        int get_part_hp_max( const bodypart_id &id ) const;

        int get_part_healed_total( const bodypart_id &id ) const;

        virtual void set_part_hp_cur( const bodypart_id &id, int set );
        virtual void set_part_hp_max( const bodypart_id &id, int set );
        void set_part_healed_total( const bodypart_id &id, int set );
        virtual void mod_part_hp_cur( const bodypart_id &id, int mod );
        virtual void mod_part_hp_max( const bodypart_id &id, int mod );
        void mod_part_healed_total( const bodypart_id &id, int mod );


        virtual void set_all_parts_hp_cur( int set );
        void set_all_parts_hp_to_max();

        virtual int get_speed_base() const;
        virtual int get_speed_bonus() const;
        virtual float get_speed_mult() const;
        virtual int get_block_bonus() const;

        virtual float get_dodge_base() const = 0;
        virtual float get_hit_base() const = 0;
        virtual float get_dodge_bonus() const;
        virtual float get_hit_bonus() const;
        virtual int get_bash_bonus() const;
        virtual int get_cut_bonus() const;

        virtual bool has_grab_break_tec() const = 0;

        /*
         * Setters for stats and bonuses
         */
        virtual void mod_stat( const std::string &stat, float modifier );

        virtual void set_num_blocks_bonus( int nblocks );
        virtual void mod_num_dodges_bonus( int ndodges );

        virtual void set_armor_bash_bonus( int nbasharm );
        virtual void set_armor_cut_bonus( int ncutarm );
        virtual void set_armor_bullet_bonus( int nbulletarm );

        virtual void set_speed_base( int nspeed );
        virtual void set_speed_bonus( int nspeed );
        virtual void set_speed_mult( float nspeed );
        virtual void set_block_bonus( int nblock );

        virtual void mod_speed_bonus( int nspeed );
        virtual void mod_speed_mult( float nspeed );
        virtual void mod_block_bonus( int nblock );
        virtual void mod_bash_bonus( int nbash );
        virtual void mod_cut_bonus( int ncut );
        virtual void mod_size_bonus( int nsize );

        virtual void set_dodge_bonus( float ndodge );
        virtual void set_hit_bonus( float nhit );

        virtual void mod_dodge_bonus( float ndodge );
        virtual void mod_hit_bonus( float  nhit );

        virtual units::mass weight_capacity() const;

        /** Returns settings for legacy pathfinding. */
        virtual const pathfinding_settings &get_legacy_pathfinding_settings() const = 0;
        /** Returns a set of points we do not want to path through with legacy pathfinding. */
        virtual std::set<tripoint> get_legacy_path_avoid() const = 0;

        /** Returns a pathfinding and route settings pair for pathfinding */
        using pf_pair = std::pair<PathfindingSettings, RouteSettings>;
        virtual pf_pair get_pathfinding_pair() const = 0;

        int moves = 0;
        void draw( const catacurses::window &w, point origin, bool inverted ) const;
        void draw( const catacurses::window &w, const tripoint &origin, bool inverted ) const;
        /**
         * Write information about this creature.
         * @param w the window to print the text into.
         * @param vStart vertical start to print, that means the first line to print.
         * @param vLines number of lines to print at most (printing less is fine).
         * @param column horizontal start to print (column), horizontal end is
         * one character before  the right border of the window (to keep the border).
         * @return The line just behind the last printed line, that means multiple calls
         * to this can be stacked, the return value is acceptable as vStart for the next
         * call without creating empty lines or overwriting lines.
         */
        virtual int print_info( const catacurses::window &w, int vStart, int vLines, int column ) const = 0;

        /** Describe this creature as seen by the avatar via infrared vision. */
        void describe_infrared( std::vector<std::string> &buf ) const;

        /** Describe this creature as detected by the avatar's special senses. */
        void describe_specials( std::vector<std::string> &buf ) const;

        // Message related stuff
        virtual void add_msg_if_player( const std::string &/*msg*/ ) const {}
        virtual void add_msg_if_player( const game_message_params &/*params*/,
                                        const std::string &/*msg*/ ) const {}
        void add_msg_if_player( const translation &/*msg*/ ) const;
        void add_msg_if_player( const game_message_params &/*params*/, const translation &/*msg*/ ) const;
        template<typename ...Args>
        void add_msg_if_player( const char *const msg, Args &&... args ) const {
            return add_msg_if_player( string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_player( const std::string &msg, Args &&... args ) const {
            return add_msg_if_player( string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_player( const translation &msg, Args &&... args ) const {
            return add_msg_if_player( string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_player( const game_message_params &params, const char *const msg,
                                Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_if_player( params, string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_player( const game_message_params &params, const std::string &msg,
                                Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_if_player( params, string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_player( const game_message_params &params, const translation &msg,
                                Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_if_player( params, string_format( msg, std::forward<Args>( args )... ) );
        }

        virtual void add_msg_if_npc( const std::string &/*msg*/ ) const {}
        virtual void add_msg_if_npc( const game_message_params &/*params*/,
                                     const std::string &/*msg*/ ) const {}
        void add_msg_if_npc( const translation &/*msg*/ ) const;
        void add_msg_if_npc( const game_message_params &/*params*/, const translation &/*msg*/ ) const;
        template<typename ...Args>
        void add_msg_if_npc( const char *const msg, Args &&... args ) const {
            return add_msg_if_npc( string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_npc( const std::string &msg, Args &&... args ) const {
            return add_msg_if_npc( string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_npc( const translation &msg, Args &&... args ) const {
            return add_msg_if_npc( string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_npc( const game_message_params &params, const char *const msg,
                             Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_if_npc( params, string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_npc( const game_message_params &params, const std::string &msg,
                             Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_if_npc( params, string_format( msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_if_npc( const game_message_params &params, const translation &msg,
                             Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_if_npc( params, string_format( msg, std::forward<Args>( args )... ) );
        }

        virtual void add_msg_player_or_npc( const std::string &/*player_msg*/,
                                            const std::string &/*npc_msg*/ ) const {}
        virtual void add_msg_player_or_npc( const game_message_params &/*params*/,
                                            const std::string &/*player_msg*/,
                                            const std::string &/*npc_msg*/ ) const {}
        void add_msg_player_or_npc( const translation &/*player_msg*/,
                                    const translation &/*npc_msg*/ ) const;
        void add_msg_player_or_npc( const game_message_params &/*params*/,
                                    const translation &/*player_msg*/,
                                    const translation &/*npc_msg*/ ) const;
        template<typename ...Args>
        void add_msg_player_or_npc( const char *const player_msg, const char *const npc_msg,
                                    Args &&... args ) const {
            return add_msg_player_or_npc( string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_npc( const std::string &player_msg, const std::string &npc_msg,
                                    Args &&... args ) const {
            return add_msg_player_or_npc( string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_npc( const translation &player_msg, const translation &npc_msg,
                                    Args &&... args ) const {
            return add_msg_player_or_npc( string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_npc( const game_message_params &params, const char *const player_msg,
                                    const char *const npc_msg, Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_player_or_npc( params, string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_npc( const game_message_params &params, const std::string &player_msg,
                                    const std::string &npc_msg, Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_player_or_npc( params, string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_msg, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_npc( const game_message_params &params, const translation &player_msg,
                                    const translation &npc_msg, Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_player_or_npc( params, string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_msg, std::forward<Args>( args )... ) );
        }

        virtual void add_msg_player_or_say( const std::string &/*player_msg*/,
                                            const std::string &/*npc_speech*/ ) const {}
        virtual void add_msg_player_or_say( const game_message_params &/*params*/,
                                            const std::string &/*player_msg*/,
                                            const std::string &/*npc_speech*/ ) const {}
        void add_msg_player_or_say( const translation &/*player_msg*/,
                                    const translation &/*npc_speech*/ ) const;
        void add_msg_player_or_say( const game_message_params &/*params*/,
                                    const translation &/*player_msg*/,
                                    const translation &/*npc_speech*/ ) const;
        template<typename ...Args>
        void add_msg_player_or_say( const char *const player_msg, const char *const npc_speech,
                                    Args &&... args ) const {
            return add_msg_player_or_say( string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_speech, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_say( const std::string &player_msg, const std::string &npc_speech,
                                    Args &&... args ) const {
            return add_msg_player_or_say( string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_speech, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_say( const translation &player_msg, const translation &npc_speech,
                                    Args &&... args ) const {
            return add_msg_player_or_say( string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_speech, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_say( const game_message_params &params, const char *const player_msg,
                                    const char *const npc_speech, Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_player_or_say( params, string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_speech, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_say( const game_message_params &params, const std::string &player_msg,
                                    const std::string &npc_speech, Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_player_or_say( params, string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_speech, std::forward<Args>( args )... ) );
        }
        template<typename ...Args>
        void add_msg_player_or_say( const game_message_params &params, const translation &player_msg,
                                    const translation &npc_speech, Args &&... args ) const {
            if( params.type == m_debug && !debug_mode ) {
                return;
            }
            return add_msg_player_or_say( params, string_format( player_msg, std::forward<Args>( args )... ),
                                          string_format( npc_speech, std::forward<Args>( args )... ) );
        }

        virtual std::string extended_description() const = 0;

        virtual nc_color symbol_color() const = 0;
        virtual nc_color basic_symbol_color() const = 0;
        virtual const std::string &symbol() const = 0;
        virtual bool is_symbol_highlighted() const;

        // TODO: There may be a cleaner way of doing it than exposing the map
        effects_map get_all_effects() const;

    protected:
        weak_ptr_fast<Creature> killer; // whoever killed us. this should be NULL unless we are dead
        void set_killer( Creature *nkiller );

        /**
         * Processes one effect on the Creature.
         */
        virtual void process_one_effect( effect &e, bool is_new ) = 0;

        pimpl<effects_map> effects;
        // Miscellaneous key/value pairs.
        std::unordered_map<std::string, std::string> values;

        // used for innate bonuses like effects. weapon bonuses will be
        // handled separately

        int num_blocks = 0; // base number of blocks/dodges per turn
        int num_dodges = 0;
        int num_blocks_bonus = 0; // bonus ""
        int num_dodges_bonus = 0;

        int armor_bash_bonus = 0;
        int armor_cut_bonus = 0;
        int armor_bullet_bonus = 0;
        int speed_base = 0; // only speed needs a base, the rest are assumed at 0 and calculated off skills

        int speed_bonus = 0;
        float speed_mult = 0.f;
        float dodge_bonus = 0.0;
        int block_bonus = 0;
        float hit_bonus = 0.0;
        int bash_bonus;
        int cut_bonus;
        int size_bonus;

        bool fake = false;
        Creature();
        Creature( const Creature & );
        Creature( Creature && ) = default;
        Creature &operator=( const Creature & ) = delete;
        Creature &operator=( Creature && ) = default;

    protected:
        virtual void on_stat_change( const std::string &, int ) {}
        virtual void on_effect_int_change( const efftype_id &, int, const bodypart_str_id & ) {}
        virtual void on_damage_of_type( int, damage_type, const bodypart_id & ) {}

    public:
        const bodypart_str_id &select_body_part( Creature *source, int hit_roll ) const;

        static void load_hit_range( const JsonObject & );
        static void reset_hit_range();
        // Empirically determined by "synthetic_range_test" in tests/ranged_balance.cpp.
        static std::vector <int> dispersion_for_even_chance_of_good_hit;
        /**
         * This function replaces the "<npcname>" substring with the @ref disp_name of this creature.
         *
         * Its purpose is to avoid repeated code and improve source readability / maintainability.
         *
         */
        std::string replace_with_npc_name( std::string input ) const;
    protected:
        /**
         * These two functions are responsible for storing and loading the members
         * of this class to/from json data.
         * All classes that inherit from this class should implement their own
         * version of these functions. They are not virtual on purpose, as it's
         * not needed.
         * Every implementation of those functions should a) call the same function
         * with the same parameters of the super class and b) store/load their own
         * members, but *not* members of any sub or super class.
         *
         * The functions are (on purpose) not part of the json
         * serialize/deserialize system (defined in json.h).
         * The Creature class is incomplete, there won't be any instance of that
         * class alone, but there will be instances of sub classes (e.g. player,
         * monster).
         * Those (complete) classes implement the deserialize/serialize interface
         * of json. That way only one json object per npc/player/monster instance
         * is created (inside of the serialize function).
         * E.g. player::serialize() looks like this:
         * <code>json.start_object();store(json);json.end_object()</code>
         * player::store than stores the members of the player class, and it calls
         * Character::store, which stores the members of the Character class and calls
         * Creature::store, which stores the members of the Creature class.
         * All data goes into the single json object created in player::serialize.
         */
        // Store data of *this* class in the stream
        void store( JsonOut &jsout ) const;
        // Load creature data from the given json object.
        void load( const JsonObject &jsin );

    private:
        int pain = 0;
        bool underwater = false;
};



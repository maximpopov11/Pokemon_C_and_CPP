#include <cstdlib>
#include <ctime>
#include <climits>
#include <cmath>
#include <getopt.h>
#include <ncurses.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include "heap.h"

#define SCREEN_HEIGHT 24
#define TILE_WIDTH_X 80
#define TILE_LENGTH_Y 21
#define WORLD_WIDTH_X 399
#define WORLD_LENGTH_Y 399
#define WORLD_CENTER_X 199
#define WORLD_CENTER_Y 199
#define TERRAIN_BORDER_WEIGHT 1
#define MINIMUM_TURN 5
//77 = minimum number of paths in Tile - 1 for PC so all trainers can be placed
#define MAX_NUM_TRAINERS 77
#define INVERSE_POKEMON_ENCOUNTER_CHANCE 10

//Author Maxim Popov
enum character_type {
    PLAYER,
    RIVAL,
    HIKER,
    RANDOM_WALKER,
    PACER,
    WANDERER,
    STATIONARY
};

enum combat_option {
    FIGHT,
    SWITCH,
    BAG,
    RUN
};

class Terrain {
public:
    int id;
    char printable_character;
    int path_weight;
    int pc_weight;
    int rival_weight;
    int hiker_weight;
    short color;

    Terrain() : Terrain(0, '_', 0, 0, 0, 0, COLOR_BLACK) {}

    Terrain(int id, char printable_character, int path_weight, int pc_weight, int rival_weight, int hiker_weight, short color) :
        id(id), printable_character(printable_character), path_weight(path_weight), pc_weight(pc_weight),
        rival_weight(rival_weight), hiker_weight(hiker_weight), color(color) {}
};

static Terrain *none = new Terrain(0, '_', 0, 0, 0, 0, COLOR_BLACK);
static Terrain *edge = new Terrain(1, '%', INT_MAX, INT_MAX, INT_MAX, INT_MAX, COLOR_WHITE);
static Terrain *clearing = new Terrain(2, '.', 5, 10, 10, 5, COLOR_YELLOW);
static Terrain *grass = new Terrain(3, ',', 10, 15, 15, 5, COLOR_GREEN);
static Terrain *forest = new Terrain(4, '^', 100, INT_MAX, INT_MAX, 10, COLOR_GREEN);
static Terrain *mountain = new Terrain(5, '%', 150, INT_MAX, INT_MAX, 10, COLOR_WHITE);
static Terrain *lake = new Terrain(6, '~', 200, INT_MAX, INT_MAX, INT_MAX, COLOR_BLUE);
static Terrain *path = new Terrain(7, '#', 0, 5, 5, 5, COLOR_YELLOW);
static Terrain *center = new Terrain(8, 'C', INT_MAX, 5, INT_MAX, INT_MAX, COLOR_MAGENTA);
static Terrain *mart = new Terrain(9, 'M', INT_MAX, 5, INT_MAX, INT_MAX, COLOR_MAGENTA);

//commented due to failing make
//class DatabaseInfo {
//public:
//    virtual std::string toString();
//};

class PokemonInfo {
public:
    int id;
    std::string name;
    int species_id;
    int height;
    int weight;
    int base_experience;
    int order;
    int is_default;
    std::string pokemonString;

    PokemonInfo(std::string id, std::string name, std::string species_id, std::string height, std::string weight,
                std::string base_experience, std::string order, std::string is_default) :
        id(stoi(id)), name(name), species_id(stoi(species_id)), height(stoi(height)), weight(stoi(weight)),
        base_experience(stoi(base_experience)), order(stoi(order)), is_default(stoi(is_default)) {
        pokemonString = "";
        if (this->id != -1) {
            pokemonString += id;
        }
        pokemonString += ",";
        if (this->name != "-1") {
            pokemonString += name;
        }
        pokemonString += ",";
        if (this->species_id != -1) {
            pokemonString += species_id;
        }
        pokemonString += ",";
        if (this->height != -1) {
            pokemonString += height;
        }
        pokemonString += ",";
        if (this->weight != -1) {
            pokemonString += weight;
        }
        pokemonString += ",";
        if (this->base_experience != -1) {
            pokemonString += base_experience;
        }
        pokemonString += ",";
        if (this->order != -1) {
            pokemonString += order;
        }
        pokemonString += ",";
        if (this->is_default != -1) {
            pokemonString += is_default;
        }
        pokemonString = id + "," + name + "," + species_id + "," + height + "," + weight + "," + base_experience + "," +
                order + "," + is_default;
    }

    std::string toString() {
        return pokemonString;
    }
};

class Move {
public:
    int id;
    std::string name;
    int generation_id;
    int type_id;
    int power;
    int pp;
    int accuracy;
    int priority;
    int target_id;
    int damage_class_id;
    int effect_id;
    int effect_chance;
    int contest_type_id;
    int contest_effect_id;
    int super_contest_effect_id;
    std::string moveString;

    Move(std::string id, std::string name, std::string generation_id, std::string type_id, std::string power,
    std::string pp, std::string accuracy, std::string priority, std::string target_id, std::string damage_class_id,
         std::string effect_id, std::string effect_chance, std::string contest_type_id, std::string contest_effect_id,
         std::string super_contest_effect_id) {
        this->id = stoi(id);
        this->name = name;
        this->generation_id = stoi(generation_id);
        this->type_id = stoi(type_id);
        this->power = stoi(power);
        this->pp = stoi(pp);
        this->accuracy = stoi(accuracy);
        this->priority = stoi(priority);
        this->target_id = stoi(target_id);
        this->damage_class_id = stoi(damage_class_id);
        this->effect_id = stoi(effect_id);
        this->effect_chance = stoi(effect_chance);
        this->contest_type_id = stoi(contest_type_id);
        this->contest_effect_id = stoi(contest_effect_id);
        this->super_contest_effect_id = stoi(super_contest_effect_id);
        moveString = "";
        if (this->id != -1) {
            moveString += id;
        }
        moveString += ",";
        if (this->name != "-1") {
            moveString += name;
        }
        moveString += ",";
        if (this->generation_id != -1) {
            moveString += generation_id;
        }
        moveString += ",";
        if (this->type_id != -1) {
            moveString += type_id;
        }
        moveString += ",";
        if (this->power != -1) {
            moveString += power;
        }
        moveString += ",";
        if (this->pp != -1) {
            moveString += pp;
        }
        moveString += ",";
        if (this->accuracy != -1) {
            moveString += accuracy;
        }
        moveString += ",";
        if (this->priority != -1) {
            moveString += priority;
        }
        moveString += ",";
        if (this->target_id != -1) {
            moveString += target_id;
        }
        moveString += ",";
        if (this->damage_class_id != -1) {
            moveString += damage_class_id;
        }
        moveString += ",";
        if (this->effect_id != -1) {
            moveString += effect_id;
        }
        moveString += ",";
        if (this->effect_chance != -1) {
            moveString += effect_chance;
        }
        moveString += ",";
        if (this->contest_type_id != -1) {
            moveString += contest_type_id;
        }
        moveString += ",";
        if (this->contest_effect_id != -1) {
            moveString += contest_effect_id;
        }
        moveString += ",";
        if (this->super_contest_effect_id != -1) {
            moveString += super_contest_effect_id;
        }
    }

    std::string toString() {
        return moveString;
    }
};

class PokemonMove {
public:
    int pokemon_id;
    int version_group_id;
    int move_id;
    int pokemon_move_method_id;
    int level;
    int order;
    std::string pokemonMoveString;

    PokemonMove(std::string pokemon_id, std::string version_group_id, std::string move_id, std::string pokemon_move_method_id,
            std::string level, std::string order) {
        this->pokemon_id = stoi(pokemon_id);
        this->version_group_id = stoi(version_group_id);
        this->move_id = stoi(move_id);
        this->pokemon_move_method_id = stoi(pokemon_move_method_id);
        this->level = stoi(level);
        this->order = stoi(order);
        pokemonMoveString = "";
        if (this->pokemon_id != -1) {
            pokemonMoveString += pokemon_id;
        }
        pokemonMoveString += ",";
        if (this->version_group_id != -1) {
            pokemonMoveString += version_group_id;
        }
        pokemonMoveString += ",";
        if (this->move_id != -1) {
            pokemonMoveString += move_id;
        }
        pokemonMoveString += ",";
        if (this->pokemon_move_method_id != -1) {
            pokemonMoveString += pokemon_move_method_id;
        }
        pokemonMoveString += ",";
        if (this->level != -1) {
            pokemonMoveString += level;
        }
        pokemonMoveString += ",";
        if (this->order != -1) {
            pokemonMoveString += order;
        }
    }

    std::string toString() {
        return pokemonMoveString;
    }
};

class PokemonSpecies {

public:
    int id;
    std::string name;
    int generation_id;
    int evolves_from_species_id;
    int evolution_chain_id;
    int color_id;
    int shape_id;
    int habitat_id;
    int gender_rate;
    int capture_rate;
    int base_happiness;
    int is_baby;
    int hatch_counter;
    int has_gender_differences;
    int growth_rate_id;
    int forms_switchable;
    int is_legendary;
    int is_mythical;
    int order;
    int conquest_order;
    std::string pokemonSpeciesString;

    PokemonSpecies(std::string id, std::string name, std::string generation_id, std::string evolves_from_species_id,
         std::string evolution_chain_id, std::string color_id, std::string shape_id, std::string habitat_id,
         std::string gender_rate, std::string capture_rate, std::string base_happiness, std::string is_baby,
         std::string hatch_counter, std::string has_gender_differences, std::string growth_rate_id,
         std::string forms_switchable, std::string is_legendary, std::string is_mythical, std::string order,
         std::string conquest_order) {
        this->id = stoi(id);
        this->name = name;
        this->generation_id = stoi(generation_id);
        this->evolves_from_species_id = stoi(evolves_from_species_id);
        this->evolution_chain_id = stoi(evolution_chain_id);
        this->color_id = stoi(color_id);
        this->shape_id = stoi(shape_id);
        this->habitat_id = stoi(habitat_id);
        this->gender_rate = stoi(gender_rate);
        this->capture_rate = stoi(capture_rate);
        this->base_happiness = stoi(base_happiness);
        this->is_baby = stoi(is_baby);
        this->hatch_counter = stoi(hatch_counter);
        this->has_gender_differences = stoi(has_gender_differences);
        this->growth_rate_id = stoi(growth_rate_id);
        this->forms_switchable = stoi(forms_switchable);
        this->is_legendary = stoi(is_legendary);
        this->is_mythical = stoi(is_mythical);
        this->order = stoi(order);
        this->conquest_order = stoi(conquest_order);
        pokemonSpeciesString = "";
        if (this->id != -1) {
            pokemonSpeciesString += id;
        }
        pokemonSpeciesString += ",";
        if (this->name != "-1") {
            pokemonSpeciesString += name;
        }
        pokemonSpeciesString += ",";
        if (this->generation_id != -1) {
            pokemonSpeciesString += generation_id;
        }
        pokemonSpeciesString += ",";
        if (this->evolves_from_species_id != -1) {
            pokemonSpeciesString += evolves_from_species_id;
        }
        pokemonSpeciesString += ",";
        if (this->evolution_chain_id != -1) {
            pokemonSpeciesString += evolution_chain_id;
        }
        pokemonSpeciesString += ",";
        if (this->color_id != -1) {
            pokemonSpeciesString += color_id;
        }
        pokemonSpeciesString += ",";
        if (this->shape_id != -1) {
            pokemonSpeciesString += shape_id;
        }
        pokemonSpeciesString += ",";
        if (this->habitat_id != -1) {
            pokemonSpeciesString += habitat_id;
        }
        pokemonSpeciesString += ",";
        if (this->gender_rate != -1) {
            pokemonSpeciesString += gender_rate;
        }
        pokemonSpeciesString += ",";
        if (this->capture_rate != -1) {
            pokemonSpeciesString += capture_rate;
        }
        pokemonSpeciesString += ",";
        if (this->base_happiness != -1) {
            pokemonSpeciesString += base_happiness;
        }
        pokemonSpeciesString += ",";
        if (this->is_baby != -1) {
            pokemonSpeciesString += is_baby;
        }
        pokemonSpeciesString += ",";
        if (this->hatch_counter != -1) {
            pokemonSpeciesString += hatch_counter;
        }
        pokemonSpeciesString += ",";
        if (this->has_gender_differences != -1) {
            pokemonSpeciesString += has_gender_differences;
        }
        pokemonSpeciesString += ",";
        if (this->growth_rate_id != -1) {
            pokemonSpeciesString += growth_rate_id;
        }
        pokemonSpeciesString += ",";
        if (this->forms_switchable != -1) {
            pokemonSpeciesString += forms_switchable;
        }
        pokemonSpeciesString += ",";
        if (this->is_legendary != -1) {
            pokemonSpeciesString += is_legendary;
        }
        pokemonSpeciesString += ",";
        if (this->is_mythical != -1) {
            pokemonSpeciesString += is_mythical;
        }
        pokemonSpeciesString += ",";
        if (this->order != -1) {
            pokemonSpeciesString += order;
        }
        pokemonSpeciesString += ",";
        if (this->conquest_order != -1) {
            pokemonSpeciesString += conquest_order;
        }
    }

    std::string toString() {
        return pokemonSpeciesString;
    }
};

class Experience {
public:
    int growth_rate_id;
    int level;
    int experience;
    std::string experienceString;

    Experience(std::string growth_rate_id, std::string level, std::string experience) {
        this->growth_rate_id = stoi(growth_rate_id);
        this->level = stoi(level);
        this->experience = stoi(experience);
        experienceString = "";
        if (this->growth_rate_id != -1) {
            experienceString += growth_rate_id;
        }
        experienceString += ",";
        if (this->level != -1) {
            experienceString += level;
        }
        experienceString += ",";
        if (this->experience != -1) {
            experienceString += experience;
        }
    }

    std::string toString() {
        return experienceString;
    }
};

class TypeName {
public:
    int type_id;
    int local_language_id;
    std::string name;
    std::string typeNameString;

    TypeName(std::string type_id, std::string local_language_id, std::string name) {
        this->type_id = stoi(type_id);
        this->local_language_id = stoi(local_language_id);
        this->name = name;
        typeNameString = "";
        if (this->type_id != -1) {
            typeNameString += type_id;
        }
        typeNameString += ",";
        if (this->local_language_id != -1) {
            typeNameString += local_language_id;
        }
        typeNameString += ",";
        if (this->name != "-1") {
            typeNameString += name;
        }
    }

    std::string toString() {
        return typeNameString;
    }
};

class PokemonStat {
public:
    int pokemon_id;
    int stat_id;
    int base_stat;
    int effort;
    std::string pokemonStatString;

    PokemonStat(std::string pokemon_id, std::string stat_id, std::string base_stat, std::string effort) {
        this->pokemon_id = stoi(pokemon_id);
        this->stat_id = stoi(stat_id);
        this->base_stat = stoi(base_stat);
        this->effort = stoi(effort);
        pokemonStatString = "";
        if (this->pokemon_id != -1) {
            pokemonStatString += pokemon_id;
        }
        pokemonStatString += ",";
        if (this->stat_id != -1) {
            pokemonStatString += stat_id;
        }
        pokemonStatString += ",";
        if (this->base_stat != -1) {
            pokemonStatString += base_stat;
        }
        pokemonStatString += ",";
        if (this->effort != -1) {
            pokemonStatString += effort;
        }
    }

    std::string toString() {
        return pokemonStatString;
    }
};

class PokemonType {
public:
    int pokemon_id;
    int type_id;
    int slot;
    std::string pokemonTypeString;

    PokemonType(std::string pokemon_id, std::string type_id, std::string slot) {
        this->pokemon_id = stoi(pokemon_id);
        this->type_id = stoi(type_id);
        this->slot = stoi(slot);
        pokemonTypeString = "";
        if (this->pokemon_id != -1) {
            pokemonTypeString += pokemon_id;
        }
        pokemonTypeString += ",";
        if (this->type_id != -1) {
            pokemonTypeString += type_id;
        }
        pokemonTypeString += ",";
        if (this->slot != -1) {
            pokemonTypeString += slot;
        }
    }

    std::string toString() {
        return pokemonTypeString;
    }
};

class Pokemon {
public:
    PokemonInfo *pokemonInfo;
    int base_health;
    int base_attack;
    int base_defense;
    int base_special_attack;
    int base_special_defense;
    int base_speed;
    int health_iv = rand() % 16;
    int attack_iv = rand() % 16;
    int defense_iv = rand() % 16;
    int special_attack_iv = rand() % 16;
    int special_defense_iv = rand() % 16;
    int speed_iv = rand() % 16;
    int level;
    int maxHealth;
    int health;
    //must have between 1 and 4 moves (2 on creation if possible, 1 is always possible if not 2)
    std::vector<Move *> moves;
    std::vector<int> typeIDs;
    bool male;
    bool shiny;
    bool knockedOut = false;

    Pokemon(PokemonInfo *pokemonInfo, int base_health, int base_attack, int base_defense, int base_speed,
            int base_special_attack, int base_special_defense, int level, std::vector<Move *> moves, bool male,
            bool shiny) :
            pokemonInfo(pokemonInfo), base_health(base_health), base_attack(base_attack), base_defense(base_defense),
            base_speed(base_speed), base_special_attack(base_special_attack), base_special_defense(base_special_defense),
            level(level), moves(moves), male(male), shiny(shiny) {
        this->maxHealth = ((base_health + health_iv) * 2 * level) / 100 + level + 10;
        this->health = maxHealth;
    }

    int getHealth() {
        return health;
    }

    int getAttack() {
        return ((base_attack + attack_iv) * 2 * level) / 100 + 5;
    }

    int getDefense() {
        return ((base_defense + defense_iv) * 2 * level) / 100 + 5;
    }

    int getSpecialAttack() {
        return ((base_special_attack + special_attack_iv) * 2 * level) / 100 + 5;
    }

    int getSpecialDefense() {
        return ((base_special_defense + special_defense_iv) * 2 * level) / 100 + 5;
    }

    int getSpeed() {
        return ((base_speed + speed_iv) * 2 * level) / 100 + 5;
    }

    int takeDamage(int amount) {
        this->health -= amount;
        if (health <= 0) {
            health = 0;
            knockedOut = true;
            return 1;
        }
        else {
            return 0;
        }
    }

    int heal(int amount) {
        this->health += amount;
        if (health > maxHealth) {
            health = maxHealth;
        }
    }

    bool revive() {
        if (!this->knockedOut) {
            return false;
        }
        else {
            this->knockedOut = false;
            this->health = this->maxHealth / 2;
        }
    }

};

class Bag{
public:
    int numPotions;
    int numRevives;
    int numPokeballs;

    Bag() {
        this->numPotions = 3 + rand() % 3;
        this->numRevives = 1 + rand() % 2;
        this->numPokeballs = 1 + rand() % 2;
    }

    int usePotion(Pokemon *pokemon) {
        if (numPotions > 0) {
            pokemon->heal(20);
            numPotions--;
            return 0;
        }
        else {
            return 1;
        }
    }

    int useRevive(Pokemon *pokemon) {
        if (numPotions > 0) {
            if (pokemon->revive()) {
                return 0;
                numRevives--;
            }
            else {
                return 2;
            }
        }
        else {
            return 1;
        }
    }

    int usePokeball(std::vector<Pokemon *> pokemon) {
        if (numPokeballs > 0) {
            if (pokemon.size() < 6) {
                return 0;
            } else {
                return 2;
            }
        }
        else {
            return 1;
        }
    }

};

class Character {
public:
    int x;
    int y;
    enum character_type type_enum;
    std::string type_string;
    char printable_character;
    short color;
    int turn;
    int direction_set;
    int x_direction;
    int y_direction;
    int in_building;
    int defeated;
    //always between 1 and 6 active pokemon (starts with 1 on PC creation, chance for more for trainers)
    std::vector<Pokemon *> activePokemon;
    Bag *bag;

    Character(int x, int y, enum  character_type type_enum, std::string type_string, char printable_character, short color,
              int turn, int direction_set, int x_direction, int y_direction, int in_building, int defeated) : x(x), y(y),
              type_enum(type_enum), type_string(type_string), printable_character(printable_character), color(color),
              turn(turn), direction_set(direction_set), x_direction(x_direction), y_direction(y_direction),
              in_building(in_building), defeated(defeated) {
        bag = new Bag();
    }
};

class PlayerCharacter : public Character {

};

class NonPlayerCharacter : public Character {

};

class Point {
public:
    int x;
    int y;
    Terrain terrain;
    //for looped non-queue plant_seeds growth
    Terrain grow_into;
    Character *character;
    int distance;
    heap_node_t *heap_node;

    Point() {
        this->x = -1;
        this->y = -1;
        this->terrain = *none;
        this->grow_into = *none;
        this->character = NULL;
        this->distance = INT_MAX;
        this->heap_node = NULL;
    }
};

static int32_t comparator_trainer_distance_tile(const void *key, const void *with) {
    return ((Point *) key)->distance - ((Point *) with)->distance;
}

static int32_t comparator_character_movement(const void *key, const void *with) {
    return ((Character *) key)->turn - ((Character *) with)->turn;
}

class Tile {
public:
    Point tile[TILE_LENGTH_Y][TILE_WIDTH_X];
    int x;
    int y;
    int north_x;
    int south_x;
    int east_y;
    int west_y;
    PlayerCharacter *player_character;

    Tile() {
        this->x = -1;
        this->y = -1;
        this->north_x = -1;
        this->south_x = -1;
        this->east_y = -1;
        this->west_y = -1;
        this->player_character = NULL;
    }
};

int rival_distance_tile[TILE_LENGTH_Y][TILE_WIDTH_X];
int hiker_distance_tile [TILE_LENGTH_Y][TILE_WIDTH_X];

int print_usage();
int storePokemon();
int storeMoves();
int storePokemonMoves();
int storePokemonSpecies();
int storeExperience();
int storeTypeNames();
int storePokemonStats();
int storePokemonTypes();
//commented due to database info failing to make
//int printData(std::vector<DatabaseInfo *> dataVector);
int turn_based_movement();
int player_turn();
int move_character(int x, int y, int new_x, int new_y);
int combat_trainer(Character *from_character, Character *to_character);
Pokemon * create_pokemon();
int combat_pokemon(Pokemon *wildPokemon);
int getWildPokemonMove(Pokemon *wildPokemon);
int doCombat(Pokemon *friendlyPokemon, int friendlyPokemonMoveIndex, Pokemon *wildPokemon, int wildPokemonMoveIndex);
int attack(Pokemon *attackingPokemon, int moveIndex, Pokemon *defendingPokemon);
int battlePause();
int fight_action(Pokemon *selectedPokemon);
Pokemon *switch_pokemon_action(Pokemon *selectedPokemon);
int bag_action(bool wildPokemonBattle, Pokemon *selectedPokemon, Pokemon *enemyPokemon);
int usePokeball(bool success, Pokemon *targetPokemon);
int run_action(Pokemon *characterPokemon, Pokemon *wildPokemon, int numAttempts);
int enter_center();
int enter_mart();
int change_tile(int x, int y);
Tile create_tile(int x, int y);
Tile create_empty_tile();
int generate_terrain(Tile *tile);
int plant_seeds(Tile *tile, Terrain terrain, int num_seeds);
int grow_seeds(Tile *tile);
int place_edge(Tile *tile);
int set_terrain_border_weights(Tile *tile);
int generate_paths(Tile *tile, int north_x, int south_x, int east_y, int west_y);
int generate_buildings(Tile *tile, int x, int y);
int place_building(Tile *tile, Terrain terrain, double chance);
int place_player_character(Tile *tile);
int select_pokemon(Character *playerCharacter);
int place_trainers(Tile *tile);
int place_trainer_type(Tile *tile, int num_trainer, enum character_type trainer_type, char character);
int dijkstra(Tile *tile, enum character_type trainer_type);
int legal_overwrite(Point point);
double distance(int x1, int y1, int x2, int y2);
int print_tile_terrain(Tile *tile);
int print_tile_trainer_distances(Tile *tile);
int print_tile_trainer_distances_printer(Tile *tile);

class UserInterface {
public:
    virtual void initializeTerminalUI() = 0;
    virtual void endwinUI() = 0;
    virtual char getchUI() = 0;
    virtual void clearUI() = 0;
    virtual void refreshUI() = 0;
    virtual void addchUI(char ch) = 0;
    virtual void addstrUI(const char * string) = 0;
    virtual void mvaddchUI(int y, int x, char ch) = 0;
    virtual void mvaddstrUI(int y, int x, const char * string) = 0;
    virtual void init_pairUI(int i, short color1, short color2) = 0;
    virtual void attrsetUI(int i) = 0;
    virtual void attroffUI(int i) = 0;
};

class Ncurses : public UserInterface {
public:
    void initializeTerminalUI() {
        initscr();
        raw();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
    }
    void endwinUI() {
        endwin();
    }
    char getchUI() {
        return getch();
    }
    void clearUI() {
        clear();
    }
    void refreshUI() {
        refresh();
    }
    void addchUI(char ch) {
        addch(ch);
    }
    void addstrUI(const char * string) {
        addstr(string);
    }
    virtual void mvaddchUI(int y, int x, char ch) {
        mvaddch(y, x, ch);
    }
    void mvaddstrUI(int y, int x, const char * string) {
        mvaddstr(y, x, string);
    }
    void init_pairUI(int i, short color1, short color2) {
        init_pair(i, color1, color2);
    }
    void attrsetUI(int i) {
        attrset(i);
    }
    void attroffUI(int i) {
        attroff(i);
    }
};

//mocks getch, does nothing for the others
class NoNcurses : public UserInterface {
public:
    void initializeTerminalUI() {}
    void endwinUI() {}
    char getchUI() {
        std::cout << "\n";
        char c;
        std::cin >> c;
        return c;
    }
    void clearUI() {
        std::cout << "\n";
    }
    void refreshUI() {}
    void addchUI(char ch) {
        std::cout << ch;
    }
    void addstrUI(const char * string) {
        std::cout << string;
    }
    void mvaddchUI(int y, int x, char ch) {
        std::cout << "\n" << ch;
    }
    void mvaddstrUI(int y, int x, const char * string) {
        std::cout << "\n" << string;
    }
    void init_pairUI(int i, short color1, short color2) {}
    void attrsetUI(int i) {}
    void attroffUI(int i) {}
};

//todo: ASSIGNED: set filePath to main file location ("." doesn't work)
//todo: ASSIGNED: set file path to "" pre submission
std::string filePath = "/Users/maximpopov/CLionProjects/Pokemon_C_and_CPP/";
UserInterface *interface;
std::vector<PokemonInfo *> allPokemonInfo;
std::vector<Move *> allMoves;
std::vector<PokemonMove *> allPokemonMoves;
std::vector<PokemonSpecies *> allPokemonSpecies;
std::vector<Experience *> allExperience;
std::vector<TypeName *> allTypeNames;
std::vector<PokemonStat *> allPokemonStats;
std::vector<PokemonType *> allPokemonTypes;
Tile *world[WORLD_LENGTH_Y][WORLD_WIDTH_X] = {0};
int current_tile_x;
int current_tile_y;
Character *player_character;
int num_trainers;
struct heap turn_heap;

int main(int argc, char *argv[]) {

    //todo: ASSIGNED: change to Ncurses on submission
    interface = new Ncurses();

    //get arguments
//    int opt = 0;
    int numtrainers = 10;
//    static struct option long_options[] = {
//            {"numtrainers", required_argument,0,'t' },
//            {0,0,0,0   }
//    };
//    int long_index =0;
//    while ((opt = getopt_long(argc, argv,"t:", long_options, &long_index )) != -1) {
//        switch (opt) {
//            case 't' : numtrainers = atoi(optarg);
//                break;
//            default: print_usage();
//                exit(EXIT_FAILURE);
//        }
//    }

    //save database information into classes
    if (storePokemon() != 0) {
        std::cout << "File not opened successfully. File: pokemon.csv" << "\n";
    }
    if (storeMoves() != 0) {
        std::cout << "File not opened successfully. File: moves.csv" << "\n";
    }
    if (storePokemonMoves() != 0) {
        std::cout << "File not opened successfully. File: pokemon_moves.csv" << "\n";
    }
    if (storePokemonSpecies() != 0) {
        std::cout << "File not opened successfully. File: pokemon_species.csv" << "\n";
    }
    if (storeExperience() != 0) {
        std::cout << "File not opened successfully. File: experience.csv" << "\n";
    }
    if (storeTypeNames() != 0) {
        std::cout << "File not opened successfully. File: type_names.csv" << "\n";
    }
    if (storePokemonStats() != 0) {
        std::cout << "File not opened successfully. File: pokemon_stats.csv" << "\n";
    }
    if (storePokemonTypes() != 0) {
        std::cout << "File not opened successfully. File: pokemon_types.csv" << "\n";
    }
    if (argc < 2) {
        std::cout << "No arguments provided." << "\n";
    }
    else {
        std::string fileName = argv[1];
        if (fileName == "pokemon") {
            for (int i = 0; i < (int) allPokemonInfo.size(); i++) {
                std::cout << allPokemonInfo[i]->toString() << "\n";
            }
        } else if (fileName == "moves") {
            for (int i = 0; i < (int) allMoves.size(); i++) {
                std::cout << allMoves[i]->toString() << "\n";
            }
        } else if (fileName == "pokemon_moves") {
            for (int i = 0; i < (int) allPokemonMoves.size(); i++) {
                std::cout << allPokemonMoves[i]->toString() << "\n";
            }
        } else if (fileName == "pokemon_species") {
            for (int i = 0; i < (int) allPokemonSpecies.size(); i++) {
                std::cout << allPokemonSpecies[i]->toString() << "\n";
            }
        } else if (fileName == "experience") {
            for (int i = 0; i < (int) allExperience.size(); i++) {
                std::cout << allExperience[i]->toString() << "\n";
            }
        } else if (fileName == "type_names") {
            for (int i = 0; i < (int) allTypeNames.size(); i++) {
                std::cout << allTypeNames[i]->toString() << "\n";
            }
        } else if (fileName == "pokemon_stats") {
            for (int i = 0; i < (int) allPokemonStats.size(); i++) {
                std::cout << allPokemonStats[i]->toString() << "\n";
            }
        } else {
            std::cout << "Input file name: " << fileName << " is not a valid file" << "\n";
            return 2;
        }
    }


    //check argument legality
    if (numtrainers < 0) {
        numtrainers = 0;
    }
    else if (numtrainers > MAX_NUM_TRAINERS) {
        numtrainers = MAX_NUM_TRAINERS;
    }
    num_trainers = numtrainers;

    //run program
    srand(time(NULL));
    interface->initializeTerminalUI();
    heap_init(&turn_heap, comparator_character_movement, NULL);
    Tile home_tile = create_tile(WORLD_CENTER_X, WORLD_CENTER_Y);
    current_tile_x = WORLD_CENTER_X;
    current_tile_y = WORLD_CENTER_Y;
    world[WORLD_CENTER_Y][WORLD_CENTER_X] = &home_tile;
    place_player_character(world[current_tile_y][current_tile_x]);
    while (turn_based_movement() == -1) {
        //-1 signals map was changed: call turn_based_movement for new map/turn heap
        //old and new Tile and heap have been updated correctly in change Tile (removed from old heap in turn_based_movement)
    }
    interface->endwinUI();
    return 0;

}

int print_usage() {

    //print expected inputs

    return 0;

}

int storePokemon() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/pokemon.csv");
    if (file.is_open()) {
        std:: string id, name, species_id, height, weight, base_experience, order, is_default;
        getline(file, id, '\n');
        while(getline(file, id, ',')) {
            getline(file, name, ',');
            getline(file, species_id, ',');
            getline(file, height, ',');
            getline(file, weight, ',');
            getline(file, base_experience, ',');
            getline(file, order, ',');
            getline(file, is_default, '\n');
            if (id == "") {
                id = "-1";
            }
            if (name == "") {
                name = "-1";
            }
            if (species_id == "") {
                species_id = "-1";
            }
            if (height == "") {
                height = "-1";
            }
            if (weight == "") {
                weight = "-1";
            }
            if (base_experience == "") {
                base_experience = "-1";
            }
            if (order == "") {
                order = "-1";
            }
            if (is_default == "") {
                is_default = "-1";
            }
            PokemonInfo *pokemon = new PokemonInfo(id, name, species_id, height, weight, base_experience, order, is_default);
            allPokemonInfo.push_back(pokemon);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

int storeMoves() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/moves.csv");
    if (file.is_open()) {
        std:: string id, name, generation_id, type_id, power, pp, accuracy, priority, target_id, damage_class_id, effect_id,
                effect_chance, contest_type_id, contest_effect_id, super_contest_effect_id;
        getline(file, id, '\n');
        while(getline(file, id, ',')) {
            getline(file, name, ',');
            getline(file, generation_id, ',');
            getline(file, type_id, ',');
            getline(file, power, ',');
            getline(file, pp, ',');
            getline(file, accuracy, ',');
            getline(file, priority, ',');
            getline(file, target_id, ',');
            getline(file, damage_class_id, ',');
            getline(file, effect_id, ',');
            getline(file, effect_chance, ',');
            getline(file, contest_type_id, ',');
            getline(file, contest_effect_id, ',');
            getline(file, super_contest_effect_id, '\n');
            if (id == "") {
                id = "-1";
            }
            if (name == "") {
                name = "-1";
            }
            if (generation_id == "") {
                generation_id = "-1";
            }
            if (type_id == "") {
                type_id = "-1";
            }
            if (power == "") {
                power = "-1";
            }
            if (pp == "") {
                pp = "-1";
            }
            if (accuracy == "") {
                accuracy = "-1";
            }
            if (priority == "") {
                priority = "-1";
            }
            if (target_id == "") {
                target_id = "-1";
            }
            if (damage_class_id == "") {
                damage_class_id = "-1";
            }
            if (effect_id == "") {
                effect_id = "-1";
            }
            if (effect_chance == "") {
                effect_chance = "-1";
            }
            if (contest_type_id == "") {
                contest_type_id = "-1";
            }
            if (contest_effect_id == "") {
                contest_effect_id = "-1";
            }
            if (super_contest_effect_id == "") {
                super_contest_effect_id = "-1";
            }
            Move *move = new Move(id, name, generation_id, type_id, power, pp, accuracy, priority, target_id, damage_class_id,
                                  effect_id, effect_chance, contest_type_id, contest_effect_id, super_contest_effect_id);
            allMoves.push_back(move);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

int storePokemonMoves() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/pokemon_moves.csv");
    if (file.is_open()) {
        std:: string pokemon_id, version_group_id, move_id, pokemon_move_method_id, level, order;
        getline(file, pokemon_id, '\n');
        while(getline(file, pokemon_id, ',')) {
            getline(file, version_group_id, ',');
            getline(file, move_id, ',');
            getline(file, pokemon_move_method_id, ',');
            getline(file, level, ',');
            getline(file, order, '\n');
            if (pokemon_id == "") {
                pokemon_id = "-1";
            }
            if (version_group_id == "") {
                version_group_id = "-1";
            }
            if (move_id == "") {
                move_id = "-1";
            }
            if (pokemon_move_method_id == "") {
                pokemon_move_method_id = "-1";
            }
            if (level == "") {
                level = "-1";
            }
            if (order == "") {
                order = "-1";
            }
            PokemonMove *pokemonMove = new PokemonMove(pokemon_id, version_group_id, move_id, pokemon_move_method_id, level,
                order);
            allPokemonMoves.push_back(pokemonMove);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

int storePokemonSpecies() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/pokemon_species.csv");
    if (file.is_open()) {
        std:: string id, identifier, generation_id, evolves_from_species_id, evolution_chain_id, color_id, shape_id,
            habitat_id, gender_rate, capture_rate, base_happiness, is_baby, hatch_counter, has_gender_differences,
            growth_rate_id, forms_switchable, is_legendary, is_mythical, order, conquest_order;
        getline(file, id, '\n');
        while(getline(file, id, ',')) {
            getline(file, identifier, ',');
            getline(file, generation_id, ',');
            getline(file, evolves_from_species_id, ',');
            getline(file, evolution_chain_id, ',');
            getline(file, color_id, ',');
            getline(file, shape_id, ',');
            getline(file, habitat_id, ',');
            getline(file, gender_rate, ',');
            getline(file, capture_rate, ',');
            getline(file, base_happiness, ',');
            getline(file, is_baby, ',');
            getline(file, hatch_counter, ',');
            getline(file, has_gender_differences, ',');
            getline(file, growth_rate_id, ',');
            getline(file, forms_switchable, ',');
            getline(file, is_legendary, ',');
            getline(file, is_mythical, ',');
            getline(file, order, ',');
            getline(file, conquest_order, '\n');
            if (id == "") {
                id = "-1";
            }
            if (identifier == "") {
                identifier = "-1";
            }
            if (generation_id == "") {
                generation_id = "-1";
            }
            if (generation_id == "") {
                generation_id = "-1";
            }
            if (evolves_from_species_id == "") {
                evolves_from_species_id = "-1";
            }
            if (evolution_chain_id == "") {
                evolution_chain_id = "-1";
            }
            if (color_id == "") {
                color_id = "-1";
            }
            if (shape_id == "") {
                shape_id = "-1";
            }
            if (habitat_id == "") {
                habitat_id = "-1";
            }
            if (gender_rate == "") {
                gender_rate = "-1";
            }
            if (capture_rate == "") {
                capture_rate = "-1";
            }
            if (base_happiness == "") {
                base_happiness = "-1";
            }
            if (is_baby == "") {
                is_baby = "-1";
            }
            if (hatch_counter == "") {
                hatch_counter = "-1";
            }
            if (has_gender_differences == "") {
                has_gender_differences = "-1";
            }
            if (growth_rate_id == "") {
                growth_rate_id = "-1";
            }
            if (forms_switchable == "") {
                forms_switchable = "-1";
            }
            if (is_legendary == "") {
                is_legendary = "-1";
            }
            if (is_mythical == "") {
                is_mythical = "-1";
            }
            if (order == "") {
                order = "-1";
            }
            if (conquest_order == "") {
                conquest_order = "-1";
            }
            PokemonSpecies *pokemonSpecies = new PokemonSpecies(id, identifier, generation_id, evolves_from_species_id,
                evolution_chain_id, color_id, shape_id, habitat_id, gender_rate, capture_rate, base_happiness, is_baby,
                hatch_counter, has_gender_differences, growth_rate_id, forms_switchable, is_legendary, is_mythical, order,
                conquest_order);
            allPokemonSpecies.push_back(pokemonSpecies);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

int storeExperience() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/experience.csv");
    if (file.is_open()) {
        std::string growth_rate_id, level, experience;
        getline(file, growth_rate_id, '\n');
        while(getline(file, growth_rate_id, ',')) {
            getline(file, level, ',');
            getline(file, experience, '\n');
            if (growth_rate_id == "") {
                growth_rate_id = "-1";
            }
            if (level == "") {
                level = "-1";
            }
            if (experience == "") {
                experience = "-1";
            }
            Experience *experience1 = new Experience(growth_rate_id, level, experience);
            allExperience.push_back(experience1);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

int storeTypeNames() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/type_names.csv");
    if (file.is_open()) {
        std::string type_id, local_language_id, name;
        getline(file, type_id, '\n');
        while(getline(file, type_id, ',')) {
            getline(file, local_language_id, ',');
            getline(file, name, '\n');
            if (type_id == "") {
                type_id = "-1";
            }
            if (local_language_id == "") {
                local_language_id = "-1";
            }
            if (name == "") {
                name = "-1";
            }
            TypeName *typeName = new TypeName(type_id, local_language_id, name);
            allTypeNames.push_back(typeName);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

int storePokemonStats() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/pokemon_stats.csv");
    if (file.is_open()) {
        std::string pokemon_id, stat_id, base_stat, effort;
        getline(file, pokemon_id, '\n');
        while(getline(file, pokemon_id, ',')) {
            getline(file, stat_id, ',');
            getline(file, base_stat, ',');
            getline(file, effort, '\n');
            if (pokemon_id == "") {
                pokemon_id = "-1";
            }
            if (stat_id == "") {
                stat_id = "-1";
            }
            if (base_stat == "") {
                base_stat = "-1";
            }
            if (effort == "") {
                effort = "-1";
            }
            PokemonStat *stat = new PokemonStat(pokemon_id, stat_id, base_stat, effort);
            allPokemonStats.push_back(stat);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

int storePokemonTypes() {

    std::ifstream file;
    file.open(filePath + "pokedex/pokedex/data/csv/pokemon_types.csv");
    if (file.is_open()) {
        std::string pokemon_id, type_id, slot;
        getline(file, pokemon_id, '\n');
        while(getline(file, pokemon_id, ',')) {
            getline(file, type_id, ',');
            getline(file, slot, '\n');
            if (pokemon_id == "") {
                pokemon_id = "-1";
            }
            if (type_id == "") {
                type_id = "-1";
            }
            if (slot == "") {
                slot = "-1";
            }
            PokemonType *type = new PokemonType(pokemon_id, type_id, slot);
            allPokemonTypes.push_back(type);
        }
    }
    else {
        //file not opened successfully
        return 1;
    }

    return 0;

}

//Commented due to DatabaseInfo failing to make
//Print any data type to follow DRY principle
//int printData(std::vector<DatabaseInfo *> dataVector) {
//
//    for (int i = 0; i < (int)allPokemonInfo.size(); i++) {
//        std::cout << dataVector[i]->toString() << "\n";
//    }
//
//    return 0;
//
//}

int turn_based_movement() {

    Tile *tile = world[current_tile_y][current_tile_x];
    static Character *character;
    while ((character = (Character *) (heap_remove_min(&turn_heap)))) {
        if (character->type_enum == PLAYER) {
            interface->clearUI();
            interface->addstrUI("It's your turn! Enter a command or press z for help!\n");
            print_tile_terrain(tile);
            interface->refreshUI();
            int result = player_turn();
            if (result != 0) {
                return result;
            }
        }
        else if (character->type_enum == RIVAL) {
            if (character->defeated == 1) {
                //no longer paths to PC
                character->turn += MINIMUM_TURN;
            } else {
                //find a legal Point to change_tile to
                int new_x;
                int new_y;
                int new_distance = INT_MAX;
                for (int x = -1; x <= 1; x++) {
                    for (int y = -1; y <= 1; y++) {
                        int candidate_x = character->x + x;
                        int candidate_y = character->y + y;
                        if (candidate_x > 0 && candidate_x < TILE_WIDTH_X && candidate_y > 0 &&
                            candidate_y < TILE_LENGTH_Y
                            && rival_distance_tile[candidate_y][candidate_x] != INT_MAX
                            && (tile->tile[candidate_y][candidate_x].character == NULL
                                || (tile->tile[candidate_y][candidate_x].character->type_enum == PLAYER &&
                                    character->defeated == 0))) {
                            if (rival_distance_tile[candidate_y][candidate_x] < new_distance) {
                                new_x = candidate_x;
                                new_y = candidate_y;
                                new_distance = rival_distance_tile[candidate_y][candidate_x];
                            }
                        }
                    }
                }
                if (new_distance != INT_MAX) {
                    //if legal Point to move to found, change_tile there
                    move_character(character->x, character->y, new_x, new_y);
                    character->turn += tile->tile[new_y][new_x].terrain.rival_weight;
                } else {
                    //no legal Point to change_tile to found
                    character->turn += MINIMUM_TURN;
                }
            }
        }
        else if (character->type_enum == HIKER) {
            if (character->defeated == 1) {
                //no longer paths to PC
                character->turn += MINIMUM_TURN;
            }
            else {
                int new_x;
                int new_y;
                int new_distance = INT_MAX;
                for (int x = -1; x <= 1; x++) {
                    for (int y = -1; y <= 1; y++) {
                        int candidate_x = character->x + x;
                        int candidate_y = character->y + y;
                        if (candidate_x > 0 && candidate_x < TILE_WIDTH_X && candidate_y > 0 &&
                            candidate_y < TILE_LENGTH_Y
                            && hiker_distance_tile[candidate_y][candidate_x] != INT_MAX
                            && (tile->tile[candidate_y][candidate_x].character == NULL
                                || (tile->tile[candidate_y][candidate_x].character->type_enum == PLAYER &&
                                    character->defeated == 0))) {
                            if (hiker_distance_tile[candidate_y][candidate_x] < new_distance) {
                                new_x = candidate_x;
                                new_y = candidate_y;
                                new_distance = hiker_distance_tile[candidate_y][candidate_x];
                            }
                        }
                    }
                }
                if (new_distance != INT_MAX) {
                    move_character(character->x, character->y, new_x, new_y);
                    character->turn += tile->tile[new_y][new_x].terrain.hiker_weight;
                } else {
                    character->turn += MINIMUM_TURN;
                }
            }
        }
        else if (character->type_enum == RANDOM_WALKER) {
            int new_x = character->x + character->x_direction;
            int new_y = character->y + character->y_direction;
            //if we have a direction set and can continue in it
            if (character->direction_set == 1 && new_x > 0 && new_x < TILE_WIDTH_X && new_y > 0 && new_y < TILE_LENGTH_Y
                && tile->tile[new_y][new_x].terrain.rival_weight != INT_MAX
                && (tile->tile[new_y][new_x].character == NULL
                    || (tile->tile[new_y][new_x].character->type_enum == PLAYER && character->defeated == 0))) {
                move_character(character->x, character->y, new_x, new_y);
                character->turn += tile->tile[new_y][new_x].terrain.rival_weight;
            }
                //no direction set or can't continue in set direction
            else {
                //if at least 1 direction legal, choose randomly until legal direction is found
                int has_possible_direction = 0;
                for (int y = -1; y <= 1; y++) {
                    for (int x = -1; x <= 1; x++) {
                        if (x != 0 || y != 0) {
                            if (tile->tile[character->y + y][character->x + x].terrain.rival_weight != INT_MAX
                                && (tile->tile[character->y + y][character->x + x].character == NULL
                                    || (tile->tile[character->y + y][character->x + x].character->type_enum == PLAYER
                                        && character->defeated == 0))) {
                                has_possible_direction = 1;
                            }
                        }
                    }
                }
                if (has_possible_direction == 1) {
                    int found = 0;
                    int x;
                    int y;
                    while (found == 0) {
                        x = rand() % 3 - 1;
                        y = rand() % 3 - 1;
                        new_x = character->x + x;
                        new_y = character->y + y;
                        if ((x != 0 || y != 0) && new_x > 0 && new_x < TILE_WIDTH_X && new_y > 0 && new_y < TILE_LENGTH_Y
                            && tile->tile[new_y][new_x].terrain.rival_weight != INT_MAX
                            && (tile->tile[new_y][new_x].character == NULL
                                || (tile->tile[new_y][new_x].character->type_enum == PLAYER && character->defeated == 0))) {
                            found = 1;
                        }
                    }
                    character->x_direction = x;
                    character->y_direction = y;
                    character->direction_set = 1;
                    move_character(character->x, character->y, new_x, new_y);
                    character->turn += tile->tile[new_y][new_x].terrain.rival_weight;
                }
                else {
                    character->turn += MINIMUM_TURN;
                }
            }
        }
        else if (character->type_enum == PACER) {
            int new_x = character->x + character->x_direction;
            int new_y = character->y + character->y_direction;
            //change_tile in direction
            if (character->direction_set == 1 && new_x > 0 && new_x < TILE_WIDTH_X && new_y > 0 && new_y < TILE_LENGTH_Y
                && tile->tile[new_y][new_x].terrain.rival_weight != INT_MAX
                && (tile->tile[new_y][new_x].character == NULL || tile->tile[new_y][new_x].character->type_enum == PLAYER)) {
                move_character(character->x, character->y, new_x, new_y);
                character->turn += tile->tile[new_y][new_x].terrain.rival_weight;
            }
                //reverse direction
            else if (character->direction_set == 1) {
                character->x_direction *= -1;
                character->y_direction *= -1;
                int new_x = character->x + character->x_direction;
                int new_y = character->y + character->y_direction;
                move_character(character->x, character->y, new_x, new_y);
            }
                //no direction yet
            else {
                //if at least 1 direction legal, choose randomly until legal direction is found
                int has_possible_direction = 0;
                for (int y = -1; y <= 1; y++) {
                    for (int x = -1; x <= 1; x++) {
                        if (x != 0 || y != 0) {
                            if (tile->tile[character->y + y][character->x + x].terrain.rival_weight != INT_MAX
                                && (tile->tile[character->y + y][character->x + x].character == NULL
                                    || (tile->tile[character->y + y][character->x + x].character->type_enum == PLAYER
                                        && character->defeated == 0))) {
                                has_possible_direction = 1;
                            }
                        }
                    }
                }
                if (has_possible_direction == 1) {
                    int found = 0;
                    int x;
                    int y;
                    while (found == 0) {
                        x = rand() % 3 - 1;
                        y = rand() % 3 - 1;
                        new_x = character->x + x;
                        new_y = character->y + y;
                        if ((x != 0 || y != 0) && new_x > 0 && new_x < TILE_WIDTH_X && new_y > 0 && new_y < TILE_LENGTH_Y
                            && tile->tile[new_y][new_x].terrain.rival_weight != INT_MAX
                            && (tile->tile[new_y][new_x].character == NULL
                                || (tile->tile[new_y][new_x].character->type_enum == PLAYER && character->defeated == 0))) {
                            found = 1;
                        }
                    }
                    character->x_direction = x;
                    character->y_direction = y;
                    character->direction_set = 1;
                    move_character(character->x, character->y, new_x, new_y);
                    character->turn += tile->tile[new_y][new_x].terrain.rival_weight;
                }
                else {
                    character->turn += MINIMUM_TURN;
                }
            }
        }
        else if (character->type_enum == WANDERER) {
            int new_x = character->x + character->x_direction;
            int new_y = character->y + character->y_direction;
            if (character->direction_set == 1 && new_x > 0 && new_x < TILE_WIDTH_X && new_y > 0 && new_y < TILE_LENGTH_Y
                && tile->tile[new_y][new_x].terrain.id == tile->tile[character->y][character->x].terrain.id
                && (tile->tile[new_y][new_x].character == NULL
                    || (tile->tile[new_y][new_x].character->type_enum == PLAYER && character->defeated == 0))) {
                move_character(character->x, character->y, new_x, new_y);
                character->turn += tile->tile[new_y][new_x].terrain.rival_weight;
            }
            else {
                //if at least 1 direction legal, choose randomly until legal direction is found
                int has_possible_direction = 0;
                for (int y = -1; y <= 1; y++) {
                    for (int x = -1; x <= 1; x++) {
                        if (x != 0 || y != 0) {
                            if ((tile->tile[character->y + y][character->x + x].terrain.id
                                 == tile->tile[character->y][character->x].terrain.id)
                                && (tile->tile[character->y][character->x].character == NULL
                                    || (tile->tile[character->y][character->x].character->type_enum == PLAYER && character->defeated == 0))) {
                                has_possible_direction = 1;
                            }
                        }
                    }
                }
                if (has_possible_direction == 1) {
                    int found = 0;
                    int x;
                    int y;
                    while (found == 0) {
                        x = rand() % 3 - 1;
                        y = rand() % 3 - 1;
                        new_x = character->x + x;
                        new_y = character->y + y;
                        if ((x != 0 || y != 0) && new_x > 0 && new_x < TILE_WIDTH_X && new_y > 0 && new_y < TILE_LENGTH_Y
                            && tile->tile[new_y][new_x].terrain.id == tile->tile[character->y][character->x].terrain.id
                            && (tile->tile[new_y][new_x].character == NULL
                                || (tile->tile[new_y][new_x].character->type_enum == PLAYER && character->defeated == 0))) {
                            found = 1;
                        }
                    }
                    character->x_direction = x;
                    character->y_direction = y;
                    character->direction_set = 1;
                    move_character(character->x, character->y, new_x, new_y);
                    character->turn += tile->tile[new_y][new_x].terrain.rival_weight;
                }
                else {
                    character->turn += MINIMUM_TURN;
                }
            }
        }
        else if (character->type_enum == STATIONARY) {
            character->turn += MINIMUM_TURN;
        }
        heap_insert(&turn_heap, character);
    }
    heap_delete(&turn_heap);

    return 0;

}

int player_turn() {

    Tile *tile = world[current_tile_y][current_tile_x];
    int turn_completed = 0;
    int in_help = 0;
    int x = player_character->x;
    int y = player_character->y;
    while (turn_completed == 0) {
        int input = interface->getchUI();
        int moving = 0;
        int new_x = x;
        int new_y = y;
        //determine input
        if (input == '7' || input == 'y') {
            moving = 1;
            new_x--;
            new_y--;
        } else if (input == '8' || input == 'k') {
            moving = 1;
            new_y--;
        } else if (input == '9' || input == 'u') {
            moving = 1;
            new_x++;
            new_y--;
        } else if (input == '6' || input == 'l') {
            moving = 1;
            new_x++;
        } else if (input == '3' || input == 'n') {
            moving = 1;
            new_x++;
            new_y++;
        } else if (input == '2' || input == 'j') {
            moving = 1;
            new_y++;
        } else if (input == '1' || input == 'b') {
            moving = 1;
            new_x--;
            new_y++;
        } else if (input == '4' || input == 'h') {
            moving = 1;
            new_x--;
        } else if (input == '>') {
            if (tile->tile[y][x].terrain.id == center->id) {
                enter_center();
            } else if (tile->tile[y][x].terrain.id == mart->id) {
                enter_mart();
            } else {
                interface->clearUI();
                interface->addstrUI("There is no pokecenter or pokemart here so you can't enter one!\n");
                print_tile_terrain(tile);
            }
        } else if (input == '<') {
            if (player_character->in_building == 1) {
                interface->clearUI();
                interface->addstrUI("You have left the building!\n");
                print_tile_terrain(tile);
            } else {
                interface->clearUI();
                interface->addstrUI("You aren't in a building so you can't leave one!\n");
                print_tile_terrain(tile);
            }
        } else if (input == '5' || input == ' ' || input == '.') {
            player_character->turn += MINIMUM_TURN;
            turn_completed = 1;
        } else if (input == 't') {
            NonPlayerCharacter *trainers [num_trainers];
            int count = 0;
            for (int i = 1; i < TILE_LENGTH_Y - 1; i++) {
                for (int j = 1; j < TILE_WIDTH_X - 1; j++) {
                    Character *character = tile->tile[i][j].character;
                    if (character != NULL && character->type_enum != PLAYER) {
                        trainers[count] = (NonPlayerCharacter *) character;
                        count++;
                    }
                }
            }
            int position = 0;
            int screen_row = 1;
            int type_x = 0;
            int position_x = 19;
            int defeated_status_x = 40;
            interface->clearUI();
            interface->addstrUI("Trainer list: Press escape to return to the map\n");
            for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                NonPlayerCharacter *trainer = trainers[i];
                interface->mvaddstrUI(screen_row, type_x, trainer->type_string.c_str());
                interface->mvaddstrUI(screen_row, position_x, " ");
                if (trainer->y != player_character->y) {
                    char y_distance[3];
                    if (trainer->y < player_character->y) {
                        sprintf(y_distance, "%d ", player_character->y - trainer->y);
                        interface->addstrUI(y_distance);
                        interface->addstrUI("North ");
                    }
                    else {
                        sprintf(y_distance, "%d ", trainer->y - player_character->y);
                        interface->addstrUI(y_distance);
                        interface->addstrUI("South ");
                    }
                }
                if (trainer->x != player_character->x) {
                    char x_distance[3];
                    if (trainer->x < player_character->x) {
                        sprintf(x_distance, "%d ", player_character->x - trainer->x);
                        interface->addstrUI(x_distance);
                        interface->addstrUI("West");
                    }
                    else {
                        sprintf(x_distance, "%d ", trainer->x - player_character->x);
                        interface->addstrUI(x_distance);
                        interface->addstrUI("East");
                    }
                }
                if (trainer->defeated == 1) {
                    interface->mvaddstrUI(screen_row, defeated_status_x, "Defeated");
                }
                interface->addstrUI("\n");
                screen_row++;
            }
            interface->refreshUI();
            chtype command = -1;
            while (command != 27 && command != ACS_UARROW && command != ACS_DARROW) {
                command = interface->getchUI();
                screen_row = 1;
                if (command == 27) {
                    turn_completed = 1;
                }
                else if (command == KEY_UP) {
                    //xtodo: BUG TEST: test scroll up
                    if (position > 1) {
                        position -= SCREEN_HEIGHT - 1;
                        if (position < 0) {
                            position = 0;
                        }
                        interface->clearUI();
                        interface->addstrUI("Trainer list: Press escape to return to the map\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            interface->mvaddstrUI(screen_row, type_x, trainer->type_string.c_str());
                            interface->mvaddstrUI(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                interface->mvaddstrUI(screen_row, defeated_status_x, "Defeated");
                            }
                            interface->addstrUI("\n");
                            screen_row++;
                        }
                        interface->refreshUI();
                    }
                    else {
                        interface->clearUI();
                        interface->addstrUI("You are already at the top of the list so you cannot scroll up.\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            interface->mvaddstrUI(screen_row, type_x, trainer->type_string.c_str());
                            interface->mvaddstrUI(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                interface->mvaddstrUI(screen_row, defeated_status_x, "Defeated");
                            }
                            interface->addstrUI("\n");
                            screen_row++;
                        }
                        interface->refreshUI();
                    }
                }
                else if (command == KEY_DOWN) {
                    if (position < num_trainers - SCREEN_HEIGHT + 1) {
                        position += SCREEN_HEIGHT - 1;
                        interface->clearUI();
                        interface->addstrUI("Trainer list: Press escape to return to the map\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            interface->mvaddstrUI(screen_row, type_x, trainer->type_string.c_str());
                            interface->mvaddstrUI(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                interface->mvaddstrUI(screen_row, defeated_status_x, "Defeated");
                            }
                            interface->addstrUI("\n");
                            screen_row++;
                        }
                        interface->refreshUI();
                    }
                    else {
                        interface->clearUI();
                        interface->addstrUI("You are already at the bottom of the list so you cannot scroll down.\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            interface->mvaddstrUI(screen_row, type_x, trainer->type_string.c_str());
                            interface->mvaddstrUI(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    interface->addstrUI(y_distance);
                                    interface->addstrUI("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    interface->addstrUI(x_distance);
                                    interface->addstrUI("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                interface->mvaddstrUI(screen_row, defeated_status_x, "Defeated");
                            }
                            interface->addstrUI("\n");
                            screen_row++;
                        }
                        interface->refreshUI();
                    }
                }
                else {
                    //command is invalid
                    interface->clearUI();
                    interface->addstrUI("That is not a valid command! Press escape to return to the map.\n");
                    for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                        NonPlayerCharacter *trainer = trainers[i];
                        interface->mvaddstrUI(screen_row, type_x, trainer->type_string.c_str());
                        interface->mvaddstrUI(screen_row, position_x, " ");
                        if (trainer->y != player_character->y) {
                            char y_distance[3];
                            if (trainer->y < player_character->y) {
                                sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                interface->addstrUI(y_distance);
                                interface->addstrUI("North ");
                            }
                            else {
                                sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                interface->addstrUI(y_distance);
                                interface->addstrUI("South ");
                            }
                        }
                        if (trainer->x != player_character->x) {
                            char x_distance[3];
                            if (trainer->x < player_character->x) {
                                sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                interface->addstrUI(x_distance);
                                interface->addstrUI("West");
                            }
                            else {
                                sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                interface->addstrUI(x_distance);
                                interface->addstrUI("East");
                            }
                        }
                        if (trainer->defeated == 1) {
                            interface->mvaddstrUI(screen_row, defeated_status_x, "Defeated");
                        }
                        interface->addstrUI("\n");
                        screen_row++;
                    }
                    interface->refreshUI();
                }
            }
        } else if (input == 'Q') {
            interface->clearUI();
            interface->addstrUI("Are you sure you want to quit (y/n)? All progress will be lost.\n");
            interface->refreshUI();
            int quit = -1;
            while (quit != 'y' || quit != 'n') {
                quit = interface->getchUI();
                if (quit == 'y') {
                    return 1;
                } else if (quit == 'n') {
                    interface->clearUI();
                    interface->addstrUI("It's your turn! Enter a command or press z for help!\n");
                    print_tile_terrain(tile);
                } else {
                    interface->clearUI();
                    interface->addstrUI("Please enter (y/n) to quit. If you quit all progress will be lost.\n");
                    interface->refreshUI();
                }
            }
        } else if (input == 'z') {
            if (in_help == 0) {
                //enter help
                interface->clearUI();
                interface->addstrUI("Enter z to enter/leave the help menu.\n");
                interface->addstrUI("Enter 9 or u to move one cell to the upper right.\n");
                interface->addstrUI("Enter 8 or k to move one cell up.\n");
                interface->addstrUI("Enter 7 or y to move one cell to the upper left.\n");
                interface->addstrUI("Enter 6 or l to move one cell to the right.\n");
                interface->addstrUI("Enter 5 or space or . to rest for a turn.\n");
                interface->addstrUI("Enter 4 or h to move one cell to the left.\n");
                interface->addstrUI("Enter 3 or n to move one cell to the lower right.\n");
                interface->addstrUI("Enter 2 or j to move one cell down.\n");
                interface->addstrUI("Enter 1 or b to move one cell to the lower left.\n");
                interface->addstrUI("Enter > to enter a pokemart or pokecenter.\n");
                interface->addstrUI("Enter < to leave a pokemart or pokecenter.\n");
                interface->addstrUI("Enter t to display a list of trainers.\n");
                interface->addstrUI("Enter up arrow to scroll up on the trainer list.\n");
                interface->addstrUI("Enter down arrow to scroll up on the trainer list.\n");
                interface->addstrUI("Enter escape to leave the trainer list.\n");
                interface->addstrUI("Enter Q to quit the game.\n");
                interface->refreshUI();
            }
            else {
                //exit help
                interface->clearUI();
                interface->addstrUI("It's your turn! Enter a command or press z for help!\n");
                print_tile_terrain(tile);
            }
            in_help = 1 - in_help;
        } else {
            interface->clearUI();
            interface->addstrUI("That is not a valid command. Enter z for help!\n");
            print_tile_terrain(tile);
            interface->refreshUI();
        }

        //call movement function if moving
        if (moving == 1) {
            //if Terrain can be crossed
            if (tile->tile[new_y][new_x].terrain.pc_weight == INT_MAX) {
                interface->clearUI();
                interface->addstrUI("You can't cross that kind of Terrain!\n");
                print_tile_terrain(tile);
            }
                //if there is an undefeated trainer there
            else if (tile->tile[new_y][new_x].character != NULL && tile->tile[new_y][new_x].character->defeated != 0) {
                interface->clearUI();
                interface->addstrUI("You have already defeated that trainer so they are too scared to battle you again!");
                print_tile_terrain(tile);
            }
                //if you are exiting the map
            else if (new_y == 0 || new_y == TILE_LENGTH_Y - 1 || new_x == 0 || new_x == TILE_WIDTH_X - 1) {
                if (change_tile(tile->x + new_x - x, tile->y + new_y - y) == 0) {
                    tile->tile[y][x].character = NULL;
                    //Tile in this function is new Tile
                    tile = world[current_tile_y][current_tile_x];
                    //successfully changed tiles
                    //updates PC coordinates
                    //todo: RUN BUG: going back to old map SOMETIMES creates a new map replacing old map: debug by writing map coords when in map
                    if (new_x == 0) {
                        player_character->x = TILE_WIDTH_X - 2;
                    } else if (new_x == TILE_WIDTH_X - 1) {
                        player_character->x = 1;
                    } else if (new_y == 0) {
                        player_character->y = TILE_LENGTH_Y - 2;
                    } else if (new_y == TILE_LENGTH_Y - 1) {
                        player_character->y = 1;
                    }
                    tile->tile[player_character->y][player_character->x].character = player_character;
                    //refactors trainer distance tiles
                    dijkstra(tile, RIVAL);
                    dijkstra(tile, HIKER);
                    //tells turn_based_movement that we have changed tiles
                    return -1;
                }
                else {
                    //todo: RUN BUG TEST: test trying to move off of edge of world
                    //cannot change Tile because at edge of world
                    interface->clearUI();
                    interface->addstrUI("You can't go off of the edge of the world like that! It's your turn! Enter a command or press z for help!\n");
                    print_tile_terrain(tile);
                    interface->refreshUI();
                }
            }
            else {
                move_character(x, y, new_x, new_y);
                player_character->turn += tile->tile[new_y][new_x].terrain.pc_weight;
                //recreate distance tiles for new PC location
                dijkstra(tile, RIVAL);
                dijkstra(tile, HIKER);
                turn_completed = 1;
            }
        }
    }
    return 0;

}

int move_character(int x, int y, int new_x, int new_y) {

    Tile *tile = world[current_tile_y][current_tile_x];
    Point point = tile->tile[new_y][new_x];
    Character *from_character = tile->tile[y][x].character;
    Character *to_character = tile->tile[new_y][new_x].character;
    //if moving onto character
    if (to_character != NULL) {
        //pc-trainer combat_trainer instigated by either party
        if (from_character->type_enum == PLAYER || to_character->type_enum == PLAYER) {
            if (from_character->type_enum == PLAYER && to_character->defeated == 1) {
                //PC should not be allowed to move to a defeated trainer's location (as of assignment 1.05)
                return 2;
            }
            else {
                combat_trainer(from_character, to_character);
            }
        }
            //trainer -> trainer = no move
        else {
            //this should never happen because this is checked in turn_based_movement in trainer movement
            return 1;
        }
    }
    else {
        tile->tile[y][x].character->x = new_x;
        tile->tile[y][x].character->y = new_y;
        Character *temp_character = tile->tile[y][x].character;
        tile->tile[y][x].character = NULL;
        tile->tile[new_y][new_x].character = temp_character;
    }
    if (from_character->type_enum == PLAYER && point.terrain.id == grass->id) {
        if (rand() % INVERSE_POKEMON_ENCOUNTER_CHANCE == 0) {
            Pokemon *pokemon = create_pokemon();
            combat_pokemon(pokemon);
        }
    }
    return 0;

}

int combat_trainer(Character *from_character, Character *to_character) {

    //todo: ASSIGNED: same as combat pokemon, but trainer gets choices too

    if (from_character->type_enum == PLAYER) {
        //player attacks trainer
        to_character->defeated = 1;
        to_character->color = COLOR_YELLOW;
        interface->clearUI();
        interface->addstrUI("Victory! You challenged a trainer to a duel and defeated them soundly! Press escape to leave.\n");
        interface->refreshUI();
    }
    else {
        from_character->defeated = 1;
        from_character->color = COLOR_YELLOW;
        //trainer attacks player
        interface->clearUI();
        interface->addstrUI("Victory! A trainer challenged you to a duel and you trounced them! Press escape to leave.\n");
        interface->refreshUI();
    }
    int command = -1;
    while (command != 27) {
        command = interface->getchUI();
        interface->clearUI();
        interface->addstrUI("Invalid command. Press press escape to stop your victory dance after defeating that trainer.\n");
        interface->refreshUI();
    }
    return 0;

}

Pokemon * create_pokemon() {

    PokemonInfo *pokemonInfo = allPokemonInfo[rand() % allPokemonInfo.size()];
    double distanceDouble = distance(current_tile_x, current_tile_y, WORLD_CENTER_X, WORLD_CENTER_Y);
    int distance = trunc(distanceDouble);
    int minLevel;
    int maxLevel;
    if (distance <= 200) {
        minLevel = 1;
        maxLevel = distance / 2;
        if (maxLevel < 1) {
            maxLevel = 1;
        }
    } else {
        minLevel = (distance - 200) / 2;
        if (minLevel < 1) {
            minLevel = 1;
        }
        maxLevel = 100;
    }
    int level;
    if (minLevel == maxLevel) {
        level = minLevel;
    } else {
        level = minLevel + rand() % (maxLevel - minLevel);
    }
    PokemonMove *pokemonMove = allPokemonMoves[0];
    int found = 0;
    while (found == 0) {
        pokemonMove = allPokemonMoves[rand() % allPokemonMoves.size()];
        if (pokemonMove->pokemon_id == pokemonInfo->species_id && pokemonMove->pokemon_move_method_id == 1) {
            found = 1;
        }
    }
    int base_health;
    int base_attack;
    int base_defense;
    int base_speed;
    int base_special_attack;
    int base_special_defense;
    for (int i = 0; i < (int) allPokemonStats.size(); i += 6) {
        if (allPokemonStats[i]->pokemon_id == pokemonInfo->id) {
            base_health = allPokemonStats[i]->base_stat;
            base_attack = allPokemonStats[i + 1]->base_stat;
            base_defense = allPokemonStats[i + 2]->base_stat;
            base_speed = allPokemonStats[i + 3]->base_stat;
            base_special_attack = allPokemonStats[i + 4]->base_stat;
            base_special_defense = allPokemonStats[i + 5]->base_stat;
            break;
        }
    }
    //todo: BUG: need to check if move level is not above pokemon level
    //todo: BUG: move2 is always null because legal Moves is always a single element. Maybe matching moves incorrectly?
        //todo: ^ match pokemon_moves rather than moves. Then find move in moves based off of accepted pokemon_moves.
    std::vector<Move *> legalMoves;
    for (int i = 0; i < (int) allMoves.size(); i++) {
        if (allMoves[i]->id == pokemonMove->move_id) {
            legalMoves.push_back(allMoves[i]);
        }
    }
    std::vector<Move *> moves;
    if (legalMoves.size() == 1) {
        moves.push_back(legalMoves[0]);
    }
    else {
        Move *move1 = legalMoves[rand() % legalMoves.size()];
        Move *move2 = legalMoves[rand() % legalMoves.size()];
        while (move2 == move1) {
            move2 = legalMoves[rand() % legalMoves.size()];
        }
        moves.push_back(move1);
        moves.push_back(move2);
    }
    bool male = rand() % 2;
    bool shiny = false;
    if (rand() % 8192 == 0) {
        shiny = true;
    }

    Pokemon *pokemon = new Pokemon(pokemonInfo, base_health, base_attack, base_defense, base_speed, base_special_attack,
                                   base_special_defense, level, moves, male, shiny);
    for (int i = 0; i < allPokemonTypes.size(); i++) {
        PokemonType *pokemonType = allPokemonTypes.at(i);
        if (pokemonType->pokemon_id == pokemon->pokemonInfo->id) {
            pokemon->typeIDs.push_back(pokemonType->type_id);
        }
    }
    return pokemon;

}

int combat_pokemon(Pokemon *wildPokemon) {

    //todo: BUG: don't allow to switch to already selected pokemon
    //todo: ASSIGNED: print message for EVERY return (ex. successful potion action doesn't print)
    //todo: ASSIGNED: if all pokemon knocked out before battle starts immediately lose
    //todo: ASSIGNED: if attempting to select pokemon and can't because knocked out, if has any revives first offer to use them before saying can't use pokemon
    //todo: ASSIGNED: use bag outside of battle

    bool victory = false;
    bool battleOver = false;
    int numRunAttempts = 0;
    Pokemon *selectedPokemon = player_character->activePokemon.at(0);
    while (!battleOver) {
        bool actionSelected = false;
        int moveIndex = -1;
        while (!actionSelected) {
            Pokemon *switchResult;
            int bagResult;
            int runResult;
            //moves index = moveInput - 1
            interface->clearUI();
            interface->addstrUI("You have found a wild ");
            interface->addstrUI(wildPokemon->pokemonInfo->name.c_str());
            interface->addstrUI("!\n");
            interface->addstrUI("Input a command: 'F' to fight; 'S' to switch pokemon; 'B' to open your bag; 'R' to run away");
            interface->refreshUI();
            const char input = interface->getchUI();
            switch (input) {
                case 'F':
                    moveIndex = fight_action(selectedPokemon);
                    if (moveIndex != -1) {
                        actionSelected = true;
                    }
                    break;
                case 'S':
                    switchResult = switch_pokemon_action(selectedPokemon);
                    if (switchResult != NULL) {
                        selectedPokemon = switchResult;
                        actionSelected = true;
                    }
                    break;
                case 'B':
                    bagResult = bag_action(true, selectedPokemon, wildPokemon);
                    if (bagResult == 0) {
                        actionSelected = true;
                    }
                    else if (bagResult == -1) {
                        actionSelected = true;
                        victory = true;
                        battleOver = true;
                    }
                    break;
                case 'R':
                    runResult = run_action(selectedPokemon, wildPokemon, numRunAttempts);
                    if (runResult == 0) {
                        actionSelected = true;
                        victory = false;
                        battleOver = true;
                    }
                    else if (runResult == 1) {
                        actionSelected = true;
                    }
                    break;
                default:
                    interface->clearUI();
                    interface->addstrUI(&input);
                    interface->addstrUI(" is not a valid command."
                                        "\n'F' to fight; 'S' to switch pokemon; 'B' to open your bag; 'R' to run away");
                    interface->refreshUI();
            }
        }
        if (!battleOver) {
            int wildPokemonMoveIndex = getWildPokemonMove(wildPokemon);
            doCombat(selectedPokemon, moveIndex, wildPokemon, wildPokemonMoveIndex);
            if (wildPokemon->knockedOut) {
                victory = true;
                battleOver = true;
            } else {
                bool noActivePokemonRemaining = true;
                for (int i = 0; i < player_character->activePokemon.size(); i++) {
                    if (!player_character->activePokemon.at(i)->knockedOut) {
                        noActivePokemonRemaining = false;
                        break;
                    }
                }
                if (noActivePokemonRemaining) {
                    victory = false;
                    battleOver = true;
                }
            }
        }
    }
    if (victory) {
        interface->clearUI();
        interface->addstrUI("Victory! You have defeated a wild ");
        interface->addstrUI(wildPokemon->pokemonInfo->name.c_str());
        interface->addstrUI("! Press esc to continue.");
        interface->refreshUI();
        while (interface->getchUI() != 27) {
            interface->clearUI();
            interface->addstrUI("Invalid input. You won the battle. Press esc to continue.");
            interface->refreshUI();
        }
    }
    else {
        interface->clearUI();
        interface->addstrUI("Defeat! You have been defeated by a wild ");
        interface->addstrUI(wildPokemon->pokemonInfo->name.c_str());
        interface->addstrUI("! Press esc to continue.");
        interface->refreshUI();
        while (interface->getchUI() != 27) {
            interface->clearUI();
            interface->addstrUI("Invalid input. You lost the battle. Press esc to continue.");
            interface->refreshUI();
        }
    }

    return 0;

}

int getWildPokemonMove(Pokemon *wildPokemon) {

    return rand() % wildPokemon->moves.size();

}

int doCombat(Pokemon *friendlyPokemon, int friendlyPokemonMoveIndex, Pokemon *wildPokemon, int wildPokemonMoveIndex) {

    //determine attack order
    bool friendlyPokemonFirst;
    bool bothAttack;
    if (friendlyPokemonMoveIndex < 0 && wildPokemonMoveIndex < 0) {
        return 0;
    }
    else if (friendlyPokemonMoveIndex < 0) {
        friendlyPokemonFirst = false;
        bothAttack = false;
    }
    else if (wildPokemonMoveIndex < 0) {
        friendlyPokemonFirst = true;
        bothAttack = false;
    }
    else {
        bothAttack = true;
        int friendlyPokemonPriority = friendlyPokemon->moves.at(friendlyPokemonMoveIndex)->priority;
        int wildPokemonPriority = wildPokemon->moves.at(wildPokemonMoveIndex)->priority;
        if (friendlyPokemonPriority > wildPokemonPriority) {
            friendlyPokemonFirst = true;
        } else if (wildPokemonPriority > friendlyPokemonPriority) {
            friendlyPokemonFirst = false;
        } else {
            int friendlyPokemonSpeed = friendlyPokemon->getSpeed();
            int wildPokemonSpeed = wildPokemon->getSpeed();
            if (friendlyPokemonSpeed > wildPokemonSpeed) {
                friendlyPokemonFirst = true;
            } else if (wildPokemonSpeed > friendlyPokemonSpeed) {
                friendlyPokemonFirst = false;
            } else {
                if (rand() % 2 == 0) {
                    friendlyPokemonFirst = true;
                } else {
                    friendlyPokemonFirst = false;
                }
            }
        }
    }

    //do faster attack
    if (friendlyPokemonFirst) {
        attack(friendlyPokemon, friendlyPokemonMoveIndex, wildPokemon);
    }
    else {
        attack(wildPokemon, wildPokemonMoveIndex, friendlyPokemon);
    }

    //if either pokemon is at 0 health, the second attacker is at 0 health, so we end the battle
    if (friendlyPokemon->health == 0 || wildPokemon->health == 0) {
        return 0;
    }
    //else do the second attack
    if (bothAttack) {
        if (!friendlyPokemonFirst) {
            attack(friendlyPokemon, friendlyPokemonMoveIndex, wildPokemon);
        } else {
            attack(wildPokemon, wildPokemonMoveIndex, friendlyPokemon);
        }
    }

    return 0;

}

int attack(Pokemon *attackingPokemon, int moveIndex, Pokemon *defendingPokemon) {

    Move *move = attackingPokemon->moves.at(moveIndex);

    //determine if hits or evaded
    bool hit = rand() % 100 < move->accuracy;

    int line = 0;
    interface->clearUI();
    interface->mvaddstrUI(line, 0, attackingPokemon->pokemonInfo->name.c_str());
    interface->addstrUI(" used ");
    interface->addstrUI(move->name.c_str());
    interface->addstrUI("!");
    line++;
    if (hit) {
        //do damage
        int critical = 1;
        if (rand() % 256 < attackingPokemon->speed_iv / 2) {
            critical = 1.5;
        }
        int stab = 1;
        bool typeMatch = false;
        for (int i = 0; i < attackingPokemon->typeIDs.size(); i++) {
            if (attackingPokemon->typeIDs.at(i) == move->type_id) {
                typeMatch = true;
                break;
            }
        }
        if (typeMatch) {
            stab = 1.5;
        }
        double type = 1;
        //todo: BUG: if move->power == -1 (not provided) set damageDouble to 0
        double damageDouble = (((2.0 * attackingPokemon->level / 5 + 2)
                * move->power * attackingPokemon->getAttack() / defendingPokemon->getDefense()) / 50 + 2)
                * critical * (rand() % 16 + 85) / 100 * stab * type;
        int damage = (int) damageDouble;
        int damageDealt = defendingPokemon->takeDamage(damage);

        //print hit message (different if critical hit or results in knock out)
        if (type != 1) {
            interface->mvaddstrUI(line, 0, "It was ");
            if (type == 0) {
                interface->addstrUI("incredibly ");
            }
            else if (type == 0.25 || type == 4) {
                interface->addstrUI("super ");
            }
            if (type < 1) {
                interface->addstrUI("ineffective");
            }
            else {
                //type > 1
                interface->addstrUI("effective");
            }
            interface->addstrUI("!");
            line++;
        }
        interface->mvaddstrUI(line, 0, defendingPokemon->pokemonInfo->name.c_str());
        interface->addstrUI(" took ");
        interface->addstrUI(std::to_string(damage).c_str());
        interface->addstrUI(" damage!");
        line++;
    }
    else {
        //print no hit message
        interface->mvaddstrUI(line, 0, "It missed!");
        line++;
    }
    interface->mvaddstrUI(line, 0, defendingPokemon->pokemonInfo->name.c_str());
    interface->addstrUI(" health: ");
    interface->addstrUI(std::to_string(defendingPokemon->getHealth()).c_str());
    interface->addstrUI(" / ");
    interface->addstrUI(std::to_string(defendingPokemon->maxHealth).c_str());
    line++;
    if (defendingPokemon->knockedOut) {
        interface->mvaddstrUI(line, 0, defendingPokemon->pokemonInfo->name.c_str());
        interface->addstrUI(" has fainted!");
    }
    interface->refreshUI();
    battlePause();

    //todo: BUG: attacker does way too much damage (up to over 500 for a lvl 1 pokemon with 11 - 13 hp)

    return 0;

}

int battlePause() {

    //todo: ASSIGNED: do esc for message rather than forced pause

    std::chrono::seconds dura(5);
    std::this_thread::sleep_for(dura);

    return 0;

}

/**
 * Shows available moves for the selected pokemon and asks the user to chose one of the moves.
 * @param selectedPokemon: selected pokemon
 * @return the chosen move number (ex. 1 for move 1) or 0 to go back
 */
int fight_action(Pokemon *selectedPokemon) {

    //shows pokemon moves
    int line = 0;
    interface->clearUI();
    interface->mvaddstrUI(line, 0,"Select a move by inputting the number corresponding to the move or press esc to go back.");
    line++;
    for (int i = 0; i < selectedPokemon->moves.size(); i++) {
        interface->mvaddstrUI(line, 0, "Move ");
        interface->addstrUI(std::to_string(i+1).c_str());
        interface->addstrUI(": ");
        interface->addstrUI(selectedPokemon->moves.at(i)->name.c_str());
        line++;
    }
    interface->refreshUI();

    //chose a move
    while (true) {
        const char input = interface->getchUI();
        if(selectedPokemon->knockedOut && input != 27) {
            line = 0;
            interface->clearUI();
            interface->mvaddstrUI(line, 0, &input);
            interface->addstrUI("The active pokemon has fainted and cannot make any moves. Press esc to go back.");
            line++;
            for (int i = 0; i < selectedPokemon->moves.size(); i++) {
                interface->mvaddstrUI(line, 0, "Move ");
                interface->addstrUI(std::to_string(i+1).c_str());
                interface->addstrUI(": ");
                interface->addstrUI(selectedPokemon->moves.at(i)->name.c_str());
                line++;
            }
            interface->refreshUI();
        }
        else {
            int inputInt = input - '0';
            if (inputInt > 0 && inputInt <= selectedPokemon->moves.size()) {
                return inputInt - 1;
            } else if (input == 27) {
                return -1;
            } else {
                line = 0;
                interface->clearUI();
                interface->mvaddstrUI(line, 0, &input);
                interface->addstrUI(" is not a valid input. Press a number corresponding to a move or esc to go back.");
                line++;
                for (int i = 0; i < selectedPokemon->moves.size(); i++) {
                    interface->mvaddstrUI(line, 0, "Move ");
                    interface->addstrUI(std::to_string(i + 1).c_str());
                    interface->addstrUI(": ");
                    interface->addstrUI(selectedPokemon->moves.at(i)->name.c_str());
                    line++;
                }
                interface->refreshUI();
            }
        }
    }

}

Pokemon *switch_pokemon_action(Pokemon *selectedPokemon) {

    //todo: BUG: don't allow switch to already selected pokemon
    //todo: ASSIGNED: when recalling a pokemon, when knocked out, or when battle ends, lose all status effects and conditions
    //shows pokemon choices
    int line = 0;
    interface->clearUI();
    interface->mvaddstrUI(line, 0, "Select a pokemon by inputting the corresponding number or press esc to go back.");
    line++;
    for (int i = 0; i < player_character->activePokemon.size(); i++) {
        interface->mvaddstrUI(line, 0, std::to_string(line).c_str());
        interface->addstrUI(". ");
        interface->addstrUI(player_character->activePokemon.at(i)->pokemonInfo->name.c_str());
        interface->addstrUI(" ");
        interface->addstrUI(std::to_string(player_character->activePokemon.at(i)->getHealth()).c_str());
        interface->addstrUI("/");
        interface->addstrUI(std::to_string(player_character->activePokemon.at(i)->maxHealth).c_str());
        interface->addstrUI(" ");
        interface->addstrUI(" HP");
        line++;
    }
    interface->refreshUI();

    //user's choice
    while (true) {
        const char input = interface->getchUI();
        int inputInt = input - '0';
        if (inputInt > 0 && inputInt <= player_character->activePokemon.size()) {
            Pokemon * pokemon = player_character->activePokemon.at( inputInt- 1);
            if (pokemon->knockedOut == false) {
                return pokemon;
            }
            else {
                int line = 0;
                interface->clearUI();
                interface->mvaddstrUI(line, 0, "That pokemon has fainted. Please input a number corresponding to a pokemon or esc to go back.");
                line++;
                for (int i = 0; i < player_character->activePokemon.size(); i++) {
                    interface->mvaddstrUI(line, 0, std::to_string(line).c_str());
                    interface->addstrUI(". ");
                    interface->addstrUI(player_character->activePokemon.at(i)->pokemonInfo->name.c_str());
                    interface->addstrUI(" ");
                    interface->addstrUI(std::to_string(player_character->activePokemon.at(i)->getHealth()).c_str());
                    interface->addstrUI(" HP");
                    line++;
                }
                interface->refreshUI();
            }
        }
        else if (input == 27) {
            return NULL;
        }
        else {
            int line = 0;
            interface->clearUI();
            interface->mvaddstrUI(line, 0, "That is not a valid input. Please input a number corresponding to a pokemon or esc to go back.");
            line++;
            for (int i = 0; i < player_character->activePokemon.size(); i++) {
                interface->mvaddstrUI(line, 0, std::to_string(line).c_str());
                interface->addstrUI(". ");
                interface->addstrUI(player_character->activePokemon.at(i)->pokemonInfo->name.c_str());
                line++;
            }
            interface->refreshUI();
        }
    }

}

int bag_action(bool wildPokemonBattle, Pokemon *selectedPokemon, Pokemon *enemyPokemon) {

    //show bag choices
    interface->clearUI();
    interface->addstrUI("Select a bag item by inputting the corresponding number or press esc to go back.");
    interface->mvaddstrUI(1, 0, "1. Potions: ");
    interface->addstrUI(std::to_string(player_character->bag->numPotions).c_str());
    interface->mvaddstrUI(2, 0, "2. Revives: ");
    interface->addstrUI(std::to_string(player_character->bag->numRevives).c_str());
    interface->mvaddstrUI(3, 0, "3. Pokeballs: ");
    interface->addstrUI(std::to_string(player_character->bag->numPokeballs).c_str());
    interface->refreshUI();

    //user's choice
    while (true) {
        const char input = interface->getchUI();
        std::string message;
        if (input == '1') {
            if (player_character->bag->usePotion(selectedPokemon) == 0) {
                return 0;
            }
            else {
                message = "You have 0 potions. Select a bag item by inputting the corresponding number or press esc to go back.";
            }
        }
        else if (input == '2') {
            int reviveUsage = player_character->bag->useRevive(selectedPokemon);
            if (reviveUsage == 0) {
                return 0;
            }
            else if (reviveUsage == 1) {
                message = "You have 0 revives. Select a bag item by inputting the corresponding number or press esc to go back.";
            }
            else {
                message = "The pokemon doesn't need a revive. Select a bag item by inputting the corresponding number or press esc to go back.";
            }
        }
        else if (input == '3') {
            int pokeballUsage = player_character->bag->usePokeball(player_character->activePokemon);
            if (pokeballUsage == 0) {
                if (wildPokemonBattle) {
                    return usePokeball(true, enemyPokemon);
                }
                else {
                    message = "You cannot use a pokeball during a trainer battle. Select a bag item by inputting the corresponding number or press esc to go back.";
                }
            }
            else if (pokeballUsage == 1) {
                message = "You have 0 pokeballs. Select a bag item by inputting the corresponding number or press esc to go back.";
            }
            else {
                usePokeball(false, enemyPokemon);
                return 0;
            }
        }
        else if (input == 27) {
            return 1;
        }
        else {
            message = "Invalid input. Select a bag item by inputting the corresponding number or press esc to go back.";
        }
        interface->clearUI();
        interface->addstrUI(message.c_str());
        interface->mvaddstrUI(1, 0, "1. Potions: ");
        interface->addstrUI(std::to_string(player_character->bag->numPotions).c_str());
        interface->mvaddstrUI(2, 0, "2. Revives: ");
        interface->addstrUI(std::to_string(player_character->bag->numRevives).c_str());
        interface->mvaddstrUI(3, 0, "3. Pokeballs: ");
        interface->addstrUI(std::to_string(player_character->bag->numPokeballs).c_str());
        interface->refreshUI();
    }

}

int usePokeball(bool success, Pokemon *targetPokemon) {

    player_character->bag->numPokeballs--;
    interface->clearUI();
    if (success) {
        player_character->activePokemon.push_back(targetPokemon);
        interface->addstrUI("You have captured ");
        interface->addstrUI(targetPokemon->pokemonInfo->name.c_str());
        interface->addstrUI("!");
        interface->refreshUI();
        battlePause();
        return -1;
    }
    else {
        interface->addstrUI("You have failed capture ");
        interface->addstrUI(targetPokemon->pokemonInfo->name.c_str());
        interface->addstrUI("!");
        interface->refreshUI();
        battlePause();

        return 0;
    }

}

int run_action(Pokemon *characterPokemon, Pokemon *wildPokemon, int numAttempts) {

    //confirm run away
    interface->clearUI();
    interface->addstrUI("Are you sure you wish to run away from this battle? (y/n)");
    interface->refreshUI();
    while (true) {
        char input = interface->getchUI();
        if (input == 'y') {
            break;
        } else if (input == 'n') {
            return -1;
        } else {
            interface->clearUI();
            interface->addstrUI("That is not a valid input. Do you wish to run away? (y/n)");
            interface->refreshUI();
        }
    }

    //calculate odds of escape
    int oddsEscape = (characterPokemon->getSpeed() * 32 / ((int)(wildPokemon->getSpeed()) % 256)) + 30 * numAttempts;
    bool succeeded = rand() % 256 < oddsEscape;

    //print status message
    if (succeeded) {
        interface->clearUI();
        interface->addstrUI("You successfully ran away!");
        interface->refreshUI();
        battlePause();
        return 0;
    }
    else {
        interface->clearUI();
        interface->addstrUI("You were too slow to run away! Maybe you'll be able to next time!");
        interface->refreshUI();
        battlePause();
        return 1;
    }

}

int enter_center() {

    player_character->in_building = 1;
    interface->clearUI();
    interface->addstrUI("You are in a pokecenter! Unfortunately this center is rather barren. Leave by entering \'<\'\n");
    interface->refreshUI();

    return 0;

}

int enter_mart() {

    player_character->in_building = 1;
    interface->clearUI();
    interface->addstrUI("You are in a pokemart! Unfortunately this mart is rather barren. Leave by entering \'<\'\n");
    interface->refreshUI();

    return 0;

}

int change_tile(int x, int y) {

    //todo: BUG: remove trainers and add new trainers with time = player time to heap upon changing tile. Code is currently commented in change_tile function.
    //todo: RUN BUG TEST: test moving onto new Tile with large game time for trainers time being updated correctly
    if (x >= 0 && x < WORLD_WIDTH_X && y >= 0 && y < WORLD_LENGTH_Y) {
        if (world[y][x] == NULL) {
            Tile *new_tile = new Tile();
            *new_tile = create_tile(x, y);
            world[y][x] = new_tile;
        }
        world[current_tile_y][current_tile_x]->player_character = NULL;
        current_tile_x = x;
        current_tile_y = y;
        Tile *new_tile = world[current_tile_y][current_tile_x];
        new_tile->player_character = (PlayerCharacter *) player_character;
        //set all characters in heap to same turn value as PC
        //todo: RUN BUG: moving between tiles sometimes crashes
        //todo: RUN BUG: upon re-entering ORIGINAL map (but not other maps) post leaving it, trainers never move
        //todo: RUN BUG TEST: uncomment below once heap crash insert bug fixed
//        if (new_tile->turn_heap->size > 0) {
//            NonPlayerCharacter *trainer = heap_remove_min(new_tile->turn_heap);
//            while (trainer->turn < player_character->turn) {
//                trainer->turn = player_character->turn;
//                heap_insert(new_tile->turn_heap, trainer);
//                trainer = heap_remove_min(new_tile->turn_heap);
//            }
//        }
        heap_insert(&turn_heap, player_character);
        return 0;
    }
    else {
        return 1;
    }

}

Tile create_tile(int x, int y) {

    Tile tile = create_empty_tile();
    tile.x = x;
    tile.y = y;
    generate_terrain(&tile);
    int north_x;
    if (y > 0 && world[y - 1][x] != NULL) {
        //north_x = world[y - 1][x]->south_x;
    }
    else {
        north_x = rand() % (TILE_WIDTH_X - 10) + 5;
    }
    int south_x;
    if (y < WORLD_LENGTH_Y - 1 && world[y + 1][x] != NULL) {
        //south_x = world[y + 1][x]->north_x;
    }
    else {
        south_x = rand() % (TILE_WIDTH_X - 10) + 5;
    }
    int east_y;
    if (x < WORLD_WIDTH_X - 1 && world[y][x + 1] != NULL) {
        //east_y = world[y][x + 1]->west_y;
    }
    else {
        east_y = rand() % (TILE_LENGTH_Y - 10) + 5;
    }
    int west_y;
    if (x > 0 && world[y][x - 1] != NULL) {
        //west_y = world[y][x - 1]->east_y;
    }
    else {
        west_y = rand() % (TILE_LENGTH_Y - 10) + 5;
    }
    generate_paths(&tile, north_x, south_x, east_y, west_y);
    generate_buildings(&tile, x, y);
    place_trainers(&tile);
    return tile;

}

Tile create_empty_tile() {

    Tile tile;
    Point *empty_point = new Point();
    for (int i = 0; i < TILE_LENGTH_Y; i++) {
        for (int j = 0; j < TILE_WIDTH_X; j++) {
            tile.tile[i][j] = *empty_point;
            tile.tile[i][j].x = j;
            tile.tile[i][j].y = i;
        }
    }
    tile.north_x = -1;
    tile.south_x = -1;
    tile.east_y = -1;
    tile.west_y = -1;
    return tile;

}

int generate_terrain(Tile *tile) {

    const int NUM_TALL_GRASS_SEEDS = rand() % 5 + 2;
    const int NUM_CLEARING_SEEDS = rand() % 5 + 2;
    const int NUM_FOREST_SEEDS = rand() % 5;
    const int NUM_MOUNTAIN_SEEDS = rand() % 4;
    const int NUM_LAKE_SEEDS = rand() % 3;
    plant_seeds(tile, *grass, NUM_TALL_GRASS_SEEDS);
    plant_seeds(tile, *clearing, NUM_CLEARING_SEEDS);
    plant_seeds(tile, *forest, NUM_FOREST_SEEDS);
    plant_seeds(tile, *mountain, NUM_MOUNTAIN_SEEDS);
    plant_seeds(tile, *lake, NUM_LAKE_SEEDS);
    grow_seeds(tile);
    place_edge(tile);
    set_terrain_border_weights(tile);

    return 0;

}

int plant_seeds(Tile *tile, Terrain terrain, int num_seeds) {

    for (int i = 0; i < num_seeds; i++) {
        int placed = 0;
        while (placed == 0) {
            int x = rand() % (TILE_WIDTH_X - 2) + 1;
            int y = rand() % (TILE_LENGTH_Y - 2) + 1;
            if (tile->tile[y][x].terrain.id == none->id) {
                tile->tile[y][x].terrain = terrain;
                placed = 1;
            }
        }
    }

    return 0;

}

int grow_seeds(Tile *tile) {

    //queue implementation:
    //add each (coordinate, Terrain) tuple structure to queue
    //while queue not empty
    //pop
    //if does not have Terrain
    //give Terrain as specified in tuple
    //give space weight value for dijkstra (as defined earlier)
    //add all spaces within 3x and 1y to queue with same Terrain

    //loop through non-edge to grow seeds
    int complete = 0;
    while (complete == 0) {
        //if no changes made in a loop then no more loops required
        complete = 1;
        //determine what must grow
        for (int i = 1; i < TILE_LENGTH_Y - 1; i++) {
            for (int j = 1; j < TILE_WIDTH_X - 1; j++) {
                if (tile->tile[i][j].terrain.id == none->id) {
                    //loop through nearby area to copy first Terrain found
                    for (int k = -1; k <=1; k++) {
                        for (int l = -1; l <= 1; l++) {
                            int x = j+k;
                            int y = i+l;
                            if (x > 0 && x < TILE_WIDTH_X - 1 && y > 0 && y < TILE_LENGTH_Y - 1) {
                                Terrain new_terrain = tile->tile[y][x].terrain;
                                if (new_terrain.id != none->id) {
                                    tile->tile[i][j].grow_into = new_terrain;
                                }
                            }
                        }
                    }
                    complete = 0;
                }
            }
        }
        //grow what must grow
        for (int i = 1; i < TILE_LENGTH_Y - 1; i++) {
            for (int j = 1; j < TILE_WIDTH_X - 1; j++) {
                Terrain new_terrain = tile->tile[i][j].grow_into;
                if (new_terrain.id != none->id) {
                    tile->tile[i][j].terrain = new_terrain;
                }
            }
        }
    }

    return 0;

}

int place_edge(Tile *tile) {

    //places edge (stones with different name and higher weight) on edges
    for (int i = 0; i < TILE_WIDTH_X; i ++) {
        tile->tile[0][i].terrain = *edge;
        tile->tile[TILE_LENGTH_Y - 1][i].terrain = *edge;
    }
    for (int i = 0; i < TILE_LENGTH_Y; i ++) {
        tile->tile[i][0].terrain = *edge;
        tile->tile[i][TILE_WIDTH_X - 1].terrain = *edge;
    }

    return 0;

}

int set_terrain_border_weights(Tile *tile) {

    //Sets borders between non-edge Terrain types to weight 0
    for (int i = 1; i < TILE_LENGTH_Y - 1; i++) {
        for (int j = 1; j < TILE_WIDTH_X - 1; j++) {
            Terrain terrain = tile->tile[i][j].terrain;
            for (int k = -1; k <=1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int x = j+k;
                    int y = i+l;
                    if (x > 0 && x < TILE_WIDTH_X - 1 && y > 0 && y < TILE_LENGTH_Y - 1) {
                        Terrain other_terrain = tile->tile[y][x].terrain;
                        if (terrain.id != other_terrain.id && other_terrain.id != edge->id) {
                            tile->tile[i][j].terrain.path_weight = TERRAIN_BORDER_WEIGHT;
                        }
                    }
                }
            }
        }
    }

    return 0;

}

int generate_paths(Tile *tile, int north_x, int south_x, int east_y, int west_y) {

    north_x = 39;
    south_x = 39;
    west_y = 10;
    east_y = 10;

    //used in both paths:
    int current_x;
    int current_y;
    //x = none; n = north; s = south; e = east; w = west
    char last_move;
    int moves_since_last_change;
    const int repetitive_limit = 3;

    //North/South path
    current_x = north_x;
    current_y = 0;
    last_move = 'x';
    moves_since_last_change = 0;
    tile->tile[current_y][current_x].terrain = *path;
    while (current_y < TILE_LENGTH_Y - 2) {
        //determine weights
        int east_weight = INT_MAX;
        int west_weight = INT_MAX;
        int south_weight = INT_MAX;
        if (current_x < TILE_WIDTH_X - 4 && last_move != 'w'
            && !(moves_since_last_change > repetitive_limit && last_move == 'e')) {
            east_weight = tile->tile[current_y][current_x + 1].terrain.path_weight;
        }
        if (current_x > 2 && last_move != 'e' && !(moves_since_last_change > repetitive_limit && last_move == 'w')) {
            west_weight = tile->tile[current_y][current_x - 1].terrain.path_weight;
        }
        if (current_y < TILE_LENGTH_Y - 1 && !(moves_since_last_change > repetitive_limit && last_move == 's')) {
            south_weight = tile->tile[current_y + 1][current_x].terrain.path_weight;
        }
        //choose the lowest weight
        if (east_weight < west_weight && east_weight < south_weight) {
            current_x++;
            tile->tile[current_y][current_x].terrain = *path;
            if (last_move == 'e') {
                moves_since_last_change++;
            }
            else {
                last_move = 'e';
                moves_since_last_change = 0;
            }
        }
        else if (west_weight < south_weight) {
            current_x--;
            tile->tile[current_y][current_x].terrain = *path;
            if (last_move == 'w') {
                moves_since_last_change++;
            }
            else {
                last_move = 'w';
                moves_since_last_change = 0;
            }
        }
        else {
            current_y++;
            tile->tile[current_y][current_x].terrain = *path;
            if (last_move == 's') {
                moves_since_last_change++;
            }
            else {
                last_move = 's';
                moves_since_last_change = 0;
            }
        }
    }
    if (current_x < south_x) {
        for (int i = current_x; i <= south_x; i++) {
            current_x = i;
            tile->tile[current_y][current_x].terrain = *path;
        }
    }
    else if (current_x > south_x) {
        for (int i = current_x; i >= south_x; i--) {
            current_x = i;
            tile->tile[current_y][current_x].terrain = *path;
        }
    }
    tile->tile[current_y + 1][current_x].terrain = *path;

    //West/East path
    current_x = 0;
    current_y = west_y;
    last_move = 'x';
    moves_since_last_change = 0;
    tile->tile[current_y][current_x].terrain = *path;
    while (current_x < TILE_WIDTH_X - 2) {
        //determine weights
        int north_weight = INT_MAX;
        int south_weight = INT_MAX;
        int east_weight = INT_MAX;
        if (current_y < TILE_LENGTH_Y - 3 && last_move != 'n'
            && !(moves_since_last_change > repetitive_limit && last_move == 's')) {
            south_weight = tile->tile[current_y + 1][current_x].terrain.path_weight;
        }
        if (current_y > 2 && last_move != 's' && !(moves_since_last_change > repetitive_limit && last_move == 'n')) {
            north_weight = tile->tile[current_y - 1][current_x].terrain.path_weight;
        }
        if (current_x < TILE_WIDTH_X - 2 && !(moves_since_last_change > repetitive_limit && last_move == 'e')) {
            east_weight = tile->tile[current_y][current_x + 1].terrain.path_weight;
        }
        //choose the lowest weight
        if (north_weight < south_weight && north_weight < east_weight) {
            current_y--;
            tile->tile[current_y][current_x].terrain = *path;
            if (last_move == 'n') {
                moves_since_last_change++;
            }
            else {
                last_move = 'n';
                moves_since_last_change = 0;
            }
        }
        else if (south_weight < east_weight) {
            current_y++;
            tile->tile[current_y][current_x].terrain = *path;
            if (last_move == 's') {
                moves_since_last_change++;
            }
            else {
                last_move = 's';
                moves_since_last_change = 0;
            }
        }
        else {
            current_x++;
            tile->tile[current_y][current_x].terrain = *path;
            if (last_move == 'e') {
                moves_since_last_change++;
            }
            else {
                last_move = 'e';
                moves_since_last_change = 0;
            }
        }
    }
    if (current_y < east_y) {
        for (int i = current_y; i <= east_y; i++) {
            current_y = i;
            tile->tile[current_y][current_x].terrain = *path;
        }
    }
    else if (current_y > east_y) {
        for (int i = current_y; i >= east_y; i--) {
            current_y = i;
            tile->tile[current_y][current_x].terrain = *path;
        }
    }
    tile->tile[current_y][current_x + 1].terrain = *path;

    tile->north_x = north_x;
    tile->south_x = south_x;
    tile->west_y = west_y;
    tile ->east_y = east_y;

    //uncomment below and modify it to match paths across tiles.
//    //used in both paths:
//    int current_x;
//    int current_y;
//    //x = none; n = north; s = south; e = east; w = west
//    char last_move;
//    int moves_since_last_change;
//    const int repetitive_limit = 3;
//
//    //North/South path
//    current_x = north_x;
//    current_y = 0;
//    last_move = 'x';
//    moves_since_last_change = 0;
//    Tile->Tile[current_y][current_x].Terrain = terrain_path;
//    while (current_y < TILE_LENGTH_Y - 2) {
//        //determine weights
//        int east_weight = INT_MAX;
//        int west_weight = INT_MAX;
//        int south_weight = INT_MAX;
//        if (current_x < TILE_WIDTH_X - 4 && last_move != 'w'
//            && !(moves_since_last_change > repetitive_limit && last_move == 'e')) {
//            east_weight = Tile->Tile[current_y][current_x + 1].weight;
//        }
//        if (current_x > 2 && last_move != 'e' && !(moves_since_last_change > repetitive_limit && last_move == 'w')) {
//            west_weight = Tile->Tile[current_y][current_x - 1].weight;
//        }
//        if (current_y < TILE_LENGTH_Y - 1 && !(moves_since_last_change > repetitive_limit && last_move == 's')) {
//            south_weight = Tile->Tile[current_y + 1][current_x].weight;
//        }
//        //choose lowest weight
//        if (east_weight < west_weight && east_weight < south_weight) {
//            current_x++;
//            Tile->Tile[current_y][current_x].Terrain = terrain_path;
//            if (last_move == 'e') {
//                moves_since_last_change++;
//            }
//            else {
//                last_move = 'e';
//                moves_since_last_change = 0;
//            }
//        }
//        else if (west_weight < south_weight) {
//            current_x--;
//            Tile->Tile[current_y][current_x].Terrain = terrain_path;
//            if (last_move == 'w') {
//                moves_since_last_change++;
//            }
//            else {
//                last_move = 'w';
//                moves_since_last_change = 0;
//            }
//        }
//        else {
//            current_y++;
//            Tile->Tile[current_y][current_x].Terrain = terrain_path;
//            if (last_move == 's') {
//                moves_since_last_change++;
//            }
//            else {
//                last_move = 's';
//                moves_since_last_change = 0;
//            }
//        }
//    }
//    Tile->Tile[current_y + 1][current_x].Terrain = terrain_path;
//
//    //West/East path
//    current_x = 0;
//    current_y = west_y;
//    last_move = 'x';
//    moves_since_last_change = 0;
//    Tile->Tile[current_y][current_x].Terrain = terrain_path;
//    while (current_x < TILE_WIDTH_X - 2) {
//        //determine weights
//        int north_weight = INT_MAX;
//        int south_weight = INT_MAX;
//        int east_weight = INT_MAX;
//        if (current_y < TILE_LENGTH_Y - 3 && last_move != 'n'
//            && !(moves_since_last_change > repetitive_limit && last_move == 's')) {
//            south_weight = Tile->Tile[current_y + 1][current_x].weight;
//        }
//        if (current_y > 2 && last_move != 's' && !(moves_since_last_change > repetitive_limit && last_move == 'n')) {
//            north_weight = Tile->Tile[current_y - 1][current_x].weight;
//        }
//        if (current_x < TILE_WIDTH_X - 2 && !(moves_since_last_change > repetitive_limit && last_move == 'e')) {
//            east_weight = Tile->Tile[current_y][current_x + 1].weight;
//        }
//        //choose lowest weight
//        if (north_weight < south_weight && north_weight < east_weight) {
//            current_y--;
//            Tile->Tile[current_y][current_x].Terrain = terrain_path;
//            if (last_move == 'n') {
//                moves_since_last_change++;
//            }
//            else {
//                last_move = 'n';
//                moves_since_last_change = 0;
//            }
//        }
//        else if (south_weight < east_weight) {
//            current_y++;
//            Tile->Tile[current_y][current_x].Terrain = terrain_path;
//            if (last_move == 's') {
//                moves_since_last_change++;
//            }
//            else {
//                last_move = 's';
//                moves_since_last_change = 0;
//            }
//        }
//        else {
//            current_x++;
//            Tile->Tile[current_y][current_x].Terrain = terrain_path;
//            if (last_move == 'e') {
//                moves_since_last_change++;
//            }
//            else {
//                last_move = 'e';
//                moves_since_last_change = 0;
//            }
//        }
//    }
//    Tile->Tile[current_y][current_x + 1].Terrain = terrain_path;
//
//    Tile->north_x = north_x;
//    Tile->south_x = south_x;
//    Tile->west_y = west_y;
//    Tile ->east_y = east_y;

    return 0;

}

int generate_buildings(Tile *tile, int x, int y) {

    double chance;
    if (x == WORLD_CENTER_X && y == WORLD_CENTER_Y) {
        chance = 100;
    }
    else {
        chance = ((-45 * distance(x, y, WORLD_CENTER_X, WORLD_CENTER_Y) / 200) + 50);
        if (chance < 5) {
            chance = 5;
        }
    }
    place_building(tile, *center, chance);
    place_building(tile, *mart, chance);

    return 0;

}

int place_building(Tile *tile, Terrain terrain, double chance) {

    if (rand() % 100 < chance) {
        int x;
        int y;
        int valid = 1;
        while (valid == 1) {
            x = rand() % (TILE_WIDTH_X - 2) + 1;
            y = rand() % (TILE_LENGTH_Y - 2) + 1;
            Point point = tile->tile[y][x];
            if (!legal_overwrite(point)) {
                if ((x > 0 && tile->tile[y][x - 1].terrain.id == path->id)
                    || (x < TILE_WIDTH_X - 1 && tile->tile[y][x + 1].terrain.id == path->id)
                    || (y > 0 && tile->tile[y - 1][x].terrain.id == path->id)
                    || (y < TILE_LENGTH_Y - 1 && tile->tile[y + 1][x].terrain.id == path->id)) {
                    valid = 0;
                }
            }
        }
        tile->tile[y][x].terrain = terrain;
    }

    return 0;

}

int place_player_character(Tile *tile) {

    int x;
    int y;
    int found = 0;
    while (found == 0) {
        x = rand() % 78 + 1;
        y = rand() % 19 + 1;
        if (tile->tile[y][x].terrain.id == path->id) {
            found = 1;
        }
    }

    player_character = new Character(x, y, PLAYER, "PLAYER", '@', COLOR_CYAN,
                                     0, 0, 0, 0, 0, 0);
    select_pokemon(player_character);
    tile->player_character = (PlayerCharacter *) player_character;
    tile->tile[y][x].character = player_character;
    heap_insert(&turn_heap, player_character);
    //create distance tiles
    dijkstra(tile, RIVAL);
    dijkstra(tile, HIKER);

    return 0;

}

int select_pokemon(Character *playerCharacter) {

    //create pokemon choices
    Pokemon *pokemon1 = create_pokemon();
    Pokemon *pokemon2 = create_pokemon();
    Pokemon *pokemon3 = create_pokemon();

    //present pokemon choices to player
    interface->clearUI();

    int lineNumber = 0;

    interface->addstrUI("Input 1/2/3 to choose the respective starting pokemon!");
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Option: ");
    interface->mvaddstrUI(lineNumber, 20, "1. ");
    interface->addstrUI(pokemon1->pokemonInfo->name.c_str());
    interface->mvaddstrUI(lineNumber, 40, "2. ");
    interface->addstrUI(pokemon2->pokemonInfo->name.c_str());
    interface->mvaddstrUI(lineNumber, 60, "3. ");
    interface->addstrUI(pokemon3->pokemonInfo->name.c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Level: ");
    interface->mvaddstrUI(lineNumber, 20, std::to_string(pokemon1->level).c_str());
    interface->mvaddstrUI(lineNumber, 40, std::to_string(pokemon2->level).c_str());
    interface->mvaddstrUI(lineNumber, 60, std::to_string(pokemon3->level).c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Move 1: ");
    interface->mvaddstrUI(lineNumber, 20, pokemon1->moves.at(0)->name.c_str());
    interface->mvaddstrUI(lineNumber, 40, pokemon2->moves.at(0)->name.c_str());
    interface->mvaddstrUI(lineNumber, 60, pokemon3->moves.at(0)->name.c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Move 2: ");
    if (pokemon1->moves.size() > 1) {
        interface->mvaddstrUI(lineNumber, 20, pokemon1->moves.at(1)->name.c_str());
    }
    else {
        interface->mvaddstrUI(lineNumber, 20, "none");
    }
    if (pokemon1->moves.size() > 1) {
        interface->mvaddstrUI(lineNumber, 40, pokemon2->moves.at(1)->name.c_str());
    }
    else {
        interface->mvaddstrUI(lineNumber, 40, "none");
    }
    if (pokemon1->moves.size() > 1) {
        interface->mvaddstrUI(lineNumber, 60, pokemon3->moves.at(1)->name.c_str());
    }
    else {
        interface->mvaddstrUI(lineNumber, 60, "none");
    }
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "HP: ");
    interface->mvaddstrUI(lineNumber, 20, std::to_string(pokemon1->getHealth()).c_str());
    interface->mvaddstrUI(lineNumber, 40, std::to_string(pokemon2->getHealth()).c_str());
    interface->mvaddstrUI(lineNumber, 60, std::to_string(pokemon3->getHealth()).c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Attack: ");
    interface->mvaddstrUI(lineNumber, 20, std::to_string(pokemon1->getAttack()).c_str());
    interface->mvaddstrUI(lineNumber, 40, std::to_string(pokemon2->getAttack()).c_str());
    interface->mvaddstrUI(lineNumber, 60, std::to_string(pokemon3->getAttack()).c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Defense: ");
    interface->mvaddstrUI(lineNumber, 20, std::to_string(pokemon1->getDefense()).c_str());
    interface->mvaddstrUI(lineNumber, 40, std::to_string(pokemon2->getDefense()).c_str());
    interface->mvaddstrUI(lineNumber, 60, std::to_string(pokemon3->getDefense()).c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Special Attack: ");
    interface->mvaddstrUI(lineNumber, 20, std::to_string(pokemon1->getSpecialAttack()).c_str());
    interface->mvaddstrUI(lineNumber, 40, std::to_string(pokemon2->getSpecialAttack()).c_str());
    interface->mvaddstrUI(lineNumber, 60, std::to_string(pokemon3->getSpecialAttack()).c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Special Defense: ");
    interface->mvaddstrUI(lineNumber, 20, std::to_string(pokemon1->getSpecialDefense()).c_str());
    interface->mvaddstrUI(lineNumber, 40, std::to_string(pokemon2->getSpecialDefense()).c_str());
    interface->mvaddstrUI(lineNumber, 60, std::to_string(pokemon3->getSpecialDefense()).c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Speed: ");
    interface->mvaddstrUI(lineNumber, 20, std::to_string(pokemon1->getSpeed()).c_str());
    interface->mvaddstrUI(lineNumber, 40, std::to_string(pokemon2->getSpeed()).c_str());
    interface->mvaddstrUI(lineNumber, 60, std::to_string(pokemon3->getSpeed()).c_str());
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Gender: ");
    if (pokemon1->male == true) {
        interface->mvaddstrUI(lineNumber, 20, "Male");
    }
    else {
        interface->mvaddstrUI(lineNumber, 20, "Female");
    }
    if (pokemon2->male == true) {
        interface->mvaddstrUI(lineNumber, 40, "Male");
    }
    else {
        interface->mvaddstrUI(lineNumber, 40, "Female");
    }
    if (pokemon3->male == true) {
        interface->mvaddstrUI(lineNumber, 60, "Male");
    }
    else {
        interface->mvaddstrUI(lineNumber, 60, "Female");
    }
    lineNumber++;

    interface->mvaddstrUI(lineNumber, 0, "Shiny: ");
    if (pokemon1->shiny == true) {
        interface->mvaddstrUI(lineNumber, 20, "Yes");
    }
    else {
        interface->mvaddstrUI(lineNumber, 20, "No");
    }
    if (pokemon2->shiny == true) {
        interface->mvaddstrUI(lineNumber, 40, "Yes");
    }
    else {
        interface->mvaddstrUI(lineNumber, 40, "No");
    }
    if (pokemon3->shiny == true) {
        interface->mvaddstrUI(lineNumber, 60, "Yes");
    }
    else {
        interface->mvaddstrUI(lineNumber, 60, "No");
    }

    interface->refreshUI();

    //player chooses pokemon
    Pokemon *chosenPokemon;
    while (true) {
        const char choice = interface->getchUI();
        if (choice == '1') {
            chosenPokemon = pokemon1;
            break;
        }
        else if (choice == '2') {
            chosenPokemon = pokemon2;
            break;
        }
        else if (choice == '3') {
            chosenPokemon = pokemon3;
            break;
        }
        else {
            interface->mvaddstrUI(0, 0, &choice);
            interface->addstrUI(" is not a valid input. Please input 1/2/3 to choose the respective starting pokemon!");
        }
    }
    playerCharacter->activePokemon.push_back(chosenPokemon);

    return 0;

}

int place_trainers(Tile *tile) {

    //todo: RUN BUG: trainers placed illegally (ex. rivals in mountains)
    int num_trainers_copy = num_trainers;
    int num_rivals = 0;
    int num_hikers = 0;
    int num_random_walkers = 0;
    int num_pacers = 0;
    int num_wanderers = 0;
    int num_stationaries = 0;
    while (num_trainers_copy > 0) {
        if (num_rivals == 0) {
            num_rivals++;
        }
        else if (num_hikers == 0) {
            num_hikers++;
        }
        else {
            int random = rand()%10;
            if (random >= 0 && random <= 2) {
                num_rivals++;
            }
            else if (random >= 3 && random <= 5) {
                num_hikers++;
            }
            else if (random == 6) {
                num_random_walkers++;
            }
            else if (random == 7) {
                num_pacers++;
            }
            else if (random == 8) {
                num_wanderers++;
            }
            else if (random == 9) {
                num_stationaries++;
            }
        }
        num_trainers_copy--;
    }

    place_trainer_type(tile, num_rivals, RIVAL, 'r');
    place_trainer_type(tile, num_hikers, HIKER, 'h');
    place_trainer_type(tile, num_random_walkers, RANDOM_WALKER, 'n');
    place_trainer_type(tile, num_pacers, PACER, 'p');
    place_trainer_type(tile, num_wanderers, WANDERER, 'w');
    place_trainer_type(tile, num_stationaries, STATIONARY, 's');

    return 0;

}

int place_trainer_type(Tile *tile, int num_trainer, enum character_type trainer_type, char character) {

    while (num_trainer > 0) {
        int x;
        int y;
        int found = 0;
        while (found == 0) {
            x = rand() % 78 + 1;
            y = rand() % 19 + 1;
            if (tile->tile[y][x].character == NULL) {
                if (trainer_type == HIKER) {
                    //spawns anywhere hiker can reach PC from
                    if (hiker_distance_tile[y][x] < INT_MAX) {
                        found = 1;
                    }
                }
                else {
                    //spawns anywhere rival can reach PC from
                    if (rival_distance_tile[y][x] < INT_MAX) {
                        found = 1;
                    }
                }
            }
        }
        std::string type_string;
        //initialize type_string
        if (trainer_type == RIVAL) {
            type_string = "RIVAL";
        }
        else if (trainer_type == HIKER) {
            type_string = "HIKER";
        }
        else if (trainer_type == RANDOM_WALKER) {
            type_string = "RANDOM WALKER";
        }
        else if (trainer_type == PACER) {
            type_string = "PACER";
        }
        else if (trainer_type == WANDERER) {
            type_string = "WANDERER";
        }
        else if (trainer_type == STATIONARY) {
            type_string = "STATIONARY";
        }
        else {
            //trainer is not one of the trainer types
            return 1;
        }
        Character *trainer = new Character(x, y, trainer_type, type_string, character,
                                           COLOR_RED, 0, 0, 0, 0,
                                           0, 0);
        trainer->activePokemon.push_back(create_pokemon());
        //60% chance for trainer to get another pokemon if just got a pokemon, up to 6
        for (int i = 0; i < 5; i++) {
            if (rand() % 10 < 6) {
                trainer->activePokemon.push_back(create_pokemon());
            }
            else {
                break;
            }
        }
        heap_insert(&turn_heap, trainer);
        tile->tile[y][x].character = trainer;
        num_trainer--;
    }

    return 0;

}

int dijkstra(Tile *tile, enum character_type trainer_type) {

    for (int y = 0; y < TILE_LENGTH_Y; y++) {
        for (int x = 0; x < TILE_WIDTH_X; x++) {
            tile->tile[y][x].distance = INT_MAX;
        }
    }
    //updates appropriate trainer distance Tile for the data to endure through future dijkstra calls
    for (int i = 0; i < TILE_LENGTH_Y; i++) {
        for (int j = 0; j < TILE_WIDTH_X; j++) {
            if (trainer_type == RIVAL) {
                rival_distance_tile[i][j] = tile->tile[i][j].distance;
            }
            else {
                //printable_character type_enum = hiker
                hiker_distance_tile[i][j] = tile->tile[i][j].distance;
            }
        }
    }

    //todo: BUG: uncomment code below and debug. Currently replaced by all points distance set to int max = no trainer movement.
//    int start_x = tile->player_character->x;
//    int start_y =tile->player_character->y;
//
//    for (int y = 0; y < TILE_LENGTH_Y; y++) {
//        for (int x = 0; x < TILE_WIDTH_X; x++) {
//            tile->tile[y][x].distance = INT_MAX;
//        }
//    }
//    tile->tile[start_y][start_x].distance = 0;
//
//    struct heap heap;
//    static Point *point;
//    heap_init(&heap, comparator_trainer_distance_tile, NULL);
//    for (int y = 0; y < TILE_LENGTH_Y; y++) {
//        for (int x = 0; x < TILE_WIDTH_X; x++) {
//            int weight;
//            if (trainer_type == RIVAL) {
//                weight = tile->tile[y][x].terrain.rival_weight;
//            }
//            else {
//                //character_type type_enum == hiker
//                weight = tile->tile[y][x].terrain.hiker_weight;
//            }
//            if (weight != INT_MAX) {
//                tile->tile[y][x].heap_node = heap_insert(&heap, &(tile->tile[y][x]));
//            }
//            else {
//                tile->tile[y][x].heap_node = NULL;
//            }
//        }
//    }
//    //todo: BUG: crashes on heap remove min in heap restructuring after some number of iterations. Usually when crashing heap size is around 800 to 1100
//    while ((point = (Point *) heap_remove_min(&heap))) {
//        point->heap_node = NULL;
//        for (int y = -1; y <= 1; y++) {
//            for (int x = -1; x <= 1; x++) {
//                if (point->y + y >= 0 && point->y + y < TILE_LENGTH_Y && point->x + x >= 0 && point->x + x < TILE_WIDTH_X)
//                {
//                    Point *neighbor = &(tile->tile[point->y + y][point->x + x]);
//                    int candidate_distance;
//                    if (trainer_type == RIVAL) {
//                        candidate_distance = point->distance + neighbor->terrain.rival_weight;
//                    } else {
//                        //character_type type_enum == hiker
//                        candidate_distance = point->distance + neighbor->terrain.hiker_weight;
//                    }
//                    if (neighbor->heap_node != NULL && candidate_distance < neighbor->distance &&
//                        candidate_distance > 0) {
//                        neighbor->distance = candidate_distance;
//                        heap_decrease_key_no_replace(&heap, neighbor->heap_node);
//                    }
//                }
//            }
//        }
//    }
//    heap_delete(&heap);
//
//    //updates appropriate trainer distance Tile for the data to endure through future dijkstra calls
//    for (int i = 0; i < TILE_LENGTH_Y; i++) {
//        for (int j = 0; j < TILE_WIDTH_X; j++) {
//            if (trainer_type == RIVAL) {
//                rival_distance_tile[i][j] = tile->tile[i][j].distance;
//            }
//            else {
//                //printable_character type_enum = hiker
//                hiker_distance_tile[i][j] = tile->tile[i][j].distance;
//            }
//        }
//    }

    return 0;

}

int legal_overwrite(Point point) {

    if (point.terrain.id == edge->id
        || point.terrain.id == path->id
        || point.terrain.id == center->id
        || point.terrain.id == mart->id) {
        return 1;
    }
    else {
        return 0;
    }

}

double distance(int x1, int y1, int x2, int y2) {
    double square_difference_x = (x2 - x1) * (x2 - x1);
    double square_difference_y = (y2 - y1) * (y2 - y1);
    double sum = square_difference_x + square_difference_y;
    double value = sqrt(sum);
    return value;
}

int print_tile_terrain(Tile *tile) {

    for (int y = 0; y < TILE_LENGTH_Y; y++) {
        for (int x = 0; x < TILE_WIDTH_X; x++) {
            char printable_character = tile->tile[y][x].terrain.printable_character;
            if (tile->tile[y][x].character != NULL) {
                printable_character = tile->tile[y][x].character->printable_character;
                //todo: RUN BUG: color isn't showing. Currently testing preset color rather than obtained from data.
                interface->init_pairUI(1, COLOR_RED, COLOR_BLACK);
                //interface->init_pairUI(1, Tile->Tile[y][x].Character->color, COLOR_BLACK);
                interface->attrsetUI(COLOR_PAIR(1));
                interface->mvaddchUI(y + 1, x, printable_character);
                interface->refreshUI();
                interface->attroffUI(COLOR_PAIR(1));
            }
            else {
                interface->init_pairUI(1, tile->tile[y][x].terrain.color, COLOR_BLACK);
                interface->attrsetUI(COLOR_PAIR(1));
                interface->mvaddchUI(y + 1, x, printable_character);
                interface->refreshUI();
                interface->attroffUI(COLOR_PAIR(1));
            }
        }
    }
    interface->addchUI('\n');
    interface->refreshUI();

    return 0;

}

int print_tile_trainer_distances(Tile *tile) {

    dijkstra(tile, RIVAL);
    printf("Rival distance Tile:\n");
    print_tile_trainer_distances_printer(tile);
    dijkstra(tile, HIKER);
    printf("Hiker distance Tile:\n");
    print_tile_trainer_distances_printer(tile);

    return 0;

}

int print_tile_trainer_distances_printer(Tile *tile) {

    for (int i = 0; i < TILE_LENGTH_Y; i++) {
        for (int j = 0; j < TILE_WIDTH_X; j++) {
            int distance = tile->tile[i][j].distance;
            if (distance == INT_MAX) {
                printf("  ");
            }
            else if (distance == 0) {
                printf("\033[31m");
                printf("00");
                printf("\033[0m");
            }
            else {
                printf("%02d", tile->tile[i][j].distance % 100);
            }
            printf(" ");
        }
        printf("\n");
    }

    return 0;

}
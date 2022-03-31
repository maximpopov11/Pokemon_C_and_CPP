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

    Character(int x, int y, enum  character_type type_enum, std::string type_string, char printable_character, short color,
            int turn, int direction_set, int x_direction, int y_direction, int in_building, int defeated) : x(x), y(y),
            type_enum(type_enum), type_string(type_string), printable_character(printable_character), color(color),
            turn(turn), direction_set(direction_set), x_direction(x_direction), y_direction(y_direction),
            in_building(in_building), defeated(defeated) {}
};

class PlayerCharacter : public Character {

};

class NonPlayerCharacter : public Character {

};

//todo: ASSIGNED: constructor for point and tile

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
    struct heap *turn_heap;

    Tile() {
        this->x = -1;
        this->y = -1;
        this->north_x = -1;
        this->south_x = -1;
        this->east_y = -1;
        this->west_y = -1;
        this->player_character = NULL;
        this->turn_heap = NULL;
    }
};

//commented due to failing make
//class DatabaseInfo {
//public:
//    virtual std::string toString();
//};

//commented due to DatabaseInfo failing to make
//class Pokemon : public DatabaseInfo {
class Pokemon {
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

    Pokemon(std::string id, std::string name, std::string species_id, std::string height, std::string weight,
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

int rival_distance_tile[TILE_LENGTH_Y][TILE_WIDTH_X];
int hiker_distance_tile [TILE_LENGTH_Y][TILE_WIDTH_X];

static int32_t comparator_trainer_distance_tile(const void *key, const void *with) {
    return ((Point *) key)->distance - ((Point *) with)->distance;
}

static int32_t comparator_character_movement(const void *key, const void *with) {
    return ((Character *) key)->turn - ((Character *) with)->turn;
}

int print_usage();
int storePokemon();
int storeMoves();
//commented due to database info failing to make
//int printData(std::vector<DatabaseInfo *> dataVector);
int initialize_terminal();
int turn_based_movement();
int player_turn();
int move_character(int x, int y, int new_x, int new_y);
int combat(Character *from_character, Character *to_character);
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
int place_trainers(Tile *tile);
int place_trainer_type(Tile *tile, int num_trainer, enum character_type trainer_type, char character);
int dijkstra(Tile *tile, enum character_type trainer_type);
int legal_overwrite(Point point);
double distance(int x1, int y1, int x2, int y2);
int print_tile_terrain(Tile *tile);
int print_tile_trainer_distances(Tile *tile);
int print_tile_trainer_distances_printer(Tile *tile);

std::vector<Pokemon *> allPokemon;
std::vector<Move *> allMoves;
Tile *world[WORLD_LENGTH_Y][WORLD_WIDTH_X] = {0};
int current_tile_x;
int current_tile_y;
Character *player_character;
int num_trainers;

int main(int argc, char *argv[]) {

    //get arguments
//    int opt = 0;
//    int numtrainers = 10;
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

    //todo: ASSIGNED: store and print all files like Pokemon
    //todo: ASSIGNED: don't print -1 empty placeholder
    //todo: ASSIGNED: read from other places first
    if (argc < 2) {
        std::cout << "No arguments provided." << "\n";
        return 1;
    }
    std::string fileName = argv[1];
    if (fileName == "pokemon") {
        for (int i = 0; i < (int)allPokemon.size(); i++) {
            std::cout << allPokemon[i]->toString() << "\n";
        }
    }
    else if (fileName == "moves") {
        for (int i = 0; i < (int)allMoves.size(); i++) {
            std::cout << allMoves[i]->toString() << "\n";
        }
    }
    else if (fileName == "pokemon_moves") {
        std::cout << "Opening pokemon_moves.csv..." << "\n";
    }
    else if (fileName == "pokemon_species") {
        std::cout << "Opening pokemon_species.csv..." << "\n";
    }
    else if (fileName == "experience") {
        std::cout << "Opening experience.csv..." << "\n";
    }
    else if (fileName == "type_names") {
        std::cout << "Opening type_names.csv..." << "\n";
    }
    else {
        std::cout << "Input file name: " << fileName << " is not a valid file" << "\n";
        return 2;
    }


    //check argument legality
//    if (numtrainers < 0) {
//        numtrainers = 0;
//    }
//    else if (numtrainers > MAX_NUM_TRAINERS) {
//        numtrainers = MAX_NUM_TRAINERS;
//    }
//    num_trainers = numtrainers;

    //run program
//    srand(time(NULL));
//    initialize_terminal();
//    Tile home_tile = create_tile(WORLD_CENTER_X, WORLD_CENTER_Y);
//    current_tile_x = WORLD_CENTER_X;
//    current_tile_y = WORLD_CENTER_Y;
//    world[WORLD_CENTER_Y][WORLD_CENTER_X] = &home_tile;
//    place_player_character(world[current_tile_y][current_tile_x]);
//    while (turn_based_movement() == -1) {
//        //-1 signals map was changed: call turn_based_movement for new map/turn heap
//        //old and new Tile and heap have been updated correctly in change Tile (removed from old heap in turn_based_movement)
//    }
//    endwin();
    return 0;

}

int print_usage() {

    //print expected inputs

    return 0;

}

int storePokemon() {

    std::ifstream file;
    file.open("pokedex/pokedex/data/csv/pokemon.csv");
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
            Pokemon *pokemon = new Pokemon(id, name, species_id, height, weight, base_experience, order, is_default);
            allPokemon.push_back(pokemon);
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
    file.open("pokedex/pokedex/data/csv/moves.csv");
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

//Commented due to DatabaseInfo failing to make
//Print any data type to follow DRY principle
//int printData(std::vector<DatabaseInfo *> dataVector) {
//
//    for (int i = 0; i < (int)allPokemon.size(); i++) {
//        std::cout << dataVector[i]->toString() << "\n";
//    }
//
//    return 0;
//
//}

int initialize_terminal() {

    initscr();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    return 0;

}

int turn_based_movement() {

    Tile *tile = world[current_tile_y][current_tile_x];
    struct heap *turn_heap = tile->turn_heap;
    static Character *character;
    while ((character = (Character *) (heap_remove_min(turn_heap)))) {
        if (character->type_enum == PLAYER) {
            clear();
            addstr("It's your turn! Enter a command or press z for help!\n");
            print_tile_terrain(tile);
            refresh();
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
        heap_insert(turn_heap, character);
    }
    heap_delete(turn_heap);

    return 0;

}

int player_turn() {

    Tile *tile = world[current_tile_y][current_tile_x];
    int turn_completed = 0;
    int in_help = 0;
    int x = player_character->x;
    int y = player_character->y;
    while (turn_completed == 0) {
        int input = getch();
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
                clear();
                addstr("There is no pokecenter or pokemart here so you can't enter one!\n");
                print_tile_terrain(tile);
            }
        } else if (input == '<') {
            if (player_character->in_building == 1) {
                clear();
                addstr("You have left the building!\n");
                print_tile_terrain(tile);
            } else {
                clear();
                addstr("You aren't in a building so you can't leave one!\n");
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
            clear();
            addstr("Trainer list: Press escape to return to the map\n");
            for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                NonPlayerCharacter *trainer = trainers[i];
                mvaddstr(screen_row, type_x, trainer->type_string.c_str());
                mvaddstr(screen_row, position_x, " ");
                if (trainer->y != player_character->y) {
                    char y_distance[3];
                    if (trainer->y < player_character->y) {
                        sprintf(y_distance, "%d ", player_character->y - trainer->y);
                        addstr(y_distance);
                        addstr("North ");
                    }
                    else {
                        sprintf(y_distance, "%d ", trainer->y - player_character->y);
                        addstr(y_distance);
                        addstr("South ");
                    }
                }
                if (trainer->x != player_character->x) {
                    char x_distance[3];
                    if (trainer->x < player_character->x) {
                        sprintf(x_distance, "%d ", player_character->x - trainer->x);
                        addstr(x_distance);
                        addstr("West");
                    }
                    else {
                        sprintf(x_distance, "%d ", trainer->x - player_character->x);
                        addstr(x_distance);
                        addstr("East");
                    }
                }
                if (trainer->defeated == 1) {
                    mvaddstr(screen_row, defeated_status_x, "Defeated");
                }
                addstr("\n");
                screen_row++;
            }
            refresh();
            chtype command = -1;
            while (command != 27 && command != ACS_UARROW && command != ACS_DARROW) {
                command = getch();
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
                        clear();
                        addstr("Trainer list: Press escape to return to the map\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            mvaddstr(screen_row, type_x, trainer->type_string.c_str());
                            mvaddstr(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    addstr(y_distance);
                                    addstr("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    addstr(y_distance);
                                    addstr("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    addstr(x_distance);
                                    addstr("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    addstr(x_distance);
                                    addstr("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                mvaddstr(screen_row, defeated_status_x, "Defeated");
                            }
                            addstr("\n");
                            screen_row++;
                        }
                        refresh();
                    }
                    else {
                        clear();
                        addstr("You are already at the top of the list so you cannot scroll up.\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            mvaddstr(screen_row, type_x, trainer->type_string.c_str());
                            mvaddstr(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    addstr(y_distance);
                                    addstr("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    addstr(y_distance);
                                    addstr("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    addstr(x_distance);
                                    addstr("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    addstr(x_distance);
                                    addstr("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                mvaddstr(screen_row, defeated_status_x, "Defeated");
                            }
                            addstr("\n");
                            screen_row++;
                        }
                        refresh();
                    }
                }
                else if (command == KEY_DOWN) {
                    if (position < num_trainers - SCREEN_HEIGHT + 1) {
                        position += SCREEN_HEIGHT - 1;
                        clear();
                        addstr("Trainer list: Press escape to return to the map\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            mvaddstr(screen_row, type_x, trainer->type_string.c_str());
                            mvaddstr(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    addstr(y_distance);
                                    addstr("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    addstr(y_distance);
                                    addstr("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    addstr(x_distance);
                                    addstr("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    addstr(x_distance);
                                    addstr("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                mvaddstr(screen_row, defeated_status_x, "Defeated");
                            }
                            addstr("\n");
                            screen_row++;
                        }
                        refresh();
                    }
                    else {
                        clear();
                        addstr("You are already at the bottom of the list so you cannot scroll down.\n");
                        for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                            NonPlayerCharacter *trainer = trainers[i];
                            mvaddstr(screen_row, type_x, trainer->type_string.c_str());
                            mvaddstr(screen_row, position_x, " ");
                            if (trainer->y != player_character->y) {
                                char y_distance[3];
                                if (trainer->y < player_character->y) {
                                    sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                    addstr(y_distance);
                                    addstr("North ");
                                }
                                else {
                                    sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                    addstr(y_distance);
                                    addstr("South ");
                                }
                            }
                            if (trainer->x != player_character->x) {
                                char x_distance[3];
                                if (trainer->x < player_character->x) {
                                    sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                    addstr(x_distance);
                                    addstr("West");
                                }
                                else {
                                    sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                    addstr(x_distance);
                                    addstr("East");
                                }
                            }
                            if (trainer->defeated == 1) {
                                mvaddstr(screen_row, defeated_status_x, "Defeated");
                            }
                            addstr("\n");
                            screen_row++;
                        }
                        refresh();
                    }
                }
                else {
                    //command is invalid
                    clear();
                    addstr("That is not a valid command! Press escape to return to the map.\n");
                    for (int i = position; i < position + SCREEN_HEIGHT - 1 && i < num_trainers; i++) {
                        NonPlayerCharacter *trainer = trainers[i];
                        mvaddstr(screen_row, type_x, trainer->type_string.c_str());
                        mvaddstr(screen_row, position_x, " ");
                        if (trainer->y != player_character->y) {
                            char y_distance[3];
                            if (trainer->y < player_character->y) {
                                sprintf(y_distance, "%d ", player_character->y - trainer->y);
                                addstr(y_distance);
                                addstr("North ");
                            }
                            else {
                                sprintf(y_distance, "%d ", trainer->y - player_character->y);
                                addstr(y_distance);
                                addstr("South ");
                            }
                        }
                        if (trainer->x != player_character->x) {
                            char x_distance[3];
                            if (trainer->x < player_character->x) {
                                sprintf(x_distance, "%d ", player_character->x - trainer->x);
                                addstr(x_distance);
                                addstr("West");
                            }
                            else {
                                sprintf(x_distance, "%d ", trainer->x - player_character->x);
                                addstr(x_distance);
                                addstr("East");
                            }
                        }
                        if (trainer->defeated == 1) {
                            mvaddstr(screen_row, defeated_status_x, "Defeated");
                        }
                        addstr("\n");
                        screen_row++;
                    }
                    refresh();
                }
            }
        } else if (input == 'Q') {
            clear();
            addstr("Are you sure you want to quit (y/n)? All progress will be lost.\n");
            refresh();
            int quit = -1;
            while (quit != 'y' || quit != 'n') {
                quit = getch();
                if (quit == 'y') {
                    return 1;
                } else if (quit == 'n') {
                    clear();
                    addstr("It's your turn! Enter a command or press z for help!\n");
                    print_tile_terrain(tile);
                } else {
                    clear();
                    addstr("Please enter (y/n) to quit. If you quit all progress will be lost.\n");
                    refresh();
                }
            }
        } else if (input == 'z') {
            if (in_help == 0) {
                //enter help
                clear();
                addstr("Enter z to enter/leave the help menu.\n");
                addstr("Enter 9 or u to move one cell to the upper right.\n");
                addstr("Enter 8 or k to move one cell up.\n");
                addstr("Enter 7 or y to move one cell to the upper left.\n");
                addstr("Enter 6 or l to move one cell to the right.\n");
                addstr("Enter 5 or space or . to rest for a turn.\n");
                addstr("Enter 4 or h to move one cell to the left.\n");
                addstr("Enter 3 or n to move one cell to the lower right.\n");
                addstr("Enter 2 or j to move one cell down.\n");
                addstr("Enter 1 or b to move one cell to the lower left.\n");
                addstr("Enter > to enter a pokemart or pokecenter.\n");
                addstr("Enter < to leave a pokemart or pokecenter.\n");
                addstr("Enter t to display a list of trainers.\n");
                addstr("Enter up arrow to scroll up on the trainer list.\n");
                addstr("Enter down arrow to scroll up on the trainer list.\n");
                addstr("Enter escape to leave the trainer list.\n");
                addstr("Enter Q to quit the game.\n");
                refresh();
            }
            else {
                //exit help
                clear();
                addstr("It's your turn! Enter a command or press z for help!\n");
                print_tile_terrain(tile);
            }
            in_help = 1 - in_help;
        } else {
            clear();
            addstr("That is not a valid command. Enter z for help!\n");
            print_tile_terrain(tile);
            refresh();
        }

        //call movement function if moving
        if (moving == 1) {
            //if Terrain can be crossed
            if (tile->tile[new_y][new_x].terrain.pc_weight == INT_MAX) {
                clear();
                addstr("You can't cross that kind of Terrain!\n");
                print_tile_terrain(tile);
            }
                //if there is an undefeated trainer there
            else if (tile->tile[new_y][new_x].character != NULL && tile->tile[new_y][new_x].character->defeated != 0) {
                clear();
                addstr("You have already defeated that trainer so they are too scared to battle you again!");
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
                    clear();
                    addstr("You can't go off of the edge of the world like that! It's your turn! Enter a command or press z for help!\n");
                    print_tile_terrain(tile);
                    refresh();
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
    Character *from_character = tile->tile[y][x].character;
    Character *to_character = tile->tile[new_y][new_x].character;
    //if moving onto PC
    if (to_character != NULL) {
        //pc-trainer combat instigated by either party
        if (from_character->type_enum == PLAYER || to_character->type_enum == PLAYER) {
            if (from_character->type_enum == PLAYER && to_character->defeated == 1) {
                //PC should not be allowed to move to a defeated trainer's location (as of assignment 1.05)
                return 2;
            }
            else {
                combat(from_character, to_character);
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
    return 0;

}

int combat(Character *from_character, Character *to_character) {

    if (from_character->type_enum == PLAYER) {
        //player attacks trainer
        to_character->defeated = 1;
        to_character->color = COLOR_YELLOW;
        clear();
        addstr("Victory! You challenged a trainer to a duel and defeated them soundly! Press escape to leave.\n");
        refresh();
    }
    else {
        from_character->defeated = 1;
        from_character->color = COLOR_YELLOW;
        //trainer attacks player
        clear();
        addstr("Victory! A trainer challenged you to a duel and you trounced them! Press escape to leave.\n");
        refresh();
    }
    int command = -1;
    while (command != 27) {
        command = getch();
        clear();
        addstr("Invalid command. Press press escape to stop your victory dance after defeating that trainer.\n");
        refresh();
    }
    return 0;

}

int enter_center() {

    player_character->in_building = 1;
    clear();
    addstr("You are in a pokecenter! Unfortunately this center is rather barren. Leave by entering \'<\'\n");
    refresh();

    return 0;

}

int enter_mart() {

    player_character->in_building = 1;
    clear();
    addstr("You are in a pokemart! Unfortunately this mart is rather barren. Leave by entering \'<\'\n");
    refresh();

    return 0;

}

int change_tile(int x, int y) {

    //todo: RUN ASSIGNED: upon entering map set all trainers there to same heap time as PC
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
        //todo: RUN BUG: heap insert crash: fix with 1 global heap and remove/re-add trainers (with time update) upon changing maps
        //todo: RUN BUG TEST: uncomment below once heap crash insert bug fixed
//        if (new_tile->turn_heap->size > 0) {
//            NonPlayerCharacter *trainer = heap_remove_min(new_tile->turn_heap);
//            while (trainer->turn < player_character->turn) {
//                trainer->turn = player_character->turn;
//                heap_insert(new_tile->turn_heap, trainer);
//                trainer = heap_remove_min(new_tile->turn_heap);
//            }
//        }
        heap_insert(world[current_tile_y][current_tile_x]->turn_heap, player_character);
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
    struct heap turn_heap;
    heap_init(&turn_heap, comparator_character_movement, NULL);
    tile.turn_heap = &turn_heap;
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

    struct heap *turn_heap = tile->turn_heap;

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
    heap_insert(turn_heap, player_character);
    tile->player_character = (PlayerCharacter *) player_character;
    tile->tile[y][x].character = player_character;
    //create distance tiles
    dijkstra(tile, RIVAL);
    dijkstra(tile, HIKER);

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

    struct heap *turn_heap = tile->turn_heap;
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
        //todo: CRASH: Segmentation fault. Maybe giving pointer/non pointer when should give the other? This is the first heap insert.
        heap_insert(turn_heap, trainer);
        tile->tile[y][x].character = trainer;
        num_trainer--;
    }

    return 0;

}

int dijkstra(Tile *tile, enum character_type trainer_type) {

    int start_x = tile->player_character->x;
    int start_y =tile->player_character->y;

    for (int y = 0; y < TILE_LENGTH_Y; y++) {
        for (int x = 0; x < TILE_WIDTH_X; x++) {
            tile->tile[y][x].distance = INT_MAX;
        }
    }
    tile->tile[start_y][start_x].distance = 0;

    struct heap heap;
    static Point *point;
    heap_init(&heap, comparator_trainer_distance_tile, NULL);
    for (int y = 0; y < TILE_LENGTH_Y; y++) {
        for (int x = 0; x < TILE_WIDTH_X; x++) {
            int weight;
            if (trainer_type == RIVAL) {
                weight = tile->tile[y][x].terrain.rival_weight;
            }
            else {
                //character_type type_enum == hiker
                weight = tile->tile[y][x].terrain.hiker_weight;
            }
            if (weight != INT_MAX) {
                tile->tile[y][x].heap_node = heap_insert(&heap, &tile->tile[y][x]);
            }
            else {
                tile->tile[y][x].heap_node = NULL;
            }
        }
    }
    while ((point = (Point*) (&heap))) {
        point->heap_node = NULL;
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                if (point->y + y >= 0 && point->y + y < TILE_LENGTH_Y && point->x + x >= 0 && point->x + x < TILE_WIDTH_X)
                {
                    Point *neighbor = &(tile->tile[point->y + y][point->x + x]);
                    int candidate_distance;
                    if (trainer_type == RIVAL) {
                        candidate_distance = point->distance + neighbor->terrain.rival_weight;
                    } else {
                        //character_type type_enum == hiker
                        candidate_distance = point->distance + neighbor->terrain.hiker_weight;
                    }
                    if (neighbor->heap_node != NULL && candidate_distance < neighbor->distance &&
                        candidate_distance > 0) {
                        neighbor->distance = candidate_distance;
                        heap_decrease_key_no_replace(&heap, neighbor->heap_node);
                    }
                }
            }
        }
    }
    heap_delete(&heap);

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
                init_pair(1, COLOR_RED, COLOR_BLACK);
                //init_pair(1, Tile->Tile[y][x].Character->color, COLOR_BLACK);
                attrset(COLOR_PAIR(1));
                mvaddch(y + 1, x, printable_character);
                refresh();
                attroff(COLOR_PAIR(1));
            }
            else {
                init_pair(1, tile->tile[y][x].terrain.color, COLOR_BLACK);
                attrset(COLOR_PAIR(1));
                mvaddch(y + 1, x, printable_character);
                refresh();
                attroff(COLOR_PAIR(1));
            }
        }
    }
    addch('\n');
    refresh();

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
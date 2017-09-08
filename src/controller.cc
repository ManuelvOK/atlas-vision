#include <controller.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include <vision.h>
/**
 * parse line of input data
 *
 * @param line
 *   line to parse
 */
static void parse_line(std::string line);

/**
 * parse number of cores from input line
 *
 * @param line
 *   line to parse with index at first parameter
 */
static void parse_n_cores(std::stringstream *line);

/**
 * parse job from input line
 *
 * @param line
 *   line to parse with index at first parameter
 */
static void parse_job(std::stringstream *line);

/**
 * parse schedule cores from input line
 *
 * @param line
 *   line to parse with index at first parameter
 */
static void parse_schedule(std::stringstream *line);

/**
 * calculate values that are essential for the player
 * This has to happen after the input file is parsed.
 */
static void calculate_player_values();

/**
 * perform logic steps for simulation player
 */
static void player_tick();

/**
 * state of application
 */
static struct state *state;

const struct state *init_state(void) {
    assert(state == nullptr);
    state = new struct state;
    state->running = 1;
    state->n_cores = -1;
    state->jobs = std::vector<struct job>();
    state->schedules = std::vector<struct schedule>();
    state->hovered_job = -1;
    state->player.running = 0;
    state->player.position = 0;
    state->player.max_position = 0;
    return state;
}

void handle_input(const struct input *input) {
    if (input->quit) {
        state->running = 0;
        return;
    }
    if (input->toggle_play) {
        state->player.running = !state->player.running;
    }
    if (input->rewind) {
        state->player.position = 0;
    }
    state->hovered_job = get_hovered_job(input->mouse_position_x, input->mouse_position_y, state);
}

void read_input_from_stdin() {
    std::string line;
    while (std::getline(std::cin, line)) {
        parse_line(line);
    }
    calculate_player_values();
}

void read_input_from_file(std::string path) {
    std::ifstream input_file(path);
    if (not input_file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(input_file, line)) {
        parse_line(line);
    }
    input_file.close();
    calculate_player_values();
}

void parse_line(std::string line) {
    std::stringstream ss(line);
    char type = 0;
    ss >> type;
    switch (type) {
        case 'c': parse_n_cores(&ss);  break;
        case 'j': parse_job(&ss);      break;
        case 's': parse_schedule(&ss); break;
        case '#': break;
        default: std::cerr << "Parse error: \"" << type << "\" is not a proper type." << std::endl;
    }

}

static void parse_n_cores(std::stringstream *line) {
    *line >> state->n_cores;
    std::cout << line->str() << std::endl;
}

static void parse_job(std::stringstream *line) {
    int id, deadline, time_estimate, time, submission;
    *line >> id >> deadline >> time_estimate >> time >> submission;
    state->jobs.push_back({id, deadline, time_estimate, time, submission});
}

static void parse_schedule(std::stringstream *line) {
    int job_id, core, player_time, begin, time;
    *line >> job_id >> core >> player_time >> begin >> time;
    state->schedules.push_back({job_id, core, player_time, begin, time});
}

bool check_state() {
    return true;
}

void control() {
    player_tick();
}

void player_tick() {
    if (!state->player.running) {
        return;
    }
    state->player.position += 0.04;
    if (state->player.position > state->player.max_position) {
        state->player.running = 0;
        state->player.position = 0;
    }
}

void calculate_player_values() {
    struct schedule last_schedule = state->schedules.back();
    //struct job last_job = state->jobs[last_schedule.job_id];

    state->player.max_position = last_schedule.begin + last_schedule.execution_time;
}

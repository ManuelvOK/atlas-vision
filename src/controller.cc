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
    return state;
}

void handle_input(const struct input *input) {
    if (input->quit) {
        state->running = 0;
        return;
    }
    state->hovered_job = get_hovered_job(input->mouse_position_x, input->mouse_position_y);
}


void read_input_from_stdin() {
    std::string line;
    while (std::getline(std::cin, line)) {
        parse_line(line);
    }
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
}
void parse_line(std::string line) {
    std::stringstream ss(line);
    char type;
    ss >> type;
    switch (type) {
        case 'c': parse_n_cores(&ss);  break;
        case 'j': parse_job(&ss);      break;
        case 's': parse_schedule(&ss); break;
    }

}

static void parse_n_cores(std::stringstream *line) {
    *line >> state->n_cores;
}
static void parse_job(std::stringstream *line) {
    int id, deadline, time;
    *line >> id >> time >> deadline;
    state->jobs.push_back({id, deadline, time});
}
static void parse_schedule(std::stringstream *line) {
    int job_id, core, start;
    *line >> job_id >> core >> start;
    state->schedules.push_back({job_id, core, start});
}
bool check_state() {
    return true;
}

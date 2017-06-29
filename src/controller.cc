#include <controller.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <cstdio>

static void parse_n_cores(std::string line);
static void parse_job(std::string line);
static void parse_schedule(std::string line);

static struct state *state;

const struct state *init_state(void) {
    assert(state == nullptr);
    state = new struct state;
    state->running = 1;
    state->n_cores = -1;
    state->jobs = std::vector<struct job>();
    state->schedules = std::vector<struct schedule>();
    return state;
}

void handle_input(const struct input *input) {
    if (input->quit) {
        state->running = 0;
    }
}


void read_input_from_stdin() {
    return;
}

void read_input_from_file(std::string path) {
    std::ifstream input_file(path);
    if (not input_file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(input_file, line)) {
        switch (line[0]) {
            case 'c': parse_n_cores(line);  break;
            case 'j': parse_job(line);      break;
            case 's': parse_schedule(line); break;
        }
    }
    input_file.close();
}

static void parse_n_cores(std::string line) {
    std::sscanf(line.c_str(), "c %d", &state->n_cores);
}
static void parse_job(std::string line) {
    int id, deadline, time;
    std::sscanf(line.c_str(), "j %d %d %d", &id, &time, &deadline);
    state->jobs.push_back({id, deadline, time});
}
static void parse_schedule(std::string line) {
    int job_id, core, start, time;
    int i = std::sscanf(line.c_str(), "s %d %d %d %d", &job_id, &core, &start, &time);
    state->schedules.push_back({job_id, core, start, time});
}
bool check_state() {
    return true;
}

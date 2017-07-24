#pragma once

#include <input.h>
#include <vector>
#include <string>

struct job {
    int id;
    int deadline;
    int time;
};

struct schedule {
    int job_id;
    int core;
    int start;
};

struct state {
    int running;
    int n_cores;
    std::vector<struct job> jobs;
    std::vector<struct schedule> schedules;
};

const struct state *init_state(void);
void handle_input(const struct input *input);
void read_input_from_stdin();
void read_input_from_file(std::string path);
bool check_state();

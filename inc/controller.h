#pragma once

#include <input.h>
#include <vector>
#include <string>

/**
 * Object representing a job
 *
 * @param id
 *   job id
 * @param deadline
 *   timestamp for the deadline of the job
 * @param execution_time_estimate
 *   estimated execution time
 * @param execution_time
 *   real execution time
 * @param submission_time
 *   timestamp the job gets submitted
 */
struct job {
    int id;
    int deadline;
    int execution_time_estimate;
    int execution_time;
    int submission_time;
};

/**
 * Object representing a schedule for a certain job
 *
 * @param job_id
 *   Id of the concerning job
 * @param core
 *   core on wich the job gets executed
 * @player_state
 *   player state in wich the schedule is active
 * @param begin
 *   start time of job execution
 * @param execution_time
 *   time the scheduled job runs
 */
struct schedule {
    int job_id;
    int core;
    int player_state;
    int begin;
    int execution_time;
};

/**
 * Representation of the simulation player
 *
 * @param running
 *   determines if the simulation gets played
 * @param position
 *   current position of player
 * @param max_position
 *   length of full simulation
 */
struct player {
    int running;
    float position;
    float max_position;
};

/**
 * Representation of the application data
 *
 * @param running
 *   flag to determine whether the application runs
 * @param n_cores
 *   number of cores the jobs get scheduled on
 * @param jobs
 *   list of jobs
 * @param schedules
 *   list of schedules for the jobs
 * @param player
 *   simulation player
 */
struct state {
    int running;
    int n_cores;
    std::vector<struct job> jobs;
    std::vector<struct schedule> schedules;
    int hovered_job;
    struct player player;
};

/**
 * initialises the application state.
 *
 * @returns
 *   Constant Pointer to the state object
 */
const struct state *init_state(void);

/**
 * handle user input data
 *
 * @param input
 *   object with input data
 */
void handle_input(const struct input *input);

/**
 * parse data from stdin
 */
void read_input_from_stdin();

/**
 * parse data from file
 *
 * @param path
 *   file path of input file
 */
void read_input_from_file(std::string path);

/**
 * check plausibility of state
 *
 * Call this after input parsing
 *
 * @returns
 *   True of state is plausible. False otherwise
 */
bool check_state();

/**
 * perform logic step for each tick
 */
void control();

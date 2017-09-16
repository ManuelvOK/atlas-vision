#pragma once

#include <input.h>
#include <vector>
#include <string>
#include <map>

#include <job.h>
#include <schedule.h>
#include <schedule_change.h>



/**
 * initialises the application model.
 *
 * @returns
 *   Constant Pointer to the model object
 */
const Model *init_model(void);

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
 * check plausibility of model
 *
 * Call this after input parsing
 *
 * @returns
 *   True of model is plausible. False otherwise
 */
bool check_model();

/**
 * perform logic step for each tick
 */
void control();

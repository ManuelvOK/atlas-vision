#pragma once

#include <vector>
#include <string>
#include <map>

#include <input.h>
#include <model/job.h>
#include <model/model.h>
#include <model/schedule.h>
#include <model/schedule_change.h>

class View;

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
void handle_input(const struct input *input, View *view);

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

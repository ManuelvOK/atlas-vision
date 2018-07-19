#include <controller.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <algorithm>

#include <model.h>
#include <view.h>

/**
 * parse input data from ifstream
 */
static void parse_file(std::istream *input);

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
 * parse schedule from input line
 *
 * @param line
 *   line to parse with index at first parameter
 */
static void parse_schedule(std::stringstream *line);

/**
 * parse schedule altering from input line
 * changes are only parsed to apply them later
 *
 * @param line
 *   line to parse with index at first parameter
 * @param changes
 *   vector to put Schedule_change object to
 */
static void parse_change(std::stringstream *line, std::vector<Schedule_change> *changes);

/**
 * parse cfs visibility. This is the foremost ATLAS job that cfs can schedule
 *
 * @param line
 *   line to parse with index at first parameter
 */
static void parse_cfs_visibility(std::stringstream *line);

/**
 * add schedule change to appropriate schedule
 *
 * @param change
 *   Schedule_change to apply
 *
 * @return
 *   true if applied succesfull. Otherwise false
 */
bool apply_schedule_change(const Schedule_change &change);

/**
 * model of application
 */
static Model *model;

const Model *init_model(void) {
    assert(model == nullptr);
    model = new Model();
    return model;
}

void handle_input(const struct input *input, View *view) {
    if (input->quit) {
        model->running = 0;
        return;
    }
    /* Player input */
    if (input->player.toggle_play) {
        model->player.toggle();
    }
    if (input->player.rewind) {
        model->player.rewind();
    }
    if (input->player.position >= 0) {
        model->player.set(input->player.position);
    }
    view->rescale(input->rescale);
#if 0
    /* window changed */
    if (input->window.changed) {
        update_window(input->window.width, input->window.height);
    }
    /* hovered job */
    model->hovered_job = get_hovered_job(input->mouse_position_x, input->mouse_position_y);
#endif
}

void read_input_from_stdin() {
    parse_file(&std::cin);
}

void parse_file(std::istream *input) {
    std::vector<Schedule_change> changes;
    std::string line;
    while (std::getline(*input, line)) {
        std::stringstream ss(line);
        char type = 0;
        ss >> type;
        switch (type) {
            case 'c': parse_n_cores(&ss);          break;
            case 'j': parse_job(&ss);              break;
            case 's': parse_schedule(&ss);         break;
            case 'a': parse_change(&ss, &changes); break;
            case 'v': parse_cfs_visibility(&ss);   break;
            case 0:
            case '#': break;
            default: std::cerr << "Parse error: \"" << type << "\" is not a proper type."
                     << std::endl; break;
        }
    }

    bool succ = true;
    /* apply changes */
    for (const Schedule_change &change: changes) {
        succ = succ && apply_schedule_change(change);
    }

    /* check for validity of input */
    if (!check_model()) {
        model->running = 0;
        return;
    }

    /* init player */
    model->player.init(model);
}


void read_input_from_file(std::string path) {
    std::ifstream input_file(path);
    if (not input_file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    parse_file(&input_file);
    input_file.close();
    /* sort jobs id wise */
    std::sort(model->jobs.begin(), model->jobs.end(),
            [](const Job &a, const Job &b) -> bool {return a.id < b.id;});
}

void parse_n_cores(std::stringstream *line) {
    *line >> model->n_cores;
}

void parse_job(std::stringstream *line) {
    int id, deadline, time_estimate, time, submission;
    *line >> id >> deadline >> time_estimate >> time >> submission;
    model->jobs.emplace_back(id, deadline, time_estimate, time, submission);
}

void parse_schedule(std::stringstream *line) {
    int id, job_id, core;
    int submission_time, begin, time;
    char scheduler;
    *line >> id >> job_id >> core >> scheduler >> submission_time >> begin >> time;
    model->schedules.emplace(id, Schedule{id, job_id, core, scheduler, submission_time, begin, time});
}

void parse_change(std::stringstream *line, std::vector<Schedule_change> *changes) {
    int schedule_id;
    int timestamp;
    int value = -1;
    char type;
    *line >> type >> timestamp >> schedule_id;
    if (static_cast<change_type>(type) != change_type::erase) {
        *line >> value;
    }
    changes->emplace_back(schedule_id, timestamp, type, value);
}

void parse_cfs_visibility(std::stringstream *line) {
    int schedule_id;
    int begin, end;
    *line >> schedule_id >> begin >> end;
    model->cfs_visibilities.emplace_back(schedule_id, begin, end);
}

bool apply_schedule_change(const Schedule_change &change) {
    if (static_cast<unsigned>(change.schedule_id) >= model->schedules.size()) {
        std::cerr << "input error: validity\t- There is no Schedule with id " << change.schedule_id
                  << " to change" << std::endl;
    }
    Schedule *schedule = &model->schedules.at(change.schedule_id);
    switch (change.type) {
        case change_type::erase:
            schedule->end = change.timestamp;
            break;
        case change_type::shift:
            schedule->begin.emplace(change.timestamp, change.value);
            break;
        case change_type::change_execution_time:
            schedule->execution_time.emplace(change.timestamp, change.value);
            break;
        default: break;
    }
    return true;
}

bool check_model() {
    return true;
}

void control() {
    model->player.tick();
}

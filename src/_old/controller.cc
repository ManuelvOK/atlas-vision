#include <controller.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <algorithm>

#include <model/model.h>
#include <parser.h>
#include <view/view.h>

/**
 * parse input data from ifstream
 */
static void parse_file(std::istream *input);


/**
 * add schedule change to appropriate schedule
 *
 * @param change
 *   ScheduleChange to apply
 *
 * @return
 *   true if applied succesfull. Otherwise false
 */
bool apply_schedule_change(const ScheduleChange *change);

/**
 * model of application
 */
static Model *model;

/**
 * parser
 */
static Parser parser;

const Model *init_model(void) {
    assert(model == nullptr);
    model = new Model();
    return model;
}

void handle_input(const struct input *input, View *view) {
    if (input->quit) {
        model->_running = 0;
        return;
    }
    /* Player input */
    if (input->player.toggle_play) {
        model->_player.toggle();
    }
    if (input->player.rewind) {
        model->_player.rewind();
    }
    if (input->player.position >= 0) {
        model->_player.set(input->player.position);
    }
    if (input->player.offset != 0) {
        view->shift_player(input->player.offset);
    }
    view->rescale(input->rescale);
#if 0
    /* window changed */
    if (input->window.changed) {
        update_window(input->window.width, input->window.height);
    }
    /* hovered job */
    model->_hovered_job = get_hovered_job(input->mouse_position_x, input->mouse_position_y);
#endif
}

void read_input_from_stdin() {
    parse_file(&std::cin);
}

void parse_file(std::istream *input) {
    std::string line;
    while (std::getline(*input, line)) {
        parser.parse_line(line);
    }

    bool succ = true;
    model->_n_cores = parser._n_cores;
    model->_jobs = parser._jobs;
    model->_schedules = parser._schedules;
    model->_cfs_visibilities = parser._cfs_visibilities;
    model->_messages = parser._messages;

    for (std::pair<int, int> d: parser._dependencies) {
        if (model->_jobs.size() < std::max(d.first, d.second)) {
            std::cerr << "Error parsing dependency \"d " << d.first << " " << d.second << "\": job vector has size of " << model->_jobs.size() << std::endl;
        }
        model->_jobs[d.first]->_known_dependencies.push_back(model->_jobs[d.second]);
    }
    /* apply changes */
    for (const ScheduleChange *change: parser._changes) {
        succ = succ && apply_schedule_change(change);
    }

    /* calculate dependency level */
    for (Job *job: model->_jobs) {
        job->calculate_dependency_level();
        std::cerr << "dependency level of job " << job->_id << ": " << job->_dependency_level << std::endl;
    };


    /* check for validity of input */
    if (!check_model()) {
        model->_running = 0;
        return;
    }

    /* init player */
    model->_player.init(model);
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
    std::sort(model->_jobs.begin(), model->_jobs.end(),
            [](const Job *a, const Job *b) -> bool {return a->_id < b->_id;});
}

bool apply_schedule_change(const ScheduleChange *change) {
    if (static_cast<unsigned>(change->_schedule_id) >= model->_schedules.size()) {
        std::cerr << "input error: validity\t- There is no Schedule with id " << change->_schedule_id
                  << " to change" << std::endl;
    }
    Schedule *schedule = model->_schedules.at(change->_schedule_id);
    switch (change->_type) {
        case ChangeType::erase:
            schedule->_end = change->_timestamp;
            break;
        case ChangeType::shift:
            schedule->_begin.emplace(change->_timestamp, change->_value);
            break;
        case ChangeType::change_execution_time:
            schedule->_execution_time.emplace(change->_timestamp, change->_value);
            break;
        default: break;
    }
    return true;
}

bool check_model() {
    return true;
}

void control() {
    model->_player.tick();
}

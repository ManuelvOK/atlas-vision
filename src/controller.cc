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
    model->hovered_job = get_hovered_job(input->mouse_position_x, input->mouse_position_y);
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
    model->n_cores = parser.n_cores;
    model->jobs = parser.jobs;
    model->schedules = parser.schedules;
    model->cfs_visibilities = parser.cfs_visibilities;
    model->messages = parser.messages;

    for (std::pair<int, int> d: parser.dependencies) {
        if (model->jobs.size() < std::max(d.first, d.second)) {
            std::cerr << "Error parsing dependency \"d " << d.first << " " << d.second << "\": job vector has size of " << model->jobs.size() << std::endl;
        }
        model->jobs[d.first]->known_dependencies.push_back(model->jobs[d.second]);
    }
    /* apply changes */
    for (const ScheduleChange *change: parser.changes) {
        succ = succ && apply_schedule_change(change);
    }

    /* calculate dependency level */
    for (Job *job: model->jobs) {
        job->calculate_dependency_level();
        std::cerr << "dependency level of job " << job->id << ": " << job->dependency_level << std::endl;
    };


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
            [](const Job *a, const Job *b) -> bool {return a->id < b->id;});
}

bool apply_schedule_change(const ScheduleChange *change) {
    if (static_cast<unsigned>(change->schedule_id) >= model->schedules.size()) {
        std::cerr << "input error: validity\t- There is no Schedule with id " << change->schedule_id
                  << " to change" << std::endl;
    }
    Schedule *schedule = model->schedules.at(change->schedule_id);
    switch (change->type) {
        case ChangeType::erase:
            schedule->end = change->timestamp;
            break;
        case ChangeType::shift:
            schedule->begin.emplace(change->timestamp, change->value);
            break;
        case ChangeType::change_execution_time:
            schedule->execution_time.emplace(change->timestamp, change->value);
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

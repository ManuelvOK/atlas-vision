#pragma once

#include <map>

#include <model/cfs_visibility.h>
#include <model/job.h>
#include <model/message.h>
#include <model/player.h>
#include <model/schedule.h>
#include <model/schedule_change.h>

/**
 * Representation of the application data
 */
class Model {
public:
    int _running = 1; /**< flag to determine whether the application runs */
    int _n_cores = -1; /**< number of cores the jobs get scheduled on */
    std::vector<Job *> _jobs; /**< list of jobs */
    std::map<int, Schedule *> _schedules; /**< list of schedules for the jobs */
    std::vector<CfsVisibility *> _cfs_visibilities; /**< list of atlas schedules visibile for cfs scheduler */
    std::vector<Message *> _messages; /**< messages to display at a given timestamp */
    Player _player; /**< simulation player */
    /* TODO: move out of model */
    int _hovered_job = -1; /**< ID of Job that gets hovered */

    /**
     * Constructor
     */
    Model() : _jobs(), _schedules(), _cfs_visibilities(), _player() {}
};

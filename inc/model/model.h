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
    /**
     * flag to determine whether the application runs
     */
    int running = 1;

    /**
     * number of cores the jobs get scheduled on
     */
    int n_cores = -1;

    /**
     * list of jobs
     */
    std::vector<Job> jobs;

    /**
     * list of schedules for the jobs
     */
    std::map<int, Schedule> schedules;

    /**
     * list of atlas schedules visibile for cfs scheduler
     */
    std::vector<Cfs_visibility> cfs_visibilities;

    /**
     * messages to display at a given timestamp
     */
    std::vector<Message> messages;

    /**
     * simulation player
     */
    Player player;

    /**
     * TODO
     */
    int hovered_job = -1;

    Model() : jobs(), schedules(), cfs_visibilities(), player() {}
};

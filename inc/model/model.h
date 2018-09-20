#pragma once

#include <map>

#include <model/job.h>
#include <model/schedule.h>
#include <model/schedule_change.h>
#include <model/cfs_visibility.h>
#include <model/player.h>

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
     * simulation player
     */
    Player player;

    /**
     * TODO
     */
    int hovered_job = -1;

    Model() : jobs(), schedules(), cfs_visibilities(), player() {}
};

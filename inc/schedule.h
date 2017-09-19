#pragma once

#include <map>
#include <tuple>

enum class scheduler_type {
    ATLAS = 'a',
    recovery = 'r',
    CFS = 'c'
};

/**
 * Object representing a schedule for a certain job
 */
class Schedule {
public:
    /**
     * id of this schedule
     */
    int id;

    /**
     * Id of the concerning job
     */
    int job_id;

    /**
     * core on wich the job gets executed
     */
    int core;

    /**
     * the scheduler this schedule is on
     */
    std::map<int, scheduler_type> scheduler;

    /**
     * time this schedule gets submittet
     */
    int submission_time;

    /**
     * start times of job execution
     * this is a map for storing different execution times for different player states
     */
    std::map<int, int> begin;

    /**
     * possible end of this schedule
     */
    int end = -1;

    /**
     * time the scheduled job runs
     */
    std::map<int, int> execution_time;

    /**
     * Constructor
     */
    Schedule(int id, int job_id, int core, char scheduler, int submission_time, int begin,
            int execution_time);

    /**
     * get relevant data for rendering for a given timestamp
     */
    std::tuple<int, scheduler_type, int> get_data_at_time(float timestamp = 0) const;

    /**
     * check if the schedule does exist at a given timestamp
     */
    bool exists_at_time(float timestamp) const;

    int get_maximal_end() const;
};


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
    std::map<float, scheduler_type> scheduler;

    /**
     * time this schedule gets submittet
     */
    float submission_time;

    /**
     * start times of job execution
     * this is a map for storing different execution times for different timestamps
     */
    std::map<float, float> begin;

    /**
     * possible end of this schedule
     */
    float end = -1;

    /**
     * time the scheduled job runs
     */
    std::map<float, float> execution_time;

    /**
     * Constructor
     */
    Schedule(int id, int job_id, int core, char scheduler, float submission_time, float begin,
            float execution_time);

    /**
     * get relevant data for rendering for a given timestamp
     */
    std::tuple<float, scheduler_type, float> get_data_at_time(float timestamp = 0) const;

    /**
     * check if the schedule does exist at a given timestamp
     */
    bool exists_at_time(float timestamp) const;

    float get_maximal_end() const;
};


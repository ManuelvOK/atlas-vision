#pragma once

#include <map>
#include <tuple>

enum class SchedulerType {
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
    std::map<int, SchedulerType> scheduler;

    /**
     * time this schedule gets submittet
     */
    int submission_time;

    /**
     * start times of job execution
     * this is a map for storing different execution times for different timestamps
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
    std::tuple<int, SchedulerType, int> get_data_at_time(int timestamp = 0) const;

    /**
     * check if the schedule does exist at a given timestamp
     */
    bool exists_at_time(int timestamp) const;

    /**
     * check if the schedule gets executed at a given timestamp
     */
    bool is_active_at_time(int timestamp) const;

    int get_maximal_end() const;
};


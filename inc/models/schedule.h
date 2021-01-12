#pragma once

#include <map>
#include <set>
#include <tuple>

#include <models/schedule_change.h>

/** type of scheduler encoded as ascii character */
enum class SchedulerType {
    ATLAS = 'a',
    recovery = 'r',
    CFS = 'c'
};

struct ScheduleData {
    SchedulerType _scheduler;   /**< the scheduler this schedule is on */
    int _begin;                 /**< start time of schedule execution */
    int _execution_time;        /**< time the scheduled job runs */
    bool _does_execute = true;  /**< flag determining whether the schedule does run */
};

/** Object representing a schedule for a certain job */
class Schedule {
    ScheduleData &data_at_time(int timestamp);
    const ScheduleData &data_at_time(int timestamp) const;
public:
    int _id;                            /**< id of this schedule */
    int _job_id;                        /**< Id of the concerning job */
    int _core;                          /**< core on wich the job gets executed */
    int _submission_time;               /**< time this schedule gets submitted */

    std::map<int, ScheduleData> _data;  /**< data for a given time period */
    int _end = -1;                      /**< possible end of this schedule */
    std::set<int> _change_points;       /**< list of points at which changes happen */

    /**
     * Constructor
     * @param id schedule id
     * @param job_id id of corresponding job
     * @param core id of cpu this schedule gets executed on
     * @param scheduler type of scheduler this schedule gets executed on
     * @param submission_time timestamp of schedules submission
     * @param begin time the schedule begins execution
     * @param execution_time time the schedule gets executed
     */
    Schedule(int id, int job_id, int core, char scheduler, int submission_time, int begin,
             int execution_time);

    /**
     * Add a change to this schedule
     * @param change change object to add
     */
    void add_change(const ScheduleChange *change);

    /**
     * get relevant data for rendering for a given timestamp
     * @param timestamp timestamp to get data for
     * @returns schedules data at given timestamp
     */
    ScheduleData get_data_at_time(int timestamp = 0) const;

    /**
     * check if the schedule does exist at a given timestamp
     * @param timestamp timestamp to check against
     * @returns True if schedule exists at timestamp. False otherwise.
     */
    bool exists_at_time(int timestamp) const;

    /**
     * check if the schedule gets executed at a given timestamp
     * @param timestamp timestamp to check against
     * @returns True if the schedule gets executed at timestamp. False otherwise.
     */
    bool is_active_at_time(int timestamp) const;

    /**
     * get the worst-case end timestamp for this schedule
     * @returns worst-case end timestamp
     */
    int get_maximal_end() const;
};


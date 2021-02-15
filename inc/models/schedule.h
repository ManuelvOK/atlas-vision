#pragma once

#include <map>
#include <set>
#include <tuple>

#include <models/printable.h>

class Job;
class ParsedChange;

/** type of scheduler encoded as ascii character */
enum class SchedulerType {
    ATLAS = 'a',
    recovery = 'r',
    CFS = 'c'
};

struct ScheduleData {
    int _timestamp;
    SchedulerType _scheduler;   /**< the scheduler this schedule is on */
    int _begin;                 /**< start time of schedule execution */
    int _execution_time;        /**< time the scheduled job runs */
    bool _does_execute = true;  /**< flag determining whether the schedule does run */

    int end() const;
};

/** Object representing a schedule for a certain job */
class Schedule : public Printable {
    static int _next_id;
    ScheduleData &data_at_time(int timestamp);
    const ScheduleData &data_at_time(int timestamp) const;

public:
    int _id;                            /**< id of this schedule */
    Job *_job;                          /**< the concerning job */
    unsigned _core;                     /**< core on wich the job gets executed */
    int _submission_time;               /**< time this schedule gets submitted */

    std::map<int, ScheduleData> _data;  /**< data for a given time period */
    int _end = -1;                      /**< possible end of this schedule */
    std::set<int> _change_points;       /**< list of points at which changes happen */

    /**
     * Constructor
     * @param id schedule id
     * @param job corresponding job
     * @param core id of cpu this schedule gets executed on
     * @param scheduler type of scheduler this schedule gets executed on
     * @param submission_time timestamp of schedules submission
     * @param begin time the schedule begins execution
     * @param execution_time time the schedule gets executed
     */
    Schedule(int id, Job *job, unsigned core, SchedulerType scheduler, int submission_time,
             int begin, int execution_time);

    Schedule(Job *job, unsigned core, SchedulerType scheduler, int submission_time, int begin,
             int execution_time);

    Schedule(const Schedule *s);

    virtual ~Schedule() = default;

    static int next_id();

    static void reset_next_id();

    /**
     * Add a change to this schedule
     * @param change change object to add
     */
    void add_change(const ParsedChange &change);

    void add_change(int timestamp, int begin, int execution_time);

    void add_change_begin(int timestamp, int begin);

    void add_change_shift_relative(int timestamp, int shift);

    void add_change_execution_time_relative(int timestamp, int execution_time_difference);

    void add_change_end(int timestamp, int end);

    void add_change_delete(int timestamp);

    /**
     * get relevant data for rendering for a given timestamp
     * @param timestamp timestamp to get data for
     * @returns schedules data at given timestamp
     */
    ScheduleData get_data_at_time(int timestamp = 0) const;

    ScheduleData get_vision_data_at_time(int timestamp = 0) const;

    /**
     * get first Schedule Data
     * @returns first ScheduleData in _data
     */
    ScheduleData first_data() const;

    /**
     * get last Schedule Data
     * @returns last ScheduleData in _data
     */
    ScheduleData last_data() const;
    ScheduleData &last_data();

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

    std::string to_string() const override;
};


class AtlasSchedule : public Schedule {
  public:
    using Schedule::Schedule;
    AtlasSchedule(int id, Job *job, int core, int submission_time, int begin, int execution_time)
        : Schedule(id, job, core, SchedulerType::ATLAS, submission_time, begin, execution_time) {}
    AtlasSchedule(Job *job, int core, int submission_time, int begin, int execution_time)
        : Schedule(job, core, SchedulerType::ATLAS, submission_time, begin, execution_time) {}

};

class DependencySchedule : public AtlasSchedule {
  public:
    using AtlasSchedule::AtlasSchedule;
};

class CfsSchedule : public Schedule {
  public:
    using Schedule::Schedule;
    CfsSchedule(int id, Job *job, int core, int submission_time, int begin, int execution_time)
        : Schedule(id, job, core, SchedulerType::CFS, submission_time, begin, execution_time) {}
    CfsSchedule(Job *job, int core, int submission_time, int begin, int execution_time)
        : Schedule(job, core, SchedulerType::CFS, submission_time, begin, execution_time) {}
    CfsSchedule(AtlasSchedule *s, int submission_time, int begin, int execution_time);
};

class EarlyCfsSchedule : public CfsSchedule {
  public:
    using CfsSchedule::CfsSchedule;
    AtlasSchedule *_atlas_schedule = nullptr;
    EarlyCfsSchedule(AtlasSchedule *s, int submission_time, int begin, int execution_time)
        : CfsSchedule(s, submission_time, begin, execution_time), _atlas_schedule(s) {}
};

class LateCfsSchedule : public CfsSchedule {
  public:
    using CfsSchedule::CfsSchedule;
};

class RecoverySchedule : public Schedule {
  public:
    using Schedule::Schedule;
    RecoverySchedule(int id, Job *job, int core, int submission_time, int begin, int execution_time)
        : Schedule(id, job, core, SchedulerType::recovery, submission_time, begin,
                   execution_time) {}
    RecoverySchedule(Job *job, int core, int submission_time, int begin, int execution_time)
        : Schedule(job, core, SchedulerType::recovery, submission_time, begin, execution_time) {}
    RecoverySchedule(const Schedule *s);
};

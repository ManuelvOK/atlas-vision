#pragma once

#include <vector>

#include <printable.h>

/** Object representing a job */
class BaseJob : public Printable {
public:
    unsigned _id;                        /**< job id */
    int _deadline;                  /**< timestamp for the deadline of the job */
    unsigned _execution_time;            /**< real execution time */
    int _submission_time;           /**< timestamp the job gets submitted */

    /**
     * Constructor
     * @param id job id
     * @param deadline timestamp for jobs deadline
     * @param execution_time_estimate estimated time for job execution
     * @param execution_time real execution time
     * @param submission_time timestamp of jobs submission
     */
    BaseJob(unsigned id, int deadline, unsigned execution_time, int submission_time)
        : _id(id), _deadline(deadline), _execution_time(execution_time),
          _submission_time(submission_time) {}

    virtual int deadline(int timestamp) const;

    virtual unsigned time_executed(int timestamp) const = 0;

    virtual unsigned execution_time_left(int timestamp) const;

    virtual bool finished(int timestamp) const;
};

class BaseSchedule;
template <typename T>
class Job : public BaseJob {
    static_assert(std::is_base_of<BaseSchedule, T>::value);
public:
    using BaseJob::BaseJob;
    std::vector<T *> _schedules;

    T *schedule_at_time(int timestamp) {
        for (T *s: this->_schedules) {
            auto data = s->get_data_at_time(timestamp);
            if (data._begin <= timestamp && data.end() > timestamp) {
                return s;
            }
        }
        return nullptr;
    }

    virtual unsigned time_executed(int timestamp) const override {
        unsigned time_executed = 0;
        for (T *s: this->_schedules) {
            auto data = s->get_data_at_time(timestamp);
            if (not data._does_execute) {
                continue;
            }
            unsigned value = data._execution_time;
            if (data._begin <= timestamp) {
                value = std::min<unsigned>(value, timestamp - data._begin);
            }
            time_executed += value;
        }
        return time_executed;
    }

};

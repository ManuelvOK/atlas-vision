#pragma once

#include <map>
#include <set>
#include <tuple>

#include <gui/gui_schedule_data.h>
#include <job.h>
#include <util/parser.h>

class ParsedChange;

struct ScheduleData {
    unsigned _timestamp;
    unsigned _begin;                 /**< start time of schedule execution */
    unsigned _execution_time;        /**< time the scheduled job runs */
    bool _does_execute = false; /**< flag determining whether the schedule does run */
    bool _end_known = true;     /**< flag determining whether the end of the schedule is known */

    unsigned end() const;
};

static bool same_data(const ScheduleData &a, const ScheduleData &b) {
    return a._begin == b._begin
           and a._execution_time == b._execution_time;
}


class BaseSchedule {
  protected:
    static unsigned _next_id;

    BaseJob *_job;                      /**< the concerning job */
  public:
    unsigned _id;                            /**< id of this schedule */
    unsigned _submission_time;               /**< time this schedule gets submitted */
    unsigned _core;                     /**< core on wich the job gets executed */

    unsigned _end = -1;                      /**< possible end of this schedule */
    std::set<unsigned> _change_points;       /**< list of points at which changes happen */
    bool _simulation_ended = false;

    BaseSchedule(unsigned id, BaseJob *job, unsigned submission_time, unsigned core);

    BaseSchedule(BaseJob *job, unsigned submission_time, unsigned core)
        : BaseSchedule(BaseSchedule::next_id(), job, submission_time, core) {}

    BaseSchedule(const BaseSchedule *s)
        : BaseSchedule(BaseSchedule::next_id(), s->_job, s->_submission_time, s->_core) {}

    static unsigned next_id();
    static void reset_next_id();

    BaseJob *job() const;

    /**
     * check if the schedule does exist at a given timestamp
     * @param timestamp timestamp to check against
     * @returns True if schedule exists at timestamp. False otherwise.
     */
    bool exists_at_time(unsigned timestamp) const;

    virtual GuiScheduleData get_vision_data_at_time(unsigned timestamp = 0) const = 0;

    virtual unsigned get_maximal_end() const = 0;

};

/** Object representing a schedule for a certain job */
template <typename T>
class Schedule : public BaseSchedule {
    static_assert(std::is_base_of<ScheduleData, T>::value);

protected:
    T &data_at_time(unsigned timestamp) {
        /* find dataset for given timestamp */
        unsigned data_index = this->_submission_time;
        for (std::pair<unsigned, T> d: this->_data) {
            if (timestamp < d.first) {
                break;
            }
            data_index = d.first;
        }
        return this->_data.at(data_index);
    }

    const T &data_at_time(unsigned timestamp) const {
        /* find dataset for given timestamp */
        unsigned data_index = this->_submission_time;
        for (std::pair<unsigned, T> d: this->_data) {
            if (timestamp < d.first) {
                break;
            }
            data_index = d.first;
        }
        return this->_data.at(data_index);
    }

public:
    std::map<unsigned, T> _data;             /**< data for a given time period */

    using BaseSchedule::BaseSchedule;

    Schedule(const Schedule *s)
        : BaseSchedule(s),
          _data(s->_data) {
    }

    virtual ~Schedule() = default;


    /**
     * get relevant data for rendering for a given timestamp
     * @param timestamp timestamp to get data for
     * @returns schedules data at given timestamp
     */
    T get_data_at_time(unsigned timestamp = 0) const {
        return this->data_at_time(timestamp);
    }

    virtual GuiScheduleData get_vision_data_at_time(unsigned timestamp = 0) const override {
        ScheduleData data = this->data_at_time(timestamp);

        if (not data._end_known) {
            if (data._begin > timestamp) {
                data._execution_time = 0;
            } else {
                data._execution_time = std::min(data._execution_time, timestamp - data._begin);
            }
        }
        if (not data._does_execute and timestamp >= data._begin) {
            if (data.end() < timestamp) {
                data._execution_time = 0;
            } else {
                data._execution_time = data.end() - timestamp;
            }
            data._begin = timestamp; // + 1;
        }
        GuiScheduleData gui_data{data._begin, data._execution_time, 0};
        return gui_data;
    }


    /**
     * get first Schedule Data
     * @returns first ScheduleData in _data
     */
    T first_data() const {
        T data = this->_data.begin()->second;
        return data;
    }

    /**
     * get last Schedule Data
     * @returns last ScheduleData in _data
     */
    T last_data() const {
        T data = this->_data.rbegin()->second;
        return data;
    }

    T &last_data() {
        T &data = this->_data.rbegin()->second;
        return data;
    }

    /**
     * Add a change to this schedule
     * @param change change object to add
     */
    void add_change(const ParsedScheduleChange &change) {
        /* get last data before change */
        ScheduleData &old_data = this->data_at_time(change._timestamp);
        ChangeType type = static_cast<ChangeType>(change._type);
        if (type == ChangeType::erase) {
            this->_end = change._timestamp;
            /* add information that the schedule does not run because of unknown dependencies */
            old_data._does_execute = false;
            return;
        }
        /* copy old data */
        ScheduleData new_data = old_data;
        new_data._timestamp = change._timestamp;
        switch (type) {
            case ChangeType::shift:
                new_data._begin = change._value;
                break;
            case ChangeType::change_execution_time:
                new_data._execution_time = change._value;
                break;
            default: break;
        }
        if (not same_data(old_data, new_data)) {
            this->_data.emplace(change._timestamp, new_data);
            this->_change_points.insert(change._timestamp);
        }
    }

    void add_change(unsigned timestamp, unsigned begin, unsigned execution_time) {
        T new_data = this->last_data();
        new_data._timestamp = timestamp;
        new_data._execution_time = execution_time;
        new_data._begin = begin;
        this->_data[timestamp] = new_data;
    }

    void add_change_begin(unsigned timestamp, unsigned begin, bool did_execute = true){
        T &old_data = this->last_data();
        T new_data = old_data;
        new_data._timestamp = timestamp;
        new_data._begin = begin;
        if (not did_execute) {
            old_data._does_execute = false;
        }
        this->_data[timestamp] = new_data;
    }

    void add_change_does_execute(unsigned timestamp, bool does_execute){
        T new_data = this->last_data();
        new_data._timestamp = timestamp;
        new_data._does_execute = does_execute;
        this->_data[timestamp] = new_data;
    }

    void add_change_end_known(unsigned timestamp, bool end_known){
        T new_data = this->last_data();
        new_data._timestamp = timestamp;
        new_data._end_known = end_known;
        this->_data[timestamp] = new_data;
    }

    void add_change_shift_relative(unsigned timestamp, unsigned shift){
        T new_data = this->last_data();
        new_data._timestamp = timestamp;
        new_data._begin += shift;
        this->_data[timestamp] = new_data;
    }

    void add_change_execution_time_relative(unsigned timestamp, int execution_time_difference){
        T new_data = this->last_data();
        new_data._timestamp = timestamp;
        new_data._execution_time += execution_time_difference;
        this->_data[timestamp] = new_data;
    }

    void add_change_end(unsigned timestamp, unsigned end){
        T new_data = this->last_data();
        new_data._timestamp = timestamp;
        if (end < new_data._execution_time) {
            new_data._execution_time = 0;
        } else {
            new_data._execution_time = end - new_data._begin;
        }
        this->_data[timestamp] = new_data;
    }

    void add_change_delete(unsigned timestamp){
        T last_data = this->last_data();

        /* when CFS schedules get deleted there has been an end change before */
        if (last_data._timestamp == timestamp) {
            this->_data.erase(last_data._timestamp);
        }
        T &old_data = this->last_data();

        this->_end = timestamp;
        T new_data = old_data;
        old_data._does_execute = false;
        new_data._execution_time = 0;
        this->_data[timestamp] = new_data;
    }

    void end_simulation(unsigned timestamp){
        this->_simulation_ended = true;
        T data = this->last_data();
        if (data._execution_time == 0 or not data._does_execute) {
            this->_end = timestamp;
        }
    }

    /**
     * check if the schedule gets executed at a given timestamp
     * @param timestamp timestamp to check against
     * @returns True if the schedule gets executed at timestamp. False otherwise.
     */
    bool is_active_at_time(unsigned timestamp) const {
        T data = this->get_data_at_time(timestamp);
        return (data._begin <= timestamp && data._begin + data._execution_time > timestamp);
    }

    /**
     * get the worst-case end timestamp for this schedule
     * @returns worst-case end timestamp
     */
    unsigned get_maximal_end() const {
        auto max_data = std::max_element(this->_data.begin(), this->_data.end(),
            [](const std::pair<unsigned, T> &a, const std::pair<unsigned, T> &b){
                return a.second._begin < b.second._begin;
            });
        ScheduleData data = max_data->second;
        /* I guess the execution time will not get worse for now
         * TODO: check this. */
        return data._begin + data._execution_time;
    }
};

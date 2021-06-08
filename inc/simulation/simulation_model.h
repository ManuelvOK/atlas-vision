#pragma once

#include <list>
#include <map>
#include <vector>
#include <set>

#include <SDL_GUI/models/model_base.h>
#include <SDL_GUI/gui/drawable.h>

#include <job.h>
#include <message.h>
#include <schedule.h>
#include <simulation/simulation_action.h>


class BaseSimulationModel : public SDL_GUI::ModelBase {
public:
    unsigned _timestamp = 0;
    unsigned _n_cores = -1;                      /**< number of cores the jobs get scheduled on */

    std::list<SimulationAction *> _actions_to_do;
    std::list<SimulationAction *> _actions_done;

    std::vector<Message *> _messages;       /**< messages to display at a given timestamp */


    std::map<const SDL_GUI::Drawable *, Message *> _drawables_messages;
    std::map<Message *, SDL_GUI::Drawable *> _messages_underlines;

    bool _dirty = true;
    bool _simulated = false;
    bool _only_simulation = false;

    std::string _output_file = "";

    Message *_hovered_message = nullptr;
    std::vector<std::string> _debug_messages;

    std::set<unsigned> _highlighted_jobs;

    virtual std::vector<BaseSchedule *> schedules() const = 0;

    virtual std::vector<BaseJob *> jobs() const = 0;

    void add_message(unsigned timestamp, std::string text, std::set<unsigned> jobs = std::set<unsigned>());
};

template <typename T>
bool compare_jobs_deadline(const T *a, const T *b) {
    return a->_deadline < b->_deadline;
}


template <typename T>
bool compare_schedules(const T *a, const T *b) {
    int begin_a = a->last_data()._begin;
    int begin_b = b->last_data()._begin;
    if (begin_a == begin_b) {
        return a->_id < b->_id;
    }
    return begin_a < begin_b;
}


template <typename S, typename J>
class SimulationModel : public BaseSimulationModel {
    static_assert(std::is_base_of<BaseSchedule, S>::value);
    static_assert(std::is_base_of<BaseJob, J>::value);
public:
    std::vector<J *> _jobs;               /**< list of jobs */
    /** list of schedules for the jobs */
    std::set<S *, decltype(&compare_schedules<S>)> _schedules;

    std::map<const SDL_GUI::Drawable *, std::set<unsigned>> _drawables_jobs;

    SimulationModel() : _schedules(compare_schedules) {}

    virtual std::vector<BaseSchedule *> schedules() const override {
        return std::vector<BaseSchedule *>(this->_schedules.begin(), this->_schedules.end());
    }

    virtual std::vector<BaseJob *> jobs() const override {
        return std::vector<BaseJob *>(this->_jobs.begin(), this->_jobs.end());
    }

    std::vector<J *> specific_jobs() const {
        return this->_jobs;
    }

    virtual const S *active_schedule(unsigned core, unsigned timestamp) const {
        for (const S *s: this->_schedules) {
            if (s->is_active_at_time(timestamp) and s->_core == core) {
                return s;
            }
        }
        return nullptr;
    }

    virtual void resort_schedules() {
        auto old_schedules = this->_schedules;
        this->_schedules.clear();
        for (S *s: old_schedules) {
            this->_schedules.insert(s);
        }
    }

    virtual void reset_for_simulation() {
        this->_timestamp = 0;

        for (S *schedule: this->_schedules) {
            delete schedule;
        }
        this->_schedules.clear();

        for (J *job: this->_jobs) {
            job->_schedules.clear();
        }
        for (SimulationAction *action: this->_actions_done) {
            delete action;
        }
        this->_actions_done.clear();
    }
};

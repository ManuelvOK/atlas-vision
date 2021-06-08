#include <cbs/constant_bandwidth_server.h>

#include <cbs/cbs_schedule.h>

unsigned ConstantBandwidthServer::id() const {
    return this->_id;
}

unsigned ConstantBandwidthServer::budget(unsigned timestamp) const {
    unsigned last_fill_time = 0;
    for (unsigned fill_time: this->_budget_fill_times) {
        if (fill_time > timestamp) {
            break;
        }
        last_fill_time = fill_time;
    }

    unsigned budget = this->_max_budget;
    for (SoftRtSchedule *schedule: this->_schedules) {
        CbsScheduleData data = schedule->last_data();
        if (data._begin >= timestamp) {
            continue;
        }

        if (data.end() <= last_fill_time) {
            continue;
        }

        budget -= data._execution_time;
        /* schedule begins before last fill */
        if (data._begin < last_fill_time) {
            budget += last_fill_time - data._begin;
        }
        /* schedule ends after timestamp */
        if (data.end() > timestamp) {
            budget += (data.end() - timestamp);
        }
    }
    return budget;
}

unsigned ConstantBandwidthServer::max_budget() const {
    return this->_max_budget;
}

std::set<unsigned> ConstantBandwidthServer::budget_fill_times() const {
    return this->_budget_fill_times;
}

unsigned ConstantBandwidthServer::deadline(unsigned timestamp) const {
    unsigned deadline = 0;
    for (auto [t, dl]: this->_deadlines) {
        if (t > timestamp) {
            break;
        }
        deadline = dl;
    }
    return deadline;
}

SoftRtJob *ConstantBandwidthServer::job() const {
    if (this->_job_queue.empty()) {
        return nullptr;
    }
    return this->_job_queue.front();
}

std::list<SoftRtJob *> ConstantBandwidthServer::job_queue() const {
    return this->_job_queue;
}

std::map<unsigned, unsigned> ConstantBandwidthServer::budget_line() const {
    std::vector<SoftRtSchedule *> schedules(this->_schedules.begin(), this->_schedules.end());
    std::sort(schedules.begin(), schedules.end(),
        [](SoftRtSchedule *a, SoftRtSchedule *b) {
            return a->last_data()._begin < b->last_data()._begin;
        });

    std::map<unsigned, unsigned> budget_line;
    budget_line[0] = this->_max_budget;
    unsigned current_budget = this->_max_budget;
    for (SoftRtSchedule *schedule: schedules) {
        CbsScheduleData data = schedule->last_data();

        /* add point at beginning of schedule */
        unsigned current_time = data._begin;
        budget_line[current_time] = current_budget;

        unsigned execution_time_left = data._execution_time;

        /* add points for filling up */
        while (execution_time_left >= current_budget) {
            current_time += current_budget;
            budget_line[current_time] = 0;

            execution_time_left -= current_budget;
            current_budget = this->_max_budget;
        }

        current_budget -= execution_time_left;
        budget_line[data.end()] = current_budget;
    }

    for (unsigned fill_time: this->_budget_fill_times) {
        budget_line[fill_time] = 0;
    }

    return budget_line;
}

float ConstantBandwidthServer::utilisation() const {
    return static_cast<float>(this->_max_budget) / this->_period;
}

bool ConstantBandwidthServer::is_active() const {
    return not this->_job_queue.empty();
}

void ConstantBandwidthServer::add_schedule(SoftRtSchedule *schedule) {
    this->_schedules.push_back(schedule);
}

unsigned ConstantBandwidthServer::generate_new_deadline_and_refill(unsigned timestamp) {
    unsigned deadline;
    if (this->is_active()) {
        deadline = this->deadline(timestamp) + this->_period;
    } else {
        deadline = timestamp + this->_period;
    }
    this->_deadlines.emplace(timestamp, deadline);
    this->refill_budget(timestamp);
    return deadline;
}

void ConstantBandwidthServer::refill_budget(unsigned timestamp) {
    this->_budgets[timestamp] = this->_max_budget;
    this->_budget_fill_times.insert(timestamp);
}

void ConstantBandwidthServer::dequeue_job(SoftRtJob *job) {
    if (job == this->_job_queue.front() or not job) {
        this->_job_queue.pop_front();
    }
}

void ConstantBandwidthServer::enqueue_job(SoftRtJob *job) {
    this->_job_queue.push_back(job);
}

std::string ConstantBandwidthServer::to_string() const {
    std::stringstream ss;
    ss << "S " << this->_id << " " << this->_max_budget << " " << this->_period << std::endl;
    return ss.str();
}

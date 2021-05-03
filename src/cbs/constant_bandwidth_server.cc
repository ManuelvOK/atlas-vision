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
    unsigned deadline = timestamp + this->_period;
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

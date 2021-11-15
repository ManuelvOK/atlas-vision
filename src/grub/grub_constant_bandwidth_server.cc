#include <grub/grub_constant_bandwidth_server.h>

#include <cmath>
#include <limits>
#include <iostream>

#include <grub/grub_job.h>
#include <grub/grub_schedule.h>

unsigned GrubConstantBandwidthServer::id() const {
    return this->_id;
}

unsigned GrubConstantBandwidthServer::period() const {
    return this->_period;
}

int GrubConstantBandwidthServer::deadline(int timestamp) const {
    int deadline = 0;
    for (auto [t, dl]: this->_deadlines) {
        if (t > timestamp) {
            break;
        }
        deadline = dl;
    }
    return deadline;
}

int GrubConstantBandwidthServer::deadline() const {
    return this->_deadline;
}

void GrubConstantBandwidthServer::set_deadline(int timestamp, int deadline) {
    this->_deadlines[timestamp] = deadline;
    this->_deadline = deadline;
}

float GrubConstantBandwidthServer::virtual_time(int timestamp) const {
    float virtual_time = 0;
    for (auto [t, vt]: this->_virtual_times) {
        if (t > timestamp) {
            break;
        }
        virtual_time = vt;
    }
    return virtual_time;
}
float GrubConstantBandwidthServer::virtual_time() const {
    return this->_virtual_time;
}

void GrubConstantBandwidthServer::set_virtual_time(int timestamp, float virtual_time) {
    this->_virtual_times[timestamp] = virtual_time;
    this->_virtual_time = virtual_time;
    this->_last_virtual_time_update = timestamp;
}

void GrubConstantBandwidthServer::update_virtual_time(int timestamp, float total_utilisation) {
    if (not this->_running) {
        this->set_virtual_time(timestamp, this->_virtual_time);
        return;
    }

    int time_distance = timestamp - this->_last_virtual_time_update;
    float virtual_time_rate = 1.0 * total_utilisation / this->_processor_share;
    float new_virtual_time = this->_virtual_time + time_distance * virtual_time_rate;
    this->set_virtual_time(timestamp, new_virtual_time);

}

int GrubConstantBandwidthServer::next_virtual_time_deadline_miss(float total_utilisation) {
    if (not this->_running) {
        return std::numeric_limits<int>::max();
    }
    int virtual_distance = this->_deadline - this->_virtual_time;
    //std::cout << "vdist: " << virtual_distance << std::endl;
    float virtual_time_rate = 1.0 * total_utilisation / this->_processor_share;
    //std::cout << "vrate: " << virtual_time_rate << std::endl;

    return ceil(this->_last_virtual_time_update + virtual_distance / virtual_time_rate);
}

bool GrubConstantBandwidthServer::running() const {
    return this->_running;
}

void GrubConstantBandwidthServer::set_running(bool running) {
    this->_running = running;
}


SoftGrubJob *GrubConstantBandwidthServer::job() const {
    if (this->_job_queue.empty() or not this->is_active_contending()) {
        return nullptr;
    }
    return this->_job_queue.front();
}

std::map<int, SoftGrubJob *> GrubConstantBandwidthServer::jobs() const {
    return this->_jobs;
}

std::list<SoftGrubJob *> GrubConstantBandwidthServer::job_queue() const {
    return this->_job_queue;
}

float GrubConstantBandwidthServer::processor_share() const {
    return this->_processor_share;
}

bool GrubConstantBandwidthServer::is_active_contending() const {
    return this->_state == GrubState::ACTIVE_CONTENDING;
}

void GrubConstantBandwidthServer::add_schedule(SoftGrubSchedule *schedule) {
    this->_schedules.push_back(schedule);
    SoftGrubJob *job = schedule->_grub_job;
    this->_jobs[job->_submission_time] = job;
}

void GrubConstantBandwidthServer::dequeue_job(SoftGrubJob *job) {
    if (job == this->_job_queue.front() or not job) {
        this->_job_queue.pop_front();
    }
}

void GrubConstantBandwidthServer::enqueue_job(SoftGrubJob *job) {
    this->_job_queue.push_back(job);
}

std::string GrubConstantBandwidthServer::to_string() const {
    std::stringstream ss;
    int budget = this->_period * this->_processor_share;
    ss << "S " << this->_id << " " << budget << " " << this->_period << std::endl;
    return ss.str();
}

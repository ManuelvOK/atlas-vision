#pragma once

#include <list>
#include <map>
#include <set>
#include <vector>

#include <printable.h>

enum class GrubState {
    INACTIVE,
    ACTIVE_CONTENDING,
    ACTIVE_NON_CONTENDING,
};

class SoftGrubJob;
class SoftGrubSchedule;
class GrubConstantBandwidthServer : public Printable {
    const unsigned _id;
    const unsigned _period;
    const float _processor_share;

    std::list<SoftGrubJob *> _job_queue;

    std::map<int, SoftGrubJob *> _jobs;

    std::map<int, int> _deadlines;
    int _deadline;

    std::map<int, float> _virtual_times;
    int _last_virtual_time_update = 0;
    float _virtual_time;

    bool _running = false;

  public:
    GrubState _state = GrubState::INACTIVE;

    std::vector<SoftGrubSchedule *> _schedules;

    SoftGrubSchedule *_active_schedule = nullptr;

    GrubConstantBandwidthServer(unsigned id, unsigned max_budget, unsigned period)
        :_id(id), _period(period), _processor_share(1.0*max_budget/period) {}

    unsigned id() const;

    unsigned period() const;

    int deadline(int timestamp) const;
    int deadline() const;
    void set_deadline(int timestamp, int deadline);

    float virtual_time(int timestamp) const;
    float virtual_time() const;
    void set_virtual_time(int timestamp, float virtual_time);
    void update_virtual_time(int timestamp, float total_utilisation);

    int next_virtual_time_deadline_miss(float total_utilisation);

    bool running() const;
    void set_running(bool running);

    SoftGrubJob *job() const;

    std::map<int, SoftGrubJob *> jobs() const;

    std::list<SoftGrubJob *> job_queue() const;

    float processor_share() const;

    bool is_active_contending() const;

    void add_schedule(SoftGrubSchedule *schedule);

    void dequeue_job(SoftGrubJob *job = nullptr);

    void enqueue_job(SoftGrubJob *job);

    virtual std::string to_string() const override;

};

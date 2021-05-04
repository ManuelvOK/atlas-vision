#pragma once

#include <list>
#include <map>
#include <set>
#include <vector>

class SoftRtJob;
class SoftRtSchedule;
class ConstantBandwidthServer {
    const unsigned _id;
    const unsigned _max_budget;
    const unsigned _period;

    std::map<unsigned, unsigned> _budgets;
    std::set<unsigned> _budget_fill_times;

    std::list<SoftRtJob *> _job_queue;

    std::map<unsigned, unsigned> _deadlines;


  public:
    std::vector<SoftRtSchedule *> _schedules;

    SoftRtSchedule *_active_schedule = nullptr;

    ConstantBandwidthServer(unsigned id, unsigned max_budget, unsigned period)
        :_id(id), _max_budget(max_budget), _period(period) {}

    unsigned id() const;

    unsigned budget(unsigned timestamp) const;

    unsigned max_budget() const;

    std::set<unsigned> budget_fill_times() const;

    unsigned deadline(unsigned timestamp) const;

    SoftRtJob *job() const;

    std::list<SoftRtJob *> job_queue() const;

    std::map<unsigned, unsigned> budget_line() const;

    float utilisation() const;

    bool is_active() const;

    void add_schedule(SoftRtSchedule *schedule);

    unsigned generate_new_deadline_and_refill(unsigned timestamp);

    void refill_budget(unsigned timestamp);

    void dequeue_job(SoftRtJob *job = nullptr);

    void enqueue_job(SoftRtJob *job);
};

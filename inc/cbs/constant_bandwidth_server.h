#pragma once

#include <list>
#include <map>
#include <set>
#include <vector>

#include <printable.h>

class SoftRtJob;
class SoftRtSchedule;
class ConstantBandwidthServer : public Printable {
    const unsigned _id;
    const unsigned _max_budget;
    const unsigned _period;

    std::map<int, unsigned> _budgets;
    std::set<int> _budget_fill_times;

    std::list<SoftRtJob *> _job_queue;

    std::map<int, SoftRtJob *> _jobs;

    std::map<int, int> _deadlines;


  public:
    std::vector<SoftRtSchedule *> _schedules;

    SoftRtSchedule *_active_schedule = nullptr;

    ConstantBandwidthServer(unsigned id, unsigned max_budget, unsigned period)
        :_id(id), _max_budget(max_budget), _period(period) {}

    unsigned id() const;

    unsigned budget(int timestamp) const;

    unsigned max_budget() const;

    unsigned period() const;

    std::set<int> budget_fill_times() const;

    int deadline(int timestamp) const;

    SoftRtJob *job() const;

    std::map<int, SoftRtJob *> jobs() const;

    std::list<SoftRtJob *> job_queue() const;

    std::map<int, unsigned> budget_line() const;

    float utilisation() const;

    bool is_active() const;

    void add_schedule(SoftRtSchedule *schedule);

    int generate_new_deadline_and_refill(int timestamp);

    void refill_budget(int timestamp);

    void dequeue_job(SoftRtJob *job = nullptr);

    void enqueue_job(SoftRtJob *job);

    virtual std::string to_string() const override;
};

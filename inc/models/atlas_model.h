#pragma once

#include <models/cfs_visibility.h>
#include <models/simulation_model.h>

/** Model for all the data related to the ATLAS schedule */
class AtlasModel : public SimulationModel {
    void tidy_up_queue(std::list<Job *> *queue);
public:
    int _cfs_factor = 1;                    /**< time factor when running on CFS */


    std::set<AtlasSchedule *, decltype(&compare_schedules)> _atlas_schedules;
    std::vector<CfsSchedule *> _cfs_schedules;
    std::vector<EarlyCfsSchedule *> _early_cfs_schedules;
    std::vector<RecoverySchedule *> _recovery_schedules;

    std::map<unsigned, std::list<Job *>> _cfs_queue;
    std::map<unsigned, CfsSchedule *> _cfs_schedule;
    std::map<unsigned, std::list<Job *>> _recovery_queue;
    std::map<unsigned, RecoverySchedule *> _recovery_schedule;


    AtlasModel();

    void add_atlas_schedule(AtlasSchedule *schedule);
    void add_recovery_schedule(RecoverySchedule *schedule);
    void add_cfs_schedule(CfsSchedule *schedule);
    void add_early_cfs_schedule(EarlyCfsSchedule *schedule);

    const Schedule *active_schedule_on_scheduler(unsigned core, SchedulerType scheduler, int timestamp) const;
    AtlasSchedule *next_atlas_schedule(unsigned core) const;
    std::vector<AtlasSchedule *> next_atlas_schedules(unsigned core) const;
    std::vector<Job *> next_atlas_scheduled_jobs(unsigned core) const;

    void tidy_up_queues();
    void resort_schedules() override;

};

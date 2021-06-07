#pragma once

#include <atlas/atlas_job.h>
#include <atlas/atlas_schedule.h>
#include <simulation/simulation_model.h>

class CfsVisibility;
/** Model for all the data related to the ATLAS schedule */
class AtlasSimulationModel : public SimulationModel<BaseAtlasSchedule, AtlasJob> {
    void tidy_up_queue(std::list<AtlasJob *> *queue);
public:
    int _cfs_factor = 1;                    /**< time factor when running on CFS */


    std::set<AtlasSchedule *, decltype(&compare_schedules<BaseAtlasSchedule>)> _atlas_schedules;
    std::vector<CfsSchedule *> _cfs_schedules;
    std::vector<EarlyCfsSchedule *> _early_cfs_schedules;
    std::vector<RecoverySchedule *> _recovery_schedules;

    std::map<unsigned, std::list<AtlasJob *>> _cfs_queue;
    std::map<unsigned, CfsSchedule *> _cfs_schedule;
    std::map<unsigned, std::list<AtlasJob *>> _recovery_queue;
    std::map<unsigned, RecoverySchedule *> _recovery_schedule;


    AtlasSimulationModel();

    void add_atlas_schedule(AtlasSchedule *schedule);
    void add_recovery_schedule(RecoverySchedule *schedule);
    void add_cfs_schedule(CfsSchedule *schedule);
    void add_early_cfs_schedule(EarlyCfsSchedule *schedule);

    const BaseAtlasSchedule *active_schedule_on_scheduler(unsigned core,
                                                          AtlasSchedulerType scheduler,
                                                          int timestamp) const;
    AtlasSchedule *next_atlas_schedule(unsigned core) const;
    std::vector<AtlasSchedule *> next_atlas_schedules(unsigned core) const;
    std::vector<AtlasJob *> next_atlas_scheduled_jobs(unsigned core) const;

    unsigned space_on_atlas(unsigned begin, unsigned end) const;

    void tidy_up_queues();
    void resort_schedules() override;

};

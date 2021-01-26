#pragma once

#include <vector>
#include <map>
#include <set>

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/models/model_base.h>

#include <models/cfs_visibility.h>
#include <models/job.h>
#include <models/message.h>
#include <models/schedule.h>
#include <models/simulation_action.h>

bool compare_schedules(const Schedule *a, const Schedule *b);

/** Model for all the data related to the ATLAS schedule */
class AtlasModel : public SDL_GUI::ModelBase {
    void tidy_up_queue(std::list<Job *> *queue);
public:
    int _n_cores = -1;                      /**< number of cores the jobs get scheduled on */
    int _cfs_factor = 1;                    /**< time factor when running on CFS */
    std::vector<Job *> _jobs;               /**< list of jobs */

    /** list of schedules for the jobs */
    std::set<Schedule *, decltype(&compare_schedules)> _schedules;
    int _timestamp = 0;

    std::vector<Message *> _messages;       /**< messages to display at a given timestamp */

    std::set<AtlasSchedule *, decltype(&compare_schedules)> _atlas_schedules;
    std::vector<CfsSchedule *> _cfs_schedules;
    std::vector<RecoverySchedule *> _recovery_schedules;
    std::list<SimulationAction *> _actions_to_do;
    std::list<SimulationAction *> _actions_done;

    std::list<Job *> _cfs_queue;
    CfsSchedule *_cfs_schedule;
    std::list<Job *> _recovery_queue;
    RecoverySchedule *_recovery_schedule;

    int _highlighted_job = -1;
    std::map<const SDL_GUI::Drawable *, const Job *> _drawables_jobs;

    bool _dirty = true;
    bool _simulated = false;

    /** list of atlas schedules visibile for cfs scheduler */
    std::vector<CfsVisibility *> _cfs_visibilities;

    AtlasModel();

    void add_atlas_schedule(AtlasSchedule *schedule);

    void add_message(int timestamp, std::string text);

    const Schedule *active_schedule(int timestamp) const;
    const Schedule *active_schedule_on_scheduler(SchedulerType scheduler, int timestamp) const;
    AtlasSchedule *next_atlas_schedule() const;
    std::vector<AtlasSchedule *> next_atlas_schedules() const;
    std::vector<Job *> next_atlas_scheduled_jobs() const;
    void tidy_up_queues();

    void reset_for_simulation();

};

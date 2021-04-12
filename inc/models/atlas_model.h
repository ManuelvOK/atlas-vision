#pragma once

#include <vector>
#include <map>
#include <set>

#include <SDL_GUI/gui/drawable.h>
#include <SDL_GUI/models/model_base.h>

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
    std::vector<EarlyCfsSchedule *> _early_cfs_schedules;
    std::vector<RecoverySchedule *> _recovery_schedules;
    std::list<SimulationAction *> _actions_to_do;
    std::list<SimulationAction *> _actions_done;

    std::map<unsigned, std::list<Job *>> _cfs_queue;
    std::map<unsigned, CfsSchedule *> _cfs_schedule;
    std::map<unsigned, std::list<Job *>> _recovery_queue;
    std::map<unsigned, RecoverySchedule *> _recovery_schedule;

    std::set<int> _highlighted_jobs;
    std::map<const SDL_GUI::Drawable *, std::set<int>> _drawables_jobs;
    std::map<const SDL_GUI::Drawable *, Message *> _drawables_messages;
    std::map<Message *, SDL_GUI::Drawable *> _messages_underlines;

    bool _dirty = true;
    bool _simulated = false;

    Message *_hovered_message = nullptr;
    std::vector<std::string> _debug_messages;

    AtlasModel();

    void add_atlas_schedule(AtlasSchedule *schedule);
    void add_recovery_schedule(RecoverySchedule *schedule);
    void add_cfs_schedule(CfsSchedule *schedule);
    void add_early_cfs_schedule(EarlyCfsSchedule *schedule);

    void add_message(int timestamp, std::string text, std::set<int> jobs = std::set<int>());

    const Schedule *active_schedule(unsigned core, int timestamp) const;
    const Schedule *active_schedule_on_scheduler(unsigned core, SchedulerType scheduler, int timestamp) const;
    AtlasSchedule *next_atlas_schedule(unsigned core) const;
    std::vector<AtlasSchedule *> next_atlas_schedules(unsigned core) const;
    std::vector<Job *> next_atlas_scheduled_jobs(unsigned core) const;

    void tidy_up_queues();
    void resort_schedules();

    void reset_for_simulation();

};

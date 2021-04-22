#pragma once

#include <list>
#include <map>
#include <vector>
#include <set>

#include <SDL_GUI/models/model_base.h>
#include <SDL_GUI/gui/drawable.h>

#include <models/job.h>
#include <models/message.h>
#include <models/schedule.h>
#include <models/simulation_action.h>

bool compare_schedules(const Schedule *a, const Schedule *b);

class SimulationModel : public SDL_GUI::ModelBase {

public:
    int _n_cores = -1;                      /**< number of cores the jobs get scheduled on */
    std::vector<Job *> _jobs;               /**< list of jobs */

    /** list of schedules for the jobs */
    std::set<Schedule *, decltype(&compare_schedules)> _schedules;
    int _timestamp = 0;

    std::list<SimulationAction *> _actions_to_do;
    std::list<SimulationAction *> _actions_done;

    std::vector<Message *> _messages;       /**< messages to display at a given timestamp */


    std::set<int> _highlighted_jobs;
    std::map<const SDL_GUI::Drawable *, std::set<int>> _drawables_jobs;
    std::map<const SDL_GUI::Drawable *, Message *> _drawables_messages;
    std::map<Message *, SDL_GUI::Drawable *> _messages_underlines;

    bool _dirty = true;
    bool _simulated = false;

    Message *_hovered_message = nullptr;
    std::vector<std::string> _debug_messages;


    SimulationModel();

    void add_message(int timestamp, std::string text, std::set<int> jobs = std::set<int>());

    const Schedule *active_schedule(unsigned core, int timestamp) const;

    virtual void resort_schedules();

    void reset_for_simulation();

};

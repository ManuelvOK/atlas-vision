#pragma once

#include <vector>
#include <map>

#include <SDL_GUI/inc/models/model_base.h>

#include <models/cfs_visibility.h>
#include <models/job.h>
#include <models/message.h>
#include <models/schedule.h>

class AtlasModel : public SDL_GUI::ModelBase {
public:
    int _n_cores = -1; /**< number of cores the jobs get scheduled on */
    std::vector<Job *> _jobs; /**< list of jobs */
    std::map<int, Schedule *> _schedules; /**< list of schedules for the jobs */
    std::vector<CfsVisibility *> _cfs_visibilities; /**< list of atlas schedules visibile for cfs scheduler */
    std::vector<Message *> _messages; /**< messages to display at a given timestamp */

};

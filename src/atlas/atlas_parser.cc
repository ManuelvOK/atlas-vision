#include <atlas/atlas_parser.h>

#include <iostream>

#include <atlas/atlas_job.h>
#include <atlas/atlas_simulation_model.h>

void AtlasParser::parse_line(std::string line, AtlasSimulationModel *atlas_model) {
    std::stringstream ss(line);
    char type = ' ';
    /* first char in each line specifies type of line to parse */
    ss >> type;
    switch (type) {
        case 'c': this->parse_n_cores(&ss);          break;
        case 'f': this->parse_cfs_factor(&ss);       break;
        case 'j': this->parse_job(&ss, atlas_model); break;
        case 's': this->parse_schedule(&ss);         break;
        case 'a': this->parse_change(&ss);           break;
        case 'v': this->parse_cfs_visibility(&ss);   break;
        case 'm': this->parse_message(&ss);          break;
        case 'd': this->parse_dependency(&ss);       break;
        case ' ':
        case 0:
        case '#': break;
        default: std::cerr << "Parse error: \"" << type << "\" is not a proper type."
                           << std::endl; break;
    }
}


void AtlasParser::parse_cfs_factor(std::stringstream *line) {
    unsigned factor = 1;
    *line >> factor;
    this->_cfs_factor = factor;
}

void AtlasParser::parse_job(std::stringstream *line, AtlasSimulationModel *atlas_model) {
    unsigned id, deadline, time_estimate, time, submission;
    *line >> id >> deadline >> time_estimate >> time >> submission;
    AtlasJob *job = new AtlasJob(atlas_model, id, deadline, time_estimate, time, submission);
    this->_jobs.emplace(id, job);
}

void AtlasParser::parse_schedule(std::stringstream *line) {
    unsigned id, job_id, core;
    unsigned submission_time, begin, time;
    char scheduler;
    *line >> id >> job_id >> core >> scheduler >> submission_time >> begin >> time;
    this->_parsed_schedules.emplace_back(id, job_id, core, scheduler, submission_time, begin, time);
}

void AtlasParser::parse_change(std::stringstream *line) {
    unsigned schedule_id;
    unsigned timestamp;
    int value = -1;
    char type;
    *line >> type >> timestamp >> schedule_id;
    if (static_cast<ChangeType>(type) != ChangeType::erase) {
        *line >> value;
    }
    this->_changes[schedule_id].emplace_back(schedule_id, timestamp, type, value);
}

void AtlasParser::parse_cfs_visibility(std::stringstream *line) {
    unsigned schedule_id;
    unsigned begin, end;
    *line >> schedule_id >> begin >> end;
    this->_visibilities.emplace_back(schedule_id, begin, end);
}

void AtlasParser::parse_dependency(std::stringstream *line) {
    char type;
    unsigned job1;
    unsigned job2;
    *line >> type >> job1 >> job2;
    this->_dependencies.emplace_back(type, job1, job2);
}

void AtlasParser::parse(std::istream *input, AtlasSimulationModel *atlas_model) {
    /* parse input. Jobs get directly created */
    std::string line;
    while (std::getline(*input, line)) {
        this->parse_line(line, atlas_model);
    }

    atlas_model->_n_cores = this->_n_cores;
    atlas_model->_cfs_factor = this->_cfs_factor;

    /* add dependencies to jobs */
    for (const ParsedDependency &d: this->_dependencies) {
        /* runtime check if referenced job exists with at() */
        AtlasJob *dependent_job = this->_jobs.at(d._dependent_job_id);
        AtlasJob *dependency_job = this->_jobs.at(d._dependency_id);
        if (d._type == 'k') {
            dependent_job->add_known_dependency(dependency_job);
        } else {
            dependent_job->add_unknown_dependency(dependency_job);
        }
    }

    /* calculate jobs dependency level and put them into model */
    for (const std::pair<unsigned, AtlasJob *> &p: this->_jobs) {
        p.second->calculate_dependency_level();
        atlas_model->_jobs.push_back(p.second);
    }

    /* sort jobs id wise */
    std::sort(atlas_model->_jobs.begin(), atlas_model->_jobs.end(),
              [](const AtlasJob *a, const AtlasJob *b) -> bool {return a->_id < b->_id;});

    /* create Schedules, apply changes and put them into model */
    for (const ParsedAtlasSchedule &s: this->_parsed_schedules) {
        const std::map<char, AtlasSchedulerType> scheduler_map = {
            {'a', AtlasSchedulerType::ATLAS},
            {'r', AtlasSchedulerType::recovery},
            {'c', AtlasSchedulerType::CFS},
        };
        /* runtime check if referenced job exists with at() */
        AtlasJob *job = this->_jobs.at(s._job_id);
        BaseAtlasSchedule *schedule = new BaseAtlasSchedule(s._id, job, s._submission_time, s._core,
                                                            scheduler_map.at(s._scheduler),
                                                            s._begin, s._execution_time);
        /* apply changes */
        for (const ParsedScheduleChange &c: this->_changes[s._id]) {
            schedule->add_change(c);
        }
        this->_schedules.emplace(s._id, schedule);
        atlas_model->_schedules.insert(schedule);
    }

    /* create visibilities and put them into model */
    for (const ParsedVisibility &v: this->_visibilities) {
        /* runtime check if referenced schedule exists with at() */
        BaseAtlasSchedule *schedule = this->_schedules.at(v._schedule_id);
        (void) schedule;
        //CfsVisibility *visibility = new CfsVisibility(schedule, v._begin, v._end);
        //atlas_model->_cfs_visibilities.push_back(visibility);
    }

    atlas_model->_messages = this->_messages;
}

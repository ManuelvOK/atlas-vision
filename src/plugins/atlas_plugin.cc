#include <plugins/atlas_plugin.h>

#include <fstream>
#include <string>


#include <util/parser.h>

AtlasModel *AtlasPlugin::atlas_model_from_file(std::string path) const {
    std::ifstream input_file(path);
    if (not input_file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    AtlasModel *model = parse_file(&input_file);
    input_file.close();
    return model;
}

AtlasModel *AtlasPlugin::atlas_model_from_stdin() const {
    return parse_file(&std::cin);
}

AtlasModel *AtlasPlugin::parse_file(std::istream *input) const {
    Parser parser;
    AtlasModel *model = new AtlasModel();
    std::string line;
    while (std::getline(*input, line)) {
        parser.parse_line(line);
    }

    bool succ = true;
    model->_n_cores = parser._n_cores;
    model->_jobs = parser._jobs;
    model->_schedules = parser._schedules;
    model->_cfs_visibilities = parser._cfs_visibilities;
    model->_messages = parser._messages;

    for (std::tuple<int, int, bool> d: parser._dependencies) {
        int from = std::get<0>(d);
        int to = std::get<1>(d);
        bool known = std::get<2>(d);
        if (model->_jobs.size() < std::max(from, to)) {
            std::cerr << "Error parsing dependency \"d " << from << " " << to
                      << "\": job vector has size of " << model->_jobs.size() << std::endl;
        }
        if (known) {
            model->_jobs[from]->_known_dependencies.push_back(model->_jobs[to]);
        } else {
            model->_jobs[from]->_unknown_dependencies.push_back(model->_jobs[to]);

        }
    }
    /* apply changes */
    for (const ScheduleChange *change: parser._changes) {
        succ = succ && this->apply_schedule_change(model, change);
    }

    /* calculate dependency level */
    for (Job *job: model->_jobs) {
        job->calculate_dependency_level();
    };

    return model;
}

bool AtlasPlugin::apply_schedule_change(AtlasModel *model, const ScheduleChange *change) const {
    if (static_cast<unsigned>(change->_schedule_id) >= model->_schedules.size()) {
        std::cerr << "input error: validity\t- There is no Schedule with id "
                  << change->_schedule_id << " to change" << std::endl;
    }
    Schedule *schedule = model->_schedules.at(change->_schedule_id);
    switch (change->_type) {
        case ChangeType::erase:
            schedule->_end = change->_timestamp;
            break;
        case ChangeType::shift:
            schedule->_begin.emplace(change->_timestamp, change->_value);
            break;
        case ChangeType::change_execution_time:
            schedule->_execution_time.emplace(change->_timestamp, change->_value);
            break;
        default: break;
    }
    schedule->_change_points.insert(change->_timestamp);
    return true;
}

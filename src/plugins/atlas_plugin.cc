#include <plugins/atlas_plugin.h>

#include <fstream>
#include <string>

#include <util/parser.h>


AtlasPlugin::AtlasPlugin(): SDL_GUI::PluginBase("Atlas Plugin") {
    this->_command_line.register_option("simulate", "s");
    this->_command_line.register_option("visualise", "v");
}

AtlasModel *AtlasPlugin::build_atlas_model() const {
    AtlasModel *model;
    std::string vision_input = this->_command_line.get_argument("visualise");
    std::string simulation_input = this->_command_line.get_argument("simulate");
    if (vision_input.size() and simulation_input.size()) {
        std::cerr << "Both simulation and visualisation input given. Using visualisation input"
                  << std::endl;
    }
    if (vision_input.size()) {
        model = this->atlas_model_from_file(vision_input);
        model->_simulated = true;
    } else if (simulation_input.size()) {
        model = this->atlas_model_from_file(simulation_input);
    } else {
        std::cerr << "No input given." << std::endl;
        exit(1);
    }
    /* sort jobs id wise */
    std::sort(model->_jobs.begin(), model->_jobs.end(),
              [](const Job *a, const Job *b) -> bool {return a->_id < b->_id;});

    return model;
}

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
    parser._atlas_model = model;
    std::string line;
    while (std::getline(*input, line)) {
        parser.parse_line(line);
    }


    bool succ = true;
    model->_n_cores = parser._n_cores;
    model->_cfs_factor = parser._cfs_factor;
    model->_jobs = parser._jobs;
    for (auto p: parser._schedules) {
        Schedule *schedule = p.second;
        /* TODO: what happens if the parsed schedules have a missing id? */
        schedule->_job = model->_jobs[schedule->_job_id];
        model->_schedules.push_back(schedule);
    }
    model->_cfs_visibilities = parser._cfs_visibilities;
    model->_messages = parser._messages;

    for (std::tuple<int, int, bool> d: parser._dependencies) {
        int from = std::get<0>(d);
        int to = std::get<1>(d);
        bool known = std::get<2>(d);
        if (static_cast<long>(model->_jobs.size()) < std::max(from, to)) {
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
        return false;
    }
    Schedule *schedule = model->_schedules.at(change->_schedule_id);
    schedule->add_change(change);
    return true;
}

#include <application.h>

#include <iostream>
#include <string>

#include <SDL_GUI/inc/controllers/keyboard_input_controller.h>
#include <SDL_GUI/inc/models/keyboard_input_model.h>
#include <SDL_GUI/inc/views/interface_view.h>

#include <config/input_config.h>

#include <controllers/atlas_controller.h>
#include <controllers/interface_controller.h>
#include <controllers/mouse_input_controller.h>
#include <controllers/player_controller.h>

#include <models/interface_model.h>
#include <models/mouse_input_model.h>
#include <models/player_model.h>
#include <models/player_view_model.h>

#include <util/parser.h>


Application::Application(std::string application_title, int argc, char *argv[]) : SDL_GUI::Application(application_title) {
    if (argc > 1) {
        this->_atlas_model = this->atlas_model_from_file(argv[1]);
    } else {
        this->_atlas_model = this->atlas_model_from_stdin();
    }
    /* sort jobs id wise */
    std::sort(this->_atlas_model->_jobs.begin(), this->_atlas_model->_jobs.end(),
            [](const Job *a, const Job *b) -> bool {return a->_id < b->_id;});
}

AtlasModel *Application::atlas_model_from_file(std::string path) const {
    std::ifstream input_file(path);
    if (not input_file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    AtlasModel *model = parse_file(&input_file);
    input_file.close();
    return model;
}

AtlasModel *Application::atlas_model_from_stdin() const {
    return parse_file(&std::cin);
}

AtlasModel *Application::parse_file(std::istream *input) const {
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

    for (std::pair<int, int> d: parser._dependencies) {
        if (model->_jobs.size() < std::max(d.first, d.second)) {
            std::cerr << "Error parsing dependency \"d " << d.first << " " << d.second << "\": job vector has size of " << model->_jobs.size() << std::endl;
        }
        model->_jobs[d.first]->_known_dependencies.push_back(model->_jobs[d.second]);
    }
    /* apply changes */
    for (const ScheduleChange *change: parser._changes) {
        succ = succ && this->apply_schedule_change(model, change);
    }

    /* calculate dependency level */
    for (Job *job: model->_jobs) {
        job->calculate_dependency_level();
        std::cerr << "dependency level of job " << job->_id << ": " << job->_dependency_level << std::endl;
    };

    return model;
}

bool Application::apply_schedule_change(AtlasModel *model, const ScheduleChange *change) const {
    if (static_cast<unsigned>(change->_schedule_id) >= model->_schedules.size()) {
        std::cerr << "input error: validity\t- There is no Schedule with id " << change->_schedule_id
                  << " to change" << std::endl;
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
    return true;
}

void Application::init_MVCs() {
    /**********
     * Models *
     **********/
    /* init interface model */
    InterfaceModel *interface_model = new InterfaceModel();
    this->_model_list.push_back(interface_model);

    /* init keyboard input model */
    this->_keyboard_input_model = new SDL_GUI::KeyboardInputModel<InputKey>();
    this->_model_list.push_back(this->_keyboard_input_model);

    /* init keyboard input model */
    MouseInputModel *mouse_input_model = new MouseInputModel();
    this->_model_list.push_back(mouse_input_model);

    /* init atlas model */
    AtlasModel *atlas_model = new AtlasModel();
    this->_model_list.push_back(atlas_model);

    /* init player model */
    PlayerModel *player_model = new PlayerModel();
    this->_model_list.push_back(player_model);

    /* init player view model */
    PlayerViewModel *player_view_model = new PlayerViewModel();
    this->_model_list.push_back(player_view_model);

    /***************
     * Controllers *
     ***************/
    /* init keyboard input controller */
    SDL_GUI::KeyboardInputController<InputKey> *keyboard_input_controller = new SDL_GUI::KeyboardInputController<InputKey>(this->_keyboard_input_model, input_config);
    this->_controller_list.push_back(keyboard_input_controller);

    /* init mouse input controller */
    MouseInputController *mouse_input_controller = new MouseInputController();
    mouse_input_controller->set_model(mouse_input_model);
    this->_controller_list.push_back(mouse_input_controller);

    /* init interface controller */
    InterfaceController *interface_controller = new InterfaceController("./templates/main.tpl", interface_model, mouse_input_model, player_model);
    this->_controller_list.push_back(interface_controller);

    /* init ATLAS Controller */
    AtlasController *atlas_controller = new AtlasController(atlas_model);
    this->_controller_list.push_back(atlas_controller);

    /* init player controller */
    PlayerController *player_controller = new PlayerController(player_model, player_view_model, this->_keyboard_input_model, mouse_input_model, atlas_model, interface_model);
    this->_controller_list.push_back(player_controller);

    /********
     * View *
     ********/
    /* init interface view */
    SDL_GUI::InterfaceView *interface_view = new SDL_GUI::InterfaceView(this->_renderer, interface_model);
    this->_view_list.push_back(interface_view);

}

void Application::update_running() {
    if (this->_keyboard_input_model->is_down(InputKey::QUIT)) {
        this->_is_running = false;
    }
}

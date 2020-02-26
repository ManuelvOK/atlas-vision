#include <controllers/player_controller.h>

#include <algorithm>
#include <utility>

#include <iostream>

#include <SDL_GUI/inc/gui/rgb.h>

#include <gui/arrow.h>

PlayerController::PlayerController(PlayerModel *player_model, PlayerViewModel *player_view_model,
        const SDL_GUI::KeyboardInputModel<InputKey> *keyboard_input_model, const MouseInputModel *mouse_input_model,
        const AtlasModel *atlas_model,
        InterfaceModel *interface_model) :
    _player_model(player_model),
    _player_view_model(player_view_model),
    _keyboard_input_model(keyboard_input_model),
    _mouse_input_model(mouse_input_model),
    _interface_model(interface_model) {
    this->init(atlas_model);
}

void PlayerController::update() {
    this->evaluate_input();
    this->_player_model->tick();
}

void PlayerController::evaluate_input() {
    if (this->_keyboard_input_model->is_down(InputKey::PLAYER_TOGGLE)) {
        this->_player_model->toggle();
    }
    if (this->_keyboard_input_model->is_down(InputKey::PLAYER_REWIND)) {
        this->_player_model->rewind();
    }
    if (this->_keyboard_input_model->is_down(InputKey::PLAYER_FORWARDS)) {
        this->_player_model->set(this->_player_model->_position + 10);
    }
    if (this->_keyboard_input_model->is_down(InputKey::PLAYER_BACKWARDS)) {
        this->_player_model->set(this->_player_model->_position - 10);
    }
}

void PlayerController::init(const AtlasModel *atlas_model) {
    /* set max position */
    int max_position = 0;
    for (std::pair<int, Schedule *> p: atlas_model->_schedules) {
        const Schedule *s = p.second;
        max_position = std::max(max_position, s->get_maximal_end());
    }
    max_position = (max_position / 20 + 1) * 20;
    this->_player_model->_max_position = max_position;

    /* get interface tree */
    SDL_GUI::Tree<SDL_GUI::Drawable> *tree = this->_interface_model->drawable_tree();

    /* bind player position line to the models variable */
    std::vector<SDL_GUI::TreeNode<SDL_GUI::Drawable> *> player_position_line = tree->filter([](SDL_GUI::Drawable *d){return d->has_attribute("player_position_line");});
    const PlayerModel *player_model = this->_player_model;
    const InterfaceModel *interface_model = this->_interface_model;
    player_position_line[0]->node()->add_recalculation_callback([player_model, interface_model](SDL_GUI::Drawable *d){
            d->set_x(interface_model->px_width(player_model->_position));
        });

    /* initialise the viewmodel of the player */
    /* TODO: The shift_x value should be directly changed inside the interface model */
    std::vector<SDL_GUI::TreeNode<SDL_GUI::Drawable> *> player = tree->filter([](SDL_GUI::Drawable *d){return d->has_attribute("player");});
    const PlayerViewModel *player_view_model = this->_player_view_model;
    player[0]->node()->add_recalculation_callback([player_view_model](SDL_GUI::Drawable *d) {
            d->set_scroll_position_x(-player_view_model->_shift_x);
        });

    this->_player_view_model->_shift_x_max = this->_interface_model->get_player_width_px();

    this->create_sub_and_dl_arrows(atlas_model);
}

void PlayerController::create_sub_and_dl_arrows(const AtlasModel *atlas_model) {
    std::map<int, std::vector<int>> submissions;
    int max_submissions = 0;
    std::map<int, std::vector<int>> deadlines;
    int max_deadlines = 0;
    for (const Job *job: atlas_model->_jobs) {
        /* subs */
        submissions[job->_submission_time].push_back(job->_id);
        max_submissions = std::max(max_submissions, static_cast<int>(submissions[job->_submission_time].size()));

        /* dls */
        deadlines[job->_deadline].push_back(job->_id);
        max_deadlines = std::max(max_deadlines, static_cast<int>(deadlines[job->_deadline].size()));
    };

    SDL_GUI::Tree<SDL_GUI::Drawable> *tree = this->_interface_model->drawable_tree();
    SDL_GUI::TreeNode<SDL_GUI::Drawable> *deadline_rect = tree->filter([](SDL_GUI::Drawable *d){return d->has_attribute("deadline");})[0];

    /* create subs */
    for (std::pair<int, std::vector<int>> submissions_at_time: submissions) {
        int submission_position_x = this->_interface_model->px_width(submissions_at_time.first);

        /* TODO: get rid of magic number */
        int offset = deadline_rect->node()->height() - 7 * (submissions_at_time.second.size() - 1);
        for (int job_id: submissions_at_time.second) {
            Arrow *a = new Arrow({submission_position_x, offset - 1});
            a->_default_style._color = SDL_GUI::RGB("white");//this->_viewmodel->get_color(job);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }

    /* create deadlines */
    for (std::pair<int, std::vector<int>> deadlines_at_time: deadlines) {
        int deadline_position_x = this->_interface_model->px_width(deadlines_at_time.first);

        /* TODO: get rid of magic number */
        int offset = 7 * (deadlines_at_time.second.size() - 1);
        for (int job: deadlines_at_time.second) {
            Arrow *a = new Arrow({deadline_position_x, offset-1}, Arrow::direction::DOWN);
            a->_default_style._color = SDL_GUI::RGB("white");//this->_viewmodel->get_color(job);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset -= 7;
        }
    }

}

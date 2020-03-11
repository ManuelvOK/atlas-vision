#include <controllers/player_controller.h>

#include <algorithm>
#include <utility>

#include <iostream>

PlayerController::PlayerController(PlayerModel *player_model, PlayerViewModel *player_view_model,
        const SDL_GUI::InputModel<InputKey> *input_model, const AtlasModel *atlas_model,
        InterfaceModel *interface_model) :
    _player_model(player_model),
    _player_view_model(player_view_model),
    _input_model(input_model),
    _interface_model(interface_model) {
    this->init(atlas_model);
}

void PlayerController::update() {
    this->evaluate_input();
    this->_player_model->tick();
}

void PlayerController::evaluate_input() {
    if (this->_input_model->is_down(InputKey::PLAYER_TOGGLE)) {
        this->_player_model->toggle();
    }
    if (this->_input_model->is_down(InputKey::PLAYER_REWIND)) {
        this->_player_model->rewind();
    }
    if (this->_input_model->is_down(InputKey::PLAYER_FORWARDS)) {
        this->_player_model->set(this->_player_model->_position + 1000);
    }
    if (this->_input_model->is_down(InputKey::PLAYER_BACKWARDS)) {
        this->_player_model->set(this->_player_model->_position - 1000);
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_SCROLL_LEFT)) {
        this->_interface_model->find_first_drawable("player")->scroll_left();
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_SCROLL_RIGHT)) {
        this->_interface_model->find_first_drawable("player")->scroll_right();
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

    /* bind player position line to the models variable */
    SDL_GUI::Drawable * player_position_line = this->_interface_model->find_first_drawable("player_position_line");
    const PlayerModel *player_model = this->_player_model;
    const InterfaceModel *interface_model = this->_interface_model;
    player_position_line->add_recalculation_callback([player_model, interface_model](SDL_GUI::Drawable *d){
            d->set_x(interface_model->px_width(player_model->_position));
        });

    /* set maximum scroll  */
    SDL_GUI::Drawable *player = this->_interface_model->find_first_drawable("player");
    player->enable_scrolling_x();
    player->set_overscroll_x(1000);
}

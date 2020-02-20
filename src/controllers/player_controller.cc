#include <controllers/player_controller.h>

#include <utility>

PlayerController::PlayerController(PlayerModel *player_model,
        const KeyboardInputModel *keyboard_input_model, const MouseInputModel *mouse_input_model,
        const AtlasModel *atlas_model) :
    _player_model(player_model),
    _keyboard_input_model(keyboard_input_model),
    _mouse_input_model(mouse_input_model) {
    this->init(atlas_model);
}

void PlayerController::update() {
    if (this->_keyboard_input_model->is_pressed(KeyboardInputModel::Key::PLAYER_TOGGLE)) {
        this->_player_model->toggle();
    }
    if (this->_keyboard_input_model->is_pressed(KeyboardInputModel::Key::PLAYER_REWIND)) {
        this->_player_model->rewind();
    }
    if (this->_keyboard_input_model->is_pressed(KeyboardInputModel::Key::PLAYER_FORWARDS)) {
        this->_player_model->set(this->_player_model->_position + 10);
    }
    if (this->_keyboard_input_model->is_pressed(KeyboardInputModel::Key::PLAYER_BACKWARDS)) {
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
}

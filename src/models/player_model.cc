#include <models/player_model.h>

#include <iostream>

void PlayerModel::toggle() {
    this->_running = !this->_running;
}

void PlayerModel::rewind() {
    this->_running = 0;
    this->_position = 0;
}

void PlayerModel::set(int position) {
    this->_position = position;
    /* check if over end of simulation */
    if (this->_position > this->_max_position) {
        this->_position = this->_max_position;
        this->_running = false;
    }
    /* check if before start of simulation */
    if (this->_position < 0) {
        this->_position = 0;
    }
}

void PlayerModel::tick() {
    if (!this->_running) {
        return;
    }
    this->_position += this->_speed;
    /* check if at end of simulation */
    if (this->_position > this->_max_position) {
        this->_position = this->_max_position;
        this->_running = false;
    }
}


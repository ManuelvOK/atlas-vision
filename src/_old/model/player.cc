#include <model/player.h>

#include <iostream>

#include <model/model.h>

void Player::toggle() {
    this->_running = !this->_running;
}

void Player::rewind() {
    this->_running = 0;
    this->_position = 0;
}

void Player::set(int position) {
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

void Player::tick() {
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

void Player::init(const Model *model) {
    /* set max position */
    this->_max_position = 0;
    for (std::pair<int, Schedule *> p: model->_schedules) {
        const Schedule *s = p.second;
        this->_max_position = std::max(this->_max_position, s->get_maximal_end());
    }
    this->_max_position = (this->_max_position / 20 + 1) * 20;
}

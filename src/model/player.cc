#include <model/player.h>

#include <iostream>

#include <model/model.h>

void Player::toggle() {
    this->running = !this->running;
}

void Player::rewind() {
    this->running = 0;
    this->position = 0;
}

void Player::set(int position) {
    this->position = position;
}

void Player::tick() {
    if (!this->running) {
        return;
    }
    this->position += this->speed;
}

void Player::init(const Model *model) {
    /* set max position */
    this->max_position = 0;
    for (auto p: model->schedules) {
        const Schedule &s = p.second;
        this->max_position = std::max(this->max_position, s.get_maximal_end());
    }
    this->max_position = (this->max_position / 20 + 1) * 20;
}

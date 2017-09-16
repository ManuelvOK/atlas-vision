#include <player.h>

#include <iostream>

#include <model.h>

void Player::toggle() {
    this->running = !this->running;
}

void Player::rewind() {
    this->running = 0;
    this->position = 0;
}

void Player::set(float position) {
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
    for (const Schedule &s: model->schedules) {
        this->max_position = std::max(this->max_position, s.get_maximal_end());
    }
}

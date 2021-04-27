#include <player/player_model.h>

#include <iostream>

void PlayerModel::toggle() {
    this->_running = !this->_running;
}

void PlayerModel::rewind() {
    this->_running = 0;
    this->set_position(0);
}

void PlayerModel::skip_forwards(int value) {
    this->set_position(this->_position + (value / this->_zoom));
}

void PlayerModel::skip_backwards(int value) {
    this->set_position(this->_position - (value / this->_zoom));
}

void PlayerModel::set_position(int position) {
    this->_position = std::min(std::max(position, 0), this->_max_position);
    int half_width_unit = this->_half_width / this->_zoom;
    int window_begin = this->_scroll - half_width_unit;
    int window_end = this->_scroll + half_width_unit;
    if (this->_position < window_begin + 100
        || this->_position > window_end - 100) {
        this->recenter();
    }
}

void PlayerModel::set_position_with_click(int position) {
    this->set_position((position - 10 - this->_half_width) / this->_zoom + this->_scroll);
}

void PlayerModel::tick() {
    if (!this->_running) {
        return;
    }
    this->set_position(this->_position + this->_speed);

    /* check if at end of simulation */
    if (this->_position == this->_max_position) {
        this->_running = false;
    }
}

void PlayerModel::zoom_in(float value) {
    this->set_zoom(this->_zoom * value);
}

void PlayerModel::zoom_out(float value) {
    this->set_zoom(this->_zoom / value);
}

void PlayerModel::set_zoom(float value) {
    this->_zoom = std::max(value, this->_zoom_min);
    this->reposition_scroll();
}

float PlayerModel::zoom() const {
    return this->_zoom;
}



void PlayerModel::set_width(int width) {
    this->_width = width;
    this->_half_width = this->_width / 2.0;
    this->_zoom_min = this->_width * 1.0 / this->_max_position;
    this->_zoom = this->_zoom_min;
    this->set_scroll(0);
}


void PlayerModel::scroll_left(int value) {
    this->set_scroll(this->_scroll + (value / this->_zoom));
}

void PlayerModel::scroll_right(int value) {
    this->set_scroll(this->_scroll - (value / this->_zoom));
}

void PlayerModel::set_scroll(int value) {
    int half_width_unit = this->_half_width / this->_zoom;
    this->_scroll = std::min(std::max(value, half_width_unit),
                             this->_max_position - half_width_unit);
}

int PlayerModel::scroll() const {
    return this->_scroll;
}

int PlayerModel::scroll_offset() const {
    return (this->_scroll * this->_zoom) - this->_half_width;
}

void PlayerModel::reposition_scroll() {
    this->set_scroll(this->_scroll);
}

void PlayerModel::recenter() {
    this->set_scroll(this->_position);
}

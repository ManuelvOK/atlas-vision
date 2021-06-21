#include <player/player_controller.h>

#include <algorithm>
#include <utility>

#include <iostream>

#include <SDL_GUI/gui/primitives/vertical_line.h>
#include <SDL_GUI/gui/primitives/text.h>

#include <config/interface_config.h>

PlayerController::PlayerController(PlayerModel *player_model, InputModel *input_model,
                                   const BaseSimulationModel *simulation_model,
                                   InterfaceModel *interface_model,
                                   SDL_GUI::InterfaceModel *default_interface_model) :
    _player_model(player_model),
    _input_model(input_model),
    _simulation_model(simulation_model),
    _interface_model(interface_model),
    _default_interface_model(default_interface_model) {
    this->init();
}

void PlayerController::update() {
    if (this->_player_model->_dirty) {
        this->init();
    }
    this->evaluate_input();
    this->drag();
    this->_player_model->tick();
}

void PlayerController::evaluate_input() {
    if (this->_input_model->is_down(InputKey::PLAYER_TOGGLE)) {
        this->_player_model->toggle();
    }
    if (this->_input_model->is_down(InputKey::PLAYER_REWIND)) {
        this->_player_model->rewind();
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_FORWARDS)) {
        this->_player_model->skip_forwards();
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_BACKWARDS)) {
        this->_player_model->skip_backwards();
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_SCROLL_LEFT)) {
        this->_player_model->scroll_left();
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_SCROLL_RIGHT)) {
        this->_player_model->scroll_right();
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_ZOOM_IN)) {
        this->_player_model->zoom_in();
    }
    if (this->_input_model->is_pressed(InputKey::PLAYER_ZOOM_OUT)) {
        this->_player_model->zoom_out();
    }

    if (this->_input_model->state() == InputState::IN_PLAYER) {
        if (this->_input_model->is_pressed(InputKey::SHIFT)) {
            if (this->_input_model->mouse_wheel()._y > 0) {
                this->_player_model->scroll_left(this->_input_model->mouse_wheel()._y * 45);
            }

            if (this->_input_model->mouse_wheel()._y < 0) {
                this->_player_model->scroll_right(-this->_input_model->mouse_wheel()._y * 45);
            }
        } else {
            if (this->_input_model->mouse_wheel()._y > 0) {
                this->_player_model->zoom_in_pos(this->_input_model->_mouse_in_player._x, 1.25);
            }

            if (this->_input_model->mouse_wheel()._y < 0) {
                this->_player_model->zoom_out_pos(this->_input_model->_mouse_in_player._x, 1.25);
            }
        }
    }

    if (this->_input_model->is_pressed(InputKey::PLAYER_SET_POSITION)) {
        this->_player_model->set_position_with_click(this->_input_model->_mouse_in_player._x);
    }
    if (this->_input_model->is_down(InputKey::PLAYER_DRAG)) {
        this->_dragging = true;
    }
    if (this->_input_model->is_up(InputKey::PLAYER_DRAG)) {
        this->_dragging = false;
    }

    if (this->_input_model->is_down(InputKey::MESSAGE_CLICK)
            and this->_simulation_model->_hovered_message) {
        this->_player_model->set_position(this->_simulation_model->_hovered_message->_timestamp);
    }
}

void PlayerController::drag() {
    if (not this->_dragging) {
        return;
    }
    int offset_x = this->_input_model->mouse_offset()._x;
    this->_player_model->scroll_right(offset_x);
}

void PlayerController::init() {
    /* set max position */
    int max_position = 0;
    for (BaseSchedule * schedule: this->_simulation_model->schedules()) {
        int max_end = schedule->get_maximal_end();
        int deadline = schedule->job()->deadline(max_end);
        max_position = std::max(max_position, std::max(max_end, deadline));
    }
    max_position = (max_position / 20.0 + 10) * 20;
    this->_player_model->_max_position = max_position;

    int min_position = 0;
    for (BaseJob *job: this->_simulation_model->jobs()) {
        min_position = std::min(min_position, job->_submission_time);
    }
    min_position = (min_position / 20.0 - 10) * 20;
    this->_player_model->_min_position = min_position;
    this->_player_model->_position = this->_player_model->_min_position;

    /* bind player position line to the models variable */
    SDL_GUI::Drawable *player_position_line =
        this->_default_interface_model->find_first_drawable("player-position-line");
    const PlayerModel *player_model = this->_player_model;
    const InterfaceModel *interface_model = this->_interface_model;
    player_position_line->add_recalculation_callback([player_model,
                                                     interface_model](SDL_GUI::Drawable *d){
            d->set_x(interface_model->px_width(player_model->_position));
        });

    /* show current time */
    SDL_GUI::Drawable *current_time_rect =
        this->_default_interface_model->find_first_drawable("current_time");

    SDL_GUI::Text *current_time = new SDL_GUI::Text(this->_default_interface_model->font(), "0");
    current_time_rect->add_child(current_time);

    current_time->add_recalculation_callback(
        [current_time, player_position_line, player_model](SDL_GUI::Drawable *d){
            std::stringstream ss;
            ss << player_model->_position;
            current_time->set_text(ss.str());
            SDL_GUI::Position line_pos = player_position_line->absolute_position();
            int new_x = line_pos._x - d->parent()->position()._x - (d->width() / 2);
            int max_x = d->parent()->width() - d->width();
            d->set_x(std::min(std::max(0, new_x), max_x));
        });

    SDL_GUI::Drawable *player = this->_default_interface_model->find_first_drawable("player");

    /* set minimal zoom */
    this->_player_model->set_width(player->width() - 20);

    /* set maximum scroll  */
    player->enable_scrolling_x();
    player->add_recalculation_callback([interface_model, player_model](SDL_GUI::Drawable *d) {
            d->set_scroll_position_x(-player_model->scroll_offset());
        });

    InputModel *input_model = this->_input_model;
    player->add_recalculation_callback([input_model, player_model](SDL_GUI::Drawable *d) {
            SDL_GUI::Position mouse_position = input_model->mouse_position();
            if (d->is_inside(mouse_position)) {
                input_model->_mouse_in_player = mouse_position - d->absolute_position();
                if (input_model->state() != InputState::IN_PLAYER) {
                    input_model->set_state(InputState::IN_PLAYER, false);
                }
            } else if (input_model->state() == InputState::IN_PLAYER) {
                input_model->set_state(InputState::ALL, false);
            }
        });

    /* add grid */
    SDL_GUI::Drawable *grid =this->_default_interface_model->find_first_drawable("player-grid");
    unsigned height = grid->height();
    unsigned grid_dark_distance =
        interface_config.player.grid_dark_distance * interface_config.player.grid_distance;
    /* TOOD: Add grid for t < 0 */
    unsigned grid_distance = interface_config.player.grid_distance;
    int begin = min_position - (min_position % grid_distance) - grid_distance;
    for (int i = begin; i < max_position; i += grid_distance){
        SDL_GUI::VerticalLine *l = new SDL_GUI::VerticalLine();
        /* every 5th line has a different color */
        if (i % grid_dark_distance == 0) {
            l->_style._color = SDL_GUI::RGB(interface_config.player.grid_dark_grey);
        } else {
            l->_style._color = SDL_GUI::RGB(interface_config.player.grid_grey);
        }
        l->set_height(height);
        l->add_recalculation_callback([interface_model, i](SDL_GUI::Drawable *d) {
                d->set_x(interface_model->px_width(i));
            });
        grid->add_child(l);
    }
    /* scale scheduler background frames */
    std::vector<SDL_GUI::Drawable *> backgrounds =
        this->_default_interface_model->find_drawables("scheduler");
    for (SDL_GUI::Drawable *d: backgrounds) {
        d->add_recalculation_callback([interface_model, max_position, min_position](SDL_GUI::Drawable *d) {
                d->set_width(interface_model->px_width(max_position - min_position));
            });
    }

    this->_player_model->_dirty = false;
}

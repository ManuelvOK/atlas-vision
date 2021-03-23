#include <controllers/player_controller.h>

#include <algorithm>
#include <utility>

#include <iostream>

#include <SDL_GUI/inc/gui/primitives/vertical_line.h>

#include <config/interface_config.h>

PlayerController::PlayerController(PlayerModel *player_model, InputModel *input_model,
                                   const AtlasModel *atlas_model, InterfaceModel *interface_model,
                                   SDL_GUI::InterfaceModel *default_interface_model) :
    _player_model(player_model),
    _input_model(input_model),
    _atlas_model(atlas_model),
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

    if (this->_input_model->is_pressed(InputKey::SHIFT)) {
        if (this->_input_model->mouse_wheel()._y > 0) {
            this->_player_model->scroll_left(this->_input_model->mouse_wheel()._y * 45);
        }

        if (this->_input_model->mouse_wheel()._y < 0) {
            this->_player_model->scroll_right(-this->_input_model->mouse_wheel()._y * 45);
        }
    } else {
        if (this->_input_model->mouse_wheel()._y > 0) {
            this->_player_model->zoom_in(1.25);
        }

        if (this->_input_model->mouse_wheel()._y < 0) {
            this->_player_model->zoom_out(1.25);
        }
    }

    if (this->_input_model->_player_hovered) {
        if (this->_input_model->is_pressed(InputKey::RIGHT_CLICK)) {
                this->_player_model->set_position_with_click(this->_input_model->_mouse_in_player._x);
        }
        if (this->_input_model->is_down(InputKey::CLICK)) {
            this->_dragging = true;
        }
        if (this->_input_model->is_up(InputKey::CLICK)) {
            this->_dragging = false;
        }
    } else if (this->_input_model->is_down(InputKey::CLICK)
               and this->_atlas_model->_hovered_message) {
        this->_player_model->set_position(this->_atlas_model->_hovered_message->_timestamp);
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
    for (Schedule * schedule: this->_atlas_model->_schedules) {
        max_position = std::max(max_position, schedule->get_maximal_end());
    }
    max_position = (max_position / 20 + 10) * 20;
    this->_player_model->_max_position = max_position;

    /* bind player position line to the models variable */
    SDL_GUI::Drawable *player_position_line =
        this->_default_interface_model->find_first_drawable("player-position-line");
    const PlayerModel *player_model = this->_player_model;
    const InterfaceModel *interface_model = this->_interface_model;
    player_position_line->add_recalculation_callback([player_model,
                                                     interface_model](SDL_GUI::Drawable *d){
            d->set_x(interface_model->px_width(player_model->_position));
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
                input_model->_player_hovered = true;
                input_model->_mouse_in_player = mouse_position - d->absolute_position();
            } else {
                input_model->_player_hovered = false;
            }
        });

    /* add grid */
    SDL_GUI::Drawable *grid =this->_default_interface_model->find_first_drawable("player-grid");
    unsigned height = grid->height();
    unsigned grid_dark_distance =
        interface_config.player.grid_dark_distance * interface_config.player.grid_distance;
    for (int i = 0; i < max_position; i += interface_config.player.grid_distance) {
        SDL_GUI::VerticalLine *l = new SDL_GUI::VerticalLine();
        /* every 5th line has a different color */
        if (i % grid_dark_distance == 0) {
            l->_default_style._color = SDL_GUI::RGB(interface_config.player.grid_dark_grey);
        } else {
            l->_default_style._color = SDL_GUI::RGB(interface_config.player.grid_grey);
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
        d->add_recalculation_callback([interface_model, max_position](SDL_GUI::Drawable *d) {
                d->set_width(interface_model->px_width(max_position));
            });
    }
    this->_player_model->_dirty = false;
}

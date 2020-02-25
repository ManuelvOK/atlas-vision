#pragma once

#include <SDL_GUI/inc/models/model_base.h>

class PlayerViewModel : public SDL_GUI::ModelBase {
public:
    int _shift_x = 0;
    int _shift_x_max = 0;
    int _shift_x_min = 0;
};

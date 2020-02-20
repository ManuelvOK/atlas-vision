#include <controllers/interface_controller.h>

#include <vector>

#include <SDL_GUI/inc/util/tree.h>
#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>
#include <SDL_GUI/inc/gui/primitives/rect.h>

InterfaceController::InterfaceController(const std::string template_file_path,
        SDL_GUI::InterfaceModel *interface_model, const MouseInputModel *mouse_input_model, const PlayerModel *player_model)
    : SDL_GUI::InterfaceController(template_file_path, interface_model, mouse_input_model), _player_model(player_model) {
        this->init_this();
    }

void InterfaceController::init_this() {
    SDL_GUI::Tree<SDL_GUI::Drawable> *tree = this->_interface_model->drawable_tree();
    std::vector<SDL_GUI::TreeNode<SDL_GUI::Drawable> *> player_position_line = tree->filter([](SDL_GUI::Drawable *d){return d->has_attribute("player_position_line");});
    const PlayerModel *player_model = this->_player_model;
    player_position_line[0]->node()->add_recalculation_callback([player_model](SDL_GUI::Drawable *d){
            //d->set_x(player_model->_position);
            d->set_x(d->position()._x + 1);
            });
}

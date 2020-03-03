#include <controllers/interface_controller.h>

#include <vector>

#include <SDL_GUI/inc/util/tree.h>
#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>
#include <SDL_GUI/inc/gui/primitives/rect.h>

InterfaceController::InterfaceController(const std::string template_file_path,
        InterfaceModel *interface_model, const SDL_GUI::InputModelBase *input_model, const PlayerModel *player_model)
    : SDL_GUI::InterfaceController(template_file_path, interface_model, input_model), _player_model(player_model) {
        this->init_this();
    }

void InterfaceController::init_this() {
}

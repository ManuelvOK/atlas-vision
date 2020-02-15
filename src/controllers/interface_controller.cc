#include <controllers/interface_controller.h>

#include <vector>

#include <SDL_GUI/inc/util/tree.h>
#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>
#include <SDL_GUI/inc/gui/primitives/rect.h>

InterfaceController::InterfaceController(const std::string template_file_path,
        SDL_GUI::InterfaceModel *interface_model, const MouseInputModel *mouse_input_model)
    : SDL_GUI::InterfaceController(template_file_path, interface_model, mouse_input_model) {
        this->init_this();
    }

void InterfaceController::init_this() {
    SDL_GUI::Tree<SDL_GUI::Drawable> *tree = this->_interface_model->drawable_tree();
    std::vector<SDL_GUI::TreeNode<SDL_GUI::Drawable> *> player = tree->filter([](SDL_GUI::Drawable *d){return d->has_attribute("player");});
    SDL_GUI::Rect *r = new SDL_GUI::Rect({0,0}, 100, 100);
    player[0]->add_child(r);

    r->_default_style._color = SDL_GUI::RGB("black");
    r->add_recalculation_callback([r](){
            static int i = 0;
            i++;
            if (i % 20 == 0) {
                r->set_x(r->position()._x + 1);
            }
            });
}

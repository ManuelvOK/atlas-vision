#include <application.h>

#include <SDL_GUI/inc/models/interface_model.h>
#include <SDL_GUI/inc/views/interface_view.h>

#include <controllers/interface_controller.h>
#include <controllers/keyboard_input_controller.h>
#include <controllers/mouse_input_controller.h>
#include <models/keyboard_input_model.h>
#include <models/mouse_input_model.h>

void Application::init() {
    this->init_SDL();
    this->init_window();
    this->init_renderer();

    /**********
     * Models *
     **********/
    /* init keyboard input model */
    KeyboardInputModel *keyboard_input_model = new KeyboardInputModel();
    this->_model_list.push_back(keyboard_input_model);

    /* init keyboard input model */
    MouseInputModel *mouse_input_model = new MouseInputModel();
    this->_model_list.push_back(mouse_input_model);

    /* init interface model */
    SDL_GUI::InterfaceModel *interface_model = new SDL_GUI::InterfaceModel();
    this->_model_list.push_back(interface_model);

    /***************
     * Controllers *
     ***************/
    /* init keyboard input controller */
    KeyboardInputController *keyboard_input_controller = new KeyboardInputController(&this->_is_running);
    keyboard_input_controller->set_model(keyboard_input_model);
    this->_controller_list.push_back(keyboard_input_controller);

    /* init mouse input controller */
    MouseInputController *mouse_input_controller = new MouseInputController();
    mouse_input_controller->set_model(mouse_input_model);
    this->_controller_list.push_back(mouse_input_controller);


    /* init interface controller */
    InterfaceController *interface_controller = new InterfaceController("./templates/main.tpl", interface_model, mouse_input_model);
    this->_controller_list.push_back(interface_controller);

    /********
     * View *
     ********/
    /* init interface view */
    SDL_GUI::InterfaceView *interface_view = new SDL_GUI::InterfaceView(this->_renderer, interface_model);
    this->_view_list.push_back(interface_view);

}

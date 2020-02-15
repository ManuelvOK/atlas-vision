#pragma once

#include <SDL_GUI/inc/application.h>

class Application : public SDL_GUI::Application {
public:
    Application(std::string application_title) : SDL_GUI::Application(application_title) {}
    void init() override;
};


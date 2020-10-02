#include <iostream>

#include <SDL_GUI/inc/application.h>
#include <SDL_GUI/inc/plugins/default_plugin.h>

#include <plugins/atlas_plugin.h>

int main(int argc, char *argv[]) {

    SDL_GUI::Application<AtlasPlugin, SDL_GUI::DefaultPlugin> app("Application", argc, argv);
    app.run();

    return 0;
}

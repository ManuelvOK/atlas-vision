#include <iostream>

#include <SDL_GUI/application.h>
#include <SDL_GUI/plugins/default_plugin.h>

#include <plugins/atlas_plugin.h>

int main(int argc, char *argv[]) {

    SDL_GUI::Application<AtlasPlugin, SDL_GUI::DefaultPlugin> app("Application", argc, argv, 1500,
                                                                  1000);
    app.run();

    return 0;
}

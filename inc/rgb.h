#include <SDL2/SDL.h>

class RGB {
public:
    int r = 0;
    int g = 0;
    int b = 0;
    void activate(SDL_Renderer *renderer, int alpha = 255) const;
};

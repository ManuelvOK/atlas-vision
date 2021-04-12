# Dependencies

This project depends on the SDL_GUI library I build. This is a submodule of
this repository. Make sure you have pulled recursively.

The library has the following dependencies:

 * sdl2
 * sdl2_ttf
 * sdl2_image
 * fontconfig

# Building

`$ make` builds the projet to `build/main`.

# Usage

`$ ./build/main [-n] INPUT_FILE`

Use the `-n` flag to specify input that already has been simulated.
For reading from stdin use `-` as `INPUT_FILE`.

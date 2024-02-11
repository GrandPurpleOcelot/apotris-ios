#include <SDL2/SDL.h>

extern void initShaders(SDL_Window *window);

extern void drawWithShaders(SDL_Window * window, SDL_Surface *img, bool shadersEnabled);

extern void freeShaders();

extern void refreshShaderResolution(int w, int h, float s);

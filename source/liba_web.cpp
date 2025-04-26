#ifdef WEB

#include "liba_web.h"
#include <fstream>
#include <list>

#include "def.h"
#include "nanotime.h"
#include <dirent.h>
#include <emscripten.h>
#include <emscripten/html5.h>

void resizeWindow(int width, int height);

std::list<uint32_t> currentKeys;
std::list<uint32_t> previousKeys;

std::list<uint32_t> currentlyPressed;

int rowStart = (240 - 240) / 2;
int rowEnd = rowStart + 240;
// int rowStart = 0;
// int rowEnd = 160;

#define FPS_TARGET 60

double clock_timer = 0;

static uint64_t frame_start = nanotime_now();
nanotime_step_data stepper;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_GameController* controller;

bool render = true;

uint32_t start_time = 0;
uint32_t frame_time = 0;

float fps = 0;

int screenWidth = 1280;
int screenHeight = 720;

float windowScale = 4;

EM_JS(int, getWindowWidth, (), { return window.innerWidth; });

EM_JS(int, getWindowHeight, (), { return window.innerHeight; });

int resizeCallback(int eventType, const EmscriptenUiEvent* event,
                   void* userData) {
    screenWidth = getWindowWidth();
    screenHeight = getWindowHeight();

    resizeWindow(screenWidth, screenHeight);
    return 0;
}

bool initController(int eventType, const EmscriptenGamepadEvent* gamepadEvent,
                    void* userData) {
    if (SDL_NumJoysticks() > 0) {
        controller = SDL_GameControllerOpen(0);
    }
    return true;
}

void windowInit() {
    // Initialize SDL

    SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");

    EM_ASM(
        // Make a directory other than '/'
        FS.mkdir('/offline');
        // Then mount with IDBFS type
        FS.mount(IDBFS, {}, '/offline');

        // Then sync
        FS.syncfs(true, function(err){
                            // Error
                        }););

    emscripten_sleep(100);

    window = SDL_CreateWindow("Apotris Web", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    resizeWindow(getWindowWidth(), getWindowHeight());

    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    loadAudio("");

    nanotime_step_init(&stepper, (uint64_t)(NANOTIME_NSEC_PER_SEC / FPS_TARGET),
                       nanotime_now_max(), nanotime_now, nanotime_sleep);

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr,
                                   false, resizeCallback);

    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

    emscripten_set_gamepadconnected_callback(
        nullptr, false,
        reinterpret_cast<em_gamepad_callback_func>(initController));
}

void key_poll() {
    previousKeys = currentKeys;
    currentKeys.clear();

    // if(windowWidth != getWindowWidth() || windowHeight != getWindowHeight()){

    //     windowWidth = getWindowWidth();
    //     windowHeight = getWindowHeight();

    //     resizeWindow(windowWidth, windowHeight);
    // }

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        uint32_t key = 0;
        auto it = currentlyPressed.begin();
        bool found = false;

        switch (event.type) {
        case SDL_KEYDOWN:
            key = event.key.keysym.sym;
            if (key == SDLK_F11) {
                // log("here!");
                // emscripten_request_fullscreen(document.documentElement,1);
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                break;
            }

            it = std::find(currentlyPressed.begin(), currentlyPressed.end(),
                           key);
            found = (it != currentlyPressed.end());
            if (!found)
                currentlyPressed.push_back(key);
            break;
        case SDL_KEYUP:
            key = event.key.keysym.sym;
            it = std::find(currentlyPressed.begin(), currentlyPressed.end(),
                           key);
            found = (it != currentlyPressed.end());
            if (found)
                currentlyPressed.remove(key);
            break;
        case SDL_JOYBUTTONDOWN:
        case SDL_CONTROLLERBUTTONDOWN:
            key = event.jbutton.button;
            it = std::find(currentlyPressed.begin(), currentlyPressed.end(),
                           key);
            found = (it != currentlyPressed.end());
            if (!found)
                currentlyPressed.push_back(key);
            break;
        case SDL_JOYBUTTONUP:
        case SDL_CONTROLLERBUTTONUP:
            key = event.jbutton.button;
            it = std::find(currentlyPressed.begin(), currentlyPressed.end(),
                           key);
            found = (it != currentlyPressed.end());
            if (found)
                currentlyPressed.remove(key);
            break;
        case SDL_QUIT:
            quit();
            break;
        default:
            break;
        }
    }

    currentKeys = currentlyPressed;
}

uint32_t key_is_down(uint32_t key) {
    if (key == KEY_FULL)
        return (!currentKeys.empty());

    auto it = currentKeys.begin();

    it = std::find(currentKeys.begin(), currentKeys.end(), key);

    return (it != currentKeys.end());

    return 0;
}

uint32_t key_hit(uint32_t key) {
    if (key == KEY_FULL)
        return (previousKeys.empty() && !currentKeys.empty())
                   ? currentKeys.front()
                   : 0;

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(), previousKeys.end(), key);
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(), currentKeys.end(), key);
    bool curr = (it != currentKeys.end());

    return (!prev && curr);

    return 0;
}

uint32_t key_released(uint32_t key) {
    if (key == KEY_FULL)
        return (!previousKeys.empty() && currentKeys.empty());

    auto it = previousKeys.begin();
    it = std::find(previousKeys.begin(), previousKeys.end(), key);
    bool prev = (it != previousKeys.end());

    it = currentKeys.begin();
    it = std::find(currentKeys.begin(), currentKeys.end(), key);
    bool curr = (it != currentKeys.end());

    return (prev && !curr);

    return 0;
}

uint32_t key_first() {
    if (currentKeys.empty())
        return KEY_FULL - 1;

    for (auto key : currentKeys) {
        auto it = previousKeys.begin();
        it = std::find(previousKeys.begin(), previousKeys.end(), key);
        bool prev = (it != previousKeys.end());

        if (!prev) {
            return key;
        }
    }

    return KEY_FULL - 1;
}

void updateWindow(uint8_t* framebuffer) {
    const int in_width = SCREEN_WIDTH;
    const int in_height = SCREEN_HEIGHT;

    SDL_Surface* img = SDL_CreateRGBSurfaceFrom(
        framebuffer, in_width, in_height, 32, in_width * 4, 0x0000ff, 0x00ff00,
        0xff0000, 0xff000000);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);

    SDL_FreeSurface(img);

    SDL_RenderClear(renderer);

    SDL_Rect rect = {
        0,
        0,
        in_width,
        in_height,
    };

    const int w = in_width * windowScale;
    const int h = in_height * windowScale;

    SDL_Rect dest_rect = {
        -(w - screenWidth) / 2,
        -(h - screenHeight) / 2,
        w,
        h,
    };

    if (render) {
        SDL_RenderCopy(renderer, texture, &rect, &dest_rect);
    }

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

    // Cap FPS
    nanotime_step(&stepper);

    Uint64 end = SDL_GetPerformanceCounter();

    float elapsedMS =
        (end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    if (elapsedMS > 100) {
        nanotime_step_init(&stepper,
                           (uint64_t)(NANOTIME_NSEC_PER_SEC / FPS_TARGET),
                           nanotime_now_max(), nanotime_now, nanotime_sleep);
    }

    fps = 1000.0f / elapsedMS;

    frame_start = SDL_GetPerformanceCounter();
}

void refreshWindowSize() {
    if (savefile != nullptr) {
        if (savefile->settings.zoom > -1) {
            windowScale = 1 + (float)savefile->settings.zoom / 10;
        } else {
            if (savefile->settings.integerScale) {
                windowScale = (int)windowScale;

                while (windowScale > 0 &&
                       screenHeight / windowScale > 160 * 2) {
                    windowScale++;
                }
                while (windowScale > 1 && screenHeight / windowScale < 160) {
                    windowScale--;
                }
            } else {
                windowScale = screenHeight / 200.0;
            }
        }
    }

    if (windowScale <= 0)
        return;

    rowStart = (SCREEN_HEIGHT - (screenHeight / windowScale)) / 2;
    rowEnd = (SCREEN_HEIGHT + (screenHeight / windowScale)) / 2;

    if (rowStart < 0)
        rowStart = 0;

    if (rowEnd > SCREEN_HEIGHT)
        rowEnd = SCREEN_HEIGHT;

    if (savefile != nullptr)
        setGradient(savefile->settings.backgroundGradient);
}

bool closed() {
    emscripten_sleep(0);
    songEndHandler();
    // // Cap to 60 FPS

    // //show fps
    // frame_time = SDL_GetTicks()-start_time;
    // fps = (frame_time > 0)? 1000.0 / frame_time : 0.0;
    // start_time = SDL_GetTicks();

    // std::cout << "fps: " << fps << "\n";

    return true;
}

void initRumble() {};

void loadSavefile() {
    EM_ASM(var s = FS.open("/offline/Apotris.sav", "a"); FS.close(s););

    std::ifstream input("/offline/Apotris.sav",
                        std::ios::binary | std::ios::in);

    if (!input) {
        log("Error when trying to load save.");
        return;
    }

    if (savefile == nullptr)
        savefile = new Save();

    char* src = (char*)savefile;

    input.read(src, sizeof(Save));

    input.close();
}

void saveSavefile() {
    std::ofstream output("/offline/Apotris.sav",
                         std::ios::binary | std::ios::out);

    if (!output) {
        log("Error when trying to write save.");
        return;
    }

    char* dst = (char*)savefile;

    const int saveSize = 1 << 15;

    char temp[saveSize];

    memset32_fast(temp, 0, saveSize / 4);
    memcpy32_fast(temp, dst, sizeof(Save) / 4);

    output.write(temp, saveSize);

    output.close();

    EM_ASM(FS.syncfs(function(err){
        // Error
    }););
    emscripten_sleep(100);
}

void quit() {
    freeAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void resizeWindow(int width, int height) {
    emscripten_set_canvas_size(width, height);
    SDL_SetWindowSize(window, width, height);

    screenWidth = width;
    screenHeight = height;
}

std::string stringFromKey(uint32_t key) { return SDL_GetKeyName(key); }

void toggleRendering(bool r) { render = r; }

#endif

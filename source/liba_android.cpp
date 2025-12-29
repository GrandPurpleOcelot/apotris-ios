#ifdef ANDROID

#include "liba_android.h"
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <jni.h>
#include <sys/stat.h>
#include <unordered_set>

#include "def.h"

#include "shader.h"

#include "nanotime.h"

static bool inBackground = false;
static bool pendingBackground = false;

static jobject vibrateService = nullptr;
static jmethodID vibrateMethodId = 0;
static jmethodID vibrateCancelMethodId = 0;

void handleAnalogInput(int axis, int value);
void handleInput();
void updateBatteryInfo();
void setLayout();

bool vibrationInit();
void vibrationStart(int durationMS);
void vibrationStop();
void vibrationDeinit();

std::unordered_set<uint32_t> currentKeys;
std::unordered_set<uint32_t> previousKeys;

std::unordered_set<uint32_t> currentlyPressed;

std::list<SDL_Rect> rects;

// int rowStart = (288 - 288) / 2;
// int rowEnd = rowStart + 288;
int rowStart = 0;
int rowEnd = SCREEN_HEIGHT;

#define FPS_TARGET 60

double clock_timer = 0;

static uint64_t frame_start = nanotime_now();

uint32_t start_time = 0;
uint32_t frame_time = 0;

float fps = 0;

int batteryPercentage = -1;
bool charging = false;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
SDL_GameController* controller;

Layout layout;

int controllerCount = 0;

bool fullscreen = false;

bool render = true;

int screenWidth = 1280;
int screenHeight = 720;

float windowScale = 3;

float gameWindowScale = 6;

InputType lastInputType = InputType::KEYBOARD;

nanotime_step_data stepper;

ShaderStatus shaderStatus = ShaderStatus::NOT_INITED;

SDL_Point touchLocation = {0, 0};

bool controlsUnlocked = false;

int moving = 0;

#define DEADZONE 12000

int KEY_A = (SDL_CONTROLLER_BUTTON_A << 16) | packKey(SDLK_RETURN);
int KEY_B = (SDL_CONTROLLER_BUTTON_B << 16) | packKey(SDLK_BACKSPACE);
int KEY_L = (SDL_CONTROLLER_BUTTON_LEFTSHOULDER << 16) | packKey(SDLK_1);
int KEY_R = (SDL_CONTROLLER_BUTTON_RIGHTSHOULDER << 16) | packKey(SDLK_2);
int KEY_UP = (SDL_CONTROLLER_BUTTON_DPAD_UP << 16) | packKey(SDLK_w);
int KEY_DOWN = (SDL_CONTROLLER_BUTTON_DPAD_DOWN << 16) | packKey(SDLK_s);
int KEY_LEFT = (SDL_CONTROLLER_BUTTON_DPAD_LEFT << 16) | packKey(SDLK_a);
int KEY_RIGHT = (SDL_CONTROLLER_BUTTON_DPAD_RIGHT << 16) | packKey(SDLK_d);
int KEY_SELECT = (SDL_CONTROLLER_BUTTON_BACK << 16) | packKey(SDLK_3);
int KEY_START = (SDL_CONTROLLER_BUTTON_START << 16) | packKey(SDLK_ESCAPE);

void windowInit() {
    // Initialize SDL
    window = nullptr;

    SDL_Init(SDL_INIT_VIDEO);

    // Disables the default accelerometer "joystick" on android
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

    window = SDL_CreateWindow("Apotris Android", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL |
                                  SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    if (SDL_NumJoysticks() > 0) {
        controller = SDL_GameControllerOpen(0);
        controllerCount = SDL_NumJoysticks();
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_ShowCursor(SDL_DISABLE);

    refreshWindowSize();

    loadAudio("");

    nanotime_step_init(&stepper, (uint64_t)(NANOTIME_NSEC_PER_SEC / FPS_TARGET),
                       nanotime_now_max(), nanotime_now, nanotime_sleep);

    vibrationInit();
}

void pressKey(int key, InputType type) {
    lastInputType = type;

    currentlyPressed.insert(key);
}

void unpressKey(int key, InputType type) {
    lastInputType = type;

    currentlyPressed.erase(key);
}

void key_poll() {}

void saveLayout() {
    if (savefile == nullptr) {
        return;
    }

    savefile->settings.onScreenLayouts[screenWidth > screenHeight] = layout;
}

void handleInput() {
    if (pendingBackground) {
        inBackground = true;
        pendingBackground = false;
        render = false;
    }

    SDL_Event event;

    int x = 0, y = 0, k = 0, dx = 0, dy = 0;
    int index = 0;

    bool eventAvailable = true;

    while (eventAvailable) {
        if (inBackground) {
            eventAvailable = SDL_WaitEvent(&event);
        } else {
            eventAvailable = SDL_PollEvent(&event);
        }

        if (!eventAvailable)
            break;

        int key = 0;
        auto it = currentlyPressed.begin();
        bool found = false;

        switch (event.type) {
        case SDL_KEYDOWN:
            key = event.key.keysym.sym;
            pressKey(key, InputType::KEYBOARD);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            key = event.cbutton.button;
            pressKey(key, InputType::CONTROLLER);
            break;
        case SDL_KEYUP:
            key = event.key.keysym.sym;
            unpressKey(key, InputType::KEYBOARD);
            break;
        case SDL_CONTROLLERBUTTONUP:
            key = event.cbutton.button;
            unpressKey(key, InputType::CONTROLLER);
            break;
        case SDL_CONTROLLERAXISMOTION:
            handleAnalogInput(event.caxis.axis, event.caxis.value);
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                screenWidth = event.window.data1;
                screenHeight = event.window.data2;
                refreshWindowSize();
            }
            break;
        case SDL_JOYBUTTONDOWN:
            key = event.jbutton.button;
            pressKey(key, InputType::CONTROLLER);
            break;
        case SDL_JOYBUTTONUP:
            key = event.jbutton.button;
            unpressKey(key, InputType::CONTROLLER);
            break;
        case SDL_FINGERDOWN:
            if (controllerCount > 0)
                break;

            x = touchLocation.x = event.tfinger.x * screenWidth;
            y = touchLocation.y = event.tfinger.y * screenHeight;

            for (auto const& button : layout.buttons) {
                if (x > button.x - button.radius &&
                    x < button.x + button.radius &&
                    y > button.y - button.radius &&
                    y < button.y + button.radius) {

                    if (button.key == -1) {
                        controlsUnlocked = !controlsUnlocked;
                        if (!controlsUnlocked) {
                            saveLayout();
                        }
                        continue;
                    }

                    if (!controlsUnlocked) {
                        pressKey(button.key, InputType::TOUCH);
                        vibrationStart(16);
                    } else {
                        moving = index;
                    }
                }
                index++;
            }

            break;
        case SDL_FINGERMOTION:
            if (controllerCount > 0)
                break;

            dx = event.tfinger.dx * screenWidth;
            dy = event.tfinger.dy * screenHeight;

            x = touchLocation.x = event.tfinger.x * screenWidth;
            y = touchLocation.y = event.tfinger.y * screenHeight;

            if (!controlsUnlocked) {
                for (auto const& button : layout.buttons) {
                    if (!(x > button.x - button.radius &&
                          x < button.x + button.radius &&
                          y > button.y - button.radius &&
                          y < button.y + button.radius) &&
                        ((x - dx) > button.x - button.radius &&
                         (x - dx) < button.x + button.radius &&
                         (y - dy) > button.y - button.radius &&
                         (y - dy) < button.y + button.radius)) {

                        unpressKey(button.key, InputType::TOUCH);
                    } else if ((x > button.x - button.radius &&
                                x < button.x + button.radius &&
                                y > button.y - button.radius &&
                                y < button.y + button.radius) &&
                               !((x - dx) > button.x - button.radius &&
                                 (x - dx) < button.x + button.radius &&
                                 (y - dy) > button.y - button.radius &&
                                 (y - dy) < button.y + button.radius)) {

                        if (!controlsUnlocked) {
                            pressKey(button.key, InputType::TOUCH);
                            vibrationStart(16);
                        }
                    }
                }
            } else {

                if (moving >= 0) {
                    layout.buttons[moving].x = x;
                    layout.buttons[moving].y = y;
                }
            }

            break;
        case SDL_FINGERUP:
            if (controllerCount > 0)
                break;

            x = touchLocation.x = event.tfinger.x * screenWidth;
            y = touchLocation.y = event.tfinger.y * screenHeight;

            if (!controlsUnlocked) {
                for (auto const& button : layout.buttons) {
                    if (x > button.x - button.radius &&
                        x < button.x + button.radius &&
                        y > button.y - button.radius &&
                        y < button.y + button.radius) {

                        unpressKey(button.key, InputType::TOUCH);
                    }
                }
            } else {
                moving = -1;
            }
            break;
        case SDL_APP_WILLENTERBACKGROUND:
            paused = true;
            vibrationStop();
            break;
        case SDL_APP_DIDENTERBACKGROUND:
            pendingBackground = true;
            break;
        case SDL_APP_WILLENTERFOREGROUND:
            inBackground = false;
            pendingBackground = false;
            render = true;
            break;
        case SDL_APP_DIDENTERFOREGROUND:
            inBackground = false;
            pendingBackground = false;
            render = true;
            break;
        case SDL_APP_TERMINATING:
        case SDL_APP_LOWMEMORY:
            addGameStats();
            saveSavefile();
            if (event.type == SDL_APP_TERMINATING) {
                quit();
            }
            break;
        case SDL_QUIT:
            quit();
            break;
        default:
            break;
        }
    }

    previousKeys = currentKeys;
    currentKeys.clear();

    currentKeys = currentlyPressed;
}

int splitKey(uint32_t key) {
    if (lastInputType == InputType::KEYBOARD) {
        return unpackKey(key & 0xffff);
    } else {
        return (key & 0xffff0000) >> 16;
    }
}

uint32_t key_is_down(uint32_t key) {
    if (key == KEY_FULL)
        return (!currentKeys.empty());

    key = splitKey(key);

    return currentKeys.count(key);
}

uint32_t key_hit(uint32_t key) {
    if (key == KEY_FULL)
        return (previousKeys.empty() && !currentKeys.empty());

    key = splitKey(key);

    bool prev = previousKeys.count(key);
    bool curr = currentKeys.count(key);

    return (!prev && curr);
}

uint32_t key_released(uint32_t key) {
    if (key == KEY_FULL)
        return (!previousKeys.empty() && currentKeys.empty());

    key = splitKey(key);

    bool prev = previousKeys.count(key);
    bool curr = currentKeys.count(key);

    return (prev && !curr);
}

uint32_t key_first() {
    if (currentKeys.empty())
        return KEY_FULL - 1;

    for (auto key : currentKeys) {
        bool prev = previousKeys.count(key);

        if (!prev) {
            return key;
        }
    }

    return KEY_FULL - 1;
}

uint32_t keys_raw() {
    if (currentKeys.empty())
        return KEY_FULL - 1;

    u32 allKeys = KEY_FULL;
    for (auto key : currentKeys) {
        allKeys ^= key;
    }

    return allKeys;
}

void updateWindow(uint8_t* framebuffer) {
    const int in_width = 512;
    const int in_height = 512;

    // Create the SDL_Surface from the framebuffer
    SDL_Surface* img = SDL_CreateRGBSurfaceFrom(
        framebuffer, in_width, in_height, 32, in_width * 4, 0x0000ff, 0x00ff00,
        0xff0000, 0xff000000);

    if (img == NULL) {
        printf("Failed to create SDL_Surface: %s\n", SDL_GetError());
        return;
    }

    if (shaderStatus != ShaderStatus::OK || savefile->settings.shaders == 0) {
        // Create the streaming texture once
        if (texture == NULL) {
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                        SDL_TEXTUREACCESS_STREAMING, in_width,
                                        in_height);
            if (texture == NULL) {
                printf("Failed to create texture: %s\n", SDL_GetError());
                SDL_FreeSurface(img);
                return;
            }
        }

        // Lock the texture to update its pixel data
        void* pixels;
        int pitch;
        if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
            printf("Failed to lock texture: %s\n", SDL_GetError());
            SDL_FreeSurface(img);
            return;
        }

        // Copy the surface's pixel data into the streaming texture
        for (int y = 0; y < in_height; y++) {
            memcpy((uint8_t*)pixels + y * pitch, // Destination pointer
                   (uint8_t*)img->pixels + y * img->pitch, // Source pointer
                   in_width * 4                            // Row size in bytes
            );
        }

        SDL_UnlockTexture(texture);

        // Clear the renderer
        SDL_RenderClear(renderer);

        // Define source and destination rectangles
        SDL_Rect src_rect = {0, 0, in_width, in_height};
        const int w = in_width * windowScale;
        const int h = in_height * windowScale;
        SDL_Rect dest_rect = {-(w - screenWidth) / 2, -(h - screenHeight) / 2,
                              w, h};

        // Copy the updated texture to the renderer
        if (render) {
            SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
        }

        for (auto const& rect : rects) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);

            // Render rect
            SDL_RenderFillRect(renderer, &rect);
        }

        if (controllerCount == 0) {
            for (auto const& button : layout.buttons) {
                if (!controlsUnlocked) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                } else {
                    SDL_SetRenderDrawColor(renderer, 180, 255, 180, 100);
                }

                SDL_Rect r;
                r.x = button.x - button.radius;
                r.y = button.y - button.radius;
                r.w = button.radius * 2;
                r.h = button.radius * 2;

                // Render rect
                SDL_RenderFillRect(renderer, &r);
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        }

        rects.clear();

        SDL_RenderPresent(renderer);
    } else {
        if (render) {
            drawWithShaders(window, img, true);
        }
    }

    SDL_FreeSurface(img);

    handleInput();

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

    // handle controller hotplug
    if (SDL_NumJoysticks() != controllerCount) {
        if (controllerCount == 0) {
            controller = SDL_GameControllerOpen(0);
        } else if (SDL_NumJoysticks() == 0) {
            SDL_GameControllerClose(controller);
        }

        controllerCount = SDL_NumJoysticks();
    }

    updateBatteryInfo();
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

    if (savefile != nullptr) {
        setGradient(savefile->settings.backgroundGradient);

        if (savefile->settings.shaders != 0) {
            refreshShaderResolution(screenWidth, screenHeight, windowScale);
        }

        setLayout();
    }

    int offsetx = 0, offsety = 0;
    if (screenWidth > screenHeight) {
        offsetx = (SCREEN_WIDTH - 240) / 2;
        offsety = (SCREEN_HEIGHT - 160) / 2;
    } else {
        offsetx = (SCREEN_WIDTH - 240) * 0.5;
        offsety = (SCREEN_HEIGHT - 160) * 0.35;
    }
    setScreenOffset(offsetx, offsety);
}

bool closed() {
    songEndHandler();

    return true;
}

void toggleRendering(bool r) { render = r; }

void initRumble(){};

std::string getSavefilePath() {
    char* path = SDL_GetPrefPath("akouzoukos", "Apotris");
    if (!path) {
        return "Apotris.sav";
    }
    std::string s = std::string(path) + "Apotris.sav";
    SDL_free(path);
    return s;
}

void loadSavefile() {
    std::string filePath = getSavefilePath();
    std::ifstream input(filePath, std::ios::binary | std::ios::in);

    if (savefile == nullptr)
        savefile = new Save();

    char* src = (char*)savefile;

    input.read(src, sizeof(Save));

    if (!input) {
        log("Error when trying to load save.");
        return;
    }

    input.close();
}

void saveSavefile() {
    std::string filePath = getSavefilePath();
    std::ofstream output(filePath, std::ios::binary | std::ios::out);

    char* dst = (char*)savefile;

    const int saveSize = 1 << 15;

    char temp[saveSize];

    memset32_fast(temp, 0, saveSize / 4);
    memcpy32_fast(temp, dst, sizeof(Save) / 4);

    output.write(temp, saveSize);

    if (!output) {
        log("Error when trying to write save.");
        return;
    }

    output.close();
}

void quit() {
    vibrationDeinit();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    freeAudio();
    exit(0);
}

std::map<int, std::string> keyToString = {
    {SDL_CONTROLLER_BUTTON_A, "A"},
    {SDL_CONTROLLER_BUTTON_B, "B"},
    {SDL_CONTROLLER_BUTTON_X, "X"},
    {SDL_CONTROLLER_BUTTON_Y, "Y"},
    {SDL_CONTROLLER_BUTTON_BACK, "Select"},
    {SDL_CONTROLLER_BUTTON_START, "Start"},
    {SDL_CONTROLLER_BUTTON_DPAD_LEFT, "Left"},
    {SDL_CONTROLLER_BUTTON_DPAD_UP, "Up"},
    {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "Right"},
    {SDL_CONTROLLER_BUTTON_DPAD_DOWN, "Down"},
    {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "LB"},
    {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "RB"},
    {SDL_CONTROLLER_BUTTON_LEFTSTICK, "LS"},
    {SDL_CONTROLLER_BUTTON_RIGHTSTICK, "RS"},
    {(1 << 14) | (0 << 5) | (0), "Left +X"},
    {(1 << 14) | (1 << 5) | (0), "Left -X"},
    {(1 << 14) | (0 << 5) | (1), "Left +Y"},
    {(1 << 14) | (1 << 5) | (1), "Left -Y"},
    {(1 << 14) | (0 << 5) | (2), "Right +X"},
    {(1 << 14) | (1 << 5) | (2), "Right -X"},
    {(1 << 14) | (0 << 5) | (3), "Right +Y"},
    {(1 << 14) | (1 << 5) | (3), "Right -Y"},
    {(1 << 14) | (0 << 5) | (4), "LT"},
    {(1 << 14) | (0 << 5) | (5), "RT"},
    {0xffff, ""},
};

std::string stringFromKey(uint32_t key) {
    if (lastInputType == InputType::KEYBOARD) {
        key = unpackKey(key & 0xffff);

        if (key == 0xffff)
            return "";

        return SDL_GetKeyName(key);
    } else {
        return keyToString[key >> 16];
    }
}

void setKey(int& dest, uint32_t key) {
    if (lastInputType == InputType::KEYBOARD) {
        dest = (dest & 0xffff0000) | packKey(key);
    } else {
        dest = (dest & 0xffff) | (key << 16);
    }
}

void unbindDuplicateKey(int& dest, uint32_t key) {
    if (lastInputType == InputType::KEYBOARD) {
        if ((dest & 0xffff) == (key & 0xffff))
            dest = (dest & 0xffff0000) | 0xfffe;
    } else {
        if ((dest >> 16) == key)
            dest = (dest & 0xffff) | (0xfffe << 16);
    }
}

void rumbleStart(int length) {
    if (lastInputType != InputType::CONTROLLER || length <= 0)
        return;

    SDL_GameControllerRumble(controller, 0xffff, 0xffff, length * 64);
}

void rumbleStop() { SDL_GameControllerRumble(controller, 0, 0, 1); }

void setFullscreen(bool state) {
    fullscreen = state;
    if (!fullscreen) {
        SDL_SetWindowFullscreen(window, 0);
    } else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

void shaderInit(int index) { initShaders(window, index); }

void shaderDeinit() { freeShaders(); }

void handleAnalogInput(int axis, int value) {
    lastInputType = InputType::CONTROLLER;

    int key = 0;

    key |= (axis & 0xf) | (1 << 14);

    if (value > DEADZONE) {
        pressKey((key), InputType::CONTROLLER);
        unpressKey((key) | (1 << 5), InputType::CONTROLLER);
    } else if (value < -DEADZONE) {
        pressKey((key) | (1 << 5), InputType::CONTROLLER);
        unpressKey((key), InputType::CONTROLLER);
    } else {
        unpressKey((key), InputType::CONTROLLER);
        unpressKey((key) | (1 << 5), InputType::CONTROLLER);
    }
}

void drawRect(int x, int y, int w, int h) {
    // Creat a rect at pos ( 50, 50 ) that's 50 pixels wide and 50 pixels high.
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;

    rects.push_back(r);
}

void setGameWindowScale() {
    windowScale = gameWindowScale;
    rowStart = (SCREEN_HEIGHT - (screenHeight / windowScale)) / 2;
    rowEnd = (SCREEN_HEIGHT + (screenHeight / windowScale)) / 2;

    if (rowStart < 0)
        rowStart = 0;

    if (rowEnd > SCREEN_HEIGHT)
        rowEnd = SCREEN_HEIGHT;

    if (savefile != nullptr) {
        setGradient(savefile->settings.backgroundGradient);

        if (savefile->settings.shaders != 0) {
            refreshShaderResolution(screenWidth, screenHeight, windowScale);
        }
    }
}

bool vibrationInit() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        SDL_Log("Failed to get JNI environment");
        return false;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity) {
        SDL_Log("Failed to get SDL Activity");
        return false;
    }

    jclass clazz = env->GetObjectClass(activity);
    if (!clazz) {
        SDL_Log("Failed to get Activity class");
        env->DeleteLocalRef(activity);
        return false;
    }

    jmethodID getSystemService_id = env->GetMethodID(
        clazz, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    if (!getSystemService_id) {
        SDL_Log("Failed to get getSystemService method ID");
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(activity);
        return false;
    }

    jstring service_name = env->NewStringUTF("vibrator");
    jobject vibrator_service_local =
        env->CallObjectMethod(activity, getSystemService_id, service_name);
    if (!vibrator_service_local) {
        SDL_Log("Failed to get Vibrator service");
        env->DeleteLocalRef(service_name);
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(activity);
        return false;
    }

    vibrateService = env->NewGlobalRef(vibrator_service_local);
    if (!vibrateService) {
        SDL_Log("Failed to create global ref for Vibrator service");
        env->DeleteLocalRef(vibrator_service_local);
        env->DeleteLocalRef(service_name);
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(activity);
        return false;
    }

    jclass vibrator_class = env->GetObjectClass(vibrateService);
    if (!vibrator_class) {
        SDL_Log("Failed to get Vibrator class");
        env->DeleteGlobalRef(vibrateService);
        vibrateService = nullptr;
        // Clean up other local refs
        env->DeleteLocalRef(vibrator_service_local);
        env->DeleteLocalRef(service_name);
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(activity);
        return false;
    }

    // Get the method IDs for vibrate and cancel
    vibrateMethodId = env->GetMethodID(vibrator_class, "vibrate", "(J)V");
    vibrateCancelMethodId = env->GetMethodID(vibrator_class, "cancel", "()V");

    // Clean up all local references
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(service_name);
    env->DeleteLocalRef(vibrator_service_local);
    env->DeleteLocalRef(vibrator_class);

    if (!vibrateMethodId || !vibrateCancelMethodId) {
        SDL_Log("Failed to get vibrate or cancel method ID");
        env->DeleteGlobalRef(vibrateService);
        vibrateService = nullptr;
        return false;
    }

    SDL_Log("Vibration service initialized successfully");
    return true;
}

void vibrationStart(int durationMS) {
    if (!vibrateService || !vibrateMethodId) {
        SDL_Log("Vibration service not initialized");
        return;
    }

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        SDL_Log("Failed to get JNI environment for vibrate call");
        return;
    }

    jlong duration = durationMS;
    env->CallVoidMethod(vibrateService, vibrateMethodId, duration);
}

void vibrationStop() {
    if (!vibrateService || !vibrateCancelMethodId) {
        SDL_Log("Vibration service not initialized");
        return;
    }

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        SDL_Log("Failed to get JNI environment for cancel call");
        return;
    }

    env->CallVoidMethod(vibrateService, vibrateCancelMethodId);
}

void vibrationDeinit() {
    if (vibrateService) {
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        if (env) {
            env->DeleteGlobalRef(vibrateService);
        }
        vibrateService = nullptr;
        vibrateMethodId = 0;
        vibrateCancelMethodId = 0; // Reset the cancel method ID
        SDL_Log("Vibration service cleaned up");
    }
}

#define SPACE 210
#define RADIUS 95

std::vector<Layout> getDefaultLayouts() {

    Button portraitButtons[11] = {
        Button(SDL_CONTROLLER_BUTTON_A, screenWidth * 0.75 + SPACE / 2,
               screenHeight * 0.70 + SPACE, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_B, screenWidth * 0.75 - SPACE / 2,
               screenHeight * 0.70 + SPACE, RADIUS),

        Button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
               screenWidth * 0.75 + SPACE / 2, screenHeight * 0.70 + SPACE * 2,
               RADIUS),
        Button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
               screenWidth * 0.75 + SPACE / 2, screenHeight * 0.70, RADIUS),

        Button(SDL_CONTROLLER_BUTTON_DPAD_UP, screenWidth * 0.30,
               screenHeight * 0.70, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_DPAD_DOWN, screenWidth * 0.30,
               screenHeight * 0.70 + SPACE * 2, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_DPAD_LEFT, screenWidth * 0.30 - SPACE,
               screenHeight * 0.70 + SPACE, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, screenWidth * 0.30 + SPACE,
               screenHeight * 0.70 + SPACE, RADIUS),

        Button(SDL_CONTROLLER_BUTTON_BACK, screenWidth * 0.25 - SPACE / 2,
               screenHeight * 0.60 - SPACE / 2, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_START, screenWidth * 0.25 + SPACE / 2,
               screenHeight * 0.60 - SPACE / 2, RADIUS),
        Button(-1, screenWidth - 50, 50, 100)};

    Button landscapeButtons[11] = {
        Button(SDL_CONTROLLER_BUTTON_A, screenWidth * 0.85 + SPACE / 2,
               screenHeight * 0.50 + SPACE, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_B, screenWidth * 0.85 - SPACE / 2,
               screenHeight * 0.50 + SPACE, RADIUS),

        Button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
               screenWidth * 0.85 + SPACE / 2, screenHeight * 0.50 + SPACE * 2,
               RADIUS),
        Button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
               screenWidth * 0.85 + SPACE / 2, screenHeight * 0.50, RADIUS),

        Button(SDL_CONTROLLER_BUTTON_DPAD_UP, screenWidth * 0.20,
               screenHeight * 0.50, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_DPAD_DOWN, screenWidth * 0.20,
               screenHeight * 0.50 + SPACE * 2, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_DPAD_LEFT, screenWidth * 0.20 - SPACE,
               screenHeight * 0.50 + SPACE, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, screenWidth * 0.20 + SPACE,
               screenHeight * 0.50 + SPACE, RADIUS),

        Button(SDL_CONTROLLER_BUTTON_BACK, screenWidth * 0.05 - SPACE / 2,
               screenHeight * 0.20 - SPACE / 2, RADIUS),
        Button(SDL_CONTROLLER_BUTTON_START, screenWidth * 0.05 + SPACE / 2,
               screenHeight * 0.20 - SPACE / 2, RADIUS),
        Button(-1, screenWidth - 50, 50, 100)};

    return {Layout(portraitButtons), Layout(landscapeButtons)};
}

void setLayout() {
    layout = savefile->settings.onScreenLayouts[(screenWidth > screenHeight)];
}

void updateBatteryInfo() {
    SDL_PowerState state = SDL_GetPowerInfo(NULL, &batteryPercentage);

    charging = false;
    if (state == SDL_POWERSTATE_CHARGING) {
        charging = true;
    } else if (state != SDL_POWERSTATE_ON_BATTERY) {
        batteryPercentage = -1;
    }
}
#endif

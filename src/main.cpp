#include "SDL3/SDL_events.h"
#include "SDL3/SDL_scancode.h"
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>
#include <string>

constexpr uint32_t kGameWidth = 40;
constexpr uint32_t kGameHeight = 30;
constexpr uint32_t kBlockSizeInPixels = 20;
constexpr uint32_t kWindowWidth = (kGameWidth * kBlockSizeInPixels);
constexpr uint32_t kWindowHeight = (kGameHeight * kBlockSizeInPixels);

struct AircraftContext {
    float xpos;
    float ypos;
};

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    AircraftContext aircraft_ctx;
};

SDL_AppResult LogErrAndFail(const std::string msg) {
    const std::string msg_with_err = msg + "\n%s";
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg_with_err.c_str(), SDL_GetError());
    return SDL_APP_FAILURE;
}

// run once at startup
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_SetAppMetadata("unnamed_game", "1.0", "org.libsdl.unnamed_game")) {
        return LogErrAndFail("Failed to set app metadata");
    }
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        return LogErrAndFail("Couldn't init SDL");
    }
    AppState *app = new AppState();
    if (!app) {
        return LogErrAndFail("Couldn't allocate resources for app state");
    }
    *appstate = app;
    app->aircraft_ctx.xpos = (kWindowWidth / 2.0f) - kBlockSizeInPixels;
    app->aircraft_ctx.ypos = kWindowHeight - 2 * kBlockSizeInPixels;
    if (!SDL_CreateWindowAndRenderer("unnamed_game", kWindowWidth,
                                     kWindowHeight, SDL_WINDOW_RESIZABLE,
                                     &app->window, &app->renderer)) {
        return LogErrAndFail("Failed to create window/renderer");
    }
    SDL_SetRenderLogicalPresentation(app->renderer, kWindowWidth, kWindowHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return SDL_APP_CONTINUE;
}

static SDL_AppResult HandleKeyDownEvent(AircraftContext *ctx,
                                        const SDL_Scancode key_code) {
    const float xpos = ctx->xpos;
    const float ypos = ctx->xpos;
    switch (key_code) {
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q:
            return SDL_APP_SUCCESS;
        case SDL_SCANCODE_LEFT:
            if (xpos - kBlockSizeInPixels >= 0) {
                ctx->xpos = xpos - kBlockSizeInPixels;
            } else {
                ctx->xpos = 0;
            }
            break;
        case SDL_SCANCODE_RIGHT:
            if (xpos + kBlockSizeInPixels <= kWindowWidth) {
                ctx->xpos = xpos + kBlockSizeInPixels;
            } else {
                ctx->xpos = kWindowWidth - kBlockSizeInPixels;
            }
            break;
        case SDL_SCANCODE_UP:
            if (ypos - kBlockSizeInPixels >= 0) {
                ctx->ypos = ypos - kBlockSizeInPixels;
            } else {
                ctx->ypos = 0;
            }
            break;
        case SDL_SCANCODE_DOWN:
            if (ypos + kBlockSizeInPixels <= kWindowHeight) {
                ctx->ypos = ypos + kBlockSizeInPixels;
            } else {
                ctx->ypos = kWindowHeight - kBlockSizeInPixels;
            }
            break;
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AircraftContext *ctx = &((AppState *)appstate)->aircraft_ctx;
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            return HandleKeyDownEvent(ctx, event->key.scancode);
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

// run once per frame
SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *app = (AppState *)appstate;
    SDL_FRect rect;
    SDL_SetRenderDrawColor(app->renderer, 32, 32, 32, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);
    SDL_SetRenderDrawColor(app->renderer, 7, 54, 66, SDL_ALPHA_OPAQUE);
    rect.x = app->aircraft_ctx.xpos;
    rect.y = app->aircraft_ctx.ypos;
    rect.w = rect.h = kBlockSizeInPixels;
    SDL_RenderFillRect(app->renderer, &rect);
    SDL_RenderPresent(app->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != NULL) {
        AppState *app = (AppState *)appstate;
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }
    SDL_Log("app quit successfully");
    SDL_Quit();
}

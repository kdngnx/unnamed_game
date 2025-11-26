#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>
#include <string>
#include <vector>

constexpr uint32_t kGameWidth = 40;
constexpr uint32_t kGameHeight = 30;
constexpr uint32_t kBlockSizeInPixels = 20;
constexpr uint32_t kWindowWidth = (kGameWidth * kBlockSizeInPixels);
constexpr uint32_t kWindowHeight = (kGameHeight * kBlockSizeInPixels);

struct Bullet {
    float xpos;
    float ypos;
    uint64_t last_step;
};

struct Aircraft {
    float xpos;
    float ypos;
    std::vector<Bullet> bullets;
    uint32_t bullet_length = kBlockSizeInPixels / 2;
    uint32_t bullet_velocity_ms = 20;
};

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Aircraft aircraft;
};

SDL_AppResult LogErrAndFail(const std::string msg) {
    const std::string msg_with_err = msg + "\n%s";
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg_with_err.c_str(), SDL_GetError());
    return SDL_APP_FAILURE;
}

static void AddBullet(Aircraft *aircraft) {
    Bullet bullet = {aircraft->xpos + aircraft->bullet_length,
                     aircraft->ypos - aircraft->bullet_length, SDL_GetTicks()};
    aircraft->bullets.push_back(bullet);
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
    app->aircraft.xpos = (kWindowWidth / 2.0f) - kBlockSizeInPixels;
    app->aircraft.ypos = kWindowHeight - 2 * kBlockSizeInPixels;
    if (!SDL_CreateWindowAndRenderer("unnamed_game", kWindowWidth,
                                     kWindowHeight, SDL_WINDOW_RESIZABLE,
                                     &app->window, &app->renderer)) {
        return LogErrAndFail("Failed to create window/renderer");
    }
    SDL_SetRenderLogicalPresentation(app->renderer, kWindowWidth, kWindowHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return SDL_APP_CONTINUE;
}

static SDL_AppResult HandleKeyDownEvent(Aircraft *ctx,
                                        const SDL_Scancode key_code) {
    const float xpos = ctx->xpos;
    const float ypos = ctx->ypos;
    switch (key_code) {
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q:
            return SDL_APP_SUCCESS;
        case SDL_SCANCODE_SPACE:
            AddBullet(ctx);
            break;
        case SDL_SCANCODE_LEFT: {
            const float new_pos = ctx->xpos - kBlockSizeInPixels;
            ctx->xpos = (new_pos < 0) ? 0 : new_pos;
            break;
        }
        case SDL_SCANCODE_RIGHT: {
            const float new_pos = ctx->xpos + kBlockSizeInPixels;
            const float barrier = kWindowWidth - kBlockSizeInPixels;
            ctx->xpos = (new_pos > barrier) ? barrier : new_pos;
            break;
        }
        case SDL_SCANCODE_UP: {
            const float new_pos = ctx->ypos - kBlockSizeInPixels;
            ctx->ypos = (new_pos < 0) ? 0 : new_pos;
            break;
        }
        case SDL_SCANCODE_DOWN: {
            const float new_pos = ctx->ypos + kBlockSizeInPixels;
            const float barrier = kWindowHeight - kBlockSizeInPixels;
            ctx->ypos = (new_pos > barrier) ? barrier : new_pos;
            break;
        }
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *app = (AppState *)appstate;
    Aircraft *aircraft = &app->aircraft;
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            return HandleKeyDownEvent(aircraft, event->key.scancode);
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_FRect GetAircraftSprite(const AppState *app) {
    SDL_FRect rect;
    SDL_SetRenderDrawColor(app->renderer, 7, 54, 66, SDL_ALPHA_OPAQUE);
    rect.x = app->aircraft.xpos;
    rect.y = app->aircraft.ypos;
    rect.w = rect.h = kBlockSizeInPixels;
    return rect;
}

// run once per frame
SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *app = (AppState *)appstate;
    Aircraft *aircraft = &app->aircraft;
    const uint64_t now = SDL_GetTicks();

    SDL_SetRenderDrawColor(app->renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    SDL_FRect aircraft_sprite = GetAircraftSprite(app);
    SDL_RenderFillRect(app->renderer, &aircraft_sprite);

    SDL_SetRenderDrawColor(app->renderer, 20, 20, 20, SDL_ALPHA_OPAQUE);
    std::vector<int> removed_bullets;
    for (int i = 0; i < aircraft->bullets.size(); i++) {
        if (aircraft->bullets[i].ypos < 0) {
            removed_bullets.push_back(i);
            continue;
        }
        while ((now - aircraft->bullets[i].last_step) >=
               aircraft->bullet_velocity_ms) {
            aircraft->bullets[i].ypos--;
            aircraft->bullets[i].last_step += aircraft->bullet_velocity_ms;
        }
        SDL_RenderLine(app->renderer, aircraft->bullets[i].xpos,
                       aircraft->bullets[i].ypos - (kBlockSizeInPixels / 2.0f),
                       aircraft->bullets[i].xpos, aircraft->bullets[i].ypos);
    }
    SDL_Log("removed bullets: %lu", removed_bullets.size());

    for (int index : removed_bullets) {
        aircraft->bullets.erase(aircraft->bullets.begin() + index);
    }
    SDL_Log("bullets: %lu", aircraft->bullets.size());

    SDL_RenderPresent(app->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != NULL) {
        AppState *app = (AppState *)appstate;
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        app->aircraft.bullets.clear();
        delete app;
    }
    SDL_Log("app quit successfully");
    SDL_Quit();
}

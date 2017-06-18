#include "Display.h"
#include "Platform.h"
#include <iostream>
namespace JustRay
{
bool Display::Startup(int xResolution, int yResolution, bool showWindow)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    window_ = SDL_CreateWindow("Shiny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, xResolution, yResolution, SDL_WINDOW_OPENGL | (showWindow ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN));
    if (window_ == nullptr) {
        return false;
    }
    SDL_GLContext glContext = SDL_GL_CreateContext(window_);
    if (!glContext) {
        return false;
    }
    if (SDL_GL_SetSwapInterval(1) < 0) {
        return false;
    }

    SDL_GL_SetSwapInterval(1);
    //    SDL_SetRelativeMouseMode(SDL_TRUE);
    glewExperimental = true;
    glewInit();

    int a = 0, b = 0;
    //std::cerr << SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &a) << ": " << a << std::endl;
    //std::cerr << SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &b) << ": " << b << std::endl;
    //glEnable(GL_MULTISAMPLE);
    running_ = true;
    return true;
}

void Display::Update()
{
    SDL_GL_SwapWindow(window_);
    SDL_Event event;
    if (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            running_ = false;
        }
        input_.UpdateKeyboard(SDL_GetKeyboardState(nullptr));
    }
}

void Display::Shutdown()
{
    running_ = false;
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

bool Display::Running()
{
    return running_;
}

const Input* Display::GetInput() const
{
    return &input_;
}
}
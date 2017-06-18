#pragma once
#include "Input.h"
#include "SDL\SDL.h"
#undef main
namespace JustRay
{
class Display {
public:
    bool Startup(int xResolution, int yResolution, bool showWindow = false);
    void Update();
    void Shutdown();
    bool Running();
    const Input* GetInput() const;
private:
    bool running_ = false;
    SDL_Window* window_ = nullptr;
    Input input_;
};
}
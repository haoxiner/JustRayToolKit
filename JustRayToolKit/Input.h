#pragma once
#include <cstdint>
namespace JustRay
{
class Input {
public:
    void UpdateKeyboard(const uint8_t* state);
    void UpdateMouse(unsigned int state, int x, int y);
    float GetLeftHorizontalAxis() const;
    float GetLeftVerticalAxis() const;
    float GetRightHorizontalAxis() const;
    float GetRightVerticalAxis() const;
    bool Fire0() const;
    bool Test() const;
    bool Jump() const;
    bool Quit() const;
private:
    float leftHorizontalAxis_ = 0.0f;
    float leftVerticalAxis_ = 0.0f;
    float rightHorizontalAxis_ = 0.0f;
    float rightVerticalAxis_ = 0.0f;
    bool fire0_ = false;
    bool test_ = false;
    bool jump_ = false;
    bool quit_ = false;
};
}
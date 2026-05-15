#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>

enum class JoystickInput {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

struct InputState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool a = false;
    bool b = false;
    JoystickInput joystickInput = JoystickInput::NONE;

    JoystickInput joystickRisingEdge = JoystickInput::NONE;
    bool upRisingEdge = false;
    bool downRisingEdge = false;
    bool leftRisingEdge = false;
    bool rightRisingEdge = false;
    bool aRisingEdge = false;
    bool bRisingEdge = false;
};

class InputManager {
public:
    void begin();
    void update();
    const InputState &state() const;

private:
    InputState current;
    InputState previous;
};

#endif

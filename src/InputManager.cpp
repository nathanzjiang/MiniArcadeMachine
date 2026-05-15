#include "InputManager.h"

void InputManager::begin() {
    current = InputState{};
    previous = InputState{};

    pinMode(5, INPUT);
    pinMode(6, INPUT);
}

void InputManager::update() {
    previous = current;

    int dUp = 1800 - analogRead(A0);
    int dDown = analogRead(A0) - 2000;
    int dLeft = analogRead(A1) - 2050;
    int dRight = 1850 - analogRead(A1);

    current.a = digitalRead(5);
    current.b = digitalRead(6);

    current.up = dUp > 0;
    current.down = dDown > 0;
    current.left = dLeft > 0;
    current.right = dRight > 0;

    if (dUp > 0 && dUp > max(dDown, max(dLeft, dRight))) {
        current.joystickInput = JoystickInput::UP;
    } else if (dRight > 0 && dRight > max(dDown, max(dLeft, dUp))) {
        current.joystickInput = JoystickInput::RIGHT;
    } else if (dDown > 0 && dDown > max(dUp, max(dLeft, dRight))) {
        current.joystickInput = JoystickInput::DOWN;
    } else if (dLeft > 0 && dLeft > max(dUp, max(dDown, dRight))) {
        current.joystickInput = JoystickInput::LEFT;
    } else {
        current.joystickInput = JoystickInput::NONE;
    }

    current.joystickRisingEdge = current.joystickInput == previous.joystickInput ? JoystickInput::NONE : current.joystickInput;

    // Button pin reads will go here once the controls are wired.
    current.upRisingEdge = current.up && !previous.up;
    current.downRisingEdge = current.down && !previous.down;
    current.leftRisingEdge = current.left && !previous.left;
    current.rightRisingEdge = current.right && !previous.right;
    current.aRisingEdge = current.a && !previous.a;
    current.bRisingEdge = current.b && !previous.b;
}

const InputState &InputManager::state() const {
    return current;
}

#pragma once

#include "CoreMinimal.h"

class GAMECORE_API InputContainer {

public:
    InputContainer();
    ~InputContainer();
    InputContainer(InputContainer &other);
    InputContainer &operator=(InputContainer &other);

    void setForwardAxis(float value);
    void setRightAxis(float value);

    void setInteractKeyPressedTrue();
    bool interactKeyWasPressed();

    float forwardAxisValue();
    float rightAxisValue();

private:
    //W and S key
    float forwardAxis = 0.0f;
    
    //A and D key
    float rightAxis = 0.0f;

    bool spacebarPressed = false;
    bool interactKeyPressed = false;
};
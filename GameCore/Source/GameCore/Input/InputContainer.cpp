#include "InputContainer.h"


InputContainer::InputContainer(){

}

InputContainer::~InputContainer(){

}

InputContainer::InputContainer(InputContainer &other){
    if(this != &other){
        *this = other;
    }
}

InputContainer &InputContainer::operator=(InputContainer &other){
    if(this != &other){
        forwardAxis = other.forwardAxis;
        rightAxis = other.rightAxis;
        spacebarPressed = other.spacebarPressed;
    }
    return *this;
}


float InputContainer::forwardAxisValue(){
    return forwardAxis;
}
float InputContainer::rightAxisValue(){
    return rightAxis;
}

void InputContainer::setForwardAxis(float value){
    forwardAxis = value;
}

void InputContainer::setRightAxis(float value){
    rightAxis = value;
}

void InputContainer::setInteractKeyPressedTrue(){
    interactKeyPressed = true;
}

bool InputContainer::interactKeyWasPressed(){
    if(interactKeyPressed){
        interactKeyPressed = false;
        return true;
    }
    return false;
}
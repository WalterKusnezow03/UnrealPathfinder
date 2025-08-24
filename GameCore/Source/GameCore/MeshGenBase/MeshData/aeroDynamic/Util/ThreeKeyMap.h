#pragma once

#include "CoreMinimal.h"

/**
 * will save a area of a triangle by 3 indices
 */
class GAMECORE_API ThreeKeyMap{

public:
    ThreeKeyMap();
    ~ThreeKeyMap();

    bool findValue(int v0, int v1, int v2, float &outValue);
    void replaceValue(int v0, int v1, int v2, float value);

    void removeValue(int v0);
    void removeValue(int v0, int v1, int v2);

private:


    std::vector<FString> createAllKeys(int v0, int v1, int v2);
    FString createKey(int v0, int v1, int v2);
    std::map<FString, float> stringKeyMap;
};
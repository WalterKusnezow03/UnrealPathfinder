#pragma once

#include "CoreMinimal.h"

class GAMECORE_API PlayerInfo {

public:
    static FVector playerLocation();
	static FVector playerLookDir();
	static FRotator playerRotation();

    static void Update(
        FVector location,
        FRotator rotation,
        FVector lookDir
    );


private:
    static FVector playerLocationData;
    static FVector playerLookDirData;
    static FRotator playerRotationData;

};
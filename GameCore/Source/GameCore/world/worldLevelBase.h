#pragma once

#include "CoreMinimal.h"

/**
 * acts as a task interface for plugins - processed in Sub Game Project!
 */
class GAMECORE_API worldLevelBase {

public:
    static void addOutpostAt(FVector &location);

    static bool DebugSkelletonRecordMode(){
        return false;
    }

protected:
    static TArray<FVector> outpostsToCreate;
    
};
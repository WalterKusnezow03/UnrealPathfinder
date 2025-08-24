#pragma once

#include "CoreMinimal.h"

/// @brief add ignored actors here which can be removed later by ptr.
/// provides FCollision params, trace complex set to false. More dynamic handling of tracked actors
/// then FCollisionQueryParams
/// Is needed because FCollisionQueryParams cant remove by pointer!
class GAMECORE_API CollisionTracker {

public:
    CollisionTracker();
    ~CollisionTracker();

    /// @brief adds a actor to the raycast params if not nullptr
    /// @param actor 
    void AddIgnoredActor(AActor *actor);

    /// @brief tries to remove an actor from the raycast params
    /// @param actor 
    void RemoveIgnoredActor(AActor *actor);

    /// @brief gets the collison params with ignored actor list internal
    FCollisionQueryParams getCollisonParams();

    /// @brief copies the tracked actor array
    TArray<AActor *> copyTracked();

    /// @brief creates raycast params with passed ignored actors
    /// @param actors actors to ignore 
    static FCollisionQueryParams makeParams(TArray<AActor *> &actors);

private:
    TArray<AActor *> tracked;
};
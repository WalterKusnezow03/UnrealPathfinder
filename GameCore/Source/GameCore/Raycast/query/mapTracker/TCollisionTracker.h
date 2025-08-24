#pragma once

#include "CoreMinimal.h"
#include "GameCore/Raycast/query/CollisionTracker.h"

/// @brief tracks actors by type T for collision trackings
/// @tparam T 
template <typename T>
class GAMECORE_API TCollisionTracker {

public:
    TCollisionTracker(){

    }

    ~TCollisionTracker(){

    }

    /// @brief adds a actor to the raycast params if not nullptr with repect to template type t
    void AddIgnoredActor(T type, AActor *actor){
        CollisionTracker &ref = find(type);
        ref.AddIgnoredActor(actor);
    }

    /// @brief removes an actor from the raycast params found by type t
    void RemoveIgnoredActor(T type, AActor *actor){
        CollisionTracker &ref = find(type);
        ref.RemoveIgnoredActor(actor);
    }

    /// @brief creates the raycast params with respect to t
    /// @param type type to ignore
    FCollisionQueryParams getCollisonParams(T type){
        CollisionTracker &ref = find(type);
        return ref.getCollisonParams();
    }

    /// @brief gets the collision params without respect to template type
    FCollisionQueryParams getAllCollisonParams(){
        TArray<AActor *> allActors;
        for(auto &pair : collisionTrackerMap){
            CollisionTracker &current = pair.second;
            TArray<AActor *> currentPtrs = current.copyTracked();
            for (int i = 0; i < currentPtrs.Num(); i++){
                AActor *actor = currentPtrs[i];
                if(actor && !allActors.Contains(actor)){
                    allActors.Add(actor);
                }
            }
        }
        return CollisionTracker::makeParams(allActors);
    }

private:

    CollisionTracker &find(T type){
        if(collisionTrackerMap.find(type) == collisionTrackerMap.end()){
            collisionTrackerMap[type] = CollisionTracker();
        }
        return collisionTrackerMap[type];
    }

    std::map<T, CollisionTracker> collisionTrackerMap;
};
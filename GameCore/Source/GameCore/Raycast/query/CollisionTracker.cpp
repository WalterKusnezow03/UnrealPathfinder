#include "CollisionTracker.h"
#include "GameCore/DebugHelper.h"

CollisionTracker::CollisionTracker(){

}

CollisionTracker::~CollisionTracker(){
    tracked.Empty();
}

void CollisionTracker::AddIgnoredActor(AActor *actor){
    if(actor){
        if(!tracked.Contains(actor)){
            tracked.Add(actor);
            //DebugHelper::logMessage("CollisionTracker tracked added actor ", tracked.Num());
        }
    }
}

void CollisionTracker::RemoveIgnoredActor(AActor *actor){
    if(actor){
        if(tracked.Num() > 0 && tracked.Contains(actor)){
            int foundAtIndex = -1;
            //swap with back
            for (int i = 0; i < tracked.Num(); i++){
                AActor *current = tracked[i];
                if(current == actor){
                    foundAtIndex = i;
                    break;
                }
            }

            //swap to avoid array shift.
            if(foundAtIndex != -1){
                tracked[foundAtIndex] = tracked[tracked.Num() - 1]; //swap
                tracked.Pop(); //pop back
            }
        }
    }
}



/// @brief params with tracked actors, bTraceComplex false
/// @return 
FCollisionQueryParams CollisionTracker::getCollisonParams(){
    return makeParams(tracked);
}

FCollisionQueryParams CollisionTracker::makeParams(TArray<AActor *> &actors){
    FCollisionQueryParams Params;
    Params.bTraceComplex = false;
    for (int i = 0; i < actors.Num(); i++){
        if(AActor *current = actors[i]){
            Params.AddIgnoredActor(current);
        }
    }
    return Params;
}




TArray<AActor *> CollisionTracker::copyTracked(){
    return tracked;
}
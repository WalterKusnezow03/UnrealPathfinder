
#include "raycastTask.h"
#include "CoreMinimal.h"
#include "PathFinder/pathFinding/PathFinder.h"

raycastTask::raycastTask(){
    wasExecuted = false;
    a = nullptr;
    b = nullptr;
    worldPointer = nullptr;
}

raycastTask::~raycastTask(){
    a = nullptr;
    b = nullptr;
    worldPointer = nullptr;
}

raycastTask::raycastTask(const raycastTask &other){
    *this = other;
}

raycastTask &raycastTask::operator=(const raycastTask &other){
    if(this == &other){
        return *this;
    }
    a = other.a;
    b = other.b;
    wasExecuted = other.wasExecuted;

    return *this;
}

void raycastTask::setup(
    UWorld *world,
    APathFinder::Node *aIn,
    APathFinder::Node *bIn
){
    worldPointer = world;
    a = aIn;
    b = bIn;
}


void raycastTask::execute(){
    if(!wasExecuted){
        if(a != nullptr && b != nullptr){
            if(passTangentialCheck()){
                if(syncRaycastCanSee()){
                    //lock
                    a->addTangentialNeighbor(b);
                    b->addTangentialNeighbor(a);
                }
            }
        }
        wasExecuted = true;
    }
}

bool raycastTask::passTangentialCheck(){
    if(a != nullptr && b != nullptr){
        APathFinder *instance = APathFinder::instance();
        if(instance != nullptr){
            return instance->passTangentailCheck(a, b);
        }
    }
    return false;
}

bool raycastTask::syncRaycastCanSee(){
    if(a == nullptr || b == nullptr){
        return false;
    }


    if(worldPointer){
        FHitResult HitResult;
        FCollisionQueryParams Params;

        //add params from entity manager (contains all bots for example, which can be ignored)
        //part of a bigger context im working on, comment out or provide your own params
        if(APathFinder *e = APathFinder::instance()){
            Params = e->getIgnoredRaycastParams();
        }
        Params.bTraceComplex = false; // HIER LOWER RAYCAST DETAIL-> FASTER

        //default casting
        FVector dir = b->pos - a->pos;
        dir = dir.GetSafeNormal();
        FVector Start = a->pos + dir * 100.0f;
        FVector End = b->pos;

        bool hittedAnything = worldPointer->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

        if(hittedAnything){
            FVector hitPos = HitResult.ImpactPoint;
            //if distance close enough to target, forgive the hit.
            if(FVector::Dist(hitPos, b->pos) <= 25){ //25cm
                hittedAnything = false; //testing needed  
            }
        }
        

        bool canSee = !hittedAnything;
        return canSee;
    }
    return false;
}




